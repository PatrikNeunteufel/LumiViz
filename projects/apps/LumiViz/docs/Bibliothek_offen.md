# Offen für eine Hochschulbibliothek

> Angelegt S155 (2026-10-26). Patrik hat **bis Frühling 2027 keinen Hochschulzugang**; diese Liste sammelt für alle
> Bereiche von LumiViz, was nur über eine Bibliothek (Abo, Fernleihe, Besuch vor Ort) oder einen Kauf zu bekommen ist.
> Neue Einträge unten anhängen, mit Bereich und Anlass.
>
> **Prüfstand der Angaben:** Titel, Zeitschrift, Band, Seiten und DOI der Zeitschriftenartikel aus der Crossref-API
> (`api.crossref.org`, abgerufen S155, 2026-10-26); die Verlagsseiten (Links) folgen aus der DOI. Gelesen ist keiner
> dieser Artikel. Was ohne DOI dasteht, stammt aus den Agentenberichten und ist am Katalog zu prüfen.
> Ablage nach dem Beschaffen: Quellen zur Modellierung unter `modellierung/<klasse>/quellen/` (gitignored), Eingang über
> `LumiViz\dld\` (gitignored).

## Wie prüfen

- **J. Acoust. Soc. Am. (JASA)** liegt bei AIP Publishing (`pubs.aip.org`), nicht bei Wiley. Im Katalog der Bibliothek
  nach „Journal of the Acoustical Society of America“ suchen; ist JASA im Abo, sind alle JASA-Artikel unten auf einmal
  frei („Access through your institution“ auf der Verlagsseite).
- Bücher und Tagungsbände über den Katalog bzw. Fernleihe.
- Kein Einzelkauf, solange ein Bibliothekszugang absehbar ist; muss es früher sein, zuerst Weinreich 1977 und
  Hundley 1978.

## Vorrang (S155, nach SQ2 a)

Nach SQ2 a haben Verstimmung, γ, δ und `highDecay` schon Grenzen aus Quellen. **Offen ohne Quelle sind nur `decay`,
`lateDecay` (zwei Stufen je Taste an einem Flügel) und u (ungleicher Stoß).** Danach richtet sich der Vorrang:

1. **Hundley, Benioff, Martin 1978** — der Titel nennt genau das Fehlende (mehrstufiges Abklingen); einziger Kauf, der
   sich vor einem Bibliothekszugang lohnen könnte.
2. **Martin 1947** — Abklingraten über die Tastatur; ergänzt 1.
3. **Weinreich 1977** — Mechanismus, Zahlen nur an wenigen Tönen zu erwarten; die KTH-Vorlesung deckt das Qualitative.
4. **Askenfelt & Jansson III** — einzige Aussicht auf eine Messung zu u.
5. Alles Übrige nur bei Bibliothekszugang (Kirk, Chaigne & Askenfelt II, Conklin, Capleton: die Größen sind schon belegt).

**Bis die Dokumente da sind:** nichts wartet darauf. Gesetzt wird nach SQ2 a mit Faktor 3 bzw. 0…1 für `decay`,
`lateDecay` und u, im Bericht als „ohne Quelle“ benannt. Kommt eine Quelle, werden ihre Zahlen nachgelesen, die Grenzen
in `modellierung/hammer-saite/grenzen.md` und im Konzept nachgetragen und die gesetzten Werte dagegen geprüft — neu
gesetzt wird nur, wo einer außerhalb liegt (Kipppunkt von SQ2 a).

**Spur für später (ungeprüft):** In den Graphen von Connected Papers um Martin 1947 und Kirk 1959 (Screenshots Patrik,
S155) stehen außerdem Schuck & Young 1943 („Observations on the Vibrations of Piano Strings“), Railsback 1938, Young
1952, Fletcher 1962 und 1964, Suzuki 1990, Hall 1986–1988, Cartling 2005, Lattard 1993, Giordano 2015, Hinrichsen 2012.
Nicht gesucht, ob frei; Connected Papers braucht dafür keine Suchen — die Nummer im Link ist die von Semantic Scholar,
deren Schnittstelle (`api.semanticscholar.org/graph/v1/paper/<nummer>/citations|references`) frei abfragbar ist.

## Modellierung — Hammer-Saite (Anlass: S155, SQ1 a, Grenzen beim Setzen)

Recherchen dazu: `modellierung/hammer-saite/recherche/2026-10-25_S153_zweite_polarisation.md` und
`…/2026-10-26_S155_bereich_*.md`.

| Vorrang | Quelle | Verlagsseite | Wofür (Größe in `HammerSaite`) | Stand |
|---|---|---|---|---|
| 3 | G. Weinreich, „Coupled piano strings“, JASA 62, 1474–1484 (1977) | [10.1121/1.381677](https://doi.org/10.1121/1.381677) | `lateDecay`, Verstimmung, γ/δ, Stoß | Bezahlschranke |
| 1 | Hundley, Benioff, Martin, „Factors contributing to the multiple rate of piano tone decay“, JASA 64, 1303–1309 (1978) | [10.1121/1.382116](https://doi.org/10.1121/1.382116) | `decay`, `lateDecay` | Bezahlschranke |
| 2 | Martin, „Decay Rates of Piano Tones“, JASA 19, 535–541 (1947) | [10.1121/1.1916516](https://doi.org/10.1121/1.1916516) | `decay`, `lateDecay` | Bezahlschranke |
| 5 | Kirk, „Tuning Preferences for Piano Unison Groups“, JASA 31 (1959) | [Artikel S. 1644](https://pubs.aip.org/jasa/article/31/12/1644/600354/Tuning-Preferences-for-Piano-Unison-Groups); Crossref nennt dazu [10.1121/1.1930231](https://doi.org/10.1121/1.1930231) mit S. 1565 (vermutlich der Tagungsauszug) | Verstimmung im Chor | nur Abstract gesehen |
| 5 | Chaigne, Askenfelt, „Numerical simulations of piano strings. II. Comparisons with measurements …“, JASA 95, 1631–1640 (1994) | [10.1121/1.408549](https://doi.org/10.1121/1.408549) | `highDecay` (b1, b3 für C2/C4/C7) | Bezahlschranke; Teil I ([10.1121/1.408459](https://doi.org/10.1121/1.408459)) liegt lokal als Kopie einer Kursseite |
| 5 | Askenfelt, Jansson, „From touch to string vibrations. I: Timing in the grand piano action“, JASA 88, 52–63 (1990) | [10.1121/1.399933](https://doi.org/10.1121/1.399933) | Stoß je Saite | Bezahlschranke |
| 5 | Askenfelt, Jansson, „… II: The motion of the key and hammer“, JASA 90, 2383–2393 (1991) | [10.1121/1.402043](https://doi.org/10.1121/1.402043) | Stoß je Saite | Bezahlschranke |
| 4 | Askenfelt, Jansson, „… III: String motion and spectra“, JASA 93, 2181–2196 (1993) | [10.1121/1.406680](https://doi.org/10.1121/1.406680) | Stoß je Saite | Bezahlschranke |
| 5 | Conklin, „Design and tone in the mechanoacoustic piano. Part I. Piano hammers and tonal effects“, JASA 99, 3286–3296 (1996) | [10.1121/1.414947](https://doi.org/10.1121/1.414947) | Stoß, Hammer | Bezahlschranke |
| 5 | Conklin, „… Part II. Piano structure“, JASA 100, 695–708 (1996) | [10.1121/1.416233](https://doi.org/10.1121/1.416233) | Boden | Bezahlschranke |
| 5 | Conklin, „… Part III. Piano strings and scale design“, JASA 100, 1286–1298 (1996) | [10.1121/1.416017](https://doi.org/10.1121/1.416017) | Saiten | Bezahlschranke |
| 5 | Capleton, „False beats in coupled piano string unisons“, JASA 115, 885–892 (2004) | [10.1121/1.1636461](https://doi.org/10.1121/1.1636461) | Verstimmung | ein Agent zitiert S. 888 — Zugang ungeprüft |
| 5 | Mori, Kishi, Nakamura, ICA 2004 (Kyoto); Naganuma, Nakamura, ISMA 2004 (Nara) | — (online nicht gefunden) | γ, δ an E1 | japanische Tagungsbände |
| 5 | Tanaka, Mizutani, Nagai 1996 (ASA-Tagung) | — (keine DOI gefunden) | γ, δ | nur Abstract |
| 5 | Valette, Cuesta 1993 (Buch, Titel ungeprüft: *Mécanique de la corde vibrante*) | — | `highDecay` | nicht online |
| 5 | Fletcher, Rossing, *The Physics of Musical Instruments*, Kap. 12 | — | alle | Buch |
| 5 | Capleton, *The Physics of Piano Unisons* (Buch, Titel nur aus einer Händlerangabe) | — | Verstimmung | Buch |
