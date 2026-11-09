# Recherche IC1: Messkette für andere Instrumente — Verfahren jenseits der Teiltonmessung

> **Auftrag:** Welche Verfahren (mit Literatur und offenen Implementierungen) messen, was die Teiltonmessung des
> Flügels nicht kann: gehaltene Töne, Einschwingen, Vibrato, Rauschanteile, nicht harmonische Reihen, Tonhöhengleiten;
> wie die Literatur Synthesemodelle gegen Aufnahmen prüft.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich] = Wortlaut/Feldinhalt, den das Werkzeug als Zitat zurückgab; [nur Zusammenfassung] = sonst
> (kein Beleg).
> **Reichweite:** im Bericht selbst (18 Suchen, 25 Abrufe, nur Englisch; HAL, UPF, AES, core.ac.uk gesperrt).
> **Von der Sitzung selbst nachgelesen:** siehe `../messkette.md` (je Angabe vermerkt); was dort nicht als
> „nachgelesen“ steht, gilt als „nur Bericht“.
> **Eingeflossen in:** `../messkette.md`.
> **Anmerkung der Sitzung:** Die zwei PDFs, die WebFetch selbsttätig zwischengespeichert hat (Pantazis/Stylianou
> 2008, Chaigne/Touzé/Thomas 2005), liegen im Cache der Claude-Sitzung, nicht in diesem Repo.

---

# Messkette für andere Instrumente: was eine Teiltonmessung für ausklingende Töne nicht kann

Recherchebericht, Abrufdatum aller Angaben: 2026-10-26.

## Reichweite

- **Umfang:** 18 Websuchen und 25 Abrufe (WebFetch), also 43 Werkzeugaufrufe. Das liegt über dem Rahmen von 25–40. Ein Teil davon waren Weiterleitungen und Fehlschläge.
- **Sprache der Suchbegriffe:** nur Englisch.
- **Suchbegriffe (Auswahl):**
  - „DESAM toolbox ESPRIT Badeau“, „Ege Boutillon High-resolution modal analysis“
  - „Serra Smith 1990 Spectral Modeling Synthesis“, „McAulay Quatieri 1986“
  - „YIN de Cheveigné“, „SWIPE Camacho“, „Mellody Wakefield violin vibrato“, „vibrato rate extent measurement … body resonance“
  - „Laroche Stylianou Moulines HNS“, „Stylianou 2001 harmonic plus noise“, „Verma Meng transient modeling“
  - „Chaigne Touzé Thomas gongs cymbals“, „timpani pitch glide“, „Legge Fletcher shallow gongs“
  - „McAdams Beauchamp Meneguzzi 1999“, „Horner Beauchamp relative spectral error“
  - „Guettler Askenfelt pre-Helmholtz“, „attack transient wind instrument“, „Boersma 1993 HNR“
- **Abgerufen wurden:** GitHub (Rohdateien von Lizenzen und README), SourceForge, arXiv/ar5iv, J-STAGE, MIT Lincoln Lab, Essentia-Doku, Quelltext von librosa.
- **Nicht erreichbar:**
  - HAL (Bot-Sperre „Access Denied“)
  - repositori.upf.edu (Sperre)
  - AES E-Library (403)
  - core.ac.uk (403)
  - air.unimi.it (403)
  - code.soundsoftware.ac.uk (Verbindung abgelehnt), damit auch das pYIN-Projekt dort
  - DAFx25-Server (Verbindung abgelehnt)
  - Telecom-Paris-Seite der DESAM-Toolbox (Weiterleitung ins Leere)
  - Columbia-Kopie von Stylianou 2001 (HTTP 421)
  - librosa-Doku-Seite zu pyin (404); ersatzweise den Quelltext gelesen
  - Erlangen-Kopie von Serra & Smith 1990 (404)
