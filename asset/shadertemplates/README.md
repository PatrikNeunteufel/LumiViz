# Shader-Vorlagen (mitgeliefert)

Startpunkte für die Shader-Felder der Effektkette. Ausgewählt werden sie im
**großen Editor** (⤢) über die Zeile „Vorlage:“, nicht im Panel. Die Wahl ersetzt
den Editor-Inhalt; Strg+Z nimmt sie zurück.

> **Diese Datei ist der Anker der Verzeichnissuche.** `ShaderVorlagen` sucht vom
> Programmordner aufwärts nach `asset/shadertemplates/README.md`. Wird sie entfernt,
> findet die App die mitgelieferten Vorlagen nicht mehr („Leer“ gibt es trotzdem —
> es kommt aus dem Code).

## Aufbau

```
asset/shadertemplates/<schlüssel>/<Name>.<endung>
```

| Schlüssel | Feld | Einstieg | Endung |
|---|---|---|---|
| `shadertoy` | Shadertoy, Image und Buffer A–D | `void mainImage(out vec4 fragColor, in vec2 fragCoord)` | `.glsl` |
| `meshwarp` | Mesh Warp | `vec2 warp(vec2 uv)` | `.glsl` |
| `pixelfilter` | Pixel Filter | `vec4 farbe(vec2 uv, vec4 src)` | `.glsl` |
| `gpuparticles` | GPU-Partikel, Kraftfeld | `vec2 kraft(vec2 pos, vec2 vel, float alter)` | `.glsl` |
| `isffilter` | ISF-Filter, Fragment | `void main()` | `.fs` |
| `isffilter/vs` | ISF-Filter, Vertex | `void main()` mit `isf_vertShaderInit()` | `.vs` |
| `milkdrop/warp` | MilkDrop Warp-Shader | `shader_body { … }` (HLSL) | `.hlsl` |
| `milkdrop/comp` | MilkDrop Comp-Shader | `shader_body { … }` (HLSL) | `.hlsl` |

Der Name der Datei ohne Endung ist der Anzeigename.

**ISF-Vorlagen sind Generatoren ohne Bildquelle.** Bildquellen und Regler eines
ISF-Shaders stehen in seinem JSON-Kopf und kommen über „ISF-Datei importieren…“ in
den Knoten — eine Vorlage ersetzt nur den Code, und ein frischer Knoten hat kein
`inputImage`.

Eigene Vorlagen („Als Vorlage speichern…“) landen **nicht** hier, sondern im
Benutzerordner `<AppData>/LumiViz/shadertemplates/<schlüssel>/`. Bei gleichem Namen
gewinnt die eigene Datei.

## Prüfung

`test_ShaderVorlagen` hält fest: jede Vorlage nennt ihren Einstieg, jede GLSL-Vorlage
kompiliert im echten Knoten, jede HLSL-Vorlage übersetzt der HlslTranspiler.
