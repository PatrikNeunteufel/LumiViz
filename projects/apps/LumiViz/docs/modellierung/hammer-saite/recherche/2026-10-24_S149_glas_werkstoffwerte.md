# Recherche S149: Werkstoffwerte für einen Boden aus Glas (31c-8)

> **Datum:** 2026-10-24 (Session 149) · **Auftrag:** Frage 11 aus PL31 a — Werte für Glas mit Quelle (Dichte, E, ν,
> G, vor allem der Verlustfaktor η im Bereich 50 Hz–5 kHz) für Kalk-Natron-Floatglas, Borosilikat, ESG/VSG und
> zum Vergleich PMMA; dazu Instrumente oder Lautsprecher aus Glas, Festigkeit gegen die Saitenlast.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [wörtlich] = im Volltext gesehen, [nur Zusammenfassung] = Abstract, Suchergebnis oder Sekundärquelle.
> **Von Claude selbst nachgelesen** (an den Kopien des Abrufwerkzeugs, mit `quelle_suchen.py`): Irvine 2010 S. 5,
> Tab. 7: „Glass 2.5 60e+10 4.9e+05 0.6-2e-03“ (und Fir „8e-03“); Lenk & Coult 2010 S. 5, Tab. 1: „Glass 1x10-5 –
> 2x10-3“ und im Text die zwei Spannen nach Ashby und Beards; NGA FM05-12 S. 1: „71.7 GPa“. Alles andere: nur Bericht.
> **Quellen-PDFs:** nicht abgelegt (die Kopien des Abrufwerkzeugs liegen außerhalb des Repos und gehen mit der
> Sitzung verloren).
> **Eingeflossen in:** noch nicht — Vorlage der Fragen 10, 11 und 13 zu 31c-8 (PL31 a).

---

## Kurzfazit: empfohlene Datensätze

| Werkstoff | ρ (kg/m³) | E (GPa) | ν | G (GPa) | η (Raumtemp.) | beste Quelle | Sicherheit |
|---|---|---|---|---|---|---|---|
| Kalk-Natron-Floatglas (EN 572) | 2500 | 70 (NGA: 71,7) | 0,2 (NGA: 0,22) | ≈ 29,2 aus E, ν (NGA gibt 29,6 an) | **1·10⁻³** (Spanne 0,6…2·10⁻³), frequenzunabhängig angesetzt | ρ, E, ν: AGC- und Saint-Gobain-Datenblätter bzw. NGA FM05-12. η: Cremer/Heckl, über Irvine | ρ, E, ν sicher (±3 %). η unsicher, nur eine Spanne aus einer Lehrbuchtabelle. Ein Messwert für monolithisches Glas im Bereich 50 Hz–5 kHz wurde nicht gefunden. |
| Borosilikatglas (Borofloat 33, EN 1748-1-1) | 2230 | 64 | 0,2 | ≈ 26,7 aus E, ν | wie Floatglas ansetzen (≈ 1·10⁻³). Ein eigener Wert wurde nicht gefunden. | SCHOTT-Datenblatt | ρ, E, ν sicher. η nicht belegt. |
| ESG / TVG | wie Floatglas | wie Floatglas | wie Floatglas | – | wie Floatglas | Vorspannung ändert nur die Festigkeit | Dass Steifigkeit und Dämpfung gleich bleiben, ist Fachwissen und nicht eigens belegt. |
| VSG (2 × Glas + PVB) | Schichten getrennt rechnen | Glas wie oben; PVB viskoelastisch | – | – | **Verbund-η ≈ 0,1–0,3 bei 20 °C**. Stark von der Temperatur abhängig (10 °C → 30 °C: Faktor bis ≈ 7) | Patente (Dow, ISO-16940-Messungen) | Nur aus Zusammenfassungen. Größenordnung plausibel, Einzelwerte nicht gesichert. |
| PMMA (Plexiglas) | 1190 | 3,3 statisch (kurzzeitig, 23 °C); **≈ 5,6 dynamisch** | 0,37 | – | **2…4·10⁻²** | Datenblatt Plexiglas XT; dynamische Werte aus Cremer/Heckl | Statische Werte sicher. η und E_dyn nur aus einer Tabelle ohne Frequenzangabe. |

