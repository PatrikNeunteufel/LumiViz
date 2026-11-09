# Modell-Entwurf — Bogen-Saite (Klasse 3)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Körper [gemeinsam/koerper.md](../../koerper/ic/koerper.md), Messkette [gemeinsam/messkette.md](../../messung/ic/messkette.md),
> Bestand in LumiViz (IC-B1 a). Berichte: `recherche/2026-10-26_physik_rechenverfahren.md`,
> `…_korpus_daempfer_ensemble.md`, `…_spielweisen_steuerung.md`, `…_aufnahmen.md`.
> **Kennzeichnung:** **nachgelesen** (von der Sitzung selbst, mit Seite) · **nur Bericht** · **Einschätzung** · **offen**.
> Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand in LumiViz

Nachgelesen, `Spielweisen_Konzept.md`:
- **SF9 a:** Bogen-Saite — „Saitensatz und Korpus da, neu die Anregung durch den Bogen“; mitgeliefert heute `violin`,
  `strings`, `pizzicato_violin`, `upright_bass`.
- **SF10 a:** „Geige, Streicher, Kontrabass“ = Bogen × Saite, Ausstattung leer; „die Saite [dient] vier [Anregungen]
  (Zupfen, Hammer, Bogen, …)“.
- **SF6 a:** Bogen-Saite — Sustain nein, Sostenuto nein, Expression „ja (Bogen)“, Vibrato ja, Pitch „ja (Glissando)“.
- **SK4 a:** Expression bei dauernder Anregung = „die Stärke der Anregung (Blasdruck, Bogendruck) samt Klangfarbe“.
- **SK10 a:** ohne eigene Rate 5,5 Hz — „dass gesungenes und gestrichenes Vibrato um 5 bis 7 Hz liegt, aus dem
  Gedächtnis — in der Recherche der Bogen-Saite zu belegen“.
- **Zielbild §1, Streicher:** „Pizzicato, *con sordino*, Tremolo, Spiccato, Col legno, Vibrato“; **SF8 a:** Pizzicato,
  Tremolo, Spiccato, Col legno je Note; „Pizzicato und Arco“ an der Note; Dämpfer über eine Passage.

Nachgelesen, `Saiten_Konzept.md`:
- **S-K1 a:** Waveguide-Saite — „Trägt später den Bogen (Violine).“
- **S-K4 a / S-K7 a:** ein Korpus braucht je Resonanz Frequenz, Güte, Stärke aus veröffentlichten Messungen;
  „abgeleitet zählt als belegt, wenn Rechenweg oder Abbildung mit Fundstelle dabeistehen“; *Violin* und *Upright bass*
  bleiben draußen, bis ihre Lücke belegt ist (Violine: Stärken; Kontrabass: Güte — S132-Recherche).
- Mensur der Vorlage Violine 327 mm; Spread Violine 7,0 ms, Kontrabass 17,5 ms (S-W19 a); heute ist `violin` eine Säge
  (`saw`, Art `pads`).

Nachgelesen, `modellierung/hammer-saite/modell.md`: die Saite des Flügels ist **modal** (Bank, Zambon & Fontana 2010).

## 2. Kern

1. **Saite modal, Reibstelle am Bogenpunkt** (Einschätzung; F-2): Moden der Saite mit eigener Dämpfung je Mode, eine
   nichtlineare Reibkraft am Bogenpunkt, nicht iterativ gelöst. Gemessen in Matlab: Laufzeit zu Echtzeit **0,19 / 0,39 /
   0,91** bei 1-, 2-, 5-facher Überabtastung von 44,1 kHz für eine steife, gedämpfte Cello-D3-Saite (Russo, Ducceschi &
   Bilbao 2022, Tab. 2, nachgelesen S. 129; CC BY 4.0, nachgelesen S. 122). Gegenweg: Waveguide mit Reibstelle nach
   McIntyre/Woodhouse (S-K1 a); der Woodhouse-Typ mit Torsion lief bei 200 kHz (Serafin et al. 1999, nur Bericht).
2. **Reibung, gestuft:**
   - Stufe 1 Reibkurve: „μ = 0.4e(v−vb)/0.01 + 0.45e(v−vb)/0.1 + 0.35“ (Galluzzo, Woodhouse & Mansour 2017, Gl. 1,
     nachgelesen S. 5) — μs = 1,2, μd → 0,35 (eigene Rechnung). Dieselbe Arbeit gewinnt eine ganz andere Kurve
     „µ = 0.4e(v−vb)/0.7 + 0.35“ (Gl. 2, nachgelesen) — die Kurve ist unsicher.
   - Stufe 2 thermisch-plastisch (Smith & Woodhouse 2000): eine Kontakttemperatur als Zustand; nach Galluzzo et al.
     „significantly better“, nach Woodhouse 2014 aber „a bit too benign and smooth“ (beides nur Bericht). Stoffwerte aus
     Woodhouse 2003 **offen** (nicht erreicht).
   - Setzen an gemessenen Saitenantworten mit Roboterbogen (Lampis, mdw, `cc-by-4.0` nachgelesen; Messplan M2).
