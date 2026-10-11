# Changelog Session 111 (2026-10-11)

> **Instrumente bestehen aus Schichten.** Ein Instrument trägt eine bis vier
> Schichten, jede mit eigener Schwingung, Hüllkurve, Lautstärke, Versatz in
> Halbtönen, spätem Einsatz und einem Verzerrer. Dazu kamen 35 Instrumente
> (49 insgesamt) und 24 Beispiel-Loops, neunzehn davon nach Genre. **Was in
> dieser Session dazukam, ist nach Rechnung gesetzt und im Programm weder
> gesehen noch gehört** — geprüft ist es mit Unit-Tests, Bildern aus dem Test
> und gerechneten Summen.

## Neu

- **Schichten im Instrument-Creator:** links die Liste **Layers** — ein Klick
  wählt eine Schicht, der Haken schaltet sie stumm. **Add layer**, **Add layer
  from instrument...** (kopiert die Schichten eines anderen Instruments,
  mitgeliefert oder aus einer Datei), **Duplicate**, **Remove**, **▲ ▼**.
- **Je Schicht:** **Name**, **Level**, **Transpose** (−24 bis +24 Halbtöne),
  **Start offset** (die Schicht setzt später ein — kein Echo) und **Drive**
  (ein Verzerrer). Oben ein **Level** für das ganze Instrument.
- **Die Bilder** zeigen die Summe aller Schichten und gestrichelt die
  gewählte; **Selected layer only** lässt die Klaviatur nur sie spielen.
- **Sechzehn Instrumente aus Schichten:** Bell, Tine Piano, Brass, Flute,
  Super Saw, Fifth Pad, Overdrive Guitar, Kalimba, Marimba — und sieben
  ungewöhnliche: Cascade (jede Note spielt vier Töne nacheinander), Siren,
  Alien Choir, Broken Radio, Ghost, Growl Bass, Reverse Swell.
- **Neunzehn weitere für die Genre-Beispiele:** Trance Lead, Trance Pluck,
  Rolling Bass, Hoover, Clavinet, Slap Bass, Clean Guitar, Muted Guitar, Round
  Bass, 808 Bass, Reese Bass, Upright Bass, Nylon Guitar, Sax, Vibraphone,
  Steel Drum — und als Schlagwerk Cowbell, Conga, Clave.
- **Fünf Beispiele** unter `asset\loop`: `bells_demo`, `synthwave_demo`,
  `rock_demo`, `strange_demo`, `ambient_demo`.
- **Neunzehn Beispiele nach Genre** unter `asset\loop\genres`: Trance
  (dreimal), Funk, Disco, Reggae, Ska, Reggaeton, Hip-Hop, Trap, Drum and
  Bass, Techno, Bossa Nova, Blues, Jazz Waltz, Lo-Fi, Synth-Pop, Afrobeat,
  Metal.
- **Benutzerhandbuch §16:** Schichten, Beispiele.

## Geändert

- **Das Format `.lvinst` trägt jetzt Schichten** (Fassung 2). Dateien aus
  Session 110 liest LumiViz weiter und spielt sie unverändert; gespeichert
  wird im neuen Format, das die Fassung aus Session 110 nicht richtig liest.
  Eine Datei aus einem neueren LumiViz wird abgelehnt: das Muster schweigt,
  das Panel Loop meldet „Instrument needs a newer LumiViz".
- **Sichttest-Steuerung:** das bezogene Paket ist v0.4.0 (vorher v0.3.4).

## Behoben

- **Der Drum-Editor mit einem leeren Muster:** in einem hohen Fenster standen
  Kopf, Raster-Zeile und Spuren weit auseinander. Die freie Höhe bleibt jetzt
  unten; ein Muster ohne Spuren zeigt einen Hinweis auf **Add track**.

## Für Entwickler

- [Loop_Konzept.md](../audio/Loop_Konzept.md) 0.22.0 — §4f: die gebilligte
  Skizze der Schichten, Fassung 2 der `.lvinst`; Entscheide **F1 a** (Liste
  `layers`) und **E1 a** (Verzerrer im Instrument, Hall und Echo am Muster);
  Stufen 10c (gebaut) und 12 (Hall und Echo, offen).
- `loop/Instrument`: `Schicht`, `kSchichtenMax`, `kInstrumentFassung`.
- `asset/loop/make_examples.py` und `asset/loop/make_genres.py` erzeugen die
  Instrumente und Beispiele dieser Session; maßgebend sind die Dateien im Repo.
- `tools\bau_clang.bat [Ziel]` baut Release-clang in der Umgebung von Visual
  Studio 18 und bricht ab, wenn `Sichttest` oder `LumiViz` läuft.
- `sichttest.pin` v0.4.0: Köpfe S 1.1, am Code nichts zu ändern.
- Bilder aus den Tests: `LUMIVIZ_INSTRUMENT_CREATOR_BILD=<datei.png>` zeigt
  drei Schichten, `LUMIVIZ_DRUM_EDITOR_BILD=<datei.png>` legt dazu
  `<datei>.leer.png` ab; `LUMIVIZ_INSTRUMENT_WAV=<Ordner>` schreibt Hörproben
  aller Beispiele.

## Offen

- Der Blick ins Programm und das Ohr auf alles aus dieser Session: ob die
  Instrumente brauchbar klingen und die Loops nach ihrem Genre.
- Hall und Echo am Muster (Stufe 12), davor die Skizze.
- Ein größeres Drumkit — Cowbell, Conga und Clave sind bis dahin Instrumente.
- `asset\loop` liegt im Release-Paket nicht neben der Exe; dort schweigen
  Instrument-Clips.

## Tests

1044 Cases, voller Lauf 1044 / 1044, 0 Skips (Testing-MSVC). Unter
Release-clang die Fälle zu Sichttest-Aktionen, Loop und Editoren. Nicht
gemacht: Debug-MSVC, der volle Lauf unter clang.
