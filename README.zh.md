<div align="center">

<picture>
  <source media="(prefers-color-scheme: light)" srcset="docs/assets/hero-public-light.svg">
  <img src="docs/assets/hero-public.svg" alt="bo2-mac：使用你自己的 Steam 正版副本，将《Call of Duty: Black Ops II》从 x86-32 Windows 静态重编译为原生 ARM64 macOS。在 Apple M1 上：战役 51 至 60 帧每秒，Zombies 57 至 60，对战机器人的多人模式 53 至 60。" width="100%">
</picture>

# bo2-mac

**《Call of Duty: Black Ops II》，从 32 位 x86 Windows 静态重编译为原生 ARM64 macOS。**<br>
战役、Zombies 以及对战机器人的多人模式，使用你自己的 Steam 正版副本，在你的 Mac 上构建。

<a href="README.md">English</a> · <a href="README.fr.md">Français</a> · <a href="README.es.md">Español</a> · <a href="README.pt-BR.md">Português</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.ru.md">Русский</a> · <a href="README.ja.md">日本語</a> · <a href="README.ko.md">한국어</a> · <b>中文</b>

<a href="#安装"><img src="docs/assets/btn-play.svg" alt="开始游玩" height="40"></a>&nbsp;
<a href="#结果"><img src="docs/assets/btn-results.svg" alt="结果" height="40"></a>&nbsp;
<a href="#流水线"><img src="docs/assets/btn-pipeline.svg" alt="流水线" height="40"></a>&nbsp;
<a href="#发现"><img src="docs/assets/btn-findings.svg" alt="发现" height="40"></a>&nbsp;
<a href="#路线图"><img src="docs/assets/btn-roadmap.svg" alt="路线图" height="40"></a>

<a href="https://news.ycombinator.com/item?id=50449891"><img src="docs/assets/btn-hackernews.svg" alt="登上 Hacker News" height="44"></a>

