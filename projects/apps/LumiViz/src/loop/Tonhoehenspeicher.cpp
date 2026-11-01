/**
 * @file Tonhoehenspeicher.cpp
 * @brief Loop: den Tonhoehenverlauf einer Datei einmal messen und merken (Stufe 15g, S169)
 */

#include "loop/Tonhoehenspeicher.hpp"

#include <QCryptographicHash>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>

#include <map>
#include <mutex>

namespace lumi::loop
{

namespace
{

QString& ordnerImTest()
{
    static QString ordner;
    return ordner;
}

std::mutex& sperre()
{
    static std::mutex m;
    return m;
}

std::map<QByteArray, Tonhoehenverlauf>& gemerkt()
{
    static std::map<QByteArray, Tonhoehenverlauf> verlaeufe;
    return verlaeufe;
}

const QByteArray kKopf = QByteArrayLiteral("LVPITCH");

}  // namespace

QString tonhoehenOrdner()
{
    if (!ordnerImTest().isEmpty()) return ordnerImTest();
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return basis.isEmpty() ? QString() : QDir(basis).filePath(QStringLiteral("pitch"));
}

void setzeTonhoehenOrdnerFuerTests(const QString& ordner)
{
    ordnerImTest() = ordner;
}

QByteArray tonhoehenAbdruck(const QString& pfad)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return {};
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(QByteArray::number(kTonhoehenFassung));
    hash.addData(QByteArrayLiteral("|"));
    if (!hash.addData(&datei)) return {};
    return hash.result().toHex();
}

bool schreibeTonhoehenverlauf(const QString& datei, const Tonhoehenverlauf& verlauf)
{
    if (verlauf.hz.size() != verlauf.stimmhaft.size()) return false;
    QSaveFile aus(datei);
    if (!aus.open(QIODevice::WriteOnly)) return false;
    QDataStream s(&aus);
    s.setByteOrder(QDataStream::LittleEndian);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.writeRawData(kKopf.constData(), static_cast<int>(kKopf.size()));
    s << static_cast<qint32>(kTonhoehenFassung) << static_cast<qint32>(verlauf.rate) << static_cast<qint32>(verlauf.schritt)
      << static_cast<qint64>(verlauf.hz.size());
    for (std::size_t i = 0; i < verlauf.hz.size(); ++i) s << verlauf.hz[i] << verlauf.stimmhaft[i];
    return s.status() == QDataStream::Ok && aus.commit();
}

std::optional<Tonhoehenverlauf> liesTonhoehenverlauf(const QString& datei)
{
    QFile ein(datei);
    if (!ein.open(QIODevice::ReadOnly)) return std::nullopt;
    QDataStream s(&ein);
    s.setByteOrder(QDataStream::LittleEndian);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    QByteArray kopf(kKopf.size(), '\0');
    if (s.readRawData(kopf.data(), static_cast<int>(kopf.size())) != kKopf.size() || kopf != kKopf) return std::nullopt;
    qint32 fassung = 0;
    qint32 rate = 0;
    qint32 schritt = 0;
    qint64 rahmen = 0;
    s >> fassung >> rate >> schritt >> rahmen;
    // eine andere Fassung oder unsinnige Zahlen: nicht diese Messung
    if (s.status() != QDataStream::Ok || fassung != kTonhoehenFassung || rate <= 0 || schritt <= 0 || rahmen < 0 ||
        rahmen * 8 > ein.size())
        return std::nullopt;
    Tonhoehenverlauf v;
    v.rate = rate;
    v.schritt = schritt;
    v.hz.resize(static_cast<std::size_t>(rahmen));
    v.stimmhaft.resize(static_cast<std::size_t>(rahmen));
    for (qint64 i = 0; i < rahmen; ++i) s >> v.hz[static_cast<std::size_t>(i)] >> v.stimmhaft[static_cast<std::size_t>(i)];
    if (s.status() != QDataStream::Ok) return std::nullopt;
    return v;
}

std::optional<Tonhoehenverlauf> tonhoehenverlaufVon(const QString& pfad, const Dekodierer& dekodierer, int rate)
{
    const QByteArray abdruck = tonhoehenAbdruck(pfad);
    if (abdruck.isEmpty()) return std::nullopt;
    {
        const std::lock_guard<std::mutex> lock(sperre());
        if (const auto da = gemerkt().find(abdruck); da != gemerkt().end()) return da->second;
    }
    const QString ordner = tonhoehenOrdner();
    const QString datei = ordner.isEmpty() ? QString() : QDir(ordner).filePath(QString::fromLatin1(abdruck) + QStringLiteral(".lvpitch"));
    std::optional<Tonhoehenverlauf> verlauf = datei.isEmpty() ? std::nullopt : liesTonhoehenverlauf(datei);
    if (!verlauf)
    {
        // neu messen — ausserhalb der Sperre, die Messung dauert
        if (!dekodierer) return std::nullopt;
        const std::vector<float> stereo = dekodierer(pfad, rate, 3600.0 * 24.0);
        if (stereo.empty()) return std::nullopt;
        verlauf = messeTonhoehenverlauf(stereo, rate);
        if (!datei.isEmpty() && QDir().mkpath(ordner)) (void)schreibeTonhoehenverlauf(datei, *verlauf);
    }
    const std::lock_guard<std::mutex> lock(sperre());
    gemerkt()[abdruck] = *verlauf;
    return verlauf;
}

void vergissTonhoehenverlaeufe()
{
    const std::lock_guard<std::mutex> lock(sperre());
    gemerkt().clear();
}

}  // namespace lumi::loop
