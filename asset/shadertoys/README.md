# Eigene Shadertoy-Shader (Strang S, Session 65)

20 **eigene** Shader (Standard-Techniken: Plasma, Voronoi, SDF, FBM, Gray-Scott
— Allgemeingut; KEIN kopierter Shadertoy-Code) als Veröffentlichungs-Vorrat
für Patriks Shadertoy-Konto (Ziel: ≥10 Veröffentlichungen für den API-Key).

## Portabilitäts-Regel

Jede Datei nutzt NUR Standard-Shadertoy-Uniforms (`iTime`, `iResolution`,
`iFrame`, `iChannel0..3`) — **kein** `bass/mid/treb/vol/beat` (das sind
LumiViz-Extras). Audio kommt aus `iChannel0` bzw. `iChannel1` als
512×2-Textur: **Zeile y≈0,25 = FFT-Spektrum, Zeile y≈0,75 = Waveform** —
identisch zur Shadertoy-Musik-Textur. Damit läuft der Code UNVERÄNDERT in
beiden Welten:

- **In LumiViz testen:** `python asset/shadertoys/make_lvfx.py` erzeugt je
  Shader eine .lvfx-Vorlage unter `asset/effectchain/shadertoys/` — im
  Import Browser doppelklicken (oder AvsStandalone). Die .glsl-Dateien sind
  die SSOT; die .lvfx werden generiert (nicht von Hand editieren).
- **Auf Shadertoy veröffentlichen:** Datei-Inhalt in einen neuen Shader
  einfügen; bei `*.bufferA.glsl` den Tab „Buffer A" anlegen (iChannel0 =
  Buffer A, iChannel1 = Music — steht je Datei im Kopfkommentar) und
  `*.image.glsl` in den Image-Tab; bei Single-Pass-Shadern iChannel0 = Music.

## Dateien

Einfach → anspruchsvoll; `[M]` = Multipass (Buffer A + Image):

| # | Datei | Idee |
|---|---|---|
| 01 | audio_ringpulse | Konzentrische Ringe, Radius aus Waveform, Farbe zyklisch |
| 02 | spektrum_stadt | FFT-Skyline mit Fenstern + Wasser-Spiegelung |
| 03 | lissajous_gluehspur `[M]` | Leuchtspur mit Decay, Kurvenfrequenzen aus Bass/Höhen |
| 04 | plasma_interferenz | Sinus-Summen-Plasma, Phasen audio-moduliert |
| 05 | voronoi_zellatmung | Wandernde Voronoi-Kerne, Zellglühen je Spektralband |
| 06 | tunnel_herzschlag | Polarer Tunnel, Vortrieb aus dem Bass |
| 07 | reaktions_diffusion `[M]` | Gray-Scott in Buffer A, feed audio-moduliert |
| 08 | sternenfeld_warp `[M]` | Pseudo-3D-Sternenflug mit Trail, Warp bei Bass |
| 09 | raymarch_blobs | SDF-Metaballs (smooth-min) mit Phong, Radien = Bänder |
| 10 | julia_drift | Julia-Menge, c wandert auf audio-gestörter Bahn |
| 11 | aurora_vorhang | Nordlicht-Bänder (Value-Noise) + Sterne, Schimmer aus Höhen |
| 12 | kaleidoskop_beat | Polar-Spiegel-Kaleidoskop, Segmentzahl aus dem Bass |
| 13 | regentropfen_fenster | Tropfen-Raster bricht ein Farb-Bokeh (Refraktions-Offset) |
| 14 | spektrogramm_rad `[M]` | Scrollende FFT-Historie in Buffer A, polar aufgerollt |
| 15 | hexgitter_puls | Hex-Zellen leuchten je Spektralband, Ring-Puls je Zelle |
| 16 | laser_scanner | Rotierende Leuchtstrahlen mit Glow, Tempo aus dem Bass |
| 17 | wellen_wasser | Wellen-Superposition als Höhenfeld + Fake-Beleuchtung |
| 18 | feuer_saeule | FBM-Feuer, Intensität/Zug aus der Lautstärke |
| 19 | orbit_partikel `[M]` | 48 Partikel auf Orbits mit Trail-Buffer |
| 20 | moire_scheiben | Zwei wandernde Ringsysteme, Moiré-Frequenz aus den Mitten |

Experten-Serie (21–40, Session 65 Runde 2 — Raymarching/Fraktale/Volumetrik):

