# Messkette — was die Teiltonmessung des Flügels für andere Klassen nicht kann

> **Klasse 0 (Gemeinsam).** **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorab-Ausarbeitung, nichts entschieden.
> **Grundlage:** LumiViz (nur gelesen): `tests/unit/UnitTests/Teiltonmessung.hpp` (Kopf und Strukturen), Messplan des
> Flügels (`docs/modellierung/hammer-saite/messplan.md`), Entscheide UF3 a, UF5 a, IK1; Bericht
> `recherche/2026-10-26_messkette_verfahren.md` (Agent).
> **Nachgetragen 2026-10-27 (IC Session 3, Auftrag Patrik):** §3a und §4a aus den Lehren L1, L3, L4 des Flügels
> ([VORBILD_FLUEGEL.md](../../anbindung-ic/ic/VORBILD_FLUEGEL.md), Input [LEHREN_SETZEN_FLUEGEL.md](../../anbindung-ic/ic/LEHREN_SETZEN_FLUEGEL.md)).
> **Kennzeichnung je Angabe:** **nachgelesen** (von dieser Sitzung im Code, Volltext oder Feld selbst gelesen) ·
> **nur Bericht** (Agentenbericht) · **Einschätzung** (Claude, kein Beleg). Offene Entscheidungen:
> [offene-fragen.md](../../gemeinsam/ic/offene-fragen.md).
> **Stand 2026-10-29, IC Session 4 (Cloud): gebaut** unter `rechnung/messung/` — die Messkette des Flügels portiert
> (`Teiltonmessung`, `Fluegelmessung`, `Strukturmessung`, `Zielsumme`, `LevenbergMarquardt`, `NelderMead`,
> `Kurvenabstand`; nur Namensraum und FFT-Schnittstelle geändert) und die Bausteine **B1–B8** (`Bausteine.hpp`: B6 als
> reelles ESPRIT ohne Bänder und ohne ESTER — Ordnung fest, offen; B4 als Rest nach kleinsten Quadraten je Rahmen),
> dazu `amplitudenBei` für die Messung des Modells an der Struktur des Bezugs (§3a). Geprüft an synthetischen Signalen
> (`test_messung.cpp`): Teiltonmessung B auf 3 %, f0 auf 10⁻⁵, T60 auf 3 %, Schwebung auf 2 %; Modalfit Frequenz
> 10⁻⁴, T60 3 %, Pegel 0,5 dB (drei Moden, Rauschen −60 dB) und am gerechneten Xylophon; YIN 10⁻³, Vibrato Rate 2 %,
> Tiefe 5 % (mit Berichtigung des Fensters), AM 10 %; Rest gegen das Verhältnis periodisch/Rauschen 3 %; Gleiten 0,2 %;
> Einschwingen 15 %; das Zielformat `salamander_ziel.txt`/`salamander_struktur.txt` gelesen und wieder geschrieben.
> **Nur lokal:** `ic_messe` misst einen Datensatz unter `IC_SAMPLES` (Wurzel nie im Code; Prüfung über `SHA256SUMS`;
> AIFF noch nicht lesbar — offen).

## 1. Was die Messung heute voraussetzt (nachgelesen, `Teiltonmessung.hpp`)

Dieselbe Messung an Aufnahme und Modell (UF3 a), reines C++ ohne Qt nur gegen `Fft` (IK1). Ablauf von `miss`:
Einsatz (erste Probe über 1 % der Spitze) → **ein** langes Spektrum ab 50 ms → Teiltöne um **k·f0·√(1 + B·k²)**, f0
und B als Gerade ausgeglichen, fremde Spitzen verworfen → Hüllkurven je Teilton aus einer Kurzzeit-FFT bei **fester
Frequenz** → zwei Abklingstufen aus der rückwärts integrierten Energie (Schroeder) → Schwebung im Rest →
σ(ω) = b1 + b3·ω² → dazu Pegel, Direktschall, Terzbänder, Phantomteiltöne.

Daraus die Annahmen:

