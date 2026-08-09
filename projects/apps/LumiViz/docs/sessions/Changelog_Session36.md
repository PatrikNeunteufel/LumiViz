# Changelog Session 36 (2026-07-21) — AVS-Import komplett + Import-Browser

> **Typ:** Produkt-Changelog
> **Bezug:** [Import_Modul_Abdeckung.md](../visuals/Import_Modul_Abdeckung.md) ·
> [Import_Modul_Umsetzungsplan.md](../visuals/Import_Modul_Umsetzungsplan.md)
> **Tests:** LumiViz.UnitTests 287 Cases grün, 0 Skips, 3950 Assertions (vorher 251)

## Neu

### Import-Browser-Panel

Dockbarer Ordner-Browser zum bequemen Prüfen von Presets: Navigation in
Unterverzeichnisse, Filter **AVS · MilkDrop · LumiViz (.lvfx) · Alle**, Doppelklick
importiert (Endungs-Dispatch: `.avs`/`.lvfx`/`.milk`). Der Multi-Effect-Host wird bei
Bedarf automatisch aktiviert; Rückmeldung nur bei Problemen (nicht-modale Statuszeile).
**Der Multi-Effect-Host ist jetzt der Start-Default-Visualizer.**

### AVS-Import — praktisch vollständig

Die AVS-Import-Phase ist durch (Builtins ✅ 42, APEs ✅ 16 — kein darstellbarer
Effekt mehr offen). Neu in dieser Session, je voll verdrahtet (Shader/Handler +
Parser + Translator + Serializer + Panel + Tests):

- **Trans/Render:** Dynamic Distance Modifier, Dynamic Shift, Moving Particle,
  Color Clip, Unique Tone, Interleave, Simple, Bass Spin, Oscilliscope Star, Ring,
  Rotating Stars, Picture.
- **APEs:** Color Map, Buffer blend, Jheriko: Global, Convolution, Normalise,
  MultiFilter, Add Borders, Picture II, Texer, Texer II, Triangle.
- **Sonderfälle:** Comment (stiller no-op), Set Render Mode (jetzt auch Blend-Mode),
  Framerate Limiter / Text (no-op + Import-Notiz).
- **Bild-Lader:** referenzierte Bilder werden beim Import **base64 ins `.lvfx`
  eingebettet** (self-contained); fehlt eine Datei → Import-Notiz.

## Geändert

- `.avs`-Parser: `decodeApe` ist jetzt die einzige APE-Autorität (Core- + Community-
  APEs; unbekannte bleiben Roh-Blob + Warnung). NUL-terminierte Strings unterstützt.
- Import-Events tragen optionale Pfade (Direkt-Import ohne Dialog).

## Bekannte Rauheiten (Sichttest-Nachzug)

- **GL/UI-Sichttest steht komplett aus** (Shader/Renderer kompilieren erst zur Laufzeit).
- **Approximiert / kalibrierbar:** Texer II (Format- + Punkt-EEL-Annahmen), Triangle
  (Wireframe-Näherung, EEL-Variablen), MultiFilter (Chrome-Mathematik), Normalise
  (32×32-Readback), Scope-Geometrie, Color-Map-Farbreihenfolge.
- **Text** (GDI-Textrendering) noch nicht dargestellt (no-op).

## Verifikation

287/287 Cases grün, 0 Skips; VS-Testing baut (`/WX`). Neue Unit-Tests je Effekt
(Translator-Mapping, Serializer-Roundtrip, `effectTypeName`) + Parser-Byte-Tests für
die kniffligen Binärformate (Color Map, Jheriko, Picture). GL/UI-Sichttest offen.
