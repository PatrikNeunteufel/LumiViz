# Rohr — ein Rohrmodell für Lippe, Rohrblatt und Luftstrahl (Klassen 7, 8, 9)

> **Klasse 0 (Gemeinsam).** **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorab-Ausarbeitung, nichts entschieden.
> **Grundlage:** Bericht `recherche/2026-10-26_rohr_verfahren.md` (Agent); Rahmen aus [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md).
> **Kennzeichnung je Angabe:** **nachgelesen** (von dieser Sitzung im Volltext oder Feld selbst gelesen, mit Ort) ·
> **nur Bericht** (aus dem Agentenbericht; dort teils vom Agenten selbst im PDF gelesen, hier nicht wiederholt) ·
> **Einschätzung** (Claude, kein Beleg). Offene Entscheidungen: [offene-fragen.md](../../gemeinsam/ic/offene-fragen.md).

## 1. Was ein Rohr hier leisten muss

- **Klassen:** 7 Lippe (Trompete, Posaune, Horn, Tuba, Dämpfer), 8 Rohrblatt (Klarinette, Saxophon, Oboe, Fagott),
  9 Luftstrahl (Querflöte, Blockflöte, Shakuhachi, Labialpfeifen); dazu der **Resonator der Stabspiele** (Klasse 4,
  unten geschlossenes Rohr, siehe [koerper.md](../../koerper/ic/koerper.md) §2).
- **Grundsatz der Physik** (nur Bericht, Guillemain et al. 2005; Bilbao 2008): ein **lineares** Rohr (Bohrung,
  Wandverluste, Tonlöcher/Ventile, Abstrahlung) und ein **nichtlinearer** Anreger am Eingang (Bernoulli-Strömung durch
  Lippe oder Rohrblatt, Strahl an der Kante), gekoppelt über **Druck p und Volumenfluss u am Rohreingang**. Das Rohr
  lässt sich vorab aus der Bohrung rechnen, die Selbsterregung nur im Zeitbereich.
- **Steuerung über die Zeit — passt zum Bestand in LumiViz:** Ein Blaston hängt vom **Verlauf** der Steuerung ab
  (Anblasdruck, Lippenspannung, Ansatz); Einschwingen, Lautstärke und Klangfarbe entstehen daraus (Einschätzung). LumiViz
  sieht das schon vor (nachgelesen, `Spielweisen_Konzept.md`): SF10 a — „Expression, Vibrato, Pitch nur bei dauernder
  Anregung (Bogen, Lippe, Rohrblatt, Luftstrahl)“ und „das Rohr dient drei Anregungen“; SK12 a — „Spur und Kurve
  stehen im Namen des Tons; ein Clip mit Kurve hat eigene Töne“. Ein Ton je Note mit ihrer Spur ist also der heutige
  Weg des Tonspeichers (Schlüssel nach TV2 a). Offen ist nur, **wie** Expression (heute „0 … 1 als Stärke“, SK4 a),
  Vibrato und Pitch auf Anblasdruck, Lippenspannung und Ansatz abgebildet werden — eine Frage der Klassen 7–9, nicht
  dieses Kerns.

## 2. Verfahren für das Rohr

