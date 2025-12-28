# App Template

> **Version:** 0.2.1  
> **Date:** 2025-12-18  
> **Type:** Template  
> **Status:** Active  
> **German:** [README_de.md](README_de.md)

---

CMake Architecture V2 - App-Container Template

## Overview

This template provides the standard structure for App-Containers in the CMake Architecture V2 build system. It separates the entry point (`main/`) from application logic (`include/`, `src/`) for maximum testability.

## Directory Structure

```
App/
├── README.md                      # This file (English)
├── README_de.md                   # German version
├── include/                       # Public headers
│   ├── Source.cmake
│   └── Application.hpp
├── src/                           # Implementation
│   ├── Source.cmake
│   └── Application.cpp
├── main/                          # Entry point (not testable)
│   ├── Source.cmake
│   └── main.cpp
├── pch/                           # Precompiled header
│   └── pch.h
└── tests/                         # Tests
    ├── unit/
    │   └── {TestName}/            # Each test in named subdirectory
    │       ├── Source.cmake
    │       ├── test_main.cpp
    │       └── *.cpp
    ├── integration/
    │   └── {TestName}/
    │       └── ...
    ├── performance/
    │   └── {TestName}/
    │       └── ...
    ├── smoke/
    │   └── {TestName}/
    │       └── ...
    └── system/
        └── {TestName}/
            └── ...
```

## Architecture

| Directory | Responsibility | Testable |
|-----------|----------------|----------|
| `include/` + `src/` | All logic, UI, services | ✅ Yes |
| `main/` | Entry point only | ❌ No |
| `pch/` | Precompiled header | — |
| `tests/` | Test code | — |

## Usage

### 1. Copy Template

Copy this directory to your project:

```bash
cp -r projects/templates/App projects/apps/YourAppName
```

### 2. Configure Solution.json

#### Minimal Configuration

```json
"apps": [
    {
        "name": "YourAppName",
        "displayName": "Your Application",
        "version": "0.1.0",
        
        "core": {
            "dependencies": [],
            "externals": []
        },

        "runner": {
            "type": "GUI",
            "externals": []
        },

        "pch": {
            "enabled": true
        },

        "tests": {
            "framework": "doctest",
            "targets": [
                {
                    "name": "UnitTests",
                    "type": "unit",
                    "path": "tests/unit/UnitTests"
                }
            ]
        }
    }
]
```

#### Full Configuration (Multiple Tests)

```json
"apps": [
    {
        "name": "YourAppName",
        "displayName": "Your Application",
        "version": "0.1.0",
        "description": "Application description",
        
        "core": {
            "dependencies": ["SomeLibrary"],
            "externals": ["bass", "qt6"]
        },

        "runner": {
            "type": "GUI",
            "externals": ["glad", "glfw"]
        },

        "pch": {
            "enabled": true
        },

        "tests": {
            "skip": false,
            "framework": "doctest",
            "targets": [
                {
                    "name": "Core_UnitTests",
                    "type": "unit",
                    "path": "tests/unit/core",
                    "timeout": 30,
                    "labels": ["unit", "core", "fast"]
                },
                {
                    "name": "Utils_UnitTests",
                    "type": "unit",
                    "path": "tests/unit/utils",
                    "timeout": 30,
                    "labels": ["unit", "utils", "fast"]
                },
                {
                    "name": "IntegrationTests",
                    "type": "integration",
                    "path": "tests/integration/IntegrationTests",
                    "timeout": 120,
                    "labels": ["integration", "slow"],
                    "externals": ["bass"]
                },
                {
                    "name": "Benchmarks",
                    "type": "performance",
                    "path": "tests/performance/Benchmarks",
                    "timeout": 300,
                    "labels": ["performance", "nightly"],
                    "skip": true
                }
            ]
        },

        "platforms": ["windows", "linux", "macos"]
    }
]
```

### 3. Create Test Directories

For each test target, create the matching directory structure:

```bash
# For "name": "UnitTests", "path": "tests/unit/UnitTests"
mkdir -p tests/unit/UnitTests
```

Each test directory needs:
- `Source.cmake` — File list
- `test_main.cpp` — Framework entry point
- `*.cpp` — Your test files

### 4. Customize Application Class

Edit `src/Application.cpp`:
- Initialize your services in `init()`
- Implement your main loop in `run()`
- Clean up resources in `shutdown()`

## Test Configuration

### Test Target Schema

```json
{
    "name": "TestName",           // Required: Target name
    "type": "unit",               // Required: Test type
    "skip": false,                // Optional: Skip this test
    "path": "tests/unit/TestName", // Optional: Path (default: tests/{type}/{name})
    "framework": "doctest",       // Optional: Override global framework
    "timeout": 30,                // Optional: Timeout in seconds
    "labels": ["unit", "fast"],   // Optional: CTest labels
    "externals": ["bass"],        // Optional: Additional externals
    "parallel": true              // Optional: Allow parallel execution
}
```

### Known Test Types

