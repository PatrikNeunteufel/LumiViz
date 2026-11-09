# Übergabe an LumiViz: der Hörtest aus InstrumentsCalculation

> **Von:** IC Session 6 (`IC_Session6_2026-11-09_aufloesung-uebergabe`), 2026-11-09. **An:** LumiViz (LV).
> **Anlass:** IC wird aufgelöst (Entscheid Patrik 2026-11-09, LumiViz S182, LP2 b; Vermerk oben in `CLAUDE.md`). Die
> Integration der IC-Hörlisten in den Hörtest führt IC nicht mehr weiter; **LV übernimmt** die 11 Listen und alle
> offenen Punkte dazu. Umzug der Dateien nach dem LV-Plan
> `LumiViz/projects/apps/LumiViz/docs/modellierung/anbindung-ic/Aufloesung_IC_Plan.md` (U6, AU3) — in IC wurde nichts
> verschoben oder gelöscht.
> **Sync:** `cmake/sync_hoertest` (Leitung HT). IC tritt aus; die Übergabe steht dort als Nachricht IC → LV, cc HT.
>
> **Belegkraft** je Angabe in eckigen Klammern: **[gelesen]** = am 2026-11-09 in der genannten Datei nachgelesen;
> **[nachgeprüft]** = am 2026-11-09 lesend nachgerechnet (Befehl genannt); **[Nachricht]** = steht in einer
> Sync-Nachricht, hier nicht nachgemessen; **[Vermutung]** = Deutung ohne Prüfung.

## 1 Was in IC zum Hörtest liegt

| Teil | Ort | Inhalt | Beleg |
|---|---|---|---|
| Werkzeug `ic_hoerproben` | `rechnung/werkzeuge/hoerproben.cpp` (Aufruf `:8`, `main` `:655`), Ziel in `rechnung/CMakeLists.txt:101` | je Klasse die Hörproben der Vorgaben (drei Noten über den Umfang, drei Stärken, Spielweisen der Klasse) in eine `hoer::Sammlung` | [gelesen] |
| Ablage nach Konzept 0.3.0 §11/§12 | `rechnung/werkzeuge/Hoerproben.hpp` (Typen `Klang`, `Sammlung`, `achsenAus`), `rechnung/werkzeuge/Sammlung.cpp` | `liste.json`, `klaenge/<id>.wav`, `klaenge.sha256`; Achsen aus dem Namen (`instrument`, `ton`, `staerke` = `a<dB>` oder `e<Stärke×100>`, `spielweise`); `mitschwingen`/`ohne` → Punkt `paar`, sonst `einzel` mit `frage`; nicht endliche Töne ausgelassen und gemeldet (`Sammlung.cpp:142–148`) | [gelesen] |
| Lautheit | `rechnung/gemeinsam/Werkzeug.cpp:263` `lautheit` (Kopf `Werkzeug.hpp:51`) | BS.1770-4: K-Filter, Blöcke 400 ms / 75 %, Gate −70 LUFS absolut, −10 LU relativ; **unter 0,4 s** Mittel über den ganzen Klang ohne Gate (`:278–283`); ganz still → −300 (`:302`) | [gelesen] |
| Tests | `rechnung/gemeinsam/test_gemeinsam.cpp:135` (Tech 3341 Fälle 1–4, auf Mono umgerechnet); `rechnung/werkzeuge/test_sammlung.cpp:42`, `:75` | letzter voller Lauf MSVC Release 2026-10-31 23:13–23:16: **125 Fälle, 1503 Prüfungen, alle grün** (`rechnung/out/Testing/Temporary/LastTest.log`, Zeilen 123–129); seither keine Quelle unter `rechnung/` geändert | [gelesen], [nachgeprüft] (`find -newer LastTest.log`) |
| 11 Listen | `hoertest/listen/01_hammer_saite` … `11_tonrad`, je `liste.json` und `klaenge.sha256` | **versioniert ab dem Commit von U0** (heute uncommittet); `gruppe_nach: ["instrument"]`, `kontrolle: "moderat"`, `katalog: null`, `toleranz_lu: 1` | [gelesen] |
| Klänge | `hoertest/listen/*/klaenge/*.wav` | **592 WAVs, 237 MB, nicht in Git** (`.gitignore`: `hoertest/**/klaenge/`, `hoertest/beispiele/`) | [nachgeprüft] (`find … -name '*.wav' | wc -l`, `du -sh hoertest`) |
| Hook | `.claude/settings.local.json` | `sync_check -Side IC` — lokal, nicht versioniert; entfällt mit dem Austritt | [Journal 2026-10-30] |

