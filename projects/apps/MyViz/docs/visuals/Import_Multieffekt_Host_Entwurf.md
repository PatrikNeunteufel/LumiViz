# MyViz — Multieffekt-Host (Entwurf, Import-Phase Roadmap 5)

> **Typ:** Entwurf (freigegeben) · **Bezug:**
> [Import_Analyse_AVS_MilkDrop.md](Import_Analyse_AVS_MilkDrop.md) §5.1/§5.2/§8.2 Baustein 3/§9.5 ·
> [Import_Fundament_Entwurf.md](Import_Fundament_Entwurf.md) (R4, liefert den Unterbau)
> **Status:** ✅ **Roadmap 5 vollständig umgesetzt** (Session 34, 2026-07-20) —
> Schritte 5.1–5.7b; Freigabe E1–E7 (E4 Re-Propagation, E5 Kette editierbar)

## Motivation / Ist-Zustand

Roadmap 1–4 hat alle **gemeinsamen Fundamente** der Import-Phase gebaut:
Transpiler (EEL→Lua), .avs-Parser (`AvsParser`, Baum aus `EffectNode`),
Skript-Kontext (`ScriptContext`/`ScriptSlotHost`), skriptbare Module (Grid/LUT),
GL-Feedback (`FeedbackBuffer` + `OffscreenBufferPool`) und den Beat-Prädiktor
(`BeatEstimator`). Was fehlt, ist der Baustein, der aus diesen Teilen ein echtes
AVS-Preset **rendert**: der **Multieffekt-Host** (Analyse §8.2 Baustein 3).

Ohne ihn ist kein nennenswertes AVS-Preset importierbar — AVS-Presets sind
**Effektketten**, keine flachen Parametersätze. Der `AvsParser` liefert den Baum
bereits fehlerfrei (Korpus 35/35); er endet aber bewusst an der Grenze
„Bytes → Effekt-Baum". R5 schließt die Lücke „Effekt-Baum → Bild".

**Wichtige Abgrenzung (Analyse §8.3, Concept §6.1):** Das ist ein *begrenzter,
importgetriebener* Ketten-Unterbau — **kein frei verdrahtbarer Node-Editor**. Die
Topologie kommt aus dem .avs-Baum; Bearbeitung ist Nicht-Ziel dieser Stufe.

## Zielbild

Ein neuer Visualizer **`MultiEffectVisualizer`** (über `VisualizerBase`), der:

1. einen **Laufzeit-Effektbaum** hält, der 1:1 die Topologie eines
   `AvsParser::EffectNode`-Baums abbildet (Container-Listen + Blatt-Effekte),
2. diesen Baum pro Frame nach dem **AVS-Render-Modell** (§5.1) rendert:
   Ping-Pong-Arbeitspuffer, pro Nicht-Root-Liste ein **persistenter Buffer**
   (`thisfb`) mit **Input-/Output-Blend** (14 Modi), **OnBeat-Aktivierung** und
   **EEL-Listen-Slots** (`enabled/clear/beat/alphain/alphaout`),
3. die **AVS-Kernmenge** (~16 Effekte, §5.2) als konkrete GL-/Skript-Effekte
   ausführt und alle nicht abgedeckten Effekte als **Passthrough + Import-Warnung**
   konserviert,
4. über einen **Übersetzer** `AvsParser`-Baum → Host-Baum befüllt wird und das
   Ergebnis als **eigenes verschachteltes Preset** persistiert (nicht über das
   flache `.lvp`-Schema).

Alle Skriptträger eines Presets teilen **einen** `shared_ptr<ScriptContext>`
(reg/q/gmegabuf preset-lokal, Entscheid §10.3). GL ausschließlich im Render-Thread.

## 1. Baustein „Laufzeit-Effektbaum" (Datenmodell, GL-frei)

Spiegelt `EffectNode`, aber als render-fertige Laufzeit-Knoten (nicht der
Parser-Blob). Zwei Knotenarten:

