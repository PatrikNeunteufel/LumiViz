# Bootstrap & Integration — Init-Reihenfolge, Services, Event-Flow, Shutdown

> **Version:** 1.2.0
> **Datum:** 2026-08-07
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## 1. Übersicht

Einstiegskette (verifiziert gegen `main/main.cpp`, `src/Application.cpp`,
`src/UI/MainWindow.cpp`):

```
main() ──► Application::init() ──► Application::run() ──► Application::shutdown()
                 │
                 └── erstellt QApplication + MainWindow
                            │
                            └── MainWindow besitzt ServiceContainer, DockManager,
                                MenuManager und registriert alle Services
```

Wichtig gegenüber der Alt-Doku: **nicht `Application`, sondern `MainWindow` erstellt den
`ServiceContainer`** und registriert die Services. `Application` kümmert sich um
Qt-Lifecycle, GPU-Auswahl, Preset-Verzeichnisse und die Frame-Loop.

DI-Container-Details (Lifetimes, API, Thread-Safety) stehen in der Modul-Doku
[`../../include/services/ServiceContainer.md`](../../include/services/ServiceContainer.md).

---

## 2. Init-Reihenfolge

### 2.1 Application::init() (`src/Application.cpp`)

1. GPU-Enumeration + Auswahl (`GpuInfo`/`GpuSelector`, `gpu.ini`) — vor QApplication
2. `QApplication` erstellen (Name/Version/Organisation setzen)
3. User-Preset-Verzeichnisse setzen (nach QApplication wegen `QStandardPaths`,
   vor MainWindow): `ColorGradientModule`, `SmoothingModule`, `AudioSourceModule`
4. `MainWindow` erstellen (erzeugt OpenGL-Kontext) und `show()`
5. Frame-Timing initialisieren

### 2.2 MainWindow-Konstruktor (`src/UI/MainWindow.cpp`)

1. `ServiceContainer` erstellen
2. `registerSingleton<IEventBus, EventBus>()`
3. `setupAudioServices()` — Audio-Services registrieren **und sofort auflösen**
   (siehe 3.)
4. `setupUi()`:
   - `DockManager` erstellen (erzeugt intern PanelManager + alle Panels aus der
     PanelRegistry, abonniert seine Events)
   - `setupMenuBar()` — `MenuManager` + `buildMenuBar()`; Panels-/Perspectives-Menüs
     vom DockManager befüllen lassen
   - `setupStatusBar()` — FPS-Label
   - `setupDefaultLayout()` — **erst** initialen Visualizer erstellen
     (`setVisualizer("pulsing")`), **dann** `restoreLayout()` (Qt-ADS kann nur
     existierende Widgets positionieren); Default-Perspektive speichern, falls keine
     existiert
   - `setupEventHandlers()` — MainWindow-Events abonnieren
5. Audio-Update-Timer starten (`QTimer`, 33 ms ≈ 30 Hz → `onAudioUpdate()`)

### 2.3 Application::run()

- Frame-Timer (`QTimer`, `Qt::PreciseTimer`) treibt `MainWindow::requestRender()`
  + FPS-Messung; Intervall je nach FrameMode (Limited/Unlimited/VSync)
- Qt-Signal `MainWindow::frameModeChangeRequested(int)` → FrameMode + VSync umschalten
- danach `QApplication::exec()` (Qt-Event-Loop, keine manuelle processEvents-Schleife)

### 2.4 Registries (automatisch)

Die fünf Registries (Menu, Panel, Dialog, Widget, Visualizer) initialisieren sich
**lazy beim ersten `instance()`-Aufruf** über ihre `initXxxDefaults()` — es gibt keinen
expliziten Registrierungs-Schritt im Bootstrap
(Details: [Registries.md](Registries.md)).

---

## 3. Service-Registrierung (Ist-Stand)

Alle Registrierungen passieren im MainWindow (`setupAudioServices()` + Konstruktor):

| Interface | Implementierung | Anmerkung |
|-----------|-----------------|-----------|
| `IEventBus` | `EventBus` | erster registrierter Service |
| `IAudioEngine` | `BassEngine` | Factory ruft `initialize()`; Fehler → nullptr |
| `IPlaylist` | `Playlist` | Factory mit **captured** EventBus-Referenz |
| `IAudioPlayer` | `AudioPlayer` | Factory mit captured Engine/EventBus/Playlist; verbindet Player↔Playlist |

