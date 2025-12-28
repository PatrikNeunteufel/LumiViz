# Git Externals: Networking & Threading — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-14  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [Git_Externals_Network.md](../../en/references/externals/Git_Externals_Network.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [HTTP / REST](#2-http--rest)
3. [Async I/O](#3-async-io)
4. [WebSocket](#4-websocket)
5. [Threading / Parallelism](#5-threading--parallelism)
6. [Schnellreferenz](#6-schnellreferenz)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Dieses Dokument beschreibt Bibliotheken für Netzwerk-Kommunikation, asynchrone I/O und Threading.

### Kategorien

| Kategorie | Bibliotheken |
|-----------|--------------|
| HTTP / REST | cpp-httplib, cpr |
| Async I/O | asio |
| WebSocket | ixwebsocket |
| Threading | taskflow, thread-pool, concurrentqueue |

---

## 2. HTTP / REST

### 2.1 cpp-httplib

> **Zweck:** Single-header HTTP/HTTPS Client und Server.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/yhirose/cpp-httplib.git` |
| **Aktueller Tag** | `v0.15.3` |
| **CMake Support** | ✅ |
| **Hook** | – |
| **Header-only** | ✅ |
| **SSL** | Optional (OpenSSL) |

```json
"cpp-httplib": {
    "git": "https://github.com/yhirose/cpp-httplib.git",
    "tag": "v0.15.3"
}
```

**Verwendung (Client):**
```cpp
#include <httplib.h>

httplib::Client cli("https://api.example.com");
auto res = cli.Get("/endpoint");
if (res && res->status == 200) {
    std::cout << res->body << std::endl;
}
```

**Verwendung (Server):**
```cpp
httplib::Server svr;
svr.Get("/hello", [](const auto& req, auto& res) {
    res.set_content("Hello World!", "text/plain");
});
svr.listen("0.0.0.0", 8080);
```

---

### 2.2 cpr (C++ Requests)

> **Zweck:** Moderner HTTP Client inspiriert von Python Requests.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/libcpr/cpr.git` |
| **Aktueller Tag** | `1.10.5` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Abhängigkeiten** | libcurl (bundled oder system) |

```json
"cpr": {
    "git": "https://github.com/libcpr/cpr.git",
    "tag": "1.10.5"
}
```

**PreFetch Hook (cpr.cmake):**
```cmake
set(CPR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CPR_BUILD_TESTS_SSL OFF CACHE BOOL "" FORCE)
set(CPR_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)  # Bundled curl
```

**Verwendung:**
```cpp
#include <cpr/cpr.h>

cpr::Response r = cpr::Get(
    cpr::Url{"https://api.example.com/data"},
    cpr::Header{{"Accept", "application/json"}}
);
std::cout << r.status_code << ": " << r.text << std::endl;

// POST mit JSON
auto r2 = cpr::Post(
    cpr::Url{"https://api.example.com/submit"},
    cpr::Body{R"({"key": "value"})"},
    cpr::Header{{"Content-Type", "application/json"}}
);
```

---

## 3. Async I/O

### 3.1 asio (Standalone)

> **Zweck:** Cross-platform asynchrone I/O Bibliothek (Basis für Boost.Asio).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/chriskohlhoff/asio.git` |
| **Aktueller Tag** | `asio-1-29-0` |
| **CMake Support** | ⚠️ (Header-only, kein native CMake) |
| **Hook** | PostFetch erforderlich 🔧 |

```json
"asio": {
    "git": "https://github.com/chriskohlhoff/asio.git",
    "tag": "asio-1-29-0",
    "cmakeSupport": false
}
```

**PostFetch Hook (asio.cmake):**
```cmake
add_library(${HOOK_EXTERNAL_NAME} INTERFACE)
target_include_directories(${HOOK_EXTERNAL_NAME} 
    INTERFACE "${HOOK_SOURCE_DIR}/asio/include"
)
target_compile_definitions(${HOOK_EXTERNAL_NAME} 
    INTERFACE ASIO_STANDALONE
)
_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
```

**Verwendung:**
```cpp
#include <asio.hpp>

asio::io_context io;
asio::steady_timer timer(io, asio::chrono::seconds(5));
timer.async_wait([](const asio::error_code& ec) {
    std::cout << "Timer expired!" << std::endl;
});
io.run();
```

---

## 4. WebSocket

### 4.1 ixwebsocket

> **Zweck:** WebSocket Client und Server mit TLS-Support.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/machinezone/IXWebSocket.git` |
| **Aktueller Tag** | `v11.4.4` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |

```json
"ixwebsocket": {
    "git": "https://github.com/machinezone/IXWebSocket.git",
    "tag": "v11.4.4"
}
```

**PreFetch Hook (ixwebsocket.cmake):**
```cmake
set(USE_TLS ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
```

**Verwendung (Client):**
```cpp
#include <ixwebsocket/IXWebSocket.h>

ix::WebSocket ws;
ws.setUrl("wss://echo.websocket.org");
ws.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        std::cout << "Received: " << msg->str << std::endl;
    }
});
ws.start();
ws.send("Hello WebSocket!");
```

---

## 5. Threading / Parallelism

### 5.1 taskflow

> **Zweck:** Parallele und heterogene Task-Programmierung mit DAG-basiertem Scheduling.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/taskflow/taskflow.git` |
| **Aktueller Tag** | `v3.6.0` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Header-only** | ✅ |

```json
"taskflow": {
    "git": "https://github.com/taskflow/taskflow.git",
    "tag": "v3.6.0"
}
```

**PreFetch Hook (taskflow.cmake):**
```cmake
set(TF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TF_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
```

**Verwendung:**
```cpp
#include <taskflow/taskflow.hpp>

tf::Executor executor;
tf::Taskflow taskflow;

auto [A, B, C, D] = taskflow.emplace(
    []() { std::cout << "Task A\n"; },
    []() { std::cout << "Task B\n"; },
    []() { std::cout << "Task C\n"; },
    []() { std::cout << "Task D\n"; }
);

A.precede(B, C);  // A runs before B and C
D.succeed(B, C);  // D runs after B and C

executor.run(taskflow).wait();
```

---

### 5.2 thread-pool

> **Zweck:** Einfacher, schneller Thread Pool.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/bshoshany/thread-pool.git` |
| **Aktueller Tag** | `v4.1.0` |
| **CMake Support** | ✅ |
| **Hook** | – |
| **Header-only** | ✅ |

```json
"thread-pool": {
    "git": "https://github.com/bshoshany/thread-pool.git",
    "tag": "v4.1.0"
}
```

**Verwendung:**
```cpp
#include <BS_thread_pool.hpp>

BS::thread_pool pool;

auto future = pool.submit([] {
    return expensive_computation();
});

int result = future.get();
```

---

### 5.3 concurrentqueue

> **Zweck:** Lock-free, high-performance Concurrent Queue.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/cameron314/concurrentqueue.git` |
| **Aktueller Tag** | `v1.0.4` |
| **CMake Support** | ✅ |
| **Hook** | – |
| **Header-only** | ✅ |

```json
"concurrentqueue": {
    "git": "https://github.com/cameron314/concurrentqueue.git",
    "tag": "v1.0.4"
}
```

**Verwendung:**
```cpp
#include <concurrentqueue.h>

moodycamel::ConcurrentQueue<int> q;

// Producer
q.enqueue(42);

// Consumer
int item;
if (q.try_dequeue(item)) {
    std::cout << item << std::endl;
}
```

---

## 6. Schnellreferenz

| Bibliothek | Tag | CMake | Hook | Hauptverwendung |
|------------|-----|-------|------|-----------------|
| cpp-httplib | v0.15.3 | ✅ | – | Simple HTTP client/server |
| cpr | 1.10.5 | ✅ | PreFetch | Modern HTTP client |
| asio | asio-1-29-0 | ⚠️ | PostFetch 🔧 | Async I/O |
| ixwebsocket | v11.4.4 | ✅ | PreFetch | WebSocket |
| taskflow | v3.6.0 | ✅ | PreFetch | Task parallelism |
| thread-pool | v4.1.0 | ✅ | – | Simple thread pool |
| concurrentqueue | v1.0.4 | ✅ | – | Lock-free queue |

---

## 7. Siehe auch

- [Git_Externals_Reference.md](Git_Externals.md) — Hauptübersicht
- [Git_Externals_Core.md](Git_Externals_Core.md) — Logging & Testing
- [Git_Externals_Data.md](Git_Externals_Data.md) — JSON & Database

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Initial: Ausgelagert aus Git_Externals_Reference** |
