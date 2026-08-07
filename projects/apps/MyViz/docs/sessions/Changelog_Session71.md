# Changelog Session 71 (2026-08-07)

> **🎯 Der Kamera-Teardown-Deadlock aus S70 ist gelöst und abgenommen** —
> Ursache war der eigene Thread für die Multimedia-Seite, nicht der Treiber.
> Dazu ein Lebenszyklus-Regelwerk und der Einstieg in den Filter-Ausbau.
> Tests 563 grün, alle 3 Builds grün.

## Teil 1 — Kamera-Teardown: Ursache gefunden

**S70 hatte es als „treiberintern, in-Prozess nicht heilbar" eingestuft und
mit `std::_Exit(0)` abgefangen. Beides war falsch.**

- **Ursache: ein eigener `QThread`, der Kamera-Frames verarbeitet hat,
  verhindert das Prozessende** — auch wenn er vorher sauber beendet wurde.
  A/B am Prüfstand mit echter Kamera (Schalter `LUMIVIZ_MEDIEN_THREAD`
  isoliert genau diese eine Variable): **MIT Thread 2/2 Hänger, OHNE 3/3
  sauberes Ende**. Unabhängig vom Media-Backend (FFmpeg wie WMF), von der
  Lage der Pipeline und von COM-Init auf dem Thread.
  → `LiveVideoFeed` arbeitet wieder auf dem **Main-Thread** (Stand `f93c83a`);
  `LUMIVIZ_MEDIEN_THREAD=1` schaltet den Worker-Thread für Messungen ein.
- **`std::_Exit()` beendet den Prozess NICHT**, wenn ein Thread im
  Grafiktreiber steckt. Die Logzeile „erzwungenes Prozessende" täuschte seit
  S70 einen Erfolg vor, während die App weiterhing. Notausgang **entfernt**.
- **Kein UI-Lag mehr**, obwohl die RGBX-Wandlung wieder auf dem Main-Thread
  läuft: die 720p/30fps-Formatklemme aus S70 reicht aus. Der Zielkonflikt
  „Lag gegen Prozessende" ist damit aufgelöst statt abgewogen.

**Auf dem Weg gefunden und behoben (bleibt, weil für sich richtig):**

- **Abschalt-Vertrag** `feedStilllegen()` als einzige Abbau-Stelle: Totflagge
  → Senke abklemmen → Barriere → erst dann stop/zerstören.
- **`altenFeedRaeumen()`** — `m_feeds[id] = neu` zerstörte den Vorgänger als
  Nebenwirkung der Zuweisung, unter dem Mutex und ohne Vertrag.
- **Bau-Riegel `m_imBau`** — der Render-Thread ruft `starte*` in JEDEM Frame;
  ohne Riegel räumte jeder neue Auftrag die eben gebaute Kamera wieder ab
  (die App fror nach dem Freigabe-Dialog ein, die Kamera lief nie an).
- **Wandler-Thread-Ende in der Schleife** — ein einzelnes `quit()` direkt nach
  dem Feed-Abbau geht verloren (3/3 Timeout gegen 4/4 Erfolg).
- Settings-**Testaufnahme** war eine zweite, unbewachte MF-Pipeline: meldet
  sich jetzt an und wird bei `aboutToQuit` synchron abgebaut.
- `VideoFrameCache`: `herunterfahren()` statt aboutToQuit-Lambda;
  `abbrechen()` wirkt auch in der verschachtelten Event-Loop.

**Messwerte:** Feed-Teardown **5320 ms → 200 ms** (stabil über 4 Läufe),
App-Schließen ohne Kamera 671 ms. **Sichttest Patrik ✅:** Dialog → Kamera
läuft → Preset-Wechsel → Schließen sauber, kein Prozess bleibt zurück.

## Teil 2 — Lebenszyklus-Vertrag (Wunsch Patrik)

