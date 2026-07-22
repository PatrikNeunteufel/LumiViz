# Kalibrier-Presets M3 — MD1-Render-Kern

> **Zweck:** Jedes Preset isoliert EINEN Aspekt des MilkdropVisualizer-Kerns
> (Session 39, M3). Reihenfolge einhalten — 01 kalibriert die Orientierung,
> auf der alle weiteren aufbauen. Import: Doppelklick im Import-Browser.
> Stellschrauben: `src/visualizers/MilkdropVisualizer.cpp` (Suchmarken im Code).

## 01_orientierung.milk — Y/X-Orientierung (ZUERST!)

**Bild:** ein roter Kreis, statisch (kein Zoom/Warp), positioniert bei
wave_x=0.75 / wave_y=0.75.
**Erwartung (Original-Semantik):** Kreis **oben rechts** (höheres wave_y =
höher im Bild).
**Abweichung:** Kreis unten rechts → Y-Flip im Composite togglen (Suchmarke
„presentation flip" in `compositeToScreen`, vRef-Zuweisung lo/hi tauschen).
Kreis oben LINKS → X gespiegelt (unwahrscheinlich; dann u-Zuweisung prüfen).

## 02_zoom.milk — Zoom-Richtung

**Bild:** weißer Kreis in der Mitte, zoom=1.03.
**Erwartung:** Die Spuren wandern **nach außen** (Bild bläht sich auf), die
Mitte bleibt ruhig. Gleichmäßig in alle Richtungen (kein Drift).
**Abweichung:** Spuren ziehen nach innen → Zoom invertiert (zoom2Inv in
`computeWarpMesh`). Drift zur Seite → cx/cy- oder Halb-Texel-Fehler.

## 03_rotation.milk — Drehrichtung + Drehzentrum

**Bild:** horizontale grüne Linie (Mode 6), rot=0.05.
**Erwartung:** Die Spur dreht **gleichmäßig um die Bildmitte** — kein Taumeln,
kein Auswandern des Zentrums. Drehrichtung mit Original-MilkDrop vergleichen
und notieren (wichtig für spätere per_pixel-rot-Presets, Wormhole-Lektion!).
**Abweichung:** Richtung falsch → sinRot-Vorzeichen in `computeWarpMesh`;
Zentrum wandert → cx/cy-Behandlung.

## 04_warp_ripple.milk — Warp-Amplitude/-Tempo

**Bild:** blauer Kreis, warp=2 (Original-Default wäre 1).
**Erwartung:** organisches, weiches Wabern der Spuren (4-Term-Ripple),
deutlich sichtbar, aber kein Zerreißen. Tempo ruhig (fWarpAnimSpeed=1).
**Abweichung:** zu stark/zu schwach → 0.0035-Konstanten prüfen (sollten exakt
stimmen); zu hektisch → warpTime-Kopplung.

## 05_decay.milk — Decay-Kennlinie

**Bild:** weiße Linie; per_frame wechselt decay alle ~12 s zwischen 0.94 und 1.0.
**Erwartung:** Phase A (0.94): Spuren verblassen in **unter 1 Sekunde**
vollständig zu Schwarz (kein Grauschleier-Rest!). Phase B (1.0): Spuren
bleiben unbegrenzt stehen.
**Abweichung:** Grauschleier bleibt → decay-Anwendung (uDecay im Warp-Shader);
zu schnell/langsam → Framerate-Kopplung prüfen.

## 06_wave_modi.milk — Basis-Waveform Modi 0–7

**Bild:** alle 2 s der nächste Wave-Mode (0→7, dann von vorn).
**Erwartung je Mode:** 0 Kreis · 1 Spirale (rotierend) · 2 Spiro-Wolke
(schwach) · 3 Spiro (lautstärke-abhängig heller) · 4 horizontale Schrift-Linie
mit Schwung · 5 rotierendes Explosions-Muster · 6 EINE horizontale Linie ·
7 ZWEI getrennte Linien (Stereo).
**Abweichung:** notieren, welcher Mode falsch aussieht — jeder hat eine eigene
Formel in `drawBasicWave` (case 0..7).

## 07_composite_echo.milk — Video-Echo + Orientierungen

**Bild:** Kreis rechts oben; echo_alpha=0.5, echo_zoom=2; alle 2 s die nächste
Echo-Orientierung (0..3).
**Erwartung:** eine zweite, doppelt so große, halbtransparente Kopie des
Bilds. Orientierung 0: gleiche Lage · 1: horizontal gespiegelt (Kopie links) ·
2: vertikal gespiegelt (Kopie unten) · 3: beides (diagonal gegenüber).
**Abweichung:** Flips vertauscht → orient-Bits in `compositeToScreen`
(drawLayer, `orient & 1` / `orient >= 2`).

## 08_gamma_filter.milk — Gamma-Pulsieren + Invert-Blitze

**Bild:** grauer Kreis; gamma pulsiert 1↔3 (~12 s Zyklus); ganz kurze
Invert-Blitze (~alle 30 s).
**Erwartung:** weiches Auf- und Abblenden der Gesamthelligkeit OHNE
Farbverschiebung und OHNE Stufen/Banding-Sprünge (Gamma = additive
Mehrfach-Draws — Sprünge deuten auf Pass-Zählfehler). Invert: kompletter
Negativ-Blitz.
**Abweichung:** Helligkeitssprünge bei gamma≈2.0/3.0-Schwellen → nPasses-
Rundung in `compositeToScreen`.

## 09_borders.milk — Rahmen (ob/ib)

**Bild:** statisch; roter Außenrahmen (5 %), blauer Innenrahmen (3 %) direkt
innen anliegend.
**Erwartung:** beide Rahmen umlaufend geschlossen, gleichmäßig dick, blau
direkt an rot anschließend (keine Lücke, keine Überlappung), Ecken sauber.
**Abweichung:** Lücken/Versatz → Ring-Rechtecke in `drawBorders`.

## 10_per_pixel_tunnel.milk — per_pixel + rad

**Bild:** oranger Kreis; per_pixel erhöht zoom mit rad → Tunnel.
**Erwartung:** Mitte praktisch statisch, Ränder fließen deutlich nach außen —
ein runder (bei Nicht-Quadrat-Fenster NICHT elliptischer!) Tunnel-Sog.
Prüft per-Vertex-Skriptpfad, rad-Berechnung und Aspect-Korrektur.
**Abweichung:** elliptisch → Aspect in rad-Formel (`computeWarpMesh`);
alles statisch → Point-Slot läuft nicht (Report auf Skript-Fehler prüfen).
