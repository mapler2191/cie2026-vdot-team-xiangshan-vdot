"""Run oracle tests and actual RTL simulation. Fail if tools/tests are missing."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
from datetime import datetime, timezone

ROOT=Path(__file__).resolve().parents[1]
os.chdir(ROOT)
OUT=ROOT/'results'
OUT.mkdir(exist_ok=True)
suite=ROOT/'tools/oss-cad-suite'
env=os.environ.copy()
env['PATH']=os.pathsep.join([str(suite/'bin'),str(suite/'lib'),env.get('PATH','')])
env['PYTHONUTF8']='1'

def run(args,name):
    print('Running:', ' '.join(str(x) for x in args), flush=True)
    p=subprocess.run([str(x) for x in args],env=env,text=True,encoding='utf-8',errors='replace',
                     stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=240)
    output=p.stdout+p.stderr
    (OUT/name).write_text(output,encoding='utf-8')
    print(output[-1500:])
    if p.returncode: raise SystemExit(f'{name}: process exited with {p.returncode}')
    return output

if __name__=='__main__':
    # Invalidate stale success before starting a new run.
    report=OUT/'unit-test-report.json'
    report.write_text(json.dumps({'status':'running'}),encoding='utf-8')
    run([sys.executable,'-m','unittest','discover','-s','tests','-v'],'python-tests.log')
    run([sys.executable,'tools/generate_vectors.py'],'generation.log')
    iv=shutil.which('iverilog.exe' if os.name=='nt' else 'iverilog',path=env['PATH'])
    vvp=shutil.which('vvp.exe' if os.name=='nt' else 'vvp',path=env['PATH'])
    if not iv or not vvp: raise SystemExit('Install Icarus Verilog; RTL tests were NOT run.')
    version=run([iv,'-V'],'iverilog-version.log').splitlines()[0]
    run([iv,'-g2012','-Wall','-s','tb_vdot8','-o','results/vdot8.vvp',
         'rtl/vdot8.sv','tests/tb_vdot8.sv'],'rtl-compile.log')
    output=run([vvp,'results/vdot8.vvp'],'rtl-test.log')
    if 'PASS vectors=69656' not in output: raise SystemExit('Missing completion marker')
    report.write_text(json.dumps({'status':'pass','scope':'standalone RTL, NOT XiangShan difftest',
        'timestamp_utc':datetime.now(timezone.utc).isoformat(),'simulator':version,
        'oracle_vectors':69656,'checked_outputs':69657,'cancelled_transactions':4,
        'seed':20261023,'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in
        [Path('rtl/vdot8.sv'),Path('tests/tb_vdot8.sv'),Path('results/vectors.hex')]}},indent=2),encoding='utf-8')
