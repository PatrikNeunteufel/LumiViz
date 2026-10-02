# LumiViz — Lyrics-Block: Zeilen-Puffer, Wort-Ebene, Auslöser (Kurzkonzept)

> **Version:** 0.2.0
> **Datum:** 2026-10-02 (Session 90)
> **Typ:** Konzept — **Entscheide gefallen (Patrik S90): L1 a · L2 b mit
> fließender Größe, a schaltbar · L3 a · L4 a mit eigenen Feldern für die
> Vorschau-Zeilen · L5 a**; L6 bis vor Stufe LB5 vertagt (freigegeben);
> nichts gebaut
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** `Offene_Punkte.md` §7 „Lyrics: Zeilen-Puffer für den Trail,
> Wort-Ebene, Auslöser für unsynchronen Text" (Ideen Patrik S89) ·
> `Karaoke_Ebene_Konzept.md` (Ebene, Trail-Liste, K4 b, K6) · Benutzerhandbuch
> §11 „Lyrics Trail"
> **Sprache:** Deutsch

## 1. Ziel

Fünf zusammenhängende Wünsche vom Ende der Session 89:

1. Der Lyrics Trail zeigt **genau das Bild, das stand** — nicht eine neu
   gezeichnete Fassung der alten Zeile.
2. Das **laufende Wort** lässt sich hervorheben (größer, eigene Farbe).
3. Skripte kennen das laufende Wort (`word`, `wordcount`) und bekommen ein
   Feld **Word**, das einmal je Wortwechsel läuft.
4. Unsynchroner Text lässt sich **per Skript weiterrücken** (`linetrigger`,
   `wordtrigger`).
5. Das Panel Lyrics Sync wird nachgerüstet: **Wort-Zeiten klopfen** und
   **Duett** — Stimmen setzen (Nachtrag Patrik S90).

Nicht Ziel: Wort-Zeiten des Dienstes (seit S88 zurückgestellt), Zeilen oder
Worte **im Voraus** je Index gestalten (siehe Entscheid L4), Songtext als
Zeichenkette im Skript.

## 2. Was es heute gibt

Alles in diesem Abschnitt ist **aus dem Code gelesen** (`runLyrics`,
`runLyricsTrail`, `zeichneLyricsZeilen`, `runParamScript`, `LyricsModel.hpp`),
nicht im laufenden Programm oder per Test nachgestellt — bis auf den
Pause-Befund (Test, S90). Die mit *Hypothese* markierte Bitgleichheit prüft
der erste Bildtest der Stufe LB1.

| Baustein | Stand heute | Folge für den Block |
|---|---|---|
| Rastern | `zeichneLyricsZeilen` malt **alle Reihen** (laufende Zeile + Vorschau, ggf. Marke) in **eine** Textur in voller Bildgröße; bei getrennter Kontur zwei (Ring, Füllung). Neu gerastert wird nur, wenn sich der Zustandsschlüssel ändert | die laufende Zeile ist nicht einzeln greifbar |
| Trail | `runLyricsTrail` zeichnet die alte Zeile **neu**: aus dem Text und den Feldern des Lyrics-Knotens (`lyricsEffektiv`, mit Line-Code `lyricsEffektivAlt`), ganz in einer Farbe, verschoben um den eigenen Versatz beim Rastern | Wort-Stand geht verloren; die Zeile wird für den Trail jedes Mal neu gerastert |
| Trail, synchron | den Zeitpunkt rechnet der Trail selbst aus den LRC-Zeiten; die alte Zeile ist „die letzte mit Text vor der laufenden" | **Befund S90 (per Zähltest belegt, behoben):** nach einer Pause-Zeile (leere Zeile mit Zeit) zeichnete der Trail dieselbe Zeile ein zweites Mal — beim Beginn der Pause und beim Beginn der nächsten Textzeile. Seit S90 gilt nur die Zeile unmittelbar vor der laufenden; Test „Lyrics Trail S90: Pause-Zeile" |
| Trail, unsynchron | braucht den laufenden Lyrics-Knoten (`lyricsVorZeile`, `lyricsWechselZeit`) | bleibt so |
| Wort-Zeiten | `Zeile::worte` (erweitertes LRC `<mm:ss.xx>`); `worteBei` zählt die gesungenen Worte; gezeichnet werden zwei Stücke: `vorn` (gesungen, Stimmfarbe) und `hinten` (Vorschau-Farbe) | ein dritter Zustand „gerade jetzt" fehlt |
| Skript | Slots Init, Frame, Beat, Song, Line; `line` / `linecount`; Felder, die neu rastern, folgen dem Skript aus Init, Song, Beat, Line | Slot Word nach demselben Muster |
| Unsynchroner Block | rückt nur über *beats per line* | Auslöser fehlen |
| Lyrics Sync | klopft Zeilen-Zeiten; `schreibeLrc` kann Wort-Zeiten schon schreiben | Bedienweg fürs Klopfen je Wort fehlt |
| Karaoke-Ebene | die Zeile rastert in **Fenstergröße**, die Trail-Liste rechnet in **Kettengröße** | ein übergebenes Zeilenbild wird im Trail verkleinert abgetastet |

