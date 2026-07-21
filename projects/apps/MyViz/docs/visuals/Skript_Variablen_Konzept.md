# MyViz — Skript-Variablen, Highlighting & Variable-Set-Modul (Konzept)

> **Version:** 0.1.0 · **Datum:** 2026-07-21 (Session 37) · **Typ:** Konzept/Entwurf
> **Status:** ENTWURF — zur Freigabe. Noch nichts davon implementiert (außer der
> korrigierten Funktions-/Konstanten-Liste im bestehenden Highlighter).
> **Bezug:** `scripting/LuaScriptEngine`, `EelTranspiler`, MultiEffectPanel-Editor

---

## 1. Zweck

Präzise Konzeption von vier ineinandergreifenden Dingen:
1. **Variablen-Modell** — welche Variablen es je Modul/Slot gibt, in welcher **Kategorie**.
2. **Syntax-Highlighting nach Kategorie** — read-only / input / output / in-out /
   Konstanten / Custom-Globals / Custom-Locals unterschiedlich einfärben.
3. **Fehler-Markierung** — illegales oder redundantes Schreiben rot markieren.
4. **Variable-Set-Modul** — benannte Globals in den `per…`-Feldern setzen/ändern.

---

## 2. Verifizierter IST-Stand (Laufzeit, aus dem Quellcode)

**Funktionen (nutzbar, aus `buildSandbox` + EelTranspiler):**
`sin cos tan asin acos atan atan2 sqrt sqr invsqrt pow exp log abs sign floor
ceil mod min max rand sigmoid above below equal band bor bnot` · Puffer:
`megabuf(i) gmegabuf(i)` · Steuerung: `if(c,a,b) loop while`.

**Konstanten:** `pi` (3.14159), `pi2` (6.28319). ⚠ `phi`/`e` kennt der Transpiler
syntaktisch, die Sandbox definiert sie **nicht** → nil/Fehler. `getspec/getosc/
gettime` ebenso: vom Transpiler erkannt, **nicht** von der Engine gebunden →
aktuell **nicht verfügbar** (siehe §7 Zukunft).

**Speicher-/Global-Modell (aus `ScriptContext`/`ScriptSlotHost`):**
- `reg00`…`reg99` — **preset-global** (über alle Module/Slots geteilt, vom Host
  an Slot-Grenzen synchronisiert).
- `q1`…`q64` — preset-global Snapshots (MD3-Superset).
- `gmegabuf(i)` — preset-globaler Scratch-Puffer.
- `megabuf(i)` — engine-lokal (= modul-lokal).
- benannte Bezeichner (z. B. `myphase`) — **modul-lokal** (leben in der Engine-Env
  eines Moduls über dessen init/frame/beat/point, aber nicht modulübergreifend).
- `app.gget/gset(i)` — 32 app-globale Atomic-Slots (modul- UND preset-übergreifend).

**Audio (aktuell):** die Werte `bass mid treble vol beat time` werden **nicht** von
der Engine bereitgestellt, sondern **je Modul vom Host injiziert** (Fraktale:
`computeAudioBands`; SuperScope: `v` = Waveform-Sample, `b` = Beat). Es gibt
**noch kein** einheitliches, engine-weites Audio-Variablen-Set.

---

## 3. Variablen-Kategorien (Modell)

Jeder Bezeichner in einem Slot fällt in **genau eine** Kategorie. Die Kategorie
hängt vom **Modul** und **Slot** ab (kontextabhängig — der Editor kennt das aktive
Modul, siehe §5).

| Kategorie | Semantik | Beispiele | Schreibbar? |
|---|---|---|---|
| **read-only system** | Host setzt, Skript liest, Schreiben zwecklos | `w h dt time n(?) i v` | nein (Schreiben = Warnung) |
| **input** | Host setzt pro Frame/Point, Skript liest | `bass mid treble vol beat b` | Lesen |
| **output** | Skript setzt, Host liest zurück | `x y red green blue skip cx cy zoom …` | ja (Sinn nur beim Setzen) |
| **in/out** | beides — persistenter Zustand oder getriebener Param | `t` (Akkumulator), Modul-Params (`power`, `warp`, …) | ja |
| **constant** | fest | `pi pi2` | nein |
| **custom global** | user-definiert, **preset-übergreifend** | `reg00…reg99`, `gmegabuf`, (neu) benannte Globals | ja |
| **custom local** | user-definiert, **modul-lokal** | jeder sonstige Name (`myphase`, `tmp`) | ja |

**Wichtig:** dieselbe Variable kann je **Slot** die Kategorie wechseln — z. B. `n`
ist in `frame` in/out (Punktzahl änderbar), in `point` read-only. Das Modell ist
also eine Tabelle **(Modul × Slot × Name → Kategorie + Typ + Range + Beschreibung)**.
Diese Tabelle ist die **SSOT** — sie speist Referenz (§Editor), Highlighting und
Fehler-Markierung gemeinsam.

