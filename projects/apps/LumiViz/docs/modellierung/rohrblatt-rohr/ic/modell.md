# Modell Rohrblatt und Rohr — gebaut (Klasse 8, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/08_rohrblatt_rohr/` rechnet.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Rohrblatt.hpp/.cpp`, `Vorgaben.cpp`, `Stimmung.inc` (erzeugt
> von `ic_stimme_rohrblatt`); gemeinsam `Rohr` (`RohrKern`, `gitterGrenzeHz`). **Tests:** `test_rohrblatt.cpp`
> (6 Testfälle). **Offen:** [offen.md](offen.md). **Entscheide:** [offene-fragen.md](offene-fragen.md) F-1 … F-4 a (über
> FRAGEN_KONSOLIDIERT T2, T3a, T3c, T5, T7, T8). Spielbar: **Klarinette, Sopransaxophon, Tenorsaxophon, Altsaxophon**
> (ungemessen). Oboe, Fagott: Gerüst ohne Bohrung (T8 Kipppunkt b; F-B2 a/F-B4 a ohne Fund).

**Altsaxophon** (F-B4 a, 2026-10-30): Kegel r = 6,25 mm + 0,028·x nach Petersen et al. 2020 Tab. A1 S. 11 („apex angle
θ = 0.028 rad and input radius a_in = 6.25 mm, corresponding to the geometry of the alto saxophone“, **Quelle**, CC BY
4.0), Länge 1,05 m nach Ukshini & Dirckx 2025 S. 3 („about 105 cm“, **Quelle**, CC BY 4.0), Mundstück als fehlender
Kegel 223 mm / 9,1 cm³ (Rechnung wie beim Tenor); Probe c/(2·1,273 m) = 135 Hz ≈ D♭3. Jede dritte Note bei pp/mf/ff:
**21/33** auf ±10 Cent (Sopran 29/33, Tenor 30/33) — die tiefe Lage D♭3+3 … C♯4 (51–61) findet kein Fenster im
Grundregister und überbläst in die Oktave (Befund, offen.md).

## 1. Aufbau (T5 a)

**Rohr modal:** die Moden der Eingangsimpedanz je Griff (Übertragungsmatrix mit Wandverlusten, Stufen-Kegel,
Mundstückvolumen parallel) im `RohrKern` (Eingang Fluss, Ausgang Druck, Durchgriff D geschlossen gelöst). **Griff:**
je Note eine **wirksame Länge** mit offenem Ende (die Tonlöcher sind nicht einzeln gebaut, Debut et al. nur als Bericht);
dahinter das **Gitter der offenen Tonlöcher** als Abschluss Z_L → Z_c oberhalb f_c (Benade; Übergang x⁸/(1+x⁸) —
offen), nicht beim längsten Griff; Spitzen mit Q < 5 (über f_c) sind keine Moden (ihre Flanken verfälschten die Summe). Zweites Register: das Registerloch schwächt Mode 1 (×0,1, offen). **Rohrblatt**
mit einem Freiheitsgrad: m·ÿ + m·g·ẏ + k·y = S_r·Δp + F_Bahn + F_Zunge, Spalt H − y, Fluss nach Bernoulli plus der
**Fluss des Blatts** S_r·ẏ; Bahn nach Hunt–Crossley k_lay·h²·(1 + r_lay·ẏ). **Zunge** drückt das Blatt zu und gibt es
in 10 ms frei (T3c a); Flatterzunge periodisch. **Bend** über einen Vokaltrakt in Reihe (Resonanz unter der Note,
nur abwärts, Grenze 300 Cent). **Legato** als Überblenden der Moden in 10 ms (Maestre 2018). **Abstrahlung:** je Mode
das Gewicht p_Abstrahlung/p_Eingang aus der Kettenmatrix (ohne Gitter, siehe offen.md). **Atemgeräusch:** Rauschen ×
Spalt, nicht physikalisch.

**Abweichungen vom Entwurf (begründet):** (1) wirksame Länge statt Tonlochgliedern; (2) das Tonlochgitter als Abschluss
(ohne es sprang das Modell auf hohe Moden); (3) die Blattwerte nach Silva 2007 (mit Lippe) statt Chatziioannou 2019
(Blasmaschine) — mit diesen quietschte es bei 1,7 kHz; (4) Druck je Note im **Fenster** des Registers statt über eine
Schwelle (das Modell ist mehrstabil).

## 2. Größen