- **`EffectListNode`** (Container): hält `children`, den **Blend-/OnBeat-Zustand**
  aus `ListInfo` (`blendIn()/blendOut()/clearEveryFrame()/enabled()/beatRender…`),
  ein optionales **EEL-Listen-Slot-Paar** (Init/Frame über `ScriptSlotHost`) und —
  wenn Nicht-Root — einen **persistenten Listen-Buffer**.
- **`EffectLeaf`** (ein konkreter Effekt): trägt seine typisierten Parameter und ggf.
  einen `ScriptSlotHost`/`ScriptGridModule`/`ScriptLutModule`.

Der Baum wird beim Preset-Laden aus dem `AvsParser`-Baum **übersetzt** (Baustein 5)
und ist **editierbar** (E5): Knoten hinzufügen/entfernen/umordnen + Parameter ändern.
Mutationen laufen über den GUI-Thread unter `renderMutex()` (Threading-Vertrag §12);
nach jeder Mutation läuft der **Ketten-Compile-Pass** (u.a. Set-Render-Mode-Ausrollen,
E4) neu.

## 2. Baustein „Render-Kern" (AVS-Render-Modell, GL)

Faithful-Umsetzung von `r_list.cpp` (§5.1):

- **Arbeitsfläche = Ping-Pong-FBO-Paar** in Fenstergröße (physische Pixel,
  `size() * devicePixelRatio()`). **Render-Effekte** (Scopes) zeichnen in-place auf
  den aktuellen Puffer; **Transform-Effekte** (Blur, Movement, Color Mod …) lesen den
  aktuellen Puffer als Textur, schreiben in den Partner und **swappen** (entspricht
  AVS-Rückgabe-Bit 0).
- **Nicht-Root-Liste:** blendet den Eltern-Puffer per **Input-Blend** in ihren
  `thisfb` (persistent → Feedback/Trails innerhalb der Liste), rendert ihre Kinder auf
  einem eigenen Ping-Pong über `thisfb`, blendet per **Output-Blend** in den Eltern
  zurück.
- **Root-Liste:** rendert direkt auf die Arbeitsfläche; am Ende Blit auf den
  Default-FBO des Fensters (`FeedbackBuffer::endFrame`-Muster, das explizite
  READ/DRAW-Binds macht → funktioniert auch für verschachtelte Ziele).
- **OnBeat-Aktivierung:** eine Liste mit `beatRender` ist für `beatRenderFrames`
  Frames nach einem Beat aktiv; sonst übersprungen.
- **EEL-Listen-Slots:** vor dem Kinder-Lauf schreibt der Frame-Slot
  `enabled/clear/beat/alphain/alphaout` in Host-Variablen; der Host liest sie zurück
  und steuert damit Skip/Clear/Blend-Alpha der Liste.

