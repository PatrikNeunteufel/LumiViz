# Befund: die IC-Vorlage der Klasse 2 am gerechneten Ton (vor dem Teilplan 34k)

> **Sitzung:** S192 (2026-11-12). **Zweck:** vor dem Teilplan 34k messen, was die Vorlage unter
> `harvest/instruments-calculation/rechnung/klassen/02_zupf_saite/` (IC `4409088`) am Ton tut — nicht den IC-Tests
> glauben (Lehre aus S188, Klasse 10). **Programm:**
> [proben/2026-11-12_S192_ic_vorlage_messung.cpp](proben/2026-11-12_S192_ic_vorlage_messung.cpp), gebaut mit
> [proben/2026-11-12_S192_bau.bat](proben/2026-11-12_S192_bau.bat) (clang, Schalter wie IC); **Rohausgabe:**
> [proben/2026-11-12_S192_ic_vorlage_messung.txt](proben/2026-11-12_S192_ic_vorlage_messung.txt). Nichts im Repo
> geändert. Rechenzeit unter Drosselung (Kühlung, S181) — nur zur Größenordnung.

## Beleg des Baus

Alle drei IC-Abdrücke gleich `werte/abdruck_msvc.txt`: `zupf_Gitarre_m52_mf`, `zupf_Cembalo_m60_mf`,
`zupf_Harfe_clip_C4_E4`. Der Bau rechnet wie IC.

## Was gemessen wurde

Je Instrument jede dritte Taste bei Stärke 0 / 0,5 / 1, gezupft, 1 s gehalten, 2 s gerechnet: die Tonhöhe zweifach — die
Spektralspitze der Vorlage (±60 Cent, dieselbe, mit der `stimme` die Laufzeiten stimmt) und unabhängig YIN (0,1–0,5 s) —,
die Lautheit nach BS.1770 über den Ton, die Spitze; bei 0,5 ohne Dämpfen (*laissez vibrer*, 8 s) die Zeit bis −20 und
−60 dB unter dem lautesten 20-ms-Block. Dazu die Spielweisen und die Rechenzeit.

## Ergebnis je Instrument

| Instrument | Umfang | Lautheit über den Umfang bei 0,5 | Dynamik 0 → 1 je Taste | Tonhöhe | T60 (ohne Dämpfen) |
|---|---|---|---|---|---|
| Cembalo | F1–F6 (29–89) | **36,1 LU** (−47,1 … −11,1 LUFS), von Taste zu Taste springend (38: −30,0, 41: −13,2) | 3,5–4,4 LU | Spitze 0 / 63, YIN 3 daneben (Terz 5 : 4 an 32) | 1,8 … ≥ 8 s |
| Harfe | C1–G7 (24–103) | **26,3 LU** (−53,6 … −27,2) | 15,2–18,1 LU | Spitze 0 / 84, YIN 9 daneben (unten Oktav-/Teiltonsprünge, oben +8,5 / +21 Cent an 102 / 103) | 0,24 … ≥ 8 s |
| Gitarre (Deluxe) | E2–B5 (40–83) | **19,4 LU** (−46,6 … −27,2) | 17,5–18,1 LU | Spitze 0 / 48, YIN 3 daneben (Quinte 3 : 2 an 52) | 0,96 … ≥ 8 s |
| Laute | G2–G5 (43–79) | **20,0 LU** (−41,2 … −21,1) | 16,4–18,1 LU | Spitze 0 / 39, YIN 0 | 1,2 … 5,6 s |
| Oud | E2–F5 (40–77) | **18,6 LU** (−44,1 … −25,5) | 16,4–18,1 LU | Spitze 0 / 42, YIN 0 | 1,2 … 7,2 s |

- **Keines der fünf hält `LP3 a` (≤ 15 LU über den Umfang).** Am Cembalo und an der Harfe liegen die lautesten Tasten
  dort, wo eine Stegmode nahe am Grundton liegt (Cembalo 41, 53, 65; Harfe 42, 51, 60, 63, 69) — die Lautheit springt um
  bis zu 22 LU zwischen gemessenen Nachbartasten im Abstand einer kleinen Terz (Cembalo 65 → 68: −11,1 → −32,9 LUFS). Der Diskant fällt bei allen ab (Cembalo −47 LUFS über
  C6, Harfe −53 LUFS über C6).
