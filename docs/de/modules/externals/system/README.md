# externals/system — System Externals Handler

> **Version:** 1.0.0  
> **Datum:** 2025-12-20  
> **Typ:** README  
> **Status:** Aktiv  
> **Phase:** 9 (System Externals)

---

## Übersicht

Das `externals/system/` Verzeichnis enthält Module zur Verarbeitung von **System Externals** — große, extern installierte Pakete wie Qt6 oder Boost, die über `find_package()` gefunden werden.

---

## Verzeichnisstruktur

```
cmake/externals/system/
├── Handler.cmake          # Haupthandler für system: true
├── PathResolver.cmake     # Mehrstufige Pfadauflösung
└── packages/              # Paket-spezifische Hooks
    ├── Qt6.cmake          # Qt6: AUTOMOC, windeployqt
    └── Boost.cmake        # Boost: MSVC Auto-Link Fix
```

---

## Module

| Modul | Beschreibung | Dokumentation |
|-------|--------------|---------------|
| **Handler.cmake** | find_package Integration, Hook-System | [Handler_cmake.md](Handler_cmake.md) |
| **PathResolver.cmake** | ENV-Variablen, hints, backup | [PathResolver_cmake.md](PathResolver_cmake.md) |
| **packages/Qt6.cmake** | Qt6-spezifische Konfiguration | [packages/Qt6_cmake.md](packages/Qt6_cmake.md) |
| **packages/Boost.cmake** | Boost-spezifische Konfiguration | [packages/Boost_cmake.md](packages/Boost_cmake.md) |

---

## Funktionsweise

### 1. Erkennung

Ein System External wird erkannt durch `"system": true` in der JSON-Definition:

```json
"qt6": {
    "system": true,
    "package": "Qt6",
    "components": ["Core", "Widgets"]
}
```

### 2. Verarbeitung

```
Orchestrator.cmake
       │
       ├── system: true erkannt
       ▼
Handler.cmake
       │
       ├── Package-Hook laden (falls vorhanden)
       ├── PathResolver aufrufen
       ├── find_package() ausführen
       └── Als SYSTEM External registrieren
```

### 3. Anwendung

```
*Create.cmake (Executable/Library/App)
       │
       ├── apply_external_to_target()
       ▼
Handler._apply_system_external_to_target()
       │
       ├── Package::Component Targets linken
       └── _Package_configure_target() Hook aufrufen
```

---

## Eigene Package-Hooks erstellen

### Neuen Hook anlegen

```cmake
# cmake/externals/system/packages/MyPackage.cmake

include_guard(GLOBAL)

# Optional: Standard-Pfade
function(_get_MyPackage_standard_paths OUT_VAR)
    set(_paths
        "C:/MyPackage/1.0"
        "/opt/mypackage"
    )
    set(${OUT_VAR} "${_paths}" PARENT_SCOPE)
endfunction()

# Optional: Nach find_package
function(_MyPackage_post_find)
    # Post-processing
endfunction()

# Optional: Target-Konfiguration
function(_MyPackage_configure_target TARGET_NAME)
    # Target-spezifische Einstellungen
endfunction()
```

### In Solution.json verwenden

```json
"mypackage": {
    "system": true,
    "package": "MyPackage"
}
```

---

## Fehler-Codes

| Code | Beschreibung |
|------|--------------|
| E502 | `package` Feld fehlt |
| E503 | `find_package()` fehlgeschlagen |
| W501 | Backup-Pfad verwendet |

---

## Siehe auch

- [Orchestrator_cmake.md](../Orchestrator_cmake.md) — Dispatcht hierher
- [Solution_Schema.md](../../references/Solution_Schema.md) — JSON-Schema (§5.4)
- [System_Externals_Concept.md](../../projects/buildsystem/concepts/System_Externals_Concept.md) — Konzept-Dokument
