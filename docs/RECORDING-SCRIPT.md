# 三阶段录屏拍摄脚本（照着念即可）

适用：Windows 宿主 + Ubuntu 24.04 VMware 虚拟机，工作区 `D:\RISV`。
目标：三段视频分别对应赛题要求 —— ① 第一阶段操作过程、② 第二阶段解题思路、
③ 第三阶段波形仿真与性能分析。

**使用方式**：从第 0 节开始照做，每一段的「操作」直接复制粘贴，「照念」逐字读出来。
念错不用停，接着说；卡住了就重录这一小段。

---

# 第 0 节 拍之前的准备（约 30 分钟，这一步别省）

## 0.1 把要展示的文件放进 VM

在 Windows 里把整个 `D:\RISV\docs` 和下面几个结果目录复制到虚拟机
（VMware 共享文件夹、`scp`、或用 U 盘都行）：

```
~/risv/docs/                                  ← D:\RISV\docs 整个目录
~/risv/results/t5b-clean-ref/                 ← 三支 a0 回归日志
~/risv/results/t2-mtval-probe/                ← mtval-probe.log
~/risv/results/t3-highbits-backtoback-v2/     ← highbits-v2.log、backtoback.log
~/risv/results/t5-nemu-fixed-regressions/     ← A/B 对照日志 + nemu-tval.patch
~/risv/results/kunminghu-v2-final/            ← 两份性能日志
~/risv/results/latency-comparison/            ← latency-evidence.json + 两份 focused VCD
```

复制完在终端里确认一句：

```bash
ls ~/risv/docs | head
ls ~/risv/results/latency-comparison/*.vcd
```

## 0.2 确认六样东西都在（缺一样就补）

```bash
cd ~/risv/project/software
export EMU=~/risv/xs-env/XiangShan/build/emu
export REF=~/risv/xs-env/NEMU/build/riscv64-nemu-interpreter-so
export FLASH=~/risv/artifacts/boot-minimal.bin

ls -l $EMU $REF $FLASH
make evidence 2>&1 | tail -3
ls -l build/hello.bin build/vadd.bin build/vdot-basic.bin \
      build/vdot-wave.bin build/exception-check-a0.bin build/mtval-probe.bin
gtkwave --version | head -2
```

- `make evidence` 里有 `vdot-basic`，是这次新加的**三类基础功能测试**（正数/负数/边界），
  5–10 秒跑完，专门为视频二准备。
- 没装 GTKWave 就先装：`sudo apt install -y gtkwave`（在录制前装好，不要录进去）。

## 0.3 确认 hello 的身份信息是真的

```bash
cat ~/risv/project/software/hello_identity.h
```

必须看到你自己的团队名和 VM 的 IP（不是 `YOUR_TEAM` / `YOUR_VM_IP`）。
**顺便把这份文件复制回 Windows 的 `D:\RISV\software\hello_identity.h` 存档**——
工作区那份还是占位符，交付时要一致。

## 0.4 屏幕与软件设置（照做）

1. 虚拟机全屏：VMware 菜单 `View → Full Screen`，或按 `Ctrl+Alt+Enter`。
2. 分辨率 1920×1080：Ubuntu `Settings → Displays`。
3. 终端字体放大到 18：终端里 `Edit → Preferences → 勾选 Custom font → 18`，
   主题选深色（黑底白字）。
4. 关通知：`Settings → Notifications` 关掉。
5. 关掉不用的窗口（浏览器、聊天、邮件），桌面只留终端。
6. 录制软件二选一：
   - **最简单**：Windows 按 `Win+G` 打开 Xbox Game Bar → 点圆点开始录 →
     在 `设置 → 捕获` 里把"麦克风"打开。
   - **画质更好**：OBS Studio → 来源加"窗口采集"选 VMware 窗口 → 音频加"麦克风" →
     输出 1920×1080、30fps → 开始录制。
   - Ubuntu 自带：`Ctrl+Alt+Shift+R` 开始/停止，文件在 `~/Videos`（不带麦克风，不推荐）。

## 0.5 声音自检（1 分钟）

先录 20 秒随便说两句，回放听：有没有回声、音量是不是太小、键盘声是不是太响。
合适了再正式录。

## 0.6 三条通用规则

