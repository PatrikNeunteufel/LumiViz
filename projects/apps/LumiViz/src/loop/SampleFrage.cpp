/**
 * @file SampleFrage.cpp
 * @brief Loop: die Rueckfrage zu einer gewaehlten Tondatei (Entscheid S2 a)
 */

#include "loop/SampleFrage.hpp"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>

namespace lumi::loop
{

namespace
{

QString text(const char* quelle)
{
    return QCoreApplication::translate("lumi::loop::SampleFrage", quelle);
}

}  // namespace

SampleKopie waehleSampleKopie(QWidget* eltern, const QString& pfad, const QString& ohneKopie, const QString& objektName)
{
    // Liegt die Datei schon bei den eigenen Samples, ist nichts zu fragen.
    const bool schonEigen = QDir::cleanPath(QFileInfo(pfad).absolutePath())
                                .compare(QDir::cleanPath(eigenerSampleOrdner()), Qt::CaseInsensitive) == 0;
    if (schonEigen) return SampleKopie::Kopieren;
    const SampleKopie gemerkt = sampleKopieWahl();
    if (gemerkt != SampleKopie::Fragen) return gemerkt;

    QMessageBox frage(eltern);
    frage.setObjectName(objektName);
    frage.setWindowTitle(text("Sample"));
    frage.setIcon(QMessageBox::Question);
    frage.setText(text("Where should %1 be kept?").arg(QFileInfo(pfad).fileName()));
    frage.setInformativeText(text("A copy in My samples goes along with an export and stays when the original moves. "
                                  "%1\n%2")
                                 .arg(ohneKopie, QDir::toNativeSeparators(pfad)));
    QPushButton* kopieren = frage.addButton(text("Copy the file to My samples (recommended)"), QMessageBox::AcceptRole);
    QPushButton* amOrt = frage.addButton(text("Use it where it is"), QMessageBox::ActionRole);
    frage.addButton(QMessageBox::Cancel);
    frage.setDefaultButton(kopieren);
    auto* merken = new QCheckBox(text("Don't ask again (Settings → Panels changes it)"), &frage);
    frage.setCheckBox(merken);
    frage.exec();
    if (frage.clickedButton() != kopieren && frage.clickedButton() != amOrt) return SampleKopie::Fragen;
    const SampleKopie wahl = frage.clickedButton() == kopieren ? SampleKopie::Kopieren : SampleKopie::AmOrt;
    if (merken->isChecked()) setzeSampleKopieWahl(wahl);
    return wahl;
}

}  // namespace lumi::loop