---

## 4. Highlighting nach Kategorie

Der bisherige Regex-Highlighter (Funktionen/Zahlen/Kommentare) wird um eine
**modul-bewusste Variablen-Ebene** erweitert: der Highlighter bekommt die
Symboltabelle des **aktiven Moduls+Slots** und färbt bekannte Bezeichner je
Kategorie. Vorschlag (Dark-Theme, an VS-Code angelehnt):

| Kategorie | Farbe (Vorschlag) | Stil |
|---|---|---|
| Funktion | `#4FC1FF` (hellblau) | — |
| Konstante | `#4FC1FF` (hellblau) | kursiv |
| read-only system | `#9CDCFE` (blassblau) | — |
| input | `#4EC9B0` (türkis) | — |
| output | `#DCDCAA` (gelb) | — |
| in/out | `#C586C0` (violett) | — |
| custom global | `#D7BA7D` (gold) | — |
| custom local | `#D4D4D4` (neutral) | — |
| Zahl | `#B5CEA8` (grün) | — |
| Kommentar `//` | `#6A9955` (grün) | kursiv |
| **Fehler** | Unterringelung **rot** `#F44747` | siehe §6 |

**„rot, wenn das noch frei ist":** Rot ist bewusst **nur** für Fehler reserviert
(keine reguläre Kategorie nutzt Rot) — die Fehler-Markierung (§6) hat Rot damit
exklusiv frei.

---

## 5. Kontext-Abruf im Editor (schon teils gebaut)

Bereits umgesetzt (Session 37): der **ⓘ-Button** je Code-Feld zeigt die Referenz
für das **aktive Modul** (`scriptReferenceHtml`), der **⤢-Button** öffnet den
großen Editor mit Referenz daneben. **Erweiterung:** die Symboltabelle (§3) wird
zur SSOT; `scriptReferenceHtml` und der Highlighter (§4) ziehen aus **derselben**
Tabelle (heute ist die Referenz noch hartkodierter HTML-Text pro Modul).

---

## 6. Fehler-Markierung (Regeln)

1. **Unbekannte System-Variable geschrieben** (Name ist keine bekannte
   output/in-out des Moduls **und** kein deklarierter Custom) → **rot** (wahrsch.
   Tippfehler). Konfigurierbar: hart (Fehler) vs. weich (nur Custom-Local anlegen).
2. **Schreiben auf read-only/constant** (`w`, `pi`, …) → **rot** (Warnung).
3. **Init-Regel für Custom-Globals:** wird in `init` eine **bereits anderswo
   initialisierte** globale Variable erneut geschrieben → **rot** (Doppel-Init /
   Konflikt). Erfordert einen **preset-weiten Global-Deklarations-Tracker**: `init`
   = Deklaration (Erst-Schreiben), zweites `init`-Schreiben derselben Global =
   Konflikt.
4. **Lesen einer nie initialisierten Custom-Global** → **gelbe** Warnung (nicht rot).

Die Regeln 1–2 sind rein statisch (Symboltabelle). Regeln 3–4 brauchen den
**preset-weiten Tracker** (§8, Umsetzungsschritt).

---

## 7. Variable-Set-Modul

**Zweck:** ein Modul ohne visuelle Ausgabe, das in den üblichen Slots
(`init/frame/beat`) **benannte globale Variablen setzt/modifiziert**, die andere
Module lesen können — wie „Global Variables" (Jheriko), aber mit **benannten**
Globals statt nur `reg00…99`.

**Heutiger Stand:** „Global Variables" (`JherikoGlobalParams`) macht das bereits
über `reg/gmegabuf`. Zwei Optionen:

- **(A) minimal:** „Global Variables" als das offizielle Variable-Set-Modul
  dokumentieren + im Editor mit dem Global-Tracker (§6.3) verdrahten. Kein neues
  Modul.
- **(B) neu:** eigenes **`VariableSet`-Modul** mit benannten Globals. Da EEL/Lua
  keine echten benannten Cross-Modul-Globals kennt (nur `reg/gmegabuf`), müsste
  der Host **Namen → reg/gmegabuf-Slots** mappen (Alias-Tabelle je Preset) oder das
  ScriptContext um eine **benannte Global-Map** erweitern. Mehr Aufwand, aber
  lesbarer (`speed` statt `reg07`).

**Empfehlung:** (A) jetzt (schnell, deckt den Bedarf), (B) als spätere Kür, wenn
benannte Globals wirklich gewünscht sind.

---

## 8. Umsetzungsschritte

1. **Symboltabelle (SSOT)** — ✅ **Session 37** (Teil): globale Kategorie-Klassifikation
   `symbolCategory()` (SymCat: ReadOnly/Input/Output/InOut/Constant/CustomGlobal).
   *Vereinfachung ggü. §3:* aktuell **modul-unabhängige** Namensklassifikation (nicht
   Modul×Slot) — die präzise Semantik lebt in der ⓘ-Referenz. Modul×Slot-Verfeinerung
   offen.
