# LumiViz — Sichttest-Steuerung (für Entwickler)

> **Version:** 1.2.0
> **Datum:** 2026-10-19 (Session 136)
> **Typ:** Wegleitung (**umgesetzt**, Session 105)
> **Zielgruppe:** App-Entwickler — **kein Teil des Benutzerhandbuchs**, das
> Release-Paket enthält weder Tester noch Steuerung
> **Bezug:** `SichtTest_Helper/docs/Specification.md` (verbindliche
> Spezifikation der Schnittstelle, englisch maßgeblich, deutsch
> `Specification.de.md`; eigenes, öffentliches Repository — bis `v0.4.0` hieß sie
> `docs/Konzept_Steuerung.md`, §3 bis §9 tragen dieselben Nummern) ·
> [Composer_Konzept.md](ui/Composer_Konzept.md) §12.2, Nähte N19 bis N26 ·
> `include/sichttest/Anbindung.hpp` · `include/sichttest/AppAktionen.hpp` ·
> `include/composer/SichttestAktionen.hpp` · `include/loop/SichttestAktionen.hpp`
> **Sprache:** Deutsch

---

## 1. Wozu

Was sich nur ansehen und anhören lässt, prüfen **Sichttests**: Listen von
Prüfpunkten, die das Werkzeug **Sichttest** anzeigt und deren Urteile es
festhält. Viele Abschnitte beginnen mit denselben Handgriffen — eine Vorlage
laden, den Composer einschalten, Edit setzen, Fit drücken. Mit dem Schalter
`--testing` übernimmt der Tester diese Handgriffe; der Mensch urteilt nur noch.

Ohne `--testing` lädt LumiViz nichts davon und öffnet keinen Tester.

---

## 2. Starten

- Neben `LumiViz.exe` liegt nach jedem Bau die Verknüpfung
  **LumiViz.exe - Testing** — sie startet `LumiViz.exe --testing`.
- LumiViz fährt hoch wie immer, mit allem, was es sich gemerkt hat (Preset,
  Komposition, Startdatei, Wiederaufnahme des Players). **Danach** lädt es
  `SichttestSteuerung1.dll` aus dem Ordner der Exe, meldet seine Aktionen an
  und verbindet sich mit dem Tester; läuft noch keiner, startet die DLL
  `sichttest\Sichttest.exe`.
- Der Tester findet über `.sichttest/sichttest.projekt.json` (Repo-Root) die
  Exe und die Listen `Sichttest_Composer_*.md` unter `.claude/handover`.
  Die Datei nennt `anwendung` `LumiViz`, `start` die Release-clang-Exe
  (`out/build/windows-ninja-release-clang/exec/LumiViz/bin/Release/LumiViz.exe`
  mit `--testing`) und `listen` (Ordner `.claude/handover`, `muster`
  `Sichttest_Composer_*.md`) — das Format beschreibt Specification §7.
- An jedem Punkt **Vorbereiten** steht im Tester **▶ Ausführen**.
- Die Steuerung verbindet sich erst, **nachdem** LumiViz seinen Startzustand
  wiederhergestellt hat (in `MainWindow`, nach dem Wiederherstellen, hinter
  `lumi::sichttest::verlangt`) — sonst überschriebe das Wiederherstellen, was
  eine erste Aktion eben hergestellt hat.

**Was dabei geschieht:**

- Bevor der Tester etwas auslöst, das Ungespeichertes ersetzt, fragt er einmal
  nach. Mit dem Haken *In dieser Sitzung nicht mehr fragen* bleibt die Frage
  aus, bis der Tester geschlossen wird.
- Eine geladene Vorlage **hält am Anfang**: Composer aus, Stelle 0:00, nichts
  läuft — jeder Abschnitt beginnt im selben Zustand. Soll sie gleich spielen:
  **Settings → Panels**, Block Composer, Zeile **Sichttest**, Haken **A loaded
  template starts playing** (Vorgabe aus).