| | Annahme | steckt in |
|---|---|---|
| A1 | Teiltöne bilden eine **Saitenreihe** k·f0·√(1+Bk²) | Suche und Ausgleich der Frequenzen |
| A2 | jeder Teilton hat **eine feste Frequenz** über den Ton | ein langes Spektrum; Hüllkurve an fester Stelle |
| A3 | der Ton **klingt aus** (kein gehaltener Teil) | Schroeder-Abklingen, zwei Stufen, „mindestens 10 dB Abfall“ |
| A4 | was zählt, ist **periodisch**; Rauschen ist Boden | Boden zwischen den Teiltönen nur als Grenze |
| A5 | der **Einsatz** ist ein Schlag (Hammer) | Einsatz bei 1 % der Spitze; Spektrum erst ab 50 ms |
| A6 | Abklingen über der Frequenz folgt b1 + b3·ω² | Bank 2010, Gl. 7 — ein Gesetz der Saite |

## 2. Wo die Annahmen brechen — je Klasse

✗ = bricht, ~ = teilweise, ✓ = hält (Einschätzung, aus der Physik der Klassen in [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md)).

| Klasse | A1 Reihe | A2 feste f | A3 Ausklang | A4 periodisch | A5 Schlag | A6 b1+b3ω² |
|---|---|---|---|---|---|---|
| 1 Hammer-Saite (Rest) | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 2 Gezupfte Saite | ✓ | ~ (Bending, Vibrato) | ✓ | ✓ | ✓ (Zupfen) | ~ |
| 3 Bogen-Saite | ✓ (harmonisch erzwungen) | ✗ Vibrato | ✗ gehalten | ✗ Bogengeräusch | ✗ Einschwingen | ✗ |
| 4 Platte und Stab | ✗ (Marimba etwa 1 : 4 : 10, Unterschnitt) | ✓ (Vibraphon: Tremolo als AM) | ✓ | ✓ | ✓ | ✗ |
| 5 Membran | ✗ (Bessel-Reihe, mit Luftlast verschoben) | ✗ Gleiten nach dem Schlag | ✓ | ~ (Teppich der Snare) | ✓ | ✗ |
| 6 Becken, Gong | ✗ (dichte Moden) | ✗ (Gleiten, Energiefluss nach oben) | ✓ | ✗ | ✓ | ✗ |
| 7/8/9 Rohr | ✓ (harmonisch erzwungen) | ✗ Vibrato | ✗ gehalten | ✗ Anblasgeräusch | ✗ Einschwingen | ✗ |
| 10 Zunge | ✓ | ~ | ✗ gehalten | ~ | ✗ | ✗ |
| 11 Tonrad | ✓ (Zugriegel, fast Sinus) | ✗ Leslie (Doppler) | ✗ gehalten | ~ (Klick) | ✗ | ✗ |

**Einschätzung:** Für die Klassen 1, 2 und 4 (nach A1-Ersatz) trägt die Messung des Flügels weitgehend; für alle
selbsterregten Klassen (3, 7–11) fehlen der gehaltene Teil, der Tonhöhenverlauf, das Geräusch und das Einschwingen.

## 3. Neue Bausteine

Vorschlag (Einschätzung): die Messung in einen **Analysekern ohne vorgegebene Reihe** und eine **Deutung je Klasse**
teilen — der Kern liefert Spuren (Frequenz und Amplitude je Teilton über der Zeit, Restsignal), die Deutung macht
daraus die Messgrößen, die eine Modellgröße setzen (Lehre aus dem Flügel: je Modellgröße die Messgröße, die sie
verursacht).

