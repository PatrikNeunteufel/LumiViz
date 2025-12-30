/**
 ****************************************************************************************
 * @file   AudioAnalyzer.cpp
 * @brief  Audio Analyzer Service Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "pch.h"
#include "audio/AudioAnalyzer.hpp"
#include "audio/IAudioEngine.hpp"
#include "audio/AudioPlayer.hpp"  // Concrete type for currentStream()
#include "services/IEventBus.hpp"

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <deque>

// =============================================================================
// Private Implementation
// =============================================================================

struct AudioAnalyzer::Impl
{
    IAudioEngine& engine;
    AudioPlayer& player;
    IEventBus& eventBus;
    
    // Configuration
    IAudioAnalyzer::FFTSize fftSize = IAudioAnalyzer::FFTSize::Size1024;
    AudioAnalyzer::NormalizationMode normMode = AudioAnalyzer::NormalizationMode::Logarithmic;
    float smoothingFactor = 0.7f;
    float beatSensitivity = 0.5f;
    
    // State
    bool enabled = true;
    bool beatDetectionEnabled = true;
    
    // Raw data buffers
    std::vector<float> rawSpectrum;
    std::vector<float> smoothedSpectrumBuffer;
    std::vector<float> rawWaveform;
    
    // Levels
    float levelL = 0.0f;
    float levelR = 0.0f;
    float smoothedLevelL = 0.0f;
    float smoothedLevelR = 0.0f;
    
    // Frequency bands (smoothed)
    FrequencyBands currentBands;
    FrequencyBands smoothedBandsBuffer;
    
    // Beat detection
    BeatInfo currentBeat;
    std::deque<float> energyHistory;  // For rolling average
    std::deque<std::uint64_t> beatTimestamps;  // For BPM calculation
    float lastBassEnergy = 0.0f;
    static constexpr size_t ENERGY_HISTORY_SIZE = 43;  // ~1 second at 43 fps
    static constexpr size_t BPM_HISTORY_SIZE = 10;
    
    explicit Impl(IAudioEngine& eng, AudioPlayer& pl, IEventBus& bus)
        : engine(eng), player(pl), eventBus(bus)
    {
        int size = static_cast<int>(fftSize);
        rawSpectrum.resize(size / 2);  // FFT returns size/2 useful bins
        smoothedSpectrumBuffer.resize(size / 2);
        rawWaveform.resize(size);
    }
};

// =============================================================================
// Helper Functions
// =============================================================================

namespace
{

/**
 * @brief Convert linear value to logarithmic (dB-like) scale
 */
float toLogScale(float value)
{
    if (value <= 0.0f) return 0.0f;
    // Convert to dB-like scale, normalize to 0-1
    float db = 20.0f * std::log10(value);
    // Assume -60dB floor, 0dB ceiling
    return std::clamp((db + 60.0f) / 60.0f, 0.0f, 1.0f);
}

/**
 * @brief Get current timestamp in milliseconds
 */
std::uint64_t getTimestampMs()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()
    ).count();
}

} // anonymous namespace

// =============================================================================
// Construction / Destruction
// =============================================================================

AudioAnalyzer::AudioAnalyzer(IAudioEngine& engine,
                             AudioPlayer& player,
                             IEventBus& eventBus,
                             QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>(engine, player, eventBus))
{
    BasicLogger::logDebug("AudioAnalyzer created");
}

AudioAnalyzer::~AudioAnalyzer()
{
    BasicLogger::logDebug("AudioAnalyzer destroyed");
}

// =============================================================================
// FFT Size Configuration
// =============================================================================

IAudioAnalyzer::FFTSize AudioAnalyzer::fftSize() const
{
    return m_impl->fftSize;
}

void AudioAnalyzer::setFFTSize(FFTSize size)
{
    if (m_impl->fftSize == size) return;
    
    m_impl->fftSize = size;
    int intSize = static_cast<int>(size);
    m_impl->rawSpectrum.resize(intSize / 2);
    m_impl->smoothedSpectrumBuffer.resize(intSize / 2);
    m_impl->rawWaveform.resize(intSize);
    
    // Clear smoothed data
    std::fill(m_impl->smoothedSpectrumBuffer.begin(), 
              m_impl->smoothedSpectrumBuffer.end(), 0.0f);
}

