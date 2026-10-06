/**
 ****************************************************************************************
 * @file   TagViewPanel.cpp
 * @brief  Tags und Cover des laufenden Titels (nur Anzeige)
 ****************************************************************************************
 */

#include "UI/panels/TagViewPanel.hpp"

#include "audio/AlbumCover.hpp"
#include "audio/IAudioPlayer.hpp"
#include "composer/AppAnbindung.hpp"  // Composer-Naht N15: der laufende Song beim Einblenden
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"  // Songtext des laufenden Titels (S88)

#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QStringList>
#include <QTimer>
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

    // S88: der Songtext des laufenden Titels, wie ihn der Lyrics-Knoten fuehrt
    m_lyricsSource = zeile("tagLyricsSource", tr("Lyrics"));
    m_lyrics = new QPlainTextEdit(this);
    m_lyrics->setObjectName(QStringLiteral("tagLyrics"));
    m_lyrics->setReadOnly(true);
    m_lyrics->setPlaceholderText(tr("No lyrics for this track"));
    layout->addWidget(m_lyrics, 1);

    m_uhr = new QTimer(this);
    m_uhr->setInterval(500);
    connect(m_uhr, &QTimer::timeout, this, &TagViewPanel::zeigeSongtext);
}

void TagViewPanel::zeigeSongtext()
{
    const auto s = MultiEffectVisualizer::songtext();
    QString quelle = QStringLiteral("—");
    QString text;
    if (!s.leer())
    {
        quelle = QStringLiteral("%1 · %2 · %3")
                     .arg(QString::fromStdString(s.quelle),
                          s.synchron ? tr("synced") : tr("not synced"),
                          tr("%n line(s)", nullptr, static_cast<int>(s.zeilen.size())));
        QStringList zeilen;
        zeilen.reserve(static_cast<qsizetype>(s.zeilen.size()));
        for (const auto& z : s.zeilen) zeilen << QString::fromStdString(z.text);
        text = zeilen.join(QLatin1Char('\n'));
    }
    // nur bei Aenderung setzen — sonst springt die Rollstellung zweimal je Sekunde
    if (m_lyricsSource->text() != quelle) m_lyricsSource->setText(quelle);
    if (m_lyrics->toPlainText() != text) m_lyrics->setPlainText(text);
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
    // Composer-Naht N15 (docs/ui/Composer_Konzept.md §12.2): fuehrt der
    // Composer den Ton, ist der laufende Titel der laufende Song der
    // Komposition — nicht der des (angehaltenen) Players.
    if (const lumi::composer::TonStand stand = lumi::composer::tonStand(); stand.fuehrt && stand.hatSong)
        zeige(stand.song);
    zeigeSongtext();
    m_uhr->start();
}

void TagViewPanel::onDeactivate()
{
    m_uhr->stop();
    m_eventSubscriptions.clear();
}
