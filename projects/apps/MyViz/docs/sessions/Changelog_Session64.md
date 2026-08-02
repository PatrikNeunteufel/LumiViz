# Changelog — Session 64 (2026-08-02)

Zweite Runde der **MilkDrop-Kalibrierung**: die 22 Rest-Auffälligen aus S63
sind auf **acht Fehlerklassen** zurückgeführt und sechs davon gefixt.
OK-Quote der 311 Pack-Presets: 289 → **295** — bei gleichzeitig ehrlicherer
Messmethodik (Prüfstände starten jetzt saatlos wie die Referenz). Die
verbleibenden 16 Auffälligen sind vollständig klassifiziert: 5 sind
nachweislich original so (PRESET-IST-SO), 11 Port-Bugs mit bekannter
Ursachenklasse. Tests **493/493**.

## Neu für Benutzer

- **Deutlich mehr Presets zeigen das richtige Bild:** Ganze Familien sind
  geheilt — die „glass bead game"-Serie, „Rainbow Attack NEON",
  „rainbow spider2", „infinity 2/3", die „riding the wave"-Familie,
  „witchcraft", „biohazard", „Moebius spiral 1" u. a.
- **Motion Vectors ohne Geisterstrahlen:** Ein Vertragskonflikt zweier
  älterer Fixes ließ jeden Motion Vector ein weißes Quad aus der Bildmitte
  ziehen — betroffene Presets übersteuerten zu Weiß.

## Unter der Haube (die acht Fehlerklassen)

1. **D3D9-Divisionsvertrag:** Original-Shader rechnen `0/0 = 0`
   (Legacy-Float), GLSL liefert NaN — ein NaN in einer Summe schwärzte
   Vollbilder. Der Transpiler emittiert Divisionen jetzt D3D9-treu.
2. **Synthetik-Audio:** Die drei Loudness-Bänder des Prüfstands waren nach
   der Normalisierung identisch — Presets, die durch Band-DIFFERENZEN
   teilen, liefen in 0/0. Jetzt band-eigene Hüllkurven.
3. **Loudness-Kaltstart wie das Original** (Ramp statt Erste-Wert-Saat —
   die Saat erzeugte bitgleiche Bänder und damit garantierte 0/0-NaNs).
4. **Skip- vs. Trenner-Semantik** im Scope-Renderer (`breakStrip`).
5. **8-bit-Rundung:** D3D9 schneidet beim Rendertarget-Write ab, GL rundet —
   Decay-Werte blieben bei uns auf einem Grauboden stehen (exakt an der
   theoretisch vorhergesagten Stall-Grenze). Jetzt trunkiert der
   Feedback-Pfad wie D3D9.
6. **NaN-Vertex-Koordinaten** werden nach der dokumentierten
   D3D9-Konvertierungsregel (NaN→0) bereinigt.
7. **q-Variablen-Kaltstart:** Presets ohne per_frame_init lesen in der
   Referenz uninitialisierten Speicher (winzige Werte statt exakt 0) —
   nachgebaut als deterministisches Epsilon unterhalb der
   EEL-Vergleichstoleranz.
8. **Riesen-UV-Behandlung** in doppelter Präzision (float verliert dort
   alle Nachkommabits).

## Messmethodik

- **Prüfstände starten saatlos** (Entscheid Patrik) — derselbe Kaltstart wie
  der Referenz-Renderer mit genulltem VRAM. Die Kaltstart-Saat bleibt
  App-Verhalten (`MilkdropStandalone --seed`). Gewinn: zwei Presets sind
  jetzt korrekt als „original so" klassifiziert statt falsch-OK, und
  „piercing 01" — Patriks ursprüngliche App-Beobachtung — ist erstmals im
  Prüfstand messbar.
- `compare_ref.py --dir` für beliebige Triage-Läufe;
  `LUMIVIZ_MILKDROP_DUMP_WARP` dumpt Warp-Ein-/Ausgabe zur Diagnose.

## Erforscht und dokumentiert: die Legacy-Grenze

Der charakteristische Look der „R-Serie" (MilkDrop2077) entsteht in der
Referenz aus **doppelt undefiniertem Verhalten**: uninitialisierter Heap
(q-Werte) multipliziert mit Fixpunkt-Überlauf im D3D9-Sampler. Wir stellen
die Klasse nach (endlich, deterministisch), der konkrete Bild-Charakter ist
prinzipiell nicht reproduzierbar.

## Entschieden (Patrik)

- **Regelwerk-Strang** (nächste Session): Node-Parameter
  `Legacy | Modern | Benutzerdefiniert` (Master + Einzelschalter je
  Emulation), Import-Default legacy, Neubauten modern; dazu
  PS-Version-Override je Shader-Typ (auto/PS2/PS3/MD1-erzwingen).
- **Shadertoy-Import** als Backlog-Idee: eigener Node-Typ (modernes
  Regelwerk), Audio als Shadertoy-natives FFT/Waveform-iChannel plus
  LumiViz-Uniforms; Lizenz-Vorbehalt (CC BY-NC-SA).

## Offen

11 Port-Bugs (pixies-Partikelsystem ×2, Gin Tonic 003, q-/UB-Dunkelklasse ×7,
piercing 01) · myPresets/ (41) ungeprüft · Details: `Offene_Punkte.md` §3.