**Einordnung für das Modell:**
- Der Verlustfaktor von Glas ist etwa eine Größenordnung kleiner als der von Fichte. Die Fichten-Resonanzböden in den Piano-Arbeiten von Ege/Boutillon liegen bei 1–3 %; das kenne ich nur aus Suchergebnissen.
- Bei einem Glasboden werden deshalb Abstrahlung, Einspannung, Rippen und Steg die Modendämpfung bestimmen, nicht das Glas selbst. Ein konstantes η = 1·10⁻³ für das Glas ist vertretbar. Die Quellen sagen ausdrücklich, dass Strukturdämpfung aus Fügestellen die Materialdämpfung übersteigt (Lenk/Coult).
- Eigene Rechnung, nicht aus einer Quelle: Bei gleicher Masse wird der Glasboden sehr dünn. 9 mm Fichte mit ρ ≈ 420 entsprechen etwa 1,5 mm Glas.
- Eigene Rechnung: Die Koinzidenzfrequenz einer Glasplatte liegt bei f_c ≈ 12/h[m] Hz. Das sind etwa 2,0 kHz bei 6 mm und 1,3 kHz bei 9 mm, also mitten im Rechenbereich.

## Quellen und Funde

**1. NGA (National Glass Association), Glass Technical Paper FM05-12 (2023): „Physical and Mechanical Properties of Typical Soda Lime Float Glass“.** Frei zugänglich als PDF auf glass.org. Volltext gelesen.
- [wörtlich] S. 1: „Modulus of Elasticity for glass is ~ 10.4 x 106 psi (71.7 GPa)“
- [wörtlich] S. 1: Schubmodul „~ 4.3 x 106 psi (29.6 GPa)“, Poisson „~ 0.22“
- [wörtlich] S. 2: Dichte „~ 156 lb/ft3 (2500 kg/m3)“
- [wörtlich] S. 2: Bruchmodul (Modulus of Rupture), Mittelwert / Bemessung bei 8 von 1000:
  - Annealed (gekühltes Floatglas) 41 / 19 MPa
  - Heat-strengthened (teilvorgespannt) 83 / 39 MPa
  - Fully tempered (vollvorgespannt) 166 / 77 MPa
  - Mit der Anmerkung „short load durations (under 1 minute)“.
- Keine Angabe zur Dämpfung.

**2. AGC Planibel, Saint-Gobain Sekurit, Vasglass, Whitefloat-Datenblatt.**
- [nur Zusammenfassung] Durchgehend 2500 kg/m³, 70 000 N/mm² und ν = 0,2. Bei Saint-Gobain Sekurit ohne ν.
- Die AGC-PDF war nicht abrufbar (503), die Sekurit-Seite ebenfalls nicht (403).
- **Widerspruch:** Die europäischen Datenblätter nennen 70 GPa und ν = 0,2, die NGA 71,7 GPa und 0,22. Der EN-572-1-Wert selbst ist nicht gesehen, weil die Norm kostenpflichtig ist.

**3. SCHOTT BOROFLOAT 33.**
- (a) Produktseite schott.com/en-us/products/borofloat/technical-details:
  - [wörtlich] „Density ρ (25° C): 2.23 g/cm3“
  - [wörtlich] „Young´s Modulus E (according to DIN 13316): 64 kN/mm2“
  - [wörtlich] „Poisson's Ratio μ (according to DIN 13316): 0.2“
  - Keine Dämpfung, keine Zahl zur Biegefestigkeit.
- (b) SCHOTT-Datenblatt bei UQG Optics, Stand 01.12.14:
  - [wörtlich] Dichte 2.2 g/cm3
  - [wörtlich] Biegefestigkeit „25 MPa (to DIN 52 292 T1)“
  - [wörtlich] Bezug auf „EN 1748 T1“
- Die 25 MPa sind offenbar ein vorsichtiger Gebrauchswert und nicht mit den Mittelwerten der NGA vergleichbar.

**4. BauNetz Wissen, „Festigkeit von Glas“ (baunetzwissen.de).** Sekundärquelle für die Normwerte.
- [wörtlich, als Zitat des Abrufwerkzeugs] „DIN EN 572-1, … Floatglas: 45 MPa“
- [wörtlich, als Zitat des Abrufwerkzeugs] „DIN EN 1863-1, teilvorgespanntes Glas (TVG): 70 MPa“
- [wörtlich, als Zitat des Abrufwerkzeugs] „DIN EN 12150-1, … (ESG): 120 MPa“
- Das sind charakteristische Biegezugfestigkeiten. Laut Seite sind sie ein Qualitätsmerkmal und keine Materialkonstante.
- [nur Zusammenfassung] Unter Dauerlast setzt DIN 18008 einen Lastdauerbeiwert ab; Floatglas wird dadurch deutlich schwächer.

