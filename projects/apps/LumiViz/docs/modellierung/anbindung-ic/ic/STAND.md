# Stand — InstrumentsCalculation

> **Stand 2026-11-09 – aufgelöst, übergeben an LumiViz** (IC Session 6, `IC_Session6_2026-11-09_aufloesung-uebergabe`;
> Verlauf im Journal `.claude/journal/2026-10-30.md`, `2026-10-31.md`, `2026-11-09.md`):
> - **IC wird aufgelöst** (Entscheid Patrik 2026-11-09, LumiViz S182, LP2 b; Vermerk oben in `CLAUDE.md`). Hier wird
>   nichts mehr gebaut, gerechnet oder entschieden; den Umzug macht LumiViz nach
>   `LumiViz/projects/apps/LumiViz/docs/modellierung/anbindung-ic/Aufloesung_IC_Plan.md` (U0–U7, offen AU1–AU3).
> - **Arbeitsbaum:** IC Session 5 (Hörschwellen, Probe, Glätte des Ziels, IC-1/IC-2 mit 11 Hörlisten, Messpläne,
>   Journal), die Prüfberichte unter `pruefungen/` (LV S166, LV S172) und der Vermerk sind fertig, aber **uncommittet**
>   (U0, Patrik). Code unverändert seit dem letzten vollen Lauf MSVC Release 2026-10-31: 125 Fälle, 1503 Prüfungen grün.
> - **Hörtest:** an LumiViz übergeben — [UEBERGABE_LUMIVIZ_HOERTEST.md](UEBERGABE_LUMIVIZ_HOERTEST.md) (Bestand, Entstehung
>   der Klänge, Offenes, Anweisung in Reihenfolge). Im Sync `cmake/sync_hoertest` ist IC ausgetreten.
> - **Offen, alles bei LumiViz:** Befunde 1 und 2 von HT zu den Listen (gleiche Abdrücke je Stärke, Klänge abseits des
>   Pegels), IC-3 (Bezüge auf 48 kHz), L5 (592 Klänge ungehört), Alphorn A2 NaN; aus der Prüfung S166 die Vorlagen zu
>   IC-A1, A5–A7, A10 (Schritte 2/3), A11, A12, A14–A17 und ohne Entscheid A8, A9 (Schritt 2), A13, A18.

> **Stand 2026-10-30, IC Session 4 (Cloud, Bau der Rechenkerne nach [AUFTRAG_BAU.md](AUFTRAG_BAU.md); Zweig
> `claude/youthful-ritchie-0iorxi`, Git-Ausnahme §8 von Patrik beim Start bestätigt) — **Sitzung abgeschlossen**; Bilanz der Sitzung (Zusammenfassung für
> Merge und lokale Planung: [BERICHT_BAU.md](BERICHT_BAU.md)):**
> - **Gebaut (Grundlagen) — alle Klassen 1–11, je mit Kern, Vorgaben mit Herkunft, Tests, Abdruck gcc = clang,
>   Hörproben, `modell.md`, `offen.md`, README, KLASSEN-Zeile mit zwei Zahlen, FRAGEN §3:**
>
>   | Klasse | Kern | spielbar (ungemessen) | Prüfsteine | Rechenzeit |
>   |---|---|---|---|---|
>   | 1 Hammer-Saite | Flügel portiert (Prüfstein), Saitenkern | Upright, Honky-tonk, CP-70, Clavinet, Hackbrett, Cimbalom | Flügel gegen Bezug 10⁻⁶ dB | ≤ 4,6 % |
>   | 2 Gezupfte Saite | Waveguide in 2 Polarisationen, 2×2-Admittanz | Cembalo, Harfe, Gitarre Deluxe, Laute, Oud (Koto zurückgestellt); **Mitschwingen je Clip** (K2 F-3 a) | zweistufiges Abklingen (Bank & K.), Zupfpunkt (Välimäki) | 0,3–0,5 % |
>   | 3 Bogen-Saite | Waveguide mit Reibstelle, Korpus am Steg | Cello 48/48, Violine 42/42, Viola 40/42, Kontrabass 42/42 (F-B3 a), Ensemble | **Wolfton entsteht**, Einschwingen 25–55 ms (Guettler) | 1,9 % |
>   | 4 Platte und Stab | FE-Balken, Resonator, Tonabnehmer | zehn Instrumente; **Vibraphon gestrichen** (T1 a, oben 18/18) | Rucz, Falaize & Hélie | 0,2–3,6 % |
>   | 5 Membran | Fellmoden mit Luftlast (Rayleigh) und Kessel | Tom, Pauke, Bassdrum, Snare, Tabla, Conga, Bongo | **Pauke gegen Packer auf 3 %**, Tabla harmonisch | 3,1–3,4 % |
>   | 6 Becken und Gong | von Kármán modal, SAV | Tamtam, Gong, Crash, China, Splash, Ride, Hi-Hat (flach) | Energie 10⁻¹³, Aufblühen bei ff, Leissa | vorab 1–7 × |
>   | 7 Lippe und Rohr | Rohrschleife (Zug, Schmettern) | Posaune (jede Note), Alphorn, **Trompete 29/30** (F-B4 a, Ventile); **Wah-Wah** am Harmon (T3b a) | Schmettern-Richtung (Myers) | 2,4 % |
>   | 8 Rohrblatt | Rohr modal mit Tonlochgitter | Klarinette 32/33, Sopran-/Tenorsax, **Altsax 21/33** (F-B4 a) | Chalumeau, Almeida, Li | ≈ 5 % |
>   | 9 Luftstrahl | Strahl mit Laufzeit, tanh | Labialpfeife (alle), Blockflöte 20/25, Shakuhachi 6/25 | Fletcher & Douglas | 0,6 % |
>   | 10 Zunge | Zunge mit träger Strömung (Puranik) | Akkordeon 42/42, Harmonium, Melodica, Mundharmonika | Musette 2,00 Hz (Hergert) | 2,4 % |
>   | 11 Tonrad | Orgel über den Clip | organ, house_organ | Räder ±0,02 Cent, Rückfaltung, Scanner | 2,5–2,9 % |
>
> - **Nach dem Programm (entschieden, ohne Aufnahmen baubar):** Bogen × Stab (T1 a; Reibung nach `gemeinsam/Reibung`,
>   tief mit Resonator +100 Cent — Befund, κ/κ_c offen), Wah-Wah (T3b a; Restfläche zu offen), Mitschwingen je Clip
>   (K2 F-3 a; Harfe 47 Saiten 26 % Echtzeit je Clip). **F-B5 a:** Hackbrett/Cimbalom mitschwingend je Clip (C4 +1,1 dB
>   am Grundton nach 2,5 s; Cimbalom nur mit Pedal), Pedale der Harfe (Diagramm, Vorschlag, Spur, Glissando,
>   Bisbigliando); Clavinet: Wahl der Tonabnehmer und Verstärker-Shelves nach Gabrielli 2013 (Patrik lokal); Kennlinie (Einheit nicht genannt) und Klangschalter (Normierung unklar) offen; Suchrunde nach Bohrungen
>   (mit erweiterten Freigaben): Didgeridoo-Bohrung gefunden (Causse et al., Abb. 2), spielt aber mit der Lippe der
>   Posaune keinen Bordun (Befund, nicht spielbar); **Oboe-Bohrung** aus Patriks Download (Mamou-Mani et al. Abb. 11),
>   mit dem Blatt der Klarinette nur 7/36 (Befund, nicht spielbar — Doppelrohrblatt offen, Almeida et al. auf HAL:
>   Schliessdruck um 6 kPa, Dynamik fehlt); Fagott (keine digitalen Daten), Horn (Edinburgh überlastet), Tuba,
>   Querflöte (Strahl und Mund fehlen) ohne Bau.
> - **Messkette** (B1–B8, an synthetischen Signalen) und **Setz-Werkzeug** nach L8–L15 (an synthetischen Aufgaben:
>   Werte auf 1 % wieder, wirkungslos/verrauscht erkannt, wellig 1D, Wiederaufnahme) — der Lauf an Aufnahmen bleibt lokal.
> - **Tests:** 114 Testfälle, 1363 Prüfungen grün unter gcc 13.3 und clang 18.1; Abdrücke je Compiler in
>   `rechnung/werte/` (gcc = clang in allen Klassen). Rechenzeit relativ zum portierten Flügel (Grenze hier 5,5–7 %).
> - **Offen (je Klasse `offen.md`), die größten Befunde:** Saxophone mehrstabil (Kipppunkt Lippe); Shakuhachi 6/25 (Mund
>   offen); Becken als flache Platte bis 1,2–3 kHz (Form F-5 a lokal); Korpusse der Gitarre/Violine ohne Stärken (Fit an
>   Mores/CNSM lokal); Bend der Zunge gekappt (−4 Cent); Teppich der Snare ohne Drahtmoden. (Mitschwingen je Clip ist seit
>   2026-10-30 gebaut, s. oben.)
> - **Fragen an Patrik** (FRAGEN_KONSOLIDIERT §5): **F-B1, F-B2, F-B3 — alle a** (Patrik 2026-10-30). F-B3 a gebaut
>   (Viola, Kontrabass aus Händlerdaten, ungemessen). **F-B1 a und F-B2 a ausgeführt** (nach Freigabe der Hosts): auf
>   HAL und arXiv **keine Bohrung in Zahlen** — HAL-05529764 (Trompete, CC BY 4.0, abgelegt) zeigt nur eine Skizze, der
>   Tuba-Aufsatz keine Tuba-Bohrung; Acta Acustica selbst steht hinter einer Bot-Prüfung. Die neun Bläser bleiben
>   Gerüst; **F-B4 a** (Patrik lädt lokal): **Trompete** nach Soares et al. 2025 (Ventile, Griff am Modell) und
>   **Altsaxophon** nach Petersen 2020/Ukshini 2025 gebaut; Tuba (Myers nicht mehr abrufbar), Horn, Didgeridoo, Oboe,
>   Fagott, Querflöte, Altflöte bleiben Gerüst — weitere Suche als Auftrag.
> - **2026-10-30 nachmittags:** Hendrie 2007 (Patriks Download) gelesen — **kein ganzes Horn, keine Tuba** (nur ein
>   Prüfkörper mit Bessel-Becher und Naturhorn-Bögen); Horn und Tuba bleiben Gerüst. **L16–L18** aus LumiViz
>   (`LEHREN_SETZEN_FLUEGEL.md` §6 auf master) nach **F-B6 a** (Patrik bestätigt) gebaut: Setz-Werkzeug staffelt nicht, wenn zwei freie
>   Parameter dieselbe Wahrnehmung tragen (`traegt`), `welligkeit` prüft die Glätte des Ziels, Bericht = „Vorschlag“;
>   `gemeinsam/messkette.md` §3b; `gemeinsam/hoerschwellen.md` angelegt (T9 a, Zahlen aus L17, nur Bericht); alle elf `messplan.md` mit „Feinabstimmung und Ausstieg (L15–L18)“.
> - **Abschlussbericht:** [BERICHT_BAU.md](BERICHT_BAU.md) — Zusammenfassung für den Merge und die lokale Planung.
> - **Umgebung:** alles da (cmake, gcc, clang, python3, pdftotext); nicht möglich: Aufnahmen/Messdaten (lokal), LumiViz
>   (Kopie genutzt); ein Neustart des Containers während einer Stimmung (danach neu erzeugt).
> - **Hörproben:** `ic_hoerproben <Ordner> [Klasse …]` — rund 450 WAVs über alle Klassen, **nicht gehört** (L5, lokal).
> - **Als Nächstes (lokal, Reihenfolge in [BERICHT_BAU.md](BERICHT_BAU.md) §6.2):** PR #1 mergen (ohne Konflikt geprüft),
>   lokal bauen und testen (auch Windows), Hörproben hören (L5), Bezüge laden (§5.1), Setzen nach Plan A–D je
>   `messplan.md`; Integration in LumiViz nach IK1 (Einteilung A–D in §4); weitere Suche nach Bohrungen (§6.1);
>   `dld/` auf master kann weg (CC-Dateien liegen unter `quellen/`).
> - Journal: `.claude/journal/2026-10-29.md`, `.claude/journal/2026-10-30.md` (J-1 a).

