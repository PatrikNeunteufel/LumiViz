/**
 * @file ExportAuftrag.cpp
 * @brief Loop: *Export audio…* ohne Oberflaeche (Stufe 30c)
 */

#include "loop/ExportAuftrag.hpp"

#include "loop/Instrument.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>

#include <cmath>
#include <utility>

namespace lumi::loop
{

namespace
{

QString text(const char* quelle)
{
    return QCoreApplication::translate("lumi::loop::ExportAuftrag", quelle);
}

/// Was in Dateinamen nicht geht, wird zu `_`.
QString alsDateiname(QString name)
{
    static const QRegularExpression verboten(QStringLiteral(R"([\\/:*?"<>|\x00-\x1f])"));
    name.replace(verboten, QStringLiteral("_"));
    name = name.trimmed();
    return name.isEmpty() ? QStringLiteral("loop") : name;
}

}  // namespace

ExportAuftrag::ExportAuftrag(ExportWunsch wunsch, Dekodierer dekodierer)
    : m_wunsch(std::move(wunsch)), m_dekodierer(std::move(dekodierer))
{
    m_thread = std::thread([this] { arbeite(); });
}

ExportAuftrag::~ExportAuftrag()
{
    brichAb();
    warte();
}

void ExportAuftrag::brichAb()
{
    m_abbruch.store(true);
    const std::lock_guard<std::mutex> halt(m_sperre);
    if (m_toene != nullptr) m_toene->brichAb();
}

void ExportAuftrag::warte()
{
    if (m_thread.joinable()) m_thread.join();
}

ExportStand ExportAuftrag::stand() const
{
    ExportStand stand;
    stand.phase = static_cast<ExportPhase>(m_phase.load());
    stand.toeneGesamt = m_toeneGesamt.load();
    {
        const std::lock_guard<std::mutex> halt(m_sperre);
        stand.toeneErledigt = m_toene != nullptr ? m_toene->erledigt() : m_toeneErledigt.load();
    }
    stand.anteil = m_anteil.load();
    return stand;
}

bool ExportAuftrag::beendet() const
{
    const auto phase = static_cast<ExportPhase>(m_phase.load());
    return phase == ExportPhase::Fertig || phase == ExportPhase::Abgebrochen || phase == ExportPhase::Fehler;
}

std::optional<ExportErgebnis> ExportAuftrag::ergebnis() const
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    return m_ergebnis;
}

void ExportAuftrag::endet(ExportPhase phase, ExportErgebnis ergebnis)
{
    {
        const std::lock_guard<std::mutex> halt(m_sperre);
        m_ergebnis = std::move(ergebnis);
    }
    m_phase.store(static_cast<int>(phase));  // zuletzt: wer `beendet` sieht, findet das Ergebnis
}

