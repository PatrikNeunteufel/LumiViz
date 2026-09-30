# LumiViz — Regression S84: Tempo, Lag, Vollbild-Freeze

> **Version:** 1.0.0
> **Datum:** 2026-09-30 (Ende Session 84)
> **Typ:** Befund + Suchplan
> **Status:** **Offen — Ursache nicht gefunden, nicht gemessen.** Erste Aufgabe
> der nächsten Sitzung (S85).
> **Sprache:** Deutsch

## 1. Symptome (Meldung Patrik, 30.09.2026 abends)

Wortlaut (zwei Meldungen, sinngemäß zusammengefasst, Kernsätze wörtlich):

1. „die Änderungen haben dazu geführt, dass z. B. beim Wechsel in den
   Fullscreen oder zurück 1 s freezed wird"
2. „es ist viel mehr kaputt gegangen … manche Presets (egal ob MilkDrop oder
   AVS) laufen jetzt **schneller oder langsamer**, dazu der Lag … irgendwelche
   Umbauten waren zu invasiv … lass uns zuerst feststellen, ab wann"

Beobachtet in der **Sichttest-Exe** (`out\build\windows-ninja-release-clang\
exec\LumiViz\bin\Release\LumiViz.exe`), Läufe laut Log um 18:51, 19:07 und
19:10. Welche Presets genau betroffen sind, ist **noch nicht notiert** →
in S85 als Erstes bei Patrik erfragen (Liste mit „schneller"/„langsamer").

## 2. Zeitachse: Commits und Builds in S84

| Commit | Zeit | Inhalt | Render-Pfad berührt? |
|---|---|---|---|
| `968df01` | 12:51 | **Stand Ende S83** (Basis, vermutlich gut) | — |
| `cc16a80` | 15:06 | Drag & Drop: Effect Browser → Chain (Presetverweis), Playlist nimmt an/gibt ab (Umsortieren im Dienst), Drop auf Player/Visualisierung, Audio Browser + Albumcover (eigener Thread `CoverLader`), Vorlagen-Zeile im Groß-Editor | **gering:** `VisualizerWidget::setAcceptDrops(true)` + drei Drag-Handler; `IAudioEngine::supportedExtensions` (neue virtuelle Funktion) |
| `6154972` | 15:28 | Feld-Tooltips englisch (673 Kommentare in `EffectChain.hpp`, nur Kommentarzeilen ersetzt), `FieldDocs.cpp` neu, Ernter-Regel `SKRIPT_MUSTER`, Kamera-Clip `camera_clip_…` | **nein** (nur Kommentare; Ernter/FieldDocs sind UI/Werkzeug) |
| `6808317` | 18:51 | **Block F, F1 + F2:** erzeugte Feldtabelle `FieldBindings.cpp`; `renderNode` legt für Knoten mit Parameter-Skript eine **Frame-Kopie der Params** an; `runParamScript` bindet **alle** Tabellenfelder, schreibt nach dem Lauf in die Kopie (Typpflege, Strukturregel mit `LeafRuntime::strukturWert`) | **JA, stark** |
| *uncommitted* | 19:1x | F5 (Referenz + Highlighter aus der Tabelle), **Fix Verlaufs-LUT** (Strange Attractor/Flame bauten die CPU-LUT jeden Frame und lasen dabei alle `.grad`-Dateien), **Messpunkte** im Render-Thread (`VisualizerRenderThread.cpp`, Log bei Schritten > 50 ms, `resize` immer) und im Vollbild-Wechsel (`MainWindow.cpp`) | ja: LUT-Cache (Verbesserung), Messpunkte (nur Log) |

Patriks Beobachtung fällt auf den **Build nach `6808317`** (F2). Ob die
Symptome schon mit `cc16a80` oder `6154972` auftraten, ist **nicht belegt** —
Patrik hat dazwischen keinen Vollbild-/Tempo-Sichttest gemacht.

