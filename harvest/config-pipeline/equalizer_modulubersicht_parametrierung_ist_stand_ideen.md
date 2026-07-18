# Equalizer – Modulübersicht & Parametrierung (IST‑Stand + Ideen)

> Ziel: den bestehenden Equalizer in **wiederverwendbare Module** zerlegen (Bands, Peak‑Hold‑Spawner, Peak‑Partikel) und **alle Parameter** inkl. Wirkung, Bereich und Interaktionen dokumentieren. Am Ende stehen **Ideen**, die besprochen, aber **nicht** implementiert sind – als Fahrplan für echte Modularisierung.

---

## Architektur auf einen Blick

**Pipeline**
1) **AudioSource → FFT → Bandmapping**  
   Normalisierte Bandwerte `0..1` nach gewählter Skala (Linear/Log/Mel) inkl. EMA‑Glättung und dB‑Floor/Ceil.
2) **Color Mapping (Gradient)**  
   Bandfarbe per Gradient – nach **X‑Position** (Frequenz) oder **Amplitude**.
3) **Bars Rendering**  
   Geometrie, Abstände, Orientierung (BottomUp/TopDown).
4) **Peak‑Hold Modul**
   - **Spawner**: sichtbarer Peak‑Marker je Band (Position/Physik + Farbe).  
   - **Partikel**: optionale „Spur“/History‑Marken, die beim Peak gespawnt werden.

---

## Modul A – Bands (Audio/FFT/Mapping)

**Aufgabe**
- FFT‑Daten holen, in `m_Bands` visuelle Bänder remappen, normalisieren, glätten und ggf. clampen.

**Wichtige Parameter**

| Key | Typ | Range/Enum | Default | Wirkung/Notizen |
|---|---|---|---|---|
| `bands` | int | 1..4096 (UI: 8..256 Slider) | 64 | Anzahl Visual‑Bänder. Mehr Bänder ⇒ höhere visuelle Auflösung. *Hinweis:* Für reale Auflösung **FFT** erhöhen. |
| `gain` | float | ≥ 0 | 1.0 | Skaliert Balkenhöhen nach Normalisierung. |
| `bar_gap_px` | float | ≥ 0 | 2.0 | Pixelabstand zwischen Balken. Begrenzung, damit min. 2px Barbreite bleibt. |
| `orientation` | enum | BottomUp/TopDown | BottomUp | Zeichenrichtung. Beeinflusst Vorzeichen der Peak‑Gravity. |

**AudioSource (audsrc::Settings)**

| Key | Typ | Range/Enum | Default | Wirkung |
|---|---|---|---|---|
| `audio.scale` | enum | Linear/Log/Mel | Linear | Frequenz→Band‑Verteilung. |
| `audio.emaAlpha` | float | 0..1 | 0.0 | Zeitliches Glätten (EMA). 0 = aus, 1 = sehr träge. |
| `audio.clamp01` | bool | — | true | Clampt Pegel auf `0..1`. |
| `audio.requestedFft` | int | ≥ 64 (Load toleriert ≥16) | projektspez. | Bevorzugte FFT‑Größe. Höher ⇒ bessere spektrale Auflösung. |
| `audio.floorDb` | float | ~[-120,0] | - | Pegel, der `0.0` entspricht. |
| `audio.ceilDb` | float | ~[-60,20] | - | Pegel, der `1.0` entspricht. |
| `audio.primeEMA` | bool | — | false | EMA mit erstem Frame primen (keine Einlaufdelle). |

**Hinweise & Risiken**
- „Mehrere gleich große Nachbar‑Bänder“ bei sehr **vielen Bändern** ⇒ FFT‑Auflösung limitiert. **Gegenmaßnahme:** `audio.requestedFft` erhöhen (und/oder `m_Bands` moderat halten).

---

## Modul B – Color Mapping (Gradient)

**Aufgabe**
- Erzeugt Farbwert pro Band (Bar/Spawner/Partikel) aus Gradient.

**Parameter**

