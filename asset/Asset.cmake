# ==============================================================================
# Asset.cmake — was von asset/ neben die Exe kommt
# ==============================================================================
#
# HIER wird eingetragen, was LumiViz mitliefert. Sonst nirgends: das
# CMakeLists.txt im Projekt-Root bindet diese Liste nur ein, und
# asset/AssetAbgleich.cmake fuehrt sie bei jedem Bau von LumiViz aus.
# Steuerdokument: projects/apps/LumiViz/docs/core-services/Datenablage_Konzept.md
# (Entscheide Patrik S114: D2 a, D3 a, D4 a, D10 c).
#
#   lumi_asset(<Bereich> [NACH <Ziel>] [NUR <Muster>...] [OHNE <Dateiname>...])
#
#   <Bereich>  Ordner unter asset/ (auch tiefer: img/logo/icons)
#   NACH       Ziel relativ zum Ordner der Exe; ohne Angabe asset/<Bereich>
#   NUR        nur diese Muster, relativ zum Bereich, je Muster EIN Ordner
#              (kit/*.wav trifft keine Unterordner von kit). Ohne NUR: alles,
#              mit allen Unterordnern.
#   OHNE       Dateinamen, die in keinem Ordner des Bereichs mitkommen
#
# Der Abgleich kopiert, was sich geaendert hat, und ENTFERNT am Ziel, was hier
# nicht (mehr) dazugehoert. Am Ziel also nichts von Hand ablegen.
#
# ⚠ NUR EIGENE WERKE. Was hier steht, wird mit der Exe weitergegeben. Fremde
# Presets, Shader und Texturen gehoeren nicht hinein — siehe
# THIRD_PARTY_NOTICES.md, „Nicht enthaltene Inhalte“. Einzige Ausnahme: die
# Samples unter loop/samples (CC BY 3.0, CC BY 4.0, CC0, mit Nennung; unten bei loop) und die
# Griffbilder aus chords-db unter loop/chords (MIT, mit Nennung in chords/CREDITS.md; S137). Draussen bleiben auch
# Pflegenotizen (README.md), Erzeuger-Skripte und Arbeitsdateien.
#
# Eine Datei unter asset/ geaendert? LumiViz bauen — das Programm liest die
# Kopie neben der Exe (Schalter dafuer: Settings, „Bundled Data“).
# ==============================================================================

# ------------------------------------------------------------------------------
# 1. Was das Programm selbst sucht            ->  <Exe>/asset/<Bereich>
#    (core/Mitgeliefert.hpp; ein neuer Bereich braucht dort einen Aufrufer)
# ------------------------------------------------------------------------------

# Voreinstellungen der Effekt-Module (NodePresetStore)
lumi_asset(nodepresets      OHNE "README.md")

# Shader-Vorlagen (ShaderVorlagen)
lumi_asset(shadertemplates  OHNE "README.md")

# Karaoke: Trail-Presets (TrailPresetStore) und die Karaoke-Presets daneben
lumi_asset(karaoke          OHNE "README.md")

# Composer: Blenden, Formen, Mutes (FormPresets, Mute)
lumi_asset(composer         OHNE "README.md")

# Loop: das Kit, die mitgelieferten Samples, die Instrumente, die Beispiel-Loops.
# Nicht: make_*.py, README.md und Arbeitsdateien im Ordner (test.lvloop,
# DDdemo.lvloop, ...) — ein Beispiel heisst <name>_demo.lvloop oder liegt unter
# genres/; unter vergleich/ die Vergleiche alt gegen neu (S133, make_saiten_loops.py). Die Samples unter samples/ sind die eine Ausnahme von „nur eigene
# Werke": fremde Aufnahmen unter CC BY 3.0, CC BY 4.0 und CC0 (Entscheid 2 a, Patrik S123) — ihre
# Nennung samples/CREDITS.md geht mit, und THIRD_PARTY_NOTICES.md fuehrt sie.
lumi_asset(loop
    NUR "kit/*.wav"
        "samples/*.wav"
        "samples/CREDITS.md"
        "kits/*.lvkit"
        "chords/guitar/*.lvchord"
        "chords/ukulele/*.lvchord"
        "chords/drop_d/*.lvchord"
        "chords/bass/*.lvchord"
        "chords/violin/*.lvchord"
        "chords/voicings/*.lvchord"
        "chords/CREDITS.md"
        "instruments/*.lvinst"
        "genres/*.lvloop"
        "vergleich/*.lvloop"
        "*_demo.lvloop")

# Formen (shape/Kontur): die mitgelieferten Zeichnungen. Nicht: make_contours.py
# und README.md.
lumi_asset(contours
    NUR "*.lvcontour")

# Format-Icons (PresetTypeIcons) und das Skript, das sie in Windows eintraegt.
# Nicht: die Python-Skripte, die sie erzeugen.
lumi_asset(img/logo/icons
    NUR "*.ico"
        "*.svg"
        "register_file_icons.ps1")

# ------------------------------------------------------------------------------
# 2. Sammlungen, die der Anwender selbst oeffnet   ->  <Exe>/<Name>
# ------------------------------------------------------------------------------

# Mitgelieferte Presets (S73) — Pflege: asset/presets/README.md.
# UI/StartPreset.hpp nennt <Exe>/presets/avs/EyeCandy2/02_flowers.avs fest.
lumi_asset(presets NACH "presets" OHNE "README.md")

# Im Nachgang zu klaeren, ohne Prioritaet (Patrik S114): Beispiele aus
# examples, composits, shadertoys, effectchain — je Ordner erst die Herkunft
# pruefen.
