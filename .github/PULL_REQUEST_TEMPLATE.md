# Pull Request

## Was ändert sich und warum?

<!-- Kurz und konkret. Beispiel: "Der Blur-Knoten rundete beim Trail-Modus in die
     falsche Richtung — dadurch verblasste die Spur eine Stufe zu schnell." -->

## Zugehöriges Issue

<!-- Closes #123 — bitte ausfüllen, falls vorhanden. -->

## Checkliste

- [ ] Debug **und** Testing bauen ohne Fehler
- [ ] `ctest --preset ctest-vs-x64-Testing -R UnitTests` grün, 0 übersprungen
- [ ] `clang-format` gelaufen
- [ ] Neue Dateien sind in der zuständigen `Source.cmake` eingetragen
- [ ] Neues Verhalten hat einen Test
- [ ] Keine absoluten Pfade, keine lokalen Eigenheiten
- [ ] Kein fremdes Material (Presets, Shader, Texturen, Logos) hinzugefügt
- [ ] Bei übernommenem Fremdcode: Herkunft im Dateikopf **und** Eintrag in
      `THIRD_PARTY_NOTICES.md`

## Sichttest (nur bei Änderungen am Rendern)

<!-- Screenshot vorher/nachher. Bei Import-Effekten zusätzlich der Vergleich
     gegen tools/AvsRef bzw. tools/MilkdropRef. Ohne Bild ist eine
     Render-Änderung nicht beurteilbar. -->

## Worauf soll ich beim Review besonders schauen?

<!-- Optional, aber hilfreich. -->
