# Recherche S146: Resonanzboden eines Flügels — Geometrie und Holz

> **Datum:** 2026-10-23 (Session 146) · **Auftrag:** Geometrie und Material eines Flügel-Resonanzbodens mit
> Zahlen (Umriss, Dicke, Rippen, Stege, Wölbung, Downbearing, Rand; Holzwerte Fichte/Buche/Ahorn), bevorzugt
> Steinway D, Yamaha, Bösendorfer, Pleyel, Bechstein.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [wörtlich] = im Volltext gesehen, [nur Zusammenfassung] = Abstract, Suchergebnis oder Sekundärquelle.
> **Von Claude selbst nachgelesen:** Steinway D Platte 9 mm → 6 mm (auch in RT-0425 §2.1). Alles andere: nur Bericht.
> **Quellen-PDFs:** `../quellen/` (nicht versioniert); die im Bericht genannten Pfade ins Scratchpad gibt es nicht mehr.
> **Eingeflossen in:** `docs/audio/Spielweisen_Konzept.md` §4.1, „Befund Recherche Boden und Körper“.

---

**Kurz:** Es gibt frei lesbare Quellen mit Zahlen zur Dicke, zum Dickenverlauf, zum Rippenabstand, zu Steg-Querschnitten und zu Holzwerten. Für einen Konzertflügel fehlen aber nach wie vor Rippenzahl, Rippenquerschnitt, Steg-Lage und gemessene Wölbung bzw. Downbearing. Ein vollständiger Datensatz ist nur für den Pleyel P131 belegt, und der ist ein Klavier, kein Flügel.

## 1. Funde

**A. Boutillon, Ege & Paulello: „Comparison of the vibroacoustical characteristics of different pianos“** (arXiv:1210.3948, 2012), https://arxiv.org/pdf/1210.3948
- [wörtlich] Fußnote 1: „In the Steinway B and D, the thickness of the wood panel varies between 9 mm in the centre to 6 mm at the rim.“
- [wörtlich] Tabelle 1 gibt den mittleren Rippenabstand p und die Grenzfrequenz fg zwischen tiefem und hohem Bereich an. Die Spalten sind im Textauszug verschoben. Am wahrscheinlichsten ist diese Zuordnung: Steinway B p = 12,2 cm / fg = 1477 Hz; Steinway D p = 12,7 cm / fg = 1355 Hz. Die Zuordnung ist von mir erschlossen: Atlas = 13 cm passt zur anderen Arbeit.
- [wörtlich] Holzwerte: Norway spruce ρ = 440 kg/m³, Ex = 15,8 GPa, Ey = 0,85 GPa. „average spruce“ ρ = 380, Ex = 11,5, Ey = 0,74. „excellent spruce“ ρ = 350, Ex = 12,6, Ey = 1,13.
- Dazu die Annahme, die Rippen seien aus demselben Holz (Er = Ex).

**B. Boutillon & Ege: „Vibroacoustics of the piano soundboard: Reduced models, mobility synthesis, and acoustical radiation regime“** (arXiv:1305.3057, 2013, später JSV)
- [wörtlich] S. 2: Die Breite des Bodens beträgt „more or less 140 cm“. Die Länge reicht bis „more than 2 m for exceptionally large concert grands“. Die Plattendicke ist „w ≈ 8 ± 2 mm“, der Rippenabstand „p ranges from 10 to 18 cm“.
- [wörtlich] Die Stege sind aus Ahorn („Two maple bars – the bridges – thicker than the ribs“). Rippen aus Fichte, „sometimes sugar pine“, dazu Abstreben („cut-off bars“) aus Tanne.
- [wörtlich] Tabelle 1, Werte für EL / ER / GLR in GPa, νLR, ρ in kg/m³:

| Holz | EL | ER | GLR | νLR | ρ |
|---|---|---|---|---|---|
| Sitka | 11,5 | 0,47 | 0,5 | 0,3 | 392 |
| Mediocre | 8,8 | 0,35 | 0,4 | 0,3 | 400 |
| Norway | 15,8 | 0,85 | 0,84 | 0,3 | 440 |
| Fir | 8,86 | 0,54 | 1,6 | 0,3 | 691 |
| Maple | 10 | 2,2 | 2,0 | 0,3 | 660 |

  Quellen der Tabelle laut Text: Berthaut, Paulello, Haines.