1. 每段开头先说「接下来这一段，我们……」，让评委知道要看什么；
2. 每段结束前说一句「这一段对应提交要求的第 X 条」；
3. 屏幕上出现的每个数字，都要说清它是**哪来的**（跑出来的 / 日志里读的 / 文档里写的）。

---

# 视频一：第一阶段 · 环境部署 + hello + 向量加法执行过程

**目标时长 10–12 分钟。** 文件命名 `01-stage1-env-hello-vadd.mp4`。
覆盖提交要求 1.1、1.2、1.3。

---

## 段 1　开场（0:00–0:50）

**操作**：终端全屏，什么都不用打。

**照念**：

> 「评委老师好，我们是 **［团队名］** 队，作品是基于香山昆明湖 V2 的自定义向量点积指令
> `vdot.vv`。
> 这段视频对应第一阶段的三项提交要求：环境部署验证、完整的操作过程日志、
> 以及向量加法指令在香山里的执行过程。
> 我们全程在一台 Ubuntu 24.04 虚拟机上，用香山官方 `xs-env` 环境完成，
> 下面的每一步都可以在仓库里复现。」

---

## 段 2　系统与工具链（0:50–2:00）

**操作**（一条一条打，打完停 2 秒再打下一条）：

```bash
uname -a
nproc
free -g | head -2
df -h ~ | tail -1
```

```bash
verilator --version
java -version 2>&1 | head -3
riscv64-linux-gnu-gcc --version | head -1
git --version
```

**照念**：

> 「这是运行环境：Ubuntu 24.04、［看 nproc 的数字］个核、内存 ［看 free 的数字］G。
> 香山的仿真流程需要三样工具：Verilator 负责把生成的 RTL 编成 C++ 仿真器，
> JDK 负责跑 Chisel 精化，交叉编译器负责编裸机测试程序。
> 版本分别是 ［念 verilator 版本］、［念 java 版本］、［念 gcc 版本］。
> 这些版本号在仓库的 `version-info.txt` 里也有记录，和提交的二进制是对应的。」

---

## 段 3　源码版本与构建产物（2:00–3:40）

**操作**：

```bash
cd ~/risv/xs-env/XiangShan && git log -1 --format='%H %s'
cd ~/risv/xs-env/NEMU && git log -1 --format='%H %s'
```

```bash
cd ~/risv/xs-env/XiangShan
ls -l build/emu
sha256sum build/emu
cat ~/risv/results/kunminghu-v2-final/version-info.txt
```

> 如果 `git log` 报 `not a git repository`（有些环境是解压版），跳过那两条，
> 直接念 `version-info.txt` 里记录的 commit 即可，不要现场折腾。

**照念**：

> 「香山固定在提交 ［念前 8 位］，NEMU 固定在 ［念前 8 位］。
> 仿真器是 `build/emu`，大小 ［念］，SHA256 前 16 位是 ［念你屏幕上的］。
> 我们提交版记录的 emu 哈希前 16 位是 `755c80de1eebdd3f`、boot flash 是 `fea548655a8b6eec`；
> 如果屏幕上的对得上，说明当前仿真器就是提交那一版。
> 也就是说后面所有日志，都能追到这三个二进制上。」

> 「我们的改动分两块：RTL 侧是 `xiangshan.patch` 加新增的 `Vdot8Datapath.scala`，
> 参考模型侧是 `nemu.patch`。两个补丁都只包含已跟踪文件的修改，新增文件单独归档。」

---

## 段 4　操作过程日志（3:40–5:10）

**操作**：

```bash
cd ~/risv
ls
ls results/
```

```bash
tail -40 ~/.bash_history
```

```bash
cat ~/risv/project/software/Makefile | head -40
```

**照念**：

> 「这是工作目录：`xs-env` 是香山和 NEMU 的源码，`project/software` 是我们的裸机测试程序，
> `artifacts` 放 boot flash，`results` 按阶段归档每一次运行的日志。
> 左边这些 `t1`、`t2`、`t3`、`t5b` 目录，就是每个证据对应的原始日志。」

> 「完整操作日志方面：这是当前的 shell 历史，可以看到从下载源码、编译、
> 跑回归到分析波形的命令序列；更早的完整记录已经按阶段归档在 `results` 下，
> 每条日志都带运行命令和退出码。
> 我们刻意没有只留成功记录——失败的尝试也留着，比如 `t2-exception-matrix-v2`
> 那个目录，标记为废弃，原因写在证据索引里。」

