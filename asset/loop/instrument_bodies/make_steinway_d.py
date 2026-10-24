"""Erzeugt den Bauplan des Resonanzbodens eines Steinway D aus RT-0425 (S147, Stufe 31c-1;
Entscheide Patrik BM1 a, BM2 a, BM3 a, GB1 a).

    python -I asset/loop/instrument_bodies/make_steinway_d.py <RT-0425.pdf> [--ziel <ordner>] [--bild <datei.png>]
    python -I asset/loop/instrument_bodies/make_steinway_d.py <RT-0425.pdf> --maesstro <MAESSTRO.pdf> [--ziel <ordner>] [--bild <datei.png>]

Mit `--maesstro` (S150, PF1 a, PF2 a) entsteht statt des Steinway der Pruefstein von MAESSTRO
(Elie, Cotte & Boutillon, Acta Acustica 6 (2022) 30, „RP"): `maesstro_rp.lvinstbodyplan`, von
sich aus unter den Testdaten `projects/apps/LumiViz/tests/unit/UnitTests/koerper/` (nicht
mitgeliefert). Umriss, Rippenlinien und Stegpunkte wie im Steinway; Platte 8 mm, zwei Stege
(Bass 60 mm fuer die Tasten 21–40, Diskant 20 mm fuer 41–108, je 32 mm breit; der Haken von
RT-0425 entfaellt), Rippen ohne Verjuengung 27 → 17 mm hoch und 25 → 21 mm breit, Faser 135
Grad, Sitka und Buche aus Tab. 2 mit η = 2 %. `--bild` legt den Pruefstein dann auf Abb. 4 (a)
(Umriss orange, Rippen schwarz, Bass-Steg rot, Diskant-Steg lila, Rib15 aus der Liste der
Oberflaeche hellblau) und misst Zahl, Winkel und Abstand der Rippen von MAESSTRO im Bild.
Derselbe Lauf schreibt die drei Varianten aus MAESSTRO §3.2 (S151, Stufe 31c-8, VA1 a) daneben:
`maesstro_mp1` (Platte 16 mm), `maesstro_mp2` (doppelter Rippenabstand — jede zweite Rippe ab
der Bass-Rippe, 9 von 17 wie Abb. 4 b), `maesstro_mp3` (ohne Rippen); mit `--bild` liegt MP2
zusaetzlich auf Abb. 4 (b) (`<bild>_mp2.png`).

Quelle: J. Chabassier, M. Duruflé, „Physical parameters for piano modeling", Inria RT-0425
(2012), hal-00688679 — die PDF liegt nur lokal unter
`projects/apps/LumiViz/docs/modellierung/hammer-saite/quellen/` (nicht im Repo). Gebraucht
werden `pdftotext` und `pdfimages` (Poppler), dazu numpy, scipy, Pillow.

Schreibt `steinway_d.lvinstbodyplan` (Format `lumiviz-instrument-body-plan`, Fassung 2) neben
dieses Skript oder nach `--ziel`, dazu die zwei Glasboeden aus dem Steinway (S151, Stufe 31c-8,
GB1 a, GL1 a): `steinway_d_glass` (Dicke wie Holz) und `steinway_d_glass_same_mass` (gleiche Masse
der Platte). Kein Zufall: zwei Laeufe ergeben dieselbe Datei.
**Massgebend ist die Datei im Repo, nicht ein neuer Lauf** — zuerst mit `--ziel` in eine Kopie
schreiben und vergleichen.

Was woher kommt:

- **Umriss:** Abb. 2 (a), die einzige Ansicht ohne Achsen — (c) und (d) schneiden den Boden an
  den Achsen ab (unten rechts liegt B7 bei y = -0,03 m). (a) wird so gelegt, dass die
  Stegpunkte aus Anhang A auf ihrem Steg (magenta) liegen und der Rand von (c), wo er nicht
  abgeschnitten ist, auf ihrem Rand (zwei Stuetzen; Massstab und Versatz je Achse).
- **Achsen von (c) und (d):** 290 px/m in beiden Richtungen; x = 0,2 m beim Teilstrich 276,5 px,
  y = 0 bei 617,5 px (Teilstriche rechts alle 58 px = 0,2 m). Ein Pixel rund 3,4 mm.
- **Dicke der Platte:** Abb. 2 (c), Farbe gegen die Farbskala des Bildes (6–10 mm; die
  Bildunterschrift sagt „AU", die Skala traegt Zahlen), je Zelle von 2 cm der Median; Zellen
  unter Rippen und Steg aus den Nachbarn, Zellen ausserhalb von (c) vom naechsten Wert.
  §2.1 nennt „between 6 mm and 9 mm"; das Bild zeigt in der Mitte bis 9,9 mm.
- **Rippen und Steg:** Abb. 2 (d), Farbe gegen ihre Skala (0,01–0,09 m) = Dicke **gesamt**
  (Platte und Leiste, so rechnet RT-0425); die Hoehe der Leiste ist gesamt minus Platte an
  derselben Stelle. Rippen: Richtung aus der schaerfsten Haeufung, je Rippe das laengste
  Stueck (der Steg kreuzt), neun Stuetzpunkte. Steg: ein Zug vom Ende des Bass-Arms ueber den
  Haken bis ins Diskant; wo (d) ihn abschneidet, aus (a) verlaengert (Hoehe dort wie der
  letzte Wert). Ausreisser (Kreuzungen) per gleitendem Median.
- **Breiten:** RT-0425 nennt keine. Rippen 25 mm (Bass) bis 21 mm (Diskant), Steg 32 mm —
  MAESSTRO Tab. 1 (Elie, Cotté & Boutillon 2022), BM2 a.
- **Holz:** Chabassier, Joly & Chaigne 2013 (JASA) Tab. I, „Spruce for the table and the
  ribs, Beech for the bridge" — die zweite Fichtenzeile von RT-0425 §2.1. Verlustfaktor 0 =
  keine Angabe (die modale Daempfung nach RT-0425 Gl. 1 legt 31c-5 fest).
- **Faser:** quer zu den Rippen (Annahme: im Klavierbau laufen Rippen quer zur Faser);
  MAESSTRO setzt fuer seinen vereinfachten Steinway D 135 Grad.
- **Stegpunkte:** RT-0425 Anhang A (x0, y0) fuer C1 … B7 (Anhang B traegt dieselben Werte).
  A0, A#0, B0 und C8 fehlen dort: entlang des Stegs verlaengert, im Abstand der drei
  naechsten Noten, Quelle „extended along the bridge".
- **Hoerorte** (S148, BM6 a, BM6-1 a): ein Stereopaar `left`/`right` 12 cm ueber den Saiten
  (die Readme von *Salamander*: „~12cm above the strings"), ueber den Stegpunkten der Tasten
  am ersten und dritten Viertel der Tastatur (MIDI 43 und 86). z ueber der Mittelflaeche der
  Platte: halbe Plattendicke plus Hoehe des Stegs dort plus 12 cm.
"""
import copy
import json
import re
import subprocess
import sys
import tempfile
from collections import deque
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage, optimize, spatial

# --- Eichung von Abb. 2 (c) und (d) ---------------------------------------------------
PX_JE_M = 290.0
X_BEZUG_PX, X_BEZUG_M = 276.5, 0.2
Y_NULL_PX = 617.5
RAHMEN = (233, 55, 623, 621)  # x0, y0, x1, y1 innen
SKALA_X = 676
RASTER_M = 0.02
RAND_DICKE = 0.006  # §2.1: am Rand 6 mm
MIKROFON_UEBER_SAITEN = 0.12  # Salamander: „~12cm above the strings" (BM6-1 a)

NOTEN = {"C": 0, "Cd": 1, "D": 2, "Dd": 3, "E": 4, "F": 5, "Fd": 6, "G": 7, "Ab": 8, "A": 9, "Ad": 10, "B": 11}


def taste(name):
    m = re.match(r"^([A-G](?:d|b)?)(\d)$", name)
    return 12 * (int(m.group(2)) + 1) + NOTEN[m.group(1)]


# --- Stegpunkte aus Anhang A ------------------------------------------------------------
def stegpunkte(pdf):
    zahl = re.compile(r"^-?\d\.\d\d$")
    note = re.compile(r"^[A-G](?:d|b)?\d$")
    punkte = {}
    for seite in range(11, 19):  # Anhang A
        text = subprocess.run(["pdftotext", "-layout", "-f", str(seite), "-l", str(seite), pdf, "-"],
                              capture_output=True, text=True, encoding="latin-1", check=True).stdout
        block = None
        bloecke = []
        for zeile in text.splitlines():
            teile = zeile.split()
            if "Note" in teile:
                block = {"noten": [t for t in teile[teile.index("Note") + 1:] if note.match(t)], "reihen": []}
                bloecke.append(block)
            elif block is not None:
                block["reihen"].append([t for t in teile if zahl.match(t)])
        for b in bloecke:
            reihen = [r for r in b["reihen"] if len(r) == len(b["noten"]) and b["noten"]]
            if len(reihen) < 2:
                continue
            for n, x, y in zip(b["noten"], reihen[-2], reihen[-1]):
                punkte[taste(n)] = (float(x), float(y))
    if sorted(punkte) != list(range(24, 108)):
        raise SystemExit(f"Anhang A: erwartet C1 … B7 (84 Noten), gefunden {len(punkte)}")
    return punkte


