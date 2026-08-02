# Changelog — Session 65 (2026-08-02)

> Schwerpunkt: **Strang R (Regelwerk Legacy/Modern) komplett** · **Strang S
> (Shadertoy S1–S4) komplett** · Shadertoy-Browser-Panel · Editor-Komfort ·
> 100-Shader-Vorrat. Tests **509 grün, 0 Skips**.
> Plan-Doku: `../visuals/Regelwerk_und_Neue_Module_Plan.md` (1.0.0 → 1.4.0)

## Strang R — Regelwerk Legacy/Modern (R1–R5, abgenommen)

- **Schema (MilkdropPresetState 1.1.0):** `regelwerk` ∈ Legacy|Modern|
  Benutzerdefiniert (Import-/Migrations-Default LEGACY = alle S64-Emulationen an),
  Einzelschalter `divVertragD3d9` · `unormTrunkierung` · `qGarbageEpsilon` ·
  `uvSanitize` (nur bei Benutzerdefiniert wirksam), PS-Overrides `psWarp`/`psComp`
  ∈ Auto|PS2|PS3|MD1erzwingen. Renderer/Transpiler lesen NUR `effektiveSchalter()`.
- **Serialisierung (MilkdropSerializer 1.1.0):** Felder optional; fehlend ⇒
  Legacy+Auto — alle Bestands-.lvfx laden unverändert (Wächter-Tests).
- **Gates (R3):** HlslTranspiler **1.5.0** `TranspileOptions.d3d9Div` (false ⇒
  roher `/`; Goldens beider Emissionen; hpp-Versionshinker 1.3→1.5 behoben) ·
  MD1-Warp trunkiert über `uTruncate`-Uniform statt eingebacken · Custom-Warp-
  Epilog konditional · q-Epsilon/UV-Sanitize je Node-Zustand · MD1erzwingen setzt
  die Klassifikation je Stufe auf None bei ERHALTENEM Shader-Text (S64-Strip-
  Bisektion als App-Feature) · PS2/PS3 = reine Angabe (GLSL-Emission identisch).
- **Panel (R4):** Regelwerk-Combo + Schalter (nur bei Benutzerdefiniert aktiv,
  WYSIWYG-Übernahme beim Wechsel) + zwei PS-Combos, Tooltips je Fixklasse.
- **Abnahme (R5):** Voll-Triage `out/milkdrop_triage_s65` **klassengleich s64f**
  (311 Presets, 0 Wechsel, OK 295); Modern-Stichprobe `out/milkdrop_modern_s65`:
  NEON 0,199→0,000 (NaN-Schwarz = div-Vertrag) · riding 0,707→0,817 (Trunkierung) ·
  R211 regelwerkskonform über den q/UV-Weg; Bug-Fixes (gb003, infinity 2) unter
  Modern unberührt. NEU: `LUMIVIZ_MILKDROP_REGELWERK=modern|legacy`
  (Diagnose-Override im NOSEED-Muster). Doku: MilkdropVisualizer.md **1.19.0**.

## Strang S — Shadertoy-Modul (S1–S4, komplett)

- **Node (S1):** Chain-Typ `shadertoy` host-nativ (Reaction-Diffusion-Muster;
  Entscheid: keine eigene Visualizer-Klasse). `ShadertoyWrapper.hpp` (1.1.0,
  GL-frei): voller Shadertoy-Uniform-Satz + LumiViz-Extras, `#line 1` ⇒
  Kompilierfehler mit NUTZER-Zeilennummern, Blend-Epilog; Kompilierung nur bei
  Code-Wechsel, Fehler ⇒ Passthrough + `shadertoyError()` im Panel.
- **Audio (S2):** 512×2-R8-Textur (Zeile 0 FFT, Zeile 1 Waveform) am wählbaren
  iChannel + `bass/mid/treb/vol/beat`-Uniforms; iTime = deterministische Sim-Uhr.
- **URL-/ID-Import (S3, code-komplett):** `ShadertoyImport.hpp` (netz-/GL-frei,
  offline getestet) — ID-Extraktion, API-/Query-/Thumbnail-URLs, Antwort-Parser
  (common in allen Pässen, music→Audio-Kanal, Platzhalter-Meldungen); Editor mit
  URL-Feld, Importieren-Knopf (GET nur auf Knopfdruck), 🌐-Knopf; API-Key in
  QSettings `shadertoy/apiKey` (nur lokal). Klärung: App-Key statt Login,
  Sichtbarkeit „public+api". **Netz-Abnahme offen** (Patrik braucht erst den Key —
  dafür der Shader-Vorrat, s. u.).
