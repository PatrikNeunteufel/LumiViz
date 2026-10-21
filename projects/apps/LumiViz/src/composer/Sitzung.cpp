/**
 * @file Sitzung.cpp
 * @brief Composer: die geladene Komposition eines Fensters und ihr Transport (Stufe C2b)
 */

#include "composer/Sitzung.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "audio/AudioEvents.hpp"
#include "audio/IAudioEngine.hpp"
#include "audio/IAudioPlayer.hpp"
#include "composer/AppAnbindung.hpp"  // AusgeschalteterClipEvent (S130)
#include "composer/Bearbeiten.hpp"
#include "composer/Bildquelle.hpp"
#include "loop/Instrument.hpp"  // saitenAusInstrumenten, instrumentFassung (Stufe 29e)
#include "loop/Loop.hpp"
#include "loop/Offen.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSettings>
#include <QTimer>

#include <BasicLogger.h>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>

namespace lumi::composer
{

namespace
{
const QLatin1String kResume("composer/resume");
const QLatin1String kSchleife("composer/loop");
const QLatin1String kOeffnenHaelt("composer/openHolds");
const QLatin1String kAmEnde("composer/atEnd");
const QLatin1String kPlayerWeiter("composer/playerResumes");
const QLatin1String kNachEinWeiter("composer/resumesWhenSwitchedOn");
const QLatin1String kNeuerClip("composer/newClipSeconds");
const QLatin1String kMitlaufenAn("composer/scrollWithPlayheadOn");
const QLatin1String kMitlaufenArt("composer/scrollWithPlayhead");
const QLatin1String kSync("composer/syncEffectChain");
const QLatin1String kSichttestSpielt("composer/sichttestPlaysAfterLoad");
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
        gibTonFrei(false);  // solange Player und Engine noch leben: die Playlist haengt wieder
        loeseKette();  // die Panels nicht an einem Clip haengen lassen, wenn alles abgebaut wird
        protokolliereMessung();
    });
    m_sync = QSettings().value(kSync, false).toBool();
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
    // C5: die Laenge der Songs kennt die Audio-Engine — ein Ton-Clip reicht nie
    // ueber seinen Song hinaus, einer aus einer alten Datei bis zu dessen Schluss
    SongLaenge laenge;
    if (IAudioEngine* engine = m_widget.dienste().tryResolve<IAudioEngine>())
        laenge = [engine](const QString& datei) {
            return static_cast<std::int64_t>(engine->probeDurationMs(datei));
        };
    if (!ladeKomposition(pfad, neu, bericht, laenge)) return false;
    neu.richte();  // was das Panel zeigt, ist was laeuft
    // S100: eine Datei ohne Kanalfarben bekommt sie hier, nach der Reihenfolge
    // der Kanaele — sie gilt damit nicht als geaendert, die Farben stehen beim
    // naechsten Speichern in der Datei
    neu.vergibFarben();

    setAn(false);
    m_komposition = std::move(neu);
    m_gespeichert = m_komposition;
    m_datei = QFileInfo(pfad).absoluteFilePath();
    m_geladen = true;
    ++m_ladung;
    m_stelleMs = 0;
    m_liefVorAus = false;  // eine andere Komposition laeuft nicht „weiter"
    bereiteToeneVor();     // H1 a: die Toene der Loop-Clips schon jetzt, im Hintergrund
    Q_EMIT kompositionGeaendert();
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

void Sitzung::neu()
{
    setAn(false);
    m_komposition = neueKomposition();
    m_gespeichert = m_komposition;
    m_datei.clear();
    m_geladen = true;
    ++m_ladung;
    m_stelleMs = 0;
    m_liefVorAus = false;  // eine andere Komposition laeuft nicht „weiter"
    BasicLogger::logInfo("Composer: new composition");
    Q_EMIT kompositionGeaendert();
    Q_EMIT geaendert();
}

void Sitzung::entlade()
{
    if (!m_geladen) return;
    setAn(false);
    m_komposition = Komposition{};
    m_gespeichert = Komposition{};
    m_datei.clear();
    m_geladen = false;
    ++m_ladung;
    m_stelleMs = 0;
    m_liefVorAus = false;  // eine andere Komposition laeuft nicht „weiter"
    BasicLogger::logInfo("Composer: composition closed");
    Q_EMIT kompositionGeaendert();
    Q_EMIT geaendert();
}

// =============================================================================
// Bearbeiten und Speichern
// =============================================================================

