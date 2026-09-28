# Sichttest-Protokoll S79/S80 — Effekt-Palette und Formelfelder

> **Version:** 1.1.0
> **Datum:** 2026-09-28 (Session 80)
> **Typ:** Prüfprotokoll zum Ausfüllen
> **Status:** Großteils automatisch abgenommen — Handrest siehe unten
> **Sprache:** Deutsch
> **Bezug:** [Offene_Punkte.md](Offene_Punkte.md) · [Formelfelder_Inventar.md](visuals/Formelfelder_Inventar.md)
> · `Changelog_Session79.md`

## Auf einen Blick

| | Zeilen |
|---|---|
| ✅ **auto** — durch einen Test belegt, nichts mehr zu tun | A1–A19 (A19 von Hand abgenommen), B1–B9, C1–C3, D1, D2, D7, F3–F6 (Kern) |
| 🟡 **teils auto** — Kern belegt, ein Rest braucht die Hand | D3, D4, F4 (Zentrum, 270°) |
| ✋ **Hand** — braucht ein Urteil | **D5**, **D6**, **D8**, **F1**, **F2** — plus einmal echtes Ziehen mit der Maus (A12–A15) und ein Blick auf die Kontaktbögen |

Die Ergebnis-Spalte der Tabellen sagt je Zeile, **wodurch** sie belegt ist.
Wo die Hand noch gefragt ist, steht das Feld leer.

## Die Werkzeuge

| Werkzeug | Deckt ab | Aufruf |
|---|---|---|
| `tests/unit/UnitTests/test_PaletteUi.cpp` — 21 UI-Tests offscreen | A1–A18, C1, C3 — Palettenfenster und **echtes** MultiEffectPanel mit Host; Drops als Ereignisfolge, gezählt an der **Kette** | läuft mit allen Unit-Tests; Widget-Screenshots mit `LUMIVIZ_UI_SCHNAPPSCHUSS=<ordner>` |
| `asset/calibration/sichttest/sichttest_auto.py` — Ketten-Stichproben | B1–B8, D1–D4, D7 — `.lvfx` rendern (`AvsStandalone`, Beat alle 30 Frames), Urteil an Zahlen, Kontaktbogen je Punkt. Führt die UI-Tests mit aus | `python sichttest_auto.py` → `out/sichttest_auto/report.md` (alles an einer Stelle) |
| `test_ScriptVariablesGl`, `test_EelTranspiler` | B9, C2 | Unit-Tests |

**Stand 2026-09-28:** UI-Tests 21/21 · Stichproben 12/12 · alle Unit-Tests 631/631.

**Grenzen der Automatik** (ehrlich, damit nichts als geprüft gilt, was es nicht ist):

- **Maus:** Die Drops sind echte Qt-Ereignisse, aber ohne Mauszeiger und ohne
  Einfügelinie auf dem Schirm. Ob sich das Ziehen *gut anfühlt*, sieht nur ein
  Mensch.
- **Neustart (A9, A18):** simuliert durch ein neues Fenster, das dieselben
  QSettings liest — kein echter App-Neustart. A18 prüft die Größe, nicht die
  Bildschirmposition.
- **Synthetisches Signal (B):** „folgt dem Bass" heißt: das Bild ändert sich
  gegen eine Kontrolle ohne Skript. Ob es *schön* reagiert, zeigt der
  Kontaktbogen.
- **D3/D4:** geprüft ist das **Feld** im Knoten, nicht die Checkbox im Panel.

Zwei Dinge, die beim Automatisieren auffielen und auch am Bildschirm gelten:

- **B5:** Mit der Formel `2+bass*6` ändert sich bei ruhiger Musik nichts —
  `octaves` ist ganzzahlig, `bass` bleibt oft unter 0,17, das Ergebnis also
  unter 3. Zum Sehen besser `octaves=2+bass*20` (so prüft es auch das Skript).
- **B7/B8:** Eine Host Group **blendet beim Laden 2 s ein** (`crossfadeSeconds`)
  und mischt so lange mit eigener Deckkraft — `outadjustalpha` wirkt erst
  danach. Kurz warten oder die Einblendzeit auf 0 stellen.

## Für die Hand

- **Exe:** `out\build\windows-ninja-release-clang\exec\LumiViz\bin\Release\LumiViz.exe`
  (nicht die Testing-Exe — die läuft auf der 610M und ruckelt), Build von S80.
- **Ausgangslage:** Visualizer **Multi Effect**, Panel **Effect Chain** offen,
  Musik läuft.
- **Ergebnis-Spalte:** `✅` wie erwartet · `❌` weicht ab (dann Notiz).

