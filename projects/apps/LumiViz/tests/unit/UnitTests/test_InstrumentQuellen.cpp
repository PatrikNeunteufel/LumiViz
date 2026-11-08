/**
 ****************************************************************************************
 * @file   test_InstrumentQuellen.cpp
 * @brief  Stufe 34f-2: der Waechter der Quellen — kein Wert einer mitgelieferten `.lvinst` aus einer Quelle, deren
 *         Werte nicht ausgeliefert werden duerfen; jeder Schluessel steht im Register; jede benutzte Quelle mit Pflicht
 *         zur Nennung steht in `instruments/CREDITS.md` (docs/audio/Instrumente_IC_Konzept.md IK2-12 a, WZ1 a, WZ2 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Liest vom Arbeitsordner aus (die Test-Exe aus dem Projekt-Root starten): `asset/loop/instrument_quellen.json`, jede
 * `asset/loop/instruments/*.lvinst` und `asset/loop/instruments/CREDITS.md`. Gesehen werden die Schluessel der Kurven
 * einer Kern-Schicht (`coreParams.<name>.sources`, Fassung 22) und die der Vorgaben aus der Tafel des Kerns
 * (`ParameterAngabe::quellen`, fuer Werte, die keine Kurve sind). Rot: ein unbekannter Schluessel, eine Quelle mit
 * `shippable: false`, eine Kurve ohne Schluessel, eine Pflichtquelle, die in `CREDITS.md` fehlt — dann
 * `python asset/loop/make_instrument_credits.py` (schreibt die Datei ab der Marke neu).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentModels/Kerne.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace
{

const QString kRegister = QStringLiteral("asset/loop/instrument_quellen.json");
const QString kInstrumente = QStringLiteral("asset/loop/instruments");
const QString kCredits = QStringLiteral("asset/loop/instruments/CREDITS.md");

struct Quelle
{
    std::string credit;
    bool creditRequired = false;
    bool shippable = true;
};

using Register = std::map<std::string, Quelle>;

QJsonObject liesJson(const QString& pfad)
{
    QFile f(pfad);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(f.readAll()).object();
}

Register liesRegister(std::vector<std::string>* fehler = nullptr)
{
    Register r;
    for (const QJsonValue& v : liesJson(kRegister).value(QStringLiteral("sources")).toArray())
    {
        const QJsonObject o = v.toObject();
        const std::string key = o.value(QStringLiteral("key")).toString().toStdString();
        const bool vollstaendig = o.contains(QStringLiteral("credit")) && o.contains(QStringLiteral("license")) &&
                                  o.value(QStringLiteral("creditRequired")).isBool() && o.value(QStringLiteral("shippable")).isBool();
        if (fehler != nullptr && (key.empty() || !vollstaendig)) fehler->push_back("Eintrag unvollstaendig: " + key);
        if (fehler != nullptr && r.count(key) != 0) fehler->push_back("Schluessel doppelt: " + key);
        r[key] = Quelle{o.value(QStringLiteral("credit")).toString().toStdString(), o.value(QStringLiteral("creditRequired")).toBool(),
                        o.value(QStringLiteral("shippable")).toBool()};
    }
    return r;
}

/// Je Schluessel, wo er benutzt wird; dazu die Fehler einer `.lvinst` (als JSON) gegen das Register.
struct Pruefung
{
    std::map<std::string, std::set<std::string>> benutzt;
    std::vector<std::string> fehler;
};

void pruefeSchluessel(const Register& r, const std::string& wo, const std::vector<std::string>& quellen, Pruefung& p)
{
    for (const std::string& key : quellen)
    {
        p.benutzt[key].insert(wo);
        const auto it = r.find(key);
        if (it == r.end())
            p.fehler.push_back(wo + ": nennt '" + key + "', das nicht im Register steht");
        else if (!it->second.shippable)
            p.fehler.push_back(wo + ": stammt aus '" + key + "' - daraus darf nichts ausgeliefert werden");
    }
}

void pruefeInstrument(const Register& r, const std::string& name, const QJsonObject& instrument, Pruefung& p)
{
    for (const QJsonValue& s : instrument.value(QStringLiteral("layers")).toArray())
    {
        const QJsonObject schicht = s.toObject();
        if (!schicht.contains(QStringLiteral("core"))) continue;
        const std::string kern = schicht.value(QStringLiteral("core")).toString().toStdString();
        const QJsonObject werte = schicht.value(QStringLiteral("coreParams")).toObject();
        for (auto it = werte.begin(); it != werte.end(); ++it)
        {
            const QJsonObject wert = it.value().toObject();
            if (!wert.contains(QStringLiteral("points"))) continue;
            const std::string wo = name + " " + it.key().toStdString();
            std::vector<std::string> quellen;
            for (const QJsonValue& q : wert.value(QStringLiteral("sources")).toArray()) quellen.push_back(q.toString().toStdString());
            if (quellen.empty()) p.fehler.push_back(wo + ": nennt keine Quelle");
            pruefeSchluessel(r, wo, quellen, p);
        }
        // die Vorgaben, die keine Kurve sind: ihre Quellen stehen in der Tafel des Kerns
        const lumi::kern::KernTyp* typ = lumi::kern::kerne().typ(kern);
        if (typ == nullptr)
        {
            p.fehler.push_back(name + ": unbekannter Kern '" + kern + "'");
            continue;
        }
        for (const lumi::kern::ParameterAngabe& a : typ->tafel.werte)
        {
            if (a.art == lumi::kern::WertArt::Kurve) continue;
            const std::string wo = name + " " + a.name + " (Vorgabe der Tafel)";
            if (a.quellen.empty()) p.fehler.push_back(wo + ": nennt keine Quelle");
            pruefeSchluessel(r, wo, a.quellen, p);
        }
    }
}

std::string zusammen(const std::vector<std::string>& z)
{
    std::string s;
    for (const std::string& t : z) s += "\n  " + t;
    return s;
}

}  // namespace

TEST_CASE("Instrument Quellen 34f: das Register ist vollstaendig und eindeutig")
{
    std::vector<std::string> fehler;
    const Register r = liesRegister(&fehler);
    REQUIRE_MESSAGE(!r.empty(), "asset/loop/instrument_quellen.json nicht gefunden — aus dem Projekt-Root starten");
    CHECK_MESSAGE(fehler.empty(), zusammen(fehler));
    // die Eintraege aus IC §3, die nichts ausliefern lassen (IK2-12 a), und die eigene Vorgabe
    REQUIRE(r.count("tu-note-violine") == 1);
    CHECK_FALSE(r.at("tu-note-violine").shippable);
    REQUIRE(r.count("weinzierl2017") == 1);
    CHECK_FALSE(r.at("weinzierl2017").shippable);
    REQUIRE(r.count("lumiviz") == 1);
    CHECK_FALSE(r.at("lumiviz").creditRequired);
    CHECK(r.size() >= 31);
}

TEST_CASE("Instrument Quellen 34f: jede mitgelieferte .lvinst nennt nur bekannte, auslieferbare Quellen")
{
    const Register r = liesRegister();
    REQUIRE(!r.empty());
    const QStringList dateien = QDir(kInstrumente).entryList({QStringLiteral("*.lvinst")}, QDir::Files, QDir::Name);
    REQUIRE_MESSAGE(!dateien.isEmpty(), "asset/loop/instruments nicht gefunden — aus dem Projekt-Root starten");
    Pruefung p;
    int mitKern = 0;
    for (const QString& d : dateien)
    {
        const QJsonObject o = liesJson(kInstrumente + QLatin1Char('/') + d);
        for (const QJsonValue& s : o.value(QStringLiteral("layers")).toArray())
            if (s.toObject().contains(QStringLiteral("core")))
            {
                ++mitKern;
                break;
            }
        pruefeInstrument(r, QFileInfo(d).completeBaseName().toStdString(), o, p);
    }
    CHECK_MESSAGE(p.fehler.empty(), zusammen(p.fehler));
    MESSAGE(mitKern << " Instrumente mit Kern, " << p.benutzt.size() << " Quellen benutzt");
    CHECK(mitKern >= 1);  // heute der Fluegel
    // jede benutzte Pflichtquelle steht in CREDITS.md (sonst: python asset/loop/make_instrument_credits.py)
    QFile c(kCredits);
    REQUIRE_MESSAGE(c.open(QIODevice::ReadOnly), "asset/loop/instruments/CREDITS.md fehlt — make_instrument_credits.py");
    const std::string credits = QString::fromUtf8(c.readAll()).toStdString();
    std::vector<std::string> fehlt;
    for (const auto& [key, wo] : p.benutzt)
        if (const auto it = r.find(key); it != r.end() && it->second.creditRequired && credits.find(it->second.credit) == std::string::npos)
            fehlt.push_back(key + " (" + *wo.begin() + ")");
    CHECK_MESSAGE(fehlt.empty(), "in instruments/CREDITS.md fehlt — python asset/loop/make_instrument_credits.py:" << zusammen(fehlt));
}

TEST_CASE("Instrument Quellen 34f: der Waechter wird rot bei ND-Quelle, unbekanntem Schluessel und ohne Quelle")
{
    const Register r = liesRegister();
    REQUIRE(!r.empty());
    const auto mitQuellen = [](const QJsonArray& quellen) {
        QJsonObject kurve{{QStringLiteral("points"), QJsonArray{QJsonArray{21, 1.0}}}, {QStringLiteral("sources"), quellen}};
        QJsonObject schicht{{QStringLiteral("core"), QStringLiteral("hammerString")},
                            {QStringLiteral("coreParams"), QJsonObject{{QStringLiteral("decay"), kurve}}}};
        return QJsonObject{{QStringLiteral("layers"), QJsonArray{schicht}}};
    };
    const auto fehlerVon = [&](const QJsonObject& o) {
        Pruefung p;
        pruefeInstrument(r, "probe", o, p);
        return zusammen(p.fehler);
    };
    CHECK(fehlerVon(mitQuellen({QStringLiteral("salamander")})).empty());
    CHECK(fehlerVon(mitQuellen({QStringLiteral("salamander"), QStringLiteral("tu-note-violine")})).find("darf nichts ausgeliefert") !=
          std::string::npos);
    CHECK(fehlerVon(mitQuellen({QStringLiteral("gibt-es-nicht")})).find("nicht im Register") != std::string::npos);
    CHECK(fehlerVon(mitQuellen({})).find("nennt keine Quelle") != std::string::npos);
    QJsonObject fremd = mitQuellen({QStringLiteral("salamander")});
    QJsonArray schichten = fremd.value(QStringLiteral("layers")).toArray();
    QJsonObject s0 = schichten[0].toObject();
    s0[QStringLiteral("core")] = QStringLiteral("gibtEsNicht");
    schichten[0] = s0;
    fremd[QStringLiteral("layers")] = schichten;
    CHECK(fehlerVon(fremd).find("unbekannter Kern") != std::string::npos);
}
