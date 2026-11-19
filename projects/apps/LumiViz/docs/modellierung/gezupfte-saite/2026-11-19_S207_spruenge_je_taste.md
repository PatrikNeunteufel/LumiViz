# Sprünge der Lautheit je Taste über KQ34 a: Ursache und Messgröße (S207, 34k-6, KQ36 a)

> **Sitzung:** S207 (2026-11-19, Fable 5.2). Auftrag: Handover S206 Punkt (1) — Harfe größter Sprung 14,5 LU (78 / 80)
> und mittlerer 3,3 LU, Gitarre größter Sprung 12,7 LU (68 / 71), alle über KQ34 a (12 / 3 LU).
> **Version:** 1.5.0 (S208: §10 Harfe am Übergang Metall / Darm, §11 Bericht zu Auftrag 35 der Werkbank)
> **Proben:** `proben/2026-11-19_S207_*` (Kopf jedes Skripts sagt, was es rechnet; Ausgaben daneben). Messungen: g34
> des Archivs Mores (`E:\samples\Klasse 2 - Gezupfte Saite\Mores_2021_Zenodo4604577\qualified_selected_impulses.mat`),
> TU Berlin (`…\TU_Berlin_Weinzierl_DepositOnce19858\`, nur die ff-Aufnahmen, CC BY-NC-SA 4.0 — im Repo liegen nur
> die daraus gemessenen Lautheiten je Mikrofon). **Kein Programmcode, kein Körper, kein Test geändert.**

## 1 Ergebnis

Die Sprünge kommen nicht aus Saite, Steg oder Fit. Das Modell hört an einem Punkt (Harfe: `front`, 1 m vor dem
Boden) oder an zwei (Gitarre: Mikrofone 10 cm über der Decke); die Schwellen von KQ34 a stammen aus dem
Leistungsmittel über 32 Mikrofone in rund 2 m. Gleich gemessen (16 Hörorte in 2,06 m) liegt die Harfe bei Spanne
20,0 LU, größtem Sprung 12,8 LU (43 / 47) und mittlerem 3,0 LU; die Gitarre liegt mit 12,7 LU innerhalb dessen, was
die echte Gitarre an Mikrofonpaaren zeigt. Entschieden mit **KQ36 a** (Konzept §6).

## 2 Zerlegung

Prüfstand `2026-11-15_S199_pruefstand_zupf.py`; je Taste die Kraft am starren Steg je Teilton (K-bewertet), |p/F| je
Hörort und der Anteil des Teiltons an der K-bewerteten Energie (`…_gitarre_zerlegung.py`, `…_harfe_zerlegung.py`).

| | Gitarre 68 / 71 (Saite 1) | Harfe 78 / 80 |
|---|---|---|
| Anteil des Grundtons | 77 % / 93 % | 54 % / 99 % |
| Kraft am Steg am Grundton | −13 / −11 dBK | 2 / 3 dBK |
| Lautheit mit starrem Steg | −7,1 / +5,6 LUFS (12,7 LU) | −26,3 / −9,6 LUFS (16,7 LU) |
| \|p/F\| am Grundton | −3,9 / −0,2 dB → +11,0 / +10,0 dB (links / rechts) | −39 dB → −16 dB |
| größter Sprung mit \|p/F\| im Terzmittel | 7,9 LU (Saite 1) | 5,9 LU (Tasten 72–84) |

- **Gitarre:** das Tal bei 415 Hz macht die schmale Mode 407,1 Hz (Q 42) — oberhalb ihrer Resonanz löscht sie den
  Ausläufer der Mode 200 Hz; die Spitze bei 494 Hz macht die Mode 504,6 Hz (Q 14) mit dem größten Abstrahlgewicht
  aller Moden unter 1,1 kHz (0,33 / 0,24 gegen 0,20 / 0,17 der Mode 200 Hz) (`…_gitarre_moden.py`).
- **Harfe:** Taste 80 (831 Hz) liegt an der Mode 841 Hz; bei Taste 78 (740 Hz) löschen sich am Hörort die Beiträge
  der Moden 329, 233 und 698 Hz aus.

## 3 Die Gitarre gegen die Messung g34

|p/F| am Schlagort `treble`, Modell mit dem Fenster der Messung (`…_gitarre_gegen_g34.py`):

| | 415 Hz | 494 Hz | Schritt 68 → 71 |
|---|---|---|---|
| Messung `mbass` / `mtreble` (= `left` / `right`) | −4,0 / −1,0 dB | +8,6 / +8,5 dB | +10,8 dB (Leistungsmittel) |
| Modell `left` / `right` | −2,2 / +0,2 dB | +11,0 / +10,0 dB | +11,3 dB |
| Messung `east` (nicht im Körper) | +8,3 dB | −1,0 dB | −9,2 dB |
| Messung, alle drei Mikrofone | | | +2,8 dB |

Über die 20 Grundtöne der Saite 1: Modell − Messung links −1,8 dB (Streuung 2,6 dB), rechts −3,7 dB (Streuung
4,7 dB; Taste 75: −19,5 dB). Der Sprung ist an diesen zwei Punkten gemessen; an einem dritten Punkt im Nahfeld läuft
er umgekehrt. Ein Fernfeld lässt sich aus drei Nahfeld-Mikrofonen nicht rechnen.

## 4 Die TU-Aufnahmen je Zahl der Mikrofone

Je Ton das Momentary-Maximum jedes der 32 Mikrofone (`…_tu_je_mikrofon.m`, Octave 11), Kennzahlen für das
Leistungsmittel über k Mikrofone (`…_tu_auswerten.py`: k = 1 alle 32, k = 2 alle 496 Paare, sonst 500 gezogene
Auswahlen). Größter / mittlerer Sprung in LU, Median (5.–95. Perzentil):

| k | Gitarre ff | Harfe ff ab D2 |
|---|---|---|
| 1 | 11,1 (10,1–14,9) / 2,8 (2,4–3,3) | 13,9 (11,6–16,3) / 3,1 (2,6–3,8) |
| 2 | 10,5 (9,7–13,3) / 2,7 (2,3–3,0) | 11,9 (10,5–14,0) / 2,8 (2,4–3,3) |
| 4 | 10,4 (9,8–11,2) / 2,6 | 11,3 (10,2–12,6) / 2,6 |
| 32 | 10,3 / 2,5 | 11,1 / 2,5 |

An einem einzelnen Mikrofon hält die echte Harfe den größten Sprung ≤ 12 LU an 12 % der Positionen, den mittleren
≤ 3 LU an 38 %; die Gitarre an 59 % und 78 %. **Ein Überschlag aus der Streuung über die Mikrofone (unabhängig von
Ton zu Ton gezogen) gab für die Gitarre mit zwei Mikrofonen 13,6 LU und war falsch** — die Abweichungen je Mikrofon
hängen von Ton zu Ton zusammen.

## 5 Das Modell wie die Aufnahme gemessen

**Harfe.** Kopie des Bauplans mit 16 Hörorten auf der vorderen Halbkugel (2,06 m um die Mitte des Bodens,
Fibonacci-Gitter, mindestens 10° über der Platte; `…_harfe_viele_orte_plan.py`), Körper mit `InstrumentBodyTool …
--to 5000` (158 s), Prüfstand je Taste und Ort (`…_harfe_viele_orte.py`, Tabelle `…_harfe16_mik.tsv`):

| k | größter Sprung | mittlerer Sprung | Spanne |
|---|---|---|---|
| 1 | 16,2 (14,2–18,6) | 3,5 (2,9–3,9) | 23,8 |
| 2 | 14,3 (12,3–17,0) | 3,1 (2,6–3,5) | 21,2 |
| 4 | 13,2 (11,8–14,9) | 3,0 | 20,2 |
| 16 | 12,8 (43 / 47) | 3,0 | 20,0 |
| heutiger Ort `front` (1 m) | 14,5 (78 / 80) | 3,3 | 21,9 |

Größte Paare im Mittel der 16 Orte: 43 / 47 12,8 LU, 44 / 47 12,3, 58 / 62 11,9, 51 / 55 10,9, 64 / 67 10,4; das
Paar 78 / 80 liegt bei 6,9 LU. Die echte Harfe hat ihre größten Paare an derselben Stelle (45 / 48 11,1 LU,
41 / 44 9,5, 51 / 55 9,4) — am Übergang von den Metall- zu den Darmsaiten. Bei jeder Zahl von Orten liegt das Modell
rund 2 LU über der echten Harfe.

Eine Näherung ohne neuen Körper — |p/F|² als Summe über die Moden statt als Summe am Hörort
(`…_leistungssumme.py`) — gibt Gitarre 12,3 / 7,6 / 1,4 LU (Spanne / größter / mittlerer) und Harfe 15,5 / 15,0
(64 / 67) / 1,9 LU; sie überzeichnet Tasten auf einer Mode und ersetzt die Rechnung mit Hörorten nicht.

**Tasten auf einer Mode.** Taste 58 (233,1 Hz) trifft die Mode 233,0 Hz, Taste 64 (329,6 Hz) die Mode 328,5 Hz,
Taste 80 (830,6 Hz) die Mode 841 Hz. Lautheit am Ort `front` (`…_harfe_guete.py`):

| | 58 | 60 | 62 | 64 | 67 | 78 | 80 |
|---|---|---|---|---|---|---|---|
| wie Datei | −5,8 | −14,9 | −18,7 | −5,0 | −17,9 | −26,4 | −11,9 |
| Verlustfaktor × 2 | −8,6 | −15,7 | −20,0 | −9,7 | −18,4 | −26,6 | −17,0 |
| Verlustfaktor × 4 | −12,1 | −17,1 | −21,9 | −14,7 | −19,0 | −27,6 | −24,0 |
| Moden +3 % | −10,6 | −12,9 | −15,4 | −11,9 | −18,0 | −23,2 | −16,2 |
| Moden −3 % | −7,7 | −16,0 | −19,5 | −8,9 | −17,3 | −25,3 | −12,2 |

Der Körper ist ohne die Last der Saiten gerechnet; Bell nennt für die besaitete Harfe niedrigere Güten (Offene
Punkte §6 (22)).

## 6 Grenzen

- Die 16 Orte liegen nur vor der Platte, in Plattenkoordinaten; die Lage der TU-Mikrofone zur Harfe und die
  Abstrahlung nach hinten sind nicht nachgebildet.
- Das Leistungsmittel aus den Maxima je Mikrofon ist eine Näherung (die Maxima liegen nicht immer im selben
  Fenster); an den TU-Daten gibt sie dieselben Kennzahlen wie das Mittel der Leistung (10,3 / 2,5 und 11,1 / 2,4–2,5).
- Die Gitarre lässt sich nicht im Fernfeld prüfen; ihr Vergleich läuft gegen die TU-Gitarre (Kohno 1985, ein anderes
  Instrument) bei gleicher Zahl von Mikrofonen, im Nahfeld gegen 2 m.
- Die Höhe der Bäuche an der Harfe (Harfenbericht §16) ist nicht untersucht.

## 7 Laute, Oud, Cembalo nach KQ36 a

Prüfkörper je Instrument mit `…_viele_orte_plan.py <name> <ordner>` (16 Orte in 2,06 m um die Mitte des Umrisses,
der bisherige Ort als erster) und `InstrumentBodyTool … --to 5000` (53 / 97 / 367 s); Prüfstand je Taste und Ort
`…_viele_orte.py` (Saiten, Zupf und Tasten wie `chor_instrument` / `tasten_instrument`), Tabellen
`…_kq36_<name>_mik.tsv`, Auswertung `…_kq36_<name>_auswertung.txt`. Die Moden der vier Prüfkörper sind die der
mitgelieferten Körper (`…_koerper_gleich.py`: Frequenz, Verlustfaktor, Formwerte, Abstrahlgewicht des ersten Orts und
statistischer Teil gleich).

Spanne / größter / mittlerer Sprung in LU:

| | nach KQ36 a (Mittel der 16 Orte) | bisheriger Ort | KQ34 a | |
|---|---|---|---|---|
| Laute (38–79) | 12,9 / 8,4 (61 / 63) / 2,4 | 15,2 / 11,7 / 2,6 (`front`, 0,5 m) | 15 / 12 / 3 | hält |
| Oud (36–72) | **18,8 / 14,7 (40 / 44)** / 1,8 | 15,8 / 11,2 / 2,7 (`front`, 0,5 m) | 15 / 12 / 3 | Spanne und größter Sprung verfehlt |
| Cembalo (29–89), mit GZ13 a | 5,8 / 3,0 (83 / 84) / 0,6 | 7,1 / 6,1 / 1,4 (`above`, 1 m) | 15 / 12 / 3 | hält |
| Harfe ab D2 | 20,0 / 12,8 (43 / 47) / 3,0 | 21,9 / 14,5 / 3,3 (`front`, 1 m) | 22 / 12 / 3 | größter Sprung 0,8 darüber |

- **Cembalo:** der Prüfstand rechnet mit fester Ablösekraft; GZ13 a (2,8 dB je Oktave fallend, Bezug C4) ist
  nachträglich aufgelegt (`…_cembalo_gz13.py`, der Prüfstand ist in der Kraft linear). Ohne GZ13 a: Spanne 18,8 LU,
  Sprünge 3,6 / 0,9 LU.
- **Oud:** an den 16 Orten streut die Lautheit je Ton nur um 0,4 dB (Laute ebenso) — die Decke strahlt in 2 m fast
  ungerichtet; der Unterschied zum bisherigen Ort ist der Abstand, nicht die Richtung. In 2 m fällt der einzelne
  Basschor stärker ab als in 0,5 m: Taste 40 liegt 14,6 LU unter Taste 44 (in 0,5 m 11,2 LU), Taste 40 gegen 41
  8,0 LU (in 0,5 m 3,2 LU). Die Spanne verfehlte schon am bisherigen Ort (15,8 LU, KQ34 a).
- **Grenze:** mit 17 Hörorten zieht `lade_koerper` den statistischen Teil anders (je Hörort eine Phase aus derselben
  Folge): die gezogenen Moden über 5 kHz sind eine andere Ziehung als im mitgelieferten Körper. Am bisherigen Ort
  ändert das die Laute an Chor 8 um bis 2,1 LU (Taste 79), die übrigen Tasten und die Harfe um höchstens 0,4 LU.

## 8 Die Oud am Basschor (KQ37 a)

**Zerlegung** (`…_oud_zerlegung.py`, Tasten 36–48, Mittel der 16 Orte): ohne den Basschor (ab Taste 41) liegt die
Oud bei 11,4 / 10,3 / 1,6 LU; der Basschor liegt 12,9 LU unter dem Median der übrigen.

- Eine Saite statt zwei: bei gleicher Auslenkung (2 mm je Saite) ist die Kraft am Steg je Teilton 4–5 dB kleiner
  als an Chor 2.
- |p/F| in 2 m: 66 Hz −35 dB, 131 Hz −19 dB, 165–196 Hz −10 dB gegen +3 dB bei 208 Hz und +8 dB bei 262–277 Hz —
  der zweite und dritte Teilton des Basschors liegen im Tal zwischen der Luftmode (99,9 Hz) und den Moden 208 /
  269 Hz. Taste 43 trifft mit dem Grundton die Luftmode (70 % der Energie), Taste 44 mit dem zweiten Teilton die
  Mode 208 Hz (72 %), Taste 41 mit dem dritten die Mode 269 Hz (80 %).
- Die Tasten 38–40 treffen mit dem fünften Teilton die Moden 351 / 362 / 415 Hz, die den Steg stark bewegen
  (Formwert 7,8 / 3,6 / 9,3) und kaum abstrahlen; mit Körper sind die Tasten 4–9 LU leiser als mit starrem Steg.

**Zupf mit gleicher Kraft** (`…_oud_gleiche_kraft.py`, Pegel je Chor nachträglich verschoben): je Chor
14,1 / 13,1 (52 / 56) / 1,9 LU, je Saite 20,1 / 16,8 / 2,1 LU — keine der beiden Setzungen hält.

**Gegen die Messung der Abstrahlung** (`…_oud_gegen_degirmenli.py`; Değirmenli 2017, GU J Sci Part C 5(2), Abb. 7,
S. 29, in S207 am Seitenbild abgelesen auf rund ±10 Hz und ±1,5 dB: sieben türkische Ouds, Schalldruck je Kraft in
1 m, Hammer am Steg an drei Stellen, Terzglättung). Modell: Prüfkörper mit einem Hörort 1 m vor der Mitte der Decke
(`…_oud_plan_1m.py`), Mittel über die Stegpunkte der Chöre 1, 4 und 6, Terzglättung:

| Stelle | Messung: Hz, dB re 20 µPa/N, gegen die Hauptspitze | Modell: Hz, dB, gegen die Hauptspitze |
|---|---|---|
| unter der Luftmode (0,65 f_Luft) | 80, 53–59, −41 | 62, 70,7, −38 |
| Luftmode (erste Spitze) | 123, 81–87, −13 | 96, 92,5, −16 |
| Tal dahinter | 145, 70–72, −26 | 131, 76,2, −33 |
| Hauptspitze | 205, 93–101, 0 | 257, 108,8, 0 |
| Plateau | 300–450, 84–92, −9 | 370–560, 98,5, −10 |

Die Form stimmt auf 3–7 dB; das Tal ist gemessen. Zwei Abweichungen: **die Hauptspitze liegt im Modell beim
2,7-Fachen der Luftmode (257 Hz), an den gemessenen Ouds beim 1,7-Fachen (205 Hz)**, und das Modell liegt absolut
8–12 dB höher (bis 6 dB davon trägt die Schallwand der Rechnung; andere Instrumente).

**Gegen die Quelle des Bauplans** (Abdallah u. a. 2026, Abb. 6 S. 8 und Tab. 3 S. 10, am Seitenbild): die gemessene
Antwort hat Spitzen bei rund 100 / 139 / 197–210 / 280 / 360 Hz mit 1,2 / 3,3 / 6,5 / 2,8 / 2,4 m/s² je N — die
stärkste bei 204 Hz; Tab. 3 nennt als erste gemessene Mode 142 Hz mit einem Bauch in der Mitte der Decke. Im Modell
ist am Steg (Chor 1, Formwert² · Q) die Mode 207,8 Hz die schwächste unter 420 Hz (40 gegen 185 / 664 / 1877 /
3005 bei 99,9 / 168,6 / 269,4 / 351,3 Hz), sie trägt aber das größte Abstrahlgewicht (0,143 an `front`); die Mode
168,6 Hz bewegt den Steg stark und strahlt kaum (0,010). Der Bauplan wurde in S196 an der Lage der Spitzen
101 / 203 / 280 / 362 Hz ausgerichtet, die Spitze bei 139 Hz blieb ohne Gegenstück — **die Höhen und Formen sind
nicht verglichen** (Messort und Schlagort von Abb. 6 sind nicht der Stegpunkt; der Vergleich der Höhen ist deshalb
nur ein Hinweis). Verdacht: die Grundmode der Decke liegt im Modell bei 168,6 statt 139–142 Hz, und die Zuordnung
der Moden um 200 Hz stimmt nicht; dann fehlt dem Basschor Abstrahlung bei 130–250 Hz.

**Formen der Moden** (`…_oud_formen_bild.py`, Bild `…_oud_formen.png`; Bauplan im Repo, `InstrumentBodyTool --to 450
--shapes`). Auslenkung gegen den größten Betrag der Mode, und das verdrängte Volumen gegen das Integral des Betrags:

| Mode | Steg Mitte | Antwortpunkt 3 | Bauch bei y | Volumen | Form |
|---|---|---|---|---|---|
| 100,4 Hz | +0,80 | +0,90 | 13 cm | +1,00 | alles gleichsinnig, Bauch im Feld des Stegs (Luftmode) |
| 169,1 Hz | −0,61 | −0,57 | 29 cm | −0,02 | Stegfeld gegen das Feld am großen Loch |
| 208,8 Hz | **−0,28** | −0,41 | **19,5 cm** | −0,85 | gleichsinnig, Bauch zwischen den kleinen Löchern, nicht am Steg |
| 270,2 Hz | −0,91 | −0,51 | 31 cm | −0,61 | Stegfeld und Lochfeld gleichsinnig, Mitte dagegen |
| 352,2 Hz | +1,00 | −0,21 | 9 cm | −0,22 | drei Bäuche quer im Stegfeld |

Die Mode bei 209 Hz ist die gleichsinnige Deckenmode, hat ihren Bauch aber im mittleren Feld (zwischen Balken 2 und
3) und am Steg nur 28 % ihres größten Werts; den Steg trägt die Mode 270 Hz. An echten Ouds ist die Spitze um
200 Hz die stärkste — am Antwortpunkt 3 von Abdallah (203 Hz: 6,5 gegen 2,8 m/s² je N bei 280 Hz, +7 dB) und in der
Abstrahlung (Değirmenli). Am selben Antwortpunkt rechnete S196 (Bericht S196 §8, Mittel über das Anregungsraster)
209 Hz zu 0,23 gegen 1,0 bei 276 Hz (−13 dB) und vermerkte die Höhen als Grenze. **Der Abstand zwischen Messung und
Modell an dieser einen Größe ist rund 20 dB**; keine der in S196 gerechneten Fassungen (Decke 1,3–1,6 mm, Balken
wie gezeichnet oder am Rand) macht die Spitze um 200 Hz zur stärksten. Der Bauplan trifft die Lagen der Spitzen und
verfehlt, welche Mode der Steg treibt.

**KQ37 a (Patrik S207):** die Oud bleibt bekannte Abweichung mit Ursache; Auftrag an die Research-Werkbank
`2026-11-19_35_oud_basschor_einzeltoene` (Pegel einzelner Töne je Chor, Abstrahlung oder Stegadmittanz einer
arabischen Oud mit Zuordnung der Moden, Zupf des Basschors, Zug je Saite).

## 9 Offen

- Harfe: größter Sprung 12,8 LU an 43 / 47 (Schwelle 12), mit Mitschwingen 13,3 LU (44 / 47) — untersucht in §10:
  die Abstrahlung der Moden S(1,1)− und S(1,2) gegen Bell Abb. 7.23 und Le Carrou 2007 Abb. 7; KQ39 a, KQ40 a.
- Oud: Spanne 18,8 LU und größter Sprung 14,7 LU (40 / 44). **KQ38 a (Patrik S207):** den Bauplan neu suchen, mit
  zwei weiteren Zielen (am Antwortpunkt 3 die Spitze um 203 Hz die stärkste unter 400 Hz; Hauptspitze der
  Abstrahlung beim 1,7- bis 2,0-Fachen der Luftmode), über Deckendicke (auch als Verlauf), Balkenenden, Stegleiste,
  Hohlraum und Rand — der Bericht zu Auftrag 35 ist in S208 gegengelesen (§11), Ziel 2 nach KQ41 a im Band 1,5–2,0.

## 10 S208: die Harfe am Übergang Metall / Darm (43 / 47)

Proben `proben/2026-11-19_S208_*` (Ausgaben daneben), Prüfkörper mit 16 Hörorten wie §5 neu gerechnet (163 s).
**Kein Programmcode, kein Körper, kein Test geändert.**

**Die Saiten am Übergang.** Die oberste Metallsaite ist das G (Bell Tab. 2.2: 92 Hz leer); die Tasten 43 (G2) und 44
(G♯2, Pedal oben) spielen sie, 45–47 die zwei tiefsten Darmsaiten (104 / 116 Hz leer). Das ist der Bau jeder
Konzertharfe: Camac 2022 (Saitentabelle, Textebene) besaitet bis A 33 mit Darm, ab G 34 mit Metall. Die „Auswahl der
Harfensaiten“ aus dem Handover S207 ist die Auswahl der mitschwingenden Saiten und mit GZ8 a entschieden.

**Zerlegung** (`…_harfe_uebergang.py`, Mittel der 16 Orte):

| | 43 Metall | 44 Metall | 45 Darm | 47 Darm |
|---|---|---|---|---|
| Zug / Kraft der Kraftstufe | 634 N / 10,1 N | 634 N / 10,8 N | 301 N / 5,0 N | 299 N / 5,2 N |
| Lautheit | −16,8 LUFS | −17,2 | −26,3 | −29,6 |
| Teilton mit dem größten Anteil, \|p/F\| | 2. (195 Hz) −17,0 dB, 99 % | 2. (206 Hz) −18,6 dB, 98 % | 2. (220 Hz) −23,2 dB, 84 % | 1. / 2. (123 / 246 Hz) −38 / −29 dB |

Gegenproben, Tasten 38–56: die Taste 44 auf der As-Saite (Darm) statt auf dem Gis — größter Sprung 12,8 LU (43 / 47),
unverändert; Metallsaiten mit 80 % der Masse eines vollen Kupferrings (Zug 517–552 N) — 11,8 LU; gleiche Kraft je Taste
— 8,5 LU an 43 / 47, größter dann 10,6 LU (51 / 55), aber das Metallregister wird noch leiser (unten).

**Gegen die echte Harfe** (TU ff, Leistungsmittel der 32 Mikrofone): 41–43 −8,5 / −8,9 / −11,6, 45 −8,7, 48–51
−19,9 … −17,9, 54–56 −10,6 … −7,8 LUFS. Die echte Harfe hat keine Stufe zwischen Metall und Darm; ihre Senke liegt bei
48–51 (131–156 Hz). Gegen die Darmsaiten 52–56 ist das Metallregister 38–44 in der Aufnahme 1,6 LU lauter, im Modell
4,5 LU leiser — **die Kraft der Metallsaiten ist nicht zu groß**; zu leise sind im Modell die Tasten 45–51 gegen beide
Seiten.

**Mitschwingen nach GZ8 a** (`…_harfe_mitschwingen_16.py`, 20 Saiten, Pedale in der Mitte, die gespielte Saite mit
ihrem Pedal; rund 50 s je Taste): Spanne / größter / mittlerer Sprung **20,9 / 13,3 (44 / 47) / 3,1 LU** gegen 20,0 /
12,8 (43 / 47) / 3,0 ohne. Weitere Paare 44 / 48 12,1, 58 / 62 12,0, 43 / 47 11,8 LU. Das Mitschwingen ändert nur
Tasten, deren Grundton auf einer Saite in der Mitte liegt (43 −1,9, 48 −2,0, 52 −1,7, 57 +1,4 LU), die Tasten mit Pedal
oben kaum (≤ 0,2 LU). Die Last der Saiten gleicht also nichts aus.

**Die Abstrahlung der tiefen Moden** (`…_harfe_gegen_bell_723.py`; \|p/F\| an der Öse jeder Saite bei ihrer leeren
Frequenz, gegen Saite 26 = 207 Hz):

| Saite | 33 | 32 | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 | 23 | 22 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Hz | 104 | 116 | 123 | 139 | 155 | 165 | 185 | 207 | 233 | 247 | 277 | 311 |
| Bell Abb. 7.23 (besaitet, dB) | −18 | −16 | −23 | −24 | −26 | −20,5 | −2 | 0 | −8 | −4 | +2,5 | 0 |
| Modell, 16 Orte | −27,6 | −22,6 | −17,7 | −7,7 | −11,2 | −9,8 | −2,9 | 0 | +11,7 | +0,6 | −6,4 | −7,1 |
| Modell − Bell | −9,6 | −6,6 | +5,3 | **+16,3** | **+14,8** | **+10,7** | −0,9 | 0 | **+19,7** | +4,6 | −8,9 | −7,1 |

Bell Abb. 7.23 (PDF 309) und Le Carrou, Gautier & Foltête 2007 Abb. 7 (S. 564, Camac, \|p/F\| an Punkt C in 2,35 m)
in S208 am Seitenbild von Auge gelesen (±1 dB bzw. ±0,5 · 10⁻³ Pa/N). Über alle 24 Saiten liegt das Modell im
Median +0,2 dB neben Bell (Streuung 8,0 dB), ab Saite 24 ohne Muster. Le Carrou gegen 204 Hz, Modell an der Öse der
Saite 21: 110 / 125 / 130 Hz −39 / −39 / −25 gegen −19 / −12 / −8 dB; 225 / 240 / 250 Hz −13 / −17 / −17 gegen
+10 / +12 / +5 dB.

Beide Messungen an zwei Harfen sagen dasselbe: **an der echten Harfe strahlt der Körper unter rund 170 Hz 16–26 dB
schwächer ab als um 200 Hz, im Modell nur 8–11 dB; und die Mode S(1,2) strahlt im Modell um 20 dB zu stark.** Die
Lage der drei Moden passt dazu: Modell 134 / 198 / 233 Hz, Bell in der besaiteten Harfe 187 / 207 / 257 Hz (S(1,1)−
−28 %, S(1,1)+ −4 %, S(1,2) −9 %; Bell lose → eingebaut → besaitet 153 → 170 → 187 Hz, §8 des Harfenberichts). Die
Güte der Mode S(1,2) besaitet ist gemessen (Bell Q 32) und liegt beim Modell (34) — eine kleinere Güte wäre eine
Anpassung gegen diese Messung. **Ursache im Bauplan offen**: das Modell liegt mit 134 Hz noch unter Bells losem Kasten
(153 Hz), obwohl es die Schale starr rechnet; Kandidaten sind der Luftraum (Volumen, Öffnungen, Mündungskorrektur, die
die Kopplung der Helmholtz-Luft mit der ersten Bodenmode bestimmen), der Halt am Rand (gestützt, Feder 0, aus S198) und
die Last aller 46 Saiten als Steife an den Ösen — keiner gerechnet.

**KQ39 a (Patrik S208):** KQ34 a misst die Harfe mit Mitschwingen nach GZ8 a am Prüfkörper mit 16 Hörorten (`…_harfe_mitschwingen_16.py`). **KQ40 a (Patrik S208):** der Harfenkörper bekommt die Lage von S(1,1)− und S(1,1)+ (Bell in der Harfe ohne Saiten 170 / 216 Hz, 7 %) und die Abstrahlung an den Saiten 33–22 gegen Bell Abb. 7.23 (6 dB, Bezug Saite 26) als Ziele; die Ursache wird im Bauplan gesucht (Luftraum, Halt am Rand, Last der Saiten), mit Fable; danach Körper neu, KQ31-2 a und KQ34 a neu.


## 11 S208: der Bericht zu Auftrag 35 (Oud) gegen KQ37 a und KQ38 a

Abgelegt unter `recherche/2026-11-19_S208_oud_basschor_werkbank.md`, der Kopf sagt, was gegengelesen ist (Değirmenli
2023 S. 527–531 am Seitenbild, Abb. 3 in 300 dpi, D'Addario im HTML). PDFs unter `…\cmake\quellen\gezupfte-saite\`.

**Pegel je Chor: nicht gefunden.** Die einzige bekannte Messreihe (leere Saiten von zehn Ouds, mechanisch gezupft,
Mikrofon in 30 cm) steht in der Dissertation Değirmenli 2018, die der Lauf nicht erreicht hat (YÖK Açık Bilim nicht
auflösbar); die Arbeit von 2023 wertet sie nur als Hörversuch aus. **KQ37 a bleibt** — seine Kippbedingung (der tiefe
Chor echter Ouds fällt um 10 LU oder mehr ab) ist nicht belegt, und die Abstrahlkurven unten sprechen dagegen.

**Zug:** D'Addario EJ95A (arabisch, CFADGC, einzelnes C2): C2 8,0 lbs = 35,6 N gegen den F2-Chor 2 × 6,3 lbs = 56,0 N,
also −3,9 dB Kraft bei gleichem Winkel — die erste Ursache aus §8 (Modell 4–5 dB) der Größe nach bestätigt. Mensur
nennt die Seite nicht.

**Abstrahlung** (Değirmenli 2023 Abb. 3, drei türkische Ouds, Schalldruck je Kraft nach Hammerschlag am Steg, über 36
Richtungen gemittelt, Abstand und Bezug ungenannt, ungeglättet; in 300 dpi nachgelesen): an den zwei lauten Ouds (Ud 2,
Ud 6) liegt die erste Spitze bei 125–140 Hz (rund 92 dB), ein Tal bei 140–160 Hz (Ud 6 bis 57 dB bei 155 Hz), das
Maximum breit bei 170–225 Hz (96–103 dB). Der zweite und dritte Teilton des Basschors liegen dort 0 … −8 dB unter dem
Maximum unter 300 Hz, **im Modell −27 / −18 dB** unter der Spitze 262–277 Hz. Verhältnis der zwei stärksten Spitzen
unter 300 Hz: Değirmenli 2023 rund 1,5–1,6 (130–135 → 200–205 Hz), Değirmenli 2017 1,7 (123 → 205 Hz), Modell 2,7
(96 → 257 Hz in 1 m). Die leise Oud (Ud 7) ist flacher (120–250 Hz 73–82 dB).

**Moden einer fertigen, besaiteten Oud** (Değirmenli 2023 Abb. 1, Ud 2, Saiten gestimmt und gedämpft; „Sönüm %“ als
Dämpfungsgrad gelesen, Q = 1/(2ζ), die Quelle definiert es nicht) gegen den Körper `oud.lvinstbody` (Q = 1/η; Formen
aus §8):

| gemessen | Form am Bild | Modell | Form |
|---|---|---|---|
| 72,7 Hz, Q 15 | großflächig, mit dem Hals | — | Schale und Hals starr gerechnet |
| 117,7 Hz, Q 25 | ein Bauch im unteren Feld | 99,9 Hz, Q 26 | Luftmode, gleichsinnig, Bauch im Stegfeld |
| 177,8 Hz, Q 64 | Bauch um die Rosette, Knotenlinie quer darunter | 168,6 Hz, Q 32 | Stegfeld gegen das Feld am großen Loch |
| 206,0 Hz, Q 15 | ein Bauch im unteren Feld, Lage wie 117,7 Hz | 207,8 Hz, Q 35 | gleichsinnig, Bauch zwischen den kleinen Löchern |
| 260,8 Hz, Q 45 | zwei Bäuche übereinander | 269,4 Hz, Q 42 | Steg- und Lochfeld gleichsinnig, Mitte dagegen |

Lesart: die Folge der Formen stimmt (zwei einbäuchige Moden, dazwischen eine mit querer Knotenlinie, die schmal ist
und kaum abstrahlt — gemessen Q 64, im Modell strahlt 168,6 Hz kaum). Das Verhältnis der beiden einbäuchigen Moden ist
gemessen 1,75 (Abdallah am Antwortpunkt 3: 203 / 101 = 2,0), im Modell 2,08. Die Mode um 206 Hz ist gemessen breit
(Q 15), im Modell schmal (Q 35) — das hinge an der Lesart von „Sönüm“. **Die Lage der Bäuche gegen den Steg ist am Bild
nicht bezifferbar** (Steg nicht eingezeichnet, kein Maßstab); den Verdacht aus §8 (die Mode 207,8 Hz hat am Steg nur
28 %) bestätigt das Bild weder, noch widerlegt es ihn — ihn prüft weiter nur Ziel 1 von KQ38 a (Abdallah,
Antwortpunkt 3).

**Für KQ38 a:** Formen oder eine Stegadmittanz einer **arabischen** Oud bringt der Bericht nicht; die Ouds bei
Değirmenli sind türkisch (kleiner, Stimmung nicht genannt). Nach dem Wortlaut von KQ38 a kommt damit kein Ziel dazu, die
Suche kann beginnen. **KQ41 a (Patrik S208):** Ziel 2 von KQ38 a gilt im Band 1,5–2,0 (Değirmenli 2017 1,7, Değirmenli 2023 1,5–1,6,
Abdallah am Antwortpunkt 3 2,0); Formen und Güten aus Değirmenli 2023 Abb. 1 prüft die Suche nur als Bericht. Für eine Zahl zum Pegel des Basschors an einer echten Oud bliebe die Dissertation Değirmenli 2018
(Ladeliste im Bericht, Handle 20.500.12812/581867), von einem anderen Netz aus vielleicht erreichbar.
