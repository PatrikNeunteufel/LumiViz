/**
 * @file LoopPanel.hpp
 * @brief Loop: das Panel — in Stufe 5b nur Laden und Abspielen
 *        (docs/audio/Loop_Konzept.md §5; Entscheide L4 a und L7 a)
 *
 * Eine Datei-Zeile mit **Load…** und ein **Play / Stop**. Das Raster (Spuren,
 * Schritte, Muster, Abfolge) folgt in Stufe 6.
 *
 * Play spielt den geladenen Loop allein, von vorn, im Kreis; der Player
 * schweigt so lange, die Visuals reagieren auf den Loop. Fuehrt der Composer
 * den Ton, ist Play gesperrt und sagt, warum (L7 a); spielte der Loop gerade,
 * haelt er an.
 */
#pragma once

#include "UI/panels/PanelBase.hpp"
#include "loop/Spieler.hpp"

#include <QString>
#include <QStringList>

class QLabel;
class QPushButton;
class QTimer;

namespace lumi::loop
{

class LoopPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit LoopPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~LoopPanel() override;

    [[nodiscard]] int preferredArea() const override { return Qt::BottomDockWidgetArea; }
    /// Flach — nicht das hohe Format der uebrigen Panels.
    [[nodiscard]] QSize sizeHint() const override { return {560, 180}; }

    /// Eine `.lvloop` laden. false = die Datei ist kein Loop; es bleibt alles,
    /// wie es war. Spielt gerade ein Loop, spielt der neue weiter.
    bool lade(const QString& pfad);
    [[nodiscard]] const QString& datei() const { return m_datei; }

    /// Den geladenen Loop von vorn spielen. false = nichts geladen, der
    /// Composer hat den Ton, oder es gibt keine Audio-Engine.
    bool spiele();
    void halte();
    [[nodiscard]] const Spieler& spieler() const { return m_spieler; }

private:
    void setupUI();
    void frageNachDatei();
    void zeigeDatei();
    void zeigeStand();
    void beimTakt();

    Spieler m_spieler;
    QString m_datei;
    bool m_geladen = false;
    QStringList m_ladeBericht;  ///< was beim Lesen der Datei auffiel

    QLabel* m_dateiZeile = nullptr;
    QPushButton* m_laden = nullptr;
    QPushButton* m_play = nullptr;
    QLabel* m_info = nullptr;
    QLabel* m_stelle = nullptr;
    QLabel* m_hinweis = nullptr;
    QLabel* m_bericht = nullptr;
    QTimer* m_takt = nullptr;
};

}  // namespace lumi::loop
