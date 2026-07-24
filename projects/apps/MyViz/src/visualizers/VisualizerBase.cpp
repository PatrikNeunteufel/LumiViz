/**
 ****************************************************************************************
 * @file   VisualizerBase.cpp
 * @brief  VisualizerBase implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/VisualizerBase.hpp"

#include <QMutexLocker>
#include <QDebug>

#include <algorithm>

// =============================================================================
// Construction
// =============================================================================

VisualizerBase::VisualizerBase(const QString& id,
                               const QString& name,
                               const QString& description)
    : m_id(id)
    , m_name(name)
    , m_description(description)
{
}

// =============================================================================
// IVisualizer Implementation
// =============================================================================

void VisualizerBase::initialize()
{
    qDebug() << "VisualizerBase::initialize() - m_initialized was:" << m_initialized;
    
    if (m_initialized)
    {
        qDebug() << "  Already initialized, returning";
        return;
    }

    qDebug() << "  Calling onInitialize()...";
    onInitialize();
    m_initialized = true;
    
    qDebug() << "  onInitialize() complete, m_initialized=" << m_initialized;
}

void VisualizerBase::render(float deltaTime)
{
    static int s_renderCount = 0;
    s_renderCount++;
    
    if (s_renderCount <= 5)
    {
        qDebug() << "VisualizerBase::render() - m_initialized=" << m_initialized 
                 << ", deltaTime=" << deltaTime;
    }
    
    if (!m_initialized)
    {
        if (s_renderCount <= 5)
        {
            qDebug() << "  NOT initialized, skipping render!";
        }
        return;
    }

    onRender(deltaTime);
}

void VisualizerBase::resize(const QSize& size)
{
    m_viewportSize = size;

    if (m_initialized)
    {
        onResize(size);
    }
}

void VisualizerBase::cleanup()
{
    if (!m_initialized)
    {
        return;
    }

    onCleanup();
    m_initialized = false;
}

// =============================================================================
// Audio Data (Thread-Safe)
// =============================================================================

void VisualizerBase::updateSpectrum(const float* spectrum, int count)
{
    QMutexLocker locker(&m_audioMutex);

    m_spectrum.assign(spectrum, spectrum + count);
    m_hasNewAudioData = true;
}

void VisualizerBase::updateWaveform(const float* waveform, int count)
{
    QMutexLocker locker(&m_audioMutex);

    m_waveform.assign(waveform, waveform + count);
    m_hasNewAudioData = true;
}

// =============================================================================
// Protected Accessors
// =============================================================================

float VisualizerBase::aspectRatio() const
{
    if (m_viewportSize.height() == 0)
    {
        return 1.0f;
    }
    return static_cast<float>(m_viewportSize.width()) / 
           static_cast<float>(m_viewportSize.height());
}

namespace
{
/// Mono fallback for stereo-only feeds (chain path/standalones feed only
/// updateAudioStereo): average L/R — mirrors the channel getters' mono
/// fallback in the other direction.
std::vector<float> mixToMono(const std::vector<float>& l, const std::vector<float>& r)
{
    const size_t n = std::min(l.size(), r.size());
    std::vector<float> mix(n);
    for (size_t i = 0; i < n; ++i) mix[i] = (l[i] + r[i]) * 0.5f;
    return mix;
}
}  // namespace

std::vector<float> VisualizerBase::getSpectrum() const
{
    QMutexLocker locker(&m_audioMutex);
    if (m_spectrum.empty() && !m_spectrumL.empty())
        return mixToMono(m_spectrumL, m_spectrumR);
    return m_spectrum;
}

std::vector<float> VisualizerBase::getWaveform() const
{
    QMutexLocker locker(&m_audioMutex);
    if (m_waveform.empty() && !m_waveformL.empty())
        return mixToMono(m_waveformL, m_waveformR);
    return m_waveform;
}

void VisualizerBase::updateAudioStereo(const float* specInterleaved, int binsPerCh,
                                       const float* waveInterleaved, int frames,
                                       int channels)
{
    QMutexLocker locker(&m_audioMutex);
    const int ch = channels < 1 ? 1 : channels;
    auto split = [ch](const float* src, int n, std::vector<float>& L,
                      std::vector<float>& R) {
        if (src == nullptr || n <= 0) { L.clear(); R.clear(); return; }
        L.resize(static_cast<size_t>(n));
        R.resize(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i)
        {
            L[static_cast<size_t>(i)] = src[i * ch];
            R[static_cast<size_t>(i)] = src[i * ch + (ch > 1 ? 1 : 0)];
        }
    };
    if (specInterleaved != nullptr && binsPerCh > 0)
    {
        split(specInterleaved, binsPerCh, m_spectrumL, m_spectrumR);
        m_hasNewAudioData = true;
    }
    if (waveInterleaved != nullptr && frames > 0)
    {
        split(waveInterleaved, frames, m_waveformL, m_waveformR);
        m_hasNewAudioData = true;
    }
}

std::vector<float> VisualizerBase::getSpectrumChannel(int channel) const
{
    QMutexLocker locker(&m_audioMutex);
    const std::vector<float>& src =
        channel == 1 ? m_spectrumR : m_spectrumL;
    return src.empty() ? m_spectrum : src;  // fall back to mono
}

std::vector<float> VisualizerBase::getWaveformChannel(int channel) const
{
    QMutexLocker locker(&m_audioMutex);
    const std::vector<float>& src =
        channel == 1 ? m_waveformR : m_waveformL;
    return src.empty() ? m_waveform : src;  // fall back to mono
}

bool VisualizerBase::hasNewAudioData()
{
    QMutexLocker locker(&m_audioMutex);

    bool result = m_hasNewAudioData;
    m_hasNewAudioData = false;
    return result;
}
