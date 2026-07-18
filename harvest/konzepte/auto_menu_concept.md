# AutoMenu & Service-Registrierung — Konzeptdokument

> Der Code in diesem Dokument ist **UTF‑8**, nutzt `#pragma once` und folgt deinem Stil (Membervariablen mit `m_`). Kommentare und Bezeichner im Code bleiben **Englisch**, aber die Erklärungen sind auf **Deutsch**. Alle Beispiele sind in **kleine Gruppen** unterteilt und jeweils direkt erläutert.

---

# 1) AutoMenu — Automatisches, hierarchisches Menüsystem

## 1.1 Überblick

AutoMenu erstellt Anwendungsmenüs auf Basis von **selbstregistrierten Metadaten**. UI-Komponenten (Panels/Dialogs) und Menü-Topologie (Items/Groups) deklarieren sich einmal, das System baut daraus einen **geordneten Baum** über einen oder mehrere **Hosts** (z. B. `MainWindow`, `DocWindow`).

**Zentrale Ideen**

- **MenuItem**: Knoten im Menübaum (Top‑Level oder Untermenü). Sortiert über `order`.
- **MenuGroup**: Abschnitt innerhalb eines MenuItems; erzeugt **Separatoren** zwischen Gruppen.
- **MenuEntry**: Panel oder Dialog, das an ein MenuItem gebunden ist (optional mit Gruppe). Sortiert über `order`.
- **MenuHost**: Gibt an, zu welcher Menüleiste der Knoten gehört (Standard: `MainWindow`).

---

## 1.2 Kern-Typen (Header‑only)

### 1.2.1 Datenstrukturen

*(Code bleibt Englisch, Erklärung auf Deutsch)*

```cpp
// ... Code wie im Original (MenuItemDef, MenuGroupDef, MenuMeta, RegistryEntry, MenuCatalog)
```

**Erklärung**

- Definiert das **Datenmodell** sowie den `MenuCatalog`, der Items, Gruppen und Entries speichert.
- `build()` erzeugt einen **baumartigen Menüaufbau**, der nach Host → Wurzelknoten → Kinder sortiert ist, inkl. gruppierter und ungruppierter Einträge.

---

### 1.2.2 Registrar & Makros

```cpp
// ... Code wie im Original (PanelRegistrar, DialogRegistrar, REGISTER_MENU_ITEM, REGISTER_PANEL usw.)
```

**Erklärung**

- Vier Makros für **MenuItems**: Top‑Level, Untermenü, anderer Host, Untermenü auf Host.
- `REGISTER_MENU_GROUP` erzeugt logische Gruppen innerhalb eines Items, die beim Rendern Separatoren erzeugen.
- Panels/Dialogs registrieren sich über **itemId** (und optional group + order).

---

## 1.3 Registrierungs-Beispiele

### 1.3.1 Top‑Level Items im MainWindow

```cpp
REGISTER_MENU_ITEM("file", "File", 10);
REGISTER_MENU_ITEM("view", "View", 20);
REGISTER_MENU_ITEM("help", "Help", 100);
```

**Erläuterung**: Drei Items im `MainWindow`, sortiert nach Order: File → View → Help.

### 1.3.2 Untermenü unter View

```cpp
REGISTER_MENU_ITEM_UNDER("view_windows", "Windows", "view", 10);
REGISTER_MENU_GROUP("view_windows", "Visuals", 10);
REGISTER_MENU_GROUP("view_windows", "Tools",   20);
```

**Erläuterung**: Fügt `View → Windows` hinzu. Darin zwei Gruppen (`Visuals`, `Tools`), mit Separator zwischen den Gruppen.

### 1.3.3 Einträge in Gruppen

```cpp
class WaveformPanel : public ui::IPanel { public: void show() override {/*...*/} };
REGISTER_PANEL_ORDER(WaveformPanel, "waveform", "Waveform", "view_windows", "Visuals", 30);

class AnalyzerPanel : public ui::IPanel { public: void show() override {/*...*/} };
REGISTER_PANEL_ORDER(AnalyzerPanel, "analyzer", "Analyzer", "view_windows", "Tools", 20);
```

**Erläuterung**: Zwei Panels in unterschiedlichen Gruppen, Reihenfolge lokal nach Order.

### 1.3.4 Ungruppierte Einträge

```cpp
class AboutDialog : public ui::IDialog { public: void open() override {/*...*/} };
REGISTER_DIALOG(AboutDialog, "about", "About…", "help", nullptr);
```

