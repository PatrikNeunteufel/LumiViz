# Prüfstein 15h — TD-PSOLA gegen Signalsmith Stretch (S170)

> **Stufe 15h**, Entscheid WV6 a (`docs/audio/Loop_Konzept.md` §4p). Gerechnet am 2026-11-01 unter Release-clang mit
> `test_LoopDehnerPruefstein.cpp`, Fall „Pruefstein voll“ (`LUMIVIZ_PRUEFSTEIN_15H=<Ordner>`, dazu
> `LUMIVIZ_VOCADITO=<Ordner>` für zwei echte Stimmen). Die vollständigen Tabellen stehen unten, so wie der Test sie
> schreibt; die Hörproben liegen lokal unter `.claude/handover/hoerproben_15h/` (nicht im Repo).

## Urteil

**PSOLA hat einen klaren Vorteil — *Voice* bleibt (WV6 a a).** Gegen Signalsmith Stretch hält es die Hülle des Spektrums
sechsmal genauer, die Formanten vier- bis sechsmal genauer, trifft die Zieltonhöhe gleich gut oder besser und fügt kein
Nebengeräusch hinzu. Es kostet rund dreimal so viel Rechenzeit (3,2–3,6 % der Echtzeit, pYIN eingeschlossen). Auch
Signalsmith **mit** Formanterhalt (`setFormantFactor(1, true)`, als fairer Gegenkandidat mitgemessen) bleibt deutlich
hinter PSOLA und fügt mehr Nebengeräusch hinzu als ohne; er geht nicht ins Programm.

Zwei Befunde gehen über 15h hinaus:

1. **pYIN verlor hohe Stimmen** (unten „Befund pYIN“): an einer gerechneten Sopranstimme bei 440 Hz fand pYIN 0 %
   der Rahmen gesungen — dann verschob PSOLA nichts, und Autotune (15i) korrigierte dort nichts. Mit der Talregel
   wie in librosa 99,6 %, an vocadito gleich gut. **Entschieden PY2 a** (Patrik S170): die Talregel gilt; die
   Tabellen unten sind mit ihr gerechnet.
2. **Der Pegel folgt den Formanten:** bei großen Sprüngen nach oben verliert PSOLA bis −18 dB (/i/ bei 392 Hz, +12
   Halbtöne), Signalsmith mit Formanterhalt bis −10,2 dB. Das ist die Folge des Formanterhalts selbst (der Pegel gegen
   die Hülle weicht nur −3,9 … +4,0 dB ab, bei +12 genau 0) — keine Frage jetzt; beobachten in den Hörproben zu 15i.

## Aufbau

**Gerechnete Stimmen:** Rosenberg-Puls (Öffnen 40 %, Schließen 16 % der Periode), abgeleitet, durch drei Resonatoren in
Reihe; Formanten nach Peterson & Barney 1952 (Mittelwerte), Bandbreite 50 Hz + 6 % (von Claude gesetzt); 3 s, 48 kHz.

| Stimme | f0 Hz | F1 / F2 / F3 Hz |
|---|---|---|
| man_a_110 | 110 | 730 / 1090 / 2440 |
| man_i_130 | 130 | 270 / 2290 / 3010 |
| woman_a_220 | 220 | 850 / 1220 / 2810 |
| woman_u_262 | 262 | 370 / 950 / 2670 |
| woman_a_330 | 330 | 850 / 1220 / 2810 |
| woman_i_392 | 392 | 310 / 2790 / 3310 |
| woman_a_440 | 440 | 850 / 1220 / 2810 |
| man_a_165_vibrato | 165, Vibrato 5,5 Hz ±40 Cent | 730 / 1090 / 2440 |

Je Stimme −12, −7, −3, +3, +7, +12 Halbtöne und eine Dehnung auf 1,25 ohne Tonhöhe.

**Kandidaten:** `signalsmith` (die Vorgabe *General*), `signalsmith_formants` (mit Formanterhalt, Grundton je Block
aus pYIN), `psola` (*Voice*, Marken aus pYIN). Vor PY2 a lief PSOLA zweimal mit (pYIN ab τ = 2 und mit der
Talregel); die Zahlen der Fassung ohne Talregel stehen unten in der Zusammenfassung.

**Messgrößen** (ohne pYIN, damit der Prüfstein nicht misst, was er an PSOLA prüft):

- **Cent:** alle 20 ms der Grundton am Ergebnis gegen den am Klang mal β, beide als Summe der Teiltöne im Spektrum
  (2048 Werte, ±100 Cent um den bekannten Ton); Median des Betrags in der Mitte.
- **Hülle dB:** die Pegel der Teiltöne bis 4,5 kHz (Leistung im Band ±f0/4, 1 s) gegen die Hülle des Klangs an
  derselben Frequenz, ohne mittleren Versatz, Effektivwert.
- **F1 %, F2 %:** der höchste Teilton zwischen den Mitten zu den Nachbarformanten, parabolisch verfeinert, gegen den
  Klang; nur an einem Gipfel („–“ sonst).
- **Pegel dB** gegen den Klang; **Pegel gegen Hülle dB**: Leistung der neuen Teiltöne gegen die, die die Hülle des
  Klangs dort vorhersagt (0 = der Pegel folgt allein den Formanten).
