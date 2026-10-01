/**
 ****************************************************************************************
 * @file   TagViewPanel.hpp
 * @brief  Tags und Cover des laufenden Titels (nur Anzeige)
 *
 * @details
 * Session 86 (Wunsch Patrik): zeigt, was die App aus der laufenden Datei liest
 * — Titel, Interpret, Album, Jahr, Genre, Tracknummer, dazu Datei und Format —
 * und das Cover in groesserer Vorschau. Dieselben Werte setzt der Text-Knoten
 * in seine Vorlagen ein (`TextTags.hpp`); das Panel ist damit auch das
 * Pruefwerkzeug dafuer.
 *
 * Quelle ist `TrackChangedEvent` bzw. beim Einblenden `IAudioPlayer::
 * currentTrack()`. Bearbeiten und Online-Suche sind Ideen
 * (`docs/Offene_Punkte.md`), hier nicht gebaut.
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include "audio/AudioEvents.hpp"

#include <QImage>
#include <QString>

class QLabel;

class TagViewPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit TagViewPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~TagViewPanel() override = default;

    /// Groesste Kantenlaenge der Cover-Vorschau in Pixeln.
    static constexpr int kCoverKante = 256;

    [[nodiscard]] int preferredArea() const override;

    /// Zeigt die Tags von `track`; ein leerer `filePath` heisst „kein Titel".
    void zeige(const TrackInfo& track);

protected:
    void onActivate() override;
    void onDeactivate() override;

private:
    void setupUI();
    QLabel* zeile(const char* name, const QString& beschriftung);

    QLabel* m_cover = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_artist = nullptr;
    QLabel* m_album = nullptr;
    QLabel* m_year = nullptr;
    QLabel* m_genre = nullptr;
    QLabel* m_track = nullptr;
    QLabel* m_file = nullptr;
    QLabel* m_format = nullptr;
    class QFormLayout* m_form = nullptr;
};
