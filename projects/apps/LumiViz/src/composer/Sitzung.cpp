/**
 * @file Sitzung.cpp
 * @brief Composer: die geladene Komposition eines Fensters und ihr Transport (Stufe C2b)
 */

#include "composer/Sitzung.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/Bildquelle.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSettings>
#include <QTimer>

#include <BasicLogger.h>

#include <algorithm>
#include <memory>

namespace lumi::composer
{

namespace
{
const QLatin1String kResume("composer/resume");
const QLatin1String kSchleife("composer/loop");
const QLatin1String kOeffnenHaelt("composer/openHolds");
const QLatin1String kAmEnde("composer/atEnd");
// der Stand ueber den Neustart
const QLatin1String kStandDatei("composer/session/file");
const QLatin1String kStandAn("composer/session/on");
const QLatin1String kStandStelle("composer/session/positionMs");
const QLatin1String kStandLief("composer/session/wasPlaying");

constexpr int kTaktMs = 33;
}  // namespace

// =============================================================================
// Anlegen
// =============================================================================

Sitzung::Sitzung(VisualizerWidget& widget) : QObject(&widget), m_widget(widget)
{
    m_schleife = QSettings().value(kSchleife, false).toBool();

    m_takt = new QTimer(this);
    m_takt->setInterval(kTaktMs);
    connect(m_takt, &QTimer::timeout, this, &Sitzung::beimTakt);

    // Beim geordneten Beenden: den Stand fuer den naechsten Start festhalten
    // und die Messung einer noch laufenden Komposition ins Log schreiben
    // (Befund S94 — sonst kaeme sie nie dorthin). Der Render-Thread ist dann
    // schon angehalten, die Bildquelle lebt noch.
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        sichereStand();
        protokolliereMessung();
    });
}

Sitzung& Sitzung::fuer(VisualizerWidget& widget)
{
    if (Sitzung* da = finde(widget)) return *da;
    return *new Sitzung(widget);  // Kind des Fensters
}

Sitzung* Sitzung::finde(const VisualizerWidget& widget)
{
    return widget.findChild<Sitzung*>(QString(), Qt::FindDirectChildrenOnly);
}

Bildquelle* Sitzung::quelle() const
{
    return dynamic_cast<Bildquelle*>(m_widget.bildquelle());
}

// =============================================================================
// Die Komposition
// =============================================================================

bool Sitzung::lade(const QString& pfad, QStringList* bericht)
{
    Komposition neu;
    if (!ladeKomposition(pfad, neu, bericht)) return false;
    neu.richte();  // was das Panel zeigt, ist was laeuft

    setAn(false);
    m_komposition = std::move(neu);
    m_datei = QFileInfo(pfad).absoluteFilePath();
    m_geladen = true;
    m_stelleMs = 0;
    Q_EMIT geaendert();
    return true;
}

bool Sitzung::oeffne(const QString& pfad, QStringList* bericht)
{
    if (!lade(pfad, bericht)) return false;
    setAn(true);
    BasicLogger::logInfo("Composer: composition started: " +
                         QDir::toNativeSeparators(pfad).toStdString());
    if (!oeffnenHaelt()) spiele();
    return true;
}

void Sitzung::entlade()
{
    if (!m_geladen) return;
    setAn(false);
    m_komposition = Komposition{};
    m_datei.clear();
    m_geladen = false;
    m_stelleMs = 0;
    BasicLogger::logInfo("Composer: composition closed");
    Q_EMIT geaendert();
}

// =============================================================================
// Composer on
// =============================================================================

void Sitzung::setAn(bool an)
{
    if (an == this->an()) return;
    if (an)
    {
        if (!m_geladen) return;
        // Gesetzt wird vor der Uebergabe an den Render-Thread — danach nur
        // noch unter dem Render-Mutex.
        auto neu = std::make_unique<Bildquelle>();
        neu->setKomposition(m_komposition);
        neu->uhr().setSchleife(m_schleife);
        neu->uhr().springe(m_stelleMs);
        neu->spiegleStand();
        m_widget.setBildquelle(std::move(neu));
        if (!this->an()) return;  // das Fenster hat keinen Render-Thread
        m_takt->start();
        BasicLogger::logInfo("Composer: on");
    }
    else
    {
        protokolliereMessung();
        {
            QMutexLocker halt(&m_widget.renderMutex());
            m_stelleMs = quelle()->uhr().stelleMs();
        }
        m_takt->stop();
        m_widget.setBildquelle(nullptr);
        BasicLogger::logInfo("Composer: off");
    }
    Q_EMIT geaendert();
}

// =============================================================================
// Transport
// =============================================================================

void Sitzung::spiele()
{
    if (!m_geladen) return;
    setAn(true);
    Bildquelle* q = quelle();
    if (q == nullptr) return;
    {
        QMutexLocker halt(&m_widget.renderMutex());
        q->uhr().starte();
        q->spiegleStand();
    }
    Q_EMIT geaendert();
}

void Sitzung::pausiere()
{
    Bildquelle* q = quelle();
    if (q == nullptr) return;
    {
        QMutexLocker halt(&m_widget.renderMutex());
        q->uhr().stoppe();
        q->spiegleStand();
    }
    Q_EMIT geaendert();
}

