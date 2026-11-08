# RTL

## 文件

| 文件 | 说明 |
|---|---|
| `xiangshan/Vdot8Datapath.scala` | 集成到香山 VIMacU 的数据通路 + `VdotSpec` |
| `xiangshan/xiangshan-vdot-complete.patch` | 4 改 1 增的完整补丁（含新增 Scala 文件） |
| `standalone/vdot8.sv` | 独立 SystemVerilog 原型，用于 Icarus 穷举 |

## 应用

```bash
cd $NOOP_HOME
git apply /path/to/rtl/xiangshan/xiangshan-vdot-complete.patch
```

## 注意

- `xiangshan-vdot-complete.patch` 才是完整补丁；
- 归档目录里的 `xiangshan.patch` 只有 `git diff` 的 4 个已跟踪文件，不含新增 `Vdot8Datapath.scala`；
- 独立 SV 原型与 Scala 集成版结构一致，但不是同一份代码。69,656 向量验证的是 SV 原型；整机 difftest 验证的是 Scala 集成版。
