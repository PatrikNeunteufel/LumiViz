/**
 * @file ExportFenster.cpp
 * @brief Loop: das Fenster *Export audio* (Stufe 30c)
 */

#include "loop/ExportFenster.hpp"

#include <QButtonGroup>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <utility>

namespace lumi::loop
{

namespace
{

constexpr char kFormat[] = "loop/exportFormat";
constexpr char kOrdner[] = "loop/exportFolder";
constexpr char kEnde[] = "loop/exportEnding";
constexpr char kDurchlaeufe[] = "loop/exportPasses";
/// Die Warnfarbe wie im Dialog Match loudness (LautheitsDialog) und im Feld Zeit des Composers.
constexpr char kWarnfarbe[] = "color: #d8b400;";

std::function<void(const QString&)>& ordnerZeiger()
{
    static std::function<void(const QString&)> zeiger;
    return zeiger;
}

QString zeitText(double ms)
{
    const auto sekunden = static_cast<qint64>(std::llround(ms / 1000.0));
    return QStringLiteral("%1:%2").arg(sekunden / 60).arg(sekunden % 60, 2, 10, QLatin1Char('0'));
}

QString sekundenText(std::int64_t frames, int rate)
{
    return QString::number(rate > 0 ? static_cast<double>(frames) / rate : 0.0, 'f', 1);
}

QString groesseText(qint64 bytes)
{
    if (bytes < 1024 * 1024) return QStringLiteral("%1 KB").arg(std::max<qint64>(1, bytes / 1024));
    return QStringLiteral("%1 MB").arg(static_cast<double>(bytes) / (1024.0 * 1024.0), 0, 'f', 1);
}

QString formatSchluessel(audio::WavTiefe tiefe)
{
    switch (tiefe)
    {
    case audio::WavTiefe::Pcm24: return QStringLiteral("pcm24");
    case audio::WavTiefe::Pcm16: return QStringLiteral("pcm16");
    case audio::WavTiefe::Gleitkomma32: break;
    }
    return QStringLiteral("float32");
}

}  // namespace

ExportFenster::ExportFenster(ExportWunsch vorlage, QString loopName, QString clipText, Dekodierer dekodierer,
                             QWidget* parent)
    : QDialog(parent), m_vorlage(std::move(vorlage)), m_loopName(std::move(loopName)), m_clipText(std::move(clipText)),
      m_dekodierer(std::move(dekodierer))
{
    setObjectName(QStringLiteral("exportAudio"));
    setWindowTitle(tr("Export audio"));
    baue();
    zeigeWahl();
}

ExportFenster::~ExportFenster()
{
    m_auftrag.reset();  // bricht ab und wartet, bevor die Felder gehen
}

void ExportFenster::setzeOrdnerZeigerFuerTests(std::function<void(const QString&)> zeiger)
{
    ordnerZeiger() = std::move(zeiger);
}

QString ExportFenster::enthaltenText(const ExportWunsch& wunsch)
{
    switch (wunsch.art)
    {
    case ExportArt::Clip:
        return tr("the clip with its effects, expression and tail — without channel fader and loop volume");
    case ExportArt::Loop:
        return wunsch.loop.limiter ? tr("the whole loop as you hear it here — with faders, loop volume and limiter")
                                   : tr("the whole loop as you hear it here — with faders and loop volume (limiter off)");
    case ExportArt::Ausschnitt:
        return tr("the loop from %1 for %2, with its faders — without the composition's gain, fades and limiter")
            .arg(zeitText(wunsch.versatzMs), zeitText(wunsch.laengeMs));
    }
    return {};
}

void ExportFenster::baue()
{
    auto* aussen = new QVBoxLayout(this);
    m_wahl = new QWidget(this);
    auto* form = new QFormLayout(m_wahl);
    form->setContentsMargins(0, 0, 0, 0);

    auto* clip = new QLabel(m_clipText, m_wahl);
    clip->setObjectName(QStringLiteral("exportClip"));
    form->addRow(tr("Clip:"), clip);
    m_enthalten = new QLabel(enthaltenText(m_vorlage), m_wahl);
    m_enthalten->setObjectName(QStringLiteral("exportContains"));
    m_enthalten->setWordWrap(true);
    form->addRow(tr("Contains:"), m_enthalten);

    // die Datei: Vorschlag im gemerkten Ordner
    QSettings einstellungen;
    QString ordner = einstellungen.value(QLatin1String(kOrdner)).toString();
    if (ordner.isEmpty() || !QFileInfo(ordner).isDir()) ordner = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    auto* dateiZeile = new QWidget(m_wahl);
    auto* dateiLayout = new QHBoxLayout(dateiZeile);
    dateiLayout->setContentsMargins(0, 0, 0, 0);
    m_datei = new QLineEdit(QDir::toNativeSeparators(QDir(ordner).filePath(exportName(m_loopName, m_vorlage))), dateiZeile);
    m_datei->setObjectName(QStringLiteral("exportFile"));
    m_datei->setMinimumWidth(320);
    m_waehlen = new QPushButton(tr("Browse..."), dateiZeile);
    m_waehlen->setObjectName(QStringLiteral("exportBrowse"));
    dateiLayout->addWidget(m_datei, 1);
    dateiLayout->addWidget(m_waehlen);
    form->addRow(tr("File:"), dateiZeile);
    connect(m_waehlen, &QPushButton::clicked, this, [this] { waehleDatei(); });
    connect(m_datei, &QLineEdit::textChanged, this, [this] { pruefeDatei(); });

    // das Format (WX2 a): 32-bit float vorgewaehlt, die letzte Wahl gemerkt
    auto* formate = new QWidget(m_wahl);
    auto* formatLayout = new QVBoxLayout(formate);
    formatLayout->setContentsMargins(0, 0, 0, 0);
    m_float = new QRadioButton(tr("32-bit float"), formate);
    m_float->setObjectName(QStringLiteral("exportFormatFloat"));
    m_float->setToolTip(tr("Keeps every value, also above 0 dB — the best for further work"));
    m_pcm24 = new QRadioButton(tr("24-bit"), formate);
    m_pcm24->setObjectName(QStringLiteral("exportFormat24"));
    m_pcm24->setToolTip(tr("Values above 0 dB are clipped"));
    m_pcm16 = new QRadioButton(tr("16-bit (dither)"), formate);
    m_pcm16->setObjectName(QStringLiteral("exportFormat16"));
    m_pcm16->setToolTip(tr("Like a CD; values above 0 dB are clipped"));
    auto* gruppe = new QButtonGroup(this);
    for (QRadioButton* knopf : {m_float, m_pcm24, m_pcm16})
    {
        gruppe->addButton(knopf);
        formatLayout->addWidget(knopf);
    }
    const QString format = einstellungen.value(QLatin1String(kFormat)).toString();
    (format == QLatin1String("pcm24") ? m_pcm24 : format == QLatin1String("pcm16") ? m_pcm16 : m_float)->setChecked(true);
    form->addRow(tr("Format:"), formate);

    // beim ganzen Loop: Ending und Passes (WX9 a)
    if (m_vorlage.art == ExportArt::Loop)
    {
        auto* enden = new QWidget(m_wahl);
        auto* endeLayout = new QVBoxLayout(enden);
        endeLayout->setContentsMargins(0, 0, 0, 0);
        m_nahtlos = new QRadioButton(tr("Seamless loop"), enden);
        m_nahtlos->setObjectName(QStringLiteral("exportSeamless"));
        m_nahtlos->setToolTip(tr("One pass as it sounds from the second on — repeats without a gap"));
        m_mitNachklang = new QRadioButton(tr("With tail"), enden);
        m_mitNachklang->setObjectName(QStringLiteral("exportWithTail"));
        m_mitNachklang->setToolTip(tr("Starts from silence and rings out after the end"));
        auto* endeGruppe = new QButtonGroup(this);
        endeGruppe->addButton(m_nahtlos);
        endeGruppe->addButton(m_mitNachklang);
        endeLayout->addWidget(m_nahtlos);
        endeLayout->addWidget(m_mitNachklang);
        (einstellungen.value(QLatin1String(kEnde)).toString() == QLatin1String("tail") ? m_mitNachklang : m_nahtlos)
            ->setChecked(true);
        form->addRow(tr("Ending:"), enden);
        m_durchlaeufe = new QSpinBox(m_wahl);
        m_durchlaeufe->setObjectName(QStringLiteral("exportPasses"));
        m_durchlaeufe->setRange(1, kDurchlaeufeMax);
        m_durchlaeufe->setValue(std::clamp(einstellungen.value(QLatin1String(kDurchlaeufe), 1).toInt(), 1, kDurchlaeufeMax));
        m_durchlaeufe->setToolTip(tr("How many times the loop plays in the file"));
        form->addRow(tr("Passes:"), m_durchlaeufe);
    }
    aussen->addWidget(m_wahl);

    // Fortschritt, Ergebnis, Warnung
    m_balken = new QProgressBar(this);
    m_balken->setObjectName(QStringLiteral("exportProgress"));
    m_balken->setRange(0, 100);
    m_status = new QLabel(this);
    m_status->setObjectName(QStringLiteral("exportStatus"));
    m_status->setWordWrap(true);
    m_warnung = new QLabel(this);
    m_warnung->setObjectName(QStringLiteral("exportWarning"));
    m_warnung->setWordWrap(true);
    m_warnung->setStyleSheet(QLatin1String(kWarnfarbe));
    aussen->addWidget(m_balken);
    aussen->addWidget(m_status);
    aussen->addWidget(m_warnung);

    auto* knoepfe = new QHBoxLayout;
    knoepfe->addStretch(1);
    m_ordner = new QPushButton(tr("Show in folder"), this);
    m_ordner->setObjectName(QStringLiteral("exportShowFolder"));
    m_abbrechen = new QPushButton(tr("Cancel"), this);
    m_abbrechen->setObjectName(QStringLiteral("exportCancel"));
    m_export = new QPushButton(tr("Export"), this);
    m_export->setObjectName(QStringLiteral("exportStart"));
    m_export->setDefault(true);
    m_schliessen = new QPushButton(tr("Close"), this);
    m_schliessen->setObjectName(QStringLiteral("exportClose"));
    for (QPushButton* knopf : {m_ordner, m_abbrechen, m_export, m_schliessen}) knoepfe->addWidget(knopf);
    aussen->addLayout(knoepfe);

    connect(m_export, &QPushButton::clicked, this, [this] { (void)starte(); });
    connect(m_abbrechen, &QPushButton::clicked, this, [this] { reject(); });
    connect(m_schliessen, &QPushButton::clicked, this, [this] { accept(); });
    connect(m_ordner, &QPushButton::clicked, this, [this] {
        const QString datei = m_datei->text();
        if (ordnerZeiger())
            ordnerZeiger()(datei);
        else
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(datei).absolutePath()));
    });

    m_uhr = new QTimer(this);
    m_uhr->setInterval(100);
    connect(m_uhr, &QTimer::timeout, this, [this] { zeigeStand(); });
}

