# Changelog Session 146 (2026-10-23)

> **Der Flügel klingt — als gerechnete Saite.** Eine Schicht *Hammer* + *Modal* rechnet einen Ton wie ein
> Konzertflügel: Filzhammer, zwei oder drei Saiten je Taste mit Schwebung und zweistufigem Ausklang, gestreckte
> Stimmung, Längsschwingung, Dämpfer. Laut gespielt klingt sie heller, nicht nur lauter. Noch ohne Resonanzboden
> und noch nicht als mitgeliefertes Instrument — beides folgt.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (HK1 a, HK2 a, HT1 a bis HT7 a, KB1 a).

## Neu

- **Hammer-Saite** in der `.lvinst` (Fassung 16): an einer Saite die *Excitation* (`pluck`, `hammer`), das Verfahren
  *Modal*, der Block `hammerString` mit 15 Kurven über die Tastatur (Inharmonizität, zwei Abklingzeiten, Chor,
  Verstimmung, Hammer, Anschlagstelle, Dämpfer, Saitenlänge und -masse, Längsverhältnis, Winkel am Steg) und am
  Instrument der Block `keyboard` (Umfang, erste Taste ohne Dämpfer, Pedale je Platz).
- **Ein Ton** nach Bank, Zambon & Fontana 2010, mit Saiten- und Hammerdaten eines Steinway D (Chabassier & Duruflé
  2012). Gemessen: die Teiltöne liegen auf 1 Cent dort, wo die Inharmonizität sie hinlegt; ff gegen pp ist am
  achten Teilton 11 dB heller; mit verstimmten Saiten zwei Stufen des Ausklangs; Phantomteiltöne im Bass.
- **Anschlagstärke** gestuft: der Ton eines Hammer-Instruments heißt nach seiner Stufe (`:a-6`), der Rest ist Pegel
  — die Lautstärke stimmt genau.
- **Ablage `docs/modellierung/`:** je Instrumentenklasse Recherchen, Modell, Messplan; fremde Arbeiten nur lokal.

## Geändert

- Die Planung erfährt über ein Instrument jetzt Saiten **und** Hammer (Auskunft `InstrumentVon` statt `SaitenVon`).
- Die Recherchen zur gezupften Saite (Korpus, Tonabnehmer, Spielweisen, Spread, verbundene Noten) liegen unter
  `docs/modellierung/gezupfte-saite/recherche/`.
- Wrights Dissertation (1996) ist nicht mehr im Repo („All rights reserved“).

## Intern

- `loop/HammerSaite`, `loop/HammerTon`, `loop/Auskunft`; `Instrument` (Anregung, `SaitenModell::Modal`,
  `Tastatur`), `TonAngabe::anschlagStufe`, `tonImClip(…, hammer)`.
- Neue Tests `test_LoopHammerSaite`, `test_LoopHammerTon`; 1520 Fälle. Bereich unter Release-clang grün,
  `werte/klangfassung.txt` neu (nur die Abdrücke der Eingaben); **kein voller Lauf**.
- Resonanzboden: sieben Recherchen (Geometrie, Mobilität, Abstrahlung, Numerik, Verfahren, gemeinsame Grundlage,
  offene Daten); vorläufig aus *Salamander* geschätzt, ein physikalischer Boden als nächster Schritt.
