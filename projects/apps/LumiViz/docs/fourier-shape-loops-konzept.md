# Fourier-Shape-Loops – Konzept

Aus einer vorgegebenen 2D-Form (Kontur) werden über eine abgeschnittene Fourier-Reihe periodische Signale erzeugt. Diese Signale werden als loopbare Audio-Samples bzw. Wavetables exportiert. Die Anzahl Harmonischer **K** ist der zentrale Klangparameter.

---

## 1. Grundidee

Eine geschlossene Kontur ist ein periodisches 2D-Signal:

$$
z(t) = x(t) + i\,y(t), \qquad t \in [0, 2\pi)
$$

Ihre komplexe Fourier-Reihe:

$$
z(t) = \sum_{k=-\infty}^{\infty} C_k\, e^{ikt}
$$

Abgeschnitten bei |k| ≤ K:

$$
z_K(t) = \sum_{k=-K}^{K} C_k\, e^{ikt}
$$

- Eine Periode von t entspricht einem Durchlauf der Kontur. Das ist ein **Single-Cycle**.
- Weil z_K per Konstruktion periodisch ist, loopt das Ergebnis **nahtlos**, ohne Crossfade.
- K bestimmt gleichzeitig zwei Dinge: wie gut die Form erkannt wird (visuell) und wie hell der Klang ist (spektral).

Daraus entstehen drei Ausgabemodi:

| Modus | Signal | Verwendung |
|---|---|---|
| **Stereo-XY** | L = x_K(t), R = y_K(t) | Oszilloskop-Musik: Die Form wird im XY-Modus sichtbar |
| **Mono-Projektion** | p_θ(t) = Re(e^{-iθ} z_K(t)) | Single-Cycle-Waveform; θ dient als Klangfarben-Parameter |
| **Wavetable** | Frames für K = 1 … K_max (oder für θ) | Morphing in Serum, Vital und ähnlichen Synths |

---

## 2. Pipeline

```
Form-Input ─► Kontur extrahieren ─► Resampling (Parametrisierung)
          ─► Normalisieren ─► FFT ─► Harmonische wählen (K, Fenster)
          ─► Synthese ─► Mono/Stereo/Wavetable ─► Normalisieren ─► WAV-Export
```

### 2.1 Form-Input
- **SVG-Pfad** (bevorzugt): exakt und skalierbar. Bibliothek: `svgpathtools`.
- **Bitmap (PNG)**: Kontur mit `skimage.measure.find_contours` oder OpenCV `findContours` extrahieren und die grösste geschlossene Kontur nehmen.
- **Punktliste (CSV)**: direkte Vorgabe.
- Voraussetzung: ein **einzelner geschlossener Pfad**. Bei mehreren Teilpfaden (z. B. Löcher) diese entweder mit einer Brücke zu einem Pfad verbinden oder pro Teilpfad ein eigenes Signal erzeugen.

### 2.2 Parametrisierung (wichtig)
Die Koeffizienten hängen davon ab, **wie schnell** der Stift die Kontur abfährt (siehe Threads-Post). Für Audio bestimmt das direkt die Klangfarbe. Die Parametrisierung ist deshalb ein bewusster Parameter:

| Variante | Beschreibung | Klangwirkung |
|---|---|---|
| `arclength` | konstante Bahngeschwindigkeit (Standard) | ausgewogen, neutrale Referenz |
| `raw` | Originalpunkte, wie im SVG/Bitmap | vom Zufall der Punktdichte abhängig, nicht empfohlen |
| `curvature` | langsamer in Kurven, schneller auf Geraden | betont Details, weicherer Klang |
| `angle` | konstante Winkelgeschwindigkeit um den Schwerpunkt | nur für sternförmige Formen sinnvoll |

Umsetzung: kumulierte Bogenlänge s_m berechnen, auf [0, 1) normieren, ggf. mit einer Gewichtung w(s) verzerren und auf M äquidistante Punkte interpolieren (M = 4096 … 16384).

### 2.3 Normalisieren der Form
- Schwerpunkt abziehen. Das entspricht C_0 = 0 und damit **keinem DC** im Audio.
- Auf max(|x|, |y|) = 1 skalieren.
- Optional: Umlaufrichtung vereinheitlichen (z. B. gegen den Uhrzeigersinn), damit Formen untereinander vergleichbar sind.

### 2.4 FFT und Auswahl der Harmonischen
```
C = fft(z) / M
k = fftfreq(M, 1/M)
```
Strategien zur Auswahl:
- **Nach Ordnung**: |k| ≤ K. Entspricht dem Video, bandbegrenzt und damit audiotauglich.
- **Nach Betrag**: die K grössten |C_k|. Visuell effizienter, aber nicht bandbegrenzt; nur mit zusätzlichem Limit k ≤ k_max verwenden.

**Gibbs-Kontrolle** (Ecken führen zu Überschwingern, Ringing ist hörbar als «Fizz»):
- σ-Faktoren nach Lanczos: C_k ← C_k · sinc(k / (K+1))
- alternativ ein Raised-Cosine- oder Hann-Fenster über k
- Parameter `window ∈ {none, lanczos, hann}`

