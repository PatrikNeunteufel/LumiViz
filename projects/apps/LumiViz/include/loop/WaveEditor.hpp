/**
 * @file WaveEditor.hpp
 * @brief Loop: der **Wave editor** — Datei, Ausschnitt, Lage, Gain, Blenden und die Spur
 *        Expression eines Wave-Musters, als eigenes Fenster (docs/audio/Loop_Konzept.md
 *        §4p, Stufe 15d; Skizze SK15-1 a gebilligt, Patrik S168)
 *
 * Wie Drum- und Instrument-Editor: ein Fenster, die Basis wechselt darin. **Kopf:** Base
 * mit der Zahl ihrer Clips, Fit, Bars, Play pattern, FX, Peak. **Zeile Datei:** File (die
 * Nennung, Laenge, Rate, Kanaele), Browse..., Record..., Edit sample..., Gain, Fade in, Fade
 * out. **Zeile Tempo:** Reference tempo, Set to loop tempo, Follow tempo, Pitch method.
 * **Zeile Raster:** das Raster des Loops (ohne Swing offset). **Flaeche** (`WaveFlaeche`)
 * ueber einem Taktlineal: oben die Spur Expression, in der Mitte die Welle, unten die Spur
 * Autotune.
 *
 * In Stufe 15d noch grau, mit Hinweis: *Follow tempo* (15j), die Spur Autotune (15i),
 * *Record...* (15k). Die Spur Pitch kommt mit 15f, wenn sie wirkt.
 *
 * Er aendert den Loop nicht selbst: er meldet die neue Fassung mit `bearbeitet`, das
 * Loop-Panel fuehrt sie ueber Undo aus und gibt sie mit `setLoop` zurueck.
 */
#pragma once

#include "loop/Bearbeiten.hpp"  // ClipOrt
#include "loop/Loop.hpp"
#include "loop/Tonauftrag.hpp"  // Dekodierer

#include <QString>
#include <QWidget>

#include <functional>
#include <optional>
#include <vector>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QScrollBar;
class QSpinBox;

namespace lumi::loop
{

class RasterZeile;

/// Was von einer Tondatei zu zeigen ist: ihre Spitzen und was sie ist.
struct WaveBild
{
    /// Je `kSpitzenFrames` Abtastwerte der Betrag der groessten Spitze (beide Seiten), 0 … 1.
    std::vector<float> spitzen;
    double sekunden = 0.0;  ///< die Laenge der Datei; 0 = nicht zu lesen
    int rate = 0;           ///< ihre Abtastrate (aus dem Kopf einer WAV); 0 = unbekannt
    int kanaele = 0;        ///< ihre Kanaele (aus dem Kopf einer WAV); 0 = unbekannt

    /// So viele Abtastwerte bei `kBildRate` fasst eine Spitze.
    static constexpr int kSpitzenFrames = 256;
    /// In dieser Rate wird zum Zeigen dekodiert.
    static constexpr int kBildRate = 22050;
};

/// Das Bild einer Datei rechnen: dekodiert ueber `dekodierer` (bis `maxSekunden`), Rate und
/// Kanaele aus dem Kopf, wenn es eine WAV ist. Leer (sekunden 0) = nicht zu lesen.
[[nodiscard]] WaveBild waveBildVon(const QString& datei, const Dekodierer& dekodierer, double maxSekunden = 3600.0);

/// Die Flaeche: Taktlineal, Spur Expression, Welle, Spur Autotune. Sie rollt selbst
/// waagrecht (`setRoll`); ihr Muster haelt sie bis zum Loslassen, dann meldet sie es einmal.
///
/// Welle: der Ausschnitt farbig, der Rest der Datei grau, ausserhalb des Musters
/// schraffiert. An seinen Kanten zieht man den Ausschnitt (der Ton bleibt, wo er ist); in
/// ihm verschiebt man die ganze Welle — sie rastet in Schritten, mit Shift in Bruchteilen.
/// Spur Expression wie im Instrument-Editor: ein Klick setzt einen Punkt auf den Schritt
/// (mit Alt frei) und zieht ihn weiter, ein Punkt laesst sich ziehen, ein Doppelklick nimmt
/// ihn weg; im Menue *Delete point* und *Clear lane*. Ein Klick ins Lineal waehlt die
/// Stelle, an der *Play pattern* beginnt.
class WaveFlaeche : public QWidget
{
    Q_OBJECT

public:
    explicit WaveFlaeche(QWidget* parent = nullptr);