3. **Korpus in der Rückkopplung am Steg** (Einschätzung): die Korpusmoden als Admittanz am Steg, an der die Saite
   endet — nicht als Filter dahinter. Nur so entsteht der Wolfton: die Korpusresonanz hebt die Mindest-Bogenkraft, die
   Helmholtzbewegung kippt (Woodhouse 2014, nur Bericht; Mechanismus „under-sized compared to a scaled-up violin“,
   nachgelesen PDF-S. 11). Das passt zu KB1 a (Körper = Moden) und zum Waveguide-Korpus „hinter dem Steg“ in S-K1 a nur,
   wenn dieser rückwirkt.
4. **Bogen geschwindigkeitsgetrieben**, Bogen-Steg-Abstand β und Kraft F als Verlauf je Note; daraus Expression als
   **Weg** (F-3): „the amplitude of the string vibration is proportional to vB/β“ (Schoonderwaldt, Sinclair & Wanderley
   2007, nachgelesen PDF-S. 2); die Kraft trägt vor allem die Obertöne (ebd., nur Bericht). Der Weg muss im
   Schelleng-Fenster bleiben: Fmin ∝ β⁻², Fmax ∝ β⁻¹ (nur Bericht) — aber gemessen „The minimum bow force was found to be
   independent of bow velocity, which is in clear contradiction to Schelleng’s prediction“ (Schoonderwaldt, Guettler &
   Askenfelt 2008, nachgelesen S. 604): das Fenster wird **am Modell** ermittelt, nicht aus der Formel.
5. **Einschwingen** als Prüfstein: annehmbar „50 ms (≤10 nominal periods, open G string, violin)“ bei verlängerten
   Perioden, „90 ms (≤18 periods)“ bei Mehrfachgleiten (Guettler & Askenfelt 1997, nachgelesen S. 2903). Der Ansatz wird
   nach dem Guettler-Diagramm geformt (Kraft konstant, Beschleunigung im Keil — nur Bericht).
6. **Pizzicato** = dieselbe Saite, gezupft (SF10 a: eine Saite, vier Anregungen); „in violin pizzicato no tone build-up
   lasted longer than 4 ms“ (Melka, bei Guettler 2002, nur Bericht).
7. **Vibrato** = Längenänderung des Fingers (FM); die AM je Teilton entsteht durch den Korpus von selbst — „the absence
   of amplitude modulation causes marked changes in both sound quality …“ (Mellody & Wakefield 2000, nur Bericht), also
   **kein** eigener Parameter (wie F-R3 a bei den Bläsern).

**Deluxe** (Einschätzung): zwei Polarisationen und Finger-/Griffbrettkontakt in Finiten Differenzen (Desvages & Bilbao,
„does not run in real-time“, nur Bericht); Torsion; Bogenbreite (Rauschen durch „differential slipping“, nur Bericht);
Richtcharakteristik je Band („directional tone colour“, Weinreich).

## 3. Je Instrument

| Instrument | Saite | Korpus | Datenlage | Risiko (Einschätzung) |
|---|---|---|---|---|
| **Cello** | D3: 146,8 Hz, 111 N, 2,7 g/m, Z0 0,55 kg/s, Biegesteife 3,0·10⁻⁴ N·m² (Galluzzo Tab. 1, nur Bericht) | Wolfton-Mode 171,7–176,5 Hz, **Q 13,4** gehalten vom Spieler gegen 18,1/18,5 im Gestell, **−25,6 dB re 1 m/(N·s)** (Zhang & Woodhouse Tab. II, nachgelesen PDF-S. 9, Tabellentext verschoben); A0 „near 104 Hz“, Bridge Hill um 1 kHz (nur Bericht) | **beste Bogendaten** (Lampis, Pàmies-Vilà, Chudy) | mittel |
| **Violine** | D: 1,29 g/m, Z0 0,25 kg/s; E: 0,41 g/m, Z0 0,18 kg/s (Schoonderwaldt 2008, nur Bericht); Mensur 327 mm (Bestand) | A0 270–275 Hz, CBR ~407, B1− ~475, B1+ ~541 Hz, ζ ≈ 1 % (Bestand S132); Bridge Hill um 2,3 kHz (nur Bericht); **Stärken offen** (F-7) | gut (Aufnahmen, CNSM-Mobilität) | mittel |
| Viola | offen | Resonanzen „20–40% lower than with the violin“ (Pätynen & Lokki, nur Bericht); keine Güte/Stärke | Aufnahmen gut, Korpus schwach | mittel–hoch |
| Kontrabass | offen | A0 65–67 Hz, T1 115 Hz (Bestand S132); **Güte offen** (Askenfelt 1982 nicht erreicht) | Aufnahmen gut (VSCO mit/ohne Vibrato) | mittel–hoch |
| **Ensemble** | N Stimmen (F-8) | je Stimme gestreut | Einsatzstreuung 30–50 ms im Trio (Rasch 1979, nur Bericht); Stimmungsstreuung nur am Chor gemessen | mittel |

