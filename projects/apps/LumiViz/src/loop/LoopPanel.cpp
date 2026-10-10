/**
 * @file LoopPanel.cpp
 * @brief Loop: das Panel — in Stufe 5b nur Laden und Abspielen
 */

#include "loop/LoopPanel.hpp"

#include "audio/IAudioEngine.hpp"
#include "audio/IAudioPlayer.hpp"
#include "composer/AppAnbindung.hpp"
#include "loop/Kit.hpp"
#include "services/ServiceContainer.hpp"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

#include <cmath>

namespace lumi::loop
{

namespace
{

const char* const kLetzterOrdner = "loop/lastDir";
const char* const kLetzteDatei = "loop/lastFile";
constexpr int kTaktMs = 33;

void setzeText(QLabel* label, const QString& text)
{
    if (label->text() != text) label->setText(text);
}

}  // namespace

LoopPanel::LoopPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "loop", tr("Loop"), parent),
      // Rangfolge (Entscheid Patrik S108, L4): der Composer steht ueber dem Loop
      m_spieler([] { return lumi::composer::fuehrtTon(); })
{
    setupUI();
    // der zuletzt geladene Loop steht wieder im Panel (gespielt wird nicht von selbst)
    const QString letzte = QSettings().value(QLatin1String(kLetzteDatei)).toString();
    if (!letzte.isEmpty() && QFileInfo::exists(letzte)) lade(letzte);
    zeigeDatei();
    zeigeStand();

    m_takt = new QTimer(this);
    m_takt->setInterval(kTaktMs);
    connect(m_takt, &QTimer::timeout, this, &LoopPanel::beimTakt);
    m_takt->start();
}

LoopPanel::~LoopPanel()
{
    // die Audio-Engine lebt laenger als die Panels: hier noch freigeben
    m_spieler.halte(false);
}

void LoopPanel::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    auto* dateiReihe = new QHBoxLayout;
    m_dateiZeile = new QLabel(this);
    m_dateiZeile->setObjectName(QStringLiteral("loopFile"));
    m_dateiZeile->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_laden = new QPushButton(tr("Load..."), this);
    m_laden->setObjectName(QStringLiteral("loopLoad"));
    m_laden->setToolTip(tr("Load a loop file (.lvloop)"));
    dateiReihe->addWidget(m_dateiZeile, 1);
    dateiReihe->addWidget(m_laden);
    aussen->addLayout(dateiReihe);

    auto* spielReihe = new QHBoxLayout;
    m_play = new QPushButton(tr("Play"), this);
    m_play->setObjectName(QStringLiteral("loopPlay"));
    m_info = new QLabel(this);
    m_info->setObjectName(QStringLiteral("loopInfo"));
    m_stelle = new QLabel(this);
    m_stelle->setObjectName(QStringLiteral("loopPosition"));
    spielReihe->addWidget(m_play);
    spielReihe->addWidget(m_info, 1);
    spielReihe->addWidget(m_stelle);
    aussen->addLayout(spielReihe);

    m_hinweis = new QLabel(tr("The Composer plays the audio — start it in the Composer panel"), this);
    m_hinweis->setObjectName(QStringLiteral("loopHint"));
    m_hinweis->setWordWrap(true);
    m_hinweis->hide();
    aussen->addWidget(m_hinweis);

    m_bericht = new QLabel(this);
    m_bericht->setObjectName(QStringLiteral("loopReport"));
    m_bericht->setWordWrap(true);
    m_bericht->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_bericht->hide();
    aussen->addWidget(m_bericht);
    aussen->addStretch(1);

    connect(m_laden, &QPushButton::clicked, this, &LoopPanel::frageNachDatei);
    connect(m_play, &QPushButton::clicked, this, [this] {
        if (m_spieler.spielt())
            halte();
        else
            spiele();
    });
}

