/**
 * @file Kontur.hpp
 * @brief Klang aus einer Form: die Zeichnung aus Konturen, ihre Datei
 *        `.lvcontour` und die Reihe, die aus einer Kontur einen Wellenzug macht
 *        (docs/audio/Shape_Konzept.md §5, §7; Stufe 19a; Entscheide Patrik
 *        S120: F1 a, D1 a, D2 a, D3 a, D4 b)
 *
 * Eine **Zeichnung** haelt fest, was im Fenster Edit shape entsteht: alle
 * Konturen, je mit Farbe und dem Haken „gewaehlt", ihre Reihenfolge, die Wahl
 * As layers oder As one path, die Art der Spruenge und die Herkunft (D1 a).
 * Eine **Kontur** ist ein geschlossener Pfad aus Strecken und Kurven in den
 * Koordinaten der Zeichnung (D4 b) und traegt eine **feste Kennung**: ein
 * Instrument nennt die Datei und die Kontur ueber diese Zahl (D3 a) — wird
 * eine Kontur geloescht, verrutschen die anderen nicht.
 *
 * Die Datei liegt wie Instrumente und Samples (D2 a): mitgelieferte unter
 * `asset/contours` und heissen beim Namen (`heart`), eigene im Benutzerordner
 * unter `contours` als `user:<name>`, eine Datei an freiem Ort beim Pfad. Der
 * Ordner liegt nicht unter `loop`: auch die Visuals lesen ihn.
 *
 * Die **Rechnung** (§5): die Kontur wird in gleichen Abstaenden abgetastet
 * (der Stift faehrt gleich schnell), aus den Punkten `z = x + i*y` entstehen
 * die Koeffizienten einer Fourier-Reihe. Mit `harmonische` Teiltoenen
 * zurueckgerechnet, ergibt sie die Form mehr oder weniger genau — das ist der
 * Klangregler Harmonics. In der Datei stehen keine Koeffizienten: sie werden
 * beim Laden gerechnet.
 *
 * **y zeigt nach oben.** Die Zeichnung rechnet wie der Bildschirm (y nach
 * unten); beim Abtasten wird y umgedreht, damit ein Scope, das rechts gegen
 * links zeichnet, die Form aufrecht zeigt.
 *
 * Kein Widget, keine Audio-Engine. `QPainterPath` traegt die Stuecke und
 * laeuft ohne Qt-Anwendung (belegt in test_ShapeBelege.cpp).
 */
#pragma once

#include <QJsonObject>
#include <QPainterPath>
#include <QPointF>
#include <QString>

#include <complex>
#include <cstdint>
#include <vector>

namespace lumi::shape
{

/// Die Fassung, die dieses Programm schreibt; eine hoehere liest es nicht.
inline constexpr int kKonturFassung = 1;
/// Die Endung der Datei, mit Punkt. `.lvshape` gehoert den Stueck-Presets des
/// Composers (F1 a).
inline constexpr const char* kKonturEndung = ".lvcontour";
/// Der Vorsatz einer eigenen Zeichnung: `user:mein_herz`.
inline constexpr const char* kEigenVorsatz = "user:";
/// So viele Konturen traegt eine Zeichnung hoechstens (K2 a: die groessten 64).
inline constexpr int kKonturenMax = 64;
/// So viele Harmonische traegt eine Reihe hoechstens.
inline constexpr int kHarmonischeMax = 512;
/// So fein wird eine Kontur fuer die Reihe abgetastet.
inline constexpr int kAbtastPunkte = 4096;

/// Eine geschlossene Kontur der Zeichnung.
struct Kontur
{
    int kennung = 0;       ///< fest, beim Anlegen vergeben; groesser als 0
    QString name;          ///< wie sie in der Liste steht; leer = „Contour"
    QString farbe;         ///< `#rrggbb`
    bool gewaehlt = true;  ///< der Haken in der Liste
    /// Ein geschlossener Zug aus Strecken und Kurven (kubische Bezier-Stuecke),
    /// in den Koordinaten der Zeichnung — y nach unten, wie der Bildschirm.
    QPainterPath pfad;

    [[nodiscard]] bool operator==(const Kontur&) const = default;
};

/// Was aus mehreren gewaehlten Konturen wird (Z5 a; „Use as" im Fenster).
enum class Nutzung : int
{
    Schichten = 0,  ///< As layers: je Kontur eine Schicht
    EinPfad,        ///< As one path: nacheinander in einem Umlauf
};

/// Wo der Stift bei As one path von einer Kontur zur naechsten springt (B1).
enum class Spruenge : int
{
    Liste = 0,  ///< in Reihenfolge der Liste, an den naechsten Punkten (Vorgabe)
    Kuerzeste,  ///< das Programm verbindet auf den kuerzesten Wegen
    VonHand,    ///< eingezeichnet, `Zeichnung::bruecken`
};

/// Ein von Hand gesetzter Sprung: von einem Punkt einer Kontur zu einem Punkt
/// einer anderen (G2 a).
struct Bruecke
{
    int von = 0;         ///< Kennung der Kontur
    QPointF vonPunkt;    ///< in den Koordinaten der Zeichnung
    int nach = 0;
    QPointF nachPunkt;

