# Durchschlagende Zunge (Klasse 10) — der Kern `freeReed`

> **Seit:** S182 (2026-11-09), Auflösung von InstrumentsCalculation (AU1 a); gebaut S189–S192, Stufe 34j. **Zweck:** was
> LumiViz von der Klasse 10 rechnet, woher die Werte kommen, wie es geprüft ist und was es kostet. Die **Entscheide**
> stehen in [`audio/Instrumente_IC_Konzept.md`](../../audio/Instrumente_IC_Konzept.md) §6 (Teilplan 34j, ZV1 a–ZV14 a);
> die **Übernahme** je Baustein im [Übernahme-Journal](../anbindung-ic/Uebernahme_Journal.md) §6e (J66–J74).

## Was LumiViz rechnet

Ein Kern für alle Zungeninstrumente: je Zunge ein Schwinger, der den Spalt zur Platte öffnet und schließt (Strömung durch
den Spalt nach Bernoulli, Kanzelle als Luftvolumen, Kanal und Ventil dazwischen), angeregt vom Wind über die ganze Note;
mehrere Chöre je Taste, auf der Bass-Seite eigene. Die Größen je Taste sind Kurven der Tafel mit Quelle und Belegkraft
(Rechenfassung 2, ZV4 a ff.). Die Herleitung steht im Kopf von `InstrumentModels/zunge/Zunge.hpp` und in
[2026-11-11_S190_vorgaben_je_taste.md](2026-11-11_S190_vorgaben_je_taste.md).

| Datei | Instrument | Umfang | Besonderes | Art |
|---|---|---|---|---|
| `accordion` | Accordion | F1–A6 (29–93) | Diskant Musette (zwei Chöre 8′, 2 Hz Schwebung an A4 nach Hergert), Stradella-Bass unter F3 mit 8′ + 4′ (ZV6 a), Bend über das Ventil | keys |
| `harmonium` | Harmonium | C3–F6 (48–89) | ein Chor, Wind 0,2–1,5 kPa (Puranik & Scavone 2023), Gehäuse als Tiefpass 3 kHz | keys |
| `melodica` | Melodica | F3–C6 (53–84) | ein Chor, Atem 200–1600 Pa (Annahme), Bend über das Ventil | wind |
| `harmonica` | Harmonica | C4–C7 (60–96) | ein Chor, Atem 150–1000 Pa (Annahme; Millot & Baumann rechnen 570 und 783 Pa) | wind |

Alle vier sind **ungemessen** (Z2 a). Genre *Folk* (aus `free_reeds_demo`).

## Wind, Stärke und Spielweisen

- **Der Wind ist die Expression** (ZV3 a, SK4 a): `windMin` bei Expression 0, `windMax` bei 1, geometrisch dazwischen;
  die App gibt einem Kern mit dauernder Anregung (`KernAuskunft::anregungDauernd`) Expression, Pitch und Vibrato der Note
  als Spuren über die Zeit — ein Crescendo am gehaltenen Ton wird lauter und heller, die Tonhöhe fällt mit dem Druck
  (ZV5 a, bis −24 Cent bei ff).
- **Stärke je Stufe** (ZV10 a): je Taste gestimmt, bei welcher Stärke der Ton 2, 4 … 20 dB leiser klingt; unter der Dynamik
  der Taste 0 (Akkordeon bei −18 und −20 dB an jeder Taste, Melodica ab −16 dB). Mit Expression gilt Stärke mal Expression.
- **Nachklang je Taste** (ZV13 a): der längste Ausklang über Haltedauern 0,1–4 s und Stärken 0 / 0,5 / 1 bis −60 dB unter
  der lautesten Stelle der Note — Akkordeon bis 4,6 s im Bass, Mundharmonika bis 0,34 s.
- **Spielweisen:** `vibrato` (Cent; der Druck schwankt um ±0,2 % je Cent, 5 Hz), `pitch` (Cent über das Ventil, nur
  Akkordeon und Melodica, ZV7 a–ZV9 a), `bellows` (Richtung 0/1, am Wechsel ein Druckeinbruch; im Loop erst mit 31g).
  Als feste Werte je Drum-Spur (`coreTracks`) in den Loops der Nuancen.

## Loops

