# Changelog Session 91 (2026-10-03)

> **Langsamer klopfen, und viele kleine Lücken geschlossen.** Das Panel
> Lyrics Sync spielt den Titel zum Klopfen langsamer ab. Der Effect Browser
> kennt Favoriten, der Lyrics Trail ein Skriptfeld je gegangener Zeile, und
> der Trail der Karaoke-Ebene springt beim Preset-Wechsel nicht mehr zurück.

## Neu

- **Lyrics Sync — langsamer abspielen:** der Schieber **Speed** (50–100 %)
  spielt den Titel langsamer ab, damit sich Zeilen und Worte in Ruhe klopfen
  lassen. Der Ton wird dabei tiefer. Die geklopften Zeiten bleiben Zeiten im
  Titel — sie passen bei normalem Tempo. Das Tempo gilt nur, solange das
  Panel sichtbar ist; verborgen läuft der Titel mit 100 %, der Schieber
  behält seinen Wert.
- **Favoriten im Effect Browser:** Favoriten tragen ein ★ vor dem Namen.
  Rechtsklick auf ein Preset → **Add to favorites** / **Remove from
  favorites**, ohne es zu laden. Der Filter **★ Favorites** zeigt alle
  Favoriten aus allen Ordnern als eine Liste; Bild ab / Bild auf blättern in
  dieser Ansicht durch die Favoriten. Es sind dieselben Favoriten wie beim
  Stern der Preset-Zeile.
- **Lyrics Trail — Skriptfeld Line:** läuft einmal je gegangener Zeile und
  liest `line` (die Zeile, die eben ging) und `linecount`. Damit lässt sich
  jede Zeile anders losschicken, etwa
  `xshift = rand(21) - 10; yshift = -6 - rand(8);`.

## Geändert

- **Load… im Effect-Chain-Panel** bietet jedes Preset an: `.lvfx`, `.lvfx2`,
  `.avs` und `.milk` in einem Dialog. Eine `.avs` oder `.milk` geht von dort
  an den passenden Import — auch wenn sie in **File → Load Effect Chain**
  über „All Files" gewählt wurde.
- **Panel Lyrics Sync:** Statuszeile und mindestens drei Listenzeilen bleiben
  immer sichtbar; ein zu kleines Panel scrollt, statt zu beschneiden.

## Behoben

- **Trail der Karaoke-Ebene beim Preset-Wechsel:** wechselte das Preset,
  während eine Zeile wegwehte, sprang der Trail zurück und wehte noch einmal
  von der Stelle der Zeile los.
- **Statuszeile im Panel Lyrics Sync** war in einem knappen Panel unter dem
  Wort-Streifen abgeschnitten.
- **Load… mit `.avs` / `.milk`:** eine über „All Files" gewählte Datei wurde
  als Effektkette gelesen und scheiterte.

## Noch nicht im laufenden Programm gesehen

Alles oben ist per Test, an gerenderten Bildern und Panel-Schnappschüssen
geprüft, aber nicht im laufenden Programm — besonders der Ton und das Klopfen
bei verlangsamtem Titel, der Sprung im Ton beim Verbergen des Panels, das
Rechtsklick-Menü im Effect Browser und der Preset-Wechsel bei laufendem
Trail.