### 2.5 Synthese
Direkt per Summe (exakt, für beliebige Ausgabelänge N):

$$
z_K[n] = \sum_{|k|\le K} C_k\, e^{i 2\pi k n / N}, \qquad n = 0 \dots N-1
$$

Alternativ per Zero-Padding und inverser FFT der Länge N (schneller).

### 2.6 Mono aus 2D
Projektion unter dem Winkel θ:

$$
p_\theta(t) = x\cos\theta + y\sin\theta = \mathrm{Re}\!\left(e^{-i\theta} z(t)\right)
$$

- θ = 0 ergibt x, θ = π/2 ergibt y.
- Ein Sweep über θ liefert eine eigene Wavetable-Achse (Klangfarbe ohne Formänderung).
- Spektral hat p_θ an der Harmonischen k ≥ 1 den Koeffizienten ½(e^{-iθ}C_k + e^{iθ}\overline{C_{-k}}). Die Drehrichtung der Kontur geht also in die Phasenbeziehung ein.

Optional zusätzlich: |z(t)| (Radius-Hüllkurve) als Modulationsquelle, nicht als Oszillator.

---

## 3. Audio-Randbedingungen

### 3.1 Bandbegrenzung
Bei Grundfrequenz f_0 und Samplerate f_s muss gelten:

$$
K \cdot f_0 < f_s / 2 \quad (\text{Reserve: } K \cdot f_0 \le 0.45\, f_s)
$$

| f_s | f_0 | K_max |
|---|---|---|
| 48 kHz | 55 Hz (A1) | ~390 |
| 48 kHz | 220 Hz (A3) | ~98 |
| 48 kHz | 880 Hz (A5) | ~24 |

Weil die Auswahl nach Ordnung erfolgt, ist das Signal **per Konstruktion aliasfrei**, sofern K passend zu f_0 gewählt wird. Wavetable-Synths erzeugen für höhere Tonlagen eigene Mipmaps. Für sie gilt: K_max ≤ N/2 − 1.

### 3.2 Loop-Länge
- **Single-Cycle / Wavetable-Frame**: N = 2048 Samples (Serum/Vital-Standard). Exakt eine Periode, Loop-Punkte 0 … N−1.
- **Fester Ton als Sample**: N = f_s / f_0 muss ganzzahlig sein. Sonst f_0 leicht anpassen oder mehrere Perioden pro Loop nehmen, sodass m · f_s / f_0 ganzzahlig wird.
- **Tempo-synchroner Loop**: Loop-Dauer T = Beats · 60 / BPM. Grundfrequenz so wählen, dass f_0 = m / T mit ganzzahligem m, und T · f_s ganzzahlig.

### 3.3 Pegel und DC
- C_0 = 0 (siehe 2.3) bedeutet kein DC.
- Peak-Normalisierung auf −1 dBFS (bei Wavetables pro Frame oder global wählbar; global erhält die Lautstärkeunterschiede beim Morphen).
- Export als 32-Bit-Float-WAV.

---

## 4. Evolving Loops (länger als ein Zyklus)

Für Loops, die sich über Zeit verändern und trotzdem nahtlos sind:

1. **K-Morph**: K(τ) über die Loop-Dauer modulieren. Statt K ganzzahlig zu springen, Harmonische weich einblenden: Gewicht g_k(τ) = clip(K(τ) − |k| + 1, 0, 1).
2. **θ-Rotation**: θ(τ) = 2π · τ / T. Eine volle Drehung ist automatisch periodisch.
3. **Form-Morph**: zwischen den Koeffizienten zweier Formen interpolieren, C_k(τ) = (1−α)C_k^A + α C_k^B. Das setzt dieselbe Parametrisierung und denselben Startpunkt voraus.
4. **Nahtlosigkeit**: Jede Modulationsfunktion muss bei τ = T denselben Wert und dieselbe Ableitung haben wie bei τ = 0, z. B. ½(1 − cos(2πτ/T)) oder ein Hin-und-Zurück-Verlauf.

Der Startpunkt auf der Kontur (Verschiebung in t) ändert nur die Phasen, C_k ← C_k e^{ikt_0}. Er hat keinen Einfluss auf das Betragsspektrum, wohl aber auf die Wellenform und damit auf das Crest-Verhältnis.

---

## 5. Parameter-Übersicht

| Parameter | Typ | Default | Wirkung |
|---|---|---|---|
| `input` | Pfad (SVG/PNG/CSV) | – | Form |
| `param` | enum | `arclength` | Klangfarbe über die Geschwindigkeit |
| `M` | int | 8192 | interne Abtastung der Kontur |
| `K` | int / Liste | 1 … 64 | Harmonische; Helligkeit und Formtreue |
| `select` | `order` / `magnitude` | `order` | Bandbegrenzung |
| `window` | enum | `lanczos` | Gibbs-Unterdrückung |
| `mode` | `stereo` / `mono` / `wavetable` | `wavetable` | Ausgabe |
| `theta` | float [rad] | 0 | Projektion im Mono-Modus |
| `N` | int | 2048 | Samples pro Zyklus/Frame |
| `frames` | int | 64 | Anzahl Wavetable-Frames |
| `frame_axis` | `K` / `theta` / `morph` | `K` | was sich über die Frames ändert |
| `fs` | int | 48000 | Samplerate (Sample-Modi) |
| `f0` / `bpm`+`beats` | float | – | Tonhöhe bzw. Loop-Länge |
| `normalize` | `global` / `per_frame` | `global` | Pegel |