- `modelled_instruments/free_reeds_demo.lvloop` (`make_zungen_loops.py`; Folk, 104 bpm, 8 Takte): Akkordeon mit Melodie
  und Bass-Seite, das Harmonium mit dem Crescendo über die Expression am Clip (0,3 → 1 über sechs Takte), danach Melodica
  und Mundharmonika.
- Je Instrument ein Loop der Nuancen (`make_modelled_loops.py`): Lage, Stärke, Länge, Expression; *Vibrato* und an
  Akkordeon und Melodica *Pallet half closed*.

## Prüfung

- **Bezug:** `tests/unit/UnitTests/werte/zunge_ic.txt` — bis 34j-5 die Abdrücke aus IC (Beleg
  [`abdruck_zunge.bat`](../anbindung-ic/abdruck_zunge.bat), MSVC `/MD`), seit Rechenfassung 2 vom Kern geschrieben
  (`LUMIVIZ_ZUNGE_BEZUG_SCHREIBEN=1`, Vermerk J70): Abdrücke je Instrument tief/mitte/hoch, Spuren, Stufe; die gestimmten
  Kurven und die Bezüge. Geprüft im Kern (`test_InstrumentModelsZunge`) und an den mitgelieferten Dateien
  (`test_LoopZunge`), unter Release-clang und Testing-MSVC Bit für Bit gleich; **`clang-cl` braucht
  `/clang:-ffp-contract=off`**.
- **Wächter:** jede Taste spricht im Windbereich an (benannte Ausnahme F1 bei 200 Pa), der Bend erreicht seine Grenze, die
  Stärke je Stufe am Ton auf 1,8 LU (Akkordeon, Melodica) bzw. 0,3 LU nachgemessen, nach dem Nachklang jede Haltedauer
  still; `Loop Pegel LP3 a` ohne Ausnahme (über den Umfang 4,3 / 3,7 / 4,2 / 0,9 LU).
- **Messfälle:** `LUMIVIZ_34J_MESSUNG=<datei.md>` (Tonhöhe, Lautheit, Einschwingen, Amplitude je Taste und Wind; zuletzt
  [proben/2026-11-12_S192_messung_34j7.md](proben/2026-11-12_S192_messung_34j7.md)), `LUMIVIZ_ZV12_MESSUNG=<datei.md>`
  (Klangfarbe über dem Druck, [proben/2026-11-12_S192_messung_zv12.md](proben/2026-11-12_S192_messung_zv12.md)), alle vier
  stimmen mit `LUMIVIZ_34J_STIMMEN=1` (rund 41 s).
- **Hörliste** `hoertest/listen/zunge_34j` (`LUMIVIZ_34J_WAV`, Hörort `…\cmake\samples\Hoerproben\LumiViz\zunge_34j`):
  53 Klänge, 51 Punkte — je Instrument tief / mitte / hoch bei −18 / −9 / 0, Crescendo, Vibrato, Balgwechsel, Bass-Seite,
  Musette gegen einen Chor, die Befunde ZV14 a; löst IC `10_zunge` ab. **Ungehört.**

## Befunde (offen bis zur Hörprobe)

- **Unterer Diskant mit dem Wind dunkler** (ZV14 a): Akkordeon E3–A3 und Melodica F3–A3 0,77–0,98-fach; das Ventil drosselt
  dort (mit Ventil × 4 heller 1,15–1,23). Befund
  [2026-11-12_S192_zv12_klangfarbe_ueber_druck.md](2026-11-12_S192_zv12_klangfarbe_ueber_druck.md); der übrige Befund
  „lauter wird dunkler“ (S191) war ein Messfehler (Teiltöne an der Sollfrequenz gelesen).
- **Amplitude des Harmoniums über dem Druck:** der Kern wächst 1,5- bis 2-fach, gemessen fast gleich (Cottingham u. a.
  1999).
- Nicht im Bestand: Ricot, Caussé & Misdariis 2005, Koopman & Cottingham 1997, Llanos-Vázquez u. a. 2014.
- Nicht in 34j (gebilligt S188): Register als Spur, Bisonorik und Bellows shake, Prolongement und Percussion des
  Harmoniums, Bending und Overblow der Mundharmonika, Cassotto und Verdeck.

## Rechenzeit (IK2-6 a)

Gemessen am 2026-11-12 (S192) unter Release-clang, **unter Drosselung** (die Kühlung ist noch nicht getauscht, S181 —
nicht gültig für RZ1-1 a): ff, 2 s gehalten, das Beste aus drei Läufen.