| # | Datei | Idee |
|---|---|---|
| 21 | mandelbulb_puls | Raymarched Mandelbulb, Power atmet mit dem Bass, Orbit-Trap-Farben |
| 22 | menger_flug | Flug durch unendlich gekachelte Menger-Schwämme, Kanten-Glow |
| 23 | nebel_galaxie | Log-Spiral-Galaxie aus FBM-Nebel + funkelnde Sternebenen |
| 24 | ozean_weite | Heightfield-Ozean mit Fresnel-Himmelspiegelung + Sonne |
| 25 | apollonian_tanz | Apollonisches Kugelgepäck (Kugel-Inversionen), Bass pumpt |
| 26 | truchet_neon | Verschlungene Neon-Bahnen (Truchet), Lauflicht + Band je Kachel |
| 27 | schwarzes_loch | Gravitationslinse + wirbelnde Akkretionsscheibe, Doppler-Tönung |
| 28 | kifs_kristall | KIFS-Faltungs-Fraktal als glühendes Adergeflecht, Mitten morphen |
| 29 | fluss_feld `[M]` | Curl-Noise-Fluid: Tinten-Advektion im Wirbelfeld, Bass rührt |
| 30 | wolken_licht | Volumen-Marsch durch FBM-Wolken mit Sonnen-Selbstabschattung |
| 31 | neon_stadt_3d | Raymarched Endlos-Stadt, Fensterbänder = Spektrum je Haus |
| 32 | seifenblase | Dünnschicht-Interferenz (echte Farbphysik) auf wabernder Kugel |
| 33 | lava_lampe | Metaball-Feld mit harter Iso-Kante + Innenglut |
| 34 | wurmloch_flug | FBM-verformter Torsions-Tunnel mit Tiefen-Nebel |
| 35 | licht_faeden `[M]` | Seidige Lichtfäden auf Curl-Noise-Bahnen (Lights-Ästhetik) |
| 36 | mandel_zoom | Mandelbrot mit atmendem Exponential-Zoom, glatte Iterationsfarben |
| 37 | glas_torus | Raymarched Glas-Torus: Fresnel-Mix aus Reflexion + Fake-Brechung |
| 38 | duenen_daemmerung | Terrain-Raymarch: scharfe Dünenkämme im Abendlicht |
| 39 | blitz_gewitter | Prozedurale FBM-Blitze, Bass zündet, Wolken flackern mit |
| 40 | spiegel_kammer | Unendlich gespiegelte Neon-Rahmen (2D-IFS), Spiegel drehen mit den Mitten |

Themen-Serie (41–80, Session 65 Runde 3 — Life-Automaten/Bio/Feuerwerk/Technik/Kristalle/Glow):

| # | Datei | Idee |
|---|---|---|
| 41 | leben_neon `[M]` | Conways Game of Life, Zellalter färbt, Bass sät nach |
| 42 | leben_glut `[M]` | Generations-Ableger: Gestorbene verglühen in Glutstufen |
| 43 | zell_kolonie `[M]` | Larger-than-Life (5×5-Bänder): amöbenhafte Kolonien + Membran-Glow |
| 44 | kristall_zucht `[M]` | DLA-Wachstumsautomat: Kristallnadeln mit Schimmer-Jahresringen |
| 45 | leben_im_fraktal `[M]` | Game of Life IN einer wandernden Julia-Petrischale |
| 46 | feuerwerk_nacht `[M]` | Raketen + ballistische Explosionen mit Trails, Bass startet extra |
| 47 | funken_fontaene `[M]` | Boden-Vulkan sprüht Goldfunken, Lautstärke = Strahlhöhe |
| 48 | zellteilung | Mitose: Metaball-Zellen schnüren sich sichtbar durch |
| 49 | neuronen_feuer | Voronoi-Nervennetz, Pulse feuern über die Bahnen |
| 50 | dna_helix | Rotierende Doppelhelix mit Basenpaar-Sprossen, Tiefenwirkung |
| 51 | mikroben_tanz | Wimpertierchen-Ketten schlängeln (laufende sin-Welle) |
| 52 | blatt_adern | KIFS-Adernetz auf Blattform im Gegenlicht |
| 53 | burning_ship | Burning-Ship-Fraktal in Feuer-Rampe, atmender Zoom |
| 54 | newton_bassin | Newton-Fraktal (z³), Wurzeln rotieren, Grenzen fraktal |
| 55 | sierpinski_puls | Sierpinski-Kanten als Neon, Lauflicht durch die Ebenen |
| 56 | fraktal_baum | Verzweigungsbaum über Faltung, Wind + Mitten-Böen |
| 57 | phoenix_flug | Phönix-Iteration (z²+c+p·z₋₁), Bass = Flügelschlag |
| 58 | platinen_pulse | Leiterbahn-Truchet mit Signal-Läufern + Lötaugen |
| 59 | daten_regen | Fallende Blockglyphen-Kolonnen, Kopf blitzt weiß |
| 60 | radar_phosphor | Radarschirm mit echtem Phosphor-Nachleuchten (bufferfrei) |
| 61 | zahnrad_werk | Drei kämmende Zahnräder (SDF), Bass = Drehmoment |
| 62 | hologramm_globus | Drahtgitter-Globus mit Scanlines, Störungen, Chroma-Versatz |
| 63 | oszilloskop_phosphor | ECHTE Waveform als Oszilloskop-Strahl mit Glow |
| 64 | eis_stern | Schneeflocke wächst/schmilzt zyklisch, Glitzer aus den Höhen |
| 65 | edelstein_facetten | Rubin-Facetten blitzen nacheinander im rotierenden Licht |
| 66 | opal_schimmer | Opalisierende Farbpatches, Blickwinkel-Drift |
| 67 | prisma_spektrum | Weißer Strahl → Dispersion-Fächer + Staub im Licht |
| 68 | kristall_hoehle | Raymarch: Höhle voller schimmernder Kristallnadeln |
| 69 | glut_asche | Aufsteigende Glutpartikel + Hitzeflimmern + Glutbett |
| 70 | laser_raum | Perspektivisches Lasergitter (Boden+Decke) im Nebel |
| 71 | puls_ringe | Expandierende Schockwellen-Ringe, Bass gebiert Zentrumsringe |
| 72 | sternen_tor | Portalring mit Chevrons + wabernder Ereignisfläche |
| 73 | magnet_feld | Dipol-Feldlinien (Potential-Höhenlinien) mit Strom-Lauflicht |
| 74 | tinten_wirbel | Domain-Warping (fbm(p+fbm(p+fbm(p)))) — fließende Tinte |
| 75 | lichtsaeulen | Spektrum als Lichtsäulen-Bühne mit Bodenreflex |
| 76 | plasma_kugel | Plasmaglobus: zuckende Blitzfäden mit Wand-Fußpunkten |
| 77 | sonnen_korona | Brodelnde Sonne, FBM-Korona, Bass-Protuberanzen |
| 78 | gluehwuermchen | Blinkende Leuchtkäfer über Gras-Silhouetten, Mond |
| 79 | kometen_schweif | Komet: Ionen-+Staubschweif aus nachgerechneter Bahn-Vergangenheit |
| 80 | bio_lumineszenz | Leuchtplankton-Brandung: Wellenkante glüht türkis |

