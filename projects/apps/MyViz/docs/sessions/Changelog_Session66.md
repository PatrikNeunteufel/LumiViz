# Changelog — Session 66 (2026-08-03)

> Anlass: Befund Patrik — dasselbe MilkDrop-Preset sieht je nach VORGÄNGER anders
> aus (S63-Feedback-Erbe des In-Place-Tauschs). Tests: **511 Cases grün, 0 Skips**.
> Builds: VS-Testing + VS-Debug + Ninja-Clang-Release grün.

## Neu

- **Puffer-Wechsel-Schalter** je Milkdrop-Node (Sektion Parameter): „Puffer bei
  Preset-Wechsel" ∈ App-Einstellung / Behalten (Original) / Löschen / Fading
  (einmaliger Mix, Erbe-Regler 0–100 %) / **Ausblenden (über Zeit,
  Sekunden-Regler 0,1–60)**. App-Default im Settings-Panel (Panels-Tab:
  „MilkDrop Preset Switch" + Fade Amount + Fade-out Time).
  - Behalten = Original-Semantik (Puffer erbt), Löschen = deterministische
    Rausch-Saat + frisch gewürfelter rand_preset-Seed (bewusst KEIN
    Loudness-Reset und ≠ Schwarz), Fading = einmaliger Mix Erbe/Saat,
    Ausblenden = Echo-Dämpfung je Frame nach dem Warp (exponentiell, nach der
    Dauer < 1/256) — frische Zeichnungen des neuen Presets bleiben.
  - Node-Einstellung überlebt das Preset-Durchblättern; Chain-Persistenz mit
    Migration (fehlend ⇒ App-Einstellung); Umstellen startet das laufende
    Preset nicht neu. Sichttest Behalten/Löschen bestanden, Ausblenden offen.
- **Benutzerhandbuch 1.5.1:** §11 „Preset-Wechsel: das geerbte Bild" (5-Modi-
  Tabelle, exakte Behalten-Liste, „Löschen ≠ Schwarz") + „Regelwerk" (S65) +
  NEU §12 Shadertoy (S65) + §8 GPU-Auswahl (S61) — mit **2 skriptgesteuert aus
  der App erzeugten Screenshots** (`docs/bilder/handbuch/`).
- **Panel-Komfort:** Effect-Chain-Splitter Baum/Editor (Stellung persistiert) +
  Mindestbreite 420 px · Panel-Startgröße ≥ 560×780 (PanelBase::sizeHint) ·
  Milkdrop-Sektionen in **Render-Pipeline-Reihenfolge** mit getrennten Sektionen
  Shader **Warp** / Shader **Comp** (Parameter → Code → Warp → Shapes → Waves →
  Comp → Sprites; Ids stabil, reine Anzeige) · **Container-Ausstieg:** ↑/↓ am
  Rand schiebt die Selektion eine Ebene hinaus (vor/hinter die Effect List).
- **Diagnose:** `loadDiag f1..f5` im MilkDrop-Trace (Audio-Futter der ersten
  fünf Frames nach jedem Preset-Load).

## Geändert / Fixes

- `/bigobj` für `MultiEffectVisualizer.cpp` (C1128 im Debug-Build) in beiden
  betroffenen Source.cmake (visualizers + AvsStandalone).
- FeedbackBuffer **1.1.0** (FBO-Handle-Getter für den Erbe-Mix) ·
  MilkdropVisualizer.md **1.21.0** · Feld-Inventar **727 Felder**, 0 Tooltip-
  Lücken · Offene_Punkte **1.35.0**.

## Offen (SSOT: Offene_Punkte.md)

- **TOP 1 nächste Session — Preset-Wechsel-Divergenz endgültig lösen** (Auftrag
  Patrik): Trotz bewiesener Saat/Seed-Löschung (Trace) sieht Patrik bei der
  Rock-The-House-Familie weiter Farb-Übernahme. Forschungsstand: loadDiag belegt
  massiv unterschiedliches Audio-Futter (Stille vs. Musik); RTHs Farben sind rein
  time-getrieben. Plan: headless GL-Wächter-Test (Frame-Hash Kaltstart vs.
  Wechsel bei deterministischem Audio) → kontrolliertes A/B mit TestAudio +
  DUMP_WARP → GPU-Verdächtige. Danach Sichttest Ausblenden + Abnahme.
- Meganode-Split-Strang vorgemerkt (Container umordenbar mit Pipeline-Wirkung,
  Fremd-Renderer im Feedback-Loop; Import-Default = Original-Reihenfolge).
