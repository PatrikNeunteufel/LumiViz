# Changelog Session 221 (2026-11-25)

> Diese Sitzung hat den Umlauf-Filter aus 34k-7c an Gitarre, Laute und Oud mit TK2 a nachgemessen und die längere
> Rechenzeit des Entwurfs aufgeschlüsselt (RZ2 a). TK2 a hält überall und bleibt; die Mehrzeit kommt aus mehr
> Schritten der Anpassung, nicht aus der Wärme.

## Neu

- **Messfall Gitarre, Laute, Oud mit TK2 a** (`proben/2026-11-25_S221_umlauf_cpp_tk2_saiten.md`, 1 h 49 min): alle
  Saiten halten UL2 a außer Laute Chor 8.1 (bekannte Ausnahme); der Kipppunkt von TK2 a greift nicht. Gegen den Lauf
  vor TK2 a (`…_S220_umlauf_cpp.md`) gleich bis auf Gitarre Saite 5 (je 0,01 Cent besser), Laute Chor 2.1 (anderer
  Start 1,30 statt 1,02, Bünde 0,07 statt 0,10 Cent, fremd 0,16 statt 0,14 Cent) und eine andere Dämpfung an Laute
  Chor 3.2 und 6.1 bei gleicher Güte.
- **RZ2 a:** `InstrumentMeasure` 0.5.1 — `UmlaufEntwurf` mit `EntwurfOptionen::rangverlust` (`KleinsteNorm` = TK2 a,
  Vorgabe; `WieScipy` nur für die Gegenprobe) und `Rechenaufwand` je Entwurf (Läufe der Anpassung, angenommene und
  verworfene Schritte, Auswertungen, Ableitungen, SVD, Rangverluste, Zeiten je Teil). Die Vorgabe rechnet wie 0.5.0.
- `test_InstrumentMeasureUmlauf`: neuer Fall „Rechenaufwand und Schalter Rangverlust (RZ2 a)“ (die Zähler gehen auf;
  Harfe T46 ohne Rangverlust rechnet in beiden Fassungen gleich: 4 Läufe, 48 Schritte, 13 verworfen); Prüfung der
  Zähler auch im Fall der gebundenen Tafel. Messfall mit `LUMIVIZ_34K7C_TASTEN=<T32,Saite 5,…>` und
  `LUMIVIZ_34K7C_RANGVERLUST=norm|scipy|beide`, dazu eine Tabelle „Rechenaufwand“.

## Geändert

- Die Meldung „geschrieben:“ des Messfalls zeigt den Pfad statt einer Adresse (`const char*` in doctest `MESSAGE`).
- `Solution.json`: `InstrumentMeasure` 0.5.1; `InstrumentMeasure.md` 0.5.1; Offene Punkte 1.432.0. **1939 Cases**.

## Zahlen

Cembalo T32, zwei Durchgänge (`proben/2026-11-25_S221_rechenaufwand_1.md`, `_2.md`; Start bei 96 / 66 °C):

| Fassung | s | angenommen | verworfen | Auswertungen | s in der SVD | Rangverluste | Cent | Abklingen % |
|---|---|---|---|---|---|---|---|---|
| TK2 a | 858 / 793 | 6708 | 1864 | 8598 | 843 / 780 | 6710 von 6710 | 1,22 | 3,78 |
| wie scipy | 250 / 255 | 1832 | 780 | 2638 | 246 / 251 | 1832 von 1832 | 1,23 | 4,11 |

- Die Wärme erklärt höchstens 8 %; die Zeit je Schritt ist gleich (0,125 s), TK2 a braucht 3,7-mal so viele.
- Ohne die Logit-Grenze wächst die Norm von x und der Abbruch nach xtol greift früher (wie S220 an T73). Mit TK2 a
  läuft die Anpassung länger und kommt etwas weiter. TK2 a bleibt, ZK1 a bleibt (der Entwurf läuft einmal je
  Instrument, offline); ein Hebel wäre eine schnellere SVD.

## Verifikation

- `*34k-7c*` 6 / 6 (12 144 Prüfungen) unter Release-clang und Testing-MSVC; kein voller Lauf (geändert ist nur der
  Entwurf, den nur sein eigener Test und der Messfall rufen).
