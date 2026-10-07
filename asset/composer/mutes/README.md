# Composer — Mute-Profile

Mitgelieferte Mute-Profile (`.lvmute`) fuer den Mute-Knopf: wie der Ton bei
einer spontanen Durchsage weggeht und wiederkommt. Ein Profil traegt

- `level` — den Pegel, auf den abgesenkt wird (0 = stumm, 1 = voll),
- `out` und `outShape` — Dauer (ms) und Kurvenform des Ausblendens,
- `in` und `inShape` — Dauer (ms) und Kurvenform des Einblendens,
- `visualsFollow` — ob die Visuals die Absenkung sehen (fehlt = nein: das Bild
  reagiert weiter auf die volle Musik).

Die Kurvenformen sind dieselben wie in den Blenden-Presets (`../fades`).
Eigene Profile liegen im Benutzerordner `<AppData>/composer/mutes/` und
verdraengen ein mitgeliefertes gleichen Namens.

| Profil | Pegel | aus | ein |
|---|---|---|---|
| Announcement | 20 % | 0,4 s, S-Kurve | 1,5 s, S-Kurve |
| Soft mute | 0 % | 0,6 s, S-Kurve | 1,2 s, S-Kurve |
| Hard mute | 0 % | sofort | sofort |

Diese Datei ist der Anker, an dem die App den Ordner findet — nicht loeschen.
Beschreibung: `projects/apps/LumiViz/docs/ui/Composer_Konzept.md` §4.
