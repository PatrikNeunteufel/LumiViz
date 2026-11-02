# Changelog Session 172 (2026-11-02)

> **Planungssitzung, nichts am Programm geändert.** Vorbereitet wird die Anbindung der Rechenkerne aus
> `InstrumentsCalculation` (elf Instrumentklassen) an LumiViz. Steuerdokument: `docs/audio/Instrumente_IC_Konzept.md`.

## Neu

- **Konzept `Instrumente_IC_Konzept.md` 0.2.0:** Zielbild (Patrik): zuerst Stäbe/Platten und Trommeln, dann Upright,
  Honky-tonk, CP-70, Clavinet, dann Zungen und Zupfinstrumente; ungemessen ausliefern nach Hörproben mit Kennzeichnung;
  IC bleibt Werkstatt; Plugins (eigene nach außen, fremde in LumiViz) als spätere Stufen. Vorlage IK2-0 (Form der
  Schnittstelle) steht, Antwort folgt.
- **Berichte unter `docs/modellierung/anbindung-ic/`:** Architektur-Vergleich (Dubletten, Schnittstellen aller Kerne,
  Kopplung), Recherche CLAP/VST3 (Lizenzen MIT, Marke, Offline-Modus, Host-Bausteine), Messung unter Windows-clang
  (Abdrücke bitgleich mit MSVC; Rechenzeit aller ersten Klassen unter 2,5 % der Echtzeit).

## Intern

- Tests unverändert (**1753 Cases**). IC `rechnung/` unter Windows-clang gebaut und gemessen; Antwort auf IC-A9/IC-A10
  als Datei im IC-Repo (`pruefungen/LV-2026-11-02-clang-windows-messung.md`).
