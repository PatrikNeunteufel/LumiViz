# Offen — Lippe und Rohr (Klasse 7)

> **Stand:** 2026-10-30, IC Session 4 (Cloud). Gebaut: [modell.md](modell.md). Alle spielbaren Instrumente
> **ungemessen**; Bezug Iowa MIS, TinySOL, VSCO 2 CE für Dämpfer (lokal, F-7 a).

| Größe | Stand | Quelle | Vorgabe | Frage |
|---|---|---|---|---|
| Trompete: Bohrung | **gebaut** (F-B4 a), ungemessen | Soares et al. 2025 Abb. 4 (Quelle, CC BY 4.0) — vereinfacht in 7 Abschnitten, kürzer als eine echte B♭-Trompete (1,355 m) | 29/30 Prüfpunkte (jede dritte Note pp/mf/ff) auf ±10 Cent; Feinprobe jede Note 83/87 | Kessel/Kehle/Seele als ein Volumen (Rechnung); am Prototyp gemessene Impedanz Abb. 8 zum Vergleich (lokal) |
| Trompete: Griffe, Ventillängen, Stimmzug | Rechnung | — | Griff am Modell (8 Teilmengen, Lippe im Sekantenverfahren); Ventile L·(2^(k/12) − 1); Stimmzug oberes Quartil | Griffe weichen teils vom Spielergriff ab; echte Ventilzüge (länger, Kombinationsfehler) offen |
| Trompete: Lippe, Ruhespalt, Dämpfer | übertragen | Posaune / Petiot 2023 | wie Posaune; keine Dämpfer | — |
| Tuba, Horn: Bohrung | **nicht spielbar** | Acta 2022 keine Tuba-Bohrung; Myers 2019 nicht mehr abrufbar; Hendrie 2007 (gelesen 2026-10-30) nur Prüfkörper und Naturhorn-Bögen, kein ganzes Instrument | Gerüst | weitere Suche (Auftrag); Kandidaten: Messungen an einem ganzen Horn (z. B. Pulsreflektometrie Kemp/Campbell, Edinburgh) |
| Didgeridoo: Bohrung | **da** (F-B5 a): Causse, Goepp, Sluchin (HAL-01161327) Abb. 2 S. 3, Didgeridoo 5 abgelesen — 1,465 m, 13,4 → 51,2 mm; erste Spitze 80,6 Hz (Causse: 60–88 Hz), zweite 187,9 Hz (×2,33) | Abbildung (abgelesen) | — | — |
| Didgeridoo: Lippe | **nicht spielbar** (Befund) | — | die Lippe der Posaune treibt keinen Bordun (nur ~2 kHz oder nichts, auch ohne Vokaltrakt, größerer Kessel) — der weite Eingang | Lippenwerte für das Didgeridoo mit Quelle (Fletcher 1983, Tarnopolsky) |
| Alphorn A2 (tiefste Hörprobe) | **Befund 2026-10-31, offen** (IC Session 5) | `ic_hoerproben` | der Ton wird ab Wert 2567 (53 ms) NaN, davor still — bei e0, e50 und e100; A3 (Abdruck `blech_Alphorn_m57_mf`) und A4 rechnen | Ursache am Modell suchen (Anblasschwelle, Stimmung der tiefen Lage?); bis dahin lässt die Sammlung den Ton aus und meldet ihn |
| Posaune: Profil des Schallstücks | offen | Willemsen: nur Anfangs- und Endradius | Bessel-Horn ε = 0,7 | — (Impedanz UNSW Boutin, lokal) |
| Posaune: Sprünge des Radius im Zug (0,69/0,72/0,74) | vereinfacht | Willemsen Tab. 1 | ein Zylinder r 0,7 cm | — |
| Zylinderverluste mit dem Zug | vereinfacht | — | bei Grundlänge + halbem Zug gerechnet | — |
| Mundstück: Hals und Seele (Trägheit) | **nicht gebaut** | IWK-Werte (nur Bericht) | nur der Kessel | — |
| Lippe µ, Q (Posaune) | offen | Petiot an der Trompete | 1,3 kg/m², 5 | — (M2, M3; 31-Spieler-Datensatz Zenodo 18679877 lokal) |
| Weg des Drucks: 1,3·P_s … 6·P_s | offen | — | Vorgabe | T3a a (Ort entschieden) — M2 |
| Zungenstoß 5 ms | offen | — | Anstieg | T3c a — M3 |
| Schmettern: Größe | offen | Myers: ×2,7 am Schallstück | Modell ×1,54 | — (M4 an Iowa ff); prüfen, ob die Zylinderlänge (nur Zug, ohne Schwanenhals/Stimmzug) zu kurz ist |
| Pegel über den Umfang | **auffällig** | — | Bass bis 50 dB leiser als die Höhe | — (Pegel an Iowa) |
| Alphorn: R1, Mundstück, Becher, Länge | offen | Marcinnò Tab. 3 ohne R1/R5; Fis-Horn 347 cm (Hersteller, nur Bericht) | 3,08 m — Reihe ≈ 3 Halbtöne hoch, nicht harmonisch | F-B1 (Rest der Geometrie) |
| Dämpfer: alle Werte | offen | nur Straight 77 Hz, Cup −20 dB (nur Bericht) | Helmholtz-Zweig | T3b a (Ort entschieden) — M5 an VSCO |
| Wah-Wah (Öffnungsgrad an der Note) | **gebaut** (T3b a), ungemessen | — | Harmon: Restfläche offen 0,05, zu 0,01 (offen); die Filter beider Stellungen linear gemischt (Näherung — die Zwischenstellung der Hand ist keine Mischung) | Restfläche zu und Mischung an einer Wah-Aufnahme (lokal) |
| Stopfen des Horns | **nicht gebaut** (Horn ohne Bohrung) | — | — | T3b a |
| Vokaltrakt: Spitze, Abbildung der Spur „Vokal“ u → i | offen | Fletcher 2006 (nur Bericht) | 2·10⁶ Pa·s/m³, unverändert | T3c a |
| Growl: Fläche der Stimmlippen, Mundraum | offen | — | 0,2 cm², 50 cm³ | T3c a |
| Halbventil | **nicht gebaut** (Deluxe, FDTD) | Bilbao DAFx-11 | — | T5 a |
| Ventile: Zusatzlängen | gebaut (Trompete), Länge im Zylinder, wechselt sofort | Rechnung | s. Trompete | — |
| Vibrato: Verhältnis Lippe → Tonhöhe | aus dem Modell, **nicht kalibriert** | — | Tiefe an der Lippe | T3a a („die Tiefe, die die angegebene Tonhöhenschwankung ergibt“) — Kalibrierung offen |

## Feinabstimmung (lokal, L6, T9 a)

Plan A–D nach L15–L18 steht im [messplan.md](messplan.md) (F-B6 a, 2026-10-30); Bezug Iowa (Posaune pp/mf/ff), Spielerdaten
Zenodo 18679877 (Mundstückdruck gegen Klang, CC BY 4.0, nur Bericht).
