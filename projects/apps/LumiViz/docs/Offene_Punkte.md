# LumiViz — Offene Punkte (Arbeitsliste)

> **Version:** 1.303.0
> **Datum:** 2026-10-30 (Sessions 164 bis 167)
> **Typ:** Status/Arbeitsliste
> **Status:** Aktiv — **SSOT für „was ist noch offen"**
> **Sprache:** Deutsch
> **Ersetzt:** `Offene_Implementierungen.md` + `Offene_Sichttests.md` (beide standen
> auf Session 37/38 und waren in weiten Teilen überholt — Git hat sie)

Dieses Dokument beantwortet **eine** Frage: was ist noch zu tun. Die Detail-Konzepte
bleiben, wo sie sind; hier steht je Punkt nur so viel, dass man ihn aufgreifen kann,
plus der Verweis auf die Quelle.

**Pflege:** am Ende jeder Session nachziehen — zusammen mit dem Handover
(`.claude/handover/HANDOVER.md`, lokal). Wo beide sich widersprechen, gilt das
Handover; dann dieses Dokument nachziehen.

Legende: 🔴 blockiert anderes · 🟠 Befund mit Messwert · 🟡 Entscheid nötig ·
⬜ Sichttest/Urteil offen · ⚪ Backlog (bewusst nichts tun) · 🔧 Kleinkram

> ## ✅ Erledigt in S66 (Aufträge Patrik)
>
> **Puffer-Wechsel-Schalter** (Anlass: „Rock The House sieht je nach Vorgänger
> anders aus" = S63-Feedback-Erbe): je Milkdrop-Node Behalten/Löschen/Fading
> (einmaliger Mix, Erbe-Regler)/**Ausblenden über Zeit** (Echo-Dämpfung je
> Frame nach dem Warp, Sekunden-Regler), App-Default im Settings-Panel;
> Sichttest Behalten/Löschen bestanden, **Sichttest Ausblenden offen**.
> Merke: Löschen = Rausch-Saat + rand_preset-Seed-Reset, NICHT schwarz
> (Verstärker-Presets) und OHNE Loudness-Reset (loadDiag: Musik auf leerer
> Warmup-Rampe ⇒ bass≈17-Explosion — erprobt und verworfen).
>
> ## ✅ TOP 1 GELÖST in S67 — Preset-Wechsel-Divergenz („RTH erbt Farbe")
>
> **Ursache gefunden und gefixt (S67):** `ensureCustomPrograms()` stand in
> `onRender` hinter dem Gate „nur wenn Custom-Quellen vorhanden" — beim
> In-Place-Wechsel auf ein Md1Default-/None-klassifiziertes Preset (RTH_2024:
> Warp `ret*0.93`, Comp Passthrough) wurde der Rev-Wechsel nie verarbeitet und
> die **Warp-/Comp-GL-PROGRAMME DES VORGÄNGERS renderten das neue Preset
> weiter** (Trace-Signatur: `warpSrc=nein … Branch warp=CUSTOM`). Bei aa9dc9d0
> unsichtbar (jeder Wechsel = frische Instanz), in der Triage unsichtbar (ein
> Preset je Lauf). Beweis: **MilkdropStandalone 1.1.0 `--ab`-Wechsellauf**
> (FNV-Hash je Frame): vorher Spotlight→RTH vs. Beauty→RTH **0/300** gleiche
> Frames (rot-abklingend vs. weiß-aufhellend) trotz Trace-bewiesenem Wipe —
> nachher **300/300 bitgleich**. Kaltstart vs. Wechsel+Audio-Neustart
> konvergiert ab ~f120 (Rest = bewusst behaltene Loudness-Historie).
> Doku: MilkdropVisualizer.md 1.22.0. Tests grün, alle 3 Builds grün.
> **App-Sichttest Fix + Sicht-Blende: ✅ Patrik (S67, „passt nun").
> Noch offen aus dem S66-Plan: Sichttest Ausblenden-Modus.**
>
> ## 🟡 S67 — Rausch-Saat sichtbar: UNTERSUCHT, Entscheid offen
>
> **Befund Patrik (S67):** Beim Preset-Wechsel und nach App-Start ist bei
> fast allen Presets zu Beginn Rauschen sichtbar — „das war zuvor nie".
> **Untersuchung (S67, abgeschlossen):** Quelle geklärt — Patriks QSettings
> stehen seit S66 auf `milkdrop/pufferWechsel=loeschen`; Löschen wipet auf die
> S63-Rausch-Saat, die der ehrliche Decay-Pfad ~0,5–1 s zeigt. Vor S66 war
> effektiv immer Behalten aktiv (Code-Default `behalten`) — nie Rauschen.
> Der Start-Fall ist die S63-Kaltstart-Saat (Puffer-Erzeugung beim ersten
> Load). **Messung Standalone `--ab --wechsel behalten` (mit S67-Fix):**
> kein Rauschen, Vorgänger-Einfluss bei RTH_2024 nach ~0,5 s optisch weg,
> ab f0120 (2 s) BIT-IDENTISCH über verschiedene Vorgänger — und ab ~2 s
> sogar bitgleich mit dem Löschen-Lauf (Modus konvergiert).
> **Empfehlung: App-Default zurück auf Behalten** (Settings-Panel → Keep);
> Löschen bleibt als Spezialwerkzeug je Node. **✅ Sicht-Blende UMGESETZT
> (S67, Entscheid Patrik „mit Settings-Schalter"):** `setSichtBlende` im
> Kern (Ease-in ~0,5 s über Kaltstart-/Resize-Saat und Löschen-/Fading-Wipe,
> Composite-seitig — f60-Hash mit/ohne Blende identisch = beweisbar rein
> kosmetisch), QSettings `milkdrop/sichtBlende` Default AN, Settings-Panel
> „MilkDrop Start Fade-in", Standalone `--blende`; Prüfstände unverändert
> (Kern-Default AUS). Handbuch 1.6.0. **Sichttest: ✅ Patrik (S67).**
>
> **Benutzerhandbuch 1.5.0** (Behalten-Semantik exakt, Regelwerk,
> §12 Shadertoy, GPU, 2 Screenshots — skriptgesteuert aus der App).
> **Panel-Komfort:** Splitter Baum/Editor (Stellung persistiert),
> Panel-Mindestbreite + sizeHint-Default 560×780, Milkdrop-Sektionen in
> Pipeline-Reihenfolge mit GETRENNTEN Warp-/Comp-Sektionen. Feld-Inventar 726.
> **NEU OFFEN (Entscheid Patrik, S66): Meganode-Split-Strang** — MilkDrop-
> Stufen-Container (Waves/Shapes/…) im Baum UMORDENBAR mit echter Pipeline-
> Wirkung + zusätzliche Container für AVS-/LumiViz-Renderer MITTEN in der
> MilkDrop-Pipeline; Import-Default bleibt IMMER die Original-Reihenfolge.
> Braucht Konzept-Kapitel in `visuals/Vereinheitlichung_Konzept.md`
> (Puffer-Übergabe FeedbackBuffer↔Chain-Buffer, Y-Konvention, Datenmodell
> Stufen-Reihenfolge + Serialisierung mit Migration).

> ## ✅ Erledigt in S65 (Aufträge Patrik)
>
> **Strang R (Regelwerk Legacy/Modern) KOMPLETT + abgenommen** — Triage s65
> klassengleich s64f (0 Wechsel, OK 295), Modern-Stichprobe beweist die Schalter.
> **Strang S (Shadertoy) S1–S4 KOMPLETT** — Node host-nativ, 512×2-Audio,
> URL-/ID-Import (Netz-Abnahme braucht API-Key), Multipass Buffer A–D,
> NEU ShadertoyBrowserPanel. Dazu Editor-Komfort (ⓘ/⤢/Tooltips für GLSL/HLSL),
> Speichern-Namensvorschlag und der **100-Shader-Vorrat** in `asset/shadertoys/`
> (fürs Shadertoy-Konto → API-Key). Details: `Changelog_Session65.md`,
> Plan-Doku `visuals/Regelwerk_und_Neue_Module_Plan.md` (1.4.0).
> **NEU OFFEN:** Sichttests neue UI (Patrik) · S3-Netz-Abnahme mit Key ·
> ~~Audio-Skala dB-vs-linear~~ **✅ S67: Sonde gefahren + dB umgesetzt**
> (WebAudio-Vertrag in der Audio-Textur, ShadertoyWrapper.md 1.2.0; dazu NEU
> `getspecdb()` für eigene Chain-Skripte — Idee Patrik; Absolut-Offset-
> Feinabgleich wandert in die S3-Netz-Abnahme) ·
> Shader-Feintuning nach Ton-Test (45/56/64 vorgemerkt).

> ## ✅ Erledigt in S53 (war die Vorgabe Patrik aus S52)
>
> **Panel-Editoren für `Metaballs 3D` und `Tentacles 3D`** — beide Knoten haben
> jetzt volle Editoren im `MultiEffectPanel` (Metaballs: Kugelzahl, Radius,
> Tempo, Isowert, Blend · Tentacles: Zahl, Segmente, Länge, Dicke, Tempo, Blend),
> je mit editierbarer Farbtafel, und stehen in der Palette („Scopes & Sources").
> Die Farbtafel-Zeile ist dabei aus dem SuperScope-Block herausgelöst und
> geteilt worden (`addColorTable`).

---

## 0. Fertig-Kriterium der Kalibrier-Runde — ✅ ERFÜLLT (S60, 2026-08-01)

**Alle sechs Haken sitzen — die Kalibrier-Runde (S44–S60) ist abgeschlossen.**
Von den zwei benannten §1-Treue-Resten außerhalb der Haken ist **Tie Tunnel
seit S61 GELÖST** (Dot-Plane-Startwinkel, s. §1); es bleibt **VR09-AVI**
(reine Indeo-Decoder-Differenz) — §9-Kandidat nach Sichturteil Patrik mit Ton.

Die Kalibrier-Runde (lief seit S44) ist **fertig**, wenn alle sechs Haken
sitzen. Das Kriterium selbst ist **eingefroren**: neue Befunde, die schärfere
Metriken noch ausgraben, verlängern diese Liste nicht — sie fallen unter
Haken 2 (fixen oder als bewusste Grenze in §9 abnehmen). Alles, was nicht
unter §0 fällt, ist Nach-Kalibrier-Arbeit, lebt in §6/§7 und taucht in
Session-Plänen erst wieder auf, wenn §0 abgehakt ist.

| # | Haken | Stand (S60) | |
|---|---|---|---|
| 1 | **Modul-Sonden grün** — und halten es bis zum Abschluss | ✅ **91/91** (S60-Ende; +2 wrap-Sonden, nach allen Umbauten gehalten) | ✅ |
| 2 | **Modul-Matrix 43/43** nach Mittelwert- **und** Flächen-Urteil. Eine Zeile, die nicht grün wird, darf nur über einen begründeten Eintrag in §9 schließen, **mit Abnahme Patrik** (Kandidaten: `water`-Chaos-Rauschen · `grain`-Zufallsstrom · ggf. Tie Tunnel) | ✅ **43/43 — ERFÜLLT (S60):** 41 Zeilen grün gemessen (S60 gelöst: `31_water_bump` Uniform-Fix · vier Flächen-Befunde PIXELGENAU · `04_blitter_feedback` nearest+y-Anker) + `20_water`/`24_grain` **von Patrik als bewusste Grenze abgenommen** (§9, Montagen; Zeilen bleiben Wächter) | ✅ |
| 3 | **Feld- und Edit-Sonden-Vollauf grün** (kein WIRKUNGSLOS, kein unerklärter STUMM) — stand seit S57 aus | ✅ **erfüllt (S60):** Edit-Vollauf **567 GLEICH · 127 TEILWEISE · 10 VERDECKT · 0 WIRKUNGSLOS** · Feld-Vollauf nach dem waterBump-Fix **674 WIRKT · 29 SCHWACH · 1 STUMM** — der eine ist `convolution.edgeMode`, der dokumentierte Schatten der offenen wrap-Arithmetik (Haken 5); der erste Lauf (664/29/11) hatte die waterBump-Regression gefunden (§1). waterBump-Edit-Sonden 1/9/0 nachgeholt | ✅ |
| 4 | **Abschluss-Messlauf der abgenommenen Kern-Presets** in beiden Größen, ohne Regression | ✅ **erfüllt (S60):** 24/24 s-Kalibrier-Sonden UND 8/8 Kern-Presets OK in 320×240 + 640×480 (Alternate Reality 0,002 · splendora 0,003 · hypno07 0,008 · Lost Cause 0,002 · Bright Light District 0,002 · Rotor 0,011 · Real Impressionist 0,008 · Deep Red Sea 0,004) — alle auf/unter Abnahme-Niveau | ✅ |
| 5 | **Mess-Werkzeug-Schulden** | ✅ **alle drei erledigt (S60):** (a) `srand` nach Preset-Laden war seit S52 in `avsref_main` drin — der §8-Eintrag war eine Doku-Leiche · (b) **`r_avi`-Uhr virtualisiert** (`patched/r_avi.cpp`, `avsref_tick()` über `g_avsref_tick_ms`) — AVI-Presets sind messbar; erste VR09-Messung deckt sofort einen App-Befund auf (§1) · (c) **wrap-Arithmetik an der Original-APE vermessen und umgesetzt** (2 Dauersonden + scale-2/4/128-Grenztest: scale 1 = Sättigung, scale ≥ 2 = (pos−neg) mod 65536 mit UNSIGNED-Division — 254/255-Werte exakt getroffen); Modul-Sonden **91/91**, `convolution.edgeMode` **WIRKT** (Grundkonfig scale 2). ~~Trail-Kreislauf-Sonde~~ (hinfällig S60) | ✅ |
| 6 | **Offene Entscheide getroffen** oder ausdrücklich nach §7 verschoben: die §5-Tabelle · Feldreihenfolge (§7) · Umfang + Namenskonvention der Basis-Voreinstellungen (§7 — Entscheid vorziehen, Umsetzung nach §0) | ✅ **komplett (S60)** — §5 abgeräumt (Hotkeys, Playlist, Lights→Backlog, Config-Pipeline-Tabelle gestrichen, MilkDrop-Texturen beschafft), Feldreihenfolge bleibt Ist, Vorlagen-Konvention steht | ✅ |

**Nicht Teil von §0** (ausdrücklich danach): Basis-Voreinstellungen je Modul
(§7) · Vereinheitlichung V2–V5 (§6) · Video-Weg Stufe 2 (§7) ·
Grafikkarten-Auswahl (§8, braucht den Vorher/Nachher-Messlauf) · die
Seite-an-Seite-Urteile und Sichttest-Rückstände (§2–§4) — Handarbeit Patrik,
eigener Strang, blockieren das maschinelle Kriterium nicht.

## 1. AVS-Kalibrierung — offene Befunde mit Messwert

### 🟠 Listen-Blend: Adjustable sättigt bei uns ab 128 (S74, Spur)

Prüfstand: Gitter-Raster als Untergrund, Verlauf als Prüf-Knoten in der Liste,
`blendin = 10` (Adjustable), `inblendval` über die Skala. Verglichen wurden
nicht nur die Messwerte, sondern die **Bild-Gruppierung je Seite**:

| `inblendval` | 0 | 64 | 128 | 192 | 255 |
|---|---|---|---|---|---|
| Referenz | A | B | C | D | E — **fünf verschiedene** |
| LumiViz | A | B | C | C | C — **drei verschiedene** |

Ab 128 ändert sich bei uns nichts mehr, die Referenz läuft bis 255 weiter.
Verdacht: die Alpha-Skala wird als 0…128 statt 0…255 gelesen, oder sie wird
bei 0,5 geklemmt.

Dieselbe Auswertung zeigt zwei weitere Modi, die bei uns auf „Replace"
zusammenfallen und in der Referenz eigenständig sind: **7 (Multiplikativ-Kette)**
und **10 (Adjustable)**.

**Noch nicht quantifiziert.** Der Messwert bleibt in allen Fällen bei MAE
0,002 — die Abweichung betrifft zu wenig Fläche. Ein Gegenversuch mit zwei
deckenden Flächen (Verlauf + Graukeil) hat die Blende gar nicht angeregt:
dort kollabieren **beide** Seiten auf ein einziges Bild. Es fehlt eine Probe,
die die Blende flächig fordert und dabei beide Ebenen sichtbar lässt.

**Sauber bestanden im selben Lauf:** alle 13 Blend-In-Modi (MAE ≤ 0,003),
beide Reihenfolgen (der Unterschied durch die Reihenfolge tritt auf beiden
Seiten gleich auf), und die Buffer-Proben — letztere allerdings **ohne
Aussagekraft**: mit unbeschriebenen Buffern liefern beide Seiten je ein
einziges Bild, die Probe unterscheidet nicht. Buffer braucht einen eigenen
Prüfstand mit BufferSave/BufferRestore.

### ✅ SuperScope-Punktzahl: Grenze auf Original-Niveau (S74, behoben)

`SuperscopeModule.cpp` klemmte `n` auf **4096**, das Original
(`r_sscope.cpp:282`) auf `128*1024` = **131072** — Faktor 32 zu niedrig.
**Jedes AVS-Preset mit mehr als 4096 Punkten rendert abgeschnitten**, und zwar
unauffällig: das Bild sieht vollständig aus, es endet nur früher.

Behoben an drei Stellen über eine gemeinsame Konstante
`SuperscopeModule::kMaxPointCount` (Setzen, Skript-`n`, Editor-Obergrenze).
Keine Puffergrenze dahinter — Punkt- und Vertex-Listen wachsen dynamisch.

Gitter-Raster allein gegen die Referenz, vorher/nachher:

| Gitter | Punkte | vorher | nachher |
|---|---|---|---|
| 64 | 4096 | 0,001 | 0,001 |
| 72 | 5184 | 0,056 | **0,001** |
| 80 | 6400 | 0,100 | **0,001** |
| 96 | 9216 | 0,165 | **0,001** |
| 128 | 16384 | — | **0,002** |

Regression: die zehn mitgelieferten Presets liefern unverändert dieselben
Werte (5/10 OK) — keines nutzt mehr als 4096 Punkte. Tests 593 grün.

### 🟠 `07_movin wall`: die SuperScopes tragen, das Movement verstärkt (S75)

**Die S74-These ist widerlegt** — der Befund ist umgedeutet. Kurzfassung:

| Sonde (Raster, 120 Frames, 320×240) | MAE |
|---|---|
| Movement (Id 15) **solo, in Rückkopplung** | **0,001** |
| SuperScope `l03` solo | 0,012 |
| SuperScope `l06` solo | 0,009 |
| **`l06` + Movement** | **0,109** |
| `l06` + Movement, `ti` fest statt `getspec` | 0,063 |

**1. `g_blendtable` nachzubilden wäre Arbeit an totem Code.** `BLEND4`
(`r_defs.h:328`) hat zwei Zweige. Der `NO_MMX`-Zweig mit `g_blendtable` und der
`/255`-Normierung **lässt sich nicht einmal kompilieren**: `mmx_adjblend_block`
(:642) ruft dort ein nicht existierendes `inblendval` (gemeint war `v`), und
`BLEND_ADJ` ist unter `NO_MMX` gar nicht definiert. Gegengeprüft mit einem
AvsRef-Build mit `-DNO_MMX` — er bricht mit `C2065` ab. `NO_MMX` wird zudem
nirgends definiert, weder im Original noch in `tools/AvsRef`.

Was AVS **ausführt**, ist der MMX-Zweig: separabel in zwei Stufen,
`(a·(255−p) + b·p) >> 8`, drei Trunkierungen. Genau das rechnet unser Shader
(`MultiEffectVisualizer.cpp:843-845`) in Ganzzahlen nach — seit S46 und
korrekt. Auch `BLEND_AVG` (:226) und die Tabellen-Erzeugung
(`r_trans.cpp:425-500`, Skript-Zweig mit `is_rect`) stimmen Zeile für Zeile mit
`ScriptGridModule.cpp:295-365` überein.

**2. Das Movement ist entlastet.** Bisher war es nur einmalig auf Raster
gemessen. Solo über 120 Frames — also mit der Rückkopplung, die den Verdacht
begründet hatte — liefert es **0,001**.

**3. Es ist ein Verstärker.** Das Preset-Movement ist
`y=(y+1)*((1-y)); x=(1+x)*(1-x)` (`effect=32767`, `rectangular=1`,
`subpixel=1`, `wrap=0`, `blend=0`) — eine starke Vergrößerung. Sie bläst den
1-Pixel-Kantenversatz des SuperScope von 0,009 auf **0,109** auf, mehr als das
Gesamt-Preset (0,073). Dass `l06` stärker durchschlägt als `l03` (0,015), passt
zur Lage: `l06` zeichnet mit `y=-sin(t)*u` in die Bildhälfte, die der Warp am
stärksten dehnt.

**4. Die Zeitdrift ist nicht die Ursache.** Beide Scopes tragen
`frame: t=t-ti` mit `beat: ti=getspec(0.7,0.5,0)` — ein Akkumulator an einem
Audio-Wert, der bei kleinster Abweichung linear driftet. Mit festem `ti` bleibt
die Kombination bei **0,063**. (Die Zahl ist *nicht* gegen 0,109 aufrechenbar —
festes `ti` erzeugt eine andere Figur; aussagekräftig ist allein, dass der
Fehler bleibt.) Solo ändert sich gar nichts: 0,009 mit und ohne.

**Der eigentliche Befund ist damit die SuperScope-Rasterung** — die
Subpixel-Lage der Linien, dieselbe Familie wie der S58-Befund „Pixelmitte der
Scope-Linien" bei `30 Bright Light District`. Prüfstand steht
(`solo_l03`/`solo_l06` + Raster); der Movement ist als Lupe brauchbar, um den
Versatz sichtbar zu machen.

<details>
<summary>S74-Fassung (widerlegt) — Movement-Subpixel schaukelt sich auf</summary>

Das Preset war der Fall „kein Einzelknoten auffällig, ganzes Preset bei 0,073".
Mit Raster gemessen sind alle 9 Knoten einzeln sauber (max. 0,015). Die
kumulative Bisektion zeigt zwei Sprünge:

| Stufe | dazu | MAE |
|---|---|---|
| l01–l06 | APE, UniqueTone, SuperScope, FastBrightness, Fadeout, SuperScope | ≤ 0,005 |
| **l07** | **+ Movement (Id 15)** | **0,037** |
| t02 | + UniqueTone (Id 38) auf Wurzelebene | 0,077 |

**Der Frameverlauf entscheidet den Charakter** (Regel 7): der Fehler wächst
stetig — 0,0032 (Frame 4) · 0,0060 (13) · 0,0084 (25) · 0,0106 (28) · 0,037
(120) — und unser Mittelwert liegt durchgehend **leicht über** der Referenz.
Das ist kein Rechenfehler je Frame, sondern ein kleiner Überschuss, der sich
in der Rückkopplung aufsummiert.

Eingegrenzt auf die **Subpixel-Interpolation** des Movement. Die
Adress-Tabelle ist originalgetreu portiert, samt 5-Bit-Quantisierung der
Anteile (`ScriptGridModule.cpp:342-365`). Der Unterschied steckt im
Verrechnen:

| | Original (`BLEND4`, `r_defs.h:328`) | Wir (`kMovementShader`) |
|---|---|---|
| Gewichte | vier vorab, je über `g_blendtable` **abgeschnitten** | separabel in zwei Schritten |
| Normierung | **/255** — die Tabelle ist `(unsigned char)((i/255.0)*j)` | **`>>8`, also /256** |
| Abschneide-Punkte | 8 (4 Gewichte + 4 Produkte) | 3 |

Die Anteile gehen im Original als `(transp>>24) & (31<<3)` hinein, also 0…248
in Achterschritten — nie 255. Bei einem Bildmittel von 0,09 (≈ 23/255) ist ein
Unterschied von 1 LSB bereits ~4 %, und genau in dieser Größenordnung liegt
die gemessene Abweichung je Frame.

**Nicht geändert.** Die `/256`-gegen-`/255`-Normierung ist ein belegter
Vertragsunterschied, ihr Vorzeichen allein erklärt die Richtung aber nicht —
sie macht uns dunkler, gemessen sind wir heller. Bit-Treue hier herzustellen
heißt, `g_blendtable` samt seiner Abschneide-Punkte nachzubilden; das ist eine
eigene, abgegrenzte Aufgabe. Der Prüfstand dafür steht (Raster + Movement,
Frameverlauf über `--save-every`).

</details>

### ✅ Raster-Blends waren vertikal gespiegelt (S74, behoben)

AVS zählt Bildzeilen von **oben**, `gl_FragCoord.y` von **unten**. Bei gerader
Bildhöhe kippt damit die Parität — und die beiden Blend-Modi, die auf einem
Zeilen- oder Pixelraster arbeiten, trafen genau die Gegenpixel:

- **Modus 7** „jede zweite Zeile" (`r_list.cpp:617`, kopiert die oberen
  geraden Zeilen)
- **Modus 8** „jeder zweite Pixel" (`r_list.cpp:628`, zeilenversetztes
  Schachbrett; identisch als `r_stack.cpp` Blend 3)

Aufgefallen im neuen Buffer-Prüfstand: der Kreislauf
Raster → sichern → zweites Bild → zurückholen mit Blend 3 lieferte
**MAE 0,451**.

**Ohne Codeänderung belegt:** dieselbe Probe bei ungerader Höhe liefert 0,002
statt 0,451 (320×241 gegen 320×240). Damit war die Ursache gezeigt, bevor eine
Zeile geändert wurde.

Behoben im gemeinsamen Blend-Shader (`MultiEffectVisualizer.cpp`): neues
Uniform `uResY`, beide Modi rechnen jetzt über die AVS-Zeile
`uResY - 1 - gl_FragCoord.y`.

| | vorher | nachher |
|---|---|---|
| Buffer-Kreislauf Blend 3 | 0,451 | **0,002** |
| Buffer-Kreislauf Blend 5 | — | **0,002** |

**Der ganze Buffer-Pfad ist damit abgenommen: 19/19 OK** — sechs Blend-Modi,
alle acht Buffer, Kreuzprobe (in 0 sichern, aus 1 holen), ungeschriebener
Buffer, Nur-Sichern und der wechselnde Modus. Beide Seiten erzeugen aus den
sechs Modi **je sechs verschiedene Bilder** mit gleicher Gruppierung — der
Prüfstand unterscheidet also wirklich (Regel 6).

> **Eigener Irrtum unterwegs:** Blend 3 heißt in `r_stack.cpp` nicht
> „einstellbar". Ich hatte ihn im Generator so benannt und daraus geschlossen,
> „der Alphawert wirkt nicht" — er wirkt dort gar nicht, weil der Modus ein
> Schachbrett ist. Benennung im Generator korrigiert.

### ✅ Der `enabled`-Schalter im Effekt-Blob wurde ignoriert (S74, behoben)

**Der größte Einzelbefund der Session, und er lag nicht dort, wo er aussah.**

`Water` (Id 20) maß gegen das Kalibrier-Raster MAE 0,332. Der Port wurde Zeile
für Zeile gegen `r_water.cpp` gelegt — Vier-Nachbarn-Summe, `/2`, minus
Vorframe, Klemmung je Kanal, `lastframe` als **Eingang** des Vorframes
(`:455`), Randbehandlung: alles korrekt. Es gab nichts zu fixen.

Der Frame-für-Frame-Lauf hat es entschieden:

| Frame | MAE | unser Mittel | Referenz-Mittel |
|---|---|---|---|
| 1 | 0,100 | 0,198 | **0,0997** |
| 3 | 0,323 | 0,415 | **0,0997** |
| 7 | 0,455 | 0,529 | **0,0997** |
| 12 | 0,331 | 0,383 | **0,0997** |

Der Referenz-Mittelwert steht **konstant** — dort passiert überhaupt nichts.
Im Preset steht `enabled = 0`, und `r_water.cpp` beginnt mit
`if (!enabled) return;`. **Wir führten einen Effekt aus, den das Original
überspringt.**

Ursache im Übersetzer: der `enabled`-Schalter wurde **je Effekt einzeln**
gelesen, und bei `kWater` und `kScatter` fehlte er schlicht. Behoben als
**zentrale Vorgabe** vor dem Effekt-Switch (`AvsChainTranslator.cpp`), damit
die Lücke sich nicht wiederholen kann. Dafür neu:
`EffectNode::hasField()` — `field()` liefert 0 sowohl für „fehlt" als auch für
„ist 0", und bei einem Flag bedeutet das das Gegenteil voneinander. Zweige, die
den Wert als Bitfeld oder Modus lesen (Starfield, Text …), überschreiben die
Vorgabe danach.

**Wirkung:**

| | vorher | nachher |
|---|---|---|
| Water gegen Raster | 0,332 | **0,001** |
| `02_color extasy` | 0,119 (PRUEFEN) | **0,004 (OK)** |
| Schaufenster-Presets | 5/10 OK | **6/10 OK** |

**Regression geprüft:** Kalibrier-Korpus **25/26 OK**. Der eine Ausreißer
(`sonde_div2`, dMaxLuma 0,928 bei MAE 0,003) ist vorbestehend — mit
zurückgenommener Änderung gegengemessen, identische Werte. Tests 593 grün.

> **Merkregel:** Ein Effekt, der bei uns rechnet und im Original abgeschaltet
> ist, sieht in jeder Metrik wie ein Rechenfehler aus. Bevor ein Port
> auseinandergenommen wird, gehört geprüft, ob er auf der anderen Seite
> überhaupt läuft. Der Frame-1-Wert verrät es: weicht schon das erste Bild ab,
> ist es kein Aufschaukeln.

`SuperscopeModule.cpp:586` klemmt `n` auf **4096**, das Original
(`r_sscope.cpp:282`) erst auf `128*1024` = **131072** — Faktor 32 zu niedrig.
**Jedes AVS-Preset mit mehr als 4096 Punkten rendert bei uns abgeschnitten.**

Belegt an einer Dichte-Reihe, Gitter-Raster allein gegen die Referenz:

| Gitter | Punkte | MAE | dMaxLuma |
|---|---|---|---|
| 32 | 1024 | 0,001 | 0,004 |
| 64 | 4096 | 0,001 | 0,004 |
| 72 | 5184 | 0,056 | 0,140 |
| 80 | 6400 | 0,100 | 0,262 |
| 90 | 8100 | 0,144 | 0,352 |
| 96 | 9216 | 0,165 | 0,397 |

Die Kante liegt exakt bei 4096: bis dahin null Abweichung, darüber wächst sie
proportional zum abgeschnittenen Anteil. Im Bild füllt die Referenz die ganze
Fläche, unser Bild bricht bei rund 64 % der Höhe ab (bei 6400 Punkten sind
4096 gezeichnet).

**Fix-Ort benannt, nicht ausgeführt:** die Grenze steht an zwei Stellen
(`:372` beim Setzen, `:586` beim Auswerten von `n`), dazu `p.maxValue = 4096`
als Editor-Obergrenze. Vor dem Anheben zu klären, ob der Vertex-Puffer
mitwächst — 131072 Punkte je Scope ist eine andere Größenordnung.

**Nebenwirkung auf die Messungen:** der Blend-In-Lauf über alle 13
Listen-Modi ist deshalb ungültig. Er meldet durchgehend Abweichungen
(MAE 0,131–0,184), aber sechs der dreizehn Modi zeigen exakt die Zahl des
kaputten Messmittels (0,165/0,251/0,397) — gemessen wurde überwiegend die
Probe, nicht der Blend. Zu wiederholen mit einem Raster unter 4096 Punkten.

**Folge: der Blend-In-Lauf über alle 13 Listen-Modi ist wertlos.** Er meldet
durchgehend Abweichungen (MAE 0,131–0,184), aber sechs der dreizehn Modi
zeigen exakt die Zahl des kaputten Messmittels (0,165/0,251/0,397) — gemessen
wurde überwiegend die Probe, nicht der Blend. Zu wiederholen, sobald ein
dichtes Messmittel abgenommen ist.

> **Regel, die das kostet:** ein Messmittel wird abgenommen, BEVOR damit
> gemessen wird. Beim 32er-Raster habe ich das getan (0,001, leeres
> Differenzbild), beim 96er nicht — und hätte um ein Haar dreizehn
> Blend-Befunde gemeldet, die keine sind.

### ✅ `Water` (Id 20) verschmiert viel zu stark (S74) — überholt

*(Bereinigt S129: kein Fehler des Ports — der Effekt war im Preset mit
`enabled = 0` abgeschaltet, und wir führten ihn trotzdem aus; behoben S74,
Water gegen Raster 0,332 → 0,001, s. „Der `enabled`-Schalter im Effekt-Blob
wurde ignoriert“ oben. Der Text unten ist der Befund vor der Klärung.)*

Gemessen mit dem Kalibrier-Raster (s. §1.0): Raster + **nur** dem
Water-Knoten aus `02_color extasy`, 120 Frames, 320×240.

| | dMean | dMaxLuma | MAE |
|---|---|---|---|
| ohne Raster (schwarze Fläche) | 0,000 | 0,000 | 0,000 → „OK" |
| **mit Raster** | **0,375** | 0,023 | **0,332** |

Im Bild: die Referenz lässt das Raster nahezu unberührt, die schwarzen
Zwischenräume zwischen den Linien bleiben schwarz. Bei uns läuft die ganze
Fläche mit Farbe voll — der Effekt streut weit über die Nachbarschaft hinaus,
die das Original benutzt. Verdacht: Reichweite oder Dämpfung der
Wasser-Nachbarschaft (`r_water.cpp` ist ein 4-Nachbarn-Mittel mit Dämpfung).

**Das ist der größte bisher gemessene Einzelknoten-Befund** und erklärt den
Preset-Wert von `02_color extasy` (MAE 0,119), ohne den bisher ein Knoten
auffällig gewesen wäre.

Daneben, kleiner, aus demselben Lauf: **`Blur` (Id 6)** MAE 0,009 / dMean
0,025 — ebenfalls erst mit Raster sichtbar.

🟠 **Blur gegen das Kalibrier-Raster — nachgemessen S129 (Entscheid Patrik
B1 a): der Befund steht.** `bisect_avs.py --solo --raster` an
`02_color extasy`, Blur = `solo_l04` (Id 6), gegen AvsRef
(`out/blur_s129/`). Messmittel abgenommen: Raster allein dMean 0,001 /
MAE 0,001; Gegenprobe Water (`solo_l03`) 0,001 / 0,001.
**Blur:** 320×240 dMean **0,025** / dMaxLuma 0,001 / MAE **0,009** (PRUEFEN,
dieselben Zahlen wie S74) · 740×460 dMean 0,015 / MAE 0,006 (OK).
**Im Bild** (Montage): Struktur und Farben deckungsgleich; der Unterschied
ist eine gleichmäßige schwache Tönung über die ganze Fläche — ein kleiner
systematischer Versatz, kein Strukturfehler — plus die oberste und die
unterste Bildzeile.
**Eingegrenzt S129 (Ursache offen):** der Knoten steht auf Stärke 1
(`enabled=1`, „light“) und Round mode aus. LumiViz minus Referenz, in
Byte-Stufen (320×240, f120): **Blau +8** auf 98 % der Fläche (sonst +5/+6),
Rot und Grün im Inneren gleich (Mittel +0,02 / −0,13); oberste Zeile Rot +9,
unterste drei Zeilen Grün bis −20. **Ausgeschlossen:** (1) die Rundung in
`anteil()` — mit vorher auf ganze Stufen gerundetem Texturwert identische
Zahlen, zurückgenommen; (2) der MMX-Zweig der Referenz (`AvsRef` baut ohne
`NO_MMX`) — er rechnet dieselbe Formel wie der C-Zweig (`paddb` je Byte,
gleiche Masken und Nachbarn); (3) die Byte-Masken (`MASK_SH*` trennen die
Kanäle sauber). **Gefunden, nicht nachgebildet:** die linke Randspalte rechnet
das Original bei Stärke 1 als `4 × DIV_4` der Nachbarn **ohne** die Mitte
(`r_blur.cpp:631`), bei Stärke 2 die oberste/unterste Zeile mit dem leichten
Kern (`:158`). **Nächster Schritt:** eine Sonde mit flacher Farbe je Kanal
(etwa reines Blau 200) und Blur, Frame 1 gegen Frame 120 — entsteht der
Versatz je Durchlauf oder erst über die Rückkopplung?

### ✅ Skript-Funktionen: AVS 40/40, MilkDrop 32/32 (S74)

Beide Zweige vollständig durchgemessen — jede Funktion einzeln, nicht in
Presets vermutet.

| Zweig | Werkzeug | Umfang | Ergebnis |
|---|---|---|---|
| **AVS** | `make_eelprobe_avs.py` | 40 (33 ns-eel + 7 AVS-eigene) | **40/40** |
| **MilkDrop** | `make_eelprobe_milk.py` | 32 (ns-eel2) | **32/32** |

AVS auf MAE 0,000 bei 24-Bit-Kodierung über 320 Abtaststellen; MilkDrop auf
0,0039 = 1/255, also reine 8-Bit-Rundung.

**Kein einziger Funktionswert weicht ab.** Was der Durchlauf zutage gefördert
hat, waren zwei andere Dinge: die fehlenden Speicher-Funktionen (unten,
behoben) und der Grammatik-Unterschied (weiter unten, offen).

> **Zwei eigene Messfehler unterwegs, beide gefunden und korrigiert:**
> (1) Die AVS-Proben für `exec2`/`exec3`/`loop` benutzten eine Zuweisung im
> Argument — AVS lehnt das ab, die Referenz rechnete nichts, und die Probe maß
> die Grammatik statt der Funktion. Mit legaler Syntax: alle drei OK.
> (2) Der innere MilkDrop-Rahmen taugte nicht als Messmittel (Referenz füllt
> ihn mit konstantem Rot); gemessen wird im Randbereich. Beim ersten Lauf
> meldete das **0 von 31** — wenn alles ausschlägt, ist das Messgerät
> verdächtig, nicht die 31 Funktionen.

### ✅ MilkDrop: `memset` und `memcpy` umgesetzt (S74, behoben)

Voll-Durchlauf über alle 31 Funktionen von ns-eel2 (`make_eelprobe_milk.py`),
je Funktion sechs Abtaststellen, ein Prozess je Preset:
**29 von 31 stimmen** (Abweichung 0,0039 = 1/255, reine 8-Bit-Rundung).

| Funktion | LumiViz | MilkdropRef |
|---|---|---|
| `memset(20, 0.5, 4)` → `megabuf(20)` | **0,000** | 0,498 |
| `memcpy(28, 8, 4)` → `megabuf(28)` | **0,000** | 0,373 |

Bestätigt im Code: `EelTranspilerCodeGen.hpp:465` führt `memset`, `memcpy`
und `freembuf` als **bekannte No-Ops** („ersetzt durch 0.0"), mit Warnung im
Import-Bericht. Der Messwert bestätigt den Stub — neu ist damit nicht die
Tatsache, sondern die **Auswirkung**: ein Preset, das über `memset` ein Feld
vorbelegt und danach daraus liest, bekommt bei uns Nullen.

`freembuf` ist als No-Op dagegen **unschädlich**: die Referenz behält den Wert
nach `freembuf(0)` ebenfalls (beide Seiten 0,375). Nur die beiden
schreibenden Funktionen fehlen wirklich.

**Behoben:** `LuaScriptEngine::lMemset` / `lMemcpy` arbeiten auf demselben
megabuf, den `megabuf()` schon benutzt; `EelTranspilerCodeGen.hpp` bildet die
beiden Namen darauf ab statt auf den No-Op. Übernommen aus `nseel-ram.c`:
das Beschneiden negativer Indizes nach vorn (Länge verkürzen, bei 0 beginnen)
und die Rückgabe des ursprünglichen Ziel-Index. `memcpy` läuft über eine
Zwischenkopie, weil Quelle und Ziel sich überlappen dürfen.

| | vorher | nachher |
|---|---|---|
| `memset(20, 0.5, 4)` → `megabuf(20)` | 0,000 | **0,502** (Referenz 0,498) |
| `memcpy(28, 8, 4)` → `megabuf(28)` | 0,000 | **0,376** (Referenz 0,373) |
| MilkDrop-Funktionen gesamt | 29/31 | **32/32** |

`freembuf` bleibt No-Op — nachweislich zu Recht: die Referenz behält den Wert
nach `freembuf(0)` ebenfalls (beide Seiten 0,375).

Regression: Tests 593 grün, AVS-Schaufenster-Presets unverändert 7/10.

> **Eigener Fehler im ersten Anlauf:** meine `memcpy`-Probe las eine Zelle, die
> ich unmittelbar davor selbst gesetzt hatte — sie meldete OK, ohne `memcpy`
> je anzufassen. Im Generator korrigiert und kommentiert. Dieselbe Klasse wie
> „Prüf-Knoten zeichnet nicht": eine Probe, die ihr eigenes Ergebnis vorwegnimmt.

### ✅ EEL-Grammatik: wir erlauben Zuweisungen, die AVS ablehnt — jetzt gemeldet (S74)

**Der erste nachgewiesene echte Verhaltensunterschied — und ausgerechnet in der
Richtung, dass WIR zu viel können.**

Gefunden im Voll-Durchlauf über alle 40 AVS-EEL-Funktionen (s. u.). Drei
Funktionen wichen ab: `exec2`, `exec3`, `loop`. Ursache ist aber keine dieser
Funktionen, sondern die **Grammatik**:

| Ausdruck | AvsRef | LumiViz |
|---|---|---|
| `q=5;r=exec2(q,q+1)` — Zuweisung davor | ✅ | ✅ |
| `q=0;loop(4,q);r=9` — Schleife ohne Zuweisung | ✅ | ✅ |
| `r=exec2(1,2)` · `r=exec3(1,2,3)` · `assign(q,7)` | ✅ | ✅ |
| `q=0;r=(q=3)+1` — Zuweisung in Klammern | ❌ | ✅ |
| `r=if(1,q=3,0)+1` — Zuweisung als Argument | ❌ | ✅ |
| `r=exec2(q=3,q+1)` | ❌ | ✅ |
| `loop(8,q=q+2)` — Zuweisung im Schleifenrumpf | ❌ | ✅ |

**Regel:** In AVS-EEL ist eine Zuweisung nur als eigenständige Anweisung
erlaubt, nie als Teilausdruck. Das steckt in den Parser-Tabellen
(`nseel-eval.c` / `nseel-lextab.c`) — reine C-Daten, also **Original-Verhalten
und kein Build-Artefakt** wie der `atan`-Fall.

**Folge:** Ein Preset mit solch einer Zuweisung compiliert in AVS gar nicht;
der betroffene Effekt rendert dort **nichts**. Wir übersetzen ihn und
zeichnen. Fehlerhaft ist unser Ergebnis nicht — aber es ist ein anderes.

**Praktische Tragweite: gering.** Ein Preset-Autor sieht in AVS sofort, dass
sein Effekt verschwindet, und schreibt es um. In freier Wildbahn dürfte die
Form deshalb kaum vorkommen. Genau deshalb ist sie aber gefährlich, wenn sie
doch auftaucht: sie fällt niemandem auf.

**Umgesetzt (Vorschlag Patrik):** Der AVS-Übersetzer prüft beim Import jeden
Code-Slot und meldet den Fund im Import-Bericht — mit Knotenpfad, Slot und
Zeile:

> `root/0 (point): Zeile 1: Zuweisung innerhalb eines Ausdrucks (z. B. als
> Funktionsargument oder im Rumpf von loop). AVS laesst das NICHT zu — dort
> scheitert die Uebersetzung des ganzen Skripts, und der Effekt bleibt
> unsichtbar. LumiViz fuehrt ihn aus; das Bild weicht damit vom Original ab.
> Gleichstand erreicht man, indem man die Zuweisung als eigene Anweisung
> davorzieht — oder den Knoten abschaltet, denn genau das tut AVS.`

Die Erkennung sitzt in `AvsChainTranslator::reportEelGrammatik`, nicht im
Transpiler-Aufrufpfad: `ScriptSlotHost` **verwirft** die Transpiler-Warnungen,
und der Skript-Compile läuft ohnehin erst beim ersten Rendern an — der
Import-Bericht entsteht früher. Der Transpiler meldet es zusätzlich
(`findeUnzulaessigeZuweisungen`, nur im AVS-Dialekt), sodass die Regel an
einer Stelle definiert ist.

**Kein Fehlalarm:** Scan über 50 echte Presets (Schaufenster, Kalibrier-Korpus,
Testpresets) → **0 Treffer**. Das deckt sich mit der Erwartung: ein
Preset-Autor sieht in AVS sofort, dass sein Effekt verschwindet.

Fünf Unit-Tests halten die Regel fest, inklusive der Gegenprobe, dass
**MilkDrop keine Warnung bekommt** — ns-eel2 erlaubt die Form, gegen
MilkdropRef gemessen.

> Das ist der Fall, für den ein Dialog gedacht war. Beim `atan`/`log`-Befund
> wäre er falsch gewesen (dort war unser Messgerät kaputt) — hier ist er
> richtig, weil der Unterschied gemessen und reproduzierbar ist.

### ✅ AVS-EEL: alle 40 Funktionen einzeln gegen die Referenz (S74)

`make_eelprobe_avs.py` erzeugt je Funktion ein Preset, das sie über 320
Abtaststellen ihres Definitionsbereichs durchläuft; der Wert wird über drei
Kanäle kodiert (`red = v`, `green = Rest(v·256)`, `blue = Rest(v·65536)`) und
erreicht damit rund 24 Bit statt der 8 eines Einzelkanals.

**Ergebnis: 37 von 40 übereinstimmend** (MAE 0,000). Die drei Abweichler sind
`exec2`, `exec3`, `loop` — und zwar aus dem Grammatik-Grund darüber, nicht
wegen der Funktionen selbst.

Mitgeprüft und in Ordnung: `getosc`, `getspec` (hängen am synthetischen Audio),
`gettime` (mit `--tick-hz 60`), `megabuf`, `gmegabuf`, `getkbmouse`.
`rand` ist nicht wertvergleichbar (auf 0 gezwungen, geprüft wird nur der Lauf);
`invsqrt` weicht **absichtlich** ab (schnelle Näherung, dMaxLuma 0,081).

### ✅ `AvsRef`: JIT-Fragmente von `atan` und `log` waren überlang (S74, behoben)

**Ein Fehler im Referenz-Werkzeug, nicht in LumiViz** — er hat jeden Vergleich
von Presets entwertet, die eine der beiden Funktionen verwenden, und dabei wie
ein massiver LumiViz-Befund ausgesehen.

**Ursache, an den Adressen belegt.** `AVSREF_EELTEST=1` gibt die
Fragmentgrößen der JIT-Tabelle aus:

| | Start | Ende | Größe |
|---|---|---|---|
| `atan` | 0059B160 | 0059B1E0 | **128** |
| `atan2` | 0059B190 | 0059B1D0 | 64 — **liegt mitten in `atan`** |
| `log` | 0059B4B0 | 0059B500 | **80** |
| `log10` | 0059B4D0 | 0059B4F0 | 32 — **liegt mitten in `log`** |

Der Linker hat die leeren `_end`-Marken hinter die jeweils *folgende* Funktion
sortiert. `atan`s Fragment umfasste damit den kompletten `atan2`-Körper; nach
dem eigenen Code lief die Ausführung in Fremdcode und `0xCC`-Füllbytes
(`int 3`). Im Preset fängt die SEH-Absicherung von AVS das ab — der Effekt
rendert schwarz, ohne jede Meldung.

**Behoben** mit `/Gy-` für `patched/nseel-cfunc.c`
(`tools/AvsRef/CMakeLists.txt`): ohne Funktions-COMDATs liegen die Funktionen
in **Quellreihenfolge**, und genau das setzt der JIT-Vertrag voraus. Danach:
`atan` 48 Bytes, `log` 32, keine Überlappungen.

| | vorher | nachher |
|---|---|---|
| Funktions-Selbsttest | Absturz bei `atan` (0x80000003) | **0 von 19 fehlerhaft** |
| `fn_atan` / `fn_log` | Referenz schwarz | **MAE 0,000** |
| **`10_the ring`** | **0,113 (PRUEFEN)** | **0,001 (OK)** |
| Schaufenster-Presets | 6/10 OK | **7/10 OK** |

Kalibrier-Korpus unverändert 25/26.

> **`invsqrt` ist absichtlich ungenau** — schneller Bit-Trick plus eine
> Newton-Iteration, liefert 0,49915 statt 0,5. Im Selbsttest mit eigener
> Toleranz geführt; wer dort scharf stellt, jagt eine Absicht.

#### Was das für die Presets heißt

**Nichts — sie waren nie kaputt, und LumiViz war nie falsch.** Nach dem Fix
stimmen unsere `atan`- und `log`-Ergebnisse mit der Referenz überein
(MAE 0,000). Ein Dialog „hier verhält sich LumiViz anders als AVS" wäre also
sachlich falsch gewesen; die Warnung, die ich zwischenzeitlich ins
Kalibrier-Handbuch geschrieben hatte, ist mit dem Fix gegenstandslos und
wieder entfernt.

> **Merkregel:** Ein Referenz-Werkzeug, das jemand neu baut, ist selbst ein
> Messgerät und gehört abgenommen. Der JIT-Vertrag von ns-eel („Fragmentlänge
> = Adressdifferenz zweier Funktionen") setzt Quellreihenfolge im Speicher
> voraus — eine Annahme, die moderne Compiler-Vorgaben stillschweigend
> brechen. `AVSREF_EELTEST=1` prüft das jetzt in einem Lauf.

<details>
<summary>Der Weg dorthin (S74) — vier Messungen, die je eine Erklärung ausschlossen</summary>

Gefunden beim Aufklären von `10_the ring` (s. u.). Fünfzehn EEL-Funktionen
einzeln als SuperScope-Farbe geprüft, 60 Frames, sonst identischer Aufbau:

| Ergebnis | Funktionen |
|---|---|
| ✅ übereinstimmend | `sin` `cos` `tan` `asin` `acos` **`atan2`** `sqrt` `pow` **`log10`** `exp` `abs` `sigmoid` `invsqrt` |
| ❌ Referenz zeichnet **gar nichts** | **`atan`** · **`log`** |

#### Der Mechanismus ist belegt, nicht vermutet

Vier Messungen, jede schließt eine Erklärung aus:

1. **Kein Wertunterschied.** Die Referenz liefert `max = 0,000` über das ganze
   Bild — kein falsches Bild, sondern gar keins. Es gibt keinen Wert zu
   vergleichen.
2. **Keine Frage der Stelligkeit oder Bedeutung.** `atan` steht in
   `nseel-compiler.c:140` mit **1** Parameter, `atan2` mit 2; `log` ist der
   natürliche, `log10` der dekadische Logarithmus. Verschiedene Funktionen,
   korrekt aufgerufen — eine falsche Stelligkeit hätte AvsRef als
   Compile-Fehler gemeldet, und es kam keiner.
3. **Nicht der Aufrufort, nicht die Verwendung.** Die **bloße Anwesenheit**
   von `atan` in irgendeinem Slot legt den Knoten lahm — auch im `init`-Slot,
   auch wenn das Ergebnis nirgends benutzt wird und der Punkt-Code feste
   Farben setzt.
4. **Nicht das Preset, nicht der Renderer.** Ein Funktions-Selbsttest in
   AvsRef (`AVSREF_EELTEST=1`) compiliert und JIT-führt jede Tabellenfunktion
   einzeln aus, ohne Preset und ohne Renderer. `sin` `cos` `tan` `asin` `acos`
   laufen — **`atan` beendet den Prozess mit `STATUS_BREAKPOINT` (0x80000003)**.

`0xCC` ist der Füllbyte-Wert, mit dem MSVC den Raum zwischen Funktionen
auffüllt, und `0xCC` ist zugleich `int 3`. ns-eel kopiert Maschinencode-
Fragmente zwischen `nseel_asm_<fn>` und `nseel_asm_<fn>_end` und bestimmt
deren Größe als **Adressdifferenz zweier nackter Funktionen**
(`nseel-cfunc.c:109-118`). Stimmt die Größe nicht, wird Füllmaterial
mitkopiert und ausgeführt — genau das passiert hier. Im Preset fängt die
SEH-Absicherung von AVS den Fehler ab, der Effekt wird übersprungen, das Bild
bleibt schwarz; der Selbsttest hat keine solche Absicherung und stirbt sofort.

**Damit ist es ein Fehler DIESES Builds, nicht von AVS.** Das Original wurde
mit VC6 gebaut; `tools/AvsRef/CMakeLists.txt` arbeitet gegen dieselbe
Fragilität schon mit `/OPT:NOICF /INCREMENTAL:NO`, für diese beiden Fragmente
reicht es nicht.

#### Was das für die Presets heißt

**Nichts.** Weder ist LumiViz hier falsch, noch sind die Presets kaputt:

- `atan` ist eine dokumentierte, verbreitete AVS-Funktion, und Preset-Autoren
  schreiben, was sie wirken sehen. Wäre sie in Winamp reihenweise abgestürzt,
  stünde sie in keinem Preset — sie steht aber in `10_the ring`.
- Ein Dialog, der dem Anwender „hier verhält sich LumiViz anders als AVS"
  meldet, wäre nach dieser Messlage **falsch**: Ein Verhaltensunterschied ist
  gerade nicht nachgewiesen, sondern ausgeschlossen worden. Die Warnung gehört
  ins Kalibrier-Werkzeug, nicht in die App.

</details>

**Folge für die Kalibrierung, sofort wirksam:** ein Preset, dessen EEL `atan`
oder `log` enthält, ist gegen `AvsRef` **nicht messbar**. Vor jedem Befund
prüfen:

```bash
grep -c "atan\|log" <preset-quelle>
```

`atan2` und `log10` sind ausdrücklich in Ordnung — nur die beiden kurzen Namen.

Zu tun: `nseel-cfunc.c` in `tools/AvsRef/patched/` kopieren und die beiden
Funktionszeiger gegen eigene, nicht-intrinsische Wrapper tauschen
(`#pragma function(atan, log)` oder eine eigene `double lumi_atan(double)`).
Danach die 15er-Probe wiederholen — sie liegt als Prüfstand vor.

### 🟠 `10_the ring`: der SuperScope-Befund war der `atan`-Fehler (S74, entwertet)

> **Aufgelöst (S74):** der Knoten enthält `blue=atan(i+y)`, und `AvsRef`
> rendert bei `atan` gar nichts (s. Befund darüber). Der Sichtbefund „wir
> zeichnen eine Linie, die Referenz nicht" war damit **kein LumiViz-Fehler,
> sondern die Blindstelle des Referenz-Werkzeugs.** Unsere Ausgabe ist
> formeltreu — nachgerechnet: `t` bleibt 0, damit `red = 0`,
> `blue = atan(3i-1)` läuft 0 → 1, `green = cos(2i-1)·sin(i)` bleibt unter
> 0,45; das ergibt die blaue Linie, unten am hellsten. **Bewiesen ist das
> nicht** — solange die Referenz nichts zeichnet, gibt es nichts zu
> vergleichen. Das Preset ist erst nach dem `atan`-Fix messbar.
>
> Die drei ursprünglichen Kandidaten sind dabei alle ausgeschlossen worden,
> jeder mit einer eigenen Variante des Knotens: **entartete Geometrie** (alle
> Punkte auf x=0) — `x` spreizen ändert nichts · **EEL-Semantik bei
> undefinierten Variablen** (`ti`) — `t=0` erzwingen ändert nichts ·
> **Farb-Clipping** — mit festen Farben zeichnen beide Seiten identisch
> (Mittel 0,0031 auf beiden). Der letzte Test war der Treffer: nicht die
> Farben *werte* waren das Problem, sondern die Funktion in ihrer Berechnung.

Bisektion mit `bisect_avs.py`, Aufbau des Presets:
Top-Level `[36 SuperScope, 15 Movement, -2 Liste, 43 DynamicMovement]`.

| Stufe | Inhalt | MAE klassisch | MAE musik |
|---|---|---|---|
| t01 | nur SuperScope | 0,001 | 0,001 |
| t02 | + Movement | **0,188** | **0,363** |
| t03 | + Liste | 0,156 | 0,302 |
| t04 | vollständig | 0,113 | 0,264 |

**Die Zahl bei t01 lügt.** Der Sprung sieht nach „Movement ist der Täter" aus,
aber die Montage zeigt: schon bei t01 ist **AvsRef vollständig schwarz**
(`Luma max = 0,000`), während LumiViz eine blaue Vertikale in der Bildmitte
zeichnet. Eine 1-Pixel-Linie bewegt den Mittelwert kaum — der MAE von 0,001
hat den Unterschied verdeckt. Movement (`x=0`, Rechteck-Koordinaten) schmiert
diese Linie danach über das ganze Bild, und erst dort wird die Abweichung
messbar. **Der Täter ist der SuperScope, nicht das Movement.**

Der Knoten:

```
init:  n=800
frame: t=t-ti*0.5
point: x=0; y=i*2-1; green=cos(y)*sin(i+t); blue=atan(i+y); red=sin(t*0.2)
```

Unsere Ausgabe ist formeltreu: `ti` ist undefiniert (= 0), also bleibt `t = 0`;
damit `red = 0`, `blue = atan(3i-1)` läuft 0 → 1, `green = cos(2i-1)·sin(i)`
bleibt unter 0,45 — eine blaue Linie, unten am hellsten. Genau das zeigt das
Bild. **Warum die Referenz gar nichts zeichnet, ist offen.**

Ausgeschlossen ist bereits:

- **Kein AvsRef-Defekt im SuperScope-Pfad allgemein**: `09_rotating_things`
  enthält **15** SuperScopes und stimmt bei MAE 0,010 überein.
- **Kein Seitenverhältnis-Problem**: der MAE des ganzen Presets bleibt über
  240×240 / 320×240 / 480×240 flach (0,108 / 0,113 / 0,117). Die Ring-Form
  ändert sich mit dem Format auf beiden Seiten gleichermaßen.

Zu klären als Nächstes: warum die Referenz diesen einen Knoten verwirft —
degenerierte Geometrie (alle 800 Punkte auf derselben x-Koordinate),
Farb-Clipping oder eine EEL-Semantik bei undefinierten Variablen. Die Antwort
entscheidet, ob wir zu viel zeichnen oder die Referenz zu wenig.

**Zweiter, unabhängiger Befund im selben Preset:** der äußere Ring ist bei uns
deutlich dunkler (Referenz hell gelb/weiß, wir dunkel orange). Getrennt
verfolgen — er hängt an der Liste, nicht am SuperScope.

---

### §1.0 Methode: erst einzeln, dann Kombinationen — und Transforms brauchen ein Raster

Zwei Vorgaben Patriks aus S74, beide sofort durch Messwerte bestätigt:

**„Prüfe alle separat, bevor du Kombinationen testest."** Die kumulative
Bisektion (`stage_t<k>`) beantwortet „ab wo läuft es auseinander" — und
beantwortet es falsch, sobald ein früher Knoten für sich schon abweicht und
die Metrik das verschluckt. Bei `10_the ring` meldete Stufe t01 MAE 0,001,
obwohl die Referenz dort schwarz ist und wir eine Linie zeichnen; der Sprung
erschien erst bei t02 und zeigte auf das falsche Modul. Neu:
`bisect_avs.py … --solo` erzeugt je Knoten ein Preset mit genau diesem Knoten.

**„Für Transform-Knoten würde ein vordefiniertes Raster helfen."** Transforms
(Movement, Water, Blur, Fadeout, …) sind einzeln nicht messbar: auf der
schwarzen Startfläche liefern beide Renderer schwarz und der Vergleich meldet
0,000 — was nur „nichts zu sehen" heißt, nicht „geprüft". Neu:
`make_raster_avs.py` erzeugt ein Punktgitter als **echten SuperScope im
Preset** (kein Host-Overlay — nur so zeichnet die Referenz dasselbe Raster).
Die Farbe kodiert die Herkunft (rot = Spalte, grün = Zeile), Verschiebung und
Verzerrung sind damit direkt ablesbar.
`bisect_avs.py … --solo --raster raster.avs` setzt es als Quelle davor.

**Abnahme des Messmittels:** das Raster allein liefert gegen `AvsRef` MAE
0,001 bei leerem Differenzbild — es ist neutral und als Grundlage brauchbar.

> **Das Raster gehört IN die Liste, nicht davor.** Am laufenden Paar
> nachgemessen: vor die Liste gesetzt ist es im Ergebnis nicht mehr zu sehen,
> beide Renderer liefern schwarz — die Liste räumt die Fläche unter sich weg.
> Erst als erstes KIND derselben Liste steht die Quelle im selben
> Blend-Kontext wie der Knoten, der gemessen werden soll. Der erste Anlauf hat
> genau daran nichts gefunden.

**Folge:** alle Einzelknoten-Urteile „0,000 = OK" aus einem Lauf **ohne**
Raster sind wertlos und müssen mit Raster wiederholt werden. Der Water-Befund
oben ist genau so entstanden — von „0,000 OK" auf MAE 0,332.

#### Listen-Prüfstand: Blend In/Out, Buffer, Reihenfolge

`make_listenprobe_avs.py` (neu, S74) baut Proben mit dem Raster als Untergrund
und einer Liste darüber, deren Kopf variiert wird: alle 13 Blend-In-Modi, die
Adjustable-Skala (0/64/128/192/255), Buffer 0–2 mit und ohne Invertierung,
sowie beide Reihenfolgen im selben Listen-Kontext.

Zwei Format-Fallen haben dabei je einen kompletten Lauf entwertet, beide
liefern **falsch-negative** Ergebnisse — die Probe misst brav über alle Modi
denselben Wert und sieht aus wie ein bestandener Test:

- **Die Größe der erweiterten Daten steht im oberen Byte von `mode`**
  (`set_extended_datasize(36)`). Fehlt sie, ist `ext = 5`, `load_config` liest
  **keinen** der acht Kopf-Werte und läuft mitten in die Kinder. Symptom:
  `abgeschnittener Config-Blob (deklariert 128, vorhanden 52)` — beide
  Renderer verwerfen die Liste stillschweigend.
- **`blendout` liegt invertiert im Feld** (`((mode>>16)&31)^1`), und 0 heißt
  nicht „Vorgabe", sondern „Ergebnis nicht zurückschreiben". Die Liste rendert
  dann intern und wirft das Ergebnis weg; sichtbar bleibt nur der Untergrund.
  Richtig ist 1 = Replace.

---

Metrik ist der Abstand zur Referenz (`AvsRef`), 0 = gleich. Methodik:
[AVS_Kalibrier_Methodik.md](visuals/AVS_Kalibrier_Methodik.md) — **Urteil über
gezeichnete Pixelmenge + Schwerpunkt**, nicht über dMean allein (die Metrik lügt bei
dünnen Inhalten).

| Preset / Sonde | Wert | Diagnosestand | |
|---|---|---|---|
| ~~**07 Milky Way Xtreme**~~ | **0,346 → 0,033** | ✅ derselbe Texer-Farbfix (S52), Vorstand gemessen | ✅ |
| ~~**19 High Voltage**~~ | **0,126 → 0,040** | ✅ derselbe Texer-Farbfix (S52), Vorstand gemessen | ✅ |
| ~~**15 Alien Alloy**~~ | **0,647 → 0,008** | ✅ **gelöst (S52):** Texer II setzte die Sprite-Farbe **je Punkt** auf Weiß zurück — also *nach* dem Frame-Slot, in dem dieses Preset sie berechnet (`red=sin(ct+2.07)*0.5+0.5` …). Wir zeichneten durchgehend weiße Sprites; da sie die einzige Energiequelle des Wirbels sind, lief das Bild über die Frames nach Schwarz. Gegenstück zum `sizex`/`sizey`-Befund aus S51, dieselbe Ursache an anderer Variable. Rest-MAE 0,298 = Phasenversatz, gehört zum rand-Faden | ✅ |
| ~~**01 Picture II**~~ | 0,516 → **0,008 / MAE 0,029** (320) · **0,008 / 0,016** (640) | ✅ **grün (S58), vier Befunde.** (1) **Parser:** der Dateiname von „Picture II" steht in einem FESTEN 260-Byte-Feld (MAX_PATH), nicht als NUL-terminierte Zeichenkette — der Blob misst 284 = 260 + 6×4. Wir lasen variabel, blieben nach dem ersten NUL stehen und nahmen den Rest des Puffers als Zahlen; hier stand noch `.bmp` von einer früheren Eingabe, also war `blendMode` 1 886 350 382 statt 0 und alle sechs Felder Müll. Das Bild kam nie an. (2) **Randspalte:** der SuperScope zeichnet `x=1-2/w`, was in double exakt Spalte w−1 ergibt und über float gerundet w−2; ein Movement (`dg=.9015*sw/320` — die Falte liegt bei 640 ausserhalb des Bildes, bei 320 mitten drin) sammelt bei 640×480 ALLE Ausgabepixel auf genau dieser Spalte ein und zieht sie über den Schirm. Daher die Größenabhängigkeit. (3) **Dynamic Shift** am Rand — s. eigene Zeile. (4) **Die Picture-II-APE hat SECHS Betriebsarten**, wir kannten drei: der zweite Aufruf im Preset mischt mit **Maximum**, bei uns wurde daraus ein Mittelwert und das Bild lief hell aus. Damit grün in beiden Größen | ✅ |
| ~~**Custom BPM (id 33)**~~ | Zeile 9 → **0** | ✅ **gelöst (S52):** vier Abweichungen von `r_bpm.cpp` — (1) **Off-by-one**: das Original lässt jeden `skipVal+1`-ten Beat durch, wir jeden `skipVal`-ten · (2) die drei Betriebsarten sind dort **exklusiv** (jeder Zweig kehrt sofort zurück), bei uns liefen sie hintereinander und konnten sich kombinieren · (3) `skipfirst` wurde geparst, aber nie übernommen · (4) `skipval=0` heißt „jeden Beat", unsere Untergrenze 1 machte „jeden zweiten" daraus. Sonde `bpm_zaehler_skip3` jetzt MAE 0,000 | ✅ |
| ~~**Inhaler**~~ | **0,345 → 0,168** | ✅ Custom-BPM-Fix. Bisektion (S52): der Scope allein war referenzgleich (MAE 0,003), es hing am Filter davor. Der rand-**Strom** ist als Ursache ausgeschlossen (vier Sonden, MAE 0,000) | ✅ |
| ~~**Reflectosphere**~~ | **0,174 → 0,085** | ✅ derselbe Fix | ✅ |
| ~~**The Lion King**~~ | **0,021 → 0,013 (grün)** | ✅ derselbe Fix | ✅ |
| ~~**Deep Red Sea**~~ | **0,943 → 0,011** | ✅ **gelöst (S52)** — zwei Fehler: (a) der **Adjustable-Blend** war vertauscht (`v` gewichtet in `r_defs.h:250-257` den *Framebuffer*, wir gaben es der neuen Farbe) · (b) `runClear` reichte die **eigene** Clear-Aufzählung roh in `applyLineBlend`, das die BLEND_LINE-Tabelle erwartet — aus „50/50 gegen Schwarz" wurde „MAX gegen Schwarz", also ein **No-op**, das Bild klang nie ab. Zieht mit: Wtf I'm Lost 0,094 → 0,003 · High Voltage 0,114 → 0,005 | ✅ |
| ~~**Alternate Reality**~~ | 0,622 → **0,025 / MAE 0,029** (320) · 0,867 → **0,015 / 0,017** (640) | ✅ **abgenommen (S58, Entscheid Patrik): der Rest gilt als Rauschen.** Zuerst war die REFERENZ nicht reproduzierbar (s. Zeile „AvsRef: APE-DLL"), danach zwei App-Befunde: Channel Shift zog einen Zufallszug zuviel (Startwert statt Zug im ersten Frame), und die Convolution-APE rechnet ganz anders als angenommen (eigene Zeile). Damit sind die Bisektionsstufen 1–8 grün (0,000–0,006) und das gesättigte Pink ist weg — die Referenz bleibt weiß, wir jetzt auch. **Rest (MAE 0,339):** die Rückkopplung **Buffer Save Slot 0 → Dynamic Movement mit `buffern=1`**. Frame 1 ist exakt, ab Frame 2 laufen beide Seiten um 1,3 % auseinander, gesättigt bei 3 % ab Frame ~20; die zweite Convolution verstärkt das auf 24 %, weil ihr `absolute` jedes negative Zwischenergebnis auf 255 kippt. **Die Ursache war der BEAT-BEREICH:** in `r_list.cpp:747-751` wirken `SET_BEAT`/`CLR_BEAT` eines Kindes nur auf die lokale `isBeat`-Variable der Liste, also auf deren nachfolgende Kinder — der Elternteil sieht davon nichts. Wir stellten `m_frameBeat` global um; das Custom BPM in der Unterliste filterte damit jeden vierten Beat für die GANZE Kette. Die beiden Dynamic Movements dahinter zogen ihre acht `rand(4)` je Beat-Code deshalb viermal seltener als in der Referenz, und der geteilte Zufallsstrom lief auseinander. Mit dem Listen-Bereich: MAE 0,339 → **0,029**. Zuvor **ausgeschlossen** (gemessen, nicht vermutet): das Rand-Modell aus S57 (Abweichung gleichmäßig verteilt, Randanteil 6 % = Flächenanteil; mit abgeschaltetem Discard bleibt es bei 0,231), die Dynamic Movement selbst (verbatim auf Referenzbild, 2×2-Schachbrett und Punktgitter: 0,000–0,031), ihr Gitter- und `BLEND_ADJ`-Rechenwerk (seit S49 ganzzahlig), die 50/50-Mischung, die verworfene Liste. **Rest:** bei 320×240 liegt dMean mit 0,025 knapp über der Schwelle 0,02 (MAE 0,029 hält sie); bei 640×480 ist die Zeile grün. Kein Handlungsbedarf — wäre allenfalls noch ein kleines Anpassen. Ursprünglicher Befund: 🔴 **S52: wird durch den korrekten Clear SCHLECHTER** (Vorstand dreimal gemessen). Das Preset klart 50/50 gegen ein helles Orange; solange unser Clear ein No-op war, lag der Hintergrund zufällig näher an der Referenz. Jetzt sättigt er pink, die Referenz bleibt **weiß** — bei identischer Labyrinth-Struktur. Der kaputte Clear hat hier einen **zweiten** Fehler verdeckt; Augenmerk auf die Listen-Blends um den Clear | 🔴 |
| ~~**greatwho2006 15/16**~~ | Symmetrie **0,042 → 0,0000** | ✅ **gelöst (S52), Befund Patrik „sollten sauber spiegeln":** Mirror wertete alle vier Richtungen in EINEM Shader-Durchgang aus der unveränderten Textur aus, die Regeln überschrieben sich. Das Original läuft vier Schleifen nacheinander (`r_mirror.cpp` 167/188/210/230), jede sieht das Ergebnis der vorigen — daher ergeben zwei aktive Achsen dort ein symmetrisches Bild. Jetzt ein Durchgang je Richtung in Referenz-Reihenfolge | ✅ |
| ~~**30 Bright Light District**~~ | 0,252 → **0,017 / MAE 0,021** (320) · 0,206 → **0,020 / 0,018** (640) | ✅ **grün in beiden Größen (S58).** Gefallen sind hier der Dynamic-Shift-Umbau (dMean 0,270 → 0,026) und die `skip`-Semantik der Scopes (zwei der drei Flügel fehlten ganz). Der letzte Schritt (0,070 → 0,017 bei 320) kam von der **Pixelmitte der Scope-Linien**: die gesamte Restabweichung entsprang EINEM Movement an Position 3, dessen Skript mit `y=if(below(d,.5775),0,y)` die ganze Innenscheibe aus EINER Quellzeile holt und mit `x=sin(ly*80)*4/sw` über vier Spalten schwenkt — eine 240-fache Vergrösserung derselben Bildspalte, unter der jeder Rasterversatz sichtbar wird. Ausgeschlossen wurden davor: die Zufallszahl im Skript (drei feste `p` ändern nichts), jede einzelne Skriptzeile für sich (sechs Teilsonden grün) und die Zeilenabbildung (Subpixel-Kennlinie über 20 Stufen deckungsgleich) | ✅ |
| ~~**P3_HpR20 Rotor**~~ | 0,461 → **0,018** (320) · **0,006** (640) | ✅ **grün (S58)** — ohne eigene Arbeit gefallen, die Befunde dieser Session haben gereicht. Montage: dasselbe Flammenblatt an derselben Stelle, der 4×-Diff ist Kantenrauschen. Der Rest stand seit S48 | ✅ |
| ~~**Tie Tunnel DM**~~ | 0,074/0,098 → **0,010/0,009** (320) · **0,002/0,010** (640) | ✅ **GELÖST (S61) — der GESPEICHERTE STARTWINKEL der Dot Plane.** Die Bisektion lief in vier Schritten: (1) Top-Level-Stufen — die Bänder-Liste allein maß 0,000, +Dynamic Movement 0,073; aber das „0,000" war schwarz-gegen-schwarz (die Liste rendert NUR in Buffer-Slot 0, die DM liest ihn per `buffern=1`). (2) Blit-Sonde auf Slot 0 (DM `nomove=1`): der Buffer selbst divergiert schleichend ab ~Frame 12 — Täter in der Rückkopplungsschleife Water→DotPlane→Scopes→BlitterFB→ChannelShift→Mosaic→BufferSave. (3) Auslass-Sonden: ohne Dot Plane fällt die Divergenz von 0,044 auf 0,017, alle anderen Glieder ändern nichts (Water dämpft nur als Verstärker). (4) Dot Plane isoliert in Preset-Konfiguration: MAE 0,006, Montage zeigt die Ebene VERDREHT. Ursache: `r_dotpln::load_config` liest ein 8. Feld **`r = rr/32`** — die im Preset gespeicherte laufende Rotation (hier 1435/32 = **44,84°**). Unser Import warf das Feld weg (AvsParser dekodierte `r_raw` längst, der Translator ignorierte es), die Ebene stand dauerhaft 44,84° verdreht; die fast-schwarzen Band-Farben (Scopes 0x000001!) machten aus dem verdrehten Farbmaterial über die Verstärkerschleife die falsche **Farb-Phase der Tunnelbänder** — Geometrie ref-gleich, weil die DM nur die Quell-ZEILEN anders traf. Fix: `startRotation` in DotPlane- UND DotFountainParams (r_dotfnt speichert genauso), STARTWERT-Bauart wie `interfRotationSeed`. Verifikation: Dot-Plane-Sonde 0,000 über ALLE 120 Frames · Tie Tunnel grün in beiden Größen · Schwester-Preset Tie Tunnel SSC 0,001 · Matrix 01/19 weiter pixelgenau · Zwillinge 67/67 (2 erklärte Refreezes) · Tests 485. Historie: 🟠 **S60 nachgemessen: deutlich besser, und der WÜRFEL SITZT** — die Montage zeigt ihn deckungsgleich (die S59-Linien-Umbauten haben die Versatz-These erledigt; `dt→dt_p` ist als D2-Regel implementiert und die Skripte weisen `dt` vor jedem Lesen zu — die Spur ist tot). **Rest: die FARB-PHASE der Tunnelbänder** (ref magenta-lastig, wir blau-lastig in denselben Geometrien) — braucht eine eigene Bisektions-Session (Band-Quelle isolieren, Farbzyklus-Phase messen). Historie S57: 🟠 **S57 angearbeitet, nicht gelöst.** Die Montage zeigt: **Struktur stimmt** (Tunnel, Streifen, Würfel), aber der **Würfel steht versetzt** — im Diff erscheint er doppelt — und die Tunnelflächen sind leicht verschoben. Die sieben SuperScopes (Cockpit, Crossbar, vier Wings, Stars) teilen denselben Frame-Code `t=t+0.02` und berechnen daraus dieselbe Kamerabahn; ein Versatz von EINEM Frame würde genau so aussehen. **Das ist ausgeschlossen:** mit 119 bzw. 121 Frames steigt der Abstand auf 0,307 bzw. 0,402, mit 120 bleibt er bei 0,148 — die Zeitbasis läuft synchron. Nächste Verdächtige: die perspektivische Projektion der Scopes (Grösse statt Position) und der `dt → dt_p`-Umbau des Imports (sieben Hinweise in diesem Preset). Der Wert hat sich gegenüber der S49-Notiz leicht verbessert (0,154 → 0,148), vermutlich durch den Water-Umbau — das Preset nutzt Water | 🟠 |
| **Blitter-Zoom-in: Trail weicher als Referenz** | Beispiele 0,062–0,163 | 🟠 **NEU (S61, Beispiel-Sweep):** drei der 90 .avs-Beispiele fallen auf — `blitterFeedback - Inward Pull` (0,163/MAE 0,143), `Beat-Sprung` (0,062) und `dynamicMovement - Molecule Grid` (0,060). Die Montagen zeigen ÜBERALL dieselbe Struktur und Farbe, unser Bild ist über die Rückkopplung nur **weicher** (Ringe verschmieren, die Referenz bleibt scharf) — der Zoom-IN-Pfad (subpixel/BLEND4) verliert je Durchgang etwas mehr Hochfrequenz, die Trail-Szene stapelt das über 120 Frames. Kein Strukturfehler; die Matrix-Zeile 04 (Grundkonfig) blieb grün, sichtbar wird es erst bei DAUER-Zoom-in. Kandidat: Bilinear-Gewichte/Rundung des blitter_normal-Pfads gegen r_blit BLEND4 vermessen (Bauart wie der S60-Zoom-out-Befund). Metrik-Bremse: fixen oder §9 mit Abnahme | 🟠 |
| ~~**Sonde `convolution_kante`**~~ | 4547/3981 → **4547/4540 px** | ✅ **gelöst (S57).** Die APE berechnet die **letzte Zeile und die letzte Spalte nicht** — ein Off-by-one ihrer Schleife. Der Zielpuffer behält dort seinen alten Inhalt, und weil er im Wechsel wiederverwendet wird, läuft der Wert über die Frames auf: in der Referenz stand dort **255** (gesättigt nach mehrfacher 8×-Verstärkung), bei uns jedes Mal frisch **128** = 8 × 16. Der Weg dorthin: die 560 Pixel im Bild verorten (Zeile 239 + Spalte 319, je vollständig), dann mit der **Identitäts-Sonde** belegen, dass das Eingangsbild identisch ist, und über den Kern (`[0]*24 + [8] + [0]*24` — nur die Mitte) die Randbehandlung ausschliessen. Modul-Sonden **78/80 → 79/80**. Nebenwirkung: `convolution.edgeMode` fällt von 0,0014 auf 0,0007 (SCHWACH), weil der Rand-Unterschied jetzt nur noch links und oben entstehen kann — erklärbar, kein Befund | ✅ |
| ~~**Sonde `6_alloy/paar_original`**~~ | 39546/37671 → **grün** | ✅ **gelöst (S59) — durch den bit-exakten Texer-II-Resampler**, nicht durch die S57-These (rand-Startphase): die Sprite-Rasterung selbst wich ab (Subpixel-Phase, Malbreite). Ursprünglich: 🟠 **S57: Ursache bewiesen — die ZUFALLS-STARTPHASE.** Die Kette ist Dynamic Movement + Texer II, und ihr Init lautet `t=rand(100)/50`. Zwei neue Sonden mit **festem** `t` (`paar_t_fest_082`, `paar_t_fest_2`) sind **grün** (Menge 0,02 · Lage 0,5 bzw. 1,5) — bei gleichem Startwert rechnen beide Seiten also gleich, und Physik wie Darstellung scheiden aus. Unser `rand()` ist der exakte MSVC-LCG mit Seed 1 (erster Zug 41 → `t = 0,82`); es steht beim Init dieses Effekts nur an einer anderen Stelle des Stroms als bei AvsRef. **Nächster Schritt:** bestimmen, wie viele Züge die Referenz vor dem ersten Effekt-Init macht (Konstruktoren ziehen — Grain zieht 491+1, S49) und unseren Strom entsprechend ausrichten | 🟠 |
| ~~**Modul-Matrix-Reste**~~ | **40/43** (S60) → **43/43** | ✅ **erledigt (S60; bereinigt S129):** `20_water` und `24_grain` sind von Patrik als bewusste Grenze abgenommen (§0 Haken 2, §9) — die beiden „Offen (2)“ unten sind damit geschlossen. **S60: vier der fünf Flächen-Befunde PIXELGENAU gelöst** (je Menge 0,00/Deckung 1,00 in beiden Größen): **`13_rotating_stars`** — exakter r_rotstar-Port: Sterngröße aus dem größten LOKALEN Peak der rohen visdata-Bytes (Bedingung: beide Nachbarn um >4 unterboten, in SIGNED char — Bytes >127 zählen nie), Größe `(s+9)/352` je Achse, `rotSpeed` 0,1, und die Cast-Eigenheit: der ERSTE Eckpunkt entsteht mit getrennten `(int)`-Casts, die Schleifen-Ecken mit EINEM Cast über die Summe (floor statt trunc bei negativem Anteil) · **`02_osc_star`** — exakter r_oscstar-Port: 64 Schritte je Arm, fortlaufender Byte-Index über die 5 Arme, dfactor-Hüllkurve 1/1024→~1/128, `is=min(w·s,h·s)` ganzzahlig · **`14_osc_ring`** — exakter r_oscring-Port: 80 Segmente, Spiegel-Index `q>40?80−q:q`, negativer Winkellauf, EIN impliziter Cast über die Summe · **`17_dot_grid`** — das Gitter stand VERTIKAL GESPIEGELT (Push ohne y-Negation vor `avsZeile`); bei spacing 8 traf nie eine Zeile, dMean sah es nicht. Alle Sternchen-Segmente laufen jetzt durch den linedraw-Bresenham-Port (ThickLines, Breite aus SRM) statt GL-Linien. **`04_blitter_feedback` GRÜN (S60, zwei Befunde):** (1) **blitter_out (Zoom-out) sampelt IMMER nearest** (`src[s_x>>16]`, r_blit.cpp:143) — der `subpixel`-Schalter wirkt nur im Zoom-in (`blitter_normal`/BLEND4); unser Shader bilinear-filterte auch im Zoom-out (konstante Kanten-Differenz ab Frame 2, KEINE Akkumulation — die anderen MAT_TRAIL-Zeilen waren grün). (2) **y-Anker der Abbildung war h−1,5 statt h−0,5** — jede Quellzeile lag eine AVS-Zeile zu tief, der Fehler stapelte sich über die Feedback-Kaskade (Deckung 0,72 → 0,93). Jetzt 0,002/0,007 (320) · 0,001/0,004 (740), Menge 0,01. Restdiff = verstreute Einzelpixel entlang der Waveform (Messerkanten über die Rückkopplung). **Offen (2):** `water` (Chaos-Rauschen, Deckung 0,96) · `grain` (Zufallsstrom-Versatz, Deckung 0,56) — §9-Kandidaten, Abnahme Patrik angefragt (S60, Montagen übergeben). Historie S59: das Flächen-Urteil deckte die fünf Befunde auf (Stand davor 40/43 nach Mittelwert allein) — dünne Zeichner, die die Mittelwert-Metrik nie sehen konnte: `13_rotating_stars` (Ref: winzige Punkt-Sterne, wir GROSSE — Deckung 0!) · `02_osc_star` (Deckung 0,02) · `14_osc_ring` (0) · `17_dot_grid` (gleiche Menge, NULL Überlapp — Verdacht Bewegungs-Akkumulation) · `04_blitter_feedback` (grenzwertig 0,30/0,72). Dazu die drei Altbestände `water` (Chaos-Rauschen) · `grain` (Zufallsstrom-Versatz) · `water_bump` (Trail-Zeile, s. eigener S59-Punkt). Stand vor dem Flächen-Urteil: 40/43. Historie S57: **`dot_grid` ist grün** (dMean 0,047 → **0,000**): wir zeichneten 2 Pixel grosse Punkte statt einzelner, setzten sie auf Zwischenpositionen statt auf ganze Pixel (`xp`/`yp` sind 8.8-Festkomma) und interpolierten die Farbe zu 1 statt zu 63/64. **`interferences` ist GRÜN** (0,053 → **0,027**, dMean 0,025 → **0,001**): die Referenz laesst eine Kopie ausserhalb des Bildes **nichts** beitragen, unser Shader klemmte auf den Randpixel und schmierte ihn nach innen. **`water` erheblich verbessert** (dMean 0,029 → **0,002**, MAE 0,069 → 0,055): Nachbarn ausserhalb werden weggelassen statt geklemmt, und die Halbierung ist ganzzahlig — bei einem rueckgekoppelten Effekt bleibt ein Randfehler nicht am Rand. Der Rest ist **kein Strukturfehler**: die Montage zeigt beide Seiten deckungsgleich, der Diff ist feines Rauschen ueber den Wellenzonen — Chaos-Verstaerkung wie bei `24_grain`. Zwei neue Zeilen kamen hinzu und sind beide grün: `06_blur/02_trail_rounddown` und `03_trail_roundup` (§1c). **Korrektur S53:** der S52-Stand „37/41, vier Reste" war ungenau — `24_grain/01_static100` ist gelb (dMean **0,000**, MAE **0,046**, dreimal identisch gemessen, also kein Rauschen). Die Montage zeigt beide Seiten deckungsgleich, der 4×-Diff ist ein **gleichmäßiges Flächenrauschen**: die Kornmenge stimmt, der Zufallsstrom ist gegen die Referenz versetzt. Kein Strukturfehler — und es betrifft **statisches** Grain, der 🔧-Punkt unten meint das nicht-statische | ✅ |
| ~~**`Dot Fountain` ist nicht portiert**~~ | **0,000 / 0,000 — Diff komplett schwarz** | ✅ **portiert (S57), pixelgenau.** Das 30×256-Gitter, die Alterungs-Schleife (jede Stufe rutscht eine weiter und bekommt dabei ihre Physik), die Erzeugung aus der Wellenform (`t*5/4 - 64`, bei Beat +128) und die 3D-Matrix stehen jetzt zeilengenau nach `r_dotfnt.cpp`. Der letzte Fehler war die Kanalzuordnung: die Farbtabelle liegt in 0x00RRGGBB vor, nicht in AVS-Reihenfolge — rot und blau waren vertauscht, die Montage zeigte es sofort. Matrix-Helfer und `initcolortab` teilen sich Dot Plane und Dot Fountain jetzt (`avsMat*`, `avsInitColorTab`); vorher lagen sie doppelt. **Die Warnung zur Zeile bleibt gültig:** sie mass auch im völlig falschen Zustand 0,002, weil beide Bilder überwiegend schwarz sind — ein flächenbasiertes Urteil fehlt weiterhin. Ursprünglicher Befund: 🔴 **S53:** Die Referenz `r_dotfnt.cpp` ist ein **30×256-Gitter** (7680 Punkte, rotierende Höhenwand, 3D-Matrix `translate(0,-20,400)` wie `Dot Plane`, Höhe aus dem Spektrum). Unser Renderer sind **400 freie Partikel** mit eigener Physik — der Header sagt es selbst („Simplified particle model here"). Die Montage zeigt links einen hohen geordneten Brunnen über die volle Bildhöhe, mittig einen flachen Fleck von ~⅕ der Fläche; der 4×-Diff **ist** das Referenzbild. Die Matrix-Zeile `19_dot_fountain` misst trotzdem 0,002 und zählt zu den 37 — **die Metrik lügt bei dünnen Inhalten**, beide Bilder sind überwiegend schwarz. Faktisch also **36/41**. Fix = echte Portierung nach dem `Dot Plane`-Muster (Matrix + Farbtabellen-Arithmetik liegen dort zeilengenau vor); zusätzlich braucht die Zeile ein **flächenbasiertes** Urteil, sonst bewacht sie weiter nichts | 🔴 |
| ~~Color-Map-Kennlinie~~ | ±1 → **0** | ✅ **gelöst (S57).** Die APE rechnet in DREI ganzzahligen Schritten, und der Verlust steckt in der Schrittweite selbst: `step = 65536/span`, dann `t = (d·step) >> 8`, dann `(a·(256−t) + b·t) >> 8`. Das erklärt, warum Zweierpotenzen exakt waren (65536/16 geht auf) und 200 nicht (327,68 → 327). Unsere alte Formel `a + (b−a)·d/span` traf auf dem Graukeil **1 von 255** Punkten. Jetzt **922/922** über sechs Spannweiten, beide Kennlinien-Blöcke 0 Abweichungen. Die `04_span*`-Sonden existierten seit S49, wurden aber nie ausgewertet — der Bericht prüfte 15 Stichproben, und daran sind fünf Formeln nicht unterscheidbar; die Auswertung ist jetzt in `analyse_colormap.py` | ✅ |
| ~~Colorfade-Zufalls-Beatmodus~~ | — | ✅ **gelöst (S57)**, zusammen mit dem `enabled`-Bitfeld — es war dieselbe Sache. Colorfade folgt jetzt `r_colorfade` zeilengenau: Bitfeld beim Import (1 an · 2 on-beat-random · 4 slow fade), persistenter Fader-Zustand, Nachziehen um EINEN Schritt je Frame samt der Grün/Blau-**Vertauschung** (`faderpos[1]` folgt `faders[2]`, beim direkten Setzen gilt sie nicht), drei exklusive Beat-Zweige. Folge: **ohne `slowFade` wirken die Beat-Fader nicht** — im Original ist ihr Zweig gar nicht erreichbar. Alle 12 Felder wirken | ✅ |
| ~~nicht-statisches Grain~~ | Sonde **0,0363** | ✅ **umgesetzt (S57)**, mit benannter Grenze: der Pfad existiert und flimmert je Frame (§1c), aber die **Zug-Reihenfolge** der Referenz ist nicht nachgebildet — sie läuft sequentiell durch eine 491-Byte-Tabelle und zieht den Faktor nur, wenn die Schwelle trifft, also inhaltsabhängig. Parallel je Pixel nicht berechenbar. Der geteilte `rand()`-Strom wird um die **Obergrenze** `(w·h·2)/16` weitergestellt (die Referenz zieht datenabhängig weniger) — das ist die S49-Merkregel, so weit sie hier einlösbar ist | ✅ |

| ~~**AvsRef lud eine APE-DLL statt seines eingebauten Effekts**~~ | 6 von 8 Läufen verschieden | ✅ **gelöst (S58) — das war die Voraussetzung für alles andere.** `channelshift.ape` ruft in `load_config` ein `srand(time(0))`, und zwar im EIGENEN CRT der DLL: das `srand(kRandSeed)` von `avsref_main` erreicht sie nicht. Die Referenz wählte damit **je Wanduhr-Sekunde** eine andere Kanal-Permutation — acht Läufe im Sekundenabstand, sechs verschiedene Ergebnisse. Jedes Preset mit „Channel Shift on beat" war so unmessbar; die „~0,68" bei Alternate Reality waren zu einem guten Teil das. AvsRef trägt jetzt `initbuiltinape()` VOR `initdll()` ein und überspringt eine `.ape`, die einen eingebauten Namen doppelt (`_dll_name_taken`). Betrifft fünf Namen: Channel Shift · Color Reduction · Multiplier · Holden04: Video Delay · Holden05: Multi Delay. Danach vier Läufe, viermal dasselbe Bild — und genau das, was der `srand(1)`-Strom vorhersagt | ✅ |
| ~~**Channel Shift zog einen Zufallszug zuviel**~~ | Referenz BGR, wir BRG | ✅ **gelöst (S58).** `r_chanshift:124-126` zieht NUR auf dem Beat; bis dahin gilt die Vorgabe aus dem Preset. Wir zogen zusätzlich im ersten Frame (`apeChanMode < 0`) — ein Zug zuviel aus dem geteilten Strom, danach war jede weitere Permutation um eine Ziehung versetzt. Der Startwert kommt jetzt aus dem **Preset-Wert** und wird bei Änderung neu gesetzt (Bauart wie `interferences.rotation`, S57) | ✅ |
| ~~**Convolution-APE: bias/absolute/twoPass falsch verstanden**~~ | 33/33 Sonden exakt | ✅ **gelöst (S58).** Die APE rechnet ganzzahlig in BYTES: `x = (summe + bias·256) / scale`, Richtung Null abgeschnitten, dann 0..255 geklemmt. **`bias` zählt also in ganzen 256ern** — schon `bias=1` hebt jeden Kanal über 255; wir addierten `bias/255`. Belegt an (200,100,40): `bias=1/scale=2` → (228,178,148), `bias=1/scale=256` → (1,1,1). Dazu: **`absolute` ist kein Betrag** (negatives Ergebnis → 255; Kern −4 bei scale 2 misst weiß statt 80), **`twoPass` verdoppelt** statt zweimal zu falten (Kern 1 bei scale 4 → v/2, nicht v/16), `scale=0` gilt als 1, und der Konfigurationswert von `bias` wirkt als **vorzeichenbehaftetes Byte** (+128 wie −128, +256 wirkungslos, +257 wie +1). Fünf Dauersonden in `2_trans/convolution_*` | ✅ |
| ~~**SuperScope: die letzte Bildspalte**~~ | Spalte w−2 statt w−1 | ✅ **gelöst (S58).** `SuperscopePoint::x/y` waren **float**. `x=1-2/w` ergibt in double exakt `w−1`, über float gerundet `w−1−ε` und damit `w−2`. Die Formel in `renderPixelDots` (`(int)((x+1)·w·0.5)`) war richtig, nur der Eingangswert war verloren. Der Fehler steckt in **jeder** Bildbreite, fällt aber nur auf, wenn ein Effekt genau diese Spalte abtastet. Jetzt double bis zur Pixelabbildung; die Vertex-Puffer bekommen weiter float, ausdrücklich gecastet. **Ohne automatischen Wächter** — eine einzelne Spalte liegt unter jeder Schwelle der Sonden-Metrik (Menge 0 · Lage 1 px · MAE 0,003); ein Versuch mit Movement-Verstärker maß am Ende das Movement und ist wieder raus | ✅ |
| ~~**Dynamic Shift: weicher Rand statt harter Kante**~~ | Randspalte exakt | ✅ **gelöst (S58), zieht Bright Light District mit** (dMean 0,270 → 0,026). `r_shift.cpp:190-310` verschiebt in ganzen Pixeln plus 8-Bit-Bruchteil, und der Bruchteil kippt asymmetrisch: `if (part<0) part=-part; else { ++a; part=255-part; }`. Daraus folgt, was man nicht rät — **eine Verschiebung von NULL in y kostet die erste und die letzte Zeile** (`ya++`, `endy=h-1+ya`), `x=-.2` kostet die letzte **Spalte**, und die Referenz schreibt sie **hart auf Schwarz**. Wir verschoben normiert mit weichem Rand, der Saum blieb stehen. Dazu war der Blend vertauscht (`BLEND_ADJ(verschoben, framebuffer, ialpha)` — das VERSCHOBENE Bild trägt `ialpha`) und `ialpha ≤ 0` heisst in der Referenz **sofortige Rückkehr**, nicht Durchsicht | ✅ |
| ~~**Der Beat gehörte dem FRAME statt der LISTE**~~ | AR 0,339 → **0,029** | ✅ **gelöst (S58) — der grösste Einzelschritt der Session.** `SET_BEAT`/`CLR_BEAT` eines Kindes wirken in `r_list.cpp:747-751` nur auf die lokale `isBeat`-Variable der Liste, also auf deren nachfolgende Kinder; dasselbe gilt für das `beat` des Listen-Skripts. Wir stellten `m_frameBeat` global um. Folge: ein `Custom BPM` in einer Unterliste filterte den Beat für die GANZE Kette, und jeder Effekt mit Beat-Code dahinter zog seine Zufallszahlen viel seltener als in der Referenz — bei Alternate Reality acht `rand(4)` je Dynamic Movement. `renderList` sichert und stellt den Beat jetzt wieder her | ✅ |
| ~~**SuperScope `skip` verwarf den Punkt**~~ | zwei von drei Flügeln fehlten | ✅ **gelöst (S58).** `skip` unterdrückt in AVS NUR das Zeichnen des Segments, das in diesem Punkt endet — `lx/ly` werden trotzdem gesetzt (r_sscope:295-334, die Zuweisung steht HINTER dem if-Block), der Punkt bleibt also Ankerpunkt. Wir verwarfen ihn ganz; bei einem Skript, das `skip` je Punkt umschaltet (`ip=bnot(ip); skip=ip`), blieb damit zwischen zwei gezeichneten Punkten immer ein Bruch und es wurde **gar nichts** gezeichnet. Dazu die Schwelle: die Referenz prüft `< 0.00001`, nicht `> 0.5`. Folgefehler derselben Änderung, mitgefixt: die Richtungsbestimmung der dicken Linien fragte den VORGÄNGER auf `skip` ab und gab dem Endpunkt eines Strichs die Richtung (0,0) — das Viereck war verdreht und deckte statt sechs nur drei Spalten. Sonde `1_render/scope_skip_wechsel` | ✅ |
| ~~**Picture II: drei Betriebsarten statt sechs**~~ | 0,22 → **0,002** | ✅ **gelöst (S58).** An der Referenz gemessen (Sonden `p2_bm0..5` auf bekanntem Untergrund): **0 ersetzen · 1 additiv · 2 Maximum · 3 Minimum · 4 50/50 · 5 Subtraktion (Framebuffer minus Bild)**. Unser Import warf alles ab 2 auf 50/50. In „The Real Impressionist" mischt der zweite Aufruf mit **Maximum** — daraus wurde ein Mittelwert, und das Bild lief hell aus. Der Bildshader nummeriert weiter 0..2 wie bisher (dort hängt `Picture`, ID 34, dran), der Import bildet über eine Tabelle ab | ✅ |
| ~~**Scope-Linien lagen eine Spalte links**~~ | Diagonale 159..161 → **160..162** | ✅ **gelöst (S58).** AVS zieht Linien zwischen GANZZAHLIGEN Pixeln (`x=(int)((var_x+1)*w*0.5)`, dann Bresenham), GL rastert gegen Pixel-MITTEN — eine Stützstelle bei Fensterkoordinate 160,0 liegt genau auf der Grenze und fällt eine Spalte nach links. Die Stützstellen werden jetzt auf die Mitte des Pixels gesetzt, den AVS berechnet. Aufgefallen ist es erst durch eine 240-fache Vergrösserung in Bright Light District; gemessen am Referenzbild selbst (Diagonale je Zeile). Wirkung: BLD 320 von 0,070 auf **0,017**, Matrix-Zeile `36_superscope` auf **0,000/0,000** | ✅ |
| ~~**Scope-Segmente hatten einen Farbverlauf**~~ | Stufe t03 → **0,000** | ✅ **gelöst (S58).** AVS färbt ein Liniensegment EINFARBIG: `line(framebuffer, lx,ly, x,y, w,h, thiscolor, linesize)` bekommt die Farbe des AKTUELLEN Punktes und malt damit die ganze Strecke vom Vorgänger her (r_sscope:297/325) — entlang der Strecke wird nichts interpoliert. Wir liessen die Vertexfarben interpolieren; bei wenigen Stützstellen mit starkem Verlauf ist das sichtbar („Lost Cause" zieht eine Linie über FÜNF Punkte von Weiss nach Blau). `flat` mit der GL-Vorgabe LAST_VERTEX trifft die Referenz genau: das Segment bekommt die Farbe seines Endpunkts | ✅ |
| ~~**12 Lost Cause**~~ | → **0,002 / 0,002** | ✅ **grün (S58/S59):** Farbtafel-gehört-dem-Frame-Fix (S58); mit den S59-Linien-Umbauten nachgemessen 0,002. Ursprünglich: 🟠 **NEU (S58, Befund Patrik: im Vollbild vorher schwarz).** Die Geometrie stimmt — derselbe Tunnel, dieselben Stufen; bei uns läuft das mittlere Band nach WEISS aus, wo die Referenz ein geschichtetes Blau behält. `dMaxLuma` ist in beiden Größen exakt 0,175, also ein konstanter Helligkeitsüberschuss. **Eingegrenzt:** ohne den Fade Out in der Liste ist das Preset grün (0,012 / 0,023) — der Fade ist aber nicht die Ursache, sondern macht sie sichtbar: ohne ihn sättigt die Schleife auf beiden Seiten und verdeckt alles. **Ausgeschlossen (gemessen):** die Abklingkurve des Fade Out selbst (184/168/152/120/72/8/0 auf beiden Seiten identisch) · die Beat-Zeitpunkte (Leiter-Sonde: acht Sprossen in denselben Zeilen) · die Dynamic Movement mit den zwei statischen Scopes (0,003). **Der Verlauf zeigt, wo es passiert:** bis Frame 31 grün (0,005), bei Frame 60 schon 0,141 — die Abweichung entsteht in der ABKLINGPHASE nach der Injektion, nicht bei der Injektion. Die Schleife ist: Liste mit `blendIn=0` (eigener, stehender Puffer) + Fade Out innen, `blendOut=4` (additiv) in den Elternpuffer, der von einer Dynamic Movement getragen wird | 🟠 |
| ~~**splendora**~~ | 0,178/0,157 → **0,003/0,002** (320) · **0,004/0,006** (640) | ✅ **grün (S59), zwei Befunde:** (1) **Channel-Shift-Mode 1023 ist RGB** — das Preset trägt die Dialog-IDs der ORIGINAL-`channelshift.ape` (aus der DLL-Ressource geparst: 1023=RGB), der vis_avs-Port verlegte RGB auf 1183; unser default-Zweig klemmte 1023 auf BGR = R↔B-Tausch. (2) **Eine AVS-Linie malt ihr MAX-Ende nie** (linedraw.cpp ende-exklusiv, Fast-Paths klemmen VOR der Exklusivität auf h−1/w−1) — der Movement-Clamp des Ring-Hintergrunds sampelte die unterste Zeile, bei uns mit der falschen Farbe. `renderThickLines` läuft jetzt je Segment den Integer-Bresenham der Referenz mit; `linesize` RUNDET (`(int)(x+0.5)`) | ✅ |
| ~~**el-vis_hypno07_FTL01_v2**~~ | 0,104/0,248 → **0,008/0,009** (320) · **0,008/0,008** (640) | ✅ **grün (S59), Uhren-Familie:** Das Preset misst per `gettime`-Zählfenster seine eigene FPS. (1) AvsRef maß die WANDUHR (Batch: 120 Frames in Sekundenbruchteilen → fps=0) — Patch `--tick-hz N` = virtuelle Frame-Uhr in `patched/avs_eelif.cpp`, Harness-Default 60. (2) Unser Host injizierte jeden Frame `time` und überschrieb die USER-Konstante `time=2.0` — `compileAll()` scannt jetzt auf Zuweisung, dann keine Injektion. (3) Skript-Uhr: Start bei 0 statt 1/60 (`m_scriptClock` VOR dem Inkrement), double-Akkumulator, ms-Körnung wie GetTickCount. (4) **Texer-II-Resampler bit-exakt** (s. eigene Zeile) | ✅ |
| ~~**Texer-II-Resampler**~~ | Sprite-Ringe ~1 px → **14 Impuls-Fälle bit-exakt** | ✅ **S59, per IMPULSANTWORT reverse-engineered** (1-Pixel-Bild durch die Original-APE = der Sampling-Kernel): Zentrum `(pos/2+0,5)·(dim−1)` · Rechteck `c ± iw·s/2 ∓ 0,5` (iw=IW−1, fistp) · Phase `(r2.left−r.left)/Spanne` · Schritt `(iw−1)/(iw·s)` in 16.16 · Malende `max(ceil(r.right), r2.right|1)` · 8-Bit-Gewichte mit XOR-Komplement + Trunc-Kaskade (Peak 252). Default-Sprite = gemessene 21×21-Matrix (no-resize = 1:1-Blit). Community-Quelle (grandchild) weicht numerisch ab — Struktur-Vorlage, keine Bit-Referenz. Nebeneffekt: `paar_original` grün | ✅ |
| **el-visVR09(war)** | Gate referenz-treu; Rest = Decoder | 🟠 **S60, zweiter Teil: das `speed`-Gate ist jetzt REFERENZ-TREU** — der Befund war unser Sofort-Start (`aviTexture==0` zündete Video-Frame 0 bei t=0; die Referenz zeigt bis zum ersten Gate-Ablauf nichts und lag dauerhaft einen Video-Frame zurück). Bewiesen mit drei Gate-Sonden auf dem deterministischen Testvideo: **speed 0/100/400 alle dMean 0,000** (vorher 0,29–0,34). Stream-Länge verifiziert identisch (`r_avi length=31` = unser Cache). **Der VR09-Rest (0,385/0,220) ist reine DECODER-DIFFERENZ** (VfW-Indeo-32-bit vs. Qt/FFmpeg-Indeo, durchs Feedback aufgeschaukelt) — in 64-Bit nicht bit-exakt schließbar → §9-Kandidat nach Deinem **Sichturteil mit echtem Ton**. avi-Feld-Sonden 6/6 WIRKT, Edit 4/2/0 | 🟠 |
| ~~**Convolution „wrap"-Arithmetik**~~ | vermessen + umgesetzt | ✅ **S60, an der ORIGINAL-APE vermessen** (Sonden `convolution_wrap_neg`/`_scale` + scale-2/4/128-Grenztest): bei **scale 1 ist wrap wirkungslos** (kein Divisionspfad im JIT — identisch zur Sättigung), ab **scale ≥ 2** gilt `lane16 = (pos−neg) mod 65536` mit **UNSIGNED-Division** durch scale (Belege exakt: Untergrund 16 → −16 → 65520/256 = **255**; Linienpixel −494 → 65042/256 = **254**). Shader rechnet den Zweig jetzt ganzzahlig nach (pos/neg getrennt akkumuliert, Normal-Pfad byte-identisch); exklusiv mit `absolute`; bias+wrap unvermessen (dokumentiert). Modul-Sonden **91/91**, `convolution.edgeMode` **WIRKT** (Grundkonfig scale 2) | ✅ |
| ~~**Rand der Convolution: der INHALT**~~ | 552 px → **alle 8 Sonden MAE 0,000** | ✅ **GELÖST (S59).** Tom Holdens Original-Quelle (2002) lebt als Laufzeit-JIT im community-vis_avs (`e_convolution.cpp`). Daraus + Gradient-Sonden: Reads klemmen IMMER auf Zeile h−2/Spalte w−2 (die letzte Eingabe-Zeile/-Spalte wird NIE gelesen); bei `zerostringl ≥ 24` (erster Kernel-Eintrag ab Zentrum) schreibt die APE IN-PLACE — die zurückgeklemmten Rand-Taps lesen dann schon BEARBEITETE Pixel („doppelt gefaltete" Kante, 16→5→1). Als exakter Zwei-Pass umgesetzt. **Dazu: „wrap" ist KEIN Koordinaten-Wrap** (Fehldeutung S57), sondern psubw statt psubusw — Überlauf statt Sättigung der Negativ-Verrechnung (darum exklusiv mit `absolute`); der fract()-Zweig ist raus. **Offen als eigener Punkt: die Überlauf-Arithmetik selbst ist unvermessen** (keine Sonde mit Negativ-Kernel + wrap). Nebeneffekt: Alternate Reality 0,026 → **0,002** — echt grün, die S58-Rausch-Abnahme ist hinfällig. Ursprünglich: 🟠 **S58, Nebenbefund.** Das Modell aus S57 („die APE berechnet die letzte Zeile und Spalte nicht") stimmt, aber der Zielpuffer enthält bei uns etwas anderes als bei der Referenz. Drei der fünf neuen Convolution-Sonden melden deshalb PRÜFEN — immer mit derselben Signatur: unsere Pixelzahl liegt um **552** über der Referenz. Sichtbar, sobald eine Faltung die Bildhelligkeit ändert. Bei Alternate Reality als Ursache **ausgeschlossen** (Abweichung dort gleichmäßig verteilt) | 🟠 |

### ✅ `41_interferences` gelöst — und was `water`/`water_bump` daraus lernen (S57)

**Die Randbehandlung war die Ursache.** Die Referenz liest eine verschobene Kopie
nur innerhalb des Bildes (`if (xp >= 0 && xp < w && yoffs[i] != -1)`, :236) und
laesst den Beitrag sonst auf **0**; unser Shader klemmte auf den Randpixel und
schmierte ihn nach innen. **dMean 0,025 → 0,001, MAE 0,053 → 0,027, Zeile grün.**

Dazu zwei weitere Abweichungen von `r_interf.cpp`, beide vorher behoben (sie
allein brachten 0,053 → 0,051):

1. **Die Übergangswerte sind ganzzahlig.** Der `(int)` steht in der Referenz um
   die Interpolation, nicht um das Ergebnis
   (`_distance = distance + (int)((float)(distance2-distance) * s)`, :194-196),
   und die Versätze sind **ganze Pixel** (`xpoints[i] = (int)(cos(a)*_distance)`,
   :205). Wir gaben den Bruchteil an den Shader, der dazwischen interpoliert.
   Ebenso ist `rotation` dort ein **int** und wird ganzzahlig akkumuliert (:384).
2. **Die Gewichtung läuft über die Byte-Tabelle** `g_blendtable[_alpha][wert]`
   (:216), also `(alpha·wert)/255` als Ganzzahl — jede Kopie verliert bis zu ein
   255stel, und bei vier Kopien summiert sich das. Der Shader rechnet das jetzt
   in 8-Bit-Einheiten nach, wie der Blur seit S57.

**Dieselbe Klasse bei `water`** (dMean 0,029 → 0,002): dort behandelt die Referenz
jeden Rand als eigenen Zweig (Ecke 2, Kante 3, Mitte 4 Nachbarn) und halbiert
**ganzzahlig**. Eine Kuriosität hat sich dabei NICHT bestaetigt: die oberste und
unterste Zeile werden im Original gar nicht halbiert (:168-188) — nachgebildet
wurde das Bild deutlich schlechter (0,131), AvsRef zeigt diesen Saum also nicht.
Der Sonderfall ist bewusst weggelassen.

### ~~`31_water_bump`~~: Ganzzahl-Kern portiert (S59) — ✅ GRÜN (S60): der „Trail-Rest" war ein stilles Uniform-Loch

**S60, gefunden durch den Feld-Sonden-Vollauf (10× STUMM):** `setzeBlob` setzte
`uDropPos` und `uResI` per `setUniformValue(QPoint)` — **Qt lädt QPoint als
FLOAT-vec2 (`glUniform2fv`) hoch**, die Shader deklarieren `ivec2`. Der stille
GL-Typfehler ließ beide Uniforms auf (0,0); mit `uResI=(0,0)` nahm jedes Pixel
den Rand-Zweig, und dessen doppelte Y-Spiegelung ergab zufällig die perfekte
Identität: der ganze Knoten war ein **bit-exakter Passthrough** — ohne Warnung.
Die S59-Schlussmessung der Trail-Zeile (0,100/0,148) maß diesen Passthrough,
nicht den Ganzzahl-Kern. Fix: Integer-Uniforms IMMER über `glUniform*i`
(wie beim `uDropClip`-ivec4 schon geschehen). **Danach: Feld-Sonden 10/10
WIRKT · Matrix-Zeile 31 GRÜN — 0,003/0,011 (320) · 0,002/0,006 (740),
Deckung 0,98.** Die in S59 geplante Trail-Kreislauf-Sonde ist damit hinfällig.
Merkregel: **eine Qt-Komfort-Überladung sagt nichts über den GLSL-Typ** —
`QPoint` sieht nach int aus und kommt als float an. — Historie S59:

**S59: der komplette Umbau auf Referenz-Einheiten ist drin** (SineBlob
`(int)((cos+0xffff)·h)>>19` mit CPU-geclipptem ende-exklusivem Rechteck,
CalcWater `(sum8>>2 − prev) − (newh>>density)`, Displacement als LINEARER
Pufferversatz mit Zeilenüberlauf, Rand = feste Wand, Puffer RGBA32F,
`displaceScale`-Default 1,0 statt 6,0). **Statisch referenztreu:** auf dem
bit-identischen refbild bleiben 28–48 Restpixel von 76 800 (GPU-cos vs.
x87-cos an /8-Messerkanten). **Offen: die Trail-Matrixzeile** — 320:
0,100/0,148 (vor dem Umbau 0,034/0,134), 640 gleichauf (0,007/0,027). Der
Nachbau des 1-Frame-Rand-Lags (ref-fbout = Eingabe des Vorframes) verschluckte
den ersten Frame (Ursache im GL-Ablauf nicht verortet) und verschlechterte die
Messung → nach Merkregel zurückgenommen; Rand = Kopie der aktuellen Eingabe
(bewusste Näherung). ✅ **Entschieden (Patrik, S60): der Ganzzahl-Kern
bleibt** (quellentreu + statisch exakt). Offen ist nur noch die
Trail-Kreislauf-Sonde für den 320er-Rest (S57-Lehre: die Sonde muss den
Kreislauf schließen) — §0 Haken 5. — Historie S57:

Die Montage zeigt einen **strukturellen** Unterschied, keinen Rundungsrest: die
Referenz ist grob **gestuft** (zerklüftete Farbflächen), unsere Welle glatt und
regelmäßig. Der Grund steht in einer Zeile:

```c
ofs = offset + buffer_w*(dy>>3) + (dx>>3);   // r_waterbump.cpp:330
```

Der Versatz ist eine **ganzzahlige Pixelverschiebung**. Der Shift rundet gegen
−unendlich, und bei einer Höhendifferenz unter 8 ist der Versatz schlicht
**null** — daher die Stufen. Wir verschieben stufenlos und interpolieren linear,
zeichnen also glatte Ringe.

**Ein blosses `floor()` reicht nicht** — nachgemessen wird es damit schlechter
(0,137 → **0,233**): unsere Höhen stehen nicht in den Einheiten der Referenz,
also quantisiert es an der falschen Stelle. Der Höhenpuffer ist RGBA16F, der
Wertebereich also nicht das Problem; die **Skala** ist es. Umzustellen wären
zusammen: die Tropfen-Erzeugung (`SineBlob` mit `>> 19`), die Dämpfung
(`newh - (newh >> density)` — ein arithmetischer Shift mit eigener Asymmetrie bei
negativen Werten) und das Displacement.

Eine referenztreue Teilkorrektur ist bereits drin (ohne Messgewinn, 0,136 →
0,137): die Referenz **beschreibt die äusserste Zeile und Spalte nie**
(`CalcWater` läuft von `buffer_w + 1`, die Puffer sind nullinitialisiert), dort
steht also eine feste Wand, an der die Welle reflektiert.

**Vor „Regression!" den Vorstand MESSEN** (stash + Rebuild), nie gegen notierte Zahlen
einer anderen Messreihe — zwei von drei Auffälligkeiten in S51 waren auf HEAD identisch.

**Und: die Montage ansehen, bevor man eine Zahl deutet.** Bei Alien Alloy stand hier
acht Zeilen lang „es fehlt Menge, die Sprites sind zu wenige". Tatsächlich war unser
Bild *schwarz bis auf die Sprites* — die gemessenen 4685 Pixel WAREN die Sprites, und
gefehlt hat alles andere. Aus der falschen Leseart folgten drei Sonden, die alle grün
waren und nichts fanden (`reg00`-Transport, `sizex`-Vertrag, Randgeometrie).

**Die Streuung liegt bei AvsRef, nicht bei uns** (S52, gemessen auf Nachfrage Patrik).
Vier Läufe desselben Presets, jeder Renderer mit **sich selbst** verglichen:

| | MAE über vier Läufe |
|---|---|
| **AvsStandalone (wir)** | **0,0000** — bit-identisch |
| **AvsRef (Referenz)** | 0,055 – 0,064 |

Jeder Vergleich hat damit eine Rauschgrenze, unter die gar nicht gemessen werden kann.
`--beat-period` hilft nicht (0,071) — der Beat ist nicht die Quelle. Zwei belegte
Mechanismen: `r_chanshift.cpp:340` ruft `srand(time(0))` in **`load_config`**, jedes
Preset mit Channel Shift sät den CRT-Strom beim Laden mit der aktuellen Sekunde neu;
und AvsRef lädt mit `--ape-dir` echte APE-DLLs, deren Fremdcode eigene Zeitbezüge haben
kann. **Vor jeder Deutung einer Einzelzahl: mehrfach messen.**

**Beat-getriebene Presets sind mit `--beat-period 0` gar nicht vergleichbar** (S52).
Beide Renderer erkennen Beats selbst aus dem synthetischen Audio und kommen dabei auf
verschiedene Zeitpunkte; wo ein Effekt je Beat Zustand aufbaut, divergiert er
zwangsläufig, ohne dass ein Render-Fehler vorliegt. Beleg Moving Particle: die Physik
ist zeilengleich mit `r_parts.cpp`, und mit festem Beat konvergiert es —
beat-period 0 → MAE 0,042 · 24 → 0,005 · **60 → 0,001**. Solche Presets deshalb
**immer mit `--beat-period`** vergleichen, sonst misst man die Beat-Erkennung statt
des Effekts.

**Eine Sonde, die den Hintergrund jeden Frame neu zeichnet, kann keinen
Rückkopplungs-Verlust sehen.** Vier weitere Sonden blieben deshalb grün (DM+Texer,
DM-Flags, alle drei Blend-Modi): sie säten mit einem Vollbild-Muster je Frame neu.
Erst der Nachbau des echten Paares — Sprites am Rand als **einzige** Energiequelle —
zeigte die Divergenz. Wächst der Abstand über die Frames (hier 0,010 → 0,568), ist
die Ursache im Kreislauf, und die Sonde muss ihn schließen.

## 1b. Wirkt ein Feld beim EDITIEREN wie nach dem Laden? (Strang F, S55)

**Herkunft:** Beobachtung Patrik — „der wirkliche Effekt wird erst sichtbar,
wenn man das Preset gespeichert hat und es wieder geladen wird", bemerkt an
Movement. Der Mechanismus ist belegt: **Laden** setzt
`m_pendingRuntimeReset` → `resetRuntimes()`, jeder Knoten baut seine Runtime
frisch auf; **Editieren** ruft `recompileChain()`, und das ist nur
`compileChain(m_root)` — **kein** Runtime-Reset. Was ein Knoten einmalig beim
Aufbau übernimmt, hängt danach fest.

**Werkzeug:** `asset/calibration/fields/run_edit_probes.py` + `--edit-nach` im
AvsStandalone (bildet den Panel-Weg nach: Params tauschen, `recompileChain()`,
kein Reset). Urteil über **vier** Bilder — geladen · editiert · Vorgabe ·
Gegenrichtung: `TEILWEISE` = wirkt, trägt aber noch die Vorgeschichte (bei
Effekten mit Verlauf der Normalfall) · `GLEICH`. Ist editiert Pixel für Pixel
die Vorgabe, entscheidet das **vierte** Bild (S56, s. u.):
`WIRKUNGSLOS` = auch die Gegenrichtung ändert nichts, der Wert kommt nicht an
(harter Befund) · `VERDECKT` = die Gegenrichtung ändert das Bild, der Wert
kommt an und wird hier nur von der Vorgeschichte verdeckt (**kein** Befund).

**Stand:** die 15 WIRKUNGSLOS aus S55 sind abgearbeitet — **7 echte Befunde,
alle gefixt** (3 in S55, 4 in S56), **8 waren Messartefakte** (`VERDECKT`).
Nebenbei: die Rede von „13 verbliebenen" war eine Fehlzählung — `avi` stand mit
**zwei** Feldern in der Liste (`filename` · `resolvedPath`), es waren 12.

| Feld | MAE | Stand |
|---|---|---|
| ~~`movement.sourceMapped`~~ | 0,081 | ✅ **gefixt S55** — wurde nur bei frischer Runtime übernommen (`< 0`). Jetzt wird der zuletzt übernommene Preset-Wert mitgeführt, das Beat-Kippen bleibt. Nachgemessen: Movement 7/7 GLEICH |
| ~~`avi.filename` · `avi.resolvedPath`~~ | 0,234 | ✅ **gefixt S55** — `aviTried` merkte sich nur, DASS geöffnet wurde; ein Pfadwechsel griff nie. Jetzt Pfad-Schnappschuss + Neuöffnen (Textur wird verworfen). Nachgemessen: keine WIRKUNGSLOS mehr, Feld-Sonden 6/6 ohne Regression |
| ~~`texer.imageData`~~ | 0,070 | ✅ **gefixt S56** — `ensureEmbeddedTexture` stieg bei `picTexture != 0` aus, die Textur hing am Aufbau fest. Jetzt Schnappschuss der Bilddaten (`picSnapshot`, Bauart wie `cmSnapshot`), bei Wechsel wird neu hochgeladen. Nachgemessen: texer 6/6 GLEICH; Picture/Picture II/Texer II 19/19 GLEICH ohne Regression |
| ~~`milkdrop.meshX` · `meshY` · `debugGrid`~~ | 0,036 · 0,053 · 0,006 | ✅ **gefixt S56** — die drei standen IM Revisions-Block von `runMilkdropNode`, und `revision` zählt nur Preset-/Skript-/Shader-Edits. Es sind reine `setParam`-Zuweisungen ohne Neuaufbau, stehen jetzt je Frame außerhalb. Nachgemessen: debugGrid GLEICH, meshX/meshY TEILWEISE (Feedback-Puffer), keine WIRKUNGSLOS |
| ~~`bufferSave.slot` · `dir` · `initCode` · `frameCode` · `beatCode`~~ | 0,109 | ✅ **kein Befund (S56)** — `VERDECKT`, s. u. |
| ~~`bassSpin.smoothing`~~ | 0,184 | ✅ **kein Befund (S56)** — `VERDECKT` |
| ~~`customBpm.skip`~~ | 0,109 | ✅ **kein Befund (S56)** — `VERDECKT` |
| ~~`mirror.slower`~~ | 0,014 | ✅ **kein Befund (S56)** — `VERDECKT` |

### Der Quercheck aus S55 war falsch (Korrektur S56)

S55 erklärte alle 13 für bestätigt, mit dieser Regel: *wirkt das Feld in den
Feld-Sonden, und ist der Edit-MAE gleich dem Feld-MAE? Dann ist es ein Befund.*
**Die Regel unterscheidet nicht, was sie unterscheiden soll.** „Editiert ==
Vorgabe" hat zwei mögliche Ursachen, und beide erfüllen sie:

(a) der Wert kommt beim Edit nicht an — der Befund;
(b) der Wert kommt an, kann aber nichts mehr ausrichten, weil der Zustand aus
    der ersten Hälfte ihn **verdeckt** (der Puffer hält seinen Inhalt, die
    Rampe ist abgelaufen, der Zähler steht anderswo).

Getrennt wird das durch die **Gegenrichtung**: Start = Sonde (Feld gesetzt),
Edit → Grund. Der Zustand der ersten Hälfte ist dann der des *gesetzten* Feldes
und kann den Rückweg nicht auf dieselbe Weise verdecken. Ergebnis für alle
acht: **das Bild ändert sich** (`bufferSave.*` 0,1094 · `bassSpin.smoothing`
0,2295 · `customBpm.skip` 0,1094 · `mirror.slower` 0,0143) — der Wert kommt an.

Dazu zwei **Positivkontrollen**, die dieselbe Vorwärtsrichtung fahren und nur
die Verdeckung wegnehmen:

- `bufferSave.slot` mit dem Leser auf dem **leeren Slot 7**: WIRKUNGSLOS →
  **GLEICH** (MAE 0,0000). Der Slot-Wechsel kommt vollständig an.
- `mirror.slower` mit `onBeatRandom`, damit die 16-Stufen-Rampe nie zur Ruhe
  kommt: WIRKUNGSLOS → **TEILWEISE**.

Die Sonde kann das jetzt selbst: das vierte Bild ist in `run_edit_probes.py`
eingebaut und kostet nur im Verdachtsfall einen Renderlauf. Neues Urteil
`VERDECKT`; der Rückgabewert bleibt nur bei `WIRKUNGSLOS` ungleich 0.

**Merkregel bleibt, aber schärfer:** `WIRKUNGSLOS` ist wie `STUMM` zuerst eine
**Frage**. Beantwortet wird sie weder mit einer plausiblen Geschichte noch mit
einem Quercheck, der beide Ursachen gleich behandelt, sondern mit einer
Messung, die sie **trennt**.

**Geprüft und in Ordnung:** alle 16 Knoten mit Verlauf (multiDelay, videoDelay,
bufferSave, blitterFeedback, rotoBlitter, waterBump, water, fyrewurX,
movingParticle, bassSpin, timescope, avi, customBpm, reactionDiffusion,
fractalZoomer, milkdrop) übernehmen ihre Felder **unbedingt je Frame** — keine
einzige bedingte Übernahme (statische Prüfung S55, Vorgabe Patrik).

Das passt zum Befund S56: kein einziger der acht `VERDECKT`-Fälle liegt an
einer festgehaltenen Übernahme — sie lesen ihre Felder je Frame, wie hier
statisch geprüft, und die Gegenrichtung bestätigt es messend. Auch
`bufferSave.initCode`/`frameCode`/`beatCode` sind sauber: `runParamScript`
übersetzt bei jeder Textänderung neu (`paramCompiled != combined`).

**Entscheid Patrik S55:** einzeln je Knoten reparieren, nicht generisch. Ein
generisches Verwerfen des Aufbau-Zustands bei jedem Reglerdreh würde Skripte
neu übersetzen und Bilder/Videos neu laden — das kann beim Ziehen ruckeln, und
die Grenze zwischen „Aufbau" und „Verlauf" müsste je Knoten von Hand gezogen
werden.

## 1c. Die stummen Feld-Sonden (Strang E, Durchsicht S56–S57)

**Stand: 115 → 15 → 0** (S57 abgeschlossen). `STUMM` ist wie `WIRKUNGSLOS`
**zuerst eine Frage** — aber nicht immer dieselbe Antwort: von den ersten 100
durchgesehenen waren **zwei** ein Befund an der App (Roto Blitter, Kleinian), von
den **letzten 15 sechs**. Die leichten Fälle standen vorn, und wer nach 100
Messartefakten aufhört, lässt die Befunde liegen.

### Der eine App-Befund: die Alt-Format-Weichen griffen zu oft

Drei Stellen im Deserialisierer erkannten ein Alt-Dokument an der **Abwesenheit
des neuen Felds** statt an der **Anwesenheit des alten**. Ein Preset, das
`zoomScale` schlicht nicht nennt, bekam deshalb den Migrationspfad — und der
rechnet aus Werten, die nie im Dokument standen:

```
Roto Blitter ohne `zoomScale`  ->  zoomScale = (1,0 - 1,0) * 1024 = 0
                                   statt des neutralen 31
```

Der Knoten zoomte so weit hinein, dass das Bild **einfarbig gelb** war; alle
fünf seiner Felder galten als stumm. Dieselbe Bauart bei `blitterFeedback`
(`scale`) und `simpleScope` (`mode`). Die Weiche hängt jetzt am Altfeld, und ein
Dokument mit beidem folgt dem neuen. **Roto Blitter 5 → 0, Simple Scope 1 → 0.**

Das ist auch die Auflösung der „Migrations-Artefakte" aus §1d: der Zweig griff
wirklich zu oft — nur war der Schluss „mein Test ist zu streng" halb falsch.

### Alles andere lag am Testaufbau oder am Werkzeug

| Ursache | Fälle | Beispiel |
|---|---|---|
| Betriebsart nicht aktiv | ~20 | `list.inAdjustAlpha` braucht Blend = Adjustable · `colorClip.distance` nur im Modus „near" · `convolution.absolute` braucht einen Kern ≠ Identität · `superScope.colors` wird bei `colorBlend = 0` nie gelesen (der Verlauf gewinnt) |
| Beat-Ziel gleich dem Normalwert | 9 | `movingParticle.size2`, `blitterFeedback.scale2`, `interleave.x2`, `mosaic.quality2` |
| Knoten zeichnet nichts | 5 | **Texer II** hat kein `pointCount`; `n` kommt allein aus dem Init-Slot. Ohne ihn läuft der Punkt-Code null mal — gemessen identisch mit „gar kein Knoten" |
| Zähler-Phase | 4 | **Custom BPM** mit `skipCount = 1` lässt die Beats 2, 4, 6 durch — der siebte (Schlussframe) fällt heraus, und der Gegenwert 16 lässt nie durch. Beide löschten am Schluss nicht. Lauflänge auf 151 (sechster Beat) |
| `{"type": 1}` statt `{"ftype": 1}` | 8 | `type` ist der KNOTENTYP — der Eintrag machte aus dem Fraktal-Knoten den unbekannten Typ „1", und daraus baut der Leser bewusst einen **Passthrough** |
| Gegenwert trifft die Vorgabe | 6 | `timescope.blend` · `kleinian.colorScale` (ganzzahlig, s. u.) · `text.normSpeed` · `interferences.rotation` (255 = volle Umdrehung = 0) |
| Trennzeichen / erfundenes Nachbarfeld | 7 | Der Text-Knoten trennt mit `;`, nicht mit Leerzeichen · `text.xShift` nannte ein `shiftSpeed`, das es nicht gibt |
| Nicht prüfbar, mit Grund | 3 | `hostgroup.curveIn`/`curveOut` (nur linear implementiert) · `customBpm.arbitraryMs` (hängt an der **Wanduhr**, nicht am Frame) |

**Kleinian war der zweite echte Befund**, aber an der Vorgabe: die Färbung läuft
über `fract(Spiegelungszahl · colorScale)`, und die Spiegelungszahl ist eine
**ganze Zahl** — jedes ganzzahlige `colorScale` ergibt exakt 0, also eine
einfarbige Scheibe. Gemessen: 1,0 und 4,0 liefern dasselbe Bild (MAE 0,0000),
erst 0,17 zeigt die Kachelung. Fünf weitere Felder (`p`, `q`, `morph`,
`iterations`, `rotation`) standen deshalb als stumm da. Vorgabe jetzt 0,17.

### Drei Wächter aus drei Fehlern

- **Zusatzfelder einer Grundkonfiguration** werden gegen das Feld-Inventar
  geprüft (`make_field_probes.py`) — getrennt nach Schadenswirkung: der *innere*
  Schlüssel landet im Preset und ist ein Fehler, der *äußere* wählt nur aus und
  wird gezählt (71 aus den Kreuzprodukten).
- **Verwaiste Sonden werden gelöscht.** Was ein Lauf nicht mehr erzeugt, muss
  weg — `hostgroup/curveIn.lvfx` lief weiter mit und stand als „stumm" im
  Report, obwohl längst als nicht prüfbar erklärt.
- **Panel-Schlüssel** gegen das Inventar (§10).

### Ein Eigentor, das hierher gehört

Meine erste Runde Tabellen-Einträge stand **vor** den alten im selben Dict — im
Python-Dict gewinnt der spätere, sie waren wirkungslos. Und die `add*`-Helfer
bekamen in §10 den Feldnamen als erstes Argument, wodurch `lo`/`hi` in der Ernte
um eine Position rutschten: der Ernter fand **269 → 0 Bereiche**, und alle
Zahlen-Gegenwerte fielen still auf die Notregel. Aufgefallen nur, weil
`text.normSpeed` partout stumm blieb.

### Die letzten 15 (es waren 16): aufgelöst (S57)

**0 stumme Sonden**, mit Vollauf belegt. Neun lagen am Messaufbau, eine ist mit
diesem Testsignal grundsätzlich nicht prüfbar, und **sechs waren Befunde an der
App** — jeder derselben Bauart wie die fünf aus S53: ein Feld steht im Panel,
lässt sich verstellen, und **kein Renderer liest es**.

Damit kippt die Bilanz der Durchsicht: von 100 durchgesehenen waren zwei ein
App-Befund, von den letzten 16 waren es sechs. Die leichten Fälle standen vorn.

**Es waren 16, nicht 15.** Der Vollauf hat `bump.durationFrames` zutage gebracht —
in S56 viermal als stumm gemessen, zuletzt in einem gezielten `bump`-Teillauf, und
in der kumulierten Bilanz „15" trotzdem nicht enthalten. Genau davor warnte der
Satz „ein Gesamtstand ist **nicht gemessen**": eine Zahl, die aus Teilläufen
addiert wird, kann einen Fall verlieren.

**Vollauf-Bilanz (S57 gegen S56, 702 bzw. 707 Sonden):**

| | S56 | S57 |
|---|---|---|
| WIRKT | 568 | **674** |
| SCHWACH | 24 | 28 |
| STUMM | 115 | **0** |

**Keine einzige Sonde ist schlechter geworden**, 109 sind besser — die vier
zusätzlichen SCHWACH sind aus STUMM aufgestiegen, nicht aus WIRKT gefallen. Die
fünf Sonden Differenz sind die als *nicht prüfbar* festgeschriebenen Felder, die
in S56 noch mitliefen (`camera3d.fogColor`, `customBpm.arbitraryMs`,
`hostgroup.curveIn`/`curveOut`, `rotatingStars.bandHi`). Der Vollauf selbst zählte
673/28/1; die letzte stumme Zeile wurde danach gezielt gelöst und nachgemessen
(`bump.durationFrames` → 0,1156), sie steht hier als WIRKT.

**Am Messaufbau (9)** — alle nachgemessen:

| Feld | Ursache | jetzt |
|---|---|---|
| `list.onBeatFrames` | Der Gegenwert war **3**, nicht 200: die Dämpfung für weite Bereiche (1..200 gegen Vorgabe 1) nimmt das Dreifache. Drei Frames sind bei einem Schlussframe fünf Frames nach dem Beat genauso abgelaufen wie einer. Dazu stand das **Kind** der Liste still — ohne bewegten Inhalt zeigt keine Fensterlänge etwas | 0,2459 |
| `bloom.post` | Wählt, WO der Glow entsteht (Present oder Kette). Beide Wege erzeugen denselben Glow aus derselben Quelle; der Unterschied lebt davon, dass der **nächste Frame** ihn sieht. Auf einem Untergrund, der jeden Frame löscht, gibt es kein nächstes Mal | 0,7997 |
| `camera3d.tz` | Zwei Dinge: der Wert stand **zweimal** in `HANDWERK` (0,6 gewann über die Korrektur 3,0 — im Python-Dict gewinnt der spätere, die S56-Merkregel), und die Kamera steht auf der z-Achse: die Blickrichtung wird normalisiert, für jedes `tz < pz` ist sie exakt (0,0,−1). Erst ein Blickziel **neben** der Achse macht die Tiefe zu einem Winkel | 0,0345 |
| `convolution.edgeMode` | Wählt, was der Kern jenseits des Bildrandes liest. Der Untergrund ist am Rand überall 0x101010 — bei einfarbigem Rand liefern Festklemmen und Umlaufen dasselbe, und zwar exakt | 0,0014 |
| `dotPlane.colors` | Die Tafel hat **fünf** Stützstellen, der Gegenwert setzte zwei — und die zweite traf die Vorgabe. Es änderte sich genau eine von fünf | 0,0368 |
| `multiDelay.mode` | Der Gegenwert 2 machte Prüfling **und** Nachfolger zu Lesern des geteilten Rings. Niemand schrieb, und auf einem leeren Ring sind „aus" und „auslesen" derselbe No-op | 0,0094 |
| `setRenderMode.adjustAlpha` | „Adjustable" ist beim Set Render Mode **7**; die Grundkonfiguration setzte 10, und `runSetRenderMode` klemmt auf 0..9 — aus dem Raten wurde still 9 = Minimum. Die Nummer ist je Knoten eine andere (`bufferSave.blend` 10, `colorMap.blendMode` 9), das steht jetzt am Tabelleneintrag | 0,0031 |
| `terrain3d.colorLow` | Die Palette läuft über die Höhe. Mit der Vorgabe `ringAmp = 1` schiebt das Spektrum das ganze Gelände nach oben — im Bild ist nur die Gipfelfarbe | 0,0580 |
| `bump.durationFrames` | Auf einem Beat setzt Bump die Tiefe auf `depth2` — **unabhängig** von der Rampenlänge. Am Schlussframe, der selbst ein Beat ist, steht deshalb in beiden Presets dieselbe Tiefe. Eigene Lauflänge 201 (20 Frames nach dem letzten Beat): dort ist die Vorgabe 15 abgelaufen, der Gegenwert 100 hält noch — bei ihm ist der Rampenschritt `\|30−100\|/100` als Integer-Division sogar 0, die Tiefe fällt gar nicht (der S46-Sonderfall) | 0,1156 |

**Nicht prüfbar, festgeschrieben (1):** `rotatingStars.bandHi` — der Knoten nimmt
aus dem Fenster nur die **Spitze**, und das Spektrum des Standalone-Signals fällt
ab jeder Stelle monoton: das Maximum liegt immer im ersten Band, unabhängig
davon, wo das Fenster endet. Mit sechs Fenstern gemessen ([0,·), [3,·), [4,·),
[12,·), [40,·), [120,·)), jedes schmalste gab Pixel für Pixel dasselbe Bild wie
das weite. Gegenprobe: `bandLo` kommt an — fünf einzelne Bänder, fünf
verschiedene Bilder. Damit sind es **drei** nicht prüfbare Felder, zusammen mit
`customBpm.arbitraryMs` (hängt an der **Wanduhr**: 181 Frames rendern in
Millisekunden, weder 500 ms noch 5000 ms lösen aus) und `camera3d.fogColor`
(**dämpft** Sprites nur, statt sie zu färben — unser Zeuge ist ein SuperScope 3D,
also ausschließlich Sprites).

**Sechs App-Befunde, alle behoben (S57)** — Messwerte der Sonde nach dem Fix:

| Feld | Befund | Fix | jetzt |
|---|---|---|---|
| `blur.roundUp` | AVS rechnet den Blur in **8-Bit-Ganzzahlen** und schneidet jeden Teilterm ab (`DIV_2`/`DIV_4`/`DIV_8`/`DIV_16` sind Byte-Shifts); „round mode" legt je Kernel einen festen Ausgleich obendrauf. Unser Shader rechnete in float mit exakter Gewichtssumme — kein Verlust, kein Ausgleich, das Feld nirgends gelesen. Dazu stand unsere **Vorgabe auf `true`**, AVS' Default ist 0 | Byte-Arithmetik im Shader, Ausgleich **+4/+5/+3** je Stärke (r_blur.cpp), Vorgabe auf `false`. Der Mittelterm braucht zwei Gewichte: `DIV_2 + DIV_4` sind zwei getrennt abgeschnittene Terme, nicht einer mit 0,75 | 0,0118 = 3/255, die Arithmetik trifft exakt |
| `grain.staticGrain` | Wir zeichneten **immer** statisch (feste Rauschtextur), die Vorgabe `false` versprach das Gegenteil. Der Code sagte es selbst: „nur der NICHT-statische Pfad braucht die Tabelle" | Zweiter Shader-Zweig, je Frame frische Werte über das `randAt`-Muster von Scatter. **Nicht** bitgleich zur Referenz: dort läuft eine Position sequentiell durch eine 491-Byte-Tabelle und der Faktor wird nur gezogen, wenn die Schwelle trifft — datenabhängig, parallel nicht berechenbar. Gleich sind Verteilung, Wertebereich, Frame-Frische | 0,0363 |
| `oscRing.channel` · `oscStar.channel` | Beide riefen `getWaveform()`/`getSpectrum()` **ohne Kanal**. Die S56-Vermutung „das Signal ist nicht stereo" war falsch — die Wellenform des Standalone hat längst L≠R (Phasenversatz 0,7), und `simpleScope.channel` wirkt, weil dieser Knoten `visWaveform(channel)` liest | `waveOfChannel`/`specOfChannel` neben den bestehenden Accessoren, eine Stelle für beide Knoten. **Bildneutral** für vorhandene Presets: `getWaveform()` mischt bei Stereo selbst zur Mitte, und die Vorgabe ist `channel = 2` = Mitte | 0,0013 · 0,0118 |
| `texer.blend` | Der Renderer kannte nur „0" und „alles andere": **1 und 2 waren derselbe GL-Zustand**, 50/50 gab es nicht, und 0 („ersetzen") war additiv ohne Alpha-Gewichtung | Alle drei Betriebsarten mit den Faktoren aus `applyLineBlend`, damit ein Sprite wie eine Linie mischt | 0,0655 |
| `texerII.wrapAround` | Nirgends gelesen | Torus-Wiederholung: die Kopien 2,0 entfernt, gezeichnet nur wo sie den Sichtbereich schneiden. Referenz ist eine Binär-APE (`texer2.ape`, kein Quellcode im ref-Baum) — umgesetzt ist die Semantik, die am Feld steht | 0,0278 |

**~~Beobachtung~~ ✅ gelöst (S57):** `fractalZoomer.feedback` heißt „trail
persistence 0..1", der Shader las es aber nur als **Schalter**
(`> 0.01f ? 50/50 : ersetzen`) — 0,3 und 1,0 ergaben dasselbe Bild. Jetzt ein
echtes Gewicht (`mix(col, alt, feedback)`) über ein eigenes Uniform, damit der
Nachbar-Knoten `fractal2D` seine **benannte** Betriebsart „50/50" behält. Der
Test, der vorher 0,0000 gab (0,3 gegen 1,0), liefert **0,2391**; die Vorgabe 0,5
ist exakt das alte 50/50, bestehende Presets bleiben also unverändert.

### Die Matrix hatte keinen Blur mit Trail (S57)

Der Grund, warum `blur.roundUp` zwei Kalibrier-Runden überlebt hat: die einzige
Blur-Zeile lief auf **statischem** Material, das jeden Frame neu gezeichnet wird.
Eine Rundung um 4/255 ist dort eine Stelle hinter der Anzeige — nachgemessen
liefern `roundUp` an und aus in `01_normal` beide **MAE 0,003**. Erst ohne Basis
(`MAT_TRAIL`) akkumuliert sie sichtbar.

Zwei neue Zeilen bewachen jetzt beide Richtungen, und sie belegen den Fix:

| Zeile | 320×240 dMean/dMaxLuma/MAE | Urteil |
|---|---|---|
| `06_blur/02_trail_rounddown` | 0,007 / 0,016 / 0,009 | OK |
| `06_blur/03_trail_roundup` | 0,005 / **0,001** / 0,012 | OK |

Der harte Beleg, dass der Fix nötig war: die beiden **Referenzbilder**
unterscheiden sich in **230 400 von 307 200 Bytes** bei maximaler Abweichung 255
— zwei völlig verschiedene Bilder. Vor dem Fix hätte unser Renderer für beide
Presets dasselbe geliefert (das Feld wurde nirgends gelesen), also wäre
mindestens eine der Zeilen krachend falsch gewesen. Jetzt sind beide grün.

### Zwei Werkzeug-Löcher, die dabei aufgefallen sind

- **Ein Faltungskern ist keine Farbtafel.** Die Listen-Regel füllte alle 49
  Stellen von `convolution.kernel` mit Palettenfarben — Gewichte um 16 Millionen,
  das Bild übersteuert vollständig, und die Sonde meldete mit MAE **0,8906**
  „wirkt", ohne etwas gemessen zu haben. Jetzt Laplace aus `HANDWERK`: **0,0993**.
- **Weiß gehört nicht in die Gegen-Palette.** `0xFFFFFF` ist der Ersatzwert, den
  mehrere Renderer für eine LEERE Tafel einsetzen (`cycleScopeColor`,
  `paletteRgb`) — eine Tafel mit Weiß darin kann dort genau das Vorgabebild
  treffen. In der ersten Fassung meiner neuen Tafel-Regel fiel
  `superScope.colors` deshalb von WIRKT auf **STUMM**, drei weitere wurden
  schwächer. Ohne Weiß und Graustufen wirken alle dreizehn Tafeln, mehrere
  deutlich stärker als vorher (`oscStar.colors` 0,0009 → 0,0099).

## 1d. Vorgaben: die zweite Quelle ist abgeschafft (S56)

Jede Vorgabe stand an **zwei** Stellen: als Initialisierer im `…Params`-Struct
und als dritter Parameter im Deserialisierer (`getInt(o, "x", 5)`). Liefen sie
auseinander, hing der Wert davon ab, **woher der Knoten kam** — ein frisch
eingefügter trug den Struct-Wert, ein **geladener** den des Deserialisierers.

Aufgefallen am Kleinian-Fix (§1c): Struct-Vorgabe geändert, gebaut — die Sonden
sahen nichts, weil sie aus einem Preset laden.

**Entscheid Patrik: die zweite Quelle abschaffen.** Umgesetzt — der
Deserialisierer bezieht die Vorgabe jetzt aus dem Ziel selbst:

```cpp
p.iterations = getInt(o, "iterations", p.iterations);   // vorher: …, 30);
```

Das Params-Objekt wird ohnehin auf Vorgabe konstruiert, also steht die Zahl nur
noch im Struct. **415 Stellen** maschinell umgestellt, dazu **243 `getStr`**,
die gar keinen Vorgabewert kannten (neue Überladung mit Default). Vier
`BlendMode`-Ziele brauchten einen Cast von Hand.

**Eine bewusste Ausnahme bleibt:** `starfield.blend` liest `1`, obwohl die
Vorgabe `0` ist — *„legacy files rendered additively"*. Der Wert steht dort für
**alte Dateien**, nicht für den Knoten. Sie trägt ihre Begründung im Code und
ist im Wächter benannt.

**Zwei Vorbedingungen**, damit der Umbau bild-neutral bleibt:

- **Demo-Skripte raus aus vier Structs** (`dynamicShift`, `bump`,
  `dynamicDistanceModifier`, `texerII.pointCode`). Der Deserialisierer ließ
  diese Slots beim Laden schon immer leer; wären sie im Struct geblieben, hätte
  ein importiertes Preset sie ab jetzt **geerbt**. Ihr Platz ist eine
  Voreinstellung (§11) — die Texte stehen in der Git-Historie.
- **`metaballs3d.colors`/`tentacles3d.colors`** von `{}` auf `{0xFFFFFF}`: der
  Leser heilte eine leere Tafel ohnehin auf Weiß, eine leere ließ sich also gar
  nicht speichern. Gefunden vom neuen Roundtrip-Test.

**Eine gewollte Verhaltensänderung:** `timescope.blend` wird beim Laden nicht
mehr auf `0` gezwungen, sondern erbt die Struct-Vorgabe `3` (= folge dem Set
Render Mode, der AVS-Default laut `r_timescope.cpp:147-148`). Betroffen sind nur
Presets, die das Feld gar nicht nennen.

### Zwei Wächter statt eines

| | prüft | wo |
|---|---|---|
| **statisch** | steht im Deserialisierer noch ein Vorgabe-**Literal**? | `harvest_field_docs.py` — „Vorgabe-Literale im Leser: 0" |
| **Roundtrip** | übersteht jedes Feld Struct → JSON → Struct? | `test_FieldInventory.cpp` |

### Korrektur an meiner eigenen Meldung

Der erste Wächter-Entwurf fütterte `nodeFromJson` mit `{"type": …}`, also einem
JSON **ohne jedes Feld**, und meldete **20 Abweichungen**. Diese Zahl war falsch:
neun davon waren **Alt-Format-Migrationen**, die genau auf einem leeren Objekt
anspringen und dann aus fehlenden Werten rechnen (`blitterFeedback.scale2` kam so
auf **−102** — im Code steht dort `30`, wie im Struct).

Echt waren: **2** abweichende Literale (`starfield.blend` bewusst,
`timescope.blend` nicht) und **9** Skript-Felder, bei denen `getStr` keinen
Vorgabewert kannte. Ermittelt wurde das statisch, nicht zur Laufzeit — dieselbe
Lehre wie bei den Sonden: *eine gemeldete Abweichung ist zuerst eine Frage an das
Messverfahren.*

## 1e. Bedienung: der Editor muss nach jedem Baumumbau dastehen

**Befund Patrik (S55, S56).** Wer einen Knoten einfügt oder verschiebt, will ihn
sofort bearbeiten. Beides ging nicht: der Eigenschaften-Editor blieb leer, bis
man einmal weg- und wieder hinklickte.

Die Ursache ist an beiden Stellen dieselbe. `selectByPath`/`selectPaths` setzen
die Auswahl unter `m_selecting`, und das Auswahl-Signal ist dabei **stillgelegt**
(sonst würde jeder Zwischenschritt einen Editor-Neubau auslösen). Das
anschliessende `setCurrentItem` ändert nur noch das *aktuelle* Element — es
ändert die Auswahl nicht mehr und löst deshalb kein Signal aus. Also baut
niemand den Editor.

Der Doppelschritt ist die Lösung: **markieren UND `buildPropertyEditor` rufen.**
Einfügen hat ihn seit S55, Verschieben seit S56.

⬜ **Beides ist nicht erprobt** — das geht nur durch Anklicken bzw. Ziehen.

## 1f. Strang F abgearbeitet: wirkt jedes Feld auch beim EDITIEREN? (S57)

Der Rückstand aus S56 ist gemessen — ein Vollauf über alle 702 Sonden, nachdem
S57 sechs Renderer und mehrere Sonden-Aufbauten verändert hatte.

**558 GLEICH · 133 TEILWEISE · 11 VERDECKT · 0 WIRKUNGSLOS.**

Der Lauf fand **einen** Befund, und der ist behoben:
`interferences.rotation` ist ein **Startwert** für einen selbstlaufenden Zähler
(`rt.interfRotation += rotInc` je Frame). Übernommen wurde er nur unter
`!interfSeeded` — also **einmal beim Aufbau**. Da ein Panel-Edit nur
`recompileChain()` ruft und nicht `resetRuntimes()`, blieb der Zähler für immer
auf dem Wert des ersten Aufbaus: ein neu eingestellter Startwinkel kam nie an.

```cpp
if (!rt.interfSeeded || rt.interfRotationSeed != params.rotation)   // vorher: nur !interfSeeded
```

Verglichen wird der **Preset-Wert**, nicht die Frame-Kopie — sonst würde ein
Skript, das `rotation` je Frame schreibt, den Zähler in jedem Frame zurücksetzen.
Danach `GLEICH 0,0000`, und die Feld-Sonde liegt unverändert bei 0,0548 (keine
Regression beim Laden).

Das ist eine eigene Fehlerklasse, die vorher keinen Namen hatte: **ein Startwert
ist kein Parameter.** Wo ein Feld nur einen fortlaufenden Zustand initialisiert,
macht ein `…Seeded`-Flag den Wert nach dem ersten Aufbau unerreichbar. Strang F
hat alle 702 Felder daraufhin geprüft und genau diesen einen Fall gefunden.

Die Verschiebung gegenüber S56 (587/112/8 → 558/133/11) kommt von den Sonden, die
S57 verändert hat: mehr Bewegung im Bild heißt mehr Vorgeschichte, also wandern
Felder von GLEICH nach TEILWEISE. `TEILWEISE` ist bei Effekten mit Verlauf der
Normalfall und kein Befund.

## 2. Urteile, die nur Seite-an-Seite fallen können

Prüfplan, Presets, Audio und Kriterien stehen in
[AVS_Sichttest_Protokoll.md §7](visuals/AVS_Sichttest_Protokoll.md). Angelegt in S45,
**keine Zeile ausgefüllt**. Test-Audio: `…\cmake\TestAudio\` (WAV = Master, MP3 für
Winamp-Komfort).

| # | Frage | Ohne Antwort passiert | |
|---|---|---|---|
| P1 | **S7** — konvergiert echtes AVS bei XOR-in/50-50-out-Listen auch zu Uniform-Grau? | unklar, ob überhaupt ein Bug vorliegt | ⬜ |
| P2 | **S13** — zeichnet AVS im nicht-quadratischen Fenster eine Ellipse, wo wir einen Kreis zeichnen? | Scope-Mapping bleibt aspektquadratisch | ⬜ |
| P3 | **S9-Rest** — sättigt das Original bei ZeroG/Novae/Rotor auch zu Weiß/Gelb? | ColorMap-auf-Weiß / FastBright-Ketten unbisektiert | ⬜ |
| P4 | **S12** — Spektrum-Amplitudenskala (`kSpecGain`) | Sichtkalibrierung offen | ⬜ |
| P5 | **S1** — fuzzify/blocky-Optik (Körnung, Blockraster) | S1-Rest nicht geschlossen | ⬜ |
| P6 | **Ego** — Subtract-Helligkeitsbalance | Schwarz ist gefixt (S14), Balance ungeprüft | ⬜ |
| P7 | Kür: Blend-Modi + L/R-Kanalvertrag | — | ⬜ |

**Nicht nachgemessen seit S45** (ein Teil dürfte durch S48–S52 erledigt sein):

- **HISTORY-pRELOADED, 12 Befunde** ([Protokoll §5](visuals/AVS_Sichttest_Protokoll.md)):
  HpR05 fast nur schwarz · HpR10 Kontrast · HpR11/HpR12 zu dunkel · HpR14 wird in <1 s
  weiß · HpRX2 „da fehlt was" · HpRX3/HpRX4/HpRX6 nur schwarz · HpRX5 wird zu hell ·
  HpRX7 Matrix-Text-Dichte.
- **JC-big stuff, 3 Weiß-Sättigungs-Verdachtsfälle** (§2.1): don't make a mess ·
  don't try to aphect ME · how much 4 the cool glowin thingi — Bisektion ausstehend.

## 3. MilkDrop

### ✅ MilkDrop-Presets stocken manchmal (Beobachtung Patrik S105 — S107: Ursache außerhalb der App)

**Beobachtung Patrik (2026-10-09, im Programm, Release-clang, Komposition
`asset/composed2b.lvcomp`):** manche MilkDrop-Presets ruckeln zeitweise und
laufen dann plötzlich wieder flüssiger. Stärkstes Beispiel
`asset\Milkdrop3\presets\martin - glassworks 3.milk` — dort fällt die Bildrate
auf etwa 35 fps, **aber nur manchmal**; gesehen an beiden Stellen, an denen das
Preset in der Komposition liegt (CH1 5:15–6:54 und CH2 11:36–15:40), um etwa
5:30 und um etwa 14:40, „nicht so eng mit der Zeit". AVS hat er dabei nicht
betrachtet.

**Nichts gemessen, nichts untersucht.** Auftrag Patrik: später, in einer
eigenen Sitzung nach dem Sichttest-Helper. Dann zuerst das Log und eine
Messung, keine Ursache annehmen — offen ist, ob das Preset allein stockt
(Kette ohne Composer), ob das Mischen zweier Kanäle oder der Anlauf eines
Nachbar-Clips der Anlass ist, und auf welcher Grafikkarte die Exe lief.

**Zweite Beobachtung, mit Zahlen aus dem Log (2026-10-09 gegen 23:52, beim
Aufnehmen der Bilder für die README):** LumiViz (Release-clang) wurde zehnmal
gestartet, je an einer anderen Stelle von `asset/composed2.lvcomp` (über Resume
after restart), und lief je rund 20 s. Patrik sah das Stocken in **drei** der zehn
Läufe — und genau diese drei melden im `LumiViz.log` neben der Exe viele lange
Bilder (`RenderThread: render … ms`), gezählt ab 6 s nach dem Start, also nach
dem Anlauf:

| Stelle | Clips an der Stelle | lange Bilder nach 6 s | längstes |
|---|---|---|---|
| 6:00 | *martin - glassworks 3* (CH1) | **25** | 179 ms |
| 13:30 | *martin - glassworks 3* (CH2) | **33** | 139 ms |
| 19:30 | *martin - double chase* (CH3) + *martin - Gin Tonic on Ice 004* (CH4) | **25** | 434 ms |
| die sieben übrigen (1:00, 2:50, 4:30, 7:20, 8:45, 11:00, 16:30) | verschiedene | je 1 bis 2 | höchstens 142 ms |

Belegt ist damit nur: das Stocken ist im Log zählbar, es hängt an der Stelle der
Komposition und kam bei *glassworks 3* in beiden Kanälen. **Nicht** belegt: die
Ursache; ob es auch ohne Composer und ohne Karaoke-Ebene auftritt; ob es über die
Laufzeit anhält (die Läufe waren kurz und begannen den Clip frisch).

**Dasselbe Log genauer gelesen (S106, 2026-10-10 — nur gelesen, nichts gebaut,
nichts gestartet):** alle zehn Läufe liefen auf der RTX 4090 bei 1836 × 909. Die
Zeiten stammen aus der Zusammenfassung des Composers am Ende jedes Laufs
(`Composer: summary` samt Folgezeilen); ihre Liste der langen Bilder bricht nach
rund zwanzig Einträgen ab, die Anzahl davor ist vollständig.

| Stelle | Bilder je Sekunde über den Lauf | Mittel `compute (CPU side)` | Einbruch beginnt | Renderer zugleich |
|---|---|---|---|---|
| 6:00 | 50,5 | 16,6 ms | 8,6 s nach der Wiederaufnahme | 1 |
| 13:30 | 46,6 | 19,5 ms | 7,8 s | 1 |
| 19:30 | 50,5 | 9,5 ms | 7,8 s | 2 |
| die sieben übrigen | 58,5 bis 59,1 | 2,7 bis 5,3 ms | keiner | 1 oder 2 |

- **Form:** die langen Bilder folgen lückenlos aufeinander, je 100 bis 139 ms
  (am Rand 52 bis 99; einzelne 179 und 434 ms), über rund 2 bis 4 s; danach läuft
  es von selbst wieder mit 60. Die Zeile `FPS (render thread)` meldet für das
  5-s-Fenster des Einbruchs 7 bis 10. Es ist also ein kurzer Einbruch auf etwa
  8 Bilder je Sekunde, kein Dauerlauf mit 35.
- **Wo die Zeit liegt:** im `render()` der Bildquelle (`compute (CPU side)`);
  `present` kostet im Mittel 0,01 ms.
- **Nicht das Mischen zweier Kanäle, nicht der Anlauf eines Nachbar-Clips:** in
  beiden Läufen mit *glassworks 3* lief höchstens ein Renderer, und während des
  Einbruchs steht keine andere Zeile im Log (auch keine `start-up`-Zeile).
- **Zweites Muster:** in den acht Läufen ohne *glassworks 3* steht genau ein
  langes Bild 5,1 bis 5,4 s nach der Wiederaufnahme (113 bis 175 ms); in den
  zwei Läufen mit *glassworks 3* fehlt es. Der feste Abstand ist beobachtet,
  nicht erklärt.
- **Im Preset nachgerechnet, erklärt es nicht:** *glassworks 3* hat in
  `per_frame` Schleifen, deren Schrittzahl von `1/fps` abhängt (`EC_Steps`);
  bei 8 Bildern je Sekunde sind das höchstens rund 500 Durchläufe je Bild.

**Weiter nicht belegt:** die Ursache; ob es ohne Composer auftritt; welche Stufe
des MilkDrop-Renderers die Zeit verbraucht. Die erste Beobachtung (um 5:30 und
um 14:40) lag 15 s und rund 3 min nach Clip-Beginn — es hängt also nicht nur am
Anlauf. **Vorgeschlagene Reihenfolge:** (1) *glassworks 3* allein in der Effect
Chain, Composer aus, etwa 60 s, dann das Log lesen (ohne Code); (2) eine
Diagnosezeile je langem Bild im MilkDrop-Renderer, nach Stufen aufgeteilt, samt
dem `fps`-Wert, den das Preset sieht; (3) das einzelne lange Bild bei 5 s
getrennt davon.

**Zwei Läufe von Patrik (S106, 2026-10-10, 13:12 und 13:20; Release-clang,
RTX 4090, 1836 × 909, mit Ton) — kein Einbruch, auch nicht gesehen:**

| Lauf | Was lief | Dauer | `FPS (render thread)` | lange Bilder nach dem Anlauf |
|---|---|---|---|---|
| 13:12 | Composer an ab 6:36 in `composed2.lvcomp`: *glassworks 3* (CH1) bis 7:39, daneben *juggernaut brighter* (CH2) | 83 s | 59 bis 61 | eines, 57 ms am Clip-Wechsel 7:39 |
| 13:20 | Composer nach 3 s aus, danach die Kette der App (beim Start war *glassworks 3* als letztes Preset wiederhergestellt, ein Wechsel steht nicht im Log) | rund 140 s | 59 bis 62 | zwei, 51 und 67 ms |

Das einzelne lange Bild rund 5 s nach der Wiederaufnahme fehlt in beiden.

**Acht Kurzläufe unter Last (S106, 13:36 bis 13:41; von Claude gefahren):** wie
am 09.10. ein frischer Start über Resume bei 6:00 in `composed2.lvcomp`, je rund
20 s, dazu teils Last von außen — CPU: 16 oder 32 rechnende Prozesse (der
Rechner hat 32 logische Kerne); GPU: zwei `MilkdropStandalone` in 3840 × 2160
(einer allein lastete die 4090 in der Probe zu rund 50 % aus). Die Auslastung
der CPU während der Läufe ist **nicht** gemessen (die Abfrage lieferte 0).

| Lauf | Last | Bilder je Sekunde | Mittel `compute` | Bild |
|---|---|---|---|---|
| 13:36 | keine | 58,8 | 12,8 ms | kein Einbruch |
| 13:38 | keine | 51,0 | 18,0 ms | **Einbruch** 6:08,6 bis 6:09,4 und 6:10,9 bis 6:11,5 (15 Bilder, 50 bis 239 ms) |
| 13:40 | keine | 43,8 | 20,7 ms | **Einbruch** ab 6:07,0 und noch einmal ab etwa 6:17 (44 Bilder, 52 bis 145 ms) |
| 13:37 | CPU 16 | 29,5 | 32,7 ms | gleichmäßig langsam (28 bis 30), kein Einbruch |
| 13:38, 13:39 | CPU 32 | 0,3 | über 3 s | steht praktisch: sechs Bilder in 20 s |
| 13:37 | GPU 2 | 24,9 | 39,1 ms | gleichmäßig langsam (23 bis 37) |
| 13:39 | GPU 2 | 17,1 | 57,1 ms | gleichmäßig langsam (18 bis 19), 120 Bilder zu 51 bis 105 ms |

- **Der Einbruch kommt ohne jede Last von außen** — in zwei von drei Läufen an
  derselben Stelle wie am 09.10. (dort ab 6:08,7), im dritten nicht.
- **Last von außen sieht anders aus:** sie bremst gleichmäßig über den ganzen
  Lauf, statt für 1 bis 4 s einzubrechen und sich zu erholen.
- **Das Preset ist für sich schon teuer:** ohne Einbruch 12,8 ms je Bild im
  Mittel (die anderen Presets am 09.10.: 2,7 bis 5,3 ms) — bei 60 Bildern je
  Sekunde bleiben 16,7 ms.
- **Offen, nicht geprüft:** Patriks Lauf von 13:12 begann erst bei 6:36, also
  **hinter** den Stellen 6:07 bis 6:20 — ob der Einbruch an der Stelle der
  Musik hängt statt am Abstand zum Start, trennen diese Läufe nicht. Das Preset
  würfelt beim Start (`per_frame_init`: `rand` für `nstab`, `r1`, `sax1` …);
  das könnte erklären, warum derselbe Lauf einmal einbricht und einmal nicht —
  **Vermutung, nicht gemessen.**

**Sechs weitere Kurzläufe ohne Last (S106, 13:45 bis 13:49) — kein Einbruch:**
drei mit Start bei 6:00 (je rund 20 s) und drei mit Start bei 5:50 (je rund
30 s, bis 6:19), gedacht als Trennung „Stelle der Musik" gegen „Abstand zum
Start". Alle sechs liefen mit 57,6 bis 59,0 Bildern je Sekunde, Mittel
`compute` 13,3 bis 14,9 ms. Lange Bilder nach dem Anlauf: in vier Läufen
keines, in zweien je drei bis vier zu 52 bis 91 ms (bei 6:02,6 und bei 6:12,7
bis 6:13,0). Die Trennung ist damit **nicht** gelungen: die Stellen 6:07 bis
6:11 wurden sechsmal ohne Einbruch durchlaufen, der Abstand von rund 8 s zum
Start ebenfalls. Über den Tag: Einbruch in 2 von 9 Kurzläufen ohne Last; am
09.10. in 2 von 2 an dieser Stelle. Beobachtet, nicht geprüft: die zwei Läufe
mit Einbruch von heute folgten unmittelbar auf einen Lauf mit Last (GPU 2,
CPU 32). **Ohne Messung im Renderer selbst ist hier nichts mehr zu holen** —
weiter mit Schritt (2) der Reihenfolge oben.

**Messung im Renderer (S106, 13:52 bis 14:10) — der Rechner bremst, nicht die
App:** `MilkdropVisualizer::onRender` schreibt für jedes Bild über der Schwelle
(50 ms; `LUMIVIZ_MILKDROP_LANG_MS=<ms>` setzt sie) **eine** Zeile `langesBild`
in die Trace-Datei `<TEMP>/lumiviz_milkdrop_trace.log`: Dauer, `dt` und `fps`
wie das Preset sie sieht, Bildgröße, die Anteile der dreizehn Stufen, für die
Shapes Skript gegen Zeichnen samt bedeckter Fläche, und für den Thread die
verbrauchten Prozessorzyklen und den Kern. Damit an Kurzläufen wie oben
(Start bei 6:00, *glassworks 3*, 1836 × 909, Schwelle 14 ms):

| | normales Bild | Bild im Einbruch |
|---|---|---|
| Dauer | 14 bis 17 ms | 100 bis 155 ms |
| Stufe shapes (1350 Instanzen) | 11 bis 14 ms | 87 bis 128 ms |
| davon Skript / Zeichnen | 4 bis 6 / 7 bis 10 ms | 36 bis 54 / 49 bis 73 ms |
| `per_frame`, Mesh, Waves | 0,5 bis 1,7 ms | 3,6 bis 15,6 ms |
| bedeckte Fläche der Shapes | 0,9 bis 5 Bildschirme | 0,9 bis 1,3 Bildschirme |
| Zyklen des Threads je Millisekunde | 2,41 bis 2,46 Mio | 2,33 bis 2,42 Mio |

- **Alle Stufen werden im selben Verhältnis langsamer** (rund sieben- bis
  achtfach), Lua-Skript und GL-Aufrufe gleichermaßen; die Arbeit je Bild ist
  dieselbe (gleiche Instanzen, gleiche Fläche). Füllrate ist es nicht: das
  zweite Bild eines Laufs bedeckte 307 Bildschirme und brauchte 15 ms.
- **Der Thread rechnet durch und wartet nicht:** die Zyklen je Millisekunde
  sind im Einbruch dieselben. Er braucht für dieselbe Arbeit die sieben- bis
  achtfache Zeit am Prozessor.
- **Gegenprobe außerhalb der App:** ein eigener Prozess, der neben LumiViz alle
  20 ms dasselbe kleine Stück rechnet (Median 5,0 ms), wurde in **genau
  denselben Sekunden** 7- bis 12-mal langsamer — viermal in 62 s, je rund 3 s
  lang, im Abstand von 10 bis 11 s (14:09:34–37, 14:09:44–47, 14:10:05–08,
  14:10:16–19); die langen Bilder von LumiViz liegen in denselben Sekunden.
  Zwischen den zwei Läufen, ohne LumiViz, blieb die Probe 17 s lang ruhig.
- **Häufigkeit über den Tag** (Kurzläufe ohne Last, in der Reihenfolge der
  Serien): 2 von 3, 0 von 6, 2 von 4, 1 von 4, 0 von 5, 4 von 5, 2 von 2 — kein
  klarer Verlauf, es kommt in Schüben.
- **Das „35 fps" der ersten Beobachtung** ist der geglättete `fps`-Wert, der im
  Einbruch von 55 auf 24 bis 31 fällt; das einzelne Bild dauert dabei rund 110 ms.

**Schluss:** der Einbruch ist eine Drosselung des Prozessors für rund 3 s, die
auch einen fremden Prozess trifft; in der Gegenprobe kam sie nur, während
LumiViz lief (eine Probe, 62 s). *glassworks 3*
macht sie sichtbar, weil es mit 13 bis 15 ms je Bild fast das ganze Budget
braucht; ein Preset mit 3 ms bliebe auch achtfach gebremst unter 50 ms.
**Nicht belegt:** wer drosselt und warum (Temperatur, Leistungsgrenze der
Firmware, Energiesparplan „Turbo"). Das Ereignisprotokoll nennt keine
Drosselung durch die Firmware (Kernel-Processor-Power, nur Kennung 55); es
nennt als unterste Drosselstufe 17 % der Nennfrequenz, was zur Größe des
Einbruchs passt — ein Indiz, kein Beleg. Temperatur und Takt wurden nicht
gemessen (der Zähler dafür lieferte nichts).

**Abschluss S107 — Ursache außerhalb der App:** es drosselt der
Temperaturschutz des Prozessors am Entwicklungsrechner. Tctl steht an
100 °C, danach fällt der Kerntakt für 1 bis 3 s auf rund ein Zehntel, alle
10 bis 11 s (Temperatur und Takt je Sekunde aus dem Libre Hardware Monitor,
daneben eine Taktprobe). In einem Lauf mit *glassworks 3* und zwei
rechnenden Nebenprozessen (60 s) lagen sechs Einbrüche und 108 Bilder über
40 ms in denselben Sekunden; ein Bild dauert darin 90 bis 96 ms statt 10 bis
11 ms. **An der App ist dafür nichts zu tun.** Für künftige
Leistungsbefunde: zuerst Temperatur und Takt des Rechners prüfen; die
Trace-Zeile `langesBild` zeigt die Sekunden. Offen bleibt aus diesem Punkt
nur Stufe 3 (siehe unten).

**Shapes billiger, Stufe 1 und 2 (S106, Entscheid Patrik) — gebaut:** die
Stufe shapes vorher im Einzelnen (Mittel über rund 980 normale Bilder, zwei
Läufe): Eingabe der 51 Variablen je Instanz 1,5 ms, Lauf des Skripts samt
Abgleich der q- und reg-Werte 2,4 ms, Zurücklesen 0,5 ms, Fächer bauen 0,5 ms,
1350 einzelne Zeichenaufrufe 3,2 ms, 500 Umrisse 2,9 ms.
- **Stufe 1:** `drawCustomShapes` sammelt aufeinanderfolgende Instanzen ohne
  Textur und ohne Rand zu einer Dreiecksliste und zeichnet sie in einem Aufruf.
  Eine Instanz mit Textur oder Rand, oder ein Wechsel der Mischart, zeichnet
  zuerst das Gesammelte — die Reihenfolge bleibt.
- **Stufe 2:** die Farbe des Rands wird einmal je Instanz gelesen statt je
  Eckpunkt; die Puffer für Fächer, Ring und Umriss entstehen einmal je Bild.
  Die Umrisse laufen **weiter** einzeln über den Scope-Renderer — ihn zu
  umgehen hätte die Pixelgleichheit gefährdet, das ist nicht gebaut.
- **Vergleich alt gegen neu:** 16 Presets (Flächen, Ränder, Texturen, bis 3150
  Instanzen), je 241 Bilder in 960 × 540 mit `MilkdropStandalone --ab`: alle
  3856 Bild-Hashes gleich; der alte Stand zweimal gefahren, ebenfalls gleich.
- **Wirkung an *glassworks 3*** (Kurzläufe wie oben): Zeichnen 3,4 bis 4,4 ms
  statt 6,6 ms, ein Bild 10,0 bis 11,4 ms statt 13,6 ms. Das Skript unverändert
  rund 4,3 ms. Die Schätzung für Stufe 2 (1 bis 1,5 ms) ist nicht erreicht,
  weil die Umrisse einzeln bleiben.
- **Das Stocken behebt es nicht:** gedrosselt dauert ein Bild weiter über 50 ms.
- **Zurückgestellt (Entscheid Patrik S106): Stufe 3** — die Übergabe ans Skript
  über vorbereitete Schlüssel statt über Namen (`LuaScriptEngine`, träfe alle
  Visualizer); geschätzt 1 bis 1,5 ms an diesem Preset. Erst messen, ob es an
  weiteren Presets ins Gewicht fällt.
- **Zwei Kanäle gemessen (S106, 14:40 bis 14:45, neuer Stand):** eine
  Mess-Komposition mit *glassworks 3* 0:00–3:00 in CH1 und CH2 und einem Song
  (im Scratchpad der Sitzung, nicht im Repo), Start bei 0:05, je rund 30 s,
  Release-clang, RTX 4090, ohne Last.

  | Kanäle | Läufe | Bilder je Sekunde | Mittel `compute` |
  |---|---|---|---|
  | 1 | 2 | 59,0 und 59,4 | 10,7 und 11,2 ms |
  | 2 | 5 | 45,4 · 33,5 · 40,9 · 42,3 · 41,8 | 21,5 · 29,1 · 23,9 · 23,1 · 23,4 ms |

  Je Instanz im Betrieb mit zwei Kanälen 10,0 bis 10,4 ms (Skript 4,1 bis 4,2,
  Zeichnen 3,7 bis 4,0); die übrigen rund 3 ms je Bild sind das Mischen und der
  Rest der Bildquelle. **Zwei solche Presets halten also auch nach dem Umbau
  keine 60 Bilder je Sekunde**, sie laufen mit rund 41 bis 45. Der Lauf mit
  33,5 lief über 20 s gleichmäßig mit 35 bis 36 und hatte zusätzlich einen
  Einbruch; warum er langsamer war, ist nicht untersucht. Vor dem Umbau ist der
  Fall **nicht** gemessen (gerechnet rund 30 ms, also etwa 33 Bilder je
  Sekunde). Für 60 müsste eine Instanz unter rund 6,8 ms kommen — Stufe 3
  allein (geschätzt 1 bis 1,5 ms) reicht dafür nicht.
- **Nicht gemessen:** andere Presets als *glassworks 3*; die 610M.

### ✅ Blanke Pipeline blendete aus und driftete nach oben links (Befund S80, behoben S80)

**Befund Patrik (S80):** Neue Kette → Milkdrop → eine Custom Shape
(`asset/effectchain/202609_g_milk.lvfx`). Das Bild blendet von selbst aus und
wandert stetig nach **oben links** — obwohl der Knoten nichts davon verlangt:
`decay=1`, `zoom=1`, `rot=0`, `dx=dy=0`, `warp=0`, `sx=sy=1`, keine
Warp-/Comp-Shader.

**Sonde S80 — belegt: unser Fehler, nicht das Original.** Preset ohne jede
Bewegung (`PSVERSION=0`, `decay=1`, `zoom=1`, `warp=0`, keine Wave), eine
weiße Shape blitzt nur in den Frames 1–3 auf; danach je Frame-Marke ein
eigener Prozess je Seite, saatlos, 640×480, `--silence`:

| Frame | Referenz: Schwerpunkt · Energie · Spitze | LumiViz: Schwerpunkt · Energie · Spitze |
|---|---|---|
| 3 | (320,5 / 240,5) · 1093 · 1,000 | (319,0 / 239,0) · 1223 · 1,000 |
| 10 | unverändert | (315,5 / 235,5) · 1220 · 1,000 |
| 30 | unverändert | (305,5 / 225,5) · 1176 · 1,000 |
| 60 | unverändert | (290,5 / 210,5) · 1077 · 0,953 |
| 120 | unverändert | (260,5 / 180,5) · 872 · 0,835 |

Die Referenz steht über 120 Frames **pixelgenau still**. Bei uns wandert das
Nachbild **exakt 0,5 Pixel je Frame** in x **und** y nach oben links (60 px in
120 Frames) und verliert dabei 29 % Energie durch Weichzeichnen. Das ist genau
die Signatur des Halbtexel-Versatzes. Sonde als Regressionsprobe:
`asset/calibration/milkdrop/drift/` (`python drift_sonde.py`, urteilt selbst
STEHT/DRIFTET).

**Ursache (durch die Gegenprobe belegt):** der Halbtexel-Versatz im Warp-Mesh,
`MilkdropVisualizer.cpp` (`u += 0.5/width; v += 0.5/height`). Im
Original gleicht er die D3D9-Pixelmitten-Konvention aus (dort liegen
Pixelmitten auf ganzen Koordinaten) und verschiebt **nichts**. Unter OpenGL
liegen die Pixelmitten schon auf +0,5 — derselbe Versatz verschiebt das Bild
dann **je Frame um ein halbes Pixel** (Drift diagonal) und mischt bei
`GL_LINEAR` je Frame vier Nachbarn (Weichzeichnen, das wie Ausblenden
aussieht). Beides passt zum Befund, beides fällt bei importierten Presets kaum
auf, weil deren eigene Bewegung und `decay<1` es überdecken.

**Fix (Entscheid Patrik a, S80):** Versatz entfernt. Die Bedingung
„importierte Presets nicht verändern" war als Treue-Schutz gemeint — die
Sonde zeigte, dass sie heute schon genau um diese Drift vom Original
abweichen; der Fix bringt sie näher heran. **Gegenprobe:** Sonde nachher
0,00 px Wanderung, 0,0 % Energieverlust (Referenz ebenso).

**Messreihe** (22 mitgelieferte Presets, saatlos, Marken 10/30/120, 320×240
und 640×480; Vorher- und Nachher-Stand beide gegen **dieselben**
Referenzbilder gerechnet, damit das Wanduhr-Rauschen der Referenz nicht
mitmisst):

| Größe | Frame 10 | Frame 30 | Frame 120 |
|---|---|---|---|
| 320×240 | Δ −0,0015 · 2 besser / 0 schlechter | Δ −0,0014 · 4 / 3 | **Δ −0,0070 · 6 / 1** |
| 640×480 | Δ −0,0006 · 1 / 0 | Δ +0,0012 · 0 / 2 | **Δ −0,0065 · 4 / 0** |

Größte Gewinne bei Frame 120: `Playaround` 0,417 → 0,329 (320) bzw.
0,396 → 0,341 (640), `Twisted`/`Twisted b` −0,016 bis −0,030,
`Supernova-Lens` 0,094 → 0,068, `Neomtwister` −0,021. Schlechter nur knapp:
`Twisted` bei Frame 30 (+0,008 bis +0,018 — dort streut die Referenz selbst
0,014–0,021), `In The Spotlight V1` f120 +0,006, `Dancing Hearts (2nd edit)`
f30 +0,006. Urteile: 320×240 **3 OK / 10 Befunde** (vorher 3 / 11 —
`Dancing Hearts (2nd edit)` BEFUND → PRÜFEN), 640×480 **2 OK / 11 Befunde**
(vorher 2 / 12). Alle Einzelwerte: `asset/calibration/milkdrop/drift/Messreihe_S80.md`.

**Der alte Look bleibt wählbar — und mehr (S80, Wunsch Patrik):** Gruppe
„Gestaltung (kein Original-Verhalten)" im Milkdrop-Node — Linear-Drift
(Stärke + Richtung), Radial-Drift (Stärke, Winkel außen/Wirbel/Sog, Zentrum)
und Weichzeichnen, je Werkzeug ein Schalter, Winkel in Grad; per `per_frame`
steuerbar (`drift_lin/dir/rad/ang/cx/cy`, `weich`, Bogenmaß). Knöpfe
„Original" / „Weicher Sog ↖" (bitgleich der Stand vor dem Fix). Richtungen am
Bild geprüft (`sichttest_auto.py` G1). Details: Benutzerhandbuch §11,
MilkdropVisualizer.md 1.27.0. **Offen:** Sichttest am Panel (Knöpfe, Gefühl
der Regler) — Protokoll Teil F.

**Rest, klein und getrennt:** die Shape selbst liegt bei uns **1 px** weiter
oben links als in der Referenz (Schwerpunkt 319,5 statt 320,5) und ist etwa
6 % flächiger (Energie 1156 statt 1093). Das ist die Rasterung der Shape,
nicht der Warp — sie wandert nicht mehr. ⚪ offen, nur bei Pixelvergleichen
relevant.

Verwandt: E3 im [Sichttest-Protokoll](Sichttest_Protokoll_S79.md)
(Diagonalenkreuz bei `a`≠`a2` — dort kein Fehler, aber das Weichzeichnen
verstärkt es).

### 🟡 `martin - stratospheric turbulences` (1 und 2) weicht grob ab (Befund S80 — S131: dieselbe Szene, Rest kleiner Versatz)

**Stand S131 (Entscheide Patrik S2 a, S3 a):** Mit dem Stand nach S130/S1 a sitzt
das Tunnelzentrum mittig wie in der Referenz (Anteil 3 unten erledigt). Die Szene
war aber verdreht, die Röhren hatten vertauschte Farben. Der Zufallsstrom ist es
nicht — eine Sonde mit den wörtlichen `per_frame_init`-Zügen ist auf beiden
Seiten gleich (`tr2`, `tubes`, …). Zwei Fehler, beide belegt mit Sonden in
`asset/calibration/milkdrop/ysonde/`:

1. **Selbst gebaute quadratische Matrizen waren transponiert**
   (`HlslTranspiler.hpp`, Konstruktor-Emission): nur `float2x3`/`float3x2`
   wurden zeilenweise umgeordnet, `float2x2(a,b,c,d)` ging als `mat2(a,b,c,d)`
   durch — HLSL füllt zeilenweise, GLSL spaltenweise; `mul` setzt dagegen
   GLSL = HLSL voraus. Jede Rotation drehte andersherum, seit C1/C3, in beiden
   Ständen. Sonden `M_comp_skalar`/`M_comp_zeilen` 0,076 („OK“ genannt, im Bild
   die vertauschte Achse) → 0,000. Reichweite: im Testsatz 0 von 22, im
   MegaPack 12 595 Presets (12 108 mit `mul`). **Behoben (S2 a):**
   `TranspileOptions::quadratMatrixZeilenweise` (HlslTranspiler 1.7.0), gesetzt
   aus denselben Feldern je Pass; Combos heißen jetzt *Warp shader* / *Comp shader*.
2. **2D-Rauschtexturen wurden gespiegelt gelesen:** sie lagen in D3D-Zeilenfolge,
   gelesen wird im Raum der Referenz über `lv_koord` — im Warp schon immer
   gespiegelt (`N_warp` unter „bisher“ 0,339), im Comp seit S130 (`N_comp`
   Original 0,340, bisher 0,013). **Behoben (S3 a):** eine zeilenweise
   gespiegelte Kopie (`m_noiseTexGl`, GL-Lage wie alle anderen Texturen) wird
   gebunden, wenn der Pass im Raum der Referenz liest; „bisher“ bindet die alte.
   `tex3D` war nie betroffen. Sonden `N_comp` 0,011, `N_warp` 0,004 (mit
   `--zufall-lumiviz`).

**Belegt:** `stratospheric` 1 und 2, 640×480, Saat 0, f10/f30/f120/f300:
0,100/0,061/0,062/0,049 → **0,063/0,041/0,027/0,023**, im Bild dieselbe Szene
(Röhrenfarben, Fenster, Helligkeit); Ablage `out/strato_s131/`. Sonden 16/16 OK.
Vergleichslauf 22 × 6 Formate (`out/wp_s131b/`): „bisher“ 748/748 bitgleich zu
S129, „Original“ 748/748 bitgleich zum Stand nach S1 a (kein Testpreset baut
Matrizen oder liest 2D-Rauschen — der Testsatz deckt beides nicht ab).
**Rest:** ein kleiner Versatz in Maßstab/Phase (Wanduhr der Referenz?), nicht zerlegt.
Nebenbefund: LumiViz nimmt `sampler_pw_noise_lq` ohne Deklaration an, die
Referenz bricht mit X3004 ab.

Aufgefallen, weil sich der Look mit dem Halbtexel-Fix sichtbar änderte (Frage
Patrik: „ist er jetzt näher am Original?"). Gemessen gegen MilkdropRef, 640×480,
Marken 10/30/120/300, derselbe Referenzlauf gegen beide Stände
(`LUMIVIZ_MILKDROP_HALBTEXEL=1` = alter Versatz): **neu ist in allen acht
Messungen näher, aber nur um 0,001–0,004 MAE** — der Fix ist hier nicht die
Ursache der Abweichung. Die eigentliche Abweichung ist viel größer
(MAE 0,11–0,16) und hat drei sichtbare Anteile:

1. **Helligkeit etwa halb** (Referenz 0,14–0,21, wir 0,07–0,09).
2. **Gelb-orange Lichtkugeln fehlen** bei uns ganz.
3. **Tunnelzentrum:** bei Frame 10 sitzt es bei uns oben rechts, in der
   Referenz mittig; die farbigen „Fenster" liegen oben/unten vertauscht. Kann
   Phasenversatz sein (die Referenz taktet an der Wanduhr) oder dieselbe
   Y-Frage wie die offene `warp_1`-Sonde (unten, Strang S75) — dort zuerst
   nachsehen.

Montagen (Referenz | neu | alt): Scratchpad S80 `strato/out/`.

**Untersuchung S80 — vier Zufallsquellen, zwei davon gelöst:**

1. ✅ **Rausch-Texturen** würfelt das Original aus der **Uhr**
   (`plugin.cpp:2480`), **`rand_preset`/`rot_*`** bei jeder Ladung aus
   `rand()` (`state.cpp:1875`). Die Referenz war bei diesem Preset damit
   selbst nicht reproduzierbar (Ref↔Ref MAE 0,05–0,08). **Neu:**
   `MilkdropRef --zufall-lumiviz` schreibt dieselben Werte wie LumiViz in den
   Kern (gemeinsamer Header `milkdrop/MilkdropZufall.hpp`, Kern unangetastet)
   → Ref↔Ref **0,003–0,009**. LumiViz rechnet nach dem Umzug in den Header
   bitgleich (belegt f10/f120). Nebenbefund: `rand_preset`, `rand_frame` und
   `rot_rand*` zogen ihre Zahlen als Funktionsargumente — Reihenfolge nur
   Compiler-Gewohnheit (MSVC-ABI: rechts nach links); jetzt festgeschrieben.
2. ⚠️ **`rand_frame`** würfelt der Kern mitten im Frame — für den Host
   unerreichbar. Dieses Preset nutzt es nicht.
3. 🟠 **EEL-`rand()` im MilkDrop-Dialekt ist bei uns falsch.** Das Original
   (ns-eel2, gebaut **ohne** `NSEEL_EEL1_COMPAT_MODE`,
   `vis_milk2/plugin.vcxproj`) liefert eine **Kommazahl** in
   [0, ⌊max(x,1)⌋] aus einem Mersenne-Twister mit festem Startwert
   `0x4141f00d` (`nseel-cfunc.c:42/101`). Wir liefern ganze Zahlen aus dem
   AVS-Strom — `rand(1)` ist bei uns immer 0, im Original gleichverteilt.
   Nachgebaut in `scripting/NseelZufall.hpp`, vorerst nur per Diagnose
   `LUMIVIZ_MILKDROP_EEL_RAND=original`. Wirkung an `stratospheric`: Blick
   jetzt **mittig in den Tunnel** wie im Original (vorher schräg oben rechts),
   MAE 0,122 → 0,112 (f10), 0,155 → 0,141 (f120); Röhren aber noch diagonal
   statt waagrecht. **Messreihe (22 Presets, gegen dieselben Referenzbilder):
   gemischt** — `Dancing Hearts` 0,223 → 0,157 und `Supernova-Lens` 0,068 →
   0,003 (f120, 640), aber alle drei `Rock The House` +0,033…+0,036. Deutung:
   die Semantik (Kommazahl) stimmt, der **Stromstand** nicht — ein Suchlauf
   über 0…39 übersprungene Züge fand kein klares Minimum (0,096…0,117).
   **Aufgeklärt (S80, Messkopie `tools/MilkdropRef/patched/nseel-cfunc.c`,
   zählt/loggt die Züge, Verhalten gleich):** der Kern lädt beim Hochfahren
   schon ein Startpreset, dessen `per_frame_init` Züge verbraucht (hier 8).
   `--zufall-lumiviz` setzt den Strom jetzt vor `LoadPreset` auf den
   Prozessstart. **Unser Nachbau trifft die ersten acht Originalwerte exakt**
   (`test_NseelZufall`). Mit abgeglichenem Strom zeigt `stratospheric`
   **dieselbe Szene** wie das Original (Röhren, Fenster, Farben); Rest:
   einige Grad Drehung (Phase/Wanduhr?), gelbe Leuchtkugeln bei uns grün
   (Comp-Shader, `GetPixel`/`GetBlur2` × `float3(0.8,0.8,0)` — eingegrenzt:
   ohne Comp-Shader stimmt die Szene auf 0,004–0,012).
   **Messreihe mit abgeglichenem Strom** (22 Presets, `compare_milkref.py
   --zufall-lumiviz`): weiterhin gemischt. `Supernova-Lens` f10 0,043 →
   0,002, aber `Rock The House` (alle drei) f120 0,030 → 0,066. RTH würfelt
   **je Gitterpunkt** (`per_pixel`: `zoom=1+0.1*sin(…)*(rand(3)-1)`, 825
   Züge/Frame bei 32×24); die Referenz zieht dort nachweislich Kommazahlen
   wie wir. Das Mesh gleichzuziehen (`MilkdropRef --mesh 32x24` — MilkDrop3
   legt 64×64 fest, Winamp und wir 32×24) ändert nichts (Mittel 0,0858 vs.
   0,0861). Verdacht war die Reihenfolge der Gitterpunkte — **widerlegt und
   der Zufall je Gitterpunkt belegt (S80, Gitter-Sonde
   `asset/calibration/milkdrop/gitter/gitter_sonde.py`):** (1) ein Schub nur
   an Punkt K landet bei 13 geprüften K auf beiden Seiten an derselben
   Stelle (±1 px; Punkt 0 links unten, zeilenweise nach oben); (2) Züge je
   Frame gleich (1650 bei 2×rand je Punkt, Log `LUMIVIZ_MILKDROP_EEL_RAND_LOG`
   gegen die Messkopie); (3) **Wirkung von `dx/dy=0.004*(rand(3)-1)` je Punkt,
   Korrelation mit dem Original: Original-rand +0,85 / +0,97 / +0,98 (5/10/30
   Frames), bisher −0,03 / +0,03 / −0,04** — das bisherige Muster hat mit dem
   Original nichts zu tun. Obergrenze: ein Muster ohne Zufall erreicht
   +0,82…+0,89 (Rest = Abgriff D3D9↔GL). **Folgerung:** Generator, Strom und
   Reihenfolge stimmen; die schlechtere MAE bei `Rock The House` stammt aus
   dessen anderen, bekannten Abweichungen (und MAE bevorzugt dunkle Bilder —
   das bisherige RTH-Bild war schlicht dunkler, nicht treuer). Schalter
   „Skript-Zufall" im Milkdrop-Knoten umgesetzt (Standard bisher, Entscheid
   Patrik a); Standard-Frage offen (s. Session 80).
4. 🟠 **Helligkeit etwa halb, gelbe Lichtkugeln fehlen — Ursache gefunden
   (S80):** nicht der Comp-Shader. Comp-Quellen-Sonde
   (`asset/calibration/milkdrop/comp_quellen.py`, Comp-Shader durch je eine
   Quelle ersetzt): schon das rohe Feedback-Bild (`GetPixel`) hat bei uns fast
   kein Rot/Grün (R 0,002 / G 0,003 gegen 0,023 / 0,013; Blau gleich) — die
   gelb-orangen Shapes 1 und 3 fehlen. Die zeichnen an Positionen aus
   `gmegabuf(1000…)`, die `per_frame` aus einer **unsichtbaren Spektrum-Wave**
   baut (`wave_0`, `bSpectrum=1`, schreibt `value1+value2` nach
   `gmegabuf(0…127)`). **Spektrum-Wave-Sonde**
   (`asset/calibration/milkdrop/spektrum_wave.py`, dieselbe Wave sichtbar):
   das Original liefert ein flaches Profil um 0,3, wir eine steile Kurve
   (0,93 links → 0,28 rechts). **Ursache:** unsere Custom-Waves nehmen das
   Spektrum der App (BASS-FFT) mit einem per Auge geschätzten Faktor
   (`kSpecPortScale = 8`, S39); das Original rechnet ein EIGENES FFT aus den
   576 PCM-Samples (`fft.cpp`: Hüllkurve, Entzerrung, 512 Bänder). Das
   betrifft **jedes Preset mit Spektrum-Wave**. Auch die Wellenform-Werte
   (`bSpectrum=0`) hingen an einem geschätzten Faktor (`kWavePortScale = 192`).
   **Umgesetzt (S80, Entscheid Patrik a):** `milkdrop/MilkdropFft.hpp` — Port
   von `fft.cpp` (Dämpfung, Sinus-Hüllkurve, Entzerrung, 1024-FFT, float wie
   im Original) auf der 8-bit-Wellenform (`lround(x·127)`); Custom-Waves mit
   den Original-Faktoren 0,15/0,004. Schalter je Milkdrop-Knoten
   (`PresetState.audioOriginal`; Diagnose `LUMIVIZ_MILKDROP_AUDIO`) — hieß
   zuerst „Wave-Audio“ mit Standard bisher; seit dem zweiten Schritt unten
   EIN Feld **„Audio“** für Waves und Lautheit, **Standard Original**
   (Entscheid Patrik a, S80; `MilkdropPresetState.hpp`, bereinigt S129).
   **Beleg:** `test_MilkdropFft` — dieselbe Wellenform ergibt das Spektrum
   des Original-Kerns bis auf 5·10⁻⁷ (alle 512 Bänder; Sollwerte aus
   `MILKDROPREF_AUDIO_DUMP`).

   🔴 **Dabei gefunden — Fehler im MESSGERÄT:** `MilkdropRef` gab dem Kern
   das PCM mit **128 als Mitte** (`lround(x·127)+128`). Winamp liefert aber
   **vorzeichenbehaftete** 8-bit-Werte (oberes Byte der 16-bit-Samples,
   `winamp_orig/…/VIS.cpp:766/774`), und der Kern wandelt mit
   `(pcm ^ 128) - 128` (`pluginshell.cpp:1668`). Der Kern sah dadurch
   `v−128` bzw. `v+128` — eine zerhackte Wellenform voller Breitband, in
   Custom-Waves, Spektrum **und `bass`/`mid`/`treb`**; Stille (`128`) war
   volles negatives Gleichsignal (−128). `AvsRef` machte es richtig
   (`sw & 0xFF`). **Behoben** (Zweierkomplement, Stille = 0). **Alle
   MilkDrop-Referenzzahlen seit S63 stehen unter diesem Vorbehalt** — die
   Messreihe S80 läuft schon gegen die korrigierte Referenz.

   **Messreihe S80 gegen die korrigierte Referenz** (22 mitgelieferte
   Presets, `--zufall-lumiviz`): über „Wave-Audio“ sagt sie **nichts** —
   keines der 22 hat eine aktive Custom-Wave (alle Bilder bitgleich); dafür
   braucht es eine eigene Auswahl. Sie zeigt aber, was der PCM-Fix an der
   Referenz verschiebt: Mittel kaum verändert (320: 0,107 → 0,108), einzeln
   stark — besser `Neomtwister` f120 0,306 → 0,184, `Playaround` 0,326 →
   0,234; **schlechter `Magma 2` f120 0,002 → 0,160, `Magma` 0,022 → 0,161,
   `Rock The House` f10 0,12 → 0,22.** `Magma` hat weder `rand()` noch
   Custom-Waves — der Einbruch kommt allein aus dem jetzt richtigen Audio der
   Referenz. **Deutung:** unser `bass/mid/treb` (MilkLoudness, S43) rechnet
   das Verfahren des Originals nach, speist es aber mit dem App-Spektrum —
   und wurde gegen die verzerrte Referenz kalibriert. Das Original nimmt sein
   eigenes FFT (`pluginshell.cpp:1690ff`), das jetzt exakt portiert ist.

   ✅ **Umgesetzt (S80, Entscheid Patrik a): bass/mid/treb aus der
   Original-Aufbereitung** — `milkdrop/MilkdropAudio.hpp`: je Frame
   AnalyzeNewSound (Welle, Wave-Spektrum), **AlignWaves** (Port
   `pluginshell.cpp:2342`, Ausrichtung am Vorframe) und
   DoCustomSoundAnalysis (zweites FFT der ausgerichteten LINKEN Welle ohne
   Dämpfung, Bänder = Sechstel-Summen; Original-Eigenheit
   `myfft.Init(576,512,-1)`: −1 ist wahr, Entzerrung läuft). MilkLoudness
   bleibt, bekommt nur das neue `imm`. EIN Schalter „Audio“ (Waves + Lautheit).
   **Beleg `test_MilkdropAudio`:** 12 Frames des Original-Kerns nachgespielt —
   ausgerichtete Welle in beiden Kanälen **identisch**, `imm` auf 1e-5.
   Custom-Waves mit `bSpectrum=0` sehen jetzt die ausgerichtete Welle.
   **Messreihe S80 „Audio“** (gegen dieselben, korrigierten Referenzbilder,
   `--zufall-lumiviz`; Skript-Zufall Original):

   | Gruppe | Frame 10 | Frame 30 | Frame 120 |
   |---|---|---|---|
   | 20 mitgelieferte, 320×240 | 0,087 → **0,055** (5 besser / 0 schlechter) | 0,107 → **0,088** (11/0) | 0,144 → **0,118** (11/1) |
   | 20 mitgelieferte, 640×480 | 0,088 → **0,057** (4/2) | 0,101 → **0,084** (10/0) | 0,155 → **0,128** (13/0) |
   | 20 mit Custom-Wave, 320×240 | 0,203 → 0,202 (3/5) | 0,259 → 0,254 (6/1) | 0,291 → **0,274** (8/3) |

   Größte Gewinne (f120): `Gin Tonic on Ice 003` 0,865 → 0,514,
   `Playaround` 0,237 → 0,109, `Dancing Hearts` 0,185 → 0,099,
   `Neomtwister` 0,150 → 0,073. Schlechter: `MilkDrop2077.R030` 0,178 →
   0,274, `Pixies Party (Hakan mash-up) 8.4` 0,161 → 0,212. Die
   Custom-Wave-Gruppe liegt insgesamt noch hoch — dort wirken weitere,
   ungeklärte Abweichungen (u. a. die y-Grundlinie der Waves, s. unten).

   ✅ **Geklärt und umgesetzt (S81, Entscheid Patrik a): (b) y-Grundlinie
   der Custom-Waves** — das Original streckt x/y um `m_fInvAspectX/Y`
   (`milkdropfs.cpp:2612`), bei 4:3 y um 4/3 um die Bildmitte; wir liessen
   den Faktor seit S39 bewusst weg (Kommentar „Referenz rechnet quadratisch"
   war falsch, der Puffer des Originals ist fenstergross). Beleg
   `spektrum_wave.py`: 480×480 Abweichung 0,003 (kein Unterschied), 640×480
   Wellenform-Probe 0,122 → **0,003** mit Streckung. Schalter
   `PresetState.aspektOriginal` „Seitenverhältnis (Waves)“, Standard
   Original. **Messreihe S81** (22 Presets mit Custom-Wave = die ersten 20
   aus `asset/Milkdrop3/presets` + R030 + Pixies Party 8.4; saatlos,
   `--zufall-lumiviz`), bisher → Original:

   | Größe | Frame 10 | Frame 30 | Frame 120 |
   |---|---|---|---|
   | 320×240 | 0,171 → 0,174 (0/3) | 0,235 → 0,235 (3/1) | 0,196 → 0,194 (3/2) |
   | 640×480 | 0,150 → 0,157 (0/2) | 0,225 → 0,225 (4/1) | 0,179 → 0,177 (2/1) |

   In dieser Gruppe **kaum messbar** — die meisten Waves liegen nahe der
   Mitte oder bewegen sich im Rückkopplungsrauschen; die Gruppe wird von
   anderen Abweichungen beherrscht (10 BEFUND, schon ab Frame 10). Einzelfälle
   streuen gegenläufig (`R030` f120 320: 0,281 → 0,248, 640: 0,356 → 0,405;
   `Gin Tonic on Ice 004` f10 640: 0,101 → 0,186 — Montage: Unterschied in
   Ringen/Echo, die Wave selbst liegt bei y ≈ 0,48). Der Treue-Beleg ist die
   Sonde, nicht diese Gruppe.

   ⬜ **Neu offen (S81):** `fishbrain and flexi - lovecraft [mr understatement]
   [organic aftermath]` — MilkdropRef bricht mit rc=2 ab (BMP fehlt), in
   beiden Größen; Ursache nicht untersucht.

   **Kalibrierlauf S81 — die 22 mitgelieferten, neu gegen die PCM-korrigierte
   Referenz** (`compare_milkref.py --formate aspekt --zufall-lumiviz`, Marken
   10/30/120; ersetzt alle MilkDrop-Zahlen mit PCM-Vorbehalt). Gültig ist nur
   der **saatlose** Teil: der mitgefahrene 0x5EED63-Lauf ist verworfen — die
   Referenz säte einen Frame zu spät (danach behoben, INDEX §2.3). Urteile
   daraus neu gebildet, mit der geschärften Anlaufregel
   (`out/milkref_kalibrierung_S81/formate_s0.md`):

   | Format | MAE f10 / f30 / f120 | OK / PRUEFEN / BEFUND |
   |---|---|---|
   | 1:1 560×560 | 0,077 / 0,086 / 0,116 | 5 / 7 / 10 |
   | 4:3 640×480 | 0,064 / 0,077 / 0,112 | 4 / 7 / 11 |
   | 16:9 736×416 | 0,056 / 0,067 / 0,098 | 5 / 7 / 10 |
   | 3,4:1 1024×304 | 0,045 / 0,055 / 0,084 | 8 / 4 / 10 |
   | 3:4 hoch 480×640 | 0,062 / 0,104 / 0,123 | 6 / 5 / 11 |

   (Anlaufregel: `Playaround` wechselt von PRUEFEN zu BEFUND — sein grünes
   f10 war nur der Warmlauf der Referenz, das erste echte Urteil ist f30
   0,25.)

   Je Preset (4:3, f10/f30/f120):
   - **OK in allen Formaten:** `Rock The House` (+ `_2024`, `_newmove`)
     0,016/0,019/0,026 — nach dem PCM-Fix, vor dem Audio-Fix S80 lag f10
     bei 0,22; `Starfield`
     0,002/0,004/0,001.
   - **Referenz stumm, wir zeichnen:** `Helix`, `Starfield (…edit)` samt
     Starfield-2-Familie, `The Beauty and the Math(s)` (f120 0,497). Die
     Referenz ist in **allen** Marken ganz schwarz (mean 0, max 0), auch
     gesät. Gemeinsam: `PSVERSION=4` (ps_3_0). Das „FORMATABHAENGIG“ der
     Starfield-2-Familie (f10 1:1 0,197 … hoch 0,003) misst nur, wie viel
     WIR je Format schon zeichnen — kein Urteil möglich. **Ursache
     gefunden (S81): Treiber der 610M** — der Kern paart ps_3_0 mit vs_1_1,
     die 610M zeichnet dann nichts. Auf der 4090 (INDEX §2.3 Punkt 3):
     `Beauty` **OK** (f120 0,038), `Helix`/Starfield-edit STARTABHAENGIG,
     saatlos gut (Helix 0,000/0,013/0,083).
   - **Große Abweichung, formatunabhängig:** `Twisted`/`Twisted b` f30
     0,29 (breiter = kleiner: 3,4:1 0,10), `Playaround` f30 0,25,
     `Dancing Hearts` f10 0,21 (Farben, Aufgabe 3), `Neomtwister`
     0,196/0,096/0,071, `Lasershow` f10 0,12.
   - **Schwankend:** `Magma`/`Magma 2` f120 0,01–0,15 je Format (f10/f30
     ≈ 0).

   ✅ **Neu gefahren auf der 4090 (S82; Entscheid Patrik S81: 4090 =
   Standard-Referenz, Referenz und Prüfling auf derselben Karte).** Alle
   22 × 5 Formate × beide Startwerte (0 und 0x5EED63 — seit dem Saat-Fix
   S81 gültig), `--zufall-lumiviz`, Ausgabe
   `out/milkref_kalibrierung_S81_4090/formate.md`. **Diese Tabelle ersetzt
   die 610M-Tabelle oben** (die bleibt als Kartenvergleich stehen):

   | Format | MAE saatlos f10 / f30 / f120 | MAE gesät f10 / f30 / f120 | OK / PRUEFEN / BEFUND / STARTABH. |
   |---|---|---|---|
   | 1:1 560×560 | 0,082 / 0,070 / 0,086 | 0,118 / 0,136 / 0,217 | 2 / 2 / 3 / 15 |
   | 4:3 640×480 | 0,062 / 0,061 / 0,081 | 0,122 / 0,113 / 0,218 | 2 / 0 / 4 / 16 |
   | 16:9 736×416 | 0,058 / 0,051 / 0,066 | 0,115 / 0,154 / 0,220 | 2 / 1 / 2 / 17 |
   | 3,4:1 1024×304 | 0,049 / 0,038 / 0,047 | 0,108 / 0,139 / 0,239 | 2 / 0 / 2 / 18 |
   | 3:4 hoch 480×640 | 0,065 / 0,092 / 0,107 | 0,128 / 0,124 / 0,234 | 1 / 1 / 4 / 16 |

   **Kartenvergleich 610M ↔ 4090** (saatlose Früh-MAE je Preset):
   - **Gleich auf beiden Karten** (Abstand ≤ 0,005 in allen Formaten): alle
     ps_2-Presets — `Dancing Hearts` (+ 2nd edit), `Lasershow`, `Magma`
     (+ 2), `Playaround`, `Rock The House`-Familie, `Starfield`,
     `Twisted` (+ b), `In The Spotlight V1/V2`. Die Karte ist für sie
     kein Messfaktor.
   - **Nur auf der 4090 messbar** (610M: Referenz schwarz, ps_3_0):
     `The Beauty and the Math(s)` **OK** in allen Formaten (f10 0,055;
     3:4 hoch `Maths` PRUEFEN wegen f120 0,175 saatlos, gesät 0,033),
     `Helix` und Starfield-edit-Familie jetzt STARTABHAENGIG statt BEFUND.
     Die FORMATABHAENGIG-Markierung der Starfield-2-Familie (f10 1:1
     0,186 … hoch 0,006) bleibt auch auf der 4090 — jetzt ein echtes
     Urteil, nicht mehr nur „wie viel WIR zeichnen“.
   - **`Neomtwister`:** auf der 4090 deutlich näher (4:3 0,196 → 0,122,
     16:9 0,200 → 0,133, hoch 0,233 → 0,141), aber weiter BEFUND/PRUEFEN.
   - **Echte Befunde, kartenunabhängig:** `Dancing Hearts` (Farben,
     Aufgabe 3), `Lasershow` f10 0,12 in allen Formaten, `Neomtwister`.

   **Vorbehalt Früh-MAE 0,000:** `Helix`, `Magma`, `Magma 2`,
   `Playaround` sind saatlos in f10 auf **beiden** Seiten ganz schwarz
   (mean 0, max 0) — die 0,000 in `formate.md` ist kein Treuebeleg. Das
   Urteil trägt dort der gesäte Lauf.

   Messbetrieb (S82): CPU im Mittel 98–99 °C, 11–23 % der Zeit ≥ 100 °C,
   Spitze 101,3 °C, kein Absturz. Die Last liegt auf CCD1 (97–99 °C),
   CCD2 bleibt bei 66–70 °C — Tctl zeigt den heißeren CCD.

   Beobachtung: Die MAE fällt mit der Breite (16:9, 3,4:1 kleiner) — bei den
   meisten Presets liegt der Inhalt in der Mitte, breite Formate haben mehr
   dunklen Rand (MAE bevorzugt dunkle Bilder). Formatvergleiche daher an den
   Urteilen und Montagen, nicht am Mittel.

### ✅ Y-Achse war invertiert — Shapes, Wellenformen und Comp-Shader (S75, behoben)

**Das ist die Ursache hinter dem „Modus 7"-Befund unten — und sie ist global,
nicht auf eine Wellenform beschränkt** (Verdacht Patrik, S75: „andere
MilkDrop-Effekte wirken auch, als stünden sie auf dem Kopf").

**Beweis** — eine Sonde aus `Blank.milk` mit ersetztem Comp-Shader
(`ret = float3(uv.y, 0, 0)`), 320×240, saatlos, beide Seiten je eigener
Prozess:

| | Rot oben | Rot unten | ⇒ |
|---|---|---|---|
| **Referenz** (MilkdropRef) | 0,125 | 0,875 | `uv.y` wächst **nach unten** (D3D-Konvention, (0,0) links oben) |
| **LumiViz** | 0,875 | 0,125 | `uv.y` wächst **nach oben** (GL-Konvention) |

Damit ist auch **die Referenz entlastet**: sie wurde *nicht* versehentlich
„korrigiert" (Verdacht Patrik). `tools/MilkdropRef` enthält keine
Inhalts-Spiegelung — nur `bih.biHeight = -h`, die BMP-Konvention für
top-down-Zeilen; die Datei wurde seit ihrer Entstehung (S63) zweimal angefasst.
Gegenprobe am Bild: das Herz in `Dancing Hearts` steht in der Referenz
**richtig** (Rundungen oben, Spitze unten), bei uns auf dem Kopf.

Die Datei dokumentiert im Kopf „Y convention: ONE internal math space … the
single vertical flip happens in the composite pass" — genau dieser Vertrag ist
verletzt, sobald ein Preset einen eigenen Comp-Shader mitbringt.

**✅ Teil 1 behoben (S75): der Custom-Comp-Zweig hatte den Flip nicht.**
`compositeToScreen` trägt laut Vertrag den einzigen vertikalen Flip — der
MD1-Zweig setzt ihn über `vRef = (py < 0) ? hi : lo`, der Custom-Zweig zeichnete
sein Präsentations-Quad dagegen mit `(-1,-1) → uv(0,0)`, also ungeflippt.
Korrigiert; die Y-Sonde liefert seitdem auf **beiden** Seiten 0,125 oben /
0,875 unten, Urteil OK. Gegenprobe über alle 19 (f120, saatlos): keine
Verschlechterung.

**🔴 Teil 2 offen — und das ist der, den man sieht.** `Dancing Hearts` hat
**keinen** Comp-Shader (null `comp_*`-Zeilen; es nutzt `nWaveMode=7` +
`shapecode_0_enabled=1`) und ist nach dem Fix unverändert bei MAE 0,376. Im
Bild sind **beide** Elemente gespiegelt — das Herz *und* der kleine Knoten, der
in der Referenz rechts **oben** sitzt und bei uns rechts **unten**. Betroffen
ist also die Zeichenebene für **Wellenformen und Shapes**, nicht das Composite.

**Tiefenanalyse (S75): die Formeln stimmen — die Textur-Konvention nicht.**
Zeile für Zeile gegen `milkdropfs.cpp` gelegt, alle drei Stellen sind
wortgleich zum Original:

| | Original | wir |
|---|---|---|
| Shape-Vertex | `v[0].y = var_pf_y*-2+1` (:2300) | `cyN = y*-2.0+1.0` (:3406) |
| Wave-Punkt | analog | `pt.y = py*-2.0+1.0` (:3677) |
| Wave-Position | `fWavePosY = cy*2-1` (:2762) | `wavePosY = waveY*2.0-1.0` (:3747) |

Auch der Original-Sonderfall stimmt: `wave_y` ist dort ausdrücklich
„backwards (top==1) … so we keep it that way" (:2726) — genau so rechnen wir.

**Damit liegt es nicht an den Formeln, sondern am Raum, in den sie zeichnen.**
Unsere Feedback-Textur ist gegenüber der D3D-Textur vertikal gespiegelt —
deshalb *braucht* das Composite überhaupt einen Flip (der laut Kopfkommentar
„einzige"). Folge: Geometrie, die nach Original-Formel bei NDC `+1` (oben)
landet, sitzt in unserer Textur oben — und der Präsentations-Flip kippt sie
nach unten. Die uv-Abbildung ist danach korrekt (Y-Sonde grün), die
**gezeichnete Geometrie** dagegen gespiegelt. Genau das sieht man am Herz.

**✅ Teil 2 behoben (S75), Variante (b) — Spiegelung möglichst spät, Formeln
bleiben wörtlich am Original.** Drei Stellen:

| Stelle | vorher | jetzt |
|---|---|---|
| Custom Shapes, Mittelpunkt (:3406) | `y*-2+1` | `y*2-1` |
| Custom Shapes, Ring (:3452) | `cyN + rad*sin` | `cyN − rad*sin` |
| Custom-Wave-Punkte (:3686) | `py*-2+1` | `py*2-1` |
| **Alle Wellenform-Modi** (:3949) | — | `for (auto& p : pts) p.y = -p.y;` |

Die Wellenform wird **an einer einzigen Stelle** gespiegelt — nach dem
Modus-Block, vor dem Glätten. Das hält die Modus-Formeln deckungsgleich zum
Original und lässt `wavePosY` unangetastet, was nötig ist: Modus 7 benutzt es
nicht additiv, sondern in der Stereo-Trennung `sep = (wavePosY*0.5+0.5)²`.

**Gegenprobe (alle 19, f120, saatlos): 7 besser, 0 schlechter.**

| Preset | vorher | nachher |
|---|---|---|
| Dancing Hearts | 0,376 | **0,230** |
| Dancing Hearts (2nd edit) | 0,329 | **0,231** |
| Neomtwister | 0,359 | **0,326** |
| Twisted / Twisted b | 0,288 | **0,251** |
| Lasershow | 0,081 | **0,056** |
| Helix | 0,118 | **0,112** |

Die scheinbare Ausnahme `Starfield (Supernova-Edit)` (0,002 → 0,043) ist keine:
dort ist die **Referenz schwarz**, der Wert misst nur, wie viel *wir* zeichnen —
ohne Referenzbild gibt es nichts zu treffen. Y-Sonde jetzt **MAE 0,000**,
Gegenprobe „gespiegelt passt besser" ist umgeschlagen (0,374 statt vorher
0,225), und im Bild steht das Herz richtig: Rundungen oben, Spitze unten,
Knoten rechts oben wie in der Referenz.

Die verbleibenden 0,230 bei `Dancing Hearts` sind der **Farbbefund**
(Aufgabe 3: bei uns weiß/rosa, im Original gelb/orange) — ein eigener,
unabhängiger Punkt.

**✅ Behoben S130** (Entscheide WP1 a, WP1-R a, WP2 b; Zeile „Shader sehen y wie die Referenz“ in der
Tabelle unten): der Warp-Shader sah y gespiegelt, und — neu gefunden — der Comp-Shader las auf dem Kopf.
**Vorgeschichte, Rest offen — S129 gemessen: ja, Befund** (als 🔴 in der Tabelle „MilkDrop:
Kalibrier-Runde und offene Punkte“ unten, Sonde `ysonde/Y_warp.milk`): ob der **Warp-Shader** dieselbe Spiegelung braucht — dafür eine
eigene Sonde mit `warp_1` (analog zur Y-Sonde). Bisher spricht nichts dafür:
die y-symmetrischen Presets (`Magma`, `Rock The House`, `Starfield`) blieben
über den Fix hinweg unverändert bei ≤0,047, und ein falsch orientierter Warp
hätte sie bewegt.

**Zusatzwunsch Patrik (S75)** (nicht gebaut; als 🟡 in der Tabelle unten geführt): im MilkDrop-Host-Modul zwei Optionsfelder
**Flip X / Flip Y** anbieten — unabhängig vom Fix, als Gestaltungsmittel und
als Diagnosehilfe.

**Gegenprobe für den Fix (zwingend):** die Y-Sonde muss danach auf beiden
Seiten denselben Verlauf zeigen, UND die 5 Presets, die heute gespiegelt besser
passen, müssen sich verbessern, ohne dass die übrigen 14 schlechter werden.

Gemessen über alle 19 mitgelieferten (Marke f120, saatlos): **5 passen
gespiegelt besser** — `Dancing Hearts` (0,376 → 0,225), `Dancing Hearts (2nd
edit)` (0,326 → 0,218), `Neomtwister` (0,359 → 0,271), `Lasershow`
(0,079 → 0,054), `Playaround` (0,430 → 0,416). Die anderen 14 sind
y-symmetrisch oder stumm und entscheiden daher nichts — sie **widerlegen den
globalen Flip nicht**, anders als S73 geschlossen hatte.

<details>
<summary>S73-Fassung: „Wellenform-Modus 7 steht auf dem Kopf" (überholt — die
Beschränkung auf Modus 7 war ein Artefakt y-symmetrischer Presets)</summary>

Aufgefallen an `GreatWho - Dancing Hearts`: das Herz zeigt bei uns nach oben,
im Original nach unten. **Kein globaler Spiegelfehler** — der Gegentest über
alle 19 Presets des mitgelieferten Satzes zeigt, dass nur die
`nWaveMode=7`-Presets gespiegelt besser passen:

| Preset | nWaveMode | MAE normal | MAE gespiegelt |
|---|---|---|---|
| Dancing Hearts | 7 | 0,388 | **0,260** |
| Dancing Hearts (2nd edit) | 7 | 0,323 | **0,213** |
| Rock The House | 7 | 0,042 | 0,040 |
| Rock The House_newmove | 7 | 0,034 | 0,031 |
| 14 weitere | 0 bzw. 6 | — | keine Verbesserung, teils schlechter |

⚠️ **Einschränkung der Beweislage (Befund Patrik, S73):** die Zahlen stammen aus
einem `--auto`-STAPELLAUF, und MilkDrop-Presets **erben das Bild des Vorgängers**.
Preset N zeigt darin Reste von N−1 — an `Helix` sichtbar belegt: im Stapel stand
ein Herz im Bild (das alphabetisch davor laufende `Dancing Hearts`), im eigenen
Prozess gestartet verschwindet es. **Unverunreinigt ist nur das jeweils erste
Preset eines Laufs** — hier `Dancing Hearts (2nd edit)` mit 0,323 → 0,213.
Die Richtung des Befunds steht damit, die genauen Werte sind neu zu erheben
(Plan-Aufgabe 1).

Die beiden Rock-The-House sind zu dunkel, als dass der Spiegel die Zahl bewegt —
die Richtung stimmt trotzdem. Damit ist der Fehler auf **einen** Zweig eingegrenzt:
`MilkdropVisualizer.cpp:3887-3903`, Modus 7 („zwei Linien (Stereo) mit
Trennung", links `+sep`, rechts `−sep`, beide über `perpX`/`perpY`).

**Zwei Verdächtige, noch nicht entschieden:** das Vorzeichen von `perpY`
(`ang2 + 1.57` gegen `ang2 - 1.57`) oder der Übergang in Bildschirmkoordinaten.
Was davon stimmt, entscheidet der Zeilenvergleich gegen `milkdropfs.cpp` —
nicht raten, die Y-Konvention ist in diesem Port ausdrücklich EINE einzige
Rechenebene mit genau einem Flip im Composite (Kopfkommentar der Datei).

**Gegenprobe nach dem Fix:** die vier Mode-7-Presets müssen ohne Spiegel besser
werden, die 14 anderen dürfen sich NICHT verschlechtern. Der Vergleichslauf
steht in §8 unter „MilkDrop-Vergleich".

➜ **Steuerdokument:** [Kalibrier-Plan mitgelieferte Presets](visuals/Kalibrier_Plan_Mitgelieferte_Presets.md)
— vier Aufgaben (Vergleichswerkzeug · Modus 7 · Farben · fünf AVS-Befunde) mit
Reihenfolge, Gegenproben und Fertig-Kriterium.

**Restabweichung beachten:** auch gespiegelt bleibt Dancing Hearts bei MAE 0,260
— die Farben unterscheiden sich zusätzlich (bei uns weiß/rosa, im Original
gelb/orange). Das ist ein ZWEITER Befund, der nichts mit der Spiegelung zu tun
hat und noch niemand angesehen hat.

</details>

### MilkDrop: Kalibrier-Runde und offene Punkte

*(Bereinigt S129: dieser Teil stand bis 1.247.0 versehentlich im eingeklappten
Block „S73-Fassung … überholt“ unter dem erledigten Y-Achsen-Kopf.)*

**Kalibrier-Runde läuft seit S63.** Werkzeuge: Triage-Kette
`asset/calibration/milkdrop/` (`triage_presets.py` → `make_triage_report.py`
→ `compare_ref.py --dir <lauf>`) + **MilkdropRef** (`tools/MilkdropRef/`,
Ground Truth um den Original-Kern; README dort = SSOT, Test-Presets IMMER
unter `asset/Milkdrop3/` ablegen). **S64: acht Fixklassen, OK 289→295,
Prüfstände jetzt SAATLOS** (Entscheid Patrik; `--seed` = App-Opt-in) —
Baseline `out/milkdrop_triage_s64f`: 16 auffällig = 5 korrekt PRESET-IST-SO
(R239/R239b, sprite3, XorDev 001b, acid wiring 2) + **11 Port-Bugs**.
**S67: Fixklasse 9 (EEL-Divisions-Vertrag) ⇒ OK 295→298** — neue Baseline
`out/milkdrop_triage_s67b`: exakt 3 Klassenwechsel, alle aufwärts (pixies ×2
SCHWARZ→OK, XorDev 001b MONOCHROM→OK; letzteres lebt bei uns jetzt lebhaft,
die Ref bleibt an ANDEREM UB schwarz — wandert in den Dunkelklasse-Entscheid).
`out/milkdrop_triage_s67a` = Zwischen-Gate: Shader-Leichen-Fix + Sicht-Blende
allein sind klassengleich zu s64f.
Diagnose-Schalter: `LUMIVIZ_MILKDROP_NOSEED`, `LUMIVIZ_MILKDROP_DUMP_WARP`,
NEU `LUMIVIZ_MILKDROP_TRACE_VARS` (Variablen-Trace nach per_frame).

| Was | Stand | |
|---|---|---|
| **🎯 Regelwerk-Strang (Entscheid Patrik S64, NÄCHSTE SESSION)** | Node-Param `Legacy \| Modern \| Benutzerdefiniert` (Master-Combo + Einzelschalter je Emulation: Div-Vertrag, UNORM-Trunc, q-Epsilon, UV-/NaN-Sanitize); Import-Default legacy, Neubauten modern. Dazu PS-Version-Override je Typ (warp/comp × auto/PS2/PS3/MD1-erzwingen = Strip-Bisektion als App-Feature). **Genauer Plan (R1–R5 + Abnahme): [Regelwerk_und_Neue_Module_Plan.md](visuals/Regelwerk_und_Neue_Module_Plan.md)** — dort auch Strang S (Shadertoy) und G (GPU-Module) | 🎯 |
| **🟠 Rest-Port-Bugs — S67-Stand: von 11 auf 8, davon 7 = Dunkelklasse** | **✅ pixies-Familie (2) GEHEILT (S67):** Ursache war der **EEL-Division-Vertrag** — die Referenz-EEL liefert bei Nenner 0 exakt 0 (dreifach per Sonde gemessen, `asset/Milkdrop3/sonden/`), unsere Lua-Engine IEEE-inf/NaN; die NaN-Kaskade (`cal=0 → 0.2/0 → cos(inf)`) zerstörte die Kamera-Matrix in f0 (Beweis: `LUMIVIZ_MILKDROP_TRACE_VARS`, reg30–38=NaN). Fix: EelTranspiler 1.3.0 emittiert `eel.div` (NUR Milkdrop-Dialekt; AVS wartet auf AvsRef-Sonde). searchlight 0,000→0,103 (Ref 0,054), 2000 shapes 0,000→0,111 (Ref 0,268) · **✅ piercing 01 GEKLÄRT — kein Port-Bug:** Rausch-Input `rand*0.0042` (±0,0021) liegt UNTER der 1/255-Quantisierung und stirbt an der UNORM-Trunkierung (auch im Original!) — das Preset braucht Fremd-Energie; die Ref zündet aus nicht-genulltem D3D9-Speicher, der saatlose Prüfstand kann das nicht. Mit `--seed` (App-Pfad): lebt, 0,406/max 0,812 ≈ Ref 0,306/0,794. Einstufung: Saatlos-Vertragsgrenze · **Gin Tonic 003 — S67 UNTERSUCHT, 🟡 Einstufung zur Absegnung:** Der „bass overbright"-Warp (`blur = sin(bass·90)·rad`, Periode 0,07 in bass) macht das Bild-Regime zentistellen-empfindlich auf die Bandwerte. BEIDE Renderer besitzen beide Regimes: unter `--silence` ist die REF weiß (1,000) und WIR moderat (0,34) — exakt invertiert zum Beat-Audio-Fall (Ref 0,035 dunkel, wir 1,000 weiß). Band-Pipelines (unsere Synthetik vs. Ref-FFT des PCM) können auf 0,07-Präzision strukturell nie übereinstimmen ⇒ **Empfehlung: IST-SO-artig (Audio-Statistik-Vertragsgrenze des Prüfstands), kein Renderpfad-Bug** · **q-/UB-Dunkelklasse (7+1): ✅ ENTSCHIEDEN (Patrik, S67) — IST-SO-artig (Legacy-Grenze wie R239)**: purple pulsator, crystal palace, R039, R068, R070, R211, R248 + XorDev 001b (invers: Ref stirbt an UB, wir leben) — Look hängt an doppeltem UB der Referenz (Heap-Garbage-q × Fixpunkt-Überlauf), nicht exakt reproduzierbar; aus der Bug-Liste raus. Formales Urteil: `out/milkdrop_triage_s67b/VERGLEICH.md` (R070/R248 mit Saat nahe Ref: 0,017/0,019 bzw. 0,008/0,022). **⚪ Optionaler Backlog-Pfad (Entscheid Patrik): erst Einzeldossiers (Screenshot-Paar + UB-Ursache je Preset), dann gezielte Emulation — NUR wenn Ressourcen übrig.** Verbleibender echter Rest-Bug: **Gin Tonic 003** (s. oben) | ✅ |
| ~~**Prüfpunkt R239/R239b**~~ | ✅ **aufgelöst (S64):** Prüfstände saatlos → beidseitig schwarz, korrekt PRESET-IST-SO. Die S63-Falle („OK geworden ≠ richtig geworden") ist damit methodisch beseitigt | ✅ |
| ~~**AVS-EEL-Divisions-Sonde**~~ | ✅ **gemessen (S67):** `asset/calibration/avs/sonden/` (make_div_sonde.py erzeugt Superscope-Sonden binär) — AvsRef: **2/0 = inf** (wie unsere rohe Lua-Division) ⇒ **AVS bleibt korrekt bei roher Division, das Dialekt-Gate in EelTranspiler 1.3.0 ist bestätigt.** ⚪ Bewusste Grenze entdeckt: `0/0` ist im alten evallib NaN mit x87-UNORDERED-Vergleichs-Quirk (equal(NaN,0)=1 UND above(NaN,1e6)=1 gleichzeitig — Ref malt Gelb, wir Schwarz, da IEEE-Vergleiche alle false); nur relevant für Presets, die auf 0/0-Ergebnissen vergleichen — kein Korpus-Befund bekannt, nicht emulieren | ✅ |
| ~~**myPresets/ (41)**~~ | ✅ **triagiert (S67, 4 Läufe `out/milkdrop_triage_mypresets_s67*` — Wurzel + 3 Unterordner, das Triage-Skript globbt nicht rekursiv):** 27 OK, 14 auffällig — alle 14 sind eigene GreatWho-WIP-Presets der **Energie-/Verstärker-Klasse**: saatlos dunkel, auch mit Einmal-Saat (Saat zerfällt, Presets brauchen Dauer-Energie/Erbe). Stichproben-Beweis: Spotlight V1 ist in der REFERENZ saatlos ebenfalls flach-dunkel (0,05) und enthält null Divisionen ⇒ keine Port-Bugs, App-Verhalten (mit Erbe) ist der relevante Test. Optional ⚪: Ref-Vollvergleich der 14, falls je gewünscht | ✅ |
| ~~**Loudness bei Stille**~~ | ✅ **geprüft (S67, Sonden 5–7):** Referenz bei Stille ebenfalls ≈1,0 (Bracket-Sonde: bass ∈ (0,9…1,1]), nur nicht BIT-exakt wie unsere Guard (1,000; `equal(bass,1)` feuert bei uns, in der Ref nicht). Materiell äquivalent; das Restrisiko exakter Band-Differenzen (0/0 in Presets wie gb003) ist seit `eel.div` (0/0 ⇒ 0) entschärft — kein Handlungsbedarf | ✅ |
| ~~**MilkdropRef-MessageBoxen**~~ | ✅ **erledigt (S67):** `patched/ref_msgbox.h` per `/FI` in alle Übersetzungseinheiten — alle MessageBoxen werden stderr-Zeilen + IDOK (nachdem die Falle erneut einen Sonden-Lauf blockierte). Merke: `data/include.fx` wird relativ `<presetordner>/../data` gesucht — Sonden nach `asset/Milkdrop3/sonden/` | ✅ |
| **MD3/2077-Feature-Frage (Patrik)** | erst nach dem MD2-Grundpfad: welche 2077-Presets nutzen MD3-exklusive Features, die uns fehlen? | 🟡 |
| **`rot_*`-Matrizen optisch** | seit S52 im Code (24 Matrizen + Matrix-Indizierung), 9 Presets im Pack betroffen. Werte sind zufällig und unser PRNG ist ein anderer als der von MilkDrop — prüfbar ist nur: laden sie durch, bewegt sich Plausibles | ⬜ |
| ~~**Texturen `worms`, `rose`, `grad3`**~~ | ✅ **beschafft (S60):** im Original-Winamp-Pack fehlen sie tatsächlich (21 Texturen, geprüft gegen `ref/winamp_orig`), aber der **MegaPack-`textures`-Ordner** (`…\VisualsPresets\MilkDrop 135k+ Presets MegaPack 2026\textures`, 8415 Dateien) hat alle drei — nach `asset/Milkdrop3/textures/` kopiert. Damit sind 27 von 35 Fehler-Log-Zeilen weg. Herkunft Community-Pack: die drei bleiben lokal — `asset/Milkdrop3/` ist per `.gitignore` ausgeschlossen, im Repo liegt nichts davon (geprüft S129). Bei künftigen fehlenden Texturen zuerst dort suchen (verwandt: „Assets-Ordner-Fallback" §7) | ✅ |
| **In-App-Sichttest-Runde** | c1- + m5-Presets über den Node-Pfad, Panel-Baum + Editor-Sektionen, Session-A-Features (Wave/Shape/Sprite anlegen/entfernen/klonen, Sprite-Editor, fShader-Wash). Offen seit S42 — [Status Punkt 0](visuals/MilkDrop_Import_Status.md) | ⬜ |
| **Decay-Dither** + **`.milk`-Export** | Punkt 8, ganz ans Ende | 🟡 |
| ~~**Playlist-Anbindung**~~ | ✅ **entfällt (bereinigt S129):** hing an E6, der Visual-Playlist — die ist geschlossen (Entscheid Patrik S93, C4 a, aufgegangen in Composer und Host Switcher; §6 „P2 Visual-Playlist") | ✅ |
| Host-Gruppen-Feinschliff | exakter paarweiser 2er-Mix statt sequentiellem Adjustable | 🔧 |
| **✅ Shader sehen y wie die Referenz (S130, war: 🔴 Warp-Shader sieht `uv.y` gespiegelt)** | Zerlegt mit neun Sonden (`asset/calibration/milkdrop/ysonde/`, Urteil am Bild mit `ysonde_auswerten.py` — die MAE meldete ein gespiegeltes Quadrat als OK). Ursache: alle Texturen (Puffer, Blur, geladene Bilder) liegen in GL-Lage, y nach oben. Der **Warp** bekam `uv` in GL-Lage (`1 − v` im Mesh) — Werte aus `uv.y`, `uv_orig.y`, `ang` und jeder senkrechte Versatz gespiegelt; der **Comp** bekam `uv` in D3D-Lage (S75), las damit aber direkt in den GL-Texturen — alles Gelesene stand **auf dem Kopf** (neuer Befund, Sonde `Y_shape_custom`). Mit beiden Fehlern heben sich die Spiegelungen bei Presets mit Warp UND Comp auf. Betroffen (Obergrenzen, 190 221 Presets): 40 037 Warp-Shader mit uv.y/uv_orig.y/ang, 141 439 Comp-Shader, die Puffer oder Blur lesen. **Behoben (Entscheide Patrik WP1 a, WP1-R a, WP2 b):** jeder Shader sieht den Raum der Referenz, jedes Lesen geht über `lv_koord` (Preamble), `TranspileOptions::referenzRaum` (HlslTranspiler 1.6.0: `tex2D` → `lv_tex2D`); Felder `warpYOriginal` / `compYOriginal` am MilkDrop-Knoten, Standard Original, Combos *Warp shader Y* / *Comp shader Y*. **Belegt:** Sonden 9/9 (bisher 5/9); Vergleichslauf 22 Presets × 6 Formate × 3 Marken × 2 Saaten: mit beiden Schaltern auf „bisher“ 748/748 bitgleich zu S129; Original: Starfield …2 / …2ps3 / …2ps3_exp nicht mehr FORMATABHÄNGIG (1:1 0,186 → 0,006, 16:9 0,078 → 0,004, 3,4:1 0,072 → 0,003), Twisted / Twisted b s0 f30 0,292 → 0,139, f120 0,26 → 0,14 (Streuung über die Startwerte jetzt wie die Referenz, 0,806 / 0,802), Playaround s0 f30 0,252 → 0,161, Neomtwister 3,4:1 0,217 → 0,166; Dancing Hearts unverändert (keine Shader — die ±0,01 sind Streuung der Referenz). Offen siehe nächste Zeile | ✅ |
| **✅ Shader-Eingänge rad/ang und Wrap von `sampler_main` wie die Referenz (S131, war: 🔴 Starfield (Supernova-Lens-Effect edit): LumiViz fast schwarz)** | Gesehen S130: Referenz ab f30 hell und farbig, LumiViz fast schwarz — in beiden Y-Ständen. **Zwei Ursachen, beide seit Stufe C1 (S40):** (1) `sampler_main` folgte `bTexWrap` (`MilkdropVisualizer.cpp`, `feedCustomUniforms`); die Referenz bindet `sampler_main` ohne Präfix **immer** mit Wrap (plugin.cpp:2962, milkdropfs.cpp:3839), `bTexWrap` wirkt dort nur auf den erzeugten Standard-Warp (plugin.cpp:8795 schreibt dann `sampler_fc_main`). Starfield liest im Comp `uv*2-1` und `uv1+texsize` — weit außerhalb, mit Clamp fiel fast alles auf eine Ecke. (2) `rad`/`ang` im Vorspann waren `length((uv_orig-0.5)*aspect.xy)` für beide Pässe; die Referenz gibt sie als Vertex-Attribute: Warp aus dem Mesh (plugin.cpp:2289/2293, x/y in −1..1, y nach oben — rad doppelt so groß), Comp aus `UvToMathSpace` (milkdropfs.cpp:3779, rad an den Ecken 1, ang 0..2π). Dazu der Nebenbefund aus S130: unter „Original“ war `ang` im Warp gespiegelt (uv_orig wurde vor der Rechnung nach unten gedreht). Reichweite (Textsuche, MegaPack 109 937 Presets mit Shadern): 44 376 lesen `sampler_main` bei `bTexWrap=0`, `rad` im Comp 14 870, im Warp 14 417. **Behoben (Entscheid Patrik S1 a):** dieselben Felder `warpYOriginal`/`compYOriginal` schalten je Pass alle drei Eingänge, Combos heißen *Warp shader input* / *Comp shader input*. **Belegt:** neue Sonden `R_warp`, `R_comp`, `W_comp` (`ysonde/`), Original 0,001, „bisher“ 0,057 / 0,193 / 0,125; alle zwölf Sonden OK. Vergleichslauf 22 Presets × 6 Formate × 3 Marken × 2 Saaten (`out/wp_s131/`): „bisher“ **748/748 bitgleich** zu S130; Original ändert 340 Bilder in 13 Presets (Starfield-Familie, Magma, Playaround, Twisted, Spotlight, Beauty) — Starfield f30 Kreis, Strahlen und Sterne wie die Referenz, Magma 640×480 f120 0,118 → 0,067, Magma 2 0,113 → 0,052, Playaround f30 0,161 → 0,141, Beauty bleibt OK. Die übrigen MAE-Sprünge (bis 0,27) sind Streuung der Referenz: Neomtwister und Dancing Hearts sind bei uns bitgleich, die Referenz weicht zwischen zwei Läufen um 0,30 ab. Rest offen: das Leuchten von Starfield hängt an einem Eckpixel des Puffers, hundertfach verstärkt — startabhängig, die Starfield-2-Familie bleibt mit Saat zu dunkel (MAE f30 0,45 → 0,50, im Bild jetzt das fleckige Muster der Referenz) | ✅ |
| **Flip X / Flip Y am MilkDrop-Host-Modul** | Wunsch Patrik S75 (oben unter dem Y-Achsen-Fix): zwei Optionsfelder als Gestaltungsmittel und Diagnosehilfe. Nicht gebaut — `flipx`/`flipy` gibt es nur an Sprites (`MilkdropVisualizer.cpp`). Bis 1.247.0 nur im erledigten Block vermerkt; offen, wann er eingeplant wird | 🟡 |

## 4. Sichttests, die nie stattgefunden haben

| Was | Umfang | |
|---|---|---|
| **Batch H — 9 Fraktal-Module** | Fractal 2D (9 Typen) · Fractal 3D (Raymarch-DE) · Domain Warp · Fractal Zoomer · Lyapunov · Kleinian · Strange Attractors · Flame/IFS · Reaction-Diffusion. Gebaut in S37, Unit-Tests grün, **GL-Sichttest komplett offen**. Kalibrierpunkte je Modul: Färbung/Banding, Kamera-Defaults, feed/kill, Reseed bei Divergenz, Punktzahl vs. Helligkeit. Querschnitt: Gradient-Paletten je Modul, Blend über die Kette, Audio-EEL-Reaktion | ⬜ |
| **FeedbackBuffer / `post.trail.*`** | Roadmap 4.3 — Sichttest (Trail-Look, Resize, Undock/Vollbild) + **Frametime-Vergleich** ausstehend. Einziges Modul-Doku mit offenem Status ([FeedbackBuffer.md](../include/visualizers/render/FeedbackBuffer.md)) | ⬜ |
| **BeatEstimator** | Roadmap 4.4 — Beat-Stabilität am laufenden Bild | ⬜ |
| **Multi-Drag zwischen Listen** | Block-Reparenting, Index-Mathematik nur compile-verifiziert; bei Bugs auf „Reorder in gleicher Ebene" beschränken | ⬜ |
| **Stereo `getspec`/`getosc`** | ch=1 (L) vs. ch=2 (R) getrennt? Nur compile-verifiziert. ⚠ **Hörtest**. Stellschrauben: `BASS_DATA_FFT_INDIVIDUAL`-Layout (`bin*chans+ch`) + Waveform-Interleaving | ⬜ |
| **Reset Start Folder** (S73) | Einstellungen › Panels › *Reset Start Folder* zeigt jetzt in den **Programmordner** statt in den Benutzerordner. Der Startordner-Weg ist am laufenden Programm belegt (Log), der **Knopf selbst wurde nie gedrückt**. Zwei Klicks: drücken, Browser muss im Programmordner stehen und `presets/` zeigen | ⬜ |
| **Presetverweis + Song-Slot** (S82) | Palette „Preset Reference…" mit .avs/.milk/.lvfx; Kette speichern → Datei enthält nur den Pfad; Quelle ändern → Kette neu laden zeigt die Änderung; relativer Pfad; Verweis im Host Switcher; Schalter Kopie ↔ Verweis. Song-Feld: z. B. SuperScope `Song: n=rand(200)+50;`, Titel im Player wechseln. Unit-Tests grün (4 + 1 neu). **✅ abgenommen S83** (Patrik: „die Sichttests sind gut, die Switch funktionieren so wie erwartet, auch mit den Preset-Referenzen") | ✅ |
| **Host Switcher** (S82) | Zwei bis drei Gruppen in die Liste, umsortieren, Startgruppe · Wechsel nach Zeit (Crossfade sichtbar?) · Songwechsel im Player mit und ohne Haken · Skript `nextgroup = above(beatcount,31)` und Song-Slot `switch = 1` · Gruppe umbenennen → Liste zieht mit, doppelter Name → Zähler · Speichern während des Laufs → Datei unverändert · Switcher-Auge aus → gespeicherter Zustand gilt. Unit-Tests grün (5 neu, davon 1 GL). **✅ abgenommen S83** (s. Zeile oben) | ✅ |
| **Preset as Group** (S82) | Palette „Preset as Group…" mit je einer `.avs`, `.milk`, `.lvfx` (mit und ohne eigene Host-Gruppen): Bild der Gruppe = Bild des Presets allein? AVS mit „clear every frame" (innere Liste) · „Aus Quelle neu laden" nach Änderung der Datei · Tiefenregel-Meldung beim Einfügen in eine Gruppe · Crossfade zwischen zwei Preset-Gruppen. Unit-Tests grün (5 neu). **✅ abgenommen S82** (Patrik: „funktioniert wie erwartet") | ✅ |
| **Preset-Zeile, Element-Presets, Shader-Vorlagen, Script active** (S83) | (1) Knoten-Preset wählen → Name bleibt; Regler ziehen → „(unsaved) – Name"; 🗑 bei eigenem Preset auch im „(unsaved)"-Zustand; Name/Beschreibung ändern → bleibt sauber. (2) Custom Wave/Shape einzeln markieren → Zeile *Preset*, z. B. *Pulse With Bass*, Editor bleibt in der Element-Ansicht. (3) SuperScope 3D/3D Camera: frühere Template-Namen stehen in der Preset-Zeile. (4) ⤢ an jedem Shader-Feld: *Template:* → Empty/Vorlagen, Strg+Z, *Save as template…*. (5) *Script active* aus → Bild wie ohne Skript, an → wie vorher; Host Switcher wechselt ohne Skript weiter nach Zeit. **✅ abgenommen S83** (Patrik: „Sichttests sind gut", Namen → englisch umgesetzt) | ✅ |
| **Milkdrop Code-/Parameter-Presets** (S83, Nachtrag) | Sektion *Parameter* markieren → *Old LumiViz Look (pre-S80)* laden: Bild zieht weich nach oben links, Skript-Zufall/Audio/Seitenverhältnis auf „bisher"; *Original* stellt alles zurück. Sektion *Code* → *Slow Rotation*, *Ripple (per pixel)*: nur das eigene Feld wird gesetzt. Unit-Tests grün (D3 + Feld-Wache), **Sichttest offen** | ⬜ |
| **Die 60 neuen Feld-Sonden** (S73) | Sonden für `gpuParticles`, `isfFilter`, `meshWarp`, `pixelFilter`, `shadertoy`, `videoSource`, `dotFountain`/`dotPlane`-`startRotation`, `milkdrop/puffer*` sind erzeugt und committet, aber **nie gelaufen**. Ein Sondenlauf zeigt, welche Felder wirken und welche STUMM melden — erwartungsgemäß findet sich dabei auch Testaufbau-Rauschen (§1c) | ⬜ |

## 5. Offene Entscheide

| Quelle | Entscheid | |
|---|---|---|
| ~~[Hotkey_Konzept §9](ui/Hotkey_Konzept.md)~~ | ✅ **entschieden (Patrik, S60):** §9.2 Stufe 1 blättert NICHT in Unterordner (nur aktuelle Ebene) · §9.3 am Verzeichnisende **umlaufen** · §9.4 Blättern wirkt auf den **zuletzt gewählten** Eintrag · §9.5 Menü-Einträge zeigen die Tasten. *(§9.1 in S52 entschieden: `Bild ab` = vorwärts.)* Umsetzung/Konzept-Nachzug bei der nächsten Hotkey-Arbeit (nach §0) — **nicht geschehen** (S129 geprüft): `ImportBrowserPanel::nextPresetRow` hält am Ende an („nicht umlaufen“, Stand S51, vor dem Entscheid), `Hotkey_Konzept.md` steht auf „§9.2–§9.5 offen“; als 🔧 in §6 geführt | ✅ |
| ~~[Visual_Playlist §6](ui/Visual_Playlist_Konzept.md)~~ | ✅ **entschieden (Patrik, S60):** **Pfad-Referenzen** statt eingebetteter `.lvfx` (kaputte Pfade beim Laden melden) · **Auslöser-Default: KEIN automatischer Wechsel** (manuell; Songwechsel/Timer sind opt-in) · Timer-Wechsel, wenn aktiviert, **beat-quantisiert** (nächster Beat) · Import-Browser-Erweiterung **erst mit der Playlist**. P2 damit entsperrt, Umsetzung nach §0 | ✅ |
| ~~[Lights_Module_Entwurf](visuals/Lights_Module_Entwurf.md)~~ | ✅ **entschieden (Patrik, S60): nach §7 (Backlog) verschoben** — BASS-Lookahead-Service (`AudioLookahead`) erst mit den Lights-Modulen; bis dahin reichen Beat-Prädiktion + `gettime()` | ✅ |
| [Parameter_Reference §10](visuals/Parameter_Reference.md) | Deklarierte Preset-Defaults vs. Dropdown-Indizes bereinigen · `solidColor`/`peak.color.fixed` ohne deklarierten Default | 🔧 |
| ~~**60 neue Feld-Sonden**~~ (S73) | ✅ **entschieden (Patrik, S73): mitgenommen.** Der Lauf von `make_field_probes.py` (nötig für den Pfad-Fix) hat nebenbei Sonden für Knotentypen erzeugt, die es seit S69–S72 gibt und für die nie welche existierten: `gpuParticles`, `isfFilter`, `meshWarp`, `pixelFilter`, `shadertoy`, `videoSource`, `dotFountain/startRotation`, `dotPlane/startRotation`, `milkdrop/puffer*`. Keine lokalen Pfade darin. **Sie sind noch nie gelaufen** — ob sie etwas messen oder STUMM melden, ist offen; das entscheidet erst ein Sondenlauf (§4) | ✅ |
| ~~MilkDrop-Texturen~~ | ✅ beschafft aus dem MegaPack (S60), siehe §3 | ✅ |
| ~~**Multi Delay: wem gehört die Verzögerung?**~~ (Befund S55) | ✅ **entschieden und umgesetzt (Patrik S55): Puffer-Besitz, original-treu.** Der Ausgabe-Knoten liest jetzt den **ältesten** Frame des Rings (`head`), sein eigenes `delay` wirkt nicht mehr — wie `outpos[buffer]` im Original. Bis dahin änderte das `delay` des Schreibers nur die Ringgröße und blieb unsichtbar. Vorstand gemessen: ungleiche Werte kann es im Original **gar nicht geben** (jeder Knoten speichert alle sechs Puffer-Einstellungen und schreibt sie global, `r_multidelay.cpp:387-401`), im Referenz-Korpus nutzen **2** Presets den Effekt, in eigenen `.lvfx` nur die Sonden | ✅ |
| ~~**Colorfade: `enabled`-Bitfeld fehlt**~~ (Befund S55) | ✅ **erledigt S57, Zeile war nur nie abgehakt (bemerkt S60):** Colorfade ist nach `r_colorfade` portiert — Bitfeld beim Import, persistenter Fader-Zustand, Annäherungsrampe, drei exklusive Beat-Zweige; alle 12 Felder wirken (§1-Zeile „Colorfade-Zufalls-Beatmodus") | ✅ |
| ~~[Config_Pipeline_Umsetzungsplan](visuals/Config_Pipeline_Umsetzungsplan.md)~~ | ✅ **entschieden + umgesetzt (S60):** die Abnahme-Tabellen A1–A8/N1–N7 sind **als erledigt gestrichen** (Vermerk im Dokument; die `⬜` bleiben als Historie stehen) — Schritte 0–7 ✅ seit S30, Sichttests 5.1–5.5 abgenommen | ✅ |

## 6. Konzept-Phasen, noch nicht begonnen

- 🟡 **Loop — Drums und später Instrumente als Arrangement in Takten** (Wunsch
  Patrik S108, **im Bau**). Steuerdokument:
  [Loop_Konzept.md](audio/Loop_Konzept.md) — Entscheide L1 bis L8, Stufen,
  Beleg. Im Loop-Panel legt man Kanäle an, darin Drumkit-Clips; die Editoren
  sind eigene Fenster; die Datei `.lvloop` ist das ganze Arrangement und
  lässt sich als Clip in einen Audio-Kanal des Composers legen. **Stand
  S110: dazu die Stufen 9a bis 10b** — Instrument-Clips, Instrument-Editor,
  Instrumente als Dateien, Instrument-Creator (Konzept 0.19.0, unten je
  Stufe); Patrik hat den Loop aus S108 / S109 im Programm gesehen („sieht
  gut aus"), ohne Testlog — alles aus S110 ist ungesehen. **Stand
  S109: die Stufen 6a bis 8 sind gebaut** (Konzept 0.12.0) — die gemeinsame
  Basis `lumi::ui::Achse` aus der Zeitachse des Composers (Entscheid W1 b),
  das Loop-Panel als Arrangement mit Undo (6a), der Drum-Editor als eigenes
  Fenster mit *Play pattern* und *Peak* (6b), der Audio Browser zeigt
  `.lvloop` (6c), der Loop als Clip im Composer mit Tags und Verbindung zum
  Loop-Panel (7; Entscheide T1 a und T2), Benutzerhandbuch §16,
  `Audio_System.md` §6a und eine Sichttest-Vorlage (8). Tests 1025 / 1025.
  **Im Programm nicht gehört und nicht gesehen — keine der Stufen seit S108.**
  Offen:
  - ⬜ **der erste Blick ins Programm:** Liste
    `Sichttest_Composer_10_Loop.md` (lokal, seit S110 87 Punkte: Panel Loop,
    Drum-Editor, Instrument-Clips, Audio Browser, Loop als Clip, 15 Urteile). Die Liste ist
    aus dem Code geschrieben, nicht aus dem laufenden Programm — stimmt ein
    „Erwartet" nicht, kann es an der Liste liegen. Der erste Hörtest ist
    dort Punkt L3
  - 🟡 **von Claude gesetzt, nicht entschieden** (Konzept §4c, §4d, §4e;
    als Urteile in der Liste): Farbe je Basis statt je Kanal; ein Clip
    springt beim Verschieben über einen anderen; die Lautstärke je Schritt
    in eigener Zeile; *Play pattern* spielt nur das Muster; das Player-Panel
    zeigt einen Durchlauf als Länge; ein Loop-Clip rastet wie ein Song,
    nicht auf Takte
  - ✅ **beim Beenden fragt das Loop-Panel nach Ungespeichertem** (S110) —
    Save / Discard / Cancel wie im Composer, dieselbe Frage jetzt auch vor
    New und Load (vorher nur Discard / Cancel); ein Fall in
    `test_LoopArrangement.cpp`. Den Dialog selbst hat noch niemand gesehen
    (Liste 10, L17 und L18)
  - 🟠 **Nachsetzen der Tonführung bei gedrosseltem Rechner:** gemessen ist
    nur der ruhige Fall (Abstand zur Uhr −8 bis +4 ms, Drift 14 bis 35 ppm,
    kein Nachsetzen in 40 s; Konzept §6a). Was in den Schüben geschieht, in
    denen der Rechner drosselt, ist ungemessen; wäre es zu hören, ist der
    nächste Schritt, nur an einer Durchlauf-Grenze nachzusetzen
  - ✅ **Aktionen der Sichttest-Steuerung für das Panel Loop** (S110):
    `loop_laden`, `loop_wiedergabe`, `loop_editor`
    ([Sichttest_Steuerung.md](Sichttest_Steuerung.md) §3); Liste 10 nutzt sie
    in L0 und E0. Mit dem Tester noch nicht ausgelöst. Handarbeit bleibt: den
    Ordner im Audio Browser wählen (B0)
  - ✅ Menüs, Dialoge (Tags, Umbenennen) und das Ziehen an den Schiebern des
    Editors sind getestet (S125, `test_LoopMenues`: Taktachse, Schritt- und
    Notenraster, Spur, Rolle, Add track, Tags..., die Schieber; Menüs und
    Dialoge laufen dafür über `UI/widgets/Bedienhilfe`). 🔧 Ohne Test bleiben
    das Hineinziehen eines Loops über das echte Composer-Panel und die
    Anzeige der Tags in Player-Panel und Karaoke.
    ✅ Ein eingefügter Loop-Clip ist einen Durchlauf lang und lässt sich
    beliebig länger ziehen (`fuegeSongEin`, Test in `test_LoopSpieler`) — von
    Patrik im Programm gesehen (S125)
  - ✅ ein Loop, der allein im Loop-Panel spielt, meldet sich als Song (S127,
    Entscheid 7a: beim Play des ganzen Loops, nicht bei Play pattern);
    ✅ Kanäle und Spuren lassen sich umsortieren (S127, Entscheid 7b a: am
    Namen ziehen oder Rechtsklick Move up/down). ✅ Seit S120 das Icon für
    `.lvloop`; ✅ seit S125 merken Drum- und Instrument-Editor Größe und Lage
    (`loop/drumEditorGeometry`, `loop/instrumentEditorGeometry`; in der
    Test-Exe abgeschaltet, `test_LoopEditorLage`)
  - ✅ `Composer_Konzept.md` nachgezogen (S110, §9f): die Basis `Achse`, der
    Loop-Clip in der Tonführung. ✅ S128 (N1 b): nur noch der Composer hängt
    am Loop — der Loop fragt über `loop/ComposerAbfragen`, die Farbreihen
    liegen in `UI/widgets/Farbreihen`; ein Test hält die Richtung fest
  - ✅ **Haken Karaoke im Composer nach dem Start leer, obwohl die Ebene an
    war** (Befund Patrik S110): das Hauptfenster setzt den Schalter jetzt vor
    dem Aufbau der Panels. Aus der Reihenfolge im Code gelesen und behoben,
    im Programm nicht nachgeprüft
  - 🟡 **9 Instrument-Clip** — **Entscheid I1 c (Patrik S110):** Instrumente
    gerechnet oder aus Samples, zuerst gerechnet. **9a gebaut (S110):** Noten
    in Modell und Datei, sieben gerechnete Instrumente (Piano, Strings,
    Violin, Guitar, Bass, Organ, Lead), Noten im Rechenkern, Beispiel
    `asset/loop/instrument_demo.lvloop` (Konzept §4f). **Nicht gehört:** die
    Instrumente sind nach Rechnung eingestellt — ⬜ Patriks Urteil an den
    Hörproben (`LUMIVIZ_INSTRUMENT_WAV=<Ordner>` am Fall „instrument_demo")
    oder im Panel Loop mit dem Beispiel. **9b gebaut (S110):** der
    Instrument-Editor nach der gebilligten Skizze (Klaviatur über sechs
    Oktaven, Noten setzen, verlängern, verschieben, löschen, Lautstärke je
    Note, eine Taste spielt den Ton an) und **New instrument clip**;
    Benutzerhandbuch §16. **Im Programm nicht gesehen und nicht gehört** —
    ⬜ Liste 10, Abschnitt I (22 Punkte, I2 ist der erste Hörtest der
    Instrumente) und die Urteile U9 bis U15; von Claude gesetzt und als
    Urteile offen (Konzept §4f): das Anspielen pausiert den Player, die Zeile
    unten stellt alle Noten eines Schritts, kein Zoom, das Raster des Loops
    steht nur im Drum-Editor. 🟠 Ungemessen: beim Bearbeiten im laufenden
    Loop lädt der Spieler bei jedem neuen Ton alle Klänge neu. Danach **10**
    Instrument-Creator und **11** Instrumente aus Samples
  - 🟡 das Kit ist klein (Patrik S108: klingt gut, aber eine kleine
    Sammlung) — welche Klänge dazukommen, nennt Patrik
  - 🟡 **10 Instrument-Creator** — Entscheide Patrik S110: I2 b (ein eigenes
    Instrument ist eine Datei `.lvinst`), I3 a (auch die sieben
    mitgelieferten, unter `asset/loop/instruments`), Skizze gebilligt.
    **10a gebaut:** das Format, die Dateien, ein Muster nennt ein eigenes
    beim Pfad, dazu sieben Instrumente für House (Wunsch Patrik) mit dem
    Beispiel `asset/loop/house_demo.lvloop`. **10b gebaut:** das Fenster,
    *Edit instrument...*, *Load instrument...*, Anspielen aus dem Creator;
    Benutzerhandbuch §16. **Im Programm nicht gesehen und nicht gehört** —
    ⬜ Liste 10, Abschnitt K (Creator) und Urteile U16 bis U18; die
    Einstellungen aller vierzehn Instrumente sind gerechnet, nicht abgehört.
    Nicht drin: hören, was man einstellt, während das Muster spielt; eine
    Resonanz am Filter (der Acid Bass hat keine)
  - ✅ **10c Instrumente aus mehreren Schichten** (Wunsch Patrik S110) —
    **S111 gebaut:** Skizze gebilligt, Entscheide **F1 a** (Fassung 2 der
    `.lvinst` mit der Liste `layers`) und **E1 a** (Verzerrer im Instrument,
    Hall und Echo am Muster); eine bis vier Schichten, je Schicht Level,
    Transpose, Start offset und Drive, im Creator die Liste Layers, zwei
    Kurven je Bild, Selected layer only; dazu sechzehn Instrumente aus
    Schichten (sieben ungewöhnliche) und fünf Beispiel-Loops (Wunsch Patrik
    S111). **Im Programm nicht gesehen und nicht gehört** — ⬜ Liste 10,
    Abschnitt S und Urteile U19 bis U23; alle Einstellungen gerechnet, nicht
    abgehört. Nicht gemessen: die Rechenzeit mit vier Schichten. Dazu
    **neunzehn Beispiel-Loops nach Genre** (`asset/loop/genres`, Trance bis
    Metal) und neunzehn Instrumente dafür (neunundvierzig insgesamt) —
    ⬜ Liste 10, S19 bis S21 und U24; das Kit hat nur sechs Klänge, Cowbell,
    Conga und Clave sind deshalb Instrumente (🔧 ein größeres Kit wäre der
    sauberere Weg). ✅ S128 (C1 a): `lib:cowbell`, `lib:cowbell_tip`
    (OLPC) und `lib:clave` (ESMUC, CC0) sind mitgeliefert; ✅ C2 a: die
    Instrumente `cowbell`/`clave` spielen sie (Grundton 81 bzw. 84), Kits
    Reggaeton und Hand drums haben sie als perc; ⬜ Patriks Ohr an
    `.claude/handover/hoerproben_s128/` (vorher/nachher)
  - ✅ **12 Effekte am Clip** (S113; aus „Hall und Echo am Muster", E1 a) —
    **gebaut:** Entscheide **E2 b** (jeder Clip im Loop-Panel trägt eigene
    Effekte, für Drumkit- und Instrument-Clips gleich), **F2 a** (Feld
    `effects` am Clip, `.lvloop` in Fassung 3), **G1 b** (die Effekte rechnen
    im Ton-Thread mit; nach einem Sprung über 200 ms beginnt der Nachklang
    neu). Distortion, Echo, Reverb je Clip; das Fenster **Clip effects**
    (Rechtsklick → Effects...), die Marke **fx**; mehrere Clips markieren
    (Ctrl+Klick, Rahmen, Ctrl+A), als Gruppe ziehen, kopieren, löschen;
    Beispiel `asset/loop/effects_demo.lvloop`. **Im Programm:** Patrik hat
    F0 bis F4 der Liste 10 geprüft — „alle ok" (im Gespräch, ohne Testlog).
    ⬜ Liste 10, F5 bis F17 und Urteile U25 bis U31. Alle Werte der Effekte
    und des Beispiels sind gerechnet, nicht abgehört. 🔧 Nicht gemessen: die
    Last im Ton-Thread selbst und mit vielen Clips (gemessen ist eine Kette
    außerhalb: rund 0,2 % eines Kerns unter Release-clang); nicht geprüft:
    ein Loop-Clip mit Effekten im Composer (kein eigener Testfall, in der
    Liste F17); ✅ die Menüs des Rechtsklicks (S125, `test_LoopMenues`). Von Claude gesetzt und nicht
    entschieden: alle Grenzen und Vorgaben, die Bauart des Halls (Freeverb),
    Mix blendet über statt dazuzumischen, ein verschobener Clip verliert
    seinen Nachklang — steht in `Loop_Konzept.md` §4 und §4g
  - ✅ **13 Mehrfachauswahl in Drum- und Instrument-Editor** (Wunsch Patrik
    S113) — **gebaut:** Entscheide **V1 a** (Einfügen im Drum-Editor
    ersetzt), **V2 a** (nur die markierten Schritte zählen), **V3 a**
    (Pfeiltasten halten am Rand an), dazu Alt+Pfeil dreht im Kreis.
    Shift+Ziehen und Ctrl+Klick markieren, Ctrl+C / X / V an der
    Einfügemarke, Ctrl+D, Entf, Pfeiltasten; im Instrument-Editor mehrere
    Noten als Gruppe ziehen; in beiden der Haken **Right-click deletes**
    (anfangs aus — **im Instrument-Editor löscht der Rechtsklick seither
    nicht mehr von selbst**). **Im Programm:** Patrik hat M0 bis M4, M12 und
    M13 der Liste 10 geprüft — „sind ok" (S114, im Gespräch, ohne Testlog).
    ⬜ Liste 10, M5 bis M11, M14 bis M16 und Urteile U32 bis U36. ✅ Die Menüs
    des Rechtsklicks sind getestet (S125). 🔧 Nicht geprüft: ob die Tasten im
    gezeigten Fenster beim Raster ankommen
  - 🟡 **11 Instrumente aus Samples** — als Art von Schicht, mit Root note,
    Fine tune und Detect (Idee S113, von Patrik nicht verworfen;
    `Loop_Konzept.md` §4h). **Multisamples** vorgemerkt: vorziehen, wenn Tests
    an Sample-Instrumenten zeigen, dass ein Sample über mehrere Oktaven nicht
    trägt (Patrik S113). Nächster Schritt: die Skizze
  - 🟡 **14 Palette der Instrumente** (Entscheid **P1 a**, S113): ein Fenster
    wie das Palettenfenster der Effektkette statt der Dropdown-Liste, mit
    Filtern nach **Genre**. **Skizze gebilligt (Patrik S114)**, Entscheid
    **F3 b**: Art, Genres und Beschreibung als Felder in der `.lvinst`,
    Fassung 3; **P2 a**: eigene Instrumente als `user:<name>` im
    Benutzerordner, nichts wird verdrängt (ein Schalter „Replace the
    built-in one for all loops" vorgemerkt, nicht gebaut). **Gebaut 14a** —
    das Modell, Fassung 3, die 49 Dateien über
    `asset/loop/instrument_katalog.py` (Art und Beschreibung nach Titel und
    Aufbau, Genres aus den Loops gezählt; Guitar und Violin ohne Genre),
    eigene Instrumente im Modell (damit auch Datenablage A3 im Kern). Im
    Programm nichts zu sehen. **Gebaut 14b** — im Creator die zweite Zeile
    Type, Genres, Description; **Save** legt bei einem mitgelieferten eine
    eigene Kopie unter einem Namen in den Benutzerordner (GEÄNDERT: war
    grau), Save as... bleibt für einen freien Ort (**S117 GEÄNDERT**,
    Entscheide E1 a und E1b c: Save as... fragt nur nach dem Namen und
    schreibt nach `instruments`; ein Original wird nie überschrieben, der
    Dialog sagt es. E2 a: die drei losen `.lvinst` aus der Wurzel von
    Patriks Benutzerordner liegen unter `instruments`. E3 a: Load
    instrument... fragt, ob die Datei zu den eigenen kopiert wird oder am
    Ort bleibt. Alles aus S117 im Programm nicht gesehen — Liste 10, K9,
    K9b, K9c). Im Programm nicht
    gesehen; nicht geprüft: der Dialog „Save instrument" und die Vorschläge
    der Genres im gezeigten Fenster. **Gebaut 14c** — das Fenster
    **Instruments** (`loop/InstrumentPalette`): Suche, Filter nach Genre,
    Favorites, Recently used, die acht Arten, My files, Anspielen per Klick
    mit Preview note, Use; im Instrument-Editor **Browse...** statt der
    Dropdown-Liste (GEÄNDERT). **Stufe 14 ist gebaut.** Im Programm von
    Patrik geprüft (S115, im Gespräch, ohne Testlog): **P0 bis P3 ok**;
    **P5 Befund** — Ctrl+Z nahm nach Use nichts zurück, erst nach einem Klick
    ins Panel Loop; behoben nach Entscheid Z1 a (Ctrl+Z und Ctrl+Y gelten im
    Fenster selbst), **P5 danach von Patrik neu geprüft: ok**. Alles andere
    ungesehen — ⬜ Liste 10,
    Abschnitt P (P4, P6 bis P14) und Urteile U37 bis U41; ⬜ ob Ctrl+Z im
    Instrument-Creator ankommt (hängt genauso am Editor, nicht geprüft).
    Nicht geprüft: das Menü des Rechtsklicks, die
    Tasten, das Fenster neben dem Editor. Von Claude gesetzt und nicht
    entschieden: die Zuordnung der 49 nach Art und ihre Beschreibungen, die
    Genres aus den Loops gezählt (Guitar und Violin ohne), ein eigenes
    Instrument steht nur unter My files, Preview note je Art. 🔧 Vorgemerkt:
    der Schalter „Replace the built-in one for all loops" (P2 c)
  - 🟡 **Datenablage** (S114; Steuerdokument
    [Datenablage_Konzept.md](core-services/Datenablage_Konzept.md), Entscheide
    D1 b bis D10): **gebaut A1** — eine Stelle für den Ordner eines
    mitgelieferten Bereichs (`core/Mitgeliefert`), sieben Sucher rufen sie;
    zuerst die Kopie neben der Exe, in den Settings umschaltbar auf den
    Projektordner (nur im Entwicklungsbaum sichtbar, gilt nach Neustart) —
    und **A2** — `asset/Asset.cmake` listet, was neben die Exe kommt
    (`nodepresets`, `shadertemplates`, `karaoke`, `composer`, `loop`,
    `img/logo/icons`, `presets`), der Abgleich läuft bei jedem Bau von LumiViz
    (421 Dateien, unverändert rund eine Sekunde, einmal gemessen).
    **Im Programm nicht gesehen:** ob LumiViz die Kopie liest (Log-Zeile
    „Bundled data …" beim Start), die Zeile „Bundled Data" im Settings-Panel.
    **Gebaut A4a** — der Kern von Export und Import ohne Fenster: miniz
    3.1.2 als lokales External mit eigener `Include.cmake` im Projekt
    (E2 b), `core/ZipArchiv`, `core/Datensicherung` (neun Bereiche, seit S115 zehn mit „Samples", vier
    Regeln bei gleichem Namen, feste Liste der Einstellungen, E1 a); im
    und **A4b** — die Dialoge: im Settings-Panel **User Data: Open Folder,
    Export..., Import...**, „Export your data", „Import data" und die Liste
    „Let me decide for each file". Im Programm von Patrik geprüft (S115, im
    Gespräch, ohne Testlog): **D1 und D3 ok** (der Ordner `asset` neben der
    Exe; Icons, Voreinstellungen und Instrumente sind da). Alles andere
    ungesehen — ⬜ lokale Liste `Sichttest_Datenablage.md` (D2, D4 bis D14). ✅ Offene Listen
    des Loops lesen nach einem Import neu (S125: `UserDataImportedEvent` nach Import data und
    Import package; das Panel Loop vergisst gerechnete Töne, Samples und Formen und lässt
    Spuren, Fenster Instruments und Fenster Kits neu lesen; `test_LoopNachImport`).
    🔧 Weiter offen: die Listen der Visuals (Presets, Gradients, Module presets, die
    Formen am Knoten Shape) hören nicht darauf; ganze Spalten in der Liste über drei
    Knöpfe statt Haken im Kopf.
    **A3** — eigene Instrumente im Benutzerordner — ist mit Stufe 14 des
    Loops gebaut; damit ist die Datenablage gebaut.
    🔧 Vorgemerkt (Frage Patrik S114): ein Instrument trägt höchstens vier
    Schichten (`kSchichtenMax`, von Claude in S111 gesetzt, nicht gemessen)
    — vor einem Anheben die Rechenzeit je Ton mit vier, sechs, acht messen.
    **Gemessen und probiert (S120, Frage Patrik: trägt ein Instrument auch
    acht?):** ja. *Rechenzeit* (`test_LoopBausteine.cpp`, Fall „Loop
    Instrument Messung", je Schicht Säge mit drei Stimmen, Filter, Drive,
    langem Ausklang): sie wächst gerade mit der Zahl — unter Testing-MSVC
    4,5 / 19,5 / 28,9 / 38,8 ms je Sekunde Ton bei 1 / 4 / 6 / 8 Schichten,
    unter Release-clang 2,0 / 8,3 / 12,1 / 16,3 ms; ein Ton wird vorab
    gerechnet, acht Schichten kosten also rund 16 ms je Sekunde Ton im
    ausgelieferten Bau. *Mit der Grenze probeweise auf 8* (nicht
    committet): von 150 Fällen zu Loop, Instrumenten, Drum und
    Sichttest-Aktionen fallen nur die zwei durch, die die Grenze selbst
    prüfen (sie zählen bis vier); die festgehaltenen Werte der mitgelieferten
    Instrumente bleiben gleich; der Creator zeigt acht Schichten in der
    Liste, im Bild aus dem Test ohne Rollbalken. **Zu beachten vor einem
    Anheben:** (1) die *Spitze* wächst mit — 0,41 / 1,31 / 1,98 / 2,52 bei
    allen Level auf 1; schon vier Schichten übersteuern, der Creator zeigt
    die Zahl (Peak), nimmt aber nichts zurück; (2) eine *Datei* mit mehr als
    vier Schichten läse ein älteres Programm still gekürzt — das verlangt
    eine neue Fassung der `.lvinst`, sie fiele mit der für Pan zusammen
    (Fassung 8, `Shape_Konzept.md` §6a); (3) bei der Form: As layers reichte
    dann für acht Konturen. **Entschieden (Patrik S120): die Grenze geht auf
    acht** — mit Stufe 18 (Pan je Schicht), in `.lvinst` Fassung 8;
    **gebaut in S120**, im Programm ungesehen. 🟠 **Befund:** in
    der Konfiguration Testing unter MSVC fehlt `/MD` bei LumiViz, Core und
    miniz — die Objektdateien verlangen `LIBCMT`, Qt ist `/MD`; seit miniz
    meldet der Linker es an `LumiViz.exe` (LNK4098, LNK4217). Bestand
    vorher, Ursache nicht untersucht (die Schalter der Konfiguration
    Testing sind leer, auch ohne Optimierung) — an CMakeCraft melden. 🔧 Im Nachgang,
    ohne Priorität: Beispiele aus `examples`, `composits`, `shadertoys`,
    `effectchain` neben die Exe (je Ordner erst die Herkunft prüfen);
    ~~Icons für `.lvloop`, `.lvinst`, `.lvdata`~~ (seit S120); Wunsch an CMakeCraft: Daten-Ordner
    einer App als Angabe in `Solution.json`. Folge von D9 a: **eine unter
    `asset/` geänderte Datei sieht `LumiViz.exe` erst nach einem Bau**
  - ⚪ **15 Wave-Clips (Tonspuren wie Gesang) und Autotune am Clip** —
    zurückgestellt (Patrik S113: Autotune darf warten, bis es Wave-Clips gibt)
  - 🟠 **Rückmeldung Patrik zum Instrument-Editor (S110):** Tasten anspielen
    und Notenlängen ziehen gingen bei ihm nicht — beides war gebaut.
    Nachgebessert (Griff, Mauszeiger, Hinweis bei stummen Tasten); **nicht
    geklärt, woran es lag** — nachfragen
  - ✅ das Kit und die Instrumente liegen seit S114 als `asset/loop` neben
    der Exe (Datenablage A2). 🔧 Weiter offen: Debug-MSVC seit S108 nicht gebaut, der volle Lauf nie unter clang; ob
    `mixerRemoveSource` auf einen laufenden Rückruf wartet, ist nicht
    geprüft (die Tonführung lässt den Klangerzeuger deshalb vier Takte
    weiterleben)

- 🟡 **Block F — Modulparameter auf dem generischen Weg skriptbar machen**
  (angesetzt direkt nach S83, Entscheid Patrik E2b: kein Einzel-Nachtrag in
  `ScriptVariables.cpp`, sondern gleich der allgemeine Weg). **Konzept S84:**
  [Feldsteuerung_Konzept.md](visuals/Feldsteuerung_Konzept.md) — Feldtabelle
  (erzeugt, Zeiger-auf-Member) als SSOT, ein generischer Läufer in
  `renderNode`, Strukturfelder, Modulation ohne Formel; Entscheide E1–E6
  gefallen (S84, E6 präzisiert S85: Ganzzahlen abgeschnitten). **Stand S85:**
  F1, F2, F5, F2-Migration (51 Renderer ohne Feld-Hand-Listen), F2b (alle
  19 eigenen Hosts an der Tabelle, Regler kompilieren nicht mehr neu) und F4
  (Modulation ohne Formel, „~"-Knopf, 257 Felder in 56 Modulen) ✅; **S86:**
  F4c ✅ — SuperScope, Effect List, Host-Gruppe, Dynamic Movement, Text, AVI,
  Debug Bars modulierbar (17 Felder, Vergleichslauf 84/84 bitgleich); Effect
  List belegt `alphain/alphaout` je Frame vor wie AVS (35 Presets 210/210
  bitgleich); F2c ✅ — **SuperScope und Effect List: alle Tabellenfelder per
  Skript** (Vergleichslauf 294/294 bitgleich, +1,5 % Laufzeit); Klemmfehler
  bei Auswahlfeldern ohne Tabellenbereich behoben; `startIndex` = Zustand,
  Render Scale `divisor` = Struktur; F2d ✅ — Dynamic Movement, Host-Gruppe
  und Host Switcher skriptbar (Vergleichslauf 294/294), Host Switcher
  `intervalSeconds` modulierbar; F2e ✅ — Text und AVI mit eigenen
  Init/Frame/Beat-Feldern — **offen:** Sichttest Text-/AVI-Skript (AVI ohne
  Bildsonde); Movement, Render Scale, Debug Bars, Color Modifier `recompute`
  bleiben ohne (Entscheid Patrik S86); F4b ✅ — Farbfelder modulierbar
  (26 Felder, Helligkeit/Farbton/Kanal; im Bild nur am Clear-Knoten geprüft
  — Sichttest an weiteren Knoten offen); Effekt-Palette um Text, AVI,
  Comment ergänzt (Fund Patrik S86, Wächter-Test); **Namensfang-Prüfung über die
  ganze Sammlung für alle seit S84 gebundenen Typen** (`namensfang.py` im
  Scratchpad S86 — bisher nur SuperScope, Effect List, Dynamic Movement
  geprüft; Fund: DM `blend`); einmalige Bildabweichung bei
  `fUk - afterburner remix` (1 von 12 Läufen, nicht reproduziert);
  F4d ✅ (S86) — LFO-Rate in Beats („Beat sync", eigene Beat-Uhr; Sichttest
  mit echter Musik offen); Voreinstellungen für Text (7) und AVI (5); F6
  (MIDI/OSC-Adresse, mit der Idee).
  Farbtafeln/Stützstellen (`colors[i]`) weiter §7 des Konzepts.
  Die Tabelle unten ist der Stand VOR Block F (Historie). **Vorarbeit S83 —
  Bestandsaufnahme über alle Palettentypen:** was im Editor steht, aber keine
  Skriptvariable hat (Stand `HEAD` 80201b3):

  | Gruppe | Typen · nicht skriptbare Felder |
  |---|---|
  | Modus-/Beat-Schalter | customBpm `arbitrary/skip/invert` · onBeatClear `blend` · starfield/blitterFeedback/mosaic/bump/interferences/multiFilter/channelShift/interleave `onBeat` · mirror `onBeatRandom/smooth` · colorfade `slowFade/onBeatRandom` · waterBump `randomDrop` · reactionDiffusion `seedOnBeat` · rotoBlitter `beatReverse/beatReverseSpeed/beatZoomJump` · videoDelay/multiDelay `useBeats` |
  | Blend/Subpixel/Wrap | `blend` bei bufferSave, movingParticle, starfield, metaballs3d, tentacles3d, dynamicShift, dynamicDistanceModifier, blitterFeedback, rotoBlitter, mosaic, bump, interferences, interleave, grain, fractal2D/3D, domainWarp, lyapunov, kleinian, strangeAttractor, flame, reactionDiffusion · `subpixel`/`wrap`/`rectCoords` bei den Movement-Typen |
  | Typ-/Qualitätswahl | fractal2D `type/smooth` · fractal3D `type/maxSteps/maxIter/ao` · fractalZoomer `type/maxIter` · lyapunov `sequence/warmup/iterations` · kleinian `iterations` · strangeAttractor `type/points/useGradient` · flame `functions/points` · multiDelay `mode` · renderScale `divisor/filter` · jherikoGlobal `loadMode` · terrain3d `resolution/drawMesh/drawDots` · superScope3d `renderMode/audioChannel/spectrumSource` · bloom `downsample/vignette/post` |
  | Farben/Verläufe | `gradientPreset` überall (Fraktal-Familie, SuperScope) · Farbtafeln `colors` (superScope, simpleScope, oscStar, oscRing, rotatingStars, metaballs3d, tentacles3d, dotGrid, dotPlane, dotFountain) · colorMap Stützstellen · convolution Kern |
  | Quellen/Kanäle | superScope `audioChannel/spectrumSource/lineBlend/colorBlend/dotSize/colorCycleFrames` · oscStar/oscRing/timescope `channel/position/source` · rotatingStars `bandLo/bandHi` · bassSpin `left/right` · shadertoy `imageInput[0..3]`, Buffer-Eingänge · isfFilter Quellen/Geometrie/Passes · videoSource alles · picture/texer Bild |
  | Sonstiges | clear `onlyFirst` · brightness `exclude` · blur `roundUp` · uniqueTone `invert` · colorClip `mode` · colorModifier `recompute` · texerII `resizing/wrapAround/colorFiltering` · triangle `filled/lineWidth` · dotPlane/dotFountain `startRotation` · hostSwitcher Liste/Intervall/Reihenfolge |

  Ohne Skript-Slots: renderScale, debugBars, water, scatter, invert, normalise.
  Nebenbefund dazu in §8 (Milkdrop-Tabelle unvollständig).

- 🟡 **Mash-Up-Sätze für 0.5.0 freigeben** (S75) — `GreatWho + Flexi - Rock The
  House [Caturday Night]` (93) und `GreatWho + Martin + Geiss + Flexi -
  Lasershow [240 bipolar mixes]` (243) sollen mitgeliefert werden, sind für
  0.4.0 aber **bewusst draußen**: es sind Mash-Ups **Dritter** auf Basis
  eigener Presets (vermutlich von **Flexi**), und die Erlaubnis des Erstellers
  steht aus. **Vor der Aufnahme:** Kontakt herstellen, dann Namensnennung in
  `asset/presets/README.md` **und** `THIRD_PARTY_NOTICES.md`. Aufnahme selbst
  ist ein Kopiervorgang — die Sätze liegen unter `asset/Milkdrop3/presets/`,
  der POST_BUILD-`copy_directory` nimmt neue Unterordner ohne Zutun mit.

- 🟡 **Startwert (Seed) als Preset-Feld** (Idee Patrik, S75) — der Kaltstart-Seed
  des Feedback-Puffers wandert aus der globalen Umgebung (`LUMIVIZ_MILKDROP_SEED`)
  in ein **Init-Feld des Presets**. Anlass: der erste vollständige
  MilkDrop-Referenzlauf stuft **6 von 19** Presets als `STARTABHÄNGIG` ein — ihr
  Bild reagiert stärker auf den Startzustand als auf den Renderer
  ([kalibrierung/INDEX §2.2](kalibrierung/INDEX.md)). Mit einem Seed im Preset
  wird aus „hängt vom Zufall ab" ein **reproduzierbarer, gestalteter Zustand**:
  der Autor legt fest, aus welchem Rauschen sein Preset zündet.
  Importierte Presets haben das Feld nicht (dann Vorgabe wie bisher) — für
  eigene Presets ist es ein Gestaltungsmittel. Zu klären: Feldname und Ablage
  (eigener Namensraum, damit MilkDrop-Kompatibilität bleibt), Verhalten beim
  Preset-Wechsel (Seed neu setzen oder Erbe behalten), und ob dasselbe für den
  AVS-Zweig sinnvoll ist.
  **Grundstein umgesetzt (S76):** `PresetState.kaltstartSaat` (Bool, Default
  an; Serializer-Fallback an, Panel „Kaltstart-Rauschsaat") — die Saat ist je
  Preset **abschaltbar** (aus = schwarze Basis, gleicher Vertrag wie NOSEED;
  die blanke Palette-Pipeline startet ohne). Offen bleibt die **Seed-Wahl**
  als Feld (welches Rauschen, nicht nur ob).
- ✅ **Umgesetzt in S82 als Song-Slot** (Freigabe Patrik S82) — fuenfter
  Slot der Engine (`Slot::Song`), Code am Knoten (`ChainNode::songCode`,
  JSON `songCode` nur wenn belegt), laeuft einmal je Songwechsel VOR dem
  Frame-Slot in derselben Engine; fuer alle 79 Knotentypen mit Frame-Slot
  (Effect List nur mit „Listen-Code"; nicht Movement, nicht MilkDrop — dessen
  eigene Pipeline bleibt offen). Ausloeser nur Songwechsel (`TrackChangedEvent`).
  Test: `test_ScriptVariablesGl` „Song-Slot" prueft jeden Typ. **Offen:**
  MilkDrop-Node, Playlist-Start/Stop als Ausloeser.
  Urspruenglicher Eintrag (bereinigt S129: als Song-Slot umgesetzt, s. o.):
- ~~**`onChange`-Slot: Skript beim Song-Wechsel**~~ (Idee Patrik, S75) — neben
  `init` ein Slot, der **beim Wechsel des Songs** ausgelöst wird; für
  MilkDrop-, AVS- und eigene Module gleichermaßen. Damit kann ein Preset auf
  einen neuen Titel reagieren (Farbwechsel, Reset von Zählern, neuer Seed —
  s. o.), statt nur auf Beat und Zeit. Verwandt mit dem **Szenen-Wechsler**
  (§7): der schaltet ganze Szenen, dieser Slot gibt dem einzelnen Preset einen
  Einstiegspunkt. Zu definieren: Auslöser (nur Songwechsel oder auch
  Playlist-Start/Stop?), welche Variablen dort sichtbar sind, Verhalten bei
  Presets ohne den Slot, und wie er in den bestehenden Slot-Quartetten
  (`init`/`frame`/`beat`/`point`) eingeordnet wird. **Nächste freie Session.**

- 🟡 **Vereinheitlichung V2–V5** ([Konzept v1.2.0](visuals/Vereinheitlichung_Konzept.md)):
  V2 Audio-SSOT (MilkLoudness überall, `visdata`-Baustein, `getspec`/`getosc` in
  Milk-Slots, `ScriptInputFeeder`) · V3 Konstanten + Funktions-Abgleich gegen EEL2 ·
  V4 Standalones → Module · V5 Gradient-Parameter-Typ + LUT-Baustein.
  **V1** (Basis-Key-Registry + Import-Umbenennung) ist mit `ScriptBaseKeys.hpp` und
  der D2-Regel in S51 vorgezogen worden. Ausführung ausdrücklich **nach** der
  Kalibrier-Runde.
- 🟡 **P3 Skript-SSOT modul×slot** ([Skript_Variablen_Konzept](visuals/Skript_Variablen_Konzept.md) §3/§8):
  1. Symboltabelle Modul×Slot×Name → Kategorie/Typ/Range/Text · 2. Referenz daraus
  generieren statt Hand-HTML · 3. Kategorie-Highlighter modul-bewusst ·
  4. Fehler-Markierung je Slot statt global konservativ.
- ~~**P2 Visual-Playlist**~~ — ✅ **geschlossen (Entscheid Patrik S93, C4 a):**
  entfällt als eigenes Vorhaben, aufgegangen in Composer
  ([Composer_Konzept.md](ui/Composer_Konzept.md)) und Host Switcher.
- 🔧 **P1 Set Render Mode auf alle Scope-Effekte** — durch S45/S3 weitgehend erledigt
  (`drawScopeShape`, `drawDots` zeichnen über den SRM-Zustand); als Punkt nie
  formal geschlossen. Prüfen und schließen.
- ⚪ **Hotkeys Stufe 2/3** — Stufe 2 (Transport) ist in S52 verdrahtet; Stufe 3
  (Composer-Spuren) ist Fernziel.
- ✅ **Hotkeys Stufe 1: Entscheide §9.2–§9.5 (Patrik S60) umsetzen und das
  Konzept nachziehen** — **gebaut S129:** §9.3 umlaufen, §9.5 das Menü zeigt
  die Taste aus der Registry (View → Karaoke, View → Fullscreen; F11 jetzt über
  den Filter, umbelegbar, wirkt auch im Vollbild), §9.4 als erfüllt geprüft;
  `Hotkey_Konzept.md` 1.3.0, Handbuch 1.179.0. ⬜ Im Programm ungesehen.
  (Eingetragen S129 — stand bis dahin nur als Halbsatz an
  der erledigten Zeile in §5.) Abweichend vom Entscheid hält
  `ImportBrowserPanel::nextPresetRow` am Verzeichnisende an statt umzulaufen
  (§9.3); §9.4 (Blättern ab dem zuletzt gewählten Eintrag) und §9.5 (Menüs
  zeigen die Tasten) nicht geprüft; `ui/Hotkey_Konzept.md` steht auf
  „Entscheide §9.2–§9.5 offen“.
- 🟡 **Audio-Effekt: Vocals unterdrücken oder allein behalten** (Idee Patrik
  S129; noch keine Vorlage, kein Konzept). Zwei Wege, die sich nicht
  ausschließen:
  1. **Ohne KI, als Effekt in Echtzeit:** „Mitte auslöschen“ (L−R, der
     Karaoke-Trick) — Vocals sind meist mittig gemischt. Nimmt auch Bass,
     Kick, Snare mit, Ergebnis mono; besser nur im Stimmbereich (etwa
     150 Hz–6 kHz) oder im Frequenzbereich (STFT-Maske: je Bin dämpfen, was
     links und rechts gleich ist), dann mit Artefakten und abhängig vom Mix.
     Umgekehrt (nur Mitte behalten) bleibt die Stimme mit allem Mittigen —
     zum Isolieren kaum brauchbar.
  2. **Mit KI, Quellentrennung in Stems** (Vocals, Drums, Bass, Rest; etwa
     Demucs, Spleeter, Open-Unmix): deutlich besser, in beide Richtungen.
     Kosten: ONNX Runtime oder LibTorch als Abhängigkeit, Modelle einige
     hundert MB; nicht in Echtzeit, sondern einmal je Song im Hintergrund,
     Stems im Cache (Bauart wie `SongLautheiten`), im Composer als eigene
     Ton-Clips spielbar — passt zu Karaoke. **Vorher belegen:** Lizenzen von
     Code und Modellgewichten.
- 🟡 **Loop: Saiteninstrumente** (Idee Patrik, S128; S129: „nicht mal so viel
  später“ — bald). **S132: Konzept und Bau begonnen** —
  [Saiten_Konzept.md](audio/Saiten_Konzept.md): Ziel S-Z1 a (Modell, dann
  Spielweisen, dann Tabs), Modell S-M1 a bis S-M3 a, Klang S-K1 (Waveguide und
  Extended KS schaltbar), S-K2 b (Classic bleibt), S-K3 a (Regler nach der
  Physik), Creator-Skizze S-U1 a gebilligt, Baustufen 23a bis 23g freigegeben
  (S-B1 a). **23a gebaut** (Modell und Datei, `.lvinst` 10, `.lvloop` 8).
  Tonumfang S-T1 a: Noten A0 bis C8 (vorher C1 bis C7). **23b gebaut**
  (S-A1 a: Auskunft `SaitenVon`; Saitenwahl Per chord / Follow hand,
  Abdämpfen an der nächsten Note derselben Saite, Tonname mit Saite und Ende).
  **23c ohne Korpus gebaut** (Extended KS, gemessen; S-K4 a, S-K5 a
  entschieden). S-K6 a, S-U3 a entschieden (zweite Recherche; Skizze Pickups,
  Mensur). **23d erster Teil gebaut** (Waveguide, Mensur, Tonabnehmer als
  Abtaststellen, Piezo). S-K7 a entschieden; **23d fertig** (Cable aus
  Messwerten, Mensuren der Vorlagen). S-K8 a: **Korpus Parlor gebaut**
  (gekoppeltes Modell nach Hess; Spitzen 116/219 Hz wie gemessen).
  **S133:** S-K9 a/S-K10 a — **Korpus Classical guitar** nach Wrights TOP26
  gebaut; S-P2 a/S-K11 a — **Bemessung der Tonabnehmer** (Fehler im ersten
  Abtastwert behoben, gleich laut wie die Saite am Steg, Spitze ≤ 0,9);
  **23e Creator** (Tuning, Model, String mit Saitenbild, Body, Pickups);
  S-U2 a/S-U2b a — **23f Instrument-Editor** (String choice, Saite je Note,
  1 = die hohe, Menü String); S-G1 a bis S-G4 a — **23g**: acht mitgelieferte
  umgestellt (die alten als `(old)`), sechs neue, Vergleichs- und
  Beispiel-Loops, Handbuch 1.189.0. Patriks Ohr: 23c „passt“, Korpus „passt
  eigentlich soweit“, 23d war zu leise (behoben). Offen: Patriks Sichttest
  (`.claude/handover/Sichttest_Saiten.md`), Finger Bass und Pizzicato nach
  Gehör (S-G4 a), der volle Lauf erst mit Tabs und Akkorden; als Nächstes die
  Spielweisen (S-W1 a, siehe die Idee in §7) und die Tabs.
- 🟡 **Loop: weitere Effekte** (Frage Patrik S129). Heute am Clip Filter,
  Distortion, Echo, Reverb (`ClipBaustein`, `loop/Loop.hpp`), an der Schicht
  Filter, Drive, Pan. Nicht geplant waren bisher Flanger, Chorus, Phaser,
  Stereo-Breite, Tremolo, Kompressor — sie passen als weitere Bausteine in
  dieselbe Reihe (Variant, Fenster Clip effects, Signal path). Noch keine
  Vorlage.

## 7. Backlog (bewusst nichts tun)

### ⚪ Befund: „Datensicherung: nur gewaehlte Bereiche“ einmal rot im vollen Lauf unter Testing-MSVC (S157)

Im vollen Lauf nach 31d-8 Schritt 8.4 (Testing-MSVC, 1633 / 1634) scheiterte `test_Datensicherung.cpp:224`
(`REQUIRE(leer.ok)`): der zweite Export schreibt dasselbe Archiv neu, kurz nachdem der Fall es mit `ds::oeffne`
geöffnet hat. Allein fünfmal grün, im vollen Lauf davor (gleiche Exe) grün, unter Release-clang 1634 / 1634.
Vermutung, nicht geprüft: die Datei war kurz gesperrt (Virenscanner, Index). Der Fall meldet den Grund nicht — beim
nächsten Auftreten zuerst den Fehlertext von `exportiere` in die Prüfung nehmen, dann entscheiden (eigene Datei je
Export oder Wiederholung beim Ersetzen). Hat mit dem Flügel nichts zu tun: seit dem grünen Lauf waren nur Loop-Daten
geändert.

### ⚪ Befund: ein Testfall hängt an der Reihenfolge — `presetIconDir()` merkt sich den Ordner (S131)

Mit dem Filter `*Milkdrop*,*Milk*,*Hlsl*,*Handbuch*,*nventar*,*Palette*,*ChainSerializer*`
schlug „PaletteUi S109: Audio Browser zeigt die Loops des Ordners …“ unter beiden
Compilern fehl (`test_PaletteUi.cpp:3987`, `dateiArtIcon("lvloop")` ist ein
Null-Icon); allein und im vollen Lauf ist er grün. `presetIconDir()`
(`UI/widgets/PresetTypeIcons.hpp`) sucht den Ordner einmal und hält ihn in
einem `static` — ruft ein Fall es zuerst auf, solange er nicht gefunden wird,
bleibt es für den ganzen Lauf leer. Welcher Fall das ist, ist nicht
untersucht; ändern erst auf Patriks Wort.

**Dazu S138: „Karaoke-Panel S89" schwankt** unter Testing-MSVC: `test_PaletteUi.cpp:769`
(`combo->itemText(0) == "Abend"` — die Zeile der Presets nach `processEvents`) einmal rot im
Bereichslauf `*Akkord*,*Griffbild*,*Editor*,*Spieler*,*Instrument*,*Zoom*` und einmal allein;
derselbe Bereich gleich danach 130 / 130, allein dreimal grün, unter Release-clang grün. Der
Fall berührt nichts aus Stufe 26 (Karaoke-Presets, `PresetListen`). ~~Ursache nicht untersucht.~~
✅ **Ursache gefunden und behoben S141 — ein Fehler im Programm:** nach dem Speichern
(`speichereKaraokeNach` meldet `PresetListsChangedEvent`) setzte das Panel nur den Namen neu, den
Stern „geändert“ erst der nächste Takt (500 ms). Tickte der Takt zwischen Edit und Speichern —
unter Last, unter Testing-MSVC —, stand danach „Abend *“. Jetzt liest das Panel dort gleich den
Stand (`aktualisiereZustand`). Der Test wartet vor dem Speichern bewusst einen Takt ab: ohne den Fix
jedes Mal rot, mit ihm grün (beide Compiler, je dreimal).

### ✅ Regression S84 — Tempo, Lag, Vollbild-Freeze (gelöst S85)

Keine Regression aus S84: Presetverweise bekamen beim Laden keine eigenen
Knoten-IDs (seit S82), alle Szenen von `list.lvfx2` liefen auf einer
gemeinsamen Laufzeit. Behoben in `baueLvfxBaum`; Vollbild läuft laut Patrik
wieder sauber. Messung, Ursache, Lehre:
**[Regression_S84_Tempo_Vollbild.md](Regression_S84_Tempo_Vollbild.md)** §7.

### ⚪ Wunsch: Shader-Uniforms im Skript-Editor einfärben (Patrik S85)

Eigene Uniforms (Shadertoy, MeshWarp, PixelFilter, GpuParticles) und
ISF-Eingänge sind Skriptvariablen (Block E, S79; bei Feld-Kollision
`in_<name>`, S85), der Highlighter kennt sie aber nicht — er färbt nur die
Referenz-Variablen des Moduls (`modulKategorien`, F5). Weg: die Namen, die
`EigeneUniformKopien::sammle` bzw. `isfSkriptName` bilden, als eigene
Kategorie in `modulKategorien` aufnehmen (gleiche Namensregel an einer
Stelle, sonst färbt der Editor anders, als das Skript bindet) und in der
ⓘ-Seite auflisten.

### ⚪ Beobachtung: erster Wechsel auf ein Preset hakt kurz (Patrik S85)

Beim **ersten** Wechsel auf ein Preset bleibt das Bild manchmal kurz stehen,
beim erneuten Wechsel auf dasselbe Preset nicht mehr. **Beobachtet beim
Wechsel auf ein AVS-Preset** (Nachtrag Patrik S85) — Kandidaten dort: der
.avs-Import selbst (Parser + Übersetzer; ob zwischengespeichert: ungeprüft), das
erste Kompilieren der EEL→Lua-Slots, APE-/Bild-Laden. Kein Killer; später
strategisch untersuchen. Verdacht (ungeprüft): einmalige Kosten beim ersten
Laden — Shader-/Skript-Kompilat, Textur-Upload, Datei-Cache. Messpunkte im
Render-Thread (> 50 ms, S84) zeigen den Schritt, wenn er dort sitzt. Verwandt:
Idee Preset-Warmup/Pre-Roll.

### ✅ Drag aus Effect Browser/Playlist, Audio Browser — Weg zur Timeline (Wunsch Patrik S83, umgesetzt S84, Sichttest bestätigt)

- ✅ **Effect Browser → Effect Chain** als **Presetverweis** (auch aus dem
  Explorer, mehrere Dateien; Einfüge-Regeln wie bei der Palette, `dropOrt`).
- ✅ **Playlist** nimmt Audiodateien an (an der Stelle) und gibt Einträge als
  Datei-URL + Zeilennummern ab; **Befund behoben:** Umsortieren änderte vorher
  nur die Anzeige (Doppelklick spielte danach den falschen Titel).
- ✅ **Drop auf Player-Panel / Visualisierung** = abspielen bzw. zum
  Playlist-Eintrag springen (Wunsch Patrik S84).
- ✅ **Audio Browser** (neu): Endungen von der Engine, Doppelklick = anhängen
  + abspielen; Icon = **Albumcover** (eingebettet ID3/FLAC/MP4 → Ordnerbild
  → SVG je Formatfamilie), Laden im Hintergrund.
- Gemeinsame Bausteine: `UI/widgets/DateiDrag.hpp`, `DateiListe.hpp`,
  `AudioDateien.hpp`, `audio/AlbumCover.hpp` — die Timeline nimmt dieselben
  Datei-URLs an.
- **Konzept (S93, gebilligt, alle Entscheide gefallen):**
  [Composer_Konzept.md](ui/Composer_Konzept.md) — das Vorhaben heißt
  **Composer** (Vorgabe Patrik S93; „Timeline" ist nur noch die Zeitachse im
  Panel). Renderweg: eigener Mischer über je einem Renderer je laufendem
  Clip; rückbaubar; Stufen C0, C0b (Belege), C1a (Mischer, Nähte R1 / R2)
  und C1b (Modell, `.lvcomp`, Uhr, Wiedergabe ohne Panel) erledigt — C1a
  und C1b im Zweig `composer`, noch nicht nach `master` geholt. Die folgenden
  Notizen sind dort eingearbeitet und bleiben als Quelle stehen; die Fragen
  aus S93 (Kürzel sperren, Start) sind dort entschieden.
- ✅ **Einzelne Clips ausschalten (Wunsch Patrik S126, D1 c, gebaut):**
  *Clip enabled* im Rechtsklick und Ctrl+E, nur mit *Edit*; ein
  ausgeschalteter Clip spielt nicht (Ton, Bild, laufender Song, Next /
  Previous), bleibt aber bearbeitbar. Composer_Konzept §9h. Im Programm
  nicht gesehen.
- 🔧 **Lautstärke-Abgleich der Ton-Clips (S126, entschieden; G1 bis G3 gebaut):**
  Anlass `asset/compare.lvcomp` — der Loop `titanium_versuch` ist viel leiser
  als der Song, und ein Clip kann heute nicht lauter werden als seine Datei.
  Entschieden L1 a (Gain je Ton-Clip in dB, getrennt von der Pegelkurve),
  L2 a (EBU R128, LUFS, Peak daneben), L3 a (Target: jeder Clip oder Fixed
  level, Vorgabe der lauteste), L3b a (gespielter Ausschnitt ohne Kurve),
  L4 a (Limiter am Mixer ab 0 dB). Dialog **Match loudness** als Skizze.
  Gebaut G1 (Gain je Ton-Clip), G2 (Feld Gain in der Fußzeile, Marke im
  Clip, Reset gain) und G3 (Messung nach R128, an EBU Tech 3341 belegt; ein
  Loop-Durchlauf gleicht dem nächsten). Dabei behoben: die BassEngine klemmte
  die Lautstärke auf 1, ein Gain über 0 dB kam bis zum Beleg G4 nicht an.
  Belegt: die Visuals sehen das Signal hinter einem DSP am Mixer. G4 Teil 1
  gebaut (Limiter `audio/Begrenzer` an der Engine), G4b a entschieden (Haken
  in der Zeile der Komposition). S127: G4 Teil 2 gebaut (Haken **Limiter**,
  Datei, die Tonführung schaltet ihn am Mixer) — G4 fertig. G5 entschieden
  (G5-1 a bis G5-4 a: Vorhören live mit Listen A/B und ▶, ein Undo-Schritt
  beim OK, Messen im Hintergrund mit Cache für Songs, Knopf neben Gain und
  Rechtsklick). S127: G5 gebaut (Messung je Clip mit Cache für Songs, der
  Dialog Match loudness), im Programm nicht gesehen; G6 fertig (Liste
  `Sichttest_Composer_11_Lautstaerke.md`, Vorlage `composer_L_lautstaerke`,
  voller Lauf 1263 / 1264 — der eine die Vorlage mitten im Lauf). **Offen:
  Patriks Sichttest 11 mit den Urteilen U1 bis U4.** Steuerdokument
  [Composer_Konzept.md](ui/Composer_Konzept.md) §9g.
- ✅ **Loop-Editor läuft mit dem Composer mit (Wunsch Patrik S127, nach G5/G6):**
  spielt im Composer ein Loop-Clip, dessen Loop gerade in einem Loop-Editor
  offen ist, soll die Abspielstelle des Editors simultan mitlaufen — an der
  Stelle im Loop, die der Composer gerade spielt. **Entscheid W1 a (S127):**
  Panel Loop (Marke im Arrangement), Drum- und Instrument-Editor (Marke im
  Muster, solange die Stelle in einem Clip mit diesem Muster liegt).
  **Entscheid W2 a (S127):** spielt der Composer denselben Loop mehrfach
  zugleich, folgt die Marke dem gerade lautesten Clip dieses Loops (seine
  Pegelkurve, ohne Gain — die Regel des laufenden Songs, S97). **Zusatz
  Patrik:** dieselbe Kaskade gilt auch vom Panel Loop zu den Editoren — heute
  zeigen die Editoren nur bei *Play pattern* eine Marke, beim Play des ganzen
  Arrangements nicht (`LoopPanel::zeigeStand`); beides geht über dieselbe
  Umrechnung Arrangement → Muster. **Entscheid W3 a (S127):** im Panel Loop
  ein Haken **Scroll with playhead** wie im Composer, Vorgabe an, gemerkt;
  an, folgt der Ausschnitt der Marke — Panel, Drum- und Instrument-Editor,
  bei jeder Wiedergabe (Play, Play pattern, Composer); die Art (stetig,
  seitenweise) aus der Einstellung des Composers; beim Ziehen am Rollbalken
  rollt nichts. Heute rollt die Achse des Panels nur beim eigenen Play mit,
  fest seitenweise (`LoopPanel::zeigeStand`; in der Vorlage W3 als „rollt nie
  mit“ falsch angegeben, berichtigt S127), die Editoren gar nicht. ✅ Gebaut
  S127 (Loop_Konzept „Die Marke läuft mit“, Sichttest 11 Abschnitt W), im
  Programm nicht gesehen.
- 🟡 **Idee Patrik S127 — Spielweisen der Saiteninstrumente** (**S133: Ziel
  entschieden, S-W1 a** — Strum und Palm mute, Let ring, Tapping, Harmonics,
  Hammer-on / Pull-off, Slide, Bend, in zwei Stufen; `audio/Saiten_Konzept.md`
  §1). **S134: entschieden S-W2 a bis S-W12 a, Stufe 24 (24a–24f, S-W9 a);
  gebaut 24a Modell/Datei (`.lvloop` 9, `.lvinst` 11), 24b Planung (Strum
  von sich aus an, Spread 8 ms je Saite), 24c Klang (Punktdämpfer nach
  Pakarinen für Palm mute und Harmonic, Tapping, Up), 24d Creator (N, O, P).
  S135: 24e Instrument-Editor gebaut (H–M; S-W13 a Vorgaben beim
  Instrumentwechsel, S-W14 a Spur Strum oben, S-W15 a Pfeil an jedem
  Anschlag); im Programm gefunden und behoben: der Editor übernahm die
  Anschläge nicht. Dazu Zoom in beiden Editoren (E-Z1 a, Loop_Konzept). S136:
  24f gebaut (S-W16 a bis S-W22 a: Spread je Bauart, Mute der Bässe 0,37, Tap
  level bei 220 Hz, Strum in den mitgelieferten Loops, vier Beispiele; Handbuch
  1.190.0; Sichttest `.claude/handover/Sichttest_Spielweisen.md`). **Zoom der
  Editoren: vorerst nicht gemerkt** (Patrik S136) — im Composer merkt die App
  die Zeilenhöhe (ZO9 a), den Zeit-Zoom nicht; angleichen wäre eine eigene
  Frage. Stufe 25 (S-L1 a bis S-L12 a): gebaut 25a Modell und `.lvloop` 10, 25b
  Planung der Ketten, 25c-1 Lagrange 5. Ordnung, 25c-2 `.lvinst` 12 (Fretless, Slide
  time, Legato level), 25c-3a Klang (Bahn), 25c-3b die Planung spielt Ketten. S137:
  25c-4 *Follow string* gebaut (S-L12 a, S-L13 a: Zeile *In a legato* im Kasten Source, „~“
  in der Liste); Bend-Lautstärke geklärt und nach S-L14 a behoben (Ring time der gebogenen
  Höhe, jetzt −0,6 dB). **Tabs und Akkorde entschieden** (S-T1 a bis S-T7 a: Tab-Linien und
  Griffbilder, Fenster *Chords*, `.lvchord` Griffbild oder Voicing, Vorlagen aus chords-db
  plus gerechnet, Skizze A–G, Stufe 26a–26h) und gebaut **26a** (Modell, Datei,
  Datensicherung, Publish), **26b** (6373 Vorlagen, Icon), **26c** (Tab-Linien), **26d**
  (Legato, Slide, Bend im Editor). S138: S-L15 b gebaut (Bend bis 2 Ganztöne); Skizzen und
  Entscheide S-T8 a bis S-T14 a (Galerie, vier Spielarten zum Anhören und gemerkt, Dialog
  *Edit chord*, Lagen aus chords-db bleiben mit Zeichen *unusual*, Arten in `types.json`,
  Voicings); gebaut **26e** (Fenster *Chords*, Namenserkennung, eigene Griffbilder), **26f**
  (Voicings mit Klaviatur), **26g** (*Play as* mit Hammer-on, Slide, Bend), **26h** (Beispiele
  `tabs_demo`, `voicings_demo`, Handbuch 1.191.0, Sichttest
  `.claude/handover/Sichttest_TabsAkkorde.md`). Nebenbei *Fit* im Instrument-Editor behoben.
  **Stufe 26 ist durch bis auf den vollen Lauf** (Patrik S138: „den vollen Lauf noch nicht“).
  **Offen:** voller Lauf; Sichttest Tabs und Akkorde (Exe neu bauen). Durchsicht der gerechneten Griffbilder durch Patrik
  (`.claude/handover/akkorde_26b/bogen.html`, Kipp-Bedingung S-T4 a). ~~Bei kurzen Noten
  überdeckt in den Tab-Linien die Höhe des Bends die nächste Zahl (Befund S137).~~ ✅ **behoben
  S141:** erst Dauer und Bend, darüber die Zahlen; die Höhe steht nur, wo sie keine Zahl verdeckt
  (`tabBendFeld`), sonst nennt sie der Tooltip der Note.
  Befund ohne Entscheid: `.sichttest/sichttest.projekt.json` sucht nur
  `Sichttest_Composer_*.md` — `Sichttest_Saiten.md`, `Sichttest_Spielweisen.md` findet der
  Tester nicht (Frage an Patrik S136, unbeantwortet).
  **Ideen Patrik S136, nicht eingeplant (erst nach dem Plan vorlegen):** (1) **Rechtsklick
  auf die Spur Strum** öffnet das Menü Strum direkt, dort jede Art mit ihrem Pfeil
  (Follow hand, Down ↓, Up ↑, Together) zum Wählen — heute öffnet der Rechtsklick dort
  nichts (`NotenRaster::contextMenuEvent` beginnt erst unter der Spur), und im Menü an der
  Note stehen die Arten ohne Symbol. (2) **Pick scrape:** mit dem Plektrum die Saite
  entlang streichen — die Windungen umsponnener Saiten (Bass, tiefe Gitarrensaiten) geben
  ein eigenes Rauschen; als Spielweise einer einzelnen Saite (vielleicht zwischen zwei
  benachbarten). Physik wie das Gleitgeräusch (Pakarinen u. a., JASA 2007: Frequenz
  n · v · d_w, Pegel ∝ Geschwindigkeit; Recherche 25 §3) — braucht die Windungsdichte
  d_w der Saiten (Recherche 25, Lücke 4).**
  **Idee Patrik (S138): Spielweisen auch für die anderen Instrumentfamilien** — so wie die
  Saiten Bend, Tapping, Palm mute haben, sollen Klavier, Bläser usw. ihre eigenen haben:
  etwa die **Pedale** am Klavier (Sustain, wohl auch Sostenuto und Una corda),
  **Dämpfer** an Blechbläsern, dazu was die Familien sonst tragen (bei Claude als Beispiele,
  noch nicht vorgelegt: Atem und Zungenstoß bei Bläsern, Vibrato, Pizzicato und *con
  sordino* bei Streichern). **S139: Konzept begonnen** — `docs/audio/Spielweisen_Konzept.md`
  0.5.0: Zielbild (alle Familien, Klang nach echten Instrumenten), SF1 c (Steuer-Spielweisen für
  alle, Klang-Spielweisen aus der Bauart), SF2 a, SF3 a (Sustain, Sostenuto, Expression, Vibrato,
  Pitch), SF4 a, SF5 d (Spuren je Muster, Expression auch am Clip). **S140: Konzept bis Stufe 1
  entschieden, Stufe 28 gebaut bis 28c-2a** — `Spielweisen_Konzept.md` 0.22.0: SF6 a (Bauart als
  Obergrenze, Ausstattung je Instrument), SF7 a (Spuren aller Schichten, *Follow* folgt bei den
  gemeinsamen), SF8 a (drei Orte der Klang-Spielweisen), SF9 a (Reihenfolge: Spuren, Hammer-Saite,
  Bogen-Saite, Platte und Stab, Lippe und Rohr, Rohrblatt und Rohr), SF10 a (Bauart = Anregung ×
  Schwinger), Skizze Stufe 1 §1a (Spuren oben unter Strum, Zeile *Lanes*, Expression auf dem Clip),
  SK1 a bis SK11 a; gebaut 28a (Zeitspuren, `.lvloop` 12), 28b (Pedale in der Planung), 28c-1
  (Expression als Pegel e², Vibrato und Pitch im Ton, *Pitch range*, `.lvinst` 14), 28c-2a
  (Vibrato und Pitch an der Saite, Vibratohebel, `.lvinst` 15). **Offen:** 28c-2b Expression an
  der Saite (je Schicht, nur mit Tonabnehmer — in den Ton gerechnet), Hörproben 28c-1/28c-2, 28d
  Instrument-Editor (Zeile *Lanes*, Spuren zeichnen), 28e Kurve am Clip, 28f Abschluss mit vollem
  Lauf; im Programm noch nichts angesehen (Regler *Pitch range*, Haken *Tremolo arm* nur im Test).
  Je Familie danach Recherche/Modell/Messung; der Ort jeder Klang-Spielweise; die Zeit zum
  Umstecken eines Dämpfers belegen; die Vorgaben des Hebels (2/1 Halbtöne) und 5,5 Hz in den
  Hörproben prüfen.
  **S141: Stufe 28 gebaut bis auf den vollen Lauf** — 28c-2b (Expression an der Saite hinter dem
  Tonabnehmer, SK12 a), 28d (Zeile *Lanes*, Spuren im Instrument-Editor), 28e (Kurve am Clip, SK13 a),
  28f (`lanes_demo`, Handbuch 1.197.0, Sichttest `Sichttest_Spuren.md`). **Konzept Hammer-Saite
  begonnen** (`Spielweisen_Konzept.md` §4, 0.27.0): Zielbild (alle Instrumente, alle Teile, alle
  Spielweisen), Befund „Töne werden bei Play bzw. 2 s vor einem Composer-Clip auf dem UI-Thread
  gerechnet, ohne Speicher je Tonname“, H1 a bis H3 a (Töne vorab), F1 a bis F6 a (Freeze, Idee
  Patrik), vorgemerkt Compute-Shader. **Offen:** Stufe 29 Töne vorab (Plan in S142, zuerst die
  Thread-Sicherheit von `rendereTon`), 30 Freeze, 31 ff. Hammer-Saite (Recherche; ein Upright mit
  geprüfter Lizenz fehlt); gesammelt später: voller Lauf 26–28, Sichttests, Hörproben.
  **S142: Stufe 29 gebaut bis 29e** (`Spielweisen_Konzept.md` §4.5, 0.35.0): TV1 a bis TV7 a
  entschieden; 29a `rendereTon` auf mehreren Threads, 29b `loop/Tonspeicher`, 29c `loop/Tonauftrag`,
  29d Loop-Panel (Play wartet, Vorladen, „Preparing sounds…“, Hinweiszeilen in beiden Editoren),
  29e Composer (Vorladen beim Öffnen, Play wartet, „Preparing sounds…“ im Feld Zeit, C und D).
  **29e nur unter Release-clang geprüft** (Patrik: MSVC zum Abschluss weggelassen). Dazu WX1 a
  (Stufe 30: erst die Rechnung, dann *Export audio…*, dann Freeze) und der **Audio-Clip im Loop als
  Stufe 31**, die Hammer-Saite rückt auf **32 ff.** **Offen:** 29e unter Testing-MSVC; 29f (Grenze
  in den Settings, Handbuch, Sichttest-Liste, Bereichslauf); WX2 Format, WX3 Mix des Loop-Clips im
  Composer; im Programm nichts angesehen.
  **S143: Stufe 29f gebaut** (`Spielweisen_Konzept.md` 0.36.0): TS1 a (Settings → Panels **Loop
  Sound Memory** mit Stand und **Clear**), Handbuch 1.198.0, Sichttest-Liste
  `.claude/handover/Sichttest_ToeneVorab.md` (S, P, E, K, Urteile U1–U4) mit der Vorlage
  `composer_M_toene.lvcomp`. **Stufe 29 damit gebaut — offen:** Testing-MSVC für 29e und 29f
  (Patrik S143: zurückgestellt), Sichttest Töne vorab, voller Lauf 26–29; danach Stufe 30 mit WX2
  (Format) und WX3 (Mix des Loop-Clips im Composer) einzeln.
  **S143 weiter: Stufe 30 entschieden und 30a gebaut** (`Spielweisen_Konzept.md` 0.38.0): WX2 a (WAV,
  32-bit float als Vorgabe, 24 und 16 Bit), WX3 a mit Zusatz Patrik (Loop-Clip im Composer ohne Gain,
  Blenden, Pegelkurve und Limiter), WX4 a (Teilschritte 30a–30d ff.), **WX5 a** (die Lautstärke des
  Loops wirkt nach den Effekten — Befund: sie verstellte die Zerrung; nur `effects_demo` *Acid* klingt
  anders, `Loop Werte` sieht es nicht). 30a `loop/Wellenform` (Clip mit Nachklang, Ausschnitt des
  Composers). 30b `audio/WavDatei` (32-bit float, 24, 16 Bit mit Dither; auch die Aufnahme schreibt
  darüber). 30c *Export audio…* (WX6 a bis WX9 a: eigenes Fenster; ein Clip, der ganze Loop mit
  Ending und Passes, ein Loop-Clip im Composer). **Offen:** Sichttest Export
  (`.claude/handover/Sichttest_Export.md`), Hörprobe `effects_demo` nach WX5 a (dort H4).
  **Freeze:** FZ1 a (30d–30i); 30d gebaut (`loop/Freeze`: Abdruck eines Clips, Klangfassung mit
  Wächter an `loop_werte.txt`; FZ2 a: Dateien über ihren Inhalt, nicht über Ort und Zeit); 30e gebaut (Ordner `frozen`, Grenze nach letztem Gebrauch, Settings
  *Frozen Clips*, FZ3 a). **S144: 30f–30h gebaut** — 30f Abspielen (FV1 a: der Nachklang klingt über
  die Naht; FV2 a: `frozen` trägt den Abdruck des eingefrorenen Stands, `.lvloop` 13; Spieler, Composer,
  Vorladen, Lautheit planen mit `eingefrorenVon`), 30g Bedienung (FZ4 a: Marke, *Freeze clip* /
  *Unfreeze clip*, Undo, Speichern friert neu ein, Hinweiszeile; Kurzform „verändert“ im Warndreieck,
  Zusatz Patrik), 30h Messung (FZ5 a bis FZ7 a: Sanduhr mit Rechenzeit, *Freeze hint* in den Settings,
  computing work / waiting time). Nebenbei behoben: Tonführung Fall C (falscher Klang nach einer
  Änderung, mit Test belegt) und der Absturz beim Beenden mit spielendem Loop (LZ1 a,
  `Bootstrap_Integration.md` §6.8; Gegenprobe am Stand vor S144). **S145: 30i gebaut, Stufe 30 fertig
  bis auf den vollen Lauf** — Publish nimmt die Wellenformen eingefrorener Clips mit (FP1 a: zum jetzigen
  Abdruck, fehlende rechnet Publish vorher; FP2 a: Gruppe *Frozen clips* und Haken *Include frozen
  clips* im Dialog, Publish wartet; FP3 a: der Import legt nur ab, was beim Empfänger zum Abdruck passt);
  Handbuch 1.202.0 („Clips einfrieren: Freeze“). **Offen:** Sichttest Freeze im Programm
  (`.claude/handover/Sichttest_Freeze.md`; nichts angesehen außer dem Beenden). **Bauplan geändert (Patrik S144):** Hammer-Saite
  wieder **Stufe 31 ff.**, der Audio-Clip im Loop danach.
  **Stufe 31 Hammer-Saite (S145 begonnen, nichts gebaut):** Recherche (Quellen der Noten, Literatur zu den
  Rechenverfahren), Entscheide BL1 a (drei Chopin-Ausschnitte als Beispiel-Loops), BL2 a (von Hand aus
  gemeinfreier Ausgabe vor 1900), HS1 a (modal nach Bank/Zambon/Fontana 2010), HS2 a (Teilschritte 31a–31h),
  HA1 a bis HA4 a (Datei: Kurven über die Tastatur, *Excitation* + *Modal* an der Schicht, Block *Keyboard*,
  Anschlagstärke gestuft im Namen). **S146: 31a und 31b gebaut** (Modell, Datei Fassung 16, ein Ton mit Saite,
  Hammer, Chor, Längsschwingung, Dämpfer, Stufe in der Planung; HK1 a, HK2 a, HT1 a bis HT6 a; Rechenzeit 2,3 % der
  Echtzeit im Bass); **31c begonnen:** HT7 a vorläufig (Boden aus *Salamander*), Recherche mit sieben Agenten, KB1 a
  (gemeinsame Grundlage für Körper); Berichte und Modell unter `docs/modellierung/`. **S147: 31c — der Boden wird
  gerechnet** (BM1 a: eigenes FEM-Werkzeug; Netzdatei zu RT-0425 nicht mehr zu haben; PL31c a: 31c-1 bis 31c-8; HT7 b:
  Salamander nur Vergleich; BM2 a, GB1 a (Glasboden in 31c-8, Schimmel-Glasflügel als Wunsch), BM3 a/-1 a/-2 a, BM5 a,
  BM5-1 a, IC1 a, BM4 a, EL1 a). **Gebaut 31c-1** (Bibliothek `InstrumentBody`, Bauplan `.lvinstbodyplan`, Steinway D
  aus RT-0425 Abb. 2, Icons), **31c-2** (Netz, CDT 2.0.1 als External), **31c-3** (Platte nach Reissner-Mindlin,
  Eigen 3.4.1 als External; gegen Navier auf < 0,1 %; Steinway bis 5 kHz 568 515 Größen). **S148: 31c-4 und 31c-5
  fertig** (EG1 a, EG2 a, BM6 a, BM6-1 a, PL31c-5 a, ST1 a): Spectra 1.2.0 als External, Eigenlöser mit Zählung nach
  Sylvester und Deflation, `InstrumentBodyTool`, Hörorte (`.lvinstbodyplan` Fassung 2), Auswertung je Mode, statistischer
  Teil bis 20 kHz, **`steinway_d.lvinstbody` gerechnet und abgelegt** (820 Moden bis 5 kHz, 2,9 MB, 22 min), Wächter.
  **S149: BM7 a** — die Grenze bleibt an der höchsten gerechneten Mode (5 kHz); 31c-6 prüft bandweise Konvergenz und
  Prüfstein und senkt sie nur, wo ein Band versagt.
  **S149: PL31 a** — erst 31c abschließen, vorher alle Umfangsfragen klären; geklärt mit PS1 a, PF1 a, KV1 a, AW1 a,
  FT1 a, PG1 a, SB1 a, DM1 a, GL1 a, VA1 a, ZA1 a, EM1 a; **fester Plan IP1, 31c-6 bis 31c-9** im
  `Spielweisen_Konzept` §4.1; IP1 (Fenster *Instruments* misst im Hintergrund, Loop_Konzept §4n) davor — **gebaut
  S149**, Release-clang `*Pegel*,*Palette*` 115 / 115; **S158: CR1 a** — der Creator misst ebenso auf einem Thread und gemerkt (beim Flügel vorher 0,65–1,9 s im Vordergrund nach jeder Änderung), gebaut; ~~*Chords* mit dem Flügel „ewig“~~ **erledigt S167 (AK1 a):** nicht der Flügel, sondern das erste Lesen von 6373 einzelnen `.lvchord` (frisch 25,6 s, jede Datei wird von Windows geprüft); jetzt sechs Sammlungen `.lvchords` (34 ms), gelesen je Stimmung (`Saiten_Konzept` §5ze). **Offen bei CMakeCraft:** die Konfiguration Testing baut
  unter MSVC mit `/MT` und ohne Optimierung (CC-Befund F1, F2) — **TC1 a** (Patrik S149): Testing optimiert mit
  Asserts und Debug-Info (`/O2 /Ob1 /Zi`, `-O2 -g`, ohne `NDEBUG`), CRT `/MD`, Pflichtzeile
  `project(<Name> LANGUAGES NONE)`; in CMakeCraft v0.13.0, mit SH und CS geklärt (beide ohne Einwand). Unter v0.13.0
  brach Testing-MSVC mit C1128 ab (`/O2` mit `/Zi`) — **BG1 a** (Patrik S149): `/bigobj` für alle MSVC-Ziele in
  v0.13.1. **Bei LV erledigt (S149):** Pin v0.13.1, `project()`-Zeile, die drei `/bigobj` je Datei entfernt; Testing-MSVC
  baut optimiert mit `/MD` (Bau der Unit-Tests 11 min 44 s; `*Loop Koerper*` 41 / 41 in 44 s, Release-clang 17 s).
  **31c-6 Teil 1 gebaut (S149):** Dämpfung je Werkstoff (DM1 a), Steinway neu gerechnet (Werte Bit für Bit gleich,
  neu die Anteile je Werkstoff).
  **31c-6 Teil 2 gebaut (S149):** Mobilität nach MAESSTRO Gl. 10/12, Terzbänder, Bericht im Werkzeug.
  **S150: 31c-6 fertig, 31c-7 gebaut.** 31c-6 Teil 3 (MAESSTRO-Prüffall unter den Testdaten, PF2 a: Steg geteilt bei
  40/41 — MAESSTRO Abb. 6 zählt ab C1; PF3 a: siebzehnte Rippe), Teil 4 (Prüffall gerechnet, Steinway mit k = 4
  bis 5 kHz: 822 Moden, 44 min, 9,6 GB), Teil 5 (`test_LoopKoerperPruefstein`: PS1 a (3) und EM1 a bestehen, KV1 a
  nach KV2 a; PS1 a (1) und (2) nur Bericht nach PS3 a und PS4 a — MAESSTRO legt die Formen auf eine Sinus-Basis
  unbekannter Größe und verschiebt die Stegpunkte, PS2 a: Code gelesen). 31c-7: pocketfft (FF1 a) hinter `loop/Fft`,
  Antwort des Bodens (`InstrumentBody/Antwort.hpp`, `loop/Boden`), Messteil (Antwort 0,62–0,77 s,
  `kSchalldruckBezug` 97,3 Pa), Ton der Hammer-Saite durch den Boden in Stereo, `.lvinst` Fassung 17
  (`soundboard.body`, „Body not found“), Hörproben `LUMIVIZ_31C_WAV`.
  **S151: 31c-8 und 31c-9 gebaut — 31c fertig.** 31c-8: Baupläne MP1–MP3 (Testdaten) und zwei Glasböden
  (`steinway_d_glass`, `steinway_d_glass_same_mass`, mitgeliefert), alle fünf gerechnet; die Richtungen von MAESSTRO
  bestehen (Moden MP1 < RP < MP2 < MP3, Mobilität MP1 am niedrigsten, MP3 am höchsten, Abstand Bass/Diskant bei MP1
  kleiner). Unterwegs: **RS1 a** (der Eigenlöser schärft eine Mode mit zu großem Rest nach — MP1/MP2 brachen ab),
  **AW2 a** (die Antwort reicht bis −60 dB, keine Grenze 8 s — der dünne Glasboden klingt 19,9 s), **AW3 a** vor dem
  Bau gekippt, **AW3-1 a** (Antwort auf einem Raster, schmale Resonanzen exakt: −96 … −104 dB gegen exakt, 14- bis
  19-fach schneller; 88 Tasten dünnes Glas 17,6 s statt 659 s). 31c-9: Bereich 640 / 640 und Composer 151 / 151, **voller
  Lauf 1582 / 1582 unter Release-clang (6,2 min) und Testing-MSVC (6,0 min)**; Hörproben aller Körper mit Liste unter
  `.claude/handover/hoerproben_31c9/`. Befund: `InstrumentBodyTool` rechnet einfädig; Alternativen (Löser mit
  Superknoten, Teilfenster in Prozessen) notiert, nicht gebaut.
  **Offen:** **Patriks Urteil über die Hörproben** (`.claude/handover/hoerproben_31c9/liste.txt`; Kipppunkte VA1 a — MP1
  klar hörbar anders → Varianten am echten Steinway — und DM1 a — Glas unnatürlich lang → Dämpfung durch Abstrahlung);
  die **Antwort der Autoren von MAESSTRO** (PS3 a; Entwurf der Anfrage lokal `.claude/handover/Anfrage_MAESSTRO_Daten.md`,
  Patrik verschickt) — mit den Daten die Nachbearbeitung nachbauen und PS1 a (1) gleich gegen gleich prüfen; viele
  Vorgaben vorläufig bis 31d (`docs/modellierung/hammer-saite/modell.md` §2).
  **S151: 31d–31h geplant (PL31 a).** 17 Umfangsfragen entschieden (UF1 a bis UF17 a): Salamander setzt, was keine
  Quelle hat, und vergleicht den Rest; das Original 48 kHz/24 bit; Messung C++ im Testbaum, ein Code für Aufnahme und
  Modell; Boden relativ, Raum und Mikrofon kürzen sich; Hörproben gleich laut; 31e nur Mitschwingen (Rückwirkung später
  als Erweiterung), Messschritt mit Kriterium; 31f mit Geräuschen aus Salamander; 31g Kurveneditor nach Skizze; kein
  Körper-Editor in Stufe 31; `grand_piano` neu neben `piano`, zwei Glasflügel; Chopin aus der Breitkopf-Gesamtausgabe;
  Deluxe als Marke; voller Lauf am Ende jedes Teilschritts. **IK1:** ein eigenes Projekt `InstrumentsCalculation` kommt
  später für die anderen Instrumente; bis dahin neuer Rechen- und Messcode ohne Qt. **Fester Plan 31d bis 31h** im
  `Spielweisen_Konzept` §4.1. ~~Als Nächstes: 31d-1~~ (erledigt S152).
  **S152: 31d-1 bis 31d-5 gebaut, 31d-6 begonnen.** Salamander beschafft (`…\cmake\Salamander\`, README mit SHA-256);
  WL1 a (WAV lesen ohne Qt, `audio/WavFormat`); `Teiltonmessung.hpp` und `Fluegelmessung.hpp` (Messung ohne Qt, für
  Aufnahme und Modell); Salamander gemessen (`werte/salamander_c5.txt`, D1 a: `undampedFrom` 89 aus der SFZ, `damper`
  nicht messbar; V1 a: Helligkeit über dem Pegel); dieselbe Messung am Modell; S1 a bis S3 a: die Vorgabe der
  Hammer-Saite durch das Modell hindurch gesetzt (`decay`, `highDecay`, `detune`, B im Bass, `bridgeAngle`); Hörproben
  31d (`.claude/handover/hoerproben_31d/`). 31d-6: an den Hüllkurven als Bild — das Modell löscht an den Einbrüchen fast
  vollständig aus; N1 a (Abklingen nach Schroeder, gebaut); N2 a: die zweite Polarisation als **31d-7 „Plan
  erweitern“**, bis dahin bleibt die Vorgabe aus 31d-4, der dauerhafte Test prüft nur B und Winkel. Voller Lauf
  1601 / 1601 unter beiden Compilern.
  **S153: Teilplan 31d-7 entschieden, 31d-7 A gebaut.** Recherche (P1 a), P2 a bis P8 a, ZS1 a (A Modell, B Messung und
  Setzen, C Abschluss), P3-1 a, RZ1-1 a, WK1 a; gebaut: Bezug `werte/hammer_bezug_*`, `loop/Eigenzerlegung`,
  `ebenenModen`, Längskraft je Saite, `.lvinst` Fassung 18 (γ = δ = h = 0), Klangfassung 2. Voller Lauf 1611 / 1611.
  **S154: 31d-7 B bis 7.4.** TB1 a (Tiefe der Einbrüche je Periode), Salamander neu gemessen (`tiefe_db`, leise/laut,
  Teilton 2–3), Werkzeuge `LUMIVIZ_31D7_BILD`, `…_TIEFE`, `…_RASTER`, `…_DISKANT`, `…_BLIND`, `…_MODEN`, `…_PROBE2`;
  SZ2 a: ungleicher Stoß je Saite (US1 a–US7 a, `.lvinst` 19, Klangfassung 3, gebaut und getestet); US6-1 a
  (Messgrenze 90 dB); RB1 a zurückgenommen, RB1-1 a (die zweite Stufe im Diskant fehlte wegen der Verstimmung der
  Vorgabe). Das Setzen lief dreimal und ist nicht brauchbar (decay läuft weg, Suche nicht eindeutig) → SZ3 a, SM1 a,
  SM2 a, SM3 a (gekippt), **SM4 a: an den Abklingkurven je Teilton setzen** — Teilplan offen. Nichts in die Vorgabe
  eingetragen. Kein MSVC, kein voller Lauf seit S153.
  **S155: Teilplan 7.5 zu SM4 a entschieden, 7.5.1 bis 7.5.3 gebaut, Probe 7.5.4 gestartet.** K1 a–K6 a, K6-1 a,
  SQ1 a, SQ2 a (Grenzen aus Quellen nur für Verstimmung, δ, γ, `highDecay`; `decay`, `lateDecay`, u ohne Quelle),
  SW1 a (Schwellen der Probe), SE1 a (Setzen je Taste mit Wiederaufnahme, zuerst Stichprobe 21, 27, 45, 72, 96, 108).
  Recherche mit sieben Agenten (`modellierung/hammer-saite/recherche/2026-10-26_S155_bereich_*`, `grenzen.md`),
  `docs/Bibliothek_offen.md`. Gebaut: Kurven nach Schroeder und Autokovarianz in der Messung, `werte/salamander_kurven.txt`
  (939 Kurven), `Kurvenabstand.hpp`, Werkzeug `LUMIVIZ_75_PROBE`; Teil (1): Setzen hochgerechnet 5,8–7,4 h (≤ 12 h).
  Kein MSVC, kein voller Lauf seit S153.
  **S156: Probe 7.5.4 zweimal gefahren, SM4 a gekippt, Teilplan SM4 b entschieden.** Rechenweg gemessen
  (`LUMIVIZ_75_ZEIT`, Taktprobe: Prozessor an der Wärmegrenze, 11 Threads am schnellsten); bitgleich schneller:
  Längskraft flach, Pool. Probe (54 min): (2), (3), (4), (7) verfehlt; Gegenprobe (`LUMIVIZ_75_SPRUNG`): Sprung an 48 aus
  Regel (b) → GP1 a, RV1 a (fehlende Kurve = sofort bis −100 dB); Probe nach RV1 a verfehlt wieder → SM4 b. Teilplan
  SM4 b: ZF1 a–ZF9 a (Toleranz aus Ebenen und Kanälen, Modell an der Struktur des Ziels, Levenberg–Marquardt, Schritte
  7.5.7–7.5.11 vor 7.5.5); **ZZ1 a: genau ein Anlauf — verfehlt die Probe, Gehör statt Setzen, weiter mit 31d-7 C und
  31e.** PB1 a: Berichte unter `modellierung/hammer-saite/proben/`. Voller Lauf 1631 / 1631 unter beiden Compilern
  (vor RV1 a), Rechenzeit Taste 40 3,73–3,82 %. Seit 31d-4 (S152) klingt der Flügel unverändert.
  **S157 (Sitzung mit Fable 5.1): SP1 a bis SP3 a, Teilplan 31d-8 „Spielbar“ vor SM4 b.** Lage: die Hörproben 31d
  (Kipppunkt von ZZ1 a) seit dem 25.10. ungehört; das Setzen ein Forschungsproblem (teils die Auflösung der Aufnahme:
  `lateDecay` 355 gegen 637 s an 33 sind über 23 s 1,7 dB); im Programm kein anspielbarer Flügel. SP1 a: 31d-8 mit
  Hörurteil (8.1), `grand_piano.lvinst` vom Programm geschrieben (8.2), Pegel (8.3), `grand_piano_demo.lvloop` aus
  `make_fluegel_loops.py` (8.4), Hörproben u/γ/δ (8.5), Gehör eintragen (8.6), Abschluss (8.7); SP2 a:
  Levenberg–Marquardt über die Stützpunkte der Kurven statt je Taste (ändert ZF6 a); SP3 a: eindeutig an den
  Messgrößen (ändert ZF8 a (3)). Lehren für die übrigen Klassen in `InstrumentsCalculation/LEHREN_SETZEN_FLUEGEL.md`.
  **31d-8 8.2–8.5 gebaut (S157):** `grand_piano` mitgeliefert (Level 4,1788), `grand_piano_demo`, Hörproben
  `.claude/handover/hoerproben_31d8/` (zehn Dateien: u an 33, 60, 84 mf/ff; γ und δ an 24 und 27 — Befund: γ wirkt
  nur mit δ); `LumiViz.exe` unter Release-clang gebaut. **Offen:** **31d-8** 8.1 (Patrik hört `hoerproben_31d`), die
  Wahl aus 8.5 (Patrik hört `hoerproben_31d8`), 8.6 (eintragen), 8.7 (Abschluss); **im Programm ansehen:**
  *Grand Piano* im Fenster *Instruments*, der Loop *Grand piano demo* — Wartezeit bis zum ersten Ton.
  **Seit RF1 a (S157) gilt die Tabelle „Offen — Stufe 31 bis zum Ende“ unten; der Text danach ist der Stand vor RF1 a.**
  Danach **7.5.7** Toleranzen messen, 7.5.8 Messung an der Struktur des Ziels, 7.5.9 Ziel, 7.5.10
  Levenberg–Marquardt (nach SP2 a), 7.5.11 Probe nach ZF8 a mit SP3 a; hält sie: 7.5.5 nach SE1 a (Stichprobe zuerst), 7.5.6 Vorgabe, 7.6
  dauerhafter Test voll, 7.7 beide Compiler und voller Lauf; verfehlt sie: ZZ1 a (die Werte aus 8.6 bleiben); **31d-7 C** (Hörproben, `modell.md`, dazu Stoß); nach Befunden S154: der Sprung
  unter −90 dB nach rund 5 s im Modell, der Nachklang an MIDI 48 bei sehr kurzem `decay` (Boden?), die Schein-Schwebung
  „6,0 Cent“ am Modell; Weinreich 1977 (Patrik); ~~Teilplan 31d-7 vorlegen~~ (S153);
  ~~danach neu setzen~~ → 31d-7 B; vierte Fassung der Hörproben → 31d-7 C; Kipppunkte aus 31d-6: Winkel an
  MIDI 33 an der Grenze 10°, Direktschall des Modells zu stark (SB1 a), Helligkeit im Diskant (HT2); **Patriks
  Hörurteil** über `hoerproben_31d` (Stufenschritt UF4 a / HT6 a; Kipppunkt N2 a → c) und über `hoerproben_31c9`.

  **Offen — Stufe 31 bis zum Ende (RF1 a, Stand S157; dieselbe Tabelle im `Spielweisen_Konzept` §4.1, dort
  maßgebend):**

  | Schritt | Inhalt | Stand | hängt an |
  |---|---|---|---|
  | 31d-8 · 8.1 | Hörurteil über `hoerproben_31d` (neun Dateien) | erledigt 2026-10-30 (PV1 a → ZZ1 a, Setzen nach Gehör in 8.5 und 8.6); Verlauf: `Spielweisen_Konzept` §4.1 von „Hörurteil S158“ bis PV1 a | — |
  | 31d-8 · 8.4 | *Grand piano demo* neu als ein Muster (HD1 a), Pegel alt = neu, Gain 0, ohne Freeze (DL1 a) | gebaut S157 (Muster *Piano*, 83 Noten; Spitze 0,066 → 0,277); voll Release-clang 1634 / 1634, Testing-MSVC 1633 / 1634 (rot nur ein Fall der Datensicherung, allein 5 / 5 grün — §7 Backlog); `LumiViz.exe` neu | — |
  | 31d-8 · 8.5 | Hörproben für das Setzen nach Gehör (ZZ1 a, PV1 a): je Taste 33, 60, 84 *Salamander* / Vorgabe / Suche „ohne A4“ (`proben/S165_suche2_probe_ohne_a4/`), dazu Patriks Wahl von u, γ, δ | in Arbeit seit 2026-10-30 (S166, 8.5): Hörtest `tools/hoertest` an `hoerproben_pv1` bereit, Antworten bei Patrik | — |
  | 31d-8 · 8.5-HT | Hörtest über das Werkzeug HörTest_Helper (HT1 a–HT9 a, Konzept dort 0.2.0): Ordner `hoertest/` (HT8 a), Katalog Flügel, PV1 als Liste; `tools/hoertest/` entfällt danach | offen | HörTest_Helper Stufe S2 |
  | 31d-8 · 8.6 | Gehör eintragen (`baueVorgabe()`), Werkzeug, Wertedateien | offen | 8.5 |
  | 31d-8 · 8.7 | Abschluss: beide Compiler voll, Rechenzeit mit Taktprobe, `modell.md` §2 | offen | 8.6 |
  | 31d-9 | Nocturne Op. 9 Nr. 2 ganz (CH1 a): Übertragung nach UF15 a, ein Muster mit Pedal-Spur, Stimmen je Note (HD2 a, HD2-1 a: bis 16, neue Fassung der `.lvloop`) | 9.1–9.6 fertig (S158): ganz übertragen (TR1 a, Kistner gegengeprüft), `nocturne_demo` mitgeliefert (Daten `asset/loop/nocturne_op9_2.py`, 40 Takte, 3:38), Fader +1,4 dB (Spitze −1 dBFS), Werte und Klangfassung, Hörprobe; voll 1640 / 1640 unter Release-clang (587 s) und Testing-MSVC (600 s) — NP1 a (Fader +1,4 dB); Hörurteil (DY1 a) bei Patrik; **DY2 a ausgeführt (S160):** die gerechnete Nocturne hatte 21,6 LU Loudness Range (Patrik: „zu krass“), das Modell spreizt 0,9–1,4 dB je dB Stufe (gemessen A0 … A7), Quelle Goebl, Bresin & Galembo 2004 (Hammer 0,37–4,07 m/s = 21 dB) → Tafel ppp −21 … ff 0, Akzent und Gabel 4 dB, Fader +1,2 dB (Spitze −1,0 dBFS mit dem Boden nach HU4-1 a), 15,5 LU; **NP2 a (S161):** Fader +7 dB mit Limiter nach Patriks Hörurteil (+5 dB noch etwas leise; Spitze +4,8 dBFS, der Limiter fasst rund 0,7 s der Kadenz); Loop, Werte, Klangfassung nachgezogen | — |
  | 31d-7 C | vierte Fassung der Hörproben, `modell.md` §2, §5, §6, §9, §10 | offen | 8.6 |
  | 31e | Mitschwingen (UF8 a, UF9 a) | offen, Plan steht | — |
  | 31f | Spielweisen: halbes Pedal, Sostenuto, Una corda, Geräusche (UF10 a, UF11 a) | offen, Plan steht | 31e |
  | 31g | Creator und Editor (UF12 a, UF13 a), dazu die Anzeige der Stimmen (HD2 a) | offen, Skizze zuerst | — |
  | Spuren und Tempo | SV3 a: Skizze (Vorgabe-Linie, Sprung greifen), Umfangsfragen (dazu TP1 a), dann Bau: Punktemodell mit Sprung als zwei Punkte (SV2 a) und Migration, Vorgabe sichtbar und gültig vor dem ersten Punkt (SV1 a), Editor, Tempo-Spur am Loop (TP1 a) | offen | — |
  | 31h | Instrumente, Deluxe, Chopin (Étude ganz nach CH2 a; Ballade ganz erst mit Tempo, sonst Ausschnitt), Abschluss der Grundlagen | offen, Plan steht | 31e–31g, Tempo |
  | 31i | **Feinabstimmung:** Probe 7.5.11 verfehlt (S163) → V2 a: 7.5.12 (A: δ im Chor, LM je Taste entkoppelt, Wanderer still, `detune` gestaffelt, drei Startpunkte; Lauf über Nacht, Probe wiederholt), 7.5.13 (B: Hörschwellen aus der Literatur, SP3 a neu, Ausreißer je Teilton), 7.5.14 (C: Dämpfung je Teilton nur bei Befund), dann 7.5.5, 7.5.6, 7.6, 7.7; verfehlt die Probe wieder, ZZ1 a; später Optimierung mit Spark möglich (eigener Entscheid); Befunde S154 (Sprung unter −90 dB nach rund 5 s, Nachklang an 48 bei `decay` < 1 s, Schein-Schwebung „6,0 Cent“); Rest 31d-6 (Winkel an 33 an der Grenze 10°, Direktschall SB1 a, Helligkeit im Diskant HT2); Stufenschritt (UF4 a) | offen | 31h |
  | daneben | Bibliothek (Hundley 1978, Weinreich 1977; `Bibliothek_offen.md`), Kühlung des Rechners | offen | — |

  Ursprung: Anschlag **down** und **up** klingen verschieden und spielen
  die Saiten eines Akkords leicht versetzt in ihrer Reihenfolge; Muster wie
  down-up-down-up einstellbar; **Tapping** mit eigenem Attack. Ein Instrument
  trägt dafür mehrere Anschlag-Charakteristiken nebeneinander (Artikulationen).
  Die Töne wahlweise über **Akkord-Tabulaturen** setzen — **optional neben
  der Piano-Roll, nicht statt ihr** (Präzisierung Patrik): ein Akkord lässt
  sich auch in der Piano-Roll darstellen, nur mit dem Versatz der Saiten;
  Tabs eigen oder aus Vorlagen, auf dem Griffbrett verschiebbar; welche Saite
  welchen Ton spielt, folgt aus dem Griff und der **Stimmung** (Standard
  E A D G H E, aber variabel — je Instrument zu definieren). Gilt für alle Saiteninstrumente mit ihrer
  Saitenzahl (Gitarre, Bass, Violine, vielleicht Harfe). Bei der Vorlage zu
  klären: Violine wird eher gestrichen als angeschlagen, die Harfe hat kein
  Griffbrett (eine Saite je Ton), Stimmung (Standard, Drop D, Capo).
  Gemeinsam mit dem Mitlaufen: die Umrechnung Arrangement → Muster und die
  Marke im Muster braucht ein Tab-Editor genauso.
- **Sichttests, S103 — neu gefasst, mit festen Vorlagen (Anlass Patrik: „ich
  habe die Sichttests aufgegeben, sie sind zum Teil etwas unklar
  beschrieben"):**
  - **Abgleich aller Listen S88 bis S102** (845 Punkte, nur gelesen): 542
    gelten noch, 178 sind bestanden, der Rest ersetzt oder überholt. Woran die
    alten Texte krankten: kein festes Material („etwa", „eine eigene"),
    Arbeitsdateien als Vorlage (`composed2*.lvcomp` haben sich seit S98
    geändert, `singlecomp.lvcomp` hat keinen Ton), mehrere Handlungen je
    Punkt, Zustand aus früheren Punkten, Ergebnisse ohne Maß, interne Wörter
    statt der Namen im Programm, Prüfung und Urteil vermischt.
  - **Der Befund S100 M11 („die shape sollte sich proportional verschieben")
    war kein Befund am Programm:** die zwei Bilder zeigen das Stück eines
    Clips, das anteilig mitwandert, wie gebaut. Gemeint war das Stück der
    Zeile MASTER (feste Zeit, Entscheid F7-1 a) — der Text des Punkts sagte
    das nicht. Neu gefragt in Liste 1, M2 bis M4, mit Gegenprobe und dem
    **Urteil, ob die feste Zeit an der Zeile MASTER recht ist** (offen).
  - **Neu, im Repo:** der Ordner `asset/sichttest/` — elf Vorlagen
    `composer_A_kurz.lvcomp` bis `composer_J_einrasten.lvcomp` mit runden
    Zeiten, acht Presets unter sprechenden Namen, sieben Testsignale als MP3
    (Entscheid Patrik: der Ton liegt im Repo), ein Songtext `.lrc`, dessen
    Zeilen ihre eigene Zeit nennen; und `test_ComposerVorlagen.cpp`, das jede
    Vorlage lädt und gegen die Zusagen der Listen prüft.
  - **Neu, lokal** (`.claude/handover/`): neun Listen
    `Sichttest_Composer_1_…` bis `_9_…` nach Thema (394 Punkte, davon 41
    „Vorbereiten"): Master-Zeilen · Zoom und Rollen · Kanäle · Songwechsel
    und Schleife · Karaoke · Mute · Start und Dateien · Curve Editor und
    Pegelkurve · Bauen und Sync. Je Abschnitt ein Punkt *Vorbereiten* mit
    Vorlage, je Punkt *Tun* und *Erwartet*, Urteile getrennt, die alte
    Kennung in Klammern. Sie ersetzen die offenen Composer-Punkte aus S94b
    bis S102; die alten Listen S98 bis S102 liegen in `sichttests-alt/`.
    **Im Programm ist davon nichts gesehen.**
  - **Aus dem Code gelesen, nicht gesehen** (steht im Kopf der Listen): ein
    Song ohne Titel-Tag erscheint als Dateiname ohne Endung (Liste 4); der
    Composer nimmt den Songtext aus der `.lrc` neben dem Song (Liste 5); wo
    ein Preset landet, das auf einen Clip gezogen wird (Liste 9, B4).
  - **Offen — die Listen außerhalb des Composers, rund 246 Punkte** (S88 bis
    S93: Karaoke-Ebene, Lyrics-Knoten, Lyrics Sync, Trail, Effektkette Save /
    Save As, Effect Chain, Sprite, Browser). Dafür fehlt Material: Songs mit
    synchronem und unsynchronem Songtext, Wort-Zeiten, Tags. Reihenfolge
    Patrik: zuerst die Abstimmung zur Steuerung (nächster Punkt), danach
    diese.
  - **Offen — Patriks Wort:** die 19 Punkte der Liste S95 gelten laut Report
    pauschal als „ohne Werkzeug: alles in Ordnung", ohne Lauf — zählen sie
    als bestanden? Dazu zwei alte Befunde ohne Antwort: S94b B1 (liegt das
    Original näher an Render Scale 1 oder 4?) und S95b D3 (die Wirkung der
    Mischart war mit der Demo nicht zu sehen). Weiter offen aus S98: der
    Wunsch zu Next / Previous (Liste 4, S13) und „Wirkung schwach" bei leisem
    Ton (Liste 4, N1, N2).
  - **Der Tester soll die App steuern** (Vorlage laden, Panel holen, Zustand
    herstellen — wie im Comm Studio): als eigener Schritt, abgestimmt über
    den Sync `…\cmake\sync_sichttest` (SH SichtTest_Helper, LV, CS Comm
    Studio, CC CMakeCraft). Richtung Patrik: das Werkzeug wird in
    `SichtTest_Helper` entwickelt, bleibt ein eigenes Programm; der Helper
    baut eine **DLL** mit der ganzen Schnittstelle, die Anwendungen binden
    sie ein; der ganze Tester wird nicht als Bibliothek eingebunden. Die
    Abstimmung führt eine eigene LV-Sitzung. Folgen hier: die Kopie
    `projects/exec/Sichttest` entfällt, sobald die Steuerung läuft; die Blöcke
    *Vorbereiten* der Listen sind gleich gebaut, damit der Tester sie später
    ausführen kann.
    **Stand S104 — der Bezug steht, benutzt wird er noch nicht:** External
    `sichttest` (CMakeCraft v0.10.0, Art `archive`) an Kern und Runner,
    `sichttest.pin` auf das Release `v0.2.0`; neben `LumiViz.exe` liegen
    `SichttestSteuerung1.dll` und `sichttest\`. Naht N19 (Composer_Konzept
    §12.2), Commit `5d49259`. Gebaut in Release-clang, Testing-MSVC und
    Debug-MSVC; **kein Kopf eingebunden, die DLL nie geladen, die App nicht
    gestartet, Unit-Tests nicht ausgeführt.**
    **Entscheide Patrik S104:** Z1 a (Zweig `composer`); **W1 a, vorerst** —
    `--testing` ändert den Start nicht, LumiViz stellt alles wieder her wie
    immer (fünf der neun Listen prüfen einen Neustart); offen bleibt, ob es
    später einen eigenen Zwischenspeicher für Tests gibt wie im Comm Studio;
    P1 a — die Projektdatei wird versioniert; **P2 a** — sie liegt in jedem
    Projekt unter `.sichttest/sichttest.projekt.json` im Root, Pfade ab Root
    (ändert Konzept der Steuerung §7 und die Suche im Tester — liegt bei SH).
    **Als Nächstes:** Antwort von SH zu P2 abwarten; per Test belegen, dass
    die Aktionen unter *Vorbereiten* aus jedem wiederhergestellten
    Startzustand denselben Stand erreichen, und nachsehen, ob die App sich
    beim Beenden die Test-Komposition merkt; dann die Nähte für `--testing`
    und die Aktionen eintragen und bauen (zuerst `komposition_laden`,
    `composer`, `edit`, `fit`).
    **Stand S105 — die Steuerung läuft:** `LumiViz.exe --testing` lädt nach
    dem Startzustand die DLL, meldet **fünfzehn Aktionen** an und verbindet
    mit dem Tester (Nähte N20 bis N26, Composer_Konzept §12.2; eigener Code
    unter `sichttest/` und `composer/SichttestAktionen`). Pins:
    `sichttest.pin` **v0.4.0** (seit S111, Entscheid Patrik 11.10.; in S107
    v0.3.4, in S105 v0.3.3 — v0.4.0 bringt die Köpfe S 1.1 mit
    `meldeZustand` und `wenn=<zustand>`, die LumiViz nicht nutzt, und im
    Tester die Ergebnis-Datenbank `testergebnisse.sqlite` je Ablage; beide
    Bauarten gebaut, DLL und Tester bytegleich mit dem Paket, die Probe
    meldet „0.4.0 (interface S 1.1, protocol P 1 to 1)" und verbindet;
    ⬜ `LumiViz.exe --testing` mit dem Tester v0.4.0 im Programm ungesehen),
    `cmakecraft.pin` **v0.11.1** (seit S112; die Paketdatei 1.1.1 nimmt dem
    Deploy-Ziel unter den Visual-Studio-Generatoren die Recipe-Datei — bei
    LumiViz am Bau nichts geändert, beide Bauarten gebaut; ob die Warnung
    hier je auftrat, ist nicht gemessen). Projektdatei
    `.sichttest/sichttest.projekt.json`; die Kopie `projects/exec/Sichttest`
    ist entfernt; neben der Exe entsteht bei jedem Bau die Verknüpfung
    „LumiViz.exe - Testing". Die neun Listen `Sichttest_Composer_1…9`
    (lokal) tragen 156 `aktion:`-Stücke an ihren Punkten *Vorbereiten*;
    jede Vorbereitung, die eine Vorlage lädt, setzt Edit, Loop und Sync on
    Effect Chain ausdrücklich (wo der Text nichts sagt: aus).
    **Belegt per Test** (`test_ComposerSichttestStart.cpp`): aus neun
    wiederhergestellten Startzuständen führen Load, Composer on, Edit, Fit
    zum selben Stand; die App merkt sich beim Beenden die Test-Komposition
    (`composer/session/file`) und lädt sie beim nächsten Start.
    **Entscheide Patrik S105:** **L1 a** — `komposition_laden` lädt und
    hält (Composer aus, 0:00), dazu die Einstellung *Sichttest: A loaded
    template starts playing*, Vorgabe aus; **C1 a** — `/bigobj` am
    Test-Target (N22); **LV2 a** — die Kopie des Testers entfällt (N23);
    **P1 a** — `LoadEffectChainEvent` bekommt `confirmed` (N24); bei
    SichtTest_Helper entschieden: Haken „In dieser Sitzung nicht mehr
    fragen" und Hinweis mit Knopf „Tester beenden" (im Paket v0.3.2).
    **Im Programm gesehen (Patrik, 09.10.):** der Tester kommt hoch und zeigt
    die neun Listen; ▶ Ausführen an Liste 2 G0 und T0 samt Rückfrage; der
    Stand danach wie zugesagt. **Nicht gesehen:** elf der fünfzehn Aktionen
    im Programm (der gute Weg von `panel_zeigen` und `karaoke_laden` ist
    seit dem Abend per Test belegt: echte Docks, Visualizer Multi Effect —
    974 Cases); ob nach `preset_laden` wirklich das Preset
    läuft; die Meldung bei laufender Instanz; Trennen und Wiederverbinden;
    der Haken und der Hinweis im Tester v0.3.2; die Verknüpfung per
    Doppelklick; die Stücke der Listen 1 und 3 bis 9 im Tester.
    **Offen:** Scroll with playhead kommt aus den Einstellungen durch (dafür
    gibt es keine Aktion); die Vorbereitungen ohne Vorlage (Liste 4 T0,
    Liste 6 D0, Liste 9 B0) setzen Edit, Loop und Sync nicht; (Liste 8 B0
    und Liste 9 F0 nennen seit S106 „Composer on" in Text und Aktionen —
    Entscheid Patrik V1: a; im Tester nicht gesehen;)
    ein gescheitertes Laden nach `preset_laden` meldet LumiViz in einem
    eigenen Dialog, der Tester erfährt es nicht; ein eigener
    Zwischenspeicher für Tests (W1) ist weiter nicht entschieden; die
    Gegenrichtung beim Beenden (LumiViz reagiert auf das Ende des Testers)
    bräuchte eine neue Fassung der Schnittstelle.
  - **✅ `composer` ist nach `master` überführt (Entscheid Patrik S105, M1: a;
    geschehen am 09.10.2026):** Vorspulen auf `ec836c6`, `master` trug seit
    `93d0be4` keinen eigenen Commit; dieser Stand trägt die Marke
    `vor-composer`. **Gearbeitet wird auf `master`.** Die Rückbau-Regeln aus
    Composer_Konzept §12 sind abgeschlossen — die Nahtliste bleibt als
    Beschreibung, neue Nähte werden nicht mehr eingetragen, Commits nicht
    mehr getrennt. Offen: der Push von `master` und ob der Zweig `composer`
    stehen bleibt (beides bei Patrik).
  - **✅ Release v0.12.0 (S105, 2026-10-10):** Version der App 0.12.0 in
    `Solution.json` (vorher 0.4.0, seit August unverändert); die Marke `v0.12.0`
    steht auf `f2faeee`, `master` ist gepusht. Paket
    `LumiViz-v0.12.0-win-x64.zip` aus dem Release-clang-Bau, **ohne** Tester,
    `SichttestSteuerung1.dll` und Verknüpfung, mit den 32 eigenen Presets;
    aus dem entpackten Zip gestartet, meldet sich als v0.12.0. Das
    Benutzerhandbuch beschreibt den Tester nicht mehr — er ist ein Werkzeug
    für Entwickler (`Sichttest_Steuerung.md`); die Zeile *Sichttest* in den
    Settings erscheint nur, wenn die Steuerung neben der Exe liegt.
    **Geschnürt wird von Hand** (der Ordner neben der Exe ohne `sichttest\`,
    die DLL, die `.lnk` und `LumiViz.log`) — ein Ziel im Bau gibt es dafür
    nicht. **Nicht gesehen:** das Paket auf einem Rechner ohne Qt und Visual
    Studio; die Settings-Zeile im Programm (mit und ohne Steuerung).
    README: Block „Composer mit Karaoke-Ebene" mit sechs Bildern aus
    `asset/composed2.lvcomp` — sie zeigen fremde Presets und Songtexte, mit
    Hinweis (Entscheid Patrik R1: b).
  - **🔧 Bauen (S104, S105):** Release mit clang baut nur in der Umgebung
    von **Visual Studio 18** (`vcvars64.bat` von dort, CMake von dort) —
    ohne sie scheitert `LumiViz.rc` ohne Meldung, mit der Umgebung von
    Visual Studio 2022 scheitert das Linken an fehlenden STL-Symbolen
    (`__std_find_end_1` …). Erledigt in S105: C1128 im Debug-Bau der
    Unit-Tests (`/bigobj`, N22) und die Kopie von DLL und Ordner nach einem
    Paketwechsel (eigenes Ziel `LumiViz_deploy_sichttest`, CMakeCraft
    v0.11.0). Ein Wechsel der Paketversion übersetzt Kern und Exe trotzdem
    neu: der Include-Pfad trägt die Version. In `.externals/` lagen in S105
    viermal Pakete, bevor LumiViz sie selbst geholt hatte (v0.3.0, v0.3.1,
    v0.3.2, v0.3.3) — jedes Mal beiseitegelegt, neu geholt und inhaltsgleich
    gefunden; woher sie kamen, ist nicht geklärt.
  - ✅ *(S141: Ursache gefunden und behoben — s. „Dazu S138: Karaoke-Panel S89 schwankt" in §7.)*
    **🔧 `test_PaletteUi.cpp:765` wackelt wieder:** der Fall „Karaoke-Panel
    S89: Schalter, Editor der Ebene, Ablage und Karaoke-Presets" fiel in
    zwei von vier vollen Läufen um (`combo->itemText(0) == "Abend"`); in S96
    zweimal, in S97 bis S102 über 44 Läufe nie. Von S103 nicht berührt;
    während der Läufe arbeiteten mehrere andere Sitzungen auf dem Rechner —
    ob die Last mitspielt, ist eine Vermutung. **Ursache nicht untersucht.**
    **S105:** in zwölf vollen Läufen einmal umgefallen — im ersten
    Lauf, nachdem der neue Fall „Aktionen der App" dazukam (er schaltet
    Karaoke on um und stellt es zurück); die fünf Läufe danach grün. Ein
    Zusammenhang ist weder gezeigt noch ausgeschlossen.
  - **🔧 `test_PaletteUi.cpp:1609` fiel einmal um (S112):** der Fall
    „PaletteUi Panel A10/A11: Add aus dem Palettenfenster" meldete im ersten
    vollen Lauf nach dem Pin auf CMakeCraft v0.11.1 `3 == 2` — ein Klick auf
    **Add** legte zwei Knoten in die Liste statt einem. Einzeln fünfmal grün,
    alle 81 Fälle `PaletteUi*` grün, der zweite volle Lauf grün. Am Code war
    nichts geändert. **Ursache nicht untersucht**; ein zweiter Fall dieser
    Art neben `:765`.
  - **🔧 Ein voller Lauf brach ab (S113):** der erste volle Lauf nach Stufe
    13 endete nach 267 Fällen — ein Fall fehlgeschlagen, **keine**
    fehlgeschlagene Prüfung, die übrigen 801 nicht mehr gefahren; das sieht
    nach einem Abbruch mitten in einem Fall aus. Welcher es war, wurde nicht
    mitgeschnitten; nach der Reihenfolge der Testliste liegt die Stelle bei
    den Composer-Fällen „S102 Zeitachse" (Nr. 265 bis 271, Höhen-Zoom mit
    Alt+Rad), die ein echtes Fenster auf Bildschirm 2 öffnen. Die neun Fälle
    `*S102 *` danach einzeln fünfmal grün, der zweite volle Lauf 1068 / 1068.
    Am Composer war in S113 nichts geändert; die Loop-Fälle waren im ersten
    Lauf noch nicht an der Reihe. **Ursache nicht untersucht.** Kommt es
    wieder: den Lauf mit voller Ausgabe mitschneiden.
- **Composer, S102 — Zoom und Rollen der Zeitachse (Konzept 0.68.0 §7;
  alle sechs Schritte gebaut, im Programm nichts gesehen — Sichttest
  `.claude/handover/Sichttests_S102.md`):**
  neun Entscheide Patrik, ZO1 bis ZO9 (Ctrl+Rad Zeit, Alt+Rad Zeilenhöhe,
  Ctrl+Alt+Rad beides, Ctrl+0; Master-Zeilen immer 60 %; Höhe 50 % bis
  300 %; das Lineal bleibt oben stehen; Festpunkt Mauszeiger; Schalter
  *Scroll with playhead*, die Art in Settings, Vorgabe stetig; Schwenken
  mit der mittleren Maustaste; Knopf *Fit*; die Höhe merkt die App).
  Bauplan in sechs Schritten. ✅ **Schritt 1 gebaut:** die Zeitachse rollt
  senkrecht selbst, das Lineal bleibt stehen, Rollbalken rechts — **im
  Programm nicht gesehen**. ✅ **Schritt 2 gebaut:** der Zeit-Zoom
  (Ctrl+Rad um den Mauszeiger, Shift+Rad waagrecht, Rollbalken unten, die
  Kanalnamen bleiben stehen, Lineal bis auf Zehntelsekunden) — **im
  Programm nicht gesehen**, nur ein Bild aus dem Test. ✅ **Schritt 3
  gebaut:** der Höhen-Zoom (Alt+Rad, Ctrl+Alt+Rad beides, 50 % bis 300 %,
  Master-Zeilen immer 60 %, die Höhe bleibt über den Neustart) — **im
  Programm nicht gesehen**, Bilder bei 50 % und 300 % aus dem Test.
  **Am Gerät zu prüfen:** ob Alt+Rad unter Windows überhaupt ankommt und
  ob nach dem Loslassen von Alt die Menüleiste in Ruhe bleibt (im Test
  ja). **Urteil Patrik offen:** die Schwelle für die Punkte der Kurve gilt
  für die Kanalhöhe und dann für alle Zeilen (50 % und 60 % ohne Punkte) ·
  ✅ **Schritt 4 gebaut:** Schwenken mit der mittleren Maustaste, der Knopf
  *Fit*, Ctrl+0 im Panel, *Zoom* im Rechtsklick-Menü — **im Programm nicht
  gesehen**; am Gerät zu prüfen: ob Ctrl+0 im angedockten Panel ankommt ·
  ✅ **Schritt 5, das Eigene, gebaut:** der Schalter *Scroll with
  playhead* (rechts in der Datei-Zeile, daneben *Fit*), stetig oder
  seitenweise, nur bei laufender Komposition — **im Programm nicht
  gesehen**; gemessen im Test: rund 11 ms je Bild ohne, rund 10 ms mit
  Mitlaufen (die Achse zeichnet ohnehin je Takt), neben dem Renderer im
  Programm nicht gemessen · ✅ **Schritt 5, die Naht N9, gebaut:**
  Settings → Panels → *Scroll with playhead* (Smooth / By page) — **im
  Programm nicht gesehen** · ✅ **Schritt 6:** Benutzerhandbuch 1.111.0
  (§15 Zoomen und Rollen), Rückbau-Liste im Konzept, `Sichttests_S102.md`.
  **Nicht gebaut, als Grenze vermerkt:** beim Ziehen eines Clips an den
  Rand rollt die Achse nicht von selbst mit; der Curve Editor zoomt nicht.
  ✅ **Der Vergleichslauf nach R2 (S101) entfällt** (Entscheid Patrik
  S102, VL1: a) — siehe den Block darunter.
- **Composer, Stand S101 am Ende (Konzept 0.61.1), kurz:** in S101 gebaut
  und **im Programm weder gesehen noch gehört:** der Fix am Panel Karaoke
  bei laufendem Composer, `LumiViz.exe <datei>` mit der einen Instanz je
  Exe, die Master-Zeilen auf 60 %, die Zeilen VISUAL, AUDIO und KARAOKE,
  der Haken Karaoke, der Mute für Durchsagen mit Mute-Profilen (Knopf im
  Panel Player und im Panel Composer). **Als Nächstes Patriks Sichttest
  `.claude/handover/Sichttests_S101.md` (67 Punkte, P, X, Z, Y, D)**; aus
  `Sichttests_S100.md` sind 46 Punkte weiter offen. **Danach (Patrik):**
  Zoom, nicht nur in der Zeitachse — Umfang zuerst klären. Offen dazu die
  Entscheide zu `LumiViz.exe <datei>` (Öffnen-Befehl im Explorer-Skript,
  Audiodatei als Argument, Preset gegen wieder eingeschalteten Composer).
  ✅ **Der Vergleichslauf nach den zwei Eingriffen am Renderer (Naht R2:
  `karaokeFassung`, der Pegel beim Auflegen der Karaoke-Ebene) entfällt**
  (Entscheid Patrik S102, VL1: a — schließen mit Begründung, ohne Lauf;
  verworfen: der übliche Lauf alt gegen neu, weil er die geänderte Stelle
  nicht ausführt, und ein gezielter Lauf mit Karaoke-Ebene, weil er einen
  zweiten Build und einen Testfall in zwei Ständen kostet für etwas, das
  die Rechnung deckt). Begründung: der Zähler `karaokeFassung` wird vom Renderer nicht gelesen (nur
  von der Sitzung des Composers); der Faktor in `legeKaraokeAuf` ist auf
  dem normalen Renderweg genau 1, und mal 1 ändert eine Gleitkommazahl
  nicht; die Harnesse `compare_avsref.py` / `compare_milkref.py` rendern
  über die Standalones ohne Karaoke-Ebene und erreichen die Stelle nicht;
  sonst hat sich an den Quellen der Visualizer seit dem Stand vor S101
  nichts geändert (`git diff --stat`). Die Wirkung des Pegels bleibt durch
  den Test aus S101 belegt (Pegel 0,5 = 49,8 %). **Nicht geprüft:** dass
  die Karaoke-Ebene in den Standalones aus ist, folgt aus der Suche im
  Quelltext, kein Standalone lief dafür; ein Bildvergleich seit S100
  liegt nicht vor. **Bleibt:** der Vergleichslauf vor dem Holen nach
  `master` (Konzept §12.4) — er deckt dann alles seit S93.
  ✅ **Nachgeholt S129 (Entscheid Patrik B2 a)** — vorher nie gefahren
  (weder Report S105 noch Changelogs S105–S128 nennen ihn), seit
  `vor-composer` (`93d0be4`) +1257/−99 Zeilen in den Visualizern. Stand
  `vor-composer` per `git archive` neben das Repo gelegt, beide Stände unter
  Release-clang, dieselben Preset-Dateien aus dem Repo, Bildschirm 2,
  verglichen die LumiViz-Bilder alt gegen neu:
  **AVS** (`compare_avsref.py`, 26 Kalibrier-Presets + 10 mitgelieferte,
  320×240, Aspekt-Reihe, 2560×734): **252 / 252 bitgleich.**
  **MilkDrop** (`compare_milkref.py --zufall-lumiviz`, 22 mitgelieferte,
  640×480, Aspekt-Reihe, 2560×734, je drei Marken und zwei Saaten):
  **747 / 748 bitgleich**; das eine (Helix, 640×480, Saat 0x5EED63, f30)
  stammt aus einem Lauf, der danach mit Kartenfehler abbrach — Helix je
  Stand zweimal einzeln nachgefahren: alle vier Läufe bitgleich, gleich dem
  alten Stand. **Reichweite:** die Harnesse rendern über die Standalones
  (AVS-Import, MilkDrop-Knoten); Composer, Karaoke-Ebene und `.lvfx`-Ketten
  mit den neuen Knoten (Shape, XY Scope) erreichen sie nicht. Für den
  alten Stand nötig (unversioniert): BASS- und Lua-Binaries,
  `asset/Milkdrop3`, `tools/*Ref/build`, `tools/MilkdropRef/third_party`.
  Die Einzelheiten stehen in den Blöcken unter „Stand S100".
- **Composer, Stand S100 am Ende (Konzept 0.52.1), kurz:** gebaut sind die
  Zeile MASTER (Pegelkurve der ganzen Komposition, samt Ziehen des festen
  Endes), das Preset *Late fade in*, Kanalfarben, Kanäle verschieben und
  nach Art ordnen, das Icon für `.lvcomp` im Effect Browser und im Panel
  Composer, dazu der Fix zum Befund A2. **Im Programm ist davon nichts
  gesehen** — als Nächstes Patriks Sichttest
  `.claude/handover/Sichttests_S100.md` (70 Punkte). Die Einzelheiten stehen
  in den folgenden Blöcken.
  - **Sichttest S100, erster Teil ausgewertet (Patrik, 07.10. 12:55 bis
    13:15 Uhr, Exe von 12:22 Uhr):** von 70 Punkten 23 bestanden, 1 mit
    Bemerkung, 0 durchgefallen, 46 offen. **Im Programm gesehen, ohne
    Befund:** der Nachtest zum Player-Panel (A1 bis A4 — der Fix zu A2
    trägt; A4 „kam der Befund so zustande?" ist ohne Bemerkung bestanden,
    ich lese das als Ja), die Zeile MASTER in der Grundfunktion (M0 bis M9:
    Zeile, Anklicken, Fade in / Fade out mit Feld und Griff, Form, Curve
    Editor, ein Stück, zu sehen und zu hören), die Icons (I1 bis I6, samt
    Doppelklick auf eine `.lvcomp` und Taskleiste), *Visuals overlap
    themselves* (G2, G3 „ruckelt es?" — nein, G4). **Mit Bemerkung — M17,
    die Höhe: „Master (auch künftige Audio und Visual Master) auf 60 %
    reduzieren"** — umgesetzt in S101 (`Zeitachse::kMasterHoehe`, 21 statt
    36 Pixel; im Programm ungesehen). **Vorbehalt:** während des Tests
    liefen meine Builds und Unit-Test-Läufe (Last, leiser Ton); das Log
    zeigt in der Zeit einzelne lange Bilder (bis 131 ms um 13:02, 88 und
    98 ms um 13:10 / 13:11 bei G2 / G3) — ob vom Vorwärmen des zweiten
    Exemplars oder von meiner Last, lässt sich daraus nicht trennen; nach
    jedem Lauf „audio sources realigned 0 times", keine Warnung. **Offen:**
    A0, A5, M10 bis M16, M18 bis M24, L, K, V, I7, G0, G1, G5, R.
  - ⬜ **Befund Patrik S101, behoben, im Programm ungesehen: das Panel
    Karaoke lud bei laufendem Composer nichts** („Could not load: <Pfad>",
    ohne Grund; im Panel stand „works with the Multi Effect visualizer").
    Ursache: bei Composer an und Sync aus meldet die Sitzung „kein
    Visualizer", damit Effect Chain ruht — die Karaoke-Editoren hörten mit.
    Entscheid KP1: a. Jetzt bleiben sie an der Ebene der App (Naht N16), und
    der Composer setzt jede Änderung seinem Karaoke-Träger ein (R2 wächst um
    `karaokeFassung`). Per Test gezählt (Fall S101, `test_ComposerSitzung.cpp`).
    **Nachtest:** Composer on, im Panel Karaoke ein Preset laden und ein Feld
    verstellen — beides sofort im Bild; **Urteil:** ruckelt es beim Ziehen
    eines Reglers, reißt der Trail ab? (Nicht gemessen.)
  - **Audio-Master und Visual-Master, Schritt 1 gebaut, im Programm
    ungesehen (S101, Entscheid Patrik AM1: a „um beide Optionen
    freizuhalten"):** unter MASTER die Zeilen **VISUAL** und **AUDIO**, je
    eine volle Kurve wie MASTER (Blenden mit Form, Grundpegel, Stücke an
    fester Zeit, Curve Editor, Undo, Datei `.lvcomp`: `visualMaster`,
    `audioMaster`). Malgenommen: Bild = Clip × MASTER × VISUAL, Ton = Clip ×
    MASTER × AUDIO (`Komposition::bildPegelBei` / `tonPegelBei`). **Von mir
    gesetzt (Urteile im Sichttest):** Namen und Reihenfolge der drei Zeilen;
    immer sichtbar, nicht einklappbar (63 Pixel); VISUAL mit kühlem, AUDIO
    mit warmem Grau; die Felder *Fade in* / *Fade out* gehören weiter nur
    MASTER; das feste Ende zieht man nur an MASTER; die Texte des Curve
    Editors sprechen weiter vom Clip. **Nicht geprüft:** ob die Griffe in
    den 21 Pixel hohen Zeilen gut zu greifen sind.
  - **Schritt 2a gebaut (S101), ohne Knopf und ohne Ton: das Mute-Profil
    und sein Verlauf** (`composer/Mute`): `MuteProfil` (Dauer und Form des
    Ausblendens, Dauer und Form des Einblendens, Zielpegel), `MuteVerlauf`
    (Faktor über der Zeit; mitten in einer Blende umgeschaltet geht es ohne
    Sprung weiter, der Rest dauert anteilig), Profile als Dateien `.lvmute`
    (mitgeliefert `asset/composer/mutes`: *Announcement* 20 %, *Soft mute*,
    *Hard mute*; eigene im Benutzerordner), das gewählte Profil unter
    `composer/muteProfile`. **Von mir gesetzt:** die Dauer gehört zum Profil;
    die drei mitgelieferten Profile und ihre Werte; Vorgabe ohne Profil 0,4 s
    aus, 0,8 s ein, auf 0 %. **Entschieden (Patrik S101, AM2: c):** der Mute
    gehört dem Player und gilt immer — ein Knopf im Panel Player, derselbe
    noch einmal im Panel Composer, beide schalten denselben Zustand.
    **Befund zur nächsten Frage (aus `test_ComposerNahtMixer.cpp`, Zeile 137
    und 166):** der Mixer läuft dort mit 3 % Lautstärke, und die Linie im
    Spektrum liegt über 0,3 — die Lautstärke des Mixers ändert sein Spektrum
    nicht. Ein Mute an der Lautstärke lässt die Visuals also weiter voll
    reagieren; die Zeile AUDIO (Pegel an den Quellen) dämpft sie. Für den
    Stream des normalen Players ist dasselbe **nicht** gemessen.
  - **Karaoke im Composer, gebaut, im Programm ungesehen (S101, Entscheid
    Patrik KS1: a — Spur und Haken):** vierte Master-Zeile **KARAOKE** mit
    voller Kurve (`Komposition::karaokeKurve`, Datei `karaokeMaster`); die
    Wiedergabe legt die Ebene mit dem Pegel der Kurve an der laufenden
    Stelle auf (Naht R2 wächst: `legeKaraokeEbeneAuf(pegel)`, vor dem Bau
    eingetragen). Dazu der Haken **Karaoke** im Panel Composer, gekoppelt
    mit *Karaoke on*. **Gemessen im GL-Test:** bei Pegel 0,5 ist der
    Abstand zum Bild ohne Text 49,8 % dessen bei Pegel 1; bei 0 liegt
    nichts auf. **Von mir gesetzt:** die Kurve steht für sich (MASTER und
    VISUAL blenden den Songtext nicht mit, wie seit S99); Name, Platz als
    unterste Master-Zeile, grünliches Grau; vier Zeilen = 84 Pixel.
    **Grenze:** ein Teil mit Mischart XOR kennt keine Stärke — voll da, bis
    die Kurve null erreicht. **Urteil im Sichttest:** nehmen vier
    Master-Zeilen zu viel Platz — einklappbar machen?
  - *Erledigt, zur Einordnung:* **Wunsch Patrik S101: Karaoke im Composer schalten** — ein Haken im
    Panel Composer, gekoppelt mit *Karaoke on* des Panels Karaoke (wie der
    Mute-Knopf an zwei Orten), „oder gleich auch eine Spur wie für die
    Master — gleich bedienbar wäre am konsistentesten". Aus dem Code
    gelesen: die Ebene wird im Composer über `legeKaraokeEbeneAuf` aufgelegt,
    je Teil mit einer Stärke in Prozent — ein Pegel der ganzen Ebene ließe
    sich dort als weiterer Faktor einsetzen (Naht R2 wüchse). Entscheid KS1
    vorgelegt.
  - **Schritt 2b gebaut (S101), noch ohne Knopf: der Mute wirkt am Ton**
    (Entscheid Patrik AM3: c — ob die Visuals den Mute sehen, steht im
    Profil; Vorgabe nein). **Naht N17** (vor dem Bau eingetragen): der
    Player hat wieder einen zweiten Faktor neben der Lautstärke (`setPegel`,
    die frühere N7a); `MuteSchalter` (der eine Zustand der App, Kind der
    QApplication) setzt ihn nach dem gewählten Profil, in einem eigenen Takt
    während der Blende; der Composer liest ihn je Takt mit der Lautstärke
    und setzt beides an seinem Mixer. **N13 wächst:** `onAudioUpdate` nimmt
    Spektrum und Wellenform mit `composer::muteFaktorFuerVisuals()` mal —
    unter 1 nur bei einem Profil mit *Visuals follow* (`visualsFollow`).
    Belegt: Verlauf am Papp-Player, Mixer folgt und die Quellen bleiben, der
    echte `AudioPlayer` (Regler bleibt, kein Ereignis, hartes Mute gewinnt,
    neuer Titel beginnt abgesenkt). **Nicht belegt:** die Zeilen in
    `onAudioUpdate` (das Hauptfenster läuft in keinem Unit-Test); der Ton an
    der echten Engine; dass niemand den Player an den Schalter hängt —
    **das tut erst der Knopf**, bis dahin wirkt im Programm nichts.
  - **Schritt 2c gebaut (S101), im Programm ungesehen und ungehört: der
    Mute-Knopf** (`composer/MuteKnopf`) — im Panel Player (Naht N18: ein
    Aufruf `composer::erzeugeMuteKnopf`) und im Panel Composer, beide zeigen
    denselben Zustand; Rechtsklick wählt das Profil; Dialog *Mute Profiles*
    (Zielpegel, Dauer und Form je Blende aus den Blenden-Presets, *Visuals
    follow*, Save As..., Delete). **So gelesen, nicht eigens entschieden:**
    ein NEUER Knopf neben dem Lautsprecher (AM2 c: „ein Knopf im Panel
    Player") — der Lautsprecher bleibt das harte Mute. **Von mir gesetzt:**
    Text „Mute", orange im gedrückten Zustand, kein Tastenkürzel, der
    Zustand wird nicht gespeichert, nur der Ton (kein Bild-Mute).
    **Sichttest:** (a) Playlist spielt, Mute drücken — der Ton geht weich
    weg, noch einmal — er kommt zurück; (b) dasselbe bei laufender
    Komposition; (c) Knopf im Panel Composer und im Panel Player zeigen
    dasselbe; (d) Rechtsklick → *Announcement*: die Musik bleibt leise
    hörbar; (e) ein eigenes Profil mit *Visuals follow* — das Bild beruhigt
    sich, ohne den Haken nicht; (f) **Urteile:** Name und Platz des Knopfs
    neben dem Lautsprecher, braucht es ein Tastenkürzel, die Werte der drei
    mitgelieferten Profile.
  - *Erledigt, zur Einordnung — die ursprüngliche Vorgabe:* **der Mute-Knopf selbst (Vorgabe Patrik
    S101):** Durchsagen sind meist spontan, also ein Knopf „Mute" (ein Klick
    auf die Zeile AUDIO wäre riskant). Mute-Profile, zu speichern und zu
    laden, bestimmen: die Kurve des Ausblendens, die Kurve des Einblendens
    und den Pegel, auf den abgesenkt wird (0 bis 100 % — also vielleicht
    gar nicht ganz stumm). **Vor dem Bau einzeln vorlegen:** nur Ton oder
    auch Bild; Dauer der zwei Blenden als Teil des Profils; wo der Knopf
    sitzt und ob es ein Tastenkürzel gibt; ob Mute auch ohne Composer wirkt
    (normaler Player); ob die Visuals die Absenkung sehen sollen (heute
    sähen sie sie — der Pegel sitzt an den Quellen des Mixers); wo die
    Profile liegen und welches gilt.
  - **Wunsch Patrik S100 (ursprünglicher Wortlaut): Audio-Master
    und Visual-Master** neben der Zeile MASTER — je eine eigene Pegelkurve
    nur für den Ton und nur für das Bild, „da kann man z. B. einfach die
    Musik für einen Moment stummschalten für z. B. Mikro-Durchsagen". Heute
    wirkt die Zeile MASTER immer auf beides zugleich
    (`Komposition::blendeBei` in `Wiedergabe::rendere` und
    `Tonfuehrung::takt`). Vor dem Bau als Entscheid vorlegen: zwei weitere
    Zeilen unter dem Lineal (**Urteil M17 liegt vor: Master-Zeilen auf 60 %
    der Kanalhöhe, auch diese zwei**), ob sie sich einklappen lassen, wie sich die drei Kurven
    verrechnen (malnehmen), und ob zum Stummschalten ein Schalter genügt
    statt eines Stücks in der Kurve.
- **Composer, Stand S100 (Konzept 0.44.0) — Sichttest S99 ausgewertet
  (Patrik, 06.10. abends):** von 52 Punkten 31 bestanden, 3 mit Bemerkung,
  1 durchgefallen, 17 offen. **Im Programm gesehen und gehört, ohne Befund:**
  die Überlappung in der Schleife (E2 bis E15) und die Blende der ganzen
  Komposition (F1 bis F6, F8), dazu A3 bis A7, B1 bis B3, C1. **Damit
  gebilligt, was ich gesetzt hatte:** im selben Kanal liegt der Clip des
  Anfangs über dem des Endes (E6); Sprung ins hintere Fenster mit beiden
  Stellen, ins vordere ohne das Ende (E7, E8); Halt im Fenster hält beide
  (E9); Loop aus im Fenster spielt das Ende aus (E10); Vorwärmen und „kein
  Sprung" am Umschlag bei jeder Schleife (E14); die Karaoke-Ebene blendet
  nicht mit (F6). `LumiViz.log` (fünf Läufe ab 22:21 Uhr, die Läufe zu
  Abschnitt A und B stehen nicht mehr darin): keine Warnung, kein Fehler,
  nach jedem Lauf „audio sources realigned 0 times".
  - **A2, durchgefallen — behoben in S100, im Programm nicht gesehen:** beim
    ersten Ausschalten blieb im Panel Player „Composer - <Song>" stehen, beim
    zweiten kam „No track loaded" (A6). Per Test gezählt: ein Player-Panel,
    das beim Ausschalten nicht zu sehen ist, hat kein Abo am Bus, verpasst den
    leeren Songwechsel und zeigt nach dem Einblenden den alten Stand. Naht N5,
    Anzeige, sechste Stelle: `PlayerPanel::onActivate` geht ohne Titel im
    Player und ohne laufenden Song des Composers in den Ruhezustand.
    **Nicht belegt, dass es bei Patrik dieser Weg war** — Indiz: die Stelle
    im Panel (01:45) lag rund 3,6 s hinter der der Komposition (11:48,2).
    Nachtest: A2 einmal mit sichtbarem, einmal mit verdecktem Panel Player.
  - **F7, Urteil Patrik — neuer Wunsch:** die Blende der ganzen Komposition
    soll **wie bei Clips** sein, auch editierbar (heute linear und fest).
    **Entschieden (Patrik S100): c — die ganze Pegelkurve wie am Clip:**
    Ein- und Ausblendung mit Kurve, Grundpegel und Stücke an frei wählbaren
    Stellen, in einer eigenen Spur unter dem Lineal, mit denselben Griffen
    und demselben Curve Editor. **F7-1 (Patrik S100): a — ein Stück liegt
    an fester Zeit**, hinter dem Ende wirkt es nicht und bleibt gespeichert.
    **Gebaut, beide Schritte (Konzept 0.46.0, §4; Benutzerhandbuch
    1.99.0):** Modell und Datei; die Zeile **MASTER** unter dem Lineal mit
    Griffen, Rechtsklick, Doppelklick und Curve Editor wie am Clip; die
    Keile im Lineal sind entfallen. Geändert: überdecken sich *Fade in*
    und *Fade out*, teilen sie den Platz wie am Clip (bisher galt der
    kleinere Wert). **Im Programm nicht gesehen** — nur per Unit-Test und
    am Bild `.master.png`. Von mir gesetzt, im nächsten Sichttest zu
    fragen: Name, Ort und Höhe der Zeile (das Panel braucht 36 Pixel
    mehr), Wegfall der Keile, Hineinrücken am Rand und Stauchen wie am
    Clip, die Beschriftung „Base level of the clip" im Curve Editor.
  - **B4:** Bemerkung „wurde doch überarbeitet" — die Erwartung der Liste
    war schon in S99 berichtigt; kein Befund.
  - **Nicht gespielt:** Abschnitt G ganz (*Visuals overlap themselves*, G1
    bis G6), D1 bis D4 (Text-Vorlage, Wechsel am Kreuzungspunkt, Kopieren
    ohne Edit), C2, C3, B5, A8, E1.
  - **Sichttest S100 (Patrik), als Nächstes:**
    `.claude/handover/Sichttests_S100.md` — 70 Punkte (dazu Abschnitt I, Icons): A (Nachtest Panel
    Player, sichtbar und verdeckt), M (die Zeile MASTER, mit den Urteilen
    zu dem, was ich gesetzt habe), L (Preset Late fade in), K (Kanalfarben),
    V (Kanäle verschieben),
    G (*Visuals overlap themselves*) und R
    (der ungespielte Rest aus S99). Reihenfolge bei knapper Zeit: A1 bis
    A4, M1 bis M9, G2, G3.
- **Composer, Stand S99 (Konzept 0.43.1), kurz:** der zweite Test der S98
  ist ausgewertet (33 von 33); die Lücke „Composer off ohne Titel im Player"
  ist geschlossen; die **Überlappung in der Schleife** und die **Blende der
  ganzen Komposition** sind entschieden und gebaut. **Im Programm ist davon
  nichts gesehen oder gehört** — als Nächstes Patriks Sichttest
  `.claude/handover/Sichttests_S99.md` (52 Punkte, A bis G), danach der
  Entscheid zu „Flipped und einfache Einblendung" und die Wünsche zur
  Zeitachse. Die Einzelheiten stehen im folgenden Block.
- **Composer, Stand S98 (Konzept 0.34.0) — Stufe C5b, Audio-Kanäle
  überlagern: alle fünf Schritte gebaut, das Überlagern im Programm
  gehört.** Die Songs aller eingeschalteten Audio-Kanäle laufen als Quellen
  in einem Mixer, die Uhr führt (`Tonfuehrung` neu); die Visuals lesen den
  Mixer (Naht N13); der laufende Song geht über den Bus an Karaoke, Tags,
  Tag View, Audio Browser und das Panel Player („Composer - <Song>", Naht N5
  Anzeige — Entscheid Patrik S98, ersetzt „zeigt die Komposition"); Tag View
  beim Einblenden (N15); Weiterspielen beim Wechsel Player / Composer
  einstellbar; Naht N7a zurückgenommen. Dazu die Bedienung der Zeitachse:
  Einfügemarke, Kopieren / Ausschneiden / Einfügen, Schatten beim
  Hineinziehen, Zeit unter dem Zeiger, Vorgabelänge neuer Clips (Konzept §4,
  §7, §8, §12.2; Benutzerhandbuch §15). **Hörtest Patrik (06.10. mittags,
  erster Teil):** 26 von 34 geprüften Punkten ohne Befund, kein Knackser; ein
  Fehler (Tag View, behoben), Nachsetzen im Log (Ursache gemessen, behoben).
  **Zweiter Test Patrik (06.10. abends, `Sichttests_S98b.md`, ausgewertet
  S99, Konzept 0.35.0):** 33 von 33 bestanden, keine Bemerkung — Korrekturen
  nach dem Hörtest, Weiterspielen, Einfügemarke, Kopieren / Einfügen,
  Schatten, Tooltip, Vorgabelänge im Programm gesehen. `LumiViz.log`: in
  allen 17 Läufen des Abends **kein Nachsetzen**, trotz langer Bilder bis
  191 ms. **Gebilligt, was ich gesetzt hatte:** beide Haken „Weiterspielen"
  Vorgabe an; die Einfügemarke rückt hinter den eingefügten Clip; ohne Marke
  landet die Kopie hinter dem angeklickten Clip; eine Kopie wird nicht
  gekürzt; der Tooltip nur im Lineal; kein Hinweis bei falscher Art.
  **Wellenform beim Kürzen eines Song-Clips:** wird abgeschnitten (Patrik
  S99) — erledigt.
  **Lücke „Composer off ohne Titel im Player" geschlossen (S99, Entscheid
  Patrik: a, Konzept 0.36.0):** die Sitzung meldet einen leeren
  Songwechsel, das Panel Player geht in den Ruhezustand (Naht N5, Anzeige,
  fünfte Stelle), Tag View zeigt „No track loaded". Vor dem Bau gezählt:
  auch Tag View zeigte weiter den letzten Song.
  **Offen, in dieser Reihenfolge:**
  - **Sichttest S99 (Patrik):** `.claude/handover/Sichttests_S99.md` — der
    Rest der ersten Liste, der im zweiten Test nicht vorkam (P13 bis P15,
    Start nach dem Beenden, Panel Player: Balken, Sprung des Balkens, Klick
    in der Überlagerung, Tag View ohne Vorsatz, Text-Vorlage `{title}`), das
    Urteil zum Wechsel am Kreuzungspunkt, und die geschlossene Lücke im
    Programm (auch Songtext und Host Switcher — per Test nicht belegt).
  - **Von mir gesetzt, in keiner Liste gefragt:** Kopieren geht auch ohne
    Edit. (Vorlauf 2 s und Abweichung 80 ms / Schonfrist 500 ms gelten als
    im Hörtest bestätigt.)
  - **Befund Patrik S99, noch nicht nachgestellt — Flipped und einfache
    Einblendung:** „Flipped shape" und ein einfaches Einblenden passen nicht
    zusammen; den Clip später beginnen zu lassen, nähme dem Effekt sein
    Einfahren. Aus dem Code gelesen (`VisualClip::pegelBei`): eine
    Einblendung beginnt immer am Clip-Anfang und rechnet Grundpegel × Blende
    (bei Grundpegel 0 also nichts); ein gekipptes Stück hebt vom Grundpegel
    auf voll und kehrt zum Grundpegel zurück; bei Grundpegel 100 % wirkt es
    gar nicht (so der CH3-Clip in `asset/singlecomp.lvcomp`). „Unsichtbar
    einfahren, später einblenden und oben bleiben" lässt sich damit nicht
    ausdrücken. **Zielbild von Patrik bestätigt (S99).**
    **Erledigt in S100 (Entscheid Patrik: a, Konzept 0.47.0):** per Test
    belegt, dass es doch geht — mit einem Stück am Clip-Anfang, dessen Kurve
    unten beginnt, ohne *Flipped*. Dafür das mitgelieferte Stück-Preset
    **Late fade in** und ein Absatz im Benutzerhandbuch (1.100.0, §15).
    Im Programm nicht gesehen; im Sichttest S100 Abschnitt L.
  - **Wünsche Patrik S99 zur Zeitachse (nicht begonnen, je als Entscheid
    vorlegen):** **(1) ist erledigt (S100, Entscheid Patrik K1: b, Konzept
    0.48.0, Benutzerhandbuch 1.101.0):** zwei Farbreihen (Bild kühl, Ton
    warm), die Farbe gehört dem Kanal und steht in der Datei, Rechtsklick →
    *Channel color...* / *Reset channel color*; im Programm nicht gesehen,
    Sichttest S100 Abschnitt K. **(2) ist erledigt (S100, Entscheid Patrik
    V1: a, Konzept 0.50.0, Benutzerhandbuch 1.103.0):** Kanäle am Namen
    ziehen oder per Rechtsklick *Move channel up / down*; die Reihenfolge
    bleibt die Mischreihenfolge der Bild-Kanäle; im Programm nicht gesehen,
    Sichttest S100 Abschnitt V. **Dazu (Wunsch Patrik S100):** bei *Fixed
    end* das Ende an der rechten Kante der Zeile MASTER ziehen — gebaut,
    Sichttest M24. **(3) ist erledigt (S100, Entscheid Patrik S1: a, Konzept
    0.51.0, Benutzerhandbuch 1.104.0):** Rechtsklick → *Sort channels* →
    *Visuals first* / *Audio first*, innerhalb der Art bleibt die
    Reihenfolge; im Programm nicht gesehen, Sichttest S100 V7 und V8.
    Offen bleiben (4) Zoom und Schwenken und (5) das Icon für `.lvcomp`.
    **Zu (4), Vorgabe Patrik S100:** der Zoom muss sich **zurücksetzen**
    lassen — **Ctrl+0** als bekanntes Kürzel (wieder die ganze Länge auf
    der Breite, wie heute). Das Urteil M17 liegt vor (Master-Zeilen 60 %).
    **Vorgabe Patrik S101, Abschluss — als Nächstes, in S102:** der Zoom
    „soll nicht nur in der Zeitachse zoombar sein" — **was noch zoombar
    sein soll, ist nicht gesagt** und wird zu Beginn von S102 mit Patrik
    geklärt (zuerst sein Zielbild erfragen, dann erst Optionen).
    Die Wünsche lauteten:
    (1) **Kanalfarben** — heute hat jeder Bild-Kanal einen
    eigenen Ton, alle Audio-Kanäle denselben (`kanalFarbe` / `tonFarbe` in
    `ComposerPanel.cpp`); gewünscht: beide Arten gleich behandelt, die Farbe
    je Kanal einstellbar. (2) **Kanäle von Hand verschieben** (die Reihenfolge
    frei, Bild und Ton gemischt wie heute). (3) **Rechtsklick: sortieren nach
    Typ**, evtl. mit Reihenfolge Typ → Name. (4) **Zoom** waagrecht und
    senkrecht, zusammen oder einzeln, mit Rollbalken; Mausrad mit
    verschiedenen Tasten; evtl. Schwenken mit linker / rechter Maustaste
    (Zoom der Zeitachse steht schon in Konzept §7 als fehlend).
    (5) **Ein Icon für Composer-Dateien** (`.lvcomp`; Wunsch Patrik am Ende
    von S99) — im Konzept als Teil der Naht N8 vorgesehen („Dateityp-Icon"),
    nicht gebaut. Vorbild: die Icons der Preset-Arten (`PresetIconKind`,
    `build_lvfx2_icons.py`). Vor dem Bau klären: wo es erscheinen soll
    (Effect Browser, Dateidialoge, Explorer über eine Registrierung des
    Dateityps) und wie es aussieht.
    **Stand S100 zu (5):** entschieden ist der Ort (Patrik, I1: a — Effect
    Browser zeigt `.lvcomp` mit Icon, Doppelklick öffnet die Komposition;
    Icon vor dem Dateinamen im Panel Composer; die Naht N8 wächst) und das
    Aussehen (I2: der Fächer; I2b a: mit Zeitstrahl und Abspielmarke).
    **(5) ist erledigt (S100, Konzept 0.52.0, Benutzerhandbuch 1.106.0):**
    `lumiviz_lvcomp.ico`, im Effect Browser gelistet (Doppelklick öffnet),
    Icon im Panel Composer; im Programm nicht gesehen, Sichttest S100
    Abschnitt I. Von den Wünschen zur Zeitachse bleibt nur (4) Zoom und
    Schwenken.
  - **Ziel Patrik S100: alle Datei-Icons im Explorer, vorerst Windows.**
    Die Liste der zugewiesenen Icons steht in
    `asset/img/logo/icons/README.md` (Tabelle oben, samt dem, was fehlt).
    Dazu das Skript `asset/img/logo/icons/register_file_icons.ps1`: trägt
    die Icons je Benutzer unter `HKCU\Software\Classes` ein (`-Apply`,
    fremde Endungen nur mit `-Foreign`, Audio nie), nimmt sie wieder heraus
    (`-Remove`), ohne Schalter zeigt es nur an. **Nur der Anzeige-Modus ist
    gelaufen**, `-Apply` und `-Remove` sind nicht ausgeführt. Nur das Icon,
    kein „Öffnen mit LumiViz" — der Öffnen-Befehl (`shell\open\command`)
    kommt erst ins Skript, wenn die Übergabe an die laufende App steht.
  - **`LumiViz.exe <datei>` (S101, Entscheid Patrik X1: a — eine Instanz,
    die Datei geht an die laufende App; Patrik arbeitet immer mit einem
    Fenster). Schritt 1 gebaut, im Programm ungesehen:** beim Start gilt
    die erste Angabe, die kein Schalter ist und eine vorhandene `.avs`,
    `.milk`, `.lvfx`, `.lvfx2`, `.lvkar` oder `.lvcomp` benennt
    (`UI/StartDatei.hpp`). Ein Preset läuft statt des zuletzt aktiven; ein
    Karaoke-Preset und eine Komposition kommen nach dem Wiederherstellen
    dazu (Komposition: Naht N8 wächst). Belegt per Test sind Auswahl, Art
    und Ladeweg — **nicht** die Zeilen im Hauptfenster (es läuft in keinem
    Unit-Test). **Schritt 2 gebaut, im Programm ungesehen (Entscheid X1b:
    a — eine Instanz je Exe-Datei, Schalter `--new-instance`, kein Haken in
    den Settings; „muss dokumentiert werden": Benutzerhandbuch §18, bis
    S108 §17):**
    `core/EinzelInstanz` — der erste Start nimmt eine Sperrdatei im
    Temp-Ordner und lauscht auf einem lokalen Kanal, jeder weitere gibt
    seinen Pfad ab und endet (Exit 0); das Hauptfenster kommt nach vorn und
    lädt mit den Rückfragen des Effect Browsers. Per Test belegt (zweiter
    Start in einem eigenen Thread): Übergabe mit und ohne Datei, Pfad mit
    Leerzeichen und Umlaut, nächster Start nach dem Ende, Sperre ohne
    Antwort → läuft für sich. **Nicht belegt:** zwei echte Prozesse; dass
    das Fenster unter Windows wirklich nach vorn kommt; Start und Ende der
    App mit der Sperre. **Grenzen, im Handbuch genannt:** antwortet die
    laufende App drei Sekunden nicht, startet der neue Aufruf für sich;
    kommt eine zweite Datei während des Hochfahrens, ist nicht festgelegt,
    welche danach läuft. **Offen:** (1) ⬜ Sichttest (unten); (2) 🟡 eine
    Audiodatei als Argument (spielen, anhängen, ersetzen?); (3) 🟡 ein
    Preset als Argument, während *Resume after restart* den Composer wieder
    einschaltet: heute liegt die Komposition dann über dem Preset; (4) der
    Öffnen-Befehl (`shell\open\command`) im Explorer-Skript — braucht den
    Pfad der Exe, die gelten soll.
    **Sichttest:** (a) Exe läuft, im Explorer eine `.lvfx` auf die Exe
    ziehen → kein zweites Fenster, das laufende kommt nach vorn und zeigt
    das Preset; (b) dasselbe mit `.lvkar` und `.lvcomp`; (c) Exe läuft
    nicht, Datei auf die Exe ziehen → startet mit ihr; (d) Exe ein zweites
    Mal ohne Datei → nur nach vorn, auch aus dem minimierten Zustand; (e)
    `LumiViz.exe --new-instance` → zweites Fenster; (f) Debug-App aus
    Visual Studio neben der Sichttest-Exe → beide laufen.
  - **Das Icon der App (Wunsch Patrik S100) — gebaut:** `LumiViz.exe` trägt
    eine Icon-Ressource (`projects/apps/LumiViz/main/LumiViz.rc` mit
    `lumiviz_app.ico`, eingetragen in `main/Source.cmake`, nur Windows). Am
    Build-System war nichts zu ändern; beide Bauarten bauen, das aus der Exe
    gelesene Icon ist das LumiViz-Icon. **Nicht gesehen:** Taskleiste,
    Titelleiste, Alt+Tab im laufenden Programm (Sichttest S100 I5) — zeigt
    das Fenster das Icon nicht, fehlt `setWindowIcon` beim Start.
  - **Wunsch Patrik, optional (P7):** Next / Previous springt in die Mitte
    des Übergangs statt an den Song-Anfang.
  - **Nur im Programm prüfbar, noch nicht eigens gefragt:** die Zeile in
    `MainWindow::onAudioUpdate` (N13 — das Bild reagierte im Hörtest).
  - **Überlappung in der Schleife (Frage Patrik S97): entschieden in S99,
    alle sieben Schritte gebaut — im Programm nichts davon gesehen oder
    gehört** (Konzept 0.43.0, §4; Benutzerhandbuch 1.98.0; Sichttest S99
    Abschnitte E, F und G). Nicht gemessen: was *Visuals overlap
    themselves* kurz vor dem Umschlag an Bildzeit und Grafikspeicher kostet — *Loop overlap* und
    *Fade in* / *Fade out* je Komposition, beide Fenster in der Zeitachse,
    zwei Haken für Clips in beiden Fenstern. Gebaut: Modell und Datei, Uhr
    und Bildquelle, das Bild im Fenster samt Vorwärmen über den Umschlag,
    der Ton im Fenster samt zweiter Quelle (Konzept 0.40.0). **Schon jetzt
    im Programm, nicht gesehen und nicht gehört:** im Loop — auch ohne
    Überlappung — werden die ersten Clips vorgewärmt, und der Umschlag
    setzt einen weiterlaufenden Song nicht mehr neu.
    Panel (Loop overlap, Songs overlap themselves) und Zeitachse (Fenster,
    zweite Marke) sind gebaut.
    Offen: Blende der
    ganzen Komposition, zuletzt *Visuals overlap themselves*. Im Programm
    wirkt noch nichts.
  - **Weiter offen aus S97:** Lyrics Sync bei führendem Composer; Songs
    mit verschiedenen Abtastraten im Mixer nicht gemessen.
- **Composer, Stand S97 (Konzept 0.28.1) — Stufe C5b, Audio-Kanäle
  überlagern** (*die Schritte 2 bis 5 und die Frage zu N7a darunter sind in
  S98 erledigt, siehe oben*)**:** **entschieden, Schritt 1 von 5 gebaut.** Entscheide Patrik:
  ein Mixer in der Engine (BASSmix), die Uhr führt; die Visuals sehen die
  Summe mit den Pegeln; der laufende Song ist der mit dem höchsten Wert der
  Pegelkurve, ihm folgen Karaoke, Tags, Songwechsel-Zähler, Tag View, Audio
  Browser; das Panel Player zeigt bei führendem Composer die Komposition
  (Konzept §4, §8, §11). Gebaut: Belege (`test_ComposerBelegeTon.cpp`), der
  Wechsel auf CMakeCraft v0.9.2 mit `BASS_MIX` an Kern, Runner und
  Test-Target, **Naht N12** — der Mixer in `IAudioEngine` / `BassEngine`
  (`test_ComposerNahtMixer.cpp`). **Im Programm überlagern sich die Songs
  weiter nicht** — der Composer benutzt den Mixer noch nicht.
  **Offen aus S97, in dieser Reihenfolge:**
  - **Schritt 2:** `Tonfuehrung` auf den Mixer umbauen (die Uhr führt, je
    Ton-Clip eine Quelle, Pegel je Takt), der laufende Song, `PappEngine` mit
    Mixer. **Erster Punkt der nächsten Session.**
  - **Schritt 3:** Naht N13 — `MainWindow::onAudioUpdate` liest Spektrum und
    Wellenform vom Mixer, solange der Composer Ton führt.
  - **Schritt 4:** Naht N5 wächst um die Anzeige des Player-Panels — etwa
    sechs Stellen in `PlayerPanel.cpp` (drei Ereignis-Empfänger,
    `onActivate`, `onProgressReleased`, `onLoopClicked`); vor dem Bau einzeln
    aufzählen.
  - **Schritt 5:** Sichttest-Liste, Benutzerhandbuch, Rückbau-Liste.
  - **Danach, vorgemerkt (Patrik S97):** in der Schleife überlagern sich Ende
    und Anfang der Komposition, Bild und Ton, Länge einstellbar. Frage
    Patrik: gilt ein Ein- und Ausblenden der ganzen Komposition nur in der
    Schleife, immer, oder ist es einstellbar. Die Vorlage folgt, wenn der
    heutige Sprung der Schleife gelesen ist (`Uhr::setSchleife`,
    `Wiedergabe`).
  - Nicht gemessen: ob der Mixer ohne Puffer unter Last aussetzt oder
    knistert (dann Puffer größer und die Pegel als Hüllkurve vorausplanen —
    die Hüllkurve ist belegt); Songs mit verschiedenen Abtastraten im Mixer.
  - Von mir gesetzt, nicht entschieden: der Mixer läuft mit der Abtastrate
    der Engine (44100).
  - Naht N7a (`IAudioPlayer::setPegel`) braucht C5b nicht mehr — nach dem
    Bau prüfen, ob sie zurückgenommen wird.
  - **Lyrics Sync bei führendem Composer** (Nebenbefund): das Panel springt
    und bremst direkt am Player (`LyricsSyncPanel.cpp:287`, `:300`, `:592`
    ff.) — mit dem Mixer wirkt das nicht mehr. Nicht eingeplant.
  - Die Rückbau-Liste führt `e7fe2d5` als Commit mit Naht und Eigenem
    zusammen (wie `356a868`).
- **Composer, Stand S96 (Konzept 0.26.1):** **C4 Teil 2 (Pegelform, Curve
  Editor, Stücke, Kurven-Presets) und C5 (Audiokanal) gebaut.** Die Pegelkurve
  eines Clips besteht aus Grundpegel, Ein- und Ausblendung (Dauer + Form) und
  Stücken mitten im Clip (Lage, Weite, Flipped + Form); die Formen sind
  zeitlos und als Presets speicherbar (`.lvfade`, `.lvshape`); zu kurze Clips
  stauchen die Teile, die eingestellten Werte bleiben. Fenster **Curve
  Editor**; Griffe für Stücke; Presets im Rechtsklick-Menü. **Audiokanal:**
  ein Clip-Typ für Bild und Ton, Songs als Clips mit Wellenform, die
  Komposition führt den Player (der Ton führt die Uhr, in Lücken die
  Wanduhr), die Playlist ruht, das Panel Player steuert den ganzen Composer;
  Nähte N7a, N7b, N5 (Transport). **Im Programm:** Patrik hat den Ton gehört
  (S96); sonst ist von S96 nichts gesehen — Liste
  `.claude/handover/Sichttests_S96.md` (zuerst J2, J3, dann A), dazu weiter
  `Sichttests_S95c.md` ohne Abschnitt H (zuerst J und K).
  **Offen aus S96:**
  - **Befund Patrik: Audio-Kanäle überlagern sich nicht.** Es spielt immer
    nur ein Song (der Player hat einen Stream); der Song eines zweiten Kanals
    setzt erst ein, wenn der des ersten zu Ende ist, und wird abgeschaltet,
    sobald auf dem oberen Kanal wieder einer liegt — Ein- und Ausblenden
    zweier Songs überlappen also nicht. **Erwartet:** die Kanäle spielen
    zugleich, jeder mit seiner Pegelkurve (Überblenden, auch drei Kanäle).
    Braucht je Audio-Kanal einen eigenen Stream — und dann muss die Uhr
    führen statt des Tons (Konzept §4, „kippt mit einem zweiten Audiokanal").
    **Erster Punkt der nächsten Session.**
  - Ob 30 Pegel-Schritte je Sekunde bei einer Blende als Stufen zu hören
    sind, ist nicht beurteilt (BASS kann gleitend setzen).
  - Die Schwellen der Führung (40 ms Toleranz, 1,5 s als Sprung) sind
    gesetzt, nicht gemessen.
  - In einer Lücke zeigt das Panel Player „pausiert", obwohl die Komposition
    läuft.
  - Im Curve Editor wirkt eine Änderung beim Loslassen der Maus, nicht
    während des Zugs.
  - Selbst gesetzt, nicht entschieden (Urteile K2, K3, H7 der Liste S96):
    Position eines Stücks = seine Mitte; Stücke halten beim Ziehen an
    Nachbarn an; Ease in / Ease out wirken an der Ausblendung umgekehrt wie
    in S95.
  - `test_PaletteUi.cpp:750` (Karaoke-Panel, Preset-Zeile „Abend") fiel im
    vollen Lauf zweimal um und läuft einzeln durch — vermutlich der
    500-ms-Zeitgeber des Karaoke-Panels; der Test sollte auf den Text warten.
    Kein Composer-Code, nicht angefasst.
  - C6 fehlt: PgUp / PgDn, Laden von Hand bei laufendem Composer, Lock.
  **Stand S95 (Konzept 0.16.1):** Stufen C2b, C3a, C3b, C3c und
  der erste Teil von C4 gebaut. Panel Composer (View → Panels → Composer)
  mit Composer on, Transport, Loop, Resume after restart; Bearbeiten im Panel
  (Schalter Edit: verschieben, Kanten, Kanal wechseln, löschen, Mischart,
  Undo, Save); New, Presets hineinziehen, Kanäle; Ein- und Ausblenden mit
  sieben Formen samt Bézier; Sync on Effect Chain mit Speichern der Kette
  eines Clips unter neuem Namen. **Im Programm gesehen:** C2b (Sichttest S95)
  und C3a (S95b: 17 bestanden, drei Anmerkungen, B3 unklar gefragt).
  **Nicht gesehen:** C3b, C4, C3c — Liste `.claude/handover/Sichttests_S95c.md`
  (A bis K; zuerst J und K; Abschnitt H ist durch `Sichttests_S96.md`
  ersetzt). Der Punkte-Editor der Pegelkurve ist in S96 gebaut.
  **Offen aus S95:**
  - Keine Rückfrage, wenn mit ungespeicherten Änderungen an der Kette eines
    Clips ein anderer Clip angeklickt oder der Composer ausgeschaltet wird.
  - Im Ruhezustand zeigt Effect Chain ihren allgemeinen Hinweistext; die
    Preset-Zeile dort kennt nur das Preset der Kette der App. Beides bräuchte
    eine Naht in `MultiEffectPanel`.
  - Der Wechsel eines Clips auf ein neu gespeichertes Preset läuft nicht über
    Undo.
  - Alt beim Ziehen (ohne Einrasten) ist im echten Programm nicht bestätigt.
  - Der Effect Browser lässt nur eine Auswahl zu: mehrere Presets auf einmal
    kommen nur aus dem Explorer.
  - Aus Konzept §7 fehlen: Lock, Follow player, Zoom der Zeitachse.
  - **Composer Variables** (Konzept §9e, entschieden: feste Variablen
    `comp_…` und freie `cv1` …; preset-weit, Namen geschützt, nur lesbar) —
    nicht begonnen.
  - ~~Die Rückbau-Liste (§12.4) ist vor dem Holen nach `master` zu prüfen; der
    Commit `356a868` enthält Naht N11 und eigenen Code zusammen.~~ ✅ hinfällig:
    `composer` ist überführt, §12 abgeschlossen (S105, M1: a); der
    Vergleichslauf wird nachgeholt (B2 a, S129, s. Block „Stand S101“).
  **Offen aus C2a:**
  - Nachlauf des Sichttests: `.claude/handover/Sichttests_S94b.md` — C1–C5
    und D1 offen.
  - Das erste Bild einer Komposition kostet 190–300 ms (Anlegen des ersten
    Renderers rund 100 ms, dazu das Laden); `list.lvfx2` mit 20
    Presetverweisen lädt 110–1100 ms im Bild.
  - Auf der Radeon 610M reichen fünf Clips gleichzeitig nicht für 60 Bilder
    je Sekunde (Median 24), der Grafikspeicher ist dann erschöpft.
  - Die Läufe auf der RTX 4090 streuten stark (938 gegen 1444 Bilder je
    Sekunde bei derselben Komposition); nicht geklärt.
  - Laden eines Presets bei laufender Komposition bleibt bis C6 ohne
    Rückfrage unsichtbar.
- **Movement-Tabelle in Scheiben (S94, betrifft jedes Preset):** in der App
  entsteht die Tabelle des Knotens Movement über mehrere Bilder verteilt
  (Settings → Performance → Movement Table; „At once" = wie das Original).
  Standalones, Vergleichslauf und Unit-Tests bauen weiter in einem Stück.
  **Offen:** wie nah das Gitter-Bild der ersten ein bis zwei Sekunden am
  genauen Bild liegt, ist nur im Sichttest beurteilt (F2 bestanden); wie viele
  Presets der Sammlungen Movement-Skripte mit `reg` / `megabuf` / `rand` /
  Audio haben, ist nicht gezählt; Dynamic Movement und andere Knoten sind
  nicht auf Kosten im ersten Bild untersucht (`202609_2b2_md.lvfx` zeigt noch
  130–145 ms). **Hinweis:** die Beobachtung „erster Wechsel auf ein Preset
  hakt kurz" (S85, weiter unten) und der Vollbild-Freeze (S84) können dieselbe
  Ursache gehabt haben — nicht geprüft.
- **Ausblick Timeline-Panel / Composit** (noch auszuarbeiten, eigene
  Konzept-Session): *n* Audiokanäle und *n* Visual-Kanäle zum Mischen;
  Einblenden/Ausblenden im Prinzip wie bei der Host-Gruppe (Crossfade,
  Kurven), aber **in der Timeline verwaltet** statt über Host Switcher.
  Anknüpfungen: Presetverweis (Visual-Clips als Verweis), Host-Gruppen-
  Crossfade (Kurven), Visual-Playlist P2, Idee „Video aufnehmen" (ein
  Timeline-Arrangement offline rendern).
- **Blättern und Timeline (Notiz Patrik S93, nichts gebaut):**
  - **Befund:** Bild ab / Bild auf blättern in der Liste, die der Effect
    Browser zeigt, ab der dort markierten Zeile — nicht ab dem laufenden
    Preset. Kommt das laufende Preset aus einem anderen Ordner (Preset-Zeile,
    File → Load), geht das nächste Blättern im Ordner des Browsers weiter.
    **Entscheid Patrik S93 (2a):** bleibt so bis zum Timeline-Konzept; der
    Browser wechselt nicht von selbst in den Ordner des laufenden Presets,
    dafür gibt es seit S93 den Knopf ◎.
  - **Offene Frage für das Konzept:** sollen Bild ab / Bild auf und die
    Kürzel für nächstes / voriges Lied bei aktiver Timeline überhaupt wirken?
    Idee Patrik: sie lassen sich bei Timeline **sperren**.
  - **Vorarbeit S93:** das laufende Preset sichert und lädt das Hauptfenster
    (`UI/StartPreset.hpp`, `MainWindow::setzeLaufendesPreset`), nicht mehr
    der Effect Browser. Eine Timeline setzt das laufende Preset über denselben
    Aufruf. Offen für das Konzept: was der Start wiederherstellt, wenn eine
    Timeline lief (ein Preset oder die Abfolge samt Position).
- **Ideen Patrik S88 zum Timeline-Panel** (Notiz, nichts gebaut; Umfang für
  die Konzept-Session):
  - **Visual-Kanäle:** mindestens zwei, weitere lassen sich hinzufügen.
  - **Pegelkurve je Kanal** (Skizze Patrik S88, zwei Kanäle über der Zeit):
    jeder Kanal hat einen Pegel, der über Rampen steigt und fällt.
    **Übergang** („trans", Patriks Wort „Transienten" = Ein-/Ausblenden):
    CH1 steigt, während CH2 fällt — der Wechsel von einem Visual zum
    anderen. **Überlagerung** („zwischendurch"): laufen in beiden Kanälen
    im selben Zeitbereich verschiedene Presets, kann der eine Kanal mitten
    im Clip teilweise zurückgehen und der andere teilweise dazukommen (in
    der Skizze beide auf etwa halbem Pegel, eine Weile gehalten) und danach
    wieder zurück — also nicht nur Wechsel, sondern auch Mischen auf Zeit.
    Rampen mit eigenen Zeiten je Kanal, so frei einstellbar wie möglich.
  - **Mischarten:** für Übergänge wie für Überlagerungen nicht nur
    Überblenden, sondern auch andere Verknüpfungen (Add, Sub, Xor usw.).
  - **Übergangs-Editor:** die Ein-/Ausblend-Kurve von Hand editieren.
    Kandidat zum Vorziehen: die Host-Gruppe hat schon Crossfade-Kurven, ein
    Editor könnte dort zuerst nützen.
  - **Dateiarten:** `.lvfx` = Einzel-Preset, `.lvfx2` = Preset-Stapel (mit
    Host Switcher), **eigenes Icon** für `.lvfx2` (heute teilen sich beide
    `lumiviz.ico`). Die Timeline kann ein `.lvfx2` abspielen, **warnt aber
    beim Hinzufügen**: dann wechseln zwei Wechsler (Timeline und Host
    Switcher) — möglich, nicht empfohlen. **Entscheid Patrik 1a (S88):** die
    Endung bleibt, wie sie ist (`.lvfx2` = Kette mit Host-Gruppe, auch ohne
    Switcher), bekommt ihr eigenes Icon; gewarnt wird nur, wenn die Datei
    wirklich einen Host Switcher enthält. **✅ Icon gebaut (S88):**
    `lumiviz_lvfx2.ico` („Stapel", Wahl Patrik aus drei Entwürfen), der
    Effect Browser zeigt es für `.lvfx2` (`PresetIconKind::NativeStapel`).
    Die Entwürfe „Marke" und „Fächer" liegen als Reserve daneben; Ideen
    (Marke für Knoten-Voreinstellungen, Fächer für eine Timeline-Datei) im
    README der Icons. **Sichttest offen (Patrik):** Icon im Effect Browser.
  - **Karaoke in der Timeline:** zuschaltbar, geschaltet etwa wie die Liste
    in `20261001.mix.lvfx2`, gesteuert über ein neues **Karaoke-Panel** mit
    zwei Reitern — (1) das Lyrics-Modul, (2) „Lyrics Trail": im Prinzip die
    Effect List aus dem Mix, die begrenzt Module aufnimmt (keine Render-
    Quellen, keine Hosts; **Umfang noch zu klären**). Auswahl über den
    vorhandenen Paletten-Dialog mit einem **übergeordneten Filter**, der
    nicht Erlaubtes vorab ausblendet. **Entscheid Patrik 2a (S88):** die
    Karaoke-Ebene wird eine eigene Ebene über allem Laufenden, schaltbar im
    Karaoke-Panel, und kann vor der Timeline gebaut werden (eigener Block,
    erst Kurzkonzept); die Timeline schaltet später denselben Schalter.
    **Der Lyrics-Knoten bleibt** — die Ebene ist ein weiterer Weg, kein
    Ersatz. **Die Effektkette gewinnt:** enthält das laufende Preset selbst
    Lyrics, zeichnet es sie (es ist womöglich auf das Preset abgestimmt) und
    die Ebene tritt zurück — gezeigt wird in beiden Fällen derselbe Songtext,
    nie doppelt. **Kurzkonzept (S88):**
    [Karaoke_Ebene_Konzept.md](visuals/Karaoke_Ebene_Konzept.md) — Einhängen
    beim Present wie der Post-Bloom (die Root-Surface bleibt unberührt),
    Stufen KE1–KE4. Entscheide Patrik S88: K1 a, K2 a, K5 a; K3/K4 vorläufig
    a. **✅ KE1 gebaut (S88):** die Ebene im Renderer mit festem Aussehen,
    Schalter Settings → Audio → *Karaoke layer*; Root-Surface bitgleich mit
    Ebene an und aus. **✅ Mischarten (K6 a, S88):** je Teil (Zeile, Trail)
    Normal · Additive · 50/50 · Subtract · Invert · XOR · Multiply · Maximum ·
    Minimum mit Stärke, Kontur bleibt normal; vorerst Settings → Audio.
    Entscheide jetzt vollständig: K3 b (Karaoke-Presets, Icon „Marke"), K4 b
    (Zeile in Fensterauflösung). **✅ Dieselben Mischarten am Lyrics-Knoten
    und am Lyrics Trail (S88):** Feld Blend mit neun Arten; Replace, Additive,
    50/50 bitgleich wie bisher, ab Subtract bleibt die Kontur normal.
    ✅ KE2–KE4 (Panel, Ablage mit Karaoke-Presets, Trail-Reiter,
    Menü/Kürzel), K4 b und das Feld **Blend strength** an Lyrics / Lyrics
    Trail sind in S89 gebaut (Changelog 1.82.0; `FieldBindings.cpp`
    `blendStrength`; bereinigt S129). **Offen:** **Sichttest Patrik** (über AVS-/MilkDrop-Presets, mit
    Post-Bloom, mit Render Scale).

### 🟡 Lautstärke im Loop — Stufe 27 gebaut, Sichttest und voller Lauf offen (S139)

Patrik S139: Instrumente klangen im Loop teils zu leise (Titanium). Gemessen (EBU R128): die
Instrumente streuten bei gleicher Probe um 21 LU. Entschieden LL1 a bis LL14 a und gebaut 27a bis
27g (`docs/audio/Loop_Konzept.md` §4n): Instrumente auf den Bezug −13,4 LUFS, Kits auf einen
Drum-Anker −21,89 LUFS, Fader je Kanal, Limiter im Loop-Panel, *Match* im Creator. **Offen:**

- **Sichttest** `.claude/handover/Sichttest_Lautstaerke.md` (lokal), Urteile U1 bis U6.
- **Drei gekappte Kit-Rollen** (LL11 a): Crash in Rock'n'Roll (5,5 LU), Snare in Strange und Clap in
  Lo-fi (beide `lib:snap`, 5,4 und 3,1 LU) — so lassen oder einen lauteren Klang wählen.
- **Fader beim Ziehen** klingt erst beim Loslassen (live wäre besser zum Mischen nach Gehör).
- Der Name eines ausgeschalteten Kanals wird mit „(off)" früh gekürzt.
- *Load instrument* als Schichten legt das Volume in die Schichten, deren Level bei 1 endet — ein
  Instrument über 1 wird dabei leiser.
- Eigene Instrumente mit Sample-Schicht werden im Fenster *Instruments* erst gemessen, wenn die
  Aufnahme geladen ist (dann ohne Marke).
- Drums liegen beim Anlegen 8,5 LU unter den Instrumenten (LL10 a) — U2 im Sichttest.
- **Voller Lauf** der Stufen 26 und 27 (Patrik: noch nicht).

### 🟡 Umstellung auf Englisch fortsetzen (Entscheide Patrik S83; S84 bis auf die Screenshots erledigt)

- ✅ **Feld-Tooltips — Entscheid 1a (S84):** die Doku-Kommentare an den Feldern
  der 92 `…Params`-Structs sind englisch (673 Kommentare, `//`-Notizen bleiben
  deutsch); Muster „Parameter script (Strang D): …", „Script (Block B, S79): …".
  Der Ernter erkennt Skriptvariablen an der Muster-Form `script (…):`
  (`SKRIPT_MUSTER`) — ein beiläufiges „the script's alpha" zählt nicht mehr;
  dadurch entfallen zwei Zufallstreffer der alten Regel (`dynamicMovement.nomove`:
  `alpha`; `superScope.pointCount`: n/i/v/…, für den Lint fallen bei SuperScope
  `i`/`v` weg — beides Eingaben, keine gelesenen Variablen). `FieldDocs.cpp` +
  Inventar neu, Wächter-Test „Feld-Tooltips sind englisch" (mit Gegenprobe).
- ✅ **Kamera-Clip-Dateiname — Entscheid 2a (S84):** neue Clips
  `camera_clip_<Datum>_<Zeit>.mp4`; kein Skript/keine Sonde suchte nach
  `testaufnahme_*`; Ordner `testaufnahmen` bleibt (vorhandene Clips gültig).
- Handbuch-Screenshots mit alter Oberfläche neu aufnehmen
  (`milkdrop_puffer_wechsel.png`, `settings_milkdrop_wechsel.png`).

### ⚪ Später: Code-Kommentare englisch (eigene Session, geringe Priorität)

Entscheid Patrik S83: alles wird englisch — die Code-Kommentare auch, aber in
einer separaten Session und ohne Eile. Die Projekt-Konvention „Kommentare
deutsch" ändert sich damit; bis dahin gilt sie weiter.

### ⚪ Später: Sprachwahl der Oberfläche (i18n)

Idee Patrik S83: Oberflächenbegriffe in mehreren Sprachen, dann auch wieder
Deutsch. Voraussetzung ist erfüllt: alle Oberflächentexte laufen über
`tr()` und sind englisch (Quellsprache). Weg: Qt Linguist (`lupdate` →
`.ts` je Sprache → `lrelease` → `.qm`, `QTranslator` beim Start/Umschalten).
Offen: was nicht über `tr()` läuft (Referenzseiten als `QStringLiteral`,
Feld-Tooltips aus `FieldDocs.cpp`) braucht einen eigenen Weg.

### ⚪ Idee: Video Source — nur die Felder der gewählten Quelle zeigen (Befund S83)

Der Editor des Video-Source-Knotens zeigt die Felder **aller drei Quellen
gleichzeitig** (Videodatei + „Videodatei wählen…", Kamera + „Kamera
freigeben", Testaufnahme), obwohl immer nur eine gilt — man sieht nicht, dass
etwa „Videodatei" bei Quelle *Kamera* nichts bewirkt. Ebenso wirken *Tempo*
nur ohne Echtzeit-Streaming und *Schleife* nur bei Datei/Testaufnahme.
Vorschlag: unwirksame Felder ausgrauen (Wert bleibt sichtbar) oder
ausblenden; Tooltip nennt den Grund. Anmerkung Patrik S83: der Knoten ist ein
**fertiges Modul, kein Prüfstand** — „Testaufnahme" ist darum ein
irreführender Name für die Kamera-Clips (Umbenennung in §8, Oberflächentexte).

### ⚪ Idee: Bild an andere Programme weitergeben — Spout / NDI (Wunsch S83)

Anlass: Einordnung neben VDMX und MadMapper (S83). LumiViz ist dort die
**Quelle** — es erzeugt das Bild, VDMX mischt, MadMapper mappt. Die Brücke
fehlt: **Stand S83 gibt es weder Spout noch NDI** (Suche im Code: nichts;
einzige Spur der Konzeptabschnitt „SpoutSyphonNode" im Referenzhandbuch des
Vorgängers, `harvest/konzepte/viz_2025_node_reference_manual_visualizer_pipeline.md`
§9.4 — Eingang Image, Parameter Stream Name).
- **Spout** (Windows, GPU-Textur-Teilen ohne Kopie, Latenz nahe null) — der
  naheliegende erste Schritt; Gegenstück Syphon wäre nur für macOS.
- **NDI** (Netzwerk, auch rechnerübergreifend; SDK mit eigener Lizenz —
  vor dem Einbau prüfen, wie die Lizenz zu BASS/Lua/Qt passt).
- Offen: als **Knoten** in der Kette (Abgriff an beliebiger Stelle, wie der
  harvest-Entwurf) oder als **Ausgang des Visualizer-Fensters** (immer das
  Endbild) — oder beides. Auflösung/Bildrate des Ausgangs getrennt vom Fenster?
- Umgekehrt denkbar: Spout/NDI als **Quelle** (neben Video/Kamera, §13 im
  Handbuch) — Bild aus VDMX/Resolume in die Kette holen.

### ⚪ Idee: Steuerung per MIDI / OSC (Wunsch S83)

Anlass: Vergleich mit Synesthesia (S83) — dort liegen Regler, Szenenwechsel und
Effekte auf MIDI-Controllern oder kommen per OSC aus anderer Software. **Stand
S83:** nichts davon im Code; Bedienung nur über Maus und Tastenkürzel
(`ShortcutManager`, Hotkey-Konzept).
- **MIDI-Learn:** Regler im Editor anklicken, Controller bewegen → gebunden.
  Ziel sind dieselben Felder, die ein Skript setzen kann — Anknüpfung an
  **Block F** (generischer Weg für Modulparameter), damit MIDI, OSC und Skript
  eine gemeinsame Adressierung der Felder nutzen statt dreier eigener.
- **Aktionen:** Preset/Host-Gruppe wechseln, Host Switcher anstoßen
  (`nextgroup`), Tap-Tempo, Script active umschalten.
- **OSC:** dieselben Adressen über UDP (z. B. `/lumiviz/<knoten>/<feld>`),
  in beide Richtungen denkbar (Rückmeldung an Controller-Apps).
- Zu klären: Bibliothek (Qt hat kein MIDI — RtMidi o. ä.; OSC ist über
  `QUdpSocket` klein), Speicherort der Belegung (je Kette oder global),
  Glättung/Wertebereich je Feld.

### ⚪ Idee: Video aufnehmen — Bild + Ton als Datei (Wunsch S39/S83)

Bisher nur als Stichwort in der Aufzählung unten („Video-Capture-Modul").
**Stand S83, genau geprüft:** die App kann das Bild **nicht** als Video
aufnehmen. Vorhanden sind Bausteine:
- **Screenshot** in der App (Tastenkürzel): EIN Bild, per `requestCapture()`
  auf dem Render-Thread (`glReadPixels`), `VisualizerRenderThread`.
- **Bildfolgen** nur im Entwickler-Werkzeug `AvsStandalone`:
  `--auto --save-every M` schreibt jeden M-ten Frame als PNG (für Sequenz-/
  GIF-Vergleiche mit AvsRef, S46) — ohne Ton, mit synthetischem oder
  Datei-Audio, nicht in der App. `MilkdropStandalone` hat das nicht.
- **Aufnahme mit `QMediaRecorder`** gibt es schon — aber nur für die
  **Kamera-Testaufnahmen** (Einstellungen → Kamera, S70): sie nimmt das
  Kamerabild auf, nicht das Visual.
- **Der Weg ist in Qt da:** installiert ist Qt 6.10.1; seit 6.8 nehmen
  `QVideoFrameInput` + `QAudioBufferInput` in einer `QMediaCaptureSession`
  selbst erzeugte Bilder und Audio-Puffer an und schreiben sie über
  `QMediaRecorder` (FFmpeg-Backend, das wir für Video-Quellen ohnehin nutzen)
  in eine Datei.

**Wozu** (Wunsch S39 + S83): Visuals teilen (Clip für Social Media, Demo,
README-Schaufenster), ganze **Sets** festhalten (Playlist mit Host Switcher,
passt zur Visual-Playlist P2), Material für den Videoschnitt, und — nebenbei
— Belege für Sichttests und Kalibrier-Befunde (heute Screenshots/GIFs).

**Zwei Aufnahmearten** — beide gebraucht, sie lösen verschiedene Aufgaben:

| | **Live-Aufnahme** | **Song rendern (offline)** |
|---|---|---|
| Was | nimmt auf, was gerade läuft — samt Bedienung, Szenenwechseln, Kamera | rechnet einen Titel Bild für Bild mit fester Bildrate durch, schneller oder langsamer als Echtzeit |
| Takt | Uhr/Vsync; Ruckler landen im Video | Sim-Uhr, deterministisch — dieselbe Technik wie `AvsStandalone --audio-datei` |
| Ton | Player-Strom (BASS), mitgeschnitten | Audiodatei dekodiert, jede Bildzeit bekommt genau ihr Audio-Fenster |
| Auflösung | Fenster- oder eigenes Zielformat | frei wählbar, auch 4K auf schwacher Karte (dauert nur länger) |
| Live-Quellen | Kamera, externes Audio möglich | nur Datei-Quellen (Kamera ist nicht wiederholbar) |

**Bedienung (Vorschlag):** Start/Stopp per Menü + Tastenkürzel, sichtbare
REC-Anzeige mit Laufzeit und Dateigröße; aufgenommen wird **nur die
Visualizer-Fläche** (keine Panels). Dialog je Aufnahme: Auflösung, Bildrate,
Qualität (Voreinstellungen „Teilen / Hoch / Schnitt"), Zielordner, Dateiname
aus Preset/Titel. „Song rendern" im Kontextmenü der Playlist (ein Titel oder
die ganze Liste → je Titel eine Datei oder ein Film).

**Technik:**
- Bild vom **Render-Thread** abgreifen (GL nur dort, wie der Screenshot);
  **asynchron** über Pixel-Buffer-Objekte (PBO), damit `glReadPixels` den
  Frame nicht blockiert; Kodieren in einem eigenen Thread, Warteschlange mit
  Grenze — Live: lieber ein Bild doppeln als die Anzeige bremsen.
- Eigenes Render-Ziel in Aufnahmegröße (FBO), unabhängig vom Fenster.
- Schreiben über `QVideoFrameInput`/`QAudioBufferInput` → `QMediaRecorder`
  (Qt ≥ 6.8, FFmpeg-Backend). Ausweichweg: FFmpeg als externer Prozess per
  Pipe (volle Codec-Kontrolle, aber eigene Beschaffung).
- **Derselbe Bild-Abgriff trägt Spout/NDI** (Idee oben) — einmal bauen,
  dreimal nutzen (Datei, Spout, NDI).

**Formate und Lizenz:** H.264/MP4 ist der Teilen-Standard, trägt aber
Patentfragen (Encoder über FFmpeg/OS); lizenzarm sind VP9/AV1 in WebM. Für
den Schnitt ein verlustarmes Format (etwa ProRes/MJPEG, groß). Vor dem
Einbau prüfen, welche Encoder das FFmpeg-Backend von Qt unter Windows
mitbringt und was davon wir weitergeben dürfen (THIRD_PARTY_NOTICES).

**Tests:** Offline-Render zweimal → identische Bilder (Prüfsumme je Frame);
Bild-Ton-Versatz messen: ein Kick-Signal aus `TestAudio` (07–09, 60/120/180
BPM, S45) gegen einen Knoten, der auf dem Beat weiß blitzt; Live-Aufnahme: keine verlorenen Frames bei 60 fps in
1080p auf der Referenzkarte, Frametime der Anzeige unverändert.

**Offen:** Ton auch bei externem Audio-Eingang (Idee unten) · Aufnahme
eines einzelnen Knotens statt des Endbilds (Zwischenstand der Kette) ·
Alpha-Kanal für Overlays (nur mit passendem Codec).

### ⚪ Idee: Cover-Modul — mit dem Albumcover arbeiten (Wunsch S84)

Anlass: seit S84 zeigt der Audio Browser eingebettete Cover als Icon (Lesen
aus ID3/FLAC/MP4, sonst `cover.jpg`/`folder.jpg` im Ordner, sonst eigenes
Icon). Dieselbe Quelle kann das **Bild** speisen. Fallback, wenn es kein Cover
gibt: Ordnerbild → (optional, nur mit Freigabe) Online-Suche → LumiViz-Logo
oder ein aus dem Titel erzeugtes Muster — nie ein leeres Bild.

Vorschläge (Claude S84):
- **Cover als Bildquelle statt eigenem Knoten** — zuerst eine Quelle
  „Current cover" für alle vorhandenen Bild-Eingänge: Picture/Picture II,
  Texer, Milkdrop-Sprites, Shadertoy-`iChannel`, ISF-Bildeingang. Ein Weg,
  jeder Knoten kann es (SSOT), und die Kette wechselt das Bild beim
  Titelwechsel von selbst.
- **Cover-Knoten** darüber für das Typische: zentriert / füllend / „Vinyl"
  (rund, dreht mit dem Tempo), **unscharfer Hintergrund + scharfes Cover
  davor**, Puls mit dem Bass, Kacheln/Mosaik, Zerfall in Partikel beim
  Titelwechsel (Anbindung an Song-Slot und Crossfade der Host-Gruppen).
- **Farben aus dem Cover**: dominante Farben (k-means über ein kleines Bild)
  als Farbverlauf und Skriptvariablen (`cover_r/g/b`, `cover_hue` …) — die
  ganze Kette färbt sich passend zum Album, auch ohne das Bild zu zeigen.
- **Online-Cover** (MusicBrainz Cover Art Archive o. ä.) nur als Opt-in:
  sendet Titeldaten nach außen (Datenschutz), Cache im Benutzerordner.
- Zu klären: Ogg/Opus (`METADATA_BLOCK_PICTURE`, Base64 im Kommentar), WMA,
  mehrere Bilder je Datei (Vorder-/Rückseite, Typ 3 = Front bevorzugen).

### ⚪ Idee: Text-Knoten ausbauen — Listen, Tags, Schrift (Wunsch S84)

Es gibt den Knoten **Text** schon (AVS „Render / Text", `TextParams`): Wörter
mit `;` getrennt, Wechsel nach Frames oder auf den Beat, zufällige Position,
Schrift (Name, Höhe, fett/kursiv), Kontur/Schatten. Ausbau statt neuem Modul —
AVS-Import bleibt damit treu, alles Neue sind zusätzliche Felder.

**Stand S86:** der Knoten steht in der Effekt-Palette, hat Init/Frame/Beat-
Felder und mitgelieferte Voreinstellungen. **Nachfragen Patrik S86:**
(1) *Schrifttyp wählen* — heute ein freies Textfeld „Font" (Name eintippen);
fett gibt es nur als Zahl „Weight" (700), kein Häkchen neben Italic/Underline.
**✅ S86 umgesetzt:** Schrift-Auswahlliste der installierten Schriften
(`QFontComboBox`, weiter frei editierbar für importierte Namen) + Häkchen
**Bold** (700/400, im Gleichschritt mit Weight). Offen: mitgelieferte
Schriftdateien, echte Zwischengewichte (der Renderer kennt nur normal/fett).
**Befund Patrik S86 — Größenwechsel:** beim Ziehen am Fensterrand verlor der
Oscilloscope Star den Blur-Nachzug und der Text flackerte. Ursache: jeder
Größenwechsel verwarf den Bildspeicher (`ensureSurfacePair`, „Resize policy
5.2: discard"). **✅ behoben:** das Bild wird in die neue Größe übernommen
(Blit, wie AVS `config_reuseonresize`) — Wurzel, Effect List, Host-Gruppe;
Test „Größenwechsel S86". Das gemeldete Flackern „in anderen Farben" ließ
sich im Renderer nicht messen (weiße Kette: nirgends Farbe, auch während
des Größenwechsels) — vermutlich der Wechsel zwischen scharfem und
verwischtem Text; **Sichttest Patrik offen**. Nicht erfasst: Puffer
einzelner Knoten (Milkdrop-Node, Feedback-Puffer, Buffer Save).
(2) *Titel/Interpret aus den ID3-Tags* — **✅ S86 umgesetzt:**
Platzhalter `{title} {artist} {album} {year} {genre} {track} {file}` im
Textfeld, wahlfreie Abschnitte `[ … ]` (entfallen bei leerem Tag), Knopf
„Insert tag", Voreinstellung „Now Playing" (`TextTags.hpp`; Tags laufen mit
`TrackChangedEvent` über `notifySongChanged(tags)` zum Renderer; Jahr, Genre
und Tracknummer liest `BassEngine::getMetadataExtra` neu). **Nicht an echten
Dateien geprüft:** das Lesen von Jahr/Genre/Track je Format (ID3v2.2/2.3/2.4,
ID3v1.1, Vorbis, APE, MP4) — nur nach dem Format gebaut; Sichttest Patrik.
Offen: Laufzeitwerte (Zeit, BPM, Presetname), Ein-/Ausblenden beim
Titelwechsel.
**Panel „Tag View" (✅ S86, Wunsch Patrik):** zeigt Tags, Datei, Dauer und
Cover (256 px) des laufenden Titels — das Prüfwerkzeug für das Tag-Lesen
(`TagViewPanel`, id `tag_view`). Abtastrate, Kanäle und Bitrate bleiben leer:
`AudioPlayer` füllt diese `TrackInfo`-Felder nicht. **Ideen Patrik S86
(nichts gebaut):** Tags im Panel **nachträglich eintragen** (Schreiben in die
Datei — braucht eine Tag-Bibliothek, BASS liest nur; Kandidat TagLib,
Lizenz LGPL/MPL prüfen) und **Online-Datenbank durchsuchen** wie beim
CD-Rippen (Kandidat MusicBrainz; früher freedb/CDDB — nur als Opt-in, sendet
Titeldaten nach außen).
**Songtexte — Entscheide Patrik S86 (1a–7a) und Stand:**
(1a) eigener Knoten **Lyrics** · (2a) Suchreihenfolge synchron vor unsynchron,
nah vor fern · (3a) Online-Suche nur als Opt-in, nach den lokalen Quellen ·
(4a) ein Online-Fund wird gespeichert, Einstellung *User folder* (Vorgabe) /
*Next to the song* · (5a) synchron an der Abspielposition mit Versatz,
unsynchron als Block oder alle n Beats, gekennzeichnet · (6a) Zeile +
Vorschau, Wort für Wort nur mit echten Wort-Zeiten · (7a) Stimm-Marken →
Farbe je Stimme.
**✅ S86 gebaut (Stufe 1):** `LyricsModel.hpp` (Modell Zeile/Wort/Stimme,
LRC lesen und schreiben — die EINE Schnittstelle für alle Quellen und
Werkzeuge), `audio/SongtextQuelle` (Suche 1–6, `speichereSongtext`),
`BassEngine::getLyrics` (ID3 `USLT`/`SYLT`, Vorbis/APE/MP4 `LYRICS`), Knoten
`lyrics` (`LyricsParams`, ans Ende der Variante) mit Vorschau-Zeilen,
Wort-für-Wort-Füllung, Duett-Farben, Versatz, unsynchronem Block,
Formelfeldern und drei Voreinstellungen; Abspielposition über
`notifyPlayPosition/-State` (zwischen den Meldungen des Players zählt die
Wanduhr weiter). Tests: Modell, Suchreihenfolge an Dateien, Bild.
**Nicht an echten Dateien geprüft:** das Lesen eingebetteter Texte
(`USLT`/`SYLT`/Vorbis) — nach dem Format gebaut; `SYLT` nur im Zeitformat
Millisekunden. LRC-`[offset:]` als „positiv = früher" angenommen (aus dem
Gedächtnis; gegen echte Dateien prüfen). Die Duett-Marken (`v1:`/`v2:`/
`v1+v2:`, `M:`/`F:`/`D:`) sind eine eigene Festlegung, keine Norm.
**✅ S86 gebaut (Stufe 2): Online-Abruf** (3a/4a) — `audio/SongtextOnline`,
Dienst LRCLIB `GET /api/get` (Dokumentation https://lrclib.net/docs gelesen
am 2026-10-01: kein Schlüssel, `User-Agent` mit Name/Version/Projektseite
verlangt, Pflicht `track_name` + `artist_name`, Treffer nur bei Dauer ±2 s,
Antwort `syncedLyrics`/`plainLyrics`, 404 = kein Text, 429 mit `Retry-After`
ist zu befolgen). Einstellungen `lyrics/onlineSearch` (Vorgabe aus) und
`lyrics/saveLocation` (Settings → Audio); gefragt wird nur, wenn lokal nichts
gefunden wurde; der Fund wird als `.lrc` (synchron) oder `.txt` gespeichert,
bei gesperrtem Song-Ordner im Benutzerordner. Die dokumentierte
Beispielanfrage lieferte am 2026-10-01 genau die erwartete Form. Tests ohne
Netz: Anfrage, Antwort, Speicherort. **Abruf aus der laufenden App belegt
(Lauf Patrik 2026-10-01 15:58–16:00):** sechs gespeicherte Dateien im
Benutzerordner `%APPDATA%\LumiViz Project\LumiViz\lyrics` (zwei `.lrc`, vier
`.txt`). **Nicht geprüft:** das Verhalten bei 429.
**Befund Patrik S86, behoben:** die Zeile „Lyrics" im Editor meldete „No
lyrics found", während ein Text lief — sie wurde nur beim Öffnen der Seite
gefüllt. Sie läuft jetzt mit (500 ms); neu Knopf **Open folder** für den
Ordner der gelesenen Datei (`Songtext::datei`).
**Herkunftsvermerk (Entscheid Patrik S86: in der Datei, keine `source.txt`):**
ein online geholter Text trägt die Kopfzeilen `[re:LumiViz]` und
`[source:LRCLIB id <n>, fetched <Datum>]` (`Songtext::herkunft`), in `.lrc`
wie in `.txt`; der Leser überspringt sie, die Quellzeile im Editor zeigt sie.
Die sechs vor dieser Änderung gespeicherten Dateien tragen keinen Vermerk.
`[source:]` ist eine eigene Festlegung, keine Norm — ein fremder Player
zeigt die Kopfzeilen einer `.txt` womöglich als Text.
**✅ Befund Patrik S86 „online geholter Text erscheint erst beim nächsten
Abspielen" — in S87 geklärt und behoben.** Keine der drei vermuteten Spuren
(Thread, Pfadvergleich, zweites Ereignis) traf zu: der Lauf mit Diagnose-
Zeilen (62 Anfragen) zeigte 19-mal **HTTP 503 mit `Retry-After: 1`** — der
Dienst ist beschäftigt, und LumiViz fragte danach nicht wieder. Die
Dokumentation nennt das selbst („503 ServerOverloaded … retry after the
Retry-After header (usually 1 second)"). Jetzt: Wiederholung für den laufenden
Titel nach 2/5/10/20/40 s (`songtextWartezeitMs`), 400 ms Atempause vor der
ersten Anfrage („Request Throttling"), Zustand des Abrufs (`lyrics::Abruf`).
**✅ S87 gebaut:** Lyrics-Angaben als **Tabelle** mit Status
(„…downloading…"); **Alternatives…** (Suche `/api/search` mit eigenen
Begriffen, Zeilenzahl, Zeitmarken farbig, bis zu drei Fassungen nebeneinander,
Balken an der Abspielposition, nicht modal); **Wrong lyrics** mit Sperrliste
(`blocked.json`, Settings → Audio); zweite Suchstufe ohne Zusätze in Titel
und Interpret („(Video Version)", „feat. …"), übernommen nur bei passender
Dauer; Fassungen eines Songs im Benutzerordner nach Dauer getrennt
(`[length:]`, `… [m-ss]`); wiederholte Zeilen blenden vor dem Einsatz.
**Notiz Patrik S87 — nicht weiterverfolgen:** zu „Gettin' Over" (David
Guetta, auf *Greatest Hits* und *One Love*) führt LRCLIB offenbar keine
Fassung, die wirklich passt. Bewusst liegengelassen („Phantome jagen bringt
nichts").
**Nicht geprüft (S87):** eine echte Suche über den Dialog gegen den Dienst
durch Claude (nur nachgestellte Antworten); das Verhalten bei 429; ob das
Spulen im Titel den Text mitnimmt.
**✅ S87 gebaut — Knoten „Lyrics Trail"** (Idee Patrik S87, Typ `lyricsTrail`,
`LyricsTrailParams` am Ende der Variante): zeichnet die eben beendete Zeile
für `holdMs` noch einmal an seiner Stelle der Kette; Schrift und Lage vom
Lyrics-Knoten (`source` = Name, leer = der erste), eigener Versatz, optional
eigene Farbe, Formelfelder. Kein eigener Puffer — das Verschwinden machen die
Effekte danach. Rastern teilt er mit dem Lyrics-Knoten
(`zeichneLyricsZeilen`). Beispiel im Mix `20261001.mix.lvfx2`: Effect List
(Additive, behält ihr Bild) mit Blitter Feedback `scale` 35, Fadeout 4,
Trail. **Befund bei der Abstimmung:** `scale` 33 bewegt die Zeile unter einem
Pixel je Frame, das Fadeout frisst die Vorderkante — sie schrumpft zum Strich;
Blur taugt dabei nicht (ohne `roundUp` löscht er dünne Striche, mit `roundUp`
bleibt ein Grauschleier über der ganzen Liste). **✅ S88: beide Reste des
Trails gebaut** — er liest die Felder des Lyrics-Knotens, wie dieser sie
zuletzt gezeichnet hat (nach seinem Skript, `LeafRuntime::lyricsEffektiv`;
„Karaoke Bounce"/„Flip" nehmen die alte Zeile mit), und er kennt
unsynchronen Text (die Zeile vor dem letzten Beat-Wechsel, Haltezeit auf der
Sim-Uhr; braucht dort den eingeschalteten Lyrics-Knoten). Kein Vergleichslauf:
der Pfad betrifft nur Lyrics/Lyrics Trail. **Sichttest offen (Patrik):** Trail
mit „Karaoke Bounce" und mit unsynchronem Text. **Sichttest offen (Patrik):** Wirkung im Mix über laufendem
Bild (geprüft nur auf Schwarz, an gerenderten Bildern).
**Möglich, nicht gebaut:** falsche Texte an den Dienst melden (`POST
/api/flag`, verlangt je Meldung einen Rechen-Nachweis wie `/api/publish`);
selbst synchronisierte Texte veröffentlichen (`/api/publish`) — beides nur
auf ausdrücklichen Wunsch, es geht etwas hinaus.
**✅ S87 gebaut: Panel „Lyrics Sync"** (`UI/panels/LyricsSyncPanel`, id
`lyrics_sync`, View → Panels): Zeilenliste mit Zeiten, Tap (Knopf, Enter, T),
±100 ms je Zeile / ab hier, Zeit löschen, Zeit von Hand (F2), Vorhören per
Doppelklick (1,5 s Vorlauf), Vorschau sofort am Knoten, Speichern als `.lrc`
neben der Quelle — nur wenn jede Textzeile eine Zeit hat (der LRC-Leser
verwirft Zeilen ohne Zeit). Ungespeicherte Arbeit bleibt beim Titelwechsel
stehen. **Nicht gebaut:** Wort-Zeiten, automatisches Verteilen.
**✅ S88 gebaut: Undo / Redo** (Knöpfe, Ctrl+Z / Ctrl+Y in der Liste; bis
200 Schritte, Speichern und Laden leeren die Geschichte; der Versatz je Titel
gehört nicht dazu). **Sichttest offen (Patrik):** ob Ctrl+Z in der Liste beim
Panel ankommt und nicht beim Undo der Effektkette (im Test nur als direkt
zugestellter Tastendruck geprüft).
**Sichttest offen (Patrik):** das Panel im Betrieb — ob Enter/T beim Klopfen
ankommen (die Leertaste gehört dem Player), Bedienfluss, Breite der Knöpfe.
**Offen, zurückgestellt (Freigabe Patrik S88):** Wort-Zeiten des Dienstes —
laut lrclib.net/docs (gelesen 2026-10-02) stehen sie nicht im LRC-Text,
sondern im Feld `lyricsfile`, einem YAML-Dokument mit `lines[].words`
(`hasWordSync`); das Format beschreibt die Seite nicht, einen YAML-Leser hat
LumiViz nicht. Erst Formatbeschreibung oder Beispieldatensätze, dann Leser.
**S88 Lyrics-Editor:** Knöpfe in zwei Reihen (Alternatives…/Wrong lyrics,
Lyrics file…/Open folder), „Offset for this track" als eigene Formularzeile
(Wunsch Patrik nach Schnappschuss).
**✅ S88 gebaut: Knopf „Lyrics file…"** (Entscheid Patrik: kopieren) —
`audio::weiseSongtextZu`: die gewählte `.lrc`/`.txt` wird unverändert in den
Benutzerordner kopiert (`<Interpret> - <Titel>`, Endung nach Zeitmarken),
ersetzt dort eine Datei gleichen Namens; die mit der anderen Endung weicht
nur, wenn LumiViz sie geholt hat. Die Suchreihenfolge bleibt — gewinnt eine
Quelle mit Vorrang (eingebettet synchron, `.lrc` neben dem Song, eigene
`.lrc` im Benutzerordner), meldet der Knopf das. Vorrang für zugewiesene
Dateien: **Entscheid Patrik a (S88) — nein, Reihenfolge bleibt.** Die
Kopie trägt kein `[length:]`, gilt also für alle Fassungen des Namens. Der
Knopf selbst (Dateidialog, Meldungen) hat keinen Test. **Sichttest offen
(Patrik).**
**✅ S88 gebaut: Versatz je Titel** (Entscheid Patrik a: eigene Ablage, keine
Songtext-Datei wird umgeschrieben) — `offsets.json` im Benutzerordner
(`audio::songtextTitelVersatzMs`/`setzeSongtextTitelVersatz`, Schlüssel
`songtextSchluessel` = Interpret, Titel, Album, Dauer in Sekunden), am
Renderer `MultiEffectVisualizer::setSongtextTitelVersatz` (gesetzt beim
Songwechsel; Lyrics und Lyrics Trail rechnen ihn zu `offsetMs` und
`[offset:]` dazu). Felder: **Offset for this track** im Lyrics-Editor,
**Track offset** in Lyrics Sync (Sprünge, Klopfen und Wellenform-Striche
rechnen ihn ein). Das Feld im Lyrics-Editor hat keinen eigenen Test.
**Sichttest offen (Patrik).** Nicht einbezogen: der Balken im
Alternativen-Dialog.
**✅ S88 gebaut: Tag View zeigt den Songtext** (Zeile *Lyrics* mit Quelle,
synced/not synced, Zeilenzahl; Textfeld nur lesend; zieht alle 500 ms nach).
**Sichttest offen (Patrik).**
**✅ S88 gebaut: Merker „online nichts gefunden"** (`notfound.json` im
Benutzerordner, `songtextFehlanzeige`/`merkeSongtextFehlanzeige`): 404,
Eintrag ohne Text und „keine Fassung mit passender Dauer" werden 7 Tage nicht
wieder angefragt, instrumental 30 Tage; Schlüssel = Interpret, Titel, Album,
Dauer in Sekunden. Kein Merker bei 429/503 und Netzfehlern. Zurücksetzen:
Settings → Audio → *Lyrics not found* → **Ask again**; eine Freigabe in der
Sperrliste vergisst die Merker ebenfalls. Fristen 7/30 Tage: Entscheid
Patrik a (S88). **Sichttest offen (Patrik):**
zweites Abspielen eines Titels ohne Text → Log „not asked again", Knopf.
Die Dokumentation sagt nichts zur Lizenz der Texte.
**Datei-Icons (S86, Wunsch Patrik):** `.lrc`-Icon erzeugt; alle Icons liegen
als `.ico` in `asset/img/logo/icons/` (Zuordnung Endung → Icon und Bauskript
dort im README). **Notiz für einen späteren Installer:** Dateitypen und
Icons wählbar registrieren — eigene Formate mit „Öffnen mit", fremde nur auf
Wunsch, Audioformate nicht umbiegen; Einzelheiten und Registry-Pfade im
selben README. Vorher prüfen, ob `LumiViz.exe <datei>` die Datei öffnet.
**Idee Patrik S86 — unsynchron zu synchron machen:** von Hand mitklopfen
(Taste je Zeile, während der Titel läuft), per Skript grob verteilen, per KI
ausrichten — und danach von Hand nachjustieren. **Die Schnittstelle dafür
ist gelegt:** jedes solche Werkzeug füllt nur `Zeile::zeitMs` (bzw.
`Wort::zeitMs`) im Modell und ruft `speichereSongtext()`; der Knoten merkt
keinen Unterschied, und die Suche findet die `.lrc` beim nächsten Mal von
selbst. Nötig wäre ein kleiner Editor (Zeilenliste mit Zeiten, Mitklopfen,
Zeile ± verschieben, Vorhören).
(3) **Neue Idee S86: Text als Milkdrop-
Element** — neben Wave, Shape und Sprite ein Text-Element im Milkdrop-Node,
das in dessen Pipeline (vor Warp/Composite) gezeichnet wird und so mit dem
Feedback verschmiert; Quelle und Schrift wie beim Text-Knoten.

**Quellen des Textes:**
- **Listen** — eine Zeile je Eintrag (im Feld oder als Datei), Wechsel wie
  beim Host Switcher: nach Zeit, auf den Beat (jeder n-te), beim Titelwechsel,
  per Skript (`textnext`, `textindex`); der Reihe nach, zufällig, ohne
  Wiederholung.
- **Tags** des laufenden Titels (ID3 „mp3tags", Vorbis-Kommentare, MP4):
  Titel, Interpret, Album, Jahr, Genre, Tracknummer, BPM, Kommentar — als
  **Vorlage** `{artist} – {title} ({year})`, damit fehlende Tags nicht
  „– ()" hinterlassen (leere Teile samt Trennzeichen weglassen).
- **Songtexte**: eingebettete Lyrics (ID3 `USLT`) und **zeitsynchron**
  (`SYLT` oder `.lrc` neben der Datei) — Karaoke-Zeile, die mitläuft.
- **Laufzeit-Werte**: Zeit gespielt/übrig, Preset-/Gruppenname, Uhrzeit,
  BPM der Beat-Erkennung, fps.

**Darstellung:**
- Schrift aus allen installierten Schriften (`QFontDatabase`) **und** aus
  mitgelieferten Dateien (`.ttf/.otf` in `asset/fonts/`, damit ein Preset auf
  jedem Rechner gleich aussieht); Größe relativ zur Bildhöhe.
- Farbe oder Farbverlauf, Kontur, Schatten, Leuchten; Ausrichtung/Anker.
- **Animationen**: Ein-/Ausblenden, Schreibmaschine, Laufschrift (Ticker),
  Wort für Wort, Buchstaben tanzen auf dem Beat, Zoom beim Titelwechsel,
  Größe/Helligkeit mit Bass oder Pegel.
- Technik: Text ändert sich selten → per `QPainter` in eine Textur, nur bei
  Änderung neu; für Effekte je Buchstabe ein Glyphen-Atlas (SDF), damit
  Skalieren/Leuchten im Shader scharf bleibt.

**Zusammen mit dem Cover-Modul** ergibt das eine „Now playing"-Einblendung
(Cover + Titel/Interpret, fährt beim Titelwechsel herein, blendet nach n
Sekunden aus) — als mitgeliefertes Preset statt fest eingebaut.

### ⚪ Idee: gemeinsame Farbquelle — Tafel, Verlauf, Zeitbasis (Wunsch S84)

Anlass SuperScope: die Grundfarbe kommt aus zwei Quellen, gemischt über
`colorBlend` — einer AVS-Farbtafel (`colors`), die alle `colorCycleFrames`
**Frames** einen Schritt weiterspringt, und einem Verlauf (`gradientPreset`),
der über die **Punkte** läuft. Patrik: der Verlauf könnte auch **über die Zeit**
laufen (gleichbleibendes dt, nur die Periode ist festzulegen), und die Tafel
wechselt in Frame-Einheiten, die man nicht als Zeit einstellen kann. Und das
gilt nicht nur für SuperScope: Bass Spinner und andere Module mit einer Farbe
oder Tafel könnten dieselbe Farbquelle haben, haben sie aber nicht.

Vorschlag (Claude S84):
- Ein **Farbquellen-Baustein** für alle Module mit Farbe: Modus *fest · Tafel ·
  Verlauf · Tafel+Verlauf gemischt*; **Lauf über** *Punkte · Zeit · Beats*;
  **Periode** in Sekunden oder Beats (statt Frames — dann unabhängig von der
  Bildrate); Richtung/Pingpong; Phase per Skript/Modulation (Block F).
- Alte Felder bleiben lesbar: `colorCycleFrames` wird beim Laden in eine
  Periode umgerechnet (bei 60 fps) oder als „bisher"-Einstellung geführt
  (Treue-Muster S81).
- Gehört zur **Vereinheitlichung V5** (Gradient-Parameter-Typ + LUT-Baustein)
  und nutzt die Feldtabelle aus Block F (Farben `_r/_g/_b`, Tafeln später als
  `colors_<i>_r` …).
- **Betroffen: fast alle Zeichen-Module** (Patrik S84). Bestandsaufnahme S84 über
  die 36 Palette-Einträge mit Rolle „Render" (Felder aus dem Inventar):

  | Farbquelle heute | Module |
  |---|---|
  | **Tafel + Verlauf** (1) | SuperScope |
  | **nur Tafel** (9, AVS-Takt je Frame) | Dot Fountain, Dot Grid, Dot Plane, Metaballs 3D, Oscilloscope Star, Ring, Rotating Stars, Simple (Scope), Tentacles 3D |
  | **nur Verlauf** (10, mit `colorScale/colorCycle`) | Domain Warp, Flame, Fractal 2D, Fractal 3D, Fractal Zoomer, Kleinian, Lyapunov, Reaction Diffusion, Strange Attractor (+ `useGradient`) — plus feste `insideColor`/`negColor` |
  | **nur feste Farbe(n)** (8) | Bass Spin (links/rechts), Clear, Debug Bars, Moving Particle, OnBeat Clear, Starfield, Terrain 3D (3 Farben), Timescope |
  | **keine Farbfelder** (8) | FyrewurX (`hueDrift`), Glow Orbs, SuperScope 3D, Triangle (Farbe nur per Skript) · Picture, Picture II, Texer, Texer II, Video (Bildquellen — Farbquelle nicht sinnvoll, höchstens Tönung) |

  Kandidaten für die Farbquelle also rund **28 Module** (alle außer den
  Bildquellen); Clear/OnBeat Clear eher eine feste Farbe mit optionalem
  Zeitverlauf.
- Zu klären: ob die AVS-Importe ihre Originalfarbe behalten (Vorgabe =
  heutiges Verhalten, neue Möglichkeiten nur als Zusatz).

### ⚪ Idee: externer Audio-Eingang — Loopback, Line-in, Mikrofon (Wunsch S83)

Heute analysiert LumiViz nur, was der **eigene Player** abspielt. Für Live-
Einsatz (DJ-Pult, Band, Musik aus einem anderen Programm) braucht es einen
Eingang von außen. **Stand S83:** angelegt, nicht gebaut —
`IAudioEngine::createLoopbackStream()` ist deklariert, `BassEngine` findet das
Loopback-Gerät, bricht dann aber mit „Loopback not implemented" ab
(`BassEngine.cpp`, braucht **BASSWASAPI**). Für Line-in/Mikrofon gibt es noch
keinen Pfad (BASS: `BASS_RecordInit`/`BASS_RecordStart`).
- **Loopback** (WASAPI): „was gerade aus den Lautsprechern kommt" — jedes
  Programm als Quelle, ohne Kabel.
- **Aufnahmegerät** (Line-in, Mikrofon, Audio-Interface).
- Zu klären: Umschalten Player ↔ extern (Einstellung? automatisch?), Latenz
  und Puffergröße, Wirkung auf **Beat-Erkennung** und die kalibrierten
  Audio-Werte (MilkdropAudio/FFT sind auf den Player-Pfad abgeglichen, S80),
  Song-Slot und Host Switcher ohne Songwechsel-Signal (Stille-Erkennung als
  Ersatz?). Lizenz: BASSWASAPI gehört zur un4seen-Familie wie BASS selbst.

### ✅ Drop auf die freie Fläche unter der Kette (Befund S80, behoben S80)

Nachgestellt im neuen UI-Test am echten Panel (`test_PaletteUi`): gewöhnliche
Module landeten auf der Leerfläche schon immer am Ende. Verloren ging eine
**Custom Shape/Wave/Sprite** — sie braucht einen Milkdrop-Knoten als Ziel, und
die Leerfläche hat keins. „Direkt unter der letzten Zeile" klappte nur, weil
die letzte Zeile die Sprites-Sektion des Milkdrop-Knotens war. Fix: auf der
Leerfläche geht das Element in den **letzten Milkdrop-Knoten in
Baumreihenfolge** (`letzterMilkdropPfad`, `MultiEffectPanel.cpp`). Test
`PaletteUi Panel A14b`.

### ✅ Vorlagen für Milkdrop-Elemente und Shader (Wunsch S80, umgesetzt S83)

**Umgesetzt S83** (Entscheide Patrik E3a/E4a): Custom Wave/Shape/Sprite tragen
eine eigene Preset-Zeile (Typ-Schlüssel `milkWave`/`milkShape`/`milkSprite`,
JSON-Ebene im NodePresetStore); Werks-Presets *Oscilloscope Line*,
*Spectrum Circle*, *Color Cycle* (Waves) und *Pulse With Bass*, *Slow Spin*,
*Slow Fade* (Shapes); dazu die Sektionen *Code* (`milkCode`) und *Parameter*
(`milkParams`, u. a. *Original* und *Old LumiViz Look (pre-S80)* = Gestaltung +
„bisher"-Schalter in einem Zug). Die fest verdrahteten Vorlagen von
SuperScope 3D (7) und 3D Camera (6) sind nach `asset/nodepresets/` gewandert.
Shader: Vorlagen **nur im Groß-Editor** (Zeile „Template:"), „Empty" aus dem Code,
21 Dateien unter `asset/shadertemplates/`, jede im echten Knoten kompiliert
(Test). Offen: Sprite-Werks-Presets (brauchen ein mitgeliefertes Bild).


Wer eine Shape, Wave, Sprite oder einen Shader neu anlegt, bekommt heute nur
neutrale Werte — ein schneller „Einstiegspunkt“ fehlt. Wunsch Patrik: Vorlagen
wie „Shape pulsiert mit Bass“, „langsames Ausblenden“, „Drift nach oben
links“ — also genau die Effekte, die jetzt **unerklärt** auftreten (§3, oben),
aber **gewollt und benannt**. Anknüpfung: die Werks-Voreinstellungen der
Module (Versionen 1.45.0/1.46.0 dieser Liste — dort sind milkdrop/shadertoy
bewusst ausgenommen).
Die Voraussetzung (§3-Drift klären) ist seit S80 erfüllt — Drift und
Ausblenden gibt es jetzt nur noch, wenn ein Preset sie verlangt.

### ⬜ Visualizer-Tab-X löscht statt ausblenden (Befund S77)

Das X am Visualizer-Tab schließt mit `DockWidgetDeleteOnClose` — der
Visualizer ist samt Eintrag in View > Panels weg, zurück nur über „New
Visualizer" oder Neustart. Für den **ersten** Visualizer wäre Ausblenden
(ohne Delete) konsistenter mit den Panels. Entscheid offen; Details
`include/UI/managers/DockManager.md` §5.6.

### ➜ Formelfelder für alle Module (Entscheid 2a, S79)

Inventar aller 84 Palettenmodule + Vorschläge:
[visuals/Formelfelder_Inventar.md](visuals/Formelfelder_Inventar.md). **Block A
✅ (S79):** acht Module versprachen Skriptvariablen, die nie gebunden waren —
gebunden; Variablenreferenz im Panel aus der neuen SSOT
`ScriptVariables.hpp` (vorher „No dedicated variable reference" bei rund
vierzig Modulen), Test `test_ScriptVariablesGl` hält sie an der Laufzeit.
**Block B ✅ (S79):** 16 Module mit Farbfeldern haben `<feldname>_r/_g/_b`
(0..1, Klemmung erst beim Zeichnen) — Sichttest offen; Nachgang Farbtafeln der
Scopes. **Block C ✅ (S79):** alle sieben Fraktale mit `colorscale`,
`colorcycle`, `colorphase` (laufende Phase, je Frame les-/schreibbar) und
Innen-/Hintergrund-/Negativfarbe als `_r/_g/_b` — Sichttest offen.
**Block D ✅ (S79):** 43 weitere Felder in 14 Modulen (Liste im Inventar) —
Sichttest offen. **Block E ✅ (S79):** Set Render Mode, ISF Filter (jeder Regler
unter seinem key), eigene Uniforms in Shadertoy/Pixel Filter/Mesh
Warp/GPU-Partikel (Entscheid 1a), Host Group mit `outadjustalpha` (Entscheid
2a); dabei behoben: Set Render Mode speicherte `adjustAlpha` nicht. Sichttest
offen. **Offen:**
F generischer Weg (automatische Bindung + Mod-Quelle je Feld — eigene
Konzept-Session). Jede neue Variable: Bindung in `run*` **und** Zeile in
`ScriptVariables.cpp`, der Test meldet einseitige Änderungen.

### ⬜ Feldbreiten-Vertrag nur in der Effektkette (S78)

Seit S78 gilt in der Effektkette: Felder ≥ 120 px, 5 px Panelrand,
seitliches Scrollen statt Überbreite, Dock-Minimum 240 px + Ränder
(`include/UI/panels/MultiEffectPanel.md` §Feldbreiten). Die übrigen Panels
(ConfigPanel, Settings, Playlist …) folgen dem noch nicht — sie hängen weiter
in der automatischen Qt-ADS-Scrollarea, ConfigPanel mit eigenen festen
Mindestbreiten. Übertragen nur auf Wunsch. Offen außerdem: die Log-Zeilen
`Effect Chain: Feldzeile '…' braucht N px` einmal durch alle Module sichten
(bekannt breit: 7×7-Kernel, bewusst).

### ✅ Shadertoy-Modul (Entscheid Patrik S64, Strang S — gebaut S65)

*(Bereinigt S129: Strang S S1–S4 ist in S65 komplett gebaut, s. „Erledigt in
S65“ oben. Offen bleiben die Sichttests und die Netz-Abnahme mit API-Key. Der
Text unten ist der Plan vor dem Bau.)*

Eigener Node-Typ mit **modernem Regelwerk**, inkl. **URL-/ID-Import**
(Shadertoy-API auf Knopfdruck, API-Key in den Settings; Fallback
Code-Einfügen) und eingebauter/nachrüstbarer **Audioreaktivität**
(Shadertoy-natives 512×2-Audio-iChannel FFT+Waveform + LumiViz-Uniforms
bass/mid/treb/beat + Audio-Mod-Skript-Slot). Ausbaustufe Multipass A–D.
⚠ Lizenz CC BY-NC-SA: Inhalte bleiben lokal, nichts ins Repo.
**Genauer Plan (S1–S4): [Regelwerk_und_Neue_Module_Plan.md](visuals/Regelwerk_und_Neue_Module_Plan.md)**
— Start nach Strang R.

### ⬜ Editor-Komfort im Multi-Chain-Panel: Apply + Beautify (Wunsch Patrik, 2026-08-05 — **umgesetzt S69, Sichttest offen**)

Umgesetzt (S69), alle drei Punkte — offen ist nur noch der UI-Sichttest:
1. **Apply-Button** (`QDialogButtonBox::Apply`) im Groß-Editor
   (`EelScriptEditing.hpp` 1.1.0, `ScriptEditorHooks`): übernimmt +
   recompiliert (Text → Inline-Feld → `mutate` → `recompileChain`), Dialog
   bleibt offen. Fehler IM Dialog: EEL synchron per `eel::transpile`-Probe
   (Dialekt des Knotens: Milkdrop vs. AVS), HLSL synchron per
   `hlsl::transpile`, Shadertoy-GLSL per Nach-Polling (300/900/1800 ms) auf
   `shadertoyError(nodeId)` — GL kompiliert erst im Render-Thread, d. h. der
   Fehler kommt nur an, solange der Viewport rendert.
2. **Beautify-Button**: Kerne in `include/scripting/ScriptFormatter.hpp`
   (pur, ohne Qt — 26 doctest-Cases inkl. Whitespace-only-Vertrag: Token
   byte-identisch). GLSL/HLSL = brace-basiertes Re-Indent; EEL = ein
   Statement pro Zeile + Einzug nach Klammertiefe. Milkdrop-Spezialfall
   (geklärt 2026-08-05): `per_frame_N=`-Rückverteilung betrifft NUR einen
   etwaigen `.milk`-Export — als Chain-Node/`.lvfx` leben die Skripte als
   ganze Strings (`assembleCode`).
3. **Settings**: neuer Tab „Editor" im Settings-Panel — Einzugsbreite,
   Leerzeichen um Operatoren (nur EEL), max. Leerzeilen (QSettings
   `editor/...`, Defaults = `FormatOptions`, je Beautify-Klick frisch
   gelesen).
4. **Import…/Export… für Shader-Felder** (Nachwunsch Patrik, gleiche
   Session): Groß-Editor der Shader-Felder (Shadertoy Image+Buffer A–D,
   Milkdrop Warp/Comp) lädt/speichert Dateien (UTF-8, Filter
   `*.glsl *.hlsl *.frag *.txt`); Export-Vorschlag `preset_name.modul.glsl`
   (`shaderExportName()` — Basis: Shader-Metadaten- bzw. Node-Name,
   sanitisiert; Modul: image/bufferA–D/warp/comp — Endung laut Vorgabe
   auch für die HLSL-Felder `.glsl`). Letzter Ordner in QSettings
   `editor/shaderFileDir`. Import ersetzt nur den Editor-Inhalt —
   übernommen wird erst mit Apply/OK.

### 🟠 Key-Variablen-Regel für JEDES Import-Format (S72)

> **SSOT der Regel:** `include/scripting/ScriptBaseKeys.hpp` (Entscheid D2,
> `Vereinheitlichung_Konzept.md` §4). Dort steht die AVS/MilkDrop-Seite
> vollständig — alles Weitere hier ist **neu aus S72**.
>
> **Geltungsbereich (Vorgabe Patrik S72):** Die Regel gilt für **alle**
> Import-Formate — AVS, MilkDrop, **Shadertoy**, **ISF** — und für jedes
> künftige. Sie ist bewusst formatunabhängig formuliert: ein neues Format
> bringt nur seine Builtin-Liste mit, die Regel selbst bleibt.

**Die Regel, wie Patrik sie in S72 formuliert hat** (Beispiel `bass`):
`bass` ist ein **MilkDrop**-Original. Ein natives Lumi-Skript darf es für das
Bass-Signal benutzen. Ein importiertes **AVS**-Preset kennt es dort aber
*nicht* als Key-Variable — also ist `bass` in dieser Datei eine gewöhnliche
Nutzer-Variable und wird zu **`bass_p`** umbenannt. Gleiches gilt für alle
anderen. Verschärfung Patrik: *„wenn es mehrere `bass` als Original gibt und
die Definition der Auswertung gleich ist, passt es ja auch"* — dann ist es
kein Konflikt, sondern ein **gemeinsamer Schlüssel**, und das Binden an den
injizierten Wert IST die treue Semantik.

Daraus drei Fälle, maschinell entscheidbar:

| Lage im Quellformat | Folge |
|---|---|
| vorhanden, **gleiche** Auswertung | gemeinsamer Schlüssel — **nicht** umbenennen |
| vorhanden, **andere** Auswertung | umbenennen |
| **nicht** vorhanden | gewöhnliche Nutzer-Variable — umbenennen |

**Die Builtin-Listen je Format** (das Einzige, was ein neues Format mitbringen
muss):

| Format | Builtins, die NIE umbenannt werden |
|---|---|
| AVS | `beat` (r_list); Rest siehe `kInjectedKeys` |
| MilkDrop | `bass`, `mid`, `treb`, `treble`, `vol`, `time` |
| Shadertoy | `iTime`, `iTimeDelta`, `iResolution`, `iFrame`, `iFrameRate`, `iMouse`, `iDate`, `iSampleRate`, `iChannel0..3`, `iChannelTime`, `iChannelResolution`, `mainImage` |
| ISF | `TIME`, `TIMEDELTA`, `FRAMEINDEX`, `RENDERSIZE`, `PASSINDEX`, `DATE`, `isf_FragNormCoord`, `vv_FragNormCoord` |
| LumiViz (Zusatz in allen) | `bass`, `mid`, `treb`, `vol`, `beat`, `dt`, `getosc`, `getspec`, `getspecdb`, `_lumi*` |

**Offen (vier Befunde S72):**

0. **🟠 Shadertoy hat dieselbe Lücke — seit S65, unbemerkt.** Die
   Shadertoy-Prälude deklariert `uniform float bass;` (und mid/treb/vol/beat).
   Ein Shader von shadertoy.com, der eine **eigene** Variable `bass` benutzt,
   ergibt damit eine **Doppel-Deklaration** und kompiliert nicht — mit einem
   Fehler, der auf den Nutzer-Code zeigt, obwohl die Ursache in unserer
   Prälude sitzt. Dieselbe Regel, dieselbe Behebung wie unten. Häufigkeit im
   Netz-Korpus noch nicht gemessen (Shadertoy-Inhalte liegen lizenzbedingt
   nicht im Repo — Messung über den Browser-Import, sobald jemand darauf
   stößt).
1. **ISF fehlt in der Tabelle.** `IsfFilterWrapper::istPraeludenName()` trägt
   eine **eigene** Liste reservierter Namen — eine zweite Quelle neben
   `kInjectedKeys`. Richtig wäre: ISF-Builtins (`TIME`, `TIMEDELTA`,
   `FRAMEINDEX`, `RENDERSIZE`, `PASSINDEX`, `DATE`, `isf_FragNormCoord`,
   `vv_FragNormCoord`) in die SSOT, und der Wrapper fragt
   `collidesOnImport(name, format)`.
2. **Namensschema inkonsistent.** Entscheid D2 lautet `name` → `name_p`
   (mit Eskalation bei erneuter Kollision). Für ISF wurde stattdessen
   `_lumiIn_<name>` erfunden — dasselbe Konzept in zwei Schreibweisen. Auf
   `_p` vereinheitlichen; die Eskalationsregel mit übernehmen, denn ein
   `INPUT` *kann* `bass_p` heißen.
3. **`KeyOrigin` ist zu eng.** Ein Wert je Name kann „mehrere Formate, gleiche
   Auswertung" nicht ausdrücken. Ersetzen durch eine Format-**Menge** plus
   Kurzbeschreibung der Auswertung.

⚠ **Groß-/Kleinschreibung:** die AVS-Regel vergleicht case-INSENSITIV (EEL),
GLSL ist case-SENSITIV — `time` und `TIME` sind für ISF **zwei verschiedene
Namen**. Die gemeinsame Regel braucht das format-abhängig.

**Warum es zählt:** ein `INPUT bass` ist in ISF ein **Schieberegler, den der
Nutzer zieht**. Ihn an unsere Audio-Energie zu binden, machte daraus still
einen audiogesteuerten Wert — das wäre keine Treue, sondern eine
Verhaltensänderung. Im Vidvox-Korpus kommt der Fall (Stand S72) kein einziges
Mal vor; die Regel greift bei der ersten Datei, die es tut.

### ✅ ISF-Import + Parameter-Baum — UMGESETZT in S72 (als EIGENER Knoten)

> Der Plan darunter ist **überholt**: er wollte ISF auf den `pixelFilter`
> abbilden. Entscheid Patrik S72: **eigener Knotentyp `isfFilter`** mit
> beiden Shader-Stufen — damit entfielen alle Kompromisse des Plans.
> Stand: 321 von 327 Vidvox-Dateien kompilieren und linken (inkl. Multipass).
> Offen davon nur noch der **Sichttest**. Details: `Changelog_Session72.md`.

### ➜ (überholt) ISF-Import + generischer Parameter-Baum (Plan S71)

> **Steuerdokument:** [`visuals/ISF_Import_Parameterbaum_Plan.md`](visuals/ISF_Import_Parameterbaum_Plan.md)
> (1.0.0 — vier Stufen, Pflichtkette, Lizenz-Querschnitt, Risiken; Freigabe offen)

**Ziel:** ISF-Filter (`.fs`, s. §7-Block „Filter-Fundgruben") laden und mit
ihren eigenen Parametern bedienbar machen. Der Vorbau steht seit S71
(Vertrags-SSOT, Namensschema, Ordner je Vertrag, Vertragsprüfung beim Import —
Changelog 1.65.0). Drei Bausteine, in dieser Reihenfolge:

1. **ISF-Parser** — pur, GL-/Qt-frei, unit-testbar: JSON-Kopf aus dem
   führenden Blockkommentar extrahieren, `INPUTS` typisiert einlesen, den
   GLSL-Körper auf den `farbe()`-Vertrag übersetzen (`IMG_PIXEL`/
   `IMG_NORM_PIXEL`/`IMG_SIZE` → `uTex`/`uv`, `RENDERSIZE`/`TIME`/
   `TIMEDELTA`/`FRAMEINDEX`/`isf_FragNormCoord`, `inputImage` → `src`).
   Muster: `PixelFilterWrapper.hpp` (GL-frei + testerzwungen).
2. **Generische Parameter-Ablage im Node** — getippte Werte, die der
   Serializer mitschreibt. Zieht die Pflichtkette nach sich (Leser-Klemmen,
   FieldDocs/Inventar-Gate, Wächter-Test).
3. **Parameter-Baum im Panel (Entwurf Patrik):** aufgebaut **wie die
   Effect-Chain** — Baum-Ansicht, aber mit **Wert-Spalte rechts** und
   **typsicheren Editoren** je Zeile (ISF deklariert den Typ: `bool` →
   Checkbox, `float` → SpinBox mit MIN/MAX aus dem JSON, `long` mit
   `LABELS`/`VALUES` → echtes Dropdown mit Klartext, `color` → Farbwähler,
   `point2D` → zwei Felder). Bewusst **nested-fähig** angelegt, auch wenn
   ISF-`INPUTS` flach sind — `PASSES`/`IMPORTED` sind es nicht, und die
   Struktur trägt dann mehr als nur ISF.

**⚪ NEU (Idee Patrik S71): dasselbe Prinzip auch für die ANDEREN Module.**
Ein generischer, typsicherer Parameter-Baum wäre kein ISF-Sonderweg, sondern
die gemeinsame Darstellung für alle Knoten — und genau die Infrastruktur, die
der Backlog-Punkt „dynamische Modulparameter" (alle Params per
init/frame/beat/point) ohnehin braucht. Beim Bau von Stufe 2/3 deshalb nicht
ISF-spezifisch schneiden. Prüfen, wie sich das zur bestehenden
Panel-Erzeugung (`addInt`/`addDouble`/`addCombo`/`addCodeEditor`) verhält:
Ablösung oder zweiter Weg?

**Lizenz-Pflicht gilt auch hier** (s. „LIZENZ-PFLICHT bei JEDEM Fremd-Import“
im §7-Block unten): Herkunft, Autor und Lizenz (`CREDIT` im ISF-JSON) gehören
ins Preset, nicht nur in die Doku. ✅ Umgesetzt (Changelog 1.67.0;
`EffectChain.hpp` `Herkunft`, `IsfImport.hpp` CREDIT → Autor; bereinigt S129).

### ⚪ Szenen-Wechsler-Modul (Idee Patrik, 2026-08-06 — Konzept in eigener Session)

> **Stufe Host-Gruppen umgesetzt (S82): Host Switcher** — Liste von Gruppen
> (Name = ID, eindeutig mit Zaehler, Umbenennen zieht mit), Startgruppe,
> Wechsel nach Zeit / Songwechsel (Liste oder zufaellig) / Skript (Init,
> Frame, Beat, Song; `switch`, `nextgroup`), schaltet nur zur Laufzeit
> (`effectiveEnabled`, Entscheide Patrik S82: 1a nur Gruppen, 2a Laufzeit,
> 3a EEL-Slots). Offen bleibt der Rest dieses Eintrags: Szenen = Effect
> Lists in beliebiger Tiefe (Laufzeit-Gewicht je Liste), Energie-Klassen,
> Strophe/Refrain. Songwechsel-Signal: `MultiEffectVisualizer::
> notifySongChanged()` (prozessweiter Zaehler) — Grundlage fuer den
> Song-Slot aller Module (§6, Vorschlag S82).

Szenen = **EffectListen in beliebiger Verschachtelungstiefe** mit stabiler ID
(neues Feld oder Name) — WICHTIG (Einwand Patrik): Host-Gruppen wohnen nur im
Top-Level, das Konzept darf nicht an ihnen hängen; sie sind der Spezialfall
„Top-Level-Szene mit Gratis-Crossfade" (HG2). Ein neues Wechsler-Modul
schaltet Szenen an/aus — **Fading auch nested**: Laufzeit-Gewicht je Liste
(blendWeight-Analogon), das der Wechsler über eine Rampe fährt und das mit
dem Out-Alpha der Liste multipliziert wird (persistiertes `enabled` bleibt
unangetastet — Dokument ≠ Laufzeit). **Re-Init nested**: ListRuntime hat
`needsClear` schon — „Szene frisch" = needsClear + Init-Slots erneut, pro
Liste statt pro Gruppe. Trigger-Stufen:
1. **Sicher machbar:** Song-Wechsel (Player/Playlist-Events), echte
   Track-Position auch nach Spulen (`trackTime`-Input), Beat-/Takt-Zähler,
   Live-Energie-Klassen (ruhig/laut/Build-up/Drop via Langzeit-Statistik +
   Novelty).
2. **Strophe/Bridge/Refrain:** live nur „mehr oder weniger" (Wechsel-
   Erkennung ja, Label nein); GUT per **Offline-Pre-Analyse beim Laden**
   (BASS-Decode → Self-Similarity → Segmentgrenzen + Wiederholungs-Cluster:
   wiederholt = Refrain-Kandidat). Verwandt: AudioLookahead-Service
   (Lights-Backlog §7). Prüfstand: das schwach portierte Lights-WebGL-Demo
   (`asset/effectchain/lights_demo.*`).
Bezug: P2 Visual-Playlist (§6) und Hotkeys Stufe 3 (Composer) überschneiden
sich — im Konzept abgrenzen.

### 🟡 Lyrics-Block: Zeilen-Puffer, Wort-Ebene, Auslöser, Lyrics Sync (S90/S91 gebaut — Sichttest begonnen S92)

**Steuerdokument:** `visuals/Lyrics_Block_Konzept.md` (Entscheide L1–L6,
B1–B3, Stand je Stufe mit „geprüft / nicht geprüft").

**✅ Gebaut in S90:** LB1 Zeilen-Puffer (der Trail legt das Bild auf, das
stand) · LB1b Vorschau-Zeilen (Größe, Gewicht, Lage, Zeilenabstand) · LB2
laufendes Wort (*Word scale*, *Word color*, *Word layout*, *Word flow*) · LB3
`word` / `wordcount` und Skriptfeld Word · B2 a Wort-Masken auf der
Grafikkarte (Lage, Größe und Farben folgen dem Skript aus jedem Feld; Gleiten
0,4 statt 6 ms je Frame) · B3 a1 eine Schriftgröße je Song · LB4
`linetrigger` / `wordtrigger` · LB5-1 Stimmen im Panel Lyrics Sync · LB5-2
Wort-Zeiten klopfen. Dazu drei Fixes: Doppel-Trail nach Pause-Zeilen, Sprung
der Schrift beim Zeilenwechsel (B1), AVI-Knoten unsichtbar ohne Text-Knoten
davor (Regression aus S89; seither steht ein AVI-Preset in der lokalen
Regressionsliste).

**✅ Gebaut in S91:** LB5-3 langsamer abspielen — Schieber **Speed**
(50–100 %) im Panel Lyrics Sync über die Abtastrate (`BASS_ATTRIB_FREQ`, der
Ton wird tiefer); die Abspielposition zwischen zwei Meldungen des Players
zählt mit dem Tempo; Entscheid **T1 a**: das Tempo gilt nur, solange das
Panel sichtbar ist. Dazu die kleinen Reste aus S89/S90: Statuszeile des
Panels nicht mehr abgeschnitten (Mindesthöhen, das Panel scrollt), der Trail
der Karaoke-Ebene hält über den Preset-Wechsel (er sprang zurück), **Load…**
im Effect-Chain-Panel bietet `.avs` / `.milk` mit an, **Favoriten im Effect
Browser** (Entscheid **F1 a**: ★, Rechtsklick, Ansicht ★ Favorites mit
Blättern), Skriptfeld **Line am Lyrics Trail** (Entscheid **T2 a**: einmal je
gegangener Zeile, `line` = die Zeile, die eben ging), Handbuch-Beispiel
`wordtrigger = if(below(word, wordcount), b, 0);` per Test belegt.

**✅ Sichttest S92, erste Runde (Patrik):** abgenommen A1 (Trail zeigt das
Bild, das stand), D1 (Word scale, Reflow), J1 und J3 (Schieber Speed, Sprung
beim Reiterwechsel). Vier Befunde, alle in S92 behoben: **H1** Enter / T
klopften nur mit Fokus in der Zeilenliste → gelten jetzt im ganzen Panel;
**E1** der Puls im Handbuch klang zu schnell ab → 0.93 statt 0.85; **G1**
„eine Zeile je Beat" unrealistisch → Beispiel für jeden vierten Beat; **D2**
in Overlay zu wenig Luft um das große Wort → Feld **Word spacing (%)**.

**✅ Gebaut in S92** (alles Wünsche und Entscheide Patriks aus dem Sichttest):
Panel Lyrics Sync mit **Pre-roll** (Vorlauf des Doppelklicks, 0–10 s),
**Sprungleiste** über den ganzen Titel und **Kontextmenü** je Zeile / Wort ·
Feld **Word spacing (%)** am Lyrics-Knoten (steckt in der Schrift:
`QFont::setWordSpacing`; folgt dem Skript aus Init, Song, Beat, Line, Word) ·
**Trail-Presets**: die Trail-Liste der Karaoke-Ebene als eigenes Preset
(`.lvtrail`, Kettenformat; `TrailPresetStore`; mitgeliefert
`asset/karaoke/trails/`, acht Stück; eigene im Benutzerordner), Zeile **Trail
preset** im Karaoke-Panel, im Kettenbaum **Add trail preset ▸** / **Save as
trail preset…** per Rechtsklick, eigenes Icon (`lumiviz_trail.ico`) für jede
Effect List mit Lyrics Trail · **Prüfer für die Skriptbeispiele des
Benutzerhandbuchs** (`test_HandbuchBeispiele.cpp`: 20 Beispiele, 14 belegt,
6 nur übersetzt) · die **vier Fragen aus S89** entschieden: Doppelklick auf
ein `.lvkar` schaltet die Ebene ein (1a), `current.lvkar` steht nicht mehr im
Effect Browser (2a), die Rückfrage vor der Dateiauswahl bleibt (3a), Ctrl+K
ruht in Textfeldern (4a).

**Offen:**
0. **Sichttests S93 (Patrik):** `.claude/handover/Sichttests_S93.md` (A–I:
   Kontextmenüs im Kettenbaum, Sprite mit Bilddatei, Start und Effect
   Browser, Knopf ◎, neue leere Kette, Audio Browser) — im laufenden
   Programm ist davon nichts gesehen; **zuerst G** (der Startablauf ist
   nicht per Test abgesichert). Zum Abhaken gibt es seit S93 das Werkzeug
   `Sichttest` (`projects/exec/Sichttest`, README dort); begonnen damit:
   S88 A1, A2 ✓.
1. **Sichttests (Patrik):** `.claude/handover/Sichttests_S92.md` (A–F: alles
   aus S92 — im laufenden Programm ist davon nichts gesehen; zuerst A1, B1,
   B3, D1, D3, E1) und der Rest von `Sichttests_S90.md` (A2–A6, B, C, D3–D6,
   E2–E6, F, G2–G5, H2–H8, J2, J4, J5, I1); dazu aus S91 ohne eigenen
   Listenpunkt: Load… mit `.avs` bei ungespeicherter Kette (genau eine
   Rückfrage), Rechtsklick und Ansicht ★ Favorites im Effect Browser, Feld
   Line am Lyrics Trail (Kette und Karaoke-Ebene), Preset-Wechsel bei
   laufendem Trail der Ebene.
1a. **Nicht geprüft aus S92:** eine eingefügte Trail-Liste im Bild einer
   echten Kette (nur die Struktur ist getestet); ob ein Trail-Preset mit
   zeichnenden Modulen (über **Save as trail preset…** möglich) in der Ebene
   sinnvoll aussieht; Zoom Burst und Swirl drehen um die Bildmitte (Zeile
   unten → der Trail wandert zum Rand); Vorschau-Zeilen mit Word spacing
   nicht eigens gemessen; **Ctrl+S** im Skript-Editor (Menü-Kürzel, läuft
   nicht über die Kürzel-Tabelle — weder geprüft noch angefasst); sechs
   Handbuch-Beispiele nur übersetzt (vier Drift-Zeilen des Milkdrop-Nodes,
   `nextgroup` am Host Switcher, das Song-Muster).
2. **Nicht geprüft aus S91:** `AudioPlayer` mit echter Engine (keine
   Engine-Attrappe — das BASS-Verhalten ist nur durch eine Probe belegt);
   ein sichtbar zerstörtes Sync-Panel setzt das Tempo nicht zurück; Ebene mit
   Host-Gruppe oder Buffer Save in der Trail-Liste über den Preset-Wechsel
   (der Puffer-Pool wird weiter geleert); ob der Effect Browser im Programm
   nach jedem Laden seine Liste neu aufbaut (er hört jetzt auf
   `PresetListsChangedEvent`); die Wahl ★ Favorites über einen Neustart.
3. **Nicht geprüft** (aus dem Konzept): Masken-Weg in der Karaoke-Ebene mit
   Render Scale; Overlay mit getrennter Kontur im Bild; Modulation ohne
   Skript; Speicher bei sehr großen Fenstern; Trail beim Rücken per Auslöser.
4. **Ideen, nicht eingeplant:** Stimme je Wort (L6-3 b), Tempo über `bass_fx`
   ohne Tonhöhenänderung (L6-4 b), Einsätze in der Wellenform einrasten
   (L6-5 b), Worte aus dem Signal erkennen (braucht Stimmentrennung und
   Sprachmodell), geschätzte Wort-Zeiten als Schalter (L3 b), Zeilen und
   Worte im Voraus je Index gestalten (L4 b).
5. **Zurückgestellt:** Wort-Zeiten des Dienstes.

**Ursprüngliche Notiz (S89), zur Einordnung:**

Vier zusammenhängende Wünsche vom Ende der Session 89, nichts davon gebaut.
Ausgangslage: der Lyrics-Knoten kennt seit S89 den Slot **Line** und die
Variablen `line` / `linecount`; Wort-Zeiten (`<mm:ss.xx>` im LRC) liest er
seit S86 und füllt die laufende Zeile damit Wort für Wort (zwei Zustände:
gesungen / noch nicht).

1. **Die aktive Zeile puffern — sie ist es, die in den Trail wandert**
   (Hinweis Patrik). Heute zeichnet der Lyrics Trail die alte Zeile **neu**,
   aus Text und Feldern; S89 hebt dafür nur die Felder der alten Zeile auf
   (`lyricsEffektivAlt`, nur mit Line-Code). Sauberer: das **fertige Bild** der
   aktiven Zeile aufheben und beim Wechsel an den Trail übergeben — dann zeigt
   er genau, was stand (Wort-Stand, verkleinerte lange Zeile, Skriptwerte).
   Zu lösen: (a) die laufende Zeile getrennt von den Vorschau-Zeilen rastern;
   (b) *Own color* des Trails braucht Füllung und Kontur getrennt (gibt es
   schon für die Mischarten: `GlyphTeil`); (c) der Versatz des Trails wirkt
   dann beim Auflegen (UV-Verschiebung) statt beim Rastern. Baut den
   Trail-Renderer um → eigene Runde Bildtests und Vergleichslauf.
2. **Das laufende Wort hervorheben** (größer, eigene Farbe): dritter Zustand
   „gerade jetzt" neben gesungen / noch nicht; Felder etwa *Word scale (%)*
   und *Color current word*. Ausbau am Rastern der Zeile (heute `vorn` /
   `hinten`), neu rastern je Wortwechsel.
3. **Zeilen und Worte per Index ansprechen** (Frage Patrik: „kann man die
   diversen Zeilen mit Index ansprechen, und Worte auch mit Index pro
   Zeile"). Zeilen: ja, seit S89 über `line` im Slot Line
   (`if(equal(line,3), …)`). Worte: fehlt — Vorschlag: Variablen `word`
   (laufendes Wort der Zeile, 0 = vor dem ersten) und `wordcount`, dazu ein
   Slot **Word** (einmal je Wortwechsel, wie Line). Offen, ob ein Skript
   auch **andere** Zeilen/Worte als die laufenden stellen können soll
   (Tabelle je Index statt „nur das Laufende") — erst klären.
4. **Wort-Zeiten erzeugen:** Lyrics Sync setzt heute nur Zeilen-Zeiten;
   Klopfen je Wort im Panel fehlt (die Wort-Zeiten des Dienstes sind seit S88
   zurückgestellt).
5. **`linetrigger` / `wordtrigger` für unsynchronen Text** (Idee Patrik):
   Skriptvariablen, die den Block weiterrücken — `linetrigger = 1` eine Zeile,
   `wordtrigger = 1` ein Wort —, nach dem Muster von `switch` / `nextgroup` am
   Host Switcher (gelten nur für den Frame, in dem sie gesetzt werden). Heute
   rückt unsynchroner Text nur über *beats per line*. Damit ließe sich z. B.
   auf Bass-Schläge oder eine eigene Zählung weiterschalten; `wordtrigger`
   setzt die Wort-Ebene aus Punkt 2/3 voraus.

Reihenfolge-Vorschlag: 1 (Puffer) → 2 + 3 (Wort-Ebene) → 5 (Auslöser) → 4
(Sync-Panel). Vor dem Bau ein Kurzkonzept mit Entscheiden, wie bei der
Karaoke-Ebene.

### ✅ Host Switcher: Puffer je Gruppe — „Keep group state" (Idee Patrik S89, umgesetzt S89 — Sichttest offen)

**Idee:** der Host Switcher hält für jede gelistete Gruppe einen Puffer
(anfangs leer). Wird die Gruppe verlassen, bleibt ihr Stand im Puffer; wird
dieselbe Gruppe wieder betreten, läuft sie von dort weiter. Das Puffern ist
einschaltbar, die Puffer lassen sich zurücksetzen; bei leerem Puffer beginnt
die Gruppe einfach neu.

**Befund (S89, am Code und im Test):** ohne Puffer verliert eine fertig
ausgeblendete Gruppe am Frame-Ende ihre **ganze Gruppen-Laufzeit** (Bild,
Buffer-Save-Pool, Skript-Kontext reg/q/gmegabuf, Laufzeit `activeSeconds`) und
die Laufzeiten ihrer verschachtelten Effect Lists — `renderNode` zeichnet sie
nicht mehr, und was einen Frame lang nicht gesehen wurde, wird weggeräumt. Die
Laufzeiten der **Knoten** darin (Skriptvariablen, Partikel, Milkdrop-Zustand)
bleiben dagegen liegen: der heutige Neustart ist also kein vollständiger.
(Die erste Lesung in S89 — „Pool und Kontext bleiben erhalten" — war falsch;
der Bildtest hat es gezeigt.)

**Umgesetzt:**

- Feld **Keep group state** am Host Switcher (`HostSwitcherParams::keepState`,
  Vorgabe aus, skriptbar als `keepstate`). An: die ruhenden Gruppen der Liste
  behalten Bild, Pool, Skript-Kontext, Laufzeit und ihre verschachtelten
  Listen (`m_gruppenPuffer`, `haltePufferAmLeben`); gezeichnet wird weiter
  nichts, solange sie ruhen.
- **Zurücksetzen** (Entscheid Patrik a — wirklich frisch): Knopf **Reset
  buffers** im Editor und Skriptvariable `clearbuffers = 1`
  (`leereSwitcherPuffer`, `leerePufferVon`): Bild, Laufzeit, Pool,
  Skript-Kontext, verschachtelte Listen **und** die Laufzeiten aller Knoten
  der ruhenden Gruppen — die Init-Skripte laufen beim nächsten Betreten
  wieder. Die laufende (oder noch ausblendende) Gruppe bleibt unberührt.
- Ohne das Feld bleibt alles wie vor S89 (Vergleichslauf 84/84 bitgleich).
- Geprüft im Bildtest (`test_HostSwitcher.cpp`): ohne Puffer beginnt das Bild
  nach der Rückkehr leer; mit Puffer läuft es weiter (auch wenn eine
  verschachtelte Liste das Bild trägt); `clearbuffers` und der Knopf leeren;
  die laufende Gruppe wird nicht angefasst; Roundtrip des Felds.

**Offen / Grenzen:**

- **Sichttest** mit echten Presets (Milkdrop-Node, Partikel, Video Source in
  einer ruhenden Gruppe).
- Ein Größenwechsel leert die Bilder (andere Auflösung) — die übrigen
  Zustände bleiben.
- Speicher: je gepufferter Gruppe bleiben ihre Flächen im VRAM liegen, auch
  die der verschachtelten Listen und die Buffer-Save-Puffer.
- Nur zur Laufzeit. Über App-Neustart oder Neuladen der Kette hinweg müssten
  Bilder auf Platte — eigener, größerer Schritt, nicht Teil dieser Idee.
- Der Puffer hängt am Switcher: eine von Hand (Auge) geschaltete Gruppe
  verhält sich wie bisher.

### ✅ Effekt-Palette als Fenster statt Dropdown (Entscheid + Umsetzung S79 — Sichttest offen)

**Umgesetzt S79** (`EffectPaletteWindow`, Daten `EffectPalette.{hpp,cpp}`,
Doku `include/UI/panels/MultiEffectPanel.md` §Funktion). **Sichttest Patrik
offen:** Suche/Enter, Chips, ★/Zuletzt, Drag in Liste/zwischen Knoten/auf
Leerfläche, Milkdrop-Element auf einen Milkdrop-Node, Fenster bleibt offen.
Die Rollen-Zuordnung (Punkt darunter) ist mit umgesetzt — Filter-Chips im
Fenster; eine Rollen-Spalte im Kettenbaum steht noch aus.

Die Add-Combo trug 84 Module in 12 Kategorien, ohne Suche und
Filter. **Entscheid S79:** ein **nicht-modales Palettenfenster** ersetzt sie —
„+" öffnet es; Button „Hinzufügen" (in die gewählte Liste, sonst Root) **und**
Drag & Drop in den Kettenbaum; es bleibt offen, bis man es schließt (X oder
„Schließen"). Inhalt: Suchzeile (Name, Original-AVS-Name, Beschreibung),
Kategorie-Baum mit Zählern, Filter-Chips Herkunft (AVS/APE/MilkDrop/LumiViz/
GLSL) und **Rolle** (zeichnet / verändert Bild / Farbe / Steuerung — der
Punkt darunter), zuletzt benutzt + Favoriten ★, Kurzbeschreibung mit Hinweis
auf Formelfelder. Das Fenster hängt nicht an der Panelbreite (S78-Vertrag
bleibt unberührt).

### ⚪ Effekt-Palette: Rolle sichtbar machen + filtern (Wunsch Patrik, 2026-08-06)

Die Palette/Baumliste zeigt heute nur Herkunft (AVS/MilkDrop/LumiViz-Icon),
nicht die ROLLE eines Moduls — Render (zeichnet) vs. Transform (verformt das
Bild davor; Anlass: Mesh Warp alleine in der Kette = schwarz, war als
Transform nicht erkennbar). Ideen (Ausarbeitung in eigener Session):
zweites Typ-Icon neben dem Herkunfts-Icon ODER eigene Typ-Spalte; nach
Rolle filterbar; womöglich ist das Add-Dropdown insgesamt das falsche
Werkzeug dafür. Erst konzipieren, dann bauen.

### ⚪ Shadertoy-Buffer-FBOs: GL_NEAREST statt GL_LINEAR (Befund Tutorial-Screenshots, 2026-08-05)

Die Multipass-Buffer des Shadertoy-Nodes filtern beim Lesen GL_NEAREST
(Qt-FBO-Default, kein `glTexParameteri` im Render-Pfad) — shadertoy.com
filtert LINEAR. Folge: Feedback-Shader mit Unsharp-Mask/Sharpen explodieren
in Pixelgrieß, Blur-/DOF-Taps mit Zwischenpositionen brauchen Workarounds.
Beweis + Workaround: die generierten Tutorial-Chains (PimpedKaleidoscope,
CompositePostfx) tragen eine `lesBilinear()`-GLSL-Anpassung, als
„LumiViz-Anpassung" im Shader-Kopf markiert. **Fix:** je Buffer-FBO
`GL_TEXTURE_MIN/MAG_FILTER = GL_LINEAR` setzen; danach die
`lesBilinear`-Workarounds aus den Chains zurückbauen und die betroffenen
Tutorial-Bilder neu rendern. (Ein zuvor angebotener Task-Chip hierzu wurde
geschlossen — dieser Eintrag ist der Merkposten.)

### ➜ GPU-Vertex-Module (Strang G — **G1+G2 ✅ umgesetzt S69, Sichttests offen; G3 = Kür-Notiz**)

**Legacy-Imports bleiben CPU:** das per-Vertex-EEL läuft im Original
SEQUENTIELL (Zustand über Vertizes: Akkumulatoren, rand()-Strom, gmegabuf) —
GPU-Parallelität kann diesen Vertrag prinzipiell nicht halten, und 825
Mesh-Vertizes kosten auf der CPU nichts (die teure per-Pixel-Arbeit ist
längst GPU). **GPU-Vertex-Arbeit gehört in NEUE Module (modernes Regelwerk).**

**G1 Mesh-Warp ✅ (S69):** Chain-Node `meshWarp` — Nutzer-GLSL
`vec2 warp(vec2 uv)` je Gitter-Vertex im Vertex-Shader (Gitter 2..256×192,
Mix-Regler, Wrap/Clamp im Shader), `MeshWarpWrapper.hpp` (GL-frei, 9 Tests),
Panel mit GLSL-Groß-Editor (Apply/Beautify/Import/Export), Parameter-Skripte
`gridx`/`gridy`/`mixamount`. **Entscheid Patrik: G1 VOR Vereinheitlichung
V2** — Audio ad-hoc als Uniforms (Shadertoy-Muster); Details
[Regelwerk_und_Neue_Module_Plan.md](visuals/Regelwerk_und_Neue_Module_Plan.md)
1.5.0. Sichtbeweis `asset/effectchain/meshwarp_sonde.lvfx`
(`out/meshwarp_sonde_s69/`). **⬜ Sichttest Patrik offen.**

**G2 GPU-Partikel ✅ (S69):** Chain-Node `gpuParticles` (Render-Modul) —
Zustand als RGBA32F-Ping-Pong (ein Texel = pos+vel), Alter/Respawn
hash-basiert OHNE Speicher (deterministisch, kein rand()), instanzierter
Sprite-Draw (max. 65536 Partikel), 17 Regler + Farben/Additiv, optionales
Kraftfeld-GLSL `kraft(pos, vel, alter)` mit Audio-Uniforms (Groß-Editor
komplett), Parameter-Skripte `spawnx…size`. Details
[Regelwerk_und_Neue_Module_Plan.md](visuals/Regelwerk_und_Neue_Module_Plan.md)
1.6.0; Sichtbeweis `asset/effectchain/gpuparticles_sonde.lvfx`
(`out/gpuparticles_sonde_s69/`). **⬜ Sichttest Patrik offen.**

Kür-Idee G3 (Notiz): „Mesh-Qualität"-Option für nachweislich ZUSTANDSLOSE
per_pixel-Skripte (Transpiler-Analyse) mit GPU-Auswertung und feinerem
Gitter als Modern-Schalter.

### ⚪ fractalZoomer: float-Erschöpfung im Endlos-Zoom (Befund S61)

Der Zoomer akkumuliert seinen Zoom unbegrenzt — nach Minuten Laufzeit ist
float32 erschöpft, das Bild wird Pixelbrei oder schwarz (Befund Patrik,
Sichtprüfung der Beispiele; die 120-Frame-Validierung konnte das nicht sehen).
Die Vorlagen sind entschärft (langsamere zoomSpeeds), das verlängert aber nur.
**Echter Fix ist Host-seitig:** Tiefen-Reset bzw. Zoom-Loop (bei erreichter
Präzisionsgrenze weich auf den Startausschnitt zurückblenden), ggf. plus
double-Präzision fürs Zentrum. Der akkumulierte Zoom ist per Skript nicht
erreichbar (nur `zoomspeed`), ein Vorlagen-Workaround existiert also nicht.

### ⬜ Video-/Kamera-Quellmodul + Stilfilter (Wunsch Patrik S55; Teil A umgesetzt S70)

**Stand S70: Teil A (Quellmodul) UMGESETZT — Sichttest offen.** Neuer
Chain-Node `videoSource` (Palette „Scopes & Sources"): Quelle
Datei/Kamera/Testaufnahme, Echtzeit-Streaming (NEU `services/LiveVideoFeed`)
vs. deterministischer Frame-Schritt (VideoFrameCache 1.1.0, NEU `Clip::fps`),
Tempo/Schleife/Einpassung/Blend/Deckkraft, Parameter-Skripte `speed`/`opacity`,
relative Pfade lösen gegen den Preset-Ordner auf. Settings-Tab „Kamera" mit
Testaufnahmen (benutzerlokal, AppData); Kamera-Freigabe je App-Lauf
(Panel-Knopf bzw. Aufnahme-Klick — nie beim Preset-Laden). Sonde
`videosource_sonde.lvfx` (Doppellauf SHA256-identisch, Warnungen=0,
Sichtprüfung Balken+Letterbox ok). Benutzerhandbuch §13.
**BEFUND S70 (behoben): `QVideoFrame::toImage()` liefert unter Qt 6.10.1
(FFmpeg-Backend) SCHWARZE Bilder bei korrekten Zeitstempeln** — Rohdaten per
`map()` vollständig da. Workaround NEU `services/VideoFrameUtil.hpp`
(`videoFrameZuBild`), gilt für LiveVideoFeed UND VideoFrameCache — damit war
auch der avi-Cache-Fallback (VR09, S59) auf dieser Qt-Version schwarz.
**Kamera-Teardown-Deadlock: ✅ GELÖST UND ABGENOMMEN (S71, Sichttest Patrik).**
Ursache war **der eigene Thread für die Multimedia-Seite** — A/B am Prüfstand
mit echter Kamera: mit Worker-Thread 2/2 Hänger, ohne 3/3 sauberes
Prozessende; unabhängig von Media-Backend (FFmpeg/WMF), von der Lage der
Pipeline und von COM-Init. Qt-Multimedia läuft deshalb wieder auf dem
Main-Thread (Stand `f93c83a`), und der UI-Lag bleibt dank der 720p/30fps-Klemme
aus — der Worker-Thread war für die Bildrate gar nicht nötig.
**`std::_Exit()` war nie eine Lösung** und ist entfernt: es beendet den Prozess
nicht, wenn ein Thread im Treiber steckt (die Logzeile „erzwungenes
Prozessende" täuschte seit S70 einen Erfolg vor). Auf dem Weg dorthin ebenfalls
behoben und behalten: Abschalt-Vertrag `feedStilllegen()`, `altenFeedRaeumen()`
und der Bau-Riegel `m_imBau` (Details Changelog 1.60.0–1.63.0,
`include/services/LiveVideoFeed.md` §4a). Regelwerk:
`docs/core-services/Bootstrap_Integration.md` §6 (neu §6.6). (Knoten-Panel, Kamera-Pfad ✅ Patrik S70
(USB-WebCam läuft, Lag behoben), Testaufnahme-Aufnahme, Streaming-Langvideo) · Demo folgt
mit Teil B (`pixelFilter` — Video + Comic-Filter ist das eigentliche
Schaufenster) · Teil B siehe Stilfilter-Architektur unten ·
⚪ **Rest-Risiko fremde Kameras:** `videoFrameZuBild` wandelt nur
RGB-Formate selbst; YUV (NV12/YUYV) fällt auf `toImage()` zurück — liefert
eine Kamera nur YUV UND trifft dort der toImage-Schwarz-Bug, bliebe sie
schwarz. Rezept: manuelle NV12/YUYV→RGB-Wandlung in `VideoFrameUtil.hpp`
nachrüsten, erst wenn eine echte Kamera es zeigt. ·
**NEU S70: Kamera-Freigabe-Dialog beim Preset-Laden** (Wunsch Patrik) —
`MultiEffectPanel::pruefeKameraFreigabe()` fragt einmal je geladener Kette,
wenn sie eine aktive Kamera-Quelle trägt und die Freigabe fehlt (der Dialog
ist die ausdrückliche Nutzeraktion; Standalone bleibt dialogfrei). Als
EINZIGE Ausnahme erscheint er auch über dem Vollbild (Eltern = aktives
Fenster + WindowStaysOnTop — Entscheid Patrik S70). ·
**BEFUND S70 (behoben, Stack-Beweis Patrik): Zombie-Prozess nach App-Ende
bei benutzter Kamera.** Zwei Schichten: (a) die Queued-Stopps des
LiveVideoFeed kommen beim Herunterfahren nicht mehr an → Fix
`alleStoppen()` synchron an `QCoreApplication::aboutToQuit`. (b) Der
eigentliche Hänger (Haupt-Thread-Stack: `Application::shutdown` →
`~QApplication` → Event-Queue-Entsorgung → `~Feed`): das stopp()-Lambda
trug den LETZTEN shared_ptr auf den Feed — wird das Event nie zugestellt,
zerstört die Event-Queue-Entsorgung die QCamera MITTEN im
QApplication-Abriss → Deadlock. Fix: Eigentum bleibt IMMER beim Dienst
(`m_friedhof`), Queued-Lambdas tragen nichts; `alleStoppen()` räumt Feeds
UND Friedhof synchron ab und zerstört die Qt-Objekte explizit (Kamera-LED
geht beim Fenster-Schließen übrigens trotzdem aus — Windows beendet nur
die Geräte-Sitzung, nicht die Prozess-Objekte). Merkregel: **Queued-Lambdas
dürfen nie letztes Eigentum an Qt-Multimedia-Objekten tragen.**

Ursprüngliche Sondierung (S55, weiter gültig für die Rest-Punkte):

- **Quellmodul** (LumiViz-eigen, **neben** dem AVS-`avi`-Knoten — der bleibt, die
  Kalibrierung hängt an ihm). Ein Knoten, ein Quellumschalter: **Datei oder
  Kamera**. Umfang laut Entscheid Patrik: Datei **und** Kamera **und**
  Frame-Schritt.
- **Technik steht bereit:** Qt Multimedia liegt in der Installation
  (`C:/Qt/6.10.1/msvc2022_64`), Backends `ffmpegmediaplugin.dll` **und**
  `windowsmediaplugin.dll`. Damit MP4/H.264, MKV, WebM, MOV, WMV — **ohne neue
  Fremdbibliothek**. In `Solution.json` nur `"Multimedia"` zu den Qt6-Komponenten
  plus Plugin-Deploy. Kamera über `QMediaDevices::videoInputs()` +
  `QCamera`/`QMediaCaptureSession`; beide Quellen liefern über **denselben**
  `QVideoSink`, deshalb ein Knoten und nicht zwei.
- **Frame-Schritt ist Pflicht**, nicht Kür: Qt liefert Frames uhrzeitgetrieben,
  der `avi`-Knoten holt sie nach Index. Nur deshalb sind zwei Läufe bit-identisch
  — die Grundlage der ganzen Feld-Sonden-Familie (`STUMM` = MAE exakt null). Ohne
  eine deterministische Betriebsart für den Standalone wäre der Knoten
  `NICHT_PRUEFBAR`.
- **Kamera nie automatisch öffnen** — nicht im Standalone, nicht in Tests, im
  Panel erst auf ausdrückliche Gerätewahl. Sonst fragt Windows zur Unzeit nach
  der Kameraberechtigung.
- **Stilfilter sind NICHT Teil des Quellmoduls.** Die Kette arbeitet auf dem
  Framebuffer, ein Filterknoten wirkt also auf **jede** Quelle — Video, Kamera,
  Superscope, MilkDrop. Wunsch Patrik: **Comic-/Rotoskopie-Look wie „Take On Me"
  (a-ha)**. Technisch drei Bausteine, je ein Fragment-Shader-Schritt:
  Kantenzug (Sobel oder Difference-of-Gaussians = Bleistiftstrich) ·
  Farbquantisierung auf wenige flache Töne (optional mit Bilateral-Vorglättung,
  damit die Flächen ruhig werden) · Schraffur/Rauschen für die Zeichentrick-
  Textur. Reiht sich neben die anderen Stil-Ideen (s. Lights-Module).
- **Erhebungen/Entscheide S70 (Diskussion Patrik, Strang eröffnet):**
  - **Abkürzer:** Qt Multimedia ist seit S59 im Build (`"Multimedia"` in
    `Solution.json`) und `services/VideoFrameCache` (deterministischer
    Frame-Index-Decode via FFmpeg-Backend) existiert — der Datei-Pfad des
    Quellmoduls setzt darauf auf, neu sind Node/Panel/Kamera.
  - **Kamera-Testaufnahmen in den Settings** (Idee Patrik): Settings-Tab
    „Kamera" — Gerätewahl, Aufnahme-Knopf (wenige Sekunden), Ablage
    **benutzerlokal** (AppData; NICHT im Repo, Kameramaterial ist persönlich —
    für eingecheckte Tests stattdessen ein synthetisch erzeugter Clip).
    Dreifacher Nutzen: der Windows-Berechtigungsdialog kommt beim bewussten
    Klick statt zur Unzeit · die Aufnahme ist eine Datei → läuft über den
    VideoFrameCache im Frame-Schritt, damit wird der Kamera-Pfad
    sonden-/standalone-prüfbar · der Quellknoten bekommt drei Betriebsarten
    **Datei · Kamera (live) · Testaufnahme** (letztere auch als Fallback,
    wenn kein Gerät da ist).
  - **Stilfilter-Architektur entschieden UND UMGESETZT (Teil B, S70 —
    ⬜ Sichttest offen):** EIN skriptbares Filtermodul **`pixelFilter`**
    (Nutzer-GLSL `vec4 farbe(vec2 uv, vec4 src)` mit Nachbar-Sampling fürs
    Ketten-Bild; meshWarp-Muster: Audio-Uniforms, geteiltes `stError`,
    Groß-Editor mit Apply/Beautify; NEU `PixelFilterWrapper.hpp` GL-frei +
    testerzwungen) — die Filter selbst als **Werks-Voreinstellungen**
    (`asset/nodepresets/pixelFilter/`, 12 Looks inkl. Katalog-Zeilen), NICHT
    1–3 Festmodule. **BEFUND S70: `filter` ist in GLSL ein RESERVIERTES
    Wort** (AMD lehnt ab) → Vertrag heißt `farbe()` (kraft()-Muster);
    Wächter-Test erzwingt, dass der Wrapper das Wort nie als Bezeichner
    trägt. Flaggschiff **Take-On-Me-Comic** (Sobel-Kantenzug +
    Farbquantisierung + Beat-zitternde Schraffur), dazu Bleistift-Skizze
    (XDoG), Posterize-PopArt, Zeitungsdruck-Halftone, CRT-Monitor, VHS-Band,
    Kuwahara-Oelbild, Sepia-Nostalgie (Wunsch Patrik), Noir-Schwarzweiss,
    Waermebild, Pixel-Art, Duotone-Neon. Beweise: `pixelfilter_sonde.lvfx`
    (Doppellauf SHA256-identisch; Posterize-Stufen + Konturen sichtgeprüft)
    + Demo `asset/examples/pixelFilter - Take On Me.lvfx` (Stimm-Befund:
    Comic-Szenen brauchen HELLE Grundflächen und harte Kanten — dunkle
    Hintergründe verschraffieren vollflächig). Benutzerhandbuch 1.8.0 §14.
    Grenze der Bauart: EIN Pass — Multipass-Looks laufen über den
    Shadertoy-Knoten (s. iChannel-Erweiterungen unten). Der Filter-Stack
    ist die Kette selbst (mehrere Knoten, umsortierbar; Entscheid Patrik
    S70 — keine interne Unterliste, Klone sind unabhängige Kopien).
  - **✅ LIZENZ-PFLICHT bei JEDEM Fremd-Import (Vorgabe Patrik S71 — muss im
    Loader stecken, nicht nur in der Doku, sonst geht es unter)** — umgesetzt
    (Changelog 1.67.0: `Herkunft` in `EffectChain.hpp` mit Name, Autor, URL,
    Lizenz, gesetzt von ISF-Import und Shadertoy-Browser, gespeichert im
    Preset, angezeigt im Panel; bereinigt S129): Shadertoy
    verpflichtet API-Nutzer ausdrücklich, die Lizenz **jedes einzelnen
    Shaders** zu respektieren (Default dort ist meist CC BY-NC-SA); ISF trägt
    ein `CREDIT`-Feld im JSON-Kopf. **Regel: ein Import schreibt Herkunft
    (URL/ID bzw. Dateiname), Autor und Lizenz als Felder ins Preset** und
    zeigt sie im Panel an — sonst wandern fremde Shader unbemerkt in
    weitergegebene `.lvfx`. Beim Bauen des Importers ist das Teil der
    Pflichtkette, nicht Kür. Zu klären ist dabei, ob die Felder in den
    ChainNode oder in die Preset-Metadaten gehören (Serializer + FieldDocs).
  - **Filter-Fundgruben online:** **ISF (Interactive Shader Format,
    isf.video / editor.isf.video, Vidvox)** = „Shadertoy für Filter" —
    GLSL + JSON-Parameterdeklaration. **Format geklärt (S71, gegen die Spec
    geprüft):** das JSON steckt als **Blockkommentar `/*{ … }*/` am
    Dateianfang**, es gibt KEINE Nachbardatei; Endung **`.fs`** (optional
    `.vs` für einen Vertex-Shader gleichen Namens) — beides sind normale
    GLSL-Dateien mit spezialisierter Endung. **FX-Erkennung ist maschinell
    eindeutig: ein Filter hat einen Input `inputImage` vom Typ `image`**
    (Generatoren haben keinen, Übergänge stattdessen `startImage`/`endImage`/
    `progress`). Übersetzungsschicht auf `pixelFilter`: INPUTS-Typen
    (`float`/`bool`/`long`/`color`/`point2D`/`image`/`audio`/`audioFFT`) →
    unsere Regler; ISF-Uniforms `RENDERSIZE`/`TIME`/`TIMEDELTA`/`FRAMEINDEX`/
    `isf_FragNormCoord` und die Helfer `IMG_PIXEL()`/`IMG_NORM_PIXEL()`/
    `IMG_SIZE()` → unser `uv`/`uTex`. → **ISF-Import (Teilmenge FX) als
    eigener Strang.** Außerdem:
    ReShade-Shader-Repos (github.com/crosire/reshade-shaders, qUINT,
    SweetFX — HLSL-Dialekt, Ideen-Quelle) · godotshaders.com (Kategorie
    „Screen-reading shaders") · obs-shaderfilter-Sammlungen.
    **Shadertoy-Suchworte für Bildfilter:** toon, cel shading, kuwahara,
    hatching, pencil, halftone, CRT. Literatur-Stichworte: **XDoG**
    (Winnemöller — Bleistift/Comic-Kanten) · **anisotropes Kuwahara**
    (Ölbild-Flächen).
  - **iChannel-Erweiterungen des Shadertoy-Knotens** (machen ihn zur
    Multipass-Filterplattform; Shadertoy-Bildfilter mit `iChannel0`-Eingang
    werden direkt lauffähig): (1) NEU Quelle **„Ketten-Eingang"** — heute
    kann ein iChannel nur Audio oder Buffer A–D (Kommentar „Video-Inputs:
    Stufe 2" in `MultiEffectVisualizer` setupPass); (2) NEU Quelle
    **„AVS-Global-Buffer 1–8"** (Frage Patrik S70 — **geht**: die
    Buffer-Save-Slots sind GL-FBOs im `OffscreenBufferPool`,
    `activePool().get(slot,…)->texture()` bindet direkt als Textur;
    Host-Gruppen-Scoping kommt über `activePool()` gratis mit; Semantik =
    Buffer-Stand beim Lauf des Knotens, die Reihenfolge in der Kette zählt
    wie bei Buffer Save). **Handbuch-Pflicht (Wunsch Patrik S70):** bei
    Umsetzung beide neuen iChannel-Quellen ins `Benutzerhandbuch.md` §12
    (Kanal-Tabelle: Audio · Buffer A–D · Ketten-Eingang · AVS-Buffer 1–8,
    je mit Anwendungsbeispiel — z. B. Shadertoy-Bildfilter auf die Kette,
    eingefrorenes Buffer-Save-Bild als Textur).
- Abgrenzung: **nicht** das Video-**Capture**-Modul (Aufnahme von Bild+Ton) —
  das ist der Punkt unten in derselben Liste, umgekehrte Richtung.

⚪ Preset-Warmup/Pre-Roll (bei Laden/Resize N Frames vorrechnen — gegen
Schwarz-Start/Flackern) · BASS-Lookahead-Service `AudioLookahead`
(Lights-Entwurf Entscheid 3 — hierher verschoben S60, erst mit den
Lights-Modulen) · Custom-Functions-Modul · Video-Capture-Modul ·
dynamische Modulparameter (alle Params per init/frame/beat/point) ·
Stereo `bass`/`mid`/`treb` · Variable-Set Variante B (benannte Globals) ·
Assets-Ordner-Fallback für Bild-Lader · **MilkdropRef** (zurückgestellt —
Reaktivierungs-Kriterium: ein MilkDrop-Treue-Bug, der nach mehr als einer Session
Diagnose keine klare Ursache hat. Für *Semantik*-Fragen reicht der Quelltext:
`cmake/ref/winamp_orig/Src/Plugins/Visualization/vis_milk2/`).

### ✅ Zwei unbekannte APEs nachgebaut (S52, Befund Patrik)

`Metaballs 3D` und `Tentacles 3D` (beide UnConeD, Pack „Whacko AVS IV") waren
Passthrough — „Yummy Plastics" und „Rubber Starfish" blieben deshalb **leer**. Beide
sind seit S52 als **Verhaltens-Nachbau** umgesetzt (Parser → Params → Translator →
Renderer → Serializer), Muster wie FyrewurX (S38): ihr 72-Byte-Blob trägt **nur eine
Farbtafel** (16 Slots + Anzahl), die Geometrie ist host-eigen. Beide Presets laden
jetzt warnungsfrei und zeichnen.

**Was daran offen bleibt** — ein Nachbau ist kein Port: die Metrik wird nie
konvergieren (Yummy Plastics dMean 0,205), weil Bahnen, Anzahl und Farbfolge unsere
sind. Beurteilbar ist nur der *Charakter*, und der stimmt im Seite-an-Seite:
verschmelzende, plastisch schattierte Körper mit Glanzlicht bzw. schwingende Tentakel.
Drei Punkte wurden dabei am Bild der echten APE kalibriert (Deck- statt Additiv-Blend,
gewichtete statt nächster-Nachbar-Farbe, Schattierung über die Kuppelhöhe statt über
den rohen 1/r²-Gradienten). Feinschliff ist Kür. 🔧

### ➜ Vorgaben Patrik für Session 54

Ausgearbeitet als **Etappen 7–9** im
[Knoten_Parameter_Konzept.md](visuals/Knoten_Parameter_Konzept.md) (§9–§11):

1. ✅ **Test-Presets für JEDES Modul und JEDES Feld** (Strang E, Konzept §9) —
   umgesetzt als Feld-Sonden (S54, §8 „Strang E — Feld-Sonden“), Vollauf grün
   (§0 Haken 3, S60); offen nur der erste Lauf der 60 Sonden aus S73 (§4;
   bereinigt S129). Ein
   Preset je Feld, nur dieses eine vom Default abweichend; für Transformationen
   mit klar definiertem statischem Untergrundbild wie auf der MilkDrop-Seite.
   Urteil = zwei Läufe (Default vs. gesetzt); kein Unterschied heißt: das Feld
   wirkt nicht. Nach S53 dringlich — 47 Renderer haben je drei neue Skriptfelder,
   und `movement3b.lvfx` hat gezeigt, dass ein Feld dastehen kann, ohne wirken zu
   können (§9 unten).
2. ✅ **Tooltip an JEDEM Feld** (Strang F, Konzept §10) — **erledigt S56,
   717/717 Felder.** Erzeugte Tabelle `src/UI/panels/FieldDocs.cpp` aus den
   Doxygen-Kommentaren, Feldname als erstes Argument in allen **826**
   `add*`-Aufrufen (maschinell über die Setter-Zuordnung, nicht über das Label),
   Tooltip auf Bedienelement **und** Beschriftung. Von den 178 gemeldeten Lücken
   waren **163 echt** (geschrieben) und **15 Blindstellen des Ernters**.
   Zwei Wächter: `test_FieldDocs.cpp` hart ohne Ausnahmeliste, und der Ernter
   prüft die Panel-Schlüssel gegen das Feld-Inventar.
   **Nicht erprobt:** dass der Hinweis im Betrieb erscheint — das geht nur durch
   Überfahren.
3. 🟡 **Basis-Voreinstellungen für alle Module** (Strang G, Konzept §11) —
   **Entscheid Patrik (S60):** (a) Umfang = **Teil-Presets** (nur Geometrie/
   Verhalten, Farbtafel bleibt) · (b) Ablage `asset/nodepresets/<typkey>/`,
   Dateiname = sprechender Anzeigename wie die 16 bestehenden Vorlagen
   („Kalter Schwarm.json") · (c) Quelle = `…\cmake\VisualsPresets` via Import.
   **Erste Lieferung ✅ (S60, nach §0-Abschluss): 68 neue Vorlagen für 23
   Typen** (u. a. dynamicMovement 6 · movement 12 — davon 8 wörtliche
   r_trans-Builtin-Formeln — · convolution 5 · colorMap 4 · rotoBlitter 3 ·
   waterBump/dotGrid/blitterFeedback je 3 · dynamicShift/DDM/mirror/
   interferences/starfield/uniqueTone/interleave/videoDelay u. a. je 2) —
   Hand-Design aus den kalibrierten Referenz-Semantiken, Katalog mit
   Erklärungen in `asset/nodepresets/README.md`.
   **Zweite Lieferung ✅ (S61): 41 neue Vorlagen, Katalog jetzt 125 für 39
   Typen** — (a) **Fraktal-Familie + Bloom** (27): bloom 3 (Referenzwerte aus
   `lights_demo`) · fractal2D/fractal3D/domainWarp/fractalZoomer je 3 ·
   lyapunov/kleinian/flame je 2 · strangeAttractor/reactionDiffusion je 3;
   (b) **Histogramm-Lücken per Hand** (8): blur 2 · simpleScope 3 ·
   customBpm 3; (c) **Import-Ernte** (6): kuratierte Dynamic-Movement-Warps
   wörtlich aus der Sammlung (UnConeD Milkyway, nebulous, Take the Veil,
   Molecules, Life Is Violated — Quellen im Katalog). Ernte-Werkzeug:
   `AvsStandalone --dump` über Whacko AVS V + Winamp 5 Picks (89 Presets →
   455 SuperScope-/100 DM-/120 Movement-/30 ColorMap-Kandidaten, Rest
   unkuratiert im Scratchpad). Der Ladbarkeits-Test deckt alle neuen Dateien
   (485/485). **Offen:** Sichtprüfung Patrik im Betrieb (jetzt 125, insb.
   die geschätzten Wertebereiche bei starfield/interferences/interleave und
   die neuen Fraktal-Ausschnitte) · weitere Ernte-Runden (SuperScope-Szenen,
   Movement, ColorMap-Paletten) · **Host-Klassiker (Equalizer/Oscilloscope/
   Waveform/Pulsing): 🟡 ENTSCHEID NÖTIG** — deren Presets laufen über den
   `VisualizerPresetManager`, der NUR den Benutzerordner (AppData) kennt;
   mitgelieferte Vorlagen bräuchten dort erst einen Asset-Suchpfad nach dem
   Muster von `NodePresetStore` (kleiner Umbau, wartet auf Freigabe).
   **Priorisierung liegt bereit** (S60-Häufigkeits-Scan über 3380 Korpus-
   Presets; Anteil = Presets, die den Effekt nutzen): Dynamic Movement 63 % ·
   Movement 57 % · Simple 45 % · Color Map 39 % · Convolution 34 %
   (Kernel-Vorlagen: Blur/Schärfen/Kanten/Relief!) · SuperScope 29 %
   (13 Vorlagen existieren) · Blur 26 % · Water 24 % · Unique Tone 23 % ·
   Bump 23 % · Buffer Save 21 % · Roto Blitter 18 % · Mirror 16 % ·
   Custom BPM 15 % · Texer II 5 %. Dazu die host-eigenen Module ohne
   AVS-Statistik (Equalizer · Oscilloscope · Pulsing · Fraktal-Familie —
   Vorlagen dort = Hand-Design mit Sichtprüfung).
4. ✅ **Feldreihenfolge im Editor — entschieden (Patrik, S60): es bleibt bei
   Init · Frame · Beat · Point.** Die Anzeige folgt der Ausführung
   (`runParamScript` und alle Skript-Träger fahren Frame VOR Beat, wie AVS)
   und der AVS-Dialog-Konvention — kein Umbau.

### ✅ Movement: Beat-Umkehr kann strukturell nicht wirken (Befund Patrik S53 — entschieden S53, s. u.)

*(S129: die toten Felder `initCode`/`frameCode`/`beatCode` der beiden Movement-Knoten
sind aus der Datei genommen — der Serialisierer liest sie seit dem Entscheid
nicht mehr, am Bild ändert sich nichts; der Point-Code mit `bt` blieb. Der Text
hier beschreibt die Datei, wie sie in S53 war.)*
`asset/effectchain/movement3b.lvfx` setzte `bt=1` (Init), `bt=(-1)*bt` (Beat) und
liest `bt` im Point-Code (`r=r+bt*0.02`). Die Rotation kehrt nie um — aus **zwei**
unabhängigen Gründen:

1. **Zwei getrennte Skript-Umgebungen.** Init/Frame/Beat laufen seit S53 im
   Parameter-Skript (`runParamScript`, eigener `ScriptSlotHost`), der Point-Code
   dagegen im `ScriptGridModule`. Geteilt sind zwischen Hosts nur `reg00..reg99`,
   `q1..q64` und `gmegabuf` — ein freier Name wie `bt` ist im Point-Code schlicht
   0. Damit ist `r=r+bt*0.02` ein No-op; nur `d=d-0.02` wirkt.
2. **Movement ist eine statische Tabelle.** `applyMovementTable` cacht über
   `rectCoords + wrap + subpixel + code` — den **Skripttext**, nicht die Werte.
   Der Point-Code läuft nur bei Größen- oder Textänderung (r_trans.cpp:453-526,
   w*h Skript-Läufe). Selbst mit geteiltem `bt` bliebe das Bild stehen. AVS
   macht es genauso — deshalb hat AVS-Movement gar keinen Beat-Code.

Für beat-abhängige Bewegung ist **Dynamic Movement** der richtige Knoten: er
wertet je Frame aus und hat Init/Frame/Beat/Point in EINEM Host. Der Editor weist
seit S53 darauf hin.

✅ **Entschieden (Patrik, S53): Movement verliert seine Strang-D-Felder wieder.**
Sie hätten nur `sourcemapped` und `blend` je Frame umschalten können — alles
Geometrische landet im Tabellen-Cache. Der Nutzen stand in keinem Verhältnis zur
Verwirrung, die sie stiften. Damit hat Movement wieder genau einen Skript-Träger
(den Point-Code im `ScriptGridModule`), und die Regel „alle Slots eines Knotens
teilen eine Umgebung" gilt wieder **ausnahmslos** — geprüft über alle Renderer.

**Merkregel:** Movement und Dynamic Movement sind keine Stufen desselben Effekts.
`r_trans` legt eine Tabelle **je Pixel** an (exakt, keine Interpolation, aber
statisch) und bringt die 23 eingebauten Effekte mit; `r_dmove` rechnet ein
**Gitter** je Frame (beweglich, interpoliert, mit Buffer-Zugriff und Alpha).

### ⚪ Wunsch: ein Sample im Creator bearbeiten — Knopf „Edit" neben File... und Record... (Patrik S118; entworfen S119, nicht gebaut)

**Stand S119:** entworfen und entschieden, `audio/Loop_Konzept.md` §4k —
M1 a (Edit... öffnet, was die Schicht spielt; Kit und freie Datei
schreibgeschützt), M2 b (Save überschreibt das eigene, Save as... kopiert),
M2b a (das Namensfeld entfällt, auch nach einer Aufnahme), M3 a (die Rückfrage
nennt die eigenen Instrumente, Sample start der offenen Schicht zieht mit),
M4 a (Edit..., Fenster „Edit sample – <Name>", Record bleibt); die Skizze ist
gebilligt, die Baustufen freigegeben (B2 a). **17a gebaut** (das Fenster:
Save und Save as... statt Namensfeld, ein bestehendes Sample öffnen) — im
Programm zeigt Record... die neuen Knöpfe. **17b gebaut** (der Knopf Edit...
im Creator; Sample start und Schleife rücken mit; über 30 Sekunden öffnet er
nicht). **17c gemacht** (Benutzerhandbuch 1.148.0, Liste 10 Abschnitt D,
voller Lauf 1153 / 1153). **Stufe 17 ist fertig — im Programm ungesehen:**
Liste 10, D0 bis D10, Urteile U72 bis U75. Folge: aus
dem Drum-Editor lässt sich ein vorhandenes eigenes Sample nicht mehr ersetzen,
nur über Edit... im Creator. Was darunter steht, ist die Notiz aus S118.

Im Instrument-Creator, an einer Sample-Schicht, neben **File...** und
**Record...** ein dritter Knopf (Patrik: „Bearbeiten"): er öffnet ein
**bestehendes** Sample im Fenster wie nach einer Aufnahme (Record sample —
zuschneiden, anhören) und speichert es danach. **Mitgelieferte Samples sind
schreibgeschützt wie die mitgelieferten Instrumente:** sie lassen sich nur
unter einem neuen Namen bei den eigenen (My files) speichern. Für später,
nur notiert — nicht entworfen, nicht eingeplant.

Offen, wenn es drankommt: der englische Name des Knopfs; ob ein eigenes
Sample beim Speichern überschrieben wird oder immer eine Kopie entsteht (bei
Instrumenten: Save überschreibt das eigene, Save as... kopiert); was mit
Schichten und Drum-Spuren geschieht, die das bearbeitete Sample schon
spielen; ob auch eine Tondatei an freiem Ort so zu öffnen ist. Heute gibt es
keine mitgelieferten Samples außer den Klängen des Kits (`kit:<name>`) — ob
die gemeint sind, ist zu fragen. Vorbild und Unterbau: `loop/AufnahmeFenster`,
`loop/Sample` (`nimmZuEigenen`), die Regeln aus S117 (E1 a, E1b c).

### 🟡 Vorlagen S123: Formen, Samples, Kits, Loops — dazu ein Versuch „Titanium" und das VST3-Konzept (Wunsch Patrik S123)

Auftrag: mehr Vorlagen für das, was zuletzt entstand — Formen (einzeln und
mehrere), Instrumente daraus, Samples aus freien Quellen (gern mit Stimmen),
Instrumente daraus (auch mehrere Schichten), ein Drumkit aus zusätzlichen
Drums, Loops mit allem; danach ein Konzept für VST3. Entscheide Patrik S123:
**1 a** (mitgelieferte Samples unter `asset/loop/samples`, im Loop
`lib:<name>`, eigene Gruppe *Samples* in den Klanglisten), **2 a** (nur CC0,
CC BY, gemeinfrei, mit Nennung in `samples/CREDITS.md`, die mit neben die Exe
geht), **3 a** (Kits als eigene Einheit, mit Skizze vorher).

**Gebaut (S123), im Programm ungesehen und ungehört:**
- 14 Formen mehr (`asset/contours`, jetzt 20; vier mit mehreren Konturen).
- `lib:` im Code: `istBibliotheksKlang`, `istMitgelieferterKlang`,
  `bibliothekOrdner`, `bibliotheksKlaenge`; die Gruppe *Samples* im Drum-Editor
  und im Sample-Menü des Creators; `Asset.cmake` nimmt `samples/*.wav` und
  `CREDITS.md` mit (die eine Ausnahme von „nur eigene Werke",
  `THIRD_PARTY_NOTICES.md`).
- 38 Samples aus der OLPC-Sammlung (CC BY 3.0), aufbereitet von
  `samples/make_samples.py`.
- 35 Instrumente mehr (90): elf aus Formen, dreizehn aus Samples, elf Drums
  (sechs gerechnet, fünf aus Samples geschichtet).
- 9 Loops mehr unter `genres/` (`make_showcase.py`), vier neue Genres.
- Tests: Hörproben eines Loops als WAV (`LUMIVIZ_LOOP_WAV`); ein Fall zu `lib:`.

**Offen:**
- 🟡 **Kits (3 a) — Stufe 21**, Entscheide K1 a bis K9 a, Skizzen gebilligt
  (`audio/Loop_Konzept.md` §4l, mit dem Inhalt aller vierzehn Kits). **21a und 21b
  gebaut** (Modell, `.lvkit`, Rollen, Kitwechsel, zehn Kits mit gemessenem
  Volume, Bereich Kits in Export und Import; im Drum-Editor der Knopf Kit, das
  Etikett Role, Add track aus dem Kit, Save as kit... — im Programm ungesehen).
  **21c gebaut** (S124): das Fenster Kits (Edit kits...) mit Duplicate, Rename,
  Delete, Sound, Volume, Pan, Add role, Remove role, ▶, Play all, Undo, Save,
  Used by — im Programm ungesehen; von Claude gesetzte Regeln in
  `audio/Loop_Konzept.md` §4l „Wie 21c gebaut ist"; kosmetisch offen: Striche
  der Auswahlfarbe am Rand der Zellen der gewählten Zeile.
  **21d gebaut** (S124, D1 a bis D6 a): 37 Stereo-Samples aus DrumGizmo
  (`make_drumgizmo.py`), drei gerechnete Besen, die Kits Rock'n'Roll,
  Hardrock, Heavy Metal, Jazz Brushes, der Loop Brush Swing; offen Patriks
  Ohr an `hoerproben_21d` (Mischung, Besen — Befund „Hats abgeschnitten":
  die Reihen-Proben waren nach 3 s gekappt; seit D6 a bleiben die Samples
  ganz, nachzuhören). **K10 a, K11 a gebaut** (S124, Befund Patrik am
  Programm: ein Kit auf einem leeren Muster tat nichts): zwei gemerkte Haken
  im Kit-Menü — alle Rollen anlegen, leere fremde Spuren entfernen; das * in
  der Hinweisfarbe für fremde Spuren mit Schritten. Ursprünglicher Plan:
  ~~**21d** Rock'n'Roll, Hardrock, Heavy Metal
  und Jazz Brushes aus DrumGizmo (DRSKit, CrocellKit, CC BY 4.0 — Downloads zur
  Freigabe) mit gerechneten Besen und dem Loop Brush Swing~~. **21e gemacht** (S124): Icon `.lvkit` (I2 a), Handbuch §16 „Kits", Liste
  `Sichttest_Kits.md`, voller Lauf 1215 / 1215 — **Stufe 21 fertig**; offen: Patriks
  Sichttest mit der Liste (K, N, R, F, A, I, U) und sein Ohr an `hoerproben_21d`;
  `register_file_icons.ps1 -Apply` (Patrik). Ursprünglich offen: ~~**21e** Icon
  (I1 a: Trommel mit Sticks), Handbuch, Sichttest~~. Patrik überlegt, das
  Besen-Paket von Ben Burnes zu kaufen — vor dem Einbau die Lizenz im Download
  prüfen (2019 hieß es CC0).
- ⬜ Patriks Ohr an `.claude/handover/hoerproben_s123/` (Instrumente und die
  neun Loops) — besonders die Schleifen der gehaltenen Stimmen (Knacken?),
  Cat und die Lautstärke der Loops.
- ⚪ Kein Männerchor: die drei Männerstimmen halten keinen Ton (gemessen).
- ⚪ **Versuch „Titanium"** (lokal, `.claude/handover/titanium/`, nicht im
  Repo): der Song ohne Gesang als Loop über 121 Takte, mit drei eigenen
  Instrumenten. Gemessen gegen die MP3: Akkorde je Takt Kosinus 0,77 im Mittel
  (Drops 0,9), Verlauf der Energie tief 0,55 / hoch 0,59; über 500 Hz 6 bis
  10 dB zu wenig — dort sitzt die Stimme.
- 🟡 **VST3** — Konzept `audio/VST3_Konzept.md` 0.1.0, Entscheide V1 bis V5
  offen; Lizenz belegt (MIT seit VST 3.8, Pressemitteilung Steinberg
  29.10.2025), die Usage Guidelines zur Marke nicht gelesen.

### ⚪ Idee: Klang aus einer Form — Waveform „Shape" im Instrument-Creator (Patrik S119)

Aus einer geschlossenen Kontur wird über eine abgeschnittene Fourier-Reihe ein
nahtlos loopender Wellenzug; die Zahl der Harmonischen ist der Klangregler.
Grobkonzept ohne Bezug zu LumiViz: `docs/fourier-shape-loops-konzept.md`
(Anstoß: Beiträge mit Epizykel-Animationen — zwei Fourier-Reihen führen den
Stift in x und y).

**Seit S120 auf LumiViz umgeschrieben:** `docs/audio/Shape_Konzept.md` 0.1.0 —
dort die Rechnung, die Regler, die Stufen als Vorschlag und die Liste der
sieben Vorlagen (§13). Drei Befunde aus dem Nachsehen: **(1) die Endung
`.lvshape` ist vergeben** — seit S96 heißen so die Stück-Presets des Curve
Editors im Composer (`composer/FormPresets.hpp`); die Kontur-Datei braucht
eine andere, oder die Presets ziehen um (erste Vorlage). **(2) Die Visuals
bekommen vom Loop beide Seiten getrennt** (`MainWindow.cpp`, Zufuhr über den
Mixer des Loops, 512 Werte je Seite und Bild) — ein Ton unter rund 94 Hz
zeigt bei 48 kHz in einem Bild keinen ganzen Umlauf. **(3) Qt Svg liegt in
der Installation** (6.10.1); ob es Pfade als Geometrie herausgibt, ist nicht
nachgeprüft. Dazu festgehalten: Start und Reverse ändern nur die Phasen der
Teiltöne — an einer Schicht allein hört man sie nicht, wohl hinter Drive,
über einer zweiten Schicht derselben Form und im Scope.

**Entschieden (Z1 a):** in LumiViz, als Waveform einer Schicht im
Instrument-Creator, mit einem eigenen Fenster für die Form; alles in C++, kein
Python-Werkzeug daneben. Der Export für andere Synths ist eine spätere Zugabe.

**Patriks Eingabewege:** mit einfachem Zeichnen; als SVG importiert; aus einem
Bild (jpg, png …), dessen Konturen mit geeigneten Filtern erkannt und dann
ausgewählt werden — verschiedene Konturen, verschiedene Farben.

**Z2 a, im Grundsatz (Patrik):** eine Schicht mit dem Regler **Angle** (dreht
die Form, bevor der Ton abgenommen wird) und dem Haken **Stereo XY** (x links,
y rechts). Dazu zwei Gedanken von Patrik, die er als zwei Ideen trennt:
- **XY auf die Stereokanäle** — das Bild der Form im Ton. Dazu gehört ein
  **Modul für die Effect Chain**, das die Form zeigt.
- **Die Richtungen +x, −x, +y, −y** für sich überlagernde Schichten. Mit Angle
  von 0° bis 360° sind das die Stellungen 0°, 180°, 90°, 270°: −x ist +x mit
  umgekehrtem Vorzeichen — allein gleich im Klang, über einer anderen Schicht
  derselben Form löscht oder verstärkt es.

**Nachgesehen (S119):** eine Schicht hat kein Panorama; ein Ton wird in Stereo
gerechnet, bei Sample-Schichten dürfen die Seiten verschieden sein, alle
gerechneten Schwingungen sind auf beiden Seiten gleich. Die Effekte am Clip
rechnen links und rechts getrennt. Die Effect Chain kennt die Wellenform je
Seite (`getosc` mit Kanal, `getWaveformChannel`), der Superscope hat eine
Vorgabe Lissajous. **Nicht geprüft:** ob die Visuals vom Loop im Lauf beide
Seiten getrennt bekommen.

**Z3 a (Patrik):** Stereo an seinem Ort — im Creator bekommt jede Schicht im
Kasten Level einen Regler **Pan** (für jede Waveform; schließt U19), die
Shape-Schicht dazu den Haken Stereo XY; bewegtes Stereo (Flanger, Chorus)
entsteht bei den Effekten am Clip. Folge: eine neue Fassung der `.lvinst`.

**Regler der Shape-Schicht, soweit besprochen:** **Harmonics** (die Zahl der
Harmonischen — Frage Patrik; der zentrale Klangregler), **Harmonics
envelope**, **Angle** (0° bis 360°), **Stereo XY**; von Claude vorgeschlagen,
nicht entschieden: **Start** (der Startpunkt auf der Kontur) und **Reverse**
(die Umlaufrichtung).

**Z4 a (Patrik):** Harmonics läuft weich — zwischen zwei ganzen Werten wird
die nächste Harmonische eingeblendet, keine Stufen; **Harmonics envelope**
wie Filter envelope: die Hüllkurve der Schicht öffnet zusätzliche Harmonische
(negativ: die Note beginnt als Ellipse und wächst zur Form). Hohe Noten
bekommen von selbst weniger Harmonische, damit nichts über der halben
Abtastrate klingt.

**Z5 a (Patrik):** mehrere gewählte Konturen gehen beide Wege, im Form-Fenster
wählbar, jede Kontur in einer Farbe. **As layers:** je Kontur eine Schicht in
ihrer Farbe, bis das Instrument voll ist (vier). **As one path:** die
gewählten Konturen nacheinander in einem Umlauf, für eine Schicht — nur so
erscheint im Scope eine Zeichnung aus mehreren Teilen (zwei Schichten mit
Stereo XY ergeben eine Kurve, die Punkt für Punkt ihre Summe ist, nicht zwei
Formen).

**Die Form ist eine Datei (Patrik) — seit S120 `.lvcontour` (F1 a; in S119
hieß sie `.lvshape`, die Endung gehört aber seit S96 dem Composer)** — und soll auch von den
Visuals benutzt werden können: das Modul der Effect Chain, das die Form aus
ihren Koeffizienten zeichnet, liest dieselbe Datei. **Start** und **Reverse**
sind Regler der Schicht (Patrik: ja).

**Die Datei ist entschieden (S120, `Shape_Konzept.md` §7):** D1 a eine
Zeichnung mit allen Konturen, D2 a Ort und Nennung wie bei Instrumenten und
Samples (`asset/contours`, `user:<name>`, Bereich Shapes im Export), D3 a das
Instrument nennt die Form über eine feste Kennung der Kontur, D4 b Pfade
aus Strecken und Kurven, ohne Quellbild und ohne Koeffizienten.

**Offen, wird noch geklärt:** die
Konturerkennung aus einem Bild (OpenCV liegt nicht im Repo — selbst zu
schreiben); SVG (das Qt-Modul Svg steht nicht in der `Solution.json`; ob es
in der Installation liegt, ist nicht geprüft); die Brücke zwischen Konturen
bei As one path; das Modul für die Effect Chain (zwei Arten: ein XY-Scope, das
links gegen rechts zeichnet, und eines, das die Form aus ihren Koeffizienten
zeichnet); die Skizze
des Form-Fensters; was Qt für SVG hergibt und wie Konturen ohne OpenCV erkannt
werden; mehrere gewählte Konturen; die Entscheide aus §10 des Grobkonzepts.
Nichts entworfen, nichts gebaut; das Grobkonzept ist seit S120 auf LumiViz
umgeschrieben (`docs/audio/Shape_Konzept.md`), die Vorlagen stehen aus.

**Stand S121 — Stufe 19b gebaut, der Knoten Shape** (`Shape_Konzept.md`
0.13.0, §10). Entschieden (Patrik): V1 a fünf Skriptfelder (Init, Frame, Beat,
Song, Per point), V2 a die Regler mit Contour, Points, eigener Farbe und Line
blend, V3 a neun Presets, V4 a XY Scope bekommt dieselben fünf Felder. Der
Absatz „Offen, wird noch geklärt" darüber ist seit S120 überholt (alle sieben
Vorlagen sind geschlossen). **Offen daraus:**
- ⬜ **Im Programm ungesehen:** der Knoten, sein Panel, die neun Presets, Show
  circles. Gesehen sind nur Bilder aus den Tests. Eine Sichttest-Liste dafür
  gibt es noch nicht.
- ✅ ~~Eine geänderte `.lvcontour` sieht der Knoten erst, wenn die Nennung
  wechselt~~ — S122 mit Stufe 20a: `zeichnungFassung`, der Knoten liest neu.
- 🔧 Für `shape` gibt es keine Feld-Sonden unter
  `asset/calibration/fields/probes` (`make_field_probes.py` nicht gefahren).
- 🔧 Die Zeile Contour trägt einen „~"-Knopf (das Feld ist eine Zahl in der
  Feldtabelle); eine Modulation dort springt zwischen Kennungen. Lassen oder
  ausnehmen.

**Stand S121 — Stufe 19c gebaut, die Schwingung Shape im Klangerzeuger**
(`Shape_Konzept.md` 0.14.0, §5 und §9; `.lvinst` Fassung 9). Entschieden
(Patrik): T1 a der Pegel (kein Ausgleich, die Form am fernsten Punkt
gemessen), T2 a Angle dreht die Form auch mit Stereo XY. **Offen daraus:**
- ⬜ **Patriks Ohr — `.claude/handover/hoerproben_19c`** (36 Dateien, lokal):
  die Formen mit 4, 16 und 64 Teiltönen, Angle, Stereo XY, Harmonics
  envelope, ein Pfad aus zwei Konturen. Nichts davon ist gehört.
- 🟡 **Ecken dämpfen** (§5): fest, als Regler, oder gar nicht — die Proben
  `corners_*_plain` gegen `corners_*_damped`. Der Klangerzeuger dämpft nicht.
- 🟠 **Die Kante des Sprungs** (§9): bei A4 liegen −33 bis −34 dB des Tons
  als zurückgefallener Anteil neben den Teiltönen, gleich bei 16, 48 und 160
  Harmonics. Die Proben `onepath_*`. Stört es: ein längerer Sprung (Regler)
  oder eine Reihe über den ganzen Umlauf.
- ✅ ~~Wird eine Form geschrieben, entstehen schon gerechnete Töne nicht
  neu~~ — S122: das Fenster Edit shape zählt `zaehleInstrumentFassung`. Eine
  Form, die anders als über das Fenster geschrieben wird, zählt weiter nicht.
- 🔧 Die Arten der Sprünge „kürzeste Wege" und „von Hand" spielen wie die
  Reihenfolge der Liste (Stufe 20d).
- Im Creator ist Shape nicht wählbar (Stufe 19e); im Programm ist von 19c
  nichts zu sehen oder zu hören.

**Stand S121 — Stufe 19d gebaut, der Knoten XY Scope** (`Shape_Konzept.md`
0.15.0, §10; Benutzerhandbuch 1.156.0). Entschieden (Patrik): X1 a die
Regler, X2 a sechs Presets; die Reihenfolge der Stufen bleibt. **Offen
daraus:**
- ⬜ **Im Programm ungesehen:** das Scope zu Musik, seine Presets, sein
  Panel. Keine Sichttest-Liste, keine Feld-Sonden für `xyScope`.

**Stand S121 — Stufe 19e gebaut, die Shape-Schicht im Instrument-Creator**
(`Shape_Konzept.md` 0.16.0, §6; Benutzerhandbuch 1.157.0). Damit ist ein
Instrument mit einer Form im Programm spielbar. **Offen daraus:**
- ⬜ **Im Programm ungesehen und ungehört:** Waveform Shape im Creator, die
  Regler, das Bild „Shape, as it plays", der Ton, das XY Scope an einer
  Schicht mit Stereo XY. Gesehen ist ein Bild des Creators aus dem Test.
- ✅ ~~Der Knopf **Edit...** neben Shape fehlt~~ — S122 mit Stufe 20a.
- 🟡 Kein mitgeliefertes Instrument trägt eine Shape-Schicht — eines oder
  zwei dazulegen? (Katalog, Zahlen in den Tests und `loop_werte.txt` sind
  dann nachzustellen.)

**Stand S122 — Stufe 20a gebaut, das Fenster Edit shape** (`Shape_Konzept.md`
0.17.0, §8 und §8a; Benutzerhandbuch 1.158.0). Entschieden (Patrik): **Z1 a**
Pen zeichnet frei Hand und Punkt für Punkt gemischt, Move point schiebt
Ankerpunkte; **Z2 a** der Knopf Draw entfällt. Dazu im Creator **Edit...**
neben Shape und **New shape...** im Menü Browse... (Patriks Frage S122).
**Offen daraus:**
- ⬜ **Im Programm ungesehen:** das Fenster, Zeichnen mit der Maus, Move
  point, Play, Edit... und New shape... im Creator, ein Knoten Shape, der
  nach Save neu zeichnet. Gesehen ist ein Bild des Fensters aus dem Test.
  Keine Sichttest-Liste.
- 🟡 Von Claude gesetzt, im Sichttest zu prüfen: wie stark geglättet wird (2
  Bildpunkte), ab wann ein Ziehen ein Zug ist (4), der Fangkreis (8), die
  feste Ansicht beim Öffnen (wandert nicht mit), Undo mit hundert Schritten,
  Play bei A3 mit Pegel 0,4.
- 🔧 Nicht gebaut: den Ausschnitt verschieben oder vergrößern, Punkte
  einfügen oder löschen, Griffe einzeln ziehen.

**Stand S122 — Stufe 20b gebaut, SVG** (`Shape_Konzept.md` 0.18.0, §8;
Benutzerhandbuch 1.159.0). Entschieden (Patrik): **O1 a** Replace, Add oder
Cancel, wenn schon Konturen da sind (gilt auch für 20c); **O2 a** die 64
längsten, Haken für alle bis acht, sonst nur die längste; **O3 a** eine
Farbe, die alle teilen, wird durch die Farben der Reihe ersetzt. **Offen daraus:**
- ⬜ **Im Programm ungesehen:** Open SVG... mit Patriks Proben.
- 🟡 **Farben über CSS-Variablen** (`var(--…)`) kommen nicht an: die sechs
  `-raw`-Dateien in `asset/shapes/2` liefern keine Kontur (die aufgelösten
  daneben schon). Auflösen hieße die Variablen im Text selbst einsetzen —
  vorzulegen, wenn es gebraucht wird.
- ✅ ~~Die Farben aus potrace sind schwarz~~ — O3 a (Patrik S122): eine Farbe,
  die alle Konturen teilen, wird durch die Farben der Reihe ersetzt.
- ✅ **Farben für Schichten** — entschieden und gebaut mit 20d (F1 a, V1 a).
  Vorher: Schichten hatten keine Farbe; §9 sieht für As layers „je Kontur eine Schicht in ihrer Farbe" vor.
  Ob Schichten generell Farben bekommen und wie sie mit den Konturen
  übereinstimmen — mit Stufe 20d vorzulegen (Patrik S122: ja, mit 20d).
  **Patriks Idee (S122):** je Waveform eine eigene Palette in Abstufungen —
  Sine Blautöne, Saw Rottöne usw. —, für Shape eine breitere. Für die
  Vorlage zu prüfen: Schichten derselben Art bekommen die nächste Abstufung
  (Platz in der Art, nicht im Instrument); die breite Shape-Palette darf die
  Familien der anderen Arten nicht treffen (heute ist die erste Farbe der
  Konturen Cyan, nah an Blau); wie viele Abstufungen je Familie bei acht
  Schichten unterscheidbar bleiben.

**Stand S122 — Stufe 20c gebaut, Konturen aus einem Bild** (`Shape_Konzept.md`
0.19.0, §8; Benutzerhandbuch 1.160.0). Entschieden (Patrik): **R1 a** die
Regler ersetzen nur die Konturen aus dem Bild; b als Möglichkeit über den
Haken **Apply live** im Fenster (**R2 a**). **Offen daraus:**
- ⬜ **Im Programm ungesehen:** Open image..., die Regler, Apply live und Take
  contours, Show image — an echten Fotos und Logos.
- 🟡 Von Claude gesetzt, im Sichttest zu prüfen: die Vorgaben (Brightness,
  Levels 3, Smooth 1, Min size 0,2 %, Detail 50 %), Detail von 0,75 bis 5
  Punkte, die Pause der Regler (150 ms), das Bild mit 35 % Deckung,
  Durchsichtiges zählt als Weiß, die Startmitten der Farbteilung.

**Stand S122 — Stufe 20d gebaut, mehrere Konturen; damit ist Stufe 20
vollständig** (`Shape_Konzept.md` 0.20.0, §6, §8a, §9; Benutzerhandbuch
1.161.0). Entschieden (Patrik): **L1 a** Save mit Layers legt Kopien der
offenen Schicht an; **F1 a** Farben der Schichten nach Patriks Idee
(Familien je Waveform, Shape in der Farbe ihrer Kontur, eigene Farbe auf
Wunsch); **V1 a** dafür keine neue Fassung der `.lvinst`. **Offen daraus:**
- ⬜ **Im Programm ungesehen und ungehört:** Use as, Jumps, Bridge, Shortest
  und Drawn by hand im Ton (eine Kontur in zwei Stücken), Save mit Layers, die
  Farbpunkte. Keine Sichttest-Liste.
- 🟡 Von Claude gesetzt, im Sichttest zu prüfen: die Abstufungen der
  Familien; Shortest als Näherung (sechs Runden, 2-opt); der Anfang bei
  Drawn by hand an der ersten Brücke; wer eine Brücke setzt, stellt One path
  und Drawn by hand; ein Klick auf die Mitte einer Brücke entfernt sie; im
  Creator behält die offene Schicht ihre Kontur, solange sie einen Haken hat.
- 🟡 Im Bild aus dem Test erschien „One path" ungefüllt, obwohl gesetzt — wohl
  die Übergangsanimation des Stils beim sofortigen Abfotografieren; im
  Programm nachsehen.

### ✅ Wunsch: Bars kleiner als 4 im Drum- und Instrument-Editor (Patrik S123) — gebaut S124

**Gebaut (S124, Entscheide B1 a, B2 a, B3 a, B4 b; `audio/Loop_Konzept.md` §4m):**
Muster ab 1 Takt, neue weiter 4; der Haken Repeat when lengthening; die Kopie
trägt den Zeitraum des Rahmens; dazu (Wunsch Patrik S124) ein Klick in den
Zeitstrahl springt — im Lineal des Panels und mit einer eigenen Abspielstelle
in den Editoren (die Einfügemarke mit Shift+Klick). `.lvloop` Fassung 6.
Dazu E1 a (Wunsch Patrik S124): der Schalter der Effekte je Clip — FX in den
Editoren, Effects on im Fenster Clip effects, die matte Marke fx; Play pattern
spielt jetzt mit den Effekten des Bezugsclips. `.lvloop` Fassung 7.
Im Programm ungesehen. Der Befund unten bleibt als Vorgeschichte stehen.


„Zumindest für das Austesten würde ein kleinerer Loop helfen, dass man Bar 1
komponiert und erst nachher die anderen Bars auffüllt." Heute geht das Feld
Bars nicht unter 4. **Befund (nachgesehen, nichts geändert):** `kTakteMin` = 4
(`loop/Loop.hpp`, „Vorgabe Patrik S108") ist zugleich die **Untergrenze** (die
Felder Bars beider Editoren, `setzeMusterTakte`, das Richten beim Laden) und
die **Länge eines neuen Clips** (`Bearbeiten.cpp`, `Taktachse.cpp`,
`LoopPanel.cpp`). Für den Wunsch wären die beiden zu trennen (etwa Untergrenze
1, neue Clips weiter 4). Offen, vorzulegen: ob die Untergrenze 1 oder 2 ist; was
ein Clip eines kurzen Musters auf der Achse zeigt; ob „auffüllen" nur das
Verlängern meint oder auch ein Kopieren von Bar 1 in die neuen Takte. Nicht
gebaut — nach Stufe 21.

**Dazu (Befund Patrik S124, mit diesem Punkt zu bauen — Wunsch Patrik):**
Kopieren und Einfügen hat einen falschen Versatz, wenn der erste oder letzte
markierte Schritt leer ist: eingefügt wird ab dem ersten **belegten** Schritt,
und die Einfügemarke rückt nur bis zum letzten belegten weiter — statt den
markierten Bereich (die markierten Takte) einzufügen. **Im Code nachgesehen
(nichts geändert):** im Instrument-Editor markiert man nur Noten
(`NotenRaster`), `kopiereNoten` misst von der ersten Note bis zum Ende der
letzten — leere Zeit davor und danach fällt weg, immer. Im Drum-Editor nehmen
Rahmen und Ctrl+A die leeren Schritte mit (`rahmeBis`, `markiereAlle`), dort
stimmt der Versatz; mit Ctrl+Klick zählen nur die angeklickten (Entscheid
V2 a, S113) — dann derselbe Fehler. Kern: eine Kopie kennt keinen markierten
Zeitraum. Vorzulegen mit dem Punkt oben: ob die Kopie den Zeitraum des
Rahmens trägt oder auf ganze Takte gerundet wird, und ob V2 a dafür neu
gefasst wird.

### 🟡 Publish — etwas mit allem, was dazugehört, als Paket weitergeben (Patrik S120)

**S125: entschieden, nichts gebaut.** Steuerdokument
[core-services/Publish_Konzept.md](core-services/Publish_Konzept.md) 0.1.0 —
Entscheide Patrik P1 a bis P8 a (alle Arten, zuerst die Loop-Familie;
Publish... hinter Save as an sieben Orten, Publish a file... und Import
package... im Menü File; Eigenes und freie Orte gehen mit und werden
`user:`; Medien per Haken, anfangs aus; `.lvdata` Fassung 2; gleiche Namen
mit Vergleich, Vorgabe Keep both als `<name>_2`; Dialoge Publish und Import
package), drei Skizzen als Beschreibung. **Stufe P-A gebaut (S125):** Sammler,
Packen, Import, die Dialoge, Publish... an fünf Orten, File → Publish a file...
und Import package... (Konzept §5a–§5e, Handbuch §16 „Weitergeben: Publish“);
im Programm ungesehen — offen: Patriks Sichttest, dann Stufe P-B (Komposition,
Effect chain, Medien). Nebenbefund Keep both: `bass (2).lvinst` übergeht das
Programm — mit einem Test belegt und behoben (S125, `<name>_2`).

*Darunter die Vorgeschichte (S120, S123):*

**Patrik S123:** angehen, **nachdem die laufenden Arbeiten abgeschlossen
sind** (Kits, Vorlagen S123) — nicht vorher. Anlass: der Versuch „Titanium"
(`.claude/handover/titanium/`) — der Loop nennt drei Instrumente als Datei
neben sich; wer ihn allein weitergibt oder verschiebt, verliert sie. Ein Paket
„als Package speichern / aus Package laden" löst genau das. Export und Import
aller eigenen Daten (`.lvdata`, S114) gibt es schon und ist der Unterbau.

Anlass: der Entscheid D3 a zur Form (`Shape_Konzept.md` §7) — ein Instrument
**nennt** seine Form, wie es heute schon seine Samples nennt. Eine `.lvinst`
allein ist deshalb nicht vollständig; wer sie weitergibt, gibt den Klang
nicht mit.

**Patriks Lösung, für diesen Fall und andere:** **Publish** archiviert ein
Ding mit **allen zugehörigen Dateien** und der **Ordnerstruktur des
Benutzerordners** in ein Paket, mit einem **Manifest**. Das Paket heißt
`<name>.<type>.lvdata`, wobei `<type>` die Art nennt — etwa
`mein_bass.lvinst.lvdata`. Der richtige Import ergibt sich aus Ordnerstruktur
und Manifest.

**Nachgesehen (S120):** das passt auf das, was es gibt. Eine `.lvdata` ist
heute ein Zip mit `manifest.json` (Format, Fassung, LumiViz-Version, Datum,
Zahlen) und `data/<Pfad im Benutzerordner>` (`core/Datensicherung.hpp`); der
Import schreibt nur in die Ordner bekannter Bereiche. Ein Paket wäre
dieselbe Datei mit einer engeren Auswahl — statt ganzer Bereiche die Dateien,
an denen ein Ding hängt.

**Gleiche Namen beim Import (Patrik S120, zweite Runde):** es gibt eine
Option **Vergleichen**, an der man entscheidet: **überschreiben**,
**umbenennen** oder **nicht übernehmen**. Beim **Umbenennen** werden die
**Nennungen im Paket automatisch mitgeändert** — das Instrument aus dem Paket
spielt danach die umbenannte, mitgebrachte Datei. Seine Frage, ob es das
schon gibt: ja, als Muster an zwei Stellen — der **Host Switcher** (S82):
wird eine Host-Gruppe umbenannt, führt jeder Switcher, der sie listet, den
neuen Namen (`hostGruppeUmbenennenInSwitchern`, `EffectChain.hpp`, mit Test
„Umbenennen zieht jede Switcher-Liste mit"); und im **Loop**: Rename base gilt
für alle Clips, die die Basis spielen (`loop/Bearbeiten.hpp`). Der Import der
`.lvdata` tut es heute **nicht**: er gibt der Datei nur eine Zahl an den Namen
(`Datensicherung.cpp`, `nameMitZahl`). Die drei Wege selbst gibt es dort schon
als Keep mine, Replace mine, Keep both und je Datei; ein Vergleichen des
Inhalts nicht — die Liste zeigt Datum und Größe.

**Offen, nichts entworfen** (S120; seit S125 entschieden, siehe oben):
- **Welche Arten** sich veröffentlichen lassen (Instrument, Loop,
  Komposition, Preset …) und wo der Befehl steht.
- **Was dazugehört:** ein Instrument nennt Samples und Formen; ein Loop
  nennt Instrumente und Samples; Mitgeliefertes geht nicht mit. Was mit einer
  Datei an freiem Ort geschieht (sie liegt nicht im Benutzerordner).
- **Gleiche Namen beim Import:** „Keep both" gibt der importierten Datei eine
  Zahl an den Namen (`nameMitZahl`: `Neon (2).json`) — das Ding im Paket
  nennt seine Zugehörigen aber beim alten Namen. Ohne ein Umschreiben der
  Nennungen spielte es dann die eigene Datei, nicht die mitgebrachte. Dazu,
  **aus den Köpfen gelesen und nicht geprüft:** ein eigenes Instrument oder
  Sample darf nur `a`–`z`, Ziffern und `_` im Namen tragen (`istEigenerName`);
  eine Datei `bass (2).lvinst` könnte ein Muster dann gar nicht nennen. Ob
  das heute beim Import ganzer Bereiche schon so ist, wäre mit einem Test zu
  klären.
- **Manifest:** ob ein Paket dieselbe Fassung der `.lvdata` bleibt (dann
  liest es auch der heutige Import) oder eine neue mit den Feldern Art und
  Hauptdatei.
- Ob der Dialog Import data ein Paket anders zeigt als einen Export.

### ⚪ Idee: Flanger als Effekt am Clip (Patrik S119)

Ein weiterer Baustein für die Zeile Signal path im Fenster Clip effects, wie
Patrik ihn vom Gitarren-Effektgerät (Korg) kennt: der Ton wird mit einer sehr
kurz und schwankend verzögerten Kopie seiner selbst gemischt. Verwandt:
Chorus, Phaser. Er macht aus einem Ton in der Mitte etwas Breites, wenn die
Schwankung links und rechts versetzt läuft — im Zusammenhang der Frage
genannt, wo Stereo entstehen soll. Nur notiert.

## 8. Werkzeug- und Doku-Schulden

- 🟠 **Settings: der Reiter „Panels" ist ein Sammelbecken — bereinigen (Befund
  S95, Patrik: eigener Punkt).** Anlass: **Movement Table** steht in der App im
  Reiter **Panels**; Handbuch (§11 „bisher"-Schalter und Changelog 1.79.0),
  `Changelog_Session94.md`, dieser Datei (§7) und die Sichttest-Liste S94b
  nennen „Settings → Performance". Patrik: in Performance ergibt der Schalter
  mehr Sinn — aber dann sind auch die anderen Zeilen des Reiters zu prüfen.
  Heute stehen dort (`SettingsPanel::createPanelsTab`): Effect Browser (Reset
  Start Folder), AVS Import Render Scale, Movement Table, Open Composition,
  End of a Composition, MilkDrop Preset Switch / Fade Amount / Fade-out Time /
  Start Fade-in, Image Search Folder, User Data. Mit einem Panel hat davon nur
  die erste Zeile zu tun. **Zu tun:** eine Einteilung vorschlagen (etwa
  Performance · Import · MilkDrop · Composer · Folders) und Patrik vorlegen;
  danach umziehen und die Doku-Stellen nachziehen. Die beiden Composer-Zeilen
  sind Naht N9 (`Composer_Konzept.md` §12.2) — ziehen sie um, die Naht-Zeile
  dort anpassen. **Bis dahin stimmt die Doku zu Movement Table nicht.**

- 🟠 **Vergleichslauf: ein Preset startet gelegentlich doppelt (Befund S88).**
  In einem von fünf Läufen der Regressionsliste wich `3ds.lvfx` ab (6 Bilder,
  MAE 0,009–0,023). Das Log dieses Laufs zeigt die Zeile
  `=== Preset 1/1: 3ds.lvfx ===` **zweimal** — der Standalone hat das Preset
  nach dem ersten Bild noch einmal von vorn begonnen; drei Wiederholungen mit
  demselben Build waren untereinander und gegen die Basis bitgleich. Ursache
  offen (Kandidat: ein Fenster-/Bildschirm-Ereignis startet den Auto-Lauf
  neu). **Vermutlich dieselbe Ursache wie die einmalige Abweichung bei
  `fUk - afterburner remix` aus S86** (1 von 12 Läufen, „schon im ersten
  Frame"). Merke: bei einer Abweichung zuerst `stdout.txt` des Laufs auf die
  doppelte Preset-Zeile prüfen, dann erst den Code verdächtigen.
  **Zweite Spur (S88):** in zwei späteren Läufen lag im Ordner `list` eine
  zusätzliche Datei `list_lvfx2_manuell.png` — ein von Hand ausgelöster
  Screenshot. Die Messfenster sind sichtbar und nehmen Eingaben an; ein
  Tastendruck oder Klick, der eigentlich einem anderen Fenster galt, kann
  dort landen. Das würde auch den Neustart des Presets erklären. Nicht
  geprüft; Gegenmittel wäre, dass der Auto-Lauf Eingaben ignoriert.
- ✅ **Resume playback (Wunsch Patrik S88):** `audio/Wiederaufnahme.hpp` —
  beim geordneten Beenden werden Datei, Playlist-Index, Position und „lief
  gerade" festgehalten (`player/resume…`), beim Start nach dem
  Wiederherstellen der Playlist aufgenommen (über die Playlist, sonst direkt
  laden; dann Sprung). Schalter `player/resumePlayback` (Vorgabe aus) in
  Settings → Audio und als Taste im Player-Panel. Ablauf mit Player-Attrappe
  getestet. **Nicht geprüft / Sichttest Patrik:** mit echter Wiedergabe —
  ob beim Beenden der Player noch „spielt" meldet, ob der Sprung nach dem
  Start sitzt, ob Songtext und Visual sauber anlaufen.
- ✅ **Laufzeiten entfernter Knoten (offen seit S87, erledigt S88):** seit
  Knoten-Ids nie wiederverwendet werden, blieb der Laufzeitblock eines im
  Panel entfernten Knotens (Texturen, Skript-Hosts, Milkdrop-Kern) bis zum
  nächsten Laden liegen, und ein laufender Video-/Kamera-Feed lief weiter.
  Jetzt räumt der Render-Thread nach jedem Panel-Edit ab
  (`raeumeVerwaisteLaufzeiten`, dazu der Modulations-Zustand; Listen-,
  Gruppen- und Switcher-Laufzeiten räumte das Frame-Ende schon immer ab); der Feed gehört dem Knoten allein (Schlüssel = Id),
  andere Knoten auf derselben Kamera und die Camera-Clip-Aufnahme der
  Einstellungen bleiben. Vergleichslauf Regressionsliste 84/84 bitgleich
  gegen S86. **Nicht geprüft:** der Feed-Stopp mit echter Kamera/Datei
  (**Sichttest Patrik:** Video-Source-Knoten mit Kamera löschen → die
  Kamera-Leuchte geht aus); ein per Undo zurückgeholter Knoten beginnt mit
  frischem Zustand.

- ✅ **Oberflächentexte englisch — erledigt** (die Feld-Tooltips und der
  Dateiname der Kamera-Clips folgten in S84, s. §7 „Umstellung auf Englisch
  fortsetzen“; offen dort nur die Handbuch-Screenshots; bereinigt S129) (S83,
  Entscheid Patrik a). **Umgesetzt:** ~220 `tr()`-Texte + ~150 sichtbare
  Nicht-`tr()`-Texte (ⓘ-Referenzseiten, Import-Meldungen ISF/Shadertoy,
  Groß-Editor-Dialog, Tastenkürzel-Namen, Palette) englisch; „Testaufnahme"
  → **Camera Clip**, Reiter „Kamera" → „Camera". **Wächter:**
  `test_NodePresetStore` „Oberflaechentexte sind englisch" prüft alle `tr()`-
  Texte und die HTML-Referenzseiten unter `src/UI` + `include/UI` gegen
  `deutsche_woerter.txt` (Kommentare und Log-Meldungen ausgenommen;
  Gegenprobe S83 bestanden). Nebenfund: die `row()`-Lambda der vier
  Shader-Referenzen las ihre Texte per `QLatin1String` — Nicht-ASCII (auch die
  früheren Umlaute) erschien dort als Zeichensalat; mit englischem ASCII-Text
  erledigt. **Offen:**
  - **Feld-Tooltips** (`FieldDocs.cpp`, 383 von 803 deutsch, 227 verschiedene
    Texte) — erzeugt aus den Doxygen-Kommentaren in `EffectChain.hpp` durch
    `asset/calibration/fields/harvest_field_docs.py`; der Ernter liest daraus
    auch die Skriptvariablen für die Feld-Sonden (Muster „Parameter-Skript
    (Strang D): …"). **Entscheid Patrik S83: 1a** — Kommentare englisch +
    Ernter anpassen; Umsetzung S84 (§7).
  - Dateiname der Kamera-Clips `testaufnahme_<Datum>.mp4` (sichtbar in der
    Clip-Liste). **Entscheid Patrik S83: 2a** — neue Clips `camera_clip_…`,
    vorhandene bleiben; Umsetzung S84 (§7).
  - Log-Meldungen bleiben deutsch (keine Oberfläche).

  *Bestandsaufnahme (Stand vor der Umsetzung):* **mindestens 239** `tr()`-Texte waren deutsch (Suche über
  Umlaute + `deutsche_woerter.txt` + UI-Wörter — eine **Untergrenze**, reine
  Fachwörter wie „Echo-Zoom", „Aussen-Groesse" rutschen durch). Verteilung:

  | Bereich | Texte | Beispiel |
  |---|---|---|
  | Ketten-Editor `MultiEffectPanel.cpp` | 196 | — |
  | · Milkdrop *Parameter* (Sektion 5, inkl. Gestaltung) | 31 | „Startwerte des Presets …", „Stärke (px/Frame)" |
  | · Video Source | 25 | „Quelle", „Einpassung", „Kamera freigeben" |
  | · Host-Gruppe / Host Switcher | 17 + 6 | Hinweistexte, Wechsel-Modi |
  | · Milkdrop-Knoten (Kopf, Sektionen 0–4) | 17 + 33 | „(aus)", „Keine Custom-Shapes im Preset" |
  | · ISF-Filter / Shadertoy | 16 + 14 | „ISF-Datei importieren…" |
  | · Baum-Beschriftungen, Dialoge, Rest | ~37 | „Waves (%1 Custom aktiv)", „Presetverweis auf …" |
  | Einstellungen (Tab Kamera) `SettingsPanel.cpp` | 19 | „Testaufnahme starten", „Geräte neu suchen" |
  | Shader-Editor-Dialog `EelScriptEditing.hpp` | 12 | „Übernommen — keine Fehler.", Import-Warnungen |
  | Shadertoy-Browser `ShadertoyBrowserPanel.cpp` | 11 | „Suche läuft …" |
  | Parameter-Baum `ParameterBaum.hpp` | 1 | „Farbe wählen" |

  Mitzunehmen: **„Testaufnahme" → „Camera Clip"** (Knoten + Einstellungen +
  Handbuch — Patrik S83: der Video-Source-Knoten ist kein Test-Werkzeug).
  Danach denkbar: der Wächter aus `test_NodePresetStore` auch über die
  `tr()`-Texte (dieselbe Wortliste). Werkzeug der Zählung:
  Scratch-Skript S83, Methode im Session-Report.

- 🟡 **Milkdrop-Variablentabelle unvollständig** (Befund S83, Bestandsaufnahme):
  `ScriptVariables.cpp` listet für den Milkdrop-Node nur einen Teil dessen, was
  `MilkdropVisualizer.cpp` (per_frame-Bindung, etwa Z. 1050–1125) wirklich
  bindet — es fehlen u. a. `echo_*`, `ob_*`/`ib_*`, `mv_*`,
  `wave_mode/usedots/thick/additive/brighten`, `darken_center`, `wrap`,
  `invert`, `brighten`, `darken`, `solarize`, `blur*_min/max`,
  `blur1_edge_darken`, `zoomexp`, `wave_mystery`, `drift_*`, `weich`. Alle
  Milkdrop-Zeilen stehen auf `probe=false` — der Bindungstest deckt sie nicht.
  Gehört zu Block F (§6).
- ✅ **ISF-Editoren zeigten die Pixel-Filter-Referenz** (`uTex`, `farbe()`) —
  behoben S83, eigene ISF-Seite (Sprache, Makros, LumiViz-Extras, `.vs`).

- 🟠 **Die AVS-Befundlage hängt stark am Prüfsignal** (Messung S74). Alle 10
  mitgelieferten Presets, 120 Frames, 320×240, **`--beat-period 30` in beiden
  Läufen** — der Beat ist damit konstant, es unterscheidet sich nur das
  Prüfsignal:

  | Preset | klassisch MAE / dMaxLuma | musik MAE / dMaxLuma | |
  |---|---|---|---|
  | `10_the ring` | 0,113 / 0,262 | **0,264 / 0,209** | **verdreifacht** (dMean 0,064 → 0,354) |
  | `02_color extasy` | 0,119 / 0,092 | 0,091 / 0,128 | bleibt |
  | `07_movin wall` | 0,073 / 0,000 | 0,055 / 0,000 | bleibt |
  | `09_rotating_things` | 0,010 / 0,002 | 0,008 / 0,002 | bleibt (klein) |
  | `02_flowers` | 0,015 / **0,150** | 0,005 / 0,013 | **verschwindet** |
  | `01_fractal Dreams` | 0,012 / 0,004 (OK) | 0,004 / **0,104** | **neu** |
  | 4 weitere | OK | OK | — |

  Drei Klassen, die getrennt gehören:

  1. **Signal-unabhängig** (`02_color extasy`, `07_movin wall`,
     `09_rotating_things`) — in beiden Mustern gleich groß. Das sind die
     belastbarsten Befunde; hier lohnt die Bisektion zuerst.
  2. **Skaliert mit der Signalstärke** (`10_the ring`) — MAE mehr als
     verdoppelt, dMean fünffach. Das passt zur Vermutung im Kalibrier-Plan
     (Clipping/Additiv-Blending) und ist jetzt der **größte** Befund
     überhaupt. Das kräftigere Signal hat ihn erst sichtbar gemacht.
  3. **Signal-abhängig in beide Richtungen** (`02_flowers` weg,
     `01_fractal Dreams` neu) — beides bleibt ein echter Unterschied
     zwischen den Renderern, nur regt ihn jeweils das andere Signal an. Ein
     Befund verschwindet nicht dadurch, dass man ihn nicht mehr anregt; die
     Information ist, **welche** Eingabe ihn auslöst, und das grenzt den
     Knoten ein (Spektrum-abhängig statt wellenform-abhängig).

  **Folge für Aufgabe 4:** nicht mehr fünf Befunde, sondern sechs — und mit
  einer Reihenfolge, die vorher nicht sichtbar war. Noch offen: dieselbe
  Messung bei 1024×768 (Merkregel S47, zwei Größen).
- 🔧 **`MilkdropRef` liess sich seit S67 nicht mehr voll neu bauen** (gefunden
  und behoben S74). `patched/ref_msgbox.h` wird per `/FI` in ALLE
  Übersetzungseinheiten gezwungen — auch in die neun `.c`-Dateien des
  ns-eel2-Kerns — und zog mit `<cstdio>` einen C++-Header sowie namenlose
  Parameter hinein (`STL1003`, dann `C2055`). Solange nur die eigene `.cpp`
  neu übersetzt wurde, fiel nichts auf; der erste Vollaufbau danach stand.
  Behoben mit `<stdio.h>`/`<wchar.h>` und benannten Parametern. **Merkregel:
  ein `/FI`-Kopf muss in JEDER Sprache des Projekts übersetzen.**

- 🟠 **`PanelManager::restoreState()` wird nirgends aufgerufen** (Befund S73). Die
  Kette Panel → `restoreState()` ist gebaut, aber tot: `PanelManager::restoreState()`
  hat im ganzen Programm **keinen einzigen Aufrufer**. Damit laufen auch
  `PanelBase::restoreState()`, `PlaylistPanel::restoreState()` und
  `ImportBrowserPanel::restoreState()` nie. Das Gegenstück `saveState()` **wird**
  gerufen — aus `DockManager.cpp:211`, mit dem Kommentar „nothing else calls
  PanelManager::saveState()". Aufgefallen, weil die Preset-Wiederherstellung (S73)
  zuerst dort aufgehängt war und stumm nichts tat; sie sitzt jetzt im Konstruktor.
  Zu tun: entweder den Aufruf ergänzen (dann prüfen, was die drei Überschreibungen
  beim Start plötzlich tun) oder die tote Kette entfernen. **Nicht einfach den
  Aufruf nachziehen, ohne die Folgen zu prüfen** — `refresh()` und
  Zustands-Wiederherstellung laufen dann doppelt.
- 🔧 **`asset/calibration/fields/probes/avi/resolvedPath.lvfx` ist maschinengebunden**
  (bewusst, S73). Die Sonde setzt **nur** `resolvedPath` — das Feld *ist* der
  absolute Pfad, ein relativer Wert wäre dort wirkungslos, und
  `resolveAviPaths` fasst einen gesetzten `resolvedPath` absichtlich nicht an.
  Als einzige getrackte Datei enthält sie damit einen lokalen Pfad. Vor Gebrauch
  auf einem anderen Rechner neu erzeugen (`make_field_probes.py`). Begründung
  steht im Generator.

- ✅ **Die eingefrorenen `.lvfx`-Zwillinge stehen auf 27/67** — erledigt S59
  („Zwillinge durchgesehen + refreezed 67/67“, Changelog 1.14.0; S62 wieder
  67/67; bereinigt S129) (vorgefunden S58, nicht
  von dort verursacht). `freeze_lvfx_twins.py --verify` meldet durchweg dieselbe
  Abweichung: die Strang-D-Felder `initCode`/`frameCode`/`beatCode` sind seit dem
  Parameter-Skript-Ausbau in jedem Knoten dazugekommen, die Zwillinge aber nie
  nachgezogen worden. Der Prüfstand ist damit blind. Zu tun: einmal durchsehen,
  ob wirklich nur diese Felder dazukamen, dann `--refreeze`. **Kein Zwilling
  enthält Picture II** — der Parser-Fix aus S58 ist nicht beteiligt.
- ✅ **Der SuperScope-Randspalten-Fix hat keinen Wächter** — erledigt S59
  („Randspalten-Wächter-Sonde (89.)“, Changelog 1.14.0; bereinigt S129) (S58). Eine einzelne
  Bildspalte liegt unter jeder Schwelle der Modul-Sonden-Metrik (Menge 0 · Lage
  1 px · MAE 0,003). Ein Versuch, sie über ein Movement als Verstärker sichtbar zu
  machen, ist gescheitert: die Sammelstelle des Movements liegt selbst auf einer
  Klemm-/Wrap-Grenze, die Sonde maß am Ende das Movement und ist wieder raus.
  Beleg ist derzeit nur die Spaltenmessung im Report S58.
- ✅ **Timescope maß 1/320 seiner Wirkung** (S55 gesehen, S56 behoben). Alle acht
  Felder lagen unter der SCHWACH-Schwelle (MAE 0,0004–0,0009). Der Knoten zeichnet
  eine **ein Pixel breite Spalte je Frame** und schiebt sie um eins weiter — der
  Untergrund malte sie im Folgeframe wieder zu, im Schlussbild stand genau **eine**
  Spalte. Dieselbe Familie wie die Rückkopplungs-Sonden in §1: *ein Untergrund, der
  jeden Frame neu zeichnet, kann nichts sehen, was sich über Frames aufbaut.* Fix:
  neue Tabelle `UNTERGRUND_JE_TYP` in `make_field_probes.py` — für Timescope löscht
  der Untergrund **nur im ersten Frame**, die Spalten sammeln sich über die Lauf-
  länge. **7 SCHWACH → 7 WIRKT** (MAE 0,0009 → 0,127).
  Dazu war `timescope.blend` **STUMM aus einem zweiten Grund**: die Vorgabe `3`
  heißt „folge dem Set Render Mode", und dessen Vorgabe ist `0` = Ersetzen — der
  abgeleitete Gegenwert 0 war Pixel für Pixel dieselbe Betriebsart. Gemessen:
  blend 0 → 0,0000 · 1 → 0,0249 · 2 → 0,1815. HANDWERK-Eintrag auf **2** (50/50),
  weil das als einziges auch dann nicht mit der Vorgabe zusammenfällt, wenn später
  ein Set Render Mode mit anderem `lineBlend` in den Untergrund gerät.
  **Timescope jetzt 8/8 WIRKT**, Edit-Sonden 8/8 TEILWEISE (die gesammelten
  Spalten sind Vorgeschichte — genau das erwartete Urteil).

- ✅ **AvsRef deterministisch machen** (S22) — **war seit S52 erledigt und stand
  hier als Doku-Leiche (bemerkt S60):** `avsref_main.cpp:430` setzt
  `srand(kRandSeed)` exakt an der geforderten Stelle (NACH `__LoadPreset`,
  damit es jedes `load_config`-`srand(time(0))` überschreibt). Die
  ~0,06-Rauschgrenze war schon seit S58 (`initbuiltinape`) faktisch weg.
- 🔧 **`bisect_avs.py` Pfad-Modus** rekonstruiert nicht verlustfrei (dieselbe
  Konstruktion: Referenz einmal 240, einmal 4 Pixel) — bis dahin nur die
  Top-Level-Leiter nutzen.
- 🔴 **Wächter-Lücken in der Modul-Matrix**: „Effect-List mit Extended-Config + EEL"
  und „Scope liest `reg` aus einem anderen Knoten" — beide Konstruktionen haben in
  S50 je einen **Totalausfall** verursacht und hatten keinen Wächter. Ebenso eine
  APE-Zeile (Convolution, Texer II, Video Delay, Multiplier, Picture II,
  Channel Shift, AddBorders, Multifilter). Die Bauer stehen seit S50 in
  `avs_preset_lib.py`.
- 🔴 **D2-Kollisionsregel hat keinen Wächter**: Matrix und Sonden enthalten keine
  kollidierenden Namen und können eine Regression strukturell nicht sehen. Nur ein
  Sweep über echte Presets bewacht sie.
- 🔧 **Produkt-Changelog Session 45 fehlt** in [sessions/](sessions/) — die Reihe
  läuft 43, 44, **46**, 47 … Der Session-Report existiert lokal
  (`.claude/sessions/LumiViz_Session45_…`), nur der Changelog wurde nie
  geschrieben. Nachziehen oder die Lücke bewusst vermerken.
- ✅ **Knoten-Parameter-Ausbau** (Vorgabe Patrik, S53) — Steuerdokument
  [Knoten_Parameter_Konzept.md](visuals/Knoten_Parameter_Konzept.md), **alle sechs
  Etappen umgesetzt**; der Sichttest im Betrieb läuft seit S54 maschinell
  (s. Strang E unten).
  Vier Stränge: (A) ✅
  Voreinstellungen je Knoten, generisch über `nodeToJson`/`nodeFromJson` — greift
  für **alle 85 Knotentypen** (gemessen S54 über `std::variant_size_v`; die
  früher notierten 81 waren zu niedrig) (`NodePresetStore` + Zeile im Panel +
  `asset/nodepresets/`), mit **Merge-Semantik** und **Feldauswahl beim Speichern**;
  die 13 SuperScope-Figuren sind Dateien geworden, das „Figure"-Dropdown ist
  entfallen; (B) ✅ **vollständig** — zuletzt `Convolution.kernel` (7×7-Gitter),
  `ColorMap.stopPos/stopColor` (Stützstellen) und die **Bild-Felder** (Zeile
  „Image" mit `Choose…`/`Clear` + Bilder-Suchordner in den Einstellungen, damit
  ist auch der S50-Punkt erledigt); (C) ✅ **vollständig** — Klasse B (Rotating
  Stars, das **null** Parameter hatte · Osc Star · Osc Ring · Metaballs ·
  Tentacles · FyrewurX · Triangle) und Klasse A (Dot Plane · Bass Spin · Moving
  Particle, mit **⚠-Kennzeichnung** bei Abweichung vom AVS-Wert, Entscheid §8.4);
  (D) ✅ **vollständig** — dynamische EEL-Felder (init/frame/beat) für **jeden
  Knoten mit numerischen Parametern**: 48 Renderer rufen `runParamScript`, ein
  leeres Feld kostet nichts (kein Host, kein Transpiler, kein Lua-Aufruf).
  **Der Default-Vertrag ist der kritische Teil** — jeder neue Parameter muss bei
  Default pixelgleich bleiben, sonst kippt die Kalibrier-Runde.
- ✅ **Strang E — Feld-Sonden** (Vorgabe Patrik §9, umgesetzt S54):
  `asset/calibration/fields/` mit Inventar-Golden (**85 Typen, 717 Felder**,
  C++-Gate `test_FieldInventory`), Ernte der Wertebereiche/Beschreibungen/
  Skriptvariablen, Generator und Zwei-Läufe-Urteil. **Was er sofort fand:**
  43 von 129 Skript-Frame-Kopien wurden nie gelesen (das Skript rechnete, sein
  Ergebnis verfiel), und der **Init-Slot** war bei allen 47 Renderern
  wirkungslos, weil die Vorbelegung jedes Frames ihn überschrieb. Beides
  behoben; dazu die nie abgefragte `lastError()`-Meldung, `timescope.useChannel`
  und skriptbare Beat-Fader in Colorfade.
- ✅ **Grafikkarten-Auswahl in den Einstellungen** (Vorgabe Patrik S54,
  konkretisiert S61, **umgesetzt S62**) — SettingsPanel → Performance:
  Combo „Automatic / High Performance / Power Saving" + Anzeige der
  tatsächlich genutzten GPU (`GL_RENDERER`, Tooltip listet alle erkannten
  Karten); eine Änderung schreibt den Windows-Eintrag pro Anwendung
  (`HKCU\…\DirectX\UserGpuPreferences`, neues Kernmodul `core/GpuPreference`,
  Modul-Doku dort) und löst **SOFORT den Neustart** aus. Das alte, wirkungslose
  Trio `gpu.ini`/`GpuSelector`/`NvOptimusEnablement`-Export ist entfernt —
  die Registry ist die einzige Steuerung (SSOT); Windows überstimmt die
  Export-Flags ohnehin, sobald ein Eintrag existiert. Tests: 491 (+6
  Token-Logik `test_GpuPreference`).
  - **§8-Messlauf gefahren (S62), Ergebnis: KEINE Verschiebung.** Vorher-
    Baseline auf der 610M (`out/gpu_baseline_radeon610m/`): Matrix 41/43
    (0 Fehler, PRUEFEN = nur die abgenommenen Wächter water/grain) ·
    Modul-Sonden 91/91 · Feld-Sonden WIRKT 669/SCHWACH 33/STUMM 0 ·
    Bit-Identität 3/3 · Zwillinge 67/67. Nachher auf der RTX 4090 (per
    Registry-Eintrag für den Standalone, danach entfernt —
    `out/gpu_after_rtx4090/VERGLEICH.md`): **alle Urteile und Metriken
    identisch**, die drei Doppellauf-Sonden sogar **karten-übergreifend
    SHA256-identisch**. Die Prüfstände sind auf diesem Gerätepaar
    GPU-stabil; einzige Abweichung war die dokumentierte rand()-Instabilität
    zwischen Läufen. Offen bleibt nur der Klick-Sichttest des
    Sofort-Neustarts in der App (Handarbeit Patrik).
- 🔴 **`bilinear` wirkt nicht — und ist ein ORIGINAL-Feld** (Dynamic Distance
  Modifier + Dynamic Shift, Befund der Feld-Sonden S54). Im AVS heißt es
  `subpixel` und ist ein echter Preset-Wert (`r_ddm.cpp:175`,
  `r_shift.cpp:98`); unser Import liest ihn korrekt aus
  (`AvsChainTranslator.cpp:661`). Er schaltet zwei Abtastwege:
  `BLEND4(...)` mit Zwischenwerten gegen ganzzahliges Abtasten
  (`r_ddm.cpp:313`) — glatte Übergänge gegen harte Pixelkanten. **Es fehlt
  also etwas**, der zweite Zweig existiert bei uns gar nicht; wir tasten immer
  mit BLEND4 ab.
  **Zweiter Teil des Befunds:** die Vorgabe steht bei Distance Modifier
  verkehrt — das Original startet mit `subpixel = 0` (`r_ddm.cpp:210`),
  Dynamic Shift mit 1 (`r_shift.cpp:127`), bei uns beide `true`. Jedes
  importierte DDM-Preset mit dem Original-Default rendert damit interpoliert
  statt hart.
  Anbinden und Default berichtigen gehören zusammen — beides verschiebt das
  Bild und muss über die Matrix gemessen werden.
- 🔴 **104 Felder ohne erzeugbare Sonde** — durchweg Skriptfelder von Knoten
  **ohne** `runParamScript` (Fractal 2D/3D, Flame, Fractal Zoomer, Domain Warp …).
  Sie haben eigene Slots, deren schreibbare Variablen der Ernter nicht kennt.
  Entweder dort dieselbe Quelle erschließen oder die Namen in einer Tabelle
  pflegen.
- 🔧 **Kanalabhängige Felder sind derzeit blind**: der Standalone erzeugt das
  Spektrum für beide Kanäle gleich (`main.cpp`: `spec[b*2+0] == spec[b*2+1]`),
  nur die Waveform ist stereo. Timescopes Kanalfelder stehen deshalb als
  „nicht prüfbar", nicht als Befund. Für ein Urteil bräuchte es echtes
  Stereo-Material (`…\cmake\TestAudio`) — die Änderung des synthetischen Signals
  würde alle Matrix- und Sonden-Zahlen verschieben.
- 🔧 Kür: en-Übersetzungen (de = SSOT) · `CMakeUserPresets.json` → `.example` ·
  App-Umbenennung **LumiViz → LumiViz** · Pulsing-Defaults-Mismatch ·
  `File → Open Audio…`-Stub · Undock-Dauertest · Waveform-Glättungs-Default.

## 9. Bewusste Grenzen — kein Handlungsbedarf

| Bereich | Grenze |
|---|---|
| **`20_water`** | ✅ **Abgenommen (Patrik, S60):** Struktur deckungsgleich (Deckung 0,96, dMean 0,002), der Diff ist feines Rauschen ausschließlich über den Wellenzonen — Chaos-Verstärkung von GPU-float- vs. x87-Rundung durch die rückgekoppelte Wellengleichung. Randbehandlung seit S57 referenz-treu. Die Matrix-Zeile bleibt als Wächter mit diesen Erwartungswerten stehen |
| **`24_grain`** | ✅ **Abgenommen (Patrik, S60):** Kornmenge und Helligkeit exakt (dMean 0,000, Menge 0,00, dreimal identisch), nur die Einzelkörner sitzen anders (Deckung 0,56): die Referenz zieht ihren Zufall sequentiell und DATENABHÄNGIG durch die 491-Byte-Tabelle — parallel je Pixel prinzipiell nicht nachbildbar (S57). Zeile bleibt als Wächter stehen |
| AVS-Builtins | SVP Loader (10) = externe UVS/SVP-DLL, nicht decodierbar |
| AVS-APEs | 5 verworfen: GeissFluid · ParticleSystem · MIDI Trace · AVI Player · AVSTrans Automation (closed-source bzw. Meta) · Framerate Limiter = no-op (der Host taktet) |
| HLSL-Transpiler | `#elif` und Nicht-Literal-`#if` → sauberer Fehler, MD1-Fallback wie im Original |
| MilkDrop-Referenz | GPU-Rendering ist nicht bit-deterministisch — Vergleich über Statistik/Montagen, nie Pixelgleichheit |

## 10. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.303.0 | 2026-10-30 | Session 167 (Opus 5.5). §7 Instrumente: *Chords* friert beim ersten Öffnen ein — Ursache gemessen (6373 einzelne `.lvchord`, frisch 25,6 s), AK1 a: Sammlungen `.lvchords` je Stimmung, lazy gelesen (`Saiten_Konzept` §5ze). |
| 1.302.0 | 2026-10-30 | Session 166 (Opus 5.5). §7 Spielweisen: 31d-8 · 8.5 — Hörproben nach PV1 a an 33, 60, 84 (p, mf, ff; *Salamander* / Vorgabe / Suche „ohne A4“) unter `.claude/handover/hoerproben_pv1/`, dazu der blinde Hörtest `tools/hoertest/` (Wunsch Patrik); Antworten bei Patrik. Neu die Zeile 8.5-HT: Hörtest als eigenes Projekt HörTest_Helper (HT1 a–HT9 a). |
| 1.301.0 | 2026-10-30 | Sessions 164 und 165 (Opus 5.5). §7 Spielweisen: 7.5.13 B1 (Hörschwellen recherchiert), HW1 a–HW4 a, Bauauftrag 7.5.12; 7.5.12 gebaut (S165), Probelauf und A4-Wege an sechs Tasten, Glätte an keinem Weg; PV1 a — Probe 7.5.12 verfehlt, ZZ1 a, kein Lauf über Nacht. Tabelle „Offen — Stufe 31 bis zum Ende“: 8.1 auf den Zustand gekürzt (Verlauf im Konzept), 8.5 als Hörproben für das Setzen nach Gehör neu gefasst. |
| 1.300.0 | 2026-10-29 | Session 163 (Fable 5.1). §7 Spielweisen: Probe 7.5.11 verfehlt mit Befunden (Wanderer, γ/h ohne δ wirkungslos, Entartung bei h = 1, Kopplung über λ, `detune` wellig); V2 a (Plan A–D als zweiter, vorläufig letzter Anlauf), Teilplan 7.5.12–7.5.14; Werkzeuge Schnitt, LM je Taste, δ im Chor; Lehren und Bauauftrag für InstrumentsCalculation. |
| 1.299.0 | 2026-10-29 | Session 162 (Opus 5.5). §7 Spielweisen: 7.5.9 und 7.5.10 gebaut, Suche und Vergleich mit Nelder–Mead gelaufen (Zeile in S162 nicht nachgetragen, nachgeholt S163). |
| 1.298.0 | 2026-10-29 | Session 161 (Fable 5.1). §7 Spielweisen: dritter Körper neu, voller Lauf beide Compiler; NP2 a; SG1 a–SG3 a, UG1 a, UF17-1 a (voller Lauf nur bei großen Änderungen oder auf Anforderung), AB1 a (Sitzungen je Modell); 7.5.7 und 7.5.8 gebaut (Toleranzen, Struktur, Ziel); Bauauftrag für S162. |
| 1.297.0 | 2026-10-28 | Session 160 (Fable 5.1). §7 Spielweisen: DY2 a ausgeführt (Nocturne-Tafel 21 dB nach gemessenen Hammergeschwindigkeiten, Fader +1,2 dB); Befund HU4 (Überschuss in den Moden 38 / 61 / 165 Hz, Kurzschluss wirkt im Nahfeld nicht — HU4 a geprobt, 0 dB), HU4-1 a (Dämpfungsgesetz c = 4,5 im Bauplan Fassung 3, Körper neu; Glas gleicher Masse steht aus), HU4-2 b (Rückseite bleibt, aus); nächster Schritt HU2 a (3) SM4 b. S159 (1.294–1.296) hatte nur den Kopf gehoben. |
| 1.293.0 | 2026-10-28 | Session 158. §7 Spielweisen: 31d-9 fertig — Nocturne Op. 9 Nr. 2 ganz übertragen, `nocturne_demo` mitgeliefert (Daten `nocturne_op9_2.py`, Beschreibung, `CREDITS.md`), Pegel, Werte, Hörprobe, voller Lauf unter beiden Compilern. NP1 a. Offen bei Patrik: Hörurteil (DY1 a). |
| 1.292.0 | 2026-10-28 | Session 158. §7 Spielweisen: Kostprobe der Nocturne (Takte 0–15) als `.lvloop`, Befund zur Spreizung der Tafel am Flügel, DY1 a (Gabeln zuerst). CR1 a (der Creator misst auf einem Thread, gemerkt, wie IP1 a) gebaut; *Chords* beim Flügel offen. |
| 1.290.0 | 2026-10-27 | Session 157 (Fable 5.1). §7 Spielweisen: SP1 a (31d-8 „Spielbar“ vor SM4 b), SP2 a (Levenberg–Marquardt über die Stützpunkte), SP3 a (Eindeutigkeit an den Messgrößen); Teilplan 31d-8 (8.1–8.7); Lehren für InstrumentsCalculation. Nichts gebaut. |
| 1.289.0 | 2026-10-27 | Session 156. §7 Spielweisen: Probe 7.5.4 zweimal (Pool, bitgleich schneller), SM4 a gekippt (GP1 a, RV1 a), Teilplan SM4 b (ZF1 a–ZF9 a), ZZ1 a (ein Anlauf, sonst Gehör), PB1 a; voller Lauf 1631 / 1631 unter beiden Compilern. |
| 1.288.0 | 2026-10-26 | Session 155. §7 Spielweisen: Teilplan 7.5 (K1 a–K6 a, K6-1 a, SQ1 a, SQ2 a, SW1 a, SE1 a); 7.5.1–7.5.3 gebaut (Kurven nach Schroeder, `salamander_kurven.txt`, Kurvenabstand); Probe 7.5.4 gestartet; Recherche Grenzen, `Bibliothek_offen.md`. |
| 1.287.0 | 2026-10-26 | Session 154. §7 Spielweisen: 31d-7 B bis 7.4 (TB1 a, SZ1 a, SZ2 a, US1 a–US7 a, US6-1 a, Stoß je Saite gebaut, `.lvinst` 19, Klangfassung 3; RB1 a zurückgenommen, RB1-1 a); Setzen dreimal nicht brauchbar → SZ3 a, SM1 a–SM3 a, SM4 a (Abklingkurven je Teilton, Teilplan offen). |
| 1.286.0 | 2026-10-25 | Session 153. §7 Spielweisen: Teilplan 31d-7 (Recherche P1 a, P2 a bis P8 a, ZS1 a), 31d-7 A gebaut (zwei Ebenen, Eigenlöser, Längskraft je Saite, Fassung 18; P3-1 a, RZ1-1 a, WK1 a); voller Lauf 1611 / 1611. |
| 1.285.0 | 2026-10-25 | Session 152. §7 Spielweisen: 31d-1 bis 31d-5 gebaut (Salamander, WL1 a, Teiltonmessung, Messung an Salamander und Modell, D1 a, V1 a, S1 a bis S3 a, Vorgabe gesetzt, Hörproben 31d); 31d-6 begonnen (N1 a Schroeder, N2 a: 31d-7 zweite Polarisation als „Plan erweitern“); voller Lauf 1601 / 1601. |
| 1.284.0 | 2026-10-25 | Session 151. §7 Spielweisen: 31c-8 (Varianten MP1–MP3 und zwei Glasböden gerechnet, Richtungen von MAESSTRO bestanden; RS1 a, AW2 a, AW3 a gekippt, AW3-1 a) und 31c-9 (Bereich, voller Lauf 1582 / 1582 unter beiden Compilern, Hörproben aller Körper) — 31c fertig; 31d–31h geplant (UF1 a bis UF17 a, IK1, fester Plan); offen Urteil über die Hörproben, Antwort von MAESSTRO. |
| 1.283.0 | 2026-10-24 | Session 150 (Zeile nachgetragen S151). §7 Spielweisen: 31c-6 fertig (Prüffall, PF2 a, PF3 a, PS2 a, PS3 a, KV2 a, PS4 a), 31c-7 gebaut (FF1 a pocketfft, Antwort des Bodens, `.lvinst` Fassung 17, Hörproben). |
| 1.282.0 | 2026-10-24 | Session 149. §7 Spielweisen: BM7 a (Grenzfrequenz bleibt an der höchsten gerechneten Mode, 31c-6 prüft bandweise). CMakeCraft-Pin auf v0.12.0 (`/MP8` unter MSVC, MP3 a). PL31 a und zwölf Entscheide zum Rest von 31c, fester Plan IP1 und 31c-6 bis 31c-9; IP1 a (Fenster *Instruments*). Anfrage an CMakeCraft: Konfiguration Testing ohne Optimierung. Gebaut: IP1, 31c-6 Teil 1 (DM1 a, Steinway neu gerechnet) und Teil 2 (Mobilität); TC1 a und BG1 a, Pin v0.13.1. |
| 1.281.0 | 2026-10-24 | Session 148. §7 Spielweisen: 31c-4 (Eigenlöser, Spectra) und 31c-5 (Hörorte, Auswertung, statistischer Teil, Körperdatei) gebaut, `steinway_d.lvinstbody` abgelegt; offen BM7 und 31c-6. Bauzeit MSVC (`/MP`) an CMakeCraft übergeben (MP1 a). |
| 1.280.0 | 2026-10-23 | Session 147. §7 Spielweisen: Stufe 31c — der Boden gerechnet (BM1 a bis BM5-1 a, PL31c a, HT7 b, GB1 a, IC1 a, EL1 a); 31c-1 bis 31c-3 gebaut (Bauplan, Netz mit CDT, Platte mit Eigen). |
| 1.279.0 | 2026-10-23 | Session 146. §7 Spielweisen: Stufe 31 — 31a und 31b gebaut, 31c begonnen (HK1 a, HK2 a, HT1 a bis HT7 a, KB1 a); Ablage `docs/modellierung/`. |
| 1.278.0 | 2026-10-22 | Session 145. §7 Spielweisen: Stufe 31 Hammer-Saite begonnen (BL1 a, BL2 a, HS1 a, HS2 a, HA1 a bis HA4 a; nichts gebaut). |
| 1.277.0 | 2026-10-22 | Session 145. §7 Spielweisen: Freeze 30i gebaut (FP1 a bis FP3 a: Publish mit eingefrorenen Clips), Handbuch Freeze, Sichttest-Liste Freeze; Stufe 30 fertig bis auf den vollen Lauf. |
| 1.276.0 | 2026-10-22 | Session 144. §7 Spielweisen: Freeze 30f–30h gebaut (FV1 a, FV2 a, FZ4 a bis FZ7 a); Hammer-Saite wieder Stufe 31; Fix Tonführung Fall C, Fix Absturz beim Beenden (LZ1 a). |
| 1.275.0 | 2026-10-22 | Session 143. §7 Spielweisen: FZ3 a, Stufe 30e gebaut (Freeze: Dateien, Settings). |
| 1.274.0 | 2026-10-22 | Session 143. §7 Spielweisen: FZ1 a, Stufe 30d gebaut (Freeze: Abdruck, Klangfassung). |
| 1.273.0 | 2026-10-22 | Session 143. §7 Spielweisen: WX6 a bis WX9 a, Stufe 30c gebaut (*Export audio…*). |
| 1.272.0 | 2026-10-22 | Session 143. §7 Spielweisen: Stufe 30b gebaut (`audio/WavDatei`). |
| 1.271.0 | 2026-10-22 | Session 143. §7 Spielweisen: WX2 a bis WX5 a, Stufe 30a gebaut (`loop/Wellenform`); Lautstärke des Loops nach den Effekten. |
| 1.270.0 | 2026-10-22 | Session 143. §7 Spielweisen: Stufe 29f gebaut (TS1 a Loop Sound Memory, Handbuch, Sichttest-Liste Töne vorab); Stufe 29 gebaut. |
| 1.269.0 | 2026-10-21 | Session 142. §7 Spielweisen: Stufe 29 Töne vorab gebaut bis 29e (TV1 a bis TV7 a); WX1 a; Audio-Clip im Loop als Stufe 31, Hammer-Saite 32 ff. |
| 1.268.0 | 2026-10-21 | Session 141. §7 Spielweisen: Stufe 28 gebaut bis auf den vollen Lauf (28c-2b bis 28f); Konzept Hammer-Saite begonnen (Zielbild, H1 a bis H3 a Töne vorab, F1 a bis F6 a Freeze, Bauplan 29/30/31); Befund Tab-Linien und Karaoke-Panel S89 behoben. |
| 1.267.0 | 2026-10-20 | Session 140. §7 Spielweisen der Instrumentfamilien: SF6 a bis SF10 a, Skizze Stufe 1, SK1 a bis SK11 a entschieden; Stufe 28 gebaut bis 28c-2a (Zeitspuren, Pedale, Expression, Vibrato, Pitch, Vibratohebel); offen 28c-2b, Hörproben, 28d bis 28f. |
| 1.266.0 | 2026-10-20 | Session 139. §7 Spielweisen der Instrumentfamilien: Konzept begonnen (SF1 c bis SF5 d); §7 NEU Lautstärke im Loop: Stufe 27 gebaut (LL1 a bis LL14 a), offen Sichttest, drei gekappte Kit-Rollen, Fader live, voller Lauf. |
| 1.265.0 | 2026-10-19 | Session 138. §7 Spielweisen: S-L15 b gebaut; S-T8 a bis S-T14 a entschieden; 26e bis 26h gebaut (Fenster *Chords* mit Galerie, Anhören, Favoriten, eigenen Griffbildern und Voicings; *Play as* mit Hammer-on, Slide, Bend; zwei Beispiele, Handbuch 1.191.0, Sichttest-Liste); Stufe 26 durch bis auf den vollen Lauf; Idee Patrik: Spielweisen der anderen Instrumentfamilien (Pedale, Dämpfer); Befund Karaoke-Panel S89 schwankt unter Testing-MSVC. |
| 1.264.0 | 2026-10-19 | Session 137. §7 Spielweisen: 25c-4 *Follow string* gebaut (S-L13 a), Bend nach S-L14 a; Tabs und Akkorde entschieden (S-T1 a bis S-T7 a, S-L15 b) und gebaut 26a bis 26d; Vorgabe Patrik für das Fenster *Chords* (Anhören, Favoriten); offen S-L15 b umsetzen, 26e–26h, Durchsicht der gerechneten Griffbilder. |
| 1.263.0 | 2026-10-19 | Session 136. §7 Spielweisen: 24f fertig (S-W16 a bis S-W22 a); Stufe 25 entschieden (S-L1 a bis S-L12 a) und gebaut bis 25c-3b; Zoom der Editoren vorerst nicht gemerkt; Ideen Patrik (Rechtsklick auf die Spur Strum mit Pfeilen, Pick scrape); Befund Muster der Projektdatei Sichttest. |
| 1.262.0 | 2026-10-18 | Session 135. §7 Spielweisen: 24e gebaut (S-W13 a bis S-W15 a), Fehler „Anschläge kamen nicht im Loop an“ behoben; Zoom in den Loop-Editoren (E-Z1 a). |
| 1.261.0 | 2026-10-18 | Session 134. §7 Spielweisen: S-W2 a bis S-W12 a entschieden; Stufen 24a bis 24d gebaut (Modell, Planung, Klang, Creator). |
| 1.260.0 | 2026-10-18 | Session 133. §6 Saiteninstrumente: Korpus Classical guitar, Bemessung der Tonabnehmer, 23e, 23f, 23g gebaut (S-K9 a bis S-G4 a); §7 Spielweisen: Ziel S-W1 a. |
| 1.259.0 | 2026-10-18 | Session 132. §6 Saiteninstrumente: S-K8 a; Korpus Parlor gebaut (gekoppeltes Modell nach Hess). |
| 1.258.0 | 2026-10-18 | Session 132. §6 Saiteninstrumente: S-K7 a; 23d fertig (Cable, Mensuren der Vorlagen). |
| 1.257.0 | 2026-10-18 | Session 132. §6 Saiteninstrumente: S-K6 a, S-U3 a; Stufe 23d erster Teil gebaut (Waveguide, Tonabnehmer). |
| 1.256.0 | 2026-10-18 | Session 132. §6 Saiteninstrumente: S-K4 a, S-K5 a entschieden, Stufe 23c ohne Korpus gebaut (Extended KS). |
| 1.255.0 | 2026-10-18 | Session 132. §6 Saiteninstrumente: S-A1 a entschieden, Stufe 23b gebaut (Saitenwahl, Abdämpfen beim Planen). |
| 1.254.0 | 2026-10-18 | Session 132. §6 Saiteninstrumente: Tonumfang S-T1 a — Noten A0 bis C8 (vorher C1 bis C7). |
| 1.253.0 | 2026-10-18 | Session 132. §6 Saiteninstrumente: Konzept `audio/Saiten_Konzept.md` mit zehn Entscheiden (S-Z1 a bis S-B1 a), Creator-Skizze gebilligt, Stufe 23a gebaut (Modell, `.lvinst` 10, `.lvloop` 8); Befund Tonumfang B0. |
| 1.252.0 | 2026-10-17 | Session 131. §3 `stratospheric turbulences` 🟠 → 🟡: Tunnelzentrum mittig, zwei Fehler behoben — quadratische Matrizen transponiert (S2 a, HlslTranspiler 1.7.0, 12 595 Presets im MegaPack), 2D-Rauschtexturen gespiegelt gelesen (S3 a); vier neue Sonden; bisher 748/748 bitgleich; Rest kleiner Versatz. |
| 1.251.0 | 2026-10-17 | Session 131. §3 **Shader-Eingänge rad/ang und Wrap von `sampler_main` wie die Referenz** (S1 a, war 🔴 Starfield fast schwarz): zwei Ursachen seit C1, Nebenbefund `ang` im Warp aus S130, drei neue Sonden, Vergleichslauf (bisher 748/748 bitgleich, Original 340 Bilder in 13 Presets, Magma besser). §7 Befund: ein Testfall hängt an der Reihenfolge (`presetIconDir()`). |
| 1.250.0 | 2026-10-17 | Session 130. §3 **Shader sehen y wie die Referenz** (WP1 a, WP1-R a, WP2 b): Warp und Comp, Felder warpYOriginal/compYOriginal, neun Y-Sonden mit Auswertung am Bild, Vergleichslauf (bisher 748/748 bitgleich, Starfield-2-Familie nicht mehr formatabhängig, Twisted besser); neu 🔴 Starfield (Supernova) in beiden Ständen fast schwarz. Dazu aus dem Sichttest 11: Limiter mit Vorschau (L2-1 b), C1 c, Resume-Knopf, W8 a, W8-2 a. |
| 1.249.0 | 2026-10-16 | Session 129, zweiter Teil. §6 Hotkeys Stufe 1 gebaut (§9.3 umlaufen, §9.5 Menü zeigt die Taste, F11 über den Filter). §3 **🔴 Warp-Shader sieht `uv.y` gespiegelt** — Sonden `ysonde/Y_comp.milk` (OK) und `Y_warp.milk` (Befund). §1 Blur eingegrenzt: Blau +8 bei Stärke 1, drei Ursachen ausgeschlossen, Ursache offen. `movement3b.lvfx` ohne die toten Movement-Felder. §6 neu: Saiteninstrumente (bald), weitere Loop-Effekte. |
| 1.248.0 | 2026-10-16 | Session 129. **Widersprüche bereinigt** (Liste aus der Übersicht S125, 16 von 18; je Stelle gegen Dokument, Changelog und Code geprüft): §1 Water-Kopf überholt (enabled-Schalter, S74), Modul-Matrix-Reste 43/43, vier VERDECKT-Zeilen ✅ · §3 Feld „Audio“ mit Standard Original, Texturen bleiben lokal (gitignored), Playlist-Anbindung entfällt (S93 C4 a), **die Tabelle der laufenden Kalibrier-Runde stand im eingeklappten „überholt“-Block** — `</details>` an die richtige Stelle, eigener Kopf; Warp-Sonde (🔧) und Flip X/Y (🟡, Wunsch Patrik S75) als eigene Zeilen · §5 Hotkeys: Entscheid S60 nicht umgesetzt — neu 🔧 in §6 · §6 onChange als Song-Slot umgesetzt · §7 Stärke-Regler gebaut (S89), Shadertoy-Modul gebaut (S65), Lizenz-Pflicht umgesetzt, Test-Presets als Feld-Sonden umgesetzt, Movement-Beat-Umkehr entschieden · §8 Oberflächentexte erledigt, `.lvfx`-Zwillinge 67/67 und Randspalten-Wächter seit S59. Entschieden: B1 a (Blur mit Raster nachgemessen, §1: der Befund steht — 320×240 dMean 0,025 / MAE 0,009, gleichmäßiger kleiner Versatz plus Randzeilen, 740×460 OK), B2 a (Vergleichslauf seit `vor-composer` auf `master` nachgeholt: AVS 252 / 252 und MilkDrop 747 / 748 bitgleich, der eine ein Ausreißer eines abgebrochenen Laufs, §7). §6 neu: Idee Patrik — Vocals unterdrücken oder allein behalten (ohne KI: Mitte auslöschen; mit KI: Stems). Dazu F1 a (Patrik): Fenster der Standalones und der Unit-Tests erscheinen auf Bildschirm 2 ohne Fokus (voller Lauf 1279 / 1279 unter Release-clang und Testing-MSVC). |
| 1.247.0 | 2026-10-16 | Session 129. §7 Loop: die Tests über alle mitgelieferten Loops lesen die Liste aus `asset/Asset.cmake`, Arbeitsdateien prüfen sie nicht mehr (W1 a, W2 a; Loop_Konzept 7e). |
| 1.246.0 | 2026-10-16 | Session 128. §7 Loop: der Loop hängt nicht mehr am Composer (N1 b); Loop-Kleinkram abgeschlossen. |
| 1.245.0 | 2026-10-16 | Session 128. §7 Loop: Cowbell und Clave als Instrumente aus den Aufnahmen, in zwei Kits (C2 a). |
| 1.244.0 | 2026-10-16 | Session 128. §7 Loop: Cowbell und Clave als mitgelieferte Samples (C1 a). |
| 1.243.0 | 2026-10-15 | Session 127. §7 Loop: 7b (Kanäle und Spuren umsortieren) entschieden und gebaut. |
| 1.242.0 | 2026-10-15 | Session 127. §7 Loop: Kleinkram 3b bis 6 erledigt, 7a (Loop allein als Song) entschieden und gebaut. |
| 1.241.0 | 2026-10-15 | Session 127. §7: Mitlaufen der Loop-Fenster entschieden (W1 a mit Kaskade, W2 a, W3 a) und gebaut; Idee Spielweisen der Saiteninstrumente aufgenommen. |
| 1.240.0 | 2026-10-15 | Session 127. §7 Composer: G6 fertig, Sichttest 11 offen. |
| 1.239.0 | 2026-10-15 | Session 127. §7 Composer: G5 gebaut (Dialog Match loudness), offen G6. |
| 1.238.0 | 2026-10-15 | Session 127. §7 Composer: G4 fertig (Teil 2), G5 entschieden (G5-1 bis G5-4); Wunsch: Loop-Editor läuft mit dem Composer mit. |
| 1.237.0 | 2026-10-15 | Session 126, Abschluss. §7 Composer: G4 Teil 1 gebaut, G4b a entschieden. |
| 1.236.0 | 2026-10-15 | Session 126. §7 Composer: Beleg G4, Gain über 0 dB in der BassEngine behoben. |
| 1.235.0 | 2026-10-15 | Session 126. §7 Composer: einzelne Clips ausschalten (D1 c) gebaut. |
| 1.234.0 | 2026-10-15 | Session 126. §7 Composer: Lautstärke-Abgleich G2 gebaut. |
| 1.233.0 | 2026-10-15 | Session 126. §7 Composer: Lautstärke-Abgleich G1 und G3 gebaut. |
| 1.232.0 | 2026-10-15 | Session 126. §7 Composer: Lautstärke-Abgleich der Ton-Clips entschieden (L1 a bis L4 a, Composer_Konzept §9g), nicht gebaut. |
| 1.231.0 | 2026-10-15 | Session 125. §6 Loop: Menüs des Rechtsklicks, Rename, Tags und Schieber getestet. |
| 1.230.0 | 2026-10-15 | Session 125. §6 Loop: offene Listen des Loops lesen nach einem Import neu. |
| 1.229.0 | 2026-10-15 | Session 125. §6 Loop: Editoren merken Größe und Lage; Clip-Länge eines Loops und Icons als erledigt vermerkt. |
| 1.228.0 | 2026-10-15 | Session 125. §7 Publish: Stufe P-A gebaut (Knöpfe, Menü File), offen Sichttest und P-B. |
| 1.227.0 | 2026-10-15 | Session 125. Keep both belegt und behoben (`nameMitZahl` → `<name>_2`). |
| 1.226.0 | 2026-10-15 | Session 125. §7 Publish entschieden (P1 a bis P8 a), Steuerdokument `core-services/Publish_Konzept.md` 0.1.0; Nebenbefund Keep both (`nameMitZahl`). |
| 1.225.0 | 2026-10-14 | Session 124. E1 a gebaut: Schalter der Effekte je Clip, Play pattern mit Effekten (Loop_Konzept §4m); 1221 Cases. |
| 1.224.0 | 2026-10-14 | Session 124. §7 „Bars kleiner als 4" **gebaut** mit dem Fehler beim Einfügen und dem Springen im Zeitstrahl (B1 a, B2 a, B3 a, B4 b; Loop_Konzept §4m). 1219 Cases. |
| 1.223.0 | 2026-10-14 | Session 124. Kits **21e gemacht**, **Stufe 21 fertig**: Icon `.lvkit` (I2 a), Benutzerhandbuch 1.163.0, Liste `Sichttest_Kits.md`; voller Lauf 1215 / 1215. Als Nächstes: „Bars kleiner als 4" mit dem Fehler beim Einfügen (§7). |
| 1.222.0 | 2026-10-14 | Session 124. K10 a und K11 a gebaut (Kitwechsel mit zwei Haken, * für fremde Spuren); §7 „Bars kleiner als 4" um den Befund Kopieren/Einfügen ergänzt (Versatz bei leeren Rändern der Auswahl — mit jenem Punkt zu bauen, Wunsch Patrik). 1215 Cases. |
| 1.221.0 | 2026-10-14 | Session 124. Kits **21d gebaut** (D1 a bis D6 a): 37 Stereo-Samples aus DrumGizmo (CC BY 4.0), drei Besen, vier Kits (vierzehn), Brush Swing; 75 Samples, 93 Instrumente. Offen: 21e, Patriks Ohr an `hoerproben_21d`. |
| 1.220.0 | 2026-10-14 | Session 124. Kits **21c gebaut**: das Fenster Kits, die Auswahl Sound als gemeinsames Bauteil, Used by mit den zuletzt geöffneten Loops. 1213 Cases; 240 Bereichsfälle unter Testing-MSVC grün. Offen: 21d, 21e. |
| 1.219.0 | 2026-10-14 | Session 123, Abschluss. `sessions/Changelog_Session123.md`. **Stand:** Vorlagen (20 Formen, 38 Samples `lib:`, 90 Instrumente, 9 Loops), Kits Stufe 21a und 21b gebaut (Modell, zehn Kits, Drum-Editor), VST3-Konzept 0.1.0, Versuch Titanium (lokal). **Offen:** 21c (Fenster Kits) als Nächstes, 21d, 21e; §7 NEU Wunsch Patrik „Bars kleiner als 4" (Befund: `kTakteMin` ist Untergrenze und Länge neuer Clips zugleich); Patriks Ohr an `hoerproben_s123`. 1210 Cases, voller Lauf 1202 / 1202 nach den Vorlagen, danach 236 Bereichsfälle unter beiden Compilern grün. |
| 1.218.0 | 2026-10-14 | Session 123. §7 NEU „Vorlagen S123": Formen (20), mitgelieferte Samples `lib:` (38, CC BY 3.0, Entscheide 1 a, 2 a), 35 Instrumente mehr (90), neun Loops; offen die Kits (3 a, Skizze gezeigt, Entscheide ausstehend), Patriks Ohr an `hoerproben_s123`; Versuch „Titanium" (lokal); VST3-Konzept `audio/VST3_Konzept.md` 0.1.0. |
| 1.217.0 | 2026-10-14 | Session 122, Abschluss. `sessions/Changelog_Session122.md`. **Stand:** Klang aus einer Form — **Stufe 20 vollständig**: 20a (Edit shape), 20b (SVG), 20c (Bild), 20d (mehrere Konturen, Brücken, Farben der Schichten); `audio/Shape_Konzept.md` 0.20.0, Benutzerhandbuch 1.161.0; dazu die Sichttest-Liste `Sichttest_Shape_Klang-aus-einer-Form.md` (lokal) mit den Vorlagen `asset/sichttest/presets/Shape.lvfx` und `XyScope.lvfx`. **Offen:** im Programm ungesehen und ungehört (19b bis 20d) — Patriks Sichttest mit der Liste; Patriks Ohr an `hoerproben_19c`; die von Claude gesetzten Zahlen und Regeln; CSS-Variablen in SVG; mitgelieferte Instrumente mit Shape-Schicht. 1200 Cases, voller Lauf 1199 / 1199 nach 20d (Testing-MSVC, Bildschirm 2), der Fall danach unter beiden Compilern grün. |
| 1.216.0 | 2026-10-14 | Session 122. §7 „Klang aus einer Form": **Stufe 20d gebaut — mehrere Konturen** (Entscheide Patrik L1 a, F1 a, V1 a; `audio/Shape_Konzept.md` 0.20.0, Benutzerhandbuch 1.161.0); Stufe 20 damit vollständig. Offen: im Programm ungesehen und ungehört; die gesetzten Abstufungen und Regeln. |
| 1.215.0 | 2026-10-14 | Session 122. §7 „Klang aus einer Form": **Stufe 20c gebaut — Konturen aus einem Bild** (Entscheide Patrik R1 a, R2 a; `audio/Shape_Konzept.md` 0.19.0, Benutzerhandbuch 1.160.0); Patriks Idee zu den Farben der Schichten (Paletten je Waveform) beim Punkt zu 20d. Offen: im Programm ungesehen; die gesetzten Vorgaben. **Als Nächstes Stufe 20d.** |
| 1.214.0 | 2026-10-14 | Session 122. §7 „Klang aus einer Form": **Stufe 20b gebaut — SVG** (Entscheide Patrik O1 a, O2 a, O3 a; `audio/Shape_Konzept.md` 0.18.0, Benutzerhandbuch 1.159.0). Offen: im Programm ungesehen; CSS-Variablen; Farben für Schichten (mit 20d); als Nächstes Stufe 20c. |
| 1.213.0 | 2026-10-14 | Session 122. §7 „Klang aus einer Form": **Stufe 20a gebaut — das Fenster Edit shape** (Entscheide Patrik Z1 a, Z2 a; `audio/Shape_Konzept.md` 0.17.0, Benutzerhandbuch 1.158.0), mit Edit... und New shape... im Creator; erledigt: der Knoten liest eine geschriebene Form neu, gerechnete Töne entstehen neu, der Knopf Edit.... Offen: im Programm ungesehen; die von Claude gesetzten Zahlen; als Nächstes Stufe 20b. |
| 1.212.0 | 2026-10-13 | Session 121, Abschluss. `sessions/Changelog_Session121.md`. **Stand:** Klang aus einer Form — gebaut die Stufen **19b** (Knoten Shape), **19c** (die Schwingung Shape im Klangerzeuger, `.lvinst` Fassung 9), **19d** (Knoten XY Scope), **19e** (die Shape-Schicht im Instrument-Creator); `audio/Shape_Konzept.md` 0.16.0, Benutzerhandbuch 1.157.0. **Offen:** im Programm ungesehen und ungehört bis auf Patriks Blick auf den Knoten Shape (ohne Befund); Patriks Ohr an `hoerproben_19c`, danach die Vorlagen zu den Ecken und zur Kante des Sprungs; mitgelieferte Instrumente mit Shape-Schicht; **als Nächstes Stufe 20a**, das Fenster Edit shape — mit dem Knopf Edit... im Creator und dem Neuladen einer geänderten Form. Voller Lauf 1181 / 1181, 0 Skips (Testing-MSVC, Bildschirm 2), seither nur Doku geändert. |
| 1.211.0 | 2026-10-13 | Session 121. §7 „Klang aus einer Form": **Stufe 19e gebaut — die Shape-Schicht im Instrument-Creator** (`audio/Shape_Konzept.md` 0.16.0, Benutzerhandbuch 1.157.0). Offen: im Programm ungesehen und ungehört; der Knopf Edit... kommt mit 20a; kein mitgeliefertes Instrument mit Shape-Schicht. **Als Nächstes Stufe 20a**, das Fenster Edit shape. |
| 1.210.0 | 2026-10-13 | Session 121. §7 „Klang aus einer Form": **Stufe 19d gebaut — der Knoten XY Scope** der Effect Chain (Entscheide Patrik X1 a, X2 a; `audio/Shape_Konzept.md` 0.15.0, Benutzerhandbuch 1.156.0). Offen: im Programm ungesehen, keine Sichttest-Liste, keine Feld-Sonden. **Als Nächstes Stufe 19e**, die Shape-Schicht im Creator. |
| 1.209.0 | 2026-10-13 | Session 121. §7 „Klang aus einer Form": **Stufe 19c gebaut — die Schwingung Shape im Klangerzeuger**, `.lvinst` Fassung 9 (Entscheide Patrik T1 a, T2 a; `audio/Shape_Konzept.md` 0.14.0). Offen: Patriks Ohr an den Hörproben; Ecken dämpfen; die Kante des Sprungs (gemessen −33 dB bei A4); gerechnete Töne entstehen nach dem Schreiben einer Form nicht neu. **Als Nächstes Stufe 19d.** |
| 1.208.0 | 2026-10-13 | Session 121. §7 „Klang aus einer Form": **Stufe 19b gebaut — der Knoten Shape** der Effect Chain (Entscheide Patrik V1 a bis V4 a; `audio/Shape_Konzept.md` 0.13.0, Benutzerhandbuch 1.155.0). Offen: im Programm ungesehen; eine geänderte Form-Datei wird erst bei neuer Nennung gelesen; keine Feld-Sonden für `shape`; der „~"-Knopf an Contour. **Als Nächstes Stufe 19c.** Voller Lauf 1169 / 1169, 0 Skips (Testing-MSVC, Bildschirm 2). |
| 1.207.0 | 2026-10-13 | Session 120, Abschluss. `sessions/Changelog_Session120.md`. **Stand:** Klang aus einer Form — das Konzept ist vollständig (`audio/Shape_Konzept.md` 0.12.0: sieben Vorlagen geschlossen, zwei Skizzen gebilligt, Baustufen freigegeben); gebaut **Stufe 18** (Pan je Schicht, acht Schichten, `.lvinst` Fassung 8) und **Stufe 19a** (`shape/Kontur`, die Datei `.lvcontour`, sechs mitgelieferte Formen, der Bereich Shapes im Export); daneben acht Datei-Icons, im Explorer-Skript und an vier Stellen der App. **Offen:** im Programm ungesehen und ungehört — Liste 10 Abschnitt V (V0 bis V11, U77 bis U79), dazu weiter Abschnitt D, X7 und die Reste aus S118; `register_file_icons.ps1 -Apply -Foreign` (Patrik); **als Nächstes Stufe 19b**, der Knoten Shape für die Effect Chain; in 19c die Kante des Sprungs hören; klein: Pan bei Stereo XY, die Bereiche von Harmonics, `.lvp` ohne Icon, `milk2` in `presetTypeIconForSuffix`, README der Icons gegen Handbuch zum Öffnen einer Datei über die Exe; Ideen Publish und Flanger. Voller Lauf 1163 / 1163, 0 Skips (Testing-MSVC, Bildschirm 2), seither nur Doku geändert. |
| 1.206.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": **Stufe 19a gebaut** (`Shape_Konzept.md` 0.12.0) — `shape/Kontur`: die Zeichnung aus Konturen mit festen Kennungen, die Datei `.lvcontour` Fassung 1, mitgelieferte unter `asset/contours` (sechs Formen), eigene als `user:<name>`, die Reihe mit weichem Gewicht; der Bereich **Shapes** in Export und Import (Benutzerhandbuch 1.154.0). Neu `test_ShapeKontur.cpp` (sechs Fälle, **1163 Cases**). Im Programm ist davon nur der neue Bereich im Export zu sehen. Voller Lauf 1163 / 1163, 0 Skips (Testing-MSVC, Bildschirm 2). **Als Nächstes: Stufe 19b** — der Knoten Shape für die Effect Chain. |
| 1.205.0 | 2026-10-13 | Session 120. **Stufe 18 gebaut: Pan je Schicht und acht Schichten je Instrument** (`Loop_Konzept.md` 0.70.0 §5, `Shape_Konzept.md` 0.11.0) — `.lvinst` Fassung 8; Pan mit dem Gesetz von Spur und Muster, im Creator der Regler im Kasten Level; alte Instrumente rechnen Bit für Bit wie vorher; Benutzerhandbuch 1.153.0; lokal Liste 10 um Abschnitt V (V0 bis V11, U77 bis U79; darin auch die Punkte zu den Datei-Icons dieser Sitzung), B1 und U19 angepasst. **Im Programm ungesehen und ungehört.** Voller Lauf 1157 / 1157, 0 Skips (Testing-MSVC, Bildschirm 2, 263 s). **Als Nächstes: Stufe 19a** — die Form als Daten und die Datei `.lvcontour`. |
| 1.204.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": **Baustufen freigegeben (Patrik)** — 18 (Pan je Schicht), 19a bis 19e (Daten und Datei, Knoten Shape, Schwingung im Klangerzeuger, Knoten XY Scope, Shape-Schicht im Creator), 20a bis 20d (das Fenster Edit shape); **die Grenze der Schichten geht auf acht (Patrik)**, mit Stufe 18. `Shape_Konzept.md` 0.10.0; alle sieben Vorlagen sind geschlossen, das Konzept ist vollständig. **Als Nächstes: Stufe 18 bauen.** Nichts gebaut. |
| 1.203.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": **die Skizzen gebilligt (Patrik)** — das Fenster Edit shape und die Shape-Schicht im Creator, als Beschreibung in `Shape_Konzept.md` 0.9.0 (§6, §8a); **G1 a** (die Zeile Contour an der Schicht entscheidet, was sie spielt), **G2 a** (Regeln für Sprünge von Hand); berichtigt nach Patriks Einwand: der Wellenzug der Preview hat je Zeitpunkt einen Wert. Die sechste der sieben Vorlagen ist geschlossen; **offen: die Freigabe der Baustufen** und ob die Grenze der Schichten auf acht geht. |
| 1.202.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": **die Module entschieden (Patrik): M1 a, M2 b, M3 a** (`Shape_Konzept.md` 0.8.0, §10, §14) — zwei neue Knoten für die Effect Chain: **XY Scope** (links gegen rechts, volle Genauigkeit) und **Shape** (zeichnet die Konturen einer `.lvcontour`, mit Harmonics und Show circles), beide mit den üblichen Skriptfeldern und einigen Presets; Shape kommt gleich nach Datei und Koeffizienten, XY Scope nach dem ersten Ton. Die fünfte der sieben Vorlagen ist geschlossen; **offen: die Skizze** des Form-Fensters und der Shape-Schicht, danach die Baustufen; **nicht entschieden:** ob die Grenze der Schichten von vier auf acht geht. |
| 1.201.0 | 2026-10-13 | Session 120. **Acht Schichten je Instrument gemessen und probiert (Frage Patrik)** — eingetragen bei der Datenablage unter `kSchichtenMax`: die Rechenzeit wächst gerade mit der Zahl (Release-clang rund 2 ms je Sekunde Ton und Schicht, acht also 16 ms), mit der Grenze probeweise auf 8 fallen nur die zwei Fälle durch, die die Grenze prüfen; zu beachten die Spitze (2,52 bei acht Schichten auf Level 1) und eine neue Fassung der `.lvinst`. Neu ein Messfall in `test_LoopBausteine.cpp` (**1156 Cases**). Die Grenze steht weiter auf vier, nicht entschieden. Dazu `Shape_Konzept.md` 0.7.0: die Abtastrate der Engine ist 44,1 kHz, nicht 48 (berichtigt), und was die Visuals vom Ton bekommen, genauer nachgesehen. |
| 1.200.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": **As one path entschieden (Patrik): B2 d, B1 wählbar** (`Shape_Konzept.md` 0.6.0, §9) — jede Kontur behält ihre eigene Reihe und bleibt geschlossen, eine Schicht spielt sie nacheinander in einem Umlauf, dazwischen ein kurzer gerader Sprung; wo der Stift springt, ist im Form-Fenster wählbar: Reihenfolge der Liste (Vorgabe), kürzeste Wege, von Hand eingezeichnet. Die vierte der sieben Vorlagen ist geschlossen; als Nächstes die zwei Module für die Effect Chain. **Offen, in Stufe 19b zu hören:** die Kante des Sprungs im Ton (in der Probe −31 bis −51 dB über der eingestellten Zahl von Harmonischen, grob, nicht gehört). |
| 1.199.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": **SVG entschieden (Patrik): S1 a, mit Beleg** (`Shape_Konzept.md` 0.5.0, §8) — Qt Svg zeichnet auf ein mitschreibendes Zeichengerät, Kurven, Lage und Füllfarbe kommen an; dazu belegt: `QPainterPath` trägt Strecken und Kurven ohne Qt-Anwendung (D4 b). Neu `test_ShapeBelege.cpp` (zwei Fälle, **1155 Cases**), Qt Svg in der `Solution.json`. Die dritte der sieben Vorlagen ist geschlossen; als Nächstes die Brücke bei As one path. Nicht belegt: Text, `use`, CSS-Stile, Verläufe, Dateien aus Inkscape oder Illustrator. Voller Lauf steht aus. |
| 1.198.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": **Konturerkennung entschieden (Patrik): K1 a, K2 a** (`Shape_Konzept.md` 0.4.0, §8) — über Flächen, Split by Brightness, Colour oder Transparency, Regler Levels, Smooth, Min size, Detail; davor eine Probe an drei Bildern; die zweite der sieben Vorlagen ist geschlossen, als Nächstes SVG. §7 **Publish, Nachtrag (Patrik):** bei gleichen Namen eine Option Vergleichen mit den Wegen überschreiben, umbenennen, nicht übernehmen; beim Umbenennen werden die Nennungen im Paket mitgeändert — das Muster gibt es beim Host Switcher (S82) und bei Rename base im Loop, der Import der `.lvdata` tut es heute nicht. |
| 1.197.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": **die Datei `.lvcontour` entschieden (Patrik): D1 a, D2 a, D3 a, D4 b** (`Shape_Konzept.md` 0.3.0, §7; bei D4 zuerst a, dann b: Konturen als Pfade aus Strecken und Kurven) — damit ist die erste der sieben Vorlagen geschlossen; als Nächstes die Konturerkennung aus einem Bild. §7 **NEU Idee Patrik: Publish** — ein Ding mit allen zugehörigen Dateien, der Ordnerstruktur des Benutzerordners und einem Manifest als Paket `<name>.<type>.lvdata`; nachgesehen: eine `.lvdata` ist heute schon ein Zip mit Manifest und `data/<Pfad im Benutzerordner>`; offen die Arten, was dazugehört, gleiche Namen beim Import (die Nennungen im Paket), das Manifest. Nichts entworfen, nichts gebaut. |
| 1.196.0 | 2026-10-13 | Session 120. **Datei-Icons in der App, A2 a (Patrik) gebaut:** die Datei-Zeile des Loop-Panels, die Kopfzeile des Instrument-Editors, der Kopf des Dialogs Import data und die Liste „je Datei entscheiden" tragen das Icon ihrer Dateiart (Benutzerhandbuch 1.152.0) — im Programm ungesehen, vier Bilder aus den Tests angesehen. Bewusst ohne Icon: Fenster Instruments, Creator, die Menüs Fade in, Fade out, Shape, Mute-Profile, Verlaufs-Combos. **Nebenbefund, nicht behoben:** `presetTypeIconForSuffix` kennt `milk2` nicht — eine `.milk2` bekäme dort das AVS-Icon; nicht nachgesehen, ob der Effect Browser `.milk2` überhaupt zeigt. |
| 1.195.0 | 2026-10-13 | Session 120. **Datei-Icons in der App (A1 b, Patrik):** im Audio Browser trägt eine `.lvloop` jetzt `lumiviz_lvloop.ico` statt des Kreispfeils (`dateiArtIcon` in `PresetTypeIcons.hpp`; Benutzerhandbuch 1.151.0) — im Programm ungesehen. Durchgesehen, wo die App Dateien der acht Arten sonst zeigt; **offen A2:** welche Stellen ein Icon bekommen — vorgeschlagen die Datei-Zeile des Loop-Panels, die Kopfzeile des Instrument-Editors, Kopf und Baum „je Datei entscheiden" des Dialogs Import data; nicht vorgeschlagen das Fenster Instruments, der Creator, die Menüs Fade in, Fade out, Shape, die Mute-Profile und die Verlaufs-Combos. |
| 1.194.0 | 2026-10-13 | Session 120. **F1 a (Patrik):** die Kontur-Datei heißt `.lvcontour` (`Shape_Konzept.md` 0.2.0). **Acht Datei-Icons gebaut (I1 a, Patrik):** `.lvloop`, `.lvinst`, `.lvcontour`, `.lvfade`, `.lvshape`, `.lvmute`, `.lvdata`, `.grad` — alle als Marke mit einem Zeichen im Kreis, die Zeichen von Patrik an vier Vorschaublättern gewählt; im Skript für den Explorer eingetragen, Benutzerhandbuch 1.150.0 (§17). **Offen:** `-Apply` für die neuen Endungen (Patrik); die App zeigt die acht Icons noch nirgends (im Audio Browser steht bei Loops weiter der Kreispfeil des Systems); `.lvp` ohne Icon — nicht nachgesehen, ob das Format noch geschrieben wird; die README der Icons sagt, `LumiViz.exe <datei>` lade keine Datei, das Handbuch sagt, eine Datei lasse sich auf die Exe ziehen — nicht abgeglichen. |
| 1.193.0 | 2026-10-13 | Session 120. §7 „Klang aus einer Form": das Grobkonzept ist auf LumiViz umgeschrieben — **NEU `docs/audio/Shape_Konzept.md` 0.1.0** (Rechnung, Regler, Tests, Stufen 18 bis 21 als Vorschlag, sieben Vorlagen in fester Reihenfolge). Nachgesehen: **die Endung `.lvshape` ist seit S96 vergeben** (Stück-Presets des Curve Editors) — in S119 nicht aufgefallen, gehört in die erste Vorlage; die Visuals bekommen vom Loop beide Seiten getrennt (512 Werte je Seite und Bild; die Frage aus S119 ist beantwortet); Qt Svg liegt in der Installation, steht nicht in der `Solution.json`. Festgehalten: Start und Reverse ändern nur Phasen. Nicht nachgeprüft: ob Qt Svg Pfade als Geometrie herausgibt; die Rate der Engine. Nichts entworfen, nichts gebaut. |
| 1.192.0 | 2026-10-13 | Session 119, Abschluss. `sessions/Changelog_Session119.md`. **Stand:** Stufe 17 (ein Sample bearbeiten) und X5 (Abweichung im Kasten) sind gebaut, voller Lauf 1153 / 1153 unter Testing-MSVC; die Idee „Klang aus einer Form" ist in Z1 a bis Z5 a gefasst. **Offen:** im Programm ungesehen Liste 10 Abschnitt D, X7, U72 bis U76 und der Rest aus S118; Patriks Ohr an `hoerproben_16e`; `asset/loop/effects_demo2.lvloop` liegt seit S119 im Repo — bleibt sie als mitgelieferter Loop (dann die Werte festhalten) oder soll sie heraus; **als Nächstes die Form:** Konzept auf LumiViz umschreiben, dann Dateiformat, Konturerkennung, SVG, Module, Skizze |
| 1.191.0 | 2026-10-13 | Session 119. §7 NEU Idee Patrik: **Klang aus einer Form** — Waveform „Shape" im Instrument-Creator (Fourier-Reihe einer Kontur; Grobkonzept `fourier-shape-loops-konzept.md`); Z1 a: in LumiViz, in C++; Eingabe durch Zeichnen, SVG oder Konturen aus einem Bild; Z2 a im Grundsatz (Angle, Stereo XY), dazu die Richtungen ±x und ±y für überlagerte Schichten und ein Modul für die Effect Chain ; Z3 a (Pan je Schicht, Stereo XY an der Shape-Schicht, bewegtes Stereo bei den Effekten), Z4 a (Harmonics läuft weich, dazu Harmonics envelope), Z5 a (mehrere Konturen als Schichten oder als ein Pfad), die Form als Datei `.lvshape`, auch für die Visuals; Start und Reverse als Regler — nichts entworfen, nichts gebaut; offen Dateiformat, Konturerkennung, SVG, die Module, die Skizze. §7 NEU Idee Patrik: **Flanger** als Effekt am Clip — nur notiert |
| 1.190.0 | 2026-10-13 | Session 119. **X5 gebaut** (Clip effects, mehrere Clips mit derselben Folge): weicht irgendein Wert eines Bausteins ab, steht im Kasten „≠" vor dem Kurzwert, der Kasten ist bernsteinfarben getönt (die Fläche statt des Rands, Vorschlag Patrik — auch am gewählten Kasten zu sehen), der Tooltip nennt die Regler — im Programm ungesehen (Liste 10, X7, U76) |
| 1.189.0 | 2026-10-13 | Session 119. §7 „ein Sample im Creator bearbeiten": **Stufe 17 fertig** (17c: Benutzerhandbuch 1.148.0, Liste 10 Abschnitt D mit D0 bis D10 und U72 bis U75, voller Lauf 1153 / 1153 unter Testing-MSVC) — im Programm ungesehen. **X5 entschieden (Patrik, a):** weicht irgendein Wert eines Bausteins zwischen den Clips ab, steht die zweite Zeile des Kastens in Bernstein mit „≠" davor, der Tooltip nennt die Regler; dazu Patriks Frage nach einem farbigen Rahmen — **offen, nicht gebaut** |
| 1.188.0 | 2026-10-13 | Session 119. §7 „ein Sample im Creator bearbeiten": **17b gebaut** (Edit... im Creator) — im Programm ungesehen; offen 17c. **Befund Patrik zu X5, mit drei Bildern (Clip effects, zwei Clips mit derselben Reihe):** weicht ein Wert ab, der im Kasten steht (Time, Mix), zeigt der Kasten „mixed"; weicht nur einer ab, der dort nicht steht (Damping, Feedback), sieht man es erst, wenn der Baustein gewählt ist. Vorschlag Patrik: eine andere Schriftfarbe im Kasten, damit es gleich zu erkennen ist — **offen, noch nicht entworfen** |
| 1.187.0 | 2026-10-13 | Session 119. §7 „ein Sample im Creator bearbeiten": Baustufen freigegeben (B2 a), **17a gebaut** (das Fenster Record sample öffnet auch ein bestehendes Sample; Save und Save as... statt Namensfeld) — im Programm ungesehen. **Offen:** 17b, 17c; aus dem Sichttest vom 13.10.: X5 nicht bestanden (Befund Patrik, noch nicht besprochen) |
| 1.186.0 | 2026-10-13 | Session 119. §7 Wunsch „ein Sample im Creator bearbeiten": entworfen und entschieden (`Loop_Konzept.md` 0.65.0, §4k — M1 a, M2 b, M2b a, M3 a, M4 a, Skizze gebilligt). **Offen:** Patriks Freigabe der Baustufen 17a bis 17c; nichts gebaut |
| 1.185.0 | 2026-10-13 | Session 118, Abschluss. `sessions/Changelog_Session118.md`. **Stand:** Stufe 16 (Signal path) ist ganz gebaut — Modell und Dateien, Klangerzeuger, Creator, Clip effects, Drums; voller Lauf 1152 / 1152 unter Testing-MSVC. **Offen:** alles aus S118 im Programm ungesehen und ungehört (Liste 10: G1, G2, W1 bis W7, X1 bis X6, F3, F4, U66 bis U71); Patriks Ohr an `hoerproben_16e`; die Hi-Hat-Schwingung; der Wunsch zum Bearbeiten eines Samples (§7) |
| 1.184.0 | 2026-10-13 | Session 118. Loop, Signal path **Stufe 16e gebaut, Stufe 16 fertig:** die gerechneten Drums Kick und Clap nutzen die Reihe — Kick mit Drive hinter der Envelope, Clap mit High-pass und Band-pass hintereinander (`.lvinst` Fassung 7); gegen das Kit gemessen: Kick 0,9 / 1,6 dB in den Bändern (vorher 4,9 / 15,2), Clap 1,1 / 3,5 dB (vorher 4,7 / 12,5) — `Loop_Konzept.md` §4j, Tabelle. Die festgehaltenen Werte für beide neu geschrieben (6 von 199). **Offen:** Patriks Ohr an `.claude/handover/hoerproben_16e/` (`vergleich_<klang>.wav`, `alt_neu_kick.wav`, `alt_neu_clap.wav`) — U64, U65, U71; die Hi-Hat-Schwingung; alles aus Stufe 16 im Programm ungesehen und ungehört (Liste 10: G1, G2, W1 bis W7, X1 bis X6, U66 bis U71) |
| 1.183.0 | 2026-10-13 | Session 118. Loop, Signal path: Entscheid Patrik K5 a (mehrere markierte Clips mit verschiedenen Reihen — die Zeile zeigt einen, gestellt werden die mit derselben Folge; Use this path for all gleicht an). **Stufe 16d gebaut:** das Fenster Clip effects zeigt die Zeile **Signal path** — Clip, die Bausteine, + Add, Out; jeder Effekt beliebig oft und in freier Folge, **Filter** als vierter Baustein mit Cutoff in Hertz; Showing und der Hinweis bei verschiedenen Reihen (`Loop_Konzept.md` §4j). Das Bild aus dem Test angesehen, im Programm ungesehen und ungehört. **Offen:** Patriks Blick (Liste 10, X1 bis X6, Urteile U69, U70); 16e (Drums nachziehen, `werte/loop_werte.txt` neu schreiben, voller Lauf) |
| 1.182.0 | 2026-10-13 | Session 118. §7 NEU Wunsch Patrik: im Creator ein Knopf zum **Bearbeiten eines bestehenden Samples** neben File... und Record... (wie nach einer Aufnahme, dann speichern; mitgelieferte Samples schreibgeschützt, nur unter neuem Namen bei den eigenen) — nur notiert, für später |
| 1.181.0 | 2026-10-13 | Session 118. Loop, Signal path: Skizze des Creators gebilligt (Entscheid Patrik S1 a). **Stufe 16c gebaut:** der Creator zeigt je Schicht die Zeile **Signal path** — Source, die Bausteine Filter, Drive, Envelope, + Add, Level; ein Klick auf einen Kasten zeigt seine Regler, Ziehen und ← Move / Move → ordnen um, Remove entfernt (`Loop_Konzept.md` §4j). Die Gruppen Envelope und Filter entfallen. Das Bild aus dem Test angesehen, im Programm ungesehen. **Offen:** Patriks Blick (Liste 10, W1 bis W7, Urteile U66 bis U68); 16d (davor K5), 16e; der volle Lauf am Ende der Stufe 16 |
| 1.180.0 | 2026-10-13 | Session 118. Loop, Signal path **Stufe 16b gebaut:** der Klangerzeuger rechnet die Reihe an Schicht und Clip in freier Folge, der Filter am Clip in Hertz (`Loop_Konzept.md` §4j). Beleg: 199 vor dem Umbau festgehaltene Einträge (165 Töne aus 55 Instrumenten, 34 Loops) rechnen Bit für Bit gleich, unter Testing-MSVC und Release-clang; Last gemessen (acht Reverb an einem Clip 0,75 % eines Kerns im Release-Bau, 4,5 % im Testing-Bau). Im Programm ist davon nichts zu erreichen, bis 16c und 16d die Reihe zeigen. **Offen:** 16c (Skizze vorher noch einmal zeigen), 16d (davor K5), 16e — dort `werte/loop_werte.txt` neu schreiben; der volle Lauf am Ende der Stufe 16 |
| 1.179.0 | 2026-10-13 | Session 118. Loop, Signal path: Baustufen freigegeben (Entscheid Patrik B1 a — in der vorgelegten Reihenfolge). **Stufe 16a gebaut:** Schicht und Clip tragen eine Reihe von Bausteinen statt der festen Felder, `.lvinst` Fassung 7, `.lvloop` Fassung 5 (`Loop_Konzept.md` §4j); jede mitgelieferte Datei liest zu der Reihe, die sie fest hatte. Klangerzeuger, Creator und Clip effects arbeiten bis 16b, 16c, 16d mit dem ersten Baustein je Art — am Programm ist nichts neu zu sehen. **Offen:** 16b (vorher die gerechneten Werte aller Instrumente und Beispiel-Loops festhalten), 16c (Skizze vorher noch einmal zeigen), 16d (davor K5), 16e; der volle Lauf steht bis zum Ende der Stufe 16 aus; im Programm ungesehen (Liste 10, G1, G2) |
| 1.178.0 | 2026-10-13 | Session 117. Loop: die Vorlage zur Kette für Filter und Effekte ist entschieden — **Signal path**, `Loop_Konzept.md` §4j, K1 a (eine Reihe von Bausteinen), K2 a (Schicht: Filter, Drive, Envelope), K3 a (Clip: Filter in Hertz, Distortion, Echo, Reverb), K4 a (höchstens acht). **Offen:** Patriks Freigabe der Baustufen 16a bis 16e; K5 (mehrere markierte Clips mit verschiedenen Reihen); die Skizze vor 16c noch einmal zeigen. Nichts gebaut |
| 1.177.0 | 2026-10-12 | Session 117. Loop **Stufe 11f gebaut:** sechs gerechnete Drum-Instrumente unter Drums (`asset/loop/make_drums.py`), für die Note C2, gegen das Kit gemessen (`Loop_Konzept.md` §4h, Tabelle), leiser als das Kit (Spitze 0,42 bis 0,58 gegen 0,9) — im Programm nicht gesehen und nicht gehört. **Offen:** Patriks Ohr an `.claude/handover/hoerproben_11f/vergleich_*.wav` (dazu die aus 11d, 11e und `hoerproben_drumkit`) — daran hängt, ob Drive hinter der Hüllkurve (Kick), zwei Filter (Clap) und die Hi-Hat-Schwingung gebaut werden; ob die gerechneten so laut sein sollen wie das Kit (ginge nur mit einer Änderung am Klangerzeuger); die Vorlage zur Kette für Filter und Effekte |
| 1.176.0 | 2026-10-12 | Session 117. Loop, Instrument-Editor: **Load instrument...** fragt, ob die Datei zu den eigenen kopiert wird oder am Ort bleibt (E3 a) — im Programm nicht gesehen; E2 a erledigt (die drei losen `.lvinst` liegen unter `instruments`) |
| 1.175.0 | 2026-10-12 | Session 117. Loop, Instrument-Creator: **Save as...** fragt nur nach dem Namen und schreibt nach `instruments` (E1 a), ein Original wird nie überschrieben (E1b c) — im Programm nicht gesehen; offen E2 (drei lose `.lvinst` in der Wurzel des Benutzerordners) und E3 (Startordner von Load instrument...) |
| 1.174.0 | 2026-10-12 | Abschluss Session 116 — **Loop: Stufe 11c fertig, 11d und 11e gebaut** (Drum-Spur mit Klangliste, „…“ und Edit sound...; im Creator Noise, Pitch sweep, der Filter mit Type und Resonance), dazu auf Wunsch Patrik: Gruppen einklappen, Repeat note mit „every … beats“, die Suchordner für AVS und MilkDrop; `.lvinst` in Fassung 6; „Favorites“ überall; behoben: der Creator wuchs über den Bildschirm; voller Lauf 1140 / 1140; im Programm gesehen nur das Einklappen — lokale Liste 10, Abschnitte T, N, H; offen **11f** (nächste Sitzung), die Hörproben zu 11d und 11e, was bei MilkDrop zeitweise nichts zeigte, die Vorlage zur Kette für Filter und Effekte; Changelog `sessions/Changelog_Session116.md` |
| 1.173.0 | 2026-10-12 | Session 116 — **Suchordner für Bilder und Texturen als Liste** (Wunsch Patrik): Settings „Image and Texture Search Folders“ mit Add..., Remove, ▲ ▼; neu `core/Suchordner.hpp` (`import/searchDirs`), der AVS-Import (Picture, Texer, Texer II, AVI) und MilkDrop (Texturen, Sprites) suchen dort, wenn beim Preset nichts liegt; voller Lauf 1140 / 1140; ⬜ im Programm ungesehen. **Anlass, untersucht und nicht als Fehler im Code bestätigt:** Patrik meldete, AVS sähen zum Teil anders aus und MilkDrop zeige meist nichts (später: MilkDrop wieder gut). Befunde: seit dem 9.10. hat außer `MilkdropVisualizer.cpp` (S106) kein Commit Render-Code geändert, zwischen heute 13:03 (`4f4a95f7`) und jetzt keiner; die Kopien neben der Exe (Datenablage, S114) sind Datei für Datei gleich dem Repo; 20 MilkDrop-Presets im Einzelprogramm: keines schwarz, kein GL-Fehler; die AVS-Presets unter `asset\avs\Community Picks` nennen Texer-Bilder, die nur in `VisualsPresets\avs` liegen (82 Bilder), ein Suchordner war nicht gesetzt; „AVS Import Render Scale“ steht seit dem 5.10. auf window / 4. 🔧 offen: was genau bei MilkDrop zeitweise nichts zeigte — nicht nachgestellt |
| 1.172.0 | 2026-10-12 | Session 116 — im Instrument-Creator **Repeat note gebaut** (Wunsch Patrik): der angeklickte Ton spielt immer wieder, die Taste ist dabei hervorgehoben; daneben **every 1 / 2 / 4 / 8 / 16 beats** (sein Zusatz: „bei delay usw will man etwas mehr hören") — so lang darf der Ton klingen; im Programm ungesehen und ungehört — lokale Liste 10, H9 und H10; 🔧 offen: beim Anspielen aus dem Creator sind die Effekte am Clip (Echo, Hall) nicht zu hören — er spielt ein Instrument an, keinen Clip; als Nächstes 11f |
| 1.171.1 | 2026-10-12 | Session 116 — **Befund Patrik behoben** (im Programm gesehen): der Instrument-Creator wuchs mit jeder aufgeklappten Gruppe, bis über die Höhe des Bildschirms; die Regler der Schicht stehen jetzt in einem Feld mit Rollbalken, das Fenster behält seine Größe; das Mausrad über einem Regler rollt das Feld; ⬜ im Programm nachsehen (Liste 10, H1 und H8); 🔧 Wunsch Patrik, als Nächstes: im Creator ein Haken, mit dem ein Ton periodisch spielt — die Tonhöhe wählt man auf der Klaviatur, die Taste ist dabei hervorgehoben |
| 1.171.0 | 2026-10-12 | Session 116 — Loop **Stufe 11e gebaut**: der Filter einer Schicht mit **Type** (Low-pass, High-pass, Band-pass) und **Resonance** an allen drei Arten (Entscheid Patrik H1 b), `.lvinst` in Fassung 6; dazu auf Wunsch Patrik: die Gruppen Oscillator, Envelope, Filter und Vibrato des Creators lassen sich **ein- und ausklappen**, die Stellung wird gemerkt; voller Lauf 1137 / 1137; **im Programm ungesehen und ungehört** — lokale Liste 10, Abschnitt H, und sechs Hörproben unter `.claude/handover/hoerproben_11e/`; 🔧 der Tiefpass wechselt zwischen Resonance 0 und 1 % die Rechnung (ein kleiner Sprung in der Helligkeit, vorgelegt); im Creator heißen jetzt zwei Felder „Type“ (die Art des Instruments, die Art des Filters); offen 11f (sechs gerechnete Drum-Instrumente), die Hörproben zum Kit, die Vorlage zur Kette für Filter und Effekte |
| 1.170.0 | 2026-10-12 | Session 116 — Loop **Stufe 11d gebaut**: im Instrument-Creator die Schwingung **Noise** und die Regler **Pitch sweep** (die Note fällt oder steigt auf ihre Tonhöhe, −24 bis +48 st) und **Sweep time**, auch für eine Sample-Schicht; `.lvinst` in Fassung 5; Entscheide Patrik N1 (beide Richtungen) und N2 a; voller Lauf 1136 / 1136; **im Programm ungesehen und ungehört** — lokale Liste 10, Abschnitt N, und fünf Hörproben unter `.claude/handover/hoerproben_11d/`; nicht angefasst: Decay in Schritten von 10 ms (der Klick der Kick lässt sich am Regler nicht genau stellen); offen 11e (Hochpass, Bandpass), 11f, die Hörproben zum Kit, die Vorlage zur Kette für Filter und Effekte |
| 1.169.0 | 2026-10-12 | Session 116 — Loop **11c Schritt 3 gebaut, 11c damit gebaut**: **Edit sound...** im Rechtsklick auf eine Drum-Spur (aus Kit-Klang, eigenem Sample oder Tondatei wird ein Instrument der Art Drums mit einer Sample-Schicht; am Loop ändert sich nichts vor Save; danach Note auf dem Root note, Length so lang wie die Aufnahme); das Fenster Instruments stellt sich aus einer Drum-Spur auf Drums — **zu sehen erst mit 11f**, heute steht unter Drums nichts; voller Lauf 1134 / 1134; **im Programm ungesehen und ungehört** — lokale Liste 10, Abschnitt T (T12 bis T15, U54); offen die Hörproben zum Kit, die Vorlage zur Kette für Filter und Effekte, 11d bis 11f |
| 1.168.0 | 2026-10-12 | Session 116 — Loop **11c Schritt 2 gebaut**: im Drum-Editor die Klangliste (Kit, My samples, Instrument..., Sample file... mit der Frage aus S2 a, Record sample...) und an einer Instrument-Spur der Knopf „…“ mit Note, Length, Play, Browse..., Edit instrument...; **im Programm ungesehen und ungehört** — lokale Liste 10, Abschnitt T; offen 11c Schritt 3 (Edit sound..., das Fenster Instruments auf Drums gestellt), die Hörproben zum Kit, die Vorlage zur Kette für Filter und Effekte, 11d bis 11f |
| 1.167.0 | 2026-10-12 | Session 116 — **„Favorites“ angeglichen** (Entscheid Patrik 12.10.): im Fenster Instruments heißt der Knopf „★“ und die Überschrift „★ Favorites“, wie in der Effect Palette; Effect Palette und Preset-Zeile schreiben „Favorites“ statt „Favourites“; im Programm ungesehen |
| 1.166.0 | 2026-10-12 | Abschluss Session 115 — **Stufe 11 des Loops:** 11a (Sample als Schicht, Detect, Reference tone) und 11b (Record sample) gebaut, 11c begonnen (Schritt 1, der Kern der Drum-Spur mit drei Arten von Klang); `.lvinst` und `.lvloop` in Fassung 4; davor Befund P5 behoben (Ctrl+Z im Fenster Instruments); alles aus Stufe 11 im Programm ungesehen und ungehört — lokale Liste 10, Abschnitte Q und R; voller Lauf 1132 / 1132; offen 11c Schritt 2 und 3, die Hörproben zum Kit, die Vorlage zur Kette für Filter und Effekte, 11d bis 11f; 🔧 Frage Patrik: der Knopf für Favoriten im Fenster Instruments trägt Text statt des Sterns der Effect Palette — angleichen; entschieden: überall „Favorites“ (die Effect Palette schreibt heute „Favourites“); Changelog `sessions/Changelog_Session115.md` |
| 1.165.0 | 2026-10-12 | Session 115 — Loop **11c Schritt 1 gebaut**: der Kern ohne Fenster — eine Drum-Spur nennt einen Klang des Kits, ein eigenes Sample (`user:<name>`) oder ein Instrument mit Note und Length, `.lvloop` Fassung 4, `kit:` in der Sample-Schicht, die Art Drums; **GEÄNDERT:** eine Sample-Schicht spielt die Aufnahme in ihrer Lautstärke (nach 11a halb so laut); offen 11c Schritt 2 (Drum-Editor) und 3 (Edit sound...); 🔧 vorgemerkt auf Wunsch Patrik: Filter je Schicht und Effekte am Clip als Kette, „in einer Art Node-Editor“ — Vorlage mit Skizze nach 11c |
| 1.164.0 | 2026-10-12 | Session 115 — Loop, Stufe 11c ff.: **Entscheide K1 a** (Drum-Spur nennt Kit-Klang, eigenes Sample oder Instrument; Edit sound... macht aus jedem Klang ein Instrument; dazu als Plan 11d Rauschen und fallende Tonhöhe, 11e Hochpass und Bandpass, 11f sechs gerechnete Drum-Instrumente) und **K2 a** (Note und Length in ms); **Rechenversuch:** Tom und Snare lassen sich damit vollständig bauen, für Kick (Drive hinter der Hüllkurve), Clap (zwei Filter hintereinander) und Hi-Hats (sechs Rechtecke in einer Schicht) fehlt je ein weiterer Baustein — ⬜ Patrik hört die Hörproben unter `.claude/handover/hoerproben_drumkit/`; die Skizze der Klangliste ist noch nicht gebilligt; nichts gebaut |
| 1.163.0 | 2026-10-12 | Session 115 — Loop **11b Schritt 3 gebaut**: **Record...** im Instrument-Creator öffnet das Fenster „Record sample“, Save and use setzt die Aufnahme in die Sample-Schicht; **11b ist damit gebaut** und im Programm erreichbar, **nicht gesehen und nicht gehört** — ⬜ lokale Liste 10, Abschnitt R (R0 bis R9); voller Testlauf 1128 / 1128; offen 11c (Drum-Editor) mit dem Entscheid davor (Drum-Ausbau A, B, C) |
| 1.162.0 | 2026-10-12 | Session 115 — Loop **11b Schritt 2 gebaut**: das Fenster „Record sample“ (`loop/AufnahmeFenster`) nach der gebilligten Skizze — Input und Pegel, Record und Stop, das Bild mit zwei Griffen, Play mit Repeat, Trim silence, Normalize, Name, Save and use; **noch öffnet es niemand** — Schritt 3 (Record... im Creator) steht aus; nicht geprüft: das Fenster mit der echten Engine |
| 1.161.0 | 2026-10-12 | Session 115 — Loop **11b Schritt 1 gebaut**: der Kern ohne Fenster — die Aufnahme in `IAudioEngine` und `BassEngine` (Eingänge, starten, Pegel, stoppen; endet von selbst), neu `loop/Aufnahme` (Stille an den Rändern, ausschneiden mit Blende, Normalize, 16-Bit-WAV, speichern unter den eigenen Samples); im Programm nichts zu sehen; offen Schritt 2 (das Fenster) und 3 (Record... im Creator) |
| 1.160.0 | 2026-10-12 | Session 115 — Loop, **Beleg vor 11b**: BASS nimmt neben der laufenden Engine auf (48000 Hz Stereo float, Blöcke von 100 ms), drei Eingänge an Patriks Rechner, darunter der Mitschnitt der Ausgabe; `test_LoopBelegeAufnahme.cpp`, der aufnehmende Fall nur mit `LUMIVIZ_AUFNAHME_BELEG=1`; gebaut ist an 11b nichts |
| 1.159.0 | 2026-10-12 | Session 115 — Loop **11a Schritt 3 gebaut**: Detect mit Apply (neu `loop/Grundton`, misst den Grundton einer Aufnahme) und Reference tone im Instrument-Creator; **11a ist damit gebaut**, im Programm nicht gesehen und nicht gehört — ⬜ lokale Liste 10, Abschnitt Q (Q0 bis Q12); nicht geprüft: Detect an einer echten Aufnahme; offen 11b (Record sample, Beleg-Test davor) und 11c (Drum-Editor, Entscheid davor) |
| 1.158.0 | 2026-10-12 | Session 115 — Loop **11a Schritt 2 gebaut**: der Instrument-Creator mit Waveform: Sample (File..., die Frage aus S2 a, Root note, Fine tune, Sample start, Sustain loop, das Bild der Aufnahme mit ziehbaren Marken), Settings → Panels: Chosen Sample Files; **im Programm nicht gesehen und nicht gehört** — ⬜ lokale Liste 10, Abschnitt Q (Q0 bis Q9); vorgemerkt zu den Drums (Patrik fragt, ob Drum-Editor und Creator eins werden): Drum-Ausbau A (Drum-Spur nennt ein Instrument), B (Rauschen, fallende Tonhöhe), C (Hochpass, Bandpass) — Entscheid vor 11c |
| 1.157.0 | 2026-10-12 | Session 115 — Loop **Stufe 11 begonnen**: Skizze des Creators mit einer Sample-Schicht und Skizze „Record sample" gebilligt, Entscheide S1 a (Ordner `samples`, `user:<name>`) und S2 a (File... fragt: kopieren oder am Ort lassen); **11a Schritt 1 gebaut** — der Kern ohne Fenster (`loop/Sample`, Sample als Schicht, `.lvinst` Fassung 4, Bereich „Samples" im Export); offen: 11a Schritt 2 und 3, 11b, 11c mit Entscheid zum Drum-Editor (Patrik: dem Creator anpassen oder eins werden); im Programm nichts zu sehen; Einzelheiten `Loop_Konzept.md` §4h |
| 1.156.0 | 2026-10-12 | Session 115 — von Patrik im Programm geprüft: Datenablage D1 und D3 ok, Liste 10 P0 bis P3 ok; **Befund P5** (Ctrl+Z kam im Fenster Instruments nicht an), behoben nach Entscheid Z1 a, P5 danach neu geprüft: ok; offen daneben: Ctrl+Z im Instrument-Creator |
| 1.155.0 | 2026-10-12 | Abschluss Session 114 — **Datenablage gebaut** (A1 bis A4b, A3): Mitgeliefertes neben der Exe über `asset/Asset.cmake`, Export und Import der eigenen Daten als `.lvdata` mit miniz; **Stufe 14 des Loops gebaut** (14a bis 14c): `.lvinst` in Fassung 3, eigene Instrumente als `user:<name>`, das Fenster Instruments mit Browse...; alles im Programm ungesehen — lokale Listen `Sichttest_Datenablage.md` und Liste 10 Abschnitt P; Befund `/MD` in der Konfiguration Testing; vorgemerkt: Schalter aus P2 c, Beispiele neben die Exe, Icons, mehr als vier Schichten |
| 1.154.0 | 2026-10-12 | Session 114 — Loop: M0 bis M4, M12, M13 der Liste 10 von Patrik geprüft (ok); neu der Block **Datenablage** (Steuerdokument `core-services/Datenablage_Konzept.md`, Entscheide D1 b bis D10): Stufe A1 (eine Stelle für mitgelieferte Bereiche, Schalter in den Settings) und A2 (`asset/Asset.cmake`, Abgleich bei jedem Bau) gebaut, A3 (eigene Instrumente) und A4 (Export und Import) offen; der Kleinkram „Kit und Instrumente fehlen neben der Exe" ist damit erledigt; Stufe 14 des Loops wartet auf A4 (D1 b), F3 b notiert |
| 1.153.0 | 2026-10-11 | Session 113 — §6 Loop: Stufe 12 (Effekte am Clip; E2 b, F2 a, G1 b) und Stufe 13 (Mehrfachauswahl in den Editoren; V1 a, V2 a, V3 a, der Haken Right-click deletes) gebaut; von Stufe 12 hat Patrik F0 bis F4 geprüft, Stufe 13 ist ungesehen; neu vorgemerkt Stufe 11 (Samples als Schicht, Multisamples), 14 (Palette der Instrumente, P1 a), 15 (Wave-Clips und Autotune, zurückgestellt); ein voller Testlauf brach einmal bei den Composer-Fällen „S102 Zeitachse" ab, Ursache nicht untersucht |
| 1.152.0 | 2026-10-11 | Session 112 — §6 Sichttest-Steuerung: `cmakecraft.pin` v0.11.1; ein neuer Wackler in `test_PaletteUi.cpp:1609` (A10/A11, einmal `3 == 2`, Ursache nicht untersucht) |
| 1.141.0 | 2026-10-11 | Session 109, Abschluss — §6 **Loop:** die Stufen 6a bis 8 gebaut (gemeinsame Basis `Achse`, Loop-Panel als Arrangement, Drum-Editor, Audio Browser, Loop als Clip im Composer mit Tags, Benutzerhandbuch §16, `Audio_System.md` §6a, Sichttest-Vorlage); Entscheide W1 b, T1 a, T2; offen der erste Blick ins Programm (Liste `Sichttest_Composer_10_Loop.md`, 56 Punkte), die gesetzten Regeln als Urteile, die Rückfrage beim Beenden, das Nachsetzen bei gedrosseltem Rechner, Aktionen der Steuerung für das Panel Loop; 1025 / 1025; im Programm ist seit S108 nichts davon gesehen oder gehört; der Sichttest mit dem Tester hat auch in S109 nicht stattgefunden. Verweis auf das Benutzerhandbuch §17 auf §18 nachgezogen |
| 1.140.0 | 2026-10-11 | Session 108, Abschluss — §6 neu: **Loop** (Wunsch Patrik), im Bau nach [Loop_Konzept.md](audio/Loop_Konzept.md); gebaut Beleg, Modell und Datei `.lvloop` in Fassung 2 (Arrangement aus Kanälen und Clips), Rechenkern, gerechnetes Kit, Naht in der Audio-Engine, Klangerzeuger, Spieler mit Rangfolge, erstes Panel; offen 6a bis 9, der erste Hörtest, das Kit im Release-Paket; 1002 / 1002; der Sichttest mit dem Tester hat auch in S108 nicht stattgefunden |
| 1.139.0 | 2026-10-10 | Session 107, Abschluss — Block „Stand S105": `sichttest.pin` steht auf v0.3.4 (Bezug, zwei Bauarten, 974 / 974, Probe der DLL verbunden; im Programm ungesehen); der Sichttest mit dem Tester hat auch in S107 nicht stattgefunden |
| 1.138.0 | 2026-10-10 | Session 107 — §3 *MilkDrop-Presets stocken manchmal*: geschlossen, Ursache außerhalb der App — es drosselt der Temperaturschutz des Prozessors am Entwicklungsrechner (Tctl an 100 °C, danach der Kerntakt für 1 bis 3 s auf rund ein Zehntel, alle 10 bis 11 s); in einem Lauf mit *glassworks 3* liegen sechs Einbrüche und 108 lange Bilder in denselben Sekunden; an der App nichts zu tun, offen bleibt nur Stufe 3 |
| 1.137.0 | 2026-10-10 | Session 106 — §3: *glassworks 3* in zwei Kanälen gemessen (neuer Stand) — ein Kanal 59,0 bis 59,4, zwei Kanäle 33,5 bis 45,4 Bilder je Sekunde (Mittel 21,5 bis 29,1 ms je Bild, je Instanz rund 10 ms); zwei solche Presets halten auch nach dem Umbau keine 60; vor dem Umbau nicht gemessen |
| 1.136.0 | 2026-10-10 | Session 106 — §3: MilkDrop-Shapes billiger, Stufe 1 und 2 gebaut (Flächen ohne Textur und Rand in einem Zeichenaufruf; Randfarbe einmal je Instanz) — 3856 Bild-Hashes aus 16 Presets alt gegen neu gleich, *glassworks 3* 10,0 bis 11,4 ms je Bild statt 13,6; Stufe 3 (Übergabe ans Skript) zurückgestellt, Entscheid Patrik; das Stocken bleibt eine Drosselung des Rechners |
| 1.135.0 | 2026-10-10 | Session 106 — §3 *MilkDrop-Presets stocken manchmal*: Messung im Renderer (neue Trace-Zeile `langesBild`, Schwelle über `LUMIVIZ_MILKDROP_LANG_MS`) — im Einbruch werden alle Stufen gleich stark langsamer, der Thread rechnet durch, und ein unabhängiger Prozess wird in denselben Sekunden 7- bis 12-mal langsamer: der Prozessor wird für rund 3 s gedrosselt, die App bremst nicht selbst; wer drosselt, ist nicht belegt; offen, ob *glassworks 3* billiger werden kann (1350 Shape-Instanzen) |
| 1.134.0 | 2026-10-10 | Session 106 — §3 *MilkDrop-Presets stocken manchmal*: sechs weitere Kurzläufe ohne Last (Start 6:00 und 5:50) ohne Einbruch — die Trennung „Stelle der Musik" gegen „Abstand zum Start" ist nicht gelungen; über den Tag 2 von 9 Kurzläufen ohne Last mit Einbruch, beide unmittelbar nach einem Lauf mit Last; weiter nur mit einer Messung im Renderer |
| 1.133.0 | 2026-10-10 | Session 106 — §3 *MilkDrop-Presets stocken manchmal*: zwei Läufe von Patrik (mit und ohne Composer, 83 s und rund 140 s) ohne Einbruch; acht Kurzläufe bei 6:00 unter Last von außen — der Einbruch kommt in zwei von drei Läufen **ohne** Last, CPU- und GPU-Last bremsen dagegen gleichmäßig; *glassworks 3* kostet für sich 12,8 ms je Bild; offen, ob es an der Stelle der Musik oder an den Zufallswerten des Presets hängt |
| 1.132.0 | 2026-10-10 | Session 106 — §3 *MilkDrop-Presets stocken manchmal*: das Log der zehn Läufe genauer gelesen — der Einbruch ist kurz (rund 2 bis 4 s, etwa 8 Bilder je Sekunde, Beginn 7,8 bis 8,6 s nach der Wiederaufnahme), die Zeit liegt im `render()` der Bildquelle, bei *glassworks 3* lief nur ein Renderer; zweites Muster: ein einzelnes langes Bild rund 5 s nach der Wiederaufnahme in den acht übrigen Läufen; Ursache weiter offen, Reihenfolge der Messung vorgeschlagen. Sichttest-Listen 8 B0 und 9 F0 nennen „Composer on" (Entscheid Patrik V1: a) |
| 1.131.0 | 2026-10-10 | Session 105, Abschluss — **Release v0.12.0**: Version der App 0.12.0, Marke auf `f2faeee`, Paket ohne Tester geschnürt und aus dem Zip gestartet; der Tester steht nicht mehr im Benutzerhandbuch, sondern in `Sichttest_Steuerung.md`; die Settings-Zeile *Sichttest* nur mit Steuerung neben der Exe; README mit Composer, Karaoke und sechs Bildern |
| 1.130.0 | 2026-10-10 | Session 105, Nachtrag — §3 *MilkDrop-Presets stocken manchmal*: zweite Beobachtung Patrik beim Aufnehmen der README-Bilder, im Log zählbar — in drei von zehn Läufen 25 bis 33 lange Bilder (glassworks 3 an zwei Stellen; double chase + Gin Tonic), in den übrigen je 1 bis 2; Ursache weiter nicht untersucht |
| 1.129.0 | 2026-10-09 | Session 105, Nachtrag — **`composer` ist nach `master` überführt** (M1: a; Vorspulen auf `ec836c6`, Marke `vor-composer` auf `93d0be4`); gearbeitet wird auf `master`, die Rückbau-Regeln sind abgeschlossen (Composer_Konzept 0.81.0) |
| 1.128.0 | 2026-10-09 | Session 105, Nachtrag — `sichttest.pin` v0.3.3; die neun Listen tragen 156 `aktion:`-Stücke, jede Vorbereitung mit Vorlage setzt Edit, Loop und Sync on Effect Chain ausdrücklich; der gute Weg von `panel_zeigen` und `karaoke_laden` per Test belegt (Naht N26: `qt-ads` am Test-Target), **974 Cases**; Bauen: clang mit `--parallel 4`, wenn alle Tests neu übersetzen |
| 1.127.0 | 2026-10-09 | Session 105 — **Die Sichttest-Steuerung läuft:** `LumiViz.exe --testing`, fünfzehn Aktionen, Projektdatei, Verknüpfung „LumiViz.exe - Testing", Pins `sichttest.pin` v0.3.2 und `cmakecraft.pin` v0.11.0, Nähte N20 bis N25; die Kopie `projects/exec/Sichttest` entfernt; Entscheide Patrik L1 a, C1 a, LV2 a, P1 a; im Programm von Patrik gesehen: Liste 2 G0 und T0; was ungesehen und was offen ist, steht im Block. **Entscheid M1: a** — `composer` wird nach `master` überführt, die Rückbau-Regeln gelten danach als abgeschlossen (Git-Schritte bei Patrik). §3 neu: **MilkDrop-Presets stocken manchmal** (Beobachtung Patrik, vorgemerkt für später). Bauen: Release-clang braucht die Umgebung von Visual Studio 18; `test_PaletteUi.cpp:765` einmal in zwölf Läufen |
| 1.126.0 | 2026-10-09 | Session 104 — **Sichttest-Steuerung, Bezug:** External `sichttest` und `sichttest.pin` (Release `v0.2.0`), CMakeCraft v0.10.0, Naht N19, Commit `5d49259`; gebaut in drei Presets, benutzt wird das Paket noch nicht; Entscheide Patrik Z1 a, W1 a (vorerst), P1 a, P2 a (`.sichttest/sichttest.projekt.json`); drei Bau-Befunde (clang und `LumiViz.rc`, C1128 im Debug-Bau der Tests, Kopie nur beim Bau der Exe) |
| 1.125.0 | 2026-10-08 | Session 103 — **Sichttests neu gefasst:** Abgleich aller Listen S88 bis S102 (542 von 845 Punkten gelten noch); der Befund S100 M11 war ein Fehler der Beschreibung, kein Fehler am Programm; neu im Repo `asset/sichttest/` (elf Vorlagen, Presets, Testsignale, Songtext) und `test_ComposerVorlagen.cpp`; lokal neun Listen nach Thema (394 Punkte) für alle offenen Composer-Punkte aus S94b bis S102; offen die rund 246 Punkte außerhalb des Composers, Patriks Wort zu S95 und zwei alten Befunden; die Steuerung der App durch den Tester läuft als eigener Schritt über den Sync `sync_sichttest` (DLL als Gegenstelle); `test_PaletteUi.cpp:765` fiel in zwei von vier vollen Läufen um, Ursache nicht untersucht; 967 Cases; `Changelog_Session103.md` |
| 1.124.0 | 2026-10-07 | Session 102, Abschluss — **Composer:** Stand am Ende der Sitzung: Zoom und Rollen der Zeitachse in sechs Schritten gebaut (acht Commits, `ed32789` bis `8e01648`), 966 Cases, im Programm nichts gesehen — Sichttest S102 mit 64 Punkten, zuerst Abschnitt G (kommen die Gesten am Gerät an?); offen das Urteil U1 zur Schwelle der Kurvenpunkte; `Changelog_Session102.md` |
| 1.123.0 | 2026-10-07 | Session 102 — **Composer:** der Vergleichslauf nach den zwei Eingriffen an R2 (S101) entfällt (Entscheid Patrik VL1: a, mit Begründung: ungelesener Zähler, Faktor genau 1, die Harnesse rendern ohne Karaoke-Ebene); es bleibt der Vergleichslauf vor dem Holen nach `master` |
| 1.122.0 | 2026-10-07 | Session 102 — **Composer, Zoom Schritt 6:** Benutzerhandbuch 1.111.0 (§15 Zoomen und Rollen), Rückbau-Liste im Konzept 0.68.0, Sichttest-Liste S102; das Vorhaben Zoom ist gebaut, im Programm nichts gesehen |
| 1.121.0 | 2026-10-07 | Session 102 — **Composer, Zoom Schritt 5, die Naht N9 gebaut:** die Auswahl *Scroll with playhead* in Settings, Konzept 0.67.0; im Programm nicht gesehen |
| 1.120.0 | 2026-10-07 | Session 102 — **Composer, Zoom Schritt 5, das Eigene gebaut:** Mitlaufen mit der Abspielmarke (Schalter *Scroll with playhead*, stetig oder seitenweise), Konzept 0.66.0; offen die Auswahl in Settings (Naht N9); im Programm nicht gesehen |
| 1.119.0 | 2026-10-07 | Session 102 — **Composer, Zoom Schritt 4 gebaut:** Schwenken mit der mittleren Maustaste, der Knopf *Fit*, Ctrl+0 im Panel, *Zoom* im Rechtsklick-Menü, Konzept 0.65.0; im Programm nicht gesehen |
| 1.118.0 | 2026-10-07 | Session 102 — **Composer, Zoom Schritt 3 gebaut:** der Höhen-Zoom (Alt+Rad, Ctrl+Alt+Rad beides, 50 % bis 300 %, Master-Zeilen immer 60 %, die Höhe bleibt über den Neustart), Konzept 0.64.0; am Gerät zu prüfen: Alt+Rad und die Menüleiste; im Programm nicht gesehen |
| 1.117.0 | 2026-10-07 | Session 102 — **Composer, Zoom Schritt 2 gebaut:** der Zeit-Zoom (Ctrl+Rad um den Mauszeiger, Shift+Rad waagrecht, waagrechter Rollbalken, die Kanalnamen bleiben stehen), Konzept 0.63.0; im Programm nicht gesehen |
| 1.116.0 | 2026-10-07 | Session 102 — **Composer, Zoom und Rollen der Zeitachse:** neun Entscheide Patrik (ZO1 bis ZO9, Konzept 0.62.0 §7), Bauplan in sechs Schritten; **Schritt 1 gebaut** (die Zeitachse rollt senkrecht selbst, das Lineal bleibt stehen, Rollbalken im `ZeitachsenRahmen`), im Programm nicht gesehen; der Vergleichslauf am Renderer bleibt nachzuholen (Patrik: später) |
| 1.115.0 | 2026-10-07 | Session 101, Abschluss — **Composer:** Stand am Ende der Sitzung (alles aus S101 gebaut, im Programm weder gesehen noch gehört — Sichttest S101 mit 67 Punkten); Vorgabe Patrik für S102: Zoom, nicht nur in der Zeitachse — der Umfang wird zuerst geklärt |
| 1.114.0 | 2026-10-07 | Session 101 — **Composer:** Mute, Schritt 2c gebaut — der Mute-Knopf im Panel Player (Naht N18) und im Panel Composer, Profil per Rechtsklick, Dialog der Mute-Profile (Konzept 0.61.0, Benutzerhandbuch 1.110.0); im Programm ungesehen und ungehört, Sichttest in sechs Punkten eingetragen |
| 1.113.0 | 2026-10-07 | Session 101 — **Composer:** Mute, Schritt 2b gebaut (Entscheid Patrik AM3: c, Vorgabe „Visuals laufen weiter") — Naht N17 (Pegel am Player), `MuteSchalter`, der Composer folgt am Mixer, N13 wächst um den Faktor für die Visuals (Konzept 0.60.0); noch ohne Knopf, im Programm wirkt nichts; offen Entscheid AM4 (ein Mute-Knopf oder zwei im Panel Player) |
| 1.112.0 | 2026-10-07 | Session 101 — **Composer:** Karaoke im Composer gebaut (Entscheid Patrik KS1: a) — Zeile KARAOKE mit voller Kurve und der Haken Karaoke, gekoppelt mit dem Panel Karaoke; Naht R2 wächst um den Pegel beim Auflegen (Konzept 0.59.0, Benutzerhandbuch 1.109.0); im Programm ungesehen |
| 1.111.0 | 2026-10-07 | Session 101 — **Composer:** Mute, Schritt 2a gebaut (Mute-Profil, Verlauf, Profile als `.lvmute`; Konzept 0.58.0) — noch ohne Knopf und ohne Ton; Entscheid Patrik AM2: c (der Mute gehört dem Player, Knopf in beiden Panels); Befund: die Lautstärke des Mixers ändert sein Spektrum nicht; neuer Wunsch Patrik: Karaoke im Composer schalten, als Haken oder als Spur (Entscheid KS1 vorgelegt) |
| 1.110.0 | 2026-10-07 | Session 101 — **Composer:** Audio-Master und Visual-Master, Schritt 1 gebaut (Entscheid Patrik AM1: a — zwei volle Kurven, Zeilen VISUAL und AUDIO; Konzept 0.57.0, Benutzerhandbuch 1.108.0), im Programm ungesehen; Schritt 2 offen: Mute-Knopf mit Mute-Profilen (Vorgabe Patrik: Ausblend-Kurve, Einblend-Kurve, Zielpegel; speichern und laden), Fragen dazu eingetragen |
| 1.109.0 | 2026-10-07 | Session 101 — **Composer:** Sichttest S100, erster Teil ausgewertet (23 bestanden, 1 mit Bemerkung, 0 durchgefallen, 46 offen); Urteil M17 umgesetzt — die Zeile MASTER ist 60 % so hoch wie ein Kanal (Konzept 0.56.0), im Programm ungesehen; Vorbehalt: während des Tests liefen Builds und Testläufe |
| 1.108.0 | 2026-10-07 | Session 101 — **`LumiViz.exe <datei>`, Schritt 2:** eine laufende App je Exe-Datei (Entscheid Patrik X1b: a, mit `--new-instance`; `core/EinzelInstanz`, Benutzerhandbuch 1.107.0 §17, Konzept 0.55.0 — N8 zweite Stelle), im Programm ungesehen; Sichttest in sechs Punkten eingetragen; offen Audiodatei als Argument, Preset gegen wieder eingeschalteten Composer, Öffnen-Befehl im Skript |
| 1.107.0 | 2026-10-07 | Session 101 — **`LumiViz.exe <datei>`:** Entscheid Patrik X1: a (eine Instanz, die Datei geht an die laufende App); Schritt 1 gebaut — die Datei als Argument beim Start (`UI/StartDatei.hpp`, Naht N8 wächst um die `.lvcomp`; Konzept 0.54.0), im Programm ungesehen; offen die eine Instanz (Entscheid X1b), Audiodatei als Argument, Preset gegen wieder eingeschalteten Composer, Öffnen-Befehl im Skript |
| 1.106.0 | 2026-10-07 | Session 101 — **Composer:** Befund Patrik, das Panel Karaoke lud bei laufendem Composer nichts; behoben (Entscheid KP1: a, Naht N16, R2 wächst; Konzept 0.53.0), per Test gezählt, im Programm ungesehen — Nachtest und Urteil zum Ruckeln eingetragen |
| 1.105.0 | 2026-10-07 | Session 100, Abschluss — **Composer:** Stand am Ende der Sitzung (alles gebaut, im Programm nichts gesehen — Sichttest S100 mit 70 Punkten); neuer Wunsch Patrik: Audio-Master und Visual-Master neben der Zeile MASTER (etwa Musik stumm für eine Durchsage), nicht begonnen |
| 1.104.0 | 2026-10-07 | Session 100 — **Icons:** Icon für `.lvcomp` gebaut (Fächer mit Zeitstrahl; Effect Browser mit Doppelklick, Panel Composer; Naht N8 wächst, Konzept 0.52.0), Icon der App als Ressource der Exe; Sichttest S100 mit Abschnitt I, 70 Punkte. Von den Wünschen zur Zeitachse offen: Zoom und Schwenken |
| 1.103.0 | 2026-10-07 | Session 100 — **Icons:** Ziel Patrik: alle Datei-Icons im Explorer (vorerst Windows) — Liste der zugewiesenen Icons in `asset/img/logo/icons/README.md`, Skript `register_file_icons.ps1` (nur Anzeige-Modus gelaufen); offen: Icon der App (Exe-Ressource, Fenster-Icon), `LumiViz.exe <datei>`; zum Icon `.lvcomp` der Ort entschieden (I1: a), das Aussehen offen. Zoom der Zeitachse: Reset mit Ctrl+0 vorgemerkt |
| 1.102.0 | 2026-10-07 | Session 100 — **Composer:** Wunsch Sortieren nach Typ erledigt (Entscheid Patrik S1: a — nur nach Art, das Bild ändert sich nicht; Konzept 0.51.0); Sichttest S100 mit V7 und V8, 63 Punkte. Offen von den Wünschen zur Zeitachse: Zoom und Schwenken, Icon für `.lvcomp` |
| 1.101.0 | 2026-10-07 | Session 100 — **Composer:** Wunsch Kanäle verschieben erledigt (Entscheid Patrik V1: a — die Reihenfolge bleibt die Mischreihenfolge; Konzept 0.50.0), dazu das feste Ende an der Zeile MASTER ziehen (Konzept 0.49.0); Sichttest S100 mit Abschnitt V und M24, 61 Punkte |
| 1.100.0 | 2026-10-07 | Session 100 — **Composer:** Wunsch Kanalfarben erledigt (Entscheid Patrik K1: b) — zwei Farbreihen, die Farbe gehört dem Kanal, per Rechtsklick einstellbar (Konzept 0.48.0); Sichttest S100 mit Abschnitt K, 54 Punkte |
| 1.99.0 | 2026-10-07 | Session 100 — **Composer:** Befund „Flipped und einfache Einblendung" erledigt (Entscheid Patrik: a) — Stück-Preset Late fade in und Handbuch-Absatz, Rezept per Test belegt (Konzept 0.47.0); Sichttest S100 mit Abschnitt L, 49 Punkte |
| 1.98.0 | 2026-10-07 | Session 100 — **Composer:** Pegelkurve der ganzen Komposition, Schritt 2 — die Zeile MASTER unter dem Lineal mit Griffen, Rechtsklick und Curve Editor wie am Clip (Konzept 0.46.0, Benutzerhandbuch 1.99.0); im Programm nicht gesehen |
| 1.97.0 | 2026-10-07 | Session 100 — **Composer:** Pegelkurve der ganzen Komposition entschieden (Patrik: F7 c — alles, was ein Clip hat, in einer eigenen Spur; F7-1 a — ein Stück liegt an fester Zeit), Schritt 1 von 4 gebaut — Modell und Datei (Konzept 0.45.0). Im Panel noch nichts einstellbar; offen Curve Editor, Spur „MASTER", Handbuch |
| 1.96.0 | 2026-10-07 | Session 100 — **Composer:** Sichttest S99 ausgewertet (31 bestanden, 3 mit Bemerkung, 1 durchgefallen, 17 offen): Überlappung in der Schleife und Blende der ganzen Komposition im Programm gesehen und gehört, ohne Befund; was im Fenster von mir gesetzt war, ist gebilligt. Befund A2 (Panel Player blieb beim ersten Ausschalten auf dem Song der Komposition) per Test nachgestellt und behoben — Naht N5, Anzeige, sechste Stelle (Konzept 0.44.0); im Programm nicht gesehen. Neuer Wunsch F7: Blende der Komposition wie bei Clips, editierbar. Nicht gespielt: Abschnitt G, D1 bis D4, C2, C3, B5, A8, E1 |
| 1.95.0 | 2026-10-06 | Session 99, Abschluss — **Composer:** die Überlappung in der Schleife (Loop overlap, beide Fenster in der Zeitachse, Songs / Visuals overlap themselves) und die Blende der ganzen Komposition (Fade in / Fade out) in sieben Schritten gebaut (Konzept 0.43.1, Benutzerhandbuch 1.98.0); im Programm nichts davon gesehen oder gehört — Sichttest S99 mit 52 Punkten; nicht gemessen: Bildzeit und Grafikspeicher von *Visuals overlap themselves*. Weiter offen: Entscheid „Flipped und einfache Einblendung", vier Wünsche zur Zeitachse und ein Icon für Composer-Dateien. 939 Cases |
| 1.94.0 | 2026-10-06 | Session 99 — **Composer:** Überlappung in der Schleife entschieden (vier Entscheide Patrik), Schritt 1 von 7 gebaut — Modell und Datei (Konzept 0.37.0). §7 neu: Befund Patrik „Flipped und einfache Einblendung" (Zielbild bestätigt: unsichtbar einfahren, später einblenden, oben bleiben — Vorlage steht aus) und vier Wünsche zur Zeitachse (Kanalfarben einstellbar, Kanäle verschieben, sortieren nach Typ, Zoom und Schwenken) |
| 1.93.0 | 2026-10-06 | Session 99 — **Composer:** Lücke „Composer off ohne Titel im Player" geschlossen (Entscheid Patrik: a — leerer Songwechsel, Panel Player im Ruhezustand; Konzept 0.36.0, Benutzerhandbuch 1.95.0); §7: offen der Sichttest S99 (Rest der ersten Liste, Urteil Kreuzungspunkt, die geschlossene Lücke im Programm); Vorlauf und Schwelle gelten als im Hörtest bestätigt |
| 1.92.0 | 2026-10-06 | Session 99 — **Composer:** Patriks zweiter Test (`Sichttests_S98b`) ausgewertet — 33 von 33 bestanden, im Log kein Nachsetzen; §7 Block „Stand S98": gebilligte Vorgaben eingetragen (Weiterspielen, Einfügemarke, Kopie, Tooltip), Wellenform beim Kürzen erledigt (wird abgeschnitten); offen der Rest der ersten Liste (P13–P16, Panel Player, Urteile R), die Lücke im Panel Player, Vorlauf und Schwelle ohne Urteil (Konzept 0.35.0) |
| 1.91.0 | 2026-10-06 | Session 98 — **Composer, Stufe C5b: alle fünf Schritte gebaut, das Überlagern im Programm gehört** (Mixer, die Uhr führt, laufender Song, Panel Player „Composer - <Song>", Nähte N13, N14, N15, N5 Anzeige, N9 wächst; N7a zurückgenommen); Korrekturen nach dem Hörtest (Tag View, Nachsetzen, Weiterspielen einstellbar); Bedienung der Zeitachse (Einfügemarke, Kopieren / Einfügen, Schatten, Tooltip, Vorgabelänge). §7: neuer Block „Composer, Stand S98" mit dem, was offen ist — Patriks zweiter Test, Wellenform beim Kürzen, bekannte Lücke im Panel Player, gesetzte Vorgaben ohne Urteil |
| 1.90.0 | 2026-10-05 | Session 97 — **Composer, Stufe C5b (Audio-Kanäle überlagern):** vier Entscheide Patrik (Mixer in der Engine, Summe mit den Pegeln für die Visuals, laufender Song nach der Pegelkurve, Player-Panel zeigt die Komposition), Belege, Wechsel auf CMakeCraft v0.9.2 mit BASSmix als Pflichtpaket, Naht N12 gebaut (Konzept 0.28.1); §7 neuer Block „Stand S97" mit den offenen Schritten 2 bis 5, der vorgemerkten Überlagerung in der Schleife und dem Nebenbefund Lyrics Sync. Im Programm überlagern sich die Songs weiter nicht. 930 Cases |
| 1.89.0 | 2026-10-05 | Session 96 — **Composer:** C4 Teil 2 (Pegelform, Curve Editor, Stücke, Kurven-Presets) und C5 (Audiokanal mit den Nähten N7a, N7b, N5 Transport) gebaut (Konzept 0.26.1); §7 Stand und offene Punkte neu — zuerst der Befund Patrik: Audio-Kanäle überlagern sich nicht (ein Stream), nächster Schritt je Kanal ein eigener Stream; Nebenbefund: `test_PaletteUi.cpp:750` wackelt im vollen Lauf |
| 1.88.0 | 2026-10-05 | Session 95 — **Composer:** Stufen C2b, C3a, C3b, C3c und C4 Teil 1 gebaut (Konzept 0.16.1); §7 Stand und offene Punkte neu, nächster Schritt Punkte-Editor der Pegelkurve; Composer Variables als geplantes Modul. **§8 neu:** der Reiter Panels der Settings ist ein Sammelbecken (Anlass Movement Table — die Doku nennt Performance) |
| 1.87.0 | 2026-10-04 | Session 94 — **Composer:** Stufe C2a erledigt (Naht R3, Bildquelle, Naht N8, Messung; Konzept 0.9.2), Sichttest 34 von 42 mit drei behobenen Befunden, nächste Stufe C2b; offene Punkte aus C2a eingetragen. **Movement-Tabelle in Scheiben** als eigener Punkt (Einstellung Movement Table, offene Fragen) |
| 1.86.0 | 2026-10-04 | Session 93 — **Composer:** Konzept `ui/Composer_Konzept.md` (0.7.0, alle Entscheide gefallen, rückbaubar), Stufen C0 / C0b / C1a / C1b erledigt (C1a, C1b im Zweig `composer`), nächste Stufe C2a; **P2 Visual-Playlist geschlossen** (aufgegangen in Composer und Host Switcher); Notiz **Blättern und Timeline** (PgUp/PgDn blättern in der Liste des Browsers, Entscheid 2a; im Composer entschieden als C5). **Gebaut:** Kontextmenü im Kettenbaum (Milkdrop-Elemente, Add preset, Add module), Sprite mit Bilddatei, „+ New empty effect chain", Fix Start-Preset und Umzug ins Hauptfenster (`UI/StartPreset.hpp`), Knopf ◎ in Effect Browser und Audio Browser, Werkzeug `Sichttest`. **Sichttests S93** als Punkt 0 der Lyrics-Liste eingetragen (`Sichttests_S93.md`, A–I). Tests 866 (Zweig), 851 (`master`) |
| 1.85.0 | 2026-10-03 | Session 92 — **Sichttest S90/S91 begonnen** (A1, D1, J1, J3 abgenommen; vier Befunde H1, E1, G1, D2 behoben). **§7 Lyrics-Block:** Lyrics Sync mit Pre-roll, Sprungleiste, Kontextmenü; Feld Word spacing; **Trail-Presets** (`.lvtrail`, acht mitgeliefert, Zeile im Karaoke-Panel, Rechtsklick im Kettenbaum, eigenes Icon); Prüfer für die Skriptbeispiele des Handbuchs; die vier Fragen aus S89 entschieden (1a, 2a, 3a, 4a). Offen: Sichttests S88–S92 |
| 1.84.0 | 2026-10-03 | Session 91 — **§7 Lyrics-Block: LB5-3 gebaut** (Schieber Speed im Panel Lyrics Sync, Entscheid T1 a) und die kleinen Reste aus S89/S90 erledigt: Statuszeile des Sync-Panels, Trail der Karaoke-Ebene über den Preset-Wechsel, Load… mit `.avs` / `.milk`, Favoriten im Effect Browser (F1 a), Feld Line am Lyrics Trail (T2 a). Offen: Sichttests S88–S91, die vier Fragen aus S89 |
| 1.83.0 | 2026-10-03 | Session 90 — **§7 Lyrics-Block gebaut bis auf LB5-3** (Zeilen-Puffer, Vorschau-Zeilen, laufendes Wort, `word` / `wordcount` und Feld Word, Wort-Masken auf der Grafikkarte, eine Schriftgröße je Song, `linetrigger` / `wordtrigger`, Lyrics Sync mit Stimmen und Wort-Zeiten; Steuerdokument `visuals/Lyrics_Block_Konzept.md`); drei Fixes (Doppel-Trail nach Pausen, Sprung beim Zeilenwechsel, AVI-Knoten unsichtbar). Offen: langsamer abspielen (LB5-3), Sichttests S90 |
| 1.82.0 | 2026-10-02 | Session 89 — **Karaoke-Ebene KE2–KE4 gebaut** (Panel mit Reitern Lyrics / Lyrics Trail, Karaoke-Presets `.lvkar`, View → Karaoke, Ctrl+K; Steuerdokument `visuals/Karaoke_Ebene_Konzept.md`), K4 b (Zeile in Fensterauflösung). **Save / Save As**, **Rückfrage bei ungespeicherten Änderungen** (alle Ladewege, Beenden, Visualizer-Wechsel), **Preset-Zeile** mit Favoriten und Zuletzt in den Panels Effect Chain und Karaoke. **§7 ✅ Host Switcher: Puffer je Gruppe** (*Keep group state*, *Reset buffers*, `clearbuffers`; Befund berichtigt: ruhende Gruppen verlieren ohne Puffer ihre ganze Gruppen-Laufzeit). Skriptfeld **Line** am Lyrics-Knoten (`line`, `linecount`), Feld **Blend strength** an Lyrics / Lyrics Trail. **§7 NEU:** Lyrics — Zeilen-Puffer für den Trail, Wort-Ebene (laufendes Wort, `word` / `wordcount`, Slot Word), `linetrigger` / `wordtrigger`, Wort-Zeiten klopfen (für S90). Sichttests: alles aus S89 offen (`.claude/handover/Sichttests_S89.md`). Tests 810, Felder 846 |
| 1.79.0 | 2026-09-30 | Session 83 — **§4:** Sichttests S82 (Presetverweis, Song-Slot, Host Switcher, Preset as Group) abgenommen; neue Zeile S83. **Preset-Zeile:** Combo behält die Wahl, „(unsaved) – Name" nach echter Änderung (Vergleich vorher/nachher, nur solange sauber), 🗑 zielt auf die Quelle (Entscheid a); Name/Beschreibung zählen nicht. **§7 ✅ Vorlagen für Milkdrop-Elemente und Shader** (Element-Presets, 3D-Template-Listen → nodepresets, Shader-Vorlagen im Groß-Editor mit Kompiliertest). **Schalter „Script active"** am Knoten (E1a, zentral in `renderNode`, 79 Typen per Test mit Gegenprobe). **§6 NEU Block F** mit der Bestandsaufnahme als Eingang (E2b). **§8:** Milkdrop-Variablentabelle unvollständig (neu), ISF-Referenz ✅. **§7 NEU (Ideen):** Bild an andere Programme (Spout/NDI — Stand: nicht vorhanden) · externer Audio-Eingang (Loopback angelegt, nicht gebaut; Line-in/Mikrofon fehlt) · MIDI/OSC (nicht vorhanden; Anknüpfung Block F) · Video aufnehmen (App: nur Screenshot; PNG-Folgen nur im AvsStandalone; Qt 6.10 bietet den Weg) · Video Source: nur Felder der gewählten Quelle zeigen. **§7 NEU:** S84-Plan (Drag aus Effect Browser/Playlist, Audio Browser, Timeline-Ausblick; Englisch fortsetzen mit Entscheiden 1a/2a) · später Code-Kommentare englisch · später Sprachwahl (i18n). **§8:** Oberflächentexte englisch (≈ 370 Texte, „Testaufnahme" → „Camera Clip", Wächter über `tr()` + Referenzseiten); offen Feld-Tooltips (FieldDocs, Entscheid) und Clip-Dateiname. Tests 677. **Nachtrag:** Namen englisch (Vorgabe Patrik) — **alle** mitgelieferten Presets (≈ 150), Beispiele (134) und Shader-Vorlagen; Zuordnung alt → neu in `asset/nodepresets/README.md` (ältere Einträge dieser Liste nennen noch die alten Namen), Kommentare, Beschreibungen und Knotennamen darin ebenfalls englisch (74 Kommentar-/Beschreibungszeilen, 7 Knotennamen). **Wächter** gegen eine Liste bekannter deutscher Namen und Wörter (`tests/unit/UnitTests/deutsche_woerter.txt`: 147 alte Namen + 148 Wörter ohne englische Doppelgänger) plus Umlaute — prüft Dateinamen, Code-Kommentare, `description`/`name`; neue deutsche Wörter, die auffallen, kommen in die Liste. Presets für Milkdrop *Code* und *Parameter* inkl. *Old LumiViz Look (pre-S80)*; Feld-Wache im Ladbarkeitstest. Tests 676 |
| 1.78.0 | 2026-09-29 | Session 82 — **§3 Kalibrierlauf 4090 komplett** (22 × 5 Formate × 2 Saaten, ersetzt die 610M-Tabelle; Kartenvergleich: ps_2-Presets kartenunabhängig, `Beauty` OK, `Neomtwister` näher; Vorbehalt Früh-MAE 0,000 = beide schwarz). Harness `--kerne ccd2` (Affinität, Kontrollmessung: Zahlen gleich, Tctl −3–4 °C). **Preset as Group** (Palette, `.avs`/`.milk`/`.lvfx` als Kopie in eine Host-Gruppe, „Aus Quelle neu laden", Wurzel-Einstellungen bleiben erhalten) — Sichttest in §4. „Import Browser" → **Effect Browser**. **Host Switcher** (Stufe Host-Gruppen des Szenen-Wechslers, §7) + eindeutige Gruppennamen + Songwechsel-Signal an den Renderer; Fix: eingeklappte Container bleiben nach Strukturänderungen zu. **Song-Slot für alle Skript-Knoten** (§6 `onChange`-Slot ✅; Engine-Slot 5, `ChainNode::songCode`, 79 Typen per Test). **Presetverweis** (Host-Gruppe mit `verweis`: nur Pfad gespeichert, Auflösung beim Laden mit Zyklus-Schutz, Palette „Preset Reference…"). Tests 665 |
| 1.76.0 | 2026-09-29 | Session 80 — **MilkDrop-Treue gemessen statt geschätzt (§3):** Gestaltung (Linear-/Radial-Drift, Weichzeichnen, skriptfähig; „Weicher Sog ↖“ bitgleich zum alten Look); `stratospheric turbulences` → vier Zufallsquellen, `MilkdropRef --zufall-lumiviz`, **Skript-Zufall ns-eel2** (exakt, Standard Original), Gitter-Sonde (+0,98); gelbe Kugeln → Spektrum der Waves → **MilkdropFft/MilkdropAudio** (FFT, AlignWaves, bass/mid/treb exakt gegen den Kern; **Audio Standard Original**; Messreihe mitgelieferte f10 0,087 → 0,055); **🔴 MilkdropRef-PCM-Format seit S63 falsch — behoben**, alle MilkDrop-Referenzzahlen davor unter Vorbehalt. Offen: y-Grundlinie der Custom-Waves (nächste Session). Tests 648. — **✅ §3: blanke Pipeline blendete aus und driftete nach oben links** — Sonde gegen MilkdropRef belegt (Referenz steht, wir 0,5 px/Frame + Weichzeichnen), Ursache Halbtexel-Versatz im Warp-Mesh, entfernt (Entscheid a); Messreihe vorher/nachher gegen dieselben Referenzbilder: Frame 120 Δ −0,007 (320) / −0,0065 (640), 10 besser / 1 schlechter; Sonde als Regressionsprobe `asset/calibration/milkdrop/drift/`. **Sichttest automatisiert:** `test_PaletteUi` (21 UI-Tests offscreen, Palette + echtes Panel mit Host, Screenshots per `LUMIVIZ_UI_SCHNAPPSCHUSS`) und `asset/calibration/sichttest/sichttest_auto.py` (12 Ketten-Stichproben B/D mit Kontaktbögen) — dabei **✅ §7 Drop auf die Leerfläche behoben** (betraf nur Milkdrop-Elemente). NEU §7: Vorlagen für Milkdrop-Elemente/Shader. Editor: `b` als in/out gefärbt wie `r`/`g` (Entscheid a); Mehrfachzuweisung `r = r2 = red;` getestet + Referenzsatz. Prüfprotokoll [Sichttest_Protokoll_S79.md](Sichttest_Protokoll_S79.md). Tests 610/610 |
| 1.75.0 | 2026-09-27 | Session 79 (Auftrag Patrik: Modulauswahl + Formelfelder) — **Palettenfenster umgesetzt** (nicht-modal; Suche über Name/Originalname/Beschreibung, Chips Herkunft AVS/APE/MilkDrop/LumiViz + Rolle, ★ Favoriten + Zuletzt, Detailzeile mit Formelfeldern, Add/Enter/Doppelklick + Drag & Drop in den Kettenbaum; Palettendaten nach `EffectPalette.{hpp,cpp}` ausgelagert, je Eintrag Rolle/Kurzbeschreibung/Originalname; Test `test_EffectPalette`; Sichttest offen). **Formelfelder Block B:** Farben per Skript als `<feldname>_r/_g/_b` in 16 Modulen (`FarbKopie`, Referenz + Struct-Doku + Tooltips nachgezogen). **Block C:** Farbpaletten der sieben Fraktale per Skript (`colorscale`, `colorcycle`, `colorphase`, Innen-/Hintergrund-/Negativfarbe). **Block E:** Set Render Mode + ISF Filter mit Parameter-Skript (ISF-Regler unter ihrem key); selbst deklarierte Uniforms in Shadertoy/Pixel Filter/Mesh Warp/GPU-Partikel als Skriptvariablen (Entscheid 1a, Shadertoy neu mit Init/Frame/Beat); Host Group `outadjustalpha` (Entscheid 2a); Fix: Set Render Mode schrieb `adjustAlpha` nicht. **Block D:** 43 weitere Felder in 14 Modulen skriptbar (Fraktal-Details, Bump-Tiefe, 3D-Profile, Interferences-Beatziele, GPU-Partikel, Water-Bump-Tropfenort). **Formelfelder:** Inventar aller 84 Module (`visuals/Formelfelder_Inventar.md`); Block A umgesetzt — `distance` (Brightness), `skipfirst` (Custom BPM), `x2`/`y2` (Interleave), `blend` (Timescope, Dot Grid), `edgemode` (Convolution), `p`/`q` (Kleinian), `variation`/`scale` (Flame) gebunden; Variablenreferenz aus der SSOT `ScriptVariables.hpp`, geprüft durch `test_ScriptVariablesGl` (Namens-Sonde in `LuaScriptEngine`). Neu in §7: Palettenfenster, Formelfelder B–F |
| 1.74.0 | 2026-09-27 | Session 78 (Vorgabe Patrik: Feldbreiten der Effektkette) — Bestandsaufnahme aller ~90 Modul-Editoren, dann zentraler Breiten-Vertrag in `MultiEffectPanel`: Felder ≥ 120 px, 5 px Panelrand, seitliches Scrollen des Panels statt Überbreite, Dock-Minimum 240 px + Ränder, Effektliste mit Spalten-Mindestbreiten und eigenem Balken. Von Patrik abgenommen. Neu in §7: Vertrag nur in der Effektkette |
| 1.73.0 | 2026-09-23 | Session 77 (Bugfix, Meldung Patrik: Visualizer nach einer Mausbewegung verschwunden, auch nach Neustart) — im gespeicherten Dock-Layout fehlte `visualizer` (nur ein leerer Floating-Container), Qt-ADS versteckte ihn deshalb als *unassigned*; Reset Layout spielte dasselbe Layout ab, View > Panels kannte ihn nicht. Behoben in `DockManager` 2.2.0 (§5.6): fehlende Visualizer nach Restore/Reset andocken, Reset aufs Werks-Layout, Visualizer im Panels-Menü. Neu in §7: Tab-X löscht den Visualizer (Entscheid offen) |
| 1.72.0 | 2026-08-09 | Session 74 (das Pruefsignal, Auftrag Patrik: „fuer Vergleiche synthetisch, aber aus dem MP3 intensiver synthetisiert") — **das synthetische Signal kommt jetzt aus EINER Quelle** (`projects/exec/common/SynthAudio.hpp`) statt aus vier Kopien. Bis S74 stand die Formel in AvsStandalone, MilkdropStandalone, `tools/AvsRef` und `tools/MilkdropRef` je einmal, jedes Mal mit dem Kommentar „formelgleich zu …" — genau die Sorte Abmachung, die still auseinanderlaeuft. Der Kopf ist bewusst Qt-frei und C++14-tauglich, damit das 32-bit-AvsRef und der D3D9-MilkdropRef ihn uebersetzen. Dabei festgehalten, was vorher unausgesprochen war: die WELLENFORM war in allen vier gleich, das SPEKTRUM nicht — der AVS-Pfad legt einen Beat-Faktor ueber alle Bins, der MilkDrop-Pfad gibt Bass/Mitten/Hoehen eigene Huellkurven (S64, sonst 0/0-NaN bei Banddifferenzen). Beide Fassungen bleiben, `Geschmack` waehlt aus; Zusammenlegen waere ein Eingriff in Messreihen und gehoert eigens entschieden. **NEU `--audio-muster klassisch|musik` an ALLEN VIER Werkzeugen** plus `compare_avsref.py`: `musik` traegt acht log-verteilte Bandhuellkurven je Bild und eine Beat-Spur, gewonnen aus 20 s echter Aufnahme, und synthetisiert daraus NEU (acht Sinus mit musikalisch bewegten Amplituden). Es wird keine Musik abgespielt, das Profil laesst sich nicht zurueckrechnen — und weil beide Seiten dieselbe Datei einbinden, **bleiben Referenzvergleiche gueltig**, anders als bei `--audio-datei`. Erzeugt wird das Profil mit `AvsStandalone --audio-profil-schreiben` (Analyse: FFT je Bild, Bandnormierung je Band, Onset-Erkennung, Tempo per Autokorrelation mit 120-BPM-Prior — ohne den Prior landete die Schaetzung zweimal eine Oktave daneben, 257 dann 63 statt ~129 BPM). **REGRESSION BELEGT:** `klassisch` ist bit-identisch zu vor dem Umbau (gleicher PNG-Hash), und der Referenzvergleich liefert exakt die S73-Zahlen (`09_rotating_things` MAE 0,010 / `02_flowers` 0,015). Im AVS-Pfad kommt der Beat im Musik-Muster aus der Beat-SPUR — dafuer `MultiEffectVisualizer::setBeatTrackOverride()` als Geschwister des vorhandenen `setBeatPeriodOverride()` (Test-Hook, wie dieser). **ZWEI BEFUNDE NEBENBEI**, beide in §8: `02_flowers` verliert im Musik-Muster seinen dMaxLuma-Befund (0,150 → 0,017) — noch kein Freispruch, der Beat unterscheidet sich zwischen den Laeufen; und **MilkdropRef liess sich seit S67 nicht mehr voll neu bauen** (`/FI ref_msgbox.h` zog `<cstdio>` und namenlose Parameter in die neun `.c`-Dateien des ns-eel2-Kerns), gefunden und behoben. Tests 593 gruen, alle vier Werkzeuge gebaut |
| 1.71.0 | 2026-08-09 | Session 74 (echte Musik in den Standalones) — **Aufgabe 6 des Kalibrier-Plans umgesetzt, vorgezogen auf Wunsch Patriks:** beide Standalones nehmen jetzt eine Audiodatei statt des synthetischen Sinus (`--audio-datei`, `--audio-start`, `--audio-gain`, `--audio-stumm`). Gemeinsames Bauteil `projects/exec/common/AudioDateiQuelle.{hpp,cpp}` — dekodiert die Datei per `QAudioDecoder` EINMAL komplett (kein BASS noetig, Qt Multimedia lag schon dabei), liefert je Bild 576 Stereo-Rahmen PCM plus 512 Baender aus einer Hann-gefensterten 1024-Punkt-FFT, linear 0..22050 Hz — **dasselbe Bandmodell, das `MilkdropVisualizer::updateAudio` fuer die MilkLoudness-Terzen (200/761,2/2897,1/11025 Hz) voraussetzt**. Abgetastet wird nach BILD-INDEX (`t = Bild/60`), nicht nach Echtzeit: zwei Laeufe mit derselben Datei und derselben Bildzahl sind bitgleich — **belegt an einer Sequenz: 7 von 7 Bildern identisch** (90 Frames, `--save-every 15`). Interaktiv laeuft die Datei zusaetzlich HOERBAR mit (QAudioSink im Pull-Betrieb ueber das dekodierte PCM); `--auto` und `--ab` bleiben still, damit Stapellaeufe deterministisch bleiben. **DREIFACH ABGESICHERT, weil echte Musik jeden Referenzvergleich wertlos macht** (AvsRef/MilkdropRef erzeugen ihr Audio selbst): `[Audio] ACHTUNG`-Zeile bei jedem Lauf, harte Sperre in `compare_avsref.py` (`run_lumi` wirft bei `--audio-datei`), und die hoerbare Ausgabe schaltet sich im Stapellauf selbst ab. **EIGENER FEHLER, gefunden und behoben:** der erste Kopierpfad rief `QAudioBuffer::constData<T>()` je Sample (~21 Mio. Aufrufe bei vier Minuten) — Ladezeit **68 s**, und der Standalone sah dabei aus wie aufgehaengt, dieselbe Taeuschung wie beim Debug-Build. Basiszeiger und Formatverzweigung aus der Schleife gezogen: **3,8 s**. Werkzeug-Wegleitung 1.1.0 (siebte Falle + Rezept), Kalibrier-Plan 1.1.0 (Aufgabe 6 ✅) |
| 1.70.0 | 2026-08-09 | Session 73 (Schaufenster + Anwender-Doku) — **README zeigt jetzt, was LumiViz kann:** zwei Galerien mit je vier Standbildern (AVS: fractal Dreams, flowers, rings, wormhole · MilkDrop: Rock The House, The Beauty and the Math, Twisted, Playaround) und ein Kapitel „Fremde Presets importieren", das AVS und MilkDrop als das benennt, was sie sind — zwei sehr verschiedene Formate, beide gegen ihren Original-Renderer gemessen. Die Falschaussage „Presets werden nicht mitgeliefert" ist raus (seit dem Vorgaenger-Commit werden 29 mitgeliefert). NEU **drei Anwender-Dokumente**: `Preset_Quickstart.md` (eine Seite, fuenf Schritte), `Preset_Anleitung.md` (zehn Kapitel: Reihenfolge als halber Look, die 54 Knotentypen in drei Sorten, Rueckkopplung, Audio, die vier Formel-Etappen, Fehlersuch-Tabelle) und `Werkzeug_Wegleitung.md`. Handbuch + beide Preset-Dokumente wandern per POST_BUILD nach `<Build>/docs/` (SSOT bleibt `docs/`, dort entstehen nur Kopien; die Wegleitung bleibt draussen — Entwicklerstoff). **ZWEI WERKZEUG-LUECKEN GESCHLOSSEN, beide beim Screenshot-Machen aufgefallen:** (a) **`AvsStandalone` kannte den Render-Scale-Divisor nicht** — die App setzt vor jedem Import `setImportRenderScaleDivisor()` aus `import/avsRenderScaleDivisor` (bei Patrik 4), der Standalone nie. Folge: JEDER Standalone-Vergleich bei grossen Fenstern war systematisch falsch; bei den ueblichen kleinen Fenstern faellt es nicht auf, deshalb blieb es unbemerkt. NEU `--render-scale N`. (b) **Das synthetische Audio hatte eine starre Klangfarbe** — fester 1/f-Abfall, feste Bandgewichte, Beat starr bei 2 Hz; jeder Frame klang gleich gefaerbt. NEU `--beat-hz N` und `--klangfarbe` (wandernde Spektralbalance + Oberwellen). **Ohne die Schalter aendert sich nichts** (kipp=0 faellt exakt aufs alte Verhalten zurueck) — bestehende Kalibrierlaeufe bleiben bit-identisch. Messwert am Preset „Rock The House": Mittelwert 0,011 → **0,203**, Luma max 0,29 → **1,00**. **DRITTE FALLE, ohne Codeanteil:** Stapellaeufe im DEBUG-Build sind **Faktor 20** langsamer (306 s gegen 15 s fuer zehn Presets) und wirken wie ein Haenger — dokumentiert in der Werkzeug-Wegleitung samt GPU-Vorgabe am EXE-Pfad, Pfaden mit Leerzeichen und „Fenster nicht schliessen" |
| 1.69.0 | 2026-08-09 | Session 73 (Umbenennung + Versions-SSOT) — **die App heisst jetzt ueberall LumiViz.** `projects/apps/MyViz/` → `projects/apps/LumiViz/` (898 Dateien), Targets `MyViz`/`MyViz.Core`/`MyViz.UnitTests` → `LumiViz*`, 290 Textstellen in 145 Dateien byte-weise ersetzt (MyViz ist reines ASCII — keine Kodierungswandlung, keine Mojibake-Gefahr). Erleichternd: es gab **keinen `myviz`-Namensraum und keine `MyViz`-Klassen**, im C++ waren es nur Zeichenketten und Kommentare. **NEU: Name und Version sind SSOT** — beide stehen in `Solution.json`, das Root-`CMakeLists.txt` liest sie per `string(JSON …)` heraus und reicht sie als `LUMI_APP_NAME`/`LUMI_APP_VERSION` an `LumiViz` und `LumiViz.Core`; im Code fragt nur noch `AppInfo.hpp`. Vorher standen sie **dreifach** (Solution.json, hart in `Application.cpp`, Literal `"Version 0.1.0"` im About-Dialog) — der About-Dialog zeigte seit dem Vorlagen-Stand eine Version, die es nie gab. **Version 0.1.0 → 0.2.0** (Entscheid Patrik): erste tatsaechlich beziehbare Fassung; 0.9.0 waere ein falsches Signal („1.0 steht bevor") bei offener Kalibrier-Runde. Solution-Name `MinimalSolution` (Vorlagen-Rest) → `LumiViz`. **EINSTELLUNGS-UEBERNAHME:** Organisation und Anwendungsname bilden den QSettings-Pfad — ohne Zutun waeren Fensteranordnung, Hotkeys, GPU-Wahl und das gemerkte Preset scheinbar weg. `migriereAlteEinstellungen()` kopiert einmalig von „MyViz Project/MyViz" nach „LumiViz Project/LumiViz", nur wenn dort noch nichts steht, und loescht die alten Werte NICHT. Am laufenden Programm belegt: **37 Schluessel uebernommen**. **DABEI EIN EIGENER FEHLER AUFGEFLOGEN:** war ein Preset gemerkt, dessen Datei es nicht mehr gibt (hier: der alte `exec/MyViz`-Build), fiel der Start auf gar nichts zurueck statt auf das mitgelieferte — die Vorgabe griff nur bei LEEREM Wert. Behoben, mit Log-Hinweis. Der Test-Timeout aus 1.68.0 hat sich sofort bewaehrt: der Kaltlauf nach dem Neuaufbau brauchte 26,1 s und waere an den alten 30 s knapp vorbeigeschrammt. **NOTIZ:** die GPU-Vorgabe in `UserGpuPreferences` haengt am EXE-PFAD — aus `MyViz.exe` wurde `LumiViz.exe`, der alte Eintrag verwaist und ist bei Bedarf neu zu setzen. Tests 593 gruen (`LumiViz.UnitTests`), Debug- und Testing-Build gruen |
| 1.68.0 | 2026-08-08 | Session 73 (Veroeffentlichung + Startzustand) — Repo lizenz- und datenschutzbereinigt und OEFFENTLICH gestellt (Historie mit `git filter-repo` umgeschrieben, 87→52 MB; duale Lizenz MIT/Apache-2.0, THIRD_PARTY_NOTICES, README, BUILDING, `.github/`). **Drei App-Fehler behoben, alle drei am laufenden Programm nachgewiesen:** (a) **`.lvfx`-Ketten waren an den Rechner des Erstellers gebunden** — `resolveAviPaths` lief NUR beim `.avs`-Import, beim `.lvfx`-Laden uebernahm der Serialisierer den gespeicherten absoluten Pfad unveraendert; `videoSource` hatte die Aufloesung seit S70, `avi` nicht. Fix in `loadChainFile`, die Suche geht ueber den blanken Dateinamen und repariert damit auch Altbestand. (b) **Der Blaettern-Hotkey tat nach jedem Neustart nichts**, bis man einmal im Browser etwas geladen hatte: `onPresetStep` liest die Liste, die aber erst in `onActivate()` entstand — war das Panel nie offen, war sie leer. Die Liste wird jetzt im Konstruktor gefuellt; belegt mit `step 1 from 4 -> 5 (11 entries)` ohne je das Panel zu oeffnen. (c) **Der Startzustand wurde nicht wieder aufgenommen** — Import-Browser laedt und markiert jetzt das zuletzt geladene Preset (Vorgabe `presets/avs/EyeCandy2/02_flowers.avs`, RELATIV zur Exe), und der zuletzt gewaehlte Visualizer wird gemerkt (`ui/lastVisualizer`; `MainWindow` setzte vorher hart `multieffect`). NEU ausserdem: **Preset-Auswahl wandert per POST_BUILD neben die Exe** (`asset/presets/` → `<Build>/presets/`, Pflegenotiz erklaert das Erweitern), Import-Browser startet im **Programmordner** statt im Benutzerordner, Test-Timeout 30→120 s (der ERSTE ctest-Lauf nach einem Build schlug reproduzierbar an die 30 s). **DREI FALLEN festgehalten:** `PanelManager::restoreState()` hat KEINEN Aufrufer (§8) · die Sonden-Liste taugt beim Start nicht als Anker · QSettings-Lesen MUSS innerhalb von `beginGroup()` stehen, sonst kommt der Wert aus der Wurzel und ist immer leer. Ausserdem **60 neue Feld-Sonden** fuer die Knotentypen seit S69–S72 mitgenommen (Entscheid Patrik) — erzeugt, aber noch nie gelaufen (§4). Tests 593 gruen, Debug- und Testing-Build gruen |
| 1.67.0 | 2026-08-08 | Session 72 (Filter-Strang KOMPLETT) — **ISF ist ein EIGENER Knotentyp `isfFilter`** (Entscheid Patrik: „lassen wir den pixel filter lieber so wie er war"), der Shader UNVERAENDERT ausfuehrt: beide Shader-Stufen in getrennten Feldern, Import-Knopf im Panel, bearbeitbare Quellen-Liste, Sorten-Info, Lizenzfeld, Parameter-Baum, Bauart Quad/Dreieck/Gitter — **und Multipass (PASSES)**. Der Kurswechsel hat vier Mechaniken ERSATZLOS gestrichen (Makro-Ersetzung, main-Umschreiben, .vs-Falten, Varying-Spiegelung) und alle drei Ablehnungen aufgehoben: **es gibt keine Kategorien, nur unterschiedlich viele Bildquellen** (0/1/2 = Generator/Filter/Uebergang; jede Sorte darf einen .vs haben — am Korpus belegt). Messwert GL-Smoke (echter 3.3-Core-Kontext, kompilieren UND linken): 251 → 319 (Multipass) → **321 von 327** (98 %); Waechter-Untergrenze 95 %. Kopf-Korpus 327/327 angenommen. **VIER BEHOBENE FEHLER:** (a) `nodeToJson` schrieb `o["name"] = displayName`, danach ueberschrieb der Shadertoy-Visitor genau diesen Schluessel — ein umbenannter Shadertoy-Knoten VERLOR SEINEN NAMEN beim Speichern; (b) `aktualisiereAudioTexturen()` lief NACH dem Binden des Kettenbilds und lud auf Einheit 0 — `_lumiPrev` zeigte auf die Audio-Textur, JEDER Filter tastete Audio statt Bild ab (Sichttest Patrik „kein einziger filter funktioniert"; weder Unit-Tests noch GL-Smoke koennen das sehen); (c) beide ISF-Audio-Typen hingen an der kombinierten 512×2-Shadertoy-Textur — ein Shader, der bei y=0.5 abtastet, las eine MISCHUNG aus Spektrum und Waveform (aufgedeckt allein durch die Rueckfrage „geht das auch richtig?"); (d) `IMG_SIZE` lieferte pauschal RENDERSIZE statt der Sampler-Groesse. NEU ausserdem: gemeinsame Audio-Texturen aus EINER Rechnung (Idee Patrik, hoechstens ein Upload je Frame statt einem je Knoten), `getosc()`/`getspec()` als GLSL-Funktionen mit AVS-Signatur (Idee Patrik), generische `Herkunft` + Herkunfts-Kopf beim Shader-Export (S71-Befund geschlossen), generischer Parameter-Baum (`ParamGruppe`/`ParameterBaum.hpp`), **iChannel-Ausbau des Shadertoy-Knotens** (Ketten-Eingang + AVS-Buffer 1–8, Handbuch §12). Der S71-Vorbau (`ShaderVertrag`, Import-Pruefung, Export-Namensschema) war GANZ UNGETESTET — hat jetzt eine eigene Suite. NEU 🟠-Block **„Key-Variablen-Regel fuer JEDES Import-Format"** (Vorgabe Patrik, gilt auch fuer Shadertoy und kuenftige) mit vier Befunden, darunter ein UNGEPRUEFTER Shadertoy-Fall seit S65. Tests 593, Feld-Inventar 91 Typen/781 Felder mit 0 Luecken, alle 3 Builds gruen. Benutzerhandbuch 1.10.0 |
| 1.66.0 | 2026-08-07 | Session 71 (Filter-Strang, Planung) — **NEU §7-Block „ISF-Import + generischer Parameter-Baum" als nächster Strang** (Entscheid Patrik). Dreistufig: (1) ISF-Parser pur/testbar nach dem `PixelFilterWrapper`-Muster, (2) generische Parameter-Ablage im Node inkl. Pflichtkette (Serializer/FieldDocs/Wächter), (3) **Parameter-Baum im Panel — Entwurf Patrik: Aufbau wie die Effect-Chain, aber mit Wert-Spalte und typsicheren Editoren je Zeile** (ISF deklariert den Typ; `long` mit LABELS/VALUES wird ein echtes Klartext-Dropdown), bewusst nested-fähig (INPUTS sind flach, `PASSES`/`IMPORTED` nicht). **⚪ NEU (Idee Patrik): dasselbe Prinzip für ALLE Module** — kein ISF-Sonderweg, sondern die Infrastruktur, die auch „dynamische Modulparameter" braucht; beim Bau nicht ISF-spezifisch schneiden, Verhältnis zur bestehenden Panel-Erzeugung klären. KLEINFIXES am Vorbau: Export schlägt jetzt einen **freien Dateinamen** vor (`preset(2).image.shadertoy.glsl` — Zähler VOR den Endungen, damit `.vertrag.endung` intakt bleibt; Qts Überschreib-Frage bleibt als Netz) · `.vert` im Dateifilter ergänzt (Gegenstück zum vorhandenen `.frag`; `.gs` bewusst NICHT — ISF kennt nur `.fs`/`.vs`, und wir haben kein Geometry-Feld). **KORREKTUR einer früheren Aussage in dieser Session: der Shadertoy-Import ist im Kern NICHT offen** — `ShadertoyBrowserPanel` (Strang S3) hat Query-API-Suche, Thumbnail-Grid, Doppelklick-Import als .lvfx, API-Key-Feld und Link-Knopf; **offen sind dort der iChannel-Ausbau UND die Lizenz-Kette** (Einwand Patrik): die Felder `name`/`author`/`url`/`license` existieren zwar samt Serializer und Panel-Anzeige, hängen aber NUR an `ShadertoyParams` (für ISF/pixelFilter fehlen sie) — und der **Shader-Export im Editor schreibt sie nicht mit**, die Herkunft geht beim Export also verloren. Beides ist im Plan §5.2/§8 aufgenommen |
| 1.65.0 | 2026-08-07 | Session 71 (Filter-Strang, Vorbau) — **Shader-Import gegen Vertrags-Verwechslung gesichert (Einwand Patrik: „muss man da nicht aufpassen, dass man nicht Shadertoy im pixelFilter importiert?").** Ist-Stand war ungeschützt: EIN gemeinsames Ordner-Gedächtnis `editor/shaderFileDir` für alle Shader-Felder, ein Filter für alle, keine Prüfung — ein Shadertoy in einem pixelFilter ergab nur einen kryptischen Compilerfehler aus dem Wrapper. NEU **`ShaderVertrag`-SSOT** in `EelScriptEditing.hpp` (key · Anzeige · Einstiegsfunktion · Signatur · echte Endung) für pixelfilter/shadertoy/meshwarp/gpuparticles/milkdrop, die drei Dinge gemeinsam trägt: (1) **Namensschema `<preset>[.<slot>].<vertrag>.<endung>`** — Klassifikation von rechts nach links immer spezifischer (Entscheid Patrik, Muster der Dateiendungen selbst); vorher stand nur der Slot drin (`preset.image.glsl` verriet nicht, dass es Shadertoy ist). (2) **Ordner-Gedächtnis je Vertrag** `editor/shaderFileDir/<vertrag>` (mit Ausweich auf den alten gemeinsamen Wert) + Dateifilter „Passende Shader" zuerst, `.fs`/`.vs` (ISF) mit aufgenommen. (3) **Vertragsprüfung beim Import** (`pruefeShaderVertrag`): fehlt der erwartete Einstieg und wird ein fremder erkannt, kommt eine Klartext-Warnung samt **Hinweis auf den zuständigen Knoten** — bewusst mit „Trotzdem laden" (Fragmente/Hilfsfunktionen bleiben erlaubt, Entscheid Patrik); ISF-Dateien werden am JSON-Kopf erkannt und benannt. **Milkdrop-Felder exportieren jetzt `.hlsl`** statt `.glsl` (Entscheid Patrik nach Empfehlung: der Inhalt IST HLSL, externe Editoren färben richtig ein — für unsere eigene Prüfung ist die Endung ohnehin nicht maßgeblich, die schaut in den Inhalt). **NEU 🔴 Lizenz-Pflicht im §7-Block** (Vorgabe Patrik): jeder Fremd-Import muss Herkunft/Autor/Lizenz ins Preset schreiben — im Loader, nicht nur in der Doku. **ISF-Format gegen die Spec geprüft** (kein separates JSON: eingebetteter Kopf, `.fs`; FX-Erkennung über `inputImage`) |
| 1.64.0 | 2026-08-07 | Session 71 (Abschluss) — **✅ SICHTTEST PATRIK ABGENOMMEN: der Kamera-Strang ist komplett.** (1) Dialog bestätigen → Kamera läuft → Preset-Wechsel → Schließen: **sauber, kein Prozess bleibt zurück** (zusätzlich per Task-Manager geprüft). (2) **Kein UI-Lag mehr spürbar**, obwohl die RGBX-Wandlung wieder auf dem Main-Thread läuft — die 720p/30fps-Formatklemme aus S70 reicht also aus, der Worker-Thread war für die Bildrate gar nicht nötig. Damit ist der Zielkonflikt „Lag gegen Prozessende" AUFGELÖST statt abgewogen: `LUMIVIZ_MEDIEN_THREAD` bleibt nur als Messschalter. Der 🔴-Punkt aus 1.59.0 ist geschlossen |
| 1.63.0 | 2026-08-07 | Session 71 (Fortsetzung 3) — **🎯 URSACHE DES PROZESS-HÄNGERS GEFUNDEN (A/B am Prüfstand): der EIGENE THREAD für die Multimedia-Seite.** Anstoß Patrik (Erinnerung „es ging, als alles im Main-Thread lief" + Auftrag, die Commits zu vergleichen). Der Commit-Vergleich zeigte: nur ZWEI Commits berühren LiveVideoFeed.cpp — `f93c83a` (S70 Teil A, dort steht wörtlich „App-Ende ohne/mit Kamera sauber") und `ace8d1c`. Der einzige strukturelle Unterschied: f93c83a hatte **keinen Zusatz-Thread**. A/B mit Schalter `LUMIVIZ_MEDIEN_THREAD` isolierte genau diese Variable: **MIT Thread 2/2 Hänger, OHNE Thread 3/3 sauberes Prozessende** (7,8 s = reine Laufzeit), Kamera läuft in beiden Fällen. Unabhängig von Media-Backend (FFmpeg wie WMF je 2/2 Hänger), von der Lage der Pipeline (Main- vs. Worker-Thread — der Ein-Thread-Besitz-Umbau auf den Medien-Thread half NICHT) und von COM-Init (`CoInitializeEx`/`CoUninitialize` auf dem Thread half nicht). **FIX: LiveVideoFeed arbeitet per Vorgabe auf dem Main-Thread**; `LUMIVIZ_MEDIEN_THREAD=1` schaltet den Worker-Thread für Messungen wieder ein. **ZWEITER BEFUND: `std::_Exit()` beendet den Prozess NICHT** (am Prüfstand reproduziert) — ExitProcess muss alle Threads abräumen, einer steckte im Treiber. Deshalb stand seit S70 „erzwungenes Prozessende" im Log, während die App weiterhing: die Logzeile täuschte einen Erfolg vor, den es nie gab. Der _Exit-Notausgang ist ersatzlos ENTFERNT (App + Standalone). Bootstrap_Integration 1.3.0: §6.4.5 korrigiert (_Exit ist kein Notausgang), NEU §6.6 (kein eigener Thread für fremde Medien-Pipelines). Tests 563 grün, alle 3 Builds grün, App-Schließen ohne Kamera 677 ms. **OFFEN: Sichttest Patrik mit KAMERA** (Dialog → Kamera läuft → Preset-Wechsel → Schließen; zusätzlich UI-Lag beurteilen, da die Wandlung wieder auf dem Main-Thread läuft — die 720p/30fps-Klemme aus S70 bleibt aktiv) |
| 1.62.0 | 2026-08-07 | Session 71 (Fortsetzung 2) — **REGRESSION aus 1.61.0 gefunden und behoben: App fror nach dem Kamera-Freigabe-Dialog ein, die Kamera ging gar nicht erst an** (Sichttest Patrik). Ursache: das neue `altenFeedRaeumen()` im Zusammenspiel mit dem Aufrufmuster des Render-Threads — der ruft `starte*` in JEDEM Frame, und bis der gequeuete Bau durch ist (Kamera-Pipeline-Start dauert), steht der Feed noch nicht in `m_feeds`. Jedes Frame queute also einen weiteren Bauauftrag, und jeder räumte über `altenFeedRaeumen()` die eben gebaute Kamera wieder ab ⇒ Endlosschleife aus Auf-/Abbau auf dem Main-Thread. FIX: NEU **Bau-Riegel `m_imBau`** — ein Bauauftrag je Knoten und Zielquelle, ausgetragen an jedem Ausgang von `baueDatei`/`baueKamera`; `alleStoppen()` verwirft offene Aufträge. VERIFIKATION am Prüfstand (echte Kamera, 3 Preset-Wechsel, 2 Läufe): Kamerabild kommt an (Screenshot-Sichtprüfung: Raumkonturen durch den Take-On-Me-Filter), Feed-Teardown **201/202 ms**, Warnungen=0. Tests 563 grün, alle 3 Builds grün, App-Schließen ohne Kamera 671 ms. **OFFEN: Sichttest Patrik** (Dialog bestätigen → Kamera muss anlaufen; Preset-Wechsel; Schließen) |
| 1.61.0 | 2026-08-07 | Session 71 (Fortsetzung) — **Teardown AM PRÜFSTAND GEMESSEN statt vermutet (Auftrag Patrik): Feed-Abbau 5320 ms → 200 ms, stabil über 4 Läufe.** Der erste S71-Fix griff NICHT (Sichttest Patrik: „schliesst noch immer nicht" + wachsendes Rauschen beim Preset-Wechsel). NEU Prüfstand: `AvsStandalone --kamera-freigeben --feed-teardown-messen` (Kamera-Vertrag: das explizite Flag IST die Nutzeraktion; ohne Flag bleibt der Standalone gerätefrei) + `LiveVideoFeed::herunterfahren()` am Standalone-Ende. **DREI Hypothesen nacheinander WIDERLEGT** (Beleg im Log statt Vermutung): (a) „Feeds akkumulieren" — falsch, beim Schließen `aktive Feeds=0 Friedhof=0`; (b) „Wandler steckt im map()" — falsch, `begonnen==fertig`, keine Frames in Arbeit; (c) „Queue-Stau" — falsch, Zähler stand still. **ECHTE URSACHE: ein einzelnes `QThread::quit()` unmittelbar nach dem Feed-Abbau geht VERLOREN** — der Wandler arbeitet dann noch interne Ereignisse der sterbenden MF-Pipeline ab und ist nicht quit-fähig, OBWOHL seine Event-Loop normal läuft (Herzschlag-Timer als Beweis) und kein Frame in Arbeit ist. Gemessen: `quit(); wait(5000)` 3/3 Timeout · dieselbe Stelle mit Pause davor 3/3 Erfolg · von innen gepostetes Quit-Event 3/3 Timeout · **wiederholtes `quit()`+`wait(200)` 4/4 Erfolg im zweiten Versuch**. FIX = Schleife (selbstkorrigierend statt geratener Wartezeit) → Vertrag 6.4.4. ZUSÄTZLICH gefunden und behoben: `m_feeds[nodeId] = neu` zerstörte den Alt-Feed als NEBENWIRKUNG der Zuweisung — unter dem Mutex und ohne Abschalt-Vertrag (genau der Gifttrigger, greift bei jedem Quellen-/Preset-Wechsel) → NEU `altenFeedRaeumen()`. **Rest-Hänger EINGEGRENZT:** nach einem Kamera-Lauf hängt der Prozess NACH `main()` im `~QGuiApplication`/fremden statischen Destruktoren (Marker-Beweis) — außerhalb unseres Codes, dafür ist der `_Exit`-Notausgang da. Bootstrap_Integration 1.2.0: NEU §6.4.4 (Joins wiederholen) + §6.5 (Messen statt vermuten). Tests 563 grün, alle 3 Builds grün, App-Schließen ohne Kamera 677 ms. **OFFEN: Sichttest Patrik** (App mit Kamera + Preset-Wechsel — Rauschdauer sollte nicht mehr wachsen) |
| 1.60.0 | 2026-08-07 | Session 71 — **🔴 KAMERA-TEARDOWN-DEADLOCK: URSACHE GEFUNDEN UND BEHOBEN (⬜ Sichttest Patrik).** Der Befund kam aus dem LOG, nicht aus dem Debugger: zwischen `Stopping video feeds...` und der nächsten Zeile lagen in JEDEM Lauf mit Feed exakt 5 s = der Timeout von `m_wandler->wait(5000)`. **Der Wandler-Thread beendete sich nie** — `alleStoppen()`/`friedhofLeeren()` riefen `kamera->stop()` und zerstörten die Objekte, WÄHREND der Wandler noch ein `QVideoFrame` derselben Pipeline mappte; ein `map()` auf einer sterbenden MF-/D3D-Pipeline blockiert im Treiber und hält deren Puffer für immer. **Genau daran starben die nvwgf2umx-Worker nie** — die S70-Einstufung „treiberintern, in-Prozess nicht heilbar" war falsch, es war hausgemacht. FIX (LiveVideoFeed 1.3.0): NEU **Abschalt-Vertrag** `feedStilllegen()` als einzige Abbau-Stelle — (1) Totflagge `Feed::tot` (der Wandler prüft sie VOR dem map und gibt den Puffer sofort zurück), (2) Senke abklemmen, (3) NEU `wandlerBarriere()` (leeres Lambda hinter die Queue + Semaphore, 2 s Timeout) — erst danach (4) stop/zerstören. `herunterfahren()` löscht den Wandler-Kontext erst NACH dem Thread-Ende (das `deleteLater()` davor wurde nie ausgeführt: Event an einen Thread ohne Event-Loop). **ZWEI weitere Befunde behoben:** (a) `kameraGelaufen()` → **`feedGelaufen()`** — der Notausgang hing am falschen Kriterium: der Lauf 00:28:55 hatte NUR einen Datei-Feed und hing genauso; (b) die **Settings-Testaufnahme war eine zweite, unbewachte MF-Pipeline** (QCamera/Session/Recorder als Panel-Kinder, am Dienst vorbei) — sie meldet sich jetzt per `merkeFremdenFeed()` und wird an `aboutToQuit` SYNCHRON abgebaut (`deleteLater` ist dort wirkungslos, die Objekte starben sonst per Parent-Destruktor mitten im Fenster-Abbau). **NEU: Lebenszyklus-Vertrag** `docs/core-services/Bootstrap_Integration.md` **1.1.0 §6** (Regelung Einschalten vs. Herunterfahren, Wunsch Patrik) — §5 zeigt jetzt die volle Kette inkl. aller aboutToQuit-Handler; VideoFrameCache 1.3.0 danach angeglichen: `herunterfahren()` statt aboutToQuit-Lambda (Abbau-Position darf nicht davon abhängen, wann ein lazy Singleton zuerst gebraucht wurde) + `abbrechen()` wirkt jetzt auch in der verschachtelten Event-Loop (Wachtimer in `warteAuf` — `quit()` beendet eine solche Loop nicht). Tests 563 grün, alle 3 Builds grün, Start/Schließen-Regressionsprobe 699 ms sauber. **OFFEN: der Beweis am echten Gerät** — Sichttest Kamera-Lauf + Schließen |
| 1.59.0 | 2026-08-07 | Session 70 (Abschluss) — **🔴 Kamera-Teardown-Deadlock BLEIBT OFFEN (Sichttest Patrik):** auch MIT dem _Exit-Ausweg aus 1.58.0 hängt die App — und zwar auch dann, wenn die Kamera zur LAUFZEIT gestoppt wurde (Wechsel auf einen Effekt ohne Kamera) und erst später geschlossen wird. Konsequenz: schon die ZERSTÖRUNG der Kamera-Pipeline im laufenden Prozess vergiftet ihn (nvwgf2umx-D3D-Worker sterben nie); der Hänger kann dann VOR dem _Exit-Punkt sitzen (Fenster-/GL-Abbau). Ohne Kamera-Start schließt die App sauber. **Ansätze für die frische Session:** (a) Haupt-Thread-Stack im Hänge-Zustand MIT externem Code (wo genau vor _Exit?), (b) Kamera-Feed bei Knoten-Wechsel NICHT zerstören, sondern nur stoppen/pausieren und bis Prozessende am Leben halten (Zerstörung ist der Gifttrigger, nicht der Betrieb), (c) `QT_MEDIA_BACKEND=windows` (WMF- statt FFmpeg-Backend) testen — anderes Teardown, evtl. auch ohne toImage-Bug, (d) GPU-Präferenz-Gegenprobe (610M vs. 4090, Hybrid-Verdacht), (e) _Exit noch früher (direkt im closeEvent nach Settings-Sicherung) als Brachial-Variante. Alle S70-Fixes davor bleiben gültig (Lag behoben, Zombie-Klassen 1+2 behoben, Riegel) |
| 1.58.0 | 2026-08-07 | Session 70 (Fortsetzung 8) — **Schließ-Hänger ENDBEFUND + kontrollierter Ausweg.** Nach der Reihenfolge-Umkehr: Musik aus (eigenes Cleanup komplett), aber Haupt-Thread hängt in Qt6Core bei den Multimedia-Postroutinen des ~QApplication — Screenshot: ~10 nvwgf2umx-Threads (NVIDIA-D3D11-Worker der Kamera-MF-Pipeline) sterben NIE. In drei Abbau-Reihenfolgen reproduziert → in-Prozess nicht heilbar (Treiber-/MF-Deadlock, mutmaßlich Hybrid-GPU 610M+4090). **Ausweg (Plan B, chirurgisch):** `Application::shutdown` beendet den Prozess per `std::_Exit(0)` NUR wenn in diesem Lauf eine Kamera lief (`LiveVideoFeed::kameraGelaufen()`), NACH vollständigem eigenem Cleanup (Fenster/GL weg, BASS frei, Feeds tot, Settings gesichert) — übersprungen werden nur fremde statische Destruktoren. Ohne Kamera bleibt der normale Abbauweg unverändert. Tests 563 grün, alle 3 Builds grün |
| 1.57.0 | 2026-08-07 | Session 70 (Fortsetzung 7) — **Schließ-Hänger: ABBAU-REIHENFOLGE UMGEKEHRT.** Sichttest zeigte: Fenster zu, LED aus, Musik läuft weiter, Haupt-Thread IN nvoglv64 (nicht in einem Wait) → der GL-Teardown wartet auf einen GPU-Kanal, den die parallel STERBENDE Kamera-MF-/D3D-Pipeline verklemmt. Zur Laufzeit koexistieren GL und Kamera-D3D problemlos — also: (1) aboutToQuit setzt NUR den Riegel (`beendenVorbereiten`, keine Neustarts), (2) Fenster/GL sterben, während die Kamera normal weiterläuft, (3) NEU `LiveVideoFeed::herunterfahren()` (alleStoppen + Wandler-Ende) läuft in `Application::shutdown` ZWISCHEN MainWindow- und QApplication-Abbau. closeEvent stoppt nicht mehr. Falls auch das nicht reicht, ist die Ursache Treiber-intern → Entscheid-Kandidat: harter Prozess-Abschluss am Ende von main() nach vollständigem eigenem Cleanup. Tests 563 grün, alle 3 Builds grün |
| 1.56.0 | 2026-08-06 | Session 70 (Fortsetzung 6) — **Schließ-Hänger Ursache #3: Kamera-WIEDERBELEBUNG im Teardown.** Nach alleStoppen() rief der noch laufende Render-Thread weiter `starteKamera()` auf (er rendert bis zum Fenster-Abbau) → Feed weg → baueKamera wurde NEU gequeued und startete mitten im Teardown eine frische MF-Pipeline gegen den GL-Abbau (Haupt-Thread hing in nvoglv64, LED war da schon aus — passt exakt zu „hängt nur bei laufender Kamera"). DREITEILIGER FIX: (1) **Shutdown-Riegel** `m_beendet` — alleStoppen() sperrt alle starte*/baue*-Pfade, auch schon gequeuete Events prüfen ihn beim Lauf; (2) **Kamera-Stopp schon im MainWindow::closeEvent** (MF-Abwicklung bekommt den Schließvorgang als Vorsprung, aboutToQuit bleibt Sicherheitsnetz); (3) `runVideoSource` lädt Kamera-Frames per **glTexSubImage2D** statt glTexImage2D-Neuallokation je Frame (Treiber-Churn, auch Lag-relevant). Tests 563 grün, alle 3 Builds grün |
| 1.55.0 | 2026-08-06 | Session 70 (Fortsetzung 5) — **Schließ-Hänger Ursache #2 gefunden (Kamera blieb AN, aboutToQuit nie erreicht):** die 1.53.0-Idee „Wandlung auf dem Render-Thread" hängt bei HARDWARE-Frames — map()/toImage() funktioniert nur im Lieferkontext; der Render-Thread blockierte, das Fenster-Schließen wartete auf ihn (Screenshot Patrik: 180 Threads, Haupt-Thread in Qt6Core). **Endgültige Architektur (LiveVideoFeed 1.2.0): eigener WANDLER-Thread** — die videoFrameChanged-Verbindung wird queued an einen Kontext auf diesem Thread zugestellt, die RGBX-Wandlung läuft dort (sicherer Lieferkontext, Puffer sofort zurück), fertige QImages unter Mutex; GUI- und Render-Thread fassen nie ein QVideoFrame an. Senke bleibt auf dem Main-Thread (kein moveToThread-Exot). aboutToQuit: alleStoppen → Wandler-Kontext deleteLater → Thread quit+wait. Hinweis Sichttest: wiederholte 0xC00D3EA3 („von anderer App verdrängt") = ein ALTER Zombie hielt die Kamera — vor jedem Test prüfen, dass kein LumiViz.exe mehr läuft. Skript-Test Start→Dialog→Schließen: sauber beendet; Tests 563 grün, alle 3 Builds grün |
| 1.54.0 | 2026-08-06 | Session 70 (Fortsetzung 4) — **Schließ-Hänger nach dem Lag-Fix behoben (Befund #3, Regression aus 1.53.0):** durch die Roh-Frame-Haltung wartete `QCamera::stop()` im aboutToQuit auf die Rückgabe des in `feed->roh` gehaltenen Puffers — Main-Thread hing, App ließ sich nicht schließen. Fix: Roh-Frame wird in `alleStoppen`/`friedhofLeeren` VOR dem Stopp freigegeben. **Lag-Nachstimmung:** `baueKamera` wählt jetzt ein Kamera-Format um 720p/≤30fps statt des Geräte-Maximums (RGBX-Wandlung skaliert mit der Pixelzahl; 1080p60 > 4× so teuer wie 720p30, ohne Gewinn für den Visualizer). Merkregel ergänzt: gehaltene QVideoFrames sind ausgeliehene PIPELINE-PUFFER — immer freigeben, bevor die Quelle gestoppt wird. Tests 563 grün, alle 3 Builds grün |
| 1.53.0 | 2026-08-06 | Session 70 (Fortsetzung 3) — **LAG-BEFUND behoben (Sichttest Patrik: extremes UI-Einfrieren mit Live-Kamera, Drag&Drop tot):** ZWEI Main-Thread-Fresser im S70-Code. (a) Haupttäter Kamera: `verbindeSenke` wandelte JEDES Kamera-Frame (30-60 fps, 1080p ≈ 8 MB) auf dem GUI-Thread nach RGBX — Event-Loop gesättigt. Fix LiveVideoFeed 1.1.0: Senke legt nur das rohe QVideoFrame ab (refcounted); NEU `bildNummer()` als billiger Vorab-Check, Wandlung in `letztesBild()` auf dem RENDER-Thread und nur bei neuen Frames. (b) Frame-Schritt-Datei: `VideoFrameCache::lade` dekodierte KOMPLETT auf dem Main-Thread (Seek je Frame + verschachtelte Event-Loops — bricht auch DnD). Fix 1.2.0: eigener Decoder-Thread mit Event-Loop + `abbrechen()`-Haken an aboutToQuit (Zombie-Klasse). Sonden-Doppelläufe nach der Threading-Änderung weiter SHA256-identisch; Tests 563 grün, alle 3 Builds grün. Merke: pixelFilter-GPU-Kosten (max. 36 Abtastungen) drücken höchstens FPS auf der 610M — GPU-Wahl auf die 4090 stellen, wenn Filter-Stapel + Kamera kombiniert werden |
| 1.52.0 | 2026-08-06 | Session 70 (Teil B) — **§7 Stilfilter UMGESETZT (⬜ Sichttest):** Chain-Node `pixelFilter` (Nutzer-GLSL `farbe(uv, src)` je Pixel, Nachbar-Sampling, Mix-Regler + Parameter-Skript `mixamount`; NEU `PixelFilterWrapper.hpp`; runPixelFilter im transformPass-Muster, Fehler ins geteilte stError; Palette „— GPU-Module —") + **12 Werks-Voreinstellungen** (Take-On-Me-Comic als Flaggschiff; Sepia auf Wunsch Patrik; Katalog-Zeilen im nodepresets-README) + 7 Wrapper-/Serializer-Tests (563 gesamt) + FieldDocs 90 Typen/774 Felder (0 Lücken) + Sonde (SHA256-Doppellauf identisch) + Demo. **BEFUND: `filter` = GLSL-Reserviert-Wort** (AMD lehnt ab) → Vertrag `farbe()`, Wächter-Test. Stimm-Befund: Comic braucht helle Grundflächen/harte Kanten. Benutzerhandbuch 1.8.0 (§14) |
| 1.51.0 | 2026-08-06 | Session 70 (Fortsetzung 2) — **Zombie-Prozess Schicht 2 gefunden und behoben (Stack-Beweis Patrik):** Haupt-Thread hing in `Application::shutdown` → `~QApplication` → Event-Queue-Entsorgung → `~Feed` — das stopp()-Queued-Lambda trug den letzten shared_ptr, die QCamera starb mitten im QApplication-Abriss. Fix: Feed-Eigentum bleibt beim Dienst (`m_friedhof` + `friedhofLeeren()` auf dem Main-Thread), Queued-Lambdas tragen kein Eigentum, `alleStoppen()` räumt Feeds+Friedhof synchron und zerstört die Qt-Objekte explizit. Verifikation: App-Start→Schließen ohne und mit Kamera-Dialog beendet sauber (2 Skript-Läufe); Tests 556 grün, alle 3 Builds grün. NEU Merkregel: Queued-Lambdas nie mit letztem Eigentum an Qt-Multimedia-Objekten |
| 1.50.0 | 2026-08-06 | Session 70 (Fortsetzung) — **Kamera-Sichttest Patrik: läuft** (USB-WebCam im videoSource-Knoten). Drei Nachzüge: (1) **Kamera-Freigabe-Dialog beim Preset-Laden** (Wunsch Patrik): einmal je geladener Kette, nur bei aktiver Kamera-Quelle ohne Freigabe; EINZIGE Vollbild-Ausnahme (aktives Fenster + StaysOnTop). (2) **BEFUND behoben: Zombie-Prozess nach App-Ende bei benutzter Kamera** — Queued-Stopps erreichen den Main-Thread beim Herunterfahren nicht mehr, MF-Capture-Threads hielten den Prozess (LED aus ≠ Teardown); Fix `LiveVideoFeed::alleStoppen()` synchron an aboutToQuit. (3) ⚪ Rest-Risiko fremde YUV-Kameras notiert (Rezept: NV12/YUYV-Wandlung in VideoFrameUtil). Tests 556 grün, alle 3 Builds grün |
| 1.49.0 | 2026-08-06 | Session 70 — **§7 Video-Weg Teil A UMGESETZT (⚪→⬜ Sichttest):** Chain-Node `videoSource` (Datei/Kamera/Testaufnahme; Streaming via NEU `services/LiveVideoFeed`, Frame-Schritt via VideoFrameCache 1.1.0 mit `Clip::fps`; Einpassung/Blend/Deckkraft; Parameter-Skripte `speed`/`opacity`) + Settings-Tab „Kamera" (Testaufnahmen benutzerlokal; Aufnahme-Klick = Kamera-Freigabe des App-Laufs) + Panel/Palette/Serializer/Klemmen + FieldDocs 89 Typen/769 Felder (0 Lücken) + 2 Serializer-Tests (556 gesamt grün). Sonde `videosource_sonde.lvfx`: Doppellauf SHA256-identisch. **BEFUND behoben: `QVideoFrame::toImage()` = schwarz unter Qt 6.10.1** (Rohdaten per map() da) → NEU `services/VideoFrameUtil.hpp` als toImage-Ersatz für LiveVideoFeed UND VideoFrameCache (betraf auch den avi-Cache-Fallback VR09) |
| 1.48.0 | 2026-08-06 | Session 70 — **§7 Video-/Kamera-Quellmodul + Stilfilter: Strang eröffnet (Entscheid Patrik), Sondierung erweitert:** Abkürzer VideoFrameCache/Multimedia liegt seit S59 · NEU Kamera-**Testaufnahmen** in den Settings (benutzerlokal; macht den Kamera-Pfad deterministisch prüfbar; dritte Betriebsart + Fallback) · Stilfilter-Architektur entschieden: EIN skriptbarer **`pixelFilter`**-Node + Filter als Werks-Voreinstellungen (statt Festmodule) · Filter-Fundgruben notiert (**ISF** = „Shadertoy für Filter", mögl. späterer FX-Import; ReShade/godotshaders/obs-shaderfilter; Shadertoy-Suchworte toon/cel/kuwahara/hatching/pencil/halftone/CRT; XDoG/anisotropes Kuwahara) · **iChannel-Erweiterungen** Shadertoy-Knoten: „Ketten-Eingang" + „AVS-Global-Buffer 1–8" (technisch geklärt: FBO-Pool-Texturen, activePool-Scoping) |
| 1.47.0 | 2026-08-06 | **NEU 5 Demos in asset/examples** (Konvention `<typkey> - <Name>.lvfx`): meshWarp Bass-Tunnel (Audio-Ring + Sog-Feedback), Wellengang (Waveform-Band), Spiegelkabinett (Kaleido-Scheibe: rotieren→falten→atmen); gpuParticles Feuerfontaene (Gold-Glut, Bass-Tempo), Wirbelnebel (16k-Galaxie + Schmier-Swirl = beide GPU-Module). Alle gegengerendert + sichtgeprüft (Warnungen=0); Befunde beim Stimmen: additive Partikel brauchen kräftiges Fadeout (Weiß-Ausbrand), Kaleido-Faltung braucht ZENTRIERTEN Inhalt (außermittig kollabiert das Feedback), dünne Linien verlieren Helligkeit über LINEAR-Resampling im Warp-Feedback |
| 1.46.0 | 2026-08-06 | **Voreinstellungs-Batch 1 gebaut (18 Dateien, 12 Typen):** list (Beat-Gate, Bass-Blende, Puls-Layer, AB-Wechsler — Listen-Slots ohne `time`, eigener Akkumulator), brightness (Bass-Boost, Kanal-Atmung), colorfade (Beat-Blitz, Farbdrift), colorModifier (Kontrast-Pump, Gamma-Atmung), mosaic Beat-Kachel, channelShift Beat-Rotation, colorClip Bass-Fresser, multiFilter Chrome-Beat, addBorders Puls-Rahmen, onBeatClear Vierer-Reset, clear Nachtblau-Schleier, bufferSave Echo-Speicher. Wächter-Test grün (704 Assertions). Offen: Batch 2 Scope-Figuren (texerII/triangle/superScope3D/terrain3D/glowOrbs/dotPlane/dotFountain/camera3d …) + Sichttest der Formeln |
| 1.45.0 | 2026-08-06 | **NEU Werks-Voreinstellungen** für meshWarp (5: Bass-Swirl, Tunnel-Sog, Wellengang, Fischaugen-Atmung, Spiegelkabinett) + gpuParticles (5: Fontäne, Funkenregen, Bass-Explosion, Nebel-Drift, Wirbelsturm) — Wächter-Test grün. Befund: 32 skriptfähige Typen ohne Voreinstellungen (Batch-Plan: EffectList + Farb-/Transform-Klasse zuerst, Scope-Figuren danach; milkdrop/shadertoy bewusst ohne — eigene Preset-Ebene). **NEU ⚪ §7 Szenen-Wechsler-Modul** (Idee Patrik: Szenen-IDs + Fading + Re-Init, Trigger bis Strophe/Refrain via Offline-Pre-Analyse — Konzept in eigener Session) |
| 1.44.0 | 2026-08-06 | Session 69 (Fortsetzung) — **Strang G2 GPU-Partikel UMGESETZT (⬜ Sichttest offen), Strang G damit KOMPLETT:** Chain-Node `gpuParticles` (RGBA32F-Ping-Pong pos+vel, hash-basierter Lebenslauf ohne Speicher, instanzierter Sprite-Draw max. 65536, Kraftfeld-GLSL + Parameter-Skripte), NEU `GpuParticlesWrapper.hpp` (GL-frei). FieldDocs/Inventar 88 Typen/756 Felder, 0 Lücken. `/bigobj` für test_ChainSerializer.cpp (C1128 Debug, Varianten-Wachstum). Sonde `gpuparticles_sonde.lvfx` + Render-Sichtbeweis (Fontäne). Tests 554 (+7), alle 3 Builds grün |
| 1.43.0 | 2026-08-06 | **NEU ⚪ §7:** Effekt-Palette soll die ROLLE zeigen (Render vs. Transform — Typ-Icon neben Herkunfts-Icon oder Typ-Spalte, filterbar; Add-Dropdown evtl. ersetzen). Anlass: Mesh Warp alleine = schwarz. Ausarbeitung in eigener Session (Entscheid Patrik) |
| 1.42.0 | 2026-08-06 | Session 69 (Fortsetzung) — **Strang G1 Mesh-Warp UMGESETZT (⬜ Sichttest offen):** Chain-Node `meshWarp` (Nutzer-GLSL `warp(uv)` je Gitter-Vertex, Gitter 2..256×192, Mix/Wrap, Parameter-Skripte), NEU `MeshWarpWrapper.hpp` (GL-frei) + `runMeshWarp` (transformPass-Muster, LINEAR nur je Draw + restauriert, Fehler ins geteilte `stError` → Apply-Poll), Palette „— GPU-Module —", FieldDocs/Inventar-Gates nachgezogen (87 Typen/735 Felder). **Entscheid Patrik: G1 VOR Vereinheitlichung V2** (Audio ad-hoc, Shadertoy-Muster; Plan-Doku 1.5.0). Sonde `meshwarp_sonde.lvfx` + Render-Sichtbeweis. Tests 547 (+9), alle 3 Builds grün |
| 1.41.0 | 2026-08-05 | Session 69 — **§7 Editor-Komfort UMGESETZT (⚪→⬜ Sichttest offen):** Groß-Editor mit Apply (übernehmen + recompilen ohne Schließen; Fehler IM Dialog — EEL/HLSL synchrone Transpiler-Probe, Shadertoy-GL per Nach-Polling auf `shadertoyError`) + Beautify (NEU `include/scripting/ScriptFormatter.hpp`, pur: EEL-Statement-Umbruch mit Klammertiefen-Einzug, GLSL/HLSL-Brace-Re-Indent, Whitespace-only-Vertrag) + Settings-Tab „Editor" (Einzugsbreite, Operator-Abstände, Leerzeilen-Klemme; QSettings `editor/...`) + **Import…/Export… für Shader-Felder** (Nachwunsch: Datei ↔ Editor, Vorschlag `preset_name.modul.glsl`, Ordner-Gedächtnis). EelScriptEditing 1.1.0 (`ScriptEditorHooks`), alle 3 Editor-Stellen des Multi-Chain-Panels verdrahtet. Tests 538 (+26 ScriptFormatter), alle 3 Builds grün |
| 1.40.0 | 2026-08-05 | **NEU ⚪ §7:** Editor-Komfort Multi-Chain-Panel — Apply-Button (übernehmen ohne Schließen, Fehler im Dialog) + Beautify (GLSL-Re-Indent, EEL-Statement-Umbruch mit Klammertiefen-Einzug, Milkdrop-Zeilen-Roundtrip) + Format-Settings (Einzugsbreite u. a.). Wunsch Patrik aus der Heart-Equation-Experimentier-Session |
| 1.39.0 | 2026-08-05 | Tutorial-Session (paralleler Doku-Strang) — **NEU ⚪ §7:** Shadertoy-Buffer-FBOs filtern GL_NEAREST statt LINEAR (Befund der Tutorial-Screenshot-Läufe; `lesBilinear`-Workaround in den generierten Chains, App-Fix als Backlog-Eintrag statt Task-Chip). Kontext: Shader-Tutorial-Serie komplett nach `docs/tutorials/` umgezogen (FNM-Namen), formalisiert (Tutorial_Base/Overview_Base/Reference_Base) und via AvsStandalone gegengerendert — Details in `docs/tutorials/ShaderTutorials-overview.md` |
| 1.38.0 | 2026-08-04 | Session 67 (Abschluss-Nachträge) — **Audio-Skala dB-vs-linear ✅** (Sonde + Umsetzung: Shadertoy-Audio-Textur nach WebAudio-Vertrag, ShadertoyWrapper.md 1.2.0; NEU `getspecdb()` für Chain-Skripte, Idee Patrik — LuaScriptEngine + Transpiler-Whitelist + Editor-Referenz, Test-Case 512). **Dunkelklasse ✅ ENTSCHIEDEN (Patrik): IST-SO-artig** (7+1 inkl. XorDev 001b), formales Urteil `s67b/VERGLEICH.md` (ref_shots erzeugt); ⚪ optionaler Backlog: Dossiers → gezielte Emulation bei freien Ressourcen. Rest-Bug-Liste = Gin Tonic 003 |
| 1.37.0 | 2026-08-04 | Session 67 (Fortsetzung) — **Fixklasse 9: EEL-Divisions-Vertrag, OK 295→298.** Referenz-EEL dividiert safe (Nenner 0 ⇒ exakt 0; dreifach per Sonde gegen MilkdropRef gemessen, Sonden-Methodik NEU in `asset/Milkdrop3/sonden/`), unsere Lua-Engine IEEE ⇒ NaN-Kaskaden. EelTranspiler 1.3.0 `eel.div` (nur Milkdrop-Dialekt; AVS-Sonde als neuer ⬜). **pixies ×2 GEHEILT** (Kamera-Matrix-NaN via `LUMIVIZ_MILKDROP_TRACE_VARS` gefunden), **piercing 01 geklärt** (Saatlos-Vertragsgrenze: Rausch-Input < 1/255-Quantisierung, App mit Saat korrekt), XorDev 001b lebt (Ref stirbt an anderem UB → Dunkelklasse-Entscheid). Baseline `s67b` (3 Wechsel, alle aufwärts; `s67a` = Gate Shader-Fix+Blende klassengleich). MilkdropRef-MessageBoxen → stderr (`ref_msgbox.h`, /FI). Standalone 1.1.0: `--ab`, `--blende`, `--audio-beat`. Tests 511 grün |
| 1.36.0 | 2026-08-04 | Session 67 (Zwischenstand) — **TOP 1 GELÖST:** „RTH erbt Farbe" war ein GPU-Programm-Rest: `ensureCustomPrograms()` hinter dem Quellen-Gate ⇒ beim In-Place-Wechsel auf ein Md1Default-Preset rendeten die Warp-/Comp-Programme des VORGÄNGERS weiter. Fix in MilkdropVisualizer (Aufruf unconditional, 1.22.0); Beweis MilkdropStandalone 1.1.0 `--ab` (0/300 → 300/300 bitgleiche Frames). **NEU 🟠:** Rausch-Saat beim Löschen-Wechsel sichtbar (stört; vorher von Shader-Leichen überdeckt? — Untersuchung + Entscheid a/b/c offen). Hinweis: 1.33.0–1.35.0 wurden nur im Kopf gebumpt (S65/S66), Zeilen fehlen — Historie in den Session-Changelogs |
| 1.32.0 | 2026-08-02 | Session 64 — **zweite MilkDrop-Kalibrier-Runde: acht Fixklassen, OK 289→295, Prüfstände SAATLOS** (Entscheid Patrik; `--seed` = App-Opt-in). §3 neu geschnitten: R239-Prüfpunkt ✅ aufgelöst (korrekt IST-SO), Rest = 11 Port-Bugs in vier Klassen (piercing 01 erstmals messbar · pixies ×2 · Gin Tonic 003 · q-/UB-Dunkelklasse ×7 mit 🟡 Einstufungs-Entscheid); **🎯 Regelwerk-Strang** (Legacy/Modern/Benutzerdefiniert + PS-Version-Override je Typ) als Auftrag der nächsten Session; §7 + Shadertoy-Import-Idee. Fixklassen: D3D9-Div-Vertrag (`_div`), Band-Hüllkurven, Loudness-Kaltstart-Ramp, `breakStrip` (MV-Geisterquads), UNORM8-Trunkierung, NaN-Sanitize, q-Garbage-Epsilon, Double-UV-Wrap. Forschung: 2077-Look der R-Serie = Heap-Garbage × Fixpunkt-Überlauf (doppelt UB, dokumentierte Legacy-Grenze). Tests 493/493, 5 Voll-Triagen, 0 Regressionen |
| 1.31.0 | 2026-08-02 | Session 63 (Abschluss) — §3 neu geschnitten nach der Triage (Kaltstart-Saat + Feedback-Erhalt, 10 geheilt, 22 Rest als Arbeitsliste), R239-Prüfpunkt + myPresets/-Triage + Loudness-Stille als offene Punkte aufgenommen |
| 1.30.0 | 2026-08-01 | Session 62, Teil 2 — Befund Patrik: domain.lvfx-Richtungsumkehr war ein stummer No-op (`dx`/`dy` kein domainWarp-Vertrag; alt MAE 0,0000, gefixt 0,1817 über `speed`-Vorzeichen + `oy`-Pan). Daraus: **neuer Prüfstand Strang G** `lint_chain_scripts.py` (stumme Skript-Zuweisungen in Asset-Chains; fields/README) — fand DREI Ernter-Vertragslücken (`b` beim Attraktor pauschal verworfen, Movements Feld `code` fehlte im Filter, Nur-Lese-Ziele wie Texer-II-`x`/`y` unsichtbar; alle gefixt, inventory_docs +9 Variablen, Teil-Sondenlauf 47 WIRKT/3 SCHWACH/0 STUMM). domainWarp-Vorlagen Tintenstrom/Nebelkammer tragen jetzt Skriptfelder (Beispiele regeneriert, Beat-Wirkung gemessen 0,12/0,038). Rest-Lint: 9 tote Variablen in importierten Originalen (bleiben wörtlich) |
| 1.29.0 | 2026-08-01 | Session 62 — **Grafikkarten-Auswahl umgesetzt** (§8 ✅): `core/GpuPreference` (UserGpuPreferences-Registry als SSOT), SettingsPanel-Combo mit Sofort-Neustart, `gpu.ini`/`GpuSelector`/Export-Flags entfernt, Tests 491. §8-Messlauf beidseitig gefahren: Baseline 610M (`out/gpu_baseline_radeon610m/`) und RTX 4090 (`out/gpu_after_rtx4090/`) — **keine Verschiebung**, Doppellauf-Sonden karten-übergreifend SHA256-identisch |
| 1.28.0 | 2026-08-01 | Session 61 (Abschluss) — Vorgabe Patrik für S62 in §8 eingearbeitet: **Grafikkarten-Auswahl ist der Auftrag der nächsten Session** (einmal gesetzt, via Settings umstellbar, Änderung löst SOFORT Neustart aus; §8-Regel Vorher/Nachher-Messlauf gilt). Session-Report `LumiViz_Session61…`, Handover und `Changelog_Session61.md` nachgezogen |
| 1.27.0 | 2026-08-01 | Session 61 (Sichtprüfung, zweite Welle) — **(1) SuperScope-Herz stand KOPF:** die Figuren-EEL läuft im AVS-kalibrierten Chain-Host (+y = unten), die Mathe-Formel war y-up; y im EEL negiert (Modul-SSOT `SuperscopeModule` + Vorlagen-Datei synchron, der Bibliotheks-Wächter verlangt Gleichheit; die NATIVE Rechnung des Standalone-Visualizers bleibt y-up und damit unverändert richtig). **(2) „bloom zeigt gar nichts":** der Present-Bloom war intakt — die Beispiel-SZENE war untauglich (1-px-Spirale verliert im Glow-Downsample fast alle Energie, der Threshold frisst den Rest). Bloom-Beispiele stehen jetzt auf einer hellen Lichtfaden-Bühne mit Trail; Wirkung bewiesen (Mit/Ohne-Bloom-Diff 0,134). Tests 485/485 |
| 1.26.0 | 2026-08-01 | Session 61 (Sichtprüfungs-Runde Patrik über die Beispiele) — **drei Befund-Klassen behoben:** (1) **Farb-Doppelfehler in BEIDEN Generatoren**: `avsColor` ist ein NO-OP (AVS-Farben sind bereits 0x00RRGGBB, S46) — mein R/B-Tausch beim Erzeugen verdrehte ALLE Farben (Feuer→blau, Ozean→orange), und der AvsRef-Vergleich blieb grün, weil beide Seiten dieselbe verdrehte Datei lasen. **Lehre: ein Rückabbildungs-Vergleich beweist Übereinstimmung, nicht Intention.** (2) **domainWarp stand praktisch still** — `uTime` wanderte nur ins Warp-Feld, nie in den Abtastpunkt; Shader-Fix Grunddrift (Host-Modul, kein Referenzvertrag; Bewegungs-Diff 0,006→0,025). (3) **Lebendigkeits-Pass über 15 Vorlagen** (Starfield-Dichte, Attraktor-Audio-Morphing, Lorenz-Tempo, DDM/Shift kräftiger, Burning-Ship-Ausschnitt, domainWarp-Tempi, Zoomer entschärft). **Neu notiert (⚪ §7): `fractalZoomer` erschöpft nach Minuten die float-Präzision** (Endlos-Zoom → Pixelbrei/Schwarz) — braucht einen Host-seitigen Tiefen-Reset/Loop. Tests 485/485, beide Sammlungen regeneriert und beidseitig re-validiert |
| 1.25.0 | 2026-08-01 | Session 61 (vierter Block) — **`asset/composits/`: 200 GANZE Presets in 10 Themenwelten** (je 20, el-vis/UnConeD-Bauarten: Buffer-Tunnel, Feedback-Schleifen, Beat-Physik, 3D-EEL-Projektionen). 8 Themen als Original-EEL-konformes .avs (Wurmlöcher · Plasmanebel · Kaleidoskop · Sternenstaub · Maschinenraum · Drahtgeister · Tiefsee · Farbenrausch), 2 als .lvfx mit Host-Modulen (Fraktalträume · Lichterstadt). Generator `make_composit_presets.py` (seeded, reproduzierbar). **Validiert: 200/200 rendern in LumiViz, 160/160 .avs auch in AvsRef, keine Schwarz-/Weiß-Ausreißer.** Dabei gelernte Schleifen-Hygiene im Generator dokumentiert (Echo als REPLACE mit Verstärkung < 1, Quelle ans Kettenende, Fadeout im Buffer-Kreis) und eine unvermessene Beobachtung notiert: extremes On-Beat-Mosaik (quality2 ≲ 25) kollabiert in der REFERENZ auf schwarze Block-Anker, bei uns nicht — bei Bedarf per Sonde vermessen |
| 1.24.0 | 2026-08-01 | Session 61 (dritter Block) — **`asset/examples/`: je Vorlage ein Beispiel-Preset (125, flach, Name = `<typkey> - <Vorlagenname>`)** mit Matrix-Material als Render-Quellen; Generator `asset/calibration/avs/make_example_presets.py` (Rückabbildung des kalibrierten Imports). **90 als .avs** (AvsRef-vergleichbar) · 35 als .lvfx (host-eigene Typen, Host-Konstanten-Vorlagen, LumiViz-EEL-Dialekt — die S53-Figuren Star/Starburst nutzen `mod()`/`>`, das Original-EEL nicht kennt). **Vergleichslauf über alle 90: 85 OK, 5 PRUEFEN, 0 Fehler** — 2× grain (bekannte §9-Grenze, Menge exakt) + 3× Feedback-Weichheit (s. neue §1-Zeile). Dabei zwei Vorlagen-Fixes: `flame/Horseshoe Nebula` kollabierte mit 4 Abbildungen (jetzt 3) · `mirror/Random Mirror` trug `slower: true` statt einer Zahl. Tests 485/485 |
| 1.23.0 | 2026-08-01 | Session 61 (Fortsetzung) — **Vorlagen-Fließarbeit (Strang C): 41 neue Basis-Voreinstellungen, Katalog 125 für 39 Typen.** Fraktal-Familie + Bloom komplett erstausgestattet (27, Bloom-Werte aus `lights_demo`), Histogramm-Lücken blur/simpleScope/customBpm (8), Import-Ernte-Erstling: 6 kuratierte Dynamic-Movement-Warps wörtlich aus der Sammlung (Quellen im Katalog). Ernte-Werkzeug über `AvsStandalone --dump` (89 Presets, 700+ Kandidaten für weitere Runden). Tests 485/485 (Ladbarkeits-Gate deckt alle neuen Dateien). **Host-Klassiker-Vorlagen brauchen einen Entscheid** (VisualizerPresetManager kennt keinen Asset-Ordner — Umbau nach NodePresetStore-Muster nötig, §7.3) |
| 1.22.0 | 2026-08-01 | Session 61 — **Tie Tunnel DM GELÖST: 0,074/0,098 → 0,010/0,009 (320) · 0,002/0,010 (640).** Die Farb-Phase der Tunnelbänder war der **weggeworfene Startwinkel der Dot Plane**: `r_dotpln`/`r_dotfnt` speichern ihre laufende Rotation als 8. Feld (`r = rr/32`, hier 44,84°) — Parser kannte es (`r_raw`), Translator verwarf es. Neues Feld `startRotation` (beide Params, STARTWERT-Bauart wie `interfRotationSeed`), Serializer/Panel/Tooltips/Inventar nachgezogen. Bisektionsweg dokumentiert in der §1-Zeile (Blit-Sonde auf Buffer-Slot 0 + Auslass-Sonden). Verifikation: Dot-Plane-Sonde 0,000 über alle Frames · Tie Tunnel SSC 0,001 · Matrix 01/19 pixelgenau · Zwillinge 67/67 (2 erklärte Refreezes) · Tests **485/485** · beide Builds grün. §0-Reste damit auf EINEN geschrumpft (VR09 = Indeo-Decoder, wartet auf Sichturteil mit Ton) |
| 1.21.0 | 2026-08-01 | Session 60 (Nachspiel) — **AVI-Gate referenz-treu** (Sofort-Start entfernt; 3 Gate-Sonden auf dem Testvideo 0,000; Stream-Länge 31=31 verifiziert; VR09-Rest = Indeo-Decoder-Differenz → §9-Kandidat nach Sichturteil) · **Tie Tunnel nachgemessen 0,148 → 0,074/0,098, Würfel sitzt** (dt→dt_p-Spur tot — D2 implementiert, Skripte weisen `dt` vor jedem Lesen zu); Rest = Farb-Phase der Tunnelbänder, eigene Bisektions-Session · **Basis-Voreinstellungen, erste Lieferung: 68 Vorlagen für 23 Typen** mit Katalog in `asset/nodepresets/README.md` (Teil-Presets aus den kalibrierten Referenz-Semantiken; darunter die 8 wörtlichen r_trans-Builtin-Formeln als Movement-Vorlagen; Sichtprüfung Patrik offen) |
| 1.20.0 | 2026-08-01 | Session 60 (Finale) — **§0 KOMPLETT: DIE KALIBRIER-RUNDE (S44–S60) IST ABGESCHLOSSEN.** Haken 4 erfüllt: Abschluss-Messlauf 24/24 s-Sonden + 8/8 Kern-Presets in beiden Größen ohne Regression. Endstand: Modul-Sonden 91/91 · Matrix 43/43 (41 gemessen + water/grain §9) · Feld-/Edit-Vollauf grün (0 WIRKUNGSLOS, 0 unerklärte STUMM) · Zwillinge 67/67 · Tests 485/485. Außerhalb der Haken: zwei benannte §1-Reste (Tie Tunnel · VR09-Vorschub) — Entscheid Nacharbeit vs. §9 steht aus |
| 1.19.0 | 2026-08-01 | Session 60 (Nacht) — **Haken 2, 5 und 6 sind zu.** `water`/`grain` von Patrik als bewusste Grenze abgenommen (§9) → **Matrix 43/43 erfüllt**. Werkzeug-Schulden: `srand` war seit S52 drin (Doku-Leiche §8) · **`r_avi`-Uhr virtualisiert** (`patched/r_avi.cpp`, AVI-Presets messbar — erste VR09-Messung deckt sofort den Vorschub-Befund auf: unser `avi`-Knoten kennt das `lastspeed+speed`-Gate der Referenz nicht, §1) · **wrap-Arithmetik an der Original-APE vermessen und im Shader umgesetzt** (scale 1 = Sättigung; scale ≥ 2 = (pos−neg) mod 65536, unsigned Division; zwei neue Dauersonden) — Modul-Sonden **91/91**, `convolution.edgeMode` WIRKT. Tests 485/485. Abschluss-Messlauf (Haken 4) läuft |
| 1.18.0 | 2026-07-31 | Session 60 (Abschluss) — **`04_blitter_feedback` grün, Matrix 41/43.** Zwei Befunde: blitter_out (Zoom-out) sampelt IMMER nearest (subpixel wirkt nur im Zoom-in) · der y-Anker der Sampling-Abbildung war um eine Zeile zu tief (h−1,5 statt h−0,5) und stapelte sich über die Feedback-Kaskade. 0,023/Deckung 0,72 → **0,007/Deckung 0,93** (320). Blitter-Feldsonden 8/8 WIRKT, Tests 485/485. Übrig in Haken 2: `water`/`grain` — Montagen an Patrik zur §9-Abnahme übergeben |
| 1.17.0 | 2026-07-31 | Session 60 (Fortsetzung) — **vier der fünf Flächen-Befunde PIXELGENAU gelöst, Matrix 40/43** (Menge 0,00/Deckung 1,00 je Zeile in beiden Größen): rotating_stars (r_rotstar-Port: Lokal-Peak in signed visdata-Bytes, `(s+9)/352`, rotSpeed 0,1, Erstpunkt-vs-Schleifen-Cast) · osc_star (r_oscstar-Port: 64 Schritte/Arm, laufender Byte-Index, dfactor-Hüllkurve) · osc_ring (r_oscring-Port: Spiegel-Index, negativer Winkellauf, Summen-Cast) · dot_grid (Gitter stand vertikal gespiegelt — Push ohne y-Negation). Sternchen-Segmente rastern jetzt über den linedraw-Bresenham-Port (ThickLines Breite 1) statt GL-Linien. **blitter_feedback diagnostiziert:** konstante Quell-Differenz der Blitter-Arithmetik (keine Akkumulation), 8-Bit-Port nach Blur-Muster ausstehend. Vorgaben-Pipeline komplett nachgezogen: Struct-Anker (Referenzwerte), Default-Vertrag-Test, Inventar-Golden, Ernter (719 Tooltips), Zwillinge 67/67 refreezed (3 erklärte Diffs), Sonden neu erzeugt (audioGain/bandLo begründet „nicht prüfbar": Lokal-Peak existiert im monotonen Testsignal nicht; Audio-Zweig über Matrix-Zeile 13 belegt). **Verifikation: Matrix 40/43 · Modul-Sonden 89/89 · Tests 485/485 · beide Builds grün** |
| 1.16.0 | 2026-07-31 | Session 60 — **Feld-/Edit-Sonden-Vollauf nachgeholt (Haken 3)** und er hat sich sofort bezahlt gemacht: Edit-Vollauf grün (567/127/10/0 WIRKUNGSLOS), Feld-Vollauf fand **waterBump komplett stumm** (10 Felder, vorher alle WIRKT) — Ursache ein **stilles Uniform-Loch** (`setUniformValue(QPoint)` lädt float-vec2 auf ivec2-Uniforms, `uResI=(0,0)` machte den Rand-Zweig zur perfekten Identität = bit-exakter Passthrough). Fix `glUniform2i`; danach 10/10 WIRKT **und Matrix-Zeile 31 GRÜN** (0,003/0,011 · 0,002/0,006, Deckung 0,98) — der „S59-Trail-Rest" 0,100/0,148 war dieser Passthrough, die geplante Kreislauf-Sonde ist hinfällig. Matrix **36/43**. Zweiter STUMM `convolution.edgeMode` = erwarteter Schatten der offenen wrap-Arithmetik (Haken 5). **Haken 6 komplett abgeräumt** (Entscheide S60: Hotkeys §9.2–9.5 · Playlist inkl. Default „kein automatischer Wechsel" · Lights-Lookahead→Backlog · Config-Pipeline-Abnahmetabelle gestrichen · Feldreihenfolge bleibt · Vorlagen-Konvention; Colorfade-§5-Zeile war seit S57 erledigt) · **MilkDrop-Texturen worms/rose/grad3 aus dem MegaPack beschafft** · Vorlagen-Priorisierung per Effekt-Histogramm über 3380 Korpus-Presets (§7). **Verifikation:** Feld-Sweep nach Fix 674/29/1 (der eine = edgeMode, erklärt) · waterBump-Edit-Sonden 1 GLEICH/9 TEILWEISE/0 WIRKUNGSLOS · Tests 485/485 · beide Builds grün → **Haken 3 ✅** |
| 1.15.0 | 2026-07-31 | Session 60 — **§0 Fertig-Kriterium der Kalibrier-Runde festgenagelt** (sechs messbare Haken; das Kriterium ist EINGEFROREN — neue Metrik-Funde fallen unter Haken 2 statt die Liste zu verlängern; Nach-§0-Arbeit ausdrücklich ausgelagert: Basis-Voreinstellungen, V2–V5, Video-Weg Stufe 2, Grafikkarten-Auswahl, Sichttest-Stränge §2–§4) · **water_bump entschieden (Patrik): Ganzzahl-Kern bleibt**, offen nur noch die Trail-Kreislauf-Sonde (Haken 5) · Basis-Voreinstellungen als erster Post-Kalibrier-Strang terminiert |
| 1.14.0 | 2026-07-31 | Session 59 — **Strang B komplett, Strang C komplett, D-Bestand weitgehend; Modul-Sonden ERSTMALS 89/89.** **splendora grün** (Channel-Shift-ID 1023=RGB der Original-APE; AVS-Linien am MAX-Ende exklusiv → renderThickLines = Integer-Bresenham der Referenz; linesize RUNDET) · **hypno07 grün** (Uhren-Familie: AvsRef `--tick-hz`-Frame-Uhr für gettime, `time`-Injektion nur wenn das Skript den Namen nicht besitzt, Skript-Uhr Start 0/double/ms-Körnung; **Texer-II-Resampler per Impulsantwort bit-exakt**, Default-Sprite = gemessene 21×21-Matrix — zieht `paar_original` mit) · **el-visVR09: S58-Urteil gekippt** (nach 10 s hat die Referenz Bild — `ELVIS_WAR.AVI` ist Indeo 3.2, 64-Bit-VfW kann es nicht) **und gefixt**: `services/VideoFrameCache` (Qt Multimedia/FFmpeg) als Decode-Fallback, deterministisch nach Frame-Index (Entscheid Patrik: Stufe 2 sofort; nur `"Multimedia"` in Solution.json, windeployqt deployt selbst) · **Convolution-Rand GELÖST** (Original-JIT-Quelle: Reads klemmen IMMER auf h−2/w−2; `zerostringl ≥ 24` → in-place, Rand liest bearbeitete Pixel → exakter 2-Pass; „wrap" = Arithmetik-Flag psubw/psubusw, KEIN Koordinaten-Wrap — alle 8 Sonden 0,000; **Alternate Reality dadurch echt grün 0,002**) · Randspalten-Wächter-Sonde (89.) · Zwillinge durchgesehen + refreezed 67/67 · **water_bump auf Ganzzahl-Kern** (statisch 28–48 Restpixel; Trail-Zeile offen, Entscheid Patrik) · **Flächen-Urteil (Menge/Deckung) im Matrix-Runner deckt 5 versteckte Befunde auf** (rotating_stars, osc_star, osc_ring, dot_grid, blitter_feedback) — Matrix ehrlich 35/43 · Changelogs 45+56 nachgeschrieben (Reihe lückenlos). Tests 485/485 |
| 1.13.0 | 2026-07-30 | Session 58 — **die vier unangetasteten Kalibrier-Befunde abgeraeumt: alle vier bei 640×480 gruen, drei davon auch bei 320×240.** Zuerst musste die REFERENZ reproduzierbar werden: AvsRef lud `channelshift.ape` statt seines eingebauten Effekts, und die DLL sät ihren Zufall aus der Wanduhr (sechs verschiedene Ergebnisse in acht Läufen). Danach fünf App-Befunde: **Channel Shift** zog einen Zufallszug zuviel · die **Convolution-APE** rechnet ganzzahlig in Bytes mit `bias` in 256er-Schritten, `absolute` = 255 statt Betrag, `twoPass` = Verdopplung · **Picture II** liest seinen Dateinamen aus einem festen 260-Byte-Feld · **SuperScope**-Punkte müssen double bleiben, sonst fehlt die letzte Bildspalte · **Dynamic Shift** verschiebt ganzzahlig mit hart geschwärztem Rand und vertauschtem Blend. Danach drei weitere: **der Beat gehört der LISTE, nicht dem Frame** (ein Custom BPM in einer Unterliste filterte den Beat für die ganze Kette und brachte damit den geteilten Zufallsstrom aus dem Tritt — der grösste Einzelschritt, Alternate Reality MAE 0,339 → 0,029) · **SuperScope `skip`** verwarf den Punkt, statt nur das Segment zu unterdrücken (zwei von drei Flügeln in Bright Light District fehlten ganz) · **Picture II hat sechs Betriebsarten**, nicht drei. Ergebnis in zwei Grössen (dMean / MAE): Alternate Reality 0,622 → **0,025/0,029** · **0,015/0,017 ✅** — Picture II 0,516 → **0,008/0,029 ✅** · **0,008/0,016 ✅** — Rotor 0,461 → **0,018/0,020 ✅** · **0,006/0,016 ✅** — Bright Light District 0,252 → **0,017/0,021 ✅** · **0,020/0,018 ✅**. Der letzte Schritt kam von der **Pixelmitte der Scope-Linien** (AVS zieht zwischen ganzzahligen Pixeln, GL rastert gegen Pixelmitten). Damit sind alle vier bei 640×480 grün und drei von vier auch bei 320×240; nur Alternate Reality liegt dort mit dMean 0,025 knapp über der Schwelle. Matrix 40/43 (`36_superscope` jetzt 0,000/0,000), Modul-Sonden 84/88 (fünf neue Convolution-Sonden + `scope_skip_wechsel`), Tests 485/485 grün |
| 1.12.0 | 2026-07-30 | Session 57b — **drei weitere Kalibrier-Befunde gelöst**, damit sieben von zwölf. **`41_interferences` grün** (dMean 0,025 → 0,001): die Referenz laesst eine Kopie ausserhalb des Bildes nichts beitragen, wir klemmten auf den Randpixel. **`17_dot_grid` grün** (0,047 → 0,000): 2 Pixel grosse Punkte statt einzelner, Zwischenpositionen statt ganzer Pixel, Farbmischung zu 1 statt zu 63/64. **`Dot Fountain` portiert** (0,000/0,000, Diff schwarz): 30×256-Gitter statt 400 freier Partikel. **`convolution_kante` gelöst** (px 3981 → 4540): die APE berechnet die letzte Zeile/Spalte nicht, der Zielpuffer läuft dort auf — Randbehandlung war es NICHT. **`20_water`** von Struktur- auf Rauschbefund reduziert (dMean 0,029 → 0,002). Bewiesen, aber nicht behoben: **`6_alloy/paar_original`** ist die Zufalls-Startphase (zwei neue Sonden mit festem `t` sind grün), **Tie Tunnel DM** ist KEIN Frame-Versatz (119/121 messen schlechter), **`31_water_bump`** braucht die Höhen-Skala der Referenz. Vier Fehlerklassen-Treffer derselben Art: die Referenz rechnet ganzzahlig und liest ausserhalb nichts. Matrix 40/43, Modul-Sonden 81/82, Tests 485/485 grün |
| 1.11.0 | 2026-07-30 | Session 57 (Fortsetzung) — **vier Kalibrier-Befunde aus §1 gelöst.** `fractalZoomer.feedback` ist eine echte Stärke statt eines Schalters (0,3 gegen 1,0: 0,0000 → **0,2391**). **Color-Map-Kennlinie exakt**: die APE rechnet in DREI ganzzahligen Schritten mit `step = 65536/span` als eigentlichem Verlust — **922/922** Punkte über sechs Spannweiten, vorher traf unsere Formel auf dem Graukeil 1 von 255. Die `04_span*`-Sonden lagen seit S49 unausgewertet herum; ihre Auswertung ist jetzt im Analyseskript. **Colorfade nach `r_colorfade` portiert** (löst „Zufalls-Beatmodus" UND „fehlendes `enabled`-Bitfeld" — dieselbe Sache): Bitfeld beim Import, persistenter Fader-Zustand, Nachziehen um einen Schritt je Frame samt Grün/Blau-Vertauschung, drei exklusive Beat-Zweige; ohne `slowFade` wirken die Beat-Fader nicht, wie im Original. Alle 12 Colorfade-Felder wirken. **`interferences` teilweise**: zwei belegte Abweichungen behoben (ganzzahlige Übergangswerte und Pixel-Versätze; Byte-Blendtable), 0,053 → 0,051 — der Rest ist als Kanten-Helligkeit eingegrenzt, nächster Verdacht ist die Randbehandlung. Matrix 38/43 und Tests 485/485 unverändert |
| 1.10.0 | 2026-07-30 | Session 57 — **die stummen Feld-Sonden sind bei 0** (§1c). Von den letzten 15 waren **sechs ein Befund an der App**, alle behoben: `blur.roundUp` (AVS rechnet den Blur in 8-Bit und schneidet jeden Teilterm ab, „round mode" legt +4/+5/+3 je Stärke obendrauf — wir rechneten float ohne Verlust und lasen das Feld nirgends; Vorgabe stand zudem falsch auf `true`), `grain.staticGrain` (wir zeichneten immer statisch, die Vorgabe versprach das Gegenteil), `oscRing.channel`/`oscStar.channel` (riefen `getWaveform()` ohne Kanal — die S56-Vermutung „Signal ist nicht stereo" war falsch), `texer.blend` (1 und 2 waren derselbe GL-Zustand, 50/50 fehlte), `texerII.wrapAround` (nirgends gelesen). Acht lagen am Messaufbau — darunter `camera3d.tz` mit einem **Doppeleintrag im selben Dict** (die S56-Merkregel, nochmal) und `setRenderMode.adjustAlpha`, wo „Adjustable" 7 heißt und geraten 10 dastand. Eine als **nicht prüfbar** festgeschrieben (`rotatingStars.bandHi`, mit sechs Fenstern belegt). Zwei neue Werkzeug-Tabellen (`VORLAUF_JE_FELD`, `UNTERGRUND_JE_FELD`), Tafel-Regel füllt jetzt alle Stützstellen, `convolution.kernel` misst statt zu übersteuern. **Matrix 36/41 unverändert** (keine Regression) **+ zwei neue Blur-Trail-Zeilen, beide grün** — die Matrix hatte vorher keinen Blur mit Rückkopplung, deshalb überlebte der Befund zwei Kalibrier-Runden. Tests 485/485, beide Builds grün. **Strang F nachgemessen** (§1f, Freigabe Patrik): Vollauf **558 GLEICH / 133 TEILWEISE / 11 VERDECKT / 0 WIRKUNGSLOS**, ein Befund gefunden und behoben — `interferences.rotation` ist ein **Startwert** für einen selbstlaufenden Zähler und wurde nur unter `!interfSeeded` übernommen, kam beim Editieren also nie an. Neue Fehlerklasse: **ein Startwert ist kein Parameter.** Die sechs Renderer-Fixes sind von Patrik im Betrieb abgenommen |
| 1.9.1 | 2026-07-30 | Session 56, Abschluss — **§1e neu**: der Eigenschaften-Editor muss nach JEDEM Baumumbau dastehen. Nach dem **Verschieben** blieb er leer, aus derselben Ursache wie beim Einfügen in S55: `selectPaths` setzt die Auswahl unter `m_selecting`, das Auswahl-Signal ist dabei stillgelegt, und das folgende `setCurrentItem` löst keines mehr aus — also baute niemand den Editor. Befund Patrik, behoben (Doppelschritt markieren + `buildPropertyEditor`, wie beim Einfügen). Dazu die stummen Feld-Sonden von 115 auf **15**: Buffer Blend (Gegenwert 24 wählte wie die Vorgabe den aktuellen Frame, und Modus 0 liest gar kein A), Interferences (Schlussframe mitten in den Beat-Übergang gelegt), Effect List (Init-Slot über eine geteilte Variable; das Beat-Fenster greift laut `r_list` nur bei einer AUSgeschalteten Liste) |
| 1.9.0 | 2026-07-29 | Session 56 (Fortsetzung) — **stumme Feld-Sonden 115 → 27** (§1c): ein App-Befund — die drei Alt-Format-Weichen im Deserialisierer griffen bei JEDEM Preset, das das neue Feld nicht nennt, und Roto Blitter zoomte dadurch auf 0 statt auf den neutralen 31 (Bild einfarbig, alle fünf Felder stumm). Dazu Kleinians ganzzahliges `colorScale`, Texer II ohne `n`, Custom BPMs Zähler-Phase. **Zweite Quelle der Vorgaben abgeschafft** (§1d, Entscheid Patrik): 415 Literale und 243 `getStr` beziehen ihre Vorgabe jetzt aus dem Struct, eine benannte Ausnahme bleibt. Vier neue Wächter (Zusatzfelder einer Grundkonfiguration, verwaiste Sonden, Panel-Schlüssel, Roundtrip). §10 Tooltips 717/717. Tests 485, beide Builds grün |
| 1.8.0 | 2026-07-29 | Session 56 — **§1b berichtigt:** der S55-Quercheck unterschied nicht, was er unterscheiden sollte; von den 15 `WIRKUNGSLOS` waren **7 echte Befunde** (alle gefixt) und **8 Messartefakte**. Neues Urteil `VERDECKT` samt Gegenrichtung in der Edit-Sonde. **§8:** Timescope maß 1/320 seiner Wirkung (Untergrund übermalte die Ein-Pixel-Spalten) — behoben, 8/8 WIRKT. **§10 Tooltips erledigt:** 717/717 Felder erklärt, 826 Panel-Zeilen tragen ihren Feldnamen, zwei Wächter |
| 1.6.0 | 2026-07-27 | Session 53 — Vorgaben Patrik für S54 aufgenommen (Test-Presets je Feld · Tooltips · Basis-Voreinstellungen · Feldreihenfolge) und der Movement-Befund aus `movement3b.lvfx` dokumentiert (Beat-Umkehr kann dort strukturell nicht wirken) |
| 1.5.0 | 2026-07-27 | Session 53 — Sonden-Bilanz berichtigt: **78/80** (`6_alloy/paar_original` kam dazu, am Vorstand als Altbestand belegt); Etappen 2–4 des Knoten-Parameter-Ausbaus + S50-Punkt ✅ |
| 1.4.0 | 2026-07-27 | Session 53 — Matrix-Bilanz berichtigt: **36/41** mit fünf Resten (`grain` kam dazu, war in S52 nicht mitgezählt); Etappe 1 des Knoten-Parameter-Ausbaus ✅ |
| 1.3.0 | 2026-07-27 | Session 53 — **`Dot Fountain` ist keine Portierung** (§1, 🔴), Matrix-Zeile falsch grün → faktisch 35 echte grüne; Abdeckung ✅→◐, Builtin-Bilanz 44→43 |
| 1.2.0 | 2026-07-27 | Session 53 — Knoten-Parameter-Ausbau als 🟡 aufgenommen (§8), Steuerdokument `Knoten_Parameter_Konzept.md` angelegt |
| 1.1.0 | 2026-07-27 | Session 53 — Panel-Editoren Metaballs/Tentacles erledigt (Kopfblock), neuer Kleinkram-Punkt: weitere nicht editierbare Farbtafeln |
| 1.0.0 | 2026-07-27 | Erstfassung (Session 52) — zusammengeführt aus `Offene_Implementierungen.md` (Stand S37) und `Offene_Sichttests.md` (Stand S37/38), beide überholt und entfernt, plus den aktuellen Befunden aus Handover, `MilkDrop_Import_Status.md` und `AVS_Sichttest_Protokoll.md` |
