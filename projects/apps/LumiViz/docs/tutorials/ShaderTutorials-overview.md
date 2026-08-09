# Shader-Tutorials — Übersicht & Wegleitung

> **Dokumenttyp:** Overview  
> **Ebene:** Kategorie  
> **Version:** 1.2.0  
> **Status:** Stabil  
> **Domain:** Programming  
> **Kategorie:** Algorithms  
> **Gültigkeit:** Shader-Tutorial-Serie in `projects/apps/LumiViz/docs/tutorials/` (Shadertoy-GLSL ↔ LumiViz)  
> **Zweck:** Fachliche Orientierung über die Tutorial-Serie: Fokus je Tutorial, Lesereihenfolge, Technik-Index, Dokumentenlandkarte  
> **Zielgruppe:** Shader-Einsteiger und LumiViz-Autoren  
> **Sprache:** Deutsch

> ℹ **Hinweis:** Dieses Dokument enthält ASCII-Diagramme; die Darstellung setzt eine Monospace-Schrift voraus.

## Inhaltsverzeichnis

1. Zweck & Scope
2. Taxonomie
3. Überblick: Welches Tutorial hat welchen Fokus? (Konzept-Vergleich)
4. Dokumentenlandkarte
5. Lesehilfe: In welcher Reihenfolge? (Leseempfehlungen)
6. Technik-Index (Entscheidungsmatrix)
7. Ausschlusskriterien
8. Glossar (Verweis)
9. Weiterführende Dokumente & Ausblick
10. Changelog

## Zweck & Scope

