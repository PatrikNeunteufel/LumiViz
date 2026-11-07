# Changelog Session 175 (2026-11-07)

> **Die Rechenkerne bekommen ihre Bibliothek.** Teilschritt 34c ist fertig: `InstrumentCore` trägt Tonkern, Fabrik,
> Parametertafel und die gemeinsamen Bausteine; Fft, Eigenzerlegung und Anschlagstufe sind aus der App dorthin
> umgezogen, der Klang ist Bit für Bit derselbe. Teilschritt 34d (der Flügel als erster Kern) ist begonnen und
> entschieden (FU1 a–FU3 a, BK1 a), aber nicht fertig — sein Zwischenstand ist nicht committet
> (`docs/audio/Instrumente_IC_Konzept.md` 0.6.0).

## Neu

- **Bibliothek `InstrumentCore`** (`projects/libs/InstrumentCore`, STATIC, KN1 a): die Ton-Schnittstelle der
  Rechenkerne (`Tonkern`, Fabrik, Auskunft, Tonauftrag in Frames), die Parametertafel (Gruppe, Art, Darstellung,
  Grenzen, Vorgabe, Belegkraft, Herkunft, gestimmte Werte mit Abdruck der Eingaben, „ungemessen“) und die gemeinsamen
  Bausteine: Fft mit pocketfft, Eigenzerlegung bis 32 × 32, Kurve über die Tastatur, Modal, Pool, SHA-256,
  Anschlagstufe. Fließkomma-Schalter nach E1 a nur an der Bibliothek; Quellpaket `package_instrumentcore` für IC.
- **Übernahme-Journal** `docs/modellierung/anbindung-ic/Uebernahme_Journal.md`: je Baustein Quelle, Ziel, was übernommen
  und verworfen wurde und warum, Wächter vorher/nachher.

## Geändert

- **pocketfft** auf Commit `4d865cb` (die Fassung, die IC führt; nur die Fehlerbehandlung ist neu, die Rechnung gleich).
- Fft, Eigenzerlegung und Anschlagstufe der App liegen jetzt in `InstrumentCore` (KN2 a); `loop/Fft`,
  `loop/Eigenzerlegung` gibt es nicht mehr.

## Entschieden (34d)

- **FU1 a** Körper an der Schicht (`body`), nicht in der Tafel; **FU2 a** Dämpfer, Soft und Moderator in der Tafel, getreten
  über Spuren `soft`/`practice`, `keyboard` nur für die Planung; **FU3 a** der Creator zeigt die Kern-Schicht schon in 34d;
  **BK1 a** zwei Bibliotheken — `InstrumentCore` ohne Kern, `InstrumentModels` mit allen Kernen.

## Intern

- Tests **1765 Cases** (12 neu `*InstrumentCore*`); voller Lauf 1765 / 1765 unter Release-clang und Testing-MSVC,
  `Loop Werte`, `Klangfassung` und Flügel-Bezug unverändert; Rechenzeit Taste 40 in Ruhe 3,9–4,2 % wie vorher.
- Befund: der Schalter `/fp:contract-` aus IC kennt MSVC nicht (D9002) — in LumiViz entfernt, für IC vorgemerkt.
