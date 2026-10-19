/**
 * @file GriffbildBild.hpp
 * @brief Loop: ein Griffbild zeichnen — klein in der Galerie, gross in der Leiste
 *        des Fensters *Chords* und im Dialog *Edit chord* (docs/audio/Saiten_Konzept.md
 *        S-T8 a D, E; S-T11 a; Stufe 26e, S138)
 *
 * Die uebliche Form: die Saiten senkrecht, die tiefste links, oben der Sattel (dick)
 * oder links die Zahl des ersten gezeigten Bunds („fr n" in der Galerie); ueber dem
 * Sattel × fuer nicht gespielt, ○ fuer leer; die Punkte auf den Buenden, gross mit
 * dem Finger, ein Barre als Balken.
 *
 * Kleines und grosses Bild zeichnet derselbe Code; die Lage der Punkte
 * (`GriffbildLage`) rechnet er ohne Qt-Anwendung — der Dialog trifft damit seine
 * Klicks.
 */
#pragma once

#include "loop/Akkord.hpp"

#include <QPointF>
#include <QRectF>
#include <QWidget>

class QPainter;
class QPalette;

namespace lumi::loop
{

/// Welche Buende ein Griffbild zeigt (von Claude gesetzt): mindestens vier, sonst
/// so viele wie der Griff braucht; ab dem Sattel, solange der hoechste gegriffene
/// Bund hineinpasst, sonst ab dem tiefsten gegriffenen.
struct GriffbildFenster
{
    int erster = 1;  ///< der erste gezeigte Bund; 1 = am Sattel
    int anzahl = 4;

    [[nodiscard]] bool amSattel() const { return erster == 1; }
    [[nodiscard]] bool operator==(const GriffbildFenster&) const = default;
};
[[nodiscard]] GriffbildFenster griffbildFenster(const Akkord& akkord);

/// Die Lage der Teile eines Griffbilds in einem Feld.
class GriffbildLage
{
public:
    GriffbildLage(const QRectF& feld, int saiten, GriffbildFenster fenster);

    /// Die Saite senkrecht (x), die tiefste ist 0.
    [[nodiscard]] double xSaite(int saite) const;
    /// Die Linie unter dem Bund `bund` (y); der Sattel liegt bei `erster − 1`.
    [[nodiscard]] double yLinie(int bund) const;
    /// Der Punkt eines gegriffenen Bunds: in der Mitte zwischen zwei Linien.
    [[nodiscard]] QPointF punkt(int saite, int bund) const;
    /// Die Mitte des Zeichens ueber dem Sattel (× oder ○).
    [[nodiscard]] QPointF oben(int saite) const;
    /// Was liegt unter `ort`? Saite und Bund (0 = ueber dem Sattel); nichts ausserhalb.
    struct Treffer
    {
        int saite = -1;
        int bund = -1;

        [[nodiscard]] bool operator==(const Treffer&) const = default;
    };
    [[nodiscard]] std::optional<Treffer> trifft(QPointF ort) const;
    /// Der Halbmesser eines Punkts.
    [[nodiscard]] double halbmesser() const;
    [[nodiscard]] const QRectF& griffbrett() const { return m_brett; }
    [[nodiscard]] const GriffbildFenster& fenster() const { return m_fenster; }

private:
    QRectF m_brett;  ///< von der tiefsten zur hoechsten Saite, vom Sattel bis zur letzten Linie
    int m_saiten = 6;
    GriffbildFenster m_fenster;
    double m_obenHoehe = 0.0;
};

/// Wie gross gezeichnet wird.
enum class GriffbildGroesse : int
{
    Klein = 0,  ///< in der Galerie: ohne Finger, die Bundzahl klein
    Gross = 1,  ///< in der Leiste und im Dialog: mit Fingern
};

/// Das Feld, in dem das Griffbild in `aussen` liegt (mit Rand fuer × / ○ und die
/// Bundzahl).
[[nodiscard]] GriffbildLage griffbildLageIn(const QRectF& aussen, const Akkord& akkord, GriffbildGroesse groesse,
                                            std::optional<GriffbildFenster> fenster = std::nullopt);

/// Das Griffbild in `aussen` zeichnen; Farben aus der Palette (Text fuer Linien,
/// Highlight fuer die Punkte). Ein Voicing zeichnet nichts.
void zeichneGriffbild(QPainter& p, const QRectF& aussen, const Akkord& akkord, GriffbildGroesse groesse,
                      const QPalette& palette, std::optional<GriffbildFenster> fenster = std::nullopt);

/// Ein Griffbild gross, als Widget — in der Leiste des Fensters *Chords* (S-T8 a E)
/// und im Dialog *Edit chord* (S-T11 a).
class GriffbildFeld : public QWidget
{
    Q_OBJECT

public:
    explicit GriffbildFeld(QWidget* parent = nullptr);
    void setzeAkkord(const Akkord& akkord);
    [[nodiscard]] const Akkord& akkord() const { return m_akkord; }
    /// Welche Buende es zeigt; nichts = wie der Griff es braucht (`griffbildFenster`).
    void setzeFenster(std::optional<GriffbildFenster> fenster);
    [[nodiscard]] const std::optional<GriffbildFenster>& fenster() const { return m_fenster; }
    /// Die Lage der Teile, wie sie gerade gezeichnet sind.
    [[nodiscard]] GriffbildLage lage() const;
    [[nodiscard]] QSize sizeHint() const override { return {150, 176}; }
    [[nodiscard]] QSize minimumSizeHint() const override { return {110, 130}; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Akkord m_akkord;
    std::optional<GriffbildFenster> m_fenster;
};

}  // namespace lumi::loop
