> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_08_waldhorn_tuba_bohrung` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 21:36–21:51, claude-opus-5-5, 80 Runden, success, 3.17 USD; Status `fertig`.
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Quellen unter `…\cmake\quellen\lippe-rohr\`):
> - **Bonet Filella 2014 in der Textebene:** Messverfahren (Schieblehre, 0,8 mm abgezogen, Maßband), Sandbach, Gautrot,
>   „August 2014“ — gefunden.
> - **Mattéoli u. a. (2207.05395v3) Abb. 8:** Bildunterschrift gefunden.
> - **Braden 2006 Tab. 5.1 (S. 72) am Seitenbild:** alle 14 Zeilen stimmen; Summe der Längen 271,914 cm und Becher
>   Ø 21,72 cm nachgerechnet.
> - **OpenWInD, Besson E0925** (selbst über die GitLab-API gelesen, 2026-11-17): 3271 Zeilen, Kopf „unit = m“,
>   „diameter = False“ (also Radius), letzte Punkte 2,06360 / 0,05373 und 2,06570 / 0,05837 — stimmt. Der Lauf hat die
>   Dateien mit `urllib` nur in den Speicher gelesen (`ow_kopf.py`), nicht abgelegt.
> - **Nicht gegengelesen:** Bonet Abb. 17 und Abb. 31–33 (Zeichnungen), Hélie-Stürze, Braden Gl. (2.37) am Bild, die
>   Ladeliste (Norman 2013 u. a.).
> Der Auftrag liegt seit S202 im Archiv der Werkbank (`archiv/auftraege/`, `archiv/ergebnisse/2026-11-16_08_waldhorn_tuba_bohrung/` mit
> `seiten/` und Textauszügen, `archiv/laeufe/`). Wo der Bericht `ergebnisse/…` oder `eingang/…` nennt, ist
> `archiv/ergebnisse/…` bzw. `archiv/eingang/…` gemeint.
> **Abgerufene PDFs** liegen nur unter `…\cmake\quellen\` (der Bericht nennt die Namen der Werkbank):
> - `Bonet_2014_Dissertation.pdf` → `quellen\lippe-rohr\BonetFilella_2014_InputImpedanceHornsCrooks_MSc_Edinburgh.pdf` — MSc-Arbeit
> - `Braden_2006_Bohrungsoptimierung.pdf` → `quellen\lippe-rohr\Braden_2006_BoreOptimisationImpedanceBrass_PhD_Edinburgh.pdf`
> - `Nat_2010_DellISMA2010.pdf` → `quellen\lippe-rohr\Dell_James_Davidson_Wolfe_2010_HandMuteHornImpedance_ISMA2010.pdf` — Name der Werkbank war falsch (Nat_2010)
> **Rechnungen und Ausgaben des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_waldhorn_tuba_bohrung_werkbank/`: `ow_kopf.py`.

# Bericht: Vollständige Bohrungsprofile für Waldhorn und Tuba (Stufe 34m)

- **Auftrag:** `auftraege/2026-11-16_08_waldhorn_tuba_bohrung.md`
- **Datum:** 2026-11-16
- **Modell:** claude-opus-5-5 (Budget 5 USD)
- **Reichweite:** 17 Websuchen (Suchmaschine, US); 9 Abrufe über das Abrufwerkzeug (davon 4 erfolgreich: RCS-Portal, NESS-Seiten, ERA-Datensatz Norman, Edinburgher Thesenliste, ART; 2 PDFs mit Ablage: Braden 2006, Dell 2010); rund 10 `curl`-Abfragen (Kopfzeilen, GitLab-API von OpenWInD); 1 kleines Python-Skript (`ow_kopf.py`, liest Textdateien aus dem OpenWInD-Repo in den Speicher, keine Ablage). Lokal gelesen: Bonet 2014 (`abrufe/Bonet_2014_Dissertation.pdf`), `quellen/lippe-rohr/2207.05395v3.pdf`.
- **Nicht erreicht:** `core.ac.uk` (Cloudflare-Prüfung, 403 — nicht umgangen); `era.ed.ac.uk/bitstreams/…` (liefert per `curl` nur eine 751-Byte-HTML-Hülle; Abrufwerkzeug: Datei > 10 MB); `test.historicbrass.org` (TLS-Fehler bei Abrufwerkzeug und `curl`, Exit 35); `historicbrass.org` (404); Edinburgher Dissertationen Harrison 2018 (18,3 MB) und Logie 2012 (15,7 MB) — zu groß für das Abrufwerkzeug, Laden nicht freigegeben.

