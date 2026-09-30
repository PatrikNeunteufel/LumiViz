/**
 ****************************************************************************************
 * @file   AudioBrowserPanel.hpp
 * @brief  Ordner-Browser fuer Audiodateien („Audio Browser", S84)
 *
 * @details
 * ## AudioBrowserPanel
 *
 * Das Gegenstueck zum Effect Browser fuer Musik (Plan S84, Entscheide 1a/2a/3a):
 * - zeigt einen Ordner: Unterordner, dann die abspielbaren Dateien — welche
 *   Endungen das sind, weiss die Engine (`IAudioEngine::supportedExtensions`,
 *   bei BASS samt geladenen Plugins);
 * - als Icon das **Albumcover** (eingebettet, sonst `cover.jpg` u. ä. im
 *   Ordner; `audio/AlbumCover.hpp`), sonst ein Icon je Formatfamilie
 *   (`asset/img/logo/icons/audio_*.svg`);
 * - **Doppelklick** = an die Playlist haengen und abspielen (steht die Datei
 *   schon drin: zu diesem Eintrag springen);
 * - **Ziehen** = Datei-URL: in die Playlist (an die Stelle), auf Player oder
 *   Visualisierung (abspielen), spaeter in die Timeline.
 *
 * Der Ordner wird bei jedem Wechsel gemerkt (nicht erst in saveState).
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <QDir>
#include <QHash>
#include <QString>

#include <memory>

namespace lumi::audio
{
class CoverLader;
}

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLineEdit;
class QLabel;

/**
 * @class AudioBrowserPanel
 * @brief Folder browser for audio files: double-click plays, drag carries files
 */
class AudioBrowserPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit AudioBrowserPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~AudioBrowserPanel() override;

    /// Kantenlaenge der Listen-Icons (Cover brauchen mehr als 16 px).
    static constexpr int kIconKante = 32;

    [[nodiscard]] int preferredArea() const override;

    /// Was ein Eintrag ist (Qt::UserRole+1); der Pfad steht in Qt::UserRole.
    enum EntryType
    {
        Type_Up = 0,
        Type_Dir = 1,
        Type_Audio = 2
    };

protected:
    void onActivate() override;

private Q_SLOTS:
    void onItemDoubleClicked(QListWidgetItem* item);
    void onUpClicked();
    void onBrowseClicked();
    void onPathEntered();
    void onSearchChanged(const QString& text);

private:
    void setupUI();
    void navigateTo(const QString& dirPath);
    void refresh();
    void setStatus(const QString& text);

    QLineEdit* m_pPathEdit = nullptr;
    QPushButton* m_pUpButton = nullptr;
    QPushButton* m_pBrowseButton = nullptr;
    QLineEdit* m_pSearchEdit = nullptr;
    QListWidget* m_pListWidget = nullptr;
    QLabel* m_pStatusLabel = nullptr;

    QDir m_dir;

    /// Cover als Icon (S84): eingebettet → Ordnerbild → Typ-Icon. Geladen im
    /// Hintergrund; das Typ-Icon steht, bis (und falls) ein Cover kommt.
    std::unique_ptr<lumi::audio::CoverLader> m_coverLader;
    int m_coverAuftrag = 0;
    QHash<QString, QListWidgetItem*> m_eintragNachPfad;  ///< fuer eintreffende Cover
};