- **PDFs:** Zwei PDFs (Pantazis/Stylianou ICASSP 2008 und Chaigne/Touzé/Thomas 2005) konnte das Werkzeug nicht lesen, weil der Text komprimiert war. **Hinweis zur Regel „nichts speichern“:** WebFetch hat beide PDFs dabei *selbsttätig* in seinen Ergebniscache gelegt, unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\` (webfetch-…-89upjc.pdf, webfetch-…-nopk7f.pdf). Ich habe sie weder angefordert noch geöffnet. Löschen ist Sache von Patrik.
- **Nicht gesucht:** deutsch- und französischsprachige Literatur, Google Scholar, IEEE Xplore direkt, Matrix-Pencil- und Prony-Umsetzungen, Tony, Orgel- und Tonrad-Literatur, durchschlagende Zunge, Membranen nach Bessel, Marimba-Stäbe, Snare-Teppich. Zu diesen Punkten liefert der Bericht daher **keine Belege**.

**Kennzeichnung:** [wörtlich] heißt, das Werkzeug gab Wortlaut oder Feldinhalt als Zitat zurück. [nur Zusammenfassung] heißt, es handelt sich um eine Werkzeug- oder Suchmaschinen-Zusammenfassung; das ist **kein Beleg**.

---

## 1. Gehaltene Töne: Sinusmodell, SMS, Einschwingen

**McAulay & Quatieri 1986** (Sinus-Modellierung, sinusoidal modeling)
- Quelle: MIT Lincoln Laboratory, Publikationsseite: https://www.ll.mit.edu/r-d/publications/speech-analysissynthesis-based-sinusoidal-representation
- Ort laut Seite: IEEE Trans. Acoust. Speech Signal Process., Vol. ASSP-34, No. 4, August 1986, S. 744–754 [wörtlich, als Feldinhalt].
- DOI nicht ermittelt. Volltext auf dieser Seite nicht frei.
- Verfahren:
  - Parameter aus der Kurzzeit-FFT, „using a simple peak-picking algorithm“ [wörtlich]
  - Teiltonverfolgung über „‚birth‘ and ‚death‘ of the underlying sine waves“ [wörtlich]
  - Phase: „a cubic function is used to unwrap and interpolate the phase“ [wörtlich]
- **Unterschied zum Flügel:** Es gibt keine feste Frequenz je Teilton. Jeder Teilton trägt einen eigenen Frequenzverlauf und kann entstehen und enden.

**Serra & Smith 1990** (SMS, deterministisch + stochastisch)
- Computer Music Journal 14(4), S. 12–24, DOI 10.2307/3680788 [nur Zusammenfassung; JSTOR, nicht frei].
- Die freie Kopie an der Universität Erlangen war beim Abruf nicht erreichbar (404), der UPF-Eintrag gesperrt. Inhaltliche Angaben daher [nur Zusammenfassung]: Sinus mit stückweise linearen Amplituden- und Frequenzhüllkurven, dazu zeitveränderliches gefiltertes Rauschen.
- Serras Dissertation von 1989 habe ich **nicht abgerufen**.

**Verma & Meng 2000** (Transientenmodell, transient modeling synthesis, TMS)
- „Extending Spectral Modeling Synthesis with Transient Modeling Synthesis“, Computer Music Journal 24(2), S. 47–59, DOI 10.1162/014892600559317 [nur Zusammenfassung; nicht frei].
- Idee [nur Zusammenfassung]: Transienten werden mit einem Sinusmodell im Frequenzbereich beschrieben (Dualität zu Sinus im Zeitbereich). Das ist ein Kandidat für Anschlag und Bogen-Einsatz.

**Bogen-Einschwingen: Guettler & Askenfelt 1997**
- „Acceptance limits for the duration of pre-Helmholtz transients in bowed string attacks“, JASA 101(5), S. 2903–2913.
- PDF frei bei knutsacoustics.com/files/acceptance-limits-jas002903.pdf; die PDF selbst **nicht abgerufen**.
- Inhalt [nur Zusammenfassung]: Hörtest mit 20 Streichern ergab eine annehmbare Dauer des Vor-Helmholtz-Einschwingens (pre-Helmholtz transient) von etwa 50 ms bei verlängerten Perioden und etwa 90 ms bei mehrfachem Rückgleiten (multiple flyback). Gemessen an der leeren G-Saite der Violine.
- Für das Projekt heißt das: Messgröße ist die **Dauer bis zur regelmäßigen Helmholtzbewegung**, mit einer Wahrnehmungsschwelle als Toleranz.

**Einschwingen bei Blasinstrumenten** (alles [nur Zusammenfassung] aus einer einzigen Suche, keine Quelle selbst abgerufen)
- Castellengo 1999 (Acta Acustica), Flöten-Transienten: Geräusche und inharmonische Anteile kommen vor den Harmonischen. PDF: lam.jussieu.fr/Membres/Castellengo/publications/1999a1-Flute transients Analysis.pdf
- Gruppe an der UNSW (Almeida, Li, Smith, Wolfe), Klarinette: Teiltöne wachsen unterschiedlich schnell, Anstieg annähernd exponentiell.
- Eine einheitliche Definition der Anstiegszeit (rise time) wurde nicht gefunden.

## 2. Vibrato und Tonhöhenverlauf

**Tonhöhenverfolger**
- **YIN:** Der librosa-Quelltext (`librosa/core/pitch.py`, Docstring von `yin`) zitiert „De Cheveigné, Alain, and Hideki Kawahara.“ / „YIN, a fundamental frequency estimator for speech and music.“ / „The Journal of the Acoustical Society of America 111.4 (2002): 1917-1930.“ [wörtlich]. DOI 10.1121/1.1458024 [nur Zusammenfassung]. Eine freie PDF-Kopie liegt laut Suche bei cagnazzo.wp.imt.fr (nicht abgerufen).
- **pYIN:** Docstring in librosa: „Fundamental frequency (F0) estimation using probabilistic YIN (pYIN).“ [wörtlich]
  - Referenz: „pYIN: A fundamental frequency estimator using probabilistic threshold distributions.“, „2014 IEEE International Conference on Acoustics, Speech and Signal Processing (ICASSP). IEEE, 2014.“ (Mauch & Dixon) [wörtlich]
  - Rückgabe: f0, dazu „voiced_flag“ und „voiced_prob“ als „probability that a frame is voiced“ [wörtlich]
- **CREPE:** arXiv:1802.06182, „CREPE: A Convolutional Representation for Pitch Estimation“, Kim, Salamon, Li, Bello, Comments: „ICASSP 2018“ [wörtlich]. Frei.
  - Verfahren: „a data-driven pitch tracking algorithm“ [wörtlich], ein tiefes Faltungsnetz auf der Wellenform.
  - Ergebnis laut Abstract: „performing equally or better than pYIN“ [wörtlich].
  - Für ein eigenes C++ ohne Lernmodell ist CREPE nur als Vergleich brauchbar.
- **SWIPE:** Camacho & Harris 2008, „A sawtooth waveform inspired pitch estimator for speech and music“, JASA 124(3), S. 1638–1652, DOI 10.1121/1.2951592 [nur Zusammenfassung].
  - Verfahren: Abgleich mit dem Spektrum einer Sägezahnschwingung. Die Variante SWIPE′ nutzt nur den ersten und die Primzahl-Teiltöne, das senkt Fehler um Unteroktaven.

**Vibrato-Kenngrößen**
- **Essentia, Algorithmus `Vibrato`:** „detects the presence of vibrato and estimates its parameters given a pitch contour [Hz]“ [wörtlich].
  - Ausgaben „vibratoFrequency … [Hz]“ und „vibratoExtend … [cents]“ [wörtlich; so geschrieben, mit d].
  - Suchbereich standardmäßig 4–8 Hz.
  - Referenz: Salamon & Gómez 2012, IEEE TASLP 20(6).
  - Quelle: https://essentia.upf.edu/reference/std_Vibrato.html
- **Arroabarren, Rodet & Carlosena 2006** (IEEE TASLP, DOI 10.1109/TSA.2005.858015) [nur Zusammenfassung]:
  - Die Augenblicksfrequenz wird als Signal behandelt: der Mittelwert ist die Intonation, die Spitzenabweichung das Ausmaß (extent), die Schwankungsfrequenz die Rate.
  - Untersucht wird Gesang. Die Messung von Augenblicksamplitude und -frequenz je Teilton ist aber direkt übertragbar.
- **Violine:** Mellody & Wakefield 2000, „The time-frequency characteristics of violin vibrato: Modal distribution analysis and synthesis“, JASA 107(1), S. 598–611 [nur Zusammenfassung, aus Literaturlisten; Abstract nicht erreicht].
- **Vibrato und Körperresonanzen** (Fletcher, Blackham & Geertsen 1967, JASA; ein Schülerprojekt EUCYS 2021) [nur Zusammenfassung]:
  - Die Vibratorate bestimmt die Rate der Amplituden- und Frequenzmodulation der Teiltöne.
  - Das Ausmaß bestimmt, durch wie viele Resonanzen ein Teilton läuft.
  - Das ist der Mechanismus „Amplitudenvibrato aus Frequenzvibrato“. Eine Fachquelle dafür habe ich **nicht im Wortlaut** erreicht.

## 3. Rauschanteile

**Harmonisch plus Rauschen** (HNS, harmonic+noise)
- **Laroche, Stylianou & Moulines 1993**, „HNS: Speech modification based on a harmonic+noise model“, IEEE ICASSP 1993, Minneapolis, S. II-550–II-553 [nur Zusammenfassung; Eintrag im Forschungsportal IP Paris; kein freier Volltext gefunden].
- **Stylianou 2001**, „Applying the Harmonic Plus Noise Model in Concatenative Speech Synthesis“, IEEE TSAP [nur Zusammenfassung]. Die Columbia-Kopie war nicht erreichbar.
  - Kernidee laut Sekundärquellen: Harmonische unterhalb einer „maximum voiced frequency“, darüber **moduliertes Rauschen**.
  - Laut Bailly (Eurospeech 1999) ist das unrealistisch, weil Hauchgeräusch auch unterhalb dieser Grenze liegt.
- **Pantazis & Stylianou, ICASSP 2008:** Vergleich dreier Zeit-Hüllkurven für das Rauschen innerhalb der Periode (dreieckig, Hilbert, Energie). Im Hörtest wurden Energie- und Hilbert-Hüllkurve bevorzugt [nur Zusammenfassung; PDF unlesbar].
- Für das Projekt ist das der Fachstand zu „Rauschen moduliert mit der Periode“: Blasgeräusch und Bogengeräusch sind periodensynchron moduliert.

**Harmonics-to-Noise Ratio** (HNR)
- **Boersma 1993**, „Accurate short-term analysis of the fundamental frequency and the harmonics-to-noise ratio of a sampled sound“, Proceedings IFA Amsterdam 17, S. 97–110. Frei unter fon.hum.uva.nl [nur Zusammenfassung; nicht abgerufen].
- Verfahren: Autokorrelation im Lag-Bereich. Das ist das Praat-Verfahren.

**SMS-Residuum:** Siehe Abschnitt 1 und die sms-tools-Module unter 6.

**Nicht gefunden bzw. nicht gesucht:** Snare-Teppich, Becken als Rauschen, Rauschhüllkurven in ERB- oder Bark-Bändern. Dazu gibt es keinen Beleg.

## 4. Nicht harmonische Teiltonreihen, hochauflösende Verfahren

**Ege, Boutillon & David 2009**
- Quelle: arXiv:0909.0885, „High-resolution modal analysis“, Kerem Ege (LMS), Xavier Boutillon (LMS), Bertrand David (LTCI). Journal-Ref: „Journal of Sound and Vibration 325, 4-5 (2009) 852-869“, DOI 10.1016/j.jsv.2009.04.019 [wörtlich]. **Frei** über arXiv.
- Abstract:
  - Fourier-Verfahren versagen ab einer Modenüberlappung von 30 % [wörtlich sinngemäß aus dem Abstract].
  - „A technique based on a high-resolution analysis algorithm and an order-detection method“ [wörtlich]
  - „In each frequency-band, the number of modes is evaluated“ [wörtlich]
- Volltext (ar5iv, Abschnitt 1 bzw. 2.3.1):
  - „the high-resolution analysis algorithm ESPRIT“ [wörtlich]
  - „the order-detection method ESTER“ [wörtlich]
  - Signalmodell: „a sum of complex exponentials added to white noise“ [wörtlich]
- Vorverarbeitung (Abschnitt 2.2):
  - Bandzerlegung mit FIR-Filtern, „FIR filters are preferred because their transfer function has no pole“ [wörtlich]
  - Je Band werden die Signale „frequency-shifted toward zero and down-sampled“ [wörtlich]
  - Je Band weniger als etwa 25 komplexe Komponenten
  - Weißung des Rauschens nur bei breiten Bändern; „did not prove necessary in the applications presented here“ [wörtlich]
- Ergebnis je Mode: Frequenz, Dämpfung, komplexe Amplitude. Es braucht **keine vorgegebene Teiltonreihe**. Damit ist das der direkte Ersatz für die k·f0·√(1+Bk²)-Suche bei Stab, Glocke, Membran und Platte.

**Matrix Pencil, Prony:** nicht gesucht.

**Becken und Gong**
- **Chaigne, Touzé & Thomas 2005**, „Nonlinear vibrations and chaos in gongs and cymbals“, Acoustical Science and Technology 26(5), S. 403–409, DOI 10.1250/ast.26.403 [wörtlich, J-STAGE]. „JOURNAL FREE ACCESS“ [wörtlich].
  - Abstract: „Linear, weakly nonlinear and chaotic regimes are successively examined.“ [wörtlich]
  - Modell auf Grundlage der von-Kármán-Gleichungen [wörtlich]
  - Über Messverfahren sagt das Abstract **nichts**; die PDF war unlesbar.
- **Legge & Fletcher 1989**, „Nonlinearity, chaos, and the sound of shallow gongs“, JASA [nur Zusammenfassung]. PDF frei bei phys.unsw.edu.au/music/people/publications/Leggeetal1989.pdf (nicht abgerufen).
  - Energiefluss von tiefen zu hohen Moden.
  - Übergang zu subharmonischer oder chaotischer Bewegung, die das Spektrum zwischen den Moden auffüllt.
  - Hysterese über mehrere Sekunden.

## 5. Tonhöhengleiten

Alle Angaben dieses Abschnitts sind [nur Zusammenfassung]. Kein Messverfahren lag im Wortlaut vor.

- **Gong:** Laut einer ANU-Quelle und Rossing & Fletcher 1983 gleitet ein flacher Gong nach starkem Schlag mit abklingender Amplitude **nach unten** (versteifende Nichtlinearität). Ein gewölbter Gong gleitet **nach oben**.
- **Membran:** Avanzini, Marogna & Bank 2012 (JASA) modellieren das Gleiten über amplitudenabhängige Spannung. Der kurzzeitig gemittelte Spannungszuwachs ist etwa proportional zur Energie. Das ist Synthese, keine Messung. Die Seite war gesperrt (403).
- **Pauke:** Ein Messverfahren für das Gleiten nach dem Schlag habe ich **nicht gefunden**.

## 6. Offene Implementierungen

Nur als Vorbild. Lizenzen sind [wörtlich] mit Fundort, Abruf 2026-10-26.

| Fund | Misst / kann | Sprache | Lizenz [wörtlich] | Ort |
|---|---|---|---|---|
| **sms-tools** (MTG) | Sinus-, Harmonik-, HPR-, HPS-, Stochastik-, SPR-, SPS-Modell (Module sineModel.py … spsModel.py laut README) | Python, dazu „a compiled Cython extension for selected core routines“ | „GNU AFFERO GENERAL PUBLIC LICENSE Version 3, 19 November 2007“ | github.com/MTG/sms-tools, Datei LICENSE |
| **librosa** | yin, pyin (f0, Stimmhaftigkeit, Wahrscheinlichkeit) | Python | „ISC License“; „Copyright (c) 2013--2026, librosa development team.“ | github.com/librosa/librosa, Datei LICENSE.md |
| **Essentia** | u. a. SineModelAnal, HarmonicModelAnal, HprModelAnal, HpsModelAnal, SprModelAnal, StochasticModelAnal, PitchYin, PitchYinFFT, PitchYinProbabilistic, PitchMelodia, Vibrato, Inharmonicity, OddToEvenHarmonicEnergyRatio, Tristimulus, LogAttackTime, Envelope (Namen aus der Algorithmenliste; meine Abfrage nannte die Namen vor, die Liste bestätigte sie; einen HNR-Algorithmus fand die Abfrage nicht) | C++ (mit Python-Anbindung; Sprache nicht wörtlich geprüft) | „GNU AFFERO GENERAL PUBLIC LICENSE Version 3, 19 November 2007“ | github.com/MTG/essentia, Datei COPYING.txt |
| **aubio** | Tonhöhe, Einsätze (nicht im Einzelnen geprüft) | C | „GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007“ | github.com/aubio/aubio, Datei COPYING |
| **Loris** | „a library for sound analysis, synthesis, and morphing“ | C++ (nicht wörtlich geprüft) | „GNU General Public License version 2.0 (GPLv2)“ | sourceforge.net/projects/loris, Feld „License“; **nicht** in einer Lizenzdatei im Quelltext geprüft |
| **SPEAR** | „an application for audio analysis, editing and synthesis“, „based on the traditional McAulay-Quatieri technique“ | wxWidgets-Anwendung | **keine Lizenz auf der Seite angegeben**; sie ist nicht als offene Quelle erkennbar | klingbeil.com/spear |
| **pYIN** (c4dm, Vamp-Plugin) | pYIN | C++ (nicht wörtlich geprüft) | „GNU GENERAL PUBLIC LICENSE Version 2, June 1991“ | github.com/c4dm/pyin, Datei COPYING |
| **CREPE** | f0 über ein neuronales Netz | Python | „The MIT License (MIT)“; „Copyright (c) 2018 Jong Wook Kim“ | github.com/marl/crepe, Datei LICENSE |
| **DESAM Toolbox** | spektrale Modelle für Musiksignale; ob ESPRIT enthalten ist, ist **nicht bestätigt** | MATLAB | **nicht wörtlich erhalten**; laut Suchzusammenfassung GPL; die Projektseite leitete ins Leere | DAFx-10, HAL hal-00523319 (HAL gesperrt) |

**Nicht geprüft:** Tony (Lizenz), weitere ESPRIT-Umsetzungen, Matrix Pencil.

## 7. Validierung eines Synthesemodells gegen Aufnahmen

**McAdams, Beauchamp & Meneguzzi 1999**, „Discrimination of musical instrument sounds resynthesized with simplified spectrotemporal parameters“, JASA 105(2), S. 882–897 [nur Zusammenfassung]. Frei bei mcgill.ca/mpcl/files/mpcl/mcadams_1999_jasa.pdf (nicht abgerufen).
- Sieben Instrumente: Klarinette, Flöte, Oboe, Trompete, Violine, Cembalo, Marimba.
- Getestet wurde, ob Hörer vereinfachte Resynthesen von der vollen Resynthese unterscheiden.
- Gut unterscheidbar waren das Glätten der spektralen Hüllkurve (über 95 % außer bei der Trompete) und die Kohärenz der Amplitudenhüllkurven.
- Mittel bis schlecht unterscheidbar waren das Glätten oder Abflachen der Frequenzschwankungen und erzwungen harmonische Frequenzen.
- Für das Projekt ist das eine Rangfolge, **welche Messgröße eng toleriert werden muss**.

**Horner, Beauchamp & So 2006**, JAES 54(3), S. 140–156 [nur Zusammenfassung; AES gesperrt, nicht frei]:
- Ein **relativer spektraler Fehler** (relative spectral error) sagt die Unterscheidbarkeit am besten vorher, mit bis zu 91 % Übereinstimmung.
- Eine Folgearbeit von 2011 fand für Mel-Band- und MFCC-Metriken fast gleich gute Werte.

**Guettler & Askenfelt 1997** (siehe 1): Wahrnehmungsschwelle für die Dauer des Einschwingens.

---

## Nicht gefunden

- Ein Messverfahren für das Tonhöhengleiten der Pauke direkt nach dem Schlag.
- Eine freie, offen lizenzierte ESPRIT- oder ESTER-Umsetzung mit wörtlich geprüfter Lizenz.
- Wortlaut aus Serra 1989/1990 und Stylianou 2001 (Quellen gesperrt oder unlesbar).
- Eine einheitliche Definition der Anstiegszeit je Teilton oder der Einsatzreihenfolge.
- Eine Validierung mit gehaltenen Tönen *physikalischer* Modelle: Die Funde betreffen Resynthese und additive Synthese.
- Nicht gesucht und damit nicht belegt: Orgel, Tonrad, durchschlagende Zunge, Snare, Bark- oder ERB-Rauschhüllkurven, Matrix Pencil, Prony, Bessel-Membran, Marimba-Verhältnisse.

## Eigene Einschätzung (kein Beleg)

**Grundsätzlich**
- Die Flügelkette lässt sich in zwei Teile trennen:
  - einen **Analysekern ohne Vorgabe einer Reihe**: Teiltonverfolgung mit Frequenzverlauf nach McAulay/Quatieri bzw. SMS, oder ESPRIT je Band für Ausklingendes;
  - eine **instrumentspezifische Deutung**: Reihe, Abklinggesetz, Vibrato.
- Feste Frequenz je Teilton und ein einziges langes Spektrum ab 50 ms gehen bei allem nicht, was gleitet, vibriert oder gehalten wird.
- Schroeder-Integration und σ(ω) taugen nur für den Ausschwingteil nach dem Loslassen.

**Neu gebrauchte Messgrößen je Klasse**
- **Bogen-Saite:** f0-Verlauf (pYIN-artig); Vibrato-Rate, -Ausmaß und -Form; AM je Teilton als Folge der Körperresonanzen (Phasenlage zur FM); Dauer bis zum Helmholtz-Regime; Bogengeräusch als Residuum mit periodensynchroner Hüllkurve; HNR; Ausschwingzeit nach Bogenende; stationäres Spektrum mit Streuung.
- **Rohr mit Lippe, Rohrblatt oder Luftstrahl:** Anstiegszeit und Einsatzreihenfolge je Teilton; Vorläufergeräusch bei Luftstrahl; Anblasgeräusch (Spektralform, Pegel relativ zum Periodischen, Modulation mit der Periode); Verhältnis ungerader zu gerader Teiltöne (Klarinette); Vibrato bzw. Tremolo, getrennt nach AM und FM; Pegelabhängigkeit des Spektrums (Helligkeit über der Dynamik).
- **Platte und Stab:** Modalfit nach ESPRIT/ESTER je Band (Frequenz, Dämpfung, Amplitude je Mode, ohne Reihe); Frequenzverhältnisse der Moden; Dämpfung über der Frequenz frei, nicht als b1 + b3·ω².
- **Membran:** wie Platte, dazu ein f(t) je Mode in den ersten 100–300 ms (Gleiten, gemessen über die Augenblicksfrequenz des stärksten Modes); Fellgeräusch bzw. Teppich als Residuum im Band.
- **Becken und Gong:** Spektralschwerpunkt über der Zeit; Energie je Band über der Zeit (Anstieg *nach* dem Schlag in hohen Bändern als Maß des Energieflusses); Abklingen je Band; Tonhöhengleiten des Grundmodes; kein Teiltonmodell oberhalb der dichten Grenze.
- **Tonrad-Orgel:** stationäres Spektrum je Zugriegel-Mischung; Klick beim Einsatz und Loslassen (Transient); Leckübersprechen; Phasenlage; Lautsprecher-Vibrato als FM und AM (die Rotationsfrequenz wäre zu messen).
- **Durchschlagende Zunge:** Anstieg, stationäres Spektrum (reich an Teiltönen), Schwebung zwischen Chören (Tremolo-Rate), Druckabhängigkeit der Tonhöhe, Luftgeräusch.

**Lizenzlage als Vorbild:** librosa (ISC) und CREPE (MIT) sind am freiesten. sms-tools und Essentia stehen unter AGPL-3.0, aubio unter GPL-3, Loris und pYIN unter GPL-2. Das ist unerheblich, solange nur gelesen und eigenes C++ geschrieben wird. Das Projekt sollte aber vermerken, dass kein Code übernommen wird.

**Empfohlene Kerne zum Nachbauen:** Ege/Boutillon/David (frei auf arXiv, vollständig beschrieben) und pYIN.

**Zu klären:** Die zwei automatisch zwischengespeicherten PDFs im Ordner tool-results hat Patrik zu entscheiden; siehe Reichweite.
