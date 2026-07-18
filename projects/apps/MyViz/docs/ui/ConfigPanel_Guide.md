# ConfigPanel — Bedienung und Aufbau der Visualizer-Konfiguration

> **Version:** 1.0.0
> **Datum:** 2026-07-18
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
- **Visibility-System** für bedingte Parameter
- **Zwei Preset-Ebenen** (Visualizer- und Modul-Presets)
- **Gradientvorschau** in Farb-Preset-Dropdowns + **Gradient-Editor**
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

### 2.2 Gruppen

Parameter sind in **kollabierbare Hauptgruppen** organisiert (Klick auf den
Header klappt ein/aus); die Reihenfolge folgt den nummerierten Gruppennamen
(„1. Audio", „2. Shape", …). Innerhalb einer Gruppe bündeln **SubGroups**
(Rahmen-Boxen) zusammengehörige Parameter, z. B. „Smoothing". Gruppen-Header
erhalten je nach Namen ein Emoji-Icon (Audio 🎵, Color 🎨, Shape ⭕, …).

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
Parameter einen der geforderten Werte hat (OR-Logik). Beispiele:

| Parameter | Sichtbar nur wenn … |
|---|---|
| `timeMs` | Smoothing-Algorithmus = EMA oder DEMA |
| `windowSize` | Smoothing-Algorithmus = SMA oder WMA |
| `sides` | Shape = Ngon oder Star |

Fehlt ein erwarteter Parameter: zuerst prüfen, ob eine Abhängigkeit ihn
ausblendet oder seine Gruppe eingeklappt ist. Parameter mit `advanced`- oder
`hidden`-Flag werden generell nicht angezeigt.

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

### 3.2 Sortierung über Gruppen-Präfix-Strings

`buildUIFromParams` sortiert dreistufig: **group-String alphabetisch** →
Minimum-`order` der SubGroup → Parameter-`order`. Die Pipeline-Reihenfolge
(„1. Audio" vor „2. Shape") existiert also nur, weil die Gruppennamen
numerische Präfixe tragen — eine reine String-Konvention, keine echte
Stage-Zuordnung (Umbau auf ein Stage-Enum ist Teil des Phase-4-Konzepts).

### 3.3 Gruppen = CollapsibleGroupBox

Hauptgruppen werden lazy als `CollapsibleGroupBox` erzeugt
(`getOrCreateGroup`), inklusive Emoji-Icon per Namens-Keyword; SubGroups sind
einfache `QGroupBox` mit Inline-Stylesheet.

### 3.4 Zwei Preset-Ebenen im Code

- **Visualizer-Ebene:** `setupPresetUI()`/`refreshPresetList()` +
  `VisualizerPresetManager` (Snapshot über Param-IDs).
- **Modul-Ebene:** Save-Button wird per String-Heuristik an Preset-Dropdowns
  gehängt (ID enthält `preset` und `smooth`/`audio`/`color`);
  `onModulePresetSave()` ruft `savePreset()` des jeweiligen Moduls.

Ebenso heuristisch: Gradientvorschau (ID enthält `preset` + `color`-Variante)
und der Editor-Zugriff über eine dynamic_cast-Kaskade auf die fünf konkreten
Visualizer-Typen — je Visualizer ein anderer Weg zum `ColorGradientModule`
(beim Oscilloscope wird der Kanal aus der Param-ID geparst).

---

## 4. Bekannte Einschränkungen (Stand Phase 4-Vorbereitung)

- **Modul-Preset-Save funktioniert derzeit nur beim Pulsing-Visualizer** —
  `onModulePresetSave()` bricht für alle anderen Visualizer mit einer Warnung
  ab; die Save-Buttons sind dort wirkungslos.
- **Gradient-Editor erreicht bei Waveform nur den Mono-Kanal** — die
  Left/Right-Gradienten sind nur über die generischen Parameter erreichbar,
  nicht über den Editor-Dialog.
- Die String-Heuristiken (Gruppen-Präfixe, `preset`+`color`-Matching) brechen
  still bei Umbenennungen — beim Anlegen neuer Parameter Konventionen exakt
  einhalten. Konsolidierung ist Gegenstand des Phase-4-Umbaus
  (Blaupause: `harvest/config-pipeline/README.md`).

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
| 1.0.0 | 2026-07-18 | Neu aus `harvest/old_docs/userguide/ConfigPanel_UserGuide.md` konsolidiert; Abschnitt „Innerer Aufbau" und „Bekannte Einschränkungen" ergänzt (Basis: Session-29-Code-Analyse); Querlinks auf neue Doku-Struktur umgestellt |
