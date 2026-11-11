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
 * `asset/loop/instruments/*.lvinst` und `asset/loop/instruments/CREDITS.md`. Gesehen werden je Instrument die
 * Schluessel der Kurven einer Kern-Schicht (`coreParams.<name>.sources`, Fassung 22) und die der Werte ohne Kurve
 * (`coreSources.<name>`, Fassung 23, QM1 a) — jeder Wert der Tafel muss dort stehen. Rot: ein unbekannter Schluessel,
 * eine Quelle mit `shippable: false`, ein Wert ohne Schluessel, eine Pflichtquelle, die in `CREDITS.md` fehlt — dann
 * `python asset/loop/make_instrument_credits.py` (schreibt die Datei ab der Marke neu).
 *
 * **Schreiben** (QM1 a): `LUMIVIZ_KERN_QUELLEN_SCHREIBEN=1` liest jede mitgelieferte Kern-Datei, setzt die Quellen der
 * Werte ohne Kurve aus der Vorlage gleichen Namens (`membrane`, `bar`; sonst die der Tafel) und schreibt sie mit dem
 * Schreiber des Programms — **ohne neu zu stimmen** (Werte, Abdruecke und Bezug bleiben; der Diff ist nur die Fassung
 * und `coreSources`).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/hammer/HammerKern.hpp"
