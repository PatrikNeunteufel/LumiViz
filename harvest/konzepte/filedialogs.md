# File Dialogs – Event-Only Flow (Controller + Dialogs)

Dieses Dokument beschreibt den **event-basierten** Open/Save-Workflow ohne zusätzlichen Service – exakt passend zur aktuellen Codebasis. Es ergänzt den Architektur?Leitfaden und zeigt, wie Requests aus UI/Panels via **FileDialogController** die Dialoge öffnen und wie die Dialoge ihre Konfiguration per **Open/SaveRequested** beziehen und Ergebnisse als **Selected/Canceled** publizieren.

---

## 1) Topics & Payloads (FileDialogEvents)

**Topics (Beispiel, Namen wie in deiner Codebase)**

* `file.open.requested`  ? Payload: `OpenRequest { purpose, filters, startDir, allowMulti }`
* `file.open.selected`   ? Payload: `OpenResult  { purpose, paths }`
* `file.save.requested`  ? Payload: `SaveRequest { purpose, filters, startDir, defaultName, confirmOverwrite }`
* `file.save.selected`   ? Payload: `SaveResult  { purpose, path }`
* `file.selection.canceled` ? Payload: `Canceled { purpose }`

> **Hinweis:** Die Namen deiner `inline constexpr const char*` unterscheiden sich bewusst von den Struct?Namen (z.?B. `...Requested`). Das vermeidet Namenskonflikte mit den Payload?Typen.

---

## 2) Controller: FileDialogController

**Aufgabe:** Auf `requested` hören, passenden Dialog via `DialogManager` öffnen. **Keine** Servicekopplung, **keine** Parameterweitergabe per Service.

**Ablauf:**

1. Panel/ Menü publishen z.?B. `file.open.requested` (mit Payload + `purpose`).
2. `FileDialogController` öffnet `open_file` bzw. `save_file`.
3. Der **Dialog** abonniert selbst **`requested`** und zieht sich daraus die Konfiguration.
4. Der Dialog zeigt IGFD; Ergebnis kommt als `selected`/`canceled` zurück.

**Registrierte Dialog?IDs (empfohlen):** `open_file`, `save_file`.

---

## 3) Dialoge (Open/Save)

**Key Points:**

* Beide erben von `DialogBase` (**nicht dockbar**), `REGISTER_DIALOG("open_file"|"save_file", ...)` in `.cpp`.
* **Im Ctor** (oder beim ersten `onDraw()`): Subscribe auf **`...requested`**, einmalige Übernahme der Konfiguration.
* IGFD öffnen/anzeigen; bei OK ? `...selected`, bei Abbruch ? `file.selection.canceled`.
* Keine direkten Abhängigkeiten zu Panels/Visuals.

---

## 4) Beispiel: Menü ? Open/Save requesten

```cpp
// File -> Open...
if (ImGui::MenuItem("Open...")) {
    FileDialogEvents::OpenRequest req;
    req.purpose    = "playlist";
    req.filters    = "Audio (*.mp3 *.flac){.mp3,.flac}";
    req.startDir   = ""; // optional
    req.allowMulti = true;
    services().eventBus->publish<FileDialogEvents::OpenRequest>(
        FileDialogEvents::OpenRequested, req);
}

// File -> Save As...
if (ImGui::MenuItem("Save As...")) {
    FileDialogEvents::SaveRequest req;
    req.purpose      = "visual_preset";
    req.filters      = "Preset (*.json){.json}";
    req.startDir     = "";
    req.defaultName  = "preset.json";
    req.confirmOverwrite = true;
    services().eventBus->publish<FileDialogEvents::SaveRequest>(
        FileDialogEvents::SaveRequested, req);
}
```

---

## 5) Konsumenten (Beispiele)

* **PlaylistPanel**: hört auf `file.open.selected` (mit `purpose == "playlist"`) und lädt Dateien.
* **VisualizerManager/Config**: hört auf `file.save.selected` (`purpose == "visual_preset"`) und speichert/wendet an.

**Skizze:**

```cpp
mTokOpenSel = eb.subscribe<FileDialogEvents::OpenResult>(
    FileDialogEvents::OpenSelected,
    [this](const auto& r){ if (r.purpose == "playlist") loadPlaylist(r.paths); });

mTokSaveSel = eb.subscribe<FileDialogEvents::SaveResult>(
    FileDialogEvents::SaveSelected,
    [this](const auto& r){ if (r.purpose == "visual_preset") savePresetPath(r.path); });
```

---

## 6) Best Practices

* **Lose Kopplung**: Panels/Visuals kennen keine Dialoge; alles über Events.
* **Purpose** ist der „Routing?Key“. Je Konsument nur die relevanten Payloads beachten.
* **Unsubscribe** Tokens im Destructor immer aufräumen.
* **IDs prüfen**: `DialogManager::has("open_file")/has("save_file")` – Menüeinträge ggf. disabled rendern.
* **IGFD** nur in `.cpp` includen (Dialoge), Header schlank halten.

---

## 7) Migration/Varianten

* Wenn du später doch Parameter per Service übergeben willst (z.?B. `lastDir`), kannst du **ergänzend** einen kleinen `FileDialogService` in den `ServiceContainer` aufnehmen – ohne die Event?Verträge zu brechen.
* Für volle Entkopplung ließe sich auch ein `...config.provide`/`...config.request`?Paar ergänzen. Aktuell ist es nicht nötig, weil Dialoge direkt `...requested` abonnieren.

---

## 8) Troubleshooting

* **Dialog öffnet nicht**: `DialogRegistry` hat ID nicht registriert ? `REGISTER_DIALOG` prüfen; `.cpp` im Target?
* **Nichts passiert bei OK**: Dialog publisht nicht `...selected` ? Implementierung prüfen.
* **Mehrfach öffnet**: Dialog nur einmalig IGFD öffnen; danach Display/Close korrekt handhaben.
* **Windows/Linux Pfade**: IGFD Startpfade robust setzen; bei Unklarheit leere Strings zulassen.

---

**Fazit:** Mit diesem Flow ist Open/Save vollständig **events?getrieben**, ohne extra Service. Panels lösen nur `requested` aus; Dialoge konfigurieren sich selbst aus dem Payload und liefern `selected`/`canceled` zurück. Maximale Modularität bei minimaler Kopplung.
