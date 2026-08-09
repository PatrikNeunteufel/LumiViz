# Vereinheitlichungs-Konzept — Skript-Schicht, Module, Gradients

> **Version:** 1.2.0
> **Datum:** 2026-07-24
> **Typ:** Konzept
> **Status:** Entwurf (zur Freigabe)
> **Sprache:** Deutsch
> **Bezug:** [Import-Fundament-Entwurf](Import_Fundament_Entwurf.md) ·
> [MilkDrop-Import-Konzept](MilkDrop_Import_Konzept.md) ·
> [MilkDrop-Import-Status](MilkDrop_Import_Status.md) (SSOT) ·
> [Config-Pipeline-Konzept](Config_Pipeline_Concept.md)

---

## 1. Ziel und Einordnung

Vorbereitung für den späteren **Meganode-Split** (Revision von Entscheid E1,
Session 41) und Abbau der historisch gewachsenen Doppelgleisigkeit zwischen
AVS-Welt und MilkDrop-Welt. Am Ende gilt: **ein einheitliches Skript-Set**
(Variablen, Funktionen, Konstanten) für alle skriptfähigen Stellen — AVS-Nodes,
Milkdrop-Node(s) und LumiViz-eigene Module. Importe beider Formate bleiben
dabei **semantisch exakt** (Treue-Grundsatz).

Vereinbarte Reihenfolge (Diskussion Session 44):

1. **Kalibrier-/Treue-Runde** (läuft — AVS: d/r-Pixel-Raum, SRM-Reset,
   schwarze Korpus-Presets; Milk: Sichttest mit Musik). Erst richtige Werte,
   dann geteilte Werte.
2. **Skript-Vereinheitlichung** (dieses Konzept, §3–§6).
3. **Standalones → Module** (§7) — zugleich Generalprobe für den Split.
4. **Gradient-Einbindung** als regulärer Parameter-Typ (§8).
5. Danach erst: Meganode-Split (eigenes Konzept, nicht Teil dieses Dokuments).

## 2. Ist-Stand (Bestandsaufnahme 2026-07-24)

### 2.1 Zwei Audio-Welten

| | AVS-Seite | MilkDrop-Seite |
|---|---|---|
| Band-Werte | `computeAudioBands()` (`MultiEffectVisualizer.cpp:5580`): Drittel-Mittelwerte des Spektrums, unnormalisiert, ohne Glättung | `MilkLoudness` (`modules/processing/MilkLoudness.hpp`): bpm-Port, normalisiert (~1.0 = Durchschnitt), mit `_att`-Hüllkurven |
| Injektion | `feedAudio()` (`MultiEffectVisualizer.cpp:5683`): `bass/mid/treb/treble/vol/beat/time` — aber nur in **manche** Hosts (Bump, Shift, Texer, Triangle, Jheriko, DDM, Fractal3D); Superscope-Hosts bekommen nur `b/w/h/…` | `pushCommonInputs()` (`MilkdropVisualizer.cpp:868`): `time/fps/frame/progress/bass/mid/treb/treble/bass_att/mid_att/treb_att/vol/meshx/meshy/pixelsx/pixelsy/aspectx/aspecty` |
| Spektrum/Wellenform-Zugriff | `getspec`/`getosc`/`gettime` (LuaScriptEngine, `visdata`-Puffer 4×576 im AVS-Byte-Format, `updateVisData()`) | **fehlt** — Kanal-Getter (`getSpectrumChannel`, seit S43) existieren, sind aber nicht als Skript-Funktionen exponiert |
| `vol`-Semantik | `m_audioLevel` (Gesamt-Pegel) | Mittel aus bass/mid/treb |

Drei Probleme: **(P1)** doppelte Band-Berechnung mit unterschiedlicher
Semantik, **(P2)** uneinheitliche Injektion je Host, **(P3)** `getspec`/
`getosc` fehlen Milk-seitig, `bass_att` & Co. fehlen AVS-seitig.

### 2.2 Funktionen

