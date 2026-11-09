# Körper — welche Klassen welchen Körper brauchen, offene Daten, Übertragbarkeit der Platte des Flügels

> **Klasse 0 (Gemeinsam).** **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorab-Ausarbeitung, nichts entschieden.
> **Grundlage:** LumiViz (nur gelesen): Entscheid KB1 a (`Spielweisen_Konzept.md` §4.1), Bibliothek `InstrumentBody`
> (`projects/libs/InstrumentBody`, Doku `InstrumentBody.md` 0.14.0), Bauplan `asset/loop/instrument_bodies/
> steinway_d.lvinstbodyplan`; Berichte `recherche/2026-10-26_koerper_abstrahlung.md` (Agent) und in LumiViz
> `docs/modellierung/koerper/recherche/` (S146).
> **Kennzeichnung je Angabe:** **nachgelesen** (von dieser Sitzung im Volltext, Feld oder Code selbst gelesen, mit Ort) ·
> **nur Bericht** (aus einem Agentenbericht, nicht selbst geprüft) · **Einschätzung** (Claude, kein Beleg).
> Offene Entscheidungen: [offene-fragen.md](../../gemeinsam/ic/offene-fragen.md).

## 1. Was LumiViz heute unter „Körper“ rechnet

- **Form (KB1 a, nachgelesen, Konzept §4.1):** eine Liste von Moden — je Mode Frequenz und Dämpfung, je Einkoppelpunkt
  eine modale Masse, je Ausgang ein Abstrahlgewicht; darüber ein statistischer Teil (Modendichte, mittlere Mobilität,
  Dämpfung), mit festem Samen ausgewürfelt. Offline zu Antworten gerechnet, per Faltung angewandt. **Erweiterungen**
  tragen, was ein Instrument besonders macht — im Entscheid genannt: Flügel (Stegpunkte, Deckel), Gitarre (Luftraum,
  zwei Richtungen am Steg), Trommel (Luftlast der Membran).
- **Rechnung (nachgelesen, `InstrumentBody.md` §2–§4, `Platte.hpp`):** **ebene** Platte nach Reissner–Mindlin,
  orthotrop, Dicke je Punkt; Rippen und Stege als dickere Stellen mit eigenem Material; Netz aus Dreiecken (CDT),
  Lagrange-Elemente k = 3; Auflager **eingespannt** (Boden), gelenkig oder frei nur für Prüfplatten; Eigenlöser
  (Eigen/Spectra hinter eigener Schnittstelle); Abstrahlung je Mode zu Hörorten (Rayleigh, Platte in Schallwand);
  statistischer Teil nach Boutillon & Ege 2013.
- **Größe des Steinway D (nachgelesen, Bauplan und `InstrumentBody.md`):** Fläche 2,05 m² (aus dem Umriss des Bauplans
  gerechnet, 163 Punkte), Platte 6 mm mit Dickenraster, Fichte ρ = 380 kg/m³, E_x = 11 GPa, E_y = 0,65 GPa; Netz bis
  5 kHz 568 515 Größen; **820 Moden bis 5 kHz**, 13 Zerlegungen, 248 s.
- **Bestand Gitarre (IC-B1 a; nachgelesen, `Saiten_Konzept.md` S-K8 a, S-K10 a, §5e–§5f):** neben `InstrumentBody`
  gibt es in LumiViz schon zwei Gitarrenkorpusse als **gekoppelte Schwinger Decke und Luft**, getrieben von der Kraft am
  Steg, abgestrahlt als Änderung des Volumenflusses — *Parlor* nach Christensen/Vistisen in Wrights Schreibweise
  (Gl. A.1–A.7) mit den Werten aus Hess (2013) Tab. 1, gekoppelte Spitzen 116 und 219 Hz wie gemessen; *Classical*
  nach Wrights Satz TOP26 (Gl. A.20–A.24, zehn Deckenmoden aus Tab. 7.1 und B(1,1), Luft 133 Hz). KB1 a nennt sie als
  Vorläufer der gemeinsamen Grundlage.
