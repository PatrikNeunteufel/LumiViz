# 📚 viz2025 – Node Reference Manual (Visualizer-Pipeline)

Diese Referenz listet Module (Nodes) einer Audiovisualizer-Pipeline – von **Audioquelle** über **Analyse**, **Visualisierung**, **Compositing** bis zu den **Output-Knoten**. Die Tabellen nutzen **verschachtelte Subtabellen** für *Inputs*, *Parameter* und *Outputs*. Die **Typen** sind farbcodiert (Quadrate) zur einfachen Verbindung im Node-Editor.

---

## 🎨 Typensystem & Farbcodierung

### Formen (Port-Semantik)
- **◻ Rechteck** = **Control-Signal** (berechnete/numerische Steuersignale; kontinuierlich oder diskret)
- **△ Dreieck** = **UI-Parameter** (interaktives Bedienelement wie Slider/Knob/Toggle; kann in einen ◻ Control-Port gespiegelt werden)
- **◯ Rund** = **Datenfluss** (laufende Ströme wie PCM, Spectrum, Image, Geometry, **auch** Event)

> **Merksatz:** **Form = Semantik** (Steuerung vs. UI vs. Datenfluss), **Farbe = Datentyp**.

### Farbcode (Domänen-Datenströme – immer ◯)
| Type | Farbe | Beschreibung |
|---|---|---|
| `PCM` | 🔵 | Zeitbereichs-Audiodaten (Samples) |
| `Spectrum` | 🟣 | Frequenzbereich (FFT, Bänder) |
| `Image` | 🔴 | 2D-Framebuffer/Texture |
| `Geometry` | 🟠 | Mesh/Vertex-Daten |
| `Event` | 🟡 | Trigger/Ereignisse (Impulse) |
| `String` | ⚪ | Text/Dateipfade/Titel |
| `Meta` | ⚫ | Struktur-/Kontextdaten (Time, BPM, Device, Uniform-Mapping) |

### Farbcode (numerische Controls – ◻/△)
| Subtype | Farbe | Beispiele |
|---|---|---|
| **Float** | 🟩 | `f32`, `f64`, `f32[norm01]`, `f32[snorm11]` |
| **Int** | 🟫 | `i16`, `i32`, `i64` |
| **UInt** | 🟦 | `u16`, `u32`, `u64` |
| **Vector** | 🟪 | `f32×2/×3/×4`, `i32×2`, `u32×4` |

> **Darstellung im Typfeld (Beispiele):**  
> `🟩 f32 ◻` = berechnetes Float-Control, `🟩 f32 △` = UI-Slider,  
> `🟪 f32×3 ◻` = Vektor-Control (z. B. RGB),  
> `🔵 PCM ◯` = Datenstrom PCM.

### Implizite Konvertierungen (über Helper/Cast)
- Numerik: `u/int → float` (soweit verlustfrei), `float → bool` (Threshold), `bool → float` (0/1)
- Normierung: `f32 ↔ f32[norm01] / f32[snorm11]` via Scale/Offset
- Vektor/Skalar: Broadcast (Skalar→Vektor) / Kanalwahl (Vektor→Skalar)
- Audio↔Freq: 🔵 `PCM` → 🟣 `Spectrum` (FFT)
- Band→Control: 🟣 `Spectrum` → 🟩 `f32` (Band-Extractor, RMS/Peak)
- Control→Event: 🟩 `f32` → 🟡 `Event` (Threshold/Edge)

**Wiring-Helpers:** `Combine<T>`, `Split<T>`, `Cast<A→B>`, `VectorCompose/Decompose`, `Normalize/Denormalize`, `Clamp`, `Remap`.

---

## ⚙️ Steuerungsmodi: Parameter, Skripte & Shader

Diese Engine unterstützt **drei Steuerquellen** pro Node-Parameter:

1) **UI-Parameter** (**△**) – interaktive Bedienelemente (Slider/Knob/Toggle).  
2) **Control-Signale** (**◻**) – externe Eingänge aus dem Graph (z. B. LFO, Envelope).  
3) **Skripte/Shader** – pro Parameter oder Node global (Ergebnis wirkt wie ein ◻-Control).

**Bindungsreihenfolge (konfigurierbar):** `External ◻` → `Script` → `UI △ (Default)`  
Jeder Parameter hat einen **Binding-Schalter**: *UI* / *Input (◻)* / *Script*. Script kann *per-Parameter* (Expression) oder *Node-weit* (Lua/GLSL) definiert werden.

### Skript-Lebenszyklus (angelehnt an AVS/MilkDrop)
- **Init(ctx)** – einmalig bei Aktivierung/Reset der Node (Konstanten, Precompute, Tabellen)  
- **PerFrame(ctx)** – einmal pro Frame (Animation, globale Zustände)  
- **PerBeat(ctx)** – bei erkannter Beat-Flanke (Beat-Counter, One-Shots)  
- **PerPoint(ctx, i, n)** – N‑mal pro Punkt/Vertex (Superscope/Procedural Geometry)  
- *(optional)* **PerEvent(ctx, topic, payload)** – Reaktion auf Bus-Events (z. B. Onset)

> **Performance-Hinweise:** *PerPoint* ist „heiß“. Für große `n` Vektorisierung/Batching nutzen. *PerSample* vermeiden (zu teuer) – stattdessen *PerFrame* + FFT/Bands.

