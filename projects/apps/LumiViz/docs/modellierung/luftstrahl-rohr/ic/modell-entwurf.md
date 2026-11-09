# Modell-Entwurf — Luftstrahl und Rohr (Klasse 9)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf. Rahmen: Rohr-Kern [gemeinsam/rohr.md](../../rohr/ic/rohr.md),
> Fragen F-R1 bis F-R5 in [gemeinsam/offene-fragen.md](../../gemeinsam/ic/offene-fragen.md), Bestand in LumiViz (IC-B1 a).
> Bericht: `recherche/2026-10-26_physik_spielweisen.md`. **Kennzeichnung:** **nachgelesen** · **nur Bericht** ·
> **Einschätzung** · **offen**. Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand

Nachgelesen, `Spielweisen_Konzept.md`: SF10 a — „das Rohr dient drei Anregungen“, Pfeifenorgel „Luftstrahl oder
Rohrblatt je Register“; SF6 a — Tabelle der Spuren je Bauart: Zeilen für Lippe und Rohrblatt, **keine Zeile für
Luftstrahl**; SK4 a — Expression als Stärke der Anregung (Blasdruck) samt Klangfarbe; Modell 28a — Vibrato „in Cent
0 … 100“.

## 2. Kern

1. **Rohr vorab je Griff** (Rohr-Kern): Bohrung mit Tonlöchern bzw. Pfeife → Moden am Eingang. Die maßgeblichen
   Zeitbereichs-Arbeiten nutzen selbst einen modalen Resonator, die Blockflöte mit drei Moden (ω1 = 3547 rad/s,
   ω2 = 2,023 ω1, ω3 = 3,066 ω1; Auvray et al. 2015, Tab. 1, nur Bericht) — der Kern trägt (Einschätzung).
2. **Strahl je Note im Zeitbereich (jet-drive):** Störung am Kanalausgang (Empfänglichkeit als Querauslenkung),
   verzögert um τ = W/c_p mit c_p ≈ 0,4 U_j, verstärkt entlang des Strahls (α_i ≈ 0,3–0,4/h), am Labium gesättigt
   (Terrien et al. 2013/2014, nur Bericht). Sättigung nach Fletcher & Douglas: „U(t) = V0 b{1 + tanh[(y0 + a sin ωt)/b]}“
   (1980, nachgelesen S. 768); Strahlprofil b = 2h/5 (nur Bericht); Verluste nichtlinear mit α_vc ≈ 0,6 (nur Bericht).
3. **Versatz y0 steuert die Klangfarbe:** „even harmonics are all identically zero when the lip is symmetrically
   placed“ (Fletcher & Douglas 1980, nachgelesen S. 768) — Intonateur (Orgel) und Spieler (Querflöte) setzen damit vor
   allem die zweite Harmonische.
4. **Tonhöhe und Register über dem Druck:** mehr Druck verkürzt die Laufzeit, die Frequenz steigt leicht (Fletcher &
   Douglas S. 767, nachgelesen); Schwellen mit Hysterese — Querflöte G4 statisch 311,5 Pa, dynamische Schwelle wächst mit
   der Anstiegsrate „to the power 2/3“ (Terrien et al., nur Bericht); Blockflöte 1.→2. Register steigend 695 Pa, fallend
   534 Pa (Auvray 2015, nur Bericht).
5. **Einschwingen:** Mundtöne und Vorläufer, gesteuert wie Schneidentöne über Strahlgeschwindigkeit und Abstand
   Spalt–Labium (Castellengo 1999, nur Bericht); mittlere Einschwingzeit „25 to 30 periods for flutes, 50 periods for
   the diapason family“ (Keeler, bei Castellengo 1999, nachgelesen S. 387). Der modale Kern deckt Mundtöne **nicht** ab
   — sie liegen am Schneidenton und an Quermoden (Einschätzung des Berichts; F-4).
6. **Anblasgeräusch:** Dipolquelle, skaliert mit U_j² (Hirschberg & Verge 1995, nur Bericht); „Turbulence noise
   increases with the supply pressure“ (Castellengo 2004, nur Bericht); kein Pegel relativ zum Ton gefunden → am
   Restsignal setzen (Messkette B4).
7. **Legato:** wie Klasse 8 — Moden je Griff überblendet (Maestre et al., DAFx-18, nachgelesen), eine Kette ist ein Ton
   (F-R4).

## 3. Je Instrument

