# Werkzeug-Wegleitung — welches Programm wofür, und welche Fallen es hat

> **Version:** 1.2.0
> **Datum:** 2026-08-09 (Session 73; 1.1.0: Session 74 — echte Musik als
> Audioquelle, Falle 2.7; 1.2.0: Session 74 — das synthetische Signal hat zwei
> Muster und kommt aus EINER Quelle)
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
2. [Die sieben Fallen](#2-die-sieben-fallen)
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

**Das Prüfsignal ist gemeinsam.** Alle vier Werkzeuge erzeugen ihren
synthetischen Klang aus derselben Datei
(`projects/exec/common/SynthAudio.hpp`). Bis S74 stand die Formel viermal im
Baum, jedes Mal als Kopie mit dem Kommentar „formelgleich zu …" — genau die
Sorte Abmachung, die irgendwann still auseinanderläuft. Wer am Prüfsignal
etwas ändert, ändert es jetzt für alle vier auf einmal.

---

## 2. Die sieben Fallen

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

### 2.5 🟠 `MilkdropRef` braucht die MilkDrop-Ordnerstruktur

Der Original-Kern sucht seine Datendatei unter **`<presetordner>/../data/include.fx`**
— eine Ebene über dem Preset-Ordner. Zeigt man ihn auf einen beliebigen Ordner
mit `.milk`-Dateien, bricht er ab:

```
FEHLER: PluginInitialize
```

und im Fenster steht „Unable to read the data file". Der genannte Pfad im
Dialog verrät, wo er gesucht hat.

**Regel: Presets für einen Referenzlauf immer unter einer Wurzel ablegen, die
`data/` enthält** — also `asset/Milkdrop3/` oder eine Nachbildung davon:

```
<wurzel>/data/include.fx      (aus asset/Milkdrop3/data/)
<wurzel>/textures/            (nur nötig, wenn Presets Texturen nutzen)
<wurzel>/presets/*.milk       (hierauf zeigt der Aufruf)
```

Das betrifft **nur** MilkdropRef. LumiViz selbst braucht weder `data/` noch
`textures/`, solange ein Preset keine Textur anfordert — die mitgelieferten
19 tun das nicht.

### 2.6 🟡 `--auto` braucht ein offenes Fenster

Die Standalones rendern im GUI-Thread (`paintGL`) — dieselben Methoden, die in
der App der Render-Thread aufruft. Das Fenster muss offen bleiben, bis der Lauf
durch ist. Während eines schweren Presets reagiert es nicht.

**Regel: Fenster stehen lassen.** Der Lauf endet von selbst und meldet
`--auto abgeschlossen (N Presets), Ende.`

### 2.7 🔴 Echte Musik macht jeden Referenzvergleich wertlos

Seit S74 können beide Standalones eine Audiodatei abspielen
(`--audio-datei`, s. Rezept unten). Das ist für „sieht das gut aus" ein
Gewinn — und für Treue-Messungen ein Fallstrick.

`AvsRef` und `MilkdropRef` erzeugen ihr Audio **selbst**, formelgleich zum
synthetischen Signal der Standalones. Genau darauf beruht die
Vergleichbarkeit. Hört unsere Seite echte Musik und die Referenz weiter ihren
Sinus, vergleicht man zwei verschiedene Eingaben — die Zahlen sehen dabei
genauso seriös aus wie echte.

**Regel: `--audio-datei` ist für Schaufenster und Augenschein.
Referenzläufe bleiben beim synthetischen Signal.**

Wer für Vergleiche ein **kräftigeres** Signal braucht, nimmt nicht die
Audiodatei, sondern `--audio-muster musik` (Rezept unten): dieselbe Dynamik,
aber neu synthetisiert und auf der Referenz-Seite identisch erzeugbar.

Abgesichert an drei Stellen:

- Der Standalone schreibt bei jedem Lauf mit Audiodatei eine
  `[Audio] ACHTUNG`-Zeile in die Ausgabe.
- `compare_avsref.py` verweigert den Dienst, wenn `--audio-datei` durchgereicht
  wird (`run_lumi`).
- Hörbar wird die Datei nur im **interaktiven** Lauf; `--auto` und `--ab`
  bleiben still und deterministisch.

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

### Kräftigeres Prüfsignal, ohne die Vergleichbarkeit zu verlieren

Das klassische Prüfsignal ist ein 220-Hz-Sinus mit Beat-Puls — für viele
Presets zu wenig, um überhaupt etwas zu zeigen. Seit S74 gibt es ein zweites
Muster, das aus einer echten Aufnahme abgeleitet ist:

```bash
AvsStandalone "<preset.avs>" --auto --frames 120 --audio-muster musik --out "<zielordner>"
```

`--audio-muster musik` gibt es an **allen vier** Werkzeugen — beiden
Standalones **und** `AvsRef`/`MilkdropRef`. Alle vier binden dieselbe
`projects/exec/common/SynthAudio.hpp` ein, das Signal ist also auf beiden
Seiten identisch erzeugbar. **Referenzvergleiche bleiben damit gültig**, anders
als bei `--audio-datei` (Falle 2.7).

Auch der Vergleichs-Harness kennt es:

```bash
python asset/calibration/avs/compare_avsref.py --audio-muster musik
```

Was das Muster enthält: acht log-verteilte Bandhüllkurven je Bild plus eine
Beat-Spur, gewonnen aus einem Ausschnitt der Vorlage
(`projects/exec/common/MusikProfil.hpp`, erzeugt). Daraus wird ein Klang **neu
synthetisiert** — acht Sinus mit musikalisch bewegten Amplituden. Es wird keine
Musik abgespielt, und aus dem Profil lässt sich die Aufnahme nicht
zurückrechnen.

Neu erzeugen (anderer Titel, andere Stelle, andere Länge):

```bash
AvsStandalone --audio-datei "<musik.mp3>" --audio-start 45 --audio-profil-dauer 20 --audio-profil-schreiben projects/exec/common/MusikProfil.hpp
```

Danach **alle vier Werkzeuge neu bauen** — das Profil ist einkompiliert.

> **`klassisch` bleibt die Vorgabe und ist bit-identisch zu vor S74.** An
> diesem Signal hängen die Modul-Matrix, die Modul-Sonden und alle
> Feld-Sonden; ein Wechsel der Vorgabe würde alles neu einmessen bedeuten.

### Presets an echter Musik ansehen (Schaufenster)

Beide Standalones nehmen seit S74 eine Audiodatei statt des synthetischen
Sinus. Interaktiv läuft sie zusätzlich **hörbar** mit:

```bash
MilkdropStandalone "<preset.milk>" --size 1280x720 --audio-datei "<musik.mp3>" --audio-start 45
```

| Schalter | Wirkung |
|---|---|
| `--audio-datei PFAD` | MP3/WAV/FLAC/… statt Sinus; alles, was Qt Multimedia dekodiert |
| `--audio-start SEK` | Startversatz — trifft den Refrain statt der Einleitung |
| `--audio-gain F` | Faktor auf Wellenform und Spektrum; echte Musik ist deutlich leiser als das synthetische Signal (dort ~0,8 Spitze im Spektrum) |
| `--audio-stumm` | nur das Bild füttern, nichts hören |

Abgetastet wird nach **Bild-Index** (`t = Bild/60`), nicht nach Echtzeit — zwei
Läufe mit derselben Datei und derselben Bildzahl sind bitgleich. Die Datei wird
beim Start einmal komplett dekodiert (vier Minuten ≈ 84 MB, ~2 s).

⚠️ Nicht für Referenzvergleiche — siehe Falle 2.7.

### Treue gegen das Original prüfen

Nicht die Standalones vergleichen, sondern gegen die Referenz-Renderer:
`tools/AvsRef` bzw. `tools/MilkdropRef`. Das Vorgehen samt Sondenformen,
Bisektion und Verifikationsgürtel steht in
[AVS-Kalibrier-Methodik](visuals/AVS_Kalibrier_Methodik.md).

**Wichtig:** In zwei Größen vergleichen. Eine kleine Fläche (320×240) versteckt
größenabhängige Fehler — genau die Klasse Fehler, die auch Falle 2.2 erzeugt.
