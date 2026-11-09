# Modell Platte und Stab — gebaut (Klasse 4, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/04_platte_stab/` rechnet, mit Gleichungen und der Herkunft jeder
> Größe (nach dem Muster von `gemeinsam/vorbild-lumiviz/docs/hammer-saite/modell.md`). **Stand:** 2026-10-29, IC
> Session 4 (Cloud). **Code:** `Stab.hpp` (Kern, Herleitung im Kopf), `Stab.cpp`, `Vorgaben.cpp` (zehn Instrumente).
> **Tests:** `test_stab.cpp` (11 Testfälle). **Offen:** [offen.md](offen.md). **Entscheide:**
> [offene-fragen.md](offene-fragen.md) F-1 a … F-6 a, F-7 c (über FRAGEN_KONSOLIDIERT T1, T2, T3b, T7, T8).
> **Belegkraft je Größe:** *Quelle* (selbst nachgelesen, IC Session 1, Volltext im Cache — Rucz & Ulveczki 2021),
> *nur Bericht* (Wortlaut im Agentenbericht unter `recherche/`, an der Quelle nicht geprüft), *Rechnung* (hier
> abgeleitet), *offen* (Vorgabe mit Grund). Alle Instrumente sind **ungemessen** (keine Aufnahme in der Cloud, L6:
> Grundlagen vor Feinabstimmung).

## 1. Überblick

Der Stab ist linear, der Kontakt nichtlinear — dieselbe Trennung wie beim Flügel (Entwurf §2). Je Note:

1. **Moden vorab:** Biegemoden aus den Kurven der Verhältnisse f_n/f_1 (Moden 2, 3), darüber der Abstand des
   gleichförmigen Balkens; die Formen φ_n(x) des gleichförmigen frei–freien Balkens (bzw. des Kragbalkens bei Zungen)
   aus `gemeinsam/Balken` (FE, 120 Elemente, auf die modale Masse 1 normiert, mal der Masse des Stabs). Torsion nur
   mit Versatz e von der Längsachse: φ_t = √24·(e/b)·cos(kπx/L), f_tk = k·f_t1.
2. **Stoß** (`gemeinsam/Modal` `stosse`): q̈_n + 2σ_n q̇_n + ω_n² q_n = φ_n(x_s)·F/m; Kopf der Masse m_M mit
   v = v_max·10^(Stufe/20); F = Q0·δ^p (Hertz, Rucz Gl. 6) bzw. Stulov (Filz) bzw. Hunt–Crossley (Neopren);
   64-fach feiner (Rhodes 32, Celesta/Wurlitzer 16). Konvergenz: 64 gegen 256 auf 1 % (Test).
3. **Frei, Takt für Takt:** je Stabmode ein komplexer Zeiger; eine Stabmode mit **Partner** (Rohr, Tonebar, Kasten)
   als Paar ż_s = λ_s z_s − iκ z_p, ż_p = λ_p z_p − iκ z_s mit der exakten 2 × 2-Exponentialmatrix je Takt (mit
   Motor je Takt neu, κ·w(θ)). Daempfer: σ ← max(σ, Andruck·σ_D) ab dem Takt, den der Aufrufer gibt.
4. **Ausgang:** akustisch Σ g_n·q̈_n + Σ g_p·w·q̈_p; mit Tonabnehmer die Auslenkung y an der Spitze durch
   Φ(y) = 1/√((y − y0)² + d²) (magnetisch, Rhodes) bzw. C(y) = 1/(d − y) (elektrostatisch, Wurlitzer), Ausgang dΦ/dt.
   Normiert so, dass die Mitte des Umfangs bei Stufe 0 mit Spitze 0,5 klingt (`ausgangBezug`, von Claude gesetzt).
