# Doxygen — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** C/C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [Doxygen_Reference.md](../../../en/tooling/documentation/Doxygen_Reference.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Allgemeine Tags](#3-allgemeine-tags)
4. [Parameter & Rückgabewerte](#4-parameter--rückgabewerte)
5. [Struktur-Tags](#5-struktur-tags)
6. [Gruppierung](#6-gruppierung)
7. [Quellcode-Verweise](#7-quellcode-verweise)
8. [Formatierung](#8-formatierung)
9. [Kommentar-Stile](#9-kommentar-stile)
10. [Schnellreferenz](#10-schnellreferenz)
11. [Siehe auch](#11-siehe-auch)
12. [Changelog](#12-changelog)

---

## 1. Übersicht

**Doxygen** ist ein Dokumentationsgenerator für C, C++, Python und weitere Sprachen. Es extrahiert Dokumentation aus speziell formatierten Kommentaren.

### Grundprinzip

```cpp
/**
 * @brief Kurzbeschreibung der Funktion.
 * @param input Der Eingabewert.
 * @return Das berechnete Ergebnis.
 */
int calculate(int input);
```

---

## 2. Konventionen

### Notation

| Präfix | Bedeutung |
|--------|-----------|
| `@` | Doxygen-Kommando (alternativ: `\`) |
| `/**` | Mehrzeiliger Doxygen-Kommentar |
| `///` | Einzeiliger Doxygen-Kommentar |
| `///<` | Inline-Kommentar (nachgestellt) |

---

## 3. Allgemeine Tags

### 3.1 Beschreibung

| Tag | Beschreibung | Beispiel |
|-----|--------------|----------|
| `@brief` | Kurzbeschreibung | `@brief Startet die Anwendung.` |
| `@details` | Ausführliche Beschreibung | `@details Diese Methode...` |
| `@note` | Allgemeine Notiz | `@note Benötigt Admin-Rechte.` |
| `@warning` | Warnung | `@warning Nicht threadsicher!` |

### 3.2 Metadaten

| Tag | Beschreibung | Beispiel |
|-----|--------------|----------|
| `@author` | Autor | `@author Max Mustermann` |
| `@date` | Erstellungsdatum | `@date 2025-12-19` |
| `@version` | Version | `@version 1.0.0` |
| `@since` | Verfügbar seit | `@since v1.2.0` |

### 3.3 Status

| Tag | Beschreibung | Beispiel |
|-----|--------------|----------|
| `@deprecated` | Veraltet | `@deprecated Nutze newFunc()` |
| `@todo` | Offene Aufgabe | `@todo Multithreading implementieren` |
| `@bug` | Bekannter Fehler | `@bug Memory Leak bei >1000 Objekten` |

---

## 4. Parameter & Rückgabewerte

### 4.1 Funktionsparameter

```cpp
/**
 * @brief Berechnet die Summe.
 * @param a Erster Summand.
 * @param b Zweiter Summand.
 * @return Summe von a und b.
 */
int add(int a, int b);
```

### 4.2 Detaillierte Rückgabewerte

```cpp
/**
 * @brief Öffnet eine Datei.
 * @param path Dateipfad.
 * @return Datei-Handle.
 * @retval 0 Erfolgreich geöffnet.
 * @retval -1 Datei nicht gefunden.
 * @retval -2 Keine Berechtigung.
 */
int openFile(const char* path);
```

| Tag | Verwendung |
|-----|------------|
| `@param` | Einzelner Parameter |
| `@param[in]` | Eingabe-Parameter |
| `@param[out]` | Ausgabe-Parameter |
| `@param[in,out]` | Eingabe und Ausgabe |
| `@return` | Rückgabewert allgemein |
| `@retval` | Spezifischer Rückgabewert |

---

## 5. Struktur-Tags

### 5.1 Dateien und Klassen

| Tag | Beschreibung | Beispiel |
|-----|--------------|----------|
| `@file` | Dokumentiert Datei | `@file Logger.hpp` |
| `@class` | Dokumentiert Klasse | `@class Logger` |
| `@struct` | Dokumentiert Struktur | `@struct ConfigData` |
| `@union` | Dokumentiert Union | `@union DataBlock` |
| `@namespace` | Dokumentiert Namespace | `@namespace Utils` |
| `@enum` | Dokumentiert Enum | `@enum LogLevel` |
| `@typedef` | Dokumentiert Typedef | `@typedef IntList` |

### 5.2 Beispiel: Datei-Header

```cpp
/**
 * @file Logger.hpp
 * @brief Logging-System für die Anwendung.
 * @author Max Mustermann
 * @date 2025-12-19
 * @version 1.0.0
 */
```

### 5.3 Beispiel: Klasse

```cpp
/**
 * @class Logger
 * @brief Singleton-Logger für Anwendungs-Logging.
 * 
 * @details Diese Klasse implementiert einen Thread-sicheren
 * Logger mit Datei- und Konsolen-Ausgabe.
 * 
 * @note Verwende Logger::getInstance() für Zugriff.
 * @see LogLevel
 */
class Logger { /* ... */ };
```

---

## 6. Gruppierung

### 6.1 Gruppen erstellen

```cpp
/**
 * @defgroup Network Netzwerk-Funktionen
 * @brief Funktionen für Netzwerkkommunikation.
 * @{
 */

void connect();
void disconnect();
void send(const char* data);

/** @} */ // Ende der Gruppe
```

### 6.2 Zu Gruppe hinzufügen

```cpp
/**
 * @ingroup Network
 * @brief Empfängt Daten vom Server.
 */
void receive(char* buffer, size_t size);

/**
 * @addtogroup Network
 * @{
 */
void ping();
void pong();
/** @} */
```

---

## 7. Quellcode-Verweise

### 7.1 Interne Verweise

| Tag | Beschreibung | Beispiel |
|-----|--------------|----------|
| `@see` | Verweis auf andere Funktion | `@see Logger::init()` |
| `@ref` | Verweis auf Doku-Seite | `@ref NetworkModule` |
| `@link` ... `@endlink` | Hyperlink | `@link Logger @endlink` |
| `@anchor` | Sprungmarke erstellen | `@anchor MeinAnker` |

### 7.2 Dokumentation kopieren

```cpp
/**
 * @copydoc Logger::log()
 */
void logMessage(const std::string& msg);
```

---

## 8. Formatierung

### 8.1 Code-Blöcke

```cpp
/**
 * @brief Beispiel-Funktion.
 * 
 * Verwendung:
 * @code
 * int result = calculate(5, 3);
 * @endcode
 */
```

### 8.2 Listen

```cpp
/**
 * @brief Unterstützte Formate:
 * - PNG
 * - JPEG
 * - GIF
 * 
 * Schritte:
 * -# Datei öffnen
 * -# Header lesen
 * -# Daten dekodieren
 */
```

### 8.3 Tabellen

```cpp
/**
 * | Format | Kompression | Qualität |
 * |--------|-------------|----------|
 * | PNG    | Verlustfrei | Hoch     |
 * | JPEG   | Verlustbehaftet | Mittel |
 */
```

---

## 9. Kommentar-Stile

### 9.1 Mehrzeilig (Javadoc-Stil)

```cpp
/**
 * @brief Kurzbeschreibung.
 * @param x Parameter.
 * @return Ergebnis.
 */
void func(int x);
```

### 9.2 Mehrzeilig (Qt-Stil)

```cpp
/*!
 * @brief Kurzbeschreibung.
 * @param x Parameter.
 * @return Ergebnis.
 */
void func(int x);
```

### 9.3 Einzeilig

```cpp
/// @brief Kurzbeschreibung.
/// @param x Parameter.
/// @return Ergebnis.
void func(int x);
```

### 9.4 Inline (nachgestellt)

```cpp
int counter;    ///< Zählt die Anzahl der Elemente.
bool active;    ///< Ist das Objekt aktiv?

enum class LogLevel {
    Error,      ///< Fehler-Meldungen
    Warning,    ///< Warnungen
    Info        ///< Informationen
};
```

---

## 10. Schnellreferenz

### 10.1 Nach Kategorie

| Kategorie | Tags |
|-----------|------|
| **Beschreibung** | `@brief`, `@details`, `@note`, `@warning` |
| **Metadaten** | `@author`, `@date`, `@version`, `@since` |
| **Status** | `@deprecated`, `@todo`, `@bug` |
| **Parameter** | `@param`, `@param[in]`, `@param[out]`, `@return`, `@retval` |
| **Struktur** | `@file`, `@class`, `@struct`, `@namespace`, `@enum` |
| **Gruppierung** | `@defgroup`, `@ingroup`, `@addtogroup` |
| **Verweise** | `@see`, `@ref`, `@link`, `@copydoc` |
| **Formatierung** | `@code`, `@verbatim`, `@image` |

### 10.2 Template: Funktion

```cpp
/**
 * @brief [Kurzbeschreibung]
 * 
 * @details [Ausführliche Beschreibung falls nötig]
 * 
 * @param[in] param1 Beschreibung des Parameters.
 * @param[out] result Beschreibung des Ausgabeparameters.
 * 
 * @return Beschreibung des Rückgabewerts.
 * @retval 0 Erfolg.
 * @retval -1 Fehler.
 * 
 * @note Zusätzliche Hinweise.
 * @warning Wichtige Warnung.
 * 
 * @see relatedFunction()
 */
```

### 10.3 Template: Klasse

```cpp
/**
 * @class ClassName
 * @brief [Kurzbeschreibung]
 * 
 * @details [Ausführliche Beschreibung]
 * 
 * @tparam T Template-Parameter Beschreibung.
 * 
 * @note Verwendungshinweise.
 * @see RelatedClass
 */
```

---

## 11. Siehe auch

- [Cpp_Coding_Standard.md](../../standards/Cpp_Coding_Standard.md) — Coding-Konventionen
- [Doxygen Manual](https://www.doxygen.nl/manual/) — Offizielle Dokumentation

---

## 12. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus doxygen.md** |
