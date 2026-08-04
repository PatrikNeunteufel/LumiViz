# Raymarching — Referenz

> **Dokumenttyp:** Reference  
> **Version:** 1.0.0  
> **Status:** Stabil  
> **Domain:** Programming  
> **Kategorie:** Algorithms  
> **Programmiersprache:** GLSL (Shadertoy/WebGL2; Konzepte sprachunabhängig)  
> **Voraussetzungen:** Grundkenntnisse Fragment-Shader und GLSL — [Pyramid-Spiral-Shader-Tutorial](Pyramid-Spiral-Shader-Tutorial.md) Schritte 1–2  
> **Gültigkeit:** Distanzfeld-basiertes Raymarching (Sphere Tracing) und seine Varianten, wie sie in der Shader-Tutorial-Serie dieses Ordners verwendet werden  
> **Zweck:** Vollständige technische Referenz des Raymarching-Verfahrens: Algorithmus, Distanzfunktionen, Raumoperationen, Normalen, Beleuchtung, Varianten, Artefakte  
> **Zielgruppe:** Shader-Entwickler; Leser der Tutorial-Serie, die das „Warum hinter dem Wie" nachschlagen wollen  
> **Sprache:** Deutsch  
> **Zugehörige Tutorials:** siehe [Shader-Tutorials-Wegleitung](Shader-Tutorials-Wegleitung.md)

> ℹ **Hinweis:** Dieses Dokument enthält ASCII-Diagramme und ASCII-Formelboxen. Die Darstellung setzt eine Monospace-Schrift voraus.

---

## Inhaltsverzeichnis

