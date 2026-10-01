/**
 ****************************************************************************************
 * @file   TagViewPanel.cpp
 * @brief  Tags und Cover des laufenden Titels (nur Anzeige)
 ****************************************************************************************
 */

#include "UI/panels/TagViewPanel.hpp"

#include "audio/AlbumCover.hpp"
#include "audio/IAudioPlayer.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

TagViewPanel::TagViewPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("tag_view"), tr("Tag View"), parent)
{
    setupUI();
    zeige(TrackInfo{});
}

int TagViewPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

QLabel* TagViewPanel::zeile(const char* name, const QString& beschriftung)
{
    auto* wert = new QLabel(this);
    wert->setObjectName(QLatin1String(name));
    wert->setWordWrap(true);
    // markier- und kopierbar: wer einen Tag pruefen will, will ihn auch greifen
    wert->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    m_form->addRow(beschriftung, wert);
    return wert;
}

void TagViewPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    m_cover = new QLabel(this);
    m_cover->setObjectName(QStringLiteral("tagCover"));
    m_cover->setAlignment(Qt::AlignCenter);
    m_cover->setMinimumHeight(64);
    layout->addWidget(m_cover);

    m_form = new QFormLayout();
    layout->addLayout(m_form);
    m_title = zeile("tagTitle", tr("Title"));
    m_artist = zeile("tagArtist", tr("Artist"));
    m_album = zeile("tagAlbum", tr("Album"));
    m_year = zeile("tagYear", tr("Year"));
    m_genre = zeile("tagGenre", tr("Genre"));
    m_track = zeile("tagTrack", tr("Track"));
    m_file = zeile("tagFile", tr("File"));
    m_format = zeile("tagFormat", tr("Format"));
    layout->addStretch(1);
}

void TagViewPanel::zeige(const TrackInfo& track)
{
    // „—" statt leer: ein fehlender Tag soll als fehlend zu sehen sein
    const auto oder = [](const QString& s) { return s.isEmpty() ? QStringLiteral("—") : s; };
    const bool geladen = !track.filePath.isEmpty();

    m_title->setText(oder(track.title));
    m_artist->setText(oder(track.artist));
    m_album->setText(oder(track.album));
    m_year->setText(oder(track.year));
    m_genre->setText(oder(track.genre));
    m_track->setText(oder(track.trackNumber));
    m_file->setText(geladen ? QDir::toNativeSeparators(track.filePath) : oder(QString()));
    m_file->setToolTip(geladen ? QDir::toNativeSeparators(track.filePath) : QString());

    QStringList format;
    if (track.durationMs > 0)
    {
        const int s = track.durationMs / 1000;
        format << QStringLiteral("%1:%2").arg(s / 60).arg(s % 60, 2, 10, QLatin1Char('0'));
    }
    if (track.sampleRate > 0) format << tr("%1 Hz").arg(track.sampleRate);
    if (track.channels > 0) format << tr("%n channel(s)", nullptr, track.channels);
    if (track.bitrate > 0) format << tr("%1 kbps").arg(track.bitrate);
    m_format->setText(oder(format.join(QStringLiteral(" · "))));

    // Cover: eingebettet, sonst das Ordnerbild (wie im Audio Browser)
    const QImage bild = geladen ? lumi::audio::coverVorschau(track.filePath, kCoverKante)
                                : QImage();
    if (bild.isNull())
    {
        m_cover->setPixmap(QPixmap());
        m_cover->setText(geladen ? tr("No cover") : tr("No track loaded"));
    }
    else
    {
        m_cover->setText(QString());
        m_cover->setPixmap(QPixmap::fromImage(bild));
    }
}

void TagViewPanel::onActivate()
{
    if (IEventBus* bus = eventBus())
    {
        m_eventSubscriptions.push_back(bus->subscribeScoped<TrackChangedEvent>(
            [this](const TrackChangedEvent& e) { zeige(e.track); }));
    }
    // beim Einblenden den laufenden Titel nachholen
    if (auto* player = services().tryResolve<IAudioPlayer>())
        zeige(player->hasTrack() ? player->currentTrack() : TrackInfo{});
}

void TagViewPanel::onDeactivate()
{
    m_eventSubscriptions.clear();
}