    void setMuster(const Muster& muster, const Loop& loop);
    [[nodiscard]] const Muster& muster() const { return m_muster; }
    void setBild(WaveBild bild);
    [[nodiscard]] const WaveBild& bild() const { return m_bild; }
    /// Die Abspielmarke im Muster (Schritt mit Bruchteil); unter null = keine.
    void setMarke(double schritt);
    [[nodiscard]] double marke() const { return m_marke; }

    // --- Lage und Masse (auch fuer Tests) ----------------------------------------
    [[nodiscard]] double schrittBreite() const { return m_schrittBreite; }
    void setSchrittBreite(double pixel);
    /// Das ganze Muster in der Breite.
    void passeEin();
    /// Der erste sichtbare Schritt.
    [[nodiscard]] double roll() const { return m_roll; }
    void setRoll(double schritt);
    [[nodiscard]] double schritte() const;
    [[nodiscard]] double xVonSchritt(double schritt) const;
    [[nodiscard]] double schrittVonX(double x) const;
    /// Wo die Sekunde `sekunde` der Datei im Muster liegt, in Schritten.
    [[nodiscard]] double schrittVonDatei(double sekunde) const;
    /// Das Ende des Ausschnitts in Sekunden der Datei (bei `ende` 0 ihr Ende).
    [[nodiscard]] double ausschnittEnde() const;

    static constexpr int kLinealHoehe = 18;
    static constexpr int kSpurHoehe = 40;
    static constexpr int kAutotuneHoehe = 30;
    static constexpr int kGriff = 5;  ///< so weit neben einer Kante greift man sie
    [[nodiscard]] int spurOben() const { return kLinealHoehe; }
    [[nodiscard]] int welleOben() const { return kLinealHoehe + kSpurHoehe; }
    [[nodiscard]] int welleUnten() const { return height() - kAutotuneHoehe; }

    /// Der Wert der Spur Expression unter `y`; der Punkt unter dem Ort (−1 = keiner).
    [[nodiscard]] double spurWertVonY(int y) const;
    [[nodiscard]] int spurPunktBei(QPoint ort) const;

Q_SIGNALS:
    /// Ein Zug ist fertig: das neue Muster und ein Wort fuer Undo.
    void geaendert(const lumi::loop::Muster& neu, const QString& was);
    /// Ein Klick ins Lineal: dort beginnt *Play pattern*.
    void abspielstelleGewaehlt(int schritt);
    /// Die Breite eines Schritts hat sich geaendert (Ctrl+Rad, Fit) — der Rollbalken folgt.
    void massGeaendert();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    enum class Zug
    {
        Keiner,
        Anfang,  ///< die linke Kante des Ausschnitts
        Ende,    ///< die rechte Kante
        Lage,    ///< die ganze Welle
        Punkt,   ///< ein Punkt der Spur Expression
    };
    /// Was unter dem Ort in der Welle zu greifen ist.
    [[nodiscard]] Zug griffBei(QPoint ort) const;
    void zeigeZeiger(QPoint ort);
    void zieheWelle(QPoint ort, bool frei);
    void zieheSpurPunkt(QPoint ort, bool frei);
    [[nodiscard]] double schrittMs() const { return m_schrittMs; }
    void melde(const QString& was);

    Muster m_muster;
    Muster m_vorher;  ///< vor dem Zug — gemeldet wird nur, was sich geaendert hat
    double m_schrittMs = 125.0;  ///< ein Schritt beim Tempo des Loops
    int m_schritteJeTakt = 16;
    WaveBild m_bild;
    double m_marke = -1.0;
    double m_schrittBreite = 12.0;
    double m_roll = 0.0;
    Zug m_zug = Zug::Keiner;
    int m_punkt = -1;          ///< beim Ziehen eines Punkts: sein Platz in der Spur
    double m_zugSchritt = 0.0;  ///< wo der Zug begann, in Schritten
};

class WaveEditor : public QWidget
{
    Q_OBJECT

public:
    explicit WaveEditor(QWidget* parent = nullptr);

    /// Diese Basis des Loops zeigen. false = es gibt sie nicht (oder sie ist kein
    /// Wave-Muster) — der Editor bleibt, wie er war.
    bool zeige(const Loop& loop, const QString& muster);
    /// Eine andere Fassung desselben Loops (nach einer Aenderung, nach Undo). Heisst die
    /// Basis jetzt anders, folgt der Editor ihr; gibt es sie nicht mehr, schliesst er sich.
    void setLoop(const Loop& loop);
    [[nodiscard]] const QString& muster() const { return m_muster; }
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    [[nodiscard]] WaveFlaeche* flaeche() const { return m_flaeche; }
    [[nodiscard]] RasterZeile* rasterZeile() const { return m_rasterZeile; }

    /// Womit die Datei zum Zeigen dekodiert wird (das Loop-Panel reicht den der Engine).
    /// Ohne ihn zeigt die Flaeche keine Welle.
    void setDekodierer(Dekodierer dekodierer);

