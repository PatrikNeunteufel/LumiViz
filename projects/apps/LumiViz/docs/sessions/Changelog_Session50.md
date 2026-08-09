# Changelog Session 50 (2026-07-26)

Fokus: das Preset-Pack **Whacko Revisited** gegen AvsRef durchgemessen. Elf
Befunde, eine neue Modul-Sonden-Suite und eine dokumentierte Kalibrier-Methodik.

## Texer II — vier Befunde

- **Blob-Decoder**: der Bildname ist ein FESTER 260-Byte-Puffer mit
  NUL-Terminierung, keine längenpräfixierte Zeichenkette. Der alte Decoder las die
  ersten vier Namensbytes als Länge — Dateiname, Flags und **alle vier EEL-Slots**
  kamen leer heraus, gezeichnet wurde ein einzelner Default-Punkt. Layout an
  **579 Blobs** der Sammlung gepinnt (280 Byte Vorlauf, in allen 579 gleich).
- **Default-Sprite gemessen**: 20×20, Peak 252, weiches radiales Profil (aus einem
  Referenz-Rendering ausgelesen). Vorher 16×16-Kegel, dessen Profil durch
  `alpha=v` zusätzlich quadriert wurde → nur 14 px sichtbar. Ausdehnung =
  20 px · `sizex`, linear über 0,5..8 bestätigt.
- **Blend folgt dem BLEND_LINE-Modus** (Default REPLACE) statt fest additiv:
  derselbe Sprite über 6 Frames ließ die Bildenergie in AvsRef unverändert
  (176901), fest additiv wuchs sie auf das 1,86-fache.
- **`n`-Semantik**: Untergrenze 0 statt 1, und Vorbelegung 0 statt 100. Ohne `n`
  im Skript zeichnet AVS gar nichts.

## Effect-List `enabled` / `clear`

Der im Preset gespeicherte Schalter ist nur die **Vorbelegung** der EEL-Variablen
(r_list.cpp:399), danach entscheidet das Skript (:419). Eine deaktivierte Liste
mit Code wird nicht mehr vorab übersprungen; beide Variablen werden je Frame aus
dem gespeicherten Zustand vorbelegt statt einmalig beim Compile. Dahinter steckt
ein Standard-Idiom (Liste schaltet sich im ersten Frame selbst ein, um einen
Global-Buffer zu füllen) — allein in diesem Pack 75 Config-APEs.

## SuperScope: geteilter ScriptContext

Der Scope-Host wurde ohne ScriptContext gebaut, anders als Grid, Texer und Liste —
jeder Scope hatte einen isolierten `reg00..reg99`-Raum und las überall 0. AVS hält
diese Register global. Presets, die Kamera und Scopes darüber koppeln, verloren
ihren kompletten Vordergrund.

## Renderer

- **`linesize` je Punkt** statt je Frame (r_sscope wertet je Punkt aus). Die
  Lauf-Zerlegung im Host trennt jetzt zusätzlich nach Breite.
- **Punkt-Modus**: je Punkt genau ein getrunkiertes Ganzzahl-Pixel statt
  `GL_POINTS` mit Punktgröße und weichem Rand. Gebunden an `dotSize <= 1`, das der
  Translator nur für Importe setzt — LumiViz-eigene Ketten behalten runde Punkte.
- **Convolution-Kern war vertikal gespiegelt**: die Kernzeile zählt von oben, die
  Texturkoordinate läuft nach oben.

## Import

Bild-Auflösung sucht bis zu drei **Elternebenen** hoch. AVS legt Bilder im
AVS-Wurzelverzeichnis ab, die Presets stehen in Unterordnern — Texer II und
Picture II fanden ihre Dateien deshalb nie und meldeten „image not found".

## Werkzeuge

- Kalibrier-Skripte **mojibake-sicher**: UTF-8-Konsole (cp1252 warf bei einem
  Preset-Namen mit „š" eine Ausnahme in der Fehlerausgabe und riss den ganzen
  Sweep mit) und ASCII-sicheres Staging für AvsRef (ANSI-Programm).
- **Modul-Sonden-Suite** (`make_module_probes.py` / `run_module_probes.py`):
  22 Sonden in vier Stufen, Urteil flächenunabhängig über gezeichnete Pixelmenge
  und Schwerpunkt statt dMean. Ergänzt die Modul-Matrix, die nur Builtins mit
  Default-Parametern prüft.
- **APE-Bauer** in `avs_preset_lib.py` (Texer II, Convolution, Listen-Config,
  abschaltbare Liste) — vorher war kein APE als Testpreset baubar.
- **`docs/visuals/AVS_Kalibrier_Methodik.md`** — Standard-Vorgehen für Kalibrierung
  und Fehlersuche, im `docs/INDEX.md` verlinkt.

## Stand

- **LumiViz.UnitTests 432/432 grün, 0 Skips** (S49: 428) — vier neue Gates.
- **Modul-Matrix 37/41**, unverändert dieselben vier Reste.
- **Modul-Sonden 21/22**.
- **Whacko-Sweep 32/32 durchgelaufen, 6 grün** (vorher: Abbruch am Nicht-ASCII-
  Dateinamen, 1 grün). Mister Santa 0,781 → 0,006, Inside Gamecube 0,573 → 0,005.
- Drei Presets netto schlechter (Deep Red Sea, Alternate Reality, Inhaler) — der
  jetzt geteilte `rand()`-Strom legt dort eine bestehende Fehlausrichtung frei.