### Shader-Lebenszyklus (MilkDrop/ShaderToy‑artig)
- **Vertex/Fragment** (GLSL 3.3+/4.5) oder **Compute** Shader.  
- Standard-Uniforms werden automatisch gebunden (siehe unten).  
- Audio & Controls kommen als **Uniforms** oder **Textures** (Spectrum-Texture, PCM-SSBO).

### Standard-Uniforms / Kontext (`ctx` und GLSL Uniforms)
| Name | Bedeutung | Typ |
|---|---|---|
| `time` / `iTime` | Laufzeit in Sekunden | 🟩 `f32` ◻ / GLSL `float` |
| `delta` / `iDeltaTime` | DeltaTime | 🟩 `f32` ◻ / `float` |
| `frame` / `iFrame` | Frame-Zähler | 🟦 `u32` ◻ / `int` |
| `bpm` | erkannte BPM | 🟩 `f32` ◻ |
| `beat_phase` / `iBeatPhase` | 0..1 innerhalb Beat | 🟩 `f32[norm01]` ◻ |
| `resolution` / `iResolution` | (w,h,1/w,1/h) | 🟪 `f32×4` ◻ / `vec3/vec4` |
| `audio_rms`, `audio_peak` | Pegel | 🟩 `f32` ◻ |
| `bands_tex` / `iAudioTex` | Spectrum als 1D/2D Texture | 🔴 `Image` ◯ |
| `pcm_ssbo` | PCM als SSBO (optional) | ⚫ `Meta` ◯ |

### Konstanten (global verfügbar)
`PI` (=3.141592653589793), `TAU` (=2*PI), `HALF_PI`, `E`, `DEG2RAD`, `RAD2DEG`, `EPS` (=1e-6), `INF`  
Lua: als `math.pi`/`viz.PI`; GLSL: `const float PI = 3.14159265;` (bereitgestellt)

### Funktionsbibliothek (Auszug)
- **Math:** `sin cos tan asin acos atan atan2 pow exp log sqrt abs floor ceil fract mod min max clamp saturate lerp mix smoothstep step`
- **Vektor/Geom:** `length dot cross normalize reflect refract rotate2d rotate3d`  
- **Farbe:** `hsv2rgb rgb2hsv palette(…) gradient(…)`  
- **Noise:** `noise2/3/4`, `fbm2/3`, `hash`  
- **Audio Helpers:** `band(idx)`, `band_db(idx)`, `rms()`, `peak()`, `onset()`, `beat()`

---

## 1) Audio Inputs

### 1.1 AudioSourceNode
**Beschreibung:** Liest Audio von Gerät oder Datei, unterstützt Seek/Loop, Gain und Kanalwahl.

#### Inputs
| name | description | type |
|---|---|---|
| *(none)* | – | – |

#### Parameter
| parameter | description | type |
|---|---|---|
| Device | Eingangsquelle: WASAPI/ASIO/Default oder Dateipfad | ⚪ `String` △ |
| Gain | Vorverstärkung (linear oder dB) | 🟩 `f32` △/◻ |
| Channels | Kanalmodus: Mono, Stereo (L/R), Mid/Side | ⚫ `Meta` ◻ |
| Buffer Size | Frames pro Callback (Latenz/Glättung) | 🟦 `u32` △/◻ |
| Loop | Wiederholung am Ende | 🟫 `bool` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| PCM | Interleaved Samples (float) | 🔵 `PCM` ◯ |
| RMS | Kurzzeit-Energie pro Frame | 🟩 `f32` ◻ |
| Peak | Maximalwert pro Frame | 🟩 `f32` ◻ |
| Meta | Trackdaten: Titel, Länge, Position, Samplerate | ⚫ `Meta` ◯ |

---

### 1.2 MicrophoneInputNode
**Beschreibung:** Live-Capture vom Mikrofon mit optionalem Noise-Gate.

#### Inputs
| name | description | type |
|---|---|---|
| *(none)* | – | – |

#### Parameter
| parameter | description | type |
|---|---|---|
| Device | Aufnahmegerät | ⚫ `Meta` △/◻ |
| Gain | Preamp | 🟩 `f32` △/◻ |
| Gate Threshold | Schwellwert für Noise-Gate | 🟩 `f32[norm01]` △/◻ |
| HPF | Hochpass (Hz) | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| PCM | Live-Samples | 🔵 `PCM` ◯ |
| RMS | Pegelindikator RMS | 🟩 `f32` ◻ |
| Peak | Pegelindikator Peak | 🟩 `f32` ◻ |

---

### 1.3 TrackMetaNode
**Beschreibung:** Aggregiert/liefert Track-Metadaten und Fortschritt.

#### Inputs
| name | description | type |
|---|---|---|
| Meta | Metadaten von Quellen | ⚫ `Meta` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Mode | Durchreichen, Mergen oder Override | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Title/Artist | Textuelle Felder | ⚪ `String` ◯ |
| Duration | Gesamtlänge (s) | 🟩 `f32` ◻ |
| Position | Aktuelle Position (s) | 🟩 `f32` ◻ |
| BPM (tag) | BPM aus Tags (falls vorhanden) | 🟩 `f32` ◻ |

---

## 2) Preprocessing

