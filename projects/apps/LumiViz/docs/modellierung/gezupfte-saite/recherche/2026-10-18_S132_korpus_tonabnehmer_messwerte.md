# Recherche S132: Messwerte für Korpus und Tonabnehmer

> **Herkunft:** bis S146 Abschnitt §6 und §6b in `docs/audio/Saiten_Konzept.md`, dort herausgezogen (S146,
> Wunsch Patrik: Recherchen unter `docs/modellierung/<klasse>/`). Inhalt unverändert; die Entscheide, die darauf
> bauen, stehen weiter im Konzept.

## Erste Recherche (S132)

Recherche durch einen Agenten, die tragenden Stellen von Claude selbst in der
Textebene der Quellen nachgelesen („bestätigt"). Noch nicht festgelegt
(S-K6 a: zweite Recherche für die Lücken). Nicht erreichbar waren: Rossing,
*The Science of String Instruments*; Paiva (Aalto); Paté/Le Carrou (HAL);
Gough (IOA); Zollner 5.15.

| Korpus | Belegt | Quelle | Lücke |
|---|---|---|---|
| Classical guitar (gewählt S-K10 a: TOP26, S. 138–140, §5f) | T(1,1) 165 Hz Q 36 · T(2,1) 204 Q 66 · B(1,2) 293 Q 50 · T(1,2) 391 Q 60 · T(3,1) 432 Q 51 · T(2,2) 607 Q 25 (Schallloch zu, je mit wirksamer Masse); Helmholtz 122 Hz Q 40 (im Modell gesetzt) | H. Wright, *The Acoustics and Psychoacoustics of the Guitar*, Diss. Cardiff 1996, S. 97–98, Tab. 5.4 — bestätigt | Stärke nur über die Masse |
| Acoustic guitar (Steelstring, Parlor) | T(1,1)₁ 116 Hz, T(1,1)₂ 219 Hz, Helmholtz 130 Hz; Modellwerte (Masse, Steife, Dämpfung) | D. P. Hess, *Savart Journal* 1(3), 2013, S. 3, Tab. 1 — bestätigt | Güte nur gerechnet; keine Dreadnought |
| Upright bass | A0 65–67 Hz, T1/A1 115 Hz, C2 145, C4 130, C3 155, A2 150/158 Hz | A. W. Brown, Diss. mdw Wien 2004, S. 80–83 — bestätigt (A0, T1/A1) | keine Güte, Stärke nur als Kurve |
| Violin | A0 270 Hz, Hauptholz 420 Hz, „thick plate" 505 Hz, Bridge Hill 2,7 kHz (Mittel vieler Violinen); Bandbreite je 14 Hz | E. Jansson, *Acoustics for Violin and Guitar Makers*, KTH 2002, S. 7.11 — Frequenzen bestätigt, Bandbreiten nur in der Abbildung | keine Stärken |
| Acoustic bass | — | — | nichts gefunden |

| Tonabnehmer | Belegt | Quelle |
|---|---|---|
| Resonanz mit Kabel | 2–5 kHz bei 2–10 H und 300–600 pF; Q ≈ 3 im Betrieb; Überhöhung ≈ 20·lg Q | M. Zollner, *Physics of the Electric Guitar*, 5.5.4 — bestätigt |
| Humbucker | Polabstand 18–19 mm, Kammfilter-Lücke; tiefere Resonanz, schwächere Überhöhung (qualitativ) | Zollner 5.4.4 — bestätigt; S. Patsch, *Physik Journal* 19 (2020) |
| Lage, Apertur | Beispiel 4 und 16 cm vom Steg bei 64 cm Mensur; Apertur ≈ 1 cm | Zollner 2.8.2, 5.4.4 — bestätigt |
| Piezo | Hochpass f = 1/(2πRC), etwa 106 Hz bei 1,5 nF und 1 MΩ | Zollner 6.2 — bestätigt |

## Zweite Recherche (S-K6 a, S132) — nur die Lücken

„Bestätigt" = von Claude in der Textebene der Quelle gefunden; „Abb." = aus
einer Kurve abgelesen (±1 dB, ±10 % in der Frequenz); „Händler" = über den
Agenten aus einer Händler- oder Ratgeberseite, nicht selbst gelesen.

| Lücke | Gefunden | Quelle | Stand |
|---|---|---|---|
| Violine, Frequenzen | A0 275 ± 9 Hz, CBR 407 ± 31, B1− 475 ± 16, B1+ 541 ± 22 (17 Violinen) | G. Bissinger, „Structural acoustics of good and bad violins", *JASA* 124(3), 2008, S. 1767 | bestätigt |
| Violine, Dämpfung | ζ ≈ 1 % der kritischen im 400-Hz-Band → Q ≈ 50 | Bissinger 2008, Abb. 4 | Abb., Q abgeleitet |
| Violine, Stärke | A0, B1−, B1+ strahlen stark ab, CBR schwach — keine Zahl je Mode | Bissinger 2008, S. 1766 | nur qualitativ |
| Kontrabass, Güte | — (Askenfelt 1982 nicht erreichbar) | — | **nicht gefunden** |
| Steelstring, größer als Parlor | Gibson Hummingbird: 103 Hz (Atmung), 188, 202, 262, 315 Hz; keine Güte | D. Russell, Penn State (Webseite) | nur über den Agenten |
| Single coil mit Last | Strat R = 6 kΩ, L = 2,2 H, C = 450 pF; Q der Spule 2,1–3,5 | Zollner/Zwicker, „Factoring in the Q", GITEC 2019, S. 2; Zollner 5.5.4 | bestätigt |
| Humbucker mit Last | Q = 1,83 an 333 kΩ ∥ 450 pF (Fender, CuNiFe); Resonanz ≈ 3 kHz | Zollner/Zwicker 2019, S. 6–7 | Q bestätigt, kHz Abb.; **keine Induktivität** |
| Lagen | Strat 152 / 98 / ≈ 41 mm vom Steg, Tele 152 / ≈ 41 mm (aus den Lagen vom Sattel gerechnet); Jazz Bass Steg 54 mm, Hals ≈ 152 mm; Les Paul widersprüchlich | S. Errede, UIUC Physics 406, „Fourier Analysis IV", S. 2; Bartolini (2025) | Errede bestätigt, mm abgeleitet; Bartolini nur über den Agenten |
| Mensur | Gitarre 25½″ = 648 mm (bestätigt bei Errede); klassisch 650 mm; E-Bass 34″ = 864 mm; Ukulele Sopran 13–14″, Konzert 15–16″, Tenor 17–18″; Violine 327 mm (bestätigt bei Jansson S. 7.3); Kontrabass 4/4 106–110 cm | Errede; Yamaha; Händler; Jansson; Thomastik-Infeld | teils bestätigt, teils Händler |