- **Multipass (S4):** Buffer A–D als `ShadertoyPass` (Kanal-Kodierung −1/0..3/4;
  SSOT `imageInput`/`input`, `audioChannel` nur Lese-Migration), RGBA32F-Ping-Pong
  je Buffer, Swap nach jedem Pass = Original-Lese-Semantik, Import löst die
  Buffer-Topologie über Output-Ids; Panel mit Bindungs-Combos + Buffer-Editoren.
  Sonde `shadertoy_feedback.lvfx` (Lissajous-Trail = Vorframe-Beweis).
- **ShadertoyBrowserPanel** (neues Dock-Panel „Shadertoy Browser"): Query-Suche +
  Sortierung (Beliebt/Neu/Hot/Geliebt), Thumbnail-Grid (offizielle Vorschaubilder,
  Laufzeit-Fetch), Doppelklick lädt als Ein-Node-Chain (AppData/shadertoy/<id>.lvfx
  → LoadEffectChainEvent). **Solution.json: Qt6-Component `Network` ergänzt.**
- Sonden-Abnahme: `out/shadertoy_sonden_s65/SONDEN_ERGEBNIS.md` — uvgrad punktgenau
  (0,500/0,500/0,251), Bildorientierung y-up korrekt, Zeit-Puls deterministisch,
  Audioringe wave-moduliert. Doku: ShadertoyWrapper.md (neu, 1.1.0).

## Editor-Komfort + UI

- **`addCodeEditor`** (ⓘ-Referenz + ⤢-Groß-Editor + Tooltip) für die GLSL-Felder;
  neue **Shadertoy-Referenzseite** (Uniforms, Audio-Sample-Formeln,
  Buffer-Semantik, Portabilitäts-Warnung). Milkdrop-HLSL-Editoren: **⤢ ergänzt**
  (ⓘ seit S42). `openScriptEditor` mit `eelHighlight`-Schalter (kein EEL-Highlight
  auf HLSL/GLSL). EEL-Felder der Bestandsmodule hatten den Komfort bereits.
- **Speichern-Dialog** (Save Effect Chain) schlägt den Namen des zuletzt
  geladenen/importierten Presets vor (.avs/.milk-Import speichert als
  gleichnamige .lvfx; SSOT: ScreenshotManager).

## Shader-Vorrat (für Patriks Shadertoy-Konto → API-Key)

- **`asset/shadertoys/`: 100 eigene, portable Shader** (113 .glsl, 13 Multipass-
  Paare; NUR Standard-Uniforms ⇒ 1:1 auf shadertoy.com): Basis 01–20 ·
  Experten 21–40 (Mandelbulb, Menger, Ozean, Schwarzes Loch, Volumetrik …) ·
  Themen 41–80 (Game-of-Life-Ableger, Biologie, Feuerwerk, Technik, Kristalle,
  Glow) · Kombinationen 81–100. Jede Datei mit **STELLSCHRAUBEN-Konstantenblock**
  (Feedback Patrik) + Zeilenkommentaren; README mit Serien-Tabellen + Workflow +
  Lizenz-Empfehlung. `make_lvfx.py` generiert die Test-Vorlagen nach
  `asset/effectchain/shadertoys/` (.glsl = SSOT). **Batch-Gate 100/100 kompiliert,
  0 Schwarz** (Montagen `out/shadertoy_vorrat_s65/`). Zwei Feedback-Runden
  eingearbeitet (harte Kanten, Rotation, Audio-Tempi, Fenster-Zufall u. a.).

## Sonstiges

- Feld-Inventar 86 Typen / 724 Felder + Feld-Tooltips regeneriert (0 Lücken).
- Tests 493 → **509** (Regelwerk, Serializer-Wächter, Transpiler-Doppel-Emission,
  Wrapper/Import/Query, Buffer-Roundtrip + Migration).
- Offen: Sichttests der neuen UI (Patrik) · Netz-Abnahme S3 mit echtem Key ·
  Audio-Skala dB-vs-linear · Shader-Feintuning nach Ton-Test (vorgemerkt: 45/56/64).
