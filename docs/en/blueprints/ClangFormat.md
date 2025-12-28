# ClangFormat Blueprint – Code-Formatierung

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development (Pre-Release)  
> **Scope:** Alle `.clang-format`-Configurationen  
> **Bezug:** Cpp_Coding_Standard v0.1, C_Coding_Standard v0.1  
> **Language:** English  

---

## 1. Zweck

Dieses Blueprint definiert **Struktur und Wartung** der `.clang-format`-Configuration.

**Was dieses Dokument ist:**
- Conceptuelle Grundlage für Formatierungsregeln
- Definition von Verantwortlichkeiten und Änderungsprozessen
- Erklärung der Intentionen hinter den Regeln

**Was dieses Dokument nicht ist:**
- Die technische Configuration selbst (→ `.clang-format`-Datei)

---

## 2. Verhältnis zu anderen Standards

| Dokument | Definiert |
|----------|-----------|
| **Cpp_Coding_Standard** | Stil, Sprachfeatures, Design-Regeln |
| **C_Coding_Standard** | Stil für Embedded-C |
| **`.clang-format`** | Technische Umsetzung der Formatierung |

### Konfliktauflösung

> Bei Widersprüchen zwischen textlicher Dokumentation und `.clang-format` gilt **`.clang-format`** als verbindlich. Die Dokumentation wird nachgezogen, nicht das Tool umgangen.

---

## 3. Basis-Stil und Philosophie

### 3.1 Grundlagen

| Aspekt | Wert |
|--------|------|
| Basis-Stil | `LLVM` mit projektspezifischen Anpassungen |
| Ziele | Lesbarkeit, stabile Diffs, minimale Stil-Diskussionen |

### 3.2 Verantwortlichkeit

- Die `.clang-format`-Datei im Root-Verzeichnis wird zentral gepflegt
- Projektspezifische Overrides sind zu vermeiden
- Changes folgen dem Prozess in Abschnitt 7

---

## 4. Formatierungsrichtlinien (Concept)

> Die exakten Werte sind in `.clang-format` definiert. Dieser Abschnitt dokumentiert die **Intention**.

### 4.1 Einrückung und Whitespace

| Einstellung | Wert | Begründung |
|-------------|------|------------|
| IndentWidth | 4 | Kompromiss zwischen Dichte und Lesbarkeit |
| TabWidth | 4 | Konsistenz |
| UseTab | Never | Vermeidet Alignment-Probleme |

### 4.2 Klammern (Allman-Stil)

Öffnende Klammern stehen auf **neuer Zeile** für:
- Functions
- Klassen/Structs
- Namespaces
- Kontrollstrukturen (`if`, `for`, `while`, `switch`)

**Example:**
```cpp
if (condition)
{
    doSomething();
}
else
{
    handleElse();
}
```

**Begründung:** Klare visuelle Struktur, einfacheres Diffing.

### 4.3 Zeilenlänge

| Einstellung | Wert |
|-------------|------|
| ColumnLimit | 80 (oder 120, projektabhängig) |

**Begründung:** Vermeidet horizontales Scrollen, bessere Side-by-Side-Diffs.

### 4.4 Include-Sortierung

**Reihenfolge:**
1. Precompiled Header (`pch/pch.h`) – falls vorhanden
2. System-Header (`<...>`)
3. Projekt-Header (`"..."`)

**Einstellung:** `IncludeBlocks: Regroup`

**Begründung:** PCH muss zuerst, klare Trennung System/Projekt.

### 4.5 Arrays und Initialisierungslisten

**Stil:** Ein Element pro Zeile bei mehrzeiligen Arrays.

```cpp
int values[] =
{
    1,
    2,
    4,
    8,
};
```

**Begründung:** Ein geändertes Element = eine geänderte Zeile im Diff.

---

## 5. Projektspezifische Erweiterungen

Erlaubt:
- `Language`-spezifische Sektionen (C vs. C++)
- Embedded-Anpassungen bei Toolchain-Einschränkungen

Anforderungen:
- Dokumentation im Projekt-README
- Keine grundlosen Abweichungen vom globalen Stil

---

## 6. Tool-Integration

### 6.1 Entwickler-Workflow

1. `clang-format` in IDE/Editor integrieren
2. Code **vor** Commit formatieren
3. Code-Reviews ignorieren Formatierung (delegiert an Tool)

### 6.2 Automatisierung

| Methode | Description |
|---------|--------------|
| Pre-Commit Hook | Automatische Formatierung vor Commit |
| CI-Check | Validierung im Build-System |
| IDE-Integration | Format-on-Save |

---

## 7. Änderungsmanagement

### 7.1 Wann ändern?

Changes nur wenn:
- Aktuelle Regel schadet nachweislich der Lesbarkeit
- Neue Regel verbessert Konsistenz signifikant
- Tooling/Projekt erfordert Anpassung

### 7.2 Prozess

1. **Vorschlag:** Aktuelles Verhalten (vorher/nachher), Motivation
2. **Review:** Architecture-/Tooling-Team
3. **Umsetzung:**
   - `.clang-format` aktualisieren
   - Formatierungs-Pass auf betroffenen Code
   - Bei konzeptueller Änderung: Blueprint aktualisieren

---

## 8. Versionierung

### 8.1 Datei-Header

Jede `.clang-format`-Datei enthält:

```yaml
# C/C++ Formatting Standard
# Version: 0.1.0
# Blueprint: Documentations/Blueprints/ClangFormat_Blueprint_v0_1_0.md
# Date: 2025-12-05
```

### 8.2 Versionierungsregeln

| Änderung | Aktion |
|----------|--------|
| Conceptuell (Klammer-Stil, Einrückung) | Version erhöhen, Blueprint aktualisieren |
| Technisch/Kosmetisch | Version kann bleiben |

---

## 9. See Also

- [Cpp_Coding_Standard](Cpp_Coding_Standard_v0_1_0.md) – C++ Stil-Richtlinien
- [C_Coding_Standard](C_Coding_Standard_v0_1_0.md) – C Stil-Richtlinien (Embedded)
- [ClangTidy_Blueprint](ClangTidy_Blueprint_v0_1_0.md) – Statische Analyse

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.0** | **2025-12-05** | **Initial: Struktur, Philosophie, Änderungsprozess** |