void ExportFenster::zeigeWahl()
{
    m_wahl->setEnabled(true);
    m_balken->hide();
    m_warnung->clear();
    m_warnung->hide();
    m_export->show();
    m_abbrechen->show();
    m_abbrechen->setText(tr("Cancel"));
    m_ordner->hide();
    m_schliessen->hide();
    pruefeDatei();
}

void ExportFenster::pruefeDatei()
{
    if (laeuft()) return;
    const QString datei = m_datei->text().trimmed();
    m_export->setEnabled(!datei.isEmpty());
    m_status->setText(!datei.isEmpty() && QFileInfo::exists(datei) ? tr("This file exists and will be replaced.") : QString());
}

void ExportFenster::waehleDatei()
{
    const QString gewaehlt = QFileDialog::getSaveFileName(this, tr("Export audio"), m_datei->text(), tr("WAV (*.wav)"));
    if (!gewaehlt.isEmpty()) m_datei->setText(QDir::toNativeSeparators(gewaehlt));
}

ExportWunsch ExportFenster::wunschJetzt() const
{
    ExportWunsch wunsch = m_vorlage;
    QString datei = QDir::fromNativeSeparators(m_datei->text().trimmed());
    if (!datei.endsWith(QLatin1String(".wav"), Qt::CaseInsensitive)) datei += QStringLiteral(".wav");
    wunsch.datei = datei;
    wunsch.tiefe = m_pcm24->isChecked()   ? audio::WavTiefe::Pcm24
                   : m_pcm16->isChecked() ? audio::WavTiefe::Pcm16
                                          : audio::WavTiefe::Gleitkomma32;
    if (m_vorlage.art == ExportArt::Loop)
    {
        wunsch.ende = m_mitNachklang->isChecked() ? LoopEnde::Nachklang : LoopEnde::Nahtlos;
        wunsch.durchlaeufe = m_durchlaeufe->value();
    }
    return wunsch;
}