Beide Welten laufen auf **derselben** `LuaScriptEngine` (+ EelTranspiler +
Prelude) — das Funktions-Set ist faktisch schon gemeinsam:
math-Whitelist (`sin cos tan asin acos atan sqrt abs floor ceil exp log min
max fmod`, `mod`=fmod, `atan2`, `huge`; `LuaScriptEngine.cpp:341`), `rand`,
`getspec/getosc/gettime`, `eel.*`-Prelude (truthy/equal/above/below/band/bor/
bnot/bitand/bitor/mod/toint/sqrt/invsqrt/sigmoid/sign/rand/mbread/mbwrite/
gmbread/gmbwrite), `app.gget/gset`. Unterschiede liegen nur bei den
**Host-Inputs** (Variablen), nicht bei den Funktionen.

### 2.3 Konstanten

- **Skript:** `$pi`/`$e`/`$phi` im EelTranspiler-Lexer (textueller Wert,
  `EelTranspilerLexer.hpp:255`) = EEL-Standardumfang; Engine-Globals `pi`,
  `pi2` für handgeschriebene Lua-Skripte.
- **Shader:** include.fx-Nachbildung `M_PI`, `M_PI_2` (**= 2π!**),
  `M_INV_PI_2` (HlslTranspiler).

## 3. Zielbild: das Lumi-Skript-Set (v1)

**Grundsatz:** EIN Namensraum, überall aktiv — kein Runtime-Profil.
Kollisionen mit Preset-eigenen Bezeichnern löst der **Import** per
Umbenennung (§4). Entscheid Session 44.

### 3.1 Audio-Variablen (Quelle: MilkLoudness = SSOT)

| Name | Bedeutung |
|---|---|
| `bass` / `mid` / `treb` (+ Alias `treble`) | normalisierte Band-Loudness (~1.0 = Durchschnitt) |
| `bass_att` / `mid_att` / `treb_att` | attenuierte (zeitlich geglättete) Hüllkurven derselben Bänder |
| `vol` / `vol_att` | Gesamt-Loudness = Mittel der drei Bänder (bzw. `_att`) — D1 ✅ |
| `rms` | geglätteter Wellenform-RMS 0..1 — der heutige `m_audioLevel`; der bisherige AVS-seitige `vol`-Wert wandert auf diesen Key (D1c ✅) |
| `beat` | Frame-Beat des BeatEstimators (0/1) |

`computeAudioBands()` **entfällt** (P1 gelöst). Die MilkDrop-internen
`imm`-Rohwerte (unnormalisierte Band-Energien, heute nur im
`loudness:`-Trace) bleiben **intern** — Presets haben sie im Original nie
gesehen.

### 3.2 Audio-Funktionen (überall verfügbar)

`getspec(band, bandw, ch)`, `getosc(band, bandw, ch)`, `gettime(start)` —
Milk-seitig wird dazu der `visdata`-Puffer aus den Kanal-Gettern gefüttert;
der Aufbau-Code (`updateVisData()`, AVS-Byte-Format 4×576) wird als
gemeinsamer Baustein herausgezogen (ein Aufbau pro Frame, alle Hosts lesen
denselben Puffer).

### 3.3 Zeit / Geometrie (überall identisch)

`time` (Sekunden), `frame`, `fps`, `progress`, `dt` (Frame-Delta in
Sekunden, aus demselben Frame-Snapshot — D4 ✅);
`w`/`h` = Render-Zielgröße in Pixeln (AVS-Konvention) mit Aliassen
`pixelsx`/`pixelsy` (Milk-Konvention), `aspectx`/`aspecty`.
`meshx`/`meshy` bleiben milk-spezifisch (Mesh-Auflösung existiert nur dort).
Domänen-Inputs (Superscope `n/i/v/x/y/red/…`, Movement `d/r/x/y`, …) bleiben
Sache des jeweiligen Moduls — wie bisher (ScriptSlotHost.md §2).

### 3.4 Funktionen