## 3. Stufe LB1 — Zeilen-Puffer

**Kern:** der Lyrics-Knoten rastert die **laufende Zeile getrennt** von den
übrigen Reihen (Vorschau, Marke) in ein eigenes **Zeilenbild**. Wechselt die
Zeile, wandert das Zeilenbild samt Zeitpunkt in den Platz „alte Zeile" — ein
Tausch der Textur-Namen, keine Kopie, kein neues Rastern. Der Trail legt
dieses Bild auf, solange *Hold* läuft.

Was sich ändert:

- **`zeichneLyricsZeilen`** bekommt eine Auswahl, welche Reihen es malt (nur
  die laufende / alle außer der laufenden). Die Lage rechnet es weiter über
  alle Reihen — die Zeile steht, wo sie heute steht. Der Knoten legt dann
  zwei Texturen nacheinander auf statt einer.
  *Hypothese:* das Ergebnis ist bitgleich mit heute, solange sich die Reihen
  nicht überlappen (bei sehr breiter Kontur können sie es).
- **Übergabe** nur bei einem Wechsel nach vorn im selben Titel und nur, wenn
  die laufende Zeile Text trug. Neuer Titel, Zurückspulen und leere Zeilen
  übergeben nichts. Damit entfällt auch der vermutete Doppel-Trail nach
  Pausen.
- **Trail:** legt das alte Zeilenbild auf; sein Versatz (*X shift*, *Y shift*)
  wirkt beim **Auflegen** als Verschiebung der Textur-Koordinaten, nicht mehr
  beim Rastern. Der Text-Shader bekommt dafür einen Versatz (Vorgabe 0 — alle
  übrigen Wege rechnen wie bisher). Mischart und Stärke des Trails gelten
  unverändert.
