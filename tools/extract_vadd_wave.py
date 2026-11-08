"""Stream FST through fst2vcd; retain selected signals and a short time window."""
import json, os, re, subprocess
from pathlib import Path

root=Path(__file__).resolve().parents[1]
os.chdir(root)
env=os.environ.copy()
env['PATH']=str(root/'tools/oss-cad-suite/lib')+os.pathsep+env['PATH']
proc=subprocess.Popen([str(root/'tools/oss-cad-suite/bin/fst2vcd.exe'),
                       'results/vadd-stage1.fst'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=env)
scope=[]; declarations=[]; names={}; selected=set(); current={}; events={}; t=0
pattern=re.compile(r'Vialu|vialu|decode\.io_|decodePipeRename|dispatch\.io_fromRename|rob\.io_commits|rob\.robEntries_33|rob\.io_writeback|rob\.io_enq|rob\.difftest_delayer|^TOP\.SimTop\.clock$|trap.*cycleCnt',re.I)
header=True; meta=[]
for raw in proc.stdout:
    line=raw.decode('ascii').strip()
    if header:
        words=line.split()
        if line.startswith('$scope'):scope.append(words[2])
        elif line.startswith('$upscope'):scope.pop()
        elif line.startswith('$var'):
            _,kind,width,code,ref,*rest=words
            name='.'.join(scope+[ref])
            declarations.append([name,code,int(width)])
            if pattern.search(name):
                names.setdefault(code,[]).append(name);selected.add(code)
        elif line.startswith('$enddefinitions'):
            header=False
            Path('results/wave-signals.json').write_text(json.dumps(declarations),encoding='utf-8')
            print('Signals',len(declarations),'selected IDs',len(selected),flush=True)
        elif line:meta.append(line)
        continue
    if not line:continue
    if line[0]=='#':
        nt=int(line[1:])
        if t<2480<=nt:
            events={c:[[2480,v]] for c,v in current.items()}
        t=nt
    elif line[0] in 'bBrR':
        value,code=line[1:].split()
        if code in selected:
            current[code]=value
            if 2480<=t<=2565:events.setdefault(code,[]).append([t,value])
    elif line[0] in '01xzXZ':
        value,code=line[0],line[1:]
        if code in selected:
            current[code]=value
            if 2480<=t<=2565:events.setdefault(code,[]).append([t,value])
error=proc.stderr.read().decode(errors='replace')
if proc.wait()!=0:raise RuntimeError(error)
Path('results/vadd-window.json').write_text(json.dumps({'meta':meta,'names':names,'events':events}),encoding='utf-8')
print('Window saved. Final timestamp',t,flush=True)
