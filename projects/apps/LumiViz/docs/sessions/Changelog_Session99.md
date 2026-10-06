# Changelog Session 99 (2026-10-06)

> **Die Schleife einer Komposition kann sich überlagern.** Mit *Loop overlap*
> beginnt die Komposition schon vor ihrem Ende wieder von vorn — Ende und
> Anfang laufen zugleich, im Bild und im Ton. Dazu lässt sich die ganze
> Komposition ein- und ausblenden. Der zweite Sichttest der Session 98 ist
> ausgewertet (33 von 33), und eine Lücke im Panel Player ist geschlossen.
> **Im Programm gesehen** ist von dieser Session nichts: die Überlappung, die
> Blende und die geschlossene Lücke sind gebaut und per Test belegt.

## Neu

- **Loop overlap** (Composer, Feld unten im Panel): Mit Loop an beginnt die
  Komposition um diese Zeit vor ihrem Ende wieder von vorn. Ihre letzten und
  ihre ersten Sekunden laufen zugleich, Bild und Ton; die Runde wird um die
  Überlappung kürzer. Geblendet wird dabei nichts von selbst — jeder Clip
  behält seine Pegelkurve.
- **Die Zeitachse zeigt beide Fenster:** bei Loop an sind die ersten und die
  letzten Sekunden getönt, mit einer gestrichelten Grenze. Läuft die
  Komposition durch das Fenster, läuft die rote Marke vorn und eine zweite,
  blassere hinten mit.
- **Fade in / Fade out der ganzen Komposition:** Bild und Ton blenden am
  Anfang ein und vor dem Ende aus, zusätzlich zu den Pegelkurven der Clips.
  Das gilt immer, auch ohne Loop. Im Lineal steht dafür je ein schmaler Keil.
  Zusammen mit *Loop overlap* ergibt das eine fertige Überblendung.
- **Songs overlap themselves** (Standard: an): ein Song, der in beiden
  Fenstern liegt, spielt im Fenster zweimal — sein Ende klingt aus, während
  sein Anfang schon läuft. Aus: er spielt zu Ende und springt dann zurück.
- **Visuals overlap themselves** (Standard: aus): ein Bild-Clip in beiden
  Fenstern läuft im Fenster zweimal, ein frisch gestartetes Exemplar über dem
  laufenden. Aus: er läuft einmal durch, mit dem höheren seiner zwei Pegel.
- Alle fünf Werte gehören zur Komposition und stehen in ihrer Datei
  (`loop { overlap, songsTwice, visualsTwice }`, `fade { in, out }` — nur,
  wenn sie von der Vorgabe abweichen).

## Geändert

- **Jede Schleife beginnt weicher, auch ohne Überlappung:** die ersten Clips
  der nächsten Runde sind beim Umschlag schon geladen, und ein Song über die
  ganze Länge setzt nahtlos neu an (bisher wurde er zurückgesetzt, und die
  ersten Clips begannen kalt).
- **Nach „Composer off" ohne geladenen Titel im Player** zeigen das Panel
  Player und Tag View „No track loaded", der Songtext der Komposition
  verschwindet. Bisher blieb der letzte Song der Komposition stehen — im
  Panel Player als „Composer - <Song>", obwohl der Composer aus war.
- **Das Panel Composer** hat unter dem Fuß zwei weitere Zeilen (Schleife und
  Blende).

## Entschieden

- **Überlappung und Blende sind zwei getrennte Einstellungen:** *Loop
  overlap* wirkt nur in der Schleife und blendet nichts; *Fade in* / *Fade
  out* gelten immer.
- **Die Werte gehören zur Komposition**, nicht in die Einstellungen des
  Programms.
- **Die Zeitachse zeigt beide Fenster** und eine zweite Marke.
- **Ein Clip in beiden Fenstern: einstellbar**, mit zwei Haken je
  Komposition.
- **Composer off ohne Titel im Player:** die Sitzung meldet einen leeren
  Songwechsel, das Panel Player geht in seinen Ruhezustand.
