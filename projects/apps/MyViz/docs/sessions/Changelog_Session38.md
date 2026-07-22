# Changelog Session 38 (2026-07-21/22) — Import-Treue: Diagnose + Fixes · FyrewurX · Origin-Icons

> **Typ:** Produkt-Changelog
> **Bezug:** [Import_Treue_Fixplan.md](../visuals/Import_Treue_Fixplan.md) ·
> [Import_Modul_Abdeckung.md](../visuals/Import_Modul_Abdeckung.md) ·
> [Offene_Sichttests.md](../Offene_Sichttests.md)
> **Tests:** MyViz.UnitTests 308 Cases grün, 0 Skips, 4108 Assertions (vorher 298)

## Neu

### Import-Treue-Diagnose + Fixplan

Alle **612 AVS-Presets** aus `asset/avs/` strukturell gedumpt (0 Parse-Fehler)
und die Wiedergabe systematisch gegen die Referenz `ref/vis_avs` verglichen.
Befunde, Symptom-Zuordnung je EyeCandy2-Preset und Umsetzungsstand im neuen
Steuerdokument [Import_Treue_Fixplan.md](../visuals/Import_Treue_Fixplan.md).

### AVS-treuer Audio-Antrieb

- **Spektrum-Log-Kennlinie** (g_logtab-Port) vor getspec/getosc — beat-getriebene
  Bewegung (`ti=getspec(...)`) läuft endlich in Originalgeschwindigkeit;
  Kalibrierpunkt `kSpecGain` (aktuell 8).
- **AVS-Beat-Detektor** (main.cpp-Port: Peak-Tracker ×34/32, Floor,
  Refire-Guard) ersetzt den adaptiven RMS-Detektor; der bpm.cpp-Schätzer
  (`refine()`) liefert jetzt wirklich den Beat.

### Effekt-Korrekturen (Auswahl)

Bump additiv (statt abdunkelnd) · Clear Screen mit Replace/Additiv/50-50/
Line-Blend · Buffer Save mit korrekter r_stack-Blend-Tabelle (1=50/50, 2=additiv),
Richtungen 0–3 (alternierend) und Blend beim Speichern · SuperScope: `n`
persistiert (init `n=800` gilt), Weiß-Default-Farbe, Import-Default n=100,
skriptbares `drawmode`/`linesize` (auch pro Punkt), **Linienbreite bis 255**
(AVS linesize) · Dynamic Movement: `alpha`-Blend, `blend`/`nomove`/`subpixel`/
**`buffern`** (Global-Buffer als Quelle), **Gitter = Wert+1** (AVS-Semantik) ·
Movement: 96×72-Gitter mit statischem Cache, 50/50-Blend, `subpixel`,
**`sourcemapped`** (Scatter) · Mirror: 4 Richtungs-Bits + weiche Übergänge ·
Effect Lists: `fake_enabled` (on-beat-Listen) · Line-Blend-Default = REPLACE
(AVS-Start) · Texer/Texer II zeichnen bei fehlendem Bild den Default-Punkt.

### FyrewurX (Nachbau) + Origin-Icons

- **FyrewurX** (`FunkyFX FyrewurX v1`, in 68 Presets): Verhaltens-Nachbau ohne
  Original-Code — Beat-Feuerwerks-Bursts mit Gravitations-Funken, additiv.
  APE-Bilanz: ✅ 17 · ◐ 1 · ✖ 5.
- **Origin-Icons** (`asset/img/logo/icons/`): Palette-Dropdown und
  Effektketten-Baum kennzeichnen jede Modul-Herkunft (AVS-Port / MilkDrop-Port /
  LumiViz-Original) per Icon; Text-Fallback ohne Asset-Ordner.

## Bekannte offene Punkte

- **05_wormhole:** Tunnelform/Drehbewegung weiterhin nicht wie im Original
  (Stand `c8d2bd2` war dort besser) — Faktoren-Bisektion geplant.
- Sichttests §7/§8 in [Offene_Sichttests.md](../Offene_Sichttests.md)
  (kSpecGain-Tempo, sourcemapped-Optik, FyrewurX-Kalibrierung).
