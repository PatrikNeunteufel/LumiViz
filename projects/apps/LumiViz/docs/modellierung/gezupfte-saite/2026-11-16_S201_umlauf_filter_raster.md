# Umlauf-Filter je Taste: was einfache Filterfamilien über den ganzen Umfang leisten (S201, 34k-6 Punkt (3))

> **Stand:** 2026-11-16 (Session 201) · Befund, kein Entwurf. **Der Entwurf ist zurückgestellt (Patrik S201), bis die Berichte der Aufträge 01 und 11 der Research-Werkbank vorliegen.** Ergänzt die Probe S200
> (`proben/2026-11-16_S200_umlauf_filter.py`, vier feste Fassungen an 10–14 Tasten durch den ganzen Prüfstand).
> Proben: `proben/2026-11-16_S201_umlauf_filter_entwurf.py`, `…_umlauf_filter_varianten.py`, Rohausgabe
> `…_umlauf_filter_entwurf.txt`.

## 1 Was gerechnet ist

Nur der Umlauf Steg → Sattel → Steg der steifen, gedämpften Saite am starren Steg — dafür **jede Taste** (Gitarre
120 Griffe, Harfe 80 Tasten, Cembalo 61 Tasten) und ein Raster der Entwurfsgrößen. Zwei Größen je Teilton, gegen den
genauen Umlauf des Prüfstands: die Frequenz (Phase des Umlaufs = 2πn) in Cent und das Abklingen
σ_n = −f_n ln|E(f_n)| in Prozent. Ausgewertet an den Teiltönen 1–12 unter 10 kHz.

Das ist schärfer als die Lautheit am Ton: in S200 lag die Lautheit der Gitarre mit allen Fassungen innerhalb 0,2 LU,
obwohl die Teiltöne bis 20 Cent danebenlagen.

## 2 Steifheit: K gleiche Allpässe 1. Ordnung, ein Wert je Taste

Größter Fehler der Teiltöne 1–12 über alle Tasten / Mittel der größten Fehler je Taste, in Cent (Anpassband 5 kHz):

| K | Gitarre | Harfe | Cembalo |
|---|---|---|---|
| 0 (keine Steifheit) | 128 / 20,8 | 57,5 / 26,8 | 9,1 / 3,7 |
| 4 | 35,6 / 3,7 | 61,3 / 11,5 | 3,8 / 0,6 |
| 8 | 22,5 / 2,4 | 39,7 / 7,3 | 2,3 / 0,4 |
| 16 | 10,4 / 1,3 | 21,2 / 4,1 | 1,3 / 0,3 |
| 32 | 2,6 / 0,5 | 8,2 / 1,9 | 0,7 / 0,2 |

- Die schlechtesten Tasten: Gitarre Saite 3 (G, EI 310 · 10⁻⁶ N m²) in den hohen Lagen; Harfe die umsponnenen
  Basssaiten (Tasten 25–29, Stahlkern) und die dicken Darmsaiten (Tasten 45–49).
- **Ein Anpassband bis 10 kHz statt 5 kHz macht die Teiltöne 1–12 schlechter** (Gitarre Mittel 2,9 statt 1,3 Cent bei
  K = 16): die hohen Teiltöne ziehen den einen Wert von den tiefen weg.
- **Zwei Gruppen von Allpässen mit je eigenem Wert bringen nichts** — das Optimum liegt bei gleichen Werten (K = 8
  und 16, alle drei Instrumente, auf 0,1 Cent gleich).
- Unter 5 kHz bleiben an der Harfe im Bass Hunderte Cent (Teiltöne über dem 40.) — dort trägt keine Fassung, und der
  Stahlkern der Basssaiten ist im Prüfstand eine Annahme.
- An den höchsten Tasten passt die volle Zahl nicht in einen Umlauf (Harfe ab Taste 98: K höchstens 8); dort ist die
  Steifheit auch kaum nötig (unter 2 Cent).

## 3 Verlust

Größter Fehler des Abklingens der Teiltöne 1–12 / Mittel der größten je Taste / größter Fehler am Grundton, in Prozent:

