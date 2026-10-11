/**
 * @file DrumEditor.hpp
 * @brief Loop: der Drum-Editor — das Schritt-Raster einer Basis, als eigenes
 *        Fenster (docs/audio/Loop_Konzept.md §4d, Stufe 6b)
 *
 * Oeffnet sich ueber „Edit base" am Clip (Entscheid Patrik S108, L8 a: die
 * Aenderung gilt fuer alle Clips der Basis). **Kopf:** Basis und Zahl ihrer
 * Clips, Takte des Musters, „Play pattern", die Spitze der Summe (L6 a).
 * **Raster des ganzen Loops:** Schlaege je Takt, Schritte je Schlag, Swing.
 * **Spuren:** je Spur Stumm, Name, Klang, Lautstaerke, Panorama und ihre
 * Schritte; darunter die Lautstaerke je Schritt der angeklickten Spur.
 *
 * Der Editor aendert den Loop nicht selbst: er meldet die neue Fassung mit
 * `bearbeitet`, das Loop-Panel fuehrt sie ueber Undo aus und gibt sie mit
 * `setLoop` zurueck.
 */
#pragma once

#include "loop/Loop.hpp"

#include <QPoint>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <vector>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QScrollArea;
class QSlider;
class QSpinBox;
class QToolButton;
class QVBoxLayout;

namespace lumi::loop
{

/// Das Raster: je Spur eine Zeile Schritte, darunter die Lautstaerke je Schritt
/// der angeklickten Spur. Ein Klick schaltet einen Schritt an (volle
/// Lautstaerke) oder aus; mit gedrueckter Taste weitergezogen bekommen die
/// ueberfahrenen Schritte derselben Spur denselben Zustand. In der unteren
/// Zeile zieht man die Lautstaerke. Gemeldet wird beim Loslassen, einmal.
class SchrittRaster : public QWidget
{
    Q_OBJECT

public:
    explicit SchrittRaster(QWidget* parent = nullptr);

    void setMuster(const Muster& muster, int schlaegeJeTakt, int schritteJeSchlag);
    [[nodiscard]] const Muster& muster() const { return m_muster; }
    void setSpur(int spur);
    [[nodiscard]] int spur() const { return m_spur; }
    /// Die Abspielmarke: der Schritt im Muster (mit Bruchteil); unter null = keine.
    void setMarke(double schritt);

    // --- Geometrie (auch fuer Tests) ----------------------------------------
    static constexpr int kKopfHoehe = 18;    ///< die Zeile mit den Takten
    static constexpr int kZeilenHoehe = 26;  ///< je Spur — so hoch wie ihre Kopfzeile links
    static constexpr int kLuftHoehe = 30;    ///< zwischen den Spuren und der Lautstaerke (links: „Add track")
    static constexpr int kLautHoehe = 52;    ///< die Lautstaerke je Schritt
    static constexpr int kSchrittBreite = 12;
    static constexpr int kSchrittLuft = 2;
    static constexpr int kSchlagLuft = 5;    ///< zusaetzlich zwischen zwei Schlaegen
    [[nodiscard]] int xVonSchritt(int schritt) const;
    /// Der Schritt unter `x`; -1 = keiner (eine Luecke, ausserhalb).
    [[nodiscard]] int schrittBei(int x) const;
    [[nodiscard]] int yVonSpur(int spur) const { return kKopfHoehe + spur * kZeilenHoehe; }
    /// Oberkante der Zeile mit der Lautstaerke je Schritt.
    [[nodiscard]] int lautOben() const;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override { return sizeHint(); }

Q_SIGNALS:
    void spurGewaehlt(int spur);
    /// Die Schritte sind geaendert: das Muster danach und ein Wort fuer Undo.
    void geaendert(const lumi::loop::Muster& neu, const QString& was);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    [[nodiscard]] int schritte() const;
    void maleBei(QPoint ort);

    Muster m_muster;
    int m_schlaege = 4;
    int m_jeSchlag = 4;
    int m_spur = -1;
    double m_marke = -1.0;

