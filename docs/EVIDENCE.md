# 证据索引与 SHA256

> 说明：本文件列出**早期归档**（`evidence/`，x0 trap 字）的哈希。最终证据（a0 回归、mtval 实测、
> NEMU/RTL A/B 对照、计算路径波形、参数化验证）在仓库根目录 `results/` 下，两套都保留、各自可追溯。最终报告口径以 `docs/LIMITATIONS.md` 和 `docs/00_提交说明与文件清单.pdf` 为准；“通过”只认 a0 版 `*-a0.log`，历史 x0 日志只写“运行至结束、无 mismatch”。

本文件把“结论”映射到“文件 + 行号 + 哈希”。完整机器可读清单见 `evidence/manifest.json`。

## 1. 整机日志

| 文件 | 大小 | SHA256 | 关键行 |
|---|---:|---|---|
| `evidence/logs/vdot-regression.log` | 3,262,100 | `20876235be99589cf1ae61e0a52ac28012b6d41394d355221bfc9e4cdfc6efac` | 23435 GOOD TRAP；23436 841,260 / 790,553 |
| `evidence/logs/vadd-regression.log` | 3,065,129 | `5c64b855698f1e05639bebb290b2e00503165b29d05ead292ab2dcb3f66e6e1e` | 19061 GOOD TRAP；19062 23,876 / 16,870 |
| `evidence/logs/exception-regression.log` | 3,065,976 | `1b11e1fe58e276acf4b2f391e71973e36ed97eff71b86531d8a5d6aeab33c41e` | 19079 GOOD TRAP；19080 274 / 2,865 |
| `evidence/logs/hello.log` | 3,065,191 | `7c66ac7fd1914a4bc632f5f955ac4f132c7be9144a4b5598c3ad13f38664f36a` | 11 hello 输出（工大喵喵队）；417 / 7,754 |
| `evidence/logs/perf-baseline-first.log` | 3,076,841 | `aa1823f2b64a6c6adaf167dda39fa852088ca4252a23ea74d00f87cb9b663382` | 194 `baseline_cycles=7881 custom_cycles=5141` |
| `evidence/logs/perf-custom-first.log` | 3,076,847 | `6798ec3ae694a2c9f74c5098fe9c57e717770b0e9b6a05494ba3d2c6359a3f6b` | 194 `baseline_cycles=7882 custom_cycles=5119` |

> **hello 的两份日志**：`evidence/logs/hello.log` 是早期运行，输出 `vdot-team`（旧标识）；
> 2026-11-06 用填好的 `hello_identity.h`（工大喵喵队 / 192.168.124.133）重跑，得到
> `results/kunminghu-v2-final/hello-team20261106.log`，417 条指令 / 7,754 周期，
> 输出 `hello xiangshan, I am 工大喵喵队`。以新日志为准。

> 日志里的 `[PERF ]` 行是香山性能计数器，不是 difftest 结果。判断运行至结束看 `HIT GOOD TRAP`、`instrCnt/cycleCnt`、没有 `mismatch/CRITICAL/断言`；判断 a0=0 要看 a0 版 trap 字（`.word 0x0005006b`）的日志。历史 x0 版 `HIT GOOD TRAP` 不能当退出码。

## 2. 补丁与 RTL

| 文件 | 大小 | SHA256 |
|---|---:|---|
| `rtl/xiangshan/Vdot8Datapath.scala` | 992 | `48ff0565925fd4e2341c2708005263563039dfc8d50751fdb7446bb0ad6fca6c` |
| `rtl/xiangshan/xiangshan-vdot-complete.patch` | 6,317 | `fc3d5f5578700fc54ea990c366028683fac94782ccd798c1c1b24b99b5c7b626` |
| `rtl/standalone/vdot8.sv` | 1,751 | `1270d307e623613eed3c47931c9bfae52e078d43fc7220b1fbb08f16eb414e80` |
| `nemu/nemu-vdot-validated.patch` | 6,871 | `62af225459b3eaf80b10f5cc51bdecdd77f73e5d400deffc4b0503d40a54e506` |
| `nemu/nemu-vdot-clean.patch`（早期版本，不含 mtval 分阶段修复） | 2,674 | `c79303bb8a19832b624562516f0d4a0387614a28c21282736846f61a136d452d` |
| `nemu/nemu.config` | 5,925 | `65432da3d9cbb9418481866682190416cea2bd6800c06b8b6fba18763c3edae6` |

> `xiangshan.patch` 的完整版包含新增 `Vdot8Datapath.scala`；如果只提交 `git diff` 的 4 文件版本，会漏掉新增文件。

## 3. 波形

| 文件 | 大小 | SHA256 |
|---|---:|---|
| `evidence/waves/vadd-stage1.fst` | 20,054,170 | `b257a425a9ae0965f9cb205b1eb955d4b4d026dbe3e511acd6fc422a8d6454e8` |
| `evidence/waves/vdot-single.fst` | 15,971,480 | `6811e9a96ddc73ac37316168baddf9d4d2881e689233d4c58980afee84cd0fa5` |
| `evidence/waves/vadd-focused.vcd` | 12,213 | `1758c996369a548413ca48f4586943318012fba2984272b9c768dbc6eb6fde97` |
| `evidence/waves/vdot-focused.vcd` | 9,762 | `dc2e83f8945e7cb91ba68562169a89a34d5ee43155c3e567e3a18e25056b47a5` |
| `evidence/reports/vadd-wave-evidence.json` | 10,642 | `d6ad622c05f9ba68e8731c3ff8f95a540870a04d81b0a10d69220394e1584250` |
| `evidence/reports/vdot-wave-evidence.json` | 9,811 | `1a323abfcede5e209417a26d595238b0d5aa2eaf4eda8e6a22606e132815e2e7` |

