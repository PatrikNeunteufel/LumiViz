/**
 * @file BildKonturen.hpp
 * @brief Klang aus einer Form: Konturen aus einem Bild — das Bild in Flaechen
 *        teilen, der Rand jeder Flaeche ist eine Kontur (docs/audio/
 *        Shape_Konzept.md §8; Stufe 20c; Entscheide Patrik K1 a, K2 a, S120)
 *
 * **Der Weg (K1 a):** das Bild wird auf hoechstens `kBildKanteMax` Punkte
 * Kante verkleinert, auf Wunsch weichgezeichnet (**Smooth**) und jeder Punkt
 * einer von `stufen` Klassen zugeteilt — nach **Brightness**, **Colour** oder
 * **Transparency** (**Split by**, **Levels**). Zusammenhaengende Punkte einer
 * Klasse (je vier Nachbarn) sind eine Flaeche; eine Flaeche unter **Min size**
 * geht in ihrem groessten Nachbarn auf. Die Raender der Flaechen werden auf
 * den Kanten der Punkte abgelaufen — jeder Rand ist von selbst geschlossen —
 * und mit Douglas-Peucker so vereinfacht, wie **Detail** es sagt. Ein Rand,
 * den zwei Flaechen teilen (das Loch der einen ist der Umriss der anderen),
 * kommt einmal.
 *
 * **Die Regeln (K2 a):** ein Loch ist eine eigene Kontur; eine Kontur, die an
 * den Bildrand stoesst, kommt mit `rand` und ist anfangs nicht gewaehlt; es
 * bleiben die `kKonturenMax` groessten (nach Flaeche), die anderen zaehlt
 * `weggefallen`; anfangs gewaehlt ist die groesste, die nicht an den Rand
 * stoesst. Mit D4 b ist eine Kontur aus einem Bild ein Pfad nur aus Strecken.
 *
 * Koordinaten: die Punkte des verkleinerten Bilds, y nach unten. Kein Widget,
 * keine Qt-Anwendung (QImage rechnet ohne; nur das Laden von JPEG und anderen
 * Formaten braucht die Plugins einer laufenden Anwendung).
 */
#pragma once

#include <QImage>
#include <QJsonObject>
#include <QPainterPath>
#include <QString>

#include <vector>

namespace lumi::shape
{

/// Zum Erkennen wird das Bild auf hoechstens so viele Punkte Kante verkleinert (K2 a).
inline constexpr int kBildKanteMax = 1024;

/// Wonach das Bild geteilt wird (**Split by**).
enum class Teilung : int
{
    Farbe = 0,   ///< Colour: in `stufen` Farben
    Helligkeit,  ///< Brightness: in `stufen` Stufen
    Deckkraft,   ///< Transparency: in `stufen` Stufen der Deckkraft
};

/// Die Regler der Gruppe From image (K2 a).
struct BildEinstellung
{
    Teilung teilung = Teilung::Helligkeit;
    int stufen = 3;               ///< Levels, 2 … 8
    int glaetten = 1;             ///< Smooth: so viele Punkte weichzeichnen, 0 … 10
    double mindestProzent = 0.2;  ///< Min size: Flaechen darunter (in % des Bilds) gehen im Nachbarn auf, 0 … 5
    int detail = 50;              ///< Detail, 0 … 100: wie genau der Rand nachgefahren wird

    /// Wie weit (in Punkten des Bilds) ein Rand vom Abgelaufenen abweichen darf.
    [[nodiscard]] double toleranz() const;
    [[nodiscard]] QJsonObject alsJson() const;
    /// Liest, was es kennt, in seinen Grenzen; der Rest bleibt, wie er ist.
    void ausJson(const QJsonObject& wurzel);
    [[nodiscard]] bool operator==(const BildEinstellung&) const = default;
};

struct BildKontur
{
    QPainterPath pfad;      ///< geschlossen, nur Strecken
    double flaeche = 0.0;   ///< in Punkten des Bilds
    bool rand = false;      ///< stoesst an den Bildrand
    bool gewaehlt = false;  ///< der Haken, wie K2 a ihn anfangs setzt
};

struct BildKonturen
{
    std::vector<BildKontur> konturen;  ///< die groessten zuerst
    int weggefallen = 0;               ///< so viele kleinere ueber `kKonturenMax` hinaus
    QSize groesse;                     ///< das verkleinerte Bild
};

/// Das Bild zum Erkennen: hoechstens `kBildKanteMax` Kante, ARGB32.
[[nodiscard]] QImage bildZumErkennen(const QImage& bild);

/// Die Konturen eines Bilds, das `bildZumErkennen` vorbereitet hat.
[[nodiscard]] BildKonturen bildKonturen(const QImage& bild, const BildEinstellung& einstellung);

}  // namespace lumi::shape
