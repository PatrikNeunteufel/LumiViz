# Eigen

Lineare Algebra (dünn besetzte Matrizen, Zerlegungen) — für die Platte und den Eigenlöser des Instrumentkörpers
(Stufen 31c-3/31c-4; `projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1, Entscheide BM5-1 a und EL1 a,
Patrik S147).

| | |
|---|---|
| Fassung | 3.4.1 (Release vom 2025-09-29) |
| Quelle | <https://gitlab.com/libeigen/eigen/-/archive/3.4.1/eigen-3.4.1.zip> |
| Geladen | 2026-10-23 (Freigabe Patrik) |
| Größe des Archivs | 3 721 838 Byte |
| SHA-256 des Archivs | `C57EF61EE12F67F55856C1F5DE61877E90F2FA8C201DFC541A55DFB3C93BE880` |
| Lizenz | „Eigen is primarily MPL2 licensed" (`COPYING.README`); einzelne Dateien enthalten fremden Code unter BSD, LGPL-2.1, Apache-2.0 oder MINPACK (`COPYING.*`) |

## Was hier liegt

- `Eigen/` — der Ordner `Eigen` des Archivs, **unverändert** (mit `diff -r` verglichen; 337 Dateien).
- `COPYING.*` — alle Lizenzdateien des Archivs, unverändert.
- `Include.cmake` — **eigen**: legt das Ziel `Eigen` (INTERFACE, nur Header) an, setzt `EIGEN_MPL2_ONLY` und bindet
  es an ein Ziel. `Solution.json` nennt sie über das Feld `include` (Muster `externals/miniz`, `externals/CDT`).

Nicht übernommen: `unsupported/`, `test/`, `doc/`, `bench/`, `blas/`, `lapack/`, `demos/`, die CMake-Dateien des
Archivs.

## Lizenzen im Einzelnen (gelesen 2026-10-23)

- **`EIGEN_MPL2_ONLY`** ist gesetzt: wer eine Datei mit LGPL-Code einbindet (`IncompleteCholesky.h`,
  `IncompleteLUT.h`; Wächter `NonMPL2.h`), bekommt einen Übersetzungsfehler.
- **BSD:** die Anbindungen an MKL, BLAS und LAPACKE (`*_MKL.h`, `*_BLAS.h`, `*_LAPACKE.h`, `lapacke.h`,
  `PardisoSupport.h`) — nur mit `EIGEN_USE_MKL`/`EIGEN_USE_LAPACKE`, hier nicht gesetzt; `Geometry/AlignedBox.h`
  (Willow Garage, OSRF) — das Modul Geometry wird nicht eingebunden.
- **Freizügig ohne Vermerkpflicht:** die Umrechnung in `Core/arch/Default/Half.h` (F. Giesen 2016, „Redistribution
  and use in source and binary forms, with or without modification, are permitted").
- **Apache-2.0:** `Core/arch/Default/BFloat16.h` (TensorFlow Authors 2017).

## Wie LumiViz es benutzt

Nur in der Bibliothek `InstrumentBody`, hinter ihren Schnittstellen (Platte, Eigenlöser; BM5-1 a): kein Typ von Eigen
geht nach außen. Gebunden an die Unit-Tests und (ab 31c-4) an `InstrumentBodyTool` — **nicht an die App**.

## Auf eine neue Fassung heben

Das Archiv laden (Freigabe), Prüfsumme bilden, `Eigen/` und `COPYING.*` ersetzen, die Lizenzen oben neu durchsehen,
die Tabelle und `THIRD_PARTY_NOTICES.md` nachführen, beide Compiler bauen, `*Loop Koerper*` fahren und die
Bezugswerte vergleichen. Spectra verlangt eine dazu passende Fassung.
