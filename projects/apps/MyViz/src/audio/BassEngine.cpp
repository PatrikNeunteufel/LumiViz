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
#include <QByteArray>
#include <QCoreApplication>

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
    
    // -------------------------------------------------------------------------
    // Auto-load plugins from executable directory
    // -------------------------------------------------------------------------
    // BASS plugins (bassflac.dll, etc.) must be loaded for format support
    // and proper tag reading. Look in same directory as executable.
    QString exePath = QCoreApplication::applicationDirPath();
    int pluginCount = loadPlugins(exePath);
    if (pluginCount > 0)
    {
        BasicLogger::logInfo("Loaded " + std::to_string(pluginCount) + " BASS plugins");
    }
    else
    {
        BasicLogger::logWarning("No BASS plugins found in: " + exePath.toStdString());
    }
    
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

bool BassEngine::getFFTDataStereo(AudioStreamHandle stream, float* data, int size)
{
    if (stream == INVALID_STREAM || data == nullptr) return false;
    // FFT_INDIVIDUAL: one FFT per channel, results interleaved (bin*chans + chan).
    const DWORD flag = fftSizeToFlag(size) | BASS_DATA_FFT_INDIVIDUAL;
    const int result = BASS_ChannelGetData(static_cast<HSTREAM>(stream), data, flag);
    if (result == -1)
    {
        m_impl->lastError = BASS_ErrorGetCode();
        return false;
    }
    return true;
}

