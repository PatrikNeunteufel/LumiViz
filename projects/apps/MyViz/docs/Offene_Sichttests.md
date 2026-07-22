# MyViz — Offene Sichttests & Hörtests

> **Stand:** 2026-07-21 (Ende Session 37) · **Typ:** Status/Checkliste
> **Warum:** GL-Shader, Audio-Analyse und UI-Interaktion laufen erst zur Laufzeit —
> diese Punkte kann nur Patrik am laufenden Build verifizieren. „Gute Werte" aus den
> Tests bitte zurückmelden → werden als Defaults gesetzt / kalibriert.

Legende: ⬜ offen · ◐ teilweise · ✅ erledigt

---

## 1. Batch H — Fraktal-Module (Session 37)

Alle 9 Module sind gebaut (Unit-Tests grün), aber **GL-Sichttest komplett offen**.
Im MultiEffect-Panel unter „— Fractals —" wählbar.

- ⬜ **Fractal 2D** — 9 Typen (Mandelbrot/Julia/Burning Ship/Tricorn/Multibrot/
  Newton/Phoenix/Magnet/Nova). Kalibrieren: **Newton/Magnet/Nova-Färbung**
  (Iterations-Banding), Smooth-Coloring, Escape-Radius-Wirkung.
- ⬜ **Fractal 3D** — Raymarch-DE (Mandelbulb/Mandelbox/Menger/Quaternion-Julia/
  KIFS). Kalibrieren: Kamera-Defaults (yaw/pitch/dist/fov), Licht/AO, maxSteps
  vs. Performance.
- ⬜ **Domain Warp (fBm)** — „Plasma/Nebel". Kalibrieren: scale/warp/speed-Defaults,
  Audio-Reaktivität.
- ⬜ **Fractal Zoomer** — Endlos-Zoom + Feedback-Trail. Kalibrieren: zoomSpeed,
  feedback, Loop-Verhalten (Reset bei sehr großem Zoom).
- ⬜ **Lyapunov** — Sequenz-Muster. Kalibrieren: Färbung geordnet/chaotisch,
  Sequenz-Defaults, View-Rechteck.
- ⬜ **Kleinian** — **stilisierte** {p,q}-Kachelung (nicht rigoros). Kalibrieren:
  Geometrie/Morph — ist der Look akzeptabel?
- ⬜ **Strange Attractors** — Lorenz/Clifford/DeJong/Aizawa. Kalibrieren: Lorenz-
  Parameter-Mapping (a/b/c → σ/ρ/β), scale/dotSize, Reseed bei Divergenz.
- ⬜ **Flame / IFS** — Chaos-Game. Kalibrieren: IFS-Anker/Variationen, Punktzahl vs.
  Helligkeit, dotSize.
- ⬜ **Reaction-Diffusion** — Gray-Scott. Kalibrieren: feed/kill-Defaults, Halb-
  Auflösung ok?, Seed-on-Beat-Radius.

**Querschnitt:** ⬜ Gradient-Paletten sehen je Modul gut aus? ⬜ Blend (Replace/
Additive/50-50) über die Kette korrekt? ⬜ Audio-EEL (`bass/mid/treble/vol/beat` +
`getspec/getosc`) reagiert plausibel?

---

## 2. AVS-Import Batch A–G (Session 36) — Approximierte Effekte

- ⬜ **Texer II** (SizeString-Format + Punkt-EEL x,y,sizex,sizey,rgb,n)
- ⬜ **Triangle** (x1..y3-Vertices)
- ⬜ **MultiFilter** (Chrome-Mathematik, closed-source approximiert)
- ⬜ **Normalise** (32×32-Readback Auto-Levels)
- ⬜ **Scope-Geometrie:** Oscilliscope Star (Rotation), Bass Spin, Rotating Stars (Orbit)
- ⬜ **Dynamic Shift / DDM** — Y-Vorzeichen prüfen
- ⬜ **Color Map** — R/B-Reihenfolge
- ⬜ Import-Korpus `../ref/vis_avs` re-importieren, Report-Passthrough-Zeilen zählen.

---

## 3. Set Render Mode (Live-Node, Session 37)

- ⬜ Set Render Mode als Ketten-Knoten setzt Linienbreite/Blend für folgende
  Scopes korrekt? (wirkt aktuell **nur auf SuperScope** — Ausdehnung auf alle
  Scopes ist geplant, siehe Offene_Implementierungen §A).
- ⬜ Keine „unrolled - passthrough"-Notizen mehr beim Import.

---

## 4. Audio — echtes L/R-Stereo (Session 37) ⚠ HÖRTEST

- ⬜ **`getspec/getosc` mit ch=1 (L) vs ch=2 (R)** liefern getrennte Kanäle?
  Nur compile-verifiziert — die **BASS_DATA_FFT_INDIVIDUAL-Layout-Annahme**
  (`bin*chans+ch`) + Waveform-Interleaving sind die Stellschrauben. Bei
  Fehldarstellung: Layout in `MainWindow`/`buildVisData` justieren.
- ⬜ Mono-Streams: Fallback greift (L=R=mono, kein Regress)?

---

## 5. Editor — Skript-System (Session 37)

- ⬜ **Kategorie-Highlighting:** read-only/input/output/in-out/constant/custom-global
  je Farbe korrekt/lesbar (Legende im ⓘ-Fenster)?
