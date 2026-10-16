# LumiViz — Composer (Konzept)

> **Version:** 0.96.0
> **Datum:** 2026-10-16 (Session 130)
> **Typ:** Konzept — **gebilligt, alle Entscheide gefallen** (Patrik S93: C2 a,
> C3 eigener Mischer, C4 a, C5 a, C6 a mit Sperre, C7 a, C9 a, C10 a);
> Stufen **C0 bis C2 erledigt und im Programm gesehen**; **C3 (Bearbeiten,
> Neues hinein, Sync on Effect Chain) und der erste Teil von C4 (Blenden mit
> Bézier) gebaut** (S95; im Programm gesehen bis C3a); **C4 Teil 2
> (Pegelform, Curve Editor, Stücke, Kurven-Presets) und C5 (Audiokanal)
> gebaut** (S96; der Ton im Programm gehört, sonst nichts gesehen — offen:
> Audio-Kanäle überlagern sich nicht, §4 — **Entscheide dazu gefallen (S97,
> Stufe C5b); gebaut sind Schritt 1, der Mixer in der Engine (Naht N12), und
> Schritt 2, die Führung der Audio-Kanäle über den Mixer, und Schritt 3, das
> Spektrum des Mixers für die Visuals (Naht N13; beide S98) — die Songs
> überlagern sich, im Programm noch nicht gehört; Schritt 4, die Anzeige des
> Player-Panels, nach geändertem Entscheid gebaut (S98: „Composer - <Song>");
> Schritt 5, das Benutzerhandbuch, geschrieben; das Überlagern im Programm
> gehört (S98), Korrekturen, Weiterspielen und die Bedienung der Zeitachse
> im Programm gesehen (Sichttest S98b am 06.10. abends: 33 von 33, im Log
> kein Nachsetzen); offen aus der ersten Liste P13 bis P16, das Panel Player
> und die Urteile R**) — **S99: die Überlappung in der Schleife und die
> Blende der ganzen Komposition entschieden und in sieben Schritten gebaut,
> die Lücke „Composer off ohne Titel im Player" geschlossen; im Programm
> nichts davon gesehen oder gehört (Sichttest S99)** — dazu Entscheide aus S95 und S96
> in §4 bis §9e; seit S105 auf `master` (§12) — **S109 / S110: die gemeinsame
> Achse mit dem Loop-Panel und der Loop als Clip im Audio-Kanal (§9f)** —
> **S126: Lautstärke-Abgleich der Ton-Clips entschieden (L1 a bis L4 a),
> G1 bis G3 gebaut, im Programm nichts gesehen (§9g); einzelne Clips
> ausschalten, D1 c, gebaut (§9h)**
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

Seit S109 in die andere Richtung — der Composer **gibt ab** und **nimmt auf**
(§9f): die Zeitachse teilt Maßstab, Zoom und Rollen mit dem Loop-Panel
(`lumi::ui::Achse`), und ein Audio-Kanal nimmt neben Songs auch Loops
(`.lvloop`) auf.

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
  - *Die drei folgenden Punkte beschreiben den Stand von S96 (ein Song, der
    Ton führt). Seit C5b Schritt 2 (S98) gilt statt ihrer der Punkt
    „Entschieden (Patrik S97)" weiter unten; sie bleiben zur Einordnung
    stehen.*
  - **Wer die Zeit führt** (Entscheid Patrik S96: a): **solange ein Song
    spielt, führt der Ton** — die Stelle des Composers ist „Clip-Start +
    Stelle im Song − Einsatz", die Uhr wird im Takt der Sitzung auf den Player
    nachgestellt. **In Lücken** (kein Ton-Clip an der Stelle, nach dem letzten
    Song) **führt die Wanduhr** wie ohne Audiokanal; am Anfang des nächsten
    Ton-Clips lädt und startet der Composer den Song. Einen Sprung im Ton
    hört man, ein um ein Bild nachgestelltes Visual sieht man nicht. Kippt
    mit einem zweiten Audiokanal: zwei Songs zugleich haben nicht die eine
    Stelle im Ton — dann müsste die Uhr führen.
  - **Gebaut als `composer/Tonfuehrung`** (S96, noch nicht an der Sitzung):
    je Takt aus Stelle, Komposition und Stand des Players. **Einstieg** in
    einen Ton-Clip: laden (nur wenn der Player eine andere Datei hat), an die
    Stelle setzen, spielen oder halten. **Der Ton führt:** liegen Uhr und Ton
    mehr als 40 ms auseinander, wird die Uhr nachgestellt — auch nach einem
    Sprung im Player. **Springt die Uhr** (ein Schritt rückwärts oder mehr
    als 1,5 s voraus; bei stehender Uhr jede Bewegung), führt sie dieses eine
    Mal, und der Player wird gesetzt. **Wanduhr:** in der Lücke, und wenn der
    Song vor dem Ende seines Clips aus ist oder sich nicht laden lässt — dann
    startet ihn bis zum Clip-Ende niemand neu, außer die Uhr springt zurück
    in den Song. Der Pegel des Clips geht je Takt an eine Senke (der Anschluss
    an den Player ist Naht N7a). `songAnfang` liefert die Ziele für Next und
    Previous: zurück geht es mitten im Song an seinen Anfang, in seinen
    ersten drei Sekunden zum Song davor.
  - **Welcher Clip spielt** (geändert S96 nach Befund Patrik): **alle
    eingeschalteten Audio-Kanäle zählen**; an jeder Stelle spielt der Clip
    des obersten Kanals, der dort einen hat (`Komposition::tonBei`). Zuerst
    galt „der erste eingeschaltete Kanal" — ein leerer Kanal oben machte die
    Songs darunter stumm, und genau so entstehen Kompositionen im Panel (ein
    Song unter die letzte Zeile gezogen legt jedes Mal einen neuen Kanal an).
    Liegen zwei Songs übereinander, ist nur der obere zu hören: zwei zugleich
    brauchen einen zweiten Stream im Player (später). Next und Previous gehen
    über die Songs aller Kanäle.
  - **Befund Patrik (S96, im Programm gehört) — nächster Schritt:** die
    Audio-Kanäle sollen sich **überlagern**: jeder Kanal spielt seinen Song
    zugleich mit den anderen, jeder mit seiner Pegelkurve — so überblenden
    zwei Songs, auch drei. Heute setzt der Song des zweiten Kanals erst ein,
    wenn der des ersten zu Ende ist, und wird abgeschaltet, sobald oben
    wieder einer liegt. Dafür braucht es **je Audio-Kanal einen eigenen
    Stream** (der Player führt heute einen) — und dann **führt die Uhr**
    statt des Tons, wie beim Entscheid vorgemerkt. Offen dabei: wessen
    Spektrum die Visuals sehen, und wessen Songtext die Karaoke-Ebene zeigt.
  - **Entschieden (Patrik S97) — Stufe C5b; die Führung ist gebaut (S98,
    Schritt 2).** Das ersetzt die Punkte „Wer die Zeit führt", „Gebaut als
    `composer/Tonfuehrung`" und „Welcher Clip spielt" oben.
    - **Gebaut als `composer/Tonfuehrung`** (S98, neu geschrieben; läuft
      gegen `IAudioEngine` und `IAudioPlayer`): beim Übernehmen entsteht ein
      Mixer, der Player wird angehalten, die Playlist ausgehängt. **Je
      Takt:** die Quellen des Mixers werden an die Ton-Clips angeglichen,
      die an der Stelle der Uhr spielen (`Komposition::toeneBei` — alle
      eingeschalteten Audio-Kanäle, der oberste zuerst); eine Quelle wird an
      die Stelle der Uhr **gesetzt** beim Einstieg in ihren Clip, nach einem
      Sprung der Uhr (rückwärts oder mehr als 1,5 s voraus; bei stehender
      Uhr jede Bewegung) und wenn sie der Uhr davongelaufen ist; ihr Pegel
      ist `setVolume` auf ihrem Handle; hält die Uhr, hält der Mixer. Am
      Ende ihres Clips geht die Quelle aus dem Mixer. Ein Song, der vor
      seinem Clip aus ist, hält an seinem Ende; eine Datei, die sich nicht
      öffnen lässt, wird einmal versucht. Lautstärke und Mute des Players
      liest die Führung je Takt und setzt sie am Mixer (kein Abo auf
      `VolumeChangedEvent` nötig). Spielt der Player doch (Doppelklick im
      Audio Browser), hält ihn der nächste Takt an.
    - **Der laufende Song, gebaut:** die Führung nennt ihn im Ergebnis des
      Takts, mit seiner `TrackInfo` von der Quelle (Naht N14); die Sitzung
      meldet den Wechsel als `TrackChangedEvent` und Stelle und „läuft"
      über `MultiEffectVisualizer::notifyPlayPosition` / `notifyPlayState`.
      In einer Lücke bleibt der letzte Song gemeldet, „läuft" ist aus.
    - **Von mir gesetzt (S98); (1) und (2) im Hörtest bestätigt (S99, siehe
      unten), (3) seit S99 durch den Entscheid zum Freigeben ohne Titel
      ergänzt:** (1) **Vorlauf 2 s** —
      ein Song wird zwei Sekunden vor seinem Clip geöffnet und steht
      angehalten am Einsatz, damit das Öffnen der Datei den Einsatz nicht
      verzögert; bei einem Sprung mitten in einen Clip gibt es keinen
      Vorlauf. (2) **Abweichung 80 ms, Schonfrist 500 ms** — eine Quelle
      wird nachgesetzt, wenn sie mehr als 80 ms von ihrem **Grundabstand**
      zur Uhr abweicht, und nach einem Setzen eine halbe Sekunde lang nicht
      geprüft; jedes Nachsetzen kann zu hören sein. Das Log nennt jedes
      („audio source realigned to the clock", mit Datei und Grundabstand;
      Summe beim Freigeben). (3) **Beim Freigeben** wird der Titel des
      Players wieder als laufender Song gemeldet (nicht beim Beenden der
      App); ob der Player weiterspielt, ist seit S98 eine Einstellung (§8).
    - **Befund im Hörtest S98 und seine Ursache.** Patrik hörte keine
      Knackser, das Log zählte im langen Lauf aber 21-mal Nachsetzen, in
      zwei Mustern: rund −280 ms etwa eine Sekunde nach einem Einstieg, und
      knapp über der Schwelle an beiden Quellen zugleich. **Gemessen an der
      echten `BassEngine`** (`test_ComposerNahtMixer.cpp`, Fall „Messung",
      am Song aus `composed2.lvcomp` und an einem Sinus): das Öffnen einer
      Quelle dauert 3 bis 4 ms, das Setzen 0 ms; die Stelle, die die Engine
      als „zu hören" meldet, liegt danach **stetig 40 bis 50 ms hinter der
      Uhr** — nach Einstieg, Sprung, Pause / Play und Setzen im Halt gleich.
      Die Engine war es also nicht. **Zwei Ursachen im eigenen Code:**
      (a) die Führung las die Stelle aus dem gespiegelten Stand der
      Bildquelle, und der ist so alt wie das letzte Bild — rechnet der
      Render-Thread ein langes (ein Preset lädt), bekam der Ton eine
      veraltete Stelle; im Log steht das Nachsetzen neben „RenderThread:
      render 139 ms". (b) Die 40 bis 50 ms Latenz der Ausgabe zählten als
      Abweichung, von 80 ms Spielraum blieben nach einer Seite nur 35.
      **Behoben (S98):** `Bildquelle::standStelleJetztMs` rechnet den
      gespiegelten Stand auf jetzt hoch (Länge und Schleife mitgespiegelt);
      die Führung misst nach jedem Setzen einmal den **Grundabstand** der
      Quelle und setzt nur nach, wenn sie davon abweicht — das trägt auch
      eine Ausgabe mit großer Latenz (Funk). **Im Programm gehört
      (Sichttest S98b, A2 und A3, 06.10. abends):** keine Knackser, kein
      Rutschen nach einem Sprung; das Log zählt in allen 17 Läufen des
      Abends **kein einziges Nachsetzen**, obwohl der Render-Thread
      weiter lange Bilder meldet (103 Zeilen, die längsten 188 und
      191 ms). Vorlauf und Schwelle sind technische Werte ohne eigene
      Urteilsfrage; ihr hörbares Ergebnis ist damit bestätigt (dazu M8,
      Einsatz eines Songs, im Hörtest S98).
    - **Die Visuals sehen die Summe, gebaut (S98, Naht N13):**
      `MainWindow::onAudioUpdate` liest Spektrum und Wellenform vom Mixer,
      solange der Composer Ton führt.
    - **Am Bus, gebaut (S98, Schritt 4):** neben dem Songwechsel verschickt
      die Sitzung für den laufenden Song `PlaybackPositionEvent` (etwa
      zehnmal je Sekunde, sofort nach Songwechsel oder Sprung; in einer
      Lücke keine) und `PlaybackStateEvent`. **„Läuft" am Bus heißt: die
      Komposition läuft** (Entscheid Patrik S98) — dem Renderer meldet die
      Sitzung gleich danach direkt, ob der *Song* läuft, damit Karaoke und
      Lyrics in einer Lücke nicht weiterzählen. Hat jemand den Player
      gestartet (Doppelklick im Audio Browser), hält ihn der nächste Takt
      an, und die Sitzung meldet den laufenden Song und „läuft" noch einmal
      — der Songwechsel-Zähler zählt dabei einmal mehr.
    - **Freigeben ohne Titel im Player (Lücke aus S98, geschlossen S99 —
      Entscheid Patrik: a):** hat der Player keinen Titel geladen, meldet
      die Sitzung beim Freigeben einen **leeren Songwechsel**
      (`TrackChangedEvent` ohne Datei), sofern sie einen Song gemeldet
      hatte. Das Player-Panel geht darauf in seinen Ruhezustand (Naht N5,
      Anzeige, fünfte Stelle — §12.2), Tag View zeigt „No track loaded",
      der Audio Browser nimmt seine Markierung weg. **Vor dem Bau gezählt**
      (`test_ComposerNahtTon.cpp`): kein Songwechsel, Player-Panel
      („Composer - eins") und Tag View zeigten weiter den letzten Song der
      Komposition — die Lücke war also größer als notiert; ein leerer
      Songwechsel allein ergab im Player-Panel „Unknown Title". **Aus dem
      Code gelesen, nicht per Test belegt** (der Empfänger sitzt im
      Konstruktor von `MainWindow`): der Songtext der Karaoke-Ebene und
      die Tags der Text-Vorlagen werden leer (`findeSongtext` gibt ohne
      Pfad nichts zurück, der Versatz ist 0, die Online-Suche bricht nur
      ab); der Songwechsel-Zähler des Host Switchers zählt den leeren
      Wechsel mit, wie den Wechsel beim Freigeben mit Titel. Verworfen:
      b — nur das Player-Panel zurücksetzen (Tag View und Songtext
      blieben beim letzten Song); c — so lassen.
    - **Ein Mixer in der Engine** (Entscheid a): die Songs aller
      eingeschalteten Audio-Kanäle laufen als Quellen in **einem
      Mixer-Stream** (BASSmix). Der Composer hält den Mixer über
      `IAudioEngine` und setzt je Quelle Stelle und Pegel. **Die Uhr führt**
      — immer; der Ton wird an sie gehalten. Der Puffer des Mixers steht auf
      0. Der Player bleibt, wie er ist, und schweigt, solange der Composer
      Ton führt; die Playlist ruht wie bisher.
    - **Die Visuals sehen die Summe mit den Pegeln** (Zielbild Patrik):
      Spektrum und Wellenform kommen vom Mixer — dasselbe, was zu hören ist.
    - **Der laufende Song** (Entscheid a): der Ton-Clip mit dem höchsten
      Wert der Pegelkurve an der Stelle der Uhr. Bei Gleichstand bleibt der
      bisherige; gibt es keinen, gilt der oberste Kanal. Der Composer meldet
      ihn über die bestehenden Wege (Songwechsel, Stelle im Song, läuft /
      hält). Ihm folgen Karaoke-Ebene, Tags, Songwechsel-Zähler, Tag View
      und Audio Browser. In einer symmetrischen Überblendung wechselt alles
      am Kreuzungspunkt der beiden Kurven. Das Player-Panel zeigt ihn
      ebenfalls — als „Composer - <Song>" (Entscheid Patrik S98, §8; in S97
      hieß es zuerst, das Panel zeige die Komposition).
    - **Belegt vor dem Entscheid** (`test_ComposerBelegeTon.cpp`): zwei
      Streams der echten `BassEngine` spielen zugleich, jeder mit eigener
      Stelle; Spektrum und Pegel eines einzelnen Streams kennen weder den
      anderen Stream noch die eigene Lautstärke — einzelne Streams liefern
      die Summe mit den Pegeln also nicht. Der Mixer liefert sie (am
      dekodierenden Mixer abgezählt, Abweichung 0), dazu Hüllkurve und
      Stelle je Quelle, und ist für die Engine ein Stream wie jeder andere.
      Ein Pegelwechsel kommt am spielenden Mixer nach etwa 460 ms an
      (Puffer 0,5 s, die Vorgabe), 90 ms (0,1 s), 30 ms (0 s).
    - **Nicht gemessen:** ob Puffer 0 unter Last aussetzt oder knistert —
      dann müsste der Puffer größer werden und die Pegel als Hüllkurve
      vorausgeplant statt je Takt gesetzt (die Hüllkurve ist belegt); Songs
      mit verschiedenen Abtastraten im Mixer.
  - **Vorgemerkt (Patrik S97), nach C5b:** läuft die Komposition in der
    Schleife, sollen sich ihr Ende und ihr Anfang **überlagern** — Bild und
    Ton gleichermaßen, die Länge einstellbar. **Entscheid Patrik S99: a —
    zwei getrennte Einstellungen, gebaut ist nichts.** (1) **Loop overlap**
    (Sekunden, Vorgabe 0), wirkt nur bei Loop: der Anfang beginnt um diese
    Zeit vor dem Ende, im Fenster laufen die letzten und die ersten Sekunden
    zugleich, jeder Clip mit seiner eigenen Pegelkurve, nichts wird
    zusätzlich geblendet; die Schleife wird um die Überlappung kürzer; beim
    ersten Start spielt noch kein Ende hinein. (2) **Fade in / Fade out der
    ganzen Komposition** (Sekunden, Vorgabe 0 = aus): gelten immer, auch
    beim ersten Start und am Ende ohne Loop; mit beidem zusammen ergibt die
    Schleife eine Überblendung. Verworfen: b — ein Regler „Loop crossfade"
    nur in der Schleife mit automatischer Blende; c — ein Regler „Fade",
    der immer gilt. **Heute** (gelesen S99): `Uhr::stelleMs` rechnet in der
    Schleife Stelle modulo Länge, Bild (`Wiedergabe::rendere`) und Ton
    (`Tonfuehrung::takt`) bekommen genau eine Stelle; über den Sprung
    hinweg wird nicht vorgewärmt (aus dem Code gelesen, nicht getestet).
    **Entscheid Patrik S99 (zweiter): a — die drei Werte gehören zur
    Komposition:** im Kopf der `.lvcomp` und im Fuß des Panels neben
    *Fixed end*, in Sekunden, Vorgabe 0, über Undo; alte Dateien haben 0;
    Loop an / aus bleibt eine Einstellung des Programms; keine Naht.
    Verworfen: b — in Settings für alle Kompositionen gleich (Naht N9
    wüchse); c — Vorgabe in Settings mit Ausnahme je Datei.
    **Entscheid Patrik S99 (dritter): a — die Zeitachse zeigt beide
    Fenster:** bei Loop an und Überlappung größer 0 sind die ersten und die
    letzten Sekunden getönt (Lineal und alle Zeilen, ein Strich an der
    Grenze); im Fenster läuft die rote Marke vorn und eine zweite, blassere
    hinten; *Fade in* / *Fade out* der ganzen Komposition stehen als
    schmaler Keil im Lineal (auch bei Loop aus). Verworfen: b — nur das
    Fenster am Ende, eine Marke, keine Keile; c — nichts in der Achse.
    **Von mir gesetzt:** die Grenze des Fensters ändert zunächst nur das
    Zahlenfeld, nicht ein Ziehen in der Achse.
    **Entscheid Patrik S99 (vierter): einstellbar, zwei Haken je
    Komposition** — für einen Clip, der im vorderen und im hinteren Fenster
    zugleich liegt (ein Hintergrund-Visual, ein Song über die ganze Länge):
    **Songs overlap themselves** (Vorgabe an): der Song bekommt im Fenster
    eine zweite Quelle, sein Ende klingt aus, während sein Anfang spielt.
    **Visuals overlap themselves** (Vorgabe aus): aus = der Clip läuft
    einmal durch, ohne Neustart, im Fenster gilt der höhere der zwei Pegel;
    an = ein zweites, frisch gestartetes Exemplar wird darübergemischt.
    Verworfen: eine Auswahlliste mit drei Einträgen; ein Haken je Clip
    (lässt sich später ergänzen). **Heute** (gelesen): je Clip ein Renderer
    (`Wiedergabe::sichere`, Schlüssel Kanal / Clip) und je Song-Clip eine
    Quelle; ein Bild-Clip über die ganze Länge behält über den Sprung
    seinen Renderer, ein Song wird zurückgesetzt. **Reihenfolge (Freigabe
    Patrik S99):** der Haken für das Bild wird als letzter Schritt gebaut
    und steht bis dahin nicht im Panel — er braucht ein zweites Exemplar
    desselben Clips und ist der größte Einzelteil.
    **Bauplan:** (1) Modell und Datei ✅, (2) Uhr und Bildquelle ✅,
    (3) Bild ✅, (4) Ton ✅, (5) Panel und Zeitachse ✅, (6) Blende der
    ganzen Komposition ✅, (7) Visuals overlap themselves ✅ — **alle
    sieben Schritte gebaut (S99), im Programm nichts davon gesehen oder
    gehört.**
    **Visuals overlap themselves (Schritt 7, gebaut S99):** die Wiedergabe
    führt neben den Läufen je Clip (`m_laeufe`) zweite Exemplare
    (`m_frische`). Ein Bild-Clip in beiden Fenstern bekommt bei gesetztem
    Haken im Vorlauf vor dem Umschlag ein frisches Exemplar (vorgewärmt),
    im Fenster werden beide gemischt — das alte unten mit dem Pegel des
    Endes, das frische darüber mit dem des Anfangs. Hinter dem Fenster
    läuft **eines** weiter: das, dessen letzte Stelle im Clip der jetzigen
    am nächsten liegt (`Lauf::imClipMs`) — nach einer Runde das frische (es
    rückt nach `m_laeufe`), nach „Loop aus" im Fenster das alte. **Von mir
    gesetzt:** der festgehaltene Clip (Sync on Effect Chain) läuft immer
    nur einmal; beim Bearbeiten (`ersetzeKomposition`) gehen zweite
    Exemplare verloren und entstehen im Fenster neu. **Nicht gemessen:**
    was das zweite Laden kurz vor dem Umschlag an Bildzeit kostet, und der
    Grafikspeicher auf der schwächeren Karte.
    **Die Pegelkurve der ganzen Komposition (Entscheide Patrik S100 nach
    dem Sichttest S99, F7): c — alles, was ein Clip hat; F7-1 a — ein Stück
    liegt an fester Zeit.** Die Komposition trägt zu den zwei Dauern je eine
    Form der Blende (`einblendeForm`, `ausblendeForm`, Vorgabe linear),
    einen `grundpegel` und `gesamtStuecke` (`GesamtStueck`: Mitte in
    Millisekunden ab Anfang, Weite, gekippt, Form). Gerechnet wird wie am
    Clip: `Komposition::kurveAlsClip` legt die Kurve als Clip über
    [0, Länge) — die Lage eines Stücks wird dabei zum Anteil —, und
    `blendeBei` fragt dessen `pegelBei`. Ein Stück, dessen Mitte hinter dem
    Ende liegt, wirkt nicht und bleibt gespeichert; `setzeKurveAusClip`
    nimmt eine bearbeitete Kurve zurück und lässt solche Stücke stehen. In
    der Datei: `fade { inShape, outShape }` (nur wenn nicht linear) und
    `level { base, shapes [ { at, width, flipped, shape } ] }` — ein älterer
    Stand überliest beides, eine Datei aus S99 liest sich als lineare
    Blenden. **Geändert gegenüber S99:** überdecken sich Ein- und
    Ausblendung, teilen sie den Platz wie am Clip (bisher galt der kleinere
    Wert). **Von mir gesetzt:** ein Stück, das über den Anfang oder das Ende
    ragt, rückt hinein, und Teile, die sich überdecken, werden gestaucht —
    beides wie am Clip, damit Zeitachse und Curve Editor dasselbe zeigen,
    was wirkt. **Gebaut, Schritt 1 (Modell und Datei).**
    **Gebaut, Schritt 2 (Zeile MASTER, Griffe, Kontextmenü, Curve Editor,
    Handbuch — die geplanten Schritte 2 bis 4 in einem, weil Curve Editor
    und Griffe am angeklickten Clip der Zeitachse hängen):** unter dem
    Lineal liegt die Zeile **MASTER** (`Zeitachse::kKanaeleOben`; die
    Kanäle beginnen eine Zeile tiefer). Die Kurve ist dort EIN Clip über
    die ganze Länge mit dem „Kanal" `Zeitachse::kMaster` und dem Clip 0:
    `clipIn` / `schreibeClip` holen und schreiben ihn über `kurveAlsClip` /
    `setzeKurveAusClip`, sodass Zeichnen, Griffe (Blenden, Bézier, Stücke),
    Rechtsklick (Presets, *Edit … curve...*, *Add shape here*, *Delete
    shape*), Doppelklick und der Curve Editor dieselben Wege gehen wie am
    Clip. Nicht an ihr: Verschieben, Kanten, Kopieren, Ausschneiden,
    Löschen, Mischart, Sync on Effect Chain. Die Felder *Fade in* / *Fade
    out* bleiben und stellen dieselben Dauern; **die Keile im Lineal sind
    entfallen**. **Von mir gesetzt:** Name „MASTER", Ort direkt unter dem
    Lineal, immer sichtbar, so hoch wie eine Kanalzeile (das Panel braucht
    36 Pixel mehr Höhe); der Curve Editor nennt sie „Composition" und sein
    Feld weiter „Base level of the clip". **Im Programm nicht gesehen.**
    **Die Blende der ganzen Komposition (Schritt 6, gebaut S99):** der
    Pegel jedes Clips wird mit `Komposition::blendeBei` an seiner Stelle
    malgenommen — im Bild (`Wiedergabe::rendere`) und im Ton
    (`Tonfuehrung::takt`), im Fenster je Stelle, sodass Ende und Anfang
    überblenden. Sie gilt immer, auch in jeder Runde einer Schleife ohne
    Überlappung. Felder **Fade in** / **Fade out** im Panel (nur bei Edit,
    über Undo), je ein Keil unten im Lineal. **Von mir gesetzt:** linear;
    die Karaoke-Ebene blendet nicht mit (sie liegt über dem Gemischten);
    der laufende Song ist die Stimme mit dem höchsten Produkt aus
    Pegelkurve und Blende — im Fenster wechselt er in der Mitte der
    Überblendung.
    **Panel und Zeitachse (Schritt 5, gebaut S99):** unter dem Fuß des
    Panels eine Zeile mit **Loop overlap** (Sekunden, der eingestellte
    Wert) und **Songs overlap themselves**; beide nur bei Edit, über Undo
    (`Change the loop overlap` …). Die Zeitachse tönt bei Loop an die zwei
    Fenster (so weit die Überlappung wirkt), mit gestrichelten Grenzen über
    den Clips, und zeichnet die zweite Marke blasser (`Zeitachse::
    setSchleife`, `Sitzung::zweiteStelleMs`). **Seitdem wirkt die
    Überlappung im Programm.** Die Felder *Fade in* / *Fade out* und die
    Keile kommen mit Schritt 6, der Haken für das Bild mit Schritt 7 — ein
    Feld erscheint erst, wenn es wirkt.
    **Der Ton im Fenster (Schritt 4, gebaut S99):** `Tonfuehrung::takt`
    bekommt die `Schleifenlage` und führt **Stimmen** — jeder Ton-Clip an
    seiner Stelle der Zeitachse: an der vorderen, im Fenster auch an der
    hinteren, und im Vorlauf vor dem Umschlag die ersten Songs der nächsten
    Runde (geöffnet, angehalten). Ein Song in beiden Fenstern hat zwei
    Stimmen und zwei Quellen (*Songs overlap themselves*, Vorgabe an); ohne
    den Haken eine — er spielt sein Ende aus und springt hinter dem Fenster
    im Song zurück. Eine Quelle bleibt bei der Stimme ihres Clips, deren
    Stelle im Song ihr am nächsten liegt; ob eine Stimme gesprungen ist,
    wird **je Quelle an der Stelle im Song** gemessen, nicht mehr an der
    Uhr. Folge: **der Umschlag der Schleife ist kein Sprung mehr** — die
    Quelle, deren Song weiterläuft, wird nicht neu gesetzt. Das gilt auch
    ohne Überlappung: ein Song über die ganze Länge bekommt kurz vor dem
    Schleifenende eine zweite Quelle und beginnt nahtlos (bisher wurde die
    eine Quelle zurückgesetzt). Der laufende Song ist wie bisher die Stimme
    mit dem höchsten Pegel; am Umschlag desselben Songs gibt es keinen
    Songwechsel.
    **Das Bild im Fenster (Schritt 3, gebaut S99):** `Wiedergabe::rendere`
    bekommt die zweite Stelle (`Schleifenlage`) und mischt die Clips beider
    Stellen. Liegen in einem Kanal zwei Clips zugleich, liegt der des Endes
    unten und der des Anfangs darüber (**von mir gesetzt** — der Neue kommt
    obenauf, wie ein höherer Kanal). Ein Clip in beiden Fenstern läuft
    einmal, mit einem Renderer, und bekommt den höheren der zwei Pegel.
    **Vorwärmen über den Umschlag:** die ersten Clips der nächsten Runde
    bekommen ihren Renderer im Vorlauf vor dem Umschlag — das gilt für
    jede Schleife, auch ohne Überlappung (bisher begannen sie am
    Schleifenanfang kalt).
    **Die Uhr mit Überlappung (Schritt 2, gebaut S99):** ein Umlauf dauert
    Länge weniger Überlappung (`Uhr::umlaufMs`). `stelleMs` ist die
    **vordere** Stelle (die rote Marke), `zweiteStelleMs` die im hinteren
    Fenster, die zugleich läuft, sonst −1. Eine zweite Stelle gibt es nur,
    wenn schon ein Ende da ist, das hineinspielt: nicht in der ersten
    Runde, nicht nach einem Sprung ins vordere Fenster; ein Sprung ins
    hintere Fenster zeigt beide. Halt im Fenster, Bearbeiten, Composer aus /
    an und der Stand für den Neustart merken die hintere Stelle
    (`rueckkehrMs`) — danach laufen wieder beide. Schleife aus im Fenster:
    die Komposition spielt ihr Ende aus. Ohne Schleife und mit
    Überlappung 0 verhält sich die Uhr wie bisher. Die Bildquelle spiegelt
    beide Stellen und rechnet sie auf jetzt hoch (`standStellenJetzt`).
    **Von mir gesetzt:** das Verhalten bei Sprung, Halt und Schleife aus im
    Fenster — im Programm zu beurteilen, sobald Bild und Ton folgen.

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

- **Ein Clip-Typ für Bild und Ton** (Entscheide Patrik S96, C5): ein
  Ton-Clip hat **Start und Ende wie ein Bild-Clip** und **dieselbe
  Pegelkurve** — Grundpegel, Ein- und Ausblendung, Stücke, mit denselben
  Presets; der Pegel ist am Ton die Lautstärke, **voller Pegel = die
  Lautstärke des Player-Panels**. Was ein Clip ist, sagt sein Kanal
  (`Kanal::art`, eine Liste `Kanal::clips` für beide). Nur am Ton: der
  **Einsatz** im Song (`einsatzMs`) und die **Länge des Songs** (`quellMs`).
  Beim Hineinlegen ist der Clip so lang wie der Song; die rechte Kante nach
  links schneidet ab (dahinter Stille), die linke Kante ändert den Einsatz —
  was bleibt, ist an derselben Stelle zu hören; über Anfang oder Schluss des
  Songs hinaus lässt sich nicht ziehen. Am Nachbarn gelten dieselben drei
  Arten wie am Bild; ein Clip wechselt nur in einen Kanal seiner Art. Das
  Ende der Komposition folgt dem letzten Clip, gleich ob Bild oder Ton.
  In der Datei: `end`, `offset`, `sourceLength` und die Felder der
  Pegelkurve; ein Ton-Clip ohne `end` (bis S95) reicht bis zum Schluss
  seines Songs.
- **Ein- und Ausblenden** sind eigene Teile des Clips (seit S96, §9) — bis
  S95 die ersten und letzten Punkte einer Punktliste. Ein Clip ohne Teile
  läuft auf seinem Grundpegel.
- Clips eines Kanals **überlappen nicht**; Überlagerung entsteht nur zwischen
  Kanälen.
- **Trifft ein gezogener Clip auf einen Nachbarn** (Entscheid Patrik S95,
  Stufe C3a) — drei Arten, gewählt mit der Taste beim Ziehen:
  - **ohne Taste — anhalten:** eine Kante lässt sich nur bis an den Nachbarn
    ziehen; ein verschobener Clip landet in der nächstgelegenen Lücke, in die
    er ganz passt, bündig am Nachbarn. Nichts außer dem angefassten Clip
    ändert sich.
  - **Ctrl — kürzen:** der gezogene Clip gewinnt. Ein Nachbar, den er
    überdeckt, wird gekürzt; ein ganz überdeckter verschwindet. Liegt der
    gezogene Clip mitten in einem Nachbarn, bleibt von diesem der vordere Teil.
  - **Ctrl+Shift — schieben:** die Nachbarn rücken mit und schieben ihrerseits
    weiter; ihre Länge bleibt. Am Anfang der Zeitachse ist Schluss.
- **Die Pegelkurve beim Ziehen einer Kante** (Entscheid Patrik S96, §9):
  die Einblendung bleibt am Anfang, die Ausblendung am Ende, Stücke an ihrem
  Anteil der Länge; alle behalten ihre eingestellte Weite. Reicht der Platz
  nicht, werden die sich berührenden Teile gestaucht — errechnet, nicht
  gespeichert. Beim Verschieben wandert die Kurve unverändert mit.
- **Einrasten** (Bedienung): auf die Kanten anderer Clips (aller Kanäle) und
  die Abspielmarke, sonst auf das Raster der Datei; mit Alt frei.
- **Das Ende der Komposition** (Befund und Vorgabe Patrik S95): es **folgt
  dem letzten Clip** — wird er kürzer, verschoben oder gelöscht, kommt das
  Ende mit, und Loop bzw. die Einstellung zum Ende greifen dort. Das ist die
  Vorgabe (`length` 0 oder nicht gesetzt). Mit **Fixed end** im Panel steht
  das Ende fest (`length` in Millisekunden): die Komposition läuft bis
  dorthin, auch wenn kein Clip mehr liegt; die leere Strecke ist schraffiert,
  das Ende trägt eine eigene Marke. Ein festes Ende wächst nur, wenn ein Clip
  darüber hinausgezogen wird. **Beim Lesen** gilt eine Länge, die genau das
  Ende des letzten Clips ist, als „folgt" — sie sagt nichts Eigenes (so
  stand es in `composer_demo.lvcomp`, und so entstand der Befund).
