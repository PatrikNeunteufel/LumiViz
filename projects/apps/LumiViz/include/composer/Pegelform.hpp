/**
 * @file Pegelform.hpp
 * @brief Composer: die Form einer Blende oder eines Mittelstuecks der
 *        Pegelkurve, ohne Zeit — und wie die Teile eines Clips Platz finden
 *        (docs/ui/Composer_Konzept.md §9, Stufe C4 Teil 2)
 *
 * Eine `Pegelform` ist eine Kurve in Anteilen: x = 0…1 ueber ihre Weite,
 * y = Pegel 0…1. Dauer, Weite und Lage gehoeren nicht zu ihr — sie sind
 * Parameter am Clip (Vorgabe Patrik S96). So passt dieselbe Form auf jede
 * Laenge und laesst sich als Preset halten.
 *
 * Zwei Arten:
 * - **Blende** — laeuft von (0,0) nach (1,1). Als Ausblendung wird sie an der
 *   y-Achse gespiegelt (zeitlich rueckwaerts).
 * - **Stueck** — ein Teil mitten im Clip. An der x-Achse gespiegelt tauschen
 *   oben und unten: aus einer Delle wird ein Buckel (Pegel → 1 − Pegel).
 *
 * `verteile` legt Ein-, Ausblendung und Stuecke in einen Clip: jedes Teil
 * behaelt seine eingestellte Weite, bis sich Teile beruehren; die sich
 * beruehrenden werden gleichmaessig gestaucht. Die eingestellten Werte bleiben
 * dabei unangetastet (Entscheid Patrik S96) — `verteile` rechnet nur.
 *
 * Reines Modell: kein Widget, kein GL.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Komposition.hpp"

#include <QJsonObject>

#include <cstdint>
#include <vector>

namespace lumi::composer
{

struct FormPunkt
{
    double x = 0.0;  ///< Anteil der Weite, 0…1
    double y = 0.0;  ///< Pegel, 0…1
    Kurvenform form = Kurvenform::Linear;  ///< Form des Abschnitts BIS zum naechsten Punkt
    BezierGriffe bezier = kBezierVorgabe;  ///< nur bei `form == Bezier`
};

enum class FormArt : int
{
    Blende = 0,  ///< von (0,0) nach (1,1)
    Stueck = 1,  ///< mitten im Clip; beginnt bei x = 0 und endet bei x = 1
};

struct Pegelform
{
    FormArt art = FormArt::Blende;
    /// Nach x sortiert, mindestens zwei Punkte (nach `richte`).
    std::vector<FormPunkt> punkte;

    /// Pegel an der Stelle `x` (0…1) — so, wie die Form gezeichnet ist.
    [[nodiscard]] double wert(double x) const;
    /// Sortieren, klemmen, die Enden setzen: eine Blende beginnt bei (0,0)
    /// und endet bei (1,1); ein Stueck hat je einen Punkt bei x = 0 und x = 1
    /// (fehlt er, kommt einer mit vollem Pegel dazu).
    void richte();
};

/// Eine Blende mit einem Abschnitt in dieser Form — das, was C4 Teil 1 kannte.
[[nodiscard]] Pegelform einfacheBlende(Kurvenform form = Kurvenform::Linear,
                                       const BezierGriffe& bezier = kBezierVorgabe);
/// Die Vorgabe fuer ein Stueck: eine weiche Delle von voll auf null und zurueck.
[[nodiscard]] Pegelform delle();

/// Pegel einer Blende am Anteil `x` ihrer Dauer. `amEnde` = als Ausblendung:
/// an der y-Achse gespiegelt, sie laeuft von 1 nach 0.
[[nodiscard]] double blendenPegel(const Pegelform& form, double x, bool amEnde);
/// Pegel eines Stuecks am Anteil `x` seiner Weite. `gekippt` = an der x-Achse
/// gespiegelt: oben und unten getauscht.
[[nodiscard]] double stueckPegel(const Pegelform& form, double x, bool gekippt);

[[nodiscard]] bool gleich(const Pegelform& a, const Pegelform& b);

[[nodiscard]] QJsonObject formAlsJson(const Pegelform& form);
/// false = kein Objekt mit Punkten; die Form kommt gerichtet zurueck.
bool formAusJson(const QJsonObject& objekt, Pegelform& form);

// --- Platz im Clip ------------------------------------------------------------

/// Ein Stueck am Clip, wie es eingestellt ist.
struct StueckLage
{
    double lage = 0.5;           ///< Mitte des Stuecks als Anteil der Clip-Laenge, ab Clip-Anfang
    std::int64_t weiteMs = 0;    ///< eingestellte Weite
};

/// Wo ein Teil im Clip liegt, in Millisekunden ab Clip-Anfang.
struct Bereich
{
    std::int64_t vonMs = 0;
    std::int64_t bisMs = 0;
    bool gestaucht = false;  ///< schmaler als eingestellt

    [[nodiscard]] std::int64_t weiteMs() const { return bisMs - vonMs; }
};

struct Verteilung
{
    Bereich ein;
    Bereich aus;
    std::vector<Bereich> stuecke;  ///< in der Reihenfolge der Eingabe
};

/**
 * Die Teile in einen Clip der Laenge `dauerMs` legen. Die Einblendung haengt
 * am Anfang, die Ausblendung am Ende, jedes Stueck mit seiner Mitte am Anteil
 * `lage` (ragt es ueber eine Kante, rueckt es hinein). Ueberdecken sich Teile,
 * teilen sie den Platz, den sie zusammen belegen, im Verhaeltnis ihrer
 * eingestellten Weiten — die Einblendung zuvorderst, die Ausblendung zuhinterst,
 * die Stuecke nach ihrer Lage.
 */
[[nodiscard]] Verteilung verteile(std::int64_t dauerMs, std::int64_t einMs, std::int64_t ausMs,
                                  const std::vector<StueckLage>& stuecke = {});

}  // namespace lumi::composer
