/**
 * @file StrumZeile.hpp
 * @brief Loop: die Zeile **Strum** — Spread, das Raster der Hand und ihr Muster
 *        je Stelle eines Takts (docs/audio/Saiten_Konzept.md §4b und §3 S-W5 a;
 *        Skizzen gebilligt, Patrik S134: N im Creator, H im Instrument-Editor)
 *
 * Von links: **Strum**, **Spread** (ms von Saite zu Saite, S-W12 a), **Hand**
 * (*Beat*, *1/2 beat*, *Step*), je Stelle eines Takts ein Feld ↓ oder ↑ — ein
 * Klick dreht es um, ein Feld, das vom Wechsel abweicht, ist hinterlegt —,
 * **Alternate** (alle zurueck auf ↓ ↑ ↓ ↑) und ein Hinweis. Dasselbe Element
 * steht im Creator (die Vorgabe des Instruments, `Saitensatz::strum`) und im
 * Instrument-Editor (Strum des Musters, `Muster::strum`).
 *
 * Die Zeile haelt eine Kopie; jede Aenderung meldet sie mit `geaendert`. Kein
 * Ton, keine Datei.
 */
#pragma once

#include "loop/Spielweise.hpp"

#include <QWidget>

#include <vector>

class QComboBox;
class QDoubleSpinBox;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QToolButton;

namespace lumi::loop
{

class StrumZeile : public QWidget
{
    Q_OBJECT

public:
    /// `kennung`: der Vorsatz der Objektnamen (`creatorStrum`, `editorStrum`) —
    /// die Teile heissen `<kennung>Spread`, `<kennung>Hand`, `<kennung>Cell1` …,
    /// `<kennung>Alternate`, `<kennung>Hint`.
    explicit StrumZeile(const QString& kennung, QWidget* parent = nullptr);

    /// Diesen Strum zeigen — ohne `geaendert`.
    void zeige(const Strum& strum);
    [[nodiscard]] const Strum& strum() const { return m_strum; }
    /// Das Raster des Loops: so viele Stellen hat ein Takt im Raster der Hand
    /// (`handStellen`). Ohne Loop (im Creator) gilt 4/4 mit 4 Schritten je Schlag.
    void setzeRaster(int schlaegeJeTakt, int schritteJeSchlag);
    /// Der Text hinter Alternate (im Creator: „default for new patterns").
    void setzeHinweis(const QString& text);

    /// So viele Felder stehen da.
    [[nodiscard]] int stellen() const;
    /// Das Feld einer Stelle (fuer Tests); nullptr = keine.
    [[nodiscard]] QToolButton* feld(int stelle) const;
    /// Weicht die Stelle vom Wechsel ab (das Feld ist hinterlegt)?
    [[nodiscard]] bool weichtAb(int stelle) const;

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    void setzeSpread(double ms);
    void setzeHandRaster(HandRaster raster);
    /// Ein Klick auf ein Feld: die Richtung an dieser Stelle umdrehen.
    void dreheStelle(int stelle);
    /// **Alternate:** alle Stellen zurueck auf den Wechsel.
    void abwechselnd();

Q_SIGNALS:
    void geaendert(const Strum& strum);

private:
    void zeigeFelder();
    void melde();

    QString m_kennung;
    Strum m_strum;
    int m_schlaege = 4;
    int m_schritte = 4;
    QDoubleSpinBox* m_spread = nullptr;
    QComboBox* m_raster = nullptr;
    QHBoxLayout* m_feldReihe = nullptr;
    std::vector<QToolButton*> m_felder;
    QPushButton* m_abwechselnd = nullptr;
    QLabel* m_hinweis = nullptr;
};

}  // namespace lumi::loop
