/**
 ****************************************************************************************
 * @file   ScreenshotManager.cpp
 * @brief  ScreenshotManager implementation
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/managers/ScreenshotManager.hpp"

#include "UI/widgets/VisualizerWidget.hpp"

#include <BasicLogger.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

namespace {

/// Name der Fehlerdatei im Ordner des Laufs (eine je Lauf, wird angehaengt).
constexpr auto kErrorLog = "fehler.log";

/// Wie weit die Aufwaerts-Suche nach dem Projekt-Assetordner geht.
constexpr int kMaxParentLevels = 12;

}  // namespace

// =============================================================================
// Construction
// =============================================================================

ScreenshotManager::ScreenshotManager(const QDateTime& startedAt, QObject* parent)
    : QObject(parent)
    , m_startedAt(startedAt)
{}

// =============================================================================
// Reine Regeln
// =============================================================================

QString ScreenshotManager::sessionFolderName(const QDateTime& startedAt)
{
    // Doppelpunkte sind unter Windows in Dateinamen verboten — deshalb
    // Bindestriche in der Uhrzeit. Reihenfolge gross->klein, damit die
    // Ordner alphabetisch zugleich chronologisch stehen.
    return startedAt.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
}

QString ScreenshotManager::shotBaseName(const QString& presetPath)
{
    if (presetPath.isEmpty()) return QStringLiteral("visual");

    QString name = QFileInfo(presetPath).fileName();
    if (name.isEmpty()) return QStringLiteral("visual");

    // Endung als Namensteil behalten (S45) und alles ersetzen, was in einem
    // Dateinamen nichts zu suchen hat — Presetnamen enthalten regelmaessig
    // Zeichen wie ':' oder '?'.
    name.replace(QLatin1Char('.'), QLatin1Char('_'));
    static const QString kForbidden = QStringLiteral("<>:\"/\\|?*");
    for (QChar& c : name)
    {
        if (kForbidden.contains(c) || c.unicode() < 0x20) c = QLatin1Char('_');
    }
    return name;
}

QString ScreenshotManager::uniqueBaseName(
    const QString& base, const std::function<bool(const QString&)>& exists)
{
    if (!exists(base)) return base;
    for (int i = 2; i < 10000; ++i)
    {
        const QString candidate = base + QStringLiteral("_%1").arg(i);
        if (!exists(candidate)) return candidate;
    }
    return base;  // praktisch unerreichbar
}

// =============================================================================
// Ablage
// =============================================================================

QString ScreenshotManager::resolveBaseDir()
{
    const QString configured =
        QSettings().value(QStringLiteral("screenshot/baseDir")).toString();
    if (!configured.isEmpty()) return configured;

    // Ohne Einstellung: den Assetordner des Projekts suchen (dieselbe
    // Aufwaerts-Suche wie bei den Preset-Icons — die Exe liegt tief in out/).
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < kMaxParentLevels; ++i)
    {
        const QString candidate = dir.filePath(QStringLiteral("asset/calibration"));
        if (QFileInfo(candidate).isDir())
        {
            return candidate + QStringLiteral("/screenshot");
        }
        if (!dir.cdUp()) break;
    }

    // Ausserhalb des Projektbaums (installierte App): unter "Bilder".
    const QString pictures =
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    return (pictures.isEmpty() ? QDir::homePath() : pictures) +
           QStringLiteral("/LumiViz");
}

QString ScreenshotManager::sessionDir()
{
    if (!m_sessionDir.isEmpty()) return m_sessionDir;

    const QString path =
        resolveBaseDir() + QLatin1Char('/') + sessionFolderName(m_startedAt);
    if (!QDir().mkpath(path))
    {
        BasicLogger::logWarning("ScreenshotManager: cannot create " +
                                path.toStdString());
        return {};
    }
    m_sessionDir = path;
    BasicLogger::logInfo("ScreenshotManager: session folder " + path.toStdString());
    return m_sessionDir;
}

// =============================================================================
// Betrieb
// =============================================================================

void ScreenshotManager::attach(VisualizerWidget* widget)
{
    if (m_widget == widget) return;
    if (m_widget != nullptr) disconnect(m_widget, nullptr, this, nullptr);
    m_widget = widget;
    if (m_widget != nullptr)
    {
        connect(m_widget, &VisualizerWidget::frameCaptured,
                this, &ScreenshotManager::onFrameCaptured);
    }
}

void ScreenshotManager::setCurrentPreset(const QString& absolutePath)
{
    m_presetPath = absolutePath;
}

void ScreenshotManager::requestShot()
{
    if (m_widget == nullptr)
    {
        Q_EMIT shotFailed(tr("No visualizer to capture"));
        return;
    }
    // Den Preset-Stand JETZT festhalten: bis das Bild kommt, kann der naechste
    // Tastendruck laengst ein anderes Preset geladen haben.
    m_pending.enqueue(Pending{m_presetPath});
    m_widget->requestScreenshot();
}

void ScreenshotManager::reportProblem(const QString& context,
                                      const QStringList& problems)
{
    const QString dir = sessionDir();
    if (!dir.isEmpty())
    {
        QFile log(dir + QLatin1Char('/') + QLatin1String(kErrorLog));
        if (log.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&log);
            out.setEncoding(QStringConverter::Utf8);
            out << QDateTime::currentDateTime().toString(Qt::ISODate) << "  ["
                << context << "]\n";
            out << "  Preset: "
                << (m_presetPath.isEmpty() ? QStringLiteral("(eigene Kette)")
                                           : m_presetPath)
                << '\n';
            for (const QString& line : problems) out << "  " << line << '\n';
            out << '\n';
        }
        else
        {
            BasicLogger::logWarning("ScreenshotManager: cannot append " +
                                    log.fileName().toStdString());
        }
    }

    BasicLogger::logWarning("Problem (" + context.toStdString() + "): " +
                            problems.join(QStringLiteral(" | ")).toStdString());
    // Bild dazu, damit hinterher sichtbar ist, wie es dabei aussah.
    requestShot();
}

void ScreenshotManager::onFrameCaptured(const QImage& image)
{
    if (m_pending.isEmpty()) return;  // nicht von uns angefordert
    const Pending pending = m_pending.dequeue();

    if (image.isNull())
    {
        Q_EMIT shotFailed(tr("Captured frame is empty"));
        return;
    }
    const QString dir = sessionDir();
    if (dir.isEmpty())
    {
        Q_EMIT shotFailed(tr("Cannot create the screenshot folder"));
        return;
    }

    const QString base = uniqueBaseName(
        shotBaseName(pending.presetPath), [&dir](const QString& candidate) {
            return QFileInfo::exists(dir + QLatin1Char('/') + candidate +
                                     QStringLiteral(".png"));
        });
    const QString pngPath = dir + QLatin1Char('/') + base + QStringLiteral(".png");

    if (!image.save(pngPath))
    {
        Q_EMIT shotFailed(tr("Cannot write %1").arg(pngPath));
        return;
    }

    // Begleitzettel: der absolute Pfad des Presets. Bewusst eine eigene Datei
    // und nicht nur der Dateiname im Bildnamen — beim Kalibrieren liegen
    // gleichnamige Presets in verschiedenen Sammlungen.
    QFile note(dir + QLatin1Char('/') + base + QStringLiteral(".txt"));
    if (note.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&note);
        out.setEncoding(QStringConverter::Utf8);
        out << (pending.presetPath.isEmpty()
                    ? QStringLiteral("(eigene Kette, kein Preset geladen)")
                    : QDir::toNativeSeparators(pending.presetPath))
            << '\n';
    }

    BasicLogger::logInfo("Screenshot: " + pngPath.toStdString());
    Q_EMIT shotWritten(pngPath);
}
