# Modell der Hammer-Saite (Stand Stufe 31b, S146)

> **Typ:** eigene Ausarbeitung — was LumiViz rechnet, mit Gleichungen und der Herkunft jeder Größe.
> **Entscheide und Begründungen:** `docs/audio/Spielweisen_Konzept.md` §4.1 (HS1 a, HA1 a–HA4 a, HK1 a, HK2 a,
> HT1 a–HT6 a). **Code:** `include/loop/HammerSaite.hpp`, `include/loop/HammerTon.hpp`, `src/loop/HammerTon.cpp`.
> **Tests:** `tests/unit/UnitTests/test_LoopHammerSaite.cpp`, `test_LoopHammerTon.cpp`.
> „Vorläufig“ heißt: ohne Beleg gesetzt, die Messung an *Salamander* (Stufe 31d) setzt es.

## 1. Überblick

Modal nach Bank, Zambon & Fontana 2010 (*A Modal-Based Real-Time Piano Synthesizer*, IEEE TASLP 18(4)): je Teilton
der Saite gedämpfte Resonatoren, angestoßen von einem nichtlinearen Filzhammer, abgenommen als Kraft am Steg. Der
Chor (zwei oder drei Saiten je Taste) als gekoppelte Moden; die Längsschwingung als Spannung und Längsmoden; der
Dämpfer beim Loslassen. Ein Ton wird vorab gerechnet (je Taste, Dauer, Stufe einmal) und liegt dann im Tonspeicher.
Der Resonanzboden fehlt noch (Stufe 31c, vorläufig nach HT7 a, sauber nach KB1 a).

## 2. Größen über die Tastatur

Jede Größe ist eine Kurve aus Stützpunkten an Tasten (MIDI 21–108), in ihrer Skala monoton kubisch interpoliert
(HK1 a); eine Taste kann abweichen.

| Größe (Datei) | Skala | Vorgabe, Herkunft |
|---|---|---|
| Inharmonizität B (`inharmonicity`) | log | bis D♯2 gemessen an *Salamander* (Yamaha C5), durch das Modell hindurch (31d-4); darüber Rigaud, David & Daudet 2011, Gl. 4/5 (Diskant s_T = 9,26·10⁻², y_T = −13,64) |
| Abklingzeit 1. Stufe (`decay`) | log | gemessen an *Salamander*: T60 bei f1 aus dem Ausgleich über die Teiltöne, durch das Modell hindurch (31d-4; 16 s bei A0 … 1,8 s bei F♯7) |
| Abklingzeit 2. Stufe (`lateDecay`) | log | vorläufig dreimal `decay` — das Modell löscht an den Einbrüchen fast vollständig aus und trennt die zwei Stufen nicht wie *Salamander*; die zweite Polarisation (§5) kommt mit 31d-7 (N2 a), danach wird neu gesetzt |
| Hohe Teiltöne (`highDecay`) | log | gemessen an *Salamander* (T60 bei 10·f1 zu T60 bei f1): Bass um 0,95, C4 0,64, Diskant 0,02–0,04 (31d-4) |
| Saiten im Chor (`strings`) | linear, gerundet | vorläufig 1 bis F♯1, 2 bis C3, 3 darüber |
| Verstimmung (`detune`) | linear | gemessen an *Salamander* an der Schwebung der Teiltöne 1–3, durch das Modell hindurch, ab C♯2 (zwei Saiten): 0,6–5 Cent (31d-4); im hohen Diskant ohne Treffer, interpoliert |
| Hammermasse, Exponent, Steife (`hammerMass`, `hammerExponent`, `hammerStiffness`) | linear, linear, log | Chabassier & Duruflé, RT-0425, Gl. 2–4 (HT2 a), Steife umgerechnet Q0 = K_H·10⁻³ᵖ N/mm^p |
| Anschlagstelle (`strikePosition`) | linear | 0,12 der Länge ab Agraffe (RT-0425 Anhang A) |
| Dämpfer (`damper`) | log | vorläufig 0,8 s … 0,2 s — aus *Salamander* nicht messbar (D1 a); `undampedFrom` 89 aus der SFZ von *Salamander* |
| Länge, Masse je m (`stringLength`, `stringMass`) | log | RT-0425 Anhang A, µ = ρπd²/4 (HT1 a) |
| Längsverhältnis (`longitudinalRatio`) | linear | RT-0425 Anhang A, √(EA/T0), E = 2,02·10¹¹ Pa |
| Winkel am Steg (`bridgeAngle`) | linear | gemessen an *Salamander* an den Phantomteiltönen bis A2, durch das Modell hindurch (31d-4): 0,7–4,4°, an MIDI 33 die Grenze 10° (auffällig); darüber 4,4° |