**Erläuterung**: Ein Dialog direkt unter `Help`, ohne Gruppe. Wird **nach** Gruppen gelistet.

### 1.3.5 Mehrere Hosts

```cpp
REGISTER_MENU_ITEM_ON("doc_file",   "File",   "DocWindow", 10);
REGISTER_MENU_ITEM_UNDER_ON("doc_export", "Export", "doc_file", "DocWindow", 20);
```

**Erläuterung**: Separater Menübaum für `DocWindow`.

---

## 1.4 Rendering (ImGui-Beispiel)

### 1.4.1 Callback-Mapping

```cpp
struct MenuAction { std::function<void()> onTrigger; };
using ActionMap = std::map<std::string, MenuAction>;
```

**Erläuterung**: Bindet Aktionen an `entry.id` → Aktionen sind vom UI-Aufbau entkoppelt.

### 1.4.2 ImGui Render Helper

```cpp
// ... Code RenderMenuNode und RenderMenuBarForHost wie im Original
```

**Erläuterung**: Zeichnet Gruppen mit Separator, ungruppierte Einträge danach. Kinder sind rekursiv eingebunden.

---

## 1.5 Optionale Parameter

- `iconId`: Symbol für den Eintrag
- `shortcut`: Tastenkürzel, z. B. `Ctrl+S`
- `enabled` / `visible`: Laufzeit-Steuerung
- `checkable` / `checked`: Toggle-Menüs
- `role`: Semantische Rolle (Quit, Preferences)

---

# 2) Service-Registrierung — Lebenszyklus-Management

## 2.1 Überblick

Services sind langlebige Subsysteme (Audio, Logging, Netzwerk). Sie registrieren sich selbst, werden in **deterministischer Reihenfolge** initialisiert und in **umgekehrter Reihenfolge** heruntergefahren.

> **Hinweis:** `ServiceContainer` und `ServiceRegistry` gehören besser in **Core** (nicht App), da sie UI‑agnostisch sind. `Application` nutzt sie lediglich.

---

## 2.2 Kerninterfaces & Registry

### 2.2.1 IService & AppContext

```cpp
// ... Code wie im Original (AppContext, IService)
```

**Erläuterung**: `AppContext` stellt gemeinsame Ressourcen bereit; `IService` definiert Init/Shutdown.

### 2.2.2 ServiceRegistry

```cpp
// ... Code wie im Original (ServiceRegistry, ServiceRegistrar, REGISTER_SERVICE)
```

**Erläuterung**: Services registrieren sich mit Factory und Order. Erstellung sortiert, Shutdown durch Container.

---

## 2.3 ServiceContainer

```cpp
// ... Code wie im Original (ServiceContainer)
```

**Erläuterung**: Initialisiert alle Services in Reihenfolge, Shutdown in umgekehrter Reihenfolge. Bei Exception wird zurückgerollt.

---

## 2.4 Beispiele

### 2.4.1 Zwei Services registrieren

```cpp
class LoggerService : public app::IService { /* ... */ };
REGISTER_SERVICE(LoggerService, 10);

class AudioService : public app::IService { /* ... */ };
REGISTER_SERVICE(AudioService, 20);
```

**Erläuterung**: Logger startet vor Audio. Shutdown läuft umgekehrt.

### 2.4.2 Anwendung

```cpp
class Application {
public:
    void run() {
        app::AppContext ctx;
        app::ServiceContainer container(ctx);
        try { container.initializeAll(); /* loop */ }
        catch (...) { /* Fehlerbehandlung */ }
        container.shutdownAll();
    }
};
```

**Erläuterung**: Die App verwaltet nur den Container.

---

## 2.5 Erweiterungen

- `hostId`: Services pro Kontext
- `dependencies`: Abhängigkeiten für Topo-Sortierung
- `lazyInit`: nur bei Bedarf starten
- `singleton`: nur eine Instanz
- `priorityClass`: grobe Klassifizierung (Foundation, IO, UI)

---

# 3) Projektstruktur

- **AutoMenu** → in `ui/` (Renderer z. B. in `ui/render/`).
- **Services** → in `core/`.

So bleibt **Core** unabhängig von UI.

---

# 4) Checkliste

-

---

**Fertig.** Das Dokument enthält jetzt vollständige Beispielcodes mit deutschen Erklärungen.

