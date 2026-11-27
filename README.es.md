<div align="center">

<picture>
  <source media="(prefers-color-scheme: light)" srcset="docs/assets/hero-public-light.svg">
  <img src="docs/assets/hero-public.svg" alt="bo2-mac: Call of Duty: Black Ops II recompilado estáticamente de Windows x86-32 a macOS ARM64 nativo, a partir de tu propia copia de Steam. En un Apple M1: campaña de 51 a 60 fotogramas por segundo, Zombies de 57 a 60, multijugador contra bots de 53 a 60." width="100%">
</picture>

# bo2-mac

**Call of Duty: Black Ops II, recompilado estáticamente de Windows x86 de 32 bits a macOS ARM64 nativo.**<br>
Campaña, Zombies y multijugador contra bots, compilado en tu Mac a partir de tu propia copia de Steam.

<a href="README.md">English</a> · <a href="README.fr.md">Français</a> · <b>Español</b> · <a href="README.pt-BR.md">Português</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.ru.md">Русский</a> · <a href="README.ja.md">日本語</a> · <a href="README.ko.md">한국어</a> · <a href="README.zh.md">中文</a>

<a href="#instalación"><img src="docs/assets/btn-play.svg" alt="Jugar" height="40"></a>&nbsp;
<a href="#resultados"><img src="docs/assets/btn-results.svg" alt="Resultados" height="40"></a>&nbsp;
<a href="#pipeline"><img src="docs/assets/btn-pipeline.svg" alt="Pipeline" height="40"></a>&nbsp;
<a href="#hallazgos"><img src="docs/assets/btn-findings.svg" alt="Hallazgos" height="40"></a>&nbsp;
<a href="#hoja-de-ruta"><img src="docs/assets/btn-roadmap.svg" alt="Hoja de ruta" height="40"></a>

<a href="https://news.ycombinator.com/item?id=50449891"><img src="docs/assets/btn-hackernews.svg" alt="Destacado en Hacker News" height="44"></a>