Klänge und Pegel je Liste [nachgeprüft: Dateien in `klaenge/` gezählt, `pegel.lufs` aus `liste.json`]:

| Liste | Klänge | Pegel (LUFS) | | Liste | Klänge | Pegel (LUFS) |
|---|---:|---:|---|---|---:|---:|
| 01 Hammer-Saite | 70 | −33,3 | | 07 Lippe und Rohr | 39 | −28,2 |
| 02 Gezupfte Saite | 56 | −25,1 | | 08 Rohrblatt und Rohr | 56 | −23,0 |
| 03 Bogen-Saite | 76 | −23,1 | | 09 Luftstrahl und Rohr | 33 | −23,0 |
| 04 Platte und Stab | 108 | −29,4 | | 10 Zunge | 49 | −26,8 |
| 05 Membran | 53 | −30,8 | | 11 Tonrad | 8 | −23,0 |
| 06 Becken und Gong | 44 | −23,1 | | **zusammen** | **592** | |

## 2 Wie die Klänge entstanden sind

- **Befehl:** `ic_hoerproben hoertest` aus dem IC-Wurzelordner, Bau MSVC 2022 Release unter `rechnung/out`
  (`Release/ic_hoerproben.exe`, gebaut 2026-10-31 23:03; Listen geschrieben 23:12). Der Befehl steht im Kopf jeder
  `klaenge.sha256` („Geschrieben mit: ic_hoerproben hoertest“). Gerechnet wurde auf dem Arbeitsbaum nach `bb7e6d6`
  (Session 5, uncommittet). [gelesen: Dateizeiten, Kopf `klaenge.sha256`]
- **Format:** mono, 48 kHz, 32 Bit Gleitkomma (`Hoerproben.hpp:6`, `rate = 48000` `:32`). [gelesen]
- **Pegel:** je Klang die Lautheit nach BS.1770-4 mit Gating vor dem Angleichen (`lufs_vorher` als Messwert in der
  Liste), dann **ein gemeinsamer Pegel je Liste**: −23 LUFS, oder so viel tiefer, dass kein Klang über die Spitze 0,99
  kommt, auf 0,1 LU abgerundet (`Sammlung.cpp:28–29`, `zielPegel` `:165–173`); jeder Klang wird mit einem Faktor auf
  diesen Pegel gebracht — **außer** Klänge mit Lautheit −300 (ganz still), die bleiben unverändert (`:183`). [gelesen]
- **Abweichung von der Vorgabe:** „gleich laut auf −23 LUFS“ (Kopf `hoerproben.cpp:10`, AUFTRAG_BAU §4 Punkt 4) wäre
  bei 92 Klängen über 0,99 gegangen; deshalb der Pegel je Liste. Von HT als vereinbar mit §8.1/§12 bestätigt.
  [Journal 2026-10-31; Nachricht `HT-20261101-0845-…`]
- **Neu erzeugen** (in IC nur noch lesen, deshalb nur zur Kenntnis): `cmake -S rechnung -B rechnung/out`,
  `cmake --build rechnung/out --config Release --target ic_hoerproben`, dann `rechnung/out/Release/ic_hoerproben.exe
  hoertest [Klasse …]`; Laufzeit der 592 Klänge 547 s (Journal 2026-10-31). Der Abdruck je Klang hängt an der
  Plattform: Windows-MSVC und Windows-clang sind bitgleich, Linux weicht in 29 von 42 Abdrücken ab
  (`pruefungen/LV-2026-11-02-clang-windows-messung.md`). [gelesen]

## 3 Was offen ist

### 3.1 Befund HT 1 — gleicher Abdruck bei verschiedener Stärke

Quelle: `sync_hoertest/inbox/HT-20261101-0845-s2-gebaut-liste-pruefen-11-ic-listen-gueltig.md` Punkt 1 (HT hat S2
gebaut; `Hoertest.exe --liste-pruefen`: **alle 11 Listen gültig, 0 Fehler**, `klaenge.sha256` stimmt). [Nachricht]

**[nachgeprüft]** an den `klaenge.sha256` (gleiche SHA-256 je Liste gesucht) — dieselbe Menge wie bei HT:

