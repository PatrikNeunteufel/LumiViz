/**
 ****************************************************************************************
 * @file   Playlist.cpp
 * @brief  Playlist Service Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "pch.h"
#include "audio/Playlist.hpp"
#include "services/IEventBus.hpp"

#include <BasicLogger.h>

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <algorithm>
#include <random>

// =============================================================================
// Private Implementation
// =============================================================================

struct Playlist::Impl
{
    IEventBus& eventBus;
    
    QString name = "Playlist";
    std::vector<TrackInfo> tracks;
    int currentIndex = -1;
    
    explicit Impl(IEventBus& bus) : eventBus(bus) {}
};

// =============================================================================
// Construction / Destruction
// =============================================================================

Playlist::Playlist(IEventBus& eventBus, QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>(eventBus))
{
    BasicLogger::logDebug("Playlist created");
}

Playlist::~Playlist()
{
    BasicLogger::logDebug("Playlist destroyed");
}

// =============================================================================
// Track Management
// =============================================================================

int Playlist::addTrack(const QString& filePath)
{
    QFileInfo info(filePath);
    if (!info.exists() || !info.isFile())
    {
        return -1;
    }
    
    TrackInfo track;
    track.filePath = filePath;
    track.title = info.completeBaseName();
    // Other metadata can be filled later via refreshMetadata()
    
    m_impl->tracks.push_back(track);
    int index = static_cast<int>(m_impl->tracks.size()) - 1;
    
    publishPlaylistChanged(PlaylistChangedEvent::Action::Added, index);
    emit trackAdded(index);
    
    return index;
}

int Playlist::addTracks(const QStringList& filePaths)
{
    int added = 0;
    for (const QString& path : filePaths)
    {
        if (addTrack(path) >= 0)
        {
            added++;
        }
    }
    return added;
}

bool Playlist::insertTrack(int index, const QString& filePath)
{
    if (index < 0 || index > static_cast<int>(m_impl->tracks.size()))
    {
        return false;
    }
    
    QFileInfo info(filePath);
    if (!info.exists() || !info.isFile())
    {
        return false;
    }
    
    TrackInfo track;
    track.filePath = filePath;
    track.title = info.completeBaseName();
    
    m_impl->tracks.insert(m_impl->tracks.begin() + index, track);
    
    // Adjust current index if necessary
    if (m_impl->currentIndex >= index)
    {
        m_impl->currentIndex++;
    }
    
    publishPlaylistChanged(PlaylistChangedEvent::Action::Added, index);
    emit trackAdded(index);
    
    return true;
}

bool Playlist::removeTrack(int index)
{
    if (index < 0 || index >= static_cast<int>(m_impl->tracks.size()))
    {
        return false;
    }
    
    m_impl->tracks.erase(m_impl->tracks.begin() + index);
    
    // Adjust current index
    if (m_impl->currentIndex == index)
    {
        m_impl->currentIndex = -1;
    }
    else if (m_impl->currentIndex > index)
    {
        m_impl->currentIndex--;
    }
    
    publishPlaylistChanged(PlaylistChangedEvent::Action::Removed, index);
    emit trackRemoved(index);
    
    return true;
}

int Playlist::removeTracks(const std::vector<int>& indices)
{
    if (indices.empty()) return 0;
    
    // Sort indices in descending order to remove from end first
    std::vector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());
    
    int removed = 0;
    for (int index : sorted)
    {
        if (removeTrack(index))
        {
            removed++;
        }
    }
    
    return removed;
}

void Playlist::clear()
{
    m_impl->tracks.clear();
    m_impl->currentIndex = -1;
    
    publishPlaylistChanged(PlaylistChangedEvent::Action::Cleared);
    emit tracksCleared();
}

// =============================================================================
// Track Access
// =============================================================================

int Playlist::count() const
{
    return static_cast<int>(m_impl->tracks.size());
}

bool Playlist::isEmpty() const
{
    return m_impl->tracks.empty();
}

TrackInfo Playlist::trackAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_impl->tracks.size()))
    {
        return TrackInfo{};
    }
    return m_impl->tracks[index];
}

QString Playlist::filePathAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_impl->tracks.size()))
    {
        return QString{};
    }
    return m_impl->tracks[index].filePath;
}

std::vector<TrackInfo> Playlist::tracks() const
{
    return m_impl->tracks;
}

QStringList Playlist::filePaths() const
{
    QStringList paths;
    paths.reserve(static_cast<int>(m_impl->tracks.size()));
    for (const auto& track : m_impl->tracks)
    {
        paths.append(track.filePath);
    }
    return paths;
}

int Playlist::indexOf(const QString& filePath) const
{
    for (size_t i = 0; i < m_impl->tracks.size(); i++)
    {
        if (m_impl->tracks[i].filePath == filePath)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// =============================================================================
// Reordering
// =============================================================================

bool Playlist::moveTrack(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= count() ||
        toIndex < 0 || toIndex >= count() ||
        fromIndex == toIndex)
    {
        return false;
    }
    
    TrackInfo track = m_impl->tracks[fromIndex];
    m_impl->tracks.erase(m_impl->tracks.begin() + fromIndex);
    m_impl->tracks.insert(m_impl->tracks.begin() + toIndex, track);
    
    // Adjust current index
    if (m_impl->currentIndex == fromIndex)
    {
        m_impl->currentIndex = toIndex;
    }
    else if (fromIndex < m_impl->currentIndex && toIndex >= m_impl->currentIndex)
    {
        m_impl->currentIndex--;
    }
    else if (fromIndex > m_impl->currentIndex && toIndex <= m_impl->currentIndex)
    {
        m_impl->currentIndex++;
    }
    
    publishPlaylistChanged(PlaylistChangedEvent::Action::Reordered);
    return true;
}

bool Playlist::swapTracks(int index1, int index2)
{
    if (index1 < 0 || index1 >= count() ||
        index2 < 0 || index2 >= count() ||
        index1 == index2)
    {
        return false;
    }
    
    std::swap(m_impl->tracks[index1], m_impl->tracks[index2]);
    
    // Adjust current index
    if (m_impl->currentIndex == index1)
    {
        m_impl->currentIndex = index2;
    }
    else if (m_impl->currentIndex == index2)
    {
        m_impl->currentIndex = index1;
    }
    
    publishPlaylistChanged(PlaylistChangedEvent::Action::Reordered);
    return true;
}

void Playlist::shuffle()
{
    if (m_impl->tracks.size() < 2) return;
    
    // Remember current track
    QString currentPath;
    if (m_impl->currentIndex >= 0)
    {
        currentPath = m_impl->tracks[m_impl->currentIndex].filePath;
    }
    
    // Shuffle
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(m_impl->tracks.begin(), m_impl->tracks.end(), gen);
    
    // Restore current index
    if (!currentPath.isEmpty())
    {
        m_impl->currentIndex = indexOf(currentPath);
    }
    
    publishPlaylistChanged(PlaylistChangedEvent::Action::Reordered);
}

void Playlist::sort(const QString& field, bool ascending)
{
    if (m_impl->tracks.empty()) return;
    
    // Remember current track
    QString currentPath;
    if (m_impl->currentIndex >= 0)
    {
        currentPath = m_impl->tracks[m_impl->currentIndex].filePath;
    }
    
    auto comparator = [&field, ascending](const TrackInfo& a, const TrackInfo& b) -> bool
    {
        int cmp = 0;
        if (field == "title")
        {
            cmp = a.title.compare(b.title, Qt::CaseInsensitive);
        }
        else if (field == "artist")
        {
            cmp = a.artist.compare(b.artist, Qt::CaseInsensitive);
        }
        else if (field == "album")
        {
            cmp = a.album.compare(b.album, Qt::CaseInsensitive);
        }
        else if (field == "duration")
        {
            cmp = a.durationMs - b.durationMs;
        }
        else // "path"
        {
            cmp = a.filePath.compare(b.filePath, Qt::CaseInsensitive);
        }
        return ascending ? (cmp < 0) : (cmp > 0);
    };
    
    std::sort(m_impl->tracks.begin(), m_impl->tracks.end(), comparator);
    
    // Restore current index
    if (!currentPath.isEmpty())
    {
        m_impl->currentIndex = indexOf(currentPath);
    }
    
    publishPlaylistChanged(PlaylistChangedEvent::Action::Reordered);
}

// =============================================================================
// Current Track
// =============================================================================

int Playlist::currentIndex() const
{
    return m_impl->currentIndex;
}

void Playlist::setCurrentIndex(int index)
{
    if (index == m_impl->currentIndex) return;
    
    int previous = m_impl->currentIndex;
    
    if (index < 0 || index >= count())
    {
        m_impl->currentIndex = -1;
    }
    else
    {
        m_impl->currentIndex = index;
    }
    
    publishIndexChanged(previous);
    emit currentIndexChanged(m_impl->currentIndex);
}

TrackInfo Playlist::currentTrack() const
{
    return trackAt(m_impl->currentIndex);
}

bool Playlist::hasNext() const
{
    return m_impl->currentIndex < count() - 1;
}

bool Playlist::hasPrevious() const
{
    return m_impl->currentIndex > 0;
}

int Playlist::next(bool wrap)
{
    if (isEmpty()) return -1;
    
    int newIndex = m_impl->currentIndex + 1;
    
    if (newIndex >= count())
    {
        if (wrap)
        {
            newIndex = 0;
        }
        else
        {
            return -1;
        }
    }
    
    setCurrentIndex(newIndex);
    return newIndex;
}

int Playlist::previous(bool wrap)
{
    if (isEmpty()) return -1;
    
    int newIndex = m_impl->currentIndex - 1;
    
    if (newIndex < 0)
    {
        if (wrap)
        {
            newIndex = count() - 1;
        }
        else
        {
            return -1;
        }
    }
    
    setCurrentIndex(newIndex);
    return newIndex;
}

// =============================================================================
// Persistence
// =============================================================================

bool Playlist::save(const QString& filePath) const
{
    QString ext = QFileInfo(filePath).suffix().toLower();
    
    if (ext == "m3u" || ext == "m3u8")
    {
        return saveM3U(filePath);
    }
    else if (ext == "pls")
    {
        return savePLS(filePath);
    }
    else if (ext == "json")
    {
        return saveJSON(filePath);
    }
    
    // Default to M3U
    return saveM3U(filePath);
}

bool Playlist::load(const QString& filePath)
{
    QString ext = QFileInfo(filePath).suffix().toLower();
    
    bool success = false;
    
    if (ext == "m3u" || ext == "m3u8")
    {
        success = loadM3U(filePath);
    }
    else if (ext == "pls")
    {
        success = loadPLS(filePath);
    }
    else if (ext == "json")
    {
        success = loadJSON(filePath);
    }
    
    if (success)
    {
        publishPlaylistChanged(PlaylistChangedEvent::Action::Loaded);
        emit playlistLoaded(filePath);
    }
    
    return success;
}

// =============================================================================
// Metadata
// =============================================================================

QString Playlist::name() const
{
    return m_impl->name;
}

void Playlist::setName(const QString& name)
{
    m_impl->name = name;
}

int Playlist::totalDurationMs() const
{
    int total = 0;
    for (const auto& track : m_impl->tracks)
    {
        total += track.durationMs;
    }
    return total;
}

void Playlist::refreshMetadata(std::function<void(int, int)> callback)
{
    // This would require access to IAudioEngine to read metadata
    // For now, just call callback to indicate progress
    int total = count();
    for (int i = 0; i < total; i++)
    {
        if (callback)
        {
            callback(i, total);
        }
    }
}

// =============================================================================
// Private Methods
// =============================================================================

void Playlist::publishPlaylistChanged(PlaylistChangedEvent::Action action,
                                      int affectedIndex)
{
    PlaylistChangedEvent event;
    event.action = action;
    event.trackCount = count();
    event.affectedIndex = affectedIndex;
    m_impl->eventBus.publish(event);
}

void Playlist::publishIndexChanged(int previousIndex)
{
    PlaylistIndexChangedEvent event;
    event.currentIndex = m_impl->currentIndex;
    event.previousIndex = previousIndex;
    m_impl->eventBus.publish(event);
}

bool Playlist::saveM3U(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }
    
    QTextStream out(&file);
    out << "#EXTM3U\n";
    
    for (const auto& track : m_impl->tracks)
    {
        // Extended info line
        out << "#EXTINF:" << (track.durationMs / 1000) << "," 
            << track.artist << " - " << track.title << "\n";
        // File path
        out << track.filePath << "\n";
    }
    
    return true;
}

bool Playlist::savePLS(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }
    
    QTextStream out(&file);
    out << "[playlist]\n";
    
    int num = 1;
    for (const auto& track : m_impl->tracks)
    {
        out << "File" << num << "=" << track.filePath << "\n";
        out << "Title" << num << "=" << track.title << "\n";
        out << "Length" << num << "=" << (track.durationMs / 1000) << "\n";
        num++;
    }
    
    out << "NumberOfEntries=" << count() << "\n";
    out << "Version=2\n";
    
    return true;
}

bool Playlist::saveJSON(const QString& filePath) const
{
    QJsonObject root;
    root["name"] = m_impl->name;
    
    QJsonArray tracksArray;
    for (const auto& track : m_impl->tracks)
    {
        QJsonObject trackObj;
        trackObj["path"] = track.filePath;
        trackObj["title"] = track.title;
        trackObj["artist"] = track.artist;
        trackObj["album"] = track.album;
        trackObj["duration"] = track.durationMs;
        tracksArray.append(trackObj);
    }
    root["tracks"] = tracksArray;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }
    
    file.write(QJsonDocument(root).toJson());
    return true;
}

bool Playlist::loadM3U(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    
    clear();
    
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        
        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith("#"))
        {
            continue;
        }
        
        // This is a file path
        addTrack(line);
    }
    
    return true;
}

bool Playlist::loadPLS(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    
    clear();
    
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        
        if (line.startsWith("File", Qt::CaseInsensitive))
        {
            int eq = line.indexOf('=');
            if (eq > 0)
            {
                addTrack(line.mid(eq + 1));
            }
        }
    }
    
    return true;
}

bool Playlist::loadJSON(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
    {
        return false;
    }
    
    clear();
    
    QJsonObject root = doc.object();
    m_impl->name = root["name"].toString("Playlist");
    
    QJsonArray tracksArray = root["tracks"].toArray();
    for (const QJsonValue& val : tracksArray)
    {
        QJsonObject trackObj = val.toObject();
        
        TrackInfo track;
        track.filePath = trackObj["path"].toString();
        track.title = trackObj["title"].toString();
        track.artist = trackObj["artist"].toString();
        track.album = trackObj["album"].toString();
        track.durationMs = trackObj["duration"].toInt();
        
        if (!track.filePath.isEmpty())
        {
            m_impl->tracks.push_back(track);
        }
    }
    
    return true;
}
