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
