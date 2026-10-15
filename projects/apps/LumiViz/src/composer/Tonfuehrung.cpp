/**
 * @file Tonfuehrung.cpp
 * @brief Composer: die Audio-Kanaele im Mixer (Stufe C5b)
 */

#include "composer/Tonfuehrung.hpp"

#include "audio/AudioPlayer.hpp"  // leseTrackInfo (Naht N14)
#include "audio/IAudioPlayer.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Offen.hpp"

#include <QFileInfo>
#include <QStringList>

#include <BasicLogger.h>

#include <algorithm>
#include <iterator>
#include <cmath>
#include <cstdlib>
#include <string>

namespace lumi::composer
{

// =============================================================================
// Ein Loop als Quelle (S109, Loop_Konzept §4e)
// =============================================================================

struct Tonfuehrung::LoopQuelle
{
    lumi::loop::Klangerzeuger erzeuger;  ///< die Quelle im Mixer; lebt laenger als sie
    QString datei;
    lumi::loop::Loop loop;               ///< gerichtet — was gerade spielt
    std::uint64_t gesehen = 0;           ///< `offenFassung` beim letzten Blick ins Loop-Panel
    std::shared_ptr<const std::vector<lumi::loop::Klang>> klaenge;
    QStringList klangNamen;              ///< die Klaenge, zu denen `klaenge` gehoert
    std::uint64_t instrumente = 0;       ///< `instrumentFassung()` beim Laden der Klaenge
    int rate = 0;
    double durchlaufMs = 0.0;            ///< ein Durchlauf, wie der Klangerzeuger ihn spielt

    /// Den Loop holen: die Fassung, die das Loop-Panel offen hat (auch die
    /// ungespeicherte, L7 a) — sonst die Datei. false = kein Loop.
    bool lies()
    {
        gesehen = lumi::loop::offenFassung();
        lumi::loop::Loop neu;
        if (!lumi::loop::offenerLoop(datei, neu) && !lumi::loop::ladeLoop(datei, neu)) return false;
        neu.richte();
        loop = neu;
        return true;
    }

    /// Den Loop uebersetzen und dem Klangerzeuger geben; die Klaenge werden
    /// nur neu geladen, wenn sich die Klaenge der Spuren geaendert haben.
    void baue(IAudioEngine& engine)
    {
        rate = engine.getSampleRate();
        const QStringList namen = loop.klangNamen();
        // auch wenn eine Instrument-Datei neu gespeichert wurde
        const std::uint64_t fassung = lumi::loop::instrumentFassung();
        if (klaenge == nullptr || namen != klangNamen || fassung != instrumente)
        {
            klaenge = std::make_shared<const std::vector<lumi::loop::Klang>>(
                lumi::loop::ladeKlaenge(namen, engine, rate));
            klangNamen = namen;
            instrumente = fassung;
        }
        const auto programm = lumi::loop::baueProgramm(loop, rate, klaenge);
        durchlaufMs = rate > 0 ? 1000.0 * static_cast<double>(programm->ablauf.durchlaufFrames) / rate : 0.0;
        erzeuger.setzeProgramm(programm);
    }

    /// Die Stelle im Durchlauf zu einer Stelle im Loop (er wiederholt sich).
    [[nodiscard]] double imDurchlauf(std::int64_t sollMs) const
    {
        if (durchlaufMs <= 0.0) return 0.0;
        const double rest = std::fmod(static_cast<double>(sollMs), durchlaufMs);
        return rest < 0.0 ? rest + durchlaufMs : rest;
    }

    void setze(std::int64_t sollMs)
    {
        erzeuger.setzeStelle(std::llround(imDurchlauf(sollMs) * rate / 1000.0));
    }

    /// Wie weit der Klangerzeuger neben der Stelle liegt — im Kreis gemessen:
    /// kurz vor dem Ende des Durchlaufs ist nah an seinem Anfang.
    [[nodiscard]] std::int64_t daneben(std::int64_t sollMs) const
    {
        if (durchlaufMs <= 0.0 || rate <= 0) return 0;
        const double ist = 1000.0 * static_cast<double>(erzeuger.stelleImDurchlauf()) / rate;
        double abstand = ist - imDurchlauf(sollMs);
        if (abstand > durchlaufMs / 2.0) abstand -= durchlaufMs;
        if (abstand < -durchlaufMs / 2.0) abstand += durchlaufMs;
        return std::llround(abstand);
    }

