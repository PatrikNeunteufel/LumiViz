# Lehren aus dem Setzen des Flügels — Input aus LumiViz S157

> **Von:** LumiViz, Session 157 (2026-10-27, Sitzung mit Fable 5.1 auf Wunsch Patrik, nach ZZ1 a in S156).
> **Für:** die Sitzung in diesem Ordner. **Auftrag Patrik (S157):** „wenn du da auch noch Potential siehst, um
> optimaler vorzugehen, bitte dort auch entsprechende Inputs notieren.“
> **Bezug (nur lesen):** `…\LumiViz\projects\apps\LumiViz\docs\audio\Spielweisen_Konzept.md` §4.1 ab SM1 a (S154) bis
> SP3 a (S157), `docs\modellierung\hammer-saite\proben\` (die Messwerte der Proben), `grenzen.md`.
> **Belegkraft:** alles hier ist von Claude aus den LumiViz-Dokumenten und den Berichten der Proben gelesen, nichts aus
> fremden Quellen; Zahlen mit Herkunft (Datei) genannt. **Diese Datei ändert nichts an AUFGABE.md** — was davon in den
> Auftrag soll, entscheidet Patrik; Vorschläge unten als Z2 und Z3 formuliert.
> **Nachtrag 2026-10-29 (LumiViz S163, Auftrag Patrik):** §5 mit den Lehren L8–L15 aus der Probe 7.5.11 — die Suche
> über die Stützpunkte ist gelaufen (S162) und geprüft (S163); Z2 und Z3 sind seit 2026-10-28 im Auftrag (F-L1 a,
> F-L2 a). Die Kurzfassung aller Lehren steht in [VORBILD_FLUEGEL.md](VORBILD_FLUEGEL.md); der Bauauftrag für die
> Klassen in [AUFTRAG_BAU.md](AUFTRAG_BAU.md) nimmt L1–L15 als Regeln.
> **Nachtrag 2026-10-30 (LumiViz S164/S165, Auftrag Patrik):** §6 mit L16–L18 aus Plan A–D — Phase B (Hörschwellen
> recherchiert), Phase A gebaut und an sechs Tasten geprüft, Probe 7.5.12 an der Glätte verfehlt, Ausstieg nach ZZ1 a
> (PV1 a). **L16 berichtigt L12.**

## 1. Was passiert ist (drei Sätze)

Vom 25.10. (31d-4) bis 27.10. (S156) klang der Flügel in LumiViz unverändert; vier Sitzungen bauten Messgeräte, und
sieben Anläufe, die Größen der Hammer-Saite automatisch an *Salamander* zu setzen, scheiterten an denselben Mustern:
Suche nicht eindeutig, Parameter an Grenzen, `decay` läuft ins Unphysikalische, das Modell verliert und gewinnt
Teiltöne und Stufen an Schwellen (Konzept §4.1, Befunde S154–S156). Dabei lag der Kipppunkt des letzten Entscheids
(ZZ1 a: lohnt das automatische Setzen überhaupt, oder klingt 31d-4 schon gut?) zwei Tage ungehört in den Hörproben.
Entschieden in S157 (SP1 a–SP3 a): zuerst spielbar machen (mitgeliefertes `grand_piano`, Werte nach Gehör), die
Feinabstimmung danach — mit einer Suche über die Kurven statt je Taste und Eindeutigkeit an den Messgrößen statt an
den Parametern.

## 2. Lehren — und wohin sie hier gehören

Je Lehre: was LumiViz gelernt hat, dann der Vorschlag, wo es in diesem Projekt steht (die Sitzung hier entscheidet
mit Patrik, nichts davon ist still zu übernehmen).

### L1 — „Gut genug“ vor dem Setzen festlegen, als Zahl aus der Aufnahme

In LumiViz stand bis S156 nie fest, wann eine Vorgabe brauchbar ist; „nicht brauchbar“ fiel dreimal am Bild. Erst ZF1 a
und ZF4 a (S156) setzen die Toleranz je Größe als **gemessene Streuung der Aufnahme selbst**: zwischen benachbarten
Anschlagstufen derselben Taste und zwischen den beiden Mikrofonkanälen derselben Stufe, die größere von beiden. Befund
dazu (Probe 7.5.4, Zeile (7)): bei *Salamander* unterscheiden sich die **Kanäle** einer Taste stärker (1,5–7,0 dB) als
die **Stufen** (0,6–2,2 dB) — der Ort des Mikrofons zählt mehr als der Anschlag. Ein Modell kann der Aufnahme nicht
näher kommen als die zwei Mikrofone einander.
→ **`messplan.md` jeder Klasse:** je Messgröße die Toleranz nicht schätzen, sondern als Rezept angeben, wie sie aus dem
Datensatz gemessen wird (Nachbarstufen, Kanäle, Wiederholungen derselben Note, sofern der Datensatz sie hat);
`aufnahmen.md`: je Datensatz vermerken, ob er diese Streuung überhaupt hergibt (eine Stufe, ein Kanal → keine
Toleranz messbar → vor dem Setzen eine zweite Quelle). **`gemeinsam/messkette.md`:** ein Abschnitt „Toleranz als
Auflösung des Bezugs“.

### L2 — Die Suche setzt Kurven über den Tonumfang, nicht Werte je Taste

Die Größen des Flügels sind Kurven aus Stützpunkten, monoton kubisch interpoliert (`SCHNITTSTELLE.md` §2). Das Setzen
suchte aber **je Taste** 6–8 Parameter gegen rund 15 Messgrößen, fand je Taste Mehrdeutiges und interpolierte die
Glätte danach wieder hinein. SP2 a (S157) dreht das um: Levenberg–Marquardt über die **Stützpunkte** aller Kurven, Ziel
die Summe über alle 30 Tasten (rund 450 Messgrößen gegen rund 50 Parameter), je Taste ihr Anteil als Ausreißer-Anzeige.
→ **`modell-entwurf.md` jeder Klasse:** je Größe sagen, ob sie eine Kurve über den Tonumfang ist (und wie viele
Stützpunkte die Physik verlangt — Stegwechsel, Saitenwechsel, Register), fest, oder von der Stärke abhängig; die
Zahl der Stützpunkte ist die Zahl der zu setzenden Parameter. **`messplan.md`:** das Setzen als Ausgleich über alle
Tasten zugleich planen, nicht Taste um Taste. **`SCHNITTSTELLE.md` §Messkette:** den Satz „Setzen durch das Modell
hindurch (je Größe die Messgröße, die sie verursacht; halber Schritt)“ um „über die Stützpunkte der Kurven, nicht je
Taste“ ergänzen — der halbe Schritt (S3 a) war das Verfahren von 31d-4 und hat die zweite Stufe nicht erreicht.

### L3 — Eindeutigkeit an den Messgrößen prüfen, nicht an den Parametern

Zwei Startpunkte fanden an Taste 33 `lateDecay` 355 und 637 s — „nicht eindeutig“ nach Wortlaut. Die Aufnahme ist 23 s
lang; über 23 s unterscheiden sich die beiden um 1,7 dB. Die Zeile maß die Aufnahme, nicht die Suche. SP3 a: eindeutig
ist, wenn beide Startpunkte an jeder Messgröße innerhalb ihrer Toleranz enden; der Abstand der Parameter steht im
Bericht als „von der Aufnahme nicht aufgelöst“.
→ **`messplan.md`:** je Modellgröße den **gültigen Bereich** angeben, in dem die Aufnahme sie überhaupt bestimmt
(Abklingzeiten höchstens etwa das Dreifache der Aufnahmelänge; Schwebungen schneller als das Fenster; Tiefen unter dem
Boden) — außerhalb ist der Wert frei und gehört nach Gehör oder aus einer Quelle gesetzt, nicht gesucht.

### L4 — Ein Modell mit Struktur an Schwellen braucht eine Messung ohne eigene Erkennung

Im Chor gibt es ohne Verstimmung keine späte Stufe (Physik: ohne Verstimmung liegt keine Energie in den Gegenlagen);
ob eine zweite Stufe oder eine Schwebung *erkannt* wird, kippt an Schwellen — so wurde die Zielfläche eine Treppe, auf
der jede Suche an einem anderen Absatz hängen blieb (SM3 a, SM4 a; Probe 7.5.4 Zeile (2): glatt an 0 von 6 Tasten).
ZF3 a: die Aufnahme einmal mit Erkennung messen (Ziel), das Modell dann **an der Struktur des Ziels** messen (zwei
Geraden mit dem Knick der Aufnahme, Schwebung als Anteil bei der Frequenz der Aufnahme) — stetig von Bau aus.
→ **`gemeinsam/messkette.md`:** als Regel für jede Klasse mit Erkennungsschritten (Einschwingen, Stufen, Vibrato,
Rauschanteile): Erkennung nur am Bezug, das Modell an dessen Struktur gemessen. **`modell-entwurf.md`:** je Klasse
nennen, wo das Modell seine Struktur an Schwellen wechselt (Register eines Rohrs, Anstreichen gegen Rutschen,
Kopplung mehrerer Saiten) — dort ist Setzen durch Suche heikel, dort gehört ein Entscheid „gesucht oder gesetzt“.

### L5 — Hören vor Rechnen: das Hörurteil ist ein Kipppunkt mit Datum

Jeder der sieben Anläufe endete an einer Zahl, keiner an einem Ohr. Die Hörproben (gleich laut nach R128, Aufnahme
| Modell vor | Modell nach, mit `liste.txt`) lagen seit dem 25.10. da. ZZ1 a hängt ausdrücklich davon ab.
→ **`VORBILD_FLUEGEL.md` §Lehren:** „Eine Vorgabe gilt erst als nicht brauchbar, wenn sie gehört wurde; Hörproben
werden im selben Schritt abgelegt wie die Messung, und das Urteil hat ein Datum, bevor der nächste Setz-Anlauf
beginnt.“ **`messplan.md`:** neben den Messgrößen die Hörproben als eigene Zeile (welche Noten, welche Stärken,
gegen welchen Bezug, Frage je Datei).

### L6 — Spielbar vor Feinabstimmung

Nach fünf Sitzungen war der Flügel im Programm nicht anspielbar (`grand_piano` war erst für den Abschluss der Stufe
vorgesehen, die Bauart im Creator erst für 31g). Klangfragen wurden an Kurvenbildern entschieden, nie am Instrument
neben den anderen. SP1 a zieht das vor: mitgeliefertes Instrument mit dem **besten gemessenen Stand**, Werte ohne Quelle
und ohne Messung nach Gehör innerhalb belegter Grenzen (SQ2 a), Feinabstimmung danach mit denselben Werkzeugen.
→ **`VORBILD_FLUEGEL.md` §Der Ablauf:** die Zeile „Hörproben, Editor, Abschluss — nichts vorab“ ergänzen um einen
Schritt **„spielbar“** gleich nach „Modell bauen“: mitgeliefertes Instrument mit Vorgaben aus Quellen, bevor gesetzt
wird; der Messplan baut darauf auf, nicht davor. **`KLASSEN.md`:** je Klasse den Schritt „spielbar mit Quellenwerten“
als eigene Aufwandszeile, getrennt von „an Aufnahme gesetzt“ — die zweite Zahl war beim Flügel um Sitzungen zu klein.

### L7 — Rechenzeit, Wärme, und was eine Probe kosten darf

Der Rechner steht unter jeder Last an der Wärmegrenze (Tctl 94–99,6 °C schon bei einem Thread, `proben/S156_zeit75/`);
mehr als 11 Threads machen die Rechnung langsamer. Eine Auswertung des Flügels über 11 Stufen kostet 1,5 s, ein Setzen
30 Tasten × 2 Startpunkte rund 4 h, eine Probe 1 h. Das bestimmt, wie viele Anläufe ein Setzen haben kann.
→ **`modell-entwurf.md`:** die Rechenzeit **je Auswertung beim Setzen** (ein Ton je Stufe, gemessen) neben der
Rechenzeit je Ton beim Spielen schätzen; `messplan.md`: Zahl der Auswertungen je Suche und daraus die Dauer eines
Setz-Laufs — vor dem ersten Lauf, mit Ergebnisdatei je Taste bzw. je Iteration (SE1 a), damit ein Abbruch nicht alles
kostet (die erste Probe in S155 brach mit der Sitzung ab, die sie im Hintergrund gestartet hatte).

## 3. Vorschläge für `AUFGABE.md` (Zusatzfragen aus LumiViz; Patrik entscheidet)

**Z2 — Kurven als Suchraum.** Für jede Klasse: Welche Größen sind Kurven über den Tonumfang, welche fest, welche
hängen an der Stärke; wie viele Stützpunkte verlangt die Physik je Kurve (Wechsel von Steg, Saite, Register, Bohrung);
und reicht die Zahl der Noten im Bezugsdatensatz, um diese Stützpunkte zu bestimmen (Messgrößen gegen Parameter)?
Ergebnis als Tabelle in `modell-entwurf.md`, Lücken als Frage.

**Z3 — Toleranz als Auflösung des Bezugs.** Für jeden Datensatz in `aufnahmen.md`: Hat er Nachbarstufen, mehrere
Kanäle oder Wiederholungen derselben Note, aus denen sich je Messgröße eine Streuung messen lässt? Wenn nein: welche
zweite Quelle gäbe sie? Ergebnis als Spalte „Toleranz messbar aus“ in `aufnahmen.md` und als Rezept in `messplan.md`.

## 4. Was hier *nicht* steht

Keine Entscheide für dieses Projekt — die Sitzung hier legt Patrik Fragen im Format aus `AUFGABE.md` vor. Keine
Änderung an LumiViz-Dokumenten von hier aus (Regel bleibt). Die Entscheide SP1 a–SP3 a und der Teilplan 31d-8 stehen
im LumiViz-Konzept; was davon für die Klassen 2–11 gilt, steht oben als Vorschlag.

## 5. Nachtrag S163 — Lehren aus der Probe 7.5.11 (LumiViz S162/S163, 2026-10-29)

**Was passiert ist (drei Sätze):** In S162 lief die Suche nach SP2 a zum ersten Mal über alle 30 Tasten von
*Salamander* (232 Stützpunkte in neun Kurven, Levenberg–Marquardt, 93 min, zwei Startpunkte). Sie senkte den Rest um
rund drei Viertel (Σ r² 42 854 → 11 290, Treffer 128 → 149 von 456), aber beide Startpunkte trafen gemeinsam nur
71 von 456 Messgrößen; die Probe 7.5.11 nach ZF8 a / SP3 a ist damit verfehlt. S163 hat die Ursachen nachgerechnet
(Berichte `docs/modellierung/hammer-saite/proben/S162_suche/`, `S163_schnitt/`, `S163_taste/`, `S163_taste_delta/`
in LumiViz) und daraus den Plan A–D für einen zweiten, vorläufig letzten Anlauf gemacht (Entscheid Patrik V2 a, S163).
**Belegkraft:** alle Zahlen aus den genannten Berichten, von der Sitzung S163 selbst gerechnet; Physik nach dem
Rechenkern `HammerTon.cpp` (Kopie unter `gemeinsam/vorbild-lumiviz/code/loop/`).

### L8 — Zerfällt die Aufgabe in unabhängige Teile, dann je Teil eigenes λ und eigene Annahme

Jeder Stützpunkt bewegt nur den Ton seiner Note (bitgleich belegt, 7.5.10). Die Suche ist damit die Summe von 30
unabhängigen Aufgaben — aber ein gemeinsames λ und eine gemeinsame Annahme des Schritts (Rest der Summe sinkt) koppeln
sie künstlich: in den angenommenen Schritten von Startpunkt 1 verschlechterten sich bis zu 18 Tasten zugleich
(Iteration 2: 13 Tasten um 5136 schlechter gegen 7882 Gewinn anderswo). Der beste Stand je Taste über beide Startpunkte
läge, ohne eine neue Rechnung, bei Σ r² 9086 und 204 Treffern statt 11 290 und 149. Levenberg–Marquardt je Taste
allein war an vier von sechs Tasten deutlich besser (60: 149 statt 366), kostet aber dieselben Auswertungen, wenn alle
Tasten gemeinsam auf dem Pool gerechnet werden.
→ **Setz-Werkzeug jeder Klasse:** je Note (oder je unabhängiger Gruppe) eigener Zustand des Lösers — λ, Annahme, Ende —,
die Auswertung aller Noten zusammen in einem Pool-Aufruf. Die Entkopplung vorher am Modell belegen (Jacobi je Gruppe
gegen einzeln, wie 7.5.10).

### L9 — Parameter ohne Wirkung wandern; sie vor der Suche finden und festhalten

Unter der Marquardt-Skalierung λ·diag(JᵀJ) wird ein Parameter mit Diagonale nahe null kaum gedämpft und macht große
Schritte: Stützpunkte mit Wirkung unter 0,1 Toleranzen über ihren Bereich sprangen in 11 % ihrer Schritte um mehr als
10 % des Bereichs, oft von Rand zu Rand; bei Wirkung über 100 waren es 2 %. Folgen: 24 von 71 (Start 1) bzw. 30 von
90 (Start 2) der Stützpunkte „an der Grenze“ waren solche Wanderer (der Grenzbefund nach ZF8 a (4) ist davon
verseucht), und das Ende „Schritte klein“ konnte erst eintreten, als λ auf 1000 gestiegen war — Dämpfung, nicht
Konvergenz. In S162 waren 52 von 232 Stützpunkten ohne Wirkung (γ und h im Chor, s. L10).
→ **Setz-Werkzeug:** vor dem ersten Lauf je Parameter die Wirkung messen (√diag · Bereich, in Toleranzen; oder ein
Schnitt mit 15 Punkten, L14); Parameter unter einer Schwelle bekommen keinen Schritt und werden im Bericht als „ohne
Wirkung“ geführt, nicht als „an der Grenze“. Das Ende-Kriterium zählt nur wirksame Parameter.

### L10 — Die Physik vollständig einschalten, bevor gesetzt wird

Im Flügelmodell steht die Verstimmung der zweiten Schwingungsebene δ im Chor fest auf 0 (SP2 a: „δ nur an Tasten mit
einer Saite“, aus Vorsicht vor einem zu reichen Modell). Bei δ = 0 bleibt der senkrechte Unterraum der Ebenen unter
Stegdämpfung und Hammer exakt invariant: γ (Drehung der Ebene) und h (waagrechte Stegdämpfung) sind dann an 26 von
30 Tasten mathematisch ohne Wirkung — der Mechanismus, der nach Weinreich den langen Nachklang erzeugt (Energie
wandert in die schwach gedämpfte waagrechte Ebene), war an allen Chortasten aus. Mit δ auch im Chor frei (Schalter
`deltaImChor`, S163) wirken γ und h (10 bis 127 Toleranzen), und die Suche je Taste trifft an vier von sechs Tasten
deutlich mehr (zusammen 53 statt 44 von 104; an 84 15 von 18), mit Winkeln 18–42° und h um 0,07.
→ **Modellentwurf jeder Klasse:** jeder Mechanismus, der im Bezug hörbar ist (zweite Stufe, Schwebung, Register), muss
im Modell **eingeschaltet** und mit einem freien Parameter versehen sein, bevor gesetzt wird; „zu reich“ ist ein
Befund der Jacobi-Matrix (nicht aufgelöst), keine Vorab-Setzung. Was fest auf 0 steht, kann die Suche nicht finden.

### L11 — Entartete Moden machen Eigenlöser und finite Differenzen unbrauchbar

Bei δ = 0 und h = 1 (σ_H = σ_V) fallen senkrechte und waagrechte Mode einer Saite auf denselben Eigenwert. Der kleine
Eigenlöser lässt dann das Nachschärfen aus (`zerlegeKlein`: Abstand der Eigenwerte unter 10⁻⁸), die Eigenvektoren des
Paars sind beliebig, die Linkseigenvektoren schlecht konditioniert, und die Residuen der ganzen Taste rauschen: an
Taste 84 zeigte die Jacobi-Matrix für γ bis 98 Toleranzen „Wirkung“ — in der Iteration davor 0, danach wieder 0 —,
nachdem h als Wanderer (L9) auf 1 gesprungen war. Der Schnitt bestätigt es: Σ r² über h ist konstant (18,514) bis zum
letzten Punkt h = 1 (18,741). Das Rauschen stört die Suche an jeder Taste, an der h die 1 erreicht.
→ **Rechenkern jeder Klasse:** ein modaler Kern mit gekoppelten Schwingern braucht einen Eigenlöser, der Entartung
erkennt und meldet; das Modell vermeidet exakte Entartung durch Grenzen (δ-Untergrenze 0,01 Cent) oder behandelt den
entarteten Fall geschlossen. **Setz-Werkzeug:** verrauschte Spalten der Jacobi-Matrix (Wirkung springt zwischen
Iterationen um Größenordnungen) als Warnung melden.

### L12 — Ein welliges Ziel macht die Suche nicht eindeutig; wellige Größen gestaffelt setzen

Die Schwebung als **Anteil bei der festen Frequenz von *Salamander*** (ZF3 a) ist in `detune` wellig: läuft die
Schwebungsfrequenz des Modells an der Messfrequenz vorbei, oszilliert der Anteil. Der Schnitt (L14) zeigt an 6 von 6
Tasten 2 bis 4 Minima und 9 bis 15 Residuen mit Kante entlang `detune`; zwei Startpunkte enden deshalb in
verschiedenen Wellen, und die Glätte nach ZF8 a (2) ist verfehlt. Die Minima liegen nahe der gemessenen Schwebung
(0,3 bis 2,3 Cent).
→ **Setz-Werkzeug:** Größen, deren Ziel in einem Parameter periodisch ist (Schwebung, Register, Phasenlagen), zuerst
eindimensional durch das Modell setzen (die Frequenz des Modells an die des Bezugs, ein 1D-Löser), dann die übrigen
Parameter mit LM; nicht alles zugleich. Das ist der Geist von SG3 a (durch das Modell hindurch), ohne die Welligkeit in
der Suche.

### L13 — Die Toleranz der Aufnahme ist nicht die Hörschwelle; das Urteil braucht beide

Die Toleranz nach ZF4 a misst, was die Aufnahme auflöst; an vielen Tasten ist das die Untergrenze UG1 a (Zeiten 3 %).
Das Modell rechnet den Körper eines Steinway D gegen die Aufnahme eines Yamaha C5 mit Raum und zwei Mikrofonen: die
Stegmobilität bei der Frequenz eines einzelnen Teiltons ist bei den zwei Instrumenten verschieden, und kein Setzen der
Saitenkurven gleicht das aus. Der beste bekannte Punkt je Taste liegt im Mittel bei r ≈ 3 (drei Toleranzen), die
großen Reste bei `t60_spaet_k`, `vorwaerts_t60_k`, `knick_tiefe_db_k` mit τ = 0,03. Mit der Schwelle „beide
Startpunkte an jeder Messgröße innerhalb der Toleranz“ (SP3 a) besteht deshalb keine Suche die Probe, auch eine
perfekte nicht — die Schwelle misst das Modell, nicht die Suche.
→ **Messplan jeder Klasse:** die Toleranz der Aufnahme bleibt das Gewicht in der Suche (ZF5 a); das Urteil „getroffen“
bekommt daneben eine **hörbare Schwelle** je Größe (Hörschwellen aus der Literatur: Abkling-/Nachhallzeit, Pegel,
Modulationsfrequenz — zu recherchieren, nicht zu schätzen); „eindeutig“ heißt: beide Startpunkte enden an jeder Größe
**gleich** (auf eine Toleranz), die Treffer je Teilton stehen im Bericht. Und: wenn der Bezug ein anderes Instrument
ist als der gerechnete Körper, sind die Zielgrößen die, die das Ohr trägt (Länge der Stufen je Band, Knicktiefe,
Schwebungsrate, Helligkeit), nicht die Teilton-für-Teilton-Treue. Beim Flügel: Plan B (Phase B in V2 a).

### L14 — Die Probe vor der Suche: Schnitt je Parameter, Wirkung je Parameter, Treffer als Maß

ZF8 a (2) verlangte einen Schnitt je Parameter mit 15 Punkten — in S162 wurde er nicht gefahren; in S163 kostete er an
sechs Tasten mit je acht Parametern 3 min und zeigte auf einen Blick: welche Parameter wellig sind (L12), welche ohne
Wirkung (L9, L10), wo die Messung Kanten hat (`highDecay` am Bandrand, u bei 0), und dass `highDecay` an 19 von 30
Tasten am Bensa-Band endet. Dazu: Σ r² wird von den großen r bestimmt — bei Startpunkt 2 fiel der Rest von 61 363 auf
16 541, die Treffer blieben bei 107 bis 116.
→ **Setz-Werkzeug:** vor jedem Lauf der Schnitt je Parameter an wenigen Noten (Werkzeug `LUMIVIZ_75_SCHNITT` als
Muster: je Parameter 15 Punkte über den Bereich, die übrigen am Punkt; Minima, Sprung gegen das Dreifache der
mittleren Nachbaränderung, Residuen mit Kante, Wechsel fehlt/da), und LM je Note allein an denselben Noten
(`LUMIVIZ_75_TASTE`); der Fortschritt wird an Treffern **und** Rest berichtet.

### L15 — Der Weg nach einer verfehlten Probe (Plan A–D, Entscheid Patrik V2 a, S163)

Ein vorher festgelegter Ausstieg (ZZ1 a: ein Anlauf) ist richtig — aber eine Probe, die an Fehlern der Umsetzung
scheitert (L8, L9, L11) oder an einer unerreichbaren Schwelle (L13), hat weder Modell noch Ziel beurteilt. Der Weg,
der die Physik vollständig macht, die Suche ehrlich und das Urteil hörbar, in dieser Reihenfolge: **A** die Suche in
Ordnung bringen (δ frei mit Untergrenze, je Note eigenes λ, Wanderer still, wellige Größe gestaffelt, drei Startpunkte
je Note); **B** das Urteil hörbar machen (Hörschwelle je Größe aus der Literatur, Eindeutigkeit als Gleichheit,
Treffer im Bericht); **C** das Modell nur bei systematischem Befund (Reste je Teilton gleichgerichtet → Dämpfung je
Teilton frei); **D** hören und setzen (Hörproben gegen den Bezug, Werte eintragen, nach Gehör nur, wo das Ohr
widerspricht). Aufwand beim Flügel: zwei bis drei Bausitzungen, zwei Rechenläufe über Nacht, zwei Hörrunden.
→ **Messplan jeder Klasse:** die Feinabstimmung als A–D planen, mit Schwellen vorher und genau einem zweiten Anlauf;
die Zahl der Anläufe steht im Plan, nicht im Nachhinein.

## 6. Nachtrag S164/S165 — Plan A–D am Flügel durchgeführt (LumiViz, 2026-10-30)

**Was geschah:** Phase B zuerst — Hörschwellen aus der Literatur recherchiert (LumiViz
`docs/modellierung/hammer-saite/recherche/2026-10-30_S164_hoerschwellen.md`), daraus HW1 a–HW4 a. Dann Phase A gebaut
(je Note eigenes λ, Wanderer still, δ überall frei ab 0,01 Cent, `detune` gestaffelt per 1D-Löser, drei Startpunkte)
und an sechs Tasten geprüft. Die Suche je Note arbeitet (bitgleich wie LM je Note allein; hörbar getroffen 60 statt 26
an der Vorgabe), aber die **Glätte war an 0 von 6 Tasten erfüllt — auf allen fünf geprüften Wegen.** Nach dem vorher
gesetzten Ausstieg gilt ZZ1 a (PV1 a): kein automatisches Setzen, das Suchergebnis ist Vorschlag für das Gehör.
Zahlen: LumiViz `Spielweisen_Konzept.md` §4.1, Absätze „7.5.12 gebaut (S165)“, „A4-Wege (S165)“, PV1 a.

### L16 — Ein Ziel „Schwebung als Anteil bei fester Frequenz“ ist in jedem Parameter wellig, der die Schwebung verschiebt (berichtigt L12)

L12 schlug vor, die wellige Größe (`detune`) zuerst eindimensional an die Schwebungsfrequenz zu setzen und dann
festzuhalten. Das scheitert, sobald **zwei Mechanismen** die Schwebung tragen: beim Flügel die Verstimmung der Saiten
eines Chors (`detune`) **und** die Aufspaltung der Polarisationen jeder Saite (δ). Die gute Lösung teilt die
Schwebung auf (an Taste 84: `detune` 0,53 Cent, δ 1,07 Cent); ein 1D-Löser für `detune` allein setzt es auf 1,48 Cent
und verschlechtert drei von sechs Tasten deutlich. Alle Varianten — 1D nur als Start, beide gemeinsam per 2D-Raster,
δ an der Untergrenze — bleiben wellig: δ ist an 0 von 6 Tasten glatt, freies `detune` ebenso, und der Abstand der
Schwebungsfrequenz steigt in der Suche wieder auf das Doppelte. Die Welligkeit sitzt **im Ziel**, nicht in der
Suche: der Anteil bei einer festen Messfrequenz oszilliert, sobald die Schwebung des Modells an ihr vorbeiläuft.
→ **Messplan jeder Klasse:** bevor ein Plan mit Glätte-Schwelle beschlossen wird, die Glätte des **Ziels** in jedem
Parameter prüfen, der eine Frequenz oder Rate verschiebt (Schwebung, Vibrato, Tremolo, Register). Ist es dort wellig,
die Messgröße ändern (Frequenz und Tiefe mit eigener Suche der Frequenz messen, oder die Form der Hüllkurve), nicht die
Suche. → **Setz-Werkzeug:** Mechanismen, die dieselbe Wahrnehmung tragen, nie einzeln staffeln.

### L17 — Hörschwellen: es gibt keinen Klavier-Hörversuch; die Saite ist der nächste Beleg

Keine Hörschwelle an Klaviertönen gefunden. Am nächsten: Tolonen & Järveläinen 2000 (physikalisches Modell einer
gezupften Saite, 2-s-Töne) — Abklingen hörbar kürzer ab Faktor 0,70–0,79, länger ab 1,43–1,50; die Schwelle wird
mit der Hördauer enger (0,6 s: oben bis 5,6). Schwebungstiefe nach Ewert 2002: 1,2 dB in 20 lg m. Für Knicktiefe,
einzelne Teiltöne, späte Stufe und langsame Schwebungsraten: keine Zahl. Gesetzt wurde der strengste Satz (HW1 a),
weil Klaviertöne länger klingen als die Versuchstöne.
→ **Messplan jeder Klasse:** Hörschwellen je Größe vor der Suche recherchieren (Bericht mit Fundstellen, selbst
nachlesen); wo keine Zahl existiert, die Übertragung ausweisen; eine kleine eigene ABX-Messung am echten Reiz ist der
einzige Beleg für das eigene Instrument und lohnt sich, bevor viele Läufe gegen eine übertragene Schwelle beurteilt
werden.

### L18 — Ein vorher gesetzter Ausstieg wird gehalten; was gemessen ist, wird Vorschlag für das Ohr

Nach dem Fehlschlag lag es nahe, das Glätte-Kriterium nachträglich in Frage zu stellen (L16 legt das nahe). Das wäre
das Verschieben des Ziels nach dem Ergebnis gewesen — genau das, was der Ausstieg verhindern soll. Gehalten wurde
ZZ1 a, aber das Gemessene nicht weggeworfen: die Ergebnisse der Suche an den Proben-Noten gehen als Hörproben
(Bezug | Vorgabe | Suche) an Patrik, der nach Gehör setzt. Ein Lauf, der an den Proben-Noten bitgleich dasselbe
rechnet (unabhängige Noten, L8), entfiel.
→ **Setz-Werkzeug:** Ergebnisse je Note mit „Vorschlag“ und „gesetzt“ getrennt führen; eine verfehlte Probe beendet
das automatische Setzen, nicht das Messen. → **Messplan jeder Klasse:** der Ausstieg nennt, was mit den gemessenen
Vorschlägen geschieht.