| Key | Typ | Range/Enum | Default | Wirkung |
|---|---|---|---|---|
| `grad.domain` | enum | ByPosition/ByAmplitude | ByPosition | Steuert, ob `t` im Gradient aus **X‑Position** (Frequenz) oder **Amplitude** stammt. |
| `grad.custom` | bool | — | false | Benutzt **Custom‑Stops** statt 2‑Farben‑Gradient. |
| `grad.low` | color | — | (0,0.8,1,1) | Farbe „low“ (links bzw. niedrige Amplitude). |
| `grad.high` | color | — | (1,0.2,0.2,1) | Farbe „high“ (rechts bzw. hohe Amplitude). |
| `grad.bias_mid` | float | 0..1 | 0.5 | Bias für Mid‑Punkt des Zweifarben‑Gradienten. |
| `grad.stop.count` + `grad.stop.i.(pos|color|bias)` | mixed | pos 0..1 | — | Freie Stop‑Liste bei `grad.custom = true`.

**Interaktion**
- Bei Änderungen am Gradient wird die **Spawner‑Farbe** (wenn `peak.color_auto` & **Spawner‑Freeze aus**) sofort neu berechnet („reseed“).

---

## Modul C – Bars (Rendering/Geometrie)

**Aufgabe**
- Zeichnet die Balken pro Band. Reihenfolge kann variieren (Z‑Order).

**Parameter**

| Key | Typ | Default | Wirkung |
|---|---|---|---|
| `peak.draw_behind_bars` | bool | false | Zeichnet Peaks/Partikel **hinter** den Balken (true) oder **vor** den Balken (false). |

---

## Modul D – Peak‑Hold: Spawner

**Aufgabe**
- Der **Spawner** ist die sichtbare Peak‑Marke je Band. Seine **Position** folgt der Peak‑Logik (Hold/Gravity oder Spring‑Follow). Seine **Farbe** folgt (standardmäßig) **jedes Frame** dem aktuellen Balkenfarbwert.

**Kernverhalten**
1) **Classic Hold‑then‑Fall**  
   - Bei neuem Maximum: sofort an neue Höhe „snappen“, Delay‑Timer setzen.  
   - Nach Delay: mit `gravity` und `falloff` (Dämpfung) abfallen/steigen.
2) **Spring‑Follow (optional)**  
   - Feder‑Dämpfer‑System Richtung aktuelle Band‑Amplitude, `gravity` additiv.  
   - Optional: Delay auch auf den Spring‑Start (`spawner.use_delay`).
3) **Grenzen**  
   - Bei `respawn_on_leave`: außerhalb `[0..1]` sofort auf aktuelle Bandhöhe respawnen.  
   - Sonst: an 0/1 klemmen und Velocity mit `bounce_elasticity` invertieren.

**Farbe (Spawner)**
- Wenn `peak.color_auto = true` **und** `peak.color_freeze_spawner = false`, wird `SpawnerColor` **jedes Frame** aus dem Gradient neu bestimmt (kein Warten auf Respawn).  
- Wenn `peak.color_auto = false`, gilt `peak.color` (fix).

**Parameter (Spawner)**

| Key | Typ | Range | Default | Wirkung |
|---|---|---|---|---|
| `peak.enabled` | bool | — | true | Aktiviert Peak‑Spawner & Partikel. |
| `peak.delay_ms` | float | ≥ 0 | 120 | Hold‑Delay bis Bewegung startet. |
| `peak.falloff_per_sec` | float | ≥ 0 | 3.0 | Lineare Dämpfung (Luftwiderstand). |
| `peak.gravity` | float | -20..20 | 5.0 | Beschleunigung (Vorzeichen mit Orientation). |
| `peak.respawn_on_leave` | bool | — | false | Bei <0/>1 sofort auf aktuelle Bandhöhe respawnen. |
| `peak.color_auto` | bool | — | true | Spawner‑Farbe aus Gradient. |
| `peak.color` | color | — | (1,1,1,1) | Feste Farbe (nur wenn Auto = false). |
| `peak.color_freeze_spawner` | bool | — | false | Spawner‑Farbe wird **nicht** live aktualisiert. |
| `peak.spawner.use_delay` | bool | — | true | Delay auch im Spring‑Modus anwenden. |
| `peak.spawner.spring.enabled` | bool | — | false | Aktiviert Spring‑Follow. |
| `peak.spawner.spring.k` | float | ≥ 0 | 40.0 | Federhärte (Richtung Zielamplitude). |
| `peak.spawner.spring.damping` | float | ≥ 0 | 10.0 | Dämpfung (weniger Überschwingen). |
| `peak.spawner.bounce_elasticity` | float | 0..1 | 0.25 | Elastizität beim Anschlagen an Grenzen. |

