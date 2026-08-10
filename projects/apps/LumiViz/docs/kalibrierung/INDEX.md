# Kalibrierung — Einstieg für alle Importformate

> **Version:** 1.2.0
> **Datum:** 2026-08-10 (Session 75; 1.2.0: Regeln 9–12, MilkDrop-Harness §2.2)
> **Typ:** Guide / Einstieg
> **Status:** Aktiv
> **Sprache:** Deutsch
> **Zweck:** Eine Anlaufstelle für die Frage „stimmt unser Bild mit dem
> Original überein, und wie weise ich das nach" — formatübergreifend.

Bisher lag das verstreut: AVS-Methodik in `visuals/`, MilkDrop-Befunde in den
Offenen Punkten, die Werkzeug-Fallen in der Wegleitung, die Audio-Signale
nirgends. Dieses Verzeichnis bündelt es. **SSOT-Regel:** was anderswo schon
steht, wird hier verlinkt und nicht abgeschrieben.

## Inhalt

- [Messmittel](Messmittel.md) — die Kalibrier-Raster und wie sie abgenommen werden
- [AVS-Kalibrier-Methodik](../visuals/AVS_Kalibrier_Methodik.md) — Sondenformen,
  Bisektion, Verifikationsgürtel (SSOT für AVS)
- [AVS-Sichttest-Protokoll](../visuals/AVS_Sichttest_Protokoll.md)
- [Kalibrier-Plan mitgelieferte Presets](../visuals/Kalibrier_Plan_Mitgelieferte_Presets.md)
  — die laufende Runde über die 29 Schaufenster-Presets
- [Werkzeug-Wegleitung](../Werkzeug_Wegleitung.md) — welches Programm wofür,
  und die sieben Fallen
- [Offene Punkte §1](../Offene_Punkte.md) — SSOT der Befunde mit Messwert

---

## 1. Die zehn Regeln

Jede hat in Session 74 bzw. 75 mindestens einen Lauf gekostet und ist an
Messwerten belegt. Sie gelten formatübergreifend.

