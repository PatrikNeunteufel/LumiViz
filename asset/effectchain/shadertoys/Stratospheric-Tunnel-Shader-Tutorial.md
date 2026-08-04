# Stratospheric Tunnel – Ein Röhren-Tunnel-Flug von Grund auf

**Ziel:** Ein geraymarchter, **unendlicher Röhren-Tunnel-Flug** hoch in der Stratosphäre. Die Tunnelwand besteht aus Röhren, die um den Umfang laufen, mit Spanten-Ringen und einem Rausch-Relief darüber. In die Wand sind **Fenster** geschnitten – durch sie sieht man den Außenraum: ein Sternenfeld mit vorbeiströmenden Sternschichten und das Horizontglühen der Stratosphäre. Beleuchtet wird der Tunnel aus **vier kombinierbaren Quellen**: Neon-Streifen längs der Röhrenfugen, Ring-Lichter in Intervallen, durch die Fenster einfallendes Außenlicht und der Scheinwerfer der Kamera. Der Tunnel ist **gekrümmt** (die Achse folgt einer Pfadfunktion), der Vortrieb kehrt gelegentlich **weich um**, die Kamera legt sich in Kurven, und in regelmäßigen Abständen **gabelt** sich der Tunnel in zwei Äste – von denen die Kamera deterministisch einen wählt.

**Stil-Vorbild** (liegt im Repo unter `asset/Milkdrop3/presets/`):

- *martin – stratospheric turbulences 2.milk*: die polare Röhren-Karte des Comp-Shaders – `tuv0 = float2(.5/length(uv1), aTan2(uv1.y,-uv1.x))` macht aus dem Bild eine (Tiefe, Winkel)-Wandfläche; die **Röhrenzahl als Stellgröße** (`q6`, im `per_frame_init` gewürfelt als `tubes = pow(2,1+int(rand(3)))`); **Neon-Emission auf den Röhren** über Masken (`nshape`, `nshape2`, `nmask`); **Fenster- und Himmelsmasken** (`skym`, `skym1`, `wmask`), hinter denen ein Außenraum durchscheint; **fliegende Sterne** als radial gestreckte Schichten (die `while`-Schleife mit `dist = frac(arg)`); das **Rausch-Relief** auf den Wänden (Differenzen zweier `sampler_pw_noise_lq`-Abfragen) und die **langsam rotierenden Farb-Paletten** (`slow_roam_cos`, `rand_preset`-Farben). Alle diese Stilmittel bauen wir nach – aber nicht als 2D-Fake wie das Preset, sondern als echten 3D-Raymarcher.

Dieses Tutorial ist der direkte Nachfolger von **Crystal-Lights-Shader-Tutorial.md** (gleicher Ordner) und übernimmt dessen Schule: Stilmittel eines Milkdrop-Presets studieren, die Technik aber eigenständig und von Grund auf entwickeln.

**So funktioniert dieses Tutorial:**