# --- Bilder -----------------------------------------------------------------------------
def bilder(pdf, ordner):
    subprocess.run(["pdfimages", "-f", "9", "-l", "9", "-png", pdf, str(Path(ordner) / "b")], check=True)

    def lade(oben, unten):
        a = np.asarray(Image.open(Path(ordner) / oben).convert("RGB")).astype(int)
        b = np.asarray(Image.open(Path(ordner) / unten).convert("RGB")).astype(int)
        return np.vstack([a, b])

    a = np.asarray(Image.open(Path(ordner) / "b-000.png").convert("RGB")).astype(int)
    return a, lade("b-002.png", "b-003.png"), lade("b-004.png", "b-005.png")


def px_zu_m(px, py):
    return X_BEZUG_M + (px - X_BEZUG_PX) / PX_JE_M, (Y_NULL_PX - py) / PX_JE_M


def m_zu_px(x, y):
    return X_BEZUG_PX + (x - X_BEZUG_M) * PX_JE_M, Y_NULL_PX - y * PX_JE_M


def innen_rahmen(shape):
    m = np.zeros(shape, bool)
    x0, y0, x1, y1 = RAHMEN
    m[y0:y1 + 1, x0:x1 + 1] = True
    return m


def farbtabelle(bild, y0, y1, wert):
    farben = np.array([bild[py, SKALA_X - 3:SKALA_X + 4].mean(axis=0) for py in range(y0, y1 + 1)])
    return farben, np.array([wert(py) for py in range(y0, y1 + 1)])


def werte_aus_farbe(bild, farben, werte, maske, grenze=30.0):
    aus = np.full(bild.shape[:2], np.nan)
    ys, xs = np.nonzero(maske)
    pix = bild[ys, xs].astype(float)
    for k in range(0, len(ys), 20000):
        p = pix[k:k + 20000]
        d = ((p[:, None, :] - farben[None, :, :]) ** 2).sum(axis=2)
        i = d.argmin(axis=1)
        ok = np.sqrt(d[np.arange(len(i)), i]) < grenze
        aus[ys[k:k + 20000], xs[k:k + 20000]] = np.where(ok, werte[i], np.nan)
    return aus


def randzug(maske):
    ys, xs = np.nonzero(maske)
    start = (int(xs[np.argmin(ys * 10000 + xs)]), int(ys.min()))
    nach = [(1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1)]
    h, w = maske.shape
    zug = [start]
    aktuell, richtung = start, 6
    for _ in range(200000):
        for k in range(8):
            r = (richtung + 6 + k) % 8
            nx, ny = aktuell[0] + nach[r][0], aktuell[1] + nach[r][1]
            if 0 <= nx < w and 0 <= ny < h and maske[ny, nx]:
                aktuell, richtung = (nx, ny), r
                break
        if aktuell == start:
            break
        zug.append(aktuell)
    return zug


def vereinfacht(punkte, toleranz):
    if len(punkte) < 3:
        return punkte
    a, b = np.array(punkte[0], float), np.array(punkte[-1], float)
    ab = b - a
    n = np.hypot(*ab)
    abst = [abs(ab[0] * (p[1] - a[1]) - ab[1] * (p[0] - a[0])) / n if n > 0 else np.hypot(p[0] - a[0], p[1] - a[1])
            for p in punkte[1:-1]]
    k = int(np.argmax(abst)) + 1
    if abst[k - 1] > toleranz:
        return vereinfacht(punkte[:k + 1], toleranz)[:-1] + vereinfacht(punkte[k:], toleranz)
    return [punkte[0], punkte[-1]]


def gleitender_median(werte, breite):
    v = np.array(werte, float)
    aus = v.copy()
    h = breite // 2
    for k in range(len(v)):
        fenster = v[max(0, k - h):k + h + 1]
        fenster = fenster[~np.isnan(fenster)]
        if len(fenster):
            aus[k] = np.median(fenster)
    return aus


# --- Platte aus (c) ---------------------------------------------------------------------
def platte_aus_c(c):
    weiss = (c > 245).all(axis=2)
    rahmen = innen_rahmen(weiss.shape)
    sx, sy = (int(round(v)) for v in m_zu_px(1.3, 1.85))
    marke, _ = ndimage.label(weiss & rahmen)
    platte = rahmen & ~(marke == marke[sy, sx])
    platte = ndimage.binary_opening(platte, iterations=1)
    marke, n = ndimage.label(platte)
    platte = marke == 1 + np.argmax(ndimage.sum(platte, marke, range(1, n + 1)))
    fc, wc = farbtabelle(c, 54, 624, lambda py: 10e-3 - (py - 53.75) / 143.0 * 1e-3)
    werte = werte_aus_farbe(c, fc, wc, platte & ~weiss)
    return platte, werte


# --- Rippen und Steg aus (d) ------------------------------------------------------------
def rippen_aus_d(werte_d, maske):
    ys, xs = np.nonzero(maske)
    best = None
    for grad in np.arange(-50.0, -15.0, 0.25):  # sie steigen nach rechts: im Bild negativ
        w = np.radians(grad)
        quer = -xs * np.sin(w) + ys * np.cos(w)
        h, _ = np.histogram(quer, bins=np.arange(quer.min(), quer.max() + 2, 2.0))
        schaerfe = float((h.astype(float) ** 2).sum())
        if best is None or schaerfe > best[0]:
            best = (schaerfe, grad)
    w = np.radians(best[1])
    laengs = xs * np.cos(w) + ys * np.sin(w)
    quer = -xs * np.sin(w) + ys * np.cos(w)
    h, kanten = np.histogram(quer, bins=np.arange(quer.min() - 1, quer.max() + 3, 1.0))
    glatt = ndimage.uniform_filter1d(h.astype(float), 5)
    spitzen = []
    for k in range(2, len(glatt) - 2):
        if glatt[k] == glatt[k - 2:k + 3].max() and glatt[k] > 0.25 * glatt.max():
            if spitzen and k - spitzen[-1] < 10:
                if glatt[k] > glatt[spitzen[-1]]:
                    spitzen[-1] = k
            else:
                spitzen.append(k)
    leisten = []
    alle_ys, alle_xs = np.indices(maske.shape)
    alle_ys, alle_xs = alle_ys.ravel(), alle_xs.ravel()
    qa = -alle_xs * np.sin(w) + alle_ys * np.cos(w)
    la = alle_xs * np.cos(w) + alle_ys * np.sin(w)
    auf_rippe = np.zeros(maske.shape, bool)
    for k in spitzen:
        q = (kanten[k] + kanten[k + 1]) / 2
        auf = np.abs(quer - q) < 4
        l = np.sort(laengs[auf])
        if len(l) < 20:
            continue
        stuecke = np.split(l, np.nonzero(np.diff(l) > 25)[0] + 1)  # der Steg kreuzt: bis 25 px
        stueck = max(stuecke, key=lambda s: s[-1] - s[0])
        a, b = stueck[0], stueck[-1]
        punkte, gesamt = [], []
        for t in np.linspace(a, b, 9):
            px = t * np.cos(w) - q * np.sin(w)
            py = t * np.sin(w) + q * np.cos(w)
            fenster = auf & (np.abs(laengs - t) < 6)
            v = werte_d[ys[fenster], xs[fenster]]
            v = v[~np.isnan(v)]
            punkte.append(px_zu_m(px, py))
            gesamt.append(float(np.median(v)) if len(v) else np.nan)
        leisten.append({"punkte": punkte, "gesamt": gleitender_median(gesamt, 3)})
        auf_alle = (np.abs(qa - q) < 5) & (la > a - 30) & (la < b + 30)
        auf_rippe[alle_ys[auf_alle], alle_xs[auf_alle]] = True
    return -best[1], leisten, auf_rippe