    /// Was die gedrueckte Maus gerade tut.
    enum class Zug
    {
        Keiner,
        Setzen,   ///< Schritte anschalten
        Loeschen, ///< Schritte ausschalten
        Laut,     ///< die Lautstaerke je Schritt ziehen
    };
    Zug m_zug = Zug::Keiner;
    int m_zugSpur = -1;
    Muster m_vorZug;
};

class DrumEditor : public QWidget
{
    Q_OBJECT

public:
    explicit DrumEditor(QWidget* parent = nullptr);

    /// Diese Basis des Loops zeigen. false = es gibt sie nicht (oder sie ist
    /// kein Drumkit) — der Editor bleibt, wie er war.
    bool zeige(const Loop& loop, const QString& muster);
    /// Eine andere Fassung desselben Loops (nach einer Aenderung, nach Undo).
    /// Heisst die Basis jetzt anders, folgt der Editor ihr; gibt es sie nicht
    /// mehr, schliesst er sich.
    void setLoop(const Loop& loop);
    [[nodiscard]] const QString& muster() const { return m_muster; }
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    [[nodiscard]] SchrittRaster* raster() const { return m_raster; }

    /// Die Klaenge des Kits fuer die Auswahl je Spur (`kit:<Name>`).
    void setKitKlaenge(const QStringList& klaenge);
    /// Spielt gerade dieses Muster allein — und wo steht es (Schritt im Muster)?
    void setSpielt(bool spielt, double schritt);
    /// Die Spitze der Summe, solange etwas spielt; unter null = keine Anzeige.
    void setSpitze(double spitze);

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -----------------
    void setzeTakte(int takte);
    void setzeRaster(int schlaegeJeTakt, int schritteJeSchlag);
    void setzeSwing(double swing);
    void spurHinzu();
    void spurLoeschen(int spur);
    void spurUmbenennen(int spur, const QString& name);
    void setzeKlang(int spur, const QString& klang);
    void setzeLaut(int spur, double laut);
    void setzePan(int spur, double pan);
    void setzeStumm(int spur, bool stumm);

Q_SIGNALS:
    /// Eine Aenderung ist fertig: die neue Fassung des Loops und ein Wort fuer Undo.
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);
    /// „Play pattern" / „Stop": dieses Muster allein spielen, oder anhalten.
    void spielenVerlangt(bool spielen);

protected:
    /// Ein Klick in die Kopfzeile einer Spur waehlt sie.
    bool eventFilter(QObject* objekt, QEvent* ereignis) override;

private:
    /// Die Bedienelemente einer Spur, links neben ihrer Zeile im Raster.
    struct SpurZeile
    {
        QWidget* reihe = nullptr;
        QToolButton* stumm = nullptr;
        QLabel* name = nullptr;
        QComboBox* klang = nullptr;
        QSlider* laut = nullptr;
        QSlider* pan = nullptr;
    };
    void setupUI();
    /// Kopf, Raster und Spuren aus `m_loop` neu fuellen.
    void zeigeAlles();
    void baueSpuren(const Muster& muster);
    void fuelleKlang(QComboBox* auswahl, const QString& klang) const;
    void spurMenue(int spur, QPoint ort);
    void waehleSpur(int spur);
    [[nodiscard]] const Muster* basis() const { return m_loop.findeMuster(m_muster); }

    Loop m_loop;
    QString m_muster;
    QStringList m_kitKlaenge;
    bool m_spielt = false;

    QLabel* m_basisName = nullptr;
    QLabel* m_nutzer = nullptr;
    QSpinBox* m_takte = nullptr;
    QPushButton* m_play = nullptr;
    QLabel* m_spitze = nullptr;
    QSpinBox* m_schlaege = nullptr;
    QSpinBox* m_jeSchlag = nullptr;
    QDoubleSpinBox* m_swing = nullptr;
    QVBoxLayout* m_spurSpalte = nullptr;
    std::vector<SpurZeile> m_spuren;
    QPushButton* m_spurNeu = nullptr;
    QLabel* m_lautName = nullptr;
    QLabel* m_leerHinweis = nullptr;  ///< steht da, solange das Muster keine Spur hat
    QScrollArea* m_rollen = nullptr;
    SchrittRaster* m_raster = nullptr;
};

}  // namespace lumi::loop
