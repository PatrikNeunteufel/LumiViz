# Modell-Entwurf — Membran (Klasse 5)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Körper [gemeinsam/koerper.md](../../koerper/ic/koerper.md) (Luftlast, F-K1), Messkette [gemeinsam/messkette.md](../../messung/ic/messkette.md)
> (B6 Modalfit, B7 Band über der Zeit, B8 Gleiten), Bestand in LumiViz (IC-B1 a). Berichte: `recherche/2026-10-26_physik_rechenverfahren.md`,
> `gemeinsam/recherche/2026-10-26_spielweisen_schlagwerk.md`, `…_aufnahmen_schlagwerk.md`.
> **Kennzeichnung:** **nachgelesen** · **nur Bericht** · **Einschätzung** · **offen**. Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand in LumiViz

Nachgelesen, `Loop_Konzept.md`:
- **§4d Drum-Editor:** Spuren mit Schritten, je Schritt **eine Lautstärke** („Step volume“); kein Feld für Spielweisen
  am Schritt.
- **K1 a, K5 a:** jede Drum-Spur trägt eine **Rolle** — „kick, snare, clap, rim, hat, open hat, ride, crash, tom (tom 1,
  tom 2 …), perc (perc 1, perc 2 …), fx“; **K3 a:** `.lvkit` nennt je Rolle einen Klang mit Volume und Pan.
- **11f:** sechs gerechnete Drum-Instrumente (`drum_kick`, `drum_tom`, `drum_snare` …, `.lvinst` Art `drums`) aus
  Synth-Schichten, „Alle sechs sind für die Note C2 gebaut“; gegen das Kit gemessen, nicht gehört.
- **§4a:** ein Klang des Kits wird höchstens 30 s geladen.
- **D1 a / D1b / D6 a:** 37 DrumGizmo-Anschläge (25/60/90 % der `power`-Spanne) als `lib:drs_*`, ganze Länge.

Nachgelesen, `Spielweisen_Konzept.md`: Zielbild Schlagwerk — „Wirbel, Abdämpfen mit der Hand (Becken, Vibraphon)“;
SF8 a — Wirbel und Abdämpfen je Note; **HA3 a** — beim Hammer „Die Anschlagstärke gestuft im Namen, der Rest als Pegel“.
Membran steht nicht in SF10 a (Erweiterung, `KLASSEN.md`).

## 2. Kern

1. **Fell modal mit Luftlast** (Einschätzung, nach dem Muster des Flügels): je Fell eine Modenliste (Frequenz, T60,
   Modenform an Schlag- und Abnahmestelle), **vorab** gerechnet aus Green-Funktion/BEM oder FEM mit Kessel, oder an
   Aufnahmen gesetzt (B6). Die Luft führt die (m,1)-Moden der Pauke an die harmonische Reihe — „close to the harmonic
   ratios 2 : 3 : 4 : 5“ (Christian et al. 1984, nur Bericht). Prüfziel Pauke 64 cm: gemessen (0,1) 140 Hz, (1,1) 172,
   (2,1) 258, (3,1) 340, (4,1) 420, (5,1) 501 Hz (Packer 1993, Tab. 7.1, nur Bericht, nur Zahlen). Luft bei
   Knotendurchmessern als Massen-, bei Kreisknoten als Steifezuschlag (Avanzini & Marogna 2010, nur Bericht).
2. **Zwei Felle** über eine konzentrierte Luftfeder zwischen den (0,m)-Moden (Avanzini & Marogna, nur Bericht); jede
   Mode spaltet in ein Paar (Snare, Wagner 2006: (0,1) 188 Hz allein → 240/366 Hz mit Resonanzfell, nur Bericht).
   Doubletten bei n > 0 fehlen in diesem Modell (Risiko).
3. **Schlag:** Stock oder Schlägel als Masse mit Hunt-Crossley- bzw. Hertz-Kontakt in modalen Koordinaten (nur Bericht);
   Kontaktzeit „5–8 ms for a mezzoforte stroke at the center of a tom tom … or snare drum“ (Dahl 2011, nachgelesen
   PDF-S. 2); Kraft „from 3 N (piano) up to slightly over 100 N (forte)“ (Wagner 2006, nachgelesen PDF-S. 30).
4. **Tonhöhengleiten** über Spannungsmodulation aus der Energie, unterabgetastet — Zusatzaufwand „negligible“ (Marogna,
   Avanzini & Bank 2010, nachgelesen PDF-S. 1); gemessen an der Snare etwa +3 Hz bei 140 Hz von leise zu laut (≈ 37
   Cent, nur Bericht); bei der Pauke „a minor effect in timpani“ (Bilbao & Webb, nachgelesen PDF-S. 6).
5. **Hohe Frequenzen statistisch:** Moden bis 10 kHz ≈ 2400 (Snare) bis 4900 (Pauke) (eigene Rechnung im Bericht);
   oberhalb von etwa 4 kHz dicht und schnell abklingend, als Anschlagsgeräusch wahrgenommen (Avanzini & Marogna, nur
   Bericht) → statistischer Teil wie KB1 a.
