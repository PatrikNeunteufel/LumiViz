# Spectra

Eigenwerte großer dünn besetzter Matrizen (Lanczos mit implizitem Neustart, generalisiert und mit Shift-Invert) —
für den Eigenlöser des Instrumentkörpers (Stufe 31c-4; `projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md`
§4.1, Entscheide BM5-1 a und EL1 a, Patrik S147).

| | |
|---|---|
| Fassung | 1.2.0 (Release vom 2025-06-29, `CHANGELOG.md` des Archivs) |
| Quelle | <https://github.com/yixuan/spectra/archive/refs/tags/v1.2.0.zip> |
| Geladen | 2026-10-23 (Freigabe Patrik, S148) |
| Größe des Archivs | 381 430 Byte |
| SHA-256 des Archivs | `AD7263DB3F3EDC23BD3FD546D06660FF474749EE851DE1A115782D2CCB0A7FF9` |
| Lizenz | MPL-2.0 (`LICENSE`); jeder übernommene Header trägt den Kopf der MPL-2.0 |
| Braucht | Eigen (hier `externals/Eigen`, 3.4.1) und die Standardbibliothek, sonst nichts |

## Was hier liegt

- `Spectra/` — der Ordner `include/Spectra` des Archivs, **unverändert** bis auf den weggelassenen Unterordner
  `contrib/` (mit `diff -r` verglichen; 52 Dateien).
- `LICENSE` — die Lizenzdatei des Archivs, unverändert.
- `Include.cmake` — **eigen**: legt das Ziel `Spectra` (INTERFACE, nur Header) an, hängt es an das Ziel `Eigen` und
  bindet es an ein Ziel. `Solution.json` nennt sie über das Feld `include`; `Eigen` steht dort vor `Spectra`.

Nicht übernommen: `include/Spectra/contrib/` (LOBPCG und partielle SVD; `LOBPCGSolver.h` steht unter MIT statt
MPL-2.0 und wird nicht gebraucht), `test/`, `examples/`, `benchmark/`, `doxygen/`, `cmake/`, die CMake-Dateien des
Archivs.

## Lizenzen im Einzelnen (gelesen 2026-10-23)

- **MPL-2.0:** alle 52 übernommenen Header („This Source Code Form is subject to the terms of the Mozilla Public
  License v. 2.0"). Copyright (C) 2016-2025 Yixuan Qiu; einzelne Dateien Netherlands eScience Center (2020), Gael
  Guennebaud (2008-2010) und Jitse Niesen (2010) — die beiden letzten aus Eigen übernommen.
- Vermerk in `THIRD_PARTY_NOTICES.md` §1.
- Die Header binden außer `<Spectra/…>` nur Module von Eigen ein (Core, SparseCore, SparseCholesky, SparseLU,
  Cholesky, LU, QR, SVD, Eigenvalues, Householder, Jacobi, IterativeLinearSolvers) — mit `EIGEN_MPL2_ONLY` aus
  `externals/Eigen` verträglich.

## Wie LumiViz es benutzt

Nur in der Bibliothek `InstrumentBody`, hinter der Schnittstelle des Eigenlösers (BM5-1 a): kein Typ von Spectra oder
Eigen geht nach außen. Gebraucht wird `SymGEigsShiftSolver` mit `SymShiftInvert` (K φ = λ M φ, Shift-Invert um eine
Mittenfrequenz). Gebunden an die Unit-Tests und (ab 31c-4) an `InstrumentBodyTool` — **nicht an die App**.
