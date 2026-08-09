# MilkLoudness — relative Band-Loudness (bass/mid/treb + *_att)

> **Version:** 1.0.0  
> **Datum:** 2026-07-22  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (MilkDrop-Import M2 — Skript-Vertrag)  
> **Modul:** lumi::modules::MilkLoudness (header-only)  
> **Dateien:** MilkLoudness.hpp  
> **Abhängigkeiten:** keine (reine Rechenklasse, kein Qt/GL)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## 1. Übersicht

MilkDrop-Presets lesen `bass/mid/treb` als Loudness **relativ zum
Langzeit-Mittel** desselben Bandes (~1.0 in ruhigen Passagen, ~1.3+ auf einem
Beat) und `bass_att/mid_att/treb_att` als geglättete (attenuierte) Varianten.
Diese Klasse portiert das Referenzmodell 1:1:
`ref/MilkDrop3/code/vis_milk2/plugin.cpp:8749-8779` (BSD) +
`utility.cpp AdjustRateToFPS`.

- `avg` (→ `*_att`): asymmetrische EMA — Attack-Rate 0.2, Release-Rate 0.5
  (bei 30 fps; schneller Anstieg, träger Abfall).
- `long_avg` (Nenner): Rate 0.992, in den ersten 50 Frames 0.9 (Warm-up).
- Raten fps-korrigiert: `rate^(30/fps)`.
- `bass = imm/long_avg`, `bass_att = avg/long_avg`; Guard `|long_avg|<0.001 → 1.0`
  (Stille liest sich als 1.0, nie inf/NaN).

Gefüttert wird mit **rohen Band-Energien in beliebiger, konsistenter Skala** —
die Division kürzt die Einheit heraus (deshalb entfallen die empirischen
Normierungsfaktoren des Originals).

## 2. API

```cpp
lumi::modules::MilkLoudness loud;
loud.update(bassImm, midImm, trebImm, fps);   // 1x pro Frame
loud.bass();  loud.mid();  loud.treb();       // imm_rel  (Preset-Inputs)
loud.bassAtt(); loud.midAtt(); loud.trebAtt();// avg_rel  (*_att)
loud.reset();                                  // Kaltstart
```

## 3. Bewusste Abweichungen vom Original

1. **Kaltstart-Seeding:** Frame 0 setzt `avg`/`long_avg` auf den ersten
   gefütterten Wert (Original startet bei 1.0, verlässt sich aber auf die
   empirisch normierte ~1.0-Skala seiner Bänder). Vermeidet Riesen-Spikes in
   den ersten Frames bei beliebiger Eingangsskala.
2. **Keine Band-Aufteilung hier:** welche FFT-Bins bass/mid/treb bilden,
   entscheidet der Aufrufer (M3-Host); das Original mischt Bandbildung und
   Glättung in einer Funktion.
3. `frame()`-Zähler ist instanzlokal (Original nutzt den globalen Frame-Zähler
   für das Warm-up-Fenster).

## 4. Verwendung (geplant, M3)

Der `MilkdropVisualizer` füttert 1×/Frame aus seinem Spektrum und speist die
sechs Werte als Skript-Inputs in alle Milk-Slots (per_frame/per_pixel/Wave/
Shape); die Namen sind in der `symbolCategory`-Tabelle (MultiEffectPanel) als
Input registriert.

## 5. Tests

`test_MilkScriptContract.cpp` §MilkLoudness: Einpendeln auf 1.0, Beat-Spike
(imm sofort, att gedämpft), Release-Nachhang, Stille-Guard, fps-Invarianz
(30 vs. 60 fps), reset()-Kaltstart.

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 39, M2): Port plugin.cpp-Loudness inkl. AdjustRateToFPS, Guard, Warm-up; Kaltstart-Seeding als dokumentierte Abweichung |