**Ergebnis in einem Satz:** Ein vollständiges, als Zahlen lesbares Bohrungsprofil eines **Waldhorns** oder einer **Tuba** wurde **nicht** gefunden; gefunden wurden (a) drei **gemessene Naturhorn-Bohrungen als Zeichnung** samt gemessener Impedanzspitzen (Bonet 2014), (b) der **wahrscheinlich ergiebigste Datensatz** — Normans `AppendixB.zip` zu über hundert vermessenen Hörnern des 18. Jh. (Edinburgh 2013), nicht geöffnet, (c) für die **Gegenprobe** zwei vollständige Punktlisten: eine **tomographisch vermessene Besson-Trompete mit gemessener Impedanz am selben Instrument** (OpenWInD, 3 261 Punkte) und eine **gemessene Posaunen-Stürze** (Hélie et al. 2013, über OpenWInD), dazu eine **vollständige Posaunen-Schablone mit Bessel-Abschnitten** (Braden 2006, Tab. 5.1).

Aufgebaut wird auf dem, was der Auftrag als bekannt nennt (Wachter 2008 Abb. 52, Hendrie 2007, Acta 2022, Myers 2012, Norman/Myers/Campbell Wagnertuben, Soares 2025, Willemsen, Resch, Mattéoli) — nichts davon wurde wiederholt.

---

## 1. Waldhorn

### 1.1 Bonet Filella 2014 — drei gemessene Naturhörner (Zeichnung) und Impedanzspitzen

> Núria Bonet Filella: „Input Impedance of Horns with Crooks: Measurements and Modelling“, Abschlussarbeit (MSc), University of Edinburgh, August 2014, Betreuer Murray Campbell (PDF-S. 1). Quelle: `https://nuriabo.net/wp-content/uploads/2025/11/dissertation.pdf`, lokal `ergebnisse/2026-11-16_08_waldhorn_tuba_bohrung/abrufe/Bonet_2014_Dissertation.pdf`. Das Jahr 2014 ist damit belegt (`[wörtlich]`, Titelseite). Lizenz: auf den gelesenen Seiten kein Vermerk.

**Instrumente** (Tab. 1, S. 10–11, `[wörtlich]`, Auswahl der modellierten): 203 Orchestral Hand Horn, W. Sandbach, London (1810–1830), Master Crook and Couplers; 4092 French Horn, R.J. Ward, Liverpool (early 20th cen.), 3-valve, Terminal; 4671 Orchestral Horn, Gautrot, Paris (ca. 1875), 3-valve, Terminal. Katalognummern der Edinburgh University Collection (EUCHMI).

**Messverfahren** (S. 15, `[wörtlich]`): „The diameter of the tubing was measured with Vernier calipers; 0.8 mm were then deducted from the reading as we assumed the wall thickness to be 0.4 mm. A tape measure was used to determine the axial distance of each reading.“ Die Maße von 203 stammen von Lisa Norman, die des Mundstücks AM 1266 von Arnold Myers (S. 15). Impedanz mit BIAS 0–4096 Hz, für alle Messungen dasselbe Mundstück AM 1266 (S. 11).

**Bohrungen — nur als Zeichnung** (BIAS-Darstellung, Abb. 31 S. 44, Abb. 32 und 33 S. 45; gesehen als Seitenbild):

| Abb. | Instrument, Bogen | Gesamtlänge (abgelesen) | Auffälligkeit | Marke |
|---|---|---|---|---|
| 31 | 203 in G | ≈ 3 250 mm | Bohrung bis ≈ 2 800 mm fast gleichförmig, Becher ab ≈ 3 000 mm | `[eigene Ablesung]` |
| 32 | 4092 in G | ≈ 3 850 mm | Absatz bei ≈ 3 400–3 600 mm (Stufe, dann Becher) | `[eigene Ablesung]` |
| 33 | 4671 in G | ≈ 4 250 mm | breite Stufe ab ≈ 3 000–3 200 mm auf konstanten Wert, Becher ab ≈ 4 100 mm | `[eigene Ablesung]` |

**Ablesbarkeit:** Achsen in mm, waagrecht 0–3 400 / 0–4 000 / 0–4 500 mm, senkrecht ±300 mm, Gitter alle 100 mm. Die Rohrweite (≈ 10–15 mm) verschwindet in dieser Skala — **der enge Teil ist aus der Zeichnung nicht ablesbar**, nur Länge und Becherverlauf grob (± 20 mm). Ob die Hochachse den Radius oder den Durchmesser je Seite zeigt, sagt die Arbeit nicht; ein Becherrand bei ± 250–280 mm wäre als Radius für ein Horn unplausibel groß `[eigene Einschätzung]`. **Die Messtabelle selbst steht nicht in der Arbeit** (Suche nach „mm“, „Table“, „Appendix“: kein Anhang).

