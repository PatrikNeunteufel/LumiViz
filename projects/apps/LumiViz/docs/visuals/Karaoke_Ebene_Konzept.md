# LumiViz — Karaoke-Ebene (Kurzkonzept)

> **Version:** 0.1.0
> **Datum:** 2026-10-02 (Session 88)
> **Typ:** Konzept — Entwurf, **Entscheide K1–K5 offen**; nichts gebaut
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** `Offene_Punkte.md` §7 „Ausblick Timeline-Panel / Composit"
> (Ideen Patrik S88, Entscheid 2a) · §7 „Songtexte" (Lyrics, Lyrics Trail,
> Lyrics Sync) · Benutzerhandbuch §11 „Lyrics Trail" (das Muster aus
> `asset/effectchain/20261001.mix.lvfx2`)
> **Sprache:** Deutsch

## 1. Ziel

Songtext über **jedem** laufenden Preset — auch über importierten AVS- und
MilkDrop-Presets —, ohne dass jede Kette einen Lyrics-Knoten braucht. Ein
Schalter, ein Panel, ein Aussehen für alles.

Festgelegt (Entscheid Patrik 2a, S88):

- Die Karaoke-Ebene ist ein **weiterer Weg**, kein Ersatz: der **Lyrics-Knoten
  bleibt**, wie er ist.
- **Die Effektkette gewinnt:** enthält das laufende Preset selbst Lyrics,
  zeichnet es sie (es ist womöglich auf das Preset abgestimmt) und die Ebene
  tritt zurück. Gezeigt wird in beiden Fällen **derselbe Songtext**, nie
  doppelt.
- Die Ebene wird **vor der Timeline** gebaut; die Timeline schaltet später
  denselben Schalter.

Nicht Ziel dieser Stufe: Karaoke je Visual-Kanal verschieden (das wäre Sache
der Timeline), Wort-Zeiten des Dienstes, eigene Songtext-Quellen — Quelle,
Versatz je Titel, Lyrics Sync und Alternativen bleiben, was sie sind.

## 2. Was es heute gibt und worauf die Ebene aufsetzt

| Baustein | Stand | Rolle in der Ebene |
|---|---|---|
| Songtext des laufenden Titels (`MultiEffectVisualizer::setSongtext`, Abspielposition, Versatz je Titel) | prozessweit, unabhängig von der Kette | unverändert die Quelle |
| Rastern und Auflegen der Zeilen (`zeichneLyricsZeilen`, Glyphen-Textur mit Alpha, `composeGlyphLayer`) | S86/S87 | wird wiederverwendet |
| Lyrics Trail samt dem Muster „Effect List behält ihr Bild → Blitter Feedback → Fadeout → Trail" | S87/S88 | wird der zweite Reiter |
| Post-Bloom: wirkt **beim Present**, die Root-Surface bleibt unberührt (S48) | gebaut | **das Vorbild für die Einhängestelle** |
| Effekt-Palette mit Rollen (Render · Transform · Color · Control) | S79 | Grundlage des Filters |
| Knoten-Voreinstellungen (`nodepresets/lyrics/`, 13 Stück) | S86/S87 | Aussehen der Ebene wählbar |

## 3. Die Einhängestelle — der Kern des Konzepts

Die Ebene darf **nicht in die Root-Surface** der Kette zeichnen. Die Surface
ist das Feedback des nächsten Frames: jede Kette ohne Clear (und das sind die
meisten AVS-Presets) würde den Text mit durch ihre Bewegung ziehen und
verschmieren. Genau deshalb wirkt der Post-Bloom erst beim Present.

Die Ebene bekommt darum:

1. **eine eigene Surface** in Kettenauflösung für den Trail-Teil — sie behält
   ihr Bild zwischen den Frames (wie die Effect List im Mix), die erlaubten
   Module arbeiten nur auf ihr;
2. **zwei Draws beim Present**, nach dem Bild der Kette (bzw. nach dem
   Post-Bloom-Composite): erst die Trail-Surface additiv, dann die
   Glyphen-Textur der laufenden Zeile mit Alpha darüber (Kontur bleibt damit
   auch über hellem Bild lesbar).

