# Changelog Session 85 (2026-09-30 – 2026-10-01)

> **Die Regression ist gefunden, und Felder bewegen sich jetzt auch ohne
> Formel.** Das Tempo- und Lag-Problem aus Session 84 war ein Ladefehler bei
> Presetverweisen. Jedes Feld der Fraktale, 3D-Knoten und anderer Module ist
> per Formel steuerbar, Regler starten ein laufendes Skript nicht mehr neu, und
> der neue **~**-Knopf lässt Zahlfelder zur Musik oder per LFO schwingen.

## Behoben

- **Host Switcher mit Presetverweisen** (z. B. `list.lvfx2`): nach dem Laden
  liefen die Szenen zu schnell oder zu langsam und das Bild ruckelte, bis man
  im Panel etwas anklickte. Die verwiesenen Presets teilten sich intern einen
  Zustand. Jetzt bekommt jeder Knoten beim Laden seine eigene Identität.
- Das Vergleichswerkzeug **AvsStandalone** ließ sich seit Session 83 nicht mehr
  bauen.

## Neu

- **Werte ohne Formel bewegen (Modulation):** ein **~**-Knopf neben 257
  Zahlfeldern in 56 Modulen. Quelle Bass, Mid, Treble, Level, Beat envelope
  oder ein LFO (sine, triangle, saw, square, random); Depth als Anteil des
  Feldbereichs, Rate, Smoothing/Decay. Daneben läuft der aktuelle Wert mit;
  der Knotenschalter **Modulation active** schaltet alle Modulationen eines
  Knotens. Reihenfolge: Regler → Modulation → Formel.
- **Jedes Feld per Formel** jetzt auch bei Fraktalen, 3D-Knoten, Glow Orbs,
  Texer II, Triangle, Bump und Co. (etwa `type`, `smooth`, `blend`).
- Die **ⓘ-Hilfe jedes Skriptfelds** sagt zuerst, wann es läuft: einmal, je
  Songwechsel, je Bild, je Beat oder je Punkt (Punkt, Sprite, Dreieck,
  Gitterknoten, Orb …).

## Geändert

- **Regler bewegen, während ein Skript läuft:** der Regler setzt den Wert, das
  Skript rechnet von dort weiter — Zähler, Phasen und Zufall bleiben erhalten.
  Früher begann das Skript bei jeder Reglerbewegung von vorn.
- **Ganze Zahlen aus Formeln** werden abgeschnitten wie in AVS (3,7 Punkte →
  3), bei allen Feldern einheitlich.
- Eigene Shader-Uniforms oder ISF-Regler, die wie ein Feld des Knotens heißen,
  heißen im Skript `in_<name>`.

## Bekannt

- Beim **ersten** Wechsel auf ein AVS-Preset kann das Bild kurz hängen; beim
  zweiten Mal nicht mehr. Wird später untersucht.
- Noch nicht modulierbar: Farben, Schalter, Auswahlfelder sowie SuperScope,
  Movement, Dynamic Movement, Effect List und Host-Gruppen (SuperScope und
  Effect List kommen als Nächstes).
