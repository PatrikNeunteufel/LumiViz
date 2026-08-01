# MyViz — Offene Punkte (Arbeitsliste)

> **Version:** 1.24.0
> **Datum:** 2026-08-01 (Session 61)
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
| **Blitter-Zoom-in: Trail weicher als Referenz** | Beispiele 0,062–0,163 | 🟠 **NEU (S61, Beispiel-Sweep):** drei der 90 .avs-Beispiele fallen auf — `blitterFeedback - Sog nach innen` (0,163/MAE 0,143), `Beat-Sprung` (0,062) und `dynamicMovement - Molekuel-Raster` (0,060). Die Montagen zeigen ÜBERALL dieselbe Struktur und Farbe, unser Bild ist über die Rückkopplung nur **weicher** (Ringe verschmieren, die Referenz bleibt scharf) — der Zoom-IN-Pfad (subpixel/BLEND4) verliert je Durchgang etwas mehr Hochfrequenz, die Trail-Szene stapelt das über 120 Frames. Kein Strukturfehler; die Matrix-Zeile 04 (Grundkonfig) blieb grün, sichtbar wird es erst bei DAUER-Zoom-in. Kandidat: Bilinear-Gewichte/Rundung des blitter_normal-Pfads gegen r_blit BLEND4 vermessen (Bauart wie der S60-Zoom-out-Befund). Metrik-Bremse: fixen oder §9 mit Abnahme | 🟠 |
| ~~**Sonde `convolution_kante`**~~ | 4547/3981 → **4547/4540 px** | ✅ **gelöst (S57).** Die APE berechnet die **letzte Zeile und die letzte Spalte nicht** — ein Off-by-one ihrer Schleife. Der Zielpuffer behält dort seinen alten Inhalt, und weil er im Wechsel wiederverwendet wird, läuft der Wert über die Frames auf: in der Referenz stand dort **255** (gesättigt nach mehrfacher 8×-Verstärkung), bei uns jedes Mal frisch **128** = 8 × 16. Der Weg dorthin: die 560 Pixel im Bild verorten (Zeile 239 + Spalte 319, je vollständig), dann mit der **Identitäts-Sonde** belegen, dass das Eingangsbild identisch ist, und über den Kern (`[0]*24 + [8] + [0]*24` — nur die Mitte) die Randbehandlung ausschliessen. Modul-Sonden **78/80 → 79/80**. Nebenwirkung: `convolution.edgeMode` fällt von 0,0014 auf 0,0007 (SCHWACH), weil der Rand-Unterschied jetzt nur noch links und oben entstehen kann — erklärbar, kein Befund | ✅ |
| ~~**Sonde `6_alloy/paar_original`**~~ | 39546/37671 → **grün** | ✅ **gelöst (S59) — durch den bit-exakten Texer-II-Resampler**, nicht durch die S57-These (rand-Startphase): die Sprite-Rasterung selbst wich ab (Subpixel-Phase, Malbreite). Ursprünglich: 🟠 **S57: Ursache bewiesen — die ZUFALLS-STARTPHASE.** Die Kette ist Dynamic Movement + Texer II, und ihr Init lautet `t=rand(100)/50`. Zwei neue Sonden mit **festem** `t` (`paar_t_fest_082`, `paar_t_fest_2`) sind **grün** (Menge 0,02 · Lage 0,5 bzw. 1,5) — bei gleichem Startwert rechnen beide Seiten also gleich, und Physik wie Darstellung scheiden aus. Unser `rand()` ist der exakte MSVC-LCG mit Seed 1 (erster Zug 41 → `t = 0,82`); es steht beim Init dieses Effekts nur an einer anderen Stelle des Stroms als bei AvsRef. **Nächster Schritt:** bestimmen, wie viele Züge die Referenz vor dem ersten Effekt-Init macht (Konstruktoren ziehen — Grain zieht 491+1, S49) und unseren Strom entsprechend ausrichten | 🟠 |
| **Modul-Matrix-Reste** | **40/43** (S60) | **S60: vier der fünf Flächen-Befunde PIXELGENAU gelöst** (je Menge 0,00/Deckung 1,00 in beiden Größen): **`13_rotating_stars`** — exakter r_rotstar-Port: Sterngröße aus dem größten LOKALEN Peak der rohen visdata-Bytes (Bedingung: beide Nachbarn um >4 unterboten, in SIGNED char — Bytes >127 zählen nie), Größe `(s+9)/352` je Achse, `rotSpeed` 0,1, und die Cast-Eigenheit: der ERSTE Eckpunkt entsteht mit getrennten `(int)`-Casts, die Schleifen-Ecken mit EINEM Cast über die Summe (floor statt trunc bei negativem Anteil) · **`02_osc_star`** — exakter r_oscstar-Port: 64 Schritte je Arm, fortlaufender Byte-Index über die 5 Arme, dfactor-Hüllkurve 1/1024→~1/128, `is=min(w·s,h·s)` ganzzahlig · **`14_osc_ring`** — exakter r_oscring-Port: 80 Segmente, Spiegel-Index `q>40?80−q:q`, negativer Winkellauf, EIN impliziter Cast über die Summe · **`17_dot_grid`** — das Gitter stand VERTIKAL GESPIEGELT (Push ohne y-Negation vor `avsZeile`); bei spacing 8 traf nie eine Zeile, dMean sah es nicht. Alle Sternchen-Segmente laufen jetzt durch den linedraw-Bresenham-Port (ThickLines, Breite aus SRM) statt GL-Linien. **`04_blitter_feedback` GRÜN (S60, zwei Befunde):** (1) **blitter_out (Zoom-out) sampelt IMMER nearest** (`src[s_x>>16]`, r_blit.cpp:143) — der `subpixel`-Schalter wirkt nur im Zoom-in (`blitter_normal`/BLEND4); unser Shader bilinear-filterte auch im Zoom-out (konstante Kanten-Differenz ab Frame 2, KEINE Akkumulation — die anderen MAT_TRAIL-Zeilen waren grün). (2) **y-Anker der Abbildung war h−1,5 statt h−0,5** — jede Quellzeile lag eine AVS-Zeile zu tief, der Fehler stapelte sich über die Feedback-Kaskade (Deckung 0,72 → 0,93). Jetzt 0,002/0,007 (320) · 0,001/0,004 (740), Menge 0,01. Restdiff = verstreute Einzelpixel entlang der Waveform (Messerkanten über die Rückkopplung). **Offen (2):** `water` (Chaos-Rauschen, Deckung 0,96) · `grain` (Zufallsstrom-Versatz, Deckung 0,56) — §9-Kandidaten, Abnahme Patrik angefragt (S60, Montagen übergeben). Historie S59: das Flächen-Urteil deckte die fünf Befunde auf (Stand davor 40/43 nach Mittelwert allein) — dünne Zeichner, die die Mittelwert-Metrik nie sehen konnte: `13_rotating_stars` (Ref: winzige Punkt-Sterne, wir GROSSE — Deckung 0!) · `02_osc_star` (Deckung 0,02) · `14_osc_ring` (0) · `17_dot_grid` (gleiche Menge, NULL Überlapp — Verdacht Bewegungs-Akkumulation) · `04_blitter_feedback` (grenzwertig 0,30/0,72). Dazu die drei Altbestände `water` (Chaos-Rauschen) · `grain` (Zufallsstrom-Versatz) · `water_bump` (Trail-Zeile, s. eigener S59-Punkt). Stand vor dem Flächen-Urteil: 40/43. Historie S57: **`dot_grid` ist grün** (dMean 0,047 → **0,000**): wir zeichneten 2 Pixel grosse Punkte statt einzelner, setzten sie auf Zwischenpositionen statt auf ganze Pixel (`xp`/`yp` sind 8.8-Festkomma) und interpolierten die Farbe zu 1 statt zu 63/64. **`interferences` ist GRÜN** (0,053 → **0,027**, dMean 0,025 → **0,001**): die Referenz laesst eine Kopie ausserhalb des Bildes **nichts** beitragen, unser Shader klemmte auf den Randpixel und schmierte ihn nach innen. **`water` erheblich verbessert** (dMean 0,029 → **0,002**, MAE 0,069 → 0,055): Nachbarn ausserhalb werden weggelassen statt geklemmt, und die Halbierung ist ganzzahlig — bei einem rueckgekoppelten Effekt bleibt ein Randfehler nicht am Rand. Der Rest ist **kein Strukturfehler**: die Montage zeigt beide Seiten deckungsgleich, der Diff ist feines Rauschen ueber den Wellenzonen — Chaos-Verstaerkung wie bei `24_grain`. Zwei neue Zeilen kamen hinzu und sind beide grün: `06_blur/02_trail_rounddown` und `03_trail_roundup` (§1c). **Korrektur S53:** der S52-Stand „37/41, vier Reste" war ungenau — `24_grain/01_static100` ist gelb (dMean **0,000**, MAE **0,046**, dreimal identisch gemessen, also kein Rauschen). Die Montage zeigt beide Seiten deckungsgleich, der 4×-Diff ist ein **gleichmäßiges Flächenrauschen**: die Kornmenge stimmt, der Zufallsstrom ist gegen die Referenz versetzt. Kein Strukturfehler — und es betrifft **statisches** Grain, der 🔧-Punkt unten meint das nicht-statische | 🟠 |
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
| `bufferSave.slot` · `dir` · `initCode` · `frameCode` · `beatCode` | 0,109 | ⬜ **kein Befund (S56)** — `VERDECKT`, s. u. |
| `bassSpin.smoothing` | 0,184 | ⬜ **kein Befund (S56)** — `VERDECKT` |
| `customBpm.skip` | 0,109 | ⬜ **kein Befund (S56)** — `VERDECKT` |
| `mirror.slower` | 0,014 | ⬜ **kein Befund (S56)** — `VERDECKT` |

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

