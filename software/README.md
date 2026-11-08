# 裸机测试程序

全部用 `riscv64-linux-gnu-` 交叉编译，链接脚本 `link.ld` 把程序放在 `0x80000000`，
启动代码 `boot.S`（或 `boot_a0exit.S`）负责设栈、清 `.bss`、调 `main`，最后由 trap 指令结束仿真。

## 入口与退出码

| 入口文件 | trap 字 | 用途 |
|---|---|---|
| `boot.S` | `.word 0x0000006b`（rs1=x0） | 历史版本。XiangShan 把 trap code 取成编码里的 rs1，所以 code 恒为 0，`HIT GOOD TRAP` 只代表"执行了一条 trap 指令" |
| `boot_a0exit.S` | `.word 0x0005006b`（rs1=a0） | a0 版。trap code 等于 a0，进程退出码等于程序的退出状态：0 = 全部断言通过 |

`test_util.h` 里的 `test_exit()` / `test_finish()` 也直接用 a0 形式的 trap 字，
所以新写的测试不依赖 boot 也能给出可信退出码。

## 文件

| 文件 | 内容 |
|---|---|
| `boot.S` / `boot_a0exit.S` | 两个启动入口（见上表） |
| `kernels.S` | 五个内核：`vdot_custom`、`vdot_baseline`（标准 RVV 宽乘+宽归约）、`vdot_alias1/2`、`vdot_v0`，以及 `vadd_demo` |
| `main.c` | 1,030 组输入 × 5 种实现的功能回归（含极值、随机、别名、v0） |
| `perf_main.c` | 256 次完整内核调用的性能对照（`make perf` / `make perf-reverse`） |
| `vdot_basic.c` | 三类基础功能测试：正数、负数、边界（UART 打印，5–10 秒） |
| `exception_check.S` | 三次 trap 的异常恢复测试（VL=0、标准非法、再次 VL=0） |
| `exception_cases.S` | 8 种非法配置的故障指令与辅助函数（配合 `mtval_probe.c`） |
| `mtval_probe.c` | 8 种非法配置逐例打印 mcause/mepc/mtval |
| `mtval_check.S` | 单用例判定器：用两个期望值夹逼出实测 mtval（无 UART、纯汇编） |
| `highbits_v2.c` + `highbits_kernel.S` | 高位独立性：VL=16 全宽加载并回存证明，再 VL=8 执行 |
| `backtoback_vdot.c` + `backtoback_kernel.S` | 三条紧邻 vdot、不同目的寄存器 |
| `exit_probe.c` | 证明 a0 编码 trap 字能区分 GOOD/BAD TRAP |
| `vdot_wave_main.c` | 单组 vdot（采波形用） |
| `vadd_main.c` + `vadd_only.S` | 40 组标准 vadd 回归与单组波形程序 |
| `hello_main.c` + `hello_identity.h` | 第一阶段 hello 输出（团队名 + VM IP） |
| `reference.py` | 参考模型 |
| `Makefile` | 全部目标，见下 |

## 常用目标

```bash
make all            # vdot.bin 等历史程序
make hello vadd vadd-wave exception-check perf perf-reverse vdot-wave
make evidence       # 例外矩阵 / mtval probe / 三类基础测试 / 高位 / 背靠背 / a0 版矩阵
make a0-check       # 三支 a0 入口：vdot-a0 / vadd-a0 / exception-check-a0
```

`make a0-check` 出来的 `build/vdot-a0.bin` 等，是该仓库里**唯一能写"退出码 0"**的回归二进制。
