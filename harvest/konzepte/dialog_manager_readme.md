# DialogManager – README

> **Ziel**: So fügst du *neue Dialoge* hinzu, damit sie sich selbst registrieren, vom Menü (und überall sonst) geöffnet werden und korrekt gerendert/geschlossen werden.

---

## Architektur auf einen Blick

- **DialogRegistry**

  - Hält **Descriptors** (`DialogDescriptor { id, title }`) und **Factory-Funktionen**.
  - API (relevant):
    - `registerDialog(const DialogDescriptor&, Factory)`
    - `registerFactory(const std::string& id, Factory)` *(ergänzend)*
    - `bool has(const std::string& id) const`
    - `std::unique_ptr<IDialog> create(const std::string& id, const ServiceContainer&) const`
    - `std::vector<DialogDescriptor> descriptors() const`
  - **Factory-Signatur**: `using Factory = std::function<std::unique_ptr<IDialog>(const ServiceContainer&)>;`

- **DialogManager**

  - Öffnet/zeichnet/schließt Dialoge auf Basis der Registry.
  - Wichtige Methoden:
    - `bool open(const std::string& id)`
    - `void close(const std::string& id)`
    - `bool isOpen(const std::string& id) const`
    - `void drawAll()`
    - `void forEachOpen(fn)` *(optional für eigene Zeichenschleifen)*

- **DialogBase**

  - Basisklasse für alle Dialoge (erbt von `IDialog`).
  - Erzwingt **nicht‑dockbare** Fenster (`ImGuiWindowFlags_NoDocking`).
  - Begin/End wird zentral gehandhabt; du implementierst nur `onDraw()` und `title()`.
  - In Kombination mit **WindowSystem::ViewportsEnable** können Dialoge als **OS‑Fenster** außerhalb des Hauptfensters platziert werden.

- **IDialog**

  - Reines Interface, von `DialogBase` implementiert.
  - Signatur: `void draw(bool& open)` und `const char* title() const`.

---

## Schritt-für-Schritt: Neuen Dialog hinzufügen

### 1) Klasse anlegen

Erzeuge zwei Dateien, z. B. `src/ui/dialog/MyDialog.hpp` und `src/ui/dialog/MyDialog.cpp`.

**MyDialog.hpp** (Beispiel):

````cpp
#pragma once
#include "ui/dialog/DialogBase.hpp"
struct ServiceContainer;

class MyDialog final : public DialogBase {
public:
    using DialogBase::DialogBase;             // ctor erben: MyDialog(const ServiceContainer&)
    const char* title() const noexcept override { return "My Dialog"; }
private:
    void onDraw() override;                    // Inhalt ohne Begin/End
};
````

**MyDialog.cpp** (Beispiel + Self-Registration):

````cpp
#include "ui/dialog/MyDialog.hpp"
#include "app/ServiceContainer.hpp"
#include "ui/registry/DialogRegistry.hpp"
#include <imgui.h>

// Selbstregistrierung – sorgt dafür, dass DialogRegistry weiß,
// wie sie eine Instanz erstellt.
REGISTER_DIALOG("my_dialog", "My Dialog", MyDialog)

MyDialog::MyDialog(const ServiceContainer& services)
    : m_Services(services) {}

