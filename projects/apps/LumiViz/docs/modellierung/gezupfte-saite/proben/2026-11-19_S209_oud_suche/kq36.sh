#!/bin/sh
# S209 - eine Fassung der Oud nach KQ36 a messen. Aufruf vom Projekt-Root:
#   sh <dieser Ordner>/kq36.sh <zielordner> <decke mm> <steghoehe mm> <stegbreite mm> <muendung> <rosette g> [ohne]
S=$(dirname "$0")
PY="C:/Users/patri/AppData/Local/Programs/Python/Python312/python.exe"
P=projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben
Z=$1
shift
PYTHONPATH=$S "$PY" -X utf8 -B $S/oud_pruefkoerper_x.py $Z "$@" || exit 1
out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe $Z/oud.lvinstbodyplan --to 5000 > $Z/tool.log 2>&1 || exit 1
"$PY" -X utf8 -B $P/2026-11-19_S207_viele_orte.py oud $Z $Z/kq36_oud.tsv > $Z/kq36_oud.txt 2>&1 || exit 1
cut -f1,2,4- $Z/kq36_oud.tsv > $Z/kq36_oud_mik.tsv
echo "== $Z $*"
"$PY" -X utf8 -B $P/2026-11-19_S207_tu_auswerten.py $Z/kq36_oud_mik.tsv | head -n 2 | tail -n 1
"$PY" -X utf8 -B $P/2026-11-19_S207_tu_auswerten.py $Z/kq36_oud_mik.tsv ab=41 | head -n 2 | tail -n 1
cut -f1,2 $Z/kq36_oud.tsv | awk '{printf "%s:%.1f ", $1, $2}'
echo
