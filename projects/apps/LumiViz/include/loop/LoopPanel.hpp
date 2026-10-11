/**
 * @file LoopPanel.hpp
 * @brief Loop: das Panel — das Arrangement eines Loops
 *        (docs/audio/Loop_Konzept.md §4c, Stufe 6a; Entscheide L4 a und L7 a)
 *
 * **Datei-Zeile:** Name mit Stern, New, Load…, Save, Save As…. **Abspiel-Zeile:**
 * Play / Stop, BPM, eine Zeile mit Kanaelen, Takten und Dauer, die Stelle als
 * Bar und Step, Fit. **Achse:** die `Taktachse` mit ihren Rollbalken — Kanaele
 * als Zeilen, Clips als Bloecke. **Clip-Zeile:** Basis, Takte und Kopien des
 * angeklickten Clips.
 *
 * Das Panel haelt den Loop; jede Aenderung laeuft ueber den CommandBus der App
 * (Undo / Redo) und gilt beim Spielen ab dem naechsten Block. Die Editoren der
 * Muster (Stufe 6b) gibt es noch nicht.
 *
 * Play spielt den Loop allein, von vorn, im Kreis; der Player schweigt so
 * lange, die Visuals reagieren auf den Loop. Fuehrt der Composer den Ton, ist
 * Play gesperrt und sagt, warum (L7 a); spielte der Loop gerade, haelt er an.
 */
#pragma once

#include "UI/panels/PanelBase.hpp"
#include "loop/Loop.hpp"
#include "loop/Spieler.hpp"

#include <QString>
#include <QStringList>

class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTimer;

namespace lumi::loop
{

class Taktachse;

class LoopPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit LoopPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~LoopPanel() override;

    /// Die Achse rollt selbst (viele Kanaele).
    [[nodiscard]] bool scrollsItself() const override { return true; }
    [[nodiscard]] int preferredArea() const override { return Qt::BottomDockWidgetArea; }
    /// Breit und flach — nicht das hohe Format der uebrigen Panels.
    [[nodiscard]] QSize sizeHint() const override { return {760, 260}; }

    /// Ein leerer Loop mit einem Kanal, noch ohne Datei.
    void neu();
    /// Eine `.lvloop` laden. false = die Datei ist kein Loop; es bleibt alles,
    /// wie es war. Spielt gerade ein Loop, spielt der neue weiter.
    bool lade(const QString& pfad);
    /// Den Loop in die Datei schreiben; sie ist danach die Datei des Panels.
    bool speichere(const QString& pfad);
    [[nodiscard]] const QString& datei() const { return m_datei; }
    [[nodiscard]] bool geladen() const { return m_geladen; }
    /// Gibt es Aenderungen, die nicht in der Datei stehen?
    [[nodiscard]] bool ungespeichert() const { return m_geladen && !(m_loop == m_gespeichert); }

    /// Der Loop, den das Panel zeigt und spielt.
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    /// Eine Aenderung ausfuehren — ueber den CommandBus der App (Undo / Redo),
    /// ohne ihn direkt.
    void bearbeite(const Loop& neu, const QString& was);
    /// Die Fassung setzen, ohne Undo (das tun die Befehle des CommandBus).
    void setzeLoop(const Loop& loop);
    /// Zaehlt jedes New und Load: ein Schritt fuer Undo gilt nur fuer die
    /// Ladung, in der er entstand.
    [[nodiscard]] int ladung() const { return m_ladung; }

    /// Den geladenen Loop von vorn spielen. false = nichts geladen, der
    /// Composer hat den Ton, oder es gibt keine Audio-Engine.
    bool spiele();
    void halte();
    [[nodiscard]] const Spieler& spieler() const { return m_spieler; }
    [[nodiscard]] Taktachse* taktachse() const { return m_achse; }

private:
    void setupUI();
    void frageNachDatei();
    void speichereMitDialog(bool unter);
    /// Vor New und Load: gibt es Ungespeichertes, fragt das Panel. false = bleiben.
    [[nodiscard]] bool darfVerwerfen();
    void zeigeDatei();
    void zeigeClip();
    void zeigeStand();
    void beimTakt();

    Spieler m_spieler;
    Loop m_loop;         ///< was das Panel zeigt und spielt
    Loop m_gespeichert;  ///< was in der Datei steht (oder beim New entstand)
    QString m_datei;     ///< leer = noch nie gespeichert
    bool m_geladen = false;
    int m_ladung = 0;
    QStringList m_ladeBericht;  ///< was beim Lesen der Datei auffiel

    QLabel* m_dateiZeile = nullptr;
    QPushButton* m_neu = nullptr;
    QPushButton* m_laden = nullptr;
    QPushButton* m_speichern = nullptr;
    QPushButton* m_speichernUnter = nullptr;
    QPushButton* m_play = nullptr;
    QDoubleSpinBox* m_bpm = nullptr;
    QLabel* m_info = nullptr;
    QLabel* m_stelle = nullptr;
    QPushButton* m_fit = nullptr;
    Taktachse* m_achse = nullptr;
    QLabel* m_clipZeile = nullptr;
    QLabel* m_hinweis = nullptr;
    QLabel* m_bericht = nullptr;
    QTimer* m_takt = nullptr;
};

}  // namespace lumi::loop