| Was | Stand | |
|---|---|---|
| **`rot_*`-Matrizen optisch** | seit S52 im Code (24 Matrizen + Matrix-Indizierung), 9 Presets im Pack betroffen. Werte sind zufällig und unser PRNG ist ein anderer als der von MilkDrop — prüfbar ist nur: laden sie durch, bewegt sich Plausibles | ⬜ |
| ~~**Texturen `worms`, `rose`, `grad3`**~~ | ✅ **beschafft (S60):** im Original-Winamp-Pack fehlen sie tatsächlich (21 Texturen, geprüft gegen `ref/winamp_orig`), aber der **MegaPack-`textures`-Ordner** (`…\VisualsPresets\MilkDrop 135k+ Presets MegaPack 2026\textures`, 8415 Dateien) hat alle drei — nach `asset/Milkdrop3/textures/` kopiert. Damit sind 27 von 35 Fehler-Log-Zeilen weg. ⚠ Herkunft Community-Pack: Lizenzfrage vor dem Commit kurz bedenken. Bei künftigen fehlenden Texturen zuerst dort suchen (verwandt: „Assets-Ordner-Fallback" §7) | ✅ |
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
| ~~[Hotkey_Konzept §9](ui/Hotkey_Konzept.md)~~ | ✅ **entschieden (Patrik, S60):** §9.2 Stufe 1 blättert NICHT in Unterordner (nur aktuelle Ebene) · §9.3 am Verzeichnisende **umlaufen** · §9.4 Blättern wirkt auf den **zuletzt gewählten** Eintrag · §9.5 Menü-Einträge zeigen die Tasten. *(§9.1 in S52 entschieden: `Bild ab` = vorwärts.)* Umsetzung/Konzept-Nachzug bei der nächsten Hotkey-Arbeit (nach §0) | ✅ |
| ~~[Visual_Playlist §6](ui/Visual_Playlist_Konzept.md)~~ | ✅ **entschieden (Patrik, S60):** **Pfad-Referenzen** statt eingebetteter `.lvfx` (kaputte Pfade beim Laden melden) · **Auslöser-Default: KEIN automatischer Wechsel** (manuell; Songwechsel/Timer sind opt-in) · Timer-Wechsel, wenn aktiviert, **beat-quantisiert** (nächster Beat) · Import-Browser-Erweiterung **erst mit der Playlist**. P2 damit entsperrt, Umsetzung nach §0 | ✅ |
| ~~[Lights_Module_Entwurf](visuals/Lights_Module_Entwurf.md)~~ | ✅ **entschieden (Patrik, S60): nach §7 (Backlog) verschoben** — BASS-Lookahead-Service (`AudioLookahead`) erst mit den Lights-Modulen; bis dahin reichen Beat-Prädiktion + `gettime()` | ✅ |
| [Parameter_Reference §10](visuals/Parameter_Reference.md) | Deklarierte Preset-Defaults vs. Dropdown-Indizes bereinigen · `solidColor`/`peak.color.fixed` ohne deklarierten Default | 🔧 |
| ~~MilkDrop-Texturen~~ | ✅ beschafft aus dem MegaPack (S60), siehe §3 | ✅ |
| ~~**Multi Delay: wem gehört die Verzögerung?**~~ (Befund S55) | ✅ **entschieden und umgesetzt (Patrik S55): Puffer-Besitz, original-treu.** Der Ausgabe-Knoten liest jetzt den **ältesten** Frame des Rings (`head`), sein eigenes `delay` wirkt nicht mehr — wie `outpos[buffer]` im Original. Bis dahin änderte das `delay` des Schreibers nur die Ringgröße und blieb unsichtbar. Vorstand gemessen: ungleiche Werte kann es im Original **gar nicht geben** (jeder Knoten speichert alle sechs Puffer-Einstellungen und schreibt sie global, `r_multidelay.cpp:387-401`), im Referenz-Korpus nutzen **2** Presets den Effekt, in eigenen `.lvfx` nur die Sonden | ✅ |
| ~~**Colorfade: `enabled`-Bitfeld fehlt**~~ (Befund S55) | ✅ **erledigt S57, Zeile war nur nie abgehakt (bemerkt S60):** Colorfade ist nach `r_colorfade` portiert — Bitfeld beim Import, persistenter Fader-Zustand, Annäherungsrampe, drei exklusive Beat-Zweige; alle 12 Felder wirken (§1-Zeile „Colorfade-Zufalls-Beatmodus") | ✅ |
| ~~[Config_Pipeline_Umsetzungsplan](visuals/Config_Pipeline_Umsetzungsplan.md)~~ | ✅ **entschieden + umgesetzt (S60):** die Abnahme-Tabellen A1–A8/N1–N7 sind **als erledigt gestrichen** (Vermerk im Dokument; die `⬜` bleiben als Historie stehen) — Schritte 0–7 ✅ seit S30, Sichttests 5.1–5.5 abgenommen | ✅ |

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

1. 🔴 **Test-Presets für JEDES Modul und JEDES Feld** (Strang E, Konzept §9). Ein
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

- 🔧 **Die eingefrorenen `.lvfx`-Zwillinge stehen auf 27/67** (vorgefunden S58, nicht
  von dort verursacht). `freeze_lvfx_twins.py --verify` meldet durchweg dieselbe
  Abweichung: die Strang-D-Felder `initCode`/`frameCode`/`beatCode` sind seit dem
  Parameter-Skript-Ausbau in jedem Knoten dazugekommen, die Zwillinge aber nie
  nachgezogen worden. Der Prüfstand ist damit blind. Zu tun: einmal durchsehen,
  ob wirklich nur diese Felder dazukamen, dann `--refreeze`. **Kein Zwilling
  enthält Picture II** — der Parser-Fix aus S58 ist nicht beteiligt.
- 🔧 **Der SuperScope-Randspalten-Fix hat keinen Wächter** (S58). Eine einzelne
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
    und Rundung im Treiber, also möglicherweise die Bilder. Matrix (40/43),
    Modul-Sonden (83/87), die eingefrorenen Zwillinge und die Bit-Identität der
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
| **`20_water`** | ✅ **Abgenommen (Patrik, S60):** Struktur deckungsgleich (Deckung 0,96, dMean 0,002), der Diff ist feines Rauschen ausschließlich über den Wellenzonen — Chaos-Verstärkung von GPU-float- vs. x87-Rundung durch die rückgekoppelte Wellengleichung. Randbehandlung seit S57 referenz-treu. Die Matrix-Zeile bleibt als Wächter mit diesen Erwartungswerten stehen |
| **`24_grain`** | ✅ **Abgenommen (Patrik, S60):** Kornmenge und Helligkeit exakt (dMean 0,000, Menge 0,00, dreimal identisch), nur die Einzelkörner sitzen anders (Deckung 0,56): die Referenz zieht ihren Zufall sequentiell und DATENABHÄNGIG durch die 491-Byte-Tabelle — parallel je Pixel prinzipiell nicht nachbildbar (S57). Zeile bleibt als Wächter stehen |
| AVS-Builtins | SVP Loader (10) = externe UVS/SVP-DLL, nicht decodierbar |
| AVS-APEs | 5 verworfen: GeissFluid · ParticleSystem · MIDI Trace · AVI Player · AVSTrans Automation (closed-source bzw. Meta) · Framerate Limiter = no-op (der Host taktet) |
| HLSL-Transpiler | `#elif` und Nicht-Literal-`#if` → sauberer Fehler, MD1-Fallback wie im Original |
| MilkDrop-Referenz | GPU-Rendering ist nicht bit-deterministisch — Vergleich über Statistik/Montagen, nie Pixelgleichheit |

## 10. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.24.0 | 2026-08-01 | Session 61 (dritter Block) — **`asset/examples/`: je Vorlage ein Beispiel-Preset (125, flach, Name = `<typkey> - <Vorlagenname>`)** mit Matrix-Material als Render-Quellen; Generator `asset/calibration/avs/make_example_presets.py` (Rückabbildung des kalibrierten Imports). **90 als .avs** (AvsRef-vergleichbar) · 35 als .lvfx (host-eigene Typen, Host-Konstanten-Vorlagen, LumiViz-EEL-Dialekt — die S53-Figuren Star/Starburst nutzen `mod()`/`>`, das Original-EEL nicht kennt). **Vergleichslauf über alle 90: 85 OK, 5 PRUEFEN, 0 Fehler** — 2× grain (bekannte §9-Grenze, Menge exakt) + 3× Feedback-Weichheit (s. neue §1-Zeile). Dabei zwei Vorlagen-Fixes: `flame/Hufeisen-Nebel` kollabierte mit 4 Abbildungen (jetzt 3) · `mirror/Zufallsspiegel` trug `slower: true` statt einer Zahl. Tests 485/485 |
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