def steg_aus_d(werte_d, kern, verbund):
    marke, n = ndimage.label(verbund, structure=np.ones((3, 3)))
    maske = marke == 1 + np.argmax(ndimage.sum(verbund, marke, range(1, n + 1)))
    ys, xs = np.nonzero(maske)
    menge = set(zip(xs.tolist(), ys.tolist()))

    def bfs(start):
        dist = {start: 0}
        q = deque([start])
        while q:
            p = q.popleft()
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    nb = (p[0] + dx, p[1] + dy)
                    if nb in menge and nb not in dist:
                        dist[nb] = dist[p] + 1
                        q.append(nb)
        return dist

    d0 = bfs((int(xs[0]), int(ys[0])))
    ende = max(d0, key=lambda p: (d0[p], p))
    d = bfs(ende)
    gruppen = {}
    for p, s in d.items():
        gruppen.setdefault(s // 6, []).append(p)
    linie, gesamt = [], []
    for k in sorted(gruppen):
        g = np.array(sorted(gruppen[k]))
        g = g[kern[g[:, 1], g[:, 0]]]
        if len(g) < 3:
            continue  # eine Kreuzung mit einer Rippe
        linie.append(px_zu_m(g[:, 0].mean(), g[:, 1].mean()))
        v = werte_d[g[:, 1], g[:, 0]]
        v = v[~np.isnan(v)]
        gesamt.append(float(np.median(v)) if len(v) else np.nan)
    return linie, list(gleitender_median(gesamt, 7))


# --- Abb. 2 (a) auflegen ----------------------------------------------------------------
def umriss_aus_a(a, punkte_m, rand_c_m):
    magenta = (a[:, :, 0] > 180) & (a[:, :, 2] > 180) & (a[:, :, 1] < 170)
    brett = ndimage.binary_fill_holes(ndimage.binary_opening(a.sum(axis=2) < 700, iterations=1))
    marke, n = ndimage.label(brett)
    brett = marke == 1 + np.argmax(ndimage.sum(brett, marke, range(1, n + 1)))
    rand = brett & ~ndimage.binary_erosion(brett)
    felder = (ndimage.distance_transform_edt(~magenta), ndimage.distance_transform_edt(~rand))

    def lage(t, pm):
        return t[0] * pm[:, 0] + t[1], -t[2] * pm[:, 1] + t[3]

    def kosten(t):
        summe = 0.0
        for pm, feld in zip((punkte_m, rand_c_m), felder):
            px, py = lage(t, pm)
            if not ((px >= 0) & (px < a.shape[1] - 1) & (py >= 0) & (py < a.shape[0] - 1)).all():
                return 1e9
            summe += float(ndimage.map_coordinates(feld, [py, px], order=1).mean())
        return summe

    t = optimize.minimize(kosten, [236.0, 12.0, 233.0, 468.0], method="Nelder-Mead",
                          options={"xatol": 1e-3, "fatol": 1e-4, "maxiter": 4000}).x
    zug = randzug(brett)
    umriss_px = vereinfacht(zug + [zug[0]], 0.6)[:-1]
    umriss = [((px - t[1]) / t[0], (t[3] - py) / t[2]) for px, py in umriss_px]
    ys_m, xs_m = np.nonzero(magenta)
    magenta_m = np.column_stack([(xs_m - t[1]) / t[0], (t[3] - ys_m) / t[2]])
    for pm, feld, name in ((punkte_m, felder[0], "Stegpunkte zum Steg von (a)"), (rand_c_m, felder[1], "Rand (c) zum Rand (a)")):
        px, py = lage(t, pm)
        dd = ndimage.map_coordinates(feld, [py, px], order=1) / t[0]
        print(f"  {name}: Median {np.median(dd) * 1e3:.1f} mm, groesster {dd.max() * 1e3:.1f} mm")
    return umriss, magenta_m


# --- Bauplan ----------------------------------------------------------------------------
def abstand_zur_linie(p, linie):
    best = np.inf
    for k in range(len(linie) - 1):
        a, b = np.array(linie[k]), np.array(linie[k + 1])
        ab = b - a
        t = np.clip(np.dot(np.array(p) - a, ab) / max(np.dot(ab, ab), 1e-12), 0.0, 1.0)
        best = min(best, float(np.hypot(*(np.array(p) - (a + t * ab)))))
    return best


def lage_auf_linie(p, linie):
    """Bogenlaenge des naechsten Punktes der Linie."""
    best, s_best, s = np.inf, 0.0, 0.0
    for k in range(len(linie) - 1):
        a, b = np.array(linie[k]), np.array(linie[k + 1])
        ab = b - a
        l = float(np.hypot(*ab))
        t = np.clip(np.dot(np.array(p) - a, ab) / max(l * l, 1e-12), 0.0, 1.0)
        d = float(np.hypot(*(np.array(p) - (a + t * ab))))
        if d < best:
            best, s_best = d, s + t * l
        s += l
    return s_best


def punkt_auf_linie(s, linie):
    for k in range(len(linie) - 1):
        a, b = np.array(linie[k]), np.array(linie[k + 1])
        l = float(np.hypot(*(b - a)))
        if s <= l or k == len(linie) - 2:
            return tuple(a + (b - a) * min(max(s, 0.0), l) / l)
        s -= l
    return tuple(linie[-1])


def wert_auf_linie(p, linie, werte):
    """Der Wert am naechsten Punkt der Linie, linear zwischen ihren Stuetzpunkten."""
    best, v_best = np.inf, werte[0]
    for k in range(len(linie) - 1):
        a, b = np.array(linie[k]), np.array(linie[k + 1])
        ab = b - a
        t = float(np.clip(np.dot(np.array(p) - a, ab) / max(np.dot(ab, ab), 1e-12), 0.0, 1.0))
        d = float(np.hypot(*(np.array(p) - (a + t * ab))))
        if d < best:
            best, v_best = d, werte[k] + t * (werte[k + 1] - werte[k])
    return v_best


def innen(umriss, p):
    drin = False
    n = len(umriss)
    for k in range(n):
        (ax, ay), (bx, by) = umriss[k], umriss[k - 1]
        if (ay > p[1]) != (by > p[1]) and p[0] < (bx - ax) * (p[1] - ay) / (by - ay) + ax:
            drin = not drin
    return drin


def runde(z, stellen=5):
    return float(round(z, stellen))


def baue(pdf, bildpfad=None):
    punkte = stegpunkte(pdf)
    with tempfile.TemporaryDirectory() as tmp:
        a, c, d = bilder(pdf, tmp)

    platte_px, werte_c = platte_aus_c(c)
    fd, wd = farbtabelle(d, 53, 615, lambda py: 0.09 - (py - 54.0) / 71.5 * 0.01)
    bunt_d = innen_rahmen(d.shape[:2]) & ~(d > 245).all(axis=2)
    werte_d = werte_aus_farbe(d, fd, wd, bunt_d)
    grob_steg = bunt_d & (werte_d >= 0.042)
    rippen_maske = bunt_d & (werte_d < 0.042) & ~ndimage.binary_dilation(grob_steg, iterations=2)
    rippenwinkel, rippen, auf_rippe = rippen_aus_d(werte_d, rippen_maske)
    steg_maske = ndimage.binary_opening(bunt_d & ~(auf_rippe & (werte_d < 0.045)), iterations=1)
    steg_kern = steg_maske & (ndimage.distance_transform_edt(steg_maske) >= 2.0)
    steg_verbund = ndimage.binary_closing(steg_kern, structure=np.ones((3, 3)), iterations=6) & innen_rahmen(d.shape[:2])
    steg, steg_gesamt = steg_aus_d(werte_d, steg_kern, steg_verbund)

    print("Abb. 2 (a) gelegt:")
    zug_c = randzug(platte_px)
    rand_c = np.array([px_zu_m(*p) for p in zug_c[::2]])
    rand_c = rand_c[(rand_c[:, 0] > 0.08) & (rand_c[:, 0] < 1.36) & (rand_c[:, 1] > 0.04) & (rand_c[:, 1] < 1.92)]
    p_m = np.array([punkte[k] for k in sorted(punkte)])
    umriss, magenta_m = umriss_aus_a(a, p_m, rand_c)

    # den Steg so legen, dass er am Ende des Bass-Arms beginnt und im Diskant endet
    if np.hypot(*(np.array(steg[0]) - p_m[-1])) < np.hypot(*(np.array(steg[-1]) - p_m[-1])):
        steg, steg_gesamt = steg[::-1], steg_gesamt[::-1]
    ende = np.array(steg[-1])
    richtung = ende - np.array(steg[-4])
    richtung /= np.hypot(*richtung)
    t = (magenta_m - ende) @ richtung
    quer = np.abs((magenta_m - ende) @ np.array([-richtung[1], richtung[0]]))
    weiter = magenta_m[(t > 0.005) & (quer < 0.06)]
    tw = (weiter - ende) @ richtung
    for k in range(1, 30):
        im = weiter[(tw > (k - 1) * 0.01) & (tw <= k * 0.01)]
        if len(im) < 3:
            break
        steg.append(tuple(im.mean(axis=0)))
        steg_gesamt.append(steg_gesamt[-1])
    steg_px = vereinfacht([m_zu_px(*p) for p in steg], 0.5)
    behalten = [min(range(len(steg)), key=lambda i: np.hypot(*(np.array(m_zu_px(*steg[i])) - q))) for q in steg_px]
    steg = [steg[i] for i in behalten]
    steg_gesamt = [steg_gesamt[i] for i in behalten]

    # Dickenraster ueber den ganzen Umriss
    um = np.array(umriss)
    x0, y0 = um.min(axis=0)
    x1, y1 = um.max(axis=0)
    nx, ny = int(np.ceil((x1 - x0) / RASTER_M)), int(np.ceil((y1 - y0) / RASTER_M))
    ursprung = (x0 + RASTER_M / 2, y0 + RASTER_M / 2)
    roh = np.full((ny, nx), np.nan)
    drin = np.zeros((ny, nx), bool)
    r = RASTER_M * PX_JE_M / 2
    for j in range(ny):
        for i in range(nx):
            xm, ym = ursprung[0] + i * RASTER_M, ursprung[1] + j * RASTER_M
            drin[j, i] = innen(umriss, (xm, ym))
            if not drin[j, i]:
                continue
            px, py = m_zu_px(xm, ym)
            ya, yb, xa, xb = int(py - r), int(py + r) + 1, int(px - r), int(px + r) + 1
            if ya < RAHMEN[1] or xa < RAHMEN[0] or yb > RAHMEN[3] or xb > RAHMEN[2]:
                continue
            feld = platte_px[ya:yb, xa:xb]
            v = werte_c[ya:yb, xa:xb][feld]
            v = v[~np.isnan(v)]
            if feld.mean() >= 0.5 and len(v) >= 3:
                roh[j, i] = float(np.median(v))
    idx = ndimage.distance_transform_edt(np.isnan(roh), return_distances=False, return_indices=True)
    voll = roh[idx[0], idx[1]]
    raster = np.where(drin & np.isnan(roh), ndimage.uniform_filter(voll, 3), roh)
    raster[~drin] = np.nan

    def plattendicke(p):
        i = int(round((p[0] - ursprung[0]) / RASTER_M))
        j = int(round((p[1] - ursprung[1]) / RASTER_M))
        i, j = min(max(i, 0), nx - 1), min(max(j, 0), ny - 1)
        v = raster[j, i]
        return RAND_DICKE if np.isnan(v) else float(v)

    def hoehen(punkte_l, gesamt):
        g = np.array(gesamt, float)
        if np.isnan(g).any():
            k = np.arange(len(g))
            g[np.isnan(g)] = np.interp(k[np.isnan(g)], k[~np.isnan(g)], g[~np.isnan(g)])
        return [runde(max(0.0, gk - plattendicke(p))) for p, gk in zip(punkte_l, g)]

    # Rippen von oben (Bass) nach unten (Diskant), Breite 25 → 21 mm (MAESSTRO Tab. 1)
    rippen.sort(key=lambda r: -np.mean([p[1] for p in r["punkte"]]))
    leisten_r = []
    for k, rp in enumerate(rippen):
        breite = 0.025 - 0.004 * k / max(1, len(rippen) - 1)
        leisten_r.append({
            "name": f"rib {k + 1}",
            "material": "spruce",
            "line": [[runde(x), runde(y)] for x, y in rp["punkte"]],
            "height": hoehen(rp["punkte"], rp["gesamt"]),
            "width": [runde(breite)] * len(rp["punkte"]),
        })
    leisten_s = [{
        "name": "bridge",
        "material": "beech",
        "line": [[runde(x), runde(y)] for x, y in steg],
        "height": hoehen(steg, steg_gesamt),
        "width": [0.032] * len(steg),
    }]

    # Stegpunkte: C1 … B7 aus Anhang A, A0 … B0 und C8 entlang des Stegs verlaengert
    s = {k: lage_auf_linie(punkte[k], steg) for k in punkte}
    bass = np.mean([abs(s[k + 1] - s[k]) for k in (24, 25, 26)])
    diskant = np.mean([abs(s[k] - s[k - 1]) for k in (105, 106, 107)])
    richt_bass = np.sign(s[24] - s[25])
    richt_diskant = np.sign(s[107] - s[106])
    alle = []
    for k in range(21, 109):
        if k in punkte:
            alle.append({"key": k, "x": punkte[k][0], "y": punkte[k][1], "source": "RT-0425 Appendix A"})
        else:
            ziel = s[24] + richt_bass * bass * (24 - k) if k < 24 else s[107] + richt_diskant * diskant * (k - 107)
            x, y = punkt_auf_linie(ziel, steg)
            alle.append({"key": k, "x": runde(x, 3), "y": runde(y, 3), "source": "extended along the bridge"})

    # Hoerorte (BM6-1 a): ein Stereopaar 12 cm ueber den Saiten (wie Salamander), ueber den
    # Stegpunkten der Tasten am ersten und dritten Viertel der Tastatur; z ueber der Mittelflaeche
    # der Platte — die Oberseite des Stegs liegt eine halbe Plattendicke plus die Steghoehe darueber
    hoerorte = []
    for name, anteil in (("left", 0.25), ("right", 0.75)):
        k = int(round(21 + anteil * (108 - 21)))
        p = next(q for q in alle if q["key"] == k)
        ort = (p["x"], p["y"])
        oberseite = plattendicke(ort) / 2.0 + wert_auf_linie(ort, leisten_s[0]["line"], leisten_s[0]["height"])
        hoerorte.append({
            "name": name, "x": p["x"], "y": p["y"], "z": runde(oberseite + MIKROFON_UEBER_SAITEN),
            "source": f"BM6-1 a: 12 cm above the strings (Salamander README: \"~12cm above the strings\"), "
                      f"over the bridge point of key {k}",
        })

    faser = round(rippenwinkel + 90.0, 1)
    plan = {
        "format": "lumiviz-instrument-body-plan",
        "version": 2,
        "title": "Steinway D (RT-0425)",
        "description": "Soundboard of a Steinway D after Chabassier & Durufle, RT-0425 (2012): outline, plate "
                       "thickness, ribs and bridge read from Fig. 2, bridge points from Appendix A.",
        "source": "J. Chabassier, M. Durufle, Physical parameters for piano modeling, Inria RT-0425 (2012), "
                  "Fig. 2 (a, c, d), Sect. 2.1, Appendix A; spruce and beech after Chabassier, Joly & Chaigne, "
                  "JASA 134 (2013), Table I; widths of ribs and bridge after Elie, Cotte & Boutillon, Acta "
                  "Acustica 6 (2022), Table 1. Generated by make_steinway_d.py.",
        "units": "SI: m, kg/m^3, Pa",
        "materials": [
            {"name": "spruce", "kind": "orthotropic", "density": 380.0, "ex": 11.0e9, "ey": 0.65e9, "gxy": 0.66e9,
             "gxz": 1.2e9, "gyz": 0.042e9, "nuxy": 0.26, "lossFactor": 0.0,
             "source": "Chabassier, Joly & Chaigne 2013, Table I (spruce for the table and the ribs); RT-0425 Sect. 2.1, second row"},
            {"name": "beech", "kind": "orthotropic", "density": 750.0, "ex": 13.7e9, "ey": 2.24e9, "gxy": 1.61e9,
             "gxz": 1.06e9, "gyz": 0.46e9, "nuxy": 0.3, "lossFactor": 0.0,
             "source": "Chabassier, Joly & Chaigne 2013, Table I (beech for the bridge); RT-0425 Sect. 2.1"},
        ],
        "plate": {
            "material": "spruce",
            "grainAngle": faser,
            "outline": [[runde(x), runde(y)] for x, y in umriss],
            "thickness": RAND_DICKE,
            "thicknessGrid": {
                "origin": [runde(ursprung[0]), runde(ursprung[1])],
                "step": RASTER_M,
                "nx": nx,
                "ny": ny,
                "values": [None if np.isnan(v) else runde(float(v), 6) for v in raster.ravel()],
            },
        },
        "ribs": leisten_r,
        "bridges": leisten_s,
        "bridgePoints": alle,
        "listeningPoints": hoerorte,
    }

    # Pruefung: jeder Stegpunkt auf dem Steg (halbe Breite), Bericht der Zahlen
    abst = [abstand_zur_linie((p["x"], p["y"]), steg) for p in alle]
    print(f"Umriss {len(umriss)} Punkte, Flaeche {flaeche(umriss):.3f} m²; Raster {nx} x {ny} je {RASTER_M} m")
    v = raster[~np.isnan(raster)]
    print(f"Platte {v.min() * 1e3:.2f} … {v.max() * 1e3:.2f} mm (Median {np.median(v) * 1e3:.2f})")
    hr = [h for l in leisten_r for h in l["height"]]
    print(f"Rippen {len(leisten_r)}, Winkel {rippenwinkel:.2f} Grad, Hoehe ueber der Platte {min(hr) * 1e3:.1f} … {max(hr) * 1e3:.1f} mm; Faser {faser} Grad")
    hs = leisten_s[0]["height"]
    print(f"Steg {len(steg)} Punkte, Laenge {sum(np.hypot(*(np.array(steg[k + 1]) - steg[k])) for k in range(len(steg) - 1)):.3f} m, Hoehe {min(hs) * 1e3:.1f} … {max(hs) * 1e3:.1f} mm")
    print(f"Stegpunkte {len(alle)}: Abstand zur Steglinie Median {np.median(abst) * 1e3:.1f} mm, groesster {max(abst) * 1e3:.1f} mm")
    for h in hoerorte:
        print(f"Hoerort {h['name']}: ({h['x']}, {h['y']}) m, z {h['z'] * 1e3:.1f} mm")
    if max(abst) > 0.016:
        raise SystemExit("Ein Stegpunkt liegt nicht auf dem Steg (mehr als die halbe Breite daneben).")

    if bildpfad:
        b = Image.fromarray(np.clip(d, 0, 255).astype(np.uint8)).convert("RGB")
        z = ImageDraw.Draw(b)
        z.line([m_zu_px(*p) for p in umriss] + [m_zu_px(*umriss[0])], fill=(0, 120, 255), width=1)
        z.line([m_zu_px(*p) for p in steg], fill=(0, 200, 0), width=1)
        for l in leisten_r:
            z.line([m_zu_px(*p) for p in l["line"]], fill=(0, 0, 255), width=1)
        for p in alle:
            px, py = m_zu_px(p["x"], p["y"])
            farbe = (255, 0, 255) if p["source"].startswith("RT") else (0, 160, 255)
            z.ellipse([px - 2, py - 2, px + 2, py + 2], outline=farbe)
        b.resize((b.width * 2, b.height * 2), Image.NEAREST).save(bildpfad)
    return plan


def flaeche(umriss):
    u = np.array(umriss)
    return abs(np.dot(u[:, 0], np.roll(u[:, 1], -1)) - np.dot(u[:, 1], np.roll(u[:, 0], -1))) / 2


# --- Glasboeden (S151, Stufe 31c-8, GB1 a, GL1 a, VA1 a) ------------------------------------
# Kalk-Natron-Floatglas, monolithisch, isotrop: Datenblaetter nach EN 572 (gegengelesen NGA FM05-12:
# 71,7 GPa, ν 0,22); η nach Cremer/Heckl ueber Irvine 2010 Tab. 7 (Spanne 0,6–2·10⁻³, die Mitte)
GLAS_DICHTE = 2500.0
GLAS_E = 70.0e9
GLAS_NU = 0.2
GLAS_ETA = 1.0e-3
GLAS_QUELLE = ("soda-lime float glass, monolithic: density, E and Poisson's ratio after European data sheets to "
               "EN 572 (NGA Glass Technical Paper FM05-12 (2023) gives 71.7 GPa and 0.22); loss factor 1e-3 after "
               "Cremer & Heckl via T. Irvine, Damping Properties of Materials (2010), Table 7, middle of 0.6-2e-3 "
               "(Ashby gives less). GL1 a.")


def dicke_im_raster(platte, p):
    """Die Plattendicke an p wie `plattendicke` in `baue`: die naechste Zelle des Rasters, ausserhalb
    der Wert `thickness`."""
    r = platte["thicknessGrid"]
    i = min(max(int(round((p[0] - r["origin"][0]) / r["step"])), 0), r["nx"] - 1)
    j = min(max(int(round((p[1] - r["origin"][1]) / r["step"])), 0), r["ny"] - 1)
    v = r["values"][j * r["nx"] + i]
    return platte["thickness"] if v is None else v


def glasboeden(steinway):
    """Zwei gedachte Instrumente (GB1 a, GL1 a): der Steinway D mit einer Platte aus Glas, Rippen und Steg
    bleiben Fichte und Buche. `steinway_d_glass` mit der Dicke des Holzbodens (9 → 6 mm),
    `steinway_d_glass_same_mass` mit dem Verlauf mal 380/2500 (gleiche Masse der Platte, 1,4 → 0,9 mm).
    Die Hoerorte behalten x, y; z folgt der halben Plattendicke. Bruch unter der Saitenlast ist nicht
    gerechnet; in der Daempfung fehlen Rand und Abstrahlung (DM1 a)."""
    holz = next(m for m in steinway["materials"] if m["name"] == steinway["plate"]["material"])
    g = GLAS_E / (2.0 * (1.0 + GLAS_NU))
    glas = {"name": "glass", "kind": "isotropic", "density": GLAS_DICHTE, "ex": GLAS_E, "ey": GLAS_E, "gxy": g,
            "gxz": g, "gyz": g, "nuxy": GLAS_NU, "lossFactor": GLAS_ETA, "source": GLAS_QUELLE}
    plaene = {}
    for name, faktor, titel, was in (
            ("steinway_d_glass", 1.0, "Steinway D with a glass soundboard",
             "the plate of soda-lime glass with the thickness of the wooden plate (6-10 mm)"),
            ("steinway_d_glass_same_mass", holz["density"] / GLAS_DICHTE, "Steinway D with a thin glass soundboard",
             "the plate of soda-lime glass with the mass of the wooden plate (thickness times 380/2500)")):
        plan = copy.deepcopy(steinway)
        platte = plan["plate"]
        platte["material"] = "glass"
        platte["thickness"] = runde(steinway["plate"]["thickness"] * faktor, 6)
        platte["thicknessGrid"]["values"] = [None if v is None else runde(v * faktor, 6)
                                             for v in steinway["plate"]["thicknessGrid"]["values"]]
        plan["materials"] = steinway["materials"] + [glas]
        for h, alt in zip(plan["listeningPoints"], steinway["listeningPoints"]):
            ort = (alt["x"], alt["y"])
            h["z"] = runde(alt["z"] - dicke_im_raster(steinway["plate"], ort) / 2.0 + dicke_im_raster(platte, ort) / 2.0)
        plan["title"] = titel
        plan["description"] = (f"Imagined instrument (GB1 a): the Steinway D of RT-0425 with {was}; ribs of spruce "
                               "and bridge of beech as in steinway_d. Fracture under the string load is not "
                               "computed; damping from the rim and from radiation is missing for the glass.")
        plan["source"] = ("steinway_d.lvinstbodyplan (make_steinway_d.py) with a glass plate after GL1 a; "
                          + GLAS_QUELLE.split(":", 1)[0] + ", values see material 'glass'. Generated by make_steinway_d.py.")
        plaene[name] = plan
    return plaene


def pruefe_glasboeden(steinway, plaene):
    """Bericht: Dicke und Masse der Platte je Glasboden gegen den Holzboden."""
    def platte(plan):
        p = plan["plate"]
        dichte = {m["name"]: m["density"] for m in plan["materials"]}[p["material"]]
        r = p["thicknessGrid"]
        werte = [v for v in r["values"] if v is not None]
        return min(werte), max(werte), sum(werte) * r["step"] ** 2 * dichte
    for name, plan in [("steinway_d", steinway)] + list(plaene.items()):
        d0, d1, m = platte(plan)
        z = ", ".join("%.1f" % (h["z"] * 1e3) for h in plan["listeningPoints"])
        print(f"{name}: Platte {d0 * 1e3:.2f} … {d1 * 1e3:.2f} mm, Masse der Platte (Raster) {m:.2f} kg, "
              f"Hoerorte z {z} mm")


# --- MAESSTRO-Pruefstein (S150, PF1 a, PF2 a) ---------------------------------------------
MAESSTRO_PLATTE = 0.008                  # Tab. 1: Panel 8 mm
MAESSTRO_STEG_BASS = 0.060               # Tab. 1: Bridge #1 (bass) 60 mm
MAESSTRO_STEG_DISKANT = 0.020            # Tab. 1: Bridge #2 (treble) 20 mm
MAESSTRO_STEG_BREITE = 0.032             # Tab. 1: beide 32 mm
MAESSTRO_RIPPE_HOEHE = (0.027, 0.017)    # Tab. 1: 27 (bass rib) bis 17 mm (treble rib)
MAESSTRO_RIPPE_BREITE = (0.025, 0.021)   # Tab. 1: 25 (bass rib) bis 21 mm (treble rib)
MAESSTRO_FASER = 135.0                   # Tab. 1
MAESSTRO_ETA = 0.02                      # Tab. 2: 2 % je Werkstoff
LETZTE_BASS_TASTE = 40                   # PF2 a: der Bass-Arm traegt 21–40 (Anhang A, Abb. 6)
# Abb. 4 (a), Liste der Rippen: „Rib15", Enden in mm (der Stichprobe nach PF1 a)
MAESSTRO_RIB15 = ((0.147, 1.694), (0.336, 1.850))
# PF3 a (S150): MAESSTRO hat 17 Rippen, RT-0425 16 — die siebzehnte liegt in der Diskant-Ecke, im
# Bild 23 px (rund 105 mm) hinter der sechzehnten (`--bild` misst die Abstaende); parallel zu
# unserer letzten Rippe, am Umriss so weit zurueck wie diese
MAESSTRO_ECKRIPPE_ABSTAND = 0.105


def schnitt_mit_umriss(a, richtung, umriss):
    """Die Parameter t, bei denen die Gerade a + t·richtung den Umriss schneidet, aufsteigend."""
    ts = []
    n = len(umriss)
    for k in range(n):
        p, q = np.array(umriss[k]), np.array(umriss[(k + 1) % n])
        m = np.column_stack([richtung, p - q])
        if abs(np.linalg.det(m)) < 1e-12:
            continue
        t, u = np.linalg.solve(m, p - a)
        if 0.0 <= u <= 1.0:
            ts.append(float(t))
    return sorted(ts)


def eckrippe(rippen, umriss, diskant_steg):
    """Die siebzehnte Rippe (PF3 a): parallel zur letzten, um MAESSTRO_ECKRIPPE_ABSTAND weiter von der
    vorletzten weg; das Stueck im Umriss, das den Diskant-Steg kreuzt, an beiden Enden so weit
    zurueck wie die letzte Rippe im Mittel vom Rand."""
    letzte, vorletzte = np.array(rippen[-1], float), np.array(rippen[-2], float)
    richtung = (letzte[-1] - letzte[0]) / np.hypot(*(letzte[-1] - letzte[0]))
    quer = np.array([-richtung[1], richtung[0]])
    if np.dot(np.mean(letzte, axis=0) - np.mean(vorletzte, axis=0), quer) < 0:
        quer = -quer
    a = letzte[0] + MAESSTRO_ECKRIPPE_ABSTAND * quer
    ts = schnitt_mit_umriss(a, richtung, umriss)
    steg = [tuple(p) for p in diskant_steg]
    stuecke = [(ts[i], ts[i + 1]) for i in range(0, len(ts) - 1, 2)]
    t0, t1 = min(stuecke, key=lambda s: abstand_zur_linie(tuple(a + 0.5 * (s[0] + s[1]) * richtung), steg))
    # wie weit die letzte Rippe laengs ihrer Linie vor dem Umriss endet
    tl = schnitt_mit_umriss(letzte[0], richtung, umriss)
    ende = float(np.dot(letzte[-1] - letzte[0], richtung))
    rein = max(t for t in tl if t <= 1e-9)
    raus = min(t for t in tl if t >= ende - 1e-9)
    zurueck = 0.5 * ((0.0 - rein) + (raus - ende))
    return [tuple(a + t * richtung) for t in np.linspace(t0 + zurueck, t1 - zurueck, 9)]


def punkt_verlaengert(s, linie):
    """Der Punkt bei Bogenlaenge s; vor dem Anfang und hinter dem Ende geradeaus weiter."""
    laenge = lage_auf_linie(linie[-1], linie)
    if s < 0.0:
        a, b = np.array(linie[0]), np.array(linie[1])
        return tuple(a + (b - a) / np.hypot(*(b - a)) * s)
    if s > laenge:
        a, b = np.array(linie[-2]), np.array(linie[-1])
        return tuple(b + (b - a) / np.hypot(*(b - a)) * (s - laenge))
    return punkt_auf_linie(s, linie)


def teilstueck(linie, sa, sb):
    """Das Stueck der Linie zwischen den Bogenlaengen sa < sb, mit den Endpunkten (auch ueber die Linie hinaus)."""
    stueck = [punkt_verlaengert(sa, linie)]
    if sa < 0.0 < sb:
        stueck.append(tuple(linie[0]))
    s = 0.0
    for k in range(1, len(linie)):
        s += float(np.hypot(*(np.array(linie[k]) - np.array(linie[k - 1]))))
        if sa < s < sb:
            stueck.append(tuple(linie[k]))
    stueck.append(punkt_verlaengert(sb, linie))
    return stueck


def maesstro(steinway):
    """Der Pruefstein von MAESSTRO (Elie, Cotte & Boutillon 2022, „RP"): Umriss, Rippenlinien und
    Stegpunkte wie im Steinway D aus RT-0425, darauf die Vereinfachungen aus Text und Tab. 1–2.

    Der Steg wird geteilt (PF2 a): RT-0425 zeichnet ihn mit einem Haken durchgehend, MAESSTRO
    Abb. 4 und 6 zeigen zwei Stege. Der Bass-Steg traegt die Tasten 21–40 (der Bass-Arm, auf dem
    Anhang A sie hat; Abb. 6 springt zwischen Index 17 und 18 = MIDI 40 und 41, Index 1 = C1), der
    Diskant-Steg 41–108. Jeder endet einen Tastenabstand hinter seiner aeussersten Taste; der Haken
    dazwischen entfaellt."""
    linie = [tuple(p) for p in steinway["bridges"][0]["line"]]
    punkte = {p["key"]: (p["x"], p["y"]) for p in steinway["bridgePoints"]}
    s = {k: lage_auf_linie(p, linie) for k, p in punkte.items()}
    bass = [k for k in punkte if k <= LETZTE_BASS_TASTE]
    diskant = [k for k in punkte if k > LETZTE_BASS_TASTE]
    # der Bass-Steg laeuft von Taste 40 (Anfang der Linie) zu A0, der Diskant-Steg von F2 zu C8; jedes der
    # vier Enden einen Tastenabstand hinter der aeussersten Taste (PF2 a), wo noetig ueber die Linie hinaus
    def schritt(k, nachbar):
        return abs(s[k] - s[nachbar])

    hoch, tief = max(bass), min(bass)  # 40 und 21
    bass_anfang = s[hoch] - schritt(hoch, hoch - 1)
    bass_ende = s[tief] + schritt(tief, tief + 1)
    erste, letzte = min(diskant), max(diskant)  # 41 und 108
    diskant_anfang = s[erste] - schritt(erste, erste + 1)
    diskant_ende = s[letzte] + schritt(letzte, letzte - 1)
    if not (max(s[k] for k in bass) < bass_ende < diskant_anfang < min(s[k] for k in diskant)):
        raise SystemExit("Steg: Bass- und Diskantteil liegen nicht getrennt auf der Linie.")
    steg_bass = teilstueck(linie, bass_anfang, bass_ende)
    steg_diskant = teilstueck(linie, diskant_anfang, diskant_ende)
    um = steinway["plate"]["outline"]
    for p in (steg_bass[0], steg_bass[-1], steg_diskant[0], steg_diskant[-1]):
        if not innen(um, p):
            raise SystemExit(f"Steg: das Ende {p} liegt ausserhalb des Umrisses.")

    def leiste(name, material, punkte_l, hoehe, breite):
        return {"name": name, "material": material, "line": [[runde(x), runde(y)] for x, y in punkte_l],
                "height": [runde(hoehe)] * len(punkte_l), "width": [runde(breite)] * len(punkte_l)}

    stege = [leiste("bass bridge", "beech", steg_bass, MAESSTRO_STEG_BASS, MAESSTRO_STEG_BREITE),
             leiste("treble bridge", "beech", steg_diskant, MAESSTRO_STEG_DISKANT, MAESSTRO_STEG_BREITE)]
    # die 16 Rippen von RT-0425 (von Bass nach Diskant sortiert) und die siebzehnte in der Diskant-Ecke
    # (PF3 a); Hoehe und Breite nach Tab. 1 von der Bass- bis zur Diskant-Rippe ueber alle 17
    linien = [(r["name"], [tuple(p) for p in r["line"]]) for r in steinway["ribs"]]
    linien.append((f"rib {len(linien) + 1}",
                   eckrippe([l for _, l in linien], steinway["plate"]["outline"], steg_diskant)))
    rippen = []
    n = len(linien)
    for k, (name, linie_r) in enumerate(linien):
        t = k / max(1, n - 1)
        hoehe = MAESSTRO_RIPPE_HOEHE[0] + t * (MAESSTRO_RIPPE_HOEHE[1] - MAESSTRO_RIPPE_HOEHE[0])
        breite = MAESSTRO_RIPPE_BREITE[0] + t * (MAESSTRO_RIPPE_BREITE[1] - MAESSTRO_RIPPE_BREITE[0])
        rippen.append(leiste(name, "sitka", linie_r, hoehe, breite))

    # Hoerorte an denselben Stellen, z mit der Steghoehe des Steges, auf dem die Taste liegt
    hoerorte = []
    for h in steinway["listeningPoints"]:
        ort = (h["x"], h["y"])
        steg = min(stege, key=lambda l: abstand_zur_linie(ort, [tuple(p) for p in l["line"]]))
        z = MAESSTRO_PLATTE / 2.0 + steg["height"][0] + MIKROFON_UEBER_SAITEN
        hoerorte.append(dict(h, z=runde(z)))

    tab2 = "Elie, Cotte & Boutillon, Acta Acustica 6 (2022), Table 2"
    return {
        "format": steinway["format"],
        "version": steinway["version"],
        "title": "MAESSTRO reference piano (RP)",
        "description": "Test case after Elie, Cotte & Boutillon (2022): the Steinway D of RT-0425 with the "
                       "simplifications of MAESSTRO - constant plate thickness, two bridges of constant "
                       "height, ribs without taper, Sitka spruce and beech with a loss factor of 2 %. "
                       "Test data, not shipped.",
        "source": "Outline, rib lines and bridge points as steinway_d.lvinstbodyplan (RT-0425); plate, bridges, "
                  "ribs and grain after Elie, Cotte & Boutillon, Acta Acustica 6 (2022) 30, Sect. 3.1 and "
                  "Table 1; materials after Table 2; the bridge split between keys 40 and 41 after RT-0425 "
                  "Appendix A and Fig. 6 (note index 1 = C1). Generated by make_steinway_d.py --maesstro.",
        "units": steinway["units"],
        "materials": [
            {"name": "sitka", "kind": "orthotropic", "density": 380.0, "ex": 11.0e9, "ey": 0.65e9, "gxy": 0.66e9,
             "gxz": 1.2e9, "gyz": 0.042e9, "nuxy": 0.26, "lossFactor": MAESSTRO_ETA,
             "source": f"{tab2} (Sitka spruce)"},
            {"name": "beech", "kind": "orthotropic", "density": 750.0, "ex": 13.7e9, "ey": 2.24e9, "gxy": 1.61e9,
             "gxz": 1.06e9, "gyz": 0.46e9, "nuxy": 0.3, "lossFactor": MAESSTRO_ETA,
             "source": f"{tab2} (Beech)"},
        ],
        "plate": {
            "material": "sitka",
            "grainAngle": MAESSTRO_FASER,
            "outline": steinway["plate"]["outline"],
            "thickness": MAESSTRO_PLATTE,
            "thicknessGrid": {"origin": [0.0, 0.0], "step": 0.0, "nx": 0, "ny": 0, "values": []},
        },
        "ribs": rippen,
        "bridges": stege,
        "bridgePoints": steinway["bridgePoints"],
        "listeningPoints": hoerorte,
    }


def pruefe_maesstro(plan):
    """Bericht der Zahlen, die PF1 a gegen Abb. 4 (a) prueft; bricht ab, wenn ein Stegpunkt nicht
    auf seinem Steg liegt."""
    bass, diskant = ([tuple(p) for p in l["line"]] for l in plan["bridges"])
    groesster = 0.0
    for p in plan["bridgePoints"]:
        steg = bass if p["key"] <= LETZTE_BASS_TASTE else diskant
        groesster = max(groesster, abstand_zur_linie((p["x"], p["y"]), steg))
    for name, l in (("Bass-Steg", bass), ("Diskant-Steg", diskant)):
        laenge = sum(np.hypot(*(np.array(l[k + 1]) - l[k])) for k in range(len(l) - 1))
        print(f"{name}: {len(l)} Punkte, Laenge {laenge:.3f} m, von ({l[0][0]:.3f}, {l[0][1]:.3f}) "
              f"bis ({l[-1][0]:.3f}, {l[-1][1]:.3f})")
    print(f"Stegpunkte auf ihrem Steg: groesster Abstand {groesster * 1e3:.1f} mm")
    if groesster > MAESSTRO_STEG_BREITE / 2:
        raise SystemExit("Ein Stegpunkt liegt nicht auf seinem Steg (mehr als die halbe Breite daneben).")
    winkel = []
    for r in plan["ribs"]:
        a, b = r["line"][0], r["line"][-1]
        winkel.append(np.degrees(np.arctan2(b[1] - a[1], b[0] - a[0])))
    abstaende = []
    for r0, r1 in zip(plan["ribs"], plan["ribs"][1:]):
        mitte = np.mean(np.array(r1["line"]), axis=0)
        abstaende.append(abstand_zur_linie(tuple(mitte), [tuple(p) for p in r0["line"]]))
    print(f"Rippen {len(plan['ribs'])}, Winkel {min(winkel):.1f} … {max(winkel):.1f} Grad, "
          f"Abstand {min(abstaende) * 1e3:.0f} … {max(abstaende) * 1e3:.0f} mm, "
          f"Hoehe {plan['ribs'][0]['height'][0] * 1e3:.0f} … {plan['ribs'][-1]['height'][0] * 1e3:.0f} mm")
    a, b = MAESSTRO_RIB15
    w15 = np.degrees(np.arctan2(b[1] - a[1], b[0] - a[0]))
    naechste = min(plan["ribs"], key=lambda r: abstand_zur_linie(a, [tuple(p) for p in r["line"]])
                   + abstand_zur_linie(b, [tuple(p) for p in r["line"]]))
    da = abstand_zur_linie(a, [tuple(p) for p in naechste["line"]])
    db = abstand_zur_linie(b, [tuple(p) for p in naechste["line"]])
    print(f"Stichprobe Rib15 aus Abb. 4 (a): Winkel {w15:.1f} Grad; naechste Rippe {naechste['name']}, "
          f"Enden {da * 1e3:.0f} und {db * 1e3:.0f} mm daneben")


def varianten(rp):
    """Die drei veraenderten Fluegel aus MAESSTRO §3.2 (S151, VA1 a), je „similar to the reference piano
    RP except …“: MP1 die Platte doppelt so dick (16 mm), MP2 „inter-rib spacing twice as RP (see
    Fig. 4b)“ — Abb. 4 (b) zeigt 9 Rippen, die Bass-Rippe und jede zweite danach bis zur Eckrippe,
    jede mit ihren Massen aus RP —, MP3 „without ribs: all ribs have been removed and the bridges are
    the only superstructures“. Die Hoerorte behalten x, y; z folgt der Plattendicke."""
    sec = "Elie, Cotte & Boutillon, Acta Acustica 6 (2022) 30, Sect. 3.2"
    mp1 = copy.deepcopy(rp)
    mp1["plate"]["thickness"] = 2 * MAESSTRO_PLATTE
    for h in mp1["listeningPoints"]:
        h["z"] = runde(h["z"] + MAESSTRO_PLATTE / 2.0)
    mp2 = copy.deepcopy(rp)
    mp2["ribs"] = rp["ribs"][::2]
    mp3 = copy.deepcopy(rp)
    mp3["ribs"] = []
    plaene = {}
    for name, plan, titel, was in (
            ("maesstro_mp1", mp1, "MAESSTRO modified piano 1 (MP1)",
             "the soundboard twice as thick, 16 mm (\"the thickness of the soundboard is twice the one of RP\")"),
            ("maesstro_mp2", mp2, "MAESSTRO modified piano 2 (MP2)",
             "inter-rib spacing twice as RP: ribs 1, 3, ..., 17 of RP with their own sizes (9 ribs, as Fig. 4b)"),
            ("maesstro_mp3", mp3, "MAESSTRO modified piano 3 (MP3)",
             "without ribs (\"all ribs have been removed and the bridges are the only superstructures\")")):
        plan["title"] = titel
        plan["description"] = (f"Variant of the MAESSTRO test case RP after Elie, Cotte & Boutillon (2022): {was}. "
                               "Test data, not shipped.")
        plan["source"] = (f"maesstro_rp.lvinstbodyplan (make_steinway_d.py --maesstro) with the modification of {sec}"
                          + (" and Fig. 4b." if name == "maesstro_mp2" else ".") + " Generated by make_steinway_d.py --maesstro.")
        plaene[name] = plan
    return plaene


def pruefe_varianten(rp, plaene):
    """Bericht: Masse und Rippen je Variante gegen RP (Masse ueber Flaeche und Leisten, wie `masse`)."""
    def masse(plan):
        dichte = {m["name"]: m["density"] for m in plan["materials"]}
        m = flaeche(plan["plate"]["outline"]) * plan["plate"]["thickness"] * dichte[plan["plate"]["material"]]
        for l in plan["ribs"] + plan["bridges"]:
            for k in range(len(l["line"]) - 1):
                laenge = float(np.hypot(*(np.array(l["line"][k + 1]) - np.array(l["line"][k]))))
                h = (l["height"][k] + l["height"][k + 1]) / 2
                b = (l["width"][k] + l["width"][k + 1]) / 2
                m += laenge * h * b * dichte[l["material"]]
        return m
    print(f"RP: Platte {rp['plate']['thickness'] * 1e3:.0f} mm, Rippen {len(rp['ribs'])}, Masse {masse(rp):.2f} kg")
    for name, plan in plaene.items():
        rippen = ", ".join(r["name"].split()[-1] for r in plan["ribs"]) or "-"
        z = ", ".join("%.1f" % (h["z"] * 1e3) for h in plan["listeningPoints"])
        print(f"{name}: Platte {plan['plate']['thickness'] * 1e3:.0f} mm, Rippen {len(plan['ribs'])} ({rippen}), "
              f"Masse {masse(plan):.2f} kg, Hoerorte z {z} mm")


def maesstro_bild(plan, maesstro_pdf, bildpfad, spalten=(222, 610)):
    """Legt den Pruefstein auf Abb. 4 (a) — mit `spalten=(650, 1094)` eine Variante auf Abb. 4 (b): der
    Umriss (blau im Bildschirmfoto) bestimmt Massstab und Versatz (ein Massstab fuer beide Achsen,
    eingepasst ueber den Abstand der Randpunkte)."""
    with tempfile.TemporaryDirectory() as tmp:
        subprocess.run(["pdfimages", "-f", "6", "-l", "6", "-png", maesstro_pdf, str(Path(tmp) / "a")], check=True)
        voll = np.asarray(Image.open(Path(tmp) / "a-000.png").convert("RGB")).astype(int)
    a = voll[:, spalten[0]:spalten[1]]  # (a) rechts der Bedienleiste, (b) die rechte Haelfte
    r, g, b = a[:, :, 0], a[:, :, 1], a[:, :, 2]
    blau = (b > 215) & (b - r > 30)  # der Umriss ist blass, etwa (195, 220, 245)
    blau[:60, :] = False  # Kopf des Fensters
    blau[:, :25] = False  # Rand der Bedienleiste; die Linie zerfaellt im JPEG in Stuecke — alle zaehlen
    feld = ndimage.distance_transform_edt(~blau)
    um = np.array(plan["plate"]["outline"] + [plan["plate"]["outline"][0]], float)
    dichte = []
    for k in range(len(um) - 1):
        for t in np.linspace(0, 1, 12, endpoint=False):
            dichte.append(um[k] + t * (um[k + 1] - um[k]))
    rand = np.array(dichte)
    ys, xs = np.nonzero(blau)
    start = [(xs.max() - xs.min()) / np.ptp(rand[:, 0]), xs.min() - rand[:, 0].min() * (xs.max() - xs.min()) / np.ptp(rand[:, 0]),
             ys.max() + rand[:, 1].min() * (xs.max() - xs.min()) / np.ptp(rand[:, 0])]

    def lage(t, p):
        p = np.asarray(p, float)
        return t[0] * p[..., 0] + t[1], -t[0] * p[..., 1] + t[2]

    im_bild = np.column_stack([xs, ys])[::3].astype(float)

    def kosten(t):
        # in beide Richtungen: unser Umriss zum Rand im Bild und der Rand im Bild zu unserem Umriss
        px, py = lage(t, rand)
        if not ((px >= 0) & (px < a.shape[1] - 1) & (py >= 0) & (py < a.shape[0] - 1)).all():
            return 1e9
        hin = float(ndimage.map_coordinates(feld, [py, px], order=1).mean())
        zurueck = float(spatial.cKDTree(np.column_stack([px, py])).query(im_bild)[0].mean())
        return hin + zurueck

    t = optimize.minimize(kosten, start, method="Nelder-Mead", options={"xatol": 1e-3, "fatol": 1e-4, "maxiter": 4000}).x
    px, py = lage(t, rand)
    d = ndimage.map_coordinates(feld, [py, px], order=1)
    teil = "(a)" if spalten[0] < 600 else "(b)"
    print(f"Abb. 4 {teil} gelegt: {1e3 / t[0]:.1f} mm je Pixel; Umriss zum Rand im Bild Median {np.median(d):.1f} px, "
          f"groesster {d.max():.1f} px")

    # die Rippen von MAESSTRO (gruen): Richtung aus der schaerfsten Haeufung, Abstand der Spitzen
    gruen = (g - r > 12) & (g - b > 12) & (g < 230)
    gruen[:60, :] = False
    gy, gx = np.nonzero(gruen)
    best = None
    for grad in np.arange(-55.0, -25.0, 0.1):
        w = np.radians(grad)
        quer = -gx * np.sin(w) + gy * np.cos(w)
        h, _ = np.histogram(quer, bins=np.arange(quer.min(), quer.max() + 2, 1.5))
        schaerfe = float((h.astype(float) ** 2).sum())
        if best is None or schaerfe > best[0]:
            best = (schaerfe, grad)
    w = np.radians(best[1])
    quer = -gx * np.sin(w) + gy * np.cos(w)
    h, kanten = np.histogram(quer, bins=np.arange(quer.min() - 1, quer.max() + 3, 1.0))
    glatt = ndimage.uniform_filter1d(h.astype(float), 3)
    spitzen = []
    for k in range(2, len(glatt) - 2):
        if glatt[k] == glatt[k - 2:k + 3].max() and glatt[k] > 0.1 * glatt.max():  # die kurzen im Diskant
            if spitzen and k - spitzen[-1] < 6:
                if glatt[k] > glatt[spitzen[-1]]:
                    spitzen[-1] = k
            else:
                spitzen.append(k)
    lagen = [(kanten[k] + kanten[k + 1]) / 2 for k in spitzen]
    abst = [d_px * 1e3 / t[0] for d_px in np.diff(lagen)]
    print(f"Rippen in Abb. 4 {teil} (gruen, in (a) mit der gewaehlten Rib15): {len(lagen)}, Winkel {-best[1]:.1f} Grad, "
          f"Abstand {min(abst):.0f} … {max(abst):.0f} mm")

    # die Stege von MAESSTRO (rosa) gegen unsere: Abstand jedes rosa Pixels zum naechsten Steg
    rosa = (r - g > 15) & (b - g > 10)  # etwa (249, 220, 242); der blasse Umriss hat r < g
    rosa[:60, :] = False
    ry, rx = np.nonzero(rosa)
    unsere = []
    for steg in plan["bridges"]:
        l = np.array(steg["line"], float)
        for k in range(len(l) - 1):
            for u in np.linspace(0, 1, 20, endpoint=False):
                unsere.append(l[k] + u * (l[k + 1] - l[k]))
    ux, uy = lage(t, np.array(unsere))
    dr = spatial.cKDTree(np.column_stack([ux, uy])).query(np.column_stack([rx, ry]))[0] * 1e3 / t[0]
    print(f"Stege: rosa zu unseren Median {np.median(dr):.0f} mm, 90 % unter {np.percentile(dr, 90):.0f} mm "
          f"(die Lage im Bild ist nur ueber den Umriss eingepasst)")
    bild = Image.fromarray(np.clip(a, 0, 255).astype(np.uint8)).convert("RGB")
    z = ImageDraw.Draw(bild)

    def zug(punkte_l, farbe, breite=1):
        z.line([tuple(np.array(lage(t, p)).tolist()) for p in punkte_l], fill=farbe, width=breite)

    zug(plan["plate"]["outline"] + [plan["plate"]["outline"][0]], (255, 140, 0))
    for r in plan["ribs"]:
        zug(r["line"], (0, 0, 0))
    for steg, farbe in zip(plan["bridges"], ((200, 0, 0), (120, 0, 160))):
        zug(steg["line"], farbe, 2)
    if teil == "(a)":
        zug(MAESSTRO_RIB15, (0, 180, 255), 2)
    bild.resize((bild.width * 3, bild.height * 3), Image.LANCZOS).save(bildpfad)


def main(argv):
    if not argv or argv[0].startswith("--"):
        raise SystemExit(__doc__)
    pdf = argv[0]
    ziel = None
    bild = None
    maesstro_pdf = None
    k = 1
    while k < len(argv):
        if argv[k] == "--ziel":
            ziel = Path(argv[k + 1])
            k += 2
        elif argv[k] == "--bild":
            bild = argv[k + 1]
            k += 2
        elif argv[k] == "--maesstro":
            maesstro_pdf = argv[k + 1]
            k += 2
        else:
            raise SystemExit(f"unbekannt: {argv[k]}")
    if maesstro_pdf:
        plan = maesstro(baue(pdf))
        pruefe_maesstro(plan)
        plaene = {"maesstro_rp": plan}
        plaene.update(varianten(plan))
        pruefe_varianten(plan, {k: v for k, v in plaene.items() if k != "maesstro_rp"})
        if bild:
            maesstro_bild(plan, maesstro_pdf, bild)
            stamm = Path(bild)
            print("MP2 auf Abb. 4 (b):")
            maesstro_bild(plaene["maesstro_mp2"], maesstro_pdf, str(stamm.with_name(stamm.stem + "_mp2" + stamm.suffix)),
                          spalten=(650, 1094))
        ziel = ziel or Path(__file__).resolve().parents[3] / "projects/apps/LumiViz/tests/unit/UnitTests/koerper"
    else:
        steinway = baue(pdf, bild)
        plaene = {"steinway_d": steinway}
        plaene.update(glasboeden(steinway))
        pruefe_glasboeden(steinway, {k: v for k, v in plaene.items() if k != "steinway_d"})
        ziel = ziel or Path(__file__).resolve().parent
    ziel.mkdir(parents=True, exist_ok=True)
    for name, plan in plaene.items():
        datei = ziel / (name + ".lvinstbodyplan")
        datei.write_text(json.dumps(plan, indent=4, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n")
        print(f"geschrieben: {datei}")


if __name__ == "__main__":
    main(sys.argv[1:])