**Gemessene Impedanzspitzen am selben Instrument 203** — aber im Bogen **A**, nicht G (Abb. 17, S. 30, `[wörtlich]`, Tabelle unter dem Diagramm):

| Spitze | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| A (ch) [Hz] | 42 | 110 | 165 | 229 | 289.5 | 347.5 | 404.5 | 462 | 515 | 567.5 | 627 | 676.5 |
| A (ci) [Hz] | 39.5 | 108 | 162 | 224.5 | 284 | 341.5 | 397.5 | 455.5 | 506 | 560.5 | 612.5 | 656 |
| A=452 (Soll) | 56.5 | 113 | 169.3 | 226 | 284.7 | 338.6 | 402.7 | 452 | 507.4 | 569.5 | 603.3 | 639.2 |

(„ch“ und „ci“ sind zwei Bogenkombinationen für A; Soll-Zeilen A=440 und A=456 stehen ebenfalls dort.) Höhen der Spitzen und Güte Q: nur als Kurven (Abb. 4, 15, 20, 23, 26), nicht als Zahlen. Weitere Kurven der „EFP“ (äquivalente Grundtonhöhe, Cent) für 1874, 533, 6144, 203, 4092 in Abb. 9–21.

### 1.2 Norman 2013 — Dissertation mit Datenanhang zu über hundert Hörnern

> Lisa Norman: „Integrated approach to the analysis of eighteenth-century horns“, PhD, University of Edinburgh, 2013 (dc.date.issued 2013-11-27), Betreuer Arnold Myers, Murray Campbell. Datensatz: `https://era.ed.ac.uk/handle/1842/8860` — `[Werkzeug]` (Abruf der Datensatzseite).

Angehängt `[Werkzeug]`: **`AppendixB.zip`, 60,59 KB** (`https://era.ed.ac.uk/bitstreams/7bc115d6-d92a-480c-8aa1-41b26243fcd4/download`) und **`Norman2013.pdf`, 238,4 MB** (`https://era.ed.ac.uk/bitstreams/eb192172-09dd-4425-a473-499f75cc5a6d/download`). Die Zusammenfassung nennt „bore profile analysis“ an „a large database of over one hundred instruments“ `[Werkzeug, gekürzter Wortlaut]`. **Was in `AppendixB.zip` steht, ist nicht geprüft** — Größe und Name deuten auf Messtabellen `[eigene Einschätzung]`. Ein Rechte-/Lizenzfeld hat der Datensatz nicht; nur der Seitenfuß „copyright © The University of Edinburgh“ `[Werkzeug]`. Per `curl` kommt statt der Datei eine 751-Byte-HTML-Hülle (die ERA-Oberfläche ist eine Skript-Anwendung) — das Laden braucht wahrscheinlich einen Browser.

Bonet (S. 15) bestätigt, dass Norman das Horn 203 vermessen hat `[wörtlich]` — die Bohrung von 203 dürfte also in Normans Daten stehen, und dazu passen die Impedanzspitzen in 1.1 `[eigene Einschätzung]`.

### 1.3 Weitere Waldhorn-Fundstellen ohne Zahlen

| Quelle | Was | Stand | Marke |
|---|---|---|---|
| Benoit, Chick: „Computer Simulation of the Acoustic Impedance of Modern Orchestral Horns“, ParCo 2005, NIC Series Vol. 33, S. 423–430 (2006) | Modell des offenen B♭-Teils eines **Paxman Modell 40, B♭/F-Althorn**, mit Hand im Becher | nur über `core.ac.uk/download/35010128.pdf` gefunden → 403 | `[nur Zusammenfassung]` |
| Dell, James, Davidson, Wolfe: „The effect of hand and mute on the impedance spectra of the horn“, ISMA 2010 (`https://www.phys.unsw.edu.au/jw/reprints/DellISMA2010.pdf`, Sitzungsordner `webfetch-1794862050301-79wwq0.pdf`) | gemessene Impedanz eines Horns mit drei Handnachbildungen und vier Dämpfern; ohne Hand „impedance peaks above 450 Hz are greatly diminished in magnitude and disappear entirely above 750 Hz“; Wiederholbarkeit „0.1 dB and 0.4 Hz rms“ (S. 1) | gelesen per Textsuche, Instrument nicht identifiziert; **keine Bohrung** — für „Stopfen des Horns“ | `[wörtlich]` (Textebene, nicht am Seitenbild geprüft) |
| Norman: „An Acoustical Approach to the Question of Early Horn Technique“, HBSJ 23 (2011) 67–88 | Bohrprofile mit Schieblehre/Messstab, Hand im/außer Becher | `test.historicbrass.org` TLS-Fehler | `[nur Zusammenfassung]` |
| Widholm, HBSJ 2016 (`…/HBSJ_2016_JL01_010_Widholm.pdf`) | Titel nicht gesehen | nicht abgerufen (gleicher Server) | — |
| Watts, Illinois Phys 498POM 2009 | Impedanz eines Conn 8D mit/ohne künstliche Hand | nicht abgerufen | `[nur Zusammenfassung]` |
| Herstellerangaben Horn-Mundstücke (Schilke 27: Kessel 16,25 mm; 31B: 17,40 mm; 32: 17,83 mm, Seele .221″) | nur Händlerseiten | nicht an Herstellerkatalog geprüft | `[nur Zusammenfassung]` |

