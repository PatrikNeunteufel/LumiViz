# Modell-Entwurf — Lippe und Rohr (Klasse 7)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Rohr-Kern [gemeinsam/rohr.md](../../rohr/ic/rohr.md) (F-R1 bis F-R5 in
> [gemeinsam/offene-fragen.md](../../gemeinsam/ic/offene-fragen.md)), Bestand in LumiViz (IC-B1 a). Berichte:
> `recherche/2026-10-26_physik_spielweisen.md`, `…_alphorn_didgeridoo.md`.
> **Kennzeichnung:** **nachgelesen** · **nur Bericht** · **Einschätzung** · **offen**. Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand

Nachgelesen, `Spielweisen_Konzept.md`: SF10 a — Blech (`brass`) Lippe × Rohr, „das Rohr dient drei Anregungen“; SF6 a —
Lippe und Rohr: Sustain nein, Expression ja, Vibrato ja, Pitch ja („Lippe, Posaunenzug“); SK4 a — Expression als
„Stärke der Anregung (Blasdruck …) samt Klangfarbe“; Zielbild §1 — Blechbläser: „Dämpfer (Straight, Cup, Harmon,
Wah-Wah), Fall, Doit, Growl“; SF8 a — Dämpfer über eine Passage („das Umstecken eines Dämpfers“ rechnet das Modell),
Fall, Doit, Growl je Note; SK12 a — Spur und Kurve im Namen des Tons.

## 2. Kern (nach F-R1)

1. **Rohr vorab je Griff/Zugstellung:** Bohrung → Eingangsimpedanz (TMM/1D-FEM, Wandverluste) → Moden am Eingang;
   Abstrahlung am Schallstück als Abstrahlgewichte. Posaune: Moden auf einem **Raster von Zugstellungen**, dazwischen
   je Mode interpoliert (Einschätzung; Zuordnung der Moden über das Raster ist die Hürde). Ventile: je
   Ventilkombination ein Modensatz.
2. **Lippe je Note im Zeitbereich:** Masse-Feder mit einem Freiheitsgrad, nach außen schlagend, Fluss nach Bernoulli
   durch den Lippenspalt, gekoppelt an Druck und Fluss am Eingang. Startwerte (Tabelle §4). Die Lippenresonanz wird je
   Note **gesucht**, so dass die Spielfrequenz den Ton trifft (wie Petiot et al.: „Variable, according to the regime
   simulated“, nachgelesen S. 376).
3. **Schmettern:** Stufe am Rohrausgang — die auslaufende Welle verzerrt über eine äquivalente Länge nach der
   Stoßbildungslänge (Einschätzung, nach Gilbert & Dalmont 2010, nur Bericht). Gemessen an der Tenorposaune: der
   Spektralschwerpunkt vor der Stürze steigt im Crescendo „from 1.5 to 4.0“, im Mundstück nur „from 1.2 to 1.6“
   (Myers et al. 2012, nachgelesen S. 681) — das Schmettern entsteht im Rohr und ist für Posaune und Trompete Kern,
   nicht Deluxe (F-2).
4. **Deluxe:** FDTD (Willemsen et al.: Posaune mit dynamischem Gitter; Bilbao: Ventile mit Halbventil) für Halbventil
   und Zug-Glissando mit Bewegung.

## 3. Je Instrument

| Instrument | Rohr | Besonderes | Datenlage | Risiko (Einschätzung) |
|---|---|---|---|---|
| **Posaune** | Zug: Innenzug 70,8 cm r 0,69 cm … Schallstück 50,2 cm r 1 → 10,8 cm (Willemsen et al., Tab. 1, nachgelesen S. 157; Herkunft Smyth & Scott 2011); Länge 2,593–3,653 m (nur Bericht) | Zug als Pitch; Schmettern stark (B = 0,67–0,81 für Tenorposaunen, Myers Tab. I, nachgelesen) | **beste** — Geometrie, Lippe, Spielermessung (31 Spieler, Zenodo) | mittel (Zug-Raster, Schmettern) |
| Trompete | Bohrung **offen** (kein freies Profil); Impedanzen von 9 Trompeten nur als Moden (Petiot) | Ventile, Halbventil (Deluxe); Dämpfer | Lippe gut (Petiot), Bohrung fehlt | mittel |
| Horn | Bohrung offen | Hand in der Stürze, Stopfen („a new set of natural notes a semitone higher“, Campbell et al. 2021, nur Bericht); hohe dichte Moden | schwach | hoch |
| Tuba | Bohrung offen (Acta 2022 laut Suche mit Profilen, gesperrt) | Schmettern schwach (B ≈ 0,4 für Euphonium/Saxhorn, Myers) | schwach | mittel |
| **Alphorn** (Erweiterung) | vier Kegel + Becher, Horn 3: L 539,92 / 844,91 / 754,84 / 444,94 mm, R 16,00 / 28,50 / 44,00 mm (Marcinnò et al. 2026, Tab. 3, nachgelesen S. 18, CC BY 4.0); Fichte | Naturtonreihe mit Alphorn-Fa (11/8 = 551 Cent, Einschätzung); Holzwand mit erhöhten Verlusten (Faktor β, nur Bericht) | Geometrie gut, Impedanz nur gerechnet (verlustfrei, +64…+112 Cent gegen das Ziel, nur Bericht) | gering — **kein Deluxe** |
| **Didgeridoo** (Erweiterung) | 1,2–1,5 m, unregelmäßiger Kegel („typically 1.2 to 1.5 m“, Tarnopolsky et al. 2006 I, nachgelesen); raue Wand β ≈ 12 (nur Bericht) | **Vokaltrakt** vor der Lippe (F-R5): zwei LRC-Resonanzen um 1,5 und 2,5 kHz, Q 10 (Fletcher et al. 2006 II, Tab. I, nur Bericht); Stimme ins Rohr (Kombinationstöne); Zirkularatmung | Physik gut belegt (UNSW), Aufnahmen CC0 | mittel — **Deluxe-Kandidat**; neue Steuerung nötig (F-6); kulturelle Sensibilität (nur Bericht: CI-4-Instrumente „can also carry a secret spiritual significance“) |