5. **Steeldrum (Deluxe):** feste Verhältnisse 1 : 2 : 2 : 4 : 4 : 6 : 8 (Thomas et al. 2014, G3 — nur Bericht) und
   eine quadratische Kopplung der Moden im Verhältnis 1 : 2 (Potential V = c·m·ω_j²·q_i²·q_j/L0, L0 = 1 mm), c = 0 bis
   zu einer Messung (T8 a: Ort jetzt, Grenze neutral).

## 2. Größen je Note (Kurven) — Herkunft, Grenze, Messgröße

| Größe (Feld) | Skala | Vorgabe, Herkunft | Grenze | gesetzt an (messplan.md) |
|---|---|---|---|---|
| f_1 je Note (`grundJeNote`) | lin | 1 (Rechnung); Röhrenglocken 2/8,933 = 0,224: gehört wird eine Oktave unter Mode 4 (CCRMA, nur Bericht), f_4/f_1 = 8,933 (Rechnung, Balken) | 0,05 … 2 | Tonhöhe (M1) |
| f_2/f_1, f_3/f_1 (`verhaeltnis2`, `verhaeltnis3`) | lin | Marimba 4, 10; Xylophon 3, 9 (Rucz & Ulveczki 2021 S. 1, **Quelle**); Vibraphon 4 (CCRMA, nur Bericht), 7,84 (offen: Abstand des Balkens); Glockenspiel, Röhrenglocken, Celesta 2,757, 5,404 (Rechnung: gleichförmig); Kalimba 5,86 (D3), 5,30 (D4) (Chapman 2006 Tab. 1, nur Bericht), ×2,80 (Rechnung); Rhodes, Wurlitzer 6,267, 17,55 (Kragbalken, Rechnung, offen: Stimmfeder/Lötzinn senken es) | 1,5 … 20; 2 … 60 | Modalfit (M1, B6) |
| T60 der Mode 1 ohne Partner (`t60S`) | log | Holz: ξ = 2,57·10⁻⁴ (Rucz Tab. 1, **Quelle**, F4) → 12,3 s an F4, fester ξ über den Umfang (offen); Vibraphon, Glockenspiel, Celesta ξ = 10⁻⁴, Röhrenglocken 5·10⁻⁵ (offen, Einschätzung Metall); Rhodes σ = 10⁻⁴ + 0,005·(1,875/l)² (Sønderbo 2024 Tab. 3.1, nur Bericht; l nach Falaize & Hélie, Rechnung); Kalimba 2 s, Steeldrum 1,5 s, Wurlitzer ξ = 5·10⁻⁴ (offen) | 0,01 … 200 s | Abklingen je Mode (M2) |
| Daempfung über die Frequenz (`daempfungExponent`) | lin | κ = 0: fester Verlustfaktor (offen; Rucz S. 7: höhere Teiltöne „significantly greater“ → κ > 0 erwartet); Glockenspiel 1 (CCRMA „All but the lowest … die out quickly“, nur Bericht) | 0 … 3 | M2 |
| Masse des Stabs (`masseKg`) | log | 96,7 g (Rucz Tab. 1, F4; über den Umfang offen); Rhodes ρπr²l (Rechnung aus Falaize & Hélie, nur Bericht); sonst offen (Einschätzung) | 0,1 g … 10 kg | Pegel und Kontaktdauer (M4) |
| Torsion f_t1/f_1, T60 (`torsionVerhaeltnis`, `torsionT60S`) | lin, log | 3 (Platzhalter ohne Wirkung bei Versatz 0 — La Favre misst sie, nur Bericht, ohne Zahl); 0 = keine (Zungen) | 0 … 20 | Modalfit bei Anschlag außerhalb der Achse (M1) |
| Abstrahlgewicht (`abstrahlExponent`) | lin | g_n = n⁰ = 1 (offen; der Ausgang q̈ wichtet die Höhen schon mit ω²) | −2 … 2 | Pegel je Mode (M3) |
| Partner f_r/f_1 (`partnerVerhaeltnis`) | lin | Rohr 349,1/348,8 (Rucz Tab. 1, **Quelle**); Tonebar, Kasten 1 (offen) | 0,8 … 1,25 | M5 |
| Partner ξ (`partnerXi`) | log | 8,8·10⁻³ (Rucz Tab. 1, **Quelle**; beim Vibraphon übertragen); Tonebar 10⁻³ (offen) | 10⁻⁵ … 0,2 | M5 |
| Kopplung κ/κ_c (`kopplungJeKritisch`) | lin | 1,2 (offen — Grund: Rucz’ T60 0,7 s ist das Mittel der Dämpfungen, das verlangt κ ≥ κ_c; 1,2 meidet die Entartung bei κ = κ_c, L11) | 0 … 10, **ohne 0,95 … 1,05** | zweistufiges Abklingen des Grundtons (M5) |
| Partner-Abstrahlung (`partnerAbstrahl`) | log | gesetzt: +15 dB Pegel mit Rohr an F4 („by as much as 15 dB“, Rucz S. 7) durch das Modell hindurch (Halbierung, Rechnung) → Gewicht 22,9 (Marimba), 23,8 (Xylophon), 36,9 (Vibraphon); Celesta übertragen (offen) | 0 … 10⁶ | Pegel des Grundtons mit/ohne Rohr (M5) |
| Anschlagstelle (`anschlagStelle`) | lin | Mitte 0,5 (offen, „Mitte üblich“ — Einschätzung im Messplan); Röhrenglocken 0,02 (oben); Rhodes 2,35/7,83 (Falaize & Hélie, nur Bericht); Kalimba 1 (Spitze) | 0 … 1 | Verhältnis Mode 2 zu Mode 1 (M3) |
| Versatz von der Achse (`querVersatz`) | lin | 0 (offen) | −0,5 … 0,5 | Torsion (M1) |
| Dämpfer T60 (`daempferT60S`) | log | Vibraphon 0,3 s, Röhrenglocken 0,5 s, Celesta 0,2 s, Rhodes/Wurlitzer 0,15 s (offen) | 0,01 … 10 s | Iowa „Dampen“ (M7) |

