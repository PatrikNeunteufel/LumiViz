# Gegenprobe: woran das Gefälle der Lautheit der IC-Vorlage hängt, und was Mitschwingen je Note kostet

> **Sitzung:** S193 (2026-11-12), vor dem Teilplan 34k. **Zweck:** die vier Umfangsfragen aus dem
> [Befund S192](2026-11-12_S192_befund_ic_vorlage.md) mit Zahlen unterlegen. Die Vorlage unter
> `harvest/instruments-calculation/rechnung/klassen/02_zupf_saite/` wird nur mit anderen Werten gefahren, nichts am Repo
> geändert. **Programme:** [proben/2026-11-12_S193_gegenprobe_ursachen.cpp](proben/2026-11-12_S193_gegenprobe_ursachen.cpp),
> [proben/2026-11-12_S193_gegenprobe_mitschwingen.cpp](proben/2026-11-12_S193_gegenprobe_mitschwingen.cpp), gebaut mit
> [proben/2026-11-12_S192_bau.bat](proben/2026-11-12_S192_bau.bat); **Rohausgaben** daneben als `.txt`. Messung wie S192:
> jede dritte Taste, Stärke 0,5, 1 s gehalten, 2 s gerechnet, Lautheit nach BS.1770. Rechenzeit unter Drosselung.

## 1. Zwei Ursachen im Rechengesetz, nicht in den Werten je Taste

Gelesen in der Vorlage:

- **U1 — der Zupf ist ein Kraftstoß fester Dauer.** `Zupf.cpp:147`: eine Hann-Kuppe der Länge `stossS` (Cembalo 4 ms,
  Harfe 3 ms, Gitarre, Laute, Oud 2 ms; `Vorgaben.cpp:89, 116`, Vorgabe `Zupf.hpp:64`), für jede Taste gleich. Ihr Spektrum
  hat die erste Nullstelle bei 2 / `stossS` — 500, 667, 1000 Hz. Ein Grundton darüber wird kaum angeregt. Ein Zupf ist
  physikalisch etwas anderes: die Saite wird ausgelenkt und losgelassen (IC `offen.md`: „Zupfen als Kraftstoß statt
  Anfangsbedingung — vereinfacht“).
- **U2 — der Steg trägt wenige scharfe Moden.** 5 bis 10 Moden je Instrument (`Vorgaben.cpp:92–93, 117, 147, 165, 178`),
  am Cembalo mit Güte π·f·0,235 / ln 1000 — 37 bei 350 Hz, 192 bei 1800 Hz. Liegt ein Grundton auf einer Mode, springt die
  Lautheit; dazwischen und darüber fehlt die Antwort. Ein Resonanzboden hat über den tiefsten Moden eine dichte Folge
  von Moden.

Vier Fahrten je Instrument: **V0** Vorlage · **V1** Stoßdauer = ¼ Periode der Note · **V2** Steg dicht (6 Moden je Oktave
von der tiefsten Mode der Vorlage bis 6 kHz, Güte 8, Spitze = Mittel der Vorlage — ein glatter Steg, **kein** Modell
eines Instruments) · **V3** beides.

| Instrument | V0 Spanne / größter Sprung | V1 | V2 | V3 |
|---|---|---|---|---|
| Cembalo | 36,1 / 21,8 LU | 18,0 / 16,5 | 26,3 / 17,3 | 19,6 / **1,9** |
| Harfe | 26,3 / 17,0 | 23,1 / 13,6 | 19,8 / 8,6 | 33,0 / **4,2** |
| Gitarre | 19,4 / 10,4 | 8,0 / 7,5 | 13,1 / 7,2 | 23,4 / **3,2** |
| Laute | 20,0 / 15,6 | 13,6 / 12,9 | 5,7 / 4,3 | 16,7 / **3,2** |
| Oud | 18,6 / 16,3 | 15,9 / 15,9 | 6,2 / 1,6 | 18,2 / **3,4** |

(Spanne über den Umfang / größter Sprung zwischen gemessenen Nachbarn im Abstand einer kleinen Terz.)

**Lesart:**

- V1 hebt den Abfall im Diskant auf (Cembalo über C5: −43 … −47 → −16 … −28 LUFS), die Sprünge bleiben — U1 belegt.
- V2 nimmt die Sprünge im Bereich der Moden (Laute 15,6 → 4,3 LU, Oud 16,3 → 1,6 LU), der Abfall im Diskant bleibt —
  U2 belegt.