- ⬜ **Fehler-Markierung (rot):** Schreiben auf `w/h/dt`/`pi/pi2` + Init-Doppel-
  deklaration einer Global (`regNN`/`qN` in zwei Nodes' Init) → rote Wellenlinie?
- ⬜ **ⓘ-Referenz** + **⤢-Expand** (großer Editor + Referenz) funktionieren, Inhalte
  akkurat?

---

## 6. UI — Mehrfach-Selektion (Session 37)

- ⬜ Shift-Range / Ctrl-Toggle in der Effektliste, beschränkt auf gleiche Ebene?
- ⬜ Gruppen verschieben per ↑/↓-Buttons (Block) + Entf/Backspace entfernt Gruppe?
- ⬜ **Multi-Drag zwischen verschiedenen Listen** (Block-Reparenting) — Index-
  Mathematik nur compile-verifiziert, genau prüfen; bei Bugs → absichern.

---

## 7. Import-Treue-Fixes (Session 38) — Sicht-/Hoertests

Steuerdokument: `docs/visuals/Import_Treue_Fixplan.md` (Befund + Umsetzungsstand).
Referenz-Presets: `asset/avs/greatwho2006/4resample/EyeCandy2` (Symptomliste in
Fixplan §2 — jedes der 10 Presets sollte jetzt deutlich naeher am Original sein).

- [ ] **getspec-Gain kalibrieren:** `kSpecGain` (MultiEffectVisualizer::buildVisData,
      aktuell 12.0) — Bewegungstempo der EyeCandy-Presets mit Winamp-Original
      vergleichen; zu traege -> Gain rauf, hektisch/uebersteuert -> runter.
- [ ] **Beat-Verhalten:** AVS-Onset + refine() — feuern Beats musikalisch?
      06_Stargate: kehrt die Rotation jetzt gelegentlich? 02_flowers: dmx/dmy-Flips?
- [ ] **Bump additiv:** 02_flowers / 05_wormhole / 08_noname — Licht statt Abdunkeln?
- [ ] **Buffer-Save-Blends:** 01/05 — keine Ueberbelichtung mehr (50/50 statt additiv)?
- [ ] **Clear-Screen-Blends:** 08_noname — weiches Abklingen statt Schwarz-Clear?
- [ ] **DM-alpha:** 01/02/05/08/10 — weiches Layering, `nomove`-Faelle korrekt?
- [ ] **SuperScope n=800/n=2:** Punktdichte der Spiralen; Wormhole-Linien (n=2) da?
- [ ] **Weiss-Default-Farbe:** 05_wormhole Scope 1 — gruenstichig wie im Original?
- [ ] **Mirror smooth/Richtungen:** 05_wormhole — weiche Uebergaenge, richtige Haelfte?
- [ ] **Movement 96x72 + Cache:** 07/08/09/10 — Achsen-Schmier scharf genug? FPS ok?
- [ ] **FyrewurX (Nachbau):** Presets in greatwho2006_2 — Feuerwerks-Bursts auf Beat;
      Kalibrierpunkte: sparks=80, speed=0.7, gravity=0.8, life=1.6 s, dotSize=2;
      Gravitations-RICHTUNG pruefen (AVS +y = unten; ggf. Vorzeichen kippen).
- [ ] **Listen-fake_enabled:** Presets mit "on beat render"-Listen blitzen auf Beat auf?

## 8. Import-Treue Runde 2 (Session 38) — Sichttests

- [ ] **05_wormhole — WEITER OFFEN (P1 naechste Session):** Tunnelform +
      Drehen/Gegenlauf stimmen trotz buffern-Support und Line-Width-255-Fix
      NICHT (Patrik: Original-Winamp hat die Balken sicher nicht; Stand
      **c8d2bd2** — vor Session 38 — sah dort besser aus). Verifiziert sauber:
      DM-Flag-Bytes (blend=1/buffern=1, byteperfekt), Buffer-Index-Match,
      g_blendtable linear, load_string-Laengen. ➜ **Faktoren-Bisektion** der
      Session-38-Aenderungen gegen c8d2bd2 (Kandidaten: DM-alpha/blend-
      Aktivierung, buffern-Pfad, Beat/kSpecGain-Tempo, xres+1-Gitter,
      Line-Width 255, Mirror-Smooth, Buffer-Save-Blend/Save-Pfad).
- [ ] **kSpecGain = 8** (war 12, "etwas zu schnell"): Tempo erneut bewerten.
- [ ] **Community Picks:** alien intercourse 4 + el-vis golden + Alienated
      zeigen Inhalt (buffern-Pfad)? Helium zeigt Punkt-Sprites (Texer-II-
      Fallback)? Data flow / Ex Deux Geometrie besser (xres+1)?
- [ ] **Vollflaechen weg?** Geometric Sustinance (orange) / Alienated (gelb) —
      Line-Blend-Default ist jetzt Replace; wenn noch zu hell: naechster
      Verdacht Unique-Tone-invert-Pfad.
- [ ] **sourcemapped:** 037b/037c/113/Tuggummi "Let me out!" — Scatter-Look
      (Luecken werden von der GPU-Naeherung gefuellt; bewerten ob ok).
- [ ] **subpixel aus:** harte Pixel-Kanten sichtbar wo gewollt (z.B. alien
      intercourse DM subpixel=0)?
- [ ] **Pro-Punkt-drawmode:** kein Regress bei normalen Scopes (Split-Pfad
      greift nur, wenn der Point-Code drawmode setzt).
