"""LVR Auftrag 21: Warum lag der Entwurf aus Bericht 11 (ue.iir_bank) an 6 Tasten ueber 1?
Je Fall: Original (ue.iir_bank, Spiegeln ohne Ausgleich), dasselbe mit Ausgleich des Betrags beim Spiegeln,
Zahl der gespiegelten Pole und deren Betrag. Gl wie in ue.entwurf (Thiran 1: Gl = Gt).
Aufruf: python -B diag_bericht11.py -> diag_bericht11.txt
"""
import sys
sys.dont_write_bytecode = True
import pathlib

import numpy as np

sys.path.insert(0, str(pathlib.Path(__file__).parent))
import bank_voll as bv  # noqa: E402

ue = bv.ue


def bericht11_ohne_spiegeln(Gt, fn, N, iters=6):
    """ue.iir_bank bis vor das Spiegeln (Zeilen 87-111 von umlauf_entwurf.py, unveraendert abgeschrieben)."""
    FS, F = ue.FS, ue.F
    nfft = 1 << 16
    fu = np.linspace(0, FS / 2, nfft // 2 + 1)
    gu = np.interp(np.maximum(fu, F[0]), F, Gt, right=Gt[-1])
    c = np.fft.irfft(np.log(gu), nfft)
    c[1:nfft // 2] *= 2
    c[nfft // 2 + 1:] = 0
    Hmin = np.exp(np.fft.rfft(c, nfft))
    fk = fn[fn < 0.45 * FS]
    Hd = np.interp(fk, fu, Hmin.real) + 1j * np.interp(fk, fu, Hmin.imag)
    gk = np.abs(Hd)
    wk = 1.0 / np.maximum(1 - gk, 1e-7) ** 2
    wk /= wk.max()
    x = 2 * np.pi * fk / FS
    E = np.exp(-1j * np.outer(x, np.arange(N + 1)))
    A_prev = np.ones(len(fk), complex)
    for _ in range(iters):
        M = np.hstack([E, -Hd[:, None] * E[:, 1:]])
        s = wk / np.abs(A_prev)
        Mr = np.vstack([(M * s[:, None]).real, (M * s[:, None]).imag])
        rr = np.concatenate([(Hd * s).real, (Hd * s).imag])
        p = np.linalg.lstsq(Mr, rr, rcond=None)[0]
        b, a = p[:N + 1], np.concatenate([[1.0], p[N + 1:]])
        A_prev = E @ a
    return b, a


FAELLE = [("Harfe Taste 46", 4), ("Harfe Taste 46", 8), ("Harfe Taste 60", 8), ("Harfe Taste 96", 2), ("Harfe Taste 103", 2),
          ("Harfe Taste 103", 4), ("Cembalo Taste 29", 4), ("Laute Chor 1", 8)]

if __name__ == "__main__":
    sys.stdout = bv.Tee("diag_bericht11.txt")
    print("Fall | Pole ausserhalb (Betraege) | max|H| wie Bericht 11 (Spiegeln ohne Ausgleich) | mit Ausgleich 1/|r|")
    tasten = {t: (sa, L) for t, sa, L in ue.tasten()}
    for name, N in FAELLE:
        t = next(k for k in tasten if k.startswith(name))
        sa, L = tasten[t]
        Gt, ph = ue.ziel(sa, L)
        fn, _ = ue.teiltoene(Gt, ph)
        b, a = bericht11_ohne_spiegeln(Gt, fn, N)
        r = np.roots(a)
        aus = np.abs(r) >= 1
        # Bericht 11: Spiegeln ohne Ausgleich
        a_sp = np.real(np.poly(np.where(aus, 1 / np.conj(r), r)))
        m11 = bv.maxbetrag(b, a_sp)
        m_aus = bv.maxbetrag(b / np.prod(np.abs(r[aus])), a_sp) if aus.any() else m11
        print(f"{t[:22]:22s} iir{N} | {int(aus.sum())} ({', '.join(f'{v:.4f}' for v in np.abs(r[aus]))}) | "
              f"{m11[0]:.6f} bei {m11[1]:.0f} Hz | {m_aus[0]:.6f} bei {m_aus[1]:.0f} Hz")
