/**
 * @file Packen.cpp
 * @brief Publish, Stufe P-A: ein Fund als Paket (Publish_Konzept.md §3, P5 a)
 */
#include "paket/Packen.hpp"

#include "core/Datensicherung.hpp"
#include "core/ZipArchiv.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Freeze.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <map>
#include <set>
#include <vector>

namespace lumi::paket
{

namespace
{

namespace lp = lumi::loop;
namespace ds = lumi::core::datensicherung;
using lumi::core::ZipEintrag;

const QString kEigen = QStringLiteral("user:");

/// Schreibt die Nennungen einer Datei auf die Namen beim Empfaenger um.
class Umschreiber
{
public:
    explicit Umschreiber(const Fund& fund)
    {
        for (const Zugehoerige& z : fund.zugehoerige) m_namen[z.art][dateiSchluessel(z.datei)] = z.name;
    }

    /// Die Datei, umgeschrieben, als Bytes — Instrument, Kit und Loop. Leer =
    /// unveraendert zu kopieren (eine Form, ein Sample, oder nicht lesbar).
    [[nodiscard]] QByteArray umgeschrieben(Art art, const QString& datei) const
    {
        const QString basis = QFileInfo(datei).absolutePath();
        switch (art)
        {
            case Art::Instrument:
            {
                lp::Instrument instrument;
                if (!lp::ladeInstrument(datei, instrument)) return {};
                for (lp::Schicht& schicht : instrument.schichten)
                {
                    if (schicht.schwingung == lp::Schwingung::Sample)
                        schicht.sample = neu(Art::Sample, schicht.sample, basis);
                    else if (schicht.schwingung == lp::Schwingung::Form)
                        schicht.form = neu(Art::Kontur, schicht.form, basis);
                }
                return QJsonDocument(lp::alsJson(instrument)).toJson(QJsonDocument::Indented);
            }
            case Art::Kit:
            {
                lp::DrumKit kit;
                if (!lp::ladeKit(datei, kit)) return {};
                for (lp::KitKlang& klang : kit.klaenge)
                {
                    if (!klang.instrument.isEmpty())
                        klang.instrument = neu(Art::Instrument, klang.instrument, basis);
                    else
                        klang.klang = neu(Art::Sample, klang.klang, basis);
                }
                return QJsonDocument(lp::alsJson(kit)).toJson(QJsonDocument::Indented);
            }
            case Art::Loop:
            {
                lp::Loop loop;
                if (!lp::ladeLoop(datei, loop)) return {};
                schreibeMusterUm(loop, basis);
                return QJsonDocument(lp::alsJson(loop)).toJson(QJsonDocument::Indented);
            }
            case Art::Muster:
            case Art::Clip:
            case Art::Spur:
            {
                lp::Loop teil;
                if (!lp::ladeAustausch(datei, *austauschArt(art), teil)) return {};
                schreibeMusterUm(teil, basis);
                return lp::austauschText(*austauschArt(art), teil);
            }
            case Art::Kontur:
            case Art::Sample:
            case Art::Akkord: break;
        }
        return {};
    }

private:
    std::map<Art, std::map<QString, QString>> m_namen;

    /// Was die Muster eines Loops nennen, auf die Namen beim Empfaenger.
    void schreibeMusterUm(lp::Loop& loop, const QString& basis) const
    {
        for (lp::Muster& muster : loop.muster)
        {
            if (muster.art == lp::MusterArt::Instrument)
            {
                muster.instrument = neu(Art::Instrument, muster.instrument, basis);
                continue;
            }
            muster.kit = neu(Art::Kit, muster.kit, basis);
            for (lp::Spur& spur : muster.spuren)
            {
                if (!spur.instrument.isEmpty())
                    spur.instrument = neu(Art::Instrument, spur.instrument, basis);
                else
                    spur.klang = neu(Art::Sample, spur.klang, basis);
            }
        }
    }