Abgeleitet: f0 = f_Taste/√(1+B) (der erste Teilton liegt auf der Tonhöhe); T0 = 4L²f0²µ; ES = µ(2L·f_l0)².

## 3. Saite

Teilton k: f_k = k·f0·√(1 + B·k²), bis 0,45·fs (höchstens 400). Abklingen (Bank 2010, Gl. 7, in der Form von
`highDecay`): σ_k = σ_1·(1 + h·((f_k/f_1)² − 1)) mit σ_1 = ln 1000 / `decay` und h = (1/`highDecay` − 1)/99.

Jede Mode ist ein komplexer Zustand c mit Pol p = e^(λΔt) nach der Impulsinvarianz (Bank 2010, Gl. 10–11):
c ← p·(c + K·F·Δt); die Auslenkung der Mode ist Im c. Eingang K = (b0/ω_k)·g·sin(kπ x_H), b0 = 2/(Lµ) (Gl. 4c);
ein Takt Verzögerung — keine verzögerungsfreie Schleife mit dem Hammer.

Ausgang (Kraft am Steg, Gl. 16): F_b = cos α · Σ_k (kπT0/L)·Y_k, Y_k die Summe der Auslenkungen über die Saiten des
Chors.

## 4. Hammer

Masse m_H mit der Feder F = Q0·(Δy[mm])^p für Δy > 0 (Bank 2010, Gl. 12; ohne Hysterese, wie Bank). Δy = y_H − y_S,
y_S = Σ_k sin(kπ x_H)·Y_k / N (der Hammer trifft N Saiten, die sich während des Kontakts gemeinsam bewegen).
Geschwindigkeit beim Anschlag v = 5 m/s · 10^(Stufe/20) (HK2 a, HT6 a). Der Kontakt läuft achtfach feiner, bis der
Hammer 1 ms frei ist (höchstens 30 ms).

## 5. Chor: gekoppelte Moden (HT3 a)

*Herleitung (Claude, S146 — Weinreichs Arbeit selbst nicht gelesen; das Bild nach Banks Zusammenfassung,
Dissertation 2006, Kap. 4.2):*

Teilton k, Saite j = 1…N mit der komplexen Amplitude a_j, ihrer Kreisfrequenz ω_j = ω_k·2^(c_j/1200) (c_j die
Verstimmung: bei zwei Saiten ±c/2, bei drei −c/2, 0, +c/2 — `detune` ist der Abstand der äußeren) und dem eigenen
Verlust σ_s. Der Steg nimmt eine Kraft auf, die von allen Saiten zugleich kommt, und dämpft jede mit dem Verlust σ_b
auf ihre Summe:

    da/dt = A·a,  A = diag(iω_j − σ_s) − σ_b·J  (J: N×N, lauter Einsen)

Bei gleichen Frequenzen hat A den Eigenwert iω − σ_s − N·σ_b für die Gleichlage (Vektor aus Einsen) und iω − σ_s
(N − 1-fach) für die Gegenlagen. Die beiden Stufen sind damit σ_früh = σ_s + N·σ_b und σ_spät = σ_s: aus `decay` und
`lateDecay` folgen σ_s = σ_spät und σ_b = (σ_früh − σ_spät)/N (je σ = ln 1000 / T60). Mit einer Saite gilt nur
`decay`. Der Hammer stößt jede Saite mit F/N an (die Saiten bewegen sich während des Kontakts gemeinsam, der Hammer
sieht ihre Masse N·Lµ), am Steg zählt die Summe: mit A = VΛV⁻¹ ist die Antwort am Steg
1ᵀ·e^(At)·1/N = Σ_m g_m·e^(λ_m t), g_m = (1ᵀv_m)(w_mᵀ1)/N — je Teilton N komplexe Moden. Ohne Verstimmung ist g = 1
auf der Gleichlage und 0 auf den Gegenlagen: keine zweite Stufe — erst die Verstimmung legt Energie hinein. Über die
Teiltöne bleibt das Verhältnis σ_spät/σ_früh fest (von Claude gesetzt, vorläufig bis 31d).

