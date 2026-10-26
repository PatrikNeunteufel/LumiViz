# Changelog Session 155 (2026-10-26)

> **Das Einstellen der Hammer-Saite an der Aufnahme ist neu aufgesetzt.** Statt einzelner abgeleiteter Zahlen
> (Abklingzeit, Knick, Schwebung), an denen die Suche in Session 154 hängen blieb, vergleicht LumiViz jetzt die ganzen
> Abklingkurven je Teilton mit *Salamander*. Die Grenzen der Suche kommen, wo es Messungen an Flügeln gibt, aus der
> Literatur statt aus der Aufnahme selbst. Eine Probe vor dem eigentlichen Einstellen läuft; eingestellt ist noch nichts,
> jedes Instrument klingt wie vorher.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (K1 a bis K6 a, K6-1 a, SQ1 a, SQ2 a, SW1 a, SE1 a).

## Neu

- **`docs/Bibliothek_offen.md`** — Quellen, die nur eine Bibliothek oder ein Kauf beschafft, mit Verlagsseite (DOI),
  Anlass und Reihenfolge; in `docs/INDEX.md` verlinkt.
- **`docs/modellierung/hammer-saite/grenzen.md`** — belegte Bereiche je Größe der Hammer-Saite (Verstimmung im Chor,
  zweite Schwingungsebene, Abklingen hoher Teiltöne) mit Fundstelle, gegen *Salamander* gestellt; dazu sieben
  Recherche-Berichte unter `recherche/2026-10-26_S155_bereich_*`.

## Intern

- Die Messung legt je Teilton die **Abklingkurve nach Schroeder** und die **Autokovarianz** der Schwebung ab; die
  bisherigen Messwerte bleiben Bit für Bit gleich.
- **`werte/salamander_kurven.txt`** (neu, 939 Kurven, 1,7 MB): die Kurven von *Salamander* je Taste, Stufe v6–v16 und
  Teilton 1–3, mit `LUMIVIZ_SALAMANDER` geschrieben.
- **Kurvenabstand** zwischen Modell und Aufnahme (`Kurvenabstand.hpp`, Tests `test_Kurvenabstand`).
- Werkzeug der Probe `LUMIVIZ_75_PROBE` (`test_LoopSalamanderProbe.cpp`); Nelder–Mead als eigener Header.
- Pfade der Original-Samples (OLPC, DrumGizmo, Clave, Akkorde) zeigen auf die Wechselplatte `E:\samples`; `/dld/` in
  der `.gitignore`.
- Release-clang: Teiltonmessung, Salamander, Kurvenabstand grün; **Testing-MSVC und der volle Lauf stehen seit S153
  aus.**
