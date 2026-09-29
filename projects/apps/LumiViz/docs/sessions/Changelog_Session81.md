# Changelog Session 81 (2026-09-29)

> **Custom-Waves liegen jetzt dort, wo MilkDrop sie hinlegt — und die
> Vergleichsmessung gegen das Original ist deutlich verlässlicher geworden.**
> Auf Breitbild-Schirmen streckt LumiViz die Custom-Waves wie das Original;
> der bisherige Look bleibt per Schalter erreichbar. Die Referenzläufe laufen
> auf einem festen Bildschirm, in mehreren Formaten, auf einer festen
> Grafikkarte — und drei Fehler im Messgerät sind behoben.

## Neu

- **Schalter „Seitenverhältnis (Waves)“** im Milkdrop-Knoten (Standard:
  Original). Custom-Waves werden auf Breitbild senkrecht um das
  Seitenverhältnis gestreckt, wie in MilkDrop (4:3 um 4/3, 16:9 um 16/9).
  „LumiViz (bisher)“ holt den Look von Session 39–80 zurück.
- **Übersicht aller „bisher“-Schalter** im Benutzerhandbuch: Skript-Zufall,
  Audio, Seitenverhältnis und die Voreinstellung „Weicher Sog ↖“.
- **Messwerkzeuge** (nur für die Kalibrierung):
  - fester Bildschirm für Referenzläufe (`LUMIVIZ_REF_BILDSCHIRM`),
  - Vergleich über mehrere Formate (`--formate aspekt,monitor`) für MilkDrop
    und AVS,
  - feste Grafikkarte für Referenz und LumiViz, von jeder Messung geprüft
    (`--karte`),
  - fortsetzbare Läufe (`--fortsetzen`), Temperatur- und Leistungsprotokoll
    je Sekunde, keine Konsolenfenster mehr,
  - vollständige Argument-Dokumentation aller vier Werkzeuge.

## Behoben

- **Custom-Waves lagen auf Breitbild zu hoch bzw. ungestreckt** gegenüber
  dem Original (Faktor `m_fInvAspect` fehlte seit Session 39).
- **Referenz-Renderer (MilkdropRef):**
  - die Kaltstart-Saat kam einen Frame zu spät,
  - der Kern war auf Shader-Modell 2.x gekappt,
  - Presets mit Shader-Modell 3.0 (*Helix*, *The Beauty and the Math*,
    *Starfield*-Varianten) blieben auf der integrierten Radeon 610M schwarz —
    sie galten seit Session 75 als „Referenz stumm“. Auf der RTX 4090 zeichnet
    die Referenz sie; *The Beauty and the Math* stimmt mit LumiViz überein.

## Bekannt

- Die Anzeige des Referenz-Renderers streckt ihr Bild um einen Pixel und
  zeichnet es leicht weich — für scharfe Muster weicht sie dadurch von LumiViz
  ab. Ob LumiViz das nachbilden soll, ist offen.
