/**
 * @file InstrumentEditor.hpp
 * @brief Loop: der Instrument-Editor — die Noten einer Basis ueber einer
 *        Klaviatur, als eigenes Fenster (docs/audio/Loop_Konzept.md §4f,
 *        Stufe 9b; Skizze gebilligt, Patrik S110)
 *
 * Oeffnet sich ueber „Edit base" an einem Instrument-Clip. **Kopf:** Basis und
 * Zahl ihrer Clips, Takte des Musters, „Play pattern", die Spitze der Summe.
 * **Instrument-Zeile:** Instrument, Lautstaerke, Panorama, Stumm. **Raster:**
 * links die Klaviatur (C1 bis C7), rechts die Schritte; eine Note ist ein
 * Balken in der Zeile ihrer Tonhoehe. Darunter die Lautstaerke je Note.
 *
 * Wie der Drum-Editor aendert er den Loop nicht selbst: er meldet die neue
 * Fassung mit `bearbeitet`, das Loop-Panel fuehrt sie ueber Undo aus und gibt
 * sie mit `setLoop` zurueck.
 */
#pragma once

#include "loop/Loop.hpp"

#include <QPoint>
#include <QString>
#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QScrollBar;
class QSlider;
class QSpinBox;
class QToolButton;

namespace lumi::loop
{

/// Das Raster der Noten. Es rollt selbst — waagrecht (lange Muster) und
/// senkrecht (sechs Oktaven): Klaviatur, Kopf und die Zeile der Lautstaerken
/// bleiben stehen.
///
/// Ein Klick auf eine freie Stelle setzt eine Note von einem Schritt; mit
/// gedrueckter Taste nach rechts gezogen wird sie laenger. Eine Note zieht man
/// an ihrer rechten Kante laenger und kuerzer, sonst verschiebt man sie in
/// Zeit und Tonhoehe. Rechtsklick oder Entf loescht. Ein Klick auf eine Taste
/// verlangt ihren Ton. In der unteren Zeile zieht man die Lautstaerke der
/// Noten, die an dem Schritt beginnen. Gemeldet wird beim Loslassen, einmal.
class NotenRaster : public QWidget
{
    Q_OBJECT

public:
    explicit NotenRaster(QWidget* parent = nullptr);

    void setMuster(const Muster& muster, int schlaegeJeTakt, int schritteJeSchlag);
    [[nodiscard]] const Muster& muster() const { return m_muster; }
    /// Die Abspielmarke: der Schritt im Muster (mit Bruchteil); unter null = keine.
    void setMarke(double schritt);
    /// Die gewaehlte Note (Platz in `muster().noten`); -1 = keine.
    [[nodiscard]] int auswahl() const;
    bool loescheAuswahl();

    // --- Rollen ---------------------------------------------------------------
    void setRollen(int x, int y);
    [[nodiscard]] int rollX() const { return m_rollX; }
    [[nodiscard]] int rollY() const { return m_rollY; }
    /// So weit laesst sich rollen (0 = alles ist zu sehen).
    [[nodiscard]] int rollXMax() const;
    [[nodiscard]] int rollYMax() const;
    /// Breite und Hoehe des Ausschnitts, in dem die Noten liegen.
    [[nodiscard]] int sichtBreite() const { return width() - kTastenBreite; }
    [[nodiscard]] int sichtHoehe() const { return height() - kKopfHoehe - kLautHoehe; }
    /// So rollen, dass diese Tonhoehe in der Mitte liegt.
    void zeigeHoehe(int hoehe);

    // --- Geometrie (auch fuer Tests) ------------------------------------------
    static constexpr int kTastenBreite = 44;  ///< die Klaviatur links
    static constexpr int kKopfHoehe = 18;     ///< die Zeile mit den Takten
    static constexpr int kZeilenHoehe = 14;   ///< je Tonhoehe
    static constexpr int kSchrittBreite = 16;
    static constexpr int kLautHoehe = 56;     ///< die Lautstaerke je Note, unten
    static constexpr int kKante = 5;          ///< so breit ist der Griff am Ende einer Note mindestens
    static constexpr int kKanteMax = 9;       ///< und hoechstens (ein Drittel der Note, dazwischen)
    /// Liegt `x` auf dem Griff am Ende dieser Note? Dort zieht man ihre Laenge;
    /// der Mauszeiger zeigt es.
    [[nodiscard]] bool anKante(const Note& note, int x) const;
    [[nodiscard]] int xVonSchritt(int schritt) const { return kTastenBreite + schritt * kSchrittBreite - m_rollX; }
    /// Der Schritt unter `x`; -1 = keiner (die Klaviatur, hinter dem Ende).
    [[nodiscard]] int schrittBei(int x) const;
    [[nodiscard]] int yVonHoehe(int hoehe) const { return kKopfHoehe + (kHoeheMax - hoehe) * kZeilenHoehe - m_rollY; }
    /// Die Tonhoehe unter `y`; -1 = keine (der Kopf, die Zeile der Lautstaerken).
    [[nodiscard]] int hoeheBei(int y) const;
    /// Oberkante der Zeile mit der Lautstaerke je Note.
    [[nodiscard]] int lautOben() const { return height() - kLautHoehe; }
    /// Die Note unter dem Punkt (Platz in `muster().noten`); -1 = keine.
    [[nodiscard]] int noteBei(QPoint ort) const;
    [[nodiscard]] QSize sizeHint() const override { return {kTastenBreite + 64 * kSchrittBreite, 420}; }
    [[nodiscard]] QSize minimumSizeHint() const override { return {kTastenBreite + 8 * kSchrittBreite, 200}; }

Q_SIGNALS:
    /// Die Noten sind geaendert: das Muster danach und ein Wort fuer Undo.
    void geaendert(const lumi::loop::Muster& neu, const QString& was);
    /// Eine Taste der Klaviatur ist angeklickt.
    void tonVerlangt(int hoehe);
    /// Das Rad hat gerollt (die Rollbalken des Fensters ziehen nach).
    void gerollt();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    [[nodiscard]] int schritte() const { return m_muster.takte * m_schlaege * m_jeSchlag; }
    [[nodiscard]] bool belegt(int schritt, int hoehe, int ausser) const;
    void zieheBei(QPoint ort);

