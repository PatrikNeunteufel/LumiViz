# Befund: die IC-Vorlage der Zunge vor Stufe 34j (Klasse 10)

> **Typ:** eigene Messung am gerechneten Ton. **Stand:** 2026-11-11, LumiViz S188 (Planung 34j, nichts am Code).
> **Gemessen an:** `harvest/instruments-calculation/rechnung/klassen/10_zunge/` (IC-Commit `4409088`), gebaut unter
> Windows-clang mit den IC-Schaltern (`proben/2026-11-11_S188_bau.bat`). **Beleg des Baus:** der Abdruck
> `zunge_Akkordeon_m69_mf` ist `1a20c5e0…bae73c` wie in IC `werte/abdruck_msvc.txt`.
> **Proben:** `proben/2026-11-11_S188_ic_vorlage_messung.cpp/.txt` (IC-Abbildung), `…_ein_wind.cpp/.txt` (Gegenprobe).
> **Messkette:** Ton 1 s gehalten bei 48 kHz; Lautheit nach BS.1770-4 über die gehaltene Sekunde (IC `lautheit`);
> Tonhöhe und Einschwingen aus der IC-Messung des ersten Chors (YIN, `einschwingen`); Ausklang = letzter Wert über
> −60 dB der Spitze nach dem Ende der Note. Jede dritte Taste und die oberste.

## 1. Was die Vorlage rechnet

Eine Zunge je Chor als Masse-Feder mit Strömung nach Puranik & Scavone 2023. Über den Umfang ändert sich **nur die
Länge** (∝ 1/√f) und mit ihr die Masse; Breite, Ruhelage (0,3 mm), Seitenspalt, Kanzelle, Kanal und Ventil sind für
jede Taste gleich (`Zunge.cpp:38–42`, `offen.md` „Zungenmaße über den Umfang: Rechnung“). Die Expression wählt den Druck
**je Note** geometrisch in einem gestimmten Fenster (`Zunge.cpp:45–51`): dieselbe Expression gibt jeder Taste einen
anderen Druck.

## 2. IC-Abbildung (Expression im Fenster je Note)

| Größe | Akkordeon Diskant (53–93) | Akkordeon Bass (29–52) | Harmonium (48–89) | Melodica (53–84) | Mundharmonika (60–96) |
|---|---|---|---|---|---|
| Spielt auf ±10 Cent (e 0 / 0,5 / 1) | alle | **keine** (8 Tasten geprüft) | ab 60 alle; **48–57 fast stumm** (−42 … −53 LUFS) | alle | alle |
| Lautheit über den Umfang, e 0 | 25,9 LU | — | 52,8 LU | 17,9 LU | 28,9 LU |
| … e 0,5 | 21,8 LU | — | 50,8 LU | 19,3 LU | 17,8 LU |
| … e 1 | 23,5 LU | — | 48,9 LU | 23,6 LU | 15,4 LU |
| Dynamik e 0 → 1 je Taste | 10,1 … 28,1 LU | 0 … 7,7 LU | −0,6 … 5,6 LU | 13,6 … 28,0 LU | 4,3 … 25,9 LU |
| Druck bei e 0,5: tiefste / höchste Taste | 283 / 2 934 Pa | — | 3 490 / 5 382 Pa | 283 / 1 903 Pa | 476 / 3 805 Pa |
| Einschwingen e 0,5 | 55 … 215 ms | — | 365 … 525 ms | 125 … 215 ms | 105 … 185 ms |
| Einschwingen e 0 | 245 … 365 ms | — | 405 … 545 ms | 285 … 365 ms | 205 … 375 ms |
| Ausklang (−60 dB) nach dem Wind | 32 … 725 ms | > 1 s (Fenster zu Ende) | 31 … 34 ms | 54 … 777 ms | 32 … 199 ms |
| Amplitude der Spitze / Länge, e 1 | 5,0 … 10,9 % | — | 2,5 … 4,9 % | 5,0 … 10,9 % | 7,6 … 13,3 % |
| Bend −100 Cent verlangt (Mitte) | **+8,0 Cent**, −7,1 dB | — | +0,2 Cent, −5,4 dB | −0,9 Cent, −11,4 dB | +4,7 Cent, −3,2 dB |
| Bend −50 Cent verlangt | −0,1 Cent | — | +0,2 Cent | −0,2 Cent | 0,0 Cent |

Die Grenze des Wächters `Loop Pegel LP3 a` ist 15 LU. Bezug Cottingham 2011: Amplitude rund 15 %. Bezug Puranik &
Scavone: Harmonium 0,2–1,5 kPa „in a real harmonium“ — das gestimmte Fenster liegt bei 1,1–5,4 kPa.

**Lesart:**

- **Bass-Seite des Akkordeons:** stumm (−300 … −50 LUFS gegen −41 … −17,5 im Diskant bei e 1), die Fenster bestehen aus
  einem Punkt (`Stimmung.inc`, Tasten 30–36) oder fehlen (29, 37). In IC bekannt (`offen.md`), der IC-Test prüft nur
  „RMS > 0“.
