# LumiViz — Modul-Ideen (Übersicht)

> **Version:** 1.1.0
> **Datum:** 2026-10-02 (Session 88)
> **Typ:** Übersicht
> **Status:** Aktiv
> **Sprache:** Deutsch
> **Zweck:** Alle notierten Ideen für neue Module und für den Ausbau
> bestehender auf einer Seite — was es ist, wie weit es ist, wo die
> Einzelheiten stehen.

Diese Seite ist ein **Verzeichnis, keine zweite Quelle**: Einzelheiten,
Entscheide und Stand stehen in der Arbeitsliste
[`Offene_Punkte.md`](Offene_Punkte.md) (SSOT) beziehungsweise im genannten
Konzeptdokument. Ändert sich dort etwas, wird hier nur die Zeile nachgezogen.

**Stand:** ⚪ Idee, nichts gebaut · 🟡 teilweise gebaut · ✅ gebaut (Rest steht
in der Spalte „Offen").

## 1. Neue Knoten für die Effektkette

| Idee | Was es wäre | Stand | Offen | Einzelheiten |
|---|---|---|---|---|
| **Cover-Modul** | Albumcover als Bildquelle und eigener Knoten (zentriert, „Vinyl", unscharfer Hintergrund, Zerfall beim Titelwechsel); Farben aus dem Cover als Verlauf und Skriptvariablen | ⚪ | alles; Cover-Lesen gibt es schon für den Audio Browser | `Offene_Punkte` §7 „Cover-Modul" (S84) |
| **Custom-Functions-Modul** | Benannte EEL-Funktionen definieren, die andere Knoten aufrufen — Schwester des Global-Variables-Knotens | ⚪ | alles | `Offene_Punkte` §7 (Sammelzeile); `visuals/Skript_Variablen_Konzept.md` §7 |
| **Lights-Module** | Bildsprache von „Lights" (dunkler 3D-Neonraum, Bänder, Punktwolken, Bloom) mit eigenen Knoten | 🟡 | Bloom, SuperScope 3D, 3D Camera, Terrain 3D, Glow Orbs sind gebaut; offen der Rest des Entwurfs samt `AudioLookahead` (BASS-Vorausschau) | `visuals/Lights_Module_Entwurf.md`; `Offene_Punkte` §5/§7 |
| **GPU-Vertex-Module, Stufe G3** | Weitere Knoten mit GPU-Vertex-Arbeit nach Mesh Warp (G1) und GPU Particles (G2) | 🟡 | G3 ist nur eine Kür-Notiz | `Offene_Punkte` §7 „GPU-Vertex-Module" |
| **Standalones als Knoten** | Equalizer, Oscilloscope und Pulsing als Knoten der Effektkette statt eigener Visualizer | ⚪ | alles (V4 der Vereinheitlichung) | `visuals/Vereinheitlichung_Konzept.md`; `Offene_Punkte` §6 |
| **Gradient-Parameter-Typ + LUT-Baustein** | Verlauf als eigener Feldtyp, gemeinsamer LUT-Baustein | ⚪ | alles (V5 der Vereinheitlichung) | wie oben |

## 2. Ausbau bestehender Knoten

| Idee | Was dazukäme | Stand | Offen | Einzelheiten |
|---|---|---|---|---|
| **Text-Knoten ausbauen** | Textquellen: Listen, **Tags des Titels** (`{artist} – {title}`), Songtexte (auch zeitsynchron), Laufzeitwerte. Darstellung: **Schriftauswahl** (installierte und mitgelieferte Schriften), Verlauf, Leuchten, Animationen (Ticker, Schreibmaschine, Buchstaben auf dem Beat) | 🟡 | gebaut S86: Palette, Init/Frame/Beat-Felder, Voreinstellungen, Schrift-Auswahlliste, Häkchen Bold, **Tags des Titels als Platzhalter**. Offen: Listen, Laufzeitwerte, mitgelieferte Schriften, Verlauf/Leuchten, Animationen |
| **Songtexte / Karaoke** | Liedtext zum laufenden Titel, zeitsynchron (eingebettet oder `.lrc`), Karaoke-Zeile, Stimmen bei Duetten; Quelle lokal, online nur als Opt-in | 🟡 | gebaut S86: Knoten **Lyrics** (lokale Quellen in fester Reihenfolge, Vorschau, Wort für Wort, Duett-Farben), Online-Abruf als Opt-in mit Speicherort-Einstellung. Offen: Versatz je Titel, Datei zuweisen, Wort-Zeiten des Dienstes |
| **Songtext nachsynchronisieren (Panel)** | Ein Panel, das einen unsynchronen Text mit Zeiten versieht — mitklopfen, per Skript verteilen oder per KI ausrichten, danach von Hand nachjustieren; Ergebnis als `.lrc` | ⚪ | Panel fehlt; die Schnittstelle (Modell + `speichereSongtext`) steht |
| **Installer mit Dateityp-Registrierung** | Dateitypen und Icons in Windows wählbar registrieren (eigene Formate mit „Öffnen mit", fremde nur auf Wunsch) | ⚪ | alles; Icons als `.ico` liegen bereit, Notiz in `asset/img/logo/icons/README.md` | `Offene_Punkte` §7 „Text-Knoten ausbauen", Absatz „Songtexte" | `Offene_Punkte` §7 „Text-Knoten ausbauen" (S84/S86) |
| **Text als Milkdrop-Element** | Neben Wave, Shape und Sprite ein Text-Element im Milkdrop-Node — in dessen Pipeline gezeichnet, verschmiert mit dem Feedback | ⚪ | alles (neu S86) | `Offene_Punkte` §7 „Text-Knoten ausbauen", Punkt (3) |
| **Gemeinsame Farbquelle** | Eine Farbquelle für alle Zeichen-Knoten: Farbtafel, Verlauf über Punkte **oder über die Zeit**, Wechsel in Sekunden statt Frames | ⚪ | alles; betrifft 28 Zeichen-Knoten | `Offene_Punkte` §7 „gemeinsame Farbquelle" (S84) |
| **Szenen-Wechsler, Rest** | Szenen = Effect Lists beliebiger Tiefe, Wechsel nach Energie und Strophe/Refrain | 🟡 | gebaut S82: Host Switcher für Host-Gruppen. Offen: Listen, Energie-Klassen, Songstruktur | `Offene_Punkte` §7 „Szenen-Wechsler-Modul" |
| **Video Source: nur passende Felder** | Der Editor zeigt nur die Felder der gewählten Quelle (Datei, Kamera, Clip) | ⚪ | alles | `Offene_Punkte` §7 (S83) |
| **Skript für Movement, Render Scale, Debug Bars** | Formelfelder nachrüsten wie bei Text und AVI | ⚪ | bewusst zurückgestellt (Entscheid S86) | `visuals/Feldsteuerung_Konzept.md` §4 (F2e) |
| **Song-Slot im Milkdrop-Node** | Der Song-Slot aller Skript-Knoten auch für Milkdrop | ⚪ | alles | Handover „Weiter offen (aus S82)" |

## 3. Ein- und Ausgänge der App

| Idee | Was es wäre | Stand | Offen | Einzelheiten |
|---|---|---|---|---|
| **Externer Audio-Eingang** | Loopback („was aus den Lautsprechern kommt"), Line-in, Mikrofon — für Live-Einsatz | ⚪ | Schnittstelle deklariert, nicht gebaut (braucht BASSWASAPI) | `Offene_Punkte` §7 (S83) |
| **Video aufnehmen** | Bild und Ton als Datei, Qualität je Aufnahme wählbar | ⚪ | alles; vorhanden ist nur der Screenshot | `Offene_Punkte` §7 „Video aufnehmen" (S39/S83) |
| **Spout / NDI** | Bild an VDMX, MadMapper und Co. weitergeben | ⚪ | alles | `Offene_Punkte` §7 (S83) |
| **MIDI / OSC** | Regler und Szenenwechsel von Controllern oder anderer Software; Adresse `<knotenname>/<feld>` aus der Feldtabelle | ⚪ | alles; die Adressierung ist mit Block F vorbereitet (F6) | `Offene_Punkte` §7 (S83); `visuals/Feldsteuerung_Konzept.md` F6 |

## 4. Ablauf und Bedienung

| Idee | Was es wäre | Stand | Offen | Einzelheiten |
|---|---|---|---|---|
| **Tag View: Tags bearbeiten und online suchen** | Im Panel „Tag View" Tags nachträglich eintragen (in die Datei schreiben) und eine Online-Datenbank durchsuchen, wie beim CD-Rippen | 🟡 | gebaut S86: das Panel als reine Anzeige (Tags, Cover). Offen: Schreiben (braucht eine Tag-Bibliothek), Online-Suche (Opt-in) | `Offene_Punkte` §7 „Text-Knoten ausbauen", Absatz „Panel Tag View" |
| **Timeline / Composit** | Zeitleiste, die Visuals und Übergänge je Titel anordnet (statt Host Switcher), bis hin zum Offline-Rendern; Ideen S88: mindestens zwei Visual-Kanäle, Übergänge je Kanal mit Kurven-Editor, Karaoke zuschaltbar über ein Karaoke-Panel, Warnung bei `.lvfx2` mit Host Switcher, eigenes `.lvfx2`-Icon | ⚪ | Konzept-Session; Vorarbeit S84 (Ziehen aus Browser/Playlist); Entscheide S88: `.lvfx2` bleibt „Kette mit Host-Gruppe", Karaoke-Ebene als eigener Block vor der Timeline (Lyrics-Knoten bleibt, die Kette gewinnt) | `Offene_Punkte` §7 „Drag … Weg zur Timeline", Ausblick |
| **Visual-Playlist** | Abfolge von Visuals wie eine Musik-Playlist | ⚪ | hängt an offenen Entscheiden | `Offene_Punkte` §6 (P2), §5 |
| **Preset-Warmup / Pre-Roll** | Beim Laden, Resize und Vollbildwechsel einige hundert Frames vorrechnen — kein Schwarz-Start | ⚪ | alles | `Offene_Punkte` §7 (Sammelzeile) |
| **Startwert (Seed) im Preset** | Der Kaltstart-Zustand als Feld des Presets statt aus der Umgebung | 🟡 | Grundstein S76; Wahl des Seeds offen | `Offene_Punkte` §6 |
| **Rolle in der Effekt-Palette** | Render / Transform / Farbe / Steuerung sichtbar machen und danach filtern | 🟡 | Palettenfenster mit Rollen S79; offen die Rollen-Spalte im Kettenbaum | `Offene_Punkte` §7 „Effekt-Palette: Rolle sichtbar machen" |
| **Shader-Uniforms im Editor einfärben** | Eigene Uniforms und ISF-Eingänge im Skript-Editor hervorheben | ⚪ | alles | `Offene_Punkte` §7 (S85) |
| **Sprachwahl der Oberfläche** | Oberfläche umschaltbar (i18n) | ⚪ | alles | `Offene_Punkte` §7 |

## 5. Pflege dieser Seite

- Eine neue Idee kommt **zuerst** in `Offene_Punkte.md` (dort wird sie
  ausformuliert) und bekommt hier eine Zeile.
- Wird eine Idee gebaut, wandert der Stand auf 🟡 oder ✅; ist nichts mehr
  offen, fällt die Zeile weg — das Ergebnis steht dann im Changelog und im
  Benutzerhandbuch.

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 1.0.0 | 2026-10-01 | Session 86 — erste Fassung (Wunsch Patrik): 24 Ideen aus `Offene_Punkte.md`, den Konzeptdokumenten und den Sitzungsnotizen; neu aufgenommen „Text als Milkdrop-Element" |