// =============================================================================
// Spectrum Analysis
// =============================================================================

SpectrumData AudioAnalyzer::spectrum() const
{
    SpectrumData data;
    data.magnitudes = m_impl->rawSpectrum;
    data.binCount = m_impl->rawSpectrum.size();
    
    int sampleRate = m_impl->engine.getSampleRate();
    int fftSizeInt = static_cast<int>(m_impl->fftSize);
    data.frequencyResolution = static_cast<float>(sampleRate) / fftSizeInt;
    
    // Find peak
    auto maxIt = std::max_element(data.magnitudes.begin(), data.magnitudes.end());
    if (maxIt != data.magnitudes.end())
    {
        data.peakMagnitude = *maxIt;
        int binIndex = static_cast<int>(std::distance(data.magnitudes.begin(), maxIt));
        data.peakFrequency = binIndex * data.frequencyResolution;
    }
    
    return data;
}

std::size_t AudioAnalyzer::getSpectrum(float* data, std::size_t maxSize) const
{
    std::size_t count = std::min(maxSize, m_impl->rawSpectrum.size());
    std::copy_n(m_impl->rawSpectrum.begin(), count, data);
    return count;
}

std::vector<float> AudioAnalyzer::smoothedSpectrum(float smoothing) const
{
    std::vector<float> result = m_impl->smoothedSpectrumBuffer;
    
    // Apply additional smoothing if requested
    if (smoothing != m_impl->smoothingFactor && smoothing > 0.0f)
    {
        for (size_t i = 0; i < result.size() && i < m_impl->rawSpectrum.size(); i++)
        {
            result[i] = smoothing * result[i] + (1.0f - smoothing) * m_impl->rawSpectrum[i];
        }
    }
    
    return result;
}

// =============================================================================
// Frequency Bands
// =============================================================================

FrequencyBands AudioAnalyzer::frequencyBands() const
{
    return m_impl->currentBands;
}

FrequencyBands AudioAnalyzer::smoothedBands(float smoothing) const
{
    if (smoothing == m_impl->smoothingFactor)
    {
        return m_impl->smoothedBandsBuffer;
    }
    
    // Apply custom smoothing
    FrequencyBands result;
    result.sub = smoothing * m_impl->smoothedBandsBuffer.sub + 
                 (1.0f - smoothing) * m_impl->currentBands.sub;
    result.bass = smoothing * m_impl->smoothedBandsBuffer.bass + 
                  (1.0f - smoothing) * m_impl->currentBands.bass;
    result.lowMid = smoothing * m_impl->smoothedBandsBuffer.lowMid + 
                    (1.0f - smoothing) * m_impl->currentBands.lowMid;
    result.mid = smoothing * m_impl->smoothedBandsBuffer.mid + 
                 (1.0f - smoothing) * m_impl->currentBands.mid;
    result.highMid = smoothing * m_impl->smoothedBandsBuffer.highMid + 
                     (1.0f - smoothing) * m_impl->currentBands.highMid;
    result.high = smoothing * m_impl->smoothedBandsBuffer.high + 
                  (1.0f - smoothing) * m_impl->currentBands.high;
    
    return result;
}

float AudioAnalyzer::bandLevel(float lowFreq, float highFreq) const
{
    int lowBin = frequencyToBin(lowFreq);
    int highBin = frequencyToBin(highFreq);
    return computeBandLevel(lowBin, highBin);
}

// =============================================================================
// Waveform
// =============================================================================

std::size_t AudioAnalyzer::getWaveform(float* data, std::size_t maxSize) const
{
    std::size_t count = std::min(maxSize, m_impl->rawWaveform.size());
    std::copy_n(m_impl->rawWaveform.begin(), count, data);
    return count;
}

std::vector<float> AudioAnalyzer::waveform() const
{
    return m_impl->rawWaveform;
}

// =============================================================================
// Level Metering
// =============================================================================

float AudioAnalyzer::levelLeft() const
{
    return m_impl->smoothedLevelL;
}