- [wörtlich] Verlustfaktor gemessen an einem Klavier (Atlas): „≈ 2% ± 1% over several kHz“.
- [wörtlich] S. 26 zu Conklins Versuchsboden: 39 Rippen, „more than twice the usual number“, p etwa 5–6 cm, Breite „around 1.1 cm, approximately half of the usual value“. Daraus folgt eine übliche Rippenbreite von etwa 2,2 cm; das ist erschlossen.

**C. Ege & Boutillon** (arXiv:1212.3068, ICA 2010)
- [wörtlich] Tabelle 1 nach Berthaut 2004: Fichte 11,5 / 0,47 / 0,5 / **0,005** / 392.
- Die Spalte mit 0,005 trägt die Überschrift νLR. In B steht für dieselbe Fichte 0,3. Das ist ein Widerspruch zwischen beiden Arbeiten. Ein Verlustfaktor ist es laut Tabellenkopf nicht.

**D. Chaigne, Cotte & Viggiano: „Dynamical properties of piano soundboards“** (JASA 133 (4), 2013, S. 2456–2466), PDF auf perso.ensta.fr/~cotte/files/
- [wörtlich] Tabelle I (S. 2457), 3D-orthotrop, ρ in kg/m³, Moduln in GPa:

| Holz | ρ | EL | ER | ET | GLR | GLT | GRT | νLR | νLT | νRT |
|---|---|---|---|---|---|---|---|---|---|---|
| Picea excelsa | 440 | 15,9 | 0,69 | 0,39 | 0,62 | 0,77 | 0,0036 | 0,44 | 0,38 | 0,47 |
| Buche (Diskantsteg) | 674 | 14 | 2,28 | 1,16 | 1,64 | 1,08 | 0,47 | 0,449 | 0,518 | 0,707 |

  Quellen laut Text: Hearmon 1948, Bucur 2006.
- [wörtlich] Grundmodell: Platte 1,41 × 1,01 m², 9 mm dick, 10 Rippen mit 2,1 × 2,5 cm², Faserwinkel 131°.
- [wörtlich] Diskantsteg: „height 39 mm and width 28 mm“, ähnlich Pleyel P131.
- [wörtlich] P131-Modell: schwingende Fläche 1,365 × 0,96 m, Rippen an den Enden verjüngt, zwei Streben mit 2 × 3,5 cm. Mittlerer Rippenabstand 13,2 cm (σ = 0,97 cm, Tabelle III).
- [wörtlich] Modaler Verlustfaktor 1,5 %, durch Abgleich gewählt.
- Achtung: Das ist ein Klavier, kein Flügel.

**E. Corradi, Miccoli, Squicciarini & Fazioli: „Modal analysis of a grand piano soundboard at successive manufacturing stages“** (Applied Acoustics 2017). Preprint: https://miccoli.faculty.polimi.it/preprints/soundboard-20160727.pdf (17,7 MB; gelesen, danach gelöscht). Untersucht wird ein Fazioli F278.
- [wörtlich] Platte aus Norway spruce, mit CNC auf veränderliche Dicke gefräst („diaphragmed“). Rippen aus Sitka, Stege aus Ahorn.
- [wörtlich] Die Wölbung entsteht durch „compression crowning“: gerade Rippen werden in einer gekrümmten Form verleimt. Danach werden die Rippenenden abgehobelt.
- [wörtlich] Am Rand wird der Boden auf den Innenrast geleimt; der Rast gilt in erster Näherung als starr.
- [wörtlich] Tabelle 1 nach Abgleich. Platte: E1 = 14,6, E2 = 0,686, E3 = 0,436, G12 = 0,677, G13 = 0,644, G23 = 0,058 GPa; ν12 = 0,36, ν13 = 0,45, ν23 = 0,48; ρ = 452 kg/m³. Rippen: E1 = 12,7, E2 = 1,04, E3 = 0,478, G12 = 0,998, G13 = 0,959, G23 = 0,042 GPa; ν = 0,37 / 0,47 / 0,43; ρ = 489 kg/m³.
- Ahorn-Steg: E1 = 12,6, E2 = 1,66, E3 = 0,819 GPa. Die übrigen Werte dieser Spalte sind im Textauszug durcheinander und lassen sich nicht sicher zuordnen.
- [wörtlich] Dämpfungsgrad: 0,4–1,0 % am freien Boden, 0,8–4,0 % eingebaut. Über ~450 Hz wurden 2 % angesetzt.
- **Keine Zahlen** zu Dicke, Rippenzahl, Querschnitten, Wölbung oder Downbearing; die Geometrie stammt aus Faziolis CAD.