- **Nicht vorgesehen** in `InstrumentBody` (nachgelesen, aus dem Fehlen in `Bauplan.hpp`/`Platte.hpp`): Wölbung (Schale), Luftraum,
  zweite Platte (Boden), Zargen, Stimmstock, Löcher in der Platte, Membran (Spannung statt Biegesteife), Abstrahlung
  ohne Schallwand.

## 2. Welche Klasse braucht welchen Körper

Spalte „Erweiterung“: was nach KB1 a auf die Basis käme. Die Physik der Spalte „Körper“ ist, wo nicht anders vermerkt,
**nur Bericht** (Agent, überwiegend aus Fachwissen ohne Beleg in dieser Sitzung, `recherche/…koerper_abstrahlung.md` §1).

| Klasse | Instrument | Körper | Erweiterung nach KB1 a | Woher die Moden kommen könnten |
|---|---|---|---|---|
| 1 Hammer-Saite (Rest) | Upright | Boden mit Rippen wie Flügel, kleiner, senkrecht | keine neue — Bauplan eines Upright | gerechnet wie Steinway; Geometrie offen |
| 1 | Hackbrett, Cimbalom | Kasten: Decke, Boden, Luft, Schalllöcher | Luftraum, zweite Platte | offen (nichts gesucht) |
| 1 | CP-70, Clavinet | Tonabnehmer statt Körper (Clavinet in LumiViz schon so) | — | — |
| 2 Gezupfte Saite | Gitarre Nylon/Stahl | Decke mit Beleistung, Luftraum mit Schallloch (A0), Boden, Zargen | Luftraum (Helmholtz), Boden, zwei Richtungen am Steg, Abstrahlung ohne Schallwand | **heute in LumiViz:** Parlor und Classical als Decke + Luft (§1, S-K8 a, S-K10 a); **gemessen**: Mores, 65 Gitarren (§4); gerechnet: Platte mit Leisten passt (§3) |
| 2 | Harfe | Boden mit Saitenlinie, Kasten mit Löchern | Luftraum, Löcher | gerechnet (eben) — keine offenen Daten gefunden |
| 2 | Cembalo | Boden mit Rippen und Steg, Kasten | Luftraum (schwach?) | gerechnet wie Steinway — dem Flügel am nächsten |
| 2 | Koto, Laute/Oud | gewölbter Kasten bzw. flache Decke mit Querbalken und Muschelboden | Luftraum, Wölbung (Koto, Boden der Laute) | offen (keine Arbeit gefunden) |
| 3 Bogen-Saite | Violine, Viola, Cello, Kontrabass | gewölbte Decke und Boden, Bassbalken, Stimmstock, f-Löcher, Luftraum | **Schale** statt Platte, Stimmstock, Löcher, Luftraum; beim Kontrabass der Stachel | **gemessen**: CNSM (3 Violinen, §4); gerechnet erst nach großem Ausbau (§3) |
| 4 Platte und Stab | Marimba, Xylophon, Vibraphon | Rohrresonator unter dem Stab | kein Plattenkörper — **ein Rohr** (siehe [rohr.md](../../rohr/ic/rohr.md)) mit Kugelquelle am offenen Ende | gerechnet (Rohr) |
| 4 | Kalimba | Kasten mit Luft und Löchern | Luftraum, Löcher (Abdecken = „Wah“) | offen |
| 4 | Celesta | Holzkästen je Ton | wie Kalimba oder Rohr | nicht gesucht |
| 4 | Glockenspiel, Röhrenglocken, Steeldrum | kaum Körper; Steeldrum: Schale mit Tonfeldern und Mantel | Steeldrum: der Schwinger *ist* die Schale (Klasse 4 selbst) | — |
| 4 | Rhodes, Wurlitzer | Tonabnehmer, Verstärker | — | — |
| 5 Membran | Pauke | Membran über starrem Kessel, Luft innen und außen | **Luftlast** — bestimmt die Tonhöhen (nur Bericht: Christian et al. 1984) | gerechnet; keine offenen Daten |
| 5 | Toms, Bassdrum, Snare | zwei Felle, Kessel, eingeschlossene Luft | Kopplung der Felle **über die Luft** (nur Bericht: Zhao 1990) | gerechnet |
| 5 | Conga, Bongo, Tabla | ein Fell, offener bzw. geschlossener Körper | Luftraum | offen |
| 6 Becken, Gong | — | der Schwinger strahlt selbst | keine | — |
| 7/8/9 Rohr | Blech, Holz, Flöten, Orgel | Rohr mit Schallstück/Tonlöchern | Abstrahlung am Rohrende — in [rohr.md](../../rohr/ic/rohr.md) | gerechnet aus der Bohrung |
| 10 Zunge | Akkordeon, Harmonium | Kanzellen, Gehäuse, Balg | offen (nicht gesucht) | — |
| 11 Tonrad | Hammond | kein Körper; Leslie (Lautsprecher, Rotor, Doppler) | eigener Baustein, kein Körper nach KB1 a | — |

