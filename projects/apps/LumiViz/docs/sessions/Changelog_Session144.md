# Changelog Session 144 (2026-10-22)

> **Freeze: rechenintensive Clips einfrieren.** Ein Clip im Loop-Panel lässt sich einfrieren — er spielt
> dann seine fertig gerechnete Wellenform statt seiner Töne. Eine Sanduhr zeigt, welche Clips sich lohnen.
> Dazu zwei Fehler behoben: ein falscher Klang im Composer nach einer Änderung und ein Absturz beim
> Beenden, während ein Loop spielt.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.4 (FV1 a, FV2 a, FZ4 a bis FZ7 a).

## Neu

- **Freeze clip / Unfreeze clip** im Kontextmenü eines Clips (auch für mehrere markierte). Ein
  eingefrorener Clip spielt im Loop-Panel und im Composer seine Wellenform; Fader und Lautstärke des Loops
  bleiben live. Sein Nachklang (Hall, Echo) klingt über das Ende des Loops hinweg weiter.
- **Die Marke am Clip:** „❄ frozen“, „computing… n %“ beim Rechnen, „❄ frozen · changed“, wenn der Clip
  seit dem Einfrieren bearbeitet wurde (er spielt dann live). Auf schmalen Clips nur das Zeichen — die
  Flocke im Quadrat, verändert im Warndreieck.
- **Beim Speichern** werden veränderte eingefrorene Clips im Hintergrund neu eingefroren; ein Undo auf den
  eingefrorenen Stand gilt sofort. Eine fehlende Wellenform (geräumt, unlesbar) wird von selbst neu
  gerechnet. Die Hinweiszeile zeigt „Freezing clip 2 of 3 … 40 %“.
- **Die Sanduhr:** Clips, deren Töne länger als eine Schwelle zum Rechnen brauchen, tragen „⏳ 3.2 s“;
  der Tooltip nennt Arbeit und Wartezeit, das Menü „Freeze clip (3.2 s to compute)“.
- **Settings → Panels:** *Freeze hint* — ab welcher Zeit die Sanduhr erscheint (Vorgabe 1 s) und ob sie
  die Arbeit oder die Wartezeit zeigt.

## Geändert

- **`.lvloop` Fassung 13:** jeder Clip trägt `frozen` (leer oder der Abdruck des eingefrorenen Stands).
- **Bauplan:** nach Freeze kommt wieder die Hammer-Saite (Flügel, Upright) als Stufe 31, der Audio-Clip im
  Loop danach.

## Behoben

- **Composer:** nach einer Änderung im Loop-Panel, deren Töne noch gerechnet wurden, konnte ein Loop-Clip
  kurz schweigen oder einen falschen Klang spielen. Jetzt klingt bis dahin das Bisherige.
- **Absturz beim Beenden,** während im Loop-Panel ein Loop spielte (sichtbar im Debug-Build): die Panels
  wurden erst nach der Audio-Engine abgebaut. Der Fehler war alt.

## Intern

- `loop/Freeze` (Marken, Zustand, `eingefrorenVon`, `frozenFassung`, Rechenzeit, Settings),
  `loop/FreezeAuftrag`, Rechenkern mit Stimmen ohne Selbstschnitt, `Tonauftrag` lädt `frozen:`-Namen,
  `DockManager::baueAb`.
- Neue Tests `test_LoopFreezeSpielen`, `test_LoopFreezeBedienung`, Fälle in `test_LoopFreeze`,
  `test_LoopSpieler`, `test_LoopMenues`; 1498 Fälle. Je Teilschritt unter beiden Compilern grün, der
  Bereich `*Loop*,*Instrument*,*Shape*,*Palette*,*Drum*,*Wav*` nach 30g-2; **kein voller Lauf**.
- Doku: `Bootstrap_Integration.md` §6.8 (Panels sterben vor den Diensten), `Loop_Konzept.md` (Feld `frozen`).