*Rechnung:* A ist eine Diagonale minus eine Störung vom Rang eins; die Eigenwerte sind die Nullstellen von
R(z) = Q(z) + σ_b·Q′(z) mit Q(z) = Π (z − d_j), gelöst mit Durand–Kerner. Der Eigenvektor ist v = (D − λ)⁻¹·1; A ist
komplex symmetrisch, also ist der linke vᵀ, und g = (1ᵀv)²/(N·vᵀv).

## 6. Längsschwingung (31b-2, HT4 a)

Nach Bank 2010, Abschnitt V: die gleichmäßige Spannung T(t) − T0 = ES·π²/(4L²)·Σ n²·y_n² (Gl. 26) und K Längsmoden
(f_l,k = k·f_l0 bis 0,45·fs) mit ihrer dynamischen Antwort (Gl. 27, 28), angeregt von Paaren der Quermoden
(Gl. 22b; die Kraft von Mode 3 für die ungeraden, die von Mode 4 für die geraden). Im Chor bewegt sich jede Saite
mit Y/N (von Claude so genähert). Quermoden in die Produkte nur bis fs/4 (kein Aliasing). Abklingzeit der Längsmoden
ein Zehntel von `decay` („an order of a magnitude lower“, Bank, Diss. 2006, S. 80). Hochpass 20 Hz gegen den
Gleichanteil der Spannung (von Claude). Am Steg mit sin α (Chabassier, Joly & Chaigne 2013, Gl. 21). Gerechnet,
bis die Spannung 100 dB unter ihrer Spitze liegt (höchstens 10 s).

## 7. Dämpfer und Länge des Tons

Beim Loslassen (Taste unter `undampedFrom`) wird jeder Pol auf σ = max(σ, ln 1000 / `damper`) gesetzt — der
Dämpfer verkürzt die Abklingzeiten (Bank 2010, S. 12). Die Länge des Tons: gehalten plus Nachklang bis −60 dB unter
dem Dämpfer bzw. ohne Dämpfer bis −60 dB der zweiten Stufe, höchstens 30 s; am Ende 5 ms ausgeblendet. Ausgabe
in Einheiten von 200 N (`kHammerKraftBezug`, von Claude; den Pegel stimmt 31h).

## 8. Anschlagstärke (HA3 a, HK2 a, HT6 a)

laut der Note ∝ Hammergeschwindigkeit; Stufe = round(20·lg laut / 1 dB) auf −40 … 0, im Namen des Tons `:a<stufe>`;
der Rest laut / 10^(Stufe/20) ist Pegel am Schlag. Die Planung erfährt den Hammer über die Auskunft `InstrumentVon`
(HT5 a).

## 9. Geprüft (Release-clang, S146)

E1 fällt in 3 s um 8,14 dB (soll 8,14); Teiltöne 5, 10, 15 auf 1 Cent nach B; ff gegen pp heller (Teilton 8 zum
Grundton −21 gegen −32 dB); zwei Stufen mit Verstimmung (C4 13,6 → 2,5 dB/s); Dämpfer; Phantomteilton wächst mit ES
(+18 dB bei neunfachem ES); Auslenkung gegen Chabassier Tab. IV 0,4 … 0,6 der Tabelle; Rechenzeit 2,3 % der
Echtzeit im Bass, 1,1 % in der Mitte.

## 10. Bekannte Grenzen

- Kein Resonanzboden (31c), keine Rückwirkung des Bodens auf die Saite außer über die Kurven.
- Hammer ohne Hysterese und ohne Schaft; Chor-Näherung (gemeinsame Bewegung beim Kontakt); Längsschwingung je Saite
  mit Y/N.
- Viele Vorgaben vorläufig (Tabelle in §2); die Auslenkung liegt unter Chabassiers Werten (Ursache offen).
