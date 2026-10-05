/**
 * @file Komposition.hpp
 * @brief Composer: das Datenmodell einer Komposition und die Datei `.lvcomp`
 *        (docs/ui/Composer_Konzept.md §6, Stufe C1b)
 *
 * Eine Komposition ist eine Zeitachse in Millisekunden mit Kanaelen; in den
 * Kanaelen liegen Clips. Ein Visual-Clip verweist auf eine Preset-Datei
 * (`.avs`, `.milk`, `.lvfx`, `.lvfx2`) — eingebettet wird nichts (Entscheid
 * Patrik S60). Kein Qt-Widget, kein GL: reines Modell.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Mischer.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <array>
#include <cstdint>
#include <vector>

namespace lumi::composer
{

/// Wie der Pegel von einem Punkt zum naechsten laeuft. Die Formen sind die
/// des Crossfades der Host-Gruppe, dazu „Halten" (Sprung am naechsten Punkt).
enum class Kurvenform : int
{
    Linear = 0,
    SKurve = 1,
    EaseIn = 2,
    EaseOut = 3,
    Exponentiell = 4,
    Halten = 5,
    Bezier = 6,  ///< kubische Bézier-Kurve mit zwei Anfassern (C4, Wunsch Patrik S95)
};

inline constexpr int kKurvenformen = 7;

/// Die zwei Anfasser einer Bézier-Kurve: x1, y1, x2, y2 — je 0…1, x = Anteil
/// der Zeit, y = Anteil des Wegs (wie `cubic-bezier` in CSS). Die Kurve laeuft
/// von (0,0) nach (1,1).
using BezierGriffe = std::array<double, 4>;
/// Die Vorgabe: eine weiche S-Kurve.
inline constexpr BezierGriffe kBezierVorgabe = {0.4, 0.0, 0.6, 1.0};

/// Anteil 0…1 des Wegs nach Anteil `t` 0…1 der Zeit. `Bezier` braucht seine
/// Anfasser — ohne sie (diese Fassung) laeuft die Form linear.
[[nodiscard]] double kurvenWert(Kurvenform form, double t);
/// Der Weg einer Bézier-Kurve mit diesen Anfassern nach Anteil `t` der Zeit.
[[nodiscard]] double bezierWert(const BezierGriffe& griffe, double t);

struct PegelPunkt
{
    std::int64_t zeitMs = 0;  ///< im Clip, ab Clip-Start
    double pegel = 1.0;       ///< 0…1
    Kurvenform form = Kurvenform::Linear;  ///< Form des Abschnitts BIS zum naechsten Punkt
    BezierGriffe bezier = kBezierVorgabe;  ///< nur bei `form == Bezier`
};

/// Anteil des Wegs im Abschnitt, der bei `punkt` beginnt — mit seinen Anfassern.
[[nodiscard]] double kurvenWert(const PegelPunkt& punkt, double t);

struct VisualClip
{
    QString datei;  ///< Preset-Datei, absolut (in der Datei relativ zur `.lvcomp`, wo moeglich)
    std::int64_t startMs = 0;
    std::int64_t endeMs = 0;  ///< ausschliesslich: der Clip laeuft in [start, ende)
    Mischart mischart = Mischart::Normal;
    /// Pegelkurve, nach Zeit sortiert. Leer = voller Pegel. Vor dem ersten
    /// Punkt gilt dessen Pegel, nach dem letzten der letzte.
    std::vector<PegelPunkt> kurve;

    [[nodiscard]] std::int64_t dauerMs() const { return endeMs - startMs; }
    /// Pegel an der Stelle `zeitImClipMs` (0 = Clip-Start).
    [[nodiscard]] double pegelBei(std::int64_t zeitImClipMs) const;
    /// Ein- und Ausblenden als Kurve setzen (ersetzt die Kurve): von 0 auf 1
    /// in `einMs`, von 1 auf 0 in den letzten `ausMs`. 0 = kein Blenden an
    /// diesem Ende. Ueberlappen sich beide, treffen sie sich in der Mitte.
    void setzeBlenden(std::int64_t einMs, std::int64_t ausMs,
                      Kurvenform form = Kurvenform::Linear);
};

struct AudioClip
{
    QString datei;
    std::int64_t startMs = 0;    ///< auf der Zeitachse
    std::int64_t einsatzMs = 0;  ///< ab dieser Stelle im Song
};

enum class KanalArt : int
{
    Visual = 0,
    Audio = 1,
};

struct Kanal
{
    KanalArt art = KanalArt::Visual;
    QString name;
    bool an = true;
    std::vector<VisualClip> visual;  ///< nur bei Visual, nach Start sortiert, ueberlappungsfrei
    std::vector<AudioClip> audio;    ///< nur bei Audio
};

struct Komposition
{
    /// Reihenfolge = Stapelreihenfolge der Visual-Kanaele, unten zuerst.
    std::vector<Kanal> kanaele;
    /// 0 = das Ende folgt dem letzten Clip (Vorgabe); groesser = festes Ende —
    /// die Komposition laeuft bis dorthin, auch wenn kein Clip mehr liegt.
    std::int64_t laengeMs = 0;
    double rasterSekunden = 1.0;  ///< Einrasten beim Ziehen (Bedienung)

    /// Die gesetzte Laenge, sonst das Ende des letzten Visual-Clips.
    [[nodiscard]] std::int64_t laenge() const;
    /// Das Ende des letzten Visual-Clips, gleich ob eine Laenge gesetzt ist.
    [[nodiscard]] std::int64_t clipEnde() const;
    /// Clips je Kanal nach Start sortieren, Unsinn richten (Ende vor Start,
    /// Ueberlappung im selben Kanal: der spaetere Clip beginnt, wo der
    /// fruehere endet), Kurven sortieren und klemmen. Meldet, was gerichtet
    /// wurde.
    void richte(QStringList* bericht = nullptr);
};

/// Die Komposition als JSON. `basisOrdner` nicht leer: Dateien darunter
/// werden relativ geschrieben.
[[nodiscard]] QJsonObject alsJson(const Komposition& komposition, const QString& basisOrdner = {});
/// JSON lesen; relative Dateien werden gegen `basisOrdner` aufgeloest.
/// false = kein Composer-Dokument. Der Bericht nennt, was gerichtet wurde
/// und welche Dateien fehlen (die Clips bleiben trotzdem erhalten).
bool ausJson(const QJsonObject& wurzel, Komposition& komposition, const QString& basisOrdner = {},
             QStringList* bericht = nullptr);

bool speichereKomposition(const Komposition& komposition, const QString& pfad);
bool ladeKomposition(const QString& pfad, Komposition& komposition, QStringList* bericht = nullptr);

}  // namespace lumi::composer
