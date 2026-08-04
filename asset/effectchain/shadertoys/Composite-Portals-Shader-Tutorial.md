# Composite: Portals – Drei Shader werden EIN Werk

**Ziel:** Ein **Composite** – kein neuer Shader von Grund auf, sondern der handwerkliche **Merge dreier fertiger Shader** dieser Tutorial-Serie zu einem neuen Werk. Das Endbild: der Flug durch den Neon-Röhren-Tunnel aus *Stratospheric Tunnel* – aber durch dessen **Fenster** sieht man nicht mehr nur ein gemaltes Sternenfeld, sondern die **echte Space-Debris-Szene**: taumelnde Trümmer, den Glutplaneten, den Atmosphären-Saum. Und der Tunnelboden ist auf regelmäßigen Streckenabschnitten kein Röhrenblech mehr, sondern das **Crystal-Lights-Terrain**: halbtransparenter Kristall mit blinkenden Lampen darunter. Drei Welten, ein Strahl, ein Bild.

Der Weg dorthin ist der eigentliche Lehrstoff. Vier Techniken trägt dieses Tutorial, die in keinem der Einzel-Tutorials vorkommen: **Kondensieren** (ein 300-Zeilen-Gesamtlisting auf ein Skelett eindampfen, das den Look noch trägt), **Namespacing** (zwei komplette Welten kollisionsfrei in einer Datei), die **Portal-Technik** (ein Strahl wechselt mitten im Bild die Welt) und der **Material-Id-Dispatch** (eine map, zwei Materialien). Dazu die Königs-Lektion jedes Composites: **Kohärenz** – warum drei gute Shader zusammen erst einmal wie eine Collage aussehen, und was dagegen hilft.

**Stil-Vorbilder** (diesmal keine Milkdrop-Presets, sondern die Quell-Tutorials – alle drei liegen im selben Ordner):

- **`Stratospheric-Tunnel-Shader-Tutorial.md`** – die **Wirtsszene**: Röhrenwand mit Neonfugen, Fenstermaske auf der abgerollten Wand-Karte `(w, z)`, Kamera-Scheinwerfer, Vortrieb entlang der Achse. Von hier stammen `tunnelMap`, das Fensterraster und die Neon-Emission.
- **`Space-Debris-Shader-Tutorial.md`** – die **Außenwelt hinter den Fenstern**: 3D-Zellgitter mit Zellregel und Zellwand-Klammer, taumelnde Brocken als Funktion der Zell-Id, der analytische Glutplanet mit Atmosphären-Saum, die Kamera-Blase. Von hier stammen `debrisMap`, `planetHit` und das ganze Taumel-Besteck.
- **`Crystal-Lights-Shader-Tutorial.md`** – der **Boden-Merge**: Höhenfeld mit Voronoi-Platten, der Lücken-Trick (`mix(-6, h, maske)`), die vereinfachte Brechung mit Beer-Lambert-Absorption und die `1/d²`-Lampen unter dem Kristall. Von hier stammen `kristallHoehe`, `kristallLampen` und `shadeKristall`.

**So funktioniert dieses Tutorial:**