---

## Teil A — Effekt-Palette (Modulauswahl)

| # | Was tun | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|
| A1 | In der Effect Chain auf **„+“** klicken | Eigenes Fenster **„Effect Palette“** geht auf; die Kette dahinter bleibt bedienbar | ✅ auto | `Panel A10/A11`: „+“ öffnet das Fenster, danach wird im Kettenbaum weiter gewählt |
| A2 | Ins Suchfeld `fract` tippen, **Enter** | Der oberste Treffer (ein Fraktal) wird in die Kette eingefügt | ✅ auto | `A2` (Enter meldet das Fraktal) + `Panel A10/A11` (Add fügt ein) |
| A3 | Im Suchfeld **Pfeil ↓**, dann mit ↑/↓ einen anderen Treffer wählen, **Enter** | Genau der markierte Eintrag wird eingefügt | ✅ auto | `A3` |
| A4 | Suche leeren. Einen Herkunfts-Chip (**AVS**, **APE**, **MilkDrop**, **LumiViz**) drücken | Liste zeigt nur Module dieser Herkunft | ✅ auto | `A4-A6` |
| A5 | Zusätzlich einen Rollen-Chip (**Draws**, **Transforms**, **Colour**, **Control**) drücken | Liste zeigt nur, was **beide** Bedingungen erfüllt | ✅ auto | `A4-A6` |
| A6 | Einen zweiten Chip **derselben** Zeile dazunehmen | Liste wird **größer** (innerhalb einer Zeile gilt „oder“) | ✅ auto | `A4-A6` |
| A7 | Ein Modul markieren | Detailzeile unten zeigt Beschreibung, Herkunft/Rolle, Originalname und ob es Formelfelder hat | ✅ auto | `A7` (am Beispiel Clear) |
| A8 | Bei einem Modul **★** setzen | Es steht oben unter **★ Favourites** | ✅ auto | `A8/A9` |
| A9 | Zwei Module einfügen, dann App beenden und neu starten, „+“ | ★ ist noch gesetzt; unter **Recently used** stehen die eingefügten Module | ✅ auto | `A8/A9` — Neustart simuliert (neues Fenster, dieselben QSettings) |
| A10 | Eine **Liste** (Effect List) in der Kette markieren, in der Palette **Add** | Modul landet **in** der markierten Liste | ✅ auto | `Panel A10/A11` |
| A11 | Einen gewöhnlichen Knoten markieren, **Add** | Modul landet **unter** dem markierten Knoten | ✅ auto | `Panel A10/A11` |
| A12 | Modul aus der Palette **auf eine Liste ziehen** | Landet in der Liste | ✅ auto | `Panel A12` — Maus-Gefühl: Hand |
| A13 | Modul **zwischen zwei Knoten** ziehen (Einfügelinie) | Landet genau dort | ✅ auto | `Panel A13` — Maus-Gefühl: Hand |
| A14 | Modul auf die **leere Fläche** unter der Kette ziehen | Landet am Ende der Kette; eine Custom Wave/Shape/Sprite im **letzten** Milkdrop-Knoten | ✅ auto | `Panel A14`, `A14b`. **Befund S80, behoben S80:** gewöhnliche Module gingen schon; eine **Shape** ging auf der Leerfläche still verloren — nur direkt unter der letzten Zeile traf sie zufällig die Sprites-Sektion des Milkdrop-Knotens |
| A15 | **Custom Wave** (bzw. Custom Shape / Sprite) auf einen **Milkdrop-Knoten** ziehen | Wird Wave/Shape/Sprite **in** diesem Knoten — kein eigener Kettenknoten | ✅ auto | `Panel A15` |
| A16 | Custom Wave auf einen Nicht-Milkdrop-Knoten ziehen | Wird abgewiesen (kein Einfügen, kein Absturz) | ✅ auto | `Panel A16`; fremde Daten (kein Paletteneintrag) ebenso: `A16` |
| A17 | Durchgehend beobachten | Palette bleibt nach jedem Einfügen offen; schließt erst mit **Close** oder X | ✅ auto | `A1/A17`, `Panel A10/A11` — Close geprüft, das X der Titelleiste nicht |
| A18 | Palette schließen, wieder öffnen | Öffnet an derselben Stelle/Größe | ✅ auto | `A18` — Größe geprüft, Bildschirmposition nicht |
| A19 | *(Durchsicht, kein Klicktest)* Rollen und Kurztexte der Einträge lesen | Passen die Texte? Korrekturen hier notieren — Quelle `src/UI/panels/EffectPalette.cpp` | ✅ | Patrik 2026-09-28: passt |

