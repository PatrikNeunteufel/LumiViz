/**
 * @file Bildquelle.cpp
 * @brief Composer: die Komposition als Bildquelle der App (Stufe C2a)
 */

#include "composer/Bildquelle.hpp"

#include "composer/Wiedergabe.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"

#include <QFileInfo>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <algorithm>

namespace lumi::composer
{

namespace
{

// GL_NVX_gpu_memory_info (NVIDIA)
constexpr GLenum kSpeicherGesamtNvx = 0x9048;  // GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
constexpr GLenum kSpeicherFreiNvx = 0x9049;    // GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX

// GL_ATI_meminfo (AMD)
constexpr GLenum kTexturFreiAti = 0x87FC;  // TEXTURE_FREE_MEMORY_ATI

/// Freier Grafikspeicher in kB; negativ = der Treiber nennt ihn nicht.
int freierSpeicherKb(QOpenGLContext& kontext, int* gesamtKb)
{
    QOpenGLFunctions* f = kontext.functions();
    if (kontext.hasExtension(QByteArrayLiteral("GL_ATI_meminfo")))
    {
        // AMD: vier Werte, der erste ist der freie Texturspeicher in kB; eine
        // Gesamtgroesse nennt die Erweiterung nicht
        GLint werte[4] = {-1, -1, -1, -1};
        f->glGetIntegerv(kTexturFreiAti, werte);
        if (gesamtKb != nullptr) *gesamtKb = -1;
        return werte[0];
    }
    if (!kontext.hasExtension(QByteArrayLiteral("GL_NVX_gpu_memory_info"))) return -1;
    GLint frei = -1;
    f->glGetIntegerv(kSpeicherFreiNvx, &frei);
    if (gesamtKb != nullptr)
    {
        GLint gesamt = -1;
        f->glGetIntegerv(kSpeicherGesamtNvx, &gesamt);
        *gesamtKb = gesamt;
    }
    return frei;
}

}  // namespace

QStringList Messung::alsText() const
{
    QStringList z;
    if (bilder == 0)
    {
        z << QStringLiteral("Composer: summary — no frame computed");
        return z;
    }
    z << QStringLiteral("Composer: summary — %1 frames computed in %2 s (%3 per second)")
             .arg(bilder)
             .arg(dauerSekunden, 0, 'f', 1)
             .arg(dauerSekunden > 0.0 ? bilder / dauerSekunden : 0.0, 0, 'f', 1);
    z << QStringLiteral("  on %1, image %2 x %3").arg(gpu).arg(groesse.width()).arg(groesse.height());
    z << QStringLiteral("  compute (CPU side): mean %1 ms, longest %2 ms at %3 s, first frame %4 ms, "
                        "longest after the first %5 ms at %6 s")
             .arg(rechnenMittelMs(), 0, 'f', 3)
             .arg(rechnenMaxMs, 0, 'f', 1)
             .arg(static_cast<double>(rechnenMaxStelleMs) / 1000.0, 0, 'f', 2)
             .arg(erstesBildMs, 0, 'f', 1)
             .arg(spaeterMaxMs, 0, 'f', 1)
             .arg(static_cast<double>(spaeterMaxStelleMs) / 1000.0, 0, 'f', 2);
    z << QStringLiteral("  present (CPU side): mean %1 ms, longest %2 ms")
             .arg(zeichnenMittelMs(), 0, 'f', 3)
             .arg(zeichnenMaxMs, 0, 'f', 2);
    z << QStringLiteral("  renderers at once: at most %1").arg(rendererMax);
    if (speicherFreiVorherKb >= 0 && speicherFreiTiefstKb >= 0)
    {
        z << QStringLiteral("  video memory (driver, whole system): free before %1 MB, lowest %2 MB, "
                            "difference %3 MB, total %4")
                 .arg(speicherFreiVorherKb / 1024)
                 .arg(speicherFreiTiefstKb / 1024)
                 .arg((speicherFreiVorherKb - speicherFreiTiefstKb) / 1024)
                 .arg(speicherGesamtKb >= 0 ? QStringLiteral("%1 MB").arg(speicherGesamtKb / 1024)
                                            : QStringLiteral("unknown"));
    }
    else
    {
        z << QStringLiteral("  video memory: not available (driver has neither "
                            "GL_NVX_gpu_memory_info nor GL_ATI_meminfo)");
    }
    return z;
}

Bildquelle::Bildquelle() = default;

Bildquelle::Bildquelle(Uhr uhr) : m_uhr(std::move(uhr)) {}

Bildquelle::~Bildquelle() = default;

// =============================================================================
// Steuerung
// =============================================================================

void Bildquelle::setKomposition(Komposition komposition)
{
    m_richtBericht.clear();
    komposition.richte(&m_richtBericht);
    m_uhr.stoppe();
    m_uhr.setLaengeMs(komposition.laenge());
    m_uhr.anDenAnfang();
    m_wartend = std::move(komposition);
    m_neueKomposition = true;
    m_nurErsetzen = false;
}

void Bildquelle::ersetzeKomposition(Komposition komposition)
{
    const bool lief = m_uhr.laeuft();
    const std::int64_t stelle = m_uhr.stelleMs();
    // Wartet schon eine ganz neue Komposition, bleibt es dabei: dann ist das
    // hier deren geaenderte Fassung, und alte Renderer gehoeren ihr nicht.
    const bool ganzNeu = m_neueKomposition && !m_nurErsetzen;
    setKomposition(std::move(komposition));
    m_nurErsetzen = !ganzNeu;
    m_uhr.springe(stelle);  // geklemmt, falls die Komposition kuerzer wurde
    if (lief && (m_uhr.laengeMs() <= 0 || m_uhr.stelleMs() < m_uhr.laengeMs())) m_uhr.starte();
}

int Bildquelle::rendererAnzahl() const
{
    return m_wiedergabe != nullptr ? m_wiedergabe->rendererAnzahl() : 0;
}

int Bildquelle::gemischteClips() const
{
    return m_wiedergabe != nullptr ? m_wiedergabe->gemischteClips() : 0;
}

bool Bildquelle::karaokeAufgelegt() const
{
    return m_wiedergabe != nullptr && m_wiedergabe->karaokeAufgelegt();
}

QStringList Bildquelle::anlaufText() const
{
    QStringList z;
    if (m_messung.langeGesamt > 0)
    {
        QStringList teile;
        for (const Messung::LangesBild& b : m_messung.lange)
            teile << QStringLiteral("%1 s: %2 ms")
                         .arg(static_cast<double>(b.stelleMs) / 1000.0, 0, 'f', 2)
                         .arg(b.ms, 0, 'f', 0);
        z << QStringLiteral("  long frames after the first (over %1 ms): %2 — %3")
                 .arg(Messung::kLangMs, 0, 'f', 0)
                 .arg(m_messung.langeGesamt)
                 .arg(teile.join(QStringLiteral(", ")));
    }
    // Nach dem Aufraeumen (App beendet, Fenster abgedockt) gibt es die
    // Wiedergabe nicht mehr — dann gilt, was sie zuletzt wusste.
    if (m_wiedergabe == nullptr) return z + m_anlaufZuletzt;
    for (const Anlauf& a : m_wiedergabe->anlaeufe())
    {
        z << QStringLiteral("  start-up at %1 s, CH%2 clip %3 (%4): load %5 ms, initialize %6 ms, "
                            "first frame %7 ms, second frame %8 ms, total %9 ms — %10")
                 .arg(static_cast<double>(a.stelleMs) / 1000.0, 0, 'f', 2)
                 .arg(a.kanal + 1)
                 .arg(a.clip + 1)
                 .arg(a.vorgewaermt ? QStringLiteral("warmed up") : QStringLiteral("on entry"))
                 .arg(a.ladenMs, 0, 'f', 1)
                 .arg(a.anlegenMs, 0, 'f', 1)
                 .arg(a.erstesBildMs, 0, 'f', 1)
                 .arg(a.zweitesBildMs, 0, 'f', 1)
                 .arg(a.summeMs(), 0, 'f', 1)
                 .arg(QFileInfo(a.datei).fileName());
    }
    return z;
}

QStringList Bildquelle::bericht() const
{
    QStringList alles = m_richtBericht;
    if (m_wiedergabe != nullptr) alles += m_wiedergabe->bericht();
    return alles;
}

// =============================================================================
// IVisualizer (Render-Thread)
// =============================================================================

void Bildquelle::initialize()
{
    m_wiedergabe = std::make_unique<Wiedergabe>();
    m_versorgt.clear();
    m_bildSteht = false;
    m_letzteStelle = -1;
    // Eine vor dem Anlegen gesetzte Komposition wartet in m_wartend; eine vor
    // einem cleanup schon uebernommene wird wieder vorgelegt.
    m_neueKomposition = true;
    m_angelegt = true;
}

void Bildquelle::resize(const QSize& size)
{
    m_groesse = size;
}

void Bildquelle::cleanup()
{
    if (m_wiedergabe != nullptr)
    {
        // Der Anlauf je Clip fuer die Zusammenfassung beim Beenden der App:
        // nur die Zeilen der Clips, ohne die Zeile der langen Bilder davor
        m_anlaufZuletzt = anlaufText();
        if (m_messung.langeGesamt > 0 && !m_anlaufZuletzt.isEmpty()) m_anlaufZuletzt.removeFirst();
        // Die Komposition ueberlebt das Aufraeumen — ausser es wartet schon
        // eine neuere.
        if (!m_neueKomposition) m_wartend = m_wiedergabe->komposition();
        m_wiedergabe.reset();  // entlaedt alle Renderer, braucht den GL-Kontext
    }
    m_versorgt.clear();
    m_bildSteht = false;
    m_angelegt = false;
}

void Bildquelle::render(float deltaTime)
{
    spiegleStand();  // fuer die Anzeige im Panel, auch wenn nichts gerechnet wird
    QOpenGLContext* kontext = QOpenGLContext::currentContext();
    if (!m_angelegt || m_wiedergabe == nullptr || kontext == nullptr || m_groesse.isEmpty()) return;

    bool neu = false;
    if (m_neueKomposition)
    {
        // Eine bearbeitete Fassung behaelt die Renderer ihrer Clips (C3c):
        // das Visual laeuft weiter, statt von vorn zu beginnen.
        if (m_nurErsetzen)
            m_wiedergabe->ersetzeKomposition(std::move(m_wartend));
        else
            m_wiedergabe->setKomposition(std::move(m_wartend));
        m_wartend = Komposition{};
        m_neueKomposition = false;
        m_nurErsetzen = false;
        m_versorgt.clear();
        neu = true;
    }

    const bool laeuft = m_uhr.laeuft();
    const std::int64_t stelle = m_uhr.stelleMs();
    // Steht die Uhr, stehen die Clips — die Karaoke-Ebene aber laeuft weiter
    // (Befund Patrik S94): solange sie an ist (und ein Bild danach, damit sie
    // auch wieder verschwindet), wird neu gemischt, ohne die Clips zu rechnen.
    const bool karaoke = MultiEffectVisualizer::karaokeEbene();
    const bool clipsStehen = !laeuft && !neu && m_bildSteht && stelle == m_letzteStelle &&
                             m_groesse == m_letzteGroesse;
    if (!clipsStehen || karaoke || m_letzteKaraoke)
    {
        m_letzteKaraoke = karaoke;
        if (m_messung.bilder == 0)
        {
            m_laufzeit.start();
            m_messung.gpu = QString::fromLatin1(
                reinterpret_cast<const char*>(kontext->functions()->glGetString(GL_RENDERER)));
            m_messung.speicherFreiVorherKb = freierSpeicherKb(*kontext, &m_messung.speicherGesamtKb);
            m_messung.speicherFreiTiefstKb = m_messung.speicherFreiVorherKb;
        }
        QElapsedTimer uhr;
        uhr.start();
        // Steht die Uhr, laeuft auch die Zeit der Visuals nicht weiter.
        m_bildSteht = m_wiedergabe->rendere(stelle, (laeuft || clipsStehen) ? deltaTime : 0.0f,
                                            m_groesse, clipsStehen);
        const double ms = static_cast<double>(uhr.nsecsElapsed()) / 1.0e6;
        m_letzteStelle = stelle;
        m_letzteGroesse = m_groesse;
        ++m_gerechnet;
        versorgeNeue();

        // --- Messung (C2a)
        if (m_messung.bilder == 0)
        {
            m_messung.erstesBildMs = ms;
        }
        else
        {
            if (ms > m_messung.spaeterMaxMs)
            {
                m_messung.spaeterMaxMs = ms;
                m_messung.spaeterMaxStelleMs = stelle;
            }
            if (ms > Messung::kLangMs)
            {
                ++m_messung.langeGesamt;
                if (static_cast<int>(m_messung.lange.size()) < Messung::kLangeMax)
                    m_messung.lange.push_back({stelle, ms, m_wiedergabe->rendererAnzahl()});
            }
        }
        ++m_messung.bilder;
        m_messung.dauerSekunden = static_cast<double>(m_laufzeit.nsecsElapsed()) / 1.0e9;
        m_messung.rechnenSummeMs += ms;
        if (ms > m_messung.rechnenMaxMs)
        {
            m_messung.rechnenMaxMs = ms;
            m_messung.rechnenMaxStelleMs = stelle;
        }
        m_messung.groesse = m_groesse;
        m_messung.rendererMax = std::max(m_messung.rendererMax, m_wiedergabe->rendererAnzahl());
        if (m_messung.speicherFreiVorherKb >= 0)
        {
            const int frei = freierSpeicherKb(*kontext, nullptr);
            if (frei >= 0) m_messung.speicherFreiTiefstKb = std::min(m_messung.speicherFreiTiefstKb, frei);
        }
    }

    const unsigned int ziel =
        m_ziel >= 0 ? static_cast<unsigned int>(m_ziel) : kontext->defaultFramebufferObject();
    if (m_bildSteht)
    {
        QElapsedTimer uhr;
        uhr.start();
        m_wiedergabe->mischer().zeichneIn(ziel, m_groesse);
        const double ms = static_cast<double>(uhr.nsecsElapsed()) / 1.0e6;
        ++m_messung.gezeichnet;
        m_messung.zeichnenSummeMs += ms;
        m_messung.zeichnenMaxMs = std::max(m_messung.zeichnenMaxMs, ms);
    }
    else
    {
        QOpenGLFunctions* f = kontext->functions();
        f->glBindFramebuffer(GL_FRAMEBUFFER, ziel);
        f->glViewport(0, 0, m_groesse.width(), m_groesse.height());
        f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
    }
}

// =============================================================================
// Audiodaten
// =============================================================================

void Bildquelle::updateSpectrum(const float* spectrum, int count)
{
    if (spectrum == nullptr || count <= 0) return;
    m_spektrum.assign(spectrum, spectrum + count);
    if (m_wiedergabe == nullptr) return;
    m_wiedergabe->fuerJedenRenderer([&](MultiEffectVisualizer& r) {
        r.updateSpectrum(spectrum, count);
        ++m_audioGereicht;
    });
}

void Bildquelle::updateWaveform(const float* waveform, int count)
{
    if (waveform == nullptr || count <= 0) return;
    m_welle.assign(waveform, waveform + count);
    if (m_wiedergabe == nullptr) return;
    m_wiedergabe->fuerJedenRenderer([&](MultiEffectVisualizer& r) {
        r.updateWaveform(waveform, count);
        ++m_audioGereicht;
    });
}

void Bildquelle::updateAudioStereo(const float* specInterleaved, int binsPerCh,
                                   const float* waveInterleaved, int frames, int channels)
{
    const int kanaele = std::max(1, channels);
    if (specInterleaved != nullptr && binsPerCh > 0)
        m_spektrumStereo.assign(specInterleaved,
                                specInterleaved + static_cast<size_t>(binsPerCh) * kanaele);
    else
        m_spektrumStereo.clear();
    if (waveInterleaved != nullptr && frames > 0)
        m_welleStereo.assign(waveInterleaved, waveInterleaved + static_cast<size_t>(frames) * kanaele);
    else
        m_welleStereo.clear();
    m_stereoBins = binsPerCh;
    m_stereoFrames = frames;
    m_stereoKanaele = kanaele;
    if (m_wiedergabe == nullptr) return;
    m_wiedergabe->fuerJedenRenderer([&](MultiEffectVisualizer& r) {
        r.updateAudioStereo(specInterleaved, binsPerCh, waveInterleaved, frames, channels);
        ++m_audioGereicht;
    });
}

void Bildquelle::reicheAlles(MultiEffectVisualizer& renderer)
{
    if (!m_spektrum.empty())
    {
        renderer.updateSpectrum(m_spektrum.data(), static_cast<int>(m_spektrum.size()));
        ++m_audioGereicht;
    }
    if (!m_welle.empty())
    {
        renderer.updateWaveform(m_welle.data(), static_cast<int>(m_welle.size()));
        ++m_audioGereicht;
    }
    if (m_stereoKanaele > 0)
    {
        renderer.updateAudioStereo(m_spektrumStereo.empty() ? nullptr : m_spektrumStereo.data(),
                                   m_stereoBins,
                                   m_welleStereo.empty() ? nullptr : m_welleStereo.data(),
                                   m_stereoFrames, m_stereoKanaele);
        ++m_audioGereicht;
    }
}

void Bildquelle::versorgeNeue()
{
    std::vector<MultiEffectVisualizer*> jetzt;
    m_wiedergabe->fuerJedenRenderer([&](MultiEffectVisualizer& r) {
        jetzt.push_back(&r);
        if (std::find(m_versorgt.begin(), m_versorgt.end(), &r) == m_versorgt.end())
            reicheAlles(r);
    });
    m_versorgt = std::move(jetzt);
}

}  // namespace lumi::composer
