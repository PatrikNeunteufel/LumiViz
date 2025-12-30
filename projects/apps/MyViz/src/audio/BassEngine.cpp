/**
 ****************************************************************************************
 * @file   BassEngine.cpp
 * @brief  BASS Library Audio Engine Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "pch.h"
#include "audio/BassEngine.hpp"

#include <BasicLogger.h>

// BASS Library
#include <bass.h>
// Note: basswasapi.h would be needed for loopback capture (future feature)

#include <QDir>
#include <QFileInfo>

#include <cmath>
#include <algorithm>

// =============================================================================
// Private Implementation
// =============================================================================

struct BassEngine::Impl
{
    bool initialized = false;
    int deviceId = -1;
    int sampleRate = 44100;
    int lastError = 0;
    QString lastErrorMsg;
    
    // Loaded plugins
    std::vector<HPLUGIN> plugins;
    
    // Supported extensions (built-in + plugins)
    QStringList extensions = {"mp3", "wav", "ogg", "aiff"};
};

// =============================================================================
// Helper Functions
// =============================================================================

namespace
{

/**
 * @brief Convert BASS error code to string
 */
QString bassErrorToString(int error)
{
    switch (error)
    {
        case BASS_OK:               return "OK";
        case BASS_ERROR_MEM:        return "Memory error";
        case BASS_ERROR_FILEOPEN:   return "Cannot open file";
        case BASS_ERROR_DRIVER:     return "Cannot find driver";
        case BASS_ERROR_BUFLOST:    return "Buffer lost";
        case BASS_ERROR_HANDLE:     return "Invalid handle";
        case BASS_ERROR_FORMAT:     return "Unsupported format";
        case BASS_ERROR_POSITION:   return "Invalid position";
        case BASS_ERROR_INIT:       return "BASS not initialized";
        case BASS_ERROR_START:      return "Output not started";
        case BASS_ERROR_ALREADY:    return "Already initialized";
        case BASS_ERROR_NOTAUDIO:   return "Not audio content";
        case BASS_ERROR_NOCHAN:     return "Cannot get channel";
        case BASS_ERROR_ILLTYPE:    return "Illegal type";
        case BASS_ERROR_ILLPARAM:   return "Illegal parameter";
        case BASS_ERROR_NO3D:       return "No 3D support";
        case BASS_ERROR_NOEAX:      return "No EAX support";
        case BASS_ERROR_DEVICE:     return "Illegal device number";
        case BASS_ERROR_NOPLAY:     return "Not playing";
        case BASS_ERROR_FREQ:       return "Illegal sample rate";
        case BASS_ERROR_NOTFILE:    return "Not a file stream";
        case BASS_ERROR_NOHW:       return "No hardware support";
        case BASS_ERROR_EMPTY:      return "Empty";
        case BASS_ERROR_NONET:      return "No internet connection";
        case BASS_ERROR_CREATE:     return "Cannot create file";
        case BASS_ERROR_NOFX:       return "Effects not available";
        case BASS_ERROR_NOTAVAIL:   return "Not available";
        case BASS_ERROR_DECODE:     return "Cannot decode";
        case BASS_ERROR_DX:         return "DirectX error";
        case BASS_ERROR_TIMEOUT:    return "Timeout";
        case BASS_ERROR_FILEFORM:   return "Unsupported file format";
        case BASS_ERROR_SPEAKER:    return "Unavailable speaker";
        case BASS_ERROR_VERSION:    return "Invalid version";
        case BASS_ERROR_CODEC:      return "Codec not available";
        case BASS_ERROR_ENDED:      return "Stream ended";
        case BASS_ERROR_UNKNOWN:    
        default:                    return "Unknown error";
    }
}

/**
 * @brief Get FFT flag for BASS based on size
 */
DWORD fftSizeToFlag(int size)
{
    switch (size)
    {
        case 256:   return BASS_DATA_FFT256;
        case 512:   return BASS_DATA_FFT512;
        case 1024:  return BASS_DATA_FFT1024;
        case 2048:  return BASS_DATA_FFT2048;
        case 4096:  return BASS_DATA_FFT4096;
        case 8192:  return BASS_DATA_FFT8192;
        default:    return BASS_DATA_FFT1024;
    }
}

} // anonymous namespace

