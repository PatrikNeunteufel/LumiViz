# InstrumentsCalculation — Vorarbeiten für gerechnete Instrumente

> **Angelegt:** 2026-10-26 (LumiViz Session 154), auf Wunsch Patrik. **Stand:** nur Struktur und Auftrag, noch keine
> Recherche. **Bezug:** LumiViz, Entscheid IK1 (S151) im `Spielweisen_Konzept.md` §4.1 — der Flügel wird in LumiViz
> fertig; dieses Projekt leistet die Vorarbeiten für die übrigen Instrumentklassen und nimmt später die Rechnung auf.

## Zweck

Für jede Instrumentklasse vorab erarbeiten, was LumiViz beim Flügel (Stufe 31) Sitzung für Sitzung erst finden musste:
Literatur, freie Aufnahmen mit geprüfter Lizenz, Daten von Bauteilen, ein Entwurf des Modells, ein Messplan und die
offenen Entscheidungsfragen. Danach kann jede Klasse gezielt nach dem Muster des Flügels ausgearbeitet werden.

**Die Abstimmung mit LumiViz kommt später.** Hier wird nichts an LumiViz geändert und keine Schnittstelle festgelegt;
`SCHNITTSTELLE.md` hält nur die Grundidee aus dem Flügel fest, als Rahmen für die Entwürfe.

## Inhalt

| Datei / Ordner | Inhalt |
|---|---|
| [KLASSEN.md](KLASSEN.md) | die Instrumentklassen, was vorab recherchiert werden kann, geschätzter Aufwand und erwartete Qualität |
| [BERICHT_BAU.md](BERICHT_BAU.md) | **Abschlussbericht** IC Sessions 1–4: was gebaut ist, Erkenntnisse, was sich in LumiViz integrieren lässt, was lokal zu laden ist, was noch abzuklären ist, Hinweise zum Merge |
| [AUFGABE.md](AUFGABE.md) | der Auftrag an eine Claude-Sitzung in diesem Ordner (Vorab-Recherche, erledigt für 0–11) |
| [AUFTRAG_BAU.md](AUFTRAG_BAU.md) | der Auftrag für den Bau der Rechenkerne aller Klassen unter `rechnung/` (Cloud-Sitzung ab 2026-10-29): Fragen konsolidieren, Grundgerüst, Klassen, Messkette, Setz-Werkzeug nach L1–L15 |
| [FRAGEN_KONSOLIDIERT.md](FRAGEN_KONSOLIDIERT.md) | alle offenen Fragen der Klassen 0 und 3–11 (62) nach Themen in 13 Bündeln, mit den Entscheiden der Klassen 1–3 als Vorbild — hier antwortet Patrik; die Empfehlung a ist die Vorgabe für den Bau |
| [LEHREN_SETZEN_FLUEGEL.md](LEHREN_SETZEN_FLUEGEL.md) | die Lehren aus dem Setzen des Flügels (L1–L7 aus S157, L8–L15 aus S163 mit Zahlen, L16–L18 aus S164/S165; L16 berichtigt L12) |
| `gemeinsam/cloud-global/` | wörtliche Kopie der globalen Regeln Patriks und der Werkzeuge `patchlib.py`, `quelle_suchen.py` aus `~/.claude` — gilt nur in Cloud-Sitzungen (G-1 a), nicht pflegen |
| `gemeinsam/vorbild-lumiviz/` | Kopie des Flügel-Vorbilds aus LumiViz (Rechenkern, Eigenlöser, Messkette, Suche, Konzepte; Stand S163) für Sitzungen ohne Zugriff auf LumiViz — nicht gepflegt, nicht ändern |
| [SCHNITTSTELLE.md](SCHNITTSTELLE.md) | die Grundidee der Schnittstelle aus dem Flügel (unverbindlich) |
| [VORBILD_FLUEGEL.md](VORBILD_FLUEGEL.md) | wie der Flügel in LumiViz erarbeitet wurde — das Muster, mit Lehren |
| `klassen/<klasse>/` | je Klasse: Recherchen, Quellen, Modellentwurf, Messplan, offene Fragen |
| `gemeinsam/` | was mehrere Klassen teilen (Körper, Abstrahlung, Rohr, Messkette) |

## Aufbau je Klasse (wie `LumiViz/projects/apps/LumiViz/docs/modellierung/<klasse>/`)

```
klassen/<klasse>/
  README.md          Inhaltsverzeichnis, Stand, Instrumente der Klasse
  recherche/         Berichte, je Datei eine: JJJJ-MM-TT_Thema.md, mit Kopf (siehe AUFGABE.md)
  quellen.md         Literatur: Angabe aus der Quelle selbst, Fundort, Zugang (frei / nicht frei), Lizenz
  quellen/           fremde PDFs — versioniert, Dateiname Autoren_Jahr_Titel_Ort.pdf, Rechtslage in quellen.md
  aufnahmen.md       freie Aufnahmen und Messdaten: Quelle, Lizenz am Ort geprüft, Inhalt, Größe, Eignung
  modell-entwurf.md  was gerechnet werden könnte: Gleichungen, Größen und ihre Herkunft, Rechenzeit
  messplan.md        welche Größen aus Aufnahmen messbar sind, wie, mit welcher Toleranz
  offene-fragen.md   Entscheidungsfragen an Patrik, im Format aus AUFGABE.md
```

## Ablage und Lizenzen

Das Repo liegt **privat** auf GitHub und bleibt privat. Es dient einzig der Recherche für Erweiterungen von LumiViz
(Loop-Panel). Dokumente — Berichte **und** fremde PDFs unter `quellen/` — werden **versioniert**, nicht ignoriert,
damit auch eine Claude-Sitzung ohne diesen Rechner (auf GitHub) damit arbeiten kann. (Anweisung Patrik, 2026-10-26.)

Dafür wird die **Rechtslage jeder Datei genau notiert** (in `quellen.md` bzw. `aufnahmen.md`), damit bei einer
Übernahme nach LumiViz sauber geklärt ist, was mitgehen darf: Lizenz oder Rechtsvermerk im Wortlaut, wo er steht,
Datum des Abrufs, und was davon übernommen werden darf. Was im Repo liegt, ist damit noch nicht für LumiViz frei.
Aufnahmen und Messdaten liegen **nicht** hier, sondern lokal unter `…\cmake\samples\<Klasse n - Name>\` (auch für
LumiViz erreichbar); im Repo je Datensatz nur `HERKUNFT.md`, `SHA256SUMS` und `LADELISTE.tsv` (A-1 a, Patrik
2026-10-26). Die Klassenordner heißen „Klasse n - Name“ (etwa `klassen/Klasse 3 - Bogen-Saite/`). Welche Dateien bei der Integration in LumiViz ignoriert werden müssen und wo Lizenzen zu nennen sind,
steht in [UEBERNAHME_LUMIVIZ.md](UEBERNAHME_LUMIVIZ.md).

## Regeln (Kurzfassung, ausführlich in AUFGABE.md)

- Deutsch; Belegkraft kennzeichnen („nachgelesen“ / „nur Bericht“); eine Zusammenfassung eines Werkzeugs ist kein Beleg.
- Kein Download ohne Patriks Freigabe (Datei, Quelle, Größe nennen); keine Git-Befehle, die etwas ändern.
- Nichts in LumiViz ändern.