### 1.4 Reicht das für eine Bohrung? — Waldhorn

- **Belegt bauen lässt sich nichts Vollständiges.** Bonet gibt Gesamtlängen dreier Naturhörner in G (≈ 3,25 / 3,85 / 4,25 m) und die grobe Lage des Bechers, aber **keine Durchmesser des engen Teils** und keinen Becherrand in sicherem Maßstab.
- **Prüfgröße vorhanden:** gemessene Spitzenfrequenzen 1–12 am Horn 203 (Bogen A) — sobald Normans Daten die Bohrung von 203 liefern, wäre das ein Paar *Profil + Impedanz am selben Instrument* (Bogen A statt G beachten).
- **Annahme bliebe:** Mundrohr, Ventilzüge (für Doppelhorn gar nichts gefunden), Becherkontur, Mundstück (außer Wachter).
- **Nächster Schritt mit Aussicht:** `AppendixB.zip` (Norman 2013) laden und ansehen.

---

## 2. Tuba (und Euphonium)

### 2.1 Euphonium — Profil als Zeichnung (aus Campbell, Gilbert, Myers 2021)

> Mattéoli et al.: „Diversity of ghost notes in tubas, euphoniums and saxhorns“, arXiv 2207.05395v3, lokal `quellen/lippe-rohr/2207.05395v3.pdf`, Abb. 8, S. 10 (am Seitenbild gesehen).

Bildunterschrift `[wörtlich]`: „Top: bore profile as a function of the axial distance to mouthpipe end, taken from [Campbell et al., 2021] (where bore profiles of several other instruments can be found); bottom: modulus of the input impedance as a function of frequency (zoom on the first two peaks). Red: bass trombone; purple: B♭-baritone saxhorn; blue: B♭-euphonium.“

Grobe Ablesung der blauen Kurve (B♭-Euphonium), Achse „Bore diameter (mm)“ 0 – ≈ 65 mm, waagrecht 0–3 000 mm `[eigene Ablesung, ± 3 mm / ± 50 mm]`:

| x [mm] | 0 | 1 000 | 1 500 | 2 000 | ≈ 2 180 |
|---|---|---|---|---|---|
| Durchmesser [mm] | ≈ 12 | ≈ 16 | ≈ 22 | ≈ 44 | Kurve verlässt das Bild (≈ 65) |

Der Becher ist **abgeschnitten**, die Ventilzüge fehlen. Die Kurven unten sind auf 20–140 Hz beschränkt; ob gemessen oder gerechnet, sagt die Unterschrift nicht. **Diese Euphonium-Bohrung ist sehr wahrscheinlich die, die als „Acta 2022: keine Tuba-Bohrung“ schon bekannt ist** — neu ist hier nur der Hinweis auf die Ursprungsquelle:

> Campbell, Gilbert, Myers: *The Science of Brass Instruments*, Springer/ASA Press 2021 (Kapitel z. B. `https://link.springer.com/chapter/10.1007/978-3-030-55686-0_4`) — laut Mattéoli „bore profiles of several other instruments“. Buch, nicht frei; nicht eingesehen.

### 2.2 Tuba — nichts mit Zahlen

| Quelle | Was | Stand | Marke |
|---|---|---|---|
| Myers 2019 „The Typology and Timbre of the Tuba“ — **zweite Fundstelle:** Royal Conservatoire of Scotland, `https://pure.rcs.ac.uk/en/publications/the-typology-and-timbre-of-the-tuba/` | Volltext erschienen in: C. Philipsen (Hg.), *Vom Serpent zur Tuba …*, Bericht über das 33. Musikinstrumentenbau-Symposium Michaelstein 2014, Michaelsteiner Konferenzberichte Bd. 83, Wißner, Augsburg, S. 159–174, ISBN 978-3-95786-204-4 | **kein PDF** an der RCS-Seite; Fußzeile „All rights are reserved, including those for text and data mining …“ | `[Werkzeug]` |
| Myers 1998, PhD Edinburgh (Bohrungsrekonstruktionen von D. Sharp, Fallstudie Wagnertuba) | wahrscheinlich `era.ed.ac.uk/bitstreams/2a943b41-…/download` (> 10 MB) | nicht erreicht | `[nur Zusammenfassung]` |
| Heard 2020, DMA LSU (Mundrohr-Varianten einer Tuba) | `https://repository.lsu.edu/gradschool_dissertations/5205` | nicht abgerufen | `[nur Zusammenfassung]` |
| Tuba-Mundstück: Kessel 32,89 mm, Rand 47,67 mm, Seele 8,2 mm (Händlerliste, Hersteller unklar) | Sweetwater | nicht geprüft | `[nur Zusammenfassung]` |