| Verlustfilter | Gitarre | Harfe | Cembalo |
|---|---|---|---|
| Pol 1. Ordnung, Betrag bei f₀ und 8 f₀ (Kern heute) | 40 / 15 / 0 | 128 / 33 / 0 | 41 / 36 / 0 |
| FIR symmetrisch 5 Werte, Beiwerte ≥ 0 | 36 / 16 / 13 | 41 / 16 / 11 | 35 / 25 / 21 |
| FIR symmetrisch 9 Werte, Beiwerte ≥ 0 | 36 / 16 / 12 | 34 / 15 / 9 | 32 / 20 / 21 |
| FIR 9 Werte ohne Vorzeichenzwang | 16 / 5 / 11 — an 20 von 120 Tasten unzulässig | 9 / 4 / 9 — an 37 von 80 | 31 / 19 / 20 |
| Pol + FIR 5 Werte | 19 / 9 / 10 — an 44 von 120 unzulässig | 9 / 4 / 7 — an 44 von 80 | 28 / 18 / 20 |

- „Unzulässig“ heißt: der Betrag des Filters überschreitet irgendwo 1 (die Schleife wäre nicht passiv). Der Zwang
  „Beiwerte ≥ 0“ aus S200 sichert das, kostet aber die Form.
- **Am Pol mit Betrag bei f₀ und 8 f₀ wird der Betrag an hohen Harfentasten größer als 1** (1,0003): 8 f₀ liegt dort
  über der halben Abtastrate.
- **Das Cembalo bleibt mit jeder dieser Fassungen bei 20–35 %**, auch am Grundton: der Verlust des Drahts im
  Prüfstand (Luft nach Stokes, wächst etwa mit √f, dazu Biegung) hat eine Form, die ein kurzes symmetrisches FIR
  (Verlust ∝ f² bei tiefen Frequenzen) nicht trägt.
- Der Ausreißer aus S200 (Harfe, Tasten 96 / 103, fir9+ap: +5 LU, +25 … +43 dB nach 1–2 s) ist hier nicht
  nachgestellt; nach diesem Raster kommt dafür in Frage, dass Filter und Allpässe zusammen nicht in den Umlauf passen
  (rund 23 und 15 Takte). Nicht geprüft.

## 4 Was daraus folgt

- Mit **gleichen Allpässen 1. Ordnung** braucht eine Schwelle von 10 Cent an den Teiltönen 1–12 rund K = 16 an der
  Gitarre und K = 32 an der Harfe; 3 Cent brauchen K = 32 und mehr. Das sind 16–32 Multiplikationen je Abtastwert,
  Saite und Polarisation allein für die Steifheit.
- Für den **Verlust** trägt keine der einfachen Formen alle drei Instrumente; die besseren sind nicht überall passiv.
- Beides sind Fragen, für die es in der Literatur gezielte Entwürfe gibt (Allpässe höherer Ordnung nach Abel,
  Välimäki & Smith 2010; Rauhala & Välimäki 2006; Verlustfilter nach Bank & Välimäki 2003). Die Aufträge
  `2026-11-16_01_umlauf_filter_saite_steg` und `2026-11-16_11_umlauf_filter_grundlagen` der Research-Werkbank holen
  genau diese Formeln und rechnen sie an unseren Saiten nach; der erste ist angehalten (Zwischenstand liegt vor).
- **Nicht entschieden und nötig vor dem Entwurf:** die Schwellen des Kerns gegen den Prüfstand je Teilton (Cent,
  Abklingen in Prozent, bis zu welchem Teilton) — bisher gibt es nur die Schwellen am Ton (GZ9 a: Tonhöhe ±10 Cent).

## 5 Offen

- Schwellen je Teilton (Vorlage, sobald die Verfahren der Literatur daneben liegen).
- Der Ausreißer Harfe 96 / 103 am ganzen Ton.
- Die gebrochene Stelle des Verzugs und die Quelle des Zupfs im Zeitschritt — hier nicht berührt.
- Laute und Oud (Chöre) sind im Raster nicht dabei.