- **Kanal wechseln** (Wunsch Patrik S95): ein Clip, der beim Verschieben in
  eine andere Zeile gezogen wird, zieht in diesen Kanal um — mit derselben
  Regel am Nachbarn und denselben Tasten.
- Zeiten in Millisekunden. Ein Raster (Einrasten beim Ziehen) ist Bedienung,
  nicht Datenmodell.
- **Nicht in der Datei:** ob der Composer an ist, ob er gesperrt ist, die
  Stelle der Abspielmarke — das sind Einstellungen (`composer/…`).

---

## 7. Das Panel

**View → Panels → Composer**, eigenes andockbares Panel, breit gedacht.

**Einfügemarke, Kopieren und Einfügen, Schatten (Wünsche Patrik nach dem
Hörtest S98, gebaut S98 — eigener Code bis auf das Zahlenfeld in Settings,
Naht N9):**

- **Einfügemarke** (Entscheid Patrik S98: a — eigene Marke statt der
  Abspielmarke): ein Klick auf eine freie Stelle einer Kanalzeile setzt sie,
  im Bearbeiten; sie rastet ein wie ein Clip (Alt frei). Gezeichnet als
  Strich mit Dreieck in ihrer Zeile, die Zeile leicht hervorgehoben; die
  Clip-Zeile unten nennt Kanal und Zeit. Die Abspielmarke bleibt, wo sie
  ist. Verworfen: die Abspielmarke als Ziel (sie wandert im Lauf, und ein
  Klick ins Lineal lässt die Musik springen).
- **Kopieren / Ausschneiden / Einfügen** (Ctrl+C / Ctrl+X / Ctrl+V bei
  angeklicktem Clip, dazu im Rechtsklick-Menü *Copy clip*, *Cut clip*,
  *Paste clip here*): die Ablage trägt einen Clip mit allem — Länge,
  Pegelkurve, Mischart, Einsatz im Song — und gilt für das ganze Programm.
  Eingefügt wird mit dem **Anfang an der Einfügemarke**, nur in einen Kanal
  der Art des Clips; liegt dort schon einer, in die nächste Lücke, in die er
  ganz passt (`Bearbeiten::fuegeKopieEin` — er wird nicht gekürzt). Über
  Undo. **Von mir gesetzt, von Patrik im Sichttest S98b gebilligt (S99;
  D2, D3, C12 — ohne Bemerkung abgehakt):** nach dem Einfügen rückt die
  Marke hinter den neuen Clip; ohne Marke landet die Kopie hinter dem
  angeklickten Clip; eine Kopie wird nicht gekürzt. Bei der falschen Art
  geschieht nichts, ohne Hinweis (C10, ebenso abgehakt). **Weiter nur von
  mir gesetzt:** Kopieren geht auch ohne Edit (stand nicht in der Liste).