| # | Regel | Was passiert sonst |
|---|---|---|
| 1 | **Das Messmittel wird abgenommen, bevor damit gemessen wird.** | Ein 96×96-Raster lieferte allein schon MAE 0,165 — der ganze Blend-Lauf darüber war wertlos, sah aber nach dreizehn Befunden aus |
| 2 | **Erst jeden Knoten einzeln, dann Kombinationen.** | Die kumulative Bisektion zeigte bei `10_the ring` auf das Movement; der Täter war der SuperScope eine Stufe davor |
| 3 | **Transform-Knoten brauchen eine Quelle.** | Auf schwarzer Fläche liefern beide Renderer schwarz; `Water` maß so „0,000 OK" statt MAE 0,332 |
| 4 | **Die Quelle gehört in denselben Blend-Kontext wie der Prüf-Knoten.** | Vor die Liste gesetzt räumt die Liste sie weg — beide Seiten schwarz, nichts gemessen |
| 5 | **Eine kleine Zahl ist kein Beweis. Ins Bild sehen.** | Ein 1-Pixel-Unterschied bewegt den Mittelwert nicht: MAE 0,001, während eine Seite schwarz ist und die andere zeichnet |
| 6 | **Bei Schalterreihen die Gruppierung je Seite vergleichen, nicht nur den Messwert je Stufe.** | Die Adjustable-Blende bestand jede Stufe mit MAE 0,002 — erst der Vergleich „wie viele verschiedene Bilder erzeugt jede Seite über die Skala" zeigte, dass wir ab 128 sättigen und die Referenz nicht |
| 7 | **Erst prüfen, ob der Effekt auf der Gegenseite überhaupt läuft — dann den Port zerlegen.** | `Water` wurde Zeile für Zeile gegen das Original gelegt und war korrekt. Im Preset stand `enabled = 0`: die Referenz übersprang den Effekt, wir führten ihn aus. MAE 0,332 für einen fehlerfreien Port |
| 8 | **Die Referenz ist selbst ein Messgerät. Wo sie schweigt, ist nichts bewiesen.** | `AvsRef` rendert eine leere Fläche, sobald ein Skript `atan` oder `log` benutzt — ein Linker-Schaden im JIT dieses Builds. Der Befund „wir zeichnen, die Referenz nicht" war deren Fehler, nicht unserer: `10_the ring` fiel nach dem Fix von MAE 0,113 auf 0,001. Abnahme: `AVSREF_EELTEST=1` (§2.1) |
| 9 | **Beim Original-Quelltext zählt nur der Zweig, der WIRKLICH kompiliert wird.** | Der `#ifdef NO_MMX`-Zweig von `BLEND4` beschreibt eine ganz andere Arithmetik (`g_blendtable`, `/255`, acht Trunkierungen) als der MMX-Zweig, den AVS ausführt (separabel, `>>8`, drei). S74 hat den falschen gelesen und daraus eine Aufgabe „`g_blendtable` bit-treu nachbilden" abgeleitet — sie hätte den korrekten Port verschlechtert. Der tote Zweig **lässt sich nicht einmal übersetzen** (`inblendval` undeklariert). Prüfrezept: das Symbol im ganzen Baum suchen (`NO_MMX` wird nirgends definiert) und im Zweifel einen Build mit dem Schalter erzwingen |
| 10 | **Ein Verstärker im Pfad macht einen fremden Fehler zum eigenen Befund.** | Bei `07_movin wall` ist jeder Einzelknoten grün (≤0,012) und das Ganze bei 0,073. Der Movement dehnt das Bild stark: er hebt den 1-Pixel-Kantenversatz eines SuperScope von 0,009 auf 0,109 — über den Wert des Gesamt-Presets. Die kumulative Bisektion zeigt dann auf den Verstärker, nicht auf die Quelle. Gegenmittel: **Knoten paarweise messen** (Verdächtiger + Verstärker), nicht nur solo und kumulativ |
| 11 | **Beide Seiten müssen im selben STARTZUSTAND beginnen.** | Bei MilkDrop streuen wir beim Kaltstart Rauschen in den Feedback-Puffer (`seedFeedbackNoise`, S63 — Verstärker-Presets sterben sonst), die Referenz startet mit genulltem VRAM. In den ersten Frames zeichnen wir also, während sie noch schwarz ist. Das erzeugt einen scheinbaren Befund pro Preset. Gemessen wird deshalb über eine **Reihe von Startwerten** auf beiden Seiten (`compare_milkref.py --saaten`, §2.2). ⚠️ **Falle:** `MilkdropStandalone` setzt `LUMIVIZ_MILKDROP_NOSEED` **selbst**, solange `--seed` fehlt (sein Prüfstands-Vertrag seit S64) — eine Env-Variable allein bleibt wirkungslos. In S75 waren dadurch drei „verschiedene" Seeds bit-identisch, und eine erste Messung meldete fälschlich „die Saat ändert nichts (≤0,006)". Erst `--seed` **plus** `LUMIVIZ_MILKDROP_SEED` wirkt; abgenommen an `Magma`: ohne/mit Saat 0,274, zwei Seeds untereinander 0,044 |
| 12 | **Die Warmlaufphase der Referenz ist keine Messung.** | Der MilkDrop-Kern braucht Frames, bis überhaupt etwas im Bild steht: `Starfield` ist bei Frame 10 noch schwarz und liefert ab Frame 30 MAE 0,005. Wer die frühe Marke als Befund wertet, bekommt eine Liste von Scheinfunden (S75, erster Durchgang: 0 von 19 OK). `compare_milkref.py` verwirft eine Marke, in der die Referenz nicht zeichnet — **aber nur**, wenn eine spätere zeigt, dass sie danach zeichnet. Bleibt sie durchgehend stumm, ist der Unterschied echt |

> Regel 1, 5 und 6 sind dieselbe Regel aus drei Richtungen: **die Metrik lügt
> bei dünnen Inhalten.** Das Urteil fällt über Bild + Metrik + Verhalten über
> eine Schalterreihe, nie über die Metrik allein.

