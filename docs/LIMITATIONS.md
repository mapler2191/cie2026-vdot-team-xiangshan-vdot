# 已知限制与边界

本文列出当前实现和证据能支持的范围。报告里凡是超出这里的说法，都需要补新证据。

## 1. 指令配置

只实现并验证：

- `VL=8`
- `SEW=8`
- `LMUL=1`
- `vstart=0`
- `vm=1`
- `VLEN=128`

非法配置分两类：

- VL 类（VL=0/1/7/9）：执行阶段发现，`mtval=0`；
- SEW/LMUL/vstart 类：译码阶段发现，`mtval=0x0220a18b`；
- 标准非法 `0x7b`：`mtval=0x7b`。

## 2. 异常覆盖

已测：

- M 模式 8 种非法配置，逐例打印 `mcause/mepc/mtval`；
- a0 版三次 trap：VL=0、标准非法、再次 VL=0，a0=0；
- 标准非法指令的 `mtval=0x7B` 没有被 vdot 的零 tval 污染；
- 恢复执行、`v3` 保持。

未覆盖：

- S/VS 模式异常委托；
- RNMI、NMIE=0 早退路径；
- `vm=0` 等其他非法配置；
- 重新初始化后的零 tval 标志状态；
- page fault、misaligned、断点等其他异常；
- 多核/多线程异常。

## 3. NEMU mtval

`nemu-tval.patch` 把条件拆成两段：

- 译码阶段：SEW/LMUL/vstart/vm 不匹配，保留指令字作为 mtval；
- 执行阶段：VL 不匹配，置 `vdot_illegal_no_tval`，mtval=0。

A/B 只覆盖 SEW=16 和 VL=0；LMUL=2、vstart=1 的 mtval 由 `mtval_probe` 逐例测量，还没有带 difftest 的 A/B。S/VS 委托、RNMI、NMIE=0 路径未覆盖。

## 4. 波形与性能

- vadd/vdot 波形：`MinimalConfig`；
- 正式回归和 1.53×：`KunminghuV2MinimalConfig`；
- 12×：`MinimalConfig` 下计算路径观测区间（vdot 2 周期 vs baseline 24 周期），不含访存、配置、调用和循环；
- 1.53× 是完整内核调用周期比，不是单条指令 latency，也不是整机/LLM 加速；
- 12× 不是整机加速、不是吞吐率，不能外推到正式配置；
- 正式配置没有采 FST。

## 5. 参数化

- 只做到独立模块级：n=4/8/16 生成 Verilog；
- 模块级 iverilog 验证：471/479/449 个输出全过，3 处变异被抓；
- 未接进香山，不是整核参数化；
- 没有综合、STA，所以没有面积、频率、功耗、时序余量；
- Chisel 源码是平衡树，但 firtool 输出是左结合加法链，不能据此说时序更好。

## 6. 验证范围

- 69,656 向量验证的是独立 `vdot8.sv` 原型，不是 Scala 集成版；
- 整机 difftest 验证的是 Scala 集成版，跑了 1,030 个程序级用例 × 5 变体；
- a0 版三份回归：vdot 841,260/790,553、vadd 23,876/16,870、exception 274/2,865；
- 没有形式化等价、没有 100% 覆盖率、没有随机指令流回归。

## 7. 构建与环境

- `KunminghuV2MinimalConfig` 早期出现 OpenLLC 负位宽错误；设置 `NOOP_HOME`/`NEMU_HOME` 后构建通过，但没有做控制变量实验，根因未证实；
- FST 构建曾因 `NOOP_HOME` 缺失、VM 关机等失败；最终通过独立 `emu-fst` 构建；
- 归档二进制哈希来自 `version-info.txt`，二进制未随仓库提交。

## 8. 可以写的结论

- 在 `KunminghuV2MinimalConfig` 下，a0 版 vdot/vadd/exception 三份回归均 GOOD TRAP、a0=0、无 difftest mismatch。
- 固定输入、256 次完整内核调用：1.53–1.54×，不是单条指令延迟。
- `MinimalConfig` 计算路径观测区间：2 vs 24 周期，12×，不是整机加速。
- 8 种非法配置的 mtval 分两类：VL 类 0，SEW/LMUL/vstart 类 0x0220a18b，标准非法 0x7b。
- 参数化模块级 n=4/8/16 功能验证通过，3 处变异被抓；未综合、未整核集成。

## 9. 不能写的结论

- “所有测试退出码 0”“HIT GOOD TRAP 就是全部断言通过”；
- “所有非法配置/异常路径都对齐了”；
- “整机加速 12 倍”“延迟降低 12 倍”；
- “1.53× 是单条指令加速比”；
- “加法树 log2(n) 层所以时序更好”；
- “参数化已经整核集成/已经综合”；
- “面积/频率/功耗有收益”。
