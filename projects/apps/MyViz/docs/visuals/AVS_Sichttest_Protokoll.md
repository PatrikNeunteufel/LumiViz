# AVS-Sichttest-Protokoll — Kalibrier-Runde (SSOT Punkt 9)

> **Version:** 0.5.0 (wird laufend nachgeführt)
> **Datum:** 2026-07-24 (Session 44)
> **Typ:** Arbeitsprotokoll
> **Status:** In Arbeit
> **Sprache:** Deutsch
> **Zweck:** EIN Ort für „welches Preset wurde gesichtet, was war der Befund,
> welche Korrektur folgt daraus" — je Preset-Sammlung eine Tabelle.
> Werkzeuge: In-App-Import (Sichttest Patrik) + `AvsStandalone [pfad] [--auto]
> [--dump]` (Pixel-Statistik, Warnungen, Ketten-Bisektion).
> **Bezug:** [MilkDrop_Import_Status.md](MilkDrop_Import_Status.md) Punkt 9
> (Kalibrier-Runde; dort auch die 10 schwarzen ref-Korpus-Presets).

---

## 1. Systematische Befunde (sammlungsübergreifend)

| # | Befund | Ursache (belegt) | Korrektur | Status |
|---|---|---|---|---|
| S1 | Presets melden beim Import `"Movement" not supported yet — passthrough` | Movement-Builtin-Tabelle (`AvsChainTranslator.cpp:141`) hat `nullptr` für Index **1 „slight fuzzify"** und **7 „blocky partial out"** — im Original kein `eval_desc`, sondern Spezial-C-Code: 1 = statische 3×3-Zufallsverschiebung (einmal je Fenstergröße, r_trans.cpp:316-323), 7 = 2×2-Blöcke im 4×4-Raster samplen aus 7/8-Zoom (r_trans.cpp:339-358) | ✅ **gefixt (S44):** `MovementParams.builtinRemap` + dedizierter Per-Pixel-Shader (`kMoveRemapFragmentShader`; fuzzify als statischer Positions-Hash statt rand-Tabelle — gleiches Bild, deterministisch); Subpixel für 1/2/7 im Translator abgeschaltet (wie r_trans.cpp:308); Gates: Serializer-Roundtrip + Translator-Test; alle 5 JC-Presets warnungsfrei. **Offen: Sichtvergleich fuzzify/blocky gegen echtes AVS (Patrik)** | ✅ |
| S2 | `d`/`r` der Movement-Skripte im NDC- statt PIXEL-Raum (aspektabhängig verzerrt) | r_dmove.cpp:307-329 (Befund Session 43, Wormhole) | Koordinaten im Pixel-Raum rechnen | ⬜ offen |
| S3 | Set-Render-Mode-Zustand persistiert bei uns über Frames/Listen | Original setzt `g_line_blend_mode` je Frame zurück + rettet ihn um Listen (r_list.cpp:433/440/693-694) | Reset je Frame + Save/Restore um Listen | ⬜ offen |
| S4 | **CRASH (0xC000041D) bei 6 JC-Presets im Standalone**, erster Frame | `runDotPlane`: bei leerem Spektrum schützte der `sl=1`-Guard nur die Division, `spec[0]` las trotzdem den **leeren** Vektor (`MultiEffectVisualizer.cpp:5121`); leer war das Spektrum wegen S5. Bisektion: Minimal-`.lvfx` nur mit Dot Plane crasht | ✅ **gefixt (S44):** Empty-Guard (Stille → flache Ebene); Verifikation: alle 6 + Minimal-Kette laufen, exit 0 | ✅ |
| S5 | **Mono-Audio-Puffer leer im Chain-/Standalone-Pfad** — alle Effekte, die `getSpectrum()`/`getWaveform()` (mono) lesen, sahen Stille: `computeAudioBands` → bass/mid/treb=0, Dot Plane flach, RMS=0 (`m_audioLevel`) | AVS-Zwilling des S43-Milk-Loudness-Bugs: `feedSyntheticAudio` (AvsStandalone) und der Chain-Pfad füttern nur `updateAudioStereo`; die Kanal-Getter fielen auf Mono zurück, die Mono-Getter aber **nicht** auf den Stereo-Mix | ✅ **gefixt (S44):** `getSpectrum()`/`getWaveform()` liefern bei leerem Mono-Puffer den L/R-Mix (`VisualizerBase.cpp`) — symmetrisch zu den Kanal-Gettern | ✅ |
| S6 | `"SVP Loader" not decoded — passthrough` (when i come around.avs) | SVP/UVS-Render-Plugin = externe Binär-DLL — nicht decodierbar, Passthrough ist korrekt | keine (bekannte Grenze; ggf. Doku) | — |
| S7 | **„Weiß/Grau-Sättigung":** Presets konvergieren zu uniformem Grau (Standalone, 0.502 = 128/255) bzw. Weiß (in-app) | Prefix-Bisektion „don't make a mess": Verursacher ist eine **Effect List mit blendIn=Xor, blendOut=50/50** (Movement-Zoom + OscStar innen) — allein gerendert konvergiert sie zu min=max=0.502. Code-Review dazu: `runList` ist strukturell referenztreu (persistenter Listen-Puffer = `thisfb`, In-Blend gegen Vorframe-Inhalt wie r_list.cpp:585-687, XOR bitweise + kommutativ). XOR-Feedback + Bilinear-Zoom kann auch im Original flächig konvergieren; in-app Weiß vs. Standalone Grau erklärt sich durchs unterschiedliche Audio | **Sichtvergleich gegen echtes AVS/Winamp in Bewegung (Patrik)** — erst danach entscheiden, ob hier überhaupt ein Bug liegt; ggf. dann Detail-Diff (Rundung/Clamp im 50/50, Bilinear vs. MMX-Subpixel) | ⬜ Urteil offen |
| S8 | Alias-APEs in Alt-Presets (Format 0.1) blieben Passthrough trotz decodierter Felder — z. B. „Winamp Starfield v1"/„Winamp Mosaic v1" in Spacefolding | AvsParser ließ bei Alias-Auflösung die Roh-ID stehen (0.1: Pointer-Wert); der Chain-Translator dispatcht auf `id` | ✅ **gefixt (S44):** Parser schreibt `child.id` auf den Builtin-Index um (AvsParser 1.2.0); Spacefolding rendert (0 Warnungen, Luma bis 1.0) — **Korpus-Schwarz-Liste damit 10 → 0** | ✅ |

