<div align="center">

<picture>
  <source media="(prefers-color-scheme: light)" srcset="docs/assets/hero-public-light.svg">
  <img src="docs/assets/hero-public.svg" alt="bo2-mac : Call of Duty: Black Ops II recompilé statiquement de x86-32 Windows vers ARM64 macOS natif, à partir de votre propre copie Steam. Sur un Apple M1 : campagne de 51 à 60 images par seconde, Zombies de 57 à 60, multijoueur contre des bots de 53 à 60." width="100%">
</picture>

# bo2-mac

**Call of Duty: Black Ops II, recompilé statiquement de x86 32 bits Windows vers ARM64 macOS natif.**<br>
Campagne, Zombies et multijoueur contre des bots, construits sur votre Mac à partir de votre propre copie Steam.

<a href="README.md">English</a> · <b>Français</b> · <a href="README.es.md">Español</a> · <a href="README.pt-BR.md">Português</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.ru.md">Русский</a> · <a href="README.ja.md">日本語</a> · <a href="README.ko.md">한국어</a> · <a href="README.zh.md">中文</a>

<a href="#installation"><img src="docs/assets/btn-play.svg" alt="Jouer" height="40"></a>&nbsp;
<a href="#résultats"><img src="docs/assets/btn-results.svg" alt="Résultats" height="40"></a>&nbsp;
<a href="#pipeline"><img src="docs/assets/btn-pipeline.svg" alt="Pipeline" height="40"></a>&nbsp;
<a href="#constats"><img src="docs/assets/btn-findings.svg" alt="Constats" height="40"></a>&nbsp;
<a href="#feuille-de-route"><img src="docs/assets/btn-roadmap.svg" alt="Feuille de route" height="40"></a>

<a href="https://news.ycombinator.com/item?id=50449891"><img src="docs/assets/btn-hackernews.svg" alt="À la une de Hacker News" height="44"></a>

