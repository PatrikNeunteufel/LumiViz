# Modell-Entwurf — Platte und Stab (Klasse 4)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Bestand in LumiViz ([AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md) „Bestand in LumiViz“, IC-B1 a), gemeinsame Grundlage
> [gemeinsam/](../../gemeinsam/ic/README.md). Berichte: `recherche/2026-10-26_physik_bauteile.md`, `…_spielweisen.md`,
> `…_aufnahmen.md`.
> **Kennzeichnung je Angabe:** **nachgelesen** (von dieser Sitzung im Volltext, Feld oder Code selbst gelesen, mit Ort)
> · **nur Bericht** · **Einschätzung** (Claude) · **offen**. Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand und Rahmen

- **Bauart** (nachgelesen, `Spielweisen_Konzept.md` SF10 a): Vibraphon Schlägel × Stab mit Damper; Marimba, Glocke,
  Steel drum Schlägel × Stab/Platte; Rhodes (`tine_piano`) Hammer × Stab mit Damper und Tonabnehmer; Kalimba, Thumb
  piano Zupfen × Stab. Wurlitzer und Celesta stehen dort nicht (Einschätzung: Hammer × Stab mit Damper und Tonabnehmer
  bzw. Hammer × Stab mit Damper).
- **Steuerung** (nachgelesen, SF6 a): Sustain nur über die Ausstattung *Dämpferleiste*; Expression, Vibrato, Pitch:
  nein; „der Motor des Vibraphons ist Klang-Spielweise“. SF8 a: Motor an/aus als Spur über eine Passage („das Modell
  rechnet den Übergang“); Wirbel und Abdämpfen je Note. SK6 a: halbes Pedal als Andruck des Dämpfers.
- **Vorbild Flügel** (nachgelesen, `modellierung/hammer-saite/modell.md`): Moden als komplexe Resonatoren nach
  Impulsinvarianz (Bank 2010), nichtlinearer Hammer F = Q0·Δy^p im Zeitbereich achtfach feiner, Dämpfer als
  Erhöhung von σ, Ton vorab je Taste, Dauer und Stufe.

## 2. Kern: Stab modal, Schlägel im Zeitbereich (Einschätzung)

Der Stab ist linear; nur der Kontakt ist nichtlinear. Das ist dieselbe Trennung wie beim Flügel (Saite modal, Hammer im
Zeitbereich) — der Rechenkern der Hammer-Saite trägt fast unverändert.

1. **Moden vorab, je Taste:** Frequenz f_n, Dämpfung σ_n, Formwert φ_n(x) an Anschlag- und Abnahmestelle, modale Masse
   m_n — für Biege- **und** Torsionsmoden (Torsion wird außerhalb der Längsachse angeregt und zieht Energie ab;
   nachgelesen, Rucz & Ulveczki 2021, S. 7, Abb. 5).
2. **Kontakt im Zeitbereich:** Schlägel als Masse m_M mit Hertz-Feder F = K_M·δ^(3/2) (nachgelesen, Rucz & Ulveczki,
   Gl. 6, Tab. 1); für Filz/Neopren (Rhodes, Wurlitzer, Celesta) mit Hysterese nach Hunt–Crossley,
   F = k·x^α + λ·x^α·ẋ (nur Bericht: Pfeifle 2017; Sønderbo 2024). Während des Kontakts überabgetastet (Rucz: „an
   oversampling of 100–200 times“, nachgelesen S. 5; der Flügel rechnet achtfach).
3. **Gleichungen** (Einschätzung, Form wie Bank 2010 Gl. 4c/10–12):
   q̈_n + 2σ_n q̇_n + ω_n² q_n = φ_n(x_s)·F / m_n;  y(x_s) = Σ φ_n(x_s) q_n;  m_M ÿ_M = −F;  δ = y_M − y(x_s), F = 0 für δ ≤ 0.
4. **Abnahme:** Schalldruck als Σ (Abstrahlgewicht_n · q̈_n) — der Stab ist der Körper (KB1 a: Liste von Moden mit
   Einkoppelpunkt und Abstrahlgewicht), beim Rhodes/Wurlitzer stattdessen der Tonabnehmer (§4).
5. **Dämpfer:** wie beim Flügel σ_n ← max(σ_n, σ_Dämpfer) mit dem Andruck 0…1 aus SK6 a; beim Vibraphon die ganze
   Leiste über das Pedal, bei Celesta/Rhodes/Wurlitzer je Taste (Frage F-3).

