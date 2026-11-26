/**
 ****************************************************************************************
 * @file   test_InstrumentModelsZupfBezug.cpp
 * @brief  Stufe 34k-7d: der Kern `pluckedString` Rechenfassung 2 gegen die Bezugsdatei des Pruefstands
 *         (`werte/zupf_pruefstand.txt`, geschrieben von `docs/modellierung/gezupfte-saite/proben/2026-11-26_S222_bezug_zupf.py`;
 *         docs/audio/Instrumente_IC_Konzept.md Schnittfolge 34k-7d)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton.
 *
 * - 34k-7d-3: die Saiten der Vorgaben (`InstrumentModels/zupf/Saiten`) — je Fall der Bezugsdatei die Werte jeder Saite
 *   und ihre schwingende Laenge, auf 1e-12 relativ.
 * - 34k-7d-3: die Tafel des Umlauf-Filters als Liste (`InstrumentModels/zupf/UmlaufTafel`) hin und zurueck, als
 *   gestimmter Wert gerichtet und durch `coreParams`; das Filter daraus gleich dem des Entwurfs.
 * - Schreiber `LUMIVIZ_ZUPF_UMLAUF_SCHREIBEN=alle` oder `<instrument>[:<von>-<bis>],…` (`harpsichord`, `concert_harp`:
 *   Tasten; `classical_guitar`, `lute`, `oud`: Choere ab 1 von der tiefsten an): entwirft die Tafel und schreibt
 *   `werte/zupf_umlauf.json` (je Instrument die Liste wie in `coreParams`) nach jeder Taste bzw. jedem Chor, eine Zeile je
 *   Stelle ungepuffert nach stderr (S222: ein Absturz des Rechners mitten im Lauf liess nichts uebrig). Alle fuenf rund 2 h.
 *   Der Fall „Tafel aus werte/zupf_umlauf.json“ misst jede Stelle daraus nach UL2 a.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Pool.hpp"
#include "InstrumentMeasure/UmlaufEntwurf.hpp"
#include "InstrumentMeasure/ZupfStimmen.hpp"
#include "InstrumentModels/zupf/Saiten.hpp"
#include "InstrumentModels/zupf/UmlaufTafel.hpp"
#include "loop/KernDatei.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace
{
namespace kz = lumi::kern::zupf;

/// Ein Fall der Bezugsdatei (was 34k-7d-3 davon liest).
struct Fall
{
    std::string name, instrument;
    int taste = 0, stegpunkt = 0;
    double stufe = 0.0;
    /// je Saite die Felder der Zeile `saite` (Name → Zahl)
    std::vector<std::map<std::string, double>> saiten;
};

std::vector<Fall> liesBezug()
{
    std::ifstream f(std::filesystem::path(__FILE__).parent_path() / "werte" / "zupf_pruefstand.txt");
    REQUIRE(f.good());
    std::vector<Fall> faelle;
    std::string zeile;
    while (std::getline(f, zeile))
    {
        if (zeile.empty() || zeile[0] == '#') continue;
        std::istringstream z(zeile);
        std::string art;
        z >> art;
        if (art == "fall")
        {
            Fall x;
            std::string w;
            z >> x.name >> x.instrument >> w >> x.taste >> w >> x.stufe >> w >> x.stegpunkt;
            faelle.push_back(x);
        }
        else if (art == "saite")
        {
            REQUIRE(!faelle.empty());
            std::map<std::string, double> m;
            std::string index, name;
            z >> index;
            double wert = 0.0;
            while (z >> name >> wert) m[name] = wert;
            faelle.back().saiten.push_back(m);
        }
    }
    return faelle;
}

/// Die Saiten eines Falls aus den Vorgaben: Werte und schwingende Laenge, in der Reihenfolge der Bezugsdatei.
struct Saite
{
    kz::Saitenwerte werte;
    double laenge = 0.0;
};

std::vector<Saite> saitenVon(const Fall& f, int& stegpunkt)
{
    std::vector<Saite> out;
    if (f.instrument == "harfe")
    {
        const auto t = kz::harfenSaite(f.taste);
        REQUIRE(t);
        out.push_back({t->werte, t->laenge});
        stegpunkt = t->stegTaste;
    }
    else if (f.instrument == "cembalo")
    {
        const kz::Tastensaite t = kz::cembaloSaite(f.taste);
        out.push_back({t.werte, t.laenge});
        stegpunkt = t.stegTaste;
    }
    else
    {
        const kz::Saitensatz satz = f.instrument == "gitarre" ? kz::saitensatzGitarre()
                                  : f.instrument == "laute" ? kz::saitensatzLaute()
                                                            : kz::saitensatzOud();
        // ohne Angabe der Saite: die hoechste, deren leere Taste nicht ueber der Note liegt (wie der Pruefstand)
        std::size_t c = 0;
        for (std::size_t i = 0; i < satz.choere.size(); ++i)
            if (satz.choere[i].leer <= f.taste) c = i;
        const double laenge = satz.mensur * std::exp2(-(f.taste - satz.choere[c].leer) / 12.0);
        for (const kz::Chorsaite& s : satz.choere[c].saiten) out.push_back({s.werte, laenge * s.laengenFaktor});
        stegpunkt = satz.choere[c].leer;
    }
    return out;
}

void gleich(double ist, double soll, const char* was)
{
    CAPTURE(was);
    CHECK(ist == doctest::Approx(soll).epsilon(1e-12));
}

}  // namespace

TEST_CASE("InstrumentModels 34k-7d-3 die Saiten der Vorgaben gegen den Bezug des Pruefstands")
{
    const std::vector<Fall> faelle = liesBezug();
    CHECK(faelle.size() == 17);
    for (const Fall& f : faelle)
    {
        CAPTURE(f.name);
        int stegpunkt = -1;
        const std::vector<Saite> saiten = saitenVon(f, stegpunkt);
        CHECK(stegpunkt == f.stegpunkt);
        REQUIRE(saiten.size() == f.saiten.size());
        for (std::size_t i = 0; i < saiten.size(); ++i)
        {
            CAPTURE(i);
            const kz::Saitenwerte& s = saiten[i].werte;
            const std::map<std::string, double>& b = f.saiten[i];
            gleich(saiten[i].laenge, b.at("laenge"), "laenge");
            gleich(s.zug, b.at("T"), "T");
            gleich(s.masse, b.at("m"), "m");
            gleich(s.biegesteife, b.at("EI"), "EI");
            gleich(s.verlustBiegung, b.at("etaB"), "etaB");
            // monofil (Durchmesser, Dichte) oder nach Valette (η_F, η_A) — was die Bezugsdatei nicht nennt, ist 0
            gleich(s.durchmesser, b.count("d") ? b.at("d") : 0.0, "d");
            gleich(s.dichte, b.count("rho") ? b.at("rho") : 0.0, "rho");
            gleich(s.verlustInnen, b.count("etaF") ? b.at("etaF") : 0.0, "etaF");
            gleich(s.verlustLuft, b.count("etaA") ? b.at("etaA") : 0.0, "etaA");
        }
    }
}

namespace
{
namespace ku = lumi::kern::umlauf;
namespace mu = lumi::messung::umlauf;
using lumi::kern::Liste;
using lumi::kern::Parametertafel;
using lumi::kern::ParameterAngabe;
using lumi::kern::Werte;

/// Eine Tafel nur mit der gestimmten Liste (bis 34k-7d-4 die Tafel des Kerns sie traegt).
Parametertafel tafelDerListe()
{
    Parametertafel t;
    t.kern = "pluckedString";
    t.titel = "Plucked string";
    t.noteMin = 21;
    t.noteMax = 108;
    t.gruppen = {"Tunable"};
    ParameterAngabe a = kz::umlaufTafelAngabe();
    a.gruppe = "Tunable";
    a.gestimmt = true;
    t.werte.push_back(a);
    return t;
}

std::filesystem::path umlaufDatei() { return std::filesystem::path(__FILE__).parent_path() / "werte" / "zupf_umlauf.json"; }

/// Die Instrumente in der Reihenfolge der Vorlagen: je Taste (Umfang) oder je Saite (Saitensatz).
struct Instrument
{
    std::string name;
    int von = 0, bis = -1;  ///< je Taste
    std::function<std::optional<kz::Tastensaite>(int)> saite;
    std::function<kz::Saitensatz()> satz;  ///< je Saite
};

const std::vector<Instrument>& instrumente()
{
    static const std::vector<Instrument> v = {
        {"harpsichord", 29, 89, [](int t) { return std::optional<kz::Tastensaite>(kz::cembaloSaite(t)); }, {}},
        {"concert_harp", 25, 104, [](int t) { return kz::harfenSaite(t); }, {}},
        {"classical_guitar", 0, -1, {}, [] { return kz::saitensatzGitarre(); }},
        {"lute", 0, -1, {}, [] { return kz::saitensatzLaute(); }},
        {"oud", 0, -1, {}, [] { return kz::saitensatzOud(); }},
    };
    return v;
}

/// UL2 a, bekannte Ausnahme (Herleitung §9): Laute Chor 8.1 (G4), bis 12,2 Cent an Teiltoenen ueber 16 kHz.
bool bekannteAusnahme(const std::string& instrument, int chor, int saite) { return instrument == "lute" && chor == 7 && saite == 0; }

}  // namespace

TEST_CASE("InstrumentModels 34k-7d-3 Tafel des Umlauf-Filters als Liste: hin und zurueck, gerichtet, durch coreParams")
{
    // je Taste: zwei Tasten der Harfe aus dem Entwurf (je rund eine Sekunde)
    const lumi::messung::UmlaufTafelEntwurf e = lumi::messung::entwirfUmlaufJeTaste(46, 47, [](int t) { return kz::harfenSaite(t); });
    REQUIRE(e.eintraege.size() == 2);
    CHECK(e.fehler.empty());
    for (const auto& [name, b] : e.bewertungen)
    {
        CAPTURE(name);
        CHECK(b.haelt());
    }
    // je Saite: eine Tafel von Hand (den Entwurf je Saite, Minuten, rechnet der Schreiber)
    kz::UmlaufEintrag s;
    s.chor = 2;
    s.saite = 1;
    s.tafel.grad = 3;
    s.tafel.radiusReihe = {{0.1, 0.2, -0.3, 0.04}, {-1.5, 0.5, 0.0, 0.25}};
    s.tafel.winkelReihe = {{-2.0, 0.1, 0.02, -0.01}, {-0.5, -0.2, 0.3, 0.0}};
    s.tafel.verlust = {{0.0, {0.99, 0.01}, {1.0, 0.002}}, {0.5, {0.98, 0.012}, {1.0, 0.003}}, {1.0, {0.97, 0.02}, {1.0, 0.004}}};
    std::vector<kz::UmlaufEintrag> eintraege = e.eintraege;
    eintraege.push_back(s);

    const Liste l = kz::umlaufTafelListe(eintraege);
    CHECK(kz::umlaufTafelEintraege(l) == eintraege);
    // als gestimmter Wert gerichtet (0.8.0: eine gestimmte Liste)
    const Parametertafel t = tafelDerListe();
    REQUIRE(lumi::kern::pruefeTafel(t).empty());
    Werte w;
    w.setze(std::string(kz::kUmlaufTafel), l);
    CHECK(lumi::kern::richteWerte(t, w).empty());
    CHECK(kz::umlaufTafelEintraege(w.liste(kz::kUmlaufTafel)) == eintraege);
    // durch `coreParams` der .lvinst und zurueck
    const Werte z = lumi::loop::werteAusJson(t, lumi::loop::werteAlsJson(t, w));
    CHECK(z.felder == w.felder);
    // je Taste: der Kern stimmt das Glied aus der Liste — es haelt UL2 a wie im Entwurf
    for (std::size_t i = 0; i < 2; ++i)
    {
        const auto saite = kz::harfenSaite(46 + static_cast<int>(i));
        REQUIRE(saite);
        ku::Umlauffilter f = kz::umlaufTafelEintraege(z.liste(kz::kUmlaufTafel))[i].filter;
        REQUIRE(ku::stimmeAmGrundton(f, saite->werte, saite->laenge));
        mu::Umlauffilter g = f;
        const mu::Bewertung b = mu::bewerte(saite->werte, saite->laenge, g);
        CHECK(b.haelt());
        CHECK(g == f);
        CHECK(b.centGroesster == e.bewertungen[i].second.centGroesster);
    }
}

TEST_CASE("InstrumentModels 34k-7d-3 Schreiber der Tafel (LUMIVIZ_ZUPF_UMLAUF_SCHREIBEN)")
{
    const char* was = std::getenv("LUMIVIZ_ZUPF_UMLAUF_SCHREIBEN");
    if (was == nullptr || std::string(was).empty())
    {
        MESSAGE("LUMIVIZ_ZUPF_UMLAUF_SCHREIBEN=<auswahl> entwirft die Tafel des Umlauf-Filters in werte/zupf_umlauf.json: "
                "alle | <instrument>[:<von>-<bis>],... (je Taste die Tasten, je Saite die Choere ab 1 von der tiefsten an), "
                "geschrieben nach jeder Taste bzw. jedem Chor (alle fuenf zusammen rund 2 h)");
        return;
    }
    // die Auswahl: je Instrument ein Bereich (je Taste Tasten, je Saite Choere ab 1); ohne Bereich das ganze Instrument
    struct Stueck
    {
        const Instrument* instrument;
        int von, bis;
    };
    std::vector<Stueck> stuecke;
    {
        std::string rest = was;
        while (!rest.empty())
        {
            const std::size_t komma = rest.find(',');
            const std::string teil = rest.substr(0, komma);
            rest = komma == std::string::npos ? std::string() : rest.substr(komma + 1);
            const std::size_t doppel = teil.find(':');
            const std::string name = teil.substr(0, doppel);
            for (const Instrument& i : instrumente())
            {
                if (name != "alle" && name != i.name) continue;
                int von = i.satz ? 1 : i.von, bis = i.satz ? static_cast<int>(i.satz().choere.size()) : i.bis;
                if (doppel != std::string::npos)
                {
                    const std::string bereich = teil.substr(doppel + 1);
                    const std::size_t strich = bereich.find('-');
                    von = std::stoi(bereich.substr(0, strich));
                    bis = strich == std::string::npos ? von : std::stoi(bereich.substr(strich + 1));
                }
                stuecke.push_back({&i, von, bis});
            }
        }
    }
    REQUIRE_MESSAGE(!stuecke.empty(), "unbekannte Auswahl: " << was);
    // S222: weniger Threads gegen die Waerme (der Rechner stuerzte unter 11 ab); die Ergebnisse haengen nicht daran
    if (const char* n = std::getenv("LUMIVIZ_POOL_THREADS"); n != nullptr && *n != '\0')
        lumi::kern::setzePoolHoechstens(static_cast<unsigned>(std::stoul(n)));
    std::fprintf(stderr, "Threads: hoechstens %u\n", lumi::kern::poolHoechstens());
    const Parametertafel t = tafelDerListe();
    const QString pfad = QString::fromStdString(umlaufDatei().string());

    // je Taste bzw. Chor: entwerfen, in die Tafel des Instruments mischen (eine Stelle ersetzt dieselbe), sofort schreiben
    const auto mische = [&](const std::string& instrument, const std::vector<kz::UmlaufEintrag>& neu) {
        QJsonObject alle;
        {
            QFile f(pfad);
            if (f.open(QIODevice::ReadOnly)) alle = QJsonDocument::fromJson(f.readAll()).object();
        }
        QJsonObject inst = alle.value(QStringLiteral("instruments")).toObject();
        std::vector<kz::UmlaufEintrag> eintraege;
        const QString schluessel = QString::fromStdString(instrument);
        if (inst.contains(schluessel))
            eintraege = kz::umlaufTafelEintraege(lumi::loop::werteAusJson(t, inst.value(schluessel).toObject()).liste(kz::kUmlaufTafel));
        for (const kz::UmlaufEintrag& u : neu)
        {
            std::erase_if(eintraege, [&](const kz::UmlaufEintrag& x) { return x.taste == u.taste && x.chor == u.chor && x.saite == u.saite; });
            eintraege.push_back(u);
        }
        std::sort(eintraege.begin(), eintraege.end(), [](const kz::UmlaufEintrag& a, const kz::UmlaufEintrag& b) {
            return std::tie(a.taste, a.chor, a.saite) < std::tie(b.taste, b.chor, b.saite);
        });
        Werte w;
        w.setze(std::string(kz::kUmlaufTafel), kz::umlaufTafelListe(eintraege));
        REQUIRE(lumi::kern::richteWerte(t, w).empty());
        inst.insert(schluessel, lumi::loop::werteAlsJson(t, w));
        alle.insert(QStringLiteral("format"), 1);
        alle.insert(QStringLiteral("written by"),
                    QStringLiteral("test_InstrumentModelsZupfBezug.cpp, LUMIVIZ_ZUPF_UMLAUF_SCHREIBEN (34k-7d-3) - do not edit by hand"));
        alle.insert(QStringLiteral("instruments"), inst);
        QSaveFile f(pfad);  // erst ganz geschrieben, dann ersetzt: ein Absturz laesst die alte Datei stehen
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write(QJsonDocument(alle).toJson(QJsonDocument::Indented));
        REQUIRE(f.commit());
    };
    for (const Stueck& s : stuecke)
        for (int k = s.von; k <= s.bis; ++k)
        {
            const Instrument& i = *s.instrument;
            const lumi::messung::UmlaufTafelEntwurf e = i.satz ? lumi::messung::entwirfUmlaufJeSaite(i.satz(), k - 1, k - 1)
                                                                : lumi::messung::entwirfUmlaufJeTaste(k, k, i.saite);
            if (e.eintraege.empty() && e.fehler.empty()) continue;  // eine Taste ohne Saite
            std::string zeile = i.name + (i.satz ? " Chor " : " T") + std::to_string(k) + ": " + std::to_string(e.sekunden) + " s";
            for (const auto& [name, b] : e.bewertungen)
                zeile += " | " + name + (b.haelt() ? " haelt " : " NICHT ") + std::to_string(b.centGroesster) + " Cent " +
                         std::to_string(b.abklingenGroesster) + " %";
            if (!e.fehler.empty()) zeile += " | OHNE ENTWURF";
            std::fprintf(stderr, "%s\n", zeile.c_str());  // ungepuffert: nach einem Absturz steht da, wie weit es kam
            std::fflush(stderr);
            CHECK(e.fehler.empty());
            mische(i.name, e.eintraege);
        }
    MESSAGE("geschrieben: " << umlaufDatei().string());
}

TEST_CASE("InstrumentModels 34k-7d-3 Tafel aus werte/zupf_umlauf.json: jede Stelle haelt UL2 a")
{
    QFile datei(QString::fromStdString(umlaufDatei().string()));
    if (!datei.open(QIODevice::ReadOnly))
    {
        MESSAGE("werte/zupf_umlauf.json fehlt noch - LUMIVIZ_ZUPF_UMLAUF_SCHREIBEN=alle schreibt sie");
        return;
    }
    const QJsonObject inst = QJsonDocument::fromJson(datei.readAll()).object().value(QStringLiteral("instruments")).toObject();
    const Parametertafel t = tafelDerListe();
    for (const Instrument& i : instrumente())
    {
        CAPTURE(i.name);
        if (!inst.contains(QString::fromStdString(i.name)))
        {
            MESSAGE(i.name << ": noch nicht in der Datei");
            continue;
        }
        const Werte w = lumi::loop::werteAusJson(t, inst.value(QString::fromStdString(i.name)).toObject());
        const std::vector<kz::UmlaufEintrag> eintraege = kz::umlaufTafelEintraege(w.liste(kz::kUmlaufTafel));
        int stellen = 0;
        if (!i.satz)
        {
            // je Taste: jede Taste mit Saite hat einen Eintrag, und das Filter daraus haelt UL2 a
            for (int taste = i.von; taste <= i.bis; ++taste)
            {
                CAPTURE(taste);
                const auto saite = i.saite(taste);
                if (!saite) continue;
                const auto it = std::find_if(eintraege.begin(), eintraege.end(), [&](const kz::UmlaufEintrag& u) { return u.taste == taste; });
                CHECK_MESSAGE(it != eintraege.end(), "die Taste fehlt in der Tafel");
                if (it == eintraege.end()) continue;
                ku::Umlauffilter f = it->filter;
                REQUIRE(ku::stimmeAmGrundton(f, saite->werte, saite->laenge));
                CHECK(mu::bewerte(saite->werte, saite->laenge, f).haelt());
                ++stellen;
            }
        }
        else
        {
            // je Saite: an jedem Bund und auf halbem Weg
            const kz::Saitensatz satz = i.satz();
            for (const kz::UmlaufEintrag& u : eintraege)
            {
                CAPTURE(u.chor);
                CAPTURE(u.saite);
                REQUIRE(u.chor >= 0);
                REQUIRE(static_cast<std::size_t>(u.chor) < satz.choere.size());
                ku::Saitentafel tafel = u.tafel;
                tafel.buende = satz.buende;
                tafel.mensur = satz.mensur;
                const kz::Saitenwerte& werte = satz.choere[static_cast<std::size_t>(u.chor)].saiten[static_cast<std::size_t>(u.saite)].werte;
                for (int k = 0; k <= 2 * satz.buende; ++k)
                {
                    const double lage = 0.5 * k;
                    CAPTURE(lage);
                    const std::optional<ku::Umlauffilter> f = ku::filterAn(tafel, werte, lage);
                    REQUIRE(f);
                    mu::Umlauffilter g = *f;
                    const mu::Bewertung b = mu::bewerte(werte, satz.mensur * std::exp2(-lage / 12.0), g);
                    if (bekannteAusnahme(i.name, u.chor, u.saite))
                        CHECK(b.auswertbar);
                    else
                        CHECK(b.haelt());
                    ++stellen;
                }
            }
            // jede Saite des Satzes hat eine Tafel (im Einklang die der ersten)
            for (std::size_t c = 0; c < satz.choere.size(); ++c)
                CHECK(std::any_of(eintraege.begin(), eintraege.end(), [&](const kz::UmlaufEintrag& u) { return u.chor == static_cast<int>(c) && u.saite == 0; }));
        }
        MESSAGE(i.name << ": " << eintraege.size() << " Eintraege, " << stellen << " Stellen gemessen");
    }
}
