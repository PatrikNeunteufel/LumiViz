# Singleton Patterns — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [Singleton_Patterns_Reference.md](../../../en/patterns/design/Singleton_Patterns_Reference.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Singleton-Varianten](#3-singleton-varianten)
4. [Schnellreferenz](#4-schnellreferenz)
5. [Verwendung in Code](#5-verwendung-in-code)
6. [Anti-Patterns](#6-anti-patterns)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert verschiedene **Singleton-Implementierungen** in C++ mit ihren Eigenschaften, Vor- und Nachteilen.

### Zielgruppe

- C++ Entwickler, die globale Ressourcen verwalten müssen
- Architekten, die Thread-Sicherheit und Lifetime-Management evaluieren

### Wann Singleton?

| Anwendungsfall | Singleton geeignet? |
|----------------|---------------------|
| Logging-System | ✅ Ja |
| Konfigurationsdaten | ✅ Ja |
| Hardware-Ressourcen | ✅ Ja |
| Datenbank-Connection-Pool | ⚠️ Bedingt |
| Business-Logik | ❌ Nein |

---

## 2. Konventionen

### Notation

| Symbol | Bedeutung |
|--------|-----------|
| ✅ | Thread-sicher ab C++11 |
| ⚠️ | Besondere Vorsicht nötig |
| ❌ | Nicht empfohlen |

### Bewertungskriterien

| Kriterium | Beschreibung |
|-----------|--------------|
| **Thread-Sicherheit** | Sicher bei parallelem Zugriff |
| **Lazy Init** | Instanz erst bei erstem Zugriff |
| **Zerstörungsreihenfolge** | Kontrollierbar oder undefiniert |
| **Testbarkeit** | Austauschbar für Unit-Tests |

---

## 3. Singleton-Varianten

### 3.1 Meyers Singleton (Empfohlen)

Die Standard-Implementierung seit C++11. Nutzt thread-sichere lokale statische Variablen.

```cpp
class MeyersSingleton {
public:
    static MeyersSingleton& getInstance() {
        static MeyersSingleton instance;
        return instance;
    }
    
    void doSomething() { /* ... */ }

private:
    MeyersSingleton() = default;
    ~MeyersSingleton() = default;
    
    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;
};
```

| Aspekt | Wert |
|--------|------|
| **Thread-Sicherheit** | ✅ Garantiert ab C++11 |
| **Lazy Init** | ✅ Ja |
| **Zerstörung** | Umgekehrte Konstruktionsreihenfolge |
| **Komplexität** | Minimal |
| **Testbarkeit** | ⚠️ Schwierig |

**Anwendung:** Standard-Singleton für die meisten Fälle.

**Vorteile:**
- Einfachste Implementierung
- Keine manuelle Speicherverwaltung
- Garantiert thread-sicher (Magic Statics)

**Nachteile:**
- Schwer testbar (nicht austauschbar)
- Static Initialization Order Fiasco möglich bei Abhängigkeiten

---

### 3.2 Double-Checked Locking (DCLP)

Explizite Thread-Synchronisation mit `std::atomic` und Mutex.

```cpp
#include <mutex>
#include <atomic>
#include <memory>

class DCLPSingleton {
public:
    static DCLPSingleton& getInstance() {
        DCLPSingleton* tmp = s_instance.load(std::memory_order_acquire);
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(s_mutex);
            tmp = s_instance.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new DCLPSingleton();
                s_instance.store(tmp, std::memory_order_release);
            }
        }
        return *tmp;
    }

private:
    DCLPSingleton() = default;
    
    static std::atomic<DCLPSingleton*> s_instance;
    static std::mutex s_mutex;
};

// In .cpp
std::atomic<DCLPSingleton*> DCLPSingleton::s_instance{nullptr};
std::mutex DCLPSingleton::s_mutex;
```

| Aspekt | Wert |
|--------|------|
| **Thread-Sicherheit** | ✅ Ja (mit korrektem Memory Order) |
| **Lazy Init** | ✅ Ja |
| **Zerstörung** | ⚠️ Manuell oder Memory Leak |
| **Komplexität** | Hoch |
| **Testbarkeit** | ⚠️ Schwierig |

**Anwendung:** Legacy-Code, Plattformen ohne C++11 Magic Statics.

**Vorteile:**
- Volle Kontrolle über Synchronisation
- Funktioniert auch auf älteren Compilern

**Nachteile:**
- Komplex und fehleranfällig
- Memory Order muss korrekt sein
- Destruktor-Aufruf nicht automatisch

---

### 3.3 Eager Initialization

Instanz wird bei Programmstart erzeugt, nicht bei erstem Zugriff.

```cpp
class EagerSingleton {
public:
    static EagerSingleton& getInstance() {
        return s_instance;
    }
    
    void doSomething() { /* ... */ }

private:
    EagerSingleton() = default;
    ~EagerSingleton() = default;
    
    static EagerSingleton s_instance;
};

// In .cpp — Instanz wird bei Programmstart erzeugt
EagerSingleton EagerSingleton::s_instance;
```

| Aspekt | Wert |
|--------|------|
| **Thread-Sicherheit** | ✅ Ja (vor main()) |
| **Lazy Init** | ❌ Nein |
| **Zerstörung** | Automatisch nach main() |
| **Komplexität** | Minimal |
| **Testbarkeit** | ⚠️ Schwierig |

**Anwendung:** Wenn Initialisierungsreihenfolge wichtig ist.

**Vorteile:**
- Kein Locking zur Laufzeit
- Deterministischer Startup

**Nachteile:**
- Static Initialization Order Fiasco
- Startup-Zeit erhöht
- Ressourcen auch wenn ungenutzt

---

### 3.4 Phoenix Singleton

Kann nach Zerstörung wiedergeboren werden. Löst das "Dead Reference Problem".

```cpp
class PhoenixSingleton {
public:
    static PhoenixSingleton& getInstance() {
        if (s_destroyed) {
            new(&s_instance) PhoenixSingleton();
            s_destroyed = false;
            std::atexit(destroy);
        }
        return s_instance;
    }

private:
    PhoenixSingleton() = default;
    ~PhoenixSingleton() { s_destroyed = true; }
    
    static void destroy() {
        s_instance.~PhoenixSingleton();
        s_destroyed = true;
    }
    
    static PhoenixSingleton s_instance;
    static bool s_destroyed;
};

// In .cpp
PhoenixSingleton PhoenixSingleton::s_instance;
bool PhoenixSingleton::s_destroyed = false;
```

| Aspekt | Wert |
|--------|------|
| **Thread-Sicherheit** | ⚠️ Benötigt zusätzliches Locking |
| **Lazy Init** | ❌ Nein (initial eager) |
| **Zerstörung** | Kann wiederbelebt werden |
| **Komplexität** | Hoch |
| **Testbarkeit** | ⚠️ Schwierig |

**Anwendung:** Selten, nur bei komplexen Shutdown-Abhängigkeiten.

**Vorteile:**
- Löst Dead Reference Problem
- Robuster bei komplexen Abhängigkeiten

**Nachteile:**
- Komplex
- Unerwartetes Verhalten möglich
- Nicht thread-sicher ohne Erweiterung

---

### 3.5 Registry/Service Locator (Testbar)

Kein echtes Singleton, aber ersetzt globalen Zugriff mit testbarer Indirection.

```cpp
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& msg) = 0;
};

class LoggerRegistry {
public:
    static void setLogger(std::unique_ptr<ILogger> logger) {
        s_logger = std::move(logger);
    }
    
    static ILogger& getLogger() {
        if (!s_logger) {
            throw std::runtime_error("Logger not initialized");
        }
        return *s_logger;
    }

private:
    static std::unique_ptr<ILogger> s_logger;
};

// In .cpp
std::unique_ptr<ILogger> LoggerRegistry::s_logger;

// Produktion
LoggerRegistry::setLogger(std::make_unique<FileLogger>());

// Unit-Test
LoggerRegistry::setLogger(std::make_unique<MockLogger>());
```

| Aspekt | Wert |
|--------|------|
| **Thread-Sicherheit** | ⚠️ Setup muss vor paralleler Nutzung erfolgen |
| **Lazy Init** | ❌ Explizite Initialisierung |
| **Zerstörung** | Explizit steuerbar |
| **Komplexität** | Mittel |
| **Testbarkeit** | ✅ Hervorragend |

**Anwendung:** Testbare Architekturen, Dependency Injection.

**Vorteile:**
- Voll testbar mit Mocks
- Explizite Abhängigkeiten
- Austauschbar zur Laufzeit

**Nachteile:**
- Mehr Boilerplate
- Muss vor Verwendung initialisiert werden

---

## 4. Schnellreferenz

### 4.1 Vergleichstabelle

| Variante | Thread-Safe | Lazy | Testbar | Empfehlung |
|----------|-------------|------|---------|------------|
| **Meyers** | ✅ | ✅ | ❌ | ✅ Standard-Wahl |
| **DCLP** | ✅ | ✅ | ❌ | ⚠️ Nur Legacy |
| **Eager** | ✅ | ❌ | ❌ | ⚠️ Init-Reihenfolge |
| **Phoenix** | ⚠️ | ❌ | ❌ | ❌ Vermeiden |
| **Registry** | ⚠️ | ❌ | ✅ | ✅ Für Tests |

### 4.2 Entscheidungsbaum

```
Brauche ich Testbarkeit?
├── Ja → Registry/Service Locator
└── Nein
    └── Brauche ich Kontrolle über Init-Reihenfolge?
        ├── Ja → Eager Initialization
        └── Nein → Meyers Singleton
```

---

## 5. Verwendung in Code

### 5.1 Meyers Singleton Zugriff

```cpp
// Typische Verwendung
MeyersSingleton::getInstance().doSomething();

// Mit Alias für häufigen Zugriff
auto& singleton = MeyersSingleton::getInstance();
singleton.doSomething();
singleton.doSomethingElse();
```

### 5.2 Registry Pattern

```cpp
// main.cpp — Initialisierung
int main() {
    LoggerRegistry::setLogger(std::make_unique<ConsoleLogger>());
    
    // Rest der Anwendung
    Application app;
    return app.run();
}

// Irgendwo in der Anwendung
void someFunction() {
    LoggerRegistry::getLogger().log("Something happened");
}
```

---

## 6. Anti-Patterns

### 6.1 Singleton-Missbrauch

```cpp
// ❌ SCHLECHT — Alles ist Singleton
class UserManager : public Singleton<UserManager> { };
class OrderManager : public Singleton<OrderManager> { };
class PaymentManager : public Singleton<PaymentManager> { };

// ✅ BESSER — Dependency Injection
class Application {
    UserManager m_users;
    OrderManager m_orders;
    PaymentManager m_payments;
};
```

### 6.2 Zirkuläre Abhängigkeiten

```cpp
// ❌ GEFÄHRLICH
class A {
    void init() { B::getInstance().useA(); }
};

class B {
    void init() { A::getInstance().useB(); }  // Deadlock oder Crash
};
```

---

## 7. Siehe auch

- [PIMPL_Pattern_Concept.md](PIMPL_Pattern_Concept.md) — PIMPL für DLL-Entwicklung
- [Rule_of_Five_Concept.md](Rule_of_Five_Concept.md) — Konstruktor-Regeln

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus singleton_types_overview.md, LogManager als Meyers Singleton.md** |
