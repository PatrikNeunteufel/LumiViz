# LumiViz — Composer (Konzept)

> **Version:** 0.3.0
> **Datum:** 2026-10-04 (Session 93)
> **Typ:** Konzept — **Entwurf, im Grundsatz gebilligt (Patrik S93)**; Stufe
> **C0 (Belege) erledigt**, sonst nichts gebaut; Entscheide §11 offen;
> Vorgabe: rückbaubar (§12); entsteht im Zweig `composer` (C10 a)
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** `Offene_Punkte.md` §7 „Weg zur Timeline" (Ideen Patrik S84, S88,
> S93) · [Visual_Playlist_Konzept.md](Visual_Playlist_Konzept.md) §5a
> („Composer" als Fernziel, S51) · [Hotkey_Konzept.md](Hotkey_Konzept.md) §1
> (Stufe 3) · [HostGruppen_Crossfade_Entwurf.md](../visuals/HostGruppen_Crossfade_Entwurf.md) ·
> [Karaoke_Ebene_Konzept.md](../visuals/Karaoke_Ebene_Konzept.md)
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

Nicht das Ziel: ein Wechsel des Visuals je Song. Das kann der **Host
Switcher** schon (innerhalb einer Kette, beim Songwechsel, nach Zeit oder per
Skript).

---

## 2. Abgrenzung

| Was | Tut | Verhältnis zum Composer |
|---|---|---|
| **Effektkette** | die Komposition **eines** Presets | ein Clip verweist auf eine Kette |
| **Host-Gruppe** | ein ganzes Visual als Teil einer Kette, mit eigenem Bild und Crossfade | Baustein des Renderwegs (§5) |
| **Host Switcher** | schaltet Host-Gruppen einer Kette um (Zeit, Songwechsel, Skript) | bleibt; läuft in einem Clip weiter, wenn die Datei ihn enthält — der Composer warnt (Entscheid 1a, S88) |
| **Visual-Playlist** (Konzept S51, nie gebaut) | geordnete Folge ganzer Presets mit Auslösern | siehe Entscheid C4 |
| **Karaoke-Ebene** | Songtext über allem Laufenden | bleibt darüber; der Composer schaltet später denselben Schalter |
| **Playlist** (Audio) | Folge von Songs | bleibt; tritt zurück, solange ein Audiokanal des Composers an ist |

---

## 3. Was es schon gibt

| Baustein | Stand | Nutzen für den Composer |
|---|---|---|
| **Presetverweis** (S82) | Host-Gruppe, die nur den Pfad speichert | ein Clip **ist** ein Presetverweis |
| **Host-Gruppen** (S39–S43) | eigenes Bild, eigener Puffer-Pool, eigener Skript-Kontext je Gruppe; mehrere gleichzeitig aktiv; stapeln sich über `blendOut` | zwei Clips übereinander = zwei aktive Gruppen |
| **Crossfade** (HG2) | Gewicht je Gruppe 0…1, Ein- und Ausgangskurve je Gruppe (fünf Formen), normalisierter Mix | Übergang zwischen zwei Clips |
| **Mischarten** | `blendOut` kennt 13 Arten (Replace, 50/50, Maximum, Additive, zweimal Subtractive, Every other line/pixel, Xor, Adjustable, Multiply, Buffer, Minimum) | „Add, Sub, Xor usw." |
| **Ziehen** (S84) | Effect Browser, Audio Browser, Playlist geben Datei-URLs ab | Clips und Songs ins Panel ziehen |
| **Lyrics Sync** (S87–S92) | Wellenform eines Titels, Sprungleiste, Positions-Hochrechnung | Zeitachse, Audiokanal |
| **Laufendes Preset** (S93) | sichert und lädt das Hauptfenster | der Composer setzt es über denselben Aufruf |
| **Freeze-Frame** (HG3) | bricht die Bildrate im Blend ein, friert die ausblendende Gruppe ein | Rückfall bei zwei schweren Visuals |

**Belegt in C0** (`test_ComposerBelege.cpp`, Stand §9a): mehrere Gruppen
zugleich, das Gewicht als Pegel, der exakte Mix im Wechsel. Die übrigen
Zeilen der Tabelle sind aus dem Code gelesen.

---

## 4. Die Uhr

Der Composer hat **eine Zeitachse** in Millisekunden ab Null. Woher die Zeit
kommt, hängt am Audiokanal:

- **Audiokanal an:** die Zeit ist die Stelle im Programm, die gerade zu hören
  ist. Der Composer führt den Player: er lädt die Songs der Audio-Clips zu
  ihren Startzeiten und spielt sie. Die Playlist ruht.
- **Kein Audiokanal, oder aus:** siehe Entscheid C2 — der Composer braucht
  dann eine eigene Uhr mit Start, Stopp und Sprung.

In beiden Fällen gilt: **die Stelle auf der Zeitachse bestimmt das Bild**,
nicht die verstrichene Zeit. Pegel werden aus der Stelle berechnet — nach
einem Sprung stimmt der Pegel sofort. Was sich nicht berechnen lässt, ist der
**Zustand** eines Visuals (Feedback, Skript-Variablen): ein Clip beginnt beim
Betreten frisch, auch nach einem Sprung mitten hinein (§10).

---

## 5. Der Renderweg

**Empfehlung (Entscheid C3): der Composer baut auf den Host-Gruppen auf.**
Aus der Komposition entsteht eine Kette, in der jeder Clip eine Host-Gruppe
ist (Presetverweis). Der Composer setzt je Bild, welche Gruppen laufen und
mit welchem Pegel. Die Kette selbst ist ein gewöhnlicher Baum — der
kalibrierte Renderer bleibt der einzige.

Was dafür schon passt (gelesen, s. §3): mehrere Gruppen laufen gleichzeitig,
jede in ihrem eigenen Bild; eine Gruppe mit Gewicht 0 rendert nicht; der
Übergang zweier Gruppen ist ein exakter gewichteter Mix.

Was fehlt — **Stand nach C0** (Messwerte in §9a):

1. **Pegel von außen** (Naht N1). Das Gewicht einer Gruppe wirkt schon als
   Pegel über dem, was darunter steht — die Überlagerung aus der Skizze S88
   geht also im Grundsatz. Es läuft aber mit der **Zeit** auf sein Ziel zu
   (`crossfadeSeconds`); der Composer muss es aus der Kurve **setzen**.
2. **Mischart mit Pegel** (Naht N2). Bestätigt: der Pegel wirkt nur bei
   „Adjustable". Additive mit Pegel 64 legt das Bild **voll** dazu; und unter
   vollem Gewicht wird immer überblendet, egal welche Mischart gewählt ist.
   „Additiv auf halbem Pegel" braucht eine Erweiterung der Mischung.
3. **Laden — kleiner als gedacht, dafür zwei andere Punkte** (Naht N3).
   Ein ruhender Clip kostet **nichts** außer dem Lesen (6–12 ms je Preset):
   nichts wird kompiliert, kein Bild angelegt, bis er läuft. 32 ruhende
   Clips ändern an zwei laufenden nichts. „Laden nach Bedarf" entfällt als
   Voraussetzung. Dafür:
   - **Anlauf-Ruck:** das erste Bild eines Clips kostet das Kompilieren
     (im Test 110–240 ms, Testing-Build) — sichtbar als Ruck am Clip-Anfang.
     Der Composer weiß im Voraus, was kommt, und kann **vorwärmen**.
   - **Entladen:** was ein Clip angelegt hat, bleibt nach seinem Ende
     liegen (8 Clips nacheinander: 19 → 174 Laufzeiten). Über ein langes
     Programm muss der Bestand wieder freigegeben werden.
4. **Tiefenregel — schwerer als gedacht.** Ein `.lvfx2` mit Host Switcher
   bleibt als Clip **schwarz**: seine Gruppen werden zu Effect Lists, der
   Switcher findet nichts mehr zu schalten, und weil er die Gruppen erst zur
   Laufzeit einschaltete, ruhen die Listen. Der Ladebericht sagt nichts
   dazu. „Warnen" (Entscheid 1a, S88) genügt damit nicht — siehe C11.

Die Alternative (ein eigener Mischer über mehreren unabhängigen Renderern)
steht in Entscheid C3.

---

## 6. Datenmodell und Datei

Eine Komposition ist eine eigene Datei, Vorschlag **`.lvcomp`** (JSON, wie
`.lvfx`). Sie enthält **Pfad-Referenzen**, keine eingebetteten Presets
(Entscheid Patrik S60 für die Visual-Playlist, hier übernommen); fehlende
Dateien meldet das Laden.

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

---

## 7. Das Panel

**View → Panels → Composer**, eigenes andockbares Panel, breit gedacht.

- **Kopf:** Schalter **Composer on**, Transport (Start, Stopp, an den Anfang),
  Stelle als Zeit, Zoom, Datei (New, Load…, Save, Save As… — die Preset-Zeile
  aus S89 als Vorbild).
- **Zeitlineal** mit Abspielmarke; Klick springt.
- **Kanäle** als Zeilen, links je Kanal Name und an/aus. Clips als Blöcke:
  ziehen = verschieben, Kanten ziehen = Start und Ende, Entf = löschen.
- **Pegelkurve** im Clip sichtbar; Punkte anfassen und ziehen
  (der „Übergangs-Editor" aus S88).
- **Ziehen hinein:** Presets aus dem Effect Browser in einen Visual-Kanal,
  Songs aus Audio Browser und Playlist in den Audio-Kanal.
- **Audio-Kanal:** die Wellenform des Songs im Clip (wie in Lyrics Sync).
- **Rechtsklick** auf Clip, Kanal und freie Fläche — wie im Kettenbaum.

---

## 8. Wer steuert was

| Zustand | Presets | Musik |
|---|---|---|
| **Composer aus** | wie heute: Browser, Preset-Zeile, PgUp/PgDn, Host Switcher | Playlist |
| **Composer an, kein Audiokanal an** | der Composer | Playlist |
| **Composer an, Audiokanal an** | der Composer | der Composer; die Playlist ruht |

Offen und als Entscheid geführt: was PgUp/PgDn und die Lied-Kürzel bei
laufendem Composer tun (C5), was ein von Hand geladenes Preset bewirkt (C6),
was der Start wiederherstellt (C7).

---

## 9. Stufen

Jede Stufe endet mit etwas Prüfbarem. Gebaut wird erst nach Freigabe des
Konzepts und der Entscheide. Für jede Stufe gelten die Regeln zum Rückbau
(§12): Eigenes in eigenen Dateien, Bestehendes nur an benannten Nahtstellen,
„Composer aus" belegt wie vorher, Rückbau-Liste nachgeführt.

| Stufe | Inhalt | Prüfbar an |
|---|---|---|
| **C0 Belege** ✅ | Tests zu §5: Pegel einer Gruppe, zwei Gruppen auf Teilpegel, Mischart mit Pegel, `.lvfx2` als Clip, Kosten von N Clips | erledigt (S93), Ergebnisse §9a |
| **C1 Modell und Wiedergabe** | Komposition als Datenmodell, `.lvcomp` lesen und schreiben, Komposition → Kette, Pegel von außen, Uhr; **ohne Panel** | Test: an Stelle *t* zeigt das Bild die erwarteten Clips mit den erwarteten Pegeln |
| **C2 Panel zum Ansehen** | Panel mit Lineal, Kanälen, Clips, Abspielmarke, Transport, Schalter Composer on | eine von Hand geschriebene `.lvcomp` läuft und ist zu sehen |
| **C3 Bearbeiten** | Clips hineinziehen, verschieben, Kanten ziehen, löschen; Kanäle anlegen; Undo | eine Komposition ganz im Panel bauen |
| **C4 Pegelkurven** | Punkte im Clip, Kurvenformen, Mischart je Clip | Übergang und Überlagerung aus der Skizze S88 |
| **C5 Audiokanal** | ein Audiokanal: Songs als Clips, Wellenform, führt den Player, Playlist ruht | Visuals an einem Song ausrichten |
| **C6 Steuerung** | Kürzel, Preset laden bei laufendem Composer, Start-Wiederherstellung | Entscheide C5–C7 im Programm |
| später | zweiter Audiokanal (Songs mischen), Takt-Raster, Karaoke-Schalter als Spur, Kompositionen als Video rendern, Parameter über der Zeit | — |

### 9a. Stand C0 — ✅ erledigt (S93)

`tests/unit/UnitTests/test_ComposerBelege.cpp`, Suite `ComposerBelege`,
vier Testfälle. Kein bestehender Code berührt. Wo ein Test eine **Lücke**
festhält, steht das am Test — er wird mit der Naht, die sie schließt,
bewusst umgeschrieben.

| Frage | Ergebnis | Messwert |
|---|---|---|
| Laufen zwei Gruppen zugleich? | ja, auch drei; sie stapeln sich in Kettenreihenfolge | Rot + Grün + Blau additiv je 0,5 → 0,502 / 0,502 / 0,502 |
| Geht ein Teilpegel? | ja, über „Adjustable" und `outAdjustAlpha`, auch je Bild per Skript | 128 → 0,498 / 0,502; per Skript 64 → 0,749 / 0,251 |
| Wirkt das Gewicht als Pegel über dem Darunterliegenden? | ja | Blau blendet über stehendem Rot ein, halbe Zeit → Blau 0,494, Rot von 0,502 auf 0,255 |
| Ist der Wechsel A → B ein exakter Mix? | ja | halbe Zeit → 0,498 / 0,502 |
| Lässt sich das Gewicht setzen? | **nein** — nur zeitgeführt | Lücke N1 |
| Nimmt eine andere Mischart einen Pegel an? | **nein** | Additive, Pegel 64 → Blau 1,000 statt 0,25; im Übergang Replace und Additive bildgleich. Lücke N2 |
| Was kostet ein ruhender Clip? | Lesen, sonst nichts | 6–12 ms je Preset; 8 und 32 ruhende Clips: 0 Laufzeiten, 0 ms je Bild |
| Was kostet der Anlauf? | das Kompilieren im ersten Bild | 110–240 ms (Testing-Build, 640 × 640) |
| Wird nach dem Ende entladen? | **nein** | 8 Clips nacheinander: 19 → 174 Laufzeiten |
| `.lvfx2` mit Host Switcher als Clip? | **schwarz**, ohne Meldung | 0 Host-Gruppen, 2 Listen, 1 Switcher; allein geladen dagegen richtig (0,251) |

**Nicht gemessen:** der Bildspeicher je laufender Gruppe (kein Zugang im
Test; rechnerisch ein Bildpaar in Kettenauflösung plus Puffer-Pool), die
Zeiten im Release-Build und auf der schnellen GPU, zwei wirklich schwere
Visuals zugleich. Die Bildzeiten des Tests (105–118 ms je Bild) sind die
des Testing-Builds und sagen nichts über den Betrieb.

**Folge für C3:** die Empfehlung a (Host-Gruppen) hält. N1 und N2 sind
kleine, abgegrenzte Erweiterungen an einer Stelle (`renderHostGroup`); N3
wird kleiner und anders (vorwärmen, entladen). Neu zu entscheiden ist die
Tiefenregel (C11).

---

## 10. Risiken

- **Ladezeit und Speicher.** Jede Gruppe hält ein eigenes Bildpaar und einen
  eigenen Puffer-Pool. Ohne Laden nach Bedarf (§5 Punkt 3) skaliert das nicht.
- **Zwei schwere Visuals zugleich.** Eine Überlagerung rechnet beide voll.
  Der Freeze-Frame-Rückfall friert heute nur die **ausblendende** Gruppe ein —
  bei einer gehaltenen Überlagerung gibt es keine ausblendende.
- **Springen.** Visuals haben Zustand; nach einem Sprung mitten in einen Clip
  sieht er anders aus als beim Durchlauf. Verwandt: die Idee Preset-Warmup
  (Bilder vorrechnen).
- **Nachladen im Lauf.** Ein Preset zu kompilieren kostet Zeit; geschieht es
  erst am Clip-Anfang, ruckt das Bild. Vorladen im Hintergrund ist mit dem
  Render-Thread abzustimmen.
- **Zweiter Audiokanal.** Die Audio-Engine führt heute einen Stream. Zwei
  Songs zu mischen heißt zwei Streams oder ein Mixer — und die Frage, wessen
  Spektrum die Visuals sehen.
- **Größe des Panels.** Eine Zeitachse mit Clips, Kurven und Wellenform ist
  das größte Bedienelement der App; Schritt für Schritt (C2–C4), nicht auf
  einmal.

---

## 11. Entscheide

**Schon entschieden:** Name **Composer** (Patrik S93) · Pfad-Referenzen statt
eingebetteter Presets (S60) · ein `.lvfx2` darf Clip sein, gewarnt wird nur
bei enthaltenem Host Switcher (S88, 1a) · die Karaoke-Ebene bleibt über allem,
der Composer schaltet später ihren Schalter (S88, 2a).

### C2 — Die Uhr ohne Audiokanal

- **Problem:** Ohne Audiokanal gibt es keine Stelle im Programm, aus der die
  Zeit käme. Die Playlist spielt daneben irgendeinen Song.
- **a (Empfehlung):** Der Composer hat einen **eigenen Transport** (Start,
  Stopp, Sprung) und läuft nach der Wanduhr. Die Playlist spielt unabhängig;
  ein Schalter „mit dem Player starten und stoppen" koppelt nur Start und
  Stopp.
- **b:** Die Uhr ist die **Stelle im laufenden Song** der Playlist; mit jedem
  Song beginnt die Komposition von vorn.
- *Warum:* Den Ausschlag gibt Patriks Satz, man könne die Visuals „anhand der
  Zeitlinie als Referenz" anordnen — das verlangt eine Zeit, die es auch ohne
  Musik gibt. Für b spricht, dass Bild und Musik ohne Zutun zusammenlaufen;
  das leistet aber der Audiokanal, sobald man einen anlegt. Kippt, wenn der
  häufigste Fall „ein Song, eine Komposition, Musik aus der Playlist" ist.

### C3 — Der Renderweg

- **Problem:** Zwei Kanäle heißt zwei Visuals zugleich, mit freiem Pegel und
  wählbarer Mischart.
- **a (Empfehlung):** **Host-Gruppen** (§5): die Komposition wird eine Kette,
  der Composer setzt die Pegel.
- **b:** Ein **eigener Mischer** über mehreren unabhängigen Renderern, je
  Kanal einer, darüber ein Mischpass.
- *Warum:* Den Ausschlag gibt, dass Gruppen, eigener Zustand je Gruppe,
  Presetverweis, Kurven und der exakte Mix vorhanden sind und der kalibrierte
  Renderer der einzige bleibt. Für b spricht die saubere Trennung: jedes
  Preset läuft unverändert, die Tiefenregel entfällt, und je Kanal wäre ein
  eigener Thread denkbar. Das wiegt weniger, solange C0 die vier Lücken aus §5
  als schließbar belegt. **Kippt zu b**, wenn C0 zeigt, dass die Tiefenregel
  gängige `.lvfx2` als Clips unbrauchbar macht oder das Laden nach Bedarf den
  Ketten-Aufbau sprengt.

### C4 — Die Visual-Playlist

- **Problem:** Das Konzept von S51 (Folge ganzer Presets mit Auslösern) ist
  entschieden, aber nie gebaut — und überschneidet sich mit dem Composer.
- **a (Empfehlung):** Sie **entfällt als eigenes Vorhaben**. Was sie sollte,
  deckt der Composer (feste Folge über der Zeit) zusammen mit dem Host
  Switcher (Wechsel nach Zeit, Songwechsel, Zufall). Das Konzept bleibt als
  Quelle stehen und bekommt einen Vermerk.
- **b:** Sie wird **vor** dem Composer gebaut, als einfache Vorstufe.
- *Warum:* Den Ausschlag gibt, dass ihr Kern — „ein Song, ein Look" — seit
  S82 mit Host Switcher und Presetverweisen geht. Für b spricht, dass sie
  klein ist und ohne Zeitachse auskommt. Kippt, wenn du eine lose Liste von
  Presets zum Durchschalten vermisst, die keine Zeiten braucht.

### C5 — PgUp/PgDn und die Lied-Kürzel bei laufendem Composer

- **Problem:** Die Tasten blättern heute im Effect Browser bzw. in der
  Playlist. Bei laufendem Composer würden sie ihm das Bild oder den Song
  wegnehmen.
- **a (Empfehlung):** Sie **springen im Composer**: PgUp/PgDn zum vorigen
  bzw. nächsten Clip-Anfang, die Lied-Kürzel (nur mit aktivem Audiokanal) zum
  vorigen bzw. nächsten Song der Komposition. Ein Schalter im Panel **sperrt**
  sie ganz (Idee Patrik S93).
- **b:** Sie sind bei laufendem Composer **immer gesperrt**.
- **c:** Sie wirken **wie heute** und schalten den Composer dabei aus.
- *Warum:* Den Ausschlag gibt das Hotkey-Konzept (§1): dieselbe Taste bedient
  in jeder Ausbaustufe dieselbe Absicht, nur die Quelle wechselt — „nächstes
  Preset" bleibt „nächstes Preset". Für b spricht die Sicherheit bei einer
  Vorführung; das deckt der Sperr-Schalter. Kippt, wenn ein Sprung auf der
  Zeitachse im Betrieb mehr stört als hilft.

### C6 — Ein Preset von Hand laden, während der Composer läuft

- **Problem:** Doppelklick im Browser oder eine Wahl in der Preset-Zeile.
- **a (Empfehlung):** Das Laden **schaltet den Composer aus**, nach einer
  Rückfrage („Composer is running — stop it and load this preset?").
- **b:** Das Laden ist **gesperrt**, solange der Composer an ist.
- **c:** Das Preset wird als **Clip an der Abspielmarke** eingefügt.
- *Warum:* Den Ausschlag gibt, dass die App sich ohne Composer wie gewohnt
  bedienen lassen muss und der Weg zurück ein Klick sein soll. Für c spricht
  der Arbeitsfluss beim Bauen; dafür gibt es das Ziehen ins Panel, und ein
  Doppelklick mit zwei Bedeutungen ist schwer zu merken.

### C7 — Was der Start wiederherstellt

- **Problem:** Seit S93 lädt der Start das zuletzt laufende Preset. Lief ein
  Composer, gibt es kein einzelnes.
- **a (Empfehlung):** Der Start lädt die **Komposition** wieder, Composer
  **an**, an den **Anfang** gestellt und **angehalten**.
- **b:** Der Start lädt sie und **läuft** an der letzten Stelle weiter (wie
  „Resume playback").
- **c:** Der Start lädt das zuletzt laufende **einzelne** Preset, die
  Komposition bleibt nur im Panel gemerkt.
- *Warum:* Den Ausschlag gibt, dass ein Programm, das von selbst losläuft,
  bei einer Vorführung überrascht. Für b spricht die Gleichbehandlung mit dem
  Player; mit aktivem Audiokanal könnte b dem Schalter „Resume playback"
  folgen.

### C9 — Die Lücke

- **Problem:** An einer Stelle, an der in **keinem** Visual-Kanal ein Clip
  liegt.
- **a (Empfehlung):** **Schwarz.** Was nicht angeordnet ist, ist nicht da.
- **b:** Das **letzte Bild** bleibt stehen.
- **c:** Die Kette, die **vor dem Einschalten** des Composers lief, scheint
  durch.
- *Warum:* Den Ausschlag gibt, dass der Composer die Kontrolle über das Bild
  hat, sobald er an ist — eine Lücke ist dann eine gewollte Pause. Für c
  spricht ein „Grund-Visual" unter allem; das lässt sich als Clip im untersten
  Kanal über die ganze Länge ausdrücken.

### C10 — Wo der Composer entsteht

- **Ort:** das Repo; bisher committet Patrik jeden Schritt auf `master`.
- **Problem:** Der Composer soll sich zurückbauen lassen (§12). Am
  einfachsten ist ein Rückbau, der gar nicht nötig wird, weil das Unfertige
  nie im Hauptzweig lag.
- **a (Empfehlung):** Ein **eigener Zweig** `composer`. `master` bleibt, wie
  er ist; nach jeder abgenommenen Stufe (Tests, Vergleichslauf, Sichttest)
  wird der Zweig nach `master` geholt. Vor dem Holen ist der Rückbau: nicht
  holen. Danach gilt die Rückbau-Liste (§12.4).
- **b:** Weiter auf **`master`**, mit den Regeln aus §12 (Naht und Eigenes
  getrennt committet). Der Rückbau läuft immer über die Rückbau-Liste.
- *Warum:* Den Ausschlag gibt, dass die Nähte N1–N3 im kalibrierten Renderer
  liegen und eine Stufe erst nach dem Sichttest als gut gilt — bis dahin soll
  `master` davon frei sein. Für b spricht, dass nichts auseinanderläuft und
  jede andere Arbeit sofort mit dem Composer zusammen getestet wird. Das wiegt
  weniger, solange der Zweig nach jeder Stufe geholt wird und nie lange lebt.
  Kippt, wenn du parallel zum Composer viel am Renderer oder am
  Effect-Chain-Panel arbeitest — dann kosten die Zusammenführungen mehr, als
  der Zweig schützt.

### C11 — Dateien mit Host-Gruppen als Clip (neu nach C0)

- **Ort:** Tiefenregel der Kette (keine Host-Gruppe in einer Host-Gruppe);
  Entscheid 1a aus S88 („ein `.lvfx2` darf Clip sein, gewarnt wird nur bei
  enthaltenem Host Switcher").
- **Heute:** In einer Host-Gruppe werden Host-Gruppen zu Effect Lists.
- **Problem (C0):** Ein Clip ist eine Host-Gruppe. Ein `.lvfx2` mit Host
  Switcher bleibt darin schwarz, ohne Meldung. Ein `.lvfx2` ohne Switcher
  läuft, verliert aber, was seine Gruppen ausmacht (eigenes Bild, eigener
  Zustand, Crossfade). 1a aus S88 ist so nicht haltbar.
- **a (Empfehlung):** In den ersten Stufen **nimmt der Composer solche
  Dateien nicht an** und sagt, warum („contains host groups — not
  supported as a clip yet"). Clips sind `.avs`, `.milk`, `.lvfx` und
  `.lvfx2` ohne Host-Gruppen. Ob eine Gruppe im Clip laufen kann, wird als
  eigene spätere Stufe belegt und dann entschieden.
- **b:** Der Clip wird eine **eigene Knotenart**, in der Host-Gruppen
  erlaubt bleiben (eine Ebene tiefer als heute) — von Anfang an.
- **c:** Wechsel zum **eigenen Mischer** (C3 b), bei dem jede Datei
  unverändert in ihrem eigenen Renderer läuft.
- *Warum:* Den Ausschlag gibt die Rückbaubarkeit: a braucht keine Naht im
  Renderer und nimmt nichts vorweg. Für b spricht, dass deine Mix-Dateien
  (`20261001.mix.lvfx2`) genau solche sind und sonst draußen bleiben. Ob b
  technisch trägt, ist **nicht belegt** — die Degradierung greift vor dem
  Rendern, ein Test dafür müsste sie erst umgehen. Für c spricht, dass die
  Frage dort gar nicht entsteht; es kostet den zweiten Renderweg. Kippt zu
  b, wenn Mix-Dateien als Clips für dich der Normalfall sind — dann gehört
  der Beleg dazu an den Anfang von C1.

---

## 12. Rückbau

**Vorgabe Patrik (S93):** Der Composer muss sich mit einem Rollback wieder
ausbauen lassen, falls etwas schiefgeht. Das ist keine Notiz für den
Ernstfall, sondern eine Bauregel für jede Stufe.

### 12.1 Regeln

1. **Eigenes liegt in eigenen Dateien.** Modell, Datei-Format, Uhr, Panel und
   Tests des Composers stehen in neuen Dateien unter eigenen Namen
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

### 12.2 Die Nahtstellen (Stand des Entwurfs)

| Naht | Wo | Was sich ändert | Vorgabe = altes Verhalten |
|---|---|---|---|
| N1 Pegel von außen | `renderHostGroup`, `GroupRuntime` | ein optionaler Pegel überschreibt das zeitgeführte Gewicht | nicht gesetzt → Gewicht läuft wie heute |
| N2 Mischart mit Pegel | Ausgangs-Mischung der Gruppe | die gewählte Mischart gilt auch unter vollem Gewicht | nur für Gruppen mit Pegel von außen; alle anderen mischen wie heute |
| N3 Vorwärmen und Entladen | Laufzeiten der Gruppen | eine Gruppe lässt sich vor ihrem Einsatz kompilieren und nach ihrem Ende freigeben | ohne Aufruf des Composers wie heute (C0: ruhende Clips kosten ohnehin nichts) |
| N4 Laufendes Preset | `MainWindow::setzeLaufendesPreset`, Start | der Composer meldet sich als Quelle; der Start kennt eine Komposition | ohne Schlüssel `composer/…` wie heute |
| N5 Kürzel | Empfänger von `PresetStepEvent` und der Lied-Kürzel | bei laufendem Composer geht das Ereignis an ihn | Composer aus → Effect Browser bzw. Playlist |
| N6 Musik | Player / Playlist | der Audiokanal führt den Player | Audiokanal aus → Playlist |
| N7 Anmeldung | Panel-Registrierung, Menü, `Source.cmake`, Dateityp-Icon | Einträge | — (reine Zusätze) |

Welche Nähte es wirklich braucht, zeigt C0. Kommt eine dazu, kommt sie in
diese Tabelle, bevor sie gebaut wird.

### 12.3 Der Beleg je Stufe

- **Unit-Tests** grün, 0 Skips — die bestehenden unverändert.
- **Vergleichslauf** nach jeder Stufe, die den Renderer berührt (N1–N3):
  bitgleich gegen den Bezugsstand, mit ausgeschaltetem Composer.
- **Ein eigener Test „Composer aus"** je Naht: derselbe Aufruf mit und ohne
  die Naht liefert dasselbe Ergebnis, solange der Composer nicht eingreift.

### 12.4 Die Rückbau-Liste

Wird mit C0 angelegt und je Stufe nachgeführt, in diesem Dokument:

| Stufe | Eigene Dateien (löschen) | Naht-Commits (zurücknehmen) | Einträge (herausnehmen) | Rückbau geprüft |
|---|---|---|---|---|
| C0 | `tests/unit/UnitTests/test_ComposerBelege.cpp` | keine | eine Zeile in `tests/unit/UnitTests/Source.cmake` | nicht nötig — nur Tests, kein bestehender Code berührt |

„Rückbau geprüft" heißt: der Stand nach dem Rückbau baut, die Unit-Tests
sind grün und der Vergleichslauf ist bitgleich. Geprüft wird das **einmal
nach C1** — der ersten Stufe mit Nähten im Renderer — und danach, wenn eine
neue Naht dazukommt.

---

## 13. Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 0.3.0 | 2026-10-04 | Session 93 — **Stufe C0 erledigt:** §9a mit den Messwerten; §5 danach neu gefasst (Gewicht wirkt als Pegel; Mischart kennt keinen Pegel; ruhende Clips kosten nichts, dafür Anlauf-Ruck und fehlendes Entladen; `.lvfx2` mit Switcher als Clip schwarz); Naht N3 = Vorwärmen und Entladen; neuer Entscheid C11; Rückbau-Liste C0; Zweig `composer` (C10 a) |
| 0.2.0 | 2026-10-04 | Session 93 — Vorgabe Patrik: rückbaubar. Neu §12 Rückbau (Regeln, Nahtstellen N1–N7, Beleg je Stufe, Rückbau-Liste), Entscheid C10 (Zweig), Stufen verweisen darauf |
| 0.1.0 | 2026-10-03 | Session 93 — Erstfassung aus den Notizen S84, S88, S93 und Patriks Vorgaben vom 03.10.: Name Composer, Visual-Kanäle als Mischer, Audiokanal als Bezug, ein- und ausschaltbar; Renderweg über Host-Gruppen als Empfehlung (gelesen, nicht getestet); Stufen C0–C6; Entscheide C2–C7, C9 offen |
