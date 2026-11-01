# Changelog Session 170 (2026-11-01)

> **Autotune ist da.** Ein Wave-Clip zieht den gesungenen Ton jetzt auf Zielnoten — von Hand gezogen oder mit
> *Detect notes* aus der Tonart vorgeschlagen, mit Strength und Speed bis zum harten Autotune-Klang. Die neue Methode
> **Voice** verschiebt eine Stimme, ohne dass sie nach Micky Maus klingt (die Formanten bleiben). Wave-Clips folgen auf
> Wunsch dem Tempo des Loops (*Follow tempo*), und jeder Drum- oder Instrument-Clip lässt sich zu einem Wave-Clip
> **bouncen**. Steuerdokument: `docs/audio/Loop_Konzept.md` §4p.

## Neu

- ***Pitch method* Voice** (15h): TD-PSOLA für eine einzelne Stimme — die Vokale bleiben, wenn die Tonhöhe sich
  ändert. An acht gerechneten Stimmen gegen Signalsmith Stretch gemessen: die Hülle sechsmal genauer, kein
  zusätzliches Rauschen (Bericht `docs/modellierung/wave-clips/proben/S170_pruefstein_15h.md`).
- **Spur Autotune** im Wave editor (15i): die gesungene Tonhöhe gestrichelt, die korrigierte orange, die Zielnoten
  als Kästen — ziehen (Shift: Cent), an den Kanten kürzen, auf freier Fläche aufziehen, Ctrl+Klick teilt, Entf löscht.
  Darunter Key, Scale, **Detect notes**, Strength, Speed, *Selected section only*; findet *Detect notes* eine Stimme,
  bietet der Editor **Use Voice** an. Ohne Gesang (Beatbox, Sprache) sagt er das, statt still nichts zu tun.
- ***Follow tempo*** (15j): ein Wave-Clip folgt dem Tempo des Loops, ohne die Tonhöhe zu ändern; Taktachse und Wave
  editor zeigen die Welle gedehnt.
- ***Bounce to wave clip*** (15k-2): im Menü eines Drum- oder Instrument-Clips — gerechnet wie Freeze, die Datei
  landet bei *My samples*, der Clip wird ersetzt. Danach **Bounce again**, **Revert to source** und die Marke
  **source changed**, wenn das ursprüngliche Muster sich geändert hat.

## Geändert

- **pYIN erkennt hohe Stimmen** (PY2 a): ein Sopran bei 440 Hz galt vorher als stumm, jetzt zu 99,6 % gesungen; an
  echtem Gesang (vocadito) gleich gut. Gemerkte Tonhöhenverläufe werden einmal neu gemessen.
- *Detect notes* ersetzt nichts mehr, wenn es nichts findet.

## Intern

- Neu `loop/Psola`, `loop/Autotune`, `loop/Bounce`, `loop/BounceAuftrag`; `Dehner::behaeltFormanten`,
  `dehnerStimme`; `waveTempoFaktor`; im Klangnamen `;voice`, `;at:…`, `;tempo:…` — ohne diese Spuren derselbe Name
  wie bisher, alte Loops klingen Bit für Bit gleich.
- `kTonhoehenFassung` 2.
- Tests: **1747 Cases** (+20): `test_LoopDehnerPruefstein`, `test_LoopAutotune`, `test_LoopFolgtTempo`,
  `test_LoopBounce`, Fälle in `test_LoopDehner`, `test_LoopPYin`, `test_LoopWaveEditor`; je Teilschritt eng unter
  Release-clang und Testing-MSVC grün, kein voller Lauf (der kommt mit 15m).
- Berichte: `proben/S170_pruefstein_15h.md`, `proben/S170_pyin_vocadito.md`; Hörproben lokal unter
  `.claude/handover/hoerproben_15h/`.
