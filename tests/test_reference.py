import unittest
from software.reference import dot8, pack, encode

class ReferenceTests(unittest.TestCase):
    def test_extrema(self):
        self.assertEqual(dot8([-128]*16, [-128]*16), 131072)
        self.assertEqual(dot8([-128]*16, [127]*16), -130048)
        self.assertEqual(dot8([127]*16, [127]*16), 129032)

    def test_high_lanes_ignored(self):
        self.assertEqual(dot8([1]*8+[127]*8, [2]*8+[-128]*8), 16)

    def test_encoding(self):
        self.assertEqual(encode(3,1,2), 0x0220A18B)
        self.assertEqual(pack([1,2]+[0]*14), 513)

    def test_reject_bad_inputs(self):
        for a in ([0]*8, [128]*16, [True]*16):
            with self.assertRaises(ValueError): dot8(a, [0]*16)
        with self.assertRaises(ValueError): encode(32,1,2)

if __name__ == '__main__': unittest.main()
