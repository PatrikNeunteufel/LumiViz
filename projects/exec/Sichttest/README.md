# Sichttest — eine Sichttest-Liste abhaken

**Zweck:** Ein eigenes Fenster, das neben dem geprüften Programm läuft. Es
führt Punkt für Punkt durch eine Liste; jeder Punkt bekommt **Pass**, **Pass
mit Befund**, **Fail** oder **Überspringen**, dazu eine Bemerkung und
Screenshots. Vorbild ist das Abnahme-Protokoll des UART-Studios
(`TestProtokollWindow`), hier ohne Kopplung an eine Anwendung: das Werkzeug
kennt LumiViz nicht und lässt sich herauslösen.

**Exe (Release-clang):**
`out/build/windows-ninja-release-clang/exec/Sichttest/bin/Release/Sichttest.exe`

Bauen: `cmake --build --preset build-ninja-release-clang --target Sichttest`

## Aufruf

```bash
Sichttest [datei|ordner]
Sichttest --pruefe <datei>
Sichttest --schnapp <png> [datei|ordner]
```

- Ohne Argument: der zuletzt benutzte Ordner und das zuletzt benutzte
  Protokoll; beim ersten Start der Ordner `.claude/handover`, vom
  Arbeitsverzeichnis oder vom Ort der Exe aufwärts gesucht.
- `--pruefe` gibt die erkannten Schritte auf die Standardausgabe — zum
  Nachsehen, was der Leser aus einer Liste macht.
- `--schnapp` baut das Fenster auf, legt es als Bild ab und endet.

## Was es liest

**Markdown-Checklisten** — jede `*.md` des Ordners, die Punkte enthält:

```markdown
# Titel der Liste

Vorspann (Beschreibung). Eine Exe in Backticks, etwa `out\…\LumiViz.exe`,
wird erkannt: das Fenster zeigt ihren Zeitstempel und kann sie starten.

## A. Abschnitt

- [ ] **A1 Titel des Punkts:** Was zu tun ist und was zu sehen sein muss,
      über mehrere eingerückte Zeilen.
```

Der Anfang des Fettdrucks ist die Kennung (`A1`), sofern er wie eine aussieht;
sonst zählt das Werkzeug durch (`P7`). Haken, die schon in der Datei stehen,
gelten als Ausgangsstand: `[x]` Pass, `[!]` Fail, `[-]` übersprungen. **Die
Liste selbst wird nie verändert.**

**`*.testprotokoll.json`** — das Format des UART-Testers: `title`,
`description`, `steps` mit `id`, `section`, `title`, `text` (Markdown).
Optional `exe`.

## Was es schreibt

Neben die Liste, in den Unterordner `sichttest-logs/`:

| Datei | Inhalt |
|---|---|
| `<Liste>_<Zeit>.testlog.json` | der Lauf: je Schritt `result` (`pass`, `pass_remark`, `fail`, `skip`, `open`), `remark`, `screenshots`, `rated`; dazu `build` (Exe, Zeitstempel) und `summary` — Format des UART-Testers |
| `<Liste>_<Zeit>.report.md` | derselbe Stand zum Lesen: zuerst Fail und Pass mit Befund samt Bemerkung und Bildern, dann Übersprungen, Offen, Pass |
| `<Liste>_<Zeit>_<Kennung>_<n>.png` | die Screenshots |

Geschrieben wird nach jeder Bewertung, nach jedem Screenshot, beim Wechsel
des Schritts und beim Schließen. Beim nächsten Start wird der **jüngste Lauf**
der Liste fortgesetzt; **▶ Neuer Lauf** beginnt frisch. Ändert sich der Text
der Liste, kommt er beim Neu-Einlesen (↻) an — die Bewertungen hängen an der
Kennung.

## Bedienung

| Was | Wie |
|---|---|
| Pass / Pass mit Befund / Fail / Überspringen | Knöpfe oder Strg+1 / 2 / 3 / 4; danach springt das Fenster zum nächsten offenen Punkt |
| Bewertung zurücknehmen | ○ Offen |
| Screenshot | Win+Shift+S, dann **📋 Screenshot anhängen**, Strg+Shift+V oder Strg+V im Bemerkungsfeld; Bilddateien ins Fenster ziehen |
| Screenshot ansehen / löschen | mit der Maus darüber (Vorschau), Doppelklick öffnet, Entf oder Rechtsklick löscht |
| Fenster über dem Programm halten | Haken **Im Vordergrund** |
| nur das Unerledigte | Haken **Nur offene und Fail zeigen** |
| Befunde weitergeben | **Report-Pfad kopieren** und im Chat einfügen |

Fail und Pass mit Befund verlangen eine Bemerkung oder einen Screenshot.
