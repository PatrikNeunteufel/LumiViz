# JsonPresetParser — Gemeinsamer Preset-JSON-Extraktor

> **Version:** 1.0.0  
> **Datum:** 2026-07-19  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Visualizers::Modules::JsonPresetParser  
> **Dateien:** JsonPresetParser.hpp (header-only)  
> **Namespace:** lumi::modules  
> **Abhängigkeiten:** C++20 STL (fstream/optional)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## 1. Übersicht

Ersetzt die drei fast identischen Hand-Parser der Modul-Presets
(AudioSourceModule `.audio`, SmoothingModule `.smooth`,
ColorGradientModule `.grad` — Phase 4 Schritt 5.6). Bewusst **kein**
allgemeiner JSON-Parser: er deckt das bekannte, flache Preset-Format ab, das
die Module selbst schreiben (find-basierte Extraktion, Semantik 1:1 wie die
früheren Lambdas).

## 2. API

```cpp
auto parser = lumi::modules::JsonPresetParser::fromFile(path);  // nullopt wenn nicht lesbar
if (!parser) return false;

std::string name = parser->getString("name");          // "" bzw. Fallback
int algo   = parser->getInt("algorithm");              // Fallback-Parameter optional
float ms   = parser->getFloat("timeMs");
bool prime = parser->getBool("primeFirstFrame");

// Array-Rohtext ohne äußere Klammern; verschachtelte Arrays per Klammerzählung
std::string stops = parser->getArrayContent("stops");  // "[0,1,0,0,1], [1,0,0,1,1]"
```

Wert-Interpretation der Arrays (Gradient-Stops/Midpoints) bleibt beim
jeweiligen Modul — der Parser liefert nur den Rohtext.

## 3. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 1.0.0 | 2026-07-19 | Initial (Phase 4 Schritt 5.6): getString/Int/Float/Bool + getArrayContent; ersetzt 3 Mini-Parser |
