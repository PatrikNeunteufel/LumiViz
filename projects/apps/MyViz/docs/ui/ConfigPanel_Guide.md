# ConfigPanel — Bedienung und Aufbau der Visualizer-Konfiguration

> **Version:** 2.0.0
> **Datum:** 2026-07-19
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## 1. Überblick

Das ConfigPanel (Panel-ID `config`, Menü **View → Panels → Visualizer Config**,
standardmäßig versteckt) ist die zentrale Oberfläche zur Konfiguration des
**aktiven** Visualizers. Es generiert seine Widgets automatisch aus den
Parameter-Beschreibungen (`paramDescs()`) des Visualizers — neue Parameter
erscheinen ohne UI-Code.

- **Automatische Widget-Generierung** je Parameter-Typ
- **Pipeline-Stufen-Gruppen** (Reihenfolge folgt dem Datenfluss, Phase 4)
- **Visibility-System** für bedingte Parameter (transitiv)
- **Zwei Preset-Ebenen** (Visualizer- und Modul-Presets, für alle Visualizer)
- **Gradientvorschau** in Farb-Preset-Dropdowns + **Gradient-Editor** (alle Handles)
- **Stage-Previews** (Live-Vorschau je Stufe, Auge-Toggle, Default aus)
- **Undo/Redo** über den CommandBus (Ctrl+Z/Y, Slider-Drag = 1 Schritt)
- **Default-Reset** per Rechtsklick (Parameter / Untergruppe / Gruppe)
- **[Custom]-Indikator** bei manuellen Änderungen

API-Details: CppModuleDoc [ConfigPanel](../../include/UI/panels/ConfigPanel.md).
Alle Parameter im Detail: [Parameter-Referenz](../visuals/Parameter_Reference.md).

---

## 2. Bedienung

### 2.1 Widget-Typen

| Parameter-Typ | Widget | Bedienung |
|---|---|---|
| Int / Float | Slider + SpinBox | Slider für grob, SpinBox für präzise — synchronisiert |
| Bool | Checkbox | An/Aus |
| Enum | Dropdown | Auswahl; Separator-Einträge (`---`) sind deaktiviert |
| String | Textfeld | Freitext |
| Color | Farbbutton | Öffnet QColorDialog |
| „…"-Parameter | Button | `displayName` endet auf `...` → wird als Aktions-Button gerendert (z. B. „Edit Gradient…") |

Tooltips zeigen Beschreibung und Wertebereich; ungültige Eingaben springen auf
den letzten gültigen Wert zurück.

### 2.2 Gruppen = Pipeline-Stufen

Parameter sind in **kollabierbare Hauptgruppen** organisiert (Klick auf den
Header klappt ein/aus). Seit Phase 4 entspricht jede Hauptgruppe einer
**Pipeline-Stufe** in Datenfluss-Reihenfolge — Titel und Icon kommen aus der
zentralen Stage-Tabelle: 🎵 „1. Audio / Analysis" → 🔀 „2. Mapping" →
🎨 „3. Color" → 🖼️ „4. Rendering" → ✨ „5. Peak / Particles" → 🌟 „6. Post FX".
Stufen ohne Parameter erscheinen nicht. Innerhalb einer Gruppe bündeln
**SubGroups** (Rahmen-Boxen) zusammengehörige Parameter, z. B. „Smoothing"
oder „Peak Hold".

### 2.3 Presets — zwei Ebenen

