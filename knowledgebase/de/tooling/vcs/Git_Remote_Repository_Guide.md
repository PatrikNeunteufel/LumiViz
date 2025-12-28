# Git Remote Repository — Guide

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  
> **English:** [Git_Remote_Repository_Guide.md](../../../en/tooling/vcs/Git_Remote_Repository_Guide.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Lokales Repository erstellen](#3-lokales-repository-erstellen)
4. [Remote Repository erstellen](#4-remote-repository-erstellen)
5. [Repositories verknüpfen](#5-repositories-verknüpfen)
6. [Safe Directory konfigurieren](#6-safe-directory-konfigurieren)
7. [Änderungen pushen](#7-änderungen-pushen)
8. [SourceTree-Workflow](#8-sourcetree-workflow)
9. [Troubleshooting](#9-troubleshooting)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

Dieser Guide beschreibt, wie ein lokales Git-Repository mit einem Remote-Repository auf einem Netzlaufwerk verbunden wird.

### Typische Struktur

```
Lokaler Arbeitsplatz              Netzwerk-Server
┌──────────────────┐              ┌──────────────────┐
│ C:\Projects\     │   push/pull  │ N:\NegalGIT\     │
│   MyProject\     │◄────────────►│   MyProject\     │
│     .git\        │              │     (bare repo)  │
└──────────────────┘              └──────────────────┘
```

---

## 2. Voraussetzungen

### Software

| Tool | Beschreibung |
|------|--------------|
| **Git** | Standalone oder via SourceTree |
| **SourceTree** | Optional, GUI für Git |

### Netzwerk

- Zugriff auf Netzlaufwerk (z.B. `N:\NegalGIT\`)
- Schreibrechte im Remote-Verzeichnis

---

## 3. Lokales Repository erstellen

### 3.1 Command Line

```bash
# In Projektverzeichnis wechseln
cd C:\Projects\MyProject

# Git initialisieren
git init
```

Das erstellt einen `.git`-Ordner mit der Repository-Struktur.

### 3.2 .gitignore erstellen

Vor dem ersten Commit eine `.gitignore` anlegen:

```gitignore
# Build-Artefakte
build/
out/
*.obj
*.exe
*.dll

# IDE-Dateien
.vs/
*.user
*.suo

# Temporäre Dateien
*.tmp
*.log
```

### 3.3 Erster Commit

```bash
# Alle Dateien stagen
git add .

# Initialer Commit
git commit -m "Initial commit"
```

---

## 4. Remote Repository erstellen

### 4.1 Bare Repository (Command Line)

Ein **Bare Repository** enthält keine Arbeitskopie — nur die Git-Datenbank.

```bash
# Ins Remote-Verzeichnis wechseln
cd N:\NegalGIT

# Projektordner erstellen
mkdir MyProject
cd MyProject

# Bare Repository initialisieren
git init --bare
```

### Struktur eines Bare Repository

```
N:\NegalGIT\MyProject\
├── HEAD
├── config
├── description
├── hooks\
├── info\
├── objects\
└── refs\
```

> **Hinweis:** SourceTree kann keine Bare Repositories erstellen. Nutze die Command Line.

---

## 5. Repositories verknüpfen

### 5.1 Remote hinzufügen

```bash
# Zurück ins lokale Repository
cd C:\Projects\MyProject

# Remote "origin" hinzufügen
git remote add origin N:\NegalGIT\MyProject
```

### 5.2 Remote verifizieren

```bash
git remote -v
# Ausgabe:
# origin  N:\NegalGIT\MyProject (fetch)
# origin  N:\NegalGIT\MyProject (push)
```

---

## 6. Safe Directory konfigurieren

Bei Netzlaufwerken muss das Verzeichnis als "sicher" markiert werden.

### 6.1 Wichtige Befehle

| Befehl | Beschreibung |
|--------|--------------|
| `git config --global --get-all safe.directory` | Alle Safe Directories anzeigen |
| `git config --global --add safe.directory <pfad>` | Directory hinzufügen |
| `git config --global --unset-all safe.directory` | Alle entfernen |
| `git config --show-origin --get-all safe.directory` | Mit Quell-Datei anzeigen |

### 6.2 Lokaler Pfad

```bash
git config --global --add safe.directory N:/NegalGIT/MyProject
```

### 6.3 UNC-Pfad (Netzwerk)

```bash
git config --global --add safe.directory //Negalserver/Mainserver/NegalGIT/MyProject
```

> **Wichtig:** Backslashes durch Forward-Slashes ersetzen!

---

## 7. Änderungen pushen

### 7.1 Erster Push

```bash
# Branch-Name prüfen
git status
# On branch master (oder main)

# Push mit Upstream-Tracking
git push -u origin master
```

### 7.2 Folgende Pushes

```bash
git push
```

---

## 8. SourceTree-Workflow

### 8.1 Repository erstellen

1. **Neuer Tab** → **Create**
2. Quellpfad auswählen (z.B. `C:\Projects\MyProject`)
3. **Erstellen** klicken
4. Bei Warnung "Ordner nicht leer" bestätigen

### 8.2 Remote hinzufügen

1. **Repository** → **Repository-Einstellungen**
2. Tab **Remotes** → **Hinzufügen**
3. Eingaben:
   - **Remote-Name:** `origin`
   - **URL/Pfad:** `N:\NegalGIT\MyProject`
   - **Standard-Remote:** ✓

### 8.3 Einschränkung

> SourceTree kann **keine Bare Repositories erstellen**. Diesen Schritt per Command Line ausführen (siehe Abschnitt 4).

---

## 9. Troubleshooting

### 9.1 "Dubious ownership" Fehler

```
fatal: detected dubious ownership in repository at 'N:/NegalGIT/...'
```

**Lösung:** Safe Directory hinzufügen (siehe Abschnitt 6).

### 9.2 Remote existiert bereits

```bash
# Remote entfernen
git remote remove origin

# Neu hinzufügen
git remote add origin N:\NegalGIT\MyProject
```

### 9.3 Push rejected

```bash
# Wenn Remote bereits Commits hat (z.B. durch anderen User)
git pull origin master --rebase
git push
```

### 9.4 Branch-Name unterschiedlich

```bash
# Lokaler Branch: main, Remote erwartet: master
git push -u origin main:master

# Oder Remote-Branch umbenennen
git branch -m master main
```

---

## 10. Siehe auch

- [Git_Standard.md](../../standards/Git_Standard.md) — Git-Konventionen
- [Git Dokumentation](https://git-scm.com/doc) — Offizielle Referenz

---

## 11. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus How_to_create_remote_Git.docx** |
