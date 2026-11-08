# 复现指南

从零到三项 a0 回归、波形和性能的最短路径。路径按 Ubuntu 虚拟机书写；把 `$HOME/risv` 换成你的目录。

## 1. 固定版本

| 组件 | 版本 |
|---|---|
| XiangShan | `16ae9ddcda54fc9a2fddffad73174cf793ac7814` |
| NEMU | `a48a11f208ff81ad765591d83271d1b339fb3861` |
| 配置 | `KunminghuV2MinimalConfig` |
| boot flash | 16 B，SHA256 `fea548655a8b6eecacace83e2f4380a18b8aa83bce964c8aae4cffbe2f5a9724` |

## 2. 环境变量

```bash
export JAVA_HOME=/opt/graalvm-jdk-21
export PATH=$JAVA_HOME/bin:/usr/local/llvm/bin:$PATH
export NOOP_HOME=$HOME/risv/xs-env/XiangShan
export NEMU_HOME=$HOME/risv/xs-env/NEMU
```

> `NOOP_HOME` 是否就是早期负位宽错误的唯一根因，我们没有做对照实验，只能说"设置后能过"。

## 3. 应用补丁

```bash
cd $NOOP_HOME
git apply /path/to/rtl/xiangshan/xiangshan-vdot-complete.patch

cd $NEMU_HOME
git apply /path/to/nemu/nemu-tval.patch
```

**NEMU 补丁要用 `nemu-tval.patch`**：它包含 mtval 分阶段修复（译码阶段保留指令字，
执行阶段 VL 不匹配才置 0）。`nemu-vdot-clean.patch` / `nemu-vdot-validated.patch` 是早期版本，
**不含**这个修复，只能作为过程版本；用它们复现会得到旧模型，SEW=16 会报 mtval mismatch。

## 4. 构建 NEMU

```bash
cd $NEMU_HOME
make riscv64-xs-ref_defconfig
make -j$(nproc)
ls -l build/riscv64-nemu-interpreter-so
```

确认 `.config`：

```text
CONFIG_RVV=y
CONFIG_RV_SMRNMI=y
CONFIG_NMIE_INIT=y
# CONFIG_MDT_INIT is not set
CONFIG_TVAL_EX_II=y
```

## 5. 构建香山

```bash
cd $NOOP_HOME
make emu CONFIG=KunminghuV2MinimalConfig JVM_XMX=12G EMU_THREADS=2 -j2
sha256sum build/emu
sha256sum $NEMU_HOME/build/riscv64-nemu-interpreter-so
sha256sum $HOME/risv/artifacts/boot-minimal.bin
```

期望与 `evidence/reports/version-info.txt` 一致（emu `755c80de…`、flash `fea54865…`；
参考库在 mtval 修复后是 `740e9213…`）。

## 6. 编译软件

```bash
cd software
make all hello vadd exception-check perf perf-reverse vdot-wave
make evidence          # 新增证据测试：mtval probe、三类基础测试、高位、背靠背
make a0-check          # 三支 a0 入口
```

## 7. a0 版三项回归（推荐）

```bash
cd $NOOP_HOME
for t in vdot-a0 vadd-a0 exception-check-a0; do
  echo "=== $t ==="
  ./build/emu -i $HOME/risv/project/software/build/$t.bin \
    --diff $NEMU_HOME/build/riscv64-nemu-interpreter-so \
    -F $HOME/risv/artifacts/boot-minimal.bin
  echo "exit code = $?"
done
```

期望（与 `results/t5b-clean-ref/*.log` 一致）：

```text
vdot-a0            : HIT GOOD TRAP at pc = 0x80000032，instrCnt = 841,260，cycleCnt = 790,553，exit 0
vadd-a0            : HIT GOOD TRAP at pc = 0x80000032，instrCnt =  23,876，cycleCnt =  16,870，exit 0
exception-check-a0 : HIT GOOD TRAP at pc = 0x80000032，instrCnt =     274，cycleCnt =   2,865，exit 0
```