### 2.1 NormalizerNode
**Beschreibung:** Pegelt PCM auf Ziel-RMS/Peak ein. Attack/Release für sanfte Regelung. Floor/Ceiling optional.

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Audiosignal | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Mode | RMS- oder Peak-Normalisierung | 🟫 `enum` △/◻ |
| Target RMS | Zielpegel (RMS) | 🟩 `f32` △/◻ |
| Ceiling | Obere Grenze (Peak Clamp) | 🟩 `f32` △/◻ |
| Floor | Untere Grenze/Noise Floor | 🟩 `f32` △/◻ |
| Floor Mode | Off / Soft Gate / Hard Gate | 🟫 `enum` △/◻ |
| Attack/Release | Zeitkonstanten in ms | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| PCM | Normalisierte Samples | 🔵 `PCM` ◯ |
| Gain Applied | Momentane Verstärkung | 🟩 `f32` ◻ |

---

### 2.2 ChannelSplitterNode
**Beschreibung:** Trennt Kanäle (L/R oder Mid/Side) für separate Verarbeitung.

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Interleaved Stereo/Mehrkanal | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Mode | L/R, Mid/Side, Custom Map | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Left | Linker Kanal | 🔵 `PCM` ◯ |
| Right | Rechter Kanal | 🔵 `PCM` ◯ |
| Mid | (L+R)/2 | 🔵 `PCM` ◯ |
| Side | (L−R)/2 | 🔵 `PCM` ◯ |

---

### 2.3 DownmixNode
**Beschreibung:** Mischt Mehrkanal auf Stereo/Mono; optional Phasenkorrektur.

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Mehrkanal | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Target | Mono/Stereo | 🟫 `enum` △/◻ |
| Preserve Energy | Energiekonservierendes Mischen (Gain-Komp.) | 🟫 `bool` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| PCM | Downmix | 🔵 `PCM` ◯ |

---

### 2.4 LimiterNode
**Beschreibung:** Brickwall/Soft Limiter zur Begrenzung von Spitzen; optional Makeup Gain.

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Audiosignal | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Ceiling | Maximale Ausgangsamplitude/Level | 🟩 `f32` △/◻ |
| Knee | Übergang Härte (Hard/Soft) | 🟫 `enum` △/◻ |
| Makeup Gain | Ausgleichsverstärkung | 🟩 `f32` △/◻ |
| Lookahead | Vorlaufzeit | 🟩 `f32` △/◻ |
| Release | Rücklaufzeit | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| PCM | Limitiertes Signal | 🔵 `PCM` ◯ |
| Gain Reduction | Momentane Reduktion | 🟩 `f32` ◻ |

---

### 2.5 NoiseGateNode
**Beschreibung:** Unterdrückt Rauschen via Floor/Threshold; optional Expander.

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Audiosignal | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Floor | Unterer Schwellwert (öffnet bei Überschreitung) | 🟩 `f32` △/◻ |
| Ratio | Expander-Verhältnis (unter Floor) | 🟩 `f32` △/◻ |
| Attack/Release | Zeitkonstanten | 🟩 `f32` △/◻ |
| Hold | Mindest-Offen/-Zu-Dauer | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| PCM | Gegatetes/expandiertes Signal | 🔵 `PCM` ◯ |
| Gate | Status (Open/Close) | 🟡 `Event` ◯ |

---

## 3) Audio Analysis

### 3.1 FFTNode
**Beschreibung:** Fensterung + FFT; liefert Spektrum (linear/log) und aggregierte Bänder.

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Audiosignal | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| FFT Size | 512…8192 | 🟦 `u32` △/◻ |
| Window | Hann/Hamming/Blackman/Nuttall | 🟫 `enum` △/◻ |
| Log Scale | Logarithmische Frequenzachse | 🟫 `bool` △/◻ |
| Smoothing | Zeitliche Glättung | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Spectrum | Magnitude-Array | 🟣 `Spectrum` ◯ |
| Bands | Aggregierte Bänder | 🟣 `Spectrum` ◯ |

---

### 3.2 EqualizerNode (hierarchisch)
**Beschreibung:** Teilt Spektrum in definierte Bänder; Gain/Q/Typ pro Band; optional Summenausgabe.

#### Inputs
| name | description | type |
|---|---|---|
| Spectrum | Frequenzdaten | 🟣 `Spectrum` ◯ |
| Mod (opt.) | Globale Gain-Modulation | 🟩 `f32` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Band Layout | Anzahl & Frequenzgrenzen je Band (10/31/Custom) | ⚫ `Meta` ◻ |
| Type per Band | Peak, Shelf, HP/LP | 🟫 `enum` △/◻ |
| Gain per Band | Verstärkung (dB) | 🟩 `f32` △/◻ |
| Q per Band | Güte | 🟩 `f32` △/◻ |
| Output Mode | Einzelbänder / Summenbild / beides | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Band[i] | Amplitude des i-ten Bandes | 🟩 `f32` ◻ |
| Bands | Array/Band-Spektrum | 🟣 `Spectrum` ◯ |

**Subnodes:** `BandNode` (Band isoliert), `PeakHoldNode` (Peak/Hold), `BandSpawnerNode` (Threshold→Event)

---