2. **Referenz** — die ⓘ-Referenz bleibt vorerst pro-Modul-HTML (akkurat); Generierung
   aus der Tabelle offen.
3. **Kategorie-Highlighter** — ✅ **Session 37**: `EelHighlighter` färbt Variablen nach
   Kategorie (§4-Farben) + Legende in der Referenz.
4. **Statische Fehler-Markierung** — ✅ **Session 37** (Regel 6.1–6.2, konservativ):
   Schreiben auf read-only (`w/h/dt`) oder Konstante (`pi/pi2`) → **rote Wellenlinie**.
   Bewusst kleine ReadOnly/Constant-Menge → keine Falsch-Positiven.
5. **Preset-weiter Global-Tracker** (Regeln 6.3–6.4) + **Variable-Set (A)** —
   ✅ **Session 37**: der Editor scannt beim Öffnen eines Nodes alle **anderen** Nodes'
   init auf Global-Writes (`nodeInitCode` generisch + `collectInitGlobalsExcept`);
   schreibt der aktuelle **init** eine Global (`regNN`/`qN`), die auch woanders im
   init deklariert wird → **rote Wellenlinie**. „Global Variables" ist offiziell das
   Variable-Set-Modul (Editor-Hinweis + im Tracker erfasst).
6. *(später)* Modul×Slot-genaue Kategorien; benannte Globals (§7B).

---

## 9. Audio-Analyse (Stand Session 37: ✅ implementiert)

- **✅ `getspec(band, width, ch)` / `getosc(band, width, ch)` / `gettime(sc)`** sind
  jetzt **real in der Engine gebunden** (AVS-treuer `getvis`-Port aus
  `avs_eelif.cpp`) und werden an **ALLE scripted Module** gespeist (SuperScope,
  alle 9 Fraktale, DDM, DShift, Bump, Global Vars, Texer II, Triangle, Effect List,
  Movement, Dynamic Movement, Color Modifier). VisData-Layout: Spektrum L/R +
  Waveform L/R (je 576 Byte), 1× pro Frame in `buildVisData` gebaut. Damit
  importieren AVS-Presets, die auf `getspec/getosc` bauen, **korrekt**.
- **✅ Einheitliches Input-Set (E1):** `bass mid treb (+treble) vol beat time` auf
  allen ScriptSlotHost-Modulen via `feedAudio`. *(Anm.: die drei Modul-gewrappten
  — SuperScope/Movement/Color Modifier — bekommen VisData/getspec; bass/mid/treb
  dort per `setVariable` nachrüstbar, falls gewünscht.)*
- **✅ Echtes L/R-Stereo (Session 37, SICHTTEST/HÖRTEST offen):** additive
  Stereo-Pipeline — `BassEngine::getFFTDataStereo` (BASS_DATA_FFT_INDIVIDUAL) +
  `getStreamChannels`; MainWindow holt Individual-FFT + interleaved Waveform,
  reicht sie (interleaved) via `VisualizerWidget/RenderThread::updateAudioStereo`
  → `VisualizerBase::updateAudioStereo` (de-interleave nach L/R) durch; `buildVisData`
  füllt Spektrum/Waveform L/R getrennt. **Mono-Pfad bleibt Fallback** (kein Regress,
  falls Stereo fehlt). ⚠ **Nur compile-verifiziert** — die FFT_INDIVIDUAL-Layout-
  Annahme (`bin*chans+ch`) + Waveform-Interleaving brauchen Patriks **Hörtest**;
  bei Fehldarstellung ist das Layout die Stellschraube.
- **Später (Entscheid offen, „wenn sinnvoll"):** `bass_att/mid_att/treb_att`
  (attenuiert), `peak`, `rms`, `fps`, `frame`.

---

## 10. Entscheide (Patrik, 2026-07-21)

- **E1 Audio-Input-Set vereinheitlichen:** ✅ **JA** — ab jetzt bekommen **alle**
  scripted Module `bass/mid/treble/vol/beat/time`.
- **E2 Variable-Set-Modul:** ✅ **(A)** — „Global Variables" offiziell + Tracker
  (nicht-brechend; `reg/gmegabuf` sind bereits preset-global). (B) benannte Globals
  = spätere Kür.
- **E3 Fehler-Härtegrad:** Vorschlag: unbekannter Name **rot markieren, aber
  lauffähig** lassen (nicht hart abbrechen). *(noch zu bestätigen)*
- **E4 Umfang jetzt:** ✅ **Schritte 1–5** — SSOT + Referenz + Kategorie-Highlighting
  **+ Fehler-Markierung + preset-weiter Global-Tracker**.

> **Wartet auf:** zusätzliche Eingaben von Patrik (bereits getippt, werden vor dem
> Implementierungsstart gelesen).
