# Changelog Session 211 (2026-11-21)

> Diese Sitzung hat die Harfe gebaut: der Boden mit Drehfeder an den Bassseiten, die Saiten als Federn an ihren Ösen.
> Beim Bau zeigte sich, dass die Lautheit aus der Suche an einer Näherung gemessen war; am gebauten Körper verfehlt die
> Harfe den größten Sprung deutlich, die Wahl der Fassung ist neu zu treffen.

## Neu

- **KQ51 a:** Fassung F4 — Drehfeder 7 000 N an beiden Seiten vom Bassende bis 0,6 m, Mündung 2,3.
- **KQ52 a:** die Saiten als Federn an den Ösen, 0,3 · E·A/L, ohne Verlust.
- **Die Harfe gebaut:** `make_harfe.py`, `concert_harp.lvinstbodyplan`, `concert_harp.lvinstbody` (129 Moden).
- **KQ53 a:** `.lvinstbody` Fassung 6 mit `springShare` je Mode; `InstrumentBody` 0.26.0; der Wächter über die
  mitgelieferten Körper rechnet die Dämpfung mit dem Anteil der Federn nach.
- **KQ54 a:** der Fall der Harfe rechnet die Platte ohne Saiten (KQ31-2 a, KQ40 a) und mit Saiten (Abstrahlung an den
  Saiten 33–22 mit 16 Hörorten) selbst.

## Geändert

- Alle sieben Körper in Fassung 6 (sechs gleich bis auf `version` und `springShare`), `make_gitarre.py` auf Fassung 6.
- Harfenbericht `2026-11-13_S197_harfe_gelesen.md` §20.5–§20.7.
- `docs/audio/Instrumente_IC_Konzept.md` 0.80.0, `docs/Offene_Punkte.md` 1.422.0.

## Zahlen

**Faktor der Saiten an F4** (Verschiebung beim Besaiten, Bell +10 / +3 / +5,7 / −0,5 / +1,8 %):

| Faktor | S(1,1)− | S(1,2) | S(1,3) | S(1,4) | S(1,5) | Σ Quadrate (5 / 3) |
|---|---|---|---|---|---|---|
| 0,2 | +3,8 % | +4,3 % | +2,4 % | +1,4 % | +0,8 % | 56 / 51 |
| 0,3 | +5,3 % | +6,5 % | +3,6 % | +2,1 % | +1,2 % | 46 / 39 |
| 0,4 | +6,5 % | +8,7 % | +4,8 % | +2,8 % | +1,6 % | 57 / 46 |

**Die gebaute Harfe** (F4, 0,3). Ohne Saiten: Moden mit Luft 164,4 / 218,9 Hz (Q 13,9 / 14,7), S(1,2)–S(1,6) −3,1 /
+6,5 / +2,3 / +4,1 / +0,2 %, alle Bäuche in der Lage innerhalb 2 Messorten. Mit Saiten: 173,1 / 228,7 Hz, Abstrahlung
an den Saiten 33–22 hält 6 / 12. Lautheit je Taste (Grenze 22 / 12 / 3 LU):

| | nur gespielte Saite | mit Mitschwingen |
|---|---|---|
| gebauter Körper | 17,6 / 15,6 / 2,7 | 18,1 / **15,7** / 2,3 |
| Suche (Modenraum, gilt nicht) | 18,4 / 12,7 / 2,8 | 19,2 / 12,8 / 2,5 |

Der Modenraum der Suche führt Luftanteil und Dämpfung nur auf der Diagonale: an den Moden mit Luft 0,50 / 0,45 und
Q 15,1 / 16,7 statt 0,65 / 0,34 und Q 11,9 / 19,0.

## Tests

`*Loop Koerper*,*Loop Werte*,*Klangfassung*,*Hammer*` ohne `*KQ18 Drehfeder*,*KQ20*` 112 / 112 unter Release-clang
und Testing-MSVC, kein voller Lauf. **1924 Cases.**

## Offen

- KQ51 neu vorlegen: F1–F3 an der vollen Rechnung messen (rund 3 h), dann alle vier Fassungen gegen alle Zielgrößen.