**5. T. Irvine, „Damping Properties of Materials“, Rev. D, 2. Juli 2010.** vibrationdata.com, frei. Volltext gelesen; Tabelle 7 nach eigener Angabe aus Cremer/Heckl, *Structure-Borne Sound*, Springer 1988.
- [wörtlich] Tabelle 7, S. 5, Zeile Glass: „2.5 / 60e+10 / 4.9e+05 / 0.6-2e-03“. Das heißt g/cm³, dyn/cm² (= 60 GPa), cm/s und Verlustfaktor.
- [wörtlich] Zeile Plexiglas: „1.15 / 5.6e+10 / 2.2e+05 / 2 - 4e-02“.
- Vergleichswerte aus derselben Tabelle: Fir (Tanne) η „8e-03“, Plywood „1.3e-02“.
- Vorbehalt: Das Tabellenlayout ist im PDF verschoben. Die Glas- und Plexiglas-Zeilen sind aber in sich stimmig (√(E/ρ) ergibt jeweils die genannte Wellengeschwindigkeit).
- Das Original Cremer/Heckl habe ich nicht gesehen.
- **Widerspruch:** E = 60 GPa für Glas statt 70 GPa. Das ist ein älterer bzw. gerundeter Lehrbuchwert; für das Modell nicht übernehmen.

