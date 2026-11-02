# Recherche: Plug-in-Formate VST3 und CLAP — Hosting, Hüllen, Offline, Determinismus

- **Datum:** 2026-11-02
- **Sitzung:** LumiViz S172 (Recherche-Agent)
- **Auftrag:** Grundlage für den Schnitt der Schnittstelle zu den Rechenkernen aus dem
  Schwesterprojekt (InstrumentsCalculation): Kerne als Bibliotheken, spätere Plug-in-Hüllen
  (VST3/CLAP) um die eigenen Bibliotheken, später ein Host für fremde Plug-ins. Sieben Fragen
  (unten je ein Abschnitt). Bereits belegt in `docs/audio/VST3_Konzept.md` (S123) und hier nicht
  wiederholt: Pressemitteilung MIT vom 29.10.2025, Ablauf `initialize` → `setupProcessing` →
  `setActive` → `setProcessing`, Threads, Zustand, `IPlugView`, Suchpfad, Wege B1/B2.
- **Werkzeuge:** `curl` (Rohdateien von `raw.githubusercontent.com`, GitHub-REST-API
  `api.github.com` ohne Anmeldung, HTML-Seiten), `pdftotext` und
  `~/.claude/tools/quelle_suchen.py` für zwei PDFs, WebFetch, WebSearch. Kein Browser.

## Reichweite der Suche

| Ort | erreicht? | Art des Lesens |
|---|---|---|
| `raw.githubusercontent.com` (vst3sdk, vst3_pluginterfaces, vst3_public_sdk, vstgui, clap, clap-host, clap-helpers, clap-wrapper, clap-plugins, JUCE, Carla, iPlug2, DPF, nih-plug, pluginval, easyvst) | ja | **Wortlaut selbst gelesen** (curl, Rohtext) |
| `api.github.com` (Bäume, Tags, Commits, Metadaten, Sprachen) | ja | **Wortlaut** der API-Antwort (JSON) |
| `steinbergmedia.github.io/vst3_dev_portal` (FAQ Processing, API Documentation) | ja | **Wortlaut** (curl, HTML entmarkiert) |
| `VST3_Usage_Guidelines.pdf` (im vst3sdk-Repo) | ja | **Wortlaut** mit `pdftotext`; das Zeichen nach „VST" in §15 kam als Ersatzzeichen an (vermutlich ®, nicht gesehen) |
| `ableton.com` Live-12-Handbuch Kap. 38 | ja | **Wortlaut** (curl, HTML entmarkiert) |
| `bitwig.com/userguide/latest` (Bouncing to Audio, Plug-in Handling and Options) | ja | **Wortlaut** (curl) |
| `reaper.fm` — „Up and Running: A REAPER User Guide v 7.84" (PDF, 462 Seiten, Link von `reaper.fm/userguide.php`) | ja | **Wortlaut** mit `pdftotext`/`quelle_suchen.py` |
| `juce.com/get-juce` | ja | nur WebFetch — **vom Werkzeug als Zitat geliefert**, nicht selbst gesehen |
| `steinberg.help` / `archive.steinberg.help` (Cubase-Handbuch) | **gesperrt**: curl „Request forbidden by administrative rules", WebFetch HTTP 403; die neue Leseansicht `steinberg.help/r/...` liefert nur eine JS-Hülle ohne Text | nur **Suchmaschinen-Zusammenfassung — kein Beleg** |
| `helpcenter.steinberg.de` (ASIO-Guard-Artikel 206103564) | **gesperrt**: curl leer, WebFetch HTTP 403 | nur Suchmaschinen-Zusammenfassung — kein Beleg |

Keine Sperre und keine Bot-Prüfung wurde umgangen (kein anderer User-Agent für die gesperrten
Steinberg-Seiten, keine Spiegel).

Stand der geprüften Fassungen: VST3 SDK **3.8.1** (Tag `v3.8.1_build_84`, Commit vom
2026-08-11; Untermodule `public.sdk` @ `586dc5e`, `pluginterfaces` @ `4f547e8`); CLAP **1.2.10**
(Tag `1.2.10`, `include/clap/version.h`, letzter Commit auf `main` 2026-07-28).

---

## 1. VST3 — Gegenprüfung