| Baustein | Misst | Verfahren (Quelle) | für Klassen | Belegkraft |
|---|---|---|---|---|
| **B1 Teiltonverfolgung** mit Frequenzverlauf | Frequenz und Amplitude je Teilton je Rahmen; Teiltöne entstehen und enden | Spitzen der Kurzzeit-FFT, Verfolgung über „birth“ und „death“, Phase kubisch interpoliert (McAulay & Quatieri 1986); deterministisch + stochastisch (Serra & Smith 1990) | 2, 3, 7–11 | nur Bericht (Wortlaut MQ vom Agenten an der Seite des Lincoln Lab) |
| **B2 Tonhöhenverlauf** | f0(t), stimmhaft ja/nein | YIN (de Cheveigné & Kawahara 2002), pYIN (Mauch & Dixon 2014), SWIPE (Camacho & Harris 2008); CREPE (lernend, nur zum Vergleich) | 3, 7–11, 5 (Gleiten) | nur Bericht |
| **B3 Vibrato** | Rate (Hz), Tiefe (Cent), Form; AM je Teilton und ihre Lage zur FM (Teiltöne laufen durch Körperresonanzen) | aus f0(t) bzw. aus B1 (Arroabarren, Rodet & Carlosena 2006; Essentia `Vibrato`, Suchbereich 4–8 Hz) | 3, 7–9, 11 (Leslie) | nur Bericht |
| **B4 Geräusch** | Restsignal nach Abzug der Teiltöne: Pegel gegen das Periodische, Hüllkurve in Bändern, **Modulation mit der Periode** | Residuum aus B1 (SMS); harmonisch + Rauschen (Laroche, Stylianou & Moulines 1993; Stylianou 2001; Pantazis & Stylianou 2008); HNR nach Boersma 1993 | 3, 5 (Teppich), 6, 7–10 | nur Bericht |
| **B5 Einschwingen** | Dauer bis zum regelmäßigen Ton, Reihenfolge und Anstieg der Teiltöne, Vorläufergeräusch | aus B1 und B4; Wahrnehmungsgrenze beim Bogen: Dauer des Vor-Helmholtz-Einschwingens (Guettler & Askenfelt 1997) | 3, 7–10 | nur Bericht |
| **B6 Modalfit ohne Reihe** | Frequenz, Dämpfung, komplexe Amplitude je Mode, auch bei überlappenden Moden | ESPRIT mit Ordnungsschätzung ESTER, je Band nach FIR-Zerlegung, Verschiebung und Unterabtastung (Ege, Boutillon & David 2009) | 4, 5, 6 (unten), Körper aus Messung ([koerper.md](../../koerper/ic/koerper.md)) | Titel, Autoren, Journal-Ref, Lizenz **nachgelesen** (arXiv 0909.0885); Verfahren nur Bericht |
| **B7 Band über der Zeit** | Energie je Band über der Zeit, spektraler Schwerpunkt, Abklingen je Band | — (Einschätzung; kein Messverfahren gefunden) | 6, 5 (Snare) | Einschätzung |
| **B8 Gleiten** | Frequenz der stärksten Mode über die ersten 100–300 ms | aus B1/B2 (Einschätzung); Physik: Avanzini, Marogna & Bank 2012 (Membran), Legge & Fletcher 1989 (Gong) | 5, 6 | nur Bericht; **kein Messverfahren gefunden** |

**Bezug zum Bestand in LumiViz (IC-B1 a; nachgelesen, `Spielweisen_Konzept.md` §1, §3):** Vibrato, Pitch und Expression
sind dort **Spuren je Muster** (SF5 d, Stufe 28), bei dauernder Anregung wirken sie auf die Anregung selbst (SF10 a),
und eine leere Spur heißt „Instrument default“. B2/B3 messen also nicht, was der Spieler tut, sondern liefern die
**Vorgaben** des Instruments (Rate und Tiefe des Vibratos, Einschwingen bei Expression 1) und den Prüfstein, ob das
Modell eine gegebene Spur wie die Aufnahme umsetzt; dasselbe gilt für Leslie langsam/schnell als Spur mit
Umschaltstellen (SF8 a — „das Modell rechnet den Übergang“, also auch das Hochlaufen, gemessen mit B3).

**Was bleibt, wie es ist** (Einschätzung): Schroeder-Abklingen für jeden **Ausklang** (auch nach dem Loslassen eines
gehaltenen Tons), Pegel, Terzbänder, Direktschall; die Regeln aus dem Flügel (dieselbe Messung an Aufnahme und Modell,
Setzen durch das Modell hindurch, gültiger Bereich je Messgröße, Bilder vor Zahlen).

