/**
 ****************************************************************************************
 * @file   ShadertoyBrowserPanel.hpp
 * @brief  Shadertoy-Browser: API-Suche mit Vorschaubildern, Doppelklick = laden
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Dock-Panel im ImportBrowserPanel-Muster (Strang S3, Browser-Ausbau):
 * - Suchfeld + Sortierung (Beliebt/Neu/Hot/Geliebt) fragen die offizielle
 *   Query-API ab (`api/v1/shaders/query`, NUR auf Knopfdruck) — sichtbar ist
 *   nur, was Autoren als „public+api" freigegeben haben.
 * - Ergebnisse als Thumbnail-Grid (offizielle Vorschaubilder
 *   `media/shaders/<id>.jpg`, ohne Key abrufbar, rein zur Laufzeit geladen).
 * - Doppelklick: Shader per API holen (ShadertoyImport), als Ein-Node-Chain
 *   unter AppData/shadertoy/<id>.lvfx speichern (Inhalte bleiben LOKAL —
 *   Lizenz-Vorbehalt CC BY-NC-SA, Plan §S3) und via LoadEffectChainEvent als
 *   aktives Visual laden — dieselbe Orchestrierung wie der Import Browser.
 * - API-Key: geteilter QSettings-Schlüssel `shadertoy/apiKey` (identisch mit
 *   dem Feld im Node-Editor; nie im Preset/Repo).
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <QString>

class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QNetworkAccessManager;
class QPushButton;
class QToolButton;

/**
 * @class ShadertoyBrowserPanel
 * @brief Shadertoy-API-Suche mit Thumbnail-Grid; Doppelklick lädt den Shader
 */
class ShadertoyBrowserPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ShadertoyBrowserPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~ShadertoyBrowserPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

private Q_SLOTS:
    void onSearchClicked();
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void setupUI();
    void setStatus(const QString& text);
    /// Vorschaubild eines Ergebnis-Items nachladen (Laufzeit, kein Cache im Repo)
    void fetchThumbnail(const QString& id, int generation, QListWidgetItem* item);
    /// Shader holen, als .lvfx (AppData/shadertoy) speichern, laden lassen
    void importShader(const QString& id);
    [[nodiscard]] QString apiKey() const;

    QLineEdit* m_searchEdit = nullptr;
    QComboBox* m_sortCombo = nullptr;
    QPushButton* m_searchButton = nullptr;
    QToolButton* m_openButton = nullptr;
    QLineEdit* m_keyEdit = nullptr;
    QListWidget* m_results = nullptr;
    QLabel* m_status = nullptr;

    QNetworkAccessManager* m_net = nullptr;  ///< lazy; GETs nur auf Nutzeraktion
    /// Suchlauf-Zähler: Thumbnail-Antworten älterer Läufe werden verworfen
    /// (die Items existieren nach clear() nicht mehr)
    int m_generation = 0;
};
