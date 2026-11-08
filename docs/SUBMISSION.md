# 提交说明

本仓库是 2026 CIE 香山社区赛道参赛作品的公有代码仓库。提交分三条线：文档、视频、代码。

## 1. 文档（上传到官网"文档提交位置"）

赛题页面：<https://rv.cie.org.cn/direction?id=8>

| 文件 | 对应要求 |
|---|---|
| `docs/00_提交说明与文件清单.pdf` | 总说明、文件清单、团队分工 |
| `docs/01_第一阶段_环境部署与hello验证.pdf` | 1.1 环境部署验证、1.2 操作日志 |
| `docs/02_第一阶段_向量加法指令执行过程.pdf` | 1.3 vadd 执行过程 |
| `docs/03_第二阶段_vdot.vv设计文档.pdf` | 2.1 设计文档 |
| `docs/04_第二阶段_RTL实现说明.pdf` | 2.2 RTL 实现 |
| `docs/05_第二阶段_正数负数边界测试报告.pdf` | 2.3.1–2.3.3 三类基础测试 |
| `docs/06_第三阶段_NEMU扩展与协同仿真.pdf` | 3.1.1 NEMU 扩展、3.1.2 协同仿真 |
| `docs/07_第三阶段_基线与延迟加速分析.pdf` | 3.2 基线、3.3 延迟加速分析 |
| `docs/08_代码仓库说明.pdf` | 仓库结构与复现方式 |

当前阶段不需要提交 PPT。

## 2. 视频（上传到官网"视频提交位置"）

1. 第一阶段操作过程：环境、hello、vadd 波形；
2. 第二阶段解题思路：语义、编码、数据通路、三类测试；
3. 第三阶段波形与性能：difftest、mtval 两类、1.53× 与 12×。

逐段脚本（操作步骤 + 逐字旁白 + 自检清单）见 `docs/RECORDING-SCRIPT.md`。

## 3. 代码与加密提交

### 3.1 创建 public 仓库

```bash
git init
git add .
git commit -m "2026CIE vdot.vv: RTL, NEMU, tests, docs and evidence"
git branch -M main
git remote add origin https://github.com/mapler2191/cie2026-vdot-team-xiangshan-vdot.git
git push -u origin main
```

> `.gitignore` 已放行 `evidence/waves/*.fst`、`evidence/waves/*.vcd`、`results/**/*.vcd`；
> 提交前用 `git status` 确认证据文件没有被忽略。

### 3.2 Fork XiangShanLab

```bash
git clone https://github.com/mapler2191/XiangShanLab.git
cd XiangShanLab/2026-CIE-RISC-V-Contest-Application-Track
```

### 3.3 填写 my_submission.txt

`submission/my_submission.txt` 已填好：仓库 `mapler2191`、邮箱、西北工业大学、周慧思老师、联系电话。提交前只需核对，不要保留尖括号占位符。

### 3.4 加密

```bash
source ./submit.sh my_submission.txt
```

生成 `01_参赛选手提交区/<team-name>/submission.asc`。

### 3.5 push 并 PR

```bash
git add .
git commit -m "提交2026CIE大赛作品：<team-name>"
git push
```

PR：base `OpenXiangShan/XiangShanLab:main`，head `mapler2191/XiangShanLab:main`。

## 4. 官方链接

- 提交指南：<https://github.com/OpenXiangShan/XiangShanLab/blob/master/2026-CIE-RISC-V-Contest-Application-Track/SUBMISSION_GUIDE.md>
- 提交脚本：<https://github.com/OpenXiangShan/XiangShanLab/blob/master/2026-CIE-RISC-V-Contest-Application-Track/submit.sh>
- 赛题页面：<https://rv.cie.org.cn/direction?id=8>

> 加密保护的是 `my_submission.txt`（提交信息），不是整个代码仓库；仓库必须是 public。