| Verfahren | Was es rechnet | Vorab / Zeitbereich | Kosten | Grenzen | Belegkraft |
|---|---|---|---|---|---|
| **Übertragungsmatrizen (TMM)**, 1D-FEM | Eingangsimpedanz aus der Bohrung | vorab (Frequenzbereich) | einmalig je Griff — unkritisch | TMM effizient verlustfrei und bei verlustbehafteten Zylindern, FEM bei vorgegebener Genauigkeit (Trompete mit Verlusten) | nur Bericht: Tournemenne & Chabassier 2019 (Abstract) |
| **Modale Impedanz** (Pole/Residuen) | Moden des Rohrs am Eingang; direkt aus der FEM-Diskretisierung möglich | Moden vorab, Kopplung im Zeitbereich | je Mode ein komplexer Resonator je Abtastwert — Einschätzung: 10–30 Moden, weit unter 1 % der Echtzeit | lineares, zeitlich festes Rohr — kein Halbventil, kein Schmettern; Moden direkt aus FEM nur mit Einschränkungen an die Abstrahlung | nur Bericht: Auvray & Chabassier 2022; Guillemain, Kergomard & Voinier 2005; Silva et al. 2014 (Moreesc) |
| **Digitaler Wellenleiter** (STK) | Laufzeit + Filter je Zylinder-/Kegelstück | Zeitbereich | sehr billig bei Zylinder/Kegel | feste Geometrie; beliebige gemessene Bohrungen umständlich | nur Bericht: STK-Doku; Bilbao 2008 |
| **Reflexionsfunktion** (McIntyre, Schumacher & Woodhouse 1983) | Faltung des Flusses mit der Reflexionsfunktion | Funktion vorab, Faltung im Zeitbereich | wächst mit der Länge der Funktion | — | nur Bericht |
| **FDTD** (Bilbao, Harrison) | Welle im Rohr auf einem Gitter | Zeitbereich | Bilbao: 6·L·f_s²/c Operationen je Sekunde, unabhängig vom Profil (nur Bericht); Zahlen unten | verlustfrei einfach; Wandverluste über Ableitungen gebrochener Ordnung teuer; Ventile mit Überlappung, Schmettern möglich | Zahlen **nachgelesen** |
| **FEM im Raum, Differenzen in der Zeit** (OpenWInD) | wie FDTD, energietreu, viskothermische Verluste im Zeitbereich | Zeitbereich | keine Zahl gefunden | — | nur Bericht: Thibault & Chabassier 2020; Ernoult et al. 2023 |

**Rechenzeiten aus der Literatur (nachgelesen):**

- Harrison, Bilbao & Perry, DAFx-15, Tabelle 6 (S. DAFX-7, im PDF-Cache der Sitzung gelesen): Ventil-Trompete,
  Rohr 1,381 m, ein Ventil, 1 s Ton, Intel Core i5-4300U — MATLAB 14,02 s, Serial C 2,69 s, GPU 12,75 s, AVX 1,22 s,
  Serial C (memory optimised) 1,98 s, **AVX (memory optimised) 0,86 s**. Dort auch: der teuerste Teil ist die Näherung
  der Ableitungen gebrochener Ordnung (Abschnitt 6).
- Bilbao, DAFx-08 (S. DAFX-5, im PDF-Cache gelesen): Klarinettengeometrie, **verlustfrei**, „approximately 3.9 s to
  generate 5 s of sound output, at 44.1 kHz, in Matlab“.

**Einschätzung zur Grenze von LumiViz** (Flügel: höchstens 4,6 % der Echtzeit je Ton im Bass): FDTD mit Verlusten liegt
bei rund 86 % — etwa das Zwanzigfache darüber, also **Deluxe** (UF16 a: Marke aus dem Verfahren). Die modale
Darstellung liegt weit darunter, weil Verluste, Schallstück und Tonlöcher schon in Dämpfung und Residuen der Moden
stecken.

## 3. Entwurf eines gemeinsamen Kerns (Einschätzung)

1. **Bohrung → Moden, vorab:** Bohrungsprofil (Radius über der Länge, Tonlöcher, Ventile) je Griff → Eingangsimpedanz
   (TMM oder 1D-FEM, mit Wandverlusten nach Zwikker–Kosten) → Moden (Frequenz, Dämpfung, Residuum) am Eingang. Ergebnis
   als **Kurven über den Tonumfang** (je Griff), wie die Größen des Flügels. Der Resonator der Stabspiele ist derselbe
   Baustein mit anderer Randbedingung (unten geschlossen).
2. **Anreger, je Note im Zeitbereich:** ein Baustein je Klasse mit derselben Schnittstelle p, u am Eingang —
   Lippe (ein Freiheitsgrad, nach außen schlagend; zwei Freiheitsgrade nach Adachi & Sato 1996 als Erweiterung),
   Rohrblatt (ein Freiheitsgrad mit Anschlag; Doppelrohrblatt nach Almeida et al. 2007), Strahl–Kante (jet-drive,
   Auvray et al. 2014). Rohrblatt und Lippe unterscheiden sich in der Schlagrichtung (nur Bericht: Bilbao 2008,
   Harrison et al. 2015).
3. **Abstrahlung:** am offenen Ende eine rationale Näherung der Abstrahlimpedanz nach Levine–Schwinger (Silva et al.
   2009, positiv-reell, damit stabil — nur Bericht: Bilbao 2008 warnt vor Reihen); bei Tonlöchern je Loch ein Ausgang.
   Als Körper nach KB1 a: je Mode ein Abstrahlgewicht je Ausgang.