- **Neben dB:** Leistung außerhalb der Bänder um die Teiltöne gegen die ganze; in Klammern der Klang.

## Zusammenfassung (Tonhöhe ohne Dehnung, alle Stimmen ohne Vibrato, mit PY2 a)

| Umsetzung | Cent Median / max | Cent Vibrato max | Hülle dB Mittel / max | \|F1\| % Mittel | \|F2\| % Mittel | Pegel dB min … max | Pegel gegen Hülle dB min … max | Nebengeräusch über Klang dB Mittel / max |
|---|---|---|---|---|---|---|---|---|
| signalsmith | 1,1 / 7,8 | 3,6 | 12,7 / 24,4 | 31,4 (n = 14) | 22,2 (n = 10) | −0,6 … 0,2 | −19,6 … 18,1 | 18,1 / 37,8 |
| signalsmith_formants | 1,0 / 3,9 | 4,5 | 5,7 / 11,5 | 9,8 (n = 22) | 5,5 (n = 14) | −10,2 … 2,0 | −13,2 … 9,5 | 23,7 / 60,7 |
| psola | 0,8 / 2,5 | 2,5 | 2,2 / 8,5 | 5,0 (n = 24) | 2,7 (n = 15) | −18,2 … −0,1 | −3,9 … 4,0 | 1,3 / 11,7 |

Vor PY2 a (pYIN ab τ = 2, woman_a_440 nicht gezählt, weil dort stumm erkannt): `psola` 0,7 / 2,5 Cent, Hülle 2,1 /
8,5 dB, F1 5,6 % (n = 21), Nebengeräusch 0,8 / 11,7 dB; `signalsmith_formants` Hülle 6,1 / 17,4 dB, Pegel bis −8,7 dB.

Dehnung auf 1,25 ohne Tonhöhe: Signalsmith Hülle 0,2–0,7 dB, PSOLA 0,0 dB; Cent beide unter 0,3 (Vibrato 2,6 bei
Signalsmith, 0,2 bei PSOLA); Pegel beide ±0,1 dB. Vor PY2 a rechnete `psola` am Sopran bei 440 Hz 35 Cent daneben.

**Rechenzeit** (Release-clang, % der Echtzeit, je Lauf über alle Fälle): `signalsmith` 1,3–1,6, `signalsmith_formants`
4,0–4,9, `psola` 3,2–3,6 (davon pYIN rund 1,5–2,2). Der Fall „Rechenzeit“ in `test_LoopDehner`: 10 s Stereo, +3
Halbtöne — Signalsmith 100 ms (1,0 %), PSOLA 315 ms (3,2 %).

**Gleichheit MSVC / clang:** die Bezugswerte `werte/dehner_bezug_td-psola.txt` (geschrieben unter Release-clang)
stimmen unter Testing-MSVC auf 10⁻⁵ (Fall „Loop Dehner 15f Bezugswerte“, 25 Werte). Unter Testing-MSVC: Signalsmith
2,1 %, PSOLA 3,8 % der Echtzeit.

## Befund pYIN — hohe Stimmen

Stufe 1 von pYIN nahm je Schwelle die erste Stelle ab τ = 2, an der d′ unter die Schwelle fällt (so steht es im
Paper, so war es in 15g gebaut). Am Sopran bei 440 Hz (48 kHz) hat d′ ein Tal bei τ = 54 (889 Hz, die halbe
Periode) mit 0,058 und das echte bei τ = 109 mit 0,0001. Fast alle Schwellen der Beta-Verteilung (Mittel 0,20) liegen
über 0,058 und wählen 889 Hz — das liegt über der obersten Stufe des HMM (875 Hz) und fällt weg; für 440 Hz bleiben
rund 7 % (die Schwellen unter 0,058), und der Rahmen gilt als stumm. Dasselbe am Ergebnis von PSOLA (woman_a_220 +7,
330 Hz): ein Tal bei τ = 47 (1021 Hz, beim F2).

**Talregel** (wie librosa `pyin`): je Schwelle das erste *Tal* von d′ unter der Schwelle mit einer Periode im Bereich
des HMM; ohne eines das tiefste Tal im Bereich mit p_a. Zuerst als Schalter zum Vergleich gebaut, seit **PY2 a**
(Patrik S170) fest; `kTonhoehenFassung` 2. Bericht an vocadito: `S170_pyin_vocadito.md` (vorher
`S169_pyin_vocadito.md`).

| Messung | vor PY2 a | mit Talregel (PY2 a) |
|---|---|---|
| woman_a_440: Rahmen gesungen | 0,0 % | 99,6 % |
| alle anderen Stimmen: Rahmen gesungen | 99,2–99,8 % | gleich |
| woman_a_440, PSOLA ±3: Cent | 86,5 / 100,0 | 2,3 / 1,1 |
| vocadito (Mittel 0,20): RPA 50 / RPA 100 | 93,90 / 94,17 % | 93,97 / 94,24 % |
| vocadito: erkannt / Fehlalarm | 95,04 / 9,67 % | 95,12 / 9,73 % |
| vocadito: Oktave / F | 0,491 / 94,16 % | 0,495 / 94,18 % |

