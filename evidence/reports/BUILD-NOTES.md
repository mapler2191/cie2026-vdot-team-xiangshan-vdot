# KunminghuV2MinimalConfig 编译说明

## 现象

早期构建 KunminghuV2MinimalConfig 时，Chisel 精化在 openLLC 报 `Invalid bit range [hi=-4, lo=0]`。

## 处理

显式导出环境变量后重新构建，构建通过：

```bash
export NOOP_HOME=$HOME/risv/xs-env/XiangShan
export NEMU_HOME=$HOME/risv/xs-env/NEMU
```

## 因果说明（重要）

我们**没有做"设 / 不设 NOOP_HOME"的对照实验**，因此不能断言负位宽错误的根因就是缺少
`NOOP_HOME`。本文件只记录"设置后构建通过"这一事实；根因未确认。
（另有一次 FST 构建报 `NoSuchElementException: NOOP_HOME`，那次的位置和现象都不同，
也不能与负位宽错误合并归因。）

## 补丁说明

- `xiangshan-vdot-complete.patch`：4 个已跟踪文件的修改 + 新增 `Vdot8Datapath.scala`；
- 归档目录里的 `xiangshan.patch` 只有 `git diff` 对已跟踪文件的输出，不含新增文件；
- NEMU 侧最终用 `nemu-tval.patch`（含 mtval 分阶段修复）。