- **Ebene:** das Zeilenbild liegt in Fenstergröße vor, die Trail-Liste hat
  Kettengröße — es wird linear verkleinert. Der Trail ist mit Render Scale
  ohnehin grob (K4 b: „so gewollt").
- **Der alte Weg bleibt** als Rückfall: läuft der Lyrics-Knoten nicht (Auge
  aus), gibt es kein Zeilenbild; der Trail zeichnet dann bei synchronem Text
  wie heute neu. Dazu Entscheid **L1** (*Own color*).
- `lyricsEffektivAlt` (S89) wird im Puffer-Weg überflüssig und bleibt nur für
  den Rückfall.
- **Streifen statt Vollbild** (Folge aus Entscheid L2): das Zeilenbild ist
  nur so hoch wie die Zeile (plus Kontur und Platz für das vergrößerte Wort)
  und trägt sein Rechteck im Bild mit. Der Text-Shader bekommt dafür Lage und
  Größe der Textur statt nur eines Versatzes; mit dem Vollbild-Rechteck
  rechnet er wie bisher. Grund: mit fließender Wortgröße (LB2) wird die
  laufende Zeile während des Übergangs **je Frame** neu gerastert — ein
  Vollbild je Frame wäre zu teuer, ein Streifen ist ein Bruchteil davon. Der
  Trail verschiebt dann nur das Rechteck.

### Stufe LB1b — Vorschau-Zeilen gestalten (aus Entscheid L4)

Die Vorschau-Zeilen werden in LB1 ohnehin getrennt gerastert; sie bekommen
eigene Felder, alle **relativ zur laufenden Zeile**:

| Feld | Vorgabe | Wirkung |
|---|---|---|
| *Preview size (%)* `previewScale` | 100 | Schriftgröße der Vorschau-Zeilen in Prozent der laufenden |
| *Preview weight* `previewWeight` | 0 | 0 = wie die laufende Zeile, sonst 400 normal / 700 fett |
| *Preview X shift (%)* `previewXShift` | 0 | Versatz gegen die Stelle, an der die Vorschau heute steht |
| *Preview Y shift (%)* `previewYShift` | 0 | dito, senkrecht |
| *Line spacing (%)* `lineSpacing` | 115 | Zeilenabstand in Prozent der Schrifthöhe (heute fest 1,15) |

Die Farbe gibt es schon (*Color next*). Mit den Vorgaben bleibt jedes Preset,
wie es ist. Die Felder sind Felder des Knotens und damit auch per Skript
stellbar (aus Init, Song, Beat, Line — sie rastern neu).

Prüfung: Bildtests — (a) Zeile mit halb gesungenem Wort-Stand wechselt, das
Trail-Bild zeigt beide Farben an derselben Stelle; (b) verkleinerte lange
Zeile; (c) Line-Skript stellt jede Zeile anders, der Trail zeigt die alte
Lage; (d) Pause-Zeile: der Trail läuft **einmal** (Zähltest, vorher gegen den
heutigen Stand laufen lassen — belegt oder widerlegt die Hypothese);
(e) Knoten allein, ohne Trail: Bild vorher gegen nachher. Kontaktbogen ansehen
(`LUMIVIZ_TRAIL_SNAP`). Vergleichslauf Regressionsliste 84/84.

## 4. Stufe LB2 — das laufende Wort hervorheben

Drei Zustände je Wort statt zwei: **gesungen** · **gerade jetzt** · **noch
nicht**. Das laufende Wort ist das zuletzt begonnene (`worteBei` − 1).

Neue Felder am Lyrics-Knoten (und damit im Karaoke-Panel):

| Feld | Vorgabe | Wirkung |
|---|---|---|
| *Word scale (%)* `wordScale` | 100 | Größe des laufenden Worts; 100 = aus |
| *Own word color* `useWordColor` | aus | laufendes Wort in *Word color* statt in der Stimmfarbe |
| *Word color* `colorWord` | 0xFFE066 | Farbe des laufenden Worts |

| *Word layout* `wordLayout` | Reflow | **Reflow:** die Nachbarn rücken zur Seite · **Overlay:** die Zeile bleibt stehen, das Wort wächst um seine Mitte und liegt über den Nachbarn (ruhiger, für den Trail die bessere Wahl) |
| *Word flow (ms)* `wordFlowMs` | 120 | Zeit, in der die Größe fließt; 0 = harter Sprung |

Mit den Vorgaben ändert sich an keinem vorhandenen Preset etwas (*Word scale*
100 = aus).

**Fließende Größe (Entscheid L2, Idee Patrik):** jedes Wort der laufenden
Zeile trägt eine eigene laufende Größe, die sich ihrem Ziel nähert (100 %
oder *Word scale*) — ein gleitender Mittelwert mit der Zeitkonstante *Word
flow*. Das neue Wort wächst, während das vorige schrumpft; die Summe der
Breiten bleibt dabei nahezu gleich, die Zeile gleitet statt zu zucken.
Dasselbe Fließen gilt in *Overlay*. Während des Übergangs wird die laufende
Zeile je Frame neu gerastert (Streifen, siehe LB1); steht alles am Ziel, ruht
das Rastern. Die Rasterzeit wird in dieser Stufe **gemessen**, nicht
angenommen.

Angelegt wird das Rastern der laufenden Zeile als **Liste von Stücken**
(Text, Farbe, Größe) statt `vorn` / `hinten` — das trägt die drei Zustände
und ließe später auch einen Stimmwechsel mitten in der Zeile zu (L6).

Zeilen **ohne** Wort-Zeiten — fast alle Texte des Dienstes — zeigen kein
laufendes Wort (Entscheid **L3 a**).

Prüfung: Bildtests mit einer Test-LRC mit Wort-Zeiten (drei Zustände an drei
Positionen, Farbe und Rahmenbreite des laufenden Worts gemessen);
Bildfolge über einen Wortwechsel (Kontaktbogen: die Zeile gleitet, kein
Sprung der Zeilenmitte über einem Schwellwert); Feld-Inventar und Ernter;
Vergleichslauf.

## 5. Stufe LB3 — `word`, `wordcount`, Feld **Word**

- `word`: Nummer des laufenden Worts der laufenden Zeile, 1 = erstes, **0 =
  vor dem ersten oder nicht bekannt**. `wordcount`: Zahl der Worte der
  laufenden Zeile. Beide in allen Skriptfeldern lesbar, wie `line` /
  `linecount`.
- Worte einer Zeile ohne Wort-Zeiten: Trennung an Leerzeichen (das Wort trägt
  sein folgendes Leerzeichen, wie im Modell). `wordcount` stimmt damit immer,
  `word` nur, wo es einen Wort-Stand gibt.
- Neuer Slot **Word** (`Slot::Word`, `wordCode`): läuft einmal je
  Wortwechsel, nach Line und vor Frame. Felder, die neu rastern, folgen dem
  Skript dann aus Init, Song, Beat, Line **und Word**.
- Bauweg: die Merkregel „Neuer Skript-Slot" aus S89 (Slot, `kSlotCount`,
  `kSlotNames`, `hatSkriptSchalter`, `leereSkriptSlots`, `slotScopeHtml`,
  Variablentabelle, Ernter).