**Woher der Stab seine Moden bekommt** (Frage F-1 im Zielbild, Einschätzung):
- **gemessen:** Frequenz, Dämpfung, Amplitude je Mode aus der Aufnahme mit dem Modalfit ohne Reihe (ESPRIT/ESTER,
  [gemeinsam/messkette.md](../../messung/ic/messkette.md) B6) — die Stimmung des Unterschnitts steckt dann in den Daten;
- **gerechnet:** 1D-Balken (Timoshenko oder Schubbalken) mit veränderlichem Querschnitt (Unterschnittprofil) als
  Bauplan, Eigenlöser wie beim Körper; das Profil aus Zielverhältnissen (1 : 4 : 10 Marimba, 1 : 3 : 9 Xylophon —
  nachgelesen Rucz & Ulveczki S. 1; Vibraphon 1 : 4 — nur Bericht CCRMA). Ein gleichförmiger Stab trifft das nicht:
  frei-frei liegen die Moden bei 1 : 2,76 : 5,40 (Einschätzung, Lehrbuch; Skript unten).
- Für die Kurven über den Tonumfang reicht „gemessen“ (Verhältnisse und Dämpfung je Mode als Kurven); „gerechnet“ ist
  nötig, wenn ein Editor den Stab bauen soll (Holz, Maße) — wie BM1 a beim Flügel.

## 3. Resonator und Motor

- **Physik** (nachgelesen, Rucz & Ulveczki 2021, gekoppeltes 3D-FE-Modell, Orff-Xylophon F4): ein gestimmter
  Resonator hebt den Pegel „by as much as 15 dB“ und kürzt T₋₆₀ von „close to 10 s“ auf 0,7 s; betroffen ist vor allem
  der Grundton; Nachbarstäbe verschieben die Resonatorfrequenz (ohne Stäbe einen Halbton höher, S. 6); Resonator
  f_a,1 = 349,1 Hz, ξ_a,1 = 8,8·10⁻³ gegen den Stab 348,8 Hz, ξ_m,1 = 2,57·10⁻⁴ (Tab. 1).
- **Entwurf** (Einschätzung): Stabmode 1 und Rohrmode als **zwei gekoppelte Schwinger** — dieselbe Form wie der
  Gitarrenkorpus in LumiViz (Decke + Luft, S-K8 a) und die gemeinsame Erweiterung „Luftraum“
  ([gemeinsam/offene-fragen.md](../../gemeinsam/ic/offene-fragen.md) F-K1); das Rohr selbst aus
  [gemeinsam/rohr.md](../../rohr/ic/rohr.md) (unten geschlossen, eine bis drei Moden), Abstrahlung am offenen Ende
  als Kugelquelle (nur Bericht: Doutaut, Matignon & Chaigne 1998). Die Rückwirkung des Rohrs auf den Stab ist nicht
  vernachlässigbar — Rucz: „cannot be generalized“ (nachgelesen S. 1).
- **Motor des Vibraphons:** die Klappe ändert periodisch die Öffnung des Rohrs → Kopplung und Abstrahlgewicht des
  Resonators werden zeitabhängig, g(t) = g_offen·w(θ(t)), θ die Drehung (Einschätzung). Drehzahl 1–12 U/s (nur
  Bericht: Patente US 9 922 628, US 10 847 126); Hochlauf mit einstellbarer Beschleunigung; Haltewinkel „aus“ offen
  (US 9 922 628) — alle nur Bericht. **Offen:** Modulationstiefe, ob eine Umdrehung eine oder zwei Perioden gibt.
  Weil die Phase des Motors vom Zeitpunkt der Note abhängt, ist ein Vibraphonton mit Motor **eine Funktion der Spur**
  (Weg SK12 a: Spur im Namen des Tons) — oder der Motor wird nach dem Ton als Modulation des Resonatoranteils
  angewandt (dann bleibt der Ton je Taste und Stufe vorab; Frage F-2).

## 4. Je Instrument

