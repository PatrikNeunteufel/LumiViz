# MyViz — Feedback-/Skript-Modul-Fundament (Entwurf, Import-Phase Roadmap 4)

> **Version:** 0.1.0
> **Datum:** 2026-07-20
> **Typ:** Entwurf (freigegeben)
> **Status:** Freigegeben (Patrik, 2026-07-20) — Entscheide E1–E5 wie empfohlen:
> Blit-Resize · SlotHost-Extraktion jetzt · bpm.cpp voll · Superscope-Trail · q64
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Import_Analyse_AVS_MilkDrop.md](Import_Analyse_AVS_MilkDrop.md) §8.2
> (Bausteine 1–2 + 4–6) · [Visualizer_Architecture.md](Visualizer_Architecture.md)
> (§3 Module, §12 Threading) · [LuaScriptEngine.md](../../include/scripting/LuaScriptEngine.md)
> · [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) §5.6 (Leitplanken)
> **Sprache:** Deutsch

---

## Motivation / Ist-Zustand

Roadmap 1–3 haben geliefert: sandboxed LuaScriptEngine (4-Slot-Modell),
EEL→Lua-Transpiler, .avs-Parser. Was fehlt, sind die **gemeinsamen Fundamente**,
auf denen sowohl der Multieffekt-Host (Roadmap 5, AVS) als auch der
MilkDrop-Import (Roadmap 6) stehen — laut Analyse §8.2:

1. **Doppelpuffer-Feedback** (Baustein 1): MilkDrop-Essenz (Warp aufs
   Vorframe-Bild), AVS-Trails. Heute rendern alle Visualizer direkt in den
   Default-Framebuffer des Fensters; es gibt **kein FBO im Codebestand** —
   Trails laufen CPU-seitig (`HoldFadeEffectT` hält Frame-Kopien als Daten).
2. **Skriptbare Module** (Baustein 2): Skript läuft heute nur im Superscope
   (pro Punkt). Es fehlen die zwei anderen Laufformen: **pro Gitterknoten**
   (AVS Movement/Dynamic Movement, MilkDrop per_vertex) und **pro LUT-Eintrag**
   (AVS Color Modifier).
3. **Geteilter Skript-Kontext** (Baustein 4): Jede LuaScriptEngine ist heute eine
   Insel (eigener gmegabuf, eigene Env). Ein Preset mit mehreren skriptbaren
   Modulen braucht geteilte `reg`/`gmegabuf`/`q1–q64` (preset-lokal, Entscheid
   §10.3) mit MilkDrop-Snapshot-Semantik.
4. **BeatService** (Baustein 5): Beat-Erkennung ist ein einfacher
   Kanten-/Energie-Detektor je Visualizer (`BeatModule`). AVS-Presets erwarten
   einen **vorhersagenden** Beat (Onset + BPM-Prädiktor, `bpm.cpp`, BSD) und
   einen **überschreibbaren** Beat je Kette (Custom BPM).
5. **Buffer-Pool** (Baustein 6): AVS' 8 globale benannte Buffer (Buffer Save,
   Blend-Modus „Buffer") haben kein Gegenstück.

Baustein 3 (Multieffekt-Host) ist bewusst **nicht** Teil dieses Entwurfs —
er ist Roadmap 5 und baut auf allem hier auf.

## Zielbild

Nach Roadmap 4 kann ein einzelner Visualizer (Keimzelle: Superscope):
sein Vorframe-Bild als Textur einlesen (GL-Echo/Trail), neben dem Punkt-Skript
auch Gitter- und LUT-Skripte betreiben, sich mit anderen Skript-Trägern
desselben Presets Daten teilen (`q`/`reg`/`gmegabuf`), und einen vorhersagenden
Beat konsumieren. Alles davon ist einzeln testbar und verhält sich im
Multieffekt-Host (Roadmap 5) unverändert — dort kommen nur Verschachtelung,
Blend-Modi und Listen-Buffer dazu.

## 1. Baustein „ScriptContext" — geteilter Skript-Kontext (CPU, zuerst)

**Neu:** `lumi::scripting::ScriptContext` (`include/scripting/ScriptContext.hpp`).

- Gehört einem Preset bzw. (heute) einer Visualizer-Instanz; Engines erhalten
  ihn als `std::shared_ptr<ScriptContext>` bei Konstruktion (Default: eigener
  privater Kontext → bestehendes Verhalten bleibt).
- **Zieht aus der Engine heraus:** `gmegabuf` (heute engine-lokal) und den
  PRNG-Seed-Verbund. `megabuf` bleibt engine-lokal (AVS: je Effekt).