Ob ein Skript auch **andere** Zeilen und Worte als die laufenden stellen kann,
ist Entscheid **L4**.

Prüfung: Zähltest (`slotRunCount`: Word läuft genau einmal je Wortwechsel,
nicht je Frame; Gegenprobe ohne Wort-Zeiten = 0 Läufe), Werte von `word` /
`wordcount` an bekannten Positionen, Hilfetext-Test, Script active aus.

## 6. Stufe LB4 — `linetrigger` / `wordtrigger`

Ausgabe-Variablen des Lyrics-Knotens nach dem Muster von `switch` /
`nextgroup` am Host Switcher: vor jedem Lauf auf 0 gesetzt, gelesen nach dem
Lauf, gelten nur für diesen Frame.

- `linetrigger > 0`: der unsynchrone Block rückt eine Zeile weiter;
  `linetrigger < 0`: eine zurück.
- `wordtrigger > 0`: das laufende Wort rückt eins weiter; `< 0`: eins zurück.
  Was am Zeilenende geschieht, ist Entscheid **L5**.
- Wirken **nur bei unsynchronem Text**; bei synchronem bestimmt weiter die
  Abspielposition (im Hilfetext so gesagt).
- *Beats per line* bleibt und wirkt zusätzlich — wer per Skript rückt, stellt
  es auf 0.
- **Wort-Stand im unsynchronen Block:** solange in einem Titel kein
  `wordtrigger` gefallen ist, steht die Zeile wie heute ganz in der
  Stimmfarbe (`word` = 0). Ab dem ersten `wordtrigger` gilt der Wort-Stand:
  gesungen / gerade jetzt / noch nicht, mit den Feldern aus LB2.
- Am Textende bleibt der Block stehen, wie heute.
- Ein Wechsel per Auslöser zeigt sich im nächsten Frame (die Slots Line und
  Word laufen dann) — dieselbe Verzögerung wie heute bei *beats per line*.
- Der Übergang an den Trail (LB1) gilt auch hier: der Block übergibt das
  Zeilenbild beim Weiterrücken.

