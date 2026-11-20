<div align="center">

<picture>
  <source media="(prefers-color-scheme: light)" srcset="docs/assets/hero-public-light.svg">
  <img src="docs/assets/hero-public.svg" alt="bo2-mac: Call of Duty: Black Ops II를 x86-32 Windows에서 네이티브 ARM64 macOS로 정적 재컴파일, 본인의 Steam 사본 사용. Apple M1 기준: 캠페인 초당 51~60프레임, Zombies 57~60, 봇과의 멀티플레이어 53~60." width="100%">
</picture>

# bo2-mac

**Call of Duty: Black Ops II를 32비트 x86 Windows에서 네이티브 ARM64 macOS로 정적 재컴파일.**<br>
캠페인, Zombies, 봇과의 멀티플레이어를 본인의 Steam 사본으로 Mac에서 직접 빌드합니다.

<a href="README.md">English</a> · <a href="README.fr.md">Français</a> · <a href="README.es.md">Español</a> · <a href="README.pt-BR.md">Português</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.ru.md">Русский</a> · <a href="README.ja.md">日本語</a> · <b>한국어</b> · <a href="README.zh.md">中文</a>

<a href="#설치"><img src="docs/assets/btn-play.svg" alt="플레이하기" height="40"></a>&nbsp;
<a href="#결과"><img src="docs/assets/btn-results.svg" alt="결과" height="40"></a>&nbsp;
<a href="#파이프라인"><img src="docs/assets/btn-pipeline.svg" alt="파이프라인" height="40"></a>&nbsp;
<a href="#발견-사항"><img src="docs/assets/btn-findings.svg" alt="발견 사항" height="40"></a>&nbsp;
<a href="#로드맵"><img src="docs/assets/btn-roadmap.svg" alt="로드맵" height="40"></a>

<a href="https://news.ycombinator.com/item?id=50449891"><img src="docs/assets/btn-hackernews.svg" alt="Hacker News에 소개됨" height="44"></a>