#include "InstrumentModels/stab/StabKern.hpp"
#include "InstrumentModels/trommel/TrommelKern.hpp"
#include "InstrumentModels/zunge/ZungeKern.hpp"
#include "loop/Instrument.hpp"
#include "loop/KernDatei.hpp"

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
        // QM1 a: die Werte ohne Kurve — jeder Wert der Tafel steht in `coreSources` mit seinen Quellen
        const lumi::kern::KernTyp* typ = lumi::kern::kerne().typ(kern);
        if (typ == nullptr)
        {
            p.fehler.push_back(name + ": unbekannter Kern '" + kern + "'");
            continue;
        }
        const QJsonObject quellenJeWert = schicht.value(QStringLiteral("coreSources")).toObject();
        for (const lumi::kern::ParameterAngabe& a : typ->tafel.werte)
        {
            if (a.art == lumi::kern::WertArt::Kurve) continue;
            const std::string wo = name + " " + a.name;
            const QString schluessel = QString::fromStdString(a.name);
            if (!quellenJeWert.contains(schluessel))
            {
                p.fehler.push_back(wo + ": fehlt in coreSources");
                continue;
            }
            std::vector<std::string> quellen;
            for (const QJsonValue& q : quellenJeWert.value(schluessel).toArray()) quellen.push_back(q.toString().toStdString());
            if (quellen.empty()) p.fehler.push_back(wo + ": nennt keine Quelle");
            pruefeSchluessel(r, wo, quellen, p);
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
    // QM1 a: die Werte ohne Kurve des Fluegels mit der eigenen Vorgabe; `wert` bekommt `wertQuellen`
    const auto mitQuellen = [](const QJsonArray& quellen, const QString& wert = {}, const QJsonArray& wertQuellen = {}) {
        QJsonObject kurve{{QStringLiteral("points"), QJsonArray{QJsonArray{21, 1.0}}}, {QStringLiteral("sources"), quellen}};
        QJsonObject jeWert;
        for (const lumi::kern::ParameterAngabe& a : lumi::kern::kerne().typ("hammerString")->tafel.werte)
            if (a.art != lumi::kern::WertArt::Kurve) jeWert[QString::fromStdString(a.name)] = QJsonArray{QStringLiteral("lumiviz")};
        if (!wert.isEmpty()) jeWert[wert] = wertQuellen;
        QJsonObject schicht{{QStringLiteral("core"), QStringLiteral("hammerString")},
                            {QStringLiteral("coreParams"), QJsonObject{{QStringLiteral("decay"), kurve}}},
                            {QStringLiteral("coreSources"), jeWert}};
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
    // QM1 a: die Werte ohne Kurve — ND-Quelle, unbekannt, leer, fehlend
    const QJsonArray salamander{QStringLiteral("salamander")};
    const QString ungedaempft = QStringLiteral("undampedFrom");
    REQUIRE(lumi::kern::findeAngabe(lumi::kern::kerne().typ("hammerString")->tafel, "undampedFrom") != nullptr);
    CHECK(fehlerVon(mitQuellen(salamander, ungedaempft, salamander)).empty());
    CHECK(fehlerVon(mitQuellen(salamander, ungedaempft, {QStringLiteral("weinzierl2017")})).find("darf nichts ausgeliefert") != std::string::npos);
    CHECK(fehlerVon(mitQuellen(salamander, ungedaempft, {QStringLiteral("gibt-es-nicht")})).find("nicht im Register") != std::string::npos);
    CHECK(fehlerVon(mitQuellen(salamander, ungedaempft, {})).find("undampedFrom: nennt keine Quelle") != std::string::npos);
    {
        QJsonObject o = mitQuellen(salamander);
        QJsonArray schichten = o.value(QStringLiteral("layers")).toArray();
        QJsonObject s0 = schichten[0].toObject();
        QJsonObject jeWert = s0.value(QStringLiteral("coreSources")).toObject();
        jeWert.remove(ungedaempft);
        s0[QStringLiteral("coreSources")] = jeWert;
        schichten[0] = s0;
        o[QStringLiteral("layers")] = schichten;
        CHECK(fehlerVon(o).find("undampedFrom: fehlt in coreSources") != std::string::npos);
    }
    QJsonObject fremd = mitQuellen({QStringLiteral("salamander")});
    QJsonArray schichten = fremd.value(QStringLiteral("layers")).toArray();
    QJsonObject s0 = schichten[0].toObject();
    s0[QStringLiteral("core")] = QStringLiteral("gibtEsNicht");
    schichten[0] = s0;
    fremd[QStringLiteral("layers")] = schichten;
    CHECK(fehlerVon(fremd).find("unbekannter Kern") != std::string::npos);
}

namespace
{

/// Die Werte mit den Quellen, die eine mitgelieferte Kern-Datei tragen soll (QM1 a): die der Vorlage gleichen Namens
/// (`membrane`, `bar`), sonst die Vorgaben der Tafel (der Fluegel).
lumi::kern::Werte sollVon(const std::string& kern, const std::string& name)
{
    namespace kn = lumi::kern;
    if (kern == kn::trommel::kKernName)
        for (const kn::trommel::Vorlage& v : kn::trommel::vorlagen())
            if (v.name == name) return kn::trommel::werteVon(v);
    if (kern == kn::stab::kKernName)
        for (const kn::stab::Vorlage& v : kn::stab::vorlagen())
            if (v.name == name) return kn::stab::werteVon(v);
    if (kern == kn::hammer::kKernName)
        for (const kn::hammer::Vorlage& v : kn::hammer::vorlagen())
            if (v.name == name) return kn::hammer::werteVon(v);
    return kn::vorgaben(kn::kerne().typ(kern)->tafel);
}

}  // namespace

TEST_CASE("Instrument Quellen QM1 a: die Kern-Dateien tragen die Quellen ihrer Vorlage")
{
    namespace kn = lumi::kern;
    const bool schreiben = qEnvironmentVariableIntValue("LUMIVIZ_KERN_QUELLEN_SCHREIBEN") == 1;
    const QStringList dateien = QDir(kInstrumente).entryList({QStringLiteral("*.lvinst")}, QDir::Files, QDir::Name);
    REQUIRE_MESSAGE(!dateien.isEmpty(), "asset/loop/instruments nicht gefunden — aus dem Projekt-Root starten");
    int mitKern = 0;
    for (const QString& d : dateien)
    {
        const QString pfad = kInstrumente + QLatin1Char('/') + d;
        QJsonObject o = liesJson(pfad);
        QJsonArray schichten = o.value(QStringLiteral("layers")).toArray();
        bool kernDatei = false;
        for (qsizetype i = 0; i < schichten.size(); ++i)
        {
            QJsonObject s = schichten[i].toObject();
            if (!s.contains(QStringLiteral("core"))) continue;
            kernDatei = true;
            const std::string kern = s.value(QStringLiteral("core")).toString().toStdString();
            const std::string name = QFileInfo(d).completeBaseName().toStdString();
            CAPTURE(name);
            const kn::KernTyp* typ = kn::kerne().typ(kern);
            REQUIRE(typ != nullptr);
            const kn::Werte soll = sollVon(kern, name);
            // ohne neu zu stimmen: nur der Block `coreSources` (die Werte, Abdruecke und der Bezug bleiben, wie sie sind);
            // seit KL4 a (Fassung 24) dazu Werte, die der Datei fehlen, aus der Vorlage (am Fluegel `noteMin`/`noteMax`)
            if (schreiben)
            {
                s[QStringLiteral("coreSources")] = lumi::loop::quellenAlsJson(typ->tafel, soll);
                QJsonObject werte = s.value(QStringLiteral("coreParams")).toObject();
                const QJsonObject vorlage = lumi::loop::werteAlsJson(typ->tafel, soll);
                for (auto it = vorlage.begin(); it != vorlage.end(); ++it)
                    if (!werte.contains(it.key())) werte[it.key()] = it.value();
                s[QStringLiteral("coreParams")] = werte;
            }
            schichten[i] = s;
            CHECK_MESSAGE(s.value(QStringLiteral("coreSources")).toObject() == lumi::loop::quellenAlsJson(typ->tafel, soll),
                          "veraltet — LUMIVIZ_KERN_QUELLEN_SCHREIBEN=1, dann instrument_katalog.py und make_instrument_credits.py");
        }
        if (!kernDatei) continue;
        ++mitKern;
        // KL4 a (Fassung 24): kein Umfang im Block `keyboard`
        if (schreiben && o.contains(QStringLiteral("keyboard")))
        {
            QJsonObject tastatur = o.value(QStringLiteral("keyboard")).toObject();
            tastatur.remove(QStringLiteral("range"));
            o[QStringLiteral("keyboard")] = tastatur;
        }
        CHECK_FALSE(o.value(QStringLiteral("keyboard")).toObject().contains(QStringLiteral("range")));
        if (schreiben)
        {
            o[QStringLiteral("layers")] = schichten;
            o[QStringLiteral("version")] = lumi::loop::kInstrumentFassung;
            QFile aus(pfad);
            REQUIRE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate));
            REQUIRE(aus.write(QJsonDocument(o).toJson(QJsonDocument::Indented)) > 0);
            MESSAGE(d.toStdString() << " geschrieben — danach instrument_katalog.py");
        }
        CHECK(o.value(QStringLiteral("version")).toInt() == lumi::loop::kInstrumentFassung);
        // und das Programm liest sie so: je Wert ohne Kurve die Quellen der Datei
        lumi::loop::Instrument ist;
        REQUIRE(lumi::loop::ausJson(o, ist));
        for (const lumi::loop::Schicht& s : ist.schichten)
        {
            if (!lumi::loop::istKernSchicht(s)) continue;
            const kn::Parametertafel& tafel = kn::kerne().typ(s.kernName.toStdString())->tafel;
            const kn::Werte soll = sollVon(tafel.kern, QFileInfo(d).completeBaseName().toStdString());
            for (const kn::ParameterAngabe& a : tafel.werte)
            {
                if (a.art == kn::WertArt::Kurve) continue;
                CAPTURE(a.name);
                REQUIRE(s.kernWerte.quellenVon(a.name) != nullptr);
                CHECK(*s.kernWerte.quellenVon(a.name) == *soll.quellenVon(a.name));
            }
        }
    }
    CHECK(mitKern == 21);  // S181: dazu upright_piano, honky_tonk_piano, electric_grand_piano, clavinet_modal (34i); S180: grand_piano, neun Stabspiele (34g), sieben Trommeln (34h)
}