- **Neu darin:** `reg00–reg99` als geteilter Namensraum (heute schlichte
  Env-Variablen — bei mehreren Engines wären sie fälschlich getrennt) und
  `q1–q64` mit **Snapshot-Semantik**: `captureInit()` nach den Init-Läufen,
  `captureFrame()` nach dem Frame-Lauf; nachgelagerte Läufe (Point/Grid/LUT)
  lesen den Frame-Snapshot (MilkDrop-Modell, MD3-Superset q64).
- **Threading-Vertrag:** Ein ScriptContext gehört genau einem Render-Thread —
  kein Locking (alle Skripte eines Presets laufen sequenziell im selben
  Thread; Visualizer_Architecture §12 bleibt unberührt). Die app-globalen
  32 Atomic-Slots (`app.gget/gset`) bleiben unverändert daneben bestehen.
- Engine-Anbindung: `reg`- und `q`-Zugriffe laufen wie bisher über Env-Variablen;
  die Engine synchronisiert sie beim Slot-Ein-/Austritt mit dem Kontext
  (Sync-Listen statt Metatable-Magie — Point-Hot-Path bleibt unangetastet,
  gesynct wird nur an Slot-Grenzen).

**Dazu:** Extraktion eines **`ScriptSlotHost`** (`include/scripting/ScriptSlotHost.hpp`)
aus dem SuperscopeModule: das wiederkehrende Muster „4 EEL-Slots → Transpile
(Dialekt Avs) → compile → run mit Fehlerzustand/Fallback" als eigener Baustein,
den SuperscopeModule und die neuen Module (§2) gemeinsam nutzen.
Superscope-Verträge (Host-Inputs, `t` gehört dem Skript, reservierte Namen)
ändern sich dabei **nicht** — die bestehende Suite zäunt das ein.

## 2. Baustein „skriptbare Module" — Grid und LUT

Zwei neue Module in `include/visualizers/modules/scripting/` (Muster
SuperscopeModule: kein GL im Modul, reine Daten rein/raus, voll unit-testbar):

### 2.1 ScriptGridModule (pro Gitterknoten)

- Konfigurierbares Gitter `xres × yres` (Default 32×24, Cap z. B. 96×72);
  Slots Init/Frame/Beat/**Point** (Point = pro Knoten).
- Knoten-Vertrag (Superset aus AVS-DMove und MilkDrop-per_vertex):
  Inputs `x, y` (−1..1), `d, r` (Polar), `w, h, time, dt, b` —
  Outputs `x, y` (neue Quellkoordinate) bzw. `d, r` (Polar-Modus, Flag
  `rectcoords` wie AVS), optional `alpha`.
- Ergebnis: ein Verschiebungsfeld (UV je Knoten) als `std::vector<Vec2f>`
  (+ Alpha), das der GL-Pfad als Mesh über das Feedback-/Quell-Bild legt
  (§3). Modul selbst bleibt GL-frei.
- Abnahme unit-seitig: bekannte Skripte (Identität, Zoom, Rotation) liefern
  erwartete Knotenwerte.

### 2.2 ScriptLutModule (pro LUT-Eintrag)

- 256-Einträge-LUT je Kanal (RGB); Slots **Level**/Frame/Beat/Init
  (AVS Color Modifier); `recompute`-Flag: LUT nur neu rechnen, wenn Frame-/
  Beat-Code Variablen ändert (sonst einmalig).
- Vertrag: Input/Output `red, green, blue` (0..1) je Eintrag.
- Ergebnis: `std::array<std::array<float,256>,3>`, GL-seitig später eine
  1D-LUT-Textur (Farb-Post); unit-testbar ohne GL (Invert-Skript → LUT
  gespiegelt).

Beide nutzen ScriptSlotHost + ScriptContext; Slots sind **EEL** (durch
EelTranspiler) — derselbe Vertrag wie die Superscope-Slots, Import-treu.

## 3. Baustein „FeedbackBuffer" — Doppelpuffer als Render-Fähigkeit

**Neu:** `include/visualizers/render/FeedbackBuffer.hpp` (+ `.cpp`) — zwei
persistente FBO+Textur-Paare (previous/current) mit `swap()` pro Frame.

- **Besitz & Thread:** lebt vollständig im Render-Thread (Erzeugung in
  `initialize()`, Zerstörung in `cleanup()` des Visualizers) — der
  Threading-Vertrag §12 bleibt unverändert.
