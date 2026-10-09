/**
 * @file Anbindung.cpp
 * @brief Sichttest-Steuerung: Laden der DLL, Anmelden der Aktionen, Pfade
 *        (docs/ui/Composer_Konzept.md §12.2 Naht N20)
 *
 * Die einzige Datei der App, die die Koepfe des Pakets einbindet. Ohne das
 * Paket (kein `SICHTTEST_VORHANDEN`) uebersetzt sie ohne Steuerung: `lade()`
 * liefert dann leer und sagt, warum.
 */

#include "sichttest/Anbindung.hpp"

#include <BasicLogger.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>

#ifdef SICHTTEST_VORHANDEN
#  include <sichttest_steuerung_qt.hpp>
#endif

namespace lumi::sichttest
{

namespace
{
const QLatin1String kSchalter("--testing");
const QLatin1String kProjektDatei(".sichttest/sichttest.projekt.json");
constexpr int kZustandTaktMs = 250;
}  // namespace

bool verlangt(const QStringList& argumente)
{
    return argumente.mid(1).contains(kSchalter);
}

QString wurzel(const QString& ab)
{
    QDir ordner(ab);
    if (ab.isEmpty() || !ordner.exists()) return {};
    for (;;)
    {
        if (QFileInfo(ordner.filePath(kProjektDatei)).isFile()) return ordner.absolutePath();
        if (!ordner.cdUp()) return {};
    }
}

QString loeseAuf(const QString& angabe, const QString& root)
{
    if (angabe.trimmed().isEmpty()) return {};
    const QString pfad = QDir::fromNativeSeparators(angabe.trimmed());
    QFileInfo datei(pfad);
    if (datei.isRelative())
    {
        if (root.isEmpty()) return {};
        datei = QFileInfo(QDir(root).filePath(pfad));
    }
    return datei.isFile() ? datei.absoluteFilePath() : QString();
}

std::optional<bool> anOderAus(const QJsonObject& argumente, const QString& schluessel)
{
    const QString wert = argumente.value(schluessel).toString().trimmed().toLower();
    if (wert == QLatin1String("an")) return true;
    if (wert == QLatin1String("aus")) return false;
    return std::nullopt;
}

struct Anbindung::Innen
{
    QString root;
    QStringList namen;
    QObject kontext;  ///< fuer den Takt, der den Zustand ins Log schreibt
    int gemeldet = -1;
#ifdef SICHTTEST_VORHANDEN
    std::unique_ptr<::sichttest::Steuerung> steuerung;  // endet zuerst (zuletzt genannt)
#endif
};

Anbindung::Anbindung() : m(std::make_unique<Innen>()) {}
Anbindung::~Anbindung() = default;

std::unique_ptr<Anbindung> Anbindung::lade(const QString& version, QString* grund,
                                           const QString& ordner)
{
#ifdef SICHTTEST_VORHANDEN
    const QString exeOrdner = ordner.isEmpty() ? QCoreApplication::applicationDirPath() : ordner;
    // Mit vollem Pfad aus dem Ordner der Exe, nie ueber den Suchpfad; die
    // grosse Fassung der Schnittstelle steht im Namen der DLL.
    const QString dll = QDir::toNativeSeparators(
        QDir(exeOrdner).filePath(QStringLiteral("SichttestSteuerung%1.dll").arg(STS_S_MAJOR)));
    QString warum;
    std::unique_ptr<::sichttest::Steuerung> steuerung =
        ::sichttest::Steuerung::lade(QStringLiteral("LumiViz"), version, &warum, dll);
    if (!steuerung)
    {
        if (grund != nullptr) *grund = warum;
        return nullptr;
    }
    std::unique_ptr<Anbindung> neu(new Anbindung);
    neu->m->steuerung = std::move(steuerung);
    BasicLogger::logInfo("Sichttest: control DLL " + neu->dllFassung().toStdString() + " loaded from " +
                         dll.toStdString());
    neu->m->root = wurzel(exeOrdner);
    if (neu->m->root.isEmpty())
    {
        // ohne Projektdatei: relative Dateien gelten ab dem Ordner der Exe
        neu->m->root = exeOrdner;
        BasicLogger::logWarning("Sichttest: no .sichttest/sichttest.projekt.json above " +
                                QDir::toNativeSeparators(exeOrdner).toStdString() +
                                " — relative files are looked up next to the exe");
    }
    return neu;
#else
    (void)version;
    (void)ordner;
    if (grund != nullptr)
        *grund = QStringLiteral("LumiViz was built without the Sichttest package.");
    return nullptr;
#endif
}

bool Anbindung::aktion(const QString& name, const QString& beschreibung, const QString& parameter,
                       Aktion aktion, unsigned schalter)
{
#ifdef SICHTTEST_VORHANDEN
    const bool angemeldet = m->steuerung->aktion(
        name, beschreibung, parameter,
        [name, aktion = std::move(aktion)](const QJsonObject& argumente) -> ::sichttest::Ergebnis {
            const Ergebnis e = aktion(argumente);
            const std::string text = e.text.toStdString();
            BasicLogger::logInfo("Sichttest: action " + name.toStdString() +
                                 (e.status == Ergebnis::Status::Ok ? " done" : " FAILED") +
                                 (text.empty() ? std::string() : ": " + text));
            switch (e.status)
            {
                case Ergebnis::Status::Ok:
                    return ::sichttest::ok(text);
                case Ergebnis::Status::Fehler:
                    return ::sichttest::fehler(text);
                case Ergebnis::Status::Ungueltig:
                    break;
            }
            return ::sichttest::ungueltig(text);
        },
        static_cast<uint32_t>(schalter));
    if (angemeldet)
        m->namen << name;
    else
        BasicLogger::logWarning("Sichttest: action " + name.toStdString() + " not registered: " +
                                letzterFehler().toStdString());
    return angemeldet;
#else
    (void)name;
    (void)beschreibung;
    (void)parameter;
    (void)aktion;
    (void)schalter;
    return false;
#endif
}

QStringList Anbindung::aktionen() const
{
    return m->namen;
}

bool Anbindung::verbinde()
{
#ifdef SICHTTEST_VORHANDEN
    const bool ok = m->steuerung->verbinde();
    if (!ok)
        BasicLogger::logWarning("Sichttest: cannot connect: " + letzterFehler().toStdString());
    // Wie es ausging, steht erst spaeter fest: jeden Wechsel einmal ins Log
    auto* takt = new QTimer(&m->kontext);
    takt->setInterval(kZustandTaktMs);
    QObject::connect(takt, &QTimer::timeout, &m->kontext, [this] {
        const int jetzt = static_cast<int>(zustand());
        if (jetzt == m->gemeldet) return;
        m->gemeldet = jetzt;
        switch (zustand())
        {
            case Zustand::Verbindet:
                BasicLogger::logInfo("Sichttest: connecting to the tester ...");
                break;
            case Zustand::Verbunden:
                BasicLogger::logInfo("Sichttest: connected to the tester");
                break;
            case Zustand::Abgelehnt:
                BasicLogger::logWarning("Sichttest: the tester refused the connection: " +
                                        letzterFehler().toStdString());
                break;
            case Zustand::Getrennt:
                BasicLogger::logInfo("Sichttest: not connected" +
                                     (letzterFehler().isEmpty()
                                          ? std::string()
                                          : ": " + letzterFehler().toStdString()));
                break;
        }
    });
    takt->start();
    return ok;
#else
    return false;
#endif
}

Anbindung::Zustand Anbindung::zustand() const
{
#ifdef SICHTTEST_VORHANDEN
    switch (m->steuerung->zustand())
    {
        case STS_VERBINDET:
            return Zustand::Verbindet;
        case STS_VERBUNDEN:
            return Zustand::Verbunden;
        case STS_ABGELEHNT:
            return Zustand::Abgelehnt;
        default:
            break;
    }
#endif
    return Zustand::Getrennt;
}

QString Anbindung::letzterFehler() const
{
#ifdef SICHTTEST_VORHANDEN
    return m->steuerung->letzterFehler();
#else
    return {};
#endif
}

QString Anbindung::dllFassung() const
{
#ifdef SICHTTEST_VORHANDEN
    const ::sichttest::Fassung f = m->steuerung->fassung();
    return QStringLiteral("%1 (interface S %2.%3, protocol P %4 to %5)")
        .arg(QString::fromStdString(f.produkt))
        .arg(f.sMajor)
        .arg(f.sMinor)
        .arg(f.pMin)
        .arg(f.pMax);
#else
    return {};
#endif
}

const QString& Anbindung::root() const
{
    return m->root;
}

}  // namespace lumi::sichttest