Dieser Ordner (`projects/apps/LumiViz/docs/tutorials/`) enthält eine zusammenhängende **Tutorial-Serie zum Shader-Bau von Grund auf** — und ist zugleich die Heimat künftiger Tutorial-Serien (Milkdrop-Presets, AVS) sowie der formalen Referenzdokumente. Alle Tutorials teilen dieselbe Schule: Jeder Schritt ist ein vollständiger, lauffähiger Shader für [shadertoy.com/new](https://www.shadertoy.com/new), jeder Schritt fügt genau eine Technik hinzu, und die Reihenfolge ist immer **Geometrie → Material → Licht → Bewegung → Politur**. Jedes Tutorial endet mit einem Gesamtlisting (STELLSCHRAUBEN-Konstantenblock, Konvention der Vorrats-Shader in `asset/shadertoys/`) und zwei Anhängen:

- **Anhang A – Audio-Reaktivität:** Bänder aus der FFT-Textur, Beat-Gates, ein shader-spezifischer Mapping-Katalog und die fertigen Einbau-Diffs.
- **Anhang B – Shadertoy ↔ LumiViz:** der Weg in die App und zurück. Die **Vollreferenz** (drei Import-Wege, Portabilitäts-Checkliste, Audio-Adapter-Muster, Buffer-A-Zustand) steht im Crystal-Lights-Tutorial; die übrigen Tutorials halten Anhang B kompakt und verweisen dorthin.

Die Stil-Vorbilder der Serie sind Milkdrop-Presets von martin (`asset/Milkdrop3/presets/`) – die Tutorials portieren sie **nicht**, sondern übernehmen ihre Stilmittel und bauen die Technik eigenständig in Shadertoy-GLSL auf.

Zum Nachschlagen neben den Tutorials: **[Raymarching-reference.md](Raymarching-reference.md)** – die formale Referenz (Blueprint-System, Typ Reference) zu Algorithmus, SDF-Katalog, Marsch-Varianten mit Entscheidungshilfe, Raumoperationen, Beleuchtung und Artefakt-Katalog. Die Tutorials leiten her, die Referenz schlägt nach.

> ⚠️ **Ehrlichkeits-Hinweis für die ganze Serie:** Alle Schritte sind **in LumiViz gegengerendert** (AvsStandalone; jedes Tutorial bringt generierte Schritt-Chains in `<name>_schritte/` und die Render-Screenshots in `<name>_bilder/` mit — das Markdown bleibt SSOT, die `make_schritte.py` dort regenerieren die Chains). Der Sichttest auf shadertoy.com ist weiterhin offen. Bekannte Nachstimm-Kandidaten aus den Render-Läufen: Tunnel-Helligkeit der Schritte 3–7, der dark-Endstand des Juggernaut (heller als beschrieben) und die Bloom-`SCHWELLE=0.7` im Postfx-Composite (in der dark-Stimmung leer — die Chains rendern mit 0.3). Hinweis zum Standalone-Testsignal: Seine FFT sättigt auf der dB-Skala; Beat-Gates mit Absolutschwellen stehen dort dauerhaft offen (auf shadertoy.com mit echter Musik nicht).

---

## Taxonomie

Die Serie gliedert sich in drei Äste; die Ordnerstruktur (ein Dokument je Thema plus `<name>_schritte/`- und `<name>_bilder/`-Unterordner) spiegelt genau diesen Baum.

```text
Shader-Tutorials (docs/tutorials/)
│
├── Basis-Tutorials ─ ein Shader je Tutorial, von Grund auf
│   ├── PyramidSpiral ········ Raymarching-Fundament (Einstieg)
│   ├── CrystalLights ········ Höhenfelder, Brechung, Kamera
│   ├── StratosphericTunnel ·· Röhren, Fenster, Pfade, Vergabelung
│   ├── SpaceDebris ·········· 3D-Repetition, Taumeln, Stimmungen
│   ├── PimpedKaleidoscope ··· 2D-Feedback-Strang (Buffer/Zustand)
│   └── Juggernaut ··········· Megastruktur, God-Rays, dark↔bright
│
├── Composite-Tutorials ─ mergen die fertigen Basis-Shader
│   ├── CompositePortals ····· Szenen-Merge, Material-Id, AA
│   ├── CompositePostfx ······ Multipass-Kette, Bloom, DOF
│   └── CompositeTransitions · Übergänge, Zustandsmaschine
│
└── Referenzdokumente ─ Nachschlagen statt Herleiten
    └── Raymarching-reference · Algorithmus, SDF-Katalog, Varianten
```

*Fig. 1 [Blockdiagramm]: Taxonomie der Tutorial-Serie*

## Überblick: Welches Tutorial hat welchen Fokus?

| Tutorial | Was entsteht | Technik-Fokus | Niveau |
|---|---|---|---|
| [Pyramid-Spiral](PyramidSpiral-tutorial.md) | Geraymarchter Kaleidoskop-Tunnel aus leuchtenden Oktaedern | **Das Fundament:** UV-Koordinaten, SDF-Raymarching, Normalen, Domain Repetition, Raumfaltung (Kaleidoskop, Twist), Cosinus-Palette, Fresnel/Specular, Glow, Nebel, Tonemapping. Anhang B: Zell-Index, 3D-Equalizer, Beat-Blitze | Einstieg |
| [Crystal-Lights](CrystalLights-tutorial.md) | Halbliquides Kristall-Terrain über blinkenden Farblichtern | **Höhenfeld-Raymarching** (Terrain statt Tunnel), Value-Noise/FBM, Voronoi, Materialfelder (Liquidität, Lücken), **Brechung + Beer-Lambert-Absorption**, Punktlicht-Raster, **Isometrie↔Perspektive**, Kamera-Choreografie. Anhang B = **Vollreferenz Shadertoy↔LumiViz** | Aufbau |
| [Stratospheric-Tunnel](StratosphericTunnel-tutorial.md) | Neon-Röhrentunnel-Flug mit Fenstern zur Stratosphäre | Zylinder-/Polar-Geometrie, **Wand-Relief** (3 Typen), **Fenster-Masken** mit Außenraum, **vier Lichtquellen** (Scheinwerfer, Neon, Ringe, Fensterlicht), **Pfadkrümmung + Richtungsumkehr + Banking**, **Tunnel-Vergabelung** | Aufbau |
| [Space-Debris](SpaceDebris-tutorial.md) | Taumelndes Trümmerfeld im Orbit über einem Glutplaneten | **3D-Domain-Repetition mit Rotation je Zelle** (Rodrigues-mat3, Zellwand-Klammer), Formbibliothek, Ausdünnung/Cluster, analytischer Planet + Atmosphäre, **Licht-Stimmungen** (Sonne, Untenlicht, Gegenlicht), Schwerelosigkeits-Kamera | Aufbau |
| [Pimped-Kaleidoscope](PimpedKaleidoscope-tutorial.md) | Psychedelisches Falt-Kaleidoskop mit Leuchtspuren | **Der Kontrast zur Serie – 2D statt Raymarching:** Feedback-Systeme über Buffer A (Vorframe, Decay, Zoom/Rot, Sharpen, Dither – Milkdrops Warp-Rezept), drei kombinierbare Faltungen, „unsichtbare Kamera" als Lese-Transformation, Wellenform-Seed | Aufbau |
| [Juggernaut](Juggernaut-tutorial.md) | Kolossale dunkle Megastruktur im Dunst | Größenwirkung/Low-Angle, **smin/smax-Verschneidung**, Panel-Greebles in Detail-Oktaven, **God-Rays/volumetrischer Glow**, Orbit-Kamera, **dark↔brighter als überblendbare Licht-Stimmung** | Vertiefung |

*Tab. 1: Basis-Tutorials — Fokus und Niveau*

**Stil-Vorbilder:** Pyramid-Spiral ← „Pyramid Spiral" (Noztol, Shadertoy) · Crystal-Lights ← *frosty caves 2* · Stratospheric-Tunnel ← *stratospheric turbulences 2* · Space-Debris ← *space debris* · Pimped-Kaleidoscope ← *shader pimped caleidoscope* · Juggernaut ← *juggernaut brighter / juggernaut 2 dark* · Audio-Gates überall ← *Rock The House*.

### Composite-Tutorials (mergen die fertigen Shader)

| Tutorial | Was entsteht | Quellen | Technik-Fokus |
|---|---|---|---|
| [Composite-Portals](CompositePortals-tutorial.md) | Tunnel-Flug, dessen Fenster in die echte Debris-Welt blicken; Kristall-Terrain als Tunnelboden | Tunnel + Debris + Crystal-Lights | **Kondensieren** (Gesamtlisting → Skelett), Namespacing zweier Welten, **Portal-Strahlen** (Szene-in-Szene, Maßstab/Diorama), **Material-Id-Dispatch**, Anti-Aliasing (fwidth, Supersampling), Kohärenz gegen den Collage-Effekt |
| [Composite-Postfx](CompositePostfx-tutorial.md) | Der Juggernaut, veredelt durch eine echte Nachbearbeitungs-Kette | Juggernaut + Kaleidoscope | **Multipass-Architektur** (Buffer A→B→C→Image, Common als SSOT), Tiefe im Alpha-Kanal, **Bloom** (Bright-Pass + separierbarer Blur — das GetBlur1/2/3-Pendant), **Depth of Field**, Kaleidoskop als Finish (vor/nach Bloom), Temporal-Glättung, „Politur ans Ketten-Ende" |
| [Composite-Transitions](CompositeTransitions-tutorial.md) | Endloser Wechsel Kristall-Terrain ↔ Juggernaut mit wählbarer Blendart | Crystal-Lights + Juggernaut | Übergangs-Phasen (Halten/Blenden), **Masken-Wipes mit Glühsaum**, Blendarten-Katalog, **Parameter-Morph** statt Bild-Mix, Kamera-Kontinuität, **Zustandsmaschine in Buffer A** (beat-getriggerter Wechsel = Milkdrop-Preset-Wechsel-Analogon) |

*Tab. 2: Composite-Tutorials — Quellen und Fokus*

---

## Dokumentenlandkarte

Die Landkarte zeigt je Thema den Dokumentbestand. Die Serie ist tutorial-getrieben: Referenz-Tiefe liefert bislang ein geteiltes Dokument (Raymarching) für den 3D-Strang; Concept-Dokumente (Herleitungen jenseits der Tutorials) existieren noch keine und sind als ausstehend markiert.

| Thema | Tutorial | Reference | Concept | Status |
|---|---|---|---|---|
| Raymarching-Fundament | [PyramidSpiral](PyramidSpiral-tutorial.md) | [Raymarching](Raymarching-reference.md) (geteilt) | ausstehend | 🟡 |
| Höhenfelder/Brechung | [CrystalLights](CrystalLights-tutorial.md) | [Raymarching](Raymarching-reference.md) (geteilt) | ausstehend | 🟡 |
| Röhren/Fenster/Pfade | [StratosphericTunnel](StratosphericTunnel-tutorial.md) | [Raymarching](Raymarching-reference.md) (geteilt) | ausstehend | 🟡 |
| 3D-Repetition/Taumeln | [SpaceDebris](SpaceDebris-tutorial.md) | [Raymarching](Raymarching-reference.md) (geteilt) | ausstehend | 🟡 |
| 2D-Feedback/Kaleidoskop | [PimpedKaleidoscope](PimpedKaleidoscope-tutorial.md) | ausstehend (Feedback-Referenz) | ausstehend | 🟡 |
| Megastruktur/God-Rays | [Juggernaut](Juggernaut-tutorial.md) | [Raymarching](Raymarching-reference.md) (geteilt) | ausstehend | 🟡 |
| Szenen-Merge/Portale | [CompositePortals](CompositePortals-tutorial.md) | [Raymarching](Raymarching-reference.md) (geteilt) | ausstehend | 🟡 |
| Multipass/PostFX | [CompositePostfx](CompositePostfx-tutorial.md) | ausstehend (Feedback-Referenz) | ausstehend | 🟡 |
| Übergänge/Zustand | [CompositeTransitions](CompositeTransitions-tutorial.md) | ausstehend (Feedback-Referenz) | ausstehend | 🟡 |

*Tab. 3: Dokumentenlandkarte (🟡 = Tutorial vorhanden und in LumiViz gegengerendert; Referenz-/Concept-Ausbau und shadertoy.com-Sichttest offen)*

**Coverage:** Tutorials 9/9 (100 %) · Reference: 1 geteiltes Dokument für den 3D-Strang, Feedback-Referenz ausstehend · Concepts 0/9 (0 % — bislang bewusst: die „Warum"-Tiefe steckt in den „Was passiert hier"-Abschnitten der Tutorials).

---

## Lesehilfe: In welcher Reihenfolge?

```
                    ┌──────────────────────┐
   PFLICHT-EINSTIEG │ 1. Pyramid-Spiral    │  Raymarching-Grundlagen (Schritte 1–7
                    └──────────┬───────────┘  sind das Fundament ALLER anderen)
                               │
                    ┌──────────┴───────────┐
                    │ 2. Crystal-Lights    │  Höhenfelder, Noise, Brechung, Kamera —
                    └──────────┬───────────┘  und Anhang B als LumiViz-Referenz
                               │
            ┌──────────────────┼──────────────────────┐
            │ 3D-STRANG        │                       │ 2D-STRANG
   ┌────────┴─────────┐ ┌──────┴─────────┐   ┌─────────┴──────────┐
   │ 3a. Stratospheric│ │ 3b. Space      │   │ 3c. Pimped         │
   │     Tunnel       │ │     Debris     │   │     Kaleidoscope   │
   └────────┬─────────┘ └──────┬─────────┘   └─────────┬──────────┘
            └──────────┬───────┘                       │
              ┌────────┴─────────┐                     │
              │ 4. Juggernaut    │                     │
              └────────┬─────────┘                     │
                       └──────────────┬────────────────┘
                              ┌───────┴────────┐
                              │ 5. Composites  │  Portals → Postfx → Transitions
                              └────────────────┘
```

*Fig. 2 [Blockdiagramm]: Empfohlene Lesereihenfolge mit Verzweigung in 3D- und 2D-Strang*

**Empfohlener Weg:**

1. **Pyramid-Spiral zuerst** – ohne Ausnahme. Dort werden die Grundlagen (Fragment-Shader-Denken, UV, SDF, Marsch, Normalen, Hash) so erklärt, dass alle anderen Tutorials sie nur noch referenzieren.
2. **Crystal-Lights als zweites** – es führt die zweite große Renderschule (Höhenfelder) und den Werkzeugkasten Noise/FBM/Voronoi ein, und sein **Anhang B ist die zentrale LumiViz-Referenz**, auf die alle anderen verweisen.
3. Danach **nach Interesse verzweigen:**
   - **Tunnel** (3a), wenn dich Architektur, Lichtdesign und Kamerapfade reizen – es ist der direkteste Nachfolger von Pyramid-Spiral (wieder ein Tunnel, aber mit Wand-Detail, Fenstern und Pfad).
   - **Debris** (3b), wenn dich Objektfelder und Beleuchtungs-Stimmungen reizen – es vertieft die Domain-Repetition aus Pyramid-Spiral Schritt 7/B1 in 3D mit Rotation.
   - **Kaleidoscope** (3c) ist **unabhängig vom 3D-Strang** und braucht nur die Pyramid-Grundlagen (UV, Hash, Palette): der richtige Einstieg, wenn du zuerst Feedback/Zustand verstehen willst – das Kapitel, das Milkdrops Warp-Schleife erklärt.
4. **Juggernaut zuletzt im 3D-Strang** – es setzt Marsch-Sicherheit voraus (Verschneidungen sind keine exakten SDFs mehr) und baut auf den Licht-Ideen von Tunnel/Debris auf.
5. **Composites** ganz am Ende – sie mergen die fertigen Shader und setzen die jeweiligen Gesamtlistings voraus. Empfohlene Reihenfolge: **Portals zuerst** (führt das Kondensieren ein, auf das Transitions verweist), dann **Postfx** (Multipass-Ketten), dann **Transitions** (nutzt Kondensieren + Buffer-Zustand).

**Anhänge quer lesen:** Anhang A (Audio) ist überall ähnlich aufgebaut (A1 Signal verstehen → A2 Mapping-Katalog → A3 Einbau). Wer Audio zum ersten Mal macht, liest A1/A2 in **Pyramid-Spiral** (dort am ausführlichsten erklärt) und danach nur noch die A2-Kataloge der anderen. Beat-Envelopes mit Gedächtnis: **Crystal-Lights B3**.

---

## Technik-Index: „Ich will … lernen"

Der Technik-Index ist zugleich die **Entscheidungsmatrix** der Serie: Zeile suchen, Tutorial + Schritt aufschlagen.

| Ich will … | Tutorial (Schritt) |
|---|---|
| Fragment-Shader-Denken, UV-Koordinaten | Pyramid-Spiral 1–2 |
| SDF-Raymarching, Normalen, Beleuchtung | Pyramid-Spiral 3–6 |
| Unendliche Wiederholung (Domain Repetition) | Pyramid-Spiral 7–8 · 3D mit Rotation: Space-Debris 3–6 |
| Raum falten (Kaleidoskop, Twist) | Pyramid-Spiral 9–10 · 2D-Faltungen: Pimped-Kaleidoscope 8–10 |
| Höhenfeld-/Terrain-Raymarching | Crystal-Lights 3 |
| Noise, FBM, Voronoi | Crystal-Lights 4–6 |
| Materialfelder (örtlich wechselndes Material) | Crystal-Lights 7–8 |
| Brechung, Absorption (Beer-Lambert), Transparenz | Crystal-Lights 10 |
| Punktlichter (1/d²), Blink-Rhythmen | Crystal-Lights 9 · Space-Debris 11 |
| Wand-Relief / Oberflächen-Displacement | Stratospheric-Tunnel 4–5 |
| Masken & Öffnungen (Fenster) | Stratospheric-Tunnel 6 · Lücken: Crystal-Lights 8 |
| Emissions-Lichtdesign (Neon, Ringe, Einfall) | Stratospheric-Tunnel 3, 8–9 |
| Gekrümmte Pfade, Vergabelungen | Stratospheric-Tunnel 10–12 |
| Boolesche SDF-Ops, Formbibliotheken | Space-Debris 5 · weich (smin/smax) + Greebles: Juggernaut 4–5 |
| Rotierende Objekte je Zelle (Taumeln) | Space-Debris 6 |
| Planeten, Atmosphäre, Sternenfelder | Space-Debris 1, 7–8, 13 |
| Feedback/Zustand (Vorframe, Decay, Trails) | Pimped-Kaleidoscope 3–7 |
| Blur/Sharpen-Kernel von Hand | Pimped-Kaleidoscope 6 |
| God-Rays, volumetrischer Glow, Größenwirkung | Juggernaut 3, 11–12 |
| Licht-Stimmungen überblenden (dark↔bright) | Juggernaut 7–9 · Ansätze: Space-Debris 9–10 |
| Kamera: Basis, Iso↔Perspektive | Crystal-Lights 12 |
| Kamera-Choreografie (Umkehr, inkommensurable Uhren) | Crystal-Lights 13 · Varianten: alle Folge-Tutorials |
| Politur (Nebel, Farbdrift, 1−exp-Tonemapping, Vignette) | jedes Tutorial, letzter Schritt |
| Audio: FFT/Bänder/Wellenform | Pyramid-Spiral A1–A2 |
| Audio: Beat-Gates (Rock-The-House-Stil) | Crystal-Lights A1 · Wellenform-Seed: Pimped-Kaleidoscope A3 |
| Audio: Envelopes/Gedächtnis (Buffer A) | Crystal-Lights B3 · Pimped-Kaleidoscope (durchgängig) |
| Shadertoy → LumiViz (3 Wege) und zurück | Crystal-Lights Anhang B (Vollreferenz) |
| Einen Shader kondensieren (Listing → Skelett) | Composite-Portals 1–3 |
| Zwei Welten in einem Shader (Namespacing) | Composite-Portals 3 · Composite-Transitions 1–3 |
| Portale / Szene-in-Szene | Composite-Portals 4–6 |
| Material-Id-Dispatch (ein Marsch, mehrere Materialien) | Composite-Portals 7–9 |
| Anti-Aliasing (fwidth, Supersampling) | Composite-Portals 10 |
| Multipass-Architektur, Common-Tab als SSOT | Composite-Postfx 2 · Pimped-Kaleidoscope 13 |
| Nebenkanal-Daten (Tiefe im Alpha) | Composite-Postfx 3 |
| Bloom (Bright-Pass, separierbarer Blur) | Composite-Postfx 4–6 |
| Depth of Field | Composite-Postfx 7 |
| Post-Effekte über 3D-Szenen (Faltungs-Finish) | Composite-Postfx 8–9 |
| Temporal-Glättung / Motion-Blur-Ersatz | Composite-Postfx 10 |
| Übergangs-Kurven & Phasen (Halten/Blenden) | Composite-Transitions 4–5 |
| Masken-Wipes mit Glühsaum | Composite-Transitions 6–8 |
| Parameter-Morph zwischen Welten | Composite-Transitions 9 |
| Zustandsmaschine in Buffer A (Ereignis-Wechsel) | Composite-Transitions 11 |
| Beat-getriggerter Preset-Wechsel, Drop-Detektor | Composite-Transitions A1–A2 |

*Tab. 4: Technik-Index / Entscheidungsmatrix (Technik → Tutorial + Schritt)*

---

## Ausschlusskriterien

Genauso wichtig wie „wo finde ich was" ist „wofür ist die Serie das falsche Werkzeug":

| Vorhaben | Warum hier nicht | Alternative |
|---|---|---|
| Ein fertiges Milkdrop-/AVS-Preset 1:1 portieren | Die Serie lehrt Eigenbau nach Stil-Vorbildern, keine Ports | Import-Pipeline der App (AVS-/MilkDrop-Import, siehe `visuals/`-Doku) |
| Schnell ein fertiges Visual für eine Chain | Tutorials sind Lernwege, keine Vorlagen-Sammlung | Vorrats-Shader in `asset/effectchain/shadertoys/` (100 .lvfx) |
| Schatten/AO, echte Volumetrik, Escape-Time-Fraktale, Texturen lernen | Noch nicht Teil der Serie | Ausblick-Kandidaten (unten); Grundzüge: [Raymarching-Referenz](Raymarching-reference.md) §8 |
| Milkdrop-Warp/Comp-Shader (HLSL) schreiben | Serie ist Shadertoy-GLSL | Milkdrop-Tutorial-Serie (künftig, dieser Ordner) |

*Tab. 5: Ausschlusskriterien mit Alternativen*

## Glossar (Verweis)

Die Fachbegriffe der Serie (SDF, Sphere Tracing, Lipschitz-Konstante, Drosselfaktor, Domain Repetition, Beer-Lambert …) definiert zentral das Glossar der [Raymarching-Referenz](Raymarching-reference.md) (§12) — hier bewusst nicht dupliziert.

---

## Weiterführende Dokumente & Ausblick

**Im Ordner:** [Raymarching-Referenz](Raymarching-reference.md) (formale Referenz zum 3D-Strang) · die 9 Tutorials gemäß Tab. 1/2. **Außerhalb:** App-Doku-Einstieg [INDEX.md](../INDEX.md) · Vorrats-Shader `asset/effectchain/shadertoys/` · Stil-Vorbild-Presets `asset/Milkdrop3/presets/`.

**Kandidaten für spätere Grundlagen-Tutorials** (bewusst nicht in die Composites gequetscht): Licht & Schatten (Schattenstrahlen, SDF-AO, Spiegelungen) · Volumetrik (echte Dichte-Integrale statt Glow) · Escape-Time-Fraktale (Mandelbox – die `mbox`-Brücke zu frosty caves) · Texturen & Cubemaps. Dazu ausstehend: eine Feedback-/Multipass-Referenz als Pendant zur Raymarching-Referenz (siehe Landkarte) und künftige Serien für Milkdrop-Presets und AVS.

## Changelog

| Version | Änderung |
|---|---|
| 1.2.0 | Formalisierung als Kategorie-Overview nach Overview_Base: Header, Taxonomie (Fig. 1), Dokumentenlandkarte mit Coverage (Tab. 3), Ausschlusskriterien (Tab. 5), Glossar-Verweis, Changelog; Tabellen/Diagramme indexiert. Bewusste Abweichungen: Leseempfehlungen (Domain-Ebene) beibehalten, da die Serie faktisch als eigenständiges Fachgebiet gelesen wird; Inhaltsverzeichnis ohne Anker-Links (Konvention der Serie). |
| 1.1.0 | Umzug nach `projects/apps/LumiViz/docs/tutorials/` + FNM-Umbenennung aller Dokumente; Raymarching-Referenz verlinkt; Render-Status der Serie (LumiViz gegengerendert) samt Nachstimm-Kandidaten nachgeführt. |
| 1.0.0 | Erstfassung als „Shader-Tutorials-Wegleitung" (Fokus-Tabellen, Lesehilfe, Technik-Index, Ausblick). |

*Tab. 6: Versionshistorie*