![source: x86-32 Windows](https://img.shields.io/badge/source-x86--32%20Windows-ff6a00?style=flat-square&labelColor=0b0b0c)
![target: ARM64 macOS](https://img.shields.io/badge/target-ARM64%20macOS-ff6a00?style=flat-square&labelColor=0b0b0c)
![lifter: Remill, LLVM 17](https://img.shields.io/badge/lifter-Remill%20%E2%86%92%20LLVM%2017-ff6a00?style=flat-square&labelColor=0b0b0c)
![renderer: D3D11 to Metal](https://img.shields.io/badge/renderer-D3D11%20%E2%86%92%20Metal-ff6a00?style=flat-square&labelColor=0b0b0c)
![Wine: none](https://img.shields.io/badge/Wine-none-ff6a00?style=flat-square&labelColor=0b0b0c)
![game files in this repository: none](https://img.shields.io/badge/game%20files-none-f4f1ea?style=flat-square&labelColor=0b0b0c)
[![latest release](https://img.shields.io/github/v/release/adrbn/bo2-mac?style=flat-square&labelColor=0b0b0c&color=ff6a00&label=release)](https://github.com/adrbn/bo2-mac/releases/latest)
![status: beta](https://img.shields.io/badge/status-beta-f4f1ea?style=flat-square&labelColor=0b0b0c)

<sub>[Pourquoi](#pourquoi) · [Résumé](#résumé) · [Méthode](#méthode) · [Limites](#limites) · [Dépôt](#dépôt) · [Mentions légales](#mentions-légales)</sub><br>
<sub>Bêta, 2026-11-03 · mesuré sur un seul Apple M1 (4 cœurs performance + 4 cœurs efficacité, 16 Go)</sub>

</div>

<p align="center"><a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4"><img src="docs/assets/preview.gif" alt="Campagne, Zombies et multijoueur tournant nativement sur macOS" width="560"></a><br>
<sub>Campagne (Angola), Zombies (Town) et multijoueur (Hijacked, contre des bots) sur un Apple M1, enregistrés en jouant ; le nombre en haut à droite est le nombre d'images par seconde, et l'enregistrement d'écran lui-même en coûte quelques-unes, surtout en campagne. <a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4">Vidéo de 80 secondes (MP4, 20 Mo)</a></sub></p>

> [!IMPORTANT]
> **Ce dépôt ne contient aucun fichier du jeu et aucun code de jeu traduit.** Il transforme votre propre copie Steam de Black Ops
> II en une app pour votre Mac, sur votre Mac. Le code du jeu se retrouve à l'intérieur de cette app : personne ne peut donc vous en
> fournir une toute faite, chaque joueur construit la sienne, et elle ne peut pas être partagée. Projet de fan non officiel, sans lien avec
> Activision, Treyarch ni Microsoft. Pour jouer, achetez le jeu. Bêta : lisez [ce qui a été prouvé et ce qui ne l'a pas été](docs/PRERELEASE.md).

---

<img src="docs/assets/rule-01.svg" alt="" width="100%">

## Pourquoi

Black Ops II n'est jamais sorti sur Mac, et sur Apple Silicon il n'était pas vraiment jouable jusqu'ici.
[AppleGamingWiki](https://www.applegamingwiki.com/wiki/Call_of_Duty:_Black_Ops_II) le classe comme injouable sous
CrossOver et non pris en charge sous Rosetta 2. Reste une machine virtuelle Windows ARM dans Parallels, avec des retours
allant de 1 image par seconde en match sur un M1 Max à 60–80 sur M3 et M4
([MacGamingDB](https://macgamingdb.app/games/call-of-duty-black-ops-ii)). Ici, le jeu est traduit une seule fois, avant son
démarrage, en une app ARM64 native : 51–60 images par seconde sur un M1 de base, sans Windows et sans machine virtuelle. Être
natif signifie aussi qu'il peut être corrigé et étendu comme n'importe quelle app Mac : l'écran partagé, que la version PC n'a jamais eu, est
disponible comme option expérimentale dans Zombies.

## Résumé

bo2-mac prend les trois exécutables de Black Ops II, la campagne (`t6sp.exe`), Zombies (`t6zm.exe`) et le
multijoueur (`t6mp.exe`), et transforme leur code machine x86 en code machine ARM64 en amont.
[Remill](https://github.com/lifting-bits/remill) élève chaque fonction en LLVM IR, une chaîne de passes maison réécrit
cet IR, et le résultat est lié à un runtime Win32 écrit à la main, dont le Direct3D 11 est dessiné par Metal. Il n'y a
ni Wine ni JIT : le jeu s'exécute en code natif, dans un seul processus. Environ 150 000 fonctions sont traduites sur les
trois exécutables. Seules les courtes séquences que la protection anticopie dissimule tournent dans un petit interpréteur x86, ainsi que
tout code que la traduction a manqué, de sorte qu'un trou est conçu pour coûter de la vitesse plutôt que faire planter le jeu.

Sur un Apple M1, la campagne tourne à **51–60 images par seconde**, et chaque mission se charge et enchaîne sur la suivante,
jusqu'au générique de fin. Zombies se joue en solo sur **six cartes** via les menus du jeu, à **57–60**.
Le multijoueur se joue contre des bots via ses vrais menus, onze modes de jeu, à **53–60**. Une seule app,
`Black Ops II.app`, ouvre la campagne, et les entrées CAMPAIGN / ZOMBIES / MULTIPLAYER de chaque menu basculent vers les
autres jeux.

Rien n'est téléchargé auprès de qui que ce soit d'autre que vous : le convertisseur lit les exécutables de la copie que vous avez achetée, les vérifie
octet par octet, et construit l'app sur votre machine.

<img src="docs/assets/rule-02.svg" alt="" width="100%">

## Résultats

| | mesuré | conditions |
|---|---|---|
| Campagne | **51–60 i/s** | chaque mission se charge et enchaîne jusqu'au générique de fin ; toutes n'ont pas été jouées de bout en bout |
| Zombies | **57–60 i/s** (Die Rise 43–58) | TranZit, Mob of the Dead, Nuketown, Origins, Buried, Die Rise, en solo, via les vrais menus |
| Multijoueur contre des bots | **53–60 i/s** | onze modes de jeu via les vrais menus : une manche complète, le tableau des scores, retour au lobby |
| Monde du jeu par rapport au temps réel | **≈ 1,0×** | l'horloge serveur propre au jeu comparée à l'horloge murale |
| Conversion sur le Mac du joueur | **environ 1 h 45** sur un M1 (la campagne seule : environ 45 minutes) | la campagne est jouable dès qu'elle est convertie ; Zombies et le multijoueur suivent |
| Disque | pic d'environ **10 Go**, environ **4 Go** conservés | après la conversion, seuls les binaires liés restent |

Le nombre d'images par seconde compte les drawables réellement présentés à l'écran, vérifiés par rapport à l'écran et au HUD Metal.

**Ce que vous pouvez jouer**

<table>
<tr>
<td width="33%" valign="top"><img src="docs/assets/card-public-campaign.svg" alt="Campagne" width="100%"><br>
Le vrai menu principal : NEW STORY, RESUME STORY, REPLAY MISSION. De <code>angola</code> au générique de fin, les
missions se chargent et enchaînent sur la suivante. Les missions Strike Force démarrent d'un clic.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-zombies.svg" alt="Zombies" width="100%"><br>
Les six cartes, en solo : menu principal, lobby, partie, Game Over, tableau des scores, puis une autre partie dans le même processus.
Les cartes DLC se jouent si votre compte Steam les possède. L'écran partagé à deux joueurs est expérimental.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-multi.svg" alt="Multijoueur" width="100%"><br>
Parties personnalisées contre des bots depuis les vrais menus : CHOOSE CLASS, le HUD, une manche complète, le tableau des scores
et retour au lobby. Les règles de match sont le script propre à ce projet.</td>
</tr>
</table>

Les trois modes : clavier et souris, ou une DualSense en XInput via GameController.framework ; son activé ; les touches
de déplacement suivent leur position sur votre clavier (ZQSD sur AZERTY, WASD sur QWERTY), sauf si vous les avez assignées vous-même dans les
options du jeu.

<img src="docs/assets/rule-03.svg" alt="" width="100%">

## Pipeline

En cinq étapes, toutes sur le Mac du joueur :

1. **Vérification.** Le convertisseur n'accepte que le build Steam exact de chaque exécutable, vérifié par SHA-256. Le
   lanceur demande au client Steam de votre Mac si le compte connecté possède le jeu, et refuse dans le cas contraire.
2. **Catalogue et élévation.** Ghidra liste les fonctions de votre propre exécutable ; Remill v6.0.1 élève chacune en
   LLVM IR. Cinq correctifs du lifter sont embarqués dans [tools/remill-patches/](tools/remill-patches/), dont
   le mode d'arrondi x87, et un qui fait un fork d'un Remill déjà lancé pour chaque fonction au lieu de recharger sa sémantique x86.
3. **Repliement.** L'IR de Remill est correct mais lent : chaque accès mémoire invité, chaque flag et chaque push x87 est un appel ou une écriture
   dans une structure `State`. Nos passes le réécrivent avant `clang -O2` : les lectures et écritures invitées sont inlinées, les séquences `lock`
   deviennent des atomiques matérielles, le `ret` invité reste à l'intérieur du code élevé, et la pile x87 est promue en
   variables locales.
4. **Runtime.** [runtime/](runtime/) implémente les imports Windows que le jeu appelle réellement, avec les arités stdcall exactes
   (celles de l'API Steam tirées des en-têtes publics de Valve). La mémoire invitée est un unique mapping de 4 Gio. Direct3D 11 est
   une couche de faux COM au-dessus de Metal ; les shaders DXBC passent par [HLSLcc](third_party/HLSLcc). XAudio2 va vers CoreAudio, et
   les vidéos sont décodées nativement avec libvpx.
5. **Threads.** Les threads invités démarrent comme des coroutines sur un seul thread hôte, ce qui supprime la plupart des sources de
   non-déterminisme et rend une exécution rejouable. La configuration rapide déplace le serveur, le backend de rendu, le mixage et le décodage du son, et les workers de jobs
   vers de vrais threads hôtes. ARM n'est pas TSO, il a donc fallu de vraies barrières mémoire.

**La protection anticopie est exécutée, pas contournée.** Le jeu cache des contrôles d'intégrité derrière des exceptions matérielles,
des pièges de pas-à-pas et du code qu'il déchiffre dans le tas. Une petite machine x86 construite sur Unicorn
([x86_guest_vm.cpp](runtime/win32/x86_guest_vm.cpp), [heap_code.cpp](runtime/win32/heap_code.cpp)) exécute exactement ces
séquences et rend la main au code recompilé. Aucun résultat de contrôle n'est forcé et aucune clé n'est posée à la main.

**Un trou dans la traduction est conçu pour coûter de la vitesse, pas un plantage.** Quand le jeu atteint du code x86 qui n'a jamais été élevé, la
même machine exécute ses octets d'origine ([interp.cpp](runtime/win32/interp.cpp)) et rend la main au code
recompilé au prochain bloc traduit, appel ou retour. Chaque adresse interprétée est journalisée (`[INTERP]`).

**Le service en ligne reçoit une réponse locale.** Zombies et le multijoueur attendent Demonware ; un répondeur écrit à partir des
requêtes du jeu lui-même ([dw_local.cpp](runtime/win32/dw_local.cpp)) répond pour le lobby, le stockage et les tickets sur votre Mac.
Rien ne quitte la machine.

<details>
<summary>Pourquoi pas Wine, CrossOver, DXVK ou DXMT ?</summary>

Ils traduisent les appels Windows à l'exécution et laissent le code x86 sous Rosetta. Ici, le code x86 lui-même a disparu : c'est du
code machine ARM64 avant même le démarrage du jeu. DXVK et DXMT sont des DLL PE qui atteignent leur backend natif par le mécanisme
unixlib de Wine ; ce portage n'a pas de Wine, et son Direct3D 11 est sa propre couche au-dessus de la mémoire invitée 32 bits. Seule la traduction
des shaders était nécessaire de l'extérieur, et HLSLcc, une simple bibliothèque C++, s'en charge.
</details>

<img src="docs/assets/rule-04.svg" alt="" width="100%">

## Constats

Chaque entrée est un symptôme vu à l'écran, la cause qui a été mesurée, et l'effet du correctif.

<details>
<summary><b>Treize constats</b>, du mode d'arrondi x87 à un faux départ à l'intérieur d'une seule instruction</summary>


**F1 · Le lifter ignorait le mode d'arrondi x87.** Dans Zombies, la vue à plat ventre tremblait. `fistp` arrondissait au plus proche là où
le jeu avait demandé la troncature, dans le codec réseau de la hauteur de vue. La hauteur reçue par le client était
fausse sur 8 échantillons sur 8, et sur 0 sur 8 après un correctif de Remill et une nouvelle élévation du codec.

**F2 · Un bit réservé du mot de contrôle x87.** Une mission de la campagne se figeait dans une boucle d'effets, uniquement avec de vrais
threads. Le lifter stockait le bit 6 du mot de contrôle différemment du vrai matériel. Un correctif d'une ligne de Remill et quatre
fonctions du runtime élevées à nouveau l'ont réglé, sans ralentissement.

**F3 · Un flag de fonctionnalité CPU change l'algorithme.** Notre `IsProcessorFeaturePresent` répondait « pas de SSE2 », si bien que le runtime
C utilisait son `ceil` x87, qui arrondissait au plus proche sous notre runtime. Un effet échantillonnait alors l'indice −1 indéfiniment : deux
missions se figeaient.

**F4 · Notre propre journal causait les saccades.** Des à-coups de 170–430 ms étaient le thread de rendu bloqué dans `write()` : notre journal,
redirigé vers un fichier sur un disque presque plein. L'envoyer par un tube vers un processus fils a fait passer les à-coups par exécution
de 12 et 8 à 0 et 0.

**F5 · La traduction des shaders décalait des constantes de 4 octets.** HLSLcc plaçait un `float2` après un `float` à l'offset 24 là où
Direct3D le lit à 20. 31 shaders sur 2 664 étaient touchés, dont les yeux et les réticules.

**F6 · La protection doit s'exécuter.** Le menu des options se figeait 8 fois sur 8. Un contrôle d'intégrité réécrit sa propre adresse
de retour pour pointer vers du code déchiffré dans le tas, ce que le runtime ignorait. Exécuter ce code, comme le fait Windows, a donné
3 sur 3 sans blocage.

**F7 · Nos propres garde-fous étaient l'obstacle.** Un de nos garde-fous arrêtait le jeu après 28 secondes de jeu. Les plus gros
gains du projet sont venus de la suppression de nos propres garde-fous et du faux partage, pas de code astucieux.

**F8 · Un stub d'API vide ressemble à un service en panne.** Deux stubs qui ne renvoyaient rien (`CryptGenRandom`,
`StringFromGUID2`) avaient l'air d'une panne réseau dans Zombies.

**F9 · Les priorités de thread font partie du programme.** `SetThreadPriority` était un stub. Transmettre la demande du mixeur à
macOS a fait passer les tampons audio trop courts d'une exécution de 908–1872 à 7–71, et les trous de son avec.

**F10 · Skinning natif, bit pour bit.** Le skinning CPU réécrit en NEON donne les mêmes bits que le x86 d'origine et a fait passer
une scène lourde de 17,8 à 30,3 i/s.

**F11 · Les décalcomanies ont besoin du depth bias du jeu.** Les traces de brûlure et le sang apparaissaient ou disparaissaient au moindre mouvement de
la caméra. Le jeu demande un depth bias sur les décalcomanies ; notre couche le laissait à zéro. Transmettre les valeurs du jeu à Metal a réglé le problème.

**F12 · Le thread de rendu copiait la géométrie du niveau à chaque image.** En multijoueur et dans Zombies, il copiait 23–63 Mo
de données de sommets vers le GPU par image, en grande partie statiques. Garder les tampons statiques résidents dans Metal a ramené les copies à
3,5–5 Mo par image : multijoueur 51,8 → 60 i/s, Zombies 43,1 → 58,2. Le même changement ne fait rien en campagne,
où le rythme est dicté par le thread d'ordonnancement. Une optimisation n'aide que le mode dont elle touche le goulot d'étranglement.

**F13 · Un faux départ à l'intérieur d'une seule instruction coûtait la moitié des images par seconde en combat.** Dans les premières conversions à partir d'une
copie Steam neuve, Zombies tombait à 25 i/s dès que le joueur tirait. Des fonctions chaudes tournaient dans l'interpréteur : le
catalogue plaçait le début de la fonction suivante à l'intérieur de la première instruction d'une racine, si bien que son balayage borné
décodait zéro octet et que la racine était abandonnée sans un mot. Balayer ces racines sans la borne, comme le faisait déjà le lifter
un par un, les a remises dans le code traduit.

</details>

<img src="docs/assets/rule-05.svg" alt="" width="100%">

## Méthode

**Prédire, puis mesurer.**

- Une prédiction est écrite avant que le résultat soit lu : environ 1 100 numérotées à la date de cette bêta.
- Au moins trois exécutions par configuration. Le bruit d'une exécution à l'autre est de 30–64 %, donc une différence inférieure à environ 40 % n'est pas visible avec n = 3.
- Un changement doit prouver qu'il est actif : le journal doit montrer qu'il se déclenche.
- Une exécution qui n'atteint jamais le niveau est une exécution morte, pas un point de données.
- L'écran l'emporte sur le journal. Quand l'écran contredit une sonde, c'est la sonde qui a tort.

<details>
<summary><b>Enregistrement et rejeu, rétractations, clean room, qui fait quoi</b></summary>

**Enregistrement et rejeu déterministes.** Chaque thread invité étant une coroutine sur un seul thread hôte, la plupart des sources de
non-déterminisme ont disparu ; le rejeu est vérifié par rapport à l'enregistrement, pas supposé. Trois frontières capturent toutes les entrées extérieures : le répartiteur d'imports, le drainage des messages de fenêtre, et les
tampons audio consommés. Le journal fait environ 16 Mo. Il permet de refaire la même exécution deux fois avec exactement un seul flag modifié.

**L'instrument est ce qui échoue.** Près de 200 constats ont été retirés en cours de route. La plus grande part
(31 %) venait d'un instrument qui répondait à une autre question ; la suivante (17 %), d'un instrument qui ne pouvait pas répondre du tout. Donc
chaque oracle ici nomme son entrée et refuse de répondre quand cette entrée manque.

**Clean room pour les règles de match.** Le multijoueur et Zombies tirent leurs règles de match d'un fichier de patch quotidien que Steam
ne distribue plus. Le remplaçant dans [runtime/match_rules/](runtime/match_rules/) a été écrit par une équipe qui n'a jamais vu le
script d'origine : une équipe a rédigé une spécification comportementale à partir du jeu, l'autre a écrit le code à partir de cette
seule spécification, et les deux ne se sont parlé que par questions et réponses écrites, conservées avec leurs dates.

**Qui fait quoi.** Les agents IA apportent le débit : écrire les shims, bissecter des millions d'appels par exécution, lire des dizaines
de milliers de lignes. L'humain apporte l'objectif, les corrections à partir de ce qui est réellement à l'écran, et refuse
la conclusion flatteuse. C'est un seul jeu, des mois de travail et beaucoup d'interventions manuelles.

</details>

<img src="docs/assets/rule-07.svg" alt="" width="100%">

## Limites

- **Local uniquement.** Le multijoueur et Zombies tournent face au répondeur sur votre Mac. Il n'y a ni jeu en ligne ni LAN. Le League Play est refusé.
- **Les règles de match sont les nôtres.** Écrites en clean room (voir [Méthode](#méthode)) ; le comportement peut différer de l'original par
  endroits.
- **Chaleur.** Le Mac chauffe énormément pendant les longues sessions (111–116 °C relevés sur le M1). Jouez branché : sur batterie,
  macOS bride fortement le jeu. Brancher évite le ralentissement, pas la chaleur.
- **Première partie sur chaque carte.** Dans Zombies et le multijoueur, les shaders sont traduits pour Metal pendant le chargement de la carte, si bien que
  ce premier chargement est plus long et que la première minute peut encore saccader légèrement. En campagne, ils sont traduits à
  leur première apparition, donc la cinématique de chargement reste fluide et le premier passage dans un lieu peut saccader. Tout
  est mis en cache : la fois suivante, le chargement est rapide et le jeu fluide.
- **Fenêtre.** Le jeu tourne dans une fenêtre de 1280×720.
- **Autres Mac.** Un seul M1 a été mesuré. Les M2, M3 et M4 devraient être plus rapides, mais aucun chiffre n'est avancé.
- **Campagne.** Chaque mission se charge et enchaîne ; toutes n'ont pas été jouées de bout en bout.
- **Zombies.** Pas de coopération en ligne ni en LAN, donc Grief et Turned ne peuvent pas être joués. L'écran partagé local à deux joueurs est
  expérimental : le joueur 2 apparaît au début de la manche 1.
- **Audio macOS.** Quand le `coreaudiod` du système est en mauvais état, la campagne se bloque sur sa première vidéo. Vérifiez que
  `afplay -t 1 /System/Library/Sounds/Tink.aiff` dure environ 1 s ; redémarrer le Mac règle le problème.

<img src="docs/assets/rule-08.svg" alt="" width="100%">

## Installation

Il vous faut un Mac Apple silicon, au moins **20 Go libres** (la conversion culmine à environ 10 Go et environ 4 Go restent), et
Steam pour Mac connecté au compte qui possède le jeu.

**macOS 26 ou ultérieur : sans Terminal.** Téléchargez `Black Ops II.app` depuis les [releases](https://github.com/adrbn/bo2-mac/releases)
et ouvrez-la. Elle embarque ses propres outils de traduction (LLVM 17, Remill, Unicorn, libvpx, HLSLcc, capstone, construits par
[scripts/make_toolchain.sh](scripts/make_toolchain.sh)) et aucun fichier du jeu. La première fois, macOS propose d'installer
les Command Line Tools gratuits d'Apple : cliquez sur « Installer », puis sur « Réessayer » dans la fenêtre de l'app.
Une fois par jour au lancement, l'app cherche une version plus récente et propose de l'installer. « Installer, garder cette version » conserve
l'app actuelle à côté, renommée avec sa version, de sorte qu'ouvrir celle-ci permet de revenir en arrière. Une mise à jour conserve ce qui est déjà converti et ne refait que ce que ses changements touchent : un correctif du runtime relie à nouveau les trois modes en 3 minutes environ sur un M1.

**macOS 14 ou 15 : depuis le Terminal**, avec Homebrew :

```bash
git clone https://github.com/adrbn/bo2-mac.git && cd bo2-mac
brew install llvm@17 cmake ninja libvpx unicorn
pip3 install capstone
scripts/make_app.sh --from-source
```

Ouvrez `Black Ops II.app`, indiquez-lui le dossier de votre jeu et laissez-la convertir : environ 1 h 45 sur un M1, la campagne d'abord en environ 45 minutes. La campagne est
jouable dès qu'elle est convertie : jouez-y alors, et Zombies et le multijoueur sont convertis la première fois que vous les choisissez
dans le menu du jeu. Sinon, la conversion se poursuit sur les trois. Si le disque tombe sous
8 Go libres, la conversion s'arrête proprement et reprend là où elle en était.

**Écran partagé dans Zombies (expérimental).** Branchez deux manettes, ouvrez le menu **Black Ops II** dans la barre des menus et
cochez **Two-Player Split Screen in Zombies**. Le réglage s'applique au prochain lancement de Zombies, depuis l'app ou depuis l'entrée
ZOMBIES du menu du jeu. Le joueur 1 joue la moitié haute, le joueur 2 la moitié basse ; le joueur 2 apparaît au début de la manche 1.
Sans seconde manette, le jeu reste à un joueur. Décochez-le pour revenir à un seul joueur.

**Mise à jour, ou si la conversion s'arrête.** Exécutez `git pull && scripts/make_app.sh --from-source` dans le dossier
du dépôt : ce qui est déjà converti est conservé et la conversion reprend. Une mise à jour ne refait que ce que ses changements touchent : un
correctif du runtime relie les trois modes en environ 3 minutes sur un M1 au lieu de tout convertir à nouveau. Si cela échoue encore,
ouvrez une issue avec le journal qu'elle indique.

Le convertisseur n'accepte que le build Steam exact, vérifié par SHA-256. Les exécutables Steam sont chiffrés par l'enveloppe
SteamStub de Valve et **ce dépôt ne fournit aucun moyen de la retirer** : le convertisseur attend les trois images déchiffrées
dans `~/.bo2-native/steam/` (`t6sp.exe.unpacked.exe`, `t6mp.exe.unpacked.exe`, `t6zm.exe.unpacked.exe`). Vous les produisez
vous-même à partir de la copie que vous avez achetée, avec un outil séparé de votre choix. Seule l'enveloppe doit disparaître : les noms de fichiers
ci-dessus et la vérification SHA-256 vous disent immédiatement si un fichier est bon. Ne partagez jamais ces fichiers.

**DLC.** Le lanceur demande à votre client Steam quels DLC votre compte possède, exactement comme le fait le jeu sous Windows. Une carte DLC
se joue quand vous la possédez et que ses fichiers sont dans le dossier de votre jeu.

<img src="docs/assets/rule-09.svg" alt="" width="100%">

## Dépôt

| chemin | ce qu'il contient |
|---|---|
| [runtime/win32/](runtime/win32/) | le runtime Win32 : kernel32, l'ordonnanceur, Direct3D 11, le son, les entrées, le répondeur du service local |
| [runtime/gfx/](runtime/gfx/) | le backend Metal : présentation, cache de shaders, cibles de rendu |
| [runtime/match_rules/](runtime/match_rules/) | les règles de match en clean room pour le multijoueur et Zombies |
| [tools/](tools/) | le pilote d'élévation, le catalogue, les passes d'IR, les outils de désassemblage et d'analyse |
| [tools/remill-patches/](tools/remill-patches/) | les correctifs embarqués par-dessus Remill v6.0.1 |
| [scripts/](scripts/) | `make_app.sh`, l'installeur et convertisseur Steam, les lanceurs |
| [third_party/HLSLcc](third_party/HLSLcc) | traduction de shaders DXBC vers Metal |
| [docs/](docs/) | l'état de la préversion, la toolchain |

<img src="docs/assets/rule-10.svg" alt="" width="100%">

## Feuille de route

1. **Cette bêta** : les trois modes à partir d'une copie Steam neuve, sur d'autres Mac que celui qui les a construits. Vos retours
   (modèle de Mac, nombre d'images par seconde, ce qui a cassé) décident de la suite.
   Premier retour extérieur, 2026-11-05 : sur un M2 Pro (16 Go), la conversion s'est terminée et le jeu a démarré, après trois
   correctifs trouvés en chemin ([#4](https://github.com/adrbn/bo2-mac/issues/4), [#5](https://github.com/adrbn/bo2-mac/issues/5), [#6](https://github.com/adrbn/bo2-mac/issues/6)).
2. **Sans Terminal** : fait. Une app à télécharger et ouvrir ; elle embarque ses outils déjà construits, donc rien n'est
   compilé sur votre Mac sauf le jeu lui-même, et elle se met à jour toute seule (macOS 26 ou ultérieur ; macOS 14 et 15 gardent
   l'installation par Terminal).
3. **Entièrement traduit** : les 46 blocs de code qui passent encore par l'interpréteur Unicorn sont traduits en
   amont, et chaque instruction traduite est vérifiée par rapport à l'interpréteur ;
   toute fonction traduite peut être renvoyée à l'interpréteur, pour trouver une fonction fautive par dichotomie.
4. **Plus d'attente de shaders** : des shaders construits avant le premier match, au lieu de l'être au lancement (environ 3 minutes en multijoueur) et
   pendant le jeu (blocages quand un nouvel effet apparaît). Commencé : les shaders se compilent désormais par lots, et ceux des
   dernières sessions se compilent en arrière-plan au lancement. En multijoueur sur un M1 sans Xcode (quelques essais, pas un
   benchmark), la compilation des shaders pendant les deux premières minutes d'un match est passée de 18–25 s à 10–14 s au premier
   lancement et à environ 5 s au suivant ; les blocages de plus de 300 ms sont passés de 15–22 à 13–15, puis 7. Le premier lancement
   se fige encore à peu près aussi souvent.
5. **Conversion plus courte** : de environ 2 h 30 à 1 h 45 sur un M1 (2026-11-06, mesuré sur les trois modes :
   la préparation lit l'exécutable une seule fois, l'élévation et le repliement utilisent tous les cœurs que la mémoire permet). Une mise à jour ne refait désormais
   que ce qui a changé (un correctif du runtime : environ 3 minutes). Ensuite : moins d'optimisation pour le code qui s'exécute rarement. Les trois modes ne partagent que 4 % de
   leurs fonctions, donc convertir le code partagé une seule fois gagnerait peu.
6. **Plus rapide, sans chaleur** : la campagne est limitée par le thread d'ordonnancement, pas par le rendu ; le multijoueur avec 11 bots
   est limité par le thread serveur. Démarré : la réverbération s'exécute en natif dans les trois modes, environ 40 % de temps en moins sur le thread audio. Des réécritures natives des autres fonctions les plus chaudes viennent ensuite, puis des fréquences d'images au-delà de 60 pour
   les écrans 120 Hz.
7. **Manettes** : vibration et aim assist.
8. **Ensemble** : l'écran partagé local dans Zombies, que la version PC n'a jamais eu, est expérimental (un seul jeu,
   deux joueurs, moitiés haute et basse, environ 28 i/s sur un M1), avec un interrupteur dans le menu de l'app. Ensuite : de vrais sockets UDP pour la coopération Zombies,
   Grief et Turned, et le multijoueur en LAN.
9. **Plus tard** : MetalFX, le post-traitement à la présentation de l'image, et des textures de remplacement chargées depuis la copie du joueur.

<img src="docs/assets/rule-11.svg" alt="" width="100%">

## Mentions légales

Pour une copie que l'utilisateur possède, sur un matériel que l'utilisateur possède.

- Aucun code source du moteur et aucun code ayant fuité ne sont utilisés.
- Ni le binaire d'entrée ni la sortie recompilée ne sont distribués. Aucun fichier du jeu n'est inclus : le joueur a besoin de sa
  propre copie Steam de Call of Duty: Black Ops II. Le convertisseur n'accepte que le build Steam exact (SHA-256), et
  la possession est vérifiée à chaque démarrage du jeu par le propre client Steam du joueur : le jeu refuse de s'exécuter quand Steam
  n'est pas connecté ou que le compte ne le possède pas.
- La protection anticopie est exécutée comme le jeu l'a écrite ; rien n'y est neutralisé. Ce dépôt ne fournit aucun moyen
  de retirer l'enveloppe SteamStub de Valve.
- Le service en ligne attendu par le jeu reçoit une réponse d'un répondeur local écrit à partir des requêtes du jeu lui-même : aucun
  serveur d'Activision n'est contacté, il n'y a pas de jeu en ligne, et la possession du jeu est toujours vérifiée par le Steam du joueur.
  Les règles de match ont été écrites en clean room.
- Le code écrit ici est sous licence GPL-3.0 ([LICENSE](LICENSE)).
- Unicorn (GPL-2.0) n'est jamais combiné avec du code GPL-3.0 dans un binaire distribué. L'app embarque la bibliothèque non modifiée d'Unicorn
  comme fichier séparé à côté des autres outils ; elle est liée au binaire du jeu sur le Mac du joueur
  ([tools/boot.py](tools/boot.py)), et ce binaire n'est jamais distribué. Chaque release publie les sources de la
  version exacte d'Unicorn qu'elle embarque (`unicorn-<version>-source.tar.gz`), comme l'exige la GPL-2.0. Les licences de chaque
  outil embarqué se trouvent dans l'app, dans `Contents/Resources/toolchain/licenses`.

Ceci est un projet de fan non officiel. Call of Duty et Black Ops sont des marques d'Activision Publishing, Inc. Steam est une
marque de Valve Corporation. Ce projet n'est ni affilié à Activision, Treyarch, Microsoft ou Valve, ni approuvé ni parrainé par eux. Pour jouer, achetez le jeu.

## Star History

<a href="https://www.star-history.com/#adrbn/bo2-mac&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
  </picture>
</a>

<div align="center"><img src="docs/assets/footer.svg" alt="" width="100%"></div>
