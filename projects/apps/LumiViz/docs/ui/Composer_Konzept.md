# LumiViz — Composer (Konzept)

> **Version:** 0.9.1
> **Datum:** 2026-10-04 (Session 94)
> **Typ:** Konzept — **gebilligt, alle Entscheide gefallen** (Patrik S93: C2 a,
> C3 eigener Mischer, C4 a, C5 a, C6 a mit Sperre, C7 a, C9 a, C10 a);
> Stufen **C0, C0b (Belege), C1a (Mischer, Nähte R1 / R2) und C1b (Modell,
> Datei, Uhr, Wiedergabe ohne Panel) erledigt** —
> im Zweig `composer`, noch nicht nach `master` geholt
> **Status:** Entwurf
> **Vorgabe:** rückbaubar (§12); entsteht ab C1 im Zweig `composer`, der
> erst nach `master` geholt wird, wenn der Composer sauber läuft
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
| **C1a Mischer, kleinste Fassung** ✅ | zwei Renderer, ein Mischpass mit Pegel und Mischart, Karaoke einmal darüber; Nähte R1 und R2 | erledigt (S93), §9c. Von den fünf offenen Punkten aus §5 sind drei belegt; Render-Thread / Fenster und die Zeiten im Release-Build gehen nach C2 |
| **C1b Modell und Wiedergabe** ✅ | Komposition als Datenmodell, `.lvcomp` lesen und schreiben, Uhr mit Transport, Pegel aus der Kurve, Vorwärmen und Entladen; **ohne Panel** | erledigt (S93), §9d |
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

### 9c. Stand C1a — ✅ erledigt (S93): der Mischer in kleinster Fassung

**Eigenes:** `include/composer/Mischer.hpp`, `src/composer/Mischer.cpp`
(Klassen `Leinwand` und `Mischer`, sieben Mischarten: Normal, Additive,
Subtract, Maximum, Minimum, Multiply, XOR — jede mit Pegel:
`mix(unten, art(unten, oben), pegel)`), `src/composer/Source.cmake`,
`tests/unit/UnitTests/test_ComposerMischer.cpp` (Suite `ComposerMischer`,
sieben Testfälle).

**Nähte am Renderer** (`MultiEffectVisualizer`, im Code als `Composer-Naht`
vermerkt; die Bildberechnung ist nicht berührt):

- **R1 Present-Ziel** — `setPresentZiel(framebuffer)`. Vorgabe 0 = das
  Fenster wie immer. Der Composer lässt den Renderer sein **fertiges** Bild
  in ein eigenes Framebuffer-Objekt legen, samt Render Scale und
  Post-Bloom, in Fenstergröße. Im Code: eine Zeile im Present
  (`glBindFramebuffer(…, m_presentZiel)` statt `…, 0`). Das ist besser als
  der zuerst gedachte Lesezugang auf das innere Bild: der Clip sieht im
  Mischer genau so aus, wie er allein am Fenster stünde.
- **R2 Karaoke** — `setKaraokeAuslassen(bool)` (Vorgabe: nein) und
  `legeKaraokeEbeneAuf()`: legt die in diesem Bild gerechnete Ebene auf den
  gebundenen Framebuffer; der Renderer ruft dasselbe am Ende seines
  Presents. Im Code: eine Abfrage am Anfang von `rechneKaraokeEbene` und
  eine öffentliche Weiterleitung.

| Frage | Ergebnis | Messwert |
|---|---|---|
| Rechnet der Mischer jede Mischart mit jedem Pegel nach der Formel? | ja | 7 Arten × Pegel 0,25 / 0,5 / 1, je Kanal höchstens 1 von 255 daneben |
| Grund, Pegel 0, Reihenfolge? | Schwarz; Pegel 0 legt nichts; das Obere gewinnt | — |
| Bleibt jeder Renderer im Mischer bitgleich mit dem Lauf allein? | ja | zwei Ketten mit Zustand, 30 Bilder, 0 Pixel; das Gemischte ist Pixel für Pixel das Mittel |
| Trägt die Leinwand das fertige Bild in Fenstergröße? | ja, auch mit Render Scale | Kette 64 × 64, Leinwand 128 × 128 |
| Karaoke einmal über dem Gemischten? | ja | Clips mit „auslassen": kein Text; der Mischer legt die Ebene eines Träger-Renderers darüber — 8671 Pixel, bildgleich mit dem Renderer allein |
| MilkDrop nebeneinander? | **bitgleich** | zwei Paare mitgelieferter `.milk`, 30 Bilder, 0 von 19 200 Pixeln |
| Was kostet das Mischen? | fast nichts | 0,29 ms je Bild bei 1280 × 720 (zwei Durchgänge + Ausgabe, Testing-Build) |
| Bildspeicher? | gerechnet, nicht gemessen | bei 1280 × 720 je Renderer mindestens 10,5 MB (Bildpaar + Leinwand), der Mischer 7 MB; dazu kommt, was das Preset selbst anlegt |

**Composer aus = wie vorher, belegt:**

