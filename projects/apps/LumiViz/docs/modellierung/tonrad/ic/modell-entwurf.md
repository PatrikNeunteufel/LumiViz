# Modell-Entwurf — Tonrad (Klasse 11)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Bestand in LumiViz (IC-B1 a). Berichte: `recherche/2026-10-26_tonradgenerator.md`, `…_leslie.md`, `…_aufnahmen.md`.
> **Kennzeichnung:** **nachgelesen** · **nur Bericht** · **Einschätzung** · **offen**. Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand in LumiViz

Nachgelesen, `Spielweisen_Konzept.md`:
- **SF10 a:** „Hammond | — | Tonrad | Leslie“ — Tonrad als „elektromechanischer Schwinger ohne Anregung“; „Expression,
  Vibrato, Pitch nur bei dauernder Anregung“; „Welche Orgel `organ` und `house_organ` sind, klärt die Recherche ihrer
  Familie.“
- **SF8 a:** „Leslie langsam/schnell“ als Spielweise über eine Passage; „das Modell rechnet den Übergang (das Hochlaufen
  des Leslie …)“. Zielbild: „Orgel: Register, Leslie langsam/schnell“.
- §4.2–§4.5: Töne werden vorab gerechnet (Tonspeicher, Freeze, Klangfassung).

Nachgelesen, `Loop_Konzept.md`: **E1 a / E2 b** — Effekte sitzen als Kette am Clip (Verzerrer, Echo, Hall, Filter) und
haben ein Gedächtnis für den Nachklang; „Nicht drin: … Effekte je Kanal oder je Drum-Spur“.

Nachgelesen, `asset/loop/instruments`: `organ` — „A sine with five overtones that holds at full level while the key is
down.“ (Teiltöne 0,8 / 0,5 / 0,3 / 0 / 0,2); `house_organ` — „A sine with four overtones and a quick attack, for stabbed
chords.“ (Sustain 0,6, Decay 120 ms). Welches Vorbild `house_organ` hat, steht nicht in der Datei (offen, F-7).

## 2. Kern

1. **Generator als 91 periodische Räder mit gemeinsamer Phase** (Einschätzung des Berichts): jedes Rad eine feste
   Welle (die tiefen „roughly square-wave“, nur Bericht; Filter ab Rad 44 bzw. 49, nur Bericht), Frequenz aus
   Motordrehzahl × Zähne × Übersetzung — Abweichung von der gleichstufigen Stimmung bis „0.69 cents“ in der Mitte und
   knapp 2 Cent oben (Bericht, eigene Rechnung des Agenten). Zwei Tasten, die dasselbe Rad nutzen, addieren sich
   **kohärent** — darum keine eigenen Startphasen je Note (Einschätzung).
2. **Tasten × Zugriegel × Räder als Mischmatrix**: neun Fußlagen (16′ … 1′), Stufen 0–8, „in 3 dB increments“ (Werner &
   Abel, nur Bericht); Rückfaltung in der obersten Oktave (nur Bericht); Übersprechen innerhalb eines Radfachs, Pegel
   **offen** (setBfree setzt −40 dB, nur Bericht).
3. **Tastenklick**: neun Kontakte, die nicht gleichzeitig schließen, mit Prellen — gemessen „usually less than 5 ms“,
   insgesamt 10–60 ms bei angeschlagenen Tasten (Moro et al. 2017, CC BY 4.0 nachgelesen; Werte nur Bericht).
4. **Summenstufen** über die Passage (Einschätzung; F-1): Perkussion (eine Hüllkurve für das ganze Obermanual, neu nur
   nach dem Loslassen aller Tasten, 1′ stumm bei Perkussion an — nur Bericht), Scanner-Vibrato/Chorus, Vorverstärker
   mit leichter Kompression, Schweller, Leslie.
5. **Scanner-Vibrato/Chorus**: LC-Kette als Wave Digital Filter, abgetastet von einem Rotor mit 412 U/min = 6,87 Hz
   (Handbuch, nur Bericht); Gesamtverzögerung „≈ 0.85 ms“, mit „4× oversampling … effective though expensive“ (Werner et
   al. 2016, nachgelesen PDF-S. 6); V3 ≈ 1,5 % Frequenzhub ≈ 26 Cent (Handbuch, nur Bericht); Chorus = Mischung mit
   unverzögertem Signal (nur Bericht).