### 3.3 EnvelopeFollowerNode
**Beschreibung:** Bildet Hüllkurve 0..1 mit Attack/Release; Ceiling/Floor/ Hold optional.

#### Inputs
| name | description | type |
|---|---|---|
| PCM / Band | Signal oder einzelnes Band | 🔵 `PCM` ◯ / 🟩 `f32` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Attack/Release | ms | 🟩 `f32` △/◻ |
| Sensitivity | Skalierung | 🟩 `f32` △/◻ |
| Ceiling | Maximaler Envelope-Wert | 🟩 `f32[norm01]` △/◻ |
| Floor | Minimaler Envelope-Wert | 🟩 `f32[norm01]` △/◻ |
| Hold | Haltedauer vor Decay | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Env | Hüllkurve | 🟩 `f32[norm01]` ◻ |

---

### 3.4 BeatDetectorNode
**Beschreibung:** Detektiert Beats (Downbeats/Onsets); schätzt BPM.

#### Inputs
| name | description | type |
|---|---|---|
| PCM / Spectrum | Audiosignal | 🔵 `PCM` ◯ / 🟣 `Spectrum` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Sensitivity | Detektor-Schwellwert | 🟩 `f32` △/◻ |
| BPM Range | erwarteter Bereich (min,max) | 🟪 `i32×2` △/◻ |
| Debounce | Mindestabstand in ms | 🟦 `u32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Beat | Beat-Trigger | 🟡 `Event` ◯ |
| BPM | Geschätzte BPM | 🟩 `f32` ◻ |

---

### 3.5 OnsetDetectorNode
**Beschreibung:** Detektiert transiente Ereignisse (perkussive Onsets).

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Audiosignal | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Window | Analysefenster | 🟫 `enum` △/◻ |
| Threshold | Auslösewert | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Onset | Trigger bei Transienten | 🟡 `Event` ◯ |

---

## 4) Visual Generators

### 4.1 ScopeWaveNode (Oscilloscope)
**Beschreibung:** Erzeugt Linien-/Flächen-Geometrie der Wellenform im Zeitbereich.

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Zeitbereichssignal | 🔵 `PCM` ◯ |
| Scale (opt.) | Vertikale Skalierung | 🟩 `f32` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Mode | Line/Points/Fill | 🟫 `enum` △/◻ |
| Thickness | Linienbreite | 🟩 `f32` △/◻ |
| Gain | Zeichnungsverstärkung | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Geometry | Waveform-Linien/Flächen | 🟠 `Geometry` ◯ |

---

### 4.2 BarsSpectrumNode
**Beschreibung:** Erzeugt Balken-Geometrie oder gerendertes Bild inkl. Peak-Hold.

#### Inputs
| name | description | type |
|---|---|---|
| Spectrum | Bänder | 🟣 `Spectrum` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Orientation | Vertikal/Horizontal | 🟫 `enum` △/◻ |
| Falloff | Abfallgeschwindigkeit | 🟩 `f32` △/◻ |
| Peak Hold | Ein/Aus | 🟫 `bool` △/◻ |
| Bar Count | Anzahl Balken (Resampling) | 🟦 `u32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Geometry | Instanzierte Balken | 🟠 `Geometry` ◯ |
| Image | Direkt gerendertes Bild (optional) | 🔴 `Image` ◯ |

---

### 4.3 ParticleEmitterNode
**Beschreibung:** Spawnt Partikel durch Events/Schwellen; Audio-gekoppelte Attribute.

#### Inputs
| name | description | type |
|---|---|---|
| Trigger | Event oder Threshold aus Audio | 🟡 `Event` ◯ / 🟩 `f32` ◻ |
| Control (opt.) | Rate/Farbe/Scatter | 🟪 `f32×N` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Shape | Punkt, Kreis, Mesh | 🟫 `enum` △/◻ |
| Rate | Partikel/s | 🟩 `f32` △/◻ |
| Lifetime | Lebensdauer | 🟩 `f32` △/◻ |
| Velocity/Gravity | Bewegungsparameter | 🟪 `f32×3` △/◻ |
| Color Source | Fix, Palette, Audio-gemappt | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Geometry | Partikel-Instanzen | 🟠 `Geometry` ◯ |
| Event | Spawn-Event (durchgereicht) | 🟡 `Event` ◯ |

---

### 4.4 ShapeGeneratorNode
**Beschreibung:** Erzeugt Primitive (Kreis, Polygon, Grid).

#### Inputs
| name | description | type |
|---|---|---|
| *(optional)* Controls | Parametersteuerung (Radius/Segmente/Rotation) | 🟪 `f32×N` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Primitive | Circle/Polygon/Grid | 🟫 `enum` △/◻ |
| Radius/Size | Größe | 🟩 `f32` △/◻ |
| Segments | Detailgrad | 🟦 `u32` △/◻ |
| Rotation | Grad | 🟩 `f32` △/◻ |
| Fill/Outline | Füllmodus | 🟫 `bool` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Geometry | Primärgeometrie | 🟠 `Geometry` ◯ |

---

### 4.5 SuperscopeNode (AVS‑Style)
**Beschreibung:** Prozedurale Linien/Dot-Scopes über **PerPoint**‑Skripte; `n` Punkte generieren `x,y` (−1..1) und optional `color, thickness`.

