"""Reproducible oracle vectors; these are not RTL execution results."""
import json
import random
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from software.reference import dot8, pack

def generate():
    rng = random.Random(20261023)
    cases = []
    def add(name, a, b): cases.append((name, a, b, dot8(a,b)))
    add('positive', list(range(1,17)), [2]*16)
    add('negative', [-i for i in range(1,17)], [2]*16)
    add('zero', [0]*16, [-128]*16)
    add('max_positive', [-128]*16, [-128]*16)
    add('max_negative', [-128]*16, [127]*16)
    add('max_int8', [127]*16, [127]*16)
    add('cancellation', [127,-127]*8, [127]*16)
    add('ignored_upper', [0]*8+[-128]*8, [0]*8+[-128]*8)
    for lane in range(16):
        a=[0]*16; a[lane]=-128
        add(f'lane_{lane}',a,[-128]*16)
    for i in range(4096):
        add(f'random_{i}',[rng.randrange(-128,128) for _ in range(16)],
            [rng.randrange(-128,128) for _ in range(16)])
    # Exhaust all byte pairs on lane zero; arithmetic and sign boundaries.
    for a in range(-128,128):
        for b in range(-128,128):
            add(f'pair_{a}_{b}',[a]+[0]*15,[b]+[0]*15)
    out=Path('results');out.mkdir(exist_ok=True)
    (out/'vectors.hex').write_text(''.join(
        f'{pack(a):032x} {pack(b):032x} {r & 0xffffffff:08x}\n' for _,a,b,r in cases),encoding='ascii')
    (out/'vectors.json').write_text(json.dumps({
        'seed':20261023,'count':len(cases),'directed':[
            {'name':n,'a':a,'b':b,'expected':r} for n,a,b,r in cases[:24]]},indent=2),encoding='utf-8')
    print(f'Generated {len(cases)} vectors')

if __name__=='__main__': generate()
