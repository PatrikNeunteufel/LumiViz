# Changelog Session 219 (2026-11-24)

> Diese Sitzung hat KQ70 a geprüft: was ändert das Besaiten am tiefen Modenpaar der Harfe (Bell besaitet 187 / 207 Hz,
> Modell 174 / 217 Hz)? Drei Ursachen sind verworfen, die vierte (der Rahmen aus Sockel, Säule und Hals) braucht Maße,
> die Bell nicht nennt, und geht an die Werkbank. Bells Zahlen für das Paar sind schwächer belegt, als die Schwelle sie
> behandelte; sie sind jetzt Bericht statt Schwelle. Kein Programmcode außer dem Harfenfall im Test.

## Neu

- **KQ73 a (erledigt, gekippt):** die Wölbung des Bodens unter dem Saitenzug als Probe — verworfen als Ursache.
- **KQ74 a (ändert KQ57 a):** besaitet gelten als Schwelle die Lagen ab S(1,2) (257 / 334 / 430 / 555 / 697 Hz); das
  Paar 187 / 207 Hz ist Bericht mit Wert; der Gipfel des Modells bei 217 Hz am Messort 17 ist bekannte Abweichung;
  KQ70 a ruht bis zum Bericht der Werkbank zu Auftrag `2026-11-24_36_harfe_rahmen_masse`.
- Proben unter `modellierung/gezupfte-saite/proben/`:
  - `2026-11-24_S219_harfe_gipfel.py` — die Harfe gemessen wie bei Bell (Gipfel der Eingangsadmittanz je Messort,
    Saiten als Dämpfer, Schallprofile; `hebe=` hebt eine Mode auf Bells Höhe).
  - `2026-11-24_S219_harfe_rahmen.py` — ebener Rahmen aus Kasten, Sockel, Säule, Hals; Saiten als Federn zwischen Öse
    und Hals; Saitenzug als Vorspannung (geometrische Steife). Querschnitte und Massen angenommen.
  - `2026-11-24_S219_harfe_woelbung.py` — der Boden nach von Kármán unter der statischen Last der 46 Saiten, drei
    Fälle für den Halt der Ränder in der Ebene; Ableitungen gegen Differenzen geprüft.

## Geändert

- `test_LoopKoerperLuftraum`, Fall „Luftraum die mitgelieferte Harfe“: S(1,1)− und S(1,1)+ besaitet aus der Schwelle
  genommen (Frequenz weiter festgehalten, Abweichung als Meldung); neu die bekannte Abweichung „Gipfel am Messort 17“
  (216,75 Hz, 10,2 dB über dem höheren Tal zwischen 175 und 245 Hz; rot, wenn er wandert oder verschwindet).
  1930 Cases wie vorher.
- Harfenbericht `2026-11-13_S197_harfe_gelesen.md` §27, `docs/audio/Instrumente_IC_Konzept.md` 0.88.0,
  `docs/Offene_Punkte.md` 1.430.0.

## Befunde

| Vermutung | Probe | Ergebnis |
|---|---|---|
| 187 / 207 Hz sind Ablesungen an überlappenden Moden | Gipfel je Messort am Modell | verworfen: 169–171 / 214–218 Hz, höchstens 4 Hz neben den Eigenfrequenzen; auch unter einer S(1,2) von Bells Höhe bleibt 217 Hz |
| der Saitenzug spannt den Rahmen vor | Rahmenprobe mit geometrischer Steife | verworfen: keine Mode ändert sich um mehr als 3 Hz (Druck Säule 5,5–6,3 kN) |
| der Rahmen ordnet das Paar um | Raster über 12 Annahmen | nicht entscheidbar: kein Fall hält beide Zustände, die Lage der Rahmenmoden hängt an den angenommenen Querschnitten |
| die Wölbung des Bodens schwächt die Kopplung | von Kármán, drei Randfälle | verworfen: Wölbung 3,4 mm, Volumen der ersten Mode auf 1 % gleich, Paar 174,4–181,7 / 217,7–238,8 Hz |

Aus den Quellen: Bell liest 187 / 207 Hz an flachen Profilen mit Güte „indeterminate“ (Abb. 7.8 / 7.9), am Messort 17
steht besaitet zwischen 120 und 250 Hz kein Gipfel (Abb. 7.6); B2 fällt beim Besaiten von 135 auf rund 110 Hz
(Abb. 7.18). Le Carrou u. a. 2007 (andere Harfe, besaitet): Paar 152,2 / 168,5 Hz mit Q 22 / 36, drei Moden des ganzen
Kastens bei 55–123 Hz, Kasten so stark bewegt wie der Boden.

## Prüfung

`*Luftraum die mitgelieferte Harfe*` unter Release-clang und Testing-MSVC je 1 Fall, 729 Prüfungen grün. Geändert ist
allein diese Testdatei; kein anderer Fall liest die Stelle. Kein voller Lauf.

## Offen

- KQ70 a ruht: Auftrag 36 an die Werkbank (Maße, Massen, Verbindungen von Säule, Hals, Sockel; Moden der ganzen Harfe
  ohne und mit Saiten) ist gestellt, nicht gestartet.
- B2 135 → 110 Hz beim Besaiten ohne Erklärung; Luftmoden A3–A5 6–10 % zu tief; Abstrahlung an Saite 29; ohne Saiten
  tragen S(1,2)–S(1,5) im Modell 15–50 % Luft.
- Danach Herleitung §12 (Tastenstoß, 4′, Lautenzug), 34k-7.
