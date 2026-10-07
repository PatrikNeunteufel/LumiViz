/**
 * @file EinzelInstanz.cpp
 * @brief Eine laufende App je Exe-Datei (S101)
 */

#include "core/EinzelInstanz.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLockFile>
#include <QThread>

#include <BasicLogger.h>

#ifdef Q_OS_WIN
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#endif

namespace lumi::core
{

namespace
{
const QLatin1String kSchalter("--new-instance");
const QByteArray kAngekommen("ok\n");
}  // namespace

EinzelInstanz::EinzelInstanz(QObject* parent) : QObject(parent) {}

EinzelInstanz::~EinzelInstanz()
{
    if (m_server != nullptr) m_server->close();
    // die Sperre gibt QLockFile im Destruktor frei
}

QString EinzelInstanz::nameFuer(const QString& exePfad)
{
    const QString pfad = QDir::fromNativeSeparators(QDir::cleanPath(exePfad)).toLower();
    QString benutzer = qEnvironmentVariable("USERNAME");
    if (benutzer.isEmpty()) benutzer = qEnvironmentVariable("USER");
    const QByteArray hash =
        QCryptographicHash::hash((pfad + QLatin1Char('|') + benutzer.toLower()).toUtf8(),
                                 QCryptographicHash::Sha1);
    return QStringLiteral("LumiViz-") + QString::fromLatin1(hash.toHex().left(16));
}

QString EinzelInstanz::sperrDatei(const QString& name)
{
    return QDir::temp().filePath(name + QStringLiteral(".lock"));
}

bool EinzelInstanz::neueInstanzVerlangt(const QStringList& argumente)
{
    return argumente.mid(1).contains(kSchalter);
}

EinzelInstanz::Rolle EinzelInstanz::starte(const QString& name, const QString& datei, int geduldMs)
{
    m_sperre = std::make_unique<QLockFile>(sperrDatei(name));
    // Nie nach Alter verwerfen: ob die Sperre verwaist ist (die App ist
    // abgestuerzt), erkennt QLockFile daran, dass ihr Prozess nicht mehr lebt.
    m_sperre->setStaleLockTime(0);

    QElapsedTimer uhr;
    uhr.start();
    do
    {
        if (m_sperre->tryLock(0))
        {
            m_server = new QLocalServer(this);
            m_server->setSocketOptions(QLocalServer::UserAccessOption);
            QLocalServer::removeServer(name);  // ein verwaister Socket (nicht unter Windows)
            connect(m_server, &QLocalServer::newConnection, this, &EinzelInstanz::nimmVerbindungAn);
            if (!m_server->listen(name))
                BasicLogger::logWarning("Single instance: cannot listen for further starts: " +
                                        m_server->errorString().toStdString());
            return Rolle::Erste;
        }
        if (uebergib(name, datei, 500))
        {
            m_sperre.reset();
            return Rolle::Uebergeben;
        }
        QThread::msleep(100);
    } while (uhr.elapsed() < geduldMs);

    m_sperre.reset();
    return Rolle::Allein;
}

bool EinzelInstanz::uebergib(const QString& name, const QString& datei, int wartenMs)
{
    QLocalSocket kanal;
    kanal.connectToServer(name);
    if (!kanal.waitForConnected(wartenMs)) return false;
#ifdef Q_OS_WIN
    // Nur wer gerade im Vordergrund ist, darf ein Fenster nach vorn holen —
    // das ist dieser Start (der Explorer hat ihn eben angeworfen), nicht die
    // laufende App. Er gibt das Recht weiter.
    AllowSetForegroundWindow(ASFW_ANY);
#endif
    kanal.write(datei.toUtf8() + '\n');
    if (!kanal.waitForBytesWritten(wartenMs)) return false;
    // Die Antwort bestaetigt nur: angekommen ist der Pfad mit dem Schreiben.
    // Eine App, die gerade hochfaehrt, liest ihn, sobald ihre Schleife laeuft —
    // ein zweiter Versuch wuerde die Datei doppelt oeffnen.
    if (kanal.waitForReadyRead(wartenMs)) (void)kanal.readAll();
    kanal.disconnectFromServer();
    return true;
}

void EinzelInstanz::nimmVerbindungAn()
{
    while (QLocalSocket* kanal = m_server->nextPendingConnection())
    {
        connect(kanal, &QLocalSocket::disconnected, kanal, &QObject::deleteLater);
        const auto lies = [this, kanal]() {
            if (!kanal->canReadLine()) return;
            QByteArray zeile = kanal->readLine();
            while (zeile.endsWith('\n') || zeile.endsWith('\r')) zeile.chop(1);
            kanal->write(kAngekommen);
            kanal->flush();
            Q_EMIT dateiUebergeben(QString::fromUtf8(zeile));
        };
        connect(kanal, &QLocalSocket::readyRead, this, lies);
        lies();  // die Zeile kann schon da sein
    }
}

}  // namespace lumi::core
