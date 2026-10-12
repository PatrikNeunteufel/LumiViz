/**
 * @file InstrumentCreator.hpp
 * @brief Loop: der Instrument-Creator — die Bausteine eines gerechneten
 *        Instruments als Regler, als eigenes Fenster
 *        (docs/audio/Loop_Konzept.md §4f, Stufe 10b; Skizze gebilligt,
 *        Patrik S110; Entscheide I2 b und I3 a)
 *
 * **Kopf:** Name, die Vorlage, Reset, Save, Save as..., Level des ganzen
 * Instruments, Peak.
 * **Links (Stufe 10c; Skizze gebilligt, Patrik S111):** die Liste der
 * Schichten — Haken (an oder stumm), Name, Schwingung, Lautstaerke und
 * Versatz; darunter Add layer, Add layer from instrument..., Duplicate,
 * Remove, hinauf und hinunter.
 * **Mitte:** die gewaehlte Schicht — Name, Level, Transpose, Start offset;
 * Oscillator (Waveform, Voices, Detune — bei Sine die Obertoene, bei Plucked
 * string Damping), Envelope, Filter, Drive, Vibrato.
 * **Rechts:** das Bild einer Schwingung und die Huellkurve einer Note — die
 * Summe aller Schichten als Linie, die gewaehlte gestrichelt — und eine
 * kleine Klaviatur zum Anspielen; „Selected layer only" laesst die gewaehlte
 * allein hoeren.
 *
 * Der Creator arbeitet an einer Kopie. Erst **Save** schreibt die Datei
 * `.lvinst` — ein mitgeliefertes Instrument laesst sich nur unter neuem Namen
 * speichern (Save as...). Zum Anspielen schreibt er die Kopie in eine
 * Vorhoer-Datei und nennt sie dem, der spielt.
 */
#pragma once

#include "loop/Instrument.hpp"

#include <QPoint>
#include <QPolygonF>
#include <QString>
#include <QWidget>

