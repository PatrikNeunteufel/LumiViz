/**
 * @file FreezeAuftrag.cpp
 * @brief Loop: Clips einfrieren ohne Oberflaeche (Stufe 30g)
 */

#include "loop/FreezeAuftrag.hpp"

#include "loop/Freeze.hpp"
#include "loop/Instrument.hpp"
#include "loop/Wellenform.hpp"

#include <QCoreApplication>
#include <QFileInfo>

#include <utility>

namespace lumi::loop
{

namespace
{

QString text(const char* quelle)
{
    return QCoreApplication::translate("lumi::loop::FreezeAuftrag", quelle);
}

}  // namespace

FreezeAuftrag::FreezeAuftrag(Loop loop, std::vector<ClipOrt> orte, int rate, Dekodierer dekodierer)
    : m_loop(std::move(loop)), m_orte(std::move(orte)), m_rate(rate), m_dekodierer(std::move(dekodierer))
{
    m_thread = std::thread([this] { arbeite(); });
}

FreezeAuftrag::~FreezeAuftrag()
{
    brichAb();
    warte();
}

void FreezeAuftrag::brichAb()
{
    m_abbruch.store(true);
    const std::lock_guard<std::mutex> halt(m_sperre);
    if (m_toene != nullptr) m_toene->brichAb();
}

void FreezeAuftrag::warte()
{
    if (m_thread.joinable()) m_thread.join();
}

FreezeStand FreezeAuftrag::stand() const
{
    FreezeStand stand;
    stand.clip = m_clip.load();
    stand.clips = static_cast<int>(m_orte.size());
    stand.anteil = m_anteil.load();
    {
        const std::lock_guard<std::mutex> halt(m_sperre);
        // beim Laden der Toene: ihr Anteil bis zur Haelfte
        if (m_toene != nullptr && m_toene->gesamt() > 0)
            stand.anteil = 0.5 * static_cast<double>(m_toene->erledigt()) / m_toene->gesamt();
    }
    stand.beendet = beendet();
    return stand;
}

std::optional<FreezeErgebnis> FreezeAuftrag::ergebnis() const
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    return m_ergebnis;
}

void FreezeAuftrag::arbeite()
{
    FreezeErgebnis ergebnis;
    for (std::size_t i = 0; i < m_orte.size(); ++i)
    {
        m_clip.store(static_cast<int>(i));
        m_anteil.store(0.0);
        if (m_abbruch.load() || !friere(m_orte[i], ergebnis))
        {
            ergebnis.abgebrochen = true;
            break;
        }
    }
    {
        const std::lock_guard<std::mutex> halt(m_sperre);
        m_ergebnis = std::move(ergebnis);
    }
    m_beendet.store(true);  // zuletzt: wer `beendet` sieht, findet das Ergebnis
}

bool FreezeAuftrag::friere(const ClipOrt& ort, FreezeErgebnis& ergebnis)
{
    // die Auskunft ueber die Saiten lebt auf diesem Thread — nur er fragt sie
    const SaitenVon saiten = saitenAusInstrumenten();
    const QByteArray abdruck = clipAbdruck(m_loop, ort, m_rate, saiten);
    const std::optional<Loop> fassung = clipFassung(m_loop, ort);
    if (abdruck.isEmpty() || !fassung)
    {
        ergebnis.fehlgeschlagen.push_back(ort);
        ergebnis.bericht << text("Clip not found: bar %1").arg(ort.startTakt + 1);
        return true;
    }
    // schon da (gleiche Clips teilen eine Datei): nichts zu rechnen
    if (QFileInfo(frozenDatei(abdruck)).isFile())
    {
        ergebnis.gelegt.push_back(ort);
        return true;
    }

    // --- die Toene: aus dem Speicher, sonst auf mehreren Kernen
    std::shared_ptr<const Klaenge> klaenge;
    {
        Tonauftrag toene(fassung->klangNamen(saiten), m_rate, m_dekodierer);
        if (!toene.ausSpeicher())
        {
            {
                const std::lock_guard<std::mutex> halt(m_sperre);
                m_toene = &toene;
            }
            if (!m_abbruch.load()) toene.starte();
            toene.warte();
            const std::lock_guard<std::mutex> halt(m_sperre);
            m_toene = nullptr;
        }
        if (m_abbruch.load()) return false;
        klaenge = std::make_shared<const Klaenge>(toene.klaenge());
        for (const QString& zeile : toene.bericht())
            if (!ergebnis.bericht.contains(zeile)) ergebnis.bericht << zeile;
    }

    // --- die Wellenform (F3 a) und die Datei
    m_anteil.store(0.5);
    const Fortschritt fortschritt = [this](double anteil) {
        m_anteil.store(0.5 + 0.5 * anteil);
        return !m_abbruch.load();
    };
    const std::optional<Wellenform> welle = rechneClip(m_loop, ort, m_rate, klaenge, saiten, fortschritt);
    if (m_abbruch.load()) return false;
    if (!welle || welle->klang.frames() == 0 || !legeFrozen(abdruck, welle->klang.daten, m_rate))
    {
        ergebnis.fehlgeschlagen.push_back(ort);
        ergebnis.bericht << text("Clip could not be frozen: bar %1").arg(ort.startTakt + 1);
        return true;
    }
    m_anteil.store(1.0);
    ergebnis.gelegt.push_back(ort);
    return true;
}

}  // namespace lumi::loop