| Instrument | Strahl, Geometrie | Spieler steuert | Datenlage | Risiko (Einschätzung) |
|---|---|---|---|---|
| **Blockflöte** | fest: Kanal „a little more than 1 mm high“ am Eingang, „about 0,9 mm“ am Ausgang (Bolton 1983, nur Bericht); Labor W = 4 mm, h = 1 mm, Innendurchmesser 19 mm (Auvray 2015, nur Bericht); w/h ≈ 4 (Castellengo 1999, nur Bericht) | nur Druck (und Vokaltrakt, F-R5) | gut (Parameter, Druck/PIV-Daten) | gering |
| **Orgel-Labialpfeife** | fest je Pfeife; Normmensur „155.5 mm … at 8′ C“, Mundbreite ein Viertel des Umfangs (Wikipedia, CC BY-SA, nur Bericht — prüfbedürftig); Einzelpfeife 312 mm × 27 mm, Mund 21 × 7 mm (Castellengo 2004, nur Bericht) | nichts je Ton; Wind fest je Werk, Tremulant, Schwellkasten (F-3) | mittel (VAO: Pfeifen mit Maßen, BY-SA) | gering–mittel; Einschwingen ist der Klangkern |
| **Querflöte** | W ≈ 1 cm, H = 6 mm, y0 = 0,2 mm, h = 1 mm (Terrien 2013, nur Bericht); U_j „20 to 60 metres per second“ (UNSW, nur Bericht) | Druck, W (Lippen vor), Öffnung, Abdecken, y0 (F-2) | Bohrung nur UNSW ohne Lizenz; Aufnahmen gut | mittel |
| Altflöte | wie Querflöte, skaliert | wie Querflöte | keine Geometrie | mittel–hoch |
| Shakuhachi | offene Kante ohne Kernspalt; Museumsmaß 603 mm, Bohrung 19,7–22,3 mm, utaguchi 11,4 × 2,8 mm (NMM, nur Bericht, Rechte ungeprüft) | Druck, Ansatz; meri/kari ändern den Resonator | schwach (ein Ton frei) | hoch |

## 4. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| Bohrung, Tonlöcher, Pfeifenmaße | fest (Bauplan je Griff/Pfeife) | Blockflöte Auvray; Pfeife VAO; Querflöte offen | Tonhöhe je Griff (M1) |
| W, h, y0, H, b | fest (Blockflöte, Pfeife) bzw. Kurve über den Umfang (Querflöte) | Terrien 2013, Auvray 2015, Bolton (nur Bericht) | Spektrum, H2/H1 (M2) |
| c_p/U_j, α_i, α_vc | fest | Terrien, Auvray (nur Bericht) | Schwellen (M3) |
| Blasdruck über Expression | Stärke (F-R2) | Querflöte rund 300–900 Pa (Terrien, nur Bericht); Blockflöte bis ~700 Pa vor dem Überblasen | Pegel, Schwerpunkt (M2), Schwellen (M3) |
| Ansatz der Querflöte (W, Abdecken, y0) über Expression | Weg (F-R2, F-2) | Fletcher 1975, de la Cuadra 2008 (nur Bericht, Zahlen nicht erreicht) | Tonhöhe stabil über pp/mf/ff (M1, M2) |
| Vibrato-Umsetzung | Spur (F-R3) | Druck ±10 %, um 5 Hz (Fletcher, nur Bericht) | Rate, Tiefe, AM (M4) |
| Mundtöne, Vorläufer | Pfad (F-4) | Castellengo 1999/2004 (nur Bericht) | Einschwingen (M5) |
| Rauschanteil | Kurve | offen; Shakuhachi „well pronounced around 9 kHz“ (Castellengo 2004, nur Bericht) | Restsignal (M6) |
| Tremulant (Orgel) | Werk (F-3) | Druckhub „some 75%“ Spitze–Spitze, Rate um 4–6 Hz (Pykett u. a., nur Bericht) | Modulation (M4) |

## 5. Rechenzeit

Keine Zahl für Flöten gefunden. Der Strahl ist eine Verzögerungsleitung mit einer tanh-Kennlinie, der Resonator modal
wie in Klasse 8 („above 30 times faster than real-time“, Maestre, nachgelesen) — also gleiche Größenordnung
(Einschätzung). Die Orgel braucht viele Pfeifen vorab, aber je Pfeife wenig (Einschätzung).

## 6. Abweichungen vom Rahmen

- SF6 a hat keine Zeile für Luftstrahl; Vorschlag in F-3 (Flöten wie Rohrblatt, Orgel ohne Spuren je Ton).
- Vibrato als Druckmodulation mit Amplitudenmodulation der oberen Teiltöne (F-R3).
- Expression an der Querflöte als Weg in (Druck, W, Abdecken, y0) (F-R2, F-2).
- Mundtöne als eigener Pfad neben dem modalen Kern (F-4).