**注意**：如果 `~/.bash_history` 是空的，跳过第二条命令，只念 results 部分。

---

## 段 5　hello 实跑（5:10–6:40）

**操作**：

```bash
cd ~/risv/project/software
$EMU -i build/hello.bin --diff $REF -F $FLASH
echo "exit code = $?"
```

**等它跑完（约 15 秒）。** 屏幕上会出现一行类似：

```
hello xiangshan, I am ［团队名］, IP address ［VM 的 IP］
```

**照念**（在等的时候也要说话）：

> 「现在跑第一阶段的 hello 程序：把编译好的裸机程序装进香山，打开 difftest 和 NEMU 参考模型
> 对齐，Flash 固定用这个 16 字节的 boot。
> 程序通过串口打印一行文本，把自己是谁、IP 是多少带出来。
> 串口地址是 `0x40600004`，每次写一个字节。
> ［看到输出后］这里是输出：`hello xiangshan, I am …`，IP 是 …。
> 最后是 `HIT GOOD TRAP`，退出码 0。」

**要强调的一句**：

> 「注意：hello 这个程序只有串口输出这一条断言，所以它的退出码 0 只代表仿真器正常结束。
> 真正用退出码判通过的，是后面那些 `*-a0.bin`：我们把 a0 编进 trap 指令的 rs1 字段，
> 所以进程退出码就等于程序里的 a0 —— 0 就是全过、非 0 就是失败。第二阶段会展开讲。」

---

## 段 6　向量加法执行过程（6:40–10:40）

### 6a　打开执行过程文档（6:40–8:10）

**操作**：

```bash
less ~/risv/docs/vadd-stage1-wave-report.md
```

按 `↓` 慢慢翻到「指令与有效事件」那张表，然后停住。

**照念**：

> 「这一份是标准向量加法 `vadd.vv` 的执行过程分析，依据是我们自己采的 FST 波形。
> 被分析的这条指令在 PC `0x80000044`，编码 `0x021101d7`，就是 `vadd.vv v3, v1, v2`，
> 前置配置是 VL=16、SEW=8、LMUL=1。
> 表里每一行是一个真实采样值：2514 进译码流水，2516 分派，2538 出调度队列，
> 2541 进 ALU，2542 ALU 输出，2544 有效提交。
> 输入是 A=[0..15]、B=[0,2,4,…,30]，输出 C=[0,3,6,…,45]，16 个元素逐个满足 C[i]=A[i]+B[i]。
> 这份分析一共核验了 34 项信号断言。」

> 「顺便说一个我们踩过的坑：香山普通 `--dump-wave` 用的是**仿真周期计数**当时间轴，
> 哪怕 VCD 头里写的是 1ps，也不能拿它算物理频率。所以这份文档里只说周期，不说纳秒。」

### 6b　GTKWave 看波形（8:10–10:10）

**操作**（逐句执行）：

```bash
gtkwave ~/risv/results/vadd-focused.vcd &
```

1. 窗口打开后，按 `f`（或点工具栏的 **Zoom Fit**），让整段窗口铺满。
2. 左侧是信号树，顶部有个 **Filter/Search 输入框**：输入 `io_in_valid`，回车。
3. 单击匹配到的信号，按 `Insert`（或点工具栏的 **Append**），它会出现在右边波形区。
4. 再依次用 `io_out_valid`、`commitValid` 各加一次。
   （如果 Filter 里搜不到，就点开树，一层层展开到 `TOP` 下面找同名的信号。）
5. 时间轴上的数字就是采样值，窗口大概在 2480–2565 之间。
   用菜单 **Search → Goto Time**（快捷键 `Ctrl+G`）输入 `2541`，回车。
6. 鼠标移到这里，用工具栏放大镜放大到能看清 `valid`、`ready` 两行的高低。

**照念**（边操作边说）：

> 「这是从 FST 里裁出来的精简波形，只保留这条指令前后的窗口。
> 左边是信号树，我们把译码有效、提交有效这几根信号拉出来。
> ［按 f 之后］这里每一格是一个周期。
> ［Goto 2541］这一拍是 ALU 的输入有效、并且握手成功，也就是操作数进了运算单元；
> 下一拍 2542，输出有效、写回使能拉高，结果就出来了。
> 输入到输出相隔一拍的采样，也就是这条 `vadd.vv` 在功能单元里的一拍延迟。」