- Die Vorlage **ersetzt, was geladen war.** Beim Beenden merkt sich LumiViz die
  Vorlage als zuletzt geladene Komposition.
- Endet LumiViz geordnet, zeigt der Tester „Die Anwendung wurde beendet" mit
  dem Knopf **Tester beenden**. Er schließt nicht von selbst — manche
  Prüfpunkte verlangen gerade einen Neustart von LumiViz.

**Wenn es nicht verbindet:**

- Läuft LumiViz schon ohne `--testing`, meldet der neue Start „LumiViz is
  already running. Close it and start again with --testing." und endet mit
  Rückgabe 1; das laufende LumiViz bleibt unberührt.
- Im Log (`LumiViz.log` neben der Exe) stehen die Zeilen mit **Sichttest:** —
  die Fassung der geladenen DLL, der Zustand der Verbindung und jede ausgelöste
  Aktion mit ihrem Ergebnis. Fehlt die DLL, sagt das Log es in einem Satz, und
  die App läuft normal weiter.

---

## 3. Die Aktionen

Keine öffnet einen Dialog. Eine relative Datei gilt ab dem Root des Projekts
(dem Ordner über `.sichttest`).

| Aktion | Was sie tut | Schalter |
|---|---|---|
| `komposition_laden datei=<pfad>` | lädt eine `.lvcomp` und **hält**: Composer aus, 0:00 | fragt vorher |
| `komposition_schliessen` | Composer aus, Komposition aus dem Panel | fragt vorher, nur im ersten Lauf |
| `composer an\|aus` | Schalter Composer on | — |
| `edit an\|aus` | Schalter Edit des Panels Composer | — |
| `fit` | wie der Knopf Fit | — |
| `abspielmarke zeit=<m:ss>` | springt, auf die Länge begrenzt | — |
| `wiedergabe start\|pause\|stopp` | Transport des Composers | — |
| `schleife an\|aus` | Loop des Composers | — |
| `sync_kette an\|aus` | Sync on Effect Chain | — |
| `panel_zeigen name=<Kennung oder Titel>` | Panel öffnen und nach vorn | — |
| `preset_laden datei=<pfad>` | `.avs`, `.milk`, `.lvfx`, `.lvfx2` in die Effektkette | fragt vorher |
| `karaoke an\|aus` | Schalter Karaoke on | — |
| `karaoke_laden datei=<pfad>` | `.lvkar` in die Karaoke-Ebene, schaltet sie ein | fragt vorher |
| `titel_laden datei=<pfad> [spielen=ja]` | Titel in die Playlist, ohne Doppeleintrag | — |
| `player start\|pause\|stopp` | Transport des Players | — |
| `loop_laden datei=<pfad>` | lädt eine `.lvloop` ins Panel Loop und **hält**: ein spielender Loop wird angehalten | fragt vorher |
| `loop_wiedergabe start\|stopp` | Play und Stop des Panels Loop; `start` spielt den ganzen Loop von vorn. Hat der Composer den Ton, scheitert `start` und sagt es | — |
| `loop_editor basis=<Name>` | öffnet den Editor dieser Basis, wie **Edit base** am Clip: den Drum-Editor oder, bei einem Instrument-Muster, den Instrument-Editor | — |

`preset_laden` gibt den Auftrag nur ab: scheitert das Laden selbst, meldet
LumiViz das in einem eigenen Dialog, die Antwort an den Tester sagt es nicht.

**Warum `komposition_laden` hält** (Entscheid Patrik L1, Sync `LV-20261009-1905`):
sie lädt ohne Dialog, schaltet den Composer aus und steht auf 0:00 — jeder
Abschnitt beginnt im selben Zustand; erst `composer an` schaltet ein.