这是唯一能写"退出码 0"的回归；历史 `vdot.bin`（x0 trap 字）只能说明"运行至结束、无 mismatch"。

## 8. 历史三项回归（x0 trap 字）

```bash
for t in vdot vadd exception-check hello; do
  ./build/emu -i $HOME/risv/project/software/build/$t.bin \
    --diff $NEMU_HOME/build/riscv64-nemu-interpreter-so \
    -F $HOME/risv/artifacts/boot-minimal.bin
done
```

数字与上面相同；hello 会打印 `hello xiangshan, I am <团队名>, IP address <VM IP>`。

## 9. 性能

```bash
./build/emu -i $HOME/risv/project/software/build/perf.bin         --diff $NEMU_HOME/build/riscv64-nemu-interpreter-so -F $HOME/risv/artifacts/boot-minimal.bin
./build/emu -i $HOME/risv/project/software/build/perf-reverse.bin --diff $NEMU_HOME/build/riscv64-nemu-interpreter-so -F $HOME/risv/artifacts/boot-minimal.bin
```

期望：

```text
PERF_ORDER baseline-first
PERF_V1 iterations=256 baseline_cycles=7881 custom_cycles=5141

PERF_ORDER custom-first
PERF_V1 iterations=256 baseline_cycles=7882 custom_cycles=5119
```

## 10. 8 种非法配置的 mtval

```bash
make evidence
./build/emu -i $HOME/risv/project/software/build/mtval-probe.bin -F $HOME/risv/artifacts/boot-minimal.bin --no-diff
```

期望 8 行 `[RESULT n]`：VL=0/1/7/9 的 `mtval=0x0`；SEW=16/LMUL=2/vstart=1 的 `mtval=0x220a18b`；
标准非法 `0x7b`。结尾 `[PROBE] done traps=8`。这次是 `--no-diff`，测的是 DUT 本身。

## 11. 单元测试

```bash
python3 tools/run_tests.py
```

期望 `PASS vectors=69656 checked=69657 cancelled=4 cycles=108440`。
需要 Icarus Verilog 14.0。仓库不包含 600 MB 的 OSS CAD Suite；把 OSS CAD Suite 解压到
`tools/oss-cad-suite`，或确保 `iverilog`/`vvp` 在 PATH 中。

## 12. 波形

```bash
cd software && make vadd-wave
cd $NOOP_HOME
./build/emu-fst -i $HOME/risv/project/software/build/vadd-wave.bin \
  --diff $NEMU_HOME/build/riscv64-nemu-interpreter-so \
  -F $HOME/risv/artifacts/boot-minimal.bin --dump-wave
```

`emu-fst` 需要在 `difftest/verilator.mk` 中开 `EMU_TRACE=fst` 独立构建，避免覆盖普通 `emu`。
vdot 同理（`make vdot-wave`）。离线分析：

```bash
python3 tools/extract_vadd_wave.py && python3 tools/report_vadd_wave.py
python3 tools/extract_vdot_wave.py && python3 tools/report_vdot_wave.py
```

精简 VCD 在 `evidence/waves/*-focused.vcd` 与 `results/latency-comparison/*-focused.vcd`。

## 13. 常见问题

| 现象 | 处理 |
|---|---|
| 第 2 条指令 mismatch | 两边显式 `-F` 同一个 16 B flash |
| 异常 `mcause/mepc=0` | 确认 `CONFIG_NMIE_INIT=y`、`MDT_INIT` 关闭 |
| `mtval` 不一致 | 确认用了 `nemu-tval.patch`；vdot 的 VL 类应为 0，SEW/LMUL/vstart 类应为指令字 |
| `NoSuchElementException: NOOP_HOME` | 显式 `export NOOP_HOME` |
| FST 构建失败 / VM 关机 | 保留生成 RTL/C++，用 `OPT_FAST=-O0`、单任务续编 |
| 日志交错 | 以完整行 `GOOD TRAP`、`PERF_V1` 为准 |
