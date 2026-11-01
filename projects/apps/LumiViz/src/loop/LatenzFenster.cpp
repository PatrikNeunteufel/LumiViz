/**
 * @file LatenzFenster.cpp
 * @brief Loop: das Fenster Measure latency (Stufe 15k-1, S169)
 */

#include "loop/LatenzFenster.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace lumi::loop
{

namespace
{

constexpr int kTaktMs = 30;

}  // namespace

LatenzFenster::LatenzFenster(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopMeasureLatency"));
    setWindowTitle(tr("Measure latency"));
    auto* aussen = new QVBoxLayout(this);
    auto* anleitung = new QLabel(tr("Hold the microphone near a speaker (or connect the output to the input), then "
                                    "Measure. Five clicks are played and found again in the recording."),
                                 this);
    anleitung->setWordWrap(true);
    aussen->addWidget(anleitung);
    m_text = new QLabel(this);
    m_text->setObjectName(QStringLiteral("latencyResult"));
    m_text->setWordWrap(true);
    aussen->addWidget(m_text);
    m_messen = new QPushButton(tr("Measure"), this);
    m_messen->setObjectName(QStringLiteral("latencyMeasure"));
    m_nehmen = new QPushButton(tr("Use"), this);
    m_nehmen->setObjectName(QStringLiteral("latencyUse"));
    m_nehmen->setToolTip(tr("Remember this latency for this input and output, and use it"));
    auto* abbrechen = new QPushButton(tr("Cancel"), this);
    auto* fuss = new QHBoxLayout;
    fuss->addWidget(m_messen);
    fuss->addStretch(1);
    fuss->addWidget(m_nehmen);
    fuss->addWidget(abbrechen);
    aussen->addLayout(fuss);
    m_takt = new QTimer(this);
    m_takt->setInterval(kTaktMs);
    connect(m_takt, &QTimer::timeout, this, &LatenzFenster::takt);
    connect(m_messen, &QPushButton::clicked, this, [this] { (void)starteMessung(); });
    connect(m_nehmen, &QPushButton::clicked, this, [this] { (void)nimm(); });
    connect(abbrechen, &QPushButton::clicked, this, &QWidget::close);
    resize(420, 160);
    zeige();
}

LatenzFenster::~LatenzFenster()
{
    if (misst()) beende();
}

void LatenzFenster::setze(IAudioEngine* engine, int eingang, const QString& eingangName, const QString& ausgang)
{
    if (misst()) beende();
    m_engine = engine;
    m_eingang = eingang;
    m_eingangName = eingangName;
    m_ausgang = ausgang;
    m_ergebnis = {};
    m_meldung.clear();
    zeige();
}

bool LatenzFenster::starteMessung()
{
    if (m_engine == nullptr || misst()) return false;
    m_ergebnis = {};
    m_meldung.clear();
    const int rate = m_engine->getSampleRate();
    auto geber = std::make_unique<KlickGeber>(rate);
    const AudioStreamHandle mixer = m_engine->createMixer();
    if (mixer == INVALID_STREAM)
    {
        m_meldung = tr("Could not open the output.");
        zeige();
        return false;
    }
    const AudioStreamHandle quelle = m_engine->mixerAddGenerator(mixer, geber.get());
    if (quelle == INVALID_STREAM)
    {
        m_engine->freeStream(mixer);
        m_meldung = tr("Could not open the output.");
        zeige();
        return false;
    }
    // erst der Eingang, dann die Klicks: die Uhr ist die Stelle des Klickgebers
    KlickGeber* uhr = geber.get();
    m_engine->setRecordClock([uhr] { return uhr->uhr(); });
    const double sekunden = static_cast<double>(messungFrames(rate)) / rate + 1.0;
    if (!m_engine->recordStart(m_eingang, rate, sekunden))
    {
        m_engine->setRecordClock({});
        m_engine->mixerRemoveSource(quelle);
        m_engine->freeStream(mixer);
        m_meldung = tr("Could not open this input.");
        zeige();
        return false;
    }
    m_geber = std::move(geber);
    m_mixer = mixer;
    m_quelle = quelle;
    m_engine->setSourcePaused(m_quelle, false);
    m_engine->play(m_mixer);
    m_takt->start();
    zeige();
    return true;
}

void LatenzFenster::takt()
{
    if (!misst()) return;
    if (m_geber->fertig()) beende();
}

void LatenzFenster::beende()
{
    m_takt->stop();
    std::vector<float> aufnahme;
    std::vector<IAudioEngine::RecordClockMark> marken;
    int rate = m_geber != nullptr ? m_geber->rate() : 0;
    if (m_engine != nullptr)
    {
        aufnahme = m_engine->recordStop();
        marken = m_engine->recordClockMarks();
        m_engine->setRecordClock({});
        m_engine->mixerRemoveSource(m_quelle);
        m_engine->freeStream(m_mixer);
    }
    m_mixer = INVALID_STREAM;
    m_quelle = INVALID_STREAM;
    m_geber.reset();
    const std::optional<std::int64_t> versatz = uhrVersatz(marken);
    if (!versatz || aufnahme.empty())
        m_meldung = tr("Nothing arrived from this input.");
    else
        m_ergebnis = messeLatenz(aufnahme, *versatz, klickStellen(rate), rate);
    zeige();
}

QString LatenzFenster::ergebnisText() const
{
    if (!m_meldung.isEmpty()) return m_meldung;
    if (misst()) return tr("Measuring…");
    if (m_ergebnis.ms)
        return tr("Result: %1 ms (%2 of %3 clicks found, spread %4 ms)")
            .arg(*m_ergebnis.ms, 0, 'f', 0)
            .arg(m_ergebnis.gefunden)
            .arg(m_ergebnis.erwartet)
            .arg(m_ergebnis.streuungMs, 0, 'f', 0);
    if (m_ergebnis.erwartet > 0)
        return tr("Only %1 of %2 clicks found — hold the microphone closer, or turn the output up.")
            .arg(m_ergebnis.gefunden)
            .arg(m_ergebnis.erwartet);
    if (m_engine == nullptr) return tr("No audio engine — measuring is not available");
    return tr("Input: %1 · Output: %2").arg(m_eingangName, m_ausgang);
}

bool LatenzFenster::nimm()
{
    if (!m_ergebnis.ms) return false;
    merkeLatenzMs(m_eingangName, m_ausgang, *m_ergebnis.ms);
    Q_EMIT latenzGewaehlt(*m_ergebnis.ms);
    close();
    return true;
}

void LatenzFenster::zeige()
{
    m_text->setText(ergebnisText());
    m_messen->setEnabled(m_engine != nullptr && !misst());
    m_nehmen->setEnabled(m_ergebnis.ms.has_value() && !misst());
}

}  // namespace lumi::loop