- **Schatten beim Hineinziehen** (Wortlaut Patrik: „in den passenden Kanälen
  an der voraussichtlichen Einfügestelle ein Schatten des Clips"): solange
  ein Drag aus Effect Browser, Audio Browser, Playlist oder Explorer über
  der Achse steht, zeichnet sie jeden Clip dort, wo er landen würde — in
  seiner Länge, mit einem Strich am Anfang. Der Schatten kommt aus
  demselben Code wie das Fallenlassen (`Zeitachse::fallSchatten` rechnet es
  auf einer Kopie), er kann also nicht danebenliegen. Über einer Zeile der
  anderen Art gibt es keinen, und der Drag wird dort abgelehnt.
- **Die Zeit unter dem Zeiger:** im Lineal zeigt ein Tooltip die Zeit an der
  Stelle der Maus — nur im Lineal, nicht über den Zeilen (im Sichttest
  S98b ohne Einwand, C1).
- **Vorgabelänge neuer Bild-Clips** in Settings (*New clip length*, 1 bis
  600 s, Vorgabe 20) — Naht N9.
- **Im Programm gesehen (Sichttest S98b, S99 ausgewertet):** der Schatten
  während eines echten Drags (C13 bis C17; der Unit-Test fragt nur
  `fallSchatten` und schickt kein Ziehen), der Tooltip (C1), die Kürzel
  neben den Kürzeln des Hauptfensters (C5 bis C9), die Vorgabelänge (C18).
  Aussehen von Marke und Schatten: gut zu unterscheiden von der
  Abspielmarke (D4).
- **Wellenform beim Kürzen eines Song-Clips:** sie wird abgeschnitten, nicht
  zusammengeschoben (Patrik S99 zu D5) — die Beobachtung aus dem Hörtest
  S98 ist damit erledigt.

**Zoom und Rollen der Zeitachse (Vorgabe Patrik S101 / S102: „Zoom in x und
y, wobei beide zusammen wie auch einzeln gezoomt werden können"; neun
Entscheide Patrik S102, ZO1 bis ZO9 — gebaut sind Schritt 1, das Rollen
(ZO4), Schritt 2, der Zeit-Zoom, Schritt 3, der Höhen-Zoom, Schritt 4,
Schwenken und Zurücksetzen, und Schritt 5, das Mitlaufen samt der Auswahl
in Settings (Naht N9); im Programm nicht gesehen):**

- **Mausrad (ZO1: a):** Rad allein rollt senkrecht (wie bisher), Shift+Rad
  waagrecht. **Ctrl+Rad zoomt die Zeit, Alt+Rad die Zeilenhöhe, Ctrl+Alt+Rad
  beides.** **Ctrl+0** setzt beides zurück (Vorgabe Patrik S100) — die ganze
  Länge auf der Breite, die Zeilen in Normalhöhe; das Kürzel gilt, solange
  das Panel den Fokus hat. Verworfen: Ctrl+Rad als Lupe für beides (b), Rad
  ohne Taste als Zeit-Zoom (c).
- **Master-Zeilen (ZO2: a):** sie wachsen und schrumpfen mit den Kanälen,
  immer 60 % der Kanalhöhe (das Urteil M17 ist ein Verhältnis). Verworfen:
  feste Höhe (b), eigener Zoom (c).
- **Bereich der Höhe (ZO3: a; Zusatz Patrik: die Höhe muss auch schrumpfen
  können, um mehr Kanäle zu sehen):** 50 % bis 300 % (Kanal 18 bis 108
  Pixel, Master-Zeile 11 bis 65). Passt ein Name nicht mehr in seine Zeile,
  wird er nicht gezeichnet, der Tooltip nennt ihn weiter. In Zeilen unter 24
  Pixeln lassen sich die Punkte der Pegelkurve nicht ziehen; Clips
  verschieben und an den Kanten ziehen geht weiter. Verworfen: 25 % (b),
  67 % (c).
  **Gebaut S102 (Schritt 3):** `zeilenProzent` / `setZeilenProzent`,
  `kanalHoch`, `masterHoch` (immer 60 % von `kanalHoch`, ZO2);
  **Alt+Rad** ändert die Höhe um 10 % je Raste um den Mauszeiger (die
  Zeile unter ihm bleibt stehen), **Ctrl+Alt+Rad** Zeit und Höhe zusammen;
  ein feines Rad sammelt, bis es eine Raste ist. Die Höhe merkt das Panel
  in `composer/rowHeightPercent` (ZO9) — die Achse selbst kennt keine
  Einstellungen. Ein Name, der nicht passt (die Zeile ist niedriger als
  die Oberlänge der Schrift), bleibt weg und steht im Tooltip
  (`versteckterName`). **Abweichung vom Wortlaut, von mir gesetzt:** die
  Schwelle „unter 24 Pixeln" gilt für die **Kanalhöhe** und dann für alle
  Zeilen — also bei 50 % und 60 % gibt es nirgends Punkte der Kurve zu
  greifen, ab 70 % überall. Wörtlich je Zeile genommen wären die
  Master-Zeilen schon in Normalhöhe (21 Pixel) gesperrt gewesen; ihre
  Griffe gibt es seit S100, und sie bleiben. Den Curve Editor öffnet der
  Doppelklick in jeder Höhe. **Alt und die Menüleiste:** Alt allein
  losgelassen schaltet unter Windows die Menüleiste auf die Tastatur; das
  Rad hält Qt davon nicht ab. Die Achse schickt deshalb nach Alt+Rad eine
  Mausbewegung an ein Objekt ohne Wirkung — per Test mit Gegenprobe
  belegt, am echten Hauptfenster nicht gesehen. **Wie Windows das Rad mit
  Alt meldet** (senkrecht oder quer), ist ohne Gerät nicht zu prüfen
  (die Quellen von Qt sind nicht installiert): die Achse nimmt die Achse
  des Rads, die sich bewegt — beides ist im Test.
- **Was beim Rollen stehen bleibt (ZO4: a):** das Zeitlineal bleibt oben
  stehen, die Master-Zeilen rollen mit den Kanälen; die Spalte mit den
  Kanalnamen bleibt links stehen (von mir gesetzt). Rollbalken in beiden
  Richtungen (Wunsch Patrik S99, bestätigt S102). Die Zeitachse führt ihr
  Rollen dafür selbst, statt in einer `QScrollArea` zu liegen — eigener
  Code, keine Naht. **Gebaut S102 (Schritt 1), senkrecht:** `Zeitachse`
  trägt `rollY` / `setRollY` / `rollYMax` / `inhaltHoehe` und meldet
  `rollenGeaendert`; jede Stelle, die eine Zeile aus der Höhe rechnet, geht
  über `zeileBei` und `zeileOben` (das Lineal liefert `kKeineZeile` und
  deckt, was darunter rollt). Gezeichnet wird in zwei Gängen mit je einem
  Ausschnitt — erst die Zeilen, dann das Lineal. Der Rollbalken sitzt im
  neuen `ZeitachsenRahmen` und erscheint nur, wenn nicht alle Zeilen ins
  Fenster passen; das Mausrad ohne Taste rollt 60 Pixel je Raste. Eine
  andere Komposition beginnt oben.
  **Gebaut S102 (Schritt 2), der Zeit-Zoom:** ohne Zoom zeigt die Achse
  die ganze Länge auf der Breite („ganz", `zeitGanz`) — gerechnet wie bis
  S101. Hineingezoomt steht der Maßstab fest (`msJePixel`), auch wenn die
  Komposition wächst oder das Fenster breiter wird; passt dann wieder
  alles hinein, ist sie wieder ganz. **Ctrl+Rad** zoomt um den Mauszeiger
  (`zoomeZeit`, Faktor 1,25 je Raste, bis 1 Millisekunde je Pixel),
  **Shift+Rad** rollt waagrecht (60 Pixel je Raste; auch ein Rad, das von
  sich aus quer rollt). `rollX` / `setRollX` / `rollXMax`; der waagrechte
  Rollbalken sitzt unter der Achse im `ZeitachsenRahmen` und erscheint
  nur hineingezoomt. Die Spalte der Kanalnamen bleibt stehen: alles, was
  an einer Zeit hängt, wird nur rechts von ihr gezeichnet, und ein Clip,
  der darunter gerollt ist, wird dort nicht getroffen. Gezeichnet wird nur
  das sichtbare Stück (Striche, Clips, Wellenform, Pegelkurve) — beim
  feinsten Maßstab im Test 6 ms. Das Lineal teilt sich bis auf
  Zehntelsekunden und beschriftet dann mit der Zehntelsekunde. Eine
  geladene Komposition beginnt ganz (ZO9), die bearbeitete Fassung (auch
  nach Undo) behält den Zoom. **Von mir gesetzt:** der Name eines Clips
  bleibt am linken Rand stehen, wenn sein Anfang hinausgerollt ist; ein
  Klick ins Lineal über der Spalte der Kanalnamen springt an die erste
  sichtbare Stelle (ohne Zoom wie bisher an den Anfang), ebenso das
  Fallenlassen dort.
  **Grenze, nicht gebaut:** wird ein Clip an den Rand gezogen, rollt die
  Achse nicht von selbst mit (das Mausrad rollt auch im Zug). Verworfen: auch die Master-Zeilen fest (b — bei 300 %
  nähmen sie mit dem Lineal 284 Pixel), alles rollt wie bisher (c).
- **Festpunkt (ZO5: a):** gezoomt wird um den Mauszeiger — die Zeit unter
  dem Zeiger bleibt beim Zeit-Zoom an ihrer Stelle, die Zeile unter dem
  Zeiger beim Höhen-Zoom. Verworfen: die Abspielmarke (b), je nach Ort des
  Zeigers (c).
- **Mitlaufen (ZO6: Patrik — „a und c einstellbar in den Settings, c
  default"):** der Schalter **Scroll with playhead** im Kopf, Vorgabe an.
  Die Art steht in Settings: **stetig** (Vorgabe — die Abspielmarke bleibt
  in der Mitte, die Clips ziehen unter ihr durch) oder **seitenweise**
  (erreicht die Marke den rechten Rand, blättert der Ausschnitt eine Seite
  weiter). Solange eine Maustaste in der Zeitachse gedrückt ist, steht der
  Ausschnitt. Die Einstellung ist die **Naht N9, sie wächst** (§12.2).
  Verworfen: ohne Schalter (b), gar nicht mitlaufen (d).
  **Gebaut S102 (Schritt 5, das Eigene):** `Zeitachse::setMitlaufen`
  (`Aus`, `Stetig`, `Seitenweise`) — das Panel setzt es je Takt: `Aus`,
  wenn der Schalter aus ist, nichts läuft oder die ganze Länge dasteht.
  Gefolgt wird also **nur bei laufender Komposition**; ein Sprung im Stand
  lässt den Ausschnitt, wo er ist. Stetig: die Marke in der Mitte, am
  Anfang und am Ende der Komposition läuft sie selbst. Seitenweise:
  verlässt die Marke den Ausschnitt, steht sie danach an seinem linken
  Rand. Gehalten wird, solange eine Maustaste in der Achse gedrückt ist
  **oder ein Rollbalken gezogen wird** (von mir ergänzt — die Marke zöge
  ihn sonst zurück). Schalter und Art liest und schreibt `Sitzung`
  (`composer/scrollWithPlayheadOn`, Vorgabe an; `composer/scrollWithPlayhead`,
  0 = stetig); der Schalter bleibt über den Neustart (von mir gesetzt).
  **Platz, anders als angekündigt:** *Scroll with playhead* und *Fit*
  stehen rechts in der **Datei-Zeile** — in der ersten Zeile machten sie
  das Panel breiter; seine schmalste Breite ist so 743 Pixel (gemessen im
  Test). **Gemessen (die offene Frage zu ZO6):** die Achse zeichnet auch
  ohne Mitlaufen je Takt neu, weil die Marke wandert — das Mitlaufen
  kommt nicht dazu, es ersetzt nur, was sich bewegt. Im Test (1600 × 600
  Pixel, zwölf Kanäle, acht davon Songs mit Wellenform, mit `grab`):
  rund 11 ms je Bild ohne Mitlaufen, rund 10 ms stetig, bei 30 Takten je
  Sekunde. Neben dem Renderer im Programm nicht gemessen. **Grenze:** der
  Takt des Panels ist 30-mal je Sekunde — flüssiger als das läuft der
  Ausschnitt nicht.
- **Schwenken (ZO7: a):** mit gedrückter mittlerer Maustaste in beide
  Richtungen. Verworfen: linke Taste auf leerer Fläche (b), Alt + linke
  Taste (c), kein Schwenken (d); beide Tasten zusammen (Frage Patrik) —
  die zuerst gedrückte löst schon ihre eigene Wirkung aus.
  **Gebaut S102 (Schritt 4):** der Inhalt folgt der Maus bis an die
  Grenzen, der Zeiger zeigt die geschlossene Hand; mitten in einem Zug an
  einem Clip, am Lineal oder an einem Kanalnamen schwenkt die mittlere
  Taste nicht.
- **Im Kopf (ZO8: a):** der Knopf **Fit** — setzt zurück wie Ctrl+0, sein
  Tooltip nennt die Gesten; dieselben Zeilen im Rechtsklick-Menü der
  Zeitachse unter *Zoom*. Verworfen: zwei Schieber (b), nichts im Kopf (c).
  **Gebaut S102 (Schritt 4):** `Zeitachse::setzeZoomZurueck` (die ganze
  Länge, Normalhöhe); der Knopf **Fit** sitzt rechts in der Datei-Zeile
  (bis Schritt 5 kurz hinter *Mute* — dort machte er das Panel breiter), sein
  Tooltip trägt die sechs Zeilen aus `Zeitachse::zoomGesten`. **Ctrl+0**
  führt das Panel als `QShortcut` für sich und seine Kinder — es gilt,
  solange der Fokus irgendwo im Panel sitzt, und ist kein Kürzel des
  Hauptfensters (keine Naht). Das Rechtsklick-Menü der Zeitachse hat das
  Untermenü *Zoom* (*Fit* mit dem Kürzel, grau, wenn nichts verstellt
  ist; darunter die Gesten zum Nachlesen, grau). **Von mir gesetzt:** das
  Menü gibt es jetzt auch ohne *Edit* — dann nur mit *Zoom*.
- **Was gemerkt wird (ZO9: a):** die Zeilenhöhe merkt die App (über den
  Neustart, für jede Komposition); der Zeit-Zoom nicht — nach dem Laden
  zeigt die Zeitachse die ganze Länge. In der `.lvcomp` steht nichts davon.
  Verworfen: beides in der Komposition (b), nichts merken (c).
- **Von mir gesetzt, nicht entschieden** (als Urteile in den Sichttest
  S102): Zeit-Zoom von „ganze Länge auf der Breite" bis 1 Millisekunde je
  Pixel, Faktor 1,25 je Raste; Höhe in Schritten von 10 % je Raste; die
  Schwelle von 24 Pixeln; die Teilung des Lineals verfeinert sich bis auf
  Zehntelsekunden; die Namen **Scroll with playhead** (nicht **Follow
  player** — der koppelt Start und Stopp, §4) und **Fit**, ihr Platz im
  Kopf.
- **Nicht dabei:** der Curve Editor bleibt, wie er ist.

- **Kopf:** Schalter **Composer on**, Schalter **Lock**, Transport (Start,
  Stopp, an den Anfang), Schalter **Follow player**, Stelle als Zeit, Zoom
  (S102: der Schalter **Scroll with playhead** und der Knopf **Fit**, siehe
  oben), Datei-Zeile (New, Load…, Save, Save As…).
- **Zeitlineal** mit Abspielmarke; Klick springt.
- **Kanäle** als Zeilen, links je Kanal Name und an/aus — in der Reihenfolge
  der Datei, **CH1 oben** (Entscheid Patrik S95, wie Spur 1 in FL Studio); im
  Bild liegt der erste Kanal zuunterst, jeder weitere darüber. Clips als Blöcke:
  ziehen = verschieben, Kanten ziehen = Start und Ende, Entf = löschen.
- **Pegelkurve** im Clip sichtbar; Punkte anfassen und ziehen
  (der „Übergangs-Editor" aus S88).
- **Ziehen hinein:** Presets aus dem Effect Browser in einen Visual-Kanal,
  Songs aus Audio Browser und Playlist in den Audio-Kanal.
- **Audio-Kanal:** die Wellenform des Songs im Clip (wie in Lyrics Sync).
  **Gebaut S96 (C5):** Rechtsklick → *Add audio channel* (`audioKanalHinzu`);
  Songs fallen nur in Audio-Kanäle, Presets nur in Visual-Kanäle, unter der
  letzten Zeile entsteht ein Kanal der Art der ersten Datei (`lasseFallen`,
  `fallDateien`); Ton-Clips tragen eine eigene Farbe und die Wellenform, so
  hoch, wie der Pegel sie lässt. Die Zeitachse kennt keine Audio-Engine: das
  Panel reicht Endungen, Song-Längen (`probeDurationMs`) und Hüllkurven
  (`probePeaks`, ein Song nach dem anderen in einem eigenen Thread) herein.
  Die Clip-Zeile nennt am Ton den Einsatz im Song; *Blend* ist dort nicht zu
  wählen. Griffe, Curve Editor und Presets gelten am Ton-Clip wie am Bild.
- **Rechtsklick** auf Clip, Kanal und freie Fläche — wie im Kettenbaum.

**Das Panel Effect Chain bei laufender Komposition** (Entscheid Patrik S95: a,
mit Ergänzung):

- **C2b:** Effect Chain bleibt an der Kette der App, unverändert. Das
  Composer-Panel zeigt zum angeklickten Clip Datei, Start, Ende und Mischart
  in einer eigenen Zeile.
- **C3c:** das Composer-Panel bekommt den Schalter **Sync on Effect Chain**
  (Entscheid Patrik S95: a — live ändern im Zusammenhang).
  **An:** Effect Chain hängt am Renderer des Clips, der im Composer
  angeklickt ist; der Composer hält diesen Renderer dafür am Leben.
  Änderungen wirken sofort im gemischten Bild. **Aus:** Effect Chain ruht,
  solange eine Komposition das Bild liefert.
  - **Binden braucht keine Naht** (Befund S95): Effect Chain und Visualizer
    Config folgen dem Ereignis `VisualizerChangedEvent`; der Composer meldet
    darüber den Renderer des Clips, „keinen" (ruht) oder wieder die Kette der
    App.
  - **Speichern** (Entscheid Patrik S95): über das gewohnte Speichern von
    Effect Chain — File → Save Effect Chain / Save As..., Ctrl+S, die
    Preset-Zeile. Solange Effect Chain an einem Clip hängt, ist es **immer ein
    Speichern unter**: das Original bleibt unberührt. Der Clip zeigt danach
    auf die neue Datei. Benutzen weitere Clips der Komposition dieselbe alte
    Datei, fragt ein Dialog mit einer Liste zum Ankreuzen, welche ebenfalls
    auf die neue Datei wechseln. Das braucht eine Naht in
    `MainWindow::speichereKette` (ein Aufruf am Anfang) — sie kommt vor dem
    Bau in §12.2.
  - **In drei Schritten:** (0) beim Bearbeiten behält ein Clip seinen
    Renderer ✅ S95 — sonst ginge jede ungespeicherte Änderung an seiner Kette
    mit dem nächsten Zug verloren; (1) festhalten, binden, ruhen; (2)
    Speichern unter und Umhängen der Clips.

**Transport im Kopf** (Wunsch Patrik S95): Play, Pause, Stopp und ein Knopf
**Loop** für die ganze Komposition — wie im Panel Player.

**Andocken:** das Panel ist ein Panel wie jedes andere — es lässt sich an
jede Seite docken, in Reiter legen und ablösen. Unten ist nur die Stelle, an
der es beim ersten Öffnen erscheint.

**Schalter Edit** (Wunsch Patrik S95, für C3): Bearbeitet wird nur, wenn der
Schalter **Edit** im Kopf an ist — Clip-Längen, Verschieben, Übergänge und
Mischarten. Aus (Vorgabe) ist das Panel wie in C2b nur zum Ansehen und
Abspielen; ein versehentliches Ziehen ändert nichts. Der Schalter ist nicht
**Lock**: Lock sperrt Kürzel und Ladewege für Vorführungen (§8), Edit sperrt
das Verändern der Komposition.

**Übergänge** (Wunsch Patrik S95, für C4): linear und als **Bézier-Kurve**
mit Anfassern. Heute kennt die Pegelkurve sechs feste Formen je Abschnitt
(linear, S-Kurve, Ease in, Ease out, exponentiell, halten); die Bézier-Form
kommt als weitere dazu, ihre Anfasser als zusätzliche Felder am Punkt — eine
`.lvcomp` ohne diese Felder liest sich wie bisher.

---

## 8. Wer steuert was

| Zustand | Presets | Musik |
|---|---|---|
| **Composer aus** | wie heute: Browser, Preset-Zeile, PgUp/PgDn, Host Switcher | Playlist |
| **Composer an, kein Audiokanal an** | der Composer | Playlist |
| **Composer an, Audiokanal an** | der Composer | der Composer; die Playlist ruht |

**Das Player-Panel bei eingeschaltetem Audiokanal** (Entscheid Patrik S96):
seine Knöpfe steuern den **ganzen Composer** — auch über Hotkey und Menü,
alle laufen durch dieselbe Stelle (`TransportCommandEvent`).

| Im Player-Panel | Wirkung |
|---|---|
| Play / Pause | die ganze Komposition läuft oder hält, auch in einer Lücke ohne Song |
| Stop | die Komposition hält und geht an den Anfang, wie Stop im Composer |
| Next / Previous | Sprung zum Anfang des nächsten bzw. vorigen Songs der Komposition |
| Fortschrittsbalken | Sprung im laufenden Song; die Komposition springt mit (der Ton führt) |
| Lautstärke, Mute | wie immer; der Pegel des Clips kommt obendrauf |

Der Balken bleibt der Balken des Songs; die Zeit der Komposition steht im
Composer-Panel. **Die Playlist ruht**, solange der Audiokanal führt: der
Composer hängt sie am Player aus (`setPlaylist(nullptr)`) und beim
Ausschalten wieder ein — am Titelende spielt dann nichts von selbst weiter,
und ein Doppelklick in der Playlist tut nichts.

**Wechsel zwischen Player und Composer** (Wunsch Patrik nach dem Hörtest
S98, einstellbar in Settings, Vorgabe beide an — die Vorgabe von Patrik im
Sichttest S98b gebilligt, D1; B1 bis B6 bestanden): Spielte der Player, als der
Composer den Ton übernahm, **spielt er von selbst weiter**, wenn der Composer
den Ton wieder abgibt (Composer aus, letzter Audio-Kanal aus) — nicht beim
Beenden der App. Lief die Komposition, als der Composer ausgeschaltet wurde,
**läuft sie beim Einschalten weiter**; eine neu geladene Komposition folgt
der Einstellung „Open Composition". Mit ausgeschalteten Haken bleibt der
Player angehalten bzw. wartet Composer on an der gemerkten Stelle — so war
es bis S98 (das zweite seit S95).

**Ab C5b (Entscheid Patrik S98: a, mit Anpassung — ersetzt den Entscheid b
aus S97; gebaut S98):** Der Play-Knopf des Panels zeigt „Pause", solange
die Komposition läuft — auch in einer Lücke, in der kein Song spielt — und
„Play", wenn sie hält (Entscheid Patrik S98: a). Solange der Composer Ton
führt, zeigt das
Player-Panel **den laufenden Song** (§4): als Titel **„Composer - <Song>"**
(Anpassung Patrik), dazu Künstler, Stelle im Song und Länge. Ein Klick in
den Balken springt in diesem Song; der Composer rechnet daraus die Stelle
der Komposition und springt dorthin. In einer Lücke bleibt der zuletzt
gezeigte Song stehen, der Balken hält. Lautstärke und Mute wirken auf den
ganzen Ton der Komposition. Der Loop-Knopf des Panels ist ausgegraut,
solange der Composer Ton führt (die Schleife steht im Composer-Panel). Am
Kreuzungspunkt einer Überblendung wechselt das Panel mit allem anderen auf
den neuen Song — der Balken springt dort. Die Zeile „Fortschrittsbalken"
der Tabelle bleibt damit im Kern, nur führt jetzt die Uhr: der Klick setzt
die Uhr, der Ton folgt.

*Zur Einordnung, in S97 entschieden (b) und in S98 zurückgenommen (Patrik:
die Fragestellung war anders verstanden):* das Panel zeigt die Komposition —
Dateiname, Zeitachse, Loop-Knopf = Schleife der Komposition. Gebaut wurde
davon nichts. Weiter verworfen: das Panel ruht.

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

**Der Start** (C7, neu gefasst S95 — Patrik): War beim Beenden eine
Komposition im Composer-Panel geladen, lädt der Start sie wieder **ins
Panel**. Was dann läuft, bestimmt der Knopf **Resume after restart** im
Composer-Panel (Vorgabe aus, wie der gleiche Knopf im Panel Player):

- **Knopf aus:** die Komposition liegt im Panel, **der Composer ist aus** —
  das Bild liefert die Effektkette. Im Panel lässt sich alles vorbereiten;
  **Composer on** schaltet die Komposition ins Bild.
- **Knopf an:** der Composer hält beim geordneten Beenden Datei, Stelle,
  „Composer an" und „lief gerade" fest; der Start stellt diesen Stand wieder
  her — lief die Komposition, läuft sie von der letzten Stelle an weiter.
  Nach einem Absturz gilt der Stand des letzten geordneten Beendens.

War keine Komposition geladen, lädt der Start wie seit S93 das zuletzt
laufende Preset. Das Preset der Kette wird in jedem Fall geladen; die
Komposition legt sich bei „Composer on" darüber (Naht R3).

**Geladen ist nicht an** (Folge daraus, S95): Das Panel hält die Komposition,
auch wenn der Composer aus ist. **Composer on** setzt die Bildquelle, aus
nimmt sie weg; die Komposition bleibt im Panel.

**Öffnen von Hand** (Entscheid Patrik S95): Open Composition lädt die Datei,
schaltet den Composer an und **startet die Uhr sofort** — das ist die
Vorgabe. Eine Einstellung in Settings stellt um auf „hält am Anfang" (los
geht es dann mit Play).

**Das Ende, wenn Loop aus ist** (Entscheid Patrik S95: alle drei wählbar,
Vorgabe die erste) — Einstellung in Settings:

- **Halten** (Vorgabe): die Uhr steht am Ende, das Bild ist schwarz, die
  Karaoke-Ebene läuft weiter, der Composer bleibt an. Play beginnt von vorn.
- **An den Anfang:** die Uhr springt an den Anfang und hält, wie Stopp.
- **Composer aus:** der Composer schaltet sich aus, die Effektkette liefert
  wieder das Bild; die Komposition bleibt im Panel, die Stelle am Anfang.

**Play bei ausgeschaltetem Composer** schaltet ihn ein und spielt ab der
gemerkten Stelle (gebaut S95, nicht eigens entschieden). Ein Klick ins
Zeitlineal bei ausgeschaltetem Composer merkt nur die Stelle.

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
| **C2 Panel zum Ansehen** ✅ | Panel mit Lineal, Kanälen, Clips, Abspielmarke, Transport, Schalter Composer on; der Composer als Bildquelle der App | erledigt (C2a S94, C2b S95) — eine von Hand geschriebene `.lvcomp` läuft und ist im Panel zu sehen; Sichttest S95 bestanden |
| **C3 Bearbeiten** | Clips hineinziehen, verschieben, Kanten ziehen, löschen; Kanäle anlegen; Datei-Zeile; Undo | eine Komposition ganz im Panel bauen |
| **C4 Pegelkurven** | Punkte im Clip, Kurvenformen, Mischart je Clip | Übergang und Überlagerung aus der Skizze S88. **Stand S95:** Ein- und Ausblenden mit sieben Formen samt Bézier und die Mischart je Clip sind gebaut; **offen: freie Punkte im Clip** |
| **C5 Audiokanal** | ein Audiokanal: Songs als Clips, Wellenform, führt den Player, Playlist ruht | Visuals an einem Song ausrichten |
| **C6 Steuerung** | Kürzel, Laden bei laufendem Composer, Lock, Start-Wiederherstellung | §8 im Programm |
| später | zweiter Audiokanal (Songs mischen), Takt-Raster, Karaoke-Schalter als Spur, Kompositionen als Video rendern, Parameter über der Zeit | — |

**Der Rückbau wird einmal wirklich durchgespielt, nach C1a** — der ersten
Stufe mit Nähten (§12.4).

**Pegelkurve, nächste Schritte** (Wunsch Patrik S95): in einer Ein- oder
Ausblendung — linear wie Bézier — sollen sich **zusätzliche Punkte** setzen
und die **Punkte selbst verschieben** lassen (Zeit und Pegel); freie Punkte
mitten im Clip danach. Vorschlag S95: ein Punkte-Editor für beides — am
angeklickten Clip sind alle Punkte der Kurve zu sehen und zu ziehen,
Doppelklick setzt einen neuen, die Form gilt je Abschnitt. Die zwei Griffe
für die Länge der Blenden bleiben als Abkürzung.

**Drei Teile, je für sich speicherbar** (Vorgabe Patrik S95): die Pegelkurve
eines Clips wird als **Einblendung, Mitte und Ausblendung** betrachtet. Jeder
Teil soll sich als **Preset** speichern und auf andere Clips anwenden lassen.
Ein Blenden-Preset gilt für beide Seiten: als Ausblendung wird es
**gespiegelt** angewandt. Für den Bau heißt das: der Punkte-Editor arbeitet
je Teil, und ein Blenden-Preset speichert Form, Anfasser und Zwischenpunkte
in Anteilen der Blende (0 … 1), nicht in Millisekunden — so passt es auf jede
Länge und lässt sich spiegeln.

**Entscheide S96 (Patrik) — Form und Parameter getrennt:**

- **Die Form ist zeitlos.** Ein eigener Editor legt nur die Kurvenform fest
  (Punkte, Form je Abschnitt, Bézier-Anfasser), in Anteilen 0 … 1. Am Clip
  wird sie aus einem Preset geladen oder dort editiert.
- **Typ 1 — Blende:** Parameter am Clip ist die **Dauer**, je Seite. Die
  Seite bestimmt die Richtung: als Ausblendung läuft dieselbe Form
  rückwärts (an der y-Achse gespiegelt).
- **Typ 2 — Stück mitten im Clip:** Parameter am Clip sind **Weite** und
  **Position** (ab Clip-Anfang). An der x-Achse gespiegelt tauschen **oben
  und unten** (Rückfrage 1: a): aus einer Delle wird ein Buckel, gerechnet
  als Pegel → 1 − Pegel.
- **Strecken und Stauchen eines Clips:** die Blenden hängen an ihren
  Kanten, die Stücke verschieben sich anteilig; alle behalten ihre Weite,
  **bis sie sich berühren** — dann werden die sich berührenden gleichmäßig
  gestaucht (im Verhältnis ihrer eingestellten Weiten).
- **Die eingestellten Werte bleiben** (Entscheid: a), auch in der Datei;
  die gestauchte Fassung wird nur errechnet. Wird der Clip wieder länger,
  kommen die Teile in ihrer eingestellten Größe zurück. Das Panel zeigt an
  einem gestauchten Teil beide Zahlen.
- **Ein gestauchtes Teil von Hand ändern** (Rückfrage 2a: a): die ganze
  sich berührende Gruppe wird auf den sichtbaren Stand festgeschrieben,
  dann gilt die Änderung; Teile außerhalb der Gruppe behalten ihre Werte.
- **Der Editor** (Rückfrage 2: a): ein eigenes Fenster **Curve Editor**,
  nicht modal, aus dem Composer-Panel geöffnet (Doppelklick auf eine Blende
  oder ein Stück, Rechtsklick → Edit curve…); es folgt dem angeklickten
  Teil, Änderungen wirken sofort und gehen über Undo. Keine Naht. Auf der
  Zeitachse bleiben die Griffe für Dauer, Weite und Position.

- **Der Pegel zwischen den Teilen** (Entscheid: a): je Clip ein
  **Grundpegel** (Base level, 0 … 1, Vorgabe 1). Die Blenden laufen von null
  bis dorthin; eine Delle senkt von dort in Richtung null, ein Buckel hebt
  von dort in Richtung voll — an den Rändern eines Stücks gibt es keinen
  Sprung. **Alle Teile sind freiwillig:** ein Clip braucht weder Ein- noch
  Ausblendung noch ein Stück (ein Clip mit Grundpegel 0 zeigt sich nur in
  seinen Buckeln).

**Gebaut S96:**

- `composer/Pegelform` — `Pegelform` (Art Blende oder Stück, Punkte in
  Anteilen), `blendenPegel`, `stueckPegel`, `formAlsJson` / `formAusJson`,
  und `verteile`: der Platz von Einblendung, Stücken und Ausblendung in
  einem Clip samt Stauchen. Die Position eines Stücks ist die **Mitte** des
  Stücks als Anteil der Clip-Länge (Annahme, nicht eigens entschieden).
- **Am Clip:** `VisualClip` trägt `grundpegel`, `ein`, `aus` (`ClipBlende`:
  Dauer und Form) und `stuecke` (`ClipStueck`: Lage, Weite, gekippt, Form)
  statt der Punktliste; `pegelBei` rechnet daraus, `verteilung` sagt, wo die
  Teile gerade liegen. `Bearbeiten::schreibeFest` schreibt den sichtbaren
  Stand fest; die Zeitachse ruft es, bevor ein Griff gezogen wird, und
  zeichnet die Griffe an der gestauchten Lage.
- **Datei, Version 2:** `base`, `fadeIn` / `fadeOut` (`ms`, `shape`),
  `shapes` (`pos`, `width`, `flipped`, `shape`) — nur, was der Clip hat. Das
  alte Feld `levels` wird beim Laden übersetzt (`ausAltenPunkten`): das
  Muster einer einfachen Blende in Blenden, jede andere Kurve in **ein
  Stück über den ganzen Clip** — der Pegelverlauf bleibt derselbe.
- **Folge für die Ausblendung:** bis S95 fiel sie mit ihrer Kurvenform von
  1 auf 0; jetzt ist sie die rückwärts gelesene Blende. Für Linear und
  S-Kurve ist das dasselbe, für Ease in, Ease out, Exponential, Hold und
  Bézier nicht — *Fade out → Ease in* kommt jetzt langsam an, statt langsam
  loszugehen. Alte Dateien behalten ihren Verlauf (die Form wird beim Lesen
  umgerechnet).

- **Presets** (Entscheid Patrik S96: a — wie die Trail-Presets und die
  Knoten-Voreinstellungen): `composer/FormPresets`, je Art ein Ordner und
  eine Endung — Blenden `.lvfade` in `asset/composer/fades/` und
  `<AppData>/composer/fades/`, Stücke `.lvshape` in `…/shapes/`. Ein eigenes
  Preset verdrängt ein mitgeliefertes gleichen Namens; mitgelieferte lassen
  sich in der App nicht überschreiben oder löschen; eine Datei der anderen
  Art wird abgelehnt. **Mitgeliefert:** die sieben Formen aus C4 Teil 1 als
  Blenden (Linear, S-curve, Ease in, Ease out, Exponential, Hold, Bezier)
  und drei Stücke (Dip Soft, Dip Sharp, Dip Flat). **Dazu seit S100 das
  Stück Late fade in** (Befund Patrik S99 „Flipped und einfache
  Einblendung", Zielbild: unsichtbar einfahren, später einblenden, oben
  bleiben; Entscheid Patrik S100: a — Preset und Handbuch, kein neuer Wert
  am Clip): seine Kurve beginnt unten, hält bis 60 % der Weite auf null und
  steigt dann weich auf den Grundpegel. Es gehört an den Clip-Anfang und
  ist das einzige mitgelieferte Stück, das nicht auf dem Grundpegel
  beginnt — mitten im Clip spränge der Pegel an seinem Anfang auf null.
  Per Test belegt (`test_ComposerWiedergabe.cpp`, Modell-Fall der
  Pegelkurve): Pegel null, dann Einblenden, dahinter der Grundpegel, die
  Ausblendung wie gewohnt; ohne *Flipped*.

- **Das Fenster Curve Editor** (`composer/KurvenEditor`): `Formflaeche`
  (Punkte ziehen, Doppelklick setzt oder nimmt einen Punkt, Rechtsklick wählt
  die Form des Abschnitts, Bézier-Anfasser; die Enden einer Blende sind
  fest, die eines Stücks wandern nur in der Höhe) und `KurvenFenster`
  (Preset-Zeile, darunter Duration bzw. Width, Position, Flipped und Base
  level; an einem gestauchten Teil stehen beide Zahlen). Das Fenster ändert
  nichts selbst: es meldet den geänderten Clip, das Composer-Panel führt ihn
  über den CommandBus aus und zeigt dem Fenster den neuen Stand — auch nach
  Undo. Es folgt dem angeklickten Clip; ohne Edit zeigt es nur an. Eine
  Änderung an Dauer, Weite oder Lage schreibt zuerst den sichtbaren Stand
  fest (Rückfrage 2a).
- **Auf der Zeitachse:** Doppelklick auf eine Blende oder ein Stück öffnet
  die Kurve; Rechtsklick → *Edit fade in curve...*, *Edit fade out
  curve...*, *Add shape here*, *Edit shape curve...*, *Delete shape*; das
  Teil im Editor trägt einen gelben Strich unten im Clip. Die Länge einer
  Blende lässt sich auch ziehen, wenn ihre Form eigene Punkte hat; die Form
  einer Seite zu wählen lässt die andere Seite unberührt.

- **Das Fenster zeigt die Wirkung am Clip** (Entscheid Patrik S96: a): eine
  Ausblendung ist rückwärts gezeichnet (fällt von links oben nach rechts
  unten), ein gekipptes Stück als Buckel; gezogen wird in dieser Ansicht,
  gespeichert weiter die eine Form. `Formflaeche::setAnsicht` — die Ansicht
  sitzt allein in `ortVon`, `xVon`, `yVon`. Eine Zeile unter der Kurve sagt,
  wenn gespiegelt gezeigt wird. Der Grundpegel geht nicht in die Ansicht ein:
  die Fläche zeigt immer die volle Höhe.

- **Griffe der Stücke auf der Zeitachse:** oben im Clip je Stück eine Raute
  über der Mitte (verschieben) und ein Strich an jeder Kante (Weite, die
  Mitte bleibt). `Bearbeiten::verschiebeStueck`, `setzeStueckWeite`,
  `spielraumVon`: beide schreiben zuerst den sichtbaren Stand fest
  (Rückfrage 2a) und **halten an den Nachbarn an** — an der nächsten Blende
  und am nächsten Stück, wie ein Clip am Nachbarn; durch Ziehen wird nichts
  gestaucht, und kein Stück schiebt sich durch ein anderes (gebaut S96,
  nicht eigens entschieden). Einrasten wie bei den Clips, Alt frei. Die
  Felder Width und Position im Curve Editor setzen die Werte dagegen frei —
  dort kann ein Stück auch in einen Nachbarn hinein und wird dann gestaucht.

- **Presets im Rechtsklick-Menü der Zeitachse:** *Fade in* / *Fade out*
  listen die Blenden-Presets (mitgelieferte und eigene), der Haken sitzt an
  dem, dessen Form die Blende trägt; an einem Stück listet *Shape* die
  Stück-Presets. `Zeitachse::setzeBlendenPreset`, `setzeStueckPreset` — nur
  die Form wechselt, Dauer, Lage, Weite, Flipped und die andere Seite
  bleiben. Findet die App `asset/composer` nicht und gibt es keine eigenen
  Presets, zeigt das Menü wie zuvor die sieben festen Formen
  (`setzeBlendenForm`).

**Offen:**

- Eine Änderung wirkt beim Loslassen der Maus, nicht schon während des Zugs.

### 9e. Composer Variables — geplant (Wunsch Patrik S95)

Ein Modul **Composer Variables** in der Modul-Palette (Patrik:
„Compositor_Variables"): Skriptvariablen, die der **Composer** setzt. Läuft
das Preset ohne Composer, gelten die **Vorgaben des Moduls** — darum trägt
jede Variable dort einen Vorgabewert.

**Entschieden (Patrik S95):** beide Vorschläge unten, mit drei Regeln —
die Variablen gelten **im ganzen Preset** (nicht nur in einem Knoten); ihre
**Namen sind geschützt**, sie dürfen im Preset nicht doppelt belegt werden;
und das Preset kann sie **nur lesen**, nicht setzen. Zahl der freien
Variablen und die übrigen Fragen unten bleiben offen.

*Vorschlag 1 — feste Variablen, die der Composer von selbst liefert:*

| Variable | Bedeutung unter dem Composer | Vorgabe ohne Composer |
|---|---|---|
| `comp_on` | 1 | 0 |
| `comp_level` | Pegel des Clips aus seiner Kurve, 0 … 1 | 1 |
| `comp_pos` | Anteil des Wegs durch den Clip, 0 … 1 | 0 |
| `comp_time` | Sekunden seit Clip-Start | 0 |
| `comp_len` | Länge des Clips in Sekunden | 0 |

Damit kann ein Preset auf sein Ein- und Ausblenden antworten (etwa die
Bewegung mit dem Pegel beruhigen) oder sich über die Clip-Länge entwickeln.

*Vorschlag 2 — freie Variablen `cv1` … `cv8`:* je Clip eine Kurve über der
Zeit, im Composer-Panel gezeichnet mit demselben Editor wie die Pegelkurve;
im Modul je Variable ein Name und ein Vorgabewert. Das ist der Punkt
„Parameter über der Zeit" aus der Tabelle oben, als Skriptvariable statt als
Parameter-Bindung.

*Offen:* wie viele freie Variablen; ob sie frei benannt werden; ob eine
Kurve je Clip gilt oder als eigene Spur über die ganze Komposition; ob das
Modul nötig ist, damit ein Preset die Variablen sieht, oder nur die Vorgaben
trägt. Technisch günstig: jeder Clip hat seinen eigenen Renderer, die
Variablen eines Clips stören keinen anderen.

### 9f. Nachtrag S109 / S110 — die gemeinsame Achse, der Loop als Clip, der Haken Karaoke

Drei Dinge haben den Composer berührt, ohne dass dieses Dokument sie nannte.
Maßgebend für den Loop bleibt [Loop_Konzept.md](../audio/Loop_Konzept.md);
hier steht nur, was sich **am Composer** geändert hat.

**Die gemeinsame Basis `lumi::ui::Achse`** (S109, Loop-Konzept Entscheid
W1 b; `UI/widgets/Achse.hpp`). Maßstab, Zeit-Zoom, Zeilenhöhe, Rollen,
Mausrad, Schwenken mit der mittleren Taste und das Mitlaufen mit der
Abspielmarke — alles, was §7 unter ZO1 bis ZO9 beschreibt — liegen nicht mehr
in der `Zeitachse` des Composers, sondern in dieser Basis; die `Zeitachse`
erbt davon (rund 320 Zeilen sind gewandert), ebenso die `Taktachse` des
Loop-Panels. Die Basis rechnet in **Ticks**: für den Composer ist ein Tick
eine Millisekunde, für den Loop der 960. Teil eines Takts. Am Verhalten des
Composers hat sich nichts geändert: 33 Bilder der Zeitachse und ihre
Geometrie in Zahlen waren vor und nach dem Umbau bytegleich
(`test_AchseBilder.cpp`; `LUMIVIZ_ACHSE_BILDER=<Ordner>` legt sie ab). **Wer
an Zoom oder Rollen etwas ändert, ändert es für beide Panels** — und fährt
vorher und nachher den Bildvergleich.

**Der Loop als Clip im Audio-Kanal** (S109, Loop-Konzept §4e; Entscheide
T1 a und T2).

- **Modell und Datei unverändert:** ein Loop-Clip ist ein gewöhnlicher `Clip`
  in einem Audio-Kanal, dessen Datei auf `.lvloop` endet
  (`composer::istLoop`). Die `.lvcomp` kennt kein neues Feld.
- **Kein Ende:** ein Loop trägt nie eine Song-Länge (`quellMs` 0) — er lässt
  sich beliebig lang ziehen und wiederholt sich; ein neuer Clip ist einen
  Durchlauf lang. In der Zeitachse steht statt einer Wellenform ein
  gestrichelter Strich an jeder Stelle, an der der Loop von vorn beginnt.
- **Tonführung** (§4): je Loop-Clip hängt ein eigener **Klangerzeuger** als
  Quelle im Mixer (`IAudioEngine::mixerAddGenerator`), statt einer Datei. Er
  wird zwei Sekunden vor dem Einsatz angehalten eingehängt, seine Klänge
  werden dabei geladen (bei Instrument-Mustern: gerechnet). `setSourcePositionMs`
  der Engine wirkt auf eine solche Quelle nicht — die Stelle wird am
  Klangerzeuger selbst gesetzt und verglichen, im Kreis des Durchlaufs;
  nachgesetzt wird wie bei einem Song ab 80 ms (T1 a).
- **Laufender Song** (§8): ein Loop-Clip zählt als laufender Song (T2). Das
  Player-Panel zeigt „Composer - <Titel>" aus den Tags der `.lvloop` (ohne
  Titel den Dateinamen); Länge und Stelle sind die eines Durchlaufs.
- **Verbunden mit dem Loop-Panel:** hat das Panel dieselbe Datei offen
  (`loop/Offen.hpp`), spielt der Clip dessen Fassung — auch die
  ungespeicherte — und folgt jeder Änderung. Play im Loop-Panel ist gesperrt,
  solange der Composer den Ton hat (Rangfolge Composer, Loop-Panel, Player).
- **Abhängigkeit in einer Richtung — entschieden (Patrik S128, N1 b):**
  `composer/Tonfuehrung` nimmt `loop/Klangerzeuger`, `loop/Kit` und
  `loop/Offen`; der Loop kennt den Composer nicht. Er fragt über
  `loop/ComposerAbfragen` (führt der Composer den Ton, wo spielt er den
  Loop, rollt die Marke seitenweise); der Composer meldet die Antworten mit
  `composer::meldeLoopAbfragenAn` an, das Hauptfenster ruft es vor dem
  Aufbau der Panels. Die Farbreihen der Kanäle liegen in
  `UI/widgets/Farbreihen` (kühl/warm), `composer::reihenFarbe` ruft sie;
  die Taktachse nimmt sie von dort. Bis S127 hing der Loop über
  `composer::fuehrtTon` und `composer::reihenFarbe` am Composer.
- **Nicht drin:** ein Tempo der Komposition, an dem Loop-Clips einrasten
  (Loop-Konzept L2 a, zurückgestellt) — ein Loop-Clip rastet wie ein Song, an
  Kanten und am Raster in Sekunden.

**Der Haken Karaoke im Kopf des Panels** (S110, Befund Patrik): er war nach
dem Start leer, obwohl die Karaoke-Ebene an war, bis jemand sie schaltete.
Ursache war die Reihenfolge beim Start: das Panel liest den Schalter beim
Aufbau aus dem Renderer (`MultiEffectVisualizer::karaokeEbene`), der ihn erst
danach aus den Einstellungen bekam; Panel Karaoke und Menü lesen die
Einstellung selbst und stimmten. Seit S110 setzt das Hauptfenster den
Schalter **vor** dem Aufbau der Panels (`MainWindow::setupUi`).

### 9g. Lautstärke-Abgleich der Ton-Clips — entschieden S126, G1 bis G3 gebaut

**Anlass (Patrik S126):** in `asset/compare.lvcomp` liegen der Song
*Titanium* (A1) und der nachgebaute Loop `titanium_versuch.lvloop` (A2)
übereinander. Der Loop ist „viel viel leiser als das Original", obwohl seine
Pegelkurve bei fast 1 und die des Songs bei etwa 0,85 steht. Ursache im
Modell: ein Clip kann nur leiser werden als seine Datei — die Pegelkurve
läuft von 0 bis 1, und 1 heißt „so laut wie die Datei" (§6). Wunsch Patrik:
jeden Clip einzeln lauter oder leiser stellen, und ein Dialog, der die
Lautheit der Clips misst und in einem Mixer je Clip eine Grundlautstärke
vorschlägt, **sodass Ein- und Ausblenden 0…1 für jeden Clip gleich klingt**.

**Belegt vor den Entscheiden:** BASS verstärkt über 1,0 (Doku
`BASS_ATTRIB_VOL`: „above 1.0 = amplification", abgerufen S126); der Mixer
des Composers rechnet in Fließkomma ohne Begrenzer
(`BassEngine.cpp`, `BASS_SAMPLE_FLOAT | BASS_MIXER_NONSTOP`); messbar sind
ein Song über `decodeStereo`, ein Loop über den Klangerzeuger ohne Engine.

**Entscheide (Patrik S126):**

- **L1 a — Gain je Ton-Clip, getrennt von der Pegelkurve.** Neues Feld
  **Gain** in dB, −24 bis +24, Vorgabe 0. Zu hören ist *Datei × Gain ×
  Pegelkurve × Kurve der Komposition*. Die Pegelkurve bleibt 0…1 und ist nur
  noch Blende und Stück. In der `.lvcomp` `"gain": 13.3`; fehlt das Feld,
  gilt 0 dB — alte Kompositionen klingen gleich. Der Clip zeigt den Wert als
  Marke, sobald er nicht 0 ist. Bild-Clips haben kein Gain. Verworfen: die
  Pegelkurve über 1 (vermischt Angleichen und Blenden), Gain je Kanal (ein
  Kanal trägt verschiedene Songs).
- **L2 a — Lautheit nach EBU R128 / ITU-R BS.1770, in LUFS.** K-Filter,
  Blöcke von 400 ms, Gate gegen Stille; daneben die Spitze (Peak) in dB.
  Ein Messbaustein ohne Engine und ohne Qt-Anwendung, gegen die Prüftöne der
  Norm getestet. Verworfen: RMS ohne Gewichtung (zählt den Bass voll — ein
  Loop mit kräftiger Kick bekäme zu wenig Gain), Peak (löst das Problem
  nicht).
- **L3 a — Target:** eine Liste mit **jedem Ton-Clip der Komposition** und
  **Fixed level** mit einem Feld in LUFS (Vorgabe −14); zuerst gewählt ist
  der **lauteste Clip**. Der Bezugsclip bekommt 0 dB, jeder andere den
  Abstand seiner Lautheit. Gespeichert wird nur das Gain je Clip; die letzte
  Wahl merkt sich der Dialog in den Einstellungen. Verworfen: nur Bezugsclip,
  nur fester Wert.
- **L3b a — gemessen wird der gespielte Ausschnitt, ohne Pegelkurve.** Genau
  der Teil der Datei, den der Clip spielt; beim Loop-Clip ein Durchlauf,
  nachdem einer vorgerechnet und verworfen ist (damit Echo und Hall des
  vorigen mitklingen). Die Auswahl „Measure" der Skizze entfällt.
  **Vor dem Bau zu prüfen:** ob ein Durchlauf jedem anderen gleicht; weichen
  sie ab, wird die ganze Länge des Clips gemessen. Verworfen: die ganze
  Datei, nur der Teil auf vollem Pegel.
- **L4 a — Limiter am Mixer des Composers.** Decke −0,1 dB, kurze Vorschau,
  weiches Loslassen; er greift **nur**, wo heute hart abgeschnitten würde —
  darunter bleibt das Signal Bit für Bit gleich. Der Dialog schlägt das volle
  Gain vor; geht die Spitze eines Clips dadurch über 0 dB, sagt die Zeile es
  in Gelb („Peak would reach +3.5 dB – the limiter keeps it below 0 dB.").
  Haken **Limiter**, Vorgabe an; sein Ort wird mit dem Bau festgelegt und im
  Bild gezeigt. **Vor dem Bau zu prüfen:** ob die Visuals das Signal vor oder
  nach einem Baustein am Mixer (`BASS_ChannelSetDSP`) sehen. Verworfen:
  Vorschlag auf −1 dB Peak kappen (der Loop bliebe leiser als das Original),
  nur warnen (hartes Abschneiden).

**Welche Clips (Antwort an Patrik S126, als Annahme):** der Dialog führt
jeden Ton-Clip aller Audio-Kanäle, Loop-Clips eingeschlossen, jeden als
eigene Zeile; zwei Clips derselben Datei werden je für sich gemessen (sie
können verschiedene Ausschnitte spielen); Clips ausgeschalteter Kanäle
stehen grau in der Liste und werden mitgemessen.

**Die Skizze (S126, als Beschreibung; nach L3b a ohne „Measure"):** Dialog
**Match loudness – <Komposition>**. Oben **Target** (Liste: Clips mit Namen,
Fixed level) und **Measure again**. Darunter je Ton-Clip eine Zeile: Farbe
und Name des Kanals, Name des Clips · **Loudness** (LUFS) · **Peak** (dB) ·
**Gain** als Schieber mit Wert in dB · **Suggested** (dB, gelb mit Warnzeichen,
wenn die Spitze über 0 dB käme, darunter der Satz zum Limiter). Unten **Use
suggestions** und **Listen A/B** links, **Cancel** und **OK** rechts. Im
Composer trägt ein Clip mit Gain die Marke „+13.3 dB" rechts im Clip.
**Nicht entschieden, in der Skizze nur angedeutet:** was Listen A/B genau
tut, wie Gain am Clip selbst bedient wird (Feld, Rad mit Taste — Ctrl+Rad
und Alt+Rad gehören schon der Achse), ob Use suggestions und OK ein
Undo-Schritt sind. Wird mit der Stufe vorgelegt, die es baut.

**Baustufen (freigegeben S126, Patrik: „G1 und G3 sofort, vor dem
Loop-Kleinkram“):**

- ✅ **G1 Modell, Datei, Ton** (S126): `Clip::gainDb`, `gainFaktor()`,
  `kGainMinDb`/`kGainMaxDb`; `richte` klemmt, ein Bild-Clip hat 0; die
  Fassung der `.lvcomp` bleibt 2 (freiwilliges Feld). Den laufenden Song
  bestimmt weiter die Pegelkurve allein (Entscheid S97), das Gain wirkt nur
  auf die Lautstärke. Test „S126 Gain je Ton-Clip“ in
  `test_ComposerTonfuehrung.cpp`. Bis G2 nur von Hand in der Datei zu setzen.
  Geplant war: `gain` am Ton-Clip, `.lvcomp` liest und
  schreibt es, die Tonführung multipliziert es (`Tonfuehrung.cpp`, neben
  `pegelBei`); Tests gegen die Engine aus Pappe (Volume über 1 kommt an,
  alte Dateien unverändert).
- ✅ **G2 Gain am Clip bedienen** (S126, Entscheid Patrik **G2 a**, mit
  Skizze): in der Fußzeile des angeklickten Clips steht bei einem Ton-Clip
  **Gain:** (`composerGain`, −24 … +24 dB, eine Stelle, Pfeile und Rad 0,5 dB)
  an der Stelle von *Blend*; bei einem Bild-Clip und ohne Auswahl bleibt
  *Blend*. Nur mit *Edit*, jede Änderung ein Undo-Schritt „Change gain“. Ein
  Ton-Clip mit Gain trägt rechts eine Marke („+6.0 dB“, `gainText`); sie
  weicht, wenn der Clip zu schmal ist. Rechtsklick **Reset gain (+6.0 dB)**
  nur an einem Ton-Clip mit Gain (`Zeitachse::gainZuruecksetzen`, Undo
  „Reset gain“). Test „S126 G2 Panel“ in `test_ComposerPanel.cpp`,
  `LUMIVIZ_COMPOSER_GAIN_BILD=<datei.png>` legt ein Bild ab. Von mir gesetzt:
  das Feld zeigt kein „+“ (Qt-Feld), die Marke schon; Schritt 0,5 dB auch
  für die Pfeile (vorgelegt war 0,1 für Pfeile und 0,5 für das Rad — ein
  Qt-Feld kennt nur einen Schritt); bei mehreren markierten Clips stellt das
  Feld nur den angeklickten.
- ✅ **G3 Messbaustein** (S126): `composer/Lautheit` — `LautheitsMessung`
  (stückweise fütterbar) und `messeLautheit`, K-Filter für jede Abtastrate
  (Kennwerte wie in libebur128), Blöcke 400 ms / 75 %, Gates −70 LUFS und
  −10 LU, Sample-Spitze (kein True Peak). `test_ComposerLautheit.cpp`:
  - die Fälle 1 bis 5 aus **EBU Tech 3341** (2023, Tabelle 1, gedruckte
    S. 8) bei 48 und 44,1 kHz innerhalb ±0,1 LU;
  - ein Sinus bei 50 Hz misst 4,6 dB leiser als einer bei 1 kHz gleicher
    Spitze (der Grund für R128 statt RMS);
  - **Beleg L3b:** von 46 Loops unter `asset/loop` sind bei 37 der zweite
    und dritte Durchlauf Bit für Bit gleich, bei 9 weichen sie um höchstens
    5·10⁻⁵ ab (Effekte), in der Lautheit auf vier Stellen gar nicht; der
    erste (ohne Nachklang) weicht um höchstens 0,015 LU ab. Ein Durchlauf
    nach einem vorgerechneten genügt also;
  - zehn Minuten Stereo bei 44,1 kHz: 126 ms unter Release-clang, 622 ms
    unter Testing-MSVC (ohne Dekodieren).
- **G4 Limiter** am Mixer, Haken; Beleg, was die Visuals sehen; Test: unter
  0 dB Bit für Bit gleich.
  **Beleg vor dem Bau (S126, „Beleg G4“ in `test_ComposerNahtMixer.cpp`,
  echte BassEngine, rund eine Sekunde sehr leiser Ton):** Wellenform und
  Spektrum des Mixers — was die Visuals lesen (`MainWindow::onAudioUpdate`)
  — zeigen das Signal **hinter** einem DSP am Mixer (×0,5 am Mixer: Spitze
  0,125 statt 0,25). Ein Limiter am Mixer ändert also auch, was die Visuals
  sehen — nur an Spitzen über 0 dB. Das Gain einer Quelle kommt **vor** dem
  DSP an (×4 an der Quelle, ×0,5 am Mixer: 0,5).
  **Befund dabei — G1 wirkte nicht über 0 dB:** `BassEngine::setVolume`
  klemmte auf 0…1; ein Gain über 0 dB kam in der echten Engine nie an (der
  Test zu G1 lief gegen die Engine aus Pappe, die nicht klemmt). Behoben:
  Grenze `kHoechsteLautstaerke` = 16 (+24 dB) in `IAudioEngine.hpp`; alle
  anderen Aufrufer geben höchstens 1 (Lautstärke des Players ist auf 0…1
  geklemmt).
  **Teil 1 gebaut (S126, `49fa0f56`):** `audio/Begrenzer` — Stereo, beide
  Kanäle mit demselben Faktor, Decke −0,1 dBFS, sofortiges Absenken am
  lauten Sample, Loslassen mit 80 ms Zeitkonstante, **ohne Vorschau** (von
  mir gesetzt, abweichend von „kurze Vorschau“ in L4 a: eine Vorschau
  verzögert das ganze Signal — dann ginge nichts mehr Bit für Bit durch, und
  der Ton läge gegen die Uhr versetzt; Preis: kurze Spitzen klingen etwas
  härter). `IAudioEngine::setMixerLimiter` / `mixerLimiterFrames`, die
  BassEngine hängt ihn als DSP an den Mixer. Tests `test_ComposerBegrenzer`
  und im Beleg G4 (Quelle ×8: Spitze 0,9886 statt 2,0). **Der Composer
  schaltet ihn noch nicht ein.**
  **Sichttest 11, L2 (S130): „mit Limiter kracht es“.** Gemessen (Begrenzer
  nach Python übertragen, `15_pseudo_musik_120bpm_60s`, +24 dB): ohne
  Vorschau sprang die Verstärkung an einer Spitze in einem Frame um bis zu
  23 % — hörbar als Knacken (157 Fenster je Sekunde mit Energie über 5 kHz,
  die das Original nicht hat; hart bei 1,0 abgeschnitten 319, mit 5 ms
  Vorschau 46). Der Preis „etwas härter“ war unterschätzt. Warum es ohne
  Limiter bei Patrik nicht stärker kracht, ist nicht belegt (vermutet: das
  System fängt Fließkomma über 1 selbst weicher ab).
  **Entscheid Patrik L2-1 b (S130):** der Limiter schaut **rund 5 ms
  voraus** (`Begrenzer::kVorschauMs`): die Ziel-Verstärkung ist das Minimum
  dessen, was die nächsten n Frames brauchen, mit 80 ms Loslassen; die
  Verstärkung eines Frames ist das Mittel der letzten n Ziele — sie sinkt
  gleitend und hält die Decke trotzdem ein. Der Ton kommt dafür
  `verzoegerungFrames()` später (220 Frames bei 44,1 kHz), **nur bei
  gesetztem Haken** (der DSP hängt nur dann am Mixer); ein Umschalten
  springt um diese 5 ms. Unter der Decke Bit für Bit gleich, nur versetzt.
  Verworfen: a (Verzögerung immer, solange der Composer führt), c (ohne
  Vorschau, Absenken über 1 ms). Test: größter Sprung je Frame 0,0034 statt
  rund 0,75 bei einem Schlag von +12 dB.
  **Entscheid Patrik G4b a (S126, mit Skizze):** der Haken **Limiter** steht
  in der Zeile der Komposition hinter *Visuals overlap themselves*, Vorgabe
  an, nur mit *Edit*, Undo-Schritt; in der `.lvcomp` `"limiter": false` nur,
  wenn aus. Verworfen: Settings → Composer für alle Kompositionen.
  **Teil 2 gebaut (S127):** `Komposition::limiter` (Vorgabe an), in der
  `.lvcomp` `"limiter": false` nur, wenn aus; die Tonführung gibt ihn im
  ersten Takt nach `uebernimm` und danach nur bei einem Wechsel an den Mixer
  (`m_limiter`, wie `m_laut`) — mit dem Mixer geht auch sein Limiter. Eine
  Engine ohne Limiter antwortet false; die Führung versucht es dann nicht
  bei jedem Takt neu. Der Haken `composerLimiter` steht hinter *Visuals
  overlap themselves*, nur mit *Edit*, Undo „Turn on/off limiter“. Tests:
  in `test_ComposerTonfuehrung` „S127 G4 Limiter“ (die Engine aus Pappe
  schreibt die Aufrufe in `limiterAufrufe`, nicht ins `protokoll`, das
  manche Fälle als Ganzes vergleichen), in `test_ComposerPanel` im Fall
  „C3a Panel“ der Haken mit Undo, Bild `<LUMIVIZ_COMPOSER_PANEL_BILD>.limiter.png`.
  Handbuch 1.171.0.
- **G5 Dialog Match loudness** nach der Skizze, mit Bild aus dem Test;
  vorher Listen A/B und Undo vorlegen.
  **Entscheid Patrik G5-1 a (S127) — Vorhören:** der Dialog sperrt den
  Composer nicht (Play, Stop, Klicks in die Zeitachse gehen weiter); was im
  Dialog steht, hört man sofort, wenn der Composer spielt — noch kein
  Undo-Schritt, *Cancel* stellt zurück. **Listen A/B** schaltet zwischen den
  Gains von vor dem Dialog (A) und denen im Dialog (B), der Knopf zeigt
  „Hearing: A (before)“ / „Hearing: B (dialog)“. Je Zeile ein Knopf **▶**:
  setzt die Abspielstelle in die Mitte des gespielten Ausschnitts dieses
  Clips und startet den Composer; Clips, die dort auch liegen, klingen mit.
  Verworfen: nur A/B ohne ▶, nur ▶ ohne A/B, kein Vorhören (modal, hören nach
  OK).
  **Entscheid Patrik G5-2 a (S127) — Undo:** *OK* schreibt **einen**
  Undo-Schritt „Match loudness“ mit allen geänderten Gains. *Use
  suggestions*, Schieber und *Listen A/B* ändern nur den Dialog und das
  Gehörte, nicht Undo; *Cancel* verwirft ohne Spur. Solange der Dialog offen
  ist, ist *Edit* grau (Abspielen, Stelle setzen, ▶ gehen). Ändert sich die
  Komposition doch (etwa Ctrl+Z), liest der Dialog die Liste neu: die Gains
  bleiben bei Clips mit derselben Datei und demselben Ausschnitt, neue oder
  geänderte Clips werden neu gemessen. Grund: Clips haben keine eigene
  Kennung, der Dialog kennt sie nur an Kanal und Platz. Verworfen:
  Bearbeiten bei offenem Dialog (bräuchte eine Kennung je Clip), zwei
  Schritte (Use suggestions eigener Schritt), jeder Zug ein Schritt.
  **Entscheid Patrik G5-3 a (S127) — Messen:** beim Öffnen misst der Dialog
  von selbst, im Hintergrund, Clip für Clip; eine Zeile zeigt „Measuring…
  35 %“, bis Loudness und Peak dastehen; der Dialog bleibt bedienbar (auch ▶
  und Listen A/B). *Suggested* und *Use suggestions* gelten je Zeile, sobald
  sie und das Ziel gemessen sind. **Songs** merkt sich LumiViz in einer
  Datei im Cache-Ordner des Benutzers (Schlüssel: Datei, Größe,
  Änderungszeit, Ausschnitt) — auch über einen Neustart; **Loops** werden
  bei jedem Öffnen neu gemessen (ihr Klang hängt an Instrumenten, Kits und
  Samples). **Measure again** misst alle neu und überschreibt Gemerktes.
  Schließen oder *Cancel* bricht eine laufende Messung ab. Der Cache gehört
  nicht zu den eigenen Daten (kein Export, kein Publish). Ein Song wird
  **stückweise** ab dem Anfang des Ausschnitts dekodiert — `decodeStereo`
  lädt die ganze Datei und ist für kurze Klänge gedacht. **Im Bau zu
  melden:** wie lange das Dekodieren eines Songs dauert; bleibt es unter
  einer halben Sekunde, ist die Cache-Datei überflüssig (dann vorlegen,
  bevor sie gebaut wird). Verworfen: nur bis zum Beenden merken, Messen erst
  auf Klick mit wartendem Fortschrittsfenster.
  **Gemessen (S127, `test_ComposerClipLautheit`, `LUMIVIZ_BELEG_SONG`):**
  *Titanium* (rund vier Minuten MP3) ganz dekodiert und gemessen in 235 ms
  unter Release-clang, 490–530 ms unter Testing-MSVC — unter der Schwelle.
  Die Merk-Klasse war da schon gebaut (gegen die eigene Ankündigung, nicht
  angeschlossen) und wurde vorgelegt. **Entscheid Patrik G5-3b a:** die
  Cache-Datei bleibt (für viele Songs, lange Mixes und einen gedrosselten
  Prozessor); verworfen: nur im Speicher, gar nichts merken.
  **Gebaut (S127): die Messung je Ton-Clip.**
  `IAudioEngine::decodeStereoChunks` (die BassEngine dekodiert ab dem Anfang
  des Ausschnitts in Stücken, PRESCAN für genaue Sprünge in MP3, darf in
  einem eigenen Thread laufen); `composer/ClipLautheit`: `messeSong`,
  `bereiteLoopVor` (Thread der Oberfläche: was das Loop-Panel offen hat, die
  Klänge über die Engine, ein eigenes Programm je Messung) und `messeLoop`
  (zwei Durchläufe, gemessen der zweite; ohne Engine), `SongLautheiten`
  (`composer_loudness.json` im Cache-Ordner, Schlüssel Datei, Größe,
  Änderungszeit, Ausschnitt, Rate). Ein Sinus von −23 dBFS misst −23 LUFS;
  hinter 5 s Stille −23,12 LUFS — die Blöcke über der Grenze zählen nach R128
  mit.
  **Gebaut (S127): der Dialog.** `composer/Abgleich` (Modell ohne Fenster:
  Zeilen, Vorschlag auf ein Zehntel, Spitze mit Vorschlag, Use suggestions,
  Neu-Lesen — „vorher“ folgt der Komposition, ein im Dialog geändertes Gain
  bleibt, das Ziel folgt seinem Clip, sonst der lauteste),
  `composer/LautheitsDialog` (nicht modal; eine Messung zur Zeit im eigenen
  Thread, Loops im Thread der Oberfläche vorbereitet; jede Messung nummeriert,
  ein Ergebnis einer abgebrochenen gilt nicht), im Panel der Knopf
  `composerMatchLoudness` neben Gain, der Rechtsklick
  `composerMatchToClip`, *Edit* grau über `Zeitachse::setBearbeiten(false)`
  bei stehendem Haken. Die markierte Zeile ist fett statt ausgewählt (eine
  Auswahl zeichnete sich über den Schiebern unsauber — von mir gesetzt).
  **Stolperstein:** `Sitzung::setzeKomposition` meldet „bearbeitet“ noch
  während des Aufrufs — der Dialog hielt seine eigene Vorschau zuerst für eine
  Änderung von außen; jetzt markiert er die eigenen Aufrufe. Tests:
  `test_ComposerAbgleich` (zwei Fälle zum Modell, einer zum Dialog im Panel
  gegen die Engine aus Pappe, die einen Sinus je Datei „dekodiert“;
  `LUMIVIZ_MATCH_LOUDNESS_BILD`). Handbuch 1.172.0. Im Programm nicht
  gesehen und nicht gehört.
  **Entscheid Patrik G5-4 a, Ort angepasst (S127) — Eingang:** ein Knopf
  **Match loudness...** in der Fußzeile **neben dem Feld Gain** (Patrik:
  „eher im Bereich, wo man auch Gain angeben kann“) — sichtbar wie das Feld,
  also wenn ein Ton-Clip gewählt ist, aktiv nur mit *Edit*. Dazu im
  Rechtsklick eines Ton-Clips **Match loudness to this clip...**, nur mit
  *Edit*, mit diesem Clip als *Target*. **Von mir gesetzt:** der Knopf öffnet
  mit der Vorgabe aus L3 a (der lauteste Clip oder die gemerkte Wahl) und
  markiert die Zeile des gewählten Clips — er macht ihn **nicht** zum Ziel:
  neben seinem Gain-Feld erwartet man, dass dieser Clip angeglichen wird,
  nicht dass er bei 0 dB bleibt und die anderen sich bewegen. Verworfen: der
  Knopf in der Datei-Zeile (vorgeschlagen als a), nur Knopf, nur Rechtsklick.
  **Nachtrag Patrik (S127, nach dem Bau):** „Match loudness kann doch immer
  angezeigt werden, nicht nur wenn ein Audioclip markiert ist“ — der Knopf
  steht jetzt immer ganz rechts in der Fußzeile, sobald eine Komposition
  geladen ist; wählbar mit *Edit* und mindestens einem Ton-Clip. Ein
  angeklickter Ton-Clip wird im Dialog markiert, sonst keiner
  (`ComposerPanel::zeigeAbgleichKnopf`, auch nach jeder Bearbeitung).
- ✅ **G6** Handbuch, Sichttest-Liste, voller Lauf (S127). Handbuch mit G4
  (1.171.0) und G5 (1.172.0); Liste `.claude/handover/Sichttest_Composer_11_Lautstaerke.md`
  (Abschnitte G Gain, C Clip aus, L Limiter, M Match loudness, Urteile U1 bis
  U4) mit der neuen Vorlage `asset/sichttest/composer_L_lautstaerke.lvcomp`
  (Testmusik, *beat_A*, Sinus nacheinander; Zusage in `test_ComposerVorlagen`).
  Voller Lauf unter Testing-MSVC: 1263 / 1264, 0 Skips — der eine war die
  Vorlage, die während des Laufs ohne Zusage dazukam; danach die Fälle zu
  Vorlagen und Abgleich unter beiden Compilern grün. **Stufen G1 bis G6
  fertig; offen sind Patriks Sichttest und Urteile.**

### 9h. Einzelne Clips ausschalten — entschieden und gebaut S126

**Wunsch Patrik S126:** „einzelne Clips (audio und visuals) sollte man per
Kontextmenü deaktivieren können". **Entscheid D1 c:** wie das Ausschalten
eines Kanals, dazu ein Tastenkürzel.

- **Bedienung:** Rechtsklick auf einen Clip → Haken **Clip enabled**
  (Kürzel **Ctrl+E** für den angeklickten Clip, in der Zeitachse); nur mit
  *Edit*, wie *Channel enabled* (das war in der Vorlage als „auch ohne Edit"
  angenommen und vor dem Bau im Code nachgesehen: `kanalSchalten` verlangt
  *Edit*). Undo „Disable clip" / „Enable clip".
- **Wirkung:** ein ausgeschalteter Clip spielt nicht — kein Ton
  (`Komposition::toeneBei`), kein Bild und kein Vorwärmen (`Wiedergabe`),
  kein laufender Song (`Tonfuehrung::stelleImLaufenden`), kein Ziel für Next /
  Previous (`songAnfang`). Er bleibt in der Zeitachse und lässt sich
  bearbeiten; das Ende der Komposition folgt auch ihm. Der festgehaltene Clip
  von *Sync on Effect Chain* wird weiter gezeigt (wie in einem ausgeschalteten
  Kanal).
- **Darstellung:** blasse Schrift und „(off)" hinter dem Namen; die Farbe wie
  bei einem ausgeschalteten Kanal (bei *Composer off* sind alle Clips
  gedämpft, dann unterscheidet nur die Schrift).
- **Im Panel Player (S130, Sichttest 11 C1, Entscheid Patrik C1 c):** liegt
  an der Stelle kein gespielter Song, aber ein ausgeschalteter Ton-Clip, zeigt
  das Panel ihn wie die Zeitachse: „Composer - <Titel> (off)", sein Künstler,
  der Balken läuft durch ihn, ein Klick in den Balken springt in ihm. Er ist
  **kein Songwechsel**: ein eigenes Ereignis (`AusgeschalteterClipEvent` in
  `composer/AppAnbindung.hpp`), Tag View, Karaoke, Songtext, Audio Browser
  und die Stelle der Visuals bleiben beim letzten gespielten Song. Ist er
  vorbei, holt das Panel den laufenden Song aus `tonStand()`; in einer Lücke
  ohne Clip gilt weiter S98 (der letzte Song steht). Mehrere ausgeschaltete
  Clips an einer Stelle: der des obersten Kanals. Titel und Länge liest die
  Tonführung einmal beim Eintritt (eine Datei kurz geöffnet, ein Loop
  gelesen). Verworfen: a (die Regel aus S98 auch hier — dann nannte das Panel
  einen vorbeigegangenen Song, während die Achse „(off)" zeigt), b (in jeder
  Lücke kein Song). Tests: „S130 C1 c" in `test_ComposerTonfuehrung` und
  `test_ComposerNahtTon`.
- **Datei:** `Clip::an`, in der `.lvcomp` `"enabled": false` nur, wenn aus;
  Fassung bleibt 2.
- **Tests:** „S126 D1 Clip aus" (`test_ComposerTonfuehrung`), „S126 D1 GL"
  (`test_ComposerWiedergabe`), Ctrl+E, Undo, Speichern im Fall „S126 G2
  Panel" (`test_ComposerPanel`, Bild mit `.off.png`).

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
| C7 | Der Start | **Neu gefasst S95 (Patrik):** lädt die Komposition **ins Panel**. Knopf **Resume after restart** aus (Vorgabe): Composer **aus**, die Effektkette läuft, im Panel wird vorbereitet. Knopf an: der Stand vom Beenden kommt wieder, eine laufende Komposition läuft von der letzten Stelle an weiter | S93: Composer an, am Anfang, angehalten; immer weiterlaufen; lädt ein einzelnes Preset; Knopf aus = keine Komposition beim Start; Knopf erst mit C6 |
| — | Öffnen von Hand (S95) | Open Composition **startet sofort** (Vorgabe); per Einstellung in Settings „hält am Anfang" | nur eines von beiden, fest |
| — | Das Ende ohne Loop (S95) | **einstellbar:** halten (Vorgabe), an den Anfang, Composer aus | nur eines davon, fest |
| — | Effect Chain bei laufender Komposition (S95) | C2b: bleibt an der Kette. C3: Schalter **Sync on Effect Chain** im Composer-Panel — an: der angeklickte Clip, aus: Effect Chain ruht | schon in C2b den Clip zeigen; ohne Schalter immer ruhen |
| C9 | Die Lücke | **schwarz** | letztes Bild; durchscheinendes Preset; Grund-Visual als eigenes Feld |
| C10 | Wo der Composer entsteht | **Zweig `composer`**; nach `master` geholt wird **erst, wenn der Composer sauber funktioniert** (Patrik S93, geändert gegenüber „nach jeder abgenommenen Stufe"). Bis dahin läuft **alle** Arbeit im Zweig, auch Abschlüsse und Fixes | auf `master`; nach jeder Stufe holen |
| C11 | Dateien mit Host-Gruppen als Clip | **entfällt** — mit dem eigenen Mischer läuft jede Datei unverändert | — |
| — | Mehrere Songs zugleich: woher die Streams kommen (S97) | **ein Mixer in der Engine** (BASSmix), der Composer hält ihn; die Uhr führt | eigene Streams des Composers mit selbst gerechneter Summe (das eigene Spektrum liefe neben dem von BASS, an dem die Presets kalibriert sind — nicht geprüft, ob gleich); mehrere Streams im Player (größte Naht, löst die Summe nicht) |
| — | Wessen Spektrum die Visuals sehen (S97) | **die Summe mit den Pegeln** — das, was zu hören ist | — (Zielbild Patrik) |
| — | Der laufende Song bei mehreren zugleich (S97) | **die Pegelkurve entscheidet, und alles folgt** (Karaoke, Tags, Songwechsel-Zähler, Tag View, Audio Browser) | der gemessene Pegel entscheidet; nur die Karaoke folgt, der Rest wechselt am Clip-Anfang |
| — | Das Player-Panel bei führendem Composer (S97, **geändert S98**) | **zeigt den laufenden Song als „Composer - <Song>"** (Künstler, Stelle im Song, Länge; Klick in den Balken springt im Song und damit in der Komposition; in einer Lücke bleibt der letzte Song stehen; Loop-Knopf ausgegraut) | zeigt die Komposition (Dateiname, Zeitachse; Loop-Knopf = Schleife der Komposition) — in S97 entschieden, in S98 von Patrik zurückgenommen, nichts davon gebaut; ruht |
| L1 | Lautstärke eines Clips über die Datei hinaus (S126) | **Gain je Ton-Clip in dB**, −24 bis +24, getrennt von der Pegelkurve 0…1 (§9g) | Pegelkurve über 1; Gain je Kanal |
| L2 | Maß des Abgleichs (S126) | **Lautheit nach EBU R128 in LUFS**, Peak daneben | RMS; Peak |
| L3 | Ziel des Abgleichs (S126) | **Liste: jeder Ton-Clip und Fixed level**, Vorgabe der lauteste Clip | nur Bezugsclip; nur fester Wert |
| L3b | Gemessener Teil (S126) | **der gespielte Ausschnitt ohne Pegelkurve**, beim Loop ein Durchlauf nach einem vorgerechneten | ganze Datei; nur der Teil auf vollem Pegel |
| G4b | Ort des Hakens Limiter (S126) | **in der Zeile der Komposition**, in der `.lvcomp`, Vorgabe an | Settings → Composer, für alle |
| G5-1 | Vorhören im Dialog Match loudness (S127) | **Dialog sperrt nicht, wirkt live; Listen A/B als Umschalter vorher/Dialog; ▶ je Zeile** | nur A/B; nur ▶; modal, hören nach OK |
| G5-2 | Undo im Dialog Match loudness (S127) | **ein Schritt beim OK; Edit grau, solange der Dialog offen ist; bei Änderung Liste neu lesen** | Bearbeiten bei offenem Dialog; zwei Schritte; jeder Zug ein Schritt |
| G5-3 | Messen im Dialog Match loudness (S127) | **beim Öffnen im Hintergrund; Songs im Cache gemerkt (auch über Neustart), Loops immer neu** | nur bis Beenden merken; erst auf Klick, wartend |
| G5-3b | Cache-Datei für Songs nach der Messung, 235 ms je Song (S127) | **bleibt** | nur im Speicher; gar nichts merken |
| G5-4 | Eingang zum Dialog Match loudness (S127) | **Knopf in der Fußzeile, immer da (Nachtrag Patrik), und Rechtsklick „Match loudness to this clip...“, beide nur mit Edit** | Knopf in der Datei-Zeile; nur Knopf; nur Rechtsklick |
| D1 | Einzelne Clips ausschalten (S126) | **Haken Clip enabled im Rechtsklick und Ctrl+E**, nur mit Edit; wirkt wie ein ausgeschalteter Kanal (§9h) | Grundpegel auf 0 (*Mute clip*); ohne Kürzel |
| L4 | Spitzen über 0 dB (S126) | **Limiter am Mixer**, Decke −0,1 dB, greift nur über 0 dB; Warnung im Dialog; Haken Limiter, Vorgabe an | Vorschlag kappen; nur warnen |

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

> **Abgeschlossen (Entscheid Patrik S105, M1: a — 2026-10-09).** Der Zweig `composer` ist nach
> `master` überführt (Vorspulen auf `ec836c6`; der Stand davor trägt die Marke `vor-composer`,
> `93d0be4`). Seither wird auf `master` gearbeitet, und die Regeln dieses Abschnitts gelten nicht
> mehr als Bauvorschrift: neue Berührungen von Bestehendem werden **nicht** mehr als Naht
> eingetragen, Naht und Eigenes nicht mehr getrennt committet. Die Listen in 12.2 und 12.4
> bleiben als Beschreibung dessen stehen, was der Composer (N1 bis N18) und die
> Sichttest-Steuerung (N19 bis N26) am Bestand geändert haben — Stand Session 105, nicht
> weitergeführt. Die Vermerke `Composer-Naht` im Code bleiben als Wegweiser.

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
| R2 Karaoke ✅ | Renderer, eine Abfrage und eine Weiterleitung | `setKaraokeAuslassen`; `legeKaraokeEbeneAuf` legt die gerechnete Ebene auf den gebundenen Framebuffer. **Wächst S101 (N16):** `karaokeFassung`. **Wächst S101 ein zweites Mal (Entscheid Patrik KS1: a — eine Zeile KARAOKE mit voller Kurve und ein Haken; eingetragen vor dem Bau):** `legeKaraokeEbeneAuf(pegel)` — ein Faktor 0…1 auf die Stärke jedes aufgelegten Teils (Trail, Kontur, Füllung); der Composer reicht den Pegel seiner Karaoke-Kurve an der laufenden Stelle herein. Ein Teil mit der Mischart XOR kennt keine Stärke: es ist bis Pegel null voll da | Vorgabe: zeichnet sie wie heute; Pegel 1 (Vorgabe) = wie bisher | C1a |
| R3 Bildquelle ✅ | `VisualizerRenderThread` (`setBildquelle`, Auswahl je Bild, Herunterfahren), `VisualizerWidget` (`setBildquelle` als Durchreichung, Besitz) | der Thread bekommt neben dem einen Renderer einen zweiten Platz für eine Bildquelle (`IVisualizer`). Ist er besetzt, gehen Audiodaten und `render` an die Bildquelle; der Renderer der Kette bleibt angelegt, bekommt weiter Größe und Audiodaten, wird aber nicht gerechnet. Die Bildquelle wird auf dem Render-Thread angelegt, aufgeräumt und gelöscht (Entscheid Patrik S94: a) | Platz leer (Vorgabe) → der eine Renderer wie heute | C2a |
| N4 Start ✅ | `MainWindow` (Konstruktor, nach dem Anwerfen des letzten Presets) | **ein** verzögerter Aufruf von `composer/AppAnbindung`: war beim Beenden eine Komposition geladen, kommt sie wieder ins Panel; mit Resume auch ihr Stand (Entscheid C7, S95). Das Festhalten beim Beenden ist eigener Code (hängt schon an `aboutToQuit`); `UI/StartPreset.hpp` wird **nicht** berührt | ohne Schlüssel `composer/…` tut der Aufruf nichts — der Start wie heute | C2b (vorgezogen aus C6, S95) |
| N5 Kürzel (Transport ✅) | Empfänger von `PresetStepEvent` und der Lied-Kürzel | bei laufendem Composer geht das Ereignis an ihn; Lock. **Der Transport ist nach C5 vorgezogen (Entscheid Patrik S96) und gebaut — `composer::transport`; einziger Empfänger von `TransportCommandEvent` ist das Player-Panel, Knöpfe, Hotkeys und Menü laufen alle durch diese Funktion:** `UI/panels/PlayerPanel.cpp`, `onTransportCommand` — eine Abfrage am Anfang und ein Aufruf von `composer/AppAnbindung`: führt ein Audiokanal, gehen Play / Pause, Stop, Next und Previous an den Composer; Lautstärke und Mute bleiben beim Player. `PresetStepEvent` (PgUp / PgDn) und Lock bleiben bei C6 | Composer aus oder kein Audiokanal an → der Befehl geht wie heute an den Player | C5 (Transport), C6 (Rest) |
| N6 Laden von Hand | `MainWindow` (gemeinsame Rückfrage vor dem Ersetzen der Kette) | Rückfrage bzw. Sperre bei laufendem Composer | Composer aus → wie heute | C6 |
| N7a Pegel — **zurückgenommen S98** (Entscheid Patrik: a, nach bestandenem Hörtest; von Hand, weil `3881510` auch N7b trägt — die drei Dateien gleichen in diesen Stellen wieder dem Stand vor der Naht, der Testfall zu N7a und die Überschreibungen im Papp-Player sind entfallen). Was hier folgt, beschreibt die Naht, wie sie von S96 bis S98 bestand | `audio/IAudioPlayer.hpp`, `audio/AudioPlayer.hpp` / `.cpp` | ein zweiter Faktor neben der Lautstärke: `setPegel(0…1)` und `pegel()`, im Interface mit Vorgabe (wie `setTempo` seit S91); `AudioPlayer` multipliziert ihn überall dort auf, wo er die Lautstärke an die Engine gibt. Der Regler des Player-Panels und die gespeicherte Lautstärke bleiben unberührt (Entscheid Patrik S96: voller Pegel im Clip = die Lautstärke des Player-Panels) | Pegel 1 (Vorgabe) → die Lautstärke wie heute | C5 |
| N7b Zugang ✅ | `UI/widgets/VisualizerWidget.hpp` | **ein** Getter, `dienste()`: das Fenster gibt den Dienste-Behälter heraus, den es im Konstruktor bekommt (in der Basisklasse ist `services()` nur für abgeleitete Klassen sichtbar). Die Sitzung holt sich darüber Player und Engine — auch wenn das Composer-Panel nie offen war (Entscheid Patrik S96: a) | reiner Lesezugriff — am Verhalten des Fensters ändert sich nichts | C5 |
| — Playlist ruht ✅ | **keine Naht** | der Composer ruft die vorhandene `IAudioPlayer::setPlaylist(nullptr)` und hängt die Playlist beim Ausschalten wieder ein. **Belegt am echten `AudioPlayer`** (`test_ComposerNahtTon.cpp`, mit einer Audio-Engine aus Pappe): mit Playlist spielt am Titelende der nächste Song, ohne hält der Player und lädt nichts; `next`, `previous` und `playIndex` liefern false; der Composer kann dabei selbst laden und spielen | — | C5 |
| N8 Anmeldung | Panel-Registrierung, Menü, `Source.cmake`, Dateityp-Icon | Einträge. **C2a (Entscheid Patrik S94: a):** Menü File bekommt **Open composition…** und **Close composition** (`MenuAutoReg.cpp`: eine Gruppe, zwei Einträge); `MainWindow` meldet zwei Ereignisse an und reicht sie an `composer/AppAnbindung` weiter. Open lädt eine `.lvcomp`, setzt die Bildquelle und startet die Uhr sofort; Close nimmt sie weg. **C2b (S95):** `UI/panels/PanelAutoReg.cpp` bekommt einen Include und eine Anmeldung — Panel `composer`, Titel „Composer", View → Panels, anfangs nicht sichtbar; die Klasse selbst ist eigener Code (`composer/ComposerPanel`). **Wächst S100 um das Dateityp-Icon (Entscheide Patrik S100: I1 a — Effect Browser und Panel Composer; I2 — der Fächer; I2b a — mit Zeitstrahl und Abspielmarke; eingetragen vor dem Bau):** (1) `include/UI/widgets/PresetTypeIcons.hpp` — `PresetIconKind::Komposition`, Endung `lvcomp` → `lumiviz_lvcomp.ico` (fehlt die Datei, das Icon der Familie); (2) `UI/panels/ImportBrowserPanel.hpp` / `.cpp` — der Effect Browser listet `*.lvcomp` (Filter *LumiViz* und *All*) als eigene Art `Type_Komposition`; ein Doppelklick meldet `composer::OpenCompositionEvent` mit dem Pfad (derselbe Weg wie File → Open Composition, samt Rückfrage bei Ungespeichertem); eine Komposition ist kein Preset — nicht im Blättern, nicht bei den Favoriten, nicht in die Kette ziehbar. Die Icon-Datei selbst liegt unter `asset/img/logo/icons` (kein Code). **Wächst S101 um die Datei beim Start (Entscheid Patrik X1: a; eingetragen vor dem Bau):** `UI/MainWindow.cpp` — wird LumiViz mit einer `.lvcomp` als Argument gestartet (`LumiViz.exe <datei>`, etwa per Doppelklick im Explorer), meldet das Hauptfenster nach dem Wiederherstellen (N4) `composer::OpenCompositionEvent` mit dem Pfad: derselbe Weg wie File → Open Composition. Die Auswahl des Arguments und die Presets sind kein Composer-Teil (`UI/StartDatei.hpp`); dort steht von der Komposition nur die Endung. **Zweite Stelle (S101, Schritt 2, Entscheid X1b: a; eingetragen vor dem Bau):** `MainWindow::oeffneUebergebeneDatei` — gibt ein zweiter Start seine Datei an die laufende App ab (`core/EinzelInstanz`, kein Composer-Teil) und ist sie eine `.lvcomp`, meldet das Hauptfenster dasselbe Ereignis, hier mit der Rückfrage bei Ungespeichertem | — (reine Zusätze); ohne Klick auf die Einträge wie heute; das Panel ist ohne Klick nicht zu sehen | C2a, C2b |
| N9 Einstellung ✅ — **wächst S102 (Entscheid Patrik ZO6; eingetragen vor dem Bau, gebaut S102):** eine Auswahl **Scroll with playhead** im selben Block, nach *New clip length* — „Smooth“ (Vorgabe: die Abspielmarke bleibt in der Mitte) oder „By page“ (`composer/scrollWithPlayhead`, 0 oder 1; gelesen und geschrieben über `Sitzung::mitlaufenSeitenweise` / `setMitlaufenSeitenweise`); ohne den Schlüssel gilt die Vorgabe, bei Composer aus wirkt sie nicht. **Belegt:** der Fall „S102 Naht N9" in `test_ComposerPanel.cpp` baut das Settings-Panel — der erste Test, der diesen Block überhaupt anfasst. **Wächst S98** um zwei Haken im selben Block (Wunsch Patrik nach dem Hörtest, P9 / P10): **Composer off: Player resumes when the Composer hands the audio back** (`composer/playerResumes`) und **Composer on: Composition continues when the Composer is switched on again** (`composer/resumesWhenSwitchedOn`), beide Vorgabe an (von mir gesetzt, weil Patrik das Weiterspielen wünschte). **Erst nach dem Bau hier eingetragen** — gegen die Regel unter der Tabelle. **Wächst S98 ein zweites Mal** (Wunsch Patrik, vor dem Bau eingetragen): ein Zahlenfeld **New clip length** im selben Block — die Länge eines hineingezogenen Bild-Clips in Sekunden (`composer/newClipSeconds`, Vorgabe 20 wie bisher). Davor galt | `UI/panels/SettingsPanel.cpp` (ein Include, ein Block im Reiter Panels, nach Movement Table) | zwei Auswahlen. **Open Composition**: „Starts playing" (Vorgabe) oder „Holds at the start" (`composer/openHolds`). **At the end of a composition**: „Hold at the end" (Vorgabe), „Return to the start", „Switch the Composer off" (`composer/atEnd`). Gelesen und geschrieben über `composer/Sitzung` | Vorgaben = startet sofort und hält am Ende, wie seit C2a | C2b |
| N10 Beenden mit Ungespeichertem ✅ | `UI/MainWindow.cpp` (`closeEvent`, nach der Rückfrage zur Kette) | **ein** Aufruf von `composer/AppAnbindung`: hat die geladene Komposition ungespeicherte Änderungen, fragt ein Dialog Save / Discard / Cancel (Regel S89); Cancel hält das Fenster offen | ohne geänderte Komposition liefert der Aufruf sofort „weiter" — das Beenden wie heute | C3a |
| N11 Speichern einer Clip-Kette ✅ | `UI/MainWindow.cpp` (der Empfänger von `SaveEffectChainEvent`) | eine Abfrage und ein Aufruf von `composer/AppAnbindung`: hängt Effect Chain an einem Clip (Sync on Effect Chain), speichert der Composer dessen Kette — immer als Speichern unter, der Clip wechselt auf die neue Datei (Entscheid Patrik S95). Das gilt für jedes Speichern von Hand: File → Save Effect Chain / Save As..., Ctrl+S, die Preset-Zeile. **Nicht** in `speichereKette` selbst (so zuerst gebaut, dann verlegt): die ruft auch die Rückfrage zu ungespeicherten Änderungen an der Kette der App, und die soll bei der Kette der App bleiben | hängt Effect Chain nicht an einem Clip, geht das Ereignis wie heute an `speichereKette` | C3c |
| N12 Mixer ✅ | `audio/IAudioEngine.hpp`, `audio/BassEngine.hpp` / `.cpp`; `Solution.json` (`BASS_MIX` an App-Kern, Runner und Test-Target — schon mit dem Wechsel auf CMakeCraft v0.9.2 committet) | die Engine hat sieben Mixer-Aufrufe, im Interface mit Vorgabe „kann es nicht" (wie `setTempo`): `createMixer` (Stereo, Abtastrate der Engine, Puffer 0, läuft ohne Quellen mit Stille weiter), `mixerAddSource` (eine Datei als Quelle, kommt **angehalten** hinein), `mixerRemoveSource`, `getSourcePositionMs` / `setSourcePositionMs`, `setSourcePaused`, `isSourcePlaying`. Der Pegel einer Quelle ist das vorhandene `setVolume` auf ihrem Handle. Der Mixer selbst ist ein `AudioStreamHandle` — `play`, `pause`, `setVolume`, `getFFTData`, `freeStream` gelten unverändert. `BassEngine::loadPlugins` übergeht `bassmix.dll` (kein Decoder-Plugin). **Belegt** in `test_ComposerNahtMixer.cpp` an der echten `BassEngine`, nur über die Schnittstelle: Quellen angehalten, eigene Stelle, halber Pegel halbiert die Linie im Spektrum des Mixers, eine gehaltene Quelle fällt aus der Summe und ihre Stelle steht, der Mixer läuft ohne Quellen weiter, eine Quelle am Ende schweigt, bleibt aber im Mixer und spielt zurückgesetzt von selbst wieder; eine Engine ohne Mixer (`PappEngine`) meldet „kein Mixer". **Von mir gesetzt:** der Mixer läuft mit der Abtastrate der Engine (44100), damit sein Spektrum für Songs dieser Rate dieselben Linien hat wie der Stream des Players. **Entschieden (Patrik S97: b):** `BassEngine` linkt BASSmix fest, BASSmix ist damit Pflichtpaket wie `bassflac` (`externals/bass/SETUP.md` nachziehen). Der App-Kern ist eine statische Bibliothek, also braucht auch jede Test-Exe die DLL — und CMakeCraft v0.9.0 / v0.9.1 reicht `external_options` eines App-Tests nicht durch (`AppCreate.cmake:767`). **Voraussetzung für N12 war deshalb CMakeCraft v0.9.2** mit diesem Fix — **erledigt S97:** Pin auf v0.9.2, `BASS_MIX` steht an Kern, Runner und Test-Target, `bassmix.dll` liegt neben der Test-Exe (928 Cases grün). Bis N12 gebaut ist, warnt `BassEngine::loadPlugins` im Log einmal je Start über `bassmix.dll` (kein Decoder-Plugin, BASS-Fehler 41). Verworfen: BASSmix zur Laufzeit laden; DLL von Hand zum Test kopieren | niemand ruft die neuen Aufrufe → die Engine wie heute; eine Engine ohne Mixer liefert „kein Mixer" | C5b |
| N13 Spektrum ✅ | `UI/MainWindow.cpp`, `onAudioUpdate` | **eine** Abfrage bei `composer/AppAnbindung` (`composer::tonMixer`, gleich nach `currentStream()`): führt der Composer Ton, liest `onAudioUpdate` Spektrum und Wellenform von dessen Mixer statt vom Stream des Players — auch wenn der Player keinen Titel geladen hat. **Belegt:** `composer::tonMixer` in `test_ComposerNahtTon.cpp` (Composer aus und Audio-Kanal aus → `INVALID_STREAM`, führend → der Mixer); dass der Mixer der echten `BassEngine` alles liefert, was `onAudioUpdate` liest (Wellenform, zwei Kanäle, Spektrum je Kanal), in `test_ComposerNahtMixer.cpp`. **Nicht durch einen Test gedeckt:** die Zeile in `MainWindow` selbst — `MainWindow` läuft in keinem Unit-Test; ihr Beleg ist der Sichttest (das Bild reagiert auf die Summe) | Composer führt keinen Ton → der Stream des Players wie heute | C5b |
| N14 Tags eines Songs ✅ | `audio/AudioPlayer.hpp` / `.cpp` | Entscheid Patrik S98: a. Die Zeilen in `AudioPlayer::load`, die aus einem Stream die `TrackInfo` bauen (Dauer, Titel mit dem Dateinamen als Ersatz, Künstler, Album, Jahr, Genre, Tracknummer, eingebettete Songtexte), werden zur freien Funktion `leseTrackInfo(engine, handle, pfad)`; `load` ruft sie auf, der Composer für den laufenden Song auch — die Tags entstehen an einer Stelle. **Belegt vor dem Bau** (`test_ComposerNahtMixer.cpp`, dritter Fall): an einer Quelle des Mixers liefert die echte `BassEngine` dieselben Tags, denselben Songtext und dieselbe Dauer wie am Stream des Players. Verworfen: dieselben Zeilen als Kopie im Composer | reine Umstellung — `load` liefert dieselbe `TrackInfo` wie zuvor | C5b |
| N5 wächst: Anzeige ✅ (**neu gefasst und gebaut S98**) | `UI/panels/PlayerPanel.cpp` | Folge des Entscheids „das Panel zeigt den laufenden Song als ‚Composer - <Song>'" (S98, ersetzt S97): Titel, Stelle und „läuft" bekommt das Panel über die **bestehenden** Ereignisse (`TrackChangedEvent`, `PlaybackPositionEvent`, `PlaybackStateEvent`) — der Composer verschickt sie für den laufenden Song, das ist eigener Code. Am Panel **vier** Stellen, jede trägt `Composer-Naht N5, Anzeige`: (1) `onTrackChanged` — der Vorsatz „Composer - " vor dem Titel, solange der Composer Ton führt (`composer::fuehrtTon`); (2) `onProgressReleased` — fragt zuerst `composer::springeImSong` (Sprung im laufenden Song, der Composer rechnet die Stelle der Komposition); (3) `onActivate` — nimmt den Stand bei führendem Composer von `composer::tonStand` statt vom Player; (4) `updatePlayButton` — der Loop-Knopf ist ausgegraut, solange der Composer Ton führt, mit einem Tooltip, der auf das Composer-Panel verweist (dort, weil das Panel beim Übernehmen und Freigeben über „läuft / hält" hier vorbeikommt). **Der Play-Knopf folgt der Komposition** (Entscheid Patrik S98: a) — auch in einer Lücke zeigt er „Pause"; dafür meldet der Composer dem Bus „die Komposition läuft" und dem Renderer getrennt „der Song läuft". **Wächst S99 um eine fünfte Stelle (Entscheid Patrik S99: a; eingetragen vor dem Bau):** (5) der Empfänger von `TrackChangedEvent` in `subscribeToEvents` — kommt ein Songwechsel **ohne Datei**, geht das Panel in seinen Ruhezustand („No track loaded", Künstler leer, Balken 0, Zeit 00:00 / 00:00) statt „Unknown Title" zu zeigen. Den leeren Songwechsel verschickt nur der Composer, wenn er den Ton abgibt und der Player keinen Titel hat (eigener Code, `Sitzung::gibTonFrei`); der Player selbst entlädt nie. **Wächst S100 um eine sechste Stelle (Befund Patrik, Sichttest S99 A2; eingetragen vor dem Bau):** (6) `onActivate` — hat beim Sichtbarwerden weder der Player einen Titel noch führt der Composer den Ton, geht das Panel in denselben Ruhezustand (eigene Funktion `zeigeRuhezustand`, die auch Stelle 5 ruft). Grund: solange das Panel nicht zu sehen ist, hat es kein Abo am Bus und verpasst den leeren Songwechsel; vor dem Bau per Test gezählt — nach dem Einblenden stand weiter „Composer - <Song>" mit der alten Stelle. **Belegt** am Sitzungs-Fall in `test_ComposerNahtTon.cpp` mit dem echten Player-Panel: Titel mit Vorsatz beim Sichtbarwerden und nach einem Songwechsel, Loop-Knopf aus und wieder an, Klick in den Balken springt in der Komposition, Lücke (Titel bleibt, Stelle hält, „läuft" bleibt), nach dem Freigeben ohne Vorsatz | Composer führt keinen Ton → das Panel wie heute | C5b |
| N15 Tag View beim Einblenden ✅ (belegt am Sitzungs-Fall in `test_ComposerNahtTon.cpp` mit dem echten Tag-View-Panel) | `UI/panels/TagViewPanel.cpp`, `onActivate` | Befund Patrik im Hörtest S98 (O1): bei führendem Composer zeigte Tag View beim Einblenden „no track loaded" und erst nach dem nächsten Songwechsel den Song — `onActivate` holte den laufenden Titel nur vom Player. **Eine** Abfrage bei `composer/AppAnbindung` (`tonStand`) nach der des Players: führt der Composer den Ton und gab es schon einen laufenden Song, zeigt Tag View ihn (ein Include, drei Zeilen) | Composer führt keinen Ton → der Titel des Players wie immer | C5b |
| N16 Karaoke-Panel bei laufendem Composer (**Befund Patrik S101, Entscheid KP1: a; eingetragen vor dem Bau**) | `UI/panels/MultiEffectPanel.cpp` (der Empfänger von `VisualizerChangedEvent` in `connectToActiveVisualizer`); `visualizers/MultiEffectVisualizer.hpp` / `.cpp` (**R2 wächst**) | Befund: bei Composer an und Sync aus meldet die Sitzung „kein Visualizer", damit Effect Chain ruht — die zwei Editoren im Panel Karaoke hören auf dieselbe Meldung und verloren ihr Ziel; Load, Save, Default und die Trail-Presets taten nichts mehr, die Meldung „Could not load" nannte keinen Grund. Mit Sync an einem Clip hingen sie an dessen Renderer, der die Ebene auslässt. **Vor dem Bau per Test gezählt** (`test_ComposerSitzung.cpp`, Fall S101: Laden liefert false, die Ebene bleibt). **(1) Panel:** eine Abfrage am Anfang des Empfängers — hat das Panel einen der Karaoke-Bäume und stammt die Meldung vom Composer (`composer::meldetBindung`, wahr nur während die Sitzung ihre Bindung meldet), übergeht es sie: die Karaoke-Ebene gehört der App, nicht einem Clip. Effect Chain selbst folgt wie bisher. **(2) Renderer, R2 wächst:** `karaokeFassung()` — ein Zähler, den `recompileKaraoke` hochzählt (jede Änderung der Ebene läuft dort durch: Einstellen, Laden, Default, Trail-Preset). **Eigener Code dazu:** die Sitzung vergleicht je Takt die Fassung der Ebene am Renderer der App und reicht bei einer Änderung eine Kopie an die Bildquelle; die Wiedergabe setzt sie ihrem Karaoke-Träger ein (Knoten mit ihren Ids, damit das Bild des Trails eine Einstellung übersteht wie in der App) — bisher las der Träger die Ebene nur einmal beim Anlegen aus der Ablage | (1) Composer meldet nichts (aus, oder kein Bus) → das Panel folgt jeder Meldung wie heute; (2) niemand liest den Zähler → der Renderer wie heute | S101 |
| N17 Mute-Faktor am Player (**S101, Vorgabe Patrik: Mute-Knopf mit Mute-Profilen; Entscheide AM2: c — der Mute gehört dem Player und gilt immer; AM3: c — ob die Visuals ihn sehen, steht im Profil, Vorgabe nein; eingetragen vor dem Bau**) | `audio/IAudioPlayer.hpp`, `audio/AudioPlayer.hpp` / `.cpp`; `UI/MainWindow.cpp` (`onAudioUpdate`, **N13 wächst**) | **(1) Player:** ein zweiter Faktor neben der Lautstärke, `setPegel(0…1)` und `pegel()`, im Interface mit Vorgabe (wie `setTempo`); `AudioPlayer` nimmt ihn überall dort mal, wo er die Lautstärke an die Engine gibt. Das ist die Naht N7a von S96, die S98 zurückgenommen wurde — damals für die Pegelkurve eines Clips, jetzt für den Mute (wieder eingesetzt nach dem Commit `3881510`). Regler, gespeicherte Lautstärke und das harte Mute des Players bleiben unberührt, es gibt kein Ereignis. Der Composer liest den Faktor mit der Lautstärke je Takt und setzt beides an seinem Mixer (eigener Code, `Tonfuehrung::takt`) — so gilt der Mute, ob die Playlist spielt oder der Composer den Ton führt. **(2) `onAudioUpdate`, N13 wächst:** eine Abfrage `composer::muteFaktorFuerVisuals()` und das Malnehmen von Spektrum und Wellenform, bevor sie an die Visualizer gehen — nur wenn das laufende Mute-Profil „Visuals follow" trägt, sonst ist der Faktor 1 und nichts wird angefasst. Grund: die Lautstärke eines Streams ändert sein Spektrum nicht (gemessen am Mixer, `test_ComposerNahtMixer.cpp`: 3 % Lautstärke, volle Linie) | (1) niemand ruft `setPegel` → Pegel 1, die Lautstärke wie heute; (2) Faktor 1 → Spektrum und Wellenform unverändert | S101 |
| N18 Mute-Knopf im Panel Player (**S101, Entscheid Patrik AM2: c — ein Knopf im Panel Player, derselbe im Panel Composer; eingetragen vor dem Bau**) | `UI/panels/PlayerPanel.cpp` (im Aufbau der Knopfreihe, nach dem Lautstärke-Regler) | **ein** Include und **ein** Aufruf von `composer/AppAnbindung`: `erzeugeMuteKnopf(dienste, eltern)` liefert den fertigen Knopf (eigener Code, `composer/MuteKnopf`), das Panel legt ihn in seine Reihe. Der Knopf schaltet den Mute der App (`MuteSchalter`) nach dem gewählten Mute-Profil; Rechtsklick wählt das Profil und öffnet den Dialog der Profile. Der Lautsprecher-Knopf des Panels (hartes Mute), sein Kürzel und der Menüeintrag bleiben unberührt | ohne Klick auf den Knopf wie heute — der Pegel des Players bleibt 1 | S101 |
| N19 Sichttest-Steuerung, Bezug (**S104, Entscheid Patrik Z1: a — die Einbindung läuft auf `composer`; eingetragen vor dem Bau**) | `Solution.json` (External `sichttest`, Art `archive`; genannt vom Kern `LumiViz.Core` und vom Runner `LumiViz`), neue Datei `sichttest.pin` im Repo-Root | Die Steuerung ist kein Teil des Composers, ihre Aktionen werden aber Composer-Code rufen — deshalb steht sie hier. Der Bezug bringt den Include-Pfad und das Define `SICHTTEST_VORHANDEN` an Kern und Runner, dazu `SichttestSteuerung1.dll` und den Ordner `sichttest\` neben die Exe; gelinkt wird nichts, **kein Quelltext der App** ist berührt. Der Bezug braucht CMakeCraft ab v0.10.0 — **`cmakecraft.pin` steht deshalb auf `v0.10.0`** (vorher v0.9.2; beim Rückbau der Naht kann der Pin bleiben). Stand S104: Paket `v0.2.0` vom Release bei GitHub bezogen, in Release-clang, Debug-MSVC und Testing-MSVC gebaut; kein Kopf ist eingebunden, die DLL wird noch nicht geladen. **Die Naht wächst (S105, eingetragen vor dem Bau):** `sichttest.pin` v0.2.0 → **`v0.3.0`** (Projektdatei unter `.sichttest/`, Pfade ab Root; S 1.0 und die DLL unverändert) und `cmakecraft.pin` v0.10.0 → **`v0.11.0`** (die Laufzeitdateien kopiert dort ein eigenes Ziel `LumiViz_deploy_sichttest` — ein Wechsel des Pakets kommt ohne Neubau der Exe an). **Wächst noch einmal (S105, eingetragen vor dem Bau):** `sichttest.pin` v0.3.0 → **`v0.3.1`** — nur die Köpfe sind anders (`fassung()` nennt die Version der geladenen DLL), die DLL ist im Quelltext unverändert. **Und noch einmal (S105, eingetragen vor dem Bau):** `sichttest.pin` v0.3.1 → **`v0.3.2`** — nur der Tester ist anders (Haken „In dieser Sitzung nicht mehr fragen" in der Rückfrage; Hinweis mit Knopf „Tester beenden", wenn die App geordnet endet), Köpfe und Schnittstelle bleiben. **Danach (S105, eingetragen vor dem Bau):** `sichttest.pin` v0.3.2 → **`v0.3.3`** — wieder nur der Tester: sein Testlog trägt einen Block `steuerung` (Anwendung, Version, Fassung der DLL, Zustand). Die Aktionen und `--testing` sind eigene Nähte und werden vor ihrem Bau eingetragen | fehlt das Paket: Warnung W304, kein Include-Pfad, kein Define, keine Kopien — die App baut wie heute | S104 |
| N20 Sichttest-Steuerung, Start mit `--testing` (**S105; Entscheide Patrik LV-5: a, W1: a vorerst; eingetragen vor dem Bau**) | `Application.cpp` (vor der Einzelinstanz), `UI/MainWindow.hpp` (ein Zeiger), `UI/MainWindow.cpp` (ein Include, ein fünfter verzögerter Aufruf nach dem Wiederherstellen des Startzustands) | Nur mit dem Schalter `--testing`: das Hauptfenster lädt **nach** Preset, Komposition, Startdatei und Wiederaufnahme des Players die `SichttestSteuerung1.dll` aus dem Ordner der Exe, meldet die Aktionen an und verbindet mit dem Tester — der Start selbst bleibt, wie er ist (W1). Läuft dieselbe Exe schon, zeigt ein Start mit `--testing` die Meldung „LumiViz is already running. Close it and start again with --testing." und endet mit Rückgabe 1; die laufende App bekommt nichts (LV-5). Laden, Anmelden und Pfade sind eigener Code (`sichttest/Anbindung`); die Köpfe des Pakets stehen in genau dieser einen `.cpp` des Kerns, hinter `SICHTTEST_VORHANDEN`. Die Projektdatei `.sichttest/sichttest.projekt.json` (Entscheide P1: a, P2: a) nennt dem Tester Exe und Listen | ohne `--testing` wird nichts geladen und nichts geöffnet; fehlt die DLL oder das Paket, sagt das Log es in einem Satz, und die App läuft weiter. **Wächst (S105, eingetragen vor dem Bau):** im selben Block von `UI/MainWindow.cpp` **ein** weiterer Aufruf, `lumi::sichttest::meldeAppAktionen` — er meldet die Aktionen an, die nicht dem Composer gehören (`panel_zeigen`, `karaoke`, `karaoke_laden`, `titel_laden`, `player`; eigener Code, `sichttest/AppAktionen`). Die fünf weiteren Aktionen des Composers (`komposition_schliessen`, `abspielmarke`, `wiedergabe`, `schleife`, `sync_kette`) kommen über den Aufruf von N21 und brauchen keine neue Naht | S105 |
| N21 Sichttest-Steuerung, Aktionen des Composers (**S105; Entscheid Patrik L1: a mit Einstellung, Vorgabe aus; eingetragen vor dem Bau**) | `UI/MainWindow.cpp` (ein Aufruf im Block von N20), `UI/panels/SettingsPanel.cpp` (**N9 wächst** um einen Haken) | Der Aufruf `lumi::composer::meldeSichttestAktionen` meldet die Aktionen an, die Composer-Code rufen — zuerst `komposition_laden`, `composer`, `edit`, `fit`; sie sind eigener Code (`composer/SichttestAktionen`). `komposition_laden` lädt ohne Dialog über `Sitzung::lade`: Composer aus, Stelle 0:00, nichts läuft (L1: a). **Einstellung** im Block des Composers, **Sichttest: a loaded template starts playing** (`composer/sichttestPlaysAfterLoad`, Vorgabe aus): an = nach dem Laden Composer on und Abspielen. Belegt in `test_ComposerSichttestStart.cpp` | ohne `--testing` gibt es keine Aktionen; die Einstellung wirkt nur auf `komposition_laden` | S105 |
| N22 `/bigobj` für die Unit-Tests unter MSVC (**S105, Entscheid Patrik C1: a; eingetragen vor dem Bau**) | `Solution.json` (Test-Target `UnitTests` der App) | **eine** Zeile: `"compile_options": [ "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>" ]`. Kein Composer-Teil — die Tests des Composers haben die Objektdateien über die Grenze von MSVC wachsen lassen: im Debug-MSVC-Bau brach `LumiViz.UnitTests` mit C1128 ab (S104). Braucht CMakeCraft ab v0.11.0 (das Feld gibt es erst dort); clang bekommt den Schalter durch den Generator-Ausdruck nicht. Beim Rückbau darf die Zeile bleiben | ohne die Zeile baut alles wie bisher, nur die Unit-Tests im Debug-MSVC entstehen nicht | S105 |
| N26 Die Unit-Tests sehen die Köpfe von Qt-ADS (**S105; eingetragen vor dem Bau**) | `Solution.json` (Test-Target `UnitTests` der App, Liste `externals`) | **ein** Wort: `qt-ads` kommt in die `externals` des Test-Targets, wie Kern und Runner es schon nennen. Anlass: der Fall „panel_zeigen" in `test_ComposerSichttestStart.cpp` baut ein echtes Dock (`ads::CDockManager`, `ads::CDockWidget`), um zu belegen, dass die Aktion ein geschlossenes Panel wieder öffnet. Kein Composer-Teil; darf beim Rückbau bleiben | ohne das Wort übersetzt dieser eine Testfall nicht; an der App ändert sich nichts | S105 |
| N25 Verknüpfung „LumiViz.exe - Testing" neben der Exe (**S105, Wunsch Patrik; eingetragen vor dem Bau**) | `CMakeLists.txt` im Repo-Root (ein Block am Ende, nach den Lizenzdateien) | Nur unter Windows: nach jedem Bau von `LumiViz.exe` legt ein Schritt `[Sichttest]` neben die Exe die Verknüpfung **`LumiViz.exe - Testing.lnk`** — Ziel die Exe daneben, Argument `--testing`, Arbeitsordner der Ordner der Exe. Sie entsteht in jedem Build-Ordner neu und zeigt immer auf die Exe ihres Ordners. Das Anlegen selbst ist eigener Code (`tools/sichttest/TestingVerknuepfung.ps1`). Kein Composer-Teil | ohne den Block entsteht keine Verknüpfung; am Bau der Exe ändert sich nichts. Scheitert das Anlegen, bricht der Bau nicht ab | S105 |
| N24 Eine Kette ohne Rückfrage laden (**S105, Entscheid Patrik P1: a; eingetragen vor dem Bau**) | `services/events/UIEvents.hpp` (`LoadEffectChainEvent`), `UI/MainWindow.cpp` (der Empfänger dieses Ereignisses) | Das Ereignis bekommt das Feld **`confirmed`** (Vorgabe `false`) und einen zweiten Parameter im Konstruktor, wie `ImportAvsPresetEvent` und `ImportMilkPresetEvent` es seit S91 tragen; der Empfänger fragt nur dann nach der ungespeicherten Kette, wenn es nicht gesetzt ist. Gesetzt wird es allein von der Aktion `preset_laden` der Sichttest-Steuerung (eigener Code, `sichttest/AppAktionen`; Schalter `FragtNach` — der Tester fragt vorher). Kein Composer-Teil. Fehler beim Laden erscheinen weiter als Dialog in LumiViz | alle sieben bestehenden Absender (`StartDatei.hpp`, `StartPreset.hpp`, `MenuAutoReg.cpp`, `ImportBrowserPanel.cpp`, `MultiEffectPanel.cpp` zweimal, `ShadertoyBrowserPanel.cpp`) lassen das Feld auf der Vorgabe: jeder Weg fragt wie bisher | S105 |
| N23 Die eigene Kopie des Testers entfällt (**S105, Entscheid Patrik LV2: a; eingetragen vor dem Entfernen**) | `projects/exec/Sichttest/` (fünf Dateien: `README.md`, `src/main.cpp`, `src/Protokoll.cpp`, `src/Protokoll.hpp`, `src/Source.cmake`), `Solution.json` (der Eintrag `Sichttest` unter `executables`) | Ordner und Eintrag werden gelöscht. Der Tester wird seit dem 08.10.2026 in SichtTest_Helper entwickelt; LumiViz bezieht ihn als Paket (Naht N19), er liegt unter `sichttest\Sichttest.exe` neben `LumiViz.exe`. Die Kopie war der Stand von S93 und kannte weder Aktionen noch die Projektdatei — zwei Tester desselben Namens, von denen nur einer steuert. Das Target `Sichttest` gibt es danach nicht mehr; die Exe im Build-Ordner `exec/Sichttest/` bleibt als Altlast liegen, bis der Ordner neu angelegt wird | Rückbau = den Naht-Commit zurücknehmen: Ordner und Eintrag sind wieder da, der Tester baut wie seit S93 | S105 |
| — Lautstärke, Player schweigt ✅ | **keine Naht** | gebaut S98: der Composer liest Lautstärke und Mute des Players je Takt und setzt sie am Mixer (statt eines Abos auf `VolumeChangedEvent` — gleiche Wirkung, kein Bus nötig), hält den Player beim Übernehmen an und hängt die Playlist aus wie bisher; den laufenden Song meldet er über `TrackChangedEvent` und die statischen Wege des Renderers (`notifyPlayPosition`, `notifyPlayState`). **N7a (`setPegel`) ruft der Composer nicht mehr** (belegt S98 per Suche über `src` und `include`: nur noch definiert). **Entschieden (Patrik S98: a): N7a wird zurückgenommen, sobald der Hörtest zeigt, dass der Mixer trägt** — nicht vorher, weil ein Rückweg auf den Stand von S97 die Naht noch braucht. Dann von Hand, nicht per `git revert`: der Commit `3881510` trägt auch N7b, und die bleibt. Bis dahin steht ihr Beleg in `test_ComposerNahtTon.cpp`. Verworfen: sofort zurücknehmen; stehen lassen | — | C5b |

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
| C2b, Sitzung | `include/composer/Sitzung.hpp`, `src/composer/Sitzung.cpp`, `tests/unit/UnitTests/test_ComposerSitzung.cpp`; die Schleife in `Uhr.hpp` und der gespiegelte Stand in `Bildquelle.hpp` gehen mit den Dateien | keine Naht | eine Quelle und ein Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis); eine Zeile in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen. Zurück bleiben die Schlüssel `composer/resume`, `composer/loop`, `composer/openHolds`, `composer/atEnd`, `composer/session/…` |
| C2b, Panel | `include/composer/ComposerPanel.hpp`, `src/composer/ComposerPanel.cpp`, `tests/unit/UnitTests/test_ComposerPanel.cpp` | keine Naht (die Anmeldung folgt als N8) | eine Quelle und ein Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis); eine Zeile in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen |
| C2b, Nähte N8 / N4 / N9 | — | drei Naht-Commits, je eine Datei: **N8 `acb8a9b`** `UI/panels/PanelAutoReg.cpp` (ein Include, eine Anmeldung `composer`); **N4 `1815da7`** `UI/MainWindow.cpp` (ein verzögerter Aufruf im Konstruktor); **N9 `4ac6be9`** `UI/panels/SettingsPanel.cpp` (ein Include, ein Block mit zwei Auswahlen). Der Stand zwischen `acb8a9b` und `612289c` baut nicht (der Header des Panels brauchte den vollen Typ der Sitzung) — für den Rückbau ohne Belang, für ein `git bisect` schon. Jede Stelle trägt `Composer-Naht N8` / `N4` / `N9` | — | vor dem Holen wie oben prüfen; Pfade des Composers um `UI/panels/PanelAutoReg.cpp` und `UI/panels/SettingsPanel.cpp` erweitert. Im gespeicherten Dock-Layout bleibt der Eintrag `composer` zurück |
| C3c, Sync on Effect Chain | keine neuen Dateien — die Erweiterungen in `Wiedergabe`, `Bildquelle`, `Sitzung`, `ComposerPanel` und `AppAnbindung` gehen mit den Dateien | **N11 `356a868`** — `UI/MainWindow.cpp` (Empfänger von `SaveEffectChainEvent`: eine Abfrage, ein Aufruf), trägt `Composer-Naht N11`. **Dieser Commit enthält die Naht und eigenen Code zusammen** (C3c Schritt 2; versehentlich als einer committet) — beim Rückbau die Naht-Stelle von Hand herausnehmen oder den ganzen Commit zurücknehmen. Das Binden selbst hat keine Naht | — | vor dem Holen wie oben prüfen. Zurück bleibt der Schlüssel `composer/syncEffectChain` |
| C3b, Neues hinein | keine neuen Dateien — die Erweiterungen in `Bearbeiten`, `Sitzung`, `ComposerPanel` und `AppAnbindung` gehen mit den Dateien | keine Naht | — | vor dem Holen wie oben prüfen |
| C3a, Bearbeiten | `include/composer/Bearbeiten.hpp`, `src/composer/Bearbeiten.cpp`, `tests/unit/UnitTests/test_ComposerBearbeiten.cpp`; die Erweiterungen in `Sitzung`, `Bildquelle`, `ComposerPanel` und `AppAnbindung` gehen mit den Dateien | **N10 `4826db1`** — `UI/MainWindow.cpp` (`closeEvent`: ein Aufruf von `composer::darfKompositionErsetzen`), trägt `Composer-Naht N10` | eine Quelle und ein Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis); eine Zeile in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen |
| C4 Teil 2, Pegelform und Presets (S96) | `include/composer/` (Pegelform.hpp, FormPresets.hpp, KurvenEditor.hpp), `src/composer/` (Pegelform.cpp, FormPresets.cpp, KurvenEditor.cpp), `tests/unit/UnitTests/` (test_ComposerPegelform.cpp, test_ComposerFormPresets.cpp); **dazu der Ordner `asset/composer/`** (mitgelieferte Kurven-Presets, liegt außerhalb der Composer-Pfade); die Umstellung von `VisualClip` geht mit `Komposition`, `Bearbeiten`, `ComposerPanel` | keine Naht | drei Quellen und drei Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis); zwei Zeilen in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen; `asset/composer/` eigens löschen |
| C5, Audiokanal (S96, im Bau) | `include/composer/Tonfuehrung.hpp`, `src/composer/Tonfuehrung.cpp`, `tests/unit/UnitTests/` (test_ComposerTonfuehrung.cpp, test_ComposerNahtTon.cpp, PappTon.hpp); das Modell (ein Clip-Typ, `Kanal::clips`) geht mit `Komposition` und `Bearbeiten`, die Führung des Players mit `Sitzung`, der Transport mit `AppAnbindung` | **N7a** — `audio/IAudioPlayer.hpp` (`pegel` / `setPegel` mit Vorgabe), `audio/AudioPlayer.hpp` / `.cpp` (Feld `pegel`, `laut()`, an drei Stellen statt der Lautstärke an die Engine); **N7b** — `UI/widgets/VisualizerWidget.hpp` (Getter `dienste()`); **N5 Transport** — `UI/panels/PlayerPanel.cpp` (ein Include, eine Abfrage am Anfang von `onTransportCommand`). Alle tragen `Composer-Naht`. Commits: **N7a und N7b `3881510`** (für sich baubar), dann Eigenes `9f6a7f1`, dann **N5 `e54ad39`** (ruft `composer::transport`; der Test über das Player-Panel in `test_ComposerNahtTon.cpp` ist erst mit diesem Commit grün) | eine Quelle und ein Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis); zwei Zeilen in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen |
| Belege vor C5b (S97) | `tests/unit/UnitTests/test_ComposerBelegeTon.cpp` (Commit `72dfb06`) | keine | eine Zeile in `tests/unit/UnitTests/Source.cmake` | nicht nötig — nur Tests, kein bestehender Code berührt |
| C5b, Naht N12 Mixer (S97, im Bau) | `tests/unit/UnitTests/test_ComposerNahtMixer.cpp` | **N12 `e7fe2d5`** — **dieser Commit enthält die Naht und Eigenes zusammen** (Test, Quellenliste, Konzept; als einer committet): beim Rückbau den ganzen Commit zurücknehmen oder die Naht-Stellen von Hand herausnehmen. Die Naht: `audio/IAudioEngine.hpp` (Block „Mixer", sieben Aufrufe mit Vorgabe), `audio/BassEngine.hpp` / `.cpp` (Include `bassmix.h`, Block „Mixer", eine Abfrage in `loadPlugins`); jede Stelle trägt `Composer-Naht N12`. **Nicht Teil des Naht-Commits:** `BASS_MIX` in `Solution.json` und der Pin auf CMakeCraft v0.9.2 stehen im Commit **`b31b55a`** — beim Rückbau `BASS_MIX` an den drei Zielen herausnehmen und `externals/bass/SETUP.md` zurückstellen; der Pin kann bleiben | eine Zeile in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen; Pfade des Composers um `audio/IAudioEngine.hpp` und `audio/BassEngine.*` erweitert |
| C5b, Schritt 2: Führung über den Mixer (S98) | keine neuen Dateien — `Tonfuehrung` (neu geschrieben), `Komposition::toeneBei`, `Sitzung::fuehreTon` / `gibTonFrei` / `tonMixer`, `PappMischEngine` in `PappTon.hpp` und die umgeschriebenen Fälle in `test_ComposerTonfuehrung.cpp` / `test_ComposerNahtTon.cpp` gehen mit den Dateien; der dritte Fall in `test_ComposerNahtMixer.cpp` (Tags an einer Quelle) ebenso | **N14** — `audio/AudioPlayer.hpp` (Deklaration `leseTrackInfo`), `audio/AudioPlayer.cpp` (die Funktion; `load` ruft sie auf); beide Stellen tragen `Composer-Naht N14`. Reine Umstellung: beim Rückbau kann sie stehen bleiben oder zurückgenommen werden. Commit: **`6f87331`** (nur Naht); Eigenes danach `3e18e15` | — | vor dem Holen wie oben prüfen; Pfade des Composers um `audio/AudioPlayer.*` erweitert |
| C5b, Schritt 3: Naht N13 Spektrum (S98) | keine neuen Dateien — `composer::tonMixer` in `AppAnbindung` und die Prüfungen in `test_ComposerNahtTon.cpp` / `test_ComposerNahtMixer.cpp` gehen mit den Dateien | **N13** — `UI/MainWindow.cpp` (`onAudioUpdate`: eine Abfrage nach `currentStream()`), trägt `Composer-Naht N13`. Commit: **`3b8161c`** (nur Naht); Eigenes davor `9bf1a93`. Der Naht-Commit folgt **nach** dem eigenen, weil er `composer::tonMixer` braucht — beim Rückbau zuerst ihn zurücknehmen | — | vor dem Holen wie oben prüfen |
| C5b, Schritt 4: Anzeige des Player-Panels (S98) | keine neuen Dateien — `composer::fuehrtTon` / `tonStand` / `springeImSong` in `AppAnbindung`, `Sitzung::tonStand` / `springeImSong` und die Meldungen an den Bus, `Tonfuehrung::stelleImLaufenden` und die Prüfungen in `test_ComposerNahtTon.cpp` / `test_ComposerTonfuehrung.cpp` gehen mit den Dateien | **N5, Anzeige** — `UI/panels/PlayerPanel.cpp`, vier Stellen (`onActivate`, `onProgressReleased`, `onTrackChanged`, `updatePlayButton`), jede trägt `Composer-Naht N5, Anzeige`. Commit: **`a22e797`** (nur Naht); Eigenes davor `473ea27`. Der Naht-Commit folgt **nach** dem eigenen, weil er die drei Funktionen aus `AppAnbindung` braucht — beim Rückbau zuerst ihn zurücknehmen. Nach `473ea27` allein ist der Sitzungs-Fall in `test_ComposerNahtTon.cpp` rot (er prüft das Panel), erst mit `a22e797` grün | — | vor dem Holen wie oben prüfen |
| C5b, nach dem Hörtest (S98) | keine neuen Dateien — `Bildquelle::standStelleJetztMs`, der Grundabstand in `Tonfuehrung`, das Weiterspielen in `Sitzung` / `Tonfuehrung` und die Tests (Messung in `test_ComposerNahtMixer.cpp`, Fall in `test_ComposerBildquelle.cpp`) gehen mit den Dateien | **N15** — `UI/panels/TagViewPanel.cpp` (ein Include, eine Abfrage in `onActivate`); **N9 wächst** — `UI/panels/SettingsPanel.cpp` (zwei Haken im Composer-Block); **N7a zurückgenommen** — `audio/IAudioPlayer.hpp`, `audio/AudioPlayer.hpp` / `.cpp`. Commit: **`4b3e652`** (nur Nähte); Eigenes davor `7d3b776` — dazwischen ist der Sitzungs-Fall rot (er prüft Tag View); er folgt nach dem eigenen, weil die Haken die Einstellungen der Sitzung brauchen. Beim Rückbau ist N7a **nicht** wieder einzubauen — sie gehört zum Composer | — | vor dem Holen wie oben prüfen; Pfade des Composers um `UI/panels/TagViewPanel.cpp` erweitert. Zurück bleiben die Schlüssel `composer/playerResumes` und `composer/resumesWhenSwitchedOn` |
| Bedienung der Zeitachse (S98): Einfügemarke, Kopieren / Einfügen, Schatten, Tooltip, Vorgabelänge | keine neuen Dateien — `Bearbeiten::fuegeKopieEin`, die Erweiterungen in `Zeitachse` / `ComposerPanel`, `Sitzung::neuerClipMs` und die Tests gehen mit den Dateien | **N9 wächst** — `UI/panels/SettingsPanel.cpp` (ein Zahlenfeld *New clip length* im Composer-Block). Commit: **`45ead4e`** (nur Naht); Eigenes davor `f08d6bb` | — | vor dem Holen wie oben prüfen. Zurück bleibt der Schlüssel `composer/newClipSeconds` |
| Freigeben ohne Titel im Player (S99) | keine neuen Dateien — der leere Songwechsel in `Sitzung::gibTonFrei` und der Fall „Composer off ohne Titel im Player" in `test_ComposerNahtTon.cpp` gehen mit den Dateien | **N5, Anzeige, fünfte Stelle** — `UI/panels/PlayerPanel.cpp` (`subscribeToEvents`, im Empfänger von `TrackChangedEvent`: ein Songwechsel ohne Datei setzt das Panel in den Ruhezustand), trägt `Composer-Naht N5, Anzeige`. Commit: **`3624e1f`** (nur Naht); Eigenes davor `e3043ae`. Nach dem eigenen Commit allein ist der neue Fall rot (er prüft das Panel), erst mit der Naht grün | — | vor dem Holen wie oben prüfen |
| Überlappung in der Schleife und Blende der ganzen Komposition (S99, sieben Schritte) | keine neuen Dateien — `Komposition` (Werte, `Schleifenlage`, Datei), `Uhr`, `Bildquelle`, `Wiedergabe` (zweite Stelle, `m_frische`), `Tonfuehrung` (Stimmen), `Sitzung::zweiteStelleMs`, `ComposerPanel` / `Zeitachse` (Felder, Fenster, Keile) und die Tests gehen mit den Dateien | **keine Naht.** Commits, alle Eigenes: `fb412a7` (Modell und Datei), `1a1503d` (Uhr, Bildquelle), `14edf71` (Bild) mit `a2c49f7` (clang-Fix — **dazwischen baut der Stand mit clang nicht**), `37f9d4f` (Ton), `37f48bd` (Panel, Zeitachse), `f217900` (Blende), `c361dcf` (Visuals overlap themselves) | — | vor dem Holen wie oben prüfen. In `.lvcomp`-Dateien, die damit gespeichert wurden, bleiben die Schlüssel `loop` und `fade` — ein älterer Stand überliest sie |
| Player-Panel wird nach dem Freigeben erst eingeblendet (S100, Sichttest S99 A2) | keine neuen Dateien — die Erweiterung des Falls „Composer off ohne Titel im Player" in `test_ComposerNahtTon.cpp` geht mit der Datei | **N5, Anzeige, sechste Stelle** — `UI/panels/PlayerPanel.cpp` / `.hpp` (`onActivate`: ohne Titel im Player und ohne führenden Composer der Ruhezustand; neue private Funktion `zeigeRuhezustand`, die auch der Empfänger von Stelle 5 ruft), trägt `Composer-Naht N5, Anzeige`. Commit: **`da5339f`** (Naht und Test in einem — ohne die Naht ist die Erweiterung des Falls rot) | — | vor dem Holen wie oben prüfen |
| Karaoke-Panel bei laufendem Composer (S101, Befund Patrik) | keine neuen Dateien — `Sitzung::gleicheKaraokeAb` / `meldetBindung`, `Bildquelle::setzeKaraokeEbene`, `Wiedergabe::setzeKaraokeEbene`, `composer::meldetBindung` in `AppAnbindung` und der Fall S101 in `test_ComposerSitzung.cpp` gehen mit den Dateien | **N16** — `UI/panels/MultiEffectPanel.cpp` (ein Include, eine Abfrage im Empfänger von `VisualizerChangedEvent`), trägt `Composer-Naht N16`; **R2 wächst** — `visualizers/MultiEffectVisualizer.hpp` (`karaokeFassung`, ein Feld), `.cpp` (eine Zeile in `recompileKaraoke`), tragen `Composer-Naht R2`. Commit: **`7e62922`** — **dieser eine Commit enthält beide Nähte und den eigenen Code zusammen** (geplant waren drei: R2, Eigenes, N16 — jede Seite braucht die andere). Beim Rückbau den ganzen Commit zurücknehmen oder die drei Naht-Stellen von Hand herausnehmen (sie tragen `Composer-Naht N16` bzw. `R2`) | — | vor dem Holen wie oben prüfen; Pfade des Composers um `UI/panels/MultiEffectPanel.cpp` erweitert |
| Eine `.lvcomp` als Argument beim Start (S101) | keine — `UI/StartDatei.hpp` ist kein Composer-Teil und bleibt stehen (dort steht von der Komposition nur die Endung) | **N8 wächst** — `UI/MainWindow.cpp` (im Konstruktor, im Block nach N4: eine Abfrage und ein `publish` von `composer::OpenCompositionEvent`), trägt `Composer-Naht N8`. Commit: **`12c870b`** — **er enthält auch den übrigen Schritt** (`UI/StartDatei.hpp`, die Zeilen für Presets im Hauptfenster, Test): beim Rückbau nur die Naht-Stelle von Hand herausnehmen; `startdatei::Art::Komposition` kann bleiben, eine `.lvcomp` als Argument tut dann nichts. **Zweite Stelle (Schritt 2, eine Instanz):** `MainWindow::oeffneUebergebeneDatei` (eine Abfrage, ein `publish`), trägt `Composer-Naht N8`; Commit: **`313423c`** — auch er trägt den übrigen Schritt (`core/EinzelInstanz`, `Application`, `main.cpp`, Test, Handbuch §17), beim Rückbau nur die Naht-Stelle herausnehmen | — | vor dem Holen wie oben prüfen |
| Zoom und Rollen der Zeitachse (S102, sechs Schritte) | keine neuen Dateien — alles in `include/composer/ComposerPanel.hpp`, `src/composer/ComposerPanel.cpp` (`Zeitachse`: Rollen, Zeit-Zoom, Höhen-Zoom, Schwenken, Mitlaufen; die neue Klasse `ZeitachsenRahmen` mit den Rollbalken; im Panel der Schalter *Scroll with playhead*, der Knopf *Fit*, Ctrl+0), `Sitzung.hpp` / `.cpp` (`mitlaufenAn`, `mitlaufenSeitenweise`) und acht Fälle „S102 …“ in `test_ComposerPanel.cpp` (der neunte belegt die Naht) — geht mit den Dateien. Zurück bleiben die Schlüssel `composer/rowHeightPercent`, `composer/scrollWithPlayheadOn`, `composer/scrollWithPlayhead` | Eigenes, je Schritt ein Commit: **`ed32789`** Rollen (das Lineal bleibt stehen), **`4a5882c`** Zeit-Zoom, **`02140d9`** Höhen-Zoom, **`97e67ef`** Schwenken, Fit, Ctrl+0, **`76ab153`** Mitlaufen. **Naht, eigener Commit: `640cac5`** — **N9 wächst** (`UI/panels/SettingsPanel.cpp`, 15 Zeilen: die Auswahl *Scroll with playhead*; ihr Beleg, der Fall „S102 Naht N9“, liegt in `test_ComposerPanel.cpp`). Naht und Eigenes sind hier getrennt | keine — keine neue Datei, kein Eintrag in einer `Source.cmake` | ⬜ nicht geprüft; vor dem Holen wie oben prüfen, Pfade des Composers um `UI/panels/SettingsPanel.cpp` erweitert (schon wegen N9) |
| Master-Zeilen, Karaoke im Composer und Mute (S101) | neu: `include/composer/Mute.hpp`, `MuteKnopf.hpp`, `src/composer/Mute.cpp`, `MuteKnopf.cpp`, `tests/unit/UnitTests/test_ComposerMute.cpp`, **der Ordner `asset/composer/mutes`** (liegt außerhalb der Composer-Pfade); alles Übrige (Master-Zeilen auf 60 %, `MasterKurve` mit `bildKurve` / `tonKurve` / `karaokeKurve`, die vier Zeilen der Zeitachse, der Haken Karaoke) geht mit `Komposition`, `Wiedergabe`, `Tonfuehrung`, `ComposerPanel`, `AppAnbindung` | **Jeder dieser Commits trägt Naht und Eigenes zusammen** — beim Rückbau die Naht-Stellen von Hand herausnehmen oder den ganzen Commit zurücknehmen. **`e1e6664`** Karaoke im Composer: **R2 wächst** — `MultiEffectVisualizer.hpp` (`legeKaraokeEbeneAuf(pegel)`, `legeKaraokeAuf(pegel)`), `.cpp` (Parameter, Klemmen, ein Faktor an `uStaerke`). **`725c5cc`** Mute am Ton: **N17** — `audio/IAudioPlayer.hpp` (Block „Pegel"), `audio/AudioPlayer.hpp` / `.cpp` (Feld `pegel`, `laut()` an drei Stellen, `setPegel`); **N13 wächst** — `UI/MainWindow.cpp`, zwei Stellen in `onAudioUpdate`. **`735062b`** Mute-Knopf: **N18** — `UI/panels/PlayerPanel.cpp` (ein Aufruf nach dem Lautstärke-Regler). Ohne Naht: `49d3cf6` (Master-Zeilen 60 %), `39f8443` (VISUAL und AUDIO), `219087b` (Mute-Profil, Verlauf, Dateien) | zwei Quellen und zwei Header in `src/composer/Source.cmake` (geht mit dem Verzeichnis); eine Zeile in `tests/unit/UnitTests/Source.cmake` | vor dem Holen wie oben prüfen; Pfade des Composers um `UI/panels/PlayerPanel.cpp` (schon wegen N5) und `audio/IAudioPlayer.hpp` erweitert; `asset/composer/mutes` eigens löschen. Zurück bleibt der Schlüssel `composer/muteProfile`; in `.lvcomp`-Dateien die Schlüssel `visualMaster`, `audioMaster`, `karaokeMaster` — ein älterer Stand überliest sie |
| Vorlagen der Sichttests (S103) | `tests/unit/UnitTests/test_ComposerVorlagen.cpp`; **dazu der Ordner `asset/sichttest/`** (liegt außerhalb der Composer-Pfade): elf Vorlagen `composer_A_kurz.lvcomp` bis `composer_J_einrasten.lvcomp`, `fehlt_ein_preset.lvcomp`, `presets/` (acht Kopien aus `asset/calibration/avs/matrix` unter sprechenden Namen), `audio/` (sieben Testsignale als MP3 und ein Songtext `.lrc`) | keine Naht — kein Code der App berührt | eine Zeile in `tests/unit/UnitTests/Source.cmake` | nicht nötig — nur ein Test und Dateien; `asset/sichttest/` eigens löschen. Der Hinweis auf die Beispiele im Benutzerhandbuch §15 geht mit dem Abschnitt |
| Dateityp-Icon der Komposition (S100) | `asset/img/logo/icons/lumiviz_lvcomp.ico` und sein Vorschaubild `docs/bilder/handbuch/icons/lumiviz_lvcomp.png`; die Funktion `komposition` in `build_lvfx2_icons.py`; der Fall „Composer S100: .lvcomp im Effect Browser" in `test_PaletteUi.cpp` (Naht-Beleg, liegt außerhalb von `test_Composer*`); das Icon im Panel Composer geht mit den Dateien | **N8 wächst** — `include/UI/widgets/PresetTypeIcons.hpp` (`PresetIconKind::Komposition`, Endung `lvcomp`), `UI/panels/ImportBrowserPanel.hpp` / `.cpp` (`Type_Komposition`, `*.lvcomp` in `currentNameFilters`, Zweig in `openEntry`, kein Favorit, nicht ziehbar; `#include "composer/AppAnbindung.hpp"`), tragen `Composer-Naht N8`; dazu die Zeile `.lvcomp` in `register_file_icons.ps1`, in der README der Icons und im Benutzerhandbuch §16. Commit: **`ef57574`** (Naht, Eigenes und das Icon der Exe in einem) | — | vor dem Holen wie oben prüfen; wer `.lvcomp` im Explorer eingetragen hat, nimmt es mit `register_file_icons.ps1 -Remove` heraus |
| Pegelkurve der ganzen Komposition (S100, zwei Schritte) | keine neuen Dateien — Schritt 1: `Komposition` (Formen der Blenden, Grundpegel, `GesamtStueck`, `kurveAlsClip`, Datei) und der Fall „Kurve der Komposition S100, Modell" in `test_ComposerWiedergabe.cpp`; Schritt 2: die Zeile MASTER in `Zeitachse` / `ComposerPanel` und die Erweiterung in `test_ComposerPanel.cpp` — alles geht mit den Dateien | keine Naht — eigener Code. Commits: Schritt 1 **`4150554`**, Schritt 2 **`b9687bf`**; das Preset Late fade in **`967eeda`**; Kanalfarben (`Kanal::farbe`, `reihenFarbe`, `kanalFaerben`, Schlüssel `channels[].color`) **`9c9c5c5`**; das Ende an der Zeile MASTER ziehen **`5e56a4a`**; Kanäle verschieben (`verschiebeKanal`, `kanalVerschieben`) **`1a30181`**; nach Art ordnen (`sortiereNachArt`, `kanaeleSortieren`) **`c501653`** | — | vor dem Holen wie oben prüfen. In `.lvcomp`-Dateien, die damit gespeichert wurden, bleiben `fade.inShape` / `fade.outShape` und der Schlüssel `level` — ein älterer Stand überliest sie |
| Sichttest-Steuerung, `--testing` und Aktionen (S105, Nähte N20, N21) | `include/sichttest/` und `src/sichttest/` (Anbindung.hpp, Anbindung.cpp, AppAktionen.hpp, AppAktionen.cpp, Source.cmake), `include/composer/SichttestAktionen.hpp`, `src/composer/SichttestAktionen.cpp`, `.sichttest/sichttest.projekt.json` (Repo-Root), `tests/unit/UnitTests/test_ComposerSichttestStart.cpp`; dazu im eigenen Code `Sitzung::sichttestSpieltNachLaden` / `setSichttestSpieltNachLaden` und `ComposerPanel::setBearbeiten` / `bearbeiten` | **N20** — `Application.cpp` (drei Includes, der Block vor der Einzelinstanz), `UI/MainWindow.hpp` (Vorwärtsdeklaration, ein Zeiger), `UI/MainWindow.cpp` (drei Includes, der fünfte verzögerte Aufruf samt dem Aufruf `meldeAppAktionen`); **N21** — der eine Aufruf `meldeSichttestAktionen` im Block von N20 und der Haken im Block des Composers in `UI/panels/SettingsPanel.cpp` (N9 wächst); jede Stelle trägt `Composer-Naht N20` oder `N21` | `include(…/sichttest/Source.cmake)` in `src/Source.cmake`; zwei Zeilen in `src/composer/Source.cmake`, eine in `tests/unit/UnitTests/Source.cmake`; der Schlüssel `composer/sichttestPlaysAfterLoad` bleibt in den Einstellungen zurück | vor dem Holen wie oben prüfen |
| Qt-ADS für die Unit-Tests (S105, Naht N26) | keine | **N26** — `Solution.json`: das Wort `qt-ads` in den `externals` des Test-Targets `UnitTests`; darf beim Rückbau bleiben — nur Naht | keine | vor dem Holen wie oben prüfen |
| Verknüpfung „LumiViz.exe - Testing" (S105, Naht N25) | `tools/sichttest/TestingVerknuepfung.ps1` | **N25** — `CMakeLists.txt` (Repo-Root): der Block `[Sichttest]` am Ende; trägt `Composer-Naht N25` | keine; die schon angelegten `.lnk` in den Build-Ordnern bleiben liegen (ignoriert) | vor dem Holen wie oben prüfen |
| Eine Kette ohne Rückfrage laden (S105, Naht N24) | keine (die Aktion `preset_laden` liegt in `src/sichttest/AppAktionen.cpp` und geht mit der Steuerung) | **N24** — `services/events/UIEvents.hpp`: Feld `confirmed` und zweiter Parameter an `LoadEffectChainEvent`; `UI/MainWindow.cpp`: die Bedingung `!ev.confirmed &&` vor der Rückfrage; trägt `Composer-Naht N24` — nur Naht | keine | vor dem Holen wie oben prüfen |
| Die eigene Kopie des Testers entfällt (S105, Naht N23) | keine | **N23** — gelöscht: `projects/exec/Sichttest/` und der Eintrag `Sichttest` in `Solution.json`; den Commit zurückzunehmen bringt beides wieder — nur Naht | keine | vor dem Holen wie oben prüfen |
| `/bigobj` für die Unit-Tests (S105, Naht N22) | keine | **N22** — `Solution.json`: die Zeile `compile_options` am Test-Target `UnitTests`; darf beim Rückbau bleiben — nur Naht | keine | vor dem Holen wie oben prüfen |
| Sichttest-Steuerung, Bezug (S104, Naht N19) | `sichttest.pin` (Repo-Root; kein Composer-Teil) | **N19** — `Solution.json`: der Eintrag `sichttest` im Block `externals` und das Wort `sichttest` in den `externals` von Kern und Runner; dazu `cmakecraft.pin` v0.9.2 → v0.10.0 (darf beim Rückbau bleiben); Commit **`5d49259`** — nur Naht. S105: beide Pins gehoben (`sichttest.pin` v0.3.0, `cmakecraft.pin` v0.11.0 — darf beim Rückbau ebenfalls bleiben), eigener Naht-Commit; danach `sichttest.pin` v0.3.1, v0.3.2 und v0.3.3, je ein eigener Naht-Commit | keine Quelle, kein Eintrag in einer `Source.cmake`; der Zwischenspeicher `.externals/sichttest/` ist ignoriert | vor dem Holen wie oben prüfen |

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
| 0.96.0 | 2026-10-16 | Session 130 — §9h: Sichttest 11 C1, Entscheid **C1 c**: das Panel Player zeigt einen ausgeschalteten Clip an der Stelle mit „(off)", ohne Songwechsel |
| 0.95.0 | 2026-10-16 | Session 130 — §9g: Sichttest 11 L2 gemessen (der Limiter ohne Vorschau knackte), Entscheid **L2-1 b**: 5 ms Vorschau, Verzögerung nur bei gesetztem Haken |
| 0.94.0 | 2026-10-16 | Session 128 — §9f: der Loop hängt nicht mehr am Composer (N1 b: `loop/ComposerAbfragen`, `UI/widgets/Farbreihen`) |
| 0.93.0 | 2026-10-15 | Session 127 — §9g G5-4: Nachtrag Patrik, der Knopf Match loudness ist immer da |
| 0.92.0 | 2026-10-15 | Session 127 — §9g G6: Sichttest-Liste 11 mit Vorlage L, voller Lauf; G1 bis G6 fertig |
| 0.91.0 | 2026-10-15 | Session 127 — §9g G5 gebaut: der Dialog Match loudness (`composer/Abgleich`, `composer/LautheitsDialog`), Knopf neben Gain, Rechtsklick, Edit grau; Stolperstein der eigenen Meldung beim Setzen — G5 fertig |
| 0.90.0 | 2026-10-15 | Session 127 — §9g G5: die Messung je Ton-Clip gebaut (`decodeStereoChunks`, `composer/ClipLautheit`, Cache für Songs), Dekodieren gemessen (235 ms je Song), Entscheid **G5-3b a** (Cache bleibt); §11 eine Zeile |
| 0.89.0 | 2026-10-15 | Session 127 — §9g G4 Teil 2 gebaut: `Komposition::limiter`, Datei, die Tonführung schaltet ihn am Mixer, Haken **Limiter** mit Undo — G4 fertig; Entscheide **G5-1 a** (Vorhören: Dialog sperrt nicht, Listen A/B, ▶ je Zeile), **G5-2 a** (ein Undo-Schritt beim OK, Edit grau), **G5-3 a** (Messen im Hintergrund, Songs im Cache) und **G5-4 a** (Knopf neben Gain, Rechtsklick am Ton-Clip); §11 vier Zeilen |
| 0.88.0 | 2026-10-15 | Session 126, Abschluss — §9g G4: Teil 1 gebaut (Begrenzer ohne Vorschau, Engine-Schnittstelle), Entscheid **G4b a** (Haken in der Zeile der Komposition); §11 eine Zeile |
| 0.87.0 | 2026-10-15 | Session 126 — §9g G4: Beleg, was die Visuals hinter einem DSP am Mixer sehen; **Befund: G1 wirkte in der echten Engine nicht über 0 dB** (`BassEngine::setVolume` klemmte auf 1), behoben mit `kHoechsteLautstaerke` = 16 |
| 0.86.0 | 2026-10-15 | Session 126 — **§9h neu: einzelne Clips ausschalten** (Entscheid D1 c): *Clip enabled* im Rechtsklick, Ctrl+E, `enabled` in der `.lvcomp`; §11 eine Zeile; Kopf |
| 0.85.0 | 2026-10-15 | Session 126 — §9g: **G2** gebaut (Entscheid G2 a: Feld Gain statt Blend am Ton-Clip, Marke im Clip, Reset gain im Rechtsklick); Kopf |
| 0.84.0 | 2026-10-15 | Session 126 — §9g: **G1** (Gain je Ton-Clip in Modell, Datei und Tonführung) und **G3** (Messbaustein `composer/Lautheit` nach EBU R128, belegt an Tech 3341 Fälle 1–5; Beleg L3b: ein Loop-Durchlauf gleicht dem nächsten) gebaut; Baustufen freigegeben; Kopf. Im Programm nichts gesehen |
| 0.83.0 | 2026-10-15 | Session 126 — **§9g neu: Lautstärke-Abgleich der Ton-Clips** (Anlass Patrik: `compare.lvcomp`, der Loop viel leiser als das Original). Entscheide L1 a (Gain je Ton-Clip in dB), L2 a (EBU R128, LUFS), L3 a (Target: Clip oder Fixed level), L3b a (gespielter Ausschnitt ohne Pegelkurve), L4 a (Limiter am Mixer); die Skizze des Dialogs Match loudness als Beschreibung; Baustufen G1 bis G6 vorgelegt, nicht freigegeben; §11 fünf Zeilen; Kopf. Nichts gebaut |
| 0.82.0 | 2026-10-11 | Session 110 — **§9f neu** (Nachtrag zu S109 und S110): die gemeinsame Basis `lumi::ui::Achse` für Zeitachse und Loop-Panel, der Loop als Clip im Audio-Kanal (Klangerzeuger in der Tonführung, kein Ende, laufender Song aus den Tags, Verbindung zum Loop-Panel), der Haken Karaoke stimmt nach dem Start; §3 und Kopf nachgezogen |
| 0.81.0 | 2026-10-09 | Session 105 — §12: **Rückbau-Regeln abgeschlossen** (Entscheid Patrik M1: a): `composer` ist nach `master` überführt, Marke `vor-composer` auf dem Stand davor; die Nahtlisten bleiben als Beschreibung, werden aber nicht weitergeführt |
| 0.80.0 | 2026-10-09 | Session 105 — §12.2 neue Naht **N26** (`qt-ads` in den `externals` des Test-Targets, für den Testfall mit echtem Dock; eingetragen vor dem Bau), §12.4 Rückbau-Liste |
| 0.79.0 | 2026-10-09 | Session 105 — §12.2 und §12.4: **Naht N19 wächst** (eingetragen vor dem Bau): `sichttest.pin` auf v0.3.3 (nur der Tester: Block `steuerung` im Testlog) |
| 0.78.0 | 2026-10-09 | Session 105 — §12.2 neue Naht **N25** (Verknüpfung „LumiViz.exe - Testing" neben der Exe nach jedem Bau, Wunsch Patrik, eingetragen vor dem Bau), §12.4 Rückbau-Liste |
| 0.77.0 | 2026-10-09 | Session 105 — §12.2 und §12.4, beides **eingetragen vor dem Bau**: **Naht N19 wächst** (`sichttest.pin` auf v0.3.2, nur der Tester ist anders) und neue Naht **N24** (`LoadEffectChainEvent` bekommt `confirmed`, Entscheid Patrik P1: a — für die Aktion `preset_laden`) |
| 0.76.0 | 2026-10-09 | Session 105 — §12.2: **Naht N20 wächst** (eingetragen vor dem Bau) um einen Aufruf `meldeAppAktionen` für die Aktionen außerhalb des Composers; §12.4 nachgeführt |
| 0.75.0 | 2026-10-09 | Session 105 — §12.2 und §12.4: **Naht N19 wächst** (eingetragen vor dem Bau): `sichttest.pin` auf v0.3.1 (Köpfe mit `fassung()`) |
| 0.74.0 | 2026-10-09 | Session 105 — §12.2 neue Naht **N23** (die eigene Kopie des Testers `projects/exec/Sichttest` und ihr Eintrag in `Solution.json` entfallen, Entscheid Patrik LV2: a, eingetragen vor dem Entfernen), §12.4 Rückbau-Liste |
| 0.73.0 | 2026-10-09 | Session 105 — §12.2 neue Naht **N22** (`/bigobj` für die Unit-Tests unter MSVC, Entscheid Patrik C1: a, eingetragen vor dem Bau), §12.4 Rückbau-Liste |
| 0.72.0 | 2026-10-09 | Session 105 — §12.2 zwei neue Nähte, **eingetragen vor dem Bau**: **N20** Start mit `--testing` (Laden der Sichttest-Steuerung nach dem Startzustand, Meldung bei laufender Instanz) und **N21** Aktionen des Composers (ein Aufruf im Hauptfenster; N9 wächst um den Haken „Sichttest: a loaded template starts playing", Entscheid Patrik L1: a mit Einstellung, Vorgabe aus) |
| 0.71.0 | 2026-10-09 | Session 105 — §12.2 und §12.4: **Naht N19 wächst** (eingetragen vor dem Bau): `sichttest.pin` auf v0.3.0, `cmakecraft.pin` auf v0.11.0 |
| 0.70.1 | 2026-10-09 | Session 104 — §12.4: Kennung des Naht-Commits N19 nachgetragen (`5d49259`) |
| 0.70.0 | 2026-10-09 | Session 104 — §12.2 neue Naht **N19 Sichttest-Steuerung, Bezug** (Entscheid Patrik Z1: a, eingetragen vor dem Bau): External `sichttest` in `Solution.json` an Kern und Runner, `sichttest.pin`; §12.4 Rückbau-Liste: eine Zeile dazu; `cmakecraft.pin` auf v0.10.0. Stand: Paket `v0.2.0` vom Release bezogen, in drei Presets gebaut, kein Quelltext der App berührt |
| 0.69.0 | 2026-10-08 | Session 103 — §12.4 Rückbau-Liste: neue Zeile „Vorlagen der Sichttests" (`test_ComposerVorlagen.cpp`, der Ordner `asset/sichttest/`; keine Naht). Kein Code der App geändert. Anlass: Patrik hatte die Sichttests aufgegeben (unklar beschrieben, kein festes Material); die offenen Composer-Punkte aus S94b bis S102 sind in neun Listen nach Thema neu gefasst (lokal), jede mit fester Vorlage. Die Steuerung der App durch den Tester wird außerhalb abgestimmt (Sync `sync_sichttest`); eine Einbindung wäre eine neue Naht und ist hier noch nicht eingetragen |
| 0.68.1 | 2026-10-07 | Session 102, Abschluss — §12.4: acht eigene Fälle „S102 …" und einer für die Naht (statt „sieben"). Stand: Zoom und Rollen der Zeitachse sind in sechs Schritten gebaut, im Programm nichts gesehen (Sichttest S102, 64 Punkte); offen das Urteil zur Schwelle der Kurvenpunkte (Abweichung vom Wortlaut von ZO3); aus §7 fehlen weiter Lock und Follow player |
| 0.68.0 | 2026-10-07 | Session 102 — **Zoom, Schritt 6 — Abschluss des Vorhabens:** §12.4 Rückbau-Liste: eine Zeile für Zoom und Rollen mit den Kennungen (`ed32789`, `4a5882c`, `02140d9`, `97e67ef`, `76ab153`; die Naht N9 für sich in `640cac5`). Benutzerhandbuch 1.111.0 (§15 Zoomen und Rollen), Sichttest-Liste `Sichttests_S102.md`. Im Programm nichts gesehen |
| 0.67.0 | 2026-10-07 | Session 102 — **Zoom, Schritt 5 — die Naht N9 wächst (Entscheid Patrik ZO6):** `UI/panels/SettingsPanel.cpp`, im Block des Composers nach *New clip length*: die Auswahl **Scroll with playhead** (Smooth / By page) über `Sitzung::mitlaufenSeitenweise`; nur die Naht, das Eigene steht in 0.66.0. Test: neuer Fall „S102 Naht N9" am echten Settings-Panel. Offen: Schritt 6 (Handbuch, Sichttest-Liste, Rückbau-Liste §12.4) |
| 0.66.0 | 2026-10-07 | Session 102 — **Zoom, Schritt 5, das Eigene — Mitlaufen mit der Abspielmarke (Entscheid Patrik ZO6):** §7 — `Zeitachse::setMitlaufen` (stetig mit der Marke in der Mitte, oder seitenweise), nur bei laufender Komposition, gehalten bei gedrückter Maustaste und gezogenem Rollbalken; der Schalter **Scroll with playhead** im Panel, über den Neustart gemerkt; `Sitzung::mitlaufenAn` / `mitlaufenSeitenweise`. Ohne die Auswahl in Settings (Naht N9, eigener Commit) gilt die Vorgabe stetig. *Scroll with playhead* und *Fit* stehen rechts in der Datei-Zeile (die erste Zeile wäre sonst breiter geworden). Gemessen: rund 11 ms je Bild ohne, rund 10 ms mit stetigem Mitlaufen — die Achse zeichnet ohnehin je Takt. Test: zwei neue Fälle (die Achse allein; am echten Panel mit laufender Komposition) |
| 0.65.0 | 2026-10-07 | Session 102 — **Zoom, Schritt 4 — Schwenken und Zurücksetzen (Entscheide Patrik ZO7: a, ZO8: a; Vorgabe S100: Ctrl+0):** §7 — die mittlere Maustaste zieht den Ausschnitt in beide Richtungen; `setzeZoomZurueck`; der Knopf **Fit** im Kopf mit den Gesten im Tooltip; Ctrl+0 als Kürzel des Panels für sich und seine Kinder; im Rechtsklick-Menü der Zeitachse das Untermenü *Zoom*. Eigenes, keine Naht berührt. Von mir gesetzt: das Rechtsklick-Menü gibt es auch ohne *Edit*, dann nur mit *Zoom*; der Platz von *Fit* hinter *Mute*. Test: neuer Fall „S102 Zeitachse: Schwenken", Fit und das Kürzel im Panel-Fall. Nicht belegt: der Weg der Taste Ctrl+0 durch das Hauptfenster, das Menü selbst. Offen: Schritte 5 und 6 |
| 0.64.0 | 2026-10-07 | Session 102 — **Zoom, Schritt 3 — der Höhen-Zoom (Entscheide Patrik ZO1: a, ZO2: a, ZO3: a mit Zusatz, ZO5: a, ZO9: a):** §7 — Alt+Rad ändert die Zeilenhöhe um 10 % je Raste um den Mauszeiger, Ctrl+Alt+Rad Zeit und Höhe zusammen; 50 % bis 300 %, die Master-Zeilen immer 60 % eines Kanals; Namen, die nicht passen, bleiben weg und stehen im Tooltip; bei Kanälen unter 24 Pixeln (50 % und 60 %) keine Punkte der Kurve; die Höhe merkt das Panel (`composer/rowHeightPercent`). Eigenes, keine Naht berührt. Abweichung vom Wortlaut von ZO3, von mir gesetzt: die Schwelle gilt für die Kanalhöhe und dann für alle Zeilen — sonst wären die Griffe der Master-Zeilen (seit S100) schon in Normalhöhe weg. Nach Alt+Rad bleibt die Menüleiste in Ruhe (Test mit Gegenprobe). Test: drei neue Fälle (Höhen-Zoom, Menüleiste, Höhe über den Neustart), Bilder bei 50 % und 300 % angesehen. Offen: Schritte 4 bis 6 |
| 0.63.0 | 2026-10-07 | Session 102 — **Zoom, Schritt 2 — der Zeit-Zoom (Entscheide Patrik ZO1: a, ZO5: a, ZO9: a):** §7 — Ctrl+Rad zoomt die Zeit um den Mauszeiger (Faktor 1,25 je Raste, von der ganzen Länge bis 1 Millisekunde je Pixel), Shift+Rad rollt waagrecht, waagrechter Rollbalken im `ZeitachsenRahmen`, die Spalte der Kanalnamen bleibt stehen, das Lineal teilt sich bis auf Zehntelsekunden, gezeichnet wird nur das sichtbare Stück; eine geladene Komposition beginnt ganz. Eigenes, keine Naht berührt. Von mir gesetzt: der Name eines Clips bleibt am linken Rand stehen; über der Spalte der Kanalnamen gilt die erste sichtbare Stelle. Test: neuer Fall „S102 Zeitachse: Zeit-Zoom" (72 Prüfungen), ein Bild der gezoomten Achse angesehen (`LUMIVIZ_COMPOSER_ZOOM_BILD`). Offen: Schritte 3 bis 6 |
| 0.62.0 | 2026-10-07 | Session 102 — **Zoom und Rollen der Zeitachse, neun Entscheide Patrik (ZO1 bis ZO9), gebaut ist Schritt 1:** §7 neuer Block (Ctrl+Rad Zeit, Alt+Rad Zeilenhöhe, Ctrl+Alt+Rad beides, Ctrl+0; Master-Zeilen immer 60 %; Höhe 50 % bis 300 %; das Lineal bleibt oben stehen, die Kanalnamen links, Rollbalken in beiden Richtungen; Festpunkt Mauszeiger; Schalter **Scroll with playhead**, die Art in Settings, Vorgabe stetig; Schwenken mit der mittleren Maustaste; Knopf **Fit**; die Höhe merkt die App, die Zeit beginnt ganz). §12.2 **Naht N9 wächst** (Auswahl Scroll with playhead in Settings), vor dem Bau eingetragen. **Gebaut: Schritt 1** — die Zeitachse rollt senkrecht selbst (`rollY`, `zeileBei`, Zeichnen in zwei Gängen, `ZeitachsenRahmen` mit dem Rollbalken statt der `QScrollArea`); eigener Code, keine Naht berührt; Test: neuer Fall „S102 Zeitachse: sie rollt selbst" (84 Prüfungen). Bauplan in sechs Schritten: (1) die Zeitachse führt ihr Rollen selbst, (2) Zeit-Zoom, (3) Höhen-Zoom, (4) Schwenken, Fit, Ctrl+0, (5) Mitlaufen mit der Naht, (6) Doku und Sichttest-Liste |
| 0.61.1 | 2026-10-07 | Session 101, Abschluss — §12.4 Rückbau-Liste: eine Zeile für Master-Zeilen, Karaoke im Composer und Mute mit den Kennungen (`49d3cf6`, `39f8443`, `219087b`, `e1e6664` mit R2, `725c5cc` mit N17 und N13, `735062b` mit N18) — sie ersetzt die Vermerke „Kennung folgt" der Zeilen 0.59.0 bis 0.61.0 unten. Als Nächstes (Patrik): Zoom — nicht nur in der Zeitachse, Umfang wird in der nächsten Sitzung geklärt |
| 0.61.0 | 2026-10-07 | Session 101 — **Mute, Schritt 2c — der Knopf (Entscheid Patrik AM2: c):** §12.2 **Naht N18** (`UI/panels/PlayerPanel.cpp`: ein Aufruf `composer::erzeugeMuteKnopf`, das Panel legt den Knopf hinter den Lautstärke-Regler), vor dem Bau eingetragen. Eigenes: `composer/MuteKnopf.hpp` / `.cpp` — `MuteKnopf` (schaltet `MuteSchalter`, Rechtsklick wählt das Profil) und `MuteProfilDialog`; derselbe Knopf im Panel Composer. Der Lautsprecher-Knopf des Panels Player bleibt das harte Mute. §12.4 Rückbau: die zwei Dateien, die Naht-Stelle im Panel Player (trägt `Composer-Naht N18`); Kennung folgt — der Commit trägt Naht und Eigenes zusammen. Im Programm nicht gesehen, nicht gehört |
| 0.60.0 | 2026-10-07 | Session 101 — **Mute, Schritt 2b (Entscheid Patrik AM3: c — „Visuals follow" im Profil, Vorgabe aus):** §12.2 **Naht N17** (Pegel am Player — die frühere N7a, wieder eingesetzt; `onAudioUpdate`: N13 wächst um den Faktor für die Visuals), vor dem Bau eingetragen. Eigenes: `MuteSchalter` (der eine Mute der App, setzt den Pegel des Players nach dem Profil), `MuteProfil::visualsFolgen`, `composer::muteFaktorFuerVisuals`, `Tonfuehrung::takt` nimmt Lautstärke mal Pegel für den Mixer. §12.4 Rückbau: die Naht-Stellen in `audio/IAudioPlayer.hpp`, `audio/AudioPlayer.hpp` / `.cpp` (wie N7a: Block „Pegel", `laut()` an drei Stellen) und zwei Stellen in `UI/MainWindow.cpp` (`onAudioUpdate`), alle tragen `Composer-Naht N17` bzw. `N13`; Kennung folgt — der Commit trägt Naht und Eigenes zusammen. Noch ohne Knopf: im Programm wirkt nichts |
| 0.59.0 | 2026-10-07 | Session 101 — **Karaoke im Composer (Entscheid Patrik KS1: a — Spur und Haken; löst „Karaoke-Schalter als Spur" aus der Liste „später" in §9 ein):** `Komposition::karaokeKurve` (`MasterArt::Karaoke`, Datei `karaokeMaster`), vierte Master-Zeile KARAOKE in der Zeitachse (`kMasterKaraoke`), der Haken *Karaoke* im Panel Composer (ruft `KaraokePanel::setzeKaraokeAn`, folgt `KaraokeEnabledChangedEvent`). §12.2: **R2 wächst** um `legeKaraokeEbeneAuf(pegel)` — vor dem Bau eingetragen; die Wiedergabe reicht den Pegel der Kurve an der laufenden Stelle herein. Die Kurve steht für sich: MASTER und VISUAL blenden die Ebene nicht mit (von mir gesetzt, wie seit S99). §12.4 Rückbau: die Naht-Stellen in `MultiEffectVisualizer.hpp` / `.cpp` (ein Parameter, eine Zeile im Auflegen), Kennung folgt — der Commit trägt Naht und Eigenes zusammen. Im Programm nicht gesehen |
| 0.58.0 | 2026-10-07 | Session 101 — **Mute für spontane Durchsagen, Schritt 2a (Vorgabe Patrik: ein Knopf, Mute-Profile mit Ausblend-Kurve, Einblend-Kurve und Zielpegel, zu speichern und zu laden; Entscheid AM2: c — der Mute gehört dem Player, der Knopf sitzt in beiden Panels):** neu `composer/Mute.hpp` / `.cpp` — `MuteProfil`, `MuteVerlauf` (Faktor über der Zeit, ohne Sprung beim Umschalten mitten in einer Blende), `muteprofile` (Dateien `.lvmute`, mitgeliefert unter `asset/composer/mutes`, eigene im Benutzerordner; das gewählte unter `composer/muteProfile`); `test_ComposerMute.cpp`. Nur das Modell — **kein Knopf, kein Ton, keine Naht**; die Nähte am Player (ein Faktor neben der Lautstärke, der Knopf im Panel Player) kommen mit Schritt 2b und werden vorher eingetragen. Rückbau: die zwei Dateien, der Test, der Ordner `asset/composer/mutes`; zurück bleibt der Schlüssel `composer/muteProfile` |
| 0.57.0 | 2026-10-07 | Session 101 — **Visual-Master und Audio-Master, Schritt 1 (Entscheid Patrik AM1: a — zwei volle Kurven; dazu als Schritt 2 ein Mute-Knopf mit Mute-Profilen, noch nicht gebaut):** `Komposition` bekommt `bildKurve` und `tonKurve` (`MasterKurve`: Blenden mit Form, Grundpegel, Stücke an fester Zeit), `kurveAlsClip` / `setzeKurveAusClip` / `hatKurve` nehmen die Art (`MasterArt`), `bildPegelBei` und `tonPegelBei` nehmen MASTER mit der jeweiligen Kurve mal — die Wiedergabe rechnet das Bild, die Tonführung den Ton damit; Datei: Objekte `visualMaster` und `audioMaster`, nur wenn die Kurve wirkt (ältere Dateien lesen sich ohne sie, ein älterer Stand überliest sie); Zeitachse: drei Master-Zeilen (MASTER, VISUAL, AUDIO) mit eigenen „Kanälen" `kMaster`, `kMasterBild`, `kMasterTon` — Griffe, Rechtsklick und Curve Editor gehen denselben Weg. Nur eigener Code, keine Naht. Im Programm nicht gesehen |
| 0.56.0 | 2026-10-07 | Session 101 — Sichttest S100, erster Teil (Patrik): die Zeile MASTER in der Grundfunktion bestanden (M0 bis M9); **Urteil M17: Master-Zeilen auf 60 % der Kanalhöhe, auch ein künftiger Audio- und Visual-Master** — umgesetzt (`Zeitachse::kMasterHoehe`, `zeileOben` / `zeileHoch`; eigener Code, keine Naht); Rückbau-Liste: Kennung `313423c` der zweiten Stelle von N8. Im Programm nicht gesehen |
| 0.55.0 | 2026-10-07 | Session 101 — `LumiViz.exe <datei>`, Schritt 2 (Entscheid Patrik X1b: a — eine Instanz je Exe-Datei): §12.2 **N8, zweite Stelle** (`MainWindow::oeffneUebergebeneDatei`: eine `.lvcomp`, die ein weiterer Start abgibt; vor dem Bau eingetragen); §12.4 Rückbau-Liste, Kennung `12c870b` von Schritt 1. Im Programm nicht gesehen |
| 0.54.0 | 2026-10-07 | Session 101 — `LumiViz.exe <datei>` (Entscheid Patrik X1: a), Schritt 1: §12.2 **N8 wächst** um die `.lvcomp` als Argument beim Start (eine Stelle im Hauptfenster, vor dem Bau eingetragen); §12.4 Rückbau-Liste. Im Programm nicht gesehen |
| 0.53.1 | 2026-10-07 | Session 101 — Rückbau-Liste: Kennung `7e62922` des Karaoke-Fixes (Nähte N16, R2 und Eigenes in einem Commit) |
| 0.53.0 | 2026-10-07 | Session 101 — **Befund Patrik: das Panel Karaoke lud bei laufendem Composer nichts** („Could not load", ohne Grund). Entscheid KP1: a — das Panel bleibt bedienbar, der Composer zieht nach. §12.2: **Naht N16** (die Karaoke-Editoren übergehen die Bindungs-Meldungen des Composers) und **R2 wächst** (`karaokeFassung`), vor dem Bau eingetragen; eigener Code: die Sitzung reicht die Ebene der App je Takt an den Karaoke-Träger der Wiedergabe. Vor dem Bau per Test gezählt (Fall S101 in `test_ComposerSitzung.cpp`), danach grün. §12.4: Rückbau-Liste. **Im Programm nicht gesehen** |
| 0.52.1 | 2026-10-07 | Session 100, Abschluss — Rückbau-Liste: Kennung `ef57574` des Dateityp-Icons. Neuer Wunsch Patrik (nicht begonnen, `Offene_Punkte.md` §7): neben der Zeile MASTER ein Audio-Master und ein Visual-Master |
| 0.52.0 | 2026-10-07 | Session 100 — **Dateityp-Icon der Komposition; Naht N8 wächst** (Wunsch Patrik S99; Entscheide S100: I1 a — Effect Browser und Panel Composer, I2 — der Fächer, I2b a — mit Zeitstrahl und Abspielmarke; §12.2, vor dem Bau eingetragen). Naht: `PresetTypeIcons.hpp` (`PresetIconKind::Komposition`, Endung `lvcomp`), `ImportBrowserPanel` (`*.lvcomp` gelistet, `Type_Komposition`, Doppelklick meldet `OpenCompositionEvent` mit dem Pfad; kein Favorit, nicht ziehbar, nicht im Blättern). Eigenes: das Icon vor dem Dateinamen im Panel Composer. Icon-Datei `asset/img/logo/icons/lumiviz_lvcomp.ico` (erzeugt in `build_lvfx2_icons.py`). Neuer Fall in `test_PaletteUi.cpp`. 943 Cases. **Nicht belegt:** der Weg vom Doppelklick bis zur geladenen Komposition im Programm (der Test prüft das Ereignis; den Empfänger in `MainWindow` gab es schon). Kein Composer-Teil, nebenbei: Icon-Ressource der Exe (`main/LumiViz.rc`). Rückbau-Liste: Zeile S100 Dateityp-Icon, Kennung folgt |
| 0.51.0 | 2026-10-07 | Session 100 — **Kanäle nach Art ordnen** (Wunsch Patrik S99, Entscheid S100 S1: a — nur nach Art, innerhalb der Art bleibt die Reihenfolge; der Stapel der Bild-Kanäle und damit das Bild ändern sich nicht). `Bearbeiten::sortiereNachArt` / `nachArtSortiert`, `Zeitachse::kanaeleSortieren` — Auswahl und Einfügemarke folgen ihrem Kanal; Rechtsklick → *Sort channels* → *Visuals first* / *Audio first*, grau, was schon so liegt; über Undo („Sort channels"). **Von mir gesetzt:** beide Richtungen als eigene Einträge. Eigener Code, keine Naht. Erweiterungen in `test_ComposerWiedergabe.cpp` und `test_ComposerPanel.cpp`. Rückbau-Liste: Kennung `1a30181` (Kanäle verschieben) |
| 0.50.0 | 2026-10-07 | Session 100 — **Kanäle verschieben** (Wunsch Patrik S99, Entscheid S100 V1: a — eine Reihenfolge, Verschieben ändert den Stapel). Die Reihenfolge der Kanäle bleibt die Mischreihenfolge der Bild-Kanäle (oben in der Liste = im Bild zuunterst); Audio-Kanäle liegen frei dazwischen, für den Ton zählt die Reihenfolge nur bei Gleichstand der Pegel (der obere Kanal stellt den laufenden Song). `Bearbeiten::verschiebeKanal` / `platzNachVerschieben`; `Zeitachse::kanalVerschieben` — Auswahl und Einfügemarke folgen ihrem Kanal; Ziehen am Kanalnamen in der linken Spalte (Strich an der Zielkante, Esc bricht ab) und Rechtsklick → *Move channel up* / *Move channel down*; über Undo („Move channel"). Renderer und Tonquellen hängen an Datei und Startzeit des Clips, nicht am Platz des Kanals — **nicht belegt**, dass ein Verschieben im Lauf sie nicht neu startet (aus `Wiedergabe::ersetzeKomposition` gelesen). **Von mir gesetzt:** die zwei Wege, der Kanal fällt auf den Platz der Zeile unter der Maus. Eigener Code, keine Naht. Neuer Fall in `test_ComposerWiedergabe.cpp`, Erweiterung in `test_ComposerPanel.cpp`. 942 Cases. Rückbau-Liste: Kennung `5e56a4a` (Ende an der Zeile MASTER ziehen) |
| 0.49.0 | 2026-10-07 | Session 100 — **Das feste Ende an der Zeile MASTER ziehen** (Wunsch Patrik S100): steht das Ende fest (*Fixed end*), ist die rechte Kante des Balkens der Zeile MASTER ein Griff (`Griff::KanteRechts` am „Kanal" `kMaster`) — ziehen setzt `Komposition::laengeMs`, eingerastet wie ein Clip, über Undo („Change the end"); ohne *Fixed end* gibt es den Griff nicht. Eigener Code, keine Naht. Erweiterung in `test_ComposerPanel.cpp`. Rückbau-Liste: Kennung `9c9c5c5` der Kanalfarben |
| 0.48.0 | 2026-10-07 | Session 100 — **Kanalfarben** (Wunsch Patrik S99, Entscheid S100 K1: b — zwei Farbreihen, die Farbe gehört dem Kanal). Bisher hing die Farbe eines Bild-Kanals an seinem Platz in der Liste, alle Audio-Kanäle trugen denselben Ton. Neu: `Kanal::farbe` (0xRRGGBB, -1 = keine), `reihenFarbe` (je sechs Farben: Bild kühl, Ton warm; die erste jeder Reihe ist die bisherige von CH1 bzw. der Ton-Clips), `Komposition::freieFarbe` / `vergibFarben` / `farbeVon`; `fuegeKanalEin` gibt die nächste freie Farbe der Art; `Sitzung::lade` und `setzeKomposition` vergeben fehlende Farben — eine geladene Datei ohne Farben gilt damit nicht als geändert; in der Datei `channels[].color` als `#rrggbb`, nur wenn der Kanal eine Farbe trägt. Zeitachse: Rechtsklick → *Channel color...* (Farbwahl von Qt) / *Reset channel color*, `Zeitachse::kanalFaerben`, über Undo; der Schatten beim Hineinziehen nimmt die Farbe des Zielkanals. **Von mir gesetzt:** die zwölf Farben der Reihen; Zurücksetzen gibt die erste freie Farbe der Art, nicht die frühere. Eigener Code, keine Naht. Neuer Fall in `test_ComposerWiedergabe.cpp`, Erweiterung in `test_ComposerPanel.cpp`. 941 Cases. Im Programm nicht gesehen. Rückbau-Liste: Zeile S100 Kanalfarben, Kennung folgt |
| 0.47.0 | 2026-10-07 | Session 100 — **Spätes Einblenden am Clip** (Befund Patrik S99, Entscheid S100: a): mitgeliefertes Stück-Preset **Late fade in** (`asset/composer/shapes`), das Rezept per Test belegt — ein Stück am Clip-Anfang, dessen Kurve unten beginnt; kein neuer Wert am Clip, keine Naht (§9, Presets). Rückbau-Liste: Kennung `b9687bf` der Zeile MASTER; das Preset ist eine Datei unter `asset/composer` und geht mit dem Ordner |
| 0.46.0 | 2026-10-07 | Session 100 — **Pegelkurve der ganzen Komposition, Schritt 2: die Zeile MASTER** (§4) — unter dem Lineal, anklickbar wie ein Clip (`Zeitachse::kMaster`, `kKanaeleOben`, `clipIn`, `schreibeClip`, `waehleGesamt`, `gesamtGewaehlt`); Griffe, Rechtsklick, Doppelklick und Curve Editor gehen die Wege des Clips; der Curve Editor schreibt über `setzeKurveAusClip` in die Komposition; die Keile im Lineal entfallen. Die geplanten Schritte 2 bis 4 in einem (Curve Editor und Griffe hängen am angeklickten Clip). Eigener Code, keine Naht. Erweiterung des Falls in `test_ComposerPanel.cpp`, Bild `.master.png` angesehen. Benutzerhandbuch 1.99.0. Rückbau-Liste: Kennung `4150554` von Schritt 1, Schritt 2 folgt |
| 0.45.0 | 2026-10-07 | Session 100 — **Pegelkurve der ganzen Komposition** (Urteil Patrik F7: „gleich wie bei Clips, auch editierbar"; Entscheide F7 c — Blenden mit Kurve, Grundpegel und Stücke in einer eigenen Spur —, F7-1 a — ein Stück liegt an fester Zeit). **Gebaut, Schritt 1 von 4 (Modell und Datei, eigener Code, keine Naht):** `Komposition::einblendeForm` / `ausblendeForm` / `grundpegel` / `gesamtStuecke`, `GesamtStueck`, `kurveAlsClip`, `setzeKurveAusClip`, `hatKurve`; `blendeBei` rechnet wie die Pegelkurve eines Clips (geändert: sich überdeckende Blenden teilen den Platz, bisher galt der kleinere Wert); Datei `fade { inShape, outShape }` und `level { base, shapes }` (§4). Neuer Fall in `test_ComposerWiedergabe.cpp`. Im Panel noch nicht einstellbar. Rückbau-Liste: Kennung `da5339f` der Naht N5, sechste Stelle; Zeile „Pegelkurve der ganzen Komposition", Kennung folgt |
| 0.44.0 | 2026-10-07 | Session 100 — **Sichttest S99, Befund A2** (Patrik: beim ersten Ausschalten wechselte das Panel Player nicht auf „No track loaded", beim zweiten schon). Per Test gezählt: der leere Songwechsel kommt, aber ein Player-Panel, das dabei nicht zu sehen ist, hat kein Abo am Bus und zeigt nach dem Einblenden weiter „Composer - <Song>" mit der alten Stelle. **Naht N5, Anzeige, wächst um eine sechste Stelle** (vor dem Bau eingetragen, §12.2): `PlayerPanel::onActivate` geht ohne Titel im Player und ohne führenden Composer in den Ruhezustand. Erweiterung des Falls in `test_ComposerNahtTon.cpp`. **Nicht belegt**, dass das Panel bei Patrik in jenem Augenblick verdeckt war — Indiz: die Stelle im Panel lag rund 3,6 s hinter der der Komposition. **Sichttest S99 sonst:** Abschnitt E (Überlappung) und F (Blende) im Programm gesehen und gehört, ohne Befund — damit gebilligt, was in §4 als „von mir gesetzt" steht: Reihenfolge im selben Kanal (E6), Sprung ins hintere / vordere Fenster (E7, E8), Halt im Fenster (E9), Loop aus im Fenster (E10), Vorwärmen und „kein Sprung" bei jeder Schleife (E14), die Karaoke-Ebene blendet nicht mit (F6). **Nicht gebilligt:** die lineare Form der Blende — Urteil Patrik F7: wie bei Clips, auch editierbar (Entscheid steht aus). Nicht gespielt: Abschnitt G (*Visuals overlap themselves*); dort bleibt „von mir gesetzt" stehen. Rückbau-Liste: Zeile S100, Kennung folgt |
| 0.43.1 | 2026-10-06 | Session 99, Abschluss — Rückbau-Liste: Zeile „Überlappung in der Schleife und Blende der ganzen Komposition" mit den Kennungen der acht Commits (keine Naht; zwischen `14edf71` und `a2c49f7` baut der Stand mit clang nicht). Kopf: Stand der Stufen. **Stand:** die Überlappung, die Blende und die geschlossene Lücke im Panel Player sind gebaut und per Test belegt, im Programm nichts davon gesehen oder gehört — Sichttest S99 (52 Punkte) |
| 0.43.0 | 2026-10-06 | Session 99 — **Überlappung in der Schleife, Schritt 7 von 7: Visuals overlap themselves — alle Schritte gebaut** (eigener Code, keine Naht; §4). `Wiedergabe`: zweite Exemplare (`m_frische`, `zweitRendererVon`), vorgewärmt vor dem Umschlag, im Fenster beide gemischt (das alte unten, das frische darüber, jedes mit seinem Pegel), dahinter läuft das Exemplar weiter, dessen letzte Stelle im Clip am nächsten liegt (`Lauf::imClipMs`); `rendererAnzahl` und `fuerJedenRenderer` zählen sie mit. Von mir gesetzt: der festgehaltene Clip läuft immer einmal, beim Bearbeiten gehen zweite Exemplare verloren. Panel: Haken **Visuals overlap themselves** (Vorgabe aus), die Blenden-Felder in einer eigenen Zeile (sonst wäre das Panel zu breit). Tests: Erweiterung des GL-Falls in `test_ComposerWiedergabe.cpp` (Vorwärmen, Mischung in Zahlen, welches Exemplar weiterläuft, Sprung ins Fenster, festgehaltener Clip), `test_ComposerPanel.cpp` (Haken, Undo). Benutzerhandbuch 1.98.0. Gebaut mit MSVC und clang. Nicht gemessen: Bildzeit des zweiten Ladens, Grafikspeicher. Im Programm nicht gesehen — Sichttest S99, Abschnitt G |
| 0.42.0 | 2026-10-06 | Session 99 — **Überlappung in der Schleife, Schritt 6 von 7: die Blende der ganzen Komposition** (eigener Code, keine Naht; §4). Bild und Ton nehmen den Pegel jedes Clips mit `Komposition::blendeBei` an seiner Stelle mal (im Fenster je Stelle: Überblendung); gilt immer, auch ohne Loop. Panel: Felder **Fade in** / **Fade out** (nur bei Edit, über Undo); Zeitachse: je ein Keil unten im Lineal. Von mir gesetzt: linear, die Karaoke-Ebene blendet nicht mit, der laufende Song folgt dem Produkt aus Pegelkurve und Blende. Tests: Erweiterungen in `test_ComposerWiedergabe.cpp` (Mischung in Zahlen, auch im Fenster), `test_ComposerTonfuehrung.cpp` (Lautstärken, Songwechsel in der Mitte des Fensters), `test_ComposerPanel.cpp` (Felder, Undo); Bild `.schleife.png` angesehen. Benutzerhandbuch 1.97.0. Gebaut mit MSVC und clang. Im Programm nicht gesehen und nicht gehört — Sichttest S99, Abschnitt F. **Offen bleibt Schritt 7** (Visuals overlap themselves) |
| 0.41.0 | 2026-10-06 | Session 99 — **Überlappung in der Schleife, Schritt 5 von 7: Panel und Zeitachse — die Überlappung wirkt im Programm** (eigener Code, keine Naht; §4). Panel: Zeile mit **Loop overlap** und **Songs overlap themselves** (nur bei Edit, über Undo, der eingestellte Wert). Zeitachse: bei Loop an die zwei Fenster getönt, gestrichelte Grenzen über den Clips, zweite blassere Marke (`Zeitachse::setSchleife`, `Sitzung::zweiteStelleMs`). Noch nicht im Panel: *Fade in* / *Fade out* mit Keilen (Schritt 6), *Visuals overlap themselves* (Schritt 7). Test: Abschnitt in `test_ComposerPanel.cpp` (Feld, Haken, Undo, Sperre ohne Edit, Zeitachse kennt Loop), Bild `.schleife.png` über `LUMIVIZ_COMPOSER_PANEL_BILD` angesehen. Benutzerhandbuch 1.96.0 (§15). Gebaut mit MSVC und clang. **Im Programm nicht gesehen und nicht gehört** — Sichttest S99, Abschnitt E |
| 0.40.0 | 2026-10-06 | Session 99 — **Überlappung in der Schleife, Schritt 4 von 7: der Ton** (eigener Code, keine Naht; §4). `Tonfuehrung::takt` nimmt die `Schleifenlage` (jetzt in `Komposition.hpp`, für Bild und Ton) und führt Stimmen: Ton-Clips an der vorderen Stelle, im Fenster auch an der hinteren, vor dem Umschlag die ersten Songs der nächsten Runde im Vorlauf. Ein Song in beiden Fenstern bekommt eine zweite Quelle (`songsDoppelt`), sonst spielt er sein Ende aus und springt danach. Quellen werden den Stimmen über die Stelle im Song zugeordnet, ein Sprung wird je Quelle an der Stelle im Song erkannt — **der Umschlag der Schleife ist kein Sprung mehr** (Verhalten geändert, auch ohne Überlappung: ein Song über die ganze Länge beginnt in der Schleife nahtlos mit einer zweiten Quelle statt zurückgesetzt zu werden). `Bildquelle::standStellenJetzt` nennt die Zeit bis zum Umschlag; `Sitzung::fuehreTon` reicht beides herein. Dazu der clang-Fix von Schritt 3 (`Schleifenlage` aus der Klasse gezogen, `fix`-Commit). Neuer Fall in `test_ComposerTonfuehrung.cpp` (zweite Quelle, kein Setzen am Umschlag, Pegel beider Stimmen, ohne den Haken, zwei verschiedene Songs), `PappMischEngine::quellenVon`. Gebaut mit MSVC und clang, App und Tests. **Im Programm:** die Überlappung wirkt weiter nicht (niemand setzt sie); im Loop ohne Überlappung ist der nahtlose Schleifenanfang eines Songs zu hören — nicht gehört |
| 0.39.0 | 2026-10-06 | Session 99 — **Überlappung in der Schleife, Schritt 3 von 7: das Bild** (eigener Code, keine Naht; §4). `Wiedergabe::rendere` nimmt eine `Schleifenlage` (zweite Stelle, Zeit bis zum Umschlag): im Fenster laufen die Clips der vorderen und der hinteren Stelle; im selben Kanal liegt der Clip des Endes unten, der des Anfangs darüber (von mir gesetzt); ein Clip in beiden Fenstern läuft einmal mit dem höheren der zwei Pegel. **Vorwärmen über den Umschlag** — auch in einer Schleife ohne Überlappung: die ersten Clips der nächsten Runde werden im Vorlauf angelegt (Verhalten geändert: bisher begannen sie kalt). `Bildquelle::render` reicht beides aus der Uhr herein. Neuer GL-Fall in `test_ComposerWiedergabe.cpp` (Mischung im Fenster in Zahlen, Vorwärmen, Clip in beiden Fenstern). Kein Vergleichslauf: die Bildberechnung des Renderers ist nicht berührt, der bitgleiche Fall der Wiedergabe läuft unverändert. **Im Programm:** die Überlappung wirkt weiter nicht (niemand setzt sie); das Vorwärmen am Schleifenanfang wirkt ab jetzt bei jedem Loop |
| 0.38.0 | 2026-10-06 | Session 99 — **Überlappung in der Schleife, Schritt 2 von 7: Uhr und Bildquelle** (eigener Code, keine Naht; §4). `Uhr::setUeberlappungMs`, `ueberlappungMs` (wirksam, höchstens die halbe Länge), `umlaufMs`, `zweiteStelleMs` (die Stelle im hinteren Fenster, −1 = keine), `rueckkehrMs`, `umgelaufen`; `stelleMs` ist in der Schleife die vordere Stelle. Start, Stopp, Länge, Schleife an / aus und Überlappung halten im Fenster die hintere Stelle fest. `Bildquelle` gibt der Uhr die Überlappung der Komposition, spiegelt die zweite Stelle (`standZweiteStelleMs`, `standUeberlappungMs`, `standLaengeMs`) und rechnet beide auf jetzt hoch (`standStellenJetzt`); `ersetzeKomposition`, Composer aus und der Stand für den Neustart merken `rueckkehrMs`. Mit Überlappung 0 unverändert (die bisherigen Uhr-Fälle laufen ohne Änderung). Neue Fälle in `test_ComposerWiedergabe.cpp` (Uhr) und `test_ComposerBildquelle.cpp` (Spiegel). **Im Programm wirkt weiter nichts** — niemand setzt die Überlappung, Bild und Ton lesen die zweite Stelle noch nicht. Von mir gesetzt: Verhalten bei Sprung, Halt und Schleife aus im Fenster |
| 0.37.0 | 2026-10-06 | Session 99 — **Überlappung in der Schleife: vier Entscheide Patrik, Schritt 1 von 7 gebaut** (§4). (1) a — zwei getrennte Einstellungen: *Loop overlap* (nur bei Loop, nichts wird zusätzlich geblendet, die Schleife wird um die Überlappung kürzer) und *Fade in* / *Fade out* der ganzen Komposition (immer). (2) a — die Werte gehören zur Komposition (Kopf der `.lvcomp`, Fuß des Panels, über Undo). (3) a — die Zeitachse tönt beide Fenster, zweite blasse Marke, Keile für die Blende. (4) einstellbar mit zwei Haken: *Songs overlap themselves* (Vorgabe an), *Visuals overlap themselves* (Vorgabe aus, wird mit Freigabe als letzter Schritt gebaut). **Gebaut, Schritt 1 (Modell und Datei, eigener Code, keine Naht):** `Komposition::ueberlappungMs`, `songsDoppelt`, `bilderDoppelt`, `einblendeMs`, `ausblendeMs`; `ueberlappung()` (höchstens die halbe Länge), `umlaufMs()`, `endStelleZu()`, `blendeBei()` (linear — von mir gesetzt); in der Datei `loop { overlap, songsTwice, visualsTwice }` und `fade { in, out }`, nur wenn sie von der Vorgabe abweichen — eine Komposition ohne sie sieht aus wie bisher. Neuer Fall in `test_ComposerWiedergabe.cpp`. **Im Programm wirkt davon noch nichts** — Uhr, Bild, Ton und Panel folgen. Rückbau-Liste: Kennungen der Naht N5, fünfte Stelle (`3624e1f`, Eigenes `e3043ae`) |
| 0.36.0 | 2026-10-06 | Session 99 — **Freigeben ohne Titel im Player: die Lücke aus S98 geschlossen** (Entscheid Patrik: a). Vor dem Bau per Test gezählt: kein Songwechsel, Player-Panel und Tag View zeigten weiter den letzten Song der Komposition. **Eigenes:** `Sitzung::gibTonFrei` meldet einen leeren Songwechsel, wenn der Player keinen Titel hat und ein Song gemeldet war (§4). **Naht N5, Anzeige, wächst um eine fünfte Stelle** (vor dem Bau eingetragen, §12.2): der Empfänger von `TrackChangedEvent` im Player-Panel geht bei einem Songwechsel ohne Datei in den Ruhezustand. Neuer Fall in `test_ComposerNahtTon.cpp` (echtes Player-Panel, echtes Tag View). Nicht per Test belegt: der Empfänger in `MainWindow` (Songtext, Tags, Songwechsel-Zähler) bei leerem Titel. Dazu: Vorlauf 2 s und Abweichung 80 ms / Schonfrist 500 ms gelten als im Hörtest bestätigt (kein eigenes Urteil in der Liste). Rückbau-Liste: Zeile S99, Kennungen folgen |
| 0.35.0 | 2026-10-06 | Session 99 — **Sichttest S98b ausgewertet** (Patrik, 06.10. abends: 33 von 33 bestanden, keine Bemerkung). Im Programm gehört und gesehen: Korrekturen nach dem Hörtest (Tag View, kein Knackser, kein Rutschen), Weiterspielen beim Wechsel, Einfügemarke, Kopieren / Ausschneiden / Einfügen, Schatten, Tooltip, Vorgabelänge. **Log:** in allen 17 Läufen des Abends kein Nachsetzen, trotz langer Bilder bis 191 ms (§4). **Gebilligt, was ich gesetzt hatte:** beide Haken „Weiterspielen" Vorgabe an (§8); die Marke rückt hinter den eingefügten Clip, ohne Marke landet die Kopie hinter dem angeklickten Clip, eine Kopie wird nicht gekürzt, kein Hinweis bei falscher Art, Tooltip nur im Lineal (§7). **Wellenform beim Kürzen:** wird abgeschnitten (Patrik zu D5) — erledigt. Weiter ohne Urteil: Vorlauf 2 s, Abweichung 80 ms / Schonfrist 500 ms (Urteile R der ersten Liste, noch nicht geprüft), Kopieren ohne Edit. Kopf nachgezogen |
| 0.34.0 | 2026-10-06 | Session 98 — **Bedienung der Zeitachse (Wünsche Patrik nach dem Hörtest).** Entscheid Patrik: a — eine eigene **Einfügemarke** (Klick auf eine freie Stelle einer Kanalzeile); **Kopieren / Ausschneiden / Einfügen** von Clips mit Ctrl+C / X / V und im Rechtsklick-Menü, der Anfang an der Marke, nur in einen Kanal der passenden Art (`Bearbeiten::fuegeKopieEin`); **Schatten beim Hineinziehen** aus demselben Code wie das Fallenlassen (`Zeitachse::fallSchatten`), in einer Zeile der anderen Art weder Schatten noch Fallen; **Zeit unter dem Zeiger** im Lineal; **Vorgabelänge neuer Bild-Clips** in Settings (Naht N9 wächst, vor dem Bau eingetragen) — §7, §12.2. Belegt in `test_ComposerBearbeiten.cpp` (neuer Fall) und `test_ComposerPanel.cpp` (neuer Fall mit Marke, Ablage, Tasten, Undo, Schatten, Vorgabelänge). Rückbau-Liste: Kennung der Nähte nach dem Hörtest (`4b3e652`), Zeile „Bedienung der Zeitachse". 934 Cases. **Offen:** im Programm nichts davon gesehen; Patriks Beobachtung zur Wellenform beim Kürzen eines Songs ist nicht nachgestellt |
| 0.33.0 | 2026-10-06 | Session 98 — **Nach dem Hörtest** (Patrik: 26 bestanden, 6 mit Befund, 1 Fehler, 23 offen; der Mixer trägt). **O1 behoben, Naht N15:** Tag View zeigt beim Einblenden den laufenden Song der Komposition (§12.2). **Nachsetzen:** Ursache gemessen und behoben — die Engine liegt stetig 40 bis 50 ms hinter der Uhr (Latenz), die Ausreißer kamen von einer veralteten Stelle bei langen Bildern des Render-Threads; `Bildquelle::standStelleJetztMs`, Grundabstand je Quelle, jede Nachsetz-Zeile im Log mit Datei (§4). **Weiterspielen einstellbar** (Wunsch Patrik): Player nach Composer aus, Komposition nach Composer on — zwei Haken in Settings, Vorgabe an; Naht N9 wächst (§8, §12.2 — erst nach dem Bau eingetragen). **Naht N7a zurückgenommen** (Entscheid a, der Mixer trägt). Rückbau-Liste: Zeile „nach dem Hörtest". 932 Cases. **Offen:** im Programm nichts davon gesehen oder gehört; die Wünsche Patriks (Clips kopieren und einfügen, Tooltip am Zeitstrahl, Vorgabelänge, Wellenform abschneiden) |
| 0.32.2 | 2026-10-06 | Session 98 — **Entscheid Patrik zu Naht N7a: a** — sie wird zurückgenommen, sobald der Hörtest zeigt, dass der Mixer trägt (§12.2). Gebaut ist nichts |
| 0.32.1 | 2026-10-06 | Session 98 — **C5b Schritt 5, vorgezogen vor den Hörtest (Patrik):** Benutzerhandbuch 1.92.0, §15 Audio-Kanal neu gefasst (überlagern, Summe im Bild, laufender Song, die Uhr führt, Panel Player). Rückbau-Liste: Kennung der Naht N5, Anzeige (`a22e797`, Eigenes davor `473ea27`; dazwischen ist der Sitzungs-Fall rot). **Das Handbuch beschreibt, was gebaut und per Test belegt ist — im Programm gehört ist davon noch nichts**; nach dem Hörtest gegenlesen |
| 0.32.0 | 2026-10-06 | Session 98 — **C5b Schritt 4: das Player-Panel zeigt den laufenden Song als „Composer - <Song>".** Entscheid Patrik zum Play-Knopf: a — er folgt der Komposition, auch in einer Lücke (§8). **Eigenes:** die Sitzung verschickt für den laufenden Song auch `PlaybackPositionEvent` und `PlaybackStateEvent` („läuft" am Bus = die Komposition läuft; dem Renderer getrennt, ob der Song läuft), meldet nach einem dazwischenfunkenden Player noch einmal, hält `tonStand` und `springeImSong`; `Tonfuehrung::stelleImLaufenden` (Anteil der Songlänge → Stelle der Komposition, im Clip geklemmt), `songDauerMs`, `playerAngehalten`; `AppAnbindung`: `fuehrtTon`, `tonStand`, `springeImSong` (§4). **Naht N5, Anzeige** — vier Stellen in `PlayerPanel.cpp` (§12.2). Belegt mit dem echten Player-Panel am Sitzungs-Fall in `test_ComposerNahtTon.cpp` und in `test_ComposerTonfuehrung.cpp`. Rückbau-Liste: Zeile Schritt 4. 931 Cases. **Bekannte Lücke:** nach dem Freigeben ohne Titel im Player bleibt der letzte „Composer - <Song>" im Panel stehen (§4). **Offen:** im Programm nichts gesehen oder gehört; Schritt 5 (Benutzerhandbuch §15) |
| 0.31.0 | 2026-10-06 | Session 98 — **Entscheid zum Player-Panel geändert (Patrik), gebaut ist nichts.** Statt „das Panel zeigt die Komposition" (S97, b) gilt: **das Panel zeigt den laufenden Song als „Composer - <Song>"** — Künstler, Stelle im Song, Länge; ein Klick in den Balken springt im Song und damit in der Komposition; in einer Lücke bleibt der letzte Song stehen; der Loop-Knopf ist ausgegraut (§8, §11, §4). Naht „N5 wächst" neu gefasst (§12.2): Titel, Stelle und „läuft" kommen über die bestehenden Ereignisse, die der Composer für den laufenden Song verschickt; am Panel vier Stellen statt der drei übergangenen Ereignisse plus drei Abfragen. Offen: was der Play-Knopf in einer Lücke zeigt |
| 0.30.0 | 2026-10-06 | Session 98 — **C5b Schritt 3: Naht N13, die Visuals sehen die Summe.** `MainWindow::onAudioUpdate` fragt nach `currentStream()` `composer::tonMixer` (neu in `AppAnbindung`): führt der Composer Ton, kommen Spektrum und Wellenform von seinem Mixer, sonst vom Stream des Players wie immer (§12.2). Belegt: `composer::tonMixer` am Sitzungs-Fall in `test_ComposerNahtTon.cpp`; Wellenform, Kanalzahl und Spektrum je Kanal am Mixer der echten `BassEngine` in `test_ComposerNahtMixer.cpp`. Die Zeile in `MainWindow` selbst deckt kein Unit-Test — ihr Beleg ist der Sichttest. Rückbau-Liste: Kennung von N14 (`6f87331`), Zeile für N13. 931 Cases. **Offen:** im Programm nichts gehört oder gesehen; Schritt 4 (Anzeige des Player-Panels) |
| 0.29.0 | 2026-10-06 | Session 98 — **C5b Schritt 2: die Audio-Kanäle laufen im Mixer, die Uhr führt.** **Beleg vorab:** an einer Quelle des Mixers liefert die echte `BassEngine` dieselben Tags, denselben Songtext und dieselbe Dauer wie am Stream des Players (`test_ComposerNahtMixer.cpp`, dritter Fall, MP3 aus stillen Rahmen mit ID3v2). **Naht N14** (Entscheid Patrik: a): `leseTrackInfo` als freie Funktion in `audio/AudioPlayer`, `load` ruft sie auf (§12.2, vor dem Bau eingetragen). **Eigenes:** `Komposition::toeneBei` (alle Ton-Clips an der Stelle, mit Vorlauf) ersetzt `tonBei`; `Tonfuehrung` neu gegen `IAudioEngine` und `IAudioPlayer` — Mixer, je Clip eine Quelle, Stelle bei Einstieg / Sprung / Abweichung, Pegel je Takt, Lautstärke und Mute des Players am Mixer, der laufende Song nach der Pegelkurve; `Sitzung` meldet ihn als `TrackChangedEvent` und an den Renderer, `tonMixer()` für N13 (§4). `PappMischEngine` in `PappTon.hpp` (`PappEngine` bleibt ohne Mixer — der N12-Fall „Engine ohne Mixer" steht unverändert). Tests umgeschrieben: `test_ComposerTonfuehrung.cpp` (sieben Fälle), der Sitzungs-Fall in `test_ComposerNahtTon.cpp`, sechs Prüfungen in `test_ComposerBearbeiten.cpp`. **Von mir gesetzt:** Vorlauf 2 s, Abweichung 80 ms mit Schonfrist 500 ms, Player bleibt beim Freigeben angehalten (§4). Kein Vergleichslauf: die Bildberechnung ist nicht berührt. 931 Cases. **Offen:** im Programm nichts gehört; bis N13 reagiert das Bild bei führendem Composer nicht auf die Musik; N7a zurücknehmen oder nicht |
| 0.28.1 | 2026-10-05 | Session 97, Abschluss — Rückbau-Liste: Kennungen der Commits von S97 eingetragen (Belege `72dfb06`, Build-System `b31b55a`, N12 `e7fe2d5` — Naht und Eigenes in einem Commit). Kopf: Stand der Stufen. **Stand:** die Engine kann mehrere Songs mischen, der Composer benutzt den Mixer noch nicht — im Programm überlagern sich die Audio-Kanäle weiter nicht. Nächster Schritt: `Tonfuehrung` auf den Mixer (die Uhr führt), der laufende Song, dann N13 und N5 |
| 0.28.0 | 2026-10-05 | Session 97 — **C5b Schritt 1: Naht N12, der Mixer in der Engine.** Voraussetzung erledigt: CMakeCraft v0.9.2 reicht `external_options` an App-Tests durch, `BASS_MIX` steht an Kern, Runner und Test-Target, BASSmix ist Pflichtpaket (Entscheid Patrik: fest linken). **Naht:** `IAudioEngine` bekommt sieben Mixer-Aufrufe mit Vorgabe „kann es nicht", `BassEngine` setzt sie mit BASSmix um und übergeht `bassmix.dll` in `loadPlugins` (§12.2). **Belegt** in `test_ComposerNahtMixer.cpp` an der echten `BassEngine` über die Schnittstelle. Rückbau-Liste: Zeile „C5b, Naht N12". **Zu hören ist davon nichts** — der Composer benutzt den Mixer noch nicht (Schritt 2: `Tonfuehrung`). 930 Cases |
| 0.27.0 | 2026-10-05 | Session 97 — **Entscheide zum Überlagern der Audio-Kanäle (Stufe C5b), gebaut ist nichts.** Vorher belegt in `test_ComposerBelegeTon.cpp` (vier Fälle, baut nichts am Composer): zwei Streams der echten `BassEngine` spielen zugleich; Spektrum und Pegel eines Streams kennen weder den anderen noch die eigene Lautstärke; der BASSmix-Mixer liefert die Summe mit den Pegeln, Hüllkurve und Stelle je Quelle; Verzug eines Pegelwechsels 460 / 90 / 30 ms bei Puffer 0,5 / 0,1 / 0 s. **Entschieden (Patrik):** ein Mixer in der Engine, die Uhr führt; die Visuals sehen die Summe mit den Pegeln; der laufende Song ist der mit dem höchsten Wert der Pegelkurve, ihm folgen Karaoke, Tags, Songwechsel-Zähler, Tag View, Audio Browser (§4); das Player-Panel zeigt bei führendem Composer die Komposition (§8); §11 vier Zeilen. **Nähte eingetragen vor dem Bau** (§12.2): N12 Mixer in der Engine, N13 Spektrum in `onAudioUpdate`, N5 wächst um die Anzeige des Player-Panels; offen dabei, wie `bassmix.dll` neben die Test-Exe kommt. **Vorgemerkt:** Überlagern von Ende und Anfang in der Schleife (§4). Rückbau-Liste: Zeile „Belege vor C5b". 928 Cases |
| 0.26.2 | 2026-10-05 | Session 96, Abschluss — **Befund Patrik nach dem Hören:** der Ton kommt, aber Audio-Kanäle überlagern sich nicht — es spielt immer nur ein Song, Ein- und Ausblenden zweier Songs überlappen nicht. Als nächster Schritt in §4 eingetragen: je Audio-Kanal ein eigener Stream, dann führt die Uhr. Kopf: Stand der Stufen. Rückbau-Liste unverändert (alle Commits von S96 sind eingetragen) |
| 0.26.1 | 2026-10-05 | Session 96 — **Befund Patrik: vom Audio-Kanal war nichts zu hören.** Aus dem Log der App und der gespeicherten `composer_demo_audio.lvcomp` belegt: drei Audio-Kanäle, der oberste (A1) leer, die Songs in A2 und A3 — und es spielte nur „der erste eingeschaltete Kanal" („the audio channel leads the player", danach kein geladener Song). Nachgestellt in `test_ComposerTonfuehrung.cpp` (der Test schlug vor dem Fix fehl), behoben: `Komposition::tonBei` zählt alle eingeschalteten Audio-Kanäle, bei mehreren Clips an einer Stelle gewinnt der oberste Kanal; `Tonfuehrung::songAnfang` geht über die Songs aller Kanäle (§4). 924 Cases. Benutzerhandbuch 1.91.1 |
| 0.26.0 | 2026-10-05 | Session 96 — **C5 Schritt 4: der Audio-Kanal im Panel** (eigener Code, keine Naht). Zeitachse: `audioKanalHinzu` (Rechtsklick → *Add audio channel*), Songs hineinziehen (`lasseFallen` trennt nach Art, `fallDateien`, „New audio channel" unter der letzten Zeile), Wellenform im Ton-Clip (`setHuelle`, mit dem Pegel skaliert), eigene Farbe; `setAudioEndungen`, `setSongLaenge`. Panel: Endungen und Song-Längen aus der Audio-Engine, Hüllkurven im Hintergrund (`ladeHuellen`, ein Thread nach dem anderen, beim Schließen abgebrochen), Clip-Zeile mit dem Einsatz im Song. Belegt in `test_ComposerPanel.cpp` (neuer Fall: Kanal anlegen, Song hineinziehen auch als echter Drop, falsche Art fällt nicht, Wellenform kommt, Kanten mit der Maus, Blende, Curve Editor, Datei). 923 Cases. Benutzerhandbuch 1.91.0. **Offen:** im echten Programm nichts davon gesehen oder gehört; die Anzeige des Player-Panels in Lücken |
| 0.25.0 | 2026-10-05 | Session 96 — **C5 Schritt 3: die drei Nähte, und die Sitzung führt den Player.** **N7a** `IAudioPlayer::setPegel` / `pegel` (Vorgabe 1), im `AudioPlayer` als zweiter Faktor neben der Lautstärke — Regler, gespeicherte Lautstärke und Ereignisse bleiben. **N7b** `VisualizerWidget::dienste()`. **N5 (Transport)** `PlayerPanel::onTransportCommand` fragt zuerst `composer::transport`. **Eigenes:** `Sitzung` hält eine `Tonfuehrung`: ist der Composer an und ein Audio-Kanal eingeschaltet, übernimmt sie den Player aus den Diensten des Fensters, taktet die Führung, stellt die Uhr nach, wenn der Ton führt, und gibt den Player frei bei Composer aus, ausgeschaltetem Audio-Kanal und beim Beenden; `Sitzung::fuehrende`, `fuehrtTon`, `springeZumSong`; beim Laden kommt die Länge der Songs aus der Engine (`probeDurationMs`). `AppAnbindung::transport`: Play / Pause, Stop, Next, Previous steuern den ganzen Composer, Lautstärke und Mute bleiben beim Player. **Belegt** in `test_ComposerNahtTon.cpp`: der echte `AudioPlayer` mit einer Engine aus Pappe (Pegel; Playlist ruht ohne Naht) und die Sitzung über die echte Fassade mit dem echten Player-Panel (Übernehmen, der Ton führt, Sprung des Players und des Composers, Pegel, alle Transport-Befehle über den Bus, Freigeben). Damit ist eine Komposition mit Audio-Kanal **zum ersten Mal zu hören** — anlegen lässt sich ein Audio-Kanal im Programm noch nicht, nur über die Datei. Kein Vergleichslauf: die Bildberechnung ist nicht berührt. 922 Cases. **Offen:** Audio-Kanal und Songs im Panel (anlegen, hineinziehen, Wellenform); im echten Programm ist noch nichts davon gehört |
| 0.24.0 | 2026-10-05 | Session 96 — **C5 Schritt 2: die Führung des Players** (eigener Code, keine Naht). `composer/Tonfuehrung`: `uebernimm` / `gibFrei` (die Playlist ruht über `setPlaylist(nullptr)`), `takt` (Einstieg, der Ton führt, die Uhr führt nach einem Sprung, Wanduhr in Lücken und nach dem Ende eines Songs), `setPegelSenke`, `songAnfang` (§4). Läuft gegen `IAudioPlayer`; belegt in `test_ComposerTonfuehrung.cpp` gegen einen Player aus Pappe (sechs Fälle: Playlist ruht und kommt wieder, Lücke / Einstieg / Drift / Clip-Ende, Sprung des Players und der Uhr auch bei stehender Uhr, Song kürzer als sein Clip und fehlende Datei, Pegel, Next / Previous). An der Sitzung hängt sie noch nicht — **zu hören ist weiter nichts**. Rückbau-Liste: Zeile C5. 919 Cases |
| 0.23.0 | 2026-10-05 | Session 96 — **C5 Audiokanal: Entscheide Patrik und das Modell** (eigener Code, keine Naht). **Entschieden:** der Ton führt die Zeit, solange ein Song spielt, in Lücken die Wanduhr (§4); ein Ton-Clip hat Start und Ende wie ein Bild-Clip und dieselbe Pegelkurve mit denselben Presets, voller Pegel = die Lautstärke des Player-Panels (§6); die Knöpfe des Player-Panels steuern bei eingeschaltetem Audiokanal den ganzen Composer (§8); drei kleine Nähte — N7a Pegel im Player, N5 Transport (vorgezogen aus C6), N7b Zugang über das Fenster — und die Playlist ruht ohne Naht (§12.2, eingetragen vor dem Bau). **Gebaut:** **ein Clip-Typ** `Clip` für Bild und Ton (`VisualClip` / `AudioClip` sind Namen dafür), `Kanal::clips` statt `visual` und `audio`; neu `einsatzMs`, `quellMs`, `songStelle`; `Komposition::tonKanal`, `tonBei`, `ergaenzeTon`; `laenge` / `clipEnde` zählen auch Ton-Clips; Datei: `end`, `offset`, `sourceLength` und die Pegelkurve am Ton-Clip, ein alter Ton-Clip ohne `end` reicht bis zum Schluss seines Songs (`SongLaenge` beim Laden). `Bearbeiten`: `fuegeSongEin`, `fuegeKanalEin` mit Art (Audio-Kanäle heißen A1, A2 …), `zieheKante` und `setzeGrenzen` führen den Einsatz mit und halten sich an die Länge des Songs, ein Clip wechselt nur in einen Kanal seiner Art. Im Panel erscheinen Ton-Clips damit schon als Blöcke; **zu hören ist noch nichts**. Belegt in `test_ComposerBearbeiten.cpp` (neuer Fall) und `test_ComposerWiedergabe.cpp` (Datei). 913 Cases |
| 0.22.0 | 2026-10-05 | Session 96 — **Presets im Rechtsklick-Menü der Zeitachse** (eigener Code, keine Naht). *Fade in* / *Fade out* listen die Blenden-Presets statt der festen Formenliste (Haken an dem Preset, dem die Form gleicht; ohne jedes Preset die sieben festen Formen als Rückfall); an einem Stück *Shape* mit den Stück-Presets. `Zeitachse::setzeBlendenPreset`, `setzeStueckPreset`. Belegt in `test_ComposerPanel.cpp` (Wahl je Art, die andere Art und unbekannte Namen ändern nichts, Dauer und andere Seite bleiben, Blende ohne Länge bekommt zwei Sekunden, der Curve Editor zeigt das gewählte Preset). 912 Cases. Benutzerhandbuch 1.90.0. **Offen:** die Menüs selbst prüft nur der Sichttest |
| 0.21.0 | 2026-10-05 | Session 96 — **Griffe der Stücke auf der Zeitachse** (eigener Code, keine Naht). Oben im Clip je Stück eine Raute (verschieben) und zwei Striche (Weite); `Zeitachse::stueckGriff`, Griffe `StueckLage` / `StueckLinks` / `StueckRechts`. `Bearbeiten`: `spielraumVon`, `verschiebeStueck`, `setzeStueckWeite` — erst festschreiben, dann an den Nachbarn anhalten (§9). Belegt in `test_ComposerBearbeiten.cpp` (neuer Fall: anlegen, finden, schieben, Weite, zwei Stücke, gestaucht anfassen) und `test_ComposerPanel.cpp` (Raute und Kanten mit der Maus, Anhalten an der Ausblendung, der Curve Editor folgt). 912 Cases. Benutzerhandbuch 1.89.0 |
| 0.20.1 | 2026-10-05 | Session 96 — **Der Curve Editor zeigt die Wirkung am Clip** (Entscheid Patrik: a). Eine Ausblendung ist rückwärts gezeichnet, ein gekipptes Stück als Buckel; gezogen wird in dieser Ansicht, die Form selbst bleibt (`Formflaeche::setAnsicht`, die Ansicht sitzt in `ortVon` / `xVon` / `yVon`); eine Zeile unter der Kurve sagt, wenn gespiegelt gezeigt wird. Belegt in `test_ComposerPanel.cpp` (Delle und Buckel, Ausblendung rückwärts, ein in der Ansicht gesetzter Punkt landet in der Form hinten und senkt den Pegel früh). 911 Cases. Benutzerhandbuch 1.88.1 |
| 0.20.0 | 2026-10-05 | Session 96 — **Das Fenster Curve Editor** (eigener Code, keine Naht; Entscheid Patrik, Rückfrage 2: a). `composer/KurvenEditor`: `Formflaeche` und `KurvenFenster` (§9). `Bearbeiten`: `Teil`, `hatTeil`, `teilBei`, `bereichVon`, `fuegeStueckEin`, `loescheStueck`. Zeitachse: Doppelklick öffnet die Kurve einer Blende oder eines Stücks, Rechtsklick-Einträge für Kurven und Stücke, `oeffneKurve`, `stueckHinzu`, `stueckLoeschen`, `setTeil` (gelber Strich); die Länge einer Blende mit eigenen Punkten lässt sich ziehen, `setzeBlendenForm` ändert nur seine Seite. Panel: das Fenster entsteht bei der ersten Kurve, folgt dem angeklickten Clip, Änderungen über Undo. Damit haben Grundpegel und Stücke eine Bedienung, und ein gestauchtes Teil zeigt beide Zahlen. Belegt in `test_ComposerPanel.cpp` (Doppelklick, Punkte mit der Maus, Undo, Presets, Dauer mit Stauchen und Festschreiben, Grundpegel, Stück mit Lage, Weite und Flipped, ohne Edit, Blende entsteht beim Öffnen). 911 Cases. Benutzerhandbuch 1.88.0. **Offen:** siehe §9 |
| 0.19.0 | 2026-10-05 | Session 96 — **Kurven-Presets** (eigener Code, keine Naht; Entscheid Patrik: a, wie die Trail-Presets). `composer/FormPresets`: `liste`, `finde`, `lade`, `speichere`, `loesche` je Art — Blenden `.lvfade`, Stücke `.lvshape`, mitgeliefert unter `asset/composer/fades` und `…/shapes` (Anker `README.md`), eigene unter `<AppData>/composer/…`; ein eigenes verdrängt ein mitgeliefertes, mitgelieferte sind geschützt, die andere Art wird abgelehnt. Mitgeliefert: sieben Blenden, drei Stücke. `test_ComposerFormPresets.cpp` (drei Fälle, auch: jedes mitgelieferte lädt). Rückbau-Liste: Zeile für C4 Teil 2, `asset/composer/` eigens. 911 Cases. **Offen:** das Fenster Curve Editor |
| 0.18.0 | 2026-10-05 | Session 96 — **Die Pegelform am Clip und in der Datei** (eigener Code, keine Naht). Entscheid Patrik: Grundpegel je Clip (a), alle Teile freiwillig. `VisualClip`: `grundpegel`, `ein`, `aus`, `stuecke` statt der Punktliste `kurve`; `pegelBei` rechnet aus den Teilen, `verteilung`, `vollerPegel`. `Kurvenform`, `BezierGriffe`, `kurvenWert`, `bezierWert` sind nach `Pegelform` umgezogen. Beim Ziehen einer Kante bleiben die eingestellten Dauern (kein Klemmen mehr); `Bearbeiten::schreibeFest`; die Zeitachse zeichnet die Blenden-Griffe an der gestauchten Lage und schreibt vor einem Zug fest. **Datei Version 2** (`base`, `fadeIn`, `fadeOut`, `shapes`); `levels` wird beim Laden übersetzt (`ausAltenPunkten`), der Verlauf bleibt. **Geändert:** die Ausblendung ist die rückwärts gelesene Blende — Ease in, Ease out, Exponential, Hold und Bézier wirken an der Ausblendung anders als in 0.13.0. Tests umgeschrieben (`test_ComposerBearbeiten`, `test_ComposerWiedergabe`), neu Grundpegel, Buckel, Stauchen und Zurückkommen, alte Dateien. 908 Cases. Benutzerhandbuch 1.87.0 |
| 0.17.0 | 2026-10-05 | Session 96 — **Pegelkurve, Entscheide Patrik und das Modell dazu** (§9). Form und Parameter getrennt: die Kurvenform ist zeitlos und als Preset speicherbar; Typ 1 Blende (Parameter Dauer, als Ausblendung an der y-Achse gespiegelt), Typ 2 Stück mitten im Clip (Parameter Weite und Position, an der x-Achse gespiegelt = oben und unten getauscht). Beim Strecken und Stauchen behalten die Teile ihre Weite, bis sie sich berühren, dann werden die sich berührenden gleichmäßig gestaucht; die eingestellten Werte bleiben, auch in der Datei; ein Zug von Hand an einem gestauchten Teil schreibt die berührende Gruppe auf den sichtbaren Stand fest. Der Editor wird ein eigenes, nicht modales Fenster **Curve Editor** (keine Naht). **Gebaut:** `composer/Pegelform` (`Pegelform`, `blendenPegel`, `stueckPegel`, `formAlsJson` / `formAusJson`, `verteile`) — reines Modell, am Clip noch nicht angeschlossen; `test_ComposerPegelform.cpp` (14 Fälle). 908 Cases. **Offen:** Ablage der Presets, Pegel zwischen den Teilen, alte Kurven mit eigenen Punkten, Anschluss an `VisualClip` und Datei, das Fenster |
| 0.16.1 | 2026-10-05 | Session 95, Abschluss — Rückbau-Liste: Kennungen der Naht-Commits N10 (`4826db1`) und N11 (`356a868`, mit eigenem Code in einem Commit) eingetragen. Stand der Stufen: C2b und C3a im Programm gesehen; C3b, C4 Teil 1 und C3c gebaut, Sichttest S95c offen. Nächster Schritt: der Punkte-Editor der Pegelkurve (§9) |
| 0.16.0 | 2026-10-05 | Session 95 — **C3c Schritt 2: die Kette eines Clips speichern** (Entscheid Patrik: über das Speichern von Effect Chain, immer als Speichern unter). **Naht N11** (§12.2): der Empfänger von `SaveEffectChainEvent` in `MainWindow` fragt den Composer — hängt Effect Chain an einem Clip, speichert `composer::speichereClipKette`; die Rückfrage zur Kette der App (`darfKetteErsetzen`) bleibt unberührt, darum sitzt die Naht am Ereignis und nicht in `speichereKette`. **Ablauf:** Dateidialog mit Vorschlag `<Name>_comp` neben dem Original; das Original selbst wird abgelehnt; benutzen weitere Clips der Komposition das alte Preset, fragt eine Liste zum Ankreuzen (alle angekreuzt), welche mitwechseln — Abbrechen dort schreibt nichts. `Sitzung::speichereClipKette`, `gebundeneDatei`, `andereClipsMitDerDatei`; `festerClipHeisstJetzt` (Wiedergabe / Bildquelle) lässt den gebundenen Clip seinen Renderer über den Namenswechsel behalten — Effect Chain bleibt am selben Renderer; Clips, die mitwechseln, laden aus der neuen Datei neu. Die Komposition gilt danach als geändert (Stern), der neue Verweis kommt mit ihrem Save in die `.lvcomp`. Belegt in `test_ComposerSitzung.cpp` über die echte Fassade (Original bleibt, neue Datei trägt die Kette, gewählte Clips wechseln, kein Neubinden, Bild). 894 Cases. **Offen:** die Dialoge selbst prüft nur der Sichttest; keine Rückfrage, wenn mit ungespeicherten Änderungen an der Kette eines Clips ein anderer Clip angeklickt wird; der Verweiswechsel läuft nicht über Undo — ein Undo eines früheren Schritts stellt auch den alten Verweis wieder her |
| 0.15.0 | 2026-10-05 | Session 95 — **C3c Schritt 1: Sync on Effect Chain — festhalten, binden, ruhen** (eigener Code, keine Naht). Im Composer-Panel der Schalter **Sync on Effect Chain** (`composer/syncEffectChain`). Solange eine Komposition das Bild liefert, hängt Effect Chain (und Visualizer Config) bei Sync an am Renderer des angeklickten Clips, sonst ruht sie; ist der Composer aus, wieder an der Kette der App — gemeldet über `VisualizerChangedEvent` (`Sitzung::bindeKette`, `Sitzung::setzeBus` vom Composer-Panel). **Der festgehaltene Clip** (`Wiedergabe::setFest`, `Bildquelle::haltFest`): er hat immer einen Renderer, auch außerhalb seiner Zeit und in einem ausgeschalteten Kanal; beim Bearbeiten folgt das Festhalten seinem Renderer. **Ein Renderer, an dem die Oberfläche hängt, verschwindet nie unter ihr:** was die Wiedergabe fallen lässt (Clip gelöscht, anderer Clip gewählt, GL-Kontext verloren), wird geparkt statt gelöscht (`Parkplatz`) und erst nach `gibFrei` auf dem Render-Thread gelöscht; nach dem Verlust des GL-Kontexts bekommt der festgehaltene Clip denselben Renderer samt geänderter Kette wieder. Hängt Effect Chain an einem Clip, wird auch bei stehender Uhr jedes Bild gerechnet (Zeitschritt null), damit eine Änderung sofort zu sehen ist. Die Auswahl im Panel geht erst mit dem nächsten Takt an die Bildquelle — mitten in einem Bearbeitungsschritt nennt sie den Clip schon unter seiner neuen Nummer. Kopf des Panels in zwei Zeilen (Datei-Zeile eigens). Belegt: `test_ComposerBildquelle.cpp` (festhalten, Kette ändern, GL-Kontext geht und kommt, Clip gelöscht, anderer Clip), `test_ComposerSitzung.cpp` (die Meldungen an den Bus über die echte Fassade: ruht, Clip, bearbeiten ohne Neubindung, Platzwechsel, anderer Clip, gelöscht, Sync aus, Composer aus, ohne Bus). 894 Cases. **Offen:** das Speichern (Schritt 2, mit Naht); im Ruhezustand zeigt Effect Chain seinen allgemeinen Hinweis „Select the Multi Effect visualizer…" — ein eigener Text bräuchte eine Naht in `MultiEffectPanel`; ungespeicherte Änderungen an der Kette eines Clips gehen verloren, sobald ein anderer Clip angeklickt wird. §9e: Entscheid Patrik zu Composer Variables (beide Vorschläge; preset-weit, Namen geschützt, nur lesbar). §9: die Pegelkurve in drei Teilen, je als Preset speicherbar, Blenden gespiegelt verwendbar |
| 0.14.0 | 2026-10-05 | Session 95 — **C3c entschieden und begonnen.** Entscheid Patrik: a — Effect Chain hängt am Renderer des angeklickten Clips, Änderungen wirken live; **Speichern über das gewohnte Speichern von Effect Chain, dann immer als Speichern unter**, der Clip wechselt auf die neue Datei, für weitere Clips mit derselben alten Datei ein Dialog mit Auswahl (§7). Befund: das Binden braucht keine Naht (`VisualizerChangedEvent`), das Speichern eine in `MainWindow::speichereKette`. **Schritt 0 gebaut:** `Wiedergabe::ersetzeKomposition` — beim Bearbeiten behält ein Clip seinen Renderer, auch an anderem Platz oder in anderem Kanal (Zuordnung über Datei und nächstgelegene Startzeit); das Visual läuft weiter, statt von vorn zu beginnen (der Neubeginn aus C3a entfällt); `Wiedergabe::rendererVon`. Belegt in `test_ComposerWiedergabe.cpp` (GL). **Neu §9e Composer Variables** (Wunsch Patrik): Modul mit Vorgabewerten, zwei Vorschläge (feste Variablen `comp_…`, freie `cv1` … `cv8` mit Kurven), offene Fragen. **§9:** nächste Schritte der Pegelkurve (zusätzliche Punkte in Blenden, Punkte verschieben, dann freie Punkte). 893 Cases |
| 0.13.0 | 2026-10-05 | Session 95 — **Stufe C4, erster Teil: Ein- und Ausblenden im Panel, mit Bézier** (eigener Code, keine Naht). **Modell:** neue Kurvenform `Bezier` (7. Form, `curve` 6) — eine kubische Kurve von (0,0) nach (1,1) mit zwei Anfassern x1, y1, x2, y2 je 0…1, wie `cubic-bezier` in CSS; `PegelPunkt::bezier`, `bezierWert`, `kurvenWert(PegelPunkt, t)`; in der Datei das Feld `bezier` nur an Bézier-Punkten, alte Dateien lesen sich unverändert. `Bearbeiten`: `Blenden` (Länge, Form und Anfasser je Seite), `liesBlenden`, `setzeBlenden`, `kurvenformName`. **Panel** (nur im Edit): oben im Clip je ein Griff für die Länge der Ein- und der Ausblendung (ziehen, rastet ein, Alt frei); Rechtsklick auf den Clip → **Fade in** / **Fade out** mit den sieben Formen (eine Blende der Länge null wird dabei 2 s lang); am angeklickten Clip die zwei Bézier-Anfasser zum Ziehen; alles über Undo. **Grenze:** Griffe gibt es nur, wenn die Kurve eine einfache Ein- / Ausblendung ist; eine Kurve mit eigenen Punkten aus der Datei bleibt unangetastet und ist im Panel nicht bearbeitbar — **freie Punkte im Clip sind der zweite Teil von C4 und noch nicht gebaut**. Belegt in `test_ComposerBearbeiten.cpp` (Kurve, lesen / setzen, Datei) und `test_ComposerPanel.cpp` (Griffe und Anfasser ziehen, Form wählen, speichern und lesen). 892 Cases |
| 0.12.0 | 2026-10-05 | Session 95 — **Stufe C3b: Neues hinein** (eigener Code, keine Naht). **Presets hineinziehen:** die Zeitachse nimmt Datei-URLs an, wie sie Effect Browser und Explorer liefern (`.avs`, `.milk`, `.lvfx`, `.lvfx2`) — nur bei eingeschaltetem Edit; der neue Clip ist 20 s lang mit vollem Pegel, in einer Lücke reicht er höchstens bis zum nächsten Clip, auf einem Clip landet er in der nächsten Lücke; Ctrl kürzt, Ctrl+Shift schiebt, Alt ohne Raster; mehrere Dateien liegen hintereinander; unter der letzten Zeile entsteht ein neuer Kanal. **Warnung bei Dateien mit Host Switcher** beim Hinzufügen (Entscheid S88, 1a — war für C3 zugesagt). **Kanäle** über Rechtsklick: Add channel, Rename channel..., Channel enabled, Delete channel; Delete clip. **Datei-Zeile:** New (leere Komposition mit einem Kanal, ohne Datei — „untitled", Edit geht an), Load... (wie File → Open Composition...), Save fragt bei einer neuen Komposition nach dem Namen. **Die Achse zeigt Luft hinter dem Ende** (ein Fünftel, mindestens 10 s; leer 30 s), damit sich über das Ende hinaus ziehen und dahinter fallen lassen lässt. Sichttest S95b: 17 bestanden, drei mit Anmerkung, B3 (Einrasten und Alt) unklar formuliert — steht in der Liste S95c noch einmal. Belegt in `test_ComposerBearbeiten.cpp` und `test_ComposerPanel.cpp` (auch ein echter Drop mit Datei-URLs); dabei gefunden und behoben: das Panel las den Kanal des angeklickten Clips aus der Sitzung, die der Achse während eines Schritts einen Augenblick nachläuft (Absturz beim ersten Clip in einem neuen Kanal). 891 Cases. **Offen aus §7:** Ziehen aus dem Browser prüft nur der Sichttest (der Unit-Test schickt das Ereignis selbst); Lock, Follow player, Zoom |
| 0.11.1 | 2026-10-05 | Session 95 — **Sichttest S95b (C3a) durch (Patrik). Befund daraus: das Ende wanderte nicht mit** — wurden Clips verschoben oder verkürzt, lief die Komposition bis zur alten Endzeit schwarz weiter, und Loop / Ende-Einstellung griffen erst dort. Ursache: die Datei trug eine feste Länge (gleich dem Ende des letzten Clips), und beim Bearbeiten wuchs sie nur. Nachgestellt in `test_ComposerBearbeiten.cpp` (der Test schlug vor dem Fix fehl), behoben: beim Lesen gilt eine Länge, die genau das Ende des letzten Clips ist, als „folgt dem letzten Clip" (Vorgabe Patrik); `Komposition::clipEnde`. **Neu im Panel:** Schalter **Fixed end** mit Zeitfeld (nur im Edit, über Undo), bei festem Ende eine eigene Marke und die leere Strecke schraffiert (§6). 889 Cases |
| 0.11.0 | 2026-10-05 | Session 95 — **Stufe C3a: Bearbeiten, was da ist.** C3 in drei Teile geschnitten: C3a (Schalter Edit, verschieben, Kanten, löschen, Mischart, Undo, Save), C3b (Presets hineinziehen, Kanäle anlegen, New / Load), C3c (Sync on Effect Chain). **Entscheid Patrik:** trifft ein gezogener Clip auf einen Nachbarn, hält er an; mit Ctrl wird der Nachbar gekürzt, mit Ctrl+Shift rücken die Nachbarn mit (§6). **CH1 steht oben** (Entscheid Patrik, §7). Gebaut: `composer/Bearbeiten` (reines Modell: `verschiebeClip`, `versetzeClip` — der Clip wechselt per Ziehen den Kanal, Wunsch Patrik —, `zieheKante`, `loescheClip`, `setzeGrenzen`); die Pegelkurve bleibt beim Ziehen einer Kante an ihrer Kante; `Sitzung::setzeKomposition` (die geänderte Fassung läuft an derselben Stelle weiter — die Renderer der Clips legen sich dabei neu an), `ungespeichert`, `speichere`, `speichereNach`; im Panel Schalter **Edit** (nach jedem Start aus), Ziehen mit Einrasten auf Raster, Clip-Kanten und Abspielmarke (Alt frei, Esc bricht ab), Entf, Auswahl **Blend**, **Save** / **Save As...**, Stern am Dateinamen; **Undo / Redo über den CommandBus der App** (Edit → Undo, Ctrl+Z), Schritte einer früheren Ladung gelten nicht mehr. Rückfrage Save / Discard / Cancel vor Open, Close und — **Naht N10** — beim Beenden. Belegt: `test_ComposerBearbeiten.cpp` (sechs Fälle, reines Modell), `test_ComposerPanel.cpp` (Ziehen mit und ohne Tasten, Kanalwechsel, Undo, Save an der echten Fassade), `test_ComposerSitzung.cpp` (bearbeiten im Lauf); 888 Cases. **Offen:** ein Edit bei laufendem Composer legt alle Renderer neu an (Visuals beginnen von vorn); die Rückfrage-Dialoge sind nur im Programm zu prüfen |
| 0.10.4 | 2026-10-05 | Session 95 — **Stufe C2b im Programm gesehen** (Patrik, Sichttest S95 nach der Liste `Sichttests_S95.md`, ohne das Sichttest-Werkzeug: alles in Ordnung). **Aus dem Log der App belegt** (Release-clang, RTX 4090): die Zusammenfassung beim Beenden mit eingeschaltetem Composer steht jetzt im Log (10:59:24, 10:59:47, 11:00:07 — der Befund aus S94 war bis heute im echten Programm nicht gesehen); ohne Resume kommt die Komposition nach dem Neustart ins Panel („restored composition", kein „on"); mit Resume setzt sie an der Stelle des Beendens fort (beendet bei 2,2 s, „resumed at 2 s", der erste Clip läuft bei 2,20 s an). Rückbau-Liste: Kennungen der drei Naht-Commits eingetragen |
| 0.10.3 | 2026-10-05 | Session 95 — **C2b, die drei Nähte gebaut:** N8 — Anmeldung des Panels `composer` in `PanelAutoReg.cpp` (View → Panels → Composer); N4 — ein verzögerter Aufruf im Konstruktor von `MainWindow` stellt die zuletzt geladene Komposition wieder her; N9 — im Reiter Panels der Settings die Auswahlen **Open Composition** und **End of a Composition**. Rückbau-Liste um die Nähte erweitert. §7 ergänzt (Wünsche Patrik): das Panel dockt wie jedes andere; für C3 der Schalter **Edit** (Bearbeiten nur bei eingeschaltetem Schalter, nicht zu verwechseln mit Lock); für C4 Übergänge auch als Bézier-Kurve |
| 0.10.2 | 2026-10-05 | Session 95 — **C2b, das Panel zum Ansehen** (eigener Code, keine Naht; noch nicht angemeldet): `composer/ComposerPanel` — Kopf mit Schalter **Composer on**, Play / Pause in einem Knopf, Stopp, Loop, Resume after restart, Stelle und Länge als Zeit, Dateiname; `Zeitachse` mit Lineal, Abspielmarke (Klick und Ziehen springen; bei ausgeschaltetem Composer gestrichelt), Kanälen als Zeilen — **der oberste Kanal des Stapels in der obersten Zeile** — und Clips als Blöcken mit ihrer Pegelkurve; Clip-Zeile mit Kanal, Datei, Start, Ende und Mischart des angeklickten Clips. Die Achse zeigt die ganze Länge auf ihrer Breite; ein Zoom kommt mit C3. Das Panel bedient nur die Sitzung des Fensters. Belegt in `test_ComposerPanel.cpp` an der echten Fassade (Knöpfe, Lineal, Clips; `LUMIVIZ_COMPOSER_PANEL_BILD` legt ein Bild ab); Rückbau-Liste um C2b, Panel erweitert; 881 Cases |
| 0.10.1 | 2026-10-05 | Session 95 — **C2b, die Sitzung** (eigener Code, keine Naht): `composer/Sitzung` hält je Fenster die geladene Komposition — **geladen ist nicht an**; Composer on setzt die Bildquelle an der gemerkten Stelle, angehalten; Play, Pause, Stopp, Sprung, Loop (`composer/loop`); Play bei ausgeschaltetem Composer schaltet ihn ein. **Das Ende ohne Loop** (Entscheid Patrik: alle drei wählbar, Vorgabe halten): halten, an den Anfang, Composer aus (`composer/atEnd`) — §8, §11, N9 um die zweite Auswahl erweitert. Öffnen von Hand hält auf Wunsch am Anfang (`composer/openHolds`). **Über den Neustart:** `sichereStand` bei `aboutToQuit`, `stelleWiederHer` für den Start (Naht N4 folgt) — ohne Resume ins Panel und Composer aus, mit Resume der Stand vom Beenden. Die Bildquelle spiegelt den Stand der Uhr je Bild, damit die Anzeige ihn ohne Render-Mutex liest. `AppAnbindung` reicht Menü und Start an die Sitzung weiter; die Zusammenfassung beim Beenden hängt jetzt dort. Belegt in `test_ComposerSitzung.cpp` über die echte Fassade (vier Abschnitte: geladen ist nicht an, Öffnen von Hand, das Ende in drei Arten samt Schleife, Neustart mit und ohne Resume); Rückbau-Liste um C2b, Sitzung erweitert; 879 Cases |
| 0.10.0 | 2026-10-05 | Session 95 — **C2b vorbereitet.** §7: Entscheid zum Panel Effect Chain (Patrik: a, mit Ergänzung) — in C2b bleibt es an der Kette der App, das Composer-Panel zeigt den angeklickten Clip in einer eigenen Zeile; in C3 kommt der Schalter **Sync on Effect Chain** (an: Kette des angeklickten Clips, aus: Effect Chain ruht). Transport mit Play, Pause, Stopp und Loop (Wunsch Patrik). §12.2: Naht N8 für C2b gefasst — ein Include und eine Anmeldung in `PanelAutoReg.cpp`. **Der Start** (Patrik): §8 und C7 neu gefasst — die Komposition kommt beim Start ins Panel; Knopf **Resume after restart** aus: Composer aus, die Effektkette läuft, im Panel wird vorbereitet; an: der Stand vom Beenden kommt wieder. Daraus: „geladen ist nicht an" — das Panel hält die Komposition, **Composer on** setzt die Bildquelle. Naht N4 neu gefasst und nach C2b vorgezogen: ein Aufruf in `MainWindow`, `StartPreset.hpp` bleibt unberührt. **Öffnen von Hand** (Patrik): startet sofort, per Einstellung „hält am Anfang" — neue Naht N9 in `SettingsPanel.cpp` |
| 0.9.2 | 2026-10-04 | Session 94 — **Zusammenfassung auch beim Beenden der App:** `starteKomposition` hängt sich einmal je Fenster an `aboutToQuit` und schreibt die Messung der noch laufenden Komposition ins Log; die Bildquelle behält die Anlauf-Zeilen über ihr Aufräumen hinweg (der Render-Thread ist dann schon angehalten). Nur eigener Code, keine Naht. Belegt: der Fassaden-Test beendet die Ereignisschleife mit laufender Komposition, die Zusammenfassung steht in der Ausgabe; 877 Cases. **Die Release-clang-Exe der App enthält diesen Stand noch nicht** (sie lief beim Bau, Stand 23:36 Uhr) |
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
