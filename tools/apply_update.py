"""Apply the archived modules to a private compatible unsigned IPA."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import plistlib
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.exists() or output == ROOT or ROOT in output.parents:
        parser.error('Choose a new private output file outside the archive')
    partial = output.with_name(output.name + '.partial')
    if partial.exists():
        parser.error('A previous partial output exists. Inspect it before retrying')
    kit = ROOT / 'update-kit-v21'
    manifest = json.loads((kit / 'manifest.json').read_text(encoding='utf-8'))
    replacements = {}
    for name, digest in manifest['files'].items():
        data = (kit / name).read_bytes()
        if hashlib.sha256(data).hexdigest() != digest:
            raise ValueError('Module hash mismatch: ' + name)
        replacements[name] = data
    with zipfile.ZipFile(args.input) as source:
        names = source.namelist()
        if len(names) != len(set(names)):
            raise ValueError('Duplicate ZIP entries')
        for name in names:
            path = PurePosixPath(name)
            if path.is_absolute() or '..' in path.parts or '\\' in name:
                raise ValueError('Unsafe ZIP path')
            if 'embedded.mobileprovision' in name or '_CodeSignature' in path.parts:
                raise ValueError('Provide the private unsigned input, before personal signing')
        candidates = [name for name in names if name.startswith('Payload/')
                      and name.count('/') == 2 and name.endswith('.app/Info.plist')]
        if len(candidates) != 1:
            raise ValueError('Expected one root application')
        root = candidates[0][:-len('Info.plist')]
        info = plistlib.loads(source.read(root + 'Info.plist'))
        executable = root + info['CFBundleExecutable']
        if hashlib.sha256(source.read(executable)).hexdigest() != manifest['host_executable_sha256']:
            raise ValueError('This archive supports only the pinned Mac 2.5.0 converted host')
        if info.get('SHIMMacRoot') != 'Mac':
            raise ValueError('Expected the already converted Documents/GameData host')
        for relative in replacements:
            if root + relative not in names:
                raise ValueError('Original module is absent: ' + relative)
        child = root + 'Extensions/Downloader.appex/Info.plist'
        if child not in names:
            raise ValueError('Expected the Background Assets child extension')
        info.update(CFBundleDisplayName='赛博朋克 2077', CFBundleVersion=manifest['build'],
                    PortHDRLegacyQueryReturnOffset=manifest['hdr_query_return_offset'])
        changes = {root + name: data for name, data in replacements.items()}
        changes[root + 'Info.plist'] = plistlib.dumps(info, fmt=plistlib.FMT_BINARY)
        child_info = plistlib.loads(source.read(child))
        child_info['CFBundleVersion'] = manifest['build']
        changes[child] = plistlib.dumps(child_info, fmt=plistlib.FMT_BINARY)
        output.parent.mkdir(parents=True, exist_ok=True)
        with partial.open('xb') as stream:
            with zipfile.ZipFile(stream, 'w', zipfile.ZIP_STORED, allowZip64=False) as destination:
                for entry in source.infolist():
                    destination.writestr(entry, changes.get(entry.filename, source.read(entry.filename)))
    with zipfile.ZipFile(partial) as archive:
        if archive.testzip():
            raise ValueError('Output CRC failed')
    partial.rename(output)
    print(json.dumps({'created_unsigned_update': str(output), 'build': manifest['build'],
                      'device_installed': False}, ensure_ascii=False))


if __name__ == '__main__':
    main()
