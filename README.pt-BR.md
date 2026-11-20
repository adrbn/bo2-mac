<div align="center">

<picture>
  <source media="(prefers-color-scheme: light)" srcset="docs/assets/hero-public-light.svg">
  <img src="docs/assets/hero-public.svg" alt="bo2-mac: Call of Duty: Black Ops II recompilado estaticamente de x86-32 Windows para ARM64 macOS nativo, a partir da sua própria cópia da Steam. Em um Apple M1: campanha de 51 a 60 quadros por segundo, Zombies de 57 a 60, multiplayer contra bots de 53 a 60." width="100%">
</picture>

# bo2-mac

**Call of Duty: Black Ops II, recompilado estaticamente de x86 de 32 bits para Windows em ARM64 nativo para macOS.**<br>
Campanha, Zombies e multiplayer contra bots, compilados no seu Mac a partir da sua própria cópia da Steam.

<a href="README.md">English</a> · <a href="README.fr.md">Français</a> · <a href="README.es.md">Español</a> · <b>Português</b> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.ru.md">Русский</a> · <a href="README.ja.md">日本語</a> · <a href="README.ko.md">한국어</a> · <a href="README.zh.md">中文</a>

<a href="#instalação"><img src="docs/assets/btn-play.svg" alt="Jogar" height="40"></a>&nbsp;
<a href="#resultados"><img src="docs/assets/btn-results.svg" alt="Resultados" height="40"></a>&nbsp;
<a href="#pipeline"><img src="docs/assets/btn-pipeline.svg" alt="Pipeline" height="40"></a>&nbsp;
<a href="#descobertas"><img src="docs/assets/btn-findings.svg" alt="Descobertas" height="40"></a>&nbsp;
<a href="#roadmap"><img src="docs/assets/btn-roadmap.svg" alt="Roadmap" height="40"></a>

<a href="https://news.ycombinator.com/item?id=50449891"><img src="docs/assets/btn-hackernews.svg" alt="Destaque no Hacker News" height="44"></a>

