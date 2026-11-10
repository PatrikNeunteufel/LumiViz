"""Die Hoerlisten aus InstrumentsCalculation nachregeln (S186, PN1 a, PN2 a, KZ1 a).

IC hat jeden Klang in einem Schritt angeglichen: Rohton messen, mit einem festen Faktor auf den Pegel der Liste. Das
absolute Gate (−70 LUFS) macht die integrierte Lautheit leiser Rohtoene aber vom Massstab abhaengig; 18 Klaenge lagen
danach 1,2 bis 8,8 LU unter dem Pegel (Befund `docs/modellierung/anbindung-ic/2026-11-10_S186_lv5_pegel_der_hoerlisten.md`).

Je Liste:
- **PN2 a:** ein stiller Klang (Rohton ganz unter −70 LUFS, `lufs_vorher` −300) kommt aus der Liste — mit seinem Messwert,
  den Punkten, die ihn nennen, und seiner Datei. Er wird gemeldet (Befund am Modell, nicht an der Liste).
- **KZ1 a:** ein Klang unter 0,4 s wird hinten mit Stille auf 0,4 s aufgefuellt, damit BS.1770-4 ihn messen kann.
- **PN1 a:** jeder Klang wird nachgeregelt — verstaerken, neu messen, bis hoechstens 0,05 LU vom Pegel. Der Pegel sinkt,
  wenn sonst eine Spitze ueber 0,99 kaeme (wie IC: auf 0,1 LU abgerundet).
Eine Liste, an der nichts zu tun ist, bleibt Byte fuer Byte, wie sie ist.

Die Messung ist BS.1770-4 wie `lumi::audio::messeLautheit` und HoerTest_Helper (`Lautheit.cpp`): K-Filter, Bloecke
400 ms mit Schritt 100 ms ab dem ersten Wert, Gate −70 LUFS absolut und −10 LU relativ.

Aufruf (aus dem Repo-Wurzelordner):
    python -I tools/hoertest/ic_nachregeln.py <Hoerort>\\IC [--repo hoertest/listen/ic] [--trocken]
Mit `--repo` kommen `liste.json` und `klaenge.sha256` danach auch in den Ordner der versionierten Listen (AU3-1 a).
Danach pruefen: `Hoertest.exe --liste-pruefen <Hoerort>\\IC\\<liste>\\liste.json` (0 Fehler, 0 ausserhalb).
"""
import argparse
import hashlib
import json
import math
import shutil
import struct
import sys
from pathlib import Path

import numpy as np
from scipy.signal import lfilter

SPITZE_HOECHSTENS = 0.99  # wie IC (`Sammlung.cpp`, kSpitzeHoechstens)
GENAU_LU = 0.05  # wie die Schreiber von LumiViz (`test_LoopStab.cpp`)
KURZ_S = 0.4
VERMERK = "# Nachgeregelt S186 (PN1 a, PN2 a, KZ1 a) mit: python -I tools/hoertest/ic_nachregeln.py (LumiViz)"


# --- WAV -----------------------------------------------------------------------------------------------------------

def lies_wav(pfad):
    """Liest eine WAV; gibt die Bloecke vor und nach `data`, Format und die Werte (float64, Frames x Kanaele)."""
    b = Path(pfad).read_bytes()
    if b[:4] != b"RIFF" or b[8:12] != b"WAVE":
        raise ValueError("keine WAV: %s" % pfad)
    bloecke = []
    pos = 12
    fmt = None
    while pos + 8 <= len(b):
        name = b[pos:pos + 4]
        groesse = struct.unpack_from("<I", b, pos + 4)[0]
        inhalt = b[pos + 8:pos + 8 + groesse]
        bloecke.append((name, inhalt))
        if name == b"fmt ":
            tag, kanaele, rate, _, _, bits = struct.unpack_from("<HHIIHH", inhalt, 0)
            if tag == 0xFFFE:
                tag = struct.unpack_from("<H", inhalt, 24)[0]
            fmt = (tag, kanaele, rate, bits)
        pos += 8 + groesse + (groesse & 1)
    tag, kanaele, rate, bits = fmt
    if (tag, bits) != (3, 32):
        raise ValueError("erwartet 32-bit float: %s %r" % (pfad, fmt))
    daten = next(i for n, i in bloecke if n == b"data")
    werte = np.frombuffer(daten, dtype="<f4").astype(np.float64).reshape(-1, kanaele)
    return bloecke, rate, werte