**Einschätzung:** KB1 a trägt alle Klassen außer 6 und 11; die Erweiterungen fallen in vier Gruppen — **Luftraum**
(Gitarre, Harfe, Violine, Kalimba, Trommeln), **zweite Platte/Schale** (Gitarre, Violine), **Luftlast einer Membran**
(5), **Rohr als Resonator** (Stabspiele, gemeinsam mit [rohr.md](../../rohr/ic/rohr.md)). Eine gemeinsame Erweiterung „Luftraum“
(Helmholtz-Mode und Kopplung an die Platte) diente drei Klassen zugleich.

## 3. Übertragbarkeit der Platte des Flügels

| Ziel | Was passt | Was fehlt | Belegkraft |
|---|---|---|---|
| **Cembalo, Upright, Harfe** | ebener Boden mit Rippen/Steg — dieselbe Rechnung | Luftraum (Kasten); Harfe: Löcher | Einschätzung; Harfe/Cembalo nur Bericht |
| **Gitarrendecke** | ebene orthotrope Platte mit Leisten — Leisten wie Rippen | Luftraum (A0) und Boden; Abstrahlung ohne Schallwand; Auflager an den Zargen nicht starr | nur Bericht: Christensen & Vistisen 1980 (zwei Freiheitsgrade Decke–Luft), Elejabarrieta et al. 2001/2002 (FEM, Kasten mit Luft), Richardson/Walker (Schallwand nur „superficial“) |
| **Violine** | — | gewölbte Schale (Schalenelemente), Stimmstock, Bassbalken, f-Löcher, Luftraum, Boden | nur Bericht: Gough (Schalenmodell, nicht erreichbar), Woodhouse 2014 (Übersicht) |
| **Membran (Pauke)** | Netz, Eigenlöser, Rayleigh-Ausgang | Steife aus **Spannung** (nicht Biegung), **Luftlast** innen/außen, Kessel | nur Bericht: Christian et al. 1984; Bilbao & Webb 2013 (FDTD mit 3D-Luft, teuer) |

**Abschätzung der Netzgröße (Einschätzung, Skript unten):** Mit der Kante nach MAESSTRO Gl. 1 wie in `InstrumentBody`
wächst die Zahl der Größen wie Fläche · f / (Dicke · √(E/ρ)), die Modenzahl einer Platte wie Fläche / Dicke. Gegen den
Steinway D (2,05 m², 6 mm; 568 515 Größen, 820 Moden bis 5 kHz):

