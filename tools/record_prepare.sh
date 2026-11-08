#!/usr/bin/env bash
# 录屏前自检：在 VM 里 ~/risv/project/software 下运行。
#
#   bash tools/record_prepare.sh          # 快速自检（约 1 分钟，跑 hello / vdot-basic / a0 异常 / exit-probe）
#   bash tools/record_prepare.sh --full   # 全部自检（再加 vadd-a0 / highbits / backtoback / mtval-probe，约 6 分钟）
#
# 最后一行打印 READY 或 NOT READY。全绿再开录，别边录边修。
set -u

FULL=0
[ "${1:-}" = "--full" ] && FULL=1

EMU=${EMU:-$HOME/risv/xs-env/XiangShan/build/emu}
REF=${REF:-$HOME/risv/xs-env/NEMU/build/riscv64-nemu-interpreter-so}
FLASH=${FLASH:-$HOME/risv/artifacts/boot-minimal.bin}
DOCS=${DOCS:-$HOME/risv/docs}
RES=${RES:-$HOME/risv/results}

FAIL=0
ok()   { printf '  [ OK ]  %s\n' "$1"; }
bad()  { printf '  [FAIL]  %s\n' "$1"; FAIL=$((FAIL+1)); }
warn() { printf '  [WARN]  %s\n' "$1"; }
sec()  { printf '\n== %s ==\n' "$1"; }

# ---------------------------------------------------------------- 1. 三件套
sec "1. 仿真器 / 参考库 / Flash"
for f in "$EMU" "$REF" "$FLASH"; do
    if [ -f "$f" ]; then ok "存在 $f"; else bad "缺少 $f"; fi
done
if [ -f "$EMU" ]; then
    h=$(sha256sum "$EMU" | cut -c1-16)
    case "$h" in
        755c80de1eebdd3f) ok "emu 哈希 $h（与提交版一致）" ;;
        *) warn "emu 哈希 $h，与提交版 755c80de1eebdd3f 不同：确认是否重编过" ;;
    esac
fi
if [ -f "$REF" ]; then
    h=$(sha256sum "$REF" | cut -c1-16)
    case "$h" in
        740e921394525629) ok "参考库哈希 $h（mtval 修复后的交付库）" ;;
        69249b1524ddde67) warn "参考库哈希 $h 是调试构建（日志里会有 [vdot-ref]/[ex-enter]），录屏可用但要说明" ;;
        2019d71776cf3457) warn "参考库哈希 $h 是旧模型（mtval 未分阶段），A/B 对照会失败" ;;
        *) warn "参考库哈希 $h 不是提交版 740e921394525629" ;;
    esac
fi
if [ -f "$FLASH" ]; then
    h=$(sha256sum "$FLASH" | cut -c1-16)
    [ "$h" = "fea548655a8b6eec" ] && ok "flash 哈希 $h" || bad "flash 哈希 $h ≠ fea548655a8b6eec"
fi

# ---------------------------------------------------------------- 2. 源码
sec "2. 源码完整性"
NEED="test_util.h boot_a0exit.S exception_cases.S exception_matrix_v2.c mtval_probe.c mtval_check.S
      highbits_kernel.S highbits_v2.c backtoback_kernel.S backtoback_vdot.c exit_probe.c
      vdot_basic.c hello_main.c hello_identity.h kernels.S main.c perf_main.c
      exception_check.S vadd_main.c vadd_only.S vdot_wave_main.c link.ld Makefile"
for f in $NEED; do
    [ -f "$f" ] || bad "缺少源码 $f（从 Windows 的 D:\\RISV\\software 复制过来）"
done
[ -f Makefile ] && grep -q '^a0-check:' Makefile && ok "Makefile 含 a0-check 目标" || bad "Makefile 不是新版（缺 a0-check/evidence 目标）"
grep -q 'TRAP_FAIL_BAD' exception_check.S 2>/dev/null && ok "exception_check.S 含 -DTRAP_FAIL_BAD 分支" || bad "exception_check.S 是旧版"
if grep -q 'YOUR_TEAM\|YOUR_VM_IP' hello_identity.h 2>/dev/null; then
    bad "hello_identity.h 还是占位符（YOUR_TEAM / YOUR_VM_IP）"
else
    ok "hello_identity.h 已填：$(grep HELLO_TEAM hello_identity.h | cut -d'"' -f2) / $(grep HELLO_VM_IP hello_identity.h | cut -d'"' -f2)"
fi

# ---------------------------------------------------------------- 3. 文档与结果
sec "3. 文档与归档结果（录屏要切屏看）"
for f in RECORDING-SCRIPT.md vdot-design-doc.md vadd-stage1-wave-report.md vdot-wave-report.md \
         latency-comparison-report.md kunminghu-v2-performance.md; do
    [ -f "$DOCS/$f" ] && ok "docs/$f" || bad "缺少 $DOCS/$f（整个 docs 目录复制进来）"
done
for f in t5b-clean-ref/vdot-a0.log t5b-clean-ref/vadd-a0.log t5b-clean-ref/exception-check-a0.log \
         t2-mtval-probe/mtval-probe.log t3-highbits-backtoback-v2/highbits-v2.log \
         t3-highbits-backtoback-v2/backtoback.log \
         t5-nemu-fixed-regressions/sew16-diff-old-so.log t5-nemu-fixed-regressions/sew16-diff-new-so.log \
         latency-comparison/latency-evidence.json \
         kunminghu-v2-final/perf-baseline-first.log kunminghu-v2-final/perf-custom-first.log; do
    [ -f "$RES/$f" ] && ok "results/$f" || bad "缺少 $RES/$f"
