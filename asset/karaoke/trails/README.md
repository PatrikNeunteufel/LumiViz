# Trail-Presets der Karaoke-Ebene (mitgeliefert)

Ein Trail-Preset ist **die Trail-Liste der Karaoke-Ebene als Ganzes**: eine Effect
List mit ihren Kindern, der Lyrics Trail am Ende eingeschlossen. Gewählt wird es im
Panel **Karaoke**, Reiter **Lyrics Trail**, Zeile **Trail preset**. Eine Auswahl
ersetzt nur den Trail — Schrift, Farben und Lage der Zeile bleiben.

> **Diese Datei ist der Anker der Verzeichnissuche.** `TrailPresetStore` sucht vom
> Programmordner aufwärts nach `asset/karaoke/trails/README.md`. Wird sie entfernt,
> findet die App die mitgelieferten Trail-Presets nicht mehr.

## Aufbau

```
asset/karaoke/trails/<Name>.lvtrail
```

Die Datei trägt das gewöhnliche Kettenformat (wie `.lvfx`), Wurzel ist die Liste.
Ein Test liest jede Datei, rendert sie und prüft, dass sie genau so in der Datei
steht, wie die App sie schreiben würde — ein vertippter Feldname fällt dort auf.

Selbst gespeicherte Trail-Presets landen **nicht** hier, sondern im Benutzerordner
(`<AppData>/LumiViz/karaoke/trails/`). Bei gleichem Namen gewinnt der Benutzerordner;
mitgelieferte Dateien lassen sich in der App nicht überschreiben oder löschen.

In der Trail-Liste der Ebene sind nur Module der Rollen **Transform** und **Color**
vorgesehen; die mitgelieferten Presets halten sich daran.

## Eigene Trail-Presets mitliefern

In der App bauen, mit **Save As…** speichern, dann aus dem Benutzerordner hierher
kopieren.

## Katalog (Stand S92)

| Preset | Was es tut |
|---|---|
| **Drift Up** | das mitgelieferte Aussehen der Ebene: die alte Zeile zieht nach hinten, steigt und verblasst |
| **Fall Back** | schrumpft schnell zur Mitte, kurzer Nachzug |
| **Zoom Burst** | wächst dem Betrachter entgegen und löst sich auf |
| **Swirl** | dreht sich beim Verwehen um die Bildmitte |
| **Smoke** | weich, steigt langsam und schlängelt sich dabei |
| **Echo** | keine Bewegung: ein versetztes Nachbild, das lange steht und langsam verblasst |
| **Wave** | wellige Verformung, die zur Mitte zieht |
| **Melt** | läuft nach unten weg |
