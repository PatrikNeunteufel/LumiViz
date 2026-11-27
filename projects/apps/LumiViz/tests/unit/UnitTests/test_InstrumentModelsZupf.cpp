/**
 ****************************************************************************************
 * @file   test_InstrumentModelsZupf.cpp
 * @brief  InstrumentModels, Stufe 34k: Klasse 2 (gezupfte Saite) — der Kern `pluckedString` Rechenfassung 2
 *         (docs/audio/Instrumente_IC_Konzept.md Schnittfolge 34k-7d-4 (c), (e); ZB1 a, ZN1 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine. Die gestimmte Tafel des Umlauf-Filters kommt aus `werte/zupf_umlauf.json`
 * (34k-7d-3); den Steg (Koerper) prueft `test_InstrumentModelsZupfBezug`, hier rechnet der Kern ohne `Steg` im Auftrag
 * (starrer Steg).
 *
 * - Tafel, Fabrik, Vorlagen, Werte hin und zurueck; die Saiten der Kurven gleich `zupf/Saiten`.
 * - Staerke aus der Stufe (GZ6 a, GZ10 a, ZD1 a, ZD2 a), Nachklang, Daempfer am Notenende (ZN1 a: in `dampingTime` um
 *   60 dB; der Leitwert gegen die Summe ueber die Umlaeufe).
 * - Der Bezug nach der Tafel; Saite und Bund aus der Note (ZK2 a).
 * - Die Pedale der Harfe (aus IC, ohne das Spiel), Determinismus auf Threads.
 * - Die zweite Tonhoehenmessung (`InstrumentMeasure/Grundton`): an gerechneten Toenen ohne bzw. mit schwachem Grundton auf
 *   ±1 Cent; an den Toenen der fuenf mit Rechenfassung 2 nur Zahlen (die Pruefung ±10 Cent kommt mit 34k-7f).
 * - ZB1 a: die Abdruecke von Rechenfassung 2 in `werte/zupf_ic.txt`, vom Kern geschrieben (`LUMIVIZ_ZUPF_ABDRUCK_SCHREIBEN=1`),
 *   Bit fuer Bit. Bis S224 trug die Datei die Abdruecke der IC-Vorlage (Rechenfassung 1, Beleg der Uebernahme J77).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Pool.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentMeasure/Bausteine.hpp"
#include "InstrumentMeasure/Grundton.hpp"
#include "InstrumentMeasure/Stimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/zupf/Pedale.hpp"
#include "InstrumentModels/zupf/ZupfKern.hpp"
#include "loop/KernDatei.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <numbers>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using namespace lumi::kern;
using namespace lumi::kern::zupf;

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr Frames kFrames = 28800;  // 0,6 s bei 48 kHz

double hzVon(int n) { return 440.0 * std::exp2((n - 69) / 12.0); }

/// Die gestimmte Tafel des Umlauf-Filters je Instrument aus `werte/zupf_umlauf.json` (34k-7d-3).
const std::map<std::string, Liste>& umlaufTafeln()
{
    static const std::map<std::string, Liste> m = [] {
        std::map<std::string, Liste> r;
        QFile f(QString::fromStdString((std::filesystem::path(__FILE__).parent_path() / "werte" / "zupf_umlauf.json").string()));
        if (!f.open(QIODevice::ReadOnly)) return r;
        const QJsonObject inst = QJsonDocument::fromJson(f.readAll()).object().value(QStringLiteral("instruments")).toObject();
        for (const Vorlage& v : vorlagen())
        {
            const QJsonObject o = inst.value(QString::fromStdString(v.name)).toObject();
            r[v.name] = lumi::loop::werteAusJson(kernTyp().tafel, o).liste(kUmlaufTafel);
        }
        return r;
    }();
    return m;
}

const Vorlage& vorlage(std::string_view name)
{
    for (const Vorlage& v : vorlagen())
        if (v.name == name) return v;
    FAIL("unbekannte Vorlage " << std::string(name));
    return vorlagen().front();
}

/// Die Werte einer Vorlage mit der gestimmten Tafel, gerichtet; mit `mitBezug` der Bezug nach der Tafel.
Werte gestimmteWerte(const Vorlage& v, bool mitBezug = true)
{
    Werte w = werteVon(v);
    REQUIRE(umlaufTafeln().count(v.name) == 1);
    w.setze(std::string(kUmlaufTafel), umlaufTafeln().at(v.name));
    REQUIRE(richteWerte(kernTyp().tafel, w).empty());
    if (mitBezug)
    {
        std::vector<std::string> fehler;
        const double b = normiereNachTafel(kerne(), kKernName, w, &fehler);
        REQUIRE(fehler.empty());
        w.setze(std::string(kBezug), b);
    }
    return w;
}

std::shared_ptr<const Tonkern> kernVon(std::string_view name, bool mitBezug = true)
{
    return kerne().erzeuge(kKernName, gestimmteWerte(vorlage(name), mitBezug));
}

std::vector<float> ton(const Tonkern& k, int note, int stufe, Frames halten, Frames frames = kFrames, int saite = -1)
{
    Tonauftrag a = einzelnote(note, hzVon(note), stufe, halten, frames, 48000.0);
    a.noten.front().saite = saite;
    std::vector<float> t;
    k.rechne(a, t);
    return t;
}

double rmsDb(const std::vector<float>& t, double von, double bis)
{
    const auto a = static_cast<std::size_t>(von * 48000.0), e = std::min(t.size(), static_cast<std::size_t>(bis * 48000.0));
    double s = 0.0;
    for (std::size_t i = a; i < e; ++i) s += double(t[i]) * t[i];
    return 10.0 * std::log10(s / std::max<std::size_t>(1, e - a) + 1e-300);
}

std::vector<double> teiltonTon(double f0, double steife, const std::vector<std::pair<int, double>>& teiltoene, double rauschen, double sekunden = 0.6)
{
    const int rate = 48000;
    std::vector<double> t(static_cast<std::size_t>(sekunden * rate), 0.0);
    for (const auto& [k, a] : teiltoene)
    {
        const double f = k * f0 * std::sqrt(1.0 + steife * k * k);
        const double tau = 1.5 / std::sqrt(double(k));
        for (std::size_t i = 0; i < t.size(); ++i)
        {
            const double s = double(i) / rate;
            t[i] += a * std::exp(-s / tau) * std::sin(2.0 * kPi * f * s + 0.7 * k);
        }
    }
    std::uint64_t z = 0x9e3779b97f4a7c15ull;
    for (double& x : t)
    {
        z = z * 6364136223846793005ull + 1442695040888963407ull;
        x += rauschen * (double(z >> 11) / double(1ull << 53) * 2.0 - 1.0);
    }
    return t;
}

double centZwischen(double ist, double soll) { return 1200.0 * std::log2(ist / soll); }

/// Die Faelle der Abdruecke: je Vorlage tief / mitte / hoch, Stufe 0 / −9 / −18, 0,4 s gehalten, 0,6 s.
struct AbdruckFall
{
    std::string name;
    std::string vorlage;
    int note, stufe;
};

std::vector<AbdruckFall> abdruckFaelle()
{
    std::vector<AbdruckFall> f;
    for (const Vorlage& v : vorlagen())
    {
        const int lo = v.spiel.noteMin, hi = v.spiel.noteMax, mi = (lo + hi) / 2;
        for (const int n : {lo, mi, hi})
            for (const int s : {0, -9, -18})
                f.push_back({"zupf2_" + v.name + "_" + std::to_string(n) + "_s" + std::to_string(-s), v.name, n, s});
    }
    return f;
}

std::filesystem::path abdruckDatei() { return std::filesystem::path(__FILE__).parent_path() / "werte" / "zupf_ic.txt"; }

}  // namespace

TEST_CASE("InstrumentModels 34k-7d-4 Kern pluckedString: Tafel, Fabrik, Vorlagen, Werte hin und zurueck")
{
    const KernTyp& typ = kernTyp();
    const std::vector<std::string> pruefung = pruefeTafel(typ.tafel);
    for (const std::string& p : pruefung) MESSAGE(p);
    CHECK(pruefung.empty());
    CHECK(typ.tafel.rechenFassung == 2);
    CHECK(typ.auskunft.stegImAuftrag);
    REQUIRE(kerne().typ(kKernName) != nullptr);
    const ParameterAngabe* tafel = nullptr;
    for (const ParameterAngabe& a : typ.tafel.werte)
        if (a.name == kUmlaufTafel) tafel = &a;
    REQUIRE(tafel != nullptr);
    CHECK(tafel->gestimmt);
    CHECK(vorlagen().size() == 5u);
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        // hin und zurueck
        const Werte w = werteAus(v.spiel);
        CHECK(werteAus(spielAus(w)) == w);
        // gerichtet (mit Tafel) bleibt das Spiel, und der Kern entsteht
        const Werte g = gestimmteWerte(v, false);
        const ZupfSpiel s = spielAus(g);
        CHECK(s.noteMin == v.spiel.noteMin);
        CHECK(s.noteMax == v.spiel.noteMax);
        CHECK(s.jeSaite() == v.spiel.jeSaite());
        CHECK(!s.umlauf.empty());
        REQUIRE(kerne().erzeuge(kKernName, g) != nullptr);
        // die Saiten der Kurven sind die von `zupf/Saiten`
        if (!s.jeSaite())
            for (int t = s.noteMin; t <= s.noteMax; ++t)
            {
                CAPTURE(t);
                const std::optional<Tastensaite> soll = v.name == "harpsichord" ? std::optional<Tastensaite>(cembaloSaite(t)) : harfenSaite(t);
                const std::optional<Tastensaite> ist = tastensaiteAus(s, t);
                REQUIRE(soll.has_value() == ist.has_value());
                if (!soll) continue;
                CHECK(ist->werte == soll->werte);
                CHECK(ist->laenge == soll->laenge);
                CHECK(ist->zupfort == soll->zupfort);
                CHECK(ist->stegTaste == soll->stegTaste);
            }
        else
        {
            REQUIRE(s.satz.choere.size() == v.spiel.satz.choere.size());
            for (std::size_t c = 0; c < s.satz.choere.size(); ++c)
            {
                CHECK(s.satz.choere[c].leer == v.spiel.satz.choere[c].leer);
                REQUIRE(s.satz.choere[c].saiten.size() == v.spiel.satz.choere[c].saiten.size());
                for (std::size_t i = 0; i < s.satz.choere[c].saiten.size(); ++i)
                {
                    CHECK(s.satz.choere[c].saiten[i].werte == v.spiel.satz.choere[c].saiten[i].werte);
                    CHECK(s.satz.choere[c].saiten[i].oktave == v.spiel.satz.choere[c].saiten[i].oktave);
                    CHECK(s.satz.choere[c].saiten[i].laengenFaktor == v.spiel.satz.choere[c].saiten[i].laengenFaktor);
                }
            }
            CHECK(s.satz.mensur == v.spiel.satz.mensur);
            CHECK(s.satz.buende == v.spiel.satz.buende);
        }
    }
}

TEST_CASE("InstrumentModels 34k-7d-4 Kern pluckedString: Staerke aus der Stufe, Saite und Bund, Nachklang")
{
    // GZ6 a: die Gitarre um 10^(Stufe/20) in der Auslenkung, Neigung 0,16; GZ10 a: das Cembalo 1 dB bis pp
    const ZupfSpiel g = spielAus(gestimmteWerte(vorlage("classical_guitar"), false));
    const ZupfNote ff = zupfNote(g, 57, -1, 0, 19200, 48000.0), pp = zupfNote(g, 57, -1, -18, 19200, 48000.0);
    REQUIRE(ff.fehler.empty());
    REQUIRE(pp.fehler.empty());
    CHECK(ff.zupf.auslenkung == doctest::Approx(0.004).epsilon(1e-12));
    CHECK(pp.zupf.auslenkung == doctest::Approx(0.004 * std::pow(10.0, -18.0 / 20.0)).epsilon(1e-12));
    CHECK(pp.zupf.neigung == -18.0);
    CHECK(pp.zupf.kipp == 0.16);
    CHECK(ff.zupf.ort == 0.16);
    const ZupfSpiel c = spielAus(gestimmteWerte(vorlage("harpsichord"), false));
    const ZupfNote cff = zupfNote(c, 60, -1, 0, 19200, 48000.0), cpp = zupfNote(c, 60, -1, -18, 19200, 48000.0);
    REQUIRE(cff.fehler.empty());
    CHECK(20.0 * std::log10(cpp.zupf.auslenkung / cff.zupf.auslenkung) == doctest::Approx(-1.0).epsilon(1e-9));
    CHECK(cpp.zupf.kipp == 0.0);
    // die Kraft an C4 bei ff 1,07 N (KQ30 a)
    const Tastensaite c4 = cembaloSaite(60);
    CHECK(cff.zupf.auslenkung * c4.werte.zug * c4.laenge / (c4.zupfort * (c4.laenge - c4.zupfort)) == doctest::Approx(1.07).epsilon(1e-9));
    // die Harfe: Auslenkung und Abloesetempo im selben Verhaeltnis, keine Neigung (ZD1-1 a)
    const ZupfSpiel h = spielAus(gestimmteWerte(vorlage("concert_harp"), false));
    const ZupfNote hp = zupfNote(h, 60, -1, -18, 19200, 48000.0);
    REQUIRE(hp.fehler.empty());
    CHECK(hp.zupf.tempo / hp.zupf.auslenkung == doctest::Approx(2.0 / 0.004).epsilon(1e-12));
    CHECK(hp.zupf.kipp == 0.0);
    // Saite und Bund aus der Note (ZK2 a): A3 (57) auf der D-Saite (Index 2, Bund 7) statt der G-Saite
    const ZupfNote auf = zupfNote(g, 57, 2, 0, 19200, 48000.0);
    REQUIRE(auf.fehler.empty());
    CHECK(auf.stegTaste == 50);
    CHECK(ff.stegTaste == 55);
    CHECK(auf.saiten.front().laenge == doctest::Approx(0.65 * std::exp2(-7.0 / 12.0)).epsilon(1e-12));
    CHECK_FALSE(zupfNote(g, 39, -1, 0, 19200, 48000.0).fehler.empty());  // unter der tiefsten Saite
    CHECK_FALSE(zupfNote(g, 57, 5, 0, 19200, 48000.0).fehler.empty());   // A3 liegt nicht auf der E4-Saite
    CHECK_FALSE(zupfNote(g, 57, -1, 0, 19200, 44100.0).fehler.empty());  // nur 48 kHz
    // der Nachklang: Vorlauf, dampingTime, 20 ms (ohne Steg)
    const auto k = kernVon("classical_guitar", false);
    const Tonauftrag a = einzelnote(57, hzVon(57), 0, 19200, kFrames, 48000.0);
    CHECK(k->nachklang(a, 1 << 20) == static_cast<Frames>(std::ceil((kVorlauf + 0.08 + 0.02) * 48000.0)));
}

TEST_CASE("InstrumentModels 34k-7d-4 Kern pluckedString: der Daempfer am Notenende (ZN1 a)")
{
    // der Leitwert: ueber die Rampe faellt die Saite um 60 dB — f₁ ∫₀^τ 20 lg ρ(t) dt, ρ = (1 − Z₀ G t/τ) / (1 + Z₀ G t/τ),
    // als Summe in 10 000 Schritten gegen −60 dB
    for (const auto [z0, f1, tau] : {std::tuple{0.6, 110.0, 0.08}, std::tuple{0.3, 440.0, 0.06}, std::tuple{1.2, 70.0, 0.15}})
    {
        const double G = leitwertFuerAbklingen(z0, f1, tau);
        const int n = 10000;
        double db = 0.0;
        for (int i = 0; i < n; ++i)
        {
            const double zg = z0 * G * (i + 0.5) / n;
            db += f1 * (tau / n) * 20.0 * std::log10((1.0 - zg) / (1.0 + zg));
        }
        CAPTURE(f1);
        CHECK(db == doctest::Approx(-60.0).epsilon(0.02));
    }
    // zu wenige Umlaeufe: der staerkste Daempfer
    CHECK(leitwertFuerAbklingen(1.0, 30.0, 0.1) == doctest::Approx(0.999));
    // am Ton: nach dem Loslassen in dampingTime um rund 60 dB (die Saite allein klingt in der Zeit kaum ab)
    for (const char* name : {"classical_guitar", "harpsichord", "lute"})
    {
        CAPTURE(std::string(name));
        const auto k = kernVon(name, false);
        const int note = std::string(name) == "harpsichord" ? 60 : 57;
        const double daempfen = vorlage(name).spiel.daempfenS;
        const Frames halten = 24000;  // 0,5 s
        const std::vector<float> t = ton(*k, note, 0, halten, 72000);
        const std::vector<float> frei = ton(*k, note, 0, 70000, 72000);
        const double los = kVorlauf + 0.5;
        const double vorher = rmsDb(t, los - 0.05, los), nachher = rmsDb(t, los + daempfen, los + daempfen + 0.05);
        const double freiAbfall = rmsDb(frei, los - 0.05, los) - rmsDb(frei, los + daempfen, los + daempfen + 0.05);
        MESSAGE(name << ": nach dem Loslassen " << vorher - nachher << " dB in " << daempfen << " s (ohne Daempfer " << freiAbfall << " dB)");
        CHECK(vorher - nachher - freiAbfall > 50.0);
        // vor dem Loslassen Bit fuer Bit wie ungedaempft
        const auto bis = static_cast<std::size_t>(los * 48000.0);
        CHECK(std::equal(t.begin(), t.begin() + static_cast<std::ptrdiff_t>(bis), frei.begin()));
    }
}

TEST_CASE("InstrumentModels 34k-7d-4 Kern pluckedString: der Bezug nach der Tafel, Ton und Kraft am Steg")
{
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const Werte w = gestimmteWerte(v, true);
        const double bezug = std::get<double>(*w.finde(kBezug));
        CHECK(bezug > 0.0);
        // die Probe des Bezugs (Mitte des Umfangs, Stufe 0, 1 s): Spitze 0,5
        const auto k = kerne().erzeuge(kKernName, w);
        const Tonauftrag probe = bezugsProbe(kernTyp().tafel, w);
        std::vector<float> t;
        k->rechne(probe, t);
        float spitze = 0.0f;
        for (const float x : t) spitze = std::max(spitze, std::abs(x));
        CHECK(spitze == doctest::Approx(0.5).epsilon(1e-3));
        // die Kraft am Steg: eine, an der Taste des Stegpunkts, gleich dem Ton
        std::vector<Stegkraft> kr;
        k->rechneAmSteg(probe, kr);
        REQUIRE(kr.size() == 1u);
        CHECK(kr.front().kraft == t);
        CHECK(kr.front().taste >= 0);
    }
    // ohne Note still, Taste −1
    const auto k = kernVon("oud", false);
    Tonauftrag leer;
    leer.frames = 100;
    std::vector<Stegkraft> kr;
    k->rechneAmSteg(leer, kr);
    REQUIRE(kr.size() == 1u);
    CHECK(kr.front().taste == -1);
    CHECK(std::all_of(kr.front().kraft.begin(), kr.front().kraft.end(), [](float x) { return x == 0.0f; }));
}

TEST_CASE("InstrumentModels 34k-7d-4 Harfe Pedale — Diagramm, Vorschlag, Bisbigliando, Glissando, Spur, nicht spielbar")
{
    const Pedale cDur{0, 0, 0, 0, 0, 0, 0};
    const std::vector<int> saiten = harfenSaiten(24, 103, cDur);
    CHECK(saiten.size() == 47u);
    CHECK(saiten.front() == 24);
    bool ok = false;
    const Pedale d = pedaleVorschlag({62, 64, 66, 67, 69, 71, 73}, &ok);
    CHECK(ok);
    CHECK(d == Pedale{1, 0, 0, 1, 0, 0, 0});
    const Pedale eis{0, 0, 1, 0, 0, 0, 0};
    CHECK(saitenFuer(65, eis).size() == 2u);
    CHECK_FALSE(spielbar(64, eis));
    const std::vector<int> gl = glissandoNoten(24, 103, 60, 72, cDur);
    CHECK(gl == std::vector<int>{60, 62, 64, 65, 67, 69, 71, 72});
    CHECK(glissandoNoten(24, 103, 72, 60, cDur).front() == 72);
    (void)pedaleVorschlag({60, 61, 62, 63, 64, 65, 66, 67}, &ok);
    CHECK_FALSE(ok);
    std::vector<PedalNote> chromatisch;
    for (int n = 60; n <= 72; ++n) chromatisch.push_back({0.25 * (n - 60), n});
    const std::vector<PedalStelle> spur = pedalSpurVorschlag(chromatisch);
    CHECK(spur.size() > 1u);
    for (const PedalNote& cn : chromatisch)
    {
        const PedalStelle* gilt = &spur.front();
        for (const PedalStelle& st : spur)
            if (st.abS <= cn.abS) gilt = &st;
        CAPTURE(cn.note);
        CHECK(spielbar(cn.note, gilt->pedale));
    }
    const std::vector<int> mitD = harfenSaiten(24, 103, d);
    CHECK(std::find(mitD.begin(), mitD.end(), 66) != mitD.end());
    CHECK(std::find(mitD.begin(), mitD.end(), 65) == mitD.end());
}

TEST_CASE("InstrumentModels 34k-7d-4 Kern pluckedString: Determinismus auf Threads")
{
    const auto k = kernVon("classical_guitar", false);
    const std::vector<float> a = ton(*k, 52, 0, 19200);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { q[i] = ton(*k, 52, 0, 19200); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
}

TEST_CASE("InstrumentModels 34k-7d-4 Kern pluckedString: die Abdruecke in werte/zupf_ic.txt (ZB1 a)")
{
    std::map<std::string, std::shared_ptr<const Tonkern>> kerne_;
    for (const Vorlage& v : vorlagen()) kerne_[v.name] = kernVon(v.name, true);
    const std::vector<AbdruckFall> faelle = abdruckFaelle();
    std::vector<std::string> ist(faelle.size());
    parallelJe(faelle.size(), [&](std::size_t i) {
        const AbdruckFall& f = faelle[i];
        ist[i] = abdruck(ton(*kerne_.at(f.vorlage), f.note, f.stufe, 19200));
    });
    if (const char* s = std::getenv("LUMIVIZ_ZUPF_ABDRUCK_SCHREIBEN"); s != nullptr && std::string(s) == "1")
    {
        std::ofstream o(abdruckDatei(), std::ios::binary);
        o << "# Abdruecke des Kerns pluckedString, Rechenfassung 2 (34k-7d-4, S224; ZB1 a) - vom Kern geschrieben, nicht von Hand\n"
             "# aendern: LUMIVIZ_ZUPF_ABDRUCK_SCHREIBEN=1 im Fall \"die Abdruecke in werte/zupf_ic.txt\" (test_InstrumentModelsZupf.cpp)\n"
             "# VERMERK: bis S224 trug diese Datei die Abdruecke der IC-Vorlage (Rechenfassung 1, InstrumentsCalculation 4409088,\n"
             "#   docs/modellierung/anbindung-ic/abdruck_zupf.cpp) - Beleg der Uebernahme J77, abgeschlossen; Rechenfassung 2 ersetzt\n"
             "#   sie (ZB1 a, Journal J81)\n"
             "# abdruck <name> <sha256>: je Vorlage tief / mitte / hoch, Stufe 0 / -9 / -18 (_s<betrag>), 0,4 s gehalten, 28800 Frames\n"
             "#   bei 48 kHz, ohne Steg im Auftrag (starrer Steg), mit der Tafel aus werte/zupf_umlauf.json und dem Bezug nach der Tafel\n";
        for (std::size_t i = 0; i < faelle.size(); ++i) o << "abdruck " << faelle[i].name << " " << ist[i] << "\n";
        MESSAGE("geschrieben: " << abdruckDatei().string());
        return;
    }
    std::map<std::string, std::string> soll;
    std::ifstream f(abdruckDatei());
    std::string zeile;
    while (std::getline(f, zeile))
    {
        std::istringstream z(zeile);
        std::string art, name, wert;
        if (z >> art >> name >> wert && art == "abdruck") soll[name] = wert;
    }
    REQUIRE_MESSAGE(soll.size() == faelle.size(), "werte/zupf_ic.txt passt nicht - LUMIVIZ_ZUPF_ABDRUCK_SCHREIBEN=1 schreibt sie neu");
    for (std::size_t i = 0; i < faelle.size(); ++i)
    {
        CAPTURE(faelle[i].name);
        CHECK(ist[i] == soll[faelle[i].name]);
    }
}

TEST_CASE("InstrumentModels 34k-3 zweite Tonhoehenmessung: Grundton aus den Teiltoenen ohne und mit schwachem Grundton")
{
    using lumi::messung::grundtonAusTeiltoenen;
    // (1) ohne Grundton: G2 + 13,7 Cent, Teiltoene 2 … 10, Rauschen −80 dB
    {
        const double f0 = hzVon(43) * std::exp2(13.7 / 1200.0);
        std::vector<std::pair<int, double>> tt;
        for (int k = 2; k <= 10; ++k) tt.emplace_back(k, 1.0 / k);
        const std::vector<double> t = teiltonTon(f0, 0.0, tt, 1e-4);
        const lumi::messung::Grundton g = grundtonAusTeiltoenen(t, 48000, hzVon(43));
        REQUIRE(g.gefunden);
        MESSAGE("ohne Grundton: " << centZwischen(g.hz, f0) << " Cent daneben aus " << g.teiltoene << " Teiltoenen, B = " << g.steife);
        CHECK(std::abs(centZwischen(g.hz, f0)) < 1.0);
        CHECK(g.teiltoene == 9);
        // YIN (B2) als Median der stimmhaften Rahmen (nur Zahl)
        const lumi::messung::Tonhoehe y = lumi::messung::tonhoehe(t, 48000, 30.0, 3000.0);
        std::vector<double> hz;
        for (const double x : y.hz)
            if (x > 0.0) hz.push_back(x);
        if (!hz.empty())
        {
            std::nth_element(hz.begin(), hz.begin() + hz.size() / 2, hz.end());
            MESSAGE("YIN am selben Ton: " << centZwischen(hz[hz.size() / 2], f0) << " Cent (Median)");
        }
    }
    // (2) schwacher Grundton (−40 dB) an einer steifen Saite: A1 − 21,3 Cent, B = 3·10⁻⁴, Teiltoene 1 … 12
    {
        const double f0 = hzVon(33) * std::exp2(-21.3 / 1200.0);
        std::vector<std::pair<int, double>> tt = {{1, 0.01}};
        for (int k = 2; k <= 12; ++k) tt.emplace_back(k, 1.0 / k);
        const std::vector<double> t = teiltonTon(f0, 3e-4, tt, 1e-4);
        const lumi::messung::Grundton g = grundtonAusTeiltoenen(t, 48000, hzVon(33));
        REQUIRE(g.gefunden);
        MESSAGE("schwacher Grundton, steif: " << centZwischen(g.hz, f0) << " Cent daneben aus " << g.teiltoene << " Teiltoenen, B = " << g.steife);
        CHECK(std::abs(centZwischen(g.hz, f0)) < 1.0);
        CHECK(g.steife == doctest::Approx(3e-4).epsilon(0.1));
    }
    // (3) die Schaetzung 40 Cent daneben: der zweite Durchgang findet den Ton
    {
        const double f0 = hzVon(57);
        std::vector<std::pair<int, double>> tt;
        for (int k = 2; k <= 8; ++k) tt.emplace_back(k, 1.0);
        const std::vector<double> t = teiltonTon(f0, 0.0, tt, 1e-4);
        const lumi::messung::Grundton g = grundtonAusTeiltoenen(t, 48000, f0 * std::exp2(40.0 / 1200.0));
        REQUIRE(g.gefunden);
        CHECK(std::abs(centZwischen(g.hz, f0)) < 1.0);
    }
    // (4) ohne Signal nichts
    CHECK_FALSE(grundtonAusTeiltoenen(std::vector<double>(28800, 0.0), 48000, 110.0).gefunden);
    CHECK_FALSE(grundtonAusTeiltoenen(std::vector<double>(2000, 0.0), 48000, 110.0).gefunden);
}

TEST_CASE("InstrumentModels 34k-7d-4 zweite Tonhoehenmessung an den Toenen der fuenf (nur Zahlen)")
{
    // die Pruefung ±10 Cent an jeder Taste kommt mit 34k-7f; hier, wie weit Rechenfassung 2 mit starrem Steg liegt
    for (const Vorlage& v : vorlagen())
    {
        const auto k = kernVon(v.name, false);
        const int n = v.spiel.noteMax - v.spiel.noteMin + 1;
        std::vector<double> cent(static_cast<std::size_t>(n), 0.0);
        parallelJe(static_cast<std::size_t>(n), [&](std::size_t i) {
            const int note = v.spiel.noteMin + static_cast<int>(i);
            const std::vector<float> t = ton(*k, note, 0, 28800);
            const std::vector<double> d(t.begin(), t.end());
            const lumi::messung::Grundton g = lumi::messung::grundtonAusTeiltoenen(d, 48000, hzVon(note));
            cent[i] = g.gefunden ? centZwischen(g.hz, hzVon(note)) : 1e9;
        });
        double schlimmste = 0.0;
        int wo = v.spiel.noteMin, ueber10 = 0, ohne = 0;
        for (int i = 0; i < n; ++i)
        {
            const double c = cent[static_cast<std::size_t>(i)];
            if (c > 1e8)
            {
                ++ohne;
                continue;
            }
            if (std::abs(c) > 10.0) ++ueber10;
            if (std::abs(c) > std::abs(schlimmste))
            {
                schlimmste = c;
                wo = v.spiel.noteMin + i;
            }
        }
        MESSAGE(v.name << ": " << n << " Tasten, schlimmste " << schlimmste << " Cent an " << wo << ", ueber 10 Cent " << ueber10
                       << ", ohne Ergebnis " << ohne);
        CHECK(ohne == 0);
    }
}