Vocadito enthält keine so hohen Stimmen (mittlere Tonhöhe der 40 Ausschnitte laut `vocadito_metadata.csv` MIDI 47–65,
also bis rund 349 Hz) — dass sich dort fast nichts ändert, zeigt nur, dass die Regel tiefere Stimmen nicht
verschlechtert; für hohe Stimmen gibt es keine beschriftete echte Aufnahme im Bestand.

## Verlauf (was am Prüfstein selbst falsch war)

1. **Tonhöhe mit pYIN gemessen:** die Ergebnisse hoher Stimmen galten als stumm (der Befund oben) — ersetzt durch die
   Summe der Teiltöne.
2. **Teiltöne als DFT an genau h · f0:** schon 1 Cent Fehler im Grundton kostete bei hohen Teiltönen viel Amplitude,
   selbst Signalsmith lag bei −4 … −20 dB Nebengeräusch — ersetzt durch die Leistung im Band ±f0/4.
3. **F1 und F2 im selben Bereich** (0,6–1,5 · F überlappte bei /a/) — ersetzt durch die Mitten zu den Nachbarn.
4. **Signalsmith mit Formanterhalt** schätzte den Grundton für die Hülle selbst und lag bei 110 Hz eine Oktave daneben
   (700–1200 Cent); mit dem Grundton aus pYIN wirkte er erst gar nicht — `setFormantBase` erwartet Bruchteile der
   Abtastrate wie alle Frequenzen von Stretch (`stft.freqToBin`), nicht Hz.
5. **PSOLA auf ganzen Abtastwerten:** die Pulse schwankten um ±0,5 Abtastwerte, am Formanten bei 1 kHz rund −28 dB
   Nebengeräusch (gemessen −26 … −44 dB). Seither Marken mit Bruchteil (parabolisch am Gipfel) und Körner mit
   gefenstertem Sinc gelesen (16 Stützen, Kaiser β = 8, 1024 Phasen): Nebengeräusch auf der Höhe des Klangs.

## Hörproben

Unter `.claude/handover/hoerproben_15h/`: je Stimme `<stimme>_0.wav` (der Klang) und `<stimme>_<±ht>_<umsetzung>.wav`
für jede Tonhöhe und jeden Kandidaten, `<stimme>_stretch125_<umsetzung>.wav`; dazu **vocadito 2 und 4** (CC BY 4.0,
die ersten 8 s) bei −7, −3, +3, +7 durch jeden Kandidaten. Zum Hören vor allem: `vocadito_*` (echte Stimmen),
`woman_a_440_+3_*` (der Befund pYIN), `woman_i_392_+12_*` (der Pegel).

## Vollständige Tabellen
### man_a_110 (F 730, 1090, 2440 Hz)

pYIN findet am Klang 99.6 % der Rahmen gesungen (PSOLA verschiebt nur, wo gesungen).

| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| signalsmith | -12 | 1 | 2.4 | 17.7 | -26.4 | 13.5 | 531 | 1205 | -0.1 | -3.3 | -55.3 (-68.0) |
| signalsmith_formants | -12 | 1 | 0.8 | 5.9 | 6.3 | 3.9 | 768 | 1104 | 1.0 | -1.8 | -28.1 (-68.0) |
| psola | -12 | 1 | 0.2 | 0.5 | 6.4 | 2.3 | 768 | 1086 | -0.6 | -3.3 | -66.7 (-68.0) |
| signalsmith | -7 | 1 | 0.8 | 12.8 | -33.2 | – | 482 | 0 | -0.0 | -1.7 | -53.8 (-68.0) |
| signalsmith_formants | -7 | 1 | 0.3 | 3.4 | -5.6 | 3.0 | 682 | 1094 | 0.4 | -1.1 | -41.6 (-68.0) |
| psola | -7 | 1 | 0.2 | 0.7 | -5.5 | 1.7 | 682 | 1080 | -0.6 | -2.0 | -68.9 (-68.0) |
| signalsmith | -3 | 1 | 1.1 | 6.9 | -15.7 | -15.9 | 608 | 893 | -0.1 | -0.7 | -54.7 (-68.0) |
| signalsmith_formants | -3 | 1 | 0.3 | 2.1 | -5.2 | 3.2 | 684 | 1096 | -1.0 | -1.5 | -50.5 (-68.0) |
| psola | -3 | 1 | 0.2 | 0.6 | -5.1 | 1.5 | 685 | 1078 | -0.5 | -1.0 | -69.2 (-68.0) |
| signalsmith | 3 | 1 | 0.9 | 7.6 | – | -19.7 | 0 | 853 | -0.2 | 0.8 | -52.4 (-68.0) |
| signalsmith_formants | 3 | 1 | 0.6 | 3.1 | -0.4 | -3.4 | 719 | 1026 | -1.9 | -0.9 | -50.9 (-68.0) |
| psola | 3 | 1 | 0.2 | 0.7 | 0.5 | -6.4 | 726 | 994 | -0.6 | 0.4 | -68.8 (-68.0) |
| signalsmith | 7 | 1 | 0.4 | 14.3 | – | 1.8 | 0 | 1081 | -0.1 | 2.1 | -54.9 (-68.0) |
| signalsmith_formants | 7 | 1 | 0.7 | 3.4 | -3.4 | – | 697 | 0 | -3.0 | -0.7 | -53.6 (-68.0) |
| psola | 7 | 1 | 0.4 | 0.8 | -3.0 | – | 700 | 0 | -2.4 | -0.2 | -73.4 (-68.0) |
| signalsmith | 12 | 1 | 1.0 | 18.1 | – | 36.1 | 0 | 1445 | -0.3 | 2.6 | -54.4 (-68.0) |
| signalsmith_formants | 12 | 1 | 1.8 | 5.3 | -9.0 | -5.2 | 657 | 1006 | -3.2 | -0.2 | -54.4 (-68.0) |
| psola | 12 | 1 | 2.0 | 0.0 | -8.4 | -4.8 | 661 | 1011 | -2.9 | 0.0 | -73.3 (-68.0) |
| signalsmith | 0 | 1.25 | 0.0 | 0.3 | 0.0 | 0.0 | 722 | 1062 | -0.0 | -0.0 | -58.9 (-68.0) |
| signalsmith_formants | 0 | 1.25 | 0.0 | 0.3 | 0.0 | 0.0 | 722 | 1062 | -0.0 | -0.0 | -58.9 (-68.0) |
| psola | 0 | 1.25 | 0.0 | 0.0 | 0.0 | 0.0 | 722 | 1062 | 0.0 | 0.0 | -68.7 (-68.0) |

