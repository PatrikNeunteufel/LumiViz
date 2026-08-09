/**
 ****************************************************************************************
 * @file   AboutDialog.cpp
 * @brief  AboutDialog implementation with self-registration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/dialogs/AboutDialog.hpp"
#include "AppInfo.hpp"  // Name/Version aus Solution.json (S73)
#include "services/DialogRegistry.hpp"
#include "services/ServiceContainer.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QOpenGLContext>
#include <QSurfaceFormat>

// =============================================================================
// Helper Function (must be before usage)
// =============================================================================

namespace {

QString getOpenGLVersion()
{
    // Try to get OpenGL version from current context
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (ctx != nullptr)
    {
        QSurfaceFormat format = ctx->format();
        return QString("%1.%2")
            .arg(format.majorVersion())
            .arg(format.minorVersion());
    }
    
    // Fallback: Return default format version
    QSurfaceFormat defaultFormat = QSurfaceFormat::defaultFormat();
    return QString("%1.%2")
        .arg(defaultFormat.majorVersion())
        .arg(defaultFormat.minorVersion());
}

} // anonymous namespace

// =============================================================================
// Construction
// =============================================================================

AboutDialog::AboutDialog(ServiceContainer& services, QWidget* parent)
    : DialogBase(services, tr("About LumiViz"), parent)
{
    setupUI();
    setupConnections();
    
    // Set fixed size for About dialog
    setFixedSize(400, 350);
}

// =============================================================================
// Private Methods
// =============================================================================

void AboutDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 20);

    // Title
    m_pTitleLabel = new QLabel(lumi::appName(), this);
    QFont titleFont = m_pTitleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_pTitleLabel->setFont(titleFont);
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_pTitleLabel);

    // Version — aus Solution.json, NICHT als Literal (S73). Genau hier stand
    // seit dem Vorlagen-Stand "Version 0.1.0" und wurde nie nachgezogen.
    m_pVersionLabel = new QLabel(tr("Version %1").arg(lumi::appVersion()), this);
    m_pVersionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_pVersionLabel);

    // Description
    m_pDescriptionLabel = new QLabel(
        tr("Audio Visualizer with Qt6 and OpenGL\n"
           "Tutorial Application for Registry Architecture"),
        this);
    m_pDescriptionLabel->setAlignment(Qt::AlignCenter);
    m_pDescriptionLabel->setWordWrap(true);
    mainLayout->addWidget(m_pDescriptionLabel);

    mainLayout->addStretch();

    // System Info
    QString systemInfo = QString("Qt %1 | OpenGL %2")
        .arg(QString::fromUtf8(qVersion()))
        .arg(getOpenGLVersion());
    
    m_pSystemInfoLabel = new QLabel(systemInfo, this);
    m_pSystemInfoLabel->setAlignment(Qt::AlignCenter);
    QFont smallFont = m_pSystemInfoLabel->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    m_pSystemInfoLabel->setFont(smallFont);
    m_pSystemInfoLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(m_pSystemInfoLabel);

    // Copyright
    m_pCopyrightLabel = new QLabel(
        tr("© 2025 Patrik Neunteufel"),
        this);
    m_pCopyrightLabel->setAlignment(Qt::AlignCenter);
    m_pCopyrightLabel->setFont(smallFont);
    m_pCopyrightLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(m_pCopyrightLabel);

    mainLayout->addSpacing(10);

    // Close Button
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_pCloseButton = new QPushButton(tr("Close"), this);
    m_pCloseButton->setMinimumWidth(100);
    m_pCloseButton->setDefault(true);
    buttonLayout->addWidget(m_pCloseButton);
    
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void AboutDialog::setupConnections()
{
    connect(m_pCloseButton, &QPushButton::clicked, this, &QDialog::accept);
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================
// This macro registers the AboutDialog at program startup.
// The dialog will be available via DialogRegistry::instance().create("about", ...)

REGISTER_DIALOG_SHORTCUT("about", "About LumiViz", "Help", 900, "F1", AboutDialog)