**„fragt vorher" und „nur im ersten Lauf" sind zwei Schalter** (Befund LV,
08.10.): `komposition_laden` ersetzt Ungespeichertes und fragt deshalb vorher —
sie **ist** aber das Herstellen eines Abschnitts und läuft deshalb auch beim
Fortsetzen einer Liste wieder. `komposition_schliessen` dagegen fragt und läuft
nur im ersten Lauf (Specification §3, Zustände).

**In einer Liste** steht eine Aktion als Code-Stück im Text eines Punkts:

```markdown
- [ ] **G0 Vorbereiten:** **Load...** → `asset\sichttest\composer_B_viele.lvcomp`. …
      `aktion: komposition_laden datei="asset\sichttest\composer_B_viele.lvcomp"`
      `aktion: composer an` `aktion: edit aus` `aktion: fit`
- [ ] **T0 Vorbereiten:** wie G0, aber **Edit** an.
      `aktion: @G0` `aktion: edit an`
```

`@G0` führt die Aktionen des Punkts G0 derselben Liste aus. Ein einzelner Wert
ohne Schlüssel (`composer an`) kommt als Argument `wert` an.

**Das Format der LumiViz-Listen** (Specification §6.1 beschreibt Markdown
allgemein): je Abschnitt ein Punkt **Vorbereiten**, je Punkt *Tun* und
*Erwartet*, die Urteile getrennt in Abschnitt **U**; die Vorlagen der Listen
liegen unter `asset/sichttest/`. Vorbild ist
`.claude/handover/Sichttest_Composer_1_Master-Zeilen.md` mit dem Beispiel
„M. Der shape der Zeile MASTER bleibt an seiner Zeit".

---

## 4. Wo der Code liegt

| Was | Wo |
|---|---|
| Laden der DLL, Anmelden, Pfade — **die eine** Datei mit den Köpfen des Pakets, hinter `SICHTTEST_VORHANDEN` | `src/sichttest/Anbindung.cpp` |
| Aktionen der App (Panel, Preset, Karaoke, Playlist, Player) | `src/sichttest/AppAktionen.cpp` |
| Aktionen des Composers | `src/composer/SichttestAktionen.cpp` |
| Aktionen des Panels Loop | `src/loop/SichttestAktionen.cpp` |
| Start mit `--testing` nach dem Startzustand; Meldung bei laufender Instanz | `src/UI/MainWindow.cpp`, `src/Application.cpp` (Vermerk `Composer-Naht N20`, `N21`) |
| Projektdatei für den Tester | `.sichttest/sichttest.projekt.json` |
| Verknüpfung nach dem Bau | `CMakeLists.txt` (Block `[Sichttest]`), `tools/sichttest/TestingVerknuepfung.ps1` |
| Festes Material der Listen (Vorlagen, Presets, Testsignale) | `asset/sichttest/` |

**Nicht der Kanal:** `core/EinzelInstanz` (ein QObject mit `QLocalServer`, das
einen zweiten Start an den ersten weiterreicht) bleibt, was es ist — die
Anwendung soll für den Tester nicht lauschen; der Kanal gehört der DLL
(Specification §4).

**Eine neue Aktion** ist eine Funktion, die sich auch ohne Tester rufen lässt
(`Ergebnis f(VisualizerWidget&, const QJsonObject&)`), plus eine Zeile beim
Anmelden. Die Köpfe des Pakets braucht sie nicht — sie spricht nur mit
`lumi::sichttest::Anbindung`.

---

## 5. Bezug des Pakets

