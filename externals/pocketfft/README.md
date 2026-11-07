# pocketfft

Schnelle Fourier-Transformation (FFT), C++, nur ein Header — für die Faltung je Ton mit der Antwort des
Resonanzbodens (Stufe 31c-7; `projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1, Entscheide FT1 a,
Patrik S149, und FF1 a, Patrik S150).

| | |
|---|---|
| Fassung | Zweig `cpp`, Commit `4d865cba24359b05b37da4e75aee24498c3b78e7` vom 2026-10-18 („Add POCKETFFT_NO_EXCEPTIONS option for builds without exception handling“) |
| Quelle | <https://github.com/mreineck/pocketfft/tree/4d865cba24359b05b37da4e75aee24498c3b78e7> |
| Geladen | 2026-11-07 (S175, IK2-2-1 a, KN2 a): Zweig `cpp` geklont, die Datei aus dem Commit; dieselbe Fassung, die IC am 2026-10-29 geladen hat (IC `UEBERNAHME_LUMIVIZ.md` §3, dort ohne Commit) |
| `pocketfft_hdronly.h` | 121 769 Byte, LF, SHA-256 `518a95c552aeaee352aa30122137cd9e135d23c91e2863d9272aaa758744c86d`, Git-Blob `d28c28b589126574505a660b9c7f624775cf4367` |
| `LICENSE.md` | 1 498 Byte, SHA-256 `a85ca13fdf90160b64a0698215868c13b74d835ad0a4e2ba44713b8c5058a056`, Git-Blob `c3a4c06a92d024e12ab76ce6b20eae815979552e` |
| Lizenz | BSD-3-Clause (`LICENSE.md`; derselbe Text im Kopf des Headers) |
| Braucht | nur die Standardbibliothek |

Die Git-Blobs sind die des Klons im Commit oben (geprüft beim Laden).

**Vorher** (S150 bis S174): Commit `c90e55b3d529f8efa40ed01a20de22405f45fc65` vom 2026-06-30 („cosmetics“), 120 735 Byte,
SHA-256 `3e9a0531…8e92fdc`, Blob `de6c8d6b…`. Der Unterschied (91 Zeilen) betrifft nur die Fehlerbehandlung: 28 Stellen
`throw` → `POCKETFFT_THROW`, dazu der Schalter `POCKETFFT_NO_EXCEPTIONS` (hier nicht gesetzt — `throw` wie vorher); die
Rechnung ist unverändert. Übernahme und Wächter: `projects/apps/LumiViz/docs/modellierung/anbindung-ic/Uebernahme_Journal.md`.

## Was hier liegt

- `pocketfft_hdronly.h` und `LICENSE.md` — **unverändert** aus dem Commit oben.
- `Include.cmake` — **eigen**: legt das Ziel `pocketfft` (INTERFACE, nur Header) an, setzt
  `POCKETFFT_NO_MULTITHREADING` (LumiViz rechnet Töne schon parallel, eine FFT je Ton auf einem Thread) und bindet
  es an ein Ziel. `Solution.json` nennt sie über das Feld `include`.

Nicht übernommen: `README.md` des Repositorys, `pocketfft_demo.cc`, `meson.build`.

## Lizenzen im Einzelnen (gelesen 2026-10-24)

- **BSD-3-Clause:** `LICENSE.md` nennt „Copyright (C) 2010-2018 Max-Planck-Society“. Der Kopf des Headers nennt
  weiter: Max-Planck-Society 2010-2024, Peter Bell 2019-2020, für die DCT-IV ungerader Länge Matteo Frigo und das
  Massachusetts Institute of Technology (2003, 2007-14), für `prev_good_size` Tan Ping Liang und Peter Bell (2024),
  für die Absicherung gegen Überlauf in `good_size` Cris Luengo (2024); Autoren Martin Reinecke, Peter Bell. Der
  Lizenztext ist derselbe.
- Vermerk in `THIRD_PARTY_NOTICES.md` §1.

## Wie LumiViz es benutzt

Nur in `projects/libs/InstrumentCore/src/Fft.cpp`, hinter der Schnittstelle `lumi::kern::Fft`
(`InstrumentCore/Fft.hpp`): kein Typ von pocketfft geht nach außen; eine andere Umsetzung kommt in `fftUmsetzungen()`
dazu (Memory „Externals austauschbar einbinden“). Gebraucht werden `r2c` und `c2r` (reell ↔ Spektrum) in `double`.
Gebunden PRIVATE an die Bibliothek `InstrumentCore` (seit S175, KN2 a; vorher an `LumiViz.Core`).
