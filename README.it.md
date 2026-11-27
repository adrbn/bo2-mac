<div align="center">

<picture>
  <source media="(prefers-color-scheme: light)" srcset="docs/assets/hero-public-light.svg">
  <img src="docs/assets/hero-public.svg" alt="bo2-mac: Call of Duty: Black Ops II ricompilato staticamente da x86-32 Windows a ARM64 macOS nativo, dalla tua copia Steam. Su un Apple M1: campagna da 51 a 60 fotogrammi al secondo, Zombies da 57 a 60, multiplayer contro i bot da 53 a 60." width="100%">
</picture>

# bo2-mac

**Call of Duty: Black Ops II, ricompilato staticamente da x86 a 32 bit per Windows a ARM64 nativo per macOS.**<br>
Campagna, Zombies e multiplayer contro i bot, costruiti sul tuo Mac a partire dalla tua copia Steam.

<a href="README.md">English</a> · <a href="README.fr.md">Français</a> · <a href="README.es.md">Español</a> · <a href="README.pt-BR.md">Português</a> · <a href="README.de.md">Deutsch</a> · <b>Italiano</b> · <a href="README.ru.md">Русский</a> · <a href="README.ja.md">日本語</a> · <a href="README.ko.md">한국어</a> · <a href="README.zh.md">中文</a>

<a href="#installazione"><img src="docs/assets/btn-play.svg" alt="Giocaci" height="40"></a>&nbsp;
<a href="#risultati"><img src="docs/assets/btn-results.svg" alt="Risultati" height="40"></a>&nbsp;
<a href="#pipeline"><img src="docs/assets/btn-pipeline.svg" alt="Pipeline" height="40"></a>&nbsp;
<a href="#scoperte"><img src="docs/assets/btn-findings.svg" alt="Scoperte" height="40"></a>&nbsp;
<a href="#roadmap"><img src="docs/assets/btn-roadmap.svg" alt="Roadmap" height="40"></a>

<a href="https://news.ycombinator.com/item?id=50449891"><img src="docs/assets/btn-hackernews.svg" alt="In evidenza su Hacker News" height="44"></a>