## 3a. Erkennung nur am Bezug (Regel für jede Klasse mit Erkennungsschritten)

Lehre L4 des Flügels (ZF3 a, LumiViz S156; nachgelesen): Wo die Messung eine **Struktur erkennt** — eine zweite
Abklingstufe ja/nein, eine Schwebung ja/nein —, kippt das Ergebnis an Schwellen. Misst man das Modell mit derselben
Erkennung, wird die Zielfläche eine Treppe, und jede Suche bleibt an einem anderen Absatz hängen (beim Flügel glatt an
0 von 6 Tasten). Daher:

1. Die Erkennung läuft **einmal am Bezug** (der Aufnahme) und legt dessen Struktur fest: Knick, Frequenz der Schwebung,
   Beginn des regelmäßigen Tons, Rate des Vibratos, Bänder des Geräuschs.
2. Das Modell wird **an dieser Struktur gemessen**, ohne eigene Erkennung: zwei Geraden mit dem Knick des Bezugs, die
   Schwebung als Anteil des Rests bei der Frequenz des Bezugs — die Messgrößen des Modells sind so von Bau aus stetig.
3. Die Erkennung am Modell bleibt **Prüfgröße im Bericht** (liegt seine Struktur anderswo, zeigt sich das als
   Abweichung, nicht als Sprung).

Wo das in den neuen Bausteinen greift (Einschätzung):

| Baustein | Erkennung am Bezug | Messung des Modells an der Struktur des Bezugs |
|---|---|---|
| B1 Teiltonverfolgung | „birth“/„death“ eines Teiltons | Amplitude des Modells bei den Frequenzen und in den Zeitfenstern der Teiltöne des Bezugs |
| B2/B3 Tonhöhe, Vibrato | stimmhaft ja/nein, Rate und Tiefe | f0 des Modells im Fenster des Bezugs; Anteil bei der Vibrato-Rate des Bezugs |
| B4 Geräusch | Trennung periodisch/Rest | Rest des Modells nach Abzug seiner Teiltöne, in den Bändern des Bezugs |
| B5 Einschwingen | Zeitpunkt „regelmäßiger Ton“ | Energie und Periodizität des Modells über dem Zeitraster des Bezugs |
| B8 Gleiten | Frequenzverlauf der stärksten Mode | Frequenz des Modells in den Fenstern des Bezugs |
| Abklingen (Flügel) | eine oder zwei Stufen, Knick | zwei Geraden mit dem Knick des Bezugs |
| Register, Überblasen (7–10) | welche Mode trägt den Ton | Pegel des Modells bei den Teiltönen des Bezugs; eine Note im anderen Register ist eine große Abweichung, kein Sprung der Messung |

## 3b. Glätte des Ziels vor dem Plan (L16, berichtigt L12)

Lehre L16 des Flügels (LumiViz S164/S165, `LEHREN_SETZEN_FLUEGEL.md` §6; nachgelesen): Die Messung nach §3a (Schwebung
als Anteil bei der **festen** Frequenz des Bezugs) ist stetig, aber **wellig** in jedem Parameter, der die Schwebung
verschiebt — läuft die Schwebung des Modells an der Messfrequenz vorbei, oszilliert der Anteil. Tragen zwei Mechanismen
dieselbe Wahrnehmung (beim Flügel Chor-Verstimmung und Polarisation), scheitert jedes Staffeln nach L12; die Welligkeit
sitzt im Ziel, nicht in der Suche. Daher:

1. Vor jedem Plan mit Glätte-Schwelle die Glätte des **Ziels** in jedem Parameter prüfen, der eine Frequenz oder Rate
   verschiebt (Schwebung, Vibrato, Tremolo, Register, Leslie) — Werkzeug `ic::setzen::welligkeit` (innere Minima je
   Messgröße auf dem Schnitt) und `ic::setzen::glaette` (Σ r² nach ZF8 a (2): 15 Punkte, höchstens ein Minimum, kein
   Schritt über das Dreifache des mittleren — das Maß der Probe HW3 a).
