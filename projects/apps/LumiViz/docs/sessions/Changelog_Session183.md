# Changelog Session 183 (2026-11-09)

> **Wurlitzer, Kalimba und Clavinet klingen über den ganzen Umfang gleich laut.** Die drei gerechneten Instrumente
> hatten Bauwerte, die für alle Tasten gleich waren — das Wurlitzer war im Bass viel zu laut und im Diskant zu leise, die
> Kalimba und das Clavinet umgekehrt. Jetzt sind Zunge, Hammer, Tonabnehmer und Saite je Taste aus Quellen hergeleitet.

## Geändert

- **Reed electric piano (Wurlitzer):** die Zunge ist je Taste so schwer wie beim Instrument (aus Länge, Breite, Dicke und
  dem Lot an der Spitze), der Hammer ist der kleine harte des Wurlitzer, nicht mehr ein Klavierhammer, und der Tonabnehmer
  ist der Schlitz in der geladenen Platte, durch den die Zunge schwingt: große Wege im Bass werden reich an Obertönen, nicht
  lauter. Spanne über den Umfang 34 → 11 LU; kein Übersteuern mehr im Bass.
- **Kalimba:** der Daumen zupft mit einer Kraft, nicht mit einem festen Weg, und das Brett strahlt die Kraft am Steg ab —
  die Zungen klingen über drei Oktaven gleich laut (Spanne 37 → 1,4 LU). Zungenlängen nach Chapman 2006.
- **Clavinet (modal):** die Saiten tragen ihre Stärke je Taste (.032″ bis .009″), der Tonabnehmer hört dicke Saiten lauter
  als dünne; der Bass ist jetzt eben, die Senke an den dünnsten umsponnenen Saiten ist da wie am Instrument. Spanne 22 → 19 LU.
- Die drei Instrumente haben neue Pegel auf dem Bezug; in den mitgelieferten Loops klingt die Kalimba lauter, Wurlitzer
  und Clavinet leiser als bisher (die Loops werden mit LV1 nachgezogen).

## Bekannt

- Das Clavinet steigt im Diskant weiter an, weil der Tangentenweg für alle Tasten gleich ist und der Tonabnehmer die
  Geschwindigkeit der Saite hört; was das am Instrument bremst (Saitenlängen, Abstand der Tonabnehmer je Taste), nennt keine
  Quelle. Bleibt als Ausnahme mit Grund im Wächter.
- Glockenspiel und Xylophon brechen an den obersten Tasten ein (Ursache offen).

## Intern

- Kern `bar`: Zupfen mit Kraft (`pluckForce`), Abnahmen `slot` und `bridge`, Rechenfassung 2; Kern `struckString`:
  `stringDiameter` statt `stringMass`, magnetischer Tonabnehmer ∝ d², Rechenfassung 2. Entscheide WU1 a, WU2 a, KA1 a, CL1 a,
  CL2 a; Herleitung `docs/modellierung/anbindung-ic/2026-11-09_S183_vorgaben_je_taste.md`, Journal J60–J64. Drei IC-Abdrücke
  mit Vermerk neu (Beleg der Übernahme, kein Maßstab). Vier Quellen-PDFs unter `…\cmake\quellen`, Register +7.
- Wächter und Bereich der Loops 44 / 44 unter Release-clang und Testing-MSVC; kein voller Lauf.
- HoerTest_Helper statt HörTest_Helper an den lebenden Stellen (HT17 a).
