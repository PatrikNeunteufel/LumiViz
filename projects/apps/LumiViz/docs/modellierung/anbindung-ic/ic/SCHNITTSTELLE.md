# Grundidee der Schnittstelle — aus dem Flügel (unverbindlich)

> **Stand:** 2026-10-26 (LumiViz S154). **Verbindlich ist nichts davon** — die Abstimmung mit LumiViz kommt später
> (IK1). Die Entwürfe je Klasse (`modell-entwurf.md`) sollen sich an diesen Rahmen halten oder begründet abweichen.

## So rechnet LumiViz den Flügel

1. **Bauart** = Anregung × Schwinger (SF10 a), dazu ein **Körper** als eigene Datei (Bauplan → gerechnete Moden).
2. **Größen als Kurven über den Tonumfang:** Stützpunkte (Taste, Wert), dazwischen glatt interpoliert; je Größe eine
   Vorgabe — „die Vorgabe ist das beste gemessene Modell“; jede Vorgabe trägt ihre Herkunft (Quelle oder „gemessen an
   … gesetzt durch das Modell hindurch“).
3. **Ein Ton ist eine Funktion** der Kurven an der Taste, der Anschlagstufe und der Spielweise → Abtastwerte (Mono oder
   Stereo am Hörort des Körpers). Gerechnet wird **vorab**, nicht in Echtzeit; das Ergebnis liegt im Tonspeicher.
4. **Modal:** je Teilton Moden (komplexe Pole) mit Anteil vom Anreger zum Abnehmer; gekoppelte Systeme (Chor, zwei
   Schwingungsebenen) über einen kleinen Eigenlöser. Nichtlineares (Hammerkontakt, Längskraft) im Zeitbereich daneben.
5. **Körper:** Moden einer orthotropen Platte mit Rippen (Netz, Eigenlöser), Mobilität am Stegpunkt, Abstrahlung;
   im Ton als Faltung bzw. Antwort auf einem Raster.
6. **Datei:** JSON (`.lvinst`, Fassung je Änderung), Kurven immer geschrieben, eine fehlende Kurve bekommt die Vorgabe.

## Messkette (gegen freie Aufnahmen)

- Teiltonmessung ohne Qt: Einsatz, f0 und Inharmonizität, Hüllkurven je Teilton, zwei Abklingstufen (Schroeder),
  Schwebung und Tiefe der Einbrüche, Helligkeit über die Stärke, Phantomteiltöne, Pegel und Terzbänder.
- Dieselbe Messung an Aufnahme und Modell; **Setzen durch das Modell hindurch** (je Größe die Messgröße, die sie
  verursacht; halber Schritt; Toleranzen als Zahlen); ein dauerhafter Test prüft die Vorgabe gegen die Messung.
  **Stand S157 (nachgetragen 2026-10-27, [VORBILD_FLUEGEL.md](VORBILD_FLUEGEL.md) L1–L4, L7):**
  - gesetzt wird **über die Stützpunkte der Kurven, nicht je Taste** — ein Ausgleich (Levenberg–Marquardt) über alle
    Noten des Bezugs zugleich, je Note ihr Anteil als Ausreißer-Anzeige (SP2 a); der halbe Schritt je Taste war das
    Verfahren von 31d-4 und hat die zweite Stufe nicht erreicht;
  - die **Toleranz je Messgröße ist die Streuung des Bezugs** (Nachbarstufen, Kanäle — die größere), gemessen und
    festgelegt, bevor gesetzt wird (ZF1 a, ZF4 a);
  - **eindeutig** ist eine Suche, wenn zwei Startpunkte an jeder Messgröße innerhalb der Toleranz enden; je Messgröße
    ein gültiger Bereich, außerhalb wird gesetzt, nicht gesucht (SP3 a);
  - **Erkennung nur am Bezug:** wo die Messung eine Struktur erkennt (zweite Stufe, Schwebung), wird das Modell an der
    Struktur des Bezugs gemessen, nicht mit eigener Erkennung (ZF3 a);
  - vor dem ersten Lauf: Rechenzeit je Auswertung, Zahl der Auswertungen, Dauer des Laufs, Ergebnisdatei je Note bzw.
    Iteration (SE1 a);
  - das Setzen ist **Feinabstimmung als eigene, letzte Stufe** nach den Grundlagen (RF1 a, nachgetragen 2026-10-28);
    bis dahin trägt das Instrument Vorgaben aus Quellen und nach Gehör, die Messung dient als Prüfstein.

## Technische Rahmen (aus IK1)

- Rechen- und Messcode: reines C++ (C++20), **ohne Qt und ohne `windows.h`**, nur gegen eigene Schnittstellen
  (`Fft`, `Eigenloeser`); fremde Bibliotheken hinter eigener Schnittstelle, austauschbar, mit Bezugswerten getestet.
- Tests ohne Qt-Anwendung (doctest); Bezugsdateien je Compiler.
- Rechenzeit als Grenze je Klasse (Flügel: höchstens 4,6 % der Echtzeit je Ton im Bass); teure Verfahren tragen die
  Marke **Deluxe** (aus dem Modell abgeleitet, UF16 a).

## Was je Klasse für die Schnittstelle zu klären ist (in `offene-fragen.md`)

- Welche Größen tragen Kurven über den Tonumfang, welche sind fest, welche hängen an der Stärke? Wie viele
  Stützpunkte verlangt die Physik je Kurve — die Zahl der Parameter beim Setzen (L2; Vorschlag Z2)?
- Was ist die Spielweise (Anschlag, Bogendruck und -geschwindigkeit, Anblasdruck …) und wie wird sie als Spur geführt?
- Braucht die Klasse einen Körper (gemeinsam mit 0) oder eine eigene Abstrahlung (Schallstück, Rohrende)?
- Ist die Rechnung vorab möglich, oder hängt der Ton von Steuerungen über die Zeit ab (Bogen, Atem), sodass er pro
  Note mit ihrer Spur gerechnet werden muss?
