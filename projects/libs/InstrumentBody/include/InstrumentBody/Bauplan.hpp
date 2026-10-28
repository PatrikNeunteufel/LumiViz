/**
 * @file Bauplan.hpp
 * @brief Der Bauplan eines Instrumentkoerpers: Materialien, Platte mit Umriss und
 *        Dicke, Rippen, Stege, die Punkte, an denen die Saiten den Steg treffen
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S147: BM1 a,
 *        BM2 a, BM3 a, GB1 a, BM5 a)
 *
 * Der Bauplan ist die **Quelle** eines Koerpers: von Hand oder von Claude
 * geschrieben, in der Datei `.lvinstbodyplan`. Das Werkzeug `InstrumentBodyTool`
 * rechnet daraus den **Koerper** (`.lvinstbody`, die Moden); die App liest nur
 * Koerper (BM5 a). Lesen und Schreiben der Datei liegt in der App
 * (`loop/KoerperBauplan`), hier steht nur, was ein Bauplan ist.
 *
 * **Einheiten SI:** Meter, Kilogramm je Kubikmeter, Pascal. **Lage:** x nach
 * rechts, y nach oben, der Ursprung wie im Bauplan festgelegt — beim Steinway D
 * nach RT-0425 die untere linke Ecke des Bodens (§2.3).
 *
 * **Jedes Teil traegt sein Material** (GB1 a): Holz mit Faserrichtung
 * (orthotrop) oder ein Werkstoff ohne (isotrop, etwa Glas). Die Faserrichtung
 * gilt fuer die Platte; die Rippen liegen quer dazu, die Stege laufen frei.
 *
 * **Rippen und Stege** sind Leisten auf der Platte: eine Mittellinie, je Punkt
 * Hoehe (ueber der Platte, ohne sie) und Breite. Die Rechnung nimmt sie wie
 * MAESSTRO und Chabassier als dickere Stellen der Platte (BM1 a).
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek; keine Ein- und Ausgabe.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <string>
#include <vector>

namespace lumi::koerper
{

/// Die Fassung des Bauplans, die dieses Programm schreibt; eine hoehere liest es nicht.
/// 2 (S148, BM6 a): Hoerorte. Fassung 1 bleibt lesbar (ohne Hoerorte).
/// 3 (S160, HU4 a): die Rueckseite (Kurzschluss ueber den Rand). Fassung 1 und 2 bleiben lesbar (ohne Rueckseite).
inline constexpr int kBauplanFassung = 3;

/// Ein Punkt der Ebene in Metern.
struct Punkt2
{
    double x = 0.0;
    double y = 0.0;

    [[nodiscard]] bool operator==(const Punkt2&) const = default;
};

enum class MaterialArt : int
{
    Orthotrop = 0,  ///< Holz: Faser (x) und quer dazu (y) verschieden
    Isotrop,        ///< in jeder Richtung gleich: Glas, Acryl, Metall
};

/// Ein Werkstoff. Bei `Isotrop` gelten nur `dichte`, `ex` (E) und `nuxy` (ν);
/// die uebrigen Groessen folgen daraus (`vervollstaendigt`).
struct Material
{
    std::string name;  ///< wie die Teile es nennen; im Bauplan eindeutig
    MaterialArt art = MaterialArt::Orthotrop;
    double dichte = 0.0;  ///< kg/m³
    double ex = 0.0;      ///< Pa, in Faserrichtung
    double ey = 0.0;      ///< Pa, quer zur Faser in der Ebene
    double gxy = 0.0;     ///< Pa, Schub in der Ebene
    double gxz = 0.0;     ///< Pa, Querschub (Reissner-Mindlin)
    double gyz = 0.0;     ///< Pa, Querschub
    double nuxy = 0.0;    ///< Querzahl
    /// Verlustfaktor η des Werkstoffs; 0 = keine Angabe (dann gilt, was die
    /// Rechnung fuer den Koerper festlegt).
    double verlustfaktor = 0.0;
    std::string quelle;  ///< woher die Werte stammen

    [[nodiscard]] bool operator==(const Material&) const = default;
};

/// Ein isotroper Werkstoff mit allen Groessen: G = E / (2 (1 + ν)).
[[nodiscard]] inline Material vervollstaendigt(Material m)
{
    if (m.art != MaterialArt::Isotrop) return m;
    m.ey = m.ex;
    m.gxy = m.ex / (2.0 * (1.0 + m.nuxy));
    m.gxz = m.gxy;
    m.gyz = m.gxy;
    return m;
}

/// Werte auf einem gleichmaessigen Raster, zeilenweise (x innen, y aussen).
/// NaN = an dieser Stelle kein Wert (ausserhalb des Umrisses).
struct Raster
{
    Punkt2 ursprung;       ///< die Mitte der Zelle (0, 0)
    double schritt = 0.0;  ///< m
    int nx = 0;
    int ny = 0;
    std::vector<double> werte;  ///< nx·ny Werte

    [[nodiscard]] bool leer() const { return nx <= 0 || ny <= 0 || werte.empty(); }
    [[nodiscard]] double an(int i, int j) const { return werte[static_cast<std::size_t>(j) * nx + i]; }

    /// Der Wert an `p`, bilinear zwischen den vier Nachbarn; fehlt einer (NaN),
    /// zaehlen nur die vorhandenen. NaN = kein Nachbar hat einen Wert oder `p`
    /// liegt mehr als eine Zelle ausserhalb.
    [[nodiscard]] double wertBei(Punkt2 p) const
    {
        if (leer() || schritt <= 0.0) return std::numeric_limits<double>::quiet_NaN();
        const double fx = (p.x - ursprung.x) / schritt;
        const double fy = (p.y - ursprung.y) / schritt;
        if (fx < -1.0 || fy < -1.0 || fx > nx || fy > ny) return std::numeric_limits<double>::quiet_NaN();
        const int i0 = static_cast<int>(std::floor(fx));
        const int j0 = static_cast<int>(std::floor(fy));
        const double tx = fx - i0;
        const double ty = fy - j0;
        double summe = 0.0;
        double gewichte = 0.0;
        for (int dj = 0; dj <= 1; ++dj)
        {
            for (int di = 0; di <= 1; ++di)
            {
                const int i = std::clamp(i0 + di, 0, nx - 1);
                const int j = std::clamp(j0 + dj, 0, ny - 1);
                const double w = (di ? tx : 1.0 - tx) * (dj ? ty : 1.0 - ty);
                const double v = an(i, j);
                if (std::isnan(v) || w <= 0.0) continue;
                summe += w * v;
                gewichte += w;
            }
        }
        return gewichte > 0.0 ? summe / gewichte : std::numeric_limits<double>::quiet_NaN();
    }

    [[nodiscard]] bool operator==(const Raster& o) const
    {
        // NaN ist nie gleich NaN — fuer den Vergleich der Datei zaehlt „beide leer"
        if (!(ursprung == o.ursprung) || schritt != o.schritt || nx != o.nx || ny != o.ny) return false;
        if (werte.size() != o.werte.size()) return false;
        for (std::size_t k = 0; k < werte.size(); ++k)
        {
            if (std::isnan(werte[k]) && std::isnan(o.werte[k])) continue;
            if (werte[k] != o.werte[k]) return false;
        }
        return true;
    }
};

/// Die Platte: Umriss, Material, Faserrichtung, Dicke.
struct Platte
{
    std::string material;
    /// Winkel der Faser gegen die x-Achse, Grad, gegen den Uhrzeiger.
    double faserWinkelGrad = 0.0;
    /// Geschlossener Umriss (der erste Punkt wird nicht wiederholt).
    std::vector<Punkt2> umriss;
    /// Dicke in m, wo das Raster keinen Wert hat (oder ohne Raster ueberall).
    double dicke = 0.0;
    Raster dickeRaster;  ///< leer = ueberall `dicke`

    /// Die Dicke an `p` in m.
    [[nodiscard]] double dickeBei(Punkt2 p) const
    {
        const double v = dickeRaster.wertBei(p);
        return std::isnan(v) ? dicke : v;
    }

    [[nodiscard]] bool operator==(const Platte&) const = default;
};

/// Eine Leiste auf der Platte: Rippe oder Steg.
struct Leiste
{
    std::string name;
    std::string material;
    std::vector<Punkt2> linie;   ///< die Mittellinie, mindestens zwei Punkte
    std::vector<double> hoehe;   ///< m, je Punkt der Linie, ueber der Platte
    std::vector<double> breite;  ///< m, je Punkt der Linie

    [[nodiscard]] bool operator==(const Leiste&) const = default;
};

/// Wo die Saiten einer Taste den Steg treffen.
struct Stegpunkt
{
    int taste = 0;  ///< MIDI-Nummer (A0 = 21 … C8 = 108)
    Punkt2 ort;
    std::string quelle;  ///< etwa „RT-0425 Anhang A" oder „verlaengert"

    [[nodiscard]] bool operator==(const Stegpunkt&) const = default;
};

/// Ein Ort, an dem der Koerper gehoert wird (BM6 a): je Hoerort traegt der Koerper ein
/// Abstrahlgewicht je Mode — die Ausgaenge nach KB1 a.
struct Hoerort
{
    std::string name;  ///< im Bauplan eindeutig, etwa „left"
    Punkt2 ort;        ///< m, im Bezugssystem des Bodens
    double hoehe = 0.0;  ///< m, z ueber der Mittelflaeche der Platte (die Ebene, in der die Rechnung sie nimmt)
    std::string quelle;  ///< wie die Lage zustande kam

    [[nodiscard]] bool operator==(const Hoerort&) const = default;
};

/// Die Rueckseite der Platte im Rayleigh-Integral (HU4 a, S160): ohne Schallwand strahlt auch die
/// Unterseite, mit umgekehrtem Vorzeichen, und erreicht den Hoerort ueber den naechsten Randpunkt —
/// in der Platte bis zum Rand, die Randtiefe hinab und wieder hinauf, dann zum Hoerort. Bei tiefen
/// Frequenzen loescht sie die fernen Anteile der Vorderseite aus (akustischer Kurzschluss); oberhalb
/// `schattenHz` wird sie mit 1/(1 + j f/schattenHz) abgeschattet (Beugung ueber den Rand).
/// `randtiefe` 0 = keine Rueckseite, wie die Platte in der Schallwand (BM6 a).
struct Rueckseite
{
    double randtiefe = 0.0;   ///< m, Tiefe des Rands (der Zarge) unter der Platte; 0 = aus
    double schattenHz = 0.0;  ///< Hz, ab hier faellt die Rueckseite mit erster Ordnung; 0 = kein Schatten
    std::string quelle;       ///< wie die Werte zustande kamen

    [[nodiscard]] bool an() const { return randtiefe > 0.0; }
    [[nodiscard]] bool operator==(const Rueckseite&) const = default;
};

/// Die modale Daempfung α(f) = a f² + b f + c in 1/s, f in Hz (RT-0425 Gl. 1, gelesen als Polynom in f —
/// Befund S148). Seit HU4-1 a (S160) eine Eingabe des Bauplans: der Steinway D traegt c = 4,5 1/s, damit die
/// tiefsten Moden die an Resonanzboeden gemessenen 1–3 % erreichen (Ege, Boutillon & Rebillat 2013, Mittel
/// 2,3 %) — RT-0425 mit c = 0 gibt dort 1,1 %. Fuer einen Werkstoff mit eigenem Verlustfaktor (> 0) gilt
/// sein Wert statt des Gesetzes (DM1 a, `verlustfaktorAusAnteilen`).
struct Daempfung
{
    double a = 2.0e-5;
    double b = 0.07;
    double c = 0.0;
    std::string quelle;  ///< wie die Werte zustande kamen

    [[nodiscard]] double alpha(double frequenzHz) const { return a * frequenzHz * frequenzHz + b * frequenzHz + c; }
    /// Verlustfaktor η = α / (2π f).
    [[nodiscard]] double verlustfaktor(double frequenzHz) const
    {
        return frequenzHz > 0.0 ? alpha(frequenzHz) / (2.0 * std::numbers::pi * frequenzHz) : 0.0;
    }

    [[nodiscard]] bool operator==(const Daempfung&) const = default;
};

struct Bauplan
{
    std::string titel;
    std::string beschreibung;
    std::string quelle;
    std::vector<Material> materialien;
    Platte platte;
    std::vector<Leiste> rippen;
    std::vector<Leiste> stege;
    std::vector<Stegpunkt> stegpunkte;  ///< nach Taste aufsteigend
    std::vector<Hoerort> hoerorte;      ///< seit Fassung 2; leer = keiner
    Rueckseite rueckseite;              ///< seit Fassung 3; Vorgabe aus (Schallwand)
    Daempfung daempfung;                ///< seit Fassung 3; Vorgabe RT-0425 Gl. 1 (c = 0)

    [[nodiscard]] bool operator==(const Bauplan&) const = default;
};

/// Der naechste Punkt des geschlossenen Umrisses zu `p` und sein Abstand (auf den Kanten des Polygons).
struct Randpunkt
{
    Punkt2 punkt;
    double abstand = 0.0;
};

[[nodiscard]] inline Randpunkt naechsterRandpunkt(const std::vector<Punkt2>& umriss, Punkt2 p)
{
    Randpunkt best{p, std::numeric_limits<double>::infinity()};
    const std::size_t n = umriss.size();
    for (std::size_t k = 0; k < n; ++k)
    {
        const Punkt2& a = umriss[k];
        const Punkt2& b = umriss[(k + 1) % n];
        const double dx = b.x - a.x, dy = b.y - a.y;
        const double laenge2 = dx * dx + dy * dy;
        double t = laenge2 > 0.0 ? ((p.x - a.x) * dx + (p.y - a.y) * dy) / laenge2 : 0.0;
        t = std::clamp(t, 0.0, 1.0);
        const Punkt2 q{a.x + t * dx, a.y + t * dy};
        const double d = std::sqrt((p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y));
        if (d < best.abstand) best = {q, d};
    }
    return best;
}

// --- Geometrie -------------------------------------------------------------------

/// Der Werkstoff dieses Namens; nullptr = gibt es nicht.
[[nodiscard]] inline const Material* findeMaterial(const Bauplan& plan, const std::string& name)
{
    for (const Material& m : plan.materialien)
    {
        if (m.name == name) return &m;
    }
    return nullptr;
}

/// Die Flaeche eines geschlossenen Umrisses (Gausssche Trapezformel), immer positiv.
[[nodiscard]] inline double flaeche(const std::vector<Punkt2>& umriss)
{
    double doppelt = 0.0;
    const std::size_t n = umriss.size();
    for (std::size_t k = 0; k < n; ++k)
    {
        const Punkt2& a = umriss[k];
        const Punkt2& b = umriss[(k + 1) % n];
        doppelt += a.x * b.y - b.x * a.y;
    }
    return std::abs(doppelt) / 2.0;
}

/// Liegt `p` im geschlossenen Umriss? (gerade-ungerade-Regel)
[[nodiscard]] inline bool innen(const std::vector<Punkt2>& umriss, Punkt2 p)
{
    bool drin = false;
    const std::size_t n = umriss.size();
    for (std::size_t k = 0, l = n - 1; k < n; l = k++)
    {
        const Punkt2& a = umriss[k];
        const Punkt2& b = umriss[l];
        if ((a.y > p.y) != (b.y > p.y) && p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x) drin = !drin;
    }
    return drin;
}

/// Der kuerzeste Abstand von `p` zu einem Linienzug, in m; unendlich bei leerem Zug.
[[nodiscard]] inline double abstandZurLinie(Punkt2 p, const std::vector<Punkt2>& linie)
{
    double best = std::numeric_limits<double>::infinity();
    if (linie.size() == 1) return std::hypot(p.x - linie[0].x, p.y - linie[0].y);
    for (std::size_t k = 0; k + 1 < linie.size(); ++k)
    {
        const Punkt2& a = linie[k];
        const Punkt2& b = linie[k + 1];
        const double dx = b.x - a.x;
        const double dy = b.y - a.y;
        const double laenge2 = dx * dx + dy * dy;
        const double t = laenge2 > 0.0 ? std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / laenge2, 0.0, 1.0) : 0.0;
        best = std::min(best, std::hypot(p.x - (a.x + t * dx), p.y - (a.y + t * dy)));
    }
    return best;
}

/// Die Laenge eines Linienzugs in m.
[[nodiscard]] inline double laenge(const std::vector<Punkt2>& linie)
{
    double summe = 0.0;
    for (std::size_t k = 0; k + 1 < linie.size(); ++k) summe += std::hypot(linie[k + 1].x - linie[k].x, linie[k + 1].y - linie[k].y);
    return summe;
}

/// Die Masse einer Leiste in kg: je Stueck Laenge mal mittlerer Querschnitt mal Dichte.
[[nodiscard]] inline double masse(const Leiste& leiste, double dichte)
{
    double summe = 0.0;
    for (std::size_t k = 0; k + 1 < leiste.linie.size(); ++k)
    {
        const double l = std::hypot(leiste.linie[k + 1].x - leiste.linie[k].x, leiste.linie[k + 1].y - leiste.linie[k].y);
        const double q0 = leiste.hoehe[k] * leiste.breite[k];
        const double q1 = leiste.hoehe[k + 1] * leiste.breite[k + 1];
        summe += l * (q0 + q1) / 2.0;
    }
    return summe * dichte;
}

/// Die Masse der Platte in kg: die Dicke auf einem Raster von `schritt` Metern
/// ueber den Umriss summiert.
[[nodiscard]] inline double plattenMasse(const Bauplan& plan, double schritt = 0.005)
{
    const Material* m = findeMaterial(plan, plan.platte.material);
    if (!m || plan.platte.umriss.size() < 3 || schritt <= 0.0) return 0.0;
    double xmin = plan.platte.umriss[0].x, xmax = xmin, ymin = plan.platte.umriss[0].y, ymax = ymin;
    for (const Punkt2& p : plan.platte.umriss)
    {
        xmin = std::min(xmin, p.x);
        xmax = std::max(xmax, p.x);
        ymin = std::min(ymin, p.y);
        ymax = std::max(ymax, p.y);
    }
    double volumen = 0.0;
    for (double y = ymin + schritt / 2.0; y < ymax; y += schritt)
    {
        for (double x = xmin + schritt / 2.0; x < xmax; x += schritt)
        {
            if (innen(plan.platte.umriss, {x, y})) volumen += plan.platte.dickeBei({x, y}) * schritt * schritt;
        }
    }
    return volumen * m->dichte;
}

/// Die Masse des ganzen Bodens in kg: Platte, Rippen, Stege.
[[nodiscard]] inline double masse(const Bauplan& plan)
{
    double summe = plattenMasse(plan);
    for (const auto* leisten : {&plan.rippen, &plan.stege})
    {
        for (const Leiste& l : *leisten)
        {
            if (const Material* m = findeMaterial(plan, l.material)) summe += masse(l, m->dichte);
        }
    }
    return summe;
}

/// Was an einem Bauplan nicht stimmt, je Befund ein Satz; leer = in Ordnung.
/// Geprueft: Materialien eindeutig und mit positiven Werten, jedes genannte
/// Material vorhanden, Umriss mit mindestens drei Punkten und Flaeche, Dicke
/// positiv, je Leiste mindestens zwei Punkte und so viele Hoehen und Breiten
/// wie Punkte (alle positiv), Stegpunkte nach Taste aufsteigend und jede Taste einmal,
/// Hoerorte mit eindeutigem Namen und ueber der Platte.
[[nodiscard]] inline std::vector<std::string> pruefe(const Bauplan& plan)
{
    std::vector<std::string> befunde;
    for (std::size_t k = 0; k < plan.materialien.size(); ++k)
    {
        const Material m = vervollstaendigt(plan.materialien[k]);
        if (m.name.empty()) befunde.push_back("material without name");
        for (std::size_t l = k + 1; l < plan.materialien.size(); ++l)
        {
            if (plan.materialien[l].name == m.name) befunde.push_back("material '" + m.name + "' twice");
        }
        if (m.dichte <= 0.0 || m.ex <= 0.0 || m.ey <= 0.0 || m.gxy <= 0.0 || m.gxz <= 0.0 || m.gyz <= 0.0)
            befunde.push_back("material '" + m.name + "': density and moduli must be positive");
        // isotrop: 0 ≤ ν < 0,5; orthotrop in der Ebene: 1 − ν_xy·ν_yx > 0 mit ν_yx = ν_xy·Ey/Ex
        const bool nuGut = m.art == MaterialArt::Isotrop
                               ? m.nuxy >= 0.0 && m.nuxy < 0.5
                               : m.nuxy >= 0.0 && m.ex > 0.0 && m.nuxy * m.nuxy * m.ey / m.ex < 1.0;
        if (!nuGut) befunde.push_back("material '" + m.name + "': poisson ratio out of range");
        if (m.verlustfaktor < 0.0) befunde.push_back("material '" + m.name + "': loss factor negative");
    }
    if (!findeMaterial(plan, plan.platte.material)) befunde.push_back("plate: material '" + plan.platte.material + "' unknown");
    if (plan.platte.umriss.size() < 3 || flaeche(plan.platte.umriss) <= 0.0) befunde.push_back("plate: outline needs three points and an area");
    if (plan.platte.dicke <= 0.0) befunde.push_back("plate: thickness must be positive");
    const Raster& r = plan.platte.dickeRaster;
    if (!r.leer() && (r.schritt <= 0.0 || r.werte.size() != static_cast<std::size_t>(r.nx) * r.ny))
        befunde.push_back("plate: thickness grid has the wrong size");
    for (const double v : r.werte)
    {
        if (!std::isnan(v) && v <= 0.0)
        {
            befunde.push_back("plate: thickness grid has a value that is not positive");
            break;
        }
    }
    const auto pruefeLeisten = [&](const std::vector<Leiste>& leisten, const std::string& art) {
        for (const Leiste& l : leisten)
        {
            const std::string wer = art + " '" + l.name + "'";
            if (!findeMaterial(plan, l.material)) befunde.push_back(wer + ": material '" + l.material + "' unknown");
            if (l.linie.size() < 2) befunde.push_back(wer + ": line needs two points");
            if (l.hoehe.size() != l.linie.size() || l.breite.size() != l.linie.size())
            {
                befunde.push_back(wer + ": height and width need one value per point");
                continue;
            }
            for (std::size_t k = 0; k < l.linie.size(); ++k)
            {
                if (l.hoehe[k] < 0.0 || l.breite[k] <= 0.0)
                {
                    befunde.push_back(wer + ": height must not be negative, width must be positive");
                    break;
                }
            }
        }
    };
    pruefeLeisten(plan.rippen, "rib");
    pruefeLeisten(plan.stege, "bridge");
    for (std::size_t k = 1; k < plan.stegpunkte.size(); ++k)
    {
        if (plan.stegpunkte[k].taste <= plan.stegpunkte[k - 1].taste)
        {
            befunde.push_back("bridge points: keys must ascend, each once");
            break;
        }
    }
    for (std::size_t k = 0; k < plan.hoerorte.size(); ++k)
    {
        const Hoerort& h = plan.hoerorte[k];
        if (h.name.empty()) befunde.push_back("listening point without name");
        for (std::size_t l = k + 1; l < plan.hoerorte.size(); ++l)
        {
            if (plan.hoerorte[l].name == h.name) befunde.push_back("listening point '" + h.name + "' twice");
        }
        // auf der Platte oder darunter gibt es kein Rayleigh-Integral (Halbraum ueber der Platte)
        if (!(h.hoehe > 0.0)) befunde.push_back("listening point '" + h.name + "': must lie above the plate");
    }
    if (plan.rueckseite.randtiefe < 0.0) befunde.push_back("backside: rim depth must not be negative");
    if (plan.rueckseite.schattenHz < 0.0) befunde.push_back("backside: shadow frequency must not be negative");
    if (plan.daempfung.a < 0.0 || plan.daempfung.b < 0.0 || plan.daempfung.c < 0.0)
        befunde.push_back("damping: a, b and c must not be negative");
    return befunde;
}

}  // namespace lumi::koerper
