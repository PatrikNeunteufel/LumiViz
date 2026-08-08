# Changelog Session 72 (2026-08-07/08)

> **ISF-Filter laufen — 321 von 327 Dateien der Vidvox-Bibliothek
> kompilieren und linken.** Der neue Knoten `isfFilter` führt Shader im
> Interactive Shader Format **unverändert** aus, mit beiden Shader-Stufen und
> Multipass. Dazu die Lizenz-/Herkunft-Kette für alle Fremd-Importe, ein
> generischer Parameter-Baum und der iChannel-Ausbau des Shadertoy-Knotens.
> Tests 593 grün, alle 3 Builds grün.

## Teil 1 — Lizenz und Herkunft (gilt für JEDEN Fremd-Import)

Bis S71 hingen `name`/`author`/`url`/`license` nur an `ShadertoyParams` — für
alles andere gab es sie nicht.

- NEU **`Herkunft`** als gemeinsame Struktur; `herkunftVon()` beantwortet
  „welche Knoten sind importfähig". Das Panel zeigt sie als Feld.
- **Der Shader-Export schreibt sie jetzt mit** — vorher ging beim Weitergeben
  einer Datei Autor und Lizenz verloren (Befund S71).
- **🐞 Behoben: ein umbenannter Shadertoy-Knoten verlor seinen Namen.**
  `nodeToJson` schrieb `o["name"] = displayName`, danach überschrieb der
  Shadertoy-Visitor genau diesen Schlüssel mit dem Shader-Namen. Die Herkunft
  hat jetzt ein eigenes Nest; bestehende `.lvfx` lesen unverändert weiter.

## Teil 2 — ISF-Filter als eigener Knotentyp

**Entscheid:** ISF bekommt einen **eigenen** Knoten; der `pixelFilter` bleibt,
was er war. Der Grund: ISF kennt neben dem Fragment- auch einen
**Vertex-Shader**. Ihn in eine Filterfunktion zu falten hätte Ablehnungen für
alles Geometrische erzwungen.

**Es gibt keine Kategorien, nur unterschiedlich viele Bildquellen:**

| Bildquellen | Sorte |
|---|---|
| 0 | Generator |
| 1 | Filter |
| 2 | Übergang |

Jede Sorte darf zusätzlich einen `.vs` haben. Abgelehnt wird nur noch, was
**gar keine** ISF-Datei ist.

**Der Knoten** („ISF Filter (Fragment + Vertex)", Palette „— GPU-Module —"):

- **Import-Knopf im Panel** — lädt die `.fs` samt gleichnamiger `.vs`.
- **Quellen-Liste** mit Name und Bindung, mit „+" und „✕": Ketten-Bild
  (Vorgabe, AVS-Konvention „nichts gewählt = normale Pipeline") · schwarz ·
  Audio (Waveform) · Audio (Spektrum) · AVS-Buffer 1–8.
- **Sorten-Info**, Herkunfts-/Lizenz-Feld, Import-Hinweise als Zeile.
- **Parameter-Baum** für die `INPUTS` — echte Uniforms, ein Zug am Schieber
  kostet **keine** Neuübersetzung.
- **Geometrie** Quad (ISF-treu) · ein übergroßes Dreieck (keine geknickte
  Varying-Interpolation) · Gitter, dazu Mix und Blend.

## Teil 3 — Multipass (`PASSES`)

Die Klasse von Effekten, die in **einem** Durchgang grundsätzlich nicht geht:
separabler Weichzeichner, Bloom, Auto-Levels/Histogramm.

- Je `TARGET` ein Puffer-**Paar** — ein Durchgang darf sein eigenes Ziel lesen
  (`PERSISTENT`-Feedback: gelesen wird das Vorframe).
- `FLOAT`-Ziele als RGBA32F; frische Puffer werden geschwärzt.
- `WIDTH`/`HEIGHT` sind **Ausdrücke** (`$WIDTH/16`, `floor(…)`, `min`/`max`) —
  daran hängen die Reduktionsstufen. Was nicht aufgeht, fällt auf Vollbild
  zurück: ein zu großer Puffer rechnet langsamer, aber richtig.

**Wirkung:** 251 → **321 von 327** Dateien kompilieren und linken.

## Teil 4 — Audio

- **🐞 Behoben:** beide ISF-Audio-Typen hingen an der kombinierten
  512×2-Shadertoy-Textur. Ein Shader, der bei `y = 0.5` abtastet, las eine
  **Mischung aus Spektrum und Waveform**. ISF verlangt **zwei getrennte**
  Texturen (`audio` = Waveform, `audioFFT` = Spektrum) — jetzt so.
- **Gemeinsame Audio-Texturen:** alle drei Zuschnitte kommen aus **einer**
  Rechnung, damit dB-Kurve und Glättung nicht auseinanderlaufen; höchstens
  ein Upload je Frame (vorher lud jeder Shadertoy-Knoten dieselben Daten
  erneut hoch).
- **`getosc()`/`getspec()` als GLSL-Funktionen** — dieselben Namen und
  dieselbe Bedeutung wie in den EEL-/Lua-Skripten. `channel` steht der
  AVS-Signatur wegen da; die Analyse ist heute mono.
- **🐞 `IMG_SIZE` lieferte pauschal `RENDERSIZE`** (also die Bildbreite) →
  `textureSize(sampler, 0)`, exakt für jeden Eingang.

## Teil 5 — iChannel-Ausbau des Shadertoy-Knotens

Zwei neue Quellen je Kanal:

- **Ketten-Eingang** — macht Shadertoy-**Bildfilter** mit `iChannel0`-Eingang
  direkt lauffähig.
- **AVS-Buffer 1–8** — die „Buffer Save"-Slots als Textur; Stand beim Lauf des
  Knotens, Host-Gruppen-Trennung inklusive.

## Sonstiges

- **Der Dateidialog zeigte keine `.fs`** — die Vorauswahl stand auf
  `*.<vertrag>.<endung>`. NEU `importZusatz` an der Vertrags-SSOT.
- **🐞 Der Knotenname klebte an der ersten importierten Datei**; ein
  führendes „by " aus `CREDIT` fällt weg („von by Carter Rosenberg").
- **Import-Hinweise stehen im Panel**, nicht in einem Dialog. Lizenz und
  Sorte haben ihr eigenes Feld — eine gewöhnliche Datei meldet gar nichts.

## Verifikation

- Tests **593 grün, 0 Skips** (Start 563), alle 3 Builds grün.
- Feld-Inventar **91 Typen / 781 Felder, 0 Lücken**.
- ISF-Kopf-Korpus: **327/327** angenommen. GL-Smoke: **321/327** (98 %)
  kompiliert und gelinkt, Wächter-Untergrenze 95 %.
- Der S71-Vorbau (`ShaderVertrag`, Import-Prüfung, Export-Namensschema) war
  **ganz ungetestet** — hat jetzt eine eigene Suite.

## Offen

⬜ Sichttest der ISF-Filter nach dem Multipass-Ausbau · ⬜ Sichttest der 12
pixelFilter-Werks-Looks (seit S70) · 🟠 Key-Variablen-Regel für alle
Import-Formate vereinheitlichen (inkl. eines ungeprüften Shadertoy-Befunds
seit S65).
