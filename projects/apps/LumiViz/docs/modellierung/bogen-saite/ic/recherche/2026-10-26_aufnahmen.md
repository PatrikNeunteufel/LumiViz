# Recherche IC1: Bogen-Saite — freie Aufnahmen und Messdaten

> **Auftrag:** Klasse 3 (Violine, Viola, Cello, Kontrabass, Streicher-Ensemble): freie Aufnahmen und Messdaten mit an
> der Quelle geprüfter Lizenz, auch Messdaten mit Bogenparametern.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [S] = vom eigenen Skript ausgegebener Rohtext/Feldinhalt; [W] = vom Abrufwerkzeug als Zitat/Feldinhalt;
> [Z] = nur Zusammenfassung.
> **Reichweite:** im Bericht selbst (10 Suchen, 21 Abrufe, rund 70 Lese-Abrufe über Skripte; web.archive.org,
> Freesound-Suche, URMP-PDF nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../aufnahmen.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../aufnahmen.md`, `../messplan.md`, `../offene-fragen.md`.

---

# Bericht IC, Klasse 3 „Bogen-Saite“: freie Aufnahmen und Messdaten mit an der Quelle geprüfter Lizenz

**Auftrag:** Für Violine, Viola, Cello, Kontrabass und Streicher-Ensemble freie Aufnahmen und Messdaten suchen, gegen die sich ein gerechnetes Modell messen lässt, und je Datensatz die Lizenz an der Quelle prüfen.
**Datum:** 2026-10-26 (alle Abrufe an diesem Tag)

**Reichweite:**
- 10 Websuchen und 21 WebFetch-Abrufe.
- Rund 70 HTTP-Abrufe über eigene Skripte, die nur gelesen und nichts gespeichert haben (abgesehen von Zwischenablagen im Scratchpad):
  - GitHub-API für VCSL und VSCO-2-CE (Dateibäume, LICENSE, README),
  - 11 Seiten der University of Iowa und 9 HEAD-Abfragen für Zip-Größen,
  - die Zenodo-API mit etwa 25 Datensätzen, 6 Stichwortsuchen und 3 Metadatendateien (TinySOL_metadata.csv, OrchideaSOL_metadata.csv, Good-Sounds sounds.json, nur gelesen),
  - die Dryad-API für URMP,
  - depositonce der TU Berlin für TU-Note und die Seite des Philharmonia Orchestra.
- **Gesperrt oder nicht erreichbar:**
  - Zenodo lieferte über curl 403. Über WebFetch und Python-urllib ohne Browser-User-Agent ging es.
  - web.archive.org ist für WebFetch gesperrt, deshalb ist die Sonatina-Originalseite nicht gelesen.
  - Die Freesound-Suche lieferte einen Fehler bzw. unbrauchbare Treffer.
  - Das PDF-Dokument von URMP ist nicht gelesen.

**Kennzeichnung:**
- **[S]** = wörtlich, selbst gelesen: Rohtext oder Feld, das mein eigenes Skript ausgegeben hat.
- **[W]** = wörtlich, Werkzeug: von WebFetch als Zitat oder Feldinhalt geliefert.
- **[Z]** = nur Zusammenfassung, kein Beleg.

---

## 1. Datensätze im Einzelnen

### 1.1 University of Iowa MIS (theremin.music.uiowa.edu)

**Lizenz:** „may be downloaded and used for any projects, without restrictions.“ [W, MIS.html]. Die VPO-Seite zitiert denselben Wortlaut [W].

**Raum:** „all of the instruments were recorded in an anechoic chamber“ [W, MIS.html]. Jede Streicherseite trägt außerdem „Anechoic Chamber … 5 feet“ [S].

**Alte Sätze (16-bit, 44.1 kHz, mono):**

| Instrument | Mikrofon und Kette [S] | Spieler/Datum [S] | Dateien [S] |
|---|---|---|---|
| Violine | „Neumann KM 84 Mackie 1402-VLZ Panasonic SV-3800 DAT 16-bit, 44.1 kHz, mono“ | Josue Jean-Francois | 71 Einträge, Summe ≈ 345,6 MB |
| Viola | „Neumann KM 84 Cardioid Alexis XT20 ADAT 16-bit, 44.1 kHz, mono“ | Charletta Taylor, „March 2, 1996“ | 62 Einträge, ≈ 158,4 MB |
| Cello | KM 84 | Jean Montes; Etikett „Antonius Stradivarius … 1736-Germany“, also eine Kopie | 79 Einträge, ≈ 377,6 MB |
| Kontrabass | KM 84 | „Wurlitzer Upright Double Bass“, Alison Mann, „November 20, 2001“ | 71 Einträge, ≈ 222,9 MB |

**2012er-Sätze:**
- Kette: „Earthworks QTC40 Metric Halo … 16-bit, 44kHz and 24-bit, 96 kHz stereo and mono files available in zip format“ [S].
- Violine: Nicolai Tambovsky (1998), gespielt von Leonid Iogansen, 12-11-2011 [S].
- Viola: Anton Krutz (2001), Manuel Tabora [S].
- Cello: Charles Quenoil (1923), Yoo-Jung Chang [S].
- Kontrabass: Anton Krutz (2004), Mike White [S].
- Zip-Größen nach HEAD (24/96) [S]:

| Instrument | arco stereo | arco mono | pizz mono |
|---|---|---|---|
| Violine | 652 MB | 126 MB | 52 MB |
| Cello | 953 MB | 475 MB | 202 MB |
| Viola | 1044 MB | 514 MB | 336 MB |
| Kontrabass | nicht abgefragt | nicht abgefragt | nicht abgefragt |

**Inhalt [S, aus den Dateinamen]:**
- Nur **arco** und **pizz**, je **pp/mf/ff**, je **Saite** (Violine sulG–sulE, Viola und Cello sulC–sulA, Kontrabass sulE–sulG, 2012 zusätzlich sulC).
- Beispiel: „Violin.arco.pp.sulG.G3B3.aiff (4.1mb)“, 2012 „Bass.arco.sulC.pp.C1Eb1.stereo.aif“.
- Jede Datei enthält eine **Tonfolge eines Bereichs** auf einer Saite, Einzeltöne müssen also geschnitten werden.
- Vibrato, Sordino, Tremolo und Spiccato tauchen nicht auf. Die Suche nach „vib“ und „sord“ traf nur „Vibraphone“ [S]. Ob mit Vibrato gespielt wurde, steht nicht da.

**Eignung:** sehr gut für den Klangvergleich von Einzeltönen, reflexionsarm, drei Stärken, Saite bekannt. Vibrato ist nicht getrennt.

### 1.2 VSCO 2 Community Edition (GitHub sgossner/VSCO-2-CE)

**Lizenz:** Die LICENSE-Datei beginnt mit „CC0 1.0 Universal / Statement of Purpose“ [S]. GitHub zeigt „CC0-1.0 license“ [W].

**Streicher-Ordner [S, Git-Baum]** (Ordner, Dateien, MB):

| Bereich | Ordner |
|---|---|
| Solo Violin | Arco Vib 30 / 74,4 · Pizz 44 / 13,7 · Trem 27 / 34,0 · spic 60 / 13,4 |
| Solo Contrabass | **SusNV 28 / 52,8 · SusVib 26 / 48,0** · Pizz 40 / 20,8 · Spic 42 / 13,0 · Trem 16 / 21,3 |
| Violin Section | susVib 22 / 46,0 · Pizz 44 · Spic 44 · Trem 21 |
| Viola Section | susvib 26 / 71,7 · pizz 46 · spic 48 · trem 24 |
| Cello Section | susvib 27 / 72,6 · pizzT 52 · spic 52 · trem 25 |

- Es gibt keine Solo-Viola und kein Solo-Cello.
- Stärken aus den Dateinamen: „LLVln_ArcoVib_A3_f.wav / …_p.wav“, also zwei Stufen; „BKCtbss_SusNV_A#0_v1_rr1.wav / _v3_“, also Lagen v1/v3 [S].
- Raum, Mikrofone und Abtastrate sind nicht gelesen.

**Eignung:**
- Kontrabass: **Vibrato und Non-Vibrato getrennt**, das ist selten.
- Für das Ensemble die Section-Sätze.
- Die Solo-Violine gibt es nur mit Vibrato.

### 1.3 VCSL (GitHub sgossner/VCSL)

**Lizenz:** „This collection is under a Creative Commons 0 license.“ [W]

**Inhalt:** Unter Chordophones stehen nur Harfen, Klaviere, Cembali, Strumstick, Dan Tranh und „Psaltery, Bowed and Plucked“ [S, ganzer Baum, nicht abgeschnitten]. **Keine Violinfamilie.** Ein gestrichenes Psalterium ist für Klasse 3 nur am Rand brauchbar.

### 1.4 TinySOL (Zenodo 3685367, v6.0, 2020-01-31)

**Lizenz:**
- API-Feld `license` = {'id': 'cc-by-4.0'} [S].
- Beschreibung: „TinySOL can be used for creative purposes insofar at the use complies with the Creative Commons Attribution 4.0 International license“ [S].

**Inhalt [S, Metadaten-CSV]:**

| Instrument | Töne | Stärken |
|---|---|---|
| Violine | 284 | pp 95 / mf 95 / ff 94 |
| Viola | 309 | pp/mf/ff je 103 |
| Cello | 291 | pp/mf/ff |
| Kontrabass | 309 | pp/mf/ff je 103 |

- Alle Töne nur „ordinario“, ohne Dämpfer: „purely consists of sounds played in the so-called "ordinary" style, and in absence of mute“ [S].
- Die Saite ist je Ton angegeben („String ID“ 1–4) [S].
- Aufgenommen bei Ircam 1996–1999 [S]. „WAV files, sampled at 44.1 kHz“ [S]; mono und 16 Bit [W].
- TinySOL.tar.gz ist 1027 MB groß [S].
- Raum und Mikrofon sind nicht angegeben.

**Eignung:** gut, echte Einzeltöne mit drei Stärken und bekannter Saite. Der Raum ist unbekannt (Studio).

### 1.5 OrchideaSOL (Zenodo 3686252)

**Lizenz:**
- API-Feld cc-by-4.0 [S], aber: „The OrchideaSOL audio data can be used for creative purposes insofar at the use complies with the Ircam Forum License“ und „The OrchideaSOL metadata can be used … Creative Commons Attribution 4.0“ [S].
- Die Forum-Lizenz besagt: „Le droit d'utilisation porte sur une utilisation non commerciale.“ [W, forum.ircam.fr/legal/contrat-de-licence-forum-ircam]. Die Seite regelt nach Werkzeugangabe eigentlich Software [Z].

**Inhalt [S, Metadaten-CSV]:** Violine 1987, Viola 1952, Cello 1593, Kontrabass 1636 Töne.
- Techniken: ordinario, tremolo, artificial_harmonic(_tremolo), col_legno_battuto, col_legno_tratto, pizzicato_bartok, pizzicato_l_vib, pizzicato_secco, sforzato, sul_ponticello(_tremolo), sul_tasto_tremolo (nicht bei der Violine), behind_the_bridge.
- Dämpfer: Sordina (Violine 408 Töne), Piombo (Violine 82, Viola 81).
- Stärken: pp/mf/ff, dazu f und fp. Saite je Ton.

**Eignung:** inhaltlich die vollständigste Spielweisen-Sammlung. Das Audio ist **nicht frei**.

### 1.6 Good-Sounds (Zenodo 4588740, v1.1, 2017-06-29)

**Lizenz widersprüchlich, an der Quelle bestätigt:**
- API-Feld {'id': 'cc-by-4.0'} [S].
- Beschreibung: „This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License“ [S].

**Inhalt [S, sounds.json]:**
- Violine 1383 Töne (Spielerinnen laia, raquel), Cello 948 (margarita, nico), Kontrabass 159 (bow/pizz).
- Beim Cello gibt es **qualitative Bogenangaben**: bow_velocity slow/medium/fast (je 74) und bridge_position ponticello/tasto/middle (78/72/72), Stärken p/mf/f und Crescendo-Formen.
- Qualitätsklassen wie „good-sound“, „bad-attack“ usw.
- „separate mono .flac audio file of 48kHz and 32 bits“; „recorded in the Universitat Pompeu Fabra / Phonos recording studio“ [W]; 1–4 Mikrofone [W].
- good-sounds.zip ist 13,9 GB groß [W].
- Eine neuere Fassung soll bei Dataverse liegen (10.34810/data2314) [Z, ungeprüft].

**Eignung:** Cello mit Kontaktstelle und Bogengeschwindigkeit als Klassen ist für die Messung interessant. Die NC-Klausel hat Vorrang.

### 1.7 Vibrato Analysis Dataset (Zenodo 597285/182244) und Vibrato Dataset (Zenodo 1016050)

**Lizenz:** API-Feld {'id': 'apgl-v3'} [S], eine Softwarelizenz.
- Teilsatz von Freesound Carlos_Vaquero: „Violin, violoncello and transverse flute recordings were downloaded and used for non commercial purposes under creative commons license“ [S].

**Inhalt [S]:**
- MTG-Violine: „Single notes of two octaves and four different melodies … wav format sampled at 44.1 kHz. Each sound separated by semitones is recorded four times for no vibrato,slow, standard and fast rates of vibrato“.
- Dazu Annotationen als CSV (Ableitung der Tonhöhe).
- 229 MB (597285) bzw. 244 MB (1016050); 1016050 enthält „295 note tracks with vibratos and corresponding non vibrato tones“.

**Eignung:** sehr gut für Vibrato, weil Vibrato und Non-Vibrato getrennt und in Raten gestuft sind. Die Lizenzlage ist unklar und gemischt.

### 1.8 CNSM Dataset (Zenodo 18696785, v1.0.0, 2026-02-19)

**Lizenz:** API-Feld {'id': 'cc-by-4.0'} [S].
- Datei dataset.zip, 654 MB [S]. Die Beschreibung ist leer [S]. Die readme.md im GitHub-Repo hugo-paugesteros/CNSM-dataset hat 0 Byte [S].

**Inhalt laut zugehörigem Paper arXiv 2605.18305 [W]:**
- Drei Violinen.
- Eingangsadmittanz am Steg: „Laser Doppler Vibrometer (Polytec OFV-505/OFV-3001)“ und „impact hammer (PCB 086E80)“.
- Aufnahmen „in a large dry room“ mit einem „stereo pair of DPA 4010 omnidirectional microphones“, 48 kHz, gespielt wurden „scales and melodic phrases“.
- Bewegungserfassung: „OptiTrack Motive system comprising 12 cameras operating at 120 Hz“ mit Markern an Geige und Bogen. Abgeleitet werden Bogengeschwindigkeit, „bow-bridge distance“ und der Abstand zwischen Haar und Saite.
- **Bogenkraft wird nicht genannt.**

**Eignung:** stark für den Vergleich Admittanz → Klang, mit Bogenposition und -geschwindigkeit. Es gibt keine Einzeltöne, keine Bogenkraft und keine Stärkestufen.

### 1.9 mdw Wien, Lampis: gestrichene Cellosaiten mit Roboterarm (alle CC BY 4.0, Felder [S])

- **17749111 / 17782542** „A collection of bowed responses of cello strings (part 1/2)“, Teil 1 von 8:
  - „robotic bowing apparatus consisting of a UR5e robotic arm mounted on a monochord“.
  - Cello-G2-Saiten. „All signals were sampled at 50 kHz.“
  - „relative bow-bridge distance β from 0.02 to 0.2 and a bow force Fb from 0.1 to 4 N“, vb = 0.05/0.1/0.2 m/s.
  - whole.csv mit „bow force in N, bow velocity in m/s, bridge force in N, and nut force in N“.
  - Vier Saitentypen A–D.
  - Teil 1 sind drei 7z-Archive von je etwa 14–15 GB [S].
- **13374477** Steg-Kraftverläufe beim Anstreichen (Einschwingvorgänge, verschiedene Bogenkräfte und -beschleunigungen), 3 GB [S].
- **10619541** Steg mit Kraftsensor, statische und dynamische Stegkraft, Roboterarm, 1,3 GB [S].
- **15879128** rekonstruierte Reibkraft am Kontaktpunkt, 47 MB [S].

**Eignung:** **die beste gefundene Quelle für Bogenparameter.** Schelleng-Diagramme mit gemessener Kraft, Geschwindigkeit und β. Es ist eine Saite auf dem Monochord, kein Korpus und kein Abstrahlschall.

### 1.10 mdw Wien, Pàmies-Vilà: Cello mit Bewegungserfassung und Audio (alle CC BY 4.0, Felder [S])

- **17207713**, zeitgenössische Bogentechniken (unter anderem col legno tratto):
  - Die ReadMe [S] nennt sechs Cellisten auf den vier leeren Saiten (A3, D3, G2, C2), „OptiTrack Prime 13 … Fs = 240 fps“ mit Markern MTip, MFrog, MBow_A/B/C, MNut und MBridge.
  - „Sound WAV-file (Fs = 50000 Hz) … ROGA Instruments RG50 at 45 cm in front … AKG microphone around 2 meters away“.
  - Aufnahmeort mdw Wien, Mai 2023. Zusammen etwa 820 MB.
- **17201350** Bogenwechsel-Übungen auf leeren Saiten, sechs Cellisten.
- **10696680** Bogenkinematik von Spieler und Roboter mit Audio (44.1 kHz) und Bewegungsdaten (240 fps).

**Eignung:** Bogenkinematik mit Audio. Die Bogenkraft ist nicht gemessen, soweit es in der ReadMe steht.

### 1.11 TU-Note Violin Sample Library (TU Berlin depositonce, 2018; von Coler, Margraf, Schuladen)

Nicht zu verwechseln mit dem ausgeschlossenen „TU Berlin Anechoic“.

**Lizenz:** „dc.rights.uri https://creativecommons.org/licenses/by-nd/4.0/“ [S].

**Inhalt [S, Abstract]:**
- „single sounds which cover the entire frequency range of the instrument in four dynamic levels, two-note sequences for the study of note transitions, and solo pieces and scales“.
- „hand-labeled segmentation ground-truth files“.
- „Recordings took place in an anechoic chamber … using two microphone positions“.
- TU-Note_Violin.zip, 1,49 GB [S].

**Eignung:** sehr gut. Reflexionsarm, vier Stärken und als **einzige Quelle Legato-Übergänge zwischen zwei Tönen**, mit Segmentierung.

### 1.12 Seven Violin Dataset (Zenodo 3478442)

- API-Feld cc-by-4.0 [S].
- Sieben Töne (F5–Gb6) einer Violine, „recorded in anechoic condictions“, 3,2 MB [S].
- Eignung: nur Stichprobe.

### 1.13 Philharmonia Orchestra Sound Samples (philharmonia.co.uk/resources/sound-samples/)

**Lizenz [S]:** „You are free to use these samples as you wish, including releasing them as part of a commercial work. The only restriction is that they must not be sold or made available ‘as is’ (i.e. as samples or as a sampler instrument).“

**Inhalt:** Es gibt einen Download „Strings: Download strings samples“ [S]. Spielweisen, Stärken und Format sind auf der Seite nicht genannt [W]. Das Archiv ist nicht geöffnet.

### 1.14 URMP (Rochester)

**Lizenz:** Dryad-API: „license: https://spdx.org/licenses/CC0-1.0.html“, storageSize 12 097 841 495 Byte, veröffentlicht 2019-03-20 [S].

**Inhalt:** 44 Stücke (11 Duette, 12 Trios, 14 Quartette, 7 Quintette), jede Stimme einzeln aufgenommen, mit MIDI und Tonannotation [W, Projektseite]. Welche Streicher dabei sind, ist an der Seite nicht gelesen [Z].

**Eignung:** Phrasen und Ensemble-Stimmen, keine Einzeltöne, kein reflexionsarmer Raum.

### 1.15 Weitere Funde

- **PHENICX-Anechoic (Zenodo 840025):** Die Annotationen sind cc-by-4.0 [S]. Für das Audio gilt: „For the intellectual rights and the distribution policy of the audio recordings … contact Aalto University“ [S]. Gemeint sind reflexionsarme Orchesteraufnahmen. Lizenz ungeklärt.
- **Virtual Playing Orchestra:**
  - „I enforce no restrictions on using this library to make music, even for commercial purposes“, aber „I do not feel it is right to repackage and sell this library“ [W].
  - Enthält Teile unter „Creative Commons Sampling Plus 1.0“ (Sonatina), „CC BY-SA 3.0“ (Westlund) und „CC BY-SA 4.0“ (No Budget Orchestra) [W].
  - Eine Mischquelle, für die Messung ungeeignet.
- **Sonatina Symphonic Orchestra:** „Creative Commons Sampling Plus 1.0“ nur über das Zitat auf der VPO-Seite [W] und eine Suchzusammenfassung [Z]. Die Originalseite ist nicht erreichbar.
- **Dalmazzo/Ramírez Bogentechniken (PMC6409498):** Myo-Armband als IMU am Unterarm mit Audio, sieben Stricharten (Détaché bis Bariolage) [W]. Daten auf GitHub Dazzid/DataToRepovizz [W]. Keine Datensatzlizenz genannt [W]. Keine Bogenkraft.
- **Medley-solos-DB:** CC BY 4.0, nur Violine, Ausschnitte [Z].
- **Bach10:** Nichtkommerzielle Lizenzen nur über Ableitungen belegt [Z].
- **Bach Violin Dataset (Zenodo 6050245):** „other-open“, aus kommerziellen Aufnahmen [S]. Ungeeignet.

---

## 2. Tabelle je Instrument

| Instrument | Bester Datensatz | Lizenz | Eignung |
|---|---|---|---|
| Violine | Iowa MIS 2012 (reflexionsarm, arco/pizz, pp/mf/ff, je Saite, 24/96) | „without restrictions“ | Einzeltöne nach Schnitt; Vibrato nicht getrennt |
| Violine, Legato und Stärken | TU-Note Violin (reflexionsarm, 4 Stärken, Zweitonfolgen) | CC BY-ND 4.0 | Übergänge und Einschwingen; keine Bearbeitung weitergeben |
| Violine, Vibrato | MTG Vibrato Dataset (ohne, langsam, normal, schnell) | Feld „apgl-v3“; Teilsatz nicht kommerziell | Vibrato getrennt; Lizenz unklar |
| Violine, Bogen und Korpus | CNSM (Admittanz, Bewegungsdaten, Audio) | cc-by-4.0 | Bogenposition, -geschwindigkeit und β, keine Kraft |
| Viola | Iowa MIS 2012; TinySOL (309 Töne) | frei / CC BY 4.0 | Einzeltöne, pp/mf/ff, Saite |
| Cello | Iowa MIS 2012; TinySOL (291 Töne) | frei / CC BY 4.0 | Einzeltöne |
| Cello, Bogenparameter | Lampis, mdw (17749111 ff.) | cc-by-4.0 | Kraft, Geschwindigkeit, β und Stegkraft gemessen (Monochord) |
| Cello, Kinematik | Pàmies-Vilà, mdw (17207713, 17201350) | cc-by-4.0 | Bewegung und Audio (50 kHz), leere Saiten, col legno |
| Kontrabass | Iowa MIS; VSCO 2 CE Solo Contrabass (SusNV / SusVib) | frei / CC0 | Vibrato getrennt (VSCO), drei Stärken (Iowa) |
| Ensemble | VSCO 2 CE Sections (Violinen, Viola, Cello) | CC0 | sus mit Vibrato, pizz, spic, trem; zwei Stärken |
| Ensemble, Kammermusik | URMP | CC0 (Dryad) | Phrasen in Einzelspuren |
| Spielweisen (alle vier) | OrchideaSOL | Audio unter Ircam-Forum-Lizenz, nicht kommerziell | Referenz für die Planung, nicht übernehmbar |

## 3. Tabelle aller Datensätze

| Datensatz | Fundort | Lizenz (Wortlaut, Ort) | Inhalt | Größe | Eignung | Übernahme nach LumiViz |
|---|---|---|---|---|---|---|
| Iowa MIS (alt + 2012) | theremin.music.uiowa.edu | „may be downloaded and used for any projects, without restrictions“ (MIS.html) [W] | Vl, Va, Vc, Kb; arco/pizz; pp/mf/ff; je Saite; reflexionsarm | alt 0,16–0,38 GB je Instrument; 2012 24/96 bis 1 GB je Zip | hoch | frei |
| VSCO 2 CE | github.com/sgossner/VSCO-2-CE | LICENSE „CC0 1.0 Universal“ [S] | Solo-Vl, Solo-Kb, Sektionen Vl/Va/Vc; sus(vib/NV), pizz, spic, trem; p/f bzw. v1/v3 | Streicher ≈ 0,7 GB | mittel (Kb-Vibrato getrennt) | frei |
| VCSL | github.com/sgossner/VCSL | „Creative Commons 0“ [W] | keine Violinfamilie | – | keine | (frei) |
| TinySOL | Zenodo 3685367 | Feld cc-by-4.0; „Creative Commons Attribution 4.0 International license“ [S] | Vl 284, Va 309, Vc 291, Kb 309; ordinario; pp/mf/ff; Saite | 1,03 GB | hoch | Namensnennung |
| OrchideaSOL | Zenodo 3686252 | „audio data … Ircam Forum License“ [S]; „utilisation non commerciale“ [W] | 13 Techniken, Sordina, Piombo | Metadaten 1,9 MB | hoch (inhaltlich) | nicht |
| Good-Sounds v1.1 | Zenodo 4588740 | Feld cc-by-4.0 gegen „Attribution-NonCommercial 4.0“ [S] | Vl 1383, Vc 948 (Bogengeschwindigkeit und Kontaktstelle als Klassen), Kb 159 | 13,9 GB | mittel | nicht (NC) |
| Vibrato-Datensätze | Zenodo 597285 / 1016050 | Feld „apgl-v3“; Teil „non commercial“ [S] | Vl-Töne ohne/langsam/normal/schnell vibrato; Vc (Freesound) | 0,23 / 0,24 GB | hoch für Vibrato | ungeklärt, eher nicht |
| CNSM | Zenodo 18696785 | Feld cc-by-4.0 [S] | 3 Vl: Admittanz (LDV), Bewegung 120 Hz, Audio 48 kHz, trockener Raum | 0,65 GB | hoch für Korpus und Bogen | Namensnennung |
| Lampis, Saitenantworten | Zenodo 17749111, 17782542 (+6 Teile) | Feld cc-by-4.0 [S] | Roboterbogen, Fb, vb, Steg- und Sattelkraft, 50 kHz | ≈ 44 GB je Teil | sehr hoch (Bogenmodell) | Namensnennung |
| Lampis, Stegkraft und Anstrich | Zenodo 13374477, 10619541, 15879128 | Feld cc-by-4.0 [S] | Einschwingvorgänge, Sensorsteg, Reibkraft | 3 / 1,3 / 0,05 GB | sehr hoch | Namensnennung |
| Pàmies-Vilà, Cello | Zenodo 17207713, 17201350, 10696680 | Feld cc-by-4.0 [S] | Bewegung 240 fps, Audio 50 kHz, leere Saiten, Spieltechniken | ≈ 0,8 GB u. a. | mittel | Namensnennung |
| TU-Note Violin | depositonce 11303/7527 | dc.rights.uri „…/by-nd/4.0/“ [S] | reflexionsarm, 4 Stärken, Zweitonfolgen, Stücke, Segmentierung | 1,49 GB | sehr hoch | Namensnennung, nur unverändert |
| Seven Violin | Zenodo 3478442 | Feld cc-by-4.0 [S] | 7 Töne, reflexionsarm | 3,2 MB | gering | Namensnennung |
| Philharmonia | philharmonia.co.uk | „must not be sold or made available ‘as is’“ [S] | Streicher-Paket, Inhalt nicht geprüft | ? | mittel | nur als interne Messreferenz, nicht als Rohdaten (siehe 5) |
| URMP | Dryad 10.5061/dryad.ng3r749 | „CC0-1.0“ (Dryad-API) [S] | 44 Stücke, Einzelspuren | 12,1 GB | gering bis mittel | frei |
| PHENICX-Anechoic | Zenodo 840025 | Annotationen cc-by-4.0; Audio „contact Aalto University“ [S] | reflexionsarme Orchesterstimmen | 0,76 GB | mittel | Audio ungeklärt |
| VPO | virtualplaying.com | Mischung aus Sampling Plus, BY-SA 3.0/4.0, CC0 [W] | Mischbibliothek | ? | gering | ShareAlike / gemischt |
| Sonatina | – (nicht erreichbar) | „Sampling Plus 1.0“ nur [W] über VPO | – | – | gering | ungeprüft |
| Dalmazzo, Myo | GitHub Dazzid/DataToRepovizz | keine Lizenz genannt [W] | IMU am Unterarm, Audio, 7 Stricharten | ? | gering | nicht (ohne Lizenz) |

## 4. Nicht gefunden

- **Veröffentlichte Rohdaten von Demoucron/Schoonderwaldt** (KTH, IRCAM), **Maestre und Marchini** (UPF, Quartet/repovizz) mit Bogenkraft und Audio. Gefunden wurden nur Papers. Die „Bowstroke Database“ (Young, NIME 2007) steht auf Zenodo 1179481 nur als Paper; ob die Daten erreichbar sind, ist nicht geprüft.
- **Spieldaten mit gemessener Bogenkraft und Abstrahlschall am Instrument.** Bogenkraft gibt es nur bei Lampis, und dort auf dem Monochord.
- **Solo-Viola und Solo-Cello in VSCO 2 CE.**
- **Streicher in VCSL.**
- **Freesound-Pakete von Streichern mit Lizenz je Paket.** Die Suche lieferte einen Fehler; auf der ersten Paketseite von Carlos_Vaquero ist kein Streicherpaket zu sehen.
- **MusOpen.** Es gibt nur alte Sekundärberichte [Z]; die Lizenz ist nicht an der Quelle gelesen.
- **URMP-Instrumentliste und Lizenz von Bach10** an der Quelle.
- **Sordino, Spiccato und col legno in einer frei lizenzierten Einzeltonsammlung.** Diese Spielweisen gibt es nur in OrchideaSOL (nicht frei). Spiccato und Tremolo gibt es nur in VSCO (CC0).

Kein Treffer ist kein Beweis.

## 5. Eigene Einschätzung (kein Beleg)

- **Messkern:**
  - Einzeltöne: Iowa MIS 2012 (frei, reflexionsarm, 24/96) zusammen mit TinySOL (CC BY).
  - Übergänge und Stärken der Violine: TU-Note. ND hindert die interne Messung nicht, schließt aber eine bearbeitete Weitergabe in LumiViz aus.
  - Bogen-Saite-Physik: die Lampis-Reihe der mdw. Mit Schelleng-Diagrammen, gemessener Kraft, Geschwindigkeit und β sowie Steg- und Sattelkraft passt sie genau auf ein Reibungs- und Saitenmodell.
  - Korpusübertragung der Violine: CNSM (Admittanz und Klang von drei Violinen).
- **Vibrato:** Getrennt gibt es es frei nur beim Kontrabass in VSCO. Für die Violine ist der MTG-Datensatz inhaltlich passend, rechtlich aber unsauber („apgl-v3“ als Softwarelizenz, Teilsatz nicht kommerziell). Für die Übernahme besser nicht verwenden, nur intern ansehen. Dass das zulässig ist, ist eine Annahme.
- **Good-Sounds:** Wegen des NC-Satzes in der Beschreibung ist es trotz des Feldes cc-by-4.0 als nicht kommerziell zu behandeln, denn der strengere Wortlaut gilt.
- **Philharmonia:** Rohsamples in LumiViz mitzuliefern dürfte unter „made available ‘as is’“ fallen. Für die Messung im Entwicklungsprozess ist die Sammlung wohl unbedenklich, das ist eine Deutung.
- **Größen:** Die Lampis-Teile (≈ 44 GB je Teil, 8 Teile) sind große Aufnahmen und gehören nach README außerhalb des Repos.
- **Lücke für ein Modell mit Bogenparametern:** Es fehlt ein Instrument im Spiel mit gemessener Bogenkraft und Mikrofon. Es könnte sich lohnen, bei der mdw nachzufragen (Pàmies-Vilà, Lampis) oder bei Mores (Bogenpendel Hamburg, Zenodo 14646880, nur Videos und Paper, CC BY 4.0).
