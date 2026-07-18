# PlugInSystem – Architektur & Integrationsleitfaden

> **Ziel**: Dieses Dokument ist die **ultimative Anleitung** zum Einbinden und Erweitern des Systems: **ServiceContainer**, **Dialog- & Panel-Ökosystem (Registry/Manager/Base)**, **EventBus/CommandBus**, **UI‑Events (Skalierung)** und **Persistenz (Settings)**. Es richtet sich an Entwickler:innen, die dieses Projekt als **Basis** verwenden möchten.

---

## 1) High‑Level Architektur

```
Application
  ├─ ServiceContainer
  │   ├─ Logger, Settings
  │   ├─ EventBus, CommandBus
  │   ├─ DialogManager, PanelManager
  │   └─ (weitere Services: VisualizerManager, PlayerEngine …)
  ├─ WindowSystem  (GLFW/OpenGL3 + ImGui, Docking + Multi‑Viewport)
  ├─ UiController  (abonniert ui.* Events → WindowSystem APIs)
  └─ MainWindow    (Dockspace-Host) + Menu
```

**Kernideen**
- **Registries** (Dialog/Panel) sammeln **Descriptor + Factory**; Module registrieren sich **selbst** via Macro in ihrer `.cpp`.
- **Manager** (Dialog/Panel) öffnen/zeichnen/schließen Instanzen **lazy** (erst bei Bedarf).
- **Base-Klassen** kapseln `ImGui::Begin/End` und setzen gewünschte Flags (**Dialog = nicht dockbar**, **Panel = dockbar**).
- **EventBus** verbindet UI, Controller und Subsysteme lose (**typed** subscribe/publish).
- **CommandBus** bündelt imperatives Ausführen (z. B. später Player-Steuerung).
- **Settings** halten persistente Werte (z. B. `ui.scale`).

---

## 2) ServiceContainer & Initialisierung

### 2.1 Application::initialize() (vereinfachtes Muster)
```cpp
bool Application::initialize() {
  // Core
  m_Services->logger    = std::make_shared<Logger>();
  m_Services->settings  = std::make_shared<Settings>();
  m_Services->eventBus  = std::make_shared<EventBus>();
  m_Services->commandBus= std::make_shared<CommandBus>();

  // Persistenz laden
  (void)m_Services->settings->loadFromFile("settings.dat");

  // Manager
  m_Services->dialogManager = std::make_shared<DialogManager>(*m_Services);
  m_Services->panelManager  = std::make_shared<PanelManager>(*m_Services);

  // Window + ImGui
  m_WindowSystem = std::make_unique<WindowSystem>();
  if (!m_WindowSystem->initialize("PlugInSystem", 1280, 720)) return false;

  // UI-Events verdrahten (Hybrid):
  m_UiController = std::make_unique<UiController>(
      *m_Services->eventBus, *m_WindowSystem, m_Services->settings.get());

  // UI Root
  m_MainWindow = std::make_unique<MainWindow>(*m_Services);
  m_MainWindow->initialize();

  m_Running = true;
  return true;
}
```

### 2.2 Application::mainLoop()
```cpp
while (m_Running && !m_WindowSystem->shouldClose()) {
  m_WindowSystem->beginFrame();
  if (m_MainWindow) m_MainWindow->draw();        // Dockspace/Root
  if (m_Services->panelManager)  m_Services->panelManager->drawAll();
  if (m_Services->dialogManager) m_Services->dialogManager->drawAll();
  m_WindowSystem->endFrame();
}
```

### 2.3 Application::shutdown()
```cpp
m_UiController.reset();
(void)m_Services->settings->saveToFile("settings.dat");
// ... WindowSystem/MainWindow zerstören, Services freigeben
```

---

## 3) Dialoge – Muster & Verwendung

