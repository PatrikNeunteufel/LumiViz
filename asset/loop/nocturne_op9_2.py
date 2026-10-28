"""Nocturne Op. 9 Nr. 2 (Chopin) als Daten — von Hand uebertragen aus der Gesamtausgabe Breitkopf & Haertel 1880
(Bargiel, *Friedrich Chopin's Werke* Band IV, Plate C. IV. 2; IMSLP #113996), Takt fuer Takt mit Zoom an jeder
unsicheren Stelle, gegen die Erstausgabe Kistner 1833 (Plate 995; IMSLP #86550) geprueft (31d-9, 9.4, TR1 a,
S157–S158). Beide Ausgaben gemeinfrei. `make_fluegel_loops.py` macht daraus `nocturne_demo.lvloop`.

Einheiten: Schritte zu je einer Zweiunddreissigstel; ein 12/8-Takt hat 48 Schritte (Achtel = 4).
Je Takt: `rh`, `lh` = Liste (Anfang, Dauer, [Tonnamen klingend, mit Vorzeichen], optional ">" fuer einen Akzent);
Bruchteile sind erlaubt (Vorschlaege, Verzierungen, unregelmaessige Gruppen; N1 a). `ped` = Liste (Anfang, 1|0) nach
*Ped.* und ✱ (N3 a), oder "schlag" (nieder an jedem Schlag, hoch nach dem zweiten Achtel); `dyn` = Liste (Anfang,
Zeichen) (N2 a); `tempo` = Vortrags- und Tempoangaben fuer die Beschreibung und die Tempo-Spur (N4 a, TP1 a);
`takte` = so viele Loop-Takte belegt der Partiturtakt (Kadenz 32, Schluss 34; N8 a); `unsicher` = was die Lesung
offen laesst und wie es entschieden ist; `anm` = Bemerkungen zur Lesung.
Takt 0 ist der Auftakt (nur sein letztes Achtel klingt; N7 a).
"""

