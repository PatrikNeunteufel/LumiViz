# Changelog Session 181 (2026-11-09)

> **Klaviere und ein Clavinet aus den Rechenkernen.** Vier neue Instrumente aus `InstrumentsCalculation`: Upright,
> Honky-tonk, ein elektrischer Flügel nach dem CP-70 und ein Clavinet mit gerechneter Tangente. Damit ist Teilschritt 34i
> abgeschlossen (`docs/audio/Instrumente_IC_Konzept.md` 0.13.0).

## Neu

- **Upright piano, Honky-tonk piano, Electric grand piano** auf dem Rechenkern des Flügels: das Upright mit kürzeren,
  schwereren Basssaiten, das Honky-tonk mit verstimmten Chören, der elektrische Flügel mit 73 Tasten und einer Saite im
  tiefen Bass. Das Upright hat die Pedale Soft und Practice.
- **Clavinet modal** auf einem neuen Rechenkern „Struck string“: die Tangente schlägt die Saite an und hält sie; beim
  Loslassen klingt sie drei Halbtöne tiefer. Zwei Tonabnehmer und der Verstärker. Das bisherige Clavinet bleibt, bis ein
  Hörvergleich entscheidet.
- **Keys demo** — ein Beispiel mit allen vier.
- Alle vier sind **ungemessen** gekennzeichnet; das Upright und das Honky-tonk klingen noch ohne Resonanzboden (für einen
  Boden fehlen belegte Maße).

## Geändert

- **Der Tastenumfang verbiegt keinen Klang mehr:** hat ein Instrument weniger Tasten als sein Rechenkern, bleiben die
  Werte der Tasten darin genau so, wie sie sind (vorher konnten sie sich verschieben).
- Der Rechenkern des Flügels heißt im Instrument-Creator „Hammer string“ und zeigt den Tastenumfang je Instrument.
- Instrumentdateien in **Fassung 24** (der Tastenumfang steht beim Rechenkern); alle mitgelieferten Kern-Instrumente sind
  nachgezogen — der Klang ist unverändert.
- `instruments/CREDITS.md` nennt 25 Quellen.