> 「要特别说清楚边界：这是**功能单元内部**的延迟，不是整个程序的周期；
> 译码到提交中间隔了 30 个采样，那里面包含发射和调度等待，不能当成指令延迟。」

### 6c　实跑一次 vadd 回归（10:10–11:00）

**操作**：

```bash
$EMU -i build/vadd-a0.bin --diff $REF -F $FLASH
echo "exit code = $?"
```

**等它跑完（约 35 秒）。** 屏幕上会出现 `HIT GOOD TRAP at pc = 0x80000032` 和
`Core-0 instrCnt = 23,876, cycleCnt = 16,870`。

**照念**：

> 「最后实际跑一遍标准向量加法的回归。这里用的是 `vadd-a0`，也就是把 a0 编进 trap 字的版本，
> 所以退出码可信。40 组输入，含定向图案和随机，逐元素比对全部通过。
> 23,876 条指令、16,870 周期、退出码 0。
> 这条基线在第三阶段会作为 vdot 的性能对照组再出现一次。」

---

## 段 7　收尾（11:00–11:20）

**照念**：

> 「第一阶段的三个交付物到这里演示完了：环境部署与版本验证、操作日志、以及
> 向量加法在香山里从译码、分派、调度到写回提交的执行过程。
> 下一段视频讲第二阶段：`vdot.vv` 的设计与测试。」

---

# 视频二：第二阶段 · vdot.vv 解题思路与基础功能测试

**目标时长 12–15 分钟。** 文件命名 `02-stage2-design-tests.mp4`。
覆盖提交要求 2.1（设计文档）、2.2（RTL）、2.3.1–2.3.3（正数/负数/边界测试）。

---

## 段 1　开场（0:00–0:40）

**照念**：

> 「这一段讲第二阶段：`vdot.vv` 的设计思路、RTL 实现，以及三类基础功能测试。
> 先说一句结论：这条指令把两个 128 位向量寄存器里**低 8 个 int8 元素**做点积，
> 得到一个 int32 结果，写在目的寄存器低 32 位，高 96 位清零。
> 语义上等价于 `vwmul.vv` 加 `vwredsum.vs`，但是我们用一条指令、一个微操作做完。」

---

## 段 2　指令语义与编码（0:40–3:00）

**操作**：

```bash
less ~/risv/docs/vdot-design-doc.md
```

停在**第 1 节**，念完再按 `↓` 到**第 2 节**。

**照念（第 1 节）**：

> 「语义就是这样一页纸：输入是 vs1 和 vs2 的低 8 个字节，每个字节按 int8 解释；
> 八对乘积求和，范围是负 13 万到正 13 万 1 左右，所以用 int32 装；高 96 位清零。
> 允许的配置是 SEW=8、VL=8、LMUL=1、vstart=0、vm=1。
> 目的寄存器可以和源寄存器别名，也可以是 v0，这一点我们专门测了。」

**照念（第 2 节）**：

> 「编码是我们自定义的 `0x0220a18b`。
> 字段是 funct7 等于 0000001、funct3 等于 010、opcode 是 0001011 —— 注意不是标准 OP-V 的
> 1010111，这样不会和现有指令冲突。
> `0x0220a18b` 具体展开就是 `vdot.vv v3, v1, v2`。
> 这里有个细节：第 25 位也就是 vm 位被固定成 1，所以带掩码的形式 `vm=0` 根本不匹配这条指令，
> 会走普通非法指令，而不是进我们的 helper——这一点我们在测试里也确认过。」

---

## 段 3　RTL 实现（3:00–6:00）

**操作**：

```bash
less ~/risv/results/kunminghu-v2-final/Vdot8Datapath.scala
```

**照念**：

> 「这是新增的数据通路模块，一共 30 行。
> 上面 `VdotSpec` 定义了指令 BitPat 和功能单元操作码 `0x100`。
> 下面是八路乘法：每路取 8 位、按有符号相乘，结果用 `RegEnable` 打到乘积寄存器，
> 使能信号是第一级流水使能。
> 然后是一棵平衡树：两两相加得 17 位，再两两得 18 位，再两两得 19 位，最后 pad 到 32 位。
> 结果再打一拍寄存器，第二级使能。
> 输出是 `Cat(0.U(96.W), result)`，就是把高 96 位补零。」