Kombinations-Serie (81–100, Session 65 Runde 4 — Techniken gekreuzt):

| # | Datei | Kombination |
|---|---|---|
| 81 | leben_feuerwerk `[M]` | Game of Life × Feuerwerk: sterbende Zellen zünden aufsteigende Funken |
| 82 | kaleido_tunnel | Kaleidoskop × Tunnel: facettierter Spiegelschacht |
| 83 | voronoi_lava | Voronoi × Lava: glühende Adern zwischen Basaltschollen |
| 84 | fraktal_feuer | Julia × Feuer: die Fraktal-Silhouette brennt |
| 85 | dna_daten | DNA × Daten-Regen: Glyphen fallen die Helixstränge hinab |
| 86 | galaxie_leben `[M]` | Galaxie × Life: Zellen leben nur auf den Spiralarmen |
| 87 | wellen_kristall | Wasser × Kristall: facettiert-quantisierte Wellen-Normalen |
| 88 | blitz_adern | KIFS × Blitz: Stromstöße schießen durchs Adergeflecht |
| 89 | feuer_tunnel | Tunnel × Feuer: brennende Wände, Ofen-Fluchtpunkt |
| 90 | opal_atmung | Opal × Voronoi-Atmung: Patches atmen mit ihrem Band |
| 91 | sternen_feuerwerk `[M]` | Warp-Flug × Feuerwerk: Bursts im Sternenfeld |
| 92 | magnet_neuronen | Magnetfeld × Neuronen: Pulse feuern über Feldlinien |
| 93 | eis_galaxie | Schneeflocke × Galaxie: 6-fach gefaltete Spiralarme |
| 94 | hex_datenstrom | Hexgitter × Daten-Regen: Kolonnen fluten Hex-Zellen |
| 95 | lava_wurmloch | Wurmloch × Metaballs: Lava-Blasen ziehen durch den Sog |
| 96 | prisma_regen | Regen × Dispersion: jeder Tropfen zieht einen Regenbogen |
| 97 | korona_ozean | Sonne × Ozean: Sonnenuntergang mit glitzernder Lichtstraße |
| 98 | kristall_schockwellen | Puls-Ringe × Facetten: Wellen lassen Facetten aufblitzen |
| 99 | leuchtender_garten | Baum × Glühwürmchen × Bio-Glow: Nachtgarten |
| 100 | grand_finale `[M]` | Feuerwerk × Ozean-Spiegelung × Sterne: das Abschluss-Stück |

## Lizenz

Inhalte sind Patriks Eigenwerk — Veröffentlichung unter Lizenz eigener Wahl
(Vorschlag: CC BY 4.0 oder MIT, damit sie breiter nutzbar sind als der
Shadertoy-Default CC BY-NC-SA).