**6. P. Lenk, G. Coult (Eckersley O'Callaghan), „Damping of Glass Structures and Components“.** Challenging Glass 2, TU Delft, Mai 2010. Volltext gelesen.
- [wörtlich] Tabelle 1 („Dynamic properties of Materials“, nach Ashby 2005 und de Silva 2007): Glass „1x10-5 – 2x10-3“.
- [wörtlich] Im Text: „from 2x10-4 to 1x10-5“ nach Ashby, „higher values from 2x10-3 to 6x10-4“ nach Beards 1996.
- **Widerspruch zwischen den Lehrbüchern:**
  - Ashby: 10⁻⁵…2·10⁻⁴
  - Beards und Cremer/Heckl: 6·10⁻⁴…2·10⁻³
  - Mein Vorschlag 1·10⁻³ folgt den beiden akustischen Quellen.
- Fazit der Autoren: Die Fügestellen dämpfen mehr als das Material. Es sind gerechnete Beispiele, keine Messung an monolithischem Glas.

**7. M. Novotný, B. Poot (AGC), „Influence of Temperature on Laminated Glass Performances…“.** Challenging Glass 5, Gent 2016. Volltext gelesen.
- [wörtlich] Messung nach ISO 16940 an Balken 25×300 mm aus 2 × 4 mm Glas.
- [wörtlich] Rauschanregung „0-5000 Hz“, Temperaturen −20 bis 80 °C.
- Ergebnis nur als Rw/STL in Bildern; Zahlen zum Verlustfaktor habe ich im Text nicht gefunden.
- [wörtlich] Die Zwischenschicht „provides a significant amount of internal structural damping“.

**8. ISO 16940:2008.** Volltext-OCR auf archive.org.
- [wörtlich] „Measurements shall be carried out at (20 ± 1) °C“, Glas „nominally 4 mm thick“, Balken 25 × 300 mm.
- Keine Bezugswerte für η.

**9. Patente zum VSG-Verlustfaktor.**
- US 7297407 (Dow), Tabelle 1, 20 °C, 50–10 000 Hz, ISO/PDPAS 16940. [nur Zusammenfassung; Zuordnung zu den Frequenzen unklar]
  - Glas/90 mil Standard-PVB/Glas: η ≈ 0,10–0,19
  - Glas/Akustik-PVB/Glas: η ≈ 0,19–0,29
- Ein japanisches Interlayer-Patent: [nur Zusammenfassung] η = 0,4865 bei 180 Hz und 30 °C, 0,071 bei 250 Hz und 10 °C.
- Ein weiteres Patent: [nur Zusammenfassung] Zielwert etwa 0,20 bei 2000 Hz für Schallschutz-VSG.
- Zhu et al. 2024 (J. Low Freq. Noise Vib. Act. Control): [nur Zusammenfassung] in der Simulation η_Glas = 0,002 und η_PVB = 0,1.

**10. Plexiglas XT 0A000, Datenblatt bei pictureframesexpress.co.uk.**
- [wörtlich, als Zitat des Abrufwerkzeugs] „Modulus of elasticity Et (short-term value)“ 3300 MPa.
- Dichte 1,19 g/cm³, ν 0,37 bei 23 °C, Biegefestigkeit 105 MPa.
- [nur Zusammenfassung] Abweichende Angaben anderswo: Alro-Broschüre ν 0,35; ein niederländisches GS/XT-Blatt E 3100 MPa und Biegefestigkeit 110 MPa.
- Die Röhm/Evonik-Originalunterlage habe ich nicht gesehen.

**11. Downbearing (Saitenlast auf den Steg).**
- US 4697494 (T. A. Lowell, 1987): [wörtlich, als Zitat des Abrufwerkzeugs] „a total amount of 400 lbs. to 2,000 lbs to be in a satisfactory range“. Das sind etwa 1,8–8,9 kN (eigene Umrechnung).
- US 10204601 (Hurstwood Farm Piano Studios): [wörtlich, als Zitat des Abrufwerkzeugs] „about 4 to 8 lbs (1.8 to 3.6 Kg) per string“, zusammen „between 900 and 1 800 lbs“. Das sind etwa 4–8 kN.
- PTG-Forum zum Steinway L: [nur Zusammenfassung] Gesamtzug ≈ 165 kN, Downbearing ≈ 2,9 kN bei 1°.
- Ob ein Glasboden bricht, kann ich nicht beantworten; das braucht eine Spannungsrechnung. Anhaltspunkte:
  - Die Last von wenigen kN liegt verteilt über den Steg auf der Platte und wirkt dauerhaft.
  - Floatglas hat charakteristisch 45 MPa, unter Dauerlast deutlich weniger. ESG hat 120 MPa.
  - Ein dünner Boden auf gleiche Masse (≈ 1,5 mm) wäre sehr wahrscheinlich kritisch. Das ist eine Einschätzung, nicht belegt.

**12. Instrumente und Lautsprecher mit Glas.**
- Dumčius/Bernatavičius, „The Research of the DML Loudspeakers Properties“, Elektronika ir Elektrotechnika 2008 Nr. 8(88), KTU. Volltext gesehen.
- [wörtlich] DML-Modelle mit „glass panel 500x600x10 mm“ und 300 × 300 × 5 mm. Zahlen zur Dämpfung des Glases habe ich im Text nicht gefunden.
- Weinglas: Q ≈ 600 bei 1 s Abklingzeit um 200 Hz. Das ist eine Übungsaufgabe von Dartmouth bzw. eine Umrechnung im Suchergebnis, kein Messwert.

## Nicht gefunden

- Ein **gemessener, frequenzaufgelöster Verlustfaktor von monolithischem Kalk-Natron- oder Borosilikatglas** zwischen 50 Hz und 5 kHz. Die Ashby- bzw. Lehrbuchwerte sind Spannen ohne Frequenz.
- Die Primärarbeiten zur inneren Reibung (Alkali-γ-Peak) mit Zahlen bei Raumtemperatur im kHz-Bereich. Nur Abstracts gefunden.
- Die Tabelle aus EN 572-1 und EN 1748-1-1 im Original (kostenpflichtig).
- Das Original von Cremer/Heckl.
- Messungen von E(f) und η(f) an PMMA im Audiobereich. Es gibt nur den Hinweis, dass der dynamische E-Modul (5,6 GPa) über dem statischen (3,3 GPa) liegt; eine Frequenzangabe dazu fehlt.
- VSG-Primärdaten mit vollständigem η(f, T). Nur Zusammenfassungen von Patenten; die Volltexte von ScienceDirect und ResearchGate waren gesperrt (403).
- Ein Musikinstrument mit Glasdecke und gemessener Dämpfung. Auch zum Glasflügel von Schimmel habe ich keine technischen Daten gefunden.
- Conklins Downbearing-Zahlen (JASA 1996).

## Reichweite der Suche

- **Suchen:** 22 Websuchen, davon 7 im erweiterten Modus.
- **Volltexte gelesen** (über WebFetch, die PDFs über die vom Werkzeug angelegten Kopien mit pdftotext auf die Konsole; im Repo nichts gespeichert):
  - NGA FM05-12
  - Lenk/Coult 2010
  - Novotný/Poot 2016
  - Irvine 2010
  - SCHOTT-Datenblatt (UQG)
  - SCHOTT-Webseite
  - Dumčius 2008
  - Zemanová et al. 2018 (arXiv:1801.01110; rein rechnerisch, keine Glaswerte)
  - ISO 16940 (OCR-Fassung)
  - BauNetz Wissen
  - Plexiglas-Datenblatt
  - zwei Patentseiten zum Downbearing und eine Dow-Patentseite
- **Nicht abrufbar:** AGC-PDF (503), Saint-Gobain Sekurit (403), sandv.com (Zertifikatsfehler), ScienceDirect und ResearchGate (403), web.archive.org (gesperrt), Energy-Procedia-PDF der IUAV (403).
- Die vom Werkzeug abgelegten PDF-Kopien liegen außerhalb des Repos unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\6e750082-935a-481c-9efc-7ba02ae01ec8\tool-results\`. Ich habe sie nicht selbst angelegt.