    /// Zeigt die Nennung auf eine Zugehoerige, heisst sie `user:<name>`; sonst bleibt sie.
    [[nodiscard]] QString neu(Art art, const QString& nennung, const QString& basis) const
    {
        const QString datei = dateiDerNennung(art, nennung, basis);
        if (datei.isEmpty()) return nennung;
        const auto art_ = m_namen.find(art);
        if (art_ == m_namen.end()) return nennung;
        const auto it = art_->second.find(dateiSchluessel(datei));
        return it == art_->second.end() ? nennung : kEigen + it->second;
    }
};

QString endungVon(Art art, const QString& datei)
{
    if (art == Art::Sample) return QFileInfo(datei).suffix().toLower();
    return artText(art);
}

QString herkunftText(Herkunft herkunft)
{
    return herkunft == Herkunft::Eigen ? QStringLiteral("own") : QStringLiteral("elsewhere");
}

}  // namespace

QString bereichOrdner(Art art)
{
    switch (art)
    {
        case Art::Instrument: return QStringLiteral("instruments");
        case Art::Kit: return QStringLiteral("kits");
        case Art::Kontur: return QStringLiteral("contours");
        case Art::Sample: return QStringLiteral("samples");
        case Art::Akkord: return QStringLiteral("chords");
        case Art::Muster: return QStringLiteral("patterns");
        case Art::Clip:
        case Art::Spur: return QStringLiteral("tracks");
        case Art::Loop: break;
    }
    return {};
}

QString bereichId(Art art)
{
    switch (art)
    {
        case Art::Instrument: return QStringLiteral("instruments");
        case Art::Kit: return QStringLiteral("kits");
        case Art::Kontur: return QStringLiteral("shapes");
        case Art::Sample: return QStringLiteral("samples");
        case Art::Akkord: return QStringLiteral("chords");
        case Art::Muster: return QStringLiteral("patterns");
        case Art::Clip:
        case Art::Spur: return QStringLiteral("tracks");
        case Art::Loop: break;
    }
    return {};
}

QString paketName(const Fund& fund)
{
    QString name = fund.hauptName;
    if (name.isEmpty()) name = lp::eigenerName(QFileInfo(fund.hauptdatei).completeBaseName());
    if (name.isEmpty()) name = QStringLiteral("package");
    return name + QLatin1Char('.') + artText(fund.art) + QLatin1String(ds::kEndung);
}

QString hauptPfad(const Fund& fund)
{
    if (bereichOrdner(fund.art).isEmpty()) return QStringLiteral("package/") + QFileInfo(fund.hauptdatei).fileName();
    return QStringLiteral("data/") + bereichOrdner(fund.art) + QLatin1Char('/') + fund.hauptName + QLatin1Char('.') +
           endungVon(fund.art, fund.hauptdatei);
}

QString paketPfad(const Zugehoerige& z)
{
    return QStringLiteral("data/") + bereichOrdner(z.art) + QLatin1Char('/') + z.name + QLatin1Char('.') +
           endungVon(z.art, z.datei);
}

std::vector<FrozenTeil> frozenTeile(const lp::Loop& loop, int rate, const lp::InstrumentVon& saiten)
{
    std::vector<FrozenTeil> teile;
    for (std::size_t k = 0; k < loop.kanaele.size(); ++k)
    {
        const lp::Kanal& kanal = loop.kanaele[k];
        for (const lp::LoopClip& clip : kanal.clips)
        {
            if (clip.frozen.isEmpty()) continue;
            FrozenTeil teil;
            teil.ort = lp::ClipOrt{static_cast<int>(k), clip.startTakt};
            teil.etikett = QStringLiteral("%1 · bar %2")
                               .arg(kanal.name.isEmpty() ? clip.muster : kanal.name)
                               .arg(clip.startTakt + 1);
            teil.abdruck = lp::clipAbdruck(loop, teil.ort, rate, saiten);
            if (lp::frozenZustand(loop, teil.ort, rate, saiten) == lp::FrozenZustand::Frozen)
            {
                teil.datei = lp::frozenDatei(teil.abdruck);
                teil.groesse = QFileInfo(teil.datei).size();
            }
            teile.push_back(std::move(teil));
        }
    }
    return teile;
}

std::vector<FrozenTeil> frozenTeile(const QString& loopDatei, int rate, const lp::InstrumentVon& saiten)
{
    lp::Loop loop;
    if (!lp::ladeLoop(loopDatei, loop)) return {};
    loop.richte();
    return frozenTeile(loop, rate, saiten);
}

QString frozenPfad(const QByteArray& abdruck)
{
    return QStringLiteral("frozen/") + QString::fromLatin1(abdruck) + QStringLiteral(".wav");
}

PackErgebnis packe(const Fund& fund, const QString& ziel, const QString& appVersion, const std::vector<QByteArray>& frozen)
{
    PackErgebnis ergebnis;
    if (!fund.ok)
    {
        ergebnis.fehler = fund.fehler.isEmpty() ? QStringLiteral("Nothing to publish") : fund.fehler;
        return ergebnis;
    }
    const Umschreiber umschreiber(fund);
    std::vector<ZipEintrag> eintraege;
    std::map<QString, int> gruppen;

    const auto nimm = [&](Art art, const QString& datei, const QString& pfad) -> bool {
        QByteArray daten = umschreiber.umgeschrieben(art, datei);
        if (daten.isEmpty())
        {
            QFile f(datei);
            if (!f.open(QIODevice::ReadOnly))
            {
                ergebnis.fehler = QStringLiteral("Can't read %1").arg(datei);
                return false;
            }
            daten = f.readAll();
        }
        ergebnis.groesse += daten.size();
        eintraege.push_back({pfad, std::move(daten), QFileInfo(datei).lastModified()});
        ++ergebnis.dateien;
        if (!bereichId(art).isEmpty()) ++gruppen[bereichId(art)];
        return true;
    };

    if (!nimm(fund.art, fund.hauptdatei, hauptPfad(fund))) return ergebnis;
    QJsonArray dateien;
    for (const Zugehoerige& z : fund.zugehoerige)
    {
        if (!nimm(z.art, z.datei, paketPfad(z))) return ergebnis;
        dateien.append(QJsonObject{{QStringLiteral("path"), paketPfad(z)},
                                   {QStringLiteral("type"), artText(z.art)},
                                   {QStringLiteral("name"), z.name},
                                   {QStringLiteral("origin"), herkunftText(z.herkunft)},
                                   {QStringLiteral("namedBy"), QJsonArray::fromStringList(z.genanntVon)}});
    }
    QJsonArray fehlt;
    for (const Fehlend& f : fund.fehlt)
        fehlt.append(QJsonObject{{QStringLiteral("type"), artText(f.art)},
                                 {QStringLiteral("reference"), f.nennung},
                                 {QStringLiteral("namedBy"), f.genanntVon}});

    // die Wellenformen eingefrorener Clips (Stufe 30i, FP1 a): nur ein Loop hat welche
    QJsonArray wellenformen;
    std::set<QByteArray> genommen;
    for (const QByteArray& abdruck : frozen)
    {
        const QString datei = lp::frozenDatei(abdruck);
        if (fund.art != Art::Loop || datei.isEmpty() || !genommen.insert(abdruck).second) continue;
        QFile f(datei);
        if (!f.open(QIODevice::ReadOnly)) continue;  // inzwischen geraeumt: der Empfaenger rechnet
        QByteArray daten = f.readAll();
        ergebnis.groesse += daten.size();
        eintraege.push_back({frozenPfad(abdruck), std::move(daten), QDateTime()});
        ++ergebnis.dateien;
        ++ergebnis.frozen;
        wellenformen.append(QJsonObject{{QStringLiteral("path"), frozenPfad(abdruck)},
                                        {QStringLiteral("fingerprint"), QString::fromLatin1(abdruck)}});
    }

    QJsonArray mitgeliefert;
    for (const Nennung& n : fund.mitgeliefert)
        mitgeliefert.append(QJsonObject{{QStringLiteral("type"), artText(n.art)}, {QStringLiteral("reference"), n.text}});
    QJsonObject gruppenJson;
    for (const auto& [id, zahl] : gruppen) gruppenJson.insert(id, zahl);
    const QJsonObject paket{
        {QStringLiteral("type"), artText(fund.art)},
        {QStringLiteral("title"), fund.titel},
        {QStringLiteral("name"), fund.hauptName},
        {QStringLiteral("main"), hauptPfad(fund)},
        {QStringLiteral("files"), dateien},
        {QStringLiteral("bundled"), mitgeliefert},
        {QStringLiteral("missing"), fehlt},
        {QStringLiteral("media"), QJsonArray()},  // Stufe P-B (P4 a)
        {QStringLiteral("frozen"), wellenformen},
    };
    const QJsonObject manifest{
        {QStringLiteral("format"), QLatin1String(ds::kFormat)},
        {QStringLiteral("version"), kPaketFassung},
        {QStringLiteral("app"), QStringLiteral("LumiViz")},
        {QStringLiteral("appVersion"), appVersion},
        {QStringLiteral("exported"), QDateTime::currentDateTime().toString(Qt::ISODate)},
        {QStringLiteral("groups"), gruppenJson},
        {QStringLiteral("settings"), false},
        {QStringLiteral("package"), paket},
    };
    // das Manifest zuerst, wie im Export aller Daten
    eintraege.insert(eintraege.begin(),
                     {QStringLiteral("manifest.json"), QJsonDocument(manifest).toJson(QJsonDocument::Indented), QDateTime()});
    ergebnis.ok = lumi::core::schreibeZip(ziel, eintraege, &ergebnis.fehler);
    return ergebnis;
}

}  // namespace lumi::paket