| Instrument | Schwinger | Anregung | Besonderes | Risiko (Einschätzung) |
|---|---|---|---|---|
| Marimba | Holzstab, Unterschnitt 1 : 4 : 10 | Schlägel (Garn, Kordel, Gummi) | Resonator; Torsion | gering–mittel |
| Xylophon | Holzstab 1 : 3 : 9 | harte Schlägel | Resonator (schwächer, höhere Lage) | gering |
| Vibraphon | Aluminiumstab 1 : 4 | Schlägel | Resonator mit Motor, Pedal-Dämpferleiste, lange Ausklingzeit | mittel (Motor ohne Messwerte) |
| Glockenspiel | Stahlstab | Kunststoff, Messing | „All but the lowest transverse mode die out quickly“ (nur Bericht, CCRMA) | gering |
| Röhrenglocken | Messingrohr (Biegemoden eines Rohrs) | Hammer | Schlagton aus Moden 4–6 (nur Bericht, CCRMA) — Wahrnehmung, gegen Aufnahme prüfen; Dämpferleiste mit Pedal | mittel |
| Celesta | Stahlstab mit Holzkasten je Ton | Filzhammer über Tastatur | Einzeldämpfer, ein Pedal (nur Bericht) — fast ein kleines Klavier | mittel (keine Fachquelle) |
| Kalimba | Zunge, frei–gestützt–eingespannt | Zupfen (Daumen/Nagel) | f₁/f₀ 5,3–5,9 (nur Bericht, Chapman 2006); Kasten mit Löchern (Luftraum F-K1) | gering |
| Steeldrum | Schalenfelder | Stöcke mit Gummi | **geometrisch nichtlinear**, innere Resonanzen 1:2:2:4 …, Kopplung ab „1/25 times the thickness“ (nur Bericht, Thomas et al. 2014) — quadratische/kubische Kopplung der Moden im Zeitbereich | hoch → Deluxe |
| Rhodes | Tine + Tonebar (gekoppelt) | Hammer mit Neopren (5 Härtestufen) | **Tonabnehmer** elektromagnetisch, nichtlinear (nur Bericht: Pfeifle 2017, Falaize & Hélie); zwei Polarisationen | mittel–hoch |
| Wurlitzer | Zunge (Kragbalken, Lötzinn zum Stimmen) | Filzhammer | **elektrostatischer Tonabnehmer**, i = u₀·∂C/∂t, C(x) aus FEM (nur Bericht, Pfeifle 2017) | mittel |

**Tonabnehmer** (Rhodes, Wurlitzer): die Moden bleiben linear und vorab; die Nichtlinearität sitzt **nach** dem
Schwinger als statische Kennlinie der Auslenkung an der Spitze (Rhodes: Fluss Φ(x, y), Wurlitzer: C(x)) — billig,
und wie die Tonabnehmer der Saite in LumiViz ein Ausgang des Modells (Einschätzung). Das Tremolo von Suitcase und
Wurlitzer ist ein Effekt nach dem Instrument (Amplitude/Panning; nur Bericht, fenderrhodes.com, Wikipedia), kein Teil
des Schwingers.

## 5. Größen und ihre Herkunft

