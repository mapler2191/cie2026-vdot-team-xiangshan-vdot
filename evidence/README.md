# 证据目录

| 目录 | 内容 |
|---|---|
| `logs/` | 六份整机日志：vdot/vadd/exception/hello/perf×2 |
| `reports/` | 单元测试报告、波形证据 JSON、版本信息、BUILD-NOTES、hello_identity |
| `waves/` | vadd/vdot FST、精简 VCD、perf/vdot-wave ELF 与反汇编 |
| `vectors/` | 69,656 个 oracle 向量（hex + json） |
| `manifest.json` | 仓库内文件 SHA256 清单 |

## 快速判断

- 通过：`HIT GOOD TRAP` + 预期 `instrCnt/cycleCnt` + 无 `mismatch/CRITICAL/断言`；
- 不要被 `[PERF ]` 行里的 `*_mismatch` 计数器名字误导，那是性能计数器，不是 difftest 结果；
- 日志里可能出现 `vlmuPERF_ORDER` 交错，以完整行 `PERF_V1 ...` 为准。

## 大文件

- `waves/vadd-stage1.fst` 20 MB；
- `waves/vdot-single.fst` 16 MB；
- `vectors/vectors.hex` 5.3 MB。

如果 GitHub 网络较慢，可把 FST 放到 Release，仓库保留精简 VCD + SHA256；这不会改变结论。
