/**
 * @file Sammler.cpp
 * @brief Publish, Stufe P-A: was ein Ding nennt (Publish_Konzept.md §2a, §3)
 */
#include "paket/Sammler.hpp"

#include "loop/Akkord.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Sample.hpp"
#include "shape/Kontur.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSet>

#include <algorithm>
#include <map>

namespace lumi::paket
{

namespace
{

namespace lp = lumi::loop;
namespace sh = lumi::shape;

const QString kEigen = QStringLiteral("user:");

/// Was eine Nennung ist, nachdem sie aufgeloest wurde.
struct Aufloesung
{
    enum Was
    {
        Nichts,         ///< leer oder ein gerechneter Ton — zaehlt nicht
        Mitgeliefert,
        Datei,          ///< eigen oder an freiem Ort: `pfad`
        Fehlt,
    } was = Nichts;
    QString pfad;
};

QString absolut(const QString& pfad, const QString& basis)
{
    const QFileInfo info(pfad);
    if (info.isAbsolute() || basis.isEmpty()) return QDir::cleanPath(info.absoluteFilePath());
    return QDir::cleanPath(QDir(basis).absoluteFilePath(pfad));
}

Aufloesung alsDatei(const QString& pfad)
{
    if (pfad.isEmpty() || !QFileInfo(pfad).isFile()) return {Aufloesung::Fehlt, {}};
    return {Aufloesung::Datei, pfad};
}

Aufloesung loeseSample(const QString& nennung, const QString& basis)
{
    if (nennung.isEmpty() || lp::istTon(nennung)) return {};
    if (lp::istMitgelieferterKlang(nennung))
        return {QFileInfo(lp::klangDatei(nennung)).isFile() ? Aufloesung::Mitgeliefert : Aufloesung::Fehlt, {}};
    if (nennung.startsWith(kEigen)) return alsDatei(lp::eigeneSampleDatei(nennung.mid(kEigen.size())));
    return alsDatei(absolut(nennung, basis));
}

Aufloesung loeseInstrument(const QString& nennung, const QString& basis)
{
    if (nennung.isEmpty()) return {};
    if (nennung.startsWith(kEigen)) return alsDatei(lp::eigeneDatei(nennung.mid(kEigen.size())));
    if (lp::istInstrumentDatei(nennung)) return alsDatei(absolut(nennung, basis));
    return lp::findeInstrument(nennung) != nullptr ? Aufloesung{Aufloesung::Mitgeliefert, {}}
                                                   : Aufloesung{Aufloesung::Fehlt, {}};
}

Aufloesung loeseKontur(const QString& nennung, const QString& basis)
{
    if (nennung.isEmpty()) return {};
    if (sh::istEigeneZeichnung(nennung)) return alsDatei(sh::eigeneKonturDatei(nennung.mid(kEigen.size())));
    if (sh::istKonturDatei(nennung)) return alsDatei(absolut(nennung, basis));
    const auto& alle = sh::zeichnungen();
    const bool da = std::any_of(alle.begin(), alle.end(), [&nennung](const sh::Zeichnung& z) { return z.name == nennung; });
    return {da ? Aufloesung::Mitgeliefert : Aufloesung::Fehlt, {}};
}

Aufloesung loeseKit(const QString& nennung, const QString& basis)
{
    if (nennung.isEmpty()) return {};
    if (nennung.startsWith(kEigen)) return alsDatei(lp::eigeneKitDatei(nennung.mid(kEigen.size())));
    if (nennung.endsWith(QLatin1String(lp::kKitDateiEndung), Qt::CaseInsensitive) || QFileInfo(nennung).isAbsolute())
        return alsDatei(absolut(nennung, basis));
    const auto& alle = lp::mitgelieferteKits();
    const bool da = std::any_of(alle.begin(), alle.end(), [&nennung](const lp::DrumKit& k) { return k.name == nennung; });
    return {da ? Aufloesung::Mitgeliefert : Aufloesung::Fehlt, {}};
}

/// Der Ordner der eigenen Dateien einer Art.
QString eigenerOrdner(Art art)
{
    switch (art)
    {
        case Art::Instrument: return lp::eigenerInstrumentOrdner();
        case Art::Kit: return lp::eigenerKitOrdner();
        case Art::Kontur: return sh::eigenerKonturOrdner();
        case Art::Sample: return lp::eigenerSampleOrdner();
        case Art::Akkord: return lp::eigenerAkkordOrdner();
        case Art::Muster:
        case Art::Clip:
        case Art::Spur: return lp::eigenerAustauschOrdner(*austauschArt(art));
        case Art::Loop: break;
    }
    return {};
}

/// Liegt die Datei im Ordner der eigenen ihrer Art und traegt einen eigenen
/// Namen, ist sie eigen — auch wenn sie als Pfad genannt wurde (von Claude
/// gesetzt, Publish_Konzept.md §6). Zurueck: der Name; leer = freier Ort.
QString eigenerNameVon(Art art, const QString& datei)
{
    const QString ordner = eigenerOrdner(art);
    if (ordner.isEmpty()) return {};
    const QFileInfo info(datei);
    if (QDir::cleanPath(info.absolutePath()).compare(QDir::cleanPath(QDir(ordner).absolutePath()),
                                                     Qt::CaseInsensitive) != 0)
        return {};
    const QString name = info.completeBaseName();
    return lp::istEigenerName(name) ? name : QString();
}

QString vorgabeName(Art art)
{
    switch (art)
    {
        case Art::Instrument: return QStringLiteral("instrument");
        case Art::Kit: return QStringLiteral("kit");
        case Art::Kontur: return QStringLiteral("shape");
        case Art::Sample: return QStringLiteral("sample");
        case Art::Akkord: return QStringLiteral("chord");
        case Art::Muster: return QStringLiteral("pattern");
        case Art::Clip: return QStringLiteral("clip");
        case Art::Spur: return QStringLiteral("track");
        case Art::Loop: return QStringLiteral("loop");
    }
    return QStringLiteral("file");
}

/// Ein Schluessel, der eine Datei einmal zaehlt, auch unter zwei Schreibweisen.
QString schluessel(const QString& datei)
{
    const QString kanonisch = QFileInfo(datei).canonicalFilePath();
    return (kanonisch.isEmpty() ? QDir::cleanPath(datei) : kanonisch).toLower();
}

class Sammler
{
public:
    Fund fund;

