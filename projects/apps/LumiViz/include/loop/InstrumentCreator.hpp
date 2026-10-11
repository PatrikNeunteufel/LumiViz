/**
 * @file InstrumentCreator.hpp
 * @brief Loop: der Instrument-Creator — die Bausteine eines gerechneten
 *        Instruments als Regler, als eigenes Fenster
 *        (docs/audio/Loop_Konzept.md §4f, Stufe 10b; Skizze gebilligt,
 *        Patrik S110; Entscheide I2 b und I3 a)
 *
 * **Kopf:** Name, die Vorlage, Reset, Save, Save as..., Peak.
 * **Links:** Oscillator (Waveform, Voices, Detune — bei Sine die Obertoene,
 * bei Plucked string Damping), Envelope, Filter, Vibrato, Level.
 * **Rechts:** das Bild einer Schwingung, die Huellkurve einer Note und eine
 * kleine Klaviatur zum Anspielen.
 *
 * Der Creator arbeitet an einer Kopie. Erst **Save** schreibt die Datei
 * `.lvinst` — ein mitgeliefertes Instrument laesst sich nur unter neuem Namen
 * speichern (Save as...). Zum Anspielen schreibt er die Kopie in eine
 * Vorhoer-Datei und nennt sie dem, der spielt.
 */
#pragma once

#include "loop/Instrument.hpp"

#include <QPoint>
#include <QString>
#include <QWidget>

#include <functional>
#include <vector>

class QComboBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;

namespace lumi::loop
{

/// Eine Kurve als Bild: Werte von -1 bis 1 (oder 0 bis 1), ueber die Breite
/// verteilt; auf Wunsch ein senkrechter Strich an einer Stelle.
class KurvenBild : public QWidget
{
    Q_OBJECT

public:
    explicit KurvenBild(QWidget* parent = nullptr);
    /// `nurOben`: die Werte sind Betraege (0 bis 1), die Nulllinie liegt unten.
    /// `marke`: 0 bis 1 = ein Strich an dieser Stelle der Breite; unter null = keiner.
    void setWerte(std::vector<float> werte, bool nurOben, double marke = -1.0, const QString& markenText = {});
    [[nodiscard]] const std::vector<float>& werte() const { return m_werte; }
    [[nodiscard]] QSize sizeHint() const override { return {260, 96}; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<float> m_werte;
    bool m_nurOben = false;
    double m_marke = -1.0;
    QString m_markenText;
};

/// Zwei Oktaven Tasten, C3 bis C5: ein Klick verlangt den Ton.
class Tastenleiste : public QWidget
{
    Q_OBJECT

public:
    explicit Tastenleiste(QWidget* parent = nullptr);
    static constexpr int kTief = 48;  ///< C3
    static constexpr int kHoch = 72;  ///< C5
    /// Die Tonhoehe unter dem Punkt; -1 = keine.
    [[nodiscard]] int hoeheBei(QPoint ort) const;
    /// Ein Punkt auf der Taste dieser Tonhoehe (fuer Tests).
    [[nodiscard]] QPoint ortVon(int hoehe) const;
    [[nodiscard]] QSize sizeHint() const override { return {260, 48}; }

Q_SIGNALS:
    void taste(int hoehe);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    [[nodiscard]] QRect tastenRect(int hoehe) const;
    int m_gedrueckt = -1;
};

class InstrumentCreator : public QWidget
{
    Q_OBJECT

public:
    explicit InstrumentCreator(QWidget* parent = nullptr);

    /// Dieses Instrument zeigen — eine Kopie. Sein `name` sagt, woher es
    /// kommt: ein mitgeliefertes (Name) oder eine eigene Datei (Pfad).
    void oeffne(const Instrument& instrument);
    /// Die Kopie, wie die Regler sie gerade stellen.
    [[nodiscard]] const Instrument& instrument() const { return m_instrument; }
    /// Weicht die Kopie von dem ab, was geoeffnet oder zuletzt gespeichert wurde?
    [[nodiscard]] bool geaendert() const;
    /// Die eigene Datei, in die Save schreibt; leer = es gibt keine (ein
    /// mitgeliefertes Instrument: nur Save as...).
    [[nodiscard]] const QString& datei() const { return m_datei; }

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    /// Die Kopie aendern; Bilder, Regler und Titel ziehen nach.
    void aendere(const std::function<void(Instrument&)>& tu);
    /// Zurueck auf das, was geoeffnet oder zuletzt gespeichert wurde.
    void setzeZurueck();
    /// In die eigene Datei schreiben. false = es gibt keine, oder das
    /// Schreiben gelang nicht.
    bool speichere();
    /// Als neue Datei schreiben; sie ist danach die Datei des Creators.
    bool speichereUnter(const QString& pfad);
    /// Einen Ton mit den Einstellungen anspielen, wie sie gerade stehen: die
    /// Kopie kommt in die Vorhoer-Datei, `tonVerlangt` nennt sie. false = die
    /// Datei liess sich nicht schreiben.
    bool spieleAn(int hoehe);
    /// Die Vorhoer-Datei (im Ordner fuer Zwischendateien).
    [[nodiscard]] static QString vorhoerDatei();

Q_SIGNALS:
    /// Das Instrument steht jetzt in dieser Datei (Save, Save as...).
    void gespeichert(const QString& pfad);
    /// Einen Ton anspielen: das Instrument beim Pfad seiner Vorhoer-Datei.
    void tonVerlangt(const QString& nennung, int hoehe);

protected:
    /// Ungespeichertes: Save, Discard oder Cancel.
    void closeEvent(QCloseEvent* event) override;

private:
    /// Ein Regler mit seinem Wert daneben.
    struct Regler
    {
        QLabel* name = nullptr;
        QSlider* schieber = nullptr;
        QLabel* wert = nullptr;
        std::function<int(const Instrument&)> lese;
        std::function<QString(const Instrument&)> text;
    };
    void setupUI();
    Regler& regler(QGridLayout* gitter, const QString& name, const char* kennung, int von, int bis,
                   std::function<int(const Instrument&)> lese, std::function<void(Instrument&, int)> schreibe,
                   std::function<QString(const Instrument&)> text);
    void frageNachDatei();
    /// Regler, Sichtbarkeit, Bilder, Titel aus `m_instrument`.
    void zeigeAlles();
    void zeigeBilder();

    Instrument m_instrument;
    Instrument m_vorlage;   ///< wie geoeffnet oder zuletzt gespeichert
    QString m_datei;
    QString m_herkunft;     ///< „based on …"

    QLineEdit* m_titel = nullptr;
    QLabel* m_herkunftZeile = nullptr;
    QPushButton* m_zurueck = nullptr;
    QPushButton* m_speichern = nullptr;
    QPushButton* m_speichernUnter = nullptr;
    QLabel* m_spitze = nullptr;
    QComboBox* m_welle = nullptr;
    std::vector<Regler> m_regler;
    std::vector<std::size_t> m_nurSinus;     ///< Plaetze in `m_regler`: die Obertoene
    /// Was die gezupfte Saite nicht kennt: Voices, Detune, Decay, Sustain, Filter, Vibrato.
    std::vector<std::size_t> m_nichtSaite;
    std::size_t m_daempfung = 0;             ///< Platz des Reglers Damping (nur Saite)
    KurvenBild* m_wellenBild = nullptr;
    KurvenBild* m_huellBild = nullptr;
    Tastenleiste* m_tasten = nullptr;
};

}  // namespace lumi::loop