void Sitzung::setzeKomposition(Komposition neu)
{
    if (!m_geladen) return;
    neu.richte();
    neu.vergibFarben();  // in der Sitzung hat jeder Kanal seine Farbe (S100), wie nach dem Laden
    if (gleich(neu, m_komposition)) return;
    m_komposition = std::move(neu);
    if (Bildquelle* q = quelle())
    {
        QMutexLocker halt(&m_widget.renderMutex());
        q->ersetzeKomposition(m_komposition);
        q->spiegleStand();
    }
    m_stelleMs = std::clamp<std::int64_t>(m_stelleMs, 0, laengeMs());
    bereiteToeneVor();  // ein neuer Loop-Clip bringt neue Toene (Stufe 29e)
    Q_EMIT kompositionBearbeitet();
    Q_EMIT geaendert();
}

bool Sitzung::ungespeichert() const
{
    return m_geladen && !gleich(m_komposition, m_gespeichert);
}

bool Sitzung::speichere()
{
    return m_geladen && speichereNach(m_datei);
}

bool Sitzung::speichereNach(const QString& pfad)
{
    if (!m_geladen || pfad.isEmpty()) return false;
    if (!speichereKomposition(m_komposition, pfad)) return false;
    m_gespeichert = m_komposition;
    m_datei = QFileInfo(pfad).absoluteFilePath();
    BasicLogger::logInfo("Composer: composition saved: " +
                         QDir::toNativeSeparators(m_datei).toStdString());
    Q_EMIT geaendert();
    return true;
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
        // Einstellung (Wunsch Patrik S98): lief die Komposition, als der Composer
        // ausgeschaltet wurde, laeuft sie beim Einschalten weiter
        if (m_liefVorAus && laeuftNachEinWeiter())
        {
            // H2 a: erst, wenn die Toene der Loop-Clips da sind — sonst wartet sie (Stufe 29e)
            bereiteToeneVor();
            if (toeneDa())
                neu->uhr().starte();
            else
                m_wartetAufToene = true;
        }
        m_liefVorAus = false;
        neu->spiegleStand();
        {
            // die Karaoke-Ebene der App schon fuer das erste Bild (S101)
            QMutexLocker halt(&m_widget.renderMutex());
            m_karaokeVon = nullptr;
            gleicheKaraokeAb(*neu);
        }
        m_widget.setBildquelle(std::move(neu));
        if (!this->an()) return;  // das Fenster hat keinen Render-Thread
        m_takt->start();
        m_festStand = -1;
        bindeKette();  // Effect Chain: an den angeklickten Clip oder zur Ruhe
        BasicLogger::logInfo("Composer: on");
    }
    else
    {
        m_liefVorAus = laeuft() || m_wartetAufToene;
        m_wartetAufToene = false;
        loeseKette();  // Effect Chain zurueck an die Kette, BEVOR die Renderer der Clips gehen
        gibTonFrei();  // der Player gehoert wieder der Playlist
        protokolliereMessung();
        {
            QMutexLocker halt(&m_widget.renderMutex());
            m_stelleMs = quelle()->uhr().rueckkehrMs();  // im Fenster der Schleife die hintere Stelle
        }
        m_takt->stop();
        m_widget.setBildquelle(nullptr);
        BasicLogger::logInfo("Composer: off");
    }
    Q_EMIT geaendert();
}

// =============================================================================
// Sync on Effect Chain (C3c)
// =============================================================================

namespace
{
IEventBus* g_bus = nullptr;
}  // namespace

void Sitzung::setzeBus(IEventBus* bus)
{
    g_bus = bus;
}

void Sitzung::setSync(bool an)
{
    if (an == m_sync) return;
    m_sync = an;
    QSettings().setValue(kSync, an);
    bindeKette();
    Q_EMIT geaendert();
}

void Sitzung::waehleClip(int kanal, int clip)
{
    if (kanal == m_wahlKanal && clip == m_wahlClip) return;
    m_wahlKanal = kanal;
    m_wahlClip = clip;
    // Nicht sofort: mitten in einem Bearbeitungsschritt nennt die Auswahl den
    // Clip schon unter seiner neuen Nummer, die Bildquelle kennt die neue
    // Fassung aber noch nicht. Der naechste Takt sieht beides zusammen.
    m_bindungOffen = true;
}

