# LumiViz — Screenshots und Fehler-Ablage (Konzept)

> **Version:** 1.0.0
> **Datum:** 2026-07-27
> **Typ:** Konzept (**umgesetzt**, Session 52)
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Hotkey_Konzept.md](Hotkey_Konzept.md) (Aktion `view.screenshot`) ·
> `UI/managers/ScreenshotManager` · `UI/widgets/VisualizerRenderThread` ·
> [AVS_Kalibrier_Methodik.md](../visuals/AVS_Kalibrier_Methodik.md)
> **Sprache:** Deutsch

---

## 1. Wozu

Beim Kalibrieren wird ein Preset nach dem anderen durchgeblättert und beurteilt.
Was dabei fehlte: **ein Beleg**. Der Bildschirm-Screenshot von Windows nimmt das
ganze Fenster samt Panels auf, weiß nichts vom Preset und landet in der
Zwischenablage — eine halbe Stunde Blättern hinterlässt nichts Auswertbares.

`Druck` nimmt deshalb **das Visual** auf, benennt die Datei nach dem Preset und
legt den vollen Pfad daneben. Ein Programmlauf = ein Ordner.

---

## 2. Was entsteht

```
<Basis>/2026-07-27_11-47-57/          ← ein Ordner je Programmstart
    Alien Alloy_avs.png               ← nur das Visual, physische Pixel
    Alien Alloy_avs.txt               ← C:\…\VisualsPresets\avs\Alien Alloy.avs
    Alien Alloy_avs_2.png             ← zweite Aufnahme desselben Presets
    Alien Alloy_avs_2.txt
    fehler.log                        ← nur wenn im Vollbild etwas schiefging
```

| Regel | Warum |
|---|---|
| Ordnername = **Startzeit des Programms** | nicht die des ersten Bildes — sonst heißt der Ordner nach einem beliebigen Tastendruck |
| Ordner wird **verzögert** angelegt | ein Lauf ohne Screenshot hinterlässt keine leeren Verzeichnisse |
| Dateiname = Presetname **mit Endung** (`_avs`, `_lvfx`) | `.avs` und sein `.lvfx`-Zwilling teilen den Basisnamen; ohne Endung überschrieb der zweite den ersten (Befund S45) |
| Verbotene Zeichen → `_` | AVS-Presetnamen tragen regelmäßig `:` oder `?` |
| Zweite Aufnahme → `_2`, `_3`, … | stilles Überschreiben wäre das Gegenteil des Zwecks |
| `.txt` mit **absolutem** Pfad | gleichnamige Presets liegen in mehreren Sammlungen |

**Basisordner:** `QSettings`-Schlüssel `screenshot/baseDir`. Ohne Eintrag wird
`asset/calibration` vom Programmverzeichnis aufwärts gesucht (dieselbe Suche wie
bei den Preset-Icons, die Exe liegt tief in `out/`) und `screenshot` daran
gehängt. Außerhalb des Projektbaums: *Bilder*`/LumiViz`.

---

## 3. Warum die Aufnahme im Render-Thread sitzt

`glReadPixels` braucht den aktuellen GL-Kontext, und den hält allein der
Render-Thread. Der Hotkey setzt deshalb nur eine Marke
(`VisualizerRenderThread::requestCapture()`); aufgenommen wird **nach dem
Rendern und vor `swapBuffers`** — dort trägt der Standard-Framebuffer genau das
Bild, das gleich zu sehen ist. Das fertige Bild reist als `frameCaptured` per
queued signal in den GUI-Thread, der es schreibt.

Die Rezeptur ist die des `AvsStandalone` (dort seit S44/S45 erprobt):

1. **Physische** Pixel (`width() * devicePixelRatio()`) — mit logischer Größe
   liest man bei DPI-Skalierung nur den linken unteren Ausschnitt.
2. Vertikal spiegeln — `glReadPixels` liefert Zeile 0 von **unten**.
3. Als RGB888 speichern — FBO-Alpha ist kein Bildinhalt, Alpha-0-Pixel
   erscheinen im Betrachter sonst als weiße Phantom-Linien.

Weil das Bild einen Frame später kommt, merkt sich die Anforderung den
**Preset-Stand zum Zeitpunkt des Drucks**: beim schnellen Blättern wäre sonst
schon das nächste Preset geladen, wenn das Bild eintrifft.

---

## 4. Fehler im Vollbild (Entscheid Patrik, Session 52)

> **Im Vollbild erscheint kein Meldungsfenster.** Stattdessen: eine Zeile in
> `fehler.log` **und** ein automatischer Screenshot. Im Fenster bleibt der Dialog.

Begründung: das Vollbild ist ein randloses eigenes Fenster. Ein Dialog landet
dahinter oder reißt die Vorführung auseinander, und beim Durchblättern einer
Sammlung steht er nach jedem zweiten Preset im Weg. Der Screenshot ersetzt ihn
nicht nur — er ist mehr wert, weil man **sieht**, wie das Preset zum
Fehlerzeitpunkt aussah.

Eine Zeile im Protokoll:

```
2026-07-27T11:52:03  [Import AVS Preset]
  Preset: C:\…\VisualsPresets\avs\Whacko V\Bright Light District.avs
  Passthrough: Effekt-ID 42 unbekannt
```

Der einzige Ort, der das entscheidet, ist `MainWindow::reportProblem()` — alle
Import- und Ladewege gehen hindurch, damit die Regel nicht an einer Stelle
vergessen wird.

---

## 5. Grenzen

- **Windows kann `Druck` selbst belegen.** Öffnet die Taste bei Dir das
  Snipping Tool, ist das eine Windows-Einstellung (*Barrierefreiheit → Tastatur*)
  außerhalb der App. Die Aktion `view.screenshot` lässt sich in
  *Einstellungen → Hotkeys* auf jede andere Taste legen.
- Aufgenommen wird der **Inhalt des Visualizers**, nicht das Fenster mit Panels.
- Presets, die über den Import-Browser geladen werden, sind bekannt; eine von
  Hand gebaute Kette heißt `visual`.

---

## 6. Changelog

- **1.0.0** (2026-07-27, Session 52): Erstfassung — Ablage-Schema, Ordner je
  Programmlauf, Namensregeln, Aufnahme im Render-Thread, Vollbild-Regel für
  Fehler.
