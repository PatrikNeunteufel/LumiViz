> **Kopf (Claude, S168, 2026-10-31):** Bericht eines Recherche-Agenten (Opus 5.5, ohne Browser, nur curl
> und WebSearch), unverändert übernommen. Auftrag: Verfahren und Bibliotheken für Stufe 15 Wave-Clips
> (`Loop_Konzept.md`, Entscheide WV1 a, WV1-1 a, WV4 a) — Zeitdehnung ohne Tonhöhe, Tonhöhe ohne
> Längenänderung mit Formant-Erhalt, Tonhöhenverlauf an Gesang; Lizenz neben der proprietären BASS-DLL.
> **Stichprobe selbst nachgelesen (S168, curl):** Signalsmith Stretch `LICENSE.txt` „MIT License /
> Copyright (c) 2022 Geraint Luff / Signalsmith Audio Ltd.“; im Header `SignalsmithStretch(long seed)`,
> `setFormantFactor(Sample multiplier, bool compensatePitch=false)`, `setFormantBase(Sample baseFreq=0)`;
> Rubber Band README „commercial licence from us before doing so“; WORLD `LICENSE.txt` drei Klauseln
> (Quelltext, Binärform, kein Name zur Werbung) — BSD-3-artig. Übrige Angaben nicht nachgeprüft.
> Lizenzfragen: keine Rechtsberatung.

# Recherche: Zeitdehnung, Tonhöhe und Tonhöhenmessung für Wave-Clips in LumiViz

Abgerufen am 31.10.2026, nur mit curl (Rohdateien von GitHub, Codeberg, GitHub-API) und WebSearch. Kein Browser, nichts heruntergeladen und ausgeführt. Was in Anführungszeichen steht, ist Wortlaut aus der genannten URL. Eigene Einschätzungen sind mit **[Einschätzung]** markiert. Lizenzrechtlich ist das keine Rechtsberatung.

**Gesperrt:** `https://breakfastquay.com/rubberband/` (Lizenzseite von Rubber Band) war nicht erreichbar, über https kam ECONNREFUSED, über http eine leere Antwort. Ich habe es nicht umgangen. Die Bedingungen der kommerziellen Lizenz (Preis, Umfang) sind deshalb **nicht belegt**.

---

## 1. Übersicht: Zeitdehnung und Tonhöhe

| Kandidat | Lizenz (belegt) | Verfahren | Formant-Erhalt | Build / Abhängigkeiten | Letzte Version | Neben BASS **[Einschätzung]** |
|---|---|---|---|---|---|---|
| **Signalsmith Stretch** | MIT | STFT-/Spektralverfahren | ja, `setFormantFactor`, `setFormantSemitones`, `setFormantBase`; laut README „nicht so scharf wie PSOLA“ | header-only (`signalsmith-stretch.h`) plus Signalsmith Linear (MIT); CMakeLists vorhanden | Tag 1.4.0, Commit vom 25.09.2026, Push 22.10.2026 | unproblematisch |
| **Bungee** (Parabolic/Parabola Research) | MPL-2.0 | Phase-Vocoder, körnig („grains“, ~100/s) | **nein** in der Open-Source-Fassung (0 Treffer für „formant“ in `src/` und `bungee/`); nur Bungee Pro (kommerziell) wirbt damit | CMake, C++; Eigen und PFFFT als Submodule | v2.4.39 vom 30.10.2026 | verträglich, Copyleft nur je Datei |
| **SoundTouch** | LGPL 2.1 | „WSOLA-like“ (Zeitbereich); Tonhöhe = Dehnen plus Umtasten | **nein** (0 Treffer für „formant“ im README) | C++, CMake, Option `SOUNDTOUCH_DLL` | 2.4.1 vom 29.03.2026 | verträglich bei dynamischem Einbinden (DLL) |
| **Rubber Band Library** | GPL v2 oder später, **oder kommerziell** | R2 „Faster“ (Phase-Vocoder) und R3 „Finer“ | ja, `OptionFormantPreserved`; `setFormantScale` nur unter R3 | Meson; Single-File-Build `single/RubberBandSingle.cpp` | v4.0.0, Commit vom 25.10.2024, Push 03.03.2025 | **nicht ohne kommerzielle Lizenz** |
| **Audacity StaffPad TimeAndPitch** und `FormantShifter` | Teil von Audacity; `FormantShifter.h` trägt „SPDX-License-Identifier: GPL-2.0-or-later“; StaffPad-Dateien ohne eigene Lizenzzeile | „Phase-locked Vocoder“ | ja (`FormantShifter`, GPL) | in den Audacity-Baum eingebunden | Audacity master | GPL, wie Rubber Band ohne kommerzielle Option |
| **WORLD** (M. Morise) | BSD-artig (Wortlaut unten) | Vocoder: Analyse in F0, Hüllkurve und Aperiodizität, dann Synthese | ja, von der Bauart her (die Hüllkurve wird getrennt geschätzt) | C++, Quellen unter `src/` | Push 09.10.2026 | unproblematisch |
| **Sonic** | Apache-2.0 (GitHub-API) | für Sprache („speeding up or slowing down speech“) | nicht belegt | ANSI C | Push 14.03.2026 | unproblematisch, aber für Sprache gebaut, nicht für Gesang |
| **élastique** (zplane) | kommerziell | — | — | — | — | nicht geprüft (Auftrag: nur kurz) |

