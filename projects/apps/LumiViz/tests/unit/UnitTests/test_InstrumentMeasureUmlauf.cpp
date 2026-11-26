/**
 ****************************************************************************************
 * @file   test_InstrumentMeasureUmlauf.cpp
 * @brief  Stufe 34k-7c: der Umlauf-Filter der gezupften Saite — die Glieder (`InstrumentModels/gemeinsam/Umlauf`) und der
 *         Entwurf (`InstrumentMeasure/UmlaufEntwurf`); docs/audio/Instrumente_IC_Konzept.md UL1 a–UL3-1 a, ZK1 a
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton.
 *
 * - Die Glieder im Takt (Verlust, Kette, Thiran) gegen ihre Antwort; der Verzug der Kette und des Thiran-Glieds; das
 *   Stimmen des Glieds.
 * - Das Ziel: der Grundton der Saite aus dem Umlauf gegen die Proben.
 * - Die Tafel je Taste an Stichtasten von Harfe und Cembalo (Proben `2026-11-18_S205_tafel_je_taste_*.txt`): Grundton,
 *   Zahl der Teiltoene, Abel-Smith-Zahl wie die Probe, UL2 a haelt (3 Cent, Abklingen in der Schwelle).
 * - Die gebundene Tafel je Saite an Laute Chor 1.1 und Oud Chor 5.1 (`…_gebunden_stuetzstellen_*_rand_verlust_halb.txt`):
 *   UL2 a an Buenden, halben, Achteln und an den fremden Stellen.
 * - Messfall `LUMIVIZ_34K7C_MESSUNG=<datei.md>` (`LUMIVIZ_34K7C_NUR=Harfe,Cembalo` waehlt Instrumente): jede Taste von Harfe und Cembalo, jede Saite von Gitarre, Laute und Oud
 *   mit Ergebnis und Rechenzeit (Kipppunkt ZK1 a). RZ2 a (S221): `LUMIVIZ_34K7C_TASTEN=<T32,Saite 5,…>` waehlt Tasten bzw.
 *   Saiten, `LUMIVIZ_34K7C_RANGVERLUST=norm|scipy|beide` die Fassung (mit TK2 a, wie scipy, beide); die Datei traegt dazu
 *   je Entwurf den Rechenaufwand.
 * - Rechenaufwand und Schalter Rangverlust (RZ2 a): die Zaehler gehen auf; ohne Rangverlust rechnen beide Fassungen gleich.
 *
 * Die Saiten sind die des Pruefstands S199 (`proben/2026-11-15_S199_pruefstand_zupf.py`: `GITARRE`, `harfe_taste`,
 * `cembalo_taste`, `choere`); seit 34k-7d-3 kommen sie aus den Vorgaben des Kerns (`InstrumentModels/zupf/Saiten`).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentMeasure/UmlaufEntwurf.hpp"
#include "InstrumentModels/gemeinsam/Umlauf.hpp"
#include "InstrumentModels/zupf/Saiten.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <numbers>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
namespace ku = lumi::kern::umlauf;
namespace kz = lumi::kern::zupf;
namespace mu = lumi::messung::umlauf;
using ku::Saitenwerte;
constexpr double kPi = std::numbers::pi;

// ---------------------------------------------------------------- die Saiten (seit 34k-7d-3 aus den Vorgaben des Kerns)
// `InstrumentModels/zupf/Saiten` — dieselben Zahlen wie der Pruefstand S199; hier nur die Huellen, die die Faelle lesen
const kz::Saitensatz& gitarrenSatz()
{
    static const kz::Saitensatz s = kz::saitensatzGitarre();
    return s;
}
const double kGitarreMensur = gitarrenSatz().mensur;
const double kLauteMensur = kz::saitensatzLaute().mensur;
const double kOudMensur = kz::saitensatzOud().mensur;

Saitenwerte gitarre(int saite)  // 6 … 1
{
    return gitarrenSatz().choere[static_cast<std::size_t>(6 - saite)].saiten.front().werte;
}

std::optional<std::pair<Saitenwerte, double>> harfe(int taste)
{
    const std::optional<kz::Tastensaite> t = kz::harfenSaite(taste);
    if (!t) return std::nullopt;
    return std::make_pair(t->werte, t->laenge);
}

std::pair<Saitenwerte, double> cembalo(int taste)
{
    const kz::Tastensaite t = kz::cembaloSaite(taste);
    return {t.werte, t.laenge};
}

struct BenannteSaite
{
    std::string name;
    Saitenwerte saite;
    double mensur;
    int buende;
};

std::vector<BenannteSaite> saitenVon(const std::string& inst)
{
    std::vector<BenannteSaite> out;
    if (inst == "Gitarre")
        for (int s = 6; s >= 1; --s) out.push_back({"Saite " + std::to_string(s), gitarre(s), kGitarreMensur, gitarrenSatz().buende});
    if (inst == "Laute" || inst == "Oud")
    {
        const kz::Saitensatz satz = inst == "Laute" ? kz::saitensatzLaute() : kz::saitensatzOud();
        for (std::size_t c = 0; c < satz.choere.size(); ++c)
            for (std::size_t j = 0; j < satz.choere[c].saiten.size(); ++j)
            {
                const kz::Chorsaite& z = satz.choere[c].saiten[j];
                // im Einklang: einmal (die Verstimmung ist eine Lage auf derselben Tafel)
                if (j > 0 && z.oktave == satz.choere[c].saiten[0].oktave && z.werte == satz.choere[c].saiten[0].werte) continue;
                out.push_back({"Chor " + std::to_string(c + 1) + "." + std::to_string(j + 1), z.werte, satz.mensur, satz.buende});
            }
    }
    return out;
}

// die DFT der Impulsantwort bei ω
std::complex<double> dft(const std::vector<double>& h, double w)
{
    std::complex<double> s = 0.0;
    for (std::size_t n = 0; n < h.size(); ++n) s += h[n] * std::polar(1.0, -w * static_cast<double>(n));
    return s;
}

}  // namespace

TEST_CASE("InstrumentModels 34k-7c Glieder des Umlaufs im Takt gegen ihre Antwort")
{
    ku::Umlauffilter f;
    f.gliedOrdnung = 5;
    f.gliedVerzug = 5.3;
    f.kette = {{0.8, 0.4}, {0.5, 1.9}, {0.95, 0.05}};
    f.b = {0.6, -0.2, 0.05};  // ein Verlust mit |H| < 1
    f.a = {1.0, -0.5, 0.1};
    ku::Umlauflauf lauf(f);
    std::vector<double> h(6000);
    for (std::size_t n = 0; n < h.size(); ++n) h[n] = lauf.schritt(n == 0 ? 1.0 : 0.0);
    for (double w : {0.01, 0.3, 1.2, 2.5, 3.1})
    {
        const std::complex<double> a = f.antwort(w), b = dft(h, w);
        CHECK(std::abs(a - b) < 1e-9);
    }
    // Allpaesse: Betrag 1; der Verzug der Kette ist −arg der Antwort (abgewickelt)
    ku::Umlauffilter nurKette;
    nurKette.kette = f.kette;
    nurKette.gliedOrdnung = 1;
    nurKette.gliedVerzug = 1.3;  // (D = 1 gaebe 0/0 im Beiwert)
    double vor = 0.0, off = 0.0;
    for (int i = 1; i <= 3000; ++i)
    {
        const double w = kPi * i / 3001.0;
        const std::complex<double> hk = nurKette.antwort(w);
        CHECK(std::abs(std::abs(hk) - 1.0) < 1e-12);
        const double a = std::arg(hk);
        if (i > 1 && a - vor > kPi) off -= 2 * kPi;
        if (i > 1 && a - vor < -kPi) off += 2 * kPi;
        vor = a;
        CHECK(ku::verzugKette(f.kette, w) + ku::verzugThiran(1.3, 1, w) == doctest::Approx(-(a + off)).epsilon(1e-9));
    }
    // Thiran: bei kleinem ω der Verzug D; maximal flach
    for (double D : {4.5, 5.0, 5.49})
    {
        CHECK(ku::verzugThiran(D, 5, 1e-4) / 1e-4 == doctest::Approx(D).epsilon(1e-6));
        // stetig bis π: gegen die fein abgewickelte Phase der Antwort (S220: einmal abgewickelt lag sie ueber 4,8 kHz um 2π daneben)
        const std::vector<double> a = ku::thiranNenner(D, 5);
        const std::vector<double> b(a.rbegin(), a.rend());
        double v0 = 0.0, o = 0.0;
        for (int i = 1; i <= 2000; ++i)
        {
            const double w = 3.1 * i / 2000.0;
            const double p = std::arg(ku::antwort(b, a, w));
            if (i > 1 && p - v0 > kPi) o -= 2 * kPi;
            if (i > 1 && p - v0 < -kPi) o += 2 * kPi;
            v0 = p;
            CHECK(ku::verzugThiran(D, 5, w) == doctest::Approx(-(p + o)).epsilon(1e-9));
        }
    }
    // Stimmen: N und D treffen den Sollverzug am Grundton
    ku::Umlauffilter g = f;
    const double w1 = 2 * kPi * 220.0 / 48000.0;
    REQUIRE(ku::stimmeGlied(g, w1, 77.3 * w1));
    CHECK(g.gliedVerzug >= 4.5 - 1e-9);
    CHECK(g.gliedVerzug < 5.5 + 1e-9);
    CHECK(g.verzugGanz * w1 + ku::verzugThiran(g.gliedVerzug, 5, w1) == doctest::Approx(77.3 * w1).epsilon(1e-10));
    CHECK_FALSE(ku::stimmeGlied(g, w1, 5.0 * w1));  // kein Platz
}

TEST_CASE("InstrumentModels 34k-7c Ziel: der Grundton der Saite aus dem Umlauf")
{
    // der erste Teilton liegt, wo der Verzug 2π ist; die Proben S205 nennen ihn
    auto grund = [](const Saitenwerte& s, double L) {
        double lo = 1.0, hi = 20000.0;
        for (int i = 0; i < 200; ++i)
        {
            const double m = 0.5 * (lo + hi);
            (ku::umlaufDerSaite(s, L, m).verzug < 2 * kPi ? lo : hi) = m;
        }
        return lo;
    };
    const auto h60 = harfe(60);
    REQUIRE(h60);
    CHECK(grund(h60->first, h60->second) == doctest::Approx(261.72).epsilon(5e-5));
    const auto c29 = cembalo(29);
    CHECK(grund(c29.first, c29.second) == doctest::Approx(43.65).epsilon(5e-4));
    CHECK(grund(gitarre(6), kGitarreMensur) == doctest::Approx(82.4).epsilon(1e-3));
    CHECK(grund(gitarre(1), kGitarreMensur) == doctest::Approx(330.9).epsilon(1e-3));
    // der Betrag des Umlaufs liegt unter 1 und faellt
    const ku::Umlaufwert a = ku::umlaufDerSaite(gitarre(6), kGitarreMensur, 100.0), b = ku::umlaufDerSaite(gitarre(6), kGitarreMensur, 5000.0);
    CHECK(a.betrag < 1.0);
    CHECK(b.betrag < a.betrag);
}

TEST_CASE("InstrumentMeasure 34k-7c Tafel je Taste an Stichtasten von Harfe und Cembalo (UL2 a)")
{
    struct Stich
    {
        const char* inst;
        int taste;
        double f1;
        int zaehlend, abelSmith;
    };
    // tafel_je_taste_harfe.txt / _cembalo.txt: f1, zaehlend (mit Zugabe), Abel-Smith
    const Stich stiche[] = {{"Harfe", 27, 39.29, 57, 18},   {"Harfe", 46, 116.75, 31, 5}, {"Harfe", 60, 261.72, 25, 3},
                            {"Harfe", 96, 2095.05, 8, 0},   {"Harfe", 103, 3139.67, 5, 0}, {"Cembalo", 60, 261.63, 73, 4},
                            {"Cembalo", 84, 1046.56, 18, 0}};
    for (const Stich& st : stiche)
    {
        const std::string instrument = st.inst;  // CAPTURE haelt eine Referenz
        CAPTURE(instrument);
        CAPTURE(st.taste);
        Saitenwerte s;
        double L = 0.0;
        if (std::string(st.inst) == "Harfe")
        {
            const auto h = harfe(st.taste);
            REQUIRE(h);
            s = h->first;
            L = h->second;
        }
        else
            std::tie(s, L) = cembalo(st.taste);
        const auto t0 = std::chrono::steady_clock::now();
        const std::optional<mu::TafelTaste> t = mu::entwirfJeTaste(s, L);
        const double sek = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        REQUIRE(t);
        MESSAGE(std::string(st.inst) << " T" << st.taste << ": Thiran " << t->filter.gliedOrdnung << ", " << t->filter.kette.size() << " Biquads, "
                        << t->bewertung.centGroesster << " Cent, Abklingen " << t->bewertung.abklingenGroesster << " %, " << sek << " s");
        CHECK(t->grundHz == doctest::Approx(st.f1).epsilon(1e-4));
        CHECK(t->zaehlendMitZugabe == st.zaehlend);
        CHECK(t->abelSmith == st.abelSmith);
        CHECK(t->bewertung.haelt());
        CHECK(std::abs(t->bewertung.grundtonCent) <= 0.1);  // UL2 a: Grundton ±0,1 Cent
        // das Filter traegt, was die Bewertung sagt: noch einmal gemessen
        mu::Umlauffilter f = t->filter;
        const mu::Bewertung b = mu::bewerte(s, L, f);
        CHECK(b.centGroesster == doctest::Approx(t->bewertung.centGroesster).epsilon(1e-9));
        CHECK(f.verzugGanz == t->filter.verzugGanz);
        // 34k-7d-3: der Kern stimmt das Glied zur Laufzeit selbst (die Tafel traegt N und D nicht) — Bit fuer Bit wie der
        // Entwurf
        ku::Umlauffilter g = t->filter;
        g.verzugGanz = 0;
        g.gliedVerzug = 4.5;
        REQUIRE(ku::stimmeAmGrundton(g, s, L));
        CHECK(g == t->filter);
        CHECK(ku::grundtonDerSaite(s, L) == t->grundHz);
    }
}

namespace
{
// die Zaehler gehen auf: jede Auswertung ist ein Start, ein angenommener oder ein verworfener Schritt; jede Jacobi-Matrix ein
// Start oder ein angenommener Schritt
void pruefeAufwand(const mu::Rechenaufwand& a)
{
    CHECK(a.anpassungen > 0);
    CHECK(a.schritte > 0);
    CHECK(a.auswertungen == a.anpassungen + a.schritte + a.verworfen);
    CHECK(a.ableitungen == a.anpassungen + a.schritte);
    CHECK(a.zerlegungen >= a.schritte);
    CHECK(a.rangverluste <= a.zerlegungen);
    CHECK(a.sekundenResiduen + a.sekundenAbleitungen + a.sekundenZerlegung <= a.sekundenAnpassung);
    CHECK(a.sekundenAnpassung <= a.sekundenGesamt);
}
}  // namespace

TEST_CASE("InstrumentMeasure 34k-7c Rechenaufwand und Schalter Rangverlust (RZ2 a)")
{
    // Harfe T46: kurz, ohne Rangverlust — dort rechnen beide Fassungen denselben Weg
    const auto h = harfe(46);
    REQUIRE(h);
    const auto vorgabe = mu::entwirfJeTaste(h->first, h->second);
    mu::EntwurfOptionen scipy;
    scipy.rangverlust = mu::Rangverlust::WieScipy;
    const auto ohneTk2 = mu::entwirfJeTaste(h->first, h->second, scipy);
    REQUIRE(vorgabe);
    REQUIRE(ohneTk2);
    pruefeAufwand(vorgabe->aufwand);
    pruefeAufwand(ohneTk2->aufwand);
    MESSAGE("Harfe T46: " << vorgabe->aufwand.anpassungen << " Laeufe, " << vorgabe->aufwand.schritte << " Schritte, "
                          << vorgabe->aufwand.verworfen << " verworfen, " << vorgabe->aufwand.rangverluste << " Rangverluste");
    CHECK(vorgabe->aufwand.rangverluste == 0);
    CHECK(ohneTk2->aufwand.schritte == vorgabe->aufwand.schritte);
    CHECK(ohneTk2->aufwand.auswertungen == vorgabe->aufwand.auswertungen);
    CHECK(ohneTk2->bewertung.centGroesster == vorgabe->bewertung.centGroesster);
}

TEST_CASE("InstrumentMeasure 34k-7c Tafel je Saite gebunden: Laute Chor 1.1, Oud Chor 5.1 (UL3-1 a)")
{
    // die Gitarre (Saite 6: rund 5 min, Saite 1: rund 2 min) rechnet der Messfall; dort trifft sie die Probe Zahl fuer Zahl
    struct Stich
    {
        BenannteSaite saite;
        int M;
        double fremdCent, fremdAbklingen;  // gebunden_stuetzstellen_*_rand_verlust_halb.txt
    };
    const std::vector<BenannteSaite> laute = saitenVon("Laute"), oud = saitenVon("Oud");
    const Stich stiche[] = {{{"Laute " + laute[0].name, laute[0].saite, kLauteMensur, 12}, 8, 0.37, 12.09},
                            {{"Oud " + oud[4].name, oud[4].saite, kOudMensur, 12}, 8, 0.09, 3.99}};
    for (const Stich& st : stiche)
    {
        CAPTURE(st.saite.name);
        const auto t0 = std::chrono::steady_clock::now();
        const std::optional<mu::TafelSaite> t = mu::entwirfJeSaite(st.saite.saite, st.saite.mensur, st.saite.buende);
        const double sek = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        REQUIRE(t);
        MESSAGE(st.saite.name << ": " << t->radiusReihe.size() << " Biquads, Start " << t->startFaktor << ", Daempfung " << t->daempfung
                              << " | Buende " << t->buendeBewertung.centGroesster << ", halb " << t->halbBewertung.centGroesster << ", Achtel "
                              << t->achtelBewertung.centGroesster << ", fremd " << t->fremdBewertung.centGroesster << " Cent, Abklingen fremd "
                              << t->fremdBewertung.abklingenGroesster << " % | " << sek << " s");
        pruefeAufwand(t->aufwand);
        CHECK(static_cast<int>(t->radiusReihe.size()) == st.M);
        CHECK(t->verlust.size() == static_cast<std::size_t>(2 * st.saite.buende + 1));
        for (const mu::Bewertung* b : {&t->buendeBewertung, &t->halbBewertung, &t->achtelBewertung, &t->fremdBewertung})
            CHECK(b->haelt());
        // wie die Probe (trf wie scipy, aber eigene SVD und der Verlust mit Austausch: daher eine Zugabe)
        CHECK(t->fremdBewertung.centGroesster <= st.fremdCent + 0.2);
        CHECK(t->fremdBewertung.abklingenGroesster <= st.fremdAbklingen + 2.0);
        // das Filter an einer Lage, die die Anpassung nicht kennt
        mu::Bewertung b;
        const std::optional<mu::Umlauffilter> f = mu::filterAn(*t, st.saite.saite, 4.3, &b);
        REQUIRE(f);
        CHECK(b.haelt());
        CHECK(f->gliedOrdnung == 5);
        // 34k-7d-3: der Kern liest die Tafel ohne den Entwurf (`kern::umlauf::filterAn`) — Bit fuer Bit dasselbe Filter
        for (const double lage : {0.0, 4.3, 7.0, 11.75})
        {
            CAPTURE(lage);
            const std::optional<mu::Umlauffilter> e = mu::filterAn(*t, st.saite.saite, lage);
            const std::optional<ku::Umlauffilter> k = ku::filterAn(*t, st.saite.saite, lage);
            REQUIRE(e);
            REQUIRE(k);
            CHECK(*k == *e);
        }
    }
}

TEST_CASE("InstrumentMeasure 34k-7c Messfall Umlauf-Filter (LUMIVIZ_34K7C_MESSUNG=<datei.md>)")
{
    const char* ziel = std::getenv("LUMIVIZ_34K7C_MESSUNG");
    if (ziel == nullptr || std::string(ziel).empty())
    {
        MESSAGE("LUMIVIZ_34K7C_MESSUNG=<datei.md> entwirft jede Taste von Harfe und Cembalo und jede Saite von Gitarre, Laute, Oud "
                "und misst die Rechenzeit (ZK1 a)");
        return;
    }
    // LUMIVIZ_34K7C_NUR=<Instrument,…> misst nur diese (etwa "Harfe,Cembalo"); ohne die Variable alle fuenf.
    const char* nurText = std::getenv("LUMIVIZ_34K7C_NUR");
    const std::string nur = nurText == nullptr ? std::string() : "," + std::string(nurText) + ",";
    const auto gewaehlt = [&nur](const char* inst) { return nur.empty() || nur.find("," + std::string(inst) + ",") != std::string::npos; };
    // LUMIVIZ_34K7C_TASTEN=<T32,Saite 5,…> nur diese Tasten bzw. Saiten (Namen wie in den Tabellen, Tasten mit "T").
    const char* tastenText = std::getenv("LUMIVIZ_34K7C_TASTEN");
    const std::string tasten = tastenText == nullptr ? std::string() : "," + std::string(tastenText) + ",";
    const auto tasteGewaehlt = [&tasten](const std::string& name) { return tasten.empty() || tasten.find("," + name + ",") != std::string::npos; };
    // LUMIVIZ_34K7C_RANGVERLUST=norm|scipy|beide (RZ2 a): mit TK2 a (Vorgabe), wie scipy, oder beide je Taste nacheinander.
    const char* rvText = std::getenv("LUMIVIZ_34K7C_RANGVERLUST");
    const std::string rv = rvText == nullptr || std::string(rvText).empty() ? "norm" : rvText;
    REQUIRE((rv == "norm" || rv == "scipy" || rv == "beide"));
    std::vector<std::pair<std::string, mu::EntwurfOptionen>> fassungen;
    if (rv != "scipy") fassungen.emplace_back("TK2 a", mu::EntwurfOptionen{});
    if (rv != "norm")
    {
        mu::EntwurfOptionen e;
        e.rangverlust = mu::Rangverlust::WieScipy;
        fassungen.emplace_back("scipy", e);
    }
    const auto zeilenName = [&fassungen](const std::string& name, const std::string& fassung) {
        return fassungen.size() > 1 || fassung != "TK2 a" ? name + " (" + fassung + ")" : name;
    };
    std::ostringstream aufwand;
    aufwand << "\n## Rechenaufwand (RZ2 a)\n\n| Instrument | Taste / Saite | Fassung | Laeufe | Schritte | verworfen | Auswertungen | "
               "Ableitungen | SVD | Rangverluste | s Residuen | s Ableitungen | s SVD | s Anpassung | s Rest | s gesamt |\n"
               "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|\n";
    const auto aufwandZeile = [&aufwand](const char* inst, const std::string& name, const std::string& fassung, const mu::Rechenaufwand& a) {
        char zz[512];
        std::snprintf(zz, sizeof zz, "| %s | %s | %s | %d | %d | %d | %d | %d | %d | %d | %.1f | %.1f | %.1f | %.1f | %.1f | %.1f |\n", inst,
                      name.c_str(), fassung.c_str(), a.anpassungen, a.schritte, a.verworfen, a.auswertungen, a.ableitungen, a.zerlegungen,
                      a.rangverluste, a.sekundenResiduen, a.sekundenAbleitungen, a.sekundenZerlegung, a.sekundenAnpassung,
                      a.sekundenGesamt - a.sekundenAnpassung, a.sekundenGesamt);
        aufwand << zz;
    };
    std::ostringstream o;
    o << "# 34k-7c Entwurf des Umlauf-Filters in C++\n\n"
         "> test_InstrumentMeasureUmlauf, `LUMIVIZ_34K7C_MESSUNG`. Saiten des Pruefstands S199; UL2 a: 3 Cent, Abklingen −30,1 … "
         "+26,2 %. Zeit je Taste bzw. Saite auf einem Thread (Taste) bzw. mit dem Pool (Saite). Rangverlust: "
      << rv << (tasten.empty() ? std::string() : "; Tasten / Saiten: " + std::string(tastenText)) << ".\n";
    char z[512];
    for (const char* inst : {"Harfe", "Cembalo"})
    {
        if (!gewaehlt(inst)) continue;
        o << "\n## " << inst << " (Tafel je Taste)\n\n| Taste | f1 Hz | zaehlend | Abel-Smith | Glied | Biquads | Cent | Abklingen % | haelt | s |\n"
                                "|---|---|---|---|---|---|---|---|---|---|\n";
        double summe = 0.0;
        int gut = 0, alle = 0;
        const int von = std::string(inst) == "Harfe" ? 25 : 29, bis = std::string(inst) == "Harfe" ? 104 : 89;
        for (int taste = von; taste <= bis; ++taste)
        {
            const std::string name = "T" + std::to_string(taste);
            if (!tasteGewaehlt(name)) continue;
            Saitenwerte s;
            double L = 0.0;
            if (std::string(inst) == "Harfe")
            {
                const auto h = harfe(taste);
                if (!h) continue;
                std::tie(s, L) = *h;
            }
            else
                std::tie(s, L) = cembalo(taste);
            for (const auto& [fassung, optionen] : fassungen)
            {
                const std::string zeile = zeilenName(std::to_string(taste), fassung);
                const auto t0 = std::chrono::steady_clock::now();
                const auto t = mu::entwirfJeTaste(s, L, optionen);
                const double sek = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
                summe += sek;
                ++alle;
                if (!t)
                {
                    std::snprintf(z, sizeof z, "| %s | — | — | — | — | — | — | — | nicht rechenbar | %.1f |\n", zeile.c_str(), sek);
                    o << z;
                    continue;
                }
                gut += t->bewertung.haelt();
                std::snprintf(z, sizeof z, "| %s | %.2f | %d | %d | T%d | %zu | %.2f | %.2f | %s | %.1f |\n", zeile.c_str(), t->grundHz,
                              t->zaehlendMitZugabe, t->abelSmith, t->filter.gliedOrdnung, t->filter.kette.size(), t->bewertung.centGroesster,
                              t->bewertung.abklingenGroesster, t->bewertung.haelt() ? "ja" : "**nein**", sek);
                o << z;
                aufwandZeile(inst, name, fassung, t->aufwand);
                std::printf("%s %s %s %.1f s\n", inst, name.c_str(), fassung.c_str(), sek);
            }
        }
        std::snprintf(z, sizeof z, "\n%s: %d von %d Tasten halten UL2 a; Entwurf zusammen %.1f s.\n", inst, gut, alle, summe);
        o << z;
    }
    for (const char* inst : {"Gitarre", "Laute", "Oud"})
    {
        if (!gewaehlt(inst)) continue;
        o << "\n## " << inst << " (je Saite gebunden)\n\n| Saite | Biquads | Start | Daempfung | Buende | halb | Achtel | fremd Cent | "
                                "fremd Abklingen % | haelt | s |\n|---|---|---|---|---|---|---|---|---|---|---|\n";
        double summe = 0.0;
        for (const BenannteSaite& b : saitenVon(inst))
        {
            if (!tasteGewaehlt(b.name)) continue;
            for (const auto& [fassung, optionen] : fassungen)
            {
                const std::string zeile = zeilenName(b.name, fassung);
                const auto t0 = std::chrono::steady_clock::now();
                const auto t = mu::entwirfJeSaite(b.saite, b.mensur, b.buende, optionen);
                const double sek = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
                summe += sek;
                if (!t)
                {
                    o << "| " << zeile << " | — | — | — | — | — | — | — | — | nicht rechenbar | " << sek << " |\n";
                    continue;
                }
                const bool h =
                    t->buendeBewertung.haelt() && t->halbBewertung.haelt() && t->achtelBewertung.haelt() && t->fremdBewertung.haelt();
                std::snprintf(z, sizeof z, "| %s | %zu | %.2f | %.2f | %.2f | %.2f | %.2f | %.2f | %.2f | %s | %.1f |\n", zeile.c_str(),
                              t->radiusReihe.size(), t->startFaktor, t->daempfung, t->buendeBewertung.centGroesster,
                              t->halbBewertung.centGroesster, t->achtelBewertung.centGroesster, t->fremdBewertung.centGroesster,
                              t->fremdBewertung.abklingenGroesster, h ? "ja" : "**nein**", sek);
                o << z;
                aufwandZeile(inst, b.name, fassung, t->aufwand);
                std::printf("%s %s %s %.1f s\n", inst, b.name.c_str(), fassung.c_str(), sek);
            }
        }
        std::snprintf(z, sizeof z, "\n%s: Entwurf zusammen %.1f s.\n", inst, summe);
        o << z;
    }
    o << aufwand.str();
    std::ofstream(ziel) << o.str();
    MESSAGE("geschrieben: " << std::string(ziel));
}
