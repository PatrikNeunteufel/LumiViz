/**
 * @file Sample.cpp
 * @brief Loop: eigene Samples (Stufe 11a)
 */

#include "loop/Sample.hpp"

#include "loop/Instrument.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

#include <map>
#include <mutex>

namespace lumi::loop
{

namespace
{

const char* const kKopieWahl = "loop/sampleCopyChoice";

QString& sampleOrdnerImTest()
{
    static QString ordner;
    return ordner;
}

QString nameAus(const QString& nennung)
{
    return nennung.mid(static_cast<int>(qstrlen(kSampleVorsatz)));
}

/// Der Inhalt einer Datei als Pruefsumme; leer = nicht lesbar.
QByteArray pruefsumme(const QString& pfad)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return {};
    QCryptographicHash summe(QCryptographicHash::Sha1);
    return summe.addData(&datei) ? summe.result() : QByteArray();
}

/// Was gemerkt ist, und der Stand der Datei, als es gemerkt wurde.
struct Eintrag
{
    std::shared_ptr<const SampleDaten> daten;
    qint64 groesse = -1;  ///< -1 = es gab keine Datei
    QDateTime geaendert;
};

std::mutex& schloss()
{
    static std::mutex eines;
    return eines;
}

std::map<QString, Eintrag>& gemerkt()
{
    static std::map<QString, Eintrag> alle;
    return alle;
}

/// Der Schluessel im Speicher: die Datei, wenn es eine gibt — zwei Nennungen
/// derselben Datei teilen sich die Daten —, sonst die Nennung.
QString schluesselVon(const QString& nennung, qint64& groesse, QDateTime& geaendert)
{
    const QString datei = sampleDatei(nennung);
    const QFileInfo info(datei);
    if (datei.isEmpty() || !info.isFile())
    {
        groesse = -1;
        geaendert = {};
        return nennung;
    }
    groesse = info.size();
    geaendert = info.lastModified();
    return QDir::cleanPath(info.absoluteFilePath());
}

}  // namespace

bool istEigenesSample(const QString& nennung)
{
    return nennung.startsWith(QLatin1String(kSampleVorsatz));
}

QString eigenerSampleOrdner()
{
    if (!sampleOrdnerImTest().isEmpty()) return sampleOrdnerImTest();
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return basis.isEmpty() ? QString() : QDir(basis).filePath(QStringLiteral("samples"));
}

void setzeEigenenSampleOrdnerFuerTests(const QString& ordner)
{
    sampleOrdnerImTest() = ordner;
}

const QStringList& sampleEndungen()
{
    // was die Audio-Engine dekodiert (BASS mit BASSFLAC)
    static const QStringList kEndungen = {QStringLiteral("wav"), QStringLiteral("flac"), QStringLiteral("mp3"),
                                          QStringLiteral("ogg"), QStringLiteral("aiff"), QStringLiteral("aif")};
    return kEndungen;
}

QString eigeneSampleDatei(const QString& name)
{
    const QString ordner = eigenerSampleOrdner();
    if (ordner.isEmpty() || !istEigenerName(name)) return {};
    for (const QString& endung : sampleEndungen())
    {
        const QString pfad = QDir(ordner).filePath(name + QLatin1Char('.') + endung);
        if (QFileInfo(pfad).isFile()) return pfad;
    }
    return {};
}

QStringList eigeneSamples()
{
    QStringList alle;
    const QString ordner = eigenerSampleOrdner();
    if (ordner.isEmpty() || !QFileInfo(ordner).isDir()) return alle;
    QStringList muster;
    for (const QString& endung : sampleEndungen()) muster << QStringLiteral("*.") + endung;
    for (const QString& datei : QDir(ordner).entryList(muster, QDir::Files, QDir::Name))
    {
        const QString name = QFileInfo(datei).completeBaseName();
        if (!istEigenerName(name)) continue;  // von Hand hineingelegt, mit einem Namen, den keine Schicht nennen kann
        const QString nennung = QLatin1String(kSampleVorsatz) + name;
        if (!alle.contains(nennung)) alle << nennung;  // derselbe Name mit zwei Endungen: einmal
    }
    alle.sort();
    return alle;
}

QString sampleDatei(const QString& nennung)
{
    if (nennung.isEmpty()) return {};
    return istEigenesSample(nennung) ? eigeneSampleDatei(nameAus(nennung)) : nennung;
}

