# InstrumentBody — Rechenkern der Instrumentkörper

> **Version:** 0.25.0  
> **Datum:** 2026-11-20  
> **Typ:** CppModuleDoc  
> **Status:** im Bau (Stufe 31c; gebaut: 31c-1 Bauplan, 31c-2 Netz, 31c-3 Platte, 31c-4 Eigenlöser, 31c-5 Körper, 31c-6 Teil 1 Dämpfung je Werkstoff)  
> **Modul:** lumi::koerper (Lib **InstrumentBody**, header-only INTERFACE)  
> **Dateien:** Bauplan.hpp, Luftraum.hpp, Netz.hpp, NetzCdt.hpp (bindet CDT ein), DuenneMatrix.hpp, Platte.hpp, Eigenloeser.hpp, EigenloeserSpectra.hpp (bindet Eigen und Spectra ein), Umsetzungen.hpp (jede Umsetzung), Verformungsenergie.hpp, Auswertung.hpp, Mobilitaet.hpp, Antwort.hpp, Statistik.hpp, Koerper.hpp  
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
- **Löcher** (`Platte::loecher`, seit Fassung 4; KQ7 a, S196): je Loch ein geschlossener Innenumriss, ganz im Umriss,
  keines berührt ein anderes. Im Loch gibt es keine Platte: das Netz spart es aus, sein Rand ist frei (kein Auflager
  hält ihn), es strahlt nicht und wiegt nichts; Leisten enden am Lochrand. Die Rückseite (HU4 a) rechnet weiter nur
  über den Außenumriss — der Weg durch ein Loch gehört zum Luftraum (F-K1 a, noch nicht gebaut). Ein Bauplan ohne
  Löcher rechnet Bit für Bit wie vorher (belegt S196: die drei Steinway-Körper neu gerechnet, gleich bis auf Abdruck
  und Rechenzeit).
- **Leisten** (Rippen, Stege): Mittellinie, je Punkt Höhe über der Platte und Breite. Die Rechnung nimmt sie
  als dickere Stellen der Platte (BM1 a, wie MAESSTRO und Chabassier).
- **Leisten übereinander** (`Bauplan::leistenUebereinander`, `LeistenUebereinander`; seit Fassung 8, KQ31-1 a, S206):
  liegen an einem Punkt mehrere Leisten, zählt mit `NurDieErste` (die Vorgabe — die Rechnung bis Fassung 7) nur die
  erste des Bauplans, Stege vor Rippen; eine weitere dort ist verdeckt, ihre Höhe fehlt in Steife und Masse der
  Rechnung. Mit `Addieren` zählen die Höhen aller Leisten an dem Punkt zusammen, Werkstoff und Faserrichtung sind
  die der ersten — so liegen bei der Harfe Deck- und Zugleiste beidseits des Bodens auf derselben Linie (bis S205
  fehlten dort 0,41 kg Buche in der Rechnung). `steifeBei` rechnet danach; `ueberlagerungen(plan)` (in `Platte.hpp`)
  tastet jede Leiste ab (2 mm längs, 1 mm quer) und nennt je Paar die gemeinsame Fläche, das Volumen der unteren
  Leiste dort und ob beide denselben Werkstoff haben. `masse(plan)` zählt wie bisher jede Leiste nach ihren Maßen.
- **Stegpunkte**: je Taste (MIDI) der Ort, an dem die Saiten den Steg treffen, mit Quelle.
- **Hörorte** (`Hoerort`, seit Fassung 2; BM6 a): Name, Lage (x, y) und Höhe z über der Mittelfläche der Platte, mit
  Quelle. Je Hörort trägt der Körper ein Abstrahlgewicht je Mode (die Ausgänge nach KB1 a). Steinway D (BM6-1 a):
  `left` und `right` 12 cm über den Saiten an den Stegpunkten von MIDI 43 und 86 — die Oberseite des Stegs liegt eine
  halbe Plattendicke plus die Steghöhe über der Mittelfläche.
- **Rückseite** (`Rueckseite`, seit Fassung 3; HU4 a, S160): Randtiefe h in m (die Zarge unter der Platte; 0 = aus,
  dann die Platte in der Schallwand wie bis Fassung 2), Schattenfrequenz in Hz (0 = kein Schatten), Quelle. Mit ihr
  strahlt im Rayleigh-Integral auch die Unterseite, mit umgekehrtem Vorzeichen, über den nächsten Randpunkt
  (`naechsterRandpunkt`): Weg in der Platte zum Rand, 2h hinab und hinauf, dann zum Hörort; oberhalb der
  Schattenfrequenz mit 1/(1 + j f/f_s) abgeschattet (Beugung über den Rand). Bei tiefen Frequenzen löscht sie die
  fernen Anteile der Vorderseite aus — der akustische Kurzschluss, den die Schallwand nicht kennt (Befund HU4,
  Spielweisen_Konzept §4.1).
- **Dämpfung** (`Daempfung`, seit Fassung 3; HU4-1 a, S160): das Gesetz α(f) = a·f² + b·f + c in 1/s mit Quelle,
  Vorgabe RT-0425 Gl. 1 (a = 2·10⁻⁵, b = 0,07, c = 0); der Steinway D trägt c = 4,5 (siehe Auswertung).