    [[nodiscard]] bool operator==(const Bruecke&) const = default;
};

enum class HerkunftArt : int
{
    Gezeichnet = 0,
    Svg,
    Bild,
};

/// Woher die Konturen kamen. Das Bild selbst steht nicht in der Datei (D4 b):
/// liegt es nicht mehr am Ort, bleiben die Konturen, neu erkennen geht erst
/// wieder, wenn es neu gewaehlt ist.
struct Herkunft
{
    HerkunftArt art = HerkunftArt::Gezeichnet;
    QString quelle;             ///< der Name der Quelldatei; leer bei Gezeichnet
    QJsonObject einstellungen;  ///< die Regler der Erkennung (Split by, Levels, …)

    [[nodiscard]] bool operator==(const Herkunft&) const = default;
};

struct Zeichnung
{
    /// Wie eine Schicht sie nennt: der Name einer mitgelieferten (`heart`),
    /// eine eigene (`user:mein_herz`) oder der Pfad einer Datei. Steht nicht
    /// in der Datei.
    QString name;
    QString titel;  ///< wie sie im Programm steht
    /// In der Reihenfolge der Liste; hoechstens `kKonturenMax`.
    std::vector<Kontur> konturen;
    Nutzung nutzung = Nutzung::Schichten;
    Spruenge spruenge = Spruenge::Liste;
    std::vector<Bruecke> bruecken;  ///< nur bei `Spruenge::VonHand` von Belang
    Herkunft herkunft;
    /// Die Kennung, die die naechste neue Kontur bekommt — groesser als jede,
    /// die es in dieser Zeichnung je gab.
    int naechsteKennung = 1;

    /// Eine Kontur anhaengen; sie bekommt die naechste Kennung und, ohne
    /// Angabe, eine Farbe aus der Reihe. Zurueck: ihre Kennung; 0 = die
    /// Zeichnung ist voll oder der Pfad leer.
    int neueKontur(const QPainterPath& pfad, const QString& name = {}, const QString& farbe = {});
    /// Die Kontur mit dieser Kennung; nullptr = es gibt sie nicht (mehr).
    [[nodiscard]] const Kontur* finde(int kennung) const;
    /// Die Kontur entfernen, mit den Bruecken, die an ihr haengen. false = es gab sie nicht.
    bool entferne(int kennung);