**Dicke (Spawner‑Marker)**

| Key | Typ | Range | Default | Wirkung |
|---|---|---|---|---|
| `peak.thick.mode` | enum | Off/Direct/Inverse | Off | Relation Dicke↔Amplitude. |
| `peak.thick.base_px` | float | ≥ 0 | 1.0 | Basisdicke. |
| `peak.thick.scale_px` | float | ≥ 0 | 0.0..12 | Zusätzliche Dicke je nach Modus. |
| `peak.thick.min_px` | float | ≥ 0 | 0 | Untere Klemme. |
| `peak.thick.max_px` | float | ≥ `min` | 24 | Obere Klemme. |

**Z‑Order**: siehe Modul C.

---

## Modul E – Peak‑Partikel

**Aufgabe**
- Beim Überschreiten des alten Peaks kann pro Band ein Partikel gespawnt werden (History/Trail). Partikel bewegen sich ähnlich dem Spawner (Delay/Gravity/Dämpfung) und werden gezeichnet wie dünne Quer‑Marker.

**Spawn‑Logik**
- Aktiv, wenn `peak.spawn.each_peak = true`.  
- Spawnt nur, wenn `ΔAmplitude ≥ peak.spawn.min_delta` **und** `time_since_last ≥ peak.spawn.min_interval_ms`.

**Farbe (Partikel)**
- Beim Spawn: Farbe =  
  • **Auto**: aktuelle **Spawner‑Farbe** des Bands  
  • **Fix**: `peak.color`.  
- Optional: `peak.particles.color_bound_to_spawner = true` ⇒ Partikel **folgen** live der Spawner‑Farbe.  
- `peak.color_freeze_particles = true` ⇒ Partikel behalten **immer** ihre Spawn‑Farbe (ignorieren Binding).

**Parameter (Partikel)**

| Key | Typ | Range | Default | Wirkung |
|---|---|---|---|---|
| `peak.spawn.each_peak` | bool | — | false | Schaltet Partikelspawns an Peaks frei. |
| `peak.spawn.min_delta` | float | 0..1 | 0.0 | Mindestanstieg des Peaks für Spawn. |
| `peak.spawn.min_interval_ms` | float | ≥ 0 | 0 | Mindestzeit zwischen Spawns (pro Band). |
| `peak.max_particles_per_band` | int | ≥ 0 | projektspez. | Hard‑Limit je Band (älteste werden entfernt). |
| `peak.color_freeze_particles` | bool | — | false | Partikel behalten Spawn‑Farbe. |
| `peak.particles.color_bound_to_spawner` | bool | — | false | Partikel passen Farbe live an Spawner an. |

**Bewegung/Bounds**
- Delay/Gravity/Falloff analog Spawner (mit eigenem `tms/vel/pos`).  
- Bei `respawn_on_leave = true`: Partikel respawnen auf aktuelle Bandhöhe; sonst werden sie bei Verlassen von `[0..1]` entfernt.

**Zeichnung**
- Aktuell nutzen Partikel dieselbe **Dickenberechnung** wie der Spawner (`calcPeakThicknessPx(ampV)`).  
- Z‑Reihenfolge: abhängig von `peak.draw_behind_bars` (siehe Modul C).

**Robustheit**
- Entfernen via **erase‑remove** (Iterator‑Sicherheit).

---

## Persistenz (Param‑Keys)

> Alle Parameter sind über `paramDescs/getParam/setParam/saveConfig/loadConfig` serialisiert. Nachfolgend die wichtigsten Keys (Auszug):