| Fall (Annahmen Claude, **offen**) | Größen bis 5 kHz | Moden bis 5 kHz |
|---|---|---|
| Gitarrendecke 0,13 m², 2,5 mm | ~87 000 (×0,15) | ~125 |
| Violindecke 0,045 m², 3 mm (Wölbung übergangen) | ~25 000 (×0,04) | ~36 |
| Cembaloboden 1,0 m², 3 mm | ~555 000 (×0,98) | ~800 |

Die Rechnung einer Gitarrendecke wäre also **klein** gegen den Flügel; teuer wird nicht die Platte, sondern die Kopplung
an die Luft (Einschätzung). Rippen und Leisten verändern die Modenzahl (versteifen) — die Tabelle zeigt die
Größenordnung, nicht den Wert.

```python
# Skript (Scratchpad der Sitzung, plattenabschaetzung.py), gekürzt:
A_s, H_S, DOF_S, MODEN_S = 2.050, 0.006, 568515, 820   # Steinway D (Bauplan, InstrumentBody.md)
for name, A, h in [("Gitarrendecke", 0.13, 0.0025), ("Violindecke", 0.045, 0.003), ("Cembaloboden", 1.0, 0.003)]:
    f = (A / A_s) * (H_S / h)        # Größen ~ A/h (gleiches Holz, gleiche Frequenz); Modendichte ~ A/h
    print(name, round(DOF_S * f), round(MODEN_S * f))
```

**Einschätzung je Ziel:** Cembalo und Upright kommen ohne neuen Rechenkern aus (nur Bauplan). Für die Gitarre ist die
Platte da, und die Kopplung an die Luft hat LumiViz schon in Wrights Form (Kolben je Deckenmode + Helmholtz-Luft,
S-K10 a) — nur mit Moden aus Wrights Tabelle statt aus einem Bauplan. Der Weg zur gemeinsamen Grundlage wäre also:
Moden aus `InstrumentBody` in dasselbe gekoppelte Modell (Frage F-K1); die Gitarre hat zudem die besten freien
Messdaten (§4), gegen die ein gerechneter Körper geprüft werden kann. Für die Violinfamilie ist der Ausbau groß (Schale, Stimmstock, Löcher); ein
Körper aus **gemessenen** Moden (Modalfit) ist dort wahrscheinlich der kürzere *und* der treuere Weg — offene Daten gibt
es aber nur für drei Violinen. Für die Membran trägt die Basis das Netz und den Eigenlöser, die Physik (Spannung,
Luftlast) ist neu.

## 4. Offene Daten (Lizenz am Ort)

