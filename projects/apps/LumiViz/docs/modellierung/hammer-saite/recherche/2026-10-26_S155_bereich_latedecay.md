> **Typ:** Agentenbericht, unverändert (Recherche-Agent, S155, 2026-10-26) — nur dieser Kopf ist dazugesetzt.
> **Anlass:** SQ1 a (`docs/audio/Spielweisen_Konzept.md` §4.1, Teilplan 7.5): Grenzen beim Setzen aus Quellen zum
> Flügel statt „Faktor 3 um *Salamander*“. Größe: `lateDecay` (T60 der zweiten Stufe, Verhaeltnis spaet/frueh). Suche: im Netz.
> **Prüfstand:** Nichts davon ist von Claude im Volltext nachgelesen. „Volltext gesehen“ heißt: der Agent hat die
> abgerufene Datei gelesen. Zitierfähig ist erst, was aus der Quelle selbst unter `quellen/` nachgelesen ist.
> **Eingeflossen:** in die Zusammenstellung der Bereiche je Größe (S155, Konzept §4.1 nach SQ1 a).

# Recherche `lateDecay` — Nachklang (aftersound) des Grundtons

Stand 2026-10-26 · Recherche-Agent · alle Zitate wörtlich aus dem abgerufenen Text, sonst ausdrücklich markiert.

## Auftrag

Belegte Wertebereiche je Taste (MIDI 21–108) für `lateDecay` = T60 des Grundtons in der zweiten, langsamen
Abklingstufe (Gegenlagen im Chor bzw. horizontale Polarisation), dazu Verhältnis spät/früh, Knick (Zeit/Pegel),
Abhängigkeiten (Verstimmung, Stoß, Steg-Admittanz). Ziel: Grenzen für den Optimierer statt „Faktor 3 um eine Aufnahme“.

## Ergebnis in einem Satz

**Eine über die Tastatur gemessene Reihe für den Nachklang (T60 spät je Taste) habe ich in frei zugänglichen Volltexten
nicht gefunden.** Belegt ist genau ein Zahlenpaar (Weinreich, eine Taste, eine Saite) und eine Reihe qualitativer
bzw. struktureller Grenzen (Verstimmung, Admittanz, obere Schranke durch die Eigendämpfung der Saite).
Die klassischen Messarbeiten (Martin 1947, Hundley/Benioff/Martin 1978, Weinreich 1977, Nakamura 1988) liegen hinter
der Bezahlschranke von JASA; gesehen habe ich nur Abstracts ohne Zahlen.

## Reichweite der Suche

- **Abrufe:** rund 36 (WebFetch + WebSearch + 2 curl).
- **Suchbegriffe (WebSearch):** `piano "aftersound" decay rate dB/s measured across keyboard two-stage decay`;
  `Aramaki Bensa … coupled piano strings double decay resynthesis pdf`; `site:speech.kth.se 5_lectures weinreic`
  (Suchmaschine indiziert KTH nicht); `"Factors contributing to the multiple rate of piano tone decay" …`;
  `piano two-stage decay "initial decay" "final decay" … Martin 1947`; `Weinreich "Coupled piano strings" 1977 pdf …`;
  `piano tone "double decay" measured decay times first and second slope …` (extended);
  `"Study on double decay of individual partials of piano sound" …`; `piano partial decay "two decay rates" … Lehtonen OR Välimäki OR Bank OR Bensa` (extended).
- **Gelesen (Volltext):** KTH „Five lectures“ — Weinreich (`weinreic.html`, `motion.html`, `strings.html`, `mistuned.html`),
  Wogram (`decay.html`), Inhaltsseite, Conklin-Übersicht; Woodhouse 2022 (AAM, Cambridge Apollo); Cheng/Dixon/Mauch 2015;
  Aramaki u. a. 1999 (DAFx); Lehtonen 2005 (MSc TKK); Miranda Valiente u. a. 2024 (lag schon lokal); Kevin Hunt (Webseiten).
- **Nur Abstract/Metadaten:** Weinreich 1977 (JASA 62, 1474), Hundley/Benioff/Martin 1978 (JASA 64, 1303), Martin 1947
  (JASA 19, Suppl. 722 — Tagungs-Abstract), Nakamura 1988 (nur Suchmaschinen-Zusammenfassung, **nicht** gesehen).