2. Ist das Ziel dort wellig, **die Messgröße ändern**, nicht die Suche: Frequenz und Tiefe mit eigener Suche der Frequenz
   am Modell messen (statt Anteil bei fester Frequenz), oder die Form der Hüllkurve.
3. Parameter, die dieselbe Wahrnehmung tragen, werden **nie einzeln gestaffelt** (`Parameter::traegt`; das
   Setz-Werkzeug staffelt sie nicht und nennt sie im Bericht).

**Geprüft am Modell (IC-A4, 2026-10-31, `ic_glaette_ziel`, `rechnung/werte/glaette_ziel.txt`):** die Amplitude bei
fester Frequenz ist in allen geprüften Klassen wellig; Rate und Tiefe mit eigener Suche sind glatt bei 3, 4, 7, 9, 10,
11, nicht bestimmt bei 1 und 2 (langsame Chorschwebung braucht mehr Tondauer), offen bei 5 (Träger der Schwebung) und 8
(Vibrato ohne FM); 6 nicht prüfbar. Je Klasse im `messplan.md`. **Lücke im Baukasten:** B3 sucht die Rate nur in der FM;
eine AM-Rate (Tremolo, Musette, Rotary, Schwebung) misst heute nur das Werkzeug (`amRate`) — als Baustein offen.

Wo das in diesem Projekt greift (Einschätzung): Chöre der Klassen 1 (Hackbrett, Cimbalom: Verstimmung im Chor und
Polarisation), 2 (12-saitige Gitarre, Laute), 11 (Leslie: Rotor und Horn), 10 (Akkordeon-Tremolo: zwei Zungen und
Balg-Vibrato), 3/7/8/9 (Vibrato der Spieler: Rate und Tiefe aus der Spur).

## 4a. Toleranz als Auflösung des Bezugs

Lehren L1 und L3 des Flügels (ZF1 a, ZF4 a, SP3 a, LumiViz S156–S157; nachgelesen):

- **Toleranz je Messgröße = gemessene Streuung des Bezugs selbst**, festgelegt, bevor gesetzt wird. Beim Flügel die
  größere von zwei Streuungen: zwischen benachbarten Anschlagstufen derselben Taste und zwischen den beiden Kanälen
  derselben Stufe; verglichen wird am Mittel der Kanäle. Bei *Salamander* streuten die **Kanäle** (1,5–7,0 dB) stärker
  als die **Stufen** (0,6–2,2 dB) — der Ort des Mikrofons zählt mehr als der Anschlag; ein Modell, das an einem
  anderen Ort hört, kann der Aufnahme nicht näher kommen als die Mikrofone einander.
- **Rezept je Datensatz** — welche Streuung er hergibt (Einschätzung, nach Art der Datensätze dieser Sammlung):

  | Datensatz hat … | Streuung messbar aus | Beispiel |
  |---|---|---|
  | mehrere Stärken je Note | Nachbarstufen (pp/mf/ff: nur zwei Abstände, grob) | Iowa, TinySOL, K18 (6 Stufen) |
  | mehrere Kanäle/Mikrofone | Kanäle derselben Stufe | *Salamander*, TU-Note (DPA/B&K), VSCO (stereo) |
  | Wiederholungen derselben Note | Wiederholungen (die sauberste Streuung des Spiels) | Lampis (Wiederholungssitzungen), Roboterbogen |
  | mehrere Instrumente derselben Bauart | Streuung über Instrumente (nur für Vorgaben, nicht zum Setzen eines Instruments) | Mores (60+ Gitarren), CNSM (drei Violinen) |
  | eine Stufe, ein Kanal, keine Wiederholung | **nichts** — vor dem Setzen eine zweite Quelle | VCSL-Blockflöte (keine Stufen), Euterpea-Harmonium, fese-Melodica (je eine Stärke) |