| Größe | Klarinette | Sopransax | Tenorsax | Herkunft / Belegkraft | gesetzt an |
|---|---|---|---|---|---|
| Bohrung | Debut-Profil 664 mm (14,8 mm, Becher 60 mm) | Kegel 1,74°, 710 mm ab 9,2 mm, Mundstück 2,25 cm³ | Kegel 1,52°, 1490 mm, r0 8,4 mm, 23 cm³ | Debut 2005 (nur Bericht) / Chen 2009 (nur Bericht) / Tenor-Mundstück Rechnung | Impedanz, Tonhöhe |
| Tonlochgitter f_c | 1500 Hz | dto. (übertragen) | dto. | Benade (Gedächtnis, offen) | Register, Helligkeit (M1) |
| Blatt m, ω_r, q_r | 3,25 mg, 2π·1500, 0,4 | dto. | dto. | Chatziioannou Tab. I (m, nur Bericht), Silva 2007 Fig. 1 (ω_r, q_r, nur Bericht) | Register, Spektrum |
| Ruhespalt, Breite, p_M | 0,41 mm, 12 mm, 8 kPa | dto. | dto. | Chatziioannou (nur Bericht); Breite, p_M offen | Schwelle |
| Bahn k_lay, r_lay | 1,97·10⁶, 2 | dto. | dto. | Chatziioannou (nur Bericht; Exponent 2 gedeutet) | Schlagen |
| Zunge k_tg, Weg, Freigabe | 3,16·10⁴, 0,51 mm, 10 ms | dto. | dto. | Chatziioannou (nur Bericht); 10 ms offen | Einsatz (M3) |
| Druck je Note | Fenster des Registers (Rechnung); pp 1,15·P_s … ff min(P_max, 2,5·P_s, Obergrenze) | dto. | dto. | P_max Klarinette 5,4 kPa, Sax 7,8 kPa (Fuks & Sundberg, nur Bericht) | Pegel, Spektrum (M2) |
| Registerloch | Mode 1 ×0,1 ab A4 | ab D5 | ab D4 | offen | — |
| Vokaltrakt | Spitze 4·10⁷ Pa·s/m³ | dto. | dto. | Chen (Größenordnung, nur Bericht) | Bend (M6) |
| Atem | 10⁻⁵ | dto. | dto. | offen, nicht physikalisch | Rest (M5) |

## 3. Schwellen (L4)

| Schwelle | Wert | gesucht/gesetzt |
|---|---|---|
| Die Note spielt: Wechseldruck ≥ 30 % des Blasdrucks nach 0,6 s und Tonhöhe ±100 Cent | 0,3 / ±100 Cent | gesetzt |
| Fenster: der längste Lauf auf dem Raster 2^(1/4) zwischen 300 Pa und 1,5·p_M | — | gesetzt (`stimme`) |
| Register 1/2 je Note | `registerAb` | gesetzt (Griff) |

## 4. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Klarinette geschlossener Zylinder 1 : 3 : 5, Saxophon Kegel 1 : 2 : 3 (ungestimmt) | ±5 % / ±8 % |
| Klarinette: jede dritte Note D3 … B♭5 bei pp, mf, ff | **32 von 33** auf ±10 Cent (D4 mf springt, Befund; vorher Clarion +1940 Cent); Test: ≥ 90 % |
| Schwelle (unterer Rand des Fensters) gegen Almeida 2013 (2–4 kPa) | 2,5–6 kPa (Test 1,5–6 kPa) |
| Chalumeau: schwache gerade Teiltöne (H2 < H3, H2 < 0,3·H1) | **11 von 11** Noten D3 … C4 (nach dem Entfernen der breiten Moden; vorher H2 bis +10 dB über H1) |
| Einsatz mit Zunge gegen Li 2016 (bis 730 dB/s) | 100–3000 dB/s (G3 gemessen im Test) |
| Legato, Bend nur abwärts und gekappt, Flatterzunge, lauter mit der Stärke | grün |
| Saxophone: die Mitte trifft | ±15 Cent |
| Gerüste still, Determinismus, Abdruck (gcc = clang), Rechenzeit Klarinette D3 2 s | grün; **≈ 5 %** (Grenze hier 6,4 %) |

**Zeit je Auswertung beim Setzen (L7):** ein Ton von 0,6 s rund 30 ms; die Stimmung aller drei Instrumente (je Note
das Fenster auf 20 Rasterpunkten, zweimal) rund 9 min auf vier Threads — darum als erzeugte Datei.

## 5. Hörproben

`ic_hoerproben <Ordner> 8`: je Instrument drei Noten × pp/mf/ff, Atem, Flatterzunge, Bend, Vibrato, Legato — 42 WAVs.
Gehört: noch nicht (L5).