## Teil B — Formelfelder (Stichproben)

Belegt durch `sichttest_auto.py`: je Zeile eine Kette mit Skript gegen eine
Kontrolle ohne. Kontaktbogen `out/sichttest_auto/<Punkt>.png`.

| # | Modul | Slot | Skript | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|---|---|
| B1 | **Clear** | Frame | `color_r=sin(time)*0.5+0.5` | Hintergrund pulsiert langsam rot (G/B bleiben wie im Panel) | ✅ auto | Rot 0,12…1,00, G/B 0 |
| B2 | **Clear** | Frame | `color_r=1; color_g=0; color_b=bass` | Rot, bei Bass Richtung Magenta | ✅ auto | Rot 1,00, Blau 0,16…0,27 |
| B3 | **Fractal 2D** | Frame | `colorscale=1+bass*3` | Farbringe verdichten sich mit dem Bass | ✅ auto | gegen `colorscale=1`: MAE 0,129 |
| B4 | **Fractal 2D** | Beat | `colorphase=colorphase+0.25` | Palette springt bei jedem Beat ein Stück weiter (und läuft danach normal weiter) | ✅ auto | Abstand zur Kontrolle wächst je Beat (0,14 → 0,23 → 0,27), nach vier Beats (= 1,0) wieder deckungsgleich |
| B5 | **Domain Warp** | Frame | `octaves=2+bass*6` | Detailgrad schwankt mit dem Bass | ✅ auto | geprüft mit `2+bass*20` (Grund oben); gegen `octaves=2`: MAE 0,069 |
| B6 | **Shadertoy (GLSL)** | Image-Code + Frame | Im Image-Code oben `uniform float zoom;` einfügen und dort, wo `uv` berechnet wird, `* zoom` anhängen. Im Frame-Skript: `zoom=1+bass*2` | Bild zoomt mit dem Bass; ohne Frame-Skript steht `zoom` auf 0 | ✅ auto | gegen `zoom=1`: MAE 0,209; ohne Skript einfarbig |
| B7 | **Host Group**, *Blend Out* = **Adjustable** | Beat | `outadjustalpha=255` | Gruppe blitzt bei jedem Beat voll auf … | ✅ auto | Spitze 1,00 am Beat (Kontrolle 0,50) — Einblendung beachten (oben) |
| B8 | (dieselbe Host Group) | Frame | `outadjustalpha=outadjustalpha*0.9` | … und blendet danach aus | ✅ auto | 10 Frames nach dem Beat 0,35 |
| B9 | beliebiges Modul aus B1–B8 | — | Referenz öffnen (ⓘ bzw. rechte Spalte im großen Editor) | Die dort gelisteten Variablen sind genau die, die wirken | ✅ auto | `test_ScriptVariablesGl` — jeder Name der Referenz gegen die Laufzeit, für alle Knotentypen |

## Teil C — Editor-Nachträge S80

| # | Was tun | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|
| C1 | Milkdrop-Knoten → Shape → Frame-Code öffnen, `b = blue; b2 = blue;` | `b` hat **dieselbe Farbe** wie `r`, `g`, `r2`, `g2`, `b2` (lila = in/out) | ✅ auto | `C1` (Farbe je Zeichen aus dem Highlighter) |
| C2 | Im Shape-Frame-Code: `r = r2 = red;` | Übernommen ohne Fehler; Innen- **und** Randfarbe folgen `red` | ✅ auto | `test_EelTranspiler`: Mehrfachzuweisung setzt beide |
| C3 | Referenz (Built-ins) ganz unten lesen | Satz „Chained assignment: `r = r2 = red;` sets both …“ steht da | ✅ auto | `C3` |

## Teil D — Sichttest S76 und Drift-Fix S80