6. **Leslie** (Bericht Leslie): Verstärker (nichtlinear, „producing a pleasing ‚growl‘“, Smith et al. 2002, nachgelesen
   PDF-S. 1) → Weiche 800 Hz (Pekonen et al. modellieren „fourth-order digital Butterworth IIR filters with the cutoff
   frequency at 800 Hz“, nachgelesen PDF-S. 3; das Original 12 dB/Oktave, nur Bericht) → Horn (Doppler, Richtwirkung,
   Gehäusereflexionen) und Trommel (vor allem AM: „an AM ‚throb‘ is the main effect of the rotating woffer port“, Smith
   et al., Handout nachgelesen PDF-S. 20). Richtwirkung des Horns gemessen „in increments of 15 degrees“ (ebd.,
   nachgelesen PDF-S. 10). Drehzahl als Differenzengleichung 1. Ordnung mit getrennten Zeitkonstanten für Hochlauf und
   Auslauf, Horn schneller als Trommel (Herrera et al. 2009, nur Bericht); Werte der Zeitkonstanten **offen**.

**Ort im Signalweg:** Generator, Klick und Übersprechen ließen sich je Note rechnen, aber nur mit Phase aus der absoluten
Zeit; Perkussion, Scanner, Vorverstärker, Schweller und Leslie wirken auf die Summe und tragen einen Zustand über die
Passage. Das passt nicht zu „je Note ein vorab gerechneter Ton“ (Einschätzung beider Berichte; F-1, F-2).

**Deluxe** (Einschätzung): Leslie aus Impulsantworten über dem Rotorwinkel (Herrera: „approximately 20 filters were
sufficient“, nach Rumsey, nachgelesen PDF-S. 3) — braucht eigene Messungen; Vorverstärker als Röhrenschaltung.

## 3. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| Radfrequenzen | fest | Übersetzungen (Handbuch/Electric Druid, nur Bericht), 1200 U/min bei 60 Hz | Tonhöhe je Taste (M1) |
| Radwellenform, Filter, Pegel je Rad | fest | Handbuch Tab. V (Spannungen, nur Bericht); tiefe Räder rechteckähnlich | Spektrum einzelner Töne (M2) |
| Zugriegelstufen | fest | 3 dB je Stufe (Werner & Abel, nur Bericht) | Spektrum je Registrierung (M2) |
| Rückfaltung | fest | Radbereiche je Zugriegel (Electric Druid, nur Bericht) | Spektrum oben (M2) |
| Übersprechen | fest | **offen** (setBfree −40 dB, nur Bericht) | Restsignal im Dauerton (M3) |
| Tastenklick | je Anschlag | Moro et al. 2017 (nur Bericht) | Einsatz (M4) |
| Perkussion (fast/slow, soft) | Passage | Bonedo: „‘Fast’ (ca. 250 ms) und ‘Slow’ (ca. 900 ms)“ (nur Bericht) | Abklingen der 2./3. Harmonischen (M5) |
| Scanner (Rate, Tiefe V1–C3) | Passage (F-4) | 6,87 Hz, V3 ≈ 26 Cent (Handbuch, nur Bericht); Kette nach Werner et al. (nachgelesen) | B3 Vibrato an Aufnahmen (M6) |
| Leslie: Drehzahlen | Passage (SF8 a) | Horn ≈ 48/400 U/min, Trommel ≈ 40/342 (Nutzermessung HammondWiki, nur Bericht) | Modulationsrate (M7) |
| Leslie: Zeitkonstanten Hoch-/Auslauf | fest | **offen** | Hochlauf-Aufnahmen CC0 (M7) |
| Leslie: Horn-Radius, Gehäuse | fest | Radius **offen** (0,18–0,2 m geschätzt, nur Bericht); Gehäuse 104 × 74 × 53 cm (nur Bericht) | Doppler-Tiefe (M7) |
| Leslie-Filter (Weiche, Richtwirkung) | fest | 800 Hz (nachgelesen); Richtwirkung **offen** | Zenodo 3562442 trocken/nass (M8) |
| Vorverstärker, Schweller | Stärke (F-3) | **offen** (Kennlinie nicht gefunden) | — |

## 4. Rechenzeit

Gering: 91 Tabellenwellen und neun Multiplikationen je klingender Taste; eine LC-Kette mit 18 Gliedern je Manual; Leslie
mit Verzögerungsleitungen (Einschätzung im Bericht). Pekonen et al. kommen mit „only ten sinusoidal oscillators“ je Taste
aus (nachgelesen PDF-S. 3), ohne Rückfaltung, Übersprechen und Perkussion.

## 5. Risiken

- **Architektur** (F-1, F-2): Kopplung über Generator, Perkussion, Scanner, Leslie passt nicht zum Ton je Note.
- **Daten:** keine freie Aufnahme einzelner Zugriegel; Leslie-Zeitkonstanten und Richtwirkung ungemessen.
- **Rechte:** Handbücher ungeklärt; setBfree GPL (kein Code); die Modal-Idee von Werner & Abel ist patentiert (nur
  Bericht); Markennamen (F-7).

## 6. Abweichungen vom Rahmen

- Rechnung je Clip/Passage statt je Note (F-1).
- Expression trotz „ohne Anregung“ (F-3): der Schweller ist eine dauernde Steuerung am Ausgang.