void Sitzung::stoppe()
{
    if (Bildquelle* q = quelle())
    {
        QMutexLocker halt(&m_widget.renderMutex());
        q->uhr().stoppe();
        q->uhr().anDenAnfang();
        q->spiegleStand();
    }
    m_stelleMs = 0;
    Q_EMIT geaendert();
}

void Sitzung::springe(std::int64_t stelleMs)
{
    if (!m_geladen) return;
    stelleMs = std::clamp<std::int64_t>(stelleMs, 0, laengeMs());
    if (Bildquelle* q = quelle())
    {
        QMutexLocker halt(&m_widget.renderMutex());
        q->uhr().springe(stelleMs);
        q->spiegleStand();
    }
    m_stelleMs = stelleMs;
    Q_EMIT geaendert();
}

bool Sitzung::laeuft() const
{
    const Bildquelle* q = quelle();
    return q != nullptr && q->standLaeuft();
}

std::int64_t Sitzung::stelleMs() const
{
    const Bildquelle* q = quelle();
    return q != nullptr ? q->standStelleMs() : m_stelleMs;
}

void Sitzung::setSchleife(bool an)
{
    if (an == m_schleife) return;
    m_schleife = an;
    QSettings().setValue(kSchleife, an);
    if (Bildquelle* q = quelle())
    {
        QMutexLocker halt(&m_widget.renderMutex());
        q->uhr().setSchleife(an);
        q->spiegleStand();
    }
    Q_EMIT geaendert();
}

void Sitzung::beimTakt()
{
    Bildquelle* q = quelle();
    if (q == nullptr)
    {
        m_takt->stop();
        return;
    }
    // Rechnet der Render-Thread gerade nicht (Fenster verdeckt), spiegelt
    // niemand den Stand — dann hier, aber ohne auf ihn zu warten.
    if (m_widget.renderMutex().tryLock())
    {
        q->spiegleStand();
        m_widget.renderMutex().unlock();
    }
    if (q->standAmEnde())
    {
        switch (amEnde())
        {
            case AmEnde::Halten:
                break;
            case AmEnde::AnDenAnfang:
                stoppe();
                break;
            case AmEnde::ComposerAus:
                setAn(false);
                m_stelleMs = 0;
                Q_EMIT geaendert();
                return;
        }
    }
    Q_EMIT takt();
}

// =============================================================================
// Einstellungen
// =============================================================================

bool Sitzung::resumeAn()
{
    return QSettings().value(kResume, false).toBool();
}

void Sitzung::setResume(bool an)
{
    QSettings().setValue(kResume, an);
}

bool Sitzung::oeffnenHaelt()
{
    return QSettings().value(kOeffnenHaelt, false).toBool();
}

void Sitzung::setOeffnenHaelt(bool haelt)
{
    QSettings().setValue(kOeffnenHaelt, haelt);
}

AmEnde Sitzung::amEnde()
{
    const int wert = QSettings().value(kAmEnde, 0).toInt();
    return wert == 1 ? AmEnde::AnDenAnfang : wert == 2 ? AmEnde::ComposerAus : AmEnde::Halten;
}

void Sitzung::setAmEnde(AmEnde art)
{
    QSettings().setValue(kAmEnde, static_cast<int>(art));
}

// =============================================================================
// Ueber den Neustart
// =============================================================================

void Sitzung::sichereStand() const
{
    std::int64_t stelle = m_stelleMs;
    bool lief = false;
    if (Bildquelle* q = quelle())
    {
        QMutexLocker halt(&m_widget.renderMutex());
        stelle = q->uhr().stelleMs();
        lief = q->uhr().laeuft();
    }
    QSettings s;
    s.setValue(kStandDatei, m_geladen ? m_datei : QString());
    s.setValue(kStandAn, an());
    s.setValue(kStandStelle, static_cast<qlonglong>(stelle));
    s.setValue(kStandLief, lief);
}

bool Sitzung::stelleWiederHer(QStringList* bericht)
{
    QSettings s;
    const QString datei = s.value(kStandDatei).toString();
    if (datei.isEmpty() || !QFileInfo::exists(datei)) return false;
    if (!lade(datei, bericht)) return false;
    BasicLogger::logInfo("Composer: restored composition: " +
                         QDir::toNativeSeparators(datei).toStdString());
    if (!resumeAn()) return true;  // im Panel, Composer aus — die Effektkette laeuft

    springe(s.value(kStandStelle, 0).toLongLong());
    if (s.value(kStandAn, false).toBool())
    {
        setAn(true);
        if (s.value(kStandLief, false).toBool()) spiele();
        BasicLogger::logInfo("Composer: resumed at " + std::to_string(stelleMs() / 1000) + " s");
    }
    return true;
}

void Sitzung::protokolliereMessung()
{
    QStringList zeilen;
    {
        QMutexLocker halt(&m_widget.renderMutex());
        const Bildquelle* q = quelle();
        if (q == nullptr) return;
        zeilen = q->messung().alsText() + q->anlaufText();
    }
    for (const QString& zeile : zeilen) BasicLogger::logInfo(zeile.toStdString());
}

}  // namespace lumi::composer
