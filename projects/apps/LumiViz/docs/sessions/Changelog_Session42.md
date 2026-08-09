# Changelog — Session 42 (2026-07-22/23)

Milkdrop-Editing komplett (N3), fShader-Farbwash, **Host-Gruppen mit
Crossfade** (E5-Strang HG1–HG3), Qt-ADS-Update gegen Docking-Bugs,
sektions-genaue Skript-/Shader-Referenzen. Tests am Ende: **399 Cases grün,
0 Skips**; Builds VS-Debug/-Testing (`/WX`) + Ninja-Clang-Release grün.

## Behoben

- **Docking: doppelter Player + verschwundene Titelleiste** nach Abreißen/
  Wieder-Andocken des Effect-Chain-Panels — das waren Bugs im
  Docking-Framework: Qt-ADS von 4.4.1 auf **5.0.0** aktualisiert (dessen
  Release-Notes nennen genau diese Fixes); zusätzlich räumt die App native
  GL-Fenster jetzt auch nach Float/Redock auf (bisher nur nach Vollbild).
- **Crossfade-Einblenden wirkte nicht** (Sichttest): der Gruppen-Mix ist
  jetzt exakt normalisiert — Ein- und Ausblenden sind gleichwertig, egal in
  welcher Reihenfolge die Gruppen in der Kette liegen.
- Veralteter Import-Hinweis „Platzhalter bis C2" korrigiert; der
  hue_shader-Hinweis ist jetzt eine ℹ-Info statt Warnung.

## Neu

- **Milkdrop-Presets sind voll editierbar (N3):** Der Node zeigt sechs
  Sektionen — Code · Waves · Shapes · Shader · **Sprites** ·
  **Parameter**. Custom Waves/Shapes/Sprites lassen sich über das
  Toolbar-Dropdown **anlegen, klonen und entfernen** (je Element ein
  Baum-Eintrag mit Voll-Editor); die Parameter-Sektion macht alle
  numerischen Basiswerte des Presets einstellbar (Decay, Gamma, Echo,
  fShader, Waveform, Motion, Borders, Motion Vectors, Blur — mit Hinweis,
  wenn ein Comp-Shader die Composite-Werte einbackt).
- **fShader-Farbwash:** der klassische MilkDrop-Farbverlauf
  (`fShader`-Regler) wird gerendert — als langsam rotierender
  4-Ecken-Regenbogen, exakt nach Original-Formel (MD1-Pfad, eingebackene
  MD2-Shader und Custom-Shader-Pfad).
- **Host-Gruppen:** Ein neuer Container-Typ kapselt ein komplettes Visual
  (AVS-Kette, Milkdrop-Preset oder eigene Effekte) mit eigenem
  Feedback-Bild, eigenen Buffer-Slots und eigenen Skript-Variablen —
  beliebig viele, auch gleichzeitig aktiv und stapelbar; Gruppen in Gruppen
  sind nicht erlaubt. Ketten mit Gruppen speichern als **`.lvfx2`**.
- **Crossfade zwischen Visuals:** Das Auge einer Gruppe blendet sie weich
  ein/aus (Dauer synchron für alle Gruppen; **Ein-/Ausgangskurve je Gruppe**:
  Linear, S-Kurve, Ease-In, Ease-Out, Exponentiell) — beide Visuals laufen
  während des Übergangs live. Der Button **„Zu dieser Gruppe wechseln"**
  schaltet mit einem Klick um; bricht die Framerate ein, blendet die alte
  Gruppe automatisch als Standbild aus (Performance-Fallback). Die
  `progress`-Variable von Milkdrop-Presets läuft jetzt ab Gruppen-Start.
- **Passende Hilfe-Referenzen:** ⓘ neben jedem Milkdrop-Skriptfeld zeigt
  jetzt die zur Sektion passende Original-Variablenliste (per_frame/
  per_pixel, Wave, Shape, Sprite — inkl. Kompatibilitäts-Hinweisen); die
  **Shader-Editoren** haben erstmals eine eigene ⓘ-Referenz (Inputs,
  Konstanten, Sampler, Funktionen).
- **Benutzerhandbuch 1.1.0:** neues Kapitel „Effektketten, Milkdrop &
  Host-Gruppen".

## Verifikation

Neue Test-Gates: Host-Gruppen-Roundtrip + Tiefenregel-Degradierung.
ADS-Update per UI-Automation verifiziert (Undock/Redock-Zyklus sauber).
Sichttests: Crossfade-Ausblenden bestanden; die große Feature-Runde
(N3, fShader, Host-Gruppen mit normalisiertem Mix, Referenzen) und c1+m5
stehen in-app aus.

## Offen

Sichttest-Runde (s. o.), dann **Visual-Playlist (E6)** auf der
Host-Gruppen-Wechselmechanik → C3 → Decay-Dither + .milk-Export.
Fortschritts-SSOT: `visuals/MilkDrop_Import_Status.md` (v1.11.0);
Host-Gruppen: `visuals/HostGruppen_Crossfade_Entwurf.md` (v1.6.0).
