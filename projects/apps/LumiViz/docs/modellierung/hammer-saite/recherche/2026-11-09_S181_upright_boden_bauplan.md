# Recherche S181 — Geometrie eines Upright-Resonanzbodens mit Maßen (34i, KL3-1 a)

> **Auftrag** (S181, 2026-11-09, Opus 5.5 an einen Recherche-Agenten): eine veröffentlichte Quelle mit der Geometrie
> eines Upright-Bodens in Zahlen — Umriss, Dicke/Holz/Faser, Rippen (Lage, Querschnitt, Winkel), Diskant- und Bassteg als
> Koordinaten oder bemaßte Zeichnung, Stegpunkte je Taste — für einen Bauplan wie `steinway_d` (KL3-1 a). Schon geprüft
> und ausgeschlossen: Chaigne, Cotté & Viggiano 2013; Boutillon & Ege 2013; Ege, Boutillon & Rébillat 2013; Ege &
> Boutillon 2012. Nur WebSearch/WebFetch, kein Browser, keine Downloads ins Repo.
>
> **Kennzeichnung:** Der Text unter dem Strich ist der Bericht des Agenten, **unverändert**. **[gelesen]** = an der Quelle
> gelesen; **[Abstract/Suche]** = nur aus Suchergebnis, Abstract oder Metadaten. Die PDFs, die WebFetch im Sitzungsordner
> abgelegt hat (`tool-results/webfetch-*.pdf`), liegen nicht im Repo.
>
> **Folgerung (Claude S181):** KL3-1 a bleibt — kein frei lesbarer, vollständiger Satz. Nächster Schritt: die
> HAL-Volltexte zum Pleyel P131 (Trévisan 2016 tel-01451727, Trévisan et al. 2017 hal-01456083, Trévisan et al. 2015
> hal-01217078, Chaigne 2010 hal-00538347) **von Hand** öffnen — HAL sperrt automatische Abrufe.

---

# Recherche: Geometrie eines Upright-Resonanzbodens mit Zahlen (für einen FE-Bauplan)

