# Blueprint: Cheatsheet

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** Stable  
> **Target Audience:** Dokumentations-Autoren  
> **Language:** English  
> **German:** [Cheatsheet_Blueprint.md](../en/blueprints/Cheatsheet_Blueprint.md)

---

## 1. Overview

Dieses Blueprint definiert das Format für **Cheatsheets** — kompakte Schnellreferenzen für häufig benötigte Informationen.

### 1.1 Eigenschaften

| Eigenschaft | Wert |
|-------------|------|
| Ziel-Länge | 1-2 Seiten (gedruckt) |
| Primäres Format | Tabellen, Code-Snippets |
| Fließtext | Minimal |
| Zielgruppe | Entwickler mit Grundwissen |

### 1.2 Wann Cheatsheet verwenden?

| Situation | Cheatsheet? | Alternative |
|-----------|-------------|-------------|
| Schnellreferenz für Befehle | ✅ Yes | — |
| API-Kurzübersicht | ✅ Yes | — |
| Syntax-Reference | ✅ Yes | — |
| Concept-Erklärung | ❌ No | Concept |
| Schritt-für-Schritt Anleitung | ❌ No | UserGuide |
| Vollständige Spezifikation | ❌ No | Reference |

---

## 2. Template

### 2.1 Header

```markdown
# {Thema} — Cheatsheet

> **Version:** X.Y.Z  
> **Letzte Aktualisierung:** YYYY-MM-DD  
> **Für:** {Produkt/Tool} vX.Y  
> **Language:** English  
> **German:** [{Thema}_Cheatsheet.md](path/to/english)

---
```

### 2.2 Struktur

```markdown
## Schnellstart

{2-3 Zeilen: Importantste Info sofort}

---

## {Kategorie 1}

| {Spalte A} | {Spalte B} | {Spalte C} |
|------------|------------|------------|
| ... | ... | ... |

---

## {Kategorie 2}

### {Unterkategorie}

```{language}
{code snippet}
```

---

## Tips & Tricks

- **Tip 1:** ...
- **Tip 2:** ...

---

## See Also

- [Link 1](path) — Description
- [Link 2](path) — Description
```

---

## 3. Formatierungsregeln

### 3.1 Tabellen

Tabellen sind das Hauptelement eines Cheatsheets.

| Regel | Example |
|-------|----------|
| Kurze Zellen | `name` statt `Der Name des Elements` |
| Code in Backticks | `cmake_fatal()` |
| Keine Sätze | Stichworte |
| Max 4-5 Spalten | Lesbarkeit |

**Gut:**

| Befehl | Wirkung |
|--------|---------|
| `git add .` | Alle Changes stagen |
| `git commit -m "msg"` | Commit erstellen |

**Schlecht:**

| Befehl | Description was dieser Befehl macht |
|--------|--------------------------------------|
| `git add .` | Dieser Befehl fügt alle geänderten Dateien zum Staging-Bereich hinzu |

### 3.2 Code-Snippets

- **Kurz halten** (max 5-10 Zeilen)
- **Kommentare** nur wenn nötig
- **Syntax-Highlighting** immer angeben

```cmake
# Gut: Kurz und prägnant
cmake_fatal("E001" "Description")
cmake_warn("W001" "Description")
```

```cmake
# Schlecht: Zu ausführlich für Cheatsheet
# Diese Funktion wird verwendet um einen fatalen Error zu erzeugen
# Parameters 1: Der Errorcode im Format EXXX
# Parameters 2: Die Description des Errors
function(cmake_fatal ERROR_CODE DESCRIPTION)
    message(FATAL_ERROR "[${ERROR_CODE}] ${DESCRIPTION}")
endfunction()
```

### 3.3 Kategorien

- **Logisch gruppieren** (nicht alphabetisch)
- **Importantstes zuerst**
- **Max 6-8 Kategorien**
- **Horizontale Linien** (`---`) zwischen Kategorien

### 3.4 Visuelle Marker

| Marker | Usage |
|--------|------------|
| ✅ | Empfohlen, Success |
| ❌ | Nicht empfohlen, Error |
| ⚠️ | Warning, Vorsicht |
| 💡 | Tip |
| 📌 | Important/Merken |
| → | Ergebnis, führt zu |