**Wichtig — Versäumnis S84:** Nach F2 lief **kein** Vergleichslauf
(`compare_milkref.py` / `compare_avsref.py`), nur die Unit-Tests (710/710).
Für einen Umbau im Render-Pfad war das zu wenig (Lehre §6).

## 3. Was belegt ist — und was nicht

**Belegt (Log `…\Release\LumiViz.log`):**
- FPS des Render-Threads nach Vollbild-Wechsel: 20 (19:07:44). **Aber:**
  Einbrüche nach dem Umschalten gab es sporadisch **schon vorher** (29.09.:
  13, 30, 0; 28.09.: 48, 53). Der FPS-Wert ist kein sauberes Signal — er misst
  den Render-Thread, nicht den GUI-Thread, und mittelt über ein Fenster.
- `ColorGradientModule` lud in einem Lauf 2 182-mal alle eigenen
  Verlaufsdateien (≈36/s). Ursache gefunden: Strange Attractor (`useGradient`)
  und Flame riefen `buildCpuGradientLut` **jeden Frame** auf. **Alt** (tritt
  auch am 29.09. und am Morgen des 30.09. auf), in S84 behoben (uncommitted).
  Kann Presets mit diesen Knoten ruckeln lassen, erklärt aber nicht alles.

**Geprüft S84 (Code-Diff):** `6154972` (Tooltips) hat in `EffectChain.hpp`
**keine Codezeile** verändert — die 144 Zeilen mit Code und `///<`-Kommentar
sind im Code-Teil vorher/nachher identisch, alle übrigen Änderungen sind reine
Kommentarzeilen. Render-relevant in `6154972` bleibt nichts (Rest: UI-Tooltips,
Ernter, `SettingsPanel` Dateiname des Kamera-Clips).

**Nicht belegt / ausgeschlossen:**
- Audio Browser als Freeze-Ursache: **ausgeschlossen** — der Vollbild-Wechsel
  (`MainWindow::enterFullscreen/exitFullscreen`) hängt nur das
  Visualisierungs-Widget um, Dock-Panels bekommen kein `onActivate`.
- Messpunkte (uncommitted) wurden **noch nicht ausgewertet** — kein Lauf mit
  der instrumentierten Exe.

## 4. Hypothesen (nach Wahrscheinlichkeit)

**H1 — F2 (`6808317`) verändert das Verhalten gescripteter Knoten.**
Seit F2 bindet `runParamScript` **alle** skalaren Felder des Knotens unter
ihrem kleingeschriebenen JSON-Namen (plus Aliasse). Folgen:
- *Namensfang:* ein Skript, das eine Variable wie `speed`, `size`, `blend`,
  `mode`, `count`, `points`, `alpha`, `n` als **eigene Hilfsvariable** nutzt,
  steuert jetzt das gleichnamige Feld. Vorher war sie harmlos lokal.
- *Fortschreibung:* diese Werte bleiben über Frames erhalten (`paramState`).
- Betroffen nur Typen mit Träger **ParamSkript** (54 Typen, Liste in
  `FieldBindings.cpp`, `Traeger::ParamSkript`) **und** nicht-leerem
  `initCode`/`frameCode`/`beatCode` oder Song-Code. **Geprüft S84:** Effect
  List und Host-Gruppe haben Träger **Keiner** — F2 greift dort nicht; ebenso
  nicht bei eigenen Hosts (SuperScope, Movement, Fraktale, 3D, …) und nicht beim
  Milkdrop-Knoten. **Offen:** ob importierte AVS-/Milk-Presets überhaupt einen
  ParamSkript-Knoten mit Code enthalten — wenn nicht, läuft der F2-Pfad bei
  ihnen **nie**, und H1/H2 erklären „egal ob MilkDrop oder AVS" **nicht**. Das
  ist in S85 zuerst zu klären (einmaliger Log je Knoten, der die Frame-Kopie
  bekommt, oder Zählung in den betroffenen Presets).
