# Changelog Session 125 (2026-10-15)

> **Publish entschieden, ein Fehler im Import behoben.** Für Publish (ein Ding
> mit allem, was es nennt, als Paket weitergeben) sind alle acht Entscheide
> gefallen; das Konzept steht in `docs/core-services/Publish_Konzept.md`,
> gebaut ist davon noch nichts. **Geprüft mit Unit-Tests unter beiden
> Compilern.** Im Programm gesehen ist nichts.

## Behoben

- **Import data → Keep both:** Gab es den Namen schon, hieß die importierte
  Datei `bass (2).lvinst`. Instrumente, Samples, Kits und Formen mit einem
  solchen Namen übergeht das Programm — die Datei lag im Ordner, erschien aber
  nirgends und ließ sich nicht nennen. Jetzt heißt sie `bass_2`, wie beim
  Übernehmen eines Instruments zu den eigenen. Das gilt für alle Bereiche, auch
  für Presets (`Neon bars_2.json`).

## Geändert

- Benutzerhandbuch §1: Keep both nennt das Beispiel `bass_2`.

## Intern

- Konzept `docs/core-services/Publish_Konzept.md` 0.1.0 (Entscheide P1 a bis
  P8 a, drei Skizzen als Beschreibung); Offene_Punkte 1.226.0.
- Publish, Stufe P-A: die Sammler der Loop-Familie (`paket/Sammler`) —
  was ein Loop, Kit, Instrument, eine Form oder ein Sample über alle Stufen
  nennt; neu `test_PublishSammler` (vier Fälle). Noch ohne Oberfläche.
- Tests: in `test_Datensicherung` ein Fall zu Keep both mit Instrumenten,
  Samples, Kits und Formen; die Fälle zu `nameMitZahl` auf `_2` umgestellt.