![source: x86-32 Windows](https://img.shields.io/badge/source-x86--32%20Windows-ff6a00?style=flat-square&labelColor=0b0b0c)
![target: ARM64 macOS](https://img.shields.io/badge/target-ARM64%20macOS-ff6a00?style=flat-square&labelColor=0b0b0c)
![lifter: Remill, LLVM 17](https://img.shields.io/badge/lifter-Remill%20%E2%86%92%20LLVM%2017-ff6a00?style=flat-square&labelColor=0b0b0c)
![renderer: D3D11 to Metal](https://img.shields.io/badge/renderer-D3D11%20%E2%86%92%20Metal-ff6a00?style=flat-square&labelColor=0b0b0c)
![Wine: none](https://img.shields.io/badge/Wine-none-ff6a00?style=flat-square&labelColor=0b0b0c)
![game files in this repository: none](https://img.shields.io/badge/game%20files-none-f4f1ea?style=flat-square&labelColor=0b0b0c)
[![latest release](https://img.shields.io/github/v/release/adrbn/bo2-mac?style=flat-square&labelColor=0b0b0c&color=ff6a00&label=release)](https://github.com/adrbn/bo2-mac/releases/latest)
![status: beta](https://img.shields.io/badge/status-beta-f4f1ea?style=flat-square&labelColor=0b0b0c)

<sub>[Perché](#perché) · [Sintesi](#sintesi) · [Metodo](#metodo) · [Limiti](#limiti) · [Repository](#repository) · [Note legali](#note-legali)</sub><br>
<sub>Beta, 03-11-2026 · misurato su un solo Apple M1 (4 core prestazionali + 4 core efficienti, 16 GB)</sub>

</div>

<p align="center"><a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4"><img src="docs/assets/preview.gif" alt="Campagna, Zombies e multiplayer in esecuzione nativa su macOS" width="560"></a><br>
<sub>Campagna (Angola), Zombies (Town) e multiplayer (Hijacked, contro i bot) su un Apple M1, registrati durante il gioco; il numero in alto a destra indica i fotogrammi al secondo, e la registrazione dello schermo ne fa perdere un po', soprattutto nella campagna. <a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4">Video di 80 secondi (MP4, 20 MB)</a></sub></p>

> [!IMPORTANT]
> **Questo repository non contiene alcun file del gioco né codice di gioco tradotto.** Trasforma la tua copia Steam di Black Ops
> II in un'app per il tuo Mac, sul tuo Mac. Il codice del gioco finisce dentro quell'app, quindi nessuno può
> fornirtene una già pronta: ogni giocatore costruisce la propria, e non può essere condivisa. Progetto amatoriale non ufficiale, non affiliato a
> Activision, Treyarch o Microsoft. Per giocare, compra il gioco. Beta: leggi [che cosa è stato dimostrato e che cosa no](docs/PRERELEASE.md).

---

<img src="docs/assets/rule-01.svg" alt="" width="100%">

## Perché

Black Ops II non è mai uscito per Mac, e su Apple Silicon prima non era davvero giocabile.
[AppleGamingWiki](https://www.applegamingwiki.com/wiki/Call_of_Duty:_Black_Ops_II) lo indica come ingiocabile con
CrossOver e non supportato con Rosetta 2. Resta una macchina virtuale Windows ARM in Parallels, con segnalazioni
che vanno da 1 fotogramma al secondo nelle partite su un M1 Max a 60–80 su M3 e M4
([MacGamingDB](https://macgamingdb.app/games/call-of-duty-black-ops-ii)). Qui il gioco viene tradotto una volta sola, prima
dell'avvio, in un'app ARM64 nativa: 51–60 fotogrammi al secondo su un M1 di base, senza Windows e senza macchina virtuale. Essere
nativo significa anche poterlo correggere ed estendere come qualsiasi app per Mac: lo schermo condiviso, che la versione per PC non ha mai avuto, è
disponibile come opzione sperimentale in Zombies.

## Sintesi

bo2-mac prende i tre eseguibili di Black Ops II, la campagna (`t6sp.exe`), Zombies (`t6zm.exe`) e
il multiplayer (`t6mp.exe`), e ne trasforma in anticipo il codice macchina x86 in codice macchina ARM64.
[Remill](https://github.com/lifting-bits/remill) solleva ogni funzione in LLVM IR, una catena di passi nostri riscrive
quell'IR, e il risultato viene collegato a un runtime Win32 scritto a mano, il cui Direct3D 11 è disegnato da Metal. Non c'è
Wine e non c'è JIT: il gioco gira come codice nativo, in un solo processo. Circa 150.000 funzioni vengono tradotte nei
tre eseguibili. Solo le brevi sequenze che la protezione anticopia nasconde girano in un piccolo interprete x86, così come
qualsiasi codice che la traduzione ha mancato, quindi una lacuna è pensata per costare velocità anziché mandare in crash il gioco.

Su un Apple M1 la campagna gira a **51–60 fotogrammi al secondo**, e ogni missione si carica e passa alla successiva,
fino ai titoli di coda. Zombies si gioca in singolo su **sei mappe** attraverso i menu del gioco a **57–60**.
Il multiplayer si gioca contro i bot attraverso i suoi veri menu, undici modalità di gioco, a **53–60**. Un'unica app,
`Black Ops II.app`, apre la campagna, e le voci CAMPAIGN / ZOMBIES / MULTIPLAYER di ciascun menu passano
all'altro gioco.

Non viene scaricato nulla da nessuno tranne che da te: il convertitore legge gli eseguibili della copia che hai comprato, li verifica
byte per byte e costruisce l'app sulla tua macchina.

<img src="docs/assets/rule-02.svg" alt="" width="100%">

## Risultati

| | misurato | condizioni |
|---|---|---|
| Campagna | **51–60 fps** | ogni missione si carica e si concatena fino ai titoli di coda; non tutte giocate dall'inizio alla fine |
| Zombies | **57–60 fps** (Die Rise 43–58) | TranZit, Mob of the Dead, Nuketown, Origins, Buried, Die Rise, in singolo, attraverso i veri menu |
| Multiplayer contro i bot | **53–60 fps** | undici modalità di gioco attraverso i veri menu: un round completo, il tabellone, il ritorno alla lobby |
| Mondo di gioco rispetto al tempo reale | **≈ 1,0×** | l'orologio del server del gioco rispetto al tempo reale |
| Conversione sul Mac del giocatore | **circa 1 ora e 45** su un M1 (la sola campagna: circa 45 minuti) | la campagna è giocabile non appena è convertita; Zombies e multiplayer seguono |
| Disco | picco di circa **10 GB**, circa **4 GB** conservati | dopo la conversione restano solo i binari collegati |

Il frame rate conta i drawable realmente presentati a schermo, verificati rispetto allo schermo e al Metal HUD.

**A che cosa puoi giocare**

<table>
<tr>
<td width="33%" valign="top"><img src="docs/assets/card-public-campaign.svg" alt="Campagna" width="100%"><br>
Il vero menu principale: NEW STORY, RESUME STORY, REPLAY MISSION. Da <code>angola</code> ai titoli di coda, le
missioni si caricano e passano alla successiva. Le missioni Strike Force partono con un clic.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-zombies.svg" alt="Zombies" width="100%"><br>
Le sei mappe, in singolo: menu principale, lobby, partita, Game Over, tabellone, poi un'altra partita nello stesso processo.
Le mappe DLC si giocano se il tuo account Steam le possiede. Lo schermo condiviso per due giocatori è sperimentale.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-multi.svg" alt="Multiplayer" width="100%"><br>
Custom Games contro i bot dai veri menu: CHOOSE CLASS, l'HUD, un round completo, il tabellone
e il ritorno alla lobby. Le regole di partita sono lo script di questo progetto.</td>
</tr>
</table>

In tutti e tre i modi: tastiera e mouse, oppure un DualSense come XInput tramite GameController.framework; audio attivo; i tasti
di movimento seguono la loro posizione sulla tastiera (ZQSD su AZERTY, WASD su QWERTY), a meno che tu non li abbia assegnati
tu stesso nelle opzioni del gioco.

<img src="docs/assets/rule-03.svg" alt="" width="100%">

## Pipeline

In cinque passi, tutti sul Mac del giocatore:

1. **Verifica.** Il convertitore accetta solo l'esatta build Steam di ciascun eseguibile, verificata tramite SHA-256. Il
   launcher chiede al client Steam sul tuo Mac se l'account connesso possiede il gioco, e rifiuta in caso contrario.
2. **Catalogo e sollevamento.** Ghidra elenca le funzioni del tuo eseguibile; Remill v6.0.1 solleva ciascuna in
   LLVM IR. Cinque patch al lifter sono incluse in [tools/remill-patches/](tools/remill-patches/), tra cui la
   modalità di arrotondamento x87, e una che esegue un fork di un Remill già in esecuzione per ogni funzione invece di ricaricarne le semantiche x86.
3. **Ripiegamento.** L'IR di Remill è corretto ma lento: ogni accesso alla memoria guest, ogni flag e ogni push x87 è una chiamata o una scrittura
   in una struttura `State`. I nostri passi lo riscrivono prima di `clang -O2`: le letture e scritture guest vengono inlinate, le sequenze `lock`
   diventano atomiche hardware, il `ret` guest resta dentro il codice sollevato e lo stack x87 viene promosso a
   variabili locali.
4. **Runtime.** [runtime/](runtime/) implementa le importazioni Windows che il gioco chiama davvero, con arità stdcall esatte
   (quelle della Steam API prese dagli header pubblici di Valve). La memoria guest è un'unica mappatura da 4 GiB. Direct3D 11 è
   un livello fake-COM sopra Metal; gli shader DXBC passano per [HLSLcc](third_party/HLSLcc). XAudio2 va su CoreAudio, e
   i video sono decodificati nativamente con libvpx.
5. **Thread.** I thread guest partono come coroutine su un solo thread host, il che elimina la maggior parte delle fonti di
   non determinismo e rende un'esecuzione riproducibile. La configurazione veloce sposta il server, il backend di rendering, il mixaggio e la decodifica audio, e i worker dei job
   su veri thread host. ARM non è TSO, quindi servivano vere barriere di memoria.

**La protezione anticopia viene eseguita, non aggirata.** Il gioco nasconde i controlli di integrità dietro eccezioni hardware,
trap a passo singolo e codice che decifra nello heap. Una piccola macchina x86 costruita su Unicorn
([x86_guest_vm.cpp](runtime/win32/x86_guest_vm.cpp), [heap_code.cpp](runtime/win32/heap_code.cpp)) esegue esattamente quelle
sequenze e restituisce il controllo al codice ricompilato. Nessun esito di controllo viene forzato e nessuna chiave viene inserita a mano.

**Una lacuna nella traduzione è pensata per costare velocità, non un crash.** Quando il gioco raggiunge codice x86 che non è mai stato sollevato, la
stessa macchina esegue i suoi byte originali ([interp.cpp](runtime/win32/interp.cpp)) e restituisce il controllo al
codice ricompilato al blocco tradotto, alla chiamata o al ritorno successivo. Ogni indirizzo interpretato viene registrato nel log (`[INTERP]`).

**Il servizio online riceve risposta in locale.** Zombies e multiplayer si aspettano Demonware; un responder scritto a partire dalle
richieste del gioco ([dw_local.cpp](runtime/win32/dw_local.cpp)) risponde su lobby, storage e ticket sul tuo Mac.
Nulla lascia la macchina.

<details>
<summary>Perché non Wine, CrossOver, DXVK o DXMT?</summary>

Traducono le chiamate Windows a tempo di esecuzione e lasciano il codice x86 sotto Rosetta. Qui il codice x86 stesso non c'è più: è
codice macchina ARM64 prima che il gioco parta. DXVK e DXMT sono DLL PE che raggiungono il loro backend nativo tramite il meccanismo
unixlib di Wine; questo port non ha Wine, e il suo Direct3D 11 è un livello proprio sopra la memoria guest a 32 bit. Da fuori serviva solo la traduzione
degli shader, e HLSLcc, una semplice libreria C++, la copre.
</details>

<img src="docs/assets/rule-04.svg" alt="" width="100%">

## Scoperte

Ogni voce è un sintomo visto a schermo, la causa che è stata misurata e l'effetto della correzione.

<details>
<summary><b>Tredici scoperte</b>, dalla modalità di arrotondamento x87 a una falsa partenza dentro una sola istruzione</summary>


**F1 · Il lifter ignorava la modalità di arrotondamento x87.** In Zombies la visuale da proni tremava. `fistp` arrotondava al più vicino dove
il gioco aveva chiesto il troncamento, dentro il codec di rete dell'altezza della visuale. L'altezza ricevuta dal client era
sbagliata in 8 campioni su 8, e in 0 su 8 dopo una patch di Remill e un nuovo sollevamento del codec.

**F2 · Un bit riservato della control word x87.** Una missione della campagna si bloccava in un ciclo di effetti, solo con thread
reali. Il lifter memorizzava il bit 6 della control word in modo diverso dall'hardware reale. Una patch di una riga a Remill e quattro
funzioni del runtime sollevate di nuovo l'hanno risolto, senza rallentamenti.

**F3 · Un flag di funzionalità della CPU cambia l'algoritmo.** Il nostro `IsProcessorFeaturePresent` rispondeva «no SSE2», quindi il runtime
C usava il suo `ceil` x87, che sotto il nostro runtime arrotondava al più vicino. Un effetto campionava poi l'indice −1 all'infinito: due
missioni si bloccavano.

**F4 · Il nostro stesso log causava lo scatto.** Gli scatti di 170–430 ms erano il thread di rendering bloccato in `write()`: il nostro log,
reindirizzato su un file in un disco quasi pieno. Inviarlo tramite una pipe a un processo figlio ha portato gli scatti per esecuzione da
12 e 8 a 0 e 0.

**F5 · La traduzione degli shader spostava le costanti di 4 byte.** HLSLcc posizionava un `float2` dopo un `float` all'offset 24 dove
Direct3D lo legge a 20. Sono stati colpiti 31 shader su 2.664, tra cui gli occhi e i reticoli.

**F6 · La protezione deve girare.** Il menu delle opzioni si bloccava 8 volte su 8. Un controllo di integrità riscrive il proprio indirizzo
di ritorno per puntare a codice decifrato nello heap, cosa che il runtime ignorava. Eseguire quel codice, come fa Windows, ha dato
3 su 3 senza blocchi.

**F7 · Le nostre stesse protezioni di sicurezza erano l'ostacolo.** Una nostra protezione fermava il gioco a 28 secondi di partita. I maggiori
guadagni del progetto sono venuti dalla rimozione delle nostre protezioni e del false sharing, non da codice ingegnoso.

**F8 · Uno stub API vuoto sembra un servizio guasto.** Due stub che non restituivano nulla (`CryptGenRandom`,
`StringFromGUID2`) sembravano un guasto di rete in Zombies.

**F9 · Le priorità dei thread fanno parte del programma.** `SetThreadPriority` era uno stub. Passare la richiesta del mixer a
macOS ha ridotto i buffer audio troppo corti di un'esecuzione da 908–1872 a 7–71, e con essi i vuoti nel suono.

**F10 · Skinning nativo, bit per bit.** Lo skinning su CPU riscritto in NEON dà gli stessi bit dell'x86 originale e ha portato
una scena pesante da 17,8 a 30,3 fps.

**F11 · I decal richiedono il depth bias del gioco.** I segni di bruciatura e il sangue comparivano o sparivano con una leggera rotazione della
camera. Il gioco chiede un depth bias sui decal; il nostro livello lo lasciava a zero. Passare a Metal i valori del gioco ha risolto.

**F12 · Il thread di rendering copiava la geometria del livello a ogni fotogramma.** In multiplayer e Zombies copiava 23–63 MB
di dati dei vertici alla GPU per fotogramma, in gran parte statici. Tenere i buffer statici residenti in Metal ha portato le copie a
3,5–5 MB per fotogramma: multiplayer 51,8 → 60 fps, Zombies 43,1 → 58,2. La stessa modifica non fa nulla nella campagna,
dove il ritmo è dettato dal thread dello scheduler. Un'ottimizzazione aiuta solo il modo il cui collo di bottiglia tocca.

**F13 · Una falsa partenza dentro una sola istruzione costava metà del frame rate in combattimento.** Nelle prime conversioni da una
copia Steam nuova, Zombies scendeva a 25 fps ogni volta che il giocatore sparava. Funzioni calde giravano nell'interprete: il
catalogo collocava l'inizio della funzione successiva dentro la prima istruzione di una radice, quindi la sua scansione limitata
decodificava zero byte e la radice veniva scartata senza una parola. Scansionare tali radici senza il limite, come già faceva il lifter
una per una, le ha rimesse nel codice tradotto.

</details>

<img src="docs/assets/rule-05.svg" alt="" width="100%">

## Metodo

**Prevedi, poi misura.**

- Una previsione viene scritta prima di leggere il risultato: circa 1.100 numerate al momento di questa beta.
- Almeno tre esecuzioni per configurazione. Il rumore tra un'esecuzione e l'altra è del 30–64%, quindi una differenza inferiore a circa il 40% non si vede con n = 3.
- Una modifica deve dimostrare di essere attiva: il log deve mostrare che scatta.
- Un'esecuzione che non raggiunge mai il livello è un'esecuzione morta, non un dato.
- Lo schermo batte il log. Quando lo schermo contraddice una sonda, la sonda ha torto.

<details>
<summary><b>Registrazione e riproduzione, ritrattazioni, clean room, chi fa cosa</b></summary>

**Registrazione e riproduzione deterministiche.** Con ogni thread guest come coroutine su un solo thread host, la maggior parte delle fonti di
non determinismo sparisce; la riproduzione viene verificata rispetto alla registrazione, non data per scontata. Tre confini catturano ogni input esterno: il dispatcher delle importazioni, lo svuotamento dei messaggi della finestra e i
buffer audio consumati. Il journal pesa circa 16 MB. Permette di eseguire due volte la stessa esecuzione cambiando esattamente un flag.

**Lo strumento è ciò che fallisce.** Quasi 200 risultati sono stati ritirati lungo il percorso. La quota maggiore
(31%) era uno strumento che rispondeva a una domanda diversa; la successiva (17%), uno che non poteva rispondere affatto. Perciò
ogni oracolo qui dichiara il suo input e si rifiuta di rispondere quando quell'input manca.

**Clean room per le regole di partita.** Multiplayer e Zombies prendono le loro regole di partita da un file di patch giornaliero che Steam
non distribuisce più. Il sostituto in [runtime/match_rules/](runtime/match_rules/) è stato scritto da un team che non ha mai visto lo
script originale: un team ha scritto una specifica comportamentale a partire dal gioco, l'altro ha scritto il codice solo da quella
specifica, e i due hanno comunicato solo tramite domande e risposte scritte, conservate con le rispettive date.

**Chi fa cosa.** Gli agenti IA forniscono la capacità di elaborazione: scrivere gli shim, fare bisezione su milioni di chiamate per esecuzione, leggere decine
di migliaia di righe. L'umano fornisce l'obiettivo, le correzioni basate su ciò che è davvero a schermo, e rifiuta la
conclusione lusinghiera. È un solo gioco, mesi di lavoro e un forte intervento manuale.

</details>

<img src="docs/assets/rule-07.svg" alt="" width="100%">

## Limiti

- **Solo in locale.** Multiplayer e Zombies girano contro il responder sul tuo Mac. Non c'è gioco online né LAN. League Play viene rifiutato.
- **Le regole di partita sono nostre.** Scritte in una clean room (vedi [Metodo](#metodo)); il comportamento può differire dall'originale in
  alcuni punti.
- **Calore.** Il Mac si scalda molto durante le sessioni lunghe (111–116 °C segnalati sull'M1). Gioca con l'alimentatore collegato: a batteria,
  macOS limita pesantemente il gioco. Collegarlo evita il rallentamento, non il calore.
- **Prima partita su ogni mappa.** In Zombies e multiplayer, gli shader vengono tradotti per Metal mentre la mappa si carica, quindi
  quel primo caricamento è più lungo e il primo minuto può ancora avere qualche leggero scatto. Nella campagna vengono tradotti quando
  compaiono per la prima volta, quindi la cinematica di caricamento resta fluida e il primo passaggio in un luogo può scattare. Tutto
  viene messo in cache: la volta successiva si carica in fretta e gira fluido.
- **Finestra.** Il gioco gira in una finestra 1280×720.
- **Altri Mac.** È stato misurato un solo M1. M2, M3 e M4 dovrebbero essere più veloci, ma non si dichiara alcun numero.
- **Campagna.** Ogni missione si carica e si concatena; non ognuna è stata giocata dall'inizio alla fine.
- **Zombies.** Niente co-op online o LAN, quindi Grief e Turned non si possono giocare. Lo schermo condiviso locale per due giocatori è
  sperimentale: il giocatore 2 compare quando inizia il round 1.
- **Audio di macOS.** Quando il `coreaudiod` di sistema non è in buono stato, la campagna si blocca sul primo video. Verifica che
  `afplay -t 1 /System/Library/Sounds/Tink.aiff` duri circa 1 s; riavviare il Mac risolve.

<img src="docs/assets/rule-08.svg" alt="" width="100%">

## Installazione

Ti servono un Mac Apple silicon, almeno **20 GB liberi** (la conversione ha un picco di circa 10 GB e ne restano circa 4) e
Steam per Mac connesso all'account che possiede il gioco.

**macOS 26 o successivo: senza Terminale.** Scarica `Black Ops II.app` dalle [release](https://github.com/adrbn/bo2-mac/releases)
e aprila. Include i propri strumenti di traduzione (LLVM 17, Remill, Unicorn, libvpx, HLSLcc, capstone, costruiti da
[scripts/make_toolchain.sh](scripts/make_toolchain.sh)) e nessun file del gioco. La prima volta, macOS propone di installare i
Command Line Tools gratuiti di Apple: clicca «Installa», poi «Riprova» nella finestra dell'app.
Una volta al giorno all'avvio, l'app cerca una release più recente e propone di installarla. «Installa, mantieni questa versione» conserva
l'app attuale accanto a quella nuova, rinominata con la sua versione, così aprendo quella si torna indietro. Un aggiornamento mantiene ciò che è già convertito e rifà solo ciò che le sue modifiche toccano: una correzione del runtime ricollega le tre modalità in circa 3 minuti su un M1.

**macOS 14 o 15: dal Terminale**, con Homebrew:

```bash
git clone https://github.com/adrbn/bo2-mac.git && cd bo2-mac
brew install llvm@17 cmake ninja libvpx unicorn
pip3 install capstone
scripts/make_app.sh --from-source
```

Apri `Black Ops II.app`, indicale la cartella del gioco e lascia che converta: circa 1 ora e 45 su un M1, prima la campagna in circa 45 minuti. La campagna è
giocabile non appena è convertita: giocaci subito, e Zombies e multiplayer vengono convertiti la prima volta che li scegli
nel menu del gioco. Altrimenti la conversione prosegue su tutti e tre. Se il disco scende sotto
8 GB liberi, la conversione si ferma in modo pulito e riprende da dove era arrivata.

**Schermo condiviso in Zombies (sperimentale).** Collega un secondo controller e premi Start o A nella lobby di Zombies:
il giocatore 2 entra, come su console. B su quel controller, o scollegarlo, fa uscire il giocatore 2. Il giocatore 1 gioca
nella metà superiore, il giocatore 2 in quella inferiore; lo schermo si divide all'inizio della partita e il giocatore 2
compare quando inizia il round 1. Se nessuno entra, il gioco resta a un giocatore, a schermo intero. Non c'è niente da spuntare.

**Aggiornamento, o se la conversione si ferma.** Esegui `git pull && scripts/make_app.sh --from-source` nella cartella del
repository: ciò che è già convertito viene conservato e la conversione riprende. Un aggiornamento rifà solo ciò che le sue modifiche toccano: una
correzione al runtime ricollega i tre modi in circa 3 minuti su un M1 invece di convertire di nuovo. Se continua a fallire,
apri una issue con il log che indica.

Il convertitore accetta solo l'esatta build Steam, verificata tramite SHA-256. Gli eseguibili Steam sono cifrati dal wrapper
SteamStub di Valve e **questo repository non offre alcun modo per rimuoverlo**: il convertitore si aspetta le tre immagini decifrate
in `~/.bo2-native/steam/` (`t6sp.exe.unpacked.exe`, `t6mp.exe.unpacked.exe`, `t6zm.exe.unpacked.exe`). Le prepari
tu stesso dalla copia che hai comprato, con uno strumento separato a tua scelta. Deve sparire solo il wrapper: i nomi dei file
qui sopra e la verifica SHA-256 ti dicono subito se un file è giusto. Non condividere mai questi file.

**DLC.** Il launcher chiede al tuo client Steam quali DLC possiede il tuo account, esattamente come fa il gioco su Windows. Una mappa DLC
si gioca quando la possiedi e i suoi file sono nella cartella del gioco.

<img src="docs/assets/rule-09.svg" alt="" width="100%">

## Repository

| percorso | che cosa contiene |
|---|---|
| [runtime/win32/](runtime/win32/) | il runtime Win32: kernel32, lo scheduler, Direct3D 11, audio, input, il responder del servizio locale |
| [runtime/gfx/](runtime/gfx/) | il backend Metal: presentazione, cache degli shader, render target |
| [runtime/match_rules/](runtime/match_rules/) | le regole di partita clean-room per multiplayer e Zombies |
| [tools/](tools/) | il driver di sollevamento, il catalogo, i passi IR, gli strumenti di disassemblaggio e analisi |
| [tools/remill-patches/](tools/remill-patches/) | le patch applicate sopra Remill v6.0.1 |
| [scripts/](scripts/) | `make_app.sh`, l'installer e convertitore Steam, i launcher |
| [third_party/HLSLcc](third_party/HLSLcc) | traduzione degli shader da DXBC a Metal |
| [docs/](docs/) | lo stato della prerelease, la toolchain |

<img src="docs/assets/rule-10.svg" alt="" width="100%">

## Roadmap

1. **Questa beta**: i tre modi da una copia Steam nuova, su Mac diversi da quello che li ha costruiti. Le tue segnalazioni
   (modello di Mac, frame rate, che cosa si è rotto) decidono che cosa viene dopo.
   Prima segnalazione esterna, 05-11-2026: su un M2 Pro (16 GB), la conversione è stata completata e il gioco è partito, dopo tre
   correzioni trovate lungo la strada ([#4](https://github.com/adrbn/bo2-mac/issues/4), [#5](https://github.com/adrbn/bo2-mac/issues/5), [#6](https://github.com/adrbn/bo2-mac/issues/6)).
2. **Senza Terminale**: fatto. Un'app da scaricare e aprire; include già i suoi strumenti compilati, quindi sul tuo Mac non si
   compila altro che il gioco stesso, e si aggiorna da sola (macOS 26 o successivo; macOS 14 e 15 restano con
   l'installazione da Terminale).
3. **Tradotto per intero**: i 46 blocchi di codice che ancora passano dall'interprete Unicorn vengono tradotti in anticipo,
   e ogni istruzione tradotta viene verificata rispetto all'interprete;
   qualsiasi funzione tradotta può essere rimandata all'interprete, per trovarne una sbagliata per bisezione.
4. **Niente attesa per gli shader**: shader costruiti prima della prima partita, invece che all'avvio (circa 3 minuti in multiplayer) e
   durante il gioco (blocchi quando compare un nuovo effetto). Iniziato: gli shader ora vengono compilati a lotti, e quelli delle
   ultime sessioni vengono compilati in background all'avvio. In multiplayer su un M1 senza Xcode (qualche tentativo, non un
   benchmark), la compilazione degli shader nei primi due minuti di una partita è passata da 18–25 s a 10–14 s al primo
   avvio e a circa 5 s al successivo; i blocchi oltre i 300 ms sono passati da 15–22 a 13–15, poi a 7. Il primo avvio
   si blocca ancora più o meno con la stessa frequenza.
5. **Conversione più breve**: da circa 2,5 ore a 1 ora e 45 su un M1 (06-11-2026, misurato su tutti e tre i modi:
   la preparazione legge l'eseguibile una sola volta, il sollevamento e il ripiegamento usano tutti i core che la memoria consente). Un aggiornamento ora rifà
   solo ciò che è cambiato (una correzione del runtime: circa 3 minuti). Prossimo passo: meno ottimizzazione per il codice che gira di rado. I tre modi condividono solo il 4 % delle
   loro funzioni, quindi convertire una volta sola il codice condiviso farebbe guadagnare poco.
6. **Più veloce, senza calore**: la campagna è limitata dal thread dello scheduler, non dal rendering; il multiplayer con 11 bot
   è limitato dal thread del server. Già avviato: il riverbero gira in nativo in tutte e tre le modalità, circa il 40 % di tempo in meno sul thread audio. Seguiranno riscritture native delle altre funzioni più calde, poi frame rate oltre i 60 per
   gli schermi a 120 Hz.
7. **Controller**: vibrazione e aim assist.
8. **Insieme**: lo schermo condiviso locale in Zombies, che la versione per PC non ha mai avuto, è sperimentale (un solo gioco,
   due giocatori, metà superiore e inferiore, circa 28 fps su un M1), con un interruttore nel menu dell'app. Prossimo passo: veri socket UDP per la co-op di Zombies,
   Grief e Turned, e il multiplayer in LAN.
9. **Più avanti**: MetalFX, post-elaborazione al momento della presentazione del fotogramma texture sostitutive caricate dalla copia del giocatore, e ombre e occlusione ambientale in ray tracing sui Mac che lo hanno in hardware (M3 e successivi).

<img src="docs/assets/rule-11.svg" alt="" width="100%">

## Note legali

Per una copia posseduta dall'utente, su hardware posseduto dall'utente.

- Non vengono usati sorgenti del motore né codice trapelato.
- Né il binario di input né l'output ricompilato vengono distribuiti. Non è incluso alcun file del gioco: il giocatore ha bisogno della
  propria copia Steam di Call of Duty: Black Ops II. Il convertitore accetta solo l'esatta build Steam (SHA-256), e
  il possesso viene verificato a ogni avvio del gioco dal client Steam del giocatore: il gioco si rifiuta di partire quando Steam
  non è connesso o l'account non lo possiede.
- La protezione anticopia viene eseguita come l'ha scritta il gioco; nulla al suo interno viene rimosso con patch. Questo repository non offre alcun modo
  per rimuovere il wrapper SteamStub di Valve.
- Al servizio online che il gioco si aspetta risponde un responder locale scritto a partire dalle richieste del gioco stesso: nessun
  server Activision viene contattato, non c'è gioco online, e il possesso del gioco viene comunque verificato dallo Steam del giocatore.
  Le regole di partita sono state scritte in una clean room.
- Il codice scritto qui è distribuito con licenza GPL-3.0 ([LICENSE](LICENSE)).
- Unicorn (GPL-2.0) non viene mai combinato con codice GPL-3.0 in un binario distribuito. L'app include la libreria di Unicorn non modificata
  come file separato accanto agli altri strumenti; viene collegata al binario del gioco sul Mac del giocatore
  ([tools/boot.py](tools/boot.py)), e quel binario non viene mai distribuito. Ogni release pubblica i sorgenti dell'esatta
  versione di Unicorn che include (`unicorn-<version>-source.tar.gz`), come richiede la GPL-2.0. Le licenze di ogni
  strumento incluso si trovano nell'app, in `Contents/Resources/toolchain/licenses`.

Questo è un progetto amatoriale non ufficiale. Call of Duty e Black Ops sono marchi di Activision Publishing, Inc. Steam è un
marchio di Valve Corporation. Questo progetto non è affiliato a, approvato da né sponsorizzato da Activision, Treyarch,
Microsoft o Valve. Per giocare, compra il gioco.

## Star History

<a href="https://www.star-history.com/#adrbn/bo2-mac&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
  </picture>
</a>

<div align="center"><img src="docs/assets/footer.svg" alt="" width="100%"></div>
