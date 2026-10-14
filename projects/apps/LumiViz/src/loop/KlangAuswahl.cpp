/**
 * @file KlangAuswahl.cpp
 * @brief Loop: die Auswahl „Sound" — im Drum-Editor und im Fenster Kits
 */

#include "loop/KlangAuswahl.hpp"

#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"  // instrumentAnzeige
#include "loop/Sample.hpp"

#include <QAbstractItemView>
#include <QDir>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QStyleOptionComboBox>
#include <QStylePainter>

#include <algorithm>

namespace lumi::loop
{

KlangAuswahl::KlangAuswahl(QWidget* parent) : QComboBox(parent) {}

void KlangAuswahl::fuelle(const Spur& spur, const QStringList& kitKlaenge, const QStringList& eigene,
                          const QStringList& bibliothek, bool mitAufnahme)
{
    const QSignalBlocker ruhe(this);
    clear();
    m_instrument = spur.instrument;
    const auto kopf = [this](const QString& text) {
        addItem(text, kKlangWahlKopf);
        // eine Ueberschrift: grau, nicht zu waehlen
        if (auto* modell = qobject_cast<QStandardItemModel*>(model()))
            if (QStandardItem* eintrag = modell->item(count() - 1)) eintrag->setFlags(Qt::NoItemFlags);
    };
    const QString& klang = spur.klang;
    const bool instrument = !spur.instrument.isEmpty();
    // ganz oben, was die Spur spielt, wenn es kein Klang der Listen darunter ist
    if (instrument)
        addItem(InstrumentEditor::instrumentAnzeige(spur.instrument), kKlangWahlJetzt);
    else if (klang.isEmpty())
        addItem(tr("(none)"), QString());
    kopf(tr("Kit"));
    for (const QString& kit : kitKlaenge) addItem(kit.mid(4), kit);  // ohne den Vorsatz „kit:"
    if (istKitKlang(klang) && !kitKlaenge.contains(klang)) addItem(klang.mid(4), klang);
    // die mitgelieferten Samples (`lib:`, Entscheid 1 a, S123)
    const bool mitgeliefertFehlt = istBibliotheksKlang(klang) && !bibliothek.contains(klang);
    if (!bibliothek.isEmpty() || mitgeliefertFehlt)
    {
        kopf(tr("Samples"));
        for (const QString& nennung : bibliothek) addItem(sampleTitel(nennung), nennung);
        if (mitgeliefertFehlt) addItem(tr("%1 (not found)").arg(sampleTitel(klang)), klang);
    }
    const bool eigenerFehlt = istEigenerKlang(klang) && !eigene.contains(klang);
    if (!eigene.isEmpty() || eigenerFehlt)
    {
        kopf(tr("My samples"));
        for (const QString& nennung : eigene) addItem(sampleTitel(nennung), nennung);
        if (eigenerFehlt) addItem(tr("%1 (not found)").arg(sampleTitel(klang)), klang);
    }
    if (!klang.isEmpty() && !istMitgelieferterKlang(klang) && !istEigenerKlang(klang))
    {
        // eine Tondatei an freiem Ort
        kopf(tr("File"));
        addItem(QFileInfo(klang).fileName(), klang);
    }
    insertSeparator(count());
    addItem(tr("Instrument..."), kKlangWahlInstrument);
    addItem(tr("Sample file..."), kKlangWahlDatei);
    if (mitAufnahme) addItem(tr("Record sample..."), kKlangWahlAufnahme);
    setCurrentIndex(instrument ? 0 : findData(klang));
    // der ganze Name steht im Tooltip — im Feld kann er gekuerzt sein
    if (instrument)
        setToolTip(tr("Plays the instrument %1 — note %2, held %3 ms")
                       .arg(InstrumentEditor::instrumentAnzeige(spur.instrument), tonName(spur.hoehe))
                       .arg(spur.dauerMs));
    else if (klang.isEmpty())
        setToolTip(QString());
    else
        setToolTip(QStringLiteral("%1\n%2").arg(currentText(), istMitgelieferterKlang(klang) || istEigenerKlang(klang)
                                                                    ? klang
                                                                    : QDir::toNativeSeparators(klang)));
}

QStringList KlangAuswahl::zeilen() const
{
    QStringList liste;
    for (int i = 0; i < count(); ++i)
    {
        if (itemData(i, Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator")) continue;
        const QString wert = itemData(i).toString();
        if (wert == kKlangWahlKopf)
            liste << QStringLiteral("[%1]").arg(itemText(i));
        else if (wert == kKlangWahlJetzt)
            liste << QStringLiteral("instrument:") + m_instrument;
        else if (wert == kKlangWahlInstrument || wert == kKlangWahlDatei || wert == kKlangWahlAufnahme || wert.isEmpty())
            liste << itemText(i);
        else
            liste << wert;
    }
    return liste;
}

QString KlangAuswahl::sichtbarerText() const
{
    QStyleOptionComboBox option;
    initStyleOption(&option);
    const QRect feld = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this);
    return fontMetrics().elidedText(currentText(), Qt::ElideRight, std::max(0, feld.width() - 4));
}

void KlangAuswahl::paintEvent(QPaintEvent*)
{
    // wie QComboBox, nur dass ein zu langer Name mit „…" endet, statt abgeschnitten zu werden
    QStylePainter maler(this);
    maler.setPen(palette().color(QPalette::Text));
    QStyleOptionComboBox option;
    initStyleOption(&option);
    maler.drawComplexControl(QStyle::CC_ComboBox, option);
    option.currentText = sichtbarerText();
    maler.drawControl(QStyle::CE_ComboBoxLabel, option);
}

void KlangAuswahl::showPopup()
{
    // die Liste so breit wie ihr laengster Eintrag, mindestens so breit wie das Feld
    int breite = width();
    for (int i = 0; i < count(); ++i) breite = std::max(breite, fontMetrics().horizontalAdvance(itemText(i)) + 48);
    view()->setMinimumWidth(breite);
    QComboBox::showPopup();
}

}  // namespace lumi::loop