## 4. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| Bohrung (Radius über der Länge) je Instrument | fest (Bauplan) | Posaune: Willemsen Tab. 1 (nachgelesen); Alphorn: Marcinnò Tab. 3 (nachgelesen); Trompete, Horn, Tuba **offen** | Impedanz (UNSW Posaune Boutin, ohne Lizenz — nur lokal) |
| Wandverluste | fest | Zwikker–Kosten (Rohr-Kern); Holz: Faktor 2–3 (Einschätzung nach Boutin 2017, nur Bericht) | Abklingen nach dem Ausblasen |
| Lippe: Masse je Fläche μ | Kurve | Trompete 0,5–2, gewählt 1,3 kg/m² (Petiot Tab. 1/2, nur Bericht; nachgelesen ist dort nur der Mundruck) | Helligkeit über pp/mf/ff (M2) |
| Lippe: Güte Q | fest | 5 bzw. 4–6 (Petiot); Mensch kleiner (nur Bericht) | Einschwingen (M3) |
| Lippe: Breite, Ruhespalt | fest | 10 mm; 0,05 bzw. 0,5 mm (Petiot, widersprüchlich); Posaune wirksam 13 mm (Boutin 2020, nur Bericht) | — |
| Lippenresonanz je Note | je Note gesucht | so, dass die Spielfrequenz den Ton trifft (Einschätzung) | Tonhöhe (M1) |
| Mundruck über Expression | Stärke (F-R2) | 500–14 000 Pa (Petiot Tab. 1, nachgelesen) | Pegel und Spektrum über pp/mf/ff (M2) |
| Schmettern (Stoßbildungslänge) | aus Bohrung | B-Wert je Instrument (Myers, nachgelesen) | Spektralschwerpunkt über der Stärke (M4) |
| Dämpfer: Impedanz und Abstrahlung | je Dämpfer ein Satz | Straight: Störresonanz ~77 Hz bei der Posaune (nur Bericht); Cup −20 dB um 1 kHz (nur Bericht); **offen** | VSCO 2 CE Harmon/Straight (M5) |
| Vokaltrakt (Didgeridoo) | Kurve „Vokal“ (F-6) | Fletcher et al. 2006 II Tab. I (nur Bericht) | Formanten im Spektrum (M6) |

## 5. Rechenzeit

Modal: wenige Dutzend Moden je Griff plus eine Lippe — Maestre et al. am Saxophon „above 30 times faster than
real-time“ mit 32 Resonatoren (nachgelesen, DAFx-18 S. 7), also weit unter der Grenze des Flügels. Deluxe-FDTD: Posaune
nach Willemsen ohne Wandverluste einige Prozent einer CPU in Echtzeit (Tab. 3, nur Bericht), mit Verlusten Trompete
0,86 s je 1 s (Harrison et al. 2015, nachgelesen in [gemeinsam/rohr.md](../../rohr/ic/rohr.md)).

## 6. Abweichungen vom Rahmen

- Ton je Note mit Spur (SK12 a) — vorhanden; Legato als Kette (F-R4).
- Dämpfer: das Umstecken ist keine Klangphase (der Bläser spielt dabei nicht) — SF8 a sagt „das Modell rechnet den
  Übergang“ (F-3).
- Didgeridoo: braucht eine Steuerung, die SF5 d nicht hat (F-6).
