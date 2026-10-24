# InstrumentBody — Rechenkern der Instrumentkörper

> **Version:** 0.13.0  
> **Datum:** 2026-10-24  
> **Typ:** CppModuleDoc  
> **Status:** im Bau (Stufe 31c; gebaut: 31c-1 Bauplan, 31c-2 Netz, 31c-3 Platte, 31c-4 Eigenlöser, 31c-5 Körper, 31c-6 Teil 1 Dämpfung je Werkstoff)  
> **Modul:** lumi::koerper (Lib **InstrumentBody**, header-only INTERFACE)  
> **Dateien:** Bauplan.hpp, Netz.hpp, NetzCdt.hpp (bindet CDT ein), DuenneMatrix.hpp, Platte.hpp, Eigenloeser.hpp, EigenloeserSpectra.hpp (bindet Eigen und Spectra ein), Umsetzungen.hpp (jede Umsetzung), Verformungsenergie.hpp, Auswertung.hpp, Mobilitaet.hpp, Antwort.hpp, Statistik.hpp, Koerper.hpp  
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
  aus), Auflager eingespannt (der Boden), gelenkig für achsparallele Prüfplatten oder frei (nur für Prüfungen der
  Auswertung); K und M als `DuenneMatrix` (oberes Dreieck, erst das Muster, dann die Werte) — das Format für den
  Eigenlöser (BM5-1 a). `elemente` hält je Dreieck seine Knoten (seit 31c-5b, für die Auswertung); `Plattenelement`
  gibt seine Ansatzfunktionen über `basis` heraus.
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
  Rest ‖Kφ − λMφ‖ / ‖λMφ‖, die Zahl der nachgeschärften Moden (`nachgeschaerft`, RS1 a), die Zeit, ein Fehlertext
  (leer = in Ordnung; „cancelled" nach `weiter` = false).
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
- **Nachschärfen (RS1 a, S151):** Die Toleranz von Spectra gilt für ν = 1/(λ − σ); eine Mode weit unter σ trägt im
  eigentlichen Problem einen um rund σ/λ größeren Rest (am steifen MAESSTRO-Flügel MP1: 3·10⁻⁶ bei 68 Hz). Liegt der
  Rest einer Mode des Stücks über der Grenze, wird sie inverse iteriert — Shift bei λ · (1 − `nachschaerfAbstand`,
  Vorgabe 10⁻⁶), höchstens `nachschaerfSchritte` (10) Schritte, die übrigen Formen des Stücks gesperrt, λ als
  Rayleigh-Quotient —, danach neu geordnet; erst wenn auch das nicht reicht, der Fehler. Eine Mode unter der Grenze
  bleibt, wie sie ist: ein Körper ohne solche Moden rechnet Bit für Bit wie vorher.
- **Alternativen (EG2 a, Zusatz), wenn als nötig erkannt:** b `SparseLU` mit Pivotsuche für die Moden (wenn Reste
  über der Grenze bleiben, die ein verschobener Shift nicht behebt), c ein Löser mit Superknoten (CHOLMOD, Pardiso)
  als zweite Umsetzung (wenn Speicher oder Zeit der Zerlegung am Steinway ein Fenster unbrauchbar machen).
- **Gemessen** (Release-clang, S148): Steinway D (Netz bis 5 kHz, k = 3, 568 515 Größen) — bis 300 Hz 16 Moden
  (gezählt 16), 1 Zerlegung, 68 s, Spitze 2,3 GB; bis 1 kHz 80 Moden (gezählt 80), 3 Zerlegungen zu je rund 17 s,
  Rest 1,1·10⁻⁷, 164 s, Spitze 2,8 GB; **bis 5 kHz 820 Moden (gezählt 820), 13 Zerlegungen (248 s), Rest 4,0·10⁻⁷,
  1259 s (21 min), Spitze 4,8 GB** (mit `InstrumentBodyTool`); erste Mode 38,2 Hz (eingespannt, ohne Saiten —
  Prüfstein erst 31c-6). `SimplicialLDLT` rechnet auf einem Kern. Kipppunkt c von EG2 a nicht erreicht.

### 5.3 Auswertung je Mode — `InstrumentBody/Auswertung.hpp` (31c-5b)

Was der Körper je Mode trägt (KB1 a), gerechnet, während die Moden kommen (`jeMode`): `Auswertung` bereitet einmal
vor, `werte(mode)` gibt ein `ModeImKoerper`.

- **Am Stegpunkt** der Wert φ(x) der Form (w, bezüglich M normiert, 1/√kg) über die Ansatzfunktionen des Elements, in
  dem der Punkt liegt (`abtastung`, `wertBei`); die modale Masse dort ist 1/φ², das Vorzeichen sagt, wie zwei
  Stegpunkte zueinander schwingen. Ein Punkt außerhalb des Netzes wird auf das nächste Element geklemmt (`imNetz`,
  `abstand`).
- **Dämpfung** modal (Chabassier, Joly & Chaigne 2013 Gl. 17): α(f) = a f² + b f + c in 1/s mit f in Hz, a = 2·10⁻⁵,
  b = 0,07, c = 0 nach RT-0425 Gl. 1; gespeichert als Verlustfaktor η = α/(2πf). Die Lesart in f (nicht in ω) ist
  ein Befund: nur so liegt η bei den gemessenen 1–3 % (Spielweisen_Konzept §4.1, „Befund S148 zur Dämpfung").
- **Abstrahlung** je Hörort nach Rayleigh, die Platte in einer Schallwand: G = ρa/(2π) ∫ φ e^{−iωR/c}/R dS bei der
  Frequenz der Mode, komplex, Einheit √kg/m² (der Druck ist p = q̈ · G); integriert über die Gauss-Punkte aller
  Elemente (k + 1 je Richtung), R von der Mittelfläche (z = 0) zum Hörort; Luft nach RT-0425 §2.2 (1,29 kg/m³,
  340 m/s).
- **Geprüft** (`test_LoopKoerperAuswertung`): Polynome bis zur Ordnung des Elements genau; der Kolben (Scheibe, die
  sich ganz bewegt) auf der Achse gegen die geschlossene Lösung, auch im Nahfeld, auf 0,2 %; die erste Mode der frei
  aufliegenden Prüfplatte gegen Navier — φ in der Mitte auf 0,08 %, das Verhältnis Viertel/Mitte auf 10⁻³, das
  Fernfeld auf 0,09 %.
- **Gemessen** (Release-clang, `InstrumentBodyTool`, Steinway D bis 5 kHz, k = 3): alle 88 Stegpunkte im Netz,
  Vorbereitung 0,06 s, Auswertung aller 820 Moden 17 s (innerhalb der Zeit des Lösers).

**Dämpfung je Werkstoff (31c-6, DM1 a)** — `InstrumentBody/Verformungsenergie.hpp`. `Steife` trägt den Werkstoff
(`werkstoff`, Platz in `Bauplan::materialien`; `steifeBei` setzt ihn), `Plattenelement::integrationsPunkte` gibt die
Punkte von `matrizen`. `Verformungsenergie` bereitet je Element J⁻¹ und |det J|, je Integrationspunkt D, S und
Werkstoff vor und gibt je Mode U_m = ½ ∫_m κᵀDκ + γᵀSγ (`jeWerkstoff`) bzw. die Anteile U_m/U (`anteile`) — an
denselben Punkten mit denselben Größen wie K, also 2 Σ U_m = φᵀKφ. Mit ihr bekommt `Auswertung` je Mode
`werkstoffAnteil` und den Verlustfaktor aus `verlustfaktorAusAnteilen`: η = η_Gesetz + Σ_{η_m>0} a_m (η_m − η_Gesetz) —
ein Werkstoff mit eigenem `verlustfaktor` trägt ihn, einer mit 0 das Gesetz; ohne eigene Werte Bit für Bit das
Gesetz. Verfahren der modalen Verformungsenergie (Johnson & Kienholz 1982, aus Claudes Wissen, nicht nachgelesen).
**Grenze:** für einen Werkstoff mit eigenem Wert fehlen Verluste an Rand und durch Abstrahlung (für Holz stecken sie im
Gesetz). **Geprüft** (`test_LoopKoerperAuswertung`, „31c-6"): die Mischung (ohne eigene Werte genau das Gesetz); die
Prüfplatte mit zwei Werkstoffen (links, rechts): 2 Σ U_m = ω² auf 10⁻⁶, Summe der Anteile 1, die erste Mode
0,4999 / 0,5001; der statistische Teil mit dem Mittel der Anteile; jeder Punkt des Steinway hat einen Werkstoff.

**Mobilität (31c-6, PS1 a)** — `InstrumentBody/Mobilitaet.hpp`. `mobilitaet(moden, j, f)`: Treiberpunkt-Mobilität
nach MAESSTRO Gl. 10, Y = jω Σ φ_n(A)² / (ω_n² + jη_nω_nω − ω²) (bezüglich M normiert); `mittlereMobilitaetDb` nach
Gl. 12 (geometrisches Mittel von |Y| über 25–5000 Hz in 1-Hz-Schritten, dB re 1 s/kg); `terzbaender` (IEC 61260,
Basis 10, über die Nummer gewählt), `bandMobilitaetDb` (geometrisch über Band und Stegpunkte), `bandDichte`. Das
Werkzeug schreibt damit den Bericht (`--report`). **Geprüft** (`test_LoopKoerperMobilitaet`): Einmassenschwinger
gegen die geschlossene Lösung; Satz vom Mittelwert an einer Aluminiumplatte (mittleres Re(Y) gegen 1/(8√(ρhD)) auf
1 %). `kDiskantAb` = 41 (S150, PF2 a): ab dieser Taste zählt ein Stegpunkt zum Diskant — Werkzeug und Tests lesen
dieselbe Zahl. **Prüfstein** (`test_LoopKoerperPruefstein`, S150): Prüffall gegen MAESSTRO Abb. 6 und 5 als Bericht
(PS3 a), Steinway gegen Messungen (PS1 a (3)) und EM1 a, Konvergenz k = 3 gegen k = 4 mit `LUMIVIZ_KOERPER_K4`
(KV1 a nach KV2 a).

**Antwort (31c-7, AW1 a)** — `InstrumentBody/Antwort.hpp`. `antwort(moden, j, h, f)`: Druck am Hörort h je Kraft am
Stegpunkt j, H(ω) = Σ φ_n(A) G_n (−ω²)/(ω_n² − ω² + jη_nω_nω), Pa/N; `antwortSpektrum(moden, j, h, rate, n)`: H an
den n/2 + 1 Frequenzpunkten einer FFT der Länge n (die Rückwandlung gibt die Koeffizienten des Filters je
Abtastwert); `antwortLaengeS`: bis die langsamste Mode 60 dB gefallen ist, höchstens 8 s. Die Zeitreihe und die
Faltung bildet die App (`loop/Boden`, über `loop/Fft`). **Geprüft** (`test_LoopKoerperKlang`): Einmassenschwinger
gegen die geschlossene Lösung.

### 5.4 Statistischer Teil — `InstrumentBody/Statistik.hpp` (31c-5c)

Oberhalb der gerechneten Moden (ST1 a): `schaetze` nimmt die Größen aus dem oberen Band der gerechneten Moden (die
obere Hälfte des Fensters, beim Steinway 2,5–5 kHz), `ziehe` zieht daraus mit festem Samen die Moden bis zur oberen
Grenze (Vorgabe 20 kHz).

- **Größen** (`StatistischerTeil`, das trägt die Datei): die Dichte n(f) als Gerade durch die Zählung in zehn Stücken
  des Bands, über das Ende fortgesetzt (eine fallende Gerade wird waagrecht); je Stegpunkt das quadratische Mittel der
  Formwerte; je Hörort das quadratische Mittel von |G| (dieselbe Leistung wie die gerechneten Moden); die Dämpfung nach
  demselben Gesetz (RT-0425 Gl. 1) — seit DM1 a mit dem Mittel der Anteile je Werkstoff im Band und denselben
  Verlustfaktoren je Werkstoff; der Samen.
- **Ziehung** wie Boutillon & Ege 2013: f_{ν+1} = f_ν + r/n(f_ν), r nach Rayleigh mit Mittel 1 (Gl. 23/24); je
  Stegpunkt ein Formwert normal verteilt mit seiner Streuung, unabhängig voneinander; je Hörort ein Gewicht mit seinem
  Betrag und gleichverteilter Phase.
- **Zufall** mit eigenem Generator (`Zufall`, SplitMix64) und eigenen Umrechnungen — dieselbe Folge auf jedem Compiler.
  Zwei Züge stehen nie in einem Ausdruck (die Reihenfolge der Auswertung legt C++ dort nicht fest).
- **Geprüft** (`test_LoopKoerperStatistik`): die Folge des Generators gegen ihren Bezug, Rayleigh und normal in Mittel und
  Varianz; die Ziehung (Zahl wie ∫n df, Abstände nach Rayleigh, Streuung je Stegpunkt, Betrag und Phase je Hörort,
  Dämpfung, gleicher Samen gleiche Moden); die Schätzung an gerechneten Moden mit bekannten Größen.
- **Gemessen** (Steinway D, Band 2,5–5 kHz mit 528 Moden): Dichte 0,238 /Hz bei 5 kHz, Anstieg 2,1·10⁻⁵ /Hz², 5933
  Moden bis 20 kHz.

### 5.5 Der Körper — `InstrumentBody/Koerper.hpp` (31c-5d)

`Koerper` ist, was die Datei `.lvinstbody` trägt: Titel, Name und **Abdruck des Bauplans** (SHA-256 über sein JSON,
gebildet in der App: `loop/Koerper` `bauplanAbdruck`), wie gerechnet wurde (`Rechnung`: `kRechenFassung`, Netzgeber,
Löser, Ordnung, Netz für welche Frequenz, Fenster, freie Größen, Zeit), die Stegpunkte (MIDI) und Hörorte in der
Reihenfolge der Werte, Luft, Dämpfung, die gerechneten Moden (`ModeImKoerper`) und der statistische Teil.

- **`kKoerperFassung`** — die Fassung der Datei (2 seit S149: `materials` mit `lossFactor`, je Mode
  `materialShares`, im statistischen Teil deren Mittel; Fassung 1 bleibt lesbar, ohne Anteile).
- **`kRechenFassung`** — die Fassung der Rechnung (2 seit S149, DM1 a). Hochzählen, wenn sich bei gleichem Bauplan ein anderer Körper
  ergäbe; dann meldet der Wächter (`test_LoopKoerperDatei`, „Waechter“) jeden mitgelieferten Körper als veraltet, bis
  `InstrumentBodyTool` ihn neu rechnet. Ebenso, wenn sich der Bauplan ändert (der Abdruck passt nicht mehr).
- **Datei** (`loop/Koerper`, Qt-JSON, in der App und im Werkzeug): jedes Feld immer geschrieben; Zahlen kommen genau
  zurück; der Samen steht als Text (64 Bit); eine Mode mit falscher Zahl an Werten macht die Datei ungültig.

### 5.6 Eine eigene Umsetzung dazu

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

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperDatei.cpp` — ohne Qt-Anwendung: die Datei `.lvinstbody`
(Hin- und Rückweg genau, Felder, Samen mit 64 Bit, Fehler), der Abdruck des Bauplans, der **Wächter** über jeden
mitgelieferten Körper (Abdruck, `kRechenFassung`, Stegpunkte und Hörorte wie im Bauplan, Moden aufsteigend, Anschluss
des statistischen Teils, Größe unter rund 10 MB) — liest vom Arbeitsordner aus.

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperStatistik.cpp` — ohne Qt-Anwendung, ohne Dateien: Zufall
(Bezugsfolge, Verteilungen), Ziehung, Schätzung (§5.4).

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperAuswertung.cpp` — ohne Qt-Anwendung: Dämpfung im
gemessenen Bereich, Abtastung (Polynome genau, an einem Knoten sein Wert, außerhalb geklemmt), Rayleigh am Kolben auf
der Achse, die erste Mode der Prüfplatte gegen Navier (Wert und Fernfeld), der Steinway D (jeder Stegpunkt im Netz).

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperEigen.cpp` — gegen die Schnittstelle, für jede
Umsetzung aus `eigenloeser()`: eine Kette aus linearen Elementen mit geschlossener Lösung (200 Moden von unten, 250
mitten im Spektrum: jede Mode, keine doppelt, aufsteigend, M-orthonormal auf 10⁻⁸, Rest unter 10⁻⁶), zwei gleiche
Ketten ohne Kopplung (jeder Eigenwert doppelt — beide kommen), die frei aufliegende Prüfplatte mit k = 3 gegen den
dichten Löser von Eigen (von unten und mitten im Spektrum), die Bezugswerte `werte/eigen_pruefplatte.txt` (20 Moden:
Frequenz auf 10⁻⁷, w an fünf Stützpunkten auf 10⁻⁵; `LUMIVIZ_EIGEN_WERTE_SCHREIBEN=1`), das Nachschärfen (RS1 a,
nur Spectra: die Prüfplatte und zwei gleiche Platten ohne Kopplung mit einer Grenze strenger, als Lanczos sie trifft),
Fehler und Abbruch;
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
| 0.10.0 | 2026-10-24 | Session 149, Stufe 31c-6 Teil 2: Mobilität (`Mobilitaet.hpp`: MAESSTRO Gl. 10 und 12, Terzbänder, Dichte je Band); Bericht im Werkzeug (`--report`). |
| 0.11.0 | 2026-10-24 | Session 150, Stufe 31c-6 Teil 3–5: `kDiskantAb` = 41 in `Mobilitaet.hpp` (PF2 a); Prüfstein `test_LoopKoerperPruefstein` (PS3 a, PS4 a, KV2 a). |
| 0.12.0 | 2026-10-24 | Session 150, Stufe 31c-7 Schritt 1: Antwort des Bodens (`Antwort.hpp`: H je Stegpunkt und Hörort, im Spektrum der FFT, Länge). |
| 0.13.0 | 2026-10-24 | Session 151, Stufe 31c-8: Nachschärfen im Eigenlöser (RS1 a, `EigenErgebnis::nachgeschaerft`); der Rest in der Fehlermeldung in Exponentenschreibweise. |
| 0.9.0 | 2026-10-24 | Session 149, Stufe 31c-6 Teil 1: Dämpfung je Werkstoff (DM1 a) — `Verformungsenergie.hpp`, `Steife::werkstoff`, `Plattenelement::integrationsPunkte`, `verlustfaktorAusAnteilen`, `ModeImKoerper::werkstoffAnteil`, im statistischen Teil das Mittel; `kKoerperFassung` 2, `kRechenFassung` 2. |
| 0.8.0 | 2026-10-24 | Session 148, Stufe 31c-5d: der Körper (`Koerper.hpp`: `Koerper`, `Rechnung`, `kKoerperFassung`, `kRechenFassung`), die Datei in `loop/Koerper`, der Wächter. |
| 0.7.0 | 2026-10-23 | Session 148, Stufe 31c-5c: statistischer Teil (`Statistik.hpp`: `StatistischerTeil`, `schaetze`, `ziehe`, `Zufall`; ST1 a). |
| 0.6.0 | 2026-10-23 | Session 148, Stufe 31c-5b: Auswertung je Mode (`Auswertung.hpp`: Wert an den Stegpunkten, Dämpfung nach RT-0425 Gl. 1, Abstrahlgewicht nach Rayleigh je Hörort); `PlattenSystem::elemente`, `Plattenelement::basis`, `Auflager::Frei`. |
| 0.5.0 | 2026-10-23 | Session 148, Stufe 31c-5a: Hörorte (`Hoerort`, BM6 a), `.lvinstbodyplan` Fassung 2 mit `listeningPoints`. |
| 0.4.0 | 2026-10-23 | Session 148, Stufe 31c-4: Eigenlöser — Schnittstelle `Eigenloeser` (EG1 a), Umsetzung mit Spectra 1.2.0 und `SimplicialLDLT` (EG2 a: Zählung nach Sylvester, Teilfenster, Deflation, Rest je Mode), `Umsetzungen.hpp` mit `netzgeber()` und `eigenloeser()`, Werkzeug `InstrumentBodyTool`. |
| 0.3.0 | 2026-10-23 | Session 147, Stufe 31c-3: Platte — Reissner-Mindlin, Zonen nach RT-0425, `Plattenelement` mit `LagrangeDreieck(k)`, `baueSystem` (eingespannt, gelenkig), `DuenneMatrix` (EL1 a). |
| 0.2.0 | 2026-10-23 | Session 147, Stufe 31c-2: Netz — Schnittstelle `Netzgeber`, Auftrag aus dem Bauplan (Ränder der Leisten, Kante nach MAESSTRO Gl. 1), Umsetzung mit CDT 2.0.1 (BM4 a). |
| 0.1.0 | 2026-10-23 | Session 147, Stufe 31c-1: Bauplan (Materialien orthotrop/isotrop, Platte mit Dickenraster, Leisten, Stegpunkte, Geometrie, Masse, Prüfung). |
