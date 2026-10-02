/**
 * @file KaraokePanel.cpp
 * @brief Panel der Karaoke-Ebene (S89, KE2)
 */

#include "UI/panels/KaraokePanel.hpp"

#include "UI/panels/MultiEffectPanel.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"  // sanitize

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>

namespace
{
const QString kAktuell = QStringLiteral("current");  // Name der laufenden Ebene
}

KaraokePanel::KaraokePanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "karaoke", tr("Karaoke"), parent)
{
    setupUI();
}

void KaraokePanel::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);

    // --- Hauptschalter (bis S88 unter Settings → Audio)
    m_schalter = new QCheckBox(tr("Karaoke on"), this);
    m_schalter->setObjectName(QStringLiteral("karaokeSchalter"));
    m_schalter->setChecked(QSettings().value(QStringLiteral("karaoke/enabled"), false).toBool());
    m_schalter->setToolTip(
        tr("Shows the lyrics of the playing track on top of every preset, without a "
           "Lyrics node in the chain. If the preset has its own enabled Lyrics node, the "
           "preset wins and the layer steps back."));
    connect(m_schalter, &QCheckBox::toggled, this, [](bool an) {
        QSettings().setValue(QStringLiteral("karaoke/enabled"), an);
        MultiEffectVisualizer::setKaraokeEbene(an);
    });
    root->addWidget(m_schalter);

    // --- Karaoke-Presets: Stand der ganzen Ebene als Datei (.lvkar)
    auto* zeile = new QHBoxLayout();
    zeile->addWidget(new QLabel(tr("Karaoke preset:"), this));
    m_presets = new QComboBox(this);
    m_presets->setObjectName(QStringLiteral("karaokePresets"));
    m_presets->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_presets->setMinimumContentsLength(12);
    zeile->addWidget(m_presets, 1);
    auto* laden = new QPushButton(tr("Load"), this);
    laden->setToolTip(tr("Replace the karaoke layer with the selected preset."));
    auto* speichern = new QPushButton(tr("Save…"), this);
    speichern->setToolTip(tr("Save the current karaoke layer (lyrics and trail) under a name."));
    auto* loeschen = new QPushButton(tr("Delete"), this);
    auto* vorgabe = new QPushButton(tr("Default"), this);
    vorgabe->setToolTip(tr("Back to the built-in look of the karaoke layer."));
    for (QPushButton* b : {laden, speichern, loeschen, vorgabe}) zeile->addWidget(b);
    root->addLayout(zeile);

    connect(laden, &QPushButton::clicked, this, [this] {
        const QString name = m_presets->currentText();
        if (name.isEmpty()) return;
        if (!ladePreset(name))
            QMessageBox::warning(this, tr("Karaoke preset"),
                                 tr("Karaoke preset '%1' could not be loaded.").arg(name));
    });
    connect(speichern, &QPushButton::clicked, this, [this] {
        bool ok = false;
        const QString name = QInputDialog::getText(this, tr("Save karaoke preset"), tr("Name:"),
                                                   QLineEdit::Normal, m_presets->currentText(), &ok);
        if (!ok) return;
        const QString datei = dateiVon(name);
        if (!datei.isEmpty() && QFileInfo::exists(datei) &&
            QMessageBox::question(this, tr("Karaoke preset"),
                                  tr("'%1' already exists. Overwrite?").arg(name)) !=
                QMessageBox::Yes)
            return;
        if (!speicherePreset(name))
            QMessageBox::warning(this, tr("Karaoke preset"),
                                 tr("Karaoke preset '%1' could not be saved.").arg(name));
    });
    connect(loeschen, &QPushButton::clicked, this, [this] {
        const QString name = m_presets->currentText();
        if (name.isEmpty()) return;
        if (QMessageBox::question(this, tr("Karaoke preset"),
                                  tr("Delete karaoke preset '%1'?").arg(name)) != QMessageBox::Yes)
            return;
        (void)loeschePreset(name);
    });
    connect(vorgabe, &QPushButton::clicked, this, [this] { setzeVorgabe(); });

    // --- Reiter: 1 Lyrics, 2 Lyrics Trail — beides das Ketten-Panel im
    //     eingeschraenkten Modus (K2 a)
    m_reiter = new QTabWidget(this);
    m_editor = new MultiEffectPanel(services(), MultiEffectPanel::KaraokeLyrics{}, m_reiter);
    m_reiter->addTab(m_editor, tr("Lyrics"));

    auto* trailSeite = new QWidget(m_reiter);
    auto* trailLayout = new QVBoxLayout(trailSeite);
    trailLayout->setContentsMargins(0, 6, 0, 0);
    m_trailSchalter = new QCheckBox(tr("Trail on"), trailSeite);
    m_trailSchalter->setObjectName(QStringLiteral("karaokeTrailSchalter"));
    m_trailSchalter->setToolTip(
        tr("The line that just ended drifts away behind the running line. The list below "
           "works on the trail image only; the Lyrics Trail at its end is fixed."));
    trailLayout->addWidget(m_trailSchalter);
    m_trail = new MultiEffectPanel(services(), MultiEffectPanel::KaraokeTrail{}, trailSeite);
    trailLayout->addWidget(m_trail, 1);
    m_reiter->addTab(trailSeite, tr("Lyrics Trail"));
    root->addWidget(m_reiter, 1);

    connect(m_trailSchalter, &QCheckBox::toggled, this, [this](bool an) {
        if (m_trail->hatHost() && m_trail->wurzelAn() != an) m_trail->setWurzelAn(an);
    });
    connect(m_reiter, &QTabWidget::currentChanged, this, [this] { gleicheTrailSchalterAb(); });

    fuelleCombo();
    gleicheTrailSchalterAb();
}