> 「为什么是两级：功能单元的延迟要求是 2 拍，`require(latency == 2)`；
> 乘积寄存器一级、结果寄存器一级，正好对上。」

**操作**：

```bash
less ~/risv/results/kunminghu-v2-final/xiangshan.patch
```

**照念**（边翻边说）：

> 「集成一共四处改动。
> 第一处 `VecDecoder`：把这条指令映射到 vimac 功能单元，操作码 0x100。
> 第二处 `RiscvInst.isVecArith`：让预译码把它当向量运算，不再当未知指令。
> 第三处 `DecodeUnit`：如果发现是 vdot 但 SEW、LMUL、vstart 不合法，在**译码阶段**就报非法。
> 第四处 `VIMacU`：例化我们的数据通路，把两级使能接上；
> 原来的乘法模块在点到点积指令时关掉 fire，输出用 Mux 选我们的结果；
> `vxsat` 不影响；非法信号也按是不是点积分流——里面有 `VL≠8` 这一项。」

---

## 段 4　位宽：两个必答题（6:00–8:00）

**照念**（对着设计文档第 4 节）：

> 「评委通常会问两个位宽问题，我们提前算好。
> 第一，乘积为什么 16 位够？int8 范围是 -128 到 127，最大乘积是 `-128 × -128 = 16384`，
> 最小是 `127 × -128 = -16256`，16 位有符号最大 32767，装得下。
> 第二，求和为什么用 19 位？八项最大是 `8 × 16384 = 131072`，
> 而有符号 18 位最大只到 `131071`，差 1 就装不下。
> 所以中间结果必须 19 位：16 加到 17、18、19，每级扩一位。
> 这一点如果省成 18 位，`-128 × -128` 全满的用例就会溢出。」

> 「另外，目的寄存器高 96 位直接补零，不是留旧值。这样和基线
> 『先清零目的寄存器再做宽归约』的结果语义完全一致，可以逐位比较。」

---

## 段 5　三类基础功能测试（8:00–11:30）

**操作**：

```bash
cd ~/risv/project/software
$EMU -i build/vdot-basic.bin --diff $REF -F $FLASH
echo "exit code = $?"
```

**等它跑完（5–10 秒）。** 屏幕上会出现六行 `[VDOT] …`，类似：

```
[VDOT] positive expect=204 actual=204 upper96=0,0,0 PASS
[VDOT] negative expect=-168 actual=-168 upper96=0,0,0 PASS
[VDOT] edge-max  127x127 expect=129032 actual=129032 upper96=0,0,0 PASS
[VDOT] edge-min -128x-128 expect=131072 actual=131072 upper96=0,0,0 PASS
[VDOT] edge-mixed -128x127 expect=-508 actual=-508 upper96=0,0,0 PASS
[VDOT] edge-zero 0x0 expect=0 actual=0 upper96=0,0,0 PASS
[SUMMARY] failures=0
```

**照念**：

> 「这是专门为这三类测试写的程序，六行输出对应提交要求的三个小项。
> 第一行是**正数**：1 到 8 点乘 1 到 8，期望 204，实测 204。
> 第二行是**负数**：0 到 7 点乘 -1 到 -8，期望 -168，实测 -168。
> 第三到第六行是**边界值**：
> 全 127×127 得 129032，是有符号正极值方向；
> 全 -128×-128 得 131072，是乘积最大的一档；
> 正负混合 -128×127 得 -508；
> 全零得 0。
> 每一行还检查了目的寄存器高 96 位全是 0，因为目的寄存器事先写的是 0x55555555，
> 如果没被清零就会读出来。
> 最后 failures=0，退出码 0——注意是因为我们用了 a0 编码的 trap，退出码真的反映结果。」

---

## 段 6　更全的回归（11:30–13:00）

**操作**（不要跑完整回归，它要 26 分钟，这里读归档日志）：

```bash
tail -5 ~/risv/results/t5b-clean-ref/vdot-a0.log
grep -E 'HIT GOOD TRAP|instrCnt' ~/risv/results/t5b-clean-ref/vdot-a0.log
```

**照念**：

> 「除了这六行，我们还跑了一个大回归：1030 组输入，每组分别调用五种实现——
> 自定义 vdot、标准 RVV 组合基线、两个源寄存器别名、以及 v0 作目的寄存器，
> 一共 5150 次点积，含 127×127、-128×-128、-128×127、零和正负抵消这些极值。
> 这是跑完的日志：841,260 条指令、790,553 周期、`HIT GOOD TRAP`、退出码 0。
> 它要跑 26 分钟，所以这里读日志，不现场重跑。」

