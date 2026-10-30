# Recherche S164 — Hörschwellen je Messgröße (7.5.13 B1)

> **Auftrag** (S164, 2026-10-30, Opus 5.5 an einen Recherche-Agenten): je Messgröße der Suche (`vorwaerts_t60_k`,
> `t60_spaet_k`, `knick_tiefe_db_k`, `huelle_20/30/40_s`, `schwebung_db_k` / `tiefe_db_k`, Schwebungsfrequenz, Pegel)
> eine Hörschwelle aus der Literatur mit Fundstelle, Bedingungen und Umrechnung in die Skala der Suche (Faktor = |ln|,
> dB); Klavier- und Saitenklänge vor Sinus und Rauschen; nur WebFetch/curl, PDFs nach `quellen/`. Grundlage: V2 a,
> Teilplan 7.5.13 (Konzept `docs/audio/Spielweisen_Konzept.md` §4.1).
>
> **Kennzeichnung:** Der Text unter dem Strich ist der Bericht des Agenten, **unverändert**. Belegarten stehen je Zeile
> (Volltext / Abstract / sekundär / unbestätigt). Die Umrechnungen in ln und Spitze-Spitze-dB sind gerechnet, keine
> Literaturwerte.
>
> **Selbst nachgelesen (Claude, S164):**
> - Tolonen & Järveläinen 2000, Tab. 6 (S. 11) am Seitenbild: Sätze 1–4 oben 1,4309 / 1,4725 / 1,4904 / 1,4951, unten
>   0,7923 / 0,7317 / 0,7781 / 0,6998 (τ/τref, Mittel über die Hörer); S. 15 Empfehlung „about 75% and 140%“ — stimmt.
> - Ewert 2002, S. 40–42 (Kap. 2): Sinusträger 4 kHz, fm 16 Hz, 500 ms, 65 dB SPL, 3AFC 2-down-1-up (70,7 %), fünf
>   Hörer; „about 1.4 dB for the noise carrier and 1.2 dB for the pure-tone carrier“ bei hohen Standardtiefen; beim
>   Sinusträger Weber-Bruch etwa konstant bis −18 dB Standardtiefe, bei −28 dB 2 dB (10 lg) — stimmt.
> - Järveläinen 2003, S. 38 und S. 40: zu P6 (zwei Polarisationen) nur qualitativ, „much greater variations in decay and
>   dual-polarization parameters are tolerated“; Vibrato-Rate etwa 6 % — stimmt.
> - Thomas, Rousseau, Caussé, Marandas 1998, S. 4: Marandas misst 0,5 Cent als mittlere Verstimmung, ein Bereich ohne
>   hörbare Schwebung wohl zwischen 1 und 2 Cent — stimmt (Erfahrung eines Stimmers, kein Hörversuch).
> - Nicht nachgelesen: alle Angaben mit Belegart „Abstract“, „sekundär“, „unbestätigt“.
>
> **Eingeflossen in:** die Vorlagen B2 und B3 (S164, Konzept §4.1 „7.5.13“); Rechnung der Optionen an den Ergebnissen
> aus S162/S163 (Skripte `b2_optionen.py`, `b3_vorzeichen.py`, Scratchpad S164).

---

# B1 — Hörschwellen (JND) je Messgröße des Flügelmodells: Belege aus der Literatur

Stand: 30.10.2026 · Recherche-Agent für LumiViz (Hammer–Saite, Probe 7.5) · nur WebFetch/curl, kein Browser.

**Skalen wie im Auftrag:** „Faktor“ = |ln(a/b)| (5 % → 0,049); „dB“ = Differenz in dB.
**Belegart:** *Volltext* = selbst im Wortlaut gelesen (PDF lokal); *Abstract* = nur Kurzfassung (PubMed bzw. Verlagsseite) gelesen; *sekundär* = Zahl nur in einer anderen Quelle gelesen, die sie zitiert; *unbestätigt* = nur in einem Suchmaschinen-Ausschnitt gesehen, nicht in einer abgerufenen Quelle — **nicht als Beleg verwenden**.
**Schwellen-Definitionen sind nicht einheitlich:** 50 %-„verschieden“-Punkt (Tolonen), 70,7 % (2-down-1-up, Ewert), 71 % (Jesteadt), 75 %, 79 % (3-down-1-up 3AFC, Woodhouse). Ein 79 %-Wert liegt höher als ein 50 %-Wert derselben Hörer.