**General**: `orientation`, `bands`, `gain`, `bar_gap_px`  
**Audio**: `audio.scale`, `audio.emaAlpha`, `audio.floorDb`, `audio.ceilDb`, `audio.clamp01`, `audio.requestedFft`, `audio.primeEMA`  
**Gradient**: `grad.domain`, `grad.custom`, `grad.low`, `grad.high`, `grad.bias_mid`, `grad.stop.count`, `grad.stop.i.pos`, `grad.stop.i.color`, `grad.stop.i.bias`  
**Z‑Order**: `peak.draw_behind_bars`  
**Spawner (Kern)**: `peak.enabled`, `peak.delay_ms`, `peak.falloff_per_sec`, `peak.gravity`, `peak.respawn_on_leave`  
**Spawner (Farbe)**: `peak.color_auto`, `peak.color`, `peak.color_freeze_spawner`  
**Spawner (Spring)**: `peak.spawner.use_delay`, `peak.spawner.spring.enabled`, `peak.spawner.spring.k`, `peak.spawner.spring.damping`, `peak.spawner.bounce_elasticity`  
**Thickness**: `peak.thick.mode`, `peak.thick.base_px`, `peak.thick.scale_px`, `peak.thick.min_px`, `peak.thick.max_px`  
**Partikel**: `peak.spawn.each_peak`, `peak.spawn.min_delta`, `peak.spawn.min_interval_ms`, `peak.max_particles_per_band`, `peak.color_freeze_particles`, `peak.particles.color_bound_to_spawner`

---

## Bekannte Interaktionen & Stolperfallen

- **Farben übernehmen nicht sofort**: Wenn `peak.color_auto=true` **und** `peak.color_freeze_spawner=false`, wird Spawner‑Farbe **pro Frame** aktualisiert ⇒ kein Warten auf Respawn. Ansonsten bleibt die alte Farbe bis Respawn.  
- **Viele gleich hohe Bänder**: FFT‑Auflösung vs. `bands`. Bei sehr vielen Bändern kann es zu Gruppen identischer Höhe kommen. **Lösung:** `audio.requestedFft` erhöhen.  
- **Iterator‑Fehler (erase)**: In Partikelpfaden ist erase‑remove implementiert (kein „vector erase iterator outside range“).

---

## Ideen (besprochen, **nicht** implementiert) → Modularisierungs‑Fahrplan

**E1) Partikel‑Größe separat**
- Eigenes Thickness‑Set für Partikel (`part.thick.*`) statt Nutzung von Spawner‑Thickness.

**E2) Partikel‑Fadeouts**
- **Alpha‑Fade** über Lebensdauer oder Distanz zur Spawn‑Höhe.  
- **Size‑Fade**: Thickness skaliert mit `life/lifeMax` oder Weglänge.

**E3) Audio‑Reaktivität**
- **Pulsieren** (Größe/Alpha) synchron zur aktuellen Band‑Amplitude.

**E4) Spawn‑Varianten**
- Beat‑detektierter Spawn, Cluster‑Spawn, Trails mit mehreren Kopien.

**E5) Spawner/Partikel‑Physik trennen**
- Eigenes Gravity/Falloff‑Set für Partikel, optional Turbulenz/Noise.

**E6) Farbfluss**
- HSV‑Shift über Lebensdauer; Gradient‑Sampling entlang Zeit statt Position/Amplitude.

**E7) UI‑Modularisierung**
- Tabs pro Modul (Bands, Gradient, Spawner, Particles), Info‑Marker (Tooltip) **bei jedem Parameter**, Direkt‑Eingaben (bereits vorhanden).

---

## Quick‑Defaults (Empfehlungen)
- **Klassisch & ruhig**: `spring.enabled=false`, `falloff=3`, `gravity=5`, `respawn_on_leave=false`, `color_auto=true`, `freeze_spawner=false`, Partikel aus.  
- **Bouncy**: `spring.enabled=true`, `spring.k≈40`, `damping≈10`, `bounce≈0.25`, Delay an.  
- **Neon‑Trail**: Partikel on, `min_delta≈0.05`, `min_interval≈60ms`, `color_bound_to_spawner=true`.

---

> Nächster Schritt: Die Module als eigenständige Klassen/Komponenten abtrennen (Interfaces für Update/Draw/Config/Serialize). Danach die **Ideen** inkrementell pro Modul ergänzen, ohne den Rest zu berühren.

