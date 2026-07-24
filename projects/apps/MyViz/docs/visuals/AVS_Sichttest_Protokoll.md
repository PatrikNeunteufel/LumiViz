# AVS-Sichttest-Protokoll — Kalibrier-Runde (SSOT Punkt 9)

> **Version:** 0.12.0 (wird laufend nachgeführt)
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
| S9 | **„Wird weiß/zu hell"-Familie** (HISTORY-Pack u. a.): Presets sättigen zu Uniform-Weiß/-Gelb (min=max) | **Set-Render-Mode-Linien-Blends waren auf 3 Modi kollabiert:** AVS BLEND_LINE kennt 10 (r_defs.h:267-283 — 0 replace, 1 add, **2 MAX**, 3 avg, 4/5 sub, 6 mul, 7 adjustable, 8 xor, 9 min); Translator machte aus allem ≠0/≠3 „additiv" → der MAX-/Sub-Deckel vieler Presets fehlte (Beleg: ZeroG/Rotor = Modus 2, Ego = Modus 5, per Roh-Bit-Dump) | ✅ **umgesetzt (S44):** rohe Bits durchgereicht (Translator + Compile-Clamps 0..9), zentraler `applyLineBlend`-Helfer mit GL-Pendants (GL_MAX/GL_MIN/SUBTRACT/REVERSE_SUBTRACT, DST_COLOR-Mul, CONSTANT_ALPHA-Adjustable; XOR vorerst Additiv-Fallback), Panel-Enums auf 10 Einträge. **Aber:** ZeroG/Novae bleiben im Standalone weiß — dort wirken zusätzlich ColorMap-auf-Weiß-Mapping bzw. FastBright-Ketten; Urteil braucht Seite-an-Seite mit echtem AVS. Anwendung bisher: SuperScope + Clear — übrige Scope-Draw-Sites folgen zusammen mit S3 (SRM-Reset je Frame/Liste) | ✅ teilw. |
| S10 | **SuperScope: „Spektrum fehlt" / 3D-Formen wirken falsch** (EL-VIS6_SUPERSCOPES_3D, Befund Patrik) | AVS `which_ch` ist ein Bitfeld (r_sscope.cpp:232-240): Bits 0-1 = Kanal (0 L, 1 R, ≥2 Center), **Bit 4 = SPEKTRUM statt Waveform als `v`-Quelle**. Unser Translator las den Rohwert als LumiViz-Kanal-Enum: which_ch=4 wurde „Side" (L−R ≈ 0) mit Waveform — `v` blieb praktisch null (Beleg: first3d_spectrum flach trotz synthetischem Audio) | ✅ **gefixt (S44):** `SuperScopeParams.spectrumSource` (Bit 4) + Kanal aus Bits 0-1; Host schaltet `SuperscopeAudioSource` um; Serializer-Feld + Panel-Checkbox; Translator-Gate (which_ch 4/2/6). Sichtnachweis: Spektrum-Berge sichtbar. **Amplituden-Skala Spektrum ggf. sichtkalibrieren** (AVS-Bytes 0..255 vs. Modul 0..1) | ✅ |
| S11 | **BASS vs. Winamp-Audioanalyse** (Frage Patrik: „Eigenheiten von bass.dll berücksichtigen?" — Symptom: Spektrum wirkt einseitig, links flach) | Original-Pipeline liegt im Ref (`winamp_orig/Src/Winamp/VIS.cpp:719-745` + `FFT.cpp`): **512er-Real-FFT ohne Fensterung über 8-bit-Sample-Tops, Byte = sqrt(re²+im²)/16 LINEAR** (der Log kommt erst in AVS via g_logtab), 256 Bins → je 2 Positionen (0..511, leicht geglättet), **Positionen 512..575 = reine Abkling-Füllung (~0)**. Abgleich: (a) unser `kSpecGain=8` sättigt bei Magnitude 0,125 ≈ Winamps Sättigungspunkt 0,126 — Amplitude war schon original-nah (S38-Kalibrierung jetzt hergeleitet); das „flache Links" bei lauter Musik ist auch im Original gesättigt. (b) **Frequenzachse war falsch:** wir streckten 512 echte Bins über 576 Positionen (~12 % gestaucht + Phantomwerte in den Fade-Bändern) | ✅ **gefixt (S44):** Position p = unser Bin p (BASS-FFT1024 = exakt doppelte Winamp-Auflösung), Positionen ≥ 512 → 0 wie im Original. Offen/notiert: Winamp nutzt 8-bit-Tops + keine Fensterung (BASS float — feinere Dynamik, gleiches Leakage-Verhalten) — nur relevant, falls Feindifferenzen sichtbar bleiben | ✅ |
| S12 | **SuperScope-`v` blieb trotz S10/S11 „unverändert"** (Nachtest Patrik) | `v` wurde im Chain-Pfad aus den ROHEN Float-Arrays gerechnet (BASS-Magnituden 0..1, `sampleCount` = Waveform-Länge → 512er-Spektrum übers Ende indiziert) statt aus den visdata-Bytes. Original (r_sscope.cpp:284-289): **v = interpoliertes visdata-Byte/128 − 1 für BEIDE Quellen** — Spektrum-Stille ⇒ v = −1 (nicht 0!), Center-Kanal per char-Arithmetik | ✅ **gefixt (S44):** `SuperscopeModule::visdataValue()` — v im Lua-/Chain-Pfad exakt nach r_sscope (Quelle/Kanal auf den 576er-Blöcken, lineare Interpolation, XOR, /128−1, inkl. Center-char-Eigenheit); Float-Arrays bleiben Standalone-Preset-Pfad. Sichtnachweis first3d_spectrum: Zickzack-Teppich (sin(1)-Zähne bei Stille) + Musik-Modulation — Formel-treu; finale Bestätigung Seite-an-Seite | ✅ |

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

## 4. Sammlung „EL-VIS CYBERPUNX" (32 Presets)

**Pfad:** `…\VisualsPresets\avs\EL-VIS CYBERPUNX` · Sweep 600 Frames
(2026-07-24, nach allen S44-Fixes): **32/32 laufen, 0 Crashes, 1 schwarz.**

| Befund | Detail |
|---|---|
| el-vis_highvoltage (tribute to danjoe) schwarz | Bildquelle ist ein AVI-Video — ✅ **gelöst durch neuen AVI-Effekt (S44, s. u.):** rendert jetzt (die vier .avi der Sammlung liegen eine Ebene über den Preset-Ordnern und werden per Aufwärtssuche gefunden) |
| Text-Warnungen (cyberpunx ×1, thegoldenlemmons_flimmern ×3, golden ×1 „AVI") | ✅ **gelöst durch neuen Text-Effekt + AVI-Effekt (S44):** alle 4 Presets warnungsfrei |
| **NEU (S44): Text-Effekt (id 28) implementiert** | QPainter-Port von r_text.cpp: Wort-Zyklus (normSpeed/onBeat+Lockout, insertBlank, randomWord/randomPos), LOGFONT→QFont (Höhe/Gewicht/Kursiv/Unterstrichen/Face), H/V-Align + %-Shifts, Outline/Schatten, Blend nur auf Glyph-Pixeln (Alpha statt GDI-Farbkey). Parser decodiert CHOOSEFONTA/LOGFONTA-Rohblöcke (AvsParser 1.3.0) |
| **NEU (S44): AVI-Effekt (id 32) implementiert** | VfW-Pfad wie das Original (`AVIStreamGetFrame`, 32bpp-Ziel — Legacy-Codecs laufen), GPU-Stretch statt DrawDib, `speed`-ms-Drossel, Beat-`persist`-Fenster, `adapt`-Blend (Beat→additiv, sonst 50/50); Dateiauflösung per Aufwärtssuche ab Preset-Ordner (bis 4 Ebenen), Pfad wird beim Import in `resolvedPath` fixiert |
| **NEU (S44): Comment-Text wird übernommen** (Befund Patrik: „Import übernimmt keine Kommentare") | Parser decodiert id 21 (war undecodiert); Comment ist jetzt **eigener Node-Typ `CommentParams`** mit eigenem Mehrzeilen-Feld im Panel (Entscheid Patrik: NICHT in `description` — zerstört die Tabellen-Ansicht); persistiert als `"comment"` in `.lvfx` |
| **el-vis_rabbithole002 „zeigt zwischendurch keinen Tunnel"** (Patrik) | Code-Analyse: Tunnel-Sichtbarkeit hängt NICHT am Audio — der SuperScope führt eine Segment-Zustandsmaschine (`bw` zählt herunter, `new` spawnt zufällige neue Segmente mit `e=4+rand(5)` Kanten; Punkte mit `htt<-1.9` sind per `in`-Flag unsichtbar). **Zwischen zwei Segmenten ist der Tunnel konstruktionsbedingt weg** — sehr wahrscheinlich Original-Verhalten. Beat wirkt nur auf Helligkeit/Farbe (`bt=1` im Beat-Slot, Abkling 0,2/Frame, `col·(1+0.25·bt)`) |
| „Gefühl: nicht alle Audiosignale ausgewertet" (Patrik, unspezifisch) | visdata-Pfad geprüft: BASS-FFT ist linear (512 Bins → 576er-Mapping formtreu), Log-Kurve + kSpecGain=8 seit S38 kalibriert. Verbleibende Kandidaten: **Beat-Rate des BeatEstimators vs. Winamp** (bt-getriebene Presets wirken bei selteneren Beats träger) und Player-Zustand (ohne laufende Musik reagiert nichts). Vorschlag: `visdata:`-Trace analog zum Milk-Loudness-Trace + Beat-Raten-Gegenmessung |

## 5. Sammlung „EL-VIS HISTORY pRELOADED" — In-App-Sichtung Patrik (S44)

**Pfad:** `…\VisualsPresets\avs\EL-VIS HISTORY pRELOADED` · Sichtung ohne
Musik (Beats nur via Custom BPM). Nummerierung = Patriks Befundliste.

| # | Preset | Befund Patrik | Analyse-Stand |
|---|---|---|---|
| 1 | HpR01(Intro) | Text fehlt | **Aufgeklärt:** Text decodiert + rendert korrekt (Standalone-Beweis, Position exakt unten-mittig). In-app unsichtbar: 19-px-Text in Dunkelgrau `#454545` auf hellgrünem Grid + Panel hatte keinen Text-Editor (jetzt ergänzt). Nebenbefund behoben: Standalone-Screenshot las logische statt physischer Pixel (DPI-Crop) |
| 2 | HpR05(WormOnAcid) | fast nur schwarz (rote Linie) | offen — Bisektion |
| 3 | HpR10(AlealactaEst) | Kontrast so sicher nicht gedacht (gelbe Fläche) | offen — Verdacht Blend-/Sättigungsfamilie (S7) |
| 4 | HpR11(PowerSource) | zu viel Fläche dunkel, „Richtung Tunnel-Problem" | offen |
| 5 | HpR12(Dreamtime3) | zu dunkel | offen |
| 6 | HpR14(ZeroG) | wird in < 1 s weiß | offen — Weiß-Sättigungsfamilie (S7) |
| 7/8 | HpR16(EgoTheLivingPlanet) | beginnt > 1 s schwarz, wird dann zu hell (Init-Wartezeit ist teils normal) | offen |
| 9 | HpRX2(Source) | „da fehlt was" — Seiten-Streifen flackern nur kurz auf | offen |
| 10 | HpRX3(Narcotic) | nur schwarz | offen |
| 11 | HpRX4(RockThePlanet) | nur schwarz | offen |
| 12 | HpRX5(FrenchDream) | wird nach längerer Zeit zu hell | offen — S7-Familie |
| 13 | HpRX6(Subspace) | nur schwarz | offen |
| 14 | HpRX7(Matrixed) | Matrix-Text sollte ganzen Hintergrund + Kugel füllen | offen — nutzt vermutlich Text-Effekt mit randomPos/randomWord; prüfen, ob unser Wort-Zyklus/Random-Verhalten die Dichte des Originals erreicht |

*(Standalone-Sweep der Sammlung läuft — Ergebnisse werden ergänzt; danach
Bisektion der Schwarz-/Sättigungs-Fälle. Die Häufung „schwarz/zu hell" in
dieser Sammlung deutet auf 1–2 gemeinsame Ursachen in der S7-Familie
[Blend-Konvergenz] plus mögliche fehlende Effekte.)*

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.1.0 | 2026-07-24 | Angelegt (Session 44): Systematik S1–S3, JC-big stuff In-App-Erstsichtung (9 Presets), Sweep ausstehend |
| 0.2.0 | 2026-07-24 | Sweep 1 (Baseline) + Crash-Bisektion: S4 (Dot-Plane-OOB bei leerem Spektrum) und S5 (Mono-Getter ohne Stereo-Mix-Fallback) gefunden UND gefixt; S6 SVP Loader notiert; Dialog-Zuordnung §2.1 korrigiert (Dialog = ausgewähltes Preset) |
| 0.3.0 | 2026-07-24 | Sweep 2: 100/100 ohne Crash; Schwarz-Liste 15→0 (13× S5-Artefakt, 2× Aufbauzeit — Methodik-Merkregel ≥600 Frames); JC-Pack damit crash- und schwarzfrei, Rest = S1/S2/S3 + Weiß-Sättigungs-Bisektion |
| 0.4.0 | 2026-07-24 | ref-Korpus-Re-Sweep (§3): Schwarz-Liste 10→1 — einziger echter Fall Spacefolding (Quelle „Starfield" fehlt, + „Mosaic"); Tests 409/409 grün, VS-Debug + VS-Testing + Ninja-Release grün über den S4/S5-Fixes |
| 0.5.0 | 2026-07-24 | **S1 gefixt** (builtinRemap-Shader für fuzzify/blocky, 5 JC-Presets warnungsfrei) · **S8 gefunden + gefixt** (AvsParser 1.2.0: Alias-ID-Umschreibung — Spacefolding rendert, Korpus-Schwarz 10→0) · **S7 lokalisiert** (Prefix-Bisektion: XOR-in/50:50-out-Liste konvergiert zu Uniform-Grau — r_list-Referenzvergleich als nächster Schritt); Suite grün nach jedem Schritt |
| 0.6.0 | 2026-07-24 | §4 EL-VIS CYBERPUNX: Sweep 32/32 · 0 Crashes · 1 schwarz (highvoltage = AVI-Quelle, bekannte Grenze); rabbithole002-Analyse (Tunnel-Lücken = Segment-Zustandsmaschine, kein Audio-Bug); Audio-Gefühl → Kandidaten Beat-Rate + Trace-Vorschlag; Pfad-Notiz VisualsPresets jetzt außerhalb des Repos |
| 0.7.0 | 2026-07-24 | **Text-Effekt (id 28) + AVI-Effekt (id 32) implementiert** (Wunsch Patrik) + **Comment-Text-Übernahme** (Befund Patrik) — AvsParser 1.3.0 (3 neue Decoder), TextParams/AviParams + Translator + Serializer + Renderer (QPainter-Glyph-Layer bzw. VfW-Frames), Overlay-Shader mit Alpha-Blend; Gates: Serializer-Roundtrip + Translator-Mapping; EL-VIS-Sammlung damit komplett warnungs- und schwarzfrei; Suite grün, alle 3 Builds grün |
| 0.8.0 | 2026-07-24 | Comment → eigener Typ `CommentParams` + Panel-Feld (Entscheid Patrik); **Panel-Editoren für Text + AVI** ergänzt (Befund #1: Props-Bereich war leer); Standalone-Screenshot-DPI-Fix (physische Pixel); §5 HISTORY-pRELOADED-Sichtliste (14 Befunde Patrik) angelegt, Sweep läuft |
| 0.9.0 | 2026-07-24 | HISTORY-Sweep: 33/33, 0 Crashes, 0 Warnungen; Befund #1 (Text) aufgeklärt (rendert korrekt — Sichtbarkeits-/Panel-Thema); **S9 gefunden + umgesetzt** (volle BLEND_LINE-Tabelle, Beleg ZeroG-Bisektion + Roh-Bit-Dumps); Merkposten: GL-Suite nie parallel zu GL-Sweeps (ctest-Fehlalarm) |
| 0.10.0 | 2026-07-24 | **S10 gefunden + gefixt** (EL-VIS6_SUPERSCOPES_3D, Befund Patrik „Spektrum fehlt/3D passt nicht"): SuperScope-`which_ch` ist Bitfeld — Bit 4 = Spektrum-Quelle, Bits 0-1 = Kanal; vorher als Kanal-Enum „Side" fehlgelesen → v≈0. Sichtnachweis first3d_spectrum |
| 0.11.0 | 2026-07-24 | **S11 gefunden + gefixt** (Frage Patrik BASS↔Winamp): Winamp-Spektrum-Vertrag aus `winamp_orig`-Ref hergeleitet (linear /16, 256 Bins verdoppelt, 64 Fade-Bytes) — Frequenzachse korrigiert (Position=Bin 1:1, ≥512→0); kSpecGain=8 als Winamp-Sättigungsäquivalent bestätigt |
| 0.12.0 | 2026-07-24 | **S12 gefunden + gefixt** (Nachtest Patrik „noch nicht wirklich geändert"): SuperScope-`v` im Chain-Pfad jetzt AVS-treu aus den visdata-Bytes (r_sscope-Formel, v=Byte/128−1, Stille⇒−1) statt aus rohen Float-Arrays; Sichtnachweis Zickzack-Teppich first3d_spectrum |