namespace
{
int g_meldet = 0;  ///< > 0: eine Sitzung verschickt gerade ihre Bindung (Naht N16)

/// Fuer die Dauer einer Meldung der Bindung — der Bus stellt sofort zu.
struct MeldetBindung
{
    MeldetBindung() { ++g_meldet; }
    ~MeldetBindung() { --g_meldet; }
    MeldetBindung(const MeldetBindung&) = delete;
    MeldetBindung& operator=(const MeldetBindung&) = delete;
};
}  // namespace

bool Sitzung::meldetBindung()
{
    return g_meldet > 0;
}

void Sitzung::meldeKette()
{
    if (g_bus == nullptr) return;
    IVisualizer* kette = m_widget.visualizer();
    const MeldetBindung meldung;
    g_bus->publish(VisualizerChangedEvent{
        kette != nullptr ? kette->visualizerId().toStdString() : std::string(),
        kette != nullptr ? kette->visualizerName().toStdString() : std::string(),
        static_cast<void*>(kette), &m_widget.renderMutex()});
}

void Sitzung::meldeNichts()
{
    if (g_bus == nullptr) return;
    const MeldetBindung meldung;
    g_bus->publish(VisualizerChangedEvent{std::string(), std::string(), nullptr,
                                          &m_widget.renderMutex()});
}

void Sitzung::gleicheKaraokeAb(Bildquelle& q)
{
    // Die Ebene gehoert dem Renderer der App: an ihm haengt das Panel Karaoke,
    // auch waehrend eine Komposition das Bild liefert. Ist es kein Multi
    // Effect, gibt es keine Ebene zu reichen — der Traeger behaelt, was er hat.
    const auto* app = dynamic_cast<const MultiEffectVisualizer*>(m_widget.visualizer());
    if (app == nullptr)
    {
        m_karaokeVon = nullptr;
        return;
    }
    if (app == m_karaokeVon && app->karaokeFassung() == m_karaokeFassung) return;
    m_karaokeVon = app;
    m_karaokeFassung = app->karaokeFassung();
    q.setzeKaraokeEbene(app->karaokeKette());
}

void Sitzung::loeseKette()
{
    if (m_bindung == Bindung::Kette) return;
    meldeKette();
    m_bindung = Bindung::Kette;
    m_gebunden = nullptr;
    m_bindungOffen = false;
}

void Sitzung::bindeKette()
{
    if (g_bus == nullptr) return;
    Bildquelle* q = quelle();
    if (q == nullptr)  // Composer aus: die Kette der App
    {
        loeseKette();
        return;
    }

    if (!m_sync || m_wahlKanal < 0)  // ruht
    {
        if (m_bindung != Bindung::Ruht)
        {
            // Erst an die Kette: Visualizer Config uebergeht „nichts" und
            // hinge sonst weiter an einem Clip. Dann Effect Chain an nichts.
            meldeKette();
            meldeNichts();
        }
        {
            QMutexLocker halt(&m_widget.renderMutex());
            q->haltFest(-1, -1);
            q->gibFrei();
        }
        m_bindung = Bindung::Ruht;
        m_gebunden = nullptr;
        m_bindungOffen = false;
        return;
    }

    // Der angeklickte Clip: festhalten, und sobald der Render-Thread so weit
    // ist, an seinen Renderer.
    const MultiEffectVisualizer* soll = nullptr;
    {
        QMutexLocker halt(&m_widget.renderMutex());
        q->haltFest(m_wahlKanal, m_wahlClip);
        if (!q->festAngewandt())
        {
            m_bindungOffen = true;  // bis dahin bleibt, was ist — es ist geschuetzt
            return;
        }
        soll = q->festerRendererNurSehen();
    }
    m_bindungOffen = soll == nullptr;  // (noch) kein Renderer: der Takt versucht es wieder
    if (m_bindung == Bindung::Clip && soll == m_gebunden) return;

    // Weg vom bisherigen, bevor er freigegeben wird.
    meldeKette();
    if (soll == nullptr)
    {
        meldeNichts();
        QMutexLocker halt(&m_widget.renderMutex());
        q->gibFrei();
        m_bindung = Bindung::Ruht;
        m_gebunden = nullptr;
        return;
    }
    MultiEffectVisualizer* renderer = nullptr;
    {
        QMutexLocker halt(&m_widget.renderMutex());
        renderer = q->festerRenderer();  // ab hier geschuetzt
    }
    if (renderer == nullptr)
    {
        meldeNichts();
        m_bindung = Bindung::Ruht;
        m_gebunden = nullptr;
        m_bindungOffen = true;
        return;
    }
    const MeldetBindung meldung;
    g_bus->publish(VisualizerChangedEvent{renderer->visualizerId().toStdString(),
                                          renderer->visualizerName().toStdString(),
                                          static_cast<void*>(static_cast<IVisualizer*>(renderer)),
                                          &m_widget.renderMutex()});
    m_bindung = Bindung::Clip;
    m_gebunden = renderer;
}

