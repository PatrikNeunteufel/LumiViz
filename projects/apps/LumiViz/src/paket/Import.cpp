/**
 * @file Import.cpp
 * @brief Publish, Stufe P-A: ein Paket ansehen und einlesen
 *        (Publish_Konzept.md §3, P3 a, P5 a, P6 a)
 */
#include "paket/Import.hpp"

#include "core/ZipArchiv.hpp"
#include "loop/Akkord.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Freeze.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Sample.hpp"
#include "paket/Packen.hpp"
#include "shape/Kontur.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>

namespace lumi::paket
{

namespace
{

namespace lp = lumi::loop;
namespace sh = lumi::shape;
namespace ds = lumi::core::datensicherung;

const QString kEigen = QStringLiteral("user:");
const QString kManifest = QStringLiteral("manifest.json");

QString endungVon(const QString& pfad)
{
    return QFileInfo(pfad).suffix().toLower();
}

/// Wo eine eigene Datei dieser Art mit diesem Namen liegt (oder liegen wuerde).
QString eigenePfad(Art art, const QString& name, const QString& endung)
{
    switch (art)
    {
        case Art::Instrument: return lp::eigeneDatei(name);
        case Art::Kit: return lp::eigeneKitDatei(name);
        case Art::Kontur: return sh::eigeneKonturDatei(name);
        case Art::Akkord: return lp::eigeneAkkordDatei(name);
        case Art::Sample:
        {
            const QString ordner = lp::eigenerSampleOrdner();
            if (ordner.isEmpty() || !lp::istEigenerName(name)) return {};
            return QDir(ordner).filePath(name + QLatin1Char('.') + endung);
        }
        case Art::Loop: break;
    }
    return {};
}

/// Die eigene Datei mit diesem Namen, wenn es sie gibt — ein Sample unter jeder Endung.
QString eigeneVorhanden(Art art, const QString& name, const QString& endung)
{
    if (art == Art::Sample) return lp::eigeneSampleDatei(name);
    const QString pfad = eigenePfad(art, name, endung);
    return !pfad.isEmpty() && QFileInfo(pfad).isFile() ? pfad : QString();
}

QByteArray liesDatei(const QString& pfad)
{
    QFile datei(pfad);
    return datei.open(QIODevice::ReadOnly) ? datei.readAll() : QByteArray();
}

/// Der Pfad im Paket passt zu Art und Name (und nichts fuehrt hinaus).
bool pfadPasst(Art art, const QString& pfad, const QString& name)
{
    if (!lp::istEigenerName(name)) return false;
    const QString endung = endungVon(pfad);
    if (art == Art::Sample ? !lp::sampleEndungen().contains(endung) : endung != artText(art)) return false;
    return pfad == QStringLiteral("data/") + bereichOrdner(art) + QLatin1Char('/') + name + QLatin1Char('.') + endung;
}

struct Gelesen
{
    PaketInhalt inhalt;
    std::map<QString, QByteArray> daten;  ///< Pfad im Paket -> Bytes, nur was das Manifest nennt
    std::map<QByteArray, QByteArray> frozen;  ///< Abdruck -> Bytes der Wellenform (Stufe 30i)
};

/// 40 Zeichen `0-9a-f` — wie ein Abdruck heisst.
bool istAbdruck(const QString& text)
{
    static const QRegularExpression muster(QStringLiteral("^[0-9a-f]{40}$"));
    return muster.match(text).hasMatch();
}

Gelesen lies(const QString& datei)
{
    Gelesen g;
    PaketInhalt& p = g.inhalt;
    std::vector<lumi::core::ZipEintrag> eintraege;
    QString zipFehler;
    if (!lumi::core::leseZip(datei, eintraege, &zipFehler))
    {
        p.fehler = QStringLiteral("Can't read the package: %1").arg(zipFehler);
        return g;
    }
    std::map<QString, QByteArray> alle;
    for (auto& e : eintraege) alle[e.name] = std::move(e.daten);
    const auto manifestIt = alle.find(kManifest);
    if (manifestIt == alle.end())
    {
        p.fehler = QStringLiteral("This is not a LumiViz package (no manifest).");
        return g;
    }
    const QJsonObject manifest = QJsonDocument::fromJson(manifestIt->second).object();
    p.appVersion = manifest.value(QStringLiteral("appVersion")).toString();
    p.exportiert = QDateTime::fromString(manifest.value(QStringLiteral("exported")).toString(), Qt::ISODate);
    const int fassung = manifest.value(QStringLiteral("version")).toInt();
    if (manifest.value(QStringLiteral("format")).toString() != QLatin1String(ds::kFormat))
    {
        p.fehler = QStringLiteral("This is not a LumiViz package.");
        return g;
    }
    if (fassung > kPaketFassung)
    {
        p.fehler = QStringLiteral("This package was made with a newer LumiViz (%1). Update LumiViz to import it.")
                       .arg(p.appVersion.isEmpty() ? QStringLiteral("?") : p.appVersion);
        return g;
    }
    const QJsonObject paket = manifest.value(QStringLiteral("package")).toObject();
    if (fassung < kPaketFassung || paket.isEmpty())
    {
        p.fehler = QStringLiteral("This is a backup of user data, not a package. Use Settings → User data → Import....");
        return g;
    }
    const std::optional<Art> art = artAusText(paket.value(QStringLiteral("type")).toString());
    if (!art)
    {
        p.fehler = QStringLiteral("This package holds something this LumiViz doesn't know.");
        return g;
    }
    p.art = *art;
    p.titel = paket.value(QStringLiteral("title")).toString();
    p.name = paket.value(QStringLiteral("name")).toString();
    p.hauptPfad = paket.value(QStringLiteral("main")).toString();

    const auto nimm = [&](PaketDatei d) -> bool {
        const auto it = alle.find(d.pfad);
        if (it == alle.end() || !pfadPasst(d.art, d.pfad, d.name) || g.daten.count(d.pfad)) return false;
        d.groesse = it->second.size();
        g.daten[d.pfad] = it->second;
        p.dateien.push_back(std::move(d));
        return true;
    };

    // das Hauptding
    if (p.art == Art::Loop)
    {
        const QString dateiname = p.hauptPfad.mid(8);  // nach `package/`
        if (!p.hauptPfad.startsWith(QStringLiteral("package/")) || dateiname.isEmpty() ||
            dateiname.contains(QLatin1Char('/')) || dateiname.contains(QLatin1Char('\\')) ||
            endungVon(dateiname) != artText(Art::Loop) || !alle.count(p.hauptPfad))
        {
            p.fehler = QStringLiteral("The package is damaged (main file).");
            return g;
        }
        g.daten[p.hauptPfad] = alle[p.hauptPfad];
    }
    else
    {
        PaketDatei haupt;
        haupt.art = p.art;
        haupt.pfad = p.hauptPfad;
        haupt.name = p.name;
        haupt.hauptding = true;
        if (!nimm(haupt))
        {
            p.fehler = QStringLiteral("The package is damaged (main file).");
            return g;
        }
    }
    for (const QJsonValue& wert : paket.value(QStringLiteral("files")).toArray())
    {
        const QJsonObject o = wert.toObject();
        const std::optional<Art> a = artAusText(o.value(QStringLiteral("type")).toString());
        PaketDatei d;
        d.art = a.value_or(Art::Loop);
        d.pfad = o.value(QStringLiteral("path")).toString();
        d.name = o.value(QStringLiteral("name")).toString();
        d.eigenBeimAbsender = o.value(QStringLiteral("origin")).toString() != QStringLiteral("elsewhere");
        for (const QJsonValue& v : o.value(QStringLiteral("namedBy")).toArray()) d.genanntVon << v.toString();
        if (!a || *a == Art::Loop || !nimm(d))
        {
            p.fehler = QStringLiteral("The package is damaged (%1).").arg(d.pfad);
            return g;
        }
    }
    for (const QJsonValue& wert : paket.value(QStringLiteral("bundled")).toArray())
    {
        const QJsonObject o = wert.toObject();
        if (const auto a = artAusText(o.value(QStringLiteral("type")).toString()))
        {
            const Nennung n{*a, o.value(QStringLiteral("reference")).toString()};
            p.mitgeliefert.push_back(n);
            if (befundDerNennung(n.art, n.text, {}) != Befund::Mitgeliefert) p.mitgeliefertFehltHier.push_back(n);
        }
    }
    for (const QJsonValue& wert : paket.value(QStringLiteral("missing")).toArray())
    {
        const QJsonObject o = wert.toObject();
        p.fehlt.push_back({artAusText(o.value(QStringLiteral("type")).toString()).value_or(Art::Sample),
                           o.value(QStringLiteral("reference")).toString(),
                           o.value(QStringLiteral("namedBy")).toString()});
    }
    // Wellenformen eingefrorener Clips (Stufe 30i): nur bei einem Loop, nur unter `frozen/<abdruck>.wav`
    if (p.art == Art::Loop)
    {
        for (const QJsonValue& wert : paket.value(QStringLiteral("frozen")).toArray())
        {
            const QJsonObject o = wert.toObject();
            const QString abdruck = o.value(QStringLiteral("fingerprint")).toString();
            const QString pfad = o.value(QStringLiteral("path")).toString();
            const auto it = alle.find(pfad);
            if (!istAbdruck(abdruck) || pfad != frozenPfad(abdruck.toLatin1()) || it == alle.end() ||
                g.frozen.count(abdruck.toLatin1()))
            {
                p.fehler = QStringLiteral("The package is damaged (%1).").arg(pfad);
                return g;
            }
            p.frozenGroesse += it->second.size();
            ++p.frozen;
            g.frozen[abdruck.toLatin1()] = it->second;
        }
    }

    // gegen den Benutzerordner halten
    for (PaketDatei& d : p.dateien)
    {
        const QString eigene = eigeneVorhanden(d.art, d.name, endungVon(d.pfad));
        if (eigene.isEmpty()) continue;
        d.eigeneDatei = eigene;
        const QFileInfo info(eigene);
        d.eigeneGroesse = info.size();
        d.eigeneGeaendert = info.lastModified();
        d.zustand = liesDatei(eigene) == g.daten[d.pfad] ? ds::Zustand::Gleich : ds::Zustand::NameVergeben;
    }
    p.ok = true;
    return g;
}

/// Die Namen beim Empfaenger: je Art alt -> neu, nur wo sie sich aendern.
using Umbenennung = std::map<Art, std::map<QString, QString>>;

QString neuerName(const Umbenennung& um, Art art, const QString& nennung)
{
    if (!nennung.startsWith(kEigen)) return nennung;
    const auto a = um.find(art);
    if (a == um.end()) return nennung;
    const auto it = a->second.find(nennung.mid(kEigen.size()));
    return it == a->second.end() ? nennung : kEigen + it->second;
}

void benenneUm(lp::Instrument& instrument, const Umbenennung& um)
{
    for (lp::Schicht& s : instrument.schichten)
    {
        s.sample = neuerName(um, Art::Sample, s.sample);
        s.form = neuerName(um, Art::Kontur, s.form);
    }
}

void benenneUm(lp::DrumKit& kit, const Umbenennung& um)
{
    for (lp::KitKlang& k : kit.klaenge)
    {
        k.instrument = neuerName(um, Art::Instrument, k.instrument);
        k.klang = neuerName(um, Art::Sample, k.klang);
    }
}

void benenneUm(lp::Loop& loop, const Umbenennung& um)
{
    for (lp::Muster& m : loop.muster)
    {
        m.instrument = neuerName(um, Art::Instrument, m.instrument);
        m.kit = neuerName(um, Art::Kit, m.kit);
        for (lp::Spur& s : m.spuren)
        {
            s.instrument = neuerName(um, Art::Instrument, s.instrument);
            s.klang = neuerName(um, Art::Sample, s.klang);
        }
    }
}

bool schreibeRoh(const QString& pfad, const QByteArray& daten)
{
    if (!QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
    QSaveFile datei(pfad);
    return datei.open(QIODevice::WriteOnly) && datei.write(daten) == daten.size() && datei.commit();
}

/// Schreibt eine Datei des Pakets an `pfad`, mit umgeschriebenen Nennungen.
bool schreibe(Art art, const QByteArray& daten, const QString& pfad, const Umbenennung& um)
{
    if (!QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
    const QJsonObject json = QJsonDocument::fromJson(daten).object();
    switch (art)
    {
        case Art::Instrument:
        {
            lp::Instrument instrument;
            if (!lp::ausJson(json, instrument)) return false;
            benenneUm(instrument, um);
            return lp::speichereInstrument(instrument, pfad);
        }
        case Art::Kit:
        {
            lp::DrumKit kit;
            if (!lp::ausJson(json, kit)) return false;
            benenneUm(kit, um);
            return lp::speichereKit(pfad, kit);
        }
        case Art::Loop:
        {
            lp::Loop loop;
            if (!lp::ausJson(json, loop)) return false;
            benenneUm(loop, um);
            return lp::speichereLoop(loop, pfad);
        }
        case Art::Kontur:
        {
            sh::Zeichnung zeichnung;
            if (!sh::ausJson(json, zeichnung)) return false;
            return sh::speichereZeichnung(zeichnung, pfad);
        }
        case Art::Sample: return schreibeRoh(pfad, daten);
        case Art::Akkord:
        {
            lp::Akkord akkord;
            if (!lp::ausJson(json, akkord)) return false;
            return lp::speichereAkkord(pfad, akkord);
        }
    }
    return false;
}

/// Die Wellenformen nach FP3 a: je Clip mit Marke des eben geschriebenen Loops der Abdruck
/// beim Empfaenger; abgelegt wird nur, was einem davon gleicht.
void legeWellenformen(const Gelesen& g, const PaketWahl& wahl, PaketErgebnis& ergebnis)
{
    lp::Loop loop;
    if (wahl.rate <= 0 || !lp::ladeLoop(wahl.ziel, loop)) return;
    loop.richte();
    const lp::SaitenVon saiten = lp::saitenAusInstrumenten();
    for (std::size_t k = 0; k < loop.kanaele.size(); ++k)
        for (const lp::LoopClip& clip : loop.kanaele[k].clips)
        {
            if (clip.frozen.isEmpty()) continue;
            ++ergebnis.frozenClips;
            const lp::ClipOrt ort{static_cast<int>(k), clip.startTakt};
            const auto passend = g.frozen.find(lp::clipAbdruck(loop, ort, wahl.rate, saiten));
            if (passend != g.frozen.end() &&
                lp::legeFrozenDaten(passend->first, passend->second, wahl.rate) == lp::FrozenAblage::Gelegt)
                ++ergebnis.frozenGelegt;
            if (lp::frozenZustand(loop, ort, wahl.rate, saiten) == lp::FrozenZustand::Frozen) ++ergebnis.frozenBereit;
        }
}

}  // namespace

std::optional<Art> artAusText(const QString& text)
{
    for (const Art art : {Art::Instrument, Art::Kit, Art::Loop, Art::Kontur, Art::Sample, Art::Akkord})
        if (artText(art) == text) return art;
    return std::nullopt;
}

QString PaketInhalt::dateiname() const
{
    return QFileInfo(hauptPfad).fileName();
}

int PaketInhalt::vergeben() const
{
    int zahl = 0;
    for (const PaketDatei& d : dateien) zahl += d.zustand == ds::Zustand::NameVergeben ? 1 : 0;
    return zahl;
}

bool istPaket(const QString& datei)
{
    std::vector<lumi::core::ZipEintrag> eintraege;
    if (!lumi::core::leseZip(datei, eintraege)) return false;
    for (const auto& e : eintraege)
    {
        if (e.name != kManifest) continue;
        const QJsonObject m = QJsonDocument::fromJson(e.daten).object();
        return m.value(QStringLiteral("format")).toString() == QLatin1String(ds::kFormat) &&
               m.value(QStringLiteral("version")).toInt() >= kPaketFassung && m.contains(QStringLiteral("package"));
    }
    return false;
}

PaketInhalt oeffnePaket(const QString& datei)
{
    return lies(datei).inhalt;
}

QString keepBothName(const PaketInhalt& inhalt, const PaketDatei& datei)
{
    QSet<QString> imPaket;
    for (const PaketDatei& d : inhalt.dateien)
        if (d.art == datei.art) imPaket.insert(d.name);
    for (int zahl = 2;; ++zahl)
    {
        const QString name = datei.name.left(60) + QLatin1Char('_') + QString::number(zahl);
        if (!imPaket.contains(name) && eigeneVorhanden(datei.art, name, endungVon(datei.pfad)).isEmpty()) return name;
    }
}

QByteArray paketDaten(const QString& paket, const QString& pfad)
{
    std::vector<lumi::core::ZipEintrag> eintraege;
    if (!lumi::core::leseZip(paket, eintraege)) return {};
    for (const auto& e : eintraege)
        if (e.name == pfad) return e.daten;
    return {};
}

PaketErgebnis importierePaket(const QString& datei, const PaketWahl& wahl)
{
    PaketErgebnis ergebnis;
    const Gelesen g = lies(datei);
    const PaketInhalt& p = g.inhalt;
    if (!p.ok)
    {
        ergebnis.fehler = p.fehler;
        return ergebnis;
    }
    if (p.brauchtZiel() && wahl.ziel.isEmpty())
    {
        ergebnis.fehler = QStringLiteral("Choose where the %1 goes.").arg(QStringLiteral("loop"));
        return ergebnis;
    }

    // 1. die Namen beim Empfaenger, je Datei (Keep both: keepBothName — die Namen
    // einer Art im Paket sind verschieden, also auch ihre neuen)
    struct Auftrag
    {
        const PaketDatei* datei;
        QString name;
        bool schreiben;
        bool ersetzen;
    };
    std::vector<Auftrag> auftraege;
    Umbenennung um;
    for (const PaketDatei& d : p.dateien)
    {
        const auto w = wahl.jeDatei.find(d.pfad);
        const ds::Wahl gewaehlt = w == wahl.jeDatei.end() ? ds::Wahl::Beide : w->second;
        switch (d.zustand)
        {
            case ds::Zustand::Neu:
                auftraege.push_back({&d, d.name, true, false});
                ++ergebnis.neu;
                break;
            case ds::Zustand::Gleich:
                auftraege.push_back({&d, d.name, false, false});
                ++ergebnis.gleich;
                break;
            case ds::Zustand::NameVergeben:
                if (gewaehlt == ds::Wahl::Meine)
                {
                    auftraege.push_back({&d, d.name, false, false});
                    ++ergebnis.eigeneGenommen;
                }
                else if (gewaehlt == ds::Wahl::Importierte)
                {
                    auftraege.push_back({&d, d.name, true, true});
                    ++ergebnis.ersetzt;
                }
                else
                {
                    const QString name = keepBothName(p, d);
                    um[d.art][d.name] = name;
                    auftraege.push_back({&d, name, true, false});
                    ++ergebnis.umbenannt;
                    ergebnis.umbenennungen << d.name + QStringLiteral(" → ") + name;
                }
                break;
        }
    }

    // 2. schreiben — mit den Nennungen auf die neuen Namen
    for (const Auftrag& a : auftraege)
    {
        const PaketDatei& d = *a.datei;
        const QString pfad = eigenePfad(d.art, a.name, endungVon(d.pfad));
        if (d.hauptding) ergebnis.hauptDatei = a.schreiben ? pfad : d.eigeneDatei;
        if (!a.schreiben) continue;
        if (pfad.isEmpty())
        {
            ergebnis.fehler = QStringLiteral("There is no folder for your %1.").arg(bereichOrdner(d.art));
            return ergebnis;
        }
        // Replace mine bei einem Sample unter anderer Endung: die eigene geht
        if (a.ersetzen && !d.eigeneDatei.isEmpty() &&
            QFileInfo(d.eigeneDatei).absoluteFilePath() != QFileInfo(pfad).absoluteFilePath())
            QFile::remove(d.eigeneDatei);
        if (!schreibe(d.art, g.daten.at(d.pfad), pfad, um))
        {
            ergebnis.fehler = QStringLiteral("Can't write %1").arg(pfad);
            return ergebnis;
        }
        if (d.art == Art::Sample) lp::vergissSampleDaten(kEigen + a.name);
    }
    if (p.brauchtZiel())
    {
        if (!schreibe(p.art, g.daten.at(p.hauptPfad), wahl.ziel, um))
        {
            ergebnis.fehler = QStringLiteral("Can't write %1").arg(wahl.ziel);
            return ergebnis;
        }
        ergebnis.hauptDatei = wahl.ziel;
        legeWellenformen(g, wahl, ergebnis);
    }
    ergebnis.ok = true;
    return ergebnis;
}

}  // namespace lumi::paket