### 3.1 Interface & Basis
- **IDialog**: `virtual void draw(bool& open)`, `virtual const char* title() const`.
- **DialogBase**: Nicht‑dockbares Fenster (setzt `ImGuiWindowFlags_NoDocking`), zentralisiert `Begin/End`, du implementierst **nur** `onDraw()` und `title()`.

### 3.2 Registry & Self‑Registration
- **DialogRegistry**: `registerDialog()`, `has()`, `create()`, `descriptors()`.
- **Macro** `REGISTER_DIALOG("id", "Title", Type)` in der **.cpp** des Dialogs – sorgt dafür, dass eine Factory in die Registry eingetragen wird.

### 3.3 Manager
- **DialogManager**: `open(id)`, `close(id)`, `isOpen(id)`, `drawAll()`.
- Lazy‑Creation: Instanz wird beim ersten `open(id)` via Registry erzeugt.

### 3.4 Beispiel – About
```cpp
// AboutDialog.hpp (Ableitung von DialogBase)
class AboutDialog final : public DialogBase {
public:
  using DialogBase::DialogBase; // ctor: AboutDialog(const ServiceContainer&)
  const char* title() const noexcept override { return "About"; }
private:
  void onDraw() override { ImGui::TextUnformatted("PlugInSystem"); }
};

// AboutDialog.cpp
REGISTER_DIALOG("about", "About", AboutDialog)
```

### 3.5 Öffnen aus dem Menü
```cpp
if (m_Services.dialogManager && m_Services.dialogManager->has("about")) {
  if (ImGui::MenuItem("About...")) (void)m_Services.dialogManager->open("about");
} else {
  ImGui::MenuItem("About...", nullptr, false, false);
}
```

**Merkmale**:
- Dialoge sind **nicht dockbar**, aber dank **Viewports** außerhalb des Hauptfensters platzierbar.

---

## 4) Panels – Muster & Verwendung

### 4.1 Interface & Basis
- **IPanel**: `virtual void draw(bool& open)`, `virtual const char* title() const`.
- **PanelBase**: Dockbares Fenster (kein `NoDocking`), zentralisiert `Begin/End`; du implementierst `onDraw()`. Zugriff auf Services via `this->services()`.

### 4.2 Registry & Self‑Registration
- **PanelRegistry**: `registerPanel(PanelDescriptor{id,title,defaultVisible}, Factory)`, `has()`, `create()`, `descriptors()`.
- **Macro** `REGISTER_PANEL("id", "Title", defaultVisible, Type)` in der **.cpp**.

### 4.3 Manager
- **PanelManager**: `open(id)`, `close(id)`, `toggle(id)`, `isVisible(id)`, `descriptors()`, `drawAll()`.
- `View`‑Menü kann **automatisch** alle Panels listen (`descriptors()` + `toggle`).

### 4.4 Beispiel – SettingsPanel
```cpp
// SettingsPanel.hpp
class SettingsPanel final : public PanelBase {
public:
  explicit SettingsPanel(const ServiceContainer& svc);
  ~SettingsPanel() override;
private:
  void onDraw() override; // UI-Inhalt
  float     m_UiScale{1.0f};
  long long m_SubScaleChanged{-1};
};

// SettingsPanel.cpp (Auszug)
REGISTER_PANEL("settings", "Settings", /*defaultVisible=*/false, SettingsPanel)
```

### 4.5 Öffnen aus dem Menü
```cpp
if (m_Services.panelManager && m_Services.panelManager->has("settings")) {
  if (ImGui::MenuItem("Settings...")) (void)m_Services.panelManager->open("settings");
}
```

**Merkmale**:
- Panels sind **dockbar** (Dockspace im MainWindow) und Multi‑Viewport‑fähig.

---

## 5) EventBus – typed API & UI‑Events

### 5.1 Typed API (additiv zur bestehenden)
- **Subscribe**: `long long subscribe<T>(const std::string& topic, Fn&& callback)`
- **Unsubscribe**: `void unsubscribe(long long token)`
- **Publish**: `template<class T> void publish(const std::string& topic, const T& payload)`

