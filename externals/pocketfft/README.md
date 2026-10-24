# pocketfft

Schnelle Fourier-Transformation (FFT), C++, nur ein Header — für die Faltung je Ton mit der Antwort des
Resonanzbodens (Stufe 31c-7; `projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1, Entscheide FT1 a,
Patrik S149, und FF1 a, Patrik S150).

| | |
|---|---|
| Fassung | Zweig `cpp`, Commit `c90e55b3d529f8efa40ed01a20de22405f45fc65` vom 2026-06-30 („cosmetics“) |
| Quelle | <https://github.com/mreineck/pocketfft/tree/c90e55b3d529f8efa40ed01a20de22405f45fc65> |
| Geladen | 2026-10-24 (Freigabe Patrik, S150, FF1 a), einzeln über `raw.githubusercontent.com` |
| `pocketfft_hdronly.h` | 120 735 Byte, SHA-256 `3e9a05318d8e3b1446bda1c4617e6a103cdd23599ae0a776a92a6e8800e92fdc`, Git-Blob `de6c8d6b42a8372e68b0f1837122c3eafdfba9f6` |
| `LICENSE.md` | 1 498 Byte, SHA-256 `a85ca13fdf90160b64a0698215868c13b74d835ad0a4e2ba44713b8c5058a056`, Git-Blob `c3a4c06a92d024e12ab76ce6b20eae815979552e` |
| Lizenz | BSD-3-Clause (`LICENSE.md`; derselbe Text im Kopf des Headers) |
| Braucht | nur die Standardbibliothek |

Die Git-Blobs stimmen mit den Angaben der GitHub-Schnittstelle zu diesem Commit überein (geprüft beim Laden).

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

Nur in `src/loop/Fft.cpp`, hinter der Schnittstelle `lumi::loop::Fft` (`include/loop/Fft.hpp`): kein Typ von
pocketfft geht nach außen; eine andere Umsetzung kommt in `fftUmsetzungen()` dazu (Memory „Externals austauschbar
einbinden“). Gebraucht werden `r2c` und `c2r` (reell ↔ Spektrum) in `double`. Gebunden an den Kern der App
(`LumiViz.Core`).
