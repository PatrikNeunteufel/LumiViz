# ConfigPanel — Benutzerhandbuch

> **Version:** 1.1.0  
> **Datum:** 2026-01-04  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** Endbenutzer, UI-Designer  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Wie ändere ich Parameter?](#4-wie-ändere-ich-parameter)
5. [Wie nutze ich Presets?](#5-wie-nutze-ich-presets)
6. [Wie funktioniert das Visibility-System?](#6-wie-funktioniert-das-visibility-system)
7. [Wie sind die Parameter gruppiert?](#7-wie-sind-die-parameter-gruppiert)
8. [Stolpersteine und Lösungen](#8-stolpersteine-und-lösungen)
9. [Troubleshooting](#9-troubleshooting)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Überblick

Das ConfigPanel ist die zentrale Benutzeroberfläche zur Konfiguration von Visualizern. Es generiert automatisch passende Widgets für jeden Parameter.

### Features

- **Automatische Widget-Generierung** basierend auf Parameter-Typ
- **Visibility-System** für bedingte Parameter
- **SubGroups** für logische Gruppierung
- **Preset-Dropdowns** mit Save/Delete-Buttons
- **[Custom]-Indikator** bei manuellen Änderungen

### Widget-Typen

| Parameter-Typ | Widget | Aussehen |
|---------------|--------|----------|
| Int | Slider + SpinBox | `[====●====] [64]` |
| Float | Slider + SpinBox | `[====●====] [1.50]` |
| Bool | Checkbox | `☑ Aktiviert` |
| Enum | Dropdown | `[Option ▼]` |
| Color | Farbbutton | `[■] Farbe wählen` |
| Button | Button | `[Ausführen]` |

---

## 2. Voraussetzungen

### Für Endbenutzer

- [ ] MyViz gestartet
- [ ] Visualizer ausgewählt

### Für Entwickler

- [ ] Qt6::Widgets
- [ ] IVisualizer mit paramDescs()

---

## 3. Schnellstart

### ConfigPanel öffnen

1. Starte MyViz
2. Wähle einen Visualizer (z.B. "Pulsing")
3. Das ConfigPanel erscheint automatisch oder über **View → Config**

### Ersten Parameter ändern

1. Finde "Gain" im Audio-Bereich
2. Ziehe den Slider nach rechts
3. Die Visualisierung reagiert sofort

---

## 4. Wie ändere ich Parameter?

### 4.1 Slider verwenden

Für numerische Parameter (Int, Float):

```
        Drag hier
            ↓
[========●========] [1.50]
                      ↑
               Oder hier tippen
```

- **Slider ziehen:** Schnelle, grobe Anpassung
- **SpinBox tippen:** Präzise Eingabe
- **Beide sind synchronisiert**

### 4.2 Dropdown verwenden

Für Auswahl-Parameter (Enum):

```
┌─────────────────┐
│ EMA          ▼  │  ← Klicken
├─────────────────┤
│ None            │
│ SMA             │
│ EMA          ●  │  ← Auswählen
│ WMA             │
│ DEMA            │
└─────────────────┘
```

#### 4.2.1 Gradient-Dropdowns mit Vorschau

Gradient-Preset-Dropdowns zeigen eine **Farbvorschau** für jeden Eintrag:

```
┌───────────────────────────────┐
│ [▓▓▒▒░░▒▒▓▓] Fire          ▼  │  ← Aktuelle Auswahl mit Vorschau
├───────────────────────────────┤
│ [▓▓▒▒░░▒▒▓▓] Fire             │
│ [░░▒▒▓▓▓▓▓▓] Ocean            │
│ [▓▓░░▒▒▓▓░░] Neon          ●  │  ← Auswahl mit Vorschau
│ [░▒▓▓▓▓▓▓▒░] Rainbow          │
│ [▓▓▓▓▓▓▓▓▓▓] Monochrome       │
└───────────────────────────────┘
```

Diese Vorschau ist verfügbar für:

| Visualizer | Ort im ConfigPanel |
|------------|-------------------|
| **Pulsing** | Shape → Color → Preset |
| **Waveform** | Line Color → Preset |
| **Oscilloscope** | CH1-CH4 Color → Preset, M1-M2 Color → Preset |

**Tipp:** Die Vorschau zeigt den tatsächlichen Farbverlauf des Presets, 
sodass du vor dem Auswählen siehst, wie die Farben aussehen werden.

### 4.3 Checkbox verwenden

Für Boolean-Parameter:

```
☑ Clamp to 0-1    ← Aktiviert
☐ Clamp to 0-1    ← Deaktiviert
```

### 4.4 Farbwähler verwenden

Für Farb-Parameter:

1. Klicke auf den Farbbutton `[■]`
2. Wähle im Dialog eine Farbe
3. Bestätige mit **OK**

---

## 5. Wie nutze ich Presets?

### 5.1 Preset-Dropdown

Jeder Modul-Bereich hat ein Preset-Dropdown:

```
┌────────────────────────────────────────────┐
│ Smoothing                                  │
├────────────────────────────────────────────┤
│ Preset: [Balanced      ▼] [Save] [Delete]  │
│                                            │
│ Algorithm: [EMA ▼]                         │
│ Time (ms): [====●====] [50.0]              │
└────────────────────────────────────────────┘
```

### 5.2 [Custom] verstehen

Wenn du einen Parameter änderst:

1. Das Preset-Dropdown springt auf **[Custom]**
2. Dies zeigt an: "Werte entsprechen keinem Preset"
3. Du kannst jederzeit ein Preset wählen, um zurückzusetzen

### 5.3 Preset speichern

1. Stelle alle Parameter wie gewünscht ein
2. Klicke **Save**
3. Gib einen Namen ein
4. Klicke **OK**

### 5.4 Preset laden

1. Öffne das Preset-Dropdown
2. Wähle das gewünschte Preset
3. Alle Parameter werden sofort angepasst

### 5.5 Preset löschen

1. Wähle das zu löschende Preset
2. Klicke **Delete**
3. Bestätige die Löschung

**Hinweis:** Builtin-Presets (Fire, Ocean, etc.) können nicht gelöscht werden.

---

## 6. Wie funktioniert das Visibility-System?

### 6.1 Bedingte Parameter

Manche Parameter erscheinen nur unter bestimmten Bedingungen:

| Parameter | Erscheint nur wenn... |
|-----------|----------------------|
| `timeMs` | Algorithm = EMA oder DEMA |
| `windowSize` | Algorithm = SMA oder WMA |
| `solidColor` | Mode = Solid |
| `sides` | Shape = Ngon oder Star |

### 6.2 Beispiel: Smoothing

```
Algorithm: [SMA ▼]
                            ← timeMs ist VERSTECKT
Window Size: [===●===] [8]  ← windowSize ist SICHTBAR
```

```
Algorithm: [EMA ▼]

Time (ms): [===●===] [50]   ← timeMs ist SICHTBAR
                            ← windowSize ist VERSTECKT
```

### 6.3 Warum versteckte Parameter?

- **Weniger Verwirrung:** Nur relevante Parameter werden gezeigt
- **Weniger Fehler:** Man kann keine ungültigen Kombinationen erstellen
- **Bessere UX:** Fokus auf das Wesentliche

---

## 7. Wie sind die Parameter gruppiert?

### 7.1 SubGroups

Parameter werden in logische Gruppen unterteilt:

```
┌────────────────────────────────────────────┐
│ ▼ Audio                                    │
├────────────────────────────────────────────┤
│   Scale: [Log ▼]                           │
│   Bands: [64]                              │
│   Gain:  [1.0]                             │
│                                            │
│   ┌─────────────────────────────────────┐  │
│   │ Smoothing                           │  │
│   ├─────────────────────────────────────┤  │
│   │   Algorithm: [EMA ▼]                │  │
│   │   Time (ms): [50.0]                 │  │
│   └─────────────────────────────────────┘  │
│                                            │
└────────────────────────────────────────────┘
```

### 7.2 Haupt-Kategorien

| Kategorie | Inhalt |
|-----------|--------|
| **Audio** | FFT-Einstellungen, Gain, Smoothing |
| **Shape** | Form, Größe, Seiten |
| **Color** | Gradient, Farben, Outline |

### 7.3 Kollabierbare Gruppen

Klicke auf den Gruppen-Header, um sie ein-/auszuklappen:

```
▼ Audio      ← Ausgeklappt (zeigt Inhalt)
▶ Shape      ← Eingeklappt (versteckt Inhalt)
▶ Color      ← Eingeklappt
```

---

## 8. Stolpersteine und Lösungen

### 8.1 Slider reagiert nicht

**Problem:** Der Slider bewegt sich nicht beim Ziehen.

**Ursache:** Fokus liegt woanders.

**Lösung:** Klicke einmal auf den Slider, dann ziehen.

---

### 8.2 Wert springt zurück

**Problem:** Nach Eingabe springt der Wert zum alten Wert.

**Ursache:** Ungültiger Wert außerhalb des Bereichs.

**Lösung:** Prüfe Min/Max (Tooltip zeigt Bereich).

---

### 8.3 Parameter fehlt

**Problem:** Ein erwarteter Parameter ist nicht sichtbar.

**Ursachen:**
1. Visibility-Abhängigkeit nicht erfüllt
2. Parameter in eingeklappter Gruppe

**Lösung:**
1. Prüfe, ob verwandte Parameter den richtigen Wert haben
2. Klappe alle Gruppen aus

---

### 8.4 [Custom] erscheint ständig

**Problem:** Dropdown zeigt immer [Custom].

**Ursache:** Die aktuellen Werte entsprechen keinem Preset exakt.

**Lösung:** Normal - jede manuelle Änderung führt zu [Custom].

---

### 8.5 Save-Button fehlt

**Problem:** Neben dem Dropdown ist kein Save-Button.

**Ursache:** Es handelt sich um das Visualizer-Hauptpreset (nicht Modul-Preset).

**Lösung:** Visualizer-Presets werden über das Hauptmenü gespeichert.

---

## 9. Troubleshooting

### Checkliste

- [ ] Ist ein Visualizer ausgewählt?
- [ ] Ist das ConfigPanel sichtbar? (View → Config)
- [ ] Sind alle Gruppen ausgeklappt?
- [ ] Zeigt der Tooltip den erwarteten Bereich?

### Häufige Probleme

| Symptom | Lösung |
|---------|--------|
| Leeres Panel | Visualizer auswählen |
| Kein Preset-Save | Ist Modul-Preset (mit eigenem Save) |
| Parameter versteckt | Abhängigen Parameter ändern |
| Änderung ohne Effekt | Visualisierung neu starten |

### Tastaturkürzel

| Taste | Aktion |
|-------|--------|
| `Tab` | Zum nächsten Widget |
| `Enter` | Wert bestätigen |
| `Escape` | Änderung abbrechen |
| `↑`/`↓` | Wert erhöhen/verringern |

---

## 10. Siehe auch

### Benutzerhandbücher

- [Visualizer_Modules_UserGuide.md](Visualizer_Modules_UserGuide.md) — Modul-System
- [Preset_System_UserGuide.md](Preset_System_UserGuide.md) — Presets

### Referenzen

- [Parameter_Reference.md](../reference/Parameter_Reference.md) — Alle Parameter
- [API_Cheatsheet.md](../reference/API_Cheatsheet.md) — Schnellreferenz

---

## 11. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2026-01-04** | **Neu: Gradient-Vorschau in Dropdowns (Abschnitt 4.2.1)** |
| 1.0.0 | 2026-01-02 | Initial: Vollständiges Benutzerhandbuch |
