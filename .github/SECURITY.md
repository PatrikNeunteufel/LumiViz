# Sicherheitsrichtlinie

## Unterstützte Versionen

LumiViz ist in aktiver Entwicklung und hat noch keine nummerierten Releases.
Sicherheitsrelevante Korrekturen fließen in den `master`-Branch.

| Stand | Unterstützt |
|---|---|
| `master` | ✅ ja |
| ältere Commits | ❌ nein — bitte aktualisieren |

## Schwachstelle melden

**Bitte kein öffentliches Issue eröffnen.** Melde die Schwachstelle privat:

📧 **patrik.neunteufel@gmail.com**

Nützlich sind:

- eine Beschreibung der Schwachstelle
- Schritte zum Nachvollziehen (gern mit Datei, die den Fehler auslöst)
- die mögliche Auswirkung
- deine Kontaktdaten, falls du genannt werden oder eine Rückmeldung willst

Ich bestätige den Eingang innerhalb weniger Tage. Das ist ein Freizeitprojekt
eines Einzelnen — feste Reaktionszeiten kann ich nicht zusagen, aber ich melde
mich.

## Was besonders interessant ist

LumiViz **liest und führt fremde Dateien aus**. Das ist die Stelle, an der
sicherheitsrelevante Fehler am ehesten sitzen:

- **Preset-Parser** — `.avs`- und MilkDrop-Dateien werden binär bzw. textuell
  zerlegt. Manipulierte Dateien, die zu Speicherfehlern führen, sind ein Befund.
- **Skript-Ausführung** — EEL- und Lua-Skripte aus Presets laufen in einer
  abgeschotteten Lua-Umgebung. Ein Ausbruch daraus (Dateizugriff, Prozessstart,
  Laden fremder Bibliotheken) ist ein Befund.
- **Shader-Import** — Shadertoy-GLSL und ISF werden übersetzt und an den
  Grafiktreiber gegeben.
- **Datei- und Pfadbehandlung** — Playlists, Layouts, Preset-Ablagen.

## Was nicht in den Rahmen fällt

- Schwachstellen in Fremdkomponenten (BASS, Qt, Lua, Qt-ADS) — bitte direkt
  beim jeweiligen Projekt melden
- Abstürze durch beschädigte Dateien **ohne** Möglichkeit zur Codeausführung —
  dafür bitte ein normales Fehler-Issue
- Funktionswünsche und Bedienbarkeitsfragen

## Grundsätzliche Einordnung

LumiViz ist eine Desktop-Anwendung ohne Netzwerkdienst und ohne Rechteausweitung.
Der realistische Angriffsweg ist eine **präparierte Preset- oder Shader-Datei**,
die jemand herunterlädt und öffnet. Behandle fremde Presets mit derselben
Vorsicht wie fremde Dokumente.
