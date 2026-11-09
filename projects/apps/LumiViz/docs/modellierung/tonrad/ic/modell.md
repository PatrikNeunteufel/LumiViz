# Modell Tonrad — gebaut (Klasse 11, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/11_tonrad/` rechnet.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Orgel.hpp/.cpp`. **Tests:** `test_orgel.cpp` (7 Testfälle).
> **Offen:** [offen.md](offen.md). **Entscheide:** [offene-fragen.md](offene-fragen.md) F-1 … F-7 a (über
> FRAGEN_KONSOLIDIERT T2, T3a, T3b, T7, T8). In LumiViz „Tonewheel Organ“ und „Rotary speaker“ (F-7 a); `organ` und
> `house_organ` nach F-5 a (ungemessen).

## 1. Aufbau (F-1 a: die Orgel als Ganzes über den Clip)

**Räder:** 91 Räder, F = 20 U/s × Zähne × Übersetzung (C 85/104 … B 108/70; die obersten sieben mit 192 Zähnen auf den
Getrieben F … B), als Zeiger mit Phase aus der Zeit (alle 4096 Takte neu verankert) — zwei Tasten auf demselben Rad
addieren sich **phasengleich**. Die tiefsten zwölf rechteckähnlich (1, ⅓, ⅕), die übrigen sinusförmig; Pegel je Rad
nach Tabelle V. **Mischung:** neun Zugriegel (−12 … +36 Halbtöne), 3 dB je Stufe, Rückfaltung unter Rad 13 und über
Rad 91. **Übersprechen** im Radfach (gleiche Welle: 2/32, 4/64, 8/128, 16/192 Zähne), −40 dB. **Tastenklick:** neun
Kontakte je Taste schließen versetzt (1,5 mm Weg ÷ Tastengeschwindigkeit) und prellen 1–3-mal (bis 4 ms); der Kontakt
schaltet die Welle hart zu — der Klick entsteht daraus. **Perkussion:** 2⅔′ oder 4′, Einzeltrigger über das Manual,
fast 250 ms / slow 900 ms (als −60 dB gelesen), soft −6 dB, 1′ stumm. **Scanner:** veränderliche Verzögerung mit
Dreieck 6,87 Hz über 0,85 ms × (⅓, ½, 1), Leitung tiefpass 7 kHz, Chorus 50 % trocken. **Schweller** −36 … 0 dB,
Vorverstärker y = tanh(1,2·x)/1,2 (offen). **Rotary:** Weiche 800 Hz 2. Ordnung, Horn mit Doppler (Radius 0,18 m) und
Richtwirkung 0,55 + 0,45·cos θ, Trommel mit Amplitudenmodulation 0,75 + 0,25·cos θ; Drehzahl erster Ordnung mit
getrennten Zeitkonstanten; die Spur setzt den Sollwert (F-2 a).

**Abweichung vom Entwurf:** der Scanner als Verzögerung statt WDF der LC-Kette (dieselbe Laufzeit und Frequenzgrenze;
die Kette als Deluxe); der Rotary mono, ohne Gehäusereflexionen.

## 2. Größen

| Größe | Wert | Herkunft / Belegkraft |
|---|---|---|
| Übersetzungen, Zähne, Motor 1200 U/min | Tabelle | Bericht §1 (Handbuch/Electric Druid, nur Bericht) |
| Radpegel | Tabelle V (9,8 V … 0,2 V, Sprung −11 dB an Rad 44) | Handbuch (nur Bericht; enthält den Frequenzgang des Vorverstärkers) |
| Zugriegel | 3 dB je Stufe | Werner & Abel (nur Bericht) |
| Übersprechen | −40 dB | setBfree-Vorgabe (nur Bericht) — **offen** |
| Klick: Versatz, Prellen | 1,5 mm/11 mm, < 5 ms | Moro et al. 2017 (nur Bericht) |
| Perkussion | 250 / 900 ms, soft −6 dB | Bonedo („ca.“) / setBfree (nur Bericht) — Bedeutung der Zeiten offen |
| Scanner | 412 U/min, 0,85 ms, V3 ≈ 1,5 % | Handbuch (nur Bericht), Werner et al. (**Quelle**) |
| Rotary Drehzahlen | Horn 48/400, Trommel 40/342 U/min | Nutzermessung Fisher (nur Bericht) |
| Zeitkonstanten | Horn 0,8/1,0 s, Trommel 4/5 s | **offen** (Herrera: getrennt, Werte nicht gefunden; Forum ~5 s) |
| Hornradius, Richtwirkung, AM-Tiefe | 0,18 m, 0,45, 0,25 | **offen** |
| `organ` / `house_organ` | 888000000, Rotary langsam / 008800000 + Perkussion 3. fast, Scanner C3 | F-5 a (Registrierungen offen) |

## 3. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Räder: A = 440 Hz exakt; C −0,58, G♯ −0,71, C♯8 +1,98, F♯8 +1,36 Cent (Bericht) | auf 0,02 Cent |
| Rückfaltung je Zugriegel (Radbereiche für 61 Tasten) | alle neun |
| Gemeinsames Rad phasengleich | +6,1 dB (linear gemischt) |
| Zugriegelstufe | −3 dB |
| Perkussion Einzeltrigger | 2 Starts bei 3 Tasten (eine gehalten) |
| Scanner V3 | ±1,17 % (Handbuch: „about 1-1/2 percent decrease“) |
| Rotary Hochlauf | 48 → 299 U/min nach 1 s (τ 0,8 s) |
| Tastenklick: über 3 kHz im Einsatz gegen gehalten | > 3× |
| Determinismus, Abdruck (gcc = clang), Rechenzeit Akkord mit Rotary | 2,5–2,9 % |

## 4. Hörproben

`ic_hoerproben <Ordner> 11`: Akkord bei drei Schwellerstellungen, Rotary-Hochlauf, Scanner V3/C3, house_organ Stöße,
Einzeltrigger — 8 WAVs. Gehört: noch nicht (L5).