---

## 段 7　收尾（13:00–13:20）

**照念**：

> 「第二阶段的交付物：设计文档、RTL 实现、以及正数、负数、边界三类功能测试，
> 到这里演示完了。第三段视频讲 NEMU 扩展、协同仿真、基线对比和波形性能分析。」

---

# 视频三：第三阶段 · 协同仿真 + 波形仿真 + 性能分析

**目标时长 12–15 分钟。** 文件命名 `03-stage3-cosim-perf.mp4`。
覆盖提交要求 3.1.1（NEMU 扩展）、3.1.2（协同仿真）、3.2（基线）、3.3（延迟加速分析）。

---

## 段 1　开场（0:00–0:40）

**照念**：

> 「第三阶段有四件事：给 NEMU 加 vdot 支持、让 NEMU 和 RTL 协同仿真、
> 用标准向量指令搭一个功能相同的基线、以及在这个基础上做延迟和加速分析。
> 这一段最有价值的内容是中间这个协同仿真——我们发现并修复了参考模型的一个建模错误，
> 下面会展示修复前后的对照。」

---

## 段 2　NEMU 扩展（0:40–3:30）

**操作**：

```bash
less ~/risv/results/t5-nemu-fixed-regressions/nemu-tval.patch
```

**照念**：

> 「NEMU 侧一共三处改动。
> 第一处是译码表：加一条 IDTAB，把 `0000001 … 010 … 00010 11` 这个图样识别成 vdot。
> 第二处是 helper：读 vs1、vs2 的低 8 个字节，按 int8 做乘加，结果拆成低 32 位和
> 高 96 位写进 vd；写之前先把两个源都读完，所以目的寄存器可以和源别名。
> 第三处，也就是这一版修的地方，是非法配置的表错处理。
> 我们实测发现香山对非法配置的 mtval 分两类，所以 helper 也要分两类：
> SEW、LMUL、vstart 不合法是**译码阶段**发现的，RTL 会把指令字填进 mtval，
> 所以这里直接抛异常、走默认路径；
> 而 VL 不等于 8 是**执行阶段**发现的，RTL 没有 trapped instruction，mtval 是 0，
> 所以只有这一种才需要把 mtval 强制成 0。」

**要补一句（重要）**：

> 「我们也没有说这套模型在所有路径上都对：S 模式委托、RNMI 这些路径没有测，
> 报告里写的是『已测的 8 种 M 模式非法配置一致』。」

---

## 段 3　协同仿真：现场跑一个（3:30–5:00）

**操作**：

```bash
cd ~/risv/project/software
$EMU -i build/exception-check-a0.bin --diff $REF -F $FLASH
echo "exit code = $?"
```

**等它跑完（约 6 秒）。** 会出现三行 `[ex-enter]`（如果你的参考库是带打印的版本）
和 `HIT GOOD TRAP at pc = 0x80000032`、`instrCnt = 274, cycleCnt = 2,865`。

**照念**：

> 「这是异常恢复测试，开 difftest 跑。
> 里面三次故意触发非法：第一次 VL=0，然后一条标准非法指令，再回到 VL=0。
> 每次都要检查 mcause 是不是 2、mepc 是不是指向那条 vdot、mtval 对不对、
> 目的寄存器有没有被破坏，然后 mret 回来继续。
> 三行 ex-enter 就是三次异常，全部通过，退出码 0。
> NEMU 和 RTL 在这三次异常上状态完全一致，否则 difftest 会直接报 mismatch 打断仿真。」

---

## 段 4　mtval 分两类与 A/B 对照（5:00–7:30）

**操作**：

```bash
grep -E '^\[RESULT|^\[PROBE\] done' ~/risv/results/t2-mtval-probe/mtval-probe.log
```

**照念**：

> 「这是八种非法配置的逐例实测，来自一个专门的探针程序。
> 注意第五到第七行：SEW=16、LMUL=2、vstart=1，这三种的 mtval 是 `0x220a18b`，
> 也就是指令字本身；而前面 VL=0/1/7/9 四种，mtval 是 0。
> 第八行标准非法指令是 `0x7b`。
> 这说明 RTL 在译码和执行两个阶段处理非法配置的方式不同——译码阶段知道指令字，
> 执行阶段不知道。我们的 NEMU 模型就是按这个实测结论改的。」

