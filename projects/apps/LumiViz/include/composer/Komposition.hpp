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
#include "composer/Pegelform.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <array>
#include <cstdint>
#include <vector>

namespace lumi::composer
{

/// Eine Blende am Clip: die eingestellte Dauer und die Form. Die Seite
/// (Anfang oder Ende) bestimmt die Richtung.
struct ClipBlende
{
    std::int64_t dauerMs = 0;  ///< eingestellt; 0 = keine Blende an dieser Seite
    Pegelform form = einfacheBlende();
};

/// Ein Stueck mitten im Clip: Lage und Weite wie eingestellt, dazu die Form.
struct ClipStueck
{
    double lage = 0.5;         ///< Mitte des Stuecks als Anteil der Clip-Laenge, ab Clip-Anfang
    std::int64_t weiteMs = 0;  ///< eingestellt
    bool gekippt = false;      ///< an der x-Achse gespiegelt: oben und unten getauscht
    Pegelform form = delle();
};

/// Ein Punkt der Pegelkurve, wie `.lvcomp` sie bis S95 trug (Feld `levels`):
/// Zeit im Clip, Pegel, Form des Abschnitts bis zum naechsten Punkt.
struct AltPunkt
{
    std::int64_t zeitMs = 0;
    double pegel = 1.0;
    Kurvenform form = Kurvenform::Linear;
    BezierGriffe bezier = kBezierVorgabe;
};

struct VisualClip
{
    QString datei;  ///< Preset-Datei, absolut (in der Datei relativ zur `.lvcomp`, wo moeglich)
    std::int64_t startMs = 0;
    std::int64_t endeMs = 0;  ///< ausschliesslich: der Clip laeuft in [start, ende)
    Mischart mischart = Mischart::Normal;

    // --- Pegelkurve (Entscheide Patrik S96): alle Teile sind freiwillig ------
    /// Der Pegel zwischen den Teilen, 0…1. Die Blenden laufen von null bis
    /// hierher; eine Delle senkt von hier in Richtung null, ein Buckel hebt
    /// von hier in Richtung voll.
    double grundpegel = 1.0;
    ClipBlende ein;
    ClipBlende aus;
    std::vector<ClipStueck> stuecke;

    [[nodiscard]] std::int64_t dauerMs() const { return endeMs - startMs; }
    /// Wo die Teile bei der jetzigen Clip-Laenge liegen — gestaucht, wo sie
    /// sich ueberdecken. Die eingestellten Werte bleiben, wie sie sind.
    [[nodiscard]] Verteilung verteilung() const;
    /// Pegel an der Stelle `zeitImClipMs` (0 = Clip-Start).
    [[nodiscard]] double pegelBei(std::int64_t zeitImClipMs) const;
    /// Ein- und Ausblenden mit einer einfachen Form setzen: von 0 auf den
    /// Grundpegel in `einMs`, zurueck auf 0 in den letzten `ausMs`. 0 = kein
    /// Blenden an diesem Ende. Die Stuecke bleiben.
    void setzeBlenden(std::int64_t einMs, std::int64_t ausMs,
                      Kurvenform form = Kurvenform::Linear);
    /// Laeuft der Clip durchweg auf vollem Pegel — keine Blende, kein Stueck,
    /// Grundpegel 1?
    [[nodiscard]] bool vollerPegel() const;
};

/// Eine alte Punktkurve in die Teile des Clips uebersetzen (ersetzt Blenden,
/// Stuecke und Grundpegel). Das Muster einer einfachen Ein- / Ausblendung wird
/// zu Blenden; jede andere Kurve zu EINEM Stueck ueber die ganze Clip-Laenge —
/// der Pegelverlauf bleibt derselbe.
void ausAltenPunkten(VisualClip& clip, std::vector<AltPunkt> punkte);

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
    /// fruehere endet), Blenden und Stuecke klemmen. Meldet, was gerichtet
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
