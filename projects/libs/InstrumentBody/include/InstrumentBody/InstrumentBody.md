# InstrumentBody — Rechenkern der Instrumentkörper

> **Version:** 0.5.0  
> **Datum:** 2026-10-23  
> **Typ:** CppModuleDoc  
> **Status:** im Bau (Stufe 31c; gebaut: 31c-1 Bauplan, 31c-2 Netz, 31c-3 Platte, 31c-4 Eigenlöser)  
> **Modul:** lumi::koerper (Lib **InstrumentBody**, header-only INTERFACE)  
> **Dateien:** Bauplan.hpp, Netz.hpp, NetzCdt.hpp (bindet CDT ein), DuenneMatrix.hpp, Platte.hpp, Eigenloeser.hpp, EigenloeserSpectra.hpp (bindet Eigen und Spectra ein), Umsetzungen.hpp (jede Umsetzung)  
> **Namespace:** lumi::koerper  
> **Abhängigkeiten:** kein Qt, keine App-Teile; CDT 2.0.1 nur in `NetzCdt.hpp` (hinter der Schnittstelle des Netzes); Eigen 3.4.1 und Spectra 1.2.0 nur in `EigenloeserSpectra.hpp` (hinter der Schnittstelle des Eigenlösers)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Bauplan](#2-bauplan)
3. [Netz](#3-netz)
4. [Platte](#4-platte)
5. [Eigenlöser](#5-eigenlöser)
6. [Tests](#6-tests)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Ein **Instrumentkörper** ist nach KB1 a eine Liste gedämpfter Moden mit zwei Ausgängen: der Nachgiebigkeit an
den Stegpunkten (sie wirkt auf die Saite) und der Abstrahlung. Diese Bibliothek rechnet ihn aus einem
**Bauplan** — Platte mit Umriss und Dicke, Rippen, Stege, Materialien —, einmal und offline (BM1 a, BM5 a).

| Teil | Ort | Qt |
|---|---|---|
| Bauplan (Daten, Geometrie, Prüfung) | `InstrumentBody/Bauplan.hpp` | nein |
| Netz, Platte, Eigenlöser, Abstrahlung, statistischer Teil | ab 31c-2 hier | nein |
| Datei `.lvinstbodyplan` lesen/schreiben | App, `loop/KoerperBauplan` | ja |
| Werkzeug Bauplan → Körper | `projects/exec/InstrumentBodyTool` (31c-4: bis zu den Moden; den Körper schreibt es ab 31c-5) | ja (kompiliert den Leser mit) |
| Körper anwenden (Klang) | App, ab 31c-7 | ja |

Steuerdokument: `projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1 (BM1 a bis BM5-1 a, PL31c a,
GB1 a). Recherchen: `projects/apps/LumiViz/docs/modellierung/hammer-saite/` und `…/koerper/`.

## 2. Bauplan

`lumi::koerper::Bauplan` — **Einheiten SI** (m, kg/m³, Pa), x nach rechts, y nach oben; der Ursprung legt der
Bauplan fest (Steinway D nach RT-0425: untere linke Ecke des Bodens).

- **Materialien** (`Material`): je Teil eines (GB1 a). `Orthotrop` (Holz: Ex in Faserrichtung, Ey quer, Gxy,
  Querschub Gxz/Gyz für Reissner-Mindlin, ν_xy) oder `Isotrop` (Glas, Acryl: nur E und ν, der Rest über
  `vervollstaendigt`: G = E / (2 (1 + ν))). `verlustfaktor` η je Werkstoff, 0 = keine Angabe.
- **Platte**: Material, Faserwinkel (Grad gegen die x-Achse), geschlossener Umriss, Dicke — fest oder als
  `Raster` (bilinear; NaN = kein Wert, dann gilt die feste Dicke).
- **Leisten** (Rippen, Stege): Mittellinie, je Punkt Höhe über der Platte und Breite. Die Rechnung nimmt sie
  als dickere Stellen der Platte (BM1 a, wie MAESSTRO und Chabassier).
- **Stegpunkte**: je Taste (MIDI) der Ort, an dem die Saiten den Steg treffen, mit Quelle.
- **Hörorte** (`Hoerort`, seit Fassung 2; BM6 a): Name, Lage (x, y) und Höhe z über der Mittelfläche der Platte, mit
  Quelle. Je Hörort trägt der Körper ein Abstrahlgewicht je Mode (die Ausgänge nach KB1 a). Steinway D (BM6-1 a):
  `left` und `right` 12 cm über den Saiten an den Stegpunkten von MIDI 43 und 86 — die Oberseite des Stegs liegt eine
  halbe Plattendicke plus die Steghöhe über der Mittelfläche.
- **Prüfen**: `pruefe(plan)` nennt je Befund einen Satz (Material fehlt oder doppelt, Werte nicht positiv,
  Querzahl außerhalb der Stabilität — isotrop 0 ≤ ν < 0,5, orthotrop 1 − ν_xy·ν_yx > 0 —, Umriss ohne Fläche,
  Leiste ohne Höhe je Punkt, Stegpunkte nicht aufsteigend, Hörort ohne oder mit doppeltem Namen, nicht über der
  Platte).
- **Geometrie**: `flaeche`, `innen`, `abstandZurLinie`, `laenge`, `masse` (Platte über ein Raster von 5 mm
  summiert, Leisten als Länge mal mittlerer Querschnitt).

Die Datei `.lvinstbodyplan` (Format `lumiviz-instrument-body-plan`, Fassung 2) schreibt jedes Feld, auch leere;
NaN im Raster steht als `null`. Die Hörorte stehen in `listeningPoints` (`name`, `x`, `y`, `z`, `source`); Fassung 1
ohne diese Liste bleibt lesbar (dann keine Hörorte).

## 3. Netz

`InstrumentBody/Netz.hpp` (ohne fremde Bibliothek) und `InstrumentBody/NetzCdt.hpp` (die einzige Datei mit CDT).
Entschieden: BM4 a mit Zusatz (Patrik S147) — CDT jetzt, ein eigener Generator vielleicht später.

- **Schnittstelle** `Netzgeber::vernetze(NetzAuftrag) → Netz`. Hinein: geschlossener Umriss, offene
  Zwangslinien (dürfen sich kreuzen), Kantenlänge, kleinster Winkel (Vorgabe 25°). Heraus: Knoten, Dreiecke
  gegen den Uhrzeiger, Zwangskanten (sortiert), `fehler` (leer = gut), `unverfeinert` (nur Auskunft). Kein Typ
  von CDT geht über sie hinaus; eine Umsetzung ist zustandslos — zwei Aufrufe, dasselbe Netz.
- **Aus dem Bauplan** `netzAuftrag(plan, fmax)`: der Umriss der Platte, je Rippe und Steg die beiden Ränder
  (Mittellinie ± halbe Breite, `versetzt`), am Umriss abgeschnitten (`imUmriss`); die Kante nach MAESSTRO Gl. 1,
  `elementKante` = λ_min / 4 an der dünnsten Stelle der Platte, λ = √(2π / f) · (D / (ρ h))^¼, D = E_y h³ / 12.
- **Prüfgrößen:** `flaeche`, `kleinsterWinkelGrad`, `laengsteKante`, `groessteFlaeche(kante)` (gleichseitig),
  `aufZwangskanten(netz, a, b)`.
- **Umsetzung CDT 2.0.1** (`NetzgeberCdt`, `externals/CDT`, MPL-2.0): Punkte gleicher Lage zusammengelegt,
  Einfügen in gegebener Reihenfolge, sich kreuzende Zwangskanten aufgelöst (`TryResolve`), die äußeren Dreiecke
  gesammelt, zweimal nach größter Fläche und nach kleinstem Winkel verfeinert, dann entfernt. Fehler von CDT
  (Ausnahmen) als Text.
- **Eine eigene Umsetzung dazu:** eine Klasse, die `Netzgeber` umsetzt, in `netzgeber()` von `Umsetzungen.hpp`
  eintragen — alle Fälle von `test_LoopKoerperNetz.cpp` laufen dann für beide, und `InstrumentBodyTool --mesher`
  wählt sie; der Fall Steinway vergleicht mit `werte/netz_steinway.txt` (Knoten und Dreiecke auf 2 %, kleinster
  Winkel bis 1° darunter).
- **Gemessen** (Release-clang, S147): Steinway D bis 5 kHz — Kante 13,3 mm, 21 491 Knoten, 42 329 Dreiecke,
  kleinster Winkel 25,0° außer 8 Ecken der Eingabe (zwei Zwangskanten schließen den Winkel ein: Kreuzungen von
  Rippe und Steg), Fläche genau, jede Zwangslinie als Kantenfolge, 0,07 s.

## 4. Platte

`InstrumentBody/Platte.hpp` und `InstrumentBody/DuenneMatrix.hpp` (ohne fremde Bibliothek). Entschieden: BM1 a,
EL1 a (Patrik S147).

- **Modell:** Reissner-Mindlin, heterogen und orthotrop wie Chabassier 2013 und MAESSTRO. Größen je Punkt w, φx, φy;
  κ = (φx,x; φy,y; φx,y + φy,x), γ = (w,x − φx; w,y − φy); D = h³/12 · Q̄, S = κs h Ḡ (κs = 5/6); Masse ρh für w,
  ρh³/12 für die Drehungen.
- **Werkstoff am Punkt** (`steifeBei`): auf einem Steg dessen Material mit der Faser längs des Stegs, auf einer Rippe
  ebenso, sonst die Platte mit ihrer Faser (RT-0425 §2.1, die Zonen); Dicke = Platte + Höhe der Leiste. `gedreht`
  dreht Q̄ und Ḡ nach R. M. Jones.
- **Element** hinter der Schnittstelle `Plattenelement` (EL1 a): `LagrangeDreieck(k)`, k = 1 … 6, Knoten Ecken –
  Kanten – Inneres, volle Integration (Gauss auf dem zusammengeklappten Quadrat, k + 2 Punkte je Richtung).
- **System** (`baueSystem`): Knoten über die Kanten des Netzes durchnummeriert (Kantenknoten vom kleineren Eckknoten
  aus), Auflager eingespannt (der Boden) oder gelenkig für achsparallele Prüfplatten; K und M als `DuenneMatrix`
  (oberes Dreieck, erst das Muster, dann die Werte) — das Format für den Eigenlöser (BM5-1 a).
- **Gemessen** (Release-clang, S147): Prüfplatten nach Navier (Mindlin, frei aufliegend, orthotrop), Netz nach
  MAESSTRO Gl. 1 — k = 3 auf 0,04 % (8 mm), 0,03 % (2 mm, Dicke/Kante 0,084), 0,07 % (1 mm, 0,042); Locking bei k = 1
  (108–871 %) und k = 2 (2,5–10 %) deutlich, bei k = 3 nicht (EL1 a: Kipppunkt 1 % nicht erreicht). Steinway D bis
  5 kHz, k = 3: 568 515 freie Größen, 14,7 Mio. Einträge je Matrix, 4,7 s, Spitze 407 MB; Masse 11,77 kg (Bauplan
  11,48 kg: unter dem Steg rechnet die Zone Buche statt Fichte).

## 5. Eigenlöser

Gebaut 31c-4 (S148). Entschieden: BM5-1 a mit Zusatz (S147), EG1 a, EG2 a mit Zusatz (S148).

### 5.1 Schnittstelle — `InstrumentBody/Eigenloeser.hpp`

K φ = λ M φ mit λ = ω² (`eigenwertVon`, `frequenzVon`). Kein Typ von Eigen oder Spectra geht über die Schnittstelle.

- **`EigenAuftrag`:** K und M als `DuenneMatrix` (symmetrisch, M positiv definit; ab `vonHz` = 0 auch K positiv
  definit), das Fenster **[vonHz, bisHz)**, `toleranz` (relativ, auf den Eigenwert), der Empfänger `jeMode`
  (aufsteigend; leer = nur Frequenzen) und `weiter` (Fortschritt 0 … 1, false = abbrechen).
- **`Mode`:** Frequenz und Form, bezüglich M normiert (φᵀMφ = 1), nummeriert wie K und M (die Platte: nach
  `PlattenSystem::frei`). Das Vorzeichen ist frei.
- **`EigenErgebnis`:** die Frequenzen aufsteigend, **`imFenster`** (Zahl der Eigenwerte im Fenster nach dem
  Trägheitssatz von Sylvester — Pflicht jeder Umsetzung, EG1 a), die Zahl der Zerlegungen und ihre Zeit, der größte
  Rest ‖Kφ − λMφ‖ / ‖λMφ‖, die Zeit, ein Fehlertext (leer = in Ordnung; „cancelled" nach `weiter` = false).
- **Der Löser teilt das Fenster selbst** (EG1 a): Shifts, Teilfenster und doppelte Moden an ihren Grenzen sind seine
  Sache; er weiß nicht, was mit einer Form geschieht. Am Steinway bis 5 kHz wären alle Formen zusammen rund 11 GB.

### 5.2 Erste Umsetzung — `EigenloeserSpectra.hpp`

Spectra 1.2.0 (`SymGEigsShiftSolver`, Shift-Invert, Lanczos mit implizitem Neustart) und Eigen 3.4.1.

- **Zerlegung (EG2 a):** ein eigener Operator `LdltShiftInvert` zerlegt K − σM mit `SimplicialLDLT` (AMD-Ordnung;
  das Muster einmal analysiert — K und M haben dasselbe, sonst ein Fehler). Aus D zählt er die negativen Einträge:
  so viele Eigenwerte liegen unter σ. Liegt σ genau auf einem Eigenwert (Pivot 0), wird es um 10⁻⁹ verschoben.
- **Teilfenster:** von unten, je Schritt das Stück [P, σ) zwischen dem letzten Shift P und dem neuen; σ so gelegt,
  dass rund `modenJeSchritt` (Vorgabe 40) neue Eigenwerte darin liegen (aus der Dichte des letzten Stücks; zu viele
  laut Zählung: näher heran). Die Zählungen an P und σ sagen genau, wie viele es sind.
- **Deflation:** Spectra holt die nächsten um σ. Fehlen im Stück noch welche, läuft es noch einmal, die gefundenen
  Formen gesperrt: der Operator rechnet P (K − σM)⁻¹ mit P = I − Φ(MΦ)ᵀ, der Startvektor (fest, auf jedem Compiler
  gleich) ohne sie. So kommen auch beide Formen eines doppelten Eigenwerts — ein Lanczos mit einem Startvektor sähe
  nur eine (Fall „Eigen doppelt").
- **Geliefert wird ein Stück erst, wenn die Zahl stimmt;** was über σ schon gefunden ist, wartet (gesperrt) auf das
  nächste. Am Ende muss die Summe `imFenster` treffen.
- **Rest je Mode (EG2 a):** über `restGrenze` (Vorgabe 10⁻⁶) ein Fehler statt einer stillen Mode — `SimplicialLDLT`
  sucht keinen Pivot.
- **Alternativen (EG2 a, Zusatz), wenn als nötig erkannt:** b `SparseLU` mit Pivotsuche für die Moden (wenn Reste
  über der Grenze bleiben, die ein verschobener Shift nicht behebt), c ein Löser mit Superknoten (CHOLMOD, Pardiso)
  als zweite Umsetzung (wenn Speicher oder Zeit der Zerlegung am Steinway ein Fenster unbrauchbar machen).
- **Gemessen** (Release-clang, S148): Steinway D (Netz bis 5 kHz, k = 3, 568 515 Größen) — bis 300 Hz 16 Moden
  (gezählt 16), 1 Zerlegung, 68 s, Spitze 2,3 GB; bis 1 kHz 80 Moden (gezählt 80), 3 Zerlegungen zu je rund 17 s,
  Rest 1,1·10⁻⁷, 164 s, Spitze 2,8 GB; **bis 5 kHz 820 Moden (gezählt 820), 13 Zerlegungen (248 s), Rest 4,0·10⁻⁷,
  1259 s (21 min), Spitze 4,8 GB** (mit `InstrumentBodyTool`); erste Mode 38,2 Hz (eingespannt, ohne Saiten —
  Prüfstein erst 31c-6). `SimplicialLDLT` rechnet auf einem Kern. Kipppunkt c von EG2 a nicht erreicht.

### 5.3 Eine eigene Umsetzung dazu

Eine Klasse, die `Eigenloeser` umsetzt, in `eigenloeser()` von `Umsetzungen.hpp` eintragen: alle Fälle von
`test_LoopKoerperEigen.cpp` laufen dann auch für sie, `InstrumentBodyTool --solver <Name>` wählt sie. Ihre
Bezugswerte prüft der Fall „Eigen Bezugswerte" (`werte/eigen_pruefplatte.txt`). Der **Vergleichsfall alt gegen neu**
(Frequenzen auf Toleranz, Formen über den MAC-Wert, Rechenzeit beider) kommt mit der zweiten Umsetzung — mit einer
gibt es nichts zu vergleichen.

## 6. Tests

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperPlatte.cpp` — ohne Qt-Anwendung; die Frequenzen der
Prüfplatten rechnet dort der dichte Löser von Eigen (nur im Test): Ansatz (Kronecker, Zerlegung der Eins), Integration
(Monome genau), Werkstoff gedreht, Navier mit Konvergenz in k, Locking an 2 mm und 1 mm, Drehung (Netz und Faser um
30° gedreht: dieselben Frequenzen), Steinway D grob (bis 1 kHz, k = 2: Masse gegen den Bauplan);
`LUMIVIZ_PLATTE_MESSUNG=1` baut ihn bis 5 kHz mit k = 3 und meldet Größe und Zeit.

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperEigen.cpp` — gegen die Schnittstelle, für jede
Umsetzung aus `eigenloeser()`: eine Kette aus linearen Elementen mit geschlossener Lösung (200 Moden von unten, 250
mitten im Spektrum: jede Mode, keine doppelt, aufsteigend, M-orthonormal auf 10⁻⁸, Rest unter 10⁻⁶), zwei gleiche
Ketten ohne Kopplung (jeder Eigenwert doppelt — beide kommen), die frei aufliegende Prüfplatte mit k = 3 gegen den
dichten Löser von Eigen (von unten und mitten im Spektrum), die Bezugswerte `werte/eigen_pruefplatte.txt` (20 Moden:
Frequenz auf 10⁻⁷, w an fünf Stützpunkten auf 10⁻⁵; `LUMIVIZ_EIGEN_WERTE_SCHREIBEN=1`), Fehler und Abbruch;
`LUMIVIZ_EIGEN_MESSUNG=<Hz>` rechnet den Steinway D bis dorthin und meldet Zahl, Zeit, Zerlegungen, Rest und Spitze
des Speichers.

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperNetz.cpp` — gegen die Schnittstelle, für jede
Umsetzung aus `netzgeber()` (`Umsetzungen.hpp`): Elementgröße nach Gl. 1, Quadrat mit zwei sich kreuzenden Zwangslinien (Fläche,
Zwangslinien, Winkel, Fläche je Dreieck, Umlaufsinn, im Umriss, zwei Läufe gleich), ein Auftrag ohne Kante (Fehler),
der Steinway D bis 5 kHz mit den Bezugswerten `werte/netz_steinway.txt` (`LUMIVIZ_NETZ_WERTE_SCHREIBEN=1`).


`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperBauplan.cpp` — ohne Qt-Anwendung: Geometrie,
Dickenraster, Masse und Werkstoffe (auch Glas), `pruefe`, die Datei (Hin- und Rückweg, jedes Feld, NaN als null,
falsches Format, zu neue Fassung).

## 7. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.5.0 | 2026-10-23 | Session 148, Stufe 31c-5a: Hörorte (`Hoerort`, BM6 a), `.lvinstbodyplan` Fassung 2 mit `listeningPoints`. |
| 0.4.0 | 2026-10-23 | Session 148, Stufe 31c-4: Eigenlöser — Schnittstelle `Eigenloeser` (EG1 a), Umsetzung mit Spectra 1.2.0 und `SimplicialLDLT` (EG2 a: Zählung nach Sylvester, Teilfenster, Deflation, Rest je Mode), `Umsetzungen.hpp` mit `netzgeber()` und `eigenloeser()`, Werkzeug `InstrumentBodyTool`. |
| 0.3.0 | 2026-10-23 | Session 147, Stufe 31c-3: Platte — Reissner-Mindlin, Zonen nach RT-0425, `Plattenelement` mit `LagrangeDreieck(k)`, `baueSystem` (eingespannt, gelenkig), `DuenneMatrix` (EL1 a). |
| 0.2.0 | 2026-10-23 | Session 147, Stufe 31c-2: Netz — Schnittstelle `Netzgeber`, Auftrag aus dem Bauplan (Ränder der Leisten, Kante nach MAESSTRO Gl. 1), Umsetzung mit CDT 2.0.1 (BM4 a). |
| 0.1.0 | 2026-10-23 | Session 147, Stufe 31c-1: Bauplan (Materialien orthotrop/isotrop, Platte mit Dickenraster, Leisten, Stegpunkte, Geometrie, Masse, Prüfung). |
