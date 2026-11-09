# Modell-Entwurf — Becken und Gong (Klasse 6)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Messkette [gemeinsam/messkette.md](../../messung/ic/messkette.md) (B6, B7 Band über der Zeit, B8 Gleiten), Bestand in
> LumiViz (IC-B1 a, wie Klasse 5). Berichte: `recherche/2026-10-26_physik_rechenverfahren.md`,
> `gemeinsam/recherche/2026-10-26_spielweisen_schlagwerk.md`, `…_aufnahmen_schlagwerk.md`.
> **Kennzeichnung:** **nachgelesen** · **nur Bericht** · **Einschätzung** · **offen**. Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand in LumiViz

Wie Klasse 5 ([membran/modell-entwurf.md](../../membran/ic/modell-entwurf.md) §1): Rollen `hat`, `open hat`, `ride`, `crash`
(K5 a), Lautstärke je Schritt, Kit-Klänge bis 30 s; Zielbild „Abdämpfen mit der Hand (Becken …)“ je Note (SF8 a). Heute
sind Becken in LumiViz Rauschen oder Samples (`drum_crash`, DrumGizmo `lib:drs_crash_*`). Becken und Gong stehen nicht
in SF10 a (Erweiterung).

## 2. Kern

1. **Nichtlineare Platte bzw. Schale (von Kármán), vorab je Stärkestufe** (Einschätzung): Rechenzeit ist bei vorab
   gerechneten Schlägen **nicht mehr das Hindernis** — ein energieerhaltendes Schema für die Platte rechnet in C++ eine
   Platte von 0,06 m² in 0,484 s je s Klang (Bilbao, Webb, Wang & Ducceschi 2023, Tab. 1, nachgelesen PDF-S. 7; CC BY
   4.0, nachgelesen S. 1); ein Crash mit 40 cm hat ≈ 0,126 m² (eigene Rechnung im Bericht) → geschätzt 1–2 s je s.
   Zum Vergleich: implizite Finite Differenzen „about two hours of calculation per second of simulation“ (Ducceschi,
   Touzé & Bilbao 2013, nachgelesen PDF-S. 4); Schale „between 1 s and 160 s per second of audio output“ (Bilbao 2010,
   nachgelesen PDF-S. 8).
2. **Form:** Kuppe, Wölbung und Verjüngung bestimmen Ride und Crash; Eigenfrequenzen sind „highly sensitive to small
   changes of curvature“ (Chaigne, Touzé & Thomas 2005, nur Bericht). Die schnellen Schemata rechnen flache Platten —
   für Becken braucht es die Schale (Einschätzung im Bericht).
3. **Gleiten beim Gong:** große flache Operngongs gleiten abwärts „by as much as three semitones“, kleine stark gewölbte
   aufwärts (Chaigne et al. nach Fletcher 1985, nachgelesen PDF-S. 2); Xiaoluo 370 → 450 Hz (Jossic et al. 2017, nur
   Bericht). Bei dünnen Becken kein Gleiten (nur Bericht).
4. **Energiefluss nach oben:** „upward cascade of energy from the low-frequency modes“, Spektrum über „several seconds“
   (Legge & Fletcher 1989, nur Bericht); Tamtam blüht über etwa 1 s auf und klingt bis 10 s (nur Bericht).
5. **Stabilität:** explizites Störmer-Verlet „highly prone to explosive instability“ (DAFx23, nur Bericht) → nur
   energieerhaltende Schemata.
6. **Billige Stufe** (Einschätzung, **nicht als fertiges Verfahren belegt**): gemessene lineare Moden für tiefe Anteile
   plus ein Rauschteppich in Bändern, dessen Energie nach einem phänomenologischen Transport (Humbert et al. 2016, nur
   Bericht) nach oben wandert; Gleiten je Mode als Rückgrat ω = ω₀(1 + CA²).

## 3. Je Instrument

| Instrument | Modell (Vorschlag) | Datenlage | Risiko (Einschätzung) |
|---|---|---|---|
| **Tamtam, Gong** | von-Kármán-Platte, flach (Tamtam) bzw. mit Kuppe (Gong); Gleiten | **Iowa** Tamtam 22/28/40″ reflexionsarm, VSCO Gong 17 Stärken, Beijing-Opera-Gongs (CC BY 4.0) | mittel |
| Crash, China, Splash | von-Kármán-Schale mit Kuppe und Verjüngung | Iowa (Kuppe/Fläche/Rand × 3 Stärken, Choke); B20-Kennwerte **nicht gemessen** (nur Rechenwerte 8860 kg/m³, 110 GPa, nur Bericht) | mittel–hoch (Form, Material) |
| Ride | wie Crash; Kuppe gegen Fläche | Iowa, DrumGizmo (bell, tip, shank) | mittel–hoch |
| **Hi-Hat** | zwei Schalen mit Kontakt | DrumGizmo (closed/semi/open/pedal); das einzige Modell scheitert am Fußdruck: d negativ „because of the numerical divergence“ (Sekiguchi & Samejima 2023, nachgelesen PDF-S. 8) | **hoch** |

## 4. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| Material (E, ρ, ν) | fest | **offen** für B20 (nur Rechenwerte bei Sekiguchi & Samejima) | Modenfrequenzen (M1) |
| Form: Durchmesser, Dicke(nprofil), Kuppe, Wölbung | fest je Instrument | **offen** (Brezas 2023: 8″-Messingbecken, Dicke 1,2–1,4 mm, nur Bericht) | Moden (M1) |
| Dämpfung je Mode | fest | Plattengesetz ξ = 8·10⁻³ ω^−0,4 (Humbert, nur Bericht) | T60 je Band (M2) |
| Schlag: Kraft, Kontaktzeit | Stärke (Klasse 5 F-2) | Modellwerte 100–300 N, 0,8–1 ms (DAFx-15, nur Bericht); **nicht gemessen** | Spektrum über die Stärke, Schwelle zum Crash (M3) |
| Schlagstelle (Kuppe, Fläche, Rand) | Artikulation (Klasse 5 F-3) | Rand heller (nur Bericht) | Iowa `bell`/`normal`/`shoulder` (M4) |
| Gleiten (Gong) | aus der Nichtlinearität | ≈ 300 Cent (nachgelesen, nach Fletcher) | B8 an Beijing-Opera-Gongs (M5) |
| Hi-Hat: Öffnung, Kontakt | Artikulation (F-3) | offen | DrumGizmo closed/semi/open (M6) |
| Choke | Ende der Note (F-4) | **offen** (keine Messung) | Iowa `chokecrash`, CrocellKit `CrashLStopped` (M7) |

## 5. Rechenzeit

Platte nach DAFx23 0,15–0,99 s je s Klang (nachgelesen); Schale bis 160 s je s in MATLAB (nachgelesen); vorab je Stärke
und Schlagstelle — bei 5–10 s Klang Sekunden bis Minuten je Stufe (Einschätzung).

## 6. Risiken

- **Determinismus:** die Rechnung ist chaotisch (positive Lyapunov-Exponenten, nur Bericht) — kleinste Unterschiede der
  Rechenreihenfolge ändern den Verlauf (F-2).
- **Daten:** Form und Material echter Becken ungemessen; keine Modendaten.
- **Hi-Hat:** Fußdruck numerisch nicht gelöst.

## 7. Abweichungen vom Rahmen

- Die Vorschätzung „eher Deluxe“ (KLASSEN.md) gilt für Treue und Daten, nicht mehr für die Rechenzeit (F-1).
- Hi-Hat-Öffnung als Parameter statt zweier Rollen (F-3).