    void nimmHauptding(Art art, const QString& datei)
    {
        fund.art = art;
        fund.hauptdatei = QDir::cleanPath(QFileInfo(datei).absoluteFilePath());
        m_gesehen.insert(schluessel(fund.hauptdatei));
        fund.titel = oeffne(art, fund.hauptdatei);
        if (!fund.ok) return;
        if (art != Art::Loop)
        {
            fund.hauptName = eigenerNameVon(art, fund.hauptdatei);
            if (fund.hauptName.isEmpty()) fund.hauptName = nameAusDatei(art, fund.hauptdatei);
            m_vergeben[art].insert(fund.hauptName);
        }
        while (!m_offen.empty())
        {
            const std::size_t platz = m_offen.back();
            m_offen.pop_back();
            const Zugehoerige z = m_roh[platz];
            (void)oeffne(z.art, z.datei);
        }
        vergibNamen();
    }

private:
    std::vector<Zugehoerige> m_roh;
    std::vector<std::size_t> m_offen;  ///< noch nicht nach ihren Nennungen abgesucht
    std::map<QString, std::size_t> m_platz;
    QSet<QString> m_gesehen;
    std::map<Art, QSet<QString>> m_vergeben;
    std::vector<Nennung> m_mitgeliefert;
    QString m_von;  ///< der Titel dessen, was gerade abgesucht wird

