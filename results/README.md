# results/ —— 最终证据目录

这里的日志和 JSON 是**最终口径**的证据：a0 编码 trap 字（`.word 0x0005006b`）的回归、
8 种非法配置的 mtval 实测、NEMU/RTL 的 A/B 对照、计算路径波形证据、参数化模块验证。

文档（`docs/01`–`docs/07`）里引用的 `results/...` 路径，指的就是本目录下的相对路径。

| 子目录 | 内容 | 对应结论 |
|---|---|---|
| `t5b-clean-ref/` | 交付库版本的三支 a0 回归（vdot / vadd / exception-check）+ 跑前跑后哈希 | 841,260/790,553；23,876/16,870；274/2,865；退出码 0 |
| `t2-mtval-probe/` | 8 种非法配置逐例打印的 mcause/mepc/mtval | mtval 分两类：VL→0，SEW/LMUL/vstart→0x0220a18b，标准非法→0x7b |
| `t3-highbits-backtoback-v2/` | 高位独立性（4 组高半图案）、三条紧邻 vdot | 程序自打印 PASS、exit 0 |
| `t5-nemu-fixed-regressions/` | NEMU mtval 修复前后的 A/B 对照 + 补丁 + A/B 二进制 | 修复前 `mtval different (right=0, wrong=0x220a18b)`，修复后无 mismatch |
| `latency-comparison/` | 208 项断言的波形对照证据 + 两份精简 VCD | 计算路径 2 周期 vs 24 周期，区间比 12×（MinimalConfig） |
| `parametric-verification/` | n=4/8/16 生成模块的 iverilog 验证与变异测试 | 471/479/449 个输出全过，3 处变异全部被抓 |
| `kunminghu-v2-final/` | 正式配置的性能日志与版本信息 | 7,881/5,141 与 7,882/5,119 |

**与 `evidence/` 的区别**：`evidence/` 是早期归档（那时用 `.word 0x0000006b`，trap code 恒 0，
只能说明"运行至结束、无 mismatch"）；`results/` 是引入 a0 编码之后的证据，可以写"退出码 0"。
两套都保留，历史可追溯。