| Angabe | Quelle | Reichweite |
|---|---|---|
| Aktuelle Fassung **3.8.1**; README-Titel „Welcome to VST SDK 3.8.x" | Tags über `api.github.com/repos/steinbergmedia/vst3sdk/tags`; Commit-Nachricht „VST3 SDK 3.8.1" vom 2026-08-11; README | Wortlaut (API, Rohdatei) |
| Tag 3.8.0 (`v3.8.0_build_66`) ist vom 2025-10-20 | `api.github.com/.../commits/v3.8.0_build_66` | Wortlaut (API) |
| **Lizenz heute MIT**: `LICENSE.txt` beginnt mit „MIT License", „Copyright (c) 2026, Steinberg Media Technologies GmbH"; README §License: „VST 3 SDK is under MIT license" | `https://raw.githubusercontent.com/steinbergmedia/vst3sdk/master/LICENSE.txt`, `.../README.md` | **Wortlaut selbst gelesen** |
| Ältere Lizenzen: README „Licensing under GPLv3 and the Steinberg proprietary license is no longer available." Bis 3.7.9 war das SDK dual lizenziert — `LICENSE.txt` am Tag `v3.7.9_build_61`: Steinberg VST3 License „or alternatively" GPL Version 3 | README (master); `.../vst3sdk/v3.7.9_build_61/LICENSE.txt` | **Wortlaut selbst gelesen** |
| Die Untermodule tragen eigene Lizenzdateien: `vst3_public_sdk` @ `586dc5e` MIT (Copyright 2026); **`vstgui` trägt eine eigene BSD-artige Lizenz** („Redistribution and use in source and binary forms …") | `vst3_public_sdk/.../LICENSE.txt`, `vstgui/.../LICENSE` | Wortlaut; vstgui für LumiViz ohnehin nicht nötig (VST3_Konzept §5.6) |
| Das Repo enthält „VST 3 API", „Implementation Helper Classes", „AAX, AUv3 and AU Wrappers", „plug-ins Examples"; das **volle SDK** von steinberg.net enthält zusätzlich die „Test Host Application/Validator" | README §„The VST SDK package contains" | Wortlaut |
| **Hosting-Klassen vorhanden** unter `public.sdk/source/vst/hosting/`: `connectionproxy`, `eventlist`, `hostclasses`, `hostdataexchangehandler`, `module` (+ `module_win32.cpp`, `module_mac.mm`, `module_linux.cpp`), `parameterchanges`, `pluginterfacesupport`, `plugprovider`, `processdata`, dazu `test/` mit sieben Testdateien (u. a. `eventlisttest.cpp`, `processdatatest.cpp`, `miditovsttest.cpp`) | Baum `api.github.com/repos/steinbergmedia/vst3_public_sdk/git/trees/586dc5e…?recursive=1` (1025 Einträge, nicht abgeschnitten) | Wortlaut (API) |
| Umfang der Hosting-Klassen (Zeilen, `wc -l`): `module.cpp` 327, `module_win32.cpp` 746, `module.h` 196, `plugprovider.cpp` 321, `hostclasses.cpp` 319, `parameterchanges.cpp` 298, `processdata.cpp` 204, `hostdataexchangehandler.cpp` 390, `pluginterfacesupport.cpp` 121, `eventlist.cpp` 86, `connectionproxy.cpp` 91 — zusammen gut 3000 Zeilen | Rohdateien am Commit `586dc5e` | gezählt |
| **Beispiel-Hosts** unter `public.sdk/samples/vst-hosting/`: `audiohost` (mit `media/audioclient`, `jack/jackclient.cpp`, `miditovst.h`), `editorhost` (Plattformschicht `win32/`, `mac/`, `linux/` inkl. `wayland/`), `inspectorapp`, `validator` (`main.cpp`, `validator.cpp`, `testsuite.h`) | derselbe Baum | Wortlaut (API) |
| Einstiegssymbole einer VST3-DLL unter Windows, wie der Host sie sucht: `GetPluginFactory`, `InitDll`, `ExitDll` | `module_win32.cpp` Z. 145, 218, 225 | Wortlaut |
| **Marke — README:** Nutzung von Name/Logo „VST" ist unter MIT optional, wenn genutzt, gelten die Regeln; verboten: „VST" (oder „VSTi") in Firmen- oder Produktnamen; das Logo nur unverändert und nur in produktbezogenem Zusammenhang (Splash, About, Website, Doku/Handbuch). Nichteinhaltung berühre die MIT-Rechte nicht, könne aber Markenrecht verletzen | README §„Trademark and Logo Usage" | **Wortlaut selbst gelesen** |
| **Marke — Usage Guidelines** (`VST3_Usage_Guidelines.pdf` im Repo-Wurzelordner, gültig ab SDK „3.8 – 16.10.2025"): wer das Logo zeigt **oder auf die Marke „VST" verweist**, muss das „VST Compatible Logo" zeigen — u. a. in **jeder Form von Produkt-Doku** (bei Platzmangel stattdessen die Zuschreibung, §15), im **About-Fenster oder Gleichwertigem** („should appear"; bei Platzmangel die Zuschreibung), auf **jeder** Webseite, die das Produkt nennt (nur Fußzeile/Impressum genügt nicht) | PDF S. 1–2, §1, §4 | **Wortlaut** (pdftotext) |
| Guidelines §2: „VST" in normaler Schrift zur Angabe der Kompatibilität ist erlaubt, aber stets mit dem Logo im Zusammenhang; §3: Aussage „Compatible with … VST" nur, wenn mit dem SDK entwickelt und Logo nahebei; §7: „VST" im Firmennamen „strictly prohibited"; §13: Abwandlungen wie „VSTi" unzulässig; §14: Mindestgrößen der Logos (20 × 18,7 mm bzw. 15 × 10 mm) | PDF S. 1–3 | Wortlaut |
| Guidelines §15: erste Nennung von „VST" in produktbezogenem Material mit Markenzeichen-Symbol; in **allen** Credits und Doku die Zeile: „VST is a registered trademark of Steinberg Media Technologies GmbH." | PDF S. 4 | Wortlaut; das Symbol kam als Ersatzzeichen an (vermutlich ®) |

**Folge für V5 (Name in der Oberfläche):** Ein neutrales „Plug-in" in Oberfläche und Handbuch
löst keine dieser Pflichten aus. Wird „VST3" genannt (auch nur „VST3 plug-ins" in einer Liste),
greifen nach §1 Logo im Handbuch und About-Fenster, §15 Zuschreibungszeile und ®.

## 2. CLAP

| Angabe | Quelle | Reichweite |
|---|---|---|
| **Lizenz MIT** („MIT License / Copyright (c) 2021 Alexandre BIQUE") | `https://raw.githubusercontent.com/free-audio/clap/main/LICENSE` | Wortlaut |
| **Fassung 1.2.10** (`CLAP_VERSION_MAJOR 1`, `MINOR 2`, `REVISION 10`); Tags 1.2.6 … 1.2.10; die GitHub-„Releases" liefern über die API keine Einträge (nur Tags) | `include/clap/version.h`; `api.github.com/repos/free-audio/clap/tags` | Wortlaut |
| ABI-Versprechen: ein mit CLAP 1.x gebautes Plug-in lädt jeder Host mit CLAP 1.y | README „Learn about CLAP" | Wortlaut |
| **Umfang: reine C-Header** — `include/clap/` 68 Header, zusammen 5323 Zeilen (gezählt); daneben nur CMake, ein Plug-in-Vorlagenbeispiel (`src/plugin-template.c`, `src/main.c/.cc`) und Artwork. Einbinden über `clap/clap.h`, mit Entwürfen `clap/all.h` | Baum `api.github.com/.../clap/git/trees/main?recursive=1`; README | Wortlaut / gezählt |
| Erweiterungen = C-Schnittstellen, je mit ID-String, getrennten Host- und Plug-in-Strukturen und **„each method must have a clear thread specification"** | README §Extensions | Wortlaut |
| Einstieg: exportiertes Symbol `clap_entry` (`clap_plugin_entry_t`, mit `init(plugin_path)`, `deinit`, `get_factory`); Fabrik `CLAP_PLUGIN_FACTORY_ID` = „clap.plugin-factory", alle Methoden `[thread-safe]` | `include/clap/entry.h` Z. 132; `factory/plugin-factory.h` | Wortlaut |
| Suchpfade Windows: `%COMMONPROGRAMFILES%\CLAP`, `%LOCALAPPDATA%\Programs\Common\CLAP`; zusätzlich muss ein Host die Umgebungsvariable `CLAP_PATH` auswerten; rekursiv nach `.clap` suchen | `entry.h` Kopfkommentar | Wortlaut |
| Lebenszyklus `clap_plugin`: `init` [main-thread] → `activate(sample_rate, min/max_frames)` [main-thread & !active] — darin darf das Plug-in Speicher holen — → `start_processing` [audio-thread] → `process` [audio-thread & active & processing] → `stop_processing` → `deactivate` → `destroy`; `reset` [audio-thread]; `get_extension` [thread-safe]; `on_main_thread` | `include/clap/plugin.h` Z. 48–109 | Wortlaut |
| **`render`** (`clap.render`): `CLAP_RENDER_REALTIME = 0` (Vorgabe, „realtime" processing) und `CLAP_RENDER_OFFLINE = 1` — „For processing without realtime pressure"; „The plugin may use more expensive algorithms for higher sound quality." Dazu `has_hard_realtime_requirement` (etwa für Hardware-Proxys) und `set(mode)`, beide [main-thread]. Wer den Modus nicht braucht, soll die Erweiterung nicht umsetzen | `include/clap/ext/render.h` | **Wortlaut selbst gelesen** |
| `thread-check`: zwei (bzw. drei) symbolische Threads — main-thread (stets derselbe OS-Thread; unter Windows der GUI-Thread), audio-thread (symbolisch, kann aus einem Pool wechseln, aber nie zwei gleichzeitig je Instanz), background-thread. Ohne `render`-Erweiterung müssen alle [audio-thread]-Funktionen Echtzeit-tauglich sein | `ext/thread-check.h` | Wortlaut |
| `note-ports` (`clap.note-ports`): Dialekte `CLAP_NOTE_DIALECT_CLAP`, `_MIDI`, `_MIDI_MPE`, `_MIDI2`; je Port `supported_dialects`, `preferred_dialect` | `ext/note-ports.h` | Wortlaut |
| `params` (`clap.params`): Werte liest der Host auf dem [main-thread]; Automation kommt als `CLAP_EVENT_PARAM_VALUE` in `process()` oder `flush()` | `ext/params.h` | Wortlaut (Ausschnitte) |
| `state` (`clap.state`): `save(ostream)` / `load(istream)` [main-thread]; Host-Seite `mark_dirty`; für „Duplizieren / Preset / Projekt" zusätzlich `state-context` | `ext/state.h` | Wortlaut |
| `audio-ports` (`clap.audio-ports`): `count(is_input)`, `get(...)`; Flags u. a. `CLAP_AUDIO_PORT_IS_MAIN`, `SUPPORTS_64BITS`, `PREFERS_64BITS` | `ext/audio-ports.h` | Wortlaut |
| `gui`: eingebettet oder schwebend; Fenster-APIs `win32` (Einbetten über `SetParent`), `cocoa`, `uikit`, `x11`, `wayland` (nur schwebend) | `ext/gui.h` | Wortlaut |
| `voice-info` (`clap.voice-info`): `voice_count`, `voice_capacity`, Flag `CLAP_VOICE_INFO_SUPPORTS_OVERLAPPING_NOTES` — für polyphone Modulation des Hosts | `ext/voice-info.h` | Wortlaut |
| **`tuning` gibt es — aber nur als Entwurf** (`ext/draft/tuning.h`, ID `clap.tuning/2`) und **in Richtung Host → Plug-in**: der Host stellt eine Stimmungstabelle bereit (`clap_host_tuning.get_relative(tuning_id, channel, key, sample_offset)` in Halbtönen gegen gleichstufig A4 = 440 Hz, `should_play`, `get_tuning_count`, `get_info`), das Plug-in meldet nur `changed`. Ein Plug-in kann damit **keine eigene Stimmung nach außen melden**. (Im Feld des Ereignisses steht wörtlich `tunning_id`.) | `ext/draft/tuning.h` | **Wortlaut selbst gelesen** |
| Feinstimmung je Note in beiden Formaten: CLAP `CLAP_NOTE_EXPRESSION_TUNING` (`100 * value` Cent); VST3 `NoteOnEvent::tuning` („1.f = +1 cent") und Note-Expression `kTuningTypeID` (±120 Halbtöne) | `include/clap/events.h` Z. 196–197; `pluginterfaces/vst/ivstevents.h` Z. 43–51, `ivstnoteexpression.h` Z. 49 | Wortlaut |
| `note-name` (`clap.note-name`): Plug-in nennt Namen je (Port, Taste, Kanal), [main-thread]; README: „useful for drum machines" | `ext/note-name.h`; README | Wortlaut |
| Weitere Erweiterungen im Repo, für uns nennenswert: `latency`, `tail`, `thread-pool` (Host-Threadpool), `preset-load`, `track-info`; Entwürfe `background-activation` (Aktivieren auf einem Hintergrund-Thread, für Plug-ins mit „complex computation or even I/O during activation"), `background-progress` (Fortschritt und Abbruch), `resource-directory`, `undo`, `webview` | Baum; `ext/draft/background-activation.h`, `…/background-progress.h`, `ext/thread-pool.h` | Wortlaut |
| **`clap-helpers`**: MIT, C++ (≈ 311 kB Quelltext), letzter Push 2026-10-18; Header-Bibliothek unter `include/clap/helpers/` mit **beiden Seiten**: `plugin.hh/.hxx` (Plug-in-Basis) und `host.hh/.hxx`, `plugin-proxy.hh/.hxx`, `host-proxy`, `event-list.hh`, `param-queue.hh`, `misbehaviour-handler.hh`, `checking-level.hh`, Preset-Discovery-Hilfen. Kein README im Repo (HTTP 404) | `api.github.com/repos/free-audio/clap-helpers` (+ `/languages`, Baum) | Wortlaut (API); Inhalt der Header nicht gelesen |
| **`clap-host`**: MIT, „Reference Host", C++ (≈ 118 kB), letzter Push 2026-06-18; **nutzt Qt6 (Core, Widgets)**, RtAudio, RtMidi; README: „This repo serves as an example to demonstrate how to create a CLAP host." Kernstück `host/plugin-host.cc` 1256 Zeilen; ~40 Dateien unter `host/` | README, `CMakeLists.txt` Z. 19–41, Baum | Wortlaut |
| **`clap-wrapper`**: MIT (Copyright 2022 defiantnerd), Release **v0.17.0 vom 2026-10-20**; hüllt eine CLAP in **VST3, AUv2, AUv3, AAX und eine einfache Standalone**; Prinzip: der Wrapper ist selbst ein CLAP-Host, der die CLAP lädt, und stellt nach außen das Zielformat dar; braucht beim Bau das CLAP- und das VST3-SDK | README, LICENSE, `api.github.com/.../releases` | Wortlaut |
| **`clap-plugins`**: MIT, Beispiel-Plug-ins, C++, letzter Push 2026-10-21; GUI mit Dear ImGui, in Software gerendert, kein GPU-Kontext; eingebettet und schwebend unter Win32 | README, API | Wortlaut |
| **`clap-validator`** (robbert-vdh): MIT, Rust, letzter Push 2026-07-24 — „automatic CLAP validation and testing tool" | API; CLAP-README §Resources | Wortlaut (API) |

## 3. Offline und Bounce in VST3

| Angabe | Quelle | Reichweite |
|---|---|---|
| `ProcessSetup` trägt `processMode` (`ProcessModes`), `symbolicSampleSize`, `maxSamplesPerBlock`, `sampleRate`; `enum ProcessModes { kRealtime, kPrefetch, kOffline }` | `pluginterfaces/vst/ivstaudioprocessor.h` Z. 147–152, 171–181 | **Wortlaut selbst gelesen** |
| **kRealtime:** Aufrufe im Echtzeittakt (numSamples / Abtastrate); das Plug-in soll so schnell wie möglich rechnen | ebd. Z. 118–127 (Doxygen-Kommentar) | Wortlaut |
| **kPrefetch:** Aufrufe mit wechselndem Takt (Jitter, langsamer oder schneller als Echtzeit); gleiche Qualität wie in Echtzeit; das Plug-in darf **nicht** auf Echtzeit bremsen (z. B. Disk-Streaming); der Host soll Sampler-Plug-ins nicht im Prefetch-Modus rechnen | ebd. | Wortlaut |
| **kOffline:** schneller oder langsamer als Echtzeit; **„higher quality than realtime could be used"**; Plug-ins mit Disk-Streaming sollen sich die Zeit nehmen, notfalls auf Echtzeit oder langsamer bremsen | ebd. | Wortlaut |
| Umschalten: Realtime ↔ Prefetch geschieht im Echtzeit-Thread ohne neues `setupProcessing` (das Plug-in liest `ProcessData::processMode` je Aufruf); Realtime/Prefetch ↔ Offline verlangt ein neues `setupProcessing` | ebd. | Wortlaut |
| Plug-in-seitig steuerbar: `IPrefetchableSupport::getPrefetchableSupport` mit `kIsNeverPrefetchable`, `kIsYetPrefetchable`, `kIsNotYetPrefetchable` (optional, seit 3.6.5); ohne diese Schnittstelle entscheidet der Host | `pluginterfaces/vst/ivstprefetchablesupport.h` | Wortlaut |
| Kategorien-Kennzeichen: `kOnlyRealTime` („OnlyRT" — nichts schneller als Echtzeit), `kNoOfflineProcess`, `kOnlyOfflineProcess` | `ivstaudioprocessor.h` Z. 84–87 | Wortlaut |
| `IComponent::setIoMode(kOfflineProcessing)` kennzeichnet einen Offline-Kontext; laut FAQ wird der Offline-Modus benutzt beim Export (Downmix) und bei „Direct Offline Processing" (seit Cubase 9.5); im Offline-Kontext darf `setState/getState` im selben Thread wie `process` kommen | `ivstcomponent.h` Z. 122–127; Dev-Portal `pages/FAQ/Processing.html` | Wortlaut |
| `getTailSamples` (`kNoTail`, `kInfiniteTail`) — vom Host u. a. für Offline-Verarbeitung genutzt | `ivstaudioprocessor.h` Z. 350–360 | Wortlaut |
| **Vertrag zur Reproduzierbarkeit: nicht gefunden.** In den 62 Headern von `pluginterfaces` (`base/`, `gui/`, `vst/`) kein Treffer zu „determin", „reproduc", „bit exact", „same output"; „random" nur bei der Parameterfunktion `kRandomize` und einer Beispiel-UID. Auf der FAQ-Seite „Processing" ebenso kein Treffer | grep über alle Header am Commit `4f547e8`; FAQ-Seite | Suche über die genannten Dateien — **kein Treffer ist kein Beweis** für das ganze Dev-Portal |
| **CLAP ebenso:** in den 68 Headern kein Vertrag; „deterministic" kommt nur im Sinn von Zeitverhalten vor („execution should be as deterministic as possible to meet the audio interface's deadline") | grep über `include/clap/**` | Suche über alle Header |

## 4. Was ein Host mindestens braucht — VST3 gegen CLAP

Belegt aus den Headern und dem Dev-Portal (Abschnitte 1–3), Spalte „Reichweite" am Tabellenende.

| Baustein | VST3 | CLAP |
|---|---|---|
| Binärdatei laden | Bündel `.vst3`; Symbole `InitDll`, `GetPluginFactory`, `ExitDll` (Windows) — fertig in `hosting/module_win32.cpp` | `.clap`-DLL; Symbol `clap_entry` → `init(path)`, `get_factory`, `deinit` (gepaarte Aufrufe, `init` nicht gleichzeitig aus mehreren Threads) |
| Fabrik / Klassen | `IPluginFactory` (Klassen-ID, Kategorie) — `hosting/plugprovider` | `clap.plugin-factory`: `get_plugin_count`, `get_plugin_descriptor`, `create_plugin` (thread-safe) |
| Instanz | zwei Teile: `IComponent`+`IAudioProcessor` und `IEditController`, getrennt oder in einer Klasse; Host muss den Controller zum Rechnen nicht anlegen; `initialize` je Objekt genau einmal | ein `clap_plugin`; `init` → `activate` → `start_processing` → `process` |
| Parameter | `IEditController` (Liste), Änderungen zum Prozessor als `IParameterChanges` je Block (fertig: `hosting/parameterchanges`); Rückmeldung über `IComponentHandler` `beginEdit/performEdit/endEdit` | `clap.params`; Werte lesen [main-thread], Änderungen als Ereignisse in `process()`/`flush()`; Host-Seite `request_flush` |
| Zustand | `IComponent::getState/setState` und `IEditController::getState/setState` (Prozessor zuerst) | `clap.state` `save/load` [main-thread]; `mark_dirty` |
| Noten je Block | `IEventList` mit `NoteOnEvent`/`NoteOffEvent` (Tonhöhe 0–127, `tuning` in Cent, `velocity`, `noteId`) — `hosting/eventlist` | Ereignisliste im `clap_process` (`CLAP_EVENT_NOTE_ON` …, oder roh `CLAP_EVENT_MIDI`); Dialekte über `clap.note-ports` |
| Audio-Busse | `getBusCount/getBusInfo`, `setBusArrangements`, `AudioBusBuffers` — `hosting/processdata` | `clap.audio-ports` (`count`, `get`, Flags) |
| Threads | UI-Thread für Init/Zustand, Processing-Thread für `process` (je Funktion als `\note [UI-thread …]` vermerkt) | main-thread / audio-thread / background-thread, je Funktion markiert; `clap.thread-check` zum Prüfen |
| GUI | `IEditController::createView("editor")` → `IPlugView::attached(parent, "HWND")` | `clap.gui`: `is_api_supported("win32", floating)`, `create`, `set_parent` — eingebettet über `SetParent` oder schwebend |
| Absturzschutz | nicht Teil des Formats | nicht Teil des Formats |
| Reichweite | Wortlaut der Header (`ivstaudioprocessor.h`, `ivstcomponent.h`, `ivsteditcontroller.h`, `ivstevents.h`, `gui/iplugview.h`), Hosting-Quelltext, Dev-Portal „API Documentation" | Wortlaut der Header (`entry.h`, `plugin.h`, `factory/plugin-factory.h`, `ext/*.h`, `events.h`) |

**Absturzschutz in DAWs (nur Belegtes):**

- **Bitwig** (Handbuch „Plug-in Handling and Options", Wortlaut per curl): Plug-ins laufen in
  eigenen Sandboxes/Prozessen; Stufen *Within Bitwig* (im Audio-Prozess, ein Absturz reißt die
  Engine mit), *Together* (alle Plug-ins in einem Prozess getrennt von der Engine), *By
  manufacturer*, *By plug-in* (gleiche Plug-ins zusammen), *Individually* (jede Instanz allein —
  mehr Ressourcen, volle Trennung). Abgestürzte Plug-ins werden neu geladen.
- **REAPER** (User Guide 7.84, gedruckt S. 335, §16.18, Wortlaut per pdftotext): „Run as" je
  Plug-in: *Separate process* (alle so markierten in einem Bridge-Prozess), *Dedicated process*
  (je Plug-in ein eigener), *Native only* (im REAPER-Prozess). Das Bridging sei für 32-Bit gedacht,
  man könne aber auch 64-Bit-Plug-ins bridgen, um REAPER gegen fehlerhafte Plug-ins
  abzuschotten („firewalling"). Vorgabe „Automatic" (gedruckt S. 117, Kontextmenü im FX-Browser).
- Ableton, Cubase: nicht gesucht bzw. Cubase-Hilfe gesperrt — nicht belegt.

## 5. Kleine, freie Host-Bausteine ohne JUCE

| Projekt | Lizenz | Sprache | Umfang | zuletzt geändert | Host oder Hülle | Reichweite |
|---|---|---|---|---|---|---|
| **JUCE** (nur zur Einordnung) | dual: **AGPLv3** oder kommerzielle „JUCE 9"-Lizenz; Tarife laut juce.com: *Starter* frei bis 20 000 $ Umsatz, ***Indie* 40 $ je Nutzer und Monat bis 300 000 $ Umsatz, oder 800 $ einmalig**, *Pro* 175 $/Monat bzw. 3500 $ einmalig, *Educational* frei | C++ | — | Push 2026-11-02 | Framework (Host und Plug-in) | Lizenz: `LICENSE.md` Wortlaut; Preise: **nur WebFetch** (vom Werkzeug als Zitat geliefert) |
| **VST3-SDK, Hosting-Klassen** | MIT | C++ | ~3000 Zeilen (§1) + Beispiele `audiohost`, `editorhost`, `validator` | 2026-08-11 (3.8.1) | **Host-Bausteine** | Wortlaut |
| **clap-host** | MIT | C++ (+ Qt6, RtAudio, RtMidi) | ~118 kB, `plugin-host.cc` 1256 Z. | 2026-06-18 | **Beispiel-Host** | Wortlaut |
| **clap-helpers** | MIT | C++ (Header) | ~311 kB | 2026-10-18 | **beides**: `plugin.hh` und `host.hh`, `plugin-proxy.hh` | API-Metadaten, Dateiliste |
| **clap-wrapper** | MIT | C++ / ObjC++ | ~1,8 MB C++ | v0.17.0, 2026-10-20 | **Hülle** CLAP → VST3/AUv2/AUv3/AAX/Standalone | Wortlaut |
| **EasyVst** (iffyloop) | Unlicense oder MIT-0 (gilt nicht für das gehüllte Steinberg-SDK) | C++ | 1 Header + `EasyVst.cpp` 441 Z.; Fenster über SDL2 | 2023-05-06 (ruht) | kleiner **VST3-Host** | README, API |
| **Carla** (falkTX) | GPL-2.0-oder-später | C++ | groß (~98 MB Repo) | 2026-06-22 | **Host** (README: LADSPA, DSSI, LV2, VST2, VST3, AU) | README Wortlaut; nur genannt |
| **pluginval** (Tracktion) | **GPL-3.0** | C++ (lädt JUCE) | — | 2026-10-15 | Prüf-Host für **VST/AU/VST3** (CLAP im README nicht genannt) | README Wortlaut |
| **iPlug2** | zlib-artig („free to use in closed source projects") | C/C++ | großes Repo (~70 MB) | 2026-10-25 | **Plug-in-Framework** (CLAP, VST2, VST3, AUv2, AUv3, AAX, WAM, Standalone); als Plug-in-**Host** nicht beschrieben | README, LICENSE Wortlaut |
| **DPF** (DISTRHO) | ISC (einzelne Formate mit Zusatzbedingungen, `LICENSING.md` nicht gelesen) | C++ | ~28 MB Repo | 2026-10-26 | **Plug-in-Seite** (LADSPA, DSSI, LV2, VST2, VST3, CLAP) | README Wortlaut |
| **nih-plug** | ISC; **VST3-Export über GPLv3-Bindungen `vst3-sys`** (README §Licensing) | Rust | — | 2026-09-03 | Plug-in-Framework (VST3, CLAP) | README Wortlaut; nur genannt |
| **vst3-rs** (coupler-rs) | Apache-2.0 | Rust | — | 2026-03-28 | Bindungen an die VST3-API | API; nur genannt |
| **clap-validator** | MIT | Rust | — | 2026-07-24 | Prüf-Host für CLAP | API |

## 6. Determinismus beim Einfrieren fremder Plug-ins

| DAW | Was belegt ist | Quelle | Reichweite |
|---|---|---|---|
| **Ableton Live 12** | *Freeze Track* erzeugt je Clip einer Spur eine Sample-Datei; danach spielen die Clips die eingefrorenen Dateien. Normal „very quickly" (also schneller als Echtzeit); bei *External Instrument/External Audio Effect* (Hardware) „the freezing process happens in real time", Live erkennt das selbst. Warnung nur zu Zeiteffekten: nach Bearbeiten eingefrorener Spuren mit Hall/Echo kann der Klang nach dem Auftauen anders sein. Ablage `Samples/Processed/Freeze`. **Keine Warnung, dass ein Plug-in bei gleicher Eingabe anders klingen kann** | Live-12-Handbuch Kap. 38.1.4 „Track Freeze", `https://www.ableton.com/en/live-manual/12/computer-audio-resources-and-strategies/` | **Wortlaut selbst gelesen** (curl) |
| **Bitwig Studio** | *Bounce* mit Schalter *Real-time* — „to bounce at the actual speed … of the selection", nötig etwa für externe Hardware; ohne den Schalter also schneller als Echtzeit; *Bounce in place* ersetzt den Clip. Ein eigenes „Freeze" fand ich im Handbuch nicht. Keine Warnung zum Determinismus | `https://www.bitwig.com/userguide/latest/bouncing_to_audio` | Wortlaut (curl) |
| **REAPER 7.84** | *Freeze tracks* rendert die Spur an Ort und Stelle in ein Audio-Item, die Online-FX werden angewendet und entfernt (gedruckt S. 119, §6.17). Render-Geschwindigkeit wählbar: *Full-speed* (Vorgabe), *1x offline*, *online*, *online (idle)*, *offline x 1 (idle)* (Render-Dialog, Kap. 21, gedruckt S. 412). **Ausdrückliche Warnung:** „Online vs Offline rendering: some plug-ins might perform and sound differently if rendered offline." (gedruckt S. 413). Dazu Option *Limit apply FX/Render stems to realtime* (Vorgabe aus) | `https://www.reaper.fm/userguide/ReaperUserGuide784c.pdf` | **Wortlaut selbst gelesen** (pdftotext); ob *Freeze* die Render-Geschwindigkeit des Render-Dialogs nutzt, steht dort nicht — nicht belegt |
| **Cubase** | Nur Suchmaschinen-Zusammenfassungen: *Freeze Channel* rendert ein Instrument in eine Datei, Optionen *Include Inserts*, *Unload Instruments When Frozen*, *Tail Size*; bei externen Instrumenten/Effekten erfolge das Einfrieren in Echtzeit | steinberg.help — **gesperrt** (403 / „Request forbidden by administrative rules") | **kein Beleg** |

**Ergebnis:** Eine Zusage oder ein Vertrag, dass Einfrieren reproduzierbar ist, fand ich bei
keiner DAW und in keinem der beiden Formate (§3). Belegt ist das Gegenteil nur bei REAPER als
Warnung (online gegen offline) und bei Ableton für Hardware (Echtzeit) und Zeiteffekte.

## 7. Eigene Instrumente als Plug-in nach außen

**Was eine Hülle mindestens liefern muss:**

- **CLAP** (Wortlaut der Header): exportiertes `clap_entry` mit `init`/`deinit`/`get_factory`;
  eine Fabrik (`clap.plugin-factory`) mit Deskriptor je Plug-in; ein `clap_plugin` mit `init`,
  `destroy`, `activate`, `deactivate`, `start_processing`, `stop_processing`, `reset`, `process`,
  `get_extension`, `on_main_thread`. Für ein Instrument praktisch nötig: `note-ports`,
  `audio-ports`, `params`, `state`; optional `gui`, `render`, `latency`, `tail`, `voice-info`,
  `note-name`. Die README nennt `state`, `params`, `note-ports`, `audio-ports`, `render`,
  `latency`, `tail`, `gui`, `voice-info`, `track-info`, `tuning` als „Fundamental extensions".
- **VST3** (Dev-Portal „API Documentation", Wortlaut per curl): ein Plug-in besteht aus
  Verarbeitungsteil (`IComponent` + `IAudioProcessor`) und Edit-Controller (`IEditController`);
  die Trennung wird empfohlen, beide Teile in einer Klasse sind möglich („not recommended"); der
  Host muss den Controller zum Rechnen nicht anlegen. `IPlugView` ist optional — `createView`
  liefert die Oberfläche, ohne eigene Oberfläche zeigt der Host (nach Wissen, ungeprüft) seine
  generische Parameteransicht. Kategorie für Instrumente: `kInstrument` („Instrument") bzw.
  `Instrument|Piano`, `Instrument|Synth` usw. (`ivstaudioprocessor.h` Z. 74–80).
- **Abkürzung:** nur eine CLAP-Hülle bauen und mit `clap-wrapper` (MIT) als VST3, AU, AAX und
  Standalone ausliefern (§2). Ob das bei Instrumenten mit eigener Stimmung und vielen
  Parametern verlustfrei geht, steht nicht im README — nicht geprüft.

**Kann ein Instrument, das Töne vorab rechnet, als Plug-in laufen?** Belegt sind die
Bausteine, nicht ein fertiger Weg:

- Teure Arbeit vor dem Rechnen ist vorgesehen: CLAP erlaubt in `activate` Speicher und
  Vorbereitung; der Entwurf `background-activation` (mit `background-progress`) ist genau für
  Plug-ins mit „complex computation" beim Aktivieren gedacht; `thread-pool` gibt Zugriff auf die
  Threads des Hosts. Das Plug-in darf eigene Hintergrund-Threads starten (thread-check empfiehlt
  das für lange Aufgaben wie „asset loading").
- Rechnen schneller als Echtzeit ist im Format vorgesehen: VST3 `kOffline` und `kPrefetch`
  (§3), CLAP `CLAP_RENDER_OFFLINE`. Ein Plug-in, das vorab rechnet, kann im Echtzeitmodus
  fertige Abtastwerte ausgeben; Grenzen sind dann Latenz (`getLatencySamples` / `clap.latency`)
  und der Fall, dass eine Note eintrifft, deren Ton noch nicht gerechnet ist.
- **Vorab-Rechnung durch den Host** (Stichwort „pre-render"/„render ahead"): REAPER hat
  *Anticipative FX processing* (rechnet FX voraus, kann CPU sparen; je Spur abschaltbar,
  gedruckt S. 118 — Wortlaut); Cubase/WaveLab „ASIO-Guard" rechnet laut
  Suchmaschinen-Zusammenfassung nicht scharf geschaltete Spuren voraus (**kein Beleg**,
  Steinberg-Hilfe gesperrt); dass ASIO-Guard die VST3-Betriebsart `kPrefetch` nutzt, ist
  **nicht belegt**.
- Ein Format-Vertrag „Plug-in liefert vorab gerechnete Töne je Note an den Host" (also unser
  Weg B1 umgekehrt) wurde **nicht gefunden**.

## Offen / nicht gefunden

1. **Cubase-Handbuch** (Freeze, Real-Time Export) und **ASIO-Guard-Artikel**: gesperrt
   (`steinberg.help`, `archive.steinberg.help`, `helpcenter.steinberg.de`, 403). Nur
   Suchmaschinen-Zusammenfassungen, kein Beleg.
2. **Vertrag zur Reproduzierbarkeit** (gleiche Eingabe → gleiche Ausgabe) in VST3 oder CLAP:
   nicht gefunden (Header vollständig durchsucht, Dev-Portal nur FAQ „Processing" und „API
   Documentation").
3. Ob ASIO-Guard `kPrefetch` nutzt: nicht belegt.
4. Ob REAPERs *Freeze* die Render-Geschwindigkeit des Render-Dialogs (Full-speed / online)
   übernimmt: im User Guide nicht gefunden.
5. Ableton und Cubase zum Absturzschutz (eigener Prozess): nicht gesucht bzw. gesperrt.
6. Ein CLAP-„Release" mit Datum: die API liefert für `free-audio/clap` keine Releases, nur Tags;
   das Datum von 1.2.10 ist aus dem letzten Commit (2026-07-28) nur ungefähr.
7. Inhalt der `clap-helpers`-Header (Host-Seite) und des `audiohost`-Beispiels im VST3-SDK:
   nur Dateilisten gelesen, Quelltext nicht.
8. `VST3_Usage_Guidelines.pdf`: das Markenzeichen-Symbol in §15 und die Logos selbst (Bilder)
   nicht gesehen; ob ein bloßes „Plug-in (VST3)" in einer Formatspalte bereits als „refer to the
   VST trademark" gilt, sagen die Guidelines nicht ausdrücklich — Auslegung offen.
9. `DPF/LICENSING.md` (Zusatzbedingungen je Format) nicht gelesen.