- **Tonhöhe:** an der Spektralspitze trifft jede Taste (die Vorlage stimmt sich daran selbst — das ist kein unabhängiger
  Beleg). YIN springt dort, wo der Grundton schwach ist, auf Teiltonverhältnisse (5 : 4, 3 : 2) — ein Fehler der Messung,
  kein Ton daneben; an der Harfe oben (+8,5 / +21 Cent an 102 / 103) ist offen, ob YIN bei 4 kHz versagt oder die Laufzeit
  dort (11 Takte je Umlauf, Lagrange 3) nicht mehr trägt.
- **Dynamik:** am Cembalo 4 LU (gewollt, F-4 a: die Stärke wirkt vor allem im Tastenstoß), an den übrigen 15–18 LU
  (Kraft pp … ff 1 : 8).
- Kein nicht endlicher Wert.

## Spielweisen und Teile

- **Cembalo, Tastenstoß ohne hörbare Wirkung:** F2 mit und ohne Tastenstoß −14,6 / −14,6 LUFS bei Stärke 0 und −10,1 /
  −10,1 LUFS bei 1 (der Impuls trägt den Faktor 1e-4 und ist gegen die Saite verschwindend klein). Der IC-Fall „Tastenstoss
  lauter mit der Staerke“ misst nur im Band unter 300 Hz.
- **Cembalo, Lautenzug:** T60 an C4 2,90 → 1,44 s, 4,4 LU leiser — wirkt.
- **Harfe, laissez vibrer:** C4 gedämpft nach 0,5 s T60 0,56 s, frei 3,56 s — wirkt.
- **Laute und Oud, Doppelchor:** 5,0 / 6,2 LU lauter als eine Saite, T60 gleich (1,96 / 1,98 s) — die Schwebung zeigt
  diese Messung nicht.
- **Gitarre im Clip mit den leeren Saiten:** A3 nach 1 s −36,7 dB RMS mit, −122,6 dB ohne — das Mitschwingen wirkt.
- **Rechenzeit:** eine Note der Gitarre (E2) 0,22 % der Echtzeit; **die Harfe im Clip mit 47 mitschwingenden Saiten
  14,2 %** — dreimal über der Grenze 4,6 % (RZ1-1 a), auch nach dem Tausch der Kühlung nicht darunter zu erwarten.

## Was das für den Teilplan heißt

Wie bei Klasse 10 (S188) trägt die Planzeile „wie 34g“ nicht: Umzug Bit für Bit geht (der Bau trifft die Abdrücke), aber
ausgeliefert verfehlen alle fünf den Pegel über den Umfang, der Tastenstoß ist stumm, und das Mitschwingen der Harfe
sprengt die Rechenzeit. Dazu kommen Fragen, die die Planzeile nicht stellt:

1. **Bestand:** LumiViz hat eigene gezupfte Saiten (Waveguide mit Korpus und Tonabnehmer, `Saiten_Konzept.md`;
   Gitarren, Bass, Ukulele). Die IC-„Gitarre Deluxe“ trifft auf diesen Bestand wie das Clavinet in 34i (IK2-8 a).
2. **Vorgaben je Taste:** Länge, Zug, Güte und Stegmoden sind fast alle *offen*; ein eben gehaltener Pegel über den
   Umfang braucht eine Herleitung je Taste mit Quellen (wie ZV1 a), nicht ein Nachregeln.
3. **Mitschwingen im Clip** (K2 F-3 a, Harfe, Gitarre) braucht den Rechenweg *Clip* (31e) und eine Rechenzeit unter der
   Grenze — oder bleibt bis dahin aus.
4. **Korpus:** die Planzeile nennt `korpusImpulsantwort` und den Vergleich mit dem IC-Luftraum (IK2-10 a); die Stegmoden
   der Vorlage sind Annahmen, LumiViz rechnet Körper mit `InstrumentBody`.