---

## 2. Die Kandidaten im Einzelnen

### 2.1 Signalsmith Stretch
- **Lizenz:** „MIT License / Copyright (c) 2022 Geraint Luff / Signalsmith Audio Ltd.“, aus https://raw.githubusercontent.com/Signalsmith-Audio/signalsmith-stretch/main/LICENSE.txt. Das README sagt dazu: „Released under the MIT License - get in touch if you need anything else.“ Die Abhängigkeit Signalsmith-Audio/linear ist laut GitHub-API ebenfalls MIT.
- **Verfahren:** „This is a C++11 library for pitch and time stretching, using the final approach from the ADC22 presentation Four Ways To Write A Pitch-Shifter.“ Der Header bindet `signalsmith-linear/stft.h` ein, rechnet also im STFT-Bereich. Zur Spanne: „time-stretching sounds best for more modest changes (between 0.75x and 1.5x)“. Für Tonhöhe: „multiple octaves“. Quelle ist das README im selben Repo.
- **Formanten:** Im README `stretch.setFormantFactor(1.2)` und `setFormantBase(200/sampleRate)`, dazu der Satz „The formant correction is not a sharp as monophonic algorithms (such such as PSOLA). It also needs you to give it a rough estimate of fundamental frequency“. Im Header stehen `setFormantFactor(Sample multiplier, bool compensatePitch=false)`, `setFormantSemitones(...)`, `setFormantBase(Sample baseFreq=0)` und `setFreqMap(std::function<Sample(Sample)>)`.
- **API:** `presetDefault`, `presetCheaper`, `setTransposeFactor`, `setTransposeSemitones`, `process(inputs, inSamples, outputs, outSamples)`. Zeitdehnung entsteht über verschieden lange Ein- und Ausgabepuffer. Mit `seek` und `flush` „you can perform an exact time-stretch on a fixed-length sound“ (README).
- **Determinismus, belegt im Header:**
  - Der Standard-Konstruktor seedet mit `std::random_device{}()`. Es gibt aber einen Konstruktor mit Seed: `SignalsmithStretch(long seed)`.
  - Die Phasen werden erst ab einer Dehnung von 2 zufällig gesetzt: `maxCleanStretch{2}; // time-stretch ratio before we start randomising phases`.
  - Ohne eigenen `RandomEngine` nimmt die Klasse `std::default_random_engine`.
  - **[Einschätzung]** `default_random_engine` ist je Standardbibliothek verschieden. clang in der VS-Umgebung nutzt dieselbe MSVC-STL, dort ist es also gleich. Sicherer ist es, einen festen Engine-Typ wie `std::mt19937` als Template-Argument und einen festen Seed zu übergeben.
- **Für Gesang** **[Einschätzung]**: gut für Tempo in mäßiger Spanne und für Transponieren mit Formant-Ausgleich. Als „Autotune“ mit Tonhöhenkurve ginge `setTransposeSemitones` je Block. Nicht belegt ist, wie glatt das bei schnellen Änderungen klingt.

