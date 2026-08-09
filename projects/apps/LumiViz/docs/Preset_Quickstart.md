# Preset-Quickstart — in fünf Schritten zum eigenen Visual

> **Version:** 1.0.0
> **Datum:** 2026-08-09 (Session 73)
> **Typ:** Quickstart
> **Status:** Aktiv
> **Sprache:** Deutsch
> **Zielgruppe:** Alle, die gerade zum ersten Mal LumiViz offen haben

Eine Seite, ein Ergebnis. Wenn du danach weitermachen willst:
[Preset-Anleitung](Preset_Anleitung.md) erklärt das Warum,
[Benutzerhandbuch](Benutzerhandbuch.md) die Bedienung im Einzelnen.

---

## 1. Musik an

Ohne Ton passiert wenig — fast alles in LumiViz hängt am Audiosignal.
**File → Open** oder eine Datei ins Player-Panel ziehen, Play drücken.

## 2. Etwas ansehen, das schon läuft

Neben der Exe liegt der Ordner **`presets/`** mit mitgelieferten Presets.
Im **Import Browser** (Panel rechts) hineingehen und ein `.avs` oder `.milk`
**doppelklicken** — es lädt sofort.

Mit **Bild ab / Bild auf** blätterst du durch den Ordner, ohne die Maus
anzufassen. Das ist der schnellste Weg, ein Gefühl dafür zu bekommen, was die
Formate hergeben.

## 3. Die Kette aufmachen

**View → Panels → Effect Chain.**

Was du siehst, ist das geladene Preset als **Baum**: oben die Effekte, die
zuerst laufen, darunter die späteren. Klick einen Eintrag an — darunter
erscheint sein **Eigenschafts-Editor**.

Jetzt das Wichtigste: **schalte mit dem Auge-Symbol einzelne Effekte aus und
wieder ein.** In zwei Minuten verstehst du, welcher Eintrag welchen Teil des
Bildes macht. Das ist mehr wert als jede Erklärung.

## 4. Etwas verändern

Dreh im Eigenschafts-Editor an Werten und schau zu. Nichts kann kaputtgehen —
die Datei auf der Platte bleibt unberührt, bis du speicherst.

Wenn du eigene Effekte hinzufügen willst: in der Toolbar den Typ im Dropdown
wählen und **+** drücken. Es gibt 54 Typen; als Einstieg eignen sich

| Typ | macht |
|---|---|
| `superScope` | zeichnet Linien aus einer Formel — das Herz der AVS-Ästhetik |
| `blur` | weichzeichnen, mit Nachzieh-Effekt |
| `movement` | verzerrt das ganze Bild (Zoom, Drehung, Tunnel) |
| `colorMap` | färbt das Bild nach Helligkeit ein |
| `onBeatClear` | löscht im Takt |

Reihenfolge ändern: **↑/↓** oder Drag & Drop. **Die Reihenfolge ist der halbe
Look** — derselbe Blur vor oder nach einem Movement ergibt zwei verschiedene
Bilder.

## 5. Speichern

**File → Save Effect Chain** legt eine `.lvfx` an — das eigene Preset-Format
von LumiViz. Leg sie irgendwohin, wo du sie wiederfindest; der Import Browser
öffnet jeden Ordner.

Willst du bei null anfangen statt umzubauen:
**File → New Effect Chain** (`Ctrl+Shift+N`).

---

## Und jetzt?

- **Verstehen, was du da tust:** [Preset-Anleitung](Preset_Anleitung.md)
- **Alle Bedienelemente:** [Benutzerhandbuch](Benutzerhandbuch.md), Kapitel 11
- **Eigene Shader schreiben:** die Tutorial-Serie unter `docs/tutorials/`

Ein Rat zum Schluss: **fang mit einem fremden Preset an und bau es um.** Ein
leeres Blatt ist bei Visuals ungnädig — ein laufendes Preset, dem du Stück für
Stück Dinge wegnimmst und hinzufügst, bringt dich in einer halben Stunde
weiter als eine leere Kette in einem ganzen Abend.
