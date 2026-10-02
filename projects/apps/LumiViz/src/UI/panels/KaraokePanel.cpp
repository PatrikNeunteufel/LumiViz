/**
 * @file KaraokePanel.cpp
 * @brief Panel der Karaoke-Ebene (S89, KE2)
 */

#include "UI/panels/KaraokePanel.hpp"

#include "UI/PresetListen.hpp"
#include "UI/panels/MultiEffectPanel.hpp"
#include "UI/widgets/PresetZeile.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

using lumi::ui::PresetArt;
using lumi::ui::PresetListen;

namespace
{
/// Ordner, in dem die Dateidialoge beginnen: der zuletzt benutzte, sonst der
/// Ordner der Ablage (`<Benutzerdaten>/karaoke`).
QString startOrdner()
{
    const QString gemerkt = QSettings().value(QStringLiteral("karaoke/lastDir")).toString();
    if (!gemerkt.isEmpty() && QDir(gemerkt).exists()) return gemerkt;
    const QString ablage = MultiEffectVisualizer::karaokeAblage();
    return ablage.isEmpty() ? QString() : QFileInfo(ablage).absolutePath();
}

void merkeOrdner(const QString& datei)
{
    QSettings().setValue(QStringLiteral("karaoke/lastDir"), QFileInfo(datei).absolutePath());
}

/// Ist `pfad` die Ablage der laufenden Ebene selbst? Sie ist kein Preset.
bool istAblage(const QString& pfad)
{
    const QString ablage = MultiEffectVisualizer::karaokeAblage();
    return !ablage.isEmpty() &&
           PresetListen::normal(pfad).compare(PresetListen::normal(ablage), Qt::CaseInsensitive) == 0;
}

/// Die laufende Ebene in ihre Ablage schreiben (ausserhalb des Render-Mutex).
void schreibeAblage(const lumi::multieffect::ChainNode& ebene)
{
    const QString ablage = MultiEffectVisualizer::karaokeAblage();
    if (ablage.isEmpty()) return;
    QDir().mkpath(QFileInfo(ablage).absolutePath());
    (void)lumi::multieffect::saveChainToFile(ebene, ablage);
}
}  // namespace

KaraokePanel::KaraokePanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "karaoke", tr("Karaoke"), parent)
{
    setupUI();
}