Bestand aus §2.2 bleibt; bei Umsetzung ein **EEL2-Referenz-Abgleich**
(WDL/ns-eel-Funktionsliste gegen unsere Whitelist, z. B. `log10`, `pow`
als Funktion) — Fehlendes ergänzen, Goldens erweitern.

### 3.5 Konstanten (zweistufig)

- **Original-Set (unantastbar, Treue):** Skript `$pi`/`$e`/`$phi`; Shader
  `M_PI`/`M_PI_2`(=2π)/`M_INV_PI_2`.
- **Lumi-Erweiterungen (D3 ✅):** Skript `$tau` (2π), `$sqrt2`, `$invpi`,
  `$deg2rad` (π/180), `$rad2deg` (180/π); Shader `M_TAU`, `M_E`, `M_SQRT2`.
  Technisch nur Tabelleneinträge (Lexer bzw. Transpiler-Konstantentabelle
  sind generisch); bei Bedarf erweiterbar.

Erweiterungen dürfen Importe nie brechen → Kollisionsregel §4 gilt auch für
Konstanten-Namen (ein Preset-`#define M_E …` gewinnt in seinem Scope).

### 3.6 Unverändert

`reg00–99`, `q1–q64`, `gmegabuf`, `app.gget/gset`, PRNG-Verträge — alle
§10-/E-Entscheide der Import-Phase bleiben bestehen.

## 4. Basis-Key-Registry + Import-Kollisionsregel

**Registry (SSOT-Header, neu):** `include/scripting/ScriptBaseKeys.hpp` —
maschinenlesbare Liste aller reservierten Namen des Lumi-Sets mit Kategorie
(Variable/Funktion/Konstante) und Herkunft (avs/milk/lumi). Verbraucher:
beide Importer, die Injektions-Schicht (§6), der Skript-Editor
(Highlighting/Vervollständigung), Tests.

**Kollisionsregel (Entscheid Session 44):** „Reserviert" wirkt **relativ zum
Import-Format** — entscheidend ist das Herkunfts-Feld der Registry:

- Keys, die im **Quellformat selbst** Builtin-Bedeutung haben, werden **nie**
  umbenannt — sie binden an die Builtins, das IST die Original-Semantik
  (`bass` beim .milk-Import, `getspec` beim .avs-Import).
- Umbenannt wird nur, was im Quellformat **keine** Builtin-Bedeutung hat und
  mit dem Lumi-Set kollidiert: Milk-Herkunfts-Keys beim .avs-Import
  (`bass`, `monitor`, `time` als AVS-Preset-Variable), AVS-Herkunfts-Keys
  beim .milk-Import (Variable `getspec` in einer .milk-Sektion) und
  Lumi-only-Keys (`dt`, Erweiterungs-Konstanten) bei beiden.

Kollidierende Preset-eigene Bezeichner werden **konsistent alpha-umbenannt**
— deterministisch `name` → `name_p`
(bei erneuter Kollision `name_p2`, …; D2 ✅ — Prüfung **case-insensitiv**,
EEL; das `_p`-Suffixmuster ist selbst in der Registry reserviert), einheitlich
über alle Slots des jeweiligen Scopes (AVS: Komponente; Milk:
Preset-Sektion). Das gilt auch für Variablen, die einen **Funktionsnamen**
tragen (Var `getspec` in einem .milk) — Mechanik-Vorbild ist der
`_hl`-Alias des HlslTranspilers.

Eigenschaften:

- **Semantik-erhaltend:** auch der Read-before-write-Fall bleibt exakt —
  eine umbenannte, nie geschriebene Variable startet weiterhin
  uninitialisiert (= 0), statt plötzlich den Builtin-Wert zu liefern.
- **Sichtbar:** jede Umbenennung erscheint als ℹ-Zeile im Import-Report.
- **Kein Runtime-Profil nötig:** das einheitliche Set ist immer aktiv;
  die Komplexität liegt einmalig im Import, nicht dauerhaft in der Engine.

## 5. Semantik-Festlegungen (Kopplung an die Kalibrier-Runde)

