# Git Standard — Versionskontroll-Richtlinien

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Standard  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Scope:** Alle Git-Repositories im Unternehmen  
> **Durchsetzung:** Pre-Commit Hooks, Code Review  
> **Language:** English  
> **German:** [Git_Standard.md](../../en/standards/Git_Standard.md)

---

## Table of Contents

1. [Zweck und Scope](#1-zweck-und-geltungsbereich)
2. [Sprach-Richtlinien](#2-sprach-richtlinien)
3. [Commit-Message-Format](#3-commit-message-format)
4. [Branch-Strategie](#4-branch-strategie)
5. [Tags und Releases](#5-tags-und-releases)
6. [Release-Notes](#6-release-notes)
7. [.gitignore Best Practices](#7-gitignore-best-practices)
8. [Git-Configuration](#8-git-konfiguration)
9. [Merge-Strategien](#9-merge-strategien)
10. [Checkliste vor Commit](#10-checkliste-vor-commit)
11. [Checkliste vor Release](#11-checkliste-vor-release)
12. [See Also](#12-siehe-auch)
13. [Changelog](#13-changelog)

---

## 1. Zweck und Scope

Dieser Standard definiert **Konventionen für Git-Versionskontrolle**:

- Commit-Message-Format
- Branch-Strategie
- Tag-/Release-Konventionen
- Sprach-Richtlinien

### Zielgruppe

Alle Entwickler, die mit Git-Repositories arbeiten. Dieser Standard ist verbindlich für alle Repositories im Unternehmen.

---

## 2. Sprach-Richtlinien

### 2.1 Overview

| Artefakt | Sprache | Begründung |
|----------|---------|------------|
| Commit-Messages | **Englisch** | Technischer Code-Kontext |
| Branch-Namen | **Englisch** | Technische Identifier |
| Tag-Namen | **Englisch** | Technische Identifier |
| Release-Notes | **Englisch** | Für Releases/Veröffentlichung |
| PR/MR-Titel | **Englisch** | Konsistenz mit Commits |
| PR/MR-Description | German oder Englisch | Team-Präferenz |

### 2.2 Begründung

- Git-History ist technischer Kontext (wie Code-Kommentare)
- Releases sind potenziell öffentlich → Englisch
- Interne Diskussionen können German sein

---

## 3. Commit-Message-Format

### 3.1 Struktur

```
<type>(<scope>): <subject>

[body]

[footer]
```

### 3.2 Type (Required)

| Type | Description | Example |
|------|--------------|----------|
| `feat` | Neues Feature | `feat(audio): add volume control` |
| `fix` | Bugfix | `fix(parser): handle empty input` |
| `refactor` | Code-Umstrukturierung (keine Funktionsänderung) | `refactor(core): extract helper class` |
| `perf` | Performance-Verbesserung | `perf(render): optimize draw calls` |
| `docs` | Dokumentation | `docs(readme): update install instructions` |
| `test` | Tests hinzufügen/ändern | `test(api): add integration tests` |
| `build` | Build-System, Dependencies | `build(cmake): upgrade to CMake 3.24` |
| `ci` | CI/CD-Configuration | `ci(github): add clang-tidy workflow` |
| `style` | Formatierung (kein Code-Change) | `style: apply clang-format` |
| `chore` | Wartung, Aufräumen | `chore: remove deprecated files` |
| `revert` | Commit rückgängig machen | `revert: feat(audio): add volume control` |
| `wip` | Work in Progress (nur lokale Branches) | `wip: experimental feature` |

### 3.3 Scope (Optional)

Der Scope beschreibt den betroffenen Bereich:

| Scope-Typ | Examples |
|-----------|-----------|
| Modul/Component | `audio`, `renderer`, `parser` |
| Layer | `core`, `api`, `ui` |
| Datei/Bereich | `cmake`, `config`, `tests` |

### 3.4 Subject (Required)

| Regel | Description |
|-------|--------------|
| Imperativ | "add feature" nicht "added feature" |
| Kleinschreibung | Kein Großbuchstabe am Anfang |
| Kein Punkt | Kein `.` am Ende |
| Max. 50 Zeichen | Kurz und prägnant |

### 3.5 Body (Optional)

- Leerzeile nach Subject
- Erklärt **Was** und **Warum** (nicht Wie)
- Max. 72 Zeichen pro Zeile
- Kann mehrere Absätze haben

### 3.6 Footer (Optional)

| Footer | Usage |
|--------|------------|
| `Fixes #123` | Issue schließen |
| `Closes #456` | Issue schließen |
| `Refs #789` | Issue referenzieren |
| `BREAKING CHANGE:` | Breaking Change dokumentieren |
| `Co-authored-by:` | Co-Autor angeben |

### 3.7 Examples

**Minimal:**
```
fix(parser): handle null pointer in parse()
```

**Mit Body:**
```
feat(audio): add real-time spectrum analyzer

Implement FFT-based spectrum analysis for audio visualization.
Uses FFTW library for efficient computation.

The analyzer supports configurable bin counts and window functions.
```

**Mit Breaking Change:**
```
refactor(api)!: change return type of process()

BREAKING CHANGE: process() now returns std::optional<Result>
instead of Result*. Callers must handle the optional.

Migration: Replace nullptr checks with has_value() checks.
```

**Bugfix mit Issue-Reference:**
```
fix(config): prevent crash on invalid JSON

Handle malformed JSON gracefully instead of throwing
unhandled exception.

Fixes #142
```

---

## 4. Branch-Strategie

### 4.1 Branch-Typen

| Branch | Pattern | Langlebig | Description |
|--------|---------|-----------|--------------|
| `main` | `main` | ✅ | Stableer Release-Stand |
| `develop` | `develop` | ✅ | Entwicklungs-Integration |
| Feature | `feature/<name>` | ❌ | Neues Feature |
| Bugfix | `fix/<name>` | ❌ | Bugfix |
| Hotfix | `hotfix/<name>` | ❌ | Dringender Fix für main |
| Release | `release/<version>` | ❌ | Release-Vorbereitung |
| Experiment | `experiment/<name>` | ❌ | Experimente (nicht mergen) |

### 4.2 Branch-Namen

| Regel | Description |
|-------|--------------|
| Kleinschreibung | `feature/audio-player` nicht `Feature/Audio-Player` |
| Kebab-Case | `fix/null-pointer` nicht `fix/null_pointer` |
| Kurz und beschreibend | `feature/export-pdf` nicht `feature/add-the-new-pdf-export-functionality` |
| Issue-Reference optional | `feature/123-audio-player` oder `feature/audio-player` |

### 4.3 Workflow

```
main ─────────────────────────────●───────────────────●─── (releases)
                                  ↑                   ↑
develop ──●───●───●───●───●───●───●───●───●───●───●───●─── (integration)
          ↑       ↑   ↑       ↑
feature/a ●───●───┘   │       │
                      │       │
feature/b     ●───●───┘       │
                              │
fix/crash         ●───────────┘
```

### 4.4 Kleine Projekte (Vereinfacht)

Für kleine Projekte ohne `develop`:

```
main ─────────────────────────────●───────────────────●───
          ↑       ↑   ↑       ↑
feature/a ●───●───┘   │       │
                      │       │
fix/bug       ●───────┘       │
                              │
feature/b         ●───────────┘
```

---

## 5. Tags und Releases

### 5.1 Tag-Format

**Semantic Versioning (SemVer):**

```
v<MAJOR>.<MINOR>.<PATCH>[-<prerelease>]
```

| Teil | Bedeutung | Wann inkrementieren |
|------|-----------|---------------------|
| MAJOR | Breaking Changes | API-Bruch, Inkompatibilität |
| MINOR | Features | Neue Funktionalität (abwärtskompatibel) |
| PATCH | Fixes | Bugfixes (abwärtskompatibel) |

### 5.2 Prerelease-Tags

| Suffix | Bedeutung | Example |
|--------|-----------|----------|
| `-alpha.N` | Frühe Entwicklung | `v1.0.0-alpha.1` |
| `-beta.N` | Feature-Complete, Testing | `v1.0.0-beta.2` |
| `-rc.N` | Release Candidate | `v1.0.0-rc.1` |

### 5.3 Example-Versionsfolge

```
v0.1.0          # Erste Pre-Release
v0.2.0          # Neues Feature
v0.2.1          # Bugfix
v1.0.0-alpha.1  # Alpha für v1
v1.0.0-beta.1   # Beta
v1.0.0-rc.1     # Release Candidate
v1.0.0          # Erstes stabiles Release
v1.0.1          # Bugfix
v1.1.0          # Neues Feature
v2.0.0          # Breaking Change
```

### 5.4 Tag erstellen

```bash
# Annotated Tag (empfohlen)
git tag -a v1.0.0 -m "Release v1.0.0"

# Mit Release-Notes
git tag -a v1.0.0 -m "Release v1.0.0

Features:
- Add audio spectrum analyzer
- Implement PDF export

Fixes:
- Fix crash on empty input (#142)
- Handle unicode filenames (#156)"

# Push Tags
git push origin v1.0.0
```

### 5.5 Wann Tags erstellen

| Situation | Tag erstellen? |
|-----------|----------------|
| Öffentliches Release | ✅ Yes |
| Interner Meilenstein | ✅ Optional (z.B. `v0.5.0-internal`) |
| Jeder Merge zu main | ❌ No |
| CI/CD Deploy | ❌ No (automatische Build-Nummern nutzen) |

---

## 6. Release-Notes

### 6.1 Struktur

```markdown
# Release v1.2.0

**Release Date:** 2025-12-05

## Highlights

Brief summary of the most important changes.

## Features

- Add audio spectrum analyzer (#123)
- Implement PDF export functionality (#145)

## Fixes

- Fix crash on empty input (#142)
- Handle unicode filenames correctly (#156)

## Breaking Changes

- `process()` now returns `std::optional<Result>` instead of `Result*`

## Dependencies

- Upgrade spdlog to v1.12.0
- Add FFTW library for spectrum analysis
```

### 6.2 Changelog-Datei

`CHANGELOG.md` im Repository-Root:

```markdown
# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
- New feature in development

## [1.2.0] - 2025-12-05

### Added
- Audio spectrum analyzer (#123)
- PDF export functionality (#145)

### Fixed
- Crash on empty input (#142)
- Unicode filename handling (#156)

### Changed
- `process()` return type (BREAKING)
```

---

## 7. .gitignore Best Practices

### 7.1 Required-Einträge

```gitignore
# Build directories
build/
out/
cmake-build-*/

# IDE
.vs/
.vscode/
.idea/
*.user
*.suo

# CMake
CMakeUserPresets.json
CMakeCache.txt
CMakeFiles/

# Compiled
*.obj
*.o
*.exe
*.dll
*.so
*.dylib
*.lib
*.a

# Dependencies (falls lokal)
.externals/
vcpkg_installed/

# backup files
*.bak
```

### 7.2 Optional (projektabhängig)

```gitignore
# Logs
*.log
logs/

# Generated
generated/

# Local config
config.local.json
```

---

## 8. Git-Configuration

### 8.1 Empfohlene Einstellungen

```bash
# Globale Einstellungen
git config --global core.autocrlf input    # Linux/macOS
git config --global core.autocrlf true     # Windows

git config --global pull.rebase true       # Rebase statt Merge bei Pull
git config --global init.defaultBranch main

# Optional: Commit-Template
git config --global commit.template ~/.gitmessage
```

### 8.2 Commit-Template (`~/.gitmessage`)

```
# <type>(<scope>): <subject>
#
# Types: feat, fix, refactor, perf, docs, test, build, ci, style, chore, revert
#
# Body: What and Why (not How)
#
# Footer: Fixes #123, BREAKING CHANGE:, Co-authored-by:
```

---

## 9. Merge-Strategien

### 9.1 Feature → develop

| Strategie | Wann |
|-----------|------|
| Squash Merge | Viele kleine Commits → 1 sauberer Commit |
| Rebase + Merge | Saubere History mit linearen Commits |
| Merge Commit | Feature-Branches nachvollziehbar halten |

### 9.2 develop → main

| Strategie | Wann |
|-----------|------|
| Merge Commit | Release-Punkte nachvollziehbar |
| Fast-Forward | Wenn develop linear zu main ist |

---

## 10. Checkliste vor Commit

- [ ] Commit-Message folgt Format (`type(scope): subject`)
- [ ] Subject im Imperativ, max. 50 Zeichen
- [ ] Code kompiliert ohne Error
- [ ] Tests laufen durch
- [ ] Keine Debug-Code/Prints vergessen
- [ ] Keine sensiblen Daten (Passwörter, Keys)
- [ ] .gitignore prüfen (keine generierten Dateien)

---

## 11. Checkliste vor Release

- [ ] Alle Features für Release gemerged
- [ ] Alle Tests grün
- [ ] CHANGELOG.md aktualisiert
- [ ] Version in relevanten Dateien aktualisiert
- [ ] Release-Notes auf Englisch
- [ ] Annotated Tag erstellt (`git tag -a vX.Y.Z`)
- [ ] Tag gepusht (`git push origin vX.Y.Z`)

---

## 12. See Also

- [CMake_Standard.md](CMake_Standard.md) — Build-System
- [Language_Standards.md](../reference/Language_Standards.md) — Sprach-Richtlinien
- [Conventional Commits](https://www.conventionalcommits.org/) — Externe Spezifikation
- [Semantic Versioning](https://semver.org/) — Versionierung
- [Keep a Changelog](https://keepachangelog.com/) — Changelog-Format

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Migration auf Blueprint v0.5: Neuer Header, Table of Contents, Encoding-Fix** |
| 0.1.0 | 2025-12-05 | Initial: Commit-Format, Branches, Tags, Release-Notes |
