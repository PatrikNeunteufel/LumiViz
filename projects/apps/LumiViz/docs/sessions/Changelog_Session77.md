# Changelog Session 77 (2026-09-23)

> **Der Visualizer kann nicht mehr dauerhaft verschwinden.** Nach einer
> Mausbewegung war er weg — auch nach jedem Neustart, nur im Vollbild zu
> sehen. Das gespeicherte Fenster-Layout enthielt ihn nicht mehr, und keiner
> der naheliegenden Auswege (Neustart, „Reset Layout", Menü View > Panels)
> holte ihn zurück.

## Behoben

- **Fehlender Visualizer wird beim Start wieder angedockt.** Steht er nicht
  im gespeicherten Layout, setzt LumiViz ihn rechts neben die vorhandenen
  Panels (nicht als Tab dahinter) und vermerkt das im Log.
- **„Reset Layout" setzt wirklich zurück** — auf das Werks-Layout. Vorher
  stellte es genau das gespeicherte Layout wieder her, ein kaputtes also
  erneut.
- **Der Visualizer steht in View > Panels** (oben, abgesetzt von den Panels)
  und lässt sich dort aus- und wieder einblenden. Das gilt auch für weitere
  Visualizer aus „New Visualizer".

## Bekannte Grenze

Das **X am Visualizer-Tab** löscht den Visualizer weiterhin (samt
Menü-Eintrag); zurück über „New Visualizer" oder einen Neustart. Ob der
erste Visualizer stattdessen nur ausgeblendet werden soll, ist offen
(`Offene_Punkte.md` §7).

## Nothilfe für ältere Builds

App schließen, Registry-Schlüssel
`HKCU\Software\LumiViz Project\LumiViz\DockManager` löschen — beim nächsten
Start gilt das Werks-Layout.

## Doku

- `include/UI/managers/DockManager.md` 2.1.0 → **2.2.0** (neuer §5.6)
- `docs/Offene_Punkte.md` 1.72.0 → **1.73.0**

Version der App unverändert (0.4.0).