#include <functional>
#include <vector>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QListWidget;
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
    /// Eine zweite Kurve, gestrichelt unter der ersten; leer = keine.
    void setZweite(std::vector<float> werte);
    [[nodiscard]] const std::vector<float>& zweite() const { return m_zweite; }
    [[nodiscard]] QSize sizeHint() const override { return {260, 96}; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    [[nodiscard]] QPolygonF linie(const std::vector<float>& werte) const;

    std::vector<float> m_werte;
    std::vector<float> m_zweite;
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

    // --- die Schichten (Stufe 10c) --------------------------------------------
    /// Der Platz der gewaehlten Schicht; ihre Regler stehen in der Mitte.
    [[nodiscard]] int gewaehlt() const { return m_gewaehlt; }
    [[nodiscard]] const Schicht& schicht() const { return m_instrument.schichten[static_cast<std::size_t>(m_gewaehlt)]; }
    void waehle(int platz);
    /// Die gewaehlte Schicht aendern.
    void aendereSchicht(const std::function<void(Schicht&)>& tu);
    /// Eine neue Schicht mit Grundeinstellung, sie ist danach gewaehlt.
    /// false = das Instrument traegt schon `kSchichtenMax`.
    bool neueSchicht();
    /// Die Schichten eines anderen Instruments dazunehmen — kopiert, nicht
    /// verbunden; eine ohne Namen heisst wie das Instrument. So viele kamen
    /// dazu (was ueber `kSchichtenMax` geht, bleibt weg); die erste davon ist
    /// danach gewaehlt.
    int schichtenAus(const Instrument& quelle);
    /// Die gewaehlte Schicht noch einmal, gleich dahinter. false = kein Platz.
    bool verdopple();
    /// Die gewaehlte Schicht entfernen. false = es ist die einzige.
    bool entferne();
    /// Die gewaehlte Schicht einen Platz hinauf (-1) oder hinunter (+1).
    bool schiebe(int richtung);
    /// Anspielen nur mit der gewaehlten Schicht — auch wenn sie stumm ist.
    void setNurGewaehlte(bool an);
    [[nodiscard]] bool nurGewaehlte() const { return m_nurGewaehlte; }
    /// Wie eine Schicht in der Liste heisst: „2 Shimmer" oder „Layer 2".
    [[nodiscard]] static QString schichtTitel(const Schicht& schicht, int platz);
    /// Zurueck auf das, was geoeffnet oder zuletzt gespeichert wurde.
    void setzeZurueck();
    /// In die eigene Datei schreiben — ein eigenes Instrument an seinen Platz
    /// im Benutzerordner, eine Datei an freiem Ort dorthin. false = es gibt
    /// keine (ein mitgeliefertes: `speichereAlsEigenes`), oder das Schreiben
    /// gelang nicht.
    bool speichere();
    /// Als neue Datei an freiem Ort schreiben; sie ist danach die Datei des Creators.
    bool speichereUnter(const QString& pfad);
    /// Unter diesem Titel als eigenes Instrument in den Benutzerordner
    /// schreiben (Stufe 14b, Entscheid P2 a): die Datei heisst
    /// `eigenerName(titel)`, das Instrument danach `user:<name>`; ein
    /// vorhandenes mit dem Namen wird ersetzt. false = aus dem Titel wird
    /// kein Name, oder das Schreiben gelang nicht.
    bool speichereAlsEigenes(const QString& titel);
    /// Der Titel, den Save fuer eine eigene Kopie vorschlaegt: der eigene —
    /// oder mit einer Zahl dahinter, wenn er noch der der Vorlage ist
    /// („House Piano 2", die erste, die es unter den eigenen nicht gibt).
    [[nodiscard]] QString vorschlagTitel() const;
    /// Wie das Instrument nach dem Speichern heisst: `user:<name>` oder der
    /// Pfad seiner Datei; leer = noch nie gespeichert.
    [[nodiscard]] QString nennung() const;
    /// Die Genres aus einem Text mit Kommas — ohne Leeres, ohne Doppelte.
    [[nodiscard]] static QStringList genresAus(const QString& text);
    /// Einen Ton mit den Einstellungen anspielen, wie sie gerade stehen: die
    /// Kopie kommt in die Vorhoer-Datei, `tonVerlangt` nennt sie. false = die
    /// Datei liess sich nicht schreiben.
    bool spieleAn(int hoehe);
    /// Die Vorhoer-Datei (im Ordner fuer Zwischendateien).
    [[nodiscard]] static QString vorhoerDatei();

Q_SIGNALS:
    /// Das Instrument ist gespeichert und heisst jetzt so: `user:<name>` oder
    /// der Pfad seiner Datei (Save, Save as...).
    void gespeichert(const QString& nennung);
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
        std::function<int(const Schicht&)> lese;
        std::function<QString(const Schicht&)> text;
    };
    void setupUI();
    /// Ein Regler der gewaehlten Schicht.
    Regler& regler(QGridLayout* gitter, const QString& name, const char* kennung, int von, int bis,
                   std::function<int(const Schicht&)> lese, std::function<void(Schicht&, int)> schreibe,
                   std::function<QString(const Schicht&)> text);
    void frageNachDatei();
    /// Save bei einem mitgelieferten oder neuen Instrument: nach dem Namen
    /// der eigenen Kopie fragen und sie schreiben. false = abgebrochen.
    bool frageNachNamen();
    /// Das Menue hinter „Add layer from instrument...".
    void zeigeQuellen();
    void zeigeListe();
    /// Die Kopie fuer die Bilder: je Schicht eine Stimme ohne Vibrato;
    /// `nurGewaehlte` = nur die gewaehlte Schicht, eingeschaltet.
    [[nodiscard]] Instrument fuerBild(bool nurGewaehlte) const;
    /// Regler, Sichtbarkeit, Bilder, Titel aus `m_instrument`.
    void zeigeAlles();
    void zeigeBilder();

    Instrument m_instrument;
    Instrument m_vorlage;   ///< wie geoeffnet oder zuletzt gespeichert
    QString m_datei;
    QString m_eigenerName;  ///< der Name im Benutzerordner; leer = kein eigenes (`user:<name>`)
    QString m_herkunft;     ///< „based on …"
    int m_gewaehlt = 0;
    bool m_nurGewaehlte = false;

    QListWidget* m_liste = nullptr;
    QPushButton* m_neu = nullptr;
    QPushButton* m_neuAus = nullptr;
    QPushButton* m_doppelt = nullptr;
    QPushButton* m_weg = nullptr;
    QPushButton* m_hinauf = nullptr;
    QPushButton* m_hinunter = nullptr;
    QLabel* m_schichtKopf = nullptr;
    QLineEdit* m_schichtName = nullptr;
    QSlider* m_gesamt = nullptr;      ///< Level des ganzen Instruments
    QLabel* m_gesamtWert = nullptr;
    QLabel* m_legende = nullptr;
    QCheckBox* m_nurWahl = nullptr;
    QLineEdit* m_titel = nullptr;
    QLabel* m_herkunftZeile = nullptr;
    QPushButton* m_zurueck = nullptr;
    QPushButton* m_speichern = nullptr;
    QPushButton* m_speichernUnter = nullptr;
    QLabel* m_spitze = nullptr;
    // die zweite Zeile im Kopf (Stufe 14b): fuer das Fenster „Instruments"
    QComboBox* m_art = nullptr;
    QLineEdit* m_genres = nullptr;
    QLineEdit* m_beschreibung = nullptr;
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