int BassEngine::getStreamChannels(AudioStreamHandle stream)
{
    if (stream == INVALID_STREAM) return 1;
    BASS_CHANNELINFO info;
    if (!BASS_ChannelGetInfo(static_cast<HSTREAM>(stream), &info)) return 1;
    return info.chans > 0 ? static_cast<int>(info.chans) : 1;
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

// =============================================================================
// ID3v2 Tag Parser Helper
// =============================================================================

namespace
{

/**
 * @brief Parse a single ID3v2 text frame
 *
 * ID3v2 text frames have an encoding byte followed by the text:
 * - 0x00: ISO-8859-1 (Latin-1)
 * - 0x01: UTF-16 with BOM
 * - 0x02: UTF-16BE without BOM
 * - 0x03: UTF-8
 */
QString parseId3v2TextFrame(const char* data, int size)
{
    if (size < 1) return QString();
    
    unsigned char encoding = static_cast<unsigned char>(data[0]);
    const char* text = data + 1;
    int textSize = size - 1;
    
    if (textSize <= 0) return QString();
    
    switch (encoding)
    {
        case 0:  // ISO-8859-1 (Latin-1)
            return QString::fromLatin1(text, textSize).trimmed();
            
        case 1:  // UTF-16 with BOM
        {
            if (textSize < 2) return QString();
            
            // Check BOM
            unsigned char bom1 = static_cast<unsigned char>(text[0]);
            unsigned char bom2 = static_cast<unsigned char>(text[1]);
            
            if (bom1 == 0xFF && bom2 == 0xFE)
            {
                // Little-endian UTF-16
                return QString::fromUtf16(
                    reinterpret_cast<const char16_t*>(text + 2),
                    (textSize - 2) / 2).trimmed();
            }
            else if (bom1 == 0xFE && bom2 == 0xFF)
            {
                // Big-endian UTF-16 - need to swap bytes
                QByteArray swapped;
                swapped.reserve(textSize - 2);
                for (int i = 2; i + 1 < textSize; i += 2)
                {
                    swapped.append(text[i + 1]);
                    swapped.append(text[i]);
                }
                return QString::fromUtf16(
                    reinterpret_cast<const char16_t*>(swapped.constData()),
                    swapped.size() / 2).trimmed();
            }
            // No valid BOM, try as UTF-16LE anyway
            return QString::fromUtf16(
                reinterpret_cast<const char16_t*>(text),
                textSize / 2).trimmed();
        }
            
        case 2:  // UTF-16BE without BOM
        {
            // Swap bytes for big-endian
            QByteArray swapped;
            swapped.reserve(textSize);
            for (int i = 0; i + 1 < textSize; i += 2)
            {
                swapped.append(text[i + 1]);
                swapped.append(text[i]);
            }
            return QString::fromUtf16(
                reinterpret_cast<const char16_t*>(swapped.constData()),
                swapped.size() / 2).trimmed();
        }
            
        case 3:  // UTF-8
            return QString::fromUtf8(text, textSize).trimmed();
            
        default:
            // Unknown encoding, try Latin-1 as fallback
            return QString::fromLatin1(text, textSize).trimmed();
    }
}

/**
 * @brief Parse ID3v2 tags from raw data
 *
 * ID3v2 structure:
 * - Header (10 bytes): "ID3" + version + flags + size
 * - Frames: Frame ID (4 bytes) + Size (4 bytes) + Flags (2 bytes) + Data
 */
bool parseId3v2Tags(const char* data, QString& title, QString& artist, QString& album)
{
    if (data == nullptr) return false;
    
    // Check header "ID3"
    if (data[0] != 'I' || data[1] != 'D' || data[2] != '3')
    {
        return false;
    }
    
    unsigned char majorVersion = static_cast<unsigned char>(data[3]);
    // unsigned char minorVersion = static_cast<unsigned char>(data[4]);
    // unsigned char flags = static_cast<unsigned char>(data[5]);
    
    // Syncsafe size (7 bits per byte)
    int tagSize = ((data[6] & 0x7F) << 21) |
                  ((data[7] & 0x7F) << 14) |
                  ((data[8] & 0x7F) << 7) |
                  (data[9] & 0x7F);
    
    // Frame parsing starts after header
    const char* pos = data + 10;
    const char* end = data + 10 + tagSize;
    
    bool foundAny = false;
    
    while (pos + 10 < end)
    {
        // Frame ID (4 bytes for ID3v2.3/2.4, 3 bytes for ID3v2.2)
        char frameId[5] = {0};
        int headerSize = 10;
        int frameSize = 0;
        
        if (majorVersion >= 3)
        {
            // ID3v2.3 or ID3v2.4
            frameId[0] = pos[0];
            frameId[1] = pos[1];
            frameId[2] = pos[2];
            frameId[3] = pos[3];
            
            if (majorVersion == 4)
            {
                // ID3v2.4: syncsafe size
                frameSize = ((static_cast<unsigned char>(pos[4]) & 0x7F) << 21) |
                            ((static_cast<unsigned char>(pos[5]) & 0x7F) << 14) |
                            ((static_cast<unsigned char>(pos[6]) & 0x7F) << 7) |
                            (static_cast<unsigned char>(pos[7]) & 0x7F);
            }
            else
            {
                // ID3v2.3: big-endian size
                frameSize = (static_cast<unsigned char>(pos[4]) << 24) |
                            (static_cast<unsigned char>(pos[5]) << 16) |
                            (static_cast<unsigned char>(pos[6]) << 8) |
                            static_cast<unsigned char>(pos[7]);
            }
        }
        else
        {
            // ID3v2.2: 3-byte frame ID, 3-byte size
            frameId[0] = pos[0];
            frameId[1] = pos[1];
            frameId[2] = pos[2];
            frameId[3] = '\0';
            
            frameSize = (static_cast<unsigned char>(pos[3]) << 16) |
                        (static_cast<unsigned char>(pos[4]) << 8) |
                        static_cast<unsigned char>(pos[5]);
            headerSize = 6;
        }
        
        // Check for padding (null frame ID)
        if (frameId[0] == '\0')
        {
            break;
        }
        
        // Sanity check
        if (frameSize <= 0 || frameSize > (end - pos - headerSize))
        {
            break;
        }
        
        const char* frameData = pos + headerSize;
        
        // Parse known frames
        QString frameIdStr = QString::fromLatin1(frameId);
        
        if (frameIdStr == "TIT2" || frameIdStr == "TT2")  // Title
        {
            title = parseId3v2TextFrame(frameData, frameSize);
            if (!title.isEmpty()) foundAny = true;
        }
        else if (frameIdStr == "TPE1" || frameIdStr == "TP1")  // Artist
        {
            artist = parseId3v2TextFrame(frameData, frameSize);
            if (!artist.isEmpty()) foundAny = true;
        }
        else if (frameIdStr == "TALB" || frameIdStr == "TAL")  // Album
        {
            album = parseId3v2TextFrame(frameData, frameSize);
            if (!album.isEmpty()) foundAny = true;
        }
        
        pos += headerSize + frameSize;
    }
    
    return foundAny;
}

} // anonymous namespace

// =============================================================================
// Metadata Extraction
// =============================================================================

bool BassEngine::getMetadata(AudioStreamHandle stream,
                             QString& title,
                             QString& artist,
                             QString& album)
{
    if (stream == INVALID_STREAM) return false;
    
    title.clear();
    artist.clear();
    album.clear();
    
    HSTREAM hStream = static_cast<HSTREAM>(stream);
    
    // -------------------------------------------------------------------------
    // Try ID3v2 tags first (most common for MP3, supports Unicode)
    // -------------------------------------------------------------------------
    const char* id3v2 = BASS_ChannelGetTags(hStream, BASS_TAG_ID3V2);
    if (id3v2 != nullptr)
    {
        if (parseId3v2Tags(id3v2, title, artist, album))
        {
            BasicLogger::logDebug("Metadata from ID3v2: " + title.toStdString() + 
                                  " - " + artist.toStdString());
            return true;
        }
    }
    
    // -------------------------------------------------------------------------
    // Try ID3v1 tags (fallback for MP3, Latin-1 only, 30 char limit)
    // -------------------------------------------------------------------------
    const TAG_ID3* id3v1 = reinterpret_cast<const TAG_ID3*>(
        BASS_ChannelGetTags(hStream, BASS_TAG_ID3));
    if (id3v1 != nullptr)
    {
        title = QString::fromLatin1(id3v1->title, 30).trimmed();
        artist = QString::fromLatin1(id3v1->artist, 30).trimmed();
        album = QString::fromLatin1(id3v1->album, 30).trimmed();
        
        if (!title.isEmpty() || !artist.isEmpty())
        {
            BasicLogger::logDebug("Metadata from ID3v1: " + title.toStdString() + 
                                  " - " + artist.toStdString());
            return true;
        }
    }
    
    // -------------------------------------------------------------------------
    // Try OGG/Vorbis/FLAC comments (UTF-8)
    // -------------------------------------------------------------------------
    const char* vorbis = BASS_ChannelGetTags(hStream, BASS_TAG_OGG);
    if (vorbis != nullptr)
    {
        const char* p = vorbis;
        while (*p)
        {
            QString tag = QString::fromUtf8(p);
            int eq = tag.indexOf('=');
            if (eq > 0)
            {
                QString key = tag.left(eq).toUpper();
                QString value = tag.mid(eq + 1);
                
                if (key == "TITLE") title = value;
                else if (key == "ARTIST") artist = value;
                else if (key == "ALBUM") album = value;
            }
            p += strlen(p) + 1;
        }
        
        if (!title.isEmpty() || !artist.isEmpty())
        {
            BasicLogger::logDebug("Metadata from Vorbis: " + title.toStdString() + 
                                  " - " + artist.toStdString());
            return true;
        }
    }
    
    // -------------------------------------------------------------------------
    // Try APE tags (UTF-8) - used by some FLAC/APE/WavPack/MP3 files
    // -------------------------------------------------------------------------
    const char* ape = BASS_ChannelGetTags(hStream, BASS_TAG_APE);
    if (ape != nullptr)
    {
        const char* p = ape;
        while (*p)
        {
            QString tag = QString::fromUtf8(p);
            int eq = tag.indexOf('=');
            if (eq > 0)
            {
                QString key = tag.left(eq).toUpper();
                QString value = tag.mid(eq + 1);
                
                if (key == "TITLE") title = value;
                else if (key == "ARTIST") artist = value;
                else if (key == "ALBUM") album = value;
            }
            p += strlen(p) + 1;
        }
        
        if (!title.isEmpty() || !artist.isEmpty())
        {
            BasicLogger::logDebug("Metadata from APE: " + title.toStdString() + 
                                  " - " + artist.toStdString());
            return true;
        }
    }
    
    // -------------------------------------------------------------------------
    // Try MP4/M4A/AAC tags
    // -------------------------------------------------------------------------
    const char* mp4 = BASS_ChannelGetTags(hStream, BASS_TAG_MP4);
    if (mp4 != nullptr)
    {
        const char* p = mp4;
        while (*p)
        {
            QString tag = QString::fromUtf8(p);
            int eq = tag.indexOf('=');
            if (eq > 0)
            {
                QString key = tag.left(eq).toUpper();
                QString value = tag.mid(eq + 1);
                
                // MP4 uses different tag names
                if (key == "©NAM" || key == "TITLE") title = value;
                else if (key == "©ART" || key == "ARTIST") artist = value;
                else if (key == "©ALB" || key == "ALBUM") album = value;
            }
            p += strlen(p) + 1;
        }
        
        if (!title.isEmpty() || !artist.isEmpty())
        {
            BasicLogger::logDebug("Metadata from MP4: " + title.toStdString() + 
                                  " - " + artist.toStdString());
            return true;
        }
    }
    
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
    
    BasicLogger::logDebug("Searching for BASS plugins in: " + pluginDir.toStdString());
    
    int count = 0;
    
    // Look for BASS plugins (bass*.dll on Windows, libbass*.so on Linux)
#ifdef _WIN32
    QStringList filters = {"bass*.dll"};
#else
    QStringList filters = {"libbass*.so"};
#endif
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    BasicLogger::logDebug("Found " + std::to_string(files.size()) + " potential plugin files");
    
    for (const QFileInfo& file : files)
    {
        // Skip the main bass.dll - it's not a plugin
        if (file.fileName().compare("bass.dll", Qt::CaseInsensitive) == 0)
        {
            continue;
        }
        
        BasicLogger::logDebug("  Trying to load: " + file.fileName().toStdString());
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
                BasicLogger::logInfo("Loaded BASS plugin: " + file.fileName().toStdString() + 
                                     " (formats: " + std::to_string(info->formatc) + ")");
            }
            else
            {
                BasicLogger::logInfo("Loaded BASS plugin: " + file.fileName().toStdString());
            }
            count++;
        }
        else
        {
            int err = BASS_ErrorGetCode();
            BasicLogger::logWarning("Failed to load plugin: " + file.fileName().toStdString() + 
                                    " (error: " + std::to_string(err) + ")");
        }
    }
    
    return count;
}

QStringList BassEngine::supportedExtensions() const
{
    return m_impl->extensions;
}