**Stand:** 2026-11-09 · nur WebSearch/WebFetch, kein Browser, keine Anmeldung, keine Formulare.
**Hinweis zur Ablage:** WebFetch hat fünf abgerufene PDFs ohne mein Zutun im Sitzungsordner `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\acda622c-b2e5-4745-956d-e7d38d6cdc0b\tool-results\` abgelegt (`webfetch-*.pdf`). Das waren Wood Research 2014, Tippner/Koňas 2007, Xiao WJM 2026, Xiao MME 2026, Elie 2022, Elie ISMA 2019 und Nakamura 1993. Ich habe sie nur mit `pdftotext`/`pdftoppm` gelesen. Textauszüge und Seitenbilder liegen im Scratchpad. Ins Repo wurde nichts gespeichert. Die Dateien können gelöscht werden.

**Kennzeichnung:** **[gelesen]** heißt: an der Quelle selbst gelesen (Text oder Seitenbild). **[Abstract/Suche]** heißt: nur aus Suchergebnis, Abstract oder Metadaten. Zahlen stehen so, wie sie in der Quelle stehen. Längere wörtliche Zitate gebe ich aus urheberrechtlichen Gründen nicht wieder; die Zahlen selbst sind vollständig übernommen.

---

## Kurzbefund

**Keine erreichbare Quelle liefert alle fünf Punkte für ein Instrument.** Insbesondere fand ich **keine frei erreichbare Quelle mit Stegkoordinaten (Punkt 4) oder Stegpunkten je Saite (Punkt 5) eines Upright-Bodens**.

- **Am nächsten an Punkt 3 (Rippen):** Xiao, Su & Gong 2026. Das sind zwei Aufsätze zu Pearl-River-Uprights UH121 und UR121, offen unter CC BY 4.0.
  - Sie geben die Rippenquerschnitte aller 11 Rippen an, beim UH121 dazu die Rippenlängen und beim UR121 den Rippenwinkel 54°.
  - Es fehlen die Rippenlagen, die Stegkoordinaten und die Stegpunkte. Das CAD-Modell ist ausdrücklich nicht veröffentlicht.
- **Wahrscheinlich vollständigste Quellen, nicht erreicht:** Trévisans Arbeiten zum Pleyel P131 (Thèse 2016 und JASA 2017). Sein analytisches Modell braucht Rippenlagen und Stegverlauf als Eingabe. HAL sperrt automatische Abrufe vollständig (Anubis, „Access Denied“). Von Hand im Browser dürfte HAL erreichbar sein.

---

## Geprüfte Quellen

### 1. Xiao, Su & Gong (2026a): UH121, statische Analyse — **[gelesen]**
- **Angabe:** Xiao, W., Su, W., Gong, C.Z. (2026). *Static Response and Load Transfer of a UH121 Upright-Piano Soundboard under Full-String Downbearing: A Finite Element Study.* World Journal of Mechanics 16(9), 101–118. DOI 10.4236/wjm.2026.169006. PDF: https://www.scirp.net/pdf/wjm_4900800.pdf
- **Zugang:** frei.
- **Lizenz (S. 101):** „licensed under the Creative Commons Attribution International License (CC BY 4.0)“.
- **Herkunft der Geometrie:** aus den Produktions-CAD-Zeichnungen des UH121 rekonstruiert (S. 103). Der Erstautor arbeitet bei Guangzhou Pearl River Kayserburg Piano.
- **Punkt 1, Umriss (S. 103):** Hüllmaß 1429 mm ± 0,5 mm × 955 mm ± 0,5 mm (als „reference envelope“ bezeichnet). Abgeschnittene Ecken werden nicht beziffert.
- **Punkt 2, Dicke und Werkstoff (S. 103, 104):**
  - Dicke: Nennmaß 8 mm, laut Zeichnung ein örtlicher 7-mm-Bereich in der Mittellage.
  - Werkstoff im Modell: isotrop, E = 14150 MPa, ν = 0,328 (S. 104). Faserrichtung ist nicht angegeben.
- **Punkt 3, Rippen (S. 103–104):**
  - Anzahl: 11.
  - Breite × Höhe, Rippe 1 bis 11: 20×19, 20×20, 22×20, 22×20, 24×23, 25×27, 25×25, 24×22, 22×20, 20×19, 20×19 mm.
  - Nennlängen: 348, 506, 665, 823, 989, 1110, 996, 891, 786, 682, 577 mm.
  - Lage (Position oder Abstand) und Winkel: **nicht angegeben**.
- **Punkt 4, Stege (S. 104):** nur Querschnitte.
  - Diskant/Mittellage: Breitenpaare 35/22 mm (Mittellage) und 32/20 mm (Diskant); örtliche Maße 4,5 mm (Mittellage) und 4,25 mm (Diskant).
  - Bass: 30 mm breit, 25 mm hoch; Mittelteil 16 mm, zwei Schultern je 7 mm.
  - **Lage oder Koordinaten der Stege: nicht angegeben.**
- **Punkt 5, Stegpunkte (S. 106):** 219 Saiten mit je zwei Stegstiften, also 438 Lastpunkte; davon 344 am Diskantsteg und 94 am Bassteg.
  - Summen der Kräfte: 5004,811107 N (Diskant) und 1408,653862 N (Bass), zusammen −6413,5 N.
  - **Koordinaten der Punkte werden nicht angegeben.** Die Datei mit den Längen je Saite sei „no longer available“.
- **Daten (S. 110, §2.10):** Das Modell enthält eingeschränkte Produktgeometrie und wird nicht veröffentlicht. Abgeleitete Daten gibt es höchstens auf Anfrage.
- **Abbildungen:** Modell- und Konturbilder. Ich habe die Bilder selbst nicht angesehen; der Text nennt keine Achsenbemaßung.

### 2. Xiao, Su & Gong (2026b): UR121, Schnellbewertung — **[gelesen]**
- **Angabe:** Xiao, W., Su, W., Gong, C.Z. (2026). *A Rapid Static Assessment Method for a UR121 Upright-Piano Soundboard Based on Finite-Element Calibration and Discrete Bridge-Load Redistribution.* Modern Mechanical Engineering 16(4), 140–158. DOI 10.4236/mme.2026.164008. PDF: https://www.scirp.net/pdf/mme_1860634.pdf
- **Zugang:** frei. **Lizenz (S. 140):** CC BY 4.0, gleicher Wortlaut wie oben.
- **Herkunft der Geometrie:** aus der CAD/DXF-Zeichnung des UR121 rekonstruiert (S. 143). Das ist ein anderes Modell als der UH121.
- **Punkt 1:** Umriss nicht beziffert.
  - Ein globales Koordinatensystem ist definiert (S. 143): Ursprung am Randpunkt oben links in der Rippenseiten-Ansicht, +X nach rechts, +Y nach oben; die Platte liegt überwiegend bei +X/−Y.
  - Tabelle 1 nennt darin Orte von Extremwerten, z. B. größte Verformung bei etwa (583, −449, 8) mm.
- **Punkt 2 (S. 142):**
  - Fichtenproben parallel zur Faser: E = 14,15 GPa, ν = 0,328.
  - Holzart darüber hinaus, Feuchte und Dicke: nicht angegeben.
- **Punkt 3:**
  - Rippen 1–11 von links nach rechts, Breite/Höhe (S. 143): 22/17, 22/18, 23/19, 26/22, 28/24, 30/25, 30/25, 28/24, 24/21, 22/18, 20/16 mm.
  - Gemeinsamer Orientierungsparameter 54° (S. 147).
  - Rippenlagen und Spannweiten: nicht angegeben.
- **Punkt 4:** nur „curved bridge paths“ erwähnt, keine Zahlen.
- **Punkt 5 (S. 142):**
  - 88 Tasten, 219 Saiten (14 einchörig, 17 zweichörig, 57 dreichörig), 438 Stegkontakte.
  - Einzelkräfte zwischen 1,9377 und 20,3159 N, Summe 4234,2501 N.
  - Keine Koordinaten.

### 3. Klímová & Tippner (2014): Petrof-Upright, ANSYS — **[gelesen]**
- **Angabe:** Klímová, H., Tippner, J. (2014). *Modal analysis of soundboard of the upright piano by finite element method (FEM).* Wood Research 59(1), 123–136. PDF: https://www.woodresearch.sk/wr/201401/11.pdf
- **Zugang:** frei. **Lizenz:** keine Angabe gesehen.
- **Herkunft:** Modell nach Bauplänen von Petrof-Uprights, parametrisch in APDL (S. 125).
- **Punkt 2 (S. 124–127):**
  - Dicke: Keil 6,5–9,5 mm (Standard), Variante 5,5–7,0 mm.
  - Holz: Fichte (Picea abies), Dichte 420–460 kg/m³; Steg aus Buche.
  - Faserneigung 40° gegen die senkrechte Kante, im Text als „400“ / „40�“ (Kodierungsfehler).
- **Punkt 3 (S. 125–126):**
  - Elf Profilrippen quer zur Faser, dazu zwei prismatische Rippen links unten und rechts oben, parallel zur Faser.
  - Variante SB_3 hat zusätzlich eine waagrechte Rippe über dem unteren Ausschnitt.
  - **Keine Lagen, keine Querschnitte in Zahlen.**
- **Punkte 1, 4, 5:** keine Zahlen (Ausschnitt unten nur qualitativ genannt).

### 4. Nakamura (1993): „Acoustical research on the piano, Part 3“ — **[gelesen, Seitenbild S. 430 angesehen]**
- **Angabe:** Nakamura, I. (1993). *Vibrational and acoustic characteristics of soundboard.* J. Acoust. Soc. Jpn. (E) 14(6), 429–439. DOI 10.1250/ast.14.429. Englische Übersetzung von J. Acoust. Soc. Jpn. (J) 37, 510–517 (1981). PDF: https://www.jstage.jst.go.jp/article/ast1980/14/6/14_6_429/_pdf
- **Zugang:** frei. **Lizenz:** nicht gesehen.
- **Punkt 1 (Fig. 1, S. 430, bemaßte Skizze):**
  - (a) „typical soundboard“ eines Uprights der mittleren Preisklasse: 1400 mm × 980 mm, mit Abschneideleisten („trimming rims“).
  - (b) eigens gebauter Rechteckboden: 1384 mm × 965 mm; Maß 30 mm vom unteren Rand (Bezug in der Skizze nicht eindeutig).
- **Punkt 2 (S. 430):** Der Versuchsboden ist 8,5 mm dick, im Diskant etwas dünner.
- **Punkt 3 (S. 430):** Rippen „around 20 mm“ hoch und breit, zum Ende hin dünner. Die Lagen sind in Fig. 1(b) nur gestrichelt, ohne Bemaßung.
- **Punkt 4:** Haupt- und Basssteg sind in Fig. 1 nur skizziert, ohne Koordinaten.
- **Punkt 5:** nicht vorhanden. Fig. 6 (S. 432) zeigt eine vereinfachte Platte mit Symbolen (x₂₁, y₂₁ …), aber ohne Zahlen.
- **Für einen Bauplan:** nur Hüllmaße.

### 5. Elie, Cotté & Boutillon (2022), Acta Acustica 6, 30 — **[gelesen]**
- **Angabe:** DOI 10.1051/aacus/2022024. Verlagsseite 403; gelesen über https://perso.ensta.fr/~cotte/files/Elie_Cotte_Acta_Acustica2022.pdf
- **Lizenz:** CC BY 4.0, laut S. 1 „Open Access article distributed under the terms of the Creative Commons Attribution License“.
- **Inhalt:** Der Referenzboden ist ein **vereinfachter Steinway D (Flügel)**, Tabelle 1 auf S. 7. Er ist dem Projekt schon bekannt (MAESSTRO). Für Upright nicht verwendbar.
- **Software:** MAESSTRO auf GitLab, https://gitlab.com/benjamin.elie/maesstro. Den Baum habe ich über die API durchgesehen: keine Geometrie-Dateien, kein Pleyel- oder Upright-Beispiel, nur Schemata und Audio.

### 6. Elie et al. (2019), ISMA, „MAESSTRO …“ — **[gelesen, Seitenbild S. 3 angesehen]**
- **Quelle:** https://perso.ensta.fr/~cotte/files/Elie_ISMA2019_MAESSTRO.pdf
- **Inhalt:** Fig. 2 zeigt den **Pleyel P131 (Upright)** als Foto mit beschrifteten Rippen und abgeschnittenen Ecken, daneben das vereinfachte Modell. **Ohne Maßstab und Achsen.** Die Bildunterschrift verweist auf Trévisan et al. [9].

### 7. Trévisan, Ege & Laulagnet (2017), JASA 141(2), 690 — **nicht erreicht**
- **Angabe:** *A modal approach to piano soundboard vibroacoustic behavior.* DOI 10.1121/1.4974860. Laut OpenAlex einzige freie Stelle HAL hal-01456083 („submittedVersion“).
- **Zugang:** HAL über Anubis gesperrt; JASA hinter Paywall.
- **Aus Abstract/Suche:** analytisches Modalmodell gerippter, nicht rechteckiger, eingespannter orthotroper Platten, angewandt auf einen Upright-Boden, 0–3000 Hz.
- **Vermutung, nicht belegt:** Das Modell braucht Rippenlagen, Stegverlauf und Umriss als Eingabe; eine Geometrietabelle ist wahrscheinlich.

### 8. Trévisan (2016), Thèse INSA Lyon — **nicht erreicht**
- **Angabe:** *Prédiction des mécanismes vibroacoustiques des plaques orthotropes raidies de formes quelconques : application à la table d'harmonie de piano.* theses.hal.science/tel-01451727. Lizenzfeld laut HAL-API: `https://about.hal.science/hal-authorisation-v1/`.
- **Zugang:** HAL gesperrt (Anubis).
- **Aus API-Abstract:** vereinfachtes analytisches Modell, Ränder über Punktfedern; Kopplung mit Saite und Aufbauten.