## 3. Feste Größen je Instrument

| Größe | Vorgabe, Herkunft | Grenze |
|---|---|---|
| Schlägel Holzstäbe | 25 g, K_M = 5·10⁷ N/m^1,5 = 1581 N/mm^1,5, p = 3/2 (Rucz Tab. 1, **Quelle**; Garn, Kordel, Gummi offen) | m 0,1 g … 1 kg |
| Hammer Celesta, Wurlitzer | Stulov & Kartofelev 2009 Gl. 9 über das Vorbild an MIDI 88 bzw. 60 (übertragen vom Flügel, offen) | — |
| Hammer Rhodes | Hunt–Crossley M = 11 g, α = 2,8, k = 1,5·10¹¹ N/m^2,8, λ/k = 0,6 s/m, v bis 4 m/s (Sønderbo 2024 Tab. 3.2, nur Bericht) | — |
| v_max (`tempoMax`) | 5 m/s wie der Flügel (offen; HA3 a); Rhodes 4 (nur Bericht); Wurlitzer 1,5 (offen: die Zunge bliebe sonst nicht unter dem Abstand); Kalimba: 1 mm Auslenkung | 0,01 … 20 |
| Motor | zwei Perioden je Umdrehung, Restöffnung 0, Hochlauf 0,5 s (offen; Drehzahl 1–12 U/s laut Patenten, nur Bericht) | r 0 … 1, 0,01 … 10 s |
| Tonabnehmer | Rhodes d = 1 mm (Falaize & Hélie, nur Bericht), y0 = 0,5 mm (offen: Voicing — bei 0 fehlte der Grundton); Wurlitzer d = 3 mm (offen), 130 V (Pfeifle, nur Bericht; wirkt nur als Faktor) | — |

## 4. Schwellen, an denen das Modell seine Struktur wechselt (L4, F-L3 a)