- Unit-Tests **858 / 858 grün, 0 Skips** — die 851 bestehenden unverändert.
- **Vergleichslauf bitgleich:** 90 von 90 Bildern gegen den Bezugsstand
  `out/f4c/bp30_s91_trailline` (15 Presets, `--beat-period 30`), neuer
  Stand `out/f4c/bp30_s93_c1a`.

**Nicht belegt, geht nach C2:**

- der Betrieb im **Render-Thread** der App und am **Fenster** — der
  Composer ist noch keine Bildquelle (Naht R3);
- die Zeiten im **Release-Build** und auf der schnellen GPU (im Test:
  Testing-Build; die Bildzeiten dort sagen nichts über den Betrieb);
- der Bildspeicher gemessen statt gerechnet;
- dass der Present mit der Vorgabe wirklich am Fenster ankommt, ist im
  Unit-Test nicht lesbar (unsichtbare Fläche ohne feste Größe) — das
  belegt der Vergleichslauf, der durch ein echtes Fenster geht.

**Offen für später:** „Die Effektkette gewinnt" über mehrere Clips — zeigt
ein Clip selbst Lyrics, tritt die Ebene heute nur im **selben** Renderer
zurück. Der Mischer muss das über alle laufenden Clips entscheiden.

### 9d. Stand C1b — ✅ erledigt (S93): Modell, Datei, Uhr, Wiedergabe

Nur eigener Code, **keine Naht**, kein bestehender Code berührt.

| Datei | Inhalt |
|---|---|
| `composer/Komposition.hpp` / `.cpp` | Datenmodell (`Komposition`, `Kanal`, `VisualClip`, `AudioClip`, `PegelPunkt`), sechs Kurvenformen (Linear, S-Kurve, Ease-In, Ease-Out, Exponentiell, Halten), `pegelBei`, `setzeBlenden`, `richte`; Datei `.lvcomp` (`speichereKomposition`, `ladeKomposition`) |
| `composer/Uhr.hpp` | der Transport nach C2 a: `starte`, `stoppe`, `springe`, `anDenAnfang`, Länge; Zeitquelle austauschbar |
| `composer/Wiedergabe.hpp` / `.cpp` | `rendere(stelle, delta, größe)`: Renderer je Clip anlegen, vorwärmen, entladen, mischen, Karaoke einmal darüber |
| `tests/…/test_ComposerWiedergabe.cpp` | Suite `ComposerWiedergabe`, acht Testfälle |

**Die Datei `.lvcomp`:** JSON mit `format: "lumiviz-composition"`,
`version`, `length`, `gridSeconds`, `channels[]` (`kind`, `name`,
`enabled`, `clips[]`). Ein Visual-Clip: `file`, `start`, `end`, `blend`,
`levels[]` (`t`, `level`, `curve`). Ein Audio-Clip: `file`, `start`,
`offset`. Dateien unterhalb des Ordners der `.lvcomp` stehen **relativ**
darin, alle anderen absolut.

| Frage | Ergebnis |
|---|---|
| Zeigt das Bild an jeder Stelle die erwarteten Clips mit den erwarteten Pegeln? | ja — neun Stellen einer Komposition mit Einblenden, Ausblenden, Überlagerung (additiv auf halbem Pegel, dann steigend), Lücke und Ende; je Kanal höchstens 2 von 255 daneben |
| Auch nach einem Sprung? | ja — dieselben Stellen in anderer Reihenfolge, dasselbe Bild |
| Lücke? | schwarz (C9 a); das Ende eines Clips ist ausschließlich |
| Ausgeschalteter Kanal? | fällt weg |
| Vorwärmen, Entladen? | Renderer gibt es nur für Clips, die laufen oder im Vorlauf (Vorgabe 2 s) sind; im Vorlauf angelegt, aber nicht gemischt; nach dem Ende und nach einem Sprung weg |
| Verstellt das Vorwärmen den Clip? | nein — das Vorwärm-Bild rechnet mit Delta 0; ein vorgewärmter Clip ist nach 30 Bildern bitgleich mit dem Lauf allein (0 von 19 200 Pixeln). Belegt an einem Preset mit Feedback; für Presets, die Bilder zählen statt Zeit zu messen, nicht belegt |
| Ein Clip auf vollem Pegel? | bitgleich mit dem Renderer allein |
| Fehlende Datei? | gemeldet (`bericht()`), der Clip bleibt schwarz, kein neuer Versuch in jedem Bild |
| Karaoke? | einmal über dem Gemischten — über zwei Clips so viele Pixel wie bei einem Renderer allein (8671); auch in der Lücke; Ebene aus = sofort weg |
| `.lvcomp` | Roundtrip aller Felder; relative und absolute Pfade; fehlende Datei gemeldet, Clip bleibt; fremdes JSON wird abgelehnt |
| Uhr | steht, bis sie gestartet wird; Stopp hält die Stelle; Sprung im Stand und im Lauf; hält am Ende, ein neuer Start beginnt von vorn |

Unit-Tests **866 / 866 grün, 0 Skips**. Kein Vergleichslauf nötig: der
Renderer ist in C1b nicht berührt.