**Visualizer-Presets** (Kopfzeile „Presets" ganz oben): Gesamtzustand des
Visualizers.

```
[ [Custom] / Default / --- / MeinPreset … ▼ ]  [Save]  [Delete]
```

- **[Custom]** (Index 0): Anzeige-Zustand „Werte entsprechen keinem Preset" —
  springt automatisch an, sobald ein Parameter manuell geändert wird.
- **Default** (Index 1): setzt auf die einprogrammierten Defaults zurück
  (`resetToDefaults()`).
- Darunter, durch `---` getrennt: benutzergespeicherte Presets
  (VisualizerPresetManager, siehe [Preset-System](../presets/Preset_System.md)).
- **Save** legt ein neues Preset unter frei wählbarem Namen an, **Delete**
  entfernt das gewählte User-Preset (Built-ins sind nicht löschbar).

**Modul-Presets** (im Parameterbereich): Dropdowns wie „Audio → Preset",
„Smoothing → Preset" oder „Color → Preset" tragen einen eigenen **Save**-Button
direkt daneben und speichern nur die Werte des jeweiligen Moduls.

### 2.4 Gradientvorschau und Gradient-Editor

Farb-Preset-Dropdowns zeigen je Eintrag eine **Vorschau des Farbverlaufs**
(GradientPresetDelegate) — man sieht vor der Auswahl, wie das Preset aussieht.

Der **Gradient-Editor** (eigener Dialog) öffnet sich über den „…"-Button beim
Farbverlauf (Parameter-ID enthält `editGradient`). Dort lassen sich Stops
bearbeiten und Gradient-Presets speichern; nach dem Schließen werden die
Preset-Dropdowns und alle Widgets neu synchronisiert.

### 2.5 Visibility-System

Parameter mit `dependsOn`/`dependsValues` erscheinen nur, wenn der referenzierte
Parameter einen der geforderten Werte hat (OR-Logik). Die Auswertung ist
**transitiv**: Hängt A an B und B an C, verschwindet A auch, wenn C die
Bedingung von B versteckt. (Sub-)Gruppen ohne sichtbare Parameter werden
komplett ausgeblendet. Beispiele:

| Parameter | Sichtbar nur wenn … |
|---|---|
| `audio.smooth.timeMs` | Smoothing-Algorithmus = EMA oder DEMA |
| `audio.smooth.windowSize` | Smoothing-Algorithmus = SMA oder WMA |
| `render.sides` (Pulsing) | Shape = NGon oder Star |
| `color.left.*` (Waveform) | Channel Mode = Stereo oder Both |

Fehlt ein erwarteter Parameter: zuerst prüfen, ob eine Abhängigkeit ihn
ausblendet oder seine Gruppe eingeklappt ist. Parameter mit `advanced`- oder
`hidden`-Flag werden generell nicht angezeigt.

### 2.6 Stage-Previews (Live-Vorschau je Stufe)

Stufen-Gruppen mit Datenabgriff tragen rechts im Header ein **Auge-Toggle**:

- **Stufe 1/2:** Live-Daten des Visualizers als **Balken** (Bänder) oder
  **Kurve** (Samples) — gespeist aus den Tap-Points (`IVisualizer::tapPoints()`).
- **Stufe 3:** ein **Farbstreifen pro Gradient-Handle** (bei Solid die
  Farbfläche); Streifen folgen der Kanal-Sichtbarkeit (z. B. Channel Mode)
  und Gradient-Änderungen live.

Default ist **aus**; der Zustand wird pro Visualizer und Stufe gemerkt
(QSettings `configpanel/preview/<visualizerId>/<stage>`). Ein gemeinsamer
20-Hz-Timer läuft **nur**, solange mindestens ein Preview sichtbar ist —
ausgeblendete Previews kosten nichts (Tap nur bei Abonnent aktiv).

---

## 3. Innerer Aufbau (kompakt)

Für Entwickler; Zeilenangaben Stand 2026-07-18, `src/UI/panels/ConfigPanel.cpp`
(~1750 Zeilen).

### 3.1 Der buildUIFromParams-Fluss

```
VisualizerChangedEvent / onActivate()
      └─► setVisualizer(viz) ─► rebuildUI() ─► viz->paramDescs()
                                    └─► buildUIFromParams(params)
                                          ├─ sortieren (s. u.)
                                          ├─ je Parameter: Widget-Creator (6 Typen)
                                          ├─ Gruppen/SubGroups anlegen
                                          └─ m_paramWidgets registrieren
```

Das Panel abonniert `VisualizerChangedEvent` in `onActivate()` und meldet sich
in `onDeactivate()` wieder ab (PanelBase-Lifecycle über show/hideEvent, siehe
[Panel_System](Panel_System.md)). `m_paramWidgets` (Map Param-ID → Widget-Info)
trägt sowohl `syncFromVisualizer()` (Werte zurückspielen, mit
`QSignalBlocker`) als auch `updateVisibility()`.

### 3.2 Sortierung über die Pipeline-Stufe

`buildUIFromParams` sortiert nach **`ModuleParamDesc::stage`** (Stage-Enum
1–6) → Minimum-`order` der SubGroup → Parameter-`order`. Alle Parameter einer
Stufe teilen sich eine Gruppe (`stage:<N>`, Titel/Icon aus der zentralen
Stage-Tabelle). Für unmigrierte Visualizer (Stage `None`) greift ein
Legacy-Fallback über numerische Gruppennamen-Präfixe — seit Schritt 5 sind
alle fünf Visualizer migriert, der Fallback bleibt als Sicherheitsnetz.

### 3.3 Gruppen = CollapsibleGroupBox

Hauptgruppen werden lazy als `CollapsibleGroupBox` erzeugt
(`getOrCreateGroup`); SubGroups sind einfache `QGroupBox` mit
Inline-Stylesheet. Die Header-Zeile trägt neben dem Titel optionale
Zusatz-Widgets (`addHeaderWidget`, z. B. das Preview-Auge).

### 3.4 Zwei Preset-Ebenen im Code

- **Visualizer-Ebene:** `setupPresetUI()`/`refreshPresetList()` +
  `VisualizerPresetManager` (Snapshot über Param-IDs; formatVersion-Migration
  siehe [FileFormat_Reference](../presets/FileFormat_Reference.md)).
- **Modul-Ebene:** `onModulePresetSave()` löst das Ziel-Modul **generisch** auf —
  Audio/Smoothing über `IVisualizer::audioSourceModule()`, Gradienten über die
  **Gradient-Handles** (`IVisualizer::gradients()`, Präfix-Match) — und
  funktioniert damit bei allen Visualizern.

Auch Gradientvorschau und Gradient-Editor laufen über die Handles: keine
dynamic_casts auf konkrete Visualizer, kein Kanal-Parsing aus Param-IDs;
beim Waveform sind Mono/Left/Right, beim Oscilloscope alle sechs Kanäle
gleichermaßen erreichbar.

### 3.5 Stage-Previews im Code

`buildStagePreviews()` (nach jedem `rebuildUI`) hängt je Stufe mit Tap-Points
bzw. Gradient-Handles ein Auge-Toggle in den Gruppen-Header und
`TapPreviewWidget`s an den Gruppenanfang. Ein gemeinsamer `QTimer` (50 ms)
pollt in `onPreviewTick()` nur die sichtbaren Previews; `updatePreviewTimer()`
startet/stoppt ihn (kein Abonnent → kein Timer, N7). Farbstreifen folgen der
Sichtbarkeit des `mode`-Parameters ihres Handles (`updatePreviewVisibility()`,
angebunden an `updateVisibility()`).

---

## 4. Bekannte Einschränkungen

Die Einschränkungen der Phase-4-Vorbereitung (Modul-Preset-Save nur Pulsing,
Gradient-Editor nur Waveform-Mono, String-Heuristiken für Reihenfolge und
Preview) sind mit den Schritten 4–6 **behoben**. Verbleibend:

- Stufen 4–6 haben bewusst **keine Stage-Preview** — das Ergebnis ist die
  Hauptansicht ([Entwurf](../visuals/Preview_Viewer_Entwurf.md), Nicht-Ziele).
- `advanced`-Parameter werden weiterhin generell ausgeblendet (kein
  „Erweitert"-Umschalter).

---

## 5. Siehe auch

- [Parameter-Referenz](../visuals/Parameter_Reference.md) — alle Parameter je Visualizer
- [Preset-System](../presets/Preset_System.md) — Formate, Speicherorte, Manager
- [Panel_System.md](Panel_System.md) — Lifecycle und Docking
- CppModuleDoc: [ConfigPanel](../../include/UI/panels/ConfigPanel.md)

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 2.0.0 | 2026-07-19 | Phase-4-Stand (Schritte 4–6): Stufen-Gruppen mit Stage-Tabelle, transitives Visibility-System, generische Modul-Presets/Gradient-Handles (Einschränkungen von 1.0.0 behoben), Stage-Previews (2.6/3.5), Undo/Redo + Default-Reset erwähnt |
| 1.0.0 | 2026-07-18 | Neu aus `harvest/old_docs/userguide/ConfigPanel_UserGuide.md` konsolidiert; Abschnitt „Innerer Aufbau" und „Bekannte Einschränkungen" ergänzt (Basis: Session-29-Code-Analyse); Querlinks auf neue Doku-Struktur umgestellt |