**Deadlock-Regel:** Der historische `std::mutex`-Deadlock (resolve() in einer Factory)
ist zwar seit dem Wechsel auf einen rekursiven Mutex behoben (Test-Fundament Phase 3),
der Code vermeidet verschachtelte `resolve()`-Aufrufe in Factories aber weiterhin:
Abhängigkeiten werden **vor** der Registrierung aufgelöst und in die Factory-Lambda
captured; nach jeder Registrierung erzwingt ein `tryResolve<T>()` die sofortige
Instanziierung. Effektiv sind damit alle Audio-Services am Ende des
MainWindow-Konstruktors bereits erstellt und verdrahtet.

---

## 4. Event-Flow (Beispiele)

### Menü → DockManager

```
User klickt "View → New Visualizer"
  → MenuAutoReg-Callback: eventBus.publish(CreateVisualizerEvent{})
  → DockManager (Subscriber): createVisualizer(...), respektiert allowMultiple
```

### Panel → Visualizer

```
User wählt Visualizer im VisualSelectPanel
  → eventBus.publish(ChangeVisualizerEvent{id})
  → DockManager: aktiven VisualizerWidget auf neuen Effekt umschalten
  → VisualizerWidget publiziert VisualizerChangedEvent (u. a. für ConfigPanel)
```

### Audio → Visualizer (kein EventBus!)

```
QTimer (30 Hz) → MainWindow::onAudioUpdate()
  → IAudioPlayer::update() (publiziert Positions-/State-Events für die UI)
  → IAudioEngine::getFFTData()/getWaveformData()
  → direkt an alle VisualizerWidgets: updateSpectrum()/updateWaveform()
```

Die hochfrequenten FFT-/Waveform-Daten gehen als direkter Methodenaufruf an die
Visualizer, nicht über den EventBus; der Bus transportiert die UI-relevanten
Audio-Events (Track/Position/State, siehe [Event_System.md](Event_System.md)).

---

## 5. Shutdown-Reihenfolge

```
MainWindow::closeEvent → QCoreApplication::quit()
  → aboutToQuit feuert (SYNCHRON im Main-Thread, Handler in
    Registrierungs-Reihenfolge — siehe Regel 6.1):
       MainWindow            : Layout + Panel-Zustände sichern
       VisualizerWidget      : Render-Thread anhalten und joinen
       SettingsPanel         : laufende Kamera-Testaufnahme synchron abbauen
       LiveVideoFeed         : Riegel setzen (keine Feed-Neustarts mehr)
       VideoFrameCache       : Ladevorgänge abbrechen, Decoder-Thread beenden
  → Application::run() kehrt zurück ("Event loop exited")
  → Application::shutdown():
       MainWindow.reset()          // vor QApplication!
         → ~MainWindow:
              DockManager::closeAll() + reset()   // VOR QMainWindow-Basisdestruktor
                → DockManager::unsubscribeFromEvents() (IDs werden abgemeldet)
              ServiceContainer.reset()            // zerstört Audio-Services + EventBus
       LiveVideoFeed::herunterfahren()            // Feeds still, Wandler-Thread aus
       [Notausgang: std::_Exit(0), falls ein Multimedia-Feed lief — s. 6.4]
       QApplication.reset()
```

Kritische Regeln (im Code dokumentiert):

1. **DockManager vor dem QMainWindow-Basisdestruktor zerstören** — Qt-ADS trackt
   Floating-Container per QPointer; falsche Reihenfolge → Dangling-Pointer/Crash.
2. **Layout-Restore erst nach Widget-Erstellung** (siehe 2.2 Schritt 4).
3. **Event-Subscriptions in Destruktoren abmelden** (DockManager macht es vor;
   Unsubscribe-Pflichten: [Event_System.md](Event_System.md), Abschnitt 5).

Der Log ist das Diagnose-Werkzeug für diese Kette: `BasicLogger` schreibt
zeilenweise geflusht nach `MyViz.log`, jede Stufe oben hinterlässt eine Marke.
**Fehlt beim Hänger die Zeile `Event loop exited`, sitzt er in einem
`aboutToQuit`-Handler, nicht in `shutdown()`** — genau diese Unterscheidung hat
in Session 71 den Deadlock gefunden, ohne dass ein Debugger nötig war.

---

## 6. Lebenszyklus-Vertrag: Threads und fremde Pipelines

Aufgestellt in Session 71 nach dem Kamera-Teardown-Deadlock (S70/S71). Die
Regeln gelten für **jede** Ressource, die einen eigenen Thread betreibt oder
eine fremde Pipeline besitzt (Qt Multimedia, GL-Kontext, BASS, Decoder).