    /// Spielt gerade dieses Muster allein — und wo steht es (Schritt im Muster)?
    void setSpielt(bool spielt, double schritt);
    /// Die Marke allein, ohne den Knopf Play pattern; −1 = keine.
    void zeigeMarke(double schritt);
    /// Die Spitze der Summe, solange etwas spielt; unter null = keine Anzeige.
    void setSpitze(double spitze);
    /// Welcher Clip im Loop markiert ist (E1 a); leer = keiner.
    void setMarkierterClip(std::optional<ClipOrt> ort);
    [[nodiscard]] std::optional<ClipOrt> bezugsclip() const;
    /// Der Knopf FX: der Schalter des Bezugsclips. false = kein Bezugsclip, oder er stand schon so.
    bool setzeEffekteAn(bool an);

    // --- was die Bedienelemente ausloesen (auch fuer Tests) ------------------------
    void setzeTakte(int takte);
    /// Eine Nennung als Datei setzen (`user:`, `lib:` oder ein Pfad).
    void setzeDatei(const QString& nennung);
    /// Was Browse... nach der Rueckfrage S2 a tut: zu den eigenen Samples kopieren und
    /// `user:<name>` setzen, oder den Pfad. false = nicht zu kopieren; das Muster bleibt.
    bool nimmDatei(const QString& pfad, bool kopieren);
    void setzeGain(double db);
    void setzeFadeIn(double ms);
    void setzeFadeOut(double ms);
    void setzeBezugBpm(double bpm);
    /// *Set to loop tempo*.
    void setzeBezugAufLoop();
    void setzeTonhoehenArt(TonhoehenArt art);

    /// Wie die Datei in der Zeile steht: Titel, Laenge, Rate, Kanaele (fuer Tests).
    [[nodiscard]] QString dateiText() const;
    [[nodiscard]] QString dateiInfoText() const;
    /// Der Hinweis hinter Follow tempo: „plays 1.25× as fast when on“.
    [[nodiscard]] QString tempoHinweis() const;

    /// Groesse und Lage merken (wie die anderen Editoren); in der Test-Exe aus.
    static void setzeLageMerken(bool an);

Q_SIGNALS:
    /// Eine Aenderung ist fertig: die neue Fassung des Loops und ein Wort fuer Undo.
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);
    /// „Play pattern" / „Stop".
    void spielenVerlangt(bool spielen);
    /// Ein Klick ins Lineal: spielt das Muster allein, springt es dorthin.
    void abspielstelleGewaehlt(int schritt);
    /// *Edit sample...*: das eigene Sample im Fenster *Record sample* bearbeiten.
    void sampleBearbeitenVerlangt(const QString& nennung);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void setupUI();
    void zeigeAlles();
    void zeigeFx();
    void ladeBild();
    void passeRollbalkenAn();
    [[nodiscard]] const Muster* basis() const { return m_loop.findeMuster(m_muster); }
    /// Das Muster aendern und melden; false = es aenderte sich nichts.
    bool aendere(const QString& was, const std::function<void(Muster&)>& aenderung);

    Loop m_loop;
    QString m_muster;
    bool m_spielt = false;
    bool m_gezeigt = false;
    std::optional<ClipOrt> m_markiert;
    Dekodierer m_dekodierer;
    QString m_bildVon;  ///< fuer welche Datei (und welchen Stand) das Bild gilt

    QLabel* m_basisName = nullptr;
    QLabel* m_nutzer = nullptr;
    QSpinBox* m_takte = nullptr;
    QPushButton* m_play = nullptr;
    QPushButton* m_fx = nullptr;
    QLabel* m_spitze = nullptr;
    QLabel* m_datei = nullptr;
    QLabel* m_dateiInfo = nullptr;
    QPushButton* m_suchen = nullptr;
    QPushButton* m_aufnehmen = nullptr;
    QPushButton* m_sampleBearbeiten = nullptr;
    QDoubleSpinBox* m_gain = nullptr;
    QSpinBox* m_fadeIn = nullptr;
    QSpinBox* m_fadeOut = nullptr;
    QDoubleSpinBox* m_bezug = nullptr;
    QPushButton* m_bezugAufLoop = nullptr;
    QCheckBox* m_folgt = nullptr;
    QLabel* m_folgtHinweis = nullptr;
    QComboBox* m_tonhoehe = nullptr;
    RasterZeile* m_rasterZeile = nullptr;
    WaveFlaeche* m_flaeche = nullptr;
    QScrollBar* m_rollX = nullptr;
    QLabel* m_hilfe = nullptr;
};

}  // namespace lumi::loop
