# Changelog Session 70 (2026-08-06/07)

> Video-Weg Teil A (`videoSource` — Datei/Kamera/Testaufnahme) + Stilfilter
> Teil B (`pixelFilter` + 12 Werks-Looks, Flaggschiff Take-On-Me-Comic).
> Tests 563 grün, alle 3 Builds grün. **🔴 Offen: Kamera-Teardown-Deadlock**
> (App schließt nicht, wenn eine Kamera lief) — frische Session.

## Teil A — Video-/Kamera-Quellknoten `videoSource` (Offene_Punkte §7)

- **Drei Quellen:** Datei (MP4/MKV/WebM/MOV/WMV/AVI via FFmpeg-Backend) ·
  Kamera (live) · Testaufnahme. **Zwei Taktungen:** Echtzeit-Streaming
  (NEU `services/LiveVideoFeed`: QMediaPlayer/QCamera je Node über einen
  QVideoSink) und deterministischer **Frame-Schritt** (VideoFrameCache
  1.1.0, NEU `Clip::fps`; Sim-Uhr × fps × Tempo — Sonden-Grundlage).
- Tempo 0,05–20×, Schleife, Einpassung (Strecken/Letterbox/Füllen), Blend,
  Deckkraft; Parameter-Skripte `speed`/`opacity`; relative Pfade lösen beim
  Laden gegen den Preset-Ordner auf; eigener Overlay-Shader.
- **Kamera-Vertrag:** Gerät startet NIE automatisch — Freigabe je App-Lauf
  (Panel-Knopf, Settings-Aufnahme-Klick, NEU Freigabe-Dialog beim Laden
  eines Kamera-Presets; einzige Vollbild-Ausnahme, StaysOnTop).
- **NEU Settings-Tab „Kamera"** (Idee Patrik): Testaufnahmen benutzerlokal
  (AppData, kein PII im Repo) — dritte Betriebsart + Fallback + der
  deterministische Kamera-Stellvertreter für Sonden.

## Teil B — Stilfilter `pixelFilter` (Offene_Punkte §7, Entscheid Patrik)

- EIN skriptbares Filtermodul statt Festmodule; der Filter-STACK ist die
  Kette selbst (mehrere Knoten, umsortierbar; Klone/Preset-Laden =
  unabhängige Kopien). Nutzer-GLSL **`vec4 farbe(vec2 uv, vec4 src)`** je
  Pixel, Nachbar-Abtastung via `uTex`, Audio-Uniforms, Mix-Regler +
  `mixamount`-Skript, Groß-Editor mit Apply/ⓘ. NEU `PixelFilterWrapper.hpp`
  (GL-frei, testerzwungen); Kompilierfehler ⇒ Passthrough + Panel-Anzeige.
- **12 Werks-Voreinstellungen** (`asset/nodepresets/pixelFilter/`, alle
  deterministisch + audio-reaktiv): **Take-On-Me-Comic** (Sobel + Papier/
  Tinte + Beat-zitternde Schraffur), Bleistift-Skizze (XDoG),
  Posterize-PopArt, Zeitungsdruck-Halftone, CRT-Monitor, VHS-Band,
  Kuwahara-Oelbild, Sepia-Nostalgie, Noir-Schwarzweiss, Waermebild,
  Pixel-Art, Duotone-Neon. Katalog-README auf Stand S70 (inkl. der 28
  fehlenden S69-Zeilen, gegen die JSON-Inhalte verifiziert).

## Befunde

1. **`QVideoFrame::toImage()` = SCHWARZ** unter Qt 6.10.1/FFmpeg (Rohdaten
   per map() da) → NEU `services/VideoFrameUtil.hpp` (toImage-Ersatz) für
   LiveVideoFeed UND VideoFrameCache (betraf auch avi-Fallback VR09).
2. **`filter` ist GLSL-RESERVIERT** (AMD lehnt ab) → Vertrag `farbe()`,
   Wächter-Test.
3. **Lag (extremes UI-Einfrieren mit Live-Kamera):** Frame-Wandlung lief auf
   dem GUI-Thread; Render-Thread-Wandlung hängt bei Hardware-Frames →
   endgültig eigener **Wandler-Thread** (queued Zustellung), 720p/30fps-
   Formatwahl, glTexSubImage2D. **Behoben** (Sichttest).
4. **Zombie-Klassen behoben:** Queued-Lambdas nie mit letztem Eigentum an
   Qt-Multimedia-Objekten (Friedhof-Muster + synchrones alleStoppen);
   Decoder auf eigenem Thread (VideoFrameCache 1.2.0); Wiederbelebungs-
   Riegel gegen Kamera-Neustart im Teardown.
5. **🔴 OFFEN: Kamera-Teardown-Deadlock** — die D3D-Worker der
   MF-Kamera-Pipeline (nvwgf2umx) sterben im Prozess nie; schon die
   Laufzeit-Zerstörung der Pipeline vergiftet den Prozess, App schließt
   dann nicht (auch nicht mit kontrolliertem `_Exit`). 5 Ansätze in
   Offene_Punkte 1.59.0; ohne Kamera-Start schließt alles sauber.

## Doku & Verifikation

Benutzerhandbuch **1.8.0** (NEU §13 Video & Kamera, §14 Stilfilter) ·
LiveVideoFeed.md NEU→1.2.0 · PixelFilterWrapper.md NEU · VideoFrameUtil.hpp
NEU (Befund-Doku im Header) · Offene_Punkte **1.59.0**.
Sonden `videosource_sonde` + `pixelfilter_sonde` (Doppellauf SHA256-
identisch) · Demo `pixelFilter - Take-On-Me.lvfx` · FieldDocs 90 Typen /
774 Felder, 0 Lücken · Tests **563** (554 + 2 + 7), alle 3 Builds grün.
