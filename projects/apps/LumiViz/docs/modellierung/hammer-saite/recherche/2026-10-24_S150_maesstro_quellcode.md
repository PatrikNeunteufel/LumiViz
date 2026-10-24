# MAESSTRO im Quellcode: Mobilität am Steg und Stegpunkte (S150)

> **Datum:** 2026-10-24 · **Session:** LumiViz_Session150 · **Anlass:** PS2 a (Patrik) — vor den Entscheiden zu
> PS1 a (1) nachsehen, wie MAESSTRO die Kurve in Abb. 6 rechnet.
> **Quelle:** https://gitlab.com/benjamin.elie/maesstro (Zweig `master`, gelesen über die GitLab-Schnittstelle als
> Rohtext, nichts heruntergeladen oder ausgeführt). Wortlaut unten aus den Dateien, Fundstelle je Zitat.
> **Reichweite:** `python-maesstro/maesstro/_soundboard.py`, `_oscillateurs_chevalet.py`, `_config.py`,
> `schemata/*.json`; `matlab-maesstro/MODES_TABLE.m`, `read_parameters.m`, `@Soundboard/computemobility.m`,
> `@Soundboard/montjoiemodalshape.m`; Dateibaum ohne `maesstro-ihm` (Oberfläche, `node_modules`). Nicht gelesen:
> der C++-Kern Montjoie (nicht im Repository), die Oberfläche.

## Befunde

1. **Gl. 12 wie bei uns.** `_soundboard.py` `computemobility(self, excPoint, obsPoint, freqVec=np.arange(5001))`,
   am Ende `Ymean = 10**np.mean(np.log10(abs(Y)), axis=0)` — das geometrische Mittel in Schritten von 1 Hz. Die
   Dämpfung `Cr = np.eye(modemax) * eta * np.sqrt(np.diag(lbd)) * np.diag(mr)`, also c_n = η ω_n m_n wie in
   unserer Gl. 10; `eta = self.damping` ist **ein** Wert.
2. **Nur die Moden der Basis.** Die Mobilität summiert über `modal_mass` aus `SORTIES_MODES_TABLE.h5`; der Kopf
   von `_oscillateurs_chevalet.py` nennt sie „les N modes calculés et conservés dans la bande de fréquences
   utile“. Keine Ergänzung oberhalb von `max_freq`. „Aus den gerechneten Moden“ (PS1 a) entspricht MAESSTRO.
3. **Die Formen kommen über eine Sinus-Basis.** `montjoiemodalshape.m`: die Formen aus Montjoie werden auf einem
   Raster gelesen und mit `Umat = M\Vvec` (kleinste Quadrate) auf `xDim*yDim` Funktionen
   `sin(p*pi.*(X(:)-Xmin)/l).*sinql(:)` des umschließenden Rechtecks gelegt; `obj.modal_mass = ones(nModes,1)`.
   Ausgewertet wird die Form am Stegpunkt aus diesen Koeffizienten (`genForce` in `computemobility`). Die Zahl der
   Sinus-Funktionen `n_xsin`, `n_ysin` ist Pflichtangabe (`read_parameters.m`: „Number of x sinus not defined“),
   ohne Vorgabe; der Artikel nennt sie nicht, die Daten des Referenzflügels liegen nicht im Repository.
   **Folge:** eine Form mit mehr örtlichem Gehalt, als die Basis trägt — etwa der Knick am steifen Steg —, wird
   geglättet; φ² am Steg und damit |Y| hängen von `n_xsin`, `n_ysin` ab.
4. **Die Stegpunkte werden verschoben und auf die Stege gezogen.** `_oscillateurs_chevalet.py`
   `adjust_coupling_positions`: `aX = xref - xe[-1]`, `aY = yref - ye[-1]`, `xe = xe + aX`, `ye = ye + aY` — alle
   Punkte so verschoben, dass der **letzte** auf `SB.ref_point` liegt; dann je Punkt der nächste Punkt auf einem
   der Stege (`NearestNeighbors(n_neighbors=1)`, die Stege mit `interp1d` über x in 10 000 Punkten), und
   `IdxTmp = np.sort(IdxTmp.squeeze())`. Ein Punkt jenseits eines Stegendes landet auf dem Stegende; mehrere
   Punkte können auf denselben fallen. In Abb. 6 tragen die Indizes 83 und 84 denselben Wert (−72,72 dB, abgelesen).
   `ref_point` des Referenzflügels ist nicht veröffentlicht.
5. **Ordnung im FE-Zweig.** `MODES_TABLE.m`: `for k_ord = 1:3`, Abbruch bei `nerr < 1e-4` (in Prozent) — höchstens
   k = 3, wie der Artikel („k = 3“).
6. **Im Repository:** Code (MATLAB, Python, Oberfläche), Schemata `plan-cordes.json`, `jeu.json`, Hörbeispiele
   `examples/audio/{RP,MP1,MP2,MP3}_C3_28.wav`, `RP_Hammerklavier_3rd.wav`. **Nicht:** Geometrie, Materialdatei,
   Parameter (`n_xsin`, `n_ysin`, `max_freq`) oder Stringplan des Referenzflügels. Der Artikel: „Additional data are
   available on request from authors.“

## Was das für PS1 a (1) heißt

- Gleiche Formel und gleiche Modenmenge: ein Unterschied im **Pegel** zwischen unserem Prüffall und Abb. 6 kann aus
  der Sinus-Basis (3.) kommen, deren Größe unbekannt ist — er ist kein sicheres Zeichen eines Code-Fehlers bei uns.
  Unser Pegel ist unabhängig belegt (Einmassenschwinger exakt, Satz vom Mittelwert an der Aluminiumplatte auf 1 %).
- Die **Enden** (Indizes um 17/18 und ab 79) hängen an der Verschiebung und dem Ziehen der Stegpunkte (4.) mit
  unbekanntem `ref_point`.
- Was Abb. 6 trägt, ohne diese Unbekannten: das **Muster** längs des Stegs (Abstand Bass/Diskant, Schwankung über
  den Rippen) im Inneren der Stege.
