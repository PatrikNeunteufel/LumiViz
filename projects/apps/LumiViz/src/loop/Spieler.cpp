/**
 * @file Spieler.cpp
 * @brief Loop: ein Loop spielt allein, ohne Composer (Stufe 5b; Toene im Hintergrund seit Stufe 29d)
 */

#include "loop/Spieler.hpp"

#include "audio/IAudioPlayer.hpp"
#include "loop/Freeze.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

Spieler* g_spielender = nullptr;

/// Dekodieren ueber die Engine (TV3 a) — auf den Threads eines Auftrags.
Dekodierer dekodiererVon(IAudioEngine& engine)
{
    return [&engine](const QString& datei, int rate, double maxSekunden) { return engine.decodeStereo(datei, rate, maxSekunden); };
}

}  // namespace

Spieler::Spieler(Vorrang vorrang) : m_vorrang(std::move(vorrang))
{
    m_loop.richte();
}

Spieler::~Spieler()
{
    halte(false);
    // die Threads der Auftraege enden hier — vor der Engine (Bootstrap_Integration.md §6.7)
    m_vorab.reset();
    m_verworfen.clear();
}

Spieler* Spieler::spielender()
{
    return g_spielender;
}

AudioStreamHandle tonMixer()
{
    return g_spielender != nullptr ? g_spielender->mixer() : INVALID_STREAM;
}

void Spieler::setzeLoop(const Loop& loop)
{
    m_loop = loop;
    m_loop.richte();
    if (spielt()) ladeProgramm(false);
}

Spieler::Satz Spieler::satzJetzt(int rate) const
{
    // die Saiten der Instrumente (Stufe 23b): dieselbe Auskunft fuer Namen und Programm;
    // eine neu gespeicherte Instrument-Datei gibt andere Toene; eingefrorene Clips (Stufe 30f)
    // spielen ihre Wellenform, wenn sie zum jetzigen Abdruck vorliegt
    const SaitenVon saiten = saitenAusInstrumenten();
    Eingefroren eingefroren = eingefrorenVon(m_loop, rate, saiten);
    QStringList namen = m_loop.klangNamen(saiten, eingefroren);
    return {std::move(namen), rate, instrumentFassung(), std::move(eingefroren)};
}

bool Spieler::nimmKlaenge(Tonauftrag& auftrag, const Satz& satz)
{
    m_bericht = auftrag.bericht();
    Klaenge klaenge = auftrag.klaenge();
    if (frozenFehlt(satz.namen, klaenge))
    {
        // eine Wellenform liess sich nicht lesen (30f): neu planen, der Clip spielt live
        m_klaenge.reset();
        m_satz = {};
        return false;
    }
    m_klaenge = std::make_shared<const Klaenge>(std::move(klaenge));
    m_satz = satz;
    return true;
}

void Spieler::verwirf(std::unique_ptr<Tonauftrag>& auftrag)
{
    if (auftrag == nullptr) return;
    auftrag->brichAb();
    m_verworfen.push_back(std::move(auftrag));
}

void Spieler::raeumeAuf()
{
    // wer ruht, blockiert nicht mehr beim Zerstoeren
    std::erase_if(m_verworfen, [](const std::unique_ptr<Tonauftrag>& a) { return a->ruht(); });
}

void Spieler::ladeProgramm(bool vonVorn)
{
    // der Begrenzer am Mixer des Loops (27e-3, LL7 a): wie der Loop es sagt
    m_engine->setMixerLimiter(m_mixer, m_loop.limiter);
    const int rate = m_engine->getSampleRate();
    const Satz satz = satzJetzt(rate);
    if (m_klaenge != nullptr && satz == m_satz)
    {
        // dieselben Klaenge: das Programm gleich, ein laufender Auftrag ist ueberholt
        verwirf(m_auftrag);
        m_wartet = false;
        m_erzeuger.setzeProgramm(baueProgramm(m_loop, rate, m_klaenge, saitenAusInstrumenten(), m_satz.eingefroren));
        return;
    }
    if (m_auftrag == nullptr || !(m_auftragSatz == satz))
    {
        verwirf(m_auftrag);
        if (m_vorab != nullptr && m_vorabSatz == satz)
        {
            // was vorab rechnet, ist genau das: uebernehmen statt neu anzufangen
            m_auftrag = std::move(m_vorab);
        }
        else
        {
            auto auftrag = std::make_unique<Tonauftrag>(satz.namen, rate, dekodiererVon(*m_engine));
            if (auftrag->ausSpeicher())
            {
                // TV6 a: alles im Speicher — sofort, ohne Hintergrund (eine eingefrorene
                // Wellenform kommt dann aus dem Speicher: `nimmKlaenge` gelingt)
                (void)nimmKlaenge(*auftrag, satz);
                m_wartet = false;
                m_erzeuger.setzeProgramm(baueProgramm(m_loop, rate, m_klaenge, saitenAusInstrumenten(), m_satz.eingefroren));
                return;
            }
            auftrag->starte();
            m_auftrag = std::move(auftrag);
        }
        m_auftragSatz = satz;
    }
    if (vonVorn)
    {
        // bis die Toene da sind, still (H2 a); das alte Programm gehoerte zu einem anderen Loop
        m_erzeuger.setzeProgramm(nullptr);
        m_wartet = true;
        m_startSchritt = 0.0;
    }
    schalteUm();  // vielleicht ist er schon fertig
}

