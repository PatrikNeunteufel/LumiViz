# Changelog Session 161 (2026-10-29)

> **Der dritte Glasboden klingt mit der neuen Dämpfung, die Nocturne ist lauter, und das Setzen des Flügels an
> *Salamander* hat jetzt ein messbares Ziel.** Für jede Taste und jede Größe des Klangs steht fest, was die Aufnahme
> selbst auflöst (die Toleranz) und woran gemessen wird (die Struktur: Knick und Schwebung je Teilton). Beide Seiten,
> Aufnahme und Modell, werden künftig mit derselben Rechnung an dieser Struktur gemessen — ohne Erkennung, die zwischen
> den Ebenen springt. Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (SG1 a–SG3 a, UG1 a, UF17-1 a, 7.5.7,
> 7.5.8).

## Geändert

- **Resonanzboden** `steinway_d_glass_same_mass` neu gerechnet mit dem Dämpfungsgesetz aus S160 (alle drei
  mitgelieferten Körper in Fassung 3).
- **Nocturne Op. 9 Nr. 2** (`nocturne_demo`, NP2 a): Fader +7 dB mit Limiter (nach Patriks Hörurteil; der Limiter fasst
  rund 0,7 s der Kadenz).

## Intern

- **7.5.7 Toleranzen (ZF4 a):** je Taste und Größe die Streuung von *Salamander* zwischen benachbarten Ebenen und
  zwischen den Kanälen, die größere ist die Toleranz; Zeiten als Faktor, Pegel in dB, Schwebung in Cent. Die Kanäle
  bestimmen sie fast überall.
- **7.5.8 Messung an der Struktur des Ziels (ZF3 a):** `Strukturmessung.hpp`; `werte/salamander_struktur.txt`
  (Bereich, Knick, Schwebungsfrequenz je Taste und Teilton 1–3) und `werte/salamander_ziel.txt` (Ziel und Toleranz je
  Größe, an dieser Struktur gemessen). An der festen Struktur werden die Toleranzen enger als mit Erkennung.
- Entscheide vor SM4 b: drei Ebenen in der Suche (SG1 a), γ 0–45° (SG2 a), `detune` frei mit Start an der Schwebung
  (SG3 a), Untergrenze der Toleranz aus der Messgenauigkeit (UG1 a, in 7.5.9); voller Lauf nur bei großen Änderungen
  oder auf Anforderung (UF17-1 a); die nächsten Schritte 7.5.9–7.5.10 baut eine eigene Sitzung nach Bauauftrag (AB1 a).
- Tests: 1649 Cases; nach 7.5.7 voll 1646 / 1646 unter Release-clang, nach 7.5.8 Bereich 45 / 45; Testing-MSVC gebaut.
