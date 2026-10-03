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
#include "services/IEventBus.hpp"

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

    /// Der laufende Titel, wie der Player ihn gemeldet hat (leer = keiner).
    [[nodiscard]] const QString& aktuellerTitel() const { return m_aktuell; }
    /// Ordner, den der Browser gerade zeigt.
    [[nodiscard]] QString ordner() const { return m_dir.absolutePath(); }
    /// In den Ordner des laufenden Titels wechseln und ihn markieren (S93,
    /// Knopf ◎ — wie im Effect Browser). Loest dafuer einen Suchtext, der ihn
    /// verdeckt. false = kein Titel / Datei fehlt.
    bool springeZumAktuellenTitel();

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
    /// Den laufenden Titel uebernehmen (S93, wie der Effect Browser das
    /// laufende Preset): merken und, wenn die Liste ihn zeigt, markieren. Der
    /// Ordner des Browsers bleibt, wo er ist.
    void merkeAktuellenTitel(const QString& pfad);
    /// Den laufenden Titel in der Liste markieren, ohne ihn zu spielen.
    void markiereAktuellenTitel();
    void aktualisiereSprungKnopf();

    QString m_aktuell;  ///< laufender Titel (leer = keiner)
    /// Permanent: der Titel wird auch gemerkt, wenn das Panel nie sichtbar war.
    IEventBus::SubscriberHandle m_titelSubscription;

    QLineEdit* m_pPathEdit = nullptr;
    QPushButton* m_pJumpButton = nullptr;  ///< ◎ zum laufenden Titel (S93)
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
