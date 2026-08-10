# Mitgelieferte Presets

Was in diesem Ordner liegt, wird **nach jedem Build neben die Exe kopiert** —
nach `<Build-Ordner>/presets/`. Damit hat die Anwendung sofort etwas zu zeigen,
ohne dass jemand erst eine Sammlung beschaffen muss.

## Was aktuell mitgeliefert wird

| Ordner | Format | Dateien |
|---|---|---|
| `avs/EyeCandy2/` | AVS (`.avs`) | 10 |
| `milkdrop/fuck me im famous/` | MilkDrop (`.milk`) | 19 |
| `milkdrop/Fuck me Im Famous - revisited/` | MilkDrop (`.milk`) | 3 |

Alles Eigenwerk (GreatWho).

## Zurückgestellt für 0.5.0: die Mash-Up-Sätze

Zwei große Sätze liegen bereit, sind aber **bewusst noch nicht hier**
(Entscheid Patrik, S75) — sie kommen mit **0.5.0**, sobald die Erlaubnis
geklärt ist:

- `GreatWho + Flexi - Rock The House [Caturday Night]` (93)
- `GreatWho + Martin + Geiss + Flexi - Lasershow [240 bipolar mixes]` (243)

Es sind **Mash-Ups Dritter**: die *Basis* (`Rock The House`, `Lasershow`)
stammt von GreatWho, die Mischungen selbst hat jemand anderes erstellt
(vermutlich **Flexi**). Für diesen Ordner gilt „ausschließlich EIGENE Werke"
(Root-`CMakeLists.txt` + [THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md)
→ „Nicht enthaltene Inhalte") — deshalb erst nach Rückmeldung des Erstellers,
dann mit Namensnennung hier und in `THIRD_PARTY_NOTICES.md`.

Die Dateien selbst bleiben verfügbar unter
`asset/Milkdrop3/presets/` (nicht Teil des Binärpakets). Aufnahme = die beiden
Ordner nach `milkdrop/` kopieren, mehr ist nicht nötig.

**Das ist eine Auswahl, die wachsen soll — kein Abbild der ganzen Sammlung.**
Der vollständige eigene Bestand (die `greatwho*`- und `GreatWhoPack*`-Sätze)
bleibt bewusst außerhalb des Repositorys; hierher kommt nach und nach, was
mitgeliefert werden soll. Diese Datei wird beim Build **nicht** mitkopiert.

## Weitere Presets aufnehmen

1. **Unterordner anlegen** — unterhalb von `avs/` bzw. `milkdrop/`, benannt nach
   dem Set. Der Ordnername taucht in der Anwendung so auf, wie er hier heißt.
2. **Dateien hineinlegen.**
3. **In die `Source.cmake` eintragen?** Nein — Presets sind keine Quelldateien.
   Es wird der **ganze Ordner** kopiert (`copy_directory` im Root-`CMakeLists.txt`);
   ein neuer Unterordner wird ohne weiteres Zutun mitgenommen.
4. **Neu konfigurieren ist nicht nötig**, ein Build genügt. Nur wenn `asset/presets/`
   vorher gar nicht existierte, muss einmal neu konfiguriert werden — die Kopie
   wird beim Konfigurieren verdrahtet.
5. **Diese Datei ergänzen** — die Tabelle oben ist die Übersicht.

### Wenn nichts ankommt

Die Kopie meldet sich beim Konfigurieren:

```
-- [Presets] Mitgelieferte Presets werden nach dem Build kopiert
```

Fehlt die Zeile oder steht dort eine Warnung, greift die Verdrahtung nicht —
dann im Root-`CMakeLists.txt` nachsehen (Abschnitt „Mitgelieferte Presets").

`copy_directory` **überschreibt, löscht aber nicht**: eine hier entfernte Datei
bleibt im Build-Ordner liegen, bis der Ordner geleert wird. Beim Aufräumen also
`<Build-Ordner>/presets/` von Hand löschen und neu bauen.

## ⚠️ Nur eigene Werke

Hier darf **ausschließlich** hinein, woran die Rechte bei uns liegen. LumiViz ist
ein öffentliches Repository — was hier liegt, wird mit veröffentlicht.

**Nicht hierher gehören:**

- Community-AVS-Presets fremder Autoren
- MilkDrop-Presets und -Texturen aus fremden Sammlungen
- ISF-Shader von Vidvox, Shadertoy-Shader fremder Autoren

Die Anwendung **lädt** solche Sammlungen problemlos aus einem beliebigen Ordner —
sie werden nur nicht mitgeliefert. Begründung und Herkunftshinweise:
[`THIRD_PARTY_NOTICES.md`](../../THIRD_PARTY_NOTICES.md), Abschnitt
„Nicht enthaltene Inhalte".

**Grenzfall Remix:** Enthält ein eigenes Preset Bausteine anderer (in der
AVS-Szene üblich, oft im Preset selbst gutgeschrieben), muss die Gutschrift
in der Datei erhalten bleiben.
