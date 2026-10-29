# Changelog Session 162 (2026-10-29)

> **Das Setzen des Flügels an *Salamander* hat jetzt eine Zielsumme und ein Suchverfahren, und beides ist zum ersten
> Mal über alle 30 Tasten gelaufen.** Jede Messgröße zählt im Verhältnis zu dem, was die Aufnahme selbst auflöst; die
> Suche verbessert den Abstand zur Aufnahme um rund drei Viertel, findet aber von zwei Startpunkten aus nicht dieselbe
> Lösung. Ob das Setzen damit trägt, entscheidet die Probe 7.5.11 in der nächsten Sitzung. Am Klang des Programms hat
> sich nichts geändert. Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (7.5.9, 7.5.10).

## Intern

- **7.5.9 Zielsumme (ZF5 a, UG1 a):** je Taste und Größe der Abstand des Modells zur Aufnahme geteilt durch die
  Toleranz, nach unten begrenzt durch die Messgenauigkeit (Faktor 1,03, 0,5 dB, 2 %); Modellwert als Median über drei
  Anschlagstärken. Kurven der Suche (neun Kurven, 232 Parameter) und zwei Startpunkte. Nullprobe: das heutige Modell
  trifft 86 von 456 Messgrößen.
- **7.5.10 Levenberg–Marquardt (ZF6 a, SP2 a):** über die Stützpunkte der Kurven, Ableitungen je Kurve, Dateien je
  Iteration mit Wiederaufnahme; Vergleich mit Nelder–Mead.
- **Läufe:** LM liegt an zwei von drei Tasten deutlich tiefer als Nelder–Mead, an der dritten gleichauf mit weniger
  Rechnungen. Die Suche: Rest 42 854 → 11 290 (Startpunkt 1, 149 von 456 getroffen) und 61 363 → 16 541 (Startpunkt 2);
  gemeinsam getroffen 71 von 456 — nicht eindeutig. Berichte unter `docs/modellierung/hammer-saite/proben/S162_*`.
- Tests: 1660 Cases; Bereich `*Salamander*,*Ziel*,*Suche*` 54 / 54 unter Release-clang; Testing-MSVC gebaut.