### 9. Trévisan, Margerit, Ege & Laulagnet — **nur Abstract**
- **Angaben:**
  - Vienna Talk 2015, S. 188: *Comparisons between measured and predicted vibroacoustics characteristics of an upright piano soundboard.*
  - CFA 2015, hal-01217078: *Caractérisation vibro-acoustique d'une table d'harmonie de piano droit – comparaison mesures / modèle analytique.*
- **Aus dem Abstract [Abstract/Suche]:** Pleyel-P131-Boden auf seiner Zarge; Moden 0–400 Hz; Punktmobilität **entlang des Stegs** bis 5 kHz. Messpunkte am Steg sind also vorhanden, Koordinaten nicht belegt.
- **Volltext:** HAL gesperrt; Vienna-Talk-Seite verlinkt nur das Abstract.

### 10. Chaigne (2010), CFA, „Mesures d'admittances aux chevalets d'un piano droit“ — **nur Abstract**
- **Fundstelle:** hal-00538347, Lizenz `hal-authorisation-v1`.
- **Aus API-Abstract:** Admittanzen an den Stegen eines Pleyel-Bodens, Laser-Vibrometer.
- **Bewertung:** möglicher Träger von Stegpunkten je Taste für den P131 (Punkt 5). Volltext über HAL gesperrt.