**Regel 6 als Rezept** — für jede Stufe das Bild der eigenen Seite hashen und
zählen, wie viele verschiedene dabei herauskommen; dasselbe für die Referenz.
Weichen die Zahlen ab, verhält sich ein Schalter anders, auch wenn jede
einzelne Stufe im Messwert besteht:

```bash
Get-ChildItem "<out>\lumi\*.png" | ForEach-Object { (Get-FileHash $_).Hash.Substring(0,8) } | Select-Object -Unique
```

**Gegenprobe nicht vergessen:** kollabieren BEIDE Seiten auf ein Bild, regt die
Probe den Schalter gar nicht an — das ist kein Bestehen, sondern ein
untauglicher Prüfstand.

**Regel 7 als Rezept** — je Frame speichern und den Verlauf der Mittelwerte
nebeneinanderlegen:

```bash
AvsStandalone "<probe.avs>" --auto --frames 12 --save-every 1 --size 320x240 --beat-period 30 --out "<lumi>"
AvsRef "<probe.avs>" --frames 12 --save-every 1 --size 320x240 --beat-period 30 --out "<ref>"
```

Steht der Mittelwert einer Seite über alle Frames **konstant**, rechnet dort
nichts — der Effekt ist abgeschaltet, nicht falsch. Weicht schon Frame 1 ab,
ist es kein Aufschaukeln über die Zeit, sondern ein Unterschied in der ersten
Rechnung. Beides zeigt ein Einzelbild bei Frame 120 nicht.

> `QT_ENABLE_HIGHDPI_SCALING=0` setzen, sonst rendert der Standalone bei
> Windows-Skalierung größer als die Referenz und die Bilder lassen sich nicht
> vergleichen. `compare_avsref.py` tut das selbst; von Hand nicht vergessen.

---

## 2. Werkzeuge je Format

| Format | Unsere Seite | Referenz (Original) | Vergleichs-Harness |
|---|---|---|---|
| **AVS** | `AvsStandalone` | `tools/AvsRef` (originaler vis_avs-Kern) — vor Messreihen abnehmen, s. §2.1 | `asset/calibration/avs/compare_avsref.py` |
| **MilkDrop** | `MilkdropStandalone` | `tools/MilkdropRef` (originaler MilkDrop3-Kern, D3D9) | `asset/calibration/milkdrop/compare_milkref.py` (S75, Aufgabe 1 erledigt) — ein Prozess je Preset, Marken 10/30/120. `compare_ref.py` bleibt daneben: es wertet Screenshots eines Triage-Laufs aus und rendert selbst nichts |
| **Shadertoy** | `AvsStandalone` (lädt `.lvfx`) | — **keine** | — |
| **ISF** | `AvsStandalone` (lädt `.lvfx`) | — **keine** | — |
| **Video/Kamera** | `AvsStandalone --kamera-freigeben` | — entfällt | — |

**Bau vor jedem Lauf** (Merkregel: Debug ist Faktor 20 langsamer und sieht aus
wie ein Hänger):

```bash
cmake --build --preset build-ninja-release-clang --target AvsStandalone MilkdropStandalone
```

Die Referenz-Werkzeuge sind eigenständige 32-bit-MSVC-Projekte außerhalb der
Solution:

```bash
cmake --build tools/AvsRef/build --config Release
```

### §2.1 Das Referenz-Werkzeug selbst abnehmen

Ein Referenz-Renderer, den jemand neu baut, ist ein **Messgerät** — und
Messgeräte werden abgenommen (Regel 1). `AvsRef` hat einen eingebauten
Selbsttest:

```bash
AVSREF_EELTEST=1 AvsRef "<beliebiges preset.avs>" --frames 1 --size 64x64 --out "<tmp>"
```

Er gibt die **Fragmentgrößen der ns-eel-JIT-Tabelle** aus und rechnet 19
EEL-Funktionen gegen die C-Bibliothek nach. Erwartet: `0 von 19 fehlerhaft`
und keine als `VERDAECHTIG` markierte Größe.