### 2.2 Bungee
- **Lizenz:** Die Datei LICENSE beginnt mit „Mozilla Public License Version 2.0“ (https://raw.githubusercontent.com/bungee-audio-stretch/bungee/main/LICENSE). Im README steht: „Bungee is permissively licensed under the Mozilla Public License Version 2.0.“
- **Verfahren:** „Simple, fast phase-vocoder-based algorithm“. Die Parameter werden je Grain gesetzt (`Request::position`, `speed`, `pitch`), mit den Aufrufen `specifyGrain`, `analyseGrain`, `synthesiseGrain`. Laut README sind auch Geschwindigkeit 0 und rückwärts möglich.
- **Formanten:** nein. Ich habe alle Dateien unter `src/` und `bungee/` abgerufen, keine enthält „formant“. Erhalt der Klangfarbe verspricht nur die kommerzielle Fassung: Bungee Pro „preserve vocal and instrumental timbre“ (README).
- **Abhängigkeiten:** „The Eigen C++ library … The PFFFT library for Fast Fourier Transforms“ (README, `.gitmodules`). Gebaut wird mit CMake.
- **Pflege:** Release v2.4.39 vom 30.10.2026 (GitHub-API). „Every commit pushed to this repo's main branch is automatically tagged“.
- **[Einschätzung] zur MPL-2.0:** Das Copyleft gilt nur je Datei. Geänderte Bungee-Dateien müssen offen bleiben, der Rest des Programms und BASS sind davon nicht berührt. Für (2) mit Gesang reicht Bungee nicht, weil die Formanten fehlen.

### 2.3 SoundTouch
- **Lizenz:** COPYING.TXT beginnt mit „GNU LESSER GENERAL PUBLIC LICENSE / Version 2.1, February 1999“ (https://codeberg.org/soundtouch/soundtouch/raw/branch/master/COPYING.TXT). Im Kopf von `SoundTouch.cpp` steht „version 2.1 of the License, or (at your option) any later version“.
- **Verfahren:** „SoundTouch uses WSOLA-like time-stretching routines that operate in the time domain.“ Die Tonhöhe entsteht so: „'Pitch' control is implemented as a combination of time-stretching and sample rate transposing“, mit „linear interpolation with anti-alias filtering“. Quelle: README.html im Repo.
- **Formanten:** nein, „formant“ kommt im README nicht vor.
- **Build:** CMake, `cmake . -DSOUNDTOUCH_DLL=ON`. Proben als float (Vorgabe) oder 16 Bit.
- **Pflege:** Tags 2.4.1 (29.03.2026) und 2.4.0 (06.04.2025), Codeberg-API.
- **[Einschätzung]:** Unter LGPL ist eine eigene DLL neben BASS unproblematisch. Statisches Linken verlangt, dass Nutzer die Bibliothek austauschen und neu linken können. Für Gesang taugt das Verfahren für Tempo; die Tonhöhe verschiebt die Formanten mit („Mickey-Mouse“). Kein Kandidat für (2).

### 2.4 Rubber Band Library
- **Lizenz:** COPYING beginnt mit „GNU GENERAL PUBLIC LICENSE / Version 2, June 1991“. Das README sagt: „Rubber Band Library is distributed under the GNU General Public License (GPL)… either version 2 of the License, or (at your option) any later version“ und weiter „If you wish to distribute code using Rubber Band Library under terms other than those of the GNU General Public License, you must obtain a commercial licence from us before doing so.“ Quelle: https://raw.githubusercontent.com/breakfastquay/rubberband/default/README.md
- **Mit BASS:** Belegt ist nur, dass für eine Verteilung zu anderen Bedingungen als der GPL eine kommerzielle Lizenz nötig ist. **[Einschätzung]** Unter GPL müsste das ganze verteilte Programm GPL-verträglich sein; die proprietäre BASS-DLL ist das nicht. Ohne kommerzielle Lizenz scheidet Rubber Band also aus, sobald LumiViz als Programm weitergegeben wird.
- **Verfahren:** „R2 (Faster) engine and the R3 (Finer) engine. The R3 engine produces higher-quality results than R2 for most material, especially complex mixes, vocals and other sounds that have soft onsets and smooth pitch changes“ (README). Offline arbeitet es in zwei Durchgängen: `OptionProcessOffline`, zuerst `study()`, dann `process()`.
- **Formanten**, aus `rubberband/RubberBandStretcher.h`:
  - „OptionFormantPreserved - Preserve the spectral envelope of the unshifted signal“; laut Header für beide Engines.
  - `setFormantScale(double scale)` mit dem Vermerk „supported only in the R3 (OptionEngineFiner) engine“.
- **Threads:** `OptionThreadingNever` existiert. Ob die Ergebnisse deterministisch sind, ist **nicht belegt**.
- **Build:** Meson. Dazu der Single-File-Build: „a single `.cpp` file at `single/RubberBandSingle.cpp`“ (COMPILING.md).
- **Pflege:** v4.0.0 (Commit vom 25.10.2024; neu ist `RubberBandLiveShifter`), letzter Push 03.03.2025.
- **Nach der Qualität** **[Einschätzung]:** für Gesang mit Formanten der stärkste Kandidat. Rechtlich aber nur mit kommerzieller Lizenz, deren Bedingungen ich nicht abrufen konnte.

### 2.5 Audacity: StaffPad TimeAndPitch und FormantShifter
- **Ort:** `au3/libraries/au3-time-and-pitch/` im Repo audacity/audacity, unter anderem `StaffPad/TimeAndPitch.h|cpp`, `FormantShifter.h|cpp`, `StaffPadTimeAndPitch.h`.
- **Verfahren:** Im Kopf von `TimeAndPitch.h` steht „Arbitrary time and pitch stretching, based on the Phase-locked Vocoder.“ Laut `StaffPad/readme.md`: „thoroughly tested in the range [0.5, 2.0]“.
- **Lizenz:** `FormantShifter.h` trägt „SPDX-License-Identifier: GPL-2.0-or-later“. Die StaffPad-Dateien haben **keine eigene Lizenzzeile**, eine eigene Lizenz für StaffPad ist nicht belegt; sie gelten im Audacity-Baum als Teil von Audacity. Die Websuche fand dazu nichts.
- **Determinismus:** Die Zufallsphasen kommen aus `std::mt19937`, gesetzt mit `randomGenerator.seed(0)` (TimeAndPitch.cpp, Zeile ~61), sind also reproduzierbar.
- **[Einschätzung]:** Wegen der GPL wie Rubber Band nicht einsetzbar. Lesen lohnt sich als Vorbild: ein Phase-Vocoder mit getrenntem Formant-Schieber.

### 2.6 WORLD-Vocoder (Morise)
- **Lizenz:** LICENSE.txt enthält „Copyright (c) 2010 M. Morise / All rights reserved. / Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:“ (https://raw.githubusercontent.com/mmorise/World/master/LICENSE.txt). Die GitHub-API meldet „NOASSERTION“. **[Einschätzung]** Das ist eine BSD-artige Lizenz; wie viele Klauseln sie hat, habe ich nicht bis zu Ende gelesen.
- **Verfahren:** „It can estimate Fundamental frequency (F0), aperiodicity and spectral envelope and also generate the speech like input speech with only estimated parameters.“ (README)
- **Funktionen unter `src/world/`:** `Harvest(...)` und `Dio` für F0, `StoneMask` zum Verfeinern von F0, `CheapTrick` für die Hüllkurve, `D4C` für die Aperiodizität, `Synthesis(...)` und `synthesisrealtime.h`. `HarvestOption` hat `f0_floor`, `f0_ceil` und `frame_period` in ms.
- **Formanten:** Hüllkurve und F0 sind getrennt. Wer F0 ändert und die Hüllkurve lässt, erhält die Formanten; wer die Rahmen neu taktet, ändert die Dauer.
- **Determinismus, belegt:** Das Rauschen ist ein Xorshift mit festem Seed: `randn_reseed` setzt `g_randn_x = 123456789` … (`matlabfunctions.cpp`).
- **Für Gesang** **[Einschätzung]:** gebaut für Sprache, für Gesang üblich (Sängersynthese). Es deckt (2) und (3) mit einem Verfahren ab: Tonhöhenkurve mit Harvest, Ändern mit Synthesis. Grenzen: nur einstimmig, und Atem und Rauheit klingen nach dem Vocoder etwas „vocoderhaft“ (nicht belegt).

### 2.7 Sonic
- **Lizenz:** Apache-2.0 laut GitHub-API; COPYING liefert 404, die Datei heißt LICENSE. „Sonic is a simple algorithm for speeding up or slowing down speech“ (README). Gebaut für Sprache; als Gesangsweg nicht weiter geprüft.

### 2.8 élastique (zplane)
Kommerziell, nicht weiter geprüft.

---

## 3. Tonhöhenmessung über die Zeit

| Kandidat | Lizenz (belegt) | Sprache | Verlauf über die Zeit | Bemerkung |
|---|---|---|---|---|
| **pYIN, Vamp-Plugin** (c4dm/pyin, QMUL) | GPL v2 oder später | C++ | ja (HMM/Viterbi: `MonoPitchHMM`, `SparseHMM`) | Referenz-Umsetzung; GPL |
| **sevagh/pitch-detection** | MIT | C++ | nein, ein Wert je Block | YIN, MPM, pYIN, „Probabilistic MPM“; braucht ffts und mlpack; nur Linux und gcc offiziell |
| **librosa `pyin`** | ISC | Python | ja | als Vorlage zum Lesen, nicht zum Einbinden |
| **WORLD `Harvest` / `Dio`** | BSD-artig | C++ | ja (`frame_period`) | siehe 2.6 |
| **Essentia** | AGPL v3 | C++ | ja | AGPL, scheidet aus **[Einschätzung]** |
| **aubio** | GPL v3 | C | ja | GPL |
| **CREPE** | MIT | Python, TensorFlow/Keras | ja (`viterbi=True`) | schwere Abhängigkeit |
| **SwiftF0** | MIT | Python, ONNX | ja (16-ms-Rahmen) | 14 386 Parameter, 46,875–2093,75 Hz |

Belege je Kandidat:
- **pYIN Vamp:** COPYING beginnt mit „GNU GENERAL PUBLIC LICENSE / Version 2, June 1991“. Kopf von `PYinVamp.cpp`: „pYIN - A fundamental frequency estimator for monophonic audio / Centre for Digital Music, Queen Mary, University of London. … either version 2 of the License, or (at your option) any later version.“ Dateien: `Yin.cpp`, `YinUtil.cpp`, `MonoPitchHMM.cpp`, `SparseHMM.cpp`, `MonoNote*.cpp`, `LocalCandidatePYIN.cpp`. Letzter Push 02.07.2024. Quelle: https://github.com/c4dm/pyin (Zweig `default`).
- **sevagh/pitch-detection:** LICENSE beginnt mit „MIT License / Copyright (c) 2018 Sevag Hanssian“. Aus dem README: „The size of the FFT used is the same as the size of the input waveform, such that the output is a single pitch for the entire waveform“ und „The library depends on ffts and mlpack“ sowie „You need Linux, cmake, and gcc (I don't officially support other platforms)“. Aufrufe: `pitch::mpm`, `pitch::yin`, `pitch::pyin<double>`. Release v2023.12, Push 07.01.2025.
- **librosa:** LICENSE.md beginnt mit „ISC License / Copyright (c) 2013--2026, librosa development team.“ Dass dort `pyin` liegt, habe ich in dieser Sitzung **nicht am Quelltext geprüft** (bekannt, aber nicht belegt).
- **Essentia:** COPYING.txt: „GNU AFFERO GENERAL PUBLIC LICENSE / Version 3“. Dass es pYIN anbietet, habe ich nicht abgerufen.
- **aubio:** COPYING: „GNU GENERAL PUBLIC LICENSE / Version 3, 29 June 2007“. Das yinfft-Verfahren habe ich am Quelltext nicht geprüft.
- **CREPE:** Das README zeigt ein MIT-Badge und nennt `pip install --upgrade tensorflow`, `--model-capacity {tiny|small|medium|large|full}`, `crepe.predict(audio, sr, viterbi=True)` und „The model is trained on 16 kHz audio“. Letzte Release v0.0.7 vom 13.07.2018. Die Parameterzahl ist **nicht belegt**. torchcrepe ist laut LICENSE ebenfalls MIT.
- **SwiftF0:** Aus https://raw.githubusercontent.com/lars76/swift-f0/main/README.md: „A small neural network with 14 386 parameters“, „runs at 180 times real time on a single laptop CPU core“, „between 46.875 Hz and 2093.75 Hz“ sowie „The only hard dependencies are numpy and onnxruntime“. Lizenz MIT laut API, Version v0.3.0 vom 25.09.2026. **[Einschätzung]** Für C++ ginge es nur über ONNX Runtime. Ob die Ergebnisse zwischen MSVC und clang gleich wären, ist offen.
- **MPM:** McLeod & Wyvill (2005), „A Smarter Way to Find Pitch“; Link im sevagh-README.
- **YIN:** de Cheveigné & Kawahara (2002), JASA; Link im sevagh-README.
- **pYIN:** Mauch & Dixon (2014), ICASSP; PDF verlinkt im sevagh-README unter `eecs.qmul.ac.uk/~simond/pub/2014/MauchDixon-PYIN-ICASSP2014.pdf`.

---

## 4. PSOLA / TD-PSOLA selbst bauen

### Literatur
- **PSOLA:** Moulines & Charpentier (1990), „Pitch-synchronous waveform processing techniques for text-to-speech synthesis using diphones“, *Speech Communication* 9(5–6), 453–467. Belegt über die Websuche: Research Portal IP Paris, Praat-Handbuch https://www.fon.hum.uva.nl/praat/manual/Moulines___Charpentier__1990_.html. Die Vorfassung steht in Eurospeech 1989 mit umgekehrter Autorenfolge (https://www.isca-archive.org/eurospeech_1989/charpentier89_eurospeech.html). Eine Einführung zum Unterricht: https://courses.grainger.illinois.edu/ece420/5_PSOLA.pdf (nicht gelesen).
- **Überblick über die Zeitdehnung:** Driedger & Müller (2016), „A Review of Time-Scale Modification of Music Signals“, *Applied Sciences* 6(2), 57. Vergleicht WSOLA mit PV-TSM. Belegt über die Websuche (MDPI, FAU CRIS); den Text selbst habe ich nicht gelesen.
- **Formant-Erhalt im Phase-Vocoder:** Röbel & Rodet (DAFx 2005), „Efficient spectral envelope estimation and its application to pitch shifting and envelope preservation“, mit „true envelope“ aus dem Cepstrum (DAFx-Archiv https://dafx.de/paper-archive/details/EDSTSaO17Gd_0LV03iwDfw; Inhalt aus der Zusammenfassung der Suche, **nicht am Text geprüft**). Dazu eine Echtzeit-Umsetzung: https://hal.archives-ouvertes.fr/hal-01161347 (nicht gelesen).
- **Nicht in dieser Sitzung geprüft**, nur aus dem Gedächtnis: Verhelst & Roelands (1993, ICASSP) zu WSOLA und Laroche & Dolson (1999, IEEE TSAP) zum phasengekoppelten Vocoder.
- **Vorbild-Code:** sannawag/TD-PSOLA, MIT, Python (`td_psola.py`), Push 25.11.2019, also ruhend.

### Aufwand und Verfahren [Einschätzung]
Die Bausteine:
1. Tonhöhe je Stelle mit Stimmhaft-Urteil. Dafür gibt es schon die eigene YIN-Messung; über die Zeit fehlt noch eine Glättung wie bei pYIN (Viterbi).
2. Pitch-Marken auf die Epochen setzen, das heißt auf die Schließmomente der Stimmlippen oder die Maxima je Periode. Das ist der heikle Teil.
3. Grains von zwei Perioden mit Hann-Fenster schneiden.
4. Die Grains im neuen Periodenabstand übereinanderlegen. Für die Dauer werden Grains wiederholt oder ausgelassen.
5. Stimmlose Abschnitte (Konsonanten, Atem) unverändert durchreichen oder mit WSOLA dehnen.

Die Formanten bleiben bei TD-PSOLA von der Bauart her erhalten, weil die Grains selbst nicht umgetastet werden.

Grenzen: nur einstimmig, empfindlich auf Fehler der Marken, schwach bei Hall und starkem Vibrato.

Aufwand: der Kern einige hundert Zeilen C++. Robuste Marken und die Übergänge stimmhaft/stimmlos kosten deutlich mehr. Grob geschätzt eine Stufe mit mehreren Teilschritten und Messtests an synthetischen Stimmen. Das ist nicht belegt.

Determinismus: ohne Zufall und ohne FFT leicht bitgleich zu halten.

---

## 5. Welche Wege in Frage kommen [Einschätzung]

**(1) Zeitdehnung ohne Tonhöhe**
- **Signalsmith Stretch** (MIT, header-only, mit festem Seed und festem Engine-Typ) oder **Bungee** (MPL-2.0, Phase-Vocoder, sehr frei steuerbar, braucht Eigen; Eigen liegt im Repo schon unter `externals/`).
- SoundTouch (LGPL, WSOLA) ist der klassische, einfache Weg, nach eigener Aussage aber nur „WSOLA-like“.
- Für Gesang gibt es bei Signalsmith einen Hinweis auf die Qualität: am besten zwischen 0,75x und 1,5x.

**(2) Tonhöhe ohne Längenänderung, mit Formanten**
- Freie Wege mit belegtem Formant-Erhalt sind **Signalsmith Stretch** (`setFormantFactor`, `setFormantBase`, `compensatePitch`) und **WORLD** (Analyse und Synthese mit getrennter Hüllkurve).
- Eigenes **TD-PSOLA** wäre für einstimmigen Gesang nach Literatur das schärfste Verfahren. Signalsmith sagt selbst, seine Korrektur sei „not a sharp as … PSOLA“.
- Rubber Band R3 wäre nach der Qualität stark, verlangt aber eine kommerzielle Lizenz.
- Bungee und SoundTouch erhalten die Formanten nicht.

**(3) Tonhöhenverlauf über die Zeit**
- Die vorhandene YIN-Messung um pYIN erweitern (Kandidaten je Rahmen, HMM/Viterbi). Als Vorlage für einen **eigenen Nachbau nach dem Paper** dienen librosa (ISC) und sevagh (MIT). Den c4dm-Code (GPL) nicht übernehmen.
- Alternativ **WORLD `Harvest`** (BSD-artig), passend, wenn WORLD auch für (2) gewählt wird.
- Neuronale Verfahren (SwiftF0 mit 14 386 Parametern, CREPE) brauchen ONNX oder TensorFlow. Das ist schwer mit dem Determinismus-Test zwischen MSVC und clang zu vereinen und wohl übertrieben.

---

## 6. Offene Punkte
- Bedingungen der kommerziellen Lizenz von Rubber Band: die Seite war gesperrt bzw. nicht erreichbar.
- Ob LumiViz als Programm (mit Binärdateien) verteilt wird oder nur die Quellen auf GitHub liegen. Davon hängt ab, wie stark die GPL- und LGPL-Pflichten greifen.
- Den vollständigen Wortlaut der WORLD-Lizenz (Zahl der Klauseln) nicht zu Ende gelesen.
- Lizenz der StaffPad-Dateien außerhalb von Audacity: nicht belegt.
- Ob Signalsmith, Bungee und Rubber Band bei gleicher Eingabe Bit für Bit gleich rechnen (FFT-Umsetzung, SIMD), ist nirgends belegt. Das wäre mit einem eigenen Test zu prüfen.
- Hörvergleich der Kandidaten an Gesang: nicht gemacht. Bungee verlinkt einen Vergleich unter https://bungee.parabolaresearch.com/compare-audio-stretch-tempo-pitch-change.html (nicht abgerufen, und als Anbieterseite nicht neutral).
- `pyin` in librosa und in Essentia: nicht am Quelltext geprüft.
- CREPE: Parameterzahl und Modellgröße nicht belegt.

## 7. Reichweite der Suche
- **Abgerufen:**
  - LICENSE/COPYING und README von SoundTouch (Codeberg), Rubber Band (mit `RubberBandStretcher.h`, COMPILING.md, CHANGELOG), Signalsmith Stretch (README, Header), Bungee (README, `.gitmodules`, alle `src/`- und `bungee/`-Dateien auf „formant“), WORLD (LICENSE, README, Header, Zufall in `matlabfunctions.cpp`), Sonic, c4dm/pyin, sevagh/pitch-detection, librosa, Essentia, aubio, CREPE, torchcrepe, SwiftF0, TD-PSOLA (nur die Dateiliste).
  - Audacity `au3-time-and-pitch` (Dateiliste, Köpfe, StaffPad-README).
  - GitHub- und Codeberg-API für Daten und Tags.
- **Websuche:** Literatur zu PSOLA, Driedger & Müller, Röbel & Rodet; Lizenz von Audacity StaffPad (ohne Ergebnis).
- **Nicht gesucht oder nicht gefunden:** Ardour (verwendet nach meinem Wissen Rubber Band, nicht belegt), sbsms (laut Websuche GPL-2.0-or-later über ein Arch-Paket, nicht an der Quelle geprüft), Praat-PSOLA (GPL, nicht geprüft), weitere MPM-Umsetzungen, eine eigenständige C++-Umsetzung von pYIN unter freier Lizenz (keine gefunden; nicht breit gesucht).