Prüfung: Bildtest/Zähltest (`linetrigger = b` rückt je Beat genau eine Zeile;
einmal gesetzt rückt nicht ewig — Gegenprobe wie beim Host Switcher), synchron
= keine Wirkung, Variablentabelle und Hilfetext.

## 7. Stufe LB5 — Lyrics Sync nachrüsten: Wort-Zeiten und Duett

Auftrag Patrik (S90): der Sync-Editor soll mit dem Block Schritt halten —
**Zeitmarken je Wort** und **Duett** (mehrere Interpreten).

**Wort-Zeiten.** Klopfen je Wort, Nachstellen je Wort, Speichern als
erweitertes LRC (`schreibeLrc` schreibt `<mm:ss.xx>` schon, `leseLrc` liest
es). Zu klären: der Bedienweg (Wort-Modus in der Liste oder eigener Bereich
je Zeile), die Regel für halb geklopfte Zeilen (das Modell kennt keine Worte
ohne Zeit) und das Zusammenspiel mit Undo.

**Duett.** Das Modell trägt die Stimme **je Zeile** (`Zeile::stimme`: Eins,
Zwei, Beide), Leser und Schreiber kennen die Marken (`v1:`, `v2:`, `v1+v2:`,
`M:`, `F:`, `D:`), der Knoten färbt danach (*Color*, *Color voice 2*, *Color
both*). Das Panel Lyrics Sync zeigt und setzt die Stimme heute **nicht** (im
Quelltext des Panels kommt sie nicht vor; ob sie beim Speichern erhalten
bleibt, ist nicht geprüft — ein Test am Anfang von LB5). Nachzurüsten: eine
Spalte *Voice* mit Auswahl je Zeile, Setzen für mehrere markierte Zeilen,
Tasten dafür, Vorschau sofort im Bild. Zu klären: ob die Stimme auch
**innerhalb einer Zeile** wechseln können soll (Wort-Ebene) — das Modell und
das LRC-Format von LumiViz können das heute nicht; und ob es bei zwei Stimmen
plus „beide" bleibt.

**Langsamer abspielen (Wunsch Patrik S90).** Ein Schieber im Panel senkt die
Wiedergabegeschwindigkeit, damit sich Zeilen, Worte und Stimmwechsel besser
treffen lassen. Die Zeiten bleiben richtig, weil geklopft wird, was der
Player als Position meldet. Stand: `bass_fx` (Tempo ohne Tonhöhenänderung)
liegt unter `externals/bass/bass_fx24`, ist aber im Code nirgends
angebunden (Suche nach `bass_fx` / `BASS_ATTRIB_TEMPO` in Quellen und
`Solution.json`: kein Treffer). Zu klären in L6: Tempo über `bass_fx`
(Tonhöhe bleibt, neue Abhängigkeit im Player) oder über die Abtastrate
(ohne neue Bibliothek, die Stimme wird tiefer).

**Worte aus der Wellenform erkennen (Idee Patrik S90, nicht eingeplant).**
Einsätze im Signal lassen sich finden, aber im fertigen Mix sind das
Schlagzeug und Bass, nicht Silben; verlässliches Ausrichten von Text auf
Gesang braucht eine Stimmentrennung und ein Sprachmodell (eigene, große
Abhängigkeit). Kleine Vorstufe, die in LB5 passt: Einsätze in der
Wellenform des Panels markieren und einen Tap auf den nächsten Einsatz
einrasten lassen. Steht als Idee in L6.

Das alles ist Entscheid **L6** — Vorschlag: erst **vor** LB5 vorlegen, weil
die Wort-Trennung aus LB3 und die Erfahrung aus LB4 den Bedienweg prägen.
LB1–LB4 hängen nicht davon ab. Einzige Vorab-Frage, die LB2 berührt: sollte
die Stimme je Wort wechseln können, müsste das Rastern der laufenden Zeile
mehr als drei Farbstücke tragen — das lässt sich in LB2 gleich so anlegen
(Stücke-Liste statt `vorn` / `hinten`), ohne den Entscheid vorwegzunehmen.

