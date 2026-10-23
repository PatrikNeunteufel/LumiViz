# Recherche S146: Resonanzboden — Moden numerisch vorab rechnen

> **Datum:** 2026-10-23 (Session 146) · **Auftrag:** Verfahren, Aufwand und nutzbarer Code, um die Moden/Antwort
> eines Klavierbodens einmal vorab numerisch zu rechnen (FEM, finite Differenzen, Rayleigh-Ritz), machbar in C++
> ohne große Bibliothek.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [wörtlich] = im Volltext gesehen, [nur Zusammenfassung] = Suchergebnis/Sekundär.
> **Von Claude selbst nachgelesen:** Chabassier et al. 2013 „2 400 modes are needed … up to 10 kHz“; Elie et al.
> 2022 „about 2 h“. Alles andere: nur Bericht.
> **Quellen-PDFs:** `../quellen/` (nicht versioniert); die im Bericht genannten Pfade ins Scratchpad gibt es nicht mehr.
> **Eingeflossen in:** `docs/audio/Spielweisen_Konzept.md` §4.1, „Befund Recherche Boden und Körper“.

---

**Bericht: Moden eines Klavier-Resonanzbodens numerisch rechnen**

Was ich gelesen habe: Elie et al. 2022, Chabassier et al. 2013 (JASA) und 2014/15 (Teil 2), Ege/Boutillon/Rébillat 2013, Ege/Boutillon 2012 und Corradi et al. 2017 im Volltext (pdftotext, gezielt durchsucht). Dazu die Dateiliste des MAESSTRO-Repos über die GitLab-API. Alles Übrige stammt aus Such-Zusammenfassungen. Das Repo habe ich nicht angefasst.

### 1. Chabassier/Chaigne/Joly – Flügel, Reissner-Mindlin-FEM, modale Zerlegung
- Quelle: https://inria.hal.science/hal-00873089/file/Piano-model-revis.pdf (JASA 134, 2013)
- Verfahren: Reissner-Mindlin-Platte, orthotrop und heterogen. Zerlegt wird nach den Moden der ungedämpften Platte, die Dämpfung ist in der Modalbasis diagonal.
- **[wörtlich]** „2 400 modes are needed to model the soundboard vibrations up to 10 kHz“
- **[wörtlich]** „These modes are only computed once for all, before the time computation starts.“
- **[wörtlich]** Bildunterschrift Fig. 7: „computed with fourth-order finite elements (nearly 450 000 degrees of freedom)“. Im Text dagegen: „420 000 dofs are used to compute the 2400 first modes“.
- **[wörtlich]** zur Gesamtsimulation mit Saiten und Luft (nicht nur zum Boden): „one second of sound … takes 24 hours on a 300 cpus cluster“.
- Teil 2 (Numerik): https://hal.science/hal-01085477/file/SubM2AN_author.pdf
  - **[wörtlich]** „higher order finite elements methods based on a quadrilateral mesh“
  - **[wörtlich]** „M = 2, 400“, gewählt so, dass alle Eigenfrequenzen in [0, 10] kHz enthalten sind.
- Eine Rechenzeit nur für den Eigenlöser habe ich nicht gefunden.

### 2. Elie/Cotté/Boutillon – MAESSTRO (Acta Acustica 2022, CC-BY 4.0)
- Quelle: https://perso.ensta.fr/~cotte/files/Elie_Cotte_Acta_Acustica2022.pdf
- **[wörtlich]** „we chose to use the finite-element method, associated with the library Montjoie“. Vernetzt wird mit GMSH.
- Modell: Reissner-Mindlin, Rand eingespannt. **[wörtlich]** Die Aufbauten (Rippen, Stege) sind „modeled by the thickness variation of the soundboard“. Deshalb dicke statt dünne Platte.
- Netzregel: Elementlänge lc ≤ λmin/4, λmin ist die kleinste Biegewellenlänge quer zur Faser bei Fmax. Die FE-Ordnung wird per Konvergenz an etwa 5 hohen Testmoden gewählt. **[wörtlich]** „which leads to k = 3“.
- **[wörtlich]** „The resulting mesh contains 23,006 elements.“
- Rechenzeit, Rechner mit 4 CPUs zu 2,3 GHz: **[wörtlich]** „The computation time for the modal basis, up to 5 kHz, using a third-order finite element model is about 2 h.“
- Semi-analytische Variante nach Trévisan: **[wörtlich]** „it takes around 700 s to compute the modal basis up to 5000 Hz) at the cost of the loss of fine geometric details“.
- Gerechnet wird konservativ: reelle Moden, ein konstanter Verlustfaktor wird nachträglich aufgeschlagen.
- **Code:** https://gitlab.com/benjamin.elie/maesstro, Lizenz laut Repo-Seite **[wörtlich]** „CeCILL-C“.
  - Laut Dateiliste (API) gibt es MATLAB-Klassen wie `@Soundboard/modalbasis.m`, `extendedplate.m` (wohl die semi-analytische Variante, nicht geöffnet), `computepanelmatrix.m`, `@Ribs/computeribsmatrix.m`, `@Bridge/computebridgematrix.m`, `montjoiemodalfrequency.m` und `createmesh.m`.
  - Dazu ein Ordner `python-maesstro`.
  - Den Inhalt der Dateien habe ich nicht gelesen.

### 3. Montjoie (Duruflé)
- Gefunden habe ich es nur über eine Suchmaschinen-Zusammenfassung (Seite `math.u-bordeaux.fr/~durufle/montjoie/` war nicht erreichbar) **[nur Zusammenfassung]**:
  - C++, LGPL, Repo https://gitlab.inria.fr/durufle/montjoie, braucht nur Metis.