- **Gesperrt / nicht erreichbar:** HAL (`hal.science`, `hal-hcl.archives-ouvertes.fr`: Anubis-Botschutz) — damit fehlen
  „Comparison of the effect of hammer striking irregularities and mistuning on the double decay of piano tones“ (IRCAM) und
  Bensas Dissertation; eprints.soton.ac.uk (403: Karatsovis PhD, Miranda Internoise 2022); ioa.org.uk (404: Baxandall/Brown/Gough,
  Wogram); Springer EURASIP 2004 (Giordano/Jiang? — Titel **nicht** gesehen, Login-Umleitung); oa.mg (503); KTH-Verzeichnis `weinreic/` (403, Einzelseiten gingen).
- **Nicht gesucht** (lokal vorhanden, anderer Agent): Bank 2000/2006/2010, Chabassier, Rigaud, Tan, Cuenca & Caussé, Plath, Boutillon/Ege.
- Kein Treffer ist kein Beweis: Fletcher & Rossing Kap. 12, Askenfelt-, Hall-Vorlesung, Giordano-Buch habe ich nicht im Volltext gesehen.

## Quellen

### Q1 — Weinreich, „The coupled motion of piano strings“, in *Five lectures on the Acoustics of the piano*

- Copyright-Zeile auf der Inhaltsseite: „© 1990 Royal Swedish Academy of Music“ (Zeichen im Abruf verstümmelt). Frei lesbar (HTML), keine Lizenzangabe.
- URL: https://www.speech.kth.se/music/5_lectures/weinreic/motion.html (Volltext gesehen)
- Zitate:
  - „The initial portion, called "prompt sound," drops (in this case) at a rate of about 8 dB/sec;“
  - „the final one, called "aftersound," at less than one-quarter that rate.“
  - Bildunterschrift Fig. 1: „Typical decay of a piano tone as illustrated by the sound pressure level versus time (Eb3 = 311 Hz).“
  - Im Text zu Fig. 1: „…only one of the three strings belonging to this note (Eb4) was allowed to sound.“ (Teilzitat)
  - „the vertical polarization is the primary one excited by the hammer“ · „the decay of the vertical mode is also much more rapid“ ·
    „the relatively slight amount of horizontal vibration becomes, after a while, dominant.“
  - Fig. 2: „The two components are of equal strength during an interval around the intersection of the sloping lines.“ /
    „In this region the resulting sound level varies strongly with microphone position.“
  - Laut Abruf: Anschlag etwa bei t = 2 s, Dämpfer bei etwa t = 17 s (Zusammenfassung des Werkzeugs, **nicht wörtlich gesehen**).
- **Prüfung der S153-Angabe:** bestätigt („8 dB/sec“, „less than one-quarter that rate“). **Erweiterung, wichtig fürs Modell:**
  (1) Das Beispiel ist **eine einzelne Saite** (zwei andere gedämpft) — der Knick dort entsteht durch die **Polarisation**, nicht durch den Chor.
  (2) Notenname widersprüchlich: Unterschrift „Eb3 = 311 Hz“, Text „Eb4“; 311 Hz ist Es4 (MIDI 63) — der Text hat recht (eigene Rechnung).
  (3) Ein Flügel; Messgröße Schalldruckpegel (Mikrofon), nicht Saitenbewegung; Knickpegel hängt laut Fig. 2 vom Mikrofonort ab.
- Abgeleitet (eigene Rechnung, keine Quellenzahl): früh 8 dB/s → T60 ≈ 7,5 s; spät < 2 dB/s → **T60_spät > 30 s**; Verhältnis
  T60_spät/T60_früh **> 4**.

URL: https://www.speech.kth.se/music/5_lectures/weinreic/strings.html (Volltext gesehen)
- „Let us imagine two identical (and identically tuned) strings attached to the bridge at the same place.“
- „the decay rate will be doubled by this coupling“ (gleichphasige Mode zweier gleich gestimmter Saiten)
- „we see the possibility for the overall vibration to decompose into a prompt sound and an aftersound.“
- Versuch mit Filzkeil: „the sound level simultaneously jumps up by close to 20 dB.“ — Stilllegen einer Saite im Nachklang hebt den
  Pegel um rund 20 dB (der Nachklang ist dort die gegenphasige Mode, die fast nicht abstrahlt).