## 8. Stufen im Überblick

| Stufe | Inhalt | sichtbar bei | hängt ab von |
|---|---|---|---|
| **LB1** | Zeilen-Puffer (Streifen), Trail legt das Zeilenbild auf | jedem Text mit Trail | — |
| **LB1b** | Vorschau-Zeilen: Größe, Gewicht, Lage, Zeilenabstand | jedem Text mit Vorschau | LB1 |
| **LB2** | laufendes Wort: *Word scale*, *Word color*, *Word layout*, *Word flow* | Texten mit Wort-Zeiten (L3) | LB1 (getrennte laufende Zeile) |
| **LB3** | `word`, `wordcount`, Feld Word | Texten mit Wort-Stand | LB2 |
| **LB4** | `linetrigger`, `wordtrigger` | unsynchronem Text | LB3 |
| **LB5** | Lyrics Sync: Wort-Zeiten klopfen, Stimmen setzen (Duett) | synchronem Text | L6 |

Jede Stufe ist für sich abnehmbar; nach jeder Stufe Tests, Vergleichslauf und
eine Commit-Message.

## 9. Risiken

- **Rasterkosten:** drei statt einer Textur in voller Bildgröße je Wechsel
  (laufende Zeile, Vorschau, bei getrennter Kontur je zwei). Mit Wort-Zeiten
  wird je Wort neu gerastert — aber nur noch die laufende Zeile. Nicht
  gemessen; falls es hakt: nur den Streifen der Zeile rastern statt des
  ganzen Bilds.
- **Bitgleichheit des Knotens** nach dem Trennen der Reihen ist eine
  Hypothese (Abschnitt 3). Der Vergleichslauf deckt Lyrics nicht ab (die
  Regressionsliste läuft ohne Songtext) — hier tragen die Bildtests.
- **Sichtbarkeit von LB2/LB3:** ohne Wort-Zeiten kein laufendes Wort. Bis LB5
  gibt es sie nur in Dateien, die sie mitbringen, und im unsynchronen Block
  über `wordtrigger`.
- **Speicher:** das alte Zeilenbild ist eine zweite Textur in Bildgröße je
  Lyrics-Knoten, solange ein Trail folgt.

## 10. Entscheide

**Stand (Patrik, S90):**

