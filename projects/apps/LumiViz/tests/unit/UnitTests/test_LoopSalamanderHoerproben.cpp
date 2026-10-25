/**
 ****************************************************************************************
 * @file   test_LoopSalamanderHoerproben.cpp
 * @brief  Loop, Stufe 31d-5: Hoerproben gegen Salamander, gleich laut nach EBU R128
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S151: UF4 a, UF7 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * `LUMIVIZ_31D_WAV=<Ordner>` zusammen mit `LUMIVIZ_SALAMANDER=<Ordner>` legt die Proben ab
 * (Patrik hoert sie gesammelt spaeter); ohne die beiden meldet der Fall das und prueft nichts.
 *
 * UF7 a: A0, C2, C4, C6, C8 bei mf und C4 bei pp und ff; je Ton hintereinander Salamander, das
 * Modell vor 31d (die Vorgabe vor 31d-4, hier ausgeschrieben) und das Modell nach 31d (die
 * Vorgabe); jede Fassung auf −23 LUFS integriert (`audio/Lautheit`); die Schicht von Salamander
 * ordnet die Messung zu (Pegel zu Beginn gegen v16 wie die Stufe gegen 0 dB am Modell); gehalten
 * so lang wie die Aufnahme vor ihrer Blende, dann geschnitten (50 ms ausgeblendet). UF4 a: an C4
 * benachbarte Ebenen von Salamander und Stufen des Modells mit 1, 2, 3 und 6 dB Abstand, je Paar
 * gleich laut — fuer den Stufenschritt nach Patriks Urteil. `liste.txt` mit Nennung CC BY 3.0.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "ModellMessung.hpp"

#include "audio/Lautheit.hpp"
#include "audio/WavDatei.hpp"
#include "audio/WavFormat.hpp"
#include "loop/HammerSaite.hpp"
#include "loop/Tonspeicher.hpp"

#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace lumi::messung;

namespace
{

constexpr double kZielLufs = -23.0;
constexpr int kRate = modelltest::kRate;

/// Die Vorgabe vor 31d-4 (Stand 339d40a2): die Groessen, die 31d-4 gesetzt hat, wie sie vorher waren.
lumi::loop::HammerSaite vorgabeVor31d()
{
    lumi::loop::HammerSaite s = lumi::loop::hammerSaiteVorgabe();
    s.inharmonizitaet.punkte.clear();
    for (const int m : {21, 33, 45, 57, 69, 81, 93, 105, 108})
        s.inharmonizitaet.punkte.push_back({m, lumi::loop::rigaudB(m, lumi::loop::kRigaudBassSteigung, lumi::loop::kRigaudBassAchse)});
    s.stegWinkelGrad = {{{21, 2.0}, {108, 2.0}}, {}};
    s.abklingS = {{{21, 25.0}, {33, 20.0}, {45, 15.0}, {57, 10.0}, {69, 6.0}, {81, 3.0}, {93, 1.5}, {105, 0.7}, {108, 0.5}}, {}};
    s.abklingSpaetS = {{{21, 75.0}, {33, 60.0}, {45, 45.0}, {57, 30.0}, {69, 18.0}, {81, 9.0}, {93, 4.5}, {105, 2.1}, {108, 1.5}}, {}};
    s.abklingHoch = {{{21, 0.2}, {108, 0.2}}, {}};
    s.verstimmungCent = {{{21, 1.0}, {108, 1.0}}, {}};
    return s;
}

/// Der Pegel zu Beginn eines Stereotons (beide Kanaele zusammen), in dB.
double pegel(const std::vector<float>& stereo)
{
    auto [l, r] = trenne(stereo);
    for (std::size_t i = 0; i < l.size(); ++i) l[i] = 0.5 * (l[i] + r[i]);
    double spitze = 0.0;
    for (const double x : l) spitze = std::max(spitze, std::abs(x));
    std::size_t einsatz = 0;
    while (einsatz < l.size() && std::abs(l[einsatz]) <= 0.01 * spitze) ++einsatz;
    return pegelZuBeginn(l, kRate, static_cast<double>(einsatz) / kRate);
}

/// Auf `frames` geschnitten, die letzten 50 ms ausgeblendet, auf `kZielLufs` gestellt; zurueck die Verstaerkung in dB.
double richte(std::vector<float>& stereo, std::size_t frames)
{
    stereo.resize(2 * std::min(frames, stereo.size() / 2));
    const std::size_t blende = std::min<std::size_t>(stereo.size() / 2, kRate / 20);
    for (std::size_t f = 0; f < blende; ++f)
    {
        const auto faktor = static_cast<float>(static_cast<double>(f) / static_cast<double>(blende));
        const std::size_t i = stereo.size() / 2 - 1 - f;
        stereo[2 * i] *= faktor;
        stereo[2 * i + 1] *= faktor;
    }
    const double lufs = lumi::audio::messeLautheit(stereo, kRate).lufs;
    const double db = kZielLufs - lufs;
    const auto faktor = static_cast<float>(std::pow(10.0, db / 20.0));
    for (float& v : stereo) v *= faktor;
    return db;
}

double spitzeDb(const std::vector<float>& stereo)
{
    float s = 0.0f;
    for (const float v : stereo) s = std::max(s, std::abs(v));
    return 20.0 * std::log10(std::max(s, 1e-9f));
}

/// Die Fassungen gemeinsam so weit leiser, dass die hoechste Spitze bei −1 dBFS liegt (untereinander bleiben
/// sie gleich laut); zurueck die Absenkung in dB (0, wenn keine noetig).
double unterSpitze(std::vector<std::vector<float>*> fassungen)
{
    double spitze = -300.0;
    for (const std::vector<float>* f : fassungen) spitze = std::max(spitze, spitzeDb(*f));
    if (spitze <= -1.0) return 0.0;
    const auto faktor = static_cast<float>(std::pow(10.0, (-1.0 - spitze) / 20.0));
    for (std::vector<float>* f : fassungen)
        for (float& v : *f) v *= faktor;
    return spitze + 1.0;
}

/// Teile hintereinander mit einer Sekunde Stille dazwischen.
std::vector<float> folge(const std::vector<std::vector<float>>& teile)
{
    std::vector<float> f;
    for (std::size_t i = 0; i < teile.size(); ++i)
    {
        if (i > 0) f.insert(f.end(), 2 * static_cast<std::size_t>(kRate), 0.0f);
        f.insert(f.end(), teile[i].begin(), teile[i].end());
    }
    return f;
}

}  // namespace

TEST_CASE("Loop Salamander 31d-5 Hoerproben (LUMIVIZ_31D_WAV)")
{
    const QString zielVariable = qEnvironmentVariable("LUMIVIZ_31D_WAV");
    const QString salamanderVariable = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    if (zielVariable.isEmpty() || salamanderVariable.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D_WAV=<Ordner> mit LUMIVIZ_SALAMANDER=<Ordner> legt die Hoerproben 31d ab");
        return;
    }
    const std::filesystem::path ziel = zielVariable.toStdU16String();
    const std::filesystem::path salamander = salamanderVariable.toStdU16String();
    std::filesystem::create_directories(ziel);
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();

    const lumi::loop::HammerSaite vorSaite = vorgabeVor31d();
    lumi::loop::Instrument vor = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &vorSaite);
    vor.tastatur->ungedaempftAb = 91;  // die Vorgabe vor D1 a
    const lumi::loop::Instrument nach = modelltest::fluegel(QStringLiteral("lib:steinway_d"));
    const auto taste = [](int midi) {
        for (const SalamanderTaste& t : salamanderTasten())
            if (t.midi == midi) return t;
        return SalamanderTaste{};
    };
    const auto liesEbene = [&](int midi, int ebene) {
        const std::optional<lumi::audio::WavInhalt> wav = lumi::audio::liesWavDatei(salamanderDatei(salamander, taste(midi), ebene));
        REQUIRE_MESSAGE(wav.has_value(), salamanderDatei(salamander, taste(midi), ebene).string());
        REQUIRE(wav->rate == kRate);
        return wav->stereo;
    };
    const auto rechne = [](const lumi::loop::Instrument& instrument, int midi, int stufe, double halten) {
        lumi::loop::TonAngabe angabe;
        angabe.hoehe = midi;
        angabe.dauerMs = static_cast<int>(1000.0 * halten);
        angabe.anschlagStufe = stufe;
        return lumi::loop::rendereTon(instrument, angabe, kRate).daten;
    };

    std::ostringstream liste;
    liste << std::fixed << std::setprecision(1)
          << "Hoerproben 31d (UF7 a, UF4 a), gleich laut auf " << kZielLufs << " LUFS integriert (EBU R128).\n"
          << "Salamander Grand Piano V3 (Yamaha C5), Alexander Holm 2011, CC BY 3.0 (http://creativecommons.org/licenses/by/3.0/),\n"
          << "Fassung 48 kHz/24 bit von archive.org; hier geschnitten und im Pegel angeglichen.\n"
          << "Modell: Hammer-Saite mit dem Boden des Steinway D (lib:steinway_d), wie im Programm.\n\n"
          << "Je Datei hintereinander, mit einer Sekunde Stille: Salamander | Modell vor 31d | Modell nach 31d-4\n"
          << "(nach der Nacharbeit 31d-6 kommt eine vierte Fassung dazu).\n\n";

    // UF7 a: die Toene
    const struct
    {
        int midi;
        const char* name;
        const char* staerke;
        int stufe;
    } toene[] = {{21, "a0", "mf", -12}, {36, "c2", "mf", -12}, {60, "c4", "mf", -12}, {84, "c6", "mf", -12},
                 {108, "c8", "mf", -12}, {60, "c4", "pp", -26}, {60, "c4", "ff", 0}};
    int nr = 1;
    for (const auto& t : toene)
    {
        CAPTURE(t.midi);
        // die Schicht von Salamander: Pegel zu Beginn gegen v16 wie die Stufe gegen 0 dB am Modell (nach 31d)
        const double modellAbstand = pegel(rechne(nach, t.midi, t.stufe, 1.0)) - pegel(rechne(nach, t.midi, 0, 1.0));
        const double laut = pegel(liesEbene(t.midi, kEbenen));
        int ebene = kEbenen;
        double bester = 1e9;
        for (int e = 1; e <= kEbenen; ++e)
        {
            const double d = std::abs(pegel(liesEbene(t.midi, e)) - laut - modellAbstand);
            if (d < bester)
            {
                bester = d;
                ebene = e;
            }
        }
        std::vector<float> sal = liesEbene(t.midi, ebene);
        std::vector<double> summe(sal.size() / 2);
        for (std::size_t i = 0; i < summe.size(); ++i) summe[i] = static_cast<double>(sal[2 * i]) + sal[2 * i + 1];
        const std::size_t frames = endeVorBlende(summe, kRate);
        const double haltenS = static_cast<double>(frames) / kRate;
        std::vector<float> v = rechne(vor, t.midi, t.stufe, haltenS);
        std::vector<float> n = rechne(nach, t.midi, t.stufe, haltenS);
        const double gs = richte(sal, frames);
        const double gv = richte(v, frames);
        const double gn = richte(n, frames);
        const double leiser = unterSpitze({&sal, &v, &n});
        const std::string datei = (nr < 10 ? "0" : "") + std::to_string(nr) + "_" + t.name + "_" + t.staerke + ".wav";
        const std::vector<float> f = folge({sal, v, n});
        REQUIRE(lumi::audio::schreibeWav(QString::fromStdU16String((ziel / datei).u16string()), f, kRate, lumi::audio::WavTiefe::Gleitkomma32)
                    .geschrieben);
        liste << datei << ": " << t.name << ' ' << t.staerke << " (Stufe " << t.stufe << " dB; Salamander v" << ebene
              << ", Abstand zu v16 wie die Stufe zu 0 dB auf " << bester << " dB), je " << haltenS << " s; Verstaerkung Salamander "
              << gs << " dB, vor " << gv << " dB, nach " << gn << " dB" << (leiser > 0.0 ? "; alle drei gemeinsam " : "")
              << (leiser > 0.0 ? std::to_string(leiser).substr(0, 4) + " dB leiser (Spitze −1 dBFS), also " +
                                     std::to_string(kZielLufs - leiser).substr(0, 5) + " LUFS"
                               : std::string())
              << "; Spitzen " << spitzeDb(sal) << " / " << spitzeDb(v) << " / " << spitzeDb(n) << " dBFS\n";
        ++nr;
    }

    // UF4 a: benachbarte Stufen an C4, je Paar gleich laut, je 3 s
    const auto frames3 = static_cast<std::size_t>(3 * kRate);
    std::vector<std::vector<float>> salPaare;
    liste << "\n" << (nr < 10 ? "0" : "") << nr << "_c4_stufen_salamander.wav: Paare benachbarter Ebenen, je Paar gleich laut, A und B mit 1 s Stille, ebenso zwischen den Paaren:";
    for (const int e : {4, 8, 12})
    {
        std::vector<float> a = liesEbene(60, e);
        std::vector<float> b = liesEbene(60, e + 1);
        richte(a, frames3);
        richte(b, frames3);
        unterSpitze({&a, &b});
        salPaare.push_back(folge({a, b}));
        liste << " v" << e << "/v" << e + 1;
    }
    REQUIRE(lumi::audio::schreibeWav(QString::fromStdU16String(
                                         (ziel / ((nr < 10 ? "0" : "") + std::to_string(nr) + "_c4_stufen_salamander.wav")).u16string()),
                                     folge(salPaare), kRate, lumi::audio::WavTiefe::Gleitkomma32)
                .geschrieben);
    ++nr;
    std::vector<std::vector<float>> modellPaare;
    liste << "\n" << (nr < 10 ? "0" : "") << nr << "_c4_stufen_modell.wav: Modell nach 31d, Stufe −12 dB gegen −12 + d, je Paar gleich laut:";
    for (const int d : {1, 2, 3, 6})
    {
        std::vector<float> a = rechne(nach, 60, -12, 3.0);
        std::vector<float> b = rechne(nach, 60, -12 + d, 3.0);
        richte(a, frames3);
        richte(b, frames3);
        unterSpitze({&a, &b});
        modellPaare.push_back(folge({a, b}));
        liste << " d = " << d << " dB";
    }
    REQUIRE(lumi::audio::schreibeWav(QString::fromStdU16String(
                                         (ziel / ((nr < 10 ? "0" : "") + std::to_string(nr) + "_c4_stufen_modell.wav")).u16string()),
                                     folge(modellPaare), kRate, lumi::audio::WavTiefe::Gleitkomma32)
                .geschrieben);
    liste << "\n\nFrage zum Stufenschritt (UF4 a, HT6 a): bei welchem Abstand unterscheidet man die Klangfarbe nicht mehr?\n";
    std::ofstream(ziel / "liste.txt", std::ios::binary) << liste.str();
    MESSAGE(liste.str());
}