| Liste | gleiche Bytes |
|---|---|
| 01 | `Upright_A1_*` = `Honkytonk_A1_*` je Stärke (a0, a-12, a-30) — zwei Instrumente, derselbe Ton |
| 02 | Harfe Dis4, Dis7; Gitarre (Deluxe) Fis2, Cis4, G5; Laute A2, Cis4; Oud Ais3, Cis5 — je `e0` = `e100` |
| 07 | Alphorn A3 `e0` = `e100` |
| 08 | Sopransaxophon H3 und Altsaxophon E3 `e0` = `e50` = `e100`; Tenorsaxophon H2 `e0` = `e50` |
| 09 | Shakuhachi D5, Ais5 `e0` = `e50` = `e100` |

Folge: nach dem Angleichen bleibt von der Stärke nichts; die Punkte fragen zweimal dasselbe. **[Vermutung]** In diesen
Tönen wirkt die Stärke nur als Faktor auf die Amplitude (oder gar nicht — Saxophone mehrstabil, Shakuhachi 6/25 nach
`STAND.md`), der Pegel nimmt ihn heraus; bei Upright und Honky-tonk unterscheiden sich die beiden Vorgaben an A1
offenbar in nichts, was den Ton ändert. Ursache am Modell nicht untersucht.

### 3.2 Befund HT 2 — Klänge abseits des Pegels der Liste

Quelle: dieselbe Nachricht, Punkt 2 (Messung von HT mit `--liste-pruefen`, hier nicht nachgemessen). [Nachricht]

- Über ±1 LU: 01 Upright/Honky-tonk C4 `moderator` −8,4/−8,8 LU, Cimbalom Dis4 a-30 −7,9, Flügel Port C6 a-12 −1,2,
  Hackbrett Ais5 a-30 −1,3; 04 Vibraphon D3 a-30 −2,9 und fünf um −1,2 bis −1,7; 05 Bassdrum e0 −5,8 und fünf um
  −1,2 bis −1,7; 07 Trompete Gis4 e0 −1,2.
- Nicht messbar (unter 0,4 s oder alles unter −70 LUFS): 04 Marimba Ais6, Xylophon D6/Ais7 (je alle drei Stärken,
  0,31–0,33 s) und 8 weitere a-30/a-12; 01 Flügel Port C6 a-30; 07 Trompete A3 e0 (Spitze −70,5 dBFS) und Fis5 e0.

Was der IC-Code dazu sagt **[gelesen]**:
- **Unter 0,4 s** misst `lautheit` das Mittel über den ganzen Klang ohne Gate (`Werkzeug.cpp:278–283`), HT dagegen
  hat dann keinen Block und meldet „nicht messbar“ — die beiden Messungen sind für kurze Klänge **verschieden
  festgelegt**. [nachgeprüft: `Marimba_Ais6_a0.wav` 63 296 Byte ≈ 0,33 s bei 48 kHz, 32 Bit, mono]
- **Ganz still** (Lautheit −300) wird nicht angeglichen (`Sammlung.cpp:183`) — Trompete A3 e0 ist dann ein Befund am
  Modell (Ton spricht bei e0 nicht an), nicht an der Liste. [Vermutung für die Ursache]
- Für die Abweichungen **über 1 LU bei messbaren Klängen** (Moderator −8,4 LU, Bassdrum −5,8 LU) findet sich im
  IC-Code keine Erklärung: der Faktor ist linear, gemessen und angeglichen wird derselbe Ton. Beide Messungen an
  derselben Datei vergleichen (IC `lautheit` gegen HT) ist der erste Schritt. [Vermutung: Unterschied in Blockgrenze
  oder relativem Gate bei Klängen mit langem leisem Ausklang]

### 3.3 IC-3 — Bezugsaufnahmen auf 48 kHz

Zugesagt in der IC-Antwort zu HT-1 (`sync_hoertest/inbox/IC-20261030-2247-ht-1-antwort-ic-…md` Punkt 3): Bezüge aus
`IC_SAMPLES` (etwa Iowa mit 44,1 kHz) rechnet das liefernde Projekt beim Schreiben auf 48 kHz um, das Werkzeug
rechnet nicht um (Konzept §7.5: eine Abtastrate je Liste). **Nicht gebaut**; in `STATUS.md` IC-3 „später“. Gebraucht
erst für Paare Modell gegen Aufnahme (Setzen nach Plan A–D). Die Aufnahmen liegen auf der Wechselplatte — vor Gebrauch
bei Patrik anfragen (`CLAUDE.md`). [gelesen]

### 3.4 L5 — die Listen sind ungehört