    Muster m_muster;
    int m_schlaege = 4;
    int m_jeSchlag = 4;
    double m_marke = -1.0;
    int m_rollX = 0;
    int m_rollY = 0;
    /// Die gewaehlte Note, an ihrem Einsatz erkannt (die Plaetze aendern sich
    /// beim Sortieren); Schritt -1 = keine.
    int m_wahlSchritt = -1;
    int m_wahlHoehe = -1;

    /// Was die gedrueckte Maus gerade tut.
    enum class Zug
    {
        Keiner,
        Laenge,       ///< die rechte Kante der gewaehlten Note
        Verschieben,  ///< die gewaehlte Note in Zeit und Tonhoehe
        Laut,         ///< die Lautstaerke der Noten unter der Maus
    };
    Zug m_zug = Zug::Keiner;
    int m_griffSchritt = 0;  ///< beim Verschieben: so weit hinter dem Einsatz wurde die Note gegriffen
    Muster m_vorZug;
    bool m_neu = false;      ///< der Zug begann mit einer neuen Note
    int m_taste = -1;        ///< die Taste der Klaviatur, die gerade gedrueckt ist; -1 = keine
    /// Den Mauszeiger setzen: an der Kante einer Note der Doppelpfeil, auf ihr
    /// das Kreuz, auf der Klaviatur die Hand.
    void zeigeZeiger(QPoint ort);
};

class InstrumentEditor : public QWidget
{
    Q_OBJECT

public:
    explicit InstrumentEditor(QWidget* parent = nullptr);

    /// Diese Basis des Loops zeigen. false = es gibt sie nicht (oder sie ist
    /// kein Instrument) — der Editor bleibt, wie er war.
    bool zeige(const Loop& loop, const QString& muster);
    /// Eine andere Fassung desselben Loops (nach einer Aenderung, nach Undo).
    /// Heisst die Basis jetzt anders, folgt der Editor ihr; gibt es sie nicht
    /// mehr, schliesst er sich.
    void setLoop(const Loop& loop);
    [[nodiscard]] const QString& muster() const { return m_muster; }
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    [[nodiscard]] NotenRaster* raster() const { return m_raster; }

    /// Spielt gerade dieses Muster allein — und wo steht es (Schritt im Muster)?
    void setSpielt(bool spielt, double schritt);
    /// Die Spitze der Summe, solange etwas spielt; unter null = keine Anzeige.
    void setSpitze(double spitze);
    /// Ein Satz unter der Instrument-Zeile: warum die Tasten gerade stumm
    /// bleiben (der Composer hat den Ton, der Loop spielt). Leer = kein Hinweis.
    void setHinweis(const QString& text);

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    void setzeTakte(int takte);
    void setzeInstrument(const QString& name);
    void setzeLaut(double laut);
    void setzePan(double pan);
    void setzeStumm(bool stumm);

Q_SIGNALS:
    /// Eine Aenderung ist fertig: die neue Fassung des Loops und ein Wort fuer Undo.
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);
    /// „Play pattern" / „Stop": dieses Muster allein spielen, oder anhalten.
    void spielenVerlangt(bool spielen);
    /// Eine Taste der Klaviatur: diesen Ton des Instruments anspielen.
    void tonVerlangt(int hoehe);

protected:
    /// Beim ersten Zeigen: der Ausschnitt liegt auf den Noten (ohne Noten um C4).
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void zeigeAusschnitt();
    void zeigeAlles();
    void passeRollbalkenAn();
    [[nodiscard]] const Muster* basis() const { return m_loop.findeMuster(m_muster); }

    Loop m_loop;
    QString m_muster;
    bool m_spielt = false;
    bool m_gezeigt = false;  ///< der Ausschnitt ist schon einmal gesetzt

    QLabel* m_basisName = nullptr;
    QLabel* m_nutzer = nullptr;
    QSpinBox* m_takte = nullptr;
    QPushButton* m_play = nullptr;
    QLabel* m_spitze = nullptr;
    QComboBox* m_instrument = nullptr;
    QSlider* m_laut = nullptr;
    QSlider* m_pan = nullptr;
    QToolButton* m_stumm = nullptr;
    QLabel* m_hinweis = nullptr;
    NotenRaster* m_raster = nullptr;
    QScrollBar* m_rollX = nullptr;
    QScrollBar* m_rollY = nullptr;
};

}  // namespace lumi::loop