![source: x86-32 Windows](https://img.shields.io/badge/source-x86--32%20Windows-ff6a00?style=flat-square&labelColor=0b0b0c)
![target: ARM64 macOS](https://img.shields.io/badge/target-ARM64%20macOS-ff6a00?style=flat-square&labelColor=0b0b0c)
![lifter: Remill, LLVM 17](https://img.shields.io/badge/lifter-Remill%20%E2%86%92%20LLVM%2017-ff6a00?style=flat-square&labelColor=0b0b0c)
![renderer: D3D11 to Metal](https://img.shields.io/badge/renderer-D3D11%20%E2%86%92%20Metal-ff6a00?style=flat-square&labelColor=0b0b0c)
![Wine: none](https://img.shields.io/badge/Wine-none-ff6a00?style=flat-square&labelColor=0b0b0c)
![game files in this repository: none](https://img.shields.io/badge/game%20files-none-f4f1ea?style=flat-square&labelColor=0b0b0c)
[![latest release](https://img.shields.io/github/v/release/adrbn/bo2-mac?style=flat-square&labelColor=0b0b0c&color=ff6a00&label=release)](https://github.com/adrbn/bo2-mac/releases/latest)
![status: beta](https://img.shields.io/badge/status-beta-f4f1ea?style=flat-square&labelColor=0b0b0c)

<sub>[Por quê](#por-quê) · [Resumo](#resumo) · [Método](#método) · [Limites](#limites) · [Repositório](#repositório) · [Aviso legal](#aviso-legal)</sub><br>
<sub>Beta, 03/11/2026 · medido em um único Apple M1 (4 núcleos de desempenho + 4 de eficiência, 16 GB)</sub>

</div>

<p align="center"><a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4"><img src="docs/assets/preview.gif" alt="Campanha, Zombies e multiplayer rodando nativamente no macOS" width="560"></a><br>
<sub>Campanha (Angola), Zombies (Town) e multiplayer (Hijacked, contra bots) em um Apple M1, gravados durante o jogo; o número no canto superior direito é o de quadros por segundo, e a própria gravação de tela custa alguns, principalmente na campanha. <a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4">Vídeo de 80 segundos (MP4, 20 MB)</a></sub></p>

> [!IMPORTANT]
> **Este repositório não contém nenhum arquivo do jogo nem código do jogo traduzido.** Ele transforma a sua própria cópia
> do Black Ops II na Steam em um app para o seu Mac, no seu Mac. O código do jogo acaba dentro desse app, então ninguém
> pode lhe entregar um pronto: cada jogador compila o seu, e ele não pode ser compartilhado. Projeto de fã não oficial,
> sem qualquer vínculo com a Activision, a Treyarch ou a Microsoft. Para jogar, compre o jogo. Beta: leia [o que foi e o que não foi provado](docs/PRERELEASE.md).

## Novidades da v0.2.1

- **Tela dividida no Zombies pelo menu do app**: marque *Two-Player Split Screen in Zombies* no menu **Black Ops II**,
  conecte dois controles, e a próxima partida de Zombies começa com dois jogadores, metades superior e inferior. Sem Terminal.

## Novidades da v0.2.0

A [v0.2.0](https://github.com/adrbn/bo2-mac/releases/tag/v0.2.0), de 19/11/2026, é a primeira versão que não é uma build de teste.

- **Um app, sem Terminal** (macOS 26 ou posterior): o `Black Ops II.app` traz suas próprias ferramentas de tradução,
  encontra a pasta do seu jogo, verifica os seus três executáveis e mostra a conversão em uma janela.
- **Ele se atualiza sozinho**: ao abrir, oferece uma versão mais recente, verifica se ela está assinada e notarizada e a instala.
  Uma atualização refaz apenas o que mudou: uma correção de runtime religa os três modos em cerca de 3 minutos em um M1.
- **Zombies**: o menu principal não fica mais preto.
- **Os mapas de DLC seguem a sua posse na Steam**: um pacote de mapas que você possui é oferecido mesmo que a Steam não tenha
  baixado seus arquivos, e um que você não possui continua bloqueado.
- **Reverb nativo** nos três modos: cerca de 40 % menos tempo no reverb na thread de som.
- **Experimental: tela dividida no Zombies**, dois jogadores em um Mac, um controle para cada, metades superior e inferior,
  cerca de 28 fps em um M1. Desativada por padrão; a v0.2.1 acrescenta um interruptor para ela no menu do app (veja [Instalação](#instalação)).

Se você vem de uma versão anterior, a primeira abertura converte o jogo inteiro de novo, uma vez (cerca de 2,5 horas em um M1).

---

<img src="docs/assets/rule-01.svg" alt="" width="100%">

## Por quê

O Black Ops II nunca saiu para Mac e, no Apple Silicon, não era realmente jogável até agora.
A [AppleGamingWiki](https://www.applegamingwiki.com/wiki/Call_of_Duty:_Black_Ops_II) o lista como injogável no
CrossOver e não suportado no Rosetta 2. Resta uma máquina virtual Windows ARM no Parallels, com relatos que
vão de 1 quadro por segundo em partidas em um M1 Max a 60–80 em M3 e M4
([MacGamingDB](https://macgamingdb.app/games/call-of-duty-black-ops-ii)). Aqui o jogo é traduzido uma vez, antes de
iniciar, para um app ARM64 nativo: 51–60 quadros por segundo em um M1 básico, sem Windows e sem máquina virtual. Ser
nativo também significa que ele pode ser corrigido e estendido como qualquer app de Mac: a tela dividida, que a versão
de PC nunca teve, está na [v0.2.0](https://github.com/adrbn/bo2-mac/releases/latest) como opção experimental no Zombies.

## Resumo

O bo2-mac pega os três executáveis do Black Ops II, a campanha (`t6sp.exe`), o Zombies (`t6zm.exe`) e o
multiplayer (`t6mp.exe`), e transforma o código de máquina x86 deles em código de máquina ARM64 com antecedência.
O [Remill](https://github.com/lifting-bits/remill) eleva cada função a LLVM IR, uma cadeia de passes nossos reescreve
esse IR, e o resultado é ligado a um runtime Win32 escrito à mão, cujo Direct3D 11 é desenhado pelo Metal. Não há
Wine nem JIT: o jogo roda como código nativo, em um único processo. Cerca de 150.000 funções são traduzidas nos
três executáveis. Apenas as curtas sequências que a proteção contra cópia esconde rodam em um pequeno interpretador x86, assim como
qualquer código que a tradução tenha perdido, de modo que um buraco foi feito para custar velocidade em vez de derrubar o jogo.

Em um Apple M1, a campanha roda a **51–60 quadros por segundo**, e cada missão carrega e passa para a
seguinte, até os créditos finais. O Zombies é jogado solo em **seis mapas** pelos próprios menus do jogo a **57–60**.
O multiplayer é jogado contra bots pelos seus menus reais, onze modos de jogo, a **53–60**. Um único app,
o `Black Ops II.app`, abre a campanha, e as entradas CAMPAIGN / ZOMBIES / MULTIPLAYER de cada menu alternam para o
outro jogo.

Nada é baixado de ninguém além de você: o conversor lê os executáveis da cópia que você comprou, verifica-os
byte a byte e compila o app na sua máquina.

<img src="docs/assets/rule-02.svg" alt="" width="100%">

## Resultados

| | medido | condições |
|---|---|---|
| Campanha | **51–60 fps** | cada missão carrega e encadeia até os créditos finais; nem todas foram jogadas do início ao fim |
| Zombies | **57–60 fps** (Die Rise 43–58) | TranZit, Mob of the Dead, Nuketown, Origins, Buried, Die Rise, solo, pelos menus reais |
| Multiplayer contra bots | **53–60 fps** | onze modos de jogo pelos menus reais: uma rodada completa, o placar, de volta ao lobby |
| Mundo do jogo contra o tempo real | **≈ 1,0×** | o relógio do servidor do próprio jogo contra o relógio de parede |
| Conversão no Mac do jogador | **cerca de 1 hora e 45** em um M1 (só a campanha: cerca de 45 minutos) | a campanha é jogável assim que é convertida; Zombies e multiplayer vêm em seguida |
| Disco | pico de cerca de **10 GB**, cerca de **4 GB** mantidos | após a conversão, ficam apenas os binários ligados |

A taxa de quadros conta os drawables realmente apresentados na tela, conferidos com a tela e com o Metal HUD.

**O que você pode jogar**

<table>
<tr>
<td width="33%" valign="top"><img src="docs/assets/card-public-campaign.svg" alt="Campanha" width="100%"><br>
O menu principal real: NEW STORY, RESUME STORY, REPLAY MISSION. De <code>angola</code> até os créditos finais, as
missões carregam e passam para a seguinte. As missões Strike Force começam com um clique.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-zombies.svg" alt="Zombies" width="100%"><br>
Os seis mapas, solo: menu principal, lobby, partida, Game Over, placar e depois outra partida no mesmo processo.
Os mapas de DLC rodam se a sua conta Steam os possuir. A tela dividida para dois jogadores é experimental.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-multi.svg" alt="Multiplayer" width="100%"><br>
Custom Games contra bots pelos menus reais: CHOOSE CLASS, o HUD, uma rodada completa, o placar
e de volta ao lobby. As regras da partida são um script próprio deste projeto.</td>
</tr>
</table>

Nos três modos: teclado e mouse, ou um DualSense como XInput pelo GameController.framework; som ativado; as teclas de
movimento seguem a posição delas no seu teclado (ZQSD em AZERTY, WASD em QWERTY), a menos que você mesmo as tenha
configurado nas opções do jogo.

<img src="docs/assets/rule-03.svg" alt="" width="100%">

## Pipeline

Em cinco etapas, todas no Mac do jogador:

1. **Verificação.** O conversor aceita apenas a build exata da Steam de cada executável, verificada por SHA-256. O
   launcher pergunta ao cliente Steam do seu Mac se a conta conectada possui o jogo, e recusa caso contrário.
2. **Catálogo e elevação.** O Ghidra lista as funções do seu próprio executável; o Remill v6.0.1 eleva cada uma a
   LLVM IR. Cinco patches do lifter estão em [tools/remill-patches/](tools/remill-patches/), entre eles o
   modo de arredondamento do x87, e um que faz fork de um Remill já em execução para cada função em vez de recarregar suas semânticas x86.
3. **Dobra.** O IR do Remill é correto, mas lento: cada acesso à memória do guest, cada flag e cada push do x87 é uma chamada ou uma escrita
   em uma estrutura `State`. Nossos passes o reescrevem antes do `clang -O2`: leituras e escritas do guest são inlinadas, as sequências
   `lock` viram atômicas de hardware, o `ret` do guest permanece dentro do código elevado e a pilha x87 é promovida a
   variáveis locais.
4. **Runtime.** O [runtime/](runtime/) implementa as importações do Windows que o jogo realmente chama, com aridades
   stdcall exatas (as da API da Steam tiradas dos headers públicos da Valve). A memória do guest é um único mapeamento de 4 GiB. O Direct3D 11 é
   uma camada fake-COM sobre o Metal; os shaders DXBC passam pelo [HLSLcc](third_party/HLSLcc). O XAudio2 vai para o CoreAudio, e
   os vídeos são decodificados nativamente com libvpx.
5. **Threads.** As threads do guest começam como corrotinas em uma única thread do host, o que elimina a maior parte das fontes de
   não determinismo e torna uma execução reproduzível. A configuração rápida move o servidor, o backend de renderização, a mixagem e a decodificação de som e os workers de jobs
   para threads reais do host. O ARM não é TSO, então isso exigiu barreiras de memória reais.

**A proteção contra cópia é executada, não contornada.** O jogo esconde verificações de integridade atrás de exceções de hardware,
traps de passo único e código que ele decripta no heap. Uma pequena máquina x86 construída sobre o Unicorn
([x86_guest_vm.cpp](runtime/win32/x86_guest_vm.cpp), [heap_code.cpp](runtime/win32/heap_code.cpp)) executa exatamente essas
sequências e devolve o controle ao código recompilado. Nenhum resultado de verificação é forçado e nenhuma chave é colocada à mão.

**Um buraco na tradução foi feito para custar velocidade, não uma queda.** Quando o jogo chega a código x86 que nunca foi elevado, a
mesma máquina executa seus bytes originais ([interp.cpp](runtime/win32/interp.cpp)) e devolve o controle ao código
recompilado no próximo bloco traduzido, chamada ou retorno. Cada endereço interpretado é registrado no log (`[INTERP]`).

**O serviço online é respondido localmente.** O Zombies e o multiplayer esperam o Demonware; um responder escrito a partir das
requisições do próprio jogo ([dw_local.cpp](runtime/win32/dw_local.cpp)) responde ao lobby, ao armazenamento e ao ticket no seu Mac.
Nada sai da máquina.

<details>
<summary>Por que não Wine, CrossOver, DXVK ou DXMT?</summary>

Eles traduzem chamadas do Windows em tempo de execução e mantêm o código x86 sob o Rosetta. Aqui o próprio código x86 desaparece: ele é
código de máquina ARM64 antes de o jogo iniciar. DXVK e DXMT são DLLs PE que alcançam seu backend nativo pelo mecanismo
unixlib do Wine; este port não tem Wine, e seu Direct3D 11 é uma camada própria sobre a memória guest de 32 bits. Só a tradução
de shaders precisou vir de fora, e o HLSLcc, uma biblioteca C++ simples, a cobre.
</details>

<img src="docs/assets/rule-04.svg" alt="" width="100%">

## Descobertas

Cada entrada é um sintoma visto na tela, a causa que foi medida e o efeito da correção.

<details>
<summary><b>Treze descobertas</b>, do modo de arredondamento do x87 a uma falsa largada dentro de uma única instrução</summary>


**F1 · O lifter ignorava o modo de arredondamento do x87.** No Zombies, a visão deitado tremia. O `fistp` arredondava para o mais próximo onde
o jogo havia pedido truncamento, dentro do codec de rede da altura da visão. A altura recebida pelo cliente estava
errada em 8 amostras de 8, e em 0 de 8 após um patch no Remill e uma nova elevação do codec.

**F2 · Um bit reservado da palavra de controle do x87.** Uma missão da campanha travava em um loop de efeitos, apenas com
threads reais. O lifter armazenava o bit 6 da palavra de controle de forma diferente do hardware real. Um patch de uma linha no Remill e quatro
funções do runtime reelevadas resolveram, sem perda de velocidade.

**F3 · Uma flag de recurso da CPU muda o algoritmo.** Nosso `IsProcessorFeaturePresent` respondia "sem SSE2", então o runtime
de C usava seu `ceil` em x87, que arredondava para o mais próximo sob o nosso runtime. Um efeito então amostrava o índice −1 para sempre: duas
missões travavam.

**F4 · Nosso próprio log causava a gagueira.** Os engasgos de 170–430 ms eram a thread de renderização bloqueada em `write()`: nosso log,
redirecionado para um arquivo em um disco quase cheio. Enviá-lo por um pipe para um processo filho reduziu os engasgos por execução
de 12 e 8 para 0 e 0.

**F5 · A tradução de shaders deslocava constantes em 4 bytes.** O HLSLcc colocava um `float2` depois de um `float` no offset 24, onde
o Direct3D o lê em 20. 31 de 2.664 shaders foram afetados, entre eles os olhos e as miras.

**F6 · A proteção precisa rodar.** O menu de opções travava 8 vezes de 8. Uma verificação de integridade reescreve seu próprio endereço
de retorno para apontar para código decriptado no heap, o que o runtime ignorava. Executar esse código, como o Windows faz, deu
3 de 3 sem travamento.

**F7 · Nossas próprias salvaguardas eram o obstáculo.** Uma salvaguarda nossa parava o jogo aos 28 segundos de partida. Os maiores
ganhos do projeto vieram de remover nossas próprias salvaguardas e o false sharing, não de código engenhoso.

**F8 · Um stub de API vazio parece um serviço quebrado.** Dois stubs que não retornavam nada (`CryptGenRandom`,
`StringFromGUID2`) pareciam uma falha de rede no Zombies.

**F9 · As prioridades de thread fazem parte do programa.** `SetThreadPriority` era um stub. Repassar o pedido do mixer ao
macOS reduziu os buffers de áudio curtos de uma execução de 908–1872 para 7–71, e as falhas de som junto com eles.

**F10 · Skinning nativo, bit a bit.** O skinning em CPU reescrito em NEON dá os mesmos bits que o x86 original e levou
uma cena pesada de 17,8 para 30,3 fps.

**F11 · Decals precisam do depth bias do jogo.** Marcas de queimadura e sangue apareciam ou sumiam com um leve giro da
câmera. O jogo pede um depth bias nos decals; nossa camada o deixava em zero. Repassar os valores do jogo ao Metal resolveu.

**F12 · A thread de renderização copiava a geometria do nível a cada quadro.** No multiplayer e no Zombies, ela copiava 23–63 MB
de dados de vértices para a GPU por quadro, a maior parte estática. Manter os buffers estáticos residentes no Metal reduziu as cópias para
3,5–5 MB por quadro: multiplayer 51,8 → 60 fps, Zombies 43,1 → 58,2. A mesma mudança não faz nada na campanha,
onde o ritmo é ditado pela thread do scheduler. Um ganho de velocidade só ajuda o modo cujo gargalo ele toca.

**F13 · Uma falsa largada dentro de uma instrução custou metade da taxa de quadros em combate.** Nas primeiras conversões a partir de uma
cópia nova da Steam, o Zombies caía para 25 fps sempre que o jogador atirava. Funções quentes estavam rodando no interpretador: o
catálogo colocava o início da função seguinte dentro da primeira instrução de uma raiz, de modo que sua varredura limitada
decodificava zero bytes e a raiz era descartada sem uma palavra. Varrer essas raízes sem o limite, como o lifter
um a um já fazia, as devolveu ao código traduzido.

</details>

<img src="docs/assets/rule-05.svg" alt="" width="100%">

## Método

**Prever, depois medir.**

- Uma previsão é escrita antes de o resultado ser lido: cerca de 1.100 numeradas até esta beta.
- No mínimo três execuções por configuração. O ruído entre execuções é de 30–64 %, então uma diferença abaixo de cerca de 40 % não pode ser vista com n = 3.
- Uma mudança precisa provar que está ativa: o log tem que mostrá-la disparando.
- Uma execução que nunca chega ao nível é uma execução morta, não um ponto de dados.
- A tela vence o log. Quando a tela contradiz uma sonda, a sonda está errada.

<details>
<summary><b>Gravação e replay, retratações, clean room, quem faz o quê</b></summary>

**Gravação e replay determinísticos.** Com cada thread do guest sendo uma corrotina em uma única thread do host, a maior parte das fontes de
não determinismo desaparece; o replay é conferido com a gravação, não presumido. Três fronteiras capturam toda a entrada externa: o despachante de importações, o dreno de mensagens da janela e os
buffers de áudio consumidos. O journal tem cerca de 16 MB. Ele permite a mesma execução duas vezes com exatamente uma flag alterada.

**O instrumento é o que falha.** Quase 200 descobertas foram retiradas pelo caminho. A maior parte
(31 %) era um instrumento que respondia a uma pergunta diferente; a seguinte (17 %), um que não conseguia responder. Por isso
cada oráculo aqui nomeia sua entrada e se recusa a responder quando essa entrada falta.

**Clean room para as regras da partida.** O multiplayer e o Zombies tiram suas regras de partida de um arquivo de patch diário que a Steam
não distribui mais. A substituição em [runtime/match_rules/](runtime/match_rules/) foi escrita por uma equipe que nunca viu o
script original: uma equipe escreveu uma especificação comportamental a partir do jogo, a outra escreveu o código apenas a partir dessa
especificação, e as duas só se falaram por perguntas e respostas escritas, guardadas com suas datas.

**Quem faz o quê.** Agentes de IA fornecem a vazão: escrever os shims, fazer bisseção de milhões de chamadas por execução, ler dezenas
de milhares de linhas. O humano fornece o objetivo, as correções a partir do que realmente está na tela, e recusa a conclusão
lisonjeira. É um jogo, meses de trabalho e muita intervenção manual.

</details>

<img src="docs/assets/rule-07.svg" alt="" width="100%">

## Limites

- **Somente local.** O multiplayer e o Zombies rodam contra o responder no seu Mac. Não há jogo online nem LAN. O League Play é recusado.
- **As regras da partida são nossas.** Escritas em clean room (veja [Método](#método)); o comportamento pode diferir do original em
  alguns pontos.
- **Calor.** O Mac esquenta muito em sessões longas (111–116 °C reportados no M1). Jogue na tomada: na bateria,
  o macOS limita o jogo drasticamente. Ligar na tomada evita a lentidão, não o calor.
- **Primeira partida em cada mapa.** No Zombies e no multiplayer, os shaders são traduzidos para o Metal enquanto o mapa carrega, então
  esse primeiro carregamento é mais longo e o primeiro minuto ainda pode engasgar um pouco. Na campanha eles são traduzidos quando
  aparecem pela primeira vez, então a cinemática de carregamento fica fluida e a primeira passagem por um lugar pode engasgar. Tudo
  fica em cache: da próxima vez, carrega rápido e roda fluido.
- **Janela.** O jogo roda em uma janela de 1280×720.
- **Outros Macs.** Apenas um M1 foi medido. M2, M3 e M4 devem ser mais rápidos, mas nenhum número é afirmado.
- **Campanha.** Cada missão carrega e encadeia; nem todas foram jogadas do início ao fim.
- **Zombies.** Sem co-op online nem LAN, então Grief e Turned não podem ser jogados. A tela dividida local para dois jogadores é
  experimental: o jogador 2 aparece quando a rodada 1 começa.
- **Áudio do macOS.** Quando o `coreaudiod` do sistema não está saudável, a campanha trava no primeiro vídeo. Verifique se
  `afplay -t 1 /System/Library/Sounds/Tink.aiff` leva cerca de 1 s; reiniciar o Mac resolve.

<img src="docs/assets/rule-08.svg" alt="" width="100%">

## Instalação

Você precisa de um Mac com Apple silicon, no mínimo **20 GB livres** (a conversão atinge um pico de cerca de 10 GB e cerca de 4 GB permanecem) e
da Steam para Mac conectada à conta que possui o jogo.

**macOS 26 ou posterior: sem Terminal.** Baixe o `Black Ops II.app` das [releases](https://github.com/adrbn/bo2-mac/releases)
e abra-o. Ele traz suas próprias ferramentas de tradução (LLVM 17, Remill, Unicorn, libvpx, HLSLcc, capstone, compiladas por
[scripts/make_toolchain.sh](scripts/make_toolchain.sh)) e nenhum arquivo do jogo. Na primeira vez, o macOS se oferece para instalar
as Command Line Tools gratuitas da Apple: clique em "Instalar" e depois em "Tentar novamente" na janela do app.
Uma vez por dia, ao abrir, o app procura uma versão mais recente e se oferece para instalá-la. "Instalar, manter esta versão" mantém
o app atual ao lado, renomeado com a sua versão, de modo que abri-lo volta atrás.

**macOS 14 ou 15: pelo Terminal**, com o Homebrew:

```bash
git clone https://github.com/adrbn/bo2-mac.git && cd bo2-mac
brew install llvm@17 cmake ninja libvpx unicorn
pip3 install capstone
scripts/make_app.sh --from-source
```

Abra o `Black Ops II.app`, aponte-o para a pasta do seu jogo e deixe-o converter: cerca de 1 hora e 45 em um M1, a campanha primeiro em cerca de 45 minutos. A campanha é
jogável assim que é convertida: jogue-a então, e o Zombies e o multiplayer são convertidos quando você os escolhe pela primeira vez
no menu do jogo. Caso contrário, a conversão continua pelos três. Se o disco ficar com menos de
8 GB livres, a conversão para de forma limpa e retoma de onde estava.

**Tela dividida no Zombies (experimental).** Conecte dois controles, abra o menu **Black Ops II** na barra de menus e
marque **Two-Player Split Screen in Zombies**. Vale a partir da próxima vez que o Zombies começar, pelo app ou pela
entrada ZOMBIES do menu do jogo. O jogador 1 joga na metade superior, o jogador 2 na inferior; o jogador 2 aparece quando a
rodada 1 começa. Sem um segundo controle, o jogo continua com um jogador. Desmarque para voltar a um jogador.

**Atualizar, ou se a conversão parar.** Execute `git pull && scripts/make_app.sh --from-source` na pasta do
repositório: o que já foi convertido é mantido e a conversão é retomada. Uma atualização refaz apenas o que as suas mudanças tocam: uma
correção no runtime religa os três modos em cerca de 3 minutos em um M1 em vez de converter de novo. Se ainda assim falhar,
abra uma issue com o log que ele indica.

O conversor só aceita a build exata da Steam, verificada por SHA-256. Os executáveis da Steam são criptografados pelo wrapper
SteamStub da Valve e **este repositório não fornece nenhum meio de removê-lo**: o conversor espera as três imagens decriptadas
em `~/.bo2-native/steam/` (`t6sp.exe.unpacked.exe`, `t6mp.exe.unpacked.exe`, `t6zm.exe.unpacked.exe`). Você mesmo as faz
a partir da cópia que comprou, com uma ferramenta separada de sua escolha. Somente o wrapper deve sair: os nomes de arquivo
acima e a verificação SHA-256 dizem na hora se um arquivo está certo. Nunca compartilhe esses arquivos.

**DLC.** O launcher pergunta ao seu cliente Steam quais DLCs a sua conta possui, exatamente como o jogo faz no Windows. Um mapa de DLC
roda quando você o possui e os arquivos dele estão na pasta do seu jogo.

<img src="docs/assets/rule-09.svg" alt="" width="100%">

## Repositório

| caminho | o que contém |
|---|---|
| [runtime/win32/](runtime/win32/) | o runtime Win32: kernel32, o scheduler, Direct3D 11, som, entrada, o responder do serviço local |
| [runtime/gfx/](runtime/gfx/) | o backend Metal: apresentação, cache de shaders, render targets |
| [runtime/match_rules/](runtime/match_rules/) | as regras de partida em clean room para o multiplayer e o Zombies |
| [tools/](tools/) | o driver de elevação, o catálogo, os passes de IR, as ferramentas de disassembly e análise |
| [tools/remill-patches/](tools/remill-patches/) | os patches aplicados sobre o Remill v6.0.1 |
| [scripts/](scripts/) | `make_app.sh`, o instalador e conversor da Steam, os launchers |
| [third_party/HLSLcc](third_party/HLSLcc) | tradução de shaders de DXBC para Metal |
| [docs/](docs/) | o estado da pré-release, a toolchain |

<img src="docs/assets/rule-10.svg" alt="" width="100%">

## Roadmap

1. **Esta beta**: os três modos a partir de uma cópia nova da Steam, em outros Macs além do que os compilou. Os seus relatos
   (modelo do Mac, taxa de quadros, o que quebrou) decidem o que vem a seguir.
   Primeiro relato externo, 05/11/2026: em um M2 Pro (16 GB), a conversão foi concluída e o jogo iniciou, após três
   correções encontradas pelo caminho ([#4](https://github.com/adrbn/bo2-mac/issues/4), [#5](https://github.com/adrbn/bo2-mac/issues/5), [#6](https://github.com/adrbn/bo2-mac/issues/6)).
2. **Sem Terminal**: feito na v0.2.0. Um app para baixar e abrir; ele traz suas ferramentas já compiladas, então nada é
   compilado no seu Mac além do próprio jogo, e ele se atualiza sozinho (macOS 26 ou posterior; macOS 14 e 15 mantêm a
   instalação pelo Terminal).
3. **Totalmente traduzido**: os 46 blocos de código que ainda passam pelo interpretador Unicorn são traduzidos com
   antecedência, e cada instrução traduzida é conferida com o interpretador;
   qualquer função traduzida pode ser devolvida ao interpretador, para achar uma errada por bisseção.
4. **Sem espera de shaders**: shaders compilados antes da primeira partida, em vez de na abertura (cerca de 3 minutos no multiplayer) e
   durante o jogo (travadas quando um novo efeito aparece). Iniciado: os shaders agora compilam em lotes, e os das
   últimas sessões compilam em segundo plano na abertura. No multiplayer em um M1 sem Xcode (algumas tentativas, não um
   benchmark), a compilação de shaders durante os dois primeiros minutos de uma partida passou de 18–25 s para 10–14 s na primeira
   abertura e cerca de 5 s na seguinte; as travadas acima de 300 ms passaram de 15–22 para 13–15, depois 7. A primeira abertura
   ainda trava quase com a mesma frequência.
5. **Conversão mais curta**: de cerca de 2,5 horas para 1 hora e 45 em um M1 (06/11/2026, medido nos três modos:
   a preparação lê o executável uma vez, a elevação e a dobra usam todos os núcleos que a memória permite). Uma atualização agora refaz
   apenas o que mudou (uma correção de runtime: cerca de 3 minutos). A seguir: menos otimização para código que raramente roda. Os três modos compartilham apenas 4 % das
   suas funções, então converter o código compartilhado uma única vez ganharia pouco.
6. **Mais rápido, sem calor**: a campanha é limitada pela thread do scheduler, não pela renderização; o multiplayer com 11 bots
   é limitado pela thread do servidor. Reescritas nativas das funções mais quentes vêm a seguir, depois taxas de quadros acima de 60 para
   telas de 120 Hz.
7. **Controles**: vibração e aim assist.
8. **Juntos**: a tela dividida local no Zombies, que a versão de PC nunca teve, é experimental desde a v0.2.0 (um jogo,
   dois jogadores, metades superior e inferior), com um interruptor no menu do app desde a v0.2.1. A seguir: sockets UDP reais para o co-op do Zombies,
   Grief e Turned, e o multiplayer em LAN.
9. **Mais tarde**: MetalFX, pós-processamento na apresentação do quadro e texturas de substituição carregadas a partir da cópia do próprio jogador.

<img src="docs/assets/rule-11.svg" alt="" width="100%">

## Aviso legal

Para uma cópia que o usuário possui, em um hardware que o usuário possui.

- Nenhum código-fonte do engine e nenhum código vazado são usados.
- Nem o binário de entrada nem a saída recompilada são distribuídos. Nenhum arquivo do jogo está incluído: o jogador precisa da sua
  própria cópia do Call of Duty: Black Ops II na Steam. O conversor só aceita a build exata da Steam (SHA-256), e
  a posse é verificada a cada vez que o jogo inicia pelo cliente Steam do próprio jogador: o jogo se recusa a rodar quando a Steam
  não está conectada ou a conta não o possui.
- A proteção contra cópia é executada como o jogo a escreveu; nada nela é removido por patch. Este repositório não fornece nenhum meio
  de remover o wrapper SteamStub da Valve.
- O serviço online que o jogo espera é respondido por um responder local escrito a partir das requisições do próprio jogo: nenhum
  servidor da Activision é contatado, não há jogo online, e a posse do jogo continua sendo verificada pela Steam do jogador.
  As regras da partida foram escritas em clean room.
- O código escrito aqui é licenciado sob a GPL-3.0 ([LICENSE](LICENSE)).
- O Unicorn (GPL-2.0) nunca é combinado com código GPL-3.0 em um binário distribuído. O app traz a biblioteca não modificada do Unicorn
  como um arquivo separado ao lado das outras ferramentas; ela é ligada ao binário do jogo no próprio Mac do jogador
  ([tools/boot.py](tools/boot.py)), e esse binário nunca é distribuído. Cada release publica o código-fonte da
  versão exata do Unicorn que traz (`unicorn-<version>-source.tar.gz`), como a GPL-2.0 exige. As licenças de cada
  ferramenta incluída estão no app, em `Contents/Resources/toolchain/licenses`.

Este é um projeto de fã não oficial. Call of Duty e Black Ops são marcas registradas da Activision Publishing, Inc. Steam é uma
marca registrada da Valve Corporation. Este projeto não é afiliado, endossado nem patrocinado pela Activision, Treyarch,
Microsoft ou Valve. Para jogar, compre o jogo.

## Star History

<a href="https://www.star-history.com/#adrbn/bo2-mac&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
  </picture>
</a>

<div align="center"><img src="docs/assets/footer.svg" alt="" width="100%"></div>