// -----------------------------------------------------------------------------
// Die Kette eines Clips speichern (C3c Schritt 2)
// -----------------------------------------------------------------------------

QString Sitzung::gebundeneDatei() const
{
    // Ist die Bindung gerade im Umbruch (eine neue Auswahl, noch nicht
    // verarbeitet), nennt die Auswahl womoeglich schon einen anderen Clip.
    if (m_bindung != Bindung::Clip || m_bindungOffen) return {};
    if (m_wahlKanal < 0 || m_wahlKanal >= static_cast<int>(m_komposition.kanaele.size())) return {};
    const auto& clips = m_komposition.kanaele[static_cast<std::size_t>(m_wahlKanal)].clips;
    if (m_wahlClip < 0 || m_wahlClip >= static_cast<int>(clips.size())) return {};
    return clips[static_cast<std::size_t>(m_wahlClip)].datei;
}

bool Sitzung::gebundeneKetteHatHostGruppe() const
{
    if (m_bindung != Bindung::Clip || m_gebunden == nullptr) return false;
    QMutexLocker halt(&m_widget.renderMutex());
    return lumi::multieffect::chainHasHostGroup(m_gebunden->chain());
}

std::vector<std::pair<int, int>> Sitzung::andereClipsMitDerDatei() const
{
    std::vector<std::pair<int, int>> andere;
    const QString datei = gebundeneDatei();
    if (datei.isEmpty()) return andere;
    for (int k = 0; k < static_cast<int>(m_komposition.kanaele.size()); ++k)
    {
        const auto& clips = m_komposition.kanaele[static_cast<std::size_t>(k)].clips;
        for (int c = 0; c < static_cast<int>(clips.size()); ++c)
        {
            if (k == m_wahlKanal && c == m_wahlClip) continue;
            if (QFileInfo(clips[static_cast<std::size_t>(c)].datei) == QFileInfo(datei))
                andere.emplace_back(k, c);
        }
    }
    return andere;
}

bool Sitzung::speichereClipKette(const QString& pfad, const std::vector<std::pair<int, int>>& auchDiese)
{
    const QString alt = gebundeneDatei();
    Bildquelle* q = quelle();
    if (alt.isEmpty() || q == nullptr || m_gebunden == nullptr || pfad.isEmpty()) return false;
    // nur Speichern unter: das Original bleibt, wie es ist
    if (QFileInfo(pfad) == QFileInfo(alt)) return false;
    const QString neu = QFileInfo(pfad).absoluteFilePath();
    {
        QMutexLocker halt(&m_widget.renderMutex());
        if (!m_gebunden->saveChainFile(neu)) return false;
        m_gebunden->merkeKettenStand();  // ab jetzt gilt die Kette als gespeichert
        q->festerClipHeisstJetzt(neu);   // sein Renderer gehoert jetzt zur neuen Datei
    }
    Komposition fassung = m_komposition;
    fassung.kanaele[static_cast<std::size_t>(m_wahlKanal)]
        .clips[static_cast<std::size_t>(m_wahlClip)]
        .datei = neu;
    for (const auto& [k, c] : auchDiese)
    {
        if (k < 0 || k >= static_cast<int>(fassung.kanaele.size())) continue;
        auto& clips = fassung.kanaele[static_cast<std::size_t>(k)].clips;
        if (c < 0 || c >= static_cast<int>(clips.size())) continue;
        if (QFileInfo(clips[static_cast<std::size_t>(c)].datei) == QFileInfo(alt))
            clips[static_cast<std::size_t>(c)].datei = neu;
    }
    BasicLogger::logInfo("Composer: clip preset saved as " + QDir::toNativeSeparators(neu).toStdString());
    setzeKomposition(fassung);
    return true;
}

// =============================================================================
// Transport
// =============================================================================

