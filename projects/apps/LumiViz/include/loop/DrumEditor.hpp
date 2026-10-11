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

#include "loop/Auswahl.hpp"
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

    // --- Mehrere Schritte zugleich (Stufe 13b, Skizze §4i) -------------------
    // Markiert wird mit Shift+Ziehen (ein Bereich, auch ueber mehrere Spuren)
    // und Ctrl+Klick (ein Schritt dazu oder heraus). Ein Klick in die Zeile
    // der Takte setzt die Einfuegemarke. Alles Folgende loesen auch Tasten und
    // das Menue des Rechtsklicks aus.
    [[nodiscard]] const std::vector<SchrittOrt>& markierte() const { return m_marken; }
    [[nodiscard]] bool istMarkiert(int spur, int schritt) const;
    void markiere(const std::vector<SchrittOrt>& orte);
    void markiereUm(int spur, int schritt);
    void markiereAlle();
    /// Der Schritt, an dem Ctrl+V einfuegt (0 bis zur Laenge des Musters).
    void setzeEinfuegemarke(int schritt);
    [[nodiscard]] int einfuegemarke() const { return m_einfuegen; }
    [[nodiscard]] bool hatKopie() const { return !m_kopie.leer(); }
    /// Die markierten Schritte merken. false = nichts markiert.
    bool kopiere();
    /// Merken und leeren.
    bool schneideAus();
    /// Das Gemerkte an der Einfuegemarke einfuegen, in der Spur, in der die
    /// kopierte Auswahl begann; die Marke rueckt um die eingefuegte Laenge
    /// weiter. `ersetzen` (Entscheid Patrik S113, V1 a) schreibt auch die
    /// leeren Schritte der Kopie; false legt nur dazu.
    bool fuegeEin(bool ersetzen = true);
    /// Dasselbe an einer bestimmten Stelle („Paste here"); die Marke bleibt.
    bool fuegeEinBei(SchrittOrt ziel, bool ersetzen = true);
    /// Die markierten Schritte direkt hinter sich selbst noch einmal einfuegen.
    bool dupliziere();
    bool leereMarkierte();
    /// Die markierten versetzen; am Rand haelt die ganze Auswahl an (V3 a).
    /// Mehrere Schritte hintereinander gehen von der Stelle aus, an der die
    /// Auswahl beim ersten stand: was sie unterwegs ueberfaehrt, bleibt.
    bool versetze(int spurVersatz, int schrittVersatz);
    /// Die markierten in der Zeit im Kreis versetzen (Alt+Pfeil).
    bool drehe(int schrittVersatz);
    /// Was beim letzten Einfuegen nicht gepasst hat; leer = alles.
    [[nodiscard]] const QString& meldung() const { return m_meldung; }

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
    /// Marken, Einfuegemarke oder Meldung haben sich geaendert.
    void auswahlGeaendert();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    [[nodiscard]] int schritte() const;
    void maleBei(QPoint ort);
    /// Der Schritt, der `x` am naechsten liegt — auch in einer Luecke und am Rand.
    [[nodiscard]] int schrittNahe(int x) const;
    /// Die Spur unter `y`, auf die vorhandenen begrenzt; -1 = es gibt keine.
    [[nodiscard]] int spurNahe(int y) const;
    void rahmeBis(QPoint ort);
    /// Eine eigene Aenderung uebernehmen: zeigen, melden, die Marken setzen.
    void uebernimm(const Muster& neu, const std::vector<SchrittOrt>& marken, const QString& was);
    bool bewege(int spurVersatz, int schrittVersatz, bool imKreis);

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
        Rahmen,   ///< mit Shift: einen Bereich markieren
    };
    Zug m_zug = Zug::Keiner;
    int m_zugSpur = -1;
    Muster m_vorZug;

    std::vector<SchrittOrt> m_marken;
    std::vector<SchrittOrt> m_vorRahmen;  ///< die Marken beim Druck (mit Ctrl bleiben sie)
    SchrittOrt m_rahmenStart;
    int m_einfuegen = 0;
    SchrittKopie m_kopie;
    int m_kopieSpur = 0;  ///< die Spur, in der die kopierte Auswahl begann
    QString m_meldung;
    // Mehrere Pfeiltasten hintereinander: gerechnet wird vom Stand beim ersten
    // Druck, mit dem ganzen Versatz seither.
    Muster m_schiebeBasis;
    std::vector<SchrittOrt> m_schiebeOrte;
    Muster m_schiebeErgebnis;
    std::vector<SchrittOrt> m_schiebeMarken;
    int m_schiebeSpur = 0;
    int m_schiebeSchritt = 0;
    bool m_schiebeImKreis = false;
    bool m_schiebeGilt = false;
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
    QLabel* m_auswahlZeile = nullptr; ///< wie man markiert — oder was markiert ist und was die Tasten tun
    void zeigeAuswahl();
    QScrollArea* m_rollen = nullptr;
    SchrittRaster* m_raster = nullptr;
};

}  // namespace lumi::loop
