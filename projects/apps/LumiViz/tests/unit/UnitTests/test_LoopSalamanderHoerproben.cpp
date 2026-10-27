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
 *
 * 31d-8 (S157, SP1 a, Schritt 8.5): `LUMIVIZ_31D8_WAV=<Ordner>` mit `LUMIVIZ_SALAMANDER` legt die
 * Proben fuer u (Stoss je Saite) an 33, 60, 84 bei mf und ff, γ und δ an 24 und 27 ab (Salamander
 * hat nur kleine Terzen; γ wirkt nur mit δ, δ nur an Tasten mit einer Saite) — je Datei Salamander
 * und vier Werte innerhalb SQ2 a; Patrik waehlt nach Gehoer (8.6).
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
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
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

SalamanderTaste taste(int midi)
{
    for (const SalamanderTaste& t : salamanderTasten())
        if (t.midi == midi) return t;
    return SalamanderTaste{};
}

/// Eine Ebene von Salamander als Stereo (48 kHz).
std::vector<float> liesEbene(const std::filesystem::path& salamander, int midi, int ebene)
{
    const std::filesystem::path datei = salamanderDatei(salamander, taste(midi), ebene);
    const std::optional<lumi::audio::WavInhalt> wav = lumi::audio::liesWavDatei(datei);
    REQUIRE_MESSAGE(wav.has_value(), datei.string());
    REQUIRE(wav->rate == kRate);
    return wav->stereo;
}

/// Ein Ton des Modells, `halten` s gehalten, wie im Programm.
std::vector<float> rechne(const lumi::loop::Instrument& instrument, int midi, int stufe, double halten)
{
    lumi::loop::TonAngabe angabe;
    angabe.hoehe = midi;
    angabe.dauerMs = static_cast<int>(1000.0 * halten);
    angabe.anschlagStufe = stufe;
    return lumi::loop::rendereTon(instrument, angabe, kRate).daten;
}

/// Die Ebene von Salamander zur Stufe des Modells: ihr Pegel zu Beginn liegt gegen v16 so wie die Stufe
/// gegen 0 dB am Modell `modell`; `abstand` bekommt die Abweichung in dB.
int passendeEbene(const std::filesystem::path& salamander, const lumi::loop::Instrument& modell, int midi, int stufe,
                  double& abstand)
{
    const double modellAbstand = pegel(rechne(modell, midi, stufe, 1.0)) - pegel(rechne(modell, midi, 0, 1.0));
    const double laut = pegel(liesEbene(salamander, midi, kEbenen));
    int ebene = kEbenen;
    abstand = 1e9;
    for (int e = 1; e <= kEbenen; ++e)
    {
        const double d = std::abs(pegel(liesEbene(salamander, midi, e)) - laut - modellAbstand);
        if (d < abstand)
        {
            abstand = d;
            ebene = e;
        }
    }
    return ebene;
}

/// Wie lange Salamander an dieser Ebene klingt, bevor die Aufnahme ausblendet (Frames).
std::size_t framesVorBlende(const std::vector<float>& stereo)
{
    std::vector<double> summe(stereo.size() / 2);
    for (std::size_t i = 0; i < summe.size(); ++i) summe[i] = static_cast<double>(stereo[2 * i]) + stereo[2 * i + 1];
    return endeVorBlende(summe, kRate);
}

/// Eine Kurve, an jeder Taste derselbe Wert.
lumi::loop::Tastenkurve ueberall(double wert)
{
    return {{{21, wert}, {108, wert}}, {}};
}