### man_i_130 (F 270, 2290, 3010 Hz)

pYIN findet am Klang 99.2 % der Rahmen gesungen (PSOLA verschiebt nur, wo gesungen).

| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| signalsmith | -12 | 1 | 4.6 | 18.0 | – | -36.8 | 0 | 1446 | -0.6 | -19.6 | -54.0 (-90.0) |
| signalsmith_formants | -12 | 1 | 1.6 | 8.6 | 9.0 | -37.8 | 258 | 1424 | -4.5 | -8.0 | -29.3 (-90.0) |
| psola | -12 | 1 | 0.1 | 2.5 | 7.9 | 2.7 | 255 | 2351 | -1.3 | -2.9 | -78.6 (-90.0) |
| signalsmith | -7 | 1 | 0.8 | 14.3 | -33.0 | -33.3 | 159 | 1527 | 0.2 | -0.8 | -52.2 (-90.0) |
| signalsmith_formants | -7 | 1 | 0.7 | 4.8 | -25.6 | 2.2 | 176 | 2341 | -3.8 | -4.9 | -44.1 (-90.0) |
| psola | -7 | 1 | 0.3 | 1.8 | 4.7 | 2.1 | 248 | 2338 | -1.3 | -2.1 | -84.0 (-90.0) |
| signalsmith | -3 | 1 | 2.8 | 7.6 | -15.3 | -15.6 | 200 | 1931 | -0.1 | 2.0 | -54.8 (-90.0) |
| signalsmith_formants | -3 | 1 | 0.5 | 2.8 | -12.4 | -2.4 | 207 | 2235 | -2.4 | -0.3 | -52.9 (-90.0) |
| psola | -3 | 1 | 0.2 | 2.0 | -4.0 | -4.5 | 227 | 2186 | -1.8 | 0.3 | -86.2 (-90.0) |
| signalsmith | 3 | 1 | 1.0 | 8.8 | 18.8 | – | 281 | 0 | -0.2 | 4.7 | -63.7 (-90.0) |
| signalsmith_formants | 3 | 1 | 3.2 | 2.9 | 9.5 | -0.3 | 259 | 2283 | -2.4 | 2.5 | -57.3 (-90.0) |
| psola | 3 | 1 | 0.3 | 2.1 | 8.9 | 0.3 | 258 | 2296 | -3.1 | 1.7 | -90.0 (-90.0) |
| signalsmith | 7 | 1 | 0.4 | 15.0 | 50.1 | – | 355 | 0 | -0.4 | 4.6 | -58.2 (-90.0) |
| signalsmith_formants | 7 | 1 | 0.9 | 4.3 | – | 0.7 | 0 | 2305 | -4.1 | 0.8 | -56.1 (-90.0) |
| psola | 7 | 1 | 0.7 | 2.0 | – | 1.1 | 0 | 2316 | -5.6 | -0.7 | -90.0 (-90.0) |
| signalsmith | 12 | 1 | 1.0 | 17.0 | 100.4 | – | 474 | 0 | -0.2 | 0.4 | -53.7 (-90.0) |
| signalsmith_formants | 12 | 1 | 3.9 | 5.5 | – | 0.4 | 0 | 2300 | -1.7 | -1.2 | -64.8 (-90.0) |
| psola | 12 | 1 | 2.1 | 0.0 | – | 0.7 | 0 | 2305 | -0.5 | 0.0 | -90.0 (-90.0) |
| signalsmith | 0 | 1.25 | 0.1 | 0.6 | 0.0 | 0.0 | 237 | 2290 | -0.0 | -0.0 | -60.4 (-90.0) |
| signalsmith_formants | 0 | 1.25 | 0.1 | 0.6 | 0.0 | 0.0 | 237 | 2290 | -0.0 | -0.0 | -60.4 (-90.0) |
| psola | 0 | 1.25 | 0.1 | 0.0 | 0.0 | 0.0 | 237 | 2290 | 0.0 | 0.0 | -90.0 (-90.0) |

### woman_a_220 (F 850, 1220, 2810 Hz)

pYIN findet am Klang 99.8 % der Rahmen gesungen (PSOLA verschiebt nur, wo gesungen).

| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| signalsmith | -12 | 1 | 4.3 | 16.2 | – | – | 0 | 0 | -0.2 | -2.1 | -54.9 (-79.2) |
| signalsmith_formants | -12 | 1 | 1.6 | 5.6 | 0.9 | – | 901 | 0 | -2.4 | -4.5 | -34.5 (-79.2) |
| psola | -12 | 1 | 2.0 | 1.3 | -0.8 | – | 886 | 0 | -0.7 | -2.6 | -68.7 (-79.2) |
| signalsmith | -7 | 1 | 4.4 | 12.5 | -33.2 | – | 597 | 0 | -0.1 | -1.1 | -55.4 (-79.2) |
| signalsmith_formants | -7 | 1 | 2.6 | 4.4 | -4.1 | – | 857 | 0 | 0.5 | -0.5 | -50.8 (-79.2) |
| psola | -7 | 1 | 2.4 | 1.7 | -0.5 | – | 890 | 0 | -0.7 | -1.6 | -69.1 (-79.2) |
| signalsmith | -3 | 1 | 3.1 | 7.1 | -16.0 | – | 751 | 0 | -0.1 | 0.9 | -59.7 (-79.2) |
| signalsmith_formants | -3 | 1 | 2.6 | 3.6 | -1.4 | – | 881 | 0 | -1.2 | -0.2 | -54.5 (-79.2) |
| psola | -3 | 1 | 2.5 | 1.5 | 1.4 | – | 907 | 0 | -0.7 | 0.2 | -78.4 (-79.2) |
| signalsmith | 3 | 1 | 2.6 | 8.1 | – | – | 0 | 1062 | -0.2 | 3.3 | -61.2 (-79.2) |
| signalsmith_formants | 3 | 1 | 3.4 | 4.0 | – | – | 0 | 943 | -4.5 | -1.0 | -56.6 (-79.2) |
| psola | 3 | 1 | 2.3 | 1.5 | -5.3 | – | 846 | 0 | -2.1 | 1.5 | -75.7 (-79.2) |
| signalsmith | 7 | 1 | 1.7 | 14.6 | – | – | 0 | 1339 | -0.1 | 4.5 | -62.1 (-79.2) |
| signalsmith_formants | 7 | 1 | 2.8 | 5.3 | -9.1 | – | 812 | 0 | -5.9 | -1.3 | -56.9 (-79.2) |
| psola | 7 | 1 | 1.7 | 1.8 | 7.4 | – | 960 | 0 | -3.8 | 0.9 | -77.1 (-79.2) |
| signalsmith | 12 | 1 | 2.6 | 19.7 | – | – | 0 | 1791 | -0.3 | 0.4 | -60.3 (-79.2) |
| signalsmith_formants | 12 | 1 | 0.1 | 5.1 | -8.0 | – | 822 | 0 | -1.0 | -0.3 | -66.4 (-79.2) |
| psola | 12 | 1 | 0.1 | 0.0 | -4.3 | – | 855 | 0 | -0.7 | 0.0 | -82.5 (-79.2) |
| signalsmith | 0 | 1.25 | 0.1 | 0.5 | -0.0 | – | 894 | 0 | -0.0 | -0.0 | -62.3 (-79.2) |
| signalsmith_formants | 0 | 1.25 | 0.1 | 0.5 | -0.0 | – | 894 | 0 | -0.0 | -0.0 | -62.3 (-79.2) |
| psola | 0 | 1.25 | 0.1 | 0.0 | 0.0 | – | 894 | 0 | 0.0 | 0.0 | -77.6 (-79.2) |

### woman_u_262 (F 370, 950, 2670 Hz)

pYIN findet am Klang 99.6 % der Rahmen gesungen (PSOLA verschiebt nur, wo gesungen).

| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| signalsmith | -12 | 1 | 0.4 | 13.9 | – | – | 0 | 0 | -0.2 | -7.3 | -57.5 (-86.4) |
| signalsmith_formants | -12 | 1 | 0.3 | 7.1 | – | – | 257 | 1170 | 2.0 | -0.7 | -45.6 (-86.4) |
| psola | -12 | 1 | 0.9 | 4.9 | – | – | 267 | 0 | -1.4 | -3.9 | -79.6 (-86.4) |
| signalsmith | -7 | 1 | 7.8 | 12.1 | – | – | 0 | 0 | -0.1 | -4.6 | -62.5 (-86.4) |
| signalsmith_formants | -7 | 1 | 0.6 | 6.1 | – | – | 293 | 0 | 0.2 | -1.0 | -57.0 (-86.4) |
| psola | -7 | 1 | 1.6 | 6.2 | – | – | 0 | 944 | -1.5 | -2.8 | -78.6 (-86.4) |
| signalsmith | -3 | 1 | 0.5 | 7.9 | – | – | 0 | 0 | -0.2 | -2.0 | -62.4 (-86.4) |
| signalsmith_formants | -3 | 1 | 0.2 | 4.7 | – | – | 0 | 0 | -1.0 | -1.8 | -59.9 (-86.4) |
| psola | -3 | 1 | 1.2 | 8.5 | – | – | 0 | 0 | -1.5 | -3.7 | -84.5 (-86.4) |
| signalsmith | 3 | 1 | 1.3 | 7.3 | – | – | 0 | 0 | -0.2 | 1.3 | -65.2 (-86.4) |
| signalsmith_formants | 3 | 1 | 1.5 | 5.1 | – | – | 0 | 0 | -2.3 | -0.7 | -64.5 (-86.4) |
| psola | 3 | 1 | 1.3 | 5.1 | – | – | 0 | 0 | -1.8 | -0.3 | -86.7 (-86.4) |
| signalsmith | 7 | 1 | 0.9 | 12.4 | – | – | 0 | 0 | -0.2 | 3.1 | -62.3 (-86.4) |
| signalsmith_formants | 7 | 1 | 2.5 | 7.0 | – | – | 0 | 0 | -5.5 | -2.1 | -59.8 (-86.4) |
| psola | 7 | 1 | 1.0 | 4.2 | – | – | 0 | 0 | -6.2 | -2.8 | -83.0 (-86.4) |
| signalsmith | 12 | 1 | 0.2 | 12.9 | – | – | 0 | 0 | -0.2 | 5.3 | -63.9 (-86.4) |
| signalsmith_formants | 12 | 1 | 0.6 | 6.2 | – | – | 0 | 0 | -7.0 | -1.4 | -66.4 (-86.4) |
| psola | 12 | 1 | 0.1 | 0.0 | – | – | 0 | 0 | -5.5 | -0.0 | -87.5 (-86.4) |
| signalsmith | 0 | 1.25 | 0.0 | 0.7 | – | – | 0 | 0 | -0.0 | -0.0 | -67.4 (-86.4) |
| signalsmith_formants | 0 | 1.25 | 0.0 | 0.7 | – | – | 0 | 0 | -0.0 | -0.0 | -67.4 (-86.4) |
| psola | 0 | 1.25 | 0.0 | 0.0 | – | – | 0 | 0 | 0.0 | 0.0 | -85.6 (-86.4) |

### woman_a_330 (F 850, 1220, 2810 Hz)

pYIN findet am Klang 99.8 % der Rahmen gesungen (PSOLA verschiebt nur, wo gesungen).

| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| signalsmith | -12 | 1 | 0.7 | 15.8 | – | – | 0 | 1267 | -0.2 | -3.0 | -59.9 (-63.0) |
| signalsmith_formants | -12 | 1 | 0.2 | 6.7 | -28.9 | – | 618 | 1350 | -0.8 | -3.6 | -38.4 (-63.0) |
| psola | -12 | 1 | 0.1 | 1.6 | 9.9 | – | 955 | 0 | -0.8 | -3.4 | -73.3 (-63.0) |
| signalsmith | -7 | 1 | 1.7 | 11.6 | -33.4 | – | 578 | 0 | -0.1 | -1.5 | -58.4 (-63.0) |
| signalsmith_formants | -7 | 1 | 0.8 | 5.9 | -15.4 | – | 735 | 0 | -0.8 | -2.1 | -52.8 (-63.0) |
| psola | -7 | 1 | 0.1 | 2.9 | -7.6 | – | 803 | 0 | -0.8 | -2.1 | -62.2 (-63.0) |
| signalsmith | -3 | 1 | 0.3 | 6.4 | -15.3 | – | 736 | 0 | -0.1 | -0.1 | -59.3 (-63.0) |
| signalsmith_formants | -3 | 1 | 0.2 | 4.5 | -21.5 | – | 682 | 998 | -1.7 | -1.7 | -55.1 (-63.0) |
| psola | -3 | 1 | 0.1 | 3.0 | 7.0 | – | 929 | 0 | -0.9 | -0.9 | -62.4 (-63.0) |
| signalsmith | 3 | 1 | 0.2 | 7.1 | – | – | 0 | 1034 | 0.0 | 1.6 | -61.0 (-63.0) |
| signalsmith_formants | 3 | 1 | 0.2 | 4.9 | 4.4 | – | 907 | 0 | -3.6 | -2.1 | -56.4 (-63.0) |
| psola | 3 | 1 | 0.2 | 3.2 | 5.8 | – | 919 | 0 | -2.2 | -0.7 | -63.9 (-63.0) |
| signalsmith | 7 | 1 | 0.4 | 12.7 | – | – | 0 | 1296 | -0.1 | 2.1 | -60.0 (-63.0) |
| signalsmith_formants | 7 | 1 | 1.9 | 7.5 | 1.8 | – | 884 | 0 | -2.2 | -0.0 | -60.5 (-63.0) |
| psola | 7 | 1 | 0.3 | 2.5 | 12.2 | – | 975 | 0 | -2.8 | -0.6 | -63.6 (-63.0) |
| signalsmith | 12 | 1 | 0.6 | 18.8 | – | – | 0 | 1757 | -0.2 | 2.9 | -60.6 (-63.0) |
| signalsmith_formants | 12 | 1 | 0.9 | 8.0 | – | – | 0 | 0 | -3.3 | -0.2 | -63.1 (-63.0) |
| psola | 12 | 1 | 2.3 | 0.0 | – | – | 0 | 0 | -3.1 | 0.0 | -66.5 (-63.0) |
| signalsmith | 0 | 1.25 | 0.1 | 0.4 | -0.0 | – | 869 | 0 | -0.0 | -0.0 | -63.7 (-63.0) |
| signalsmith_formants | 0 | 1.25 | 0.1 | 0.4 | -0.0 | – | 869 | 0 | -0.0 | -0.0 | -63.7 (-63.0) |
| psola | 0 | 1.25 | 0.1 | 0.0 | 0.0 | – | 869 | 0 | 0.0 | 0.0 | -63.8 (-63.0) |