#### Inputs
| name | description | type |
|---|---|---|
| PCM/Spectrum (opt.) | Audioquelle für Skript | 🔵 `PCM` ◯ / 🟣 `Spectrum` ◯ |
| Controls (opt.) | Parameter ins Skript | 🟪 `f32×N` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| n | Punktzahl pro Frame | 🟦 `u32` △/◻ |
| Mode | Lines/Dots/Strip/Loop | 🟫 `enum` △/◻ |
| PerPoint | Skript: `x(i,n,t,…)`, `y(i,n,t,…)` | ⚪ `String` △/◻ |
| PerFrame (opt.) | Skript je Frame (Precompute) | ⚪ `String` △/◻ |
| Color/Thickness (opt.) | Skripte/Controls für Farbe/Dicke | ⚪ `String` △/◻ / 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Geometry | Linien oder Punkte | 🟠 `Geometry` ◯ |

**Skript‑IO (PerPoint):** Eingaben: `i`, `n`, `t`, `dt`, `bpm`, `beat_phase`, `band(k)`, `rms`, `peak`, `rand()`.  
Rückgabe: `x`, `y` (−1..1), optional `col` (`f32×3/×4`), `thick`.

---

### 4.6 ShaderVisualNode (MilkDrop/ShaderToy‑artig)
**Beschreibung:** Fullscreen-Shader mit Audio/Time-Uniforms; Per-Frame/Per-Vertex; kompatibel zu GLSL 3.3+/4.5 (optional HLSL Übersetzer). Unterstützt *Defines*, Mehr-Pass, und Spectrum-Textures.

#### Inputs
| name | description | type |
|---|---|---|
| PCM/Spectrum | Audio-Daten als Uniforms/Textures | 🔵 `PCM` ◯ / 🟣 `Spectrum` ◯ |
| Time/Meta | Zeit/BPM/Resolution Context | ⚫ `Meta` ◯ |
| Uniforms | Freie numerische Controls (werden als Uniforms gebunden) | 🟪 `f32×N` ◻ |
| Textures (opt.) | Externe Texturen (Sampler2D) | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Stages | Vertex/Fragment/Compute auswählen | 🟫 `enum` △/◻ |
| Shader Code | GLSL/HLSL Quelle (pro Stage) | ⚪ `String` △/◻ |
| Defines | Präprozessor-Defines (KEY=VALUE) | ⚪ `String` △/◻ |
| Uniform Map | Bindings (Name→Signal) | ⚫ `Meta` ◻ |
| Multi-Pass | Anzahl Durchläufe / Feedback | 🟦 `u32` △/◻ |
| Resolution | Zielauflösung (override) | ⚫ `Meta` ◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Shader-Output | 🔴 `Image` ◯ |

**Standard-Uniforms (auto):** `iTime`, `iDeltaTime`, `iFrame`, `iResolution`, `iBeatPhase`, `iBPM`, `iAudioTex`, `iFFTSize`, `iBandsCount`, `vizParams[0..N]`

--- (MilkDrop/ShaderToy-artig)
**Beschreibung:** Fullscreen-Shader mit Audio/Time-Uniforms; Per-Frame/Per-Vertex; kompatibel zu GLSL 3.3+/4.5 (optional HLSL Übersetzer). Unterstützt *Defines*, Mehr-Pass, und Spectrum-Textures.

#### Inputs
| name | description | type |
|---|---|---|
| PCM/Spectrum | Audio-Daten als Uniforms/Textures | 🔵 `PCM` ◯ / 🟣 `Spectrum` ◯ |
| Time/Meta | Zeit/BPM/Resolution Context | ⚫ `Meta` ◯ |
| Uniforms | Freie numerische Controls (werden als Uniforms gebunden) | 🟪 `f32×N` ◻ |
| Textures (opt.) | Externe Texturen (Sampler2D) | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Stages | Vertex/Fragment/Compute auswählen | 🟫 `enum` △/◻ |
| Shader Code | GLSL/HLSL Quelle (pro Stage) | ⚪ `String` △/◻ |
| Defines | Präprozessor-Defines (KEY=VALUE) | ⚪ `String` △/◻ |
| Uniform Map | Bindings (Name→Signal) | ⚫ `Meta` ◻ |
| Multi-Pass | Anzahl Durchläufe / Feedback | 🟦 `u32` △/◻ |
| Resolution | Zielauflösung (override) | ⚫ `Meta` ◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Shader-Output | 🔴 `Image` ◯ |

**Standard-Uniforms (auto):** `iTime`, `iDeltaTime`, `iFrame`, `iResolution`, `iBeatPhase`, `iBPM`, `iAudioTex` (Spectrum), `iFFTSize`, `iBandsCount`, `vizParams[0..N]` (gebundene Controls)

---|---|---|
| PCM/Spectrum | Audio-Daten als Uniforms | 🔵 `PCM` ◯ / 🟣 `Spectrum` ◯ |
| Time | Zeit/BPM Sync | ⚫ `Meta` ◯ |
| Uniforms | Freie Control-Uniforms | 🟪 `f32×N` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Shader Code | GLSL/HLSL Quelle | ⚪ `String` △/◻ |
| Uniform Map | Bindings (Name→Signal) | ⚫ `Meta` ◻ |
| Resolution | Zielauflösung | ⚫ `Meta` ◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Shader-Output | 🔴 `Image` ◯ |