QString sampleTitel(const QString& nennung)
{
    if (nennung.isEmpty()) return {};
    return istEigenesSample(nennung) ? nameAus(nennung) : QFileInfo(nennung).fileName();
}

QString nimmZuEigenen(const QString& quelle)
{
    const QFileInfo info(quelle);
    const QString ordner = eigenerSampleOrdner();
    const QString endung = info.suffix().toLower();
    const QString grund = eigenerName(info.completeBaseName());
    if (!info.isFile() || ordner.isEmpty() || grund.isEmpty() || !sampleEndungen().contains(endung)) return {};
    const QByteArray inhalt = pruefsumme(quelle);
    if (inhalt.isEmpty() || !QDir().mkpath(ordner)) return {};

    for (int zahl = 1; zahl < 1000; ++zahl)
    {
        const QString name = zahl == 1 ? grund : grund.left(60) + QLatin1Char('_') + QString::number(zahl);
        const QString da = eigeneSampleDatei(name);
        if (da.isEmpty())
        {
            const QString ziel = QDir(ordner).filePath(name + QLatin1Char('.') + endung);
            if (!QFile::copy(quelle, ziel)) return {};
            zaehleInstrumentFassung();  // wer Toene gerechnet hat, rechnet neu
            return QLatin1String(kSampleVorsatz) + name;
        }
        // dieselbe Datei liegt schon da (auch: sie wurde aus dem Ordner selbst gewaehlt)
        if (pruefsumme(da) == inhalt) return QLatin1String(kSampleVorsatz) + name;
    }
    return {};
}

bool loescheEigenesSample(const QString& name)
{
    const QString pfad = eigeneSampleDatei(name);
    if (pfad.isEmpty()) return false;
    vergissSampleDaten(QLatin1String(kSampleVorsatz) + name);  // solange die Datei noch da ist: ihr Schluessel
    if (!QFile::remove(pfad)) return false;
    zaehleInstrumentFassung();
    return true;
}

SampleKopie sampleKopieWahl()
{
    const QString wahl = QSettings().value(QLatin1String(kKopieWahl)).toString();
    if (wahl == QLatin1String("copy")) return SampleKopie::Kopieren;
    if (wahl == QLatin1String("keep")) return SampleKopie::AmOrt;
    return SampleKopie::Fragen;
}

void setzeSampleKopieWahl(SampleKopie wahl)
{
    QSettings einstellungen;
    if (wahl == SampleKopie::Fragen)
        einstellungen.remove(QLatin1String(kKopieWahl));
    else
        einstellungen.setValue(QLatin1String(kKopieWahl),
                               wahl == SampleKopie::Kopieren ? QStringLiteral("copy") : QStringLiteral("keep"));
}

std::shared_ptr<const SampleDaten> sampleDaten(const QString& nennung)
{
    if (nennung.isEmpty()) return nullptr;
    qint64 groesse = -1;
    QDateTime geaendert;
    const QString schluessel = schluesselVon(nennung, groesse, geaendert);
    const std::lock_guard<std::mutex> halte(schloss());
    const auto ort = gemerkt().find(schluessel);
    if (ort == gemerkt().end()) return nullptr;
    if (ort->second.groesse != groesse || ort->second.geaendert != geaendert)
    {
        gemerkt().erase(ort);  // die Datei ist eine andere geworden
        return nullptr;
    }
    return ort->second.daten;
}

void merkeSampleDaten(const QString& nennung, int rate, std::vector<float> stereo)
{
    if (nennung.isEmpty() || rate <= 0 || stereo.size() < 2) return;
    Eintrag neu;
    const QString schluessel = schluesselVon(nennung, neu.groesse, neu.geaendert);
    auto daten = std::make_shared<SampleDaten>();
    daten->rate = rate;
    daten->klang.daten = std::move(stereo);
    neu.daten = std::move(daten);
    const std::lock_guard<std::mutex> halte(schloss());
    gemerkt()[schluessel] = std::move(neu);
}

void vergissSampleDaten(const QString& nennung)
{
    qint64 groesse = -1;
    QDateTime geaendert;
    const QString schluessel = nennung.isEmpty() ? QString() : schluesselVon(nennung, groesse, geaendert);
    const std::lock_guard<std::mutex> halte(schloss());
    if (nennung.isEmpty())
        gemerkt().clear();
    else
    {
        gemerkt().erase(schluessel);
        gemerkt().erase(nennung);  // falls gemerkt wurde, als es die Datei noch nicht gab
    }
}

}  // namespace lumi::loop