- *Mechanismus „schneller/langsamer":* ein gefangener Name treibt ein
  Tempo-/Größenfeld; oder der Knoten wird langsamer (s. H2) und frame-basierte
  AVS-Animation wirkt langsamer.

**H2 — F2 kostet Rechenzeit je gescriptetem Knoten und Frame.**
Je Knoten mit Code und je Frame: Kopie von `EffectParams`, 20–40 zusätzliche
Variablen (`setNumber`/`number` über Lua), je Variable mehrere
`std::unordered_map<std::string,…>`-Zugriffe mit String-Aufbau
(`paramState`, `paramSeen`), linearer `strcmp`-Abgleich gegen die Hand-Liste.
Presets mit vielen gescripteten Listen (AVS „Whacko"-Art, 50+ Listen) können
dadurch mehrere ms/Frame verlieren → Frame-Rate sinkt → AVS läuft langsamer,
MilkDrop ruckelt.

**H3 — Vollbild-Freeze unabhängig von F2.** Beim Umschalten wird die GL-Fläche
neu angelegt und die Kette auf die neue Größe gebracht (`resize`: FBOs,
Puffer). Kandidaten: teures `resize` einzelner Knoten, `setAcceptDrops(true)`
am Visualisierungs-Widget (`cc16a80`; Drop-Site-Registrierung beim Umhängen),
oder schon alt. Messpunkte liegen bereit (§5 Schritt 4).

**H4 — Nebenwirkung aus `cc16a80`** (Drop-Handler, Cover-Thread mit
`QThread::LowPriority`, `IAudioEngine` bekam eine neue virtuelle Funktion) —
unwahrscheinlich, per Messung auszuschließen. Wird H4 wahrscheinlicher, falls
sich zeigt, dass der F2-Pfad bei den betroffenen Presets gar nicht läuft (H1).

**H5 — kein Code-Fehler, sondern Umgebung.** Die CPU läuft laut Handover schon
im Leerlauf bei 85–97 °C, harter Absturz am 29.09.; ein Energieplan-/Takt-
Wechsel würde Tempo „schneller/langsamer" je Lauf erklären. Deshalb vergleicht
Messung A **Bilder mit Sim-Uhr** (temperaturunabhängig) und Messung B Zeiten
**mehrerer Builds im selben Zeitfenster**.

**Zu H1–H5 vorab:** Die Aussage „ab F2" stützt sich allein darauf, dass Patrik
den Effekt am F2-Build bemerkt hat — ein früherer Build wurde nicht darauf
angesehen.

## 5. Plan für S85 — erst feststellen, ab wann

**Regel:** erst messen, dann deuten (Merkregel S81). Keine Fixes, bevor der
Commit feststeht.

**Schritt 0 — Stand sichern.** Patrik committet den uncommitted Rest als
**eigene Commits**, damit er bisektierbar bleibt (Vorschlag: (a) Verlaufs-LUT-
Fix, (b) Messpunkte, (c) F5). Message-Texte liefert Claude.

**Schritt 1 — betroffene Presets festhalten.** Patrik nennt die Presets
(Datei + „schneller"/„langsamer"/„Lag"). Daraus eine Liste
`…\.claude\handover\regression_s84_presets.txt` (Pfade, wie
`kurzlauf_liste.txt`).

**Schritt 2 — Worktrees anlegen** (verändern Git-Metadaten → **Patrik führt
aus** oder gibt ausdrücklich frei; Claude liefert die Befehle):

```
git worktree add ../LumiViz-wt-968df01 968df01
git worktree add ../LumiViz-wt-cc16a80 cc16a80
git worktree add ../LumiViz-wt-6154972 6154972
git worktree add ../LumiViz-wt-6808317 6808317
```

Je Worktree vor dem Konfigurieren (untracked, fehlt sonst):
- `CMakeUserPresets.json` aus dem Hauptordner kopieren (lokale Pfade, `QT_ROOT`);
- BASS-Binaries unter `externals/bass/…` (bewusst untracked, `externals/bass/SETUP.md`)
  aus dem Hauptordner kopieren;
- CMakeCraft: Bootstrap holt nach `.externals/cmakecraft/<ver>/` (Netz) —
  oder `-DCMAKECRAFT_LOCAL_DIR=../CMakeCraft` bzw. den Cache-Ordner kopieren.

Bauen je Worktree (nur was die Harnesse brauchen):
`cmake --preset windows-ninja-release-clang` und die Targets
`MilkdropStandalone`, `AvsStandalone` (für Sichttests zusätzlich `LumiViz`).
Wichtig: Die Standalones kompilieren die App-Quellen selbst (Merkregel
CMakeCraft) — F2 steckt also auch in ihnen.

**Schritt 3 — Messung A: Bildvergleich zwischen Builds** (Treue/Tempo,
deterministisch). In **jedem** Worktree mit **identischen** Argumenten
(nur normales Python, `LUMIVIZ_REF_BILDSCHIRM=DISPLAY2`):

```
…\Python312\python.exe asset/calibration/milkdrop/compare_milkref.py --presets <Liste/Ordner> --zufall-lumiviz --out out/regress_s84
…\Python312\python.exe asset/calibration/avs/compare_avsref.py <presets …> --out out/regress_s84_avs
```

Ausgewertet wird **nicht** gegen die Referenz, sondern die **LumiViz-Seite
zwischen den Builds** (PNG je Marke, bitgleich erwartet, weil Sim-Uhr und
Audio deterministisch sind). Der erste Commit, dessen Bilder abweichen, ist
der Täter. Vergleichsbasis aus früheren Läufen zusätzlich vorhanden:
`out/milkref_kalibrierung_S81_4090`, `out/milkref_kerne_S82`.
Karte gleich halten (`--karte 4090`, Registry-Eintrag, Memory
„GPU-Auswahl"); Hitzewächter nach Handover.

**Schritt 4 — Messung B: Zeit** (Lag, Vollbild). Mit dem Build, der die
Messpunkte trägt: dasselbe Preset, zweimal Vollbild hin und zurück → Log:
`Entered fullscreen mode …; hide=…ms …`, `Exited fullscreen mode; …`,
`Fullscreen exit: recreateNativeWindow=…ms`, `RenderThread: resize auf WxH
in … ms`, `RenderThread: render/swapBuffers/makeCurrent … ms` (nur > 50 ms).
Für das Tempo: Frame-Zeit je Preset zwischen den Builds vergleichen (Log
`FPS (render thread)` gleiches Preset, gleiche Fenstergröße) — falls zu grob:
Messpunkt „mittlere render-Zeit je 5 s" ergänzen.

**Schritt 5 — innerhalb des Täter-Commits eingrenzen.** Ist es `6808317`:
A/B **in einem Build** über einen Umgebungsschalter (Vorschlag:
`LUMIVIZ_OHNE_FELDSTEUERUNG=1` → `renderNode` legt keine Kopie an,
`feldBindungVor` bindet nichts). Getrennt prüfen: (i) nur Kopie ohne
Zusatzbindung, (ii) Bindung nur für bisher gebundene Namen, (iii) voll.
Dann H1 (Namensfang) gegen H2 (Kosten) unterscheiden: bei H1 ändern sich
**Bilder** (Messung A), bei H2 nur **Zeiten** (Messung B).

**Schritt 6 — Entscheid mit Patrik** (Format Entscheidungsfragen): Fix,
Rückbau hinter einen Schalter, oder Rücknahme von F2. Block F ruht bis dahin.

## 6. Lehre

Ein Umbau im Render-Pfad (hier: jede Frame-Kopie und jede Skriptbindung aller
Knoten) braucht **vor dem Commit** einen Vergleichslauf der Kalibrier-
Harnesse gegen den vorigen Stand — Unit-Tests prüfen Bindung und Wirkung
einzelner Felder, nicht Tempo, Bild und Laufzeit ganzer Presets.