void ExportFenster::merke() const
{
    const ExportWunsch wunsch = wunschJetzt();
    QSettings einstellungen;
    einstellungen.setValue(QLatin1String(kFormat), formatSchluessel(wunsch.tiefe));
    einstellungen.setValue(QLatin1String(kOrdner), QFileInfo(wunsch.datei).absolutePath());
    if (m_vorlage.art == ExportArt::Loop)
    {
        einstellungen.setValue(QLatin1String(kEnde), wunsch.ende == LoopEnde::Nachklang ? QStringLiteral("tail")
                                                                                       : QStringLiteral("seamless"));
        einstellungen.setValue(QLatin1String(kDurchlaeufe), wunsch.durchlaeufe);
    }
}

bool ExportFenster::starte()
{
    // solange ein Auftrag da ist — laufend oder noch nicht gezeigt —, beginnt kein zweiter
    if (m_auftrag != nullptr || m_datei->text().trimmed().isEmpty()) return false;
    const ExportWunsch wunsch = wunschJetzt();
    m_datei->setText(QDir::toNativeSeparators(wunsch.datei));
    merke();
    m_ergebnis.reset();
    m_auftrag = std::make_unique<ExportAuftrag>(wunsch, m_dekodierer);
    m_wahl->setEnabled(false);
    m_export->hide();
    m_balken->setValue(0);
    m_balken->show();
    m_warnung->hide();
    zeigeStand();
    m_uhr->start();
    return true;
}