**操作**：

```bash
grep -E 'mtval different|HIT (GOOD|BAD) TRAP' ~/risv/results/t5-nemu-fixed-regressions/sew16-diff-old-so.log
grep -E 'HIT (GOOD|BAD) TRAP' ~/risv/results/t5-nemu-fixed-regressions/sew16-diff-new-so.log
```

**照念**：

> 「这是最有说服力的一对证据：同一个 SEW=16 的测试程序，同一台仿真器，
> 只换参考模型。
> 修复前的参考模型把所有非法配置都报 0，于是 difftest 直接报
> `mtval different`，参考模型的值是 0、RTL 的值是 0x220a18b，仿真被打断。
> 修复后同一个程序 `HIT GOOD TRAP`，没有任何 mismatch。
> 这一对日志同时证明了：difftest 确实在比对 mtval，不是只看 mcause 和 mepc。」

---

## 段 5　基线实现（7:30–8:40）

**操作**：

```bash
grep -A12 'vdot_baseline' ~/risv/project/software/kernels.S
```

**照念**：

> 「性能对比的基线不是标量实现，而是标准 RVV 的最接近写法：
> 先用 `vwmul.vv` 做 8 位到 16 位的宽乘，得到 16 个 int16；
> 再用 `vwredsum.vs` 做宽归约，累加到 32 位。
> 因为要做 tail-undisturbed 语义，目的寄存器和种子寄存器都要先清零。
> 这两条指令在硬件上拆成 2 个宽乘微操作加 4 个归约微操作，一共 6 个 uop；
> 而我们的 vdot 只有 1 个 uop。这是后面所有延迟差异的根源。」

---

## 段 6　性能：256 次内核调用（8:40–10:00）

**操作**：

```bash
grep -E 'PERF_ORDER|PERF_V1' ~/risv/results/kunminghu-v2-final/perf-baseline-first.log
grep -E 'PERF_ORDER|PERF_V1' ~/risv/results/kunminghu-v2-final/perf-custom-first.log
```

**照念**：

> 「性能测量用的是 256 次**完整内核调用**，每次调用都包含配置、加载、计算、存储、
> 函数调用和循环开销，不是单条指令。
> 为了排除顺序带来的偏差，两个方向都测了：
> 先跑基线，基线 7881 周期、vdot 5141 周期，1.533 倍，周期减少 34.8%；
> 先跑 vdot，7882 和 5119，1.540 倍，减少 35.1%。
> 两个方向一致，所以可以说**约 1.53 到 1.54 倍的内核加速**。」

**必须补的边界**：

> 「这个数字只适用于**完整内核调用**这个口径：不能说成单条指令的延迟比，
> 也不能外推到整机或端到端应用。另外它来自昆明湖 V2 配置，而下面的波形数据来自
> MinimalConfig，两者配置不同，我们在报告里是分开写的。」

---

## 段 7　延迟：波形上的 2 和 24（10:00–13:00）

**操作**：

```bash
less ~/risv/docs/latency-comparison-report.md
```

停在「真实事件时间表」那张表。

**照念**：

> 「这是同一配置下两条路径的波形对照，一共核验了 208 项信号断言。
> 事件定义统一成：第一个算术微操作被功能单元接收，到最终完整结果在功能单元输出有效并被接收。
> vdot 是 2390 进、2392 出，2 个周期。
> 基线要 6 个微操作：宽乘两个在 2408、2409 进，2410、2411 出；
> 中间两个归约微操作 2415、2416 进，2417、2418 出，
> 后面两个依赖前一个，2423 和 2430 进，2425 和 2432 出。
> 所以基线的区间是 2408 到 2432，24 个周期。区间比 12 倍。」

**操作**：

```bash
gtkwave ~/risv/results/latency-comparison/vdot-latency-focused.vcd &
```

按 `f` 铺满，Filter 里输入 `valid`，把输入/输出有效信号 `Insert` 进来，
然后 `Ctrl+G` 跳到 2390 和 2392 分别看。

**照念**：

> 「这是 vdot 的精简波形。2390 这里输入有效、握手成功，2391 乘积寄存器更新，
> 2392 输出有效、写回使能拉高，结果 `0x…ffffff58` 就是 -168。
> 中间那一拍就是两级数据寄存里的第二级。」