- **Hohlraum** (`Hohlraum`, seit Fassung 4; F-K1 a, KQ8 a, S196): Volumen in m³ (0 = keiner — dann rechnet die Platte
  allein wie bis Fassung 3), Mündungskorrektur je Loch in Radien (Vorgabe 1,7 = 2 · 8/(3π), ein Loch in einer dünnen
  weiten Wand), Güte der Luft in den Löchern (Vorgabe 30), Quelle. Die Rechnung steht in §5.7.
  **Öffnungen außerhalb der Platte** (`Hohlraum::oeffnungen`, `Oeffnung`; seit Fassung 5, KQ17 a, S197) — etwa die
  Öffnungen in der Schale einer Harfe: je Öffnung Ort in der Ebene der Platte, Höhe über der Platte (negativ =
  dahinter), Fläche, Lippenlänge (die Länge durch die Wand, ohne Mündungskorrektur), Quelle. Die Platte ist dort
  nicht ausgespart. `luftraumGroessen` nimmt Löcher und Öffnungen parallel (Leitwert A / (Lippe bzw. Dicke +
  Mündung · Radius)); ein Loch der Platte strahlt in den Halbraum (ρ/2π), eine Öffnung frei von ihrem Ort (ρ/4π) —
  die Schale schattet in der Rechnung nicht ab. `pruefe`: Fläche positiv, Lippe nicht negativ, Öffnungen nur mit
  Volumen.
- **Rand** (`Rand`, `Bauplan::rand`; seit Fassung 6, KQ18 a, S198): der äußere Rand der Platte ist eingespannt
  (`Halt::Eingespannt`, die Vorgabe — wie bis Fassung 5) oder elastisch gehalten (`Halt::Elastisch`): w gehalten,
  gegen die Drehung um die Randlinie eine Drehfeder `drehfeder` in N (= N m/rad je m Rand), **ein Wert für den
  ganzen Rand**; 0 = nur gestützt. So liegt ein Boden, der zwischen Leisten auf einer nachgiebigen Schale sitzt
  (Harfe: Bell 1988 Abb. 2.6). Der Rand eines Lochs bleibt frei. `pruefe`: Feder nicht negativ, eine Feder nur mit
  dem Halt elastisch. Eine Wegfeder und ein Verlauf längs des Rands sind nicht im Format (Entscheid Patrik S198:
  die Messung an der Harfe trägt beides nicht).
- **Federn an Punkten** (`Punktfeder`, `Bauplan::punktfedern`; seit Fassung 9, KQ46 a, KQ49 a, S210): je Feder Ort,
  Steife in N/m gegen die Auslenkung w, Verlustfaktor und Quelle — etwa die Längssteife einer Saite an ihrer Öse
  (Harfe: der Körper ist die besaitete Harfe). Die Feder hält den Punkt gegen festen Grund und wiegt nichts.
  `setzePunktfedern(system, element, federn)` (in `Platte.hpp`) setzt je Feder k · N Nᵀ auf die w-Größen des
  Elements, in dem der Punkt liegt (N = die Ansatzwerte dort, über `abtastung`); `baueSystem(netz, element, plan)`
  ruft es, `baueSystemOhnePunktfedern` lässt die Federn weg. Ohne Federn ist K Bit für Bit wie vorher. **Dämpfung
  (KQ49 a):** der Anteil der Verformungsenergie einer Mode in den Federn trägt deren Verlustfaktor (0 = ohne
  Verlust), der Rest den der Platte — §5.3. **Grenze:** an einer schubweichen Platte wächst die Auslenkung unter
  einer Punktkraft mit feinerem Netz langsam, die Feder wird also etwas weicher; an der Prüfplatte des Tests
  0,05 % in der Frequenz zwischen 7 und 4,5 cm Kante. `pruefe`: Steife und Verlustfaktor nicht negativ, der Ort im
  Umriss und in keinem Loch.
- **Prüfen**: `pruefe(plan)` nennt je Befund einen Satz (Material fehlt oder doppelt, Werte nicht positiv,
  Querzahl außerhalb der Stabilität — isotrop 0 ≤ ν < 0,5, orthotrop 1 − ν_xy·ν_yx > 0 —, Umriss ohne Fläche,
  Leiste ohne Höhe je Punkt, Stegpunkte nicht aufsteigend, Hörort ohne oder mit doppeltem Namen, nicht über der
  Platte, Randtiefe oder Schattenfrequenz negativ, a, b oder c der Dämpfung negativ; Loch ohne Fläche, nicht ganz im
  Umriss, zwei Löcher berühren sich, Stegpunkt in einem Loch).
- **Geometrie**: `flaeche` (auch der Platte: Umriss ohne Löcher), `imLoch`, `kantenSchneidenSich`, `innen`, `abstandZurLinie`, `laenge`, `masse` (Platte über ein Raster von 5 mm
  summiert, Leisten als Länge mal mittlerer Querschnitt), `naechsterRandpunkt` (der nächste Punkt auf den Kanten
  des Umrisses und sein Abstand).

Die Datei `.lvinstbodyplan` (Format `lumiviz-instrument-body-plan`, Fassung 9) schreibt jedes Feld, auch leere;
`pointSprings` (je Feder `x`, `y`, `stiffness` in N/m, `lossFactor`, `source`; seit Fassung 9) trägt die Federn an
Punkten, Fassung 8 ohne die Liste liest sich ohne Federn;
`barOverlap` (`first` oder `add` — ein anderes Wort macht die Datei unlesbar; seit Fassung 8) sagt, wie Leisten
übereinander zählen, Fassung 7 ohne das Feld liest sich als `first`;
der Rand steht in `rim` (`support`: `clamped` oder `elastic` — ein anderes Wort macht die Datei unlesbar —,
`rotationalStiffness`, `source`); Fassung 5 ohne den Block bleibt lesbar (eingespannt);
die Löcher stehen in `plate.holes` (je Loch eine Liste von Punkten), der Hohlraum in `cavity` (`volume`,
`endCorrection`, `q`, `source`, seit Fassung 5 `openings` mit `x`, `y`, `z`, `area`, `lipLength`, `source` je
Öffnung); Fassung 4 ohne `openings` und Fassung 3 ohne Löcher und Hohlraum bleiben lesbar;
NaN im Raster steht als `null`. Die Hörorte stehen in `listeningPoints` (`name`, `x`, `y`, `z`, `source`), die
Rückseite in `backside` (`rimDepth`, `shadowHz`, `source`), die Dämpfung in `damping` (`a`, `b`, `c`, `law`,
`source`); Fassung 1 ohne die Liste und Fassung 2 ohne die Blöcke bleiben lesbar (dann keine Hörorte bzw. keine
Rückseite und RT-0425 mit c = 0).