### 11. Ege (2009), Thèse — **nicht erreicht**
- **Fundstelle:** *La table d'harmonie du piano – Études modales en basses et moyennes fréquences*, theses.hal.science/tel-00460783.
- **Zugang:** HAL gesperrt. Betrifft den Atlas-Upright der bereits geprüften Ege-Aufsätze, also kaum mehr Stegdaten zu erwarten.

### 12. Li, Tao & Shi (2014), Wood and Fiber Science 46(3) — **Paywall**
- **Angabe:** *Modal Analysis of Upright Piano Soundboards by Combining Finite Element Analysis and Computer-Aided Design.* https://wfs.swst.org/index.php/wfs/article/view/232
- **Zugang:** Das PDF leitet auf die Anmeldung für Abonnenten um.
- **Aus Abstract [Abstract/Suche]:** Studie zu Dicke, Rippenhöhe und Rippenbreite; keine Zahlen im Abstract.

### 13. Tippner & Koňas (2007), Acta Univ. Agric. Silvic. Mendel. Brun. 55(4), 111–118 — **[gelesen, Durchsuchung]**
- **Angabe:** DOI 10.11118/actaun200755040111. „Copyright (c) 2007 The author(s)“.
- **Inhalt:** „rezonanční deska klavíru“, ein Petrof-Instrument, laut Begriff wohl Flügel. Ich fand keine Rippen- oder Steglagen in Zahlen. Für Upright nicht einschlägig.