float AudioAnalyzer::levelRight() const
{
    return m_impl->smoothedLevelR;
}

float AudioAnalyzer::levelMono() const
{
    return (m_impl->smoothedLevelL + m_impl->smoothedLevelR) * 0.5f;
}

float AudioAnalyzer::levelPeak() const
{
    return std::max(m_impl->smoothedLevelL, m_impl->smoothedLevelR);
}

// =============================================================================
// Beat Detection
// =============================================================================

BeatInfo AudioAnalyzer::beatInfo() const
{
    return m_impl->currentBeat;
}

bool AudioAnalyzer::isBeat() const
{
    return m_impl->currentBeat.detected;
}

float AudioAnalyzer::beatIntensity() const
{
    return m_impl->currentBeat.intensity;
}

float AudioAnalyzer::bpm() const
{
    return m_impl->currentBeat.bpm;
}

void AudioAnalyzer::setBeatSensitivity(float sensitivity)
{
    m_impl->beatSensitivity = std::clamp(sensitivity, 0.0f, 1.0f);
}

// =============================================================================
// Processing
// =============================================================================

void AudioAnalyzer::update()
{
    if (!m_impl->enabled) return;
    
    AudioStreamHandle stream = m_impl->player.currentStream();
    if (stream == INVALID_STREAM || !m_impl->player.isPlaying())
    {
        return;
    }
    
    updateSpectrum();
    updateWaveform();
    updateLevels();
    
    if (m_impl->beatDetectionEnabled)
    {
        detectBeat();
    }
    
    publishAudioDataEvent();
}

void AudioAnalyzer::reset()
{
    std::fill(m_impl->rawSpectrum.begin(), m_impl->rawSpectrum.end(), 0.0f);
    std::fill(m_impl->smoothedSpectrumBuffer.begin(), 
              m_impl->smoothedSpectrumBuffer.end(), 0.0f);
    std::fill(m_impl->rawWaveform.begin(), m_impl->rawWaveform.end(), 0.0f);
    
    m_impl->levelL = m_impl->levelR = 0.0f;
    m_impl->smoothedLevelL = m_impl->smoothedLevelR = 0.0f;
    m_impl->currentBands = FrequencyBands{};
    m_impl->smoothedBandsBuffer = FrequencyBands{};
    m_impl->currentBeat = BeatInfo{};
    m_impl->energyHistory.clear();
    m_impl->beatTimestamps.clear();
    m_impl->lastBassEnergy = 0.0f;
}

// =============================================================================
// Analysis Control
// =============================================================================

bool AudioAnalyzer::isEnabled() const
{
    return m_impl->enabled;
}

void AudioAnalyzer::setEnabled(bool enabled)
{
    m_impl->enabled = enabled;
    if (!enabled)
    {
        reset();
    }
}

bool AudioAnalyzer::isBeatDetectionEnabled() const
{
    return m_impl->beatDetectionEnabled;
}

void AudioAnalyzer::setBeatDetectionEnabled(bool enabled)
{
    m_impl->beatDetectionEnabled = enabled;
}

// =============================================================================
// Additional Methods
// =============================================================================

void AudioAnalyzer::setSmoothingFactor(float factor)
{
    m_impl->smoothingFactor = std::clamp(factor, 0.0f, 1.0f);
}

float AudioAnalyzer::smoothingFactor() const
{
    return m_impl->smoothingFactor;
}

void AudioAnalyzer::setNormalizationMode(NormalizationMode mode)
{
    m_impl->normMode = mode;
}

AudioAnalyzer::NormalizationMode AudioAnalyzer::normalizationMode() const
{
    return m_impl->normMode;
}

// =============================================================================
// Private Methods
// =============================================================================