- Es läuft **direkt auf Shadertoy**: Jeder Schritt ist ein vollständiger, lauffähiger Shader. Kopiere ihn nach [shadertoy.com/new](https://www.shadertoy.com/new), drücke `Alt+Enter` – fertig. Der Weg in die LumiViz-App ist Thema von Anhang B.
- Jeder Schritt fügt **genau eine Technik** hinzu; unter jedem Schritt stehen Variationsideen (🎨).
- Die Reihenfolge folgt derselben Schule wie die Vorgänger: **Geometrie → Material → Licht → Bewegung → Politur.** Erst muss die Röhre stimmen, dann kommt der Schmuck.
- Raymarching-Grundlagen (SDF, Marsch-Schleife, Normalen aus Differenzen, der `fract(sin(dot(...)))`-Hash) werden zügig behandelt – wer sie noch nie gesehen hat, liest vorher die Schritte 1–7 des Pyramid-Spiral-Tutorials oder die Schritte 3–5 von Crystal Lights. Neu ist diesmal die Königsdisziplin **Blick entlang der Geometrie**: In einem Tunnel laufen die Strahlen fast parallel zur Wand – das stellt eigene Anforderungen an Marsch und Toleranzen.

**Inhalt**

| Phase | Schritte | Thema |
|---|---|---|
| Grundgerüst | 1–3 | Polar-Skizze, Raymarch des Zylinders, Wand-Karte & Scheinwerfer |
| Wand | 4–5 | Röhren & Spanten, Rausch-Relief (FBM, Varianten) |
| Fenster & Außenraum | 6–7 | Fenstermaske, Sternenfeld & Horizontglühen |
| Licht | 8–9 | Neon-Streifen, Ring-Lichter, einfallendes Fensterlicht |
| Kamera | 10–12 | Pfad & Krümmung, Vortriebs-Umkehr & Banking, Vergabelung |
| Politur | 13 | Nebel, Farbdrift, Tonemapping, der fertige Shader |
| Anhang A | A1–A3 | Audio-Reaktivität (Bänder, Beat-Gates, Mapping-Katalog) |
| Anhang B | B1–B2 | Der Weg Shadertoy → LumiViz (kompakt, mit Verweisen) |

---

## Der Bauplan: Was wir eigentlich rendern

Bevor die erste Zeile fällt, ein Blick auf die Architektur des Bildes – sie erklärt, warum die Schritte so geordnet sind:

```
        Aussenraum: Sterne + Horizontgluehen          ← nur durch Fenster sichtbar
   ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
      ______ Wand: Roehren ║ Relief ░ Fenster ▢ _____
     /  ══ Neonfuge ══   ▢     ══ Neonfuge ══   ▢    \
    |        ◯ Ringlicht        ◯ Ringlicht           |
    |  (o)→  Kamera + Scheinwerfer, Fahrt entlang +z  |
     \______ Pfad kruemmt die Achse, die Gabel _______/
             spiegelt sie in zwei Aeste
   ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
```

Und die Wand selbst, einmal **abgerollt** – das wichtigste Denkbild des ganzen Shaders:

```
   Winkel w →   (einmal um den Umfang, 0..2π)
   ┌────┬────┬────┬────┬────┬────┐
   │Rohr│Rohr│ ▢  │Rohr│ ▢  │Rohr│   z ↓ (entlang der Achse)
   ╞════╪════╪════╪════╪════╪════╡   ══ Spant-Ring / Ringlicht
   │Rohr│ ▢  │Rohr│Rohr│Rohr│ ▢  │   │  Neonfuge (zwischen den Roehren)
   └────┴────┴────┴────┴────┴────┘   ▢  Fenster
```

Der Strahl jedes Pixels startet an der Kamera auf der Tunnelachse und läuft nach vorn, bis er die Wand trifft:

1. Trifft er **Wand**, entscheidet die abgerollte Wand-Karte `(w, z)` über alles Weitere: Röhrenprofil, Relief, Neonfugen, Ringlichter, Fenstermaske.
2. Trifft er die Wand **in einem Fenster**, fällt der Blick hindurch in den Außenraum – Sterne und Horizont.
3. Einen dritten Fall gibt es nicht: Ein Tunnel hat keinen Himmel. Was der Nebel in der Ferne verschluckt, ist trotzdem Wand.

Fast jede Eigenschaft des Bildes ist ein **Feld über der abgerollten Wandfläche** – eine Funktion `f(w, z)`: das Radius-Relief, die Fenstermaske, die Neon-Aktivität, die Ring-Intervalle. Bei Crystal Lights lagen die Felder über einer Bodenebene; hier liegen sie über einem aufgeschnittenen Zylinder. Das Denkmuster ist dasselbe – nur die Karte ist eine andere.

---

## Schritt 1 – Die Bühne: der Polar-Blick in die Röhre

**Neu:** Zentrierte UV-Koordinaten und das Polar-Mapping (1/Radius, Winkel) – der Tunnelblick als reine 2D-Skizze, noch ohne echtes 3D. Genau so „rendert" das Vorbild-Preset seinen ganzen Tunnel.

```glsl
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Ursprung in die Bildmitte, Teilen durch die HOEHE (unverzerrt)
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    float r = length(uv);            // Abstand zur Bildmitte
    float w = atan(uv.y, uv.x);      // Winkel um die Bildmitte: -pi..pi

    // Polar-Mapping wie im Vorbild: Tiefe ~ 1/Radius, Umfang ~ Winkel
    float tief0 = 0.5 / max(r, 1e-3);        // Wandtiefe dieses Pixels
    float tiefe = tief0 + iTime * 1.5;       // ... und die Fahrt nach vorn

    // 14 Roehren-Baender um den Umfang, Spanten-Ringe in der Tiefe
    float roehren = 0.5 - 0.5 * cos(w * 14.0);
    float spanten = smoothstep(0.4, 0.0, abs(fract(tiefe / 4.0) - 0.5) * 4.0);

    vec3 wandfarbe = mix(vec3(0.05, 0.06, 0.10), vec3(0.15, 0.18, 0.26), roehren);
    wandfarbe += vec3(0.25, 0.30, 0.45) * spanten * 0.5;

    // Ferne (= Bildmitte) versinkt im Dunkel
    float schleier = exp(-0.10 * tief0);

    fragColor = vec4(wandfarbe * schleier, 1.0);
}
```

**Ergebnis:** Ein Blick in eine dunkle Röhre: 14 Längsbänder laufen strahlenförmig aus der Bildmitte, Querringe strömen auf den Betrachter zu, und die Mitte verliert sich in einem schwarzen Loch. Es *fühlt* sich schon wie ein Tunnelflug an – ist aber pures 2D.

### Was passiert hier

Die zwei Polar-Größen **sind** bereits die Tunnelwand: `w` ist die Position **um den Umfang**, `1/r` die Entfernung **entlang der Achse**. Warum 1/r? Perspektive: Ein Wandpunkt in Tiefe `z` auf einem Zylinder mit Radius `R` erscheint im Bild bei Radius `r ≈ R/z` – Bildradius und Tiefe sind zueinander reziprok. Das Vorbild-Preset lebt vollständig von dieser einen Zeile seines Comp-Shaders:

```
tuv0 = float2(.5/length(uv1), aTan2(uv1.y,-uv1.x));
```

Alles, was dort nach 3D aussieht – Röhren, Fenster, Neon – sind Muster auf dieser (Tiefe, Winkel)-Karte. Wir übernehmen die **Karte**, aber nicht den Fake: Ab Schritt 2 erzeugt ein echter Raymarcher dieselbe Komposition – und kann dann Dinge, die dem 2D-Trick verwehrt bleiben (Relief mit echten Normalen, gekrümmte Achse, Vergabelungen).

Die `iTime * 1.5`-Zeile ist der Vortrieb: Sie verschiebt die Karte in der Tiefe, die Spanten-Ringe wandern nach außen – Flug nach vorn. Das `max(r, 1e-3)` fängt die Division in der exakten Bildmitte ab.

### 💡 Warum zuerst ein Fake?

Aus demselben Grund, aus dem Crystal Lights mit einer 2D-Horizontlinie beginnt: **Die Zielvorgabe festlegen, solange sie eine Handvoll Zeilen ist.** Wenn der echte Raymarcher ab Schritt 3 dieselbe Bildaufteilung liefert wie diese Skizze – Bänder strahlenförmig, Ringe konzentrisch, Mitte dunkel –, wissen wir, dass Kamera und Geometrie stimmen. Weicht er ab, ist der Fehler im Marsch, nicht im Konzept.

### 🎨 Experimentieren

- `w * 14.0` → `w * 6.0` bzw. `w * 32.0`: grobe Kanäle vs. feine Rippen – dieselbe Stellschraube, die das Preset mit `tubes` würfelt
- `iTime * 1.5` → `* 5.0`: Raserei; negativ: Rückwärtsfahrt (Vorgriff auf Schritt 11)
- Rotation dazu: `w += iTime * 0.1;` → der Tunnel dreht sich um die Blickachse
- `schleier = exp(-0.03 * tief0);` → viel tiefere Sicht in die Röhre

---

## Schritt 2 – Raymarch: der echte Zylinder

**Neu:** Ray Origin auf der Tunnelachse, Ray Direction nach vorn – und die Marsch-Schleife über die Innen-SDF `RADIUS - length(p.xy)`. Ab jetzt ist der Tunnel ein Körper im Raum.

```glsl
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float RADIUS = 1.0;    // Grundradius des Tunnels

// Innen-Abstand zur Tunnelwand: positiv im Inneren, 0 auf der Wand
float mapTunnel(vec3 p)
{
    return RADIUS - length(p.xy);
}

// Marsch: laeuft am Strahl entlang, bis er die Wand beruehrt
float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;   // aufgesetzt -> Treffer
        t += d;                              // exakter Zylinder: voller Schritt
        if (t > 60.0) break;                 // tief genug -> aufgeben
    }
    return min(t, 60.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 2.0);   // Kamera auf der Achse, fliegt +z
    vec3 rd = normalize(vec3(uv, 1.4));      // 1.4 = Brennweite (groesser = Tele)

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    // Tiefe als Helligkeit + z-Streifen, damit die Fahrt sichtbar ist
    float streifen = 0.6 + 0.4 * cos(p.z * 3.0);
    vec3 color = vec3(streifen) * exp(-0.10 * t);

    fragColor = vec4(color, 1.0);
}
```

**Ergebnis:** Ein grauer, echter Tunnel: Querstreifen rasen vorbei, die Röhre verliert sich vorn im Dunkel. Die Komposition ist exakt die aus Schritt 1 – strahlige Wand, konzentrische Ringe, dunkle Mitte –, aber jetzt aus einem 3D-Marsch.

### Was passiert hier

**Die SDF steht Kopf.** Bei Terrain und Tunnel-Außenansichten ist die Distanzfunktion außen positiv; wir sitzen aber **im** Körper. Darum die Innen-Konvention: `RADIUS - length(p.xy)` ist positiv im Tunnelinneren, null auf der Wand, negativ dahinter. Für den nackten Zylinder ist das sogar die *exakte* Distanz – der nächste Wandpunkt liegt immer radial. Deshalb darf der Marsch den **vollen** Schritt `t += d` gehen und konvergiert schnell.

**Die wachsende Trefftoleranz** `0.0015 + 0.001 * t` ist im Tunnel wichtiger als je zuvor: Die Strahlen der Bildmitte laufen fast **parallel zur Wand** – ihr Wandabstand schrumpft nur langsam, sie brauchen viele kleine Schritte. In der Ferne genügt „ungefähr aufgesetzt" (ein Pixel deckt dort meterweise Wand ab); die Toleranz erlöst genau diese Strahlen und verhindert das typische Ring-Flimmern in der Tunneltiefe.

**Kein Himmel-Fall:** `march` gibt immer ein `t` zurück – notfalls das gekappte `60.0`. In einem Tunnel trifft jeder Strahl irgendwann Wand; was jenseits der 60 Einheiten liegt, wird ohnehin der Nebel schlucken (Schritt 13). Das erspart uns den kompletten „kein Treffer"-Zweig, den Terrain-Shader brauchen.

### 💡 Warum überhaupt marschieren?

Den Schnitt Strahl↔Zylinder gäbe es in geschlossener Form – eine Quadratik, zwei Zeilen, kein Marsch. Aber schon im übernächsten Schritt bekommt der Radius ein Relief `r(w, z)`, und spätestens beim gekrümmten Pfad (Schritt 10) ist die Analytik endgültig chancenlos. Der Marsch ist die Investition, die alle späteren Umbauten trägt – dieselbe Abwägung wie beim Höhenfeld von Crystal Lights.

### 🎨 Experimentieren

- Brennweite `1.4` → `0.8` (Weitwinkel: die Röhre reißt auf) bzw. `2.5` (Tele: endloser Schlund)
- `ro.xy = vec2(0.4, 0.0);` → Kamera aus der Achse: die Wand kommt einseitig nah – Vorgeschmack auf die Kurvenfahrt
- Iterationen `120` → `40` und die Bildmitte beobachten: die Grenz-Strahlen geben zu früh auf (Ringe brechen ab) – der Grund für das großzügige Budget
- `exp(-0.10 * t)` → `exp(-0.04 * t)`: tiefere Sicht; die Kappung bei 60 wird sichtbar

---

## Schritt 3 – Die Wand-Karte und der Scheinwerfer

**Neu:** Die abgerollte Wandfläche `(w, z)` als Schachbrett sichtbar gemacht, die analytische Wand-Normale – und die erste der vier Lichtquellen: der **Scheinwerfer der Kamera** als Punktlicht mit Abstands-Abfall.

```glsl
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float RADIUS = 1.0;
const float TAU    = 6.28318530;

float mapTunnel(vec3 p)
{
    return RADIUS - length(p.xy);
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d;
        if (t > 60.0) break;
    }
    return min(t, 60.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 2.0);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    float w  = atan(p.y, p.x);        // Winkel um die Achse: -pi..pi
    float wu = w / TAU + 0.5;         // 0..1 einmal um den Umfang

    // Schachbrett auf der abgerollten Wandflaeche (Winkel, z)
    float schach = mod(floor(wu * 14.0) + floor(p.z * 1.5), 2.0);
    vec3 basis = mix(vec3(0.05, 0.06, 0.10), vec3(0.16, 0.19, 0.27), schach);

    // Normale des nackten Zylinders: zeigt von der Wand zur Achse
    vec3 n = normalize(vec3(-p.xy, 0.0));

    // SCHEINWERFER: Punktlicht an der Kamera, 1/d^2-artiger Abfall
    vec3 zk = ro - p;                        // vom Wandpunkt zur Kamera
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 color = basis * (1.6 * dif / (1.0 + dk * dk * 0.12) + 0.02);

    fragColor = vec4(color, 1.0);
}
```

**Ergebnis:** Ein Schachbrett-Tunnel, der von der Kamera aus **angeleuchtet** wird: nah ist die Wand hell, nach vorn läuft das Licht in natürliches Dunkel aus. Kein künstlicher Schleier mehr – die Tiefe entsteht aus der Lichtphysik.

### Was passiert hier

**Die Wand-Karte** ist ab jetzt offiziell: `w = atan(p.y, p.x)` (Umfang) und `p.z` (Achse) sind die zwei Koordinaten, über denen *alle* späteren Felder leben – Relief, Fenster, Neon, Ringe. Das Schachbrett ist ihr Prüfbild, das Tunnel-Pendant zum Boden-Schachbrett von Crystal Lights: Verzerrungen, Nähte oder falsche Maßstäbe wären hier sofort sichtbar. Wichtig: Die Felderzahl um den Umfang (`14`) ist **ganzzahlig** – nur dann schließt sich das Muster an der Naht `w = ±π` nahtlos.

**Die Normale** ist beim nackten Zylinder geschenkt: Sie zeigt radial von der Wand zur Achse, also `normalize(vec3(-p.xy, 0))`. (Ab Schritt 4 übernimmt eine numerische Normale – aber es lohnt, einmal gesehen zu haben, wie billig der Idealfall ist.)

**Der Scheinwerfer** ist Lichtquelle Nr. 1 und die Grundbeleuchtung des ganzen Shaders: ein Punktlicht am Kameraort. `dif` ist Lambert-Kosinus, `1/(1 + d²·0.12)` der gebremste quadratische Abfall (die `1.0` verhindert die Singularität an der nahen Wand). Eine hübsche Nebenrechnung: Für die Achsen-Kamera ist `dot(n, zk/dk)` exakt `RADIUS/dk` – das Streiflicht wird mit der Entfernung *doppelt* schwächer, einmal über den Kosinus, einmal über den Abfall. Genau daher kommt das satte, natürliche Auslaufen ins Dunkel, für das Schritt 1 noch einen künstlichen `exp`-Schleier brauchte.

### 🎨 Experimentieren

- Debug-Klassiker: `color = n * 0.5 + 0.5;` → die Normalen als Farbe (ein sauberer Farbkreis um den Umfang)
- Scheinwerfer-Stärke `1.6` → `4.0`: Flutlicht; `0.5`: klaustrophobisches Funzellicht
- Restlicht `0.02` → `0.10`: die Ferne bleibt lesbar (auf Kosten der Stimmung)
- Warmes Licht: `1.6 * dif * vec3(1.0, 0.85, 0.6)` → Halogen statt LED

---

## Schritt 4 – Röhren und Spanten: die Wand bekommt Profil

**Neu:** Der Radius wird eine Funktion der Wand-Karte – **Röhren um den Umfang** (`mod`-Wiederholung auf dem Winkel, cos-Profil) und **Spanten-Ringe** entlang z. Dazu die numerische Normale und die Marsch-Drossel, die das Relief erzwingt. Der erste STELLSCHRAUBEN-Block.

```glsl
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS        = 1.0;    // Grundradius des Tunnels
const float ROEHREN       = 14.0;   // Roehren um den Umfang (ganzzahlig!)
const float ROEHREN_TIEFE = 0.10;   // Woelbung der Roehren
const float SPANT_ABSTAND = 4.0;    // Abstand der Spanten-Ringe (z)
const float SPANT_TIEFE   = 0.05;   // Hoehe der Spanten
// ----------------------------------------------------------------------------

const float TAU = 6.28318530;

// Relief: wie weit die Wand am Ort (w, z) in den Tunnel hineinragt
float wandRelief(float w, float z)
{
    // Roehren: weiches cos-Profil, Fugen bei w*ROEHREN = 2*pi*k
    float roehre = ROEHREN_TIEFE * (0.5 - 0.5 * cos(w * ROEHREN));

    // Spanten: schmale erhabene Ringe in festen z-Intervallen
    float sz    = abs(fract(z / SPANT_ABSTAND) - 0.5) * SPANT_ABSTAND;
    float spant = SPANT_TIEFE * smoothstep(0.35, 0.0, sz);

    return roehre + spant;
}

float mapTunnel(vec3 p)
{
    float w = atan(p.y, p.x);
    return RADIUS - wandRelief(w, p.z) - length(p.xy);
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d * 0.7;                 // Drossel: das Relief macht d zur Schaetzung
        if (t > 60.0) break;
    }
    return min(t, 60.0);
}

// Normale aus zentralen Differenzen der SDF
vec3 wandNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(mapTunnel(p + e.xyy) - mapTunnel(p - e.xyy),
                          mapTunnel(p + e.yxy) - mapTunnel(p - e.yxy),
                          mapTunnel(p + e.yyx) - mapTunnel(p - e.yyx)));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 2.0);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    float w  = atan(p.y, p.x);
    float wu = w / TAU + 0.5;

    float schach = mod(floor(wu * ROEHREN) + floor(p.z * 1.5), 2.0);
    vec3 basis = mix(vec3(0.05, 0.06, 0.10), vec3(0.16, 0.19, 0.27), schach);

    vec3 n = wandNormale(p);

    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 color = basis * (1.6 * dif / (1.0 + dk * dk * 0.12) + 0.02);

    fragColor = vec4(color, 1.0);
}
```

**Ergebnis:** Aus dem glatten Rohr wird ein **Röhrenbündel**: 14 gewölbte Bahnen laufen längs in die Tiefe, zwischen ihnen liegen Fugen im Schatten, und in regelmäßigen Abständen ziehen sich Spanten-Ringe quer über alle Röhren. Der Scheinwerfer modelliert die Wölbungen plastisch – die Fugen fangen kaum Licht, die Röhrenkämme glänzen.

### Was passiert hier

**Die Wiederholung um den Umfang ist ein `mod` auf dem Winkel** – nur versteckt: `cos(w * ROEHREN)` ist periodisch mit `2π/ROEHREN`, wiederholt das Profil also ROEHREN-mal um den Umfang, ohne dass man `mod` hinschreiben muss. Das Vorbild macht exakt dasselbe mit `ftu = frac(tuv.y*tubes/2)-0.5` und würfelt seine Röhrenzahl im `per_frame_init` (`tubes = pow(2,1+int(rand(3)))` → 4, 8 oder 16); wir machen die Zufallsgröße zur **Stellschraube** `ROEHREN`.

**Warum ein cos-Profil und kein Halbkreis?** Ein echtes Halbrohr (`sqrt(1-x²)`-Profil) hätte an den Fugen **senkrechte Flanken** – dort wird die Steigung unendlich, die SDF unterschätzt die wahre Distanz drastisch, und der Marsch frisst sich durch die Kanten. Das cos-Profil hat eine begrenzte maximale Steigung (`ROEHREN_TIEFE·ROEHREN/(2·RADIUS) = 0.7` in Bogenlänge – unter 1, also beherrschbar). Wer den Halbkreis trotzdem will: 🎨 unten, mit Warnung.

**Die Drossel `0.7`** ist der Preis des Reliefs: `RADIUS - relief - length` ist keine exakte Distanz mehr, sondern eine radiale Schätzung, die schräge Reliefflanken unterschätzt. 70 % Schrittweite plus das großzügige Iterationsbudget aus Schritt 2 fangen das ab – dieselbe Tempo-gegen-Sicherheit-Abwägung wie die 0.4-Drossel des Terrain-Marsches bei Crystal Lights (wir dürfen aggressiver sein, weil unser Relief flacher ist als dortige Klippen).

**Die numerische Normale** ersetzt die analytische: sechs `mapTunnel`-Aufrufe, zentrale Differenzen. Die Vorzeichen-Frage lohnt einen Blick: Unsere SDF ist **innen positiv**, ihr Gradient zeigt von der Wand weg ins Innere – also genau zur Kamera. Die Differenzen-Normale steht damit automatisch richtig herum, ohne das übliche Umdrehen.

🧠 **Merke:** `ROEHREN` (und später `FENSTER_SPALTEN`) müssen **ganzzahlig** bleiben. Der Winkel `w` springt bei ±π – nur wenn das Muster dort exakt eine Periode vollendet, ist die Naht unsichtbar. Jede Nachkommastelle wird eine sichtbare Kante längs des Tunnels.

### 🎨 Experimentieren

- `ROEHREN = 8.0` (das Vorbild-Minimum) bzw. `24.0` – Charakter von „U-Bahn-Röhre" bis „Kabelstrang"
- Der Halbkreis-Test: `float ft = fract(w * ROEHREN / TAU) - 0.5; float roehre = ROEHREN_TIEFE * sqrt(max(1.0 - 4.0 * ft * ft, 0.0));` → echte Halbrohre. Dazu Drossel auf `0.5` senken – und trotzdem die Kanten-Artefakte einmal bewusst ansehen
- `SPANT_TIEFE = 0.0` → glatte Röhren; `0.12` → die Ringe werden zu Schotten
- Spanten schräg: `sz`-Berechnung mit `z / SPANT_ABSTAND + wu` → Wendel-Spanten (Schraubentunnel)

---

## Schritt 5 – Rausch-Relief: die Wand wird organisch

**Neu:** Die Werkzeugkette Hash → Value-Noise → FBM, ein **nahtfreies** Rausch-Relief über der Wand-Karte – und das Schachbrett weicht dem endgültigen Material (mattes Metall mit Noise-Textur).

```glsl
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS        = 1.0;    // Grundradius des Tunnels
const float ROEHREN       = 14.0;   // Roehren um den Umfang (ganzzahlig!)
const float ROEHREN_TIEFE = 0.10;   // Woelbung der Roehren
const float SPANT_ABSTAND = 4.0;    // Abstand der Spanten-Ringe (z)
const float SPANT_TIEFE   = 0.05;   // Hoehe der Spanten
const float RELIEF        = 0.05;   // organisches FBM-Relief
// ----------------------------------------------------------------------------

const float TAU = 6.28318530;

// Hash: Gitterpunkt -> deterministische "Zufallszahl" 0..1
float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

// Value-Noise: weich interpolierte Zufallswerte auf einem Einheitsgitter
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(i),              hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)), hash21(i + vec2(1, 1)), u.x), u.y);
}

// Fraktales Rauschen: vier Oktaven genuegen fuer Wandstruktur
float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

float wandRelief(float w, float z)
{
    float roehre = ROEHREN_TIEFE * (0.5 - 0.5 * cos(w * ROEHREN));

    float sz    = abs(fract(z / SPANT_ABSTAND) - 0.5) * SPANT_ABSTAND;
    float spant = SPANT_TIEFE * smoothstep(0.35, 0.0, sz);

    // NEU: organisches Relief - nahtfrei, weil der Winkel als (cos, sin) eingeht
    float n = fbm(vec2(cos(w) * 1.6 + z * 0.45, sin(w) * 1.6 + z * 0.31));

    return roehre + spant + RELIEF * (n - 0.5) * 2.0;
}

float mapTunnel(vec3 p)
{
    float w = atan(p.y, p.x);
    return RADIUS - wandRelief(w, p.z) - length(p.xy);
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d * 0.7;
        if (t > 60.0) break;
    }
    return min(t, 60.0);
}

vec3 wandNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(mapTunnel(p + e.xyy) - mapTunnel(p - e.xyy),
                          mapTunnel(p + e.yxy) - mapTunnel(p - e.yxy),
                          mapTunnel(p + e.yyx) - mapTunnel(p - e.yyx)));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 2.0);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    float w = atan(p.y, p.x);

    // NEU: Grundmaterial - kuehles, mattes Metall mit Noise-Textur
    float tex = fbm(vec2(cos(w) * 3.1 + p.z * 0.9, sin(w) * 3.1 + p.z * 0.63));
    vec3 basis = mix(vec3(0.045, 0.055, 0.085), vec3(0.10, 0.12, 0.17), tex);

    vec3 n = wandNormale(p);

    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 color = basis * (1.6 * dif / (1.0 + dk * dk * 0.12) + 0.02);

    fragColor = vec4(color, 1.0);
}
```

**Ergebnis:** Die Röhren sehen nicht mehr gedreht, sondern **gebaut und gealtert** aus: Beulen und Dellen brechen das Streiflicht des Scheinwerfers in unregelmäßige Glanzflecken, die Textur gibt der Wand Patina. Das Schachbrett hat ausgedient.

### Was passiert hier

Die Kette **Hash → Noise → FBM** ist wörtlich die von Crystal Lights (Schritt 4 dort erklärt sie ausführlich) – nur die Karte darunter ist neu. Vier Oktaven statt fünf genügen: Wandstruktur braucht weniger Größenordnungen als eine Landschaft, und `wandRelief` läuft pro Pixel viele Male im Marsch.

Das Vorbild-Preset erzeugt sein Wand-Rauschen übrigens verwandt und doch anders: Es zieht **zwei versetzte Noise-Abfragen voneinander ab** (`tex2D(...,tuv*sc) - tex2D(...,(tuv+0.003)*sc)*0.75`) – die Differenz wirkt wie eine beleuchtete Prägung, ganz ohne Normalen. Wir haben echte Normalen, also darf das Rauschen direkt in die **Geometrie**: Der Scheinwerfer erledigt die Prägung physikalisch.

### 💡 Warum (cos w, sin w) statt des Winkels selbst?

`fbm(vec2(w, z))` wäre die naheliegende Zeile – und hätte eine hässliche **Naht**: Bei `w = ±π` springt der Winkel um 2π, das Rauschen reißt dort sichtbar ab (eine scharfe Linie längs des ganzen Tunnels). Der Trick: Statt des Winkels gehen `cos(w)` und `sin(w)` ins Rauschen ein – die beiden sind auf dem Kreis **stetig**, die Naht verschwindet prinzipbedingt. Anschaulich tastet das FBM nicht die abgerollte Wand ab, sondern einen Kreisring, der mit `z` durchs Rauschfeld wandert. (Die Röhren und Spanten haben das Problem nicht – sie sind als periodische Funktionen konstruiert; das gilt aber nur, solange `ROEHREN` ganzzahlig bleibt, siehe Schritt 4.)

### 🎨 Experimentieren

- `RELIEF = 0.0` → der Neubau-Tunnel; `0.12` → Tropfsteinhöhle (Drossel im Auge behalten!)
- Frequenz `1.6` → `4.0`: feiner Rostfraß statt Beulen
- **Platten via Voronoi** (Relief-Variante 3, Bausteine aus Crystal Lights Schritt 6): `hash22` und `voronoi` von dort übernehmen und im Relief ersetzen: `vec3 vo = voronoi(vec2(cos(w), sin(w)) * 2.5 + vec2(0.0, z * 0.8)); float n = hash21(vo.yz);` → jede Wandplatte bekommt einen eigenen Versatz, aus Beulen werden verschraubte Paneele mit Kanten
- Relief nur auf den Röhrenkämmen: `RELIEF * (n - 0.5) * 2.0 * (roehre / ROEHREN_TIEFE)` → die Fugen bleiben sauber, wie frisch verfugt

---

## Schritt 6 – Fenster: Löcher in der Wand

**Neu:** Eine **Fenstermaske** auf der abgerollten Wandfläche – ein Gitter aus Zellen um Umfang und Achse, in dem nur ein Teil der Zellen ein Fenster trägt, mit weichem Rand und optionalem **dynamischem Öffnen/Schließen**. Der Blick fällt vorerst auf ein stumpfes Blau – der echte Außenraum kommt in Schritt 7.

*Ab jetzt zeigen die Schritte nur noch die geänderten bzw. neuen Funktionen – alles andere bleibt wörtlich wie im vorherigen Schritt stehen. (Am Ende von Schritt 13 steht der komplette Shader noch einmal am Stück.)*

```glsl
// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float FENSTER_SPALTEN = 6.0;   // Fensterspalten um den Umfang (ganzzahlig!)
const float FENSTER_ABSTAND = 5.0;   // Fensterabstand entlang z
const float FENSTER_DICHTE  = 0.55;  // Anteil der Zellen mit Fenster (0..1)
const float FENSTER_DYN     = 0.35;  // dynamisches Oeffnen/Schliessen (0 = statisch)
// ----------------------------------------------------------------------------

// NEU: 2D-Hash mit 2D-Ergebnis (kommt in Schritt 7 fuer die Sterne dazu)
vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

// NEU: Fenstermaske - 1 = Oeffnung, 0 = Wand
float fensterMask(float w, float z)
{
    float wu = fract(w / TAU + 0.5);              // 0..1 um den Umfang (nahtfrei)
    vec2 zelle = vec2(wu * FENSTER_SPALTEN, z / FENSTER_ABSTAND);
    vec2 id = floor(zelle);

    // nicht jede Zelle hat ein Fenster
    if (hash21(id + 3.1) > FENSTER_DICHTE) return 0.0;

    // Zellkoordinaten in Wand-Einheiten (damit Fenster nicht verzerren)
    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * RADIUS / FENSTER_SPALTEN;
    c.y *= FENSTER_ABSTAND;

    // Oeffnungsgrad: Grundwert + langsames Atmen mit eigener Phase je Fenster
    float o = 0.8 + FENSTER_DYN *
              sin(iTime * (0.15 + 0.25 * hash21(id + 9.4)) + TAU * hash21(id));
    o = clamp(o, 0.0, 1.0);

    // hochkantiges Rechteckfenster mit weichem Rand
    vec2 halb = vec2(0.30, 0.85) * o;
    float d = max(abs(c.x) - halb.x, abs(c.y) - halb.y);
    return smoothstep(0.06, -0.06, d);
}
```

Und in `mainImage` bekommt der Wandpunkt seine zweite Option:

```glsl
    vec3 color = basis * (1.6 * dif / (1.0 + dk * dk * 0.12) + 0.02);

    // NEU: Fenster - Loch in der Wand, dahinter (vorerst) stumpfes Nachtblau
    float F = fensterMask(w, p.z);
    color = mix(color, vec3(0.01, 0.02, 0.05), F);
```

**Ergebnis:** In die Röhrenwand sind hochkantige Fenster geschnitten – unregelmäßig verteilt (manche Zellen bleiben Wand), mit weich auslaufendem Rand. Mit `FENSTER_DYN > 0` atmen sie: Einzelne Fenster ziehen sich langsam zu Schlitzen zusammen und öffnen sich wieder, jedes in eigenem Takt.

### Was passiert hier

**Die Maske ist das dritte Feld über der Wand-Karte** (nach Röhrenprofil und Relief) – und das erste mit Zell-Identität: `floor` liefert die Zellnummer, `hash21(id)` daran beliebig viele feste Eigenschaften je Fenster (Existenz, Phase, Tempo). Das ist wörtlich das Lampen-Muster aus Crystal Lights Schritt 9, nur auf einem Zylinder statt einer Ebene. Das Vorbild-Preset schneidet seine Fenster verwandt: `skym`/`skym1` sind Sinus-Masken auf der Röhren-Karte, hinter denen der Himmels-Layer durchscheint – unsere Version ist das Gitter-Pendant mit echter Zellkontrolle.

Zwei technische Entscheidungen verdienen den Blick:

1. **`fract(w/TAU + 0.5)` vor der Zellteilung** – die Naht-Versicherung. Der rohe Winkel würde bei ±π die Zell-Ids kippen (Spalte −3 vs. +3 wären *verschiedene* Hashes für denselben Wandstreifen). Nach dem `fract` läuft die Spaltenzählung sauber 0..5 einmal herum. Der Zusatznutzen kommt in Schritt 9: Die Funktion verträgt dann auch verschobene Winkel wie `w + π`.
2. **Zellkoordinaten in Wand-Einheiten:** `c.x` wird mit dem Bogenmaß der Spalte (`2πR/SPALTEN ≈ 1.05`) skaliert, `c.y` mit `FENSTER_ABSTAND`. Erst dadurch ist ein „0.30 breit, 0.85 hoch"-Fenster wirklich hochkant – auf der nackten Zellkoordinate wären Breite und Höhe nicht vergleichbar, und jede Änderung an `FENSTER_ABSTAND` würde die Fensterform mitverzerren.

**Das Loch selbst ist eine Shading-Entscheidung, keine Geometrie:** Der Strahl trifft die Wand ganz normal, und erst die Maske blendet am Treffer zur Außenwelt über. Die Wand ist damit **papierdünn** – eine bewusste Vereinfachung: Echte Fensterlaibungen bräuchten eine SDF-Subtraktion mit allem Marsch-Aufwand dahinter. Bei den schmalen Fenstern und dem streifenden Blick fällt die fehlende Dicke schlicht nicht auf (und der weiche `smoothstep`-Rand kaschiert den Rest als „Rahmen").

### 🎨 Experimentieren

- `FENSTER_DICHTE = 1.0` → Panorama-Röhre; `0.15` → seltene, kostbare Ausblicke
- **Rundfenster** (Bullaugen): `float d = length(c * vec2(1.0, 0.45)) - 0.42 * o;` statt der Rechteck-Zeilen
- Harte Stanzkante: `smoothstep(0.015, -0.015, d)` → Nietblech-Ästhetik
- Fenster nur in den Fugen: `if (0.5 - 0.5 * cos(w * ROEHREN) > 0.3) return 0.0;` vor dem Dichte-Test → die Röhren bleiben geschlossen, nur die Zwischenbahnen sind verglast
- `FENSTER_DYN = 0.0` und dafür `o = 0.8 + 0.2 * sin(z * 0.1);` → Öffnungsgrad als Orts- statt Zeitfunktion: Fenster-„Wellen" entlang der Strecke

---

## Schritt 7 – Der Außenraum: Sterne und Horizontglühen

**Neu:** Die Welt hinter den Fenstern – ein Stratosphären-Himmel aus drei Zutaten: Farbverlauf mit **Horizontglühen**, statische Sterne und **fliegende Sternschichten** nach dem `dist = frac(arg)`-Idiom des Vorbilds.

```glsl
// NEU: der Blick nach draussen - haengt nur von der Blickrichtung ab
vec3 aussenRaum(vec3 rd)
{
    float h = rd.y;

    // Grundverlauf: tiefes Nachtblau, oben fast schwarz
    vec3 col = mix(vec3(0.03, 0.05, 0.11), vec3(0.005, 0.01, 0.03),
                   clamp(h * 2.0 + 0.5, 0.0, 1.0));

    // Horizontgluehen der Stratosphaere: breites Violett + schmale Orange-Linie
    col += vec3(0.30, 0.14, 0.34) * exp(-abs(h + 0.12) * 8.0);
    col += vec3(0.95, 0.45, 0.15) * exp(-abs(h + 0.15) * 40.0) * 0.7;

    // statische Sterne: Hash auf der Blickrichtung, nur oberhalb des Dunstes
    vec2 su = rd.xy / (abs(rd.z) + 0.4);
    float s = hash21(floor(su * 48.0));
    col += vec3(0.9) * smoothstep(0.994, 1.0, s) * clamp(h * 3.0 + 0.5, 0.0, 1.0);

    // fliegende Sterne: drei radial stroemende Schichten (dist = fract(arg))
    for (int i = 0; i < 3; i++) {
        float dist = fract(float(i) / 3.0 - iTime * 0.06);
        vec2 uv4 = su * (1.5 + 26.0 * dist) + hash22(vec2(float(i), 3.7)) * 37.0;
        float sn = hash21(floor(uv4));
        col += vec3(0.55, 0.70, 1.00) * (1.0 - dist)
             * smoothstep(0.90, 1.0, sn)
             * smoothstep(0.32, 0.0, length(fract(uv4) - 0.5));
    }
    return col;
}
```

In `mainImage` ersetzt der Aufruf das stumpfe Blau:

```glsl
    float F = fensterMask(w, p.z);
    color = mix(color, aussenRaum(rd), F);
```

**Ergebnis:** Hinter den Fenstern liegt jetzt die Stratosphäre: unten ein violett-oranges Glühen am Horizont (die Erdkrümmung bei Nacht), darüber Sterne – und zwischen ihnen strömen hellblaue Sternschichten radial nach außen, als flöge man mit hoher Geschwindigkeit an ihnen vorbei.

### Was passiert hier

**Das Horizontglühen** sind zwei `exp(-|h|·k)`-Glocken um eine leicht abgesenkte Horizontlinie (`h + 0.12`): eine breite violette (die Dämmerungsschicht der Hochatmosphäre) und eine schmale orange (die Restsonne am Erdrand). Weil `aussenRaum` nur von `rd` abhängt, liegt der Himmel „im Unendlichen" – jedes Fenster zeigt denselben Horizont aus seinem Blickwinkel, und wenn die Kamera sich später neigt und rollt (Schritt 11), kippt der Horizont physikalisch korrekt mit.

**Die fliegenden Sterne sind das Herzstück des Vorbild-Zitats.** Der Comp-Shader des Presets baut seinen Flug-Effekt so:

```
while (n <= sanz) {
  float arg = (1.0*n/sanz - t0);
  float dist = frac(arg);
  float2 uv4 = 4*64 * dist * uv1;   // Schicht skaliert mit dist
  ...
```

Jede Schicht hat eine Phase `dist`, die mit der Zeit von 1 nach 0 läuft und dabei die Bildkoordinaten skaliert: Ein Stern auf festem Gitterplatz erscheint bei kleiner werdender Skalierung **weiter außen** – er fliegt radial aus der Bildmitte, wird dabei heller (`1 - dist`) und springt am Zyklusende unauffällig zurück in die Mitte (dort ist er dunkel und winzig). Unsere GLSL-Fassung übersetzt die `while`-Schleife in ein konstantes `for` (Shadertoy-Regel: Schleifengrenzen konstant), hasht je Schicht einen Versatz (`hash22(...) * 37.0`), damit die drei Schichten nicht dasselbe Sternmuster zeigen, und zeichnet jeden Stern als weichen Punkt in seiner Gitterzelle (`length(fract(uv4) - 0.5)`).

**`su = rd.xy / (abs(rd.z) + 0.4)`** projiziert die Blickrichtung auf eine Ebene vor der Kamera – der Standardweg, aus einem Richtungsvektor stabile 2D-Koordinaten für Sternfelder zu machen. Das `+ 0.4` verhindert die Explosion am Rand des Blickfelds.

### 🎨 Experimentieren

- Schichtenzahl `3` → `6` und Tempo `0.06` → `0.15`: Warp-Geschwindigkeit
- Das Glühen einfärben: Violett gegen `vec3(0.10, 0.35, 0.30)` tauschen → Polarlicht-Stimmung
- Sterne farbig würfeln: `vec3 sfarbe = 0.6 + 0.4 * hash22(floor(uv4)).xyx;` und damit multiplizieren
- `dist`-Kurve schärfen: `(1.0 - dist)` → `pow(1.0 - dist, 3.0)` – die Sterne „zünden" erst kurz vor dem Vorbeiflug

---

## Schritt 8 – Neon-Streifen: Licht in den Fugen

**Neu:** Lichtquelle Nr. 2 – **Neon-Streifen längs der Röhrenfugen** mit `1/d²`-Charakter, per Hash nur auf einem Teil der Fugen aktiv, mit langsamem Flackern und einer **rotierenden Farbpalette** (das `slow_roam_cos`-Erbe). Dazu wandert das Wand-Shading in eine eigene Funktion.

```glsl
// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float NEON_ANTEIL  = 0.45;   // Anteil beleuchteter Fugen (0..1)
const float NEON_STAERKE = 0.012;  // Helligkeit der Neon-Streifen
const float SCHEINWERFER = 1.6;    // Kamera-Scheinwerfer (ersetzt die 1.6 inline)
// ----------------------------------------------------------------------------

// NEU: Cosinus-Palette - kraeftige, langsam rotierende Farben
vec3 pal(float t)
{
    return 0.5 + 0.5 * cos(TAU * (t + vec3(0.0, 0.33, 0.67)));
}

// NEU: Neon-Emission am Wandort (w, z)
vec3 neonEmission(float w, float z)
{
    float fu  = w * ROEHREN / TAU;                 // Fugen bei ganzzahligem fu
    float gid = mod(floor(fu + 0.5), ROEHREN);     // Fugen-Index (nahtfrei)
    float dg  = abs(fract(fu + 0.5) - 0.5);        // Abstand zur Fuge (0..0.5)
    float ad  = dg * TAU * RADIUS / ROEHREN;       // ... in Wand-Einheiten

    // nur ein Teil der Fugen traegt Neon; dazu langsames Flackern je Fuge
    float aktiv = step(hash21(vec2(gid, 1.3)), NEON_ANTEIL);
    aktiv *= 0.6 + 0.4 * sin(iTime * (0.4 + 0.7 * hash21(vec2(gid, 8.2)))
                             + TAU * hash21(vec2(gid, 4.4)));

    // Farbe je Fuge aus der Palette, die Palette selbst rotiert langsam
    vec3 farbe = pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55 + iTime * 0.015);

    // laengs unterbrochene Baender (Masken-Zitat: nicht ueberall leuchtet es)
    float band = 0.35 + 0.65 * smoothstep(0.25, 0.6, vnoise(vec2(gid * 7.3, z * 0.25)));

    return farbe * aktiv * band * NEON_STAERKE / (0.0015 + ad * ad * 60.0);
}

// NEU: das komplette Wand-Material (ersetzt das Shading in mainImage)
vec3 shadeWand(vec3 p, vec3 ro, vec3 n, float w)
{
    float tex = fbm(vec2(cos(w) * 3.1 + p.z * 0.9, sin(w) * 3.1 + p.z * 0.63));
    vec3 basis = mix(vec3(0.045, 0.055, 0.085), vec3(0.10, 0.12, 0.17), tex);

    // (1) Scheinwerfer der Kamera
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 col = basis * (SCHEINWERFER * dif / (1.0 + dk * dk * 0.12) + 0.02);

    // (2) Neon-Streifen in den Fugen (Emission)
    col += neonEmission(w, p.z);

    return col;
}
```

In `mainImage` schrumpft der Wand-Teil auf:

```glsl
    vec3 n = wandNormale(p);
    vec3 color = shadeWand(p, ro, n, w);

    float F = fensterMask(w, p.z);
    color = mix(color, aussenRaum(rd), F);
```

**Ergebnis:** Etwa jede zweite Röhrenfuge führt jetzt einen farbigen Neon-Streifen in die Tiefe – heißer Kern, weicher Glüh-Saum quer über die Nachbarröhren. Die Streifen flackern gemächlich, sind längs immer wieder unterbrochen, und ihre Farben wandern über Minuten durch die Palette.

### Was passiert hier

**Der `1/d²`-Streifen ist das Linien-Pendant zur Punktlampe.** `ad` misst den Quer-Abstand zur nächsten Fuge in Wand-Einheiten, und `1/(0.0015 + ad²·60)` macht daraus das Abstandsgesetz mit gekappter Singularität: Bei `ad = 0` glüht der Kern mit dem Faktor ~8 (weit über Weiß – das Tonemapping in Schritt 13 fängt ihn zu einem sauberen Ausglühen ein, wie bei den Crystal-Lights-Lampen), zwei Zentimeter daneben bleibt ein sanfter Farbschimmer auf der Röhrenwölbung. Das ist Emission, kein beleuchtetes Material – darum wird sie *addiert*, unabhängig vom Scheinwerfer.

**Die Masken-Logik zitiert das Vorbild.** Dessen Neon entsteht als Produkt von Masken auf der Röhren-Karte: `nshape` (Querprofil der Röhre), `nshape2` (welcher Streifen), `nmask` (an/aus-Muster, das mit `q20`/`q21` durchgeschaltet wird). Unsere Übersetzung: `aktiv` (Hash-Auswahl der Fugen + Flackern), `band` (Noise-Unterbrechungen längs), und das Querprofil liefert der `1/d²`-Abfall gleich mit. **Die Zell-Identität `gid`** braucht dieselbe Naht-Sorgfalt wie die Fenster: `floor(fu + 0.5)` zählt die Fugen, `mod(..., ROEHREN)` klappt den Index bei ±π zusammen – Fuge 7 und Fuge −7 sind dieselbe Fuge und bekommen denselben Hash.

**Die Palette** ist die Cosinus-Palette (drei phasenversetzte Kanäle), und ihr langsamer Offset `iTime * 0.015` ist die deterministische Fassung von `slow_roam_cos` – im Vorbild mischen träge wandernde Zufalls-Cosinusse die Preset-Farben (`ocol = normalize(slow_roam_cos + bcol)`), bei uns dreht die Palette mit fester, sehr niedriger Rate. Der `* 0.4 + 0.55`-Bereich hält die Fugenfarben in einer Familie statt im vollen Regenbogen.

### 🎨 Experimentieren

- `NEON_ANTEIL = 1.0` → Las-Vegas-Röhre; `0.15` → einzelne Leitlinien im Dunkel
- Schärfe `60.0` → `300.0`: haarfeine Laserlinien; `15.0`: breite Lichtbahnen
- Farbfamilie verschieben: `* 0.4 + 0.55` → `* 0.2 + 0.0` (Rot-Orange) oder `+ 0.6` (Cyan-Blau)
- Flackern hart machen: die `0.6 + 0.4*sin(...)`-Zeile durch `step(0.3, fract(iTime * (0.5 + hash21(vec2(gid, 8.2)))))` ersetzen → defekte Leuchtstoffröhren
- Neon spiegeln lassen: in `shadeWand` zusätzlich `col += neonEmission(w + TAU / ROEHREN * 0.5, p.z) * basis * 2.0;` → die Nachbarröhre fängt einen Reflex (Fake, aber wirksam)

---

## Schritt 9 – Ring-Lichter und einfallendes Fensterlicht

**Neu:** Die Lichtquellen Nr. 3 und 4 – pulsierende **Ring-Lichter** in festen z-Intervallen (mit eigenem Blink-Charakter je Ring) und das **Außenlicht**, das durch das jeweils gegenüberliegende Fenster auf die Wand fällt.

```glsl
// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float RING_ABSTAND = 9.0;    // Abstand der Ring-Lichter (z)
const float RING_STAERKE = 0.35;   // Helligkeit der Ring-Lichter
const float FENSTERLICHT = 0.8;    // einfallendes Aussenlicht
// ----------------------------------------------------------------------------

// NEU: Puls-Kurve je Ring - meist gedimmt, gelegentlich weiches Aufleuchten
float ringPuls(float id)
{
    float ph = hash21(vec2(id, 31.7));                // eigene Phase
    float sp = 0.4 + 0.8 * hash21(vec2(id, 17.3));    // eigenes Tempo
    float wv = 0.5 + 0.5 * sin(TAU * (iTime * sp * 0.20 + ph));
    return smoothstep(0.55, 0.95, wv) * (0.3 + 0.7 * hash21(vec2(id, 5.1)));
}

// NEU: Ring-Emission am Wandort z (haengt nur von z ab - ein voller Lichtring)
vec3 ringEmission(float z)
{
    float rz = z / RING_ABSTAND;
    float id = floor(rz + 0.5);                       // Index des naechsten Rings
    float dz = (fract(rz + 0.5) - 0.5) * RING_ABSTAND; // Abstand zu ihm (z)

    vec3 farbe = pal(hash21(vec2(id, 2.7)) * 0.5 + iTime * 0.01);
    return farbe * ringPuls(id) * RING_STAERKE / (0.03 + dz * dz * 14.0);
}

// GEAENDERT: shadeWand bekommt die Quellen (3) und (4)
vec3 shadeWand(vec3 p, vec3 ro, vec3 n, float w)
{
    float tex = fbm(vec2(cos(w) * 3.1 + p.z * 0.9, sin(w) * 3.1 + p.z * 0.63));
    vec3 basis = mix(vec3(0.045, 0.055, 0.085), vec3(0.10, 0.12, 0.17), tex);

    // (1) Scheinwerfer der Kamera
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 col = basis * (SCHEINWERFER * dif / (1.0 + dk * dk * 0.12) + 0.02);

    // (2) Neon-Streifen in den Fugen
    col += neonEmission(w, p.z);

    // (3) Ring-Lichter in Intervallen
    col += ringEmission(p.z);

    // (4) Aussenlicht durch das GEGENUEBERLIEGENDE Fenster
    float einfall = fensterMask(w + 3.14159265, p.z);
    col += vec3(0.30, 0.38, 0.60) * FENSTERLICHT * einfall;

    return col;
}
```

**Ergebnis:** In der Tiefe glimmen jetzt volle Lichtringe quer über alle Röhren – jeder in eigener Farbe, jeder mit eigenem Puls: weiches Anschwellen, Verweilen, Abklingen, nie zwei im Takt. Und wo ein Fenster in der Wand sitzt, liegt auf der **gegenüberliegenden** Wandseite ein kühlblauer Lichtfleck in Fensterform – das Sternenlicht fällt herein.

### Was passiert hier

**Die Ringe sind das z-Pendant zu den Neon-Fugen** – dieselbe Dreifaltigkeit aus Zell-Index (`floor(rz + 0.5)`), Abstand (`dz`) und `1/d²`-Abfall, nur um 90° gedreht: Die Neonfugen wiederholen sich um den *Umfang*, die Ringe entlang der *Achse*. Wer beide Funktionen nebeneinander legt, sieht das gemeinsame Skelett – ein gutes Beispiel dafür, wie wenige Grundmuster ein Shader dieser Art wirklich braucht. `ringPuls` ist die Blink-Dramaturgie der Crystal-Lights-Lampen (smoothstep schneidet die Spitzen einer je-Zelle-phasenverschobenen Sinuswelle heraus), mit weicheren Schwellen: Ringe sind Architektur, keine Glühwürmchen – sie dürfen länger verweilen.

**Das Fensterlicht ist der Trick des Schritts.** Physikalisch korrekt wäre: vom Wandpunkt in Richtung aller Fenster strahlen, prüfen, was durchkommt – ein Schattenstrahl-Marsch, viel zu teuer. Die Abkürzung nutzt die Geometrie der Röhre: Licht, das durch ein Fenster fällt, landet (bei annähernd radialem Einfall) auf der **diametral gegenüberliegenden** Wandseite. Also fragen wir am Wandpunkt einfach die Fenstermaske bei `w + π` ab – ist *gegenüber* ein Fenster, empfängt *diese* Stelle Außenlicht, in Form und Weichheit des Fensters gratis inbegriffen. Eine Zeile, und die Lichtflecken wandern sogar korrekt mit, wenn die Fenster sich dynamisch schließen (die Maske ist ja dieselbe). Hier zahlt sich das `fract` aus Schritt 6 aus: `fensterMask` verkraftet den verschobenen Winkel anstandslos.

*(Bewusste Vereinfachung: Der Einfallswinkel wird ignoriert – der Fleck ist immer fenstersenkrecht. Und Licht durch* schräg *gegenüberliegende Fenster gibt es nicht. Beides fällt im bewegten Bild nicht auf; wer mehr will, summiert die Maske über mehrere Winkel-Offsets – siehe 🎨.)*

### 💡 Warum vier getrennte Lichtquellen?

Weil sie vier verschiedene **Rhythmen** ins Bild bringen: Der Scheinwerfer ist konstant und kamera-gebunden (Nähe), die Neonfugen sind orts-fest und flackern schnell (Textur), die Ringe pulsieren langsam (Takt der Strecke), das Fensterlicht kommt und geht mit den Fenstern (Zufall der Architektur). Ein einzelnes „Tunnellicht" – egal wie hübsch – hätte genau einen Rhythmus. Die Mischung ist es, die den Flug lebendig macht, lange bevor in Anhang A die Musik dazukommt; dort bekommt dann jede Quelle ihr eigenes Audio-Band.

### 🎨 Experimentieren

- `RING_ABSTAND = 4.0` → gleich `SPANT_ABSTAND`: jeder Spant leuchtet (die Ringe rasten optisch auf den Spanten ein); `18.0` → seltene Tore
- Ringe als Lauflicht: in `ringPuls` die Phase `ph` durch `id * 0.25` ersetzen → der Puls läuft geordnet von Ring zu Ring in die Tiefe
- Fensterlicht golden: `vec3(0.30, 0.38, 0.60)` → `vec3(0.9, 0.6, 0.25)` (tief stehende Sonne hinter den Fenstern)
- Breiterer Einfall: `float einfall = 0.6 * fensterMask(w + 3.14159265, p.z) + 0.2 * fensterMask(w + 2.6, p.z) + 0.2 * fensterMask(w + 3.68, p.z);` → weichere, glaubwürdigere Lichtteppiche
- Ringe schatten die Fugen: `col += ringEmission(p.z) * (0.5 + 0.5 * basis * 4.0);` → die Ringfarbe modelliert die Wandtextur mit

---

## Schritt 10 – Der Pfad: der Tunnel macht Kurven

**Neu:** Eine **Pfadfunktion** `pfad(z)` verschiebt das Tunnelzentrum mit der Tiefe – im SDF kostet die Krümmung genau eine Zeile (`p.xy -= pfad(p.z)`). Die Kamera fährt auf dem Pfad und **blickt voraus auf ihn**, mit einer richtigen Kamera-Basis.

```glsl
// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float KURVE = 0.8;    // seitliche Auslenkung des Pfads
const float TEMPO = 1.0;    // Gesamttempo der Fahrt
// ----------------------------------------------------------------------------

// NEU: das Tunnelzentrum am Ort z - zwei inkommensurable Sinus-Wellen
vec2 pfad(float z)
{
    return vec2(sin(z * 0.18), sin(z * 0.121)) * KURVE;
}

// GEAENDERT: der Tunnel folgt dem Pfad
float mapTunnel(vec3 p)
{
    vec2 q = p.xy - pfad(p.z);        // ins Pfad-System wechseln
    float w = atan(q.y, q.x);
    return RADIUS - wandRelief(w, p.z) - length(q);
}

// NEU: ersetzt die zwei Kamera-Zeilen in mainImage
void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zpos = iTime * TEMPO * 2.0;

    ro = vec3(pfad(zpos), zpos);      // Kamera sitzt AUF dem Pfad

    // Blickpunkt: ein Stueck voraus auf dem Pfad
    float za = zpos + 2.0;
    vec3 ta = vec3(pfad(za), za);
    vec3 fw = normalize(ta - ro);

    // Kamera-Basis: Rechts/Hoch per Kreuzprodukt
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.4 + rt * uv.x + up * uv.y);
}
```

Und in `mainImage` müssen die Wand-Koordinaten **dieselbe Verschiebung** machen wie das SDF:

```glsl
    vec3 ro, rd;
    kamera(uv, ro, rd);

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    // Wand-Koordinaten im Pfad-System (dieselbe Verschiebung wie mapTunnel!)
    vec2 q = p.xy - pfad(p.z);
    float w = atan(q.y, q.x);
```

**Ergebnis:** Der Tunnel **schlängelt sich**: Kurven tauchen vorn auf, die Wand kommt in der Kurveninnenseite näher, Neonfugen und Fensterreihen biegen sich sichtbar mit. Die Kamera nimmt jede Kurve sauber, weil sie nicht stur nach +z schaut, sondern dem Pfad entgegen.

### Was passiert hier

**Die eine Zeile `q = p.xy - pfad(p.z)`** ist der klassische Tunnel-Krümmungs-Trick: Statt den Zylinder zu verbiegen (schwer), verbiegen wir den **Raum**, in dem er gerade bleibt. Jede z-Scheibe des Raums wird um `pfad(z)` verschoben, der Zylinder bleibt in diesem verschobenen System ein perfekter Kreis – Relief, Fenster, Fugen: alles funktioniert unverändert weiter. Der Preis ist derselbe wie beim Relief: Die SDF ist keine exakte Distanz mehr (die Verschiebung ändert sich mit z, was die radiale Schätzung verfälscht). Die maximale Pfad-Steigung ist `KURVE · 0.18 ≈ 0.14` – gutmütig genug, dass die 0.7-Drossel aus Schritt 4 weiter reicht. Wer `KURVE` aggressiv erhöht, senkt die Drossel mit.

**Der Blick voraus** ist die Minimal-Choreografie: Zielpunkt `ta` = Pfadpunkt zwei Einheiten weiter vorn, `fw = normalize(ta - ro)`. Damit lenkt die Kamera **vor** der Kurve ein, genau wie ein Fahrer, der in den Kurvenausgang schaut – und die Röhre bleibt zentriert im Bild statt an den Rand zu wandern. Die Basis-Konstruktion (`rt`/`up` per Kreuzprodukt gegen die Welt-Hochachse) ist wörtlich die aus Crystal Lights Schritt 12.

**Die zwei Frequenzen** `0.18` und `0.121` sind inkommensurabel (keine ist ein Vielfaches der anderen) – die Kurvenfolge wiederholt sich praktisch nie, obwohl sie streng deterministisch ist. Das ist dieselbe Fünf-Uhren-Lektion wie bei der Crystal-Lights-Kamerafahrt, hier auf den *Raum* statt auf die *Zeit* angewandt.

🧠 **Merke:** Wer das SDF-Koordinatensystem verschiebt, muss **jede** Stelle nachziehen, die Weltkoordinaten in Wand-Koordinaten übersetzt – bei uns genau eine: die `w`-Berechnung in `mainImage`. Vergisst man sie, marschiert der Strahl gegen den gekrümmten Tunnel, aber Fenster und Neon rechnen mit dem geraden – die Muster „schwimmen" dann bei jeder Kurve über die Wand. (Die numerische Normale braucht keine Pflege: Sie differenziert `mapTunnel` und bekommt die Krümmung gratis mit.)

### 🎨 Experimentieren

- `KURVE = 0.0` → der Ur-Tunnel (Regressionstest: alles muss aussehen wie in Schritt 9)
- `KURVE = 1.6` mit Drossel `0.5`: Serpentinen – die Wand füllt in engen Kurven das halbe Bild
- Blickweite `zpos + 2.0` → `+ 6.0`: die Kamera schneidet Kurven wie ein Rennfahrer; `+ 0.5`: nervöses Nachlenken
- Höhenprofil dazu: `pfad` um eine dritte Welle ergänzen ist nicht möglich (sie liefert nur xy) – aber `vec2(sin(z * 0.18), sin(z * 0.121) + 0.3 * sin(z * 0.061))` überlagert der y-Achse eine langsame Berg-und-Tal-Fahrt

---

## Schritt 11 – Vortrieb mit Umkehr und Banking

**Neu:** Die Fahrt wird Choreografie – der Vortrieb bekommt einen Sinus-Anteil und kehrt **weich um** (sin-Position ⇒ cos-Geschwindigkeit, die Lektion aus der Crystal-Lights-Kamerafahrt), und die Kamera **legt sich in die Kurven**: Banking aus der Pfad-Krümmung plus ein langsamer Eigen-Roll.

```glsl
// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float ROLL = 1.2;    // Banking-Staerke in Kurven
// ----------------------------------------------------------------------------

// GEAENDERT: die Kamera bekommt ihre Choreografie
void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zt = iTime * TEMPO;

    // VORTRIEB: Grundtempo + Sinus-Anteil.
    // Geschwindigkeit = 1.1 + 1.4*cos(zt*0.2) => wird zeitweise negativ:
    // die Fahrt bremst weich ab, setzt kurz zurueck, zieht wieder an
    float zpos = zt * 1.1 + sin(zt * 0.20) * 7.0;

    ro = vec3(pfad(zpos), zpos);

    float za = zpos + 2.0;
    vec3 ta = vec3(pfad(za), za);
    vec3 fw = normalize(ta - ro);

    // BANKING: seitliche Pfad-Aenderung kippt die Kamera in die Kurve,
    // dazu ein langsamer Eigen-Roll als Ballett-Anteil
    float kruemm = pfad(zpos + 1.5).x - pfad(zpos - 1.5).x;
    float roll = -kruemm * ROLL + 0.12 * sin(zt * 0.13);

    // die "Hoch"-Richtung wird um roll gekippt - der Rest bleibt gleich
    vec3 up0 = vec3(sin(roll), cos(roll), 0.0);
    vec3 rt = normalize(cross(up0, fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.4 + rt * uv.x + up * uv.y);
}
```

**Ergebnis:** Die Fahrt lebt: Sie beschleunigt, gleitet, wird langsamer, **hängt einen Atemzug in der Schwebe und rollt ein Stück rückwärts** – die Ringe kommen einem entgegen statt entgegenzufliegen –, dann zieht der Vortrieb wieder an. In jeder Kurve kippt der Horizont der Fenster, weil die Kamera sich in die Kurve legt; dazwischen pendelt ein kaum merklicher Eigen-Roll.

### Was passiert hier

**Die Umkehr ist die spinAngle-Lektion, auf den Vortrieb übertragen.** Nicht die Geschwindigkeit vorgeben und aufintegrieren (ein Shader hat kein Gedächtnis zwischen Frames!), sondern die **Position als glatte Funktion der Zeit** schreiben: `zpos = 1.1·zt + 7·sin(0.2·zt)`. Die Ableitung ist `1.1 + 1.4·cos(0.2·zt)` – sie pendelt zwischen −0.3 und +2.5. Weil der Sinus-Hub (`7·0.2 = 1.4`) das Grundtempo (`1.1`) **übersteigt**, wird die Geschwindigkeit periodisch leicht negativ: eine echte, weiche Richtungsumkehr mit Null-Durchgang, ohne Zustand, ohne Ruck. Wer die Umkehr abschalten will, senkt die Amplitude unter `1.1/0.2 = 5.5` – dann bleibt ein An- und Abschwellen der Fahrt.

**Eine bewusste Design-Entscheidung:** Der **Blick** bleibt bei der Umkehr nach vorn (`za = zpos + 2.0`, fest positiv) – man *rollt rückwärts*, statt sich umzudrehen. Das ist kein Zufall, sondern Robustheit: Ein Blickpunkt, der mit der Geschwindigkeit skaliert (`za = zpos + 2·v`), fällt beim Null-Durchgang mit `ro` zusammen – `normalize(ta - ro)` teilt durch fast null, die Blickrichtung degeneriert und das Bild springt. Die 🎨-Variante unten zeigt den halb-mitschwenkenden Kompromiss – mit genau dieser Warnung.

**Das Banking** liest die Kurve direkt aus dem Pfad: `pfad(z+1.5).x - pfad(z-1.5).x` ist (bis auf einen Faktor) die seitliche Geschwindigkeit des Tunnelzentrums – groß in Linkskurven, negativ in Rechtskurven. Mit `-ROLL` multipliziert kippt die Kamera zur Kurveninnenseite, wie ein Flugzeug. Umgesetzt wird der Roll **nicht** durch Drehen des Bildes, sondern durch Kippen der `up0`-Referenz, aus der die Kamera-Basis gebaut wird – dadurch rollen Blickrichtung, Fenster-Horizont und Sternenfeld konsistent mit (der Außenraum hängt nur von `rd` ab und bekommt den Roll gratis).

### 🎨 Experimentieren

- `TEMPO = 0.4` → Meditationsfahrt; `2.0` → Achterbahn (die Umkehr wird zum Looping-Gefühl)
- Umkehr-Charakter: `sin(zt * 0.20) * 7.0` → `* 0.08 und * 18.0`: seltene, lange Rückwärtspassagen
- Der mitschwenkende Blick (mit Degenerations-Warnung von oben): `float v = clamp(1.1 + 1.4 * cos(zt * 0.20), -1.0, 1.0); float za = zpos + 2.0 * v + 0.3 * sign(v + 0.001);` – der Zusatzterm hält `ta` von `ro` weg; beobachte, wie die Kamera sich bei der Umkehr halb umsieht
- `ROLL = 0.0` → Gimbal-Fahrt (steril, sofort spürbar); `3.0` → Kunstflug
- Eigen-Roll `0.12` → `0.5`: der Tunnel „taumelt" – zusammen mit `ROEHREN = 8` ein sehr psychedelischer Modus

---

## Schritt 12 – Vergabelungen: der Tunnel teilt sich

**Neu:** Periodisch entlang der Strecke gabelt sich der Tunnel in **zwei Äste** – über eine Spiegel-Faltung `abs(x)` plus auseinanderlaufende Pfade. Die Kamera wählt je Gabelung **deterministisch per Hash** einen Ast und fährt hindurch.

```glsl
// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float GABEL_PERIODE = 40.0;   // Abstand der Vergabelungen (z)
const float GABEL_WEITE   = 1.6;    // maximale Ast-Auslenkung
// ----------------------------------------------------------------------------

// NEU: wie weit die beiden Aeste am Ort z auseinanderliegen (0 = ein Tunnel)
float gabel(float z)
{
    float zz = fract(z / GABEL_PERIODE);
    return GABEL_WEITE * smoothstep(0.12, 0.38, zz)
                       * (1.0 - smoothstep(0.62, 0.88, zz));
}

// NEU: welchen Ast die Kamera in dieser Gabel-Zelle nimmt (-1 oder +1)
float gabelSeite(float z)
{
    return hash21(vec2(floor(z / GABEL_PERIODE), 5.2)) < 0.5 ? -1.0 : 1.0;
}

// NEU: der Kamera-Pfad = Haupt-Pfad + gewaehlter Ast
vec2 kameraPfad(float z)
{
    return pfad(z) + vec2(gabelSeite(z) * gabel(z), 0.0);
}

// GEAENDERT: die Spiegel-Faltung im SDF - eine Zeile macht zwei Tunnel
float mapTunnel(vec3 p)
{
    vec2 q = p.xy - pfad(p.z);
    q.x = abs(q.x) - gabel(p.z);      // Faltung: Aeste bei x = +-gabel(z)
    float w = atan(q.y, q.x);
    return RADIUS - wandRelief(w, p.z) - length(q);
}
```

In `kamera` ersetzen beide `pfad`-Aufrufe ihren Nachfolger (`ro`, `ta` **und** das Banking rechnen jetzt mit `kameraPfad`), und in `mainImage` macht die Wand-Koordinate dieselbe Faltung mit:

```glsl
    // in kamera():
    ro = vec3(kameraPfad(zpos), zpos);
    ...
    vec3 ta = vec3(kameraPfad(za), za);
    ...
    float kruemm = kameraPfad(zpos + 1.5).x - kameraPfad(zpos - 1.5).x;

    // in mainImage():
    vec2 q = p.xy - pfad(p.z);
    q.x = abs(q.x) - gabel(p.z);      // dieselbe Faltung wie mapTunnel
    float w = atan(q.y, q.x);
```

**Ergebnis:** Alle 40 Einheiten öffnet sich die Röhre: Voraus erscheint eine zweite Tunnelmündung, die Wand zwischen beiden wächst zu einem Mittelgrat, die Äste laufen auseinander – und die Kamera zieht in einen der beiden hinein (mal links, mal rechts, je Gabelung fest gewürfelt). Kurz darauf laufen die Äste wieder zusammen, und die Fahrt geht in einem Tunnel weiter.

### Was passiert hier

**Die Faltung ist die billigste Vergabelung der Welt.** Konzeptionell ist ein gegabelter Tunnel die **Vereinigung zweier Tunnel-SDFs** – bei unserer Innen-Konvention wäre das `max(d1, d2)` (bei der üblichen Außen-Konvention `min`, wie im Konzept vermerkt: zwei SDFs per min kombiniert). Die Zeile `q.x = abs(q.x) - gabel(z)` rechnet exakt dasselbe mit **einer einzigen** Auswertung: `abs` spiegelt die negative x-Halbwelt auf die positive, dort steht ein Tunnel bei `x = gabel(z)` – aus Sicht der ungefalteten Welt sind das zwei Tunnel bei `±gabel(z)`. Relief, Fenster, Neon: alles läuft unverändert durch, weil es hinter der Faltung von `(w, z)` lebt.

**Das Gabel-Profil** `smoothstep(0.12, 0.38, zz) · (1 - smoothstep(0.62, 0.88, zz))` ist ein weiches Fenster über der Gabel-Zelle: Am Zellanfang und -ende ist `gabel = 0` (ein Tunnel), in der Mitte `GABEL_WEITE` (mit `1.6` liegen die Achsen 3.2 auseinander – mehr als zwei Tunneldurchmesser, die Äste sind vollständig getrennt). Weil die Auslenkung an den Zellgrenzen exakt null ist, darf `gabelSeite` dort **springen** – der Kamera-Pfad bleibt trotzdem stetig: Der Seitenwechsel passiert immer im Moment, in dem beide Seiten zusammenfallen. Das ist die ganze Magie hinter „die Kamera wählt einen Ast": ein Hash pro Zelle, ein Vorzeichen, keine Verzweigungslogik.

### 💡 Was die Faltung kann – und was nicht (ehrliche Grenzen)

- **Sie kann:** zwei Äste, beliebig weit auseinander, wieder zusammenlaufend, für den Preis von einer SDF-Auswertung; Marsch, Normalen, Wandfelder unverändert.
- **Die Äste sind exakte Spiegelbilder.** Relief, Fenster und Neon des linken Asts sind die gespiegelte Kopie des rechten – man kann *nie* in den anderen Ast schauen und dort etwas anderes sehen. Im Vorbeiflug fällt das nicht auf (man sieht den zweiten Ast nur kurz und von außen); wer echte Individualität will, braucht zwei getrennte SDFs mit eigenem `w`-Feld pro Ast – doppelter Aufwand im Marsch **und** im Shading.
- **Der Übergang ist keine modellierte Y-Geometrie.** Wo die Interieurs sich trennen, durchdringen sich schlicht zwei Zylinder; die Schnittkante ist ein scharfer Grat, keine gebaute Weiche mit Laibung. Das cos-Röhrenprofil macht den Grat gutmütig, aber wer genau hinsieht, sieht Konstruktion statt Architektur.
- **Die Spiegelachse ist fest** (die x-Achse des Pfad-Systems): Die Gabel öffnet immer seitlich, nie nach oben/unten. Variante: vor der Faltung `q = R(winkel) * q` drehen – dann gabelt es schräg (🎨).
- **Der Mittelgrat halbiert die Fenster:** Ein Fenster, das im ungefalteten `w`-Feld genau auf der Naht `x < 0 / x > 0` liegt, erscheint an beiden Ästen je zur Hälfte. Praktisch unsichtbar, aber wer es stört, legt `FENSTER_SPALTEN`-Grenzen auf die Faltachse.

### 🎨 Experimentieren

- `GABEL_WEITE = 0.9` → die Äste trennen sich nie ganz: ein „Doppellauf"-Tunnel mit durchgehendem Mittelgrat (Blick in den Nachbarlauf!)
- `GABEL_PERIODE = 16.0` → Weichenfeld im Rangierbahnhof-Takt
- Schräge Gabeln: vor der Faltung `q = R(0.6) * q;` (und dieselbe Drehung in `mainImage`) – die Äste öffnen diagonal
- Die Wahl sichtbar machen: `basis *= 1.0 + 0.3 * gabelSeite(p.z) * sign(p.x - pfad(p.z).x);` als Debug – der gewählte Ast wird heller
- Dreier-Gabel (Grenzen ausloten): zusätzlich `q.y = abs(q.y) - gabel(p.z) * 0.5;` → vier gespiegelte Äste; man sieht sofort, warum echte n-fach-Weichen andere Werkzeuge brauchen

---

## Schritt 13 – Politur: Nebel, Farbdrift, Tonemapping – der fertige Shader

**Neu:** Die Veredelung – **Distanznebel** (der im Tunnel auch die Marsch-Kappung versteckt), Fenster-Ausblendung in der Tiefe, die langsame **Farbdrift** und das Tonemapping `1 − exp(−x)` samt Gamma und Vignette – die direkte Verwandtschaft zu Crystal Lights Schritt 14 (und dahinter zu frosty caves' `ret = 1-exp(-ret)`). Danach steht der komplette Shader – hier als **Gesamtlisting** zum Einfügen.

```glsl
// ============================================================================
// "Stratospheric Tunnel" - Roehren-Tunnel-Flug in der Stratosphaere
// Endstand des Tutorials (Schritt 13). Braucht keine iChannels.
// Stil-Verwandtschaft: martin - stratospheric turbulences 2 (Polar-Roehren,
// Neon-Masken, Fenster-/Himmelsmasken, fliegende Sterne, langsame Paletten).
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS          = 1.0;    // Grundradius des Tunnels
const float ROEHREN         = 14.0;   // Roehren um den Umfang (ganzzahlig!)
const float ROEHREN_TIEFE   = 0.10;   // Woelbung der Roehren
const float SPANT_ABSTAND   = 4.0;    // Abstand der Spanten-Ringe (z)
const float SPANT_TIEFE     = 0.05;   // Hoehe der Spanten
const float RELIEF          = 0.05;   // organisches FBM-Relief
const float FENSTER_SPALTEN = 6.0;    // Fensterspalten um den Umfang (ganzzahlig!)
const float FENSTER_ABSTAND = 5.0;    // Fensterabstand entlang z
const float FENSTER_DICHTE  = 0.55;   // Anteil der Zellen mit Fenster (0..1)
const float FENSTER_DYN     = 0.35;   // dynamisches Oeffnen/Schliessen (0 = statisch)
const float NEON_ANTEIL     = 0.45;   // Anteil beleuchteter Fugen (0..1)
const float NEON_STAERKE    = 0.012;  // Helligkeit der Neon-Streifen
const float RING_ABSTAND    = 9.0;    // Abstand der Ring-Lichter (z)
const float RING_STAERKE    = 0.35;   // Helligkeit der Ring-Lichter
const float SCHEINWERFER    = 1.6;    // Kamera-Scheinwerfer
const float FENSTERLICHT    = 0.8;    // einfallendes Aussenlicht
const float KURVE           = 0.8;    // seitliche Auslenkung des Pfads
const float GABEL_PERIODE   = 40.0;   // Abstand der Vergabelungen (z)
const float GABEL_WEITE     = 1.6;    // maximale Ast-Auslenkung
const float TEMPO           = 1.0;    // Gesamttempo der Fahrt
const float ROLL            = 1.2;    // Banking-Staerke in Kurven
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float TAU = 6.28318530;

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
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

vec3 pal(float t)
{
    return 0.5 + 0.5 * cos(TAU * (t + vec3(0.0, 0.33, 0.67)));
}

// ---- Pfad & Vergabelung ----------------------------------------------------

vec2 pfad(float z)
{
    return vec2(sin(z * 0.18), sin(z * 0.121)) * KURVE;
}

float gabel(float z)
{
    float zz = fract(z / GABEL_PERIODE);
    return GABEL_WEITE * smoothstep(0.12, 0.38, zz)
                       * (1.0 - smoothstep(0.62, 0.88, zz));
}

float gabelSeite(float z)
{
    return hash21(vec2(floor(z / GABEL_PERIODE), 5.2)) < 0.5 ? -1.0 : 1.0;
}

vec2 kameraPfad(float z)
{
    return pfad(z) + vec2(gabelSeite(z) * gabel(z), 0.0);
}

// ---- Wand-Felder (alle leben auf der abgerollten Karte (w, z)) -------------

float wandRelief(float w, float z)
{
    float roehre = ROEHREN_TIEFE * (0.5 - 0.5 * cos(w * ROEHREN));

    float sz    = abs(fract(z / SPANT_ABSTAND) - 0.5) * SPANT_ABSTAND;
    float spant = SPANT_TIEFE * smoothstep(0.35, 0.0, sz);

    float n = fbm(vec2(cos(w) * 1.6 + z * 0.45, sin(w) * 1.6 + z * 0.31));

    return roehre + spant + RELIEF * (n - 0.5) * 2.0;
}

float fensterMask(float w, float z)
{
    float wu = fract(w / TAU + 0.5);
    vec2 zelle = vec2(wu * FENSTER_SPALTEN, z / FENSTER_ABSTAND);
    vec2 id = floor(zelle);

    if (hash21(id + 3.1) > FENSTER_DICHTE) return 0.0;

    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * RADIUS / FENSTER_SPALTEN;
    c.y *= FENSTER_ABSTAND;

    float o = 0.8 + FENSTER_DYN *
              sin(iTime * (0.15 + 0.25 * hash21(id + 9.4)) + TAU * hash21(id));
    o = clamp(o, 0.0, 1.0);

    vec2 halb = vec2(0.30, 0.85) * o;
    float d = max(abs(c.x) - halb.x, abs(c.y) - halb.y);
    return smoothstep(0.06, -0.06, d);
}

// ---- Geometrie -------------------------------------------------------------

float mapTunnel(vec3 p)
{
    vec2 q = p.xy - pfad(p.z);
    q.x = abs(q.x) - gabel(p.z);
    float w = atan(q.y, q.x);
    return RADIUS - wandRelief(w, p.z) - length(q);
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d * 0.7;
        if (t > 60.0) break;
    }
    return min(t, 60.0);
}

vec3 wandNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(mapTunnel(p + e.xyy) - mapTunnel(p - e.xyy),
                          mapTunnel(p + e.yxy) - mapTunnel(p - e.yxy),
                          mapTunnel(p + e.yyx) - mapTunnel(p - e.yyx)));
}

// ---- Aussenraum ------------------------------------------------------------

vec3 aussenRaum(vec3 rd)
{
    float h = rd.y;

    vec3 col = mix(vec3(0.03, 0.05, 0.11), vec3(0.005, 0.01, 0.03),
                   clamp(h * 2.0 + 0.5, 0.0, 1.0));

    col += vec3(0.30, 0.14, 0.34) * exp(-abs(h + 0.12) * 8.0);
    col += vec3(0.95, 0.45, 0.15) * exp(-abs(h + 0.15) * 40.0) * 0.7;

    vec2 su = rd.xy / (abs(rd.z) + 0.4);
    float s = hash21(floor(su * 48.0));
    col += vec3(0.9) * smoothstep(0.994, 1.0, s) * clamp(h * 3.0 + 0.5, 0.0, 1.0);

    for (int i = 0; i < 3; i++) {
        float dist = fract(float(i) / 3.0 - iTime * 0.06);
        vec2 uv4 = su * (1.5 + 26.0 * dist) + hash22(vec2(float(i), 3.7)) * 37.0;
        float sn = hash21(floor(uv4));
        col += vec3(0.55, 0.70, 1.00) * (1.0 - dist)
             * smoothstep(0.90, 1.0, sn)
             * smoothstep(0.32, 0.0, length(fract(uv4) - 0.5));
    }
    return col;
}

// ---- Lichtquellen ----------------------------------------------------------

vec3 neonEmission(float w, float z)
{
    float fu  = w * ROEHREN / TAU;
    float gid = mod(floor(fu + 0.5), ROEHREN);
    float dg  = abs(fract(fu + 0.5) - 0.5);
    float ad  = dg * TAU * RADIUS / ROEHREN;

    float aktiv = step(hash21(vec2(gid, 1.3)), NEON_ANTEIL);
    aktiv *= 0.6 + 0.4 * sin(iTime * (0.4 + 0.7 * hash21(vec2(gid, 8.2)))
                             + TAU * hash21(vec2(gid, 4.4)));

    vec3 farbe = pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55 + iTime * 0.015);

    float band = 0.35 + 0.65 * smoothstep(0.25, 0.6, vnoise(vec2(gid * 7.3, z * 0.25)));

    return farbe * aktiv * band * NEON_STAERKE / (0.0015 + ad * ad * 60.0);
}

float ringPuls(float id)
{
    float ph = hash21(vec2(id, 31.7));
    float sp = 0.4 + 0.8 * hash21(vec2(id, 17.3));
    float wv = 0.5 + 0.5 * sin(TAU * (iTime * sp * 0.20 + ph));
    return smoothstep(0.55, 0.95, wv) * (0.3 + 0.7 * hash21(vec2(id, 5.1)));
}

vec3 ringEmission(float z)
{
    float rz = z / RING_ABSTAND;
    float id = floor(rz + 0.5);
    float dz = (fract(rz + 0.5) - 0.5) * RING_ABSTAND;

    vec3 farbe = pal(hash21(vec2(id, 2.7)) * 0.5 + iTime * 0.01);
    return farbe * ringPuls(id) * RING_STAERKE / (0.03 + dz * dz * 14.0);
}

// ---- Wand-Material ---------------------------------------------------------

vec3 shadeWand(vec3 p, vec3 ro, vec3 n, float w)
{
    float tex = fbm(vec2(cos(w) * 3.1 + p.z * 0.9, sin(w) * 3.1 + p.z * 0.63));
    vec3 basis = mix(vec3(0.045, 0.055, 0.085), vec3(0.10, 0.12, 0.17), tex);

    // (1) Scheinwerfer der Kamera
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 col = basis * (SCHEINWERFER * dif / (1.0 + dk * dk * 0.12) + 0.02);

    // (2) Neon-Streifen in den Fugen
    col += neonEmission(w, p.z);

    // (3) Ring-Lichter in Intervallen
    col += ringEmission(p.z);

    // (4) Aussenlicht durch das gegenueberliegende Fenster
    float einfall = fensterMask(w + 3.14159265, p.z);
    col += vec3(0.30, 0.38, 0.60) * FENSTERLICHT * einfall;

    return col;
}

// ---- Kamera ----------------------------------------------------------------

void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zt = iTime * TEMPO;

    // Vortrieb: sin-Position => cos-Geschwindigkeit => weiche Umkehr
    float zpos = zt * 1.1 + sin(zt * 0.20) * 7.0;

    ro = vec3(kameraPfad(zpos), zpos);

    float za = zpos + 2.0;
    vec3 ta = vec3(kameraPfad(za), za);
    vec3 fw = normalize(ta - ro);

    float kruemm = kameraPfad(zpos + 1.5).x - kameraPfad(zpos - 1.5).x;
    float roll = -kruemm * ROLL + 0.12 * sin(zt * 0.13);

    vec3 up0 = vec3(sin(roll), cos(roll), 0.0);
    vec3 rt = normalize(cross(up0, fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.4 + rt * uv.x + up * uv.y);
}

// ---- Hauptprogramm ---------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro, rd;
    kamera(uv, ro, rd);

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    // Wand-Koordinaten am Treffer (dieselbe Faltung wie im SDF)
    vec2 q = p.xy - pfad(p.z);
    q.x = abs(q.x) - gabel(p.z);
    float w = atan(q.y, q.x);

    vec3 n = wandNormale(p);
    vec3 wand = shadeWand(p, ro, n, w);

    // NEU (1): Distanznebel - die Tiefe versinkt im Dunst
    wand = mix(wand, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    // Fenster: Loch in der Wand -> Aussenraum
    float F = fensterMask(w, p.z);
    // NEU (2): ganz ferne Fenster loesen sich im Dunst auf
    F *= exp(-0.001 * t * t);
    vec3 color = mix(wand, aussenRaum(rd), F);

    // NEU (3): Farbdrift - das ganze Bild wandert langsam durch kalte Toene
    color *= 0.85 + 0.15 * cos(iTime * 0.05 + vec3(0.0, 2.1, 4.2));

    // NEU (4): Tonemapping 1-exp, dann Gamma + Vignette
    color = 1.0 - exp(-color * 1.6);
    color = pow(color, vec3(1.0 / 2.2));
    color *= 1.0 - 0.30 * dot(uv, uv);

    fragColor = vec4(color, 1.0);
}
```

**Ergebnis:** Der fertige Shader. Ein endloser Röhren-Tunnel windet sich durch die Stratosphäre; Neonfugen und pulsierende Ringlichter gliedern die Tiefe, durch atmende Fenster fallen Sternenfeld, Horizontglühen und kühles Außenlicht herein; die Fahrt beschleunigt, kehrt weich um, legt sich in Kurven – und an jeder Gabelung entscheidet ein Hash, welcher Ast es diesmal wird.

### Was passiert hier – die vier Politur-Griffe

1. **Distanznebel** mit `exp(-k·t²)`: Im Tunnel hat er eine Doppelrolle mehr als sonst – er ist Atmosphäre, **und** er versteckt die beiden Marsch-Kompromisse aus Schritt 2 (die 60er-Kappung und die grobe Trefftoleranz der Grenz-Strahlen). Die Nebelfarbe ist bewusst fast schwarz-blau: Ein heller Nebel würde die Neon-Tiefenwirkung auffressen.
2. **Fenster-Ausblendung `F *= exp(-0.001·t²)`:** Ohne sie stünden in großer Tiefe winzige, gestochen helle Himmelslöcher im längst vernebelten Tunnel – ein Bruch, weil der Außenraum von Natur aus „im Unendlichen" liegt und nicht am Wand-Nebel teilnimmt. Die Ausblendung lässt ferne Fenster im selben Dunst versinken wie die Wand um sie herum.
3. **Farbdrift:** Drei phasenversetzte, sehr langsame Cosinus-Wellen multiplizieren das Gesamtbild (±15 %) – das `slow_roam_cos`-Erbe noch einmal, jetzt global: Selbst wenn gerade kein Ring pulsiert und kein Fenster atmet, lebt das Bild.
4. **Tonemapping `1 − exp(−x)`:** wörtlich die Schluss-Zeile der Schablone – und dahinter frosty caves' `ret = 1-exp(-ret)`. Für diesen Shader ist sie Pflicht: Neon-Kerne (Faktor ~8) und Ring-Zentren (~11) liegen weit über 1 und **glühen** durch die Sättigungskurve weich aus, statt hart zu clippen. Danach Standard-Gamma und eine Vignette, die den Blick in die Röhrenmitte lenkt.

### 🎨 Experimentieren – jetzt am Gesamtwerk

- Das Stellschrauben-Brett durchspielen – jede Konstante ist ein Charakter: `ROEHREN 8 / NEON_ANTEIL 0.8 / TEMPO 1.6` = Cyberpunk-Rutsche; `ROEHREN 24 / FENSTER_DICHTE 0.2 / RING_ABSTAND 18 / TEMPO 0.4` = einsame Orbital-Station
- Belichtung `* 1.6` → `* 2.8`: überstrahlter Look, das Neon frisst sich in die Röhren
- Nebel ans Neon koppeln: `vec3(0.010, 0.014, 0.030) + neonEmission(w, ro.z + 4.0) * 0.15` als Nebelfarbe → der Dunst voraus schimmert in der Farbe der kommenden Fugen (teuer, aber prächtig)
- `FENSTER_DYN = 1.0` + `RING_STAERKE = 0.0` + `SCHEINWERFER = 0.4`: fast nur noch Fensterlicht – der Tunnel als Zoetrop

🧠 **Merke:** Auch hier hat die Politur keine neue Idee gebraucht – nur Kurven (`exp`, `cos`, `pow`) auf das fertige Bild. Wenn der Tunnel in dieser Phase noch „gerettet" werden muss, liegt der Fehler in der Geometrie oder im Licht – nicht in der Politur.

---

# Anhang A: Audio-Reaktivität

Voraussetzung auf shadertoy.com: im Shader-Editor **iChannel0 mit „Music"** belegen (Kanal-Kachel → Music → beliebiger Track). Die Textur ist 512×2: Zeile 0 (`y ≈ 0.25`) das FFT-Spektrum, Zeile 1 (`y ≈ 0.75`) die Wellenform. Die ausführliche Herleitung der Band-Mittelung und der Beat-Gate-Dramaturgie steht im **Anhang A des Crystal-Lights-Tutorials** – A1 hier ist trotzdem eigenständig lauffähig, damit dieses Tutorial für sich funktioniert. A2 und A3 sind dann spezifisch für *diesen* Shader: Welche der vier Lichtquellen hört auf welches Band, und wo genau die Diffs ins Gesamtlisting gehören.

---

## Schritt A1 – bandLevel und Beat-Gate (eigenständig lauffähig)

**Neu:** Vier Frequenzbänder aus der FFT-Zeile und das binäre **Beat-Gate** – als Mini-Shader, der Pegel und Gate nebeneinander sichtbar macht, dazu einen pulsierenden Ring als Vorgeschmack auf die Ring-Lichter.

```glsl
// iChannel0: Music

// Mittelwert eines Frequenzbands aus der FFT-Zeile der Musik-Textur
float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++) {
        float x = mix(lo, hi, (float(i) + 0.5) / float(N));
        sum += texture(iChannel0, vec2(x, 0.25)).x;
    }
    return sum / float(N);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 cuv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    float bass = bandLevel(0.00, 0.05);
    float gate = smoothstep(0.60, 0.75, bass);    // DAS Beat-Gate

    vec3 color = vec3(0.02);

    // links: roher Bass-Pegel  |  rechts: das Gate (aus oder an)
    if (uv.x < 0.47) color = uv.y < bass ? vec3(0.9, 0.3, 0.3) : color;
    if (uv.x > 0.53) color = uv.y < gate ? vec3(0.3, 0.9, 1.0) : color;

    // und ein Ring in der Mitte zuckt mit dem Gate - die kommende Ringlicht-Logik
    float ring = abs(length(cuv) - 0.28 - 0.04 * gate);
    color += vec3(0.5, 0.7, 1.0) * gate * 0.02 / (0.001 + ring * ring * 40.0);

    fragColor = vec4(color, 1.0);
}
```

**Ergebnis:** Links wogt der Bass-Balken kontinuierlich, rechts springt der Gate-Balken schlagartig auf voll, sobald der Bass die Schwelle reißt – und bei jedem Kick blitzt der Ring auf und weitet sich einen Hauch.

### Was passiert hier

Das Gate ist die GLSL-Fassung von Milkdrops hartem `above(bass, 0.95)`-Schalter (die Crystal-Lights-Schablone leitet das am Rock-The-House-Preset her). Zwei Dinge muss man auch hier wissen:

1. **Die Skala ist absolut, nicht normiert.** Milkdrops `bass` schwankt um 1.0 („durchschnittlich laut"); die Shadertoy-FFT ist ein Rohpegel 0..1, der je Track völlig anders liegt. Die Schwelle `0.60/0.75` ist Handarbeit pro Musikrichtung. Die saubere, track-unabhängige Lösung (Vergleich mit dem eigenen gleitenden Mittel) braucht Gedächtnis – dafür verweist B2 auf die Buffer-A-Lösung der Schablone.
2. **`smoothstep` statt `step`:** Die schmale Rampe verhindert das Flackern, wenn der Pegel auf der Schwelle „sägt" – von weitem wirkt sie trotzdem wie ein Schalter.

Übrigens: Das Vorbild-Preset betreibt in seinem `per_frame`-Code ein ganzes **Resonator-Orchester** zur BPM-Schätzung (100 Feder-Masse-Systeme, Euler-Cauchy-Integration, `certain`-Konfidenz) – die Luxusklasse dessen, was hier in drei Zeilen steckt. Für einen Shadertoy-Shader ist das Gate der richtige Einstieg; wer die Resonator-Idee will, braucht Multipass-Zustand (B2).

### 🎨 Experimentieren

- Schwelle `0.60/0.75` → `0.35/0.50` bei ruhigen Tracks: das Gate übernimmt die Snare mit
- `gate = smoothstep(0.5, 0.65, bandLevel(0.25, 0.7));` → Hi-Hat-Gate: hektisch, glitzernd
- Zwei Gates (Bass → Rot, Höhen → Blau) nebeneinander: man *sieht* das Arrangement des Tracks

---

## Schritt A2 – Der Mapping-Katalog: welches Band an welche Lichtquelle?

Kein neuer Shader – eine Landkarte. Der Tunnel hat vier Lichtquellen, eine Geometrie und eine Kamera; die Kunst ist *musikalische Rolle → visuelle Rolle*. Alle Schnipsel beziehen sich auf das Gesamtlisting aus Schritt 13 und benutzen die Globals `gBass/gMid/gTreb/gVol/gGate` (die A3 einführt).

| # | Audio | steuert | Eingriff | warum es passt |
|---|---|---|---|---|
| 1 | Bass-**Gate** | Alle Neonfugen zünden gemeinsam | in `neonEmission()`: `aktiv = max(aktiv, gGate * (0.5 + 0.5 * hash21(vec2(gid, 6.6))));` nach der Flacker-Zeile | Der Beat schlägt längs durch den ganzen Tunnel; das Hash hält die Fugen ungleich hell – sonst Stroboskop |
| 2 | Bass (kontinuierlich) | **Tunnel-Puls**: der Radius atmet | in `mapTunnel()`: `RADIUS * (1.0 + 0.18 * gBass) - wandRelief(...) - length(q)` | Der Bass ist Masse – er darf die Geometrie selbst bewegen. Nur nach *außen* pulsieren lassen (Faktor ≥ 1): die Kamera sitzt auf der Achse und bleibt sicher |
| 3 | Mitten | Ring-Lichter laufen mit | in `ringPuls()`: Rückgabe `* (0.5 + 1.5 * gMid)` | Die Ringe sind der Takt der Strecke – Melodie und Fläche der Mitten tragen sie |
| 4 | Höhen | Sterne funkeln | in `aussenRaum()`, statische Sterne: `* (0.6 + 2.0 * gTreb)` | Hi-Hats sind spitz und fern – exakt der Charakter des Sternenfelds |
| 5 | Lautheit | **Fenster öffnen sich** | in `fensterMask()`: `float o = 0.8 + FENSTER_DYN * sin(...) + 0.5 * gVol;` | Energie reißt die Wand auf und flutet den Tunnel mit Außenlicht (Quelle 4 zieht automatisch mit!) – der dramatischste Eingriff |
| 6 | Bass-**Gate** | Die Vignette öffnet sich | in `mainImage()`: `color *= 1.0 - (0.30 - 0.12 * gGate) * dot(uv, uv);` | Beim Kick „weitet sich die Pupille" – billig, wirksam, greift nicht in die Geometrie |

Die zwei Warnungen der Vorgänger-Tutorials gelten hier in verschärfter Form:

- **Nie auf die Faktoren vor `iTime`.** Dieser Shader hat besonders viele Uhren: den Vortrieb `zpos` (Audio darauf = Teleport-Kamera bei jedem Beat!), die Fenster-Phasen, das Neon-Flackern, den Ring-Puls, die Palettenrotation. Alle Mappings oben gehen deshalb auf **Amplituden** (Stärke, Öffnungsgrad, Radius), nie auf Uhren. Wer „bei Bass schneller fliegen" will, braucht Zustand – B2 verweist auf die Buffer-Lösung.
- **Die Kamera fährt auf ungemappten Größen.** `pfad`, `gabel`, `kameraPfad` und der `zpos`-Ausdruck bleiben audio-frei – sonst springt die Kamera seitlich, sobald ein Pegel zuckt. Mapping 2 ist die eine erlaubte Geometrie-Ausnahme, *weil* es den Radius nur nach außen aufpumpt und die Achse (auf der die Kamera fährt) unberührt lässt. Selbst dort gilt: Roh-Pegel zittern im Frame-Takt – die Wand „vibriert". Dezent dosieren oder den Pegel glätten (B2).

---

## Schritt A3 – Der Tunnel hört zu

**Neu:** Die Mappings 1–6 wandern in den fertigen Shader. Gezeigt sind nur die Änderungen gegenüber dem Gesamtlisting aus Schritt 13 – auf shadertoy.com zusätzlich **iChannel0 = Music** setzen.

**(a) Vor die Stellschrauben** – Audio-Infrastruktur:

```glsl
// ---- AUDIO -----------------------------------------------------------------
float gBass = 0.0, gMid = 0.0, gTreb = 0.0, gVol = 0.0, gGate = 0.0;

float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++) {
        float x = mix(lo, hi, (float(i) + 0.5) / float(N));
        sum += texture(iChannel0, vec2(x, 0.25)).x;
    }
    return sum / float(N);
}
// ----------------------------------------------------------------------------
```

**(b) Am Anfang von `mainImage`** – einmal pro Frame füllen, **bevor** Kamera und Marsch laufen (Mapping 2 greift in `mapTunnel` ein, das vom Marsch aufgerufen wird – die Globals müssen vorher stehen):

```glsl
    gBass = bandLevel(0.00, 0.05);
    gMid  = bandLevel(0.05, 0.25);
    gTreb = bandLevel(0.25, 0.70);
    gVol  = bandLevel(0.00, 0.70);
    gGate = smoothstep(0.60, 0.75, gBass);
```

**(c) Die sechs Eingriffe** (Mapping-Nummern aus A2):

```glsl
// neonEmission(): nach der Flacker-Zeile einfuegen                    [1]
    aktiv = max(aktiv, gGate * (0.5 + 0.5 * hash21(vec2(gid, 6.6))));

// mapTunnel(): der Radius atmet mit dem Bass                          [2]
    return RADIUS * (1.0 + 0.18 * gBass) - wandRelief(w, p.z) - length(q);

// ringPuls(): Ringe laufen mit den Mitten                             [3]
    return smoothstep(0.55, 0.95, wv) * (0.3 + 0.7 * hash21(vec2(id, 5.1)))
         * (0.5 + 1.5 * gMid);

// aussenRaum(): statische Sterne funkeln mit den Hoehen               [4]
    col += vec3(0.9) * smoothstep(0.994, 1.0, s)
         * clamp(h * 3.0 + 0.5, 0.0, 1.0) * (0.6 + 2.0 * gTreb);

// fensterMask(): Energie oeffnet die Fenster                          [5]
    float o = 0.8 + FENSTER_DYN *
              sin(iTime * (0.15 + 0.25 * hash21(id + 9.4)) + TAU * hash21(id))
            + 0.5 * gVol;

// mainImage(): Vignette oeffnet sich beim Beat                        [6]
    color *= 1.0 - (0.30 - 0.12 * gGate) * dot(uv, uv);
```

**Ergebnis:** Bei jedem Kick zünden alle Neonfugen längs des Tunnels und die Röhre selbst pumpt einen Atemzug weiter auf; die Ringlichter tragen die Mitten als laufenden Puls in die Tiefe; Hi-Hats besprühen das Sternenfeld mit Funkeln. In lauten Passagen reißen die Fenster weiter auf und das Außenlicht flutet die Wände – wird es still, fällt der Tunnel auf sein sehenswertes Eigenleben zurück: einzelne Fugen, gemächliche Ringe, atmende Fenster.

### Was passiert hier

Das dramaturgische Kalkül ist dasselbe wie bei Crystal Lights: **Musik verstärkt die Grundmechanik, sie ersetzt sie nicht.** Deshalb `max(aktiv, ...)` statt Überschreiben, deshalb Faktoren wie `(0.5 + 1.5·gMid)` statt `gMid` pur – bei Stille bleibt jeder Effekt auf seinem Eigenleben-Niveau. Ein Visualizer, der ohne Musik schwarz ist, ist mit Musik meist nur ein VU-Meter.

Der riskanteste Eingriff ist Mapping 2, der einzige in der Geometrie: Der Roh-Bass zittert im Frame-Takt, und mit ihm die Wand. Bei `0.18` Hub ist das als „Vibrieren unter Last" sogar erwünscht; wer es glatter will, ersetzt `gBass` dort durch eine geglättete Version aus einem Buffer-Pass – das Muster steht in **Anhang B3 der Crystal-Lights-Schablone** und läuft unverändert auch hier.

### 🎨 Experimentieren

- Mapping 1 abschalten, nur 2 aktiv: „der Tunnel atmet" statt „der Tunnel schlägt" – zwei verschiedene Visualizer aus einer Zeile
- `gGate` zusätzlich auf den Scheinwerfer: in `shadeWand` `SCHEINWERFER * (1.0 + 1.5 * gGate) * dif ...` → der Kick reißt das Fernlicht auf
- Mapping 5 invertieren (`- 0.5 * gVol`): bei Energie schließen sich die Fenster – Bunker-Dramaturgie, überraschend spannungsvoll
- Die erlaubte „schneller fliegen"-Variante: `gBass` auf die **Banking-Stärke** (`roll`-Zeile `* (1.0 + gBass)`) – die Fahrt wirkt wilder, ohne dass die Position springt

---

# Anhang B: Der Weg in die App (kompakt)

Der fertige Shader benutzt ausschließlich Standard-Uniforms (`iResolution`, `iTime`, `iChannel0` nur für Audio) – nach der Konvention der 100 Vorrats-Shader in `asset/shadertoys/` (STELLSCHRAUBEN-Konstantenblock, keine plattformspezifischen Extras). Die **drei Import-Wege** (Copy & Paste in den Shadertoy-Node, URL-/ID-Import mit App-Key, Shadertoy-Browser-Panel) und die **allgemeine Portabilitäts-Checkliste** (kein `#version`, `mainImage` unverändert, Audio-Layout 512×2, deterministische Sim-Uhr usw.) stehen ausführlich in **Crystal-Lights-Shader-Tutorial.md, Anhang B** – sie gelten für diesen Shader wörtlich genauso und werden hier nicht wiederholt. Dieser Anhang ergänzt nur, was *shader-spezifisch* ist.

---

## B1 – Audio-Adapter konkret für diesen Shader

In LumiViz gibt es die fertigen Uniforms `bass`, `mid`, `treb`, `vol`, `beat` – die `bandLevel`-Schleifen aus A3 sind dort unnötig. Damit der Umzug ein Kommentar-Tausch bleibt, spricht der Shader die Pegel nur über Adapter-Funktionen an (das Muster aus Anhang B2 der Schablone, hier mit den Namen dieses Shaders):

```glsl
// ===== AUDIO-ADAPTER =========================================================
// Genau EINEN der beiden Bloecke aktiv lassen.

// --- Variante SHADERTOY (iChannel0 = Music) ---------------------------------
float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++)
        sum += texture(iChannel0,
                       vec2(mix(lo, hi, (float(i) + 0.5) / float(N)), 0.25)).x;
    return sum / float(N);
}
float aBass() { return bandLevel(0.00, 0.05); }
float aMid()  { return bandLevel(0.05, 0.25); }
float aTreb() { return bandLevel(0.25, 0.70); }
float aVol()  { return bandLevel(0.00, 0.70); }
float aBeat() { return smoothstep(0.60, 0.75, aBass()); }

// --- Variante LUMIVIZ (eingebaute Uniforms; Skalen-Faktor nachziehen!) ------
// float aBass() { return bass * 0.3; }
// float aMid()  { return mid  * 0.3; }
// float aTreb() { return treb * 0.3; }
// float aVol()  { return vol  * 0.3; }
// float aBeat() { return beat; }
// ============================================================================
```

In A3(b) heißt es dann `gBass = aBass();` usw. – der Rest des Shaders weiß nicht, auf welcher Plattform er läuft. Der `0.3`-Faktor ist ein Startwert: Die LumiViz-Uniforms und die rohe Shadertoy-FFT liegen auf verschiedenen Skalen (app-seitige Klärung dB-vs-linear Stand S65 offen) – beim Umzug einmal beide nebeneinander visualisieren (A1-Muster) und **nur den Adapter** anpassen, nie die sechs Mappings einzeln.

Zwei shader-spezifische Punkte für den Import:

- **Mapping 2 (Tunnel-Puls) zittert mit Roh-Pegeln.** In der App liefert `beat` bereits eine saubere Hüllkurve; für den geglätteten Bass lohnt trotzdem der **Buffer-A-Envelope-Pass aus Anhang B3 der Schablone** (Tiefpass + adaptiver Trigger + Abklingkurve in Pixel (0,0)). Der Shadertoy-Node unterstützt die nötige Selbstreferenz als Multipass (Buffer liest sein Vorframe, Ping-Pong-Semantik) – die Verdrahtung ist dort Schritt für Schritt beschrieben.
- **Die deterministische Sim-Uhr der App ist ein Feature für genau diesen Shader:** Vortrieb, Gabelwahl (`gabelSeite` hasht auf Streckenzellen!), Fensterphasen – alles ist reine Funktion von `iTime`. Derselbe Frame ergibt in LumiViz immer dasselbe Bild, inklusive derselben Gabel-Entscheidungen: prüfstand- und vergleichbild-tauglich.

---

## B2 – Welche STELLSCHRAUBEN sich als Panel-Parameter anbieten

Shadertoy hat keine Panels – LumiViz schon. Wer den Shader als Chain-Node einrichtet, zieht die dankbarsten Konstanten nach oben (der Konstantenblock ist bewusst so geschnitten, dass das ein reines Umbenennen ist):

| Stellschraube | Panel-Typ | Wirkung / Hinweis |
|---|---|---|
| `TEMPO` | Slider 0.2–2.5 | Gesamttempo der Fahrt – der wichtigste Live-Regler |
| `ROEHREN` | **Integer**-Slider 4–32 | Röhrenzahl; ganzzahlig erzwingen (Naht, Schritt 4!) – die Stellgröße, die das Vorbild würfelt |
| `NEON_ANTEIL` | Slider 0–1 | von Leitlinie bis Las Vegas |
| `NEON_STAERKE` | Slider 0–0.05 | Helligkeit; zusammen mit Tonemapping-Belichtung abstimmen |
| `FENSTER_DICHTE` | Slider 0–1 | Bunker ↔ Panorama |
| `FENSTER_DYN` | Slider 0–1 | statische vs. atmende Fenster |
| `RING_ABSTAND` | Slider 3–20 | Takt der Strecke |
| `KURVE` | Slider 0–1.6 | Kurvigkeit; ab ~1.2 die Marsch-Drossel im Blick behalten (Schritt 10) |
| `GABEL_WEITE` | Slider 0–2 | 0 = nie gabeln, 0.9 = Doppellauf, 1.6 = echte Trennung |
| `SCHEINWERFER` / `FENSTERLICHT` | Slider | Licht-Mischpult der vier Quellen (Neon/Ringe über ihre Stärken) |

Nicht als Live-Regler geeignet: `RADIUS` (skaliert fast alle abgeleiteten Konstanten mit), `FENSTER_SPALTEN` (Ganzzahl-Naht wie `ROEHREN`), `GABEL_PERIODE` (Sprünge in `gabelSeite`-Zellen beim Verstellen → die Kamera kann schlagartig den Ast wechseln).

---

## Abspann

Damit ist die Reise komplett: ein Innen-Raymarcher mit einer einzigen Karte (die abgerollte Wand), fünf Feldern darauf (Röhren, Relief, Fenster, Neon, Ringe), vier Lichtquellen mit vier Rhythmen, einer Kamera mit weicher Umkehr und Banking – und einer Gabelung, die aus einer `abs`-Zeile entsteht.

**Ein ehrliches Wort zum Schluss:** Alle Schritte dieses Tutorials sind sorgfältig konstruiert und gegengerechnet (Steigungen, Naht-Bedingungen, Helligkeits-Größenordnungen), aber **nicht gerendert oder getestet** – anders als die Vorrats-Shader hat dieser Text noch kein Pixel gesehen. Beim ersten Lauf gehören die Erfahrungswerte auf den Prüfstand: die Helligkeiten (`NEON_STAERKE`, `RING_STAERKE`, Tonemapping-Belichtung `1.6`), die Nebel-Konstanten, die Marsch-Drossel `0.7` bei aggressiven `KURVE`-/`RELIEF`-Werten. Wenn etwas nicht aussieht wie beschrieben, sind das die ersten Verdächtigen – die Struktur trägt.

Wer weitermachen will:

- **Als Vorlage in die App:** den Endstand (oder die Lieblings-Variante) als `.lvfx` neben die Vorlagen in `asset/effectchain/shadertoys/` legen – Konvention siehe dort (`.glsl` = SSOT).
- **Die Weichen zurückverfolgen:** Fast jeder 🎨-Kasten ist ein eigener Shader. Besonders ergiebig: der Doppellauf-Tunnel (Schritt 12), die Wendel-Spanten (Schritt 4), der Zoetrop-Modus (Schritt 13).
- **Die Milkdrop-Brücke:** Wer den Look als *Preset* statt als Shadertoy-Node will, hat mit *stratospheric turbulences 2* die Blaupause vor sich – dort entsteht derselbe Tunnel ohne einen einzigen Marsch-Schritt, als Masken-Malerei auf der Polar-Karte aus Schritt 1, mit dem Feedback-Buffer als Gedächtnis. Der Vergleich beider Wege ist die vielleicht beste Raymarching-Lektion von allen.

Und jetzt: Musik an. 🎵🛰️







