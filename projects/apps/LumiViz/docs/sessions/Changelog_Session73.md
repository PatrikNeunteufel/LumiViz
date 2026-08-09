# Changelog Session 73 (2026-08-08/09)

> **LumiViz ist öffentlich.** github.com/PatrikNeunteufel/LumiViz, Tag `v0.2.0`
> mit lauffähigem Windows-Binary. Aus einer Lizenz-Prüfung wurde die
> Veröffentlichung samt Historien-Umschrieb, Umbenennung, Versions-SSOT,
> Schaufenster und drei behobenen App-Fehlern. Tests 593 grün.

## Teil 1 — Rechtliche Grundlage

Die Prüfung förderte mehr zutage als erwartet. **Entgegen der eigenen Doku
waren 63 BASS-Binärdateien getrackt** — die `.gitignore` fing nur `x64/`,
`x86/` und `bin/` ab, die 32-Bit-DLL lag aber direkt in `win/`, die Linux-Libs
in `linux/libs/<arch>/`, die macOS-Dylibs in `osx/`. Ausgerechnet die zum Bauen
nötigen x64-Dateien waren als einzige korrekt ignoriert: maximaler
Lizenzschaden, null Nutzen.

Dazu 630 AVS-Community-Presets, sieben Playlists mit absoluten Pfaden in die
private Musiksammlung, ein 6,5-MB-Lesezeichen-Export, fremde Logos und ein
Lehr-Shader ohne Lizenzangabe.

**Historie umgeschrieben** (`git filter-repo`): 87 → 52 MB, 5220 → 3665 Pfade.
Das GitHub-Repo wurde gelöscht und neu angelegt, damit keine alten Objekte per
SHA erreichbar bleiben — ein Force-Push allein hätte sie liegen lassen.

**NEU:** duale Lizenz MIT **oder** Apache-2.0 · `THIRD_PARTY_NOTICES.md` (auch
für die Bibliotheken, die nur im Binärpaket liegen) · `.github/` mit vier
echten Issue-Forms, darunter „Preset-Befund" · `README.md` · `BUILDING.md`.

## Teil 2 — MyViz → LumiViz, Version als SSOT

Die App hieß intern noch MyViz. Umbenannt wurde alles: Target-Namen,
Verzeichnis, Zeichenketten, Doku. Ein `myviz`-Namensraum existierte nicht, was
den Umbau erheblich entschärft hat.

**Version 0.2.0 als SSOT** aus `Solution.json` — vorher stand sie an drei
Stellen hart im Code (`Application.cpp`, `AboutDialog.cpp`, Solution.json) und
konnte auseinanderlaufen.

## Teil 3 — Drei App-Fehler behoben

**🐞 `.lvfx`-Ketten waren an den Rechner des Erstellers gebunden.**
`resolveAviPaths` lief NUR beim `.avs`-Import; beim `.lvfx`-Laden übernahm der
Serialisierer den gespeicherten absoluten Pfad unverändert. `videoSource` hatte
die Auflösung seit S70, `avi` nicht. Der Fix sucht über den blanken Dateinamen
und repariert damit auch Altbestand.

**🐞 Der Blättern-Hotkey tat nach jedem Neustart nichts**, bis man einmal im
Browser etwas geladen hatte. `onPresetStep` liest die Liste, die aber erst in
`onActivate()` entstand — war das Panel nie offen, war sie leer. Obwohl der
Hotkey ausdrücklich ohne sichtbares Panel wirken soll.

**🐞 Der Startzustand wurde nicht wieder aufgenommen.** Import-Browser lädt und
markiert jetzt das zuletzt geladene Preset; der zuletzt gewählte Visualizer
wird gemerkt (`MainWindow` setzte vorher hart `multieffect`).

## Teil 4 — Schaufenster

**29 eigene Presets** (10 AVS, 19 MilkDrop) wandern per POST_BUILD neben die
Exe, dazu Benutzerhandbuch, **Preset-Quickstart** und **Preset-Anleitung** als
`docs/` — und die Lizenzdateien, ohne die ein weitergegebenes ZIP unvollständig
wäre.

**README** mit zwei Galerien und einem Kapitel „Fremde Presets importieren".

## Teil 5 — Drei Werkzeug-Lücken

Beim Erstellen der Screenshots fiel auf, was jahrelang unbemerkt blieb:

**`AvsStandalone` kannte den Render-Scale-Divisor nicht.** Die App setzt ihn vor
jedem Import aus `import/avsRenderScaleDivisor`, das Werkzeug nie. Bei kleinen
Fenstern fällt das nicht auf, bei 1280×720 zerfällt das Bild in Streifen —
**jeder Vergleich bei großen Fenstern war systematisch falsch.** NEU
`--render-scale N`.

**Das synthetische Audio hatte eine starre Klangfarbe.** NEU `--beat-hz N` und
`--klangfarbe`. Ohne die Schalter ändert sich nichts, bestehende Kalibrierläufe
bleiben bit-identisch.

**Debug-Builds sind Faktor 20 langsamer** (306 s gegen 15 s) und sehen aus wie
ein Hänger. Zusammen mit drei weiteren Fallen dokumentiert in der neuen
**`Werkzeug_Wegleitung.md`**.

## Teil 6 — Erster Vergleichslauf, zwei Befunde

**AVS gegen AvsRef:** 5 von 10 unter der Schwelle, 5 zum Prüfen, 0 Ladefehler.

**MilkDrop:** methodisch unsauber, und das ist selbst der Befund —
`--auto <ordner>` rendert alles in EINEM Prozess, und MilkDrop-Presets **erben
das Bild des Vorgängers**. Patrik sah es sofort: in der Helix-Montage stand ein
Herz, das alphabetisch davor laufende `Dancing Hearts`.

**🟠 Wellenform-Modus 7 steht auf dem Kopf** (Befund Patrik). Der Gegentest
über alle 19 Presets grenzt es sauber ein: nur die vier `nWaveMode=7`-Presets
profitieren vom Spiegeln. Codestelle benannt, zwei Verdächtige, keiner
geändert — das braucht den Zeilenvergleich gegen `milkdropfs.cpp`.

**NEU `docs/visuals/Kalibrier_Plan_Mitgelieferte_Presets.md`** — sechs Aufgaben
für die nächste Session, mit Gegenproben und Fertig-Kriterium.