- **Eine Band-Quelle:** MilkLoudness liefert für **beide** Welten. Auf der
  AVS-Seite waren `bass/mid/treb` nie Original-AVS (Lumi-Zugabe der
  Effekt-Ports) — der Algorithmuswechsel ist also treue-neutral für
  Importe, ändert aber sichtbar das Verhalten eigener Lumi-Ketten, die die
  bisherigen `feedAudio`-Werte nutzen. Beim Umstellen einmal sichten.
- **`vol`:** Vorschlag Mittel(bass, mid, treb) — deckt sich mit der
  Milk-Referenz; die bisherige AVS-Belegung (`m_audioLevel`) entfällt
  (**Entscheid D1**).
- **`beat`:** einheitlich der Frame-Beat des BeatEstimators (wie heute in
  `feedAudio`); effektlokale Beat-Varianten (`isbeat`/`islbeat` einzelner
  Ports) bleiben Domänen-Inputs.
- **Timing:** `time/dt/frame/fps/progress` aus einer Uhr, ein Snapshot pro
  Frame für alle Hosts (keine Drift zwischen Nodes desselben Frames).

## 6. Gemeinsame Injektions-Schicht

Die verstreuten Pfade (`feedAudio()` × 14 Aufrufstellen, je-Host-Sonderfälle,
`pushCommonInputs()`) werden durch **einen** Baustein ersetzt — Arbeitstitel
`ScriptInputFeeder` (Header neben ScriptSlotHost):

- setzt den **Common-Block** (§3.1–§3.3) in jede Engine — jeder skriptfähige
  Host bekommt alles, nicht nur eine handverlesene Teilmenge (P2 gelöst);
- verwaltet den frame-globalen `visdata`-Puffer (§3.2, P3 gelöst);
- Domänen-Inputs setzt weiterhin der Besitzer (Vertrag ScriptSlotHost.md §2
  unverändert);
- Werte kommen aus einem per-Frame-Snapshot (ein `MilkLoudness`-Lauf, eine
  Zeitbasis) — auch für spätere Stufen-Nodes des Meganode-Splits der
  natürliche Andockpunkt.

## 7. Standalones → Module

Ziel wie beim Milkdrop-Entscheid E2: am Ende registriert die Registry nur
noch den MultiEffect-Host; die Alt-Visualizer werden zu Chain-Modulen.
Zugleich ist jede Portierung eine Generalprobe für den Meganode-Split
(gleiche Fragen: Params-Struct, Compile-Clamps, Serializer, Panel-Sektion,
Revision).

| Kandidat | Stand | Weg |
|---|---|---|
| Superscope | Chain-Modul existiert (`SuperscopeModule`) | Feature-Gleichstand Alt-Visualizer ↔ Modul prüfen, dann Alt-Eintrag deregistrieren |
| Waveform | Modul-Verwandter existiert (`WaveformModule`), aber **Erweiterungen geplant** (Patrik, noch nicht spezifiziert) | **erst Design-Runde** (Mini-Konzept der Erweiterungen), dann portieren — sonst wird der Ist-Stand eingefroren (D5 ✅) |
| Equalizer | nur Standalone | als Chain-Node portieren |
| Oscilloscope | nur Standalone | als Chain-Node portieren |
| Pulsing | nur Standalone | als Chain-Node portieren |

Reihenfolge-Vorschlag: Superscope (billigster Gleichstand) → Equalizer →
Oszi → Pulsing → Waveform (nach Design-Runde).

## 8. Gradient-Einbindung

Heute Insellösungen: Fractal-LUT (`ensureFractalLut`,
`MultiEffectVisualizer.cpp:5609`) und CPU-LUT (`buildCpuGradientLut`) bauen
sich ihre Gradient-Auswertung selbst. Ziel:

- **Gradient als regulärer Parameter-Typ** für Chain-Nodes — Anschluss an
  das bestehende Pipeline-Schema (`GradientHandles`, Config-Pipeline
  Schritt 0–4) statt Preset-String je Effekt;
- ein gemeinsamer **LUT-Baustein** (256er-Textur-Upload + CPU-Tabelle,
  SSOT-Header) für alle Module, die Farbverläufe samplen;
