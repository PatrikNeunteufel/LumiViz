# Modell Gezupfte Saite (Deluxe) — gebaut (Klasse 2, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/02_zupf_saite/` rechnet.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Zupf.hpp/.cpp`, `Vorgaben.cpp`. **Tests:** `test_zupf.cpp`
> (6 Testfälle). **Offen:** [offen.md](offen.md). **Entscheide:** [offene-fragen.md](offene-fragen.md) F-1 … F-10 a.
> Spielbar: **Cembalo, Harfe, Gitarre (Deluxe), Laute, Oud** (alle ungemessen); **Koto zurückgestellt** (K2 F-1 a).

## 1. Aufbau (K2 F-1 a Stufe 1: zwei Polarisationen, 2×2-Admittanz am Steg)

**Saite:** je Polarisation (senkrecht/parallel zur Decke) zwei Verzögerungen (Steg–Zupfpunkt, Zupfpunkt–Sattel) als
Schleife, Lagrange 3; alle Leitungen vor dem Schreiben gelesen (exakte Laufzeit); am Sattel fest mit Verlust je Umlauf
(konstante Güte, parallel 10 % mehr Verlust — offen). **Steg** nach Bank & Karjalainen 2010 (**Quelle**): je Mode ein
Modalfilter mal d·dᵀ (Rang 1, positiv semidefinit, passiv); geschlossen gelöst (I + Z0·M)·u = u_vorher + 2Z0·M·v⁺.
**Zupfen:** ein Kraftstoß am Zupfpunkt in Richtung des Polarisationswinkels (Scherrer & Depalle, **Quelle**).
**Ausgang:** Schnelle des Stegs senkrecht, abgeleitet. **Feinstimmung** (Rechnung, `stimme`): Faktor auf die Laufzeiten
je dritte Note, so dass die Spitze des Grundtons auf der Note liegt. **Notenende:** Dämpfer (Cembalo) bzw. Hand (Harfe,
F-7 a), *laissez vibrer* als Ausnahme; **Lautenzug** (F-5 a): Güte ×⅛ (offen). **Tastenstoß** (F-4 a): ein Impuls in die
Stegmoden mit der Stärke. **Doppelchor** (F-9 a): zwei Saiten ± die halbe Verstimmung.

**Mitschwingen je Clip** (F-3 a, Deluxe; gebaut 2026-10-30): `rechneZupfClip` rechnet alle gespielten Saiten und die
`mitschwingNoten` gleichzeitig an **einem** Steg, geschlossen gelöst (I + ΣZ0ᵢ·M)·u = u_vorher + 2·M·ΣZ0ᵢ·v⁺ᵢ, die
Kraft auf den Steg Σ Z0ᵢ·(2v⁺ᵢ − u). Gitarre: die leeren Saiten E2 A2 D3 G3 H3 E4; Harfe: alle 47 Saiten in C-Dur
(Pedale in der Mitte — offen; Laute/Oud: Stimmung der Chöre offen, ohne). Eine Saite allein ist der Ton je Note (bis
auf die Rundung, Test). Die Harfe mit 47 Saiten rechnet 26 % der Echtzeit je Clip. Chor, Lautenzug und Tastenstoß gibt
es nur je Note.

**Abweichung vom Entwurf:** der
Korpus der Gitarre kommt aus dem Modalfit an Mores (F-2 a, lokal) — bis dahin die Spitzen von Wrights Abb. 7.1 mit
offenen Güten und Richtungen; Oshi-de (F-8 a) nicht gebaut; Pedale der Harfe seit 2026-10-30 gebaut (F-B5 a, `Pedale.cpp`).

## 2. Größen

| Größe | Cembalo | Harfe | Gitarre | Laute / Oud | Herkunft |
|---|---|---|---|---|---|
| Saite an der Bezugsnote | G4: 47,5 cm, 85,7 N | D♭2: 97,5 cm, 260 N | 650 mm, 70 N | 600 mm, 35 N | Chadefaux 2013 Tab. 2 (**Quelle**); Chadefaux 2012 PDF-S. 3 (**Quelle**); sonst **offen** |
| Länge über den Umfang | ∝ 1/f, Bass ∝ f^−0,85 | Bass ∝ f^−0,6 | fest | fest | offen |
| Zupfpunkt | 12 % (Bass) → 50 % (Diskant) | 50 % | 18 % | 18 % | Välimäki 2004 S. 936 (**Quelle**); sonst offen |
| Polarisation beim Ablösen | 18° | 25° | 30° | 30° | Chadefaux Tab. 1: 17/20° (**Quelle**); Harfe 7–43° (Bericht); sonst offen |
| Kraft pp … ff | 0,6 … 1,0 N | 1 … 8 N | 0,5 … 4 N | dto. | aus Auslenkung 0,71 mm bzw. 4 mm (Rechnung); offen |
| Steg | 37 Hz + 9 Moden, Q = π·f·0,235/ln 1000 | 5 Moden (offen) | Spitzen Wright Abb. 7.1 (112 … 880 Hz) mit deren Pegeln, Q 30 | 5 Moden (offen) | Dulcken: Helmholtz 37 Hz, T60 235 ms (**Quelle**); Wright (Bestand); Stärken, Richtungen **offen** |
| Güte der Saite | 1500 | 600 | 800 | 500 | **offen** |
| Doppelchor | — | — | — | ±2 / ±1,5 Cent | offen |

## 3. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Tonhöhe jede fünfte Note, alle fünf Instrumente | ±5 Cent (Stichprobe ±3,5) |
| **Zweistufiges Abklingen** (Bank & Karjalainen) an G3 der Gitarre | Rate früh/spät 4,6 mit Kopplung gegen 1,4 ohne |
| Cembalo: Zupfpunkt 50 % oben (Välimäki) | H2/H1 −87 dB im Diskant gegen +3,6 dB im Bass |
| Lautenzug, Tastenstoß (Penttinen: +10 dB je Stufe) | kürzer; +4,4 dB unter 300 Hz ff gegen pp (kleiner, offen.md) |
| Harfe: Hand dämpft, laissez vibrer klingt; Koto still | grün |
| Determinismus, Abdruck (gcc = clang), Rechenzeit Gitarre E2 | 0,3–0,5 % |

## 4. Hörproben

`ic_hoerproben <Ordner> 2`: je Instrument drei Noten × pp/mf/ff (2,5 s), abgedämpft; Cembalo Lautenzug, Harfe laissez
vibrer — 52 WAVs. Gehört: noch nicht (L5).
