# LumiViz — Datenablage (Konzept)

> **Version:** 0.2.0
> **Datum:** 2026-10-12 (Session 114)
> **Typ:** Konzept — Entscheid D1 b gefallen (Patrik S114): die Ablage wird
> als Ganzes geklärt und gebaut, bevor Stufe 14 des Loops (Palette der
> Instrumente) beginnt. Die Entscheide D2 bis D8 sind gefallen (§4), die
> Skizze zu Export und Import ist gebilligt (§4a); gebaut sind die Stufen A1
> und A2 (§5), im Programm noch nicht gesehen
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Loop_Konzept.md](../audio/Loop_Konzept.md) §4h (Stufe 14,
> Entscheid F3 b) · `CMakeLists.txt` im Projekt-Root (Abschnitt
> „Mitgelieferte Presets", S73) · `THIRD_PARTY_NOTICES.md` („Nicht enthaltene
> Inhalte")
> **Sprache:** Deutsch

---

## 1. Anlass

Bei der Skizze zur Palette der Instrumente (S114) war zu entscheiden, wo
eigene Instrumente liegen. Dabei zeigte sich: die Ablage ist je Bereich
einzeln gewachsen. Patrik: die Vorgehensweisen seien nicht strikt geklärt;
`asset/` liege im Projekt-Root und fehle neben der Release-Exe; und für das
Eigene im Benutzerordner brauche es einen Export und Import, der Teile oder
alles sichert.

## 2. Befund (S114, nachgesehen)

### 2.1 Mitgeliefertes

Jeder Bereich sucht seinen Ordner selbst: vom Ordner der Exe aufwärts, bis zu
zwölf Ebenen, bis `asset/<Bereich>` gefunden ist. Im Entwicklungsbaum trifft
das den Projekt-Root. Fünf der sechs Stellen sind je eine eigene Kopie
derselben Schleife; vier nehmen eine `README.md` im Ordner als Anker.

| Bereich | sucht | Stelle im Code | neben der Release-Exe |
|---|---|---|---|
| Modul-Presets | `asset/nodepresets` (Anker README) | `NodePresetStore.cpp` | fehlt |
| Shader-Vorlagen | `asset/shadertemplates` | `ShaderVorlagen.cpp` | fehlt |
| Trail-Presets | `asset/karaoke/trails` | `TrailPresetStore.cpp` | fehlt |
| Blenden, Formen | `asset/composer/…` (Anker README) | `FormPresets.cpp` | fehlt |
| Mutes | `asset/composer/…` | `Mute.cpp` | fehlt |
| Kit, Instrumente | `asset/loop` | `Kit.cpp`, `Instrument.cpp` | fehlt |
| Kalibrier-Bilder | `asset/calibration` | `ScreenshotManager.cpp` | fehlt |

Neben der Exe liegen nach dem Bau nur: `presets` (aus `asset/presets`, 32
Dateien, ohne die README), `docs` (drei Anleitungen), die Lizenzdateien und
`sichttest`. Das Kopieren steht im `CMakeLists.txt` des Projekt-Roots als
Schritt nach dem Bau (S73), nicht in CMakeCraft.

**Eine Grenze, die schon gilt:** neben die Exe dürfen nur eigene Werke
(`CMakeLists.txt`, S73; `THIRD_PARTY_NOTICES.md`). `asset/` als Ganzes
kopieren geht deshalb nicht — dort liegen auch `avs` (851 Dateien),
`Milkdrop3` (1240), `shadertoys` (125), `calibration` (1631) und weitere,
deren Herkunft hier nicht je Ordner geprüft ist.

**Nicht nachgesehen:** wer den Ordner `presets` neben der Exe liest; ob die
Suche aufwärts von einem installierten Ort aus einen fremden Ordner `asset`
treffen kann; was CMakeCraft für das Mitkopieren von Ordnern anbietet.

### 2.2 Eigenes

Benutzerordner: `AppData\Roaming\LumiViz Project\LumiViz` (Qt:
`AppDataLocation`). Darin heute:

| Unterordner | Inhalt | Muster |
|---|---|---|
| `presets/visuals/<Visualizer>` | Visualizer-Presets | nur eigene |
| `presets/gradients`, `…/smoothing`, `…/audio` | Verläufe und weitere | nur eigene |
| `nodepresets/<Typ>` | Modul-Presets | mitgeliefert + eigen, eigenes verdrängt |
| `shadertemplates` | Shader-Vorlagen | mitgeliefert + eigen |
| `karaoke/trails` | Trail-Presets | mitgeliefert + eigen, eigenes verdrängt |
| `composer/fades`, `…/mutes`, Formen | Blenden, Mutes, Formen | mitgeliefert + eigen, eigenes verdrängt |
| `shadertoy` | importierte Shadertoy-Ketten | nur eigene |
| `lyrics` | geladene Songtexte (bei Patrik 175 Dateien) | Zwischenspeicher |
| `testaufnahmen` | Aufnahmen aus dem Video-Test | Arbeitsdaten |

Daneben, mit Dateidialog an beliebigem Ort: `.lvloop`, `.lvinst`, `.lvcomp`,
`.lvfx`, `.lvkar`. Die Einstellungen (Favoriten, zuletzt benutzte Ordner,
Haken, Fensterlage) stehen über `QSettings` in der Registry.

### 2.3 Sichern

Es gibt nur den Knopf im Settings-Panel, der den Benutzerordner im Explorer
öffnet (`SettingsPanel::onOpenAppDataDir`). Kein Export, kein Import.

## 3. Die Regel (von Claude vorgeschlagen; über die Entscheide in §4 im Einzelnen festgelegt)

1. **Mitgeliefertes** liegt im Repo unter `asset/<Bereich>` und nach dem Bau
   als `asset/<Bereich>` neben der Exe. Kopiert wird eine **Liste von
   Bereichen**, nicht `asset/` als Ganzes (§2.1, eigene Werke). Das Programm
   fragt an **einer** Stelle nach dem Ordner eines Bereichs: zuerst neben der
   Exe, dann aufwärts (Entwicklungsbaum).
2. **Eigenes mit Namen** (Presets, Vorlagen, Instrumente) liegt im
   Benutzerordner, je Bereich ein Unterordner; ein eigenes mit dem Namen
   eines mitgelieferten verdrängt es; nur eigene lassen sich löschen.
   **Werke** (Loop, Composition, Kette, Karaoke) bleiben Dateien am frei
   gewählten Ort.
3. **Export und Import:** ein Dialog schreibt den Benutzerordner ganz oder
   nach Bereichen in eine Datei und liest sie wieder ein; bei gleichem Namen
   fragt er.

Folge für Stufe 14 des Loops: eigene Instrumente liegen im Benutzerordner
unter `instruments`; „My files" im Fenster „Instruments" zeigt sie.

## 4. Entscheide (D2 bis D8, einzeln gestellt)

**Entschieden — D2 a (Patrik S114):** neben die Exe kommen `nodepresets`,
`shadertemplates`, `karaoke`, `composer`, `loop` (Kit, Instrumente und die
Beispiel-Loops, ohne die Erzeuger-Skripte und ohne Arbeitsdateien) und wie
bisher `presets`. Verworfen c (ohne Beispiel-Loops). **Im Nachgang zu
klären, ohne Priorität** (Patrik: „da sollten schon examples dazu"):
Beispiele aus `examples`, `composits`, `shadertoys` und `effectchain` — je
Ordner erst die Herkunft prüfen (nur eigene Werke, §2.1). Bei den vier
Ordnern `nodepresets`, `shadertemplates`, `karaoke`, `composer` ist die
Herkunft als eigen angenommen, nicht nachgelesen.

**Entschieden — D3 a (Patrik S114), mit seiner Ergänzung:** das Kopieren
bleibt im Projekt. Eine eigene Datei **`asset/Asset.cmake`** listet geordnet,
welche Bereiche neben die Exe kommen und was je Bereich draußen bleibt
(Pflegenotizen, Erzeuger-Skripte, Arbeitsdateien) — „dann ist sofort klar,
wo man entsprechende Anpassungen macht". Das `CMakeLists.txt` im Root bindet
sie ein; ein eigenes Ziel gleicht bei jedem Bau ab, `presets` läuft darüber
mit. Der Abschnitt aus S73 wird entsprechend kürzer. Name und Ort der Datei
(im Ordner `asset`, nach dem Vorbild der `Source.cmake` je Ordner) sind von
Claude gesetzt. In CMakeCraft wird ein Wunsch vermerkt: Daten-Ordner einer
App als Angabe in `Solution.json` (heute gibt es `runtime.dirs` nur für
Externals). Verworfen b (zuerst CMakeCraft: eine zweite Sitzung und ein
Pin-Wechsel, bevor etwas neben der Exe liegt), c (wie heute, nur mehr
Schritte — die Kopie liefe weiter nur beim Neulinken).

**Entschieden — D4 a (Patrik S114):** `presets` bleibt als `<Exe>/presets`.
Unter `<Exe>/asset/` liegt, was das Programm selbst sucht (Vorlagen, Kit,
Instrumente, Voreinstellungen); `presets` ist die Sammlung, die der Anwender
im Effect Browser öffnet, und liegt sichtbar neben der Exe. `Asset.cmake`
führt beide Arten in zwei Abschnitten. Die Beispiele aus dem Nachgang zu D2
sind von der Art von `presets`. Verworfen b (nach `asset/presets`: ein
gemerkter Pfad wäre umzuschreiben, sieben Stellen in Handbuch, Quickstart
und README; `StartPreset.hpp` nennt den Ort fest).

**Entschieden — D5 a (Patrik S114):** der Export ist eine Zip-Datei mit der
Endung `.lvdata`; darin die Unterordner wie im Benutzerordner und eine
`manifest.json` (Fassung, LumiViz-Version, Datum, die enthaltenen Bereiche
mit Zahl). Geschrieben und gelesen mit **miniz** als weiteres External unter
`externals/`, mit Eintrag in `THIRD_PARTY_NOTICES.md` — Lizenz und Umfang vor
dem Einbinden an der Quelle prüfen (bisher aus dem Gedächtnis: eine
Quelldatei, MIT). Verworfen b (private Zip-Klassen von Qt 6.10.1,
`qzipwriter_p.h`: ohne Zusage), c (ein Ordner), d (eine JSON-Datei mit allem
als Text). **Vorgemerkt (Patrik):** `.lvdata` bekommt ein eigenes Icon wie
die anderen Formate — so gelesen von Claude, bei Patrik nachgefragt.

**Entschieden — D6 a (Patrik S114):** der Dialog bietet zusätzlich
„Settings" und „Downloaded lyrics" an, beide anfangs nicht angehakt. Von den
Einstellungen gehen nur die mit, die der Anwender selbst gesetzt hat und die
auf einem anderen Rechner Sinn haben (Favoriten und zuletzt benutzte Listen
der Paletten, Tastenkürzel, Haken und Optionen); nicht mit gehen Fensterlage,
Bildschirmwahl, GPU-Wahl und alle gemerkten Ordner- und Dateipfade. Zu den
Songtexten gehören die Versätze und Merker je Titel. Die Testaufnahmen der
Kamera bietet der Export nicht an. Verworfen b (nur die benannten eigenen
Dinge), c (alles, Einstellungen vollständig). **Vor dem Bau:** die Liste der
Schlüssel „geht mit / geht nicht mit" aus dem Code ziehen und Patrik
vorlegen; nachsehen, wo Versätze und Merker der Songtexte liegen.

**Entschieden — D7 a mit Ergänzung (Patrik S114: „mit Option auf c, wie es
Windows macht"):** der Dialog fragt einmal für den ganzen Import, vier
Stellungen: **Keep mine, skip the imported one** (Vorgabe), **Replace mine**,
**Keep both, add a number to the imported name**, **Let me decide for each
file**. Die vierte öffnet eine Liste der betroffenen Namen, je Zeile die
eigene und die importierte Datei mit Datum und Größe und je einem Haken —
beide angehakt heißt beide behalten (nach dem Vorbild des Kopierdialogs von
Windows; die Gestalt der Liste ist von Claude so gelesen, noch ohne Bild).
Eine importierte Datei, die Byte für Byte der eigenen gleicht, zählt nicht
als gleicher Name und wird still übergangen. Vor dem Import steht da, wie
viele Namen betroffen sind; danach eine Meldung mit den Zahlen (eingelesen,
übersprungen, ersetzt, umbenannt). Für „Settings" gilt die Regel nicht: sie
ersetzen die eigenen. Verworfen b (eine Regel je Bereich). Nicht geprüft:
bei „Keep both" zeigt ein Werk, das ein Instrument beim Namen nennt, weiter
auf das eigene.

**D8 — mit der Skizze gebilligt (Patrik S114):** siehe §4a.

**Entschieden — D9 a (Patrik S114):** das Programm nimmt
`<Exe>/asset/<Bereich>`, wenn der Ordner da ist; nur wenn er fehlt, sucht es
aufwärts wie bisher (Anker `README.md`). So liest jeder Start von
`LumiViz.exe` aus dem Bau-Ordner, was auch eine weitergegebene Exe liest. Das
Ziel aus `Asset.cmake` gleicht bei jedem Bau ab und entfernt, was im Repo
gelöscht wurde. **Folge:** wer eine Datei unter `asset/` ändert, braucht
einen Bau, bevor `LumiViz.exe` sie sieht. Die Tests liegen in einem anderen
Ordner, bekommen keine Kopie und lesen weiter aus dem Repo. Beim Start
schreibt das Programm ins Log, aus welchem Ordner es jeden Bereich liest.
**Ergänzung Patrik:** die Reihenfolge b (zuerst aufwärts, also aus dem
Projektordner) ist in den Settings umschaltbar; Vorgabe ist a. Von Claude
dazu gesetzt: der Schalter wirkt nur im Entwicklungsbaum — neben einer
weitergegebenen Exe gibt es aufwärts nichts, beide Reihenfolgen lesen dann
dasselbe; er gilt nach einem Neustart (die Ordner werden einmal aufgelöst
und gemerkt). Als alleinige Reihenfolge verworfen: b (die Kopie würde im
Entwicklungsbaum nie gelesen und nie geprüft). Nicht gemessen: die Dauer des Abgleichs, wenn sich
nichts geändert hat.

**Nachtrag zu D2 (S114, beim Lesen aller Sucher):** ein achter Sucher holt
die Format-Icons aus `asset/img/logo/icons` (`PresetTypeIcons.hpp`;
`PresetSpeichern.hpp` nimmt `lumiviz.ico` dort als Anker). Der Ordner fehlt
in D2 a. **Entschieden — D10 c (Patrik S114):** `img/logo/icons` kommt als
neunter Bereich neben die Exe, nur die `.ico` und `.svg`, dazu
`register_file_icons.ps1` (es trägt die Icons mit dem Ordner ein, in dem es
liegt — nachgelesen); die Python-Skripte und die README bleiben draußen.
Verworfen a (ohne das Skript), b (als Qt-Ressource in die Exe). Vorgemerkt:
`.lvloop`, `.lvinst` und `.lvdata` haben noch kein Icon und stehen nicht im
Skript. Der Ordner der Kalibrier-Screenshots
(`ScreenshotManager`) ist ein Ziel zum Schreiben und bleibt, wie er ist.

## 4a. Export und Import: die Skizze (gebilligt, Patrik S114; nicht gebaut)

**Im Settings-Panel** bekommt die Zeile **User data** neben **Open folder**
zwei Knöpfe: **Export...** und **Import...**.

**Export your data** (Dialog): oben **Select all** und **Select none**.
Darunter eine Liste mit Haken, je Zeile ein Bereich und rechts die Zahl der
eigenen Dateien: Visualizer presets, Gradients, Module presets, Shader
templates, Karaoke trails, Composer fades, shapes, mutes, Shadertoy imports,
Instruments (ab Stufe 14 des Loops). Anfangs alle angehakt. Abgesetzt
darunter, anfangs aus: **Settings (favorites, recent lists, options)** und
**Downloaded lyrics** mit Zahl (D6 a). Unten der Zielpfad mit einem Namen aus
dem Datum (`LumiViz_2026-10-12.lvdata`) und **Browse...**; eine Zeile „39
files in 8 groups"; **Cancel**, **Export**.

**Import data** (Dialog, nach der Wahl der Datei): oben der Dateiname und
„exported with LumiViz x.y.z". Die Liste zeigt **nur die Bereiche, die in der
Datei stecken** — ist mit allem exportiert worden, ist sie so lang wie beim
Export (Rückfrage Patrik, bestätigt). Je Zeile ein Haken, die Zahl und „n
exist", wenn es Namen schon gibt; bei Settings „replaces yours", anfangs aus.
Darunter „If a name already exists" mit den Stellungen aus D7. Unten „n names
already exist", **Cancel**, **Import**. Danach die Meldung mit den Zahlen;
die Listen im Programm zeigen das Neue ohne Neustart (nicht geprüft, ob das
überall geht).

Nicht im Export: die Werke am frei gewählten Ort (`.lvloop`, `.lvcomp`,
`.lvfx`, `.lvkar`) und die Testaufnahmen der Kamera.

## 5. Stufen (Vorschlag)

| Stufe | Inhalt | Stand |
|---|---|---|
| A1 | Eine Stelle für „Ordner eines mitgelieferten Bereichs"; die sieben Sucher rufen sie | **gebaut (S114):** `core/Mitgeliefert.hpp/.cpp` — `suche` (rein), `ordner` (mit Einstellung und einer Log-Zeile je Bereich), der Schalter „Bundled Data: Read from the project folder first" im Settings-Panel (nur im Entwicklungsbaum zu sehen). Aufrufer: `NodePresetStore`, `ShaderVorlagen`, `TrailPresetStore`, `FormPresets`, `Mute`, `Kit` (damit auch die Instrumente), `PresetTypeIcons`. `PresetSpeichern` blieb: der Programmordner zählt dort schon als mitgeliefert. Sechs Fälle in `test_Mitgeliefert.cpp`. **Im Programm nicht gesehen** |
| A2 | `asset/Asset.cmake`: die Bereiche bei jedem Bau als `asset/…` neben die Exe, `presets` über dasselbe Ziel (D2 a, D3 a, D4 a) | **gebaut (S114):** `asset/Asset.cmake` (die Liste), `asset/AssetAbgleich.cmake` (führt sie aus), das Ziel `LumiViz_Assets` im `CMakeLists.txt`, von dem `LumiViz` abhängt. 421 Dateien; ein Lauf ohne Änderung rund 1,1 s, der erste rund 2,2 s (je einmal gemessen, gegen einen Wegwerf-Ordner). Belegt: in beiden Bau-Ordnern liegen alle Bereiche. Nicht belegt: eine weitergegebene Exe an einem Ort ohne Projekt darüber |
| A3 | Eigene Instrumente im Benutzerordner (mit Stufe 14 des Loops) | offen |
| A4 | Export und Import (D5 a, D6 a, D7 a, §4a): miniz als External, Dialoge, die Liste der Einstellungen | offen; vorher die Liste der Schlüssel vorlegen und ein Bild der Liste „Let me decide for each file" |

## 6. Changelog

- **0.2.0** (S114): Entscheide D2 a, D3 a (mit `asset/Asset.cmake`), D4 a,
  D5 a, D6 a, D7 a (mit „Let me decide for each file"), D9 a (mit Schalter),
  D10 c; die Skizze zu Export und Import (§4a); Stufen A1 und A2 gebaut.
  Regel beim Bau gefunden: die Suche aufwärts beginnt wie bisher im
  Programmordner selbst, ohne App-Objekt im Arbeitsordner — sonst finden
  Tests ohne `QCoreApplication` nichts (fünf Fälle fielen so im ersten Lauf).

- **0.1.0** (S114): angelegt. Befund, Vorschlag der Regel in drei Teilen,
  offene Entscheide D2 bis D8. Entscheid D1 b (Patrik): erst die Ablage, dann
  Stufe 14.
