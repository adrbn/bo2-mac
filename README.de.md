<div align="center">

<picture>
  <source media="(prefers-color-scheme: light)" srcset="docs/assets/hero-public-light.svg">
  <img src="docs/assets/hero-public.svg" alt="bo2-mac: Call of Duty: Black Ops II, statisch rekompiliert von x86-32 Windows zu nativem ARM64 macOS, aus deiner eigenen Steam-Kopie. Auf einem Apple M1: Kampagne 51 bis 60 Bilder pro Sekunde, Zombies 57 bis 60, Mehrspieler gegen Bots 53 bis 60." width="100%">
</picture>

# bo2-mac

**Call of Duty: Black Ops II, statisch rekompiliert von 32-Bit-x86-Windows zu nativem ARM64-macOS.**<br>
Kampagne, Zombies und Mehrspieler gegen Bots, auf deinem Mac gebaut aus deiner eigenen Steam-Kopie.

<a href="README.md">English</a> · <a href="README.fr.md">Français</a> · <a href="README.es.md">Español</a> · <a href="README.pt-BR.md">Português</a> · <b>Deutsch</b> · <a href="README.it.md">Italiano</a> · <a href="README.ru.md">Русский</a> · <a href="README.ja.md">日本語</a> · <a href="README.ko.md">한국어</a> · <a href="README.zh.md">中文</a>

<a href="#installation"><img src="docs/assets/btn-play.svg" alt="Spielen" height="40"></a>&nbsp;
<a href="#ergebnisse"><img src="docs/assets/btn-results.svg" alt="Ergebnisse" height="40"></a>&nbsp;
<a href="#pipeline"><img src="docs/assets/btn-pipeline.svg" alt="Pipeline" height="40"></a>&nbsp;
<a href="#erkenntnisse"><img src="docs/assets/btn-findings.svg" alt="Erkenntnisse" height="40"></a>&nbsp;
<a href="#roadmap"><img src="docs/assets/btn-roadmap.svg" alt="Roadmap" height="40"></a>

<a href="https://news.ycombinator.com/item?id=50449891"><img src="docs/assets/btn-hackernews.svg" alt="Auf Hacker News vorgestellt" height="44"></a>

