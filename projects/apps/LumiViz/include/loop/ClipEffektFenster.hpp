/**
 * @file ClipEffektFenster.hpp
 * @brief Loop: das Fenster „Clip effects" — der Weg des Tons der markierten
 *        Clips als Reihe von Bausteinen: Filter, Distortion, Echo, Reverb
 *        (docs/audio/Loop_Konzept.md §4g, Stufe 12d; §4j, Stufe 16d —
 *        Entscheide Patrik S117: K3 a, K4 a; S118: K5 a)
 *
 * Ein Werkzeugfenster ueber dem Loop-Panel, geoeffnet ueber **Effects...** im
 * Menue eines Clips. Es folgt den Marken im Loop-Panel, solange es offen ist.
 *
 * **Die Zeile Signal path** (`SignalWeg`): fest **Clip**, dann die Bausteine
 * der Reihe, der Knopf **+ Add**, fest **Out**. Ein Klick auf einen Baustein
 * zeigt darunter seinen Haken (an oder uebersprungen) und seine Regler;
 * **← Move**, **Move →** und Ziehen ordnen um, **Remove** entfernt.
 *
 * **Mehrere Clips (K5 a):** haben alle dieselbe Folge von Bausteinen, zeigt
 * das Fenster sie gemeinsam — wo Werte abweichen, steht „mixed", und wer
 * einen Regler bewegt, setzt den Wert fuer alle. Sonst zeigt die Zeile die
 * Reihe EINES Clips, waehlbar unter **Showing**; Regler, Add, Move und Remove
 * gelten fuer alle markierten Clips mit derselben Folge. Ein Hinweis nennt,
 * wie viele das sind und welche bleiben, wie sie sind. **Use this path for
 * all** setzt die gezeigte Reihe mit ihren Werten auf alle markierten Clips.
 * Copy, Paste und Reset gelten immer fuer alle.
 *
 * Das Fenster aendert den Loop nicht selbst: es meldet die neue Fassung mit
 * `bearbeitet`, das Panel fuehrt sie ueber Undo aus und zeigt sie zurueck.
 * Ein Schieber meldet beim Loslassen.
 */
#pragma once

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"

#include <QPoint>
#include <QString>
#include <QWidget>

#include <functional>
#include <optional>
#include <variant>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QStackedWidget;
class QVBoxLayout;

namespace lumi::loop
{

class SignalWeg;

class ClipEffektFenster : public QWidget
{
    Q_OBJECT

public:
    explicit ClipEffektFenster(QWidget* parent = nullptr);

    /// Diese Clips dieses Loops zeigen. Ohne Clip ist das Fenster gesperrt
    /// und sagt, was zu tun ist. Andere Clips als bisher: die Zeile zeigt den
    /// ersten, gewaehlt ist sein erster Baustein.
    void zeige(const Loop& loop, const std::vector<ClipOrt>& orte);
    [[nodiscard]] const std::vector<ClipOrt>& orte() const { return m_orte; }
    /// Die Spitze der Summe, solange der Loop spielt; kleiner als 0 = kein Wert.
    void setSpitze(double spitze);

    // --- die Zeile Signal path (Stufe 16d) -----------------------------------
    /// Was in der Zeile gewaehlt ist: der feste Kasten Clip, der feste Kasten
    /// Out oder der Platz eines Bausteins in der gezeigten Reihe (ab 0).
    static constexpr int kStelleClip = -1;
    static constexpr int kStelleOut = -2;
    [[nodiscard]] int stelle() const { return m_stelle; }
    void waehleStelle(int stelle);
    /// **Showing:** der Clip, dessen Reihe die Zeile zeigt — sein Platz unter
    /// den markierten (`orte()`).
    [[nodiscard]] int zeigt() const { return m_zeigt; }
    void zeigeClip(int platz);
    /// Die gezeigte Reihe; leer ohne Clip.
    [[nodiscard]] ClipEffekte gezeigteReihe() const;
    /// Die Plaetze (in `orte()`) der markierten Clips mit derselben Folge von
    /// Bausteinen wie der gezeigte — fuer sie gelten Regler, Add, Move, Remove.
    [[nodiscard]] std::vector<int> gleiche() const;
    /// **+ Add:** ein neuer Baustein hinter dem gewaehlten (bei Clip vorn, bei
    /// Out hinten), eingeschaltet; er ist danach gewaehlt. false = die Reihe
    /// traegt schon `kBausteineMax`, oder kein Clip ist gezeigt.
    bool neuerBaustein(const ClipBaustein& baustein);
    /// **← Move / Move →:** den gewaehlten Baustein einen Platz nach vorn (-1)
    /// oder hinten (+1).
    bool schiebeBaustein(int richtung);
    /// Ziehen: den Baustein vom Platz `von` an den Platz `nach` der Reihe.
    bool verschiebeBaustein(int von, int nach);
    /// **Remove:** den gewaehlten Baustein entfernen.
    bool entferneBaustein();
    /// **Use this path for all:** die gezeigte Reihe mit ihren Werten auf alle
    /// markierten Clips setzen. false = sie haben sie schon.
    bool gleicheAn();
    /// Die Zeile selbst (fuer Tests).
    [[nodiscard]] SignalWeg* signalWeg() const { return m_pfad; }