void KaraokePanel::gleicheTrailSchalterAb()
{
    const bool da = m_trail->hatHost();
    m_trailSchalter->setEnabled(da);
    if (!da) return;
    const QSignalBlocker ruhe(m_trailSchalter);
    m_trailSchalter->setChecked(m_trail->wurzelAn());
}

void KaraokePanel::showEvent(QShowEvent* event)
{
    PanelBase::showEvent(event);
    gleicheTrailSchalterAb();
}

QString KaraokePanel::ordner() const
{
    const QString ablage = MultiEffectVisualizer::karaokeAblage();
    return ablage.isEmpty() ? QString() : QFileInfo(ablage).absolutePath();
}

QString KaraokePanel::dateiVon(const QString& name) const
{
    const QString sauber = lumi::multieffect::nodepresets::sanitize(name);
    const QString dir = ordner();
    if (sauber.isEmpty() || dir.isEmpty()) return {};
    // die laufende Ebene ist kein Preset (unter Windows ohne Gross/Klein)
    if (sauber.compare(kAktuell, Qt::CaseInsensitive) == 0) return {};
    return dir + QLatin1Char('/') + sauber + QLatin1String(kEndung);
}

QStringList KaraokePanel::presetNamen() const
{
    QStringList namen;
    const QString dir = ordner();
    if (dir.isEmpty()) return namen;
    const QStringList dateien = QDir(dir).entryList(
        {QStringLiteral("*") + QLatin1String(kEndung)}, QDir::Files, QDir::Name | QDir::IgnoreCase);
    for (const QString& d : dateien)
    {
        const QString name = QFileInfo(d).completeBaseName();
        if (name.compare(kAktuell, Qt::CaseInsensitive) != 0) namen.append(name);
    }
    return namen;
}

void KaraokePanel::fuelleCombo(const QString& gewaehlt)
{
    const QString vorher = gewaehlt.isEmpty() ? m_presets->currentText() : gewaehlt;
    m_presets->clear();
    m_presets->addItems(presetNamen());
    const int i = m_presets->findText(vorher, Qt::MatchFixedString);
    if (i >= 0) m_presets->setCurrentIndex(i);
}

bool KaraokePanel::speicherePreset(const QString& name)
{
    const QString datei = dateiVon(name);
    if (datei.isEmpty() || !m_editor->karaokeSpeichern(datei)) return false;
    fuelleCombo(QFileInfo(datei).completeBaseName());
    return true;
}

bool KaraokePanel::ladePreset(const QString& name)
{
    const QString datei = dateiVon(name);
    if (datei.isEmpty() || !QFileInfo::exists(datei)) return false;
    if (!m_editor->karaokeLaden(datei)) return false;
    m_trail->baumNeuLesen();
    gleicheTrailSchalterAb();
    return true;
}

bool KaraokePanel::loeschePreset(const QString& name)
{
    const QString datei = dateiVon(name);
    if (datei.isEmpty() || !QFile::remove(datei)) return false;
    fuelleCombo();
    return true;
}

void KaraokePanel::setzeVorgabe()
{
    m_editor->karaokeVorgabe();
    m_trail->baumNeuLesen();
    gleicheTrailSchalterAb();
}
