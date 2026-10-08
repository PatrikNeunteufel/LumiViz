# Changelog Session 103 (2026-10-08)

> **Am Programm hat sich nichts geändert.** Diese Session hat die Sichttests
> neu gefasst: feste Vorlagen statt „etwa diese Datei", und ein Test, der
> Vorlagen und Listen beisammen hält.

## Neu

- **Fertige Beispiel-Kompositionen** unter `asset\sichttest\`, alle mit
  runden Zeiten und eigenem Material (Presets, Testsignale, Songtext):

  | Datei | Länge | Inhalt |
  |---|---|---|
  | `composer_A_kurz.lvcomp` | 1:00 | zwei Bild-Kanäle, ein Song |
  | `composer_A_master.lvcomp` | 1:00 | dazu ein Stück in der Zeile MASTER bei 0:30 |
  | `composer_B_viele.lvcomp` | 6:00 | sechs Bild- und sechs Audio-Kanäle |
  | `composer_C_songwechsel.lvcomp` | 2:00 | drei Songs mit Überlagerung und Lücke |
  | `composer_D_schleife.lvcomp` | 0:30 | ein Clip, Loop overlap 5 s |
  | `composer_E_karaoke.lvcomp` | 1:00 | Song mit Songtext, der seine Zeiten nennt |
  | `composer_F_kurven.lvcomp` | 1:00 | Clips mit und ohne Blenden |
  | `composer_G_altformat.lvcomp` | 0:50 | das Dateiformat bis Session 95 |
  | `composer_H_dreimal.lvcomp` | 1:00 | dasselbe Preset in drei Clips |
  | `composer_I_luecke.lvcomp` | 1:00 | eine Lücke im Bild, der Ton läuft durch |
  | `composer_J_einrasten.lvcomp` | 0:20 | eine Kante zwischen zwei Sekunden |

  Sie sind die Ausgangslage der Sichttests. Zum Ausprobieren eine Kopie
  nehmen.

## Doku

- Benutzerhandbuch 1.112.0 — §15 *Eine Komposition von Hand schreiben*: die
  Felder aus den Sessions 99 bis 101 nachgetragen (`loop`, `fade`, `level`
  mit `at` als fester Zeit, `visualMaster`, `audioMaster`, `karaokeMaster`,
  `color`, `gridSeconds`).
- Composer_Konzept 0.69.0 — §12.4 Rückbau-Liste: die Vorlagen der Sichttests.
- Offene_Punkte 1.125.0.

## Tests

- **967 Cases** (vorher 966). Neu: `test_ComposerVorlagen.cpp` — jede Vorlage
  lädt ohne Meldung; Länge, Kanäle, Clip-Zeiten, Blenden und Besonderheiten
  stimmen mit dem überein, was die Sichttest-Listen sagen.
- Voller Lauf: in zwei von vier Läufen 967 / 967; in zwei fiel ein Fall um —
  `test_PaletteUi.cpp` (Karaoke-Panel), ein bekannter Wackelkandidat, von
  dieser Session nicht berührt. Ursache nicht untersucht.

## Unter der Haube

- Kein Code der App geändert, keine Naht berührt.
- Die offenen Sichttest-Punkte zum Composer (Sessions 94 bis 102) sind in
  neun Listen nach Thema neu gefasst — lokal, nicht im Repo.
- Dass der Sichttest die App künftig steuert (Vorlage laden, Panel holen),
  wird mit dem Werkzeug-Projekt abgestimmt; gebaut ist dafür nichts.
