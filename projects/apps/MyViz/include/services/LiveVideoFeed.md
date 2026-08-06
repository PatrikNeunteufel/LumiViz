# LiveVideoFeed — Live-Videoquellen je Chain-Node (Datei-Streaming + Kamera)

> **Version:** 1.0.0
> **Datum:** 2026-08-06
> **Typ:** CppModuleDoc
> **Status:** Implementiert (videoSource-Strang, Session 70)
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
  dem Main-Thread).
- `letztesBild(nodeId, &nummer)` — Mutex-Kopie (QImage-COW) + Bild-Nummer.
- `testaufnahmenOrdner()` — benutzerlokaler Ablageordner der Kamera-Clips
  (`AppDataLocation/testaufnahmen`, wird angelegt) — bewusst AUSSERHALB des
  Repos (Kameramaterial ist persönlich).

## 4. Threading

`starteDatei`/`starteKamera`/`stopp` sind render-thread-tauglich: sie prüfen
unter Mutex den Quellen-String und queuen den (Um-)Bau als Lambda-Invoke auf
den Main-Thread. Der Sink-Anschluss (`verbindeSenke`) wandelt jedes
ankommende Bild über `videoFrameZuBild()` (VideoFrameUtil.hpp — der
toImage-Ersatz, Befund S70) und legt es unter dem Dienst-Mutex ab; ein
`weak_ptr` aufs Feed verhindert Zugriffe nach `stopp()`.

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
| 1.0.0 | 2026-08-06 | Session 70 — Erstfassung (Datei-Streaming + Kamera + Testaufnahmen-Ordner, Kamera-Vertrag, toImage-Workaround) |