### 2.3 Reicht das für eine Bohrung? — Tuba / Euphonium

- **Tuba: nein.** Keine Zahl zu Bohrung, Ventilzügen oder Becher. Der einzige frei erreichbare Hinweis bleibt die Mattéoli-Arbeit; Myers 2019 gibt es nur gedruckt (Michaelstein-Band, Wißner) — **Anschaffung des Bandes** oder des Buchs Campbell/Gilbert/Myers 2021 wären die belegten Wege.
- **Euphonium: nur als Gestalt.** Aus Abb. 8 ließe sich der Verlauf bis ≈ 2,2 m nachzeichnen (Digitalisieren mit höherer Auflösung aus dem arXiv-PDF), Becher und Züge blieben Annahme.

---

## 3. Gegenprobe-Instrumente mit vollständiger Punktliste (OpenWInD, Braden)

> **OpenWInD**, Inria (Makutu), `https://gitlab.inria.fr/openwind/openwind`. Lizenz des Repos: Datei `LICENSE` — „GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007“ `[wörtlich]` (Kopf der Datei gelesen). Eine eigene Datenlizenz für die Beispielgeometrien wurde nicht gesehen; sie stehen im selben Repo unter `examples/`. **Kein Horn, keine Tuba, kein Euphonium** unter den Beispielen (Verzeichnisliste über die GitLab-API, 2 Seiten, vollständig durchgesehen).

### 3.1 Trompete Besson E0925 — Tomographie + gemessene Impedanz am selben Instrument

Datei `examples/Besson_simulations/geometries/tomo_molding_combined_0925.txt` (72 087 Bytes, 3 271 Zeilen). Kopf `[wörtlich]`:

```
! version = 0.11.1
! unit = m
! diameter = False
# Internal geometry of the Besson trumpet E0925 from the "Musée de la Musique - Philharmonie de Paris"
# obtained from tomography, performed by Elsa Lambert for C2RMF lab.
```

Erste und letzte Punkte (x, r in m) `[wörtlich]`: `0.00000 0.00952`, `0.00003 0.00950` … `2.06360 0.05373`, `2.06570 0.05837`. Damit **Länge 2,0657 m, Becherradius 58,4 mm** (Durchmesser ≈ 117 mm) `[eigene Rechnung]`. Der Anfangsradius 9,52 mm deutet auf den Kessel des Mundstücks `[eigene Einschätzung]`.

Vereinfachte, akustisch angepasste Fassung `optimized_simplified.txt` (Van Baarsel, aus Tomographie und Messung) `[wörtlich]`:

| x0 [m] | x1 [m] | r0 [m] | r1 [m] | Typ | Parameter |
|---|---|---|---|---|---|
| 0 | 0.0075 | 0.0095 | 0.00335 | Spline | 0.001875 0.00375 0.005625 0.008747 0.007749 0.006223 |
| 0.0075 | 0.0106 | 0.0034 | 0.0025 | Cone | |
| 0.0106 | 0.082 | 0.0025 | 0.0046 | Spline | 0.02895 0.0473 0.06565 0.0026232 0.0034722 0.0036638 |
| 0.082 | 0.32 | 0.0046 | 0.0055 | Cone | |
| 0.32 | 0.7 | 0.0055 | 0.0055 | Cone | |
| 0.7 | 0.8 | 0.0055 | 0.0057 | Cone | |
| 0.8 | 1.4 | 0.0057 | 0.0057 | Cone | |
| 1.4 | 1.7063 | 0.0057 | 0.0072 | Spline | 1.5 1.55 0.00575 0.0059 |
| 1.7063 | 1.988 | 0.0072 | 0.0143 | Bessel | 0.73 |
| 1.988 | 2.085 | 0.0143 | 0.06 | Bessel | 0.73 |