## 3. Netz

`InstrumentBody/Netz.hpp` (ohne fremde Bibliothek) und `InstrumentBody/NetzCdt.hpp` (die einzige Datei mit CDT).
Entschieden: BM4 a mit Zusatz (Patrik S147) — CDT jetzt, ein eigener Generator vielleicht später.

- **Schnittstelle** `Netzgeber::vernetze(NetzAuftrag) → Netz`. Hinein: geschlossener Umriss, offene
  Zwangslinien (dürfen sich kreuzen), Kantenlänge, kleinster Winkel (Vorgabe 25°). Heraus: Knoten, Dreiecke
  gegen den Uhrzeiger, Zwangskanten (sortiert), `fehler` (leer = gut), `unverfeinert` (nur Auskunft). Kein Typ
  von CDT geht über sie hinaus; eine Umsetzung ist zustandslos — zwei Aufrufe, dasselbe Netz. Seit S196 (KQ7 a)
  dazu hinein `loecher` (geschlossene Innenumrisse: im Loch kein Dreieck) und heraus `lochkanten` (die Randkanten
  auf einem Lochrand, sortiert; `lochkantenVon` bestimmt sie für jede Umsetzung aus dem Netz); `netzAuftrag`
  schneidet die Ränder der Leisten auch an den Löchern ab (`imUmriss(…, aussenBleibt)`).
- **Aus dem Bauplan** `netzAuftrag(plan, fmax)`: der Umriss der Platte, je Rippe und Steg die beiden Ränder
  (Mittellinie ± halbe Breite, `versetzt`), am Umriss abgeschnitten (`imUmriss`); die Kante nach MAESSTRO Gl. 1,
  `elementKante` = λ_min / 4 an der dünnsten Stelle der Platte, λ = √(2π / f) · (D / (ρ h))^¼, D = E_y h³ / 12.
- **Prüfgrößen:** `flaeche`, `kleinsterWinkelGrad`, `laengsteKante`, `groessteFlaeche(kante)` (gleichseitig),
  `aufZwangskanten(netz, a, b)`.
- **Umsetzung CDT 2.0.1** (`NetzgeberCdt`, `externals/CDT`, MPL-2.0): Punkte gleicher Lage zusammengelegt,
  Einfügen in gegebener Reihenfolge, sich kreuzende Zwangskanten aufgelöst (`TryResolve`), die äußeren Dreiecke
  gesammelt — dazu die Dreiecke, deren Schwerpunkt in einem Loch liegt (der Lochrand ist Zwangskante) —, zweimal
  nach größter Fläche und nach kleinstem Winkel verfeinert, dann entfernt. Fehler von CDT
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
  Auswertung); eine Randkante aus `Netz::lochkanten` hält kein Auflager (KQ7 a); K und M als `DuenneMatrix` (oberes Dreieck, erst das Muster, dann die Werte) — das Format für den
  Eigenlöser (BM5-1 a). `elemente` hält je Dreieck seine Knoten (seit 31c-5b, für die Auswertung); `Plattenelement`
  gibt seine Ansatzfunktionen über `basis` heraus.
- **Drehfeder am Rand** (KQ18 a, S198): `baueSystem(…, auflager, drehfederAmRand)` nimmt je Punkt des äußeren Rands
  eine Feder k_d in N und legt ∫ k_d φ_n² ds in K (φ_n = φ_x n_x + φ_y n_y, n die Randnormale; je Randkante mit den
  Ansatzfunktionen des Elements, Gauss auf der Strecke mit k + 2 Punkten, `gaussAufStrecke`). Sie wirkt, wo das
  Auflager die Drehungen frei lässt: mit `Gestuetzt` ist 0 der gestützte Rand, eine sehr harte Feder nähert sich
  `Eingespannt`. `baueSystem(netz, element, plan)` baut die Platte eines Bauplans mit dessen Rand — das Werkzeug und
  die Tests an Bauplänen rufen diese Fassung. Geprüft gegen Lévy (Rechteck, Federn an zwei Rändern: 1 %), gegen beide
  Grenzfälle und gegen ein gedrehtes Netz (`test_LoopKoerperPlatte`, „KQ18 Drehfeder am Rand“, „KQ18 Rand im
  Bauplan“). Die Feder trägt keinen eigenen Verlust: die Anteile der Verformungsenergie je Werkstoff zählen sie nicht.
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
  Seit HU4-1 a (S160) kommt das Gesetz aus dem Bauplan (`Bauplan::daempfung`, Block `damping`); der Steinway D
  trägt c = 4,5 1/s, damit die tiefsten Moden die gemessenen Werte erreichen (η 3,0 % bei 38 Hz, 2,3 % bei 61 Hz,
  1,5 % bei 300 Hz bis 1 kHz; Ege, Boutillon & Rébillat 2013: 1–3 %, Mittel 2,3 %) — mit c = 0 lagen sie bei 1,1 %.
  Ein Bauplan ohne den Block rechnet mit c = 0 wie bisher.