void Sitzung::spiele()
{
    if (!m_geladen) return;
    setAn(true);
    if (quelle() == nullptr) return;
    // H2 a (Stufe 29e): erst, wenn die Toene aller Loop-Clips da sind — bis dahin steht die Uhr
    bereiteToeneVor();
    if (!toeneDa())
    {
        m_wartetAufToene = true;
        Q_EMIT geaendert();
        return;
    }
    m_wartetAufToene = false;
    starteUhr();
}

void Sitzung::starteUhr()
{
    Bildquelle* q = quelle();
    if (q == nullptr) return;
    {
        QMutexLocker halt(&m_widget.renderMutex());
        q->uhr().starte();
        q->spiegleStand();
    }
    Q_EMIT geaendert();
}

void Sitzung::warteAufToene()
{
    if (m_toene != nullptr) m_toene->warte();
    if (m_wartetAufToene && toeneDa())
    {
        m_wartetAufToene = false;
        starteUhr();
    }
}

void Sitzung::verwirfToene()
{
    if (m_toene == nullptr) return;
    m_toene->brichAb();
    m_toeneVerworfen.push_back(std::move(m_toene));
}

bool Sitzung::toeneDa()
{
    if (m_toene == nullptr) return true;
    if (!m_toene->fertig()) return false;
    verwirfToene();  // fertig: die Toene liegen im Speicher, seine Threads laufen aus
    return true;
}

void Sitzung::bereiteToeneVor()
{
    std::erase_if(m_toeneVerworfen, [](const std::unique_ptr<lumi::loop::Tonauftrag>& a) { return a->ruht(); });
    IAudioEngine* engine = m_widget.dienste().tryResolve<IAudioEngine>();
    if (engine == nullptr || !m_geladen) return;  // ohne Engine kein Ton: nichts zu warten
    // die Toene aller Loop-Clips der eingeschalteten Audio-Kanaele, je Loop-Datei einmal gelesen
    const lumi::loop::SaitenVon saiten = lumi::loop::saitenAusInstrumenten();
    QStringList namen;
    QStringList gelesen;
    for (const Kanal& kanal : m_komposition.kanaele)
    {
        if (kanal.art != KanalArt::Audio || !kanal.an) continue;
        for (const Clip& clip : kanal.clips)
        {
            if (!clip.an || !istLoop(clip.datei) || gelesen.contains(clip.datei)) continue;
            gelesen << clip.datei;
            lumi::loop::Loop loop;
            if (!lumi::loop::offenerLoop(clip.datei, loop) && !lumi::loop::ladeLoop(clip.datei, loop)) continue;
            loop.richte();
            namen << loop.klangNamen(saiten);
        }
    }
    const int rate = engine->getSampleRate();
    const std::uint64_t fassung = lumi::loop::instrumentFassung();
    if (m_toeneGeprueft && namen == m_toeneNamen && rate == m_toeneRate && fassung == m_toeneFassung) return;
    verwirfToene();
    m_toeneGeprueft = true;
    m_toeneNamen = namen;
    m_toeneRate = rate;
    m_toeneFassung = fassung;
    if (namen.isEmpty()) return;
    auto auftrag = std::make_unique<lumi::loop::Tonauftrag>(
        namen, rate, [engine](const QString& datei, int r, double maxSekunden) { return engine->decodeStereo(datei, r, maxSekunden); });
    if (auftrag->ausSpeicher()) return;  // alles im Speicher: nichts zu warten
    auftrag->starte();
    m_toene = std::move(auftrag);
}