void Spieler::schalteUm()
{
    if (m_auftrag == nullptr || !m_auftrag->fertig()) return;
    const bool genommen = nimmKlaenge(*m_auftrag, m_auftragSatz);
    verwirf(m_auftrag);  // fertig: seine Threads laufen gerade aus
    if (!genommen)
    {
        // eine eingefrorene Wellenform fehlte (30f): neu planen — `eingefrorenVon` nennt ihren
        // Clip nicht mehr, er spielt live; bis dahin klingt, was klang
        const bool wartete = m_wartet;
        ladeProgramm(false);
        if (wartete && !m_wartet)
        {
            // gleich aus dem Speicher: beginnen, wie nach dem Warten
            m_erzeuger.setzeStelle(0);
            if (m_startSchritt > 0.0) (void)springe(m_startSchritt);
        }
        return;
    }
    const int rate = m_satz.rate;
    m_erzeuger.setzeProgramm(baueProgramm(m_loop, rate, m_klaenge, saitenAusInstrumenten(), m_satz.eingefroren));
    if (m_wartet)
    {
        m_wartet = false;
        m_erzeuger.setzeStelle(0);
        if (m_startSchritt > 0.0) (void)springe(m_startSchritt);
    }
}

bool Spieler::starte(IAudioEngine& engine, IAudioPlayer& player)
{
    if (!darfSpielen()) return false;
    if (spielt())
    {
        m_erzeuger.setzeStelle(0);
        m_startSchritt = 0.0;
        return true;
    }
    // es gibt nur einen Ton: ein anderer Spieler gibt ihn ab
    if (g_spielender != nullptr) g_spielender->halte(false);

    const AudioStreamHandle mixer = engine.createMixer();
    if (mixer == INVALID_STREAM) return false;
    const AudioStreamHandle quelle = engine.mixerAddGenerator(mixer, &m_erzeuger);
    if (quelle == INVALID_STREAM)
    {
        engine.freeStream(mixer);
        return false;
    }
    m_engine = &engine;
    m_player = &player;
    m_mixer = mixer;
    m_quelle = quelle;
    m_playerSpielte = player.isPlaying();
    if (m_playerSpielte) player.pause();
    m_laut = -1.0f;
    g_spielender = this;

    ladeProgramm(true);
    if (!m_wartet) m_erzeuger.setzeStelle(0);
    takt();  // die Lautstaerke steht, bevor der erste Block zu hoeren ist
    engine.setSourcePaused(m_quelle, false);
    engine.play(m_mixer);
    return true;
}

void Spieler::ladeVor(IAudioEngine& engine)
{
    if (spielt()) return;
    const Satz satz = satzJetzt(engine.getSampleRate());
    if ((m_klaenge != nullptr && satz == m_satz) || (m_vorab != nullptr && m_vorabSatz == satz)) return;
    verwirf(m_vorab);
    auto auftrag = std::make_unique<Tonauftrag>(satz.namen, satz.rate, dekodiererVon(engine));
    if (auftrag->ausSpeicher())
    {
        (void)nimmKlaenge(*auftrag, satz);
        return;
    }
    auftrag->starte();
    m_vorab = std::move(auftrag);
    m_vorabSatz = satz;
}

void Spieler::warteAufToene()
{
    if (m_auftrag == nullptr) return;
    m_auftrag->warte();
    schalteUm();
}

void Spieler::halte(bool playerWeiter)
{
    if (!spielt()) return;
    verwirf(m_auftrag);
    m_wartet = false;
    m_engine->mixerRemoveSource(m_quelle);
    m_engine->freeStream(m_mixer);
    IAudioPlayer* player = m_player;
    const bool weiter = playerWeiter && m_playerSpielte;
    m_engine = nullptr;
    m_player = nullptr;
    m_mixer = INVALID_STREAM;
    m_quelle = INVALID_STREAM;
    m_playerSpielte = false;
    if (g_spielender == this) g_spielender = nullptr;
    if (weiter) player->play();
}

void Spieler::takt()
{
    raeumeAuf();
    if (m_vorab != nullptr && m_vorab->fertig())
    {
        // vorab gerechnet: die Klaenge stehen fuer das naechste Play bereit
        (void)nimmKlaenge(*m_vorab, m_vorabSatz);  // fehlte eine Wellenform: Play plant neu
        verwirf(m_vorab);
    }
    if (!spielt()) return;
    if (!darfSpielen())
    {
        // der Composer hat uebernommen: er fuehrt jetzt auch den Player
        halte(false);
        return;
    }
    schalteUm();
    // der Player schweigt; seine Lautstaerke, sein Mute und sein Pegel gelten
    // fuer den Mixer des Loops
    if (m_player->isPlaying()) m_player->pause();
    const float laut = m_player->isMuted() ? 0.0f : m_player->volume() * m_player->pegel();
    if (laut != m_laut)
    {
        m_engine->setVolume(m_mixer, laut);
        m_laut = laut;
    }
}

bool Spieler::springe(double schritt)
{
    if (!spielt()) return false;
    if (m_wartet)
    {
        // noch kein Programm: dort beginnt er, wenn die Toene da sind
        m_startSchritt = std::max(0.0, schritt);
        return true;
    }
    const std::shared_ptr<const Programm> programm = m_erzeuger.programm();
    if (programm == nullptr || programm->ablauf.schrittFrames <= 0.0) return false;
    std::int64_t frame = std::llround(std::max(0.0, schritt) * programm->ablauf.schrittFrames);
    if (programm->ablauf.durchlaufFrames > 0) frame %= programm->ablauf.durchlaufFrames;
    m_erzeuger.setzeStelle(frame);
    return true;
}

double Spieler::stelleSchritte() const
{
    if (!spielt()) return 0.0;
    const std::shared_ptr<const Programm> programm = m_erzeuger.programm();
    if (programm == nullptr || programm->ablauf.schrittFrames <= 0.0) return 0.0;
    return static_cast<double>(m_erzeuger.stelleImDurchlauf()) / programm->ablauf.schrittFrames;
}

}  // namespace lumi::loop
