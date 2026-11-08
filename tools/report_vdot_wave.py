import json, bisect, hashlib
from pathlib import Path
root = Path(__file__).resolve().parents[1]
out = root / 'results/vdot-wave'
j = json.loads((out/'window.json').read_text())
ns = {n:c for c,nn in j['names'].items() for n in nn}
width = {n:w for n,c,w in json.loads((out/'signals.json').read_text())}
base = 'TOP.SimTop.l_soc.core_with_l2.core.backend.'
checked = []
def check(path,t,expected,mask=None):
    n=base+path; e=j['events'][ns[n]]
    i=bisect.bisect_right([x[0] for x in e],t)-1
    assert i>=0
    actual=int(e[i][1],2)
    assert (actual if mask is None else actual & mask)==expected,(n,t,actual,expected)
    checked.append(dict(signal=n,sample=t,actual=hex(actual),mask=mask))
for port,t in [('in',2352),('out',2353)]:
    p='inner_ctrlBlock.decodePipeRenameModule.io_'+port+'_'
    for s,x in [('valid',1),('ready',1),('bits_pc',0x80000044),('bits_instr',0x0220a18b)]:check(p+s,t,x)
p='inner_ctrlBlock.dispatch.io_fromRename_0_'
for s,x in [('valid',1),('ready',1),('bits_pc',0x80000044),('bits_robIdx_value',27),('bits_robIdx_flag',0),('bits_pdest',51)]:check(p+s,2354,x)
p='inner_vfScheduler.IssueQueueVfmaVialuFixVimacVppuVfaluVfcvtVipuVsetrvfwvf.io_deqDelay_0_'
for s,x in [('valid',1),('ready',1),('bits_common_robIdx_value',27),('bits_common_robIdx_flag',0),('bits_common_pdest',51)]:check(p+s,2387,x)
p='inner_vfExuBlock.exus.Vimac.'
for s,x in [('io_in_valid',1),('io_in_ready',1),('io_in_bits_ctrl_fuOpType',256),('dot.io_enable1',1)]:check(p+s,2390,x)
check(p+'dot.io_vs1',2390,0x0706050403020100,(1<<64)-1)
check(p+'dot.io_vs2',2390,0xf8f9fafbfcfdfeff,(1<<64)-1)
check(p+'dot.io_enable2',2391,1)
for i in range(8):check(p+f'dot.products_{i}',2391,(-i*(i+1))&65535)
for s,x in [('io_out_valid',1),('io_out_ready',1),('io_out_bits_ctrl_robIdx_value',27),('io_out_bits_ctrl_robIdx_flag',0),('io_out_bits_ctrl_pdest',51),('io_out_bits_ctrl_vecWen',1),('io_out_bits_res_data',0xffffff58),('dot.io_vd',0xffffff58)]:check(p+s,2392,x)
p='inner_ctrlBlock.rob.io_writeback_13_'
check(p+'valid',2393,1)
# Obtain exact flattened tag path from the declaration rather than guessing it.
for n in ns:
    if n.startswith(base+p) and n.endswith('robIdx_value'):check(n[len(base):],2393,27)
    if n.startswith(base+p) and n.endswith('robIdx_flag'):check(n[len(base):],2393,0)
p='inner_ctrlBlock.rob.io_commits_'
for s,x in [('isCommit',1),('commitValid_0',1),('info_0_debug_pc',0x80000044)]:check(p+s,2394,x)
sha=hashlib.sha256((out/'vdot-single.fst').read_bytes()).hexdigest()
(out/'evidence.json').write_text(json.dumps(dict(fst_sha256=sha,checks=checked),indent=2))
lines=['$comment Timestamps are emulator cycle samples, not physical ps. $end','$timescale 1ps $end','$scope module vdot_evidence $end']; changes={}
for i,n in enumerate(dict.fromkeys(c['signal'] for c in checked)):
    code='s'+str(i); lines.append(f'$var wire {width[n]} {code} {n[len(base):].replace(".","__")} $end')
    e=j['events'][ns[n]]; k=bisect.bisect_right([x[0] for x in e],2348)-1
    if k>=0:changes.setdefault(2348,[]).append(f'b{e[k][1]} {code}')
    for t,b in e:
        if 2348<t<=2400:changes.setdefault(t,[]).append(f'b{b} {code}')
lines+=['$upscope $end','$enddefinitions $end']
for t in sorted(changes):lines += [f'#{t}']+changes[t]
(out/'vdot-focused.vcd').write_text('\n'.join(lines)+'\n')
print('PASS',len(checked),'assertions; FST SHA256',sha)
