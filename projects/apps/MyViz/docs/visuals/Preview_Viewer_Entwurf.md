# MyViz — Preview-Viewer je Parametergruppe (Mini-Entwurf 6.1)

> **Version:** 0.1.0
> **Datum:** 2026-07-19
> **Typ:** Entwurf (½ Seite, Freigabe-Gate für 6.2/6.3)
> **Status:** Zur Freigabe durch Patrik
> **Bezug:** [Config_Pipeline_Umsetzungsplan.md](Config_Pipeline_Umsetzungsplan.md) Schritt 6 · [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) §4.2 · Tap-Points (Schritt 3.5, `IVisualizer::tapPoints()`)
> **Sprache:** Deutsch

## Ziel

Je Pipeline-Stufe im ConfigPanel eine kleine Live-Vorschau der Roh-/Zwischendaten,
damit man beim Schrauben an einer Stufe sieht, was **in** diese Stufe hineingeht bzw.
aus ihr herauskommt — ohne die Hauptansicht zu verlassen. Default **aus**, kostenlos
wenn ausgeblendet (N7).

## 1. Abgriff (Tap-Points, pull-basiert)

- Datenquelle ist ausschließlich `IVisualizer::tapPoints()` (Schritt 3.5): benannte
  Stufen-Ausgänge mit `stage` + `sample()` → `std::vector<float>`. Kein neuer
  Event-/Push-Pfad.
- **Tap nur bei Abonnent aktiv:** `sample()` wird ausschließlich für aktuell
  **sichtbare** Preview-Widgets aufgerufen. Kein sichtbares Preview → kein Timer,
  kein Sample-Aufruf, null Overhead (N7).
- Ausbau der Taps je Visualizer (heute: Equalizer-Pilot mit `tap.audio`/`tap.map`):
  je migrierter Stufe 1 und 2 ein Tap; wo Stufe 2 keine eigenen Daten hat (Pulsing),
  entfällt der Tap einfach — das Panel zeigt nur an, was der Visualizer anbietet.

## 2. Darstellung je Stufe (ein Widget, drei Zeichenmodi)

Ein kompaktes `TapPreviewWidget` (QWidget, ~48 px hoch, volle Gruppenbreite,
`paintEvent` mit QPainter — kein OpenGL):

| Stufe | Modus | Darstellung |
|---|---|---|
| 1 AudioSource | **Balken** | Bänder 0..1 (wie Mini-Equalizer) |
| 2 Mapping | **Balken** oder **Kurve** | Bänder → Balken; Sample-/Punktdaten (Waveform, Scope) → Kurve |
| 3 Color | **Farbstreifen** | Gradient des Handles der Untergruppe, direkt aus dem GradientHandle gerendert (kein Tap nötig) |
| 4 Render / 5 Peaks / 6 Post | — | keine Preview: Das Ergebnis IST die Hauptansicht; Post wirkt auf Frames |

Der Modus Balken/Kurve wird pro Tap vom Visualizer deklariert (kleines Enum-Feld
`TapPoint::display`, Default Balken) — keine Heuristik im Panel.

## 3. Ein-/Ausblenden je Gruppe

- Kleiner Toggle (Auge-Symbol) im Header jeder Stage-Gruppenbox, nur wenn der
  Visualizer für diese Stufe einen Tap (bzw. Stufe 3: ein Gradient-Handle) anbietet.
- Zustand wird **pro Visualizer + Stufe** persistiert (QSettings,
  `configpanel/preview/<visualizerId>/<stage>`), Default **aus**.

## 4. Update-Rate

- Ein gemeinsamer `QTimer` im ConfigPanel mit **20 Hz** (50 ms) treibt alle
  sichtbaren Previews; er **läuft nur**, wenn mindestens ein Preview sichtbar ist
  (Start beim ersten Einblenden, Stopp beim letzten Ausblenden/Panel-Hide).
- `sample()` liefert eine Kopie; das Widget zeichnet nur bei geändertem Puffer neu.

## 5. Abschaltbarkeit / Messung (6.3)

- Default aus (siehe 3.); zusätzlich kein Timer ohne Abonnent — damit ist N7
  konstruktiv erfüllt. Nachweis in 6.3: Frametime-Vergleich mit/ohne eingeblendetem
  Preview und mit ausgeblendetem Preview vs. Stand heute.

## Nicht-Ziele (bewusst)

Kein OpenGL im Panel, keine Historie/Skalen-Beschriftung, keine Preview für
Stufe 4–6, kein Konfigurationsdialog — das Preview bleibt ein Blickfenster,
kein Analyse-Tool.

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 0.1.0 | 2026-07-19 | Initial (Session 30) — zur Freigabe |