- **L1 a** — mit *Own color* zeichnet der Trail neu.
- **L2 b**, mit fließender Größe („eine Art moving average"), **a schaltbar**
  (für den Trail besser) → Felder *Word layout* und *Word flow*, Abschnitt 4.
- **L3 a** — ohne Wort-Zeiten kein laufendes Wort.
- **L4 a** — nur das Laufende per Skript; dazu für die Vorschau-Zeilen neben
  der Farbe eigene Felder für Größe, Gewicht und Lage (relativ zur laufenden
  Zeile) und der Zeilenabstand → Stufe LB1b.
- **L5 a** — `wordtrigger` rückt am Zeilenende in die nächste Zeile.
- **L6** vertagt bis vor LB5; hinzugekommen: Duett, langsamer abspielen,
  Worte aus der Wellenform (Abschnitt 7). **Offen aus der Rückfrage:** ob die
  Stimme auch innerhalb einer Zeile wechseln können soll — LB2 legt das
  Rastern so an, dass beides geht.

Die Vorlagen, wie sie zur Entscheidung standen:

**L1 — *Own color* am Trail mit dem Zeilen-Puffer**
*Ort:* Lyrics Trail, Feld *Own color*; `runLyricsTrail`.
*Heute:* der Trail zeichnet die alte Zeile neu und kann sie dabei ganz in
seiner eigenen Farbe malen; die Kontur behält ihre Farbe.
*Problem:* ein fertiges Bild lässt sich nicht umfärben, ohne die Kontur
mitzufärben — Füllung und Kontur stecken in derselben Textur.

- **a (Empfehlung):** mit *Own color* zeichnet der Trail wie heute **neu**
  (der Weg bleibt als Rückfall ohnehin bestehen); ohne *Own color* legt er
  das Zeilenbild auf. Im Tooltip steht: „With own color the line is redrawn
  in one color; the word state is not kept."
- **b:** der Knoten rastert die laufende Zeile zusätzlich als Paar (Füllung,
  Kontur-Ring), sobald ein Trail mit *Own color* folgt; der Trail färbt nur
  die Füllung um.

*Warum die Empfehlung:* den Ausschlag gibt, dass *Own color* die Zeile
ohnehin einfarbig macht — der Wort-Stand, den der Puffer retten soll, wäre
dort gar nicht zu sehen; a liefert also dasselbe Bild wie b ohne zweiten
Rasterweg. Für b spricht, dass eine per Word-Skript verstellte Größe oder
Lage auch mit eigener Farbe exakt erhalten bliebe und dass es nur noch einen
Trail-Weg gäbe; das wiegt weniger, weil der alte Weg für „Lyrics-Knoten aus"
bleiben muss und die Skriptwerte der Zeile über `lyricsEffektivAlt` schon
mitkommen. Kippen würde es, wenn du *Own color* zusammen mit einem
vergrößerten laufenden Wort im Trail sehen willst.

**L2 — Wie sitzt das vergrößerte Wort in der Zeile?**
*Ort:* Lyrics-Knoten, neues Feld *Word scale (%)*; `zeichneLyricsZeilen`.
*Heute:* alle Worte einer Zeile haben dieselbe Größe; die Zeile wird einmal
gesetzt und ausgerichtet.
*Problem:* ein größeres Wort ist breiter. Entweder rücken die Nachbarn, oder
es überdeckt sie.

- **a (Empfehlung):** die Zeile wird in Grundgröße gesetzt und bleibt stehen;
  das laufende Wort wächst **um seine Mitte** auf der Grundlinie und liegt
  über den Nachbarn.
- **b:** die Zeile wird je Wortwechsel neu gesetzt; die Nachbarn rücken zur
  Seite, eine zentrierte Zeile verschiebt sich dabei.
- **c:** die Zeile wird so gesetzt, als wäre **jedes** Wort vergrößert (fester
  Platz je Wort); nichts rückt, nichts überdeckt, die Abstände sind weiter.

*Warum die Empfehlung:* den Ausschlag gibt die Ruhe — bei a bewegt sich nur
das eine Wort, die Zeile bleibt lesbar und der Trail übernimmt ein Bild, das
an derselben Stelle steht wie alle Stände davor. Für b spricht ein sauberes
Schriftbild ohne Überdeckung; das wiegt weniger, weil die ganze Zeile dann
bei jedem Wort zuckt. Für c spricht Ruhe **und** keine Überdeckung, es kostet
aber Zeilenbreite (lange Zeilen werden früher verkleinert) und sieht bei
100 % anders aus als bei 130 %. Kippen würde es, wenn die Überdeckung ab etwa
130 % im Bild stört — dann c.

**L3 — Synchrone Zeilen ohne Wort-Zeiten**
*Ort:* Lyrics-Knoten; `runLyrics`, Zweig `zl.worte.empty()`.
*Heute:* die ganze Zeile steht in der Stimmfarbe; Wort für Wort nur mit
echten Wort-Zeiten (Entscheid 6a aus S86: keine vorgetäuschten Zeiten).
*Problem:* die Texte des Dienstes tragen nur Zeilen-Zeiten. Das laufende
Wort wäre damit bei fast allen Titeln nicht zu sehen, bis Wort-Zeiten
geklopft sind (LB5).

- **a (Empfehlung):** bleibt wie heute — ohne Wort-Zeiten kein laufendes
  Wort, `word` = 0, das Feld Word läuft nicht.
- **b:** neuer Schalter *Estimate word times* (Vorgabe aus): die Worte werden
  gleichmäßig nach Buchstabenzahl über die Dauer der Zeile verteilt; `word`
  läuft, das Feld Word auch.

*Warum die Empfehlung:* den Ausschlag gibt der Entscheid 6a — geschätzte
Zeiten liegen bei gehaltenen Tönen und Pausen in der Zeile sichtbar daneben,
und ein Hervorheben am falschen Wort ist schlechter als keines. Für b spricht,
dass die Wort-Ebene sofort bei jedem Titel etwas zeigt und ein Schalter mit
Vorgabe aus niemanden täuscht; das wiegt weniger, solange LB5 die echten
Zeiten liefert. Kippen würde es, wenn du die Wort-Effekte vor allem als
Effekt willst (Zucken im Takt der Worte) und nicht als Mitsing-Hilfe — dann b
als Zugabe nach LB3.

**L4 — Andere Zeilen und Worte als die laufenden ansprechen**
*Ort:* Skriptfelder des Lyrics-Knotens.
*Heute:* `line` nennt die laufende Zeile; ein Skript stellt die Felder für
die Zeile, die gerade wechselt (`if(equal(line,3), …)`).
*Problem:* deine Frage aus S89 — „kann man die Zeilen mit Index ansprechen,
und Worte auch". Offen ist, ob damit nur das Laufende gemeint ist oder auch
ein Gestalten im Voraus (etwa: Vorschau-Zeile 2 kleiner, jedes dritte Wort
rot, bevor es gesungen wird).

- **a (Empfehlung):** nur das Laufende. Zeilen über `line` im Feld Line,
  Worte über `word` im Feld Word; das Skript entscheidet im Augenblick des
  Wechsels.
- **b:** eine Tabelle je Index: Skriptfunktionen, die Farbe und Größe für
  Zeile *i* / Wort *j* im Voraus ablegen; der Knoten liest sie beim Rastern.

*Warum die Empfehlung:* den Ausschlag gibt, dass a mit LB3 schon da ist und
alles abdeckt, was am laufenden Wort oder an der laufenden Zeile geschieht.
Für b spricht, dass auch Vorschau-Zeilen und noch nicht gesungene Worte
gestaltbar wären; das wiegt weniger, weil es einen neuen Datenweg vom Skript
ins Rastern braucht (heute gehen nur Felder des Knotens) und ein Skript den
Text selbst nicht lesen kann — „jedes Wort *love* rot" ginge auch mit b
nicht. Kippen würde es, wenn du Vorschau-Zeilen einzeln gestalten willst;
dann b als eigener Block nach LB4.

**L5 — `wordtrigger` am Zeilenende**
*Ort:* Lyrics-Knoten, neue Skriptvariable `wordtrigger`.
*Heute:* gibt es nicht.
*Problem:* steht das laufende Wort auf dem letzten der Zeile und fällt ein
weiterer `wordtrigger` — rückt dann die Zeile, oder bleibt das Wort stehen?

- **a (Empfehlung):** die nächste Zeile beginnt, ihr erstes Wort ist das
  laufende. Mit `wordtrigger` allein läuft der ganze Text durch.
- **b:** das Wort bleibt auf dem letzten stehen, bis ein `linetrigger` die
  Zeile rückt.

*Warum die Empfehlung:* den Ausschlag gibt, dass ein einziges Skript wie
`wordtrigger = b;` damit den ganzen Text Wort für Wort durchläuft, ohne
selbst Worte zu zählen. Für b spricht die getrennte Steuerung (Worte auf die
Snare, Zeilen auf den Bass); das wiegt weniger, weil sich b mit a nachbauen
lässt — `if(below(word, wordcount), wordtrigger = 1, 0)` hält am Zeilenende
an —, umgekehrt aber nicht ohne Mitzählen. Kippen würde es, wenn das
selbsttätige Rücken beim Ausprobieren überrascht.

**L6 — Lyrics Sync: Bedienweg für Wort-Zeiten und Duett** — *vertagt bis vor
LB5* (Vorschlag, braucht deine Freigabe): die Vorlage kommt, wenn LB1–LB4
stehen, weil Wort-Trennung und Wort-Stand dann feststehen. Sie umfasst:
Klopfen je Wort, halb geklopfte Zeilen, Spalte *Voice*, Stimme je Zeile oder
auch je Wort, Zahl der Stimmen.