---

## 6. Referenz-Implementierung (Kern)

```python
import numpy as np

def resample_arclength(pts, M):
    pts = np.vstack([pts, pts[:1]])                      # schliessen
    seg = np.linalg.norm(np.diff(pts, axis=0), axis=1)
    s = np.concatenate([[0], np.cumsum(seg)]) / seg.sum()
    u = np.linspace(0, 1, M, endpoint=False)
    return np.interp(u, s, pts[:, 0]) + 1j*np.interp(u, s, pts[:, 1])

def coeffs(z):
    z = z - z.mean()                                     # C0 = 0, kein DC
    z = z / np.max(np.abs(np.c_[z.real, z.imag]))
    M = len(z)
    return np.fft.fft(z) / M, np.fft.fftfreq(M, 1/M).astype(int)

def synth(C, k, K, N, window="lanczos", theta=None):
    keep = np.abs(k) <= K
    Ck, kk = C[keep], k[keep]
    if window == "lanczos":
        Ck = Ck * np.sinc(kk / (K + 1))
    n = np.arange(N)
    z = (Ck[:, None] * np.exp(2j*np.pi*kk[:, None]*n/N)).sum(0)
    if theta is None:
        return np.stack([z.real, z.imag], axis=1)        # Stereo-XY
    return np.real(np.exp(-1j*theta) * z)                # Mono

def wavetable_K(C, k, K_list, N=2048):
    frames = [synth(C, k, K, N, theta=0.0) for K in K_list]
    wt = np.concatenate(frames)
    return wt / np.max(np.abs(wt)) * 10**(-1/20)         # −1 dBFS global
```

Export: `soundfile.write("shape_wt.wav", wt.astype(np.float32), 48000, subtype="FLOAT")`.

Für Serum wird die Frame-Länge über einen zusätzlichen `clm `-Chunk im WAV angegeben. Vital erkennt 2048er-Frames in der Regel automatisch. Beide Punkte vor dem Einsatz am konkreten Synth verifizieren.

---

## 7. Projektstruktur (Vorschlag)

```
fourier-shape-loops/
├── shapeloop/
│   ├── io_shape.py      # SVG/PNG/CSV → Punktliste
│   ├── param.py         # Resampling-Varianten
│   ├── fourier.py       # coeffs, Fenster, Auswahl
│   ├── synth.py         # Mono/Stereo/Wavetable, Morphs
│   └── export.py        # WAV (+ clm-Chunk), Metadaten-JSON
├── cli.py               # shapeloop input.svg --mode wavetable --frames 64 ...
├── preview.py           # Matplotlib: Kontur vs. Rekonstruktion, Epizykel-Animation
└── tests/
```

Die Metadaten-JSON pro Export (Form, Parametrisierung, K-Liste, Fenster, f_s, N) hält jedes Sample reproduzierbar.

---

## 8. Validierung

1. **Visuell**: Rekonstruktion z_K über die Originalkontur plotten; maximaler Punktfehler relativ zur Höhe (wie im Post: ~0.3–0.5 % bei K = 100).
2. **Loop-Test**: Sprung zwischen letztem und erstem Sample sowie zwischen den Ableitungen prüfen. Beide müssen in der Grössenordnung der Nachbarsample-Differenzen liegen.
3. **Spektrum**: Energie oberhalb von K muss null sein (bis auf numerisches Rauschen); kein DC.
4. **XY-Check**: Stereo-WAV mit einem Oszilloskop-Plugin oder echtem Oszilloskop im XY-Modus abspielen. Die Form muss erscheinen.
5. **Hörtest**: K-Sweep im Synth; Klicks deuten auf Phasen- oder Loop-Fehler hin, «Fizz» an Ecken auf fehlendes Fenster.

---

## 9. Ausbaustufen

- **Echtzeit-Oszillator** (C++/JUCE oder Embedded): Koeffizienten speichern, additive Synthese mit Sin/Cos-LUT, K und θ als Live-Parameter. Rechenaufwand ~2K MACs pro Sample und Kanal.
- **Formen-Bibliothek**: Batch über einen SVG-Ordner mit konsistenter Benennung (`<form>_<param>_K<n>.wav`).
- **Umkehrung**: aus einer gewünschten Klangfarbe (Betragsspektrum) eine Form suchen, mit den Phasen als freien Parametern. Das ist ein Optimierungsproblem und eher ein Spielplatz.

---

## 10. Offene Entscheide

- Primärer Zielsynth (Serum, Vital, eigener Sampler)? Davon hängen Exportformat und N ab.
- Stereo-XY als künstlerisches Ziel (Bild sichtbar) oder nur Mono-Klang?
- Soll K ganzzahlig springen (hörbare Stufen als Effekt) oder weich morphen?
