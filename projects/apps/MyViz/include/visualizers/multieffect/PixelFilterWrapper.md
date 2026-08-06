# PixelFilterWrapper — GLSL-Fragment-Wrapper des Pixel-Filter-Nodes

> **Version:** 1.0.0
> **Datum:** 2026-08-06
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Stilfilter-Strang, Session 70)
> **Modul:** lumi::pixelfilter (freie Funktionen)
> **Dateien:** PixelFilterWrapper.hpp (header-only)
> **Namespace:** lumi::pixelfilter
> **Abhängigkeiten:** keine (kein GL, kein Qt — Text → Text, testbar)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Textbausteine des **Pixel-Filter-Nodes** (`PixelFilterParams`, Offene_Punkte
§7 Stilfilter): das skriptbare Stilfilter-Modul — Entscheid Patrik S70:
EIN Filtermodul + Werks-Voreinstellungen (Take-On-Me-Comic, Posterize,
Halftone, CRT/VHS, Kuwahara, Sepia …) statt 1–3 Festmodule. Eine
Nutzer-GLSL-Funktion läuft je PIXEL im Fragment-Shader und färbt das
Chain-Bild um; sie wirkt damit auf JEDE Quelle (Video, Kamera, Scopes,
MilkDrop). Der Filter-STACK ist die Kette selbst: mehrere Filter = mehrere
Knoten, umsortierbar wie alles andere; ein Knoten = EIN Pass — Multipass-
Looks laufen über den Shadertoy-Knoten mit Buffer A–D.

**Vertrag des Nutzer-Codes:**

```glsl
vec4 farbe(vec2 uv, vec4 src)   // uv 0..1 (y oben), src = Quellpixel
```

**BEFUND S70: `filter` ist in GLSL ein RESERVIERTES Wort** (AMD lehnt die
Kompilierung ab) — daher der deutsche Name nach dem `kraft()`-Muster der
GPU-Partikel. Ein Wrapper-Test erzwingt, dass das Wort nirgends als
Bezeichner auftaucht.

## 2. API-Kern

- `wrapFragment(userCode)` — GLSL-330-Fragmentprogramm: Prälude (Uniforms
  `uTex/uResolution/uTime/uDelta/uFrame` + `bass/mid/treb/vol/beat` +
  `uMixAmount`, endet mit `#line 1` → Treiberfehler tragen Nutzer-Zeilen),
  Nutzer-Code (leer = Identität), Epilog-`main()` (`#line 100000`): ruft
  `farbe(vTex, src)` und mischt über `uMixAmount` gegen das Original.
  Eigene Namen im `_lumi`-Reserviert-Raum; Vertex-Anteil ist der geteilte
  `kQuadVertexShader` des Hosts (Varying `vTex`).
- `starterFilter()` — Palette-Vorbelegung: Mini-Comic (Posterize, dessen
  Stufenzahl mit dem Bass atmet, + Sobel-Kantenzug) — sichtbar ab Frame 1.
- Nachbar-Abtastung ist Teil des Vertrags: `texture(uTex, uv + …)` —
  Kantenzüge (Sobel/DoG), Kuwahara-Quadranten, Chroma-Versatz.

## 3. Render-Pfad (MultiEffectVisualizer::runPixelFilter)

transformPass-Muster: Quelle = `active().current()`, Ziel = Partner, danach
Swap; Draw über den geteilten Quad-VAO. Programm-Rebuild nur bei
Code-Wechsel (Snapshot `pfCompiled`). Kompilierfehler landen im geteilten
`stError` → `shadertoyError(nodeId)` versorgt Panel-Anzeige und den
Apply-Poll des Groß-Editors (S69); Fehler ⇒ Passthrough.
Parameter-Skript (Strang D): `mixamount`.

## 4. Werks-Voreinstellungen (asset/nodepresets/pixelFilter/)

12 Looks (Katalog: `asset/nodepresets/README.md`): **Take-On-Me-Comic**
(Sobel + Papier/Tinte + Beat-zitternde Schraffur — das Flaggschiff),
Bleistift-Skizze (XDoG), Posterize-PopArt, Zeitungsdruck-Halftone,
CRT-Monitor, VHS-Band, Kuwahara-Oelbild, Sepia-Nostalgie,
Noir-Schwarzweiss, Waermebild, Pixel-Art, Duotone-Neon. Alle
deterministisch (Hash/uFrame statt rand) und audio-reaktiv gestimmt.

## 5. Tests

`test_PixelFilterWrapper.cpp`: #line-Vertrag · Identitäts-Fallback ·
Uniform-Satz · Epilog-Mix-Vertrag · Reserviert-Wort-Wächter (`filter(`
darf im Wrapper nicht vorkommen) · Starter-Vertrag · Serializer-Roundtrip
+ Mix-Klemme. Beweise: `asset/effectchain/pixelfilter_sonde.lvfx`
(Doppellauf SHA256-identisch) + Demo `asset/examples/pixelFilter -
Take-On-Me.lvfx` (Sichtprüfung: Papier, Konturen, Schraffur, Tusche-Band).

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-08-06 | Session 70 — Erstfassung (farbe()-Vertrag wegen GLSL-Reserviert-Wort `filter`, Starter, 12 Werks-Voreinstellungen) |
