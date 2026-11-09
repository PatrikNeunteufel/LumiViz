# harvest/instruments-calculation — der Code von InstrumentsCalculation als Vorlage

> **Seit:** S182 (2026-11-09), Auflösung von InstrumentsCalculation (IC), Entscheid Patrik AU2 a; Plan
> `projects/apps/LumiViz/docs/modellierung/anbindung-ic/Aufloesung_IC_Plan.md` (U5).
> **Stand:** IC `rechnung/` am Commit `4409088` (2026-11-09), 125 Dateien, Byte für Byte; ohne `externals/` (doctest und
> pocketfft hat LumiViz selbst, pocketfft gepinnt `4d865cb`).
> **Wird nicht gebaut:** kein `Source.cmake` und keine `Solution.json` zeigt hierher. Das `CMakeLists.txt` darin ist das
> von IC und bleibt nur zum Nachlesen.

## Wozu

Vorlage für die Klassen, die LumiViz noch nicht rechnet (Stufe 34j Klasse 10, 34k Klasse 2; Klassen 3, 6–9, 11 ohne
Stufe), und Beleg, woraus die übernommenen Kerne stammen. Die Doku dazu liegt unter
`projects/apps/LumiViz/docs/modellierung/<klasse>/ic/`, die Abdrücke unter `rechnung/werte/` (je Compiler; unter Windows
gilt `abdruck_msvc.txt`, auch für Windows-clang).

**Regel:** Ist eine Klasse in LumiViz übernommen und ihr Wächter grün, wird ihr Teil hier gelöscht (der Commit
`4409088` von IC und dieser Ordner im Git-Verlauf bleiben der Beleg). Ist alles übernommen, entfällt der Ordner.

## Stand je Teil

Belegkraft: aus dem Übernahme-Journal (`docs/modellierung/anbindung-ic/Uebernahme_Journal.md`) und den Namen in den
Bibliotheken, nicht Zeile für Zeile verglichen.

| Teil | in LumiViz | wo |
|---|---|---|
| `klassen/01_hammer_saite` | **übernommen** (34d Flügel, 34i Upright, Honky-tonk, CP-70; Kern `struckString` mit Clavinet, Hackbrett, Cimbalom) | `projects/libs/InstrumentModels/src/hammer/`, `…/saite/` |
| `klassen/02_zupf_saite` | offen — Stufe 34k (Cembalo, Harfe, Gitarre, Laute, Oud) | — |
| `klassen/03_bogen_saite` | offen, ohne Stufe | — |
| `klassen/04_platte_stab` | **übernommen** (34g; die Steeldrum als Vorgabe ohne Datei) | `InstrumentModels/src/stab/` |
| `klassen/05_membran` | **übernommen** (34h) | `InstrumentModels/src/trommel/` |
| `klassen/06_becken_gong` … `09_luftstrahl_rohr`, `11_tonrad` | offen, ohne Stufe | — |
| `klassen/10_zunge` | offen — Stufe 34j (Akkordeon, Harmonium, Melodica, Mundharmonika) | — |
| `gemeinsam/` Balken, Membran, Reibung | übernommen | `InstrumentModels/…/gemeinsam/` |
| `gemeinsam/` Eigenzerlegung, Fft, Kurve, Modal | übernommen | `projects/libs/InstrumentCore/` |
| `gemeinsam/` Rohr, Rohrschleife | offen (Klassen 7–9) | — |
| `gemeinsam/` Koerper | nicht übernommen — LumiViz rechnet Körper mit `projects/libs/InstrumentBody` | — |
| `gemeinsam/` Json, Werkzeug | Hilfen von IC (Lautheit dort für die Hörlisten; in LumiViz `AudioCore/Lautheit`) | — |
| `messung/`, `setzen/` | übernommen (34e) | `projects/libs/InstrumentMeasure/` |
| `werkzeuge/stimme_stab.cpp` | übernommen | `InstrumentMeasure/StabStimmen` |
| `werkzeuge/stimme_blech.cpp`, `…_bogen`, `…_luftstrahl`, `…_rohrblatt`, `…_zunge` | offen, mit ihrer Klasse | — |
| `werkzeuge/hoerproben.cpp`, `Sammlung.cpp`, `Hoerproben.hpp` | nicht übernommen — schrieben die IC-Hörlisten (jetzt `hoertest/listen/ic/`, Klänge unter `…\cmake\samples\Hoerproben\IC\`) | — |
| `werkzeuge/messe.cpp`, `glaette_ziel.cpp`, Tests | Vorlage | — |
| `werte/` | Abdrücke und Bezüge von IC; die übernommenen Klassen tragen ihre Teile in `tests/unit/UnitTests/werte/*_ic.txt` | — |