    static QString nameAusDatei(Art art, const QString& datei)
    {
        QString name = lp::eigenerName(QFileInfo(datei).completeBaseName());
        return name.isEmpty() ? vorgabeName(art) : name;
    }

    /// Liest die Datei und besucht ihre Nennungen. Zurueck: ihr Titel.
    QString oeffne(Art art, const QString& datei)
    {
        const QString basis = QFileInfo(datei).absolutePath();
        const QString dateiName = QFileInfo(datei).completeBaseName();
        switch (art)
        {
            case Art::Instrument:
            {
                lp::Instrument instrument;
                if (!lp::ladeInstrument(datei, instrument)) return scheitert(datei);
                m_von = instrument.titel.isEmpty() ? dateiName : instrument.titel;
                for (const lp::Schicht& schicht : instrument.schichten)
                {
                    if (schicht.schwingung == lp::Schwingung::Sample)
                        besuche(Art::Sample, schicht.sample, basis);
                    else if (schicht.schwingung == lp::Schwingung::Form)
                        besuche(Art::Kontur, schicht.form, basis);
                }
                return m_von;
            }
            case Art::Kit:
            {
                lp::DrumKit kit;
                if (!lp::ladeKit(datei, kit)) return scheitert(datei);
                m_von = kit.titel.isEmpty() ? dateiName : kit.titel;
                for (const lp::KitKlang& klang : kit.klaenge)
                {
                    // was spielt: das Instrument, sonst der Klang (KitKlang::klang bleibt stehen)
                    if (!klang.instrument.isEmpty())
                        besuche(Art::Instrument, klang.instrument, basis);
                    else
                        besuche(Art::Sample, klang.klang, basis);
                }
                return m_von;
            }
            case Art::Loop:
            {
                lp::Loop loop;
                if (!lp::ladeLoop(datei, loop)) return scheitert(datei);
                m_von = dateiName;
                besucheMuster(loop, basis);
                return m_von;
            }
            case Art::Muster:
            case Art::Clip:
            case Art::Spur:
            {
                // ein kleiner Loop (Stufe 33): er nennt, was seine Muster nennen
                lp::Loop teil;
                if (!lp::ladeAustausch(datei, *austauschArt(art), teil)) return scheitert(datei);
                m_von = dateiName;
                besucheMuster(teil, basis);
                return m_von;
            }
            case Art::Kontur:
            {
                sh::Zeichnung zeichnung;
                if (!sh::ladeZeichnung(datei, zeichnung)) return scheitert(datei);
                // eine Zeichnung nennt nichts — der Name der Vorlage ist nur zur Anzeige
                return zeichnung.titel.isEmpty() ? dateiName : zeichnung.titel;
            }
            case Art::Sample:
                if (!QFileInfo(datei).isFile()) return scheitert(datei);
                return dateiName;
            case Art::Akkord:
            {
                lp::Akkord akkord;
                if (!lp::ladeAkkord(datei, akkord)) return scheitert(datei);
                return akkord.titel;  // ein Akkord nennt nichts
            }
        }
        return {};
    }

    /// Was die Muster eines Loops nennen: Instrument, Kit, je Spur Instrument oder Sample.
    void besucheMuster(const lp::Loop& loop, const QString& basis)
    {
        for (const lp::Muster& muster : loop.muster)
        {
            if (muster.art == lp::MusterArt::Instrument)
            {
                besuche(Art::Instrument, muster.instrument, basis);
                continue;
            }
            besuche(Art::Kit, muster.kit, basis);
            for (const lp::Spur& spur : muster.spuren)
            {
                if (!spur.instrument.isEmpty())
                    besuche(Art::Instrument, spur.instrument, basis);
                else
                    besuche(Art::Sample, spur.klang, basis);
            }
        }
    }

