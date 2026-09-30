# Changelog Session 84 (2026-09-30)

> **Ziehen statt Dateidialog, Musik mit Cover, und jedes Feld per Formel.**
> Presets und Musik lassen sich per Drag & Drop dorthin bringen, wo sie
> gebraucht werden; der neue Audio Browser zeigt Albumcover. Die Feld-Tooltips
> sind englisch, und mit Block F ist jedes Zahlen-, Schalter- und Farbfeld
> eines Knotens mit Parameter-Skript steuerbar.
>
> **⚠ Bekanntes Problem:** Nach diesen Änderungen laufen manche Presets
> schneller oder langsamer, und der Wechsel in den Vollbildmodus kann kurz
> hängen. Die Ursache wird in der nächsten Session gesucht
> (`docs/Regression_S84_Tempo_Vollbild.md`).

## Neu

- **Presets in die Effect Chain ziehen** — aus dem Effect Browser oder dem
  Windows-Explorer; dort entsteht ein **Presetverweis** an der Ablagestelle
  (auf eine Liste = hinein, an eine Kante = dazwischen, Leerfläche = Ende).
- **Audio Browser** (View → Panels): Musikordner mit **Albumcover als Icon**
  (eingebettet in MP3/FLAC/M4A, sonst `cover.jpg`/`folder.jpg` im Ordner, sonst
  ein Icon je Format). Doppelklick = an die Playlist hängen und abspielen,
  Ziehen = in die Playlist oder auf Player/Visualisierung.
- **Ziehen zum Abspielen:** Audiodatei oder Playlist-Eintrag auf das
  Player-Panel oder das Bild der Visualisierung = abspielen bzw. zum Eintrag
  springen.
- **Playlist nimmt Dateien an** — aus Audio Browser und Explorer, an der
  Stelle, wo man sie fallen lässt; Einträge lassen sich herausziehen.
- **Jedes Feld per Formel (Block F):** in Knoten mit Parameter-Skript ist jedes
  Zahlen-, Schalter-, Aufzählungs- und Farbfeld unter seinem Namen steuerbar
  (z. B. `smooth = 1;` im Mirror). Die ⓘ-Hilfe listet alle Felder mit Bereich
  und Erklärung, der Editor färbt sie. Felder, die beim Ändern neu aufbauen
  (Gittergröße, Verzögerung …), folgen dem Skript nur aus Init, Song und Beat.

## Geändert

- **Feld-Tooltips sind englisch.**
- **Vorlagen im großen Shader-Editor:** die Zeile „Template:" behält die Wahl
  und zeigt „(unsaved) – Name", sobald man etwas ändert.
- Neue Kamera-Clips heißen `camera_clip_<Datum>_<Uhrzeit>.mp4`; alte bleiben
  gültig.

## Behoben

- **Playlist:** Umsortieren per Ziehen änderte nur die Anzeige — ein
  Doppelklick spielte danach den falschen Titel.
- **Strange Attractor und Flame** lasen in jedem Bild alle eigenen
  Verlaufsdateien von der Festplatte neu ein.
