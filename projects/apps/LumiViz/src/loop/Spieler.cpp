/**
 * @file Spieler.cpp
 * @brief Loop: ein Loop spielt allein, ohne Composer (Stufe 5b)
 */

#include "loop/Spieler.hpp"

#include "audio/IAudioPlayer.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"

namespace lumi::loop
{

namespace
{

Spieler* g_spielender = nullptr;

}  // namespace

Spieler::Spieler(Vorrang vorrang) : m_vorrang(std::move(vorrang))
{
    m_loop.richte();
}

Spieler::~Spieler()
{
    halte(false);
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
    if (spielt()) ladeProgramm();
}

void Spieler::ladeProgramm()
{
    const int rate = m_engine->getSampleRate();
    const QStringList namen = m_loop.klangNamen();
    // auch wenn eine Instrument-Datei neu gespeichert wurde: ihre Toene sind dann andere
    const std::uint64_t instrumente = instrumentFassung();
    if (m_klaenge == nullptr || namen != m_klangNamen || rate != m_klangRate || instrumente != m_instrumentFassung)
    {
        m_bericht.clear();
        m_klaenge = std::make_shared<const std::vector<Klang>>(ladeKlaenge(namen, *m_engine, rate, &m_bericht));
        m_klangNamen = namen;
        m_klangRate = rate;
        m_instrumentFassung = instrumente;
    }
    m_erzeuger.setzeProgramm(baueProgramm(m_loop, rate, m_klaenge));
}

bool Spieler::starte(IAudioEngine& engine, IAudioPlayer& player)
{
    if (!darfSpielen()) return false;
    if (spielt())
    {
        m_erzeuger.setzeStelle(0);
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

    ladeProgramm();
    m_erzeuger.setzeStelle(0);
    takt();  // die Lautstaerke steht, bevor der erste Block zu hoeren ist
    engine.setSourcePaused(m_quelle, false);
    engine.play(m_mixer);
    return true;
}

void Spieler::halte(bool playerWeiter)
{
    if (!spielt()) return;
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
    if (!spielt()) return;
    if (!darfSpielen())
    {
        // der Composer hat uebernommen: er fuehrt jetzt auch den Player
        halte(false);
        return;
    }
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

double Spieler::stelleSchritte() const
{
    if (!spielt()) return 0.0;
    const std::shared_ptr<const Programm> programm = m_erzeuger.programm();
    if (programm == nullptr || programm->ablauf.schrittFrames <= 0.0) return 0.0;
    return static_cast<double>(m_erzeuger.stelleImDurchlauf()) / programm->ablauf.schrittFrames;
}

}  // namespace lumi::loop