def schreibe_wav(pfad, bloecke, werte):
    """Schreibt mit denselben Bloecken wie gelesen, nur `data` neu (32-bit float)."""
    daten = werte.astype("<f4").tobytes()
    teile = []
    for name, inhalt in bloecke:
        if name == b"data":
            inhalt = daten
        teile.append(name + struct.pack("<I", len(inhalt)) + inhalt + (b"\0" if len(inhalt) & 1 else b""))
    rumpf = b"WAVE" + b"".join(teile)
    Path(pfad).write_bytes(b"RIFF" + struct.pack("<I", len(rumpf)) + rumpf)


# --- BS.1770-4 -----------------------------------------------------------------------------------------------------

def k_filter(rate):
    fs = float(rate)
    f0, g, q = 1681.974450955533, 3.999843853973347, 0.7071752369554196
    k = math.tan(math.pi * f0 / fs)
    vh = 10 ** (g / 20)
    vb = vh ** 0.4996667741545416
    a0 = 1 + k / q + k * k
    s1 = ([(vh + vb * k / q + k * k) / a0, 2 * (k * k - vh) / a0, (vh - vb * k / q + k * k) / a0],
          [1.0, 2 * (k * k - 1) / a0, (1 - k / q + k * k) / a0])
    f0, q = 38.13547087602444, 0.5003270373238773
    k = math.tan(math.pi * f0 / fs)
    a0 = 1 + k / q + k * k
    s2 = ([1.0, -2.0, 1.0], [1.0, 2 * (k * k - 1) / a0, (1 - k / q + k * k) / a0])
    return s1, s2


def lufs(m):
    return -0.691 + 10 * math.log10(m) if m > 0 else -math.inf


def lautheit(werte, rate):
    """Integrierte Lautheit in LUFS; None, wenn kein Block (unter 0,4 s) oder alle Bloecke unter −70 LUFS."""
    s1, s2 = k_filter(rate)
    quadrat = np.zeros(werte.shape[0])
    for c in range(werte.shape[1]):
        y = lfilter(*s2, lfilter(*s1, werte[:, c]))
        quadrat += y * y
    block, schritt = round(0.4 * rate), round(0.1 * rate)
    if len(quadrat) < block:
        return None
    kum = np.concatenate(([0.0], np.cumsum(quadrat)))
    ab = np.arange(0, len(quadrat) - block + 1, schritt)
    mittel = (kum[ab + block] - kum[ab]) / block
    mit_lufs = np.array([lufs(m) for m in mittel])
    ueber = mittel[mit_lufs > -70.0]
    if ueber.size == 0:
        return None
    rel = mittel[mit_lufs > lufs(ueber.mean()) - 10.0]
    return lufs(rel.mean())


# --- je Liste ------------------------------------------------------------------------------------------------------

def als_float32(werte):
    return werte.astype(np.float32).astype(np.float64)


def regle(werte, rate, ziel):
    """Verstaerkt, bis die Lautheit der float32-Datei hoechstens GENAU_LU vom Ziel liegt."""
    ist = lautheit(werte, rate)
    aus = werte
    for _ in range(12):
        if abs(ist - ziel) <= GENAU_LU:
            return aus, ist
        aus = als_float32(aus * 10 ** ((ziel - ist) / 20))
        ist = lautheit(aus, rate)
    raise RuntimeError("nachregeln konvergiert nicht (%.2f statt %.2f LUFS)" % (ist, ziel))


def lufs_vorher(liste, klang):
    for m in liste.get("messwerte", []):
        if m.get("achsen") == klang.get("achsen") and m.get("fassung") == klang.get("fassung"):
            for w in m.get("werte", []):
                if w.get("name") == "lufs_vorher":
                    return w.get("wert")
    return None