![source: x86-32 Windows](https://img.shields.io/badge/source-x86--32%20Windows-ff6a00?style=flat-square&labelColor=0b0b0c)
![target: ARM64 macOS](https://img.shields.io/badge/target-ARM64%20macOS-ff6a00?style=flat-square&labelColor=0b0b0c)
![lifter: Remill, LLVM 17](https://img.shields.io/badge/lifter-Remill%20%E2%86%92%20LLVM%2017-ff6a00?style=flat-square&labelColor=0b0b0c)
![renderer: D3D11 to Metal](https://img.shields.io/badge/renderer-D3D11%20%E2%86%92%20Metal-ff6a00?style=flat-square&labelColor=0b0b0c)
![Wine: none](https://img.shields.io/badge/Wine-none-ff6a00?style=flat-square&labelColor=0b0b0c)
![game files in this repository: none](https://img.shields.io/badge/game%20files-none-f4f1ea?style=flat-square&labelColor=0b0b0c)
[![latest release](https://img.shields.io/github/v/release/adrbn/bo2-mac?style=flat-square&labelColor=0b0b0c&color=ff6a00&label=release)](https://github.com/adrbn/bo2-mac/releases/latest)
![status: beta](https://img.shields.io/badge/status-beta-f4f1ea?style=flat-square&labelColor=0b0b0c)

<sub>[Warum](#warum) · [Zusammenfassung](#zusammenfassung) · [Methode](#methode) · [Grenzen](#grenzen) · [Repository](#repository) · [Rechtliches](#rechtliches)</sub><br>
<sub>Beta, 03.11.2026 · gemessen auf einem Apple M1 (4 Performance- und 4 Effizienzkerne, 16 GB)</sub>

</div>

<p align="center"><a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4"><img src="docs/assets/preview.gif" alt="Kampagne, Zombies und Mehrspieler laufen nativ auf macOS" width="560"></a><br>
<sub>Kampagne (Angola), Zombies (Town) und Mehrspieler (Hijacked, gegen Bots) auf einem Apple M1, während des Spielens aufgenommen; die Zahl oben rechts sind die Bilder pro Sekunde, und die Bildschirmaufnahme selbst kostet einige davon, vor allem in der Kampagne. <a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4">Video mit 80 Sekunden (MP4, 20 MB)</a></sub></p>

> [!IMPORTANT]
> **Dieses Repository enthält weder Spieldateien noch übersetzten Spielcode.** Es verwandelt deine eigene Steam-Kopie von Black Ops
> II auf deinem Mac in eine App für deinen Mac. Der Code des Spiels landet in dieser App, deshalb kann dir niemand eine
> fertige geben: Jeder Spieler baut seine eigene, und sie darf nicht weitergegeben werden. Inoffizielles Fanprojekt, nicht verbunden mit
> Activision, Treyarch oder Microsoft. Um zu spielen, kaufe das Spiel. Beta: Lies, [was bewiesen ist und was nicht](docs/PRERELEASE.md).

---

<img src="docs/assets/rule-01.svg" alt="" width="100%">

## Warum

Black Ops II ist nie für den Mac erschienen, und auf Apple Silicon war es bisher nicht wirklich spielbar.
[AppleGamingWiki](https://www.applegamingwiki.com/wiki/Call_of_Duty:_Black_Ops_II) führt es unter CrossOver als unspielbar und unter Rosetta 2
als nicht unterstützt. Übrig bleibt eine Windows-ARM-virtuelle-Maschine in Parallels, mit Berichten,
die von 1 Bild pro Sekunde in Matches auf einem M1 Max bis zu 60–80 auf M3 und M4 reichen
([MacGamingDB](https://macgamingdb.app/games/call-of-duty-black-ops-ii)). Hier wird das Spiel einmal übersetzt, bevor es
startet, in eine native ARM64-App: 51–60 Bilder pro Sekunde auf einem Standard-M1, ohne Windows und ohne virtuelle Maschine. Nativ zu sein
heißt auch, dass es sich wie jede Mac-App korrigieren und erweitern lässt: Splitscreen, den die PC-Version nie hatte, ist
als experimentelle Option in Zombies verfügbar.

## Zusammenfassung

bo2-mac nimmt die drei ausführbaren Dateien von Black Ops II, die Kampagne (`t6sp.exe`), Zombies (`t6zm.exe`) und
den Mehrspieler (`t6mp.exe`), und wandelt ihren x86-Maschinencode vorab in ARM64-Maschinencode um.
[Remill](https://github.com/lifting-bits/remill) hebt jede Funktion auf LLVM IR, eine Kette eigener Passes schreibt
dieses IR um, und das Ergebnis wird mit einer von Hand geschriebenen Win32-Runtime gelinkt, deren Direct3D 11 von Metal gezeichnet wird. Es gibt
kein Wine und keinen JIT: Das Spiel läuft als nativer Code, in einem einzigen Prozess. Etwa 150.000 Funktionen werden über die
drei ausführbaren Dateien hinweg übersetzt. Nur die kurzen Sequenzen, die der Kopierschutz versteckt, laufen in einem kleinen x86-Interpreter, ebenso
jeder Code, den die Übersetzung verpasst hat, sodass eine Lücke darauf ausgelegt ist, Geschwindigkeit zu kosten, statt das Spiel abstürzen zu lassen.

Auf einem Apple M1 läuft die Kampagne mit **51–60 Bildern pro Sekunde**, und jede Mission lädt und übergibt an die nächste,
bis zum Abspann. Zombies wird solo auf **sechs Karten** über die eigenen Menüs des Spiels mit **57–60** gespielt.
Der Mehrspieler wird gegen Bots über seine echten Menüs gespielt, elf Spielmodi, mit **53–60**. Eine App,
`Black Ops II.app`, öffnet die Kampagne, und die Einträge CAMPAIGN / ZOMBIES / MULTIPLAYER jedes Menüs wechseln zu den
anderen Spielen.

Es wird nichts von jemand anderem als dir heruntergeladen: Der Konverter liest die ausführbaren Dateien der Kopie, die du gekauft hast, prüft sie
Byte für Byte und baut die App auf deinem Rechner.

<img src="docs/assets/rule-02.svg" alt="" width="100%">

## Ergebnisse

| | gemessen | Bedingungen |
|---|---|---|
| Kampagne | **51–60 fps** | jede Mission lädt und schließt bis zum Abspann an; nicht alle durchgehend gespielt |
| Zombies | **57–60 fps** (Die Rise 43–58) | TranZit, Mob of the Dead, Nuketown, Origins, Buried, Die Rise, solo, über die echten Menüs |
| Mehrspieler gegen Bots | **53–60 fps** | elf Spielmodi über die echten Menüs: eine volle Runde, die Punktetafel, zurück in die Lobby |
| Spielwelt gegen Echtzeit | **≈ 1,0×** | die eigene Serveruhr des Spiels gegen die Wanduhrzeit |
| Konvertierung auf dem Mac des Spielers | **etwa 1 Stunde 45** auf einem M1 (allein die Kampagne: etwa 45 Minuten) | die Kampagne ist spielbar, sobald sie konvertiert ist; Zombies und Mehrspieler folgen |
| Festplatte | Spitze etwa **10 GB**, etwa **4 GB** bleiben | nach der Konvertierung bleiben nur die gelinkten Binärdateien |

Die Bildrate zählt die Drawables, die tatsächlich auf dem Bildschirm präsentiert werden, geprüft am Bildschirm und am Metal HUD.

**Was du spielen kannst**

<table>
<tr>
<td width="33%" valign="top"><img src="docs/assets/card-public-campaign.svg" alt="Kampagne" width="100%"><br>
Das echte Hauptmenü: NEW STORY, RESUME STORY, REPLAY MISSION. Von <code>angola</code> bis zum Abspann
laden die Missionen und übergeben an die nächste. Strike-Force-Missionen starten mit einem Klick.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-zombies.svg" alt="Zombies" width="100%"><br>
Die sechs Karten, solo: Hauptmenü, Lobby, Match, Game Over, Punktetafel, dann ein weiteres Match im selben Prozess.
DLC-Karten sind spielbar, wenn dein Steam-Konto sie besitzt. Splitscreen für zwei Spieler ist experimentell.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-multi.svg" alt="Mehrspieler" width="100%"><br>
Custom Games gegen Bots aus den echten Menüs: CHOOSE CLASS, das HUD, eine volle Runde, die Punktetafel
und zurück in die Lobby. Die Matchregeln sind das eigene Skript dieses Projekts.</td>
</tr>
</table>

Alle drei Modi: Tastatur und Maus oder ein DualSense als XInput über GameController.framework; Ton an; die Bewegungstasten
folgen ihrer Position auf deiner Tastatur (ZQSD auf AZERTY, WASD auf QWERTY), es sei denn, du hast sie in den
Optionen des Spiels selbst belegt.

<img src="docs/assets/rule-03.svg" alt="" width="100%">

## Pipeline

In fünf Schritten, alle auf dem Mac des Spielers:

1. **Prüfen.** Der Konverter akzeptiert nur den exakten Steam-Build jeder ausführbaren Datei, geprüft per SHA-256. Der
   Launcher fragt den Steam-Client auf deinem Mac, ob das angemeldete Konto das Spiel besitzt, und verweigert den Start andernfalls.
2. **Katalogisieren und heben.** Ghidra listet die Funktionen deiner eigenen ausführbaren Datei auf; Remill v6.0.1 hebt jede einzelne auf
   LLVM IR. Fünf Patches für den Lifter liegen in [tools/remill-patches/](tools/remill-patches/), darunter
   der x87-Rundungsmodus und einer, der für jede Funktion einen laufenden Remill forkt, statt dessen x86-Semantik neu zu laden.
3. **Falten.** Das IR von Remill ist korrekt, aber langsam: Jeder Gast-Speicherzugriff, jedes Flag und jeder x87-Push ist ein Aufruf oder ein Store
   in eine `State`-Struktur. Unsere Passes schreiben es vor `clang -O2` um: Gast-Lese- und Schreibzugriffe werden inlined, `lock`-
   Sequenzen werden zu Hardware-Atomics, das Gast-`ret` bleibt im gehobenen Code, und der x87-Stack wird zu
   lokalen Variablen befördert.
4. **Runtime.** [runtime/](runtime/) implementiert die Windows-Imports, die das Spiel wirklich aufruft, mit exakten stdcall-
   Arities (die der Steam API aus Valves öffentlichen Headern übernommen). Der Gastspeicher ist eine einzige 4-GiB-Abbildung. Direct3D 11 ist
   eine Fake-COM-Schicht über Metal; DXBC-Shader laufen durch [HLSLcc](third_party/HLSLcc). XAudio2 geht an CoreAudio, und
   die Videos werden nativ mit libvpx dekodiert.
5. **Threads.** Gast-Threads starten als Coroutinen auf einem Host-Thread, was die meisten Quellen von
   Nichtdeterminismus beseitigt und einen Lauf wiederholbar macht. Die schnelle Konfiguration verlagert den Server, das Render-Backend, Soundmischung und Dekodierung sowie die Job-
   Worker auf echte Host-Threads. ARM ist nicht TSO, daher brauchte das echte Speicherbarrieren.

**Der Kopierschutz wird ausgeführt, nicht umgangen.** Das Spiel versteckt Integritätsprüfungen hinter Hardware-Exceptions,
Single-Step-Traps und Code, den es in den Heap entschlüsselt. Eine kleine x86-Maschine auf Basis von Unicorn
([x86_guest_vm.cpp](runtime/win32/x86_guest_vm.cpp), [heap_code.cpp](runtime/win32/heap_code.cpp)) führt genau diese
Sequenzen aus und gibt die Kontrolle an den rekompilierten Code zurück. Kein Prüfergebnis wird erzwungen und kein Schlüssel von Hand gesetzt.

**Eine Lücke in der Übersetzung ist darauf ausgelegt, Geschwindigkeit zu kosten, nicht einen Absturz.** Wenn das Spiel auf x86-Code stößt, der nie gehoben wurde, führt dieselbe
Maschine seine Original-Bytes aus ([interp.cpp](runtime/win32/interp.cpp)) und gibt die Kontrolle am nächsten übersetzten Block, Aufruf oder Rücksprung
an den rekompilierten Code zurück. Jede interpretierte Adresse wird protokolliert (`[INTERP]`).

**Der Online-Dienst wird lokal beantwortet.** Zombies und Mehrspieler erwarten Demonware; ein aus den
eigenen Anfragen des Spiels geschriebener Responder ([dw_local.cpp](runtime/win32/dw_local.cpp)) beantwortet Lobby, Speicher und Ticket auf deinem Mac.
Nichts verlässt die Maschine.

<details>
<summary>Warum nicht Wine, CrossOver, DXVK oder DXMT?</summary>

Sie übersetzen Windows-Aufrufe zur Laufzeit und lassen den x86-Code unter Rosetta. Hier ist der x86-Code selbst verschwunden: Er ist
ARM64-Maschinencode, bevor das Spiel startet. DXVK und DXMT sind PE-DLLs, die ihr natives Backend über Wines
unixlib-Mechanismus erreichen; dieser Port hat kein Wine, und sein Direct3D 11 ist eine eigene Schicht über 32-Bit-Gastspeicher. Nur die Shader-
Übersetzung wurde von außen gebraucht, und HLSLcc, eine schlichte C++-Bibliothek, deckt sie ab.
</details>

<img src="docs/assets/rule-04.svg" alt="" width="100%">

## Erkenntnisse

Jeder Eintrag ist ein auf dem Bildschirm beobachtetes Symptom, die gemessene Ursache und die Wirkung des Fixes.

<details>
<summary><b>Dreizehn Erkenntnisse</b>, vom x87-Rundungsmodus bis zu einem falschen Start innerhalb einer einzigen Instruktion</summary>


**F1 · Der Lifter ignorierte den x87-Rundungsmodus.** In Zombies wackelte die Ansicht in Bauchlage. `fistp` rundete zur nächsten Zahl, wo
das Spiel Abschneiden verlangt hatte, im Netzwerkcodec der Ansichtshöhe. Die beim Client angekommene Höhe war
bei 8 von 8 Stichproben falsch, und bei 0 von 8 nach einem Remill-Patch und einem erneuten Heben des Codecs.

**F2 · Ein reserviertes Bit des x87-Steuerworts.** Eine Kampagnenmission fror in einer Effektschleife ein, nur mit echten
Threads. Der Lifter speicherte Bit 6 des Steuerworts anders als echte Hardware. Ein einzeiliger Remill-Patch und vier
erneut gehobene Runtime-Funktionen behoben es, ohne Verlangsamung.

**F3 · Ein CPU-Feature-Flag ändert den Algorithmus.** Unser `IsProcessorFeaturePresent` antwortete „kein SSE2“, also nahm die C-
Runtime ihr x87-`ceil`, das unter unserer Runtime zur nächsten Zahl rundete. Ein Effekt griff dann ewig auf Index −1 zu: Zwei
Missionen froren ein.

**F4 · Unser eigenes Log verursachte das Ruckeln.** Hänger von 170–430 ms waren der Render-Thread, blockiert in `write()`: unser Log,
umgeleitet in eine Datei auf einer fast vollen Festplatte. Es über eine Pipe an einen Kindprozess zu schicken, senkte die Hänger pro Lauf
von 12 und 8 auf 0 und 0.

**F5 · Die Shader-Übersetzung verschob Konstanten um 4 Bytes.** HLSLcc platzierte ein `float2` nach einem `float` bei Offset 24, wo
Direct3D es bei 20 liest. 31 von 2.664 Shadern waren betroffen, darunter die Augen und die Fadenkreuze.

**F6 · Der Schutz muss laufen.** Das Optionsmenü fror 8 von 8 Mal ein. Eine Integritätsprüfung überschreibt ihre eigene Rücksprungadresse, um auf
in den Heap entschlüsselten Code zu zeigen, was die Runtime ignorierte. Diesen Code auszuführen, wie Windows es tut, ergab
3 von 3 ohne Einfrieren.

**F7 · Unsere eigenen Sicherheitsvorkehrungen waren das Hindernis.** Eine unserer Vorkehrungen stoppte das Spiel nach 28 Sekunden Spielzeit. Die größten
Gewinne des Projekts kamen vom Entfernen unserer eigenen Vorkehrungen und von False Sharing, nicht von cleverem Code.

**F8 · Ein leerer API-Stub sieht aus wie ein defekter Dienst.** Zwei Stubs, die nichts zurückgaben (`CryptGenRandom`,
`StringFromGUID2`), sahen in Zombies wie ein Netzwerkausfall aus.

**F9 · Thread-Prioritäten sind Teil des Programms.** `SetThreadPriority` war ein Stub. Die Anforderung des Mixers an
macOS weiterzureichen, senkte die kurzen Audiopuffer eines Laufs von 908–1872 auf 7–71, und die Tonaussetzer gleich mit.

**F10 · Natives Skinning, Bit für Bit.** CPU-Skinning in NEON neu geschrieben liefert dieselben Bits wie das ursprüngliche x86 und brachte
eine aufwendige Szene von 17,8 auf 30,3 fps.

**F11 · Decals brauchen den Depth Bias des Spiels.** Brandspuren und Blut erschienen oder verschwanden bei einer leichten Drehung der
Kamera. Das Spiel verlangt einen Depth Bias für Decals; unsere Schicht ließ ihn bei null. Die Werte des Spiels an Metal zu übergeben, behob es.

**F12 · Der Render-Thread kopierte jedes Bild die Geometrie des Levels.** Im Mehrspieler und in Zombies kopierte er pro Bild 23–63 MB
Vertexdaten zur GPU, größtenteils statisch. Statische Puffer in Metal resident zu halten, senkte die Kopien auf
3,5–5 MB pro Bild: Mehrspieler 51,8 → 60 fps, Zombies 43,1 → 58,2. Dieselbe Änderung bewirkt in der Kampagne nichts,
wo das Tempo vom Scheduler-Thread bestimmt wird. Eine Beschleunigung hilft nur dem Modus, dessen Engpass sie trifft.

**F13 · Ein falscher Start innerhalb einer Instruktion kostete im Kampf die halbe Bildrate.** In den ersten Konvertierungen aus einer
frischen Steam-Kopie fiel Zombies auf 25 fps, sobald der Spieler feuerte. Heiße Funktionen liefen im Interpreter: Der
Katalog setzte den Beginn der nächsten Funktion in die erste Instruktion einer Wurzel, sodass ihr begrenzter Sweep
null Bytes dekodierte und die Wurzel kommentarlos verworfen wurde. Solche Wurzeln ohne die Begrenzung zu durchlaufen, wie es der
Einzel-Lifter schon tat, brachte sie zurück in den übersetzten Code.

</details>

<img src="docs/assets/rule-05.svg" alt="" width="100%">

## Methode

**Erst vorhersagen, dann messen.**

- Eine Vorhersage wird geschrieben, bevor das Ergebnis gelesen wird: etwa 1.100 nummerierte zum Zeitpunkt dieser Beta.
- Mindestens drei Läufe pro Konfiguration. Das Rauschen zwischen Läufen beträgt 30–64 %, also ist ein Unterschied unter etwa 40 % bei n = 3 nicht erkennbar.
- Eine Änderung muss beweisen, dass sie aktiv ist: Das Log muss zeigen, dass sie greift.
- Ein Lauf, der das Level nie erreicht, ist ein toter Lauf, kein Datenpunkt.
- Der Bildschirm schlägt das Log. Wenn der Bildschirm einer Sonde widerspricht, liegt die Sonde falsch.

<details>
<summary><b>Aufzeichnen und Abspielen, Rücknahmen, Clean Room, wer was macht</b></summary>

**Deterministisches Aufzeichnen und Abspielen.** Da jeder Gast-Thread eine Coroutine auf einem Host-Thread ist, sind die meisten Quellen von
Nichtdeterminismus verschwunden; das Abspielen wird gegen die Aufzeichnung geprüft, nicht vorausgesetzt. Drei Grenzen erfassen jede äußere Eingabe: der Import-Dispatcher, das Leeren der Fensternachrichten und die
verbrauchten Audiopuffer. Das Journal ist etwa 16 MB groß. Es erlaubt, denselben Lauf zweimal auszuführen, wobei genau ein Flag geändert ist.

**Das Messinstrument ist es, das versagt.** Fast 200 Befunde wurden unterwegs zurückgenommen. Der größte Anteil
(31 %) war ein Instrument, das eine andere Frage beantwortete; der nächste (17 %) eines, das gar nicht antworten konnte. Deshalb
nennt hier jedes Orakel seine Eingabe und verweigert die Antwort, wenn diese Eingabe fehlt.

**Clean Room für die Matchregeln.** Mehrspieler und Zombies beziehen ihre Matchregeln aus einer täglichen Patchdatei, die Steam
nicht mehr ausliefert. Der Ersatz in [runtime/match_rules/](runtime/match_rules/) wurde von einem Team geschrieben, das das
Originalskript nie gesehen hat: Ein Team schrieb aus dem Spiel eine Verhaltensspezifikation, das andere schrieb den Code allein aus dieser
Spezifikation, und beide sprachen nur über schriftliche Fragen und Antworten miteinander, mit Datum aufbewahrt.

**Wer was macht.** KI-Agenten liefern den Durchsatz: die Shims schreiben, Millionen Aufrufe pro Lauf bisektieren, Zehntausende
Zeilen lesen. Der Mensch liefert das Ziel, die Korrekturen anhand dessen, was wirklich auf dem Bildschirm zu sehen ist, und lehnt den
schmeichelhaften Schluss ab. Das ist ein Spiel, Monate Arbeit und viel manuelles Eingreifen.

</details>

<img src="docs/assets/rule-07.svg" alt="" width="100%">

## Grenzen

- **Nur lokal.** Mehrspieler und Zombies laufen gegen den Responder auf deinem Mac. Es gibt kein Online-Spiel und kein LAN. League Play wird verweigert.
- **Die Matchregeln sind unsere.** In einem Clean Room geschrieben (siehe [Methode](#methode)); das Verhalten kann stellenweise vom Original
  abweichen.
- **Hitze.** Der Mac wird bei langen Sitzungen sehr heiß (111–116 °C gemeldet auf dem M1). Spiele am Netz: Im Akkubetrieb
  drosselt macOS das Spiel stark. Das Netzteil vermeidet die Verlangsamung, nicht die Hitze.
- **Erstes Spiel auf jeder Karte.** In Zombies und Mehrspieler werden Shader für Metal übersetzt, während die Karte lädt, sodass
  dieses erste Laden länger dauert und die erste Minute noch leicht ruckeln kann. In der Kampagne werden sie übersetzt, wenn
  sie zum ersten Mal erscheinen, sodass die Ladesequenz flüssig bleibt und der erste Durchgang durch einen Ort ruckeln kann. Alles
  wird zwischengespeichert: Beim nächsten Mal lädt es schnell und läuft flüssig.
- **Fenster.** Das Spiel läuft in einem Fenster von 1280×720.
- **Andere Macs.** Nur ein M1 wurde gemessen. M2, M3 und M4 sollten schneller sein, aber es wird keine Zahl behauptet.
- **Kampagne.** Jede Mission lädt und schließt an; nicht jede wurde durchgehend gespielt.
- **Zombies.** Kein Online- oder LAN-Koop, daher sind Grief und Turned nicht spielbar. Lokaler Splitscreen für zwei Spieler ist
  experimentell: Spieler 2 erscheint, wenn Runde 1 beginnt.
- **macOS-Audio.** Wenn der `coreaudiod` des Systems nicht gesund ist, bleibt die Kampagne beim ersten Video hängen. Prüfe, dass
  `afplay -t 1 /System/Library/Sounds/Tink.aiff` etwa 1 s dauert; ein Neustart des Macs behebt es.

<img src="docs/assets/rule-08.svg" alt="" width="100%">

## Installation

Du brauchst einen Mac mit Apple Silicon, mindestens **20 GB frei** (die Konvertierung erreicht eine Spitze von etwa 10 GB, etwa 4 GB bleiben), und
Steam für Mac, angemeldet mit dem Konto, das das Spiel besitzt.

**macOS 26 oder neuer: kein Terminal.** Lade `Black Ops II.app` von den [Releases](https://github.com/adrbn/bo2-mac/releases) herunter
und öffne sie. Sie bringt ihre eigenen Übersetzungswerkzeuge mit (LLVM 17, Remill, Unicorn, libvpx, HLSLcc, capstone, gebaut von
[scripts/make_toolchain.sh](scripts/make_toolchain.sh)) und keine Spieldatei. Beim ersten Mal bietet macOS an, Apples kostenlose
Command Line Tools zu installieren: Klicke auf „Installieren“, dann im Fenster der App auf „Erneut versuchen“.
Einmal am Tag beim Start sucht die App nach einem neueren Release und bietet an, es zu installieren. „Installieren, diese Version behalten“ behält
die aktuelle App daneben, umbenannt mit ihrer Version, sodass das Öffnen dieser Version zurückführt. Ein Update behält, was schon konvertiert ist, und erledigt nur neu, was seine Änderungen berühren: Ein Fix an der Runtime verlinkt die drei Modi auf einem M1 in etwa 3 Minuten neu.

**macOS 14 oder 15: über das Terminal**, mit Homebrew:

```bash
git clone https://github.com/adrbn/bo2-mac.git && cd bo2-mac
brew install llvm@17 cmake ninja libvpx unicorn
pip3 install capstone
scripts/make_app.sh --from-source
```

Öffne `Black Ops II.app`, zeige auf deinen Spieleordner und lass sie konvertieren: etwa 1 Stunde 45 auf einem M1, die Kampagne zuerst in etwa 45 Minuten. Die Kampagne ist
spielbar, sobald sie konvertiert ist: Spiele sie dann, und Zombies und Mehrspieler werden konvertiert, wenn du sie zum ersten Mal
im Menü des Spiels wählst. Andernfalls läuft die Konvertierung durch alle drei weiter. Fällt die Festplatte unter
8 GB frei, hält die Konvertierung sauber an und setzt dort fort, wo sie war.

**Zombies im Splitscreen (experimentell).** Schließe einen zweiten Controller an und drücke darauf in der Zombies-Lobby
Start oder A: Spieler 2 steigt ein, wie auf einer Konsole. B auf diesem Controller oder das Abziehen nimmt Spieler 2 wieder
heraus. Spieler 1 spielt die obere Hälfte, Spieler 2 die untere; der Bildschirm teilt sich beim Start der Partie, und
Spieler 2 erscheint, wenn Runde 1 beginnt. Steigt niemand ein, bleibt das Spiel bei einem Spieler im Vollbild. Es gibt nichts anzuhaken.

**Aktualisieren, oder wenn die Konvertierung stoppt.** Führe `git pull && scripts/make_app.sh --from-source` im Repository-
Ordner aus: Was bereits konvertiert ist, bleibt erhalten, und die Konvertierung wird fortgesetzt. Ein Update wiederholt nur, was seine Änderungen betreffen: Ein
Fix an der Runtime linkt die drei Modi in etwa 3 Minuten auf einem M1 neu, statt erneut zu konvertieren. Schlägt es trotzdem fehl,
eröffne ein Issue mit dem Log, das es nennt.

Der Konverter akzeptiert nur den exakten Steam-Build, geprüft per SHA-256. Die Steam-Executables sind durch Valves
SteamStub-Wrapper verschlüsselt, und **dieses Repository bietet keine Möglichkeit, ihn zu entfernen**: Der Konverter erwartet die drei entschlüsselten
Abbilder in `~/.bo2-native/steam/` (`t6sp.exe.unpacked.exe`, `t6mp.exe.unpacked.exe`, `t6zm.exe.unpacked.exe`). Du erstellst
sie selbst aus der Kopie, die du gekauft hast, mit einem separaten Werkzeug deiner Wahl. Nur der Wrapper muss weg: Die obigen Dateinamen
und die SHA-256-Prüfung sagen dir sofort, ob eine Datei richtig ist. Teile diese Dateien niemals.

**DLC.** Der Launcher fragt deinen Steam-Client, welche DLCs dein Konto besitzt, genau wie das Spiel es unter Windows tut. Eine DLC-Karte
ist spielbar, wenn du sie besitzt und ihre Dateien in deinem Spieleordner liegen.

<img src="docs/assets/rule-09.svg" alt="" width="100%">

## Repository

| Pfad | Inhalt |
|---|---|
| [runtime/win32/](runtime/win32/) | die Win32-Runtime: kernel32, der Scheduler, Direct3D 11, Ton, Eingabe, der Responder für den lokalen Dienst |
| [runtime/gfx/](runtime/gfx/) | das Metal-Backend: Präsentation, Shader-Cache, Render Targets |
| [runtime/match_rules/](runtime/match_rules/) | die Clean-Room-Matchregeln für Mehrspieler und Zombies |
| [tools/](tools/) | der Lift-Treiber, der Katalog, die IR-Passes, die Disassemblierungs- und Analysewerkzeuge |
| [tools/remill-patches/](tools/remill-patches/) | die Patches, die auf Remill v6.0.1 aufsetzen |
| [scripts/](scripts/) | `make_app.sh`, der Steam-Installer und -Konverter, die Launcher |
| [third_party/HLSLcc](third_party/HLSLcc) | DXBC-zu-Metal-Shader-Übersetzung |
| [docs/](docs/) | der Prerelease-Status, die Toolchain |

<img src="docs/assets/rule-10.svg" alt="" width="100%">

## Roadmap

1. **Diese Beta**: die drei Modi aus einer frischen Steam-Kopie, auf anderen Macs als dem, der sie gebaut hat. Deine Berichte
   (Mac-Modell, Bildrate, was kaputtging) entscheiden, was als Nächstes kommt.
   Erster externer Bericht, 05.11.2026: Auf einem M2 Pro (16 GB) lief die Konvertierung durch und das Spiel startete, nach drei
   unterwegs gefundenen Fixes ([#4](https://github.com/adrbn/bo2-mac/issues/4), [#5](https://github.com/adrbn/bo2-mac/issues/5), [#6](https://github.com/adrbn/bo2-mac/issues/6)).
2. **Kein Terminal**: erledigt. Eine App zum Herunterladen und Öffnen; sie bringt ihre Werkzeuge schon gebaut mit, sodass nichts
   auf deinem Mac kompiliert wird außer dem Spiel selbst, und sie aktualisiert sich selbst (macOS 26 oder neuer; macOS 14 und 15 behalten die
   Terminal-Installation).
3. **Vollständig übersetzt**: Die 46 Codeblöcke, die noch durch den Unicorn-Interpreter laufen, werden vorab
   übersetzt, und jede übersetzte Instruktion wird gegen den Interpreter geprüft;
   jede übersetzte Funktion kann an den Interpreter zurückgegeben werden, um eine falsche durch Halbieren zu finden.
4. **Kein Shader-Warten**: Shader vor dem ersten Match gebaut, statt beim Start (etwa 3 Minuten im Mehrspieler) und
   während des Spiels (Einfrieren, wenn ein neuer Effekt erscheint). Begonnen: Shader werden jetzt in Batches kompiliert, und die aus den
   letzten Sitzungen werden beim Start im Hintergrund kompiliert. Im Mehrspieler auf einem M1 ohne Xcode (einige Versuche, kein
   Benchmark) sank das Shader-Kompilieren während der ersten zwei Minuten eines Matches von 18–25 s auf 10–14 s beim ersten
   Start und auf etwa 5 s beim nächsten; Einfrierer über 300 ms sanken von 15–22 auf 13–15, dann auf 7. Der erste Start
   friert noch etwa genauso oft ein.
5. **Kürzere Konvertierung**: von etwa 2,5 Stunden auf 1 Stunde 45 auf einem M1 gesunken (06.11.2026, gemessen an allen drei Modi:
   die Vorbereitung liest die ausführbare Datei einmal, das Heben und das Falten nutzen jeden Kern, den der Speicher erlaubt). Ein Update wiederholt jetzt
   nur, was sich geändert hat (ein Runtime-Fix: etwa 3 Minuten). Als Nächstes: weniger Optimierung für Code, der selten läuft. Die drei Modi teilen nur 4 % ihrer
   Funktionen, daher brächte es wenig, gemeinsamen Code einmal zu konvertieren.
6. **Schneller, ohne Hitze**: Die Kampagne ist durch den Scheduler-Thread begrenzt, nicht durch das Rendering; Mehrspieler mit 11 Bots
   ist durch den Server-Thread begrenzt. Begonnen: Der Hall läuft in allen drei Modi nativ, rund 40 % weniger Zeit im Sound-Thread. Native Neuschreibungen der anderen heißesten Funktionen kommen als Nächstes, dann Bildraten über 60 für
   120-Hz-Bildschirme.
7. **Controller**: Vibration und Zielhilfe.
8. **Gemeinsam**: Lokaler Splitscreen in Zombies, den die PC-Version nie hatte, ist experimentell (ein Spiel,
   zwei Spieler, obere und untere Hälfte, etwa 28 fps auf einem M1), mit einem Schalter im Menü der App. Als Nächstes: echte UDP-Sockets für Zombies-Koop,
   Grief und Turned sowie Mehrspieler über LAN.
9. **Später**: MetalFX, Nachbearbeitung bei der Präsentation des Bildes Ersatztexturen, geladen aus der eigenen Kopie des Spielers, sowie Schatten und Umgebungsverdeckung per Raytracing auf Macs mit Hardware-Raytracing (M3 und neuer).

<img src="docs/assets/rule-11.svg" alt="" width="100%">

## Rechtliches

Für eine Kopie, die der Nutzer besitzt, auf Hardware, die der Nutzer besitzt.

- Es werden weder Engine-Quellcode noch geleakter Code verwendet.
- Weder die Eingabe-Binärdatei noch die rekompilierte Ausgabe werden verteilt. Es ist keine Spieldatei enthalten: Der Spieler braucht seine
  eigene Steam-Kopie von Call of Duty: Black Ops II. Der Konverter akzeptiert nur den exakten Steam-Build (SHA-256), und
  der Besitz wird bei jedem Spielstart vom Steam-Client des Spielers selbst geprüft: Das Spiel weigert sich zu laufen, wenn Steam
  nicht angemeldet ist oder das Konto es nicht besitzt.
- Der Kopierschutz wird so ausgeführt, wie das Spiel ihn geschrieben hat; nichts davon wird herausgepatcht. Dieses Repository bietet keine Möglichkeit,
  Valves SteamStub-Wrapper zu entfernen.
- Der Online-Dienst, den das Spiel erwartet, wird von einem lokalen Responder beantwortet, der aus den eigenen Anfragen des Spiels geschrieben wurde: Es wird kein
  Activision-Server kontaktiert, es gibt kein Online-Spiel, und der Besitz des Spiels wird weiterhin von Steam des Spielers geprüft.
  Die Matchregeln wurden in einem Clean Room geschrieben.
- Der hier geschriebene Code steht unter der GPL-3.0 ([LICENSE](LICENSE)).
- Unicorn (GPL-2.0) wird in einer verteilten Binärdatei nie mit GPL-3.0-Code kombiniert. Die App bringt die unveränderte
  Bibliothek von Unicorn als separate Datei neben den anderen Werkzeugen mit; sie wird auf dem eigenen Mac des Spielers in die Spiel-Binärdatei gelinkt
  ([tools/boot.py](tools/boot.py)), und diese Binärdatei wird nie verteilt. Jedes Release veröffentlicht den Quellcode der
  exakten Unicorn-Version, die es mitbringt (`unicorn-<version>-source.tar.gz`), wie es die GPL-2.0 verlangt. Die Lizenzen jedes
  mitgelieferten Werkzeugs liegen in der App, in `Contents/Resources/toolchain/licenses`.

Dies ist ein inoffizielles Fanprojekt. Call of Duty und Black Ops sind Marken von Activision Publishing, Inc. Steam ist eine
Marke der Valve Corporation. Dieses Projekt ist nicht verbunden mit, unterstützt von oder gesponsert von Activision, Treyarch,
Microsoft oder Valve. Um zu spielen, kaufe das Spiel.

## Star History

<a href="https://www.star-history.com/#adrbn/bo2-mac&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
  </picture>
</a>

<div align="center"><img src="docs/assets/footer.svg" alt="" width="100%"></div>