URL: https://www.speech.kth.se/music/5_lectures/weinreic/mistuned.html (Volltext gesehen)
- „there are no beats unless the "mistuning" is more than about 0.3 Hz“ (Beispiel mittlere Lage)
- „Above about 0.3 Hz beats do appear, as exemplified by the curve drawn for a "mistuning" of 0.64 Hz“
- Fig. 8: „For smaller values the strings lock to a common frequency,“ / „and the effect of the "mistuning" is to control the level of the aftersound“ /
  „(cf. the curves for f = 0.22 Hz and 0.06 Hz).“
- „followed by a beatless aftersound whose level depends on the "mistuning."“ · „adjust the aftersound of each note to a more uniform level“
- Fig. 7: „A resistive support (bottom) does not influence the vibration frequency but damps the motion.“
- Keine Zahl zur **Rate** des Nachklangs auf dieser Seite; die Verstimmung steuert laut Text den **Pegel** des Nachklangs.

### Q2 — Weinreich, „Coupled piano strings“, JASA 62 (6), 1474–1484 (1977), DOI 10.1121/1.381677

- Zugang: Bezahlschranke; **nur Abstract gesehen** (pubs.aip.org).
- Zitate: „depends on“ „bridge admittance, hammr irregularities, and the exact state in which the piano is tuned.“ (Tippfehler im Original-Abruf) ·
  „the great importance of the two polarizations of the string motion.“ · „The function of the una corda pedal in controlling the aftersound is explained“ ·
  „fine tuning of the unisons to make the aftersound“ „more uniform is discussed.“
- Keine Zahlen im Abstract.

### Q3 — Hundley, Benioff, Martin, „Factors contributing to the multiple rate of piano tone decay“, JASA 64, 1303–1309 (1978)

- Zugang: Bezahlschranke; **nur Abstract** (pubs.aip.org/asa/jasa/article/64/5/1303/773024). Baldwin Piano and Organ Company.
- Zitat: die zwei verwandten Faktoren (Interferenz fast gleich gestimmter Saiten; Wechsel gleich-/gegenphasig am Steg) „control the rate change in most cases“.
- Keine Zahlen im Abstract.

### Q4 — Martin, „Decay Rates of Piano Tones“, JASA 19 (4_Supplement), 722 (1947), DOI 10.1121/1.1916547

- Zugang: Tagungs-Abstract frei, Volltext (JASA 19, 535–541) Bezahlschranke. **Nur Abstract.**
- Zitate: „Decay curves were recorded for an upright piano, a baby grand, and an electrical spinet.“ ·
  „Analysis of the data reveals the degree of control exerted by the sounding‐board upon decay rate,“ „particularly in the initial stage of decay.“
- Kein Konzertflügel, keine Zahlen im Abstract.

### Q5 — Jim Woodhouse, „A necessary condition for double-decay envelopes in stringed instruments“, JASA (2022)

- Zugang: Autorenfassung (AAM) im Cambridge-Repositorium Apollo, frei; im PDF „c 2022 Acoustical Society of America“. Volltext gesehen.
- URL: https://www.repository.cam.ac.uk/bitstreams/ab932038-6866-48af-b7ab-0d31eef9c394/download (→ api.repository.cam.ac.uk …/content)
- **Lokal abgelegt:** `quellen/Woodhouse_2022_NecessaryConditionDoubleDecay_JASA_AAM.pdf` (1 115 278 Byte).
- Zitate:
  - „The antisymmetric mode has the two strings moving in opposite directions, so that the net force exerted on the bridge is zero and no energy is lost to the soundboard.“
  - „sooner or later its amplitude will fall below that of the slower-decaying mode, which will then take over and dominate the "after-sound".“
  - „In either case both modes will have the same in- trinsic damping, so for a strong contrast in total damping it is necessary that one of the modes can lose significantly more energy through the bridge than the other“
  - „The intrinsic damping will then be domi- nated by air damping“ (für die Grundmode; Luftdämpfung nach Fletcher & Rossing, Gl. 2–3 im Text)
  - „The piano was a Broadwood baby grand of early 20th-century date.“
  - „With the triple string, b rises above air by some 20 dB over the entire frequency range above about 160 Hz, so that a marked double decay can be expected for all notes in that range.“ (Fig. 2b, Basis Note A4)
  - „Lower notes use wrapped strings, single or paired, with different string properties: they are not covered by the plot shown here.“
  - „In the absence of experimental evidence from psychoacoustical studies … it is not at present possible to put a numerical value on the requirement“