- V3: die Sprünge sind weg (1,9–4,2 LU), **übrig ist ein glatter Anstieg** mit der Tonhöhe, rund 1 LU je kleine Terz
  (Cembalo −9,8 → +9,4 LUFS). Er folgt aus dem Rest des Gesetzes: gleiche Kraft für jede Taste, Stoß mit der Periode
  verkürzt, Ausgang = abgeleitete Schnelle des Stegs (`Zupf.cpp:187`, steigt mit 6 dB je Oktave). Das ist keine
  Eigenschaft eines Instruments, sondern der noch offene Teil der Herleitung: Zupf als Auslenkung und Loslassen
  (Chadefaux et al. 2012 / 2013 geben Auslenkung und Geschwindigkeit beim Ablösen), Saiten je Taste aus Mensuren, Abstrahlung.
- **Folgerung:** mit Werten je Taste allein (Länge, Zug, Güte) ist `LP3 a` nicht zu halten — keine der Kurven berührt
  U1 oder U2. Die Herleitung muss das Gesetz des Zupfs und die Herkunft der Stegmoden einschließen (Rechenfassung 2 des
  Kerns), wie ZV1 a bei der Zunge.

## 2. Mitschwingen je Note

Saiten und Steg der Vorlage sind linear. Dann ist das Mitschwingen einer ungespielten Saite die Antwort auf **eine**
gespielte Note und lässt sich je Note rechnen (die gespielte Saite und die mitschwingenden an einem Steg,
`rechneZupfClip` mit einer Note).

**Linearität belegt** (Harfe, 47 Saiten am Steg, C4 bei 0 s und E4 bei 0,25 s): Clip minus Summe der zwei Einzelnoten
−184,4 dB RMS bei *laissez vibrer* (Rundung), −86,7 dB RMS, wenn beide nach 0,5 s gedämpft werden (Clip selbst −33,3 dB —
das Dämpfen macht 53 dB unter dem Ton aus).

| Fall | Saiten am Steg | Nachklang 1–2 s (dB RMS) | Lautheit der Note (LUFS) | Rechenzeit je Note |
|---|---|---|---|---|
| Gitarre A2 / A3 / E4 / E5 ohne | 1 | −100 / −141 / −167 / −166 | −26,8 / −23,9 / −31,7 / −31,8 | 0,27 % |
| Gitarre, sechs leere Saiten | 6–7 | −63,9 / −37,8 / −61,7 / −63,7 | −26,9 / −29,2 / −34,1 / −35,4 | 1,1–1,3 % |
| Harfe G2 / C4 / E5 ohne | 1 | −210 / −225 / −267 | −32,1 / −30,7 / −49,6 | 0,27 % |
| Harfe, alle 47 | 47 | −68,8 / −46,1 / −87,9 | −40,2 / −35,9 / −54,9 | 9,4–9,9 % |
| Harfe, nur Oktaven | 6 | −57,3 / −45,6 / −95,8 | −36,3 / −35,1 / −49,7 | 1,4 % |
| Harfe, Oktaven, Quinten, Quarten | 18–20 | −69,1 / −46,7 / −90,1 | −39,9 / −35,5 / −52,8 | 3,7–4,9 % |
| Harfe, dazu Terzen und Sexten | 32–33 | −68,8 / −46,5 / −87,2 | −40,2 / −35,5 / −53,0 | 6,3–6,6 % |

**Lesart:**

- Die Saiten am Steg ändern nicht nur den Nachklang, sondern die **Lautheit der Note selbst** (Gitarre bis −5,3 LU,
  Harfe bis −8,1 LU): sie belasten den Steg. Vorgaben, die ohne sie hergeleitet werden, verschieben sich, sobald sie
  dazukommen — Pegel, Hörliste und Loops entstünden zweimal.
- Gitarre: je Note mit den sechs leeren Saiten 1,1–1,3 % der Echtzeit — unter der Grenze 4,6 % (RZ1-1 a).
- Harfe: alle 47 Saiten je Note 9,4–9,9 %, darüber. Mit Oktaven, Quinten und Quarten (18–20 Saiten) liegt der Nachklang
  bis 2,2 dB und die Lautheit der Note bis 2,1 LU neben dem Wert mit allen Saiten, bei 3,7–4,9 % — an der Grenze, unter
  Drosselung gemessen. Nur Oktaven: 1,4 %, der Nachklang bis 11,5 dB daneben.
- Diese Zahlen gelten für die Stegmoden der Vorlage (Annahmen); sie sind in der Herleitung am neuen Steg zu wiederholen.
