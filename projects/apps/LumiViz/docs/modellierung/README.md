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
  quellen/         fremde Arbeiten als PDF — nur lokal, nicht versioniert (.gitignore), Dateiname
                   Autoren_Jahr_Titel_Ort.pdf
```

## Regeln

- **Belegkraft kennzeichnen:** ein Bericht eines Agenten ist kein Beleg; was davon in ein Konzept geht, wird dort
  als „nachgelesen“ (selbst im Volltext geprüft) oder „nur Bericht“ geführt (globale Regel *Netzabrufe*).
- **Fremde PDFs nicht versionieren** — auch nicht, wenn sie frei abrufbar sind; die Lizenz steht in der Recherche.
  Eigene Ausarbeitungen und Berichte (mit kurzen Zitaten) werden versioniert.
- **Eine Aussage an einer Stelle:** Entscheide im Konzept, Herleitungen und Modell hier, je mit Verweis.

## Klassen

| Klasse | Instrumente | Stand |
|---|---|---|
| [hammer-saite](hammer-saite/README.md) | Flügel, Klavier (Stufe 31/32) | Modell 31b gebaut; Resonanzboden in Recherche |
| [koerper](koerper/README.md) | gemeinsame Grundlage für Körper aller Instrumente (KB1 a) | Recherche, Entscheid KB1 a |
| [gezupfte-saite](gezupfte-saite/README.md) | Gitarre, Bass, Ukulele, Violine (gezupft) | Recherchen S132–S136 abgelegt (aus Saiten_Konzept §6–§6c und `.claude/handover/`) |