| Schwelle | Wert | gesucht/gesetzt |
|---|---|---|
| Paarfenster: eine Stabmode bekommt einen Partner, wenn \|f_s − f_p\|/f_s < 6 % (`kPaarFenster`) | 6 % | **gesetzt** — eine Paarung, die an der Schwelle kippt, macht die Zielfläche zur Treppe; am Bezug wird die Paarung einmal festgelegt (L4) |
| kritische Kopplung κ_c: darunter behält eine Mode ihre eigene Dämpfung, darüber tragen beide das Mittel | κ/κ_c = 1 | **gesucht** über κ/κ_c, die Grenze schließt 0,95 … 1,05 aus (L11) |
| Doppelschlag (`kontakte` ≥ 2) | — | Prüfgröße, kein Parameter |
| Torsion nur mit Versatz ≠ 0 | e = 0 | **gesetzt** (die Wirkung bei e = 0 ist exakt null, L9: ohne Wirkung) |
| Modengrenze 0,45·f_s | — | gesetzt (wie der Flügel) |

## 5. Geprüft (Tests, `test_stab.cpp`)

| Prüfung | Ergebnis |
|---|---|
| Verhältnisse 1 : 4 : 10, 1 : 3 : 9; darüber der Balken; Mitte regt Mode 2 nicht an | exakt |
| Xylophon F4 gegen Rucz & Ulveczki Tab. 1: T60 ohne Rohr 12,3 s („close to 10 s“), mit Rohr (ξ_a + ξ_m)/2 → **0,71 s** (Rucz: 0,7 s), am Ton gemessen auf 15 %; +15 dB mit Rohr (gesetzt, Probe); **Kontakt 0,6 ms bei 1 m/s** (Rucz: 0,6 ms) auf 15 % | grün |
| Hertz-Kontaktzeit gegen die geschlossene Lösung (`test_gemeinsam`) | auf 1 % |
| Rhodes-Tine nach Falaize & Hélie: Stahl r = 2 mm, 7,83 cm → **440 Hz** (FE-Kragbalken) | auf 0,5 % |
| Motor 4 U/s → Modulation des Pegels bei 8 Hz | Spitze ≥ 3 × Nachbarn |
| Dämpferleiste, halbes Pedal | voll < halb < frei, > 20 dB |
| Rhodes: Klirr (H2/H1) steigt von pp nach ff | ×1,5 |
| Helligkeit steigt mit der Stärke (Marimba, Xylophon, Glockenspiel) | grün |
| Grenzen klemmen, Entartung der Kopplung ausgeschlossen | grün |
| **Vibraphon gestrichen** (Bogen × Stab, T1 a): E♭4 … F6 bei pp und mf auf ±10 Cent, Haften > 5 % | **18/18**; C4 ohne Resonator −5 Cent (mit Resonator +100 Cent, offen.md) |
| Determinismus: wiederholt und auf vier Threads Bit für Bit; Abdruck je Instrument (gcc und clang gleich) | grün |
| Rechenzeit tiefster Ton, 2 s (Cloud-Rechner; Flügel-Bezug A0 6,5 %, Grenze hier 7,1 %) | Marimba 0,6 %, Xylophon 0,4 %, **Vibraphon mit Motor 3,0–3,6 %**, Glockenspiel 0,2 %, Röhrenglocken 0,5 %, Celesta 0,4 %, Kalimba 0,2 %, Rhodes 0,3 %, Wurlitzer 0,3 %, Steeldrum 0,2 % — **kein Deluxe aus Rechenzeit** |

