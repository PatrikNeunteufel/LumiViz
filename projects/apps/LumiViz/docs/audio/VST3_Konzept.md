# VST3 in LumiViz — Konzept

> Version 0.1.0 · Session 123 (2026-10-14) · Stand: **Entwurf, nichts entschieden, nichts gebaut**
> Auftrag Patrik S123: „Konzept zum Einbinden von VST (Version 3?)".

## 1. Ziel

Fremde Klangerzeuger und Effekte im VST3-Format in LumiViz nutzen: im Loop
(Instrument-Schichten, Drum-Spuren, Effekte am Clip) und vielleicht im
Composer. Der Nutzer wählt ein installiertes Plug-in, stellt es in dessen
eigenem Fenster ein, und LumiViz speichert diese Einstellung in seinen Dateien.

Nicht Ziel: LumiViz selbst als Plug-in für andere Programme (das wäre die
andere Richtung, ein eigenes Projekt).

## 2. Belege — Lizenz und Marke

| Angabe | Quelle | Reichweite |
|---|---|---|
| **Seit VST 3.8 steht das SDK unter der MIT-Lizenz** — „Mit Veröffentlichung des VST 3.8 SDK wechselt Steinberg zur MIT Open Source License" | Pressemitteilung Steinberg, **29. Oktober 2025**, PDF von steinberg.net, Wortlaut mit `pdftotext` gelesen | eine Quelle, Erstveröffentlichung des Herstellers |
| Vorher galt ein duales Modell (GPLv3 oder proprietäre Steinberg-Lizenz); laut README des SDK auf GitHub sind beide „no longer available" | `github.com/steinbergmedia/vst3sdk`, README, abgerufen S123 (Abrufwerkzeug — Wortlaut der README nicht selbst gesehen) | zur Gegenprüfung vor dem Bau selbst lesen |
| MIT verlangt nur: Copyright-Vermerk und Lizenztext bleiben bei jeder Weitergabe dabei | Pressemitteilung (wie oben) | — |
| **Name und Logo „VST" sind eine Marke**; ihre Nutzung ist freiwillig und folgt dann den „VST Usage Guidelines", die dem SDK beiliegen | Suchmaschinen-Zusammenfassung — **kein Beleg**, die Guidelines selbst nicht gelesen | vor jeder Nennung von „VST" in der Oberfläche oder im Handbuch die Guidelines im SDK lesen |
| ASIO (nicht Teil dieses Konzepts) steht unter GPLv3 | Pressemitteilung (wie oben) | LumiViz nutzt BASS, nicht ASIO |

Folge: LumiViz (MIT oder Apache-2.0) kann das VST3 SDK einbinden, ohne seine
eigene Lizenz zu ändern; `THIRD_PARTY_NOTICES.md` bekommt einen Abschnitt.
VST2 bleibt draußen — es hat einen eigenen, nicht mehr vergebenen Vertrag.

## 3. Was es in LumiViz heute gibt — wo ein Plug-in andocken müsste

- **Töne eines Instruments werden vorab gerechnet.** `ladeKlaenge`
  (`loop/Kit.hpp`) rechnet für jede Note, die ein Loop spielt, einmal einen
  `Klang` (`rendereTon`, `inst:<name>:<hoehe>:<ms>`). Der Ton-Thread spielt
  danach nur noch fertige Abtastwerte ab. Das ist der Grund, warum ein Loop
  Bit für Bit reproduzierbar ist (`test_LoopWerte`).
- **Die Effekte am Clip laufen im Ton-Thread.** Der `Klangerzeuger` hängt als
  `IAudioGenerator` im Mixer der BassEngine und rechnet je Block die Reihe
  eines Clips (Filter, Distortion, Echo, Reverb; Blöcke von 1024, `kEffektBlock`).
- **Die Reihe einer Schicht** (Filter, Drive, Envelope) wird mit dem Ton vorab
  gerechnet — sie gehört zum Instrument, nicht zum Clip.

Ein VST3-Plug-in passt in beide Welten, aber verschieden gut:

