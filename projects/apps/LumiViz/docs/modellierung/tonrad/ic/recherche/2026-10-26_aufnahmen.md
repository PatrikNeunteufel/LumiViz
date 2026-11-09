# Recherche IC1: Tonrad — freie Aufnahmen und Messdaten

> **Auftrag:** Klasse 11 (Hammond-Orgel mit Leslie): freie Aufnahmen und Messdaten mit an der Quelle geprüfter Lizenz —
> Einzelzugriegel direkt, Leslie langsam/schnell mit Hochlauf, Leslie-Impulsantworten.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: alle Wortlaute [wörtlich, Werkzeug] (über WebFetch), keiner im Rohtext selbst gelesen.
> **Reichweite:** im Bericht selbst (3 Suchen, 43 Abrufe; GitHub-Codesuche nicht erreichbar; Freesound meist nur erste
> Ergebnisseite).
> **Von der Sitzung selbst nachgelesen:** siehe `../aufnahmen.md`.
> **Eingeflossen in:** `../aufnahmen.md`, `../messplan.md`, `../offene-fragen.md`.

---

# Klasse 11 „Tonrad“: freie Aufnahmen und Messdaten mit geprüfter Lizenz

**Auftrag:** Freie Aufnahmen und Messdaten mit an der Quelle geprüfter Lizenz finden, gegen die ein gerechnetes Tonrad- und Leslie-Modell gemessen werden kann.
**Datum:** 2026-10-26. Alle Abrufe sind von diesem Tag.

**Reichweite:**
- 3 Websuchen und 43 Seitenabrufe, insgesamt 46.
- **Orte:**
  - GitHub-API: VCSL, VSCO-2-CE, setBfree
  - Freesound: Suchen nach „hammond“, „hammond organ“ (Seiten 1 und 2), „tonewheel“, „leslie“, „leslie speaker“, „drawbar“, „B3 organ“; dazu 7 Sound-Seiten und 1 Paket-Seite
  - Magenta/NSynth
  - Wikimedia Commons: Mediensuche und API `extmetadata`, 2 Dateiseiten
  - University of Iowa MIS
  - Philharmonia
  - Zenodo-API: 2 Suchen, 1 Datensatz
  - archive.org: 2 Suchen, 1 Metadaten-Abruf
  - KVR: HamOrg
- **Nicht erreichbar:** die GitHub-Codesuche (401, ohne Anmeldung). Zenodo hat nicht gesperrt.
- **Nicht vollständig gelesen:**
  - Freesound: je Suche nur die erste Ergebnisseite, außer bei „hammond organ“ (beide Seiten).
  - Im Paket „tonewheel organ sound samples“ habe ich die Lizenz nur bei 2 von 12 Sounds einzeln geprüft.
  - Die Zenodo-Suche „hammond organ“ habe ich nur bis Zeichen 100 000 gelesen.

**Kennzeichnung:** Alle Abrufe liefen über WebFetch. Der Wortlaut ist deshalb höchstens [wörtlich, Werkzeug]; nichts davon habe ich in der Rohdatei selbst gelesen. Für die Übernahme sollte man die Lizenzzeilen einmal von Hand gegenlesen.

---

## 1. Datensätze

### 1a. Lizenzen (Wortlaut, Ort, Datum)

Alle Lizenzangaben sind [wörtlich, Werkzeug], abgerufen am 2026-10-26.

