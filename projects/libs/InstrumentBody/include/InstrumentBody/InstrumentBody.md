# InstrumentBody — Rechenkern der Instrumentkörper

> **Version:** 0.1.0  
> **Datum:** 2026-10-23  
> **Typ:** CppModuleDoc  
> **Status:** im Bau (Stufe 31c; gebaut: 31c-1 Bauplan)  
> **Modul:** lumi::koerper (Lib **InstrumentBody**, header-only INTERFACE)  
> **Dateien:** Bauplan.hpp  
> **Namespace:** lumi::koerper  
> **Abhängigkeiten:** keine (kein Qt, keine App-Teile); ab 31c-4 Eigen und Spectra, nur hinter der Schnittstelle des Eigenlösers  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Bauplan](#2-bauplan)
3. [Eigenlöser](#3-eigenlöser)
4. [Tests](#4-tests)
5. [Changelog](#5-changelog)

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

## 3. Eigenlöser

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

## 4. Tests

`projects/apps/LumiViz/tests/unit/UnitTests/test_LoopKoerperBauplan.cpp` — ohne Qt-Anwendung: Geometrie,
Dickenraster, Masse und Werkstoffe (auch Glas), `pruefe`, die Datei (Hin- und Rückweg, jedes Feld, NaN als null,
falsches Format, zu neue Fassung).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.1.0 | 2026-10-23 | Session 147, Stufe 31c-1: Bauplan (Materialien orthotrop/isotrop, Platte mit Dickenraster, Leisten, Stegpunkte, Geometrie, Masse, Prüfung). |
