# Sichttest-Protokoll S79/S80 — Effekt-Palette und Formelfelder

> **Version:** 1.0.0
> **Datum:** 2026-09-28 (Session 80)
> **Typ:** Prüfprotokoll zum Ausfüllen
> **Status:** Offen
> **Sprache:** Deutsch
> **Bezug:** [Offene_Punkte.md](Offene_Punkte.md) · [Formelfelder_Inventar.md](visuals/Formelfelder_Inventar.md)
> · `Changelog_Session79.md`

## So wird es benutzt

- **Exe:** `out\build\windows-ninja-release-clang\exec\LumiViz\bin\Release\LumiViz.exe`
  (nicht die Testing-Exe — die läuft auf der 610M und ruckelt).
  Für Teil C und D7/D8 muss es der **Build von S80** sein (liegt seit
  2026-09-28 dort).
- **Ausgangslage:** Visualizer **Multi Effect**, Panel **Effect Chain** offen,
  Musik läuft (Bass-Wirkungen sieht man sonst nicht).
- **Ergebnis-Spalte:** `✅` wie erwartet · `❌` weicht ab (dann Notiz) ·
  `—` nicht geprüft.
- Skripte: im Panel beim Modul das Feld **Frame** bzw. **Beat** öffnen
  (⤢ = großer Editor), Text eintragen, **Apply**.

---

## Teil A — Effekt-Palette (Modulauswahl)

| # | Was tun | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|
| A1 | In der Effect Chain auf **„+“** klicken | Eigenes Fenster **„Effect Palette“** geht auf; die Kette dahinter bleibt bedienbar | | |
| A2 | Ins Suchfeld `fract` tippen, **Enter** | Der oberste Treffer (ein Fraktal) wird in die Kette eingefügt | | |
| A3 | Im Suchfeld **Pfeil ↓**, dann mit ↑/↓ einen anderen Treffer wählen, **Enter** | Genau der markierte Eintrag wird eingefügt | | |
| A4 | Suche leeren. Einen Herkunfts-Chip (**AVS**, **APE**, **MilkDrop**, **LumiViz**) drücken | Liste zeigt nur Module dieser Herkunft | | |
| A5 | Zusätzlich einen Rollen-Chip (**Draws**, **Transforms**, **Colour**, **Control**) drücken | Liste zeigt nur, was **beide** Bedingungen erfüllt | | |
| A6 | Einen zweiten Chip **derselben** Zeile dazunehmen | Liste wird **größer** (innerhalb einer Zeile gilt „oder“) | | |
| A7 | Ein Modul markieren | Detailzeile unten zeigt Beschreibung, Herkunft/Rolle, Originalname und ob es Formelfelder hat | | |
| A8 | Bei einem Modul **★** setzen | Es steht oben unter **★ Favourites** | | |
| A9 | Zwei Module einfügen, dann App beenden und neu starten, „+“ | ★ ist noch gesetzt; unter **Recently used** stehen die eingefügten Module | | |
| A10 | Eine **Liste** (Effect List) in der Kette markieren, in der Palette **Add** | Modul landet **in** der markierten Liste | | |
| A11 | Einen gewöhnlichen Knoten markieren, **Add** | Modul landet **unter** dem markierten Knoten | | |
| A12 | Modul aus der Palette **auf eine Liste ziehen** | Landet in der Liste | | |
| A13 | Modul **zwischen zwei Knoten** ziehen (Einfügelinie) | Landet genau dort | | |
| A14 | Modul auf die **leere Fläche** unter der Kette ziehen | Landet am Ende der Kette | ❌ | **Befund S80:** nur unmittelbar unter dem letzten Eintrag wird es angenommen; weiter unten in der freien Fläche nicht → [Offene_Punkte §7](Offene_Punkte.md) |
| A15 | **Custom Wave** (bzw. Custom Shape / Sprite) auf einen **Milkdrop-Knoten** ziehen | Wird Wave/Shape/Sprite **in** diesem Knoten — kein eigener Kettenknoten | | |
| A16 | Custom Wave auf einen Nicht-Milkdrop-Knoten ziehen | Wird abgewiesen (kein Einfügen, kein Absturz) | | |
| A17 | Durchgehend beobachten | Palette bleibt nach jedem Einfügen offen; schließt erst mit **Close** oder X | | |
| A18 | Palette schließen, wieder öffnen | Öffnet an derselben Stelle/Größe | | |
| A19 | *(Durchsicht, kein Klicktest)* Rollen und Kurztexte der Einträge lesen | Passen die Texte? Korrekturen hier notieren — Quelle `src/UI/panels/EffectPalette.cpp` | | |

## Teil B — Formelfelder (Stichproben)

Je Zeile: Modul einfügen (am besten allein in einer frischen Kette), Skript
eintragen, Apply.