Gemessene Impedanz desselben Instruments: `examples/Besson_simulations/Impedances/MEAS_E0925_impedance_mean_20degC_11-03-22.txt` (dazu `MEAS_FacSimile_…` eines Nachbaus) — **Dateiname gesehen, Inhalt nicht gelesen**. Projektbeschreibung laut `Readme_besson.md` `[wörtlich]`: `https://inria.hal.science/hal-04352706`, CFA 2022 `https://hal.science/hal-03848341`, Vienna Talk 2022 `https://inria.hal.science/hal-03842072`.

Daneben `examples/frequential/Geom_trumpet.txt` (vereinfachte Trompete, 4 Abschnitte, Länge 1,4187 m, Bessel 0,67, Becher r 0,061 m) `[wörtlich]`.

### 3.2 Posaune — gemessene Stürze (Hélie et al. 2013)

Datei `examples/frequential/Geom_trombone_Helie.txt` `[wörtlich]`, Kopf: „Hélie, T., Hézard, T., Mignot, R., & Matignon, D. (2013). One-dimensional acoustic models of horns and comparison with measurements. Acta acustica united with Acustica, 99(6), 960-974.“ Spalten z (m), r (m), 41 Punkte:

| z [m] | r [m] | z [m] | r [m] | z [m] | r [m] |
|---|---|---|---|---|---|
| 0.0000 | 0.0104 | 0.4805 | 0.0305 | 0.5308 | 0.0526 |
| 0.0665 | 0.0104 | 0.4880 | 0.0322 | 0.5344 | 0.0555 |
| 0.1530 | 0.0118 | 0.4924 | 0.0333 | 0.5378 | 0.0589 |
| 0.1984 | 0.0129 | 0.4967 | 0.0348 | 0.5412 | 0.0623 |
| 0.2665 | 0.0150 | 0.5013 | 0.0362 | 0.5439 | 0.0653 |
| 0.3150 | 0.0167 | 0.5054 | 0.0375 | 0.5468 | 0.0687 |
| 0.3643 | 0.0188 | 0.5084 | 0.0387 | 0.5504 | 0.0735 |
| 0.4000 | 0.0208 | 0.5108 | 0.0398 | 0.5530 | 0.0773 |
| 0.4184 | 0.0220 | 0.5142 | 0.0415 | 0.5558 | 0.0814 |
| 0.4318 | 0.0231 | 0.5183 | 0.0437 | 0.5580 | 0.0858 |
| 0.4425 | 0.0243 | 0.5214 | 0.0455 | 0.5608 | 0.0902 |
| 0.4546 | 0.0259 | 0.5240 | 0.0474 | 0.5633 | 0.0955 |
| 0.4637 | 0.0273 | 0.5267 | 0.0494 | 0.5654 | 0.1018 |
| 0.4729 | 0.0289 | | | 0.5680 | 0.1100 |

Das ist **nur das Schallstück** (0,568 m, r 10,4 → 110 mm), nicht die ganze Posaune `[eigene Einschätzung aus den Werten]`. Ob Hélie et al. 2013 auch die gemessene Impedanz dieses Schallstücks zeigen, wurde nicht geprüft (Titel: „comparison with measurements“).

### 3.3 Posaune — vollständige Schablone mit Bessel-Abschnitten (Braden 2006)

> Alistair C. P. Braden: „Bore Optimisation and Impedance Modelling of Brass Musical Instruments“, PhD, University of Edinburgh, 2006. `https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Braden_Alistair__PhDThesis_UniversityOfEdinburgh_2006.pdf` (2 626 415 Bytes, 175 S.), Sitzungsordner `webfetch-1794861843258-w4d64t.pdf`. Copyright-Vermerk nicht gesucht.

Tab. 5.1 „Templated description of a trombone (units/cm)“, S. 72 (PDF-S. 89), **am Seitenbild gelesen** `[wörtlich]`; laut S. 71 „chosen to approximate“ die Bohrung einer real vermessenen Posaune (Hersteller nicht genannt):

| Nr. | Element | Länge [cm] | Radius [cm] | flare γ | Beschreibung |
|---|---|---|---|---|---|
| 1 | LIST | 26.145 | 1.286..0.664 | | Mouthpiece & lead-pipe |
| 2 | JUMP | | 0.664..0.695 | | |
| 3 | CYLR | 139.439 | 0.695 | | Main slide |
| 4 | JUMP | | 0.695..0.725 | | |
| 5 | CYLR | 26.910 | 0.725 | | Neck-pipe |
| 6 | JUMP | | 0.725..0.755 | | |
| 7 | CONE | 21.990 | 0.755..0.990 | | Tuning-slide |
| 8 | CYLR | 6.732 | 0.990 | | Sleeve |
| 9 | JUMP | | 0.990..1.053 | | |
| 10 | CONE | 5.336 | 1.053..1.105 | | Bell |
| 11 | BESS | 15.042 | 1.105..1.507 | 1.260 | |
| 12 | BESS | 15.042 | 1.507..2.228 | 0.894 | |
| 13 | BESS | 9.685 | 2.228..4.118 | 0.494 | |
| 14 | BESS | 5.593 | 4.118..10.860 | 1.110 | |

