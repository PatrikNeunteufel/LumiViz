# Werkzeug-Wegleitung — welches Programm wofür, und welche Fallen es hat

> **Version:** 1.0.0
> **Datum:** 2026-08-09 (Session 73)
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch
> **Zweck:** Verhindern, dass Prüfläufe mit dem falschen Werkzeug, der falschen
> Build-Konfiguration oder falschen Schaltern gemacht werden — und dann falsche
> Schlüsse gezogen werden.

Dieses Dokument ist aus Fehlern entstanden. Jede Falle unten hat mindestens
einmal echte Arbeitszeit gekostet, mehrere davon an einem einzigen Nachmittag.

## Inhalt

1. [Welches Werkzeug wofür](#1-welches-werkzeug-wofür)
2. [Die fünf Fallen](#2-die-fünf-fallen)
3. [Rezepte](#3-rezepte)

---

## 1. Welches Werkzeug wofür

| Werkzeug | Wofür | Wofür **nicht** |
|---|---|---|
| **LumiViz** (die App) | Alles, was ein Nutzer sieht. **Die Referenz** — nur sie wendet alle Einstellungen an | Stapelläufe, deterministische Vergleiche |
| **AvsStandalone** | `.avs`, `.lvfx`, `.milk` im Multieffekt-Host stapelweise rendern, Screenshots, Import-Report, Ketten-Bisektion | Ein Bild „wie in der App" — nur mit `--render-scale`, s. u. |
| **MilkdropStandalone** | `.milk` im Milkdrop-Knoten, Preset-Wechsel-Wächter (`--ab`), Frame-Hashes | AVS-Presets |
| **tools/AvsRef** | Der **originale** Winamp-AVS-Renderer als Vergleichsmaßstab | Alles, was LumiViz eigen ist |
| **tools/MilkdropRef** | Der **originale** MilkDrop3-Renderer als Vergleichsmaßstab | s. o. |

**Merksatz:** Die Standalones stellen die App *nach*. Sie sind nicht die App —
was die App aus Einstellungen zieht, müssen sie als Schalter bekommen.

---

## 2. Die fünf Fallen

### 2.1 🔴 Debug-Build statt Release — Faktor 20

Gemessen an zehn AVS-Presets, je 90 Frames, 1280×720, RTX 4090:

| Build | Dauer |
|---|---|
| Debug (MSVC) | **306 s** |
| Release (MSVC) | **15 s** |

Im Debug-Build wirkt der Standalone wie eingefroren — das Bild wechselt nur
alle paar Sekunden. Das ist **kein Hänger**, sondern ein normaler Debug-Build.
Wer ihn für hängend hält und das Fenster schließt, bricht den Lauf ab.

**Regel: Stapelläufe immer im Release-Build.** Debug nur, wenn wirklich ein
Debugger dranhängt.

```bash
cmake --build --preset build-vs-x64-Release --target AvsStandalone
```

### 2.2 🔴 Der Render-Scale-Divisor fehlt im Standalone

Die App legt beim AVS-Import einen **Render-Scale-Knoten** an, mit dem Divisor
aus *Einstellungen › Panels › AVS Import Render Scale*
(`import/avsRenderScaleDivisor`). Klassische Winamp-Presets rechnen mit festen
Pixelgrößen; ohne diesen Knoten zerfällt das Bild bei großen Fenstern in
Streifen und Balken.

**`AvsStandalone` kannte den Divisor bis S73 überhaupt nicht** und rendert per
Vorgabe ungeskaliert. Bei kleinen Fenstern fällt das nicht auf — genau deshalb
blieb die Lücke so lange unbemerkt.

**Regel: Wer das Bild der App nachstellen will, muss denselben Divisor setzen.**

```bash
AvsStandalone "asset/presets/avs/EyeCandy2" --auto --size 1280x720 --render-scale 4
```

Welcher Wert gilt, steht in der App unter *Einstellungen › Panels*. Steht dort
4, muss hier `--render-scale 4` stehen — sonst vergleicht man Äpfel mit Birnen.

### 2.3 🟠 Die GPU-Vorgabe hängt am EXE-Pfad

Windows merkt sich die Grafikkarten-Wahl in
`HKCU\Software\Microsoft\DirectX\UserGpuPreferences` — als Eintrag **je
EXE-Pfad**. Jeder neue Build-Ordner, jede Umbenennung und jede neue
Konfiguration hat damit **keinen** Eintrag und landet auf der integrierten
Karte.

Erkennbar an der Startzeile des Standalones:

```
[Standalone] GL initialisiert: ... | AMD Radeon(TM) 610M        ← integriert, langsam
[Standalone] GL initialisiert: ... | NVIDIA GeForce RTX 4090    ← richtig
```

**Regel: Vor einem Messlauf die GL-Zeile lesen.** Stimmt die Karte nicht,
Eintrag setzen (`GpuPreference=2` = Hochleistung) und neu starten. Die
App-eigene Verwaltung dafür steht in `core/GpuPreference`.

### 2.4 🟠 Pfade mit Leerzeichen

Ordner wie `fuck me im famous` zerfallen in vier Argumente, wenn sie nicht
gequotet werden. Der Standalone meldet das brauchbar:

```
FEHLER: keine .milk-Presets unter 'asset/presets/milkdrop/fuck' 'me' 'im' 'famous'
```

**Regel: Pfade immer quoten.** In PowerShell reicht ein Array-Element nicht —
die Anführungszeichen müssen **im String** stehen:

```powershell
Start-Process $exe -ArgumentList @('"asset/presets/milkdrop/fuck me im famous"','--auto')
```

### 2.5 🟡 `--auto` braucht ein offenes Fenster

Die Standalones rendern im GUI-Thread (`paintGL`) — dieselben Methoden, die in
der App der Render-Thread aufruft. Das Fenster muss offen bleiben, bis der Lauf
durch ist. Während eines schweren Presets reagiert es nicht.

**Regel: Fenster stehen lassen.** Der Lauf endet von selbst und meldet
`--auto abgeschlossen (N Presets), Ende.`

---

## 3. Rezepte

### Screenshots einer Preset-Sammlung (AVS)

```bash
cmake --build --preset build-vs-x64-Release --target AvsStandalone
```

Dann — GL-Zeile prüfen, Divisor wie in der App, Pfade gequotet:

```bash
AvsStandalone "<ordner>" --auto --frames 90 --size 1280x720 --render-scale 4 --out "<zielordner>"
```

Der Lauf schreibt je Preset ein PNG plus Pixel-Statistik (`mean RGB`, `Luma
min/max`) und meldet `schwarz=ja|nein`. Exit-Code 0 nur, wenn **alle** Presets
geladen haben.

### Dasselbe für MilkDrop

```bash
MilkdropStandalone "<ordner>" --auto --frames 90 --size 1280x720 --out "<zielordner>"
```

MilkDrop-Presets sind gitterbasiert und damit auflösungsunabhängig — hier gibt
es **keinen** Render-Scale-Divisor.

### Treue gegen das Original prüfen

Nicht die Standalones vergleichen, sondern gegen die Referenz-Renderer:
`tools/AvsRef` bzw. `tools/MilkdropRef`. Das Vorgehen samt Sondenformen,
Bisektion und Verifikationsgürtel steht in
[AVS-Kalibrier-Methodik](visuals/AVS_Kalibrier_Methodik.md).

**Wichtig:** In zwei Größen vergleichen. Eine kleine Fläche (320×240) versteckt
größenabhängige Fehler — genau die Klasse Fehler, die auch Falle 2.2 erzeugt.