| # | Was tun | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|
| D1 | **File → New Effect Chain** | Bild schwarz | ✅ auto | Luma max 0,000 |
| D2 | Milkdrop-Knoten einfügen | Bild **bleibt** schwarz (kein Rauschen, kein Gitter) | ✅ auto | 120 Frames, Luma max 0,000 |
| D3 | Im Milkdrop-Panel Basis-Wave einschalten / Sprite-Augen einschalten, dann wieder aus | Bild folgt den Checkboxen in **beide** Richtungen | 🟡 teils | Basis-Wave an zeichnet, aus bleibt schwarz ✅ auto. **Hand:** Sprite-Augen und die Checkboxen selbst |
| D4 | „Motion-Vector-Overlay“ an und aus | Raster erscheint bzw. verschwindet | 🟡 teils | Feld an: Raster (Luma 1,00), aus: schwarz ✅ auto. **Hand:** die Checkbox im Panel |
| D5 | Sprite-Bereich im Panel ansehen | Felder abgeschnitten? Panel-Breite ok? | | ✋ |
| D6 | Ein importiertes `.milk`-Preset laden (Import Browser) | Sieht aus wie vor S76 | | ✋ — Treue gegen das Original misst die MilkDrop-Messreihe (S80), „wie gewohnt“ nur das Auge |
| D7 | `asset/effectchain/202609_g_milk.lvfx` laden | Die Shape zieht **keine** Spur mehr nach oben links und blendet nicht von selbst aus | ✅ auto | Shape-Blitz in derselben Pipeline: Wanderung 0,00 px, Energieverlust 0,0 % (f31→f121); deine Kette im Kontaktbogen `D7.png` |
| D8 | Zwei, drei vertraute `.milk`-Presets ansehen | Wirken wie gewohnt — höchstens etwas schärfer (das Weichzeichnen je Frame ist weg) | | ✋ |

## Teil F — Gestaltung: Drift und Weichzeichnen (neu S80)

Milkdrop-Node → **Parameter** → Gruppe **„Gestaltung (kein Original-Verhalten)"**.
Richtungen, Bitgleichheit und Skript-Steuerung sind automatisch belegt
(`sichttest_auto.py` G1, Drift-Sonde, `test_MilkdropGestaltung`); hier geht es
um Bedienung und Wirkung am Bildschirm.

| # | Was tun | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|
| F1 | Ein vertrautes `.milk`-Preset laden, **„Weicher Sog ↖"** drücken | Sieht aus wie vor S80 (weich, zieht nach oben links) | | ✋ |
| F2 | **„Original"** drücken | Wieder still und scharf; die Reglerwerte bleiben stehen | | ✋ |
| F3 | Linear-Drift an, Stärke 1, Richtung durchdrehen | Schlieren folgen der Richtung (0 rechts, 90 oben …) | ✅ auto | G1 — Gefühl: Hand |
| F4 | Radial-Drift an, Winkel 0 / 90 / 180 / 270, Zentrum verschieben | außen / Wirbel ↺ / Sog / Wirbel ↻ um das Zentrum | ✅ auto | G1 (0/90/180) — Zentrum und 270: Hand |
| F5 | Im `per_frame`-Code `drift_dir = time * 0.5;` | Linear-Drift dreht langsam im Kreis | ✅ auto | G1 (Skript π) — laufend drehend: Hand |
| F6 | Kette speichern, neu laden | Gestaltung bleibt erhalten | ✅ auto | `test_MilkdropGestaltung` |

## Teil E — Befunde, die beim Testen aufgefallen sind (S80)

| # | Befund | Stand |
|---|---|---|
| E1 | Im Editor war `b` anders eingefärbt als `r`/`g` (im Shape-Code ist `b` Blau, der Editor kannte nur „Beat“) | ✅ behoben S80 → C1 |
| E2 | Wunsch: `r = r2 = red;` | ✅ ging schon; Test + Referenzsatz S80 → C2/C3 |
| E3 | Weißliches Diagonalenkreuz bei Shapes, wenn `a` ≠ `a2` | Kein Fehler: Shapes sind Dreiecksfächer (wie im Original, `D3DPT_TRIANGLEFAN`); die Deckkraft verläuft je Dreieck, auf den Diagonalen knickt der Verlauf. Für eine gleichmäßige Fläche Innen- und Randwerte gleich setzen |
| E4 | Leere Milkdrop-Kette mit nur einer Shape: Bild **blendet aus** und **wandert nach oben links**, obwohl `decay=1`, `zoom=1`, `dx=dy=0` | ✅ behoben S80 (Halbtexel-Versatz im Warp, gegen MilkdropRef belegt) → D7, [Offene_Punkte §3](Offene_Punkte.md) |
| E5 | Drop auf die freie Fläche unter der Kette wird nicht angenommen | ✅ behoben S80 — betraf nur Milkdrop-Elemente (Shape/Wave/Sprite) → A14 |
| E6 | Wunsch: Vorlagen („Einstiegspunkte“) für Shape, Wave, Sprite und Shader — z. B. ein **gewolltes** Ausblenden oder eine Drift, aber erklärt | → [Offene_Punkte §7](Offene_Punkte.md) |
| E7 | Host Group: `outadjustalpha` wirkt erst nach der Einblendung beim Laden (2 s) | Verhalten wie gebaut, nur überraschend → Hinweis oben |

## Freie Notizen

-