| # | Fundort | Lizenz im Wortlaut, Ort | Übernahme nach LumiViz |
|---|---|---|---|
| 1 | Freesound, stomachache: [136653](https://freesound.org/people/stomachache/sounds/136653/) „lesliexywindup.WAV“ und [136652](https://freesound.org/people/stomachache/sounds/136652/) „lesliexywindupwide.WAV“ | „Creative Commons 0“ – „You can copy, modify, distribute and perform the sound, even for commercial purposes“ (Sound-Seite) | frei |
| 2 | Wikimedia Commons: [LeslieCabinetSlowFastSlow.ogg](https://commons.wikimedia.org/wiki/File:LeslieCabinetSlowFastSlow.ogg) | API-Feld LicenseShortName: „Public domain“. Seite: „I, the copyright holder of this work, release this work into the public domain.“ | frei |
| 3 | Wikimedia Commons: [LezlieCabinetSequence.ogg](https://commons.wikimedia.org/wiki/File:LezlieCabinetSequence.ogg) | LicenseShortName: „Public domain“; Seite: „…has been released as public domain“ | frei |
| 4 | Freesound, hammondman, Paket [18844](https://freesound.org/people/hammondman/packs/18844/) „tonewheel organ sound samples“ | Je Sound, geprüft nur bei 333770 und 333769: „Creative Commons 0“. Die Paket-Seite zeigt keine Lizenz. | frei (nur die beiden geprüften Sounds) |
| 5 | Freesound, r0cket_fuel: [522566](https://freesound.org/people/r0cket_fuel/sounds/522566/) „Drawbar_C_Chord.wav“ | „Creative Commons 0“ | frei |
| 6 | Freesound, MrAuralization: [158628](https://freesound.org/people/MrAuralization/sounds/158628/) „Hammond Tonewheel Organ“ | „Attribution 4.0“ | Namensnennung |
| 7 | Wikimedia Commons: [Hammond Organ - Model A Medley.ogg](https://commons.wikimedia.org/wiki/File:Hammond_Organ_-_Model_A_Medley.ogg) | LicenseShortName: „CC BY 3.0“ | Namensnennung |
| 8 | archive.org: [beckstrom_hammond_organ_drones](https://archive.org/details/beckstrom_hammond_organ_drones) | licenseurl: „https://creativecommons.org/licenses/by/4.0/“; Beschreibung: „just please credit me (Chris Beckstrom)“ | Namensnennung |
| 9 | setBfree (GitHub pantherb/setBfree): `b_conv/ir/ir_leslie-44100.wav` und `ir_leslie-48000.wav` | Datei `b_conv/ir/license` nennt „http://creativecommons.org/licenses/by-sa/3.0/us/“ [Lizenzkennung wörtlich, Werkzeug; der übrige Text nur Zusammenfassung]. Rechteinhaber Ken Restivo (2009) und Robin Gareus (2012). Der Code steht unter GPL-2.0 (API-Feld `spdx_id`: „GPL-2.0“). | ShareAlike |
| 10 | Zenodo [3562442](https://zenodo.org/records/3562442), Martínez Ramírez, Benetos, Reiss: „Deep Learning for Black-Box Modeling of Audio Effects“ | API-Feld `metadata.license`: `{"id": "cc-by-4.0"}` | Namensnennung |
| 11 | NSynth (magenta.tensorflow.org/datasets/nsynth) | „…under a Creative Commons Attribution 4.0 International (CC BY 4.0) license.“ | Namensnennung (aber kaum brauchbar, siehe 1b) |
| 12 | Freesound, Charliefarley: [34136](https://freesound.org/people/Charliefarley/sounds/34136/) „hammond1.wav“ | „Creative Commons 0“ | frei, aber unbrauchbar |

### 1b. Inhalt, Größe und Eignung

Die Angaben zum Inhalt stammen von den Sound- oder Dateiseiten [wörtlich, Werkzeug], soweit sie in Anführungszeichen stehen.

| # | Inhalt | Größe, Format | Eignung |
|---|---|---|---|
| 1 | „Electric organ with spinning Leslie speaker cabinet turned on while the speaker winds up, then off again..“; „Close miked with a Sony-PCMD50 in a wide stereo pattern.“ Orgelmodell nicht genannt. | 45,8 s / 11,6 MB (136653) und 34,0 s / 8,6 MB (136652); WAV, 44,1 kHz, 24 bit, stereo | Mit Leslie. **Hochlauf und Auslauf.** Wohl die beste freie Quelle für die Rotordynamik. Keine Zugriegel-Angaben, kein Direktsignal. |
| 2 | „A simple chord played on a Hammond organ where the leslie cabinet goes from low speed to fast speed and back.“ Aufgenommen 2005; die Datei steht in der Kategorie „Synthetic sounds“ – ob echt oder Simulation, bleibt offen. | 26,5 s, 477 kB, OGG | Mit Leslie; langsam → schnell → langsam, also Hochlauf und Auslauf. Echtheit unklar. |
| 3 | „A short sequence played on a Hammond organ where the leslie cabinet speedup gets used three times.“ | 26,1 s, 471 kB, OGG | Mit Leslie, dreimal Hochlauf. Echtheit unklar. |
| 4 | 12 Einzeltöne (d3–g5, zum Beispiel f5.wav, g2.wav). Beschreibung „real organ slow rotary“, Tags „b3“, „organ“, „tonewheel“. | Je 1–2 s, WAV, 32 kHz, 16 bit, mono (81,8 kB / 125,5 kB) | Einzeltöne, aber mit Leslie langsam und sehr kurz. Zugriegel nicht genannt. Nur eingeschränkt brauchbar. |
| 5 | „a c note on a hammond organ“ | 3,2 s, WAV, 48 kHz, 16 bit, stereo, 599 kB | Ein Einzelton; Zugriegel, Leslie und Modell nicht genannt. Schwach. |
| 6 | „Hammond Tonewheel H-series Organ with Leslie cabinet. Recorded with a Zoom H1. 16bit/44.1KHz.“ Gespielt (Gospel-Jam). | 23,4 s, 3,9 MB | Musik mit Leslie, kein Messmaterial. |
| 7 | „A medley played on a Hammond Organ 1935 Model A serial 389 with String Bass, Percussion, Reverb, Leslie 122, PR40“ | 206,6 s, 5,3 MB | Musik, Instrument gut benannt. Nur zum Hörvergleich. |
| 8 | Hammond **M100**, Akkord- und Cluster-Drones. „recorded 2019-03-07 with a Røde NT1A and a Shure SM57“. Laut Werkzeug steht das NT1A im Raum, das SM57 nah am mittleren Lautsprecher. Leslie und Zugriegel nicht genannt. Datei „…organ_turning_off_pitch_down“: **Auslauf des Tonradmotors**. | 31 WAV-Dateien, je 33–74 s, je 4–10 MB, zusammen rund 210 MB | Lange stationäre Klänge über den Orgel-eigenen Lautsprecher, eher kein Leslie (meine Annahme). Gut für Spektrum und Leckage im eingeschwungenen Zustand. Die Auslauf-Datei passt zur Motordynamik. |
| 9 | Eine Impulsantwort für den Leslie-Hornpfad, die setBfree mitliefert. Herkunft und Messaufbau stehen nicht im README. | 166 kB bzw. 181 kB, WAV | Nur ein IR, kein Satz über Rotorwinkel. Brauchbar als Referenzfilter. |
| 10 | „Dry and wet bass and guitar recordings“; nass „from the horn and woofer of a 145 Leslie speaker cabinet.“ Die Trockensignale stammen aus IDMT-SMT-Audio-Effects. | AudioMDPI.zip, 967.897.691 Bytes | **Gepaarte trockene und nasse Signale mit Horn und Bass getrennt** – sehr gut, um einen Leslie-Filter gegen echte Messungen zu prüfen. Keine Orgel als Quelle. Chorale/Tremolo-Stellung, Mikrofone und Rampen sind im Datensatz nicht beschrieben. Unklar ist auch, ob IDMT-SMT unter derselben Lizenz steht (nicht geprüft). |
| 11 | Orgel-Familie: 36.577 Töne, davon 36.401 „electronic“ und 176 „acoustic“. Quelle laut Seite: „commercial sample libraries“. Format „four second, monophonic 16kHz audio snippets“. | 305.979 Töne insgesamt (Gesamtgröße nicht auf der Seite) | Die Instrumente sind nur mit Nummern benannt; ob echte Tonradorgeln darunter sind, sagt die Seite nicht. 16 kHz ist für Tastenklick und Leckage zu wenig. **Für die Messung ungeeignet.** |
| 12 | „screaming Hammond organ looped compressed“ | 15 s | Unbrauchbar. |

---

## 2. Geprüft, aber keine Tonradorgel und keine passende Lizenz

- **VCSL** (sgossner/VCSL): LICENSE beginnt mit „Creative Commons Legal Code / CC0 1.0 Universal“ [wörtlich, Werkzeug]. Der Ordner `Electrophones` enthält nur „TX81Z“, also einen FM-Synthesizer. Bei den freien Aerophonen gibt es nur Mundharmonikas und eine Sirene. **Keine Tonradorgel.**
- **VSCO 2 CE**: LICENSE beginnt mit „CC0 1.0 Universal“. `Keys/Organ` enthält Loud, Quiet und „Rode_Blower.wav“; laut Info.txt aufgenommen von „Simon Dalzell of Ivy Audio“. Es handelt sich um eine **Pfeifenorgel** – das schließe ich aus dem Gebläse und Ivy Audio, es ist nicht belegt. Keine Tonradorgel.
- **University of Iowa MIS**: keine Orgel in der Instrumentenliste. Nutzungsangabe: „may be downloaded and used for any projects, without restrictions“.
- **Philharmonia**: keine Orgel oder Tasteninstrumente. Die Bedingungen schließen eine Übernahme ohnehin aus: „must not be sold or made available ‘as is’ (i.e. as samples or as a sampler instrument)“. Daraus folgt: **nicht übernehmbar**.
- **HamOrg** (Michael Picher, KVR): „samples recorded from a direct input setup“, chromatisch, mit Perkussion sowie Vibrato und Chorus, ohne Leslie. Das wäre inhaltlich ideal. Aber: Preis 15 $, eine Free-Version wird nur erwähnt, die Seite nennt **keine Lizenz**. **Nicht übernehmbar**, solange keine Lizenz vorliegt.
- **Wikimedia Commons**, „Drawbar C Chord.ogg“: Public domain, 2,8 s, nur „A C chord on a drawbar organ“. Ob es eine Tonradorgel ist, bleibt unklar.
- **Wikimedia Commons**, „Comparison … leslie cab simulation.ogg“: CC BY-SA 4.0; Gitarre durch eine Amplitube-Simulation. Keine echte Leslie.

## 3. Nicht gefunden

Kein Treffer ist kein Beweis.

- **Einzelne Zugriegel** (etwa nur 8′) über den ganzen Umfang, direkt ohne Leslie aufgenommen, unter freier Lizenz: nichts gefunden. Weder Freesound (erste Ergebnisseiten), Commons, Zenodo noch archive.org lieferten so etwas. Die Websuche zeigte nur kommerzielle Bibliotheken.
- Aufnahmen, die Perkussion an/aus, die Vibrato/Chorus-Stellungen V1–C3 oder den Tastenklick einzeln dokumentieren: nichts.
- Leslie-Impulsantworten über den Rotorwinkel oder Messungen der Rotordrehzahl und ihrer Zeitkonstanten: nichts. Die Websuche fand nur allgemeine Raum- und Lautsprecher-IR-Sammlungen.
- In Zenodo kein Tonrad-Datensatz; die Suche „tonewheel OR rotary speaker OR leslie speaker“ lieferte nur Nr. 10.
- setBfree hat kein Beispiel mit Einzeltönen; nur `examples/Larry_setsBfree.ogg` (3,3 MB). Dessen Lizenz habe ich nicht eigens geprüft, er fällt vermutlich unter die GPL des Repos.

## 4. Eigene Einschätzung (kein Beleg)

- **Für die Leslie-Dynamik** reicht das freie Material für eine erste Prüfung. Nr. 1 ist CC0 und eine Nahaufnahme mit Hochlauf und Auslauf, Nr. 2 und Nr. 3 sind gemeinfrei. Aus der Hüllkurve der Amplituden- und Frequenzmodulation lassen sich die Zeitkonstanten von Horn und Trommel grob schätzen. Bei Nr. 2 und 3 muss offen bleiben, ob sie echt sind, wegen der Kategorie „Synthetic sounds“.
- **Für den Leslie-Filter** ist Nr. 10 (CC BY 4.0) das stärkste Material: gepaart trocken und nass, Horn und Trommel getrennt, ein echter Leslie 145. Nur die Rotorstellung ist nicht dokumentiert.
- **Für den Tonrad-Generator** gibt es kein freies Messmaterial mit bekannter Registrierung. Nr. 8 (M100-Drones mit Motor-Auslauf, CC BY 4.0) hilft beim Spektrum und bei der Motordynamik, nicht aber bei einzelnen Zugriegeln. Saubere Referenzen werden wohl nur eigene Aufnahmen liefern: je Zugriegel einzeln, direkt aus dem Vorverstärker, chromatisch, mit Klick, Perkussion und Vibrato getrennt – oder eine Anfrage an den Autor von HamOrg wegen einer Lizenz.
- **Zu meiden für LumiViz:** Philharmonia, NSynth (Töne aus kommerziellen Bibliotheken, 16 kHz) und das IR von setBfree, falls ShareAlike unerwünscht ist.
- **Vor einer Übernahme** die Lizenz bei allen 12 hammondman-Sounds einzeln prüfen und die Lizenzzeilen von Hand an der Quelle gegenlesen.

## 5. Nebenquellen der Websuche

Nur Zusammenfassungen, keine Belege:

- Yamaha Organ Session
- Hammr+
- Deep Fried Hammond (SOS-Rezension 2002: „copyright-free“; die heutige Lizenz habe ich nicht geprüft)
- Gospel Drawbars
- Organimation

Alle sind kommerziell und nicht übernehmbar.