def bearbeite(ordner, trocken):
    liste_pfad = ordner / "liste.json"
    liste = json.loads(liste_pfad.read_text(encoding="utf-8"))
    pegel = float(liste["pegel"]["lufs"])
    klaenge = []  # (klang, bloecke, rate, werte, aufgefuellt)
    still = []
    for k in liste["klaenge"]:
        bloecke, rate, werte = lies_wav(ordner / k["datei"])
        vorher = lufs_vorher(liste, k)
        kurz = werte.shape[0] < round(KURZ_S * rate)
        if not kurz and lautheit(werte, rate) is None:
            if vorher is not None and vorher > -299:
                raise RuntimeError("%s: still in der Datei, aber lufs_vorher %s" % (k["id"], vorher))
            still.append(k["id"])
            continue
        if kurz:  # KZ1 a
            werte = np.vstack((werte, np.zeros((round(KURZ_S * rate) - werte.shape[0], werte.shape[1]))))
            if lautheit(werte, rate) is None:
                still.append(k["id"])
                continue
        klaenge.append((k, bloecke, rate, werte, kurz))

    zu_tun = bool(still) or any(kurz or abs(lautheit(w, r) - pegel) > GENAU_LU for _, _, r, w, kurz in klaenge)
    if not zu_tun:
        print("%s: nichts zu tun (Pegel %.1f LUFS, %d Klaenge)" % (ordner.name, pegel, len(klaenge)))
        return False

    # der Pegel: hoechstens der bisherige, so tief, dass keine Spitze ueber SPITZE_HOECHSTENS kommt
    ziel = pegel
    while True:
        geregelt = [regle(w, r, ziel) for _, _, r, w, _ in klaenge]
        spitze = max(float(np.abs(w).max()) for w, _ in geregelt)
        if spitze <= SPITZE_HOECHSTENS:
            break
        ziel = math.floor((ziel + 20 * math.log10(SPITZE_HOECHSTENS / spitze)) * 10) / 10

    geaendert = []
    for (k, _, _, alt, kurz), (neu, ist) in zip(klaenge, geregelt):
        if kurz or not np.array_equal(alt, neu):
            geaendert.append(k["id"])
    abweichung = max(abs(ist - ziel) for _, ist in geregelt)
    print("%s: Pegel %.1f -> %.1f LUFS, Spitze %.2f dBFS, %d von %d Klaengen neu, hoechstens %.3f LU vom Pegel; "
          "aufgefuellt (KZ1 a): %s; still, herausgenommen (PN2 a): %s" % (
              ordner.name, pegel, ziel, 20 * math.log10(spitze), len(geaendert), len(klaenge), abweichung,
              ", ".join(k["id"] for k, _, _, _, kurz in klaenge if kurz) or "-", ", ".join(still) or "-"))
    if trocken:
        return True

    # Dateien
    for (k, bloecke, _, _, _), (neu, _) in zip(klaenge, geregelt):
        if k["id"] in geaendert:
            schreibe_wav(ordner / k["datei"], bloecke, neu)
    weg = set(still)
    for k in liste["klaenge"]:
        if k["id"] in weg:
            (ordner / k["datei"]).unlink()

    # Liste: Pegel, ohne die stillen Klaenge, ihre Messwerte und die Punkte, die sie nennen
    achsen_weg = [(k.get("achsen"), k.get("fassung")) for k in liste["klaenge"] if k["id"] in weg]
    liste["pegel"]["lufs"] = ziel
    liste["klaenge"] = [k for k in liste["klaenge"] if k["id"] not in weg]
    liste["messwerte"] = [m for m in liste.get("messwerte", []) if (m.get("achsen"), m.get("fassung")) not in achsen_weg]
    punkte_weg = [p["id"] for p in liste["punkte"]
                  if p.get("klang") in weg or any(x in weg for x in p.get("klaenge", []))]
    liste["punkte"] = [p for p in liste["punkte"] if p["id"] not in punkte_weg]
    if punkte_weg:
        print("  Punkte herausgenommen: %s" % ", ".join(punkte_weg))
    liste_pfad.write_text(json.dumps(liste, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")

    # Abdruecke: der Kopf bleibt, darunter der Vermerk, dann je Klang in der Reihenfolge der Liste
    summen_pfad = ordner / "klaenge.sha256"
    kopf = [z for z in summen_pfad.read_text(encoding="utf-8").splitlines() if z.startswith("#") and z != VERMERK]
    zeilen = kopf + [VERMERK]
    for k in liste["klaenge"]:
        zeilen.append("%s  %s" % (hashlib.sha256((ordner / k["datei"]).read_bytes()).hexdigest(), k["datei"]))
    summen_pfad.write_text("\n".join(zeilen) + "\n", encoding="utf-8", newline="\n")
    return True


def main():
    a = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    a.add_argument("hoerort", type=Path, help="Ordner mit den IC-Listen (…\\cmake\\samples\\Hoerproben\\IC)")
    a.add_argument("--repo", type=Path, help="Ordner der versionierten IC-Listen (hoertest/listen/ic)")
    a.add_argument("--trocken", action="store_true", help="nur rechnen und melden, nichts schreiben")
    arg = a.parse_args()
    for ordner in sorted(p.parent for p in arg.hoerort.glob("*/liste.json")):
        if (ordner / "abgeloest.txt").exists():  # LG1 a (S186): durch eine Liste von LumiViz abgeloest, nicht gehoert
            print("%s: abgeloest, uebersprungen" % ordner.name)
            continue
        if bearbeite(ordner, arg.trocken) and arg.repo and not arg.trocken:
            ziel = arg.repo / ordner.name
            if not ziel.is_dir():
                sys.exit("kein Ordner %s im Repo" % ziel)
            for datei in ("liste.json", "klaenge.sha256"):
                shutil.copyfile(ordner / datei, ziel / datei)


if __name__ == "__main__":
    main()
