# Changelog Session 173 (2026-11-03)

> **Planungssitzung, nichts am Programm geändert.** Die Anbindung der Rechenkerne aus `InstrumentsCalculation` ist
> fertig geplant: alle Entscheide und der Teilplan der Stufe 34 stehen in `docs/audio/Instrumente_IC_Konzept.md` 0.3.0.

## Neu

- **Entscheide IK2-0 bis IK2-13 (Patrik):** die Rechenkerne kommen als zwei Bibliotheken unter `projects/libs/` (Kern
  und Messung, ohne Qt), mit einer Ton-Schnittstelle für alle Instrumentklassen; der Flügel zieht als erster Kern
  bitgleich um; die `.lvinst` bekommt Fassung 21 mit einer Tafel der Werte je Kern, aus der Datei, Creator und Setzen
  lesen; jede Übernahme steht in einem Journal, auch was verworfen wurde; Quellen mit Lizenzen als Register mit Wächter,
  aus Quellen ohne Erlaubnis wird nichts ausgeliefert.
- **Instrumente:** die IC-Instrumente tragen den Namen des echten Instruments; die heutigen Nachbildungen bleiben,
  `marimba` und `vibraphone` heißen künftig `synth_marimba` und `synth_vibraphone`, die Loops klingen gleich.
- **Teilplan Stufe 34 (34a–34l):** Skizze, CMakeCraft, Bibliotheken, Flügel, Messkette, Werkzeuge, dann die Klassen
  Platte/Stab, Membran, Klavier ohne Flügel (das bisherige Upright der Stufe 32), Zunge, Zupf-Saite; Abschluss. Der Rest
  von Stufe 31 folgt auf der Bibliothek.

## Intern

- Tests unverändert (**1753 Cases**), kein Bau. Korrektur einer Angabe aus S172 im Konzept: die pocketfft-Fassung in IC
  ist belegt und neuer (IK2-2-1 a).