## 4. 单元测试与向量

| 文件 | 大小 | SHA256 |
|---|---:|---|
| `evidence/reports/unit-test-report.json` | 634 | `dfdd6aec51b80c777c92380e2a628aa72fd371ca3a11b79f691b97da628ad823` |
| `evidence/vectors/vectors.hex` | 5,293,856 | `896a78eb68abd695cb96597cf568fc5f5ec45ef309a7a00ae7b0de6447f8deb5` |
| `evidence/vectors/vectors.json` | 13,189 | `345676edacd4501745c7c7317d6fdf25d29e0b3c1f640bd9962b7f2d4885ad2e` |

`unit-test-report.json` 的 `scope` 明确写着 `standalone RTL, NOT XiangShan difftest`；不要把 69,656 向量说成整机验证。

## 5. 版本与构建

| 文件 | SHA256 |
|---|---|
| `evidence/reports/version-info.txt` | `8f7ab7ccd48d3f8134fec0ffab8207ca8e8d4474c1282579dd3cbb4783b233cf` |
| `evidence/reports/BUILD-NOTES.md` | `b256160fa07ca094637b1ca75d609dcfac04d04bddb007ada58905c8824e3990` |

`version-info.txt` 记录的是**交付版**（2026-11-06 更新）；2026-10-31 首次构建的原样记录见 `evidence/reports/version-info-2026-10-31-initial.txt`。当前内容：

- XiangShan `16ae9ddcda54fc9a2fddffad73174cf793ac7814`
- NEMU `a48a11f208ff81ad765591d83271d1b339fb3861`
- emu SHA256 `755c80de1eebdd3fad8a20f22838382ace8981554f85e869e7e6333f5ee1bfd3`
- NEMU .so SHA256 `2019d71776cf3457d6c65b62d5289a2324cce55eac819d31abe5642e75af5fad`
- flash SHA256 `fea548655a8b6eecacace83e2f4380a18b8aa83bce964c8aae4cffbe2f5a9724`

## 6. 性能 ELF/反汇编

| 文件 | SHA256 |
|---|---|
| `evidence/waves/perf.elf` | `fb30fb334c028bb153c738c88ef0b992f3810897e8bab4c686efd71e6f19a9ec` |
| `evidence/waves/perf.dis` | `ad9a07316576030053bdb92838ccbf82b6bcbd321d79dcfff0a299e2aec1e14f` |
| `evidence/waves/vdot-wave.elf` | `d902dda662f2d54178c41ac37cef51aa6f840fdc767596e9349ce29addc86b26` |
| `evidence/waves/vdot-wave.dis` | `018ea8943545e56466e0850d395820b003fdb30a4d779bdb5ae031e6115ec235` |

## 7. 一句话对应

- **a0 功能回归** → `results/t5b-clean-ref/vdot-a0.log`（841,260 / 790,553）
- **a0 异常回归** → `results/t5b-clean-ref/exception-check-a0.log`（274 / 2,865）
- **8 种 mtval** → `results/t2-mtval-probe/mtval-probe.log`
- **性能** → `results/kunminghu-v2-final/perf-baseline-first.log` / `perf-custom-first.log`
- **计算路径 12×** → `results/latency-comparison/latency-evidence.json`
- **波形** → `evidence/waves/*.fst` + `evidence/reports/*wave-evidence.json`
- **版本** → `evidence/reports/version-info.txt`
- **代码** → `rtl/xiangshan/xiangshan-vdot-complete.patch` + `nemu/nemu-tval.patch`（含 mtval 分阶段修复）

## 8. 最终证据（`results/`）

| 结论 | 文件 | 关键行 |
|---|---|---|
| a0 版 vdot 回归：841,260 / 790,553，退出码 0 | `results/t5b-clean-ref/vdot-a0.log` | `HIT GOOD TRAP at pc = 0x80000032`、`instrCnt = 841,260` |
| a0 版 vadd 回归：23,876 / 16,870 | `results/t5b-clean-ref/vadd-a0.log` | `instrCnt = 23,876` |
| a0 版异常回归：274 / 2,865，三次 trap | `results/t5b-clean-ref/exception-check-a0.log` | 三行 `[ex-enter]` |
| 8 种非法配置的 mtval | `results/t2-mtval-probe/mtval-probe.log` | `mtval=0x220a18b`（SEW/LMUL/vstart 三例） |
| mtval 修复前 mismatch | `results/t5-nemu-fixed-regressions/sew16-diff-old-so.log` | `mtval different ... right=0, wrong=0x220a18b` |
| mtval 修复后无 mismatch | `results/t5-nemu-fixed-regressions/sew16-diff-new-so.log` | `HIT GOOD TRAP`，无 `mismatch` |
| 高位独立性 / 背靠背 | `results/t3-highbits-backtoback-v2/*.log` | `[HIGHBITS]`、`[BTB]`、`[RESULT] PASS` |
| 计算路径 2 vs 24 周期 | `results/latency-comparison/latency-evidence.json` | 208 项断言 |
| 参数化 n=4/8/16 | `results/parametric-verification/parametric-verification.json` | 471/479/449 输出，3 处变异被抓 |

`results/` 与 `evidence/` 的分工见 `results/README.md`。
| hello 输出团队名与 IP | `results/kunminghu-v2-final/hello-team20261106.log` | `hello xiangshan, I am 工大喵喵队` |