TEST_CASE("Instrument Quellen QM1 a: Quellen am Wert — richten, vergleichen, Datei")
{
    namespace kn = lumi::kern;
    const kn::Parametertafel& tafel = kn::trommel::kernTyp().tafel;
    const kn::trommel::Vorlage& pauke = kn::trommel::vorlagen()[1];
    REQUIRE(pauke.name == "timpani");
    kn::Werte w = kn::trommel::werteVon(pauke);
    REQUIRE(w.quellenVon("shellVolume") != nullptr);
    CHECK(*w.quellenVon("shellVolume") == std::vector<std::string>{"packer1993"});
    // die Tafel traegt die der ersten Vorlage (Tom): dort ist der Kessel die eigene Vorgabe
    CHECK(kn::findeAngabe(tafel, "shellVolume")->quellen == std::vector<std::string>{"lumiviz"});
    // die Quellen zaehlen nicht zum Vergleich (wie die Herkunft einer Kurve)
    kn::Werte ohne = w;
    ohne.quellen.clear();
    CHECK(ohne == w);
    // richten: fehlende aus der Tafel, vorhandene bleiben, in der Reihenfolge der Tafel, keine an Kurven und Listeneintraegen
    ohne.setzeQuellen("velocityMax", {"torin2014"});
    ohne.setzeQuellen("gibtEsNicht", {"lumiviz"});
    CHECK(kn::richteWerte(tafel, ohne).empty());  // eine Quelle zu einem unbekannten Namen faellt still weg (kein Wert)
    REQUIRE(ohne.quellenVon("velocityMax") != nullptr);
    CHECK(*ohne.quellenVon("velocityMax") == std::vector<std::string>{"torin2014"});
    CHECK(*ohne.quellenVon("shellVolume") == std::vector<std::string>{"lumiviz"});
    CHECK(ohne.quellenVon("gibtEsNicht") == nullptr);
    std::size_t ohneKurve = 0;
    for (const kn::ParameterAngabe& a : tafel.werte) ohneKurve += a.art != kn::WertArt::Kurve ? 1 : 0;
    CHECK(ohne.quellen.size() == ohneKurve);
    for (const kn::Werte& e : ohne.liste("heads").eintraege) CHECK(e.quellen.empty());
    // die Datei: `coreSources` hin und zurueck, auch leer
    w.setzeQuellen("poisson", {});
    const QJsonObject j = lumi::loop::quellenAlsJson(tafel, w);
    CHECK(j.size() == static_cast<qsizetype>(ohneKurve));
    CHECK(j.value(QStringLiteral("poisson")).toArray().isEmpty());
    kn::Werte zurueck = kn::trommel::werteAus(pauke.spiel);
    lumi::loop::quellenAusJson(tafel, j, zurueck);
    CHECK(zurueck.quellen == w.quellen);
}