**Warum das nötig ist** (Befund S74): ns-eel kopiert Maschinencode zwischen
`nseel_asm_<fn>` und `nseel_asm_<fn>_end` und bestimmt die Länge als
**Adressdifferenz zweier nackter Funktionen**. Der Vertrag setzt damit
Quellreihenfolge im Speicher voraus — eine Annahme, die moderne
Compiler-Vorgaben stillschweigend brechen. Mit `/Gy` (Release-Vorgabe) bekam
jede Funktion ihre eigene COMDAT, der Linker sortierte die leeren
`_end`-Marken von `atan` und `log` hinter die jeweils folgende Funktion, und
deren Fragmente wurden 128 statt 48 bzw. 80 statt 32 Bytes lang. Die
Ausführung lief in Fremdcode und `0xCC`-Füllbytes (`int 3`); im Preset fing
die SEH-Absicherung das ab und der Effekt rendert **schwarz, ohne jede
Meldung**.

Behoben mit `/Gy-` für `patched/nseel-cfunc.c`. Das hat allein `10_the ring`
von MAE 0,113 auf 0,001 gebracht — ein Befund, der nie einer war.

> **`invsqrt` ist absichtlich ungenau** (schneller Bit-Trick + eine
> Newton-Iteration, 0,49915 statt 0,5). Im Selbsttest mit eigener Toleranz
> geführt.

### §2.2 Der MilkDrop-Lauf

```bash
python asset/calibration/milkdrop/compare_milkref.py --frames 10,30,120 --size 320x240 --out <ziel>
```

Was das Werkzeug anders macht als ein Stapellauf — und warum es nötig war:

- **Ein Prozess je Preset und Marke.** `--auto <ordner>` rendert alles in einem
  Prozess, und MilkDrop-Presets erben das Bild des Vorgängers (Original-
  Verhalten). `Helix` zeigte so ein Herz aus dem davor laufenden
  `Dancing Hearts` (S73). Alle Stapel-Zahlen davor sind Richtwerte, keine
  Befunde.
- **Mehrere Frame-Marken.** MilkDrop ist ein Rückkopplungssystem; ein
  Einzelbild bei Frame 120 vermischt echten Fehler und Phasenversatz.
- **Die Referenzwurzel baut es selbst auf** (`data/` + `presets/`, §2.5 der
  Werkzeug-Wegleitung) — abgenommen mit `Blank.milk`, das dort dasselbe Bild
  liefert wie im eingebauten Ordner.
- **Saatlos als Vorgabe** (Regel 11), **Anlaufphase verworfen** (Regel 12).

Drei Fallen, die dabei Läufe gekostet haben:

1. 🔴 **Der Kern rendert seinen Preset-Browser ins Bild.** Startet MilkdropRef
   im UI-Modus, liegt ein Verzeichnis-Overlay über dem Frame — und der
   Vergleich liefert eine Zahl, die wie ein Messwert aussieht. `m_UI_mode`
   wird deshalb vor jedem `LoadPreset` auf `UI_REGULAR` gesetzt; bleibt der
   Kern danach im UI-Modus, meldet er `PRESET-NICHT-GELADEN` und das Urteil
   lautet `REF-STUMM` statt einer Zahl.
2. 🔴 **Urteile an einer Schwellenkante sind keine Urteile.** Die erste Fassung
   stufte „tot" bei `lumaMax < 0,05` ein — damit stand `Starfield` mit MAE
   0,002 als Befund in der Liste, weil eine Seite knapp unter und die andere
   knapp über der Schwelle lag. Ein „eine Seite zeichnet nicht"-Befund
   verlangt jetzt **zwei** Bedingungen mit Abstand: eine Seite praktisch
   schwarz (< 0,02) UND die andere deutlich hell (> 0,20).
3. 🟠 **`BEIDE-STUMM` ist kein Bestehen.** Zeigt keine Seite etwas, prüft die
   Probe nichts — das Werkzeug weist es als eigenes Urteil aus, nicht als OK.