// =============================================================================
// Construction / Destruction
// =============================================================================

BassEngine::BassEngine()
    : m_impl(std::make_unique<Impl>())
{
    BasicLogger::logDebug("BassEngine created");
}

BassEngine::~BassEngine()
{
    shutdown();
    BasicLogger::logDebug("BassEngine destroyed");
}

// =============================================================================
// Initialization
// =============================================================================

bool BassEngine::initialize(int deviceId, int sampleRate)
{
    if (m_impl->initialized)
    {
        BasicLogger::logWarning("BassEngine already initialized");
        return true;
    }

    BasicLogger::logInfo("Initializing BASS audio engine...");
    
    // Check BASS version
    DWORD version = BASS_GetVersion();
    BasicLogger::logInfo("BASS version: " + 
        std::to_string(HIBYTE(HIWORD(version))) + "." +
        std::to_string(LOBYTE(HIWORD(version))) + "." +
        std::to_string(HIBYTE(LOWORD(version))) + "." +
        std::to_string(LOBYTE(LOWORD(version))));

    // Use -1 for default device, otherwise use specified device
    int device = (deviceId < 0) ? -1 : deviceId;
    
    // Initialize BASS
    // BASS_DEVICE_FREQ: Use device's default sample rate
    // BASS_DEVICE_LATENCY: Calculate device latency
    if (!BASS_Init(device, sampleRate, BASS_DEVICE_LATENCY, nullptr, nullptr))
    {
        m_impl->lastError = BASS_ErrorGetCode();
        m_impl->lastErrorMsg = bassErrorToString(m_impl->lastError);
        BasicLogger::logError("BASS_Init failed: " + m_impl->lastErrorMsg.toStdString());
        return false;
    }
    
    m_impl->initialized = true;
    m_impl->deviceId = device;
    m_impl->sampleRate = sampleRate;
    
    // Get actual device info
    BASS_INFO info;
    if (BASS_GetInfo(&info))
    {
        BasicLogger::logInfo("Audio device initialized:");
        BasicLogger::logInfo("  Sample rate: " + std::to_string(info.freq) + " Hz");
        BasicLogger::logInfo("  Speakers: " + std::to_string(info.speakers));
        BasicLogger::logInfo("  Latency: " + std::to_string(info.latency) + " ms");
    }
    
    BasicLogger::logInfo("BASS audio engine initialized successfully");
    return true;
}

void BassEngine::shutdown()
{
    if (!m_impl->initialized)
    {
        return;
    }
    
    BasicLogger::logInfo("Shutting down BASS audio engine...");
    
    // Free all plugins
    for (HPLUGIN plugin : m_impl->plugins)
    {
        BASS_PluginFree(plugin);
    }
    m_impl->plugins.clear();
    
    // Free BASS
    BASS_Free();
    
    m_impl->initialized = false;
    m_impl->deviceId = -1;
    
    BasicLogger::logInfo("BASS audio engine shut down");
}

bool BassEngine::isInitialized() const
{
    return m_impl->initialized;
}

// =============================================================================
// Device Management
// =============================================================================

std::vector<AudioDeviceInfo> BassEngine::getDevices() const
{
    std::vector<AudioDeviceInfo> devices;
    
    BASS_DEVICEINFO info;
    for (int i = 0; BASS_GetDeviceInfo(i, &info); i++)
    {
        AudioDeviceInfo device;
        device.id = i;
        device.name = QString::fromLocal8Bit(info.name);
        device.driver = QString::fromLocal8Bit(info.driver ? info.driver : "");
        device.isDefault = (info.flags & BASS_DEVICE_DEFAULT) != 0;
        device.isEnabled = (info.flags & BASS_DEVICE_ENABLED) != 0;
        device.isLoopback = (info.flags & BASS_DEVICE_LOOPBACK) != 0;
        device.sampleRate = m_impl->sampleRate;
        device.channels = 2;  // BASS doesn't provide this directly
        
        devices.push_back(device);
    }
    
    return devices;
}