void Sitzung::pausiere()
{
    m_wartetAufToene = false;
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
    m_wartetAufToene = false;
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

std::int64_t Sitzung::zweiteStelleMs() const
{
    const Bildquelle* q = quelle();
    return q != nullptr ? q->standZweiteStelleMs() : -1;
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
        gleicheKaraokeAb(*q);  // S101: was im Panel Karaoke geschah, gilt auch hier
        m_widget.renderMutex().unlock();
    }
    // Effect Chain (C3c): nachholen, was noch offen ist — und folgen, wenn der
    // festgehaltene Clip einen anderen Renderer hat (oder keinen mehr).
    if (m_bindungOffen || q->festStand() != m_festStand)
    {
        m_festStand = q->festStand();
        bindeKette();
        q = quelle();
        if (q == nullptr) return;
    }
    // H2 a (Stufe 29e): Play wartete auf die Toene der Loop-Clips — jetzt sind sie da
    if (m_wartetAufToene && toeneDa())
    {
        m_wartetAufToene = false;
        starteUhr();
    }
    fuehreTon(*q);  // C5: der Audiokanal fuehrt den Player
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
// Der Audiokanal (C5)
// =============================================================================

namespace
{
Sitzung* g_fuehrende = nullptr;  ///< wer den Player fuehrt — es gibt nur einen Player
}  // namespace

Sitzung::~Sitzung()
{
    if (g_fuehrende == this) g_fuehrende = nullptr;
}

Sitzung* Sitzung::fuehrende()
{
    return g_fuehrende;
}

void Sitzung::fuehreTon(Bildquelle& q)
{
    IAudioPlayer* player = m_widget.dienste().tryResolve<IAudioPlayer>();
    IAudioEngine* engine = m_widget.dienste().tryResolve<IAudioEngine>();
    if (player == nullptr || engine == nullptr || m_komposition.tonKanal() < 0)
    {
        gibTonFrei();  // kein Player, keine Engine oder kein eingeschalteter Audio-Kanal
        return;
    }
    if (!m_ton.fuehrt())
    {
        if (g_fuehrende != nullptr && g_fuehrende != this) return;  // eine andere Sitzung hat ihn
        if (m_tonVersagt) return;
        if (!m_ton.uebernimm(*engine, *player))
        {
            m_tonVersagt = true;
            BasicLogger::logWarning("Composer: the audio engine has no mixer — audio channels stay silent");
            return;
        }
        g_fuehrende = this;
        m_songLief = false;
        m_gemeldetLaeuft = -1;
        m_stelleTakte = 0;
        m_tonStand = {};
        BasicLogger::logInfo("Composer: the audio channels play through the mixer");
        Q_EMIT geaendert();
    }
    // Die Uhr fuehrt: die Stelle aus dem gespiegelten Stand, auf JETZT
    // hochgerechnet — ohne auf den Render-Thread zu warten, und ohne dass ein
    // langes Bild dort die Stelle veralten laesst.
    const bool laeuft = q.standLaeuft();
    // In der Schleife mit Ueberlappung (S99) kommen die zweite Stelle und die
    // Zeit bis zum Umschlag dazu.
    const Bildquelle::Stellen stellen = q.standStellenJetzt();
    Tonfuehrung::Ergebnis ergebnis = m_ton.takt(m_komposition, stellen.stelleMs, laeuft,
                                                Schleifenlage{stellen.zweiteMs, stellen.umschlagInMs});
    IEventBus* bus = m_widget.dienste().tryResolve<IEventBus>();
    // Der Player hat dazwischengefunkt (etwa ein Doppelklick im Audio Browser)
    // und ist wieder angehalten: seine Meldungen gelten nicht — den laufenden
    // Song und „laeuft" noch einmal melden.
    if (ergebnis.playerAngehalten)
    {
        m_gemeldetLaeuft = -1;
        if (ergebnis.hatSong && !ergebnis.songGewechselt && m_tonStand.hatSong)
        {
            ergebnis.songGewechselt = true;
            ergebnis.song = m_tonStand.song;
        }
    }

    // Der ausgeschaltete Clip ist vorbei (S130, C1 c): das zuerst melden — ein
    // Songwechsel im selben Takt kommt danach und gilt.
    if (!ergebnis.hatAus && m_tonStand.hatAus)
    {
        m_tonStand.hatAus = false;
        m_tonStand.aus = {};
        if (bus != nullptr) bus->publish(AusgeschalteterClipEvent{});
    }

    // Der laufende Song geht dieselben Wege wie ein Titel des Players — ueber
    // den Bus: der Songwechsel (Tags, Songtext, Tag View, Audio Browser,
    // Player-Panel), die Stelle im Song und „laeuft" (Player-Panel, Renderer).
    if (ergebnis.songGewechselt)
    {
        m_tonStand.hatSong = true;
        m_tonStand.song = ergebnis.song;
        if (bus != nullptr)
        {
            TrackChangedEvent ereignis;
            ereignis.track = ergebnis.song;
            bus->publish(ereignis);
        }
        BasicLogger::logInfo("Composer: current song: " +
                             QFileInfo(ergebnis.song.filePath).fileName().toStdString());
    }
    // „laeuft" am Bus heisst: die KOMPOSITION laeuft (Entscheid Patrik S98: der
    // Play-Knopf des Player-Panels zeigt, was ein Druck auf ihn tut — auch in
    // einer Luecke).
    if (bus != nullptr && m_gemeldetLaeuft != (laeuft ? 1 : 0))
    {
        PlaybackStateEvent zustand;
        zustand.state = laeuft ? PlaybackState::Playing : PlaybackState::Paused;
        zustand.previousState = m_gemeldetLaeuft == 1 ? PlaybackState::Playing : PlaybackState::Paused;
        m_gemeldetLaeuft = laeuft ? 1 : 0;
        bus->publish(zustand);
        m_songLief = !ergebnis.songLaeuft;  // der Renderer hoert am Bus mit: gleich richtigstellen
    }
    // Die Stelle im Song: an den Renderer in jedem Takt, an den Bus etwa
    // zehnmal je Sekunde — und sofort nach einem Songwechsel oder Sprung. In
    // einer Luecke kommt keine: der Balken des Player-Panels haelt.
    if (ergebnis.hatSong)
    {
        const int stelle = static_cast<int>(ergebnis.songStelleMs);
        const bool sprung = std::abs(stelle - m_tonStand.stelleMs) > 500;
        ++m_stelleTakte;
        if (ergebnis.songGewechselt ||
            (stelle != m_tonStand.stelleMs && (m_stelleTakte >= 3 || sprung)))
        {
            m_stelleTakte = 0;
            m_tonStand.stelleMs = stelle;
            if (bus != nullptr)
            {
                PlaybackPositionEvent ort;
                ort.positionMs = stelle;
                ort.durationMs = ergebnis.songDauerMs;
                ort.progress = ergebnis.songDauerMs > 0
                                   ? static_cast<float>(stelle) / static_cast<float>(ergebnis.songDauerMs)
                                   : 0.0f;
                bus->publish(ort);
            }
        }
        MultiEffectVisualizer::notifyPlayPosition(ergebnis.songStelleMs);
    }
    // Nur ein ausgeschalteter Clip an der Stelle (S130, C1 c): an das
    // Player-Panel allein, mit seiner Stelle etwa zehnmal je Sekunde — nicht an
    // den Renderer und nicht als Songwechsel.
    if (ergebnis.hatAus)
    {
        const bool neu = ergebnis.ausGewechselt || !m_tonStand.hatAus || ergebnis.playerAngehalten;
        if (ergebnis.ausGewechselt)
        {
            m_tonStand.aus = ergebnis.aus;
        }
        m_tonStand.hatAus = true;
        const int stelle = static_cast<int>(ergebnis.ausStelleMs);
        const bool sprung = std::abs(stelle - m_tonStand.ausStelleMs) > 500;
        ++m_ausTakte;
        if (neu || (stelle != m_tonStand.ausStelleMs && (m_ausTakte >= 3 || sprung)))
        {
            m_ausTakte = 0;
            m_tonStand.ausStelleMs = stelle;
            if (bus != nullptr)
            {
                AusgeschalteterClipEvent ereignis;
                ereignis.clip = m_tonStand.aus;
                ereignis.stelleMs = stelle;
                bus->publish(ereignis);
            }
        }
    }
    // Dem Renderer gilt „laeuft" fuer den SONG: in einer Luecke und nach dem
    // Ende eines Songs zaehlen Karaoke und Lyrics nicht weiter.
    if (ergebnis.songLaeuft != m_songLief || ergebnis.songGewechselt)
    {
        m_songLief = ergebnis.songLaeuft;
        MultiEffectVisualizer::notifyPlayState(m_songLief);
    }
}

bool Sitzung::springeImSong(double anteil)
{
    // zeigt das Player-Panel einen ausgeschalteten Clip (S130, C1 c), gilt der Klick ihm
    const std::int64_t ziel = m_tonStand.hatAus ? m_ton.stelleImAus(m_komposition, anteil)
                                                : m_ton.stelleImLaufenden(m_komposition, anteil);
    if (ziel < 0) return false;
    springe(ziel);
    return true;
}

void Sitzung::gibTonFrei(bool melden)
{
    m_tonVersagt = false;
    if (!m_ton.fuehrt()) return;
    const int nachgesetzt = m_ton.nachgesetzt();
    // Einstellung (Wunsch Patrik S98): spielte der Player, als der Composer den
    // Ton uebernahm, spielt er jetzt weiter — nicht beim Beenden der App
    m_ton.gibFrei(melden && playerSpieltWeiter());
    if (g_fuehrende == this) g_fuehrende = nullptr;
    BasicLogger::logInfo("Composer: the player is back with the playlist (audio sources realigned " +
                         std::to_string(nachgesetzt) + " times)");
    // Der Titel des Players gilt wieder als laufender Song — sonst zeigten
    // Karaoke und Tag View weiter den letzten Song der Komposition.
    if (melden)
    {
        IAudioPlayer* player = m_widget.dienste().tryResolve<IAudioPlayer>();
        IEventBus* bus = m_widget.dienste().tryResolve<IEventBus>();
        const bool spielt = player != nullptr && player->isPlaying();
        if (player != nullptr && player->hasTrack())
        {
            if (bus != nullptr)
            {
                TrackChangedEvent ereignis;
                ereignis.track = player->currentTrack();
                ereignis.playlistIndex = player->playlistIndex();
                bus->publish(ereignis);
                PlaybackPositionEvent ort;
                ort.positionMs = player->positionMs();
                ort.durationMs = player->durationMs();
                ort.progress = player->positionFraction();
                bus->publish(ort);
            }
            MultiEffectVisualizer::notifyPlayPosition(player->positionMs());
        }
        // Der Player hat keinen Titel (Entscheid Patrik S99: a): ein leerer
        // Songwechsel — sonst zeigten Player-Panel, Tag View, Karaoke und Audio
        // Browser weiter den letzten Song der Komposition. Nur, wenn ueberhaupt
        // einer gemeldet war.
        else if (bus != nullptr && (m_tonStand.hatSong || m_tonStand.hatAus))
        {
            bus->publish(TrackChangedEvent{});
        }
        // auch ohne Titel im Player: das Player-Panel erfaehrt, dass der
        // Composer den Ton nicht mehr fuehrt (Play-Knopf, Loop-Knopf)
        if (bus != nullptr)
        {
            PlaybackStateEvent zustand;
            zustand.state = spielt ? PlaybackState::Playing : PlaybackState::Paused;
            zustand.previousState = m_gemeldetLaeuft == 1 ? PlaybackState::Playing : PlaybackState::Paused;
            bus->publish(zustand);
        }
        MultiEffectVisualizer::notifyPlayState(spielt);
        m_songLief = false;
        m_gemeldetLaeuft = -1;
        m_tonStand = {};
    }
    Q_EMIT geaendert();
}

bool Sitzung::springeZumSong(bool vor)
{
    const std::int64_t ziel = Tonfuehrung::songAnfang(m_komposition, stelleMs(), vor);
    if (ziel < 0) return false;
    springe(ziel);
    return true;
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

bool Sitzung::playerSpieltWeiter()
{
    return QSettings().value(kPlayerWeiter, true).toBool();
}

void Sitzung::setPlayerSpieltWeiter(bool an)
{
    QSettings().setValue(kPlayerWeiter, an);
}

bool Sitzung::laeuftNachEinWeiter()
{
    return QSettings().value(kNachEinWeiter, true).toBool();
}

void Sitzung::setLaeuftNachEinWeiter(bool an)
{
    QSettings().setValue(kNachEinWeiter, an);
}

std::int64_t Sitzung::neuerClipMs()
{
    const int sekunden = QSettings().value(kNeuerClip, static_cast<int>(kNeuerClipMs / 1000)).toInt();
    return static_cast<std::int64_t>(std::clamp(sekunden, 1, 600)) * 1000;
}

void Sitzung::setNeuerClipSekunden(int sekunden)
{
    QSettings().setValue(kNeuerClip, std::clamp(sekunden, 1, 600));
}

bool Sitzung::mitlaufenAn()
{
    return QSettings().value(kMitlaufenAn, true).toBool();
}

void Sitzung::setMitlaufenAn(bool an)
{
    QSettings().setValue(kMitlaufenAn, an);
}

bool Sitzung::mitlaufenSeitenweise()
{
    return QSettings().value(kMitlaufenArt, 0).toInt() == 1;
}

void Sitzung::setMitlaufenSeitenweise(bool seitenweise)
{
    QSettings().setValue(kMitlaufenArt, seitenweise ? 1 : 0);
}

bool Sitzung::sichttestSpieltNachLaden()
{
    return QSettings().value(kSichttestSpielt, false).toBool();
}

void Sitzung::setSichttestSpieltNachLaden(bool an)
{
    QSettings().setValue(kSichttestSpielt, an);
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
        stelle = q->uhr().rueckkehrMs();  // im Fenster der Schleife die hintere Stelle
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
