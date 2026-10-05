/**
 * @file ComposerPanel.hpp
 * @brief Composer: das Panel zum Ansehen (docs/ui/Composer_Konzept.md §7, Stufe C2b)
 *
 * **Kopf:** Schalter Composer on, Play / Pause, Stopp, Loop, Resume after
 * restart, die Stelle als Zeit, der Name der Datei. **Zeitachse:** Lineal mit
 * Abspielmarke (Klick und Ziehen springen), die Kanaele als Zeilen, die Clips
 * als Bloecke mit ihrer Pegelkurve. **Clip-Zeile:** Datei, Start, Ende und
 * Mischart des angeklickten Clips.
 *
 * Bearbeitet wird hier noch nichts (C3). Das Panel bedient nur die
 * `composer/Sitzung` des Fensters — dieselbe, ueber die Menue und Start gehen.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`; die
 * Anmeldung in `PanelAutoReg.cpp` ist Naht N8.
 */
#pragma once

#include "UI/panels/PanelBase.hpp"
#include "composer/Komposition.hpp"

#include <QPointer>
#include <QString>
#include <QWidget>

#include <cstdint>

class QCheckBox;
class QLabel;
class QPushButton;
class VisualizerWidget;

namespace lumi::composer
{

class Sitzung;

/// Die Zeitachse: Lineal, Abspielmarke, Kanaele und Clips. Zeigt die ganze
/// Laenge der Komposition auf ihrer Breite (ein Zoom kommt mit C3).
class Zeitachse : public QWidget
{
    Q_OBJECT

public:
    explicit Zeitachse(QWidget* parent = nullptr);

    /// Was gezeigt wird; `geladen` false = der Hinweis „keine Komposition".
    void setKomposition(const Komposition& komposition, bool geladen);
    void setStelle(std::int64_t stelleMs);
    /// Der Composer ist aus: die Clips blasser, die Marke gestrichelt.
    void setAn(bool an);

    [[nodiscard]] int gewaehlterKanal() const { return m_kanal; }
    [[nodiscard]] int gewaehlterClip() const { return m_clip; }
    /// Der angeklickte Clip, nullptr = keiner.
    [[nodiscard]] const VisualClip* auswahl() const;

    // --- Geometrie (auch fuer Tests) ----------------------------------------
    [[nodiscard]] int xVon(std::int64_t stelleMs) const;
    [[nodiscard]] std::int64_t stelleVon(int x) const;
    /// Mitte der Zeile des Kanals (0 = oberste Zeile).
    [[nodiscard]] int yVonZeile(int zeile) const;
    /// Kanal einer Zeile: die Reihenfolge der Datei, der erste Kanal zuoberst
    /// (Entscheid Patrik S95) — im Bild liegt er zuunterst, spaetere darueber.
    [[nodiscard]] int kanalVonZeile(int zeile) const;

    static constexpr int kKopfBreite = 96;   ///< Spalte mit den Kanalnamen
    static constexpr int kRand = 10;         ///< frei rechts neben dem Ende
    static constexpr int kLinealHoehe = 24;
    static constexpr int kZeilenHoehe = 36;

Q_SIGNALS:
    /// Klick oder Ziehen im Lineal.
    void sprung(qint64 stelleMs);
    /// Ein Clip wurde angeklickt — oder die freie Flaeche (dann -1, -1).
    void auswahlGeaendert();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

private:
    [[nodiscard]] int zeitBreite() const;
    [[nodiscard]] QRect clipRechteck(int zeile, const VisualClip& clip) const;

    Komposition m_komposition;
    bool m_geladen = false;
    bool m_an = false;
    std::int64_t m_laengeMs = 0;
    std::int64_t m_stelleMs = 0;
    int m_kanal = -1;
    int m_clip = -1;
    bool m_zieht = false;  ///< Maus im Lineal gedrueckt
};

class ComposerPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ComposerPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~ComposerPanel() override;

    /// Die Zeitachse scrollt selbst (viele Kanaele).
    [[nodiscard]] bool scrollsItself() const override { return true; }
    [[nodiscard]] int preferredArea() const override { return Qt::BottomDockWidgetArea; }
    /// Breit und flach — nicht das hohe Format der uebrigen Panels.
    [[nodiscard]] QSize sizeHint() const override { return {900, 260}; }

    /// Das Fenster, dessen Komposition das Panel zeigt. Ohne Aufruf sucht das
    /// Panel es beim Zeigen selbst.
    void setFenster(VisualizerWidget* fenster);
    [[nodiscard]] Sitzung* sitzung() const;
    [[nodiscard]] Zeitachse* zeitachse() const { return m_zeitachse; }

    /// Eine Stelle als Text: Minuten, Sekunden, Zehntel („1:23.4").
    [[nodiscard]] static QString zeitText(std::int64_t ms);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void sucheFenster();
    /// Alles neu: eine andere Komposition (oder keine).
    void zeigeKomposition();
    /// Schalter, Knoepfe, Stelle — bei jeder Aenderung und im Takt.
    void zeigeStand();
    void zeigeClip();

    QPointer<VisualizerWidget> m_fenster;
    QPointer<Sitzung> m_sitzung;

    QCheckBox* m_an = nullptr;
    QPushButton* m_spielen = nullptr;
    QPushButton* m_stopp = nullptr;
    QPushButton* m_schleife = nullptr;
    QPushButton* m_resume = nullptr;
    QLabel* m_zeit = nullptr;
    QLabel* m_datei = nullptr;
    Zeitachse* m_zeitachse = nullptr;
    QLabel* m_clipZeile = nullptr;
    int m_gezeigt = -1;  ///< was Schalter und Knoepfe zuletzt zeigten
};

}  // namespace lumi::composer