    [[nodiscard]] bool operator==(const Zeichnung&) const = default;
};

/// Die Farbe, die die Kontur mit dieser Kennung ohne Angabe bekommt.
[[nodiscard]] QString konturFarbe(int kennung);

// --- der Pfad als Text ---------------------------------------------------------
// Wie die Pfadsprache von SVG, nur mit vier Befehlen und festen Koordinaten:
// `M x y` (Anfang), `L x y` (Strecke), `C x1 y1 x2 y2 x y` (Kurve), `Z`
// (geschlossen). Zahlen so kurz, dass sie genau zurueckkommen.

[[nodiscard]] QString pfadAlsText(const QPainterPath& pfad);
/// false = der Text ist kein Pfad; `pfad` bleibt, wie er war.
bool pfadAusText(const QString& text, QPainterPath& pfad);

// --- die Datei `.lvcontour` ------------------------------------------------------

/// Die Zeichnung als JSON (ohne `name` — der ist, wie eine Schicht sie nennt).
[[nodiscard]] QJsonObject alsJson(const Zeichnung& zeichnung);
/// Liest eine Zeichnung, alles in seinen Grenzen: hoechstens `kKonturenMax`
/// Konturen, jede Kennung einmal, Bruecken nur zwischen Konturen, die es gibt.
/// false = keine Zeichnung (falsches Format) oder eine Fassung, die dieses
/// Programm nicht kennt — dann ist `zuNeu` gesetzt; `zeichnung` bleibt, wie sie
/// war. `name` bleibt.
bool ausJson(const QJsonObject& wurzel, Zeichnung& zeichnung, bool* zuNeu = nullptr);
bool speichereZeichnung(const Zeichnung& zeichnung, const QString& pfad);
/// Liest die Datei; `name` wird der Pfad. Ohne Titel in der Datei gilt ihr Name.
bool ladeZeichnung(const QString& pfad, Zeichnung& zeichnung, bool* zuNeu = nullptr);

// --- wo sie liegt (D2 a) ---------------------------------------------------------

/// Der Ordner der mitgelieferten Formen: `asset/contours`. Leer = nicht gefunden.
[[nodiscard]] QString konturOrdner();
/// Die mitgelieferten Zeichnungen — was im Ordner als `.lvcontour` liegt, nach
/// Titel sortiert; `name` ist der Dateiname ohne Endung. Einmal gelesen und gemerkt.
[[nodiscard]] const std::vector<Zeichnung>& zeichnungen();
/// Nennt `nennung` eine eigene Zeichnung (`user:<name>`)?
[[nodiscard]] bool istEigeneZeichnung(const QString& nennung);
/// Nennt `nennung` eine Datei an freiem Ort (ein Pfad mit der Endung)?
[[nodiscard]] bool istKonturDatei(const QString& nennung);
/// Der Ordner der eigenen Formen (wird nicht angelegt); leer = Qt nennt keinen
/// Benutzerordner.
[[nodiscard]] QString eigenerKonturOrdner();
/// Nur fuer Tests: ein anderer Ordner; leer = wieder der echte.
void setzeEigenenKonturOrdnerFuerTests(const QString& ordner);
/// Aus einem Titel der Name einer eigenen Datei: klein, `a`–`z`, Ziffern und
/// `_` — dieselbe Regel wie bei eigenen Instrumenten und Samples
/// (`loop::eigenerName`). Leer = daraus wird kein Name.
[[nodiscard]] QString eigenerKonturName(const QString& titel);
/// Die Datei einer eigenen Zeichnung; leer = kein gueltiger Name oder kein Ordner.
[[nodiscard]] QString eigeneKonturDatei(const QString& name);
/// Die eigenen Zeichnungen, nach Titel sortiert; `name` ist `user:<name>`.
/// Jedes Mal neu gelesen.
[[nodiscard]] std::vector<Zeichnung> eigeneZeichnungen();
/// Eine eigene Zeichnung in den Benutzerordner schreiben (legt ihn an).
/// `zeichnung.name` wird `user:<name>`. false = kein gueltiger Name oder nicht
/// zu schreiben.
bool speichereEigeneZeichnung(Zeichnung& zeichnung, const QString& name);
/// Eine eigene Zeichnung loeschen. false = es gab keine.
bool loescheEigeneZeichnung(const QString& name);
/// Die Zeichnung, die eine Schicht nennt: eine mitgelieferte beim Namen, eine
/// eigene als `user:<name>` oder eine Datei beim Pfad. false = nicht gefunden
/// oder nicht lesbar.
bool holeZeichnung(const QString& nennung, Zeichnung& zeichnung, bool* zuNeu = nullptr);
/// Zaehlt jedes Schreiben und Loeschen einer Zeichnung und jeden Wechsel des
/// Ordners der eigenen (Stufe 20a): wer eine Zeichnung gelesen hat und sich die
/// Zahl merkt, sieht, ob er neu lesen muss — auch bei gleicher Nennung.
[[nodiscard]] std::uint64_t zeichnungFassung();

// --- die Rechnung (§5) -----------------------------------------------------------

/// Ein Punkt der Form: `x + i*y`, y nach oben.
using Punkt = std::complex<double>;

/// Die Kontur in `punkte` gleich weit entfernten Punkten entlang ihres Zugs —
/// der Stift faehrt gleich schnell. Kurven werden dafuer fein in Strecken
/// zerlegt. y wird umgedreht (nach oben). Leer = kein Pfad, oder weniger als
/// drei Punkte verlangt.
[[nodiscard]] std::vector<Punkt> abtasten(const QPainterPath& pfad, int punkte = kAbtastPunkte);

/// Die Punkte um ihren Schwerpunkt gelegt und so gross gemacht, dass der
/// groesste Ausschlag in x oder y genau 1 ist — fuer eine Kontur allein: kein
/// Gleichanteil im Ton, voller Pegel.
[[nodiscard]] std::vector<Punkt> normiert(const std::vector<Punkt>& punkte);

/// Die Fourier-Reihe eines geschlossenen Zugs.
struct Reihe
{
    Punkt mitte;               ///< der Schwerpunkt (der Koeffizient 0): die Lage der Kontur
    std::vector<Punkt> plus;   ///< die Koeffizienten 1, 2, …
    std::vector<Punkt> minus;  ///< die Koeffizienten −1, −2, …

    [[nodiscard]] int harmonische() const { return static_cast<int>(plus.size()); }
};

/// Die Reihe zu gleich weit entfernten Punkten eines geschlossenen Zugs, bis
/// zur Harmonischen `hoechste` (nicht mehr, als die Punkte hergeben).
[[nodiscard]] Reihe reihe(const std::vector<Punkt>& punkte, int hoechste = kHarmonischeMax);

/// Wie stark die Harmonische `k` bei der Stellung `harmonische` mitspielt
/// (Z4 a, weich): bis `harmonische` voll, die naechste anteilig, darueber nicht.
[[nodiscard]] double gewicht(double harmonische, int k);

/// Der Punkt der Form an der Stelle `umlauf` (0 … 1, ein Umlauf des Stifts)
/// mit `harmonische` Teiltoenen. `mitMitte` = an ihrer Stelle in der
/// Zeichnung; ohne liegt die Form um den Nullpunkt.
[[nodiscard]] Punkt wert(const Reihe& reihe, double umlauf, double harmonische, bool mitMitte = false);

/// Die Form als geschlossener Zug aus `punkte` Punkten, mit `harmonische`
/// Teiltoenen — fuer das Bild.
[[nodiscard]] std::vector<Punkt> umriss(const Reihe& reihe, double harmonische, int punkte, bool mitMitte = false);

}  // namespace lumi::shape
