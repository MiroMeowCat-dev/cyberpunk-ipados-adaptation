"""Verify the public archive manifest and reject accidental private/game payloads."""
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]
FORBIDDEN_SUFFIXES = {'.ipa', '.nsp', '.xci', '.archive', '.p12', '.pfx', '.mobileprovision', '.key', '.cer'}
SENSITIVE = re.compile(rb'(?:C:[/\\]+Users[/\\]+[^/\\\s]+|\b[0-9a-f]{8}-[0-9a-f]{16}\b|'
                       rb'[a-z0-9._%+-]+@icloud[.]com|'
                       rb'-----BEGIN (?:RSA |EC |ENCRYPTED )?PRIVATE KEY-----)', re.I)


def inspect(name, data, source_zip=False):
    path = PurePosixPath(name)
    if path.is_absolute() or '..' in path.parts or '\\' in name:
        raise ValueError('Unsafe archive name: ' + name)
    if path.suffix.lower() in FORBIDDEN_SUFFIXES:
        raise ValueError('Excluded file type: ' + name)
    if any(part in ('Payload', 'signing-checkpoints', 'device-connection', 'node_modules', 'target') for part in path.parts):
        raise ValueError('Excluded directory: ' + name)
    if path.name in ('Cyberpunk2077', 'AppIcon.icns', 'checkpoint.json', 'startup-console.txt'):
        raise ValueError('Game/private payload: ' + name)
    if SENSITIVE.search(data):
        raise ValueError('Private material detected: ' + name)
    if data[:4] == b'\xcf\xfa\xed\xfe':
        if name not in ('update-kit-v21/Frameworks/libShim_AppKit.dylib',
                        'update-kit-v21/Frameworks/libShim_CoreGraphics.dylib'):
            raise ValueError('Unapproved Mach-O payload: ' + name)


def main():
    manifest = json.loads((ROOT / 'MANIFEST.json').read_text(encoding='utf-8'))
    files = {p.relative_to(ROOT).as_posix(): p for p in ROOT.rglob('*')
             if p.is_file() and '.git' not in p.parts and '__pycache__' not in p.parts
             and p.name != 'MANIFEST.json'}
    if set(files) != set(manifest['files']):
        raise ValueError('Public file inventory differs from the manifest')
    for name, path in files.items():
        data = path.read_bytes()
        if hashlib.sha256(data).hexdigest() != manifest['files'][name]:
            raise ValueError('Hash mismatch: ' + name)
        inspect(name, data)
        if path.suffix == '.zip':
            with zipfile.ZipFile(path) as archive:
                if archive.testzip():
                    raise ValueError('Nested ZIP CRC failed')
                for entry in archive.infolist():
                    if not entry.is_dir():
                        inspect(entry.filename, archive.read(entry), source_zip=True)
    kit = json.loads((ROOT / 'update-kit-v21/manifest.json').read_text(encoding='utf-8'))
    for name, expected in kit['files'].items():
        if hashlib.sha256((ROOT / 'update-kit-v21' / name).read_bytes()).hexdigest() != expected:
            raise ValueError('Update kit module hash mismatch')
    print(json.dumps({'archive_valid': True, 'files': len(files) + 1,
                      'game_and_private_materials_included': False}))


if __name__ == '__main__':
    main()
