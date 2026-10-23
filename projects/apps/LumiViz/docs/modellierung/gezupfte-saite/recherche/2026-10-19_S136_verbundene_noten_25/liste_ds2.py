"""Schreibt die Abrufliste fuer fernzip.py getliste: Beschreibung, alle Annotationen von dataset2,
und die Audios der Technik-Aufnahmen (bending, slide, vibrato, Licks mit B/S/H) sowie G-Saite normal.

    python -I liste_ds2.py <guitar_liste.txt> <zielwurzel> <ausgabe.txt>
"""
import re
import sys
from pathlib import Path

liste, wurzel, aus = sys.argv[1], Path(sys.argv[2]), sys.argv[3]
zeilen = []
for z in open(liste, encoding="utf-8"):
    name = z.rsplit(" ", 1)[0]
    if name.endswith("SMT_GUITAR_dataset_description.pdf"):
        zeilen.append(f"{name}\t{wurzel / 'beschreibung'}")
    if "/dataset2/annotation/" in name and name.endswith(".xml"):
        zeilen.append(f"{name}\t{wurzel / 'ds2_annotation'}")
    if "/dataset2/audio/" in name and name.endswith(".wav"):
        b = name.rsplit("/", 1)[1]
        if re.search(r"_G_V_(bending|slide)|vibrato|_G_fret_0-20|Lick\d+_[FKM](?!N)", b):
            zeilen.append(f"{name}\t{wurzel / 'ds2_audio'}")
Path(aus).write_text("\n".join(zeilen) + "\n", encoding="utf-8")
print(len(zeilen))
