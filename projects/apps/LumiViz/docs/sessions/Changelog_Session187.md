# Changelog Session 187 (2026-11-11)

> **Die Klaviere spielen auch ganz leise im Diskant.** Der Hammer rechnet mit belegten Werten für den Filz; Töne, die bei
> ppp in der Höhe stumm blieben, klingen jetzt.

## Geändert

- **Flügel, Upright, Honky-tonk, CP-70, Celesta, Cimbalom:** der Filz des Hammers nach Kartofelev & Stulov 2014 (Hysterese
  und Steife je Taste), der Kontakt feiner gerechnet. Der leise Anschlag im hohen Diskant ist so laut wie an der Aufnahme
  eines Yamaha C5 (*Salamander*); die Pegel der fünf Instrumente stehen wieder auf dem Bezug.
- **Celesta-Loop:** die Spielweisen *Half damper* und *Damper off* spielen an C5, wo der Dämpfer hörbar wirkt.
- Drei Loops mit Kern-Instrumenten minimal leiser oder lauter, damit ihre Spitze unter −1 dBFS bleibt.

## Entschieden

- **HS-2 a** (Umfang an den Teiltönen nachmessen), **HS-3 a** (der belegte Satz in den Kern), **HS3-1 a** (Celesta-Spielweisen
  an Taste 72), **HS3-2 a** (Hörliste `pv1` als Stand vor HS-3 abgeschlossen), **HS-4 a**, **HS4-1 a** (Helligkeit je Stärke
  untersucht).

## Bekannt

- Im Diskant wird der Hammer mit der Stärke zu wenig heller (im Leisen zu hell, im Lauten zu dunkel). Kein geprüftes
  Filzgesetz löst das; der Punkt ruht, bis die Originalarbeiten erreichbar sind.

## Geprüft

- Voller Lauf 1860 / 1860 unter Release-clang und Testing-MSVC nach dem Umbau; Hörlisten `klasse1_34i`, `stab_34g` gültig.
