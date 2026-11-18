# Changelog Session 206 (2026-11-18)

> Die Körper von Harfe und Gitarre sind nach den Entscheiden aus S205 neu gebaut. Der Bauplan sagt jetzt, wie Leisten
> zählen, die übereinander liegen; die Körperdatei trägt je Mode die Richtung, in der sie den Steg bewegt. Alle
> mitgelieferten Körper sind neu gerechnet bzw. neu geschrieben.

## Neu

- **`.lvinstbodyplan` Fassung 8 (KQ31-1 a):** `barOverlap` — `first` (nur die erste Leiste an einem Punkt, wie bisher)
  oder `add` (die Höhen zählen zusammen, Werkstoff und Faserrichtung der ersten); immer geschrieben, Fassung 7 liest
  sich als `first`. `InstrumentBodyTool` meldet Leisten übereinander mit Fläche und Volumen. `InstrumentBody` 0.23.0.
- **`.lvinstbody` Fassung 5 (KQ35-2 a, KQ35-1 a):** je Mode `bridgeAngle` (Winkel der Stegbewegung gegen die
  Senkrechte der Decke; `bridge` ist der Formwert längs dieser Richtung), im statistischen Teil `bridgeAngle` und
  `parallelFactor` (je gezogener Mode eine zweite nur parallel zur Decke); immer geschrieben, Fassung 4 liest sich
  mit 0. `ModeImKoerper::senkrechtAm` / `parallelAm`. `InstrumentBody` 0.24.0.
- **`make_gitarre.py`, `kFitFassung` 4:** Schritt 7 füllt die Modendichte über 2,2 kHz nach Elie 2011 auf, Schritt 8
  schreibt die Richtung je Mode nach Elie 2012 und je Mode eine zweite nur parallel zur Decke.
- Tests „KQ31-1 Leisten uebereinander“ und „KQ35 Ziehung mit Richtung am Steg“; Datei Fassung 5; Mobilität mit Winkel.
- Proben `docs/modellierung/gezupfte-saite/proben/2026-11-18_S206_*` (Harfe nach KQ31-2 a, Harfe und Gitarre je Taste).

## Geändert

- **Harfe:** `barOverlap: add` (Deck- und Zugleiste), Güte der Luft 8,5 (KQ29-1 a), 131 Moden; der Test „Luftraum die
  mitgelieferte Harfe“ misst ohne Saiten an S(1,2)–S(1,6) nach KQ31-2 a und hält zwei Lagen, sechs Höhen und die Höhe
  des stärksten Bauchs als bekannte Abweichungen fest (KQ31-3 a).
- **Gitarre:** 233 Moden bis 5 kHz (75 gefittet, 42 gezogen, 116 nur parallel zur Decke), statistischer Teil
  0,0215 / Hz mit Winkel 12,76° und Anteil 0,397.
- **Alle sieben Körper mit Bauplan:** für Fassung 8 neu gerechnet (gleich bis auf Abdruck und Rechenzeit), für
  Fassung 5 neu geschrieben (gleich bis auf `version`, `units` und die neuen Felder).
- `antwort`, `antwortSpektrum*`, `mobilitaet` rechnen mit dem senkrechten Formwert (Winkel 0: Bit für Bit wie vorher).
- Der Prüfstand `proben/2026-11-15_S199_pruefstand_zupf.py` liest die Richtung aus der Körperdatei.
- `docs/audio/Instrumente_IC_Konzept.md` 0.66.0 (KQ29-1 a, KQ31-3 a, Bauvermerke), `docs/Offene_Punkte.md` 1.408.0,
  Harfenbericht §16, Gitarrenbericht KQ35 §5, README der Körper, `InstrumentBody.md`.

## Zahlen

- Harfe, Lage S(1,2)–S(1,6): −6,4 / +4,0 / +0,3 / +2,5 / −1,5 %; Zahl der Bäuche gleich; Luftmoden 134,2 Hz Q 18,2
  und 198,3 Hz Q 11,5. Je Taste ab D2: Spanne 21,9 LU, größter Sprung 14,5 LU (78 / 80), mittlerer 3,3 LU.
- Gitarre gegen Elie 2012 an der Saite 1, 1–4 kHz: Kreuzterm −14,6 dB (−12,9), Yyy −13,3 dB (−15,6). Je Taste:
  Spanne 12,7 LU, größter Sprung 12,7 LU (68 / 71), mittlerer 1,7 LU.
- **Offen:** der größte Sprung je Taste liegt an Harfe und Gitarre über KQ34 a (12 LU), an der Harfe auch der
  mittlere (3 LU); die Höhe der Bäuche an der Harfe.

## Tests

`*Loop Koerper*,*Loop Werte*,*Klangfassung*,*Hammer*` ohne `*KQ18 Drehfeder*,*KQ20*` 111 / 111 unter Release-clang
und Testing-MSVC; kein voller Lauf; **1923 Cases**.
