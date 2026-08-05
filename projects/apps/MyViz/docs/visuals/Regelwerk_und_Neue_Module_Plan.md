# Regelwerk & neue Module — Umsetzungsplan (Stränge R / S / G)

> **Version:** 1.5.0
> **Datum:** 2026-08-06 (Session 69)
> **Typ:** Steuerdokument (Plan + Entscheide)
> **Status:** Strang R ✅ + Strang S KOMPLETT (S1–S4, Session 65; Netz-Abnahme S3 braucht API-Key Patrik) + **G1 ✅ umgesetzt (S69, Sichttest offen)**; G2 offen
> **Sprache:** Deutsch
> **Kontext:** MilkDrop-Kalibrierung S63/S64 (acht Legacy-Fixklassen),
> `Vereinheitlichung_Konzept.md` (Standalones→Module), `Offene_Punkte.md` §3/§7

Drei Stränge, in dieser Reihenfolge. R ist das Fundament (legacy/modern als
explizite Betriebsart), S und G bauen darauf auf (beide sind „modern"-Bürger
erster Klasse). Die Rest-Kalibrierung (11 Port-Bugs, §3) läuft unabhängig
weiter und wird durch R nicht verändert (Import-Default = legacy = heutiges
Verhalten; die Triage-Baseline `s64f` bleibt gültig).

---

## Strang R — Regelwerk Legacy/Modern ✅ (Session 65)

**Ziel:** Jeder MilkDrop-Node deklariert seine Betriebsart. Imports laufen
per Default original-treu (alle S64-Emulationen aktiv), Neubauten modern
(IEEE, keine Legacy-Krücken). Einzelschalter erlauben Mischformen.

### R1 — Schema & Zustand

- `MilkdropPresetState` erhält:
  - `regelwerk: Legacy | Modern | Benutzerdefiniert` (Default beim Import: `Legacy`)
  - Einzelschalter (nur bei `Benutzerdefiniert` wirksam, sonst aus der Betriebsart abgeleitet):
    - `divVertragD3d9` (Transpiler emittiert `_div` statt rohem `/`)
    - `unormTrunkierung` (Feedback-Pfad trunkiert wie D3D9 statt zu runden)
    - `qGarbageEpsilon` (Init-lose Presets starten mit q≈1e-6 statt exakt 0)
    - `uvSanitize` (NaN→0 + Double-Wrap für Riesen-UVs)
  - PS-Version-Override: `psWarp`, `psComp` ∈ `Auto | PS2 | PS3 | MD1erzwingen`
    (`Auto` = PSVERSION-Felder der .milk; `MD1erzwingen` = Custom-Text
    ignorieren → die S64-Strip-Bisektion als App-Feature)
- Ableitung: `Legacy` = alle vier Schalter AN · `Modern` = alle AUS.
- SSOT der Bedeutung je Schalter: `MilkdropVisualizer.md` §Fixklassen
  (1.15.0–1.18.0) — dort ist jede Emulation mit Beweis dokumentiert.

### R2 — Serialisierung & Migration

- `.lvfx`-Roundtrip über den ChainSerializer (neue Felder optional; fehlend
  ⇒ `Legacy` + `Auto` — damit sind ALLE Bestands-Presets unverändert).
- MilkParser bleibt unangetastet (das Regelwerk ist Node-, nicht
  Dateiformat-Sache).
- Wächter-Test: Roundtrip mit gesetzten/fehlenden Feldern; Migrations-Default.

### R3 — Wirkung im Renderer/Transpiler

- `HlslTranspiler::transpile()` erhält eine Option `d3d9Div` (Default true):
  false ⇒ roher `/`-Operator. Goldens/Tests für beide Emissionen.
- `MilkdropVisualizer`: UNORM-Trunkierung (MD1-Warp-Uniform-Flag +
  Custom-Warp-Epilog-Variante), q-Epsilon und UV-Sanitize je Node-Zustand
  gaten. Shader-Rebuild bei Umschalten (Revision++, wie Param-Wechsel).
- PS-Override: Auswahl der Shader-Texte im `applyState`-Pfad (`MD1erzwingen`
  ⇒ Custom-Texte verwerfen, Klassifizierer-Weg wie bisher).

### R4 — Panel-UI

- MultiEffectPanel, Milkdrop-Editor: Combo „Regelwerk" + aufklappbare
  Einzelschalter (nur bei `Benutzerdefiniert` aktiv) + zwei PS-Combos.
- Tooltips nennen je Schalter den Original-Bezug (eine Zeile je Fixklasse).

