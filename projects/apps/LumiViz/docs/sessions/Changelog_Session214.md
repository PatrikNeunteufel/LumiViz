# Changelog Session 214 (2026-11-23)

> Diese Sitzung hat die Ursache der zu starken Kopplung an die Luft an den Quellen gesucht (KQ58 a). Sie ist
> mehrteilig; belegt und gerechnet ist der Druck längs des 1,3 m langen Kastens. Die Luft der Harfe wird als Rohr mit
> Öffnungen längs gebaut (KQ59 a), die Zielgrößen der Luft sind neu gefasst (KQ60 a). Kein Programmcode.

## Neu

- **KQ59 a:** `InstrumentBody` bekommt eine zweite Art des Hohlraums mit dem Druck längs des Kastens (Bauplan-Fassung
  10), mit einem Wächter gegen Bell Kap. 4; die Harfe wird damit neu gerechnet, die Form der ersten Mode danach als
  eigene Suche am Bauplan. Noch nicht gebaut.
- **KQ60 a:** die Luft der Harfe auf Bells Helmholtz-Mode 190 ± 1,5 Hz, ohne Saiten Bodenmode bei verschlossenen
  Öffnungen und Helmholtz-Mode auf 3 % beieinander (170 / 216 Hz bleibt Bericht); die Abstrahlung an den Saiten 33–22
  gegen den Ausgleich über alle zwölf Saiten und mit den Saiten als Dämpfer; das Profil von S(1,1)− als Zielgröße.
- Proben `modellierung/gezupfte-saite/proben/2026-11-23_S214_harfe_rohr.py` (Rohr mit 40 Längsmoden und sechs
  Öffnungen an ihrem Ort, gekoppelt an die Plattenmoden im Vakuum; `rohr`, `raster`, `rest`, `zu`, `schreibe`) und
  `…_harfe_saitendaempfung.py` (Wellenwiderstand der Saiten als Dämpfer an den Ösen).

## Geändert

- Harfenbericht `modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md` §22.
- `docs/audio/Instrumente_IC_Konzept.md` 0.83.0, `docs/Offene_Punkte.md` 1.425.0.

## Zahlen

**Aus den Quellen:** „besaitet“ heißt bei Bell (S. 147, 163) und Le Carrou 2007 (S. 560) Saiten stark gedämpft.
Versteifung des Bodens durch die eingeschlossene Luft: Bell Kasten A lose 25 Hz (176 / 201 Hz), Kasten F lose 19 Hz
(185 / 204 Hz; S. 115), Kasten G besaitet 14,5 Hz, Le Carrou 12–15 Hz; Kasten G lose und in der Harfe ohne Saiten 34
und 46 Hz, beide über einen schwachen oberen Gipfel. Modell 49–55 Hz. Schall je Kraft am Bauch gegen S(1,2): Bell
−12 / −5 dB (187 / 207 Hz), gebauter Körper −15,6 / −4,0 dB.

**Das Rohr allein gegen Bell Kap. 4:** nur Öffnung 6 offen 283 / 406 / 532 / 659 Hz gegen 283 / 414 / 545 / 680 Hz;
alle offen 189,4 Hz mit Mündungskorrektur 1,85 gegen 190 ± 1,5 Hz (ein Druck: 2,3 für 195,6 Hz).

**Gekoppelt, sonst F4 unverändert:**

| | ein Druck (gebaut) | Rohr, Mündung 1,85 | Ziel |
|---|---|---|---|
| Paar mit Luft | 173,1 / 228,7 Hz (−7,4 / +10,5 %) | 172,5 / 215,4 Hz (−7,7 / +4,0 %) | 187 / 207 Hz |
| Versteifung der ersten Bodenmode | 55 Hz | 36,5 Hz | 19–25 Hz |
| S(1,2) … S(1,6) | 0,0 / +4,4 / +4,9 / +3,4 / +2,6 % | −4,1 / −2,2 / 0,0 / +9,2 / +5,3 % | je 7 % |
| Abstrahlung 33–22 (Bezug Saite 26) | 6 / 12, größte 20,3 dB | 6 / 12, größte 11,2 dB | 12 / 12 auf 6 dB |
| Tasten 50–62 mit Mitschwingen | größter Sprung 15,7 LU | 12,6 / 10,9 / 2,8 LU | größter ≤ 12 LU |
| Öffnungen zu | – | untere Mode weg, obere 215,4 → 207,3 Hz | Bell S. 106: so |

Die Lautheit ist an einem Körper aus der Probe gemessen und gilt erst mit dem Rohr im Werkzeug. Volumenfaktor 0,5 auf
dem Rohr: Paar 183,0 / 209,5 Hz.

**Saiten als Dämpfer** (Summe der Wellenwiderstände 62,7 kg/s): Zuwachs des Verlustfaktors 0,0075 / 0,0111 / 0,0147 /
0,0088 / 0,0062 an 173 / 229 / 257 / 349 / 451 Hz; Bell beim Besaiten 0,0056 / 0,0148 / 0,0125 an S(1,2)–S(1,4).

**Form von S(1,1)−:** Bell Bauch rund 0,40 m um y ≈ 0,45 m, Knoten bei y ≈ 0,82 m; Modell 0,74 m um 0,55 m ohne
Knoten.