4. **Deluxe:** FDTD/FEM im Zeitbereich für das, was modal nicht geht — Halbventil und Übergänge zwischen Griffen,
   nichtlineare Ausbreitung (Schmettern; nur Bericht: Hirschberg et al. 1996, Berjamin et al. 2015, Harrison et al.:
   „severe stability issues as shocks develop“).

**Woran der Entwurf kippt:** wenn Übergänge zwischen Noten (Legato, Halbventil) oder Schmettern als klangwesentlich
gelten — dann ist FDTD/FEM der Kern und Blech insgesamt Deluxe (Frage F-R1).

## 4. Daten

| Daten | Inhalt | Lizenz / Rechtslage | Belegkraft | Eignung |
|---|---|---|---|---|
| UNSW Music Acoustics (Wolfe, Smith u. a.) | gemessene Eingangsimpedanzen als Excel: Querflöte (B- und C-Fuß, klassisch), Klarinette (ohne Nachgiebigkeit des Rohrblatts), Sopran- und Tenorsaxophon, Posaune (Boutin) | **keine Lizenz** auf den Seiten; Fußzeile „© 1997-2006 Music Acoustics UNSW“ — Urheberrecht | nur Bericht | Prüfstein für die gerechneten Moden (Impedanz gegen Impedanz) — nur lokal, nach Anfrage weitergeben |
| Bohrungsprofile | — | — | nur Bericht: kein freier Datensatz für Trompete, Posaune, Horn, Oboe, Blockflöte, Orgelpfeifen gefunden | **Lücke** — eigene Vermessung oder Literaturwerte |
| TU Berlin (DepositOnce 19858) | reflexionsarme Aufnahmen von Blech und Holz, Richtcharakteristiken | „In Copyright“ am Ort (nachgelesen), Paper: CC BY-SA 4.0 — **ungeklärt** | siehe [koerper.md](../../koerper/ic/koerper.md) §4 | Aufnahmen für die Messung, Abstrahlung |

## 5. Offene Implementierungen (nur Vorbild — kein Code wird übernommen)

| Code | Was | Lizenz (Wortlaut, Ort, Abruf 2026-10-26) | Belegkraft |
|---|---|---|---|
| STK (Synthesis ToolKit) | Wellenleiter: Brass, Clarinet, Saxofony, Flute, BlowHole | „Permission is hereby granted, free of charge, to any person obtaining a copy …“ (MIT-artig), „Copyright (c) 1995-2023 Perry R. Cook and Gary P. Scavone“; Bitte um Rücksendung von Änderungen „not a binding provision of this license“ — Datei `LICENSE`, github.com/thestk/stk | **nachgelesen**; Patenthinweis Stanford/Yamaha in der Doku nur Bericht |
| OpenWInD (Inria, Python) | Impedanz aus der Bohrung, Moden, Zeitbereich mit Rohrblatt/Lippe; Flöte angekündigt | PyPI-Feld `license` „GPLv3“, Klassifikator „License :: OSI Approved :: GNU General Public License v3 (GPLv3)“, Fassung 0.12.4 (pypi.org/pypi/openwind/json) | **nachgelesen**; LICENSE im GitLab nicht erreichbar |
| Modalys (IRCAM) | modale Synthese | nicht quelloffen; Forum-Lizenz „Free“ bzw. „Premium“, Zuordnung unklar | nur Bericht |
| Moreesc | modal, Rohrblatt und Lippe | Software nicht gefunden | nur Bericht |

**Einschätzung:** OpenWInD taugt als **Werkzeug zum Gegenrechnen** (Impedanz einer Bohrung, außerhalb des eigenen
Codes); übernommen wird nur das Verfahren aus den Veröffentlichungen, kein GPL-Code (IK1: eigenes C++ ohne fremde
Laufzeit).

## 6. Offen

- Bohrungsprofile mit Quelle für je ein Instrument der Klassen 7, 8, 9 — größte Datenlücke.
- Lizenz der UNSW-Impedanzen klären (Anfrage) — vor einer Übernahme nach LumiViz.
- Volltexte frei zugänglicher Grundlagen lesen (Download mit Freigabe): Guillemain et al. 2005 (HAL), Tournemenne &
  Chabassier 2019 (HAL), Auvray et al. 2014 (HAL), Ernoult et al. 2023 (CC BY 3.0).
- Anblasgeräusch, Richtwirkung am Schallstück, Tonlochmodelle nach Keefe: nicht gesucht.
