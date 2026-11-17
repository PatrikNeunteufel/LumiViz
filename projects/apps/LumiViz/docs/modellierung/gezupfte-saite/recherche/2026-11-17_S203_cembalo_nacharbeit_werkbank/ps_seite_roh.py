"""Rohtext einer PS-Seite ausgeben (gekuerzt): python ps_seite_roh.py <ps> <seite> <von> <bis>"""
import re
import sys

roh = open(sys.argv[1], "rb").read().decode("latin-1")
seiten = re.split(r"%%Page:", roh)
s = seiten[int(sys.argv[2])]
print(len(s), "Zeichen")
print(s[int(sys.argv[3]):int(sys.argv[4])])