### 14. He et al. (2024), Materials 17(5), 1004 — **[gelesen]**
- **Angabe:** DOI 10.3390/ma17051004. Lizenz CC BY 4.0.
- **Inhalt:** Upright-Böden im Maßstab 1:2 (rund 698 × 498 mm, 7,37–9,26 mm, Tabelle 1), **ohne Rippen und Stege**. Unbrauchbar für den Bauplan.

### 15. Weitere Kandidaten aus dem Auftrag
- **Berthaut, Ichchou & Jézéquel (2003), Appl. Acoust. 64, 1113–1136:** Paywall, nicht gelesen. Ein Sekundärzitat (Suchergebnis) beschreibt den untersuchten Boden als **Flügel** mit freien Rändern; daher für Upright wohl nicht einschlägig. Die Thèse Berthaut (ECL 2004) fand ich nicht online.
- **Mamou-Mani, Frelat & Besnainou (2008), JASA 123, 2401–2406:** Paywall bzw. ResearchGate, nicht erreicht. Bauart nicht belegt. Die Thèse Mamou-Mani (Paris VI 2007) fand ich nicht online.
- **Zietlow & Moore (2006), JASA 119(3), 1783–1793, DOI 10.1121/1.2164989:** Paywall, nicht gelesen. Nur Abstract: ESPI am eingebauten Boden plus FE-Modell.
- **Giordano & Jiang (2004), EURASIP JASP 2004:7, 926–933:** Springer verlangt ein Cookie bzw. eine Anmeldung; Purdue meldet „not available“. Nicht erreicht.
- **Dérogis (1994, 1995, Thèse 1997), IRCAM, Upright:** HAL gesperrt, Thèse nicht online gefunden.
- **Patent US4531442A (Nippon Gakki 1985), „Sound board assembly for an upright piano“ [gelesen, Google Patents]:** nur „from 7 to 15“ Rippen, keine Maße. Patente US4248124 und US3444771A nur aus der Suche: keine Maße (US3444771A nennt „11 ribs in an upright piano“).
- **ISMA 2023:** Mori & Bork (Modalanalyse Upright) und Raffaj (FEM-Abstimmung) — beide Konferenz-PDFs liefern 404.
- **Bang (2009), UBC-Thesis zum Upright:** von UBC als „unusual activity“ blockiert.
- **Nicht einschlägig, da Flügel (aus der Suche):** BME-Aufsatz „Piano Soundboard Analysis at Radiated Sound“ (pp.bme.hu), Corradi et al. 2017, Mokdad 2013, Chabassier/Duruflé.
- **Offene Datensätze (Zenodo o. ä.) mit Upright-Geometrie:** nicht gefunden.

