"""Verify extracted real FST samples and export a small, renamed VCD."""
import bisect
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
j = json.loads((ROOT / 'results/vadd-window.json').read_text())
names = {n: c for c, nn in j['names'].items() for n in nn}
widths = {n: w for n, c, w in json.loads((ROOT / 'results/wave-signals.json').read_text())}
base = 'TOP.SimTop.l_soc.core_with_l2.core.backend.'
signals = {}
checks = []

def check(label, path, time, expected):
    name = base + path
    events = j['events'][names[name]]
    idx = bisect.bisect_right([e[0] for e in events], time) - 1
    assert idx >= 0, name
    actual = int(events[idx][1], 2)
    assert actual == expected, (name, time, actual, expected)
    signals[label] = name
    checks.append(dict(signal=name, sample=time, value=hex(actual)))

d = 'inner_ctrlBlock.decodePipeRenameModule.'
for port, t in [('in', 2514), ('out', 2515)]:
    for s, v in [('valid', 1), ('ready', 1), ('bits_pc', 0x80000044), ('bits_instr', 0x021101d7)]:
        check('decode_' + port + '_' + s, d + 'io_' + port + '_' + s, t, v)
d = 'inner_ctrlBlock.dispatch.io_fromRename_0_'
for s, v in [('valid', 1), ('ready', 1), ('bits_pc', 0x80000044), ('bits_robIdx_flag', 1), ('bits_robIdx_value', 33), ('bits_pdest', 51)]:
    check('dispatch_' + s, d + s, 2516, v)
d = 'inner_vfScheduler.IssueQueueVfmaVialuFixVfalu.io_deqDelay_0_'
for s, v in [('valid', 1), ('ready', 1), ('bits_common_robIdx_flag', 1), ('bits_common_robIdx_value', 33), ('bits_common_pdest', 51)]:
    check('issue_' + s, d + s, 2538, v)
d = 'inner_vfExuBlock.exus_2.VialuFix.'
for s, v in [('io_in_valid', 1), ('io_in_ready', 1), ('io_in_bits_data_src_0', int.from_bytes(bytes(2*i for i in range(16)), 'little')), ('io_in_bits_data_src_1', int.from_bytes(bytes(range(16)), 'little'))]:
    check('alu_' + s, d + s, 2541, v)
for s, v in [('io_out_valid', 1), ('io_out_ready', 1), ('io_out_bits_ctrl_robIdx_flag', 1), ('io_out_bits_ctrl_robIdx_value', 33), ('io_out_bits_ctrl_pdest', 51), ('io_out_bits_ctrl_vecWen', 1), ('io_out_bits_res_data', int.from_bytes(bytes(3*i for i in range(16)), 'little'))]:
    check('alu_' + s, d + s, 2542, v)
d = 'inner_ctrlBlock.rob.io_commits_'
for s, v in [('isCommit', 1), ('commitValid_0', 1), ('info_0_debug_pc', 0x80000044)]:
    check('commit_' + s, d + s, 2544, v)
check('commit_commitValid_0', d + 'commitValid_0', 2543, 0)

sha = hashlib.sha256((ROOT / 'results/vadd-stage1.fst').read_bytes()).hexdigest()
(ROOT / 'results/vadd-wave-evidence.json').write_text(json.dumps(dict(fst_sha256=sha, checks=checks, signals=signals), indent=2), encoding='utf-8')
lines = ['$comment Derived from vadd-stage1.fst; timestamps are simulation cycle samples, NOT physical picoseconds. $end', '$timescale 1ps $end', '$scope module vadd_evidence $end']
changes = {}
for i, (label, name) in enumerate(signals.items()):
    code = 's' + str(i)
    lines.append(f'$var wire {widths[name]} {code} {label} $end')
    for t, value in j['events'][names[name]]:
        changes.setdefault(t, []).append(f'b{value} {code}')
lines += ['$upscope $end', '$enddefinitions $end']
for t in sorted(changes):
    lines.append(f'#{t}')
    lines.extend(changes[t])
(ROOT / 'results/vadd-focused.vcd').write_text('\n'.join(lines) + '\n', encoding='ascii')
print(f'PASS: {len(checks)} sample assertions; {len(signals)} exported signals; SHA256={sha}')
