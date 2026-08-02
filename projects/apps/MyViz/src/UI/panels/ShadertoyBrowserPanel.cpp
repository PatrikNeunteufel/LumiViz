/**
 ****************************************************************************************
 * @file   ShadertoyBrowserPanel.cpp
 * @brief  ShadertoyBrowserPanel implementation (Strang S3, Browser-Ausbau)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/panels/ShadertoyBrowserPanel.hpp"

#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/ShadertoyImport.hpp"

#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QToolButton>
#include <QVBoxLayout>

#include <BasicLogger.h>

namespace {

/// Ergebnis-Item: Shader-ID im UserRole
constexpr int kIdRole = Qt::UserRole;

/// Sortier-Combo-Index → API-Sortierschlüssel
const char* sortKeyForIndex(int index)
{
    switch (index)
    {
    case 1: return "newest";
    case 2: return "hot";
    case 3: return "love";
    default: return "popular";
    }
}

} // namespace

// =============================================================================
// Construction
// =============================================================================

ShadertoyBrowserPanel::ShadertoyBrowserPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("shadertoy_browser"),
                tr("Shadertoy Browser"), parent)
{
    setupUI();
}

int ShadertoyBrowserPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

// =============================================================================
// UI
// =============================================================================

void ShadertoyBrowserPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    auto* searchRow = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Suchbegriff (leer = Top der Sortierung)"));
    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItems({tr("Beliebt"), tr("Neu"), tr("Hot"), tr("Geliebt")});
    m_searchButton = new QPushButton(tr("Suchen"), this);
    m_openButton = new QToolButton(this);
    m_openButton->setText(QStringLiteral("🌐"));
    m_openButton->setToolTip(tr("shadertoy.com im Browser öffnen"));
    searchRow->addWidget(m_searchEdit, 1);
    searchRow->addWidget(m_sortCombo);
    searchRow->addWidget(m_searchButton);
    searchRow->addWidget(m_openButton);
    layout->addLayout(searchRow);

    m_keyEdit = new QLineEdit(this);
    m_keyEdit->setEchoMode(QLineEdit::Password);
    m_keyEdit->setPlaceholderText(tr("Shadertoy-API-Key (lokal gespeichert)"));
    m_keyEdit->setToolTip(
        tr("Kostenlos unter shadertoy.com → Profile → Apps. Geteilter "
           "Schlüssel mit dem Node-Editor; wird NUR lokal gespeichert."));
    {
        QSettings settings;
        m_keyEdit->setText(
            settings.value(QStringLiteral("shadertoy/apiKey")).toString());
    }
    connect(m_keyEdit, &QLineEdit::editingFinished, this, [this]() {
        QSettings settings;
        settings.setValue(QStringLiteral("shadertoy/apiKey"),
                          m_keyEdit->text().trimmed());
    });
    layout->addWidget(m_keyEdit);

    m_results = new QListWidget(this);
    m_results->setViewMode(QListView::IconMode);
    m_results->setIconSize(QSize(160, 90));
    m_results->setResizeMode(QListView::Adjust);
    m_results->setUniformItemSizes(true);
    m_results->setWordWrap(true);
    m_results->setToolTip(
        tr("Doppelklick lädt den Shader als aktives Visual (Ein-Node-Chain, "
           "gespeichert unter AppData/shadertoy — Inhalte bleiben lokal, "
           "Shadertoy-Default-Lizenz CC BY-NC-SA)."));
    layout->addWidget(m_results, 1);

    m_status = new QLabel(tr("Bereit — die API sieht nur 'public+api'-Shader."), this);
    m_status->setWordWrap(true);
    layout->addWidget(m_status);

    connect(m_searchButton, &QPushButton::clicked, this,
            &ShadertoyBrowserPanel::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this,
            &ShadertoyBrowserPanel::onSearchClicked);
    connect(m_openButton, &QToolButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.shadertoy.com/")));
    });
    connect(m_results, &QListWidget::itemDoubleClicked, this,
            &ShadertoyBrowserPanel::onItemDoubleClicked);
}

void ShadertoyBrowserPanel::setStatus(const QString& text)
{
    if (m_status != nullptr) m_status->setText(text);
}

QString ShadertoyBrowserPanel::apiKey() const
{
    return m_keyEdit != nullptr ? m_keyEdit->text().trimmed() : QString();
}

// =============================================================================
// Suche + Thumbnails
// =============================================================================

void ShadertoyBrowserPanel::onSearchClicked()
{
    const QString key = apiKey();
    if (key.isEmpty())
    {
        setStatus(tr("⚠ API-Key fehlt — kostenlos anlegen: shadertoy.com → "
                     "Profile → Apps, dann oben eintragen."));
        return;
    }
    if (m_net == nullptr)
    {
        m_net = new QNetworkAccessManager(this);
        m_net->setTransferTimeout(10000);
    }
    ++m_generation;
    const int generation = m_generation;
    m_results->clear();
    setStatus(tr("Suche läuft …"));

    const QUrl url = lumi::shadertoy::queryRequestUrl(
        m_searchEdit->text(), QString::fromLatin1(sortKeyForIndex(m_sortCombo->currentIndex())),
        key);
    QNetworkReply* reply = m_net->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation]() {
        reply->deleteLater();
        if (generation != m_generation) return;  // längst eine neuere Suche
        if (reply->error() != QNetworkReply::NoError)
        {
            setStatus(tr("⚠ Netzwerkfehler: %1").arg(reply->errorString()));
            return;
        }
        const auto result = lumi::shadertoy::parseQueryReply(reply->readAll());
        if (!result.ok)
        {
            setStatus(tr("⚠ API: %1").arg(result.error));
            return;
        }
        if (result.ids.isEmpty())
        {
            setStatus(tr("Keine Treffer."));
            return;
        }
        for (const QString& id : result.ids)
        {
            auto* item = new QListWidgetItem(id, m_results);
            item->setData(kIdRole, id);
            item->setSizeHint(QSize(176, 128));
            fetchThumbnail(id, generation, item);
        }
        setStatus(tr("%1 Treffer, %2 angezeigt — Doppelklick lädt den Shader.")
                      .arg(result.total)
                      .arg(result.ids.size()));
    });
}

void ShadertoyBrowserPanel::fetchThumbnail(const QString& id, int generation,
                                           QListWidgetItem* item)
{
    QNetworkReply* reply =
        m_net->get(QNetworkRequest(lumi::shadertoy::thumbnailUrl(id)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation, item]() {
        reply->deleteLater();
        // Nach clear() (neue Suche) wäre `item` ein toter Zeiger — Generation prüfen
        if (generation != m_generation) return;
        if (reply->error() != QNetworkReply::NoError) return;
        QPixmap pix;
        if (pix.loadFromData(reply->readAll()))
        {
            item->setIcon(QIcon(pix.scaled(160, 90, Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation)));
        }
    });
}

// =============================================================================
// Import: Shader → Ein-Node-Chain (.lvfx, AppData) → LoadEffectChainEvent
// =============================================================================

void ShadertoyBrowserPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item == nullptr) return;
    const QString id = item->data(kIdRole).toString();
    if (!id.isEmpty()) importShader(id);
}

void ShadertoyBrowserPanel::importShader(const QString& id)
{
    const QString key = apiKey();
    if (key.isEmpty() || m_net == nullptr) return;
    setStatus(tr("Lade Shader %1 …").arg(id));
    QNetworkReply* reply =
        m_net->get(QNetworkRequest(lumi::shadertoy::apiRequestUrl(id, key)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, id]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
        {
            setStatus(tr("⚠ Netzwerkfehler: %1").arg(reply->errorString()));
            return;
        }
        const auto result = lumi::shadertoy::parseApiReply(reply->readAll(), id);
        if (!result.ok)
        {
            setStatus(tr("⚠ %1").arg(result.error));
            return;
        }

        // Ein-Node-Chain bauen und LOKAL ablegen (Lizenz-Vorbehalt Plan §S3)
        lumi::multieffect::ChainNode root;
        root.params = lumi::multieffect::ListParams{};
        lumi::multieffect::ChainNode node;
        node.params = result.params;
        node.displayName = result.params.name.empty()
                               ? id.toStdString()
                               : result.params.name;
        root.children.push_back(std::move(node));

        const QString dir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            QStringLiteral("/shadertoy");
        QDir().mkpath(dir);
        const QString path = dir + QLatin1Char('/') + id + QStringLiteral(".lvfx");
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            setStatus(tr("⚠ Konnte %1 nicht schreiben.").arg(path));
            return;
        }
        file.write(
            QJsonDocument(lumi::multieffect::chainToJson(root)).toJson());
        file.close();

        if (auto* bus = eventBus())
        {
            bus->publish(LoadEffectChainEvent{path.toStdString()});
        }
        QString note = tr("%1 von %2 geladen (%3).")
                           .arg(QString::fromStdString(result.params.name),
                                QString::fromStdString(result.params.author),
                                QString::fromStdString(result.params.license));
        if (!result.report.isEmpty())
            note += QLatin1Char('\n') + result.report.join(QLatin1Char('\n'));
        setStatus(note);
        BasicLogger::logInfo("ShadertoyBrowser: importiert " + path.toStdString());
    });
}
