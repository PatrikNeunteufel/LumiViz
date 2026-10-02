# LumiViz — Karaoke-Ebene (Kurzkonzept)

> **Version:** 0.1.0
> **Datum:** 2026-10-02 (Session 88)
> **Typ:** Konzept — **alle Entscheide gefallen** (K1 a, K2 a, K3 b, K4 b,
> K5 a, K6 a); KE1 und die Mischarten (K6) gebaut, KE2–KE4 offen
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

### Stand KE1 — ✅ gebaut (S88)

- Schalter: **Settings → Audio → Karaoke layer** (QSettings `karaoke/enabled`,
  Vorgabe aus; `MultiEffectVisualizer::setKaraokeEbene`).
- Renderer: `baueKaraokeEbene` (feste Knoten mit Ids ab `0xF000…`, außerhalb
  von `m_root`), `rechneKaraokeEbene` nach dem Ketten-Walk (die Zeile wird nur
  gerastert, die Trail-Liste mischt nicht aus), `legeKaraokeAuf` nach dem
  Present-Draw (Trail additiv, Zeile mit Alpha), `ketteZeigtLyrics`.
- Aussehen fest: Vorgaben des Lyrics-Knotens; Trail = Blitter Feedback 35,
  Fadeout 4, Lyrics Trail 120 ms / 7 % nach oben (wie im Mix).
- Geprüft: Bildtest (Text da · Root-Surface **bitgleich** mit Ebene an und aus,
  über 20 Frames einer Rückkopplungs-Kette ohne Clear · ohne Songtext nichts ·
  eingeschalteter Lyrics-Knoten gewinnt, ausgeschalteter nicht); Bilder
  angesehen (`LUMIVIZ_KARAOKE_SNAP`); Vergleichslauf Regressionsliste 84/84
  bitgleich (Ebene aus).
- **Abweichungen vom Konzept in KE1:** die Laufzeiten der Ebene überleben den
  Preset-Wechsel **nicht** (der Wechsel leert alle Laufzeiten; der Trail
  beginnt danach neu); der Blend des Lyrics-Knotens (Additive, 50/50) wirkt
  in der Ebene nicht — die Zeile liegt immer deckend mit Alpha.
- **Nicht geprüft:** im laufenden Programm, mit Post-Bloom, mit Render Scale,
  mit Host Switcher (Gruppe mit Lyrics-Knoten).

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

## 9. Entscheide

**Stand (Patrik, S88):** **K1 a · K2 a · K5 a** entschieden. Zu K5: „natürlich
nein — das ist ja der springende Punkt, wenn ein Preset alles aufeinander
abstimmt." **K3 und K4:** vorläufig a, Patrik beurteilt sie nach einer
Erklärung der Begriffe neu („Look" = Zustand der ganzen Ebene im Unterschied
zu den Voreinstellungen je Knoten; K4 betrifft nur Presets mit Render Scale
unter 1). Beide lassen KE1 unberührt.

**Entschieden (Patrik S88): K3 b · K4 b · K6 a.** Damit stehen alle sechs:
K1 a, K2 a, K3 b (Karaoke-Presets als Dateien, Icon „Marke"), K4 b (Zeile in
Fensterauflösung, eigener kleiner Schritt), K5 a, K6 a (Mischart je Teil +
Stärke, Kontur bleibt normal).

**K6 vorab gebaut (S88):** `MultiEffectVisualizer::setKaraokeMischung`
(Teil Zeile/Trail, Art 0–8, Stärke 0–100 %), Schalter vorerst unter Settings →
Audio (*Karaoke line blend*, *Karaoke trail blend*; QSettings
`karaoke/LineBlend|LineStrength|TrailBlend|TrailStrength`). Umsetzung: je Art
eine Blend-Gleichung beim Present, XOR über die Logik-Operation des Kontexts
(fehlt sie, fällt XOR auf Invert zurück); bei einer Art ungleich Normal werden
Füllung und Kontur-Ring getrennt gerastert, der Ring liegt immer normal.
Geprüft im Bildtest über einfarbigem Grund: alle acht Arten treffen den
gerechneten Wert im Inneren der Buchstaben (±2), Stärke 50 % und 0 %, Kontur
vorhanden, Trail Additive gegen Subtract; XOR unterscheidet sich an der grauen
Vorschau-Zeile wie erwartet von Invert. Kontaktbogen angesehen. **Nicht
geprüft:** auf dem multisampled Fenster der App (der Test rechnet in eine
gewöhnliche Fläche) — das ist der offene Punkt für XOR.
**Folgefrage Patrik, ✅ gebaut (S88):** dieselben Mischarten auch am
Lyrics-Knoten der Kette („die Kette gewinnt" soll nicht ärmer sein als die
Ebene). Feld **Blend** von Lyrics und Lyrics Trail hat neun Werte; die Arten
3–8 rechnet der Text-Shader mit denselben Gleichungen wie der Present. Ab Art 3
werden Füllung und Kontur-Ring getrennt gerastert (`textTexture2`), der Ring
liegt normal. **Replace, Additive und 50/50 bleiben am Knoten unverändert**
(die Kontur mischt dort wie bisher mit) — vorhandene Presets rechnen bitgleich.
Unterschied zur Ebene: dort gilt die getrennte Kontur schon ab Additive, und
der Knoten hat keinen Stärke-Regler. Feldtabelle und Tooltips neu erzeugt
(Ernter), Bildtest über einfarbigem Grund mit derselben Erwartungstabelle wie
die Ebene, Vergleichslauf 84/84 bitgleich.

