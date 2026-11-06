#!/bin/bash
# update_app.sh <app> — run by the app's launcher, at most once a day: when a newer release of adrbn/bo2-mac than the
# app's (Contents/Resources/release, from make_app.sh's BO2_RELEASE) is out, asks the player, downloads its bo2-mac.zip,
# checks it is signed by team 2TWQF4T93E and notarized, and puts it in the app's place. "Install, keep this version"
# first renames the app "<name> <its tag>.app": opening that one goes back (the launcher relinks, scripts/signature.sh).
# A test build is offered test releases, a full release only full releases; an app whose tag is not published (a build
# not yet released) is offered nothing. Status 10: installed, the launcher opens the new app; else go on as before.
# Tests: BO2_UPDATE_API (the release list, file:// works), BO2_UPDATE_ANSWER=later|keep|install (no dialog).
A=$1; CUR=$(cat "$A/Contents/Resources/release" 2>/dev/null) || exit 0
ST="$HOME/.bo2-native/update_checked"
[ -n "${BO2_UPDATE_ANSWER:-}" ] || [ -z "$(find "$ST" -mtime -1d 2>/dev/null)" ] || exit 0
D=$(mktemp -d "${TMPDIR:-/tmp}/bo2-update.XXXXXX") || exit 0; trap 'rm -rf "$D"' EXIT
curl -fsSL -m 8 -o "$D/l.json" "${BO2_UPDATE_API:-https://api.github.com/repos/adrbn/bo2-mac/releases?per_page=20}" || exit 0
mkdir -p "$HOME/.bo2-native" && touch "$ST"
# The newest release after ours on our channel, with its zip: tag, zip URL, title, page.
R=$(osascript -l JavaScript - "$CUR" "$D/l.json" <<'EOF'
function run(a) {
  ObjC.import('Foundation')
  const L = JSON.parse($.NSString.stringWithContentsOfFileEncodingError(a[1], $.NSUTF8StringEncoding, null).js).filter(r => !r.draft)
  const i = L.findIndex(r => r.tag_name === a[0]); if (i < 0) return ''
  const n = L.slice(0, i).find(r => L[i].prerelease || !r.prerelease); if (!n) return ''
  const z = n.assets.find(x => x.name === 'bo2-mac.zip'); return z ? [n.tag_name, z.browser_download_url, n.name, n.html_url].join('\n') : ''
}
EOF
) && [ -n "$R" ] || exit 0
NEW=$(sed -n 1p <<< "$R"); URL=$(sed -n 2p <<< "$R"); NAME=$(sed -n 3p <<< "$R"); PAGE=$(sed -n 4p <<< "$R")
case "$(defaults read -g AppleLanguages 2>/dev/null | tr -dc 'a-zA-Z-\n' | sed -n 2p | cut -c1-2)" in
  fr) Q="Une nouvelle version de Black Ops II est sortie :"; Q2="Seul ce qu'elle change est reconverti."; L="Plus tard"
      K="Installer, garder celle-ci"; I="Installer"; T="Téléchargement…"; X="La mise à jour n'a pas pu être installée. Elle s'ouvre dans votre navigateur.";;
  de) Q="Eine neue Version von Black Ops II ist da:"; Q2="Nur was sie ändert, wird neu umgewandelt."; L="Später"
      K="Installieren, diese behalten"; I="Installieren"; T="Wird geladen…"; X="Das Update konnte nicht installiert werden. Es öffnet sich im Browser.";;
  es) Q="Hay una nueva versión de Black Ops II:"; Q2="Solo se vuelve a convertir lo que cambia."; L="Más tarde"
      K="Instalar y conservar esta"; I="Instalar"; T="Descargando…"; X="No se pudo instalar la actualización. Se abre en el navegador.";;
  it) Q="È uscita una nuova versione di Black Ops II:"; Q2="Viene riconvertito solo ciò che cambia."; L="Più tardi"
      K="Installa e tieni questa"; I="Installa"; T="Download in corso…"; X="Impossibile installare l'aggiornamento. Si apre nel browser.";;
  pt) Q="Há uma nova versão do Black Ops II:"; Q2="Só o que ela muda é convertido de novo."; L="Mais tarde"
      K="Instalar e manter esta"; I="Instalar"; T="Baixando…"; X="Não foi possível instalar a atualização. Ela abre no navegador.";;
  pl) Q="Jest nowa wersja Black Ops II:"; Q2="Ponownie konwertowane jest tylko to, co się zmienia."; L="Później"
      K="Zainstaluj, zachowaj tę"; I="Zainstaluj"; T="Pobieranie…"; X="Nie udało się zainstalować aktualizacji. Otworzy się w przeglądarce.";;
  ru) Q="Вышла новая версия Black Ops II:"; Q2="Заново конвертируется только то, что изменилось."; L="Позже"
      K="Установить, сохранив эту"; I="Установить"; T="Загрузка…"; X="Не удалось установить обновление. Оно откроется в браузере.";;
  ja) Q="Black Ops II の新しいバージョンがあります:"; Q2="変更された部分だけが再変換されます。"; L="後で"
      K="インストール(このバージョンを残す)"; I="インストール"; T="ダウンロード中…"; X="アップデートをインストールできませんでした。ブラウザで開きます。";;
  *)  Q="A new version of Black Ops II is out:"; Q2="Only what it changes is converted again."; L="Later"
      K="Install, keep this version"; I="Install"; T="Downloading…"; X="The update could not be installed. It opens in your browser.";;
esac
B=${BO2_UPDATE_ANSWER:-$(osascript - "$Q

$NAME

$Q2" "$L" "$K" "$I" <<'EOF'
on run a
  return button returned of (display dialog (item 1 of a) with title "Black Ops II" buttons {item 2 of a, item 3 of a, item 4 of a} default button 3 cancel button 1)
end run
EOF
)}
case "$B" in "$K"|keep) B=keep;; "$I"|install) B=install;; *) exit 0;; esac
echec() { [ -n "${BO2_UPDATE_ANSWER:-}" ] || { osascript -e 'on run a' -e 'display alert "Black Ops II" message (item 1 of a)' -e 'end run' "$X" >/dev/null; open "$PAGE"; }
  echo "update $NEW: $1"; exit 0; }
[ -n "${BO2_UPDATE_ANSWER:-}" ] || osascript -e 'on run a' -e 'display notification (item 1 of a) with title "Black Ops II" subtitle (item 2 of a)' -e 'end run' "$NAME" "$T"
curl -fsSL -m 1800 -o "$D/a.zip" "$URL" || echec "download failed"
ditto -x -k "$D/a.zip" "$D/x" && N=$(echo "$D"/x/*.app) && [ -d "$N" ] || echec "no app in the zip"
codesign --verify --deep --strict "$N" 2>/dev/null && codesign -dv "$N" 2>&1 | grep -q '^TeamIdentifier=2TWQF4T93E$' \
  && spctl -a -t exec "$N" 2>/dev/null || echec "not signed by team 2TWQF4T93E, or not notarized"
O="$D/old"; [ $B = install ] || { O="${A%.app} $CUR.app"; rm -rf "$O"; }
mv "$A" "$O" || echec "cannot move $A"
mv "$N" "$A" || { mv "$O" "$A"; echec "cannot write $A"; }
echo "update: $CUR -> $NEW$([ $B = install ] || echo ", kept as $O")"; exit 10
