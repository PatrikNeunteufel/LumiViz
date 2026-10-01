# Changelog Session 86 (2026-10-01)

> **Jedes Feld per Skript — auch dort, wo es bisher keines gab — und Musik, die
> ihren Text mitbringt.** SuperScope, Effect List, Dynamic Movement, Host-Gruppe,
> Host Switcher, Text und AVI lassen sich jetzt vollständig per Formel steuern.
> Farben schwingen mit, LFOs laufen im Takt. Der Text-Knoten zeigt Titel und
> Interpret, und der neue Knoten **Lyrics** zeigt den Songtext zum laufenden
> Stück.

## Neu

- **Knoten „Lyrics":** Songtext zum laufenden Titel. Synchroner Text läuft an
  der Abspielposition mit, die laufende Zeile ist hervorgehoben, darunter
  stehen Vorschau-Zeilen; mit Wort-Zeiten füllt sich die Zeile Wort für Wort.
  Duette bekommen eine Farbe je Stimme. Unsynchroner Text steht als Block.
  Gesucht wird in der Datei selbst, neben dem Song und im Benutzerordner.
- **Songtexte aus dem Netz** (aus, bis du es einschaltest): Settings → Audio →
  *Search lyrics online*. Ein Fund wird gespeichert — im Benutzerordner oder
  neben dem Song — und trägt in der Datei seinen Herkunftsvermerk.
- **Titel und Interpret im Text-Knoten:** Platzhalter `{title}`, `{artist}`,
  `{album}`, `{year}`, `{genre}`, `{track}`, `{file}`; eckige Klammern machen
  einen Teil wahlfrei. Knopf **Insert tag**, Voreinstellung **Now Playing**.
- **Panel „Tag View":** Tags, Datei, Dauer und Cover des laufenden Titels.
- **Text-Knoten:** Schrift aus einer Liste der installierten Schriften wählen,
  Häkchen **Bold**; eigene Formelfelder (Init, Frame, Beat) — ebenso beim
  **AVI**-Knoten.
- **Farbfelder modulieren:** der **~**-Knopf steht auch an Farben; Ziel
  Brightness, Hue oder ein Kanal.
- **LFO im Takt:** *Beat sync* im Modulations-Dialog — eine Schwingung dauert
  so viele Beats, wie du angibst, und beginnt auf dem Beat.
- **Mehr Felder per Formel:** SuperScope (`linewidth`, `dotsize`,
  `rendermode` …), Effect List (`blendout`, `bufferin`, `outadjustalpha` …),
  Dynamic Movement (`wrap`, `nomove`, `subpixel` …), Host-Gruppe und Host
  Switcher. Die AVS-Namen (`n`, `linesize`, `alphain` …) behalten Vorrang.
- **Mehr Felder modulierbar:** SuperScope, Effect List, Host-Gruppe, Dynamic
  Movement, Text, AVI, Host Switcher.
- **Effekt-Palette:** Text, AVI, Comment und Lyrics stehen jetzt in der Liste.
- **Voreinstellungen:** acht für Text, fünf für AVI, drei für Lyrics.
- **Datei-Icons:** ein Icon für `.lrc`; alle Icons liegen als `.ico` bereit.

## Geändert

- **Fenstergröße ändern:** das Bild der Kette, jeder Effect List und jeder
  Host-Gruppe bleibt erhalten. Blur und Trails brechen beim Ziehen am
  Fensterrand nicht mehr ab.
- **Effect List mit Listen-Code:** `alphain` und `alphaout` stehen zu Beginn
  jedes Bilds auf dem Reglerwert, wie in AVS. Der Alpha-Regler wirkt damit
  auch bei laufendem Listen-Code sofort.

## Behoben

- Auswahlfelder wie **Clear → Blend**, **Buffer Save → Blend/Dir** oder
  **Starfield → Blend** sprangen auf den ersten Eintrag, sobald der Knoten ein
  Skript trug.
- Text, AVI und Comment ließen sich nur per Import anlegen — sie fehlten in
  der Effekt-Palette.

## Bekannt

- Wird ein Songtext online geholt, erscheint er erst beim nächsten Abspielen
  des Titels; die Anzeige im Editor bleibt bis dahin auf „No lyrics found".
- Eingebettete Songtexte und die Tags Jahr, Genre, Tracknummer sind noch nicht
  an vielen Dateien erprobt.
- Ohne Formelfelder bleiben Movement, Render Scale und Debug Bars.