---

## 5) Visual Modifiers (Filter/Transform)

### 5.1 TransformNode
**Beschreibung:** Transformation im Raum/Bild (Translate/Rotate/Scale).

#### Inputs
| name | description | type |
|---|---|---|
| Geometry / Image | Eingabe | 🟠 `Geometry` ◯ / 🔴 `Image` ◯ |
| Controls (opt.) | Modulation (T/R/S) | 🟪 `f32×N` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Translate | x,y (oder x,y,z) | 🟪 `f32×2/×3` △/◻ |
| Rotate | Grad | 🟩 `f32` △/◻ |
| Scale | Faktor | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Geometry/Image | Transformiertes Ergebnis | 🟠/🔴 ◯ |

---

### 5.2 BlurNode (CPU/GPU)
**Beschreibung:** Gaussian/Kawase/Box-Blur (mehrfach iterierbar).

#### Inputs
| name | description | type |
|---|---|---|
| Image | Frame | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Radius | Weichzeichner-Radius | 🟩 `f32` △/◻ |
| Iterations | Wiederholungen | 🟦 `u32` △/◻ |
| Kernel | Filterkern | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Weichgezeichnet | 🔴 `Image` ◯ |

---

### 5.3 ColorAdjustNode
**Beschreibung:** Farbkorrektur (Hue/Sat/Brightness/Contrast/Tint).

#### Inputs
| name | description | type |
|---|---|---|
| Image | Frame | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Hue/Sat/Bright | Farbparameter | 🟪 `f32×3` △/◻ |
| Contrast | Kontrast | 🟩 `f32` △/◻ |
| Tint | Färbung (RGB/HSV) | 🟪 `f32×3/×4` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Farbjustiert | 🔴 `Image` ◯ |

---

### 5.4 KaleidoscopeNode (AVS-klassisch)
**Beschreibung:** Spiegelsymmetrien/Kachelungen im Bildraum.

#### Inputs
| name | description | type |
|---|---|---|
| Image | Frame | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Segments | Anzahl Segmente | 🟦 `u32` △/◻ |
| Angle | Drehwinkel | 🟩 `f32` △/◻ |
| Offset | Verschiebung | 🟪 `f32×2` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Kaleidoskop-Bild | 🔴 `Image` ◯ |

---

### 5.5 FeedbackNode (MilkDrop-Essenz)
**Beschreibung:** Bildfeedback (Zoom/Warp/Decay) für Trails und Echoes.

#### Inputs
| name | description | type |
|---|---|---|
| Image | Vorheriges Frame | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Zoom | Skalierung pro Frame | 🟩 `f32` △/◻ |
| Warp | Verzerrungsstärke | 🟩 `f32` △/◻ |
| Decay | Abklingen (Alpha) | 🟩 `f32[norm01]` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Feedback-bearbeitet | 🔴 `Image` ◯ |

---

### 5.6 DistortionNode
**Beschreibung:** Nichtlineare Verzerrungen (Ripple, Wave, Lens).

#### Inputs
| name | description | type |
|---|---|---|
| Image | Frame | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Type | Ripple/Wave/Lens … | 🟫 `enum` △/◻ |
| Strength | Amplitude | 🟩 `f32` △/◻ |
| Frequency | Frequenz/Periode | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Verzogen | 🔴 `Image` ◯ |

---

## 6) Compositing & Layers

### 6.1 MixNode
**Beschreibung:** Mischt zwei Bilder mit Blendmodi & Deckkraft.

#### Inputs
| name | description | type |
|---|---|---|
| Image A | Layer A | 🔴 `Image` ◯ |
| Image B | Layer B | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Blend Mode | Add/Screen/Multiply/Overlay… | 🟫 `enum` △/◻ |
| Opacity | Deckkraft | 🟩 `f32[norm01]` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Gemischtes Bild | 🔴 `Image` ◯ |

---

### 6.2 MaskNode
**Beschreibung:** Wendet Graustufen-/Alpha-Maske an, optional invertiert/gefiedert.

#### Inputs
| name | description | type |
|---|---|---|
| Image | Eingangsbild | 🔴 `Image` ◯ |
| Mask | Maske | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Invert | Maske invertieren | 🟫 `bool` △/◻ |
| Feather | Weiche Kante | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Maskiertes Bild | 🔴 `Image` ◯ |

---

### 6.3 LayerNode
**Beschreibung:** Komposition mehrerer Ebenen mit individuellen Blendmodi.

#### Inputs
| name | description | type |
|---|---|---|
| Image[n] | Beliebig viele Layer | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Order | Reihenfolge der Ebenen | ⚫ `Meta` ◻ |
| Blend Modes | Pro Layer | ⚫ `Meta` ◻ |
| Opacities | Pro Layer | 🟩 `f32[]` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Komposit | 🔴 `Image` ◯ |

---

## 7) Control & Utility

### 7.1 LFONode
**Beschreibung:** Periodischer Modulator; syncbar zu BPM/Phase.

#### Inputs
| name | description | type |
|---|---|---|
| *(optional)* Reset/Sync | Phasen-Reset/Synchronisation | 🟡 `Event` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Waveform | Sin/Saw/Square/Triangle | 🟫 `enum` △/◻ |
| Frequency | Hz | 🟩 `f32` △/◻ |
| Phase | 0..1 | 🟩 `f32[norm01]` △/◻ |
| Amplitude | Skalierung | 🟩 `f32` △/◻ |
| Offset | DC-Offset | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Value | Modulationswert | 🟩 `f32` ◻ |
| Gate | Phasen-Event | 🟡 `Event` ◯ |

