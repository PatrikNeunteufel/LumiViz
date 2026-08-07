# LiveVideoFeed — Live-Videoquellen je Chain-Node (Datei-Streaming + Kamera)

> **Version:** 1.3.0
> **Datum:** 2026-08-07
> **Typ:** CppModuleDoc
> **Status:** Implementiert (videoSource-Strang, Session 70/71)
> **Modul:** lumi::services::LiveVideoFeed (Singleton, QObject ohne Q_OBJECT)
> **Dateien:** LiveVideoFeed.hpp / LiveVideoFeed.cpp
> **Namespace:** lumi::services
> **Abhängigkeiten:** Qt Multimedia (QMediaPlayer, QCamera, QMediaCaptureSession, QVideoSink), VideoFrameUtil.hpp
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Gegenstück zum `VideoFrameCache` (deterministisch, dekodiert die Datei EINMAL
komplett in den Speicher): dieser Dienst liefert dem **videoSource-Knoten**
UHRZEITGETRIEBENE Frames — **Datei-Streaming** über `QMediaPlayer` (lange
Videos ohne RAM-Kosten, `setLoops`/`setPlaybackRate`) und **Kamera** über
`QCamera` + `QMediaCaptureSession`. Beide münden in einen `QVideoSink` je
Feed; das jeweils letzte Bild liegt als RGBX8888-`QImage` unter Mutex bereit,
der Render-Thread holt es je Frame per `letztesBild(nodeId, &nummer)` ab
(die laufende Nummer spart GL-Uploads unveränderter Bilder).

