# Hörschwellen — je Messgröße die Schwelle, ab der das Ohr einen Unterschied hört (T9 a, HW1 a)

> **Stand:** 2026-10-31, IC Session 5 (lokal; Aufgabe IC-A2 aus der Prüfung durch LumiViz S166). **Zahlen:** die Tabelle
> `rechnung/werte/hoerschwellen.txt` (Kopie aus LumiViz, HW1 a, Patrik S164) — maßgeblich ist die Datei, nicht diese
> Seite. **Gebrauch im Code:** `messung/Hoerschwellen.hpp` (portiert aus LumiViz), im Setz-Werkzeug über
> `Einstellung::hoerschwellen`; der Schlüssel ist der Name der Messgröße (längstes passendes Präfix). „Hörbar getroffen“
> und „eindeutig“ gelten bei |d| ≤ max(τ, S) (HW2 a), die Probe nach HW3 a, systematisch nach HW4 a
> (`messung/Probe.hpp`, `setzen/Setzen.hpp`). **Belegkraft:** die Fundstellen sind in IC selbst nachgelesen (lokale PDFs
> unter LumiViz `docs/modellierung/hammer-saite/quellen/`, mit `quelle_suchen.py` und `pdftotext`, 2026-10-31); was
> sekundär bleibt, steht dabei.

## Die Zahlen (HW1 a)

| Messgröße (Muster) | Skala | Schwelle | Bedingungen der Messung | Quelle, Seite | Belegkraft |
|---|---|---|---|---|---|
| Abklingzeit `vorwaerts_t60`, `t60_spaet`, `huelle_` — kürzer | Faktor (ln) | 0,7923 (ln −0,232815) | gezupfte Saite, Synthese (Schleifenfilter), Satz 1 von 8: Ton F, Impulsanregung, 2,0 s (der strengste der 2-s-Sätze); Mittel über fünf Hörer, Kopfhörer, 78 dB SPL | Tolonen & Järveläinen 2000, AES 109th Conv., Preprint 5205, **Tab. 6, S. 11** (Satz 1: oben 1,4309, unten 0,7923); Sätze und Dauern **Tab. 2/3, S. 6** (1–4: 2,0 s, 5–8: 0,6 s); Hörer S. 8 | **nachgelesen** |
| dieselben — länger | Faktor (ln) | 1,4309 (ln 0,358304) | wie oben; mit kürzerer Hördauer viel weiter (Sätze 7/8, 0,6 s: oben 5,64 / 5,41) | wie oben | **nachgelesen** |
| Knicktiefe `knick_tiefe_db` | dB | ±0,5 dB | Pegelunterscheidung am Sinus, oberes Ende (80 dB SL) | Jesteadt, Wier & Green 1977 (JASA 61, 169–177), über Penconek 2025 | **sekundär** — die Zeitschrift ist nicht frei; Übertragung, kein Beleg für einen Knick |
| Schwebungs- und Einbruchtiefe `schwebung_db`, `tiefe_db` | Ewert (Spitze-Spitze in dB, m = tanh(pp·ln10/40)) | Unterscheidung 1,2 dB in 20 lg m ab 20 lg m = −18 dB; darunter linear bis 4,1 dB bei −28 dB; darunter 4,1 dB | 16 Hz Sinus-AM auf 4-kHz-Sinus, 500 ms, 65 dB SPL, 3AFC 2-down-1-up (70,7 %), fünf Hörer | Ewert 2002, Diss. Oldenburg, **S. 40** (Reiz), **S. 41** (Tiefen −28 … −3 dB), **S. 42** (Weber-Bruch konstant bis −18 dB; „about … 1.2 dB for the pure-tone carrier“; bei −28 dB Weber-Bruch 2 dB ⇒ 10·lg(1 + 10^0,2) = 4,12 dB) | **nachgelesen** (4,1 dB nachgerechnet aus dem Weber-Bruch) |
| dieselben — beide unter 0,87 dB gleich | Ewert | m ≈ 0,05 ⇒ pp = 0,87 dB | Erkennen einer AM (nicht Unterscheiden), Literaturüberblick | Järveläinen 2003, Diss. HUT Report 68, **S. 29** (Abschn. 4.4.1: „typically around m ≈ 0.05“) | **nachgelesen** (Überblick, die Originale Schorer 1986, Moore & Sek nicht gelesen) |
| Schwebungsfrequenz `schwebung_hz` | Faktor (ln) | ±6 % (ln 0,058269) | Vibrato-Rate an Saitensynthese, fünf Hörer — nur für die Prüfung g nach A4, keine Größe des Ziels | Järveläinen 2003, **S. 38** („About 6 % of variation was allowed for vibrato rate“; Original Järveläinen 2002, ICMC, gesperrt) | **nachgelesen** in der Zusammenfassung |

