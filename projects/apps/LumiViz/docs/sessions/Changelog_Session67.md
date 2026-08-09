# Changelog — Session 67 (2026-08-04)

> Anlass: Tiefenanalyse „entscheidender Unterschied zu aa9dc9d0" (TOP 1 aus
> S66: „Rock The House erbt Farbe trotz bewiesener Löschung"). Tests:
> **512 Cases grün, 0 Skips**. Builds: VS-Testing + VS-Debug +
> Ninja-Clang-Release grün. Die MilkDrop-Rest-Bugliste ist damit **leer**.

## Gelöst

- **TOP 1 „RTH erbt Farbe": Shader-Programm-Leichen beim In-Place-Wechsel.**
  `ensureCustomPrograms()` stand hinter dem Gate „nur wenn Custom-Quellen
  vorhanden" — beim Wechsel auf ein Md1Default-/None-klassifiziertes Preset
  (z. B. `Rock The House_2024`) rendeten die Warp-/Comp-GL-Programme des
  VORGÄNGERS weiter (nach Spotlight rot, nach Beauty weiß-aufhellend). Bei
  aa9dc9d0 unsichtbar (jeder Wechsel = frische Instanz), in der 311er-Triage
  unsichtbar (ein Preset je Lauf). Fix: Aufruf unconditional. Beweis: neuer
  `--ab`-Wechsellauf — vorher 0/300 gleiche Frames bei verschiedenen
  Vorgängern, nachher 300/300 bitgleich. Sichttest Patrik ✅.
- **Fixklasse 9 — EEL-Divisions-Vertrag (OK 295→298):** Die Referenz-EEL
  dividiert safe (Nenner 0 ⇒ exakt 0; dreifach per Sonde gegen MilkdropRef
  gemessen), unsere Lua-Engine rechnete IEEE (inf/NaN → NaN-Kaskaden).
  EelTranspiler 1.3.0 emittiert `eel.div` — nur Milkdrop-Dialekt; der
  AVS-Vertrag wurde separat gemessen (AvsRef-Sonde: 2/0 = inf wie unsere rohe
  Division ⇒ AVS bleibt korrekt unangetastet). **pixies party searchlight +
  reworked 2000 shapes von SCHWARZ geheilt** (NaN-Kamera-Matrix ab Frame 0).
- **Voll-Triage `out/milkdrop_triage_s67b`:** exakt 3 Klassenwechsel vs. s64f,
  alle aufwärts, 0 Regressionen; `VERGLEICH.md` mit frischen Referenz-Shots.

## Neu

- **Sicht-Blende** („MilkDrop Start Fade-in", Settings-Panel, Default an):
  Nach jeder frischen Rausch-Saat (App-Start, Resize, Löschen-/Fading-Wechsel)
  blendet das Bild ~0,5 s mit Ease-in² von Schwarz ein — rein kosmetisch, die
  Preset-Dynamik und die Saat-Energie bleiben unangetastet (f60-Hash mit/ohne
  identisch). Kern-Default aus ⇒ Prüfstände/Triage byte-unverändert.
  Benutzerhandbuch 1.6.0 (§11).
- **Shadertoy-Audio-Skala dB** (Sonde dB-vs-linear ✅): FFT-Zeile der
  512×2-Audio-Textur nach WebAudio-Vertrag (Magnitude-Glättung τ=0,8,
  20·log10 auf [−100,−30] dB) — leise Spektral-Ausläufer (Magnitude 0,001)
  waren linear Byte 0, in dB Byte 146; Sichtbeweis `01_audio_ringpulse`
  mean 0,116→0,294. ShadertoyWrapper.md 1.2.0.
- **`getspecdb(band, breite, kanal)`** (Idee Patrik): `getspec` auf derselben
  WebAudio-dB-Skala, in ALLEN Chain-Skripten (Builtin + Transpiler-Whitelist +
  Editor-Referenz/Highlight) — als zusätzliches Audiosignal für eigene Effekte.
- **Werkzeuge:** MilkdropStandalone 1.1.0 (`--ab`, `--wechsel`, `--ab-frames`,
  `--audio-neustart`, `--blende`, `--audio-beat`) ·
  `LUMIVIZ_MILKDROP_TRACE_VARS` (Variablen-Trace nach per_frame) ·
  EEL-Sonden-Methodik (`asset/Milkdrop3/sonden/` untracked +
  `asset/calibration/avs/sonden/` mit binärem .avs-Generator) ·
  MilkdropRef-MessageBoxen → stderr (`patched/ref_msgbox.h`).

## Einstufungen / Entscheide

- **Dunkelklasse (7+1) = IST-SO-artig** (Entscheid Patrik): purple pulsator,
  crystal palace, R039, R068, R070, R211, R248 + XorDev 001b (invers: Ref
  stirbt an eigenem UB, wir leben) — Look hängt am doppelten UB der Referenz.
  ⚪ Optional: Dossiers → gezielte Emulation nur bei freien Ressourcen.
- **piercing 01 = Saatlos-Vertragsgrenze** (kein Port-Bug): Rausch-Input unter
  der 1/255-Quantisierung, stirbt an der UNORM-Trunkierung (auch im Original);
  mit Saat/App-Pfad gesund (0,406 ≈ Ref 0,306).
- **Gin Tonic 003 — 🟡 Vorschlag zur Absegnung:** `sin(90·bass)` macht das
  Bild-Regime zentistellen-empfindlich; beide Renderer besitzen beide Regimes
  (Ref unter Stille selbst weiß) ⇒ Audio-Statistik-Vertragsgrenze.
- **Loudness bei Stille ✅ geprüft:** Ref ≈1,0 (Bracket 0,9–1,1), wir exakt
  1,000 — äquivalent; 0/0-Band-Differenz-Risiko seit `eel.div` entschärft.
- **myPresets (41) ✅ triagiert:** 27 OK, 14 auffällig = eigene
  GreatWho-Energie-/Verstärker-Klasse (Referenz saatlos ebenso dunkel) —
  keine Port-Bugs.

## Doku

MilkdropVisualizer.md 1.24.0 · MultiEffectVisualizer.md · EelTranspiler.md
1.3.0 · ShadertoyWrapper.md 1.2.0 · Benutzerhandbuch 1.6.0 · MilkdropRef-README
· Offene_Punkte 1.38.0.

## Offen

Strang G (GPU-Vertex-Module, nächste Session) · Gin-Tonic-Absegnung ·
UI-Sichttests + Ausblenden-Sichttest (Patrik) · Shadertoy-Konto → API-Key →
Netz-Abnahme S3 (inkl. dB-Absolut-Offset-Feinabgleich) · Meganode-Split-Konzept.