| Daten | Inhalt | Lizenz (Wortlaut, Ort, Abruf) | Belegkraft | Eignung |
|---|---|---|---|---|
| Mores, Archiv 65 Gitarren, Zenodo 4604576 | Mobilität am Steg (zwei Kanäle) und drei Nahfeldkanäle; `qualified_selected_impulses.mat` 32 729 680 B, `Method.pdf`, `Comments.pdf`, MATLAB-Skripte, Fotos | `cc-by-4.0` (Feld `license` der Zenodo-API, 2026-10-26) | **nachgelesen** (Lizenz, Titel, Datum 2021-03-15, Dateiliste; `Method.pdf`, `Comments.pdf`, Maßliste in `quellen/`, siehe unten) | Gitarrenkörper aus Messung; Prüfstein für einen gerechneten |
| Mores, Zenodo 4010596 | analytisches Modell asymmetrisch beleisteter Gitarren, zwei .m-Dateien, PDF | `cc-by-4.0` (Zenodo-API, 2026-10-26) | **nachgelesen** | Vorbild für eine Gitarrendecke |
| CNSM, Zenodo 18696785 | `dataset.zip` 654 025 212 B; laut Paper Stegmobilität (Hammer, Laser-Vibrometer), drei Violinen | `cc-by-4.0` (Zenodo-API, 2026-10-26) | Lizenz und Datei **nachgelesen**; Inhalt nur Bericht (arXiv 2605.18305) | Violinkörper aus Messung |
| TU Berlin, „A Database of Anechoic Microphone Array Measurements of Musical Instruments“, DOI 10.14279/depositonce-19858 | Richtcharakteristiken (SOFA, OpenDAFF, GLL) und reflexionsarme Aufnahmen von 40 Instrumenten, u. a. Violine, Viola, Cello, Kontrabass, Gitarre, Harfe, Pauke, Blech, Holz | **Drei Angaben:** (1) Datensatz `dc.rights.uri` „http://rightsstatements.org/vocab/InC/1.0/“, Abzeichen „In Copyright“ (DSpace-API und Seite, Fassung 3, 2026-10-26); (2) Dokumentation der Fassung 3 (2. Februar 2024), S. 2: „provided under a Creative Commons BY-NC-SA licence“, Verweis auf creativecommons.org/licenses/by-nc-sa/4.0, Titelblatt mit Abzeichen BY NC SA; (3) Paper arXiv 2307.02110: CC BY-SA 4.0 (nur Bericht). Die `license.txt` im Bündel ist nur der Einreichungsvertrag mit der TU (keine Lizenz für Nutzer) | (1), (2) **nachgelesen**; (3) nur Bericht | Abstrahlung kleiner Körper und Aufnahmen — **erst nach Klärung** |
| Rau, Smith & Abel (CCRMA, FA2023) | 850 Messungen an Gitarren, Violinfamilie, Mandoline, Deckenhölzern | keine Lizenz auf der Seite (nur Bericht, LumiViz S146) | nur Bericht | nur nach Anfrage |
| Wright 1996 (Diss. Gitarre) | vier Größen je Mode (Frequenz, Masse, Monopolfläche, Q) | „All rights reserved“ (nur Bericht, LumiViz S146) | nur Bericht | Werte zitierbar, Datei nicht |
| „Messiah“-CT, Brandeis, DOI 10.48617/data.1109 | CT einer Stradivari | „by permission only“, „personal use only“ (nur Bericht) | nur Bericht | nicht frei |

**Mores im Einzelnen (nachgelesen, `quellen/Mores_2021_*`):**

- **Messung** (Method S. 2–4): Impulshammer (9 g, DYTRAN 5800SL) an drei Stellen des Stegs — Diskant (zwischen H- und
  e-Saite), Mitte (D/G), Bass (E/A); zwei Beschleunigungsaufnehmer (1 g, Kistler 8778A500) an Bass- und Diskantseite
  des Stegs, „allow to trace two of the degrees of freedom of the bridge“; drei Mikrofone (ROGA MI-17) 10 cm über der
  Decke (10 cm zur Diskant- bzw. Bassseite der Stegmitte, eine dritte 20 cm weiter oben); Erfassung DATaRec DIC6. Die
  Gitarre ruht nur auf dem unteren Klotz und dem Hals, „so the body vibrates freely, including the back plate“ (S. 3).
  Vor Ort in Sammlungen, mit Absorber auf dem Boden; Wände 2–6 m entfernt, „allowing 12 ms to 36 ms reflection-free
  radiation impulse responses“ (S. 1).
- **Daten** (S. 4): **48 kHz**; je Gitarre und Anschlagstelle eine ausgewählte Sekunde („yielding a resolution of 1 Hz in
  the transfer functions“), Hammer mindestens 10 N (Ausnahmen g04, g24, g55), Doppelschläge unter −30 dB (Ausnahme
  g05). Matrix `qualified_selected_impulses(guitar_number, signal, channel)`: Gitarre 1:65, Signal 1:144000 (3 × 1 s:
  Bass, Mitte, Diskant), Kanal 1 Hammer, 2 Aufnehmer Diskant, 3 Aufnehmer Bass, 4 Mikrofon oben, 5 Mikrofon Diskant,
  6 Mikrofon Bass. **Nicht normiert** — Kalibrierung und SI-Einheiten in den MATLAB-Skripten. Rohdaten (3 GB) nicht
  veröffentlicht; das Fernfeld (1 m) ist nicht im Datensatz.