- **Opt-in statt Umbau:** `VisualizerBase` bekommt geschützte Helfer
  (`enableFeedback()`, `feedback()`); ein Visualizer, der Feedback nutzt,
  rendert in den current-Buffer und blittet am Frame-Ende auf den
  Default-Framebuffer (ein Fullscreen-Quad, vorhandener Shader-Stil).
  Visualizer ohne Feedback rendern exakt wie heute — **0 Änderungen** an den
  vier anderen Visualizern.
- **„PreviousFrame" als benannter Input:** die Textur des previous-Buffers,
  abfragbar im Render-Pfad; im Stage-Schema (Leitplanke §5.6.2) bleibt der
  Parameterraum unangetastet — die Fähigkeit ist Code-seitig, kein Parameter.
- **Resize-Politik:** Empfehlung **Blit-Skalierung** des alten Inhalts auf die
  neue Größe (MilkDrop-Verhalten; Trails überleben Fenster-Resize). Verwerfen
  (AVS-Verhalten) wäre einfacher — Entscheid E1.
- **Erster Nutzer (Sichttest-Fall):** Superscope-Zusatz-Post „GL-Echo/Trail"
  (`post.trail.*`: enabled, decay, zoom) — zeichnet das Vorframe-Bild gedimmt
  und leicht gezoomt unter die neuen Punkte (klassischer AVS-Blitter-Feedback-
  Look). Der bestehende CPU-HoldFade bleibt unverändert daneben (kein Ersatz
  in dieser Roadmap).

