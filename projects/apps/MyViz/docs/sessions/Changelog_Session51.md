# Changelog Session 51 (2026-07-26/27)

Fokus: aus einer einzigen Beobachtung („ein SuperScope neben die Texer gesetzt →
sofort ein Bild, und es bleibt beim Deaktivieren stehen") wurden fünf Befunde am
AVS-Import. Dazu zwei Aufträge: das Import-Protokoll als Knoten statt als Dialog,
und eine Hotkey-Schicht samt Konzept.

## Texer II und Triangle — drei Befunde

- **`w`/`h` erreichten das Skript nie.** Alle vier Texer in „Alien Alloy" rechnen
  ihre Sprite-Zahl im **Frame**-Slot (`n=w*0.1`) und die Größe aus `h`
  (`reg00=h/280`). Ohne die Bildschirmmaße war `n=0` → kein einziges Sprite, das
  Preset blieb schwarz. Per Paar-Sonde gegen AvsRef gepinnt: `n=w*0.1` liefert
  6526 gezeichnete Pixel, genau wie das Literal `n=32` bei 320×240. `w` wird auch
  schon im **Init** gebraucht. Triangle hatte dasselbe Loch.
- **Slot-Reihenfolge von `sizex/sizey`**: die neutrale Vorbelegung stand *nach* dem
  Frame-Slot und löschte, was das Skript gerade gesetzt hatte. Die Referenz
  unterscheidet die Slots klar — `sizex=2` im Init zeichnet klein (2247 px),
  dasselbe im Frame-Slot groß (8945): AVS belegt je Frame neu vor, der Init-Wert
  überlebt nicht.
- **Triangle zeichnet gefüllte Dreiecke**, keine Umrisse. Sonde mit 16 Dreiecken:
  Referenz 23424 px, unser Drahtgitter 4009 und der Schwerpunkt 12 Zeilen daneben.
  Jetzt über die Flat-Fill-Stufe aus Session 48 (24448 px).

## Import-Kollisionsregel `_p` (Entscheid D2 umgesetzt)

Neu: **`include/scripting/ScriptBaseKeys.hpp`** — der in
`Vereinheitlichung_Konzept.md` §4 vorgesehene SSOT-Header mit **Herkunfts-Feld**
(avs/milk/lumi) und Umbenennungs-Schema.

Namen des einheitlichen Lumi-Sets, die in AVS **keine** Builtin-Bedeutung haben,
werden beim `.avs`-Import auf `_p` umbenannt — einheitlich über alle Slots der
Komponente, `_p2` bei belegtem Ziel, case-insensitiv geprüft, mit ℹ-Zeile im
Import-Report. Belegt an der Referenzquelle: im ganzen `vis_avs`-Baum sind **27**
Namen `registerVar`-registriert; `bass`, `mid`, `treb`, `treble`, `vol`, `time`
und `dt` ist keiner davon. `beat` registriert allein `r_list`, dessen Codes über
`ListInfo` laufen und darum unangetastet bleiben.

Anlass: „Alien Alloy" führt in `vol` einen Tiefpass
(`vol=vol*0.9+getspec(0.5,1,0)`), der Swirl-Stärke **und** Zeitschritt der Dynamic
Movement treibt. Weil die Injektions-Schicht `vol` je Frame setzte, konnte der
Akkumulator nie wachsen und der Inhalt wanderte nur halb so weit ins Bild
(34184 gegen 59043 gezeichnete Pixel). Nach dem Fix sind beide Transport-Sonden
pixelgleich (59043/59043 und 60478/60478, MAE 0,000).

Umbenennungs-Ziele sind **kanonisch kleingeschrieben**: EEL ist case-insensitiv,
das transpilierte Lua nicht — `VOL_p` und `vol_p` wären zwei verschiedene
Lua-Variablen und der Zustand wieder zerteilt.

## ChainSerializer — Schlüsselkollision

Der Set-Render-Mode-Visitor schrieb sein Override-Flag unter **denselben**
JSON-Schlüssel `enabled` wie der Knoten selbst (`nodeToJson` setzt ihn vor dem
Visitor, der ihn dann überschrieb). Der Auge-Zustand solcher Knoten ging beim
Speichern verloren, und beim Laden lasen beide Flags denselben Wert. Jetzt
`overrideBlend`, mit Rückfall auf das Alt-Layout; 13 eingefrorene `.lvfx`-Zwillinge
neu erzeugt (14 Zeilen Diff, ausschließlich der neue Schlüssel).

## Import Notes statt Dialog-Rauschen

`TranslationResult` trennt jetzt **Probleme** (`report`) von **Hinweisen**
(`notes`). Nur Probleme öffnen ein Meldungsfenster; das vollständige Protokoll
hängt als schreibgeschützter **„Import Notes"-Knoten** in der Kette (erstes Kind
nach Render Scale), und zwar nur, wenn es etwas zu protokollieren gibt — die
Anwesenheit des Knotens ist selbst das Signal. Ohne Zeitstempel, damit das
Import-JSON reproduzierbar bleibt.