Kurven über den Tonumfang wie beim Flügel (Stützpunkte, Vorgabe = bestes gemessenes Modell).

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an (siehe [messplan.md](messplan.md)) |
|---|---|---|---|
| Verhältnisse f_n/f_1 der Biegemoden (n = 2, 3) | Kurve | Literatur: 1 : 4 : 10 bzw. 1 : 3 : 9 (nachgelesen, Rucz S. 1); Marimba 9,2 (nur Bericht, CCRMA) | Modalfit an Iowa (M1) |
| Torsionsmoden (Frequenz, Anteil) | Kurve | offen (La Favre: Frequenz der 1. Torsion, nur Bericht) | Modalfit, Anschlag außerhalb der Mitte (M1) |
| Dämpfung je Mode σ_n | Kurve je Mode | Anker F4 Palisander ξ₁ = 2,57·10⁻⁴ (nachgelesen, Rucz Tab. 1); Gesetz über f offen | Abklingen je Mode (M2) |
| Abstrahlgewicht je Mode | Kurve | offen | Pegel je Mode bei bekanntem Anschlag (M3) |
| Schlägel: Masse m_M, Steife K_M, Exponent | fest je Schlägelart | 25 g, 5·10⁷ N/m^1,5, 3/2, Kontakt 0,6 ms bei 1 m/s (nachgelesen, Rucz Tab. 1, ein Orff-Schlägel); Garn/Kordel/Gummi offen | Helligkeit über der Stärke je Schlägel (M4) |
| Anschlaggeschwindigkeit je Stufe | Stärke | wie Flügel (HK2 a) | Pegel über pp/mf/ff (M4) |
| Anschlagstelle | Spielweise, Vorgabe | offen (Mitte regt Mode 2, 4 kaum an — nur Bericht, La Favre) | Verhältnis Mode 2 zu Mode 1 (M3) |
| Resonator: f_a, ξ_a, Kopplung | Kurve | F4: 349,1 Hz, 8,8·10⁻³ (nachgelesen, Rucz Tab. 1); Längen La Favre (nur Bericht) | Abklingen und Pegel des Grundtons mit/ohne Resonator (M5) |
| Motor: Drehzahl, Hochlauf, Haltewinkel | Spielweise / Ausstattung | 1–12 U/s, Halt offen (nur Bericht, Patente) | Modulationsrate und -tiefe am Vibraphon (M6) — **ohne Aufnahme mit bekanntem Motor offen** |
| Dämpfer σ_D, Andruck | Ausstattung | offen (Vibraphon gedämpft: Aluminium halbe Amplitude in 1,5 s, Palisander 0,5 s — Patent, nur Bericht, Herstellerprosa) | Iowa „Dampen“ (M7) |
| Rhodes: Tine-Länge, Tonebar, Hammer, Abstand Tonabnehmer | Kurve | Tine 0,1564 … 0,0226 m nach Service Manual; Hammer M_h = 0,011 kg, Hunt–Crossley α 2,8 (nur Bericht, Sønderbo Tab. 3.1/3.2); Tonabnehmer offen | Helligkeit und Klirr über die Stärke (M8) — Aufnahme fehlt (F-7) |
| Wurlitzer: Zunge, Spannung, C(x) | Kurve | 130 V gemessen (Handbuch 170 V) (nur Bericht, Pfeifle) | wie Rhodes (M8) |
| Kalimba: f₁/f₀, Maße | Kurve | D3/D4 aus Chapman Tab. 1 (nur Bericht) | Modalfit (M1) |
| Steeldrum: Kopplungskoeffizienten | — | offen | Deluxe; Aufnahme nur NC (F-7) |

## 6. Rechenzeit (Einschätzung, Skript im Scratchpad der Sitzung)

```python
# stababschaetzung.py, gekürzt: Euler–Bernoulli frei-frei, beta_n L = 4.730, 7.853, 10.996, 14.137, dann (2n+3)pi/2
# Ergebnis: Verhältnisse 1 : 2,757 : 5,404 : 8,933 : 13,345
#   C2 (65,4 Hz): 25 Biegemoden bis 20 kHz; gleichförmiger Palisanderstab von 552 mm bräuchte nur 4,2 mm Dicke
#                 für f1 — die echten Stäbe sind rund 39 mm dick (La Favre, nur Bericht): der Unterschnitt trägt die Stimmung
#   C4: 12 Biegemoden; C7 (2093 Hz): 4 Biegemoden
# modal 60 Moden (Biegung + Torsion) × 8 Operationen × 48 kHz = 23 Mio. Operationen je Sekunde Ton;
# Kontakt einmalig ~5 Mio. (1 ms, 200-fach überabgetastet) → bei grob 1e9 Operationen/s unter 3 % der Echtzeit
```

Damit liegt ein Stabton unter der Grenze des Flügels (4,6 %); vorab je Taste und Stufe wie der Flügel. **Deluxe**
(UF16 a): die Steeldrum (nichtlineare Kopplung im Zeitbereich) und ein gerechneter Stab aus Bauplan (3D-FE wie Rucz,
nur vorab im Werkzeug).

## 7. Abweichungen vom Rahmen

- **Teiltonreihe:** die Messung des Flügels sucht k·f0·√(1+Bk²) — für Stäbe falsch; nötig ist der Modalfit ohne Reihe
  ([gemeinsam/messkette.md](../../messung/ic/messkette.md) A1, B6).
- **Motor:** ein zeitabhängiger Teil im Ton (Phase des Motors), siehe §3 und F-2.
- **Spielweisen außerhalb von SF6 a** (Bogen am Vibraphon, Pitch-Bending, Kalimba-„Wa-Wa“) — siehe F-4; nicht im
  Entwurf.