1. [Zweck und Zusammenfassung](#1-zweck-und-zusammenfassung)
2. [Übersicht und Einordnung](#2-übersicht-und-einordnung)
3. [Distanzfunktionen (SDF)](#3-distanzfunktionen-sdf)
4. [Der Sphere-Tracing-Algorithmus](#4-der-sphere-tracing-algorithmus)
5. [Marsch-Varianten](#5-marsch-varianten)
6. [Raumoperationen](#6-raumoperationen)
7. [Normalen und Ableitungen](#7-normalen-und-ableitungen)
8. [Beleuchtung auf SDF-Basis](#8-beleuchtung-auf-sdf-basis)
9. [Kamera-Modelle](#9-kamera-modelle)
10. [Stabilität, Artefakte und Performance](#10-stabilität-artefakte-und-performance)
11. [Schnellreferenz](#11-schnellreferenz)
12. [Glossar](#12-glossar)
13. [Siehe auch](#13-siehe-auch)
14. [Changelog](#14-changelog)

---

## 1. Zweck und Zusammenfassung

Raymarching ist ein Renderverfahren, das eine Szene nicht aus Dreiecken aufbaut, sondern aus einer **Funktion**: Für jeden Bildpixel läuft ein Strahl schrittweise durch den Raum, und eine Distanzfunktion beantwortet an jedem Punkt die Frage „Wie weit ist die nächste Oberfläche entfernt?". Der Strahl geht genau diese Distanz vorwärts und wiederholt die Frage, bis er eine Oberfläche trifft oder die Szene verlässt. Dieses Dokument ist die technische Referenz zu diesem Verfahren für die Shader-Tutorial-Serie in diesem Ordner.

Die Kernaussagen in Kurzform:

- Die gesamte Szene ist eine einzige Funktion `map(p)` — die **vorzeichenbehaftete Distanzfunktion** (Signed Distance Function, SDF). Positiv bedeutet außerhalb, negativ innerhalb, null auf der Oberfläche.
- Der Marsch-Algorithmus (**Sphere Tracing**) ist korrekt, solange die SDF die tatsächliche Distanz nie **überschätzt**. Jede Raumverzerrung, die diese Garantie schwächt, wird mit einem Drosselfaktor unter 1 kompensiert.
- **Normalen** kommen ohne Zusatzwissen aus dem Gradienten der SDF; Beleuchtung, weiche Schatten und Umgebungsverdeckung lassen sich direkt aus Distanzabfragen konstruieren.
- Es existieren mehrere **Varianten** des Marschs (klassisch, gedrosselt, überrelaxiert, Höhenfeld, Fixschritt, analytisch); Abschnitt 5 dokumentiert alle mit Vor- und Nachteilen samt Entscheidungshilfe.
- Die typischen **Artefakte** (Durchschuss, Banding, Kantenflimmern) haben bekannte Ursachen und Standard-Gegenmittel; Abschnitt 10 katalogisiert sie.

Wo dieses Dokument ein Konzept nur referenziert statt herzuleiten, liefern die Tutorials der Serie die schrittweise Herleitung am lauffähigen Beispiel; die Zuordnung von Konzept zu Tutorial-Schritt steht in der [Shader-Tutorials-Wegleitung](Shader-Tutorials-Wegleitung.md) (Technik-Index).

## 2. Übersicht und Einordnung

### 2.1 Anwendungsbereich

Raymarching ist das dominierende Verfahren für prozedurale 3D-Szenen auf Plattformen wie Shadertoy, weil es ohne Geometrie-Pipeline auskommt: Ein einziger Fragment-Shader beschreibt und rendert die komplette Welt. Das Verfahren eignet sich besonders für Szenen, deren Geometrie sich als Mathematik ausdrücken lässt — unendliche Wiederholungen, gefaltete Räume, organische Verformungen. Es eignet sich schlecht für Szenen aus vielen individuellen, unregelmäßigen Objekten, für die klassische Dreiecks-Rasterisierung oder Raytracing gegen explizite Geometrie effizienter sind.

### 2.2 Abgrenzung zu verwandten Verfahren

Die drei Begriffe Rasterisierung, Raytracing und Raymarching werden häufig vermischt; Tab. 1 grenzt sie ab.

| Verfahren | Szenenrepräsentation | Schnittberechnung | Typischer Einsatz |
|---|---|---|---|
| Rasterisierung | Dreiecks-Meshes | Projektion + Tiefentest | Echtzeit-Engines, Spiele |
| Raytracing | Explizite Geometrie (Dreiecke, Kugeln) | Analytische Schnittformeln | Offline-Rendering, RTX |
| Raymarching | Implizite Funktion (SDF, Dichtefeld) | Iteratives Voranschreiten | Prozedurale Shader, Volumetrik |

*Tab. 1: Abgrenzung der Renderverfahren*

Der entscheidende Unterschied zwischen Raytracing und Raymarching liegt in der Schnittberechnung: Raytracing **löst** die Schnittgleichung eines Strahls mit einem Objekt exakt (beispielsweise die quadratische Gleichung für eine Kugel), Raymarching **tastet sich heran**, weil für eine beliebige SDF keine geschlossene Lösung existiert. Beide Ansätze mischen sich gut — wo eine analytische Lösung existiert (Ebene, Kugel), ist sie dem Marsch vorzuziehen (siehe Abschnitt 5.6).

### 2.3 Nicht-Raymarching-Ansätze im Shader-Kontext

Ein Fragment-Shader muss keine 3D-Szene rendern. Die wichtigsten Alternativen, die ebenfalls in der Tutorial-Serie vorkommen: rein **2D-basierte Muster** (UV-Transformationen, Faltungen, Distanzfelder in der Ebene), **Feedback-Systeme** (der Shader liest sein eigenes Vorframe und entwickelt einen Bildzustand weiter — siehe [Pimped-Kaleidoscope-Shader-Tutorial](Pimped-Kaleidoscope-Shader-Tutorial.md)) und **analytische Projektionen** (direkte Formel Bild → Farbe ohne Iteration). Diese Ansätze sind kein Gegenstand dieses Dokuments; es behandelt ausschließlich den 3D-Marsch und seine Varianten.

## 3. Distanzfunktionen (SDF)

### 3.1 Definition

Eine vorzeichenbehaftete Distanzfunktion (Signed Distance Function, SDF) ordnet jedem Punkt `p` des Raums den kürzesten Abstand zur Oberfläche einer Form zu, mit Vorzeichen: positiv außerhalb, negativ innerhalb, exakt null auf der Oberfläche. Die Oberfläche selbst ist damit die Nullstellenmenge der Funktion — die Form wird nicht gespeichert, sondern bei jeder Abfrage neu „befragt". Diese implizite Darstellung ist der Grund für die Stärken des Verfahrens: Raumtransformationen wirken auf die *Eingabe* der Funktion und kosten unabhängig von der Szenenkomplexität konstant viel.

```text
+----------------------------------------------------+
|  d = sdf(p)                                        |
|                                                    |
|  d > 0 : p liegt ausserhalb der Form               |
|  d = 0 : p liegt auf der Oberflaeche               |
|  d < 0 : p liegt innerhalb der Form                |
+----------------------------------------------------+
```

Legende: `p`: Abfragepunkt im Raum [Szeneneinheiten] (beliebig) · `d`: kürzester Abstand zur Oberfläche [Szeneneinheiten] (negativ bis positiv unbeschränkt).

### 3.2 Die zentrale Eigenschaft: keine Überschätzung

Der Marsch-Algorithmus (Abschnitt 4) verlässt sich auf genau eine Garantie: Der zurückgegebene Wert darf die wahre Distanz **niemals überschreiten**. Formal ist das die Forderung, dass die Funktion 1-Lipschitz-stetig ist — ihr Wert ändert sich höchstens so schnell, wie sich der Abfragepunkt bewegt.

```text
+----------------------------------------------------+
|  |sdf(a) - sdf(b)|  <=  L * |a - b|    mit L <= 1  |
+----------------------------------------------------+
```

Legende: `a`, `b`: zwei Abfragepunkte [Szeneneinheiten] (beliebig) · `L`: Lipschitz-Konstante [—] (exakte SDF: 1; verzerrte SDF: größer 1).

Eine **exakte** SDF (echte euklidische Distanz, `|∇sdf| = 1`) erfüllt das per Definition. Viele praktische Funktionen sind jedoch nur **Distanzschranken** (bounds): Sie liefern einen Wert kleiner oder gleich der wahren Distanz — das ist unschädlich, der Marsch wird nur langsamer. Gefährlich ist die Gegenrichtung: Verzerrungen wie Twist oder Displacement können die effektive Lipschitz-Konstante über 1 heben, die Funktion überschätzt dann lokal, und der Strahl kann Oberflächen durchspringen. Die Kompensation ist der Drosselfaktor (Abschnitt 4.3 und 6.5).

### 3.3 Grundkörper-Katalog

Tab. 2 listet die in der Tutorial-Serie verwendeten Grundkörper mit ihren Distanzformeln. Alle Formeln erwarten die Form im Ursprung; Verschiebung und Rotation erfolgen über die Eingabetransformation (Abschnitt 6.3). Eine umfassende Sammlung weiterer Formeln pflegt Inigo Quilez (siehe Abschnitt 13).

| Form | Formel (GLSL-nah) | Exaktheit | Verwendet in |
|---|---|---|---|
| Kugel (Radius r) | `length(p) - r` | exakt | Pyramid-Spiral 3 |
| Ebene (Normale n, Offset h) | `dot(p, n) - h` | exakt | Crystal-Lights 2 |
| Box (Halbmaße b) | `length(max(q,0)) + min(max(q.x,max(q.y,q.z)),0)` mit `q = abs(p) - b` | exakt | Space-Debris 5 |
| Oktaeder (Größe s) | `(abs(p).x + abs(p).y + abs(p).z - s) * 0.57735` | Schranke (kantennah) | Pyramid-Spiral 5 |
| Zylinder (Radius r, Halbhöhe h) | `max(length(p.xz) - r, abs(p.y) - h)` | Schranke (Kante) | Space-Debris 5 |
| Kapsel (Radius r, Höhe h) | `length(p - vec3(0, clamp(p.y, r, h - r), 0)) - r` | exakt | Pyramid-Spiral B2 |
| Torus (Radien R, r) | `length(vec2(length(p.xz) - R, p.y)) - r` | exakt | Space-Debris 5 (Ring) |

*Tab. 2: Grundkörper-SDFs der Tutorial-Serie*

Zwei Einträge verdienen Erläuterung. Der Faktor `0.57735` des Oktaeders ist `1/sqrt(3)`: Der Rohausdruck misst den Abstand zur Ebene mit Normalenvektor `(1,1,1)`, dessen Länge `sqrt(3)` beträgt — ohne die Normierung würde die Funktion überschätzen und der Marsch durch Kanten springen. Die Box-Formel zerlegt den Abstand in einen Außenanteil (`length(max(q,0))` — Abstand zum nächsten Kanten-/Flächenpunkt) und einen Innenanteil (`min(max(...),0)` — negative Distanz im Inneren); beide zusammen ergeben eine exakte SDF.

```glsl
// SDF-Grundkoerper: Kugel und Box als Referenz-Implementierung.
// Beide Funktionen sind exakte Distanzen (Lipschitz-Konstante 1).

float sdKugel(vec3 p, float r)
{
    return length(p) - r;                 // Abstand zum Zentrum minus Radius
}

float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;                  // in den positiven Oktanten falten
    return length(max(q, 0.0))            // Aussenanteil (0 im Inneren)
         + min(max(q.x, max(q.y, q.z)), 0.0); // Innenanteil (0 im Aeusseren)
}
// Beispiel: sdKugel(vec3(3,0,0), 1.0) == 2.0
```

## 4. Der Sphere-Tracing-Algorithmus

### 4.1 Funktionsweise (WAS und WIE)

Sphere Tracing beantwortet die Schnittfrage „Wo trifft dieser Strahl die Szene?" iterativ. An der aktuellen Strahlposition wird die SDF ausgewertet; ihr Wert ist der Radius einer garantiert leeren Kugel um diesen Punkt. Der Strahl darf deshalb genau diese Distanz gefahrlos vorwärtsgehen — nichts kann übersprungen werden. Weit weg von Geometrie macht der Strahl große Schritte, nahe an Oberflächen immer kleinere; die Schrittweite konvergiert gegen null, wenn der Strahl auftrifft.

```text
Strahl:  ro *----->----->--->-->->x Oberflaeche
              d1     d2    d3  d4
              (Schrittweite = SDF-Wert am jeweiligen Punkt)
```

*Fig. 1 [Blockdiagramm]: Sphere Tracing — jeder Schritt geht exakt so weit, wie die SDF Freiraum garantiert*

Der Kern-Update lautet:

```text
+----------------------------------------------------+
|  t_neu = t + lambda * map(ro + rd * t)             |
+----------------------------------------------------+
```

Legende: `t`: bisher zurückgelegte Strahlstrecke [Szeneneinheiten] (0 bis t_max) · `lambda`: Drosselfaktor [—] (0.3 bis 1.0; 1.0 nur bei exakten SDFs) · `ro`: Strahlursprung [Szeneneinheiten] (Kameraposition) · `rd`: normierte Strahlrichtung [—] (Länge 1) · `map`: Szenen-SDF [Szeneneinheiten].

### 4.2 Referenz-Implementierung

Dem Pseudocode folgt die GLSL-Fassung, wie sie alle 3D-Tutorials der Serie in Varianten verwenden.

```text
Pseudocode:
  t = 0
  wiederhole N-mal:
    p = ro + rd * t
    d = map(p)
    wenn d < epsilon(t):  TREFFER bei t
    wenn t > t_max:       KEIN TREFFER
    t = t + lambda * d
  KEIN TREFFER (Budget erschoepft)
```

```glsl
// Sphere Tracing: liefert die Trefferdistanz oder -1.0 bei Fehlschlag.
// MAX_SCHRITTE und Drossel sind die zwei zentralen Qualitaetsregler.
float marsch(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 100; i++) {           // Schrittbudget (konstant!)
        vec3  p = ro + rd * t;
        float d = map(p);
        if (d < 0.001 + 0.0015 * t) return t; // Toleranz waechst mit der Ferne
        if (t > 40.0) break;                  // Szenengrenze
        t += d * 0.9;                          // Drossel 0.9: kleine Sicherheitsreserve
    }
    return -1.0;                               // kein Treffer im Budget
}
```

### 4.3 Die drei Parameter

Das Verhalten des Marschs hängt an drei Stellschrauben, deren Zusammenspiel Tab. 3 zusammenfasst. Die distanzabhängige Treffertoleranz `epsilon(t)` verdient Begründung: Ein Pixel deckt in der Ferne ein größeres Raumvolumen ab als in der Nähe; eine konstante Toleranz erzeugt dort Flimmern (Sub-Pixel-Details) und verschwendet Schritte. Die lineare Kopplung an `t` gleicht das aus.

| Parameter | Wirkung bei Erhöhung | Typischer Bereich | Faustregel |
|---|---|---|---|
| Schrittbudget N | Weniger Fehlschläge an Silhouetten, mehr Kosten | 80–200 | Höhenfelder und Verzerrungen brauchen mehr |
| Drosselfaktor lambda | Schneller, aber Durchschuss-Risiko | 0.3–1.0 | 1.0 nur bei exakten SDFs; siehe Tab. 5 |
| Toleranz epsilon(t) | Weichere Treffer, weniger Flimmern | 0.001 + 0.001·t bis 0.002·t | An Szenenmaßstab anpassen |

*Tab. 3: Marsch-Parameter und ihre Wirkung*

### 4.4 Komplexität und Konvergenz (WARUM)

Die Laufzeit pro Bild beträgt `O(P * N * C_map)` mit Pixelzahl `P`, Schrittbudget `N` und Kosten `C_map` einer SDF-Auswertung. Da `P` und `N` meist fest sind, ist `C_map` der wirksamste Optimierungshebel — jede Zeile in `map` wird pro Pixel bis zu N-mal ausgeführt, bei Normalenberechnung zusätzlich vier- bis sechsmal. Der Speicherbedarf ist `O(1)`: Das Verfahren hält keinerlei Szenendaten.

Die Konvergenz ist am schnellsten bei frontalem Auftreffen (die Schrittweite fällt geometrisch) und am langsamsten bei streifenden Strahlen, die lange parallel zu einer Oberfläche laufen — dort frisst die Silhouette das Schrittbudget. Dieses Verhalten erklärt, warum der Debug-Trick „Schrittzahl als Farbe" Silhouetten hell zeichnet, und warum Fehlschläge zuerst an Kanten sichtbar werden.

## 5. Marsch-Varianten

Die Grundform aus Abschnitt 4 existiert in mehreren Varianten, die sich in Schrittstrategie und Einsatzgebiet unterscheiden. Dieser Abschnitt dokumentiert alle sechs in der Serie relevanten Varianten einzeln; Tab. 4 vergleicht sie, Tab. 5 gibt die Entscheidungshilfe.

### 5.1 Klassisches Sphere Tracing

Klassisches Sphere Tracing schreitet mit voller SDF-Distanz voran (`lambda = 1`). Es ist die schnellste korrekte Variante, verlangt aber exakte oder unterschätzende SDFs — jede Überschätzung führt unmittelbar zu Durchschuss-Artefakten. Die Variante eignet sich für Szenen aus den Grundkörpern und milden Kombinationen aus Tab. 2.

- Vorteile: minimale Schrittzahl; keine Tuning-Parameter; referenzgenau
- Nachteile: reagiert empfindlich auf jede nicht-exakte SDF; keine Reserven für Verzerrungen

Typische Anwendung: Pyramid-Spiral (Schritte 3–9) — reine Grundkörper mit Repetition und Faltung, alle Operationen distanzerhaltend oder unterschätzend.

### 5.2 Gedrosseltes (konservatives) Sphere Tracing

Das gedrosselte Verfahren multipliziert jeden Schritt mit einem Faktor unter 1 und erkauft sich damit Sicherheit gegenüber überschätzenden SDFs. Es ist die Standardwahl der Tutorial-Serie, sobald Twist, Displacement, Boolesche Verschneidungen oder Material-Mischungen im Spiel sind. Der Faktor wird aus der stärksten Verzerrung der Szene abgeschätzt und gilt global; feiner ist eine lokale Drossel nur in verzerrten Zonen.

- Vorteile: robust gegen Lipschitz-Verletzungen; ein einziger, gut verstehbarer Regler
- Nachteile: proportional mehr Schritte (lambda 0.5 verdoppelt die Schrittzahl); zu aggressive Drossel verschenkt Budget

Typische Anwendung: Pyramid-Spiral ab dem Twist (Drossel 0.9), Juggernaut (Greeble-Verschneidungen), Space-Debris (verbeulte Brocken).

### 5.3 Über-Relaxation (Over-Relaxation)

Über-Relaxation geht bewusst **weiter** als die SDF erlaubt (`lambda > 1`, typisch 1.2 bis 1.6) und prüft im Folgeschritt, ob die übersprungene Strecke frei war; falls nicht, fällt der Algorithmus auf die sichere Position zurück. Bei glatten Szenen sinkt die Schrittzahl spürbar, der Rückfall-Mechanismus macht den Code jedoch komplexer und der Gewinn verpufft bei detailreicher Geometrie.

- Vorteile: 20–40 % weniger Schritte bei glatten, weiträumigen Szenen
- Nachteile: Zusatzlogik und ein zweiter SDF-Aufruf im Rückfallpfad; bei feinem Detail netto langsamer

Typische Anwendung: In der Tutorial-Serie bewusst nicht verwendet (die Serie priorisiert Lesbarkeit); relevant für performance-kritische Einzel-Shader mit großen leeren Räumen.

### 5.4 Höhenfeld-Marsch

Der Höhenfeld-Marsch ersetzt die volumetrische SDF durch eine Höhenfunktion `h(x, z)`: Der „Abstand" ist die vertikale Differenz `p.y - h(p.xz)`, die die wahre Distanz an Hängen systematisch überschätzt. Die Kompensation ist eine deutlich stärkere Drossel (0.3 bis 0.5) plus wachsende Treffertoleranz; dafür kann die Höhenfunktion beliebig unstetig sein (Terrassen, Schluchten), was einer echten SDF schwerfällt.

- Vorteile: Landschaften, Plateaus und Abgründe mit einer simplen 2D-Funktion; Unstetigkeiten erlaubt
- Nachteile: kein Sicherheitsbeweis — die Drossel ist Empirie; steile Wände kosten viele Schritte; nur „von oben sichtbare" Geometrie

Typische Anwendung: Crystal-Lights (Schritt 3 führt das Verfahren ein; Kristallplatten und Lücken nutzen die Unstetigkeits-Freiheit aus).

### 5.5 Fixschritt-Marsch (Volumen-Marsch)

Der Fixschritt-Marsch verzichtet auf Distanzinformation und tastet den Strahl in konstanten Intervallen ab. Er ist das Mittel der Wahl, wenn keine Oberfläche gesucht wird, sondern ein **Integral entlang des Strahls** — Dichte von Nebel, Streulicht, Glühen. In der Serie tritt er in Kurzform als Glow-Akkumulation auf (ein Additionsterm pro Marschschritt); die Vollform mit eigener Schleife gehört zu den Ausblick-Themen (Volumetrik).

- Vorteile: einziger Ansatz für Volumen-Effekte; trivial zu implementieren
- Nachteile: Kosten proportional zur Strahllänge unabhängig von der Szene; Banding bei zu großen Schritten (Gegenmittel: Dither)

Typische Anwendung: Juggernaut (God-Rays als Glow-Akkumulation im regulären Marsch), künftiges Volumetrik-Tutorial.

### 5.6 Analytische Schnitte

Wo eine geschlossene Schnittformel existiert, schlägt sie jede Iteration: Der Ebenen-Schnitt `t = -(ro.y - h) / rd.y` ist eine Zeile und exakt, der Kugel-Schnitt eine quadratische Gleichung. Analytische Schnitte sind streng genommen Raytracing, mischen sich aber nahtlos mit dem Marsch — die Serie nutzt sie für Lichtebenen, Portalebenen und den Planeten.

- Vorteile: exakt, kostenlos, keine Artefakte
- Nachteile: nur für wenige Grundformen verfügbar; keine Verzerrungen möglich

Typische Anwendung: Crystal-Lights (Lichtebene, Schritt 2 und 9), Space-Debris (Planetenkugel), Composite-Portals (Portalebene).

### 5.7 Vergleich und Entscheidungshilfe

| Variante | Schrittstrategie | SDF-Anforderung | Kosten | Robustheit |
|---|---|---|---|---|
| Klassisch | `t += d` | exakt oder unterschätzend | niedrig | gering |
| Gedrosselt | `t += d * (0.3..0.9)` | Schranke genügt | mittel | hoch |
| Über-Relaxation | `t += d * (1.2..1.6)` + Rückfall | exakt | niedrig (glatte Szenen) | mittel |
| Höhenfeld | `t += (p.y - h) * (0.3..0.5)` | Höhenfunktion | mittel–hoch | empirisch |
| Fixschritt | `t += konst` | keine (Dichtefeld) | hoch | hoch |
| Analytisch | Formel, keine Iteration | Grundform | minimal | maximal |

*Tab. 4: Marsch-Varianten im Vergleich*

| Situation | Empfohlene Variante |
|---|---|
| Grundkörper, Repetition, Faltung — keine Verzerrung | Klassisch |
| Twist, Displacement, Verschneidung, gemischte Materialien | Gedrosselt (Faktor an stärkster Verzerrung ausrichten) |
| Terrain, Terrassen, Lücken, „Landschaft von oben" | Höhenfeld-Marsch |
| Nebel, Lichtstrahlen, Dichte-Integrale | Fixschritt (oder Glow-Term im regulären Marsch) |
| Ebene, Kugel, Portal — Grundform ohne Verzerrung | Analytisch |
| Performance-Not bei glatter Szene | Über-Relaxation prüfen, sonst Budget/Drossel tunen |

*Tab. 5: Entscheidungshilfe für die Marsch-Wahl*

## 6. Raumoperationen

Raumoperationen sind der Grund, Raymarching überhaupt zu wählen: Sie verbiegen die **Eingabe** der SDF, nicht die Form selbst, und kosten deshalb unabhängig von der Szenenkomplexität konstant viel. Dieser Abschnitt dokumentiert die Operationen der Serie mit ihren Distanz-Konsequenzen — denn jede Operation beantwortet auch die Frage, ob die Garantie aus Abschnitt 3.2 erhalten bleibt.

### 6.1 Boolesche Operationen

Vereinigung, Schnitt und Differenz zweier Formen entstehen aus `min` und `max` der Einzeldistanzen. Vereinigung (`min`) ist distanz-exakt; Schnitt und Differenz (`max`) liefern nur eine Schranke — an den Verschneidungskanten unterschätzt die Funktion, was harmlos ist, kombiniert mit weiteren Operationen aber die Drossel-Empfehlung aus Tab. 5 begründet.

| Operation | Formel | Distanz-Qualität |
|---|---|---|
| Vereinigung | `min(d1, d2)` | exakt (außerhalb) |
| Schnitt | `max(d1, d2)` | Schranke |
| Differenz | `max(d1, -d2)` | Schranke |

*Tab. 6: Boolesche SDF-Operationen*

### 6.2 Weiche Verschmelzung (smin/smax)

Die harte `min`-Kante zweier Formen wird organisch, wenn die Operation in einer Übergangszone der Breite `k` weich mischt. Die polynomiale Standardform lautet:

```text
+----------------------------------------------------+
|  h    = max(k - |d1 - d2|, 0) / k                  |
|  smin = min(d1, d2) - h*h * k * 0.25               |
+----------------------------------------------------+
```

Legende: `d1`, `d2`: Einzeldistanzen [Szeneneinheiten] (beliebig) · `k`: Übergangsbreite [Szeneneinheiten] (0.05 bis 1.0, szenenabhängig) · `h`: normierte Nähe der beiden Distanzen [—] (0 bis 1) · `smin`: gemischte Distanz [Szeneneinheiten].

Das weiche Maximum entsteht analog per `smax(d1, d2, k) = -smin(-d1, -d2, k)`. Beide Funktionen unterschätzen in der Übergangszone zusätzlich — bei kleinen `k` vernachlässigbar, bei großen `k` ein weiterer Grund für die Drossel. Die Herleitung am Beispiel steht im [Juggernaut-Shader-Tutorial](Juggernaut-Shader-Tutorial.md) (Schritt 4).

### 6.3 Transformationen

Verschieben, Rotieren und (vorsichtig) Skalieren wirken invers auf den Abfragepunkt, bevor die SDF ihn sieht: `sdf(p - offset)` verschiebt die Form um `+offset`, `sdf(rotationsmatrix_invers * p)` rotiert sie. Starrkörper-Transformationen (Translation, Rotation) erhalten Distanzen exakt. Gleichmäßige Skalierung erfordert die Korrektur `sdf(p / s) * s`, sonst überschätzt die Funktion um den Faktor `s`; ungleichmäßige Skalierung ist keine Distanz-erhaltende Operation und verlangt eine Drossel um den größten Achsenfaktor.

### 6.4 Wiederholung und Faltung

Domain Repetition bildet den Raum per `mod` in eine Einheitszelle ab (`p.x = mod(p.x, zelle) - 0.5 * zelle`) und erzeugt unendlich viele Kopien zum Preis von null. Die eine Regel: Die Form muss samt aller Verformungen in ihre Zelle **passen** — ragt sie hinaus, sieht die SDF die Nachbarkopie nicht und überschätzt (das „Zellregel"-Motiv der Serie; formal bewiesen im [Space-Debris-Shader-Tutorial](Space-Debris-Shader-Tutorial.md), Schritt 4). Faltungen (`abs(p.x)` für Spiegelsymmetrie, `mod` auf dem Polarwinkel für Rotationssymmetrie) sind distanz-erhaltend bis auf die Faltkante, an der Formen abgeschnitten werden können; die Winkel-Faltung leitet das [Pyramid-Spiral-Shader-Tutorial](Pyramid-Spiral-Shader-Tutorial.md) (Schritt 9) her.

### 6.5 Verzerrungen und ihre Kompensation

Twist (tiefenabhängige Rotation), Bend und additive Verformung (`d + displacement(p)`) sind die Operationen mit echtem Preis: Sie stauchen den Raum lokal, die effektive Lipschitz-Konstante steigt über 1, und die SDF überschätzt. Als Faustformel für die Kompensation dividiert die Drossel durch die Verzerrungsstärke — ein Twist mit Winkelrate `w` und maximalem Radius `r` staucht tangential um bis zu `1 + w*r`, ein Displacement mit Amplitude `A` und Frequenz `f` um bis zu `1 + A*f`. Der zugehörige Drosselfaktor `lambda = 1 / (1 + Verzerrungsmass)` hält den Marsch beweisbar sicher; die Serie rundet in der Praxis großzügig ab (Twist 0.15 → Drossel 0.9).

## 7. Normalen und Ableitungen

Die Oberflächennormale ist der normierte Gradient der SDF: Die Funktion wächst senkrecht zur Oberfläche am schnellsten. Numerisch entsteht der Gradient aus Differenzen — zwei Verfahren sind Standard, beide funktionieren unverändert für **jede** Szene, die `map` beschreibt.

**Zentrale Differenzen** (6 Auswertungen) tasten je Achse vor und zurück; das Verfahren der Tutorial-Serie, robust und leicht herzuleiten. Der **Tetraeder-Trick** (4 Auswertungen) tastet entlang der vier Ecken eines Tetraeders und spart ein Drittel der Kosten bei gleichwertiger Qualität:

```glsl
// Normale per Tetraeder-Trick: 4 statt 6 map-Auswertungen.
vec3 calcNormalTet(vec3 p)
{
    const vec2 k = vec2(1.0, -1.0);
    const float e = 0.002;                 // Epsilon: siehe Text
    return normalize(k.xyy * map(p + k.xyy * e)
                   + k.yyx * map(p + k.yyx * e)
                   + k.yxy * map(p + k.yxy * e)
                   + k.xxx * map(p + k.xxx * e));
}
```

Die Epsilon-Wahl balanciert zwei Fehler: Zu kleine Werte rauschen (Auslöschung in der Gleitkomma-Subtraktion), zu große glätten scharfe Kanten weg. Bewährt ist die Größenordnung `0.002` in Szeneneinheiten, bei Höhenfeldern zusätzlich mit der Distanz skaliert (`e * (1 + t * 0.1)`), damit ferne Details nicht als Normalen-Flimmern aliasen — Herleitung im [Crystal-Lights-Shader-Tutorial](Crystal-Lights-Shader-Tutorial.md) (Schritt 5). Für Höhenfelder genügen vier Auswertungen der Höhenfunktion (zwei Achsen statt drei), die y-Komponente wird konstruiert.

## 8. Beleuchtung auf SDF-Basis

Beleuchtung im Raymarching kombiniert die klassischen Shading-Terme mit Effekten, die nur eine Distanzfunktion liefern kann. Die klassischen Terme — Lambert-Diffus (`max(dot(n, l), 0)`), Phong-Specular (`pow(max(dot(reflect(rd, n), l), 0), exponent)`) und Schlick-Fresnel (`pow(1 - max(dot(n, -rd), 0), 3..5)`) — sind in jedem Tutorial ab dem Licht-Schritt hergeleitet und werden hier nicht wiederholt. SDF-spezifisch sind vier Techniken:

**Weiche Schatten** entstehen aus einem zweiten Marsch vom Oberflächenpunkt zur Lichtquelle. Statt binär „getroffen/frei" wird der engste Vorbeigang protokolliert — wie knapp der Schattenstrahl an Geometrie vorbeischrammt, bestimmt die Halbschatten-Stärke:

```text
+----------------------------------------------------+
|  s = min ueber alle Schritte von ( k * d / t )     |
+----------------------------------------------------+
```

Legende: `s`: Schattenfaktor [—] (0 = Kernschatten, 1 = frei) · `d`: SDF-Wert am Schattenstrahl [Szeneneinheiten] · `t`: Strecke auf dem Schattenstrahl [Szeneneinheiten] (klein anfangen, siehe Text) · `k`: Härtefaktor [—] (2 = sehr weich, 32 = hart).

Der Startpunkt muss um ein Vielfaches des Treffer-Epsilons von der Oberfläche abgehoben werden, sonst verschattet sich der Punkt selbst (Schatten-Akne). Weiche Schatten sind bisher **nicht** Teil der Tutorial-Serie (Ausblick „Licht & Schatten" in der Wegleitung) — diese Referenz dokumentiert sie, weil sie zum Kernwerkzeugkasten des Verfahrens gehören.

**Ambient Occlusion** nutzt die SDF als Enge-Messgerät: Entlang der Normale wird in wenigen festen Abständen gefragt, ob so viel Freiraum existiert, wie der Abstand erwarten ließe. Die gewichtete Differenz ergibt Verdunkelung in Spalten und Ecken — fünf Auswertungen genügen für glaubwürdige Kontaktschatten.

**Glow** akkumuliert während des regulären Marschs einen Term `k / (a + d*d)` pro Schritt: Strahlen, die knapp an Geometrie vorbeilaufen, sammeln Leuchten auf. Das ist billiger Ersatz für echte Volumetrik und in Pyramid-Spiral (Schritt 14) sowie Juggernaut (God-Rays) hergeleitet.

**Brechung und Absorption** kombinieren `refract` an der Auftreff-Normale mit Beer-Lambert-Absorption (`exp(-dicke * sigma)`) auf dem Innenweg; die vollständige Herleitung samt Eis-Färbung steht im [Crystal-Lights-Shader-Tutorial](Crystal-Lights-Shader-Tutorial.md) (Schritt 10).

## 9. Kamera-Modelle

Die Kamera des Raymarchings ist die Vorschrift, die jedem Pixel seinen Strahl `(ro, rd)` zuweist; alles Weitere ist der Marsch. Drei Modelle decken die Serie ab. Die **Lochkamera** baut aus Blickrichtung, Rechts- und Hoch-Vektor eine Basis und kippt die Strahlrichtung pro Pixel (`rd = normalize(fw * brennweite + rt * uv.x + up * uv.y)`); die Brennweite steuert das Sichtfeld. Die **Parallelprojektion** (isometrische Sicht) hält die Richtung konstant und verschiebt stattdessen den Ursprung pro Pixel — Fluchtpunkte verschwinden, ferne Objekte bleiben groß. Beide Modelle unterscheiden sich nur darin, ob der Pixel-Offset auf `rd` oder auf `ro` addiert wird; deshalb existiert eine stufenlose **Überblendung** zwischen ihnen, die im [Crystal-Lights-Shader-Tutorial](Crystal-Lights-Shader-Tutorial.md) (Schritt 12) hergeleitet ist und wie ein Dolly-Zoom wirkt.

Für Kamera-**Bewegung** gilt die wichtigste Regel des zustandslosen Renderings: Positionen als glatte Funktionen der Zeit formulieren (`sin`-Bahnen mit Cosinus-Geschwindigkeit für weiche Umkehr), niemals Geschwindigkeiten auf den Faktor vor der Zeit legen — ein Shader kann ohne Zustandsspeicher keine wechselnde Geschwindigkeit zu einer stetigen Strecke integrieren. Herleitung und Choreografie-Muster (inkommensurable Uhren): Crystal-Lights Schritt 13; die Ausnahme mit Zustandspuffer: [Composite-Transitions-Shader-Tutorial](Composite-Transitions-Shader-Tutorial.md) (Schritt 11).

## 10. Stabilität, Artefakte und Performance

### 10.1 Artefakt-Katalog

Tab. 7 sammelt die typischen Fehlerbilder des Verfahrens mit Ursache und Standard-Gegenmittel. Die ersten drei sind Marsch-Fehler, die letzten drei Abtast-Fehler.

| Symptom | Ursache | Gegenmittel |
|---|---|---|
| Löcher/Durchschuss in dünner Geometrie | SDF überschätzt (Verzerrung, Zellregel verletzt, fehlende Normierung) | Drossel senken; Verzerrung mildern; Zellgrößen nachrechnen |
| Ausgefranste Silhouetten, fehlende Kanten | Schrittbudget an streifenden Strahlen erschöpft | Budget erhöhen; Toleranz mit t wachsen lassen; Nebel kaschiert Rest |
| Ringe/Terrassen auf Flächen (Banding) | Treffertoleranz zu groß oder Fixschritt zu grob | Epsilon senken; Dither auf Startdistanz; mehr Schritte |
| Flimmern feiner Details in der Ferne | Sub-Pixel-Geometrie, Normalen-Epsilon zu fein | Epsilon distanzskaliert; Detail-Amplitude mit t ausblenden |
| Harte Treppenkanten an Silhouetten | Ein Strahl pro Pixel (Aliasing) | fwidth-Masken; Supersampling (Kostenrechnung: Composite-Portals 10) |
| Schatten-Akne / Selbstverschattung | Sekundärstrahl startet in der eigenen Oberfläche | Startpunkt um Vielfaches des Epsilons abheben |

*Tab. 7: Artefakte, Ursachen, Gegenmittel*

### 10.2 Performance-Grundsätze

Aus dem Kostenmodell `O(P * N * C_map)` folgen die drei wirksamen Hebel in absteigender Reihenfolge: Erstens `C_map` senken — teure Terme (Noise-Oktaven, Voronoi-Schleifen) nur dort auswerten, wo sie das Bild tragen, und alles Framekonstante vor den Marsch ziehen (Audio-Pegel, Paletten). Zweitens Strahlen früh beenden — Szenengrenze `t_max` eng setzen, Himmel analytisch statt per erschöpftem Budget. Drittens Sekundärstrahlen rationieren — Normalen einmal pro Treffer, Schatten/AO mit kleinen Festbudgets, Brechung analytisch abkürzen, wo möglich. Supersampling multipliziert alle Kosten mit der Sample-Zahl und steht deshalb am Ende jeder Optimierungsliste, nicht am Anfang.

## 11. Schnellreferenz

Die Schnellreferenz komprimiert das Dokument auf die Nachschlage-Essenz: Tab. 8 die Werkzeuge, Tab. 9 die Zahlenbereiche.

| Aufgabe | Werkzeug | Ein-Zeilen-Form |
|---|---|---|
| Kugel | SDF | `length(p) - r` |
| Box | SDF | `length(max(abs(p)-b, 0.)) + min(max(q.x,max(q.y,q.z)),0.)` |
| Vereinigung / Schnitt / Differenz | Boolesch | `min(a,b)` / `max(a,b)` / `max(a,-b)` |
| Weiche Vereinigung | smin | `min(a,b) - h*h*k*0.25`, `h = max(k-abs(a-b),0.)/k` |
| Wiederholen | mod | `p.x = mod(p.x, z) - 0.5*z` (Zellregel!) |
| Spiegeln / Sektor-Falten | abs / mod(Winkel) | `p.x = abs(p.x)` / `a = mod(a, s) - 0.5*s` |
| Marsch-Schritt | Sphere Tracing | `t += map(ro + rd*t) * lambda` |
| Normale | Tetraeder | 4 map-Taps, siehe Abschnitt 7 |
| Weicher Schatten | Sekundär-Marsch | `s = min(s, k*d/t)` |
| Glow | Akkumulation | `glow += k / (a + d*d)` je Schritt |
| Ebenen-Schnitt | analytisch | `t = -(ro.y - h) / rd.y` |

*Tab. 8: Werkzeugkasten in Kurzform*

| Parameter | Startwert | Bereich |
|---|---|---|
| Schrittbudget | 100 | 80–200 |
| Drossel (exakte SDF / verzerrt / Höhenfeld) | 1.0 / 0.9 / 0.4 | 0.3–1.0 |
| Treffertoleranz | `0.001 + 0.0015*t` | Maßstabsabhängig |
| Normalen-Epsilon | 0.002 | 0.001–0.01, distanzskaliert |
| Szenengrenze t_max | 40 | An Nebel koppeln |
| smin-Breite k | 0.2 | 0.05–1.0 |
| Schatten-Härte k | 8 | 2–32 |

*Tab. 9: Faustwerte der Marsch-Parameter (Szeneneinheiten wie in der Tutorial-Serie)*

## 12. Glossar

| Begriff | Definition | Symbol/Einheit |
|---|---|---|
| **Ambient Occlusion** | Verdunkelung enger Raumbereiche, hier aus wenigen SDF-Proben entlang der Normale geschätzt. | — |
| **Banding** | Sichtbare Terrassen/Ringe durch zu grobe Abtastung oder zu große Treffertoleranz. | — |
| **Beer-Lambert-Gesetz** | Exponentielle Lichtschwächung auf dem Weg durch absorbierendes Material. | `exp(-d·σ)` |
| **Distanzschranke (bound)** | Funktion, die die wahre Distanz nie über-, aber möglicherweise unterschätzt; für den Marsch zulässig. | — |
| **Domain Repetition** | Unendliche Wiederholung einer Form durch `mod`-Abbildung des Raums in eine Einheitszelle. | — |
| **Drosselfaktor** | Multiplikator unter 1 auf der Marsch-Schrittweite zur Kompensation überschätzender SDFs. | λ [—] |
| **Fresnel-Effekt** | Zunahme der Spiegelung bei streifendem Blickwinkel. | — |
| **Gradient** | Vektor der stärksten Funktionszunahme; bei SDFs die Oberflächennormale. | ∇d |
| **Lipschitz-Konstante** | Obere Schranke der Änderungsrate einer Funktion; Marsch-sicher bei L ≤ 1. | L [—] |
| **Raymarching** | Iteratives Voranschreiten eines Strahls durch ein implizit definiertes Feld bis zum Treffer. | — |
| **Raytracing** | Strahlbasiertes Rendern mit analytischer Schnittberechnung gegen explizite Geometrie. | — |
| **SDF** | Signed Distance Function — vorzeichenbehaftete Distanzfunktion; negativ innen, null auf der Oberfläche, positiv außen. | d(p) [Szeneneinheiten] |
| **Sphere Tracing** | Die Standard-Marschstrategie: Schrittweite = aktueller SDF-Wert (garantiert leerer Kugelradius). | — |
| **Supersampling** | Mehrere Strahlen pro Pixel gegen Aliasing; multipliziert die Gesamtkosten. | — |

*Tab. 10: Glossar*

## 13. Siehe auch

**Voraussetzungen**

- [Pyramid-Spiral-Shader-Tutorial](Pyramid-Spiral-Shader-Tutorial.md) — führt Fragment-Shader-Grundlagen und jeden Kernbegriff dieses Dokuments schrittweise am lauffähigen Beispiel ein

**Verwandte Dokumente**

- [Shader-Tutorials-Wegleitung](Shader-Tutorials-Wegleitung.md) — Lesehilfe der Serie samt Technik-Index (Konzept → Tutorial + Schritt)
- [Crystal-Lights-Shader-Tutorial](Crystal-Lights-Shader-Tutorial.md) — Höhenfeld-Marsch, Brechung/Absorption, Kamera-Modelle
- [Space-Debris-Shader-Tutorial](Space-Debris-Shader-Tutorial.md) — 3D-Repetition, Zellregel-Beweis, Rotation je Zelle
- [Juggernaut-Shader-Tutorial](Juggernaut-Shader-Tutorial.md) — smin/smax, Verschneidungs-Greebles, Glow-Akkumulation
- [Composite-Portals-Shader-Tutorial](Composite-Portals-Shader-Tutorial.md) — Material-Id-Dispatch, Anti-Aliasing, Marsch zweier Welten

**Weiterführend (extern)**

- Inigo Quilez: [distfunctions](https://iquilezles.org/articles/distfunctions/) — die maßgebliche SDF-Formelsammlung
- Inigo Quilez: [raymarching terrains](https://iquilezles.org/articles/terrainmarching/) — Höhenfeld-Marsch in der Tiefe
- [Shadertoy](https://www.shadertoy.com) — Referenzplattform aller Code-Beispiele

## 14. Changelog

| Version | Änderung |
|---|---|
| 1.0.0 | Erstfassung: Algorithmus, SDF-Katalog, sechs Marsch-Varianten mit Entscheidungshilfe, Raumoperationen mit Lipschitz-Betrachtung, Normalen, SDF-Beleuchtung, Kamera-Modelle, Artefakt-Katalog, Schnellreferenz. Anmerkung: Programmiersprache GLSL ist im Catalog_Programming (Tab. 5c) noch nicht geführt — Catalog-Ergänzung ausstehend. |

*Tab. 11: Versionshistorie*