    // --- was die Knoepfe ausloesen (auch fuer Tests) -------------------------
    /// Die Effekte des gezeigten Clips merken.
    bool kopiere();
    /// Die gemerkten Effekte auf alle markierten Clips setzen.
    bool fuegeEin();
    /// Alle Effekte der markierten Clips entfernen.
    bool setzeZurueck();

Q_SIGNALS:
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);

private:
    /// Ein Schieber mit seiner Anzeige: was er liest und was er setzt.
    struct Regler
    {
        QSlider* schieber = nullptr;
        QLabel* anzeige = nullptr;
        double faktor = 100.0;  ///< Schieberwert = Wert mal Faktor
        std::function<double(const ClipEffekte&)> lies;
        std::function<void(ClipEffekte&, double)> setze;
        std::function<QString(double)> text;
    };

    void setupUI();
    void neuerRegler(QVBoxLayout* gruppe, const QString& name, const QString& kennung, int von, int bis, double faktor,
                     std::function<double(const ClipEffekte&)> lies, std::function<void(ClipEffekte&, double)> setze,
                     std::function<QString(double)> text);
    /// Ein Regler fuer ein Feld des gewaehlten Bausteins dieser Art.
    template <class Art>
    void reglerFuer(QVBoxLayout* gruppe, const QString& name, const QString& kennung, int von, int bis, double faktor,
                    double Art::* feld, std::function<QString(double)> text);
    /// Der gewaehlte Baustein dieser Reihe, wenn er von dieser Art ist; sonst nullptr.
    template <class Art>
    [[nodiscard]] const Art* baustein(const ClipEffekte& e) const
    {
        if (m_stelle < 0 || m_stelle >= static_cast<int>(e.reihe.size())) return nullptr;
        return std::get_if<Art>(&e.reihe[static_cast<std::size_t>(m_stelle)]);
    }
    template <class Art>
    [[nodiscard]] Art* baustein(ClipEffekte& e) const
    {
        if (m_stelle < 0 || m_stelle >= static_cast<int>(e.reihe.size())) return nullptr;
        return std::get_if<Art>(&e.reihe[static_cast<std::size_t>(m_stelle)]);
    }
    /// Die Effekte eines markierten Clips; nullptr = den Clip gibt es nicht.
    [[nodiscard]] const ClipEffekte* effekteVon(const Loop& loop, int platz) const;
    /// Die Effekte der Clips mit derselben Folge wie der gezeigte.
    [[nodiscard]] std::vector<ClipEffekte> gleicheEffekte() const;
    /// Dieselbe Aenderung an allen Clips mit derselben Folge (`gleiche`);
    /// gemeldet wird nur, wenn sich etwas aendert.
    bool aendere(const std::function<void(ClipEffekte&)>& was);
    /// Dieselbe Aenderung an allen markierten Clips.
    bool aendereAlle(const std::function<void(ClipEffekte&)>& was);
    bool melde(const Loop& neu);
    void fuelle();
    void zeigeAddMenue(QPoint unten);
    [[nodiscard]] QString clipName(int platz) const;

    Loop m_loop;
    std::vector<ClipOrt> m_orte;
    std::optional<ClipEffekte> m_gemerkt;
    bool m_fuellt = false;  ///< die Regler werden gerade gesetzt: nichts melden
    int m_zeigt = 0;                 ///< Showing: der Platz in `m_orte`
    int m_stelle = kStelleClip;

    QLabel* m_kopf = nullptr;
    QLabel* m_clips = nullptr;
    QLabel* m_spitze = nullptr;
    QLabel* m_zeigtName = nullptr;   ///< „Showing"
    QComboBox* m_zeigtWahl = nullptr;
    QWidget* m_hinweisZeile = nullptr;
    QLabel* m_hinweis = nullptr;     ///< „2 of 3 clips have this path. …"
    QPushButton* m_angleichen = nullptr;
    SignalWeg* m_pfad = nullptr;
    QCheckBox* m_an = nullptr;       ///< der Haken des gewaehlten Bausteins, mit seinem Namen
    QPushButton* m_bausteinVor = nullptr;
    QPushButton* m_bausteinZurueck = nullptr;
    QPushButton* m_bausteinWeg = nullptr;
    QStackedWidget* m_seiten = nullptr;  ///< je Art eine Seite Regler; die erste sagt, was zu tun ist
    QLabel* m_leerText = nullptr;
    QComboBox* m_filterArt = nullptr;
    QComboBox* m_zeit = nullptr;
    QLabel* m_zeitMs = nullptr;
    QPushButton* m_kopieren = nullptr;
    QPushButton* m_einfuegen = nullptr;
    QPushButton* m_zurueck = nullptr;
    double m_spitzenWert = -1.0;
    std::vector<Regler> m_regler;
};

}  // namespace lumi::loop
