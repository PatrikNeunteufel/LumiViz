/**
 * @file SaitenZeile.hpp
 * @brief Loop: die Zeile **Tuning** des Instrument-Creators — die Saiten eines
 *        Saiteninstruments (docs/audio/Saiten_Konzept.md §4, Stufe 23e;
 *        Skizze gebilligt, Patrik S132: S-U1 a, Nachtrag S-U3 a)
 *
 * Von links: **Tuning** (die Vorlage: None, die Stimmungen aus
 * `stimmungsvorlagen()`, Custom), **Strings** (1 … `kSaitenMax`), je Saite ein
 * Feld mit dem Ton der leeren Saite, von der tiefsten an, **Capo**, **Scale**
 * (die Mensur in mm) und **String choice** (die Vorgabe fuer neue Muster).
 * *None* = kein Saiteninstrument: die Zeile zeigt dann nur die Auswahl.
 * *Custom* waehlt man nicht — es steht da, wenn die Toene zu keiner Vorlage
 * passen (die Vorlage folgt aus den Toenen, `vorlageVon`). Passt die Zeile
 * nicht in die Breite, bricht sie um.
 *
 * Die Zeile haelt eine Kopie des Saitensatzes; jede Aenderung meldet sie mit
 * `geaendert`. Kein Ton, keine Datei.
 */
#pragma once

#include "loop/Saiten.hpp"

#include <QSpinBox>
#include <QWidget>

#include <vector>

class QComboBox;
class QDoubleSpinBox;
class QLabel;

namespace lumi::loop
{

/// Ein Feld fuer eine Tonhoehe (MIDI 0 … 127), das den Namen zeigt: „E2", „C#4".
/// Getippt wird ein Name („Eb3", „g4") oder eine Zahl.
class TonFeld : public QSpinBox
{
    Q_OBJECT

public:
    explicit TonFeld(QWidget* parent = nullptr);
    /// Die Tonhoehe zu einem Namen („E2", „C#4", „Bb3", auch klein); -1 = keiner.
    [[nodiscard]] static int hoeheAus(const QString& text);

protected:
    [[nodiscard]] QString textFromValue(int wert) const override;
    [[nodiscard]] int valueFromText(const QString& text) const override;
    [[nodiscard]] QValidator::State validate(QString& text, int& pos) const override;
};

class SaitenZeile : public QWidget
{
    Q_OBJECT

public:
    explicit SaitenZeile(QWidget* parent = nullptr);

    /// Diesen Saitensatz zeigen — ohne `geaendert`.
    void zeige(const Saitensatz& saiten);
    [[nodiscard]] const Saitensatz& saiten() const { return m_saiten; }

    /// Was die Auswahl Tuning zeigt: `kKeine` (None), `kEigene` (Custom) oder
    /// der Platz in `stimmungsvorlagen()`.
    static constexpr int kKeine = -1;
    static constexpr int kEigene = -2;
    [[nodiscard]] int vorlagePlatz() const;

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    /// Eine Vorlage waehlen: ihre Toene und ihre Mensur; Capo und String
    /// choice bleiben. `kKeine` = kein Saiteninstrument.
    void waehleVorlage(int platz);
    /// Strings: so viele Saiten. Mehr = neue oben, je eine Quarte ueber der
    /// hoechsten bisher; weniger = die obersten fallen weg.
    void setzeAnzahl(int anzahl);
    /// Den Ton der leeren Saite `saite` (ab 0) setzen.
    void setzeTon(int saite, int hoehe);
    void setzeCapo(int capo);
    void setzeMensur(double mm);
    void setzeWahl(Saitenwahl wahl);

Q_SIGNALS:
    void geaendert(const Saitensatz& saiten);

private:
    /// Die Felder nach `m_saiten` stellen, ohne Signale.
    void zeigeFelder();
    /// `m_saiten` in die Grenzen bringen, zeigen, melden.
    void melde();

    Saitensatz m_saiten;
    QComboBox* m_vorlage = nullptr;
    QLabel* m_anzahlName = nullptr;
    QSpinBox* m_anzahl = nullptr;
    std::vector<TonFeld*> m_toene;  ///< `kSaitenMax` Felder; gezeigt so viele, wie es Saiten gibt
    QLabel* m_capoName = nullptr;
    QSpinBox* m_capo = nullptr;
    QLabel* m_mensurName = nullptr;
    QDoubleSpinBox* m_mensur = nullptr;
    QLabel* m_wahlName = nullptr;
    QComboBox* m_wahl = nullptr;
};

}  // namespace lumi::loop
