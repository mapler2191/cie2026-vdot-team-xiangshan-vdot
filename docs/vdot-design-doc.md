# vdot.vv 设计文档

赛题：CIE 香山社区 RISC-V 应用创新赛道 · 自定义向量点积指令 `vdot.vv`
平台：香山昆明湖 V2（`KunminghuV2MinimalConfig`），VLEN = 128
本文所有数字都对应仓库里的源码或实测日志，出处见第 8 节。

---

## 1. 指令语义

`vdot.vv vd, vs1, vs2` 计算两个 128 位向量寄存器**低 8 个 int8 元素**的逐对乘积之和，
结果按 int32 写入 `vd` 的低 32 位，`vd` 的高 96 位清零。

```
vd[31:0]   = Σ(i=0..7) (int8)vs1[8i+7:8i] × (int8)vs2[8i+7:8i]
vd[127:32] = 0
```

- 允许的向量配置：`SEW=8`、`VL=8`、`LMUL=1`、`vstart=0`、`vm=1`；
- 目的寄存器可以与 `vs1`、`vs2` 别名，也可以是 `v0`（已实测，见 8.3）；
- 不影响 `vxsat`；不产生新的 CSR 状态。

> 一句话向评委解释：「八对 int8 点积，一个 int32 结果，其余 96 位清零——语义上等价于
> `vwmul` + `vwredsum`，但一条指令做完。」

---

## 2. 指令编码

自定义编码 `0x0220a18b`，写在 OP-V 之外的一个空闲 opcode 空间里：

| 字段 | 位 | 值 | 含义 |
|---|---|---|---|
| funct7 | 31:25 | `0000001` | 自定义指令标识 |
| vs2 | 24:20 | — | 第二个源向量寄存器 |
| vs1 | 19:15 | — | 第一个源向量寄存器 |
| funct3 | 14:12 | `010` | OPMVV 类 |
| vd | 11:7 | — | 目的向量寄存器 |
| opcode | 6:0 | `0001011` | 自定义 opcode（不是标准 OP-V 的 `1010111`） |

即 `BitPat("b0000001??????????010?????0001011")`。注意第 25 位（vm）被固定为 1：
`vm=0`（掩码形式）**不匹配**本指令，会走普通非法指令路径，而不是进入 vdot 的 helper。

`0x0220a18b` 就是 `vdot.vv v3, v1, v2`（`vd=3, vs1=1, vs2=2`）。
项目内文档写作 `vdot vd,vs1,vs2`，与标准 RVV 的 `vop vd,vs2,vs1` 顺序相反，看汇编时注意。

NEMU 侧的译码表项与之一致：
`def_INSTR_IDTAB("0000001 ????? ????? 010 ????? 00010 11", OP_V, vdot)`。

---

## 3. 微架构与集成点

### 3.1 复用范围

vdot **复用** VIMacU 既有框架：发射端口、ROB 标签、重定向、向量写回、valid 流水。
**新增**的是八路乘法数据通路本身（不能说"复用了原乘法器"，原 VIMacU 里的
乘法器没有参与点积）。

### 3.2 集成所需的四处改动

| 位置 | 改动 | 作用 |
|---|---|---|
| `backend/decode/VecDecoder.scala` | 新增表项 `VdotSpec.instruction -> OPMVV(T, FuType.vimac, VdotSpec.fuOp, …, UopSplitType.VEC_VVV)` | 让译码器认识这条指令，映射到 vimac 功能单元，`fuOp=0x100` |
| `backend/decode/isa/bitfield/RiscvInst.scala` | `isVecArith` 增加 `VdotSpec.instruction === this.ALL` | 预译码按向量运算处理，不再当成未知指令 |
| `backend/decode/DecodeUnit.scala` | `exceptionII` 增加「是 vdot 且 vtype 非法 / SEW≠8 / LMUL≠1 / vstart≠0」 | **译码阶段**就能报非法配置 |
| `backend/fu/wrapper/VIMacU.scala` | 例化 `Vdot8Datapath`；`enable1 = regEnable(1) && isDot`、`enable2 = regEnable(2)`；`mod.io.fire := io.in.valid && !isDot`；输出用 `Mux(outIsDot, dot.io.vd, mgu.io.out.vd)`；`vxsat` 与 `illegal` 都按 `outIsDot` 分流 | 把新数据通路接进原有流水与异常机制 |

### 3.3 流水与延迟

两级数据寄存，**固定 2 拍**（`require(cfg.destDataBits == 128 && latency == 2)`）：

```
周期 T   : vs1/vs2 在功能单元输入有效（fuOp=0x100）
周期 T+1 : 乘积寄存器更新（enable1）
周期 T+2 : 结果寄存器更新（enable2），输出有效
```

波形实测与之一致：输入采样 2390、输出采样 2392（见 8.3 的 vdot 波形）。
译码到提交是 42 个采样周期，那里面包含发射等待，不是指令延迟。

### 3.4 异常分流

| 触发条件 | 发现阶段 | 行为 | mtval |
|---|---|---|---|
| `VL ≠ 8` | 执行阶段（`VIMacU.dotIllegal`） | 抛 `EX_II` | **0** |
| `SEW≠8` / `LMUL≠1` / `vstart≠0` / vtype 非法 | 译码阶段（`DecodeUnit`） | 抛 `EX_II` | **指令字 `0x0220a18b`** |
| 不是本指令编码 | 译码阶段（普通非法指令） | 抛 `EX_II` | 指令字（例如 `0x7b`） |

这三行都有实测日志支撑（8 种配置逐例测量），也是 NEMU 参考模型必须分阶段建模的原因。