- **Harmonium:** die unteren zehn Tasten (48–57) sprechen nicht an; darüber steigt die Lautheit um rund 20 LU bis zur Spitze, die
  obersten Tasten liegen bei 0 LUFS (der Bezug normiert an der Mitte auf Spitze 0,5 — oben übersteuert der Ton); die
  Dynamik fehlt fast ganz.
- **Gefälle der Lautheit:** bei allen vier über der Grenze von 15 LU, weil das Fenster mit der Tonhöhe nach oben
  wandert (unterer Rand 119 Pa an F3, 1 903 Pa an A6): ein Akkord bei e 0,5 bekäme an F3 283 Pa und an A6 2 934 Pa.
  Ein Balg hat einen Druck für alle Tasten (IC F-5 a: „gemeinsamer Balg“).
- **Bend:** die Abbildung Cent → Ventilöffnung trägt nicht; bei −100 Cent steigt die Tonhöhe und der Pegel bricht ein.

## 3. Gegenprobe: ein Winddruck für alle Tasten

Dieselben Zungen und dieselbe Stimmung (f₀), der Druck unmittelbar (`druckPa`), gleich für jede Taste.

| Druck | Akkordeon Diskant: spricht in 1 s an (Tasten) | Lautheit über diese Tasten | Melodica | Mundharmonika |
|---|---|---|---|---|
| 150 Pa | 53–71 | 6,1 LU | 53–71, 5,3 LU | 60–69, 9,2 LU |
| 300 Pa | 53–80 (80: −63,5 LUFS, noch im Anlauf) | 4,3 LU (53–77) | 53–80, 3,8 LU (53–77) | 60–75, 4,4 LU |
| 600 Pa | 53–86 | 3,9 LU (53–83) | 53–84 (alle), 4,5 LU | 60–81, 4,1 LU |
| 1 200 Pa | 56–93 (53 erstickt) | 9,4 LU; 59–86: 3,1 LU | 56–84, 9,6 LU | 60–90, 3,8 LU (60–87) |
| 2 400 Pa | 62–93 (53–59 ersticken) | 8,5 LU; 65–93: 2,8 LU | 62–84, 8,8 LU | 63–96, 7,4 LU |

Harmonium (200 / 400 / 800 / 1 500 Pa): es gibt keinen Druck, bei dem alle Tasten ansprechen — je
Druck spricht ein Band von anderthalb bis zwei Oktaven, das mit dem Druck nach oben wandert (400 Pa: 48–66; 800 Pa: 54–72;
1 500 Pa: 57–81).

**Lesart:** Wo die Zungen ansprechen, ist die Lautheit bei gleichem Druck über den Umfang fast eben (3–5 LU) — das
Gefälle aus §2 kommt aus der Abbildung, nicht aus dem Ausgang. Aber **die Ansprechschwelle steigt mit der Tonhöhe**
(Akkordeon: F3 ab unter 150 Pa, A6 erst ab rund 1 200 Pa mit 700 ms Anlauf) und tiefe Zungen ersticken über rund
1 kPa. Es gibt keinen Druck, bei dem alle geprüften Diskanttasten des Akkordeons in einer Sekunde ansprechen. Ursache nach §1:
über den Umfang ist nur die Länge gestuft; Dicke, Breite, Aufbiegung, Kanzelle und Kanal einer echten Stimmplatte
ändern sich mit der Tonhöhe. Das ist dieselbe Art Mangel wie bei Wurlitzer und Kalimba vor LP2 b
(`../anbindung-ic/2026-11-09_S182_pegel_ueber_den_umfang.md`).

## 4. Was daraus für 34j folgt

- Ein wortgleicher Umzug wie 34g–34i liefert vier Instrumente, von denen keines die Grenze LP3 a hält, mit stummer
  Bass-Seite, zehn fast stummen Tasten unten am Harmonium und einem Bend, der nicht biegt.
- Die Vorgaben je Taste (Zunge, Aufbiegung, Kanzelle, Kanal; Basszungen; Harmonium) brauchen eine Herleitung mit
  Quellen wie LP2 b. Die Arbeiten aus IC `ic/quellen.md` (Puranik & Scavone 2023, Adachi et al. 2026, Hergert 2023,
  Cottingham 2011) liegen **nicht** unter `…\cmake\quellen\zunge\` (Index `../../archiv/Index_quellen.md` ohne Treffer);
  sie sind frei (CC BY) und neu zu laden. Aufnahmen der Klasse (TinySOL Akkordeon, Euterpea, fese, VCSL-Mundharmonika)
  liegen ebenfalls nicht vor (`Index_samples.md`: von TinySOL nur die Streicher).
- Die Spuren Expression, Pitch und Vibrato des Musters erreichen einen Kern heute nicht: die App gibt ihm eine Note, die
  Stufe und feste Spurwerte am Beginn (`Instrument.cpp:2964–2969`) und nimmt die Expression danach als Pegel
  (`Instrument.cpp:3067 ff.`). Die Zunge ist der erste Kern mit dauernder Anregung (SK4 a: Expression ist dort die Stärke
  der Anregung).

Entscheide dazu: `../../audio/Instrumente_IC_Konzept.md` §6, Teilplan 34j (ZV1 ff.).