**Beispiel**
```cpp
struct Foo { int value; };
long long token = eventBus.subscribe<Foo>("foo.changed", [](const Foo& f){ /*...*/ });
...
eventBus.publish<Foo>("foo.changed", Foo{42});
...
eventBus.unsubscribe(token);
```

> **Threading**: Die Standard-Implementierung ruft die Handler **synchron** auf. UI‑kritische Aktionen (ImGui/Fonts) müssen dennoch **im UI‑Thread** passieren – dafür sorgt z. B. der UiController + WindowSystem (deferred Rebuild in `beginFrame()`).

### 5.2 UI‑Skalierung per Events

**Topics & Payloads** (`UIEvents.hpp`):
- `ui.scale.request` → `ScalePayload{ float scale }`
- `ui.scale.changed` → `ScalePayload{ float scale }`

**Flow**
1) `SettingsPanel` publisht bei Slider‑Änderung `ui.scale.request`.
2) `UiController` (abonniert) ruft `WindowSystem.applyUiScale(scale)`, speichert `ui.scale` in `Settings`, publisht `ui.scale.changed`.
3) `SettingsPanel` (abonniert) spiegelt den Wert im Slider.
4) `WindowSystem::beginFrame()` sieht `m_FontsDirty` und rebuildet Fonts **vor** `ImGui::NewFrame()`.

**Snippet (Panel‑Seite)**
```cpp
// Publish
services().eventBus->publish<UIEvents::ScalePayload>(UIEvents::ScaleRequest, {m_UiScale});

// Subscribe (im Ctor)
m_SubScaleChanged = services().eventBus->subscribe<UIEvents::ScalePayload>(
  UIEvents::ScaleChanged, [this](const auto& p){ m_UiScale = p.scale; });

// Unsubscribe (im Dtor)
services().eventBus->unsubscribe(m_SubScaleChanged);
```

---

## 6) CommandBus – Konzepte & Ausblick

**Heute**
- `add(name, label, handler)` und `execute(name)` – bereits genutzt für `app.quit`.

**Morgen (optional, Audio/Engine)**
- Getypte Overloads analog EventBus:
  - `add<T>(name, handler: void(const T&))`
  - `execute<T>(name, payload)`

**Beispiel (Player)**
```cpp
// Registrierung
commandBus.add<SeekPayload>("player.seek", [](const SeekPayload& p){ engine.seek(p.seconds); });

// Auslösen
commandBus.execute<SeekPayload>("player.seek", { 42.0 });
```

**Wann Commands?**
- Für **imperative** Aktionen mit klarer Absicht (Play, Stop, Save, Export …).
- Events sind für **Broadcasts** des Zustands gedacht (position/state/volume changed …).

---

## 7) Settings – Persistenz & Erweiterbarkeit

**Minimal‑Persistenz** (bereits integriert)
- `settings.loadFromFile("settings.dat")` → liest `ui.scale` (einfaches Format)
- `settings.saveToFile("settings.dat")` → speichert `ui.scale`

**Erweiterungsideen**
- Auf **Key=Value** oder **JSON** umstellen (ohne Call‑Sites zu brechen).
- Konfig‑Pfad pro OS: `%AppData%/PlugInSystem/…`, `$XDG_CONFIG_HOME/…`, `~/Library/Application Support/…`.
- Atomisches Speichern (Temp + Rename), Auto‑Save bei Änderungen.

**Best Practice**
- Settings **lesen** beim Start (vor Controller/Manager, falls nötig).
- Settings **schreiben** beim Beenden (und optional nach wichtigen Änderungen).

---

## 8) Menü – Muster & Optionen

**Aktueller Standard**
- `View` listet **alle Panels** automatisch (`descriptors()` + `toggle`).
- Einzelne Panels zusätzlich unter thematischen Menüs (z. B. `File→Settings…`).
- Guard per `has(id)`:
```cpp
if (m_Services.panelManager->has("settings")) { /* aktiv */ } else { /* disabled */ }
```