---

## 4. 数据通路与位宽

### 4.1 结构

```
vs1[7:0]   × vs2[7:0]    → p0 ┐
vs1[15:8]  × vs2[15:8]   → p1 ├─ 8 个 16 位有符号乘积（第一级寄存）
…                              │
vs1[63:56] × vs2[63:56]  → p7 ┘
        ↓ 平衡树 16 → 17 → 18 → 19 位（逐级 +1 位）
        ↓ 符号扩展到 32 位
   结果寄存器（第二级寄存）
        ↓
   Cat(0.U(96.W), result)  → vd[127:0]
```

### 4.2 为什么乘积用 16 位就够

int8 取值范围 `[-128, 127]`，乘积范围：

- 最大：`(-128) × (-128) = 16384`
- 最小：`127 × (-128) = -16256`

`16384 < 32767 = 2^15-1`，所以 **16 位有符号**装得下，不需要 17/32 位乘积寄存器。

### 4.3 为什么求和用 19 位（这是最容易讲错的一点）

8 项累加范围：`[-130048, 131072]`。

- 上界 `131072` 需要 18 位**无符号**，但有符号 18 位最大只到 `131071`，**差 1，装不下**；
- 所以中间结果取 **19 位有符号**（`16→17→18→19` 每级 `+&` 扩一位），最后再 `pad` 到 32 位。

如果有人问"能不能省一位"，答案是不能：`8 × (-128) × (-128) = 131072 > 131071`。

### 4.4 高 96 位为什么直接清零

`Cat(0.U(96.W), result)`：语义上 `vd` 是 128 位，低 32 位是点积结果，其余位按 0 写。
这样和"先清零目的寄存器再做宽归约"的 RVV 基线在结果语义上完全对齐，便于逐位比较。

---

## 5. 与 RVV 基线实现的对比

| | vdot.vv（本方案） | 标准 RVV 组合（基线） |
|---|---|---|
| 指令 | 1 条自定义指令 | `vwmul.vv` + `vwredsum.vs` 两条 |
| 微操作数 | **1 个 uop** | **6 个 uop**（宽乘 2 + 宽归约 4） |
| 显式依赖链 | 无（单 uop 内完成） | 宽乘两 uop → 归约四 uop 依次依赖 |
| 结果语义 | 低 32 位点积、高 96 位 0 | 相同（需先清零目的寄存器） |
| 计算路径观测区间 | 2 周期 | 24 周期 |

基线不是标量实现：它用标准 RVV 的"宽乘 + 宽归约"，是同一平台上最接近的向量实现，
保证比较的是"指令组合 vs 单指令"，而不是"向量 vs 标量"。

---

## 6. 验证覆盖

| 维度 | 内容 | 证据 |
|---|---|---|
| 功能 | 1030 组输入 × 5 种调用（custom / RVV 基线 / vs1 别名 / vs2 别名 / v0），含 127×127、-128×-128、-128×127、零、正负抵消 | `results/t5b-clean-ref/vdot-a0.log`（841,260 指令、退出码 0） |
| 三类基础功能 | 正数、负数、边界（最大值/最小值/正负混合/零） | `build/vdot-basic.bin` 的 UART 输出 |
| 非法配置 | 8 种：VL=0/1/7/9、SEW=16、LMUL=2、vstart=1、标准非法 `0x7b` | `results/t2-mtval-probe/mtval-probe.log` |
| 高位独立性 | VL=16 全宽加载 → 回存证明 → VL=8 执行；4 组高半图案 | `results/t3-highbits-backtoback-v2/highbits-v2.log` |
| 背靠背 | 三条紧邻 vdot、不同目的寄存器 | `results/t3-highbits-backtoback-v2/backtoback.log` |
| 混合指令 | vdot + vmul + vadd 交错 | `results/integration-tests/mixed.log` |
| 协同仿真 | NEMU 参考模型按两阶段建模，A/B 对照证明 difftest 比对 mtval | `docs/ACCEPTANCE-2026-11-04-T1-T6.md` 第 9 节 |

---

## 7. 已知边界（写报告时别越界）

1. **没有综合、没有 STA**：面积、频率、功耗、时序余量都没有数据；
2. **没有做 S/VS 模式异常、RNMI、重定向取消**的针对性测试；
3. **参数化版本（4/8/16）只到独立模块级**，没有接进整核；
4. 波形与延迟数据来自 **MinimalConfig**，性能 1.53× 来自 **KunminghuV2MinimalConfig**，
   两组配置不同，不能合成一句话；
5. 「12×」是**计算路径观测区间比**（首个算术 uop 被接收 → 最终结果输出被接收），
   不是整机加速，也不是吞吐率。

---

## 8. 出处

1. 语义与编码：`results/kunminghu-v2-final/Vdot8Datapath.scala`、`results/kunminghu-v2-final/xiangshan.patch`、
   `software/kernels.S`
2. 数据通路与位宽：同上 Scala 第 22–29 行；位宽上界的算术见 4.3 节
3. 实测：
   - 波形：`docs/vdot-wave-report.md`（48 项断言）、`docs/latency-comparison-report.md`（208 项断言）
   - 功能与 a0 退出码：`results/t5b-clean-ref/vdot-a0.log`
   - 非法配置：`results/t2-mtval-probe/mtval-probe.log`
   - 协同仿真 A/B：`results/t5-nemu-fixed-regressions/sew16-diff-old-so.log`、`sew16-diff-new-so.log`
4. 参考模型：`results/t5-nemu-fixed-regressions/nemu-tval.patch`
5. 总索引：`docs/EVIDENCE-INDEX.md`