| Type | Default Timeout | Default Parallel | Purpose |
|------|-----------------|------------------|---------|
| `unit` | 30s | ✅ true | Isolated function/class tests |
| `integration` | 120s | ✅ true | Component interaction tests |
| `performance` | 300s | ❌ false | Benchmarks, timing measurements |
| `system` | 180s | ❌ false | End-to-end tests |
| `smoke` | 10s | ✅ true | Quick "does it start?" checks |
| `fuzz` | 60s | ❌ false | Random/invalid input testing |
| `security` | 120s | ❌ false | Security vulnerability tests |
| `ui` | 180s | ❌ false | User interface tests |
| `api` | 60s | ✅ true | API endpoint tests |

Unknown types use: 60s timeout, parallel=true, label=[type]

### Framework Override

```json
"tests": {
    "framework": "doctest",           // Global default
    "targets": [
        {
            "name": "UnitTests",
            "type": "unit"
            // Uses "doctest" from global
        },
        {
            "name": "FuzzTests",
            "type": "fuzz",
            "framework": "googletest"  // Override for this test
        }
    ]
}
```

Supported frameworks: `doctest`, `googletest`, `catch2`

### Parallel Execution

Tests marked as `parallel: false` or serial-type tests run with CTest's `RUN_SERIAL` property.

⚠️ **Warning W402:** If you explicitly set `parallel: true` for types that default to serial (performance, system, fuzz, security, ui), you'll see a warning. This is allowed but may produce inaccurate results.

### Skip Feature

Tests can be temporarily disabled without removing them from configuration.

#### Global Skip (all tests)

```json
"tests": {
    "skip": true,           // Skip ALL tests for this app
    "framework": "doctest",
    "targets": [
        { "name": "UnitTests", "type": "unit" },
        { "name": "IntegrationTests", "type": "integration" }
    ]
}
```

#### Per-Target Skip

```json
"tests": {
    "framework": "doctest",
    "targets": [
        {
            "name": "UnitTests",
            "type": "unit",
            "skip": false    // Will be built
        },
        {
            "name": "SlowTests",
            "type": "integration",
            "skip": true     // Temporarily disabled
        }
    ]
}
```

#### Skip Logic

| Global `tests.skip` | Target `skip` | Result |
|---------------------|---------------|--------|
| `true` | any | ⏭️ Skipped |
| `false`/missing | `true` | ⏭️ Skipped |
| `false`/missing | `false`/missing | ✅ Built |

**Note:** Global skip takes precedence over individual target skip settings.

## Generated Targets

| Solution.json `name` | CMake Target |
|---------------------|--------------|
| `UnitTests` | `YourAppName.UnitTests` |
| `Core_UnitTests` | `YourAppName.Core_UnitTests` |
| `IntegrationTests` | `YourAppName.IntegrationTests` |

## test_main.cpp

Each test directory must contain a `test_main.cpp`:

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
```

**Important:** This define must appear exactly **once** per test executable. Do not add it to your test files!

## PCH (Precompiled Header)

### PCH Scope

- **Core (`src/`)**: Uses PCH — add `#include "pch.h"` as first line
- **Runner (`main/`)**: Does NOT use PCH
- **Tests**: Do NOT use PCH

### Enabling/Disabling PCH

```json
"pch": {
    "enabled": true,    // or false to disable
    "header": "pch.h"   // optional, default is "pch.h"
}
```

## Build Defines

| Define | Condition |
|--------|-----------|
| `APP_GUI` | `runner.type = "GUI"` (Windows only) |

## runner.type

| Type | Windows | Linux/macOS |
|------|---------|-------------|
| `GUI` | `WinMain` (no console window) | `main` |
| `CONSOLE` | `main` (with console) | `main` |

## Migration from Old Structure

If upgrading from the old `tests.unit`/`tests.integration` structure:

### Before (Old)
```json
"tests": {
    "framework": "doctest",
    "unit": { "timeout": 30 },
    "integration": { "timeout": 120 }
}
```

```
tests/unit/Application_Tests.cpp
tests/integration/Application_Integration_Tests.cpp
```

### After (New)
```json
"tests": {
    "framework": "doctest",
    "targets": [
        { "name": "UnitTests", "type": "unit", "path": "tests/unit/UnitTests" },
        { "name": "IntegrationTests", "type": "integration", "path": "tests/integration/IntegrationTests" }
    ]
}
```

```
tests/unit/UnitTests/Application_Tests.cpp
tests/integration/IntegrationTests/Application_Integration_Tests.cpp
```

## See Also

- [App Tests Targets Concept](../../docs/de/concepts/App_Tests_Targets_Concept.md)
- [Solution Schema](../../docs/de/references/Solution_Schema.md)
- [App Creation Guide](../../docs/de/userguides/App_Creation_Guide.md)

---

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| **0.2.1** | **2025-12-18** | **Added skip feature documentation (global tests.skip + per-target skip)** |
| 0.2.0 | 2025-12-18 | New tests.targets[] structure, multiple tests per type, flexible test types |
| 0.1.3 | 2025-12-18 | Added test_main.cpp for doctest |
| 0.1.2 | 2025-12-18 | Added PCH include to main.cpp |
| 0.1.1 | 2025-12-18 | Added integration/performance templates |
| 0.1.0 | 2025-12-17 | Initial template |