### woman_i_392 (F 310, 2790, 3310 Hz)

pYIN findet am Klang 99.8 % der Rahmen gesungen (PSOLA verschiebt nur, wo gesungen).

| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| signalsmith | -12 | 1 | 1.4 | 22.2 | – | – | 0 | 0 | 0.2 | -17.9 | -64.4 (-71.7) |
| signalsmith_formants | -12 | 1 | 1.3 | 7.6 | – | – | 0 | 0 | -2.4 | -7.8 | -40.2 (-71.7) |
| psola | -12 | 1 | 0.1 | 1.9 | – | -2.3 | 400 | 2717 | -1.1 | -2.4 | -87.0 (-71.7) |
| signalsmith | -7 | 1 | 0.6 | 17.3 | – | -33.3 | 0 | 1856 | -0.2 | -11.7 | -55.5 (-71.7) |
| signalsmith_formants | -7 | 1 | 0.7 | 6.8 | – | – | 0 | 0 | -2.6 | -5.6 | -48.3 (-71.7) |
| psola | -7 | 1 | 0.9 | 3.5 | – | -3.3 | 395 | 2690 | -1.1 | 1.9 | -73.1 (-71.7) |
| signalsmith | -3 | 1 | 1.1 | 9.0 | – | -15.9 | 0 | 2339 | -0.0 | -5.3 | -66.8 (-71.7) |
| signalsmith_formants | -3 | 1 | 1.2 | 3.5 | – | -4.1 | 0 | 2668 | -1.3 | -2.8 | -63.4 (-71.7) |
| psola | -3 | 1 | 1.1 | 2.7 | – | -6.3 | 0 | 2605 | -0.8 | 2.0 | -73.3 (-71.7) |
| signalsmith | 3 | 1 | 1.8 | 8.8 | – | – | 0 | 0 | -0.3 | 3.3 | -67.0 (-71.7) |
| signalsmith_formants | 3 | 1 | 0.5 | 3.7 | – | 2.6 | 0 | 2854 | -1.9 | 1.7 | -68.4 (-71.7) |
| psola | 3 | 1 | 2.0 | 1.1 | – | 0.1 | 0 | 2784 | -0.7 | 2.8 | -73.8 (-71.7) |
| signalsmith | 7 | 1 | 1.0 | 12.4 | – | – | 0 | 0 | -0.0 | 9.1 | -65.8 (-71.7) |
| signalsmith_formants | 7 | 1 | 1.1 | 6.5 | – | 7.8 | 0 | 3000 | -4.2 | 4.9 | -66.7 (-71.7) |
| psola | 7 | 1 | 1.1 | 2.7 | – | – | 0 | 0 | -5.2 | 4.0 | -71.9 (-71.7) |
| signalsmith | 12 | 1 | 1.4 | 8.4 | – | – | 0 | 0 | -0.1 | 18.1 | -65.7 (-71.7) |
| signalsmith_formants | 12 | 1 | 1.4 | 8.9 | – | – | 0 | 0 | -8.7 | 9.5 | -69.4 (-71.7) |
| psola | 12 | 1 | 0.7 | 0.0 | – | – | 0 | 0 | -18.2 | 0.0 | -60.0 (-71.7) |
| signalsmith | 0 | 1.25 | 0.0 | 0.0 | – | 0.0 | 0 | 2782 | -0.0 | -0.0 | -68.7 (-71.7) |
| signalsmith_formants | 0 | 1.25 | 0.0 | 0.0 | – | 0.0 | 0 | 2782 | -0.0 | -0.0 | -68.7 (-71.7) |
| psola | 0 | 1.25 | 0.0 | 0.0 | – | 0.0 | 0 | 2782 | -0.0 | 0.0 | -72.6 (-71.7) |

### woman_a_440 (F 850, 1220, 2810 Hz)

pYIN findet am Klang 99.6 % der Rahmen gesungen (PSOLA verschiebt nur, wo gesungen).

| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| signalsmith | -12 | 1 | 7.0 | 16.8 | – | – | 0 | 0 | -0.1 | -1.1 | -62.8 (-87.5) |
| signalsmith_formants | -12 | 1 | 0.2 | 11.5 | – | – | 0 | 1270 | -7.1 | -13.2 | -39.7 (-87.5) |
| psola | -12 | 1 | 0.1 | 2.5 | -0.5 | – | 885 | 0 | -1.1 | -2.0 | -79.9 (-87.5) |
| signalsmith | -7 | 1 | 6.1 | 12.3 | -33.1 | – | 595 | 0 | -0.2 | -0.5 | -62.5 (-87.5) |
| signalsmith_formants | -7 | 1 | 3.5 | 11.5 | -22.8 | – | 686 | 0 | -4.2 | -4.6 | -54.3 (-87.5) |
| psola | -7 | 1 | 2.2 | 5.0 | -0.0 | – | 889 | 0 | -1.0 | -1.4 | -80.1 (-87.5) |
| signalsmith | -3 | 1 | 2.4 | 6.8 | -15.9 | – | 748 | 0 | -0.0 | 4.4 | -66.5 (-87.5) |
| signalsmith_formants | -3 | 1 | 2.3 | 5.1 | -11.7 | – | 785 | 0 | -2.4 | 2.0 | -63.1 (-87.5) |
| psola | -3 | 1 | 2.3 | 3.7 | -2.8 | – | 864 | 0 | -1.5 | 3.0 | -84.3 (-87.5) |
| signalsmith | 3 | 1 | 1.2 | 7.0 | – | – | 0 | 1060 | -0.2 | 6.1 | -64.5 (-87.5) |
| signalsmith_formants | 3 | 1 | 2.5 | 4.6 | – | – | 0 | 1010 | -3.4 | 2.9 | -63.8 (-87.5) |
| psola | 3 | 1 | 1.1 | 3.9 | – | – | 0 | 980 | -2.7 | 3.5 | -84.4 (-87.5) |
| signalsmith | 7 | 1 | 1.6 | 14.4 | – | – | 0 | 1332 | -0.1 | 9.0 | -62.8 (-87.5) |
| signalsmith_formants | 7 | 1 | 1.7 | 7.4 | – | – | 0 | 1134 | -7.9 | 1.3 | -56.8 (-87.5) |
| psola | 7 | 1 | 1.6 | 2.2 | – | – | 0 | 0 | -5.7 | 3.5 | -81.8 (-87.5) |
| signalsmith | 12 | 1 | 2.6 | 24.4 | – | – | 0 | 1783 | -0.3 | -0.1 | -63.1 (-87.5) |
| signalsmith_formants | 12 | 1 | 2.6 | 9.9 | – | – | 0 | 0 | -10.2 | -10.1 | -61.2 (-87.5) |
| psola | 12 | 1 | 0.1 | 0.0 | – | – | 0 | 0 | -0.1 | 0.0 | -90.0 (-87.5) |
| signalsmith | 0 | 1.25 | 0.1 | 0.2 | -0.0 | – | 889 | 0 | -0.0 | -0.0 | -71.0 (-87.5) |
| signalsmith_formants | 0 | 1.25 | 0.1 | 0.2 | -0.0 | – | 889 | 0 | -0.0 | -0.0 | -71.0 (-87.5) |
| psola | 0 | 1.25 | 0.1 | 0.0 | 0.0 | – | 889 | 0 | 0.0 | 0.0 | -87.1 (-87.5) |

### man_a_165_vibrato (F 730, 1090, 2440 Hz, Vibrato ±40 Cent)

pYIN findet am Klang 99.6 % der Rahmen gesungen (PSOLA verschiebt nur, wo gesungen).

| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| signalsmith | -12 | 1 | 3.6 | – | – | – | 0 | 0 | -0.1 | – | – |
| signalsmith_formants | -12 | 1 | 4.5 | – | – | – | 0 | 0 | 0.6 | – | – |
| psola | -12 | 1 | 2.5 | – | – | – | 0 | 0 | -0.7 | – | – |
| signalsmith | -7 | 1 | 2.2 | – | – | – | 0 | 0 | -0.2 | – | – |
| signalsmith_formants | -7 | 1 | 2.2 | – | – | – | 0 | 0 | 0.7 | – | – |
| psola | -7 | 1 | 1.1 | – | – | – | 0 | 0 | -0.7 | – | – |
| signalsmith | -3 | 1 | 2.0 | – | – | – | 0 | 0 | -0.1 | – | – |
| signalsmith_formants | -3 | 1 | 1.6 | – | – | – | 0 | 0 | -0.2 | – | – |
| psola | -3 | 1 | 1.3 | – | – | – | 0 | 0 | -0.6 | – | – |
| signalsmith | 3 | 1 | 1.3 | – | – | – | 0 | 0 | -0.1 | – | – |
| signalsmith_formants | 3 | 1 | 1.6 | – | – | – | 0 | 0 | -3.3 | – | – |
| psola | 3 | 1 | 0.9 | – | – | – | 0 | 0 | -1.5 | – | – |
| signalsmith | 7 | 1 | 1.9 | – | – | – | 0 | 0 | -0.2 | – | – |
| signalsmith_formants | 7 | 1 | 2.2 | – | – | – | 0 | 0 | -5.4 | – | – |
| psola | 7 | 1 | 1.6 | – | – | – | 0 | 0 | -3.8 | – | – |
| signalsmith | 12 | 1 | 2.4 | – | – | – | 0 | 0 | -0.5 | – | – |
| signalsmith_formants | 12 | 1 | 3.0 | – | – | – | 0 | 0 | -3.4 | – | – |
| psola | 12 | 1 | 0.2 | – | – | – | 0 | 0 | -1.9 | – | – |
| signalsmith | 0 | 1.25 | 2.6 | – | – | – | 0 | 0 | -0.0 | – | – |
| signalsmith_formants | 0 | 1.25 | 2.6 | – | – | – | 0 | 0 | -0.0 | – | – |
| psola | 0 | 1.25 | 0.2 | – | – | – | 0 | 0 | 0.1 | – | – |

### Rechenzeit (Release oder Testing, wie gebaut)

| Umsetzung | % der Echtzeit |
|---|---|
| psola | 3.56 |
| signalsmith | 1.63 |
| signalsmith_formants | 4.90 |