## 4. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| Saite: Spannung, Dichte, Biegesteife, Dämpfung je Mode | fest je Saite | Cello D3, Violin D/E (nur Bericht); Viola, Kontrabass **offen** | Tonhöhe, Inharmonizität, Abklingen (M1) |
| Reibung (μs, μd, Form) bzw. thermische Werte | fest | Galluzzo Gl. 1 (nachgelesen); thermisch **offen** | Schelleng-Fenster und Einschwingen am Roboterbogen (M2) |
| Korpus: Frequenz, Güte, Stärke je Mode | fest je Instrument | Violine Frequenzen (Bestand), Stärken **offen**; Cello Wolf-Mode (nachgelesen); Viola, Kontrabass **offen** | Stegmobilität CNSM, Modalfit B6 (M4) |
| Expression → (vB, β, F) | Weg (F-3) | Richtung belegt (vB/β, nachgelesen); Kraft 0,5–1,5 N Violine („Normal bow forces range between 0.5 and 1.5 N for the violin“, Guettler 1997, nachgelesen PDF-S. 2); vB 16,62 cm/s im Legato am Cello (Chudy et al., nachgelesen PDF-S. 3); β ≈ 1/30 … 1/6 (nur Bericht) | Pegel und Spektrum über pp/mf/ff (M3) |
| Ansatz (Kraft, Beschleunigung) | fest je Artikulation | Guettler-Diagramm (nur Bericht) | Einschwingdauer (M5) |
| Vibrato: Rate, Tiefe | Spur (Tiefe in Cent) + Rate am Instrument (F-5) | 5,5 Hz Schüler/Studenten (Geringer & Allen 2004), 5,9 Hz und „mean excursion of ±15.2 cents“ (Mellody & Wakefield 2000), 6,63 Hz und 63 Cent Breite bei Solisten (MacLeod 2010) — alle nachgelesen (Crossref-Abstracts) | Rate, Tiefe, AM je Teilton (M6) |
| Bogenwechsel | aus Bogenlänge | Haar „normal length (650 mm)“ (Guettler o. J., nur Bericht) | Bogenwechsel (M8, F-4) |
| Springbogen (Spiccato, Sautillé) | je Note | Sprungrate „typically between 6 and 35 Hz for impacts at hair lengths of 8 to 60 cm from the frog“ (Guettler 2002, nachgelesen PDF-S. 20); Aufbau „in the order of 30 ms“ (nur Bericht) | VSCO `spic` (M7) |
| Dämpfer | Passage (SF8 a) | Masse am Steg, „its main action is to shift the bridge hill downwards“ (Woodhouse 2014, nachgelesen PDF-S. 34); 1-g-Dämpfer: 1000–2500 Hz „from 5 to 13 dB“ weniger (Elie et al., nachgelesen PDF-S. 7) | **offen** — keine freie Aufnahme mit Sordino |
| Rauschen (Bogenbreite) | Kurve | offen | Restsignal (B4, M7) |

## 5. Rechenzeit

Modal mit Reibstelle 0,19–0,91 der Echtzeit in Matlab (nachgelesen, s. §2); Korpus als Modalfilter mit 18–27 Moden
zwischen 100 Hz und 6 kHz (Maestre et al. 2013, nur Bericht) — zusammen in C++ deutlich unter Echtzeit
(Einschätzung). Ensemble mit 16 Stimmen ≈ 16-fach (Einschätzung). Vorab je Note gerechnet ist Echtzeit nicht nötig.

## 6. Risiken

- **Determinismus:** in chaotischen Bereichen „different computers gave slightly different results“ (Serafin et al.
  1999, nur Bericht) — ein vorab gerechneter Ton muss auf jedem Rechner gleich herauskommen; die Fließkomma-Regeln von
  LumiViz gelten, das Modell muss sie halten (Einschätzung).
- **Reibkurve:** zwei Kurven aus derselben Arbeit unterscheiden sich stark (s. §2); Kratzen beim Ansatz.
- **Korpus:** Stärken der Violine, Güte des Kontrabasses, alles der Viola offen (S-K7 a hält die Korpusse draußen).
- **Expression:** keine veröffentlichte Abbildung auf (vB, β, F) gefunden; der Weg wird an pp/mf/ff gesetzt.

## 7. Abweichungen vom Rahmen

- **Nachtrag Z1:** Die Zusatzfrage Z1 spricht für die Waveguide-Saite mit rückwirkendem modalem Korpus (F-2, Empfehlung
  neu b, wie S-K1 a); §2.1 „modal“ gilt nur noch, wenn die zweiseitige modale Kopplung gelingt.
- Saitenverfahren modal statt Waveguide (F-2) — gegen den Satz „Trägt später den Bogen“ in S-K1 a.
- Expression als Weg in (vB, β, F) statt „Bogendruck“ allein (F-3; SK4 a nennt „Bogendruck“).
- Der Korpus wirkt am Steg zurück (Wolfton) statt als Filter hinter der Saite.
