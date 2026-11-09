# Modellierung von Instrumenten — Recherchen, Modelle, Messpläne

> **Seit:** Session 146 (2026-10-23), auf Wunsch Patrik. **Zweck:** alles, was zur physikalischen Modellierung eines
> Instruments erarbeitet wird, an einem Ort — nach Instrumentenklasse geordnet. Die **Entscheide** stehen weiter in
> den Konzepten unter `docs/audio/` (`Saiten_Konzept.md`, `Spielweisen_Konzept.md`); hier steht, **woraus** sie
> entstanden sind und **was** gerechnet wird.

## Aufbau je Klasse

```
modellierung/<klasse>/
  README.md        Inhaltsverzeichnis: Konzept-Abschnitte, Recherchen, Modell, Messplan, Quellen
  modell.md        eigene Ausarbeitung: was LumiViz rechnet, Gleichungen, Herkunft jeder Größe
  messplan.md      wie Messungen das Modell absichern sollen (auch als Forschungsauftrag zu vergeben)
  recherche/       Berichte von Recherchen, je Datei eine — Datum_Session_Thema.md, unverändert übernommen,
                   mit Kopf: Auftrag, Kennzeichnung, was selbst nachgelesen ist, wohin es eingeflossen ist
  (kein quellen/)  fremde Arbeiten liegen seit S182 außerhalb des Repos (siehe Regeln)
```

## Regeln

- **Belegkraft kennzeichnen:** ein Bericht eines Agenten ist kein Beleg; was davon in ein Konzept geht, wird dort
  als „nachgelesen“ (selbst im Volltext geprüft) oder „nur Bericht“ geführt (globale Regel *Netzabrufe*).
- **Fremde PDFs nicht ins Repo** — auch nicht, wenn sie frei abrufbar sind; die Lizenz steht in der Recherche.
  Ablage seit S182 (Auflösung von IC, U1) unter `…\cmake\quellen\<thema>\`, Thema wie der Ordner hier, Dateiname
  Autoren_Jahr_Titel_Ort.pdf; archiviert auf der Wechselplatte, Index `../archiv/Index_quellen.md`. **Ein Pfad
  `quellen/<datei>` oder `../quellen/<datei>` in einem älteren Bericht meint diese Ablage** (die Berichte bleiben
  unverändert; woher jede Datei kam: `…\cmake\quellen\UMZUG_S182.tsv`). Eigene Ausarbeitungen und Berichte
  (mit kurzen Zitaten) werden versioniert.
- **Eine Aussage an einer Stelle:** Entscheide im Konzept, Herleitungen und Modell hier, je mit Verweis.

## Klassen

| Klasse | Instrumente | Stand |
|---|---|---|
| [hammer-saite](hammer-saite/README.md) | Flügel, Klavier (Stufe 31/32) | Modell 31b gebaut; Resonanzboden in Recherche |
| [koerper](koerper/README.md) | gemeinsame Grundlage für Körper aller Instrumente (KB1 a) | Recherche, Entscheid KB1 a |
| [gezupfte-saite](gezupfte-saite/README.md) | Gitarre, Bass, Ukulele, Violine (gezupft) | Recherchen S132–S136 abgelegt (aus Saiten_Konzept §6–§6c und `.claude/handover/`) |
| [platte-stab](platte-stab/README.md) | Marimba, Xylophon, Vibraphon, Glockenspiel, Röhrenglocken, Celesta, Kalimba, zwei elektrische Klaviere (Klasse 4, Kern `bar`) | S178: aus IC übernommen (34g), Bit für Bit wie IC, ungemessen |
| [anbindung-ic](anbindung-ic/README.md) | Anbindung der Rechenkerne aus `InstrumentsCalculation` (alle Klassen; kein Klassenordner) | S172: Architektur-Vergleich, Plugin-Recherche (CLAP/VST3), Messung unter Windows-clang; Entscheide in `audio/Instrumente_IC_Konzept.md` |
