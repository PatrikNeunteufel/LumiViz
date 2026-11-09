# Vorbild: wie der Flügel in LumiViz erarbeitet wurde

> **Stand:** 2026-10-26 (LumiViz S154). Die Quellen dazu liegen in LumiViz (nur lesen, nichts ändern):
> `projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4 (Entscheide, Befunde, fester Plan 31d–31h) und
> `projects/apps/LumiViz/docs/modellierung/hammer-saite/` (README, `modell.md`, `messplan.md`, `recherche/`, Quellenliste).
> **Nachgetragen 2026-10-27 (IC Session 3, Auftrag Patrik):** die Lehren L1–L7 aus dem Setzen (LumiViz S154–S157,
> Input [LEHREN_SETZEN_FLUEGEL.md](LEHREN_SETZEN_FLUEGEL.md)); die Entscheide ZF1 a, ZF3 a, ZF4 a, SP1 a–SP3 a in
> `Spielweisen_Konzept.md` §4.1 von dieser Sitzung im Wortlaut nachgelesen.
> **Nachgetragen 2026-10-28 (IC Session 3, nach Abklärungen in LumiViz):** **RF1 a** (S157) — zuerst die Grundlagen
> bis und mit 31h, danach die Feinabstimmung als **eigene, letzte Stufe 31i**; 31d-8 „Spielbar“ ist gebaut
> (`grand_piano.lvinst`, Hörproben `hoerproben_31d8`), das Hörurteil 8.1 steht aus (LumiViz-Handover, Stand S157, und
> Konzept §4.1 nachgelesen).

## Der Ablauf (Stufe 31)

| Teil | Inhalt | Was davon vorab gehen kann |
|---|---|---|
| Zielbild | Instrumente, was man hören muss, Spielweisen, Messen, Rechenzeit — fünf Fragen an Patrik | die Fragen je Klasse vorformulieren (`offene-fragen.md`) |
| Recherche Modell | Literatur zum Rechenverfahren (modal, Bank 2010), Saitendaten, Hammer | ganz |
| Recherche Körper | Geometrie und Holz, Mobilität, Abstrahlung, Numerik, Syntheseverfahren (sieben Berichte) | ganz |
| Freie Aufnahme | *Salamander* (Yamaha C5, CC BY 3.0): Lizenz an der Quelle geprüft, Inhalt geprüft, mit Freigabe geladen | Suche und Prüfung der Lizenz ganz, Laden mit Freigabe |
| Modell bauen | Saite, Hammer, Chor, Längskraft, Dämpfer, Körper | nur der Entwurf |
| **Spielbar** (L6; beim Flügel nachgezogen als 31d-8, SP1 a) | mitgeliefertes Instrument mit Vorgaben aus Quellen, Werte ohne Quelle nach Gehör innerhalb belegter Grenzen (SQ2 a); im Programm anspielbar, neben den anderen Instrumenten | je Größe Quellenwert und belegte Grenzen (`modell-entwurf.md`) |
| Grundlagen fertig (beim Flügel 31d-9 bis 31h, RF1 a) | Mitschwingen, Spielweisen, Editor/Creator, Abschluss — am spielbaren Instrument, mit Hörproben | nichts |
| **Feinabstimmung** (eigene, letzte Stufe; beim Flügel 31i, RF1 a) | Messung der Aufnahme, **Toleranz aus der Streuung der Aufnahme** (L1), Setzen durch das Modell hindurch **über die Stützpunkte der Kurven** (L2), Eindeutigkeit an den Messgrößen (L3), **Hörproben mit datiertem Urteil** (L5), dauerhafter Test | der Messplan: Toleranz-Rezept, gültiger Bereich, Hörproben, Dauer eines Setz-Laufs (L7) |

Die Zeile *Spielbar* stand beim Flügel nicht im Plan: das Instrument war nach fünf Sitzungen Messen und Setzen im
Programm nicht anspielbar, Klangfragen fielen an Kurvenbildern (L6). RF1 a geht weiter: die Feinabstimmung kommt
**nach** allem, was das Modell können soll (beim Flügel Mitschwingen und Una corda), weil sie an dem setzen soll, was
wirklich klingt; neu setzen kostet danach wenig, wenn Werkzeug, Wächter und Wertedateien stehen. Die Messkette selbst
(Teiltonmessung, Prüfgrößen) entsteht früher — sie dient vorher als Prüfstein, gesetzt wird mit ihr erst zuletzt.

## Lehren (für die Entwürfe)

- **Jeder Parameter wird an der Größe gesetzt, die er verursacht.** Setzt man gegen ein Modell, das die Messung nicht
  erreichen kann, findet der Ausgleich falsche Werte (31d-6, 31d-7: zwei Mal so — die zweite Schwingungsebene und der
  ungleiche Stoß des Hammers fehlten). → Im Entwurf je Größe nennen, woran sie gemessen wird, und prüfen, ob das Modell
  die beobachteten Erscheinungen überhaupt hervorbringen kann.
- **Bilder vor Zahlen:** neue Messgrößen erst am Bild der Hüllkurve glauben; mehrere plausibel aussehende Messfehler.
- **Eine Messgröße kann sättigen** (Tiefe der Einbrüche als Grundschwingung, S154) — im Messplan je Größe ihren
  gültigen Bereich angeben.
- **Freie Aufnahmen sind knapp** und bestimmen, was gemessen werden kann; die Suche früh, Lizenz an der Quelle.
- **Quellenangaben nur aus der Quelle selbst**; Zusammenfassungen von Werkzeugen sind kein Beleg (mehrere falsche
  Angaben in Wikipedia und Open Library gefunden).
- **Stufen vollständig planen**, Umfangsfragen vor dem Plan klären, Wachstum nur als Entscheid „Plan erweitern“.

## Lehren aus dem Setzen (LumiViz S154–S157)

Vom 25. bis 27.10. klang der Flügel unverändert; sieben Anläufe, die Größen automatisch an *Salamander* zu setzen,
scheiterten an denselben Mustern (Suche nicht eindeutig, Parameter an Grenzen, Struktur des Modells kippt an
Schwellen). Einzelheiten, Zahlen und Herkunft: [LEHREN_SETZEN_FLUEGEL.md](LEHREN_SETZEN_FLUEGEL.md).

- **L1 — „Gut genug“ steht vor dem Setzen fest, als Zahl aus der Aufnahme.** Die Toleranz je Messgröße ist die
  gemessene **Streuung des Bezugs selbst** — die größere aus Nachbarstufen derselben Note und den Kanälen derselben
  Stufe (ZF1 a, ZF4 a). Ein Modell kann der Aufnahme nicht näher kommen als ihre zwei Mikrofone einander (bei
  *Salamander* Kanäle 1,5–7,0 dB, Stufen 0,6–2,2 dB). Gibt ein Datensatz keine Streuung her (eine Stufe, ein Kanal),
  braucht es vor dem Setzen eine zweite Quelle.
- **L2 — Gesetzt werden Kurven über den Tonumfang, nicht Werte je Note.** Das Modell interpoliert Kurven aus
  Stützpunkten, also sucht der Ausgleich über die **Stützpunkte** aller Kurven, Ziel die Summe über alle Noten des
  Bezugs; je Note ihr Anteil als Anzeige für Ausreißer (SP2 a; beim Flügel rund 450 Messgrößen gegen rund 50
  Parameter). Die Zahl der Stützpunkte ist die Zahl der Parameter und folgt aus der Physik (Steg-, Saiten-,
  Registerwechsel).
- **L3 — Eindeutig ist eine Suche an den Messgrößen, nicht an den Parametern.** Zwei Startpunkte, die an jeder
  Messgröße innerhalb der Toleranz enden, haben denselben Klang gefunden; verschiedene Parameter heißen dann „von der
  Aufnahme nicht aufgelöst“ (SP3 a). Je Modellgröße gehört der **gültige Bereich** in den Messplan, in dem die
  Aufnahme sie überhaupt bestimmt (Abklingzeiten bis etwa zum Dreifachen der Aufnahmelänge, Schwebungen schneller als
  das Fenster, Tiefen über dem Boden); außerhalb wird die Größe aus einer Quelle oder nach Gehör gesetzt, nicht gesucht.
- **L4 — Wo das Modell seine Struktur an Schwellen wechselt, wird es an der Struktur des Bezugs gemessen.** Eine
  eigene Erkennung am Modell (zweite Stufe ja/nein, Schwebung ja/nein) macht die Zielfläche zur Treppe; die Erkennung
  läuft einmal am Bezug, das Modell wird an dessen Struktur gemessen und ist so von Bau aus stetig (ZF3 a). An jeder
  solchen Schwelle gehört ein Entscheid „gesucht oder gesetzt“ in den Entwurf.
- **L5 — Hören vor Rechnen.** Eine Vorgabe gilt erst als nicht brauchbar, wenn sie gehört wurde. Hörproben werden im
  selben Schritt abgelegt wie die Messung (gleich laut, Bezug | Modell vor | Modell nach, mit Frage je Datei), und das
  Urteil hat ein Datum, bevor der nächste Setz-Anlauf beginnt.
- **L6 — Spielbar vor Feinabstimmung, Grundlagen vor Feinabstimmung.** Gleich nach dem Bau ein mitgeliefertes
  Instrument mit Quellenwerten, Werte ohne Quelle nach Gehör innerhalb belegter Grenzen (SP1 a); danach alle Grundlagen
  bis zum Abschluss, und erst dann das Setzen an der Aufnahme als eigene, letzte Stufe mit denselben Werkzeugen
  (RF1 a; Patrik: „es muss einfach ganz klar notiert sein, was noch offen ist“ — eine Tabelle des Offenen, nachgeführt
  bei jedem Schritt). Im Aufwand je Klasse zwei Zahlen: „Grundlagen bis spielbar und fertig“ und „Feinabstimmung an der
  Aufnahme“ — die zweite war beim Flügel um Sitzungen zu klein geschätzt.
- **L7 — Was eine Probe kosten darf, steht vor dem ersten Lauf.** Rechenzeit **je Auswertung beim Setzen** neben der
  Rechenzeit je Ton beim Spielen; Zahl der Auswertungen je Suche und daraus die Dauer eines Setz-Laufs; Ergebnisdatei
  je Note bzw. je Iteration, damit ein Abbruch nicht alles kostet (SE1 a). Beim Flügel: eine Auswertung über 11 Stufen
  1,5 s, ein Setzen 30 Tasten × 2 Startpunkte rund 4 h; der Rechner steht unter Last an der Wärmegrenze, mehr als
  11 Threads werden langsamer.

**Nachgetragen 2026-10-29 (LumiViz S163):** die Suche über die Stützpunkte (SP2 a) ist in S162 gelaufen und in S163
geprüft (Probe 7.5.11 verfehlt; Entscheid Patrik V2 a: Plan A–D als zweiter, vorläufig letzter Anlauf). Einzelheiten
und Zahlen: [LEHREN_SETZEN_FLUEGEL.md](LEHREN_SETZEN_FLUEGEL.md) §5.

- **L8 — Unabhängige Teile, eigener Zustand je Teil.** Bewegt ein Stützpunkt nur seine Note, ist die Suche die Summe
  unabhängiger Aufgaben; ein gemeinsames λ und eine gemeinsame Annahme koppeln sie künstlich (bis 18 Tasten je Schritt
  schlechter; bester Stand je Taste 9086 und 204 Treffer statt 11 290 und 149). Je Note eigener Löserzustand, die
  Auswertung gemeinsam auf dem Pool.
- **L9 — Parameter ohne Wirkung wandern.** Unter λ·diag(JᵀJ) springen Parameter mit Diagonale nahe null über den
  ganzen Bereich (11 % ihrer Schritte über 10 % des Bereichs), verseuchen den Grenzbefund (24 von 71) und das
  Ende-Kriterium. Vor dem Lauf die Wirkung je Parameter messen, wirkungslose festhalten und als solche berichten.
- **L10 — Die Physik vollständig einschalten.** Was fest auf 0 steht (δ im Chor), kann die Suche nicht finden; γ und h
  waren dadurch an 26 von 30 Tasten exakt wirkungslos — der Mechanismus des Nachklangs nach Weinreich war aus. Mit δ
  frei trifft das Modell mehr (53 statt 44 von 104 an sechs Tasten). „Zu reich“ ist ein Befund der Jacobi-Matrix, keine
  Vorab-Setzung.
- **L11 — Entartete Moden.** Bei gleichen Eigenwerten (δ = 0, h = 1) fällt das Nachschärfen des Eigenlösers aus, die
  Residuen rauschen (Jacobi 98 Toleranzen aus nichts). Entartung erkennen und melden; durch Grenzen vermeiden.
- **L12 — Wellige Ziele gestaffelt setzen.** Die Schwebung als Anteil bei fester Frequenz ist in `detune` wellig
  (2–4 Minima, 9–15 Kanten an 6 von 6 Tasten) — kein Startpunkt-Paar endet gleich. Solche Größen zuerst eindimensional
  durch das Modell setzen, dann den Rest.
- **L13 — Toleranz ist nicht Hörschwelle.** Gegen ein anderes Instrument (Steinway-D-Körper, Yamaha-C5-Aufnahme) liegt
  das Modell im besten Punkt drei Toleranzen daneben; „jede Messgröße innerhalb der Toleranz“ misst das Modell, nicht
  die Suche. Toleranz als Gewicht, Hörschwelle (Literatur) als Urteil, Eindeutigkeit als Gleichheit der Startpunkte.
- **L14 — Die Probe vor der Suche.** Schnitt je Parameter (15 Punkte, 3 min für sechs Noten) und LM je Note allein
  zeigen vorher, was die Suche nicht kann; Fortschritt an Treffern **und** Rest.
- **L15 — Nach einer verfehlten Probe:** A Suche in Ordnung → B Urteil hörbar → C Modell nur bei systematischem Befund
  → D hören und setzen; genau ein zweiter Anlauf, Schwellen vorher.

**Nachgetragen 2026-10-30 (LumiViz S164/S165):** Plan A–D durchgeführt, Probe 7.5.12 an der Glätte verfehlt, Ausstieg
nach ZZ1 a (PV1 a). Einzelheiten: [LEHREN_SETZEN_FLUEGEL.md](LEHREN_SETZEN_FLUEGEL.md) §6.

- **L16 — Schwebung bei fester Frequenz ist wellig (berichtigt L12).** Tragen zwei Mechanismen dieselbe Schwebung
  (Chor-Verstimmung und Polarisation), scheitert jedes Staffeln; die Welligkeit sitzt im Ziel. Glätte des Ziels vor dem
  Plan prüfen, sonst die Messgröße ändern (Frequenz frei gesucht, Form der Hüllkurve).
- **L17 — Hörschwellen.** Kein Klavier-Hörversuch; Saite (Tolonen & Järveläinen 2000) Faktor 0,79 / 1,43, Schwebungstiefe
  1,2 dB in 20 lg m (Ewert 2002); eigene kleine ABX-Messung am echten Reiz vor vielen Läufen.
- **L18 — Ausstieg halten, Gemessenes als Vorschlag.** Eine verfehlte Probe beendet das automatische Setzen, nicht das
  Messen; Suchergebnisse gehen als Hörproben an das Ohr.