Mit der Hördauer werden die Schwellen enger (Tab. 6: Faktor 1,43 bei 2 s, 5,6 bei 0,6 s); ein länger gehörter Ton
liegt eher unter dem strengsten 2-s-Satz als darüber — darum gilt Satz 1 (Grund von HW1 a).

## Übertragung je Klasse (T9 a, IC-A2 Punkt 4)

Keine der Zahlen ist an einem Instrument dieses Projekts gemessen. Gemessen sind: Abklingen an **gezupften** Saiten
(Tolonen), Modulationstiefe an einem **4-kHz-Sinus mit 16 Hz** (Ewert), Vibrato-Rate an Saiten (Järveläinen). Je
Klasse, was davon übertragen wird und wie weit (Einschätzung; die Mechanismen aus dem „Heikel“ jedes `messplan.md`):

| Klasse | Abklingzeit (Faktor 0,79 / 1,43) | Schwebungstiefe (Ewert) | Schwebungs- oder Vibratorate (6 %) | ohne Zahl |
|---|---|---|---|---|
| 1 Hammer-Saite | Übertragung von gezupft auf angeschlagen (wie LumiViz) | Chorschwebung: Übertragung von 16 Hz auf 0,1–5 Hz | Chorschwebung, Übertragung | Knick (nur Pegel, sekundär); Mitschwingen |
| 2 Gezupfte Saite | **der gemessene Reiz** (gezupft, 2 s) — am nächsten | Polarisationen, Doppelchöre: Übertragung | Übertragung | zweistufiges Abklingen (Järveläinen 2003 S. 38: nur qualitativ) |
| 3 Bogen-Saite | nur Nachklang nach dem Absetzen | Wolfton: Übertragung | **Vibrato der Spieler — der gemessene Reiz** (Saite) | Einschwingen (Guettler 50/90 ms ist „annehmbar“, keine Schwelle) |
| 4 Platte und Stab | Übertragung (angeschlagen, oft länger als 2 s) | Vibraphon-Tremolo, Resonator-Schwebung: Übertragung | Tremolo-Rate: Übertragung | Härte des Schlägels |
| 5 Membran | Übertragung; kurze Töne — nach Tab. 6 viel weitere Schwelle | Fell gegen Kessel: Übertragung | — | Tonhöhe der Pauke (Spannung, Luftlast) |
| 6 Becken und Gong | Übertragung | Modenpaare: Übertragung | — | Aufblühen, Rauschhaftigkeit |
| 7 Lippe und Rohr | nur Nachklang | Wah, Schmettern: keine Zahl | Vibrato: Übertragung von Saite auf Lippe | Register, Ansprache |
| 8 Rohrblatt und Rohr | nur Nachklang | — | Vibrato: Übertragung | Register (Überblasen), Ansprache |
| 9 Luftstrahl und Rohr | nur Nachklang | — | Vibrato: Übertragung | Register, Anblasgeräusch |
| 10 Durchschlagende Zunge | nur Nachklang | **Musette-Tremolo** (1–7 Hz): Übertragung von 16 Hz | Musette-Rate, Balgvibrato: Übertragung | Ansprache im Bass |
| 11 Tonrad | Perkussion: Übertragung | Leslie (AM): Übertragung | Leslie-Rate, Scanner-Vibrato: Übertragung | Klick |

**Folgen.** (1) Wo eine Klasse Messgrößen mit anderen Namen führt, erhält `hoerschwellen.txt` eine eigene Zeile mit
derselben Zahl und der Übertragung in der Spalte Quelle — keine neue Zahl ohne Quelle. (2) Die Übertragung auf langsame
Schwebungen ist die schwächste: Ewert misst 16 Hz an 500 ms; bei Raten unter 0,5 Hz hört man nur wenige Perioden, und
die Schwelle wird größer (LumiViz-Recherche S164, Lee 1994, nicht gelesen). (3) Nach L17 ist eine kleine eigene
ABX-Messung am eigenen Reiz der einzige Beleg für das eigene Instrument — zuerst dort, wo die Tabelle „Übertragung“ für
den heiklen Mechanismus sagt (IC-A15 Punkt 4).

## Nicht gelesen

- Järveläinen & Tolonen, JAES 49(11), 2001 (Zeitschriftenfassung von Tolonen & Järveläinen 2000) — nicht nötig, solange
  die Zahlen aus der Konferenzfassung stammen; die Tab. 6 ist dort nachgelesen.
- Jesteadt, Wier & Green 1977 (nicht frei); Ewert & Dau 2004, JASA 116 (Zeitschriftenfassung von Ewert 2002 Kap. 2).
