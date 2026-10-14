/**
 * @file KlangAuswahl.hpp
 * @brief Loop: die Auswahl „Sound" einer Drum-Spur oder einer Rolle im Kit —
 *        dieselbe im Drum-Editor und im Fenster Kits (Stufe 11c, Schritt 2;
 *        Stufe 21c, Anmerkung Patrik zu K8 a)
 *
 * Von oben: was die Spur spielt, wenn es kein Klang der Listen ist (ihr
 * Instrument, sonst „(none)"); die Gruppen **Kit**, **Samples**, **My samples**
 * und **File**; darunter die Wege **Instrument...**, **Sample file...** und —
 * wo es eine Aufnahme gibt — **Record sample...**.
 *
 * Ist das Feld schmaler als der Name, endet er mit „…" und steht ganz im
 * Tooltip; die aufgeklappte Liste ist so breit wie ihr laengster Eintrag.
 */
#pragma once

#include "loop/Loop.hpp"

#include <QComboBox>
#include <QString>
#include <QStringList>

namespace lumi::loop
{

// Die Eintraege, die kein Klang sind (ihr Wert in `itemData`):
inline const QString kKlangWahlDatei = QStringLiteral("?file");              ///< „Sample file...": fragt nach einer Tondatei
inline const QString kKlangWahlInstrument = QStringLiteral("?instrument");   ///< „Instrument...": das Fenster Instruments
inline const QString kKlangWahlAufnahme = QStringLiteral("?record");         ///< „Record sample...": das Fenster Record sample
inline const QString kKlangWahlJetzt = QStringLiteral("?current");           ///< das Instrument, das gerade spielt
inline const QString kKlangWahlKopf = QStringLiteral("?head");               ///< eine Ueberschrift, nicht zu waehlen

class KlangAuswahl : public QComboBox
{
    Q_OBJECT

public:
    explicit KlangAuswahl(QWidget* parent = nullptr);

    /// Die Auswahl fuer diese Spur fuellen; gewaehlt ist, was sie spielt.
    /// `kitKlaenge` als `kit:<Name>`, `eigene` und `bibliothek` wie
    /// `eigeneSamples` und `bibliotheksKlaenge` — einmal gelesen, fuer alle
    /// Auswahlen eines Fensters. `mitAufnahme` = „Record sample..." anbieten.
    void fuelle(const Spur& spur, const QStringList& kitKlaenge, const QStringList& eigene, const QStringList& bibliothek,
                bool mitAufnahme);
    /// Was die Auswahl anbietet, von oben nach unten: je Klang seine Nennung,
    /// die Ueberschriften als `[Kit]`, `[My samples]`, die Wege bei ihrem Text,
    /// ein Instrument als `instrument:<Nennung>`. Trennlinien fehlen.
    [[nodiscard]] QStringList zeilen() const;
    /// Der Text im geschlossenen Feld, wie er dasteht (gekuerzt mit „…").
    [[nodiscard]] QString sichtbarerText() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void showPopup() override;

private:
    QString m_instrument;  ///< das Instrument der gefuellten Spur; leer = keins
};

}  // namespace lumi::loop
