# Anbindung InstrumentsCalculation — Berichte und Messungen

> **Seit:** Session 172 (2026-11-02). **Zweck:** Herleitungen zur Anbindung der Rechenkerne aus
> `…\cmake\InstrumentsCalculation` (`rechnung/`) an LumiViz. Die **Entscheide** (IK2-0 bis IK2-13) stehen im Konzept
> [docs/audio/Instrumente_IC_Konzept.md](../../audio/Instrumente_IC_Konzept.md); hier steht, woraus sie entstanden sind.
> Kein Klassenordner im Sinne der [README der Modellierung](../README.md) — die Klassen selbst liegen in IC unter
> `klassen/Klasse n - Name/`.

| Datei | Inhalt | Belegkraft |
|---|---|---|
| [2026-11-02_S172_architektur_rechnung_loop.md](2026-11-02_S172_architektur_rechnung_loop.md) | Architektur-Karte IC `rechnung/` gegen LumiViz `loop/`; Dubletten je Baustein mit Diff-Zahlen; alle 16 Ton-Funktionen mit Signatur; Kopplung (Qt, Schalter, GLOB); zwölf Befunde; Zeilen je Bibliothek | Bericht eines Agenten (nur gelesen); die fünf tragenden Befunde in S172 an Datei und Zeile nachgelesen |
| [2026-11-02_S172_recherche_plugins_clap_vst3.md](2026-11-02_S172_recherche_plugins_clap_vst3.md) | VST3-SDK 3.8.1 (MIT) und CLAP 1.2.10 (MIT): Lizenzen, Hosting-Klassen, Marke „VST“, Offline/Prefetch, Host-Mindestbedarf, freie Host-Bausteine, Einfrieren in DAWs, eigene Instrumente als Plug-in | Bericht eines Agenten (WebFetch/curl, kein Browser); Reichweite je Angabe im Bericht (Wortlaut gegen Zusammenfassung); Steinberg-Hilfe gesperrt |
| [2026-11-02_S172_messung_clang_windows.md](2026-11-02_S172_messung_clang_windows.md) | `rechnung/` unter Windows-clang 20.1.7 gebaut und gemessen (M1 a): 29 Abdrücke bitgleich mit MSVC, 13 mit Linux; Rechenzeit je Instrument in drei Läufen nach Taktprobe | selbst gemessen in S172 |
| [Uebernahme_Journal.md](Uebernahme_Journal.md) | Je Baustein der Bibliotheken: Quelle (Repo, Commit, Datei), Ziel, übernommen/verworfen und warum, Wächter vorher/nachher (IK2-2 a); seit 34c (S175) | fortgeschrieben je Teilschritt, Läufe selbst gefahren |