- **Gültiger Bereich je Messgröße** (L3): Abklingzeiten bestimmt die Aufnahme nur bis etwa zum Dreifachen ihrer Länge
  (beim Flügel: `lateDecay` 355 s gegen 637 s unterschieden sich über 23 s Aufnahme um 1,7 dB); Schwebungen nur, wenn
  sie schneller sind als das Fenster; Tiefen nur über dem Boden. Außerhalb ist die Größe von der Aufnahme nicht
  bestimmt — sie wird aus einer Quelle oder nach Gehör gesetzt, nicht gesucht.
- **Eindeutig** (HW2 a, löst SP3 a ab; IC-A3): eine Messgröße, an der die Startpunkte höchstens max(τ, S) auseinander
  enden — unter τ unterscheidet die Aufnahme nicht, unter der Hörschwelle S das Ohr nicht (S aus
  `rechnung/werte/hoerschwellen.txt`, `gemeinsam/hoerschwellen.md`); bei ungleichen Grenzen gilt die engere, fehlt die
  Größe an einem Startpunkt, ist sie nicht eindeutig. Ebenso **hörbar getroffen** bei |d| ≤ max(τ, S); die Treffer
  nach τ allein bleiben im Bericht. Verschiedene Parameter bei gleichen Messgrößen heißen weiter „von der Aufnahme nicht
  aufgelöst“. Werkzeuge: `ic::setzen::setzeMehrfach`, `eindeutig`, `probe` (HW3 a), `messung::systematik` (HW4 a).

## 4. Toleranzen — was die Literatur sagt

- McAdams, Beauchamp & Meneguzzi 1999 (Resynthesen von Klarinette, Flöte, Oboe, Trompete, Violine, Cembalo, Marimba):
  gut hörbar sind das Glätten der spektralen Hüllkurve und der Verlust der Kohärenz der Amplitudenhüllkurven; schlecht
  hörbar das Glätten der Frequenzschwankungen und erzwungen harmonische Frequenzen (nur Bericht). → eine Rangfolge,
  **welche Messgröße eng toleriert werden muss**: spektrale Hüllkurve eng, Frequenzschwankungen weiter (Einschätzung).
- Horner, Beauchamp & So 2006: ein relativer spektraler Fehler sagt die Unterscheidbarkeit am besten voraus (nur
  Bericht; nicht frei).
- Guettler & Askenfelt 1997: annehmbare Dauer des Einschwingens beim Bogen rund 50 bzw. 90 ms (nur Bericht).
- Eine Prüfung **physikalischer** Modelle gehaltener Töne gegen Aufnahmen wurde nicht gefunden (nur Bericht).

## 5. Vorbilder im Code (nur lesen — kein Code wird übernommen)

Lizenzen laut Bericht am Ort wörtlich gelesen, **von dieser Sitzung nicht nachgeprüft**: librosa (ISC; yin, pyin),
CREPE (MIT), sms-tools (AGPL-3.0; Sinus-, Harmonik-, Residuum-Modelle), Essentia (AGPL-3.0; Sinus-, Harmonik-,
Vibrato-, Inharmonizitäts-Algorithmen), aubio (GPL-3.0), pYIN als Vamp-Plugin (GPL-2.0), Loris (GPL-2.0, nur
SourceForge-Feld), SPEAR (keine Lizenz angegeben). Für den eigenen Code gilt IK1 (reines C++ ohne Qt, gegen `Fft`);
nachgebaut wird aus den Veröffentlichungen. Am besten beschrieben und frei lesbar: Ege, Boutillon & David 2009 (B6) und
pYIN (B2).

## 6. Offen

- Volltexte lesen (Download mit Freigabe): Ege, Boutillon & David 2009 (arXiv), Guettler & Askenfelt 1997,
  McAdams et al. 1999, Boersma 1993, de Cheveigné & Kawahara 2002.
- Nicht gesucht: Messverfahren für Orgel/Tonrad, Zunge, Snare-Teppich, Rauschhüllkurven in ERB-/Bark-Bändern, Matrix
  Pencil, Prony.
- Wo die erweiterte Messung lebt (im Testbaum wie heute oder als eigenes Werkzeug), entscheidet sich mit IK1 — nicht
  Teil dieses Auftrags.
