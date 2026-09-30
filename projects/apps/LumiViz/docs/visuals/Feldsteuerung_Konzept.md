# LumiViz — Block F: jedes Feld steuerbar (Konzept)

> **Version:** 0.1.0
> **Datum:** 2026-09-30 (Session 84)
> **Typ:** Konzept — **Entscheide E1–E6 gefallen (S84: alle a**, E3 um den
> Beat-Slot erweitert, E5 präzisiert); Umsetzung ab F1
> **Status:** Aktiv
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Formelfelder_Inventar.md](Formelfelder_Inventar.md) (Blöcke A–E, S79;
> Block-F-Ideen §F) · [Knoten_Parameter_Konzept.md](Knoten_Parameter_Konzept.md)
> (Strang D: Opt-in, Frame-Kopie, „ein Knoten, ein Skript-Träger") ·
> [Skript_Variablen_Konzept.md](Skript_Variablen_Konzept.md) (§3 SSOT Modul×Slot,
> P3) · `Offene_Punkte.md` §6 (Bestandsaufnahme S83) und §7 (MIDI/OSC)
> **Sprache:** Deutsch

## 1. Ziel

Jedes **skalare Feld** eines Effekt-Knotens — Zahl, Schalter, Aufzählung,
Farbe — lässt sich steuern:

1. **per Skript** (init · frame · beat · song) unter einem vorhersagbaren Namen,
   ohne dass jemand je Feld vier Stellen von Hand pflegt (Bindung in `run*`,
   Zeile in `ScriptVariables.cpp`, Doku-Kommentar, Test);
2. **ohne Formel** über eine Modulationsquelle je Feld (Bass/Beat/LFO × Tiefe);
3. aus **einer** Adressierung, die später auch MIDI/OSC nutzen (§7 der Offenen
   Punkte: „gemeinsame Adressierung statt dreier eigener").

Nicht Ziel: Texte, Code-Felder, Bilder, Listen/Tafeln (Farbtafeln, Stützstellen,
Kerne), Quellen-Wahl (Datei/Kamera), MilkDrop-Pipeline (eigene Welt). Sie
bleiben, wie sie sind; Farbtafeln/Stützstellen sind ein möglicher Nachfolger
(§7).

## 2. Befund (Stand S84)

**Wie ein Feld heute skriptbar wird.** Jeder Renderer kopiert ausgewählte Felder
in lokale `double`s, gibt sie als `ParamVar`-Liste an `runParamScript` und
rechnet danach mit den Kopien (`MultiEffectVisualizer.cpp:9158–9241`). Alles
andere liest er direkt aus `params` — das ist genau die Liste der nicht
skriptbaren Felder aus §6 der Offenen Punkte.

| Kennzahl | Wert |
|---|---|
| Knotentypen / Felder | 92 / 803 |
| davon Zahl / Text / Schalter / Liste | 413 / 283 / 91 / 15 |
| Typen mit `ParamVar`-Liste | 73 (55 `run*` mit `runParamScript`) |
| skalare Felder gebunden (grob) | ~240 |
| skalare Felder **ungebunden** | ~220 (87 Schalter, 54 Aufzählungen, 46 Ganzzahl, 37 Kommazahl) + Farben ohne Bindung |

**Keine Reflexion.** Es gibt keine Tabelle, die Felder eines `…Params`-Structs
aufzählt und ansprechen kann. Generisch ist nur der Serializer
(`nodeToJson` + `nodepresets::fieldNames()`, von Hand je Typ geschrieben) — ihn
nutzt das Feld-Inventar (`test_FieldInventory`), und der Ernter
(`harvest_field_docs.py`) kennt aus Header und Panel je Feld Art, Vorgabe,
Bereich `lo/hi` und Beschreibung.

**Namen.** Überwiegend gilt „Feldname klein" (`fadeLen` → `fadelen`), Farben
als `<feld>_r/_g/_b` (0..1, `FarbKopie`), ISF/Uniforms unter ihrem `key`,
Kollisionen als `in_<name>`. Abweichend und in Presets im Umlauf:
`cx/cy`←`centerX/Y`, `rot`←`rotation`, `jx/jy`←`juliaX/Y`, `ox/oy`←`offsetX/Y`,
`n`←`pointCount`/`orbCount`, `res`←`resolution`, `rings`←`ringAmp`,
`life`←`lifeSeconds`, `spread`/`dir`/`gravx`/`gravy`/`size` (GPU-Partikel),
`overrideblend`←`enabled`, `alphain/alphaout` (skaliert /255).

**Teure Felder.** Viele Felder lösen bei einer Änderung Arbeit aus, die pro
Frame nicht tragbar ist:
- *Neukompilieren eines Skript-Hosts mit Werte-Schnappschuss*: Fraktale 2D/3D,
  Zoomer, Lyapunov, Kleinian, Domain Warp, Strange Attractor, Flame, Reaction
  Diffusion, Terrain/Camera/SuperScope 3D, Glow Orbs, Dynamic Movement, Bump.
- *Shader*: Shadertoy, Mesh Warp, Pixel Filter, ISF, GPU-Partikel (Code, Quellen).
- *Textur/LUT/Datei*: Text (fast alle Felder → neu rastern), ColorMap-Stopps,
  Bilder, AVI/Video/Kamera, `gradientPreset` der Fraktale/SuperScope.
- *Puffer/Mesh/FBO*: Movement (`rectCoords/wrap/subpixel`), Terrain
  `resolution`, Mesh Warp `gridX/Y`, ISF-Geometrie/Passes, GPU-Partikel
  `count`, Bloom `downsample`, Video Delay `delay/useBeats`, Starfield
  `maxStars`, Shadertoy-Buffer-Anzahl.
- *Zustand neu setzen* (billig, verändert aber das Bild): Interferences
  `rotation`, Dot Plane/Fountain `startRotation`, Channel Shift `mode`.

⚠ **Schon heute** sind drei solche Felder frei gebunden: `gridx/gridy` (Mesh
Warp), `delay` (Video Delay), `maxstars` (Starfield) — ein Frame-Skript, das sie
ändert, baut je Frame neu auf.

## 3. Architektur

### F1 — Feldtabelle (SSOT der Adressierung)

Eine Tabelle je Knotentyp, **erzeugt** vom Ernter wie `FieldDocs.cpp`
(`FieldBindings.cpp`), je Eintrag:

| Spalte | Inhalt |
|---|---|
| `feld` | Feldname im Struct (`rotationSpeed`) |
| `name` | Skriptname nach Schema (`rotationspeed`; Farben `_r/_g/_b`) |
| `alias` | bisherige Namen, die gültig bleiben (`rotspeed`) |
| `zugriff` | Zeiger-auf-Member (`&FractalZoomerParams::rotationSpeed`) |
| `art` | Ganzzahl · Kommazahl · Schalter · Aufzählung(n Werte, Versatz) · Farbe |
| `lo/hi` | Bereich aus der Panel-Zeile (hat der Ernter schon) |
| `klasse` | **live** · **struktur** · **zustand** (§2 „Teure Felder") |

Zeiger-auf-Member heißt: ein umbenanntes oder gelöschtes Feld ist ein
**Compilerfehler**, kein stilles Loch. `klasse` pflegt eine kleine Liste im
Ernter (Vorgabe `live`); ein Wächter-Test prüft sie (§5).

### F2 — ein generischer Läufer in `renderNode`

`renderNode` hat seit S83 schon den Einhängepunkt („Script active": Kopie der
Params vor dem Dispatch). Dort, **nur** wenn der Knoten Skript oder Modulation
trägt (Opt-in, null Kosten sonst):

1. `EffectParams kopie = node.params` (die Frame-Kopie aus Strang D, jetzt an
   einer Stelle statt in jedem `run*`);
2. Werte der Tabellenfelder als Startbelegung hinein — mit derselben
   Fortschreibungs-Semantik wie heute (S54: Init setzt einmalig, Frames schreiben
   fort, eine Reglerbewegung gewinnt);
3. Slots laufen **einmal** (derselbe `ScriptSlotHost`);
4. Rücklesen mit Typpflege: runden (Ganzzahl), `> 0.5` (Schalter), klemmen auf
   `lo/hi` bzw. Aufzählungsbereich, Farben aus `_r/_g/_b` zusammensetzen;
5. `std::visit(Visitor, kopie)` — der Renderer sieht nur noch Werte.

**Umgesetzt (S84) mit einer Abweichung:** der Lauf bleibt am bisherigen Punkt
im Renderer (`runParamScript`), statt in `renderNode` vorgezogen zu werden —
manche Renderer binden dort auch **Nicht-Felder** (ISF-Regler unter ihrem
`key`, eigene Shader-Uniforms), die sonst verloren gingen, und die Zeit-
semantik (Skript vor dem Zeichnen, nach dem Aufbau) bleibt unverändert.
`renderNode` legt nur die Frame-Kopie an; `runParamScript` nimmt die
Tabellenfelder des Knotens dazu, verknüpft gleichnamige Hand-Bindungen und
schreibt nach dem Lauf in die Kopie. Die Hand-Listen sind damit für Felder
**überflüssig** und fallen typweise weg (Migration §4); was bleibt, sind
Nicht-Felder. „Ein Knoten, ein Skript-Träger" bleibt gewahrt.

Hinweis Alias: beschreibt ein Skript den bisherigen Namen (`cx`), gilt dieser
— auch in Folgeframes (Fortschreibung). Wer später auf den Schema-Namen
(`centerx`) wechselt, muss den alten nicht mehr beschreiben; beide zugleich zu
beschreiben ist nicht vorgesehen.

**Eigene Hosts** (Fraktale, 3D-Knoten, Glow Orbs … — Werte stehen im
Kompilier-Schnappschuss) und **Effekt-Skripte** (SuperScope, Color Modifier,
Movement: `x/y/i/v` …) sind ein eigener Mechanismus. Sie bekommen die Tabelle
in Stufe F2b: dieselben Felder, geschrieben in *ihren* Host, und der
Schnappschuss lernt, Live-Felder nicht mehr als Kompiliergrund zu zählen.

### F3 — Strukturfelder

Siehe Entscheid E3. Kern: ein Strukturfeld darf nicht jeden Frame einen
Neuaufbau auslösen.

### F4 — Modulation ohne Formel

Je Feld im Editor ein kleiner Knopf „~" neben dem Regler: **Quelle**
(Bass · Mid · Treble · Pegel · Beat-Hüllkurve · LFO Sinus/Dreieck/Säge/
Rechteck/Zufall-halten), **Tiefe** (Anteil des Bereichs, ±), **Rate** (Hz oder
Beats), **Glättung**. Ablage am Knoten als Liste `mods` (nur wenn belegt), also
Teil des Presets. Angewendet im selben Läufer (F2), Reihenfolge nach E5. Der
Regler zeigt die Bewegung (Schatten-Marke), der Basiswert bleibt einstellbar.

### F5 — Referenz und Hervorhebung aus der Tabelle

Die ⓘ-Referenz und `ScriptVariables` ziehen die generischen Felder aus F1
(heute von Hand, `ScriptVariables.cpp:50–607`); Hand-Zeilen bleiben nur für
Nicht-Felder (Audio-Satz, `b/w/h`, Effekt-Skript-Variablen, Laufzeitwerte wie
`colorphase`). Der Highlighter wird modulbewusst (P3 Punkt 3,
`EelScriptEditing.hpp:97` „Stoff für Block F").

### F6 — Adresse für MIDI/OSC (nur vorbereitet)

`<knotenname>/<name>` (Knotenname ist eindeutig wie bei Host-Gruppen) — dieselbe
Tabelle, dieselbe Typpflege beim Schreiben. Gebaut wird es mit der MIDI/OSC-Idee.

## 4. Stufen

| Stufe | Inhalt | Nachweis |
|---|---|---|
| **F1 ✅ S84** | Ernter erzeugt `src/visualizers/FieldBindings.cpp` (494 Felder, 84 Typen; Träger: 54 ParamSkript, 19 eigener Host, 11 keiner); API `include/visualizers/multieffect/FieldBindings.hpp`; `KLASSEN`/`ALIASSE` im Ernter | `test_FieldBindings`: Gegenwert über die Tabelle setzen → steht **genau** unter dem JSON-Namen, kein anderes Feld bewegt sich (Gegenprobe: vertauschter Member fällt auf); Index↔Typschlüssel; Namen eindeutig |
| **F2 ✅ S84** | `runParamScript` bindet selbst **alle** Tabellenfelder (Hand-Bindungen gleichen Namens werden verknüpft, Nicht-Felder wie ISF-Schlüssel bleiben) und schreibt in eine Frame-Kopie, die `renderNode` für Typen mit Träger ParamSkript **und** Code anlegt (`m_gesteuerteKopie`). Typpflege (Farbe aus `_r/_g/_b`, Aufzählung/Schalter gerundet und geklemmt, Alias mit Skala), Strukturregel E3 mit Speicher je Knoten (`LeafRuntime::strukturWert`, Regler gewinnt), einmalige Log-Meldung | `test_FeldSteuerungGl`: 238 Live-Felder in allen ParamSkript-Typen wirken (Gegenprobe: ohne Rückschreiben 240 Fehler); Strukturfeld `gridx` folgt dem Frame-Slot nicht, `mixamount` schon; `test_ScriptVariablesGl` grün |
| F2-Migration | `ParamVar`-Listen typweise entfernen | derselbe Wirkungstest je Typ |
| **F2b** | Eigene Hosts/Effekt-Skripte an die Tabelle | Kompilier-Zähler bleibt bei Live-Feldern stehen |
| **F4** | Modulation (Daten, Läufer, Editor-Knopf) | Test: LFO bewegt Feld, Tiefe 0 = unverändert; Sichttest |
| **F5** | Referenz/Highlighter aus der Tabelle | Referenz-Test: jede Tabellenzeile erscheint |

## 5. Tests und Wächter

- **Wirkungstest generisch** (neu, ersetzt den Handabgleich): für jeden Typ und
  jedes Tabellenfeld ein Frame-Skript `name = <gegenwert>` — nach dem Frame
  trägt die Kopie den Wert (Typpflege inklusive). Die Gegenwerte liefert schon
  `make_field_probes.py`/`gegenwert`. Gegenprobe: ohne Skript unverändert.
- **Klassen-Wächter**: Live-Felder dürfen bei Änderung keinen Neuaufbau
  auslösen — gezählt über Kompilier-/Allokations-Zähler (Muster
  `slotRunCount`, S83). Schlägt er an, gehört das Feld in `struktur`.
- **Namens-Wächter**: jeder heutige Skriptname bleibt gültig (Name oder Alias) —
  die Liste der heutigen Namen liefert `harvest_script_vars` einmalig als
  Golden.

## 6. Entscheide

Format je Frage: Ort · heute · Problem · Optionen · **a = Empfehlung** · Warum.

### E1 — Woher kommt die Feldtabelle?

- **Ort:** neue, erzeugte Datei `src/visualizers/FieldBindings.cpp` (F1).
- **Heute:** keine Reflexion; nur der Serializer zählt Felder auf.
- **Problem:** Ohne Tabelle kein generischer Weg; die Tabelle darf nicht still
  veralten.
- **a (Empfehlung):** *„Der Ernter erzeugt die Tabelle als C++ mit
  Zeiger-auf-Member, wie `FieldDocs.cpp`; ein umbenanntes Feld bricht den Build."*
- **b:** *„Kein Code: der Läufer geht je Frame über den JSON-Serializer
  (`nodeToJson` → Werte → `nodeFromJson`)."*
- **c:** *„Makro-Liste je Struct in `EffectChain.hpp` (`LUMI_FELDER(…)`)."*

*Warum a:* Der Ernter kennt Art, Bereich und Beschreibung schon, und der
Build-Fehler ist der beste Wächter. b braucht keine Generierung, kostet aber je
Frame einen JSON-Rundlauf (0,3–1,5 KB plus Code-Texte, bei Listen ganze
Teilbäume) und verliert Typen (Farben als double, JSON-Aliasse). c wäre
selbsterklärend, verdoppelt aber jede Felddeklaration im Header. Kippen würde
a, wenn der Ernter als Werkzeug im Build unerwünscht ist — er läuft heute von
Hand, die erzeugte Datei ist eingecheckt (wie FieldDocs).

### E2 — Wie heißen die Variablen?

- **Ort:** Spalten `name`/`alias` der Tabelle.
- **Heute:** meist Feldname klein; einige Abweichungen (§2), in Presets im Umlauf.
- **Problem:** Ein Schema für ~460 Felder, ohne vorhandene Presets zu brechen.
- **a (Empfehlung):** *„Name = Feldname klein (Farben `_r/_g/_b`); bisherige
  abweichende Namen bleiben als Alias gültig und stehen in der Referenz als
  ‚auch: …'."*
- **b:** *„Nur das neue Schema; alte Namen werden beim Laden in Skripten
  umgeschrieben (Import-Migration)."*
- **c:** *„Bisherige Namen bleiben Hauptname; das Schema gilt nur für neu
  gebundene Felder."*

**Entscheid Patrik S84: a.** Nachfrage „sind die Feldnamen über die Module
abgeglichen?" — Stand S84: 355 skalare/sonstige Feldnamen ohne Schreibweisen-
Doppel; die häufigen sind einheitlich (`blend` 37×, `color` 13×, `colors` 10×,
`speed` 7×). Uneinheitlich sind einige **Familien**: Rotation (`rotation` 5×,
`rotationSpeed` 3×, `rotSpeed`, `rot`, `spinStep`), Anzahl (`points` 4×,
`count` 3×, `pointCount` 2×, `orbCount`, `maxStars`, `stars`, `particles`),
Größe (`size`, `dotSize`, `sizePx`, `radius`, `scale` — teils verschiedene
Bedeutung). F1 benennt **keine** Felder um (JSON/Presets); ein Namensabgleich
wäre ein eigener Schritt mit JSON-Aliassen.

*Warum a:* Vorhersagbar für alle neuen Felder, kein Preset bricht, kein
Umschreiben von Nutzer-Code. b ist sauberer, fasst aber fremde Skripttexte an
(Kommentare, Stringvergleiche) — riskant für wenig Gewinn. c hielte zwei
Schemata dauerhaft nebeneinander. Kippen würde a, wenn Aliasse im Highlighter
oder in der Referenz verwirren — dann b.

### E3 — Was gilt für Strukturfelder?

- **Ort:** Spalte `klasse`, Läufer F2.
- **Heute:** meist nicht gebunden; drei doch (`gridx/gridy`, `delay`,
  `maxstars`) — Neuaufbau je Frame möglich.
- **Problem:** Skriptbar ja, aber kein Neuaufbau pro Frame.
- **a (Empfehlung):** *„Strukturfelder übernimmt der Läufer nur aus Init- und
  Song-Slot (einmalig bzw. je Titelwechsel); Änderungen im Frame-/Beat-Slot
  wirken nicht und werden einmal im Log gemeldet. Die Referenz kennzeichnet sie
  ‚nur init/song'."*
- **b:** *„Strukturfelder sind gar nicht skriptbar (Referenz: ‚fest')."*
- **c:** *„Alles frei; die Referenz warnt vor Neuaufbau je Frame."*

**Entscheid Patrik S84: a, erweitert um den Beat-Slot** — Strukturfelder
wirken aus Init-, Song- **und Beat-Slot** (etwa „alle 20 Beats eine andere
Auflösung": Zähler im Beat-Slot); nur der Frame-Slot bleibt ausgeschlossen. Ein
Neuaufbau je Beat (~2/s) ist tragbar, je Frame nicht.

*Warum a:* Die sinnvollen Fälle — „je Song eine andere Auflösung/Anzahl" —
bleiben möglich, der teure Fall ist ausgeschlossen, und die drei heute freien
Felder verlieren nur ihre (teure) Frame-Wirkung. b ist am einfachsten, nimmt
aber den Song-Slot-Nutzen. c überlässt die Falle dem Nutzer. Kippen würde a,
wenn es Presets gibt, die `gridx` o. ä. bewusst je Frame animieren — vor dem
Umbau prüfbar — **geprüft S84:** kein mitgeliefertes Preset/Beispiel
(`asset/nodepresets`, `examples`, `presets`) setzt `gridx`, `gridy`, `maxstars`
oder `delay` per Skript.

### E4 — Ersetzt der generische Läufer die Hand-Listen?

- **Ort:** `renderNode` und die 55 `run*` mit `runParamScript`.
- **Heute:** je Renderer eine Hand-Liste, ein Lauf je Knoten.
- **Problem:** Zwei Läufe je Knoten verletzten „ein Knoten, ein Skript-Träger"
  (doppelter Zustand, doppelte Kosten).
- **a (Empfehlung):** *„Der generische Läufer wird der einzige
  Parameter-Skript-Träger; `runParamScript` im Renderer wird zur Durchreiche,
  die Hand-Listen fallen typweise weg, jeder Schritt mit Wirkungstest."*
- **b:** *„Hand-Listen bleiben; der generische Läufer bedient nur die bisher
  ungebundenen Felder, in denselben Host eingespeist."*

*Warum a:* Eine Wahrheit statt 55 Listen plus Tabelle; genau das ist der Zweck
von Block F. b ginge schneller, hinterlässt aber zwei Bindungswege, die
auseinanderlaufen (Befund S54: 43 von 129 Frame-Kopien wurden nie gelesen).
Kippen würde a, wenn ein Renderer abgeleitete Werte bindet, die kein Feld sind —
die bleiben als kleine Hand-Ergänzung erlaubt (wie `colorphase`).

### E5 — Modulation und Skript: wer zuerst?

- **Ort:** Läufer F2/F4.
- **Heute:** keine Modulation.
- **Problem:** Beide wirken auf dasselbe Feld.
- **a (Empfehlung):** *„Basiswert → Modulation → Skript. Das Skript sieht den
  modulierten Wert und darf ihn überschreiben. Modulation gehört zum Knoten und
  damit ins Preset."*
- **b:** *„Basiswert → Skript → Modulation. Die Modulation liegt immer obenauf."*

**Entscheid Patrik S84: a.** Präzisiert auf Nachfrage:
- *Modulation* heißt hier: **ohne Formel** ein Feld bewegen lassen — „Größe
  schwingt mit dem Bass um 30 %", „Drehung per LFO, 4 Beats Periode". Eingestellt
  über einen „~"-Knopf an der Feldzeile, gespeichert am Knoten.
- **Schalter:** „Script active" schaltet nur die Skripte. Daneben ein eigener
  Knotenschalter **„Modulation active"** (Vorgabe an) und je Modulation ein
  eigenes Häkchen. Ohne eingetragene Modulation ändert sich nichts — bestehende
  Presets verhalten sich wie heute.

*Warum a:* Das Skript ist das mächtigere Werkzeug und hat das letzte Wort —
wer eine Modulation per Skript bändigen will (z. B. nur auf dem Beat), kann es.
b ist intuitiver für „Wackeln obendrauf", nimmt dem Skript aber die Kontrolle.
Kippen würde a, wenn Modulation vor allem live (MIDI-Stil) gedacht ist — dann
gehört sie ans Ende.

### E6 — Wie erscheinen Schalter und Aufzählungen im Skript?

- **Ort:** Typpflege beim Rücklesen (F2), Referenz (F5).
- **Heute:** gebundene Aufzählungen als Zahl (z. B. `blend` 0..3).
- **Problem:** Zahlen sind knapp, aber nicht selbsterklärend.
- **a (Empfehlung):** *„Als Zahl: Schalter 0/1 (≥ 0.5 = an), Aufzählung als
  Index, gerundet und geklemmt; die Referenz nennt die Werte (‚0 replace,
  1 additive …')."*
- **b:** *„Zusätzlich benannte Konstanten je Aufzählung
  (`blend_additive = 1` …)."*

*Warum a:* Passt zu EEL/AVS (dort ist alles Zahl) und kostet nichts. b liest
sich besser, bläht aber den Namensraum um hunderte Konstanten auf und kollidiert
leicht mit Nutzervariablen. Kippen würde a, wenn Skripte vor allem von Hand
geschrieben und gelesen werden sollen — b lässt sich später ergänzen.

## 7. Offen / später

- Farbtafeln und Stützstellen (`colors[i]`, ColorMap) als indizierte Namen
  (`colors_2_r` …) — nach F2.
- Punkt-Slots (je Punkt) bleiben außen vor (Kosten).
- Benannte Konstanten (E6 b), MIDI/OSC (F6).