TEST_CASE("Instrument Quellen 34j-6: die Vorlagen des Kerns freeReed nennen nur Schluessel aus dem Register")
{
    // die Dateien kommen mit 34j-8; bis dahin prueft der Waechter die Vorlagen selbst — jede Kurve je Taste und jeder
    // Wert ohne Kurve nennt bekannte, auslieferbare Quellen
    const Register r = liesRegister();
    REQUIRE(!r.empty());
    Pruefung p;
    for (const lumi::kern::zunge::Vorlage& v : lumi::kern::zunge::vorlagen())
    {
        const lumi::kern::Werte w = lumi::kern::zunge::werteVon(v);
        for (const auto& [name, wert] : w.felder)
            if (const auto* k = std::get_if<lumi::kern::Kurve>(&wert); k != nullptr && !k->punkte.empty())
            {
                if (k->quellen.empty()) p.fehler.push_back(v.name + " " + name + ": nennt keine Quelle");
                pruefeSchluessel(r, v.name + " " + name, k->quellen, p);
            }
        for (const auto& [name, quellen] : w.quellen)
        {
            if (quellen.empty()) p.fehler.push_back(v.name + " " + name + ": nennt keine Quelle");
            pruefeSchluessel(r, v.name + " " + name, quellen, p);
        }
    }
    CHECK_MESSAGE(p.fehler.empty(), zusammen(p.fehler));
    MESSAGE(p.benutzt.size() << " Quellen in den Vorlagen der Zunge");
    CHECK(p.benutzt.count("adachi2026") == 1);
    CHECK(p.benutzt.count("millot2007") == 1);
}