**Buffer-Pool (Baustein 6) — nur API-Gerüst:** `OffscreenBufferPool`
(gleiche Datei-Nachbarschaft): bis zu 8 benannte Textur/FBO-Slots je
Besitzer, on-demand alloziert (`get(n, w, h, allocate)` — Semantik von AVS'
`getGlobalBuffer`). In Roadmap 4 entsteht nur die Verwaltung + Lebenszyklus
im Render-Thread; erste echte Nutzer (Buffer Save, Blend „Buffer") kommen mit
dem Multieffekt-Host (Roadmap 5).

## 4. Baustein „BeatEstimator" — vorhersagender Beat

**Neu:** `include/visualizers/modules/processing/BeatEstimator.hpp` (+ `.cpp`) —
Portierung der `bpm.cpp`-Logik (BSD-3, Copyright-Hinweis mitführen):
Onset-Eingang → BPM-Schätzung mit Konfidenz, Halb-/Doppel-Beat-Korrektur,
Einrasten („sticked") und Vorhersage-Ticks.

- **Schichtung:** Der bestehende `BeatModule`-Onset (Kanten/adaptiv) bleibt der
  Eingang; der Estimator sitzt dahinter und liefert `beat()` (vorhergesagt),
  `bpm()`, `confidence()`. Konsument wählt per Enum `BeatSource`
  {Onset (Ist-Verhalten, Default), Predicted}.
- **Kein globaler Service:** chain-scoped Objekt im Besitzer (heute Visualizer,
  später Host) — kein Singleton, kein Locking (Render-Thread-Besitz).
  Der Override-Hook (Custom BPM: Beat setzen/löschen im Effektfluss) wird als
  API vorgesehen (`overrideBeat(bool)`), aber erst vom Host (Roadmap 5) genutzt.
- **Tests:** synthetische Onset-Folgen (konstant 120 BPM, Tempo-Wechsel,
  Aussetzer) → Konvergenz/Konfidenz/Vorhersage-Raster; kein Audio nötig.

## 5. Umsetzungsschritte (jeder einzeln grün)

| Schritt | Inhalt | Absicherung |
|---|---|---|
| 4.1 ✅ | ScriptContext + ScriptSlotHost-Extraktion; Superscope auf beides umgezogen *(2026-07-20, Suite 163/163; Abweichungen: PRNG bleibt engine-lokal; sourceMentions wortgenau+case-insensitiv statt Substring)* | bestehende Superscope-/Lua-/Transpiler-Suite bleibt grün; neue Context-Tests (reg/q/gmegabuf-Teilung, Snapshot-Semantik) |
| 4.2 ✅ | ScriptGridModule + ScriptLutModule (CPU-Kern) *(2026-07-20, Suite 177/177)* | neue Unit-Tests (Identität/Zoom/Rotation-Gitter; Invert/Identity-LUT; recompute-Verhalten) |
| 4.3 🔶 | FeedbackBuffer + Blit-Pfad + OffscreenBufferPool-Gerüst; Superscope `post.trail.*` als erster Nutzer *(Code 2026-07-20, Builds grün — **Sichttest + Frametime ausstehend**; GL-Klassen sind entgegen Entwurfstext nicht GL-frei unit-testbar)* | Sichttest (Trail-Look, Resize, Undock/Fullscreen) + Frametime-Vergleich |
| 4.4 ✅ | BeatEstimator (bpm.cpp-Port) + Umschalt-Param `map.beat.predict` in Pulsing/Superscope *(2026-07-20, Suite 185/185; Sichttest Beat-Stabilität steht aus)* | Unit-Tests synthetische Onset-Folgen; Sichttest Beat-Stabilität |

Reihenfolge-Begründung: 4.1/4.2 sind reine CPU-Bausteine mit voller
Testabdeckung und entkoppeln das Skript-Fundament vom GL-Risiko; 4.3 ist der
einzige GL-Eingriff und bleibt opt-in; 4.4 ist unabhängig und kann notfalls
geschoben werden.

## 6. Akzeptanzkriterien

1. Suite grün (aktuell 152 Cases, 0 Skips) **plus** neue Tests für Context,
   Grid, LUT, Estimator, Pool-Verwaltung.
2. Superscope verhält sich mit `post.trail.enabled=false` (Default) und
   Lua an/aus **bit-identisch zu heute** (keine Verhaltensänderung ohne Opt-in).
3. Feedback übersteht Resize, Undock, Fullscreen und Visualizer-Wechsel ohne
   Leak/Crash (Kontext bleibt, Surface wechselt — §12-Regeln).
4. Kein GL-Code in Modulen; kein Skript-Lauf außerhalb des Render-Threads;
   kein Logging aus dem Render-Thread.
5. Frametime Superscope mit Trail bei 1000 Punkten: Budget-Zuwachs < 1 ms
   (Messlauf wie Session 32).

## Nicht-Ziele (bewusst)

- **Kein Multieffekt-Host** (Verschachtelung, 14 Blend-Modi, Listen-Buffer,
  OnBeat-Listen) — Roadmap 5; dieser Entwurf liefert nur dessen Unterbau.
- **Kein Anschluss des AvsParser-Ergebnisses** an die Module (Übersetzung
  .avs → Preset ist Roadmap 5).
- **Kein Skript-Editor-UI** und keine Skript-Persistenz in Presets (bewusste
  Lücke aus Roadmap 1, unverändert).
- **Kein Ersatz des CPU-HoldFade** durch GL-Feedback (Koexistenz; Konsolidierung
  ist spätere Kür).
- **Keine MilkDrop-Spezifika** (Warp-Shader, Blur-Pyramide, Motion Vectors) —
  Roadmap 6; das Gitter-Modul ist aber bereits per_vertex-tauglich ausgelegt.

## Offene Fragen (bitte mit Freigabe entscheiden)

| # | Frage | Optionen | Empfehlung |
|---|---|---|---|
| E1 | Feedback bei Resize | (a) Inhalt verwerfen (AVS, einfacher) · (b) Blit-skalieren (MilkDrop, Trails überleben) | **(b) Blit** |
| E2 | ScriptSlotHost-Extraktion aus SuperscopeModule | (a) jetzt extrahieren (ein Muster, drei Nutzer) · (b) in Grid/LUT kopieren, später zusammenführen | **(a) jetzt** — Suite sichert ab |
| E3 | bpm.cpp-Port | (a) voll in Roadmap 4 (Schritt 4.4) · (b) nur Schnittstelle, Port in Roadmap 5 | **(a) voll** — BSD, gut testbar, unabhängig |
| E4 | Erster Feedback-Nutzer | (a) Superscope `post.trail.*` · (b) eigenes Demo-/Testmodul ohne Nutzwert | **(a) Superscope-Trail** — sichtbarer Mehrwert, echter Sichttest |
| E5 | q-Var-Umfang im ScriptContext | (a) q1–q32 (MD2) · (b) q1–q64 (MD3-Superset, Analyse §4.2) | **(b) q64** |

## Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.2.0 | 2026-07-20 | Freigabe Patrik — E1–E5 wie empfohlen; Umsetzung startet mit 4.1 |
| 0.1.0 | 2026-07-20 | Erstfassung zur Freigabe (Session 33) — Bausteine §8.2/1–2+4–6 als ScriptContext, ScriptSlotHost, Grid-/LUT-Module, FeedbackBuffer+Pool, BeatEstimator; 4 Umsetzungsschritte |