Lehre L5 („Hören vor Rechnen: das Hörurteil ist ein Kipppunkt mit Datum“, `LEHREN_SETZEN_FLUEGEL.md:84`;
`VORBILD_FLUEGEL.md:72`): eine Vorgabe gilt erst als nicht brauchbar, wenn sie gehört wurde. **Keiner der 592 Klänge ist
gehört.** HörTest_Helper liest Listen seit S2 (Nachricht HT 2026-11-01). [gelesen]

### 3.5 Alphorn A2 wird NaN

`klassen/Klasse 7 - Lippe und Rohr/offen.md:14`: der Ton wird ab Wert 2567 (53 ms) NaN, davor still, bei e0, e50 und
e100; A3 und A4 rechnen. Die Sammlung lässt die drei Töne aus und meldet sie auf stderr (Liste 07 hat deshalb kein `Alphorn_A2_*`).
[nachgeprüft: `grep Alphorn_A2 hoertest/listen/07_lippe_rohr/liste.json` leer] Ursache am
Modell nicht gesucht (Anblasschwelle, Stimmung der tiefen Lage?). [gelesen]

### 3.6 Zur Kenntnis: was sich mit dem Umzug überschneidet

- **Pfade der Klänge gegen U6/AU3.** Konzept 0.3.1 §12: „Pfade (`datei`, …) gelten relativ zur `liste.json`“, alle
  IC-Listen nennen `klaenge/<id>.wav`; §11: `klaenge/` liegt neben der Liste und wird ignoriert. Der Plan legt die WAVs
  nach `cmake\samples\IC_Hoerproben\<klasse>\`, die Listen nach `hoertest/listen/ic/<klasse>/` und prüft mit
  `--liste-pruefen` — **so getrennt findet die Prüfung die Klänge nicht.** [gelesen: Konzept §11/§12,
  `Aufloesung_IC_Plan.md` §3 und U6]
- **Neu schreiben geht nach dem Umzug nicht mehr:** HT8 a (Konzept §11) setzt voraus, dass das Werkzeug des Projekts
  die Klänge aus dem Code eines Commits neu schreibt; `rechnung/` geht nach `harvest/…` „nicht gebaut“ (Plan §3, AU2 a).
  Ohne Bau sind die 592 WAVs die einzige Fassung — deshalb hat AU3 sie archiviert. [gelesen]
- **Klassen 1, 4, 5 hat LV schon selbst gelistet:** `LumiViz/hoertest/listen/klasse1_34i` (Upright, Honky-tonk, CP-70,
  Clavinet; 2026-11-09), `stab_34g` (2026-11-08), `membran_34h` (2026-11-08) — aus den übernommenen Kernen, nach den
  Commit-Texten `6318c703` und `90d3fcfa` mit IC-Abdrücken Bit für Bit gleich. Nicht abgedeckt davon in IC 01: Flügel
  Port, Hackbrett, Cimbalom. [gelesen: Ordner und `git log`; Bitgleichheit nur aus dem Commit-Text]

### 3.7 Sonst offen aus IC (nicht Hörtest)

Aus der Prüfung durch LV S166 (`pruefungen/LV-2026-10-30-erster-baudurchgang.md`): erledigt IC-A2, A3, A4 (Session 5)
und Schritt 1 von IC-A9 und IC-A10 (LV S172, `pruefungen/LV-2026-11-02-clang-windows-messung.md`); **offen** die
Vorlagen zu IC-A1, A5–A7, A10 (Schritte 2/3), A11, A12, A14–A17, ohne Entscheid A8, A9 (Schritt 2), A13, A18
(`STAND.md`, Journal 2026-10-31). Nichts davon ist halb gebaut: der Code steht auf dem letzten grünen Lauf. [gelesen]

## 4 Anweisung an LumiViz — was zum Abschluss des Hörtests zu tun ist

Reihenfolge mit Begründung. Leitlinie: **erst die Listen richtig, dann hören** — ein Hörurteil an einer Liste, die
zweimal denselben Klang oder ungleich laute Klänge vorlegt, ist verlorene Hörzeit und lenkt das Urteil (lautere Klänge
wirken voller); es nachträglich auszusortieren kostet mehr als die Prüfung vorher.

1. **U0 abwarten: Patrik committet den offenen Stand von IC.** *Warum zuerst:* `klaenge.sha256` bindet jede Antwort an
   die gehörten Klänge (§11); die Listen und der Code, der sie schrieb, müssen in einem Commit stehen, sonst ist kein
   Hörurteil auf einen Stand zurückführbar. Danach den Commit in jede übernommene Liste bzw. ihre Begleitdatei
   eintragen (der Befehl steht schon im Kopf von `klaenge.sha256`, der Commit nicht).
2. **Über die Ablage der Klänge entscheiden, bevor etwas umzieht (AU3 gegen §11/§12, s. 3.6).** *Warum vor U6:*
   Mit dem Umzug nach `samples\IC_Hoerproben\` ohne Anpassung schlägt die Prüfung von U6 selbst fehl. Möglich sind
   etwa: die WAVs neben der Liste unter `hoertest/listen/ic/<klasse>/klaenge/` (ignoriert, wie §11 vorsieht) und
   zusätzlich als Archiv auf der Wechselplatte; oder ein Ablageort über die Projektdatei `hoertest.projekt.json`
   (HT-3, bei HT offen) — das ist eine Formatfrage an HT, nicht von LV allein zu lösen. Vorlage an Patrik.
3. **Je Klasse festlegen, welche Liste gilt.** *Warum:* LumiViz ist die einzige Quelle (LP2 b). Für die Klassen 1
   (ohne Flügel Port, Hackbrett, Cimbalom), 4 und 5 gibt es LV-Listen aus dem eigenen Kern; gehört werden sollte die
   Fassung, die LV ausliefert. Die IC-Listen dieser Klassen nur übernehmen, was die LV-Listen nicht haben. Für 2, 3,
   6–11 sind die IC-Listen die einzigen, bis LV die Klasse portiert (34j/34k planen 10 und 2).
4. **Befund HT 2 klären (Pegel).** Zuerst die Messung angleichen: IC `lautheit` und HT an denselben Dateien
   vergleichen (Moderator, Bassdrum e0, Vibraphon D3 a-30); für Klänge unter 0,4 s eine gemeinsame Regel mit HT
   festlegen (heute misst IC ohne Gate, HT gar nicht — 3.2). *Warum vor Befund 1:* erst wenn beide Seiten gleich
   messen, ist sicher, welche Klänge wirklich ungleich laut sind; manche „gleichen Abdrücke“ könnten sonst nur am
   Pegel liegen. Stille Klänge (Trompete A3 e0) als Befund am Modell führen, nicht angleichen.
5. **Befund HT 1 klären (Stärke ohne Wirkung).** Je Instrument entscheiden: Punkte mit gleichem Abdruck zu einem
   zusammenlegen oder streichen, und die Wirkungslosigkeit der Stärke als Befund am Modell in dessen offene Punkte.
   *Warum nicht einfach lassen:* der Prüfer würde die Stärke beurteilen, die es dort nicht gibt.
6. **Neu schreiben und prüfen:** jede geänderte Liste aus dem Code des LV-Kerns (bzw. bei unportierten Klassen nach
   Entscheid aus 2) neu schreiben, `Hoertest.exe --liste-pruefen <liste.json>` je Liste mit 0 Fehlern und ohne
   Hinweis auf Pegel oder gleiche Abdrücke (Exe: HörTest_Helper
   `out/build/windows-ninja-release-clang/exec/Hoertest/bin/Release/`, Nachricht HT 2026-11-01).
7. **Hören (L5).** Patrik hört mit `Hoertest.exe` (S2: Einzel, Paar, gleiche Paare, Antworten, Fortsetzen); Urteil mit
   Datum je Instrument, Antworten nach §13 unter `hoertest/ergebnisse/<liste>/`. *Warum erst jetzt:* L5 macht das
   Urteil zum Kipppunkt — es soll an der richtigen Liste fallen.
8. **Alphorn A2** beim Port der Klasse 7 am Modell untersuchen; bis dahin bleibt der Ton aus der Liste (vermerkt).
   *Warum spät:* betrifft einen Ton, die Klasse hat in LV noch keine Stufe.
9. **IC-3 (Bezüge auf 48 kHz)** erst mit den Paaren Modell gegen Aufnahme bauen, also mit dem Setzen nach Plan A–D;
   Aufnahmen vorher bei Patrik anfragen (Wechselplatte). *Warum zuletzt:* die heutigen Listen haben keine Bezüge.

Im Sync: LV ist dort Teilnehmer und übernimmt die IC-Hälfte (IC-1, IC-2 erledigt; IC-3 offen) in die eigene; IC ist
ab 2026-11-09 ausgetreten.