**F. Giordano: „Simple model of a piano soundboard“** (JASA 102, 1997, S. 1159). Preprint als PostScript: physics.purdue.edu/piano/articles/soundboard.ps
- [wörtlich] Quadrat von 1 m Kantenlänge, Dicke 1 cm, ρ = 0,5 g/cm³.
- [wörtlich] Fig. 3: Ex = 1·10¹⁰, Ey = 4·10⁸, G = 1·10⁹ N/m², νx = 0,4, νy = 0,01.
- [wörtlich] 12 gleichmäßig verteilte Rippen, „rib thickness of 1 cm“, E_rib = 2·10¹⁰ N/m², Breite etwa 1–2 cm. Die elastischen Konstanten stammen laut Text von Kindel.
- Bewusst grobes Modell, kein bestimmter Flügel.

**G. Steinway Gallery Singapore** (Händlerseite), https://www.steinway-gallery.com.sg/?p=685
- [wörtlich] „Steinway Model D soundboard is 9 mm thick in the center and tapers to 6 mm as it approaches the rim … before being double crowned.“
- [wörtlich] Rippen „Made from durable, resinous sugar pine“, Stege aus vertikal laminiertem Ahorn. Der Bassteg ist „continuous with treble“.
- [wörtlich] Gesamtzug der Saiten 20 418 kg; Länge 274 cm, Breite 156 cm.
- Herstellerangabe, keine Fachquelle.
- Widerspruch zu B: B nennt Fichte als Rippenholz, Steinway Zuckerkiefer.

**H. Nur aus Zusammenfassungen** [nur Zusammenfassung]
- Kugelschalen-Wölbung mit R = 43 m, etwa 8 mm Stich in der Mitte. Unter Saitenlast halbiert sich die Wölbung (Mamou-Mani et al., JASA 123, 2008, laut Suchergebnis).
- Rippenhöhe 15–25 mm, zum Diskant flacher. Diskantsteg 3–4 cm hoch und 2,5–3,5 cm breit, Bassteg 2–3 cm höher (laut Suchergebnis aus Ege/Boutillon 2013).
- Berthaut et al. 2003: Dämpfungsgrad 0,003–0,0065.
- Wood Handbook 2010 für Sitka: ET/EL = 0,043, ER/EL = 0,078, νLR = 0,372 (über Sekundärseiten, nicht nachgeprüft).

## 2. Zugang

- Frei lesbar sind A, B, C (arXiv), D (Autorenseite), E (Preprint), F (Purdue).
- Hinter Bezahlschranke bzw. nicht gesehen: Conklin 1996, Mamou-Mani 2008, Suzuki 1986, Kindel & Wang, Berthaut 2003.
- Die Seiten von HAL und eprints.soton blockierten den Abruf (403 bzw. Anubis).
- Hinweis: Den Aufbau des Flügels behandelt Conklin in **Part II** („Piano structure“, JASA 100, 695–708). Part I ist nicht die gesuchte Quelle; so zitieren es B und D.

## 3. Nicht gefunden

- Für Steinway D, Yamaha C3/C5/C7/CFX, Bösendorfer und Bechstein: keine Rippenzahl, kein Rippenquerschnitt, keine Steg-Lage und keine Umrisskoordinaten.
- Für keinen Flügel einen gemessenen Downbearing in N oder Grad.
- Keine Volltexte von Conklin, Suzuki, Kindel & Wang, Wogram, Moore & Zietlow oder Fletcher & Rossing.
- Keine Dissertationen von Ege, Berthaut, Trévisan oder Mamou-Mani im Volltext.
- Keinen Verlustfaktor des Holzes selbst je Richtung: Gefunden sind nur Dämpfungswerte des ganzen Bodens.
- Den Wood-Handbook-Volltext konnte ich nicht laden.
- Gesucht wurde über rund zwölf Websuchen; gelesen habe ich nur die genannten Volltexte.