- Aus dem Sichttest S98b, was gesetzt war und gebilligt ist: beide Haken
  „Weiterspielen" Vorgabe an; die Einfügemarke rückt hinter den eingefügten
  Clip; ohne Marke landet die Kopie hinter dem angeklickten Clip; eine Kopie
  wird nicht gekürzt; kein Hinweis bei falscher Art; der Tooltip nur im
  Lineal.

## Technik

- **Uhr:** `setUeberlappungMs`, `umlaufMs`, `zweiteStelleMs` (die Stelle im
  hinteren Fenster, die zugleich läuft), `rueckkehrMs`. In der ersten Runde
  und nach einem Sprung ins vordere Fenster gibt es keine zweite Stelle; ein
  Sprung ins hintere zeigt beide. Halt, Bearbeiten, Composer aus / an und der
  Stand für den Neustart merken im Fenster die hintere Stelle.
- **Bild:** `Wiedergabe::rendere` nimmt eine `Schleifenlage` und mischt die
  Clips beider Stellen; im selben Kanal liegt der Clip des Endes unten, der
  des Anfangs darüber. Zweite Exemplare eines Clips in `m_frische`; hinter
  dem Fenster läuft das Exemplar weiter, dessen letzte Stelle im Clip am
  nächsten liegt.
- **Ton:** `Tonfuehrung::takt` führt Stimmen (jeder Ton-Clip an seiner Stelle
  der Zeitachse) und ordnet ihnen die Quellen über die Stelle im Song zu. Ein
  Sprung wird je Quelle an der Stelle im Song erkannt — **der Umschlag der
  Schleife ist kein Sprung mehr**.
- **Modell:** `Komposition::ueberlappung()` (höchstens die halbe Länge),
  `umlaufMs()`, `endStelleZu()`, `blendeBei()` (linear).
- **Naht N5, Anzeige, fünfte Stelle** (`3624e1f`, Eigenes davor `e3043ae`):
  der Empfänger von `TrackChangedEvent` im Panel Player geht bei einem
  Songwechsel ohne Datei in den Ruhezustand. Sonst keine neue Naht — die
  ganze Überlappung ist eigener Code des Composers.
- **Ein Fehler unterwegs:** der Stand nach Schritt 3 baute mit clang nicht
  (eine eingeschachtelte Struktur als Vorgabewert eines Parameters ihrer
  Klasse; MSVC nahm es an). Behoben in `a2c49f7`; seither wird jeder Schritt
  mit beiden Compilern gebaut, App und Tests.
- **939 Cases** im Zweig `composer` (Start der Session: 934).
- Doku: `Composer_Konzept.md` 0.34.0 → 0.43.1, `Benutzerhandbuch.md` 1.94.0 →
  1.98.0, `Offene_Punkte.md` 1.91.0 → 1.95.0.

## Offen

- **Im Programm nicht gesehen und nicht gehört:** die ganze Überlappung, die
  Blende der Komposition, *Visuals overlap themselves*, die geschlossene
  Lücke im Panel Player — und was davon ohne Entscheid gesetzt ist (das
  Verhalten bei Sprung, Halt und Loop aus im Fenster; die Reihenfolge im
  selben Kanal; die lineare Blende; die Karaoke-Ebene blendet nicht mit).
- **Nicht gemessen:** was das zweite Laden eines Presets kurz vor dem
  Umschlag an Bildzeit kostet, und der Grafikspeicher im Fenster.
- **„Flipped" und einfaches Einblenden:** ein Clip soll unsichtbar anlaufen,
  später einblenden und oben bleiben. Das Modell kann es über ein Stück mit
  heruntergezogenem Anfang; der Weg ist versteckt. Offen, ob ein
  mitgeliefertes Kurven-Preset genügt oder die Einblendung einen eigenen
  Einsatz bekommt.
- **Wünsche zur Zeitachse:** Kanalfarben einstellbar (Bild und Ton gleich
  behandelt), Kanäle von Hand verschieben, nach Typ sortieren, Zoom und
  Schwenken.
- Aus dem ersten Hörtest der Session 98 nicht geprüft: Audio-Kanal
  ausschalten, Bearbeiten im Lauf, Bild und Ton über Minuten, das Panel
  Player (Balken, Klick in der Überlagerung), Text-Vorlage `{title}` in einer
  Komposition.
- Weiteres in `Offene_Punkte.md` §7.
