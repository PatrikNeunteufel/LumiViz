# Chords — sources and credits

## chords-db (guitar and ukulele)

The chord shapes under `guitar/` and `ukulele/` come from **chords-db** by David Rubert
(<https://github.com/tombatossals/chords-db>), npm package `@tombatossals/chords-db` version 0.5.1,
files `lib/guitar.json` and `lib/ukulele.json`, under the **MIT License** (below).

LumiViz converted them: one file `.lvchord` per position, frets counted from the nut instead of from
`baseFret`, checked against the `midi` notes of the source (`make_chords.py`). Guitar: standard tuning
E A D G B E, 2069 positions. Ukulele: G C E A, 2114 positions.

```
MIT License

Copyright (c) 2016 David Rubert

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Computed (LumiViz)

The shapes under `drop_d/`, `bass/` and `violin/` have no source: `make_chords.py` computes them by a
rule (chord tones across the strings, at most four frets, open strings preferred, the root in the bass,
at most four fingers). They carry `"origin": "computed"`.

## Voicings (LumiViz)

The voicings under `voicings/` (root position, inversions, open position, drop 2) follow from the
intervals of each chord type — textbook music theory, written by `make_chords.py`.