| Andockstelle | Was das Plug-in dort tut | Passt? |
|---|---|---|
| **A — Baustein in der Reihe eines Clips** | ein VST3-**Effekt** bearbeitet den Ton eines Clips im Ton-Thread, wie Echo und Reverb heute | gut: dieselbe Stelle, dieselbe Blockgröße; das Plug-in rechnet live |
| **B — Quelle einer Schicht** (Waveform „Plug-in") | ein VST3-**Instrument** erzeugt den Ton einer Note | zwei Wege, siehe §4 |
| **C — Quelle einer Drum-Spur** | wie B, Note und Länge wie bei einer Instrument-Spur | folgt aus B |
| **D — Effekt auf einem Composer-Kanal oder dem Ausgang** | ein Effekt auf allem, was ein Kanal spielt | später; braucht dieselbe Technik wie A an anderer Stelle |

## 4. Die Kernfrage: ein Instrument-Plug-in vorab rechnen oder live spielen?

**Weg B1 — vorab rechnen (offline, je Note):** LumiViz rechnet für jede Note
eines Loops einen Klang, wie heute. Für eine Plug-in-Schicht hieße das: eine
Instanz des Plug-ins im Hintergrund, Note an, so lange `process` rufen, wie die
Note plus Ausklang dauert, Note aus — das Ergebnis ist ein `Klang` wie jeder
andere. Vorteil: der ganze Rest bleibt, wie er ist (Mixer, Clips, Effekte,
Composer, Export). Nachteil: viele Synthesizer sind nicht reproduzierbar (freie
LFO-Phasen, Zufall, Sample-Streaming) — zwei Rechnungen derselben Note klingen
verschieden; Noten, die ineinander klingen (Legato, Hall im Plug-in), werden
abgeschnitten, weil jede Note für sich gerechnet wird; das Vorabrechnen dauert
bei vielen Noten.

**Weg B2 — live spielen:** eine Plug-in-Schicht ist eine Spur im Klangerzeuger,
die im Ton-Thread Noten an das Plug-in schickt und seinen Ton abholt. Vorteil:
klingt, wie das Plug-in klingen soll (Legato, gemeinsame Hallfahne, Polyphonie
im Plug-in). Nachteil: ein neuer Weg durch den Klangerzeuger (Ereignisse statt
fertiger Klänge), und das Plug-in muss in jedem Block rechtzeitig fertig sein.

Vorschlag zur Vorlage (V2 unten): **B1 zuerst**, weil es in das Bestehende passt
und dieselbe Infrastruktur (Laden, Fenster, Zustand) aufbaut, die B2 und A
brauchen — B2 als spätere Stufe.

## 5. Technik

### 5.1 Laden und Suchen

- Ein VST3-Plug-in ist ein Bündel `<Name>.vst3` (unter Windows ein Ordner mit
  der DLL darin, ältere als einzelne DLL). Üblicher Ort unter Windows:
  `C:\Program Files\Common Files\VST3`. LumiViz sucht dort und in eigenen
  Suchordnern (Vorbild: `core/Suchordner.hpp`, die Liste für Bilder).
- Laden über das SDK (`VST3::Hosting::Module::create`), daraus die
  Klassen-Fabrik; je Klasse Name, Hersteller, Kategorie (Instrument oder
  Effekt) und eine feste **Klassen-ID**. Die ID ist das, was eine Datei nennt —
  nicht der Pfad.
- Eine Liste der gefundenen Plug-ins wird gemerkt (Name, ID, Pfad, Art), damit
  nicht bei jedem Start alles geladen wird.

### 5.2 Ablauf eines Plug-ins (aus der API-Dokumentation des SDK)

Wortlaut aus „VST 3 API Documentation" (steinbergmedia.github.io, abgerufen
S123 mit einem Abrufwerkzeug — vor dem Bau im SDK selbst gegenlesen):

- Initialisieren: „Hosts should not call other functions before initialize is
  called".
