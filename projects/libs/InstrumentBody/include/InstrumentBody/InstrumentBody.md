# InstrumentBody — Rechenkern der Instrumentkörper

> **Version:** 0.2.0  
> **Datum:** 2026-10-23  
> **Typ:** CppModuleDoc  
> **Status:** im Bau (Stufe 31c; gebaut: 31c-1 Bauplan, 31c-2 Netz)  
> **Modul:** lumi::koerper (Lib **InstrumentBody**, header-only INTERFACE)  
> **Dateien:** Bauplan.hpp, Netz.hpp, NetzCdt.hpp (bindet CDT ein)  
> **Namespace:** lumi::koerper  
> **Abhängigkeiten:** kein Qt, keine App-Teile; CDT 2.0.1 nur in `NetzCdt.hpp` (hinter der Schnittstelle des Netzes); ab 31c-4 Eigen und Spectra, nur hinter der Schnittstelle des Eigenlösers  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Bauplan](#2-bauplan)
3. [Netz](#3-netz)
4. [Eigenlöser](#4-eigenlöser)
5. [Tests](#5-tests)
6. [Changelog](#6-changelog)

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
| Werkzeug Bauplan → Körper | `projects/exec/InstrumentBodyTool` (ab 31c-4) | ja (kompiliert den Leser mit) |
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
- **Prüfen**: `pruefe(plan)` nennt je Befund einen Satz (Material fehlt oder doppelt, Werte nicht positiv,
  Querzahl außerhalb der Stabilität — isotrop 0 ≤ ν < 0,5, orthotrop 1 − ν_xy·ν_yx > 0 —, Umriss ohne Fläche,
  Leiste ohne Höhe je Punkt, Stegpunkte nicht aufsteigend).
- **Geometrie**: `flaeche`, `innen`, `abstandZurLinie`, `laenge`, `masse` (Platte über ein Raster von 5 mm
  summiert, Leisten als Länge mal mittlerer Querschnitt).

Die Datei `.lvinstbodyplan` (Format `lumiviz-instrument-body-plan`, Fassung 1) schreibt jedes Feld, auch leere;
NaN im Raster steht als `null`.

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
- **Eine eigene Umsetzung dazu:** eine Klasse, die `Netzgeber` umsetzt, in `netzgeber()` von
  `test_LoopKoerperNetz.cpp` eintragen — alle Fälle laufen dann für beide; der Fall Steinway vergleicht mit
  `werte/netz_steinway.txt` (Knoten und Dreiecke auf 2 %, kleinster Winkel bis 1° darunter). Der Schalter im
  Werkzeug kommt mit dem Werkzeug (31c-4).
- **Gemessen** (Release-clang, S147): Steinway D bis 5 kHz — Kante 13,3 mm, 21 491 Knoten, 42 329 Dreiecke,
  kleinster Winkel 25,0° außer 8 Ecken der Eingabe (zwei Zwangskanten schließen den Winkel ein: Kreuzungen von
  Rippe und Steg), Fläche genau, jede Zwangslinie als Kantenfolge, 0,07 s.

## 4. Eigenlöser

*Noch nicht gebaut (31c-4).* Entschieden (BM5-1 a mit Zusatz, Patrik S147):

- **Eine Schnittstelle vor dem Löser.** Netz, Platte und Abstrahlung kennen nur sie: dünn besetzte K und M, ein
  Frequenzfenster, zurück Frequenzen und Formen. Kein Typ von Eigen oder Spectra geht über die Schnittstelle.
- **Erste Umsetzung:** Eigen 3.4.1 und Spectra 1.2.0 (MPL-2.0), unverändert, feste Fassung; Bezug (fest im Repo
  oder per Tag) nach einer Probe unter beiden Compilern. Ein eigener Löser wird eine zweite Umsetzung derselben
  Schnittstelle, gewählt im Werkzeug per Schalter.
- **Tests gegen die Schnittstelle** für jede vorhandene Umsetzung: geschlossene Lösungen (Rechteckplatte frei
  aufliegend, orthotrop; kleine Matrizen mit bekannten Eigenwerten), Vollständigkeit im Fenster, Orthogonalität
  bezüglich M; abgelegte **Bezugswerte**; ein **Vergleichsfall alt gegen neu** (Frequenzen auf Toleranz, Formen
  über den MAC-Wert, Rechenzeit beider), sobald es zwei Umsetzungen gibt.

Dieser Abschnitt wird mit 31c-4 zur eigentlichen Doku des Lösers: Schnittstelle, Umsetzung, wie eine eigene
dazukommt, Tests und Bezugswerte, wie der Vergleich läuft.

## 5. Tests

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperNetz.cpp` — gegen die Schnittstelle, für jede
Umsetzung aus `netzgeber()`: Elementgröße nach Gl. 1, Quadrat mit zwei sich kreuzenden Zwangslinien (Fläche,
Zwangslinien, Winkel, Fläche je Dreieck, Umlaufsinn, im Umriss, zwei Läufe gleich), ein Auftrag ohne Kante (Fehler),
der Steinway D bis 5 kHz mit den Bezugswerten `werte/netz_steinway.txt` (`LUMIVIZ_NETZ_WERTE_SCHREIBEN=1`).


`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperBauplan.cpp` — ohne Qt-Anwendung: Geometrie,
Dickenraster, Masse und Werkstoffe (auch Glas), `pruefe`, die Datei (Hin- und Rückweg, jedes Feld, NaN als null,
falsches Format, zu neue Fassung).

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.2.0 | 2026-10-23 | Session 147, Stufe 31c-2: Netz — Schnittstelle `Netzgeber`, Auftrag aus dem Bauplan (Ränder der Leisten, Kante nach MAESSTRO Gl. 1), Umsetzung mit CDT 2.0.1 (BM4 a). |
| 0.1.0 | 2026-10-23 | Session 147, Stufe 31c-1: Bauplan (Materialien orthotrop/isotrop, Platte mit Dickenraster, Leisten, Stegpunkte, Geometrie, Masse, Prüfung). |