### R5 — Prüfstand & Abnahme

- Triage-Doppellauf: Voll-Triage mit Default (Legacy) MUSS bitgleich zur
  s64f-Klassenliste sein (0 Wechsel) — beweist, dass R nichts am
  Import-Verhalten ändert.
- Stichproben-Lauf `Modern` über 5 bekannte Presets (NEON, R211, gb003,
  infinity 2, riding the wave) mit dokumentierten Erwartungs-Deltas
  (NaN-Schwarz kehrt zurück etc.) — beweist, dass die Schalter WIRKEN.
- Unit-Tests: Transpiler-Option (beide Emissionen), Roundtrip, Ableitung
  Legacy/Modern→Schalter.

**Fertig-Kriterium R:** Beide Läufe grün, Panel bedienbar, Doku
(MilkdropVisualizer.md + dieses Dokument) nachgezogen.

**Abnahme (Session 65):** Voll-Triage s65 klassengleich zu s64f (311 Presets,
0 Wechsel, OK 295) · Modern-Stichprobe belegt Schalter-Wirkung (NEON →
NaN-Schwarz, riding → Trunkierungs-Delta; Bug-Fixes gb003/infinity 2 bleiben
korrekt unangetastet — `out/milkdrop_modern_s65/MODERN_STICHPROBE.md`) ·
Tests 498 grün · Zusatz-Werkzeug: `LUMIVIZ_MILKDROP_REGELWERK=modern|legacy`
(Diagnose-Override, kein App-Zustand) · Panel-Sichtprüfung offen (Patrik).

---

## Strang S — Shadertoy-Modul (S1+S2 ✅ · S3 code-komplett, Session 65)

**Ziel:** Eigener Node-Typ „Shadertoy" (modernes Regelwerk, kein
Legacy-Ballast): Shader per **URL/ID direkt importieren** oder Code
einfügen, Audio-Reaktivität eingebaut und nachrüstbar.

### S1 — Node + GLSL-Wrapper (MVP, Code-Einfügen)

- Neuer Visualizer/Node `ShadertoyVisualizer` (Muster: MilkdropVisualizer,
  aber drastisch schlanker — ein Fragment-Pass, FBO in Chain-Auflösung).
- Wrapper GLSL-ES→GLSL 330: `mainImage(out vec4, in vec2)` in `main()`
  einbetten; Uniform-Block `iResolution, iTime, iTimeDelta, iFrame, iMouse,
  iDate, iSampleRate, iChannelTime[4], iChannelResolution[4]`.
  Zeit = deterministische Sim-Uhr (frame/60 — Batch-Renderer-Merkregel!).
- Editor-Feld „Shader-Code" (mehrzeilig, einfügen aus Zwischenablage);
  Kompilierfehler mit Zeilennummer im Import-Report-Stil.
- Prüfstand: 3–5 bekannte lizenzfreie Test-Shader als Sonden (eigene,
  NICHT von Shadertoy kopierte) + Standalone-Statistikpfad wiederverwenden.

### S2 — Audio (der Kern des Wunsches)

- `iChannel0` (konfigurierbar welcher Kanal) = **Shadertoy-natives
  Audio-Layout**: 512×2-Textur, Zeile 0 = FFT-Spektrum, Zeile 1 = Waveform
  (unser Analyzer liefert beides; Skalierung an Shadertoy-Wertebereich
  0..1 anpassen und per Sonde gegen einen Referenz-Shader vermessen).
- Zusätzlich LumiViz-Uniforms: `bass, mid, treb, vol, beat` (MilkLoudness +
  BeatEstimator) — damit werden auch NICHT-audioreaktive Shadertoys
  nachrüstbar (Nutzer ersetzt Konstanten im Code durch diese Uniforms).
- Ausbaustufe: „Audio-Mod"-Skript-Slot am Node (EEL/Lua wie überall):
  rechnet pro Frame benutzerdefinierte Uniforms (`mod1..mod8`) aus
  Audio-Größen — Audio-Mapping ohne Shader-Änderung.

### S3 — URL-/ID-Import (Wunsch Patrik S64)

- Editor-Feld „Shadertoy-URL oder -ID" + Knopf „Importieren":
  `https://www.shadertoy.com/view/<ID>` → offizielle API
  `https://www.shadertoy.com/api/v1/shaders/<ID>?key=<AppKey>` (JSON:
  Name/Autor/Lizenzhinweis, Renderpässe, Inputs).
