# Changelog Session 78 (2026-09-27)

> **Die Effektkette hält ihre Felder in Form.** Jedes Feld jedes Moduls ist
> mindestens 120 px breit und wächst mit dem Panel; ist das Panel schmaler als
> der Inhalt, scrollt es seitlich — von den Beschriftungen zu den Feldern —,
> statt Felder über den Rand hinauslaufen zu lassen.

## Geändert

- **Felder:** mindestens 120 px, 5 px Abstand zum Panelrand. Beschriftung und
  Feld bleiben nebeneinander.
- **Schmalste Panelbreite:** etwa zwei Felder (240 px plus Ränder). Das Panel
  lässt sich nicht mehr bis auf nichts zusammenziehen.
- **Seitliches Scrollen** des ganzen Panels, sobald es schmaler als der Inhalt
  ist; breite Module wie der 7×7-Kernel (Convolution) scrollen früher, statt
  das Panel zu verbreitern.
- **Auswahllisten** richten sich nicht mehr nach ihrem längsten Eintrag;
  aufgeklappt zeigen sie die Einträge voll.
- **Beschreibungstexte** (Bloom, Camera 3D, SuperScope 3D …) brechen um.
  Sehr lange Wörter (URLs, Pfade) werden am Rand abgeschnitten.
- **Code-Felder** (per frame, per beat, Shadertoy …) bleiben in der
  Panelbreite und scrollen in sich.
- **Farbtafel** (SuperScope, Metaballs 3D, Tentacles 3D …): vier Farben je
  Zeile statt einer immer längeren Zeile.
- **Symbolknöpfe:** „Save as…" der Voreinstellungen ist jetzt 💾, Shadertoy
  „Importieren" jetzt ⤓ — der Text steht im Tooltip.
- **Effektliste:** Spalten haben Mindestbreiten (Name 120, Typ 90,
  Beschreibung 80); reicht der Platz nicht, scrollt nur die Liste.

## Doku

- `include/UI/panels/MultiEffectPanel.md` — neuer Abschnitt „Feldbreiten"
- `docs/Offene_Punkte.md` 1.73.0 → **1.74.0**

Version der App unverändert (0.4.0).