6. **Snare-Teppich** (F-6): Mischform — modale Felle, wenige Drahtmoden mit Straf-Kontakt (Torin, Hamilton & Bilbao
   2014; Bilbao et al., nur Bericht) und ein Rauschanteil, gesteuert von der Auslenkung des Resonanzfells
   (Einschätzung). Volle FDTD = Deluxe.

**Deluxe:** 3D-FDTD mit Luft — „a computation time of 82 seconds“ je s Ton auf der GPU (Bilbao & Webb, nachgelesen
PDF-S. 6), 28 min in MATLAB auf der CPU (nur Bericht).

## 3. Je Instrument

| Instrument | Aufbau | Datenlage | Risiko (Einschätzung) |
|---|---|---|---|
| **Tom** | zwei Felle, Kessel, Luft | gut — Tom-Tom-Datensatz (16 Stimmungen, Pendel), DrumGizmo viele Stärken | gering–mittel (Doubletten) |
| Bassdrum | zwei Felle, großes Volumen, Kontakt Schlägel | mittel — DrumGizmo; keine Modenzahlen gefunden | mittel |
| **Pauke** | ein Fell, Kessel, Luftlast entscheidend | gut — Packer (Moden und T60 gemessen und gerechnet), VCSL/VSCO | gering für den Ton; Glissando offen (F-5) |
| Snare | zwei Felle, Teppich | gut (DrumGizmo 98 Anschläge) | **hoch** (Teppich) |
| Conga, Bongo | ein Fell, offener Kessel | schwach — keine Modenmessung | mittel–hoch |
| Tabla | belastete Membran (syahi, „approximately eight times as dense“, nur Bericht) | Eigenwerte aus Sathej & Adhikari (Experiment Raman: 1,03 / 2 / 3 / 4 / 5,04 …, nur Bericht); Luftlast fehlt | mittel |

## 4. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| Fell: Spannung, Flächendichte, Radius | fest je Instrument (Stimmung = Spannung) | Mylar 0,262 kg/m² (Packer; in sich widersprüchlich, nur Bericht), Pauke 5360 N/m (nur Bericht) | Modenfrequenzen (M1) |
| Moden mit Luftlast (Frequenz, T60, Form) | fest | Rechnung (Green/BEM); Packer als Prüfziel | Modalfit an Einzelschlägen (M1, M2) |
| Kopplung zweier Felle | fest | konzentrierte Luftfeder (nur Bericht) | Modenpaare (M1) |
| Schlag: Masse, Kontaktsteife, Exponent | fest je Schlägel | Schlägel 0,028 kg, K = 1,6·10⁸, α = 2,54 (Bilbao et al., nur Bericht) | Kontaktzeit, Helligkeit über die Stärke (M3) |
| Auftreffgeschwindigkeit | Stärke (F-2) | Akzent ≈ doppelte Geschwindigkeit (Dahl, nachgelesen PDF-S. 5); Lautstärke ∝ v² als Abbildung (Dolhansky, nur Bericht) | Pegel und Spektrum über die DrumGizmo-`power` (M3) |
| Schlagstelle | Artikulation (F-3) | Mitte: tiefe Moden; Rand: höhere (nur Bericht); Pauke ¼ Radius vom Rand (nur Bericht) | `snare-position` (wenn frei), Aasimonster Mitte/außermittig (M4) |
| Spannungsmodulation | aus der Energie | Marogna et al. (nachgelesen) | Gleiten (M5, B8) |
| Teppich | Mischform (F-6) | Torin et al. (nur Bericht) | Rauschanteil mit/ohne Teppich (M6, VCSL `HitSN`/`HitNS`) |
| Wirbel | Artikulation (F-4) | Buzz: Gruppen im Abstand „250msec to 350msec by professional players“ (Miura 2023, nachgelesen PDF-S. 4), 4–9 Aufschläge je Gruppe (nur Bericht) | VCSL/VSCO-Wirbel (M7) |
| statistischer Teil oben | Kurve | offen | Restsignal, B4/B7 |

## 5. Rechenzeit

Modal 2000–5000 Moden bei 44,1 kHz ≈ 2·10⁸ Operationen je s Ton (eigene Rechnung im Bericht, keine Messung) —
deutlich unter Echtzeit; mit statistischem Teil oberhalb 4–5 kHz weniger. Vorab gerechnet je Stärkestufe (F-2).

## 6. Risiken

- Snare-Teppich (Kosten, Treue); Doubletten bei Toms und Snare; Dämpfung — gerechnete und gemessene T60 bei Packer
  weichen bei (4,1)/(5,1) bis Faktor 6 ab (nur Bericht) → Dämpfung wird gemessen, nicht gerechnet.
- Rechte: nur wenige Quellen frei lizenziert; Packer nur Zahlen.

## 7. Abweichungen vom Rahmen

- Schritte tragen heute nur eine Lautstärke; Schlagstelle, Rimshot, Wirbel brauchen einen Ort am Schritt (F-3, F-4).
- Die Pauke mit Glissando passt nicht auf eine Rolle mit fester Note (F-5).
