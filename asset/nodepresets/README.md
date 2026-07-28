# Knoten-Voreinstellungen (mitgeliefert)

Benannte Parametersätze für einzelne Knoten der Effektkette. Eine Voreinstellung ist
**die `params` eines Knotens unter einem Namen** — einschließlich seiner EEL-Formeln,
aber ohne Kinder und ohne Anzeigenamen.

> **Diese Datei ist der Anker der Verzeichnissuche.** `NodePresetStore` sucht vom
> Programmordner aufwärts nach `asset/nodepresets/README.md`. Wird sie entfernt,
> findet die App die mitgelieferten Voreinstellungen nicht mehr.

## Aufbau

```
asset/nodepresets/<typkey>/<Name>.json
```

`<typkey>` ist der Schlüssel aus `effectTypeKey()` — derselbe, der auch in einer
gespeicherten Kette (`.lvfx`) unter `"type"` steht: `superScope`, `movement`,
`dynamicMovement`, `metaballs3d`, `list` …

Selbst gespeicherte Voreinstellungen landen **nicht** hier, sondern im Benutzerordner
(`<AppData>/MyViz/nodepresets/<typkey>/`). Bei gleichem Namen gewinnt der
Benutzerordner; mitgelieferte Dateien lassen sich in der App nicht überschreiben oder
löschen.

## Dateiformat

```json
{
  "format": "lumiviz-nodepreset",
  "formatVersion": 1,
  "node": {
    "type": "<typkey>",
    "<parameter>": <wert>
  }
}
```

Der `node`-Block ist derselbe, den `ChainSerializer::nodeToJson` für diesen Knoten
schreiben würde — nur ohne `name`, `description`, `enabled` und `children`. Beim Laden
wird `type` gegen den Knoten geprüft: eine Voreinstellung eines anderen Typs wird
abgelehnt, nicht stillschweigend zum Passthrough.

## Teil-Presets

**Laden ist ein Merge, kein Ersatz:** Eine Datei überschreibt genau die Felder, die sie
enthält — alles andere bleibt stehen. Eine Datei muss also nicht alle Parameter tragen.

Die **SuperScope-Figuren** in `superScope/` sind genau das: sie enthalten nur die vier
EEL-Slots und `pointCount`. Wer eine Figur lädt, behält deshalb seine Farbtafel,
Linienbreite und den Blend-Modus — dasselbe Verhalten wie das frühere
„Figure"-Dropdown, das sie ersetzt haben.

In der App entsteht ein Teil-Preset über die Feldauswahl im Dialog **Speichern
unter…**: abgehakte Felder kommen in die Datei, abgewählte nicht.

## Eigene Voreinstellungen mitliefern

Datei in den passenden `<typkey>`-Ordner legen — mehr ist nicht nötig, die App liest
das Verzeichnis bei jedem Öffnen des Editors neu. Am einfachsten entsteht so eine
Datei in der App selbst (**Speichern unter…** in der Zeile „Voreinstellung"); danach
aus dem Benutzerordner hierher kopieren.
