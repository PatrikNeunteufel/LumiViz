"""Erzeugt den Bauplan des Resonanzbodens eines Steinway D aus RT-0425 (S147, Stufe 31c-1;
Entscheide Patrik BM1 a, BM2 a, BM3 a, GB1 a).

    python -I asset/loop/instrument_bodies/make_steinway_d.py <RT-0425.pdf> [--ziel <ordner>] [--bild <datei.png>]

Quelle: J. Chabassier, M. Duruflé, „Physical parameters for piano modeling", Inria RT-0425
(2012), hal-00688679 — die PDF liegt nur lokal unter
`projects/apps/LumiViz/docs/modellierung/hammer-saite/quellen/` (nicht im Repo). Gebraucht
werden `pdftotext` und `pdfimages` (Poppler), dazu numpy, scipy, Pillow.

Schreibt `steinway_d.lvinstbodyplan` (Format `lumiviz-instrument-body-plan`, Fassung 2) neben
dieses Skript oder nach `--ziel`. Kein Zufall: zwei Laeufe ergeben dieselbe Datei.
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
import json
import re
import subprocess
import sys
import tempfile
from collections import deque
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage, optimize

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


def main(argv):
    if not argv or argv[0].startswith("--"):
        raise SystemExit(__doc__)
    pdf = argv[0]
    ziel = Path(__file__).resolve().parent
    bild = None
    k = 1
    while k < len(argv):
        if argv[k] == "--ziel":
            ziel = Path(argv[k + 1])
            k += 2
        elif argv[k] == "--bild":
            bild = argv[k + 1]
            k += 2
        else:
            raise SystemExit(f"unbekannt: {argv[k]}")
    plan = baue(pdf, bild)
    ziel.mkdir(parents=True, exist_ok=True)
    datei = ziel / "steinway_d.lvinstbodyplan"
    datei.write_text(json.dumps(plan, indent=4, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n")
    print(f"geschrieben: {datei}")


if __name__ == "__main__":
    main(sys.argv[1:])