![source: x86-32 Windows](https://img.shields.io/badge/source-x86--32%20Windows-ff6a00?style=flat-square&labelColor=0b0b0c)
![target: ARM64 macOS](https://img.shields.io/badge/target-ARM64%20macOS-ff6a00?style=flat-square&labelColor=0b0b0c)
![lifter: Remill, LLVM 17](https://img.shields.io/badge/lifter-Remill%20%E2%86%92%20LLVM%2017-ff6a00?style=flat-square&labelColor=0b0b0c)
![renderer: D3D11 to Metal](https://img.shields.io/badge/renderer-D3D11%20%E2%86%92%20Metal-ff6a00?style=flat-square&labelColor=0b0b0c)
![Wine: none](https://img.shields.io/badge/Wine-none-ff6a00?style=flat-square&labelColor=0b0b0c)
![game files in this repository: none](https://img.shields.io/badge/game%20files-none-f4f1ea?style=flat-square&labelColor=0b0b0c)
[![latest release](https://img.shields.io/github/v/release/adrbn/bo2-mac?style=flat-square&labelColor=0b0b0c&color=ff6a00&label=release)](https://github.com/adrbn/bo2-mac/releases/latest)
![status: beta](https://img.shields.io/badge/status-beta-f4f1ea?style=flat-square&labelColor=0b0b0c)

<sub>[왜 만들었나](#왜-만들었나) · [개요](#개요) · [방법론](#방법론) · [한계](#한계) · [저장소](#저장소) · [법적 고지](#법적-고지)</sub><br>
<sub>베타, 2026-11-03 · Apple M1 한 대에서 측정 (성능 코어 4개 + 효율 코어 4개, 16 GB)</sub>

</div>

<p align="center"><a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4"><img src="docs/assets/preview.gif" alt="macOS에서 네이티브로 실행되는 캠페인, Zombies, 멀티플레이어" width="560"></a><br>
<sub>Apple M1에서 플레이하며 녹화한 캠페인(Angola), Zombies(Town), 멀티플레이어(Hijacked, 봇 상대). 오른쪽 위 모서리의 숫자는 초당 프레임 수이며, 화면 녹화 자체가 어느 정도 성능을 소모합니다(특히 캠페인에서). <a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4">80초 영상 (MP4, 20 MB)</a></sub></p>

> [!IMPORTANT]
> **이 저장소에는 게임 파일도, 변환된 게임 코드도 들어 있지 않습니다.** 본인이 가진 Black Ops II의 Steam 사본을
> Mac에서, 본인의 Mac용 앱으로 바꿔 줄 뿐입니다. 게임 코드는 그 앱 안에 들어가게 되므로 완성된 앱을 누군가가
> 건네줄 수는 없습니다. 각 플레이어가 직접 빌드해야 하며, 공유해서는 안 됩니다. 비공식 팬 프로젝트이며
> Activision, Treyarch, Microsoft와 관련이 없습니다. 플레이하려면 게임을 구매하세요. 베타: [무엇이 입증되었고 무엇이 아직인지](docs/PRERELEASE.md) 확인하세요.

---

<img src="docs/assets/rule-01.svg" alt="" width="100%">

## 왜 만들었나

Black Ops II는 Mac으로 출시된 적이 없으며, Apple Silicon에서는 이전까지 제대로 플레이할 수 없었습니다.
[AppleGamingWiki](https://www.applegamingwiki.com/wiki/Call_of_Duty:_Black_Ops_II)는 CrossOver에서는 플레이 불가,
Rosetta 2에서는 지원되지 않는 것으로 분류합니다. 남는 것은 Parallels의 Windows ARM 가상 머신인데, 보고된 성능은
M1 Max의 매치에서 초당 1프레임부터 M3와 M4의 60~80까지 다양합니다
([MacGamingDB](https://macgamingdb.app/games/call-of-duty-black-ops-ii)). 여기서는 게임이 시작되기 전에 한 번 네이티브
ARM64 앱으로 번역됩니다. 기본형 M1에서 초당 51~60프레임이며, Windows도 가상 머신도 필요 없습니다. 네이티브라는 것은
다른 Mac 앱처럼 수정하고 확장할 수 있다는 뜻이기도 합니다. PC 버전에는 없었던 분할 화면은
Zombies의 실험적 옵션으로 들어 있습니다.

## 개요

bo2-mac은 Black Ops II의 실행 파일 세 개, 즉 캠페인(`t6sp.exe`), Zombies(`t6zm.exe`), 멀티플레이어(`t6mp.exe`)를
받아 x86 기계어를 사전(ahead-of-time) 방식으로 ARM64 기계어로 바꿉니다.
[Remill](https://github.com/lifting-bits/remill)이 각 함수를 LLVM IR로 리프팅하고, 자체 패스 체인이 그 IR을 다시 쓰며,
결과물은 직접 작성한 Win32 런타임과 링크됩니다. 이 런타임의 Direct3D 11은 Metal로 그려집니다. Wine도 JIT도
없습니다. 게임은 단일 프로세스에서 네이티브 코드로 실행됩니다. 세 실행 파일에 걸쳐 약 150,000개의 함수가 번역됩니다.
복사 방지가 숨기는 짧은 시퀀스와 번역에서 빠진 코드만 작은 x86 인터프리터에서 실행되므로, 구멍이 생기더라도
게임이 죽는 대신 속도가 떨어지도록 만들어져 있습니다.

Apple M1에서 캠페인은 **초당 51~60프레임**으로 실행되며, 모든 미션이 로드되고 다음 미션으로 넘어가 엔딩 크레딧까지
이어집니다. Zombies는 게임 자체 메뉴를 통해 **여섯 개 맵**에서 솔로로 **57~60**에 플레이됩니다.
멀티플레이어는 실제 메뉴를 통해 열한 가지 게임 모드에서 봇과 **53~60**으로 플레이됩니다. 앱 하나,
`Black Ops II.app`이 캠페인을 열고, 각 메뉴의 CAMPAIGN / ZOMBIES / MULTIPLAYER 항목이 다른 게임으로 전환합니다.

당신 외에는 누구에게서도 아무것도 내려받지 않습니다. 변환기는 당신이 구매한 사본의 실행 파일을 읽고, 바이트 단위로
검사한 뒤, 당신의 기기에서 앱을 빌드합니다.

<img src="docs/assets/rule-02.svg" alt="" width="100%">

## 결과

| | 측정값 | 조건 |
|---|---|---|
| 캠페인 | **51–60 fps** | 모든 미션이 로드되어 엔딩 크레딧까지 이어짐. 전부를 처음부터 끝까지 플레이해 본 것은 아님 |
| Zombies | **57–60 fps** (Die Rise 43–58) | TranZit, Mob of the Dead, Nuketown, Origins, Buried, Die Rise, 솔로, 실제 메뉴를 통해 |
| 봇과의 멀티플레이어 | **53–60 fps** | 실제 메뉴를 통한 열한 가지 게임 모드: 한 라운드 전체, 스코어보드, 로비 복귀 |
| 실제 시간 대비 게임 세계 | **≈ 1.0×** | 게임 자체 서버 시계 대 실제 경과 시간 |
| 플레이어의 Mac에서의 변환 | M1에서 **약 1시간 45분** (캠페인만: 약 45분) | 캠페인은 변환되는 즉시 플레이 가능하며 Zombies와 멀티플레이어가 뒤따름 |
| 디스크 | 최대 약 **10 GB**, 약 **4 GB** 유지 | 변환 후에는 링크된 바이너리만 남음 |

프레임 레이트는 화면에 실제로 표시된 드로어블 수를 센 것이며, 화면과 Metal HUD로 대조 확인했습니다.

**플레이할 수 있는 것**

<table>
<tr>
<td width="33%" valign="top"><img src="docs/assets/card-public-campaign.svg" alt="캠페인" width="100%"><br>
실제 메인 메뉴: NEW STORY, RESUME STORY, REPLAY MISSION. <code>angola</code>부터 엔딩 크레딧까지 미션이 로드되고
다음 미션으로 넘어갑니다. Strike Force 미션은 클릭 한 번으로 시작합니다.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-zombies.svg" alt="Zombies" width="100%"><br>
여섯 개 맵, 솔로: 메인 메뉴, 로비, 매치, Game Over, 스코어보드, 그리고 같은 프로세스에서 또 다른 매치.
DLC 맵은 Steam 계정이 보유하고 있으면 플레이할 수 있습니다. 2인 분할 화면은 실험적입니다.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-multi.svg" alt="멀티플레이어" width="100%"><br>
실제 메뉴에서 시작하는 봇 상대 Custom Games: CHOOSE CLASS, HUD, 한 라운드 전체, 스코어보드,
그리고 로비 복귀. 매치 규칙은 이 프로젝트 자체의 스크립트입니다.</td>
</tr>
</table>

세 모드 모두: 키보드와 마우스, 또는 GameController.framework를 통한 XInput 방식의 DualSense. 사운드 지원. 이동
키는 키보드상의 위치를 따릅니다(AZERTY에서는 ZQSD, QWERTY에서는 WASD). 게임 옵션에서 직접 지정한 경우는 예외입니다.

<img src="docs/assets/rule-03.svg" alt="" width="100%">

## 파이프라인

다섯 단계이며 모두 플레이어의 Mac에서 이루어집니다.

1. **검사.** 변환기는 각 실행 파일의 정확한 Steam 빌드만 SHA-256으로 확인해 받아들입니다. 런처는
   Mac의 Steam 클라이언트에 로그인한 계정이 게임을 보유하고 있는지 물어보고, 그렇지 않으면 거부합니다.
2. **목록화와 리프팅.** Ghidra가 당신의 실행 파일에서 함수 목록을 만들고, Remill v6.0.1이 각 함수를
   LLVM IR로 리프팅합니다. 리프터에 대한 패치 다섯 개가 [tools/remill-patches/](tools/remill-patches/)에 들어 있으며,
   그중에는 x87 반올림 모드 패치와, 함수마다 x86 시맨틱을 다시 로드하는 대신 실행 중인 Remill를 포크하는 패치가 있습니다.
3. **폴딩.** Remill의 IR은 정확하지만 느립니다. 게스트 메모리 접근, 플래그, x87 push마다 호출이거나 `State`
   구조체에 대한 store입니다. 우리의 패스가 `clang -O2` 전에 이를 다시 씁니다. 게스트의 읽기와 쓰기는 인라인되고,
   `lock` 시퀀스는 하드웨어 atomic이 되며, 게스트의 `ret`은 리프팅된 코드 안에 머물고, x87 스택은 로컬 변수로
   승격됩니다.
4. **런타임.** [runtime/](runtime/)은 게임이 실제로 호출하는 Windows 임포트를 정확한 stdcall 인자 개수로
   구현합니다(Steam API는 Valve의 공개 헤더에서 가져왔습니다). 게스트 메모리는 4 GiB짜리 매핑 하나입니다. Direct3D 11은
   Metal 위의 가짜 COM 레이어이고, DXBC 셰이더는 [HLSLcc](third_party/HLSLcc)를 거칩니다. XAudio2는 CoreAudio로
   연결되며, 영상은 libvpx로 네이티브 디코딩합니다.
5. **스레드.** 게스트 스레드는 호스트 스레드 하나 위의 코루틴으로 시작하는데, 이로써 비결정성의 대부분이 사라지고
   실행을 재현할 수 있게 됩니다. 고속 구성에서는 서버, 렌더 백엔드, 사운드 믹싱과 디코딩, 잡 워커를 실제 호스트 스레드로 옮깁니다. ARM은 TSO가 아니므로 이를 위해 실제 메모리 배리어가 필요했습니다.

**복사 방지는 우회가 아니라 실행됩니다.** 게임은 무결성 검사를 하드웨어 예외, 싱글 스텝 트랩,
힙에 복호화해 놓은 코드 뒤에 숨깁니다. Unicorn 위에 만든 작은 x86 머신
([x86_guest_vm.cpp](runtime/win32/x86_guest_vm.cpp), [heap_code.cpp](runtime/win32/heap_code.cpp))이 바로 그
시퀀스만 실행하고 제어를 재컴파일된 코드에 돌려줍니다. 검사 결과를 강제로 바꾸지 않으며, 키를 손으로 심지도 않습니다.

**번역의 구멍은 크래시가 아니라 속도 손실로 이어지도록 만들어져 있습니다.** 게임이 한 번도 리프팅되지 않은 x86 코드에 도달하면
같은 머신이 원래 바이트를 실행하고([interp.cpp](runtime/win32/interp.cpp)), 다음에 만나는 번역된 블록, 호출,
리턴에서 제어를 재컴파일된 코드에 돌려줍니다. 인터프리트된 주소는 모두 로그에 남습니다(`[INTERP]`).

**온라인 서비스는 로컬에서 응답합니다.** Zombies와 멀티플레이어는 Demonware를 기대합니다. 게임 자체의 요청을 바탕으로 작성한
응답기([dw_local.cpp](runtime/win32/dw_local.cpp))가 로비, 스토리지, 티켓을 당신의 Mac에서 처리합니다.
기기 밖으로 나가는 것은 없습니다.

<details>
<summary>왜 Wine, CrossOver, DXVK, DXMT가 아닌가?</summary>

이들은 실행 시점에 Windows 호출을 번역하고 x86 코드는 Rosetta 아래에 그대로 둡니다. 여기서는 x86 코드 자체가 사라집니다.
게임이 시작되기 전에 이미 ARM64 기계어입니다. DXVK와 DXMT는 Wine의 unixlib 메커니즘을 통해 네이티브 백엔드에
도달하는 PE DLL인데, 이 포트에는 Wine이 없고 Direct3D 11은 32비트 게스트 메모리 위에 올린 자체 레이어입니다. 외부에서 필요했던 것은
셰이더 번역뿐이었고, 순수 C++ 라이브러리인 HLSLcc가 이를 해결합니다.
</details>

<img src="docs/assets/rule-04.svg" alt="" width="100%">

## 발견 사항

각 항목은 화면에서 본 증상, 측정으로 밝혀낸 원인, 그리고 수정의 효과입니다.

<details>
<summary><b>열세 가지 발견</b>, x87 반올림 모드부터 명령어 하나 안의 잘못된 시작점까지</summary>


**F1 · 리프터가 x87 반올림 모드를 무시했다.** Zombies에서 엎드린 시점이 흔들렸다. `fistp`가 게임이 요청한
절사 대신 가장 가까운 값으로 반올림했고, 시점 높이의 네트워크 코덱 안에서였다. 클라이언트가 받은 높이는
8번 중 8번 틀렸고, Remill 패치와 코덱 재리프팅 후에는 8번 중 0번이었다.

**F2 · x87 컨트롤 워드의 예약 비트 하나.** 캠페인 미션 하나가 이펙트 루프에서 멈췄고, 실제 스레드를 쓸 때만 그랬다.
리프터가 컨트롤 워드의 6번 비트를 실제 하드웨어와 다르게 저장하고 있었다. Remill 한 줄 패치와 런타임 함수 네 개의
재리프팅으로 해결되었고, 속도 저하는 없었다.

**F3 · CPU 기능 플래그가 알고리즘을 바꾼다.** 우리의 `IsProcessorFeaturePresent`가 "SSE2 없음"이라고 답했고, 그래서 C
런타임이 x87 `ceil`을 택했는데, 이것이 우리 런타임에서는 가장 가까운 값으로 반올림되었다. 이후 한 이펙트가 인덱스 −1을 영원히
샘플링했고, 미션 두 개가 멈췄다.

**F4 · 끊김의 원인은 우리 자신의 로그였다.** 170~430 ms의 히치는 렌더 스레드가 `write()`에서 막혀 있던 것이었다. 우리의 로그가
거의 가득 찬 디스크의 파일로 리다이렉트되어 있었다. 이를 파이프로 자식 프로세스에 보내자 실행당 히치가
12회와 8회에서 0회와 0회로 줄었다.

**F5 · 셰이더 번역이 상수를 4바이트 옮겼다.** HLSLcc가 `float` 뒤의 `float2`를 오프셋 24에 두었지만 Direct3D는 20에서
읽는다. 2,664개 셰이더 중 31개가 영향을 받았고, 그중에는 눈과 조준점이 있었다.

**F6 · 보호 루틴은 실행되어야 한다.** 옵션 메뉴가 8번 중 8번 멈췄다. 무결성 검사가 자신의 리턴
주소를 힙에 복호화된 코드를 가리키도록 덮어쓰는데, 런타임이 이를 무시했다. Windows처럼 그 코드를 실행하자
3번 중 3번 멈춤 없이 넘어갔다.

**F7 · 장애물은 우리 자신의 안전장치였다.** 우리의 가드 하나가 플레이 28초 시점에 게임을 멈췄다. 프로젝트에서 가장 큰
성능 향상은 영리한 코드가 아니라 우리 자신의 가드와 false sharing을 제거한 데서 나왔다.

**F8 · 비어 있는 API 스텁은 망가진 서비스처럼 보인다.** 아무것도 반환하지 않던 스텁 두 개(`CryptGenRandom`,
`StringFromGUID2`)가 Zombies에서 네트워크 장애처럼 보였다.

**F9 · 스레드 우선순위도 프로그램의 일부다.** `SetThreadPriority`는 스텁이었다. 믹서의 요청을
macOS에 전달하자 한 번의 실행에서 짧은 오디오 버퍼가 908~1872개에서 7~71개로 줄었고, 사운드 끊김도 함께 줄었다.

**F10 · 네이티브 스키닝, 비트 단위로 동일.** NEON으로 다시 쓴 CPU 스키닝은 원래 x86과 같은 비트를 내며, 무거운 장면을
17.8에서 30.3 fps로 끌어올렸다.

**F11 · 데칼에는 게임의 depth bias가 필요하다.** 그을음 자국과 핏자국이 카메라를 약간만 돌려도 나타났다 사라졌다.
게임은 데칼에 depth bias를 요청하는데, 우리 레이어는 0으로 두고 있었다. 게임의 값을 Metal에 전달하자 해결되었다.

**F12 · 렌더 스레드가 매 프레임 레벨 지오메트리를 복사하고 있었다.** 멀티플레이어와 Zombies에서 프레임당 23~63 MB의
정점 데이터를 GPU로 복사하고 있었고, 대부분이 정적이었다. 정적 버퍼를 Metal에 상주시키자 복사량이
프레임당 3.5~5 MB로 줄었다. 멀티플레이어 51.8 → 60 fps, Zombies 43.1 → 58.2. 같은 변경이 캠페인에서는 아무 효과가 없는데,
캠페인의 속도는 스케줄러 스레드가 정하기 때문이다. 속도 향상은 그 병목을 건드리는 모드에서만 도움이 된다.

**F13 · 명령어 하나 안의 잘못된 시작점이 전투 중 프레임 레이트를 절반으로 만들었다.** 새로 받은 Steam 사본으로 처음 변환했을 때,
플레이어가 사격할 때마다 Zombies가 25 fps로 떨어졌다. 뜨거운 함수들이 인터프리터에서 실행되고 있었다.
목록화 단계가 다음 함수의 시작점을 루트의 첫 번째 명령어 안에 놓았고, 그래서 범위가 제한된 스윕이
0바이트를 디코딩해 루트를 아무 말 없이 버렸다. 하나씩 리프팅하는 리프터가 이미 하던 대로 그런 루트를 범위 제한 없이 스윕하자
번역된 코드 안으로 되돌아왔다.

</details>

<img src="docs/assets/rule-05.svg" alt="" width="100%">

## 방법론

**예측하고, 그다음 측정한다.**

- 결과를 읽기 전에 예측을 먼저 적습니다. 이 베타 시점까지 번호가 붙은 예측이 약 1,100개입니다.
- 구성당 최소 세 번 실행합니다. 실행 간 잡음이 30~64%이므로, n = 3에서는 약 40% 미만의 차이는 볼 수 없습니다.
- 변경은 자신이 활성 상태임을 증명해야 합니다. 로그에 그것이 동작했다는 흔적이 보여야 합니다.
- 레벨에 도달하지 못한 실행은 데이터 포인트가 아니라 죽은 실행입니다.
- 화면이 로그보다 우선합니다. 화면이 프로브와 모순되면 틀린 것은 프로브입니다.

<details>
<summary><b>기록과 재생, 철회, 클린룸, 누가 무엇을 하는가</b></summary>

**결정론적 기록과 재생.** 모든 게스트 스레드가 호스트 스레드 하나 위의 코루틴이므로 비결정성의 대부분이 사라집니다. 재생은 가정하지 않고
기록과 대조해 검증합니다. 세 경계가 외부 입력을 모두 포착합니다. 임포트 디스패처, 윈도우 메시지 드레인, 그리고
소비된 오디오 버퍼입니다. 저널은 약 16 MB입니다. 플래그를 정확히 하나만 바꾸고 같은 실행을 두 번 돌릴 수 있게 해 줍니다.

**실패하는 것은 계측 도구다.** 그 과정에서 거의 200건의 결론을 철회했습니다. 가장 큰 비중(31%)은
다른 질문에 답하고 있던 계측 도구였고, 그다음(17%)은 아예 답할 수 없던 도구였습니다. 그래서
이곳의 모든 오라클은 자신의 입력을 명시하고, 그 입력이 없으면 답하기를 거부합니다.

**매치 규칙을 위한 클린룸.** 멀티플레이어와 Zombies는 Steam이 더 이상 배포하지 않는 일일 패치 파일에서 매치 규칙을 가져옵니다.
[runtime/match_rules/](runtime/match_rules/)의 대체물은 원본 스크립트를 한 번도 본 적 없는 팀이 작성했습니다. 한 팀이 게임을
보고 동작 명세를 쓰고, 다른 팀은 그 명세만으로 코드를 썼으며, 두 팀은 날짜와 함께 보관된 서면 질의응답으로만 소통했습니다.

**누가 무엇을 하는가.** AI 에이전트는 처리량을 제공합니다. 심(shim) 작성, 실행당 수백만 번의 호출에 대한 이분 탐색,
수만 줄의 코드 읽기입니다. 사람은 목표를 제시하고, 실제로 화면에 보이는 것을 바탕으로 바로잡고, 듣기 좋은 결론을 거부합니다.
이것은 게임 하나, 수개월의 작업, 그리고 많은 수작업 개입의 결과입니다.

</details>

<img src="docs/assets/rule-07.svg" alt="" width="100%">

## 한계

- **로컬 전용.** 멀티플레이어와 Zombies는 당신의 Mac에 있는 응답기를 상대로 실행됩니다. 온라인 플레이도 LAN도 없습니다. League Play는 거부됩니다.
- **매치 규칙은 우리 것입니다.** 클린룸에서 작성되었으며([방법론](#방법론) 참조), 일부 동작은 원본과 다를 수
  있습니다.
- **발열.** 장시간 세션 동안 Mac이 매우 뜨거워집니다(M1에서 111~116 °C 보고). 전원을 연결하고 플레이하세요. 배터리에서는
  macOS가 게임을 크게 제한합니다. 전원 연결은 속도 저하를 피하게 해 줄 뿐 발열을 막지는 못합니다.
- **각 맵의 첫 게임.** Zombies와 멀티플레이어에서는 맵이 로드되는 동안 셰이더가 Metal용으로 번역되므로
  첫 로딩이 더 길고 첫 1분 동안은 약간의 히치가 남을 수 있습니다. 캠페인에서는 셰이더가
  처음 나타날 때 번역되므로 로딩 시네마틱은 부드럽지만 어떤 장소를 처음 지날 때 히치가 있을 수 있습니다. 모든 것이
  캐시되므로 다음부터는 빠르게 로드되고 부드럽게 실행됩니다.
- **창.** 게임은 1280×720 창에서 실행됩니다.
- **다른 Mac.** 측정된 것은 M1 한 대뿐입니다. M2, M3, M4는 더 빠를 것으로 보이나 수치는 주장하지 않습니다.
- **캠페인.** 모든 미션이 로드되고 이어지지만, 전부를 처음부터 끝까지 플레이해 본 것은 아닙니다.
- **Zombies.** 온라인이나 LAN 협동이 없어 Grief와 Turned는 플레이할 수 없습니다. 2인 로컬 분할 화면은
  실험적이며, 플레이어 2는 1라운드가 시작될 때 나타납니다.
- **macOS 오디오.** 시스템의 `coreaudiod`가 비정상일 때 캠페인이 첫 영상에서 멈춥니다.
  `afplay -t 1 /System/Library/Sounds/Tink.aiff`가 약 1초 걸리는지 확인하세요. Mac을 재시작하면 해결됩니다.

<img src="docs/assets/rule-08.svg" alt="" width="100%">

## 설치

Apple 실리콘 Mac, 최소 **20 GB의 여유 공간**(변환 중 최대 약 10 GB를 쓰고 약 4 GB가 남습니다), 그리고 게임을 보유한 계정으로
로그인한 Mac용 Steam이 필요합니다.

**macOS 26 이상: 터미널 불필요.** [릴리스](https://github.com/adrbn/bo2-mac/releases)에서 `Black Ops II.app`을 내려받아
엽니다. 자체 변환 도구(LLVM 17, Remill, Unicorn, libvpx, HLSLcc, capstone,
[scripts/make_toolchain.sh](scripts/make_toolchain.sh)로 빌드)를 내장하고 있으며 게임 파일은 들어 있지 않습니다. 처음 실행하면 macOS가
Apple의 무료 Command Line Tools 설치를 제안합니다. "설치"를 클릭한 뒤 앱 창에서 "다시 시도"를 클릭하세요.
하루에 한 번 실행 시점에 앱이 새 릴리스를 확인하고 설치를 제안합니다. "설치, 이 버전 유지"를 선택하면
현재 앱이 버전 이름으로 바뀌어 옆에 남으므로, 그것을 열면 이전 버전으로 돌아갑니다.업데이트는 이미 변환된 것을 그대로 두고 변경이 닿는 부분만 다시 처리합니다. 런타임 수정이라면 M1에서 세 모드를 다시 링크하는 데 약 3분이 걸립니다.

**macOS 14 또는 15: 터미널에서**, Homebrew를 사용합니다.

```bash
git clone https://github.com/adrbn/bo2-mac.git && cd bo2-mac
brew install llvm@17 cmake ninja libvpx unicorn
pip3 install capstone
scripts/make_app.sh --from-source
```

`Black Ops II.app`을 열고 게임 폴더를 지정한 뒤 변환하게 두세요. M1에서 약 1시간 45분이 걸리며, 캠페인이 먼저 약 45분입니다. 캠페인은
변환되는 즉시 플레이할 수 있습니다. 그때 플레이하세요. Zombies와 멀티플레이어는 게임 메뉴에서 처음 선택할 때 변환됩니다. 그렇지 않으면
변환은 세 모드 모두에 대해 계속됩니다. 디스크 여유 공간이
8 GB 아래로 떨어지면 변환이 깔끔하게 멈추고 멈춘 지점에서 재개됩니다.

**Zombies 분할 화면 (실험적).** 컨트롤러 두 개를 연결하고, 메뉴 막대의 **Black Ops II** 메뉴를 열어
**Two-Player Split Screen in Zombies**를 선택합니다. 다음에 Zombies를 시작할 때(앱에서든 게임의 ZOMBIES 메뉴 항목에서든)
적용됩니다. 플레이어 1은 위쪽 절반을, 플레이어 2는 아래쪽 절반을 플레이하며, 플레이어 2는 1라운드가 시작될 때 나타납니다.
두 번째 컨트롤러가 없으면 게임은 1인 플레이로 남습니다. 선택을 해제하면 1인 플레이로 돌아갑니다.

**업데이트, 또는 변환이 멈췄을 때.** 저장소 폴더에서 `git pull && scripts/make_app.sh --from-source`를 실행하세요.
이미 변환된 것은 유지되고 변환이 재개됩니다. 업데이트는 변경 사항이 건드리는 부분만 다시 처리합니다. 런타임
수정이라면 다시 변환하는 대신 M1에서 약 3분이면 세 모드를 다시 링크합니다. 그래도 실패하면
로그가 가리키는 파일을 첨부해 이슈를 열어 주세요.

변환기는 정확한 Steam 빌드만 받아들이며 SHA-256으로 확인합니다. Steam 실행 파일은 Valve의
SteamStub 래퍼로 암호화되어 있고 **이 저장소는 그것을 제거하는 방법을 제공하지 않습니다**. 변환기는 복호화된 이미지 세 개가
`~/.bo2-native/steam/`에 있기를 기대합니다(`t6sp.exe.unpacked.exe`, `t6mp.exe.unpacked.exe`, `t6zm.exe.unpacked.exe`). 구매한 사본으로
직접, 원하는 별도의 도구로 만드세요. 제거해야 하는 것은 래퍼뿐이며, 위의 파일 이름과 SHA-256 검사를 보면 파일이 올바른지
즉시 알 수 있습니다. 이 파일들을 절대 공유하지 마세요.

**DLC.** 런처는 Windows에서 게임이 하는 것과 똑같이 Steam 클라이언트에 계정이 어떤 DLC를 보유하는지 물어봅니다. DLC 맵은
보유하고 있고 그 파일이 게임 폴더에 있을 때 플레이할 수 있습니다.

<img src="docs/assets/rule-09.svg" alt="" width="100%">

## 저장소

| 경로 | 담고 있는 것 |
|---|---|
| [runtime/win32/](runtime/win32/) | Win32 런타임: kernel32, 스케줄러, Direct3D 11, 사운드, 입력, 로컬 서비스 응답기 |
| [runtime/gfx/](runtime/gfx/) | Metal 백엔드: 프레젠테이션, 셰이더 캐시, 렌더 타깃 |
| [runtime/match_rules/](runtime/match_rules/) | 멀티플레이어와 Zombies를 위한 클린룸 매치 규칙 |
| [tools/](tools/) | 리프트 드라이버, 목록화, IR 패스, 디스어셈블리 및 분석 도구 |
| [tools/remill-patches/](tools/remill-patches/) | Remill v6.0.1 위에 얹은 패치들 |
| [scripts/](scripts/) | `make_app.sh`, Steam 설치기와 변환기, 런처 |
| [third_party/HLSLcc](third_party/HLSLcc) | DXBC에서 Metal로의 셰이더 번역 |
| [docs/](docs/) | 프리릴리스 상태, 툴체인 |

<img src="docs/assets/rule-10.svg" alt="" width="100%">

## 로드맵

1. **이번 베타**: 새로운 Steam 사본에서 빌드한 세 모드를, 빌드한 Mac이 아닌 다른 Mac에서. 여러분의 보고
   (Mac 모델, 프레임 레이트, 무엇이 망가졌는지)가 다음 순서를 정합니다.
   첫 외부 보고, 2026-11-05: M2 Pro(16 GB)에서 변환이 완료되고 게임이 시작되었으며, 그 과정에서 발견된 세 가지
   수정이 있었습니다([#4](https://github.com/adrbn/bo2-mac/issues/4), [#5](https://github.com/adrbn/bo2-mac/issues/5), [#6](https://github.com/adrbn/bo2-mac/issues/6)).
2. **터미널 없음**: 완료. 내려받아 여는 앱 하나이며, 이미 빌드된 도구를 내장하고 있어
   Mac에서 컴파일되는 것은 게임 자체뿐이고, 스스로 업데이트됩니다(macOS 26 이상. macOS 14와 15는
   터미널 설치를 유지).
3. **완전한 번역**: 아직 Unicorn 인터프리터를 거치는 46개 코드 블록을 사전에 번역하고,
   번역된 모든 명령어를 인터프리터와 대조 검사합니다.
   번역된 어떤 함수든 인터프리터로 되돌려 보낼 수 있어, 절반씩 나누어 가며 잘못된 함수를 찾아냅니다.
4. **셰이더 대기 없음**: 셰이더를 실행 시점(멀티플레이어에서 약 3분)과 플레이 중(새 이펙트가 나타날 때의 멈춤)이 아니라
   첫 매치 전에 빌드합니다. 착수함: 이제 셰이더가 배치로 컴파일되고, 최근 세션의 셰이더는 실행 시점에
   백그라운드에서 컴파일됩니다. Xcode가 없는 M1의 멀티플레이어에서(몇 번의 시도이며 벤치마크는 아님), 매치 첫 2분 동안의
   셰이더 컴파일이 첫 실행에서 18~25초에서 10~14초로, 다음 실행에서는 약 5초로 줄었습니다. 300 ms가 넘는 멈춤은
   15~22회에서 13~15회, 그다음 7회로 줄었습니다. 첫 실행에서는 여전히
   거의 같은 빈도로 멈춥니다.
5. **더 짧은 변환**: M1에서 약 2.5시간에서 1시간 45분으로 단축(2026-11-06, 세 모드 모두에서 측정:
   준비 단계가 실행 파일을 한 번만 읽고, 리프트와 폴딩은 메모리가 허용하는 모든 코어를 사용). 업데이트는 이제 바뀐 부분만
   다시 처리합니다(런타임 수정: 약 3분). 다음: 거의 실행되지 않는 코드에 대한 최적화 축소. 세 모드가 공유하는 함수는 4 %뿐이므로
   공유 코드를 한 번만 변환해서는 얻는 것이 적습니다.
6. **더 빠르게, 발열 없이**: 캠페인은 렌더링이 아니라 스케줄러 스레드가 병목이며, 봇 11명의 멀티플레이어는
   서버 스레드가 병목입니다. 시작됨: 리버브는 세 모드 모두에서 네이티브로 실행되며 사운드 스레드 시간이 약 40 % 줄었습니다. 다음은 나머지 가장 뜨거운 함수들의 네이티브 재작성이고, 그다음은 120 Hz 화면을 위한 60 초과 프레임 레이트입니다.
7. **컨트롤러**: 진동과 에임 어시스트.
8. **함께**: PC 버전에는 없었던 Zombies의 로컬 분할 화면은 실험적이며(게임 하나,
   플레이어 둘, 위아래 절반 화면, M1에서 약 28 fps), 앱 메뉴에 스위치가 있습니다. 다음: Zombies 협동, Grief와 Turned를 위한 실제 UDP 소켓,
   그리고 LAN을 통한 멀티플레이어.
9. **나중에**: MetalFX, 프레임을 표시할 때의 포스트 프로세싱, 플레이어 본인의 사본에서 불러오는 교체 텍스처.

<img src="docs/assets/rule-11.svg" alt="" width="100%">

## 법적 고지

사용자가 보유한 사본을, 사용자가 보유한 하드웨어에서 사용하는 경우를 전제로 합니다.

- 엔진 소스도 유출된 코드도 사용하지 않습니다.
- 입력 바이너리도 재컴파일된 출력물도 배포되지 않습니다. 게임 파일은 포함되어 있지 않으며, 플레이어는 자신의
  Call of Duty: Black Ops II Steam 사본이 필요합니다. 변환기는 정확한 Steam 빌드(SHA-256)만 받아들이며,
  소유 여부는 게임을 시작할 때마다 플레이어 본인의 Steam 클라이언트가 확인합니다. Steam에
  로그인되어 있지 않거나 계정이 게임을 보유하고 있지 않으면 게임은 실행을 거부합니다.
- 복사 방지는 게임이 작성한 그대로 실행되며, 그 안의 어떤 것도 패치로 제거되지 않습니다. 이 저장소는 Valve의 SteamStub 래퍼를
  제거하는 방법을 제공하지 않습니다.
- 게임이 기대하는 온라인 서비스는 게임 자체의 요청을 바탕으로 작성된 로컬 응답기가 처리합니다. Activision
  서버에는 접속하지 않으며, 온라인 플레이는 없고, 게임 소유 여부는 여전히 플레이어의 Steam이 확인합니다.
  매치 규칙은 클린룸에서 작성되었습니다.
- 여기서 작성된 코드는 GPL-3.0([LICENSE](LICENSE))으로 라이선스됩니다.
- Unicorn(GPL-2.0)은 배포되는 바이너리에서 GPL-3.0 코드와 결합되지 않습니다. 앱은 Unicorn의 수정되지 않은
  라이브러리를 다른 도구 옆에 별도 파일로 담고 있으며, 이는 플레이어 본인의 Mac에서 게임 바이너리에 링크되고
  ([tools/boot.py](tools/boot.py)), 그 바이너리는 배포되지 않습니다. 각 릴리스는 GPL-2.0이 요구하는 대로
  포함된 Unicorn 버전의 정확한 소스(`unicorn-<version>-source.tar.gz`)를 공개합니다. 내장된 모든
  도구의 라이선스는 앱의 `Contents/Resources/toolchain/licenses`에 있습니다.

이것은 비공식 팬 프로젝트입니다. Call of Duty와 Black Ops는 Activision Publishing, Inc.의 상표입니다. Steam은 Valve Corporation의
상표입니다. 이 프로젝트는 Activision, Treyarch, Microsoft, Valve와 제휴하거나 이들의 보증 또는 후원을 받지 않았습니다. 플레이하려면 게임을 구매하세요.

## 스타 히스토리

<a href="https://www.star-history.com/#adrbn/bo2-mac&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
  </picture>
</a>

<div align="center"><img src="docs/assets/footer.svg" alt="" width="100%"></div>