int BassEngine::getCurrentDevice() const
{
    return m_impl->deviceId;
}

bool BassEngine::setDevice(int deviceId)
{
    if (!m_impl->initialized)
    {
        m_impl->lastErrorMsg = "Engine not initialized";
        return false;
    }
    
    if (!BASS_SetDevice(deviceId))
    {
        m_impl->lastError = BASS_ErrorGetCode();
        m_impl->lastErrorMsg = bassErrorToString(m_impl->lastError);
        return false;
    }
    
    m_impl->deviceId = deviceId;
    return true;
}

int BassEngine::getSampleRate() const
{
    return m_impl->sampleRate;
}

// =============================================================================
// Stream Management
// =============================================================================

AudioStreamHandle BassEngine::createStream(const QString& filePath)
{
    if (!m_impl->initialized)
    {
        m_impl->lastErrorMsg = "Engine not initialized";
        return INVALID_STREAM;
    }
    
    // Convert to local 8-bit encoding for BASS
    std::string path = filePath.toLocal8Bit().constData();
    
    // Create stream with float output for better quality
    // BASS_STREAM_PRESCAN: Pre-scan for accurate seeking
    // BASS_SAMPLE_FLOAT: Use floating-point sample data
    HSTREAM stream = BASS_StreamCreateFile(
        FALSE,              // Not from memory
        path.c_str(),       // File path
        0,                  // Offset
        0,                  // Length (0 = entire file)
        BASS_SAMPLE_FLOAT | BASS_STREAM_PRESCAN
    );
    
    if (stream == 0)
    {
        m_impl->lastError = BASS_ErrorGetCode();
        m_impl->lastErrorMsg = bassErrorToString(m_impl->lastError);
        BasicLogger::logError("Failed to create stream: " + 
            m_impl->lastErrorMsg.toStdString() + " - " + filePath.toStdString());
        return INVALID_STREAM;
    }
    
    BasicLogger::logDebug("Created stream for: " + filePath.toStdString());
    return static_cast<AudioStreamHandle>(stream);
}

AudioStreamHandle BassEngine::createLoopbackStream()
{
    if (!m_impl->initialized)
    {
        m_impl->lastErrorMsg = "Engine not initialized";
        return INVALID_STREAM;
    }
    
    // Find loopback device
    BASS_DEVICEINFO info;
    int loopbackDevice = -1;
    
    for (int i = 0; BASS_GetDeviceInfo(i, &info); i++)
    {
        if ((info.flags & BASS_DEVICE_LOOPBACK) && (info.flags & BASS_DEVICE_ENABLED))
        {
            loopbackDevice = i;
            break;
        }
    }
    
    if (loopbackDevice < 0)
    {
        m_impl->lastErrorMsg = "No loopback device found";
        BasicLogger::logWarning("No loopback device available");
        return INVALID_STREAM;
    }
    
    // Initialize loopback recording
    // Note: This requires BASSWASAPI on Windows
    BasicLogger::logInfo("Loopback capture not fully implemented yet");
    m_impl->lastErrorMsg = "Loopback not implemented";
    return INVALID_STREAM;
}

void BassEngine::freeStream(AudioStreamHandle stream)
{
    if (stream != INVALID_STREAM)
    {
        BASS_StreamFree(static_cast<HSTREAM>(stream));
    }
}

// =============================================================================
// Playback Control
// =============================================================================

bool BassEngine::play(AudioStreamHandle stream)
{
    if (stream == INVALID_STREAM) return false;
    
    // FALSE = don't restart if already playing
    return BASS_ChannelPlay(static_cast<HSTREAM>(stream), FALSE) != 0;
}

bool BassEngine::pause(AudioStreamHandle stream)
{
    if (stream == INVALID_STREAM) return false;
    
    return BASS_ChannelPause(static_cast<HSTREAM>(stream)) != 0;
}