Folgen:

- Das Bild der Kette und ihr Feedback bleiben **bitgleich**, ob die Ebene an
  oder aus ist — prüfbar mit dem vorhandenen Vergleichslauf.
- Ein Screenshot aus der App (liest das Fenster) enthält die Ebene; die
  Mess- und Vergleichswerkzeuge (lesen die Root-Surface) sehen sie nicht.
  Beides ist so gewollt.
- Render Scale: die Ebene wird in Kettenauflösung gerechnet und mit dem Bild
  hochskaliert. Ob der Text stattdessen in Fensterauflösung gerastert werden
  soll (schärfer bei kleinem Render Scale), ist Entscheid **K4**.

**Reichweite:** die Ebene sitzt im `MultiEffectVisualizer`, also über allem,
was als Kette läuft (`.lvfx`, `.lvfx2`, `.avs`, `.milk` über den
Milkdrop-Knoten). Die eigenständigen Visualizer (Equalizer, Oscilloscope,
Waveform, Pulsing, SuperScope als Standalone) erreicht sie nicht — die sollen
laut Vereinheitlichungs-Konzept (V4) ohnehin Knoten werden. *Zu prüfen in
Stufe 1:* ob im App-Betrieb noch ein Weg an der Kette vorbei Presets zeigt.

## 4. „Die Effektkette gewinnt"

Je Frame eine billige Prüfung: gibt es in der laufenden Kette einen
**eingeschalteten** Lyrics-Knoten (er selbst und alle Container über ihm an;
bei Host-Gruppen: die Gruppe läuft)? Dann zeichnet die Ebene in diesem Frame
nichts — weder Zeile noch Trail.

- Ein **ausgeschalteter** Lyrics-Knoten zählt nicht: die Ebene übernimmt.
- Wechselt der Host Switcher auf eine Gruppe mit Lyrics-Knoten, tritt die
  Ebene für die Dauer dieser Gruppe zurück. Der Wechsel ist hart (ein Frame);
  ein weicher Übergang wäre eine spätere Zugabe.
- Das Panel zeigt den Zustand an (*Preset shows its own lyrics*), damit ein
  scheinbar wirkungsloser Schalter erklärt ist.

## 5. Das Karaoke-Panel

Neues Panel **Karaoke** (View → Panels), oben der Hauptschalter **Karaoke
on/off** (dazu ein Tastenkürzel und ein Eintrag im Menü), darunter zwei Reiter.

**Reiter 1 — Lyrics.** Die Felder des Lyrics-Knotens für die Ebene: Schrift,
Farben, Lage, Vorschau-Zeilen, Kontur, Init/Frame/Beat-Skripte — und dieselbe
Zeile **Preset** mit den vorhandenen Lyrics-Voreinstellungen. Die
Songtext-Tabelle (Status, Quelle, Alternatives…, Wrong lyrics, Lyrics file…,
Offset for this track) steht hier ebenfalls: sie gehört zum Titel, nicht zum
Knoten, und ist damit auch ohne Lyrics-Knoten in der Kette erreichbar.

**Reiter 2 — Lyrics Trail.** Im Prinzip die Effect List aus dem Mix: eine
kurze Liste von Modulen, die auf der Trail-Surface arbeiten, am Ende fest der
Lyrics Trail. Module kommen aus dem vorhandenen Paletten-Dialog mit einem
**übergeordneten Filter**, der nicht Erlaubtes gar nicht erst zeigt (Entscheid
**K1**). Mitgeliefert wird die Vorgabe aus dem Mix (Blitter Feedback 35,
Fadeout 4, Trail) und ein Schalter **Trail on/off**.

Wie das Panel gebaut wird — der Ketten-Editor in einem eingeschränkten Modus
oder ein eigenes schlankes Panel — ist Entscheid **K2**; es ist der größte
Kostenpunkt des Vorhabens.

## 6. Daten und Ablage

- Die Ebene ist eine **Einstellung der App**, nicht des Presets: eine Datei im
  Benutzerdaten-Ordner (Format der Kette: ein Lyrics-Knoten und eine Liste),
  dazu der Schalter in den QSettings. Presets bleiben unverändert und
  portabel.
