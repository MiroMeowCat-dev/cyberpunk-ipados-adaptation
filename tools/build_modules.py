"""Build the archived compatibility modules with a locally supplied Apple SDK."""
import argparse
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def run(arguments):
    result = subprocess.run([str(value) for value in arguments], capture_output=True,
                            text=True, encoding='utf-8', errors='replace', timeout=180)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)


def mark_reexport(path, dependency):
    data = bytearray(path.read_bytes())
    cursor = 32
    found = False
    for _ in range(struct.unpack_from('<I', data, 16)[0]):
        command, size = struct.unpack_from('<II', data, cursor)
        if command in (12, 0x8000001f, 0x80000018):
            start = cursor + struct.unpack_from('<I', data, cursor + 8)[0]
            name = data[start:data.index(0, start)].decode()
            if name == dependency:
                struct.pack_into('<I', data, cursor, 0x8000001f)
                found = True
        cursor += size
    if not found:
        raise ValueError('Missing reexport dependency')
    flags = struct.unpack_from('<I', data, 24)[0]
    struct.pack_into('<I', data, 24, flags & ~0x100000)
    path.write_bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--toolchain-bin', required=True, type=Path)
    parser.add_argument('--sdk', required=True, type=Path)
    parser.add_argument('--out', required=True, type=Path)
    args = parser.parse_args()
    output = args.out.resolve()
    if output.exists() or output == ROOT or ROOT in output.parents:
        parser.error('Choose a new output directory outside this archive')
    output.mkdir(parents=True)
    tc, sdk = args.toolchain_bin.resolve(), args.sdk.resolve()
    modules = {
        'AppKit': ['AppKit.m', 'AppKit_Flat.m', 'AppKit_RuntimeHooks.m',
                   'AppKit_Stats.m', 'Shim_AppKit.m', 'LocalDiagnostics.m', 'PortZeldaGamepad.m'],
        'CoreGraphics': ['CoreGraphicsDisplay.m', 'Shim_CoreGraphics.m'],
    }
    for module, sources in modules.items():
        objects = []
        for name in sources:
            target = output / (Path(name).stem + '.o')
            run([tc / 'clang.exe', '-target', 'arm64-apple-ios26.0', '-isysroot', sdk,
                 '-F', sdk / 'System/Library/SubFrameworks', '-fobjc-arc', '-fblocks',
                 '-O2', '-Wall', '-Wextra', '-Wno-unused-parameter',
                 '-Wno-deprecated-declarations', '-Wno-incompatible-library-redeclaration',
                 '-ffile-prefix-map=' + str(ROOT) + '=/archive',
                 '-c', ROOT / 'src' / name, '-o', target])
            objects.append(target)
        library = output / ('libShim_' + module + '.dylib')
        framework = 'UIKit' if module == 'AppKit' else 'CoreGraphics'
        command = [tc / 'lld.exe', '-flavor', 'darwin', '-arch', 'arm64',
                   '-platform_version', 'ios', '26.0', '26.5', '-dylib',
                   '-no_adhoc_codesign', '-install_name', '@rpath/' + library.name,
                   '-syslibroot', sdk, '-F', sdk / 'System/Library/Frameworks',
                   '-F', sdk / 'System/Library/SubFrameworks', '-L', sdk / 'usr/lib',
                   '-o', library, *objects, '-lSystem', '-lobjc',
                   '-reexport_framework', framework]
        for dependency in ('Foundation', 'UIKit', 'QuartzCore', 'CoreGraphics',
                           'Metal', 'GameController', 'CoreMotion'):
            command.extend(['-framework', dependency])
        run(command)
        mark_reexport(library, '/System/Library/Frameworks/' + framework + '.framework/' + framework)
    print(json.dumps({'compiled': True, 'device_validated': False,
                      'output': str(output)}))


if __name__ == '__main__':
    main()