Summe der Längen **271,914 cm** `[eigene Rechnung]` — passt zu Abb. 5.3 (Achse bis ≈ 272 cm, am Seitenbild gesehen). Bessel-Form laut Braden Gl. (2.37) „R(x) = b(−x)^−γ“ (Textebene, nicht am Bild geprüft). Die Liste für Element 1 (Mundstück + Mundrohr) ist **nicht** abgedruckt, nur Anfangs- und Endradius. Becher-Durchmesser 21,72 cm `[eigene Rechnung]`. Braden betont (S. 71): „the flares of the shorter Bessel-horn sections differ somewhat from the flare of the closest equivalent single Bessel horn with γ = 0.7“ `[wörtlich]`.

### 3.4 Reicht das? — Trompete, Posaune

- **Trompete:** ja, für eine **Gegenprobe des Kerns**: vollständige gemessene Bohrung (Tomographie) **und** gemessene Impedanz desselben Instruments (Datei vorhanden, nicht gelesen). Ventile: in der Tomographie ist nur der offene Weg enthalten `[eigene Einschätzung, nicht geprüft]`.
- **Posaune, Profil des Schallstücks (offen.md):** zwei belegte Wege — Hélie 2013 (41 gemessene Punkte) und Braden Tab. 5.1 (vier Bessel-Abschnitte mit eigenem γ). Beide ersetzen das heutige „Bessel-Horn ε = 0,7“. Becherradius: Hélie 110 mm, Braden 108,6 mm; Willemsen (schon bekannt) nennt laut modell.md 10,8 cm — Braden passt dazu.

---

## 4. Offene Datensätze (Punkt 5)

| Datensatz | Inhalt Blech | Lizenz im Wortlaut | Marke |
|---|---|---|---|
| OpenWInD `examples/` (gitlab.inria.fr) | Besson-Trompete (Tomographie, vereinfacht, gemessene Impedanz), Posaunen-Stürze Hélie, vereinfachte Trompete; **kein Horn/Tuba/Euphonium** | „GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007“ (Datei `LICENSE`) | `[wörtlich]` |
| Norman 2013 `AppendixB.zip` (ERA Edinburgh) | vermutlich Bohrungen von Hörnern des 18. Jh. | kein Lizenzfeld; Fuß „copyright © The University of Edinburgh“ | `[Werkzeug]` |
| ART (artool.sourceforge.net) | keine Beispielgeometrien auf Start- und About-Seite | GPL „either version 3 of the License, or any later version“ | `[Werkzeug]` |
| NESS Brass (Edinburgh) `BrassTutorialFiles.zip` | Beispielinstrumente als `.m`; genannt nur Trompete/Posaune | „Unless explicitly stated otherwise, all material is copyright © The University of Edinburgh 2016.“ | `[Werkzeug]` |

---

## 5. Nicht gefunden — und wo gesucht wurde

| Lücke | Suchwege |
|---|---|
| Vollständige Bohrung eines (Doppel-)Waldhorns, Ventilzüge, Umschaltventil | Websuche EN/DE (Bohrungsprofil, Widholm/Kausel/Wiener Horn, Pyle, Benade, Braden, Patente, Herstellerkatalog Yamaha); Bonet 2014 (nur Naturhorn, Zeichnung); Braden 2006 (Textsuche „horn“: nur Hornfunktion/Bessel-Theorie, kein Waldhorn); OpenWInD; NESS; ART. Wiener Suche lieferte nur Geologie zu Horn (NÖ). |
| Tuba-Bohrung | Websuche (Tuba bore profile, HAL „perce tuba“, DMA-Dissertationen, Pulsreflektometrie); RCS-Fassung von Myers 2019 (kein PDF); Mattéoli 2022 (nur Euphonium, aus Campbell 2021). |
| Mundstückprofil Horn (außer Wachter) / Tuba | Händlerangaben gefunden, kein Herstellerkatalog gelesen; Bonets Mundstück AM 1266 ohne Maße. |
| Impedanz am selben Horn wie ein Profil | nur Bonet 203 (Bogen A) — Profil dazu nur bei Norman zu erwarten. |
| Benoit & Chick (Paxman-Horn) | nur auf core.ac.uk (403); Jülicher NIC-Ablage nicht gefunden. |

---