- Die Knoten der Ebene bekommen Ids aus demselben Zähler wie die Kette
  (`m_idHochwasser`), damit sich ihre Laufzeiten nie mit denen der Kette
  beißen; sie überleben den Preset-Wechsel (der Trail reißt beim Wechsel
  nicht ab).
- Ob es **mehrere benannte Karaoke-Looks** gibt, zwischen denen man wechselt,
  ist Entscheid **K3**.

## 7. Stufen

| Stufe | Inhalt | Prüfung |
|---|---|---|
| **KE1** | Ebene im Renderer: eigene Surface, zwei Present-Draws, „Kette gewinnt", Schalter (QSettings); Aussehen fest = Voreinstellung *Karaoke Bottom* + Trail-Vorgabe | Bildtests (Text da / Kette gewinnt / aus = nichts); **Vergleichslauf: Root-Surface bitgleich mit Ebene an und aus** |
| **KE2** | Ablage der Ebene (Datei, Laden beim Start), Reiter 1 mit Feldern und Voreinstellungen, Songtext-Tabelle im Panel | UI-Tests; Sichttest |
| **KE3** | Reiter 2: Liste, Paletten-Filter, Trail on/off | UI-Tests (Filter zeigt nur Erlaubtes); Sichttest |
| **KE4** | Menü-Eintrag und Tastenkürzel, Zustandsanzeige „Preset shows its own lyrics", Benutzerhandbuch | Sichttest |
| später | Timeline schaltet die Ebene; weicher Übergang beim Zurücktreten; Text in Fensterauflösung (falls K4 = b) | — |

Nach KE1 ist die Ebene schon nutzbar (Schalter über die Einstellungen), jede
weitere Stufe ist für sich abnehmbar.

## 8. Risiken

- **Editor-Wiederverwendung:** die Feld-Editoren stecken heute in einer
  großen Funktion des Ketten-Panels. Ohne K2 = a müssten die Lyrics-Felder
  herausgelöst oder doppelt gebaut werden.
- **Skripte der Ebene** laufen im Skript-Kontext der Kette oder in einem
  eigenen — ein eigener ist sauberer (kein Namensfang mit Preset-Variablen),
  heißt aber: `reg`/`gmegabuf` des Presets sind von dort nicht lesbar. In KE1
  zu klären; Vorschlag: eigener Kontext.
- **Reihenfolge mit Post-Bloom:** die Ebene liegt **über** dem Bloom (Text
  glüht nicht mit). Soll er mitglühen, müsste sie vor dem Bloom-Composite
  liegen — bewusst nicht in dieser Stufe.
- **Kosten:** eine Surface mehr und zwei Draws; nur wenn die Ebene an ist.

## 9. Offene Entscheide

