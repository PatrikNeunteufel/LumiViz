# Changelog Session 147 (2026-10-23)

> **Der Resonanzboden wird konstruiert und gerechnet.** Statt einer gemessenen Antwort bekommt der Flügel einen
> Boden aus einem Bauplan — Umriss, Platte, Rippen, Steg, Holz — und LumiViz rechnet daraus seine Schwingungen. Eigene
> Böden (dicker, kleiner, aus Glas) werden damit Dateien. Gebaut sind Bauplan, Netz und Platte; Eigenlöser, Abstrahlung
> und Klang folgen.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (BM1 a bis BM5-1 a, PL31c a, HT7 b, GB1 a, IC1 a, EL1 a).

## Neu

- **Bauplan eines Instrumentkörpers** `.lvinstbodyplan`: Materialien (Holz mit Faser oder Werkstoffe ohne, etwa
  Glas), Platte mit Dicke je Stelle, Rippen und Stege, die Punkte, an denen die Saiten den Steg treffen. Mitgeliefert
  der **Steinway D** nach Chabassier & Duruflé (RT-0425), aus deren Abbildung ausgewertet; alle 84 Stegpunkte des
  Berichts liegen auf dem ausgewerteten Steg.
- **Zwei neue Datei-Icons**: Bauplan (Umriss mit Rippen und Steg) und Körper (Umriss mit einer Schwingungsform), im
  Benutzerhandbuch §17.
- **Netz und Platte** des Bodens: ein Dreiecksnetz, das Rippen und Steg folgt, und eine Platte nach Reissner-Mindlin
  mit Elementen höherer Ordnung. Gegen die geschlossene Lösung einer Rechteckplatte auf unter 0,1 % genau.

## Geändert

- Der Boden des Flügels kommt aus dem Modell; die Aufnahmen von *Salamander* dienen nur noch als Vergleich (HT7 b).

## Intern

- Bibliothek `projects/libs/InstrumentBody` (nur Header, ohne Qt): `Bauplan`, `Netz` (Schnittstelle `Netzgeber`),
  `NetzCdt`, `DuenneMatrix`, `Platte` (Schnittstelle `Plattenelement`, `LagrangeDreieck(k)`); Modul-Doku 0.3.0.
- `loop/KoerperBauplan`; `asset/loop/instrument_bodies/` mit `make_steinway_d.py` und README.
- Neue Externals, nur für die Tests (später das Werkzeug), nicht für die App: **CDT 2.0.1** (MPL-2.0), **Eigen 3.4.1**
  (MPL-2.0, `EIGEN_MPL2_ONLY`); `THIRD_PARTY_NOTICES.md` nachgezogen.
- Neue Tests `test_LoopKoerperBauplan` (7), `test_LoopKoerperNetz` (4, Bezug `werte/netz_steinway.txt`),
  `test_LoopKoerperPlatte` (7); 1538 Fälle. Unter Release-clang grün, Testing-MSVC baut; **kein voller Lauf**.