void MyDialog::draw(bool& open) {
    if (!ImGui::Begin(title(), &open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted("Hello from MyDialog!");
    if (ImGui::Button("Close")) open = false;

    ImGui::End();
}
````

> 🔎 **Wichtig**
>
> - `#include "ui/registry/DialogRegistry.hpp"` **muss** im `.cpp` stehen, damit `REGISTER_DIALOG(...)` sichtbar ist.
> - Die **ID** (hier `"my_dialog"`) muss **eindeutig** sein.
> - Der **Title** dient z. B. als Default-Fenstertitel / Menülabel.

### 2) Kompilieren/Linken

- Stelle sicher, dass die neue `.cpp`-Datei im **CMake-Target** enthalten ist.
- **Linux**: Pfade/Schreibweise beachten (Case-Sensitive).
- Falls du Dialog-Module später in **static libs** auslagerst: Achte darauf, dass die TU nicht vom Linker „wegoptimiert“ wird (ansonsten eine explizite Bootstrap-Funktion oder Referenz anlegen). Im aktuellen Setup (Executable) ist nichts weiter zu tun.

### 3) Öffnen aus dem Menü (oder irgendwo sonst)

- **Menü** (z. B. in `Menu::drawMenuBar()`):

```cpp
if (ImGui::MenuItem("My Dialog...")) {
    (void)m_Services.dialogManager->open("my_dialog");
}
```

- **Von einem Panel oder einer anderen Stelle**:

```cpp
// Du hast Zugriff auf ServiceContainer -> dialogManager
(void)m_Services.dialogManager->open("my_dialog");
```

> Der Dialog wird dann in `Application::mainLoop()` über `dialogManager->drawAll()` gezeichnet (bereits integriert).

---

## Best Practices & Konventionen

- **Dialoge immer von ****\`\`**** ableiten** (nicht direkt von `IDialog`).
- **Nicht dockbar**: `DialogBase` setzt automatisch `ImGuiWindowFlags_NoDocking`.
- **Außerhalb platzierbar**: Stelle sicher, dass `WindowSystem` `ImGuiConfigFlags_ViewportsEnable` setzt.
- **ID-Konvention**: `snake_case`, kurz und eindeutig, z. B. `settings`, `about`, `my_dialog`.
- **Dateinamen**: `MyDialog.hpp/.cpp` im Ordner `src/ui/dialog/`.
- **Schlanke Header**: Keine ImGui-Includes im `.hpp` von Dialogen.
- **State**: Kurzlebigen UI-State im Dialog, langlebige Daten in Services (Settings, EventBus, …).
- **Threading**: UI im UI‑Thread; Cross‑Thread über Event/CommandBus.

---

## Häufige Stolpersteine (Troubleshooting)

- **Menüpunkt ist ausgegraut**: `m_Services.dialogManager` ist nicht initialisiert. In `Application::initialize()` sicherstellen:

```cpp
m_Services->dialogManager = std::make_shared<DialogManager>(*m_Services);
```

- \`\`\*\* wirkt nicht\*\*:

  - `.cpp` nicht im Target? → In CMake ergänzen.
  - Falscher Include-Pfad/Case auf Linux? → Korrigieren.

- **Dialog öffnet, aber zeichnet nicht**: Prüfe, dass `Application::mainLoop()` `dialogManager->drawAll()` aufruft **nach** `MainWindow::draw()` und **vor** `WindowSystem::endFrame()`. Außerdem muss `WindowSystem` einen gültigen ImGui‑Context besitzen **und** Viewports aktiviert sein, wenn du Fenster außerhalb platzieren willst.

## Mini-Referenz: Code-Stellen

- **Ableitung**: `class MyDialog : public DialogBase { /* onDraw(), title() */ };`
- **Registrierung**: `REGISTER_DIALOG("id", "Title", ClassType)` im Dialog‑`.cpp`.
- **Öffnen**: `(void)dialogManager->open("id");`
- **Schließen**: `dialogManager->close("id");` oder `open=false` in `onDraw()` via Button/Logik.
- **Liste aller Dialoge**: `dialogManager->descriptors()` (aus der Registry).

---

## Beispiel: „About“

Siehe `src/ui/dialog/AboutDialog.hpp/.cpp` – registriert mit:

```cpp
REGISTER_DIALOG("about", "About", AboutDialog)
```

Ableitung von `DialogBase` (nicht dockbar), nur Inhalt in `onDraw()` implementieren. Im Menü (`Help → About…`):

```cpp
if (ImGui::MenuItem("About...")) {
    (void)m_Services.dialogManager->open("about");
}
```

Fertig. Neue Dialoge folgen exakt demselben Muster.

