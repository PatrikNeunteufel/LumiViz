"""S205 - KQ31 a: der Pruefstand je Taste (`2026-11-15_S199_pruefstand_zupf.py harfe --je-taste`) an einer beliebigen
Koerperdatei der Harfe, dazu die Kennzahlen nach KQ34 a (Spanne, groesster und mittlerer Sprung ueber Paare bis 4
Halbtoene; alle Tasten und ab D2 = Taste 38) und die Moden der Datei unter 1 kHz.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <ordner mit concert_harp.lvinstbody>
Schreibt nichts.
"""
import contextlib
import importlib.util
import io
import json
import pathlib
import sys

import numpy as np

P = pathlib.Path(__file__).parent
spec = importlib.util.spec_from_file_location("pruefstand", P / "2026-11-15_S199_pruefstand_zupf.py")
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)
ps.KOERPER = pathlib.Path(sys.argv[1])

k = json.loads((ps.KOERPER / "concert_harp.lvinstbody").read_text(encoding="utf-8"))
print("Moden unter 1 kHz (Hz / Q): " + ", ".join(f"{m['f']:.1f} / {1 / m['eta']:.0f}" for m in k["modes"] if m["f"] < 1000))
puffer = io.StringIO()
with contextlib.redirect_stdout(puffer):
    ps.tasten_instrument("harfe", True)
text = puffer.getvalue()
print(text, end="")
tl = [(int(z.split("|")[0]), float(z.split("|")[2].split("/")[0])) for z in text.splitlines() if z[:3].strip().isdigit()]


def kennzahlen(tasten, l):
    l = np.asarray(l)
    paare = [(abs(l[i] - l[j]), tasten[i], tasten[j]) for i in range(len(l)) for j in range(i + 1, len(l)) if 0 < tasten[j] - tasten[i] <= 4]
    d = np.array([p[0] for p in paare])
    g = max(paare)
    return l.max() - l.min(), d.max(), (g[1], g[2]), np.median(d), tasten[int(np.argmin(l))], tasten[int(np.argmax(l))]


for titel, ab in (("alle Tasten", 0), ("ab D2 (Taste 38)", 38)):
    sel = [(t, l) for t, l in tl if t >= ab]
    sp, gr, wo, mi, leise, laut = kennzahlen([t for t, _ in sel], [l for _, l in sel])
    print(f"{titel}: Spanne {sp:.1f} LU (leiseste Taste {leise}, lauteste {laut}), groesster Sprung {gr:.1f} LU ({wo[0]}/{wo[1]}), mittlerer {mi:.1f} LU")
