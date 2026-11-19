> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-19_35_oud_basschor_einzeltoene` (gestellt in LumiViz S207 nach KQ37 a), 1 Segment: 2026-11-19 12:57–13:08, claude-opus-5-5, 102 Runden, success, 4.31 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S208** — nur dieser Kopf ist dazugekommen. Die Hilfsskripte des Laufs liegen im Ordner gleichen Namens, die PDFs unter `…\cmake\quellen\gezupfte-saite\` (`UMZUG_S208.tsv`).
>
> **Gegengelesen in S208** (Değirmenli 2023 S. 527–531 am Seitenbild, Abb. 3 neu in 300 dpi gerendert; Textebene mit `pdftotext`; D'Addario als Text aus dem abgelegten HTML; Modellwerte aus `asset/loop/instrument_bodies/oud.lvinstbody`):
> - **Abb. 1 (S. 527) — stimmt:** alle acht Kopfzeilen am Bild gelesen (72,700 Hz 3,316 % … 348,769 Hz 1,398 %), Saiten gestimmt und mit Schaumstoff gedämpft (Textebene S. 527). Die Formen sind wie beschrieben; **die Lage der Bäuche gegen den Steg ist am Bild nicht bezifferbar** (Steg nicht eingezeichnet, kein Maßstab, die Ansicht ist nicht sicher maßhaltig) — „Bauch im Stegfeld“ ist eine Einschätzung, der Abstand zum Modell (208,8 Hz mit 28 % am Steg) lässt sich an diesem Bild nicht prüfen.
> - **Abb. 3 (S. 531) — Zuordnung stimmt, die Legende ist eindeutig:** in 300 dpi stehen in der Legende rot / blau / grün = „2. / 6. / 7. Ud Ses Yayınım“ (Abstrahlung) und orange / gelb / schwarz mit Punkt = „Mobilite“. Der Bericht hatte die Zuordnung als unsicher vermerkt.
> - **Ablesung Abb. 3 — Lage stimmt, einzelne Werte bis 7 dB daneben:** nachgelesen bei 300 dpi: Spitze 125–140 Hz (Ud 2 rund 92), Tal Ud 6 57 dB bei 155 Hz, Maximum 170–225 Hz (96–103), Tal um 245 Hz — wie im Bericht. C2 T2 (131 Hz) Ud 6 rund 91 statt 87; D3 T1 (147 Hz) rund 87 / 84 / 87 statt 80 / 76 / 82. Die Aussage „C2 T2 und T3 liegen auf den zwei stärksten Spitzen unter 250 Hz“ wird dadurch eher stärker.
> - **Wortlaut S. 529–531 — stimmt** (leere Saiten, Schaumstoff, Zupfapparat „sabit bir açıda ve hızda“, 30 cm; 44,1 kHz und „26 Bit“; „120 Hz ve 180 Hz - 220 Hz“; 700 Hz). **Ungenau:** Rode M3 und Audirol UA 25 nennt S. 530 im Satz zum Hörversuch („Dinleme testi aşamasında“), nicht ausdrücklich zur Aufnahme.
> - **D'Addario EJ95A — stimmt:** Zugtabelle, Durchmesser, „tuned CFADGC“, „Silver-Plated Copper“, „Round Wound“ im abgelegten HTML wörtlich gefunden; die Zuordnung der Durchmesser zu den Tönen bleibt eine Lesart der Reihenfolge.
> - **Gegen das Modell (eigene Rechnung S208):** Güte der Moden unter 300 Hz im Körper Q = 1/η 26 / 32 / 35 / 42 bei 99,9 / 168,6 / 207,8 / 269,4 Hz; bei Değirmenli (Sönüm als Dämpfungsgrad gelesen, Q = 1/(2ζ)) 25 / 64 / 15 / 45 bei 117,7 / 177,8 / 206,0 / 260,8 Hz. Verhältnis der zwei stärksten Spitzen der Abstrahlung unter 300 Hz: Değirmenli 2023 Ud 2 / 6 rund 1,6 (130 → 205 Hz), Değirmenli 2017 1,7, Modell 2,7 (Bericht S207 §8).
> - **Nicht gegengelesen:** Abb. 2 und 4; Ziyağil 2021, die übrigen DergiPark-Dateien, UBC, Arab Instruments, Freesound (nur als Text im Bericht).

# Bericht: Oud — Pegel des tiefen Chors gegen die übrigen, Moden unter 300 Hz

- **Auftrag:** `auftraege/2026-11-19_35_oud_basschor_einzeltoene.md`
- **Datum:** 2026-11-19
- **Modell:** claude-opus-5-5
- **Reichweite:** 31 Netzsuchen (`WebSearch`, alle „standard“); 23 rohe Abrufe (13 mit `hole.py`/urllib, 10 mit `curl`),
  davon 10 mit verwertbarem Inhalt. Alles Abgelegte liegt unter `abrufe/` mit Zeile in `abrufe/INDEX.md` (URL, Zeit,
  Größe, SHA-256). Kein `WebFetch` in diesem Lauf. Seitenbilder unter `seiten/`.
- **Nicht erreicht:** YÖK Açık Bilim (`acikbilim.yok.gov.tr`, Name nicht auflösbar, zweimal versucht) — dort läge die
  Dissertation Değirmenli 2018; tezara.org (HTTP 403); YÖK Tez Merkezi (nur die Suchmaske, die Suche braucht ein
  Formular, nicht weiter verfolgt); mikeouds.com (Name nicht auflösbar, wie in Bericht 25); emirdegirmenli.com (die von
  Abdallah 2026 zitierte Seite gibt HTTP 404); Acta Acustica (HTTP 403, nicht umgangen); drei Shopify-Händler und die
  Produktseite von D'Addario (HTTP 429 bei urllib; die Übersichtsseite kam mit `curl` durch).
- **Marken:** `[wörtlich]` = im Rohtext oder am Seitenbild der abgelegten Datei gelesen; `[nur Zusammenfassung]` =
  nur aus der Antwort der Suchmaschine; `[eigene Rechnung]`, `[eigene Einschätzung]`.

**Ergebnis vorab:** Eine Messung, die den Pegel des leeren C2-Chors einer Oud gegen die anderen Chöre angibt, ist
nicht gefunden. Neu gefunden ist **Değirmenli 2023** (İdil 104): acht gemessene Moden einer **fertigen, besaiteten**
Oud mit Frequenz, Dämpfung und Form, und **ungeglättete** Abstrahl- und Stegkurven von 50 Hz bis 1 kHz für drei
Ouds. Dort liegen der zweite und dritte Teilton von C2 (131 / 196 Hz) **auf** den zwei stärksten Spitzen unter 250 Hz,
nicht in einem Tal wie im Modell. Außerdem gefunden: eine Zugtabelle des Herstellers D'Addario für einen arabischen
Satz mit einzelnem C2.

---

## 1 Pegel einzelner Oud-Töne über den Umfang

**Antwort in einem Satz:** Zahlen zum Pegel je Ton oder je leerem Chor sind nicht gefunden; die einzige bekannte
Messreihe (leere Saiten von zehn Ouds, mechanisch gezupft, 30 cm) steht in der nicht erreichbaren Dissertation
Değirmenli 2018, und in der Arbeit von 2023 wird sie nur als Hörversuch ausgewertet.

| Quelle | Stelle | Angabe | Marke |
|---|---|---|---|
| Değirmenli 2023 (siehe Liste der Quellen) | S. 529, Abschnitt „Dinleme testi“ | Zuerst wurden die **leeren Saiten** aller zehn Ouds aufgenommen. Die Ouds waren im Gestell befestigt, alle Saiten außer der gemessenen mit Schaumstoff gedämpft. Ein Gerät zupft die Saite mit dem Mizrap „unter festem Winkel und fester Geschwindigkeit“. Mikrofon in **30 cm**. | [wörtlich] S. 529: „udların açık tellerinin sesleri kaydedilmiştir“, „ölçüm alınan teller dışındakiler sünger ile sönümlendirilmiştir“, „telin sabit bir açıda ve hızda mızrapla uyarılabilmesi için tasarlanmış“, „30 cm mesafedeki bir mikrofon“ |
| dito | S. 530 | Aufnahme 44,1 kHz, „26 Bit“ (so gedruckt), Rode M3, „Audirol UA 25“ (so gedruckt) | [wörtlich] |
| dito | S. 530, Abb. 2 | Ausgewertet nur als Hörversuch (Bradley-Terry-Luce): Rangfolge der zehn Ouds nach Lautheit und „Bass-Charakter“. **Keine Pegel je Saite**, kein Vergleich der Chöre | [wörtlich] am Seitenbild `seiten/Degirmenli_2023_TinisalNitelikler_Idil10_S0006_110dpi.png` |
| dito | Kopf von Abb. 1 und 2 | Quelle der Daten: „Değirmenli, 2018: 115, 116, 125“ — die Dissertation | [wörtlich] |
| Değirmenli 2018, Dissertation Gazi Üniversitesi („Türk müziği çalgılarından udda ses oluşumunun incelenmesi …“) | — | soll zehn spielbare Ouds, eine Tanbur und eine Bağlama auf Schwingung und Klang untersuchen; Volltext nicht erreicht | [nur Zusammenfassung] |
| UBC-Wiki, Studentenseite „Course:Phys341 2020/Oud“ | Abschnitt Acoustics | Keine Pegel je Ton (siehe Lücke 2) | [wörtlich], schwach |

**Datensätze mit Einzeltönen — Ladeliste (nichts geladen):**

| Name | Adresse | Größe | Lizenz | Inhalt | Eignung | Marke |
|---|---|---|---|---|---|---|
| Freesound-Paket „Arabic Oud“ (Nutzer tarane468, 27.12.2020) | https://freesound.org/people/tarane468/packs/30959/ | nicht angegeben | je Klang, auf der Paketseite nicht lesbar; Herunterladen nur mit Anmeldung | 10 Dateien „Oud Motif D1 / F#1 / A1 / D2 / F#2 / A2 / D3 / F#3 / A3 / C#4.wav“; Beschreibung: „I found this sound somewhere in the net as a free soun…“ | **ungeeignet**: Herkunft unbekannt, „Motif“ statt Einzelton, Namen der Töne passen zu keiner Oud-Stimmung (D1 liegt unter dem Umfang) | [wörtlich] `abrufe/freesound_pack_30959_arabic_oud.html` |
| Freesound, alle Pakete mit Schlagwort „oud“ | https://freesound.org/search/?q=&f=tag%3A%22oud%22&dp=1 | — | — | 25 Klänge in 8 Paketen; außer dem obigen nur Loops, Musikausschnitte (CompMusic) und Fremdes | keine Einzeltöne | [wörtlich] `abrufe/freesound_packs_tag_oud.html` |
| PCMIR (Persian Classical Music Instrument Recognition), Mendeley Data | https://data.mendeley.com/datasets/bpyxwkv9wj | nicht ermittelt | nicht ermittelt | sieben persische Instrumente, darunter „Ud“; ob Einzeltöne, unklar | nicht geprüft | [nur Zusammenfassung] |
| Özbek & Savacı, Instrumentenerkennung, IYTE | https://openaccess.iyte.edu.tr/bitstream/11147/9824/1/Classification_of_Turkish.pdf | — | — | 293 Aufnahmen, für Ud „1996“ Ausschnitte; ob veröffentlicht, unklar | Musikausschnitte, kein Pegel | [nur Zusammenfassung] |
| OTMM Makam Recognition Dataset (Zenodo 58408) | https://zenodo.org/records/58408 | — | CC BY-NC-SA 4.0 | nur Tonhöhenverläufe, **kein Ton** („unable to distribute the audio“) | ungeeignet | [nur Zusammenfassung] |
| TinySOL / OrchideaSOL, Good-sounds | Zenodo 3685367 / 3686252 | — | CC BY 4.0 / IRCAM Forum | Orchesterinstrumente; keine Oud erwähnt | ungeeignet | [nur Zusammenfassung] |

**Nicht gefunden — Suchwege:** „oud isolated notes dataset … zenodo“; „freesound oud single notes pack open strings“;
„oud dataset instrument recognition Arabic …“; „kaggle OR huggingface oud notes dataset“; „Turkish makam instruments
recognition dataset ud kanun ney“; „free oud sample library sfz CC0“; „oud sound power level … each note“; „Arabic oud
Turkish oud comparison spectral … open strings“; Türkisch „ud enstrümanı açık teller spektral analiz ses basınç düzeyi
kaba ses teli“, „frekans tepki anahtarı ud“. Freesound roh nach Schlagwort durchsucht. VSCO/VCSL und Philharmonia
führen nach den Suchergebnissen keine Oud [nur Zusammenfassung].

---

## 2 Moden und Abstrahlung einer gebauten Oud unter 400 Hz

**Antwort in einem Satz:** An einer fertigen, besaiteten türkischen Oud (Ud 2 bei Değirmenli) liegen unter 300 Hz
Moden bei 72,7 / 117,7 / 177,8 / 206,0 / 260,8 / 282,9 Hz, die Moden 117,7 und 206,0 Hz haben ihren einzigen Bauch im
Feld zwischen Rosette und Steg, und die Abstrahlung derselben Oud hat ihre zwei stärksten Spitzen unter 250 Hz bei
rund 130 und 180–220 Hz; welche davon die Luftmode ist, sagt die Quelle nicht.

### 2.1 Gemessene Moden (Değirmenli 2023, Abb. 1, S. 527)

Verfahren laut S. 527 [wörtlich]: B&K 3050-A-060, Pulse 7781-N6, Minihammer B&K 8204, Mini-Beschleunigungsaufnehmer
B&K 4517; Saiten gestimmt und mit Schaumstoff gedämpft („tüm teller bir parça sünger kullanılarak susturulmuştur“);
Bild der Stegmessung S. 528, Resim 1 (Hammer und Aufnehmer am Steg). Die Abbildung ist übernommen aus „Değirmenli, 2018:
125“. Werte am Seitenbild gelesen und an einem Ausschnitt in 130 dpi bestätigt
(`seiten/Degirmenli_2023_TinisalNitelikler_Idil10_S0003_110dpi.png`, `seiten/Degirmenli_2023_S527_Abb1_oben_130dpi-03.png`).

| Mode (Kopf des Bildes) | Frequenz | „Sönüm“ | Güte Q = 1/(2ζ) | Form (Bauch, am Bild) | Marke |
|---|---|---|---|---|---|
| „2.Ud:Frekans, Sönüm:72,700 Hz, 3,316 %“ | 72,7 Hz | 3,316 % | 15 | großflächig: Feld unter dem Steg **und** oberes Feld bis in den Hals | Frequenz/Dämpfung [wörtlich]; Form [eigene Einschätzung] |
| „117,738 Hz, 2,007 %“ | 117,7 Hz | 2,007 % | 25 | **ein Bauch** im unteren Feld zwischen Rosette und Steg | dito |
| „177,833 Hz, 0,778 %“ | 177,8 Hz | 0,778 % | 64 | Bauch um die Rosette, schwächeres Band darunter, Knotenlinie quer dazwischen | dito |
| „206,018 Hz, 3,317 %“ | 206,0 Hz | 3,317 % | 15 | **ein Bauch** im unteren Feld, Lage wie bei 117,7 Hz | dito |
| „260,763 Hz, 1,110 %“ | 260,8 Hz | 1,110 % | 45 | zwei Bäuche übereinander im unteren Feld | dito |
| „282,902 Hz, 1,323 %“ | 282,9 Hz | 1,323 % | 38 | Bänder quer, mehrere Knotenlinien | dito |
| „324,966 Hz, 0,694 %“ | 325,0 Hz | 0,694 % | 72 | zwei Bäuche nebeneinander (links/rechts) | dito |
| „348,769 Hz, 1,398 %“ | 348,8 Hz | 1,398 % | 36 | schwach, mehrere kleine Bäuche | dito |

- Die Quelle definiert „Sönüm %“ nicht; die Güte rechnet mit ζ = Sönüm/100 als Dämpfungsgrad [eigene Rechnung, Lesart
  eigene Einschätzung].
- Die Farbskala trägt −100 … 100, die Farben wirken aber wie ein Betrag (schwarz an den Knoten); **die Phase** der
  Bäuche ist aus dem Bild nicht sicher zu lesen [eigene Einschätzung]. Ob 117,7 / 206,0 Hz gleichsinnig sind, ist
  deshalb offen.
- Die Quelle ordnet keine Mode der Luft zu, ein Mikrofon im Schallloch gibt es nicht. Lesart [eigene Einschätzung]:
  117,7 und 206,0 Hz sind das gekoppelte Paar aus Grundmode der Decke und Luft (beide ein Bauch an derselben Stelle,
  beide hoch gedämpft, beide auf Spitzen der Abstrahlung, siehe 2.2) — wie bei der Gitarre; das bleibt eine Deutung.
- **Gegen das Modell aus S207 §8** [eigene Einschätzung]: Im Modell 99,9 / 168,6 / 207,8 / 269,4 / 351,3 Hz; die
  Mode 207,8 Hz hat dort am Steg nur 28 % ihres Größtwerts (Bauch zwischen den kleinen Löchern), die Mode 168,6 Hz
  bewegt den Steg stark. An der gemessenen Oud hat die Mode um 206 Hz ihren Bauch im Stegfeld, und die erste
  einbäuchige Mode liegt bei 117,7 statt 99,9 Hz.

### 2.2 Abstrahlung und Steg, ungeglättet (Değirmenli 2023, Abb. 3 und 4, S. 531)

Was die Kurven sind [wörtlich S. 527–528, 530]: Abstrahlung = Schalldruck je Kraft nach Hammerschlag am Steg; im
normalen Raum, nicht reflexionsarm; Anordnung um 360° drehbar, nach Schleske 36 Messungen in 10°-Schritten gemittelt
(„36 ölçümün ortalamasının alınmasını önermektedir“ — als Vorschlag Schleskes zitiert, die Umsetzung mit der
Drehanordnung, Resim 2). **Abstand und Bezug des Schalldrucks nennt die Arbeit nicht**; die Achse trägt nur
„[dB/1u (m/s^2)/N]“ (die Einheit der Stegkurven). „Mobilite“ ist hier die Beschleunigung je Kraft am Steg (Aufnehmer
B&K 4517). Bereich 50 Hz – 1 kHz, keine Glättung zu erkennen.

**Zuordnung der Kurven** [eigene Einschätzung]: obere Schar (120–185) = Steg, untere Schar (40–105) = Abstrahlung;
unten rot = Ud 2 (die lauteste), blau = Ud 6 (Abb. 3) bzw. Ud 5 (Abb. 4), grün = Ud 7 (die leiseste). Die Farben in der
Legende sind im Druck nicht eindeutig.

**Ablesung** an `seiten/Degirmenli_2023_S531_Abb3_130dpi-07.png` (Ausschnitt 130 dpi), Verfahren in `kurve_kalib.py`
und `kurve_ablesen.py`: Gitterlinien in Pixeln gefunden, 382 px je Dekade (Streuung 381–384, ein Pixel ≈ 0,6 % der
Frequenz), 1,875 px je dB; Kurven nach Farbe, wo eine Kurve von einer anderen verdeckt ist, über höchstens 6 px
linear überbrückt. **Genauigkeit:** ±1 dB am Pixel, aber die ungeglätteten Kurven springen um 10 dB auf wenige Hz und
überdecken sich; der Wert an einer Frequenz ist deshalb auf etwa ±3 dB zu nehmen, wo die Linie steil ist schlechter.
„–“ = an der Stelle nicht ablesbar.

Werte an den Teiltönen der leeren Chöre (arabische Stimmung wie im Modell, gleichstufig), dB wie Achse:

| Ton | Frequenz | Ud 2 (rot) | Ud 6 (blau) | Ud 7 (grün) |
|---|---|---|---|---|
| C2 T1 | 65,4 Hz | – (Umgebung 60–69) | 63 | – (Umgebung 62–64) |
| C2 T2 | 130,8 Hz | **91** | **87** | – |
| C2 T3 | 196,2 Hz | – (bei 200 Hz: 98) | – (bei 200 Hz: 95) | 78 |
| F2 T1 | 87,3 Hz | – (Umgebung 55–69) | 58 | 53 |
| F2 T2 | 174,6 Hz | 87 (Spitze 99 bei 178 Hz) | – | 75 |
| A2 T1 | 110,0 Hz | 81 | – | – |
| A2 T2 | 220,0 Hz | 96 | 98 | 81 |
| D3 T1 | 146,8 Hz | 80 | 76 | 82 |
| D3 T2 | 293,7 Hz | 92 | 86 | 76 |
| G3 T1 | 196,0 Hz | – (bei 200 Hz: 98) | – (bei 200 Hz: 95) | 78 |
| G3 T2 | 392,0 Hz | 86 | 80 | 91 |
| C4 T1 | 261,6 Hz | – | 79 | 77 |

Gestalt der Kurven Ud 2 und Ud 6 am Bild [wörtlich am Bild abgelesen, ±5 % Frequenz, ±3 dB]: unter 100 Hz 55–70 dB;
Anstieg ab rund 105 Hz zu einer Spitze bei **125–135 Hz mit rund 88–92 dB**; Tal bei 140–160 Hz (Ud 6 bis gegen 57 dB
bei rund 155 Hz, Ud 2 um 75–80); breites Maximum **170–225 Hz mit 96–102 dB**; Tal um 240–255 Hz (rund 75); 280–450 Hz
80–92; um 700 Hz bis rund 103. Ud 7 ist flacher: unter 100 Hz 50–65, 120–250 Hz 73–82, Spitze um 390–400 Hz rund 91–94.
Text S. 531 [wörtlich]: „Bu ud'ların özellikle 120 Hz ve 180 Hz - 220 Hz frekans bölgelerinde yüksek genlikli ses
yayılımı gerçekleştirdiği tespit edilmiştir“ (die lauten Ouds strahlen besonders bei 120 Hz und 180–220 Hz stark ab).

**Gegen das Modell (S207 §8)** [eigene Rechnung aus den Zahlen oben und aus S207; Messort, Abstand und Glättung sind
verschieden, deshalb nur die Lage zur jeweils stärksten Spitze unter 300 Hz]:

| Stelle | Modell |p/F| in 2 m, gegen die Spitze 262–277 Hz (+8 dB) | Ud 2 / Ud 6, gegen das Maximum bei 200 Hz (98 / 95 dB) |
|---|---|---|
| C2 T1, 65 Hz | −43 dB | rund −35 / −32 dB |
| C2 T2, 131 Hz | −27 dB | **−7 / −8 dB** |
| C2 T3, 196 Hz | −18 dB | **0 / 0 dB** |

Bei den gemessenen Ouds 2 und 6 liegt C2 T2 auf der ersten Spitze und C2 T3 auf dem Hauptmaximum; im Modell liegen
beide im Tal. Der Unterschied ist an T2 und T3 je rund **18–20 dB** — dieselbe Größe, die S207 am Antwortpunkt 3 von
Abdallah gefunden hat. Gemessen ist dagegen ein Tal bei 140–160 Hz: an einer echten Oud fällt eher **D3 T1 (147 Hz)**
hinein als der Basschor.

### 2.3 Weitere Angaben

| Quelle | Angabe | Marke |
|---|---|---|
| UBC-Wiki „Course:Phys341 2020/Oud“ (Studentenseite, Abbildungen „Own Work“, Messung nicht beschrieben) | „Acoustic radiation is dominated by the lowest air and lowest wood modes at 150 and 220 Hz“; Tabelle „Primary Air Mode ⩰150Hz … Primary Wood Mode ⩰220Hz“; „The cavity and body resonances are measured by damping the strings and striking the bridge.“ Welche Oud, welche Stimmung: nicht gesagt; die Stimmungsangaben auf der Seite sind in sich widersprüchlich („F2, A3, D3, G3, C3, F3“) | [wörtlich] `abrufe/ubc_phys341_2020_oud.html`, Belegkraft schwach |
| Değirmenli 2023, S. 531 | „yüksek ses gürlüğü ve bas karakteristiklerine sahip ud'ların 700 Hz bölgesinde beklenenden daha yüksek bir genlikle ses yayılımı“ | [wörtlich] |
| World-Scientific-Arbeit zur Oud (Hammer bis 400 Hz, FE mit Luft, harmonische Akustik, Lochgröße, Deckendicke, Balken) | Autoren, Jahr, Zeitschrift nicht ermittelt; vermutlich aus dem Umfeld von Abdallah u. a. | [nur Zusammenfassung] |
| Acar, Karakaş, Oktav, „Tuning the structural eigenfrequencies of an oud guitar by using different brace patterns on the soundboard“ (zitiert bei Abdallah 2026, Ref. 23, Literaturliste S. 26–28 gelesen) | Volltext nicht gefunden | [wörtlich] nur der Eintrag in der Literaturliste |

**Nicht gefunden — Suchwege:** „Değirmenli ud tablası titreşim doktora tezi Gazi 2014 pdf“; „udda ses oluşumunun
incelenmesi Değirmenli tez pdf“; „Değirmenli ud ses oluşumu modal analiz makale DergiPark …“; „oud acoustic radiation
measurement air mode top plate mode Helmholtz“; „oud bridge admittance OR mobility …“; „oud vibroacoustic assembled
instrument modal analysis … Inanli Altinsoy 2019 OR 2020 OR 2021“ (keine Folgearbeit zu Inanli 2018 gefunden);
„Acar Karakaş Oktav oud …“; Literaturliste Abdallah 2026 durchgesehen (Oud-Einträge: Ref. 23–26). Değirmenli 2014
(Masterarbeit laut Ziyağil 2021 und einer Suchzusammenfassung, „PhD Thesis“ laut Inanli 2018 — der Widerspruch aus
Bericht 25 bleibt) nicht erreicht.

---

## 3 Wie der tiefe Chor gezupft wird

**Antwort in einem Satz:** Messungen von Kraft, Auslenkung oder Zupfort je Chor sind nicht gefunden; es gibt nur
Angaben zur Risha (Werkstoff, Länge, Breite, Überstand) und Lehrmeinungen ohne Zahl.

| Quelle | Angabe | Marke |
|---|---|---|
| Arab Instruments, „Plucking the Strings“ (Händler) | Risha früher Adlerfeder, heute Horn („softened first by being soaked in oil“) oder Kunststoff; „held between the fingers … with around 1cm protruding“; „downstrokes tend to produce more volume and a sharper 'attack'“ | [wörtlich] `abrufe/arabinstruments_plucking.html` |
| UBC-Wiki (siehe oben), mit Verweis auf „Pablo's Oud Page“ 2006 | „A Risha is a flat stick typically measuring 100-180mm long and 6-10mm wide.“; Zupfort: „It is generally preferable to play so the fingers of the right hand are over the sound hole“ | [wörtlich], schwach |
| Değirmenli 2023, S. 529 | Zupfmaschine „sabit bir açıda ve hızda“ — Winkel und Geschwindigkeit nicht beziffert | [wörtlich] |
| Mike's Oud Forums (mehrere Fäden) | Risha etwa 45° zur Saite; „The depth that the risha travels into the string should also be considered“; bei Doppelchören erklingen beide Saiten, im Tremolo trifft die Risha die untere nicht immer | [nur Zusammenfassung], Forum nicht erreichbar |

Ob beide Saiten eines Chors zugleich ausgelenkt werden, ist **nicht gemessen gefunden**; Anschlagstärke je Chor ebenso
wenig. **Suchwege:** „mızrap kuvvet ölçümü ud … plucking force oud risha measurement“; „oud risha technique bass
string course both strings plucked together …“; Händler- und Lehrseiten.

---

## 4 Besaitung: Zug je Saite für einen arabischen Satz mit einzelnem C2

**Antwort in einem Satz:** D'Addario nennt für den Satz EJ95A (arabisch, C F A D G C, einzelnes C2) den Zug je Saite;
das einzelne C2 hat 8,0 lbs, eine Saite des F2-Chors 6,3 lbs — Mensur und lineare Masse nennt die Seite nicht.

Quelle: D'Addario, Produktseite „Oud Normal Tension 11-String Set for Arabic Tuning“, SKU EJ95A, abgerufen 2026-11-19
mit `curl` (Weiterleitung auf https://www.daddario.com/products/arabic-oud-strings-ej95a), Datei
`abrufe/daddario_arabic_oud.html`. Wortlaut [wörtlich]: „EJ95A is a nylon core string set optimized for use with
11-string Arabic Oud, tuned CFADGC.“ „Wrap Material: Silver-Plated Copper“, „Construction: Round Wound“, „String Gauges:
.026/.026, .031/.031, .024/.024, .028/.028, .032/.032, .043“; „String Tension Chart … Item # Note Tension - C4 0.000 lbs -
C4 0.000 lbs - G3 6.300 lbs - G3 6.300 lbs NYL024W D3 8.900 lbs NYL024W D3 8.900 lbs J4304 A2 7.100 lbs J4304 A2 7.100
lbs - F2 6.300 lbs - F2 6.300 lbs J4506 C2 8.000 lbs“.

| Ton | Durchmesser (in) | Teil-Nr. | Zug laut Tabelle | Zug [eigene Rechnung, 1 lbf = 4,448 N] | Zug je Chor |
|---|---|---|---|---|---|
| C4 (2×) | .026 | – | „0.000 lbs“ (Lücke der Seite) | – | – |
| G3 (2×) | .031 | – | 6,3 lbs | 28,0 N | 56,0 N |
| D3 (2×) | .024 („W“ in NYL024W als umsponnen gelesen, eigene Lesart) | NYL024W | 8,9 lbs | 39,6 N | 79,2 N |
| A2 (2×) | .028 | J4304 | 7,1 lbs | 31,6 N | 63,2 N |
| F2 (2×) | .032 | – | 6,3 lbs | 28,0 N | 56,0 N |
| **C2 (1×)** | **.043** | **J4506** | **8,0 lbs** | **35,6 N** | **35,6 N** |

- Zuordnung der Durchmesser zu den Tönen in der Reihenfolge der Liste (C4 … C2) [eigene Lesart]; die Seite nennt sie
  getrennt von der Zugtabelle. Die Teilnummern J4506 / J4304 sind Einzelsaiten der Pro-Arte-Gitarrenreihe [eigene
  Einschätzung, nicht geprüft].
- **Mensur nicht angegeben.** D'Addario rechnet Gitarren laut Suchzusammenfassung mit 25,5 in [nur Zusammenfassung];
  die Zug-PDF (`tension_chart_13934.pdf`) kam als HTML-Seite zurück und ist nicht lesbar (Datei liegt als Beleg unter
  `abrufe/DAddario_TensionChart_13934.pdf`). Damit lässt sich die lineare Masse hier nicht rechnen.
- [eigene Rechnung] Einzelnes C2 gegen den F2-Chor: 35,6 / 56,0 N = 0,64, also **−3,9 dB** Kraft am Steg bei gleichem
  Auslenkwinkel; je Saite trägt C2 27 % mehr Zug als eine F2-Saite. Das Modell nennt 4–5 dB weniger Kraft gegen Chor 2
  (S207 §8).
- Pyramid, Kürschner, La Bella, Aquila: weiter keine Zugtabelle je Saite (Stand Bericht 25); eine Suchzusammenfassung
  nennt für den türkischen Satz EJ95 Zugwerte in kg von einem Händler [nur Zusammenfassung], nicht gelesen (HTTP 429).

---

## Liste der gelesenen Dateien (Kopf wie auf der ersten Seite)

| Datei unter `abrufe/` | Autor(en), Jahr, Titel | URL | Ergebnis |
|---|---|---|---|
| `Degirmenli_2023_TinisalNitelikler_Idil104.pdf` | Emir Değirmenli, „Titreşim ve Ses Yayınım Analizleri Kullanılarak Telli Çalgılardaki Tınısal Niteliklerin İncelenmesi: Ud Örneği“, idil 104 (2023 Nisan), S. 525–534, doi 10.7816/idil-12-104-08 (Kopfzeile jeder Seite) | https://www.idildergisi.com/makale/pdf/1684998615.pdf | Hauptfund Lücken 1 und 2; S. 527–531 als Bild gelesen |
| `dergipark_1751498.pdf` | Hakan Emre Ziyağil, „Türk mûsikîsinin telli çalgılarından ud'un ses oluşumu ve yapısal özellikler bağlamında incelenmesi“, Motif Akademi Halkbilimi Dergisi 14 (35), 2021, S. 1057–1073 (Übersichtsartikel) | https://dergipark.org.tr/en/download/article-file/1751498 | keine Messwerte; zitiert Değirmenli 2014 (Spektrum der D-Saite, Şekil 2) |
| `dergipark_3966895.pdf` | Yunis Şahinkayası, Emir Değirmenli, Hamide Şahinkayası, Ünal Öztorun, „Effects of hollowed neck designs on sound radiation and loudness of baglama“, Rast Musicology Journal 12 (3), 2024, S. 245–276 | https://dergipark.org.tr/en/download/article-file/3966895 | Bağlama, nicht Oud; nicht weiter gelesen |
| `dergipark_433700.pdf` | Yunus Korkmaz, Aytuğ Boyacı, „Adli Bilişim Açısından Ses İncelemeleri“, Fırat Üniv. Müh. Bil. Dergisi 30 (1), 2018, S. 329–343 | https://dergipark.org.tr/en/download/article-file/433700 | nicht einschlägig |
| `daddario_arabic_oud.html` | D'Addario, Produktseite EJ95A | siehe Lücke 4 | Zugtabelle |
| `ubc_phys341_2020_oud.html` | UBC Wiki, „Course:Phys341 2020/Oud“, zuletzt bearbeitet 9.4.2020 | https://wiki.ubc.ca/Course:Phys341_2020/Oud | schwach, siehe 2.3 und 3 |
| `arabinstruments_plucking.html` | Arab Instruments, „Plucking the Strings“ | https://arabinstruments.com/pages/plucking-the-strings | Risha |
| `freesound_*.html` (3) | Freesound-Suche und Paket 30959 | siehe Lücke 1 | Ladeliste |

Lizenzen der PDFs nicht geprüft. Auch Fehlerseiten liegen als Beleg bei (`INDEX.md`, Spalte Vermerk).

## Reicht das, um zu sagen, wie viele Dezibel der tiefe Chor einer arabischen Oud unter den übrigen liegt?

**Nein.** Es fehlt jede Messung des Pegels oder der Lautheit einzelner gespielter Töne je Chor. Was vorliegt, sind
zwei Teile, die zusammen nur eine Abschätzung tragen würden: die Abstrahlung je Kraft am Steg (Değirmenli 2023, an
türkischen Ouds, Abstand und Bezug ungenannt) und der Zug je Saite (D'Addario, ohne Mensur). Es fehlen die Kraft am Steg
je Teilton beim echten Zupf je Chor (Anschlag, Zupfort, ob beide Saiten schwingen), eine arabische Oud mit C2 und ein
Hörort wie in der LumiViz-Messung.

Was die Funde trotzdem sagen [eigene Einschätzung]: Die zweite Ursache aus S207 §8 — Teiltöne 2 und 3 des Basschors in
einem Tal der Abstrahlung — trifft an den gemessenen Ouds 2 und 6 **nicht** zu: dort liegen 131 und 196 Hz auf den zwei
stärksten Spitzen unter 250 Hz, im Modell 18–20 dB tiefer. Die erste Ursache (eine Saite statt zwei) bestätigt
D'Addario der Größe nach (−3,9 dB im Zug gegen den F2-Chor). Ein Abfall des Basschors um 12,9 LU ist mit diesen Kurven
schwer vereinbar; eine Zahl für die echte Oud folgt daraus nicht.

## Was nicht geliefert ist, und was die LumiViz-Sitzung selbst prüfen muss

**Nicht geliefert:** Pegel je Ton oder je Chor (Lücke 1); Zuordnung Luftmode / Grundmode der Decke aus einer Quelle
(nur eigene Deutung); Stegadmittanz einer **arabischen** Oud (die Kurven sind von türkischen Ouds, Stimmung nicht
genannt); Messungen zum Zupf; Mensur und lineare Masse zur D'Addario-Tabelle; Değirmenli 2014 und 2018 selbst.

**Selbst prüfen:**
1. Die Zuordnung der Farben in Abb. 3/4 (welche Schar Abstrahlung, welche Steg; rot = Ud 2) am Bild
   `seiten/Degirmenli_2023_S531_Abb3_130dpi-07.png` — die Legende ist nicht eindeutig.
2. Die Tabelle der Teiltöne in 2.2 ist eine Digitalisierung ungeglätteter, sich überdeckender Kurven (±3 dB, an steilen
   Stellen mehr); für einen Vergleich mit dem Modell eher die Lage der Spitzen und Täler nehmen als einzelne Werte.
3. „Sönüm %“ als Dämpfungsgrad gelesen (Q = 1/(2ζ)); die Quelle sagt es nicht.
4. Die Formen in 2.1 sind am Bild beschrieben, die Phase der Bäuche ist nicht lesbar.
5. Die Ouds bei Değirmenli sind türkisch (Gazi, Ankara; Stimmung in der Arbeit nicht genannt) — Lage der Moden gegen
   eine arabische Oud mit größerem Körper verschoben zu erwarten.
6. D'Addario: Zuordnung der Durchmesser zu den Tönen und die Lücke „C4 0.000 lbs“.
7. **Ladeliste für die Dissertation:** `Degirmenli_2018_UddaSesOlusumu_Dissertation.pdf`, YÖK Açık Bilim, Handle
   20.500.12812/581867 (https://acikbilim.yok.gov.tr/handle/20.500.12812/581867) bzw. Tez Merkezi; Größe und Lizenz
   unbekannt; wozu: S. 115–116 und 125 (Hörversuch, Moden) und die Aufnahmen der leeren Saiten von zehn Ouds in 30 cm —
   die einzige gefundene Messreihe, aus der sich Pegel je Saite ableiten ließen, falls die Arbeit sie beziffert. Der
   Server war aus diesem Lauf nicht auflösbar; vielleicht von einem anderen Netz aus erreichbar.
