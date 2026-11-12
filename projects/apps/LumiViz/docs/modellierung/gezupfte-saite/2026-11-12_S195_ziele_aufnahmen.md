# Ziele aus Aufnahmen: Lautheit über den Umfang, Ausklang, Dynamik (Stufe 34k-4)

> **Sitzung:** S195 (2026-11-12). **Zweck:** die Zielgrößen aus GZ2 a an echten Instrumenten messen, mit derselben
> Lautheitsmessung wie der Wächter `Loop Pegel LP3 a` (Momentary-Maximum aus `AudioCore/Lautheit`), bevor 34k-6 das
> Modell daran misst. **Programm:** [proben/2026-11-12_S195_ziele_aufnahmen.cpp](proben/2026-11-12_S195_ziele_aufnahmen.cpp),
> gebaut mit [proben/2026-11-12_S195_bau.bat](proben/2026-11-12_S195_bau.bat) (clang, gegen `AudioCore` aus dem Repo);
> **Rohausgabe:** [proben/2026-11-12_S195_ziele_aufnahmen.txt](proben/2026-11-12_S195_ziele_aufnahmen.txt) (952 Zeilen,
> je Ton eine). Nichts am Programm geändert.
> **Aufnahmen** (Archiv `…\samples\Klasse 2 - Gezupfte Saite\`, Begleitdateien unter [aufnahmen/](aufnahmen)): VCSL
> (CC0) — Konzertharfe 45 Töne, Volksharfe 58, fünf Cembali mit 233 Tönen *Sustains*; Iowa MIS Nylongitarre
> (Raimundo 118, schalltot, 45 Läufe pp/mf/ff je Saite, 96 kHz).

## Was gemessen wurde

Je Ton: das Momentary-Maximum (0,4 s) in LUFS — die Größe von `LP3 a` —, die integrierte Lautheit der ersten Sekunde,
die Spitze, der Abfall in dB/s früh (Gerade durch die 20-ms-Blöcke zwischen −5 und −25 dB unter dem lautesten) und spät
(−25 bis −45 dB), die Zeit bis −20 und −60 dB, der Boden der Aufnahme. Die absoluten Pegel sind zwischen Sammlungen und
gegen das Modell nicht vergleichbar; verglichen werden Spannen, Sprünge und Unterschiede.

Geprüft, bevor den Zahlen geglaubt wird:

- **Oktavlage der Namen.** Die Konzertharfe ist mit C4 = 60 benannt, alle anderen VCSL-Gruppen eine Oktave tiefer
  (Autokorrelation am Ton: 45 von 45 bzw. alle Töne je Gruppe in derselben Lage). Die Zusammenfassung rechnet mit der
  korrigierten Note.
- **Normierung.** Die Spanne der Spitzen je Gruppe folgt der Spanne der Lautheit (etwa Cembalo französisch 8,6 dB
  Spitze, 9,1 LU Lautheit) — die Dateien sind nicht je Ton normiert.
- **Iowa-Läufe.** An der Hüllkurve nachgesehen: die Töne eines Laufs sind durch digitale Stille getrennt. In 38 von 45
  Läufen passt die Zahl der Stücke zur Zahl der Noten im Namen; dort gilt die Reihenfolge (die Gegenprobe aus dem Klang
  wich in 5 von rund 330 Stücken ab, jedes Mal ein Messfehler der Gegenprobe). Die 7 übrigen Läufe (fünf davon pp)
  bleiben aus der Zusammenfassung.

## Lautheit über den Umfang

| Instrument (Register) | Töne, Umfang | Spanne | Spanne 10.–90. Perzentil | größter Sprung (Abstand ≤ 4 Halbtöne) | Sprung Median |
|---|---|---|---|---|---|
| Cembalo „Unk“ | 61, F1–F6 | **6,1 LU** | 3,7 | 4,5 | 1,3 |
| Cembalo flämisch, *Low* | 28, F♯1–C6 | **7,8** | 4,9 | 6,2 | 1,6 |
| Cembalo französisch | 30, D1–C6 | **9,1** | 6,6 | 5,4 | 1,5 |
| Cembalo italienisch | 32, F♯1–H5 | **12,5** | 7,5 | 8,2 | 1,8 |
| Cembalo englisch (Zuckermann), *Normal* | 28, A♯1–E6 | **13,6** | 11,0 | 11,3 | 2,4 |
| Cembalo flämisch, *High* | 26, F♯2–C7 | 18,3 | 12,1 | 8,6 | 1,8 |
| Cembalo englisch, *Lute* (Lautenzug) | 28, A♯1–E6 | 19,5 | 12,5 | 10,9 | 2,4 |
| Konzertharfe, f | 23, E1–F7 | **17,8** | 11,6 | 7,3 | 3,9 |
| Konzertharfe, mf | 19, H1–H6 | 21,2 | 18,1 | 8,4 | 3,1 |
| Volksharfe, laut | 29, C2–G♯6 | 18,6 | 11,3 | 12,1 | 3,7 |
| Gitarre, ff, je Saite (sechs Saiten) | je 19–20 | 8,0–28,2 | 5,5–18,9 | 3,6–16,7 | 1,0–3,9 |

- **Die mechanisch gezupften Cembali halten 15 LU** in den fünf Hauptregistern (6,1–13,6 LU). Der Diskant fällt ab
  (französisch C6 −29,2 gegen −20,1 LUFS an E3; englisch C6–E6 Median −29,6 gegen −22).
- **Die Konzertharfe hält 15 LU nicht** (17,8 LU bei f): die tiefste Oktave liegt 12–17 LU unter der Mitte (E1 −41,0,
  G1 −40,3, H1 −35,1 gegen −23 bis −28 LUFS ab D2). Ohne die Töne unter D2 beträgt die Spanne 10,8 LU (D2–F7,
  −34,1 an F2 … −23,3 an E5). Von Hand gezupft — die Streuung des Spielers steckt darin.
- **Sprünge zwischen Nachbarn:** der größte Sprung liegt bei den Cembali zwischen 4,5 und 11,3 LU, der mittlere bei
  1,3–2,4 LU. Die Aufnahmen sind raumbehaftet; Raummoden vergrößern die Sprünge, das Instrument allein springt weniger.
  Die in S193 ohne Quelle gesetzte Schwelle „kein Sprung über 6 LU“ wird von den Aufnahmen weder bestätigt noch
  widerlegt; als zweite, robuste Größe bietet sich der **mittlere Sprung** an (gemessen 1,3–2,4 LU am Cembalo, 3–4 LU
  an der Harfe). Das gehört in die Herleitung 34k-6.
- **Gitarre:** von Hand gespielt und je Saite sehr ungleich (hohe e-Saite 8,0 LU, A-Saite 28,2 LU mit einzelnen
  eingebrochenen Tönen). Als Ziel für die Spanne taugt nur das Mittel der Lagen (unten).

## Ausklang je Lage

Median je Oktave; T60 nur aus Tönen, deren Boden unter −60 dB liegt (Zahl in Klammern).

| Lage | Cembalo französisch: früh dB/s · T60 s | Cembalo „Unk“ | Cembalo englisch *Normal* | Lautenzug (englisch) | Konzertharfe f: früh · spät dB/s · T60 s | Gitarre ff: früh · spät dB/s · T60 s |
|---|---|---|---|---|---|---|
| C1–H1 | 3,4 · 14,9 (3) | 2,9 · 14,0 (7) | 3,0 · 13,1 (1) | 5,7 · 6,1 (1) | 1,9 · 1,0 · — | |
| C2–H2 | 4,2 · 12,6 (6) | 4,3 · 10,4 (12) | 4,5 · 10,8 (6) | 9,5 · 4,3 (6) | 5,9 · 2,9 · 13,8 (1) | 5,5 · 0,7 · 11,9 (8) |
| C3–H3 | 6,8 · 10,2 (8) | 6,8 · 7,8 (12) | 7,3 · 7,0 (6) | 17,2 · 2,9 (5) | 6,8 · 3,7 · 13,6 (4) | 14,8 · 1,4 · 8,7 (35) |
| C4–H4 | 13,3 · 7,6 (6) | 9,4 · 6,5 (12) | 12,9 · 5,2 (6) | 26,1 · 2,5 (6) | 18,3 · 6,1 · 9,6 (3) | 11,1 · 1,1 · 7,4 (34) |
| C5–H5 | 18,1 · 5,4 (6) | 16,1 · 5,1 (12) | 21,8 · 3,7 (6) | 64,3 · 1,9 (6) | 38,9 · 6,0 · 5,7 (4) | 11,6 · 0,9 · 5,8 (12) |
| C6–H6 | 27,8 · 4,6 (1) | 27,9 · 3,7 (6) | 32,4 · 2,7 (3) | 109,5 · 1,4 (4) | 52,6 · 44,3 · 2,2 (4) | |
| C7–H7 | | | | | 150,5 · 66,1 · 1,5 (2) | |

- **Cembalo:** die drei Instrumente liegen eng beieinander — früher Abfall 3 dB/s im Bass bis 28–32 dB/s in der
  sechsten Oktave, etwa eine Verdopplung je Oktave ab C3. Die Zeiten bis −60 dB (15 → 3–5 s) enthalten den Raum und
  sind eine obere Grenze. Der **Lautenzug** fällt zwei- bis dreimal so schnell.
- **Harfe:** zweistufig — in der Mitte früh 18–39 dB/s, spät 6 dB/s (die ungedämpften übrigen Saiten und der Raum
  tragen den Ausklang); T60 14 → 1,5 s über den Umfang.
- **Gitarre (schalltot):** früh 5–15 dB/s, spät rund 1 dB/s — der lange Rest ist bei offenen Nachbarsaiten zu
  erwarten (Mitschwingen, GZ3 a), wird hier aber nicht je Teilton getrennt. Einzelne Töne brechen in den ersten 0,3 s
  um 20 dB ein (A2 ff: 131,8 dB/s früh); ob das Spielweise ist oder am Lauf liegt, ist nicht geklärt — für die
  Herleitung die Teiltonmessung je Ton nehmen, nicht diese Hüllkurve.

## Dynamik

| Instrument | Unterschied | Median über die Noten | 10.–90. Perzentil |
|---|---|---|---|
| Konzertharfe | f − mf | 6,7 LU (19 Noten) | 3,3 … 13,9 |
| Volksharfe | laut − leise (zwei Lagen der Sammlung) | 13,9 LU | 6,6 … 20,7 |
| Gitarre je Saite | ff − mf | 3,3 … 8,9 LU | |
| Gitarre je Saite | mf − pp | 4,2 … 15,1 LU | |
| Gitarre je Saite | ff − pp | 9,4 … 21,6 LU | |
| Cembalo englisch | *Normal* gegen Lautenzug | in der Rohausgabe je Ton | |

Das Cembalo hat in VCSL nur eine Stärke; für die Dynamik bleibt Penttinen 2006 (F-4 a). Die Gitarre: ff gegen pp im
Median rund 13 LU, mit großer Streuung von Ton zu Ton (Handspiel).

## Was das für 34k-6 heißt

1. **Spanne ≤ 15 LU** ist für das Cembalo ein Ziel, das echte Instrumente halten (6–14 LU). Für die Harfe über den
   vollen Umfang nicht belegt: die tiefste Oktave liegt gemessen 12–17 LU unter der Mitte. Ob die Harfe die Grenze „ohne
   Ausnahme“ halten soll oder im Bass dem Instrument folgt, ist in 34k-6 als Entscheid vorzulegen — mit dem Modell an
   derselben Messung.
2. **Sprünge:** größter Sprung und mittlerer Sprung messen; die Schwelle 6 LU bleibt bis dahin gesetzt.
3. **Ausklang:** der frühe Abfall je Lage (Tabelle) ist die belastbare Zielgröße; T60 nur als obere Grenze (Raum).
4. **Dynamik:** Harfe f − mf 6,7 LU, Gitarre ff − pp rund 13 LU als Größenordnung für die Wirkung der Stärke.
5. **Nicht gemessen:** Helligkeit je Stärke und Abklingen je Teilton (kommt mit der Teiltonmessung in 34k-6 an Modell
   und Aufnahme zugleich); Laute und Oud — keine Aufnahme im Archiv.