Live heißt **nicht reproduzierbar** — Sonden/Standalone nutzen den
VideoFrameCache-Weg des Knotens (`streaming` aus bzw. Quelle „Testaufnahme").

## 2. Kamera-Vertrag (Offene_Punkte §7)

Ein Kameragerät startet **NIE automatisch**: `starteKamera()` ist ein No-op,
solange nicht `erlaubeKamera()` in DIESEM App-Lauf durch eine ausdrückliche
Nutzeraktion gesetzt wurde (Panel-Knopf „Kamera freigeben" oder der
Testaufnahme-Klick im Settings-Tab „Kamera"). Ein Preset-Laden alleine öffnet
damit nie den Windows-Berechtigungsdialog. Die Freigabe ist bewusst
**nicht persistiert** (gilt bis App-Ende).

## 3. API-Kern

- `instance()` — Singleton; lebt auf dem Main-Thread (Event-Loop-Pflicht von
  Qt Multimedia; Muster VideoFrameCache).
- `starteDatei(nodeId, pfad, loop, tempo)` — idempotent je Frame aufrufbar;
  nur ein Quellen-Wechsel baut den Feed neu, ein Tempo-Wechsel setzt nur die
  Abspielrate nach (Queued-Invoke).
- `starteKamera(nodeId, geraeteId)` — idempotent; Geräte-Auflösung über
  `QCameraDevice::id()` (Ausweich: description). Unbekannte ID startet
  nichts und wird als Feed-Platzhalter gemerkt (kein Retry-Spam).
- `stopp(nodeId)` — beendet und entsorgt den Feed (Qt-Objekte sterben auf
  dem Main-Thread, über den Abschalt-Vertrag in §4a).
- `feedGelaufen()` — lief in diesem App-Lauf irgendein Qt-Multimedia-Feed
  (Kamera, Datei-Streaming oder eine Settings-Testaufnahme über
  `merkeFremdenFeed()`)? Steuert den Notausgang in `Application::shutdown()`.
- `letztesBild(nodeId, &nummer)` — Mutex-Kopie (QImage-COW) + Bild-Nummer.
- `testaufnahmenOrdner()` — benutzerlokaler Ablageordner der Kamera-Clips
  (`AppDataLocation/testaufnahmen`, wird angelegt) — bewusst AUSSERHALB des
  Repos (Kameramaterial ist persönlich).

## 4. Threading

`starteDatei`/`starteKamera`/`stopp` sind render-thread-tauglich: sie prüfen
unter Mutex den Quellen-String und queuen den (Um-)Bau als Lambda-Invoke auf
den Main-Thread. **LAG-BEFUND S70 (zwei Anläufe, endgültig in 1.2.0):**
(1) Die RGBX-Wandlung je Kamera-Frame auf dem GUI-Thread fror das UI ein
(Event-Loop gesättigt, Drag&Drop tot). (2) Wandlung auf dem RENDER-Thread
hängt bei HARDWARE-Frames — `map()`/`toImage()` funktioniert nur im
Lieferkontext; der Render-Thread blockierte, das Fenster-Schließen wartete
auf ihn, die Kamera blieb an. **Endgültige Architektur: eigener
WANDLER-Thread** (`m_wandler` + `m_wandlerKontext`): die
`videoFrameChanged`-Verbindung wird QUEUED an den Kontext zugestellt, die
Wandlung (`videoFrameZuBild()`, VideoFrameUtil.hpp) läuft dort, der
Pipeline-Puffer geht sofort zurück, fertige QImages liegen unter Mutex —
GUI- und Render-Thread fassen nie ein QVideoFrame an. `bildNummer()` ist der
billige Vorab-Check des Render-Threads, `letztesBild()` die COW-Bild-Kopie.
Ein `weak_ptr` aufs Feed verhindert Zugriffe nach `stopp()`.
`baueKamera` wählt zudem ein Format um 720p/≤30 fps (Wandlungskosten
skalieren mit der Pixelzahl). Dieselbe Befund-Klasse steckte im
VideoFrameCache: dessen Voll-Decode lief auf dem Main-Thread (Seek je Frame
+ verschachtelte Event-Loops) — seit dessen 1.2.0 auf einem eigenen
Decoder-Thread.

## 4a. Abschalt-Vertrag (Befund S71) — die Reihenfolge ist alles

Ein Feed wird **nie** gestoppt oder zerstört, solange der Wandler-Thread noch
eines seiner Frames anfasst. `feedStilllegen()` ist die einzige erlaubte
Reihenfolge und die einzige Stelle, die Feeds abbaut:

1. **Totflagge** `Feed::tot` — der Wandler lässt ab jetzt jedes Frame dieses
   Feeds liegen (Prüfung VOR dem `map()`), der Puffer geht sofort zurück.
2. **Senke abklemmen** (`senke->disconnect()`) — es kommt nichts mehr nach.
3. **`wandlerBarriere()`** — ein leeres Lambda reiht sich hinter die schon
   zugestellten Frame-Events; läuft es, sind alle geliehenen Puffer zurück.
   Mit Timeout (2 s) plus Warnung, damit ein blockierter Wandler den Abbau
   nur verzögert statt ihn zu verklemmen.
4. Erst **jetzt** `stop()` und Zerstörung der Qt-Multimedia-Objekte.

**Warum:** Ohne die Schritte 1–3 mappte der Wandler ein `QVideoFrame` einer
bereits sterbenden Media-Foundation-Pipeline, blockierte im Grafiktreiber und
hielt deren Puffer für immer — die `nvwgf2umx`-D3D-Worker starben nie, die App
ließ sich nicht mehr schließen. **Log-Beweis:** das `wait(5000)` des Wandlers
lief in *jedem* Lauf mit Feed exakt in den 5-Sekunden-Timeout (`Stopping video
feeds...` → nächste Zeile). Der generelle Vertrag steht in
[Bootstrap_Integration.md §6](../../docs/core-services/Bootstrap_Integration.md).

`altenFeedRaeumen()` schließt die Lücke, an der der Vertrag zuerst brach:
`m_feeds[nodeId] = neu` zerstörte den Vorgänger als **Nebenwirkung der
Zuweisung** — unter dem Mutex und ohne Vertrag. Jeder `baue*`-Pfad räumt den
Vorgänger deshalb zuerst und ohne Mutex ab.

**Dazu gehört zwingend der BAU-RIEGEL `m_imBau`** (ein Bauauftrag je Knoten
und Zielquelle). Der Render-Thread ruft `starte*` in **jedem Frame**; bis der
gequeuete Bau auf dem Main-Thread durch ist — und der Start einer
Kamera-Pipeline dauert — steht der Feed noch nicht in `m_feeds`. Ohne den
Riegel queut jedes Frame einen weiteren Bauauftrag, und jeder davon räumt über
`altenFeedRaeumen()` die eben gebaute Kamera wieder ab: eine Endlosschleife aus
Auf- und Abbau, die den Main-Thread blockiert und die Kamera **nie hochkommen
lässt** (Befund S71 — die App fror nach dem Freigabe-Dialog ein). Ausgetragen
wird der Auftrag an jedem Ausgang von `baueDatei`/`baueKamera`.

`herunterfahren()` (Aufruf aus `Application::shutdown()`, nicht per
aboutToQuit — Vertrag 6.1) hält dieselbe Ordnung: `alleStoppen()` →
Wandler-Thread beenden → Kontext erst **nach** dem Thread-Ende direkt löschen
(ein `deleteLater()` wäre ein Event an einen Thread ohne Event-Loop und würde
nie ausgeführt). Der aboutToQuit-Haken setzt nur noch den Riegel `m_beendet`.

**Das Beenden des Wandler-Threads braucht eine Schleife** (Befund S71,
Vertrag 6.4.4): ein einzelnes `quit()` direkt nach dem Feed-Abbau verpufft,
weil der Thread noch interne Ereignisse der sterbenden Multimedia-Pipeline
abarbeitet — und das trotz laufender Event-Loop und ohne ein einziges Frame
in Arbeit. Gemessen: `quit(); wait(5000)` scheiterte 3/3, wiederholtes
`quit()` + `wait(200)` greift im zweiten Versuch (~200 ms, 4/4).

## 5. Befund S70: QVideoFrame::toImage() liefert schwarz

Unter Qt 6.10.1 (FFmpeg-Backend, Windows) liefert `toImage()` schwarze
Bilder bei korrekten Zeitstempeln — die Rohdaten sind per `map()` vollständig
da. `VideoFrameUtil.hpp::videoFrameZuBild()` baut das QImage deshalb selbst
aus den gemappten Bytes (RGB-Formate; YUV fällt auf toImage zurück). Gilt
für LiveVideoFeed UND VideoFrameCache (dessen 1.1.0 zusätzlich `Clip::fps`
für das Original-Tempo des Frame-Schritts trägt).

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.3.0 | 2026-08-07 | **Teardown-Deadlock behoben (S71, am Standalone mit echter Kamera gemessen):** NEU Abschalt-Vertrag §4a — `Feed::tot` (Wandler-Prüfung vor `map()`), `feedStilllegen()` als einzige Abbau-Stelle, `wandlerBarriere()` mit Timeout (misst 2–4 ms), NEU `altenFeedRaeumen()` gegen den Vertragsbruch bei `m_feeds[id] = neu`; **Wandler-Thread-Ende in der Schleife** (einzelnes `quit()` verpufft: 3/3 Timeout gegen 4/4 Erfolg); `herunterfahren()` löscht den Kontext erst nach dem Thread-Ende (deleteLater lief dort nie); `kameraGelaufen()` → `feedGelaufen()` (Datei-Feeds hingen genauso) + `merkeFremdenFeed()` für die Settings-Testaufnahme. Feed-Teardown 5320 ms → **200 ms** |
| 1.2.0 | 2026-08-06 | Endgültige Lag-Architektur: Wandler-Thread mit queued Frame-Zustellung (Render-Thread-Wandlung hing bei Hardware-Frames); Kamera-Format 720p/≤30fps; aboutToQuit beendet den Wandler geordnet |
| 1.1.0 | 2026-08-06 | Lag-Befund S70: NEU `bildNummer()` als billiger Vorab-Check (Zwischenstand, Wandlung auf dem Render-Thread — ersetzt durch 1.2.0) |
| 1.0.0 | 2026-08-06 | Session 70 — Erstfassung (Datei-Streaming + Kamera + Testaufnahmen-Ordner, Kamera-Vertrag, toImage-Workaround) |