done
if [ -f "$RES/t5b-clean-ref/vdot-a0.log" ]; then
    grep -q 'instrCnt = 841,260' "$RES/t5b-clean-ref/vdot-a0.log" && ok "vdot-a0.log 含 841,260" || bad "vdot-a0.log 里没有 841,260"
fi

# ---------------------------------------------------------------- 4. 工具
sec "4. 波形工具"
command -v gtkwave >/dev/null 2>&1 && ok "gtkwave $(gtkwave --version 2>/dev/null | head -1)" \
    || bad "没装 gtkwave（sudo apt install -y gtkwave）"
for f in "$RES/latency-comparison/vdot-latency-focused.vcd" \
         "$RES/latency-comparison/baseline-latency-focused.vcd" \
         "$RES/vadd-focused.vcd" "$RES/vdot-wave/vdot-focused.vcd"; do
    [ -f "$f" ] && ok "VCD $(basename "$f")" || warn "缺少 VCD $f（视频一要展示）"
done

# ---------------------------------------------------------------- 5. 编译
sec "5. 编译"
if [ -f Makefile ]; then
    make evidence > /tmp/prep-make-evidence.log 2>&1 && ok "make evidence" || { bad "make evidence 失败，见 /tmp/prep-make-evidence.log"; tail -5 /tmp/prep-make-evidence.log; }
    make a0-check > /tmp/prep-make-a0.log 2>&1 && ok "make a0-check" || { bad "make a0-check 失败，见 /tmp/prep-make-a0.log"; tail -5 /tmp/prep-make-a0.log; }
    for b in hello vdot-basic exception-check-a0 vdot-a0 vadd-a0 highbits-v2 backtoback exitprobe-ok exitprobe-fail mtval-probe; do
        [ -f "build/$b.bin" ] && ok "build/$b.bin" || bad "缺少 build/$b.bin"
    done
fi

# ---------------------------------------------------------------- 6. 实跑冒烟
run() {   # $1=bin  $2=grep 模式  $3=描述
    local bin="$1" pat="$2" desc="$3" log="/tmp/prep-$1.log"
    timeout 300 "$EMU" -i "build/$bin.bin" --diff "$REF" -F "$FLASH" > "$log" 2>&1
    local rc=$?
    if grep -qE "$pat" "$log"; then
        ok "$desc（rc=$rc）"
    else
        bad "$desc：日志里没找到 /$pat/，见 $log"
        grep -E 'HIT (GOOD|BAD) TRAP|mismatch' "$log" | head -3 | sed 's/^/          /'
    fi
}

sec "6. 上镜命令实跑（快）"
run hello            'hello xiangshan, I am'                       "hello 输出团队名与 IP"
run vdot-basic       '\[SUMMARY\] failures=0'                      "三类基础功能测试（正/负/边界）"
run exception-check-a0 'instrCnt = 274'                            "a0 版异常回归（三次 trap）"
run exitprobe-ok     'HIT GOOD TRAP'                               "exit-probe a0=0 → GOOD"
run exitprobe-fail   'HIT BAD TRAP'                                "exit-probe a0=1 → BAD"

if [ "$FULL" = "1" ]; then
    sec "7. 上镜命令实跑（慢，--full）"
    run vadd-a0        'instrCnt = 23,876'                         "a0 版 vadd 回归"
    run highbits-v2    '\[RESULT\] PASS'                           "高位独立性（4 组图案）"
    run backtoback     '\[RESULT\] PASS'                           "三条背靠背 vdot"
    timeout 400 "$EMU" -i build/mtval-probe.bin -F "$FLASH" --no-diff > /tmp/prep-mtval.log 2>&1
    grep -q '\[PROBE\] done traps=8' /tmp/prep-mtval.log && ok "mtval probe 8 例全跑（--no-diff）" \
        || bad "mtval probe 没跑满 8 例，见 /tmp/prep-mtval.log"
else
    sec "7. 慢速项（未跑，加 --full 才测）"
    warn "vadd-a0 / highbits-v2 / backtoback / mtval-probe 跳过"
fi

# ---------------------------------------------------------------- 8. 上台清单
cat <<'EOF'

== 上台命令清单（照抄，别现场敲） ==
# 环境
cd ~/risv/project/software
export EMU=~/risv/xs-env/XiangShan/build/emu
export REF=~/risv/xs-env/NEMU/build/riscv64-nemu-interpreter-so
export FLASH=~/risv/artifacts/boot-minimal.bin

# 视频一
$EMU -i build/hello.bin --diff $REF -F $FLASH ; echo "exit code = $?"
$EMU -i build/vadd-a0.bin --diff $REF -F $FLASH ; echo "exit code = $?"
gtkwave ~/risv/results/vadd-focused.vcd &

# 视频二
$EMU -i build/vdot-basic.bin --diff $REF -F $FLASH ; echo "exit code = $?"
tail -5 ~/risv/results/t5b-clean-ref/vdot-a0.log

# 视频三
$EMU -i build/exception-check-a0.bin --diff $REF -F $FLASH ; echo "exit code = $?"
grep -E '^\[RESULT|^\[PROBE\] done' ~/risv/results/t2-mtval-probe/mtval-probe.log
grep -E 'mtval different' ~/risv/results/t5-nemu-fixed-regressions/sew16-diff-old-so.log
grep -E 'PERF_V1' ~/risv/results/kunminghu-v2-final/perf-*.log
gtkwave ~/risv/results/latency-comparison/vdot-latency-focused.vcd &
EOF

printf '\n================ 结果 ================\n'
if [ "$FAIL" -eq 0 ]; then
    echo "READY：$FAIL 个失败项。可以开录。"
    exit 0
else
    echo "NOT READY：$FAIL 个失败项，先修完再录（失败项见上面 [FAIL] 行）。"
    exit 1
fi
