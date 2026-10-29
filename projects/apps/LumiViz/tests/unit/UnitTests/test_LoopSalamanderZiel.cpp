/**
 ****************************************************************************************
 * @file   test_LoopSalamanderZiel.cpp
 * @brief  Loop, Stufe 31d-7 B, 7.5.9: das Ziel der Suche — Verhaeltnis zur Toleranz mit Untergrenze, Summe je Taste,
 *         die Kurven der Suche und der Ton des Modells an den Ebenen v6, v10, v16 (docs/audio/Spielweisen_Konzept.md
 *         §4.1; Entscheide Patrik S156/S157/S161: ZF1 a, ZF5 a, ZF6 a, SP2 a, SG1 a–SG3 a, UG1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * `LUMIVIZ_75_NULLPROBE=<Ordner>` wertet das heutige Modell einmal aus — die reine Vorgabe, Startpunkt 1 und
 * Startpunkt 2 der Suche — und legt `nullprobe.txt` in den Ordner; ohne die Variable meldet der Fall das und prueft
 * nichts. Der Fall „Salamander gegen sich selbst“ misst *Salamander* an Taste 60 und braucht `LUMIVIZ_SALAMANDER`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "ModellMessung.hpp"
#include "Modellton.hpp"
#include "Strukturmessung.hpp"
#include "Suchkurven.hpp"
#include "Zielsumme.hpp"

#include "audio/WavDatei.hpp"
#include "loop/Fft.hpp"
#include "loop/HammerSaite.hpp"

#include <QDateTime>
#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::messung;

namespace
{

const std::filesystem::path kStrukturDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_struktur.txt";
const std::filesystem::path kZielDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_ziel.txt";

const char* compilerName()
{
#if defined(__clang__)
    return "clang";
#elif defined(_MSC_VER)
    return "msvc";
#else
    return "anderer";
#endif
}

/// Eine Groesse mit den Werten am Kanalmittel je Ebene.
GroesseJeEbene groesse(const std::string& name, Skala skala, const std::vector<double>& mittel)
{
    GroesseJeEbene g{name, skala, {}};
    for (const double m : mittel)
    {
        EbenenWerte w;
        w.mittel = m;
        g.ebenen.push_back(w);
    }
    return g;
}

ZielGroesse zielGroesse(Skala skala, double ziel, double toleranz)
{
    return ZielGroesse{skala, Wert{ziel, 11}, Toleranz{skala, toleranz, toleranz / 2.0, 10, 11}};
}

const Abweichung& abweichung(const TastenSumme& t, const std::string& name)
{
    for (const Abweichung& a : t.groessen)
        if (a.groesse == name) return a;
    FAIL("Groesse fehlt: " << name);
    return t.groessen.front();
}

}  // namespace

TEST_CASE("Loop Salamander 7.5.9 Ziel: Untergrenze und Verhaeltnis")
{
    // UG1 a: die Messgenauigkeit — 3 % bei Zeiten, 0,5 dB, 2 % des Ziels bei Cent
    CHECK(untergrenze(Skala::Faktor, 2.0) == doctest::Approx(std::log(1.03)));
    CHECK(untergrenze(Skala::Dezibel, -12.0) == doctest::Approx(0.5));
    CHECK(untergrenze(Skala::Cent, 5.0) == doctest::Approx(0.1));
    CHECK(untergrenze(Skala::Cent, -5.0) == doctest::Approx(0.1));

    TastenZiele ziel;
    ziel["vorwaerts_t60"] = zielGroesse(Skala::Faktor, 2.0, 0.2);   // Toleranz ueber der Untergrenze
    ziel["knick_tiefe_db"] = zielGroesse(Skala::Dezibel, -3.0, 0.1);  // darunter: auf 0,5 dB angehoben
    ziel["schwebung_db_1"] = zielGroesse(Skala::Dezibel, 4.0, 1.0);   // das Modell gibt sie nicht her
    const std::vector<GroesseJeEbene> modell{groesse("vorwaerts_t60", Skala::Faktor, {2.3, 2.4, 2.5}),
                                             groesse("knick_tiefe_db", Skala::Dezibel, {-2.0, -4.0, -1.0}),
                                             groesse("t60_spaet", Skala::Faktor, {9.0})};  // ohne Ziel: zaehlt nicht
    const TastenSumme t = vergleiche(60, ziel, modell);
    CHECK(t.midi == 60);
    REQUIRE(t.groessen.size() == 3);

    const Abweichung& v = abweichung(t, "vorwaerts_t60");
    CHECK(v.modell == doctest::Approx(2.4));  // der Median der Ebenen
    CHECK(v.toleranz == doctest::Approx(0.2));
    CHECK(v.verhaeltnis == doctest::Approx(std::log(1.2) / 0.2));
    CHECK(v.getroffen);
    CHECK_FALSE(v.fehlt);

    const Abweichung& k = abweichung(t, "knick_tiefe_db");
    CHECK(k.modell == doctest::Approx(-2.0));
    CHECK(k.toleranz == doctest::Approx(0.5));
    CHECK(k.verhaeltnis == doctest::Approx(2.0));
    CHECK_FALSE(k.getroffen);

    const Abweichung& s = abweichung(t, "schwebung_db_1");
    CHECK(s.fehlt);
    CHECK_FALSE(s.getroffen);
    CHECK(s.verhaeltnis == doctest::Approx(kFehltVerhaeltnis));

    CHECK(t.summe == doctest::Approx(std::pow(std::log(1.2) / 0.2, 2.0) + 4.0 + 100.0));
    CHECK(t.getroffen == 1);
    CHECK(t.fehlend == 1);
    CHECK_FALSE(t.fertig);

    // eine Taste, an der alles trifft, ist fertig; ein Ziel von null in Cent hat die Untergrenze null — das Modell fehlt dann
    TastenZiele nurEins;
    nurEins["vorwaerts_t60"] = zielGroesse(Skala::Faktor, 2.0, 0.2);
    CHECK(vergleiche(63, nurEins, modell).fertig);
    TastenZiele null;
    null["schwebung_cent_1"] = ZielGroesse{Skala::Cent, Wert{0.0, 11}, Toleranz{Skala::Cent, 0.0, 0.0, 10, 11}};
    CHECK(vergleiche(63, null, {groesse("schwebung_cent_1", Skala::Cent, {0.0})}).groessen.front().fehlt);

    // gesamt: Ausreisser ueber dem Dreifachen des Mittels der Tastensummen (SP2 a)
    std::vector<TastenSumme> tasten(4);
    for (int i = 0; i < 4; ++i)
    {
        tasten[static_cast<std::size_t>(i)].midi = 21 + 3 * i;
        tasten[static_cast<std::size_t>(i)].summe = i == 3 ? 20.0 : 1.0;
        tasten[static_cast<std::size_t>(i)].groessen.resize(2);
        tasten[static_cast<std::size_t>(i)].getroffen = 1;
    }
    tasten[0].fertig = true;
    const Gesamt g = gesamt(tasten);
    CHECK(g.summe == doctest::Approx(23.0));
    CHECK(g.groessen == 8);
    CHECK(g.getroffen == 4);
    CHECK(g.fertig == 1);
    CHECK(g.tasten == 4);
    REQUIRE(g.ausreisser.size() == 1);
    CHECK(g.ausreisser.front() == 30);
    const std::string bericht = berichte({t});
    CHECK(bericht.find("schwebung_db_1 10.000 (fehlt)") != std::string::npos);
    CHECK(bericht.find("Gesamt: Summe r^2") != std::string::npos);
}

TEST_CASE("Loop Salamander 7.5.9 Ziel: Salamander gegen sich selbst (LUMIVIZ_SALAMANDER)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_SALAMANDER=<Ordner> misst Salamander an Taste 60 gegen das eigene Ziel");
        return;
    }
    const std::optional<Strukturen> strukturen = liesStruktur(kStrukturDatei);
    const std::optional<Ziele> ziele = liesZiele(kZielDatei);
    REQUIRE(strukturen.has_value());
    REQUIRE(ziele.has_value());
    const std::filesystem::path ordner = variable.toStdU16String();
    const SalamanderTaste taste{60, "C4"};
    std::vector<std::array<KanalMessung, 2>> messungen;
    std::vector<std::array<KanalMessung, 2>> einzeln;
    std::vector<TonHuellen> huellen;
    for (int e = 1; e <= kEbenen; ++e)
    {
        const std::optional<lumi::audio::WavInhalt> wav = lumi::audio::liesWavDatei(salamanderDatei(ordner, taste, e));
        REQUIRE_MESSAGE(wav.has_value(), salamanderDatei(ordner, taste, e).string());
        std::array<KanalMessung, 2> jeKanal;
        TonHuellen h;
        messungen.push_back(messeTon(wav->stereo, wav->rate, tastenFrequenz(taste.midi), lumi::loop::fft(), false, nullptr, &jeKanal,
                                     e >= kKurvenEbeneAb ? &h : nullptr));
        einzeln.push_back(jeKanal);
        if (e >= kKurvenEbeneAb) huellen.push_back(std::move(h));
    }
    std::vector<GroesseJeEbene> groessen = strukturGroessen(huellen, strukturen->at(taste.midi));
    for (GroesseJeEbene& g : huellenGroessen(messungen, einzeln)) groessen.push_back(std::move(g));
    const TastenZiele& ziel = ziele->at(taste.midi);

    // alle Ebenen zusammen: der Median ist das Ziel selbst (die Datei haelt sechs Stellen)
    const TastenSumme ganz = vergleiche(taste.midi, ziel, groessen);
    CHECK(ganz.fehlend == 0);
    for (const Abweichung& a : ganz.groessen)
    {
        CAPTURE(a.groesse);
        CHECK(a.verhaeltnis < 1e-3);
    }

    // je Ebene fuer sich (Lesart Claude S162): die Toleranz ist die eigene Streuung — der Median der r liegt bei
    // hoechstens 1
    std::vector<double> r;
    for (std::size_t e = 0; e < huellen.size(); ++e)
    {
        std::vector<GroesseJeEbene> eine;
        for (const GroesseJeEbene& g : groessen) eine.push_back({g.name, g.skala, {g.ebenen[e]}});
        for (const Abweichung& a : vergleiche(taste.midi, ziel, eine).groessen)
            if (!a.fehlt) r.push_back(a.verhaeltnis);
    }
    REQUIRE(r.size() > 100);
    const double median = detail::median(r);
    MESSAGE("Salamander an 60 je Ebene gegen das eigene Ziel: " << r.size() << " Werte, Median r " << median);
    CHECK(median <= 1.0);
}

TEST_CASE("Loop Salamander 7.5.9 Ziel: Kurven der Suche")
{
    using lumi::loop::HammerSaite;
    using lumi::loop::KurvenSkala;
    const std::optional<Strukturen> strukturen = liesStruktur(kStrukturDatei);
    REQUIRE(strukturen.has_value());
    const HammerSaite& vorgabe = lumi::loop::hammerSaiteVorgabe();
    const std::vector<int> tasten = modelltest::salamanderMidi();
    REQUIRE(tasten.size() == 30);
    const modelltest::Suchraum raum = modelltest::suchraum(vorgabe, tasten, *strukturen);

    // neun Kurven; im Chor ab zwei Saiten liegen 26 Tasten (ab 33), bei einer Saite 4 (21–30)
    const std::vector<std::string> namen{"decay", "lateDecay", "highDecay", "detune", "u leise", "u laut", "gamma", "delta", "h"};
    REQUIRE(raum.kurven.size() == namen.size());
    for (std::size_t k = 0; k < namen.size(); ++k)
    {
        const modelltest::SuchKurve& kurve = raum.kurven[k];
        CAPTURE(kurve.name);
        CHECK(kurve.name == namen[k]);
        CHECK(kurve.tasten.size() + kurve.festTasten.size() == 30);
        const std::size_t frei = kurve.name == "delta" ? 4 : (kurve.name == "detune" || kurve.name.rfind("u ", 0) == 0 ? 26 : 30);
        CHECK(kurve.tasten.size() == frei);
        for (std::size_t i = 0; i < kurve.tasten.size(); ++i)
        {
            CHECK(kurve.unten[i] <= kurve.start[i]);
            CHECK(kurve.start[i] <= kurve.oben[i]);
            CHECK(kurve.unten[i] < kurve.oben[i]);
        }
    }
    CHECK(raum.zahl() == 5 * 30 + 3 * 26 + 4);
    CHECK(raum.ab(3) == 90);
    const auto finde = [&raum](const std::string& name) -> const modelltest::SuchKurve& {
        for (const modelltest::SuchKurve& k : raum.kurven)
            if (k.name == name) return k;
        return raum.kurven.front();
    };
    const auto index = [](const modelltest::SuchKurve& k, int taste) {
        return static_cast<std::size_t>(std::find(k.tasten.begin(), k.tasten.end(), taste) - k.tasten.begin());
    };

    // Grenzen und Start nach der Tabelle: decay Faktor 3 um die Vorgabe, gamma 0–45 mit Start 30, u Start 0,25 (die
    // Vorgabe ist 0), detune bis 6 Cent (ab 90 bis 7,5), h 0–1 mit Start 0
    const modelltest::SuchKurve& decay = finde("decay");
    const double d60 = lumi::loop::wertAn(vorgabe.abklingS, 60, KurvenSkala::Logarithmisch);
    CHECK(decay.start[index(decay, 60)] == doctest::Approx(d60));
    CHECK(decay.unten[index(decay, 60)] == doctest::Approx(d60 / 3.0));
    CHECK(decay.oben[index(decay, 60)] == doctest::Approx(3.0 * d60));
    CHECK(decay.log);
    const modelltest::SuchKurve& gamma = finde("gamma");
    CHECK(gamma.start[index(gamma, 21)] == doctest::Approx(30.0));
    CHECK(gamma.oben[index(gamma, 108)] == doctest::Approx(45.0));
    CHECK_FALSE(gamma.log);
    CHECK(finde("u leise").start[0] == doctest::Approx(0.25));
    CHECK(finde("u laut").start[0] == doctest::Approx(0.25));
    const modelltest::SuchKurve& detune = finde("detune");
    CHECK(detune.tasten.front() == 33);
    CHECK(detune.oben[index(detune, 87)] == doctest::Approx(6.0));
    CHECK(detune.oben[index(detune, 90)] == doctest::Approx(7.5));
    CHECK(finde("h").start[0] == doctest::Approx(0.0));
    CHECK(finde("delta").tasten.back() == 30);
    const modelltest::SuchKurve& hoch = finde("highDecay");
    const auto [hu, ho] = modelltest::bensaBand(60, lumi::loop::wertAn(vorgabe.saitenLaengeM, 60, KurvenSkala::Logarithmisch));
    CHECK(hoch.unten[index(hoch, 60)] == doctest::Approx(hu));
    CHECK(hoch.oben[index(hoch, 60)] == doctest::Approx(ho));

    // detune an 60 (SG3 a): der Median ueber die Teiltoene mit Schwebung von 1200·log₂(1 + f / (k·f_Taste)) — Median wie
    // ueberall in der Messung (`detail::median`): bei gerader Zahl der obere der beiden mittleren Werte
    {
        std::vector<double> cent;
        const TonStruktur& s = strukturen->at(60);
        for (std::size_t i = 0; i < s.size(); ++i)
            if (s[i].schwebt) cent.push_back(1200.0 * std::log2(1.0 + s[i].schwebungHz / (static_cast<double>(i + 1) * tastenFrequenz(60))));
        REQUIRE_FALSE(cent.empty());
        std::sort(cent.begin(), cent.end());
        const double median = cent[cent.size() / 2];
        CHECK(detune.start[index(detune, 60)] == doctest::Approx(std::clamp(median, 0.0, 6.0)));
        MESSAGE("detune an 60: Start " << detune.start[index(detune, 60)] << " Cent aus " << cent.size() << " Teiltoenen, Vorgabe "
                                       << lumi::loop::wertAn(vorgabe.verstimmungCent, 60, KurvenSkala::Linear));
    }

    // Startpunkt 2: log × 1,6, linear + 0,25 des Bereichs, in den Grenzen
    const std::vector<double> x1 = modelltest::startpunkt(raum, 1);
    const std::vector<double> x2 = modelltest::startpunkt(raum, 2);
    REQUIRE(x1.size() == raum.zahl());
    REQUIRE(x2.size() == raum.zahl());
    CHECK(x2 == modelltest::klemme(raum, x2));
    {
        const std::size_t j = raum.ab(0) + index(decay, 60);
        CHECK(x2[j] == doctest::Approx(std::log(1.6 * d60)));
        const std::size_t g = raum.ab(6) + index(gamma, 60);
        CHECK(x2[g] == doctest::Approx(30.0 + 0.25 * 45.0));
        const std::size_t u = raum.ab(4);
        CHECK(x2[u] == doctest::Approx(0.5));
    }

    // die Saite aus Startpunkt 1: an jeder gemessenen Taste der Start bzw. die Vorgabe; keine Abweichungen; die spaete
    // Stufe nie kuerzer als die fruehe
    const HammerSaite saite = modelltest::saiteAus(raum, x1);
    for (const modelltest::SuchKurve& k : raum.kurven)
    {
        CAPTURE(k.name);
        const lumi::loop::Tastenkurve& kurve = saite.*k.feld;
        CHECK(kurve.abweichungen.empty());
        CHECK(kurve.punkte.size() == 30);
        for (std::size_t i = 0; i < k.tasten.size(); ++i)
            if (k.feld != &HammerSaite::abklingSpaetS)
                CHECK(lumi::loop::wertAn(kurve, k.tasten[i], k.skala()) == doctest::Approx(k.start[i]).epsilon(1e-9));
        for (std::size_t i = 0; i < k.festTasten.size(); ++i)
            CHECK(lumi::loop::wertAn(kurve, k.festTasten[i], k.skala()) == doctest::Approx(k.festWerte[i]).epsilon(1e-9));
    }
    for (const int t : tasten)
        CHECK(lumi::loop::wertAn(saite.abklingSpaetS, t, KurvenSkala::Logarithmisch) >=
              lumi::loop::wertAn(saite.abklingS, t, KurvenSkala::Logarithmisch) * (1.0 - 1e-12));
    CHECK(lumi::loop::wertAn(saite.polarisationCent, 33, KurvenSkala::Linear) == doctest::Approx(0.0).epsilon(1e-12));

    // ein Stuetzpunkt wirkt an den gemessenen Tasten nur auf seine Taste (die Grundlage der Jacobi-Matrix je Kurve, 7.5.10)
    std::vector<double> x = x1;
    x[raum.ab(0) + index(decay, 60)] += 0.1;
    const HammerSaite anders = modelltest::saiteAus(raum, x);
    for (const int t : tasten)
    {
        CAPTURE(t);
        const double a = lumi::loop::wertAn(anders.abklingS, t, KurvenSkala::Logarithmisch);
        const double b = lumi::loop::wertAn(saite.abklingS, t, KurvenSkala::Logarithmisch);
        if (t == 60) CHECK(a == doctest::Approx(b * std::exp(0.1)));
        else CHECK(a == doctest::Approx(b).epsilon(1e-12));
    }
}

TEST_CASE("Loop Salamander 7.5.9 Ziel: Modellton an Taste 60")
{
    const std::optional<Strukturen> strukturen = liesStruktur(kStrukturDatei);
    const std::optional<Ziele> ziele = liesZiele(kZielDatei);
    REQUIRE(strukturen.has_value());
    REQUIRE(ziele.has_value());
    const auto start = std::chrono::steady_clock::now();
    const std::vector<GroesseJeEbene> groessen = modelltest::modellAn(lumi::loop::hammerSaiteVorgabe(), 60, strukturen->at(60));
    const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

    // 18 Groessen, jede mit drei Ebenen; jede Groesse des Ziels ist da oder als fehlend benannt
    REQUIRE(groessen.size() == 18);
    for (const GroesseJeEbene& g : groessen) CHECK(g.ebenen.size() == modelltest::kSuchEbenen.size());
    const TastenSumme t = vergleiche(60, ziele->at(60), groessen);
    CHECK(t.groessen.size() == ziele->at(60).size());
    std::ostringstream fehlend;
    for (const Abweichung& a : t.groessen)
    {
        CHECK(std::isfinite(a.verhaeltnis));
        if (a.fehlt) fehlend << ' ' << a.groesse;
    }
    MESSAGE("Modellton an 60 (Vorgabe, drei Ebenen, " << modelltest::dauerMsAus(strukturen->at(60)) << " ms): " << dauer << " s, Summe r^2 "
                                                      << t.summe << ", getroffen " << t.getroffen << " / " << t.groessen.size()
                                                      << ", fehlend:" << (t.fehlend == 0 ? std::string(" keine") : fehlend.str()));
}

TEST_CASE("Loop Salamander 7.5.9 Nullprobe (LUMIVIZ_75_NULLPROBE)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_NULLPROBE");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_NULLPROBE=<Ordner> wertet die Vorgabe und beide Startpunkte der Suche an allen 30 Tasten aus");
        return;
    }
    const std::filesystem::path ordner = variable.toStdU16String();
    std::filesystem::create_directories(ordner);
    const std::optional<Strukturen> strukturen = liesStruktur(kStrukturDatei);
    const std::optional<Ziele> ziele = liesZiele(kZielDatei);
    REQUIRE(strukturen.has_value());
    REQUIRE(ziele.has_value());

    const lumi::loop::HammerSaite& vorgabe = lumi::loop::hammerSaiteVorgabe();
    const std::vector<int> tasten = modelltest::salamanderMidi();
    const modelltest::Suchraum raum = modelltest::suchraum(vorgabe, tasten, *strukturen);
    const std::vector<lumi::loop::HammerSaite> saiten{vorgabe, modelltest::saiteAus(raum, modelltest::startpunkt(raum, 1)),
                                                      modelltest::saiteAus(raum, modelltest::startpunkt(raum, 2))};
    const char* const titel[] = {"Vorgabe (baueVorgabe, ohne Aenderung)", "Startpunkt 1 (gamma 30, detune an der Schwebung, u 0,25)",
                                 "Startpunkt 2 (log x 1,6, linear + 0,25 des Bereichs)"};
    double sekunden = 0.0;
    const std::vector<modelltest::Auswertung> auswertungen = modelltest::werteAus(saiten, tasten, *strukturen, *ziele, &sekunden);
    REQUIRE(auswertungen.size() == saiten.size());

    std::ostringstream b;
    b << std::fixed << std::setprecision(3);
    b << "# Nullprobe 7.5.9 (SM4 b, ZF5 a, UG1 a)\n"
      << "# Datum " << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << ", Compiler " << compilerName() << ", "
      << modelltest::poolThreads() << " Threads, " << saiten.size() << " Auswertungen x 30 Tasten x 3 Ebenen in " << sekunden
      << " s (" << sekunden / static_cast<double>(saiten.size()) << " s je Auswertung)\n"
      << "# Ziel: werte/salamander_ziel.txt, Struktur: werte/salamander_struktur.txt; Ebenen v6, v10, v16 (SG1 a); r = d / max(tau, "
         "tau_min), tau_min = ln 1,03 | 0,5 dB | 2 % (UG1 a); fehlender Modellwert r = 10 (Lesart S161)\n"
      << "# Suchraum: " << raum.zahl() << " Parameter in " << raum.kurven.size() << " Kurven\n";
    for (std::size_t s = 0; s < saiten.size(); ++s)
    {
        const Gesamt g = gesamt(auswertungen[s].tasten);
        b << "\n## " << titel[s] << "\n" << berichte(auswertungen[s].tasten, true);
        MESSAGE(std::string(titel[s]) << ": Summe r^2 " << g.summe << ", getroffen " << g.getroffen << " / " << g.groessen << ", fehlend " << g.fehlend
                         << ", fertige Tasten " << g.fertig);
    }
    // je Groesse ueber die Tasten: wie viele getroffen, Median r — welche Groessen das Modell heute trifft
    b << "\n## Je Groesse ueber die Tasten (getroffen / mit Ziel, fehlend, Median r)\n";
    for (std::size_t s = 0; s < saiten.size(); ++s)
    {
        b << "### " << titel[s] << "\n";
        std::map<std::string, std::vector<const Abweichung*>> je;
        for (const TastenSumme& t : auswertungen[s].tasten)
            for (const Abweichung& a : t.groessen) je[a.groesse].push_back(&a);
        for (const auto& [name, liste] : je)
        {
            int getroffen = 0, fehlt = 0;
            std::vector<double> r;
            for (const Abweichung* a : liste)
            {
                getroffen += a->getroffen ? 1 : 0;
                fehlt += a->fehlt ? 1 : 0;
                r.push_back(a->verhaeltnis);
            }
            b << name << ": " << getroffen << " / " << liste.size() << ", fehlend " << fehlt << ", Median r " << detail::median(r) << '\n';
        }
    }
    std::ofstream aus(ordner / "nullprobe.txt", std::ios::binary);
    aus << b.str();
    REQUIRE(static_cast<bool>(aus));
    MESSAGE("Nullprobe nach " << (ordner / "nullprobe.txt").string() << ", " << sekunden << " s");
}