> 「必须说清楚这个 12 倍的含义：它是**计算路径观测区间之比**，
> 已经排除了前面清零、加载和最后存储；
> 它包含基线里微操作之间的依赖和调度等待，
> 所以既不是两条指令的固定延迟之和，也不是吞吐率，更不能推出整机加速 12 倍。」

---

## 段 8　收尾（13:00–13:30）

**照念**：

> 「第三阶段四件事演示完了：NEMU 扩展、协同仿真（含 mtval 分两类的修复与前后对照）、
> 标准 RVV 组合基线、以及完整内核与计算路径两个口径的性能分析。
> 所有证据都归档在仓库里，每条主张都能追到一份日志或波形。
> 谢谢评委老师。」

---

# 第 4 节 拍完自检（逐条打勾）

## 视频一

- [ ] 出现了 `uname -a`、Verilator / JDK / gcc 版本
- [ ] 出现了 XiangShan 与 NEMU 的 commit
- [ ] 出现了 `build/emu` 的 sha256，并和 `version-info.txt` 对上
- [ ] 出现了操作日志（history 或 results 目录）
- [ ] hello 跑出 `hello xiangshan, I am …` 且**团队名、IP 是自己的**
- [ ] vadd 执行过程讲了：PC、编码、采样 2514/2516/2538/2541/2542/2544
- [ ] GTKWave 里看到 2541→2542 的输入输出握手
- [ ] 明确说了"波形时间轴是周期，不是物理时间"
- [ ] vadd 回归退出码 0

## 视频二

- [ ] 语义讲清：低 8 个 int8、int32 结果、高 96 位清零
- [ ] 编码讲清：`0x0220a18b`、BitPat、vm 位固定 1
- [ ] 四处集成改动都点到
- [ ] 讲了 16 位乘积够、19 位求和必需（131072 > 131071）
- [ ] 三类测试实跑并念出六行结果
- [ ] 说明了退出码来自 a0 编码的 trap
- [ ] 完整回归只读日志，没现场跑 26 分钟

## 视频三

- [ ] NEMU 三处改动讲清，包含 mtval 分两阶段
- [ ] 说了"S/VS、RNMI 未测"
- [ ] 现场跑了 exception-check-a0，三次异常 + 退出码 0
- [ ] 展示了 8 行 mtval 实测（两类 + 0x7b）
- [ ] 展示了 A/B：`mtval different` → 修复后无 mismatch
- [ ] 基线讲清是 vwmul+vwredsum、6 个 uop
- [ ] 1.53–1.54× 讲了两个方向，并说明是完整内核口径
- [ ] 2 vs 24、12 倍，并说明是计算路径区间、不是整机
- [ ] 说明了性能与波形的配置不同

---

# 第 5 节 常见翻车点（提前避开）

| 现象 | 原因 | 处理 |
|---|---|---|
| 终端里中文变方块 | 终端字体不支持中文 | 换 `DejaVu Sans Mono`，或念稿全用英文路径 |
| GTKWave 打不开 VCD | 没装或没加 `&` | `sudo apt install -y gtkwave`，命令末尾加 `&` |
| 跑 emu 时没有 `HIT GOOD TRAP` | 忘了 `-F` 或 `--diff` 路径错 | 三条路径先 `ls -l` 确认 |
| 忘记 `echo "exit code = $?"` | —— | 每条 emu 命令后面都跟一句 |
| 视频里出现隐私（邮箱、聊天） | 桌面没清 | 录前关掉所有窗口 |
| 说到一半卡词 | 正常 | 停 1 秒，重说这一句，后期不必剪 |

---

# 第 6 节 命名与提交

```
01-stage1-env-hello-vadd.mp4     （对应：一阶段操作过程录屏）
02-stage2-design-tests.mp4       （对应：二阶段解题思路讲解录屏）
03-stage3-cosim-perf.mp4         （对应：三阶段波形仿真与性能分析录屏）
```

上传前再确认三件事：

1. 三段都**听得清**（回放抽 30 秒）；
2. 每段开头都报了**团队名**；
3. 视频里出现的关键数字，和 `docs/EVIDENCE-INDEX.md` 里写的一致
   （841,260 / 790,553、23,876 / 16,870、274 / 2,865、1.53–1.54×、2 vs 24、12×、
   mtval 两类）。