bool BassEngine::stop(AudioStreamHandle stream)
{
    if (stream == INVALID_STREAM) return false;
    
    BASS_ChannelStop(static_cast<HSTREAM>(stream));
    // Reset position to start
    BASS_ChannelSetPosition(static_cast<HSTREAM>(stream), 0, BASS_POS_BYTE);
    return true;
}

bool BassEngine::isPlaying(AudioStreamHandle stream) const
{
    if (stream == INVALID_STREAM) return false;
    
    return BASS_ChannelIsActive(static_cast<HSTREAM>(stream)) == BASS_ACTIVE_PLAYING;
}

bool BassEngine::isPaused(AudioStreamHandle stream) const
{
    if (stream == INVALID_STREAM) return false;
    
    return BASS_ChannelIsActive(static_cast<HSTREAM>(stream)) == BASS_ACTIVE_PAUSED;
}

// =============================================================================
// Stream Properties
// =============================================================================

int BassEngine::getPositionMs(AudioStreamHandle stream) const
{
    if (stream == INVALID_STREAM) return 0;
    
    QWORD bytes = BASS_ChannelGetPosition(static_cast<HSTREAM>(stream), BASS_POS_BYTE);
    double seconds = BASS_ChannelBytes2Seconds(static_cast<HSTREAM>(stream), bytes);
    return static_cast<int>(seconds * 1000.0);
}

bool BassEngine::setPositionMs(AudioStreamHandle stream, int positionMs)
{
    if (stream == INVALID_STREAM) return false;
    
    double seconds = positionMs / 1000.0;
    QWORD bytes = BASS_ChannelSeconds2Bytes(static_cast<HSTREAM>(stream), seconds);
    return BASS_ChannelSetPosition(static_cast<HSTREAM>(stream), bytes, BASS_POS_BYTE) != 0;
}

int BassEngine::getDurationMs(AudioStreamHandle stream) const
{
    if (stream == INVALID_STREAM) return 0;
    
    QWORD bytes = BASS_ChannelGetLength(static_cast<HSTREAM>(stream), BASS_POS_BYTE);
    double seconds = BASS_ChannelBytes2Seconds(static_cast<HSTREAM>(stream), bytes);
    return static_cast<int>(seconds * 1000.0);
}

float BassEngine::getVolume(AudioStreamHandle stream) const
{
    if (stream == INVALID_STREAM) return 0.0f;
    
    float volume = 0.0f;
    BASS_ChannelGetAttribute(static_cast<HSTREAM>(stream), BASS_ATTRIB_VOL, &volume);
    return volume;
}

bool BassEngine::setVolume(AudioStreamHandle stream, float volume)
{
    if (stream == INVALID_STREAM) return false;
    
    volume = std::clamp(volume, 0.0f, 1.0f);
    return BASS_ChannelSetAttribute(static_cast<HSTREAM>(stream), BASS_ATTRIB_VOL, volume) != 0;
}

// =============================================================================
// FFT / Audio Analysis
// =============================================================================

bool BassEngine::getFFTData(AudioStreamHandle stream, float* data, int size)
{
    if (stream == INVALID_STREAM || data == nullptr) return false;
    
    DWORD flag = fftSizeToFlag(size);
    
    // Get FFT data
    // BASS returns actual float values, not indices
    int result = BASS_ChannelGetData(static_cast<HSTREAM>(stream), data, flag);
    
    if (result == -1)
    {
        m_impl->lastError = BASS_ErrorGetCode();
        return false;
    }
    
    return true;
}

bool BassEngine::getWaveformData(AudioStreamHandle stream, float* data, int size)
{
    if (stream == INVALID_STREAM || data == nullptr) return false;
    
    // Request float samples
    int bytesNeeded = size * sizeof(float);
    int result = BASS_ChannelGetData(
        static_cast<HSTREAM>(stream), 
        data, 
        bytesNeeded | BASS_DATA_FLOAT
    );
    
    if (result == -1)
    {
        m_impl->lastError = BASS_ErrorGetCode();
        return false;
    }
    
    return true;
}

