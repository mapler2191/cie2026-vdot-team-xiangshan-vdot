# 环境说明

| 项目 | 值 |
|---|---|
| 宿主 | Windows 11，VMware Workstation |
| 虚拟机 | Ubuntu 24.04 LTS，x86_64，19 GiB RAM，8 GiB swap，根分区 118 GB |
| 虚拟机 IP | 192.168.124.133（VMware NAT） |
| JDK | GraalVM 21.0，`/opt/graalvm-jdk-21` |
| LLVM/Clang | `/usr/local/llvm/bin` |
| Verilator | 5.052 |
| RISC-V GCC | `riscv64-linux-gnu-` |
| 波形 | GTKWave；Icarus Verilog 14.0（单元测试） |
| XiangShan | `16ae9ddcda54fc9a2fddffad73174cf793ac7814` |
| NEMU | `a48a11f208ff81ad765591d83271d1b339fb3861` |
| 配置 | `KunminghuV2MinimalConfig` |
| boot flash | 16 B，SHA256 `fea548655a8b6eecacace83e2f4380a18b8aa83bce964c8aae4cffbe2f5a9724` |

## 环境变量

```bash
export JAVA_HOME=/opt/graalvm-jdk-21
export PATH=$JAVA_HOME/bin:/usr/local/llvm/bin:$PATH
export NOOP_HOME=$HOME/risv/xs-env/XiangShan
export NEMU_HOME=$HOME/risv/xs-env/NEMU
```

## 构建命令

```bash
cd $NEMU_HOME
make riscv64-xs-ref_defconfig
make -j$(nproc)

cd $NOOP_HOME
make emu CONFIG=KunminghuV2MinimalConfig JVM_XMX=12G EMU_THREADS=2 -j2
```

## NEMU 关键配置

```text
CONFIG_RVV=y
CONFIG_RV_SMRNMI=y
CONFIG_NMIE_INIT=y
# CONFIG_MDT_INIT is not set
CONFIG_TVAL_EX_II=y
```

## 说明

- 波形来自 `MinimalConfig`，性能来自 `KunminghuV2MinimalConfig`。
- `KunminghuV2MinimalConfig` 早期 OpenLLC 负位宽问题在设置 `NOOP_HOME` 后消失，但未做控制变量实验，根因未证实。
- VM 当前可能不可用；录屏前先启动。