- **API-Key** (kostenlose Registrierung) in den Settings (lokal,
  `CMakeUserPresets`-Klasse „sensibel" — NICHT ins Repo).
- Grenzen sauber melden (Import-Report-Muster): API liefert nur Shader mit
  Sichtbarkeit „public+api" → Fallback bleibt Code-Einfügen; nicht
  unterstützte Inputs (Video, Cubemap, Keyboard, VR) ⇒ Platzhalter +
  Meldung. Textur-/Noise-Inputs: Stufe 1 nur Shadertoys Standard-Noise
  (generieren wir selbst), Media-Downloads erst Stufe 2.
- Metadaten (Name/Autor/URL/Lizenz) im Node speichern und im Panel zeigen —
  **Lizenz-Vorbehalt: Shadertoy-Default ist CC BY-NC-SA; Inhalte bleiben
  lokal (VisualsPresets extern), nichts davon ins Repo.**
- Netzwerk: Qt Network, nur auf Knopfdruck (kein Auto-Fetch).

### S4 — Multipass (Ausbaustufe) ✅ (Session 65)

- Buffer A–D als zusätzliche FBO-Pässe je Frame (Topologie aus dem
  API-JSON; Selbst-Referenz = FeedbackBuffer-Muster). Erst nach S1–S3,
  eigener Sichttest.

**Stand S65:** `ShadertoyPass` (Code + 4 Kanal-Bindungen; −1/0..3/4 =
nichts/Buffer/Audio, SSOT `imageInput`+`input` — `audioChannel` nur noch
Lese-Migration), RGBA32F-Ping-Pong je Buffer in Chain-Auflösung, Swap nach
jedem Pass (Selbst-/Vorwärts-Referenz = Vorframe, Rückwärts = frisch —
Original-Semantik), Buffer-Wrapper ohne Blend/Clamp, Import löst die
Buffer-Topologie über Output-Ids auf (common in allen Pässen), Panel mit
Bindungs-Combos + Buffer-Editoren. Sonde `shadertoy_feedback.lvfx`:
Lissajous-Trail beweist das Vorframe-Lesen (mean 0,015, ohne Ping-Pong
wäre es ein Einzelpunkt ~0,001). Sichttest (Patrik) offen. Tests 509.

**Fertig-Kriterium S:** MVP S1+S2 mit Sonden grün und einem
audioreaktiven Beispiel im Vorlagen-Bestand; S3 mit einem eigenen
API-Testshader belegt; Grenzenliste im Panel/Report sichtbar.

**Stand S65 (S1+S2 umgesetzt):** Chain-Node `shadertoy` (host-nativ wie
Reaction Diffusion — die eigene Visualizer-Klasse entfiel bewusst, „ein
Fragment-Pass, FBO in Chain-Auflösung" IST die Chain-Surface) +
`ShadertoyWrapper.hpp` (GL-frei: Uniform-Satz, `#line 1` für Nutzer-Zeilen
in Kompilierfehlern, Blend-Epilog, eigener Starter-Shader). Audio: geteilte
512×2-R8-Textur (Zeile 0 FFT, Zeile 1 Waveform) am wählbaren iChannel +
bass/mid/treb/vol/beat-Uniforms. Panel: Code-Editor (mono), Fehleranzeige
(`shadertoyError()`), Audio-Kanal-/Blend-Combos, Metadaten-Anzeige.
Sonden `asset/effectchain/shadertoy_{uvgrad,zeit,audioringe}.lvfx`:
uvgrad punktgenau (0,500/0,500/0,251 ≈ Soll), Orientierung y-up korrekt
(Sichtkontrolle), Zeit-Puls deterministisch (Sim-Uhr), Ringe audio-moduliert
— `out/shadertoy_sonden_s65/SONDEN_ERGEBNIS.md`. Tests 502.
OFFEN: Audio-Skala-Feinabstimmung (dB vs. linear, PORT-Annahme dokumentiert),
Audio-Mod-Skript-Slot (S2-Ausbaustufe), Sichttest mit Ton (Patrik).

**Stand S65 — S3 code-komplett (Netz-Abnahme offen):**
`ShadertoyImport.hpp` (netz-/GL-frei, voll getestet): ID-Extraktion aus
URL/Roh-Eingabe, API-/Query-/Thumbnail-URLs, Antwort-Parser (image+common
übernommen, common vorangestellt; music-Input → Audio-iChannel; Textur/Video/
Cubemap ⇒ Platzhalter-Meldung; Multipass ⇒ S4-Hinweis; Metadaten + Lizenz-
Default CC BY-NC-SA). Node-Editor: URL/ID-Feld + „Importieren" (GET nur auf
Knopfdruck, 10-s-Timeout) + 🌐-Knopf (Shader-Seite bzw. shadertoy.com) +
API-Key-Feld (QSettings `shadertoy/apiKey`, lokal — nie im Preset/Repo).
NEU dazu (Wunsch Patrik): **ShadertoyBrowserPanel** (Dock-Panel
„Shadertoy Browser", ImportBrowser-Muster) — Suche + Sortierung
(Beliebt/Neu/Hot/Geliebt) über die Query-API, Thumbnail-Grid (offizielle
Vorschaubilder, Laufzeit-Fetch), Doppelklick lädt den Shader als
Ein-Node-Chain (AppData/shadertoy/<id>.lvfx, LoadEffectChainEvent — dieselbe
Orchestrierung wie der Import Browser). WICHTIG: API nutzt App-Key, KEINE
Login-Daten — sichtbar ist nur Sichtbarkeit „public+api"; private Shader
bleiben Code-Einfügen. Qt6-Component `Network` in Solution.json ergänzt.
ABNAHME OFFEN: ein echter API-Lauf mit Key (Patrik) + Panel-Sichtprüfung.

---

## Strang G — GPU-Vertex-Module (im Zuge der Vereinheitlichung)

**Ziel:** GPU-Vertex-Arbeit dort, wo sie hingehört — NEUE Module unter
modernem Regelwerk. Legacy-Imports bleiben CPU-Mesh (Sequenz-Vertrag des
EEL, §7-Notiz „GPU-Vertex-Module").

### G1 — Mesh-Warp-Modul ✅ (Session 69, Sichttest offen)

- Warp-Funktion als GLSL im Vertex-Shader, Gitterauflösung frei (bis
  256×192 — GPU skaliert), Parameter + Audio als Uniforms, Presets über
  die Config-Pipeline (PipelineStage-Schema wie alle Module).
- Startpunkt: der bestehende Warp-Mesh-Vertexpfad des MilkdropVisualizers
  als Vorbild, aber zustandslos-parallel definiert.

**Stand S69 (umgesetzt):** Chain-Node `meshWarp` (host-nativ, RD-/Shadertoy-
Muster statt Config-Pipeline-Visualizer — dieselbe Revision des Plans wie
beim Shadertoy-Node S65). `MeshWarpWrapper.hpp` (GL-frei: Vertex-Wrapper mit
`#line`-Vertrag, Fragment mit Wrap/Mix IM Shader, Gitter-Erzeugung, Klemmen
2..256×192) + `runMeshWarp` (transformPass-Muster, Programm-/Gitter-Rebuild
nur bei Wechsel, Quell-Filter LINEAR nur für den Draw, danach restauriert) +
Panel-Sektion (GLSL-Groß-Editor mit Apply/Beautify/Import/Export S69,
Fehler-Poll über das geteilte `stError`) + Parameter-Skripte (`gridx`,
`gridy`, `mixamount`). **Entscheid Patrik (S69): G1 VOR Vereinheitlichung
V2** — Audio ad-hoc als Uniforms (Shadertoy-Muster); V2 zentralisiert später
nur die Quelle, der Uniform-Vertrag bleibt. Sichtbeweis
`asset/effectchain/meshwarp_sonde.lvfx` (`out/meshwarp_sonde_s69/`,
Warnungen=0); Tests 547 (+9). Sichttest Patrik offen.

### G2 — GPU-Partikel-Modul

- Instancing (ein Draw, N Instanzen) + Zustand in Ping-Pong-Texturen oder
  Transform-Feedback; Spawn/Kraftfeld/Lebensdauer als Parameter, Audio-
  Kopplung über Uniforms + Audio-Mod-Slot (aus S2 wiederverwendet).
- Die „moderne Antwort" auf die searchlight-Klasse: zehntausende Partikel
  statt 800×16-gmegabuf-Schleifen.

### G3 — Kür (nur Notiz, kein Auftrag)

- „Mesh-Qualität"-Option für Legacy-Imports: NUR wenn der Transpiler das
  per_pixel-Skript als ZUSTANDSLOS beweist (kein reg/gmegabuf/rand),
  optional GPU-Auswertung mit feinerem Gitter — als Modern-Schalter mit
  dokumentierter Abweichung.

**Einordnung (revidiert S69, Entscheid Patrik):** ursprünglich „G startet
erst mit der Vereinheitlichung (V2–V5)" — G1 ist stattdessen DIREKT
gestartet: Audio ad-hoc als Uniforms nach dem erprobten Shadertoy-Muster
(MilkLoudness/BeatEstimator-Bausteine liegen bereit), V2 zentralisiert
später nur die Quelle hinter demselben Uniform-Vertrag. V2–V5 bleiben als
eigener Punkt offen (Offene_Punkte §6).

---

## Reihenfolge & Abhängigkeiten

| # | Strang | Voraussetzung | Session-Schätzung |
|---|---|---|---|
| 1 | **R** Regelwerk | — (Auftrag nächste Session) | 1 Session |
| 2 | **S1–S2** Shadertoy-MVP + Audio | R (nutzt „modern") | 1 Session |
| 3 | **S3** URL-Import | S1; API-Key Patrik | 0,5 Session |
| 4 | **S4** Multipass | S1–S3 | 0,5–1 Session |
| 5 | **G1/G2** GPU-Module | Vereinheitlichung V2+ | je ~1 Session |

Parallel weiter: Rest-11-Kalibrierung (§3) — unabhängig von R/S/G.

## Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.5.0 | 2026-08-06 | Strang G1 umgesetzt (Session 69): Chain-Node `meshWarp` host-nativ (MeshWarpWrapper.hpp GL-frei, runMeshWarp im transformPass-Muster, Panel-GLSL-Editor mit Apply/Beautify/Import/Export, Parameter-Skripte). Einordnung revidiert (Entscheid Patrik): G1 VOR Vereinheitlichung V2 — Audio ad-hoc als Uniforms, Shadertoy-Muster. Sonde meshwarp_sonde.lvfx, Tests 547. G2 + Sichttest offen |
| 1.4.0 | 2026-08-02 | Strang S4 umgesetzt (Session 65) — Strang S damit KOMPLETT: ShadertoyPass + Kanal-Bindungs-Kodierung (SSOT, audioChannel-Feld entfernt → Lese-Migration), RGBA32F-Ping-Pong je Buffer (Swap nach jedem Pass = Original-Lese-Semantik), Buffer-Wrapper roh, Import mit Buffer-Topologie (Output-Ids, common überall), Panel-Bindungs-Combos + Buffer-Editoren, Sonde shadertoy_feedback (Lissajous-Trail = Vorframe-Beweis). Tests 509 |
| 1.3.0 | 2026-08-02 | Strang S3 code-komplett (Session 65): ShadertoyImport.hpp (ID/URL/Query/Thumbnail + Antwort-Parser, netz-/GL-frei getestet), Node-Editor-Import (URL-Feld, API-Key in QSettings, 🌐-Knopf) + NEU ShadertoyBrowserPanel (Dock-Panel: Query-Suche + Sortierung + Thumbnail-Grid, Doppelklick lädt via AppData-.lvfx + LoadEffectChainEvent). Klärung: API = App-Key, keine Login-Daten, nur „public+api". Qt6 `Network` ergänzt. Netz-Abnahme (echter Key) + Sichtprüfung offen |
| 1.2.0 | 2026-08-02 | Strang S1+S2 umgesetzt (Session 65): Chain-Node `shadertoy` host-nativ (Entscheid: keine eigene Visualizer-Klasse — RD-Muster), ShadertoyWrapper.hpp (GL-frei, #line-Vertrag, Blend-Epilog, eigener Starter), 512×2-Audio-iChannel + LumiViz-Uniforms, Panel-Editor mit Fehleranzeige, drei Sonden-Vorlagen (uvgrad punktgenau, y-up bestätigt, Zeit deterministisch, Ringe audio-moduliert). S3 wartet auf API-Key |
| 1.1.0 | 2026-08-02 | Strang R umgesetzt (Session 65): R1 Schema (`Regelwerk` + 4 Schalter + PS-Overrides, `effektiveSchalter()`), R2 Serialisierung (MilkdropSerializer 1.1.0, Migration fehlend ⇒ Legacy+Auto), R3 Gates (HlslTranspiler 1.5.0 `d3d9Div`, `uTruncate`-Uniform, q-/UV-Gates, MD1erzwingen), R4 Panel (Combo+Schalter+PS-Combos, Revision-Bump), R5 Abnahme (Triage s65 = s64f klassengleich, Modern-Stichprobe mit belegten Deltas). Panel-Sichtprüfung offen |
| 1.0.0 | 2026-08-02 | Erstfassung (Session 64) — Entscheide Patrik: Regelwerk-Strang mit Master+Einzelschaltern und PS-Override; Shadertoy-Modul inkl. URL-/ID-Import und Audio-Nachrüstung; GPU-Vertex-Module als Vereinheitlichungs-Bürger |
