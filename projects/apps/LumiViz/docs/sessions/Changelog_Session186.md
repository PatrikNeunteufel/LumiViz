# Changelog Session 186 (2026-11-10/11)

> **Die Hörlisten sind bereit zum Hören, und der leise Anschlag im Diskant ist untersucht.** Jede Liste hat den richtigen
> Pegel und keinen Klang doppelt; der Hammer der Klaviere hat einen belegten Fehler in der Hysterese.

## Geändert

- **Hörlisten:** jeder Klang auf dem Pegel seiner Liste (die IC-Listen nachgeregelt); keine stillen Klänge, keine ohne
  Ton, keine, die nicht auf ihrem Ton stehen, und gleich klingende nur einmal. Für Klavier (ohne Flügel), Stabspiele und
  Trommeln gelten nur die Listen aus LumiViz; dazu Hackbrett, Cimbalom und Steeldrum (Deluxe) und das Honky-tonk an der
  ersten verstimmten Saite.
- **Hammer-Kern:** die Überabtastung des Kontakts ist einstellbar (nur für Messungen; der Klang ist Bit für Bit gleich).

## Entschieden

- **PN1 a, PN2 a, KZ1 a** (Pegel der Hörlisten), **LG1 a** (welche Liste je Klasse), **LV6-1 a bis LV6-4 a** (gleiche
  Klänge), **LR1 a** (Klasse 7 als Stufe 34m), **NS1 a** (erst der leise Anschlag, dann 34j), **HS-1 a** (Hammerwerte
  zuerst belegen).

## Befunde

- Im Diskant regt der Hammer bei ppp … p viel zu schwach an. Die Hysterese des Filzes fehlt dort (ein Vorzeichen seit
  S159 ohne Beleg; richtig nach Kartofelev & Stulov 2014). Mit dem belegten Wert wird der Umfang des Tons dreimal näher an
  *Salamander*, der Hammer an C6 aber nicht mehr heller mit der Stärke — offen.

## Geprüft

- Eng 12 / 12 unter Release-clang und Testing-MSVC; alle 11 Hörlisten mit `Hoertest --liste-pruefen` 0 Fehler, 0 Hinweise.
- Hammer-Bezug Bit für Bit nach der Änderung am Kern (62 / 62 unter Release-clang und Testing-MSVC).
