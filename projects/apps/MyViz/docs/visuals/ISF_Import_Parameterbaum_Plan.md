# ISF-Import & generischer Parameter-Baum — Umsetzungsplan

> **Version:** 1.0.0
> **Datum:** 2026-08-07 (Session 71)
> **Typ:** Umsetzungsplan
> **Status:** Entwurf — Freigabe Patrik ausstehend
> **Sprache:** Deutsch
> **Anlass:** Entscheid Patrik S71 („Filter laden" als Pixelshader-Pendant) +
> Entwurf Parameter-Baum + Vorgabe „dasselbe Prinzip auch für andere Module"

---

## 1. Ziel

Zwei Dinge, die sich gegenseitig bedingen:

1. **ISF-Filter importieren** — die Kategorie „FX" des Interactive Shader
   Format hat exakt den `pixelFilter`-Vertrag (Bild rein, Bild raus). Damit
   wird eine gepflegte Online-Bibliothek nutzbar
   ([Vidvox/ISF-Files](https://github.com/Vidvox/ISF-Files), gut 200 Stück;
   [editor.isf.video](https://editor.isf.video)).
2. **Beliebige Parameter bedienbar machen** — ISF-Shader bringen eigene
   Regler mit (`INPUTS`). Ein Import ohne Regler wäre ein Torso. Der dafür
   nötige Baustein ist **kein ISF-Sonderweg**, sondern die Infrastruktur, die
   auch der Backlog-Punkt „dynamische Modulparameter" braucht (Vorgabe
   Patrik).

**Abgrenzung:** Kein ISF-*Export* (wir sind kein ISF-Host). Keine
ISF-Generatoren und -Übergänge in Stufe 1–3 (nur `FX`, s. §4.1). Multipass
(`PASSES`) bleibt vorerst außen vor — Multipass-Looks laufen über den
Shadertoy-Knoten.

---

## 2. Ist-Stand (nicht doppelt bauen)

| Vorhanden | Wo | Bemerkung |
|---|---|---|
| `pixelFilter`-Knoten, Vertrag `vec4 farbe(vec2 uv, vec4 src)` | `PixelFilterWrapper.hpp` | GL-frei, testerzwungen — **Muster für den Parser** |
| 12 Werks-Voreinstellungen + Voreinstellungs-Zeile je Knotentyp | `asset/nodepresets/pixelFilter/`, `NodePresetStore` | Filter *als Preset* ist etabliert |
| Import…/Export… an allen Shader-Feldern | `EelScriptEditing.hpp` | seit S69 |
| **Vertrags-SSOT** `ShaderVertrag`, Namensschema, Ordner je Vertrag, Vertragsprüfung beim Import | `EelScriptEditing.hpp` | **S71 — der Vorbau dieses Plans** |
| Shadertoy-Browser: Query-API, Thumbnails, Import als `.lvfx` | `ShadertoyBrowserPanel` | Strang S3, **fertig** |
| Herkunfts-Metadaten `name`/`author`/`url`/`license` | `ShadertoyParams`, `ChainSerializer`, Panel-Label | **nur für Shadertoy**, s. §7 |

**Was fehlt:** ein ISF-Parser, eine Ablage für beliebige getippte Parameter,
deren Darstellung — und ein *generisches* Herkunfts-/Lizenzkonzept.

---

## 3. Reihenfolge und Abhängigkeiten

```
Stufe 1  ISF-Parser (pur, testbar)          ──┐
                                              ├─► Stufe 3  Parameter-Baum (UI)
Stufe 2  Parameter-Ablage im Node + Herkunft ─┘         │
                                                        ▼
                                              Stufe 4  Durchziehen auf alle Module
```

Stufe 1 und 2 sind unabhängig voneinander und beide ohne UI abnehmbar.
Stufe 3 braucht beide. Stufe 4 ist ein eigener, gestaffelter Strang (§6).

---

## 4. Stufe 1 — ISF-Parser

**Neu:** `projects/apps/MyViz/include/visualizers/multieffect/IsfImport.hpp`
(+ `IsfImport.md` im CppModuleDoc-Format). **GL-frei und Qt-GUI-frei** nach
dem Muster von `PixelFilterWrapper.hpp` — dadurch vollständig unit-testbar.

### 4.1 Was der Parser tut

1. **JSON-Kopf abtrennen:** ISF verlangt als Erstes einen Blockkommentar
   `/*{ … }*/`. Fehlt er → kein ISF, klarer Fehler (der Import-Dialog aus S71
   erkennt das bereits am Kopf und meldet es).
2. **Kategorie prüfen:** `INPUTS` enthält einen Eintrag `inputImage` vom Typ
   `image` ⇒ **FX/Filter**, passt auf `pixelFilter`. Kein `inputImage` ⇒
   Generator; `startImage`/`endImage`/`progress` ⇒ Übergang. Beide werden mit
   klarer Meldung abgelehnt (später evtl. auf andere Knoten abbildbar).
3. **`INPUTS` typisiert einlesen** → `IsfParam { name, typ, label, default,
   min, max, labels[], values[] }`. Typen laut Spec: `event`, `bool`, `long`,
   `float`, `point2D`, `color`, `image`, `audio`, `audioFFT`.
4. **Code übersetzen** auf unseren Vertrag:

| ISF | LumiViz `pixelFilter` |
|---|---|
| `void main()` + `gl_FragColor` | Rumpf von `vec4 farbe(vec2 uv, vec4 src)` |
| `IMG_THIS_PIXEL(inputImage)` / `IMG_NORM_PIXEL(inputImage, uv)` | `src` bzw. `texture(uTex, uv)` |
| `IMG_PIXEL(img, px)` | `texture(uTex, px / uResolution)` |
| `IMG_SIZE(img)` | `uResolution` |
| `isf_FragNormCoord` | `uv` |
| `RENDERSIZE` | `uResolution` |
| `TIME` / `TIMEDELTA` / `FRAMEINDEX` | vorhandene Uniforms des Wrappers |
| `INPUTS`-Namen | Parameter (Stufe 2) bzw. `#define`-Fallback |

5. **Report** wie beim AVS-Import: was übersetzt wurde, was nicht abbildbar
   ist (fremde `image`-Inputs, `PASSES`, `audioFFT`-Details) — als Liste von
   Klartextzeilen für den Dialog.

### 4.2 Öffentliche Schnittstelle (Entwurf)

```cpp
struct IsfParam { std::string name, label; IsfTyp typ; /* Werte … */ };
struct IsfImportErgebnis {
    bool ok = false;
    std::string fehler;                 // leer = ok
    std::string code;                   // Rumpf für farbe(uv, src)
    std::vector<IsfParam> parameter;
    std::string name, author, url, license;   // Herkunft (§7!)
    std::vector<std::string> report;
};
IsfImportErgebnis importiereIsf(const std::string& dateiInhalt);
```

### 4.3 Abnahme Stufe 1

- Unit-Tests gegen **echte** ISF-Dateien aus `Vidvox/ISF-Files` (eine
  Handvoll ins Repo unter `asset/testdata/isf/`, Lizenz beachten — sonst
  Pfad-Test lokal).
- Wächter: ein Generator (ohne `inputImage`) wird **abgelehnt**, nicht still
  falsch übersetzt.
- Wächter: der erzeugte Code enthält nie `filter` als Bezeichner
  (GLSL-Reserviert-Befund S70) und nie `IMG_`-Reste.
- Ein importierter Filter rendert in der Sonde deterministisch
  (SHA256-Doppellauf wie `pixelfilter_sonde.lvfx`).

---

## 5. Stufe 2 — Parameter-Ablage im Node + Herkunft

### 5.1 Datenmodell

Neu in `EffectChain.hpp`, **generisch, nicht ISF-spezifisch**:

```cpp
enum class ParamTyp { Bool, Ganzzahl, Zahl, Farbe, Punkt2D, Text, Auswahl };
struct ParamWert {                     // ein Blatt im Baum
    std::string key, label;
    ParamTyp typ;
    double zahl; bool ja; std::string text;   // getippte Ablage
    double min, max; std::vector<std::string> auswahlLabels;
};
struct ParamGruppe {                   // Knoten im Baum (nested!)
    std::string key, label;
    std::vector<ParamWert> werte;
    std::vector<ParamGruppe> gruppen;
};
```

Verschachtelung bewusst von Anfang an — ISF-`INPUTS` sind zwar flach, aber
`PASSES`/`IMPORTED` nicht, und Stufe 4 braucht sie ohnehin (Entwurf Patrik).

`PixelFilterParams` bekommt ein Feld `ParamGruppe parameter;`.

### 5.2 Herkunft/Lizenz — generisch statt shadertoy-spezifisch

**Befund S71:** `name`/`author`/`url`/`license` existieren, hängen aber an
`ShadertoyParams`. Für ISF-Importe fehlen sie. Deshalb: die vier Felder in
eine gemeinsame Struktur `Herkunft` ziehen, die **jeder** importfähige Knoten
tragen kann. `ShadertoyParams` wird darauf umgestellt (Serializer liest die
alten Schlüssel weiter — Abwärtskompatibilität der bestehenden `.lvfx`).

### 5.3 Pflichtkette (nicht verhandelbar)

- **Serializer** `ChainSerializer.cpp`: schreiben + lesen inkl. Klemmen;
  schlankes JSON (leere Felder weglassen, wie bei den Metadaten heute).
- **FieldDocs/Inventar-Gate:** `LUMIVIZ_UPDATE_FIELD_INVENTORY=1` →
  `harvest_field_docs.py` → bauen. Aktueller Stand 90 Typen / 774 Felder,
  Ziel weiterhin **0 Lücken**.
- **Tests:** Roundtrip (Baum mit Verschachtelung + alle Typen), Klemmen
  (min/max), Alt-Format-Lesen der Shadertoy-Metadaten.

### 5.4 Abnahme Stufe 2

Roundtrip-Test grün, FieldDocs 0 Lücken, bestehende `.lvfx` mit
Shadertoy-Metadaten laden unverändert.

---

## 6. Stufe 3 — Parameter-Baum im Panel

**Entwurf Patrik:** Aufbau **wie die Effect-Chain** (Baum), aber mit
**Wert-Spalte rechts** und **typsicheren Editoren** je Zeile.

- Widget: `QTreeWidget`, 2 Spalten (`Parameter` | `Wert`), Editoren per
  `setItemWidget` — Gruppen sind aufklappbare Elternzeilen ohne Editor.
- Editor je `ParamTyp`: `Bool` → Checkbox · `Ganzzahl`/`Zahl` → SpinBox mit
  MIN/MAX aus der Deklaration · `Auswahl` → **Dropdown mit Klartext**
  (ISF-`long` liefert `LABELS`+`VALUES`) · `Farbe` → Farbwähler ·
  `Punkt2D` → zwei Felder · `Text` → Zeile.
- Änderungen laufen über **eine** `mutate`-Operation (renderMutex +
  `recompileChain`, undo-fähig) — wie die Voreinstellungs-Zeile.
- Neuer Baustein `include/UI/panels/ParameterBaum.hpp`, **ohne Wissen über
  Knotentypen** (arbeitet nur auf `ParamGruppe`).

### 6.1 Abnahme Stufe 3

Sichttest: ISF-Filter importieren, Regler bewegen, Wirkung im Viewport;
Preset speichern/laden erhält alle Werte; Undo/Redo greift.

---

## 7. Stufe 4 — Durchziehen auf alle Module (eigener Strang)

**Vorgabe Patrik:** dasselbe Prinzip auch für die anderen Module. Das ist der
größte Brocken und braucht eine **Staffelung**, weil heute **90 Typen mit 774
Feldern** über `addInt`/`addDouble`/`addCombo`/`addText`/`addScript`/
`addCodeEditor` in `MultiEffectPanel` gebaut werden.

**Zuerst zu entscheiden (offen, gehört an den Anfang von Stufe 4):**
> Wird der Parameter-Baum die **Ablösung** der bisherigen Panel-Erzeugung,
> oder ein **zweiter Weg** neben ihr?

- *Ablösung* = ein Weg, weniger Code, aber 90 Typen müssen ihre Felder
  deklarativ beschreiben (die Beschreibung existiert im Kern bereits als
  FieldDocs-Inventar!).
- *Zweiter Weg* = kein Risiko für Bestehendes, aber dauerhaft zwei
  Darstellungen — SSOT-Verstoß auf Raten.

**Empfehlung:** Ablösung, aber schrittweise — erst die neuen/skriptbaren
Knoten (`pixelFilter`, `meshWarp`, `gpuParticles`), dann die Farb-/Transform-
Klasse, zuletzt die Meganodes (`milkdrop`, `shadertoy`) mit ihren
Sonderdarstellungen. Jeder Schritt einzeln sichtgetestet.

**Vorarbeit, die sich lohnt:** prüfen, ob das FieldDocs-Inventar als
Deklarationsquelle taugt — dann beschreibt sich das Panel aus derselben
Quelle, die heute schon die Vollständigkeit bewacht.

---

## 8. Lizenz-Pflicht (Querschnitt, 🔴)

Vorgabe Patrik S71 — **im Loader, nicht nur in der Doku**:

1. **Jeder** Fremd-Import (ISF, Shadertoy) schreibt Herkunft, Autor, Quelle
   und Lizenz in den Knoten (§5.2) und zeigt sie im Panel.
2. **Befund S71:** der Shader-**Export** im Editor schreibt heute nur den
   Code — die Herkunft geht verloren. Fix: beim Export einen Kopf-Kommentar
   mit Name/Autor/URL/Lizenz voranstellen, wenn die Felder gefüllt sind.
3. Shadertoy verpflichtet API-Nutzer ausdrücklich auf die Lizenz **jedes
   einzelnen Shaders** (Default meist CC BY-NC-SA); ISF trägt `CREDIT`.
4. Zu klären: Hinweis beim Weitergeben eines Presets mit fremden Lizenzen —
   oder reicht die sichtbare Anzeige im Panel?

---

## 9. Risiken

| Risiko | Umgang |
|---|---|
| ISF-Vielfalt: nicht jeder FX-Shader übersetzt sauber | Report statt stiller Halbübersetzung; Wächter-Tests gegen echte Dateien |
| Stufe 4 berührt 90 Typen | Staffelung, je Schritt Sichttest; Entscheid „Ablösung vs. zweiter Weg" **vor** dem ersten Schritt |
| FieldDocs-Gate blockiert | Inventar-Lauf ist Teil jedes Schritts, nicht Nacharbeit |
| Lizenzfragen | §8, von Anfang an mitgebaut |

---

## 10. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-08-07 | Erstfassung (Session 71) — Auftrag Patrik: genauer Plan für ISF-Import + Parameter-Baum inkl. Durchziehen auf alle Module |