- Lizenzdatei und Stand des Repos habe ich nicht geprüft.

### 4. Trévisan/Ege/Laulagnet 2017 – semi-analytisch (Rayleigh-Ritz-artig)
- Quelle: https://hal.archives-ouvertes.fr/hal-01456083 (JASA 2017) **[nur Zusammenfassung]**
- Der berippte, nicht rechteckige, eingespannte orthotrope Boden wird in eine größere, frei aufliegende Rechteckplatte eingebettet. Federn sperren die Restfläche.
- Basis sind die Moden der unberippten Rechteckplatte. Der Ansatz ist variationell, mit den Energien von Platte und Aufbauten.
- Bereich 0–3000 Hz, Pianino.
- Den Begriff „Rayleigh-Ritz“ selbst habe ich nicht belegt.

### 5. Ege/Boutillon/Rébillat 2013 – Pianino, Cast3M
- Quelle: https://arxiv.org/pdf/1212.2323
- **[wörtlich]** „All finite elements are triangular thin-shell orthotropic elements. The mesh (Fig. 12) has 14267 nodes.“
- Weiter laut Text: Wölbung (Kugelschale R = 43 m), Druck der Saiten als Last auf die Stege, Rand eingespannt.
- **[wörtlich]** unter 1,1 kHz: modale Dichte „about 0.06 modes Hz-1“, der Boden verhält sich wie eine homogene Platte. Darüber sind die Wellen zwischen den Rippen eingeschlossen.
- Ege/Boutillon 2012 (https://arxiv.org/pdf/1212.3068): **[wörtlich]** „(triangular elements of 2 cm)“, die berippte Zone dort ersetzt durch eine isotrope Platte.

### 6. Corradi/Miccoli/Squicciarini/Fazioli 2017 – Fazioli-Flügel, Abaqus (kommerziell)
- Quelle: https://eprints.soton.ac.uk/408405/1/20160514_postprint.pdf
- **[wörtlich]** „About 6000 elements where used, with an average in-plane edge 20 mm“. Die Größe ist so gewählt, dass mindestens fünf Elemente auf eine Wellenlänge bei 4 kHz kommen.
- Elemente: Continuum Shell SC8R für den Boden, C3D8R für die Rippen.
- Der Fertigungsweg (Verleimen, Wölbung, Eigenspannung) wird nichtlinear durchgerechnet, danach folgt eine lineare Eigenanalyse.
- **[wörtlich]** gegen Messung „good agreement up to 4 kHz“ (Steg-Mobilität).
- Code: keiner.

### 7. Giordano 1997
- **[nur Zusammenfassung]** Laut Zusammenfassung ein einfaches Finite-Elemente-Modell, nicht finite Differenzen wie im Auftrag angenommen.
- Untersucht wird die Impedanz zwischen 50 und 104 Hz, Rippen gelten als wesentlich.
- Netz und Zahlen nicht gefunden.

### 8. Eigenlöser
- **[nur Zusammenfassung]** Spectra: Header-only C++, braucht nur Eigen, MPL-2.0 wie Eigen. Implizit neu startendes Lanczos/Arnoldi ohne ARPACK-Code.
- Die Lizenz von ARPACK habe ich nicht geprüft.

### Nicht gefunden
- Berthauts Arbeiten habe ich nicht geprüft.
- Kein GitHub-Projekt „piano soundboard FEM/modal“ und keine Beispiele für FEniCS, deal.II oder Elmer zu Klavier- oder Gitarrenböden.
- Keine Rechenzeit nur für den Eigenlöser bei Chabassier.
- Keine Bodenmodelle mit Luft als Vorabrechnung. Chabassier rechnet die Luft mit mehr als 9·10⁷ Freiheitsgraden in der Zeit, nicht modal.

### Eigene Einschätzung (nicht belegt)
- **Größenordnung:** Für einen Flügel bis 10 kHz braucht es rund 2400 Moden aus etwa 0,4 Mio. Freiheitsgraden. Bis 5 kHz reichen nach MAESSTRO etwa 23.000 Elemente dritter Ordnung, rund 2 h auf 4 Kernen. Für eine einmalige Vorabrechnung ist das machbar.
- **Gespeichert wird wenig:** je Mode Frequenz, Dämpfung, Auslenkung an den Steg-Anschlusspunkten der Saiten und eine Abstrahlgröße. Bei 2400 Moden mal etwa 90 Saitenpunkten sind das Kilobytes bis wenige Megabytes.
- **Vorlage:** Am nächsten an euer Ziel kommt das Modell von MAESSTRO/Chabassier, also eine Reissner-Mindlin-Platte, bei der die Rippen und Stege nur als dickere Stellen eingehen.
  - Es lässt sich als eigenes C++-Werkzeug offline nachbauen: viereckige Elemente höherer Ordnung, Eigen-Sparse plus Spectra im Shift-Invert-Modus. Alles MPL-2.0, kein Fortran.
  - Die rund 2400 Moden würde man in Frequenzfenstern nacheinander mit mehreren Shifts holen.
- **Montjoie selbst** einzubinden wäre schwer (Linux-Makefiles, LGPL). MAESSTRO taugt als Referenz für Geometrie, Materialien und Vergleichswerte. Ob CeCILL-C es erlaubt, Code zu übernehmen, müsstest du prüfen.
- **Der billigere Weg** wäre der semi-analytische nach Trévisan (eingebettete Rechteckplatte, Ritz-Basis): rund 700 s bis 5 kHz, in C++ ohne Netzgenerator. Dafür gehen geometrische Details verloren.