**Nicht gebaut, bewusst:** Audio-Clips stehen im Modell und in der Datei,
werden aber noch nicht gespielt (C5). Die Renderer der Clips bekommen noch
keine Audiodaten — das reicht die Bildquelle in C2 durch
(`fuerJedenRenderer`). Die Warnung bei einer Datei mit Host Switcher kommt
mit dem Hinzufügen von Clips im Panel (C3).

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
| C10 | Wo der Composer entsteht | **Zweig `composer`**; nach `master` geholt wird **erst, wenn der Composer sauber funktioniert** (Patrik S93, geändert gegenüber „nach jeder abgenommenen Stufe"). Bis dahin läuft **alle** Arbeit im Zweig, auch Abschlüsse und Fixes | auf `master`; nach jeder Stufe holen |
| C11 | Dateien mit Host-Gruppen als Clip | **entfällt** — mit dem eigenen Mischer läuft jede Datei unverändert | — |

**Früher entschieden und übernommen:** Pfad-Referenzen statt eingebetteter
Presets (S60) · bei einer Datei mit Host Switcher warnt der Composer beim
Hinzufügen (S88, 1a) · die Karaoke-Ebene bleibt über allem (S88, 2a).

**Zum Zweig (Stand S93):** C0, C0b und das Konzept bis 0.5.0 liegen auf
`master` — es sind nur eine Testdatei und Doku. Der Zweig beginnt mit C1a.
Seit dem Abschluss S93 (`e14d9c4`) enthält er auch Arbeit, die nicht zum
Composer gehört (Changelog, offene Punkte, künftig Fixes aus Sichttests).
Für den Rückbau heißt das: nicht den Zweig verwerfen, sondern die
Composer-Commits nach §12.4 zurücknehmen.

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
| R1 Present-Ziel ✅ | Renderer, eine Zeile im Present | `setPresentZiel`: der Renderer legt sein fertiges Bild in ein Framebuffer-Objekt des Aufrufers | 0 = das Fenster, wie immer | C1a |
| R2 Karaoke ✅ | Renderer, eine Abfrage und eine Weiterleitung | `setKaraokeAuslassen`; `legeKaraokeEbeneAuf` legt die gerechnete Ebene auf den gebundenen Framebuffer | Vorgabe: zeichnet sie wie heute | C1a |
| R3 Bildquelle ✅ | `VisualizerRenderThread` (`setBildquelle`, Auswahl je Bild, Herunterfahren), `VisualizerWidget` (`setBildquelle` als Durchreichung, Besitz) | der Thread bekommt neben dem einen Renderer einen zweiten Platz für eine Bildquelle (`IVisualizer`). Ist er besetzt, gehen Audiodaten und `render` an die Bildquelle; der Renderer der Kette bleibt angelegt, bekommt weiter Größe und Audiodaten, wird aber nicht gerechnet. Die Bildquelle wird auf dem Render-Thread angelegt, aufgeräumt und gelöscht (Entscheid Patrik S94: a) | Platz leer (Vorgabe) → der eine Renderer wie heute | C2a |
| N4 Laufendes Preset, Start | `MainWindow`, `UI/StartPreset.hpp` | der Start kennt eine Komposition | ohne Schlüssel `composer/…` wie heute | C6 |
| N5 Kürzel | Empfänger von `PresetStepEvent` und der Lied-Kürzel | bei laufendem Composer geht das Ereignis an ihn; Lock | Composer aus → Effect Browser bzw. Playlist | C6 |
| N6 Laden von Hand | `MainWindow` (gemeinsame Rückfrage vor dem Ersetzen der Kette) | Rückfrage bzw. Sperre bei laufendem Composer | Composer aus → wie heute | C6 |
| N7 Musik | Player / Playlist | der Audiokanal führt den Player | Audiokanal aus → Playlist | C5 |
| N8 Anmeldung | Panel-Registrierung, Menü, `Source.cmake`, Dateityp-Icon | Einträge. **C2a (Entscheid Patrik S94: a):** Menü File bekommt **Open composition…** und **Close composition** (`MenuAutoReg.cpp`: eine Gruppe, zwei Einträge); `MainWindow` meldet zwei Ereignisse an und reicht sie an `composer/AppAnbindung` weiter. Open lädt eine `.lvcomp`, setzt die Bildquelle und startet die Uhr sofort; Close nimmt sie weg | — (reine Zusätze); ohne Klick auf die Einträge wie heute | C2a, C2b |

Kommt eine Naht dazu, kommt sie in diese Tabelle, bevor sie gebaut wird.

### 12.3 Der Beleg je Stufe

- **Unit-Tests** grün, 0 Skips — die bestehenden unverändert.
- **Vergleichslauf** nach jeder Stufe, die den Renderer berührt (R1, R2):
  bitgleich gegen den Bezugsstand, mit ausgeschaltetem Composer.
- **R3 belegt der Vergleichslauf nicht** (Befund S94): er läuft über
  `AvsStandalone`, und weder dieses noch die übrigen Unit-Tests benutzen
  `VisualizerRenderThread` oder `VisualizerWidget`. Der Beleg für R3 ist
  `test_ComposerNahtR3.cpp` — der echte Thread an einem echten Fenster mit
  zählenden Visualizern: Platz leer wie immer, Platz besetzt, Wegnehmen,
  Herunterfahren. Die Durchreichung in `VisualizerWidget` belegt der dritte
  Testfall von `test_ComposerBildquelle.cpp` (einschalten, ersetzen,
  ausschalten über die echte Fassade).
- **Ein eigener Test „Composer aus"** je Naht: derselbe Aufruf mit und ohne
  die Naht liefert dasselbe Ergebnis, solange der Composer nicht eingreift.

### 12.4 Die Rückbau-Liste

| Stufe | Eigene Dateien (löschen) | Naht-Commits (zurücknehmen) | Einträge (herausnehmen) | Rückbau geprüft |
|---|---|---|---|---|
| C0, C0b | `tests/unit/UnitTests/test_ComposerBelege.cpp` | keine | eine Zeile in `tests/unit/UnitTests/Source.cmake` | nicht nötig — nur Tests, kein bestehender Code berührt |
| C1a | `include/composer/` (Mischer.hpp), `src/composer/` (Mischer.cpp, Source.cmake), `tests/unit/UnitTests/test_ComposerMischer.cpp` | **`91967c2`** — Naht (`MultiEffectVisualizer.hpp` / `.cpp`) und Eigenes liegen in diesem einen Commit; ihn zurückzunehmen baut C1a ganz zurück | `include(…/composer/Source.cmake)` in `src/Source.cmake`; eine Zeile in `tests/unit/UnitTests/Source.cmake` | ✅ S93: `git diff --stat master composer` zeigt genau diese neun Dateien; an Bestehendem drei Naht-Zeilen in der `.cpp`, 26 Zeilen im Header, je eine Zeile in zwei Quellenlisten |
| C1b | `include/composer/` (Komposition.hpp, Uhr.hpp, Wiedergabe.hpp), `src/composer/` (Komposition.cpp, Wiedergabe.cpp), `tests/unit/UnitTests/test_ComposerWiedergabe.cpp` | keine Naht; der Commit ist **`db3a026`** | zwei Quellen und drei Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis); eine Zeile in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen |
| C2a, Naht R3 | `tests/unit/UnitTests/test_ComposerNahtR3.cpp` | **`f8821db`** — nur Naht: `VisualizerRenderThread.hpp` / `.cpp` (`setBildquelle`, Kommando, Auswahl je Bild, Herunterfahren), `VisualizerWidget.hpp` / `.cpp` (`setBildquelle`, `bildquelle`, `m_bildquelle`); jede Stelle trägt `Composer-Naht R3` | eine Zeile in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen; Pfade des Composers um `UI/widgets/VisualizerRenderThread.*` und `VisualizerWidget.*` erweitert |
| C2a, Bildquelle | `include/composer/Bildquelle.hpp`, `src/composer/Bildquelle.cpp`, `tests/unit/UnitTests/test_ComposerBildquelle.cpp`, `tests/unit/UnitTests/test_ComposerMessung.cpp` (Messlauf, je eine Zeile in der Quellenliste der Tests) | keine Naht | eine Quelle und ein Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis); eine Zeile in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen |
| C2a, Einschalten (N8) | `include/composer/AppAnbindung.hpp`, `src/composer/AppAnbindung.cpp`, `asset/composer_demo.lvcomp`; der dritte Testfall in `test_ComposerBildquelle.cpp` geht mit der Datei | **`a6d4b08`** (eigener Code davor: `14fef86`) — nur Naht: `MenuAutoReg.cpp` (ein Include, eine Gruppe, zwei Einträge), `MainWindow.cpp` (ein Include, zwei Anmeldungen); jede Stelle trägt `Composer-Naht N8`. Der Naht-Commit folgt hier **nach** dem eigenen, weil er dessen Header braucht — beim Rückbau zuerst ihn zurücknehmen | eine Quelle und ein Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis) | vor dem Holen wie oben prüfen; Pfade des Composers um `UI/managers/MenuAutoReg.cpp` und `UI/MainWindow.cpp` erweitert. Zurück bleibt der Schlüssel `composer/lastDir` |

