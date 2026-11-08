"""Independent scalar oracle for the CIE low-eight-lane vdot convention."""
def dot8(a, b):
    if len(a) != 16 or len(b) != 16:
        raise ValueError('Each source must contain 16 signed bytes')
    if any(type(x) is not int or not -128 <= x <= 127 for x in (*a, *b)):
        raise ValueError('Inputs must be signed int8')
    return sum(a[i] * b[i] for i in range(8))

def pack(a):
    return int.from_bytes(bytes(x & 255 for x in a), 'little')

def encode(vd, vs1, vs2):
    if any(type(x) is not int or not 0 <= x < 32 for x in (vd, vs1, vs2)):
        raise ValueError('Vector register index must be 0..31')
    # custom-0, funct7=0000001 (vm=1), funct3=010.
    return 0x0200200B | (vd << 7) | (vs1 << 15) | (vs2 << 20)