`docs/core-services/Bootstrap_Integration.md` **1.3.0**: §5 zeigt die
vollständige Shutdown-Kette inkl. aller `aboutToQuit`-Handler, NEU §6:

- **6.1** Abbau zentral aus `Application::shutdown()` statt verstreuter
  aboutToQuit-Lambdas (deren Reihenfolge hängt bei lazy Singletons vom
  Nutzerverhalten ab)
- **6.2** Abschalt-Vertrag für Producer über Thread-Grenzen
- **6.3** fremde Puffer sind geliehen
- **6.4** Qt-Fallen · **6.4a** Abbau nie im Renderpfad auslösen
  (Auftrags-Riegel) · **6.4.4** Joins wiederholen · **6.4.5** `_Exit` ist kein
  Notausgang
- **6.5** Messen statt vermuten · **6.6** kein eigener Thread für fremde
  Medien-Pipelines

## Teil 3 — Filter-Strang eröffnet

**Shader-Import gegen Vertrags-Verwechslung gesichert** (Einwand Patrik: „muss
man nicht aufpassen, dass man nicht Shadertoy im pixelFilter importiert?").
Ist-Stand war ungeschützt — ein gemeinsames Ordner-Gedächtnis für alle
Shader-Felder, ein Filter für alle, keine Prüfung.

- NEU SSOT **`ShaderVertrag`** (key · Anzeige · Einstiegsfunktion · Signatur ·
  echte Endung) für pixelfilter/shadertoy/meshwarp/gpuparticles/milkdrop.
- **Namensschema `<preset>[.<slot>].<vertrag>.<endung>`** — Klassifikation
  von rechts nach links immer spezifischer (Entscheid Patrik). MilkDrop-Felder
  exportieren jetzt `.hlsl`, weil sie HLSL sind.
- **Ordner-Gedächtnis je Vertrag** + Dateifilter „Passende Shader" zuerst
  (`.fs`/`.vs` für ISF und `.vert` aufgenommen; `.gs` bewusst nicht).
- **Vertragsprüfung beim Import:** Klartext-Warnung mit Hinweis auf den
  zuständigen Knoten, „Trotzdem laden" bleibt möglich (Fragmente).
- **Export** schlägt einen freien Namen vor (`preset(2).…`, Zähler vor den
  Endungen).

**NEU Steuerdokument** `docs/visuals/ISF_Import_Parameterbaum_Plan.md` 1.0.0 —
vier Stufen: ISF-Parser (pur/testbar) · generische Parameter-Ablage +
Herkunft/Lizenz · **Parameter-Baum im Panel** (Entwurf Patrik: Aufbau wie die
Effect-Chain, aber mit Wert-Spalte und typsicheren Editoren) · Durchziehen auf
**alle** Module (Idee Patrik), mit dem offenen Entscheid „Ablösung vs. zweiter
Weg". ISF-Format gegen die Spec geprüft: kein separates JSON, der Kopf steckt
im führenden Blockkommentar der `.fs`; FX-Erkennung über `inputImage`.

**🔴 Lizenz-Kette** (Vorgabe Patrik, im Loader statt nur in der Doku): jeder
Fremd-Import schreibt Herkunft/Autor/Lizenz ins Preset. Befund dabei: die
Felder existieren nur an `ShadertoyParams`, und der Shader-**Export** schreibt
sie nicht mit — die Herkunft geht beim Export verloren.

## Doku

Bootstrap_Integration 1.3.0 · LiveVideoFeed.md 1.4.0 · VideoFrameCache.hpp
1.3.0 · **Benutzerhandbuch 1.9.0** (NEU §14 „Filter als Datei laden und
weitergeben" — schließt eine Lücke seit S69: Import/Export war nie
beschrieben) · MultiEffectPanel.md · Application.md · INDEX.md ·
NEU `visuals/ISF_Import_Parameterbaum_Plan.md` · Offene_Punkte **1.66.0**.
