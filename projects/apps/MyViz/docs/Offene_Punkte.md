# MyViz — Offene Punkte (Arbeitsliste)

> **Version:** 1.7.0
> **Datum:** 2026-07-28 (Session 54)
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

> ## ✅ Erledigt in S53 (war die Vorgabe Patrik aus S52)
>
> **Panel-Editoren für `Metaballs 3D` und `Tentacles 3D`** — beide Knoten haben
> jetzt volle Editoren im `MultiEffectPanel` (Metaballs: Kugelzahl, Radius,
> Tempo, Isowert, Blend · Tentacles: Zahl, Segmente, Länge, Dicke, Tempo, Blend),
> je mit editierbarer Farbtafel, und stehen in der Palette („Scopes & Sources").
> Die Farbtafel-Zeile ist dabei aus dem SuperScope-Block herausgelöst und
> geteilt worden (`addColorTable`).

---

## 1. AVS-Kalibrierung — offene Befunde mit Messwert

Metrik ist der Abstand zur Referenz (`AvsRef`), 0 = gleich. Methodik:
[AVS_Kalibrier_Methodik.md](visuals/AVS_Kalibrier_Methodik.md) — **Urteil über
gezeichnete Pixelmenge + Schwerpunkt**, nicht über dMean allein (die Metrik lügt bei
dünnen Inhalten).

| Preset / Sonde | Wert | Diagnosestand | |
|---|---|---|---|
| ~~**07 Milky Way Xtreme**~~ | **0,346 → 0,033** | ✅ derselbe Texer-Farbfix (S52), Vorstand gemessen | ✅ |
| ~~**19 High Voltage**~~ | **0,126 → 0,040** | ✅ derselbe Texer-Farbfix (S52), Vorstand gemessen | ✅ |
| ~~**15 Alien Alloy**~~ | **0,647 → 0,008** | ✅ **gelöst (S52):** Texer II setzte die Sprite-Farbe **je Punkt** auf Weiß zurück — also *nach* dem Frame-Slot, in dem dieses Preset sie berechnet (`red=sin(ct+2.07)*0.5+0.5` …). Wir zeichneten durchgehend weiße Sprites; da sie die einzige Energiequelle des Wirbels sind, lief das Bild über die Frames nach Schwarz. Gegenstück zum `sizex`/`sizey`-Befund aus S51, dieselbe Ursache an anderer Variable. Rest-MAE 0,298 = Phasenversatz, gehört zum rand-Faden | ✅ |
| **01 Picture II** | 0,516 | verarbeitet die drei Bilder falsch — unanalysiert | 🟠 |
| ~~**Custom BPM (id 33)**~~ | Zeile 9 → **0** | ✅ **gelöst (S52):** vier Abweichungen von `r_bpm.cpp` — (1) **Off-by-one**: das Original lässt jeden `skipVal+1`-ten Beat durch, wir jeden `skipVal`-ten · (2) die drei Betriebsarten sind dort **exklusiv** (jeder Zweig kehrt sofort zurück), bei uns liefen sie hintereinander und konnten sich kombinieren · (3) `skipfirst` wurde geparst, aber nie übernommen · (4) `skipval=0` heißt „jeden Beat", unsere Untergrenze 1 machte „jeden zweiten" daraus. Sonde `bpm_zaehler_skip3` jetzt MAE 0,000 | ✅ |
| ~~**Inhaler**~~ | **0,345 → 0,168** | ✅ Custom-BPM-Fix. Bisektion (S52): der Scope allein war referenzgleich (MAE 0,003), es hing am Filter davor. Der rand-**Strom** ist als Ursache ausgeschlossen (vier Sonden, MAE 0,000) | ✅ |
| ~~**Reflectosphere**~~ | **0,174 → 0,085** | ✅ derselbe Fix | ✅ |
| ~~**The Lion King**~~ | **0,021 → 0,013 (grün)** | ✅ derselbe Fix | ✅ |
| ~~**Deep Red Sea**~~ | **0,943 → 0,011** | ✅ **gelöst (S52)** — zwei Fehler: (a) der **Adjustable-Blend** war vertauscht (`v` gewichtet in `r_defs.h:250-257` den *Framebuffer*, wir gaben es der neuen Farbe) · (b) `runClear` reichte die **eigene** Clear-Aufzählung roh in `applyLineBlend`, das die BLEND_LINE-Tabelle erwartet — aus „50/50 gegen Schwarz" wurde „MAX gegen Schwarz", also ein **No-op**, das Bild klang nie ab. Zieht mit: Wtf I'm Lost 0,094 → 0,003 · High Voltage 0,114 → 0,005 | ✅ |
| **Alternate Reality** | ~0,28 → ~0,68 | 🔴 **Neu (S52): wird durch den korrekten Clear SCHLECHTER** (Vorstand dreimal gemessen). Das Preset klart 50/50 gegen ein helles Orange; solange unser Clear ein No-op war, lag der Hintergrund zufällig näher an der Referenz. Jetzt sättigt er pink, die Referenz bleibt **weiß** — bei identischer Labyrinth-Struktur. Der kaputte Clear hat hier einen **zweiten** Fehler verdeckt; Augenmerk auf die Listen-Blends um den Clear | 🔴 |
| ~~**greatwho2006 15/16**~~ | Symmetrie **0,042 → 0,0000** | ✅ **gelöst (S52), Befund Patrik „sollten sauber spiegeln":** Mirror wertete alle vier Richtungen in EINEM Shader-Durchgang aus der unveränderten Textur aus, die Regeln überschrieben sich. Das Original läuft vier Schleifen nacheinander (`r_mirror.cpp` 167/188/210/230), jede sieht das Ergebnis der vorigen — daher ergeben zwei aktive Achsen dort ein symmetrisches Bild. Jetzt ein Durchgang je Richtung in Referenz-Reihenfolge | ✅ |
| **30 Bright Light District** | 0,252 | Bruch bei Stufe 2 lokalisiert (Dynamic Shift schiebt ein 4-Pixel-Saatkorn im persistenten Puffer) — **nicht bewiesen** | 🟠 |
| **P3_HpR20 Rotor** | 0,461 | Rotor-Rest seit S48 (0,37 → 0,12 ◐, dann Rest) | 🟠 |
| **Tie Tunnel DM** | 0,154 | Altbestand seit S49 | 🟠 |
| **Sonde `convolution_kante`** | ~560 px | ≈ eine Zeile plus eine Spalte. `scale` geprüft (acht Kennlinienpunkte exakt), Kern seit S50 richtig orientiert | 🟠 |
| **Sonde `6_alloy/paar_original`** | 39546 → 37671 px | 🟠 **Neu gesehen (S53).** Menge 0,05 · Lage 2,7 · MAE 0,010 — dreimal identisch gemessen, also kein Rauschen, und **am Vorstand belegt**: derselbe Wert mit gestashtem Renderer, die Sonde war schon vor S53 rot. Der S52-Stand „Modul-Sonden 79/80, offen nur `convolution_kante`" war ungenau — es sind **78/80**. Uns fehlen ~1900 px gegenüber der Referenz | 🟠 |
| **Modul-Matrix-Reste** | **36/41** | `dot_grid` · `water` · **`grain`** · `water_bump` · `interferences`. **Korrektur S53:** der S52-Stand „37/41, vier Reste" war ungenau — `24_grain/01_static100` ist gelb (dMean **0,000**, MAE **0,046**, dreimal identisch gemessen, also kein Rauschen). Die Montage zeigt beide Seiten deckungsgleich, der 4×-Diff ist ein **gleichmäßiges Flächenrauschen**: die Kornmenge stimmt, der Zufallsstrom ist gegen die Referenz versetzt. Kein Strukturfehler — und es betrifft **statisches** Grain, der 🔧-Punkt unten meint das nicht-statische | 🟠 |
| **`Dot Fountain` ist nicht portiert** | 0,002 — **falsch grün** | 🔴 **Neu (S53).** Die Referenz `r_dotfnt.cpp` ist ein **30×256-Gitter** (7680 Punkte, rotierende Höhenwand, 3D-Matrix `translate(0,-20,400)` wie `Dot Plane`, Höhe aus dem Spektrum). Unser Renderer sind **400 freie Partikel** mit eigener Physik — der Header sagt es selbst („Simplified particle model here"). Die Montage zeigt links einen hohen geordneten Brunnen über die volle Bildhöhe, mittig einen flachen Fleck von ~⅕ der Fläche; der 4×-Diff **ist** das Referenzbild. Die Matrix-Zeile `19_dot_fountain` misst trotzdem 0,002 und zählt zu den 37 — **die Metrik lügt bei dünnen Inhalten**, beide Bilder sind überwiegend schwarz. Faktisch also **36/41**. Fix = echte Portierung nach dem `Dot Plane`-Muster (Matrix + Farbtabellen-Arithmetik liegen dort zeilengenau vor); zusätzlich braucht die Zeile ein **flächenbasiertes** Urteil, sonst bewacht sie weiter nichts | 🔴 |
| Color-Map-Kennlinie | ±1 | Altbestand | 🔧 |
| Colorfade-Zufalls-Beatmodus | — | Altbestand | 🔧 |
| nicht-statisches Grain | — | zieht inhaltsabhängig → kippt die rand-Ausrichtung des ganzen Presets (S49-Merkregel) | 🔧 |

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
kein Reset). Urteil über **drei** Bilder — geladen · editiert · Vorgabe:
`WIRKUNGSLOS` = editiert ist Pixel für Pixel die Vorgabe (harter Befund) ·
`TEILWEISE` = wirkt, trägt aber noch die Vorgeschichte (bei Effekten mit
Verlauf der Normalfall) · `GLEICH`.

**Stand:** 591 GLEICH · 101 TEILWEISE · 15 WIRKUNGSLOS (707 Sonden).

| Feld | MAE | Stand |
|---|---|---|
| ~~`movement.sourceMapped`~~ | 0,081 | ✅ **gefixt S55** — wurde nur bei frischer Runtime übernommen (`< 0`). Jetzt wird der zuletzt übernommene Preset-Wert mitgeführt, das Beat-Kippen bleibt. Nachgemessen: Movement 7/7 GLEICH |
| ~~`avi.filename` · `avi.resolvedPath`~~ | 0,234 | ✅ **gefixt S55** — `aviTried` merkte sich nur, DASS geöffnet wurde; ein Pfadwechsel griff nie. Jetzt Pfad-Schnappschuss + Neuöffnen (Textur wird verworfen). Nachgemessen: keine WIRKUNGSLOS mehr, Feld-Sonden 6/6 ohne Regression |
| `texer.imageData` | 0,070 | 🟠 vermutlich derselbe Typ (Textur einmal aufgebaut) |
| `milkdrop.meshX` · `meshY` · `debugGrid` | 0,036–0,053 | 🟠 Mesh/Gitter wird im Milkdrop-Modul aufgebaut — prüfen, ob `setParam` je Frame reicht |
| `bufferSave.slot` · `dir` · `initCode` · `frameCode` · `beatCode` | 0,109 | 🟡 **wahrscheinlich Messartefakt:** der Untergrund ist statisch, also enthalten alle Puffer-Slots dasselbe Bild; ein Slot-Wechsel ist dann unsichtbar, während er im geladenen Fall von Anfang an gilt. Erst die Montage ansehen |
| `bassSpin.smoothing` | 0,184 | 🟠 unanalysiert |
| `customBpm.skip` | 0,109 | 🟡 zählt Beats, greift erst im nächsten Zyklus — vermutlich kein Fehler |
| `mirror.slower` | 0,014 | 🟡 steuert nur das Tempo einer Rampe, die nach 90 Frames abgelaufen ist — ein neuer Wert kann am Schlussbild nichts mehr ändern; vermutlich kein Fehler |

**Merkregel:** `WIRKUNGSLOS` ist wie `STUMM` **eine Frage, kein Befund** — es
kann am Runtime-Zustand liegen (Fehler) oder daran, dass das Feld zu diesem
Zeitpunkt schlicht nichts mehr bewirken kann (kein Fehler).

**Geprüft und in Ordnung:** alle 16 Knoten mit Verlauf (multiDelay, videoDelay,
bufferSave, blitterFeedback, rotoBlitter, waterBump, water, fyrewurX,
movingParticle, bassSpin, timescope, avi, customBpm, reactionDiffusion,
fractalZoomer, milkdrop) übernehmen ihre Felder **unbedingt je Frame** — keine
einzige bedingte Übernahme (statische Prüfung S55, Vorgabe Patrik).

**Entscheid Patrik S55:** einzeln je Knoten reparieren, nicht generisch. Ein
generisches Verwerfen des Aufbau-Zustands bei jedem Reglerdreh würde Skripte
neu übersetzen und Bilder/Videos neu laden — das kann beim Ziehen ruckeln, und
die Grenze zwischen „Aufbau" und „Verlauf" müsste je Knoten von Hand gezogen
werden.

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

| Was | Stand | |
|---|---|---|
| **`rot_*`-Matrizen optisch** | seit S52 im Code (24 Matrizen + Matrix-Indizierung), 9 Presets im Pack betroffen. Werte sind zufällig und unser PRNG ist ein anderer als der von MilkDrop — prüfbar ist nur: laden sie durch, bewegt sich Plausibles | ⬜ |
| **Texturen `worms`, `rose`, `grad3`** | 27 von 35 Zeilen im Fehler-Log vom 2026-07-27. Existieren **nirgends** im Projekt, auch nicht im Original-Winamp-Pack (dort 21 Texturen, alle vorhanden seit `onefish.jpg` in S52). Entweder beschaffen oder als „nicht im Pack" abhaken — solange übertönen sie im Log alles andere | 🟡 |
| **In-App-Sichttest-Runde** | c1- + m5-Presets über den Node-Pfad, Panel-Baum + Editor-Sektionen, Session-A-Features (Wave/Shape/Sprite anlegen/entfernen/klonen, Sprite-Editor, fShader-Wash). Offen seit S42 — [Status Punkt 0](visuals/MilkDrop_Import_Status.md) | ⬜ |
| **Decay-Dither** + **`.milk`-Export** | Punkt 8, ganz ans Ende | 🟡 |
| **Playlist-Anbindung** | hängt an E6 (§6) | 🟡 |
| Host-Gruppen-Feinschliff | exakter paarweiser 2er-Mix statt sequentiellem Adjustable | 🔧 |

## 4. Sichttests, die nie stattgefunden haben

| Was | Umfang | |
|---|---|---|
| **Batch H — 9 Fraktal-Module** | Fractal 2D (9 Typen) · Fractal 3D (Raymarch-DE) · Domain Warp · Fractal Zoomer · Lyapunov · Kleinian · Strange Attractors · Flame/IFS · Reaction-Diffusion. Gebaut in S37, Unit-Tests grün, **GL-Sichttest komplett offen**. Kalibrierpunkte je Modul: Färbung/Banding, Kamera-Defaults, feed/kill, Reseed bei Divergenz, Punktzahl vs. Helligkeit. Querschnitt: Gradient-Paletten je Modul, Blend über die Kette, Audio-EEL-Reaktion | ⬜ |
| **FeedbackBuffer / `post.trail.*`** | Roadmap 4.3 — Sichttest (Trail-Look, Resize, Undock/Vollbild) + **Frametime-Vergleich** ausstehend. Einziges Modul-Doku mit offenem Status ([FeedbackBuffer.md](../include/visualizers/render/FeedbackBuffer.md)) | ⬜ |
| **BeatEstimator** | Roadmap 4.4 — Beat-Stabilität am laufenden Bild | ⬜ |
| **Multi-Drag zwischen Listen** | Block-Reparenting, Index-Mathematik nur compile-verifiziert; bei Bugs auf „Reorder in gleicher Ebene" beschränken | ⬜ |
| **Stereo `getspec`/`getosc`** | ch=1 (L) vs. ch=2 (R) getrennt? Nur compile-verifiziert. ⚠ **Hörtest**. Stellschrauben: `BASS_DATA_FFT_INDIVIDUAL`-Layout (`bin*chans+ch`) + Waveform-Interleaving | ⬜ |

## 5. Offene Entscheide

| Quelle | Entscheid | |
|---|---|---|
| [Hotkey_Konzept §9](ui/Hotkey_Konzept.md) | §9.2 Blättert Stufe 1 in Unterordner hinein? · §9.3 Am Verzeichnisende halten oder umlaufen? · §9.4 Verhalten bei Mehrfachauswahl · §9.5 Zeigen Menü-Einträge die Tasten? *(§9.1 in S52 entschieden: `Bild ab` = vorwärts)* | 🟡 |
| [Visual_Playlist §6](ui/Visual_Playlist_Konzept.md) | Pfad-Referenzen vs. eingebettete `.lvfx` · Auslöser-Default (Songwechsel vs. Timer) · Beat-Quantisierung des Timer-Wechsels · Import-Browser-Erweiterung jetzt oder mit der Playlist *(Hotkey-Frage ist nach Hotkey_Konzept ausgelagert und dort beantwortet)* | 🟡 |
| [Lights_Module_Entwurf](visuals/Lights_Module_Entwurf.md) | Entscheid 3: BASS-Lookahead als eigener Service (`AudioLookahead`) — Umfang/Session ungeplant. Bis dahin reichen Beat-Prädiktion + `gettime()` | 🟡 |
| [Parameter_Reference §10](visuals/Parameter_Reference.md) | Deklarierte Preset-Defaults vs. Dropdown-Indizes bereinigen · `solidColor`/`peak.color.fixed` ohne deklarierten Default | 🔧 |
| MilkDrop-Texturen | siehe §3 | 🟡 |
| ~~**Multi Delay: wem gehört die Verzögerung?**~~ (Befund S55) | ✅ **entschieden und umgesetzt (Patrik S55): Puffer-Besitz, original-treu.** Der Ausgabe-Knoten liest jetzt den **ältesten** Frame des Rings (`head`), sein eigenes `delay` wirkt nicht mehr — wie `outpos[buffer]` im Original. Bis dahin änderte das `delay` des Schreibers nur die Ringgröße und blieb unsichtbar. Vorstand gemessen: ungleiche Werte kann es im Original **gar nicht geben** (jeder Knoten speichert alle sechs Puffer-Einstellungen und schreibt sie global, `r_multidelay.cpp:387-401`), im Referenz-Korpus nutzen **2** Presets den Effekt, in eigenen `.lvfx` nur die Sonden | ✅ |
| **Colorfade: `enabled`-Bitfeld fehlt** (Befund S55) | Drei Schalter des Originals fehlen (an · „on beat random" · „slow fade"). Folge: unsere Beat-Fader wirken **immer**, im Original nur bei „slow fade"; die Zufallsfader gibt es gar nicht, und die Annäherungsrampe (1 Schritt je Frame) ebenfalls nicht (`r_colorfade.cpp:142-168`). Nachtragen oder als bewusste Vereinfachung festschreiben — Details in [Import_Modul_Abdeckung §11](visuals/Import_Modul_Abdeckung.md) | 🟡 |
| [Config_Pipeline_Umsetzungsplan](visuals/Config_Pipeline_Umsetzungsplan.md) | **Formale Abnahme**: A1–A8 und N1–N7 stehen sämtlich auf `⬜`, obwohl die Schritte 0–7 als ✅ gelten und die Sichttests 5.1–5.5 abgenommen sind. Entweder nachträglich abhaken oder die Tabelle als erledigt streichen | 🔧 |

## 6. Konzept-Phasen, noch nicht begonnen

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
- 🟡 **P2 Visual-Playlist** — hängt an den Entscheiden §5.
- 🔧 **P1 Set Render Mode auf alle Scope-Effekte** — durch S45/S3 weitgehend erledigt
  (`drawScopeShape`, `drawDots` zeichnen über den SRM-Zustand); als Punkt nie
  formal geschlossen. Prüfen und schließen.
- ⚪ **Hotkeys Stufe 2/3** — Stufe 2 (Transport) ist in S52 verdrahtet; Stufe 3
  (Composer-Spuren) ist Fernziel.

## 7. Backlog (bewusst nichts tun)

### ⚪ Video-/Kamera-Quellmodul + Stilfilter (Wunsch Patrik S55)

**Zeitpunkt: später** — ausdrücklich nicht in der Kalibrier-Runde. Hier steht nur,
was bei der Sondierung schon feststeht, damit es nicht noch einmal erhoben wird.

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
- Abgrenzung: **nicht** das Video-**Capture**-Modul (Aufnahme von Bild+Ton) —
  das ist der Punkt unten in derselben Liste, umgekehrte Richtung.

⚪ Preset-Warmup/Pre-Roll (bei Laden/Resize N Frames vorrechnen — gegen
Schwarz-Start/Flackern) · Custom-Functions-Modul · Video-Capture-Modul ·
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

1. 🔴 **Test-Presets für JEDES Modul und JEDES Feld** (Strang E, Konzept §9). Ein
   Preset je Feld, nur dieses eine vom Default abweichend; für Transformationen
   mit klar definiertem statischem Untergrundbild wie auf der MilkDrop-Seite.
   Urteil = zwei Läufe (Default vs. gesetzt); kein Unterschied heißt: das Feld
   wirkt nicht. Nach S53 dringlich — 47 Renderer haben je drei neue Skriptfelder,
   und `movement3b.lvfx` hat gezeigt, dass ein Feld dastehen kann, ohne wirken zu
   können (§9 unten).
2. 🟡 **Tooltip an JEDEM Feld** (Strang F, Konzept §10). Skriptfelder nennen ihre
   schreibbaren Variablen — die stehen heute nur im Doxygen-Kommentar. Text als
   Tabelle `{typkey, feldname} → Text` statt im Code verdoppelt, plus Gate gegen
   Felder ohne Eintrag.
3. 🟡 **Basis-Voreinstellungen für alle Module** (Strang G, Konzept §11) — aus
   `…\cmake\VisualsPresets` abgreifen, wo möglich. Umfang und Namenskonvention in
   S54 festlegen; **Teil-Presets** (nur Geometrie, Farbtafel bleibt) sind
   brauchbarer als solche, die alles überschreiben.
4. 🟡 **Feldreihenfolge im Editor** — Vorschlag Patrik: Init · Beat · Frame ·
   Point. Heute Init · Frame · Beat · Point, das folgt der Ausführung
   (`runParamScript` und alle Skript-Träger fahren Frame VOR Beat, wie AVS).
   Entscheiden, ob die Anzeige der Ausführung folgt oder der Erwartung.

### 🔴 Movement: Beat-Umkehr kann strukturell nicht wirken (Befund Patrik S53)

`asset/effectchain/movement3b.lvfx` setzt `bt=1` (Init), `bt=(-1)*bt` (Beat) und
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

## 8. Werkzeug- und Doku-Schulden

- 🔴 **AvsRef deterministisch machen** (S22): nach dem Laden eines Presets ein festes
  `srand(<konstant>)` setzen. Das neutralisiert den `srand(time(0))` aus
  `r_chanshift.cpp:load_config` und alles, was sich beim Laden aus demselben CRT sät.
  Solange das offen ist, hat jeder Vergleich eine Rauschgrenze von ~0,06 MAE, und
  Einzelwerte sind nur mit Mehrfachmessung deutbar. AvsRef ist unser eigener Build und
  hat bereits ein `patched/`-Verzeichnis — der Eingriff ist klein.
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
- 🔴 **Grafikkarten-Auswahl in den Einstellungen** (Vorgabe Patrik, S54) —
  **prioritär, aber ERST NACH der Kalibrier-Runde.** Der Rechner hat eine
  RTX 4090 Laptop GPU; App, Standalone und GL-Tests laufen trotzdem auf der
  `AMD Radeon(TM) 610M` (der Standalone meldet sie beim Start). Gewünscht ist
  eine **persistent gespeicherte** Auswahl.
  - Zur Laufzeit lässt sich die GPU in OpenGL nicht umschalten. Der Weg für
    eine einstellbare Präferenz ist der Windows-Eintrag pro Anwendung
    (`HKCU\…\DirectX\UserGpuPreferences`, dasselbe was die Windows-Oberfläche
    schreibt) — greift beim nächsten Start. Der statische
    `NvOptimusEnablement`-Export wirkt sofort, ist aber nicht einstellbar.
  - **Warum erst nach der Kalibrierung:** ein GPU-Wechsel ändert Interpolation
    und Rundung im Treiber, also möglicherweise die Bilder. Matrix (36/41),
    Modul-Sonden (78/80), die eingefrorenen Zwillinge und die Bit-Identität der
    Feld-Sonden müssten danach **alle neu eingemessen** werden. Der Umbau
    gehört deshalb mit einem Vorher/Nachher-Messlauf zusammen, der die
    Verschiebung beziffert.
  - Umfang: Auswahl „Automatisch / Hohe Leistung / Energiesparen", Anzeige der
    tatsächlich genutzten GPU (`GL_RENDERER`), Hinweis auf den nötigen Neustart.
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
  App-Umbenennung **MyViz → LumiViz** · Pulsing-Defaults-Mismatch ·
  `File → Open Audio…`-Stub · Undock-Dauertest · Waveform-Glättungs-Default.

## 9. Bewusste Grenzen — kein Handlungsbedarf

| Bereich | Grenze |
|---|---|
| AVS-Builtins | SVP Loader (10) = externe UVS/SVP-DLL, nicht decodierbar |
| AVS-APEs | 5 verworfen: GeissFluid · ParticleSystem · MIDI Trace · AVI Player · AVSTrans Automation (closed-source bzw. Meta) · Framerate Limiter = no-op (der Host taktet) |
| HLSL-Transpiler | `#elif` und Nicht-Literal-`#if` → sauberer Fehler, MD1-Fallback wie im Original |
| MilkDrop-Referenz | GPU-Rendering ist nicht bit-deterministisch — Vergleich über Statistik/Montagen, nie Pixelgleichheit |

## 10. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.6.0 | 2026-07-27 | Session 53 — Vorgaben Patrik für S54 aufgenommen (Test-Presets je Feld · Tooltips · Basis-Voreinstellungen · Feldreihenfolge) und der Movement-Befund aus `movement3b.lvfx` dokumentiert (Beat-Umkehr kann dort strukturell nicht wirken) |
| 1.5.0 | 2026-07-27 | Session 53 — Sonden-Bilanz berichtigt: **78/80** (`6_alloy/paar_original` kam dazu, am Vorstand als Altbestand belegt); Etappen 2–4 des Knoten-Parameter-Ausbaus + S50-Punkt ✅ |
| 1.4.0 | 2026-07-27 | Session 53 — Matrix-Bilanz berichtigt: **36/41** mit fünf Resten (`grain` kam dazu, war in S52 nicht mitgezählt); Etappe 1 des Knoten-Parameter-Ausbaus ✅ |
| 1.3.0 | 2026-07-27 | Session 53 — **`Dot Fountain` ist keine Portierung** (§1, 🔴), Matrix-Zeile falsch grün → faktisch 35 echte grüne; Abdeckung ✅→◐, Builtin-Bilanz 44→43 |
| 1.2.0 | 2026-07-27 | Session 53 — Knoten-Parameter-Ausbau als 🟡 aufgenommen (§8), Steuerdokument `Knoten_Parameter_Konzept.md` angelegt |
| 1.1.0 | 2026-07-27 | Session 53 — Panel-Editoren Metaballs/Tentacles erledigt (Kopfblock), neuer Kleinkram-Punkt: weitere nicht editierbare Farbtafeln |
| 1.0.0 | 2026-07-27 | Erstfassung (Session 52) — zusammengeführt aus `Offene_Implementierungen.md` (Stand S37) und `Offene_Sichttests.md` (Stand S37/38), beide überholt und entfernt, plus den aktuellen Befunden aus Handover, `MilkDrop_Import_Status.md` und `AVS_Sichttest_Protokoll.md` |