**Zeit je Auswertung beim Setzen (L7, gemessen auf dem Cloud-Rechner):** ein Ton von 0,5 s kostet 1,4 ms (Marimba)
bzw. 14 ms (Vibraphon mit Motor); eine Auswertung über den Iowa-Umfang (Marimba C2–C7, 61 Noten × 3 Stärken = 183
Töne) 0,27 s auf einem Thread, das Vibraphon (42 × 3) 1,8 s — mit dem Pool auf 11 Threads rund ein Zehntel. Ein
Setz-Lauf mit 50 Iterationen und 30 Parametern (Jacobi je Parameter) kostet damit an der Marimba rund 7 min.

## 5a. Bogen × Stab (T1 a, K4 F-4 a; IC Session 4, 2026-10-30)

Der gestrichene Stab ist das Paar Bogen × Stab nach SF10 a: die **Reibung der Klasse 3** (`gemeinsam/Reibung`,
Galluzzo et al. 2017 Gl. 1, Hysterese nach Friedlander — aus `Bogen.cpp` herausgelöst, Klasse 3 bitgleich) wirkt am
**Ende des Stabs** (x/L = 0, auf der Achse — die Torsion bleibt still) auf die Biegemoden. Der Stab antwortet im Takt
mit v = v_h + F·Δt·Σφ²/m (der Kraftstoß eines Takts an der Bogenstelle), daraus z_eff = m/(Δt·Σφ²) für den Schnitt mit
der Reibkurve; danach laufen Partner, Motor und Dämpfer wie beim Anschlag. Expression als Weg wie Klasse 3: v_B
0,08 … 0,4 m/s (Chudy, übertragen), F_B in der Mitte des Fensters, das `stimmeBogen` je dritte Note bei pp, mf und ff
**am Modell** sucht (Raster 2^(1/8), 10 mN … 200 N; spielt = schwingt an +20 dB, ±15 Cent an f₁, Haften 5–98 %),
stückweise geometrisch; erzeugt in `Stimmung.inc` (`ic_stimme_stab`, 3½ min). **Befund:** am Modell gibt es drei
Zustände — reines Gleiten (−10 Cent, schwingt kaum an), Haft-Gleit nahe f₁ (−3 Cent, 25 % Haften) und in der tiefen
Lage mit Resonator ein Haft-Gleit auf der oberen Mode des gekoppelten Paars Stab–Rohr (+80 … +110 Cent); ohne Resonator
spielt C4 sauber. Die Kopplung κ/κ_c ist von Rucz (Xylophon) übertragen und offen — an Iowa zu setzen, dann neu stimmen.

## 6. Hörproben

`ic_hoerproben <Ordner> 4` schreibt 108 WAVs (je Instrument drei Noten × drei Stärken, dazu Motor 2 und 6 U/s,
gedämpft, Anschlag am Rand, Vibraphon gestrichen), auf −23 LUFS normiert, mit `liste.txt` (Frage je Datei). Gehört: **noch nicht** (L5 —
Patrik lokal).

## 7. Abweichungen vom Entwurf (begründet)

- **Formen des gleichförmigen Balkens** statt mit Unterschnitt: kein Profil mit Quelle (Bericht: „Unterschnittprofile
  als Zahlenreihe … nicht gefunden“); die Frequenzen kommen aus den Kurven, die Formen wirken nur auf die Anregung je
  Mode (offen.md).
- **Resonator als Partner in der Näherung rotierender Wellen** statt eines Rohr-Kerns mit Zeitbereichskopplung: die
  Kopplung Stab–Rohr wirkt nur nahe der Gleichheit der Frequenzen (6 %), dort ist die Näherung genau; das Rohr selbst
  (ungerade Moden, Endkorrektur) ist der Fall „unten geschlossen“ des gemeinsamen Rohr-Kerns, die Moden aber als
  (2m − 1)·f_r (ideale Endkorrektur, offen.md).
- **Während des Kontakts ohne Partner** (κ·1 ms ≪ 1).
- **Kasten der Kalimba** nicht gebaut — Vorgabe die Brett-Kalimba (Maße und Volumen ohne Quelle; der Luftraum-Baustein
  steht in `gemeinsam/Koerper`, offen.md).