| Ton | Anteil der Echtzeit |
|---|---|
| Akkordeon A4 (Musette, zwei Chöre) | 1,25 % |
| Akkordeon F2 (Bass-Seite 8′ + 4′, der schwerste Ton) | 1,27 % — **0,43 × Flügel-Kern** (A0 unter derselben Last 2,94 %) |
| Akkordeon F1 | 1,25 % |
| Harmonium, Melodica, Mundharmonika an der Mitte | 0,63–0,64 % |

Die Grenze 4,6 % wird nach dem Tausch der Kühlung dreimal allein nach der Taktprobe nachgeprüft (`*34j-9 Rechenzeit*`).

## Eigene Berichte

- [2026-11-11_S188_befund_ic_vorlage.md](2026-11-11_S188_befund_ic_vorlage.md) — die IC-Vorlage am gerechneten Ton
  gemessen, vor dem Teilplan 34j: Bass-Seite stumm, Harmonium unten stumm, Lautheit über den Umfang 15–53 LU, Bend
  wirkt nicht; Gegenprobe „ein Winddruck für alle Tasten“. Proben unter [proben/](proben).
- [2026-11-11_S190_quellenlage_34j-4.md](2026-11-11_S190_quellenlage_34j-4.md) — Quellenlage nach der Recherche
  (34j-4): keine Maßtabelle je Ton, gegengelesene Anker (Akkordeonzungen, Harmoniumzunge C3, Schwellentheorie nach
  Fletcher), Stand je Instrument.
- [2026-11-11_S190_vorgaben_je_taste.md](2026-11-11_S190_vorgaben_je_taste.md) — Herleitung der Vorgaben je Taste
  am Prüfstand (34j-5): Formeln für Schließdruck und Anfachung, Kern-Form gegen das Modell nach Adachi, Gesetze über
  den Umfang für Akkordeon (Diskant, Bass), Harmonium, Mundharmonika, Melodica, Bend über das Ventil; Prüfstand und
  Ergebnisse unter [proben/](proben).
- [2026-11-12_S192_zv12_klangfarbe_ueber_druck.md](2026-11-12_S192_zv12_klangfarbe_ueber_druck.md) — ZV12: die
  Klangfarbe über dem Druck, Messfehler und Rest am unteren Diskant.
- Messungen am Kern: [proben/2026-11-11_S191_messung_34j6.md](proben/2026-11-11_S191_messung_34j6.md),
  [proben/2026-11-12_S192_messung_34j7.md](proben/2026-11-12_S192_messung_34j7.md),
  [proben/2026-11-12_S192_messung_zv12.md](proben/2026-11-12_S192_messung_zv12.md).

## Recherchen (Agentenberichte, unverändert)

- [recherche/2026-11-11_S190_akkordeon_zungenmasse.md](recherche/2026-11-11_S190_akkordeon_zungenmasse.md)
- [recherche/2026-11-11_S190_harmonium_mundharmonika_melodica.md](recherche/2026-11-11_S190_harmonium_mundharmonika_melodica.md)
- [recherche/2026-11-11_S190_schwelle_amplitude_druck.md](recherche/2026-11-11_S190_schwelle_amplitude_druck.md)
- [recherche/2026-11-12_S192_klangfarbe_ueber_druck.md](recherche/2026-11-12_S192_klangfarbe_ueber_druck.md)

## Aus InstrumentsCalculation

- [ic/](ic/README.md) — die Vorarbeiten aus IC unverändert (Modellentwurf, Modell, Messplan, offene Fragen,
  Recherchen); Wortlaut wie in IC, nur die Links gerichtet.
- Rechenkern als Vorlage: `harvest/instruments-calculation/rechnung/klassen/10_zunge/` — **seit S192 gelöscht** (übernommen);
  zuletzt im Commit `6e9f4756` (für `abdruck_zunge.bat` und die Proben S188 von dort holen).
- Hörliste aus IC: `10_zunge` — abgelöst durch `zunge_34j` (S192, wie LG1 a), aus dem Repo entfernt; am Hörort
  `…\cmake\samples\Hoerproben\IC\10_zunge` mit `abgeloest.txt`.
- Fremde Arbeiten: `…\cmake\quellen\zunge\` (28 Dateien seit S190, Herkunft je Datei in `LADELISTE.tsv` dort; Index
  `../../archiv/Index_quellen.md`).
