# Composer — Stueck-Presets

Mitgelieferte Kurvenformen fuer Stuecke mitten im Clip (`.lvshape`). Die Form
sagt, wie viel vom Grundpegel des Clips bleibt: 1 alles, 0 nichts. Gekippt
(`flipped` am Clip) tauschen oben und unten — aus einer Delle wird ein Buckel.
Eigene Presets liegen im Benutzerordner `<AppData>/composer/shapes/`.

Die Dellen (`Dip …`) beginnen und enden auf dem Grundpegel. `Late fade in`
(S100) ist die Ausnahme: es beginnt unten und gehoert an den Clip-Anfang — der
Clip laeuft unsichtbar und blendet im letzten Teil des Stuecks ein.

Diese Datei ist der Anker, an dem die App den Ordner findet — nicht loeschen.
Beschreibung: `projects/apps/LumiViz/docs/ui/Composer_Konzept.md` §9.