> **Stand 2026-10-29, IC Session 3 (lokal, vor der Cloud-Sitzung):** Lieferung aus LumiViz S163 geprüft (L8–L15 gegen
> `Spielweisen_Konzept.md`, Kopie bitgleich zu `d1c06309`, Spielweisen-Konzept erneuert, Probenberichte S162/S163
> ergänzt); **[FRAGEN_KONSOLIDIERT.md](FRAGEN_KONSOLIDIERT.md) erstellt** (AUFTRAG_BAU §2 lokal erledigt): 62 offene
> Fragen in 13 Bündeln, Entscheide der Klassen 1–3 als Vorbild; T8 a folgt K1 F-6 b (nichts zurückstellen, ungemessen
> kennzeichnen) statt den Klassenempfehlungen. Verweis in jeder `offene-fragen.md` der Klassen 4–11 und gemeinsam.
> `.claude/settings.json` (kein Co-Authored-By, kein Claude-Session-Trailer). **G-1 a (Patrik 2026-10-29):** die
> globalen Regeln und Werkzeuge liegen als wörtliche Kopie unter `gemeinsam/cloud-global/` (Stand `~/.claude` `3ee1f9b`
> plus lokale Änderung vom 2026-10-29; nicht pflegen), die Projekt-`CLAUDE.md` verweist für Cloud-Sitzungen darauf.
> **Beantwortet 2026-10-29:** alle Klassenfragen a, außer G F-D1 und K4 F-7 (laden zum Messen mit Vermerk, Repo
> bleibt privat; Laden selbst erst nach Freigabe von Datei und Größe) und K5 F-7 (nichts zurückstellen); K3 F-9-1 a;
> eingetragen je Bündel und in jeder `offene-fragen.md`. **Z-1 a:** auch in K6–K10 nichts zurückstellen. Dazu: die
> Cloud rechnet nur, Aufnahmen lädt nur eine lokale Sitzung; der Ort `samples\` kann auf eine externe Festplatte
> wandern, im Code deshalb nur über `IC_SAMPLES` (AUFTRAG_BAU §0, §8; CLAUDE.md). **Offen:** T9 (neuer Teil, gilt als a);
> das Laden für G F-D1 und K4 F-7 (lokal, nach Freigabe von Datei und Größe).
>
> **Stand 2026-10-29 (aus LumiViz S163, Patrik):** **Neuer Auftrag [AUFTRAG_BAU.md](AUFTRAG_BAU.md)
> — der Bau der Rechenkerne aller Klassen unter `rechnung/` in einer Cloud-Sitzung auf GitHub** (Patrik startet sie):
> zuerst alle offenen Fragen der Klassen 0 und 4–11 nach Themen gebündelt in `FRAGEN_KONSOLIDIERT.md` (nichts doppelt;
> Empfehlung = Vorgabe, Patrik antwortet danach im IC-Chat), dann Grundgerüst, Flügel-Kern als Prüfstein, Klassen in der
> Reihenfolge 4 → 1 → 7/8/9 → 3 → 5/6 → 11 → 10 → 2, Messkette, Setz-Werkzeug nach L8–L15. In der Cloud **kein Setzen an
> Aufnahmen** (liegen lokal). Dafür abgelegt: die Lehren **L8–L15** aus der Probe 7.5.11 (`LEHREN_SETZEN_FLUEGEL.md` §5,
> Kurzfassung in `VORBILD_FLUEGEL.md`) und die **Kopie des Flügel-Vorbilds** `gemeinsam/vorbild-lumiviz/` (Rechenkern,
> Eigenlöser, Messkette, Suche, Konzepte, Probenberichte S162/S163; Stand LumiViz `d1c06309`, in IC Session 3 bitgleich
> geprüft). Der Durchgang nach F-L3 a (Z2/Z3 je Klasse)
> steht weiter aus — die Bau-Sitzung holt ihn je Klasse beim Bau nach (`modell.md`, `offen.md`). **Stand in LumiViz:**
> die Suche über die Stützpunkte (SP2 a) ist gelaufen und die Probe 7.5.11 verfehlt; Patrik hat den Plan A–D als zweiten,
> vorläufig letzten Anlauf entschieden (V2 a) — für Klasse 1 gilt weiter: den LumiViz-Stand vor jeder Arbeit neu lesen.
>
> **Stand 2026-10-26, Ende IC_Session2_2026-10-26_klasse-10-zunge** (Session-Abschluss; kein Code, keine Tests — das
> Projekt führt keine Session-Reports, die Bilanz steht hier). Auftrag: [AUFGABE.md](AUFGABE.md). Übernahme nach
> LumiViz: [UEBERNAHME_LUMIVIZ.md](UEBERNAHME_LUMIVIZ.md).
>
> **Bilanz IC Session 2:**
> - **Klasse 10 Zunge** und **Klasse 2 Gezupfte Saite (Deluxe)** recherchiert — je alle sieben Lieferungen, Kernwerte
>   und Lizenzen selbst nachgelesen; damit sind **alle Klassen 0–11 vorab recherchiert**.
> - **Antworten Patrik eingearbeitet:** Klasse 2 F-1 bis F-10 (a; F-10 abgewandelt); Klasse 1 F-1 bis F-8 (F-6 b, F-4
>   über die Nachfrage F-4-1 a); Klasse 3 F-1 a, **F-2 b**, F-3 bis F-9 a (Downloads freigegeben). Offen: Klassen 0, 4–11.
> - **Ablage (A-1 a):** Aufsätze im Repo unter `quellen/`; **Audio und große Messdaten lokal** unter
>   `C:\Users\patri\source\repos\Visuals_Project\cmake\samples\<Klasse n - Name>\<Datensatz>\` (auch für
>   LumiViz); im Repo je Datensatz nur `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`. Klassenordner heißen jetzt
>   **„Klasse n - Name“**; Verweise nachgezogen (Agentenberichte unverändert). `.gitignore`, `AUFGABE.md`, `README.md`,
>   `CLAUDE.md`, `UEBERNAHME_LUMIVIZ.md` und alle Download-Fragen angepasst.
> - **Geladen:** Klasse 1 K18 Upright (CC BY 4.0) und VCSL-Uprights (CC0); Klasse 2 VCSL Harfen/Cembali (CC0),
>   Iowa-Nylongitarre mono 24/96, Mores-Archiv (CC BY 4.0) — alles jetzt unter `samples\`; Aufsätze Klasse 2 (Chadefaux
>   2013, Bader 2025) und Klasse 3 (Russo 2022, Maestre 2013) unter `quellen/`.
> - **Abgleich mit LumiViz:** S155 läuft am Flügel (Teilplan 7.5, SQ2 a: Verstimmung 0–6 Cent) — für das Honky-tonk
>   (Klasse 1 F-1 a) gilt diese Grenze nicht; Hinweis Patrik: bei Klasse 1 den LumiViz-Stand immer neu prüfen.
>
> **Plan für die nächste Session:**
> 1. ✅ **Erledigt IC Session 3 (2026-10-27)** außer den Saitenantworten — siehe Abschnitt Klasse 3.
>    **Klasse 3 F-9 a — Downloads nach `…\cmake\samples\Klasse 3 - Bogen-Saite\`** (freigegeben; Größen 2026-10-26
>    abgefragt, je mit `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv` im Repo unter `aufnahmen/` bzw. `daten/`):
>    - Iowa MIS 2012 Violine, Viola, Cello, Kontrabass, arco und pizz, **mono 24/96** als Einzeldateien (Seiten
>      `MISviolin2012.html` … `MISdoublebass2012.html`; zum Vergleich die Zips mono 24/96 zusammen 2 470 126 112 B).
>    - TinySOL (Zenodo 3685367, `TinySOL.tar.gz` 1 026 917 185 B) außerhalb laden, nur die Streicher auspacken.
>    - TU-Note Violin (DepositOnce, API `api-depositonce.tu-berlin.de/server/api/core/items/f81ba73c-4d9b-48de-9fbc-31cb03d5b9bc`;
>      `dc.rights.uri` „https://creativecommons.org/licenses/by-nd/4.0/“; `TU-Note_Violin.zip` 1 605 101 158 B, MD5
>      f924c0d0…; `TU-Note_Violin_Package_Insert.pdf` 215 920 B) — nur messen, nichts bearbeitet weitergeben.
>    - VSCO 2 CE `Strings/` ohne Harfe (Violin/Viola/Cello Section, Solo Violin, Solo Contrabass; 720 MB laut Baum).
>    - Lampis, Zenodo 13374477 `waveforms.zip` 3 007 564 144 B.
>    - CNSM, Zenodo 18696785 `dataset.zip` 654 025 212 B (Korpus der Violine, F-7 a).
>    - **Vorher fragen:** „ein Teil der Saitenantworten“ (Zenodo 17749111) — kleinster Teil ist eine `.7z` mit
>      14 083 546 513 B (TypeA); auf C sind nur rund 42 GB frei.
> 2. Patriks Antworten für die übrigen Klassen (0, 4–11) einarbeiten; die Downloads je Klasse nach A-1 a.
> 3. Patrik: die Audiodateien aus der Git-Historie nehmen (Commits `60bdc39`, `d965732`, rund 2 GB; `git filter-repo`
>    und Force-Push) — oder lassen; der nächste Commit löscht sie nur aus dem Baum. Die Aufsätze Brauchler 2023 und
>    Abdallah 2026 im Browser laden.
> 4. Nachlesen, was nur im Bericht steht und frei wäre: Ducceschi/Russo/Webb DAFx-2026, Le Carrou 2006, Nishiguchi et
>    al. 2021, Adachi et al. JASA 2026; zwischengespeichert Chadefaux 2013 „A model of harp plucking“, Perng DAFx-10.
> 5. Querschnitt: Rechenweg je Clip in drei Klassen (11 F-1, 2 F-3, Pedal beim Flügel) — als gemeinsame Frage, wenn
>    Patrik es wünscht.
>
> **Stolperfallen IC Session 2 (Hook-Kandidaten):**
> - **patchlib und `\r\n`:** patchlib setzt die Zeilenenden je Anker selbst; ein `\r\n` im neuen Text ergab
>   `\r\r\n` in `.gitignore` — die neuen Regeln wirkten still nicht. Kandidat: patchlib weist `\r` im neuen Text bei
>   CRLF-Dateien ab.
> - **VS-`.gitignore` verschluckt Datenordner:** `[Rr]eleases/` traf VCSL-`Releases/` (228 Dateien), nur über
>   `git status --ignored` gesehen. Kandidat: nach jedem Download `git status --ignored` prüfen (jetzt in der Merkregel).
> - **E-Mail an fremden Dienst:** Patriks Adresse ging einmal als Parameter an die Unpaywall-API — gegen die Regel.
>   Kandidat: Hook, der Bash-Befehle mit der Adresse in einer URL abweist.
> - **Abrufwerkzeug liest keine PDFs** (drei von vier Agenten): Kernwerte über `quelle_suchen.py` im Cache nachgelesen;
>   Agentenaufträge künftig darauf hinweisen.
> - **Archive über 100 MB** (K18, Iowa-Zips) gehen nicht auf GitHub — mit A-1 a ohnehin außerhalb.
> - **Zahlen in Fragen stimmen nicht immer:** „f19“ meinte F-10 (Tippfehler) — bei Unklarheit im Antworttext deuten
>   und die Deutung nennen.
>
> **Bilanz IC Session 1:** Vorab-Recherche für die Klassen 0, 1, 3, 4, 5, 6, 7, 8, 9, 11 mit je allen sieben
> Lieferungen; Zusatzfrage Z1 (Moden und Waveguides) für 1, 2, 3, 7, 8, 9; Klasse 10 nur die drei Agentenberichte
> (Lieferungen offen). Rund 30 Agentenberichte unverändert abgelegt, Kernwerte und Lizenzen je Klasse selbst
> nachgelesen (je Klasse unten). Keine Aufnahme geladen außer den vier PDFs unter `gemeinsam/quellen/`.
>
> **Plan aus IC Session 1 (Punkt 1 in IC Session 2 erledigt):**
> 1. **Klasse 10 Zunge** fertigstellen: die drei Berichte unter `klassen/Klasse 10 - Durchschlagende Zunge/recherche/` lesen, Kernwerte selbst
>    nachlesen (PDFs im Cache dieser Sitzung, Pfade in den Berichtsköpfen), dann `quellen.md`, `aufnahmen.md`,
>    `modell-entwurf.md`, `messplan.md`, `offene-fragen.md`, README, KLASSEN-Zeile 10. Kandidaten für Fragen aus den
>    Berichten: Balg = Expression; Balgwechsel als Artikulation (unisonorisch) bzw. Eigenschaft der Note
>    (bisonorisch); Musette-Stimmung als Ausstattung, Register als Passage-Spur; Mundharmonika-Bending über zwei
>    Zungen und Vokaltrakt (F-R5); Harmonium-Prolongement = Sostenuto; Bezug TinySOL (Akkordeon, CC BY 4.0), VCSL
>    (Mundharmonika, nur Blastöne), Euterpea-Harmonium (CC0).
> 2. Danach nach `KLASSEN.md`: **2 Gezupfte Saite (Deluxe)** — Z1-Befund liegt in `klassen/Klasse 2 - Gezupfte Saite/recherche/`.
> 3. Patriks Antworten auf die offenen Fragen (je Klasse `offene-fragen.md`, gemeinsam F-R1 bis F-R6, F-K1, F-D1)
>    einarbeiten, sobald sie kommen; Freigaben für Downloads je Klasse (Abschnitte unten).
>
> **Stolperfallen dieser Sitzung (Hook-Kandidaten):**
> - **Zeilenenden:** Das Write-Werkzeug schreibt LF; die Klassen-READMEs sind CRLF. Sechsmal von Hand auf CRLF
>   zurückgesetzt → Kandidat für einen Hook, der bei Write auf eine CRLF-Datei warnt (wie `patchlib --pruefe`).
> - **Agenten schreiben trotz „nichts in Dateien“:** dreimal Textauszüge im Scratchpad, einmal ZIP-Inhaltsverzeichnisse
>   per HTTP-Range — je im Berichtskopf vermerkt; Kandidat: Auftragstext um „keine Teilabrufe, keine Textauszüge“ ergänzen
>   (im Agentenauftrag der Klasse 10 schon geschehen).
> - **Heredoc mit Backslash** wurde zweimal vom Hook abgewiesen (Hook wirkt; Weg: Skript mit Write anlegen).
> - Zenodo-API: mit Browser-User-Agent 403, ohne geht es.

## Welche Klasse

**0 Gemeinsam** — erledigt (alle Downloads geladen). **4 Platte und Stab** — Vorab-Recherche erledigt, alle sieben
Lieferungen da (`klassen/Klasse 4 - Platte und Stab/`); offen: sieben Fragen (F-1 bis F-7), keine Aufnahme geladen. **1 Hammer-Saite
(Rest)** — Vorab-Recherche erledigt, alle sieben Lieferungen da (`klassen/Klasse 1 - Hammer-Saite/`); **beantwortet** (IC
Session 2), Upright-Aufnahmen unter `samples\`. **7/8/9 Rohr** — Vorab-Recherche erledigt, je alle sieben Lieferungen da
(`klassen/Klasse 7 - Lippe und Rohr/` mit Alphorn und Didgeridoo als Erweiterung, `klassen/Klasse 8 - Rohrblatt und Rohr/`,
`klassen/Klasse 9 - Luftstrahl und Rohr/`); gemeinsame Fragen F-R2 bis F-R5 neu, Befund zu F-R1; keine Aufnahme geladen. **3
Bogen-Saite** — Vorab-Recherche erledigt, alle sieben Lieferungen da (`klassen/Klasse 3 - Bogen-Saite/`); **beantwortet** (IC
Session 2), Aufsätze geladen, Aufnahmen offen (Plan Punkt 1). **5 Membran, 6 Becken und Gong** — Vorab-Recherche erledigt, je alle sieben
Lieferungen da (`klassen/Klasse 5 - Membran/`, `klassen/Klasse 6 - Becken und Gong/`, gemeinsame Berichte in `gemeinsam/recherche/`); offen: acht
bzw. sechs Fragen, keine Aufnahme geladen. **Zusatzfrage Z1** (Moden und Waveguides, `AUFGABE.md`) für die Klassen 1, 2,
3, 7, 8, 9: erledigt (Abschnitt unten). **11 Tonrad** — Vorab-Recherche erledigt, alle sieben Lieferungen da
(`klassen/Klasse 11 - Tonrad/`); offen: sieben Fragen, keine Aufnahme geladen. **10 Zunge** — Vorab-Recherche erledigt (IC Session
2), alle sieben Lieferungen da (`klassen/Klasse 10 - Durchschlagende Zunge/`); offen: neun Fragen, keine Aufnahme geladen. **2 Gezupfte Saite
(Deluxe)** — Vorab-Recherche erledigt (IC Session 2), alle sieben Lieferungen da (`klassen/Klasse 2 - Gezupfte Saite/`);
**beantwortet**, Aufnahmen unter `samples\`. **Alle Klassen vorab recherchiert.**

### Lehren aus dem Setzen des Flügels (IC Session 3, 2026-10-27)

- **Input:** [LEHREN_SETZEN_FLUEGEL.md](LEHREN_SETZEN_FLUEGEL.md) aus LumiViz S157; Entscheide ZF1 a, ZF3 a, ZF4 a,
  SP1 a–SP3 a dort im Wortlaut nachgelesen.
- **Eingearbeitet (Auftrag Patrik):** L1–L7 in [VORBILD_FLUEGEL.md](VORBILD_FLUEGEL.md) (Ablauf mit Zeile *Spielbar*,
  Lehren aus dem Setzen), [SCHNITTSTELLE.md](SCHNITTSTELLE.md) §Messkette (Stand S157), [gemeinsam/messkette.md](../../messung/ic/messkette.md)
  §3a „Erkennung nur am Bezug“, §4a „Toleranz als Auflösung des Bezugs“.
- **Geprüft:** alle Klassen gegen L1–L4, L7 — [gemeinsam/lehren-pruefung.md](lehren-pruefung.md) (drei
  Agentenberichte unter `gemeinsam/recherche/`, Stichproben nachgelesen). Überall: Toleranzen geschätzt, keine
  Stützpunkte, kein Setzen über alle Noten, keine Länge der Bezüge, keine Zeit je Auswertung. **Nichts in den Klassen
  geändert.**
- **Entschieden 2026-10-28:** F-L1–F-L5 alle a („prinzipiell alle a, es gibt aber news aus abklärungen in
  lumiViz“). Neu aus LumiViz nachgelesen: **RF1 a** — Grundlagen bis 31h zuerst, Feinabstimmung als eigene letzte
  Stufe 31i; 31d-8 gebaut, Hörurteil 8.1 offen. Eingetragen: Z2, Z3 (mit Länge der Töne) und die Ergänzungen der
  Lieferungen 3–5 und von „Fertig“ in [AUFGABE.md](AUFGABE.md); Ablauf und L6 in `VORBILD_FLUEGEL.md` nach RF1 a
  (Feinabstimmung zuletzt); F-L4 nach RF1 a angepasst (zwei Zahlen *Grundlagen* / *Feinabstimmung*, Vermerk in
  `KLASSEN.md`). F-L5 geprüft: Iowa-Stereo trägt zwei Mikrofonorte.
- **F-L5-1 a erledigt (2026-10-28):** Iowa-Streicher stereo 2496 geladen (`samples\Klasse 3 - Bogen-Saite\Iowa_MIS_Strings2012_stereo2496\`,
  264 AIFF, 8,09 GB, Begleitdateien im Repo); mono ist an allen 264 Paaren das Mittel (L+R)/2 (Violine auf 16 bit
  abgeschnitten) und deshalb unter `samples` gelöscht, Begleitdateien mit Vermerk. 17 Vl-pizz sind auch in stereo
  doppelt mono. Auf C rund 37 GB frei.
- **Offen:** **Der Durchgang
  über die Klassen 0–11 nach F-L3 a** (Z2-Tabelle, Z3-Spalten mit gemessener Länge, Schwellen gesucht/gesetzt,
  Hörproben-Zeile, Zeit je Auswertung, Aufwand in zwei Zahlen) — Arbeitsliste [gemeinsam/lehren-pruefung.md](lehren-pruefung.md).

### Klasse 3 — Bogen-Saite (Antworten IC Session 2)

- **Entscheide Patrik (2026-10-26):** F-1 a, **F-2 b** (Waveguide-Saite mit Reibstelle, Korpus modal und rückwirkend —
  die Empfehlung aus dem Nachtrag Z1), F-3 bis F-8 a, F-9 a (Downloads freigegeben).
- **Geladen:** Russo, Ducceschi, Bilbao 2022 (CC BY 4.0) und Maestre, Scavone, Smith 2013 (CC BY 3.0) unter `quellen/`
  mit `SHA256SUMS`. **Aufnahmen und Bogendaten:** geladen 2026-10-27 (IC Session 3) — Iowa MIS 2012, TinySOL (Streicher), TU-Note,
  VSCO 2 CE (Streicher), CNSM, Lampis 13374477, zusammen 14,4 GB unter `…\cmake\samples\Klasse 3 - Bogen-Saite\`;
  Begleitdateien in `aufnahmen/`, `daten/`. **Befunde:** Iowa-Violine trotz „2496“ nur 16 bit (11 Dateien 48 kHz);
  TU-Note-WAV 16 bit statt der 24 bit des Beipackzettels; CNSM-Eintrag ohne Beschreibung. **Offen:** Saitenantworten
  (17749111, ≥ 14 GB) — vorher fragen; auf C frei rund 27 GB.

### Ablage der Downloads — A-1 a (IC Session 2)

- **Frage Patrik:** bringen Samples im Repo etwas, wenn Claude auf GitHub arbeitet? („3GB auf github sind schon etwas
  viel“). Geprüft (Agent, code.claude.com): Cloud-Umgebung „4 vCPUs, 16 GB of RAM, and 30 GB of disk“; keine Grenze für
  Repogröße/Klonzeit dokumentiert; Zenodo, archive.org, Iowa nicht auf der Standard-Netzliste (je Umgebung
  freischaltbar). **Entscheid A-1 a** mit Zusatz Patrik: Audio und große Messdaten lokal unter `…\cmake\samples\<Klasse
  n - Name>\`, Klassenordner umbenannt in „Klasse n - Name“.
- **Umgesetzt:** fünf Datensätze verschoben (Klasse 1: K18, VCSL; Klasse 2: VCSL, Iowa, Mores), je `LADELISTE.tsv`
  (Prüfsumme, Pfad, Quelle) erzeugt; elf Klassenordner umbenannt; acht eigene Dateien und zwei Z1-Befunde in den
  Verweisen nachgezogen, Linkprüfung ohne Fehler; `.gitignore` schließt `klassen/**/aufnahmen/**` und `daten/**` aus,
  lässt `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv` zu.

### Klasse 2 — Gezupfte Saite (Deluxe) (IC Session 2)

- Berichte (Agenten, unverändert): `recherche/2026-10-26_korpus_deluxe.md`, `…_physik_cembalo_harfe_koto_oud.md`,
  `…_spielweisen_steuerung.md`, `…_aufnahmen.md`; Z1-Befund aus Session 1. Im Kopf je Bericht, was selbst nachgelesen ist.
- Bestand in LumiViz gelesen (`Saiten_Konzept.md` §2, S-K1 a bis S-K11 a, §5d–§5f; `modellierung/gezupfte-saite/`;
  `harp`, `koto` sind Samples).
- Selbst nachgelesen (`quelle_suchen.py` in den PDFs dieser Sitzung; HTML, APIs): Bank & Karjalainen 2010 (2×2 passiv,
  Gibson 1960er, R = 30, Schwebung und zweistufiges Abklingen); Hess 2013 Tab. 1; Scherrer & Depalle (2×2 am Steg);
  Chadefaux et al. SMAC 2013 (CC BY 3.0, Kiel 0,10/0,25 m/s, 12/7,9 ms); Chadefaux et al. 2012 (Harfe, 15 N, 4 mm,
  2 m/s); Penttinen 2006 (5/3/0,5 dB, 10 dB Korpus, 2,41 dB); Välimäki et al. 2004 (Register, 12–50 %); Bader et al.
  arXiv 2025 (CC BY 4.0, Dulcken: 37 Hz, T60 235 ms, 3938 N); Paté et al. 2017 (Abstract: Springergeschwindigkeit ohne
  Einfluss — **Widerspruch** zu Chadefaux/Penttinen, F-4); Abdallah et al. 2026 (Oud, CC BY 4.0); Nishiguchi et al.
  2021 (Koto, J-STAGE). Aufnahmen: Iowa (Wortlaut, Kopf der Gitarrenseite), Zenodo-Lizenzen und Größen (AG-PT-set,
  GuitarSet, Mores 4604577, Rabāb 10079910, 19823590), VCSL-Dateibaum, Freesound 381515.
- Abschätzung (Skript `zupfabschaetzung.py` im Scratchpad, Ergebnis in `modell-entwurf.md` §5): Gitarre Deluxe 5,6 % der
  Echtzeit; volles Mitschwingen Harfe 16 %, Cembalo 25 % — je Clip.
- **Entscheide Patrik (2026-10-26):** F-1 bis F-9 **a**; F-10 a **abgewandelt** — Downloads zuerst **in dieses Repo**, dann
  durch **A-1 a** nach `…\cmake\samples\Klasse 2 - Gezupfte Saite\` verlegt (privat,
  nur vorübergehend, wird später in LumiViz integriert); dafür neu [UEBERNAHME_LUMIVIZ.md](UEBERNAHME_LUMIVIZ.md)
  (was in LumiViz ignoriert werden muss, wo Lizenzen zu nennen sind); Ablageregel in `README.md`, `AUFGABE.md`,
  `CLAUDE.md` nachgezogen.
- **Geladen:** Mores-Archiv ohne Fotos (34,7 MB, `klassen/Klasse 2 - Gezupfte Saite/daten/`), VCSL Harfen und Cembali (566
  Dateien, `…/aufnahmen/VCSL/`), Chadefaux et al. SMAC 2013 und Bader et al. 2025 (`…/quellen/`) — je mit `SHA256SUMS`.
  Iowa-Nylongitarre mono 24/96 (Freigabe Patrik) als 45 Einzeldateien (828 MB, größte 37,6 MB — das Zip mit 627 MB
  läge über der GitHub-Grenze) unter `…/aufnahmen/Iowa_MIS_Guitar_mono2496/`.
  **Nicht geladen:** Brauchler 2023 und Abdallah 2026 (Verlag lieferte Bot-Seite — über den Browser); AG-PT-set erst mit
  dem Stahlkorpus. `.gitignore` um Ausnahmen für `aufnahmen/` und `daten/` ergänzt (die VS-Regel `[Rr]eleases/` hätte
  228 Cembalo-Releases verschluckt).
- **Fragen an Patrik:** `klassen/Klasse 2 - Gezupfte Saite/offene-fragen.md` F-1 bis F-10 (F-2 Korpus aus Messung statt Bauplan;
  F-3 Mitschwingen je Clip; F-4 Cembalo-Stärke; F-5 Manual an der Note wie die Saite; F-6 Pedaldiagramm als Spur; F-7
  Notenende = Hand dämpft; F-8 Bend mit Vorlauf; F-9 Chöre im Saitensatz, Cent-Versatz an der Note). **Zum
  Zurückstellen vorgelegt:** Koto und der gemessene Stahlkorpus bis zu einer freien Aufnahme (F-1).
- Abweichungen der Agenten: drei Agenten meldeten, das Abrufwerkzeug lese keine PDFs; einer las fünf PDFs als Bild.
  PDFs liegen im Cache dieser Sitzung (`~/.claude/projects/…/85f9b48d-…/tool-results/`), nicht im Repo. Die GitHub-Suche
  nach Korpus-Impulsantworten ist am Rate-Limit gescheitert (Lücke der Suche).

### Klasse 10 — Zunge (Berichte IC Session 1, Lieferungen IC Session 2)

- Berichte: `recherche/2026-10-26_aufnahmen.md`, `…_physik_rechenverfahren.md`, `…_spielweisen_steuerung.md` (Agenten),
  unverändert; im Kopf je Bericht vermerkt, was die Sitzung selbst nachgelesen hat.
- Selbst nachgelesen (`quelle_suchen.py` in den PDFs aus dem Cache von IC Session 1): Puranik & Scavone FA 2023 (CC BY
  3.0; „in isolation“, „inertial effect“, 0,2–1,5 kPa, „unrealistic values“, Tab. 1, N = 40); DAFx23 (Lizenzsatz CC BY
  4.0 **mit** „non-commercial“ — widersprüchlich; „1-2 Hz“, Gehäuse 10 Biquads); Cottingham 2011 (15 %, „somewhat
  below“, Vokaltrakt, „always oscillates“; kein Rechtsvermerk); Adachi et al. FA 2026 (CC BY 3.0, „not reviewed“,
  −2,5/−5,1 Hz/kPa, Tab. 1); Elejalde-García et al. 2021 („All rights reserved“, 10–35 Cent); Hergert FA 2023 (CC BY 3.0,
  Musette, 0,5–15 Hz); Cottingham-Folien 2013 („takes over“, „Max Bend“); arXiv-Lizenzlink Millot & Baumann
  (nonexclusive-distrib). **Nicht gefunden** im Wortlaut: „largely sinusoidal“ (dort „predominantly sinusoidal“),
  Cottingham 2011 „usually has little effect“ — in den Lieferungen nicht als nachgelesen geführt. Aufnahmen: TinySOL
  (`cc-by-4.0`, Akkordeon aus der CSV ausgezählt: 689 Töne, 82 Tonhöhen, pp/mf/ff je 82), VCSL und Euterpea-Harmonium
  (GitHub `CC0-1.0`), Freesound 330410 (CC0), 23039 und 325377 (CC BY 4.0), Commons „Accordion registers.ogg“ (Public
  domain).
- Abschätzung (Skript `zungenabschaetzung.py` im Scratchpad, Ergebnis in `modell-entwurf.md` §6): Kragbalken trifft
  Harmonium B5 (969 gegen 990 Hz); Masse-Feder ≈ 2 % der Echtzeit je Zunge, verteilter Balken ≈ 20 % (Deluxe).
- Bestand in LumiViz: SF10 a kennt die Zunge nicht (F-1); kein mitgeliefertes Zungeninstrument.
- Hinweis aus dem Spielweisen-Bericht: patmissin.com trägt ein „No-AI statement“ — nicht ausgelesen; so beibehalten.
- **Freigaben ausstehend:** keine beantragt — TinySOL (1,03 GB, dient auch 3, 7, 8), Euterpea-Harmonium (≈ 77 MB),
  VCSL-Mundharmonikas, fese-Melodica; drei Aufsätze CC BY 3.0 unter `quellen/` (F-9).
- **Fragen an Patrik:** `klassen/Klasse 10 - Durchschlagende Zunge/offene-fragen.md` F-1 bis F-9 (F-1 Bauart „Wind × Stab“ als Ergänzung von SF10 a;
  F-2 Vibrato in Cent gerät an der Zunge an die Grenze von F-R3 a; F-3 Balgrichtung als Spur; F-5 Bass-Seite im selben
  Instrument; F-6 Mundharmonika-Bending mit zwei Zungen und Vokaltrakt nach F-R5). **Zum Zurückstellen vorgelegt:**
  Mundharmonika-Bending und Overblow bis zu einer Bend-Aufnahme und Adachi et al. JASA 2026 (F-8).
- Nicht nachgezogen: `gemeinsam/koerper.md` Zeile „10 Zunge … offen (nicht gesucht)“ — der Gehäusefilter des Harmoniums
  (DAFx23) wäre dort ein erster Eintrag.

### Klasse 11 — Tonrad (IC Session 1)

- Berichte: `recherche/2026-10-26_tonradgenerator.md`, `…_leslie.md`, `…_aufnahmen.md` (Agenten). Der Generator-Agent
  hat eine Textausgabe im Scratchpad abgelegt (`werner.txt`, nicht im Projekt).
- Selbst nachgelesen: Werner et al. 2016 („≈ 0.85 ms“, „4× oversampling“); Pekonen et al. 2011 („ten sinusoidal
  oscillators“, „sixth harmonic“, 800 Hz Butterworth); Smith et al. 2002 („growl“) und Handout (15°-Schritte, AM
  „throb“); Rumsey 2010 („approximately 20 filters“, „separate filters“); Crossref-Lizenzen Moro et al. 2017 und Werner &
  Abel 2016 (CC BY 4.0); setBfree GPL-2.0-or-later (Kopf `tonegen.c`); Lizenzen Freesound 136653/136652/333770/333769
  (CC0), Commons (Public domain), archive.org M100 (CC BY 4.0), Zenodo 3562442 (CC BY 4.0, Größe); Bestand SF10 a, SF8 a,
  E1 a/E2 b, `organ.lvinst`, `house_organ.lvinst`.
- **Freigaben ausstehend:** keine beantragt — Leslie-Aufnahmen, Zenodo 3562442 (968 MB), M100-Drones (≈ 210 MB), drei
  freie Aufsätze erst nach F-6.
- **Fragen an Patrik:** `klassen/Klasse 11 - Tonrad/offene-fragen.md` F-1 bis F-7 (F-1 Rechenweg je Clip gegen Ton je Note, F-2
  Leslie-Zustand über Clips gegen E2 b, F-3 Expression als Schweller gegen SF10 a, F-5 klärt `organ`/`house_organ` nach
  SF10 a, F-7 Markennamen).

### Zusatzfrage Z1 — Moden und Waveguides (IC Session 1)

- Berichte (Agenten): `gemeinsam/recherche/2026-10-26_moden_und_waveguides_saite.md` (Klassen 1–3),
  `…_moden_und_waveguides_rohr.md` (Klassen 7–9); je Klasse ein Befund der Sitzung
  `klassen/<klasse>/recherche/2026-10-26_moden_und_waveguides.md` (auch für die noch offene Klasse 2).
- Selbst nachgelesen: Bank, Zambon & Fontana 2010 (PDF-S. 3 „precise modeling is not possible with digital waveguides
  in their efficient form“, PDF-S. 9 „uni-directional“); Bank 2006 („Five to ten resonators“); Lee et al. 2009 („only
  necessary to modulate the frequencies of the lower partials“); Mansour et al. 2016 („time-domain approach more
  natural“); Maestre et al. 2018 („numerical dispersion“); Cooper & Abel 2010 (Msallam, Textebene verschoben); Willemsen
  et al. („simplified bore profiles“); Mignot et al. 2009 („static parameters“); Harrison et al. („awkward“); Scavone
  2003 („reed scattering junction“).
- Folgen: **Klasse 1** neue Frage F-8 (modal bleibt, Mischform nur bei hörbarer Modengrenze; Hackbrett/Cimbalom
  zweiseitige Kopplung prüfen). **Klasse 3** F-2: **Empfehlung neu b** (Waveguide-Saite aus S-K1 a, Korpus modal und
  rückwirkend) — offen als Nachtrag vorgelegt. **Rohr** neue Frage gemeinsam F-R6 (Waveguide-Stücke im modalen Kern:
  Zug, Schmettern in der Schleife, Vokaltrakt, Strahl); **Klasse 7** F-2 Nachtrag (Rückwirkung des Schmetterns nicht
  klein). **Klasse 2** Befund liegt bereit für ihre Recherche.
- Abweichung der Agenten: der Rohr-Agent hat PDFs aus dem Cache per `pdftotext` in Textdateien im Scratchpad gewandelt
  (nicht im Projekt).

### Klassen 5 und 6 — Schlagwerk (IC Session 1)

- Berichte: `klassen/Klasse 5 - Membran/recherche/2026-10-26_physik_rechenverfahren.md`,
  `klassen/Klasse 6 - Becken und Gong/recherche/2026-10-26_physik_rechenverfahren.md`, `gemeinsam/recherche/2026-10-26_aufnahmen_schlagwerk.md`,
  `…_spielweisen_schlagwerk.md` (Agenten). Der Aufnahmen-Agent hat Inhaltsverzeichnisse von ZIP-Archiven per HTTP-Range
  im Speicher gelesen (kein Download in Dateien) — im Kopf des Berichts vermerkt.
- Selbst nachgelesen: DrumGizmo-Lizenz DRSKit und CrocellKit (Wiki-Rohtext, CC BY 4.0); Zenodo-API Tom-Tom-Datensatz,
  Oslo-Becken, Tabla strokes, Beijing Opera (je `cc-by-4.0`, Größen); arXiv-Lizenzen 0809.1320, 1405.2589
  (nonexclusive-distrib), 2307.09425 (CC BY 4.0); Bilbao & Webb (82 s, „a minor effect in timpani“); Marogna et al.
  („negligible“); Wagner (3 N bis 100 N); Dahl 2011 (5–8 ms, Faktor 2); Miura 2023 (250–350 ms); DAFx23 (CC BY 4.0,
  0,484 s); SMAC 2013 („two hours“); Bilbao 2010 (1–160 s); Chaigne et al. 2005 („three semitones“); Sekiguchi &
  Samejima (Fußdruck „numerical divergence“); Bestand Loop_Konzept §4a, §4d, §4l (K1 a–K11 a, D1 a–D6 a), 11f, HA3 a,
  Zielbild Schlagwerk.
- **Freigaben ausstehend:** keine beantragt — DrumGizmo-Teile, Tom-Tom-Datensatz (90 MB), Iowa-Becken, freie Aufsätze
  erst nach den Bezugsfragen (5 F-8, 6 F-6).
- **Fragen an Patrik:** `klassen/Klasse 5 - Membran/offene-fragen.md` F-1 bis F-8 (F-2 nach HA3 a, F-3/F-4 brauchen Spielweisen und
  Länge am Drum-Schritt, F-5 Pauke als Instrument), `klassen/Klasse 6 - Becken und Gong/offene-fragen.md` F-1 bis F-6 (F-1 revidiert
  „eher Deluxe“, F-2 Chaos und Bitgleichheit, F-3 Hi-Hat gegen K5 a). **Zum Zurückstellen vorgelegt:** Conga und Bongo
  bis zu Modendaten (5 F-7), Hi-Hat ans Ende (6 F-1).

### Klasse 3 — Bogen-Saite (IC Session 1)

- Berichte: `recherche/2026-10-26_physik_rechenverfahren.md`, `…_korpus_daempfer_ensemble.md`,
  `…_spielweisen_steuerung.md`, `…_aufnahmen.md` (Agenten).
- Selbst nachgelesen: Russo, Ducceschi & Bilbao 2022 (CC BY 4.0, Tab. 2: 0,19/0,39/0,91); Galluzzo et al. 2017 Gl. 1–2;
  Schoonderwaldt et al. 2008 S. 604; Guettler & Askenfelt 1997 S. 2903; Guettler 2002 (Sprungrate 6–35 Hz); Guettler
  1997 „Bow Notes“ (0,5–1,5 N); Schoonderwaldt et al. 2007 (vB/β); Chudy et al. 2013 (16,62 cm/s); Woodhouse 2014
  PDF-S. 11, 34, 42; Zhang & Woodhouse Tab. II (Text verschoben); Maestre et al. 2013 (CC BY 3.0); Elie et al. (5–13 dB);
  Crossref-Abstracts Geringer & Allen 2004, MacLeod 2008/2010, Mellody & Wakefield 2000; Zenodo-API TinySOL, Lampis
  17749111/13374477, Pàmies-Vilà 17207713, Seven Violin (je `cc-by-4.0`, Größen); TU-Note CC BY-ND 4.0; Bestand
  SF6 a, SF8 a, SF9 a, SF10 a, SK4 a, SK10 a, Zielbild §1, S-K1 a, S-K4 a, S-K7 a, S-W19 a, Flügel-Saite modal.
- **Freigaben ausstehend:** keine beantragt — Iowa, TinySOL, CNSM (654 MB), Lampis (3,0 GB und Teile zu 44 GB), Russo
  2022 und Maestre 2013 als PDF erst nach F-7/F-9.
- **Fragen an Patrik:** `klassen/Klasse 3 - Bogen-Saite/offene-fragen.md` F-1 bis F-9 (F-2 gegen S-K1 a, F-3 gegen SK4 a, F-5 belegt
  SK10 a, F-6 ergänzt das Zielbild, F-7 nach S-K7 a). **Zum Zurückstellen vorgelegt:** Viola und Kontrabass bis zu
  Saiten- und Korpusdaten (F-1).

### Klassen 7, 8, 9 — Rohr (IC Session 1)

- Berichte: `klassen/Klasse 7 - Lippe und Rohr/recherche/2026-10-26_physik_spielweisen.md`, `…_alphorn_didgeridoo.md`;
  `klassen/Klasse 8 - Rohrblatt und Rohr/recherche/2026-10-26_physik_spielweisen.md`;
  `klassen/Klasse 9 - Luftstrahl und Rohr/recherche/2026-10-26_physik_spielweisen.md`; `gemeinsam/recherche/2026-10-26_aufnahmen_blaeser.md`
  (alle Agenten).
- Selbst nachgelesen: Willemsen et al. (CC BY 3.0, Tab. 1, „every 20 samples“); Myers et al. 2012 (B-Werte, Schwerpunkt
  „from 1.5 to 4.0“); Petiot et al. (Mundruck, „Variable, according to the regime simulated“); Maestre, Scavone & Smith
  DAFx-18 (Überblenden, „above 30 times faster“, 32 Resonatoren); Li et al. 2016 (730 dB/s); Chen et al. 2011 („no
  consistent tuning“); Marcinnò et al. 2026 (CC BY 4.0, Tab. 3); Tarnopolsky et al. 2006 I; Fletcher et al. 2006 II
  („two series-resonant“); Fletcher & Douglas 1980 S. 767–768; Castellengo 1999 S. 387; Iowa-Lizenz; Bestand SF5 d, SF6 a,
  SF8 a, SF10 a, SK4 a, SK12 a, Modell 28a, Zielbild §1.
- Befund zu F-R1 (in `gemeinsam/offene-fragen.md`): Legato, Posaunenzug und Schmettern kippen den modalen Kern nicht;
  Halbventil bleibt Deluxe. Neu: F-R2 Expression als Weg durch Druck und Ansatz, F-R3 Vibrato-Spur bei Bläsern, F-R4
  Legato-Ketten, F-R5 Vokaltrakt.
- **Freigaben ausstehend:** keine beantragt — Iowa-Bläser, VSCO 2 CE, VAO (6,1–6,4 GB), Palo (0,1–2,1 GB) erst nach den
  Bezugsfragen (7 F-7, 8 F-4, 9 F-5).
- **Fragen an Patrik:** `klassen/Klasse 7 - Lippe und Rohr/offene-fragen.md` F-1 bis F-7, `klassen/Klasse 8 - Rohrblatt und Rohr/offene-fragen.md`
  F-1 bis F-4, `klassen/Klasse 9 - Luftstrahl und Rohr/offene-fragen.md` F-1 bis F-5. **Zum Zurückstellen vorgelegt** (Regel
  „Pausieren nur mit Freigabe“): Oboe/Fagott bis zu einer Geometrie (8 F-1), Altflöte und Shakuhachi bis zu Geometrie
  und Aufnahmen (9 F-1).

### Klasse 1 (IC Session 1)

- Berichte: `recherche/2026-10-26_upright_honkytonk_cimbalom.md`, `…_cp70_clavinet.md`, `…_aufnahmen.md` (Agenten).
- Selbst nachgelesen: K18-Metadaten (CC BY 4.0, 6 Stufen in kleinen Terzen, 424 364 716 B); VCSL `LICENSE` (CC0),
  `Info.txt` (Knight), `Note.txt` (Yamaha); FreePats Upright KW und Honky-tonk (CC0); Chaigne, Cotté & Viggiano 2013
  S. 2457–2461 (Upright-Boden); Gabrielli et al. 2013 S. 1, 3, 4, 6, 11, 15 (CC BY 2.0, Tangente, −3 Halbtöne,
  Rechenaufwand); Bestand §4.1 Zielbild, HA1 a, HA2 a, HA4 a, UF14 a, SF10 a.
- **Entscheide Patrik (2026-10-26, IC Session 2):** F-1, F-2, F-3, F-7, F-8 a; F-5 a abgewandelt (Downloads in dieses
  Repo); F-6 **b** („bis recherchen noch mehr gefunden haben“ — auch das Cimbalom aus Literatur und Modell); F-4 „beides?“
  → **F-4-1 a** (Schieber stufenlos, Tastendruck als Pitch nach oben mit Grenze 0 bis zur Messung).
  **Hinweis Patrik:** die Hammer-Saite wird in LumiViz gerade am Flügel bearbeitet (S155: Teilplan 7.5, SQ2 a mit Grenze
  der Verstimmung 0–6 Cent — für das Honky-tonk aus F-1 a gilt diese Flügel-Grenze nicht, beim Bau prüfen) — vor jeder
  Arbeit an Klasse 1 den Stand in LumiViz neu lesen.
- **Geladen (IC Session 2):** K18 Upright Piano (CC BY 4.0; Archiv 424 MB außerhalb ausgepackt, 212 WAV, 940 MB) und
  VCSL Upright Knight/Yamaha (CC0, 290 Dateien, 638 MB) — nach A-1 a unter `…\cmake\samples\Klasse 1 - Hammer-Saite\`;
  im Repo unter `aufnahmen/` je `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`. **Nicht geladen:** Gabrielli et al. 2013 (CC BY 2.0), FreePats Honky-tonk — nicht Teil von F-5.
- **Offen:** keine Frage mehr; Klasse 1 vollständig beantwortet.

### Klasse 4 (IC Session 1)

- Berichte: `recherche/2026-10-26_physik_bauteile.md`, `…_aufnahmen.md`, `…_spielweisen.md` (Agenten).
- Selbst nachgelesen: Iowa-MIS-Lizenzsatz, Raum, Mikrofone (Hauptseite); VCSL-README (CC0, Aufnahmeregel); VSCO-2-
  `LICENSE` (CC0); Rucz & Ulveczki 2021 S. 1, 5–7 (Tab. 1: Schlägel 25 g, 5·10⁷ N/m^1,5, Kontakt 0,6 ms; Resonator
  +15 dB, T60 10 s → 0,7 s; 1 : 4 : 10 / 1 : 3 : 9); Bestand SF6 a, SF8 a, SF10 a, SK6 a, SK12 a.
- Abschätzung (Skript `stababschaetzung.py` im Scratchpad, Ergebnis in `modell-entwurf.md` §6): 25 Biegemoden bei C2,
  unter 3 % der Echtzeit.
- **Freigaben ausstehend:** keine beantragt — Iowa-Dateien (Größe nicht gelesen) erst nach F-6.
- **Fragen an Patrik:** `klassen/Klasse 4 - Platte und Stab/offene-fragen.md` F-1 bis F-7 (F-2, F-3, F-4, F-5 gleichen den Bestand in
  LumiViz ab: Motor, Dämpferleiste, Bogen/Bending, Schlägel).

## Fertig (IC Session 1)

- Ablage: fremde PDFs werden versioniert, Rechtslage je Datei in `quellen.md` (Anweisung Patrik 2026-10-26;
  `.gitignore`, `README.md` „Ablage und Lizenzen“, `AUFGABE.md`, `CLAUDE.md`, die elf Klassen-READMEs).
- `gemeinsam/`: [koerper.md](../../koerper/ic/koerper.md), [rohr.md](../../rohr/ic/rohr.md),
  [messkette.md](../../messung/ic/messkette.md), [quellen.md](../../gemeinsam/ic/quellen.md), [aufnahmen.md](../../gemeinsam/ic/aufnahmen.md),
  [offene-fragen.md](../../gemeinsam/ic/offene-fragen.md), README; drei Berichte von Agenten unter `gemeinsam/recherche/`.
- Selbst nachgelesen: Zenodo-Lizenzen (Mores 4604576, Mores 4010596, CNSM 18696785: je `cc-by-4.0`); TU Berlin
  DepositOnce („In Copyright“, Dateiliste); STK `LICENSE`; OpenWInD PyPI (GPLv3); Harrison et al. 2015 Tab. 6;
  Bilbao 2008 S. DAFX-5; Ege, Boutillon & David 2009 (arXiv-Seite); in LumiViz `Teiltonmessung.hpp`,
  `InstrumentBody`, Bauplan Steinway D.
- `KLASSEN.md` Zeile 0 nachgeschärft (alte Schätzung daneben).
- Abgleich mit dem **Bestand in LumiViz** (IC-B1 a, `AUFGABE.md`): `rohr.md` §1 berichtigt (Ton je Note mit Spur ist
  in LumiViz schon der Weg: SF10 a, SK12 a — keine Abweichung), F-K1 baut auf dem Gitarrenkorpus S-K8 a/S-K10 a auf,
  `koerper.md` §1 nennt ihn, `messkette.md` bezieht B2/B3 auf die Spuren (Vorgaben, nicht Spieler).

## Offen

- **Geladen** nach `gemeinsam/quellen/` (SHA-256 in `gemeinsam/quellen.md`): TU Berlin `0_Documentation.pdf` (Befund:
  Dokumentation CC BY-NC-SA 4.0, Datensatz „In Copyright“ — F-D1 neu gefasst); Mores `Method.pdf`, `Comments.pdf`,
  Maßliste (von Patrik im Browser geladen, Zenodo sperrte Abrufe von hier) — gelesen und in `gemeinsam/koerper.md` §4
  eingetragen (48 kHz, 1 s je Stelle, sechs Kanäle, nicht normiert; Teilung deterministisch/statistisch bei 400–700 Hz
  wie KB1 a).
- **Freigaben ausstehend** — frei lesbare Volltexte: Ege, Boutillon & David 2009 (arXiv 0909.0885); Guillemain et al. 2005, Tournemenne &
    Chabassier 2019, Auvray et al. 2014 (HAL — von hier aus gesperrt, ggf. Patrik); Ernoult et al. 2023 (CC BY 3.0).
- **Fragen an Patrik:** `gemeinsam/offene-fragen.md` — F-R1 (Kern des Rohrs), F-R2 bis F-R5 (Expression, Vibrato,
  Legato, Vokaltrakt der Bläser), F-K1 (Luftraum gemeinsam), F-D1 (Lizenz TU Berlin).
- **Nicht gesucht:** Raumimpulsantworten mit geprüfter Lizenz (OpenAIR nicht erreichbar), Zunge/Orgel/Tonrad in der
  Messkette, Bohrungsprofile außerhalb Zenodo/HAL.
- **Cache der Sitzung:** WebFetch hat PDFs selbsttätig im Sitzungsordner unter `~/.claude/projects/…/tool-results/`
  abgelegt (nicht im Repo; seit den Klassen 7–9, 3, 5 und 6 deutlich mehr). Dort liegen auch zwei Textauszüge geschützter
  Werke (`b1a3qyn45.txt` DAFx-15, `b350ju9hm.txt` Galluzzo 2017), die der Agent zum Löschen empfiehlt. Löschen
  entscheidet Patrik.
