# Kalibrierung — Einstieg für alle Importformate

> **Version:** 1.0.0
> **Datum:** 2026-08-09 (Session 74)
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

## 1. Die fünf Regeln

Jede hat in Session 74 mindestens einen Lauf gekostet und ist an Messwerten
belegt. Sie gelten formatübergreifend.

| # | Regel | Was passiert sonst |
|---|---|---|
| 1 | **Das Messmittel wird abgenommen, bevor damit gemessen wird.** | Ein 96×96-Raster lieferte allein schon MAE 0,165 — der ganze Blend-Lauf darüber war wertlos, sah aber nach dreizehn Befunden aus |
| 2 | **Erst jeden Knoten einzeln, dann Kombinationen.** | Die kumulative Bisektion zeigte bei `10_the ring` auf das Movement; der Täter war der SuperScope eine Stufe davor |
| 3 | **Transform-Knoten brauchen eine Quelle.** | Auf schwarzer Fläche liefern beide Renderer schwarz; `Water` maß so „0,000 OK" statt MAE 0,332 |
| 4 | **Die Quelle gehört in denselben Blend-Kontext wie der Prüf-Knoten.** | Vor die Liste gesetzt räumt die Liste sie weg — beide Seiten schwarz, nichts gemessen |
| 5 | **Eine kleine Zahl ist kein Beweis. Ins Bild sehen.** | Ein 1-Pixel-Unterschied bewegt den Mittelwert nicht: MAE 0,001, während eine Seite schwarz ist und die andere zeichnet |
| 6 | **Bei Schalterreihen die Gruppierung je Seite vergleichen, nicht nur den Messwert je Stufe.** | Die Adjustable-Blende bestand jede Stufe mit MAE 0,002 — erst der Vergleich „wie viele verschiedene Bilder erzeugt jede Seite über die Skala" zeigte, dass wir ab 128 sättigen und die Referenz nicht |
| 7 | **Erst prüfen, ob der Effekt auf der Gegenseite überhaupt läuft — dann den Port zerlegen.** | `Water` wurde Zeile für Zeile gegen das Original gelegt und war korrekt. Im Preset stand `enabled = 0`: die Referenz übersprang den Effekt, wir führten ihn aus. MAE 0,332 für einen fehlerfreien Port |

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
| **AVS** | `AvsStandalone` | `tools/AvsRef` (originaler vis_avs-Kern) | `asset/calibration/avs/compare_avsref.py` |
| **MilkDrop** | `MilkdropStandalone` | `tools/MilkdropRef` (originaler MilkDrop3-Kern, D3D9) | `asset/calibration/milkdrop/compare_ref.py` — **nur Triage, ein Prozess je Preset fehlt** (Aufgabe 1 des Kalibrier-Plans) |
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
| **MilkDrop** | ⚠️ eingeschränkt | Referenz vorhanden, aber **ein Prozess je Preset fehlt** — Presets erben das Bild des Vorgängers, alle Stapel-Zahlen sind nur Richtwerte |
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