### 6.1 Einschalten und Herunterfahren sind spiegelbildlich — und zentral

Was zuletzt hochgefahren wird, wird zuerst abgebaut. Das gilt nur, wenn die
Reihenfolge überhaupt **festgelegt** ist: `aboutToQuit`-Handler laufen in der
Reihenfolge ihrer Registrierung, und die hängt bei lazy erzeugten Singletons
davon ab, wann sie das erste Mal gebraucht wurden — also vom Nutzerverhalten.

> **Regel:** Ein neuer Dienst mit Thread oder Pipeline wird in Abschnitt 5
> eingetragen. Ist seine Position gegenüber anderen Handlern wichtig, darf er
> sich nicht auf `aboutToQuit` verlassen, sondern gehört als expliziter Aufruf
> in `Application::shutdown()` (Vorbild: `LiveVideoFeed::herunterfahren()`).

Lazy erzeugte Dienste, deren Abbau-Position zählt, werden beim App-Start
bewusst einmal angefasst, damit ihre Registrierung nicht vom Zufall abhängt.

### 6.2 Der Abschalt-Vertrag für Producer über Thread-Grenzen

Läuft ein Producer (Kamera, Player, Decoder) auf einem anderen Thread als der
Consumer, darf die Quelle **nie** gestoppt oder zerstört werden, solange der
Consumer noch eines ihrer Objekte anfasst. Verbindliche Reihenfolge:

1. **Totflagge setzen** — der Consumer lässt ab jetzt jedes Element liegen.
2. **Quelle abklemmen** — es kommt nichts Neues mehr nach (`disconnect()`).
3. **Barriere** — warten, bis die bereits zugestellten Elemente durch sind
   (leeres Lambda hinter die Queue posten, auf Semaphore warten, **mit
   Timeout**). Erst danach sind alle geliehenen Puffer zurückgegeben.
4. **Stoppen**, dann **zerstören**.

Referenz-Implementierung: `LiveVideoFeed::feedStilllegen()` /
`wandlerBarriere()` ([LiveVideoFeed.cpp](../../src/services/LiveVideoFeed.cpp)).

**Warum das keine Formsache ist:** Wurde Schritt 1–3 übersprungen, mappte der
Wandler-Thread ein `QVideoFrame` einer bereits sterbenden Media-Foundation-
Pipeline, blockierte im Grafiktreiber und hielt deren Puffer für immer. Daran
starben die `nvwgf2umx`-Worker nie — die App ließ sich nicht mehr schließen.
Belegt durch den Log: `LiveVideoFeed`s `wait(5000)` lief in **jedem** Lauf mit
Feed exakt in den 5-Sekunden-Timeout.

### 6.3 Fremde Puffer sind geliehen

Ein `QVideoFrame` (allgemein: jedes Handle aus einer fremden Pipeline) ist ein
**ausgeliehener** Puffer. Er darf nicht über den Lieferkontext hinaus gehalten,
nicht auf fremden Threads gemappt und nie in einer Warteschlange geparkt
werden, während seine Quelle abgebaut wird. Wer ihn hält, blockiert das
Teardown der Pipeline.

### 6.4 Qt-spezifische Fallen im Teardown

1. **`deleteLater()` ist nach `aboutToQuit` wirkungslos** — es gibt keine
   Event-Loop mehr, die es ausführt. Objekte sterben dann erst in der
   Event-Queue-Entsorgung des `~QApplication` oder per Parent-Destruktor
   mitten im Fenster-Abbau. Im Shutdown-Pfad **synchron** zerstören
   (Vorbild: `SettingsPanel::beendeTestaufnahme(true)`).
2. **Queued-Lambdas dürfen nie das letzte Eigentum tragen.** Wird das Event
   nur noch entsorgt statt zugestellt, zerstört die Entsorgung das Objekt an
   der denkbar schlechtesten Stelle. Eigentum bleibt beim Dienst
   (Friedhof-Muster im `LiveVideoFeed`), das Lambda trägt nichts.
3. **Kein unbefristetes `wait()` im Teardown-Pfad.** Jeder Join bekommt einen
   Timeout und eine Log-Zeile, sonst wird aus einer Blockade ein Hänger, den
   kein Notausgang mehr erreicht.