void ExportAuftrag::arbeite()
{
    ExportErgebnis ergebnis;
    ergebnis.rate = m_wunsch.rate;
    // die Auskunft ueber die Saiten lebt auf diesem Thread — nur er fragt sie
    const SaitenVon saiten = saitenAusInstrumenten();
    QStringList namen;
    switch (m_wunsch.art)
    {
    case ExportArt::Clip:
        if (const std::optional<Loop> fassung = clipFassung(m_wunsch.loop, m_wunsch.clip)) namen = fassung->klangNamen(saiten);
        else
        {
            ergebnis.fehler = text("The clip is not in the loop any more.");
            endet(ExportPhase::Fehler, std::move(ergebnis));
            return;
        }
        break;
    case ExportArt::Loop:
        namen = loopKlangNamen(m_wunsch.loop, m_wunsch.ende, m_wunsch.durchlaeufe, saiten);
        break;
    case ExportArt::Ausschnitt:
        namen = m_wunsch.loop.klangNamen(saiten);
        break;
    }

    // --- die Toene: aus dem Speicher, sonst auf mehreren Kernen
    std::shared_ptr<const Klaenge> klaenge;
    {
        Tonauftrag toene(namen, m_wunsch.rate, m_dekodierer);
        m_toeneGesamt.store(toene.gesamt());
        if (!toene.ausSpeicher())
        {
            {
                const std::lock_guard<std::mutex> halt(m_sperre);
                m_toene = &toene;
            }
            if (!m_abbruch.load()) toene.starte();
            toene.warte();
            const std::lock_guard<std::mutex> halt(m_sperre);
            m_toeneErledigt.store(toene.erledigt());
            m_toene = nullptr;
        }
        else
            m_toeneErledigt.store(toene.gesamt());
        if (m_abbruch.load())
        {
            endet(ExportPhase::Abgebrochen, std::move(ergebnis));
            return;
        }
        klaenge = std::make_shared<const Klaenge>(toene.klaenge());
        ergebnis.bericht = toene.bericht();
    }

    // --- die Wellenform
    m_phase.store(static_cast<int>(ExportPhase::Rechnen));
    const Fortschritt fortschritt = [this](double anteil) {
        m_anteil.store(anteil);
        return !m_abbruch.load();
    };
    std::optional<Wellenform> welle;
    switch (m_wunsch.art)
    {
    case ExportArt::Clip:
        welle = rechneClip(m_wunsch.loop, m_wunsch.clip, m_wunsch.rate, klaenge, saiten, fortschritt);
        break;
    case ExportArt::Loop:
        welle = rechneLoop(m_wunsch.loop, m_wunsch.ende, m_wunsch.durchlaeufe, m_wunsch.rate, klaenge, saiten, fortschritt);
        break;
    case ExportArt::Ausschnitt:
        welle = rechneAusschnitt(m_wunsch.loop, m_wunsch.versatzMs, m_wunsch.laengeMs, m_wunsch.rate, klaenge, saiten,
                                 fortschritt);
        break;
    }
    if (m_abbruch.load())
    {
        endet(ExportPhase::Abgebrochen, std::move(ergebnis));
        return;
    }
    if (!welle || welle->klang.frames() == 0)
    {
        ergebnis.fehler = text("There is nothing to export.");
        endet(ExportPhase::Fehler, std::move(ergebnis));
        return;
    }

    // --- die Datei: 16 Bit mit Dither (WX2 a)
    m_phase.store(static_cast<int>(ExportPhase::Schreiben));
    ergebnis.frames = welle->klang.frames();
    ergebnis.clipFrames = welle->clipFrames;
    ergebnis.nachklangGekappt = welle->nachklangGekappt;
    ergebnis.wav = audio::schreibeWav(m_wunsch.datei, welle->klang.daten, m_wunsch.rate, m_wunsch.tiefe,
                                      m_wunsch.tiefe == audio::WavTiefe::Pcm16);
    if (!ergebnis.wav.geschrieben)
    {
        ergebnis.fehler = text("The file could not be written.");
        endet(ExportPhase::Fehler, std::move(ergebnis));
        return;
    }
    ergebnis.bytes = QFileInfo(m_wunsch.datei).size();
    endet(ExportPhase::Fertig, std::move(ergebnis));
}

QString exportName(const QString& loopName, const ExportWunsch& wunsch)
{
    const QString loop = alsDateiname(loopName);
    switch (wunsch.art)
    {
    case ExportArt::Clip:
    {
        const int nr = findeClip(wunsch.loop, wunsch.clip);
        const QString basis =
            nr >= 0 ? wunsch.loop.kanaele[static_cast<std::size_t>(wunsch.clip.kanal)].clips[static_cast<std::size_t>(nr)].muster
                    : QString();
        return alsDateiname(QStringLiteral("%1_%2_bar%3").arg(loop, basis).arg(wunsch.clip.startTakt + 1)) +
               QStringLiteral(".wav");
    }
    case ExportArt::Loop:
        return loop + QStringLiteral(".wav");
    case ExportArt::Ausschnitt:
    {
        const auto sekunden = static_cast<qint64>(std::llround(wunsch.versatzMs / 1000.0));
        return QStringLiteral("%1_%2-%3.wav").arg(loop).arg(sekunden / 60).arg(sekunden % 60, 2, 10, QLatin1Char('0'));
    }
    }
    return loop + QStringLiteral(".wav");
}

QString tiefeText(audio::WavTiefe tiefe)
{
    switch (tiefe)
    {
    case audio::WavTiefe::Gleitkomma32: return text("32-bit float");
    case audio::WavTiefe::Pcm24: return text("24-bit");
    case audio::WavTiefe::Pcm16: return text("16-bit");
    }
    return {};
}

}  // namespace lumi::loop
