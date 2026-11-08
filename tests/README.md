# 测试

## `tb_vdot8.sv`

- 读取 `evidence/vectors/vectors.hex`（69,656 行）；
- 逐向量驱动 `vdot8.sv`；
- 检查输出、背压、flush、reset、取消事务；
- 期望输出：`PASS vectors=69656 checked=69657 cancelled=4 cycles=108440`。

## `test_reference.py`

- Python oracle：`dot8`、`pack`、`encode`；
- 覆盖极值、高位忽略、编码、非法输入。

## 运行

```bash
python3 tools/run_tests.py
```

需要 Icarus Verilog 14.0。仓库不包含 OSS CAD Suite；可解压到 `tools/oss-cad-suite`，或把 `iverilog`/`vvp` 放到 PATH。