- Es läuft **direkt auf Shadertoy**: Jeder Schritt ist ein vollständiger, lauffähiger Shader (ab Schritt 4 zeigen wir nur noch die Änderungen – der jeweils letzte Vollstand bleibt gültig, und am Ende von Schritt 12 steht alles noch einmal am Stück). Kopieren nach [shadertoy.com/new](https://www.shadertoy.com/new), `Alt+Enter` – fertig.
- Jeder Schritt fügt **genau eine Technik** hinzu; unter jedem Schritt stehen Variationsideen (🎨).
- Die Reihenfolge ist diesmal **nicht** Geometrie → Material → Licht (die Einzelteile sind ja fertig), sondern die Schule des Zusammenbaus: **Kondensieren → Namespacing → Portal → Material-Mix → Kanten → Kohärenz.** Erst müssen die Teile klein und sauber sein, dann kommt der Merge, zuletzt der eine gemeinsame Anstrich.
- **Voraussetzung:** die drei Quell-Tutorials – nicht unbedingt durchgearbeitet, aber die jeweiligen Bauplan-Kapitel und Gesamtlistings sollte man gesehen haben. Techniken, die dort hergeleitet wurden (Terrain-Marsch, Zellregel, `1/d²`-Lichter, Beer-Lambert …), werden hier benutzt und nur noch mit einem Satz erinnert.

**Inhalt**

| Phase | Schritte | Thema |
|---|---|---|
| Kondensieren | 1–2 | Die fünf Kondensier-Regeln; Tunnel-Skelett und Debris-Skelett |
| Namespacing | 3 | Zwei Welten in einer Datei – Präfixe, geteilte Helfer, Split-Screen |
| Portal | 4–6 | Strahl-Übergabe, eigener Weltrahmen & Maßstab, Portal-Rahmen |
| Material-Id | 7–9 | map liefert (Distanz, Id); Kristallboden per min(); Kristall-Shading |
| Anti-Aliasing | 10 | Kanten in Pixelbreite (fwidth), 2×2-Supersampling mit Kostenrechnung |
| Kohärenz | 11–12 | EINE Uhr, EINE Palette, EIN Tonemapping – und das Gesamtlisting |
| Anhang A | A1–A3 | Audio-Reaktivität: EIN Audio-Satz für beide Welten |
| Anhang B | B1–B2 | LumiViz kompakt: Panel-Parameter; In-Shader-Merge vs. Chain-Composition |

---

## Der Bauplan: Was wir eigentlich rendern

Bevor die erste Zeile fällt, ein Blick auf die Architektur des Bildes – sie erklärt, warum die Schritte so geordnet sind:

```
                W a n d  (Roehren + Neonfugen)             ← Welt 1: TUNNEL (Wirt)
        ┌───▢━━━━━━▢━━━━━━━━▢━━━━━━▢━━━━━━━━▢───┐
        │   ║PORTAL ║        ║      ║            │
        │   ▼       ▼        ▼      ▼            │
   ~ ~ ~│ Truemmer taumeln, Planet glueht  ~ ~ ~ │  ← Welt 2: DEBRIS – NUR hinter
        │   (eigener Rahmen, Massstab 1:4)       │    Fenstern, per Strahl-Uebergabe
        │                                        │
        │      (o)→  Kamera, Fahrt entlang +z    │
        │                                        │
        ├── Kristallboden ▓▓▒▒░░  (Material-Id 2)┤  ← Welt 3: KRISTALL – abschnitts-
        │     ●   Lampen unter dem Kristall  ●   │    weise, per min() in der map
        └────────────────────────────────────────┘
```

Der Strahl jedes Pixels erlebt das Composite so:

1. Der **Primärstrahl** marcht die **Tunnelwelt**. Deren map liefert nicht nur eine Distanz, sondern ein Paar **(Distanz, Material-Id)** – denn der Boden der Röhre ist auf manchen Abschnitten per `min()` gegen ein Kristall-Höhenfeld getauscht.
2. Trifft der Strahl die **Wand** (Id 1), entscheidet die Fenstermaske: Wandpixel bekommen Röhren-Shading mit Neon und Scheinwerfer. **Fensterpixel aber werden nicht verworfen und nicht angemalt – der Strahl wird ÜBERGEBEN:** neuer Ursprung im Koordinatenrahmen der Debris-Welt, eigener Maßstab, und dort marcht er ein zweites Mal. Das ist die Portal-Technik, und nur Portal-Pixel bezahlen den zweiten Marsch.
3. Trifft der Strahl den **Kristallboden** (Id 2), dispatcht das Shading über die Id: vereinfachte Brechung, Lampenebene unter dem Boden, Beer-Lambert-Absorption – Crystal Lights im Kleinformat.
4. Ganz am Ende laufen **alle** Pixel durch dieselbe Klammer: EINE Farbdrift, EIN Tonemapping, EINE Vignette. Diese Klammer ist keine Nebensache – sie ist der Unterschied zwischen „ein Werk" und „drei Werke übereinander".

Zwei Denkbilder aus den Quell-Tutorials bleiben in Kraft: Die Tunnelwand ist weiter eine **abgerollte Karte** `(w, z)` (Fenster, Neon, Zonen leben darauf), und jedes Trümmerteil ist weiter eine **Funktion seiner Zell-Id**. Neu ist nur die Frage, die sich ein Composite bei jedem Pixel stellt: **In welcher Welt ist dieser Strahl gerade – und wer darf ihn anmalen?**

---
## Schritt 1 – Kondensieren I: die Regeln und das Tunnel-Skelett

**Neu:** Die Lehrtechnik dieses Tutorials – ein 300-Zeilen-Gesamtlisting wird auf ein Skelett eingedampft, das den Look noch trägt. Zuerst die Regeln, dann ihre erste Anwendung: der Tunnel.

Wer zwei fertige Shader mischt, indem er beide Gesamtlistings untereinanderkopiert, bekommt 700 Zeilen Konfliktmasse und ein GPU-Budget jenseits von Gut und Böse. Der erste Schritt jedes Composites ist darum **Subtraktion**. Fünf Regeln leiten sie:

1. **Silhouette behalten.** Die eine Geometrie-Idee, an der man den Shader auf den ersten Blick erkennt. Beim Tunnel: die Röhrenwand mit der `cos(w·N)`-Wölbung und den Fenstern. Nicht die Spanten, nicht das Relief, nicht die Gabelung.
2. **Signatur behalten.** Das eine Material-/Licht-Merkmal, das den Charakter macht. Beim Tunnel: Neonfugen (`1/d²` auf der Fugen-Distanz) plus Kamera-Scheinwerfer. Ringlichter und Fensterlicht sind Politur.
3. **Politur streichen.** Nebel, Farbdrift, Tonemapping, Vignette fliegen raus – sie kommen am Ende **einmal für das Gesamtwerk** zurück (Schritt 11). Eine einzige „Notbelichtung" bleibt vorerst drin, weil `1/d²`-Neon sonst hart clippt.
4. **Choreografie streichen.** Kamera-Uhren, Pfadkrümmung, Banking, Vergabelung: alles raus. Das Composite hat am Ende **eine** Kamera – und die bauen wir neu, nicht dreimal.
5. **Schnittstelle freilegen – und das Budget neu verhandeln.** Das Skelett wird in Funktionen mit klaren Ein-/Ausgängen geschnitten (die Debris-Welt etwa als `debrisWelt(ro, rd) → Farbe`, Schritt 2), denn genau an diesen Nahtstellen greift später der Merge. Und: Ein Skelett rendert nur noch einen Teil des Bildes – seine Schrittzahlen dürfen sinken (der Tunnel von 120 auf 90, Debris von 110 auf 60).

Der Test für „Look-tragend vs. Politur" ist einfach: **Würde jemand, der das Original kennt, das Skelett noch sicher zuordnen?** Solange ja, darf weiter gestrichen werden.

Hier das Ergebnis für den Tunnel – kondensiert aus dem Gesamtlisting von *Stratospheric Tunnel* Schritt 13 (rund 300 Zeilen → gut 90, vollständig und lauffähig):

```glsl
// ============================================================================
// SKELETT 1: Stratospheric Tunnel, kondensiert.
// Behalten:   Roehrenwand, Neonfugen, Fenster, Scheinwerfer, Vortrieb.
// Gestrichen: Pfad/Gabel, Spanten, Relief, Ringlichter, Fensterlicht,
//             Banking, Kamera-Choreografie, Politur (bis auf Notbelichtung).
// ============================================================================

// ---- STELLSCHRAUBEN: TUNNEL ------------------------------------------------
const float T_RADIUS  = 1.0;    // Grundradius der Roehre
const float T_ROEHREN = 12.0;   // Roehren um den Umfang (ganzzahlig!)
const float T_TIEFE   = 0.10;   // Woelbung der Roehren
const float T_SPALTEN = 6.0;    // Fensterspalten um den Umfang (ganzzahlig!)
const float T_ABSTAND = 5.0;    // Fensterabstand entlang z
const float T_DICHTE  = 0.55;   // Anteil der Zellen mit Fenster
const float T_NEON    = 0.010;  // Helligkeit der Neonfugen
const float T_LICHT   = 1.4;    // Kamera-Scheinwerfer
// ----------------------------------------------------------------------------

const float TAU = 6.28318530;

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

vec3 pal(float t) { return 0.5 + 0.5 * cos(TAU * (t + vec3(0.0, 0.33, 0.67))); }

float tunnelMap(vec3 p)
{
    float w = atan(p.y, p.x);
    float relief = T_TIEFE * (0.5 - 0.5 * cos(w * T_ROEHREN));
    return T_RADIUS - relief - length(p.xy);
}

float tunnelMarch(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 90; i++) {
        float d = tunnelMap(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d * 0.7;
        if (t > 40.0) break;
    }
    return min(t, 40.0);
}

vec3 tunnelNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(tunnelMap(p + e.xyy) - tunnelMap(p - e.xyy),
                          tunnelMap(p + e.yxy) - tunnelMap(p - e.yxy),
                          tunnelMap(p + e.yyx) - tunnelMap(p - e.yyx)));
}

float tunnelFenster(float w, float z)   // 0 = Wand, 1 = Fensteroeffnung
{
    vec2 zelle = vec2(fract(w / TAU + 0.5) * T_SPALTEN, z / T_ABSTAND);
    vec2 id = floor(zelle);
    if (hash21(id + 3.1) > T_DICHTE) return 0.0;
    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * T_RADIUS / T_SPALTEN;
    c.y *= T_ABSTAND;
    float d = max(abs(c.x) - 0.30, abs(c.y) - 0.85);
    return smoothstep(0.05, -0.05, d);
}

vec3 tunnelNeon(float w, float z)
{
    float fu  = w * T_ROEHREN / TAU;
    float gid = mod(floor(fu + 0.5), T_ROEHREN);
    float ad  = abs(fract(fu + 0.5) - 0.5) * TAU * T_RADIUS / T_ROEHREN;
    return pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55) * T_NEON / (0.0015 + ad * ad * 60.0);
}

vec3 tunnelShade(vec3 p, vec3 n, vec3 ro, float w)
{
    vec3 basis = vec3(0.06, 0.07, 0.10);
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    vec3 col = basis * (T_LICHT * max(dot(n, zk / dk), 0.0) / (1.0 + dk * dk * 0.12) + 0.02);
    col += tunnelNeon(w, p.z);
    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 1.5);      // Vortrieb pur, gerade Achse
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = tunnelMarch(ro, rd);
    vec3 p = ro + rd * t;
    float w = atan(p.y, p.x);

    vec3 col = tunnelShade(p, tunnelNormale(p), ro, w);
    col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    // PLATZHALTER: hier oeffnet in Schritt 4 das Portal in die Debris-Welt
    float F = tunnelFenster(w, p.z) * exp(-0.001 * t * t);
    col = mix(col, vec3(0.02, 0.03, 0.07), F);

    col = 1.0 - exp(-col * 1.6);                // Notbelichtung (s. Regel 3)
    fragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
```

**Ergebnis:** Unverkennbar der Stratosphären-Tunnel – Röhrenwand, farbige Neonfugen, dunkle Fensterrechtecke, Scheinwerferkegel – aber schnurgerade, ohne Ringe, ohne Relief, ohne Kurven. Die Fenster zeigen vorerst nur ein stumpfes Dunkelblau: Das ist die **freigelegte Schnittstelle**, an der später das Portal andockt.

### Was passiert hier

Jede Streichung ist eine Entscheidung nach den fünf Regeln, und zwei davon lohnen den zweiten Blick:

**Der Distanznebel ist geblieben** (`exp(-0.0016·t²)`), obwohl „Politur streichen" galt. Der Grund steht im Quell-Tutorial: Im Tunnel ist der Nebel keine reine Ästhetik, sondern die **Fehlerdecke** über der Marsch-Kappung bei `t = 40` – ohne ihn stünde am Fluchtpunkt ein hartes Artefakt. Kondensieren heißt nicht „alles Weiche raus", sondern „alles raus, was nicht trägt". Der Nebel trägt.

**Die Fenster sind ein Platzhalter mit Vertrag.** `tunnelFenster` liefert eine Maske 0..1, und `mainImage` mischt damit eine Ersatzfarbe ein. Genau diese eine `mix`-Zeile ist die Stelle, an der das ganze Composite später einhängt – der Platzhalter dokumentiert die Schnittstelle: *„Hier gehört die Farbe einer anderen Welt hin, gewichtet mit F."* Wer kondensiert, sollte solche Andockstellen absichtlich stehen lassen statt sie wegzuoptimieren.

💡 **Warum nicht einfach das Original nehmen und Features auskommentieren?** Weil ein Skelett mehr ist als ein kastriertes Original: Es ist **neu begründet**. Jede Zeile, die drinsteht, hat einen Satz, warum. Auskommentierter Code dagegen schleppt tote Stellschrauben, tote Helfer und ein unverhandeltes Budget mit – und beim Merge zweier Shader ist jede tote Zeile ein Kollisionskandidat mehr (Schritt 3 zeigt, wie schnell das passiert).

### 🎨 Experimentieren

- Die Streichliste anzweifeln: die Spanten aus dem Original zurückholen (`spant`-Term in `tunnelMap`) – kostet 3 Zeilen. Trägt er den Look? Wenn ja: Er darf bleiben. Kondensieren ist Geschmackssache mit Begründungspflicht
- `T_ROEHREN = 24.0`, `T_NEON = 0.02` → prüfen, dass die Stellschrauben des Originals im Skelett noch dieselbe Sprache sprechen
- Das eigene Lieblings-Feature streichen und den Zuordnungs-Test machen: Ab welcher Streichung ist es „irgendein Tunnel"?

🧠 **Merke:** Ein Skelett ist ein Gesamtlisting minus alles, was der Zuordnungs-Test nicht braucht – plus dokumentierte Andockstellen. Die Streichliste im Kopfkommentar ist Teil des Handwerks: Sie sagt dem Leser (und dem eigenen Zukunfts-Ich), was absichtlich fehlt.

---
## Schritt 2 – Kondensieren II: das Debris-Skelett

**Neu:** Dieselben fünf Regeln, zweite Anwendung – und diesmal mit dem Schnittstellen-Gedanken im Zentrum: Die ganze Außenwelt wird hinter **einer** Funktion `debrisWelt(ro, rd)` versammelt, denn genau so wird das Portal sie später aufrufen.

Die Streichliste für *Space Debris* (Gesamtlisting Schritt 14, rund 350 Zeilen → gut die Hälfte): Die **Formbibliothek** schrumpft auf den verbeulten Brocken (der Zuordnungs-Test hängt am Taumeln und am Glutplaneten, nicht an Platten und Ringen), die **Cluster-Ausdünnung** wird ein flacher Hash-Würfel, **Blinklichter, Wolkenschicht, Sternen-Parallaxe** und die sechs Kamera-Uhren fliegen raus. Bleiben müssen: **Zellregel samt Zellwand-Klammer** (das ist Korrektheit, keine Politur!), Taumeln, Sonne, Planet mit Glutadern, Atmosphären-Saum, eine Sternschicht.

```glsl
// ============================================================================
// SKELETT 2: Space Debris, kondensiert.
// Behalten:   Zellgitter + Zellregel/Zellwand-Klammer, EIN Brocken-Typ,
//             Taumeln, Sonne, Glutplanet, Atmosphaeren-Saum, eine Sternschicht.
// Gestrichen: Formbibliothek, Cluster, Blinklichter, Wolken, Parallaxe,
//             Kamera-Choreografie, Politur (bis auf Notbelichtung + Dunst).
// Schnittstelle: debrisWelt(ro, rd) -> Farbe in Linearlicht.
// ============================================================================

// ---- STELLSCHRAUBEN: DEBRIS ------------------------------------------------
const float D_ZELLE    = 3.0;   // Kantenlaenge einer Gitterzelle
const float D_DICHTE   = 0.50;  // Anteil belegter Zellen
const float D_GROESSE  = 0.9;   // Groessen-Budget je Teil (Zellregel!)
const float D_TAUMEL   = 1.0;   // Taumel-Tempo
const float D_PLANET_R = 60.0;  // Kruemmungsradius des Planeten
const float D_PLANET_H = 8.0;   // Abstand Weltnull -> Planetenoberflaeche
const float D_GLUT     = 1.2;   // Intensitaet des Lavagrunds

// abgeleitet - Zellregel: Umkugel (1.1 * Groesse) passt in die halbe Zelle
const float D_MARGE   = D_ZELLE * 0.5 - 1.1 * D_GROESSE;   // 1.5 - 0.99 = 0.51 > 0
const vec3  D_ZENTRUM = vec3(0.0, -(D_PLANET_R + D_PLANET_H), 0.0);
const vec3  D_SONNE   = normalize(vec3(0.65, 0.28, -0.70));
// ----------------------------------------------------------------------------

const float TAU = 6.28318530;

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash13(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

vec3 hash33(vec3 p)
{
    return fract(sin(vec3(dot(p, vec3(127.1, 311.7,  74.7)),
                          dot(p, vec3(269.5, 183.3, 246.1)),
                          dot(p, vec3(113.5, 271.9, 124.6)))) * 43758.5453);
}

float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(i),              hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)), hash21(i + vec2(1, 1)), u.x), u.y);
}

float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

mat3 rotAchse(vec3 a, float w)
{
    float c = cos(w), s = sin(w), k = 1.0 - c;
    return mat3(a.x * a.x * k + c,       a.y * a.x * k + a.z * s,  a.z * a.x * k - a.y * s,
                a.x * a.y * k - a.z * s, a.y * a.y * k + c,        a.z * a.y * k + a.x * s,
                a.x * a.z * k + a.y * s, a.y * a.z * k - a.x * s,  a.z * a.z * k + c);
}

float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float debrisMap(vec3 p)
{
    vec3 id = floor(p / D_ZELLE);
    vec3 q  = mod(p, D_ZELLE) - 0.5 * D_ZELLE;

    // Zellwand-Klammer: konservative Schranke fuer alles ausserhalb der Zelle
    float wand   = D_ZELLE * 0.5 - max(abs(q.x), max(abs(q.y), abs(q.z)));
    float sicher = wand + D_MARGE;

    if (hash13(id + 4.7) > D_DICHTE) return sicher;

    float gr = D_GROESSE * (0.35 + 0.65 * hash13(id + 3.1));

    vec3 achse = normalize(hash33(id + 5.7) - 0.5 + vec3(0.01, 0.02, 0.03));
    float tempo = (0.25 + 1.25 * hash13(id + 9.2)) * D_TAUMEL;
    q = rotAchse(achse, iTime * tempo + TAU * hash13(id + 1.9)) * q;

    float d = sdBox(q, gr * (0.30 + 0.28 * hash33(id + 2.6)));
    d -= 0.08 * gr * sin(4.7 * q.x) * sin(4.3 * q.y) * sin(5.1 * q.z);
    return min(d, sicher);
}

float debrisMarch(vec3 ro, vec3 rd, float tMax)
{
    float t = 0.0;
    for (int i = 0; i < 60; i++) {                 // Budget gesenkt: 110 -> 60
        float d = debrisMap(ro + rd * t);
        if (d < 0.0015 + 0.0015 * t) return t;
        t += d * 0.7;
        if (t > tMax) break;
    }
    return -1.0;
}

vec3 debrisNormale(vec3 p)
{
    const vec2 e = vec2(0.002, -0.002);
    return normalize(e.xyy * debrisMap(p + e.xyy) + e.yyx * debrisMap(p + e.yyx) +
                     e.yxy * debrisMap(p + e.yxy) + e.xxx * debrisMap(p + e.xxx));
}

float planetHit(vec3 ro, vec3 rd)
{
    vec3 oc = ro - D_ZENTRUM;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - D_PLANET_R * D_PLANET_R;
    float h = b * b - c;
    if (h < 0.0) return -1.0;
    float t = -b - sqrt(h);
    return (t > 0.0) ? t : -1.0;
}

vec3 planetFarbe(vec2 q)
{
    float grund = fbm(q * 0.045 + vec2(iTime * 0.010, 0.0));
    float adern = fbm(q * 0.16 + grund * 1.8 + 7.0);
    float glut  = pow(clamp(adern * 1.35 - 0.25, 0.0, 1.0), 2.2) * D_GLUT;
    vec3 col = vec3(0.028, 0.010, 0.012);
    col = mix(col, vec3(0.55, 0.08, 0.015), smoothstep(0.10, 0.45, glut));
    col = mix(col, vec3(1.15, 0.55, 0.10),  smoothstep(0.45, 0.85, glut));
    return col;
}

vec3 planetLicht(vec2 q)   // grobes Gluehen - spaeter auch Lichtfarbe im Tunnel!
{
    return mix(vec3(0.30, 0.05, 0.01), vec3(1.00, 0.45, 0.10),
               smoothstep(0.35, 0.80, fbm(q * 0.030)));
}

vec3 atmosphaere(vec3 ro, vec3 rd)
{
    vec3 oc = ro - D_ZENTRUM;
    float tca = -dot(oc, rd);
    if (tca < 0.0) return vec3(0.0);
    float hmin = sqrt(max(dot(oc, oc) - tca * tca, 0.0)) - D_PLANET_R;
    return exp(-max(hmin, 0.0) * 0.30) * vec3(0.90, 0.32, 0.08) * 0.55;
}

vec3 debrisHimmel(vec3 ro, vec3 rd)
{
    vec3 col = vec3(0.008, 0.010, 0.018);
    vec2 su = rd.xy / (abs(rd.z) + 0.4);
    col += vec3(0.9) * smoothstep(0.994, 1.0, hash21(floor(su * 48.0)));
    return col + atmosphaere(ro, rd);
}

vec3 debrisShade(vec3 p, vec3 n, vec3 rd)
{
    vec3 id = floor(p / D_ZELLE);
    vec3 alb = vec3(0.42, 0.44, 0.47) * (0.55 + 0.90 * hash13(id + 12.5));

    vec3 col = alb * max(dot(n, D_SONNE), 0.0) * vec3(1.30, 1.18, 1.00);
    col += pow(max(dot(reflect(rd, n), D_SONNE), 0.0), 24.0) * vec3(0.90, 0.85, 0.75) * 0.8;

    float hoehe = clamp((p.y + D_PLANET_H) / D_PLANET_H, 0.0, 2.0);
    col += max(-n.y, 0.0) * planetLicht(p.xz) * exp(-hoehe * 1.1) * 0.9;
    return col;
}

// DIE SCHNITTSTELLE: die komplette Aussenwelt als eine Funktion
vec3 debrisWelt(vec3 ro, vec3 rd)
{
    float tP = planetHit(ro, rd);
    float tMax = (tP > 0.0) ? min(50.0, tP) : 50.0;
    float tD = debrisMarch(ro, rd, tMax);

    vec3 col;
    float tHit;
    if (tD > 0.0 && (tP < 0.0 || tD < tP)) {
        vec3 p = ro + rd * tD;
        col = debrisShade(p, debrisNormale(p), rd);
        tHit = tD;
    } else if (tP > 0.0) {
        col = planetFarbe((ro + rd * tP).xz);
        tHit = tP;
    } else {
        return debrisHimmel(ro, rd);
    }

    // Dunst der Aussenwelt: Richtung Planet warm, Richtung All kalt
    vec3 dunst = mix(vec3(0.010, 0.012, 0.022), vec3(0.30, 0.13, 0.05),
                     clamp(-rd.y * 2.2, 0.0, 1.0));
    return mix(col, dunst, 1.0 - exp(-0.0009 * tHit * tHit));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(sin(iTime * 0.04) * 5.0, 1.5, iTime * 0.6);
    vec3 rd = normalize(vec3(uv, 1.4));
    rd.yz *= mat2(cos(-0.15), sin(-0.15), -sin(-0.15), cos(-0.15));  // Blick hinab

    vec3 col = debrisWelt(ro, rd);

    col = 1.0 - exp(-col * 1.3);                // Notbelichtung
    fragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
```

**Ergebnis:** Unverkennbar das Trümmerfeld – taumelnde, sonnenbeschienene Brocken über dem Glutplaneten mit orangem Saum – aber ohne Formenvielfalt, ohne Blinken, ohne Wolken, mit fast statischer Kamera. Der Zuordnungs-Test besteht.

### Was passiert hier

**Die Schnittstelle ist die eigentliche Arbeit dieses Schritts.** Im Original war die Fallunterscheidung Trümmer/Planet/Himmel Teil von `mainImage`; hier wandert sie komplett in `debrisWelt(ro, rd)`. Warum? Weil das Portal in Schritt 4 genau diesen Vertrag braucht: *„Gib mir zu einem Strahl die fertige Farbe deiner Welt – in Linearlicht, mit deinem eigenen Dunst, aber OHNE Tonemapping."* Die Notbelichtung sitzt darum bewusst **außerhalb** von `debrisWelt`, in `mainImage`. Wäre sie drin, würde das Portal später doppelt tonemappen – die Außenwelt sähe flau und ausgewaschen aus, und niemand wüsste sofort, warum. Das ist die erste konkrete Kohärenz-Falle dieses Tutorials, entschärft, bevor sie zuschnappen kann.

**Was NICHT gestrichen wurde, ist so lehrreich wie das Gestrichene:** Die Zellregel (`D_MARGE = 1.5 − 0.99 = 0.51 > 0`) und die Zellwand-Klammer sind Korrektheits-Mechanik – ohne sie frisst sich der Marsch durch Nachbarzellen und die Brocken reißen Löcher. Kondensieren streicht Schmuck, **nie Beweise**. Ebenso bleibt der Debris-eigene Dunst in `debrisWelt`: Er ist Teil der Welt (ihre Luftperspektive in *ihren* Einheiten), nicht Teil der Bild-Politur – der Unterschied wird in Schritt 5 wichtig, wenn die Welt hinter dem Portal einen eigenen Maßstab bekommt.

💡 **Warum 60 statt 110 Marsch-Schritte?** Regel 5: Budget neu verhandeln. Im Original musste der Marsch das ganze Bild tragen; im Composite rendert er nur noch die Fläche hinter Fenstern – typisch 10–20 % der Pixel, und die Blickdistanzen durch ein Fenster sind kürzer als im freien Flug. 60 Schritte mit 0.7er-Drossel reichen dort; die gesparte Hälfte ist das Budget, aus dem später der **zweite Marsch pro Pixel** überhaupt erst bezahlbar wird.

### 🎨 Experimentieren

- Die Formbibliothek zurückholen (Platte/Träger/Ring aus dem Original) – 12 Zeilen, und der Zuordnungs-Test wird noch sicherer. Preis: mehr Kollisionsmasse in Schritt 3. Beide Seiten spüren!
- `D_DICHTE = 0.85` → dichter Gürtel; mit Blick auf Schritt 4: Was dicht ist, verdeckt später das Planet-Glühen hinter den Fenstern
- Skelett-Härtetest: `D_TAUMEL = 0.0` einfrieren – trägt der Look noch? (Antwort: kaum. Das Taumeln IST die halbe Signatur – gut zu wissen, bevor man es je streicht)

🧠 **Merke:** Die Schnittstelle eines Skeletts ist ein Vertrag über drei Dinge: **Eingang** (`ro`, `rd` – mehr weiß die Welt nicht), **Ausgang** (Linearlicht, kein Tonemapping) und **Zuständigkeit** (eigener Dunst ja, globale Politur nein). Wer diese drei Fragen pro Welt beantwortet hat, hat den Merge schon halb bestanden.

---
## Schritt 3 – Namespacing: zwei Welten in einer Datei

**Neu:** Beide Skelette wandern in **einen** Shader – mit Funktions-Präfixen (`tunnel…`/`debris…`), getrennten STELLSCHRAUBEN-Sektionen und **einmaligen** gemeinsamen Helfern. Als Beweis, dass sich nichts in die Quere kommt: ein Split-Screen, links Tunnel, rechts Debris.

Die Bauanleitung ist ein Zusammensetzspiel mit fester Reihenfolge. In eine leere Shadertoy-Datei kommen, von oben nach unten:

1. der **STELLSCHRAUBEN-Block TUNNEL** aus Schritt 1 (unverändert),
2. der **STELLSCHRAUBEN-Block DEBRIS** aus Schritt 2 (samt `abgeleitet`-Teil, unverändert),
3. der folgende Block **gemeinsame Helfer** (er ERSETZT die Helfer beider Skelette),
4. alle `debris…`-, `planet…`-, `atmosphaere`- und `debrisWelt`-Funktionen aus Schritt 2 (unverändert, aber **ohne** deren `hash…`/`vnoise`/`fbm`/`rotAchse`/`sdBox`/`TAU`),
5. alle `tunnel…`-Funktionen aus Schritt 1 (unverändert, **ohne** `hash21`/`pal`/`TAU`),
6. das neue `mainImage` (unten).

```glsl
// ---- gemeinsame Helfer (jede Funktion existiert genau EINMAL) --------------
// Ersetzt die Helfer-Bloecke BEIDER Skelette.

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float TAU = 6.28318530;
const float PI  = 3.14159265;

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash13(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

vec3 hash33(vec3 p)
{
    return fract(sin(vec3(dot(p, vec3(127.1, 311.7,  74.7)),
                          dot(p, vec3(269.5, 183.3, 246.1)),
                          dot(p, vec3(113.5, 271.9, 124.6)))) * 43758.5453);
}

float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(i),              hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)), hash21(i + vec2(1, 1)), u.x), u.y);
}

float fbm(vec2 p)   // ENTSCHEID: die 4-Oktaven-Fassung fuer ALLE Welten (s. Text)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

vec3 pal(float t) { return 0.5 + 0.5 * cos(TAU * (t + vec3(0.0, 0.33, 0.67))); }

mat3 rotAchse(vec3 a, float w)
{
    float c = cos(w), s = sin(w), k = 1.0 - c;
    return mat3(a.x * a.x * k + c,       a.y * a.x * k + a.z * s,  a.z * a.x * k - a.y * s,
                a.x * a.y * k - a.z * s, a.y * a.y * k + c,        a.z * a.y * k + a.x * s,
                a.x * a.z * k + a.y * s, a.y * a.z * k - a.x * s,  a.z * a.z * k + c);
}

float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}
```

```glsl
// NEU: mainImage - Split-Screen als Kollisions-Beweis.
// Links faehrt der Tunnel, rechts treibt das Truemmerfeld - aus EINER Datei.
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 col;
    if (uv.x < -0.01) {                          // links: Welt 1, der Tunnel
        vec2 tuv = uv + vec2(0.45, 0.0);
        vec3 ro = vec3(0.0, 0.0, iTime * 1.5);
        vec3 rd = normalize(vec3(tuv, 1.4));

        float t = tunnelMarch(ro, rd);
        vec3 p = ro + rd * t;
        float w = atan(p.y, p.x);

        col = tunnelShade(p, tunnelNormale(p), ro, w);
        col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));
        float F = tunnelFenster(w, p.z) * exp(-0.001 * t * t);
        col = mix(col, vec3(0.02, 0.03, 0.07), F);
        col = 1.0 - exp(-col * 1.6);
    } else if (uv.x > 0.01) {                    // rechts: Welt 2, das Debris-Feld
        vec2 duv = uv - vec2(0.45, 0.0);
        vec3 ro = vec3(sin(iTime * 0.04) * 5.0, 1.5, iTime * 0.6);
        vec3 rd = normalize(vec3(duv, 1.4));
        rd.yz *= R(-0.15);

        col = debrisWelt(ro, rd);
        col = 1.0 - exp(-col * 1.3);
    } else {
        col = vec3(0.10);                        // Trennbalken
    }

    fragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
```

**Ergebnis:** Ein geteiltes Bild – links der Neon-Tunnel in voller Fahrt, rechts das taumelnde Trümmerfeld über dem Glutplaneten. Beide Welten leben vollständig in einer Datei, keine stört die andere. Unspektakulär anzusehen – aber dieser Schritt entscheidet, ob der Rest des Tutorials Handwerk oder Schutträumen wird.

### Was passiert hier – die Kollisions-Fallen, ehrlich

Wer die zwei **Original**-Gesamtlistings statt der Skelette zusammengeworfen hätte, wäre in genau diese Fallen gelaufen – es lohnt, sie beim Namen zu nennen:

1. **Harte Kollisionen (der Compiler schreit):** Beide Originale definieren `hash21`, `vnoise`, `fbm`, `TAU`, eine Funktion `kamera` und ein `mainImage`. Doppelte Definition = Kompilierfehler. Unangenehm, aber harmlos – der Fehler ist laut.
2. **Die leise Falle – gleicher Name, andere Bedeutung:** Der Tunnel-`fbm` hat **4 Oktaven**, der Debris-`fbm` **5**. Wer beim Aufräumen wahllos eine der beiden Fassungen behält, ändert das Aussehen der *anderen* Welt – subtil, ohne Fehlermeldung. Unser Entscheid: **eine** Fassung mit 4 Oktaven für alle (der Planet verliert dadurch ein Quäntchen Feinstruktur – im Fensterformat unsichtbar, und eine Oktave weniger ist im teuersten Codepfad sogar willkommen). Wichtig ist nicht, *welche* Fassung gewinnt, sondern dass es ein **dokumentierter Entscheid** ist statt eines Unfalls.
3. **Die semantische Falle – Stellschrauben-Namen:** `DICHTE` heißt bei Space Debris „Anteil belegter Zellen", bei Crystal Lights „Absorption im Kristall" – dasselbe Wort, zwei physikalisch völlig verschiedene Größen (und ab Schritt 9 sind **beide** im Shader!). Deshalb die Präfix-Disziplin auch bei Konstanten: `T_DICHTE` (Fensteranteil), `D_DICHTE` (Zellbelegung), `K_DICHTE` (Absorption). Der Präfix ist billiger als der Debugging-Abend.

Die **Präfix-Regel** dieses Tutorials: Ein Präfix (`tunnel…`, `debris…`, `kristall…`, `T_`, `D_`, `K_`, `P_`) bekommt alles, was es **konzeptionell in mehr als einer Welt gibt** – map, march, shade, Normale, Stellschrauben. Eindeutige Eigennamen (`planetHit`, `atmosphaere`, `rotAchse`) dürfen bleiben, solange keine zweite Welt denselben Begriff beansprucht. Und die gemeinsamen Helfer sind bewusst **präfixfrei**: Sie gehören niemandem – wer `hash21` anfasst, muss wissen, dass er *drei* Welten gleichzeitig anfasst.

💡 **Warum Split-Screen statt gleich Portal?** Die alte Regel der Serie: **eine Fehlerquelle zur Zeit.** Nach dem Merge gibt es zwei Sorten möglicher Fehler – „die Welten kollidieren im Namensraum" und „die Portal-Logik ist falsch". Der Split-Screen isoliert die erste Sorte vollständig: Sieht jede Hälfte aus wie ihr Skelett, ist der Namensraum sauber, und jeder Fehler ab Schritt 4 ist ein Portal-Fehler. (Es ist dasselbe Kalkül wie das Schachbrett in Crystal Lights Schritt 2 und die nackten Lampen in dessen Schritt 9.)

### 🎨 Experimentieren

- Den Trennbalken verschieben (`-0.01/0.01` → `±0.3`): mehr Tunnel, weniger Debris – praktisch beim Feintunen einer Seite
- Absichtlich die 5-Oktaven-`fbm` einsetzen und NUR die rechte Seite beobachten: der Planet wird körniger. Die leise Falle einmal *gesehen* zu haben impft besser als jede Warnung
- Beide Seiten mit derselben Kamera füttern (`ro`/`rd` der linken auch rechts): ein erster Vorgeschmack darauf, wie fremd sich die Welten sind, solange nichts sie verbindet

🧠 **Merke:** Namespacing ist keine Schönheitsregel, sondern die Antwort auf drei Fallentypen: laute Duplikate, leise Bedeutungs-Drift bei gleichem Namen, und Stellschrauben-Homonyme. Präfixe für alles Mehrdeutige, geteilte Helfer genau einmal und als Gemeingut markiert – dann ist ein Merge Addition statt Chemie.

---

## Schritt 4 – Das Portal: der Fensterstrahl wechselt die Welt

**Neu:** Das Herzstück. Der Split-Screen fällt, der Tunnel wird wieder Vollbild – und ein Strahl, der eine Fensteröffnung trifft, wird **nicht** weitergemarcht und nicht angemalt, sondern als neuer Strahl in die Debris-Welt **übergeben**. Nur Portal-Pixel bezahlen den zweiten Marsch.

*Ab jetzt zeigen die Schritte nur noch die geänderten bzw. neuen Funktionen – alles andere bleibt wörtlich wie im vorherigen Schritt stehen. (Am Ende von Schritt 12 steht der komplette Shader noch einmal am Stück.)*

```glsl
// NEU: globaler Portal-Austrittspunkt - fuer die Blase in debrisMap (s. unten)
vec3 gAuge = vec3(0.0);

// GEAENDERT: debrisWelt setzt die Blase auf ihren Strahl-Ursprung
vec3 debrisWelt(vec3 ro, vec3 rd)
{
    gAuge = ro;
    // ... Rest woertlich wie in Schritt 2/3 ...
}

// GEAENDERT: debrisMap - die Kamera-Blase des Originals kehrt zurueck,
// jetzt als PORTAL-Blase (nach der gr-Zeile einfuegen):
    float gr = D_GROESSE * (0.35 + 0.65 * hash13(id + 3.1));

    // Portal-Blase: Zellen direkt am Austrittspunkt schrumpfen weg -
    // sonst kleben halbe Brocken "am Glas" (Space Debris, Schritt 13)
    vec3 zentrum = (id + 0.5) * D_ZELLE;
    gr *= smoothstep(1.0, 3.5, length(zentrum - gAuge));
    if (gr < 0.02) return sicher;
```

```glsl
// GEAENDERT: mainImage - Vollbild-Tunnel, und der Platzhalter wird zum PORTAL
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 1.5);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = tunnelMarch(ro, rd);
    vec3 p = ro + rd * t;
    float w = atan(p.y, p.x);

    vec3 col = tunnelShade(p, tunnelNormale(p), ro, w);
    col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    // DAS PORTAL: statt Platzhalter-Farbe wird der Strahl UEBERGEBEN
    float F = tunnelFenster(w, p.z) * exp(-0.001 * t * t);
    if (F > 0.004) {
        vec3 aussen = debrisWelt(p, rd);     // Welt 2: Ursprung = Fensterpunkt,
        col = mix(col, aussen, F);           // Richtung unveraendert
    }

    col = 1.0 - exp(-col * 1.5);             // EINE Belichtung fuer beide Welten
    fragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
```

**Ergebnis:** Der Moment, in dem das Composite sein Versprechen einlöst: Durch die Fenster des Tunnels sieht man **die echte Trümmerwelt** – Brocken taumeln vorbei, darunter glüht der Planet, am Fensterrand schiebt sich beim Vorbeiflug die Perspektive korrekt mit. Kein gemaltes Bild, keine Textur: Es ist dieselbe Welt aus Schritt 2, betrachtet durch ein Loch in der Wand.

### Was passiert hier – die Portal-Technik

Drei Zeilen tragen den ganzen Zauber, und jede hat eine Begründung:

**`debrisWelt(p, rd)` – der Ursprungswechsel.** Der Primärstrahl endet am Wandpunkt `p`. Genau dort beginnt der Sekundärstrahl: gleicher Ort, gleiche Richtung, aber **eine andere map**. Das ist die präzise Definition eines Portals im Raymarching: *Der Strahl behält seine Geometrie und wechselt seine Welt.* Weil `rd` unverändert weiterläuft, stimmt die Perspektive automatisch – Fensterkanten beschneiden die Außenwelt exakt so, wie ein echtes Loch es täte, inklusive Parallaxe beim Vorbeifahren. Es gibt keinen „Skybox-Betrug", den man erwischen könnte.

**`if (F > 0.004)` – die Bezahlschranke.** Der zweite Marsch (60 × `debrisMap`) läuft nur, wenn das Pixel tatsächlich nennenswert Fensterfläche zeigt. Wandpixel kosten wie bisher ~90 `tunnelMap`-Auswertungen; nur die typischerweise 10–20 % Fensterpixel zahlen den Aufpreis. Das Worst-Case-Budget pro Pixel liegt bei grob 90 + 60 = 150 map-Auswertungen – dank der gesenkten Skelett-Budgets (Schritt 1/2) ist das *weniger* als eines der Originale allein. **Das ist der Performance-Satz der Portal-Technik: Zwei Welten kosten nicht zweimal – sie kosten einmal plus Portalfläche.**

**Die Portal-Blase – der ehrliche Fix.** Ohne die `gAuge`-Zeilen zeigt sich sofort ein Artefakt: Das Zellgitter der Debris-Welt füllt ja *den ganzen Raum*, auch die Gegend direkt hinter der Wand – einzelne Brocken „kleben am Glas" oder werden vom Fensterrand aufgeschnitten. Das Original hatte für das gleiche Problem (Brocken kollidieren mit der Kamera) die **Kamera-Blase**: Zellen nahe am Auge schrumpfen weich weg. Wir recyceln sie wörtlich – nur ist das „Auge" jetzt der Portal-Austrittspunkt. *(Kleine Ehrlichkeit: `gAuge` ist der Wandpunkt des jeweiligen Pixels und variiert daher minimal über die Fensterfläche – die Blase „atmet" ein paar Millimeter mit dem Blick. Da sie ein weicher smoothstep über Meter ist, ist das unsichtbar.)*

💡 **Warum wird der Strahl nicht einfach „weitergemarcht"?** Weil es kein gemeinsames `map` gibt, in dem beide Welten stehen – und das ist Absicht. Man *könnte* `min(tunnelMap, debrisMap)` bauen und alles in einem Marsch erschlagen; dann wäre die Debris-Welt aber überall, auch vor der Linse im Tunnelinneren, und jede Welt-Transformation (Schritt 5!) müsste in die gemeinsame map hineingefaltet werden. Das Portal hält die Welten **getrennt und nacheinander**: erst Wirt, dann – nur wo erlaubt – Gast. Genau diese Trennung macht den nächsten Schritt (eigener Maßstab der Außenwelt) zur Drei-Zeilen-Übung statt zur Operation am offenen Herzen.

### 🎨 Experimentieren

- `F > 0.004` → `F > 0.5`: die Bezahlschranke wird sichtbar – weiche Fensterränder schnappen hart um. Der Schwellwert ist bewusst winzig: Er spart Arbeit, ohne je das Bild zu ändern
- Die Blase abschalten (`gr *= 1.0`): das Am-Glas-kleben-Artefakt einmal absichtlich ansehen
- `debrisWelt(p, rd)` → `debrisWelt(p, reflect(rd, tunnelNormale(p)))`: aus Fenstern werden **Spiegel** in die Außenwelt – dieselbe Technik, andere Erzählung
- Vortrieb `1.5` → `4.0`: die Außenwelt rauscht vorbei; man spürt, dass sie *dieselbe* z-Achse teilt (und ahnt, warum Schritt 5 ihr einen eigenen Rahmen gibt)

🧠 **Merke:** Ein Portal ist eine Strahl-Übergabe: Treffer prüfen → NICHT shaden → Strahl mit (transformiertem) Ursprung in die zweite Welt schicken → Ergebnis mit der Portalmaske einblenden. Nur Portal-Pixel bezahlen die zweite Welt – deshalb gehört vor jede Übergabe eine Bezahlschranke.

---
## Schritt 5 – Maßstab & Weltrahmen: das Fenster als Diorama

**Neu:** Die Außenwelt bekommt einen **eigenen Koordinatenrahmen** – einen Ursprungs-Versatz und einen Maßstab. Mit `P_MASSSTAB > 1` wird der Blick durchs Fenster zum Blick in eine **andere Größenordnung**: draußen liegt eine verkleinerte „Diorama"-Welt, von der ein Fenster gleich einen ganzen Landstrich zeigt.

```glsl
// ---- STELLSCHRAUBEN: PORTAL (neu) ------------------------------------------
const float P_MASSSTAB = 4.0;    // 1 = Aussenwelt in Weltgroesse, >1 = Diorama
const vec3  D_URSPRUNG = vec3(7.0, 2.0, 13.0);   // Lage der Aussenwelt
// ----------------------------------------------------------------------------

// GEAENDERT: die Uebergabe in mainImage - der Strahl wird in den Rahmen
// der Aussenwelt TRANSFORMIERT statt roh weitergereicht
    if (F > 0.004) {
        vec3 roD = D_URSPRUNG + p * P_MASSSTAB;   // Uebergabe: Ort skaliert+versetzt
        vec3 aussen = debrisWelt(roD, rd);        // Richtung bleibt (uniforme Skala)
        col = mix(col, aussen, F);
    }
```

**Ergebnis:** Derselbe Tunnel, aber die Fenster zeigen jetzt **Übersicht statt Ausschnitt**: ganze Trümmerschwärme ziehen vorbei, der Planet krümmt sich sichtbar unter ihnen, und beim Vorbeiflug strömt die Außenwelt viermal schneller als die Wand – das vertraute Gefühl, aus einem Zugfenster auf eine weit entfernte Landschaft zu schauen, nur umgekehrt: Die Welt draußen ist *klein*.

### Was passiert hier – Koordinatenrahmen und die Mathematik dahinter

**Die Transformation ist bewusst minimal:** `roD = D_URSPRUNG + p·P_MASSSTAB`. Ein Versatz, eine uniforme Skalierung, keine Rotation. Der Versatz `D_URSPRUNG` entscheidet, *welcher Teil* der (unendlichen) Debris-Welt vor den Fenstern liegt – er ist reine Szenenwahl, wie der Bildausschnitt eines Fotografen. Die Skalierung ist der interessante Teil:

- **Richtungen bleiben unverändert.** Eine uniforme Skalierung ändert Längen, aber keine Winkel – `rd` kann unverändert weiterlaufen. (Bei nicht-uniformer Skala oder Rotation müsste `rd` mittransformiert werden; wir bleiben absichtlich beim einfachsten Fall.)
- **Was heißt Maßstab 4 konkret?** Das Fenster ist 0.6 × 1.7 Tunnel-Einheiten groß. Mit `P_MASSSTAB = 4` entsprechen dem in der Außenwelt 2.4 × 6.8 Debris-Einheiten – das Fenster überstreicht also gut zwei Zellspalten (`D_ZELLE = 3`) statt einem Bruchteil. Und die Marsch-Reichweite von 50 Debris-Einheiten entspricht nur 12.5 Tunnel-Einheiten „gefühlter" Tiefe: Die Außenwelt ist ein tiefes, aber kompaktes Diorama.
- **Der Dunst der Außenwelt rechnet in Debris-Einheiten** – und das ist richtig so. Eine verkleinerte Welt hat ihre eigene Luftperspektive; würde man den Dunst in Tunnel-Einheiten rechnen, sähe das Diorama glasklar bis zum Horizont aus und verlöre genau die Tiefenstaffelung, die es „echt" macht. (Deshalb wurde der Dunst in Schritt 2 zur Welt geschlagen, nicht zur Politur.)
- **Die Parallaxe multipliziert sich:** Die Kamera fährt mit 1.5 Einheiten/s, die Außenwelt zieht also mit 6 Debris-Einheiten/s vorbei. Das verkauft „Stratosphären-Tempo" gratis – und ist zugleich der erste Kohärenz-Moment: Innen- und Außenwelt hängen an **derselben** Kamerabewegung, nur durch den Maßstab übersetzt.

💡 **1:1-Welt oder Diorama – wann was?** `P_MASSSTAB = 1` (plus Versatz) erzählt „der Tunnel fliegt WIRKLICH durch dieses Trümmerfeld" – maximale Immersion, aber die Fenster zeigen nur Ausschnitte, und man braucht Glück, dass gerade etwas Interessantes vorbeikommt. `P_MASSSTAB = 4..8` erzählt „die Fenster sind Schaukästen in eine andere Größenordnung" – jedes Fenster ist garantiert gefüllt (Planet + Schwärme), dafür ist der Trick für aufmerksame Augen als Trick lesbar. Für einen Visualizer gewinnt fast immer das Diorama: Es garantiert Bildinhalt. Die Stellschraube macht die Entscheidung jederzeit revidierbar.

### 🎨 Experimentieren

- `P_MASSSTAB` durchfahren: `1.0` (Echtwelt), `2.0`, `8.0` (Mikro-Diorama – die Brocken werden zu Staub, der Planet zum Bühnenbild)
- `D_URSPRUNG.y = -6.0`: die Fenster schauen von *unterhalb* des Feldes auf die Trümmer – dramatische Untersicht mit viel Glut
- Das „Galerie"-Experiment: `roD += (hash21(floor(vec2(w * 3.0, p.z / T_ABSTAND))) - 0.5) * 40.0;` – jedes Fenster bekommt per Hash einen eigenen Versatz und zeigt einen **anderen** Teil der Außenwelt. Sofort interessanter Flickenteppich – und sofort weniger glaubwürdig: Die Fenster erzählen keine gemeinsame Welt mehr. Genau dieser Zielkonflikt ist die Kohärenz-Lektion von Schritt 11 im Kleinen
- `rd` beim Übergang leicht brechen: `debrisWelt(roD, normalize(rd + n * 0.1))` – die Fenster bekommen „dickes Glas"

🧠 **Merke:** Ein Portal-Übergang ist eine Transformation `(ro, rd) → (ro', rd')` zwischen zwei Koordinatenrahmen. Versatz wählt die Szene, uniformer Maßstab wählt die Größenordnung (und lässt `rd` in Ruhe), und alles hinter dem Portal – auch Dunst und Distanzen – lebt konsequent in den Einheiten der Zielwelt.

---

## Schritt 6 – Portal-Politur: atmende Öffnung und Neon-Rahmen

**Neu:** Die Fenster werden zu **Portalen** auch im Auftritt: Die Maske wird auf eine signierte **Kanten-Distanz** umgebaut (die brauchen wir in Schritt 10 sowieso), die Öffnungen **atmen** langsam, und um jede Öffnung läuft ein pulsierender **Neon-Rahmen**.

```glsl
// ---- STELLSCHRAUBEN: PORTAL (erweitert) ------------------------------------
const float P_ATMEN  = 0.25;   // Fenster oeffnen/schliessen sich (0 = statisch)
const float P_RAHMEN = 0.06;   // Breite des Neon-Rahmens um jedes Portal
// ----------------------------------------------------------------------------

// NEU (ersetzt tunnelFenster): signierte Kanten-Distanz in Weltmass.
// < 0 = im Fenster, > 0 = auf der Wand; id = Fensterzelle (fuer Rahmen & Co.)
float fensterDist(float w, float z, out vec2 id)
{
    vec2 zelle = vec2(fract(w / TAU + 0.5) * T_SPALTEN, z / T_ABSTAND);
    id = floor(zelle);
    if (hash21(id + 3.1) > T_DICHTE) return 1e3;   // keine Oeffnung in dieser Zelle

    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * T_RADIUS / T_SPALTEN;
    c.y *= T_ABSTAND;

    // Atmen: jede Oeffnung pumpt mit eigener Phase und eigenem Tempo
    float o = clamp(0.8 + P_ATMEN * sin(iTime * (0.15 + 0.25 * hash21(id + 9.4))
                                        + TAU * hash21(id)), 0.0, 1.0);
    vec2 halb = vec2(0.30, 0.85) * o;
    return max(abs(c.x) - halb.x, abs(c.y) - halb.y);
}

// NEU: pulsierender Emissions-Rahmen entlang der Portal-Kante
vec3 portalRahmen(float d, vec2 id)
{
    float band = smoothstep(P_RAHMEN, 0.0, abs(d));
    float puls = 0.6 + 0.4 * sin(iTime * (0.8 + 0.6 * hash21(id + 6.6))
                                 + TAU * hash21(id + 2.2));
    return band * puls * pal(hash21(id + 8.5) * 0.3 + 0.60) * 0.9;
}
```

```glsl
// GEAENDERT: der Portal-Block in mainImage arbeitet jetzt auf der Distanz
    vec2 fid;
    float fd = fensterDist(w, p.z, fid);
    float F  = smoothstep(0.05, -0.05, fd) * exp(-0.001 * t * t);
    if (F > 0.004) {
        vec3 roD = D_URSPRUNG + p * P_MASSSTAB;
        col = mix(col, debrisWelt(roD, rd), F);
    }
    col += portalRahmen(fd, fid) * exp(-0.0008 * t * t);
```

**Ergebnis:** Die Öffnungen wachsen und schrumpfen in Zeitlupe, jede in eigenem Rhythmus – und um jede läuft ein schmaler, pulsierender Neonsaum, der das Loch als *gebautes* Portal ausweist. In der Tiefe des Tunnels reihen sich die leuchtenden Rahmen zu einer Flucht.

### Was passiert hier

**Von der Maske zur Distanz** ist mehr als Kosmetik. `tunnelFenster` beantwortete nur „wie viel Fenster ist hier?" (0..1). `fensterDist` beantwortet „**wie weit** ist es zur Kante?" – eine signierte Distanz im Bogenmaß-korrigierten Weltmaß (die `c.x`-Skalierung rechnet den Winkel in echte Wandmeter um, wie im Quell-Tutorial hergeleitet). Aus der Distanz fallen drei Dinge heraus: die alte Maske (`smoothstep` um 0), der Rahmen (`abs(d)` klein = nahe der Kante, egal auf welcher Seite) und in Schritt 10 die pixelgenaue Antialiasing-Kante. **Eine Distanz ist die Mutter vieler Masken** – wo immer eine Kante mehrere Abnehmer hat, lohnt der Umbau.

**Das Atmen** sitzt im Öffnungsfaktor `o`, der die Halbmaße skaliert – Grundöffnung 0.8, Hub `P_ATMEN`. Zwei Details nach dem Muster der Serie: Jede Öffnung hat per `hash21(id …)` eigene Phase und eigenes Tempo (kein Synchron-Ballett), und der `clamp` hält `o` in [0, 1] – bei `o = 0` ist das Portal *zu*, `fensterDist` wird überall positiv, und F samt zweitem Marsch verschwinden von selbst. Das Atmen ist damit zugleich eine dynamische Kostenbremse – und in Anhang A der Ansatzpunkt für den Beat („der Bass reißt die Portale auf").

**Der Rahmen** ist gestaltender Ehrlichkeits-Trick: Ein nacktes Loch, hinter dem eine anders skalierte Welt liegt, wirkt wie ein Renderfehler. Ein *gerahmtes* Loch wirkt wie Absicht – Architektur. Der Rahmen erklärt dem Auge die Naht, statt sie zu verstecken. Technisch ist er reine Emission (`+=` nach allem Shading, wie das Neon), mit `exp(-t²)`-Dämpfung, damit ferne Rahmen im selben Dunst versinken wie die Wand.

### 🎨 Experimentieren

- `P_ATMEN = 1.0`: Vollhub – Portale schließen sich komplett und reißen wieder auf; der Tunnel „blinzelt"
- `P_RAHMEN = 0.15` und `puls`-Term auf `1.0`: fette, ruhige Rahmen – eher Raumstation als Cyberpunk
- Rahmenfarbe an die Außenwelt koppeln: `pal(...)` → `mix(pal(...), vec3(1.0, 0.5, 0.15), 0.5)` – der Rahmen nimmt das Planet-Orange auf (Vorgriff auf die Motivkopplung in Schritt 11)
- Nur-Rahmen-Modus: `F = 0.0;` erzwingen – die Portale zeigen nichts, aber die Rahmen pulsieren weiter: der Tunnel als Installation

🧠 **Merke:** Kanten-Features (Maske, Rahmen, Anti-Aliasing) wollen alle dieselbe Information – die signierte Distanz zur Kante. Einmal sauber berechnet, kostet jeder weitere Abnehmer nur noch ein `smoothstep`. Und: Eine sichtbare Naht, die man nicht verstecken kann, rahmt man.

---
## Schritt 7 – Material-Id: map() lernt zwei Antworten

**Neu:** Die zweite Kerntechnik des Composites. `tunnelMap` liefert statt einer Distanz ein Paar `vec2(Distanz, Material-Id)` – und der Boden der Röhre wird auf Streckenabschnitten durch eine (vorerst flache) zweite Fläche ersetzt, die das Shading später als Kristall behandelt.

```glsl
// ---- STELLSCHRAUBEN: KRISTALL (neu) ----------------------------------------
const float K_PERIODE = 24.0;   // Streckenperiode der Bodenabschnitte (z)
const float K_ANTEIL  = 0.45;   // Anteil der Strecke mit Kristallboden
const float K_HOEHE   = 0.72;   // Bodenniveau unter der Tunnelachse
// ----------------------------------------------------------------------------

// NEU: 0..1 - wo entlang der Strecke der Kristallboden liegt
float kristallZone(float z)
{
    float zz = fract(z / K_PERIODE);
    return smoothstep(0.03, 0.12, zz) * (1.0 - smoothstep(K_ANTEIL - 0.09, K_ANTEIL, zz));
}

// GEAENDERT: map liefert (Distanz, Material-Id): 1 = Roehrenwand, 2 = Boden.
// Der Boden ist VORERST eine flache Platte - erst die Geometrie, dann das Material.
vec2 tunnelMap(vec3 p)
{
    float w = atan(p.y, p.x);
    float dWand = T_RADIUS - T_TIEFE * (0.5 - 0.5 * cos(w * T_ROEHREN)) - length(p.xy);

    // Lueckentrick von Crystal Lights: ausserhalb der Zone versinkt der Boden
    float boden  = mix(-(T_RADIUS + 0.8), -K_HOEHE, kristallZone(p.z));
    float dBoden = p.y - boden;

    return (dWand < dBoden) ? vec2(dWand, 1.0) : vec2(dBoden, 2.0);
}

// GEAENDERT: der Marsch reicht die Id des Treffers heraus
vec2 tunnelMarch(vec3 ro, vec3 rd)
{
    float t = 0.02, id = 1.0;
    for (int i = 0; i < 90; i++) {
        vec2 dm = tunnelMap(ro + rd * t);
        id = dm.y;
        if (dm.x < 0.0015 + 0.001 * t) break;
        t += dm.x * 0.7;
        if (t > 40.0) break;
    }
    return vec2(min(t, 40.0), id);
}

// GEAENDERT: die Normale liest nur noch die Distanz-Komponente
vec3 tunnelNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(tunnelMap(p + e.xyy).x - tunnelMap(p - e.xyy).x,
                          tunnelMap(p + e.yxy).x - tunnelMap(p - e.yxy).x,
                          tunnelMap(p + e.yyx).x - tunnelMap(p - e.yyx).x));
}
```

```glsl
// GEAENDERT: mainImage - Dispatch ueber die Id (Debug-Farbe als Platzhalter),
// und das Portal gilt NUR auf der Wand
    vec2 hit = tunnelMarch(ro, rd);
    float t = hit.x;
    vec3 p = ro + rd * t;
    float w = atan(p.y, p.x);

    vec3 col;
    if (hit.y > 1.5) col = vec3(0.10, 0.35, 0.35);          // Debug: Boden-Material
    else             col = tunnelShade(p, tunnelNormale(p), ro, w);

    col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    if (hit.y < 1.5) {
        // ... Portal-Block und Rahmen woertlich wie in Schritt 6 ...
    }
```

**Ergebnis:** Der Tunnel bekommt einen Rhythmus: Auf knapp der halben Strecke schiebt sich eine türkise, flache Bodenplatte in die Röhre, verschwindet wieder, kommt wieder – die Debug-Farbe macht den Material-Dispatch sichtbar, bevor irgendein Material existiert.

### Was passiert hier – der (Distanz, Id)-Vertrag

**Die Idee ist ein Rückgabewert mit Beipackzettel.** Ein Raymarcher fragt die map hundertmal „wie weit?" und genau einmal – am Treffer – „**was** habe ich getroffen?". Das `vec2` beantwortet beide Fragen gleichzeitig: `min()` wird zum Vergleich mit Gedächtnis (wer näher ist, gibt Distanz *und* Namen), der Marsch reicht die Id des letzten Samples heraus, und das Shading dispatcht darüber. Der Marsch selbst bleibt wörtlich derselbe – er sortiert nur nebenbei, *wessen* Oberfläche er gefunden hat. Diese Technik skaliert: Ein dritter Werkstoff wäre eine weitere Distanz-Id-Zeile, kein neuer Marsch.

**Der Lücken-Trick, zweite Verwendung.** `mix(-(T_RADIUS + 0.8), -K_HOEHE, zone)` ist wörtlich das Manöver aus Crystal Lights Schritt 8: Statt den Boden per Sonderfall an- und abzuschalten (zweiter Codepfad, Kantenprobleme), **versinkt** er außerhalb der Zone einfach 0.8 Einheiten unter die Röhrenwand – dort gewinnt immer `dWand`, und der Boden ist geometrisch weg. Die Zonenränder werden dadurch zu sichtbaren Absenkkanten: Der Kristallboden *taucht* aus dem Röhrenboden auf und wieder ab, wie eine Vereisung, die die Strecke abschnittsweise überzieht.

**Warum das Portal jetzt einen Wächter braucht:** Das Fensterraster lebt auf der ganzen abgerollten Wand – auch auf den unteren Winkeln, die jetzt vom Boden verdeckt werden. Träfe der Strahl den Boden und würde trotzdem `fensterDist` befragt, könnte ein „Fenster unter dem Boden" die Kristallfläche mit Außenwelt übermalen. Der `if (hit.y < 1.5)`-Wächter formuliert die Regel des Bauplans: **Portale gibt es nur auf der Wand.** Material-Id und Portal-Maske sind zwei orthogonale Entscheidungen – die Id entscheidet zuerst.

💡 **Rechenprobe zur Geometrie:** Der Boden bei `y = −0.72` schneidet den Einheitskreis der Röhre bei `x = ±√(1 − 0.72²) ≈ ±0.69` – eine Bodenbreite von ~1.39 Einheiten, gut ein Drittel des Rohrdurchmessers. Breit genug für erkennbares Terrain (Schritt 8 legt Platten von ~0.33 Größe darauf), schmal genug, dass die Neonwand dominant bleibt.

### 🎨 Experimentieren

- `K_ANTEIL = 0.9`: fast durchgehender Boden – der Tunnel wird zur Eisgrotte; `0.15`: seltene Kristall-Inseln
- `K_HOEHE = 0.4`: der Boden rückt zur Bildmitte, die Röhre wird zum Halbrohr
- Debug-Disziplin: `col = vec3(hit.y - 1.0);` – Id als Grauwert; die Grenzlinie Wand/Boden muss eine saubere Kurve sein, kein Flimmersaum
- Die Zone an die Fenster koppeln: `kristallZone(p.z) * (0.3 + 0.7 * T_DICHTE)` – nur ein Vorgeschmack darauf, dass alle Felder derselben Karte gehorchen

🧠 **Merke:** `map → vec2(d, id)` ist der Standard-Vertrag für Mehr-Material-Raymarcher: gleiche Marsch-Schleife, `min()` mit Gedächtnis, Dispatch erst am Treffer. Und Flächen schaltet man nicht ab – man lässt sie unter die konkurrierende Geometrie versinken.

---

## Schritt 8 – Der Kristallboden: ein Höhenfeld im min()

**Neu:** Die flache Debug-Platte wird zum kondensierten Crystal-Lights-Terrain – FBM-Grundwellen plus Voronoi-Platten, transformiert in den Tunnelrahmen. Und damit steht die ehrliche Frage des Schritts: Darf man ein **Höhenfeld** (kein SDF!) einfach per `min()` in einen SDF-Marsch mischen?

```glsl
// NEU: 2D-Hash mit 2D-Ergebnis + Voronoi - kondensiert aus Crystal Lights.
// (Beides zu den GEMEINSAMEN Helfern legen - kuenftiges Gemeingut.)
vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

vec3 voronoi(vec2 p)   // (Abstand^2, Zell-Id)
{
    vec2 i = floor(p), f = fract(p);
    float best = 8.0;
    vec2 bestId = vec2(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 g = vec2(float(x), float(y));
        vec2 r = g + hash22(i + g) - f;
        float d = dot(r, r);
        if (d < best) { best = d; bestId = i + g; }
    }
    return vec3(best, bestId);
}

// NEU: das Kristall-Hoehenfeld im Tunnelrahmen - Grundwellen + Plattenversatz
float kristallHoehe(vec2 q)
{
    float glatt  = (fbm(q * 1.1) - 0.5) * 0.16;
    float platte = (hash21(voronoi(q * 3.0).yz) - 0.5) * 0.10;
    return glatt + platte;
}

// NEU: die Bodenhoehe als Funktion des Ortes (ersetzt die mix-Zeile in tunnelMap)
float bodenHoehe(vec3 p)
{
    float ziel = -K_HOEHE + kristallHoehe(p.xz);
    return mix(-(T_RADIUS + 0.8), ziel, kristallZone(p.z));
}

// GEAENDERT: tunnelMap - das Hoehenfeld kommt KONSERVATIV in den min-Verbund
vec2 tunnelMap(vec3 p)
{
    float w = atan(p.y, p.x);
    float dWand = T_RADIUS - T_TIEFE * (0.5 - 0.5 * cos(w * T_ROEHREN)) - length(p.xy);

    // Boden: oben reicht eine billige Schranke, erst unten zahlt das Hoehenfeld
    float dBoden = (p.y > -0.30) ? (p.y + 0.55)
                                 : (p.y - bodenHoehe(p)) * 0.5;

    return (dWand < dBoden) ? vec2(dWand, 1.0) : vec2(dBoden, 2.0);
}
```

**Ergebnis:** Der Bodenabschnitt ist keine Platte mehr, sondern ein fein zerklüftetes Feld aus gegeneinander versetzten Mini-Schollen mit weichen Bodenwellen darunter – noch in Debug-Türkis, aber die Geometrie ist unverkennbar Crystal Lights, auf Röhrenbreite geschrumpft.

### Was passiert hier – Höhenfeld und SDF im selben Marsch, ehrlich diskutiert

Hier mischen sich zwei Marsch-Philosophien, die die Quell-Tutorials getrennt gelehrt haben – und die Mischung ist nur mit zwei Zugeständnissen korrekt:

**Das Problem:** `p.y − bodenHoehe(p)` ist der **vertikale** Abstand zum Terrain, kein Sicherheitsabstand in Strahlrichtung. Crystal Lights löste das mit einer *Marsch-Drossel* (nur 40 % des Abstands gehen). Aber unsere Schleife drosselt global mit 0.7 – gut für die Wand (echtes SDF), zu forsch für ein Höhenfeld mit senkrechten Plattenkanten. Ein Strahl könnte flach über eine Scholle streichen und *hinter* ihrer Kante wieder auftauchen: Löcher.

**Zugeständnis 1 – die konservative Distanz:** `(p.y − bodenHoehe(p)) * 0.5`. Statt die ganze Schleife zu bremsen, meldet nur der Boden die Hälfte seines Abstands. Im `min()`-Verbund wirkt das exakt wie eine lokale Drossel: In Bodennähe bestimmt `dBoden` das Tempo (effektiv 0.5 · 0.7 = 0.35 – vorsichtiger als Crystal Lights' 0.4), in Wandnähe marschiert die Wand ungebremst. **Die Regel: Wer im min()-Verbund lügt (Höhenfeld), muss untertreiben.** Ein zu großer Wert reißt Löcher; ein zu kleiner kostet nur Iterationen.

**Zugeständnis 2 – die billige Schranke:** Ohne die `p.y > −0.30`-Weiche würde *jede* der ~90 map-Auswertungen `fbm` (16 Hashes) plus `voronoi` (9 Hashes) bezahlen – auch für Strahlen, die nur die Decke sehen. Die Weiche ersetzt das Höhenfeld oberhalb von `y = −0.30` durch die Ebenen-Distanz `p.y + 0.55` zur höchsten möglichen Bodenkante (Rechenprobe: Boden maximal bei `−0.72 + 0.13 = −0.59`; die Schranke `p.y + 0.55` bleibt also stets ≤ der wahren Distanz – konservativ). Wichtig ist, dass die Schranke **im min() bleibt** statt den Boden ganz zu verschweigen: Sie bremst absteigende Strahlen rechtzeitig ab, sodass keiner per Wand-Großschritt durch den Boden tunnelt.

**Warum überhaupt mischen statt zwei Marches?** Man könnte den Boden separat marchen (wie das Portal die Debris-Welt). Aber Portal und Material-Mix sind verschiedene Beziehungen: Die Debris-Welt liegt *hinter* einer Öffnung – der Kristall liegt *in derselben Welt*, verzahnt mit der Wand (die Schollen stoßen an die Röhre, die Normale an der Nahtkante braucht beide Distanzen). Faustregel des Composites: **Getrennte Welten → Portal. Ein Raum, mehrere Werkstoffe → Material-Id im min().**

### 🎨 Experimentieren

- Die 0.5 auf 1.0 stellen und flach über den Boden schauen: die Plattenkanten bekommen Fransen – das Artefakt einmal absichtlich sehen (die beste Impfung, wie schon beim Terrain-Marsch der Serie)
- `kristallHoehe` verdoppeln (`* 0.32`, `* 0.20`): dramatischere Schollen – ab wann reißt die 0.5er-Reserve?
- `voronoi(q * 3.0)` → `q * 6.0`: feiner Splitterboden; `q * 1.5`: drei, vier monumentale Platten pro Fenster Abstand
- Schranke absichtlich falsch: `p.y + 0.85` – Löcher im Boden bei steilen Blicken. Konservativ heißt konservativ

🧠 **Merke:** Ein Höhenfeld darf in einen SDF-min()-Verbund – wenn es untertreibt (Faktor ≤ 0.5 auf seine vertikale Distanz) und wenn sein teurer Teil hinter einer konservativen Billig-Schranke liegt. Beide Zahlen gehören begründet, nicht gefühlt.

---

## Schritt 9 – Kristall-Shading: Brechung, Lampen, Absorption

**Neu:** Der Dispatch bekommt sein zweites Material. Das Debug-Türkis weicht dem kondensierten Crystal-Lights-Trio: `refract` an der Oberfläche, `1/d²`-Lampen auf einer Ebene unter dem Boden, Beer-Lambert-Absorption dazwischen – plus zwei Tunnel-spezifische Glanzterme.

```glsl
// ---- STELLSCHRAUBEN: KRISTALL (erweitert) ----------------------------------
const float K_TIEFE  = 0.55;   // Lampenebene unter dem Boden
const float K_DICHTE = 0.9;    // Absorption im Kristall (Achtung: != D_DICHTE!)
const float K_ZELLE  = 0.9;    // Rasterabstand der Lampen
// ----------------------------------------------------------------------------

// NEU: 1/d^2-Lampen mit Blink-Dramaturgie - kondensiert aus Crystal Lights 9
vec3 kristallLampen(vec2 q)
{
    vec2 base = floor(q / K_ZELLE);
    vec3 acc = vec3(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 id = base + vec2(float(x), float(y));
        vec2 c  = (id + 0.5 + 0.6 * (hash22(id + 7.0) - 0.5)) * K_ZELLE;
        vec2 d  = q - c;
        float wv = 0.5 + 0.5 * sin(TAU * (iTime * (0.35 + 0.75 * hash21(id + 17.3)) * 0.25
                                          + hash21(id + 31.7)));
        float hell = smoothstep(0.70, 0.97, wv) + 0.06;    // meist aus, weiches Aufflammen
        acc += pal(hash21(id) * 0.4 + 0.55) * hell / (0.02 + dot(d, d) * 40.0);
    }
    return acc * 0.06;
}

// NEU: das Kristall-Material (ersetzt die Debug-Farbe im Dispatch)
vec3 shadeKristall(vec3 p, vec3 n, vec3 rd, vec3 ro)
{
    // (1) BRECHUNG - eine Zeile Snellius, Grenzfall abgefangen
    vec3 rr = refract(rd, n, 1.0 / 1.45);
    if (dot(rr, rr) < 0.5) rr = rd;

    // (2) DURCHLAUF zur Lampenebene (Ebenen-Algebra, kein zweiter Marsch)
    float ebene = -K_HOEHE - K_TIEFE;
    float tt = (ebene - p.y) / min(rr.y, -0.05);
    vec2 q = (p + rr * tt).xz;

    // (3) ABSORPTION: dicker Kristall schluckt Rot zuerst (Beer-Lambert)
    vec3 T = exp(-max(p.y - ebene, 0.0) * vec3(0.85, 0.30, 0.16) * K_DICHTE * 2.0);
    vec3 col = kristallLampen(q) * T;

    // (4) Glanz des Kamera-Scheinwerfers auf den Platten
    vec3 zk = normalize(ro - p);
    col += pow(max(dot(reflect(rd, n), zk), 0.0), 40.0) * vec3(0.9, 0.95, 1.0) * 0.4;

    // (5) Fresnel: der Boden spiegelt das NEON - Naeherung ueber die Spiegelrichtung
    vec3 rf = reflect(rd, n);
    float fres = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += fres * tunnelNeon(atan(rf.y, rf.x), p.z) * 0.6;

    return col;
}
```

```glsl
// GEAENDERT: der Dispatch in mainImage
    vec3 n = tunnelNormale(p);
    vec3 col = (hit.y > 1.5) ? shadeKristall(p, n, rd, ro)
                             : tunnelShade(p, n, ro, w);
```

**Ergebnis:** Der dritte Quell-Shader ist angekommen: Auf den Bodenabschnitten glüht es farbig aus der Tiefe – Lampen flammen unter dem Kristall auf, ihr Licht bricht auf jeder Scholle anders, dicke Stellen färben es blaugrün, und über allem liegt der kalte Neon-Spiegelglanz der Röhrenwand.

### Was passiert hier

**Der Kondensat-Kern von Crystal Lights** ist die Kette (1)→(2)→(3): `refract` knickt den Blick, die Ebenen-Algebra spart den zweiten Marsch (unter der Oberfläche ist nichts im Weg – dieselbe bewusste Vereinfachung wie im Original), Beer-Lambert macht Dicke zu Farbe. Gestrichen wurden Liquiditätsfeld, Glow-Zweitabtastung und Mond – im schmalen, neonbeleuchteten Tunnelboden trägt davon nichts den Look. Die Lampen-Dramaturgie (meist aus, weiches Aufflammen per `smoothstep(0.70, 0.97, …)`) bleibt: Sie ist die halbe Signatur.

**Die zwei neuen Terme sind die eigentliche Composite-Arbeit.** Ein transplantiertes Material sieht erst dann „eingewachsen" aus, wenn es auf die Lichter seines *neuen* Zuhauses antwortet: (4) lässt den Scheinwerfer der Kamera auf den Platten glänzen – dasselbe Licht, das die Wand beleuchtet. (5) lässt den Boden das Neon spiegeln: Statt einen Spiegelstrahl zu marchen, fragen wir die Neon-**Karte** einfach in Spiegelrichtung ab (`atan(rf.y, rf.x)` – welcher Fuge schaut der gespiegelte Blick entgegen?). Das ist geometrisch eine grobe Näherung (sie ignoriert, *wo* der Spiegelstrahl die Wand träfe), aber sie hat die richtige Struktur: Farbige Streifen ziehen beim Fahren über den Boden, ausgerichtet an den echten Fugen. *Materialien einwachsen lassen heißt: die Lichtquellen des Wirts zitieren.*

💡 **Die Stellschrauben-Falle aus Schritt 3, jetzt scharf:** `K_DICHTE` (Absorption) und `D_DICHTE` (Zellbelegung) koexistieren ab hier im selben Shader. Wer „die Dichte" hochdreht, muss wissen, welche Welt gemeint ist – der Präfix ist jetzt keine Pedanterie mehr, sondern die einzige Verteidigungslinie.

### 🎨 Experimentieren

- `K_DICHTE = 0.2`: Klarglas – die Lampen stechen bunt durch; `2.0`: Milcheis, nur noch Farbnebel
- `K_ZELLE = 0.45`: dichter Lichtteppich unterm Boden; `1.8`: einzelne Signalfeuer
- Absorptionsvektor tauschen (`vec3(0.2, 0.5, 0.9)`): Bernstein-Boden im Neon-Tunnel – überraschend stimmig
- Term (5) abschalten: sofort wirkt der Boden „hineinkopiert". Der beste Beweis, was das Zitieren der Wirts-Lichter leistet

🧠 **Merke:** Beim Material-Transplantat zählt zweierlei – das kondensierte Eigenleben (Brechung, Lampen, Absorption) und die Antwort auf die Lichter des Wirts (Scheinwerfer, Neon). Fehlt das zweite, bleibt es ein Fremdkörper, egal wie schön es ist.

---
## Schritt 10 – Anti-Aliasing: Kanten in Pixelbreite und 2×2-Supersampling

**Neu:** Die Portal- und Fensterkanten werden **pixelgenau** weich (`fwidth`), der Bildaufbau wandert in eine Funktion `render(uv)` – und darüber legt sich als Stellschraube ein 2×2-Supersampling, mit ehrlicher Kostenrechnung.

```glsl
// ---- STELLSCHRAUBEN: GEMEINSAM (neu) ---------------------------------------
const int AA = 1;   // 1 = aus, 2 = 2x2-Supersampling (VIERFACHE Kosten!)
// ----------------------------------------------------------------------------

// NEU: der komplette Bildaufbau als Funktion (Inhalt = bisheriges mainImage,
// OHNE Belichtung/Gamma - die ziehen nach mainImage um)
vec3 render(vec2 uv)
{
    vec3 ro = vec3(0.0, 0.0, iTime * 1.5);
    vec3 rd = normalize(vec3(uv, 1.4));

    vec2 hit = tunnelMarch(ro, rd);
    float t = hit.x;
    vec3 p = ro + rd * t;
    float w = atan(p.y, p.x);
    vec3 n = tunnelNormale(p);

    vec3 col = (hit.y > 1.5) ? shadeKristall(p, n, rd, ro)
                             : tunnelShade(p, n, ro, w);

    col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    if (hit.y < 1.5) {
        vec2 fid;
        float fd = fensterDist(w, p.z, fid);
        float px = max(fwidth(fd), 0.004);              // GEAENDERT: Kantenbreite
        float F  = (1.0 - smoothstep(-px, px, fd))      //   = eine Pixelbreite
                 * exp(-0.001 * t * t);
        if (F > 0.004) {
            vec3 roD = D_URSPRUNG + p * P_MASSSTAB;
            col = mix(col, debrisWelt(roD, rd), F);
        }
        col += portalRahmen(fd, fid) * exp(-0.0008 * t * t);
    }
    return col;
}

// GEAENDERT: mainImage - AA-Schleife, danach (und nur danach) die Belichtung
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec3 col = vec3(0.0);
    for (int m = 0; m < AA; m++)
    for (int n = 0; n < AA; n++) {
        vec2 o = (vec2(float(m), float(n)) + 0.5) / float(AA) - 0.5;
        vec2 uv = (fragCoord + o - 0.5 * iResolution.xy) / iResolution.y;
        col += render(uv);
    }
    col /= float(AA * AA);

    col = 1.0 - exp(-col * 1.5);
    fragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
```

**Ergebnis:** Mit `AA = 1` verschwindet das Treppchen-Flimmern an den Portal- und Rahmenkanten – besonders an fernen Fenstern, deren Saum bisher schmaler als ein Pixel war. Mit `AA = 2` beruhigt sich zusätzlich alles, was `fwidth` nicht erreicht: Trümmer-Silhouetten hinter den Portalen, die Plattenkanten des Kristallbodens, das Neon in großer Tiefe.

### Was passiert hier – zwei Werkzeuge, zwei Zuständigkeiten

**`fwidth` – Anti-Aliasing für analytische Kanten.** Bisher war der Fenster-Saum konstant 0.05 *Welteinheiten* breit: nah zu matschig, fern schmaler als ein Pixel (= Flimmern). `fwidth(fd)` liefert, wie stark sich `fd` zwischen diesem Pixel und seinen Nachbarn ändert – also genau „eine Pixelbreite, ausgedrückt in fd-Einheiten". Der `smoothstep(-px, px, …)` ist damit auf jede Distanz exakt einen Pixel weich: die perfekte Kante, praktisch gratis. Zwei ehrliche Fußnoten: Erstens funktioniert das nur, weil `fd` eine **stetige Funktion des Ortes** ist – an Silhouetten (wo `p` zwischen Nachbarpixeln auf verschiedene Wände springt) liefert `fwidth` Unsinn; dort hilft nur Supersampling. Zweitens die `max(…, 0.004)`-Untergrenze: Sie hält die Kante auch dort definiert, wo die Ableitung degeneriert (z. B. streifender Blick).

**Supersampling – Anti-Aliasing für alles andere.** Vier Strahlen pro Pixel, versetzt um halbe Pixel, gemittelt **in Linearlicht vor dem Tonemapping** (deshalb der Umzug der Belichtung: Das Mittel gehört auf die physikalischen Werte, nicht auf die schon gekrümmten – sonst werden helle Kanten zu dunkel gemittelt). Die Schleifengrenze ist `const int` – GLSL-konform konstant, und mit `AA = 1` optimiert der Compiler die Schleife praktisch weg.

**Die Kostenrechnung, ohne Beschönigung.** Pro Strahl kostet das Bild im schlimmsten Fall (Fensterpixel über Kristallzone): ~90 × `tunnelMap` (davon die bodennahen mit fbm+voronoi ≈ 25 Hashes) + 6 × für die Normale + 60 × `debrisMap` + 9 Lampen ≈ **rund 150 map-Auswertungen**. `AA = 2` vervierfacht das auf ~600 – das ist die Größenordnung, in der ein Mittelklasse-Notebook bei Vollbild aus dem 60-fps-Takt fällt. Deshalb ist AA eine *Stellschraube* und kein Default: Auf Shadertoy in halber Auflösung meist locker drin, in LumiViz je nach Node-Auflösung abzuwägen (Anhang B). Merksatz: **fwidth zuerst – es erledigt die auffälligsten Kanten für ein Tausendstel des Preises; Supersampling ist die Holzhammer-Reserve.**

### 🎨 Experimentieren

- `AA = 2` gegen `AA = 1` im A/B-Blick auf eine ferne Fensterflucht – und dabei die fps im Shadertoy-Kopf beobachten: Kosten *fühlen*
- Die `fwidth`-Kante absichtlich zurückbauen (`px = 0.05`) und ein fernes Portal fixieren: das Flimmern kehrt zurück
- `px` verdreifachen (`fwidth(fd) * 3.0`): weiche „Vignetten-Fenster" – aus dem AA-Werkzeug wird ein Stilmittel
- Adaptiv-Experiment: AA nur für Fensterpixel? Geht im Fragment-Shader nicht sauber (die Entscheidung bräuchte den Marsch, der Marsch das AA) – einmal durchdenken, warum; das ist der Grund, warum echte Renderer solche Entscheidungen in Vorpässen fällen

🧠 **Merke:** Kanten-AA ist Arbeitsteilung: `fwidth` + `smoothstep` für jede Kante, die als stetige Distanz vorliegt (Fenster, Rahmen, Masken) – Supersampling für Silhouetten und alles Geraymarchte. Und immer in Linearlicht mitteln, vor dem Tonemapping.

---

## Schritt 11 – Kohärenz: eine Uhr, eine Palette, ein Tonemapping

**Neu:** Die Composite-Lektion schlechthin. Drei gute Shader nebeneinander ergeben noch kein Werk – sie ergeben eine **Collage**. Dieser Schritt zieht die vier Klammern ein, die daraus ein Bild machen: EINE Zeituhr, EINE Palette, EINE Kamera mit Charakter, EINE Schluss-Politur. Dazu die Licht-**Motivkopplung**: Das Glühen des Debris-Planeten fällt als Fensterlicht in den Tunnel.

```glsl
// ---- STELLSCHRAUBEN: GEMEINSAM (erweitert) ---------------------------------
const float TEMPO      = 1.0;   // die EINE Uhr (Kamera, Taumeln, Blinken, Atmen)
const float BELICHTUNG = 1.5;   // das EINE Tonemapping
// ----------------------------------------------------------------------------

// NEU: die EINE Uhr - mainImage setzt sie, ALLE Welt-Funktionen lesen sie.
// (Einbau: in jeder Funktion der drei Welten iTime durch gZeit ersetzen -
// betroffen: debrisMap, planetFarbe, fensterDist, portalRahmen, kristallLampen.)
float gZeit = 0.0;

// GEAENDERT: die EINE Palette bekommt eine eingebaute, langsame Farb-Uhr -
// Neon, Portal-Rahmen und Kristall-Lampen wandern ab jetzt GEMEINSAM
vec3 pal(float t)
{
    return 0.5 + 0.5 * cos(TAU * (t + gZeit * 0.012 + vec3(0.0, 0.33, 0.67)));
}

// GEAENDERT: die Kamera in render() - Vortrieb mit weichem Atmen + Rollen
    float zpos = gZeit * 1.2 + sin(gZeit * 0.20) * 5.0;
    vec3 ro = vec3(0.0, 0.0, zpos);
    vec3 rd = normalize(vec3(R(0.15 * sin(gZeit * 0.13)) * uv, 1.4));

// GEAENDERT: shadeWand - MOTIVKOPPLUNG: das Licht aus dem gegenueberliegenden
// Fenster faellt in Planet-Farbe auf die Wand (vor dem return einfuegen)
    vec2 gid;
    float einfall = smoothstep(0.06, -0.06, fensterDist(w + PI, p.z, gid));
    col += einfall * planetLicht((D_URSPRUNG + p * P_MASSSTAB).xz) * 0.35;

// GEAENDERT: mainImage nach der AA-Schleife - die EINE Schluss-Klammer
    gZeit = iTime * TEMPO;                       // (als ERSTE Zeile von mainImage)
    ...
    col /= float(AA * AA);

    col *= 0.86 + 0.14 * cos(iTime * 0.045 + vec3(0.0, 2.1, 4.2));  // EINE Farbdrift
    col = 1.0 - exp(-col * BELICHTUNG);                             // EIN Tonemapping
    col = pow(col, vec3(1.0 / 2.2));

    vec2 vuv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    col *= 1.0 - 0.30 * dot(vuv, vuv);                              // EINE Vignette
```

**Ergebnis:** Schwer zu screenshotten, sofort zu spüren: Das Bild wird **eins**. Die Neonfugen, die Portal-Rahmen und die Kristall-Lampen driften gemeinsam durch den Farbkreis; die Wand gegenüber heller Fenster glimmt warm im Planet-Orange; die Kamera rollt weich in die Fahrt, beschleunigt, atmet; und über allem liegt dieselbe langsame Drift und dieselbe Ausglüh-Kurve.

### Was passiert hier – die Anatomie des Collage-Effekts

Warum sehen zusammengesetzte Shader „zusammengesetzt" aus? Weil das Auge Inkonsistenzen über Bildregionen hinweg gnadenlos verrechnet. Die vier klassischen Verräter, und ihre Klammern:

1. **Mehrere Uhren.** Wenn Tunnel-Atmen, Trümmer-Taumeln und Lampen-Blinken auf getrennten `iTime`-Ausdrücken mit getrennten Tempo-Reglern liefen, würde ein globaler Tempo-Eingriff (Stellschraube, später Audio) die Welten *auseinanderziehen* – innen Zeitlupe, draußen Normalzeit: sofort Collage. `gZeit = iTime * TEMPO` ist die eine Uhr; die inkommensurablen Frequenzen *innerhalb* der Welten (0.20, 0.13, die Blink-Tempi …) bleiben – Vielfalt im Takt ist gewollt, verschiedene Takte nicht.
2. **Mehrere Paletten.** Drei Welten, drei Farbklimata – das ist der halbe Collage-Effekt. Unsere Klammer ist die geteilte `pal` mit eingebauter Farb-Uhr: Neon (Band 0.55), Rahmen (0.60) und Lampen (0.55) zitieren **dieselbe** Familie und wandern synchron. Der Planet behält bewusst sein Orange – ein Composite braucht keinen Einheitsbrei, sondern **ein dominantes Klima plus einen benannten Kontrast**. Hier: kaltes Neon innen, warme Glut draußen – und die Motivkopplung macht aus dem Kontrast eine Beziehung.
3. **Getrennte Lichtwelten.** Das ist die feinste und wirksamste Klammer: Der `einfall`-Term lässt das Planet-Glühen der Außenwelt auf die Innenwand fallen – abgefragt an der um 180° gegenüberliegenden Wandposition (`w + PI`), in der Farbe von `planetLicht` an der portal-transformierten Stelle. Physikalisch grob (keine echte Lichtleitung durchs Fenster), aber die **Kausalität stimmt sichtbar**: Wo draußen Glut ist, wird es drinnen warm. Licht, das Weltgrenzen überquert, ist das stärkste „das ist EIN Ort"-Signal, das ein Composite senden kann.
4. **Mehrfache Politur.** Zwei Tonemappings hintereinander (die Falle aus Schritt 2), zwei Farbdrifts, zwei Vignetten – jede Doppelung krümmt eine Bildhälfte anders. Die Regel ist mechanisch: **Welt-Eigenschaften (Dunst!) leben in der Welt; Bild-Eigenschaften (Drift, Tonemapping, Gamma, Vignette) leben genau einmal, ganz am Ende.**

Die Kamera schließlich bekommt zurück, was das Kondensieren gestrichen hatte – aber **einmal, fürs Ganze**: Vortriebs-Atmen (`sin`-Term auf der Position: Tempo pendelt zwischen 0.2 und 2.2, nie rückwärts – die weiche Beinahe-Umkehr der Serie) und ein leichtes Rollen. Dass die Fenster-Parallaxe, das Vorbeiströmen der Außenwelt und der Kristallboden alle an dieser einen Bewegung hängen, ist nach den Schritten 4–9 keine Zusatzarbeit mehr – es passiert von selbst. *Das* ist die Belohnung sauberer Übergabe-Verträge.

### 🎨 Experimentieren

- Den Collage-Effekt absichtlich herstellen: die Farb-Uhr nur im Neon (`pal` kopieren als `palAlt` ohne `gZeit` für die Lampen) – nach einer Minute Betrachtung benennen können, *warum* es billiger aussieht
- `TEMPO = 0.3`: Meditationsfahrt – und alles verlangsamt gemeinsam, auch draußen. Die eine Uhr bei der Arbeit
- Motivkopplung verstärken: `* 0.35` → `* 0.9` – die Wand wird zum Glut-Reflektor (kurz vor Kitsch)
- Den Kontrast kippen: Planet-Farben auf Cyan (`planetLicht`/`planetFarbe`-Mixfarben tauschen) – plötzlich braucht das Bild einen NEUEN benannten Kontrast, sonst wird es monoton. Klima-Design ist Entscheidungs-Design

🧠 **Merke:** Kohärenz ist eine Checkliste, keine Magie: eine Uhr, eine Palette (plus benannter Kontrast), Licht, das die Weltgrenze quert, und jede Bild-Politur genau einmal ganz am Ende. Wer die vier Klammern zieht, hat aus n Shadern ein Werk gemacht – wer eine auslässt, kann sie im Bild wiederfinden.

---
## Schritt 12 – Das Gesamtlisting

**Neu:** Nichts – und das ist der Punkt. Hier steht der komplette Shader am Stück, wie er sich aus den Schritten 1–11 ergibt: drei Welten, ein Strahl, eine Klammer. Zum Einfügen auf shadertoy.com/new.

```glsl
// ============================================================================
// "Composite: Portals" - drei Shader der Serie werden EIN Werk:
//   Wirt:       Stratospheric Tunnel (kondensiert) - Roehrenwand, Neon, Fenster
//   Aussenwelt: Space Debris (kondensiert)         - hinter den Fenstern (Portal)
//   Boden:      Crystal Lights (kondensiert)       - abschnittsweise (Material-Id)
// Endstand des Tutorials (Schritt 12). Braucht keine iChannels.
// ============================================================================

// ---- STELLSCHRAUBEN: GEMEINSAM ---------------------------------------------
const float TEMPO      = 1.0;    // die EINE Uhr (Kamera, Taumeln, Blinken, Atmen)
const float BELICHTUNG = 1.5;    // das EINE Tonemapping
const int   AA         = 1;      // 1 = aus, 2 = 2x2-Supersampling (4x Kosten!)

// ---- STELLSCHRAUBEN: TUNNEL (Wirtswelt) ------------------------------------
const float T_RADIUS   = 1.0;    // Grundradius der Roehre
const float T_ROEHREN  = 12.0;   // Roehren um den Umfang (ganzzahlig!)
const float T_TIEFE    = 0.10;   // Woelbung der Roehren
const float T_SPALTEN  = 6.0;    // Fensterspalten um den Umfang (ganzzahlig!)
const float T_ABSTAND  = 5.0;    // Fensterabstand entlang z
const float T_DICHTE   = 0.55;   // Anteil der Zellen mit Fenster
const float T_NEON     = 0.010;  // Helligkeit der Neonfugen
const float T_LICHT    = 1.4;    // Kamera-Scheinwerfer

// ---- STELLSCHRAUBEN: PORTAL ------------------------------------------------
const float P_MASSSTAB = 4.0;    // 1 = Aussenwelt in Weltgroesse, >1 = Diorama
const float P_ATMEN    = 0.25;   // Fenster oeffnen/schliessen sich (0 = statisch)
const float P_RAHMEN   = 0.06;   // Breite des Neon-Rahmens um jedes Portal
const vec3  D_URSPRUNG = vec3(7.0, 2.0, 13.0);   // Lage der Aussenwelt

// ---- STELLSCHRAUBEN: DEBRIS (Aussenwelt) -----------------------------------
const float D_ZELLE    = 3.0;    // Kantenlaenge einer Gitterzelle
const float D_DICHTE   = 0.50;   // Anteil belegter Zellen
const float D_GROESSE  = 0.9;    // Groessen-Budget je Teil (Zellregel!)
const float D_TAUMEL   = 1.0;    // Taumel-Tempo
const float D_PLANET_R = 60.0;   // Kruemmungsradius des Planeten
const float D_PLANET_H = 8.0;    // Abstand Weltnull -> Planetenoberflaeche
const float D_GLUT     = 1.2;    // Intensitaet des Lavagrunds

// ---- STELLSCHRAUBEN: KRISTALL (Bodenabschnitte) ----------------------------
const float K_PERIODE  = 24.0;   // Streckenperiode der Abschnitte (z)
const float K_ANTEIL   = 0.45;   // Anteil der Strecke mit Kristallboden
const float K_HOEHE    = 0.72;   // Bodenniveau unter der Tunnelachse
const float K_TIEFE    = 0.55;   // Lampenebene unter dem Boden
const float K_DICHTE   = 0.9;    // Absorption im Kristall (!= D_DICHTE!)
const float K_ZELLE    = 0.9;    // Rasterabstand der Lampen

// ---- abgeleitet ------------------------------------------------------------
const float D_MARGE   = D_ZELLE * 0.5 - 1.1 * D_GROESSE;   // Zellregel-Reserve
const vec3  D_ZENTRUM = vec3(0.0, -(D_PLANET_R + D_PLANET_H), 0.0);
const vec3  D_SONNE   = normalize(vec3(0.65, 0.28, -0.70));
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float TAU = 6.28318530;
const float PI  = 3.14159265;

float gZeit = 0.0;              // die EINE Uhr: iTime * TEMPO (mainImage setzt sie)
vec3  gAuge = vec3(0.0);        // Portal-Austritt fuer die Debris-Blase

// ---- gemeinsame Helfer (jede Funktion existiert genau EINMAL) --------------

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash13(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

vec3 hash33(vec3 p)
{
    return fract(sin(vec3(dot(p, vec3(127.1, 311.7,  74.7)),
                          dot(p, vec3(269.5, 183.3, 246.1)),
                          dot(p, vec3(113.5, 271.9, 124.6)))) * 43758.5453);
}

float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(i),              hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)), hash21(i + vec2(1, 1)), u.x), u.y);
}

float fbm(vec2 p)   // Entscheid Schritt 3: die 4-Oktaven-Fassung fuer ALLE Welten
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

vec3 voronoi(vec2 p)   // (Abstand^2, Zell-Id)
{
    vec2 i = floor(p), f = fract(p);
    float best = 8.0;
    vec2 bestId = vec2(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 g = vec2(float(x), float(y));
        vec2 r = g + hash22(i + g) - f;
        float d = dot(r, r);
        if (d < best) { best = d; bestId = i + g; }
    }
    return vec3(best, bestId);
}

vec3 pal(float t)   // die EINE Palette - mit eingebauter, langsamer Farb-Uhr
{
    return 0.5 + 0.5 * cos(TAU * (t + gZeit * 0.012 + vec3(0.0, 0.33, 0.67)));
}

mat3 rotAchse(vec3 a, float w)
{
    float c = cos(w), s = sin(w), k = 1.0 - c;
    return mat3(a.x * a.x * k + c,       a.y * a.x * k + a.z * s,  a.z * a.x * k - a.y * s,
                a.x * a.y * k - a.z * s, a.y * a.y * k + c,        a.z * a.y * k + a.x * s,
                a.x * a.z * k + a.y * s, a.y * a.z * k - a.x * s,  a.z * a.z * k + c);
}

float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// ============================================================================
// WELT 2: DEBRIS (Aussenwelt) - kondensiert aus Space Debris
// ============================================================================

float debrisMap(vec3 p)
{
    vec3 id = floor(p / D_ZELLE);
    vec3 q  = mod(p, D_ZELLE) - 0.5 * D_ZELLE;

    // Zellwand-Klammer: konservative Schranke fuer alles ausserhalb der Zelle
    float wand   = D_ZELLE * 0.5 - max(abs(q.x), max(abs(q.y), abs(q.z)));
    float sicher = wand + D_MARGE;

    if (hash13(id + 4.7) > D_DICHTE) return sicher;

    float gr = D_GROESSE * (0.35 + 0.65 * hash13(id + 3.1));

    // Portal-Blase: Zellen direkt am Austrittspunkt schrumpfen weg
    vec3 zentrum = (id + 0.5) * D_ZELLE;
    gr *= smoothstep(1.0, 3.5, length(zentrum - gAuge));
    if (gr < 0.02) return sicher;

    vec3 achse = normalize(hash33(id + 5.7) - 0.5 + vec3(0.01, 0.02, 0.03));
    float tempo = (0.25 + 1.25 * hash13(id + 9.2)) * D_TAUMEL;
    q = rotAchse(achse, gZeit * tempo + TAU * hash13(id + 1.9)) * q;

    float d = sdBox(q, gr * (0.30 + 0.28 * hash33(id + 2.6)));
    d -= 0.08 * gr * sin(4.7 * q.x) * sin(4.3 * q.y) * sin(5.1 * q.z);
    return min(d, sicher);
}

float debrisMarch(vec3 ro, vec3 rd, float tMax)
{
    float t = 0.0;
    for (int i = 0; i < 60; i++) {
        float d = debrisMap(ro + rd * t);
        if (d < 0.0015 + 0.0015 * t) return t;
        t += d * 0.7;
        if (t > tMax) break;
    }
    return -1.0;
}

vec3 debrisNormale(vec3 p)
{
    const vec2 e = vec2(0.002, -0.002);
    return normalize(e.xyy * debrisMap(p + e.xyy) + e.yyx * debrisMap(p + e.yyx) +
                     e.yxy * debrisMap(p + e.yxy) + e.xxx * debrisMap(p + e.xxx));
}

float planetHit(vec3 ro, vec3 rd)
{
    vec3 oc = ro - D_ZENTRUM;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - D_PLANET_R * D_PLANET_R;
    float h = b * b - c;
    if (h < 0.0) return -1.0;
    float t = -b - sqrt(h);
    return (t > 0.0) ? t : -1.0;
}

vec3 planetFarbe(vec2 q)
{
    float grund = fbm(q * 0.045 + vec2(gZeit * 0.010, 0.0));
    float adern = fbm(q * 0.16 + grund * 1.8 + 7.0);
    float glut  = pow(clamp(adern * 1.35 - 0.25, 0.0, 1.0), 2.2) * D_GLUT;
    vec3 col = vec3(0.028, 0.010, 0.012);
    col = mix(col, vec3(0.55, 0.08, 0.015), smoothstep(0.10, 0.45, glut));
    col = mix(col, vec3(1.15, 0.55, 0.10),  smoothstep(0.45, 0.85, glut));
    return col;
}

vec3 planetLicht(vec2 q)   // grobes Gluehen - auch Fensterlicht im Tunnel
{
    return mix(vec3(0.30, 0.05, 0.01), vec3(1.00, 0.45, 0.10),
               smoothstep(0.35, 0.80, fbm(q * 0.030)));
}

vec3 atmosphaere(vec3 ro, vec3 rd)
{
    vec3 oc = ro - D_ZENTRUM;
    float tca = -dot(oc, rd);
    if (tca < 0.0) return vec3(0.0);
    float hmin = sqrt(max(dot(oc, oc) - tca * tca, 0.0)) - D_PLANET_R;
    return exp(-max(hmin, 0.0) * 0.30) * vec3(0.90, 0.32, 0.08) * 0.55;
}

vec3 debrisHimmel(vec3 ro, vec3 rd)
{
    vec3 col = vec3(0.008, 0.010, 0.018);
    vec2 su = rd.xy / (abs(rd.z) + 0.4);
    col += vec3(0.9) * smoothstep(0.994, 1.0, hash21(floor(su * 48.0)));
    return col + atmosphaere(ro, rd);
}

vec3 debrisShade(vec3 p, vec3 n, vec3 rd)
{
    vec3 id = floor(p / D_ZELLE);
    vec3 alb = vec3(0.42, 0.44, 0.47) * (0.55 + 0.90 * hash13(id + 12.5));

    vec3 col = alb * max(dot(n, D_SONNE), 0.0) * vec3(1.30, 1.18, 1.00);
    col += pow(max(dot(reflect(rd, n), D_SONNE), 0.0), 24.0) * vec3(0.90, 0.85, 0.75) * 0.8;

    float hoehe = clamp((p.y + D_PLANET_H) / D_PLANET_H, 0.0, 2.0);
    col += max(-n.y, 0.0) * planetLicht(p.xz) * exp(-hoehe * 1.1) * 0.9;
    return col;
}

// Schnittstelle der Aussenwelt: Strahl rein, Linearlicht raus (KEIN Tonemapping)
vec3 debrisWelt(vec3 ro, vec3 rd)
{
    gAuge = ro;

    float tP = planetHit(ro, rd);
    float tMax = (tP > 0.0) ? min(50.0, tP) : 50.0;
    float tD = debrisMarch(ro, rd, tMax);

    vec3 col;
    float tHit;
    if (tD > 0.0 && (tP < 0.0 || tD < tP)) {
        vec3 p = ro + rd * tD;
        col = debrisShade(p, debrisNormale(p), rd);
        tHit = tD;
    } else if (tP > 0.0) {
        col = planetFarbe((ro + rd * tP).xz);
        tHit = tP;
    } else {
        return debrisHimmel(ro, rd);
    }

    // Dunst der Aussenwelt - in DEREN Einheiten (Massstab bleibt draussen)
    vec3 dunst = mix(vec3(0.010, 0.012, 0.022), vec3(0.30, 0.13, 0.05),
                     clamp(-rd.y * 2.2, 0.0, 1.0));
    return mix(col, dunst, 1.0 - exp(-0.0009 * tHit * tHit));
}

// ============================================================================
// WELT 3: KRISTALL (Bodenabschnitte) - kondensiert aus Crystal Lights
// ============================================================================

float kristallZone(float z)
{
    float zz = fract(z / K_PERIODE);
    return smoothstep(0.03, 0.12, zz) * (1.0 - smoothstep(K_ANTEIL - 0.09, K_ANTEIL, zz));
}

float kristallHoehe(vec2 q)
{
    float glatt  = (fbm(q * 1.1) - 0.5) * 0.16;
    float platte = (hash21(voronoi(q * 3.0).yz) - 0.5) * 0.10;
    return glatt + platte;
}

vec3 kristallLampen(vec2 q)
{
    vec2 base = floor(q / K_ZELLE);
    vec3 acc = vec3(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 id = base + vec2(float(x), float(y));
        vec2 c  = (id + 0.5 + 0.6 * (hash22(id + 7.0) - 0.5)) * K_ZELLE;
        vec2 d  = q - c;
        float wv = 0.5 + 0.5 * sin(TAU * (gZeit * (0.35 + 0.75 * hash21(id + 17.3)) * 0.25
                                          + hash21(id + 31.7)));
        float hell = smoothstep(0.70, 0.97, wv) + 0.06;
        acc += pal(hash21(id) * 0.4 + 0.55) * hell / (0.02 + dot(d, d) * 40.0);
    }
    return acc * 0.06;
}

// ============================================================================
// WELT 1: TUNNEL (Wirt) - kondensiert aus Stratospheric Tunnel
// ============================================================================

float bodenHoehe(vec3 p)
{
    // Lueckentrick: ausserhalb der Zone versinkt der Boden unter die Wand
    float ziel = -K_HOEHE + kristallHoehe(p.xz);
    return mix(-(T_RADIUS + 0.8), ziel, kristallZone(p.z));
}

// map liefert (Distanz, Material-Id): 1 = Roehrenwand, 2 = Kristallboden
vec2 tunnelMap(vec3 p)
{
    float w = atan(p.y, p.x);
    float dWand = T_RADIUS - T_TIEFE * (0.5 - 0.5 * cos(w * T_ROEHREN)) - length(p.xy);

    // Boden: oben reicht eine billige Schranke, erst unten zahlt das Hoehenfeld;
    // Hoehenfeld im min-Verbund => konservativ halbieren (Schritt 8)
    float dBoden = (p.y > -0.30) ? (p.y + 0.55)
                                 : (p.y - bodenHoehe(p)) * 0.5;

    return (dWand < dBoden) ? vec2(dWand, 1.0) : vec2(dBoden, 2.0);
}

vec2 tunnelMarch(vec3 ro, vec3 rd)
{
    float t = 0.02, id = 1.0;
    for (int i = 0; i < 90; i++) {
        vec2 dm = tunnelMap(ro + rd * t);
        id = dm.y;
        if (dm.x < 0.0015 + 0.001 * t) break;
        t += dm.x * 0.7;
        if (t > 40.0) break;
    }
    return vec2(min(t, 40.0), id);
}

vec3 tunnelNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(tunnelMap(p + e.xyy).x - tunnelMap(p - e.xyy).x,
                          tunnelMap(p + e.yxy).x - tunnelMap(p - e.yxy).x,
                          tunnelMap(p + e.yyx).x - tunnelMap(p - e.yyx).x));
}

// Fenster: signierte Kanten-Distanz (< 0 = im Fenster) + Zell-Id
float fensterDist(float w, float z, out vec2 id)
{
    vec2 zelle = vec2(fract(w / TAU + 0.5) * T_SPALTEN, z / T_ABSTAND);
    id = floor(zelle);
    if (hash21(id + 3.1) > T_DICHTE) return 1e3;

    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * T_RADIUS / T_SPALTEN;
    c.y *= T_ABSTAND;

    float o = clamp(0.8 + P_ATMEN * sin(gZeit * (0.15 + 0.25 * hash21(id + 9.4))
                                        + TAU * hash21(id)), 0.0, 1.0);
    vec2 halb = vec2(0.30, 0.85) * o;
    return max(abs(c.x) - halb.x, abs(c.y) - halb.y);
}

vec3 tunnelNeon(float w, float z)
{
    float fu  = w * T_ROEHREN / TAU;
    float gid = mod(floor(fu + 0.5), T_ROEHREN);
    float ad  = abs(fract(fu + 0.5) - 0.5) * TAU * T_RADIUS / T_ROEHREN;
    return pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55) * T_NEON / (0.0015 + ad * ad * 60.0);
}

vec3 portalRahmen(float d, vec2 id)
{
    float band = smoothstep(P_RAHMEN, 0.0, abs(d));
    float puls = 0.6 + 0.4 * sin(gZeit * (0.8 + 0.6 * hash21(id + 6.6))
                                 + TAU * hash21(id + 2.2));
    return band * puls * pal(hash21(id + 8.5) * 0.3 + 0.60) * 0.9;
}

vec3 tunnelShade(vec3 p, vec3 n, vec3 ro, float w)
{
    vec3 basis = vec3(0.06, 0.07, 0.10);
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    vec3 col = basis * (T_LICHT * max(dot(n, zk / dk), 0.0) / (1.0 + dk * dk * 0.12) + 0.02);
    col += tunnelNeon(w, p.z);

    // Motivkopplung: Licht aus dem gegenueberliegenden Fenster in Planet-Farbe
    vec2 gid;
    float einfall = smoothstep(0.06, -0.06, fensterDist(w + PI, p.z, gid));
    col += einfall * planetLicht((D_URSPRUNG + p * P_MASSSTAB).xz) * 0.35;

    return col;
}

vec3 shadeKristall(vec3 p, vec3 n, vec3 rd, vec3 ro)
{
    // vereinfachte Brechung: refract -> Ebenen-Schnitt -> Beer-Lambert
    vec3 rr = refract(rd, n, 1.0 / 1.45);
    if (dot(rr, rr) < 0.5) rr = rd;

    float ebene = -K_HOEHE - K_TIEFE;
    float tt = (ebene - p.y) / min(rr.y, -0.05);
    vec2 q = (p + rr * tt).xz;

    vec3 T = exp(-max(p.y - ebene, 0.0) * vec3(0.85, 0.30, 0.16) * K_DICHTE * 2.0);
    vec3 col = kristallLampen(q) * T;

    // Glanz des Kamera-Scheinwerfers auf den Platten
    vec3 zk = normalize(ro - p);
    col += pow(max(dot(reflect(rd, n), zk), 0.0), 40.0) * vec3(0.9, 0.95, 1.0) * 0.4;

    // Fresnel: der Boden spiegelt das Neon (Naeherung ueber die Spiegelrichtung)
    vec3 rf = reflect(rd, n);
    float fres = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += fres * tunnelNeon(atan(rf.y, rf.x), p.z) * 0.6;

    return col;
}

// ---- ein Bild ---------------------------------------------------------------

vec3 render(vec2 uv)
{
    // die EINE Kamera: Vortrieb mit weichem Atmen + leichtes Rollen
    float zpos = gZeit * 1.2 + sin(gZeit * 0.20) * 5.0;
    vec3 ro = vec3(0.0, 0.0, zpos);
    vec3 rd = normalize(vec3(R(0.15 * sin(gZeit * 0.13)) * uv, 1.4));

    vec2 hit = tunnelMarch(ro, rd);
    float t = hit.x;
    vec3 p = ro + rd * t;
    float w = atan(p.y, p.x);
    vec3 n = tunnelNormale(p);

    // Material-Dispatch ueber die Id
    vec3 col = (hit.y > 1.5) ? shadeKristall(p, n, rd, ro)
                             : tunnelShade(p, n, ro, w);

    // Tunnel-Dunst
    col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    // DAS PORTAL - nur auf der Wand, nie auf dem Kristallboden
    if (hit.y < 1.5) {
        vec2 fid;
        float fd = fensterDist(w, p.z, fid);
        float px = max(fwidth(fd), 0.004);              // Kante = eine Pixelbreite
        float F  = (1.0 - smoothstep(-px, px, fd)) * exp(-0.001 * t * t);
        if (F > 0.004) {
            vec3 roD = D_URSPRUNG + p * P_MASSSTAB;     // Uebergabe in Welt 2
            col = mix(col, debrisWelt(roD, rd), F);
        }
        col += portalRahmen(fd, fid) * exp(-0.0008 * t * t);
    }
    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    gZeit = iTime * TEMPO;

    vec3 col = vec3(0.0);
    for (int m = 0; m < AA; m++)
    for (int n = 0; n < AA; n++) {
        vec2 o = (vec2(float(m), float(n)) + 0.5) / float(AA) - 0.5;
        vec2 uv = (fragCoord + o - 0.5 * iResolution.xy) / iResolution.y;
        col += render(uv);
    }
    col /= float(AA * AA);

    // EINE Farbdrift + EIN Tonemapping + Gamma + Vignette - fuer ALLES
    col *= 0.86 + 0.14 * cos(iTime * 0.045 + vec3(0.0, 2.1, 4.2));
    col = 1.0 - exp(-col * BELICHTUNG);
    col = pow(col, vec3(1.0 / 2.2));

    vec2 vuv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    col *= 1.0 - 0.30 * dot(vuv, vuv);

    fragColor = vec4(col, 1.0);
}
```

**Ergebnis:** Das fertige Composite. Der Neon-Tunnel zieht vorbei, seine Fugen und Portal-Rahmen wandern gemeinsam durch die Farben; durch die atmenden Fenster taumelt das Trümmerfeld über dem Glutplaneten, dessen Licht warm auf die Innenwand fällt; abschnittsweise übernimmt der Kristallboden, unter dem Lampen aufflammen und das Neon sich spiegelt. Eine Kamera, eine Uhr, eine Schluss-Klammer.

### Was passiert hier – Bilanz des Merges

Die Zahlen erzählen die Geschichte: Drei Gesamtlistings mit zusammen ~950 Zeilen wurden zu **einem** Shader von ~420 Zeilen – nicht durch Kompression, sondern durch die Schritte 1–2 (Kondensieren: nur Look-Träger überleben), Schritt 3 (geteilte Helfer statt Dreifach-Kopien) und die Disziplin, Politur genau einmal zu schreiben. Und die Architektur ist erweiterbar geblieben: Eine vierte Welt wäre ein weiteres Skelett mit `xxxWelt(ro, rd)`-Vertrag (Portal-Weg) oder eine weitere Id im `min()` (Material-Weg).

### 🎨 Experimentieren – jetzt am Gesamtwerk

- Das Stellschrauben-Brett durchspielen – jede Sektion ein Charakter: `P_MASSSTAB 8 / T_DICHTE 0.8 / K_ANTEIL 0.1` = Panorama-Express über einer Miniaturwelt; `P_MASSSTAB 1 / T_DICHTE 0.3 / K_ANTEIL 0.9` = Eisgrotte mit seltenen, echten Ausblicken
- `BELICHTUNG 2.5`: das Neon frisst sich in die Röhren, die Glut in die Fenster – Overdrive-Look
- `T_ROEHREN 24 / T_NEON 0.004 / TEMPO 0.4`: stille Orbital-Kathedrale
- Die Portale zu Spiegeln machen (🎨 aus Schritt 4) und `K_ANTEIL 0.9`: ein Kristall-Spiegelkabinett

🧠 **Merke:** Ein gutes Composite erkennt man am Gesamtlisting: getrennte STELLSCHRAUBEN-Sektionen je Welt, gemeinsame Helfer genau einmal, Welten als klar geschnittene Blöcke mit Vertrag – und eine einzige Schluss-Klammer. Wer das Listing liest, kann den Merge rückwärts erzählen.

---
# Anhang A: Audio-Reaktivität

Voraussetzung auf shadertoy.com: **iChannel0 mit „Music"** belegen (Kanal-Kachel → Music). Die Textur ist 512×2: Zeile 0 (`y ≈ 0.25`) das FFT-Spektrum, Zeile 1 die Wellenform. Die ausführliche Herleitung von Band-Mittelung und Beat-Gate-Dramaturgie steht im **Anhang A des Crystal-Lights-Tutorials** – A1 hier ist trotzdem eigenständig lauffähig. A2 und A3 sind composite-spezifisch: Was heißt Audio-Mapping, wenn ein Shader **mehrere Welten** enthält?

---

## Schritt A1 – Das Beat-Gate (eigenständig lauffähig)

**Neu:** Nichts Neues für Kenner der Serie – das Bass-Gate als eigenständiger Prüfstand, damit dieses Tutorial für sich funktioniert.

```glsl
// iChannel0: Music
float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++)
        sum += texture(iChannel0, vec2(mix(lo, hi, (float(i) + 0.5) / float(N)), 0.25)).x;
    return sum / float(N);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float bass = bandLevel(0.00, 0.05);
    float gate = smoothstep(0.60, 0.75, bass);      // DAS Beat-Gate

    vec3 col = vec3(0.02);
    if (uv.x < 0.47) col = uv.y < bass ? vec3(0.9, 0.3, 0.3) : col;   // roher Pegel
    if (uv.x > 0.53) col = uv.y < gate ? vec3(0.3, 0.9, 1.0) : col;   // das Gate
    fragColor = vec4(col + gate * vec3(0.08, 0.05, 0.02), 1.0);
}
```

**Ergebnis:** Links wogt der Bass kontinuierlich, rechts springt das Gate hart auf und zu. Die Schwellen `0.60/0.75` sind Handarbeit pro Musikrichtung; die adaptive, track-unabhängige Fassung (Buffer-A-Envelope) steht in **Crystal Lights, Anhang B3** und funktioniert hier unverändert.

---

## Schritt A2 – Der Mapping-Katalog: EIN Audio-Satz für beide Welten

Kein neuer Shader – die Landkarte. Vorweg aber das composite-spezifische Prinzip, und es ist wichtiger als jedes einzelne Mapping:

**Es gibt genau EINEN Audio-Satz** – `gBass`, `gMid`, `gTreb`, `gVol`, `gGate` – einmal pro Frame gefüllt, und **beide Welten lesen dieselben Werte**. Die Versuchung liegt nahe, je Welt eigene Schwellen oder gar eigene Bänder zu vergeben („der Tunnel hört auf den Bass, die Außenwelt auf die Höhen mit eigener Gate-Schwelle"). Das Ergebnis wäre der **Collage-Effekt im Zeitbereich**: Beim Kick zuckt die eine Welt, die andere einen Wimpernschlag später oder gar nicht – und das Auge sortiert die Welten sofort wieder in „zwei Videos". Die Kohärenz-Checkliste aus Schritt 11 bekommt also einen fünften Punkt: **ein Audio-Satz, eine Schwelle, ein Gate – die Welten dürfen verschieden *antworten*, aber sie müssen dasselbe *hören*.**

| # | Audio | steuert | Eingriff (Diffs in A3) | warum es passt |
|---|---|---|---|---|
| 1 | Bass-**Gate** | Portale öffnen sich weiter | in `fensterDist()`: `o` bekommt `+ gGate * 0.35` | Der Beat reißt den Blick nach draußen auf – das dramatischste Composite-Mapping: Er verbindet die Welten, statt eine zu schmücken |
| 2 | Bass (kontinuierlich) | Glut der Außenwelt | in `planetFarbe()`: Glut × `(0.6 + 1.4·gBass)`; in `planetLicht()`: × `(0.5 + 1.5·gBass)` | Bass = Masse = der Planet atmet. Und weil `planetLicht` AUCH das Fensterlicht im Tunnel färbt (Motivkopplung!), pumpt der Bass sichtbar **durch die Fenster hindurch** – ein Mapping, zwei Welten |
| 3 | Mitten | die EINE Palette | in `pal()`: `+ gMid * 0.25` in die Phase | Melodie = Farbe – und weil Neon, Rahmen und Kristall-Lampen alle `pal` teilen, wandert das ganze Werk gemeinsam. Der Kohärenz-Bonus der geteilten Palette, geschenkt |
| 4 | Höhen | Glanzlichter | in `shadeKristall()` (4): × `(0.2 + 2.5·gTreb)`; in `debrisShade()` Spec: × `(0.2 + 2.0·gTreb)` | Hi-Hats sind spitz und glitzernd – auf Kristallplatten UND Trümmerkanten, gleichzeitig |
| 5 | Lautheit | Kristall-Grundglimmen | in `kristallLampen()`: `+ 0.06` → `+ 0.02 + 0.25·gVol` | Laute Passagen fluten den Boden mit Dauerlicht, leise lassen nur das Einzel-Blinken |
| 6 | Bass-**Gate** | Portal-Rahmen blitzen | in `portalRahmen()`: `puls = max(puls, gGate)` | Der Rahmen ist die Naht der Welten – beim Beat leuchtet genau sie auf |

Die Warnungen der Serie gelten unverändert: **nie auf Faktoren vor der Uhr** (`gZeit`-Tempi sind Positionen – Audio darauf teleportiert Kamera und Taumler), und Geometrie-Mappings (hier: Mapping 1, das einzige geometrische) flackern mit rohen Pegeln – das Gate ist deshalb bewusst binär mit smoothstep-Saum, und die Luxus-Fassung nimmt die Envelope aus Crystal Lights B3.

---

## Schritt A3 – Das Composite hört zu

**Neu:** Die Mappings 1–6 wandern in den fertigen Shader. Gezeigt sind nur die Änderungen gegenüber dem Gesamtlisting aus Schritt 12 – auf shadertoy.com zusätzlich **iChannel0 = Music** setzen.

**(a) Vor die STELLSCHRAUBEN** – die Audio-Infrastruktur:

```glsl
// ---- AUDIO -----------------------------------------------------------------
float gBass = 0.0, gMid = 0.0, gTreb = 0.0, gVol = 0.0, gGate = 0.0;

float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++)
        sum += texture(iChannel0, vec2(mix(lo, hi, (float(i) + 0.5) / float(N)), 0.25)).x;
    return sum / float(N);
}
// ----------------------------------------------------------------------------
```

**(b) Am Anfang von `mainImage`** – einmal füllen, **vor** der AA-Schleife (der Audio-Satz ist Frame-Zustand, kein Sub-Sample-Zustand):

```glsl
    gZeit = iTime * TEMPO;
    gBass = bandLevel(0.00, 0.05);
    gMid  = bandLevel(0.05, 0.25);
    gTreb = bandLevel(0.25, 0.70);
    gVol  = bandLevel(0.00, 0.70);
    gGate = smoothstep(0.60, 0.75, gBass);
```

**(c) Die sechs Eingriffe** (Nummern aus A2):

```glsl
// fensterDist(): der Beat reisst die Portale auf                       [1]
    float o = clamp(0.8 + P_ATMEN * sin(gZeit * (0.15 + 0.25 * hash21(id + 9.4))
                                        + TAU * hash21(id))
                        + gGate * 0.35, 0.0, 1.2);

// planetFarbe(): der Bass pumpt die Glut                               [2]
    float glut = pow(clamp(adern * 1.35 - 0.25, 0.0, 1.0), 2.2)
               * D_GLUT * (0.6 + 1.4 * gBass);

// planetLicht(): ... und damit auch das Fensterlicht im Tunnel         [2]
    return mix(vec3(0.30, 0.05, 0.01), vec3(1.00, 0.45, 0.10),
               smoothstep(0.35, 0.80, fbm(q * 0.030))) * (0.5 + 1.5 * gBass);

// pal(): die Mitten schieben die EINE Farb-Uhr                         [3]
    return 0.5 + 0.5 * cos(TAU * (t + gZeit * 0.012 + gMid * 0.25
                                  + vec3(0.0, 0.33, 0.67)));

// shadeKristall() (4) und debrisShade() (Spec): Hoehen auf den Glanz   [4]
    col += pow(max(dot(reflect(rd, n), zk), 0.0), 40.0)
         * vec3(0.9, 0.95, 1.0) * 0.4 * (0.2 + 2.5 * gTreb);
    col += pow(max(dot(reflect(rd, n), D_SONNE), 0.0), 24.0)
         * vec3(0.90, 0.85, 0.75) * 0.8 * (0.2 + 2.0 * gTreb);

// kristallLampen(): Lautheit hebt das Grundglimmen                     [5]
    float hell = smoothstep(0.70, 0.97, wv) + 0.02 + 0.25 * gVol;

// portalRahmen(): der Beat blitzt in der Naht                          [6]
    float puls = max(0.6 + 0.4 * sin(gZeit * (0.8 + 0.6 * hash21(id + 6.6))
                                     + TAU * hash21(id + 2.2)), gGate);
```

**Ergebnis:** Beim Kick reißen alle Portale gemeinsam auf, ihre Rahmen blitzen, und durch die größeren Öffnungen pulst die vom selben Kick aufgeglühte Planetenwelt – deren Licht im selben Moment wärmer auf die Innenwand fällt. Die Melodie schiebt Neon, Rahmen und Bodenlampen durch dieselben Farben; Hi-Hats besprühen Kristall und Trümmer mit Glitzer. Wird es still, fällt das Composite auf sein Eigenleben zurück: atmende Portale, einzeln blinkende Lampen, taumelnde Trümmer.

### Was passiert hier

Das dramaturgische Kalkül der Serie gilt verschärft: Musik **verstärkt** das Eigenleben, ersetzt es nie (deshalb überall `(a + b·Pegel)`-Faktoren und `max(eigen, gate)`-Muster). Neu ist die Composite-Pointe: Die wirkungsvollsten Mappings sind die, die **die Weltgrenze bespielen** – Portalöffnung (1), das durchs Fenster pumpende Glutlicht (2) und der blitzende Rahmen (6) inszenieren genau die Naht, die Schritt 11 zum Motiv gemacht hat. Ein Composite, dessen Audio nur je-Welt-Schmuck steuert, verschenkt seine beste Bühne.

### 🎨 Experimentieren

- Nur Mapping 1+6 aktiv: „der Tunnel schlägt die Fenster im Takt auf" – bereits ein kompletter Visualizer
- `gGate` invertieren (`1.0 - gGate` auf `o`): Portale **schließen** beim Beat – klaustrophobisch, interessant bei harten Tracks
- Das Konsistenz-Gegenexperiment: für die Außenwelt eine eigene Gate-Schwelle (`0.4/0.5`) vergeben und zusehen, wie die Welten rhythmisch auseinanderfallen – einmal gesehen, nie wieder gebaut
- Envelope-Ausbau: `gGate` durch die Buffer-A-Envelope aus Crystal Lights B3 ersetzen – die Portale bekommen ein *Ausatmen* statt eines harten Zufallens

---
# Anhang B: Der Weg in die App – kompakt

Der fertige Shader benutzt ausschließlich Standard-Uniforms (`iResolution`, `iTime`, mit Anhang A zusätzlich `iChannel0`) und hält die Konventionen der Vorrats-Shader in `asset/shadertoys/` ein (STELLSCHRAUBEN-Block, konstante Schleifengrenzen, keine Plattform-Extras). Die **drei Import-Wege** (Copy & Paste in den Shadertoy-Node, URL-/ID-Import mit App-Key, Browser-Panel), die **Portabilitäts-Checkliste** und das **Audio-Adapter-Muster** (`aBass()`/`aBeat()` statt roher Pegel, LumiViz-Uniforms `bass`/`mid`/`treb`/`vol`/`beat` mit Skalen-Faktor) stehen ausführlich im **Crystal-Lights-Shader-Tutorial, Anhang B** – alles dort gilt hier wörtlich und wird nicht wiederholt. Zwei Dinge sind composite-spezifisch: die Panel-Frage und die Architektur-Frage.

---

## B1 – Welche STELLSCHRAUBEN als Panel-Parameter

Die Sektions-Gliederung des Konstantenblocks ist bewusst panel-tauglich geschnitten – je Welt eine Parametergruppe plus eine gemeinsame:

| Stellschraube | Panel? | Vorschlag | Bemerkung |
|---|---|---|---|
| `TEMPO` | ja | Slider 0.2–2.5 | die EINE Uhr – der wichtigste Live-Regler, zieht ALLE Welten gemeinsam |
| `BELICHTUNG` | ja | Slider 0.5–3.0 | Tonemapping-Charakter |
| `AA` | ja | **Integer** 1–2 | Qualitätsschalter mit ehrlichem 4×-Preis (Schritt 10) |
| `P_MASSSTAB` | ja | Slider 1.0–8.0 | der magischste Regler des Shaders: Echtwelt ↔ Mikro-Diorama, live |
| `P_ATMEN` | ja | Slider 0.0–1.0 | statische ↔ blinzelnde Portale |
| `P_RAHMEN` | ja | Slider 0.0–0.15 | Rahmen aus ↔ fette Neon-Tore |
| `T_DICHTE` | ja | Slider 0.0–1.0 | Bunker ↔ Panorama (steuert zugleich die Portal-Kosten!) |
| `T_NEON`, `T_LICHT` | ja | Slider | Licht-Mischpult des Wirts |
| `K_ANTEIL` | ja | Slider 0.0–0.9 | Kristall-Rhythmus der Strecke |
| `K_DICHTE` | ja | Slider 0.0–2.0 | Klarglas ↔ Milcheis |
| `D_DICHTE`, `D_TAUMEL`, `D_GLUT` | ja | Slider | Charakter der Außenwelt |
| `T_ROEHREN`, `T_SPALTEN` | nur Integer | Int-Slider | Ganzzahl-Naht am Umfang (Quell-Tutorial Tunnel, Schritt 4) |
| `D_ZELLE`, `D_GROESSE` | **nur gekoppelt** | – | die Zellregel: unabhängig verstellt wird `D_MARGE` negativ → Wand-Artefakte. Als Paar oder gar nicht |
| `D_URSPRUNG`, `D_SONNE`, `K_HOEHE` | eher nein | – | Szenen-Identität bzw. Geometrie-Verzahnung mit dem Radius; wer sie ändert, baut einen anderen Shader |

Zwei App-Vorteile zahlen genau auf dieses Composite ein: Die **deterministische Sim-Uhr** macht die komplett zufallsfreie Choreografie (Taumel-Phasen, Portal-Atmen, Kamera) frame-genau reproduzierbar – Vergleichsbilder und Prüfstände funktionieren. Und die fertigen **Audio-Uniforms** ersetzen `bandLevel` samt Schwellen-Handarbeit; `gGate = beat` über den Adapter, fertig.

## B2 – In-Shader-Merge oder Chain-Composition? Die Abgrenzung

In LumiViz gäbe es für „Tunnel + Trümmerfeld" noch einen ganz anderen Weg: **zwei Shadertoy-Nodes in der Effect-Chain**, jeder mit einem der Original-Shader, verrechnet über den Blend-Modus des zweiten Nodes (Ersetzen/Additiv/50:50). Das ist kein Ersatz für dieses Tutorial – es ist ein **anderes Werkzeug**, und die Grenze zu kennen ist die eigentliche LumiViz-Lektion dieses Anhangs:

**Chain-Composition mischt fertige BILDER.** Der Blend sieht zwei 2D-Farbfelder und verrechnet sie pixelweise – er weiß nichts von Strahlen, Tiefe oder Fenstern. Perfekt für alles, was wirklich Bild-über-Bild ist: ein Sternenfeld-Layer über allem, Bloom/Glow-Pässe, ein Vignette-Node, ein zweiter Visualizer als Ghost-Overlay. Vorteile: kein Merge-Aufwand, die Originale bleiben unangetastet und einzeln regelbar, und die Kosten sind additiv-planbar (jeder Node rendert einmal Vollbild).

**Der In-Shader-Merge mischt WELTEN vor dem Shading.** Alles, was dieses Tutorial ausmacht, ist mit Blends prinzipiell unerreichbar: Das **Portal** braucht die Strahl-Übergabe pro Pixel (die Außenwelt muss exakt durch die Fensteröffnung der Innenwelt beschnitten und perspektivisch korrekt sein – ein Blend würde die Trümmer *über die Wand* legen). Der **Material-Mix** braucht das `min()` in der map (der Kristallboden verzahnt sich geometrisch mit der Röhre). Die **Motivkopplung** braucht Licht, das die Weltgrenze quert. Und die Kohärenz-Klammer (ein Tonemapping über alles) widerspricht dem Chain-Modell, in dem jeder Node seine eigene Politur mitbringt.

Die Faustregel zum Mitnehmen: **Sobald eine Welt die andere verdecken, beschneiden, beleuchten oder geometrisch berühren soll → In-Shader-Merge (dieses Tutorial). Wenn die Schichten einander nur überlagern → Chain-Composition, und zwar ohne schlechtes Gewissen – sie ist dann das einfachere UND bessere Werkzeug.** Mischformen sind erlaubt und üblich: das Composite hier als ein Node, plus ein Chain-Bloom darüber.

---

## Abspann

Damit ist die Reise komplett – und sie war eine andere als sonst in der Serie: kein Motiv wurde von Grund auf entwickelt, sondern drei fertige Werke wurden **verheiratet**. Die Mitgift: fünf Kondensier-Regeln, eine Präfix-Disziplin mit ehrlich benannten Fallen, die Portal-Technik (Strahl-Übergabe mit Bezahlschranke, eigener Weltrahmen, Maßstab), der (Distanz, Id)-Vertrag mit einem konservativ gezähmten Höhenfeld im min()-Verbund, pixelgenaue Kanten – und die Kohärenz-Checkliste, die aus drei Shadern ein Bild macht: eine Uhr, eine Palette, Licht über die Weltgrenze, Politur genau einmal.

Wer weitermachen will:

- **Andere Ehen stiften:** Die Techniken sind paarungs-neutral. Crystal-Lights-Terrain als Außenwelt hinter den Fenstern (Blick auf ein Eisfeld statt in den Orbit)? Der Tunnel als Diorama *in* der Debris-Welt – ein Portal rückwärts? Jede Kombination ist im Wesentlichen: kondensieren, namespacen, Vertrag wählen (Portal oder min()), Klammern ziehen.
- **Die Weichen zurückverfolgen:** Fast jeder 🎨-Kasten ist ein eigener Shader – besonders ergiebig: die Spiegel-Portale (Schritt 4), die Fenster-Galerie mit ihrem ehrlichen Kohärenz-Preis (Schritt 5), der Nur-Rahmen-Modus (Schritt 6).
- **Als Vorlage in die App:** den Endstand (oder die Lieblings-Variante) als `.lvfx` neben die Vorlagen in `asset/effectchain/shadertoys/` legen – Konvention siehe dort (`.glsl` = SSOT).

**Ehrlichkeits-Hinweis:** Wie die ganze Serie ist auch dieses Tutorial am Schreibtisch konstruiert und grob gegengerechnet (Zellregel, Boden-Sehne, Schranken-Konservativität, Budget-Überschläge), aber **nicht gerendert oder getestet**. Beim ersten Einfügen sind Tippfehler-Korrekturen und Feintuning einzuplanen – erste Verdächtige sind erfahrungsgemäß die Helligkeits-Balancen zwischen den drei Welten (`T_NEON`, `D_GLUT`, der Lampen-Faktor `0.06`, `BELICHTUNG`), die Blasen- und Schranken-Konstanten und die `fwidth`-Untergrenze. Die Architektur sollte tragen; die Zahlen sind Startwerte.

Und jetzt: Musik an. 🎵🌌









