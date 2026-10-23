# Messplan Hammer-Saite — wie Messungen das Modell absichern sollen

> **Typ:** Vorschlag (Claude, S146), noch nicht entschieden. Teil A ist Stufe 31d (HS2 a) und braucht kein eigenes
> Instrument; Teil B lässt sich nur an einem echten Flügel machen und eignet sich als **Forschungsauftrag** (etwa an
> eine Hochschule mit Akustiklabor); Teil C sind Anfragen nach vorhandenen Daten.
> **Bezug:** [modell.md](modell.md) §2 (welche Größe vorläufig ist), `docs/audio/Spielweisen_Konzept.md` §4.1.

## A. An *Salamander* (Yamaha C5, CC BY 3.0) — Stufe 31d

Ausgangslage: 16 Anschlagstufen je kleine Terz, Aufnahmen beim Loslassen (Hammer chromatisch, Resonanz in Terzen),
Pedalgeräusche; zwei Mikrofone rund 12 cm über den Saiten. Werkzeug: ein Messprogramm, das je Aufnahme die Teiltöne
verfolgt (Frequenz, Hüllkurve) — in den Unit-Tests oder als Werkzeug unter `tools/`, festzulegen.

| Größe im Modell | Messung | Gilt als gemessen, wenn |
|---|---|---|
| B (`inharmonicity`) | Teiltonfrequenzen f_k, Ausgleich f_k = k f0 √(1+Bk²) | je Taste B mit Streuung; Kurve in Rigauds Form passt auf ±10 % |
| `decay`, `lateDecay` | Hüllkurve des Grundtons, zwei Geraden in dB | erste und zweite Stufe je Taste getrennt erkennbar |
| `highDecay` | Abklingen der Teiltöne über der Frequenz | Gesetz σ(f) = b1 + b3 ω² passt (Bank 2010, Gl. 7) |
| `detune` | Schwebungsfrequenz der ersten Teiltöne | je Taste ein Wert in Cent |
| Hammer (p, Q0, Masse) | Helligkeit (Teilton n zum Grundton) über den 16 Stufen | Steigung je Teilton gegen die Rechnung; entscheidet HT2 (Kipppunkt Stulov) |
| Schritt der Stufe (HT6 a) | Hörvergleich zweier benachbarter Stufen | Schritt, bei dem man die Klangfarbe nicht unterscheidet |
| `damper`, `undampedFrom` | Aufnahmen beim Loslassen | T60 unter dem Dämpfer je Taste; die erste Taste ohne Abfall |
| `bridgeAngle` | Pegel der Phantomteiltöne im Bass bei ff | Verhältnis zum Querteilton wie gerechnet |
| Boden (HT7 a, vorläufig) | Aufnahme durch gerechnete Kraft am Steg an allen Teiltönen | geglätteter Betrag 30 Hz – 10 kHz, Streuung je Band |

Grenzen: ein Instrument in einem Raum mit zwei Mikrofonen — Raum und Mikrofon stecken in jeder Messung; die Saiten­
daten (Länge, Masse) des C5 sind nicht bekannt, das Modell rechnet mit denen eines Steinway D (RT-0425).

## B. An einem echten Flügel — Forschungsauftrag

Ziel: die Werte, die frei nirgends zu haben sind (Befund S146), an **einem** Konzertflügel messen und frei
veröffentlichen (etwa CC BY 4.0), damit LumiViz sie verwenden darf.

1. **Treibpunkt-Mobilität am Steg** an rund 15 Saitenpunkten (Bass- und Diskantsteg), Impulshammer und
   Beschleunigungsaufnehmer, 20 Hz – 10 kHz, Betrag und Phase; mit und ohne gedämpfte Saiten. Vorbild: Plath 2019
   §6.3, Mores (Gitarren).
2. **Antwort Steg → Schalldruck** (Impulsantwort) an denselben Punkten, Anregung mit Impulshammer oder
   Kontaktanreger (Vorbild AudioLabs Erlangen: 10 s Chirp), Mikrofone in festen Lagen, Deckel offen und geschlossen,
   möglichst im reflexionsarmen Raum.
3. **Modalanalyse** des Bodens 50 Hz – 3 kHz (Raster mit Impulshammer, hochauflösende Verfahren wie ESPRIT nach Ege
   2013): Frequenzen, Dämpfungen, Modenformen an den Stegpunkten.
4. **Hammer:** Geschwindigkeit kurz vor dem Anschlag je Stufe (Laser oder Hochgeschwindigkeitskamera), Kontaktzeit
   und -kraft an einigen Tasten.
5. **Saiten:** Länge, Durchmesser, Wicklung, Spannung (aus Frequenz und Masse) aller Tasten — oder die Mensur des
   Herstellers.
6. **Geometrie:** Umriss des Bodens, Lage, Zahl und Querschnitt der Rippen, Lage der Stege, Dicke an einigen Punkten.

Aus 1–3 entsteht ein Körper nach KB1 a direkt aus Messung; 4–6 sichern Hammer und Saite. Mögliche Partner (nicht
angefragt): Hochschulen mit Messungen an Flügeln und Gitarren (Uni Hamburg / HAW Hamburg, KTH, LAM Paris,
AudioLabs Erlangen).

## C. Anfragen nach vorhandenen Daten (nicht gestellt)

- AudioLabs Erlangen: Freigabe der gemessenen Impulsantworten des Bodens eines Yamaha C3 (Seite
  „2023-piano-synth-lbd“, ohne Download und Lizenz).
- Plath 2019: Rohdaten Zenodo 3271370 (CC BY 4.0) prüfen — Kraft und Beschleunigung, keine Mobilität; ob sich eine
  Übertragung daraus gewinnen lässt.
- Rau, Smith & Abel (CCRMA, FA2023): Lizenz der Messungen an Gitarren und Violinen klären (für `koerper/`).