**Wichtige Folge von S5:** Die S43-Liste „10 schwarze ref-Korpus-Presets"
(SSOT Punkt 9) ist womöglich teilweise ein **Standalone-Artefakt** — mit
leerem Mono-Audio waren alle spektrum-getriebenen Effekte tot. Nach dem Fix
neu sweepen und die Liste bereinigen.

## 2. Sammlung „JC-big stuff" (100 Presets)

**Pfad:** `..\..\VisualsPresets\avs\JC-big stuff` (seit S44 außerhalb des
Repos: `…\Visuals_Project\cmake\VisualsPresets` — Backup-Entscheid Patrik)
· **Sichtung:** In-App
(Patrik, Session 44, ohne Musik — Player „Ready") + AvsStandalone-Sweeps
(60 Frames, synthetisches Beat-Audio).

### 2.1 Sicht-Befunde In-App (Screenshots Session 44)

Import-Dialog gehört jeweils zum **ausgewählten** Preset (Statuszeile hinkt
einen Import nach). Movement-Passthrough-Notes im Pack: allockate (root/2),
strashen plochkarnik (root/3), crunchi munchi (root/4), floating in a cow
shit (root/5), I'm in (root/6).

| Preset | Import-Meldung | Sicht-Befund In-App | Einordnung |
|---|---|---|---|
| 5ver.avs | — | türkis-grüne Diagonalstrahlen + weißer Glow, lebendig | Standalone-Sweep 1 meldete „schwarz" → S5-Artefakt, nach Fix erneut prüfen |
| born without a face.avs | — | blaues Blocky-Muster, lebendig | plausibel — Referenzvergleich später |
| cosmic cloud in front of a sun.avs | — | dunkel, weißer Glow, feine Punkte | Standalone crashte (S4, gefixt); in-app lief es, weil Mono-Audio dort gefüllt ist |
| crunchi munchi.avs | 1 Note: Movement root/4 (S1) | dunkelrot, rosa-weißer Glow | S1-Wirkung prüfen |
| don't make a mess.avs | — | **fast einfarbig grauweiß** | verdächtig — Bisektion (`--dump` → Teil-Ketten) |
| don't try to aphect ME.avs | — | **fast komplett weiß** | verdächtig (Weiß-Sättigung) — Bisektion |
| fire in the hole.avs | — | dunkelrote Wirbel-Struktur | plausibel |
| greenandblackinamix.avs | — | grüne Blobs auf Schwarz | Sweep 1 „schwarz" → S5-Artefakt, nach Fix prüfen |
| how much 4 the cool glowin thingi.avs | — | **fast komplett weiß** | verdächtig — Bisektion |

**Muster-Verdacht „Weiß-Sättigung"** (3 Presets fast weiß): Kandidaten —
additive Blends ohne Gegenspieler, Set-Render-Mode-Persistenz (S3), fehlende
Movement-Dämpfung (S1). Per Bisektion eingrenzen.

### 2.2 AvsStandalone-Sweep 1 (VOR den S4/S5-Fixes — Baseline)

- **100/100 geladen**, davon **6 CRASH** (S4): allockate, cosmic cloud in
  front of a sun, floating in a cow shit, is this where they fry the eggs,
  sunned CD, when calm like a god — alle enthalten Dot Plane.
- **15/94 „schwarz"** (Luma-Statistik): 5ver, Curriculum, don't know, drop
  the hate, feel the beat, fluggish, greenandblackinamix, janx spirit
  (comment in), jashe, JFB, JFB1, Mould-ification, shampoo, Sheih's worst
  nightmare, xtra — **Verdacht: großteils S5-Artefakte** (5ver +
  greenandblackinamix waren in-app nachweislich lebendig).
- **Warnungen:** nur die 5 Movement-Notes (S1) + 1× SVP Loader (S6).

### 2.3 AvsStandalone-Sweep 2 (NACH den Fixes)

- **100/100 laufen durch, 0 Crashes** (S4/S5-Fixes wirksam).
- Schwarz-Liste 15 → 2 (fluggish, greenandblackinamix) → **0**: die beiden
  (und 5ver) sind mit `--frames 600` nachweislich lebendig — reine
  **Aufbauzeit** (Feedback-Akkumulation). greenandblackinamix zeigt im
  Standalone dieselben spärlichen grünen Blobs wie in-app.
- **Methodik-Merkregel:** `--frames 60/120` ist für die Schwarz-Erkennung zu
  kurz — langsam aufbauende Presets brauchen **≥ 600 Frames (10 s)**. Das
  betrifft auch die S43-Erhebung der „10 schwarzen ref-Korpus-Presets"
  (120 Frames + leeres Mono-Audio) → Liste neu erheben.

**Stand JC-big stuff nach Sweep 2:** kein Crash, kein Schwarz-Fall. Offene
Qualität: S1 (Movement 1/7) bei 5 Presets, S6 (SVP Loader) bei 1, die drei
„Weiß-Sättigungs"-Verdachtsfälle aus §2.1 (Bisektion ausstehend) und der
generelle Treue-Feinvergleich (S2/S3) gegen echtes AVS.

## 3. Sammlung ref-Korpus (vis_avs, 35 Presets) — Re-Sweep nach S5-Fix

`AvsStandalone --auto --frames 600` (2026-07-24, nach S4/S5-Fixes):

- **35/35 laufen, 0 Crashes, Schwarz-Liste 10 → 1.** Die 9 wieder lebendigen
  (Lightspeed, Trance Travel, bouncing colorfade, fireworks kaliadascope,
  too much sex, Living power ball, Nuclear Blobs, new taste, pulsing shit)
  waren S5-/Aufbauzeit-Artefakte der S43-Erhebung (120 Frames, leeres
  Mono-Audio).
- **Letzter Schwarz-Fall „lone — Spacefolding inside a Black Hole": ✅ gelöst
  (S8)** — kein fehlendes Modul, sondern die Alias-ID-Lücke des Parsers
  (Starfield/Mosaic existierten längst als Module + Translator-Cases). Nach
  AvsParser 1.2.0: 0 Warnungen, rendert mit voller Dynamik →
  **Schwarz-Liste 10 → 0.**
- Arkanoid Legent: weiterhin 2 Warnungen (bekannt aus S43), rendert.

## 4. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.1.0 | 2026-07-24 | Angelegt (Session 44): Systematik S1–S3, JC-big stuff In-App-Erstsichtung (9 Presets), Sweep ausstehend |
| 0.2.0 | 2026-07-24 | Sweep 1 (Baseline) + Crash-Bisektion: S4 (Dot-Plane-OOB bei leerem Spektrum) und S5 (Mono-Getter ohne Stereo-Mix-Fallback) gefunden UND gefixt; S6 SVP Loader notiert; Dialog-Zuordnung §2.1 korrigiert (Dialog = ausgewähltes Preset) |
| 0.3.0 | 2026-07-24 | Sweep 2: 100/100 ohne Crash; Schwarz-Liste 15→0 (13× S5-Artefakt, 2× Aufbauzeit — Methodik-Merkregel ≥600 Frames); JC-Pack damit crash- und schwarzfrei, Rest = S1/S2/S3 + Weiß-Sättigungs-Bisektion |
| 0.4.0 | 2026-07-24 | ref-Korpus-Re-Sweep (§3): Schwarz-Liste 10→1 — einziger echter Fall Spacefolding (Quelle „Starfield" fehlt, + „Mosaic"); Tests 409/409 grün, VS-Debug + VS-Testing + Ninja-Release grün über den S4/S5-Fixes |
| 0.5.0 | 2026-07-24 | **S1 gefixt** (builtinRemap-Shader für fuzzify/blocky, 5 JC-Presets warnungsfrei) · **S8 gefunden + gefixt** (AvsParser 1.2.0: Alias-ID-Umschreibung — Spacefolding rendert, Korpus-Schwarz 10→0) · **S7 lokalisiert** (Prefix-Bisektion: XOR-in/50:50-out-Liste konvergiert zu Uniform-Grau — r_list-Referenzvergleich als nächster Schritt); Suite grün nach jedem Schritt |