void LoopPanel::frageNachDatei()
{
    QSettings einstellungen;
    QString ordner = einstellungen.value(QLatin1String(kLetzterOrdner)).toString();
    // beim ersten Mal: der Ordner mit dem mitgelieferten Beispiel
    if (ordner.isEmpty() && !kitOrdner().isEmpty()) ordner = QDir::cleanPath(kitOrdner() + QStringLiteral("/.."));
    const QString pfad =
        QFileDialog::getOpenFileName(this, tr("Load loop"), ordner, tr("Loop (*.lvloop);;All files (*)"));
    if (pfad.isEmpty()) return;
    einstellungen.setValue(QLatin1String(kLetzterOrdner), QFileInfo(pfad).absolutePath());
    if (!lade(pfad))
    {
        // was geladen war, bleibt; die Meldung steht, bis wieder etwas geladen wird
        m_ladeBericht = {tr("Not a loop file: %1").arg(QDir::toNativeSeparators(pfad))};
        zeigeStand();
    }
}

bool LoopPanel::lade(const QString& pfad)
{
    Loop loop;
    QStringList bericht;
    if (!ladeLoop(pfad, loop, &bericht)) return false;
    m_datei = pfad;
    m_geladen = true;
    m_ladeBericht = bericht;
    m_spieler.setzeLoop(loop);
    QSettings().setValue(QLatin1String(kLetzteDatei), pfad);
    zeigeDatei();
    zeigeStand();
    return true;
}

bool LoopPanel::spiele()
{
    if (!m_geladen) return false;
    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    IAudioPlayer* player = services().tryResolve<IAudioPlayer>();
    if (engine == nullptr || player == nullptr) return false;
    const bool spielt = m_spieler.starte(*engine, *player);
    zeigeStand();
    return spielt;
}

void LoopPanel::halte()
{
    m_spieler.halte(true);
    zeigeStand();
}

void LoopPanel::beimTakt()
{
    m_spieler.takt();
    if (isVisible()) zeigeStand();
}

void LoopPanel::zeigeDatei()
{
    if (!m_geladen)
    {
        m_dateiZeile->setText(tr("No loop loaded"));
        m_dateiZeile->setToolTip({});
        m_info->clear();
        return;
    }
    m_dateiZeile->setText(QFileInfo(m_datei).fileName());
    m_dateiZeile->setToolTip(QDir::toNativeSeparators(m_datei));
    const Loop& loop = m_spieler.loop();
    m_info->setText(tr("%1 BPM · channels: %2 · bars: %3 · %4 s")
                        .arg(loop.bpm, 0, 'f', loop.bpm == std::floor(loop.bpm) ? 0 : 1)
                        .arg(loop.kanaele.size())
                        .arg(loop.durchlaufTakte())
                        .arg(loop.durchlaufMs() / 1000.0, 0, 'f', 1));
}

void LoopPanel::zeigeStand()
{
    const bool spielt = m_spieler.spielt();
    const bool darf = m_spieler.darfSpielen();
    setzeText(m_stelle, [&]() -> QString {
        if (!spielt) return {};
        const Loop& loop = m_spieler.loop();
        const int schritt = static_cast<int>(m_spieler.stelleSchritte());
        return tr("Bar %1 · Step %2").arg(schritt / loop.schritteJeTakt() + 1).arg(schritt % loop.schritteJeTakt() + 1);
    }());
    const QString knopf = spielt ? tr("Stop") : tr("Play");
    if (m_play->text() != knopf) m_play->setText(knopf);
    m_play->setEnabled(m_geladen && darf);
    m_play->setToolTip(darf ? tr("Play the loop from the start; the player pauses meanwhile")
                            : tr("The Composer plays the audio — start it in the Composer panel"));
    m_hinweis->setVisible(!darf);

    QStringList zeilen = m_ladeBericht;
    zeilen << m_spieler.bericht();
    const QString text = zeilen.join(QLatin1Char('\n'));
    setzeText(m_bericht, text);
    m_bericht->setVisible(!text.isEmpty());
}

}  // namespace lumi::loop
