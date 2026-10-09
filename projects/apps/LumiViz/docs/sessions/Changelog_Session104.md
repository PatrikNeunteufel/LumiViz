# Changelog Session 104 (2026-10-09)

> **Am Programm hat sich nichts geändert.** Diese Session hat den Bezug der
> Sichttest-Steuerung eingerichtet: LumiViz holt beim Konfigurieren ein
> fertiges Paket und legt es neben die Exe. Benutzt wird es noch nicht.

## Neu

- **Paket der Sichttest-Steuerung** (aus dem Projekt SichtTest_Helper,
  Version 0.2.0): neben `LumiViz.exe` liegen jetzt `SichttestSteuerung1.dll`
  und der Ordner `sichttest\` mit dem Tester. LumiViz lädt die DLL noch
  nicht; der Start mit `--testing` und die Aktionen folgen.
- `sichttest.pin` im Repo-Root nennt Version, Adresse und Prüfsumme des
  Pakets — wie `cmakecraft.pin` für das Build-System.

## Geändert

- **Build-System:** CMakeCraft v0.10.0 (vorher v0.9.2). Erst diese Version
  kann fertige Pakete beziehen.
- Fehlt das Paket (kein Netz und nichts im Zwischenspeicher), baut LumiViz
  mit einer Warnung ohne es.

## Zu beachten

- Nach einem Wechsel der Paketversion die Exe neu bauen: DLL und Ordner
  werden nur beim Bau der Exe daneben kopiert.
- Release mit clang baut nur in der Umgebung von Visual Studio
  (`vcvars64.bat`); sonst scheitert das Icon der Exe ohne Meldung.
- Im Debug-Bau mit MSVC entstehen die Unit-Tests zurzeit nicht (C1128,
  `/bigobj` fehlt); Testing baut.

## Doku

- `Composer_Konzept.md` 0.70.1 — §12.2 Naht N19 (Sichttest-Steuerung,
  Bezug), §12.4 Rückbau-Liste.
- `Offene_Punkte.md` 1.126.0 — Stand der Einbindung, vier Entscheide.
