# Changelog Session 171 (2026-11-02)

> **Stufe 15 ist fertig.** Ein Clip kann jetzt auf einen anderen Kanal hören: der neue Effekt **Sidechain** macht ihn
> leiser, solange dieser Kanal laut ist — das klassische Pumpen eines Pads unter der Bassdrum, und der Weg, auf dem
> später die Talkbox ihre Stimme bekommt. Das Benutzerhandbuch beschreibt die Wave-Clips nun vollständig.
> Steuerdokument: `docs/audio/Loop_Konzept.md` §4p.

## Neu

- **Sidechain** im Fenster *Clip effects* (15l): **Channel**, **Depth** (bis 48 dB), **Attack**, **Release**. Der
  gewählte Kanal wird hinter seinen eigenen Effekten gehört, vor Fader und Schalter — ein ausgeschalteter Kanal
  steuert weiter. Nennen zwei Kanäle einander, wirkt keiner, und das Fenster sagt warum. Umbenennen und Verschieben
  des Kanals stören nicht.
- **Benutzerhandbuch 1.206.0** (15m): §16 „Tondateien im Loop: Wave-Clips“ — Anlegen, Wave editor, Autotune,
  Aufnehmen mit *Measure latency*, Bounce; Sidechain unter „Effekte am Clip“.

## Geändert

- **Bounce meldet, wenn es nichts ersetzt hat** — etwa „The clip changed while bouncing — nothing was replaced“ steht
  jetzt in der Berichtzeile des Panels (vorher nirgends).
- Freeze rechnet den Kanal, den ein Sidechain nennt, mit: ändert er sich, gilt der eingefrorene Clip als verändert.
- Wird ein Clip oder Kanal in einen anderen Loop eingesetzt, verliert ein Sidechain, der einen Kanal außerhalb der
  Datei nennt, seinen Kanal — die Meldung sagt es.

## Intern

- `.lvloop` **Fassung 17**: je Kanal `id`, Baustein `sidechain`. Ohne Sidechain rechnet alles Bit für Bit wie vorher
  (`Loop Werte`, Klangfassung unverändert).
- Rechenkern mit Abgriffen je genanntem Kanal (`Abgriff`, `Bus::steuer`), `SidechainEffekt`, `steuerKanal` (Kreis),
  `freieKennung`, `steuerQuellen`; Freeze-Abdruck mit festen Namen und Kennungen.
- Tests: **1753 Cases** (+6): `test_LoopSteuersignal`, ein Fall mehr in `test_LoopBounce`; voller Lauf 1753 / 1753
  unter Release-clang und Testing-MSVC (zweimal).
- Sichttest-Liste `Sichttest_WaveClips.md` (lokal).