- **Befund des Autors** (Comments S. 1): A0 „at roughly 100 Hz“, T1 um 200 Hz, darüber T2/T3 und A1 um 400 Hz; eine
  deutliche Nullstelle zwischen 400 und 700 Hz trennt den deterministischen vom „more or less statistical“ Bereich —
  **dieselbe Teilung wie KB1 a** (Moden unten, statistischer Teil oben). g62 „I would hesitate to use the data“, g07
  und g30 ohne Auswertung (schlechte Rohdaten).
- **Maße** (Maßliste, Legende S. 3): je Gitarre Breite der Decke oben/Taille/unten, Zargenhöhe an drei Stellen,
  Mensur, Länge der Decke, Breite des Schalllochs, Saitenlage, Gewicht. Beispiel g20/g40 (Faustino Conde 1964, die
  Bezugsgitarre): Breiten 281 / 242 / 367 mm, Zargen 96 / 99 / 98 mm, Mensur 652 mm, Decke 490 mm lang, Schallloch
  87 mm, 1495 g. Keine Deckendicke, keine Leisten (Fotos der Beleistung liegen im Datensatz).
- **Einschätzung:** Aus den Maßen von g20 ergibt Länge × mittlere Breite 0,49 m × 0,297 m = 0,146 m²; die
  geschwungene Form liegt darunter (Schallloch abgezogen) — die Annahme 0,13 m² in §3 liegt im Bereich.

**Nicht gefunden** (Reichweite im Bericht): offene Mobilität/Admittanz für Cello, Viola, Kontrabass, Harfe, Cembalo,
Pauke, Trommelkessel; Marimba-Resonatoren; frei lizenzierte Geometrien (Violine, Gitarre); offener Code für einen
gekoppelten Körper (FEM mit Luft).

## 5. Abstrahlung kleiner Körper

- LumiViz rechnet die Abstrahlung über das **Rayleigh-Integral einer Platte in Schallwand** (nachgelesen im Code und im
  Konzept §4.1; die Grenze unter rund 200–300 Hz beim Flügel führt das Konzept selbst als „nur Bericht“).
- Für kleine, freie Körper (Gitarre, Violine, ka < 1) stimmt die Schallwand nicht: Vorder- und Rückseite heben sich
  teilweise auf, Luftraum und Löcher strahlen als Monopol (nur Bericht: Richardson/Walker; Weinreich „sound hole sum
  rule“; Bissinger 2007 — f-Löcher tragen bei B1 über die Hälfte bei).
- **Möglichkeiten** (Einschätzung): (a) je Mode Ersatzquellen (Monopol + Dipol) mit Gewichten — passt in KB1 a als
  Abstrahlgewicht je Ausgang; (b) gemessene Richtcharakteristik je Terzband (TU Berlin, Lizenz ungeklärt); (c)
  Randelemente (BEM) vorab — teuer, kein offener Code gefunden.
- **Raum:** in LumiViz nicht Teil des Körpers; Faltung mit einer Raumimpulsantwort (Einschätzung). OpenAIR (York):
  Lizenz je Impulsantwort, Seite nicht erreichbar (nur Bericht).

## 6. Offen

- Lizenz der TU-Berlin-Datenbank: die Dokumentation sagt CC BY-NC-SA 4.0 (nicht kommerziell), der Datensatz „In
  Copyright“ — siehe [offene-fragen.md](../../gemeinsam/ic/offene-fragen.md) F-D1.
- Inhalt von CNSM `dataset.zip` (654 MB) — Download mit Freigabe, nicht ins Repo.
- Gitarre: Deckendicke und Leisten einer Bezugsgitarre mit Quelle — die Maßliste von Mores hat sie nicht; die Fotos der
  Beleistung (Zenodo 4604576) zeigen sie nur als Bild.