4. 🟠 **Die hohen f10-Werte im Saat-Modus sind ECHT, kein Versatz.** Erste
   Vermutung war ein Ein-Bild-Versatz (wir säten vor Frame 0, die Referenz
   danach). Unsere Seite ist inzwischen angeglichen — sie sät ebenfalls erst
   nach dem ersten Bild (`m_saatOffen`, folgt dem Original-Clear) —, und der
   Wert bewegte sich kaum (`Starfield` f10: 0,984 → 0,960). Ins Bild gesehen:
   die Saat **sättigt unsere Seite weiß**, während die Referenz beim selben
   Preset schwarz bleibt. Gespiegelt ist nichts (vertikal wie horizontal
   gegengerechnet: identische Werte). Der Unterschied ist real.

**Stand des ersten vollständigen Laufs** (19 Presets, Marken 10/30/120,
Startwerte `0x0` + `0x5EED63`): **0 OK · 13 BEFUND · 6 STARTABHÄNGIG.** Die
sechs startabhängigen — `Magma`, `Magma 2`, `Playaround`, `Starfield`,
`Rock The House_newmove` — sind damit erstmals *gemessen* statt vermutet: ihr
Bild reagiert stärker auf den Startwert als auf den Renderer, ein
Treue-Urteil ist dort grundsätzlich nicht zu holen. Das ist die S67-Dunkelklasse
mit einer Zahl dahinter.

### §2.3 🔴 `MilkdropRef` ist noch nicht abgenommen (S75)

**Bevor mit diesen Zahlen gearbeitet wird, muss das hier geklärt sein.** Zwei
Eigenheiten der Referenz, beide im Bild belegt:

1. **Sporadisches Browser-Overlay.** Der Kern zeichnet gelegentlich seinen
   Preset-Browser (roter Balken, „Directory of …") über den Frame. Ein Reset
   von `m_UI_mode` auf `UI_REGULAR` vor `LoadPreset` **und** je Frame beseitigt
   es *nicht* zuverlässig: der je-Frame-Zähler meldete null Kipps, während das
   Overlay im Bild stand. `m_UI_mode` ist also nicht die steuernde Größe —
   die echte Quelle ist noch zu finden.
   **Woran man es merkt:** ein Preset springt zwischen Läufen von „schwarz" auf
   einen mittleren Messwert. Beispiel `Starfield (Supernova-Edit)`: mit Overlay
   MAE 0,031 („OK"), ohne Overlay ist die Referenz **komplett schwarz**. Das
   Overlay macht aus einem Befund ein Schein-OK — die gefährlichere Richtung.
2. **Die Referenz zeigt bei mehreren Presets nichts — und das ist kein neuer
   Fehler, sondern war bisher nur unsichtbar.** Bei `Helix`, `The Beauty and
   the Math` und der `Starfield`-Familie bleibt sie kalt gestartet schwarz,
   auch mit `--audio-muster musik`.

   **Beweis, dass es am Kaltstart liegt und nicht am Werkzeug** (S75): einen
   Ordner mit `A_Blank.milk` + `B_Starfield.milk` im Stapel gerendert. Beide
   melden **denselben** Mittelwert `(0.098, 0.041, 0.147)` — das Bild von
   `B_Starfield` ist zu 100 % das geerbte von `A_Blank`. Im Stapellauf zeigt
   also jedes Preset ein Bild, aber teils das des Vorgängers; deshalb wirkte
   `MilkdropRef` bis S74 unauffällig. Ein Prozess je Preset nimmt dieses Erbe
   weg — und legt offen, dass die Referenz ohne Startenergie nicht zündet
   (dieselbe Familie wie `piercing 01`, S67: der Look hängt an nicht-genulltem
   VRAM).

   **Entscheid (Patrik, S75): über eine REIHE von Startwerten messen, nicht
   über einen.** Beide Grundzustände gehören dazu — saatlos misst den reinen
   Renderpfad, mit Saat den Zustand, den die App zeigt; erst zusammen ist die
   Kalibrierung vollständig. Und mehrere Rausch-Seeds beantworten die Frage,
   die ein einzelner nicht kann: **hängt das Preset am Startzustand?** Streut
   sein Bild über die Seeds stärker als der Unterschied zwischen den
   Renderern, ist es grundsätzlich nicht pixelvergleichbar — das ist die
   Dunkelklasse aus S67, dann aber gemessen statt vermutet.

   **Dabei aufgedeckt — der Kaltstart des Originals ist SCHWARZ.** Die
   S63-Begründung „das Original startet mit undefiniertem VRAM" ist falsch:
   `milkdropfs.cpp` nullt den Feedback-Puffer beim ersten Frame ausdrücklich
   (`// on first frame, clear OLD VS` bei `m_nFramesSinceResize == 0`). Presets
   zünden im Original beim **Preset-Wechsel**, weil der Puffer dann stehen
   bleibt — nicht aus Speichermüll. Unsere Rausch-Saat ist damit eine bewusste
   **Abweichung**, kein Nachbau; sie ersetzt beim Kaltstart das, was im
   Original das Vorgängerbild liefert. Genau deshalb muss `MilkdropRef` die
   Saat *nach* dem ersten Frame gesetzt bekommen — davor löscht der Kern sie.

   **Und: der Startzustand erklärt die stummen Presets NICHT.** Mit
   eingespeister Saat (an `Blank.milk` abgenommen: Mittelwert 0,028 → 0,970,
   die Einspeisung wirkt also) bleibt `Starfield` in der Referenz weiterhin
   komplett schwarz. Der Unterschied zu uns ist damit echt und kein Artefakt
   der Vergleichsgrundlage.

   Die Formel liegt dafür seit S75 in einer gemeinsamen Quelle,
   `projects/apps/LumiViz/include/visualizers/KaltstartSaat.hpp`
   (`lumi::saat::basis(w, h, seed)` + `reihe()`), Vorbild `SynthAudio.hpp`.
   Unsere Seite wählt über `LUMIVIZ_MILKDROP_SEED` (Bestandsschalter
   `LUMIVIZ_MILKDROP_NOSEED` = Seed aus); die App-Vorgabe `0x5EED63` bleibt
   unverändert (bit-identisch gegengeprüft).
   **Offen:** `MilkdropRef` muss die Saat noch in `m_lpVS` bekommen
   (lockbare Offscreen-Surface + `StretchRect`), und `compare_milkref.py` muss
   über die Reihe schleifen.

Bis dahin gilt für MilkDrop: Urteile nur an der **Montage**, und nur dort, wo
beide Seiten sichtbar zeichnen.

### Was für Shadertoy und ISF fehlt

Für beide gibt es **keinen Referenz-Renderer** — das Original ist eine Website
bzw. die ISF-Werkzeugkette. Damit ist der Vergleich „Pixel gegen Pixel" nicht
zu haben, und die Kalibrierung muss anders ansetzen:

- **Vorhanden:** GL-Smoke-Test über den Korpus (kompilieren UND linken in
  einem echten 3.3-Core-Kontext) — Stand S72 **321 von 327** (98 %),
  Wächter-Untergrenze 95 %.
- **Fehlt:** ein Treue-Urteil. Denkbare Wege, keiner davon entschieden:
  Referenzbilder von der Quelle einmalig einholen und einfrieren · ein
  ISF-Fremdwerkzeug als zweite Meinung · Selbstkonsistenz (dasselbe Preset
  über GL-Versionen/GPUs).
- **Die Raster aus [Messmittel](Messmittel.md) sind hier trotzdem nutzbar** —
  nicht gegen ein Original, aber als Regressionsanker gegen uns selbst.

---

## 3. Audio-Testsignale

Der Klang entscheidet mit über das Bild. Für Vergleiche muss er auf **beiden**
Seiten identisch sein — deshalb erzeugen alle vier Werkzeuge ihn aus derselben
Quelle, `projects/exec/common/SynthAudio.hpp` (S74; vorher stand die Formel
viermal im Baum).

| Signal | Schalter | Determin. | Referenz erzeugt es auch | Wofür |
|---|---|---|---|---|
| **klassisch** (Vorgabe) | — | ✅ | ✅ | Alle Bestandsmessungen. 220-Hz-Sinus + Beat-Puls; **bit-identisch seit S41/S43** |
| **musik** | `--audio-muster musik` | ✅ | ✅ | Kräftigeres Signal mit echter Musikdynamik (Bandhüllkurven + Beat-Spur aus einer Aufnahme, neu synthetisiert) |
| Stille | `--silence` | ✅ | ✅ | Hunger-Test: was zeichnet ein Preset ohne Ton |
| Beat-Takt | `--beat-hz N` | ✅ | ✅ | Dichtere Schläge für beat-getriebene Presets |
| Klangfarbe | `--klangfarbe` | ✅ | ✅ | Wandernde Spektralbalance (nur MilkdropStandalone) |
| Beat-Korrelation | `--audio-beat` | ✅ | ✅ | Alle Bänder pulsieren gemeinsam — Beat-Detektor-Presets |
| **echte Musik** | `--audio-datei X.mp3` | ✅ | ❌ | **Nur Schaufenster und Augenschein** |

> ⚠️ **`--audio-datei` macht jeden Referenzvergleich wertlos.** Die
> Referenz-Renderer erzeugen ihr Audio selbst; hört nur unsere Seite echte
> Musik, vergleicht man zwei verschiedene Eingaben. `compare_avsref.py`
> verweigert den Dienst, wenn der Schalter durchgereicht wird. Wer für
> Vergleiche mehr Dynamik braucht, nimmt `--audio-muster musik`.

**Beat konstant halten.** Beide Seiten erkennen sonst selbst, und beide
Detektoren hängen an der Wanduhr — gemessen über vier Läufe streute das bis
0,21 MAE. `--beat-period 30` erzwingt denselben Takt auf beiden Seiten und
schlägt auch die Beat-Spur des Musik-Musters. Für einen A/B zwischen den
Mustern ist das Pflicht, sonst misst man die Beat-Lage mit.

### Musik-Profil neu erzeugen

Das Muster `musik` speist sich aus einer eingebauten Tabelle
(`projects/exec/common/MusikProfil.hpp`, erzeugt — acht Bandhüllkurven je Bild
plus Beat-Spur, **keine Aufnahme**, nicht zurückrechenbar):

```bash
AvsStandalone --audio-datei "<musik.mp3>" --audio-start 45 --audio-profil-dauer 20 --audio-profil-schreiben projects/exec/common/MusikProfil.hpp
```

Danach **alle vier Werkzeuge neu bauen** — die Tabelle ist einkompiliert.

### Echte Audiodateien

- Deterministische Testsignale: `…/Visuals_Project/cmake/TestAudio` — 15
  Signale als WAV (Master) und MP3, README liegt dort. Für die App gedacht,
  nicht für Referenzvergleiche.
- Für Schaufenster-Läufe der Standalones: beliebige Datei über
  `--audio-datei`, dazu `--audio-start` und `--audio-gain`.

---

## 4. Stand je Format

| Format | Messbar | Stand |
|---|---|---|
| **AVS** | ✅ vollständig | Methodik, Harness, Bisektion, Raster, Listen-Prüfstand. Laufende Runde über die 10 Schaufenster-Presets |
| **MilkDrop** | ⚠️ Harness ja, Referenz noch nicht abgenommen | `compare_milkref.py` (§2.2) löst das Stapel-Problem. Aber **`MilkdropRef` liefert noch keine verlässlichen Bilder** — sporadisches Browser-Overlay + Erlöschen (§2.3). Die Zahlen des ersten Laufs (3 OK / 12 BEFUND / 4 PRUEFEN) sind deshalb **kein Treuestand** |
| **Shadertoy** | ❌ | nur GL-Smoke, kein Treue-Urteil |
| **ISF** | ❌ | nur GL-Smoke, kein Treue-Urteil |

---

## 5. Ein Lauf von Hand

```bash
python asset/calibration/avs/compare_avsref.py "<preset.avs>" --frames 120 --size 320x240 --beat-period 30 --out "<zielordner>"
```

Ergebnis: Konsolen-Tabelle, `report.md` und je Preset eine Montage
**Referenz | LumiViz | Differenz**. Das Urteil fällt an der Montage; die
Zahlen ordnen ein.

**In zwei Größen messen** (Merkregel S47): 320×240 versteckt größenabhängige
Fehler. Nach jedem Fix zusätzlich bei 1024×768 gegenprüfen.
