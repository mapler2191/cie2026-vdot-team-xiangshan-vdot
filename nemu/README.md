# NEMU 参考模型

| 文件 | 说明 |
|---|---|
| `nemu-vdot-validated.patch` | 跑出归档日志的版本，含诊断 `fprintf` |
| `nemu-vdot-clean.patch` | 提交版，只删除诊断打印，逻辑相同 |
| `nemu.config` | 实际使用的 `riscv64-xs-ref_defconfig` 关键项 |

## 应用

```bash
cd $NEMU_HOME
git apply /path/to/nemu/nemu-vdot-clean.patch
make riscv64-xs-ref_defconfig
make -j$(nproc)
```

## 关键配置

```text
CONFIG_RVV=y
CONFIG_RV_SMRNMI=y
CONFIG_NMIE_INIT=y
# CONFIG_MDT_INIT is not set
CONFIG_TVAL_EX_II=y
```

## 已知边界

`vdot_illegal_no_tval` 是全局标志，只在 M 模式 `EX_II/EX_VI` 消费；S/VS 委托、RNMI、重新初始化路径未覆盖。详见 `docs/LIMITATIONS.md`。