- Panel: der vorhandene Gradient-Editor der Config-Pipeline wird für
  Chain-Nodes wiederverwendet;
- damit sind auch die portierten Standalones (§7 — Equalizer/Pulsing sind
  Gradient-Nutzer) ohne Sonderwege anschließbar.

## 9. Umsetzungs-Phasen und Akzeptanzkriterien

| Phase | Inhalt | Akzeptanz |
|---|---|---|
| **V0** | Kalibrier-/Treue-Runde (läuft, S44) | SSOT-§3-Kriterien |
| **V1** | Basis-Key-Registry + Import-Umbenennung | AVS-Korpus 35/35 + 311er-Pack: Render-Ergebnis **byte-/pixelgleich** vor↔nach (Umbenennung ist reine Alpha-Konversion); Umbenennungs-Fälle als neue Unit-Tests |
| **V2** | Audio-SSOT: MilkLoudness überall, `visdata`-Baustein, `getspec`/`getosc` in Milk-Slots, `ScriptInputFeeder` | Goldens grün; ein Loudness-Trace-Format für beide Welten; Sichttest eigener Ketten (Algorithmuswechsel §5) |
| **V3** | Konstanten + Funktions-Abgleich (EEL2-Referenz) | Transpiler-Fixtures erweitert; Original-Set nachweislich unverändert (Korpus-Sweep) |
| **V4** | Standalones → Module (Reihenfolge §7) | je Modul: Feature-Gleichstand-Checkliste + Serializer-Roundtrip-Test; am Ende Registry-Bereinigung |
| **V5** | Gradient-Parameter-Typ + LUT-Baustein | Fractal3D auf den Baustein umgestellt (Referenz-Nutzer), Panel-Editor angebunden |

Jede Phase einzeln mergefähig; V1–V3 sind Voraussetzung für den späteren
Meganode-Split, V4/V5 liefern dessen Muster.

## 10. Entscheide (Patrik, 2026-07-24 — alle entschieden ✅)

| # | Frage | Entscheidung |
|---|---|---|
| D1 | `vol`-Definition | **Option c (beides):** `vol`/`vol_att` = Mittel(bass, mid, treb) aus MilkLoudness; **zusätzlich** `rms` = geglätteter Wellenform-RMS 0..1 (heutiger `m_audioLevel` — der bisherige AVS-seitige `vol`-Wert wandert auf diesen Key) |
| D2 | Umbenennungs-Schema Import | Suffix `_p`, deterministisch `_p2`, `_p3`, …; case-insensitive Prüfung |
| D3 | Lumi-Konstantenliste | `$tau`, `$sqrt2`, `$invpi`, `$deg2rad`, `$rad2deg`; Shader `M_TAU`, `M_E`, `M_SQRT2` |
| D4 | `dt` ins gemeinsame Set? | ja — Frame-Delta in Sekunden, aus dem Frame-Snapshot |
| D5 | Waveform-Erweiterungen | Mini-Konzept vor der Portierung; Waveform bleibt am Ende der Portierungsreihe |

## 11. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-24 | Erstfassung aus der Diskussion Session 44 (Skript-Set, Basis-Key-Registry + Import-Umbenennung, Injektions-Schicht, Standalone-Portierung, Gradient-Parameter-Typ, Phasen V0–V5) |
| 1.1.0 | 2026-07-24 | §4 Klarstellung (Patrik): Umbenennung wirkt format-relativ — Quellformat-eigene Builtin-Keys binden immer an die Builtins, umbenannt wird nur Format-Fremdes |
| 1.2.0 | 2026-07-24 | Entscheide D1–D5 (Patrik): D1c `vol`=Band-Mittel + neuer Key `rms` (heutiger `m_audioLevel`), D2 Suffix `_p`, D3 Konstantenliste (`$deg2rad`/`$rad2deg` statt `$deg`/`$rad`), D4 `dt` ja, D5 Waveform-Mini-Konzept vorab — §10 von „offen" auf „entschieden" |