    /// Der Loop als laufender Song: seine Tags, ohne Titel der Dateiname; die
    /// Laenge ist ein Durchlauf.
    [[nodiscard]] TrackInfo alsSong() const
    {
        TrackInfo song;
        song.filePath = datei;
        song.title = loop.tags.titel.isEmpty() ? QFileInfo(datei).completeBaseName() : loop.tags.titel;
        song.artist = loop.tags.kuenstler;
        song.album = loop.tags.album;
        song.year = loop.tags.jahr;
        song.genre = loop.tags.genre;
        song.trackNumber = loop.tags.nummer;
        song.lyricsPlain = loop.tags.text;
        song.durationMs = static_cast<int>(std::lround(durchlaufMs));
        song.sampleRate = rate;
        song.channels = 2;
        return song;
    }
};

namespace
{
/// So viele Takte lebt der Klangerzeuger einer entfernten Quelle noch.
constexpr int kAbgeloestTakte = 4;
}  // namespace

bool Tonfuehrung::uebernimm(IAudioEngine& engine, IAudioPlayer& player)
{
    if (fuehrt()) return true;
    const AudioStreamHandle mixer = engine.createMixer();
    if (mixer == INVALID_STREAM) return false;  // die Engine hat keinen Mixer
    m_engine = &engine;
    m_player = &player;
    m_mixer = mixer;
    // der Player schweigt, und die Playlist ruht: ohne sie spielt am Titelende
    // nichts von selbst weiter
    m_playerSpielte = player.isPlaying();
    if (m_playerSpielte) player.pause();
    m_playlist = player.playlist();
    player.setPlaylist(nullptr);
    m_quellen.clear();
    m_laut = -1.0f;
    m_laufend = {};
    m_laufendDauerMs = 0;
    m_liefZuletzt = false;
    m_nachgesetzt = 0;
    return true;
}

void Tonfuehrung::gibFrei(bool playerWeiter)
{
    if (!fuehrt()) return;
    for (const Quelle& q : m_quellen)
        if (q.handle != INVALID_STREAM) m_engine->mixerRemoveSource(q.handle);
    // die Klangerzeuger der Loops leben, bis der Mixer fort ist
    const std::vector<Quelle> bisher = std::move(m_quellen);
    m_quellen.clear();
    m_engine->freeStream(m_mixer);
    m_abgeloest.clear();
    m_player->setPlaylist(m_playlist);
    if (playerWeiter && m_playerSpielte) m_player->play();
    m_playerSpielte = false;
    m_engine = nullptr;
    m_player = nullptr;
    m_playlist = nullptr;
    m_mixer = INVALID_STREAM;
}

int Tonfuehrung::quellen() const
{
    return static_cast<int>(std::count_if(m_quellen.begin(), m_quellen.end(),
                                          [](const Quelle& q) { return q.handle != INVALID_STREAM; }));
}

int Tonfuehrung::loops() const
{
    return static_cast<int>(std::count_if(m_quellen.begin(), m_quellen.end(), [](const Quelle& q) {
        return q.handle != INVALID_STREAM && q.loop != nullptr;
    }));
}

Tonfuehrung::Ergebnis Tonfuehrung::takt(const Komposition& komposition, std::int64_t stelleMs, bool laeuft,
                                        Schleifenlage schleife)
{
    Ergebnis ergebnis;
    if (!fuehrt()) return ergebnis;
    IAudioEngine& engine = *m_engine;

    // --- der Player schweigt; seine Lautstaerke und sein Mute gelten fuer den ganzen Mixer
    if (m_player->isPlaying())
    {
        m_player->pause();
        ergebnis.playerAngehalten = true;
    }
    // Lautstaerke und Mute des Players — und sein Pegel (Naht N17): der Mute
    // fuer Durchsagen gilt so auch, wenn der Composer den Ton fuehrt (S101)
    const float laut = m_player->isMuted() ? 0.0f : m_player->volume() * m_player->pegel();
    if (laut != m_laut)
    {
        engine.setVolume(m_mixer, laut);
        m_laut = laut;
    }

    const bool liefZuletzt = m_liefZuletzt;
    m_liefZuletzt = laeuft;

    // --- Klangerzeuger, deren Quelle vor ein paar Takten ging: jetzt duerfen sie gehen
    for (Abgeloest& a : m_abgeloest) --a.takte;
    m_abgeloest.erase(std::remove_if(m_abgeloest.begin(), m_abgeloest.end(),
                                     [](const Abgeloest& a) { return a.takte <= 0; }),
                      m_abgeloest.end());

    // --- die Stimmen dieses Takts: jeder Ton-Clip, der spielt oder gleich
    //     beginnt, an seiner Stelle der Zeitachse. Ausserhalb der Schleife ist
    //     das die Stelle der Uhr; im Fenster der Ueberlappung (S99) kommen die
    //     Clips der hinteren Stelle dazu, und vor dem Umschlag die ersten der
    //     naechsten Runde (im Vorlauf, an einer Stelle vor null).
    struct Stimme
    {
        const Clip* clip;
        std::int64_t stelleMs;  ///< ihre Stelle auf der Zeitachse
        std::int64_t sollMs;    ///< die Stelle im Song, die dort zu hoeren ist
    };
    std::vector<Stimme> stimmen;
    const auto stimme = [](const Clip* clip, std::int64_t stelle) {
        return Stimme{clip, stelle, clip->songStelle(std::max<std::int64_t>(0, stelle - clip->startMs))};
    };
    const auto hat = [&stimmen](const Clip* clip) {
        return std::find_if(stimmen.begin(), stimmen.end(),
                            [clip](const Stimme& s) { return s.clip == clip; });
    };
    for (const Clip* clip : komposition.toeneBei(stelleMs, kVorlaufMs)) stimmen.push_back(stimme(clip, stelleMs));
    if (schleife.zweiteMs >= 0)
    {
        for (const Clip* clip : komposition.toeneBei(schleife.zweiteMs))
        {
            const auto vorn = hat(clip);
            if (vorn == stimmen.end() || komposition.songsDoppelt)
                stimmen.push_back(stimme(clip, schleife.zweiteMs));
            else
                *vorn = stimme(clip, schleife.zweiteMs);  // ohne zweite Quelle spielt er sein Ende aus
        }
    }
    if (schleife.umschlagInMs >= 0 && schleife.umschlagInMs <= kVorlaufMs)
    {
        for (const Clip* clip : komposition.toeneBei(-schleife.umschlagInMs, kVorlaufMs))
            if (hat(clip) == stimmen.end() || komposition.songsDoppelt)
                stimmen.push_back(stimme(clip, -schleife.umschlagInMs));
    }

    // --- die Quellen des Mixers den Stimmen zuordnen. Eine Quelle bleibt bei
    //     der Stimme ihres Clips, deren Stelle im Song ihr am naechsten liegt:
    //     so spielt am Umschlag die laufende Quelle das Ende weiter, und der
    //     Anfang bekommt eine frische — nichts wird dafuer neu gesetzt.
    const auto schluesselVon = [](const Clip* clip) {
        return Schluessel{clip->datei, clip->startMs, clip->einsatzMs};
    };
    struct Paar
    {
        std::int64_t abstand;
        std::size_t quelle;
        std::size_t stimme;
    };
    std::vector<Paar> paare;
    for (std::size_t s = 0; s < stimmen.size(); ++s)
        for (std::size_t q = 0; q < m_quellen.size(); ++q)
            if (m_quellen[q].schluessel == schluesselVon(stimmen[s].clip))
                paare.push_back({std::llabs(m_quellen[q].sollMs - stimmen[s].sollMs), q, s});
    std::stable_sort(paare.begin(), paare.end(), [](const Paar& a, const Paar& b) { return a.abstand < b.abstand; });
    constexpr std::size_t kKeine = static_cast<std::size_t>(-1);
    std::vector<std::size_t> quelleVon(stimmen.size(), kKeine);
    for (Quelle& q : m_quellen) q.vergeben = false;
    for (const Paar& p : paare)
    {
        if (m_quellen[p.quelle].vergeben || quelleVon[p.stimme] != kKeine) continue;
        m_quellen[p.quelle].vergeben = true;
        quelleVon[p.stimme] = p.quelle;
    }
    std::vector<Quelle> quellen;  // danach: quellen[i] gehoert zu stimmen[i]
    quellen.reserve(stimmen.size());
    for (std::size_t s = 0; s < stimmen.size(); ++s)
    {
        if (quelleVon[s] != kKeine)
        {
            quellen.push_back(m_quellen[quelleVon[s]]);
            continue;
        }
        const Clip* clip = stimmen[s].clip;
        Quelle q;
        q.schluessel = schluesselVon(clip);
        q.sollMs = stimmen[s].sollMs;
        // laesst sie sich nicht oeffnen, bleibt der Eintrag: ein Versuch, dann Stille
        if (istLoop(clip->datei))
        {
            // ein Loop: sein eigener Klangerzeuger im Mixer (angehalten eingehaengt).
            // Eine Song-Laenge hat er nicht — `dauerMs` bleibt 0, er endet nie.
            auto loop = std::make_shared<LoopQuelle>();
            loop->datei = clip->datei;
            if (loop->lies())
            {
                loop->baue(engine);
                q.handle = engine.mixerAddGenerator(m_mixer, &loop->erzeuger);
                if (q.handle != INVALID_STREAM)
                {
                    loop->setze(std::max<std::int64_t>(0, clip->einsatzMs));
                    q.loop = std::move(loop);
                    // War dieser Clip zuletzt der laufende Song, wird er neu
                    // gemeldet: seine Fassung kann seither eine andere sein
                    // (Titel, Laenge) — bei einer Datei aendert sich das nie.
                    if (q.schluessel == m_laufend) m_laufend = {};
                }
            }
        }
        else
        {
            q.handle = engine.mixerAddSource(m_mixer, clip->datei);
            if (q.handle != INVALID_STREAM)
            {
                q.dauerMs = engine.getDurationMs(q.handle);
                engine.setSourcePositionMs(q.handle, static_cast<int>(std::max<std::int64_t>(0, clip->einsatzMs)));
            }
        }
        quellen.push_back(q);
    }
    for (const Quelle& q : m_quellen)
        if (!q.vergeben && q.handle != INVALID_STREAM)
        {
            engine.mixerRemoveSource(q.handle);
            if (q.loop != nullptr) m_abgeloest.push_back({q.loop, kAbgeloestTakte});
        }
    m_quellen = std::move(quellen);

    // --- je Stimme: Pegel, Stelle, laufen — und wer der laufende Song ist
    const Clip* bester = nullptr;
    const Quelle* besteQuelle = nullptr;
    double bestPegel = -1.0;
    std::int64_t besteStelle = 0;
    bool besterZuEnde = false;
    constexpr double kGleich = 1e-6;
    for (std::size_t s = 0; s < stimmen.size(); ++s)
    {
        const Clip* clip = stimmen[s].clip;
        Quelle& q = m_quellen[s];
        const std::int64_t stimmStelle = stimmen[s].stelleMs;
        const std::int64_t soll = stimmen[s].sollMs;
        // Ist diese Stimme gesprungen (Lineal, Stop, ein Sprung im Song)? Ein
        // Schritt im Song, wie ihn kein Takt macht: rueckwaerts oder weit
        // voraus — bei stehender oder eben angelaufener Uhr jede Bewegung.
        // Der Umschlag der Schleife ist keiner: die Stimme des Endes laeuft
        // stetig weiter.
        const std::int64_t schritt = soll - q.sollMs;
        bool sprang = laeuft && liefZuletzt ? (schritt < -kToleranzMs || schritt > kSprungMs)
                                            : schritt != 0;
        q.sollMs = soll;
        if (q.handle == INVALID_STREAM) continue;
        // Ein Loop, den das Loop-Panel offen hat, folgt dessen Fassung (L7 a):
        // geaendert hat sich nur etwas, wenn das Panel seit dem letzten Blick
        // gemeldet hat. Ist der Durchlauf danach anders lang (Tempo, Clips),
        // liegt dieselbe Stelle der Uhr woanders im Loop — dann wird gesetzt.
        if (q.loop != nullptr && lumi::loop::offenFassung() != q.loop->gesehen)
        {
            q.loop->gesehen = lumi::loop::offenFassung();
            lumi::loop::Loop neu;
            if (lumi::loop::offenerLoop(clip->datei, neu))
            {
                neu.richte();
                if (!(neu == q.loop->loop))
                {
                    const double vorher = q.loop->durchlaufMs;
                    const bool tagsAnders = !(neu.tags == q.loop->loop.tags);
                    q.loop->loop = neu;
                    q.loop->baue(engine);
                    if (q.loop->durchlaufMs != vorher) sprang = true;
                    // der laufende Song heisst jetzt anders: er wird neu gemeldet
                    if ((tagsAnders || q.loop->durchlaufMs != vorher) && q.schluessel == m_laufend) m_laufend = {};
                }
            }
        }
        const auto daneben = [&engine, &q](std::int64_t stelle) -> std::int64_t {
            return q.loop != nullptr ? q.loop->daneben(stelle) : engine.getSourcePositionMs(q.handle) - stelle;
        };
        const auto setzeStelle = [&engine, &q](std::int64_t stelle) {
            if (q.loop != nullptr)
                q.loop->setze(stelle);
            else
                engine.setSourcePositionMs(q.handle, static_cast<int>(stelle));
        };
        const std::int64_t imClip = stimmStelle - clip->startMs;
        // im Vorlauf, oder der Song ist vor dem Ende seines Clips aus: die Quelle haelt
        const bool zuEnde = q.dauerMs > 0 && soll >= q.dauerMs;
        if (imClip < 0 || zuEnde)
        {
            if (q.imClip)
            {
                engine.setSourcePaused(q.handle, true);
                q.imClip = false;
            }
            if (imClip < 0) continue;
        }
        // die Pegelkurve des Clips mal die Kurven der ganzen Komposition an der
        // Stelle der Stimme — MASTER und, nur fuer den Ton, AUDIO (S101)
        const double pegelWert = clip->pegelBei(imClip) * komposition.tonPegelBei(stimmStelle);
        if (!zuEnde)
        {
            // zu hoeren ist der Pegel mal das Gain des Clips (S126, L1 a); den
            // laufenden Song bestimmt weiter die Pegelkurve allein (unten)
            const auto pegel = static_cast<float>(pegelWert * clip->gainFaktor());
            if (pegel != q.pegel)
            {
                engine.setVolume(q.handle, pegel);
                q.pegel = pegel;
            }
            const bool einstieg = !q.imClip;
            bool setze = einstieg || sprang;
            if (!setze && laeuft && std::llabs(stimmStelle - q.gesetztBeiMs) >= kSchonfristMs)
            {
                const std::int64_t abstand = daneben(soll);
                if (!q.hatGrund)
                {
                    // Was die Engine als „zu hoeren" meldet, liegt um die Latenz
                    // der Ausgabe hinter der Uhr (gemessen S98: stetig etwa 45 ms
                    // am Geraet des Rechners; ueber Funk ein Vielfaches). Das ist
                    // der Grundabstand dieser Quelle, kein Davonlaufen.
                    q.grundMs = abstand;
                    q.hatGrund = true;
                }
                else if (std::llabs(abstand - q.grundMs) > kToleranzMs)
                {
                    // die Quelle ist der Uhr davongelaufen (oder sie ihr)
                    setze = true;
                    ++m_nachgesetzt;
                    if (m_nachgesetzt <= 200 || m_nachgesetzt % 50 == 0)
                        BasicLogger::logInfo("Composer: audio source realigned to the clock (" +
                                             std::to_string(abstand - q.grundMs) + " ms off its usual " +
                                             std::to_string(q.grundMs) + " ms, " +
                                             QFileInfo(clip->datei).fileName().toStdString() + ", " +
                                             std::to_string(m_nachgesetzt) + " so far)");
                }
            }
            if (setze)
            {
                setzeStelle(soll);
                q.gesetztBeiMs = stimmStelle;
                q.hatGrund = false;  // nach dem Setzen neu messen
            }
            if (einstieg)
            {
                engine.setSourcePaused(q.handle, false);
                q.imClip = true;
            }
        }
        // der laufende Song: der hoechste Wert der Pegelkurve; bei Gleichstand
        // der bisherige, sonst der oberste Kanal (der erste in der Liste)
        const bool gleich = bester != nullptr && std::abs(pegelWert - bestPegel) <= kGleich;
        if (bester == nullptr || (!gleich && pegelWert > bestPegel) || (gleich && q.schluessel == m_laufend))
        {
            bester = clip;
            besteQuelle = &q;
            bestPegel = pegelWert;
            besteStelle = soll;
            besterZuEnde = zuEnde;
        }
    }

    // --- haelt die Uhr, haelt der Mixer
    const bool mixerSpielt = engine.isPlaying(m_mixer);
    if (laeuft && !mixerSpielt)
        engine.play(m_mixer);
    else if (!laeuft && mixerSpielt)
        engine.pause(m_mixer);

    if (bester != nullptr)
    {
        ergebnis.hatSong = true;
        // ein Loop zaehlt als laufender Song (Entscheid Patrik S109): seine
        // Laenge ist ein Durchlauf, seine Stelle die im laufenden Durchlauf
        const LoopQuelle* loop = besteQuelle->loop.get();
        ergebnis.songStelleMs =
            loop != nullptr ? static_cast<std::int64_t>(loop->imDurchlauf(besteStelle)) : besteStelle;
        ergebnis.songLaeuft = laeuft && !besterZuEnde;
        if (!(besteQuelle->schluessel == m_laufend))
        {
            m_laufend = besteQuelle->schluessel;
            ergebnis.songGewechselt = true;
            ergebnis.song =
                loop != nullptr ? loop->alsSong() : leseTrackInfo(engine, besteQuelle->handle, bester->datei);
            m_laufendDauerMs = loop != nullptr ? ergebnis.song.durationMs : besteQuelle->dauerMs;
        }
        ergebnis.songDauerMs = m_laufendDauerMs;
    }
    return ergebnis;
}

std::int64_t Tonfuehrung::stelleImLaufenden(const Komposition& komposition, double anteil) const
{
    if (m_laufend.startMs < 0 || m_laufendDauerMs <= 0) return -1;
    for (const Kanal& kanal : komposition.kanaele)
    {
        if (kanal.art != KanalArt::Audio || !kanal.an) continue;
        for (const Clip& c : kanal.clips)
        {
            if (!c.an || !(Schluessel{c.datei, c.startMs, c.einsatzMs} == m_laufend)) continue;
            const auto imSong = static_cast<std::int64_t>(std::clamp(anteil, 0.0, 1.0) * m_laufendDauerMs);
            // was vor dem Einsatz oder hinter dem Ende des Clips laege, gibt es in der Komposition nicht
            return std::clamp<std::int64_t>(c.startMs + (imSong - c.einsatzMs), c.startMs, c.endeMs - 1);
        }
    }
    return -1;
}

std::int64_t Tonfuehrung::songAnfang(const Komposition& komposition, std::int64_t stelleMs, bool vor)
{
    // die eingeschalteten Songs aller eingeschalteten Audio-Kanaele, nach ihrem Anfang
    std::vector<Clip> clips;
    for (const Kanal& kanal : komposition.kanaele)
        if (kanal.art == KanalArt::Audio && kanal.an)
            std::copy_if(kanal.clips.begin(), kanal.clips.end(), std::back_inserter(clips),
                         [](const Clip& c) { return c.an; });
    std::stable_sort(clips.begin(), clips.end(),
                     [](const Clip& a, const Clip& b) { return a.startMs < b.startMs; });
    if (vor)
    {
        for (const Clip& c : clips)
            if (c.startMs > stelleMs) return c.startMs;
        return -1;
    }
    std::int64_t ziel = -1;
    for (const Clip& c : clips)
    {
        if (c.startMs > stelleMs) break;
        // mitten im Song: an seinen Anfang; in seinen ersten Sekunden: zum Song davor
        const bool drin = stelleMs < c.endeMs;
        if (drin && stelleMs - c.startMs <= kZurueckMs)
            return ziel >= 0 ? ziel : c.startMs;
        ziel = c.startMs;
    }
    return ziel;
}

}  // namespace lumi::composer
