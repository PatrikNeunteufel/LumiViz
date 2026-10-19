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

#include <optional>
#include <utility>
#include <vector>

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

// --- Voicings: eine Klaviatur (Stufe 26f, S-T14 a D, E, H) --------------------

/// Die Lage der Tasten einer Klaviatur von `von` bis `bis` (beide Toene, `von` ein C)
/// in einem Feld: weisse Tasten gleich breit, schwarze daruber.
class KlaviaturLage
{
public:
    KlaviaturLage(const QRectF& feld, int von, int bis);
    [[nodiscard]] static bool schwarz(int ton);
    /// Das Rechteck einer Taste; leer ausserhalb.
    [[nodiscard]] QRectF taste(int ton) const;
    /// Wo der Punkt eines Tons sitzt: unten in seiner Taste.
    [[nodiscard]] QPointF punkt(int ton) const;
    /// Der Ton unter dem Punkt (die schwarze Taste zuerst); nichts ausserhalb.
    [[nodiscard]] std::optional<int> tonBei(QPointF ort) const;
    [[nodiscard]] int von() const { return m_von; }
    [[nodiscard]] int bis() const { return m_bis; }

private:
    QRectF m_feld;
    int m_von = 48;
    int m_bis = 71;
    double m_weiss = 1.0;  ///< Breite einer weissen Taste
};
/// Welche Toene eine Klaviatur fuer diese Toene zeigt: ab dem C unter dem tiefsten
/// mindestens `oktaven` Oktaven, bis zum B ueber dem hoechsten.
[[nodiscard]] std::pair<int, int> klaviaturBereich(const std::vector<int>& toene, int oktaven, int ohneToene = 48);
/// Eine Klaviatur mit Punkten auf `toene` zeichnen.
void zeichneKlaviatur(QPainter& p, const QRectF& feld, int von, int bis, const std::vector<int>& toene,
                      GriffbildGroesse groesse, const QPalette& palette);
/// Ein Akkord als Bild: ein Griffbild (schon verschoben) oder ein Voicing als
/// Klaviatur mit dem tiefsten Ton auf `tiefster` (klein zwei, gross drei Oktaven).
void zeichneAkkordbild(QPainter& p, const QRectF& aussen, const Akkord& akkord, int tiefster, GriffbildGroesse groesse,
                       const QPalette& palette);

/// Ein Griffbild gross, als Widget — in der Leiste des Fensters *Chords* (S-T8 a E)
/// und im Dialog *Edit chord* (S-T11 a); ein Voicing als Klaviatur (S-T14 a E).
class GriffbildFeld : public QWidget
{
    Q_OBJECT

public:
    explicit GriffbildFeld(QWidget* parent = nullptr);
    /// `tiefster`: bei einem Voicing der tiefste Ton (sonst ohne Bedeutung).
    void setzeAkkord(const Akkord& akkord, int tiefster = 48);
    [[nodiscard]] const Akkord& akkord() const { return m_akkord; }
    [[nodiscard]] int tiefster() const { return m_tiefster; }
    /// Bei einem Voicing: die Toene, wie sie klingen.
    [[nodiscard]] std::vector<int> toene() const;
    /// Bei einem Voicing: welche Toene die Klaviatur zeigt; nichts = wie die Toene es brauchen.
    void setzeKlaviatur(std::optional<std::pair<int, int>> bereich);
    [[nodiscard]] KlaviaturLage klaviaturLage() const;
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
    int m_tiefster = 48;
    std::optional<GriffbildFenster> m_fenster;
    std::optional<std::pair<int, int>> m_klaviatur;
};

}  // namespace lumi::loop