Wiederverwendung: `thisfb` und die Feedback-Effekte (Blitter/Roto) nutzen
`FeedbackBuffer`; die 8 globalen Buffer (Buffer Save, Blend „Buffer") den
`OffscreenBufferPool`.

## 3. Baustein „Blend-Engine" (14 Modi als Shader)

Ein Fragment-Shader mit `uMode`-Uniform blendet zwei Texturen (src=Kind/Ergebnis,
dst=Ziel) → Ziel. Modi (In wie Out, §5.1 `r_list.cpp:995`): Ignore, Replace, 50/50,
Maximum, Additive, Subtractive 1-2, Subtractive 2-1, Every-other-line,
Every-other-pixel, XOR, Adjustable (0–255), Multiply, Buffer (Alpha aus globalem
Buffer, invertierbar), Minimum. Koordinaten-/Integer-Modi (Every-other-*, XOR) über
`gl_FragCoord` bzw. Integer-Bitops in GLSL. **Buffer**-Modus zieht die Alpha-Maske
aus einem `OffscreenBufferPool`-Slot.

## 4. Baustein „chain-scoped Beat" (BeatService light)

Der Host betreibt **einen** Onset-Detektor (`BeatModule`, wie die anderen
Visualizer) + `BeatEstimator.refine()` (Prädiktion, Konfidenz). Der Beat ist im
Effektfluss **mutierbar** (§5.1): der Effekt **Custom BPM** setzt/löscht das
Beat-Signal für die Folge-Effekte (AVS-Flags `0x10000000/0x20000000`). Beat und
`isBeat` fließen in Listen-OnBeat, Beat-Slots der Skripte und Feedback-OnBeat-Zoom.

## 5. Baustein „Übersetzer" (`AvsParser`-Baum → Host-Baum)

Rein datengetrieben, GL-frei, testbar:

- Rekursiver Walk über `ParseResult.root`. `isList` → `EffectListNode` (Blend/OnBeat
  aus `ListInfo`, Listen-EEL aus `initCode/frameCode`); sonst Blatt.
- Effekt-Dispatch über `id`/`apeId`. Für die Kernmenge: typisierte `fields`/`code`/
  `colors` in die Laufzeit-Parameter, EEL-Slots (Reihenfolge im File **immer**
  [Point/Level, Frame, Beat, Init]) per Transpiler in die `ScriptSlotHost`.
- **Set Render Mode (40):** wird **beim Ketten-Compile ausgerollt** (E4): der
  Knoten bleibt im Datenmodell erhalten (und damit editierbar), aber ein
  Compile-Pass rollt Linien-Blend/-Breite in die *folgenden* Scope-Effekte aus —
  kein Laufzeit-Zustand im Host. Der Pass läuft bei Import **und nach jeder
  Ketten-Bearbeitung** neu (Re-Propagation).
- Nicht abgedeckte Effekte / `decoded==false`: **Passthrough-Knoten** (rendert nichts,
  reicht den Puffer durch) + **Import-Warnung** mit Pfad. Nie hart abbrechen.
- Der Übersetzer trägt einen **Import-Report** zusammen (übernimmt die
  Parser-Warnungen + eigene „Effekt X noch nicht implementiert").

## Umsetzungsschritte (jeder einzeln grün)

Jeder Schritt endet mit grüner Suite (CPU-Teile unit-getestet) + benanntem
Sichttest (GL-Teile). Reihenfolge so, dass früh ein sichtbares Bild entsteht.

| Schritt | Inhalt | Absicherung |
|---|---|---|
| 5.1 ✅ | **Host-Skelett + Render-Kern (flach):** `MultiEffectVisualizer` als `IVisualizer`, Registry-Eintrag, Laufzeit-Baum-Datenmodell, Ping-Pong-Arbeitsfläche, Root-Liste rendert eine flache Kette. Effekte: **Clear**, **Fadeout**, **Invert**, **DebugBars** (host-eigen), **Passthrough**. *(Session 34; test_EffectChain, Suite 194)* | Unit-Tests Datenmodell; Sichttest „flache Kette rendert" |
| 5.2 ✅ | **Blend-Engine + Verschachtelung:** 14-Modi-Shader (Batch 1 echt, Rest→Replace+Warnung), `thisfb`-Listen mit In-/Out-Blend, OnBeat-Aktivierung, EEL-Listen-Slots (`ScriptSlotHost`, geteilter `ScriptContext`), stabile `nodeId`. *(Session 34, Suite 199)* | Unit-Tests Blend-Mapping/nodeId; Sichttest verschachtelte Liste + Blend |
| 5.3 ✅ | **Frame-Transform-Effekte (skriptlos):** Brightness, Fast Brightness, Blur, Mirror, OnBeat Clear, Colorfade — 1:1 aus `ref/vis_avs` portierte Shader. *(Session 34, Suite 203)* | Sichttest je Effekt |
| 5.4a ✅ | **Skript-LUT + Beat:** Color Modifier (`ScriptLutModule`→256-LUT-Shader), Custom BPM (Beat-Mutation). *(Session 34, Suite 206)* | Unit-Tests Typen/Clamps; Sichttest |
| 5.4b ✅ | **Grid-Warp:** Movement + Dynamic Movement (`ScriptGridModule`→per-Frame-Mesh). *(Session 34)* | Unit-Tests Gitter-Clamps; Sichttest |
| 5.4c ✅ | **Feedback + Buffer:** Blitter Feedback, Roto Blitter (Roto/Zoom-Feedback-Shader), Buffer Save (`OffscreenBufferPool`). *(Session 34, Suite 209)* | Unit-Tests Typen/Clamps; Sichttest |
| 5.4d ✅ | **SuperScope (E6):** `ScopeRenderer` als wiederverwendbare Zeichenklasse extrahiert (Dots/Thin/Thick, 1:1 aus Superscope); Host-SuperScope nutzt `SuperscopeModule` (Punkt-Skript) + `ScopeRenderer`. `SuperscopeVisualizer` **unangetastet** (Migrationssuite grün); dessen Adoption des Renderers = mechanischer Rest von E6a. *(Session 34, Suite 210)* | Superscope-Migrationssuite grün; Sichttest |
| 5.5 ✅ | **Übersetzer + Korpus:** `AvsChainTranslator` (`AvsParser`-Baum → Host-Baum, alle 17 Kern-Effekte + Listen-Blend/OnBeat/EEL, Set-Render-Mode-Ausrollen, COLORREF→RGB, Rest = Passthrough+Report); `MultiEffectVisualizer::loadAvsFile()`. **Korpus 35/35 übersetzt** (163 Knoten, kein Crash). *(Session 34, Suite 217)*. File-Menü-Hookup → 5.7 | Unit-Tests Übersetzer + Korpus-Smoke |
| 5.6 ✅ | **Preset-Persistenz (Host-eigen):** `ChainSerializer` — verschachteltes JSON (`{header, root}`, alle 19 Typen), `saveChainFile()`/`loadChainFile()`; unbekannte Typ-Keys → Passthrough (vorwärtskompatibel). *(Session 34, Suite 222)* | Unit-Tests Round-Trip (Struktur/Params/enabled/name/Vorwärtskompat) |
| 5.7a ✅ | **File-Menü-Hooks:** „Import AVS Preset…" (Ctrl+I), „Load/Save Effect Chain…" (.lvfx) — Events → MainWindow: QFileDialog + `loadAvsFile`/`loadChainFile`/`saveChainFile` unter `renderMutex()`, Import-Report als MessageBox. *(Session 34)* | Sichttest Import/Load/Save |
| 5.7b ✅ | **Ketten-Editor-Panel (E5):** `MultiEffectPanel` (dockbar, „Effect Chain") — Baumansicht (Add/Remove/Up/Down/Enable-Checkbox), Parameter-Editor je Effekt (Spinner/Farbe/Enum/Skript-Textfelder für alle 19 Typen); Mutationen unter `renderMutex()` + `recompileChain()`; folgt dem aktiven Visualizer per `VisualizerChangedEvent`. *(Session 34)* | Sichttest Editier-Roundtrip |

Reihenfolge-Begründung: 5.1–5.2 bauen das Gerüst mit billigsten Effekten (früh
sichtbar, GL-Risiko isoliert); 5.3 sind reine Shader-Effekte ohne Skript; 5.4 zieht
die R4-Bausteine ein (höchstes Integrationsrisiko, aber Fundamente stehen); 5.5 ist
der reine Daten-Schritt, der das Ganze an den Parser koppelt; 5.6/5.7 setzen auf dem
stabilen Datenmodell auf (Persistenz vor UI, damit der Editor gegen ein fertiges
Format arbeitet). **Jeder Schritt ist ein eigener Session-Block** — R5 ist bewusst
eine Sub-Roadmap, kein Einzel-Commit. Die Effekt-/Blend-Reste aus E2/E3 („Rest nach
Basis-Bau") folgen als Batches, sobald 5.1–5.4 stehen.

## Akzeptanzkriterien

1. Suite grün (aktuell 185 Cases, 0 Skips) **plus** neue Tests für Datenmodell,
   Blend-Mode-Mapping, Übersetzer, Preset-Roundtrip.
2. **Bestehende Visualizer unverändert** — der Host ist additiv (neuer Typ), kein
   Eingriff in Superscope & Co. (außer der bewussten Scope-Renderer-Extraktion, E6,
   die Superscope bit-identisch lässt — Migrationssuite grün).
3. Die 35 Referenz-Presets laden und rendern ohne Leak/Crash; nicht abgedeckte
   Effekte erscheinen als Passthrough + Warnung im Import-Report (kein Hard-Fail).
4. Host übersteht Resize, Undock, Fullscreen, Visualizer-Wechsel ohne Leak/Crash
   (Render-Thread-Vertrag §12: GL nur im Render-Thread, kein Logging von dort).
5. Frametime eines mittleren Referenz-Presets bei 60 fps im Budget (Messlauf wie
   Session 32); Skript-Hot-Paths bleiben schlank (reg/q-Sync nur wo nötig).

## Nicht-Ziele (bewusst)

- **Kein frei verdrahtbarer Node-Editor** — der Ketten-Editor (5.7) bearbeitet den
  **Listen-Baum** (verschachtelte Ketten, wie AVS selbst), keine freie
  Graph-Verdrahtung (Concept §6.1).
- **Keine Voll-Abdeckung aller 46 Effekte** — Kernmenge (~16) zuerst, „Mittel/lohnend"
  danach, „Exot" (AVI, Picture, SVP, Text, Laser) verzichtbar (§5.2).
- **Keine MilkDrop-Spezifika** (Warp-Mesh, Blur-Pyramide, Motion Vectors) — Roadmap 6;
  das Grid-Modul ist aber schon per_vertex-tauglich.
- **Keine Preset-Übergänge/Crossfade** (Analyse §8.2 Punkt 8) — späteres Player-Feature
  oberhalb der Pipeline.
- **Keine bit-genaue Pixel-Parität** zu vis_avs — GL-nahe Annäherung, nicht
  Software-Renderer-Emulation.

## Entscheidungen (Patrik, 2026-07-20)

| # | Frage | Entscheid |
|---|---|---|
| E1 | Host-Integration | **(a)** neuer `IVisualizer` `MultiEffectVisualizer` + eigener verschachtelter Preset-Layer |
| E2 | Effekt-Abdeckung Stufe 1 | **(b)** tragfähiger Kern zuerst; **Rest folgt direkt auf den Basis-Bau** (Batches nach 5.1–5.4), Nicht-Kern = Passthrough+Warnung |
| E3 | Blend-Modi-Umfang | **(b)** häufige zuerst (Ignore, Replace, 50/50, Adjustable, Additive, Maximum, Minimum, Multiply, Buffer); exotische als Batch direkt danach; Fallback Replace + Warnung |
| E4 | Set Render Mode (40) | **(a) mit Re-Propagation:** Ausrollen ist ein **Ketten-Compile-Pass** — Knoten bleibt im Modell; nach jeder Bearbeitung des Knotens/der Kette wird neu ausgerollt |
| E5 | Editier-UI & Host-Preset | **(b) — abweichend von der Empfehlung:** die Kette soll **gleich editierbar** sein → Schritt 5.7 (baum-fähiges Ketten-Panel) gehört zu R5 |
| E6 | SuperScope im Host | **(a)** Scope-Render-Kern aus `SuperscopeVisualizer` extrahieren (ein Renderer, zwei Nutzer); Migrationssuite sichert Bit-Identität ab |
| E7 | Übersetzer-Ort | **(a)** im App-Modul (`MyViz`) — erzeugt GL-nahe Laufzeitobjekte, gehört nicht in eine Qt-freie Lib |

## Siehe auch

- [Import_Analyse_AVS_MilkDrop.md](Import_Analyse_AVS_MilkDrop.md) — §5.1 Render-Modell,
  §5.2 Effekt-Inventar + Priorisierung, §8.2 Baustein 3, §10 Entscheide
- [Import_Fundament_Entwurf.md](Import_Fundament_Entwurf.md) — R4-Unterbau
- [Visualizer_Architecture.md](Visualizer_Architecture.md) — §12 Threading-Vertrag
- `AvsParser.md` · `FeedbackBuffer.md` · `ScriptContext.md` · `ScriptSlotHost.md` ·
  `BeatEstimator.md` — API der konsumierten Bausteine
- Referenz: `../../../../../../ref/vis_avs` (`r_list.cpp`, `r_stack.cpp`, je Effekt `r_*.cpp`)

## Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-20 | 5.7b (Session 34): `MultiEffectPanel` — dockbarer Ketten-Baum-Editor (Add/Remove/Reorder/Enable + Parameter-Editor je Typ + EEL-Skriptfelder), Mutationen unter renderMutex + recompile. **Roadmap 5 vollständig (5.1–5.7b).** |
| 0.7.0 | 2026-07-20 | 5.7a (Session 34): File-Menü — „Import AVS Preset…" (Ctrl+I) + „Load/Save Effect Chain…" (.lvfx) via Events → MainWindow (QFileDialog + Host-API unter renderMutex). **AVS-Import ist jetzt UI-bedienbar.** Offen: 5.7b Ketten-Editor-Panel |
| 0.6.0 | 2026-07-20 | 5.6 (Session 34): `ChainSerializer` (verschachteltes JSON, Round-Trip, vorwärtskompat) + `saveChainFile`/`loadChainFile`. Suite 222/222. Offen: 5.7 Editor-UI + File-Menü |
| 0.5.0 | 2026-07-20 | 5.5 (Session 34): `AvsChainTranslator` (AVS-Baum → Host-Kette, 17 Effekte + Listen-Blend/OnBeat/EEL + SRM-Unroll + COLORREF-Swap + Passthrough-Report) + `loadAvsFile()`; **Korpus 35/35** (163 Knoten). Suite 217/217. Offen: 5.6 Persistenz, 5.7 Editor-UI + File-Menü |
| 0.4.0 | 2026-07-20 | 5.4d (Session 34): SuperScope via `ScopeRenderer` (extrahiert) + `SuperscopeModule`; `SuperscopeVisualizer` unangetastet. **5.4 komplett** — 17 Effekte, Suite 210/210. Offen: 5.5 Übersetzer, 5.6 Persistenz, 5.7 Editor-UI |
| 0.3.0 | 2026-07-20 | Umsetzung 5.1–5.4c (Session 34): Host + Render-Kern, Blend/Verschachtelung, 16 Effekte (Clear/Fadeout/Invert/Brightness/FastBright/Blur/Mirror/OnBeatClear/Colorfade/ColorModifier/Movement/DMove/BlitterFeedback/RotoBlitter/BufferSave/CustomBPM) + DebugBars/Passthrough; Suite 209/209. Offen: 5.4d SuperScope (E6, invasiv), 5.5–5.7 |
| 0.2.0 | 2026-07-20 | Freigabe Patrik — E1–E7 entschieden; E4 als Compile-Pass mit Re-Propagation; E5 abweichend (b): Ketten-Editor-UI als Schritt 5.7 in R5 aufgenommen; Umsetzung startet mit 5.1 |
| 0.1.0 | 2026-07-20 | Erstfassung zur Freigabe (Session 34) — Multieffekt-Host als `MultiEffectVisualizer`; Bausteine Laufzeit-Baum, Render-Kern (AVS-Modell), Blend-Engine, chain-scoped Beat, Übersetzer; 6 Umsetzungsschritte; offene Fragen E1–E7 |