std::string nummer(int nr)
{
    return (nr < 10 ? "0" : "") + std::to_string(nr);
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
    const auto liesEbene = [&](int midi, int ebene) { return ::liesEbene(salamander, midi, ebene); };

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
        double bester = 0.0;
        const int ebene = passendeEbene(salamander, nach, t.midi, t.stufe, bester);
        std::vector<float> sal = liesEbene(t.midi, ebene);
        const std::size_t frames = framesVorBlende(sal);
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

TEST_CASE("Loop Salamander 31d-8 Hoerproben u, gamma, delta (LUMIVIZ_31D8_WAV)")
{
    // 31d-8, Schritt 8.5 (SP1 a): u, γ und δ hat keine Quelle gemessen (SQ2 a: nur Grenzen) — Patrik waehlt sie
    // nach Gehoer. Je Datei zuerst Salamander, dann das Modell mit vier Werten der einen Groesse, alles andere die
    // Vorgabe; jede Fassung auf −23 LUFS, gemeinsam unter −1 dBFS; gehalten wie Salamander, hoechstens 12 s.
    const QString zielVariable = qEnvironmentVariable("LUMIVIZ_31D8_WAV");
    const QString salamanderVariable = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    if (zielVariable.isEmpty() || salamanderVariable.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D8_WAV=<Ordner> mit LUMIVIZ_SALAMANDER=<Ordner> legt die Hoerproben 31d-8 (u, gamma, delta) ab");
        return;
    }
    const std::filesystem::path ziel = zielVariable.toStdU16String();
    const std::filesystem::path salamander = salamanderVariable.toStdU16String();
    std::filesystem::create_directories(ziel);
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();

    const lumi::loop::HammerSaite basis = lumi::loop::hammerSaiteVorgabe();
    const lumi::loop::Instrument vorgabe = modelltest::fluegel(QStringLiteral("lib:steinway_d"));
    enum class Groesse
    {
        Stoss,
        Winkel,
        Ebenen
    };
    const struct
    {
        Groesse groesse;
        int midi;
        const char* name;
        const char* staerke;
        int stufe;
        std::array<double, 4> werte;
    } proben[] = {
        {Groesse::Stoss, 33, "a1", "mf", -12, {0.0, 0.15, 0.3, 0.5}},  {Groesse::Stoss, 33, "a1", "ff", 0, {0.0, 0.15, 0.3, 0.5}},
        {Groesse::Stoss, 60, "c4", "mf", -12, {0.0, 0.15, 0.3, 0.5}},  {Groesse::Stoss, 60, "c4", "ff", 0, {0.0, 0.15, 0.3, 0.5}},
        {Groesse::Stoss, 84, "c6", "mf", -12, {0.0, 0.15, 0.3, 0.5}},  {Groesse::Stoss, 84, "c6", "ff", 0, {0.0, 0.15, 0.3, 0.5}},
        // γ wirkt nur mit δ (S157 gemessen: an 24, 33, 60, 84 mit δ = 0 sind alle vier Fassungen Bit fuer Bit
        // gleich) — δ ist nur an Tasten mit einer Saite gesetzt, also γ dort, mit δ = 3 Cent (Mitte nach Tan 2017)
        {Groesse::Winkel, 24, "c1", "mf", -12, {0.0, 5.0, 10.0, 20.0}}, {Groesse::Winkel, 27, "ds1", "mf", -12, {0.0, 5.0, 10.0, 20.0}},
        {Groesse::Ebenen, 24, "c1", "mf", -12, {0.0, 2.0, 4.0, 6.0}},   {Groesse::Ebenen, 27, "ds1", "mf", -12, {0.0, 2.0, 4.0, 6.0}},
    };

    std::ostringstream liste;
    liste << std::fixed << std::setprecision(1) << "Hoerproben 31d-8 (SP1 a, Schritt 8.5), gleich laut auf " << kZielLufs
          << " LUFS integriert (EBU R128).\n"
          << "Salamander Grand Piano V3 (Yamaha C5), Alexander Holm 2011, CC BY 3.0 (http://creativecommons.org/licenses/by/3.0/),\n"
          << "Fassung 48 kHz/24 bit von archive.org; hier geschnitten und im Pegel angeglichen.\n"
          << "Modell: Hammer-Saite mit dem Boden des Steinway D (lib:steinway_d), wie im Programm; alles ausser der\n"
          << "einen Groesse ist die Vorgabe nach 31d-4. Gehalten wie Salamander vor der Blende, hoechstens 12 s.\n\n"
          << "Je Datei hintereinander, mit einer Sekunde Stille: Salamander | Modell Wert 1 | Wert 2 | Wert 3 | Wert 4.\n"
          << "u = ungleicher Stoss der Saiten im Chor (leise = laut); gamma = Winkel der zweiten Schwingungsebene\n"
          << "(delta dabei 3 Cent); delta = ihre Verstimmung (gamma dabei 10 Grad). gamma und delta wirken nur zusammen\n"
          << "und nur an Tasten mit einer Saite (A0-F#1); im Chor traegt die Verstimmung der Saiten die Schwebung.\n\n";
    int nr = 1;
    for (const auto& p : proben)
    {
        CAPTURE(p.midi);
        double abstand = 0.0;
        const int ebene = passendeEbene(salamander, vorgabe, p.midi, p.stufe, abstand);
        std::vector<float> sal = liesEbene(salamander, p.midi, ebene);
        const std::size_t frames = std::min(framesVorBlende(sal), static_cast<std::size_t>(12 * kRate));
        const double haltenS = static_cast<double>(frames) / kRate;
        std::vector<std::vector<float>> teile;
        teile.push_back(std::move(sal));
        for (const double w : p.werte)
        {
            lumi::loop::HammerSaite saite = basis;
            if (p.groesse == Groesse::Stoss)
            {
                saite.stossLeise = ueberall(w);
                saite.stossLaut = ueberall(w);
            }
            else if (p.groesse == Groesse::Winkel)
            {
                saite.polarisationWinkelGrad = ueberall(w);
                saite.polarisationCent = ueberall(3.0);
            }
            else
            {
                saite.polarisationWinkelGrad = ueberall(10.0);
                saite.polarisationCent = ueberall(w);
            }
            const lumi::loop::Instrument modell = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            teile.push_back(rechne(modell, p.midi, p.stufe, haltenS));
        }
        std::vector<std::vector<float>*> zeiger;
        for (std::vector<float>& t : teile)
        {
            richte(t, frames);
            zeiger.push_back(&t);
        }
        const double leiser = unterSpitze(zeiger);
        const char* groesse = p.groesse == Groesse::Stoss ? "u" : p.groesse == Groesse::Winkel ? "gamma" : "delta";
        const char* einheit = p.groesse == Groesse::Stoss ? "" : p.groesse == Groesse::Winkel ? " Grad" : " Cent";
        const std::string datei = nummer(nr) + "_" + groesse + "_" + p.name + "_" + p.staerke + ".wav";
        REQUIRE(lumi::audio::schreibeWav(QString::fromStdU16String((ziel / datei).u16string()), folge(teile), kRate,
                                         lumi::audio::WavTiefe::Gleitkomma32)
                    .geschrieben);
        liste << datei << ": " << groesse << " an " << p.name << " (MIDI " << p.midi << ") " << p.staerke << " (Stufe " << p.stufe
              << " dB; Salamander v" << ebene << ", auf " << abstand << " dB), je " << haltenS << " s; Modell mit " << groesse << " =";
        for (const double w : p.werte) liste << ' ' << std::setprecision(2) << w << einheit;
        liste << std::setprecision(1) << (leiser > 0.0 ? "; gemeinsam " + std::to_string(leiser).substr(0, 4) + " dB leiser" : std::string())
              << "\n   Frage: welcher Wert klingt Salamander am naechsten (1-4, \"zwischen 2 und 3\", oder \"kein Unterschied\")?\n";
        ++nr;
    }
    liste << "\nDie Wahl je Datei kommt nach Schritt 8.6 als Vorgabe in baueVorgabe() (Konzept §4.1, Teilplan 31d-8).\n";
    std::ofstream(ziel / "liste.txt", std::ios::binary) << liste.str();
    MESSAGE(liste.str());
}