„Rückbau geprüft" heißt: der Stand nach dem Rückbau baut, die Unit-Tests
sind grün und der Vergleichslauf ist bitgleich.

**Wie geprüft wird (Stand S93):** Der Zweig trägt seit dem Abschluss S93
auch Arbeit, die nicht zum Composer gehört — „Zweig verwerfen" ist damit
kein Rückbau mehr, und `git diff --stat master composer` zeigt mehr als
diese Liste. Geprüft wird deshalb über die **Pfade des Composers**:
`include/composer`, `src/composer`, `tests/unit/UnitTests/test_Composer*.cpp`,
`MultiEffectVisualizer.hpp` / `.cpp` und die beiden Quellenlisten. Der
Unterschied zu `master` in diesen Pfaden muss sich mit der Liste decken.
**Wirklich durchgespielt** wird der Rückbau einmal, bevor der Zweig nach
`master` geholt wird: die Composer-Commits auf einer Kopie zurücknehmen,
bauen, Unit-Tests, Vergleichslauf.

---

## 13. Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 0.9.1 | 2026-10-04 | Session 94 — **Sichttest S94 (Patrik): 34 von 42 bestanden, 3 mit Befund, 5 unklar formuliert.** Drei Folgen: (1) **Karaoke lief nicht weiter, wenn die Uhr stand** (A4, B7) — behoben: `Wiedergabe::rendere(…, clipsStehen)` mischt das letzte Bild der Clips neu, ohne sie zu rechnen, die Karaoke-Ebene läuft mit dem echten Takt weiter; die Bildquelle rechnet bei stehender Uhr, solange die Ebene an ist. §5 gilt jetzt als „steht die Uhr, stehen die Clips". (2) **„Pixeliger" (F7) war ein Fehler des Scheiben-Baus:** ein Movement in einer Liste, die nur beim Beat rechnet, bekam eine Scheibe je Beat, das Gitter-Bild blieb viele Sekunden stehen. Behoben: der Renderer baut jede laufende Tabelle je Bild weiter, auch wenn ihr Knoten nicht drankommt. Probe `Duo - Hash the Planet.avs` mit festem Beat: vorher nach 900 Bildern noch im Bau und 0,7 % der Pixel anders, jetzt nach rund 80 Bildern fertig und nach 300 und 900 Bildern bitgleich mit dem Sofort-Betrieb. (3) **Einstellung Settings → Performance → Movement Table** (Vorschlag Patrik): „Spread over frames" (Vorgabe der App) oder „At once (like original AVS)" für Vergleiche mit dem Original; Benutzerhandbuch 1.79.0. Belege: neue Testfälle in `test_ComposerBildquelle.cpp` und `test_MovementScheiben.cpp`, 877 Cases; Vergleichslauf 90 / 90 bitgleich (`bp30_s94_scheiben3`). Aus dem Log des Sichttests: die App rechnete die Demo mit 60 Bildern je Sekunde auf der 4090, Rechnen im Mittel 1,4–2,1 ms; beim Schließen der App mit laufender Komposition wird keine Zusammenfassung geschrieben |
| 0.9.0 | 2026-10-04 | Session 94 — **Auch Movement-Skripte, die den gemeinsamen Zustand lesen, bauen in Scheiben** (Entscheid Patrik: a): im Scheiben-Betrieb entsteht jede Movement-Tabelle über mehrere Bilder, auch mit `reg`, `megabuf`, `rand` oder `get…`; solche Skripte lesen diese Werte dann über mehrere Bilder verteilt statt in einem Augenblick, und ihre Tabelle kommt nicht in den gemeinsamen Speicher. Der Sofort-Betrieb ist unverändert. **Belege:** `test_MovementScheiben.cpp` erweitert (unreines Skript baut in Scheiben, zweimal selbst, nichts im Speicher, Bild gleich dem Sofort-Betrieb bei unverändertem Zustand), 874 Cases; Vergleichslauf 90 / 90 bitgleich (`bp30_s94_scheiben2`). **Messung** (Release-clang, Radeon 610M, 1920 × 1080, Scheiben 4 ms): `Duo - Hash the Planet.avs` zweites Bild 13 ms statt 504 ms; `list.lvfx2` allein kein langes Bild mehr nach dem ersten (längstes 25 ms statt 559 ms); härtere Komposition mit 15 Presets: längstes Bild nach dem ersten 147 ms statt 600–800 ms, tiefste Sekunde 15 Bilder statt 1. **Was bleibt:** das erste Bild einer Komposition 190–300 ms (Anlegen des ersten Renderers rund 100 ms, dazu das Laden); sechs Bilder zwischen 50 und 150 ms in 76 s, zwei davon beim Laden von `list.lvfx2` und eines weiteren Presets; auf der 4090 mit diesem Stand nicht gemessen |
| 0.8.9 | 2026-10-04 | Session 94 — **Ruck bei 37 s eingegrenzt.** Messung um die Liste der langen Bilder ergänzt (jedes Bild nach dem ersten über 50 ms, mit Stelle). `list.lvfx2` allein: ein langes Bild von 450–560 ms beim ersten Gruppenwechsel des Host Switchers (Stelle 5–6 s), mit und ohne Scheiben. Die ersten sieben verwiesenen Presets einzeln gemessen (Release-clang, Radeon 610M, 1920 × 1080): sechs laufen mit Scheiben in 3–41 ms an, auch das MilkDrop-Preset; **`Duo - Hash the Planet.avs` braucht für sein zweites Bild 441–509 ms, bei 960 × 540 nur 127 ms** — die Zeit wächst mit der Pixelzahl, es ist wieder eine Movement-Tabelle. Das Preset enthält ein Movement-Skript mit `gmegabuf(7)`; solche Skripte baut der Scheiben-Betrieb absichtlich sofort (0.8.8). Belegt sind die Skalierung und der Skripttext, nicht eine Messung dieses einen Knotens. Offen: Entscheid, ob auch Skripte, die den gemeinsamen Zustand lesen, in Scheiben gebaut werden |
| 0.8.8 | 2026-10-04 | Session 94 — **Movement-Tabelle in Scheiben** (Entscheid Patrik: a). Kein Teil des Composers, sondern eine Änderung am Renderer: `ScriptGridModule::beginTransTable` / `continueTransTable` bauen dieselbe Tabelle zeilenweise mit Zeitbudget; `MultiEffectVisualizer::setMovementScheibeMs` (Vorgabe 0 = sofort, wie immer — Standalones, Vergleichslauf, Unit-Tests; die App setzt 4 ms, `LUMIVIZ_MOVEMENT_SCHEIBE_MS` stellt um); bis die Tabelle steht, zeichnet der Knoten über das Gitter mit einem eigenen Modul; fertige Tabellen liegen in einem prozessweiten Speicher (Schlüssel Skript, Schalter, Bildgröße; Grenze 256 MB); Skripte, die `reg`, `megabuf`, `rand` oder `get…` enthalten, bauen weiter sofort. **Composer:** die Wiedergabe wärmt einen Clip weiter vor, solange `movementTabelleImBau()`. **Belege:** `test_MovementScheiben.cpp` (Tabelle Eintrag für Eintrag gleich, auch mit Zustand von Pixel zu Pixel; Bild nach dem Bau bitgleich mit dem Sofort-Betrieb; zweiter Renderer aus dem Speicher; unreines Skript sofort), 874 Cases; Vergleichslauf 90 / 90 bitgleich gegen `out/f4c/bp30_s93_c1a` (neuer Stand `bp30_s94_scheiben`). **Messung** (15 Presets, 1920 × 1080, Release-clang): die ersten Bilder der Movement-Presets fallen von 0,8 s auf 11–135 ms; auf der 4090 steigt der Median von 79 auf 94 Bilder je Sekunde, die tiefste Sekunde von 1 auf 6 Bilder. **Nicht gelöst:** ein Bild von 0,6–0,7 s an Stelle 37 s (610M, zweimal) — kein Clip-Anlauf, vermutlich der Gruppenwechsel des Host Switchers in `list.lvfx2`; das Laden von `list.lvfx2` (20 Presetverweise) kostet 110–1100 ms; die Läufe auf der 4090 streuten an diesem Abend stark |
| 0.8.7 | 2026-10-04 | Session 94 — **Ursache des langen ersten Bilds gefunden: der Knoten Movement, nicht MilkDrop.** `runMovement` wertet das Skript je Pixel aus und legt eine Tabelle an, die bei jedem Größen- oder Skriptwechsel neu entsteht (`MultiEffectVisualizer.cpp`, Kommentar an `runMovement`). Belegt mit vier Varianten von `202609_g2_milk.lvfx` (Release-clang, Radeon 610M), erstes Bild bei 1920 × 1080 / 960 × 540: unverändert 873 / 216 ms; beide Movement aus 25 / 25 ms; nur ein Movement an 496 / 122 ms; MilkDrop aus 753 / 191 ms. Die Zeit wächst mit der Pixelzahl (vierfach bei vierfacher Fläche) und liegt bei etwa 0,4–0,5 s je Movement-Knoten in Full HD. Der Befund betrifft nicht nur den Composer: auch das gewöhnliche Laden eines solchen Presets und jede Größenänderung des Fensters rechnen die Tabelle neu. Offen: Entscheid, wie die Tabelle aus dem Bild genommen wird |
| 0.8.6 | 2026-10-04 | Session 94 — **C2a, Anlauf je Clip aufgeschlüsselt:** `Wiedergabe::anlaeufe()` misst je angelegtem Renderer Laden (Datei, Kette, Skripte — ohne GL), Anlegen (`initialize` + `resize`), erstes und zweites Bild; `Bildquelle::anlaufText()`, im Log bei Close Composition und im Messlauf. **Ergebnis (Release-clang, 1920 × 1080, 15 Presets der Regressionsliste, beide Karten):** das Laden kostet 1–10 ms (eine Datei mit 20 Host-Gruppen 110–130 ms), das Anlegen 1–11 ms (einmalig 108 ms beim allerersten Renderer auf der 610M); **der Anlauf steckt im ersten Bild** — bei fünf der 15 Presets 630–820 ms, bei den übrigen 1–145 ms, das zweite Bild liegt danach bei 0–12 ms. Die langen ersten Bilder sind auf der 610M und der 4090 gleich lang (760–810 ms gegen 750–820 ms) — es rechnet also die CPU, nicht die Grafikkarte. **Nicht geklärt:** was im ersten Bild dieser fünf Presets so lange dauert (die Knotentypen unterscheiden sie nicht von den schnellen), und ein einzelnes Bild von 1,4 s an Stelle 43 s auf der 4090, das an keinem Clip-Anlauf liegt |
| 0.8.5 | 2026-10-04 | Session 94 — **C2a, härtere Messung** mit den 15 Presets der Regressionsliste (`regression_s84_presets.txt`): fünf Kanäle, je drei Clips zu 20 s lückenlos, die Kanäle um 4 s versetzt, 76 s, höchstens sechs Renderer zugleich (fünf laufend, einer vorgewärmt); Release-clang, 1920 × 1080. **Radeon 610M:** 41 Bilder je Sekunde im Mittel, Median 26, tiefste Sekunde 1; freier Grafikspeicher fällt von 356 auf 8 MB. **RTX 4090 (zwei Läufe):** 242 und 183 Bilder je Sekunde im Mittel, Median 113 und 92, tiefste Sekunde 2 und 1; 556 und 585 MB Grafikspeicher. **Befund: das Vorwärmen verhindert den Ruck nicht, es verlegt ihn.** Das Anlegen eines Renderers läuft im selben Bild auf dem Render-Thread; das längste Bild nach dem ersten dauert 0,8–1,3 s und liegt jeweils zwei Sekunden vor einem Clip-Start (Stellen 2,0 s, 18,0 s, 34,1 s) — in dieser Zeit steht das ganze Bild. Welcher Anteil auf Lesen, Übersetzen der Skripte und Shader entfällt, ist nicht gemessen. Offen: Entscheid, wie der Anlauf aus dem Bild genommen wird |
| 0.8.4 | 2026-10-04 | Session 94 — **C2a, Messlauf** (Wunsch Patrik: die Messung selbst fahren, GPU über den Windows-Eintrag der Exe umschalten): `test_ComposerMessung.cpp` läuft nur mit `LUMIVIZ_COMPOSER_MESSUNG=<datei>` und fährt die Komposition einmal durch — echtes Fenster, echter Render-Thread, Unlimited, künstliche Audiodaten. Messung um das längste Bild nach dem ersten ergänzt. **Zahlen, Release-clang, 1920 × 1080, `asset/composer_demo.lvcomp` (50 s, höchstens zwei Renderer zugleich):** AMD Radeon 610M — 276 und 279 Bilder je Sekunde (zwei Läufe), Rechnen im Mittel 0,14 ms (CPU-seitig), erstes Bild 156–177 ms, danach längstes Bild 3,0 ms (beim Vorwärmen an Stelle 13 s), Grafikspeicher 69 MB weniger frei; RTX 4090 — 1444 und 938 Bilder je Sekunde (zwei Läufe, die Streuung ist nicht geklärt), Rechnen im Mittel 0,20–0,33 ms, erstes Bild 35–678 ms, danach längstes Bild 138–172 ms in der ersten Sekunde, Grafikspeicher 105 MB weniger frei. **Nicht gemessen:** MilkDrop- und schwere AVS-Presets, mehr als zwei Renderer zugleich, die App selbst mit echter Musik. Rückbau-Liste: der Messlauf geht mit den Testdateien |
| 0.8.3 | 2026-10-04 | Session 94 — **C2a, Messung** (Entscheid Patrik: a — die Bildquelle misst sich in der laufenden App): `Bildquelle::messung()` führt mit: gerechnete Bilder und Bildrate, Zeit je Bild für Rechnen und Zeichnen (Mittel, längstes samt Stelle, erstes Bild), höchste Zahl gleichzeitiger Renderer, freier Grafikspeicher laut Treiber vor dem ersten Bild und am tiefsten Punkt (`GL_NVX_gpu_memory_info` bei NVIDIA, `GL_ATI_meminfo` bei AMD), GPU-Name und Bildgröße. **Close Composition** (und das Ersetzen einer laufenden) schreibt die Zusammenfassung ins `LumiViz.log`. Die Zeiten sind CPU-seitig, ohne `glFinish`; der Speicherwert gilt für das ganze System. Nur eigener Code. **Die Zahlen selbst stehen noch aus** — sie kommen aus Patriks Lauf mit der Release-clang-Exe |
| 0.8.2 | 2026-10-04 | Session 94 — **C2a, Einschalten** (Entscheid Patrik: a — zwei Menüeinträge): Naht N8 in §12.2 für C2a gefasst — File → **Open Composition...** / **Close Composition** (Schreibweise wie die übrigen Einträge des Menüs), `MainWindow` reicht zwei Ereignisse an `composer/AppAnbindung` weiter; Open lädt die `.lvcomp`, setzt die Bildquelle und startet die Uhr sofort, Close nimmt sie weg; letzter Ordner unter `composer/lastDir`. Belegt über die echte Fassade `VisualizerWidget` (dritter Testfall in `test_ComposerBildquelle.cpp`: einschalten, ersetzen, ausschalten, das Fensterbild aufgenommen) — damit hat auch die Durchreichung der Naht R3 ihren Test; 870 Cases. Beispiel `asset/composer_demo.lvcomp`. Rückbau-Liste: Zeile C2a, Einschalten. Offen für C2a: im Programm gesehen, Zeiten im Release-Build, Bildspeicher |
| 0.8.1 | 2026-10-04 | Session 94 — **C2a, Bildquelle** (eigener Code, keine Naht): `composer/Bildquelle` setzt sich über R3 an die Stelle des einen Renderers — fragt die Uhr nach der Stelle, lässt die Wiedergabe rechnen, zeichnet das Gemischte ins Fenster, reicht Audiodaten an die Renderer der Clips (auch an neu dazugekommene). **Steht die Uhr, steht das Bild:** gerechnet wird nur bei laufender Uhr oder geänderter Stelle, Größe oder Komposition. Belegt in `test_ComposerBildquelle.cpp`, auch im echten Render-Thread am echten Fenster (Aufnahme des Fensterbilds); 869 Cases. Rückbau-Liste: Commit der Naht R3 (`f8821db`) eingetragen, Zeile C2a, Bildquelle. Offen für C2a: wie die Komposition in der App eingeschaltet wird, Zeiten im Release-Build, Bildspeicher |
| 0.8.0 | 2026-10-04 | Session 94 — **Naht R3 gebaut** (Entscheid Patrik: a — zweiter Platz im Render-Thread, der Renderer der Kette bleibt angelegt): §12.2 Zeile R3 neu gefasst; §12.3 — der Vergleichslauf belegt R3 nicht, Beleg ist `test_ComposerNahtR3.cpp`; Rückbau-Liste um C2a, Naht R3 erweitert; 867 Cases |
| 0.7.1 | 2026-10-04 | Session 93 — **Zweig-Regel geändert (Patrik):** nach `master` geholt wird erst, wenn der Composer sauber funktioniert; bis dahin läuft alle Arbeit im Zweig. C10, §11 „Zum Zweig" und §12.4 angepasst: Rückbau über die Rückbau-Liste statt „Zweig verwerfen", Prüfung über die Pfade des Composers; Commit von C1b eingetragen |
| 0.7.0 | 2026-10-04 | Session 93 — **Stufe C1b erledigt** (Zweig `composer`): §9d — Datenmodell mit Pegelkurven (sechs Formen), Datei `.lvcomp`, Uhr mit Transport, Wiedergabe mit Vorwärmen, Entladen und Karaoke einmal darüber; nur eigener Code, keine Naht; 866 Cases; Rückbau-Liste: C1a geprüft (ein Commit `91967c2`), C1b eingetragen |
| 0.6.0 | 2026-10-04 | Session 93 — **Stufe C1a erledigt** (Zweig `composer`): §9c — `Leinwand` und `Mischer` mit sieben Mischarten und Pegel; Nähte R1 (Present-Ziel statt Lesezugang) und R2 (Karaoke auslassen, einmal auflegen); MilkDrop nebeneinander bitgleich; Mischen 0,29 ms je Bild; Vergleichslauf 90 / 90 bitgleich, 858 Cases; Render-Thread und Release-Zeiten gehen nach C2; Rückbau-Liste C1a |
| 0.5.0 | 2026-10-04 | Session 93 — **alle Entscheide gefallen** (Patrik): C2 a eigener Transport, C3 eigener Mischer, C4 a Visual-Playlist entfällt, C5 a Kürzel springen im Composer, C6 a Laden schaltet nach Rückfrage aus, dazu **ein Schalter Lock** für Kürzel und Laden (keine Rückfrage bei Vorführungen), C7 a Start lädt die Komposition angehalten am Anfang, C9 a Lücke schwarz, C11 entfällt. §5 neu gefasst (Mischer, Vorwärmen, Entladen, Karaoke einmal durch den Mischer, fünf offene Punkte für C1a); §8 mit den Regeln; Stufen C1a / C1b; Nahtstellen neu (R1–R3 am Rand des Renderers, keine in der Bildberechnung) |
| 0.4.0 | 2026-10-04 | Session 93 — **Stufe C0b erledigt** (Frage Patrik: ist der eigene Mischer die beste Lösung?): §9b — zwei Renderer nebeneinander sind bitgleich mit dem Lauf allein, eine Mix-Datei mit Host Switcher läuft darin unverändert, ein weiterer Renderer kostet wenig, die Karaoke-Ebene zeichnet jeder selbst; Empfehlung C3 und C11 gedreht |
| 0.3.0 | 2026-10-04 | Session 93 — **Stufe C0 erledigt:** §9a mit den Messwerten; Gewicht wirkt als Pegel; Mischart kennt keinen Pegel; ruhende Clips kosten nichts, dafür Anlauf-Ruck und fehlendes Entladen; `.lvfx2` mit Switcher als Clip schwarz; Entscheid C11; Rückbau-Liste C0 |
| 0.2.0 | 2026-10-04 | Session 93 — Vorgabe Patrik: rückbaubar. Neu §12 Rückbau (Regeln, Nahtstellen, Beleg je Stufe, Rückbau-Liste), Entscheid C10 (Zweig) |
| 0.1.0 | 2026-10-03 | Session 93 — Erstfassung aus den Notizen S84, S88, S93 und Patriks Vorgaben vom 03.10.: Name Composer, Visual-Kanäle als Mischer, Audiokanal als Bezug, ein- und ausschaltbar; Stufen; Entscheide |