**Nachtrag Patrik (S88, nach der Erklärung):**

- **K3:** „also sozusagen ein Karaoke-Preset" — und ein Kandidat für das
  Reserve-Icon **Marke** (`lumiviz_marke.ico`). Tendenz **b** (benannte
  Karaoke-Presets als eigene Dateien); Bestätigung steht aus.
- **K4:** Patrik fragt, ob b nicht flexibler wäre. Einschätzung Claude: b ist
  nicht flexibler, aber **schärfer** — es wirkt nur bei Presets mit Render
  Scale über 1 (Kette kleiner als das Fenster), dort aber deutlich. Mit dem
  Mischen über GL-Blending (K6) braucht b keine zusätzliche Fläche; Aufwand:
  die Rastergröße der Zeile vom Kettenmaß lösen, neu rastern beim Ändern der
  Fenstergröße. Der Trail bliebe in Kettenauflösung. Empfehlung jetzt: **b**,
  als eigener kleiner Schritt. Entscheid steht aus.
- **K6 (neu, Idee Patrik):** wenn die Ebene schon eine eigene Schicht ist —
  **Mischart der Ebene** wie von den Listen bekannt (Replace, Add, Sub, Xor …),
  insbesondere „die dahinterliegende Farbe invertieren, ganz oder zu einem
  gewissen Grad". Siehe K6 unten.

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

**K6 — Mischart der Ebene (Idee Patrik S88)**
*Heute (KE1):* die Zeile liegt deckend mit Alpha über dem Bild, der Trail
additiv; beides fest.
*Idee:* die Ebene mischt sich wählbar mit dem Bild dahinter — bis hin zum
Invertieren der Farbe unter dem Text.

Machbar **ohne** das Bild dahinter zu lesen, allein über das Blending beim
Present (darum auch mit K4 = b verträglich und ohne zusätzliche Fläche):

| Mischart | Wirkung | Umsetzung |
|---|---|---|
| Normal | deckend mit Alpha (heute) | Alpha-Blending |
| Additive | Text hellt auf | Quelle + Ziel |
| Subtract | Text dunkelt ab | Ziel − Quelle |
| Invert | weißer Text kehrt die Farbe darunter um; grauer Text nur teilweise — der „gewisse Grad" ist die Helligkeit/Deckkraft des Texts | Quelle·(1−Ziel) + Ziel·(1−Quelle) |
| XOR | bitweises Exklusiv-Oder wie in AVS | Logik-Operation der Grafikkarte |
| Multiply · Maximum · Minimum · 50/50 | wie bei den Listen | Blend-Gleichungen |

„Ignore" entfällt als Mischart — das ist der Hauptschalter.

- **a (Empfehlung):** ein Feld **Blend** je Teil der Ebene — eines für die
  Zeile, eines für den Trail — mit der Liste oben, dazu ein Regler
  **Strength** (0–100 %) für den Grad; Vorgabe wie heute (Zeile Normal, Trail
  Additive). Gebaut mit dem Panel in KE2.
- **b:** nur ein gemeinsames Feld für die ganze Ebene.
- **c:** nicht bauen; Farbe und Kontur wie bisher über die Lyrics-Felder.

*Warum die Empfehlung:* den Ausschlag gibt, dass Zeile und Trail verschiedene
Aufgaben haben (die Zeile soll lesbar sein, der Trail darf Effekt sein) und
der Aufwand für zwei Felder kaum höher ist als für eines. Für b spricht die
einfachere Bedienung; für c, dass Invert und XOR über mittelgrauem Bild fast
keinen Kontrast geben (Umkehrung von 50 % Grau ist 50 % Grau) — die Lesbarkeit
hängt dann am Preset. Das wiegt weniger, weil die **Kontur** normal gezeichnet
bleiben kann, während nur die Füllung mischt; das wäre Teil von a. Kippen
würde es, wenn die Logik-Operation für XOR auf dem (multisampled) Fenster der
App nicht sauber arbeitet — dann XOR weglassen oder über eine Zwischenfläche.

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