void AudioAnalyzer::updateSpectrum()
{
    AudioStreamHandle stream = m_impl->player.currentStream();
    int size = static_cast<int>(m_impl->fftSize);
    
    // Get raw FFT data
    std::vector<float> fftData(size);
    if (!m_impl->engine.getFFTData(stream, fftData.data(), size))
    {
        return;
    }
    
    // Process and normalize
    size_t binCount = m_impl->rawSpectrum.size();
    float maxVal = 0.0001f;  // Avoid division by zero
    
    for (size_t i = 0; i < binCount && i < fftData.size(); i++)
    {
        float val = fftData[i];
        
        // Apply normalization
        switch (m_impl->normMode)
        {
            case NormalizationMode::Logarithmic:
                val = toLogScale(val);
                break;
            case NormalizationMode::Peak:
                maxVal = std::max(maxVal, val);
                break;
            case NormalizationMode::RMS:
            case NormalizationMode::None:
            default:
                break;
        }
        
        m_impl->rawSpectrum[i] = val;
    }
    
    // Peak normalization (second pass)
    if (m_impl->normMode == NormalizationMode::Peak && maxVal > 0.0f)
    {
        for (float& val : m_impl->rawSpectrum)
        {
            val /= maxVal;
        }
    }
    
    // Apply smoothing
    float s = m_impl->smoothingFactor;
    for (size_t i = 0; i < binCount; i++)
    {
        m_impl->smoothedSpectrumBuffer[i] = 
            s * m_impl->smoothedSpectrumBuffer[i] + (1.0f - s) * m_impl->rawSpectrum[i];
    }
    
    // Update frequency bands
    m_impl->currentBands.sub = computeBandLevel(frequencyToBin(20), frequencyToBin(60));
    m_impl->currentBands.bass = computeBandLevel(frequencyToBin(60), frequencyToBin(250));
    m_impl->currentBands.lowMid = computeBandLevel(frequencyToBin(250), frequencyToBin(500));
    m_impl->currentBands.mid = computeBandLevel(frequencyToBin(500), frequencyToBin(2000));
    m_impl->currentBands.highMid = computeBandLevel(frequencyToBin(2000), frequencyToBin(4000));
    m_impl->currentBands.high = computeBandLevel(frequencyToBin(4000), frequencyToBin(20000));
    
    // Smooth bands
    m_impl->smoothedBandsBuffer.sub = s * m_impl->smoothedBandsBuffer.sub + 
                                      (1.0f - s) * m_impl->currentBands.sub;
    m_impl->smoothedBandsBuffer.bass = s * m_impl->smoothedBandsBuffer.bass + 
                                       (1.0f - s) * m_impl->currentBands.bass;
    m_impl->smoothedBandsBuffer.lowMid = s * m_impl->smoothedBandsBuffer.lowMid + 
                                         (1.0f - s) * m_impl->currentBands.lowMid;
    m_impl->smoothedBandsBuffer.mid = s * m_impl->smoothedBandsBuffer.mid + 
                                      (1.0f - s) * m_impl->currentBands.mid;
    m_impl->smoothedBandsBuffer.highMid = s * m_impl->smoothedBandsBuffer.highMid + 
                                          (1.0f - s) * m_impl->currentBands.highMid;
    m_impl->smoothedBandsBuffer.high = s * m_impl->smoothedBandsBuffer.high + 
                                       (1.0f - s) * m_impl->currentBands.high;
    
    emit spectrumUpdated(m_impl->smoothedSpectrumBuffer);
}

void AudioAnalyzer::updateWaveform()
{
    AudioStreamHandle stream = m_impl->player.currentStream();
    int size = static_cast<int>(m_impl->rawWaveform.size());
    
    m_impl->engine.getWaveformData(stream, m_impl->rawWaveform.data(), size);
}

void AudioAnalyzer::updateLevels()
{
    AudioStreamHandle stream = m_impl->player.currentStream();
    
    if (m_impl->engine.getChannelLevels(stream, m_impl->levelL, m_impl->levelR))
    {
        float s = m_impl->smoothingFactor;
        m_impl->smoothedLevelL = s * m_impl->smoothedLevelL + (1.0f - s) * m_impl->levelL;
        m_impl->smoothedLevelR = s * m_impl->smoothedLevelR + (1.0f - s) * m_impl->levelR;
        
        emit levelsUpdated(m_impl->smoothedLevelL, m_impl->smoothedLevelR);
    }
}