TAKTE = {
    0: dict(rh=[(44, 4, ["Bb4"])], lh=[], ped=[], dyn=[], anm="Auftakt; espress. dolce; keine Dynamik am Anfang"),
    1: dict(
        rh=[(0, 12, ["G5"]), (12, 4, ["G5"]), (16, 4, ["F5"]), (20, 4, ["G5"]), (24, 12, ["F5"]), (36, 8, ["Eb5"]),
            (44, 4, ["Bb4"])],
        lh=[(0, 4, ["Eb2"]), (4, 4, ["G3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "G4"]),
            (12, 4, ["Eb3"]), (16, 4, ["Ab3", "Eb4"]), (20, 4, ["Db4", "Eb4", "Bb4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["D2"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped="schlag",  # Ped. unter jedem Bass, ✱ unter dem dritten Achtel des Schlags
        dyn=[],
        anm="G5 im ersten Schlag neu angeschlagen (Bogen, kein Haltebogen); Bass Eb-Eb-Eb-D",
    ),
    2: dict(
        rh=[(0, 8, ["G5"]),
            # Doppelschlag ueber C5 (♮ oben und unten) nach VZ2 a: auf dem Schlag D5–C5–B♮4–C5
            (8, 1, ["D5"]), (9, 1, ["C5"]), (10, 1, ["B4"]), (11, 1, ["C5"]),
            (12, 8, ["C6"]), (20, 4, ["G5"]), (24, 12, ["Bb5"]), (36, 8, ["Ab5"]), (44, 4, ["G5"])],
        lh=[(0, 4, ["C2"]), (4, 4, ["G3", "E4"]), (8, 4, ["Bb3", "E4", "G4"]),
            (12, 4, ["C3"]), (16, 4, ["G3", "E4"]), (20, 4, ["C4", "E4", "Bb4"]),
            (24, 4, ["F2"]), (28, 4, ["F3", "Db4"]), (32, 4, ["Bb3", "Db4", "E4"]),
            (36, 4, ["F2"]), (40, 4, ["F3", "C4"]), (44, 4, ["Ab3", "C4", "F4"])],
        ped="schlag",
        dyn=[(8, "<")],
        anm="C7 (E♮) – f: vii°7 (Db, E♮) – f; Gabel < unter dem Doppelschlag",
    ),
    3: dict(
        rh=[(0, 12, ["F5"]), (12, 8, ["G5"]), (20, 4, ["D5"]), (24, 12, ["Eb5"]), (36, 12, ["C5"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "D4"]), (8, 4, ["Bb3", "D4", "Bb4"]),
            (12, 4, ["B2"]), (16, 4, ["G3", "F4"]), (20, 4, ["D4", "F4", "G4"]),
            (24, 4, ["C3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["C4", "Eb4", "G4"]),
            (36, 4, ["A2"]), (40, 4, ["Gb3", "Eb4"]), (44, 4, ["C4", "Eb4", "Gb4"])],
        ped="schlag",
        dyn=[],
        anm="Bb – G7/B♮ – c – A°7 (A♮, Gb); ♮ vor D4 in Schlag 1 (nach dem Db aus Takt 2)",
    ),
    4: dict(
        rh=[(0, 4, ["Bb4"]), (4, 4, ["D6"]), (8, 4, ["C6"]), (12, 2, ["Bb5"]), (14, 2, ["Ab5"]), (16, 2, ["G5"]),
            (18, 2, ["Ab5"]), (20, 2, ["C5"]), (22, 2, ["D5"]), (24, 12, ["Eb5"]), (44, 4, ["Bb4"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "Ab4"]),
            (12, 4, ["Ab1"]), (16, 4, ["F3", "D4"]), (20, 4, ["Bb3", "Ab4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(0, 1), (8, 0), (12, 1), (20, 0), (24, 1)],  # ab Schlag 3 gedrueckt bis zum ✱ in Takt 5
        dyn=[(0, "f")],
        anm="Vorhalt Eb4/Ab4 ueber Bb in Schlag 1, Loesung nach D4 ueber Ab1; Viertelpause in Schlag 4",
    ),
    5: dict(
        rh=[(0, 12, ["G5"]), (12, 2, ["F5"]), (14, 2, ["G5"]),
            # Pralltriller „w“ ueber F5 (VZ1 a): F5–G5–F5 auf dem Schlag, in der Zeit des Sechzehntels
            (16, 2 / 3, ["F5"]), (16 + 2 / 3, 2 / 3, ["G5"]), (16 + 4 / 3, 2 / 3, ["F5"]),
            (18, 2, ["E5"]), (20, 2, ["F5"]), (22, 2, ["G5"]),
            (24, 4, ["F5"]), (28, 8, ["Eb5"], ">"),
            (36, 2, ["Eb5"]), (38, 2, ["F5"]),
            (40, 2 / 3, ["Eb5"]), (40 + 2 / 3, 2 / 3, ["F5"]), (40 + 4 / 3, 2 / 3, ["Eb5"]),
            (42, 2, ["D5"]), (44, 2, ["Eb5"]), (46, 2, ["F5"])],
        lh=[(0, 4, ["Eb3"]), (4, 4, ["G3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "G4"]),
            (12, 4, ["Eb3"]), (16, 4, ["Ab3", "Eb4"]), (20, 4, ["Db4", "Eb4", "Bb4"]),
            (24, 4, ["Eb3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["D3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(8, 0), (12, 1), (20, 0), (24, 1), (32, 0), (36, 1), (44, 0)],
        dyn=[(0, "p")],
        anm="verzierte Wiederkehr von Takt 1; E♮5 in Schlag 2, ♭ vor dem E5 mit Akzent in Schlag 3; Bass eine "
            "Oktave hoeher als in Takt 1 (Eb3 mit Staccato); kein Ped. am Taktanfang (Pedal aus Takt 4 bis ✱)",
    ),
    6: dict(
        rh=[(0, 2, ["G5"]), (2, 2, ["B4"]), (4, 2, ["C5"]), (6, 2, ["Db5"], ">"), (8, 2, ["C5"]),
            (10, 2, ["F5"], ">"), (12, 2, ["E5"]), (14, 2, ["Ab5"], ">"), (16, 2, ["G5"]), (18, 2, ["Db6"]),
            (20, 2, ["C6"]), (22, 2, ["G5"]), (24, 12, ["Bb5"]), (36, 8, ["Ab5"]), (44, 4, ["G5"])],
        lh=[(0, 4, ["C3"]), (4, 4, ["G3", "E4"]), (8, 4, ["Bb3", "E4", "G4"]),
            (12, 4, ["C3"]), (16, 4, ["G3", "E4"]), (20, 4, ["C4", "E4", "Bb4"]),
            (24, 4, ["F2"]), (28, 4, ["F3", "Db4"]), (32, 4, ["Bb3", "Db4", "E4"]),
            (36, 4, ["F2"]), (40, 4, ["F3", "C4"]), (44, 4, ["Ab3", "C4", "F4"])],
        ped=[(0, 1), (20, 0), (24, 1), (32, 0), (36, 1), (44, 0)],
        dyn=[(6, "cresc."), (24, ">")],
        anm="verzierte Wiederkehr von Takt 2: Sechzehntel-Lauf (B♮4, E♮5 mit ♮; Db5, Db6 mit ♭), Akzente; "
            "Bass C3 statt C2; Ped. am Anfang bis ✱ unter Schlag 2",
    ),
    7: dict(
        rh=[  # Triller mit Vorschlag (VZ1 a): E♮5 auf dem Schlag, F5–G5 in Zweiunddreissigsteln, Nachschlag E♮5–F5
            (0, 1, ["E5"]), (1, 1, ["F5"]), (2, 1, ["G5"]), (3, 1, ["F5"]), (4, 1, ["G5"]), (5, 1, ["F5"]),
            (6, 1, ["G5"]), (7, 1, ["F5"]), (8, 1, ["G5"]), (9, 1, ["F5"]), (10, 1, ["E5"]), (11, 1, ["F5"]),
            (12, 4, ["G5"]), (16, 4, ["G5"]), (20, 4, ["D5"]), (24, 12, ["Eb5"]), (36, 12, ["C5"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "D4"]), (8, 4, ["Bb3", "D4", "Bb4"]),
            (12, 4, ["B2"]), (16, 4, ["G3", "F4"]), (20, 4, ["D4", "F4", "G4"]),
            (24, 4, ["C3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["C4", "Eb4", "G4"]),
            (36, 4, ["A2"]), (40, 4, ["Gb3", "Eb4"]), (44, 4, ["C4", "Eb4", "Gb4"])],
        ped="schlag",
        dyn=[],
        anm="Wiederkehr von Takt 3; Triller auf F5 mit Vorschlag ♮e und gedrucktem Nachschlag; G5 als zwei Achtel; "
            "linke Hand wie Takt 3",
    ),
    8: dict(
        rh=[(0, 4, ["Bb4"]), (4, 4, ["D6"]), (8, 4, ["C6"]), (12, 2, ["Bb5"]), (14, 2, ["Ab5"]), (16, 2, ["G5"]),
            (18, 2, ["Ab5"]),
            # Vorschlag Ab5 (durchgestrichen, gebunden) vor C5 — auf dem Schlag, zulasten der Hauptnote (VZ1 a)
            (20, 0.5, ["Ab5"]), (20.5, 1.5, ["C5"]), (22, 2, ["D5"]), (24, 12, ["Eb5"]),
            (36, 4, ["Eb5"]), (40, 4, ["D5"]), (44, 4, ["Eb5"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "Ab4"]),
            (12, 4, ["Ab1"]), (16, 4, ["F3", "D4"]), (20, 4, ["Bb3", "Ab4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(0, 1), (8, 0), (12, 1), (20, 0), (24, 1), (44, 0)],
        dyn=[(4, "<")],
        anm="Wiederkehr von Takt 4; Sechzehntel mit Staccato, Vorschlag Ab5 vor C5; Schluss E♭5 D5 E♭5 statt Pause; "
            "linke Hand wie Takt 4",
    ),
    9: dict(
        rh=[(0, 12, ["F5"]), (12, 8, ["G5"]), (20, 4, ["F5"]), (24, 12, ["F5"]), (36, 12, ["C5"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "D4"]), (8, 4, ["Bb3", "D4", "F4"]),
            (12, 4, ["Bb2"]), (16, 4, ["F3", "D4"]), (20, 4, ["Bb3", "D4", "F4"]),
            (24, 4, ["A2"]), (28, 4, ["F3", "C4"]), (32, 4, ["C4", "F4"]),
            (36, 4, ["A2"]), (40, 4, ["F3", "C4"]), (44, 4, ["C4", "F4"])],
        ped=[(0, 1), (20, 0), (24, 1), (44, 0)],
        dyn=[(0, "p"), (8, ">"), (24, "pp")],
        anm="Bb – F/A (A♮ mit ♮); Bass mit Staccato",
    ),
    10: dict(
        rh=[(0, 4, ["Eb5"]), (4, 4, ["Eb5"]), (8, 4, ["Eb5"]), (12, 4, ["Eb5"]),
            (16, 2, ["D5"]), (18, 2, ["Eb5"]), (20, 2, ["F5"]), (22, 2, ["Eb5"]),
            (24, 12, ["Eb5"]), (36, 12, ["Bb4"])],
        lh=[(0, 4, ["Ab2"]), (4, 4, ["Eb3", "C4"]), (8, 4, ["Ab3", "C4", "Eb4"]),
            (12, 4, ["Ab1"]), (16, 4, ["Eb3", "Cb4"]), (20, 4, ["Ab3", "Cb4", "Eb4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(0, 1), (20, 0), (24, 1), (44, 0)],
        dyn=[],
        tempo=[(0, "poco ritard.")],
        anm="Ab – ab (Cb4 mit ♭) – Eb; Eb5 in Achteln mit Staccato; poco ritard. (fuer die Tempo-Spur, N4 a)",
    ),
    11: dict(
        rh=[(0, 12, ["Bb5"]), (12, 8, ["A5"]), (20, 4, ["G5"]), (24, 12, ["F5", "A4"]), (36, 12, ["D5", "Bb4"])],
        lh=[(0, 4, ["E2"]), (4, 4, ["E3", "Db4"]), (8, 4, ["Bb3", "Db4", "G4"]),
            (12, 4, ["E2"]), (16, 4, ["E3", "C4"]), (20, 4, ["Bb3", "C4", "G4"]),
            (24, 4, ["F2"]), (28, 4, ["F3", "Eb4"]), (32, 4, ["C4", "Eb4", "F4"]),
            (36, 4, ["G2"]), (40, 4, ["G3", "D4"]), (44, 4, ["Bb3", "D4", "G4"])],
        ped="schlag",
        dyn=[(0, "f")],
        tempo=[(0, "Tempo I")],
        anm="E°7 (E♮2, Db4) – C7/E – F7 – g (D♮4); rechts ab Schlag 3 zweistimmig (F5/A♮4, D5/Bb4), Achtelpause "
            "der Unterstimme in Schlag 2",
    ),
    12: dict(
        rh=[(0, 12, ["Eb5", "G4"]), (12, 4, ["D5", "A4"]), (16, 4, ["C5", "A4"]), (20, 4, ["D5", "A4"]),
            (24, 4, ["Bb4", "F4"]), (28, 4, ["B4", "F#4"]), (32, 4, ["B4"]), (32, 8, ["E4"]),
            (36, 4, ["Bb4", "C5"]), (40, 4, ["F4", "A4", "C5"]), (44, 4, ["Ab4", "D5"])],
        lh=[(0, 4, ["C2"]), (4, 4, ["G3", "Eb4"]), (8, 4, ["C4", "Eb4", "G4"]),
            (12, 4, ["F2"]), (16, 4, ["F3", "Eb4"]), (20, 4, ["C4", "Eb4", "F4"]),
            (24, 4, ["Bb3", "D4"]), (28, 4, ["A3", "D#4"]), (32, 4, ["G#3"]),
            (36, 4, ["G3"]), (40, 4, ["F3", "C4", "Eb4"]), (44, 4, ["Bb3", "F4"])],
        ped=[(0, 1), (8, 0), (12, 1), (20, 0)],
        dyn=[],
        tempo=[(24, "poco rallent.")],
        unsicher="Schlag 3–4 (chromatische Kette) aus der Vorlage nicht eindeutig; gegen die Erstausgabe (Kistner, "
                 "Seite 6 Zeile 4) gelesen — dort klar: rechts Bb/F, B♮/F♯, B♮/E♮ (E♮ gebunden), Bb+C5, F A♮ C, "
                 "Ab/D; links Bb/D, A♮/D♯, G♯, G♮, F/C/Eb, Bb/F",
        anm="c – F7 – chromatisch ueber dem Bass Bb–A–G♯–G–F–Bb (B7, E/G♯, C7/G, F7, Bb7); rechts in Schlag 2 A♮4; "
            "in Schlag 3–4 kein Pedal",
    ),
    13: dict(
        rh=[  # Vorschlaege Bb4, Eb5 vor G5 — auf dem Schlag (VZ1 a)
            (0, 0.5, ["Bb4"]), (0.5, 0.5, ["Eb5"]), (1, 11, ["G5"]),
            # Vorschlaege A♮4, Bb4 vor Cb5
            (12, 0.5, ["A4"]), (12.5, 0.5, ["Bb4"]), (13, 1, ["Cb5"]), (14, 2, ["Bb4"]), (16, 2, ["C5"]),
            (18, 2, ["D5"]), (20, 3, ["G5"]), (23, 1, ["F5"]),
            (24, 8, ["F5"]), (32, 6, ["Eb5"]),  # Eb5 an 32 gebunden ueber den Schlag
            (38, 2, ["F5"]), (40, 2 / 3, ["Eb5"]), (40 + 2 / 3, 2 / 3, ["F5"]), (40 + 4 / 3, 2 / 3, ["Eb5"]),
            (42, 2, ["D5"]), (44, 2, ["Eb5"]), (46, 2, ["F5"])],
        lh=[(0, 4, ["Eb2"]), (4, 4, ["G3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "G4"]),
            (12, 4, ["Eb3"]), (16, 4, ["Ab3", "D4"]), (20, 4, ["Bb3", "D4", "Ab4"]),
            (24, 4, ["Eb3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["D3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(0, 1), (8, 0)],
        dyn=[(0, "sfp")],
        tempo=[(0, "Tempo I")],
        unsicher="rechts Schlag 1: G5 als punktierte Viertel gelesen (kein Punkt sichtbar, der Takt verlangt ihn); "
                 "links Schlag 2 D4 statt Eb4 (Hoehe knapp, Harmonie Bb7 ueber Eb)",
        anm="verzierte Wiederkehr von Takt 1/5; Vorschlaege (zweimal zwei Sechzehntel), Lauf Cb5 Bb4 C♮5 D5, "
            "Pralltriller in Schlag 4; links Bb7 ueber Eb in Schlag 2",
    ),
    14: dict(
        rh=[(0, 2, ["G5"]), (2, 2, ["B4"]), (4, 2, ["C5"]), (6, 2, ["Db5"]), (8, 2, ["C5"]),
            (10, 2, ["F5"]), (12, 2, ["E5"]), (14, 2, ["Ab5"]), (16, 2, ["G5"]), (18, 2, ["Db6"]),
            (20, 2, ["C6"]), (22, 2, ["G5"]), (24, 12, ["Bb5"]), (36, 8, ["Ab5"]), (44, 4, ["G5"])],
        lh=[(0, 4, ["C3"]), (4, 4, ["G3", "E4"]), (8, 4, ["Bb3", "E4", "G4"]),
            (12, 4, ["C3"]), (16, 4, ["G3", "E4"]), (20, 4, ["C4", "E4", "Bb4"]),
            (24, 4, ["F2"]), (28, 4, ["F3", "Db4"]), (32, 4, ["Bb3", "Db4", "E4"]),
            (36, 4, ["F2"]), (40, 4, ["F3", "C4"]), (44, 4, ["Ab3", "C4", "F4"])],
        ped=[],
        dyn=[(0, "<"), (10, "cresc."), (30, ">")],
        anm="wie Takt 6 (ohne Akzente, mit Boegen); kein Pedal gedruckt",
    ),
    15: dict(
        rh=[(0, 1, ["E5"]), (1, 1, ["F5"]), (2, 1, ["G5"]), (3, 1, ["F5"]), (4, 1, ["G5"]), (5, 1, ["F5"]),
            (6, 1, ["G5"]), (7, 1, ["F5"]), (8, 1, ["G5"]), (9, 1, ["F5"]), (10, 1, ["E5"]), (11, 1, ["F5"]),
            (12, 4, ["G5"]), (16, 4, ["G5"], ">"), (20, 4, ["D5"]), (24, 12, ["Eb5"]), (36, 12, ["C5"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "D4"]), (8, 4, ["Bb3", "D4", "Bb4"]),
            (12, 4, ["B2"]), (16, 4, ["G3", "F4"]), (20, 4, ["D4", "F4", "G4"]),
            (24, 4, ["C3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["C4", "Eb4", "G4"]),
            (36, 4, ["A2"]), (40, 4, ["Gb3", "Eb4"]), (44, 4, ["C4", "Eb4", "Gb4"])],
        ped=[],
        dyn=[(0, "p")],
        unsicher="kein Pedal gedruckt (S158 nachgesehen; zuvor wie Takt 7 angenommen); das ♮ vor F3/D4 in Schlag 1 "
                 "hebt das Db4 aus Takt 14 auf (S158, an Takt 23 gesehen: dort ebenso nach dem Db4 aus Takt 22)",
        anm="Wiederkehr von Takt 7 (Triller mit Vorschlag e♮², VZ1 a); Akzent auf dem zweiten G5",
    ),
    16: dict(
        rh=[(0, 4, ["Bb4"]), (4, 4, ["D6"]), (8, 4, ["C#6"])]
           # Lauf: 13 Zweiunddreissigstel im Raum von 12 (Schlag 2), gleichmaessig verteilt
           + [(12 + i * 12 / 13, 12 / 13, [ton]) for i, ton in enumerate(
               ["C6", "B5", "Bb5", "A5", "Ab5", "F5", "D5", "Cb5", "Bb4", "D5", "G5", "F5", "Eb5"])]
           + [(24, 12, ["Eb5"]), (36, 4, ["Eb5"]), (40, 4, ["D5"]), (44, 4, ["Eb5"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "Ab4"]),
            (12, 4, ["Ab1"]), (16, 4, ["F3", "D4"]), (20, 4, ["Bb3", "D4", "Ab4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[],
        dyn=[(18, ">")],
        unsicher="Schlag 4: Bogen vom Eb5 (Schlag 3) zum Eb5 auf 36 — wie in Takt 8 als neu angeschlagen gelesen, "
                 "nicht als Haltebogen; Schlag 4 links Bb3 Eb4 G4 (Eb4 im Bild nicht erkannt, nach Takt 8); "
                 "Gabel > ueber dem Lauf nur in der Gesamtausgabe, Lage geschaetzt",
        anm="Wiederkehr von Takt 4/8 mit Lauf: Bb4 D6 C♯6 als Achtel, dann chromatisch C♮6 B♮5 B♭5 A♮5 A♭5 und Bb7 "
            "abwaerts mit C♭5 (♭ in beiden Ausgaben), 13 Noten im Schlag (Kistner zaehlt ebenso); kein Pedal gedruckt; "
            "Lesehilfe ueber dem System hier geeicht an D6 (Takt 4/8): gemessen rund 10 px zu hoch",
    ),
    17: dict(
        rh=[(0, 12, ["F5"]), (12, 8, ["G5"]), (20, 4, ["F5"]), (24, 12, ["F5"]), (36, 12, ["C5"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "D4"]), (8, 4, ["Bb3", "D4", "F4"]),
            (12, 4, ["Bb2"]), (16, 4, ["F3", "D4"]), (20, 4, ["Bb3", "D4", "F4"]),
            (24, 4, ["A2"]), (28, 4, ["F3", "C4"]), (32, 4, ["C4", "F4"]),
            (36, 4, ["A2"]), (40, 4, ["F3", "C4"]), (44, 4, ["C4", "F4"])],
        ped=[],
        dyn=[(0, "p")],
        anm="Wiederkehr von Takt 9 (Bb – F/A, A♮2 mit ♮); gedruckt nur p — kein >, kein pp, kein Pedal",
    ),
    18: dict(
        rh=[  # Quartole: vier Achtel im Raum von dreien („4“ in beiden Ausgaben)
            (0, 3, ["Eb5"]), (3, 3, ["Eb5"]), (6, 3, ["Eb5"]), (9, 3, ["Eb5"]),
            (12, 4, ["Eb5"]), (16, 2, ["D5"]), (18, 2, ["Eb5"]), (20, 3, ["F5"]), (23, 1, ["Eb5"]),
            (24, 12, ["Eb5"]), (36, 12, ["Bb4"])],
        lh=[(0, 4, ["Ab2"]), (4, 4, ["Eb3", "C4"]), (8, 4, ["Ab3", "C4", "Eb4"]),
            (12, 4, ["Ab1"]), (16, 4, ["Eb3", "Cb4"]), (20, 4, ["Ab3", "Cb4", "Eb4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[],
        dyn=[(36, "<")],
        anm="Wiederkehr von Takt 10, verziert: Quartole Eb5, im zweiten Schlag F5 punktiert mit Eb5 als "
            "Zweiunddreissigstel (Kistner gleich); links wie Takt 10 (Cb4 mit ♭); kein poco ritard., kein Pedal",
    ),
    19: dict(
        rh=[(0, 12, ["Bb5"]), (12, 8, ["A5"]), (20, 4, ["G5"]), (24, 12, ["F5", "A4"]), (36, 12, ["D5", "Bb4"])],
        lh=[(0, 4, ["E2"]), (4, 4, ["E3", "Db4"]), (8, 4, ["Bb3", "Db4", "G4"]),
            (12, 4, ["E2"]), (16, 4, ["E3", "C4"]), (20, 4, ["Bb3", "C4", "G4"]),
            (24, 4, ["F2"]), (28, 4, ["F3", "Eb4"]), (32, 4, ["C4", "Eb4", "A4"]),
            (36, 4, ["G2"]), (40, 4, ["G3", "D4"]), (44, 4, ["Bb3", "D4", "Bb4"])],
        ped=[],
        dyn=[(0, "f"), (24, ">")],
        anm="Wiederkehr von Takt 11; links Schlag 3 C4 Eb4 A♮4 (♮ in beiden Ausgaben, Takt 11: F4), Schlag 4 "
            "Bb3 D4 Bb4 (Takt 11: G4); Gabel > ueber Schlag 3; kein Pedal gedruckt",
    ),
    20: dict(
        rh=[(0, 12, ["Eb5", "G4"]), (12, 4, ["D5", "A4"]), (16, 4, ["C5", "A4"]), (20, 4, ["D5", "A4"]),
            (24, 4, ["Bb4", "F4"]), (28, 4, ["B4", "F#4"]), (32, 4, ["B4"]), (32, 8, ["E4"]),
            (36, 4, ["Bb4", "C5"]), (40, 4, ["F4", "A4", "C5"]), (44, 4, ["Ab4", "D5"])],
        lh=[(0, 4, ["C2"]), (4, 4, ["G3", "Eb4"]), (8, 4, ["C4", "Eb4", "G4"]),
            (12, 4, ["F2"]), (16, 4, ["F3", "C4"]), (20, 4, ["C4", "Eb4", "F4"]),
            (24, 4, ["Bb3", "D4"]), (28, 4, ["A3", "D#4"]), (32, 4, ["G#3"]),
            (36, 4, ["G3"]), (40, 4, ["F3", "C4", "Eb4"]), (44, 4, ["Bb3", "F4"])],
        ped=[],
        dyn=[],
        tempo=[(24, "poco rall.")],
        unsicher="links Schlag 2 erster Akkord: in der Gesamtausgabe F3 C4 (am Kopf gemessen, gleiche Hoehe wie das "
                 "C4 des naechsten Akkords), Kistner und Takt 12 haben F3 Eb4 — die Vorlage gilt (UF15 a)",
        anm="Wiederkehr von Takt 12 (chromatische Kette, gleiche Lesung wie dort nach Kistner); kein Pedal gedruckt",
    ),
    21: dict(
        rh=[  # Vorschlaege Bb4, Eb5 vor G5 — auf dem Schlag (VZ1 a), wie Takt 13
            (0, 0.5, ["Bb4"]), (0.5, 0.5, ["Eb5"]), (1, 11, ["G5"]),
            # Vorschlaege A♮4, Bb4 vor Cb5 (Akzent)
            (12, 0.5, ["A4"]), (12.5, 0.5, ["Bb4"]), (13, 1, ["Cb5"], ">"), (14, 2, ["Bb4"]), (16, 2, ["C#5"]),
            (18, 2, ["D5"]), (20, 3, ["G5"], ">"), (23, 1, ["F5"]),
            (24, 8, ["F5"]), (32, 6, ["Eb5"]),  # Eb5 an 32 gebunden ueber den Schlag
            # tr auf dem Sechzehntel: wie der Pralltriller in Takt 13 (VZ1 a)
            (38, 2, ["F5"]), (40, 2 / 3, ["Eb5"]), (40 + 2 / 3, 2 / 3, ["F5"]), (40 + 4 / 3, 2 / 3, ["Eb5"]),
            (42, 2, ["D5"]), (44, 2, ["Eb5"]), (46, 2, ["F5"])],
        lh=[(0, 4, ["Eb2"]), (4, 4, ["G3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "G4"]),
            (12, 4, ["Eb3"]), (16, 4, ["Ab3", "D4"]), (20, 4, ["Bb3", "D4", "Ab4"]),
            (24, 4, ["Eb3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["D3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[],
        dyn=[(0, "sfp"), (24, ">"), (40, "<")],
        tempo=[(0, "Tempo I")],
        anm="Wiederkehr von Takt 13 mit C♯5 (♯) statt C♮5 im Lauf, Akzenten auf Cb5 und G5, Gabeln > (Schlag 3) "
            "und < (Schlag 4); tr ueber dem Eb5 auf 40 statt des Pralltrillers; links wie Takt 13; kein Pedal",
    ),
    22: dict(
        rh=[(0, 2, ["G5"]), (2, 2, ["B4"]), (4, 2, ["C5"]), (6, 2, ["Db5"], ">"), (8, 2, ["C5"]),
            (10, 2, ["F5"], ">"), (12, 2, ["E5"]), (14, 2, ["Ab5"], ">"), (16, 2, ["G5"]), (18, 2, ["Db6"]),
            (20, 2, ["C6"]), (22, 2, ["G5"]), (24, 12, ["Bb5"]), (36, 8, ["Ab5"]), (44, 4, ["G5"])],
        lh=[(0, 4, ["C3"]), (4, 4, ["G3", "E4"]), (8, 4, ["Bb3", "E4", "G4"]),
            (12, 4, ["C3"]), (16, 4, ["G3", "E4"]), (20, 4, ["C4", "E4", "Bb4"]),
            (24, 4, ["F2"]), (28, 4, ["F3", "Bb3", "Db4"]), (32, 4, ["Bb3", "Db4", "E4"]),
            (36, 4, ["F2"]), (40, 4, ["F3", "C4"]), (44, 4, ["Ab3", "C4", "F4"])],
        ped=[],
        dyn=[(12, "<")],
        unsicher="links Schlag 3, zweites Achtel F3 Bb3 Db4 (Takt 14: F3 Db4) — Kistner gleich; das dritte Achtel "
                 "setzt Kistner anders (C4 E♮4 F4?), die Vorlage gilt",
        anm="Wiederkehr von Takt 14/6: Sechzehntel-Lauf (B♮4, E♮5 mit ♮; Db5, Db6 mit ♭) mit Akzenten wie Takt 6, "
            "Gabel < unter Schlag 2 bis 3; kein Pedal gedruckt",
    ),
    23: dict(
        rh=[  # Triller mit Vorschlag E♮5 wie Takt 7/15 (VZ1 a)
            (0, 1, ["E5"]), (1, 1, ["F5"]), (2, 1, ["G5"]), (3, 1, ["F5"]), (4, 1, ["G5"]), (5, 1, ["F5"]),
            (6, 1, ["G5"]), (7, 1, ["F5"]), (8, 1, ["G5"]), (9, 1, ["F5"]), (10, 1, ["E5"]), (11, 1, ["F5"]),
            (12, 4, ["G5"]), (16, 4, ["G5"]), (20, 4, ["D5"]), (24, 12, ["Eb5"]), (36, 12, ["C5"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "D4"]), (8, 4, ["Bb3", "D4", "Bb4"]),
            (12, 4, ["B2"]), (16, 4, ["G3", "F4"]), (20, 4, ["D4", "F4", "G4"]),
            (24, 4, ["C3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["C4", "Eb4", "G4"]),
            (36, 4, ["A2"]), (40, 4, ["Gb3", "Eb4"]), (44, 4, ["C4", "Eb4", "Gb4"])],
        ped=[],
        dyn=[],
        unsicher="der gedruckte Nachschlag des Trillers (zwei kleine Koepfe) ist schwer zu lesen — wie Takt 7/15 als "
                 "E♮5–F5 ausgeschrieben; links Schlag 1 oben Bb4 wie Takt 3/7/15 (Hoehe knapp zwischen A4 und Bb4)",
        anm="Wiederkehr von Takt 15; ♮ vor D4 in Schlag 1 nach dem Db4 aus Takt 22; ohne p und ohne Akzent; kein Pedal",
    ),
    24: dict(
        rh=[(0, 2, ["Bb4"]), (2, 2, ["D6"]), (4, 2, ["C#6"]), (6, 2, ["C6"]), (8, 2, ["B5"]), (10, 2, ["Bb5"]),
            (12, 2, ["A5"]), (14, 2, ["Ab5"]),
            (16, 1, ["A4"]), (17, 1, ["Bb4"]), (18, 1, ["B4"]), (19, 1, ["C5"]), (20, 1, ["C#5"]), (21, 1, ["D5"]),
            # Triole aus Zweiunddreissigsteln: drei im Raum von zweien
            (22, 2 / 3, ["G5"]), (22 + 2 / 3, 2 / 3, ["F5"]), (22 + 4 / 3, 2 / 3, ["Eb5"]),
            (24, 24, ["Eb5"], ">")],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "Ab4"]),
            (12, 4, ["Ab1"]), (16, 4, ["F3", "D4"]), (20, 4, ["Bb3", "D4", "Ab4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[],
        dyn=[],
        unsicher="die Gruppierung des Laufs aus den Balken erschlossen (Schlag 1 sechs Sechzehntel, Schlag 2 zwei "
                 "Sechzehntel, sechs Zweiunddreissigstel, Triole „3“); Kistner bestaetigt alle Tonhoehen samt B♮4 und "
                 "die Triole, balkt Schlag 1–2 etwas anders (A♭5 im Zweiunddreissigstel-Aufgang)",
        anm="Wiederkehr von Takt 16 mit anderem Lauf: chromatisch ab D6 hinunter bis A♭5, dann chromatisch von A♮4 "
            "hinauf bis D5 und Triole G5 F5 Eb5; Eb5 als punktierte Halbe mit Akzent (Ende des Abschnitts); links wie "
            "Takt 16; kein Pedal gedruckt",
    ),
    25: dict(
        rh=[(0, 12, ["Eb5"]), (12, 4, ["F5"]), (16, 4, ["Eb5"]), (20, 4, ["F5"]), (24, 24, ["G5"])],
        lh=[(0, 4, ["Eb2"]), (4, 4, ["Ab3", "C4", "Eb4"]), (8, 4, ["C4", "Eb4", "Ab4"]),
            (12, 4, ["Eb3"]), (16, 4, ["Ab3", "C4", "Eb4"]), (20, 4, ["C4", "Eb4", "Ab4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(0, 1), (20, 0), (24, 1), (44, 0)],
        dyn=[(0, "p")],
        anm="Beginn des Schlusses: Ab/Eb – Eb ueber Orgelpunkt Eb; G5 als punktierte Halbe (hohler Kopf, im Bild "
            "nicht erkannt); Ped. auf Schlag 1 und 3, ✱ vor dem dritten Achtel des Schlags",
    ),
    26: dict(
        rh=[(0, 12, ["Eb5"]), (12, 2, ["Eb5"]), (14, 2, ["F5"]), (16, 2, ["Eb5"]), (18, 2, ["F5"]),
            (20, 2, ["Eb5"]), (22, 2, ["F5"]), (24, 8, ["G5"]), (32, 4, ["Eb5"]),
            (36, 4, ["Eb6"]), (40, 4, ["D6"]), (44, 4, ["C6"])],
        lh=[(0, 4, ["Eb2"]), (4, 4, ["Ab3", "C4", "Eb4"]), (8, 4, ["C4", "Eb4", "Ab4"]),
            (12, 4, ["Eb3"]), (16, 4, ["Ab3", "C4", "Eb4"]), (20, 4, ["C4", "Eb4", "Ab4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Bb3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(0, 1), (20, 0), (24, 1), (44, 0)],
        dyn=[(0, "pp")],
        tempo=[(24, "poco rubato")],
        unsicher="rechts G5 auf 24 als Viertel, Eb5 auf 32 als Achtel gelesen (Haelse, kein Faehnchen sichtbar); "
                 "die „3“ ueber Eb6 D6 C6 als Fingersatz gelesen",
        anm="Wiederholung von Takt 25 im pp, Schlag 2 als Sechzehntel Eb5–F5; links Schlag 3 G3 Bb3 Eb4; Pedal wie "
            "Takt 25",
    ),
    27: dict(
        rh=[(0, 12, ["Bb5"]),
            # Vorschlag A♮5 (klein, mit ♮) vor Ab5 — auf dem Schlag (VZ1 a)
            (12, 0.5, ["A5"]), (12.5, 3.5, ["Ab5"]), (16, 4, ["C5"]), (20, 4, ["D5"]),
            (24, 4, ["Eb5"]), (28, 2, ["F5"]),
            # Doppelschlag ueber Eb5 (VZ2 a): F5–Eb5–D5–Eb5 auf dem Schlag, in der Zeit des Sechzehntels
            (30, 0.5, ["F5"]), (30.5, 0.5, ["Eb5"]), (31, 0.5, ["D5"]), (31.5, 0.5, ["Eb5"]),
            (32, 2, ["D5"]), (34, 2, ["Eb5"]),
            (36, 4, ["G6"]), (40, 2, ["F6"]), (42, 2, ["Eb6"]), (44, 2, ["D6"]), (46, 2, ["C6"])],
        lh=[(0, 4, ["Eb3"]), (4, 4, ["F3", "D4"]), (8, 4, ["Bb3", "Ab4"]),
            (12, 4, ["Eb3"]), (16, 4, ["F3", "D4"]), (20, 4, ["Bb3", "Ab4"]),
            (24, 4, ["Eb3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "G4"]),
            (36, 4, ["A2"]), (40, 4, ["F3", "Eb4"]), (44, 4, ["C4", "Eb4", "F4"])],
        ped=[(0, 1), (20, 0), (24, 1), (32, 0), (36, 1), (44, 0)],
        dyn=[(0, "pp")],
        tempo=[(0, "sempre pp"), (36, "dolcissimo")],
        unsicher="Vorschlag vor Ab5 als A♮5 gelesen (kleiner Kopf mit ♮, Hoehe zwischen A5 und B5); Doppelschlag-"
                 "Zeichen steht ueber dem Eb5 auf 30 (nicht ueber dem F5); Schlag 4 rechts G6 als Achtel, dann vier "
                 "Sechzehntel",
        anm="Bb7 ueber Orgelpunkt Eb (Eb3 – F3 D4 – Bb3 Ab4), Schlag 4 F7/A (A♮2); sempre pp, dolcissimo auf "
            "Schlag 4; Ped. auf jedem Schlag ausser 2, ✱ vor dem dritten Achtel; Vorschlag A♮5 in Kistner bestaetigt",
    ),
    28: dict(
        rh=[(0, 4, ["Cb6"]), (4, 4, ["Bb5"]), (8, 4, ["Bbb5"]),
            (12, 2, ["Cb6"]), (14, 2, ["Bb5"]), (16, 2, ["Bb5"]), (18, 2, ["Ab5"]), (20, 3, ["Ab5"]), (23, 1, ["G5"]),
            (24, 24, ["Eb5"], ">")],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "Ab4"]),
            (12, 4, ["Bb1"]), (16, 4, ["F3", "D4"]), (20, 4, ["Bb3", "D4", "Ab4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(0, 1), (8, 0), (12, 1), (20, 0), (24, 1), (44, 0)],
        dyn=[],
        unsicher="links Schlag 2 Bb1 gelesen (Takt 16: Ab1), Hoehe unter drei Hilfslinien",
        anm="Cb6 Bb5 B𝄫5 (Doppel-♭ in Kistner deutlich, in der Gesamtausgabe verschmiert), Seufzer in Sechzehnteln, "
            "Ab5 punktiert mit G5, Eb5 punktierte Halbe mit Akzent; links Bb7 – Eb wie Takt 16; Ped./✱ je Schlag",
    ),
    29: dict(
        rh=[(0, 12, ["Eb5"])]
           # acht Sechzehntel im Raum von sechsen (Kistner: zwei Balken, keine Zahl), gleichmaessig verteilt
           + [(12 + i * 1.5, 1.5, ["Eb5" if i % 2 == 0 else "F5"]) for i in range(8)]
           + [(24, 12, ["G5"])],  # danach Viertel- und Achtelpause
        lh=[(0, 4, ["Ab1"]), (4, 4, ["Eb3", "Ab3", "C4"]), (8, 4, ["Ab3", "C4", "Eb4"]),
            (12, 4, ["C4", "Eb4", "Ab4"]), (16, 4, ["Ab3", "C4", "Eb4"]), (20, 4, ["Eb3", "Ab3", "C4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["Eb3"]), (40, 4, ["G3", "Eb4"]), (44, 4, ["Bb3", "Eb4", "G4"])],
        ped=[(0, 1), (20, 0), (24, 1), (44, 0)],
        dyn=[(0, "p"), (20, "<")],
        anm="Ab/Eb: links Ab1 und Akkorde auf und ab (Schlag 2 ohne Bass), dann Eb; rechts Eb5, acht Sechzehntel "
            "Eb–F im Raum von sechsen (in beiden Ausgaben 8 Noten), G5 punktiert, Viertel- und Achtelpause (Kistner "
            "gleich); Gabel < zum Ende von Schlag 2",
    ),
    30: dict(
        rh=[(0, 4, ["Eb5"]), (4, 2, ["Ab4"]), (6, 2, ["Bb4"]), (8, 2, ["Ab4"]), (10, 2, ["G4"]),
            (12, 2, ["Ab4"]), (14, 2, ["Cb5"]), (16, 2, ["Eb5"]), (18, 2, ["Ab5"]), (20, 2, ["Eb6"]),
            # G6 vor dem 8va, mit Bogen zum G6 unter 8va (gleiche Hoehe): als ein Ton bis 28
            (22, 6, ["G6"]),
            # ab hier 8va (klingend notiert)
            (28, 4, ["Eb6"]), (32, 8, ["Eb6", "Eb7"], ">"), (40, 4, ["D6", "D7"]),
            (44, 4, ["C6", "C7"])],
        lh=[(0, 4, ["Ab1"]), (4, 4, ["Eb3", "Ab3", "C4"]), (8, 4, ["Ab3", "C4", "Eb4"]),
            (12, 4, ["Eb3", "Ab3", "C4"]), (16, 4, ["Ab3", "C4", "Eb4"]), (20, 4, ["Eb3", "Ab3", "C4"]),
            (24, 4, ["Eb2"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["Bb3", "Eb4", "G4"]),
            (36, 4, ["A2"]), (40, 4, ["F3", "Eb4"]), (44, 4, ["C4", "Eb4", "F4"])],
        ped=[(24, 1), (32, 0), (36, 1), (44, 0)],
        dyn=[],
        tempo=[(0, "con forza"), (36, "stretto")],
        unsicher="vor G6 steht eine Sechzehntelpause (Kistner deutlich; zuerst als drei Vorschlaege verlesen, S158 "
                 "berichtigt) — sie gehoert zur Unterstimme, G6 auf 22; Kistner traegt in Schlag 1 ein tr, die "
                 "Gesamtausgabe nicht; links Schlag 2 Akkordfolge wie Takt 29 angenommen; "
                 "con forza ist kein Buchstabe der Tafel (N2 a) — die Stufe bleibt p aus Takt 29, nach der Hoerprobe "
                 "entscheiden",
        anm="Ab – Eb – F7/A wie Takt 27; rechts Arpeggio bis Eb6, G6 mit Vorschlaegen, dann 8va: G6 Eb6, Oktaven "
            "Eb6/Eb7 (Akzent), D6/D7, C♮6/C♮7 (♮ hebt das Cb aus Schlag 2 auf); stretto auf Schlag 4; Ped. nur in "
            "Schlag 3 und 4",
    ),
    31: dict(
        rh=[  # ganzer Takt 8va, klingend notiert: Oktaven
            (0, 4, ["Cb6", "Cb7"]), (4, 4, ["Bb5", "Bb6"]), (8, 4, ["Bbb5", "Bbb6"]),
            (12, 4, ["Ab5", "Ab6"]), (16, 4, ["G5", "G6"]), (20, 4, ["D5", "D6"]),
            (24, 4, ["Eb5", "Eb6"]), (28, 12, ["Eb6", "Eb7"], ">"), (40, 4, ["F5", "F6"]), (44, 4, ["C6", "C7"])],
        lh=[(0, 4, ["Bb2"]), (4, 4, ["F3", "Bb3", "Eb4"]), (8, 4, ["Bb3", "Eb4", "Ab4"]),
            (12, 4, ["B2"]), (16, 4, ["G3", "F4"]), (20, 4, ["D4", "F4", "G4"]),
            (24, 4, ["C3"]), (28, 4, ["G3", "Eb4"]), (32, 4, ["C4", "Eb4", "G4"]),
            (36, 4, ["A2"]), (40, 4, ["F3", "C4", "Eb4"]), (44, 4, ["C4", "Eb4", "F4"])],
        ped=[(0, 1), (8, 0), (12, 1), (20, 0), (24, 1), (32, 0), (36, 1), (44, 0)],
        dyn=[],
        unsicher="rechts Schlag 2 drittes Achtel als Oktave D gelesen (die untere Note liegt knapp zwischen D und E; "
                 "die Harmonie G7/B traegt D); links Schlag 4 F3 C4 Eb4 und C4 Eb4 F4 (F7/A)",
        anm="8va ueber dem ganzen Takt; Oktaven Cb Bb B𝄫 (wie Takt 28) Ab G D, Eb, Eb punktiert mit Akzent, F, C♮; "
            "links Bb – G7/B (B♮2) – c – F7/A (A♮2) wie Takt 15/23; Ped./✱ auf jedem Schlag",
    ),
}


def _kadenz():
    """Takt 32, Kadenz senza Tempo (N8 a): in notierten Werten 216 Schritte — Akkord mit Arpeggio (32), Akkord mit
    Fermate (24, doppelt: 48), 15 Vierergruppen Sechzehntel (120), vier Achtel (16); aufgerundet auf 5 Takte (240),
    alles gleichmaessig gestreckt (Faktor 240/216). Rechts bis zu den vier Achteln 8va, klingend notiert."""
    k = 240 / 216
    rh = [(0, 32, ["Cb6", "Cb7"]), (32, 48, ["Bb5", "Cb7"])]
    lh = [(i * 4, 4, [t]) for i, t in enumerate(["Bb1", "Bb2", "F3", "Ab3", "D4", "Bb3", "F4", "Ab4"])]
    lh.append((32, 48, ["D5"]))
    gruppen = [["Cb7", "Bb6", "C7", "A6"]] * 12 + [["Cb7", "Bb6", "D7", "C7"], ["Bb6", "A6", "Ab6", "G6"],
                                                    ["F6", "D6", "Eb6", "C6"]]
    s = 80
    for gruppe in gruppen:
        for ton in gruppe:
            rh.append((s, 2, [ton]))
            s += 2
    for ton in ["Bb5", "Ab5", "C5", "D5"]:  # loco
        rh.append((s, 4, [ton]))
        s += 4
    assert s == 216
    def strecke(liste):
        return [(a * k, d * k, t) for a, d, t in liste]
    return dict(
        takte=5,
        rh=strecke(rh), lh=strecke(lh),
        ped=[(0, 1), (round(214 * k), 0)],
        dyn=[(0, "ff"), (round(128 * k), "cresc."), (round(168 * k), ">")],
        tempo=[(0, "senza Tempo"), (round(184 * k), "rallent. smorz.")],
        unsicher="Kadenz: die Fermate-Akkorde Cb5/Cb6 und Bb4/Cb6 (notiert) aus den hohlen Koepfen gelesen; die "
                 "Vierergruppe Cb Bb C♮ A♮ an allen 15 Gruppen gemessen (zwei Hoehen je Paar deutlich getrennt); "
                 "die vier Achtel am Ende Bb5 Ab5 C5 D5 (das A traegt das ♭ aus Gruppe 14); nach ff steht kein "
                 "neuer Buchstabe — der Lauf bleibt nach N2 a auf ff, cresc. und > nur als Gabel (Hoerprobe)",
        anm="Takt 32 der Partitur = Loop-Takte 32–36 (N8 a, ab hier weichen die Taktzahlen ab); Ped. ueber die "
            "ganze Kadenz, ✱ vor Tempo I; LH-Arpeggio im Bassschluessel, ab F4 im Violinschluessel",
    )


TAKTE[32] = _kadenz()

TAKTE[33] = dict(
    rh=[e for s in (0, 12, 24, 36) for e in ((s, 4, ["G4", "Eb5"]), (s + 4, 4, ["Bb4"]), (s + 8, 4, ["G5"]))],
    lh=[e for s in (0, 12, 24, 36)
        for e in ((s, 4, ["Eb2" if s % 24 == 0 else "Eb3"]), (s + 4, 4, ["Bb3"]), (s + 8, 4, ["Bb2"]))],
    ped=[(0, 1)],  # gedrueckt bis zum ✱ am Ende von Takt 34
    dyn=[(0, "pp")],
    tempo=[(0, "Tempo I")],
    anm="Eb: rechts je Schlag G4/Eb5 – Bb4 – G5, links Eb2 (Schlag 1, 3) bzw. Eb3 (Schlag 2, 4) – Bb3 – Bb2; Ped. "
        "auf Schlag 1 ohne ✱ bis zum Schluss",
)

TAKTE[34] = dict(
    takte=2,  # N8 a: ein Takt mehr mit gedruecktem Pedal fuer die Schlussfermate
    rh=[(0, 12, ["G4", "Eb5"]), (12, 12, ["G5", "Eb6"]), (24, 72, ["Eb4"])],
    lh=[(0, 12, ["Eb3", "Bb3"]), (12, 12, ["Eb4", "Bb4"]), (24, 72, ["Eb2", "Bb2", "G3"])],
    ped=[(95, 0)],
    dyn=[(0, "ppp")],
    unsicher="links Schlag 2 Eb4/Bb4 im Violinschluessel gelesen; Schlussakkord links Eb2 Bb2 G3 (unterer Kopf "
             "knapp zwischen Eb2 und D2)",
    anm="Schluss: punktierte Viertel G4/Eb5 und G5/Eb6 ueber Eb3/Bb3 und Eb4/Bb4, Schlussakkord Eb (rechts Eb4, "
        "links Eb2 Bb2 G3) mit Fermate, ✱ danach; die Fermate dauert die punktierte Halbe und einen Takt",
)