    /// Die Hauptdatei laesst sich nicht lesen: kein Fund. Eine Zugehoerige, die
    /// sich nicht lesen laesst, geht trotzdem mit — nur ohne ihre Nennungen.
    QString scheitert(const QString& datei)
    {
        if (fund.hauptdatei == datei)
        {
            fund.ok = false;
            fund.fehler = QFileInfo(datei).isFile() ? QStringLiteral("Can't read %1").arg(datei)
                                                    : QStringLiteral("File not found: %1").arg(datei);
        }
        return QFileInfo(datei).completeBaseName();
    }

    void besuche(Art art, const QString& nennung, const QString& basis)
    {
        Aufloesung a;
        switch (art)
        {
            case Art::Instrument: a = loeseInstrument(nennung, basis); break;
            case Art::Kit: a = loeseKit(nennung, basis); break;
            case Art::Kontur: a = loeseKontur(nennung, basis); break;
            case Art::Sample: a = loeseSample(nennung, basis); break;
            case Art::Akkord:  // wird nicht genannt, ebenso die folgenden
            case Art::Muster:
            case Art::Clip:
            case Art::Spur:
            case Art::Loop: break;
        }
        switch (a.was)
        {
            case Aufloesung::Nichts: return;
            case Aufloesung::Mitgeliefert:
                if (std::find(m_mitgeliefert.begin(), m_mitgeliefert.end(), Nennung{art, nennung}) == m_mitgeliefert.end())
                    m_mitgeliefert.push_back({art, nennung});
                return;
            case Aufloesung::Fehlt:
            {
                const Fehlend fehlend{art, nennung, m_von};
                if (std::find(fund.fehlt.begin(), fund.fehlt.end(), fehlend) == fund.fehlt.end())
                    fund.fehlt.push_back(fehlend);
                return;
            }
            case Aufloesung::Datei: break;
        }
        const QString key = schluessel(a.pfad);
        if (m_gesehen.contains(key) && !m_platz.count(key)) return;  // die Hauptdatei selbst
        auto it = m_platz.find(key);
        if (it == m_platz.end())
        {
            Zugehoerige z;
            z.art = art;
            z.datei = QDir::cleanPath(QFileInfo(a.pfad).absoluteFilePath());
            const QString eigen = eigenerNameVon(art, z.datei);
            z.herkunft = eigen.isEmpty() ? Herkunft::FreierOrt : Herkunft::Eigen;
            z.name = eigen;
            m_roh.push_back(z);
            it = m_platz.emplace(key, m_roh.size() - 1).first;
            m_gesehen.insert(key);
            m_offen.push_back(m_roh.size() - 1);
        }
        Zugehoerige& z = m_roh[it->second];
        if (!z.nennungen.contains(nennung)) z.nennungen << nennung;
        if (!m_von.isEmpty() && !z.genanntVon.contains(m_von)) z.genanntVon << m_von;
    }