4. **Ein einzelnes `quit()` kann verloren gehen — Joins wiederholen.**
   Unmittelbar nach dem Abbau einer Multimedia-Pipeline ist der Thread, der
   deren Frames verarbeitet hat, noch nicht quit-fähig: er arbeitet interne
   Ereignisse der sterbenden Pipeline ab. Ein `quit()` in diesem Fenster
   verpufft, und das anschließende `wait()` läuft in den Timeout — **auch
   dann, wenn der Thread längst idle ist und seine Event-Loop normal läuft**.
   Gemessen (S71, Kamera am Standalone): `quit(); wait(5000)` scheiterte in
   3 von 3 Läufen, dieselbe Stelle mit einer Pause davor gelang in 3 von 3.
   Ein von innen gepostetes Quit-Event half ebenso wenig. Belastbare Lösung
   ist die **Wiederholung** — `quit()` + kurzes `wait()` in der Schleife, bis
   der Thread annimmt (`LiveVideoFeed::herunterfahren()`, greift im zweiten
   Versuch nach ~200 ms). Eine feste Wartezeit wäre geraten; die Schleife
   korrigiert sich selbst.
5. **Notausgang statt Ratlosigkeit:** Bleibt eine Fremdbibliothek nachweislich
   hängen, beendet `Application::shutdown()` den Prozess nach **vollständigem
   eigenem Cleanup** per `std::_Exit(0)`. Das Kriterium muss die Ursache
   abdecken, nicht ihr auffälligstes Symptom — es heißt „ein Multimedia-Feed
   lief", nicht „eine Kamera lief" (S71: ein reiner Datei-Feed hing genauso).
   Der Notausgang ist berechtigt: nach einem Kamera-Lauf hängt der Prozess
   **nach `main()`** im `~QGuiApplication` bzw. in fremden statischen
   Destruktoren — am Standalone reproduziert, mit einem Marker hinter dem
   letzten eigenen Aufruf belegt. Das ist außerhalb unseres Codes; heilbar
   ist nur, was davor liegt.

### 6.4a Abbau darf nie im Renderpfad ausgelöst werden

Ein Render-Thread ruft seine Dienste in **jedem Frame** an — Startaufrufe sind
dort idempotent gemeint, nicht als Auftrag. Baut ein Dienst asynchron auf
(Queued-Invoke auf den Main-Thread), steht das Ergebnis erst Frames später;
bis dahin sieht jeder weitere Frame denselben „noch nicht da"-Zustand.

> **Regel:** Ein asynchroner Aufbau bekommt einen **Auftrags-Riegel** (je
> Schlüssel und Zielzustand genau ein laufender Auftrag). Ohne ihn erzeugt der
> Renderpfad pro Frame einen Auftrag — und wenn der Aufbau seinen Vorgänger
> abräumt, wird daraus eine Endlosschleife aus Auf- und Abbau, die den
> Main-Thread blockiert.

Referenz: `LiveVideoFeed::m_imBau` / `bauVormerken()`. Der Fehler kostete in
S71 einen kompletten Fix-Zyklus: er entstand erst dadurch, dass der Abbau
(korrekt) in den Aufbaupfad gezogen wurde, und äußerte sich als eingefrorene
App, bei der die Kamera nie anlief.

### 6.5 Messen statt vermuten

Der Deadlock kostete drei Anläufe, weil jeder auf einer plausiblen Hypothese
statt auf einer Messung beruhte (erst „Treiber-intern, nicht heilbar", dann
„Feeds akkumulieren", dann „Wandler steckt im `map()`" — alle drei falsch).
Was half, war ein **Prüfstand mit echter Hardware**: `AvsStandalone` mit
`--kamera-freigeben` und `--feed-teardown-messen` fährt Preset-Wechsel mit
laufender Kamera und misst den Abbau. Zähler statt Vermutungen (angefangene
vs. fertige Frames zeigen eine Blockade; ein Timer-Tick zeigt, ob eine
Event-Loop überhaupt noch läuft), und **jede** Erklärung gegen ihre Gegenprobe
gestellt — die Kausalität stand erst fest, als 3/3 gegen 3/3 sauber trennten.
Für Teardown-Fragen ist das der vorgesehene Weg; `MyViz.log` ist zeilenweise
geflusht und trägt die Marken aus Abschnitt 5.

---

## Siehe auch

- Modul-Doku: [`ServiceContainer.md`](../../include/services/ServiceContainer.md) (DI-API, Lifetimes)
- [Event_System.md](Event_System.md) — EventBus, Handler, Thread-Safety
- [Registries.md](Registries.md) — Lazy-Init der Komponenten-Registries