- Vor dem Rechnen: `setupProcessing` („The processor must be configured before
  processing can start"), dann `setActive` („all configurations have been
  finished"), dann `setProcessing(true)`; Ende in umgekehrter Folge.
- **Threads:** „All initialisation/de-initialisation are done in the UI
  Thread"; `process` und `setProcessing` laufen im Ton-Thread — dort „avoid any
  memory allocation!".
- **Zustand:** „The host stores and restores the complete state of the
  processor and of the controller" — über `IComponent::getState/setState` und
  `IEditController::getState/setState`; beim Laden „A host must always pass
  that state to the processor first."

Für LumiViz heißt das: Laden, Aktivieren und Speichern im Hauptthread (wie die
übrigen Dateien), `process` im Ton-Thread des Klangerzeugers (Weg A und B2)
oder in einem Arbeitsthread (Weg B1).

### 5.3 Das Fenster des Plug-ins

Ein Plug-in bringt seine eigene Oberfläche (`IPlugView`). LumiViz öffnet dafür
ein eigenes, nicht andockendes Fenster und gibt dem Plug-in dessen
Fenster-Handle (`QWidget::winId()`, unter Windows ein `HWND`). Größe: das
Plug-in meldet sie, LumiViz passt das Fenster an (`IPlugFrame::resizeView`).
Änderungen meldet das Plug-in über `IComponentHandler` — das ist der Haken für
**Undo** (ein Schritt „Plug-in settings" je losgelassener Änderung, über den
CommandBus) und dafür, dass die Datei „geändert" wird.

### 5.4 In den Dateien

- `.lvinst`: eine Schicht der Waveform `plugin` trägt Klassen-ID, Name und
  Hersteller (für die Meldung, wenn es fehlt) und den **Zustand** als Base64
  (`getState` beider Teile). Neue Fassung der `.lvinst`.
- `.lvloop`: ein Baustein `plugin` in der Reihe eines Clips, ebenso.
- **Fehlt ein Plug-in** auf einem anderen Rechner: die Schicht schweigt, der
  Bericht sagt „Plug-in not found: <Name> (<Hersteller>)" — wie heute bei einem
  fehlenden Sample. Die Datei bleibt, wie sie ist.
- Datensicherung: Plug-ins gehen nicht mit (fremde Programme); ihr Zustand
  steht ja in der Datei.

### 5.5 Abstürze

Ein Plug-in läuft im Prozess von LumiViz; stürzt es ab, stürzt LumiViz mit.
Zwei Wege:

- **im Prozess** (einfach): beim Laden und Scannen absichern, ein Plug-in, das
  beim Scannen abstürzt, beim nächsten Start auf eine schwarze Liste setzen
  (wie es die meisten Programme tun).
- **eigener Prozess** (sicher, aufwendig): ein kleiner Plug-in-Wirt als eigene
  Exe, Ton über geteilten Speicher. Das kostet Latenz und viel Bau.

Vorschlag zur Vorlage (V4): im Prozess mit schwarzer Liste.

### 5.6 Bauen

- Das SDK als External über CMakeCraft (gefetcht nach `.externals/`, Fassung im
  Pin, wie miniz lokal oder Qt gefetcht), nur die Teile für einen Host
  (`pluginterfaces`, `base`, `public.sdk/source/vst/hosting`). VSTGUI wird nicht
  gebraucht — das Plug-in bringt seine Oberfläche selbst mit.
- Zum Testen ohne fremde Plug-ins: die Beispiel-Plug-ins des SDK (etwa „AGain",
  „Note Expression Synth") als Test-Fixtures bauen; der `validator` des SDK
  prüft einen Host nicht, aber ein Plug-in — er hilft beim Prüfen der Fixtures.
- Die Unit-Tests rechnen dann einen Ton mit einem Beispiel-Plug-in (Weg B1)
  und einen Clip durch einen Beispiel-Effekt (Weg A). `test_LoopWerte` hält
  Plug-in-Töne **nicht** fest (nicht reproduzierbar, siehe §4).

## 6. Alternativen, die vorzulegen sind

- **CLAP** (CLever Audio Plug-in, MIT, von Bitwig und u-he): schlanker, für
  Hosts einfacher, wächst — aber weniger verbreitet als VST3. Das Konzept
  (§3–5) gilt für CLAP fast unverändert; ein zweites Format wäre eine
  Übersetzungsschicht mehr.
- **Nur Effekte, keine Instrumente** (nur Weg A): kleinster Schritt, zeigt die
  ganze Infrastruktur (Suchen, Fenster, Zustand), lässt §4 offen.

## 7. Offen — einzeln vorzulegen, in dieser Reihenfolge

1. **V1 — Format:** nur VST3; VST3 und später CLAP; nur CLAP.
2. **V2 — Instrumente:** B1 (vorab je Note) zuerst, B2 (live) später; gleich B2;
   keine Instrumente, nur Effekte (A).
3. **V3 — Reihenfolge der Stufen:** Effekte am Clip (A) vor den Instrumenten,
   oder umgekehrt.
4. **V4 — Absturzschutz:** im Prozess mit schwarzer Liste; eigener Prozess.
5. **V5 — Name in der Oberfläche:** „Plug-in" neutral, oder „VST3" (dann gelten
   die Usage Guidelines des SDK, vorher lesen).
6. **Skizzen** (Regel: Skizze vor Modell): die Plug-in-Liste (Suchen, Kategorie,
   fehlende), die Schicht mit Waveform „Plug-in" im Creator, der Baustein im
   Fenster Clip effects.

## 8. Stufen als Vorschlag (erst nach V1 bis V5)

| Stufe | Inhalt | Beleg |
|---|---|---|
| P1 | SDK als External, ein Beleg-Fall: ein Beispiel-Plug-in laden, initialisieren, einen Block rechnen, Zustand hin und zurück | Unit-Test mit dem Beispiel aus dem SDK |
| P2 | Suchen und Merken der Plug-ins, schwarze Liste, die Liste im Programm | Test mit Wegwerf-Ordner und einem absichtlich defekten Bündel |
| P3 | Fenster des Plug-ins, Änderungen als Undo-Schritt | Bild aus dem Test, Sichttest |
| P4 | Weg A: Baustein `plugin` in der Reihe eines Clips, `.lvloop` neue Fassung | Klangerzeuger mit Beispiel-Effekt, Last gemessen |
| P5 | Weg B1: Waveform `plugin` an einer Schicht, `.lvinst` neue Fassung | Ton mit Beispiel-Synth, Länge und Ende |
| P6 | Handbuch, Sichttest-Liste | — |

## 9. Changelog

- **0.1.0** (S123, 2026-10-14): erster Entwurf auf Patriks Auftrag. Lizenz an
  der Pressemitteilung belegt, Ablauf und Threads aus der API-Dokumentation
  (Abrufwerkzeug, zum Gegenlesen markiert), Marke nur aus einer
  Zusammenfassung — offen. Nichts entschieden.