**Optionale Erweiterungen (später)**
- `PanelDescriptor.showInView` (bool) → Feinschliff, ob ein Panel unter `View` auftaucht.
- `PanelDescriptor.menuPath` ("File/Settings") + `order` → **data‑driven** Menüs.
- **Command‑zentriert**: Menüs triggern nur noch `commandBus.execute("…")`.

---

## 9) Best Practices

- **Self‑Registration nur in .cpp** (Macro + Includes). Header schlank halten.
- **Forward‑Decls** in Headern; vollständige Typen nur dort includen, wo nötig (z. B. in `.cpp`).
- **Linux Case‑Sensitivity** beachten (Datei-/Include‑Namen!).
- **Docking‑Regeln**: Dialoge nicht dockbar, Panels dockbar. Flags in den Base‑Klassen zentral pflegen.
- **UI‑Thread**: Keine ImGui‑Calls außerhalb. Teure UI‑Operationen (Font‑Rebuild) **deferred** vor `NewFrame()`.
- **Event‑Abo**: Token **immer** im Dtor unsubscriben.
- **IDs eindeutig** wählen (pro Registry). Kollisionen vermeiden.
- **Parameter‑Schatten** vermeiden (z. B. `const ServiceContainer& svc` statt `services`). Bei Bedarf `this->services()`/`PanelBase::services()` verwenden.
- **Static Libs**: Achte darauf, dass TU mit `REGISTER_*` nicht vom Linker wegoptimiert wird (ggf. Referenz erzwingen).

---

## 10) Troubleshooting (häufige Stolperfallen)

- **„invalid use of incomplete type“ / Iterator‑Fehler**: Fehlendes Include eines vollständigen Typs in der `.cpp` (z. B. `VisualizerDescriptor`). Forward‑Decl reicht **nicht**, wenn `std::vector<T>` **instanziert** wird.
- **LNK1169 / mehrfach definiert**: Methoden sowohl in Base‑.cpp **und** nochmals in einer anderen TU implementiert (Deduplizieren!).
- **Font‑Atlas Assert**: Fonts nie mitten im Frame rebuilden; nur **vor** `ImGui::NewFrame()` (Dirty‑Flag + Rebuild in `beginFrame()`).
- **Menüpunkt grau**: Manager null oder Registry kennt `id` nicht (`has(id)` prüfen). `.cpp` mit `REGISTER_*` im Target?
- **Parameter‑Name `services`** überschattet `services()` Getter → Umbenennen (z. B. `svc`) oder `this->services()` verwenden.

---

## 11) Quick‑Start Checkliste

1. `Settings` laden, `EventBus`/`CommandBus` anlegen.
2. `DialogManager`/`PanelManager` erzeugen, `WindowSystem` initialisieren.
3. `UiController` instanziieren (UI‑Events verdrahten, `ui.scale` anwenden + broadcasten).
4. `MainWindow` erstellen, `beginFrame()`/`endFrame()` im Loop.
5. Dialog/Panel erstellen → **REGISTER_*** in `.cpp` → Menüeintrag/Hotkey → fertig.

---

## 12) Ausblick

- **Audio/Player**: Commands (play/pause/seek/next …) + Events (state/position/trackChanged …), optional `AudioController` als Hybrid wie `UiController`.
- **Visualizer**: Aktiver Visual abonniert `audio.frame`; Visualizer‑Manager schaltet Subscription beim Wechsel um.
- **Konfigurierbares Menü**: `menuPath`, `order`, `showInView`; Overrides per Settings.
- **Settings**: Migration auf JSON + strukturierte Schemas; Auto‑Save.
- **Theming**: Farbprofile, High‑DPI, Font‑Stacks.

---

**Happy hacking!** Dieses System ist dafür gebaut, **modular** zu wachsen – mit klaren Verträgen (Registry/Manager, Bus, Settings) und sauberen UI‑Abstraktionen.

