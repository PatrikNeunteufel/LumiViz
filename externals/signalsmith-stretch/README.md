# Signalsmith Stretch

Dehnen und Tonhöhe ändern (Zeitstreckung und Transposition im Spektrum), C++, nur Header — für das Wave-Muster im
Loop: die Spur *Pitch* und später *Follow tempo* (Stufe 15f; `projects/apps/LumiViz/docs/audio/Loop_Konzept.md` §4p,
Entscheid WV6 a, Patrik S168). Dazu aus **Signalsmith Linear** die zwei Header, die Stretch einbindet.

| | Signalsmith Stretch | Signalsmith Linear |
|---|---|---|
| Fassung | Tag `1.4.0`, Commit `a670068d9aeb64913331d5cc29337b19a457a7df` vom 2026-09-25 (der Header nennt in `version` noch 1.3.2) | Tag `0.6.4`, Commit `de55e6a50ffcf6f8f43f649692d94691c7025151` vom 2026-09-25 — die Fassung, die Stretch 1.4.0 in seiner `CMakeLists.txt` verlangt |
| Quelle | <https://github.com/Signalsmith-Audio/signalsmith-stretch/tree/a670068d9aeb64913331d5cc29337b19a457a7df> | <https://github.com/Signalsmith-Audio/linear/tree/de55e6a50ffcf6f8f43f649692d94691c7025151> |
| Lizenz | MIT, Copyright (c) 2022 Geraint Luff / Signalsmith Audio Ltd. | MIT, Copyright (c) 2025 Signalsmith Audio |
| Braucht | Linear (`stft.h`) und die Standardbibliothek | nur die Standardbibliothek (ohne `SIGNALSMITH_USE_*`) |

Geladen am 2026-11-01 (S169, Stufe 15f nach WV6 a), einzeln über `raw.githubusercontent.com`. Die Git-Blobs stimmen
mit dem Baum der GitHub-Schnittstelle zu den beiden Commits überein (geprüft beim Laden).

| Datei hier | aus | Byte | Git-Blob | SHA-256 |
|---|---|---|---|---|
| `signalsmith-stretch.h` | Stretch `signalsmith-stretch.h` | 35 446 | `cd13f2e98b8967577c6da9bc848d4e42c5eeab2a` | `1188667959ac19dd40c0a6abbce694e44705615ec4f0e8db8af0f1cfb4c5dea7` |
| `LICENSE.txt` | Stretch `LICENSE.txt` | 1 093 | `889031e63236bb72aa28ff2ec2362db1691723d1` | `ee2ef82481ffb445ecdd4b3a4c1f82c0cddb2da8fe39d8e8dc384fffb3e7f06f` |
| `signalsmith-linear/stft.h` | Linear `stft.h` | 24 313 | `5cbbcafc530501056e7ec909cb73e22c7a722683` | `7422ff5ddc2a017dc0587dcce99ce5ba30a2414bc2996081f6b00d56905d9b8c` |
| `signalsmith-linear/fft.h` | Linear `fft.h` | 45 242 | `49986af016d6eea6bae7e09e7c53858beac400be` | `fb55f69ac14cdbac66b9c76053069ff63bc36d18af20d8c028b9acc0aad6516a` |
| `signalsmith-linear/LICENSE.txt` | Linear `LICENSE.txt` | 1 074 | `c2569820cd018245f891b974048dc4ab128f43b1` | `072b5e9eb5b22880bdf6256324654cf3ce53dba828edb911921121e1be09d9c3` |

## Was hier liegt

- Die fünf Dateien oben — **unverändert**. Nur die Lage ist eine andere: `stft.h` und `fft.h` liegen unter
  `signalsmith-linear/`, so findet `#include "signalsmith-linear/stft.h"` sie ohne die Weiterleitungen aus
  `include/signalsmith-linear/` des Repositorys von Linear.
- `Include.cmake` — **eigen**: legt das Ziel `signalsmith-stretch` (INTERFACE, nur Header) an und bindet es an ein
  Ziel. `Solution.json` nennt sie über das Feld `include`.

Nicht übernommen: von Stretch `cmd/`, `web/`, `include/` (Weiterleitung), `CMakeLists.txt` (holt Linear mit
FetchContent), `README.md`, `SUPPORT.txt`; von Linear `linear.h`, `approx.h`, `platform/` (nur mit
`SIGNALSMITH_USE_*`), `tests/`, `include/`, `CMakeLists.txt`, `README.md`, `SUPPORT.txt`.

## Wie LumiViz es benutzt

Nur in `src/loop/Dehner.cpp`, hinter der Schnittstelle `lumi::loop::Dehner` (`include/loop/Dehner.hpp`): kein Typ von
Signalsmith geht nach außen; eine andere Umsetzung (das eigene TD-PSOLA aus WV6 a) kommt in `dehnerUmsetzungen()`
dazu. `SignalsmithStretch<float, std::mt19937>` mit festem Seed: Die Vorgabe `std::default_random_engine` ist je
Standardbibliothek eine andere Maschine, `std::mt19937` ist im Standard festgelegt — zwei Läufe rechnen gleich.
Gebunden an den Kern der App (`LumiViz.Core`).