- **Federn an Punkten** (KQ49 a, S210): nach `Auswertung::setzePunktfedern(system, element, federn)` trägt jede Mode
  `federAnteil` = Σ U_f / (U_Platte + Σ U_f) mit U_f = ½ k w(p)², und ihr Verlustfaktor ist
  η = (1 − Anteil) · η_Platte + Σ a_f η_f. U_Platte kommt aus der `Verformungsenergie`; ohne sie gilt ½ (2πf)²
  abzüglich der Federn (nur für eine bezüglich M normierte Form der Platte allein). `werkstoffAnteil` bleibt auf
  die Werkstoffe normiert; `federAnteil` steht nicht in der Körperdatei. Mit Luftraum mischt `mitLuftraum` danach
  wie bisher den Anteil der Luft dazu. Ohne Federn rechnet `werte` Bit für Bit wie vorher. `Abtastung`, `bezug`,
  `abtastung` und `wertBei` stehen seit S210 in `Platte.hpp`.
- **Abstrahlung** je Hörort nach Rayleigh, die Platte in einer Schallwand: G = ρa/(2π) ∫ φ e^{−iωR/c}/R dS bei der
  Frequenz der Mode, komplex, Einheit √kg/m² (der Druck ist p = q̈ · G); integriert über die Gauss-Punkte aller
  Elemente (k + 1 je Richtung), R von der Mittelfläche (z = 0) zum Hörort; Luft nach RT-0425 §2.2 (1,29 kg/m³,
  340 m/s). Mit der **Rückseite** des Bauplans (HU4 a) je Gauss-Punkt dazu −S(f) φ e^{−iωR'/c}/R' mit R' über den
  nächsten Randpunkt (Weg zum Rand + 2h + zum Hörort) und S = 1/(1 + j f/f_s); ohne Rückseite Bit für Bit wie bisher.
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
Abtastwert) — seit S151 schnell (AW3-1 a): die Summe auf einem Raster je 64 Punkte, kubisch interpoliert; eine
Resonanz schmaler als 16 Rasterschritte wird mit einem Kosinus-Gewicht geteilt, ihr Kern (32 Schritte plus zwei
Halbwertsbreiten, Übergang 16) exakt an den feinen Punkten, der Rest aufs Raster (`RasterAntwort`); gegen
`antwortSpektrumExakt` (jede Mode an jedem Punkt) −96 … −104 dB der Spitze, 14- bis 19-fach schneller; `antwortLaengeS`: bis die langsamste Mode 60 dB gefallen ist — seit S151 ohne Obergrenze (AW2 a: die
8 s schnitten einen Boden aus Glas ab, und was über die Periode der FFT reichte, faltete sich zurück). Die Zeitreihe
und die Faltung bildet die App (`loop/Boden`, über `loop/Fft`). **Geprüft** (`test_LoopKoerperKlang`):
Einmassenschwinger gegen die geschlossene Lösung; eine Mode, die 22 s klingt, über die ganze Länge und ohne Rückfaltung.

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
  `materialShares`, im statistischen Teil deren Mittel; Fassung 1 bleibt lesbar, ohne Anteile; **3 seit S196**
  (F-K1 a): je Mode `airShare` = `ModeImKoerper::luftAnteil`, der Anteil der Bewegungsenergie in der Luft der
  Löcher, immer geschrieben, 0 ohne Hohlraum — die Dämpfung einer gekoppelten Mode ist die der Platte mal ihrem
  Anteil plus Anteil der Luft / Güte, und der Wächter rechnet sie so nach; Fassung 2 bleibt lesbar, Anteil 0);
  **5 seit S206** (KQ35-2 a, KQ35-1 a): je Mode `bridgeAngle` = `ModeImKoerper::stegWinkelGrad`, der Winkel, um den
  die Mode den Steg gegen die Senkrechte der Decke bewegt (Grad, quer zu den Saiten; ±90 = nur parallel). `bridge`
  (`amStegpunkt`) ist der Formwert längs dieser Richtung: `senkrechtAm(i)` = φ · cos, `parallelAm(i)` = φ · sin; bei
  Winkel 0 ist `senkrechtAm` Bit für Bit `amStegpunkt`. `antwort`, `antwortSpektrum*` und `mobilitaet` rechnen mit dem
  senkrechten Formwert. Im statistischen Teil `bridgeAngle` (`stegWinkelGrad`: Betrag für die gezogenen Moden, `ziehe`
  zieht das Vorzeichen — nur mit einem Winkel, sonst bleibt die Zufallsfolge wie vorher) und `parallelFactor`
  (`parallelAnteil`: zu jeder gezogenen Mode eine zweite mit Winkel 90° auf halbem Weg zur nächsten, Dämpfung wie die
  Mode, ohne Abstrahlung, Formwert = Anteil mal ihr senkrechter Formwert; nichts wird dafür gezogen). Alles immer
  geschrieben, ein Körper aus einem Bauplan schreibt 0; Fassung 4 liest sich mit 0; ein Winkel über 90° macht die
  Datei unlesbar.
- **`kRechenFassung`** — die Fassung der Rechnung (2 seit S149, DM1 a). Hochzählen, wenn sich bei gleichem Bauplan ein anderer Körper
  ergäbe; dann meldet der Wächter (`test_LoopKoerperDatei`, „Waechter“) jeden mitgelieferten Körper als veraltet, bis
  `InstrumentBodyTool` ihn neu rechnet. Ebenso, wenn sich der Bauplan ändert (der Abdruck passt nicht mehr).
