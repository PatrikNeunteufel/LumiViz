# Visual Studio Workarounds — Cheatsheet

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Cheatsheet  
> **Status:** Stabil  
> **Zielgruppe:** Windows C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [VisualStudio_Workarounds_Cheatsheet.md](../../../en/tooling/ide/VisualStudio_Workarounds_Cheatsheet.md)

---

## Typische Symptome

| Problem | Mögliche Ursache |
|---------|------------------|
| IntelliSense zeigt "Symbol nicht gefunden" | Cache veraltet |
| Methoden "existieren nicht" | Header nicht synchronisiert |
| Fehler in Vorschau, aber Build OK | IntelliSense-Datenbank korrupt |
| DLL-Änderungen nicht erkannt | IntelliSense Cache |

---

## Lösungen

### 1. IntelliSense neu starten

**Menü:** `Bearbeiten` → `IntelliSense` → `IntelliSense neu starten`

**Oder:** `Strg + Shift + P` → "Restart IntelliSense"

---

### 2. IntelliSense-Cache löschen

```
1. Visual Studio schließen
2. Ordner löschen: <Projekt>\.vs\<Projektname>\v17\ipch\
3. Visual Studio öffnen
```

---

### 3. Projektmappe bereinigen

**Menü:** `Erstellen` → `Projektmappe bereinigen`

Dann: `Erstellen` → `Projektmappe neu erstellen`

---

### 4. .vs Ordner komplett löschen

```
1. Visual Studio schließen
2. Ordner löschen: <Projekt>\.vs\
3. Visual Studio öffnen (Ordner wird neu erstellt)
```

> **Hinweis:** Lokale Einstellungen (Breakpoints, offene Tabs) gehen verloren.

---

### 5. DLL-Probleme

Bei Änderungen an DLL-Headern:

1. DLL-Projekt neu erstellen
2. Konsumierendes Projekt: `Bereinigen` → `Neu erstellen`
3. Falls nötig: VS neustarten

---

## Tastenkürzel

| Kürzel | Aktion |
|--------|--------|
| `Strg + Shift + B` | Projektmappe erstellen |
| `Strg + Alt + F7` | Alles neu erstellen |
| `F5` | Debuggen starten |
| `Strg + F5` | Ohne Debuggen starten |
| `F12` | Zur Definition springen |
| `Alt + F12` | Definition anzeigen (Peek) |
| `Strg + .` | Quick Actions |

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Aus vs workarounds.md** |