Tester und DLL kommen als fertiges Paket aus dem Repository
[SichtTest_Helper](https://github.com/PatrikNeunteufel/SichtTest_Helper):
`sichttest.pin` im Repo-Root nennt Version, Adresse und Prüfsumme, wie
`cmakecraft.pin` für das Build-System. Gelinkt wird nichts; neben `LumiViz.exe`
liegen `SichttestSteuerung1.dll` und der Ordner `sichttest\`.

- **Version wechseln:** `sichttest.pin` ändern, bauen. Kern und Exe übersetzen
  dabei neu, weil der Include-Pfad die Version trägt.
- **Solange ein Tester offen ist, nicht bauen:** er sperrt
  `sichttest\Sichttest.exe` neben der Exe.
- **Ins Release-Paket gehören Tester, DLL und Verknüpfung nicht** (die
  Verknüpfung trägt einen festen Pfad).
- **Warum der Tester in `sichttest\` liegt und nicht neben der Exe:** ein
  Debug-Build von LumiViz lädt `Qt6Cored.dll`, der Tester das Qt seines Pakets —
  im eigenen Ordner kommen sich die beiden nicht in die Quere (Specification §7.1).
- **Nur eine Instanz je Exe** (`Application.cpp`, Meldung „LumiViz is already
  running …"): ein zweiter Start endet sofort. Darum kann der Tester „gestartet,
  aber nicht verbunden" melden, wenn schon ein LumiViz ohne `--testing` läuft
  (Specification §6.3).

---

## 7. Herkunft

Gebaut in Schritt 4 der Umsetzung (Session 105): `LumiViz.exe --testing`, die
ersten Aktionen (Sync LV-1), Listen mit `aktion:`-Stücken, die eigene Kopie
`projects/exec/Sichttest` entfernt (LV-2). Benannt waren zuerst fünfzehn Aktionen
(Sync `LV-20261008-2244`), gebaut ab dem 09.10. zuerst `komposition_laden`,
`composer`, `edit`, `fit`; heute achtzehn (§3). Bis `v0.4.0` stand das alles auch
in der Spezifikation von SichtTest_Helper; seit dem 19.10.2026 beschreibt die nur
noch die Anbindung *irgendeiner* Anwendung, LumiViz-Eigenes steht hier.

---

## 6. Tests

`tests/unit/UnitTests/test_ComposerSichttestStart.cpp` (Suite
`ComposerSichttestStart`):

- die Aktionen unter *Vorbereiten* erreichen aus neun wiederhergestellten
  Startzuständen denselben Stand;
- Argumente, Fehler und die Einstellung zum Abspielen; Schalter, Root, Dateien,
  Zeiten;
- `panel_zeigen` an echten Docks, `karaoke_laden` mit dem Visualizer Multi
  Effect;
- die Aktionen des Panels Loop stehen in `test_LoopSpieler.cpp` (Fall „Loop
  Sichttest-Aktionen", gegen die Engine aus Pappe, ohne Ton und ohne Fenster);
- **die Probe der DLL** (laden, anmelden, mit dem Tester verbinden) läuft nur
  mit `LUMIVIZ_SICHTTEST_PROBE=<Ordner mit LumiViz.exe>` — sie startet den
  Tester und verbindet sich mit einem schon laufenden.

---

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 1.2.0 | 2026-10-19 | Session 136. Übergabe aus SichtTest_Helper (Sync `sync_sichttest`, `Uebergabe_LV.md` L1–L10): Bezug auf `Specification.md` statt `Konzept_Steuerung.md`; §2 Projektdatei und Verbinden nach dem Startzustand; §3 warum `komposition_laden` hält, zwei Schalter, Format der Listen; §4 EinzelInstanz ist nicht der Kanal; §5 Ordner des Testers, eine Instanz; neu §7 Herkunft. |
| 1.1.1 | 2026-10-11 | Session 110. §3 `loop_editor` öffnet auch den Instrument-Editor. |
| 1.1.0 | 2026-10-11 | Session 110. §3 drei Aktionen für das Panel Loop: `loop_laden`, `loop_wiedergabe`, `loop_editor`; §4 und §6 nachgezogen. |
| 1.0.0 | 2026-10-10 | Session 105. Erstfassung — aus dem Benutzerhandbuch (§18, dort entfernt: der Tester ist ein Werkzeug für Entwickler) und ergänzt um Aktionen, Code-Orte, Bezug des Pakets und Tests. |