KaraokePanel::Ziel KaraokePanel::ziel()
{
    return Ziel{m_editor->host(), m_editor->renderMutex(), eventBus(), this};
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
    connect(m_schalter, &QCheckBox::toggled, this,
            [this](bool an) { setzeKaraokeAn(eventBus(), an); });
    auto* kopf = new QHBoxLayout();
    kopf->addWidget(m_schalter);
    m_zustand = new QLabel(this);
    m_zustand->setObjectName(QStringLiteral("karaokeZustand"));
    m_zustand->setToolTip(
        tr("The running preset has its own enabled Lyrics node. It shows the lyrics its own "
           "way, and the karaoke layer steps back while it runs."));
    kopf->addWidget(m_zustand, 1);
    root->addLayout(kopf);
    auto* takt = new QTimer(this);
    takt->setInterval(500);
    connect(takt, &QTimer::timeout, this, [this] { aktualisiereZustand(); });
    takt->start();

    // --- Preset-Zeile: wie im Panel „Effect Chain" (Auftrag Patrik S89)
    m_presetZeile = new lumi::ui::PresetZeile(PresetArt::Karaoke, this);
    m_presetZeile->setLaufend(PresetListen::laufendesKaraokePreset());
    auto* vorgabe = new QPushButton(tr("Default"), this);
    vorgabe->setObjectName(QStringLiteral("karaokeVorgabe"));
    vorgabe->setToolTip(tr("Back to the built-in look of the karaoke layer."));
    m_presetZeile->haengeAn(vorgabe);
    root->addWidget(m_presetZeile);

    connect(m_presetZeile, &lumi::ui::PresetZeile::ladeDatei, this, [this](const QString& pfad) {
        // Cancel in der Rueckfrage ist kein Ladefehler
        if (!darfKaraokeErsetzen(ziel(), tr("Load Karaoke Preset"))) return;
        QStringList report;
        if (!ladeKaraokeDatei(ziel(), pfad, &report))
            QMessageBox::warning(
                this, tr("Karaoke preset"),
                tr("Could not load:\n%1\n%2").arg(pfad, report.join(QLatin1Char('\n'))));
    });
    connect(m_presetZeile, &lumi::ui::PresetZeile::ladenDialog, this, [this] {
        if (!darfKaraokeErsetzen(ziel(), tr("Load Karaoke Preset"))) return;
        const QString pfad = QFileDialog::getOpenFileName(
            this, tr("Load Karaoke Preset"), startOrdner(),
            tr("LumiViz Karaoke Preset (*.lvkar);;All Files (*)"));
        if (pfad.isEmpty()) return;
        merkeOrdner(pfad);
        QStringList report;
        if (!ladeKaraokeDatei(ziel(), pfad, &report))
            QMessageBox::warning(
                this, tr("Karaoke preset"),
                tr("Could not load:\n%1\n%2").arg(pfad, report.join(QLatin1Char('\n'))));
    });
    connect(m_presetZeile, &lumi::ui::PresetZeile::speichern, this,
            [this](bool saveAs) { (void)speichereKaraoke(ziel(), saveAs); });
    connect(m_presetZeile, &lumi::ui::PresetZeile::listenGeaendert, this, [this] {
        if (IEventBus* bus = eventBus()) bus->publish(PresetListsChangedEvent{});
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

    gleicheTrailSchalterAb();

    if (IEventBus* bus = eventBus())
    {
        // Menue oder Tastenkuerzel haben geschaltet: der Haken zieht nach
        m_eventSubscriptions.push_back(bus->subscribeScoped<KaraokeEnabledChangedEvent>(
            [this](const KaraokeEnabledChangedEvent& e) {
                const QSignalBlocker ruhe(m_schalter);
                m_schalter->setChecked(e.enabled);
                aktualisiereZustand();
            }));
        // Die Ebene wurde ersetzt (Laden im Panel, Doppelklick auf ein .lvkar
        // im Effect Browser): beide Reiter neu lesen
        m_eventSubscriptions.push_back(bus->subscribeScoped<KaraokeLayerChangedEvent>(
            [this](const KaraokeLayerChangedEvent&) {
                m_editor->baumNeuLesen();
                m_trail->baumNeuLesen();
                gleicheTrailSchalterAb();
                m_presetZeile->setGeaendert(false);
                m_presetZeile->setLaufend(PresetListen::laufendesKaraokePreset());
            }));
        m_eventSubscriptions.push_back(bus->subscribeScoped<PresetListsChangedEvent>(
            [this](const PresetListsChangedEvent&) {
                m_presetZeile->setLaufend(PresetListen::laufendesKaraokePreset());
            }));
    }
}

void KaraokePanel::setzeKaraokeAn(IEventBus* bus, bool an)
{
    QSettings().setValue(QStringLiteral("karaoke/enabled"), an);
    MultiEffectVisualizer::setKaraokeEbene(an);
    if (bus != nullptr) bus->publish(KaraokeEnabledChangedEvent{an});
}

// =============================================================================
// Karaoke-Presets
// =============================================================================

bool KaraokePanel::ladeKaraokeDatei(const Ziel& ziel, const QString& pfad, QStringList* report)
{
    if (ziel.host == nullptr || ziel.mutex == nullptr) return false;
    lumi::multieffect::ChainNode ebene;
    {
        QMutexLocker lock(ziel.mutex);
        if (!ziel.host->ladeKaraokeDatei(pfad, report)) return false;  // merkt den Stand selbst
        ebene = ziel.host->karaokeKette();
    }
    schreibeAblage(ebene);  // das geladene Preset ist jetzt die laufende Ebene
    PresetListen::setzeLaufendesKaraokePreset(pfad);
    PresetListen::merkeBenutzt(PresetArt::Karaoke, pfad);
    if (ziel.bus != nullptr)
    {
        ziel.bus->publish(KaraokeLayerChangedEvent{});
        ziel.bus->publish(PresetListsChangedEvent{});
    }
    return true;
}

bool KaraokePanel::speichereKaraokeNach(const Ziel& ziel, const QString& pfad)
{
    if (ziel.host == nullptr || ziel.mutex == nullptr || pfad.isEmpty()) return false;
    if (istAblage(pfad)) return false;  // die laufende Ebene ist kein Preset
    lumi::multieffect::ChainNode ebene;
    {
        QMutexLocker lock(ziel.mutex);
        ebene = ziel.host->karaokeKette();
    }
    QDir().mkpath(QFileInfo(pfad).absolutePath());
    if (!lumi::multieffect::saveChainToFile(ebene, pfad)) return false;
    {
        QMutexLocker lock(ziel.mutex);
        ziel.host->merkeKaraokeStand();  // ab jetzt gilt die Ebene als gespeichert
    }
    PresetListen::setzeLaufendesKaraokePreset(pfad);
    PresetListen::merkeBenutzt(PresetArt::Karaoke, pfad);
    if (ziel.bus != nullptr) ziel.bus->publish(PresetListsChangedEvent{});
    return true;
}

bool KaraokePanel::speichereKaraoke(const Ziel& ziel, bool saveAs)
{
    if (ziel.host == nullptr || ziel.mutex == nullptr) return false;
    const QString laufend = PresetListen::laufendesKaraokePreset();
    QString pfad;
    if (!saveAs && !laufend.isEmpty() && QFileInfo::exists(laufend))
    {
        pfad = laufend;  // „Save": das laufende Karaoke-Preset, ohne Dialog
    }
    else
    {
        const QString vorschlag =
            !laufend.isEmpty() ? laufend
                               : startOrdner() + QStringLiteral("/Karaoke") + QLatin1String(kEndung);
        pfad = QFileDialog::getSaveFileName(ziel.parent, tr("Save Karaoke Preset"), vorschlag,
                                            tr("LumiViz Karaoke Preset (*.lvkar)"));
        if (pfad.isEmpty()) return false;
        if (!pfad.endsWith(QLatin1String(kEndung), Qt::CaseInsensitive))
            pfad += QLatin1String(kEndung);
        merkeOrdner(pfad);
    }
    if (istAblage(pfad))
    {
        QMessageBox::information(
            ziel.parent, tr("Save Karaoke Preset"),
            tr("This file is the running karaoke layer itself — it saves on its own. "
               "Please choose another name."));
        return false;
    }
    if (!speichereKaraokeNach(ziel, pfad))
    {
        QMessageBox::warning(ziel.parent, tr("Save Karaoke Preset"),
                             tr("Could not save:\n%1").arg(pfad));
        return false;
    }
    return true;
}

bool KaraokePanel::darfKaraokeErsetzen(const Ziel& ziel, const QString& titel)
{
    if (ziel.host == nullptr || ziel.mutex == nullptr) return true;
    bool geaendert = false;
    {
        QMutexLocker lock(ziel.mutex);
        geaendert = ziel.host->karaokeGeaendert();
    }
    if (!geaendert) return true;

    const QString laufend = PresetListen::laufendesKaraokePreset();
    const QString name =
        laufend.isEmpty() ? tr("The karaoke layer") : QFileInfo(laufend).fileName();
    QMessageBox frage(ziel.parent);
    frage.setIcon(QMessageBox::Question);
    frage.setWindowTitle(titel);
    frage.setText(tr("%1 has unsaved changes.").arg(name));
    frage.setInformativeText(tr("Save them before continuing?"));
    frage.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    frage.setDefaultButton(QMessageBox::Save);
    switch (frage.exec())
    {
        case QMessageBox::Save: return speichereKaraoke(ziel, false);  // abgebrochen = zurueck
        case QMessageBox::Discard: return true;
        default: return false;
    }
}

bool KaraokePanel::ladeDatei(const QString& pfad)
{
    if (!darfKaraokeErsetzen(ziel(), tr("Load Karaoke Preset"))) return false;
    return ladeKaraokeDatei(ziel(), pfad);
}

void KaraokePanel::setzeVorgabe()
{
    if (!darfKaraokeErsetzen(ziel(), tr("Default Karaoke Layer"))) return;
    m_editor->karaokeVorgabe();  // setzt, liest neu, schreibt die Ablage
    m_trail->baumNeuLesen();
    gleicheTrailSchalterAb();
    PresetListen::setzeLaufendesKaraokePreset(QString());
    m_presetZeile->setGeaendert(false);
    m_presetZeile->setLaufend(QString());
}

// =============================================================================
// Zustand
// =============================================================================

void KaraokePanel::aktualisiereZustand()
{
    if (!isVisible()) return;  // die Abfragen nehmen den Render-Mutex — nur wenn man es sieht
    const bool zurueck = MultiEffectVisualizer::karaokeEbene() && m_editor->ketteZeigtLyrics();
    m_zustand->setText(zurueck ? tr("Preset shows its own lyrics") : QString());
    const Ziel z = ziel();
    if (z.host != nullptr && z.mutex != nullptr)
    {
        bool geaendert = false;
        {
            QMutexLocker lock(z.mutex);
            geaendert = z.host->karaokeGeaendert();
        }
        m_presetZeile->setGeaendert(geaendert);
    }
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