---

## 4. Example: Vollständiges Cheatsheet

```markdown
# Git — Cheatsheet

> **Version:** 1.0.0  
> **Letzte Aktualisierung:** 2025-12-18  
> **Für:** Git 2.x  

---

## Schnellstart

```bash
git clone <url>      # Repository klonen
git add . && git commit -m "msg" && git push   # Changes pushen
```

---

## Basis-Befehle

| Befehl | Wirkung |
|--------|---------|
| `git init` | Neues Repository |
| `git clone <url>` | Repository klonen |
| `git status` | Status anzeigen |
| `git log --oneline` | Kompakte History |

---

## Changes

| Befehl | Wirkung |
|--------|---------|
| `git add <file>` | Datei stagen |
| `git add .` | Alles stagen |
| `git commit -m "msg"` | Commit |
| `git commit --amend` | Letzten Commit ändern |

---

## Branches

| Befehl | Wirkung |
|--------|---------|
| `git branch` | Branches auflisten |
| `git branch <name>` | Branch erstellen |
| `git checkout <name>` | Branch wechseln |
| `git checkout -b <name>` | Erstellen + Wechseln |
| `git merge <branch>` | Branch mergen |

---

## Remote

| Befehl | Wirkung |
|--------|---------|
| `git push` | Changes hochladen |
| `git pull` | Changes holen + mergen |
| `git fetch` | Changes holen (ohne merge) |

---

## Rückgängig

| Situation | Befehl |
|-----------|--------|
| Unstaged Changes verwerfen | `git checkout -- <file>` |
| Staged → Unstaged | `git reset HEAD <file>` |
| Letzten Commit rückgängig | `git reset --soft HEAD~1` |
| ⚠️ Commit löschen | `git reset --hard HEAD~1` |

---

## Tips

- 💡 `git stash` → Changes temporär speichern
- 💡 `git diff --staged` → Staged Changes anzeigen
- 📌 Vor `--hard` immer `git stash` machen!

---

## See Also

- [Git Dokumentation](https://git-scm.com/doc)
- [Pro Git Buch](https://git-scm.com/book)
```

---

## 5. Anti-Patterns

### 5.1 Zu viel Text

❌ **Schlecht:**

```markdown
## Einführung

Git ist ein verteiltes Versionskontrollsystem, das von Linus Torvalds 
entwickelt wurde. Es ermöglicht die Zusammenarbeit mehrerer Entwickler 
an einem Projekt und die Nachverfolgung aller Changes...
```

✅ **Gut:**

```markdown
## Schnellstart

`git clone` → `git add` → `git commit` → `git push`
```

### 5.2 Zu viele Details

❌ **Schlecht:**

| Flag | Langform | Description | Default | Seit Version |
|------|----------|--------------|---------|--------------|
| `-m` | `--message` | Commit-Nachricht | — | 1.0 |

✅ **Gut:**

| Befehl | Wirkung |
|--------|---------|
| `git commit -m "msg"` | Commit mit Nachricht |

### 5.3 Alphabetische Sortierung

❌ **Schlecht:** `add`, `branch`, `checkout`, `clone`, `commit`...

✅ **Gut:** Nach Workflow: `clone` → `add` → `commit` → `push`

---

## 6. Checkliste

Vor Veröffentlichung prüfen:

- [ ] Passt auf 1-2 Seiten (gedruckt)?
- [ ] Importantstes zuerst?
- [ ] Tabellen statt Fließtext?
- [ ] Code-Snippets kurz (< 10 Zeilen)?
- [ ] Kategorien logisch gruppiert?
- [ ] Keine Erklärungen, nur Fakten?
- [ ] Visuelle Marker sparsam verwendet?
- [ ] Siehe-auch Links vorhanden?

---

## 7. See Also

- [Concept_Blueprint.md](Concept_Blueprint.md) — Für Erklärungen
- [Reference_Blueprint.md](Reference_Blueprint.md) — Für vollständige Spezifikationen
- [UserGuide_Blueprint.md](UserGuide_Blueprint.md) — Für Schritt-für-Schritt Anleitungen

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.0** | **2025-12-18** | **Initial: Cheatsheet Blueprint erstellt** |
