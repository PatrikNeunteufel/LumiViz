# I/O Streams — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** C/C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [IO_Streams_Reference.md](../../../en/languages/shared/IO_Streams_Reference.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [C++ Streams](#2-c-streams)
3. [C I/O](#3-c-io)
4. [Embedded I/O](#4-embedded-io)
5. [RAII und Ressourcen](#5-raii-und-ressourcen)
6. [Schnellreferenz](#6-schnellreferenz)
7. [Unterschiede C vs C++](#7-unterschiede-c-vs-c)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Diese Referenz behandelt I/O-Operationen in **C** und **C++**, von High-Level Streams bis zu Low-Level Registerzugriffen.

### Abstraktionsebenen

```
┌─────────────────────────────────────────────────────┐
│ C++ Streams: std::iostream, std::fstream            │ High-Level
├─────────────────────────────────────────────────────┤
│ C Standard I/O: FILE*, printf, scanf                │
├─────────────────────────────────────────────────────┤
│ POSIX/OS API: open(), read(), write()               │
├─────────────────────────────────────────────────────┤
│ Register-Zugriff: UxTXREG, memory-mapped I/O        │ Low-Level
└─────────────────────────────────────────────────────┘
```

---

## 2. C++ Streams

### 2.1 Stream-Hierarchie

| Komponente | Beschreibung |
|------------|--------------|
| `std::istream` | Basisklasse für Eingabe (`std::cin`, `ifstream`) |
| `std::ostream` | Basisklasse für Ausgabe (`std::cout`, `ofstream`) |
| `std::iostream` | Kombination Ein-/Ausgabe |
| `std::streambuf` | Low-Level-Puffer |
| `std::filebuf` | Spezialisiert für Dateien |
| `std::stringbuf` | In-Memory-Puffer auf `std::string` |

### 2.2 Datei-Streams

```cpp
#include <fstream>
#include <string>

// Schreiben
{
    std::ofstream file("output.txt");
    if (!file) {
        throw std::runtime_error("Cannot open file");
    }
    file << "Hello, World!" << std::endl;
}  // Automatisch geschlossen (RAII)

// Lesen
{
    std::ifstream file("input.txt");
    std::string line;
    while (std::getline(file, line)) {
        // Verarbeite line
    }
}
```

### 2.3 String-Streams

```cpp
#include <sstream>

// Formatierung
std::ostringstream oss;
oss << "Value: " << 42 << ", Pi: " << 3.14159;
std::string result = oss.str();

// Parsing
std::istringstream iss("100 200 300");
int a, b, c;
iss >> a >> b >> c;
```

### 2.4 Modi

| Modus | Bedeutung |
|-------|-----------|
| `std::ios::in` | Lesen |
| `std::ios::out` | Schreiben |
| `std::ios::app` | Anhängen |
| `std::ios::ate` | Am Ende positionieren |
| `std::ios::trunc` | Datei leeren |
| `std::ios::binary` | Binärmodus |

```cpp
std::fstream file("data.bin", std::ios::in | std::ios::out | std::ios::binary);
```

---

## 3. C I/O

### 3.1 Standard-Funktionen

| Funktion | Beschreibung |
|----------|--------------|
| `fopen()` | Datei öffnen |
| `fclose()` | Datei schließen |
| `fread()` | Binär lesen |
| `fwrite()` | Binär schreiben |
| `fprintf()` | Formatierte Ausgabe |
| `fscanf()` | Formatierte Eingabe |
| `fgets()` | Zeile lesen |
| `fputs()` | String schreiben |

### 3.2 Beispiel

```c
#include <stdio.h>

FILE* file = fopen("data.txt", "w");
if (file == NULL) {
    perror("Error opening file");
    return -1;
}

fprintf(file, "Value: %d\n", 42);
fclose(file);
```

### 3.3 Fehlerbehandlung

```c
FILE* file = fopen("data.txt", "r");
if (!file) {
    perror("fopen failed");
    return -1;
}

char buffer[256];
if (fgets(buffer, sizeof(buffer), file) == NULL) {
    if (feof(file)) {
        // Ende der Datei
    } else if (ferror(file)) {
        // Lesefehler
    }
}

fclose(file);
```

---

## 4. Embedded I/O

### 4.1 Register-basiert (Microchip dsPIC)

```c
// UART Senden — dsPIC33CH
U1TXREG = 0x55;                  // Byte senden
while (!U1STAbits.TRMT);         // Warten bis fertig

// UART Empfangen
while (!U1STAbits.URXDA);        // Warten auf Daten
uint8_t data = U1RXREG;          // Byte lesen
```

### 4.2 Register-basiert (TI C2000)

```c
// SCI (UART) — TMS320F28P65x
SciaRegs.SCITXBUF.all = 'A';     // Zeichen senden
while (SciaRegs.SCICTL2.bit.TXEMPTY == 0);

// Mit DriverLib
SCI_writeCharBlockingFIFO(SCIA_BASE, 'A');
while (!SCI_isTransmitterReady(SCIA_BASE));
```

### 4.3 Abstrahierte Schnittstelle

```c
// Portable HAL-Schicht
typedef struct {
    void (*send)(uint8_t byte);
    uint8_t (*receive)(void);
    bool (*is_ready)(void);
} UART_Interface;

// Plattform-spezifische Implementierung
void uart_send_dspic(uint8_t byte) {
    U1TXREG = byte;
    while (!U1STAbits.TRMT);
}

void uart_send_c2000(uint8_t byte) {
    SCI_writeCharBlockingFIFO(SCIA_BASE, byte);
}
```

---

## 5. RAII und Ressourcen

### 5.1 C++ RAII-Pattern

```cpp
class FileHandle {
    std::FILE* m_file;
    
public:
    explicit FileHandle(const char* path, const char* mode)
        : m_file(std::fopen(path, mode)) {
        if (!m_file) {
            throw std::runtime_error("Cannot open file");
        }
    }
    
    ~FileHandle() {
        if (m_file) std::fclose(m_file);
    }
    
    // Move-only
    FileHandle(FileHandle&& other) noexcept 
        : m_file(std::exchange(other.m_file, nullptr)) {}
    
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    operator std::FILE*() { return m_file; }
};

// Verwendung
{
    FileHandle file("data.txt", "r");
    std::fprintf(file, "Hello");
}  // Automatisch geschlossen
```

### 5.2 std::filesystem (C++17)

```cpp
#include <filesystem>
namespace fs = std::filesystem;

// Pfad-Operationen
fs::path p = "C:/Users/user/file.txt";
p.filename();       // "file.txt"
p.extension();      // ".txt"
p.parent_path();    // "C:/Users/user"

// Datei-Operationen
if (fs::exists(p)) {
    auto size = fs::file_size(p);
    fs::copy(p, "backup.txt");
}

// Verzeichnis durchlaufen
for (const auto& entry : fs::directory_iterator(".")) {
    std::cout << entry.path() << '\n';
}
```

### 5.3 std::filesystem::path Übergabe

```cpp
// ✅ Korrekt — const reference (path kann groß sein)
void processFile(const fs::path& path);

// ❌ Vermeiden — kopiert den String
void processFile(fs::path path);  // Nur wenn Ownership nötig
```

---

## 6. Schnellreferenz

### 6.1 C++ Stream-Klassen

| Klasse | Include | Zweck |
|--------|---------|-------|
| `std::ifstream` | `<fstream>` | Datei lesen |
| `std::ofstream` | `<fstream>` | Datei schreiben |
| `std::fstream` | `<fstream>` | Datei lesen/schreiben |
| `std::istringstream` | `<sstream>` | String parsen |
| `std::ostringstream` | `<sstream>` | String formatieren |
| `std::stringstream` | `<sstream>` | Beides |

### 6.2 C Datei-Funktionen

| Funktion | Signatur |
|----------|----------|
| `fopen` | `FILE* fopen(const char* path, const char* mode)` |
| `fclose` | `int fclose(FILE* stream)` |
| `fread` | `size_t fread(void* ptr, size_t size, size_t count, FILE* stream)` |
| `fwrite` | `size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)` |
| `fprintf` | `int fprintf(FILE* stream, const char* format, ...)` |
| `fscanf` | `int fscanf(FILE* stream, const char* format, ...)` |

### 6.3 Datei-Modi

| Modus | C | C++ |
|-------|---|-----|
| Lesen | `"r"` | `ios::in` |
| Schreiben (neu) | `"w"` | `ios::out \| ios::trunc` |
| Anhängen | `"a"` | `ios::app` |
| Lesen + Schreiben | `"r+"` | `ios::in \| ios::out` |
| Binär | `"rb"`, `"wb"` | `ios::binary` |

---

## 7. Unterschiede C vs C++

| Aspekt | C | C++ |
|--------|---|-----|
| **Ressourcenverwaltung** | Manuell (`fclose`) | RAII (automatisch) |
| **Fehlerbehandlung** | Return-Codes, `errno` | Exceptions (optional) |
| **Typsicherheit** | Gering (`void*`) | Hoch (Templates) |
| **Formatierung** | `printf`-Syntax | Operator `<<`, `std::format` |
| **Performance** | Oft schneller | Sync mit C I/O deaktivierbar |
| **Erweiterbarkeit** | Gering | Eigene Streambuf möglich |

### Sync deaktivieren (Performance)

```cpp
// Am Programmanfang
std::ios_base::sync_with_stdio(false);
std::cin.tie(nullptr);
```

> **Warnung:** Danach C und C++ I/O nicht mischen!

---

## 8. Siehe auch

- [C_IO_Embedded_Reference.md](../c/C_IO_Embedded_Reference.md) — Detaillierte Embedded-I/O
- [Cpp_Streams_Guide.md](../cpp/Cpp_Streams_Guide.md) — Fortgeschrittene C++ Streams
- [EPWM_Configuration_Reference.md](../../embedded/ti-c2000/EPWM_Configuration_Reference.md) — TI C2000 Peripherals

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus streams.md, streams filesystem raii.md, io in c.md** |
