/**
 * @file BounceAuftrag.cpp
 * @brief Loop: einen Clip fuer *Bounce* rechnen (Stufe 15k-2, S170)
 */

#include "loop/BounceAuftrag.hpp"

#include "loop/Instrument.hpp"  // instrumentAuskunft

#include <QCoreApplication>

#include <utility>

namespace lumi::loop
{

BounceAuftrag::BounceAuftrag(Loop loop, ClipOrt ort, int rate, Dekodierer dekodierer)
    : m_loop(std::move(loop)), m_ort(ort), m_rate(rate), m_dekodierer(std::move(dekodierer))
{
    m_thread = std::thread([this] { arbeite(); });
}

BounceAuftrag::~BounceAuftrag()
{
    brichAb();
    warte();
}

void BounceAuftrag::brichAb()
{
    m_abbruch.store(true);
    const std::lock_guard<std::mutex> halt(m_sperre);
    if (m_toene != nullptr) m_toene->brichAb();
}

void BounceAuftrag::warte()
{
    if (m_thread.joinable()) m_thread.join();
}

double BounceAuftrag::anteil() const
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    if (m_toene != nullptr && m_toene->gesamt() > 0) return 0.5 * static_cast<double>(m_toene->erledigt()) / m_toene->gesamt();
    return m_anteil.load();
}

std::optional<Wellenform> BounceAuftrag::welle() const
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    return m_welle;
}

QStringList BounceAuftrag::bericht() const
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    return m_bericht;
}

void BounceAuftrag::arbeite()
{
    // die Auskunft ueber die Saiten lebt auf diesem Thread — nur er fragt sie (wie bei Freeze)
    const InstrumentVon saiten = instrumentAuskunft();
    std::optional<Wellenform> welle;
    QStringList bericht;
    if (const std::optional<Loop> fassung = clipFassung(m_loop, m_ort))
    {
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
            klaenge = std::make_shared<const Klaenge>(toene.klaenge());
            bericht << toene.bericht();
        }
        if (!m_abbruch.load())
        {
            m_anteil.store(0.5);
            welle = rechneClip(m_loop, m_ort, m_rate, klaenge, saiten, [this](double anteil) {
                m_anteil.store(0.5 + 0.5 * anteil);
                return !m_abbruch.load();
            });
        }
    }
    else
        bericht << QCoreApplication::translate("lumi::loop::BounceAuftrag", "Clip not found: bar %1").arg(m_ort.startTakt + 1);
    if (m_abbruch.load()) welle.reset();
    {
        const std::lock_guard<std::mutex> halt(m_sperre);
        m_welle = std::move(welle);
        m_bericht = std::move(bericht);
    }
    m_beendet.store(true);  // zuletzt: wer `beendet` sieht, findet das Ergebnis
}

}  // namespace lumi::loop
