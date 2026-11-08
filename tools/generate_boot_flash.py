"""Emit the 16-byte default flash image used by pinned XiangShan difftest.

Pass its absolute path to emu -F so both DUT and the legacy NEMU flash API
load identical bytes. This is boot-input alignment, not a difftest bypass.
"""
import argparse
import hashlib
from pathlib import Path
import struct

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    # addiw t0,zero,1; slli t0,t0,31; jalr zero,0(t0); padding.
    data = struct.pack('<IIII', 0x0010029b, 0x01f29293, 0x00028067, 0)
    if args.output.exists() and args.output.read_bytes() != data:
        raise SystemExit('Refusing to overwrite a different existing flash image')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(data)
    print(f'{args.output.resolve()} ({len(data)} bytes)')
    print('sha256:', hashlib.sha256(data).hexdigest())

if __name__ == '__main__':
    main()
