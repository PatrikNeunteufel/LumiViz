# Changelog Session 94 (2026-10-04)

> **Der Composer zeigt sein erstes Bild in der App, und Presets mit Movement
> laufen ohne Stillstand an.** Über File → Open Composition... spielt die App
> eine von Hand geschriebene Komposition: Clips auf Kanälen, über die Zeit
> gemischt, mit der Karaoke-Ebene darüber. Beim Messen fiel auf, dass der
> AVS-Effekt Movement jedes Preset beim Laden rund eine halbe Sekunde anhält —
> seine Tabelle entsteht jetzt über mehrere Bilder verteilt. Der Composer
> bleibt im Zweig `composer`; ein Panel zum Bedienen gibt es noch nicht.

## Neu

- **File → Open Composition... / Close Composition:** lädt eine Komposition
  (`.lvcomp`) und spielt sie anstelle der Kette. Die Uhr startet sofort und
  hält am Ende; Close bringt die Kette von vorher unverändert zurück. Das
  Panel Effect Chain bleibt währenddessen bedienbar. Eine Beispieldatei liegt
  unter `asset/composer_demo.lvcomp`. Fehlt die Datei eines Clips, nennt ein
  Hinweisfenster sie, die übrigen Clips laufen.
- **Die Karaoke-Ebene liegt einmal über der Komposition** und läuft auch
  weiter, wenn die Komposition zu Ende ist oder steht.
- **Settings → Performance → Movement Table:** „Spread over frames (no
  freeze)" (Standard) oder „At once (like original AVS)". Wer ein Preset mit
  dem AVS-Original vergleichen will, wählt „At once".
- **Messung im Log:** Close Composition und das Beenden der App mit laufender
  Komposition schreiben eine Zusammenfassung ins `LumiViz.log` — Bilder je
  Sekunde, Grafikkarte, Zeit je Bild, lange Bilder mit ihrer Stelle, Anlauf je
  Clip, Grafikspeicher.

## Geändert

- **Presets mit dem AVS-Effekt Movement laufen ohne Stillstand an.** Der
  Effekt rechnet sein Skript einmal je Bildpunkt aus und legt eine Tabelle an —
  in Full HD rund eine halbe Sekunde je Movement, beim Laden, bei jeder
  Größenänderung des Fensters und wenn ein Host Switcher eine Gruppe zum ersten
  Mal zeigt. Die Tabelle entsteht jetzt über mehrere Bilder verteilt; bis sie
  steht (ein bis zwei Sekunden), arbeitet der Effekt mit einem groben Gitter.
  Eine schon gebaute Tabelle wird wiederverwendet: das zweite Laden desselben
  Presets ist sofort genau.
- **Skripte, die `reg`, `megabuf`, `rand` oder Audio lesen,** sehen diese
  Werte im Standard-Betrieb über mehrere Bilder verteilt statt in einem
  Augenblick. Mit „At once" verhalten sie sich wie im Original.

## Behoben

- **Der Songtext blieb stehen, sobald die Komposition zu Ende war** (Befund
  Sichttest). Die Karaoke-Ebene läuft jetzt unabhängig von der Uhr der
  Komposition weiter.
- **Ein Movement in einer Liste, die nur beim Beat rechnet, sah viele Sekunden
  lang grob aus** (Befund Sichttest, „pixeliger"): die Tabelle bekam nur beim
  Beat ein Stück Rechenzeit. Sie wird jetzt in jedem Bild weitergebaut.

## Intern

- **Composer, Stufe C2a** (Zweig `composer`): Naht R3 (Bildquelle im
  Render-Thread, `f8821db`), `composer/Bildquelle`, Naht N8 (Menü, `a6d4b08`),
  `composer/AppAnbindung`, Messung und Messlauf. Steuerdokument
  `docs/ui/Composer_Konzept.md` 0.9.2 mit Rückbau-Liste.
- **Messwerte** (Release-clang, 1920 × 1080, 15 Presets gleichzeitig auf fünf
  Kanälen): das erste Bild eines Movement-Presets fällt von 0,8 s auf
  11–135 ms; das längste Bild nach dem ersten von 600–800 ms auf 147 ms
  (Radeon 610M).
- **Tests:** 877 Cases (Start der Session: 866). Neu: `test_ComposerNahtR3`,
  `test_ComposerBildquelle`, `test_ComposerMessung` (Messlauf, nur mit
  Umgebungsvariable), `test_MovementScheiben`. Fenster-Tests beachten
  `LUMIVIZ_REF_BILDSCHIRM`.
- **Vergleichslauf:** 90 / 90 bitgleich gegen `out/f4c/bp30_s93_c1a` — die
  Standalones bauen die Movement-Tabelle weiter in einem Stück.

## Noch nicht gesehen

- Der Nachlauf des Sichttests (`.claude/handover/Sichttests_S94b.md`):
  Karaoke nach dem Ende, „Duo - Hash the Planet" nach dem Fix, der Schalter
  Movement Table, die Meldung bei fehlender Clip-Datei.
- Die Zusammenfassung beim Beenden der App — die Exe des Sichttests enthält
  diesen Stand noch nicht.
