# IsfImport — ISF-Filter auf den pixelFilter-Vertrag übersetzen

> **Version:** 1.0.0
> **Datum:** 2026-08-07
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Filter-Strang Stufe 1, Session 72)
> **Modul:** lumi::isf (freie Funktionen)
> **Dateien:** IsfImport.hpp (header-only)
> **Namespace:** lumi::isf
> **Abhängigkeiten:** EffectChain.hpp (nur `Herkunft`), Qt Core (JSON/Regex) —
> kein GL, keine GUI: Text → Text, voll unit-testbar
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

**ISF** (Interactive Shader Format, Vidvox — isf.video) ist „Shadertoy für
Filter": GLSL plus eine JSON-Parameterdeklaration. Die Kategorie **FX** hat
exakt unseren `pixelFilter`-Vertrag — Bild rein, Bild raus — und macht damit
eine gepflegte Bibliothek nutzbar ([Vidvox/ISF-Files][isf], MIT-Lizenz,
327 Dateien).

Dieses Modul ist die **pure Hälfte** des Imports: es übersetzt einen
Dateiinhalt in unseren Vertrag und liefert Parameter, Herkunft und einen
Report. Datei-Dialog, Knoten-Mutation und Panel-Darstellung liegen außerhalb.

[isf]: https://github.com/Vidvox/ISF-Files

## 2. Format (gegen die Spec geprüft, S71)

Das JSON steckt als **Blockkommentar am Dateianfang**, es gibt **keine
Nachbardatei**; Endung `.fs`. Eine ISF-Datei ist für sich vollständig:

```glsl
/*{
    "CREDIT": "by zoidberg",
    "INPUTS": [ { "NAME": "inputImage", "TYPE": "image" }, … ]
}*/
void main() { gl_FragColor = IMG_THIS_PIXEL(inputImage); }
```

**Die Kategorie-Erkennung ist maschinell eindeutig:** ein Filter hat einen
Input `inputImage` vom Typ `image`. Generatoren haben keinen; Übergänge
tragen stattdessen `startImage`/`endImage`/`progress`. Beide werden mit
klarer Meldung **abgelehnt** statt still halb übersetzt — ein Generator ohne
Quellbild ergäbe im Filter-Knoten nur Unsinn.

## 3. API-Kern

| Funktion | Zweck |
|---|---|
| `istIsf(inhalt)` | SSOT der Formaterkennung: Kopf da **und** gültiges JSON |
| `trenneKopf(inhalt, rumpf)` | JSON-Text zurück, `rumpf` bekommt den Code |
| `importiereIsf(inhalt, dateiName, vertexInhalt)` | die Übersetzung |

`ImportErgebnis` trägt `ok`/`error`, den fertigen `code`, die `parameter`
(`IsfParam`), die `herkunft` und `report`-Zeilen im Import-Report-Stil.

## 4. Übersetzungstabelle

| ISF | LumiViz `pixelFilter` |
|---|---|
| `void main()` + `gl_FragColor` | Rumpf von `vec4 farbe(vec2 uv, vec4 src)` |
| `IMG_THIS_PIXEL(inputImage)` | `src` |
| `IMG_NORM_PIXEL(inputImage, p)` | `texture(uTex, p)` |
| `IMG_PIXEL(inputImage, p)` | `texture(uTex, (p) / uResolution)` |
| `IMG_SIZE(inputImage)` | `uResolution` |
| `isf_FragNormCoord` · `vv_FragNormCoord` | `uv` |
| `RENDERSIZE` · `TIME` · `TIMEDELTA` · `FRAMEINDEX` | `uResolution` · `uTime` · `uDelta` · `uFrame` |
| fremder `image`-Input | `vec4(0.0)` — bleibt schwarz, Report-Zeile |

Die Makro-Ersetzung zählt Klammern: `IMG_NORM_PIXEL(inputImage, vec2(a, b))`
hat **zwei** Argumente, nicht drei. Sie läuft von hinten nach vorne, damit
sich schon berechnete Positionen durch das Einsetzen nicht verschieben.

## 5. Zwei Entwurfsentscheide, die nicht offensichtlich sind

### 5.1 `gl_FragColor` wird eine LOKALE Variable, nicht der Rückgabewert

Naheliegend wäre `gl_FragColor = X;` → `return X;`. Das zerbricht aber an
jedem vorzeitigen `return;` — in einer `vec4`-Funktion ein Compilerfehler,
und genau so schreiben viele ISF-Filter ihren Passthrough-Zweig. Deshalb:

```glsl
vec4 farbe(vec2 uv, vec4 src) {
    vec4 _lumi_frag = src;      // gl_FragColor
    …                            // nacktes `return;` → `return _lumi_frag;`
    return _lumi_frag;
}
```

### 5.2 Der `.vs`-Begleiter wandert in `farbe()` hinein

**Befund S72:** 38 der 327 Vidvox-Dateien haben einen `.vs` — und zwar genau
die interessante Klasse (Kanten, Blur, Sharpen). Der Vertex-Shader rechnet
dort die Nachbar-Koordinaten vor und reicht sie als Varying weiter:

```glsl
// Edges.vs
out vec2 left_coord;
void main() { … left_coord = clamp(texc + vec2(-d.x, 0.0), 0.0, 1.0); }
```

Ohne Übersetzung **linkt der Fragment-Shader nicht** — er erwartet einen
Eingang, den unser geteilter Quad-Vertex-Shader nicht liefert. Da unser
Filter ohnehin je Pixel läuft, wandert die Rechnung unverändert nach
`farbe()`: aus dem Varying wird eine lokale Variable, der `.vs`-Rumpf läuft
als Erstes. `isf_vertShaderInit()` entfällt (das erledigt der Host).

Bleibt danach eine `in`/`varying`-Deklaration übrig, die niemand füllen
kann, **bricht der Import mit Klartext ab** — ein Linkerfehler tief im
Treiber wäre für den Nutzer unbrauchbar.

## 6. Parameter in Stufe 1: der `const`-Block

`INPUTS` werden als `const`-Deklarationen mit ihren Vorgabewerten vor den
Code gesetzt, eingefasst von Sentinel-Kommentaren:

```glsl
// --- ISF-Parameter (erzeugt) ---
const float levels = 30.0;
// --- Ende ISF-Parameter ---
```

Damit **kompiliert und rendert der Import sofort**, ohne dass es schon eine
Parameter-Ablage gäbe. Stufe 2/3 (`ParamGruppe` + Parameter-Baum) erzeugt
den Block aus den Nutzerwerten neu, statt ihn zu suchen — die Sentinel sind
die Nahtstelle.

Typ-Abbildung: `float`→`const float` · `bool`/`event`→`const bool` ·
`long`→`const int` (mit `LABELS`/`VALUES` für das spätere Klartext-Dropdown)
· `point2D`→`const vec2` · `color`→`const vec4`.

## 7. Grenzen (Report-Zeilen, kein Hard-Fail)

- **`PASSES`** (Multipass) — nur der letzte Durchgang; Mehrpass-Looks laufen
  über den Shadertoy-Knoten mit Buffer A–D.
- **Fremde `image`-Inputs** — bleiben schwarz. Sie werden **zuerst** ersetzt,
  damit die allgemeine `IMG_*`-Regel sie nicht auf `uTex` umbiegt; das wäre
  das Kettenbild, der Shader sähe also ein *falsches* Bild statt gar keines.
- **`audio`/`audioFFT`** — der Filter-Knoten hat die Skalare
  `bass/mid/treb/vol/beat`, aber keine Audio-Textur.
- **`IMPORTED`-Texturen** — werden nicht geladen.
- **Fehlende `LICENSE`** — Hinweis, damit vor dem Weitergeben nachgetragen
  wird (Lizenz-Pflicht S72, s. `Herkunft` in EffectChain.hpp).

## 8. Tests

`test_IsfImport.cpp`, zwei Ebenen:

1. **Fixtures** unter `asset/testdata/isf/` — von Hand gegen die Spec
   geschrieben, decken jeden Zweig ab (FX · Generator · Übergang · alle
   INPUT-Typen · `.vs`-Begleiter · vorzeitiges `return;` · verschachtelte
   Makro-Argumente). Laufen immer, auch ohne Klon.
2. **Korpus** gegen `../ref/isf` (Vidvox, MIT — außerhalb des Repos, Muster
   des AvsParser-Korpus). Fehlt der Ordner, meldet der Fall das und ist grün.

**Stand S72:** 327 Dateien · 207 als Filter übersetzt · 120 korrekt
abgelehnt (Generator/Übergang) · 38 mit `.vs`-Begleiter · **0 Fehler**.
Wächter: kein `IMG_`-Rest, kein `gl_FragColor`, kein `RENDERSIZE` im
Ergebnis — und nie `filter` als Bezeichner (GLSL-reserviert, Befund S70).

## 9. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-08-07 | Erstfassung (Session 72, Filter-Strang Stufe 1). Befund: 38 Dateien brauchen ihren `.vs`-Begleiter, sonst linkt der Shader nicht |