void AudioAnalyzer::detectBeat()
{
    // Simple energy-based beat detection
    // Use bass + sub frequencies for beat detection
    float bassEnergy = m_impl->currentBands.sub + m_impl->currentBands.bass;
    
    // Add to history
    m_impl->energyHistory.push_back(bassEnergy);
    if (m_impl->energyHistory.size() > Impl::ENERGY_HISTORY_SIZE)
    {
        m_impl->energyHistory.pop_front();
    }
    
    // Calculate average energy
    float avgEnergy = 0.0f;
    if (!m_impl->energyHistory.empty())
    {
        avgEnergy = std::accumulate(m_impl->energyHistory.begin(), 
                                    m_impl->energyHistory.end(), 0.0f) / 
                    m_impl->energyHistory.size();
    }
    
    // Threshold based on sensitivity
    // Lower sensitivity = higher threshold = fewer beats detected
    float threshold = avgEnergy * (1.3f + m_impl->beatSensitivity);
    
    // Detect beat: current energy exceeds threshold and is rising
    bool beatDetected = (bassEnergy > threshold) && 
                        (bassEnergy > m_impl->lastBassEnergy * 1.1f) &&
                        (m_impl->energyHistory.size() >= 5);
    
    m_impl->currentBeat.detected = beatDetected;
    m_impl->currentBeat.intensity = std::min(bassEnergy / std::max(avgEnergy, 0.001f), 2.0f) / 2.0f;
    
    if (beatDetected)
    {
        m_impl->currentBeat.beatNumber++;
        
        // Record timestamp for BPM calculation
        std::uint64_t now = getTimestampMs();
        m_impl->currentBeat.timestampMs = now;
        m_impl->beatTimestamps.push_back(now);
        
        if (m_impl->beatTimestamps.size() > Impl::BPM_HISTORY_SIZE)
        {
            m_impl->beatTimestamps.pop_front();
        }
        
        // Calculate BPM from beat intervals
        if (m_impl->beatTimestamps.size() >= 3)
        {
            std::uint64_t totalInterval = m_impl->beatTimestamps.back() - 
                                          m_impl->beatTimestamps.front();
            size_t beatCount = m_impl->beatTimestamps.size() - 1;
            float avgInterval = static_cast<float>(totalInterval) / beatCount;
            
            if (avgInterval > 0)
            {
                m_impl->currentBeat.bpm = 60000.0f / avgInterval;
                // Clamp to reasonable BPM range
                m_impl->currentBeat.bpm = std::clamp(m_impl->currentBeat.bpm, 60.0f, 200.0f);
            }
        }
        
        emit onBeatDetected(m_impl->currentBeat.intensity);
    }
    
    m_impl->lastBassEnergy = bassEnergy;
}

void AudioAnalyzer::publishAudioDataEvent()
{
    AudioDataEvent event;
    event.spectrum = m_impl->smoothedSpectrumBuffer;
    event.waveform = m_impl->rawWaveform;
    event.levelLeft = m_impl->smoothedLevelL;
    event.levelRight = m_impl->smoothedLevelR;
    event.beatDetected = m_impl->currentBeat.detected;
    event.timestampMs = getTimestampMs();
    
    m_impl->eventBus.publish(event);
}

float AudioAnalyzer::computeBandLevel(int lowBin, int highBin) const
{
    if (lowBin < 0) lowBin = 0;
    if (highBin >= static_cast<int>(m_impl->rawSpectrum.size()))
    {
        highBin = static_cast<int>(m_impl->rawSpectrum.size()) - 1;
    }
    if (lowBin > highBin) return 0.0f;
    
    float sum = 0.0f;
    for (int i = lowBin; i <= highBin; i++)
    {
        sum += m_impl->rawSpectrum[i];
    }
    
    return sum / (highBin - lowBin + 1);
}

int AudioAnalyzer::frequencyToBin(float frequency) const
{
    int sampleRate = m_impl->engine.getSampleRate();
    int fftSizeInt = static_cast<int>(m_impl->fftSize);
    float binWidth = static_cast<float>(sampleRate) / fftSizeInt;
    
    return static_cast<int>(frequency / binWidth);
}
