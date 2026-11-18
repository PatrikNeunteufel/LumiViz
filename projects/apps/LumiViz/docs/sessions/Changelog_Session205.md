# Changelog Session 205 (2026-11-18)

> Der Umlauf-Filter der gezupften Saite ist fertig entworfen, die Herleitung des Kerns steht als Dokument. An der Harfe
> sind zwei Ursachen der zu tiefen Admittanz gefunden (im Bauplan fehlt die Zugleiste in der Rechnung; Bells Messung
> trägt eine Feder in Reihe), an der Gitarre ist die Wirkung von Modendichte und zweiter Stegrichtung gemessen. Kein
> Programmcode — die Körper von Harfe und Gitarre werden in der nächsten Sitzung neu gebaut.

## Neu

- **Herleitung 34k-6:** `docs/modellierung/gezupfte-saite/2026-11-18_S205_herleitung.md` — der Rechenweg des Kerns
  `pluckedString`, Rechenfassung 2, vom Zupf bis zum Hörort; offene Stellen in §12.
- **Umlauf-Filter, Proben** `docs/modellierung/gezupfte-saite/proben/2026-11-18_S205_*`: gebundene Anpassung längs der
  Bünde (`gebunden_stuetzstellen`), Glied für den gebrochenen Verzug (`verzug_glied_boden`), Einschwingen im Slide
  (`slide_einschwingen`), Tafel je Taste für Harfe und Cembalo (`tafel_je_taste`), Prüfung mit Körper
  (`tafel_mit_koerper`).
- **Harfe (KQ31 a):** `2026-11-13_S197_harfe_gelesen.md` §15; Proben `harfe_ort17_kurve`, `harfe_profile`,
  `harfe_bauplan_zugleiste`, `harfe_bewegte_masse`, `harfe_je_taste`, `leisten_uebereinander`.
- **Gitarre (KQ35 a):** `2026-11-18_S205_gitarre_kq35.md`; Probe `gitarre_kq35` (`dichte`, `richtung`, `ton`).

## Geändert

- `docs/audio/Instrumente_IC_Konzept.md` 0.64.0: UL3-1 a (Thiran 5, gebundene Anpassung) mit Nachschärfung und der
  Liste der Ausnahmen (Laute Chor 8.1); Befund zu KQ31 a, KQ31-1 a (Bauplan-Fassung 8: Leisten übereinander),
  KQ31-2 a (die Harfe an den Moden gemessen); Befund zu KQ35 a, KQ35-1 a (Richtung je Band und Moden parallel zur
  Decke), KQ35-2 a (Richtung je Mode in der Körperdatei).
- `docs/Offene_Punkte.md` 1.406.0: Zeile 34k.
- `docs/modellierung/gezupfte-saite/README.md`: Verweis auf die Herleitung.

## Zahlen

- Umlauf-Filter an fremden Prüfstellen: Gitarre ≤ 1,59 Cent, Laute ≤ 2,22 Cent (Chor 8.1: 12,2, Ausnahme), Oud
  ≤ 0,17 Cent; Tafel je Taste Harfe 80 / 80, Cembalo 61 / 61 in 3 Cent.
- Harfe mit Zugleiste: Lage S(1,2)–S(1,6) −6,4 / +4,0 / +0,3 / +1,6 / −1,5 % (mitgeliefert +10,4 … −9,6 %); Feder in
  Bells Messweg 2,6 · 10⁵ N/m.
- Gitarre: Auffüllen der Dichte ohne messbare Wirkung am Ton; Richtung nach Elie ändert das Abklingen an 8–14 von 44
  Tasten um mehr als 2 dB/s.

## Tests

Keine (kein Programmcode); **1921 Cases**.