- **Datei** (`loop/Koerper`, Qt-JSON, in der App und im Werkzeug): jedes Feld immer geschrieben; Zahlen kommen genau
  zurück; der Samen steht als Text (64 Bit); eine Mode mit falscher Zahl an Werten macht die Datei ungültig.

### 5.7 Luftraum — `InstrumentBody/Luftraum.hpp` (34k-5, F-K1 a)

Die Moden der Platte und die Luft in den Löchern, gekoppelt über den einen Druck des Hohlraums — das Modell der
IC-Vorlage `harvest/instruments-calculation/rechnung/gemeinsam/Koerper` (dort nach Wright 1996, Gl. A.12–A.28), hier
auf den gerechneten Moden der Platte **im Vakuum** (M-orthonormal, modale Masse 1):

    q̈_k + ω_k² q_k = a_k P + φ_k(Steg) F,   M_a ü = P,   P = −α (Σ a_k q_k + u),   α = ρ c² / V

- `a_k = ∫ φ_k dS` ist das Volumen je Auslenkung der Mode (`Auswertung::volumen`); `u` das Volumen, das durch alle
  Löcher nach außen tritt. Mehrere Löcher liegen parallel: je Loch die Länge L = Plattendicke + Mündung · Radius des
  flächengleichen Kreises, 1/M_a = Σ A_j / (ρ L_j), der Fluss teilt sich wie A_j / L_j (`luftraumGroessen`,
  `LuftraumGroessen::helmholtzHz`).
- **Gerechnet** wird das ungedämpfte System genau: Diagonale plus Rang 1 (`eigenRangEins`), die Eigenwerte als
  Nullstellen der Säkulargleichung 1 + α Σ z_i² / (d_i − λ) = 0 durch Halbieren vom näheren Pol aus, die Vektoren
  z_i / (d_i − λ); Einträge ohne Kopplung und gleiche Diagonalwerte werden vorher abgespalten. Keine fremde Bibliothek.
- `koppleLuftraum(frequenzen, volumen, groessen)` gibt je Mode Frequenz, Anteile der Plattenmoden, `oeffnung` (u) und
  den Anteil der Bewegungsenergie in der Luft; mit einem Loch eine Mode mehr als hinein. An der untersten gehen Platte
  und Luft gegeneinander, darüber miteinander.
- `mitLuftraum(auswertung, plan, luft, plattenModen)` wertet jede gekoppelte Mode aus: die gemischte Form durch
  `Auswertung::werte` bei der neuen Frequenz (Wert an den Stegpunkten, Abstrahlung der Platte), dazu die Löcher als
  kleine Quellen in ihrem Schwerpunkt (`lochAbstrahlung`); Verlustfaktor = der der Platte mal ihrem Anteil plus
  Anteil der Luft / Güte. Das Werkzeug ruft es, wenn der Bauplan einen Hohlraum hat; der Körper bleibt eine Liste von
  Moden wie ohne (KB1 a).
- **Grenzen:** nur der Druck des Hohlraums als Ganzes (keine höheren Moden der Luft im Kasten), die Wände außer der
  Platte starr, die Luft vor der Platte nur als Mündungskorrektur, Löcher für die Abstrahlung als Punktquellen (an
  Hörorten dicht über einem Loch grob), der statistische Teil über den gerechneten Moden ohne Luftraum.
- **Gemessen** (S196): Rest von `eigenRangEins` gegen Eigen 2·10⁻¹⁶; eine Prüfdecke 0,4 m × 0,3 m mit Loch über 12 l:
  Helmholtz 131 Hz und erste Deckenmode 116 Hz werden 77 und 189 Hz.

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
„KQ46 Punktfedern“ (S210): eine Feder an der frei aufliegenden Prüfplatte gegen die Reihe nach Navier
(1 + k Σ φ_r²/(λ_r − λ) = 0, alle drei Äste je (m, n) bis 40), gegen die Rechnung im Modenraum
(K = diag(λ) + k φ φᵀ), am feineren Netz; die Dämpfung nach KQ49 a; `baueSystem` mit den Federn des Bauplans,
`pruefe`, die Datei mit `pointSprings`.

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


`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperLoch.cpp` (S196, KQ7 a) — ohne Qt-Anwendung: Bauplan mit
Löchern (Fläche, Masse, `pruefe`), die Datei (Fassung 4, `holes` immer geschrieben, Fassung 3 lesbar, Abdruck), das
Netz für jede Umsetzung aus `netzgeber()` (im Loch kein Dreieck, Fläche, Lochrand als `lochkanten`, Leisten enden am
Lochrand), der Kreisring außen eingespannt und innen frei gegen die geschlossene Lösung nach Kirchhoff (Bessel,
Determinante je Knotendurchmesserzahl; die Lösung selbst am Grenzfall der vollen Scheibe geprüft; gemessen S196:
fünf Moden auf 0,09 %; mit gehaltenem Lochrand liegt die erste Mode beim Vierfachen), der Ring-Kolben auf der Achse
(das Loch strahlt nicht).

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperLuftraum.cpp` (S196, F-K1 a) — ohne Qt-Anwendung:
`eigenRangEins` gegen den dichten Löser von Eigen (auch ohne Kopplung und mit gleichen Diagonalwerten; Spur), die
geschlossenen Lösungen (Helmholtz, Kasten ohne Loch, zwei gekoppelte Schwinger, Phase von Platte und Luft), der
Hohlraum aus dem Bauplan (Länge der Luft im Loch, zwei Löcher parallel, Datei mit `cavity`, Prüfung), eine
gerechnete Decke mit Loch (`volumen`, riesiger Hohlraum lässt die Platte wie sie ist, Verschachtelung, Spur).

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperBauplan.cpp` — ohne Qt-Anwendung: Geometrie,
Dickenraster, Masse und Werkstoffe (auch Glas), `pruefe`, die Datei (Hin- und Rückweg, jedes Feld, NaN als null,
falsches Format, zu neue Fassung).

