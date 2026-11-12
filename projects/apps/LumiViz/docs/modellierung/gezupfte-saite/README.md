# Gezupfte Saite (Gitarre, Bass, Ukulele, Violine gezupft)

## Entscheide (SSOT)
`docs/audio/Saiten_Konzept.md` — Verfahren *Classic*, *Extended KS*, *Waveguide*, Tonabnehmer, Korpus, Spielweisen
(S132–S138).

## Recherchen
| Datei / Ordner | Thema |
|---|---|
| [recherche/2026-10-18_S132_korpus_tonabnehmer_messwerte.md](recherche/2026-10-18_S132_korpus_tonabnehmer_messwerte.md) | Korpus (Wright 1996, Hess 2013, Brown 2004, Jansson, Bissinger 2008) und Tonabnehmer (Zollner, Zollner/Zwicker 2019) — bis S146 Saiten_Konzept §6, §6b |
| [recherche/2026-10-18_S134_spielweisen_vorgaben_24c/](recherche/2026-10-18_S134_spielweisen_vorgaben_24c/bericht.md) | Vorgaben Up level, Mute, Tap level, Touch; Messungen an IDMT-SMT-Guitar/-Bass, Skripte und Rohtabellen (bis S146 unter `.claude/handover/recherche_24c/`) |
| [recherche/2026-10-19_S136_saitenabstand_spread_24f/](recherche/2026-10-19_S136_saitenabstand_spread_24f/bericht.md) | Saitenabstand je Bauart, Vorgabe Spread — Bericht (bis S146 `.claude/handover/recherche_24f/`) |
| [recherche/2026-10-19_S136_saitenabstand_spread_befund.md](recherche/2026-10-19_S136_saitenabstand_spread_befund.md) | der Befund daraus — bis S146 Saiten_Konzept §6c |
| [recherche/2026-10-19_S136_verbundene_noten_25/](recherche/2026-10-19_S136_verbundene_noten_25/bericht.md) | Hammer-on, Pull-off, Slide, Bend: Literatur und Messungen, Skripte (bis S146 `.claude/handover/recherche_25/`) |
| §3 S-K9 a, §5o in `docs/audio/Saiten_Konzept.md` | Luftraum der Klassikgitarre nach Wright (Satz TOP26); Herleitungen der Vorgaben der Spielweisen |

Die Berichte der Agenten aus S132/S133 (Korpus, Saitenverfahren) sind nicht abgelegt worden; ihr Befund steht in der
Datei zu S132.

## Quellen (nur lokal, `…\cmake\quellen\gezupfte-saite\`, seit S182; Index `../../archiv/Index_quellen.md`)
Wright 1996, *The Acoustics and Psychoacoustics of the Guitar* (Diss. Cardiff) — „All rights reserved“ auf
hakwright.co.uk, deshalb seit S146 nicht mehr versioniert (vorher unter `docs/audio/quellen/`).

## Noch offen
Der Gitarrenkorpus nach KB1 a auf die gemeinsame Grundlage (`../koerper/`).

## Befunde
- [2026-11-12_S192_befund_ic_vorlage.md](2026-11-12_S192_befund_ic_vorlage.md) — die IC-Vorlage der Klasse 2 am Ton
  gemessen, vor dem Teilplan 34k: keines der fünf Instrumente hält 15 LU über den Umfang (18,6–36,1 LU), der Tastenstoß
  des Cembalos ist stumm, die Harfe im Clip braucht 14 % der Echtzeit. Programm und Rohausgabe unter [proben/](proben).
- [2026-11-12_S193_gegenprobe_ursachen.md](2026-11-12_S193_gegenprobe_ursachen.md) — woran das Gefälle hängt: der Zupf
  als Kraftstoß fester Dauer und wenige scharfe Stegmoden (mit beidem behoben Sprünge 1,9–4,2 LU statt bis 21,8 LU);
  Mitschwingen je Note ist linear und kostet an der Gitarre 1,1–1,3 %, an der Harfe mit allen Saiten 9,4–9,9 % der
  Echtzeit. Programme und Rohausgaben unter [proben/](proben).

- [2026-11-12_S195_ziele_aufnahmen.md](2026-11-12_S195_ziele_aufnahmen.md) — Ziele an Aufnahmen (VCSL, Iowa MIS) mit
  der Messkette des Wächters: Cembali 6,1–13,6 LU über den Umfang, Konzertharfe 17,8 LU (ab D2 10,8 LU), Sprünge bis
  11,3 LU; Ausklang und Dynamik je Lage. Laute und Oud ohne Aufnahme.
- [2026-11-12_S195_probe_statistik_ohne_bauplan.md](2026-11-12_S195_probe_statistik_ohne_bauplan.md) — Probe 34k-5: der
  statistische Teil von `InstrumentBody` liefert ohne Bauplan keine Moden; Plattenformeln treffen nur die Größenordnung.
- [2026-11-12_S195_oud_erster_fall.md](2026-11-12_S195_oud_erster_fall.md) — erster Fall nach KQ6 a: der Ersatz-Bauplan
  der Oud trifft die erste gemessene Mode, mit keiner Deckendicke alle vier innerhalb von 10 % (ohne Schalllöcher).

## Recherche S195 (34k-4)
Unter [recherche/](recherche): vier Agentenberichte (`…_cembalo_quellen.md`, `…_harfe_quellen.md`,
`…_laute_oud_quellen.md`, `…_klassische_gitarre_quellen.md`, unverändert), die
[Ladeliste](recherche/2026-11-12_S195_ladeliste.md) mit dem Stand nach QZ1 a und der
[Gegenlese-Befund](recherche/2026-11-12_S195_gegenlesen.md) — **maßgebend ist, was dort am Original bestätigt ist**; was
nur im Agentenbericht steht, ist nicht geprüft. Die Dateien liegen unter `…\cmake\quellen\gezupfte-saite\` (41 Dateien,
`SHA256SUMS`).

## Stufe 34k (Cembalo, Harfe, Laute, Oud, Gitarre auf dem Kern `pluckedString`)
Entscheide GZ1 a–GZ4 a, GZ1-1 a und der Teilplan 34k-1 bis 34k-11 stehen in `docs/audio/Instrumente_IC_Konzept.md` §6
(S193); dort auch die Haltepunkte 34k-4 und 34k-5 mit KQ1 a bis KQ7 a (S195): Harfe nach Bell 1988 (Salvi „Orchestra“,
46 Saiten, Bauplan), Cembalo Taskin 1769, Gitarre mit Modalfit am Archiv Mores, Laute und Oud mit Ersatz-Bauplan;
`InstrumentBody` bekommt Löcher (KQ7 a).

## Aus InstrumentsCalculation (S182)

- [ic/](ic/) — die Vorarbeiten aus IC zu Klasse 2 (Cembalo, Harfe, Gitarre, Laute, Oud; Stufe 34k) (Auflösung von IC, AU1 a / AU1-1 a): Wortlaut unverändert, nur die Links gerichtet.