- Bedeutung für `lateDecay`: **obere Schranke** des Nachklangs = Eigendämpfung der Saite (Luft + Viskoelastizität) plus Rest-Steg-Verlust
  der Gegenlage; ein Optimierer sollte T60_spät nicht über die T60 der Eigendämpfung hinaus lassen. Zahlen dazu stehen nicht in Q5
  (nur Formeln); η_air ließe sich aus d, f, ρ der Saite rechnen — **nicht gerechnet, nicht belegt**.
- Bedeutung fürs Register: Doppelabklingen nach Q5 für den Broadwood ab ≈ 160 Hz (E3/Es3) mit Dreierchor klar zu erwarten; Bass nicht abgedeckt.

### Q6 — Tian Cheng, Simon Dixon, Matthias Mauch, „Modelling the decay of piano sounds“ (ICASSP 2015)

- Zugang: Autorenseite QMUL, frei. Volltext gesehen. URL: https://webspace.eecs.qmul.ac.uk/s.e.dixon/pub/2015/ChengDixonMauch-ICASSP2015-Decay.pdf
- **Lokal abgelegt:** `quellen/Cheng_Dixon_Mauch_2015_ModellingDecayPianoSounds_ICASSP.pdf` (423 748 Byte).
- Zitate: „Usually, only the lowest ten or so notes have one string per note. The subsequent (about 18) notes have two strings, and the rest have three strings.“ ·
  „For notes with multiple strings, the decay rate will double or treble if the strings are tuned to exactly the same frequency.“ ·
  „For double decay, the slope of the line which covers the larger part of the time duration is recorded as the decay rate.“ ·
  „The results also indicate that dynamics have no significant effect on the decay rate.“ · „We use the piano sounds from the RWC Musical Instrument Sound Database“.
- Abb. 8/9: Abklingraten (dB/s) über MIDI 21–108 aus RWC, **aber** je Teilton nur **eine** Rate (die längere Strecke) — keine Trennung früh/spät;
  Zahlen nur aus Abbildung ablesbar, nicht abgelesen (Text der Achsen im PDF, Kurven als Bild). Instrument im Text nicht als Konzertflügel ausgewiesen.
- Bedeutung: stützt „Rate unabhängig von der Dynamik“ (für die dominante Strecke) — d. h. Stoß ändert eher den Knick-Pegel als die späte Rate (Deutung, nicht belegt).

### Q7 — M. Aramaki, J. Bensa, L. Daudet, Ph. Guillemain, R. Kronland-Martinet, „Resynthesis of coupled piano strings vibrations based on physical modeling“ (DAFx-99)

- Zugang: DAFx-Archiv, frei. Volltext gesehen. URL: https://dafx.de/paper-archive/1999/aramaki.pdf
- **Lokal abgelegt:** `quellen/Aramaki_Bensa_Daudet_Guillemain_KronlandMartinet_1999_CoupledPianoStrings_DAFx99.pdf` (59 586 Byte).
- Zitate: „The frequency gap is typically in the range [0.1Hz ; 5Hz].“ (Doublet-Abstand) · „shall consider only the vertical polarisations“ ·
  Messaufbau „constituted of a massive support on which the two strings are tightened“ (kein Flügel).
- Keine Abklingzeiten als Zahl.

### Q8 — Klaus Wogram, „The strings and the soundboard“ (Five lectures, KTH)