## 7. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.25.0 | 2026-11-20 | Session 210, Stufe 34k-6, KQ46 a, KQ49 a: Federn an Punkten — `Punktfeder` (Ort, Steife N/m, Verlustfaktor, Quelle) in `Bauplan::punktfedern`, **`.lvinstbodyplan` Fassung 9** mit `pointSprings` (`x`, `y`, `stiffness`, `lossFactor`, `source`; immer geschrieben, Fassung 8 lesbar ohne Federn); `setzePunktfedern` (k · N Nᵀ auf K), `baueSystem(netz, element, plan)` setzt sie, `baueSystemOhnePunktfedern`; `Abtastung`, `bezug`, `abtastung`, `wertBei` von `Auswertung.hpp` nach `Platte.hpp`. Dämpfung: `Auswertung::setzePunktfedern`, `ModeImKoerper::federAnteil` (nicht in der Körperdatei), η = (1 − Anteil) · η_Platte + Σ a_f η_f. `pruefe`: Steife und Verlustfaktor nicht negativ, Ort im Umriss und in keinem Loch. Werkzeug: meldet Zahl und Summe der Federn und je Mode ihren Anteil. `kRechenFassung` bleibt 2 — ein Bauplan ohne Federn rechnet wie vorher, alle Körper mit Bauplan neu (Abdruck). Test „KQ46 Punktfedern“ in `test_LoopKoerperPlatte`: gegen die Reihe nach Navier 0,1 %, gegen den Modenraum mit allen Moden 10⁻⁶ und mit 60 Moden 0,03 %, am feineren Netz 0,05 %. |
| 0.24.0 | 2026-11-18 | Session 206, Stufe 34k-6, KQ35-2 a, KQ35-1 a: die Richtung am Steg — `ModeImKoerper::stegWinkelGrad` mit `senkrechtAm` / `parallelAm`, `StatistischerTeil::stegWinkelGrad` und `::parallelAnteil`, `ziehe` mit gezogenem Vorzeichen und Moden nur parallel zur Decke; `antwort`, `antwortSpektrum*`, `mobilitaet` mit dem senkrechten Formwert (Winkel 0: Bit für Bit wie vorher). **`.lvinstbody` Fassung 5** (`bridgeAngle` je Mode, `bridgeAngle` und `parallelFactor` im statistischen Teil, immer geschrieben; Fassung 4 liest sich mit 0). **`kFitFassung` 4**: `make_gitarre.py` füllt die Modendichte über 2,2 kHz nach Elie 2011 auf und schreibt die Richtung nach Elie 2012. Tests „KQ35 Ziehung mit Richtung am Steg“, Mobilität mit Winkel, Datei Fassung 5. `kRechenFassung` bleibt 2. |
| 0.23.0 | 2026-11-18 | Session 206, Stufe 34k-6, KQ31-1 a: Leisten übereinander — `LeistenUebereinander` (`NurDieErste` / `Addieren`) in `Bauplan::leistenUebereinander`, **`.lvinstbodyplan` Fassung 8** mit `barOverlap` (`first` / `add`, immer geschrieben, Fassung 7 lesbar als `first`); `steifeBei` addiert mit `Addieren` die Höhen aller Leisten am Punkt (Werkstoff und Richtung der ersten); `Ueberlagerung`, `ueberlagerungen(plan)`. Werkzeug: meldet je Paar von Leisten übereinander Fläche und Volumen — mit `first` als Warnung (das Volumen fehlt in der Rechnung), mit `add` als Hinweis, bei verschiedenen Werkstoffen mit Vermerk. Die Harfe trägt `add` (Masse der Rechnung 2,179 statt 1,764 kg), alle übrigen Baupläne `first`. `kRechenFassung` bleibt 2 — ein Bauplan mit `first` rechnet wie vorher, alle Körper mit Bauplan neu (Abdruck). Test „KQ31-1 Leisten uebereinander“ in `test_LoopKoerperPlatte`. |
| 0.22.2 | 2026-11-16 | Session 201, Stufe 34k-6, KQ28 a: `kFitFassung` 3 — die Form je Mode längs des Stegs wird an den Stegkurven und den Schallkurven gemeinsam gestellt (`make_gitarre.py` Schritt 4). Die zwei Aufnehmer an den Enden der Stegeinlage legen die Form zwischen 500 und 1000 Hz nicht fest: an fünf Moden widersprach sie dem, was die Mikrofone an den drei Schlagorten zeigen, und der Körper lag an der ersten Saite bei 780–880 Hz rund 10 dB unter der Messung. Am Format und an der Bibliothek sonst nichts; der Körper der Gitarre ist neu geschrieben (Pole und gezogene Moden unverändert). |
| 0.22.1 | 2026-11-15 | Session 199, Stufe 34k-5, KQ26 a: `kFitFassung` 2 — ein gemessener Körper darf über seinen gefitteten Moden einen statistischen Teil tragen, dessen Größen an der Messung bestimmt sind (Dämpfungsgesetz aus dem gemessenen Abklingen, Betrag der Abstrahlung aus dem gemessenen Pegel) statt mit `schaetze` aus den Moden; `ziehe` und das Format bleiben. An gefitteten Polen überschätzt `schaetze` das Produkt aus Formwert und Abstrahlgewicht (Gitarre: 10–14 dB), weil beide gegenläufig sind und das Gesetz unter der Dämpfung der Pole liegt. Am Code der Bibliothek sonst nichts. |
| 0.22.0 | 2026-11-15 | Session 199, Stufe 34k-5, KQ25 a: ein Körper nennt seine Herkunft — `Herkunft::Bauplan` (gerechnet) oder `Herkunft::Messung` (Modalfit an einer Messung), dazu `Koerper::messung`, `messungAbdruck`, `fitFassung` und `kFitFassung` (1; `make_gitarre.py` trägt dieselbe Zahl). **`.lvinstbody` Fassung 4**: `origin` (`plan` / `measurement`) und `measurement` (`file`, `fingerprint`, `fitVersion`), beides immer geschrieben; Fassung 3 liest sich als `plan`, eine unbekannte Herkunft macht die Datei unlesbar. Ein gemessener Körper trägt keinen Bauplan, keine Werkstoffe und kein Dämpfungsgesetz (η je Mode gemessen); seine Quelle ist die Beschreibung `.lvinstbodyfit` (Abdruck über die Bytes ohne Wagenrücklauf, `loop/Koerper`: `messungAbdruck`). `InstrumentBodyTool <körper> --rewrite` schreibt einen Körper in der heutigen Fassung neu, ohne zu rechnen. Erster gemessener Körper: `classical_guitar` (KQ23 a, KQ24 a). `kRechenFassung` bleibt 2. |
| 0.21.0 | 2026-11-14 | Session 198, Stufe 34k-5, KQ20 a: Abschnitte des Rands — `Randabschnitt` (`kanteVon`, `kanteBis`: Kanten des Umrisses; `wegfeder` in N/m², `drehfeder` in N) in `Rand::abschnitte`, **`.lvinstbodyplan` Fassung 7** mit `rim.sections` (immer geschrieben, Fassung 6 lesbar); `baueSystem` mit `abschnittAmRand` (je Randkante gefragt: die Auslenkung dort nicht gehalten, K bekommt ∫ k_w w² ds + ∫ k_d φ_n² ds; ein Knoten, der auch am übrigen Rand liegt, ist gehalten), `RandFedern`, `naechsteUmrissKante`, `abschnittDerKante`; `pruefe` (Federn nicht negativ, Kanten im Umriss, keine Überdeckung, nicht der ganze Rand). Werkzeug: meldet die Abschnitte. `kRechenFassung` bleibt 2 — ein Bauplan ohne Abschnitte rechnet wie vorher, alle sechs Körper neu (Abdruck). |
| 0.20.0 | 2026-11-13 | Session 198, Stufe 34k-5, KQ18 a: der Rand der Platte im Bauplan — `Rand` (`Halt::Eingespannt` / `Halt::Elastisch`, `drehfeder` in N, ein Wert für den ganzen Rand), **`.lvinstbodyplan` Fassung 6** mit `rim` (immer geschrieben, Fassung 5 lesbar); `baueSystem` mit `drehfederAmRand` (∫ k_d φ_n² ds über den äußeren Rand) und `baueSystem(netz, element, plan)`; `gaussAufStrecke`; `pruefe` (Feder nicht negativ, nur mit elastischem Halt). Werkzeug: rechnet mit dem Rand des Bauplans; Probe-Schalter `--rim-spring`, `--rim-spring-end`. `kRechenFassung` bleibt 2 — ein eingespannter Bauplan rechnet wie vorher, alle sechs Körper neu (Abdruck). |
| 0.19.0 | 2026-11-13 | Session 197, Stufe 34k-5, Proben an der Harfe: `Auflager::Gestuetzt` (w = 0 am ganzen Rand, beide Drehungen frei; für beliebig liegende Ränder — die untere Schranke zu `Eingespannt`); Werkzeug: `--free-rim` und `--supported-rim` als Probe-Schalter (ein so gerechneter Körper ist keiner für das Programm). `Eingespannt` und `GelenkigAchsparallel` unverändert. |
| 0.18.0 | 2026-11-13 | Session 197, Stufe 34k-5, KQ17 a: Öffnungen des Hohlraums außerhalb der Platte (`Oeffnung`, `Hohlraum::oeffnungen`; `.lvinstbodyplan` Fassung 5 mit `cavity.openings`, immer geschrieben, Fassung 4 lesbar); `LochLuft::hoehe` und `::frei`, `luftraumGroessen` nimmt Löcher und Öffnungen parallel, `lochAbstrahlung` rechnet eine Öffnung als freie Punktquelle an ihrem Ort (Löcher der Platte Bit für Bit wie vorher); `pruefe`. Test „Luftraum Oeffnungen ausserhalb der Platte (KQ17 a)“. `kRechenFassung` bleibt 2; der Abdruck ändert sich mit der Fassung — alle mitgelieferten Körper neu gerechnet. |
| 0.17.0 | 2026-11-13 | Session 196, Stufe 34k-5, KQ10 a: `ModeImKoerper::luftAnteil` (`mitLuftraum` setzt ihn), `.lvinstbody` Fassung 3 mit `airShare` je Mode; erster Körper mit Hohlraum ist die mitgelieferte Oud. |
| 0.16.0 | 2026-11-13 | Session 196, Stufe 34k-5, KQ7 a: Löcher in der Platte (`Platte::loecher`, `.lvinstbodyplan` Fassung 4 mit `plate.holes`; `imLoch`, `flaeche(Platte)`, `kantenSchneidenSich`, Prüfung); `NetzAuftrag::loecher`, `Netz::lochkanten`, `lochkantenVon`, `imUmriss` mit `aussenBleibt`; `NetzgeberCdt` spart Löcher aus; `baueSystem` lässt den Lochrand frei; Masse und Abstrahlung ohne das Loch. Test `test_LoopKoerperLoch`. Werkzeug: `--shapes`. **Luftraum** (F-K1 a, KQ8 a): `Hohlraum` im Bauplan (`cavity`), `Luftraum.hpp` (`eigenRangEins`, `luftraumGroessen`, `koppleLuftraum`, `lochAbstrahlung`, `mitLuftraum`), `Auswertung::volumen`; das Werkzeug koppelt, wenn der Bauplan einen Hohlraum hat. Test `test_LoopKoerperLuftraum`. Ohne Löcher und ohne Hohlraum Bit für Bit wie vorher. |
| 0.10.0 | 2026-10-24 | Session 149, Stufe 31c-6 Teil 2: Mobilität (`Mobilitaet.hpp`: MAESSTRO Gl. 10 und 12, Terzbänder, Dichte je Band); Bericht im Werkzeug (`--report`). |
| 0.11.0 | 2026-10-24 | Session 150, Stufe 31c-6 Teil 3–5: `kDiskantAb` = 41 in `Mobilitaet.hpp` (PF2 a); Prüfstein `test_LoopKoerperPruefstein` (PS3 a, PS4 a, KV2 a). |
| 0.12.0 | 2026-10-24 | Session 150, Stufe 31c-7 Schritt 1: Antwort des Bodens (`Antwort.hpp`: H je Stegpunkt und Hörort, im Spektrum der FFT, Länge). |
| 0.13.0 | 2026-10-24 | Session 151, Stufe 31c-8: Nachschärfen im Eigenlöser (RS1 a, `EigenErgebnis::nachgeschaerft`); der Rest in der Fehlermeldung in Exponentenschreibweise. |
| 0.15.0 | 2026-10-28 | Session 160, HU4 a und HU4-1 a: die Rückseite (`Rueckseite` im Bauplan, Fassung 3 mit `backside`; `naechsterRandpunkt`); `Auswertung` rechnet das Rayleigh-Integral zweiseitig, wenn sie an ist (Kurzschluss über den Rand, Schatten erster Ordnung), sonst Bit für Bit wie bisher; Test „HU4 Rueckseite: Kolben ohne Schallwand“. Das Dämpfungsgesetz `Daempfung` wandert in den Bauplan (Block `damping`, mit Quelle); das Werkzeug nimmt es für Moden und statistischen Teil; der Steinway D mit c = 4,5 1/s (Ege 2013). |
| 0.14.0 | 2026-10-25 | Session 151, Stufe 31c-8: `antwortLaengeS` ohne Obergrenze 8 s (AW2 a); `antwortSpektrum` schnell über ein Raster mit exaktem Kern schmaler Resonanzen (AW3-1 a), die bisherige Rechnung als `antwortSpektrumExakt`. |
| 0.9.0 | 2026-10-24 | Session 149, Stufe 31c-6 Teil 1: Dämpfung je Werkstoff (DM1 a) — `Verformungsenergie.hpp`, `Steife::werkstoff`, `Plattenelement::integrationsPunkte`, `verlustfaktorAusAnteilen`, `ModeImKoerper::werkstoffAnteil`, im statistischen Teil das Mittel; `kKoerperFassung` 2, `kRechenFassung` 2. |
| 0.8.0 | 2026-10-24 | Session 148, Stufe 31c-5d: der Körper (`Koerper.hpp`: `Koerper`, `Rechnung`, `kKoerperFassung`, `kRechenFassung`), die Datei in `loop/Koerper`, der Wächter. |
| 0.7.0 | 2026-10-23 | Session 148, Stufe 31c-5c: statistischer Teil (`Statistik.hpp`: `StatistischerTeil`, `schaetze`, `ziehe`, `Zufall`; ST1 a). |
| 0.6.0 | 2026-10-23 | Session 148, Stufe 31c-5b: Auswertung je Mode (`Auswertung.hpp`: Wert an den Stegpunkten, Dämpfung nach RT-0425 Gl. 1, Abstrahlgewicht nach Rayleigh je Hörort); `PlattenSystem::elemente`, `Plattenelement::basis`, `Auflager::Frei`. |
| 0.5.0 | 2026-10-23 | Session 148, Stufe 31c-5a: Hörorte (`Hoerort`, BM6 a), `.lvinstbodyplan` Fassung 2 mit `listeningPoints`. |
| 0.4.0 | 2026-10-23 | Session 148, Stufe 31c-4: Eigenlöser — Schnittstelle `Eigenloeser` (EG1 a), Umsetzung mit Spectra 1.2.0 und `SimplicialLDLT` (EG2 a: Zählung nach Sylvester, Teilfenster, Deflation, Rest je Mode), `Umsetzungen.hpp` mit `netzgeber()` und `eigenloeser()`, Werkzeug `InstrumentBodyTool`. |
| 0.3.0 | 2026-10-23 | Session 147, Stufe 31c-3: Platte — Reissner-Mindlin, Zonen nach RT-0425, `Plattenelement` mit `LagrangeDreieck(k)`, `baueSystem` (eingespannt, gelenkig), `DuenneMatrix` (EL1 a). |
| 0.2.0 | 2026-10-23 | Session 147, Stufe 31c-2: Netz — Schnittstelle `Netzgeber`, Auftrag aus dem Bauplan (Ränder der Leisten, Kante nach MAESSTRO Gl. 1), Umsetzung mit CDT 2.0.1 (BM4 a). |
| 0.1.0 | 2026-10-23 | Session 147, Stufe 31c-1: Bauplan (Materialien orthotrop/isotrop, Platte mit Dickenraster, Leisten, Stegpunkte, Geometrie, Masse, Prüfung). |
