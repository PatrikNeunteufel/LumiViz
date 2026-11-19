"""S207 - legt die Abloesekraft nach GZ13 a (2,8 dB je Oktave fallend, Bezug C4) auf die Tabelle des Cembalos: der
Pruefstand rechnet mit fester Kraft und ist in der Kraft linear. Aufruf: <tsv ein> <tsv aus>"""
import sys

with open(sys.argv[1], encoding="utf-8") as ein, open(sys.argv[2], "w", encoding="utf-8") as aus:
    for zeile in ein:
        t = zeile.split("\t")
        if len(t) < 2:
            continue
        k = -2.8 * (int(t[0]) - 60) / 12
        aus.write("\t".join([t[0]] + [f"{float(v) + k:.3f}" for v in t[1:]]) + "\n")