    /// Eigene behalten ihren Namen; eine Datei an freiem Ort bekommt einen aus
    /// dem Dateinamen, der in ihrer Art noch frei ist (`<name>_2`).
    void vergibNamen()
    {
        for (const Zugehoerige& z : m_roh)
            if (z.herkunft == Herkunft::Eigen) m_vergeben[z.art].insert(z.name);
        for (Zugehoerige& z : m_roh)
        {
            if (z.herkunft == Herkunft::Eigen) continue;
            const QString grund = nameAusDatei(z.art, z.datei);
            QString name = grund;
            for (int zahl = 2; m_vergeben[z.art].contains(name); ++zahl)
                name = grund.left(60) + QLatin1Char('_') + QString::number(zahl);
            m_vergeben[z.art].insert(name);
            z.name = name;
        }
        fund.zugehoerige = m_roh;
        std::stable_sort(fund.zugehoerige.begin(), fund.zugehoerige.end(), [](const Zugehoerige& a, const Zugehoerige& b) {
            if (a.art != b.art) return static_cast<int>(a.art) < static_cast<int>(b.art);
            return a.name < b.name;
        });
        fund.mitgeliefert = m_mitgeliefert;
        std::sort(fund.mitgeliefert.begin(), fund.mitgeliefert.end(),
                  [](const Nennung& a, const Nennung& b) { return a.text < b.text; });
    }
};

}  // namespace

QString artText(Art art)
{
    switch (art)
    {
        case Art::Instrument: return QStringLiteral("lvinst");
        case Art::Kit: return QStringLiteral("lvkit");
        case Art::Loop: return QStringLiteral("lvloop");
        case Art::Kontur: return QStringLiteral("lvcontour");
        case Art::Sample: return QStringLiteral("sample");
        case Art::Akkord: return QStringLiteral("lvchord");
        case Art::Muster: return QStringLiteral("lvpattern");
        case Art::Clip: return QStringLiteral("lvclip");
        case Art::Spur: return QStringLiteral("lvtrack");
    }
    return {};
}

std::optional<lp::AustauschArt> austauschArt(Art art)
{
    switch (art)
    {
        case Art::Muster: return lp::AustauschArt::Muster;
        case Art::Clip: return lp::AustauschArt::Clip;
        case Art::Spur: return lp::AustauschArt::Spur;
        default: break;
    }
    return std::nullopt;
}

std::optional<Art> artVonDatei(const QString& datei)
{
    const QString endung = QFileInfo(datei).suffix().toLower();
    if (endung == QLatin1String("lvpattern")) return Art::Muster;
    if (endung == QLatin1String("lvclip")) return Art::Clip;
    if (endung == QLatin1String("lvtrack")) return Art::Spur;
    if (endung == QLatin1String("lvinst")) return Art::Instrument;
    if (endung == QLatin1String("lvkit")) return Art::Kit;
    if (endung == QLatin1String("lvloop")) return Art::Loop;
    if (endung == QLatin1String("lvcontour")) return Art::Kontur;
    if (endung == QLatin1String("lvchord")) return Art::Akkord;
    if (lp::sampleEndungen().contains(endung)) return Art::Sample;
    return std::nullopt;
}

const Zugehoerige* Fund::finde(Art gesucht, const QString& name) const
{
    for (const Zugehoerige& z : zugehoerige)
        if (z.art == gesucht && z.name == name) return &z;
    return nullptr;
}

Befund befundDerNennung(Art art, const QString& nennung, const QString& basis, QString* datei)
{
    Aufloesung a;
    switch (art)
    {
        case Art::Instrument: a = loeseInstrument(nennung, basis); break;
        case Art::Kit: a = loeseKit(nennung, basis); break;
        case Art::Kontur: a = loeseKontur(nennung, basis); break;
        case Art::Sample: a = loeseSample(nennung, basis); break;
        case Art::Akkord:
        case Art::Muster:
        case Art::Clip:
        case Art::Spur:
        case Art::Loop: break;
    }
    if (datei != nullptr) *datei = a.was == Aufloesung::Datei ? a.pfad : QString();
    switch (a.was)
    {
        case Aufloesung::Nichts: return Befund::Nichts;
        case Aufloesung::Mitgeliefert: return Befund::Mitgeliefert;
        case Aufloesung::Datei: return Befund::Datei;
        case Aufloesung::Fehlt: break;
    }
    return Befund::Fehlt;
}

QString dateiDerNennung(Art art, const QString& nennung, const QString& basis)
{
    QString datei;
    return befundDerNennung(art, nennung, basis, &datei) == Befund::Datei ? datei : QString();
}

QString dateiSchluessel(const QString& datei)
{
    return schluessel(datei);
}

Fund sammle(const QString& datei)
{
    Sammler sammler;
    const std::optional<Art> art = artVonDatei(datei);
    if (!art)
    {
        sammler.fund.fehler = QStringLiteral("Not a file LumiViz can publish: %1").arg(datei);
        return sammler.fund;
    }
    sammler.fund.ok = true;
    sammler.nimmHauptding(*art, datei);
    return sammler.fund;
}

}  // namespace lumi::paket
