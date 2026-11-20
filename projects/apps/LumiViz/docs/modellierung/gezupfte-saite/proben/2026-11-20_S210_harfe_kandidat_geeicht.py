"""S210 - Harfe: der Kandidat aus S209 (Boden 12 -> 2 mm, Muendung 2,0, Bassende eingespannt) mit dem am freien Boden
neu geeichten Holz (`2026-11-20_S210_harfe_eichung.py`), gegen den Kandidaten mit dem Holz aus S197 und den Ausgang.
Je Fassung ohne Saiten und mit Saiten c = 0,2. Nutzt das Suchgeruest aus S209 (dessen eingefrorene Basis).

Aufruf vom Projekt-Root: python -X utf8 -B <diese datei> [fassung ...]
"""
import copy
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent / "2026-11-19_S209_harfe_suche"))
argv, sys.argv = sys.argv, sys.argv[:1]
import harfe_abtasten2 as a2  # noqa: E402
import harfe_raster1 as r1  # noqa: E402
import harfe_suche as s  # noqa: E402

# Holz des Bodens: Dichte kg/m3, E quer zum Boden (laengs der Faser), E laengs des Bodens, G in Pa
HOLZ = {"S197": (500.0, 7.3e9, 0.855e9, 0.45e9),              # geeicht mit 10 -> 3 mm
        "geeicht": (456.7, 4.771e9, 0.7678e9, 0.3874e9),       # 12 -> 2 mm, Dichte aus der Masse 1,375 kg
        "geeicht500": (500.0, 5.226e9, 0.8437e9, 0.4288e9)}    # 12 -> 2 mm, Dichte 500 (Masse 1,505 kg)


def holz(plan, name, leisten_mit=True):
    """Setzt das Holz des Bodens. `leisten_mit` = die zwei Fichtenleisten tragen dasselbe Holz (wie bisher); sonst
    behalten sie das Holz aus S197 als eigener Werkstoff."""
    p = copy.deepcopy(plan)
    d, ex, ey, g = HOLZ[name]
    alt = next(m for m in p["materials"] if m["name"] == p["plate"]["material"])
    if not leisten_mit:
        eigen = copy.deepcopy(alt)
        eigen["name"] = "spruce bars"
        p["materials"].append(eigen)
        for r in p["ribs"]:
            if r["material"] == alt["name"]:
                r["material"] = "spruce bars"
    alt.update({"density": d, "ex": ex, "ey": ey, "gxy": g})
    return p


def kandidat(plan, bass=0.012, diskant=0.002, mu=2.0, fest=True):
    p = a2.dicke(plan, bass, diskant)
    p["cavity"]["endCorrection"] = mu
    return s.bassende(p, 1e12, 1e7) if fest else p


FASSUNGEN = {
    "ausgang": lambda B: B,
    "kandidat_S197": lambda B: kandidat(B),
    "kandidat_geeicht": lambda B: holz(kandidat(B), "geeicht"),
    "kandidat_geeicht_leisten_alt": lambda B: holz(kandidat(B), "geeicht", leisten_mit=False),
    "kandidat_geeicht500": lambda B: holz(kandidat(B), "geeicht500"),
    "kandidat_geeicht_bass_gestuetzt": lambda B: holz(kandidat(B, fest=False), "geeicht"),
}

if __name__ == "__main__":
    B = s.basis()
    print("Fassung | Moden mit Luft (Anteil) | Lage S(1,2)..S(1,6) % | A Saite 33..22 Modell - Bell dB | halten")
    for name in argv[1:] or FASSUNGEN:
        r1.beide(FASSUNGEN[name](B), name)
