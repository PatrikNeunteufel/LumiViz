# Changelog Session 105 (2026-10-09)

> **Der Tester steuert LumiViz.** Mit dem Schalter `--testing` lädt LumiViz die
> Sichttest-Steuerung und verbindet sich mit dem Tester `Sichttest.exe`; der
> führt die Handgriffe unter *Vorbereiten* einer Sichttest-Liste selbst aus.
> Ein gewöhnlicher Start lädt nichts davon.

## Neu

- **`LumiViz.exe --testing`:** nach dem Wiederherstellen des Startzustands
  (Preset, Komposition, Startdatei, Wiederaufnahme des Players) lädt LumiViz
  `SichttestSteuerung1.dll` aus dem Ordner der Exe, meldet seine Aktionen an
  und verbindet sich mit dem Tester; läuft keiner, startet die DLL ihn. Im Log
  steht die Version der geladenen DLL und jede ausgelöste Aktion.
- **Fünfzehn Aktionen**, die der Tester auslösen kann — keine öffnet einen
  Dialog: `komposition_laden`, `komposition_schliessen`, `composer`, `edit`,
  `fit`, `abspielmarke`, `wiedergabe`, `schleife`, `sync_kette`,
  `panel_zeigen`, `preset_laden`, `karaoke_laden`, `karaoke`, `titel_laden`,
  `player`. Dateien gelten ab dem Root des Projekts.
- **Eine geladene Vorlage hält:** `komposition_laden` lädt ohne Rückfrage, der
  Composer ist danach aus, die Stelle steht bei 0:00, nichts läuft. Neue
  Einstellung **Settings → Panels**, Block Composer, Zeile **Sichttest:
  A loaded template starts playing** (Vorgabe aus) — an: nach dem Laden
  Composer on und Abspielen.
- **Läuft LumiViz schon,** zeigt ein Start mit `--testing` die Meldung
  „LumiViz is already running. Close it and start again with --testing." und
  endet; das laufende LumiViz bleibt unberührt.
- **Verknüpfung „LumiViz.exe - Testing"** neben der Exe: sie entsteht nach
  jedem Bau der Exe und startet LumiViz mit `--testing`.
- **Projektdatei** `.sichttest/sichttest.projekt.json` im Repo: sie nennt dem
  Tester die Exe und die Sichttest-Listen.

## Geändert

- **Paket der Sichttest-Steuerung** 0.3.2 (vorher 0.2.0); **Build-System**
  CMakeCraft v0.11.0 (vorher v0.10.0): DLL und Tester werden jetzt bei jedem
  Bau neben die Exe gelegt, nicht nur beim Linken.
- **Das Werkzeug Sichttest baut LumiViz nicht mehr selbst:**
  `projects/exec/Sichttest` ist entfernt. Der Tester kommt aus dem Paket und
  liegt unter `sichttest\Sichttest.exe` neben `LumiViz.exe`.
- **Unit-Tests im Debug-Bau mit MSVC** entstehen wieder (`/bigobj` am
  Test-Target).

## Zu beachten

- `--testing` ändert den Start nicht: LumiViz stellt die zuletzt geladene
  Komposition wieder her wie immer. Erst die Aktionen des Testers ersetzen
  sie — und beim Beenden merkt sich LumiViz dann die Test-Vorlage.
- Release mit clang baut nur in der Umgebung von **Visual Studio 18**
  (`vcvars64.bat` von dort); mit der von Visual Studio 2022 scheitert das
  Linken.
- Ein Wechsel der Paketversion übersetzt Kern und Exe neu (der Include-Pfad
  trägt die Version).

## Doku

- `Benutzerhandbuch.md` 1.113.0 — §18 NEU *LumiViz vom Sichttest steuern
  lassen*.
- `Composer_Konzept.md` 0.78.0 — §12.2 Nähte N20 bis N25, N19 und N9 wachsen;
  §12.4 Rückbau-Liste.
- `Offene_Punkte.md` 1.127.0 — Stand der Sichttest-Steuerung, Entscheid zum
  Überführen nach `master`, Vormerkung „MilkDrop-Presets stocken manchmal".
