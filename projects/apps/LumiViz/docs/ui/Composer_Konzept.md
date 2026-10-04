# LumiViz — Composer (Konzept)

> **Version:** 0.5.0
> **Datum:** 2026-10-04 (Session 93)
> **Typ:** Konzept — **gebilligt, alle Entscheide gefallen** (Patrik S93: C2 a,
> C3 eigener Mischer, C4 a, C5 a, C6 a mit Sperre, C7 a, C9 a, C10 a);
> Stufen **C0 und C0b (Belege) erledigt**, sonst nichts gebaut
> **Status:** Entwurf
> **Vorgabe:** rückbaubar (§12); entsteht ab C1 im Zweig `composer`
> **Zielgruppe:** App-Entwickler
> **Bezug:** `Offene_Punkte.md` §7 „Weg zur Timeline" (Ideen Patrik S84, S88,
> S93) · [Visual_Playlist_Konzept.md](Visual_Playlist_Konzept.md) §5a
> („Composer" als Fernziel, S51) · [Hotkey_Konzept.md](Hotkey_Konzept.md) §1
> (Stufe 3) · [Karaoke_Ebene_Konzept.md](../visuals/Karaoke_Ebene_Konzept.md)
> **Sprache:** Deutsch

---

## 1. Ziel und Name

Der **Composer** ordnet Visuals über der Zeit an — gemeint wie das
Arrangement-Fenster einer DAW (FL Studio und ähnliche): die Zeit läuft nach
rechts, untereinander liegen **Kanäle**, in den Kanälen liegen **Clips**.

Der Name ist **Composer**, nicht Timeline (Vorgabe Patrik S93). „Timeline"
heißt hier nur noch die Zeitachse im Panel. Der Begriff steht schon seit S51 in
zwei Konzepten als Fernziel; dieses Dokument löst ihn ein.

Was er können soll (Patrik S84, S88, S93):

- **Visual-Kanäle sind die Hauptsache** — sie sind der Mischer der Visuals.
  Mindestens zwei, weitere lassen sich hinzufügen.
- Ein **Clip** in einem Visual-Kanal ist ein Preset mit **Start und Ende**.
  In jeden leeren Bereich eines Kanals lässt sich ein anderes Preset legen.
- Jeder Clip hat eine **Einblend- und eine Ausblendkurve** und kann auch
  **mittendrin** den Pegel ändern. Liegen Clips in zwei Kanälen übereinander,
  ergibt das einen **Übergang** (einer steigt, einer fällt) oder eine
  **Überlagerung** (beide eine Weile auf Teilpegel).
- **Mischarten:** nicht nur Überblenden, auch Add, Sub, Xor und andere.
- Die Clips lassen sich **an der Zeitachse allein** ausrichten — oder an
  Musik: ein **Audiokanal** nimmt Songs auf, die dann als Bezug dienen, an dem
  man die Visuals abgleicht. Ein zweiter Audiokanal zum Mischen von Songs
  kommt später (größerer Aufwand).
- Der Composer lässt sich **ein- und ausschalten**, wie die Karaoke-Ebene.
  Das entscheidet, **wer die Presets steuert**.
- Ein angelegter Audiokanal lässt sich ebenfalls ein- und ausschalten. Ist er
  an, ist **er** maßgebend und nicht die Playlist.
- Für Vorführungen lässt sich der Composer **sperren** (§8).

Nicht das Ziel: ein Wechsel des Visuals je Song. Das kann der **Host
Switcher** schon (innerhalb einer Kette, beim Songwechsel, nach Zeit oder per
Skript).

---

## 2. Abgrenzung

| Was | Tut | Verhältnis zum Composer |
|---|---|---|
| **Effektkette** | die Komposition **eines** Presets | ein Clip verweist auf eine Kette und läuft in seinem eigenen Renderer |
| **Host-Gruppe, Host Switcher** | Visuals innerhalb einer Kette, mit Crossfade; der Switcher schaltet sie um | bleiben, wie sie sind; eine Datei, die sie enthält, läuft als Clip unverändert. Enthält sie einen Host Switcher, warnt der Composer beim Hinzufügen: dann wechseln zwei Wechsler (Entscheid 1a, S88) |
| **Visual-Playlist** (Konzept S51) | geordnete Folge ganzer Presets mit Auslösern | **entfällt als eigenes Vorhaben** (C4 a): die feste Folge übernimmt der Composer, den Wechsel nach Zeit, Songwechsel oder Zufall der Host Switcher |
| **Karaoke-Ebene** | Songtext über allem Laufenden | bleibt darüber; bei laufendem Composer zeichnet sie der Mischer einmal über das gemischte Bild (§5) |
| **Playlist** (Audio) | Folge von Songs | bleibt; tritt zurück, solange ein Audiokanal des Composers an ist |

---

## 3. Was es schon gibt

| Baustein | Stand | Nutzen für den Composer |
|---|---|---|
| **Mehrere Renderer nebeneinander** | belegt in C0b: bitgleich mit dem Lauf allein | der Renderweg (§5) |
| **Ladewege je Endung** | `.avs`, `.milk`, `.lvfx`, `.lvfx2` | ein Clip ist eine Pfad-Referenz auf eine dieser Dateien |
| **Mischformeln** | der Mischpass der Kette kennt 13 Arten | Vorlage für die Mischarten des Mischers (eigener Code, gleiche Formeln) |
| **Kurvenformen** | fünf Formen am Crossfade der Host-Gruppe (linear, S-Kurve, Ease-In, Ease-Out, exponentiell) | Vorlage für die Pegelkurven |
| **Ziehen** (S84) | Effect Browser, Audio Browser, Playlist geben Datei-URLs ab | Clips und Songs ins Panel ziehen |
| **Lyrics Sync** (S87–S92) | Wellenform eines Titels, Sprungleiste, Positions-Hochrechnung | Zeitachse, Audiokanal |
| **Laufendes Preset** (S93) | sichert und lädt das Hauptfenster (`UI/StartPreset.hpp`) | der Start kennt künftig auch eine Komposition (C7) |
| **Preset-Zeile** (S89) | Dropdown, ★, Load…, Save, Save As… | Vorbild für die Datei-Zeile des Panels |
| **Sperre von Kürzeln** (S92) | `notWhileTyping` in der Kürzel-Tabelle | Vorbild für die Sperre |

---

## 4. Die Uhr

Der Composer hat **eine Zeitachse** in Millisekunden ab Null.

- **Ohne Audiokanal, oder mit ausgeschaltetem** (C2 a): der Composer hat einen
  **eigenen Transport** — Start, Stopp, an den Anfang, Sprung per Klick ins
  Zeitlineal. Die Zeit läuft nach der Wanduhr, unabhängig von der Musik. Die
  Playlist spielt daneben, was sie will. Der Schalter **Follow player**
  koppelt nur Start und Stopp: startet oder pausiert der Player, tut der
  Composer dasselbe.
- **Audiokanal an:** die Zeit ist die Stelle im Programm, die gerade zu hören
  ist. Der Composer führt den Player: er lädt die Songs der Audio-Clips zu
  ihren Startzeiten und spielt sie. Die Playlist ruht.

In beiden Fällen gilt: **die Stelle auf der Zeitachse bestimmt das Bild**,
nicht die verstrichene Zeit. Pegel werden aus der Stelle berechnet — nach
einem Sprung stimmt der Pegel sofort. Was sich nicht berechnen lässt, ist der
**Zustand** eines Visuals (Feedback, Skript-Variablen): ein Clip beginnt beim
Betreten frisch, auch nach einem Sprung mitten hinein (§10).

---

## 5. Der Renderweg — der eigene Mischer

**Entscheid C3 (Patrik S93): der eigene Mischer.** Belegt in C0b (§9b).

- **Je laufendem Clip ein eigener Renderer.** Jede Datei läuft darin
  unverändert, mit allem, was sie enthält — auch Host-Gruppen und Host
  Switcher. Der kalibrierte Renderer rechnet jedes Bild wie immer.
- **Darüber der Mischer** — eigener Code des Composers. Er nimmt die fertigen
  Bilder der laufenden Clips und führt sie zusammen: von unten nach oben in
  Kanalreihenfolge, jeder Clip mit seinem **Pegel** (aus der Kurve an der
  Stelle der Zeitachse) und seiner **Mischart**.
- **Der Grund ist Schwarz** (C9 a). Liegt an einer Stelle in keinem Kanal ein
  Clip, ist das Bild schwarz.
- **Die Karaoke-Ebene** zeichnet der Mischer **einmal** über das gemischte
  Bild. Die Renderer der Clips lassen sie aus (C0b: sonst zeichnet sie jeder
  selbst).
- **Vorwärmen:** der Composer weiß, was kommt. Er legt den Renderer eines
  Clips kurz vor dessen Start an und lässt ihn sein erstes Bild rechnen,
  damit der Anlauf nicht als Ruck zu sehen ist.
- **Entladen:** endet ein Clip, wird sein Renderer weggeworfen.

Was vor C1 nicht belegt ist und dort **zuerst** geprüft wird (§9, C1a):

1. das fertige Bild eines Renderers als Textur für den Mischer (heute gibt es
   nur einen Testzugang, der ein Bild kopiert);
2. die Karaoke-Ebene je Renderer auslassen;
3. der Betrieb im Render-Thread der App und was am Ende aufs Fenster kommt
   (Render Scale, Post-Bloom macht heute der einzelne Renderer);
4. Bildspeicher je Renderer und die Zeiten im Release-Build;
5. MilkDrop-Presets nebeneinander (C0b hat AVS und native Ketten belegt).

Der andere Weg — die Komposition als eine Kette aus Host-Gruppen — ist in C0
untersucht und verworfen: er braucht drei Eingriffe in die Bildberechnung,
und Dateien mit Host Switcher bleiben darin schwarz (§9a).

---

## 6. Datenmodell und Datei

Eine Komposition ist eine eigene Datei, **`.lvcomp`** (JSON, wie `.lvfx`).
Sie enthält **Pfad-Referenzen**, keine eingebetteten Presets (Entscheid Patrik
S60, übernommen); fehlende Dateien meldet das Laden.

```
Komposition
├── Kanäle (Reihenfolge = Stapelreihenfolge, unten zuerst)
│   ├── Visual-Kanal: Name, an/aus
│   │   └── Clips: Datei, Start, Ende, Mischart,
│   │              Pegelkurve = Punkte (Zeit im Clip, Pegel 0…1, Kurvenform)
│   └── Audio-Kanal: Name, an/aus
│       └── Clips: Datei, Start, Einsatz im Song (später: Lautstärke-Kurve)
└── Länge, Raster (Sekunden; später Takte aus dem Audio)
```

- **Ein- und Ausblenden** sind die ersten und letzten Punkte der Pegelkurve,
  kein eigenes Feld. Ein Clip ohne Punkte läuft auf vollem Pegel.
- Clips eines Kanals **überlappen nicht**; Überlagerung entsteht nur zwischen
  Kanälen.
- Zeiten in Millisekunden. Ein Raster (Einrasten beim Ziehen) ist Bedienung,
  nicht Datenmodell.
- **Nicht in der Datei:** ob der Composer an ist, ob er gesperrt ist, die
  Stelle der Abspielmarke — das sind Einstellungen (`composer/…`).

---

## 7. Das Panel

**View → Panels → Composer**, eigenes andockbares Panel, breit gedacht.

- **Kopf:** Schalter **Composer on**, Schalter **Lock**, Transport (Start,
  Stopp, an den Anfang), Schalter **Follow player**, Stelle als Zeit, Zoom,
  Datei-Zeile (New, Load…, Save, Save As…).
- **Zeitlineal** mit Abspielmarke; Klick springt.
- **Kanäle** als Zeilen, links je Kanal Name und an/aus. Clips als Blöcke:
  ziehen = verschieben, Kanten ziehen = Start und Ende, Entf = löschen.
- **Pegelkurve** im Clip sichtbar; Punkte anfassen und ziehen
  (der „Übergangs-Editor" aus S88).
- **Ziehen hinein:** Presets aus dem Effect Browser in einen Visual-Kanal,
  Songs aus Audio Browser und Playlist in den Audio-Kanal.
- **Audio-Kanal:** die Wellenform des Songs im Clip (wie in Lyrics Sync).
- **Rechtsklick** auf Clip, Kanal und freie Fläche — wie im Kettenbaum.

Offen für C2: was das Panel **Effect Chain** zeigt, solange der Composer an
ist. Vorschlag dort: den Clip, der im Composer angeklickt ist.

---

## 8. Wer steuert was

| Zustand | Presets | Musik |
|---|---|---|
| **Composer aus** | wie heute: Browser, Preset-Zeile, PgUp/PgDn, Host Switcher | Playlist |
| **Composer an, kein Audiokanal an** | der Composer | Playlist |
| **Composer an, Audiokanal an** | der Composer | der Composer; die Playlist ruht |

**Kürzel bei laufendem Composer** (C5 a):

- **PgUp / PgDn** springen zum vorigen bzw. nächsten Clip-Anfang auf der
  Zeitachse, über alle Visual-Kanäle.
- **Die Lied-Kürzel** springen bei aktivem Audiokanal zum vorigen bzw.
  nächsten Song der Komposition; ohne aktiven Audiokanal wirken sie wie heute
  auf die Playlist.

**Ein Preset von Hand laden** (C6 a) — Doppelklick im Effect Browser, Wahl in
der Preset-Zeile, File → Load, Import, „+ New empty effect chain": Rückfrage
„The Composer is running. Stop it and load this preset?". Bei Ja hält der
Composer an und wird ausgeschaltet, das Preset läuft wie gewohnt; die
Komposition bleibt im Panel, **Composer on** bringt sie zurück. Bei Nein
passiert nichts.

**Lock** (Ergänzung Patrik zu C5 und C6, ein Schalter für beides): Ist der
Composer gesperrt,

- tun PgUp / PgDn und die Lied-Kürzel nichts;
- laden die Ladewege nichts, und es erscheint **keine Rückfrage** auf dem
  Bildschirm;
- sagt die Statuszeile, warum: „Composer is locked".

Die Sperre ist für Vorführungen gedacht: nichts, was man versehentlich
drückt, darf das Bild umreißen oder ein Fenster aufgehen lassen.

**Der Start** (C7 a): War beim Beenden der Composer an, lädt der Start die
**Komposition** wieder — Composer an, an den **Anfang** gestellt,
**angehalten**. Zu sehen ist das Bild der Stelle null. Losgelassen wird von
Hand. War er aus, lädt der Start wie seit S93 das zuletzt laufende Preset.

---

## 9. Stufen

Jede Stufe endet mit etwas Prüfbarem. Für jede gelten die Regeln zum Rückbau
(§12): Eigenes in eigenen Dateien, Bestehendes nur an benannten Nahtstellen,
„Composer aus" belegt wie vorher, Rückbau-Liste nachgeführt.

| Stufe | Inhalt | Prüfbar an |
|---|---|---|
| **C0 Belege** ✅ | Renderweg über Host-Gruppen untersucht | erledigt (S93), §9a — verworfen |
| **C0b Belege** ✅ | mehrere Renderer nebeneinander | erledigt (S93), §9b — trägt |
| **C1a Mischer, kleinste Fassung** | zwei Renderer, ein Mischpass mit Pegel und Mischart, Karaoke einmal darüber; die fünf offenen Punkte aus §5 belegt; Nähte R1 und R2 | Test: zwei Presets auf Teilpegel in jeder Mischart, Bild wie berechnet; Zahlen zu Speicher und Zeit im Release-Build |
| **C1b Modell und Wiedergabe** | Komposition als Datenmodell, `.lvcomp` lesen und schreiben, Uhr mit Transport, Pegel aus der Kurve, Vorwärmen und Entladen; **ohne Panel** | Test: an Stelle *t* zeigt das Bild die erwarteten Clips mit den erwarteten Pegeln; nach einem Sprung ebenso |
| **C2 Panel zum Ansehen** | Panel mit Lineal, Kanälen, Clips, Abspielmarke, Transport, Schalter Composer on; der Composer als Bildquelle der App | eine von Hand geschriebene `.lvcomp` läuft und ist zu sehen |
| **C3 Bearbeiten** | Clips hineinziehen, verschieben, Kanten ziehen, löschen; Kanäle anlegen; Datei-Zeile; Undo | eine Komposition ganz im Panel bauen |
| **C4 Pegelkurven** | Punkte im Clip, Kurvenformen, Mischart je Clip | Übergang und Überlagerung aus der Skizze S88 |
| **C5 Audiokanal** | ein Audiokanal: Songs als Clips, Wellenform, führt den Player, Playlist ruht | Visuals an einem Song ausrichten |
| **C6 Steuerung** | Kürzel, Laden bei laufendem Composer, Lock, Start-Wiederherstellung | §8 im Programm |
| später | zweiter Audiokanal (Songs mischen), Takt-Raster, Karaoke-Schalter als Spur, Kompositionen als Video rendern, Parameter über der Zeit | — |

**Der Rückbau wird einmal wirklich durchgespielt, nach C1a** — der ersten
Stufe mit Nähten (§12.4).

### 9a. Stand C0 — ✅ erledigt (S93): Host-Gruppen, verworfen

`tests/unit/UnitTests/test_ComposerBelege.cpp`, Suite `ComposerBelege`,
vier Testfälle. Kein bestehender Code berührt. Die Tests bleiben stehen: sie
halten fest, was die Host-Gruppen können und was nicht.

| Frage | Ergebnis | Messwert |
|---|---|---|
| Laufen zwei Gruppen zugleich? | ja, auch drei; sie stapeln sich in Kettenreihenfolge | Rot + Grün + Blau additiv je 0,5 → 0,502 / 0,502 / 0,502 |
| Geht ein Teilpegel? | ja, über „Adjustable" und `outAdjustAlpha`, auch je Bild per Skript | 128 → 0,498 / 0,502; per Skript 64 → 0,749 / 0,251 |
| Wirkt das Gewicht als Pegel über dem Darunterliegenden? | ja | Blau blendet über stehendem Rot ein, halbe Zeit → Blau 0,494, Rot von 0,502 auf 0,255 |
| Ist der Wechsel A → B ein exakter Mix? | ja | halbe Zeit → 0,498 / 0,502 |
| Lässt sich das Gewicht setzen? | **nein** — nur zeitgeführt | — |
| Nimmt eine andere Mischart einen Pegel an? | **nein** | Additive, Pegel 64 → Blau 1,000 statt 0,25; im Übergang Replace und Additive bildgleich |
| Was kostet ein ruhender Clip? | Lesen, sonst nichts | 6–12 ms je Preset; 8 und 32 ruhende Clips: 0 Laufzeiten, 0 ms je Bild |
| Was kostet der Anlauf? | das Kompilieren im ersten Bild | 110–240 ms (Testing-Build, 640 × 640) |
| Wird nach dem Ende entladen? | **nein** | 8 Clips nacheinander: 19 → 174 Laufzeiten |
| `.lvfx2` mit Host Switcher als Clip? | **schwarz**, ohne Meldung | 0 Host-Gruppen, 2 Listen, 1 Switcher; allein geladen dagegen richtig (0,251) |

**Warum verworfen:** drei Eingriffe in die Bildberechnung (Gewicht setzen,
Mischart mit Pegel, Entladen), und Mix-Dateien mit Host Switcher wären als
Clip unbrauchbar.

### 9b. Stand C0b — ✅ erledigt (S93): mehrere Renderer nebeneinander

Frage Patrik: wäre der eigene Mischer die beste Lösung? Dieselbe Datei,
Suite `ComposerBelegeB`, vier Testfälle, kein bestehender Code berührt.

| Frage | Ergebnis | Messwert |
|---|---|---|
| Zeigt jeder Renderer dasselbe wie allein? | **ja, bitgleich** | zwei Ketten mit eigenem Zustand (Feedback): 0 abweichende Pixel; drei Paare mitgelieferter AVS-Presets (Skripte, Zufall, Feedback), 40 Bilder: 0 von 25 600 |
| Läuft eine Mix-Datei mit Host Switcher? | **ja**, im eigenen Renderer unverändert | Switcher steht auf Grau (0,251), der Renderer daneben zeigt sein Rot; der Songwechsel schaltet weiter auf Hell |
| Was ist geteilt? | Songwechsel, Abspielposition, Songtext, Karaoke-Schalter | der Songwechsel erreicht jeden Renderer — für die Musik richtig |
| Karaoke-Ebene? | **jeder Renderer zeichnet sie selbst** | zwei Renderer, zweimal 5411 Pixel Songtext |
| Was kostet ein weiterer Renderer? | wenig: die Einrichtung fällt nur beim ersten an | `initialize` 114–127 ms beim ersten, 1 ms bei jedem weiteren; erstes Bild 10–205 ms je nach Preset (Testing-Build) |

**Nicht gemessen:** Bildspeicher je Renderer, Zeiten im Release-Build und
auf der schnellen GPU, das Mischen selbst, MilkDrop-Presets nebeneinander,
der Betrieb mit dem Render-Thread der App — alles Stoff von C1a.

---

## 10. Risiken

- **Render-Thread und Fenster.** Die App rendert in einem eigenen Thread mit
  genau einem Renderer, der auch das Fenster bedient. Mehrere Renderer plus
  Mischer in diesen Ablauf zu setzen, ist die größte Unbekannte — darum steht
  sie am Anfang (C1a).
- **Speicher.** Jeder laufende Renderer hält seine eigenen Bilder. Zwei bis
  drei zugleich sind der Normalfall; das Vorwärmen legt kurz einen weiteren
  an.
- **Zwei schwere Visuals zugleich.** Eine Überlagerung rechnet beide voll.
  Einen Rückfall bei einbrechender Bildrate gibt es im Mischer zunächst nicht.
- **Springen.** Visuals haben Zustand; nach einem Sprung mitten in einen Clip
  sieht er anders aus als beim Durchlauf. Verwandt: die Idee Preset-Warmup.
- **Vorwärmen im Lauf.** Ein Preset zu kompilieren kostet Zeit; geschieht es,
  während ein anderes läuft, kann das laufende ruckeln.
- **Was am Fenster ankommt.** Render Scale und Post-Bloom gehören heute dem
  einzelnen Renderer. Bei mehreren ist zu klären, ob sie je Clip oder einmal
  am Ende wirken.
- **Zweiter Audiokanal.** Die Audio-Engine führt heute einen Stream. Zwei
  Songs zu mischen heißt zwei Streams oder ein Mixer — und die Frage, wessen
  Spektrum die Visuals sehen.
- **Größe des Panels.** Eine Zeitachse mit Clips, Kurven und Wellenform ist
  das größte Bedienelement der App; Schritt für Schritt (C2–C4).

---

## 11. Entscheide

Alle gefallen (Patrik, Session 93). Die verworfenen Möglichkeiten stehen
dabei, damit die Gründe nachlesbar bleiben.

| | Frage | Entschieden | Verworfen |
|---|---|---|---|
| — | Name | **Composer**; „Timeline" ist nur die Zeitachse im Panel | Timeline |
| C2 | Die Uhr ohne Audiokanal | **eigener Transport** nach der Wanduhr; Schalter Follow player koppelt Start und Stopp | Stelle im laufenden Song als Uhr; beides umschaltbar |
| C3 | Der Renderweg | **eigener Mischer** über je einem Renderer je laufendem Clip | Komposition als Kette aus Host-Gruppen (C0) |
| C4 | Visual-Playlist (S51) | **entfällt als eigenes Vorhaben**; Konzept bleibt als Quelle mit Vermerk, Punkt P2 geschlossen | vor dem Composer bauen; offen stehen lassen |
| C5 | PgUp/PgDn, Lied-Kürzel | **springen im Composer** (Clip-Anfang bzw. Song der Komposition); sperrbar | immer gesperrt; wie heute mit Ausschalten des Composers |
| C6 | Preset von Hand laden | **schaltet den Composer aus, nach Rückfrage**; gesperrt lädt nichts und fragt nichts | immer gesperrt; als Clip einfügen; ohne Rückfrage |
| — | Lock | **ein** Schalter für Kürzel und Laden | zwei getrennte Schalter |
| C7 | Der Start | lädt die **Komposition**, Composer an, am **Anfang**, **angehalten** | läuft an der letzten Stelle weiter; lädt ein einzelnes Preset; letzte Stelle angehalten |
| C9 | Die Lücke | **schwarz** | letztes Bild; durchscheinendes Preset; Grund-Visual als eigenes Feld |
| C10 | Wo der Composer entsteht | **Zweig `composer`**, nach jeder abgenommenen Stufe nach `master` geholt | auf `master` |
| C11 | Dateien mit Host-Gruppen als Clip | **entfällt** — mit dem eigenen Mischer läuft jede Datei unverändert | — |

**Früher entschieden und übernommen:** Pfad-Referenzen statt eingebetteter
Presets (S60) · bei einer Datei mit Host Switcher warnt der Composer beim
Hinzufügen (S88, 1a) · die Karaoke-Ebene bleibt über allem (S88, 2a).

**Zum Zweig (Stand S93):** C0, C0b und dieses Konzept liegen auf `master` —
es sind nur eine Testdatei und Doku. Der Zweig beginnt mit C1a.

---

## 12. Rückbau

**Vorgabe Patrik (S93):** Der Composer muss sich mit einem Rollback wieder
ausbauen lassen, falls etwas schiefgeht. Das ist keine Notiz für den
Ernstfall, sondern eine Bauregel für jede Stufe.

### 12.1 Regeln

1. **Eigenes liegt in eigenen Dateien.** Modell, Datei-Format, Uhr, Mischer,
   Panel und Tests des Composers stehen in neuen Dateien unter eigenen Namen
   (`composer/…`, `ComposerPanel`, `test_Composer*.cpp`). Rückbau heißt hier:
   Dateien löschen, Einträge aus den `Source.cmake` nehmen.
2. **Bestehendes wird nur an benannten Nahtstellen berührt** (12.2). Jede
   Naht ist klein, trägt im Code den Vermerk `Composer-Naht` und steht in der
   Rückbau-Liste (12.4).
3. **Aus ist wie vorher.** Mit ausgeschaltetem Composer — und das ist die
   Vorgabe — läuft jeder Weg wie vor dem Composer. Neue Felder an
   bestehenden Strukturen sind optional und haben als Vorgabe das alte
   Verhalten. Belegt wird das je Stufe, nicht behauptet (12.3).
4. **Kein bestehendes Dateiformat ändert sich.** `.lvfx`, `.lvfx2`, `.lvkar`,
   `.lvtrail` bleiben, wie sie sind; der Composer schreibt nur `.lvcomp`.
   Einstellungen liegen ausschließlich unter `composer/…`. Nach einem Rückbau
   bleiben höchstens Dateien und Schlüssel zurück, die der alte Code nicht
   kennt und nicht liest.
5. **Naht und Eigenes werden getrennt committet.** Ein Commit ändert
   entweder Bestehendes (Naht, klein, im Betreff als `composer-naht`
   gekennzeichnet) oder fügt Eigenes hinzu — nie beides. Der Rückbau ist dann:
   die Naht-Commits zurücknehmen, die eigenen Dateien löschen.
6. **Eine Stufe ist erst fertig, wenn ihr Rückbau aufgeschrieben ist.** Die
   Rückbau-Liste wird mit jeder Stufe nachgeführt.

### 12.2 Die Nahtstellen (Stand nach den Entscheiden)

Die Bildberechnung des Renderers wird **nicht** berührt.

| Naht | Wo | Was sich ändert | Vorgabe = altes Verhalten | Stufe |
|---|---|---|---|---|
| R1 Bild herausgeben | Renderer | das fertige Bild als Textur lesbar (heute nur ein Testzugang, der kopiert) | reiner Lesezugang, ändert nichts | C1a |
| R2 Karaoke auslassen | Renderer | ein Renderer kann die Karaoke-Ebene auslassen | Vorgabe: zeichnet sie wie heute | C1a |
| R3 Bildquelle | Fenster / Render-Thread | der Composer tritt als Bildquelle an die Stelle des einen Renderers | Composer aus → der eine Renderer wie heute | C2 |
| N4 Laufendes Preset, Start | `MainWindow`, `UI/StartPreset.hpp` | der Start kennt eine Komposition | ohne Schlüssel `composer/…` wie heute | C6 |
| N5 Kürzel | Empfänger von `PresetStepEvent` und der Lied-Kürzel | bei laufendem Composer geht das Ereignis an ihn; Lock | Composer aus → Effect Browser bzw. Playlist | C6 |
| N6 Laden von Hand | `MainWindow` (gemeinsame Rückfrage vor dem Ersetzen der Kette) | Rückfrage bzw. Sperre bei laufendem Composer | Composer aus → wie heute | C6 |
| N7 Musik | Player / Playlist | der Audiokanal führt den Player | Audiokanal aus → Playlist | C5 |
| N8 Anmeldung | Panel-Registrierung, Menü, `Source.cmake`, Dateityp-Icon | Einträge | — (reine Zusätze) | C2 |

Kommt eine Naht dazu, kommt sie in diese Tabelle, bevor sie gebaut wird.

### 12.3 Der Beleg je Stufe

- **Unit-Tests** grün, 0 Skips — die bestehenden unverändert.
- **Vergleichslauf** nach jeder Stufe, die den Renderer berührt (R1, R2, R3):
  bitgleich gegen den Bezugsstand, mit ausgeschaltetem Composer.
- **Ein eigener Test „Composer aus"** je Naht: derselbe Aufruf mit und ohne
  die Naht liefert dasselbe Ergebnis, solange der Composer nicht eingreift.

### 12.4 Die Rückbau-Liste

| Stufe | Eigene Dateien (löschen) | Naht-Commits (zurücknehmen) | Einträge (herausnehmen) | Rückbau geprüft |
|---|---|---|---|---|
| C0, C0b | `tests/unit/UnitTests/test_ComposerBelege.cpp` | keine | eine Zeile in `tests/unit/UnitTests/Source.cmake` | nicht nötig — nur Tests, kein bestehender Code berührt |

„Rückbau geprüft" heißt: der Stand nach dem Rückbau baut, die Unit-Tests
sind grün und der Vergleichslauf ist bitgleich. Geprüft wird das **einmal
nach C1a** und danach, wenn eine neue Naht dazukommt.

---

## 13. Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 0.5.0 | 2026-10-04 | Session 93 — **alle Entscheide gefallen** (Patrik): C2 a eigener Transport, C3 eigener Mischer, C4 a Visual-Playlist entfällt, C5 a Kürzel springen im Composer, C6 a Laden schaltet nach Rückfrage aus, dazu **ein Schalter Lock** für Kürzel und Laden (keine Rückfrage bei Vorführungen), C7 a Start lädt die Komposition angehalten am Anfang, C9 a Lücke schwarz, C11 entfällt. §5 neu gefasst (Mischer, Vorwärmen, Entladen, Karaoke einmal durch den Mischer, fünf offene Punkte für C1a); §8 mit den Regeln; Stufen C1a / C1b; Nahtstellen neu (R1–R3 am Rand des Renderers, keine in der Bildberechnung) |
| 0.4.0 | 2026-10-04 | Session 93 — **Stufe C0b erledigt** (Frage Patrik: ist der eigene Mischer die beste Lösung?): §9b — zwei Renderer nebeneinander sind bitgleich mit dem Lauf allein, eine Mix-Datei mit Host Switcher läuft darin unverändert, ein weiterer Renderer kostet wenig, die Karaoke-Ebene zeichnet jeder selbst; Empfehlung C3 und C11 gedreht |
| 0.3.0 | 2026-10-04 | Session 93 — **Stufe C0 erledigt:** §9a mit den Messwerten; Gewicht wirkt als Pegel; Mischart kennt keinen Pegel; ruhende Clips kosten nichts, dafür Anlauf-Ruck und fehlendes Entladen; `.lvfx2` mit Switcher als Clip schwarz; Entscheid C11; Rückbau-Liste C0 |
| 0.2.0 | 2026-10-04 | Session 93 — Vorgabe Patrik: rückbaubar. Neu §12 Rückbau (Regeln, Nahtstellen, Beleg je Stufe, Rückbau-Liste), Entscheid C10 (Zweig) |
| 0.1.0 | 2026-10-03 | Session 93 — Erstfassung aus den Notizen S84, S88, S93 und Patriks Vorgaben vom 03.10.: Name Composer, Visual-Kanäle als Mischer, Audiokanal als Bezug, ein- und ausschaltbar; Stufen; Entscheide |
