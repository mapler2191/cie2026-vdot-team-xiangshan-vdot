# 工具

| 脚本 | 作用 |
|---|---|
| `generate_vectors.py` | 生成 69,656 个 oracle 向量 |
| `run_tests.py` | Python oracle + Icarus RTL 单元测试，写 `unit-test-report.json` |
| `extract_vadd_wave.py` | 从 vadd FST 提取窗口信号 |
| `report_vadd_wave.py` | 34 项采样断言 + 精简 VCD + 证据 JSON |
| `extract_vdot_wave.py` | 从 vdot FST 提取窗口信号 |
| `report_vdot_wave.py` | 48 项采样断言 + 精简 VCD + 证据 JSON |
| `generate_boot_flash.py` | 生成 16 B boot flash |
| `build_integration.py` | 从固定源码归档生成补丁（需要 `research/*.zip`） |
| `fetch_public.py` | 下载固定版本公开源码（可选） |

## 说明

- `run_tests.py` 会优先使用 `tools/oss-cad-suite`；仓库不包含 600 MB 工具包。把 OSS CAD Suite 解压到该目录，或确保 `iverilog`/`vvp` 在 PATH。
- 波形脚本读取 `evidence/waves/*.fst`，输出 `evidence/reports/*.json` 与 `evidence/waves/*-focused.vcd`；路径按仓库根目录相对路径调整。
- `build_integration.py` 需要 `research/xiangshan.zip`、`research/nemu.zip`；仓库不包含这些归档，补丁已在 `rtl/`、`nemu/` 提供。
