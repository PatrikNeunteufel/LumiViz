# BASS beschaffen

**BASS ist nicht Teil dieses Repositorys und kann es nicht sein.** Die
Bibliothek gehört un4seen developments Ltd.; ihre Lizenz schließt
Weiterverbreitung und Unterlizenzierung aus. Betroffen ist das **gesamte SDK** —
Binärdateien ebenso wie Header, Beispiele und Dokumentation.

Dieser Ordner ist deshalb leer bis auf diese Anleitung. Du musst BASS einmalig
selbst herunterladen und hier entpacken.

> **Symptom, wenn dieser Schritt fehlt:** Der Build konfiguriert sauber und
> bricht erst beim Linken ab, mit einer Meldung wie
> `…/externals/bass/bass24/win/c/x64/bass.lib … missing and no known rule to make it`.

## Lizenz — bitte vorher lesen

BASS ist **kostenlos für nicht-kommerzielle Nutzung**. Sobald du damit Geld
verdienst — Verkauf, Werbung, kommerzieller Vertrieb — brauchst du eine
Lizenz von un4seen. Die Bedingungen stehen in der `bass.txt` des Pakets,
Abschnitt *Licence*, und auf <https://www.un4seen.com/>.

Das gilt unabhängig von der LumiViz-Lizenz: LumiViz selbst steht unter MIT bzw.
Apache-2.0, BASS nicht. Siehe [`../../THIRD_PARTY_NOTICES.md`](../../THIRD_PARTY_NOTICES.md).

## Benötigte Pakete

LumiViz braucht zwei Pakete (`Solution.json`: External `bass` mit `BASS_FLAC: true`):

| Paket | Download | Zielordner hier |
|---|---|---|
| **BASS** 2.4 | <https://www.un4seen.com/bass.html> | `bass24/` |
| **BASSFLAC** | <https://www.un4seen.com/> (Add-ons) | `bassflac24/` |

Weitere Add-ons (Opus, WMA, WASAPI, MIDI, Mix, Encoder …) unterstützt das
Build-System ebenfalls — sie werden nur gebraucht, wenn du sie in `Solution.json`
unter `external_options` einschaltest. Das Namensschema ist immer dasselbe wie unten.

## Wohin die Dateien gehören

Lade das ZIP je Paket und entpacke es in einen **gleichnamigen Unterordner**
dieses Verzeichnisses. Die ZIP-Struktur von un4seen passt dabei unverändert.
Gebraucht werden pro Plattform:

### Windows x64

```
externals/bass/bass24/win/c/bass.h            ← Header
externals/bass/bass24/win/c/x64/bass.lib      ← Import-Library (Linker)
externals/bass/bass24/win/x64/bass.dll        ← Laufzeit-DLL (wird neben die Exe kopiert)

externals/bass/bassflac24/win/c/bassflac.h
externals/bass/bassflac24/win/c/x64/bassflac.lib
externals/bass/bassflac24/win/x64/bassflac.dll
```

> **Häufigster Fehler:** `win/c/bass.lib` (ohne `x64`) ist die **32-Bit**-Variante.
> Für einen x64-Build muss die Datei aus dem `x64`-Unterordner kommen.

### Linux x86_64

```
externals/bass/bass24/linux/bass.h
externals/bass/bass24/linux/libs/x86_64/libbass.so

externals/bass/bassflac24/linux/bassflac.h
externals/bass/bassflac24/linux/libs/x86_64/libbassflac.so
```

### macOS

```
externals/bass/bass24/osx/c/bass.h
externals/bass/bass24/osx/libbass.dylib
```

## Prüfen

Nach dem Entpacken muss diese Datei existieren (Windows x64):

```bash
ls externals/bass/bass24/win/c/x64/bass.lib
```

Danach konfigurieren wie in [`../../BUILDING.md`](../../BUILDING.md) beschrieben.

## Warum das nicht automatisch geht

Ein Prefetch-Hook, der die Pakete beim Konfigurieren selbst holt, wäre technisch
möglich — das Build-System hat die Infrastruktur dafür. Er ist bewusst nicht
gebaut: ein automatischer Download würde die Lizenzentscheidung an dir
vorbeitreffen. Wer BASS nutzt, soll die Bedingungen einmal gesehen haben.

## Hinweis für bestehende Arbeitskopien

Hast du LumiViz schon einmal gebaut und richtest einen zweiten Checkout ein,
kopierst du die Dateien am schnellsten direkt herüber:

```bash
cp -r <alter-checkout>/externals/bass/bass24    externals/bass/
cp -r <alter-checkout>/externals/bass/bassflac24 externals/bass/
```
