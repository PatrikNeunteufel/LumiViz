# MyViz — Changelog Session 59 (2026-07-31)

> **Thema:** Die drei letzten offenen Vergleichs-Presets sind erledigt, alle
> Einzelsonden stehen **zum ersten Mal komplett auf Grün (89/89)** — und ein
> neues Flächen-Urteil in der Prüfung deckt fünf Fehler auf, die jahrelang
> unter der Messschwelle lagen. Dazu spielt der AVI-Effekt jetzt auch Videos
> in Uralt-Codecs ab.

## Drei Presets, sieben Ursachen

**splendora** sah aus wie durch einen Farbfilter: Rot und Blau waren
vertauscht. Der Grund ist historisch: das Preset wurde mit der *originalen*
Channel-Shift-Erweiterung gespeichert, deren interner Zahlencode für „RGB —
nichts tun" ein anderer ist als der des späteren Winamp-Einbaus. Unser Import
kannte nur die neue Nummerierung und machte aus „nichts tun" einen
Kanaltausch. Dazu kam ein Zeichen-Detail: **eine AVS-Linie malt ihren
Endpunkt nie** — und wo ein Effekt genau die letzte Bildzeile abtastet, wird
aus dieser einen Zeile ein ganzer falscher Farbkeil. Unsere dicken Linien
laufen jetzt exakt den Zeichenalgorithmus des Originals nach. Ergebnis: von
deutlich sichtbarer Abweichung auf praktisch deckungsgleich.

**el-vis_hypno07** (2500 Funken-Sprites) war „langsam und nicht
audioreaktiv" — das Preset misst über die eingebaute Uhr seine eigene
Bildrate und steuert damit sein Tempo. Drei Uhren gingen falsch: der
Referenz-Renderer maß die echte Uhrzeit statt der Bildfrequenz (er bekommt
jetzt eine virtuelle Frame-Uhr), unsere Uhr startete einen Tick zu spät und
tickte feiner als das Original (jetzt: Start bei Null, ganze Millisekunden) —
und eine Komfort-Variable des Hosts überschrieb ausgerechnet die Variable
`time`, die dieses Preset selbst als Konstante benutzt. Der Host lässt die
Finger jetzt von Variablen, die ein Preset selbst setzt.

**Der Sprite-Zeichner (Texer II) arbeitet jetzt bit-genau wie das Original.**
Gemessen wurde das mit einem Trick: ein Bild aus einem einzigen weißen Pixel
durch die Original-Erweiterung gerendert zeigt deren Vergrößerungs-Verfahren
direkt — Position, Schrittweite, Rundung, alles. In vierzehn Messfällen
stimmt unser Nachbau Pixel für Pixel. Auch das eingebaute Standard-Sprite ist
jetzt die vermessene Original-Textur statt einer Rekonstruktion.

## AVI-Effekt: Uralt-Codecs laufen wieder

Ein Preset blieb schwarz, weil sein Video in **Intel Indeo 3.2** kodiert ist —
einem Codec von 1995, für den es in modernen 64-Bit-Programmen schlicht keinen
Decoder mehr gibt (das alte 32-Bit-AVS spielt ihn noch). Der AVI-Effekt hat
jetzt einen zweiten Weg: scheitert der klassische Windows-Decoder, übernimmt
die FFmpeg-Dekodierung von Qt Multimedia — die Datei wird einmal komplett in
Einzelbilder zerlegt, aus denen der Effekt dann bildgenau und reproduzierbar
liest. Das betroffene Preset zeigt wieder Bild.

## Convolution: das Rätsel der Ränder ist gelöst

Seit Wochen meldeten drei Sonden, dass am Bildrand der Faltung „etwas anderes"
steht als beim Original. Der Originalquelltext (er hat überlebt) zeigt zwei
Eigenheiten: die Faltung **liest die letzte Bildzeile und -spalte nie** — und
wenn der Faltungskern eine bestimmte Form hat, schreibt sie ihr Ergebnis **in
ihr eigenes Eingabebild**. Am Rand liest sie dann bereits verarbeitete Pixel:
die Kante wird quasi doppelt gefaltet. Beides ist nachgebaut; **alle acht
Convolution-Sonden messen jetzt exakt null Abweichung.** Nebenbei stellte
sich heraus, dass der Schalter „Wrap" nie ein Bildumlauf war, sondern eine
Rechenart (Überlauf statt Sättigung) — unser Bildumlauf dafür ist entfernt.

Als Folge fiel auch **Alternate Reality** — im Vorlauf bereits „als Rauschen
abgenommen" — auf echtes Grün: die Rand-Regel war sein letzter Rest.

## Die Prüfung urteilt jetzt auch über die Fläche

Die bisherige Messzahl (mittlere Abweichung) ist blind für dünne Inhalte: zwei
fast schwarze Bilder messen sich ähnlich, egal was auf ihnen steht. Die
Matrix-Prüfung vergleicht jetzt zusätzlich, **wie viel** leuchtet und **ob es
an denselben Stellen** leuchtet. Der erste Lauf fand sofort fünf alte Fehler,
darunter einen drastischen: bei den rotierenden Sternen zeichnet das Original
winzige Punkt-Sterne — wir zeichnen große. Die Messzahl war immer grün.

Fünf Prüfzeilen stehen damit neu auf Rot. Das ist kein Rückschritt, sondern
Ehrlichkeit: die Fehler waren vorher auch da, nur unsichtbar.

## Außerdem

- **Wasser-Wellen (Water Bump)** rechnen jetzt in den Ganzzahl-Einheiten des
  Originals — auf stehendem Material praktisch deckungsgleich (48 von 76 800
  Pixeln). Die Rückkopplungs-Prüfzeile bleibt offen; wie es dort weitergeht,
  ist als Entscheidung notiert.
- Eine neue Wächter-Sonde bewacht die **letzte Bildspalte** der Scopes (der
  Fix aus Session 58 hatte keinen Wächter).
- Die eingefrorenen Übersetzungs-Zwillinge sind durchgesehen und auf den
  aktuellen Stand gebracht: **67 von 67 stimmen**.
- Die Changelog-Reihe ist wieder **lückenlos** — die fehlenden Einträge zu
  Session 45 und 56 sind nachgeschrieben.

## Stand

- **Einzelsonden: 89 von 89 — erstmals alle grün**
- **Bildvergleich: 35 von 43** — nach neuen, strengeren Flächen-Kriterien;
  die fünf neuen Roten sind aufgedeckte Altfehler, kein Rückschritt
- Übersetzungs-Zwillinge: **67 von 67**
- **Tests: 485 von 485 grün**, alle Bauarten fehlerfrei