void ExportFenster::warteAufEnde()
{
    if (m_auftrag == nullptr) return;
    m_auftrag->warte();
    zeigeStand();
}

void ExportFenster::zeigeStand()
{
    if (m_auftrag == nullptr) return;
    if (m_auftrag->beendet())
    {
        m_uhr->stop();
        zeigeEnde();
        return;
    }
    const ExportStand stand = m_auftrag->stand();
    switch (stand.phase)
    {
    case ExportPhase::Toene:
        m_status->setText(tr("Preparing sounds… %1 / %2").arg(stand.toeneErledigt).arg(stand.toeneGesamt));
        m_balken->setValue(stand.toeneGesamt > 0 ? 100 * stand.toeneErledigt / stand.toeneGesamt : 0);
        break;
    case ExportPhase::Rechnen:
        m_status->setText(tr("Rendering… %1 %").arg(static_cast<int>(std::lround(100.0 * stand.anteil))));
        m_balken->setValue(static_cast<int>(std::lround(100.0 * stand.anteil)));
        break;
    case ExportPhase::Schreiben:
        m_status->setText(tr("Writing the file…"));
        m_balken->setValue(100);
        break;
    default:
        break;
    }
}

void ExportFenster::zeigeEnde()
{
    const auto phase = m_auftrag->stand().phase;
    m_ergebnis = m_auftrag->ergebnis();
    m_auftrag.reset();
    if (phase == ExportPhase::Abgebrochen || !m_ergebnis)
    {
        zeigeWahl();
        m_status->setText(tr("Export cancelled — nothing was written."));
        return;
    }
    const ExportErgebnis& e = *m_ergebnis;
    if (phase == ExportPhase::Fehler)
    {
        zeigeWahl();
        m_warnung->setText(e.fehler);
        m_warnung->show();
        return;
    }

    m_balken->hide();
    m_abbrechen->hide();
    m_ordner->show();
    m_schliessen->show();
    m_schliessen->setDefault(true);
    const ExportWunsch wunsch = wunschJetzt();
    QString dauer = tr("Exported %1 s").arg(sekundenText(e.frames, e.rate));
    if (e.frames > e.clipFrames) dauer += tr(" (tail %1 s)").arg(sekundenText(e.frames - e.clipFrames, e.rate));
    else if (wunsch.art == ExportArt::Loop && wunsch.durchlaeufe > 1) dauer += tr(" (%1 passes)").arg(wunsch.durchlaeufe);
    m_status->setText(QStringLiteral("%1 · %2 · %3").arg(dauer, tiefeText(wunsch.tiefe), groesseText(e.bytes)));

    QStringList warnungen;
    if (e.wav.gekappt > 0)
        warnungen << tr("%1 values above 0 dB were clipped — 32-bit float keeps them").arg(e.wav.gekappt);
    if (e.nachklangGekappt)
        warnungen << tr("The tail was cut after %1 s.").arg(kNachklangHoechstensMs / 1000);
    if (!e.bericht.isEmpty()) warnungen << tr("Not found, silent: %1").arg(e.bericht.join(QStringLiteral("; ")));
    m_warnung->setText(warnungen.join(QLatin1Char('\n')));
    m_warnung->setVisible(!warnungen.isEmpty());
}

void ExportFenster::reject()
{
    if (laeuft())
    {
        // Cancel waehrend des Exports bricht ab — das Fenster bleibt
        m_auftrag->brichAb();
        return;
    }
    QDialog::reject();
}

}  // namespace lumi::loop
