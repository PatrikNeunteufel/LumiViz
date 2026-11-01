# Changelog Session 169 (2026-11-01)

> **Der Loop kann Tonspuren wie Gesang tragen.** Ein **Wave-Clip** spielt eine Tondatei im Takt — Ausschnitt, Lage,
> Gain und Blenden stellt man im neuen Fenster **Wave editor** ein. Aufnehmen geht direkt in einen Wave-Clip, ohne
> Kopfhörer einfach so wie ein Sample, mit einer Leuchte, die je Schlag aufleuchtet. Die **Spur Pitch** hebt und senkt
> die Tonhöhe, ohne das Tempo zu ändern, und das Programm misst den Tonhöhenverlauf einer Aufnahme (pYIN) — die
> Grundlage für Autotune. Steuerdokument: `docs/audio/Loop_Konzept.md` §4p.

## Neu

- **Wave-Clips** (15b–15e): dritte Art von Muster neben Drums und Instrumenten. In der Taktachse *New wave clip...*
  auf einem freien Takt, der Clip zeigt seine Welle; der **Wave editor** mit Datei, Ausschnitt an zwei Griffen, Lage
  in Schritten (Shift: Bruchteile), Gain, Fade in / out, Spur Expression. Publish und Datensicherung nehmen die Datei
  als Sample mit.
- **Record wave clip...** (15k-1, MH2 a, LD1 a): Vorgabe *Nothing — record right away* — nimmt sofort auf wie ein
  Sample, eine **Leuchte** zählt still ein (je Schlag, die Eins hell), der Take beginnt genau auf der Eins des
  Starttakts, Stop nimmt bis zum Ende des Takts auf. Mit Kopfhörern *Click only* oder *Loop and click*: der Loop zählt
  ein, der Take sitzt im Takt, die Latenz ist abgezogen (*Measure latency...* misst sie mit fünf Klicks).
- **Spur Pitch** am Wave-Muster (15f): Tonhöhe in Halbtönen über der Zeit, rastet auf ganze Halbtöne (Shift:
  Hundertstel). Gerechnet mit **Signalsmith Stretch** (MIT).
- **Tonhöhenverlauf** einer Aufnahme mit **pYIN** (15g, eigener Nachbau nach Mauch & Dixon 2014), je Datei einmal
  gemessen und gemerkt. An 40 Ausschnitten echten Gesangs (vocadito): 93,9 % der gesungenen Stellen auf 50 Cent
  richtig.

## Geändert

- `.lvloop` **Fassung 16** (Wave-Muster). Alte Loops klingen unverändert.
- Ohne Kopfhörer hörte das eingebaute Mikrofon den Loop mit — jetzt ein Hinweis unter dem Eingang und die Wahl, was
  beim Aufnehmen spielt (MH1 a).

## Behoben

- **Absturz**, wenn das Fenster des Visualizers geschlossen wurde, während im Panel der Effektkette ein Feld mit
  Live-Anzeige offen war.
- Die Zeile im Fenster *Record wave clip* nannte „up to 30 s“ statt 600 s; der Pegel blieb nach einem Stop stehen.

## Intern

- Neu `loop/AufnahmeUhr`, `loop/LatenzFenster`, `loop/Dehner` (Schnittstelle, Signalsmith hinter ihr),
  `loop/PYin`, `loop/Tonhoehenspeicher` (Ordner `pitch` neben `frozen`); `IAudioEngine::setRecordClock`;
  `VisualizerGoneEvent`.
- External `externals/signalsmith-stretch` (Stretch 1.4.0, Linear 0.6.4), Vermerk in `THIRD_PARTY_NOTICES.md`.
- Tests: **1727 Cases** (+39). Voller Lauf nach 15f unter Release-clang und Testing-MSVC je 1719 / 1719; 15g eng.
- Recherche und Bericht zu pYIN unter `docs/modellierung/wave-clips/`; Messdaten vocadito (CC BY 4.0) lokal unter
  `…\cmake\samples\Gesang\`.
