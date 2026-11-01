/**
 * @file LatenzFenster.hpp
 * @brief Loop: das kleine Fenster **Measure latency** (docs/audio/Loop_Konzept.md §4p,
 *        Stufe 15k-1; Entscheide Patrik WV10-1 a, Skizze SK15k-1 a B)
 *
 * Spielt fuenf Klicks ueber einen eigenen Mixer der Engine (`KlickGeber`) und nimmt
 * zugleich vom gewaehlten Eingang auf, mit der Stelle des Klickgebers als Uhr. Danach steht
 * da, wie spaet die Klicks ankamen (Median, Streuung, wie viele gefunden). **Use** merkt
 * den Wert fuer das Paar aus Eingang und Ausgang und meldet ihn.
 */
#pragma once

#include "audio/IAudioEngine.hpp"
#include "loop/AufnahmeUhr.hpp"

#include <QString>
#include <QWidget>

#include <memory>

class QLabel;
class QPushButton;
class QTimer;

namespace lumi::loop
{

class LatenzFenster : public QWidget
{
    Q_OBJECT

public:
    explicit LatenzFenster(QWidget* parent = nullptr);
    ~LatenzFenster() override;

    /// Die Engine (nullptr = keine: Measure ist aus) und der Eingang, von dem gemessen
    /// wird; `ausgang` ist der Name des Ausgabegeraets — unter beiden wird der Wert gemerkt.
    void setze(IAudioEngine* engine, int eingang, const QString& eingangName, const QString& ausgang);

    // --- was die Knoepfe ausloesen (auch fuer Tests) -------------------------------
    /// **Measure.** false = keine Engine, oder Mixer oder Eingang lassen sich nicht oeffnen.
    bool starteMessung();
    [[nodiscard]] bool misst() const { return m_geber != nullptr; }
    /// Ein Takt: ist die Messung durch, wird sie ausgewertet.
    void takt();
    [[nodiscard]] const Latenzmessung& ergebnis() const { return m_ergebnis; }
    [[nodiscard]] QString ergebnisText() const;
    /// **Use:** den Wert merken und melden, das Fenster schliesst. false = kein Ergebnis.
    bool nimm();
    /// Der Klickgeber der laufenden Messung (fuer Tests); nullptr = keine.
    [[nodiscard]] KlickGeber* geber() const { return m_geber.get(); }

Q_SIGNALS:
    void latenzGewaehlt(double ms);

private:
    void beende();
    void zeige();

    IAudioEngine* m_engine = nullptr;
    int m_eingang = -1;
    QString m_eingangName;
    QString m_ausgang;
    std::unique_ptr<KlickGeber> m_geber;
    AudioStreamHandle m_mixer = INVALID_STREAM;
    AudioStreamHandle m_quelle = INVALID_STREAM;
    Latenzmessung m_ergebnis;
    QString m_meldung;

    QLabel* m_text = nullptr;
    QPushButton* m_messen = nullptr;
    QPushButton* m_nehmen = nullptr;
    QTimer* m_takt = nullptr;
};

}  // namespace lumi::loop