- Zugang frei (HTML), © 1990 Royal Swedish Academy of Music (Inhaltsseite). Volltext gesehen. URL: https://www.speech.kth.se/music/5_lectures/wogram/decay.html
- Zitate: „from the maximum level to -20 dB (T20)“ · „From the graph, we obtain the corresponding T20-values as 3.5 and 0.7 s, respectively.“ (F#4 / G4) ·
  „The decay times for the notes on a small upright piano are plotted in Fig. 11.“
- **Klavier (klein), nicht Flügel; T20 ab Maximum = frühe Stufe.** Für `lateDecay` nur als Hinweis auf große Nachbarschwankung (5:1) brauchbar.

### Q9 — Heidi-Maria Lehtonen, „Analysis and Parametric Synthesis of the Piano Sound“, MSc TKK 2005

- Zugang: TKK-Bibliothek, frei. Volltext gesehen. URL: http://lib.tkk.fi/Dipl/2005/urn007876.pdf
- **Lokal abgelegt:** `quellen/Lehtonen_2005_AnalysisParametricSynthesisPianoSound_MSc_TKK.pdf` (3 652 397 Byte).
- Zitate (S. 7–8 gedruckt): „The decay rate of the piano tone is twofold: The tone begins to decay rapidly, but after“ „some milliseconds the decay rate changes and becomes slower.“ ·
  „the amount of aftersound, that is, the portion of the "slow" decay, depends on the“ „degree of mistuning.“ ·
  „the hammer irregularities may have an effect on the two-stage decay as one string of the string group can be hit harder than another.“
- Keine Zahlen zur zweiten Stufe. „some milliseconds“ widerspricht Weinreich Fig. 1 (Knick nach Sekunden) — Sekundärdarstellung, nicht als Beleg für die Knickzeit nehmen.

### Q10 — Miranda Valiente u. a., „Modelling the una-corda effect in pianos“, J. Phys.: Conf. Ser. 2909 012036 (2024)

- CC BY 4.0, lag bereits lokal (`MirandaValiente_et_al_2024_UnaCordaEffect_JPhysConfSer.pdf`). Volltext (Text) gesehen: keine Abklingzahlen; qualitativ
  „In the una corda case, … there is no beating, the decay is more even“.

### Q11 — Kevin Hunt, Webseiten kevinhunt.com.au (?p=1396, ?p=1556, ?p=1424)

- Zugang frei, nicht begutachtet; Instrumente „Stuart“ und „Steinway“ ohne Modellangabe im gesehenen Text.
- Zitate (?p=1556, Seite „3. C3v20 MWmic5“): „The onset oscillation of the Stuart sound (i) , decays at 8.6dB/s, almost twice the speed of Steinway.“ ·
  „The Stuart oscillation transfers into the first stage of the after-sound oscillation (ii) after .5s,“ „approximately 2s before the Steinway's transition.“
- Daraus (eigene Deutung): Steinway C3, frühe Rate ≈ 4–5 dB/s, Knick ≈ 2,5 s. **Keine** Zahl für die späte Rate. Prüfstand niedrig.

## Bereich je Register

| Register / MIDI | min T60_spät | max T60_spät | typisch | Verhältnis T60 spät/früh | Knick | Quelle | Zitat-Kurzform | Prüfstand |
|---|---|---|---|---|---|---|---|---|
| Mitte, Es4 (MIDI 63), **eine** Saite (Polarisation) | > 30 s (aus „< ¼ von 8 dB/s“, eigene Rechnung) | — (nicht belegt) | — | **> 4** | Sekunden (Fig. 1, nicht abgelesen); Pegel vom Mikrofonort abhängig | Q1 motion.html | „about 8 dB/sec“ / „less than one-quarter that rate“ / „only one of the three strings“ | Volltext gesehen; 1 Flügel, 1 Taste, Mikrofonpegel |
| Mitte/Diskant mit Chor (ab ≈ 160 Hz, ≈ MIDI 51+) | — | obere Schranke = Eigendämpfung der Saite (Luft) — keine Zahl | — | „marked double decay“ erwartet | — | Q5 Woodhouse | „b rises above air by some 20 dB … above about 160 Hz“ | Volltext (AAM); Broadwood-Stutzflügel, Rechnung aus gemessener Admittanz |
| Zwei gleich gestimmte Saiten (Theorie) | — | — | — | frühe Rate = 2 × (bzw. 3 ×) Einzelsaite | — | Q1 strings.html, Q6 | „the decay rate will be doubled by this coupling“ / „double or treble“ | Volltext gesehen |
| Chor, Verstimmung (mittlere Lage) | — | — | — | Pegel des Nachklangs durch Verstimmung gesteuert; Rasten < 0,3 Hz, Schwebung > 0,3 Hz | — | Q1 mistuned.html; Q7 | „control the level of the aftersound“ / „0.3 Hz“ / „[0.1Hz ; 5Hz]“ | Volltext gesehen; Weinreich = Rechnung (Fig. 8) |
| Tief, C3 (MIDI 48) | — | — | — | — (nur frühe Rate: Stuart 8,6 dB/s, Steinway ≈ halb) | Stuart 0,5 s, Steinway ≈ 2,5 s | Q11 Hunt | „decays at 8.6dB/s, almost twice the speed of Steinway“ | Webseite, nicht begutachtet |
| Bass (MIDI 21–≈ 50, umsponnen, 1–2 Saiten) | — | — | — | — | — | Q5 | „not covered by the plot shown here“ | Lücke |
| Diskant oben (MIDI ≈ 85–108) | — | — | — | — | — | — | — | Lücke |
| Ganze Tastatur (Rate einer Strecke, nicht getrennt) | — | — | — | — | — | Q6 Cheng (RWC) | „slope of the line which covers the larger part“ | Volltext; Werte nur in Abb. 8, nicht abgelesen |

Für einen Optimierer ist aus dieser Recherche belastbar:
1. **Untere Schranke Verhältnis** spät/früh: > 4 in der Mitte für die Polarisation (Q1); für den Chor keine Zahl.
2. **Obere Schranke T60_spät**: die T60 der reinen Eigendämpfung der Saite (gegenphasige Mode gibt dem Steg keine Kraft, Q5) — als Rechengröße aus Saitendaten (η_air nach Fletcher & Rossing), nicht aus einer Messung.
3. Die Verstimmung steuert den **Pegel** (Knick), nicht in erster Linie die späte Rate (Q1 mistuned.html); die Dynamik ändert die (dominante) Rate nicht signifikant (Q6).

## Offene Punkte

- Volltexte JASA (Weinreich 1977, Hundley u. a. 1978, Martin 1947, Nakamura 1988): Bezahlschranke — Patrik müsste sie beschaffen; Weinreich 1977 hat nach Abstract „experimental data … on beats and aftersound“.
- HAL (Botschutz): IRCAM-Arbeit zu Hammer-Unregelmäßigkeit vs. Verstimmung (behandelt laut Suchmaschine das Verhältnis Nachklang/Sofortklang — **nicht gesehen**), Bensa-Dissertation. Evtl. im Browser abrufbar.
- Southampton (403): Karatsovis PhD, Miranda Internoise 2022 — evtl. mit Messungen an Flügeln.
- Lokal schon vorhanden und für Zahlen am ehesten ergiebig: Bank 2000/2010 (Modalsynthese mit gemessenen Doppeldecays?), Chabassier 2013 (Steinway D, Simulation vs. Messung), Tan 2017 (Doppelpolarisation) — Sache des anderen Agenten.
- Weinreich Fig. 1/Fig. 8: Zahlen nur aus Abbildungen; Bilder nicht ausgewertet.
- η_air je Taste aus Saitenplan rechnen (Q5, Gl. 2–3) als physikalische Obergrenze — Vorschlag, nicht getan.

## Lokal abgelegte PDFs (Freigabe Patrik über Koordinator, Ordner gitignored)

`C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\hammer-saite\quellen\`
| Datei | URL | Größe | Lizenz/Zugang |
|---|---|---|---|
| Woodhouse_2022_NecessaryConditionDoubleDecay_JASA_AAM.pdf | repository.cam.ac.uk (Apollo) | 1 115 278 B | AAM frei, © 2022 ASA |
| Cheng_Dixon_Mauch_2015_ModellingDecayPianoSounds_ICASSP.pdf | webspace.eecs.qmul.ac.uk/s.e.dixon/… | 423 748 B | Autorenseite frei, keine Lizenzangabe |
| Aramaki_Bensa_Daudet_Guillemain_KronlandMartinet_1999_CoupledPianoStrings_DAFx99.pdf | dafx.de/paper-archive/1999/aramaki.pdf | 59 586 B | DAFx-Archiv frei, keine Lizenzangabe |
| Lehtonen_2005_AnalysisParametricSynthesisPianoSound_MSc_TKK.pdf | lib.tkk.fi/Dipl/2005/urn007876.pdf | 3 652 397 B | Hochschulbibliothek frei, keine Lizenzangabe |

Die ersten drei sind Kopien der WebFetch-Zwischenablage (gleicher Inhalt), die vierte per curl.

## WebFetch-Zwischenkopien

- `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\tool-results\webfetch-1793045302671-3ertzb.pdf` (Aramaki 1999)
- `…\tool-results\webfetch-1793045314768-ofn8e9.pdf` (Cheng 2015)
- `…\tool-results\webfetch-1793045522895-twruv1.pdf` (Woodhouse 2022)
- `…\tool-results\webfetch-1793045610017-ok1ewn.pdf` (Miranda Valiente 2024, liegt schon lokal)
- Textauszüge (pdftotext) im Scratchpad: `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\scratchpad\recherche\txt\` (aramaki, cheng, woodhouse, valiente, lehtonen2005 .txt)
