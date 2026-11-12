# ZV12 a: Klangfarbe über dem Druck am Kern `freeReed` — Befund und Vorlage ZV14

> **Datum:** 2026-11-12 (Session 192) · **Auftrag:** ZV12 a (Konzept `docs/audio/Instrumente_IC_Konzept.md` §6)
> **Messung:** [`proben/2026-11-12_S192_messung_zv12.md`](proben/2026-11-12_S192_messung_zv12.md) (`LUMIVIZ_ZV12_MESSUNG`
> in `test_InstrumentModelsZunge`, unter einer Sekunde) · **Quellen:** Agentenbericht
> [`recherche/2026-11-12_S192_klangfarbe_ueber_druck.md`](recherche/2026-11-12_S192_klangfarbe_ueber_druck.md), die
> tragende Aussage bei Puranik & Scavone 2023 (FA 2023) selbst nachgelesen (PDF S. 2–3, gedr. S. 1810–1811).

## 1. Der Befund S191 war größtenteils ein Fehler der Messung

S191 las die Teiltöne bei h · Sollfrequenz der Taste (`band(t, …, h * hzVon(n))`, ebenso `schwerpunktHz(x, …, 440.0)` in
`test_LoopZunge`). Die Tonhöhe folgt aber dem Druck (ZV5 a): bei pp bis +4,8, bei ff bis −24 Cent. Das Fenster (Hann,
9 600 Werte) ist rund ±10 Hz breit; der 20. Teilton von A4 liegt bei −16,7 Cent schon 85 Hz daneben. Die hohen Teiltöne
fielen bei ff aus dem Fenster — ein Ton wirkte dunkler, als er ist.

An der **gemessenen** Tonhöhe (Nulldurchgänge der Auslenkung) gelesen, Schwerpunkt der Teiltöne 1–20, Stärke 1 gegen 0,
jede vierte Taste:

| Instrument | heller bei ff — an der Sollfrequenz (S191) | an der gemessenen Tonhöhe | Verhältnis ff/pp |
|---|---|---|---|
| Akkordeon, erste Zunge | 4 von 17 | 11 von 17 | 0,84 … 1,64 |
| Akkordeon, ganzer Ton | 10 von 17 | 12 von 17 | 0,77 … 1,70 |
| Harmonium | 4 von 11 | **11 von 11** | 1,25 … 1,74 |
| Melodica | 1 von 8 | 6 von 8 | 0,84 … 1,50 |
| Mundharmonika | 4 von 10 | **10 von 10** | 1,02 … 1,20 |

Melodica A4, im Crescendo S191 1 394 → 895 Hz: an der gemessenen Tonhöhe 1,22-fach heller.

## 2. Der Rest: Akkordeon E3–A3, Melodica F3–A3

Dunkler bleiben die Tasten 41–57 des Akkordeons (Bass-Seite oben und Diskant unten; Melodica 53 und 57 mit denselben
Gesetzen): 0,84–0,98. Gerade dort **schrumpft die Amplitude mit dem Wind** (an 53: 3,00 → 2,37 Ruheöffnungen; an 45:
3,29 → 3,20), und die Tonhöhe fällt am meisten (−17 bis −19 Cent ff gegen mf).

**Ursache am Kern — das Ventil** (Varianten an 45 / 49 / 53 / 57, je eine Größe der Taste geändert):

| Variante | ff/pp Schwerpunkt | Amplitude/Ruhe pp → ff | Cent ff gegen mf |
|---|---|---|---|
| wie gestimmt | 0,84 … 0,98 | 3,0 → 2,4 (53) | −12 … −19 |
| Ventil × 4 | 1,15 … 1,23 | 3,7 → 6,2 … 7,5 | −3 … −5 |
| Ventil × ¼ | (spricht bei pp kaum) | 0,0 → 0,2 … 0,8 | −38 … −89 |
| Kanzelle × 4 / × ¼ | 0,84 … 1,00 | wie gestimmt | wie gestimmt |
| Kanal-Länge × 4 | 1,31 … 1,49 | 2,5 → 3,9 … 4,7 | −8 … −20 |
| Kanal-Länge × ¼ | 0,84 … 1,01 | 2,5 → 1,1 … 1,5 | −1 … +2 |
| Dämpfung × 4 | 0,98 … 1,21 | 2,0 → 1,6 … 2,1 | −9 … −21 |
| Ruheöffnung × 2 | 1,19 … 2,17 | 2,0 → 3,0 … 3,4 | −16 … −18 |

Mit mehr Wind drosselt das Ventil den Zufluss in die Kanzelle; die Zunge bekommt kaum mehr Antrieb, ihre Ruhelage wandert
in den Schlitz (−0,1 → −0,8 Ruheöffnungen), die Amplitude bleibt oder fällt. Die Ventilfläche ist die Größe, die ZV5 a
(Satz D3, S190) aus der gemessenen Tonhöhe über dem Druck abgeleitet hat (Adachi u. a. 2026, Abb. 5: −16 Cent/kPa an
C4; der Kern −11).

## 3. Was die Quellen sagen

- **Kein gemessenes Spektrum des Schalls über dem Druck** in den 28 Arbeiten unter `…\cmake\quellen\zunge\` (alle
  durchsucht). Ein Schwerpunkt über dem Druck steht nirgends.
- **Die Zungenbewegung** wird mit dem Druck obertonreicher (gemessen: Puranik & Scavone 2023, Harmonium B5, 0,2 →
  1,4–1,6 kPa; Bahnson u. a. 1998 ohne Daten) — die Richtung „lauter wird heller“, an der Zunge, nicht am Schall.
- **Die Amplitude** bleibt an Harmoniumzungen nach der Schwelle fast gleich und fällt bei hohem Druck um rund 15 %, die
  Ruhelage wandert in den Schlitz (gemessen: Cottingham, Lilly & Reed 1999); am großen Versuchsventil steigt sie (Tarnopolsky
  u. a. 2000), im Modell von Adachi u. a. 2026 auch.
- **Der Bend** (Ventil halb zu) macht den Schall leiser und dunkler (gemessen: Elejalde-García u. a. 2021, Llanos-Vázquez
  u. a. 2008) — derselbe Mechanismus wie der Rest in §2: weniger Zufluss, dunkler.
- Nicht im Bestand und vermutlich entscheidend: Ricot, Caussé & Misdariis 2005 (JASA 117), Koopman & Cottingham 1997,
  Llanos-Vázquez u. a. 2014 (Acta Acustica 100).

## 4. Zielgrößen je Option (Vorlage ZV14)

| Zielgröße | a Kern bleibt, Messung richtig | b Ventil im unteren Diskant ×4 | c erst Quellen laden |
|---|---|---|---|
| Tonhöhe über Druck (gemessen −16 Cent/kPa an C4) | wie ZV5 a (−11) | an F3–A3 rund −3 … −5 Cent über den Wind statt −17 | wie a bis zur Antwort |
| Amplitude über Druck (gemessen: fast gleich, oben −15 %) | an E3–A3 −3 … −21 % (nahe der Messung), Harmonium +50 … +100 % (Abweichung, offen) | an F3–A3 +70 … +130 % (weiter weg) | wie a |
| Helligkeit über Druck (nur an der Zunge belegt: heller) | ganzer Ton 39 von 46 geprüften Tasten heller, Rest 0,77–0,98 | Rest 1,15–1,23 | offen |
| Bend über das Ventil (ZV7 a, gemessen an Adachi) | wie gestimmt | Kurven neu zu stimmen, Bend an F3 flacher | wie a |