Vorher liefen die planmäßigen `_p`-Umbenennungen mit durch den Dialog: „Milky Way
Xtreme" erzeugte **22** Zeilen, „High Voltage" 11 — echte Probleme gingen darin
unter. Jetzt melden beide 0 Probleme und öffnen keinen Dialog.

## Hotkeys (Stufe 1)

- `Bild ab` / `Bild auf` blättern durch die Presets des **aktiven Verzeichnisses
  im Import-Browser**. Ordner werden übersprungen, am Ende wird angehalten, und
  das Panel muss weder sichtbar noch fokussiert sein.
- **Einstellungen → Hotkeys**: Aufnahmefeld je Aktion, Kollisionsprüfung (eine
  belegte Taste wird abgelehnt statt still umgehängt), Zurücksetzen je Zeile und
  für alle. Nur Abweichungen landen in `QSettings`.
- Die **Transporttasten** (Leertaste, Ctrl+Pfeile, Medientasten) sind dauerhaft für
  die Musikwiedergabe reserviert und können nicht an Preset-Aktionen vergeben
  werden; sie sind gelistet, haben aber noch keine Funktion.
- Technisch ein Ereignisfilter auf `qApp`, **kein** `QShortcut`: das greift vor dem
  Fokus-Widget und verschluckt die Taste auch dann, wenn nichts passiert — ein
  `Bild ab` im EEL-Editor hätte das Preset gewechselt statt zu blättern.
- Neu: `docs/ui/Hotkey_Konzept.md` (Ausbaustufen aktives Verzeichnis →
  Visual-Playlist → Composer mit *einer* Belegung, Reservierungs-Regel,
  Aktions-Modell, Staffelung „erweiterter" Hotkeys).

## Werkzeuge und Doku

- **Sonden-Stufe `5_vars`**: Paar-Sonden — dasselbe Ergebnis einmal über eine
  Host-Variable, einmal über das Literal, das bei 320×240 herauskommt. Stimmen die
  *Referenz*bilder des Paares, ist nicht nur die Existenz der Variablen belegt,
  sondern ihr **Wert**; dieselbe Bauweise pinnt Slot-Reihenfolgen.
- `avs_preset_lib`: Triangle-Bauer + `cstr` (NtString). Weitere Sonden: die
  Alien-Alloy-Dynamic-Movement verbatim, Transport mit Saat nur am Rand, drei
  Zwischenstufen zum statischen Swirl, Skript-Zustand als Zeilenlage über `reg`,
  Colorfade mit asymmetrischen Werten.
- **Methodik §6 „Import-Roundtrip"**: importieren, als `.lvfx` speichern, JSON
  lesen — der billigste Parser-Test, und der Weg zu beiden Hauptbefunden. Dazu die
  Anschlussfrage „welche Variablen liest das Skript, und setzt der Host sie alle?"
  und der Hinweis, dass Modul-Matrix und Sonden die Kollisionsregel strukturell
  nicht bewachen.
- Visual-Playlist-Konzept 0.2.0 (Hotkeys ausgelagert, §5a Ausblick **Composer**:
  mp3 und Presets in Spuren über der Zeit) · Benutzerhandbuch §9 ·
  `AvsChainTranslator.md` auf `notes`/`report` nachgezogen.

## Verifikation

- **MyViz.UnitTests 442/442 grün, 0 Skips** (Session 50: 432) — neun neue Cases.
- **Modul-Matrix 37/41**, dieselben vier Reste mit identischen Werten.
- **Modul-Sonden 40/41**, offen nur `convolution_kante`.
- **`.lvfx`-Zwillinge 65/65** stimmen mit der Übersetzung überein.
- Alien Alloy 0,774 → **0,683** · Energy Crystal 0,123 → **0,018**.
- **Wichtig zur Einordnung:** die Whacko-Sweep-Zahlen aus Session 50 sind **nicht
  vergleichbar** (anders parametrierter Lauf). Per Vorstand-Messung bewiesen: zwei
  scheinbare Regressionen liefern auf dem Vorstand identische Werte. Genau ein
  Preset hat sich echt verändert (Inhaler 0,246 → 0,416) — dort läuft `dt` jetzt
  korrekt als Preset-eigener Akkumulator, aber von einem Zufalls-Startwert.
- Nicht automatisiert geprüft: das Fokus-Verhalten des Hotkey-Filters (Sichttest,
  inklusive Vollbild).