---

## Rangfolge: Wer kommt einem vollständigen, belegten Bauplan am nächsten?

1. **Trévisan 2016 (Thèse) und Trévisan et al. 2017 (JASA), Pleyel P131. Nicht verifiziert.**
   - Das Modell braucht als Eingabe Umriss, Rippenlagen und Stegverlauf. Zum selben Boden gibt es die gemessenen Moden (Chaigne et al. 2013, schon lokal) und Mobilitäten entlang des Stegs (Trévisan/Margerit 2015, Chaigne 2010).
   - Zusammen mit Chaigne 2010 ist das der wahrscheinlichste Weg auch zu Punkt 5.
   - **Nächster Schritt:** hal-01456083, tel-01451727, hal-01217078 und hal-00538347 **von Hand im Browser** öffnen; HAL lässt automatische Abrufe nicht durch.
2. **Xiao et al. 2026a/b (UH121/UR121). Gelesen, CC BY 4.0, aus Werks-CAD.**
   - Belegt: Punkt 3 (Querschnitte und Längen beim UH121, Querschnitte und 54° beim UR121), Punkt 2 teilweise (8 mm / 7 mm), Punkt 1 nur als Hüllmaß, Punkt 4 nur Querschnitte, Punkt 5 nur die Zahl der Punkte und die Summenkräfte.
   - Es fehlen Rippenlagen, Umrisspolygon, Stegkoordinaten und Stegpunkte. Das Modell ist ausdrücklich nicht öffentlich; abgeleitete Daten gibt es höchstens auf Anfrage beim Autor.
   - Achtung: Die Querschnitte stammen aus zwei verschiedenen Modellen (UH121 und UR121).
3. **Nakamura 1993:** bemaßte Hüllmaße (1400 × 980 mm), Dicke 8,5 mm und Rippen „around 20 mm“. Keine Lagen.
4. **Klímová & Tippner 2014:** Dickenkeil 6,5–9,5 mm, Faser 40°, 11 + 2 Rippen. Keine Lagen.

**Was überall fehlt:** Stegkoordinaten und Stegpunkte je Taste für irgendein Upright. Das habe ich in keiner frei lesbaren Quelle gefunden.

---

## Reichweite der Suche

- **Datenbanken und Dienste:**
  - Websuche: Standard und erweitert.
  - HAL: Such-API api.archives-ouvertes.fr. Treffer und Metadaten ja; Volltexte und Seiten über Anubis gesperrt, auch `/file/`-Pfade.
  - OpenAlex-API: „piano soundboard upright“, 219 Treffer, die ersten rund 50 durchgesehen.
  - Semantic Scholar: 429, nicht erreicht.
  - J-STAGE, SCIRP, PMC, Google Patents, GitLab-API (MAESSTRO), acta.mendelu.cz, woodresearch.sk, ENSTA-Seite (Cotté).
- **Gesperrt oder nicht erreicht:** HAL (Anubis), ResearchGate, Springer (Anmeldung), EDP-Verlagsseite (403), core.ac.uk (403), eprints.soton (403), UBC (Sperre), ioa.org.uk-Proceedings (404), wfs.swst.org (Abonnenten).
- **Suchbegriffe:**
  - Englisch: upright piano soundboard geometry ribs bridge coordinates finite element; Mamou-Mani downbearing; Berthaut 2003; Moore Zietlow interferometric; Giordano Jiang physical modeling; Trévisan modal approach; MAESSTRO github/json; patent upright soundboard ribs spacing; upright string scale bridge coordinates; UH121.
  - Französisch: table d'harmonie piano droit (HAL), Thèse Berthaut, Mamou-Mani, Ege, Dérogis.
  - Tschechisch: rezonanční deska pianina modální analýza.
  - Chinesisch: 立式钢琴 音板 有限元 肋木 码桥.
- **Nicht durchsucht:** CNKI und J-PlatPat (Anmeldung bzw. Formulare), Fachbücher zum Klavierbau (Junghanns, Wolfenden), Piano Technicians Journal.
- **Kein Treffer ist kein Beweis:** Die gesperrten HAL-Volltexte sind die größte Lücke.