| # | Modul | Slot | Skript | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|---|---|
| B1 | **Clear** | Frame | `color_r=sin(time)*0.5+0.5` | Hintergrund pulsiert langsam rot (G/B bleiben wie im Panel) | | |
| B2 | **Clear** | Frame | `color_r=1; color_g=0; color_b=bass` | Rot, bei Bass Richtung Magenta | | |
| B3 | **Fractal 2D** | Frame | `colorscale=1+bass*3` | Farbringe verdichten sich mit dem Bass | | |
| B4 | **Fractal 2D** | Beat | `colorphase=colorphase+0.25` | Palette springt bei jedem Beat ein Stück weiter (und läuft danach normal weiter) | | |
| B5 | **Domain Warp** | Frame | `octaves=2+bass*6` | Detailgrad schwankt mit dem Bass | | |
| B6 | **Shadertoy (GLSL)** | Image-Code + Frame | Im Image-Code oben `uniform float zoom;` einfügen und dort, wo `uv` berechnet wird, `* zoom` anhängen. Im Frame-Skript: `zoom=1+bass*2` | Bild zoomt mit dem Bass; ohne Frame-Skript steht `zoom` auf 0 | | |
| B7 | **Host Group**, *Blend Out* = **Adjustable** | Beat | `outadjustalpha=255` | Gruppe blitzt bei jedem Beat voll auf … | | |
| B8 | (dieselbe Host Group) | Frame | `outadjustalpha=outadjustalpha*0.9` | … und blendet danach aus | | |
| B9 | beliebiges Modul aus B1–B8 | — | Referenz öffnen (ⓘ bzw. rechte Spalte im großen Editor) | Die dort gelisteten Variablen sind genau die, die wirken | | |

## Teil C — Editor-Nachträge S80 (neuer Build nötig)

| # | Was tun | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|
| C1 | Milkdrop-Knoten → Shape → Frame-Code öffnen, `b = blue; b2 = blue;` | `b` hat **dieselbe Farbe** wie `r`, `g`, `r2`, `g2`, `b2` (lila = in/out) | | |
| C2 | Im Shape-Frame-Code: `r = r2 = red;` | Übernommen ohne Fehler; Innen- **und** Randfarbe folgen `red` | | |
| C3 | Referenz (Built-ins) ganz unten lesen | Satz „Chained assignment: `r = r2 = red;` sets both …“ steht da | | |

## Teil D — Sichttest S76 (noch offen, gleicher App-Lauf)

| # | Was tun | Erwartet | Ergebnis | Notiz |
|---|---|---|---|---|
| D1 | **File → New Effect Chain** | Bild schwarz | | |
| D2 | Milkdrop-Knoten einfügen | Bild **bleibt** schwarz (kein Rauschen, kein Gitter) | | |
| D3 | Im Milkdrop-Panel Basis-Wave einschalten / Sprite-Augen einschalten, dann wieder aus | Bild folgt den Checkboxen in **beide** Richtungen | | |
| D4 | „Motion-Vector-Overlay“ an und aus | Raster erscheint bzw. verschwindet | | |
| D5 | Sprite-Bereich im Panel ansehen | Felder abgeschnitten? Panel-Breite ok? | | |
| D6 | Ein importiertes `.milk`-Preset laden (Import Browser) | Sieht aus wie vor S76 | | |
| D7 | `asset/effectchain/202609_g_milk.lvfx` laden (neuer Build S80) | Die Shape zieht **keine** Spur mehr nach oben links und blendet nicht von selbst aus | | |
| D8 | Zwei, drei vertraute `.milk`-Presets ansehen | Wirken wie gewohnt — höchstens etwas schärfer (das Weichzeichnen je Frame ist weg) | | |

## Teil E — Befunde, die schon beim Testen aufgefallen sind (S80)

| # | Befund | Stand |
|---|---|---|
| E1 | Im Editor war `b` anders eingefärbt als `r`/`g` (im Shape-Code ist `b` Blau, der Editor kannte nur „Beat“) | ✅ behoben S80 → Prüfung C1 |
| E2 | Wunsch: `r = r2 = red;` | ✅ ging schon; Test + Referenzsatz S80 → Prüfung C2/C3 |
| E3 | Weißliches Diagonalenkreuz bei Shapes, wenn `a` ≠ `a2` | Kein Fehler: Shapes sind Dreiecksfächer (wie im Original, `D3DPT_TRIANGLEFAN`); die Deckkraft verläuft je Dreieck, auf den Diagonalen knickt der Verlauf. Für eine gleichmäßige Fläche Innen- und Randwerte gleich setzen |
| E4 | Leere Milkdrop-Kette mit nur einer Shape (`asset/effectchain/202609_g_milk.lvfx`): Bild **blendet aus** und **wandert nach oben links**, obwohl `decay=1`, `zoom=1`, `dx=dy=0` | ✅ behoben S80 (Halbtexel-Versatz im Warp, gegen MilkdropRef belegt) → Prüfung D7, [Offene_Punkte §3](Offene_Punkte.md) |
| E5 | Drop auf die freie Fläche unter der Kette wird nicht angenommen | → A14, [Offene_Punkte §7](Offene_Punkte.md) |
| E6 | Wunsch: Vorlagen („Einstiegspunkte“) für Shape, Wave, Sprite und Shader — z. B. ein **gewolltes** Ausblenden oder eine Drift, aber erklärt | → [Offene_Punkte §7](Offene_Punkte.md) |

## Freie Notizen

-
