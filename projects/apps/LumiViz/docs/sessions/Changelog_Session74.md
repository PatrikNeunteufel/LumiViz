# Changelog Session 74 (2026-08-09)

> **Die erste durchgehende Kalibrier-Runde seit S60.** Aus einem Schalter
> („die Standalones sollen eine MP3 abspielen können") wurden sechs behobene
> Treue-Fehler, zwei vollständig durchgemessene Skript-Sprachen und fünf neue
> Prüfstands-Generatoren. Schaufenster-Presets **5/10 → 7/10 OK**,
> Version **0.3.0**.

## Teil 1 — Echte Musik in den Standalones

Das synthetische Prüfsignal ist ein Sinus mit Beat-Puls. Presets, die auf
Musikdynamik ausgelegt sind — Bass-Einsätze, Refrain-Aufbau, Stille zwischen
Schlägen — zeigen daran nie, was sie können.

**NEU `--audio-datei`** an beiden Standalones: MP3/WAV/FLAC statt Sinus, dazu
`--audio-start`, `--audio-gain`, `--audio-stumm`. Kein BASS nötig — Qt
Multimedia lag schon dabei. Abgetastet wird nach **Bild-Index** (`t = Bild/60`),
nicht nach Echtzeit: zwei Läufe mit derselben Datei sind bitgleich (an einer
Sequenz belegt, 7 von 7 Bildern identisch). Interaktiv läuft die Datei
zusätzlich **hörbar** mit; `--auto` und `--ab` bleiben still.

**Eigener Fehler unterwegs:** der erste Kopierpfad rief
`QAudioBuffer::constData<T>()` je Sample — bei vier Minuten rund 21 Millionen
Aufrufe. Ladezeit **68 s**, und der Standalone sah dabei aus wie aufgehängt.
Basiszeiger und Formatverzweigung aus der Schleife gezogen: **3,8 s**.

> **⚠️ Echte Musik macht jeden Referenzvergleich wertlos.** `AvsRef` und
> `MilkdropRef` erzeugen ihr Audio selbst. Dreifach abgesichert: `[Audio]
> ACHTUNG`-Zeile bei jedem Lauf, harte Sperre in `compare_avsref.py`, und der
> Ton schaltet sich im Stapellauf selbst ab.

## Teil 2 — Ein gemeinsamer Prüfklang statt vier Kopien

Die Formel des synthetischen Signals stand **viermal** im Baum — in beiden
Standalones und in beiden Referenz-Werkzeugen, jedes Mal mit dem Kommentar
„formelgleich zu …". Genau die Sorte Abmachung, die irgendwann still
auseinanderläuft.

**NEU `projects/exec/common/SynthAudio.hpp`**, von allen vier eingebunden.
Bewusst Qt-frei und C++14, damit das 32-bit-`AvsRef` und der D3D9-`MilkdropRef`
sie übersetzen können.

Dabei kam heraus, was vorher niemand aufgeschrieben hatte: die **Wellenform**
war in allen vier gleich, das **Spektrum** nicht. Der AVS-Pfad legt einen
Beat-Faktor über alle Bins, der MilkDrop-Pfad gibt Bass/Mitten/Höhen eigene
Hüllkurven (S64-Fix gegen 0/0-NaN). Beide Fassungen bleiben erhalten.

**NEU `--audio-muster klassisch|musik`** an allen vier Werkzeugen. `musik`
trägt acht Bandhüllkurven je Bild und eine Beat-Spur aus 20 s echter Aufnahme
und synthetisiert daraus **neu** — es wird keine Musik abgespielt, das Profil
ist nicht zurückrechenbar, und weil beide Seiten dieselbe Datei einbinden,
**bleiben Referenzvergleiche gültig**. `klassisch` ist Vorgabe und
bit-identisch zum bisherigen Verhalten (per PNG-Hash belegt).

Die Tempo-Schätzung des Profil-Generators brauchte drei Anläufe: reine
Onset-Abstände ergaben 257 BPM (Achtel), reine Autokorrelation 63 BPM
(Halbtakt) — erst ein 120-BPM-Prior lieferte **124 BPM**, einen Frame neben dem
echten Takt.

## Teil 3 — Sechs behobene Treue-Fehler

| # | Befund | vorher → nachher |
|---|---|---|
| 1 | **SuperScope-Punktzahl** bei 4096 geklemmt, Original `128*1024` | 5184 Punkte: MAE 0,056 → **0,001** |
| 2 | **`enabled` im Effekt-Blob** wurde je Effekt gelesen, fehlte bei `Water`/`Scatter` | `02_color extasy` 0,119 → **0,004** |
| 3 | **Raster-Blends vertikal gespiegelt** | Buffer-Blend 3: 0,451 → **0,002** |
| 4 | **`AvsRef`-JIT**: Fragmente von `atan`/`log` überlang | `10_the ring` 0,113 → **0,001** |
| 5 | **`memset`/`memcpy`** waren No-Ops | MilkDrop 29/31 → **32/32** |
| 6 | **EEL-Grammatik** — Zuweisung als Teilausdruck | Import-Warnung + 5 Unit-Tests |

Drei davon verdienen eine Erklärung, weil sie nicht dort lagen, wo sie aussahen.

**Nr. 2 — `Water` war nie kaputt.** Der Port wurde Zeile für Zeile gegen
`r_water.cpp` gelegt und war korrekt. Der Frame-für-Frame-Lauf entschied es: der
Referenz-Mittelwert stand über alle Frames **konstant** — dort passiert
überhaupt nichts. Im Preset steht `enabled = 0`, und `r_water.cpp` beginnt mit
`if (!enabled) return;`. Wir führten einen Effekt aus, den das Original
überspringt. Behoben als **zentrale Vorgabe** vor dem Effekt-Switch, dazu neu
`EffectNode::hasField()` — `field()` liefert 0 sowohl für „fehlt" als auch für
„ist 0", und bei einem Flag ist das das Gegenteil voneinander.

**Nr. 3 — Parität.** AVS zählt Bildzeilen von oben, `gl_FragCoord.y` von unten.
Bei gerader Bildhöhe kippt die Parität, und „jede zweite Zeile" wie „jeder
zweite Pixel" trafen genau die Gegenpixel. **Ohne Codeänderung belegt:**
dieselbe Probe bei ungerader Höhe lieferte 0,002 statt 0,451.

**Nr. 4 — der Fehler lag im Referenz-Werkzeug.** `AvsRef` rendert eine leere
Fläche, sobald ein Skript `atan` oder `log` benutzt. Mit `/Gy` (Release-Vorgabe)
bekommt jede Funktion eine eigene COMDAT, und der Linker sortierte die leeren
`_end`-Marken hinter die jeweils folgende Funktion — `atan`s Fragment wurde 128
statt 48 Bytes lang und lief in `0xCC`-Füllbytes (`int 3`). Behoben mit `/Gy-`.
**Das war Patriks Einwand:** ich hatte es zu schnell als Werkzeugfehler
abgetan, ohne den Mechanismus zu belegen. Die Nachfrage „nicht dass wir da was
falsch machen" führte zu `AVSREF_EELTEST=1`, das die Fragmentgrößen ausgibt und
19 Funktionen gegen die C-Bibliothek nachrechnet.

## Teil 4 — Alle Skript-Funktionen durchgemessen

Ein einzelner falscher Funktionswert verstellt ein ganzes Preset, ohne dass man
ihm ansieht, woher es kommt.

| Zweig | Umfang | Ergebnis |
|---|---|---|
| **AVS** (ns-eel) | 40 Funktionen | **40/40** |
| **MilkDrop** (ns-eel2) | 32 Funktionen | **32/32** |

AVS auf MAE 0,000 bei 24-Bit-Kodierung über 320 Abtaststellen, MilkDrop auf
1/255. **Kein einziger Funktionswert weicht ab.**

Zutage kamen dabei zwei andere Dinge: die fehlenden Speicher-Funktionen (Nr. 5)
und ein **echter Verhaltensunterschied in der Grammatik**. In AVS-EEL ist eine
Zuweisung nur als eigenständige Anweisung erlaubt, nie als Teilausdruck —
`exec2(q=3,q+1)`, `loop(8,q=q+2)` und `(q=3)+1` scheitern dort am Parser, das
ganze Skript wird nicht übersetzt, der Effekt bleibt unsichtbar. Wir sind
großzügiger.

**Der Import meldet es jetzt** mit Knotenpfad, Slot, Zeile und
Handlungsanweisung („Zuweisung als eigene Anweisung davorziehen — oder den
Knoten abschalten, denn genau das tut AVS"). Für MilkDrop gilt die Regel nicht;
fünf Unit-Tests halten beide Richtungen fest.

**Praktisch betrifft es niemanden:** über 3380 Presets benutzen alle
`loop`/`exec`-Presets `assign(...)` als **Funktion** — das Idiom existiert
genau deshalb. Fehlalarm-Scan über 50 Presets: 0 Treffer.

## Teil 5 — Prüfstände und Doku

**Fünf neue Generatoren** in `asset/calibration/`: Kalibrier-Raster (vier
Muster), Listen-Proben (Blend/Buffer/Reihenfolge), Buffer-Kreislauf und je ein
Funktions-Durchlauf für AVS und MilkDrop. Dazu `bisect_avs.py --solo`
(jeder Knoten einzeln) und `--raster` (Quelle davor).

**NEU `docs/kalibrierung/`** — formatübergreifender Einstieg für AVS, MilkDrop,
Shadertoy und ISF: acht Methodenregeln, Werkzeuge je Format, die
Audio-Testsignale und der Stand je Format. Für Shadertoy und ISF ausdrücklich
festgehalten, dass es **keinen Referenz-Renderer** und damit kein Treue-Urteil
gibt (nur GL-Smoke, 321/327).

## Teil 6 — Schaufenster

Alle acht README-Bilder neu. Sechs aus den Standalones zu **echter Musik**
statt zum Testton, `flowers` und `wormhole` als Aufnahmen aus der laufenden
Anwendung — die automatisch gerenderten trafen dort nichts Sehenswertes.

**Merke:** eine Helligkeitsprüfung („wie viele Pixel sind nicht schwarz") sagt
nicht, ob ein Bild etwas zeigt. Mein `wormhole`-Screenshot war ein flächiger
grüner Verlauf, bestand die Prüfung mit 65 % und war trotzdem leer.

Version **0.2.0 → 0.3.0** (`Solution.json`, SSOT). Die README-Zeile „Es gibt
noch keine Release-Binaries" war seit v0.2.0 falsch und zeigt jetzt auf die
Releases.

## Verifikation

Tests grün · AVS-Schaufenster-Presets **7/10 OK** · Kalibrier-Korpus 25/26 (der
eine Ausreißer vorbestehend, per zurückgenommener Änderung gegengemessen) ·
Buffer-Prüfstand 19/19 · Builds Ninja-Release-Clang, VS-Testing, `AvsRef`,
`MilkdropRef` je grün.

**Nicht geprüft:** Release-Build mit 0.3.0, Release-ZIP, Linux/macOS.