---

### 7.2 RandomNode
**Beschreibung:** Rausch-/Random-Generator für Werte oder Events.

#### Inputs
| name | description | type |
|---|---|---|
| *(optional)* Seed/Trigger | Startwert/Neuziehung | 🟦 `u32` ◻ / 🟡 `Event` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Mode | White, Perlin, Step | 🟫 `enum` △/◻ |
| Range | Min/Max | 🟪 `f32×2` △/◻ |
| Seed | Startwert | 🟦 `u32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Value | Zufallswert | 🟩 `f32` ◻ |
| Trigger | Zufalls-Event | 🟡 `Event` ◯ |

---

### 7.3 TimeNode
**Beschreibung:** Stellt Zeitbasis, DeltaTime und optionale BPM-Synchronisation bereit.

#### Inputs
| name | description | type |
|---|---|---|
| *(none)* | – | – |

#### Parameter
| parameter | description | type |
|---|---|---|
| BPM | Manuell oder Auto aus Analyse | 🟩 `f32` △/◻ |
| Loop Length | Schleifenlänge (s) | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Time | Laufzeit | 🟩 `f32` ◻ |
| Delta | DeltaTime | 🟩 `f32` ◻ |
| BeatPhase | 0..1 Phase | 🟩 `f32[norm01]` ◻ |

---

### 7.4 Bool/Trigger/Switch Nodes
**Beschreibung:** Konvertieren, togglen, entprellen und verzögern Events/Bools.

#### Inputs
| name | description | type |
|---|---|---|
| Any/Control/Event | Beliebige numerische/Bool/Events | 🟩 `f32` ◻ / 🟫 `bool` ◻ / 🟡 `Event` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Threshold | Control→Bool | 🟩 `f32` △/◻ |
| Debounce | ms | 🟦 `u32` △/◻ |
| Hold | Mindestdauer | 🟦 `u32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Bool | Zustand | 🟫 `bool` ◻ |
| Event | Flanke | 🟡 `Event` ◯ |

---

### 7.5 ExpressionNode
**Beschreibung:** Einzeilige arithmetische **Expression** (mini‑DSL) zur Erzeugung von Controls; ideal für *per‑Parameter Skripte*. Bindet `time`, `bpm`, `beat_phase`, `rand()`, `band(k)`, u. v. m.

#### Inputs
| name | description | type |
|---|---|---|
| *(optional)* Vars | Zusätzliche Variablen/Controls | 🟪 `f32×N` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Expr | Ausdruck, z. B. `sin(time*2*PI)*0.5+0.5` | ⚪ `String` △/◻ |
| Clamp | Min/Max | 🟪 `f32×2` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Value | Ergebnis | 🟩 `f32` ◻ |

---

### 7.6 ScriptControlNode (Lua)
**Beschreibung:** Mehrzeilige **Lua**‑Skripte für komplexe Controls (Stateful). Unterstützt `Init/PerFrame/PerBeat` und Ein-/Ausgänge.

#### Inputs
| name | description | type |
|---|---|---|
| Controls | Externe Variablen | 🟪 `f32×N` ◻ |
| Events (opt.) | Triggers ins Skript | 🟡 `Event` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Script | Lua‑Code (mit `Init`, `PerFrame`, `PerBeat`) | ⚪ `String` △/◻ |
| Seed | RNG‑Seed | 🟦 `u32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Value | Ergebnis/Vector | 🟩 `f32` ◻ / 🟪 `f32×N` ◻ |
| Event | optionale Trigger | 🟡 `Event` ◯ |

---

## 8) Converter / Wiring Helpers / Wiring Helpers

### 8.1 CombineNode<T>
**Beschreibung:** Aggregiert mehrere Eingänge desselben Typs zu einem Array/Bus.

#### Inputs
| name | description | type |
|---|---|---|
| T[0..n] | Mehrere Eingänge vom Typ T | (Form/Farbe gemäß T) |

#### Parameter
| parameter | description | type |
|---|---|---|
| Capacity | Max. Anzahl | 🟦 `u32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| T[] | Array/Bus von T | (Form/Farbe gemäß T) |

---

### 8.2 SplitNode<T>
**Beschreibung:** Zerlegt Arrays/Busse in einzelne Leitungen.

#### Inputs
| name | description | type |
|---|---|---|
| T[] | Array/Bus | (Form/Farbe gemäß T) |

#### Parameter
| parameter | description | type |
|---|---|---|
| Indices | Auswahl | 🟦 `u32[]` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| T[i] | Selektiertes Element | (Form/Farbe gemäß T) |

---

### 8.3 CastNode<A→B>
**Beschreibung:** Typkonvertierung zwischen kompatiblen Domänen (z. B. PCM→Spectrum).

#### Inputs
| name | description | type |
|---|---|---|
| A | Eingangstyp | (Form/Farbe gemäß A) |

#### Parameter
| parameter | description | type |
|---|---|---|
| Mode | Konvertierungsmodus | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| B | Ausgabetyp | (Form/Farbe gemäß B) |