![source: x86-32 Windows](https://img.shields.io/badge/source-x86--32%20Windows-ff6a00?style=flat-square&labelColor=0b0b0c)
![target: ARM64 macOS](https://img.shields.io/badge/target-ARM64%20macOS-ff6a00?style=flat-square&labelColor=0b0b0c)
![lifter: Remill, LLVM 17](https://img.shields.io/badge/lifter-Remill%20%E2%86%92%20LLVM%2017-ff6a00?style=flat-square&labelColor=0b0b0c)
![renderer: D3D11 to Metal](https://img.shields.io/badge/renderer-D3D11%20%E2%86%92%20Metal-ff6a00?style=flat-square&labelColor=0b0b0c)
![Wine: none](https://img.shields.io/badge/Wine-none-ff6a00?style=flat-square&labelColor=0b0b0c)
![game files in this repository: none](https://img.shields.io/badge/game%20files-none-f4f1ea?style=flat-square&labelColor=0b0b0c)
[![latest release](https://img.shields.io/github/v/release/adrbn/bo2-mac?style=flat-square&labelColor=0b0b0c&color=ff6a00&label=release)](https://github.com/adrbn/bo2-mac/releases/latest)
![status: beta](https://img.shields.io/badge/status-beta-f4f1ea?style=flat-square&labelColor=0b0b0c)

<sub>[Por qué](#por-qué) · [Resumen](#resumen) · [Método](#método) · [Límites](#límites) · [Repositorio](#repositorio) · [Aviso legal](#aviso-legal)</sub><br>
<sub>Beta, 2026-11-03 · medido en un único Apple M1 (4 núcleos de rendimiento + 4 de eficiencia, 16 GB)</sub>

</div>

<p align="center"><a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4"><img src="docs/assets/preview.gif" alt="Campaña, Zombies y multijugador ejecutándose de forma nativa en macOS" width="560"></a><br>
<sub>Campaña (Angola), Zombies (Town) y multijugador (Hijacked, contra bots) en un Apple M1, grabados mientras se juega; el número de la esquina superior derecha son los fotogramas por segundo, y la propia grabación de pantalla les resta algunos, sobre todo en la campaña. <a href="https://github.com/adrbn/bo2-mac/raw/main/docs/assets/gameplay.mp4">Vídeo de 80 segundos (MP4, 20 MB)</a></sub></p>

> [!IMPORTANT]
> **Este repositorio no contiene ningún archivo del juego ni código del juego traducido.** Convierte tu propia copia de
> Steam de Black Ops II en una app para tu Mac, en tu Mac. El código del juego acaba dentro de esa app, así que nadie
> puede entregarte una ya hecha: cada jugador compila la suya, y no puede compartirse. Proyecto no oficial de aficionados,
> sin ninguna relación con Activision, Treyarch ni Microsoft. Para jugar, compra el juego. Beta: lee [lo que se ha
> demostrado y lo que no](docs/PRERELEASE.md).

---

<img src="docs/assets/rule-01.svg" alt="" width="100%">

## Por qué

Black Ops II nunca salió para Mac, y en Apple Silicon no era realmente jugable hasta ahora.
[AppleGamingWiki](https://www.applegamingwiki.com/wiki/Call_of_Duty:_Black_Ops_II) lo clasifica como injugable en
CrossOver y no compatible con Rosetta 2. Lo que queda es una máquina virtual de Windows ARM en Parallels, con informes
que van desde 1 fotograma por segundo en partidas en un M1 Max hasta 60–80 en M3 y M4
([MacGamingDB](https://macgamingdb.app/games/call-of-duty-black-ops-ii)). Aquí el juego se traduce una sola vez, antes de
iniciarse, a una app ARM64 nativa: 51–60 fotogramas por segundo en un M1 básico, sin Windows y sin máquina virtual. Ser
nativo también significa que se puede corregir y ampliar como cualquier app de Mac: la pantalla dividida, que la versión
de PC nunca tuvo, está disponible como opción experimental en Zombies.

## Resumen

bo2-mac toma los tres ejecutables de Black Ops II, la campaña (`t6sp.exe`), Zombies (`t6zm.exe`) y el
multijugador (`t6mp.exe`), y convierte su código máquina x86 en código máquina ARM64 por adelantado.
[Remill](https://github.com/lifting-bits/remill) eleva cada función a LLVM IR, una cadena de pasadas propias reescribe
ese IR, y el resultado se enlaza con un runtime de Win32 escrito a mano, cuyo Direct3D 11 se dibuja con Metal. No hay
Wine ni JIT: el juego se ejecuta como código nativo, en un solo proceso. Se traducen unas 150 000 funciones entre los
tres ejecutables. Solo las secuencias cortas que oculta la protección contra copia se ejecutan en un pequeño intérprete x86, y lo mismo
ocurre con cualquier código que la traducción haya pasado por alto, de modo que un hueco está pensado para costar velocidad en lugar de colgar el juego.

En un Apple M1, la campaña corre a **51–60 fotogramas por segundo**, y cada misión carga y da paso a la siguiente,
hasta los créditos finales. Zombies se juega en solitario en **seis mapas** a través de los menús del propio juego a **57–60**.
El multijugador se juega contra bots a través de sus menús reales, once modos de juego, a **53–60**. Una sola app,
`Black Ops II.app`, abre la campaña, y las entradas CAMPAIGN / ZOMBIES / MULTIPLAYER de cada menú cambian al
otro juego.

No se descarga nada de nadie más que de ti: el conversor lee los ejecutables de la copia que compraste, los comprueba
byte a byte y compila la app en tu equipo.

<img src="docs/assets/rule-02.svg" alt="" width="100%">

## Resultados

| | medido | condiciones |
|---|---|---|
| Campaña | **51–60 fps** | cada misión carga y encadena hasta los créditos finales; no todas jugadas de principio a fin |
| Zombies | **57–60 fps** (Die Rise 43–58) | TranZit, Mob of the Dead, Nuketown, Origins, Buried, Die Rise, en solitario, a través de los menús reales |
| Multijugador contra bots | **53–60 fps** | once modos de juego a través de los menús reales: una ronda completa, el marcador, de vuelta al lobby |
| Mundo del juego frente al tiempo real | **≈ 1.0×** | el reloj del servidor del propio juego frente al tiempo de pared |
| Conversión en el Mac del jugador | **cerca de 1 hora 45** en un M1 (solo la campaña: unos 45 minutos) | la campaña se puede jugar en cuanto se convierte; Zombies y el multijugador le siguen |
| Disco | pico de unos **10 GB**, se conservan unos **4 GB** | tras la conversión, solo permanecen los binarios enlazados |

La tasa de fotogramas cuenta los drawables realmente presentados en pantalla, verificados con la pantalla y el Metal HUD.

**A qué puedes jugar**

<table>
<tr>
<td width="33%" valign="top"><img src="docs/assets/card-public-campaign.svg" alt="Campaña" width="100%"><br>
El menú principal real: NEW STORY, RESUME STORY, REPLAY MISSION. Desde <code>angola</code> hasta los créditos finales, las
misiones cargan y dan paso a la siguiente. Las misiones de Strike Force se inician con un clic.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-zombies.svg" alt="Zombies" width="100%"><br>
Los seis mapas, en solitario: menú principal, lobby, partida, Game Over, marcador, y luego otra partida en el mismo proceso.
Los mapas de DLC se juegan si tu cuenta de Steam los posee. La pantalla dividida para dos jugadores es experimental.</td>
<td width="33%" valign="top"><img src="docs/assets/card-public-multi.svg" alt="Multijugador" width="100%"><br>
Custom Games contra bots desde los menús reales: CHOOSE CLASS, el HUD, una ronda completa, el marcador
y de vuelta al lobby. Las reglas de partida son un script propio de este proyecto.</td>
</tr>
</table>

En los tres modos: teclado y ratón, o un DualSense como XInput a través de GameController.framework; con sonido; las
teclas de movimiento siguen su posición en tu teclado (ZQSD en AZERTY, WASD en QWERTY), a menos que las hayas asignado tú mismo en las
opciones del juego.

<img src="docs/assets/rule-03.svg" alt="" width="100%">

## Pipeline

En cinco pasos, todos en el Mac del jugador:

1. **Comprobación.** El conversor acepta únicamente la compilación exacta de Steam de cada ejecutable, verificada mediante SHA-256. El
   lanzador pregunta al cliente de Steam de tu Mac si la cuenta con sesión iniciada posee el juego, y se niega si no es así.
2. **Catálogo y elevación.** Ghidra lista las funciones de tu propio ejecutable; Remill v6.0.1 eleva cada una a
   LLVM IR. En [tools/remill-patches/](tools/remill-patches/) se incluyen cinco parches al elevador, entre ellos el
   modo de redondeo x87, y uno que bifurca un Remill en ejecución para cada función en lugar de recargar su semántica x86.
3. **Plegado.** El IR de Remill es correcto pero lento: cada acceso a memoria del invitado, cada flag y cada push x87 es una llamada o un almacenamiento
   en una estructura `State`. Nuestras pasadas lo reescriben antes de `clang -O2`: las lecturas y escrituras del invitado se insertan en línea, las secuencias
   `lock` se convierten en atómicas de hardware, el `ret` del invitado permanece dentro del código elevado, y la pila x87 se promueve a
   variables locales.
4. **Runtime.** [runtime/](runtime/) implementa las importaciones de Windows que el juego realmente llama, con aridades stdcall
   exactas (las de la API de Steam tomadas de los encabezados públicos de Valve). La memoria del invitado es un único mapeo de 4 GiB. Direct3D 11 es
   una capa de COM falso sobre Metal; los shaders DXBC pasan por [HLSLcc](third_party/HLSLcc). XAudio2 va a CoreAudio, y
   los vídeos se decodifican de forma nativa con libvpx.
5. **Hilos.** Los hilos del invitado arrancan como corrutinas en un único hilo del anfitrión, lo que elimina la mayoría de las fuentes de
   no determinismo y hace que una ejecución sea reproducible. La configuración rápida traslada el servidor, el backend de renderizado, la mezcla y decodificación de sonido, y los trabajadores de trabajos
   a hilos reales del anfitrión. ARM no es TSO, así que esto exigió barreras de memoria reales.

**La protección contra copia se ejecuta, no se elude.** El juego esconde comprobaciones de integridad tras excepciones de hardware,
trampas de paso único y código que descifra en el heap. Una pequeña máquina x86 construida sobre Unicorn
([x86_guest_vm.cpp](runtime/win32/x86_guest_vm.cpp), [heap_code.cpp](runtime/win32/heap_code.cpp)) ejecuta exactamente esas
secuencias y devuelve el control al código recompilado. No se fuerza el resultado de ninguna comprobación ni se coloca ninguna clave a mano.

**Un hueco en la traducción está pensado para costar velocidad, no un cuelgue.** Cuando el juego llega a código x86 que nunca se elevó, la
misma máquina ejecuta sus bytes originales ([interp.cpp](runtime/win32/interp.cpp)) y devuelve el control al código
recompilado en el siguiente bloque traducido, llamada o retorno. Cada dirección interpretada se registra (`[INTERP]`).

**El servicio en línea se responde en local.** Zombies y el multijugador esperan Demonware; un respondedor escrito a partir de las
peticiones del propio juego ([dw_local.cpp](runtime/win32/dw_local.cpp)) responde el lobby, el almacenamiento y el ticket en tu Mac.
Nada sale del equipo.

<details>
<summary>¿Por qué no Wine, CrossOver, DXVK o DXMT?</summary>

Traducen las llamadas de Windows en tiempo de ejecución y mantienen el código x86 bajo Rosetta. Aquí el propio código x86 desaparece: es
código máquina ARM64 antes de que el juego se inicie. DXVK y DXMT son DLL PE que llegan a su backend nativo mediante el mecanismo
unixlib de Wine; este port no tiene Wine, y su Direct3D 11 es una capa propia sobre la memoria de invitado de 32 bits. Solo la traducción
de shaders hizo falta de fuera, y HLSLcc, una biblioteca de C++ sencilla, la cubre.
</details>

<img src="docs/assets/rule-04.svg" alt="" width="100%">

## Hallazgos

Cada entrada es un síntoma visto en pantalla, la causa que se midió y el efecto de la corrección.

<details>
<summary><b>Trece hallazgos</b>, desde el modo de redondeo x87 hasta un falso inicio dentro de una sola instrucción</summary>


**F1 · El elevador ignoraba el modo de redondeo x87.** En Zombies, la vista en posición tumbada temblaba. `fistp` redondeaba al más cercano donde
el juego había pedido truncamiento, dentro del códec de red de la altura de la vista. La altura recibida por el cliente era
incorrecta en 8 muestras de 8, y en 0 de 8 tras un parche de Remill y una nueva elevación del códec.

**F2 · Un bit reservado de la palabra de control x87.** Una misión de la campaña se congelaba en un bucle de efectos, solo con hilos
reales. El elevador almacenaba el bit 6 de la palabra de control de forma distinta al hardware real. Un parche de una línea en Remill y cuatro
funciones del runtime vueltas a elevar lo solucionaron, sin ninguna ralentización.

**F3 · Un flag de característica de la CPU cambia el algoritmo.** Nuestro `IsProcessorFeaturePresent` respondía «sin SSE2», así que el runtime
de C tomaba su `ceil` x87, que redondeaba al más cercano bajo nuestro runtime. Un efecto muestreaba entonces el índice −1 para siempre: dos
misiones se congelaban.

**F4 · Nuestro propio log causaba el tirón.** Los parones de 170–430 ms eran el hilo de renderizado bloqueado en `write()`: nuestro log,
redirigido a un archivo en un disco casi lleno. Enviarlo por una tubería a un proceso hijo llevó los parones por ejecución
de 12 y 8 a 0 y 0.

**F5 · La traducción de shaders desplazaba constantes 4 bytes.** HLSLcc colocaba un `float2` tras un `float` en el offset 24 donde
Direct3D lo lee en el 20. Se vieron afectados 31 de 2664 shaders, entre ellos los ojos y las miras.

**F6 · La protección tiene que ejecutarse.** El menú de opciones se congelaba 8 veces de 8. Una comprobación de integridad reescribe su propia dirección
de retorno para apuntar a código descifrado en el heap, que el runtime ignoraba. Ejecutar ese código, como hace Windows, dio
3 de 3 sin congelaciones.

**F7 · Nuestras propias salvaguardas eran el obstáculo.** Una de ellas detenía el juego a los 28 segundos de partida. Las mayores
ganancias del proyecto vinieron de eliminar nuestras propias salvaguardas y el false sharing, no de código ingenioso.

**F8 · Un stub de API vacío parece un servicio roto.** Dos stubs que no devolvían nada (`CryptGenRandom`,
`StringFromGUID2`) parecían un fallo de red en Zombies.

**F9 · Las prioridades de los hilos forman parte del programa.** `SetThreadPriority` era un stub. Pasar la petición del mezclador
a macOS redujo los búferes de audio cortos de una ejecución de 908–1872 a 7–71, y los cortes de sonido con ellos.

**F10 · Skinning nativo, bit por bit.** El skinning por CPU reescrito en NEON da los mismos bits que el x86 original y llevó
una escena pesada de 17,8 a 30,3 fps.

**F11 · Los decals necesitan el depth bias del juego.** Las marcas de quemadura y la sangre aparecían o desaparecían con un ligero giro de la
cámara. El juego pide un depth bias en los decals; nuestra capa lo dejaba en cero. Pasar a Metal los valores del juego lo solucionó.

**F12 · El hilo de renderizado copiaba la geometría del nivel en cada fotograma.** En multijugador y Zombies copiaba entre 23 y 63 MB
de datos de vértices a la GPU por fotograma, casi todos estáticos. Mantener los búferes estáticos residentes en Metal redujo las copias a
3,5–5 MB por fotograma: multijugador 51,8 → 60 fps, Zombies 43,1 → 58,2. El mismo cambio no hace nada en la campaña,
donde el ritmo lo marca el hilo del planificador. Una aceleración solo ayuda al modo cuyo cuello de botella toca.

**F13 · Un falso inicio dentro de una sola instrucción costó la mitad de la tasa de fotogramas en combate.** En las primeras conversiones a partir de una
copia nueva de Steam, Zombies caía a 25 fps cada vez que el jugador disparaba. Funciones calientes se ejecutaban en el intérprete: el
catálogo situaba el inicio de la siguiente función dentro de la primera instrucción de una raíz, de modo que su barrido acotado
decodificaba cero bytes y la raíz se descartaba sin avisar. Barrer esas raíces sin la cota, como ya hacía el
elevador uno a uno, las devolvió al código traducido.

</details>

<img src="docs/assets/rule-05.svg" alt="" width="100%">

## Método

**Predecir y luego medir.**

- Se escribe una predicción antes de leer el resultado: unas 1100 numeradas en el momento de esta beta.
- Al menos tres ejecuciones por configuración. El ruido entre ejecuciones es del 30–64 %, así que una diferencia inferior a un 40 % no se puede ver con n = 3.
- Un cambio debe demostrar que está activo: el log tiene que mostrar que se dispara.
- Una ejecución que nunca llega al nivel es una ejecución muerta, no un dato.
- La pantalla gana al log. Cuando la pantalla contradice a una sonda, la sonda está equivocada.

<details>
<summary><b>Grabación y reproducción, retractaciones, sala limpia, quién hace qué</b></summary>

**Grabación y reproducción deterministas.** Con cada hilo del invitado como corrutina en un único hilo del anfitrión, la mayoría de las fuentes de
no determinismo desaparecen; la reproducción se comprueba contra la grabación, no se da por supuesta. Tres fronteras capturan toda la entrada externa: el despachador de importaciones, el vaciado de mensajes de la ventana y los
búferes de audio consumidos. El journal ocupa unos 16 MB. Permite ejecutar dos veces lo mismo cambiando exactamente un flag.

**El instrumento es lo que falla.** Por el camino se retiraron casi 200 hallazgos. La mayor parte
(31 %) fueron un instrumento que respondía a otra pregunta; el siguiente grupo (17 %), uno que no podía responder en absoluto. Así
que cada oráculo de aquí nombra su entrada y se niega a responder cuando falta.

**Sala limpia para las reglas de partida.** El multijugador y Zombies toman sus reglas de partida de un archivo de parche diario que Steam
ya no distribuye. El sustituto en [runtime/match_rules/](runtime/match_rules/) lo escribió un equipo que nunca vio el
script original: un equipo redactó una especificación de comportamiento a partir del juego, el otro escribió el código solo a partir de esa
especificación, y ambos solo se comunicaron mediante preguntas y respuestas escritas, conservadas con sus fechas.

**Quién hace qué.** Los agentes de IA aportan el rendimiento: escribir los shims, bisecar millones de llamadas por ejecución, leer decenas
de miles de líneas. El humano aporta el objetivo, las correcciones a partir de lo que realmente se ve en pantalla, y rechaza la
conclusión complaciente. Es un solo juego, meses de trabajo y mucha intervención manual.

</details>

<img src="docs/assets/rule-07.svg" alt="" width="100%">

## Límites

- **Solo local.** El multijugador y Zombies funcionan contra el respondedor de tu Mac. No hay juego en línea ni LAN. League Play se rechaza.
- **Las reglas de partida son nuestras.** Escritas en sala limpia (consulta [Método](#método)); el comportamiento puede diferir del original en
  algunos puntos.
- **Calor.** El Mac se calienta mucho durante sesiones largas (111–116 °C reportados en el M1). Juega enchufado: con batería,
  macOS limita el juego con dureza. Enchufarlo evita la ralentización, no el calor.
- **Primera partida en cada mapa.** En Zombies y multijugador, los shaders se traducen a Metal mientras carga el mapa, así que
  esa primera carga es más larga y el primer minuto aún puede dar algún tirón. En la campaña se traducen cuando
  aparecen por primera vez, así que la cinemática de carga se mantiene fluida y el primer paso por un lugar puede dar tirones. Todo
  se guarda en caché: la próxima vez carga rápido y va fluido.
- **Ventana.** El juego se ejecuta en una ventana de 1280×720.
- **Otros Mac.** Solo se ha medido un M1. Los M2, M3 y M4 deberían ser más rápidos, pero no se afirma ninguna cifra.
- **Campaña.** Cada misión carga y encadena; no todas se han jugado de principio a fin.
- **Zombies.** No hay cooperativo en línea ni LAN, así que Grief y Turned no se pueden jugar. La pantalla dividida local para dos jugadores es
  experimental: el jugador 2 aparece cuando empieza la ronda 1.
- **Audio de macOS.** Cuando el `coreaudiod` del sistema no está sano, la campaña se atasca en su primer vídeo. Comprueba que
  `afplay -t 1 /System/Library/Sounds/Tink.aiff` tarda aproximadamente 1 s; reiniciar el Mac lo soluciona.

<img src="docs/assets/rule-08.svg" alt="" width="100%">

## Instalación

Necesitas un Mac con Apple silicon, al menos **20 GB libres** (la conversión alcanza un pico de unos 10 GB y se conservan unos 4 GB), y
Steam para Mac con la sesión iniciada en la cuenta que posee el juego.

**macOS 26 o posterior: sin Terminal.** Descarga `Black Ops II.app` desde las [releases](https://github.com/adrbn/bo2-mac/releases)
y ábrela. Incluye sus propias herramientas de traducción (LLVM 17, Remill, Unicorn, libvpx, HLSLcc, capstone, compiladas con
[scripts/make_toolchain.sh](scripts/make_toolchain.sh)) y ningún archivo del juego. La primera vez, macOS ofrece instalar
las Command Line Tools gratuitas de Apple: haz clic en «Instalar» y luego en «Volver a intentar» en la ventana de la app.
Una vez al día, al iniciarse, la app busca una versión más reciente y ofrece instalarla. «Instalar, conservar esta versión» conserva
la app actual a su lado, renombrada con su versión, de modo que abrir esa permite volver atrás. Una actualización conserva lo que ya está convertido y solo rehace lo que tocan sus cambios: una corrección del runtime vuelve a enlazar los tres modos en unos 3 minutos en un M1.

**macOS 14 o 15: desde la Terminal**, con Homebrew:

```bash
git clone https://github.com/adrbn/bo2-mac.git && cd bo2-mac
brew install llvm@17 cmake ninja libvpx unicorn
pip3 install capstone
scripts/make_app.sh --from-source
```

Abre `Black Ops II.app`, indícale la carpeta de tu juego y deja que convierta: cerca de 1 hora 45 en un M1, con la campaña primero en unos 45 minutos. La campaña se
puede jugar en cuanto se convierte: juégala entonces, y Zombies y el multijugador se convierten cuando los eliges por primera vez
en el menú del juego. De lo contrario, la conversión continúa con los tres. Si el disco baja de
8 GB libres, la conversión se detiene limpiamente y se reanuda donde estaba.

**Pantalla dividida en Zombies (experimental).** Conecta un segundo mando y pulsa Start o A en él en la sala de Zombies:
el jugador 2 se une, como en consola. B en ese mando, o desconectarlo, saca al jugador 2. El jugador 1 juega en la mitad
superior y el jugador 2 en la inferior; la pantalla se divide al empezar la partida y el jugador 2 aparece cuando empieza
la ronda 1. Si nadie se une, el juego sigue siendo de un jugador, a pantalla completa. No hay nada que marcar.

**Actualizar, o si la conversión se detiene.** Ejecuta `git pull && scripts/make_app.sh --from-source` en la carpeta del
repositorio: lo ya convertido se conserva y la conversión se reanuda. Una actualización rehace solo lo que tocan sus cambios: una
corrección del runtime vuelve a enlazar los tres modos en unos 3 minutos en un M1 en lugar de convertir de nuevo. Si aun así falla,
abre un issue con el log que indica.

El conversor solo acepta la compilación exacta de Steam, verificada mediante SHA-256. Los ejecutables de Steam están cifrados por el envoltorio
SteamStub de Valve y **este repositorio no ofrece ninguna forma de quitarlo**: el conversor espera las tres imágenes descifradas
en `~/.bo2-native/steam/` (`t6sp.exe.unpacked.exe`, `t6mp.exe.unpacked.exe`, `t6zm.exe.unpacked.exe`). Las generas
tú mismo a partir de la copia que compraste, con una herramienta aparte de tu elección. Solo debe desaparecer el envoltorio: los nombres de archivo
anteriores y la comprobación SHA-256 te dicen al instante si un archivo es correcto. No compartas nunca estos archivos.

**DLC.** El lanzador pregunta a tu cliente de Steam qué DLC posee tu cuenta, exactamente como hace el juego en Windows. Un mapa de DLC
se juega cuando lo posees y sus archivos están en la carpeta de tu juego.

<img src="docs/assets/rule-09.svg" alt="" width="100%">

## Repositorio

| ruta | qué contiene |
|---|---|
| [runtime/win32/](runtime/win32/) | el runtime de Win32: kernel32, el planificador, Direct3D 11, sonido, entrada, el respondedor del servicio local |
| [runtime/gfx/](runtime/gfx/) | el backend de Metal: presentación, caché de shaders, render targets |
| [runtime/match_rules/](runtime/match_rules/) | las reglas de partida de sala limpia para el multijugador y Zombies |
| [tools/](tools/) | el driver de elevación, el catálogo, las pasadas de IR, las herramientas de desensamblado y análisis |
| [tools/remill-patches/](tools/remill-patches/) | los parches aplicados sobre Remill v6.0.1 |
| [scripts/](scripts/) | `make_app.sh`, el instalador y conversor de Steam, los lanzadores |
| [third_party/HLSLcc](third_party/HLSLcc) | traducción de shaders de DXBC a Metal |
| [docs/](docs/) | el estado de la prerelease, la toolchain |

<img src="docs/assets/rule-10.svg" alt="" width="100%">

## Hoja de ruta

1. **Esta beta**: los tres modos a partir de una copia nueva de Steam, en otros Mac distintos del que los compiló. Tus informes
   (modelo de Mac, tasa de fotogramas, qué falló) deciden lo que viene después.
   Primer informe externo, 2026-11-05: en un M2 Pro (16 GB), la conversión se completó y el juego arrancó, tras tres
   correcciones halladas por el camino ([#4](https://github.com/adrbn/bo2-mac/issues/4), [#5](https://github.com/adrbn/bo2-mac/issues/5), [#6](https://github.com/adrbn/bo2-mac/issues/6)).
2. **Sin Terminal**: hecho. Una app que descargar y abrir; incluye sus herramientas ya compiladas, así que no se
   compila nada en tu Mac salvo el propio juego, y se actualiza sola (macOS 26 o posterior; macOS 14 y 15 mantienen la
   instalación por Terminal).
3. **Traducido por completo**: los 46 bloques de código que aún pasan por el intérprete Unicorn se traducen por adelantado,
   y cada instrucción traducida se comprueba contra el intérprete;
   cualquier función traducida puede devolverse al intérprete, para encontrar una incorrecta por bisección.
4. **Sin espera de shaders**: shaders compilados antes de la primera partida, en lugar de al iniciar (unos 3 minutos en multijugador) y
   durante el juego (congelaciones cuando aparece un efecto nuevo). Iniciado: los shaders ahora se compilan por lotes, y los de las
   últimas sesiones se compilan en segundo plano al iniciar. En multijugador en un M1 sin Xcode (unos pocos intentos, no un
   benchmark), la compilación de shaders durante los dos primeros minutos de una partida pasó de 18–25 s a 10–14 s en un primer
   inicio y a unos 5 s en el siguiente; las congelaciones de más de 300 ms pasaron de 15–22 a 13–15, y luego a 7. El primer inicio
   sigue congelándose aproximadamente igual.
5. **Conversión más corta**: de unas 2,5 horas a 1 hora 45 en un M1 (2026-11-06, medido en los tres modos:
   la preparación lee el ejecutable una vez, la elevación y el plegado usan todos los núcleos que permite la memoria). Una actualización ahora rehace
   solo lo que cambió (una corrección del runtime: unos 3 minutos). Siguiente: menos optimización para el código que rara vez se ejecuta. Los tres modos comparten solo el 4 % de
   sus funciones, así que convertir el código compartido una sola vez ganaría poco.
6. **Más rápido, sin calor**: la campaña está limitada por el hilo del planificador, no por el renderizado; el multijugador con 11 bots
   está limitado por el hilo del servidor. Ya empezado: la reverberación se ejecuta de forma nativa en los tres modos, con cerca de un 40 % menos de tiempo en el hilo de sonido. Vienen después las reescrituras nativas de las demás funciones más calientes, y luego tasas de fotogramas superiores a 60 para
   pantallas de 120 Hz.
7. **Mandos**: vibración y aim assist.
8. **Juntos**: la pantalla dividida local en Zombies, que la versión de PC nunca tuvo, es experimental (un juego,
   dos jugadores, mitades superior e inferior, unos 28 fps en un M1), con un interruptor en el menú de la app. Siguiente: sockets UDP reales para el cooperativo de Zombies,
   Grief y Turned, y el multijugador por LAN.
9. **Más adelante**: MetalFX, posprocesado al presentar el fotograma, texturas de reemplazo cargadas desde la propia copia del jugador, y sombras y oclusión ambiental con trazado de rayos en los Mac que lo tienen por hardware (M3 y posteriores).

<img src="docs/assets/rule-11.svg" alt="" width="100%">

## Aviso legal

Para una copia que el usuario posee, en hardware que el usuario posee.

- No se usa código fuente del motor ni código filtrado.
- No se distribuye ni el binario de entrada ni la salida recompilada. No se incluye ningún archivo del juego: el jugador necesita su
  propia copia de Steam de Call of Duty: Black Ops II. El conversor solo acepta la compilación exacta de Steam (SHA-256), y
  la propiedad la comprueba cada vez que se inicia el juego el propio cliente de Steam del jugador: el juego se niega a ejecutarse cuando Steam
  no tiene la sesión iniciada o la cuenta no lo posee.
- La protección contra copia se ejecuta tal como la escribió el juego; no se parchea nada de ella. Este repositorio no ofrece ninguna forma
  de quitar el envoltorio SteamStub de Valve.
- El servicio en línea que espera el juego lo responde un respondedor local escrito a partir de las peticiones del propio juego: no
  se contacta con ningún servidor de Activision, no hay juego en línea, y la propiedad del juego sigue comprobándola el Steam del jugador.
  Las reglas de partida se escribieron en sala limpia.
- El código escrito aquí se licencia bajo GPL-3.0 ([LICENSE](LICENSE)).
- Unicorn (GPL-2.0) nunca se combina con código GPL-3.0 en un binario distribuido. La app incluye la biblioteca sin modificar de Unicorn
  como un archivo aparte junto a las demás herramientas; se enlaza con el binario del juego en el propio Mac del jugador
  ([tools/boot.py](tools/boot.py)), y ese binario nunca se distribuye. Cada release publica el código fuente de la
  versión exacta de Unicorn que incluye (`unicorn-<version>-source.tar.gz`), como exige la GPL-2.0. Las licencias de todas las
  herramientas incluidas están en la app, en `Contents/Resources/toolchain/licenses`.

Este es un proyecto no oficial de aficionados. Call of Duty y Black Ops son marcas registradas de Activision Publishing, Inc. Steam es una
marca registrada de Valve Corporation. Este proyecto no está afiliado a Activision, Treyarch, Microsoft ni Valve, ni cuenta con su respaldo o patrocinio.
Para jugar, compra el juego.

## Star History

<a href="https://www.star-history.com/#adrbn/bo2-mac&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=adrbn/bo2-mac&type=Date" />
  </picture>
</a>

<div align="center"><img src="docs/assets/footer.svg" alt="" width="100%"></div>
