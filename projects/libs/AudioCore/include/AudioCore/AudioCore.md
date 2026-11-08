# AudioCore — allgemeines Audio ohne Qt

> **Version:** 0.1.0
> **Datum:** 2026-11-08
> **Typ:** CppModuleDoc
> **Status:** gebaut (Stufe 34e-2)
> **Modul:** AudioCore (STATIC, `projects/libs/AudioCore`)
> **Namespace:** `lumi::audio`
> **Abhängigkeiten:** Standardbibliothek; kein Qt, kein `windows.h`
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Zweck

Was Composer, Loop, Export, Freeze, Aufnahme und die Messung gemeinsam brauchen und nicht an ein Instrument gebunden
ist. **ME1 a** (Patrik S177, `docs/audio/Instrumente_IC_Konzept.md` §6): `Lautheit` und `WavFormat` kamen aus der App
hierher, nicht in `InstrumentMeasure` — der Composer misst Songs und soll nicht an einer Instrument-Bibliothek hängen
(Kipppunkt von KN1 a). Die App und `InstrumentMeasure` hängen an `AudioCore`; IC bezieht das Paket `audiocore` mit 34l
(TP1 a). Übernahme: Journal J20.

## 2. Bau

- `Solution.json`: STATIC, `compile_options` nach E1 a wie die Instrument-Bibliotheken (PRIVATE); gerechnet wird in
  den `.cpp`-Dateien.
- `src/Source.cmake` nennt Quellen und Köpfe. Paket `package_audiocore` → `out/package/audiocore-v<ver>.zip`.

## 3. Schnittstellen

| Kopf | Inhalt |
|---|---|
| `Lautheit.hpp` | Lautheit nach EBU R128 / ITU-R BS.1770 eines Stereo-Signals: integriert mit beiden Gates, Maximum Momentary (0,4 s) und Short-term (3 s), Spitze (EBU Tech 3341) |
| `WavFormat.hpp` | WAV lesen ohne Qt: RIFF/WAVE, PCM 16/24 Bit, IEEE float 32 Bit, Mono/Stereo, auch `WAVE_FORMAT_EXTENSIBLE` (`liesWavBytes`, `liesWavDatei`) |

Geschrieben wird WAV weiter mit Qt in der App (`audio/WavDatei`: 32/24/16 Bit mit Dither); ein Schreiber ohne Qt (IC
`schreibeWav`) fehlt hier noch (Journal J20, offen).

## 4. Tests

- `test_ComposerLautheit.cpp` (gegen die Prüftöne aus EBU Tech 3341), `test_ComposerClipLautheit.cpp`, `test_LoopPegel.cpp`.
- `test_WavDatei.cpp` (Lesen ohne Qt), die Salamander-Fälle (lesen die Aufnahme über `liesWavDatei`).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.1.0 | 2026-11-08 | Session 177. Angelegt mit 34e-2 (ME1 a): `Lautheit`, `WavFormat` aus der App. |