Ablage der geladenen PDFs: `C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\hammer-saite\quellen\` (Kopf jeweils `%PDF` geprüft).

---

## 1. Tabelle je Größe

### Größe 1 — `vorwaerts_t60_k` (Faktor), T60 der ersten Stufe je Teilton

| Schwelle (Original) | Faktor ln | Reiz / Bedingungen | Fundstelle | Belegart | Zugang |
|---|---|---|---|---|---|
| Zeitkonstante τ des Gesamtabklingens: obere Schwelle τ/τref = 1,43–1,50, untere 0,70–0,79 (Mittel über 5 Hörer, Töne 2,0 s) | oben 0,36–0,40; unten 0,23–0,36 | Kommutierte Wellenleiter-Synthese einer klassischen Gitarre, G3 196 Hz und F4 349 Hz, Impuls- oder Rest-Anregung; τref 0,77 s bzw. 0,60 s; Kopfhörer HD 580, Mittel 78 dB SPL; 5 erfahrene Hörer (2 davon die Autoren); Konstanzmethode, gleich/verschieden, je Paar 25 Urteile; Schwelle = 50 %-Punkt „verschieden“ | Tolonen & Järveläinen 2000, Tab. 6 (S. 11), Abb. 7; Methode S. 8–9 | Volltext | frei (aaltodoc), lokal |
| Kurze Töne (0,6 s): oben 2,4–5,6, unten 0,56–0,65 | oben 0,89–1,73; unten 0,43–0,59 | wie oben, Ton nach 0,6 s mit 100-ms-Rampe beendet | ebd., Tab. 6, Sätze 5–8 (S. 11) | Volltext | frei, lokal |
| Abhängigkeit von f0 (117, 196, 351, 663 Hz; 2 s): oben 1,35–1,49, unten 0,67–0,79, kein monotoner Gang | oben 0,30–0,40; unten 0,23–0,39 | Zusatzversuch, Rest-Anregung | ebd., Tab. 8 (S. 16), Abb. 9, Text S. 13–14 | Volltext | frei, lokal |
| Empfehlung der Autoren: 75 % … 140 % von τref „in most cases“ unhörbar | 0,29 (kürzer) / 0,34 (länger) | Zusammenfassung obiger Versuche | ebd. S. 15; Järveläinen 2003, S. 36 | Volltext | frei, lokal |
| Frequenzabhängiges Abklingen (Pol a des Schleifenfilters): 83 % … 116 % von aref | 0,19 / 0,15 (auf a, nicht auf T60_k) | wie oben; Dauer ohne Einfluss, Impulsanregung senkt Schwelle | Tolonen & Järveläinen 2000, Tab. 7 (S. 13), S. 15; Järveläinen 2003, S. 36 | Volltext | frei, lokal |
| JAES-Fassung: −25 % … +40 % für die Gesamt-Abklingzeit | 0,29 / 0,34 | (Zeitschriftenfassung derselben Versuche) | Järveläinen & Tolonen 2001, JAES 49(11) 1049–1059 — zitiert bei Bank 2006, S. 44 | sekundär | AES gesperrt (403) |
| Saitendämpfung ηF (Mindest-Verlustfaktor, Mitte des Spektrums) skaliert: Faktor 2,41 (Mittel der besten 4); bester Hörer < 2 | 0,88 (bester < 0,69) | Gitarren-Synthese (Woodhouse-Modell, Saite + Korpus gekoppelt), C3 auf A-Saite (131 Hz), Ton 1 s; 3AFC, 3-down-1-up (79 %); 11 Musiker + 5 Nicht-Musiker | Woodhouse et al. 2012, Abb. 9c (S. 484), Text S. 484–485 | Volltext | frei (LAM-Kopie), lokal |
| Saitendämpfung ηB (Biegeverlust, hohe Teiltöne): Faktor 2,99 bzw. 3,13 | 1,10 / 1,14 | wie oben, E2 und C3 | ebd., Abb. 9a,b (S. 484) | Volltext | frei, lokal |

### Größe 2 — `t60_spaet_k` (Faktor), T60 der späten Stufe (Nachklang)

| Schwelle (Original) | Faktor ln | Reiz / Bedingungen | Fundstelle | Belegart | Zugang |
|---|---|---|---|---|---|
| Toleranzen für die Zeitkonstante des Nachklangs gemessen; Ergebnis nur qualitativ: Empfindlichkeit für Zwei-Polarisations-Effekte „generally weak“, deutliche Abweichungen erlaubt — **keine Zahl im Volltext der Zusammenfassung** | — | Saitenmodell mit zwei Polarisationen (akustische Gitarre) | Järveläinen & Karjalainen 2002 (ISMA, Mexico City), zusammengefasst in Järveläinen 2003, S. 38 | Volltext der Diss.-Zusammenfassung; Originalartikel **nicht erhalten** | Original: alte URL 404, Web-Archiv 429, Aalto-Portal ohne Volltext |
| Ersatzweise: Gesamtabklingen 0,75 … 1,40 (Größe 1) | 0,29 / 0,34 | einstufiges Abklingen, Gitarre | wie Größe 1 | Volltext | frei |
| Doppelte Steigung im Raum (gekoppelte Volumina): JND ≈ 10 % Änderung der **Koppelöffnung** (nicht des späten T60) | (nicht umrechenbar) | Physisches Modell zweier gekoppelter Räume, ABX, Musikausschnitte | Luizard, Katz, Guastavino 2015, JASA 137(1) 75–84 | Abstract (PubMed 25618040) | HAL hinter Anubis gesperrt |

### Größe 3 — `knick_tiefe_db_k` (dB), Pegel des Übergangs schnelle → langsame Stufe

| Schwelle (Original) | dB | Reiz / Bedingungen | Fundstelle | Belegart | Zugang |
|---|---|---|---|---|---|
| **Keine direkte Schwelle gefunden.** Die Diss.-Zusammenfassung nennt Versuche zur Ähnlichkeit ein- vs. zweistufig und zum Unterschied der Zeitkonstanten der Polarisationen, ohne Zahl | — | Gitarren-Saitenmodell | Järveläinen 2003, S. 38 (über J&K 2002) | Volltext Zusammenfassung | Original nicht erhalten |
| Ersatzgröße Pegel (Nachklang-Plateau gegen Spitze ≈ Pegelvergleich): ΔL ≈ 0,5 … 1,4 dB je nach Pegel | 0,5–1,4 | Sinus, Pulse, siehe Größe 7 | Jesteadt et al. 1977 über Penconek 2025 | sekundär | siehe Größe 7 |

### Größe 4 — `huelle_20_s`, `huelle_30_s`, `huelle_40_s` (Faktor), Breitband-Hüllkurve

| Schwelle (Original) | Faktor ln | Reiz / Bedingungen | Fundstelle | Belegart | Zugang |
|---|---|---|---|---|---|
| ISO 3382-1: JND für EDT (Nachhall-Empfindung) 5 % relativ; Stärke G 1 dB; C80 1 dB | 0,049 | Raumakustik, Konzertsäle; Norm beruht laut Blevins et al. auf Seraphim 1958 | ISO 3382-1:2009 Anh. A, Tab. A.1 — gelesen nur als Folie bei Skålevik 2010 (BNAM, Folie 3) und als Satz bei Klein et al. (S. 1) und Blevins et al. (Abstract) | sekundär | ISO nicht frei; Folie und Klein lokal |
| Seraphim 1958: Unterschiedsschwelle des exponentiellen Abklingens von Rauschbandimpulsen ≈ 5 % | 0,049 | Rauschbandimpulse | Acustica 8 (1958) 280–284 | sekundär (Blevins et al. Abstract; Klein et al. S. 1) | nicht frei |
| Gemessene JND des Nachhalls in der Literatur 6 % … 39 %; eigener Vorversuch (weißes Rauschen, 30 Hörer) 22 % | 0,058 … 0,33; 0,20 | Auralisation (Elmia-Saal, ODEON), Folge-Studie mit Oktavband-Rauschen 1 kHz; 3-Intervall, 1-up-2-down | Blevins, Buck, Peng, Wang 2013 (ISRA Toronto), Abstract | Abstract | PDF auf digitalcommons.unl.edu 403 (gesperrt) |
| Nachhallunterschied im Raum: 8 % für Kastagnetten erkennbar, über alle Bedingungen 15 % | 0,077 / 0,14 | Variabler Raum, binaurale 3-DoF-Wiedergabe, Dreiecks-Test, Sprache/Musik/Kastagnetten, T30 ≈ 0,5 s | Klein, Amengual Garí, Arend, Robinson (≈2021), S. 1 (Abstract) und Ergebnistabelle | Volltext (Pilotstudie, wenige Hörer; Wiedergabe der Werte per WebFetch-freiem pdftotext geprüft) | frei, lokal |
| Kleine Regieräume: Unterschiedsschwelle 0,042 ± 0,015 s bei T 0,2–0,6 s | (≈ 7–21 %) | Musik, echter Raum, 6 Hörer | Niaounakis & Davies 2002, JAES 50 | **unbestätigt** (nur Suchmaschinen-Zusammenfassung des AES-Eintrags) | AES gesperrt |
| Für Konzertsaal-Nachhall (T ≈ 1,8 s) ≈ 6 % bei exponentiellem Abklingen | 0,058 | — | Frissen, Katz, Guastavino 2009/2010 | **unbestätigt** (Suchtreffer-Ausschnitt) | Repositorien gesperrt (403/Anubis) |
| Gesamtabklingen eines gezupften Saitentons 0,75 … 1,40 | 0,29 / 0,34 | siehe Größe 1 | Tolonen & Järveläinen 2000, S. 15 | Volltext | frei, lokal |

### Größe 5 — `schwebung_db_k` und `tiefe_db_k` (dB), Modulationstiefe der Schwebung

Umrechnung: Modulationsgrad m ↔ Spitze-Spitze-Hub pp = 20·lg((1+m)/(1−m)); für kleine m gilt pp ≈ 17,4·m dB.

| Schwelle (Original) | in pp-dB | Reiz / Bedingungen | Fundstelle | Belegart | Zugang |
|---|---|---|---|---|---|
| **Erkennen** einer AM: typisch m ≈ 0,05; Schwelle sinkt erst oberhalb 64 Hz Modulationsfrequenz | 0,87 dB | Sinusträger, Literaturüberblick (Schorer 1986; Moore & Sek 1992, 1995) | Järveläinen 2003, Abschn. 4.4.1, S. 29 | sekundär (Diss.-Überblick, Volltext gelesen) | frei, lokal |
| AM-Erkennung 1-kHz-Ton, 500 ms, 70 dB SPL, fm 16 Hz: ≈ −28 dB (20 lg m); bei 30 dB SPL ≈ −20 dB | 0,69 dB; bei leisem Ton 1,74 dB | Sinusträger | Wojtczak & Viemeister 2008 (zitiert Yost & Sheft 1997 bzw. Wojtczak & Viemeister 1999) | sekundär (über WebFetch-Auszug der PMC-Volltextseite) | PMC frei (online, nicht lokal) |
| TMTF für Sinusträger flach bis ≈ 100–130 Hz; Schwebungs-Erkennung bei langsamer Schwebung ≈ 6 dB über dem Seitenband-Pegel der AM-Schwelle | — | 5 und 10 kHz Träger | Kohlrausch, Fassel, Dau 2000, JASA 108, 723–734 | Abstract (PubMed 10955639) | JASA nicht frei |
| **Unterscheiden** zweier Tiefen: ≈ 1,2 dB in 20 lg m (Sinusträger 4 kHz, fm 16 Hz, 65 dB SPL, 500 ms, 5 Hörer, 70,7 %), konstant für Standardtiefen −18 … −3 dB; Rauschträger 1,4 dB | relativ ≈ 15 % von pp (ln 0,14) für pp ≲ 6 dB; bei pp 9,5 dB Δ ≈ 1,8 dB; bei pp 15 dB Δ ≈ 4,2 dB | siehe links | Ewert 2002 (Diss. Oldenburg, Kap. 2 = Ewert & Dau 2004, JASA 116, 478–490), S. 40–42, Abb. 2.1 | Volltext | frei (OOPS), lokal |
| Ältere Arbeiten (Fleischer 1980; Ozimek & Sek 1988 mit fm **1 … 64 Hz**, Träger 250–2000 Hz; Lee & Bacon 1997): Tiefenunterscheidung „etwas über 1 dB“ für Standards über ≈ −15 dB, unabhängig vom Träger | wie oben | — | Ewert 2002, S. 37 | sekundär (Überblick im Volltext) | frei, lokal |
| Unterhalb ≈ −18 dB (Ton) bzw. −8 dB (Rauschen) steigt der Weber-Bruch; bei −28 dB Standard 2 dB (Ton) | — | — | Ewert 2002, S. 41–42 | Volltext | frei, lokal |
| Schwebung in Zwei-Polarisations-Saiten: „Detecting a change in beating“ als Funktion der Pegeldifferenz der Polarisationen gemessen, Ergebnis nur qualitativ (schwache Empfindlichkeit) | — | Gitarren-Saitenmodell | Järveläinen 2003, S. 38 (über J&K 2002) | Volltext Zusammenfassung | Original nicht erhalten |

### Größe 6 — Schwebungsfrequenz / Verstimmung im Chor

| Schwelle (Original) | Faktor ln | Reiz / Bedingungen | Fundstelle | Belegart | Zugang |
|---|---|---|---|---|---|
| AM-Raten-Unterscheidung: ≈ 2,5–3,0 Hz absolut unterhalb 60 Hz Rate | bei 14 Hz ≈ 0,19; bei 30 Hz ≈ 0,1 (abgeleitet) | AM-Breitbandrauschen | Formby 1985, JASA 78, 70–77 | Abstract (PubMed 4019909) | nicht frei |
| ≈ 3 Hz für Raten bis 66 Hz (getestet 14–224 Hz) | wie oben | bandpassgefiltertes Rauschen 500–4000 Hz | Hanna 1992, JASA 91, 2122–2128 | Abstract (PubMed 1597603) | nicht frei |
| Ratenschwelle unabhängig vom Träger (500–4000 Hz) unter 320 Hz; sinkt um mehr als Faktor 2 mit der Dauer bis ≈ **5 Modulationsperioden**, danach flach | — | Sinusträger | Lee 1994, JASA 96, 2140–2147 | Abstract (PubMed 7963027) | nicht frei |
| Vibrato-Rate (Saitenklang): ≈ 6 % Abweichung unhörbar; Hub viel unkritischer | 0,058 | Saitensynthese mit Korpus-AM, ~5–6 Hz Vibrato, 5 Hörer | Järveläinen 2002 (ICMC Göteborg), zusammengefasst in Järveläinen 2003, S. 38 | Volltext Zusammenfassung; Original gesperrt | quod.lib.umich.edu 403 |
| Analog Zeitintervall (Klicks): Weber-Bruch 5,7 % bei 300 ms, 5,4 % bei 200 ms, 3,2 % bei 100 ms | 0,055 … 0,031 | periodische Klickfolgen (Rate 3,3–10 Hz) | Ungan & Yagcioglu 2014, Front. Psychol. 5:1453 | Volltext-Auszug (PMC, WebFetch) | frei (CC BY), nicht lokal |
| Klavier-Praxis: Verstimmung im Chor ≈ 0,5 cent im Mittel (Stimmer Marandas); zwischen ≈ 1 und 2 cent „verstimmt“ ohne hörbare Schwebung; > 2 cent hörbare Schwebung; Schwebungsperiode in der Größe der Tondauer wird nicht als Schwebung gehört | — | Modell + Erfahrung eines Stimmers, **kein Hörversuch** | Thomas, Rousseau, Caussé, Marandas 1998 (ISMA), S. 4 | Volltext | frei (HAL), lokal (lag schon vor) |

### Größe 7 — Pegel eines Teiltons im Klang und Gesamtpegel (dB)

| Schwelle (Original) | dB | Reiz / Bedingungen | Fundstelle | Belegart | Zugang |
|---|---|---|---|---|---|
| Gesamtpegel Sinus: ΔI/I = 0,463·(I/I0)^−0,072 (71 % richtig) → ΔL ≈ 1,25 dB (20 dB SL), 0,93 dB (40 dB SL), 0,69 dB (60 dB SL), 0,50 dB (80 dB SL) | 0,5–1,4 | gepulste Sinustöne 200–8000 Hz, 5–80 dB SL | Jesteadt, Wier, Green 1977, JASA 61, 169–177 | sekundär (Formel bei Penconek 2025, Front. Neurosci. 19:1532069; PubMed ohne Abstract) | JASA nicht frei |
| Breitbandrauschen um 60 dB: ≈ 0,5 dB | 0,5 | — | Fastl & Zwicker 2007, zitiert in Plath 2019 (Diss.), S. 170 | sekundär | Plath lokal |
| „Empirische Weber-Brüche um 1 dB“ für Pegelunterscheidung | ≈ 1 | Überblick | Ewert 2002, S. 36 | sekundär | lokal |
| ISO 3382-1: Stärke G 1 dB | 1 | Raumakustik | Skålevik 2010, Folie 3 | sekundär | lokal |
| **Ein Teilton** in flachem Mehrton-Komplex (Profilanalyse, Pegel randomisiert ±10 dB): harmonischer Komplex, Teilton < 1 kHz: ≈ −15 dB (ΔA/A) ≙ **1,5 dB**; logarithmischer 21-Ton-Komplex Mitte: ≈ 0,8–1,3 dB; Rand des Spektrums ≈ 3 dB; Teilton > 5 kHz bis > 16 dB | 0,8 … 1,5 (tief/mittel); bis > 16 (hoch) | Mehrton-Komplexe 200–5000 Hz, f0 100/200/400 Hz | Žera 2024 (Überblick, IJET 70(3) 587–595), S. 589–590, Abb. 3, 4, 10, 12 (nach Green et al. 1985–1988, Žera et al. 1993) | Volltext Überblick (Originale sekundär) | frei (CC BY-NC), lokal |
| Geigenklang, Pegeländerung von Korpus-Bändern (Musiker): 3 … 6 dB | 3–6 | „virtuelle Geige“ (aufgenommene Stegkraft durch Admittanz-Filter) | Fritz, Cross, Moore, Woodhouse 2007, JASA 122, 3640–3650 | Abstract (PubMed 18247771) | HAL hinter Anubis |
| Zufällige Änderungen zeitvarianter Instrumentspektren: 8 % mittlerer Spektralfehler schlecht unterschieden, 32 % und 48 % sehr gut | 8 % ≙ 0,67 dB; 16 % ≙ 1,3 dB; 32 % ≙ 2,4 dB (Amplitude) | 8 Instrumente, Resynthese | Horner, Beauchamp, So 2004, JASA 116, 1800–1810 | Abstract (PubMed 15478447) | nicht frei |
| Glätten der Spektralhülle eines Instrumentklangs „sehr gut“ unterscheidbar; Glätten der Amplitudenschwankung mäßig bis schlecht | — (keine Zahl) | 7 Instrumente, vereinfachte Resynthese | McAdams, Beauchamp, Meneguzzi 1999, JASA 105, 882–897 | Abstract (PubMed 9972573) | nicht frei |

---

## 2. Je Größe: Sicherheit, Abhängigkeiten, offene Punkte

**1 — `vorwaerts_t60_k`.** Die belastbarste Zahl für Saitenklänge ist Tolonen & Järveläinen (2000): Bei 2-s-Tönen wird eine kürzere Abklingzeit ab etwa −21 … −30 % (|ln| 0,23–0,36), eine längere ab etwa +43 … +50 % (ln 0,36–0,40) gehört; die Autoren empfehlen 75–140 %. Abhängigkeiten: stark von der **Hördauer** (0,6-s-Töne: obere Schwelle Faktor 2,4–5,6), praktisch nicht von f0 (117–663 Hz). Die Schwellen sind asymmetrisch (kürzer wird früher gehört) und proportional, nicht linear. Grenzen der Übertragung: (a) Es wurden **alle Teiltöne gemeinsam** verändert; eine Schwelle für die Abklingzeit **eines einzelnen Teiltons k** im Klang wurde **nicht gefunden**. (b) τref war 0,6–1,2 s und die Töne wurden nach 2 s abgebrochen; Klaviertöne mit T60 von 1–30 s werden länger gehört — nach dem Gang mit der Dauer dürfte die Schwelle bei langen Tönen eher kleiner sein, belegt ist das nicht. (c) 5 Hörer, 50 %-Kriterium. Woodhouse et al. 2012 (3AFC, 79 %, beste 4 Hörer) liefern für die Saitendämpfung bei 1-s-Tönen viel größere Faktoren (2,4–3,1) — passt zur Dauerabhängigkeit bei Tolonen. Am nächsten am Klavier: Tolonen & Järveläinen (gezupfte Saite, physikalisches Modell). Ein Klavier-Hörversuch zur Abklingzeit wurde nicht gefunden.

**2 — `t60_spaet_k`.** Der einzige einschlägige Versuch (Järveläinen & Karjalainen 2002, zwei Polarisationen, Toleranz der Nachklang-Zeitkonstante) ist **im Wortlaut nicht erreichbar**; die Dissertation fasst nur zusammen, dass die Empfindlichkeit schwach ist und deutliche Abweichungen erlaubt sind. Bis das Original vorliegt, ist Tolonens Wert für das Gesamtabklingen (|ln| ≈ 0,23–0,40) die nächste belegte Untergrenze; dass der späte Teil (leiser, später) eher unempfindlicher ist, ist plausibel, aber nicht belegt. Raumakustische Doppelsteigungs-Studien (Luizard et al. 2015) messen die Koppelöffnung, nicht den späten T60 — nicht übertragbar.

**3 — `knick_tiefe_db_k`.** **Nicht gefunden** (weder Klavier noch Saite noch Raumakustik mit Knick-Pegel als Größe). Die Knicktiefe verschiebt im Wesentlichen den Pegel des Nachklangs gegen den Sofortklang; als Ersatz bietet sich die Pegel-JND (0,5–1,4 dB, Größe 7) an — das ist eine **Übertragung**, kein Beleg; sie dürfte eher zu streng sein, weil der Vergleich über Sekunden und in einer abklingenden Hüllkurve geschieht. Offen; das Original J&K 2002 könnte Zahlen zur Ähnlichkeit ein-/zweistufig enthalten.

**4 — `huelle_20/30/40_s`.** Zwei Lager: Raumakustik mit 5 % (ISO 3382-1, Seraphim 1958 an Rauschbandimpulsen; nur sekundär gelesen) und gemessene 6–39 % (Blevins et al. Abstract; 22 % mit weißem Rauschen; Klein et al. 8–15 %). Gezupfte Saite: 25–40 % (Tolonen). Die 5 % stammen aus idealen Rauschbedingungen; für ein Instrument mit langem, leise werdendem Abklingen liegen die belegten Werte an Saitenklängen deutlich höher. Abhängigkeiten: Reiz (impulsiv/Kastagnetten empfindlicher als Sprache), Nachhallzeit (kleine Räume eigene Grenze, unbestätigt 0,042 s absolut), Hörerfahrung. Am nächsten am Klavier: Tolonen (Gesamtabklingen einer Saite) — die Raumwerte beschreiben Nachhall eines Raums, nicht eines Tons. Für `huelle_40_s` (bis −40 dB) kommt hinzu, dass der späte Teil nahe der Hörschwelle bzw. unter Maskierung liegen kann; dazu nichts gefunden.

**5 — `schwebung_db_k`, `tiefe_db_k`.** Gut belegte Grundlagen, Übertragung auf Klavier offen. **Hörbarkeit überhaupt:** AM mit m ≈ 0,04–0,05 (pp ≈ 0,7–0,9 dB) bei mittleren Raten und mittleren Pegeln; bei leisen Tönen (30 dB SPL) eher m ≈ 0,1 (pp ≈ 1,7 dB). **Unterscheiden zweier Tiefen:** etwa 1,2 dB in 20 lg m (Sinusträger, Ewert & Dau, Volltext), ältere Arbeiten mit Raten ab 1 Hz „etwas über 1 dB“. Auf die Skala `schwebung_db` (pp) übertragen heißt das: für pp bis ≈ 6 dB eine relative Änderung von ≈ 15 % (ln ≈ 0,14), bei pp ≈ 9,5 dB ≈ 1,8 dB, bei pp ≈ 15 dB ≈ 4 dB (Rechnung in diesem Bericht, aus der 1,2-dB-Konstanz; für pp > 15 dB, also m > 0,7, außerhalb der gemessenen Standards). Für `tiefe_db` (Einbruch je Periode) gilt dieselbe m-Schwelle, die Umrechnung hängt von der Definition des Einbruchs ab. Abhängigkeiten: Rate — für Sinusträger flach bis ≈ 100 Hz; bei sehr langsamer Schwebung (0,1–1 Hz) und nur wenigen Perioden im hörbaren Teil des Tons ist **keine Messung gefunden**; nach Lee 1994 braucht die Ratenwahrnehmung etwa 5 Perioden. Bei Klaviertönen wird die Tiefe außerdem von der Hüllkurve des Abklingens überlagert. Am nächsten am Klavier: J&K 2002 (nur qualitativ erreichbar).

**6 — Schwebungsfrequenz / Verstimmung.** Für Raten ≥ 10 Hz liegt die Unterscheidung bei ≈ 3 Hz absolut (Formby, Hanna; nur Abstracts), also relativ grob. Für die klaviertypischen 0,1–5 Hz gibt es **keinen direkten Wert**; nächste Analoga: Vibrato-Rate an Saitenklängen ≈ 6 % (Järveläinen, nur Diss.-Zusammenfassung) und Zeitintervall-Unterscheidung ≈ 5–6 % bei 200–300 ms Periode (Klicks). Beides setzt viele Perioden voraus; beim Klavier sind bei Schwebungsfrequenzen < 0,5 Hz oft nur 1–5 Perioden hörbar, dann wird die Schwelle nach Lee 1994 deutlich größer. Da die Schwebungsfrequenz f_b ≈ f·c/1731 (c in cent) proportional zur Verstimmung ist, gilt dieselbe relative Schwelle für die Verstimmung in cent. Die Stimmer-Erfahrung (Thomas et al. 1998, kein Hörversuch) besagt: Verstimmungen bis ≈ 1–2 cent ergeben im Mittelregister keine als Schwebung hörbare Modulation, wenn die Schwebungsperiode in der Größe der Tondauer liegt. Wichtiger Befund für die Bewertung: liegt die Schwebungsperiode über der hörbaren Tondauer, wird nicht die Frequenz, sondern nur noch die Form der Hüllkurve (Größe 1–3, 5) wahrgenommen.

**7 — Pegel.** Gesamtpegel: 0,5–1,4 dB je nach Pegel über der Hörschwelle (Jesteadt, sekundär), Rauschen 0,5 dB (sekundär). Einzelner Teilton im Klang: in kontrollierten Komplexen 0,8–1,5 dB unterhalb 1 kHz, aber stark frequenzabhängig (bis > 16 dB über 5 kHz, Žera-Überblick); an realen Instrumentklängen deutlich gröber: Geige 3–6 dB für Bandänderungen (Fritz et al., Abstract), zufällige Spektralfehler 8 % (≈ 0,7 dB) schlecht, 32 % (≈ 2,4 dB) sehr gut unterscheidbar (Horner et al., Abstract). Am nächsten am Klavier: Horner et al. und Fritz et al. (zeitvariante Instrumentklänge). Die Profilanalyse-Werte gelten für stationäre Komplexe mit Pegel-Roving und sind eher eine Untergrenze.

---

## 3. Quellenliste

| # | Angabe | DOI / URL | PDF |
|---|---|---|---|
| Q1 | Tolonen, T.; Järveläinen, H. (2000): *Perceptual Study of Decay Parameters in Plucked String Synthesis.* AES 109th Convention, Los Angeles, Preprint 5205. | https://aaltodoc.aalto.fi/bitstreams/6023112e-6888-4fb8-b8b0-438795681c7b/download | `quellen/Tolonen_Jarvelainen_2000_PerceptualStudyDecayParametersPluckedString_AES109_aaltodoc.pdf` |
| Q2 | Järveläinen, H.; Tolonen, T. (2001): *Perceptual tolerances for decay parameters in plucked string synthesis.* J. Audio Eng. Soc. 49(11), 1049–1059. | AES E-Library elib 10173 | gesperrt (403) |
| Q3 | Järveläinen, H. (2003): *Perception of Attributes in Real and Synthetic String Instrument Sounds.* Diss., Helsinki Univ. of Technology, Lab. Acoustics, Report 68 (nur Zusammenfassung, ohne Artikel). | https://aaltodoc.aalto.fi/server/api/core/bitstreams/13c2d237-3b04-4fba-9285-c7db2d50574c/content | `quellen/Jarvelainen_2003_PerceptionAttributesStringInstrumentSounds_Diss_HUT_Report68_aaltodoc.pdf` |
| Q4 | Järveläinen, H.; Karjalainen, M. (2002): *Perception of beating and two-stage decay in dual-polarization string models.* Proc. ISMA 2002, Mexico City. | früher http://www.acoustics.hut.fi/~mak/PUB/Jarvelainen_isma2002.pdf (404) | nicht erhalten |
| Q5 | Järveläinen, H. (2002): *Perception-based control of vibrato parameters in string instrument synthesis.* Proc. ICMC 2002, Göteborg, 287–294. | quod.lib.umich.edu (icmc bbp2372.2002.060) | gesperrt (403) |
| Q6 | Woodhouse, J.; Manuel, E. K. Y.; Smith, L. A.; Wheble, A. J. C.; Fritz, C. (2012): *Perceptual Thresholds for Acoustical Guitar Models.* Acta Acustica united with Acustica 98, 475–486. | DOI 10.3813/AAA.918531; https://www.lam.jussieu.fr/Publications/Articles/AAA_Woodhouse_2012.pdf | `quellen/Woodhouse_Manuel_Smith_Wheble_Fritz_2012_PerceptualThresholdsGuitarModels_ActaAcustica_LAM.pdf` |
| Q7 | Fritz, C.; Cross, I.; Moore, B. C. J.; Woodhouse, J. (2007): *Perceptual thresholds for detecting modifications applied to the acoustical properties of a violin.* JASA 122(6), 3640–3650. | DOI 10.1121/1.2799475; PubMed 18247771 | HAL hal-03045382 hinter Anubis (gesperrt) |
| Q8 | Ewert, S. D. (2002): *Auditory spectro-temporal processing in the envelope-frequency domain: Experiments and models.* Diss. Univ. Oldenburg (Disputation 18.12.2002); Kap. 2 = Ewert, S. D.; Dau, T. (2004): *External and internal limitations in amplitude-modulation processing.* JASA 116(1), 478–490. | http://oops.uni-oldenburg.de/216/2/eweaud02.pdf | `quellen/Ewert_2002_AuditoryModulationProcessing_Diss_Oldenburg_oops216.pdf` |
| Q9 | Wojtczak, M.; Viemeister, N. F. (2008): *Perception of suprathreshold amplitude modulation and intensity increments: Weber's law revisited.* JASA 123(4), 2220–2236. | DOI 10.1121/1.2839889; PMC2394195 | online frei, nicht lokal |
| Q10 | Kohlrausch, A.; Fassel, R.; Dau, T. (2000): *The influence of carrier level and frequency on modulation and beat-detection thresholds for sinusoidal carriers.* JASA 108(2), 723–734. | DOI 10.1121/1.429605; PubMed 10955639 | nicht frei |
| Q11 | Viemeister, N. F. (1979): *Temporal modulation transfer functions based upon modulation thresholds.* JASA 66(5), 1364–1380. | DOI 10.1121/1.383531; PubMed 500975 | nicht frei (Abstract ohne Schwellenwert in dB) |
| Q12 | Wakefield, G. H.; Viemeister, N. F. (1990): *Discrimination of modulation depth of sinusoidal amplitude modulation (SAM) noise.* JASA 88(3), 1367–1373. | DOI 10.1121/1.399714; PubMed 2229672 | nicht frei |
| Q13 | Formby, C. (1985): *Differential sensitivity to tonal frequency and to the rate of amplitude modulation of broadband noise by normally hearing listeners.* JASA 78(1), 70–77. | DOI 10.1121/1.392456; PubMed 4019909 | nicht frei |
| Q14 | Hanna, T. E. (1992): *Discrimination and identification of modulation rate using a noise carrier.* JASA 91(4), 2122–2128. | DOI 10.1121/1.403698; PubMed 1597603 | nicht frei |
| Q15 | Lee, J. (1994): *Amplitude modulation rate discrimination with sinusoidal carriers.* JASA 96(4), 2140–2147. | DOI 10.1121/1.410156; PubMed 7963027 | nicht frei |
| Q16 | Ungan, P.; Yagcioglu, S. (2014): *Significant variations in Weber fraction for changes in inter-onset interval of a click train over the range of intervals between 5 and 300 ms.* Front. Psychol. 5:1453. | DOI 10.3389/fpsyg.2014.01453; PMC4264405 | online frei, nicht lokal |
| Q17 | Thomas, Rousseau, Caussé, Marandas (1998): Hammer irregularities, mistuning, double decay (ISMA 1998), HAL hal-01105499. | — | `quellen/Thomas_Rousseau_Causse_Marandas_1998_HammerIrregularitiesMistuningDoubleDecay_ISMA_hal-01105499.pdf` (lag vor) |
| Q18 | Jesteadt, W.; Wier, C. C.; Green, D. M. (1977): *Intensity discrimination as a function of frequency and sensation level.* JASA 61(1), 169–177. | DOI 10.1121/1.381278; PubMed 833368 (ohne Abstract) | nicht frei |
| Q19 | Penconek, M. (2025): *Weber's Law as the emergent phenomenon of choices based on global inhibition.* Front. Neurosci. 19:1532069 (gibt Q18-Formel wieder). | DOI 10.3389/fnins.2025.1532069 | online frei, nicht lokal |
| Q20 | Žera, J. (2024): *Profile Analysis: A framework for the study of auditory sound spectrum analysis.* Int. J. Electronics and Telecommunications 70(3), 587–595. | DOI 10.24425/ijet.2024.149583; https://www.journals.pan.pl/Content/132216/8-4668-Zera-sk.pdf | `quellen/Zera_2024_ProfileAnalysisFramework_IJET70-3_PAN.pdf` (CC BY-NC 4.0) |
| Q21 | Horner, A.; Beauchamp, J.; So, R. (2004): *Detection of random alterations to time-varying musical instrument spectra.* JASA 116(3), 1800–1810. | DOI 10.1121/1.1778741; PubMed 15478447 | nicht frei |
| Q22 | McAdams, S.; Beauchamp, J. W.; Meneguzzi, S. (1999): *Discrimination of musical instrument sounds resynthesized with simplified spectrotemporal parameters.* JASA 105(2), 882–897. | DOI 10.1121/1.426277; PubMed 9972573 | nicht frei |
| Q23 | Blevins, M. G.; Buck, A. T.; Peng, Z.; Wang, L. M. (2013): *Quantifying the just noticeable difference of reverberation time with band-limited noise centered around 1000 Hz using a transformed up-down adaptive method.* ISRA 2013, Toronto. | https://digitalcommons.unl.edu/archengfacpub/63 (Abstract) | PDF 403 (gesperrt) |
| Q24 | Klein, F.; Amengual Garí, S. V.; Arend, J. M.; Robinson, P. W. (≈2021): *Towards determining thresholds for room divergence: A pilot study on detection thresholds.* (Konferenzbeitrag, Ort im PDF nicht genannt). | https://research.facebook.com/file/1784430561898527/… | `quellen/Klein_AmengualGari_Arend_Robinson_2021_RoomDivergencePilotDetectionThresholds_FacebookResearch.pdf` |
| Q25 | Skålevik, M. (2010): *Reverberation time — the mother of all acoustical parameters.* Vortrag BNAM 2010 (Folien; Folie 3 gibt ISO-3382-1-Tabelle wieder). | https://www.akutek.info/Presentations/MS_Reverberationtime_BNAM2010_Pres.pdf | `quellen/Skalevik_2010_ReverberationTime_BNAM2010_Praesentation_akutek.pdf` |
| Q26 | ISO 3382-1:2009, Anhang A, Tab. A.1. | — | nicht frei |
| Q27 | Seraphim, H. P. (1958): *Untersuchungen über die Unterschiedsschwelle exponentiellen Abklingens von Rauschbandimpulsen.* Acustica 8, 280–284. | — | nicht frei |
| Q28 | Luizard, P.; Katz, B. F. G.; Guastavino, C. (2015): *Perceptual thresholds for realistic double-slope decay reverberation in large coupled spaces.* JASA 137(1), 75–84. | DOI 10.1121/1.4904515; PubMed 25618040 | HAL hinter Anubis |
| Q29 | Bank, B. (2006): Diss. (lag vor) — zitiert Q2 mit −25 %/+40 % auf S. 44. | — | `quellen/Bank_2006_StringInstrumentsGeometricNonlinearities_PhD.pdf` |
| Q30 | Plath, N. (2019): Diss. (lag vor) — zitiert Fastl & Zwicker 0,5 dB auf S. 170. | — | `quellen/Plath_2019_WorkshopToConcertHall_Diss.pdf` |

---

## 4. Reichweite der Suche und Nicht-Gefundenes

**Gesucht über:** WebSearch (Standard und erweitert), PubMed-Suchseiten (Abstracts), PMC (Volltexte), aaltodoc/Aalto-Forschungsportal, research.fi, Oldenburger OOPS, PAN-Repositorium, LAM-Jussieu-Publikationsliste, digitalcommons.unl.edu, euphonics.org (Woodhouse), KTH-Seiten (Weinreich-Vortrag), und Volltextsuche in allen schon unter `quellen/` liegenden PDFs (Begriffe noticeable, JND, difference limen, audibility, inaudible, perceptible).

**Begriffe (Auswahl):** „perceptual tolerances decay parameters plucked string“, „perception of beating and two-stage decay“, „just noticeable difference reverberation time“, „double slope decay perception threshold“, „amplitude modulation depth discrimination“, „amplitude modulation rate discrimination“, „profile analysis increment single component“, „intensity discrimination Jesteadt“, „tuning preferences piano unison groups“, „perceptual thresholds guitar/violin“.

**Gesperrt (Bot-Schutz oder 403), nicht umgangen:** AES E-Library (Q2, Niaounakis & Davies 2002), HAL (hal.archives-ouvertes.fr / hal.sorbonne-universite.fr, Anubis: Q7, Q28, Frissen et al.), LISN-Seiten (403), repository.gatech.edu (403), digitalcommons.unl.edu PDF (403), quod.lib.umich.edu (403), Salford-Repositorium (403), aaltodoc-Item-Seite (403; Bitstreams gingen), Web-Archiv (429).

**Nicht gefunden (kein Treffer ist kein Beweis):**
- Ein **Hörversuch an Klaviertönen** zu Abklingzeit, Nachklang, Knick oder Schwebung — keiner gefunden. Kirk 1959 (*Tuning preferences for piano unison groups*, JASA 31, 1644–1648) ist nur als Zitat belegt; Inhalt nicht erreicht.
- Schwelle für die Abklingzeit **eines einzelnen Teiltons** im komplexen Klang.
- Schwelle für die **Knicktiefe** einer zweistufigen Hüllkurve (Instrument oder Raum).
- Zahlen aus Järveläinen & Karjalainen 2002 (Original nicht erreichbar).
- AM-Tiefen- und Ratenunterscheidung bei **sehr langsamen Raten (0,1–1 Hz)** und abklingenden Trägern.
- Meng, Zhao, He 2006 (JND Nachhall) — nur als Zitat (Klein et al., Ref. 12) gesehen.
- Ein frei zugänglicher Primärtext mit dem dB-Wert der AM-Erkennungsschwelle (Viemeister 1979, Kohlrausch et al. 2000): die Zahlen m ≈ 0,05 bzw. −28 dB stammen aus Sekundärquellen.

**Vorsicht bei den Zahlen:** pdftotext hat die Tabellen in Q1 durcheinandergebracht; Tab. 6 wurde deshalb am gerenderten Seitenbild (S. 11) geprüft. Die Umrechnungen in ln und pp-dB sind hier gerechnet (Skript `scratchpad/umrechnung.py`), keine Literaturwerte.