**K1 — Welche Module darf der Trail-Reiter aufnehmen?**
*Heute:* offen (Patrik S88: „keine Render, keine Host — Umfang noch zu klären").
*Problem:* der Filter braucht eine Regel, die auch für künftige Module gilt.

- **a (Empfehlung):** alles mit Palettenrolle **Transform** oder **Color**
  (bewegen, verformen, einfärben, verblassen); nichts mit Rolle Render oder
  Control (also keine Quellen, keine Listen, Gruppen, Switcher, Puffer).
- **b:** eine von Hand gepflegte kurze Liste (Blitter Feedback, Fadeout, Blur,
  Movement, Dynamic Movement, Color Modifier, Mirror, Water …).

*Warum die Empfehlung:* den Ausschlag gibt, dass die Rollen seit S79 an jedem
Modul stehen und ein neues Modul damit ohne Nacharbeit richtig einsortiert
ist. Für b spricht, dass nur Erprobtes angeboten wird; das wiegt weniger, weil
der Trail-Reiter eine Spielwiese sein soll und ein unpassendes Modul nur den
Trail verdirbt, nie das Preset. Kippen würde es, wenn einzelne Transform-Module
auf der Trail-Surface Unsinn machen (etwa solche, die fremde Puffer lesen) —
dann a mit einer kleinen Ausschlussliste.

**K2 — Wie wird das Panel gebaut?**
*Heute:* alle Feld-Editoren leben im Ketten-Panel (`MultiEffectPanel`).
*Problem:* das Karaoke-Panel braucht dieselben Editoren für Lyrics, Trail und
die Trail-Module.

- **a (Empfehlung):** das Ketten-Panel bekommt einen **eingeschränkten Modus**
  (fester Wurzelknoten, Paletten-Filter, kein Laden/Speichern von Presets);
  das Karaoke-Panel hält davon zwei Instanzen, je Reiter eine, gebunden an
  die Knoten der Ebene.
- **b:** ein eigenes, schlankes Panel; die Lyrics-Felder werden aus dem
  Ketten-Panel in eine gemeinsame Funktion herausgelöst, die Trail-Module
  bekommen nur ihre wichtigsten Regler.

*Warum die Empfehlung:* den Ausschlag gibt, dass bei a jedes Feld, jede
Voreinstellung, Modulation und jedes Skript sofort da ist und künftige Felder
ohne zweite Pflegestelle erscheinen. Für b spricht eine aufgeräumtere
Oberfläche; das wiegt weniger, weil b bei den Trail-Modulen entweder wenig
kann oder die Editoren doch nachbaut. Kippen würde es, wenn sich das
Ketten-Panel nicht sauber an einen zweiten Baum binden lässt — das zeigt sich
am Anfang von KE2; dann b.

**K3 — Ein Karaoke-Look oder mehrere?**
*Heute:* nichts festgelegt.

- **a (Empfehlung):** **ein** Look (die Datei der Ebene); Abwechslung über die
  vorhandenen Voreinstellungen der beiden Knoten.
- **b:** benannte Looks als eigene Dateien mit Auswahlliste im Panel.

*Warum die Empfehlung:* den Ausschlag gibt, dass die Voreinstellungen schon
da sind und a ohne neue Dateiart auskommt. Für b spricht das Umschalten eines
ganzen Looks (Lyrics + Trail + Module) mit einem Griff — genau das, was die
Timeline später brauchen könnte; es wiegt jetzt weniger, weil sich b auf a
nachrüsten lässt. Kippen würde es, wenn du schon jetzt je Abend oder je
Musikrichtung umschalten willst.

**K4 — Text in Ketten- oder Fensterauflösung?**
*Heute:* der Lyrics-Knoten rastert in Kettenauflösung.

- **a (Empfehlung):** Kettenauflösung, wie der Knoten — Ebene und Knoten
  sehen gleich aus.
- **b:** die Zeile in Fensterauflösung rastern (schärfer bei Render Scale
  unter 1), der Trail bleibt in Kettenauflösung.

*Warum die Empfehlung:* den Ausschlag gibt die Gleichheit mit dem Knoten
(„derselbe Songtext", gleiche Voreinstellungen, gleiche Lage) und der kleinere
Umbau. Für b spricht die Lesbarkeit, wenn Presets mit kleinem Render Scale
laufen; kippen würde es, sobald das im Sichttest stört.

**K5 — Trail der Ebene, wenn die Kette gewinnt?**
*Heute:* nicht festgelegt; Abschnitt 4 nimmt „die Ebene zeichnet dann nichts" an.

- **a (Empfehlung):** die Ebene zeichnet nichts, auch keinen Trail — das
  Preset bestimmt das ganze Aussehen.
- **b:** die Zeile kommt vom Preset, der Trail der Ebene läuft weiter, sofern
  das Preset keinen eigenen Lyrics Trail hat.

*Warum die Empfehlung:* den Ausschlag gibt, dass der Trail der Ebene Schrift
und Lage vom Lyrics-Knoten der Ebene leiht — über einer anders gesetzten
Preset-Zeile stünde er an der falschen Stelle. Für b spricht, dass ein Preset
mit schlichtem Lyrics-Knoten den Trail „geschenkt" bekäme; dazu müsste der
Trail der Ebene dem Knoten des Presets folgen. Kippen würde es, wenn du das
ausdrücklich willst — technisch möglich, eine Stufe mehr.