## 6. Ladeliste (nichts geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Norman_2013_AppendixB.zip` | https://era.ed.ac.uk/bitstreams/7bc115d6-d92a-480c-8aa1-41b26243fcd4/download | 60,59 KB | kein Lizenzfeld; © Univ. Edinburgh (Seitenfuß) | **erste Wahl:** wahrscheinlich Bohrungstabellen von Naturhörnern (u. a. 203?) |
| `Norman_2013_horns_thesis.pdf` | https://era.ed.ac.uk/bitstreams/eb192172-09dd-4425-a473-499f75cc5a6d/download | 238,4 MB | wie oben | Erläuterung zu Anhang B, Profile |
| `OpenWInD_Besson_E0925_tomo.txt` | https://gitlab.inria.fr/openwind/openwind/-/raw/master/examples/Besson_simulations/geometries/tomo_molding_combined_0925.txt | 72 KB | GPL-3.0 (Repo) | Trompete Gegenprobe, Bohrung |
| `OpenWInD_Besson_E0925_Z_mess.txt` | https://gitlab.inria.fr/openwind/openwind/-/raw/master/examples/Besson_simulations/Impedances/MEAS_E0925_impedance_mean_20degC_11-03-22.txt | nicht abgefragt | GPL-3.0 (Repo) | gemessene Impedanz desselben Instruments |
| `OpenWInD_Helie_2013_Posaune_Stuerze.txt` | https://gitlab.inria.fr/openwind/openwind/-/raw/master/examples/frequential/Geom_trombone_Helie.txt | < 1 KB | GPL-3.0 (Repo) | Posaunen-Schallstück (oben vollständig wiedergegeben) |
| `Braden_2006_Bohrungsoptimierung.pdf` | https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Braden_Alistair__PhDThesis_UniversityOfEdinburgh_2006.pdf | 2,6 MB | nicht geprüft | Tab. 5.1 Posaune (liegt als `webfetch-1794861843258-w4d64t.pdf` vor) |
| `Harrison_2018_FDTD_Blech.pdf` | https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Harrison_Reginald__PhDThesis_UniversityOfEdinburgh_2018.pdf | 18,3 MB | nicht geprüft | Kandidat: Instrumentbohrungen für FDTD (Inhalt unbekannt) |
| `Logie_2012_Spieleigenschaften_Blech.pdf` | https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Logie_Shona__PhDThesis_UniversityOfEdinburgh_2012.pdf | 15,7 MB | nicht geprüft | Kandidat: B-Werte/Bohrungen mehrerer Instrumente (Inhalt unbekannt) |
| `Sharp_1996_Pulsreflektometrie.pdf` | https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Sharp_David__PhDThesis_UniversityOfEdinburgh_1996.pdf | 2,4 MB | nicht geprüft | Kandidat: rekonstruierte Bohrungen (Inhalt unbekannt) |
| — (Kauf) | Philipsen (Hg.), Michaelsteiner Konferenzberichte 83, Wißner, ISBN 978-3-95786-204-4, S. 159–174 | Buch | © Verlag | Myers 2019, Tuba-Bohrungen |
| — (Kauf) | Campbell, Gilbert, Myers: *The Science of Brass Instruments*, Springer 2021 | Buch | © Verlag | Bohrprofile mehrerer Instrumente (laut Mattéoli) |

---

## 7. Was nicht geliefert ist, was die LumiViz-Sitzung prüfen muss

**Nicht geliefert:** Bohrung als Zahlen für Waldhorn, Tuba; Ventilzüge; Becherkontur Horn/Tuba; Mundstückprofile Horn (außer Wachter) und Tuba; Güte Q und Spitzenhöhen als Zahlen.

**Selbst nachprüfen:**
1. `AppendixB.zip` (Norman) laden — ob Bohrungen drinstehen und ob Horn 203 dabei ist.
2. Die Bonet-Ablesungen (Gesamtlängen) sind grob; vor einer Verwendung an den Abbildungen nachmessen.
3. OpenWInD-Daten: ob die Beispielgeometrien wirklich unter GPL-3 stehen oder eigene Rechte der Philharmonie/C2RMF tragen (HAL-04352706 ansehen).
4. Braden Tab. 5.1: Konvention der Bessel-Abschnitte (Gl. 2.37, Vorzeichen, Bezugspunkt je Abschnitt) am Seitenbild S. 17 prüfen, bevor sie in den Kern gehen — hier nur über die Textebene gesehen.
5. Hélie 2013 im Original: ob das Profil ein Posaunen-Schallstück allein ist und welche Posaune.

Hilfsdateien dieses Laufs: `ow_kopf.py` (Leseskript), `seiten/` (Seitenbilder Mattéoli S. 10, Braden S. 88–89).
