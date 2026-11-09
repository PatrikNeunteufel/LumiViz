# Modell Luftstrahl und Rohr — gebaut (Klasse 9, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/09_luftstrahl_rohr/` rechnet.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Luftstrahl.hpp/.cpp`, `Vorgaben.cpp`, `Stimmung.inc` (erzeugt
> von `ic_stimme_luftstrahl`). **Tests:** `test_luftstrahl.cpp` (6 Testfälle). **Offen:** [offen.md](offen.md).
> **Entscheide:** [offene-fragen.md](offene-fragen.md) F-1 … F-5 a (über FRAGEN_KONSOLIDIERT T2, T3a, T5, T7, T8).
> Spielbar: **Blockflöte, Labialpfeife (Prinzipal 8′), Shakuhachi** (alle ungemessen). Querflöte, Altflöte: Gerüst
> ohne Bohrung (T8 Kipppunkt b, Frage F-B2 in FRAGEN_KONSOLIDIERT).

## 1. Aufbau (T5 a)

**Strahl** (jet-drive): U_j = √(2P/ρ); die Auslenkung am Labium folgt der Schnelle am Mund mit der Laufzeit
τ = W/(0,4·U_j), verstärkt e^(α_i·W): η(t) = (h/U_j)·v(t − τ)·e^(α_i W) (Terrien, nur Bericht), dazu ein **Tiefpass**
erster Ordnung bei f_j = 0,1·U_j/W (offen — ohne ihn schwang das Modell auf Moden um 3 kHz). Fluss ins Rohr nach
Fletcher & Douglas 1980 S. 768 (**Quelle**, nachgelesen): Q = b·H·U_j·(1 + tanh((η − y0)/b)), b = 2h/5. Druckquelle
am Mund Δp = (ρ·δ_d/S_m)·dQ/dt, δ_d = (4/π)·√(2hW) (Verge 1997, aus dem Gedächtnis — offen), Verluste der Ablösung
−ρ/(2·0,6²)·v|v| (nur Bericht). **Resonator:** Moden der Admittanz am offenen Mund (`RohrKern`, Eingang Druck, Ausgang
Fluss), ein Zylinder je Note mit der wirksamen Länge (Tonlöcher nicht gebaut, wie Klasse 8). **Ausgang:** Monopol
ρ/(4π)·dU/dt am Mund. **Schneidenton** (T5 a, halbphysikalisch): Oszillator bei 0,3·U_j/W mit Hülle 30 ms im
Einschwingen; **Rauschen** der Turbulenz (Pegel offen). **Orgel:** Expression = Schweller (Pegel und Tiefpass), Vibrato
= Tremulant (Druck ±37,5 % bei Tiefe 1), fester Wind.

**Register und Tonhöhe über dem Druck** entstehen im Modell (Fletcher & Douglas S. 767: mehr Druck → kürzere Laufzeit
→ höher). Die Abweichung vom Entwurf: das Register wird nicht über ein festes κ = τ/T gewählt, sondern `stimme` sucht
je Note das **Fenster**, in dem die Note spielt (unten).

## 2. Größen

| Größe | Blockflöte | Labialpfeife | Shakuhachi | Herkunft / Belegkraft | gesetzt an |
|---|---|---|---|---|---|
| Rohr | r 9,5 mm | Normmensur: 155,5 mm an 8′ C, halbiert je 16 Halbtöne | r 10,5 mm | Auvray 2015 (nur Bericht) / Wikipedia „Normmensur“ (nur Bericht, Halbierung offen) / NMM (nur Bericht) | Spektrum (M1) |
| Kanal h, Abstand W | 1 mm, 4 mm | 1 mm, **W je Pfeife** (Intonation, `stimme`) | 1 mm, 6 mm (offen) | Auvray 2015 (nur Bericht); Orgel **Rechnung** | Einschwingen (M3) |
| Mund | 12 × 4 mm | Breite ¼ des Umfangs je Pfeife, Fläche Breite × W | 11,4 × 2,8 mm | Auvray / Wikipedia (nur Bericht) / NMM | — |
| α_i, c_p/U_j, α_vc | 350 1/m, 0,4, 0,6 | dto. | dto. | Terrien, Fletcher (nur Bericht) | — |
| Strahl-Tiefpass | 0,1·U_j/W | dto. | dto. | **offen** (Befund IC Session 4) | Helligkeit |
| Druck je Note | Fenster (Rechnung), pp … ff geometrisch darin | Wind 785 Pa fest | Fenster | Auvray: Registerwechsel 695/534 Pa (nur Bericht); 80 mm WS (Lehrbuch, offen) | Pegel, Tonhöhe über pp/mf/ff (M2) |
| Schneidenton | 0,1 des Drucks, 30 ms | dto. | dto. | **offen** (T5 a: halbphysikalisch) | Einschwingen (M3) |
| Rauschen | 0,003 | 0,003 | 0,01 | Castellengo 2004 (nur Bericht, Shakuhachi „well pronounced“); Pegel offen | Rest (M5) |
| Tremulant | — | 5 Hz, ±37,5 % | — | Pykett u. a. (nur Bericht) | — |
| Schweller | — | Pegel 0,1 + 0,9e, Tiefpass 2 + 10e kHz | — | **offen** | — |

## 3. Schwellen (L4)

| Schwelle | Wert | gesucht/gesetzt |
|---|---|---|
| Die Note spielt: Tonhöhe auf ±100 Cent an der Note, > 50 % der Rahmen stimmhaft | ±100 Cent | gesetzt (`stimme`) |
| Fenster: der längste Lauf auf dem Raster 2^(1/4), ohne je einen Randschritt, Flöten höchstens ×2 um die Mitte | — | gesetzt |
| Ein Fenster ganz unter pMin ist nicht spielbar → überblasen (Spitze 2 auf der Note) | pMin | gesetzt |
| Register 1 oder 2 (überblasen) | aus dem Modell | Rechnung (`registerJeNote`) |

## 4. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Offenes Rohr: Spitzen der Admittanz 1 : 2 : 3, f1 ≈ c/2(L + 0,61r) | ±3 % |
| Labialpfeife: jede vierte Note C2 … C7 bei mf | **alle** auf ±10 Cent (Stichprobe ±6 Cent) |
| Aufschnitt wächst mit der Mensur | C2 37 mm, Fis4 6 mm |
| Schweller | +20,5 dB von e = 0 bis 1 |
| Tremulant | Hülle 2,9 dB (ohne 0,3 dB) |
| Blockflöte: alle Noten bei mf | **20 von 25** auf ±10 Cent; 2 überblasen (C6, Cis6); D6 … E6 ohne Fenster (offen.md) |
| Blockflöte: Tonhöhe steigt mit dem Druck (Fletcher & Douglas, **Quelle**) | D5 pp −15 / ff +15 Cent |
| Shakuhachi | **6 von 25** — Befund (Mund und Abstand sind Annahmen) |
| Gerüste still, Determinismus, Abdruck (gcc = clang) | grün |
| Rechenzeit Labialpfeife C2, 2 s | **0,65 %** (Grenze hier 6,3 %) |

**Zeit je Auswertung beim Setzen (L7):** ein Ton von 0,6 s rund 15 ms; die Stimmung aller drei Instrumente (je Note
das Fenster auf dem Raster, zweimal mit Feinstimmung) rund 10 min auf vier Threads — darum als erzeugte Datei.

## 5. Hörproben

`ic_hoerproben <Ordner> 9`: je Instrument drei Noten × pp/mf/ff, Vibrato bzw. Tremulant, ohne Schneidenton (Vergleich)
— 33 WAVs. Gehört: noch nicht (L5).
