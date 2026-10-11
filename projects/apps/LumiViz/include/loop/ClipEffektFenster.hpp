/**
 * @file ClipEffektFenster.hpp
 * @brief Loop: das Fenster „Clip effects" — Distortion, Echo und Reverb der
 *        markierten Clips (docs/audio/Loop_Konzept.md §4g, Stufe 12d)
 *
 * Ein Werkzeugfenster ueber dem Loop-Panel, geoeffnet ueber **Effects...** im
 * Menue eines Clips. Es zeigt die Effekte aller Clips, die im Loop-Panel
 * markiert sind, und folgt den Marken, solange es offen ist. Haben die Clips
 * an einem Regler verschiedene Werte, steht dort „mixed"; wer ihn bewegt,
 * setzt den Wert fuer alle.
 *
 * Das Fenster aendert den Loop nicht selbst: es meldet die neue Fassung mit
 * `bearbeitet`, das Panel fuehrt sie ueber Undo aus und zeigt sie zurueck.
 * Ein Schieber meldet beim Loslassen.
 */
#pragma once

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"

#include <QString>
#include <QWidget>

#include <functional>
#include <optional>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QVBoxLayout;

namespace lumi::loop
{

class ClipEffektFenster : public QWidget
{
    Q_OBJECT

public:
    explicit ClipEffektFenster(QWidget* parent = nullptr);

    /// Diese Clips dieses Loops zeigen. Ohne Clip ist das Fenster gesperrt
    /// und sagt, was zu tun ist.
    void zeige(const Loop& loop, const std::vector<ClipOrt>& orte);
    [[nodiscard]] const std::vector<ClipOrt>& orte() const { return m_orte; }
    /// Die Spitze der Summe, solange der Loop spielt; kleiner als 0 = kein Wert.
    void setSpitze(double spitze);

    // --- was die Knoepfe ausloesen (auch fuer Tests) -------------------------
    /// Die Effekte des ersten gezeigten Clips merken.
    bool kopiere();
    /// Die gemerkten Effekte auf alle gezeigten Clips setzen.
    bool fuegeEin();
    /// Alle Effekte der gezeigten Clips zurueck auf die Vorgabe (aus).
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
    /// Ein Haken „an" einer Gruppe.
    struct Schalter
    {
        QCheckBox* haken = nullptr;
        std::function<bool(const ClipEffekte&)> lies;
        std::function<void(ClipEffekte&, bool)> setze;
    };

    void setupUI();
    QCheckBox* neuerSchalter(QVBoxLayout* gruppe, const QString& titel, const QString& kennung,
                             std::function<bool(const ClipEffekte&)> lies,
                             std::function<void(ClipEffekte&, bool)> setze);
    void neuerRegler(QVBoxLayout* gruppe, const QString& name, const QString& kennung, int von, int bis, double faktor,
                     std::function<double(const ClipEffekte&)> lies, std::function<void(ClipEffekte&, double)> setze,
                     std::function<QString(double)> text);
    /// Die Effekte der gezeigten Clips, in der Reihenfolge von `m_orte`.
    [[nodiscard]] std::vector<ClipEffekte> effekte() const;
    /// Dieselbe Aenderung an allen gezeigten Clips; gemeldet wird nur, wenn sich etwas aendert.
    bool aendere(const std::function<void(ClipEffekte&)>& was);
    void fuelle();

    Loop m_loop;
    std::vector<ClipOrt> m_orte;
    std::optional<ClipEffekte> m_gemerkt;
    bool m_fuellt = false;  ///< die Regler werden gerade gesetzt: nichts melden

    QLabel* m_kopf = nullptr;
    QLabel* m_clips = nullptr;
    QLabel* m_spitze = nullptr;
    QWidget* m_gruppen = nullptr;
    QComboBox* m_zeit = nullptr;
    QLabel* m_zeitMs = nullptr;
    QPushButton* m_kopieren = nullptr;
    QPushButton* m_einfuegen = nullptr;
    QPushButton* m_zurueck = nullptr;
    std::vector<Regler> m_regler;
    std::vector<Schalter> m_schalter;
};

}  // namespace lumi::loop