bool BassEngine::getChannelLevels(AudioStreamHandle stream, float& left, float& right)
{
    if (stream == INVALID_STREAM) return false;
    
    // Get stereo levels
    DWORD level = BASS_ChannelGetLevel(static_cast<HSTREAM>(stream));
    
    if (level == static_cast<DWORD>(-1))
    {
        m_impl->lastError = BASS_ErrorGetCode();
        return false;
    }
    
    // LOWORD = left channel, HIWORD = right channel
    // Values are 0-32768
    left = static_cast<float>(LOWORD(level)) / 32768.0f;
    right = static_cast<float>(HIWORD(level)) / 32768.0f;
    
    return true;
}

// =============================================================================
// Metadata
// =============================================================================

bool BassEngine::getMetadata(AudioStreamHandle stream,
                             QString& title,
                             QString& artist,
                             QString& album)
{
    if (stream == INVALID_STREAM) return false;
    
    // Try to get ID3v2 tags first, then ID3v1
    const char* tags = BASS_ChannelGetTags(static_cast<HSTREAM>(stream), BASS_TAG_ID3V2);
    
    if (tags == nullptr)
    {
        // Try OGG comments
        tags = BASS_ChannelGetTags(static_cast<HSTREAM>(stream), BASS_TAG_OGG);
    }
    
    // For now, return empty - full tag parsing would require more code
    title.clear();
    artist.clear();
    album.clear();
    
    // Simple fallback: could parse tags here
    return false;
}

// =============================================================================
// Engine Info
// =============================================================================

QString BassEngine::getVersion() const
{
    DWORD version = BASS_GetVersion();
    return QString("%1.%2.%3.%4")
        .arg(HIBYTE(HIWORD(version)))
        .arg(LOBYTE(HIWORD(version)))
        .arg(HIBYTE(LOWORD(version)))
        .arg(LOBYTE(LOWORD(version)));
}

QString BassEngine::getLastError() const
{
    return m_impl->lastErrorMsg;
}

int BassEngine::getLastErrorCode() const
{
    return m_impl->lastError;
}

// =============================================================================
// BASS-Specific Methods
// =============================================================================

int BassEngine::loadPlugins(const QString& pluginDir)
{
    QDir dir(pluginDir);
    if (!dir.exists())
    {
        BasicLogger::logWarning("Plugin directory not found: " + pluginDir.toStdString());
        return 0;
    }
    
    int count = 0;
    
    // Look for BASS plugins (bass*.dll on Windows, libbass*.so on Linux)
#ifdef _WIN32
    QStringList filters = {"bass*.dll"};
#else
    QStringList filters = {"libbass*.so"};
#endif
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& file : files)
    {
        std::string path = file.absoluteFilePath().toLocal8Bit().constData();
        HPLUGIN plugin = BASS_PluginLoad(path.c_str(), 0);
        
        if (plugin != 0)
        {
            m_impl->plugins.push_back(plugin);
            
            // Get plugin info for supported formats
            const BASS_PLUGININFO* info = BASS_PluginGetInfo(plugin);
            if (info != nullptr)
            {
                for (DWORD i = 0; i < info->formatc; i++)
                {
                    QString ext = QString::fromLocal8Bit(info->formats[i].exts);
                    // Parse extensions (format: "*.ext;*.ext2")
                    QStringList exts = ext.split(';');
                    for (const QString& e : exts)
                    {
                        QString clean = e.trimmed().mid(2);  // Remove "*."
                        if (!m_impl->extensions.contains(clean, Qt::CaseInsensitive))
                        {
                            m_impl->extensions.append(clean.toLower());
                        }
                    }
                }
            }
            
            BasicLogger::logInfo("Loaded BASS plugin: " + file.fileName().toStdString());
            count++;
        }
    }
    
    BasicLogger::logInfo("Loaded " + std::to_string(count) + " BASS plugins");
    return count;
}

QStringList BassEngine::supportedExtensions() const
{
    return m_impl->extensions;
}
