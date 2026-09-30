/**
 ****************************************************************************************
 * @file   test_FieldBindings.cpp
 * @brief  Feldtabelle (Block F, Stufe F1): jeder Eintrag trifft genau sein Feld
 *
 * @details
 * Die Tabelle ist erzeugt (`harvest_field_docs.py`); ihr Zugriff ueber
 * `std::get<T>(p).member` macht ein umbenanntes Feld zum Compilerfehler. Was der
 * Compiler NICHT sieht: ob der JSON-Name eines Eintrags zum Member passt (die
 * JSON-Aliasse `ftype`←`type`, `overrideBlend`←`enabled` …), ob der Typ des
 * Eintrags zum Varianten-Index passt, und ob die Typpflege stimmt. Das prueft
 * dieser Test am Serializer als unabhaengiger Quelle: Wert ueber die Tabelle
 * setzen, `nodeToJson`, und der Wert muss GENAU unter dem JSON-Namen stehen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/FieldBindings.hpp"
#include "visualizers/multieffect/ScriptVariables.hpp"

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

#include <cmath>
#include <limits>
#include <set>
#include <string>
#include <variant>

using namespace lumi::multieffect;
namespace ft = lumi::multieffect::feldtabelle;

namespace
{

/// Ein zulaessiger Wert, der sich von der Vorgabe unterscheidet (NaN = keiner).
double gegenwert(const ft::Feld& f, double vorgabe)
{
    switch (f.art)
    {
    case ft::Art::Schalter:
        return vorgabe >= 0.5 ? 0.0 : 1.0;
    case ft::Art::Farbe:
        return vorgabe == 0x123456 ? 0x654321 : 0x123456;
    case ft::Art::Aufzaehlung:
        if (f.hi <= f.lo) return std::nan("");
        return vorgabe == f.lo ? f.hi : f.lo;
    case ft::Art::Ganzzahl:
    case ft::Art::Kommazahl:
    {
        double v = std::isnan(f.lo) ? vorgabe + 1.0 : f.lo + (f.hi - f.lo) * 0.37;
        if (f.art == ft::Art::Ganzzahl) v = std::round(v);
        if (v == vorgabe) v = std::isnan(f.hi) ? vorgabe + 2.0 : (v == f.hi ? f.lo : f.hi);
        return v;
    }
    }
    return std::nan("");
}

double jsonZahl(const QJsonValue& v)
{
    return v.isBool() ? (v.toBool() ? 1.0 : 0.0) : v.toDouble(std::nan(""));
}

}  // namespace

TEST_SUITE("FieldBindings")
{
    TEST_CASE("Tabelle: Umfang, Index und Typschluessel passen zusammen")
    {
        const auto& alle = ft::alle();
        std::size_t felder = 0;
        for (const ft::Typ* t : alle) felder += t->felder.size();
        CHECK(alle.size() > 80);   // Schutz gegen eine Tabelle, die still schrumpft
        CHECK(felder > 450);
        for (std::size_t i = 0; i < std::variant_size_v<EffectParams>; ++i)
        {
            const ft::Typ* t = ft::typ(i);
            if (t == nullptr) continue;
            // Der Eintrag unter Index i beschreibt wirklich den Typ mit Index i.
            const EffectParams p = [i] {
                EffectParams out;
                // Varianten-Alternative i auf Vorgabe konstruieren
                [&]<std::size_t... I>(std::index_sequence<I...>) {
                    ((I == i ? (out.emplace<I>(), true) : false) || ...);
                }(std::make_index_sequence<std::variant_size_v<EffectParams>>{});
                return out;
            }();
            CHECK(effectTypeKey(p).toStdString() == std::string(t->typKey));
        }
    }

    TEST_CASE("Namen: je Typ eindeutig, klein, Alias kollidiert nicht")
    {
        for (const ft::Typ* t : ft::alle())
        {
            std::set<std::string> namen;
            for (const ft::Feld& f : t->felder)
            {
                CAPTURE(std::string(t->typKey) + "." + std::string(f.feld));
                const std::string n(f.name);
                CHECK(QString::fromStdString(n) == QString::fromStdString(n).toLower());
                CHECK(namen.insert(n).second);
                if (!f.alias.empty()) CHECK(namen.insert(std::string(f.alias)).second);
            }
        }
    }

    TEST_CASE("Jeder Eintrag schreibt GENAU sein Feld (Gegenwert steht im JSON)")
    {
        int geprueft = 0;
        for (std::size_t i = 0; i < std::variant_size_v<EffectParams>; ++i)
        {
            const ft::Typ* t = ft::typ(i);
            if (t == nullptr) continue;
            for (const ft::Feld& f : t->felder)
            {
                CAPTURE(std::string(t->typKey) + "." + std::string(f.feld));
                ChainNode n;
                [&]<std::size_t... I>(std::index_sequence<I...>) {
                    ((I == i ? (n.params.emplace<I>(), true) : false) || ...);
                }(std::make_index_sequence<std::variant_size_v<EffectParams>>{});

                const QJsonObject vorher = nodeToJson(n);
                const QString key = QString::fromUtf8(f.feld.data(),
                                                      static_cast<int>(f.feld.size()));
                REQUIRE(vorher.contains(key));
                const double vorgabe = f.lesen(n.params);
                CHECK(jsonZahl(vorher.value(key)) == doctest::Approx(vorgabe));

                const double soll = gegenwert(f, vorgabe);
                if (std::isnan(soll)) continue;
                f.schreiben(n.params, soll);
                CHECK(f.lesen(n.params) == doctest::Approx(soll));
                const QJsonObject nachher = nodeToJson(n);
                CHECK(jsonZahl(nachher.value(key)) == doctest::Approx(soll));
                // Gegenprobe: kein ANDERES Feld hat sich mitbewegt.
                for (auto it = vorher.constBegin(); it != vorher.constEnd(); ++it)
                    if (it.key() != key)
                        CHECK_MESSAGE(nachher.value(it.key()) == it.value(),
                                      "mitbewegt: " << it.key().toStdString());
                ++geprueft;
            }
        }
        CHECK(geprueft > 400);
    }

    // Entscheid Patrik S85: Ganzzahlen und Aufzaehlungen werden ABGESCHNITTEN
    // (Richtung 0, wie `(int)` in EEL/AVS und wie die Renderer vor der
    // F2-Migration) — bis S85 rundete die Tabelle (2.6 -> 3).
    TEST_CASE("Typpflege: abschneiden, Schalter-Schwelle, Aufzaehlung als Index")
    {
        int i = 0;
        ft::setze(i, 2.6);
        CHECK(i == 2);
        ft::setze(i, -2.6);
        CHECK(i == -2);
        ft::setze(i, std::numeric_limits<double>::quiet_NaN());
        CHECK(i == -2);  // nicht-endlich: Feld bleibt stehen
        ft::setze(i, 1e300);
        CHECK(i == std::numeric_limits<int>::max());
        bool b = false;
        ft::setze(b, 0.49);
        CHECK_FALSE(b);
        ft::setze(b, 0.5);
        CHECK(b);
        BlendMode m = BlendMode::Replace;
        ft::setze(m, 1.9);
        CHECK(ft::lese(m) == doctest::Approx(1.0));
        float x = 0.0F;
        ft::setze(x, 0.25);
        CHECK(x == doctest::Approx(0.25));
    }

    TEST_CASE("Referenz (F5): jedes Feld eines ParamSkript-Typs steht in scriptVariables")
    {
        int typen = 0;
        for (std::size_t i = 0; i < std::variant_size_v<EffectParams>; ++i)
        {
            const ft::Typ* t = ft::typ(i);
            if (t == nullptr || t->traeger != ft::Traeger::ParamSkript) continue;
            EffectParams p;
            [&]<std::size_t... I>(std::index_sequence<I...>) {
                ((I == i ? (p.emplace<I>(), true) : false) || ...);
            }(std::make_index_sequence<std::variant_size_v<EffectParams>>{});
            const ScriptVarTable* tab = scriptVariables(p);
            REQUIRE(tab != nullptr);
            std::set<std::string> namen;
            for (const ScriptVar& v : tab->vars)
                for (const std::string& n : splitScriptVarNames(v.names)) namen.insert(n);
            for (const ft::Feld& f : t->felder)
            {
                const std::string n(f.name);
                const std::string probe = f.art == ft::Art::Farbe ? n + "_r" : n;
                CHECK_MESSAGE(namen.count(probe) == 1,
                              std::string(t->typKey) << ": " << probe << " fehlt in der Referenz");
            }
            ++typen;
        }
        CHECK(typen > 45);
    }

    TEST_CASE("Klassen und Traeger: die Stichproben aus dem Konzept")
    {
        const auto finde = [](std::string_view typKey, std::string_view feld) -> const ft::Feld* {
            for (const ft::Typ* t : ft::alle())
                if (t->typKey == typKey)
                    for (const ft::Feld& f : t->felder)
                        if (f.feld == feld) return &f;
            return nullptr;
        };
        const ft::Feld* grid = finde("meshWarp", "gridX");
        REQUIRE(grid != nullptr);
        CHECK(grid->klasse == ft::Klasse::Struktur);
        const ft::Feld* mix = finde("meshWarp", "mixAmount");
        REQUIRE(mix != nullptr);
        CHECK(mix->klasse == ft::Klasse::Live);
        const ft::Feld* alphain = finde("list", "inAdjustAlpha");
        REQUIRE(alphain != nullptr);
        CHECK(alphain->alias == "alphain");
        CHECK(alphain->aliasSkala == doctest::Approx(1.0 / 255.0));
        const ft::Feld* ob = finde("setRenderMode", "overrideBlend");
        REQUIRE(ob != nullptr);
        CHECK(ob->name == "overrideblend");  // bisheriger Skriptname = Schema-Name
    }
}