![source: x86-32 Windows](https://img.shields.io/badge/source-x86--32%20Windows-ff6a00?style=flat-square&labelColor=0b0b0c)
![target: ARM64 macOS](https://img.shields.io/badge/target-ARM64%20macOS-ff6a00?style=flat-square&labelColor=0b0b0c)
![lifter: Remill, LLVM 17](https://img.shields.io/badge/lifter-Remill%20%E2%86%92%20LLVM%2017-ff6a00?style=flat-square&labelColor=0b0b0c)
![renderer: D3D11 to Metal](https://img.shields.io/badge/renderer-D3D11%20%E2%86%92%20Metal-ff6a00?style=flat-square&labelColor=0b0b0c)
![Wine: none](https://img.shields.io/badge/Wine-none-ff6a00?style=flat-square&labelColor=0b0b0c)
![game files in this repository: none](https://img.shields.io/badge/game%20files-none-f4f1ea?style=flat-square&labelColor=0b0b0c)
[![latest release](https://img.shields.io/github/v/release/adrbn/bo2-mac?style=flat-square&labelColor=0b0b0c&color=ff6a00&label=release)](https://github.com/adrbn/bo2-mac/releases/latest)
![status: beta](https://img.shields.io/badge/status-beta-f4f1ea?style=flat-square&labelColor=0b0b0c)

<sub>[为什么](#为什么) · [摘要](#摘要) · [方法](#方法) · [局限](#局限) · [仓库](#仓库) · [法律声明](#法律声明)</sub><br>
<sub>测试版，2026-11-03 · 在一台 Apple M1 上测得（4 个性能核心 + 4 个能效核心，16 GB）</sub>

</div>

<p align="center"><a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4"><img src="docs/assets/preview.gif" alt="战役、Zombies 和多人模式在 macOS 上原生运行" width="560"></a><br>
<sub>在 Apple M1 上边玩边录制的战役（Angola）、Zombies（Town）和多人模式（Hijacked，对战机器人）；右上角的数字是每秒帧数，屏幕录制本身会占用一部分性能，在战役中尤为明显。<a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4">80 秒视频（MP4，20 MB）</a></sub></p>

> [!IMPORTANT]
> **本仓库不包含任何游戏文件，也不包含任何翻译后的游戏代码。** 它在你的 Mac 上，把你自己的 Black Ops
> II Steam 正版副本变成一个供你的 Mac 使用的应用。游戏的代码最终会存在于这个应用之中，因此没有人能给你一个
> 现成的应用：每位玩家都要自行构建，且不得分享。非官方的粉丝项目，与
> Activision、Treyarch 或 Microsoft 无关。要游玩，请购买游戏。测试版：请阅读[哪些已被证明，哪些尚未](docs/PRERELEASE.md)。

---

<img src="docs/assets/rule-01.svg" alt="" width="100%">

## 为什么

Black Ops II 从未登陆过 Mac，而在 Apple Silicon 上此前也谈不上真正可玩。
[AppleGamingWiki](https://www.applegamingwiki.com/wiki/Call_of_Duty:_Black_Ops_II) 将其列为在
CrossOver 下无法游玩，在 Rosetta 2 下不受支持。剩下的只有 Parallels 中的 Windows ARM 虚拟机，据报告，
表现从 M1 Max 上对局中的每秒 1 帧，到 M3 和 M4 上的 60–80 帧不等
（[MacGamingDB](https://macgamingdb.app/games/call-of-duty-black-ops-ii)）。在这里，游戏在启动之前就一次性
翻译成原生 ARM64 应用：在入门款 M1 上达到每秒 51–60 帧，无需 Windows，也无需虚拟机。原生
也意味着它可以像任何 Mac 应用一样被修复和扩展：PC 版从未有过的分屏，
已作为 Zombies 中的实验性选项提供。

## 摘要

bo2-mac 接收 Black Ops II 的三个可执行文件，即战役（`t6sp.exe`）、Zombies（`t6zm.exe`）和
多人模式（`t6mp.exe`），并提前将它们的 x86 机器码转换为 ARM64 机器码。
[Remill](https://github.com/lifting-bits/remill) 把每个函数提升为 LLVM IR，一连串我们自己的 pass 重写
这份 IR，然后结果与一个手写的 Win32 运行时链接在一起，其中的 Direct3D 11 由 Metal 绘制。这里没有
Wine，也没有 JIT：游戏以原生代码运行，在单一进程中。三个可执行文件中共有约 150,000 个函数被翻译。只有版权保护所隐藏的
那些短代码序列，以及翻译所遗漏的任何代码，才在一个小型 x86 解释器中运行，因此漏洞被设计为损失速度而不是让游戏崩溃。

在 Apple M1 上，战役以 **每秒 51–60 帧** 运行，每个任务都能加载并衔接到下一个，
直至片尾字幕。Zombies 通过游戏自带的菜单单人游玩 **六张地图**，帧率为 **57–60**。
多人模式通过其真实菜单对战机器人，十一种游戏模式，帧率 **53–60**。一个应用，
`Black Ops II.app`，打开战役，而每个菜单中的 CAMPAIGN / ZOMBIES / MULTIPLAYER 条目会切换到
另外的游戏。

除了你自己，不会从任何人那里下载任何东西：转换器读取你所购买副本的可执行文件，逐字节
检查，并在你的机器上构建应用。

<img src="docs/assets/rule-02.svg" alt="" width="100%">

## 结果

| | 测量值 | 条件 |
|---|---|---|
| 战役 | **51–60 fps** | 每个任务都能加载并衔接至片尾字幕；并非所有任务都完整玩过 |
| Zombies | **57–60 fps**（Die Rise 43–58） | TranZit、Mob of the Dead、Nuketown、Origins、Buried、Die Rise，单人，通过真实菜单 |
| 对战机器人的多人模式 | **53–60 fps** | 通过真实菜单的十一种游戏模式：完整一局、计分板、返回大厅 |
| 游戏世界与真实时间 | **≈ 1.0×** | 游戏自身的服务器时钟与墙上时间对比 |
| 在玩家 Mac 上的转换 | M1 上 **约 1 小时 45 分**（仅战役：约 45 分钟） | 战役转换完成后即可游玩；Zombies 和多人模式随后跟进 |
| 磁盘 | 峰值约 **10 GB**，保留约 **4 GB** | 转换完成后，只保留链接好的二进制文件 |

帧率统计的是真正呈现到屏幕上的 drawable，并与屏幕和 Metal HUD 对照核实。

**你可以玩什么**

<table>
<tr>
<td width="33%" valign="top"><img src="docs/assets/card-public-campaign.svg" alt="战役" width="100%"><br>
真实的主菜单：NEW STORY、RESUME STORY、REPLAY MISSION。从 <code>angola</code> 到片尾字幕，
各任务依次加载并衔接。Strike Force 任务一点击即可开始。</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-zombies.svg" alt="Zombies" width="100%"><br>
六张地图，单人：主菜单、大厅、对局、Game Over、计分板，然后在同一进程中开始另一局。
如果你的 Steam 账户拥有 DLC 地图，也可以游玩。双人分屏为实验性功能。</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-multi.svg" alt="多人模式" width="100%"><br>
通过真实菜单对战机器人的自定义游戏：CHOOSE CLASS、HUD、完整一局、计分板
并返回大厅。对局规则是本项目自己的脚本。</td>
</tr>
</table>

三种模式均支持：键盘和鼠标，或通过 GameController.framework 以 XInput 方式使用 DualSense；有声音；移动
按键依照其在你键盘上的位置（AZERTY 上为 ZQSD，QWERTY 上为 WASD），除非你在
游戏选项中自行绑定了按键。

<img src="docs/assets/rule-03.svg" alt="" width="100%">

## 流水线

分五步，全部在玩家的 Mac 上完成：

1. **检查。** 转换器只接受每个可执行文件的确切 Steam 版本，通过 SHA-256 校验。
   启动器会询问你 Mac 上的 Steam 客户端，已登录的账户是否拥有该游戏，否则拒绝运行。
2. **编目与提升。** Ghidra 列出你自己可执行文件中的函数；Remill v6.0.1 把每个函数提升为
   LLVM IR。对提升器的五个补丁保存在 [tools/remill-patches/](tools/remill-patches/) 中，其中包括
   x87 舍入模式，以及一个为每个函数 fork 一个运行中的 Remill、而不是重新加载其 x86 语义的补丁。
3. **折叠。** Remill 的 IR 是正确的，但很慢：每一次客户机内存访问、标志位和 x87 压栈，都是一次调用或一次写入
   `State` 结构的存储。我们的 pass 在 `clang -O2` 之前对其进行重写：客户机的读写被内联，`lock`
   序列变为硬件原子操作，客户机的 `ret` 保留在被提升的代码之内，x87 栈则被提升为
   局部变量。
4. **运行时。** [runtime/](runtime/) 实现了游戏真正调用的 Windows 导入函数，stdcall 参数个数精确无误
   （Steam API 的部分取自 Valve 的公开头文件）。客户机内存是一块 4 GiB 的映射。Direct3D 11 是
   覆盖在 Metal 之上的伪 COM 层；DXBC 着色器经由 [HLSLcc](third_party/HLSLcc) 处理。XAudio2 对接 CoreAudio，
   视频则使用 libvpx 原生解码。
5. **线程。** 客户机线程作为协程在一个宿主线程上启动，这消除了大部分
   非确定性来源，并使一次运行可以重放。快速配置会把服务器、渲染后端、声音混音与解码，以及任务
   工作线程移到真正的宿主线程上。ARM 不是 TSO，因此这需要真正的内存屏障。

**版权保护是被执行的，而不是被绕过的。** 游戏把完整性检查藏在硬件异常、
单步陷阱以及它解密到堆中的代码之后。一个基于 Unicorn 构建的小型 x86 机器
（[x86_guest_vm.cpp](runtime/win32/x86_guest_vm.cpp)、[heap_code.cpp](runtime/win32/heap_code.cpp)）恰好运行这些
序列，然后把控制权交还给重编译后的代码。没有任何检查结果被强行指定，也没有任何密钥被手工放入。

**翻译中的漏洞被设计为损失速度，而不是崩溃。** 当游戏运行到从未被提升的 x86 代码时，
同一台机器会运行其原始字节（[interp.cpp](runtime/win32/interp.cpp)），并在下一个已翻译的块、调用或返回处，把控制权交还给
重编译后的代码。每个被解释执行的地址都会被记录（`[INTERP]`）。

**在线服务由本地应答。** Zombies 和多人模式期待 Demonware；一个根据
游戏自身请求编写的应答器（[dw_local.cpp](runtime/win32/dw_local.cpp)）在你的 Mac 上应答大厅、存储和票据。
没有任何数据离开这台机器。

<details>
<summary>为什么不用 Wine、CrossOver、DXVK 或 DXMT？</summary>

它们在运行时翻译 Windows 调用，并把 x86 代码保留在 Rosetta 之下。而在这里，x86 代码本身已经不复存在：它在游戏启动之前就是
ARM64 机器码。DXVK 和 DXMT 是 PE DLL，通过 Wine 的
unixlib 机制访问其原生后端；这个移植版没有 Wine，它的 Direct3D 11 是自己的、建立在 32 位客户机内存之上的一层。只有着色器
翻译需要借助外部力量，而 HLSLcc 这个纯 C++ 库就能满足。
</details>

<img src="docs/assets/rule-04.svg" alt="" width="100%">

## 发现

每个条目都包含屏幕上看到的症状、测得的原因，以及修复后的效果。

<details>
<summary><b>十三项发现</b>，从 x87 舍入模式到一条指令内部的错误起点</summary>


**F1 · 提升器忽略了 x87 舍入模式。** 在 Zombies 中，俯卧视角会抖动。`fistp` 在游戏要求截断的地方
却舍入到最近值，位置在视角高度的网络编解码器之内。客户端收到的高度
在 8 个样本中有 8 个是错的，经过一个 Remill 补丁并重新提升该编解码器之后，8 个中错 0 个。

**F2 · x87 控制字中的一个保留位。** 一个战役任务卡死在特效循环中，仅在使用真实
线程时出现。提升器对控制字第 6 位的存储方式与真实硬件不同。一个单行的 Remill 补丁加上四个
重新提升的运行时函数就修好了它，且没有任何减速。

**F3 · 一个 CPU 特性标志会改变算法。** 我们的 `IsProcessorFeaturePresent` 回答了「不支持 SSE2」，于是 C
运行时采用了它的 x87 `ceil`，而这在我们的运行时下会舍入到最近值。某个特效随后永远采样索引 −1：两个
任务卡死。

**F4 · 卡顿是我们自己的日志造成的。** 170–430 毫秒的卡顿，是渲染线程被阻塞在 `write()` 中：我们的日志
被重定向到一个几乎写满的磁盘上的文件。把它通过管道发送给子进程之后，每次运行的卡顿次数从
12 和 8 降到 0 和 0。

**F5 · 着色器翻译把常量移动了 4 个字节。** HLSLcc 把一个 `float2` 放在偏移 24 处的一个 `float` 之后，而
Direct3D 在 20 处读取它。2,664 个着色器中有 31 个受到影响，其中包括眼睛和准星。

**F6 · 保护必须运行。** 选项菜单 8 次中有 8 次卡死。一个完整性检查会改写自己的返回
地址，使其指向解密到堆中的代码，而运行时忽略了这一点。像 Windows 那样运行这段代码之后，
3 次中有 3 次不再卡死。

**F7 · 我们自己的安全保护措施成了障碍。** 我们的一个保护措施在游戏进行到 28 秒时让游戏停止。该项目最大的
收益来自移除我们自己的保护措施和伪共享，而不是巧妙的代码。

**F8 · 空的 API 桩看起来像是服务坏了。** 两个什么也不返回的桩（`CryptGenRandom`、
`StringFromGUID2`）在 Zombies 中看起来像是网络故障。

**F9 · 线程优先级是程序的一部分。** `SetThreadPriority` 曾是一个桩。把混音器的请求传递给
macOS 之后，一次运行中的短音频缓冲区数量从 908–1872 降到 7–71，声音中断也随之消失。

**F10 · 原生蒙皮，逐位一致。** 用 NEON 重写的 CPU 蒙皮得到与原始 x86 相同的位，并使
一个繁重的场景从 17.8 提升到 30.3 fps。

**F11 · 贴花需要游戏的深度偏移。** 焦痕和血迹会随着镜头的轻微转动而出现或消失。游戏会为贴花请求深度偏移；我们的层把它保持为零。把游戏的值传递给 Metal 后问题得到修复。

**F12 · 渲染线程每一帧都在复制关卡几何数据。** 在多人模式和 Zombies 中，它每帧向 GPU 复制 23–63 MB
的顶点数据，其中大部分是静态的。让静态缓冲区常驻在 Metal 中，把复制量降到
每帧 3.5–5 MB：多人模式 51.8 → 60 fps，Zombies 43.1 → 58.2。同样的改动在战役中毫无作用，
因为那里的节奏由调度线程决定。一项提速只对瓶颈被它触及的模式有帮助。

**F13 · 一条指令内部的错误起点，让战斗中的帧率损失一半。** 在从全新 Steam 副本进行的最初几次转换中，只要玩家开火，Zombies 就会掉到 25 fps。热点函数在解释器中运行：
编目把下一个函数的起点放进了某个根的第一条指令之内，因此它的有界扫描
解码了零个字节，这个根就被悄无声息地丢弃了。像逐个提升的
提升器本来就做的那样，不加边界地扫描这类根，就把它们放回了已翻译的代码中。

</details>

<img src="docs/assets/rule-05.svg" alt="" width="100%">

## 方法

**先预测，再测量。**

- 在读取结果之前先写下预测：到本测试版发布时约有 1,100 条带编号的预测。
- 每种配置至少运行三次。运行间的噪声为 30–64%，因此在 n = 3 时，低于约 40% 的差异无法看出。
- 一项改动必须证明它确实生效：日志中必须能看到它被触发。
- 从未到达关卡的运行是无效运行，而不是数据点。
- 屏幕胜过日志。当屏幕与某个探针相矛盾时，错的是探针。

<details>
<summary><b>录制与重放、撤回、净室、谁做什么</b></summary>

**确定性录制与重放。** 由于每个客户机线程都是一个位于单一宿主线程上的协程，大部分
非确定性来源都已消除；重放是与录制核对过的，而不是想当然。三个边界捕获了所有外部输入：导入分发器、窗口消息队列的清空，以及
被消费的音频缓冲区。日志约 16 MB。它允许同一次运行执行两遍，并且只改变一个标志。

**出问题的恰恰是仪器。** 一路上有近 200 项发现被撤回。最大的一部分
（31%）是回答了另一个问题的仪器；其次（17%）是根本无法回答的仪器。因此
这里的每个判据都会说明其输入，并在缺少该输入时拒绝回答。

**对局规则的净室。** 多人模式和 Zombies 的对局规则来自一个每日补丁文件，而 Steam
已不再提供它。[runtime/match_rules/](runtime/match_rules/) 中的替代品由一个从未见过
原始脚本的团队编写：一个团队根据游戏写出行为规格说明，另一个团队仅凭
该规格说明编写代码，两者只通过书面的问答交流，并附日期保存。

**谁做什么。** AI 智能体提供吞吐量：编写垫片、对每次运行中的数百万次调用做二分排查、阅读数万
行代码。人类提供目标、根据屏幕上的真实内容做出纠正，并拒绝讨好的结论。这是一款游戏，数月的工作，以及大量的人工干预。

</details>

<img src="docs/assets/rule-07.svg" alt="" width="100%">

## 局限

- **仅限本地。** 多人模式和 Zombies 针对你 Mac 上的应答器运行。没有在线游玩，也没有局域网联机。League Play 被拒绝。
- **对局规则是我们自己的。** 在净室中编写（参见[方法](#方法)）；行为在某些地方可能与原版
  不同。
- **发热。** 长时间游玩时 Mac 会非常烫（M1 上报告 111–116 °C）。请接通电源游玩：使用电池时，
  macOS 会严重限制游戏的性能。接通电源可以避免降速，但不能避免发热。
- **每张地图的首局游戏。** 在 Zombies 和多人模式中，着色器在地图加载时为 Metal 翻译，因此
  首次加载更长，第一分钟仍可能出现轻微卡顿。在战役中，它们在
  首次出现时翻译，所以加载过场动画保持流畅，而首次经过某个地点时可能会卡顿。一切
  都会被缓存：下一次，加载很快，运行也很流畅。
- **窗口。** 游戏在 1280×720 的窗口中运行。
- **其他 Mac。** 只测量过一台 M1。M2、M3 和 M4 应该更快，但我们不声称任何数字。
- **战役。** 每个任务都能加载并衔接；并非每个都完整玩过。
- **Zombies。** 没有在线或局域网合作，因此无法游玩 Grief 和 Turned。双人本地分屏
  是实验性的：第 1 回合开始时玩家 2 才会出现。
- **macOS 音频。** 当系统的 `coreaudiod` 状态不正常时，战役会卡在它的第一段视频上。请检查
  `afplay -t 1 /System/Library/Sounds/Tink.aiff` 是否耗时约 1 秒；重启 Mac 可以解决。

<img src="docs/assets/rule-08.svg" alt="" width="100%">

## 安装

你需要一台 Apple silicon 的 Mac，至少 **20 GB 可用空间**（转换峰值约 10 GB，保留约 4 GB），以及
已登录到拥有该游戏的账户的 Mac 版 Steam。

**macOS 26 或更高版本：无需终端。** 从[发布页面](https://github.com/adrbn/bo2-mac/releases)下载 `Black Ops II.app`
并打开它。它自带翻译工具（LLVM 17、Remill、Unicorn、libvpx、HLSLcc、capstone，由
[scripts/make_toolchain.sh](scripts/make_toolchain.sh) 构建），不含任何游戏文件。首次运行时，macOS 会提示安装
Apple 免费的命令行工具：点击「安装」，然后在应用窗口中点击「重试」。
每天启动时一次，应用会查找新版本并提示安装。「安装，保留此版本」会把
当前应用保留在旁边，并以其版本号重命名，因此打开那个应用即可回退。更新会保留已经转换好的内容，只重做其改动涉及的部分：修复运行时后，在 M1 上约 3 分钟即可重新链接三种模式。

**macOS 14 或 15：从终端安装**，使用 Homebrew：

```bash
git clone https://github.com/adrbn/bo2-mac.git && cd bo2-mac
brew install llvm@17 cmake ninja libvpx unicorn
pip3 install capstone
scripts/make_app.sh --from-source
```

打开 `Black Ops II.app`，指向你的游戏文件夹并让它转换：在 M1 上约 1 小时 45 分，先转换战役，约 45 分钟。战役在
转换完成后即可游玩：那时就可以玩，而 Zombies 和多人模式会在你首次于游戏菜单中选择它们时进行转换。否则转换会继续处理这三种模式。如果磁盘可用空间低于
8 GB，转换会干净地停止，并在中断处继续。

**Zombies 分屏（实验性）。** 接上第二个手柄，在 Zombies 大厅中按它的 Start 或 A：玩家 2 加入，就像在主机上一样。
在这个手柄上按 B，或拔掉它，玩家 2 就会退出。玩家 1 玩上半屏，玩家 2 玩下半屏；比赛开始时画面分割，玩家 2 在第 1 回合开始时出现。
没有人加入时，游戏仍为单人全屏。无需勾选任何选项。

**更新，或转换中途停止时。** 在仓库文件夹中运行 `git pull && scripts/make_app.sh --from-source`：
已转换的部分会被保留，转换会继续。更新只会重做其改动所涉及的部分：对运行时的
修复会在 M1 上约 3 分钟内重新链接三种模式，而不是重新转换。如果仍然失败，
请附上它所指出的日志提交一个 issue。

转换器只接受确切的 Steam 版本，通过 SHA-256 校验。Steam 的可执行文件由 Valve 的
SteamStub 包装器加密，**本仓库不提供任何移除它的方法**：转换器期望在 `~/.bo2-native/steam/` 中找到三个已解密的
镜像（`t6sp.exe.unpacked.exe`、`t6mp.exe.unpacked.exe`、`t6zm.exe.unpacked.exe`）。你需要
自己用你所购买的副本，借助你自行选择的另一个工具来制作它们。只需去掉包装器：上面的文件名
和 SHA-256 校验会立刻告诉你一个文件是否正确。切勿分享这些文件。

**DLC。** 启动器会询问你的 Steam 客户端你的账户拥有哪些 DLC，与游戏在 Windows 上的做法完全一致。当你拥有某张 DLC 地图
且其文件位于你的游戏文件夹中时，该地图即可游玩。

<img src="docs/assets/rule-09.svg" alt="" width="100%">

## 仓库

| 路径 | 内容 |
|---|---|
| [runtime/win32/](runtime/win32/) | Win32 运行时：kernel32、调度器、Direct3D 11、声音、输入、本地服务应答器 |
| [runtime/gfx/](runtime/gfx/) | Metal 后端：画面呈现、着色器缓存、渲染目标 |
| [runtime/match_rules/](runtime/match_rules/) | 多人模式和 Zombies 的净室对局规则 |
| [tools/](tools/) | 提升驱动、编目、IR pass、反汇编与分析工具 |
| [tools/remill-patches/](tools/remill-patches/) | 叠加在 Remill v6.0.1 之上的补丁 |
| [scripts/](scripts/) | `make_app.sh`、Steam 安装器与转换器、启动器 |
| [third_party/HLSLcc](third_party/HLSLcc) | DXBC 到 Metal 的着色器翻译 |
| [docs/](docs/) | 预发布状态、工具链 |

<img src="docs/assets/rule-10.svg" alt="" width="100%">

## 路线图

1. **本测试版**：在全新的 Steam 副本上运行三种模式，并且是在构建它们的那台之外的其他 Mac 上。你的报告
   （Mac 型号、帧率、哪里出了问题）决定接下来做什么。
   首份外部报告，2026-11-05：在一台 M2 Pro（16 GB）上，转换完成且游戏启动，
   其间发现并修复了三个问题（[#4](https://github.com/adrbn/bo2-mac/issues/4)、[#5](https://github.com/adrbn/bo2-mac/issues/5)、[#6](https://github.com/adrbn/bo2-mac/issues/6)）。
2. **无需终端**：已完成。下载并打开一个应用即可；它自带已构建好的工具，因此除了游戏本身，
   你的 Mac 上不需要编译任何东西，并且它会自动更新（macOS 26 或更高版本；macOS 14 和 15 仍使用
   终端安装）。
3. **完全翻译**：仍通过 Unicorn 解释器运行的 46 个代码块将被提前
   翻译，每条被翻译的指令都会与解释器核对；
   任何已翻译的函数都可以送回解释器，以便通过二分法找出出错的那一个。
4. **无需等待着色器**：着色器在第一局之前构建，而不是在启动时（多人模式中约 3 分钟）和
   游戏过程中（出现新特效时卡顿）。已经开始：着色器现在分批编译，来自
   最近几次会话的着色器在启动时于后台编译。在没有 Xcode 的 M1 上的多人模式中（试了几次，不算
   基准测试），一局比赛前两分钟内的着色器编译，在首次启动时从 18–25 秒降到 10–14 秒，
   下一次启动时约 5 秒；超过 300 毫秒的卡顿从 15–22 次降到 13–15 次，然后是 7 次。首次启动
   的卡顿频率仍大致相同。
5. **更短的转换**：在 M1 上从约 2.5 小时降到 1 小时 45 分（2026-11-06，三种模式全部测得：
   准备阶段只读取一次可执行文件，提升和折叠使用内存允许的所有核心）。现在，更新只会重做
   发生变化的部分（运行时修复：约 3 分钟）。下一步：对很少运行的代码减少优化。三种模式只共享其 4 % 的
   函数，因此只转换一次共享代码收益不大。
6. **更快，且不发热**：战役受限于调度线程，而不是渲染；有 11 个机器人的多人模式
   受限于服务器线程。已开始：混响在三种模式中都以原生方式运行，声音线程上的耗时减少约 40 %。接下来是对其他最热点函数的原生重写，然后是面向
   120 Hz 屏幕的高于 60 的帧率。
7. **手柄**：振动与辅助瞄准。
8. **一起玩**：Zombies 中的本地分屏（PC 版从未有过）是实验性的（一个游戏，
   两名玩家，上下两半屏幕，在 M1 上约 28 fps），应用菜单中有开关。下一步：为 Zombies 合作、
   Grief 和 Turned 提供真正的 UDP 套接字，以及局域网多人模式。
9. **以后**：MetalFX、画面呈现时的后期处理，以及从玩家自己的副本加载的替换纹理。

<img src="docs/assets/rule-11.svg" alt="" width="100%">

## 法律声明

针对用户拥有的副本，在用户拥有的硬件上使用。

- 不使用引擎源代码，也不使用任何泄露的代码。
- 输入的二进制文件和重编译的输出都不会分发。不包含任何游戏文件：玩家需要自己的
  Call of Duty: Black Ops II Steam 正版副本。转换器只接受确切的 Steam 版本（SHA-256），并且
  每次游戏启动时都会由玩家自己的 Steam 客户端检查所有权：当 Steam
  未登录或账户不拥有该游戏时，游戏拒绝运行。
- 版权保护按游戏编写的方式被执行；其中没有任何内容被修补掉。本仓库不提供任何方法
  移除 Valve 的 SteamStub 包装器。
- 游戏所期待的在线服务由一个根据游戏自身请求编写的本地应答器应答：不会联系任何
  Activision 服务器，没有在线游玩，并且游戏所有权仍由玩家的 Steam 检查。
  对局规则是在净室中编写的。
- 此处编写的代码以 GPL-3.0 许可（[LICENSE](LICENSE)）。
- Unicorn（GPL-2.0）绝不会在分发的二进制文件中与 GPL-3.0 代码结合。应用携带 Unicorn 未经修改的
  库，作为紧邻其他工具的单独文件；它在玩家自己的 Mac 上被链接进游戏二进制文件
  （[tools/boot.py](tools/boot.py)），而该二进制文件从不分发。每个发布版本都会公布其所携带的
  确切 Unicorn 版本的源代码（`unicorn-<version>-source.tar.gz`），这是 GPL-2.0 所要求的。每个
  所携带工具的许可证都在应用中，位于 `Contents/Resources/toolchain/licenses`。

这是一个非官方的粉丝项目。Call of Duty 和 Black Ops 是 Activision Publishing, Inc. 的商标。Steam 是
Valve Corporation 的商标。本项目与 Activision、Treyarch、
Microsoft 或 Valve 没有隶属关系，也未获得其认可或赞助。要游玩，请购买游戏。

## Star History

<a href="https://www.star-history.com/#adrbn/bo2-mac&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
  </picture>
</a>

<div align="center"><img src="docs/assets/footer.svg" alt="" width="100%"></div>