---

### 8.4 ClampNode
**Beschreibung:** Universeller Begrenzer für Steuerwerte; setzt Floor/Ceiling; optional Soft-Knee.

#### Inputs
| name | description | type |
|---|---|---|
| Value | Skalar/Vektor (Control) | 🟩 `f32` ◻ / 🟪 `f32×N` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Floor | Untere Grenze | 🟩 `f32` △/◻ / 🟪 `f32×N` △/◻ |
| Ceiling | Obere Grenze | 🟩 `f32` △/◻ / 🟪 `f32×N` △/◻ |
| Knee | Sanfter Übergang | 🟩 `f32` △/◻ |
| Mode | Hard/Soft/ClampOnly | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Value | Begrenzter Wert | 🟩 `f32` ◻ / 🟪 `f32×N` ◻ |

---

## 9) Output & Recording

### 9.1 ScreenOutputNode (Final Image Output)
**Beschreibung:** Gibt Bild an Fenster/Monitor aus; verwaltet Swapchain/VSync.

#### Inputs
| name | description | type |
|---|---|---|
| Image | Finales Frame | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Resolution | Breite×Höhe/Scaling | ⚫ `Meta` ◻ |
| VSync | Ein/Aus | 🟫 `bool` △/◻ |
| Fullscreen | Fenster/Vollbild | 🟫 `bool` △/◻ |
| Color Space | sRGB/Linear/HDR | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| *(none)* | – | – |

---

### 9.2 AudioOutputNode (Final Audio Output)
**Beschreibung:** Gibt Audio an Ausgabegerät aus (Monitoring/Pass-Through).

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Audio zum Ausgeben | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Device | Zielgerät | ⚫ `Meta` ◻ |
| Latency | Puffergröße | 🟦 `u32` △/◻ |
| Volume | Ausgabepegel | 🟩 `f32[norm01]` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| *(none)* | – | – |

---

### 9.3 VideoRecorderNode
**Beschreibung:** Nimmt Video (optional mit Audio) in Datei auf.

#### Inputs
| name | description | type |
|---|---|---|
| Image | Bildstrom | 🔴 `Image` ◯ |
| PCM (opt.) | Audiospur | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| File | Zieldatei | ⚪ `String` △/◻ |
| Codec | H264/HEVC/ProRes … | 🟫 `enum` △/◻ |
| Bitrate | Videobitrate | 🟦 `u32` △/◻ |
| FPS | Bilder pro Sekunde | 🟩 `f32` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Status | Aufnahmestatus/Ereignisse | 🟡 `Event` ◯ |

---

### 9.4 SpoutSyphonNode (Inter-App Sharing)
**Beschreibung:** Teilt Frames an andere Apps via Spout/Syphon.

#### Inputs
| name | description | type |
|---|---|---|
| Image | Frame | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Stream Name | Identifikator | ⚪ `String` △/◻ |
| Format | RGBA16F/8 | 🟫 `enum` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| *(none)* | – | – |

---

### 9.5 FileImageOutputNode
**Beschreibung:** Schreibt Bilder (PNG/JPG/EXR) – optional als Sequenz.

#### Inputs
| name | description | type |
|---|---|---|
| Image | Einzelbilder/Frames | 🔴 `Image` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Path Pattern | Dateipfad/Pattern (####) | ⚪ `String` △/◻ |
| Format | PNG/JPG/EXR | 🟫 `enum` △/◻ |
| Quality | Kompressionsstufe | 🟩 `f32[norm01]` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Written | Event pro geschriebenem Frame | 🟡 `Event` ◯ |

---

### 9.6 FileAudioOutputNode
**Beschreibung:** Schreibt Audiodatei (WAV/FLAC) aus PCM-Strom.

#### Inputs
| name | description | type |
|---|---|---|
| PCM | Audio | 🔵 `PCM` ◯ |

#### Parameter
| parameter | description | type |
|---|---|---|
| File | Zieldatei | ⚪ `String` △/◻ |
| Format | WAV/FLAC | 🟫 `enum` △/◻ |
| Normalization | Vor dem Schreiben normalisieren | 🟫 `bool` △/◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Written | Abschluss/Progress | 🟡 `Event` ◯ |

---

## 10) Presets & Scripting (optional)

### 10.1 LuaVisualNode
**Beschreibung:** Skriptbarer Visual-Node; Zugriff auf Audio/Time/Uniforms; erzeugt Bild/Events.

#### Inputs
| name | description | type |
|---|---|---|
| PCM/Spectrum | Audio-Inputs | 🔵 `PCM` ◯ / 🟣 `Spectrum` ◯ |
| Control/Meta | Steuerung/Mapping | 🟪 `f32×N` ◻ / ⚫ `Meta` ◻ |

#### Parameter
| parameter | description | type |
|---|---|---|
| Script | Lua-Quelle | ⚪ `String` △/◻ |
| Sandbox | Sicherheits-/API-Level | ⚫ `Meta` ◻ |

#### Outputs
| name | description | type |
|---|---|---|
| Image | Script-Output | 🔴 `Image` ◯ |
| Event | User-Events | 🟡 `Event` ◯ |

---

**Legende (im Typfeld):** `◯` Datenfluss, `◻` Control-Signal, `△` UI-Parameter.

