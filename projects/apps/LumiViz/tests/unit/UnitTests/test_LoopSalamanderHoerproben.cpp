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
 *
 * PV1 a (S164, ZZ1 a; gebaut S166): `LUMIVIZ_PV1_WAV=<Ordner>` mit `LUMIVIZ_SALAMANDER` legt je Taste 33, 60, 84 und
 * Ebene der Suche (v6, v10, v16) Salamander | Vorgabe | Suche 7.5.12 "ohne A4" ab (der Punkt aus
 * `proben/S165_suche2_probe_ohne_a4/ergebnis_1.txt`, nur Vorschlag fuer das Ohr), dazu in `liste.txt` je Taste die
 * Groessen des Ziels gegen beide Fassungen, nach Laenge, Nachklang und Schwebung — Patrik setzt u, γ, δ nach Gehoer.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "Hoerschwellen.hpp"
#include "ModellMessung.hpp"
#include "Modellton.hpp"
#include "Strukturmessung.hpp"
#include "Suche.hpp"
#include "Suchkurven.hpp"
#include "Zielsumme.hpp"

#include "audio/Lautheit.hpp"
#include "audio/WavDatei.hpp"
#include "audio/WavFormat.hpp"
#include "loop/HammerSaite.hpp"
#include "loop/Tonspeicher.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

const std::filesystem::path kStrukturDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_struktur.txt";
const std::filesystem::path kZielDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_ziel.txt";
const std::filesystem::path kHoerschwellenDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/hoerschwellen.txt";
/// Der Weg "ohne A4" der Suche 7.5.12 (S165): hoerbar getroffen 60 von 104 an den sechs Tasten (PV1 a).
const std::filesystem::path kSucheOhneA4 = "projects/apps/LumiViz/docs/modellierung/hammer-saite/proben/S165_suche2_probe_ohne_a4";

std::string zahl(double wert, int stellen)
{
    std::ostringstream s;
    s << std::fixed << std::setprecision(stellen) << wert;
    return s.str();
}

/// Der Stoss an einer Stufe wie im Rechenkern (`HammerTon.cpp`, US2 a): leise und laut an ihren Stufen, dazwischen linear.
double stossBei(const lumi::loop::HammerSaite& s, int midi, int stufe)
{
    const double zwischen = std::clamp(static_cast<double>(stufe - lumi::loop::kStossStufeLeise) /
                                           (lumi::loop::kStossStufeLaut - lumi::loop::kStossStufeLeise),
                                       0.0, 1.0);
    const double leise = std::clamp(lumi::loop::wertAn(s.stossLeise, midi, lumi::loop::KurvenSkala::Linear), 0.0, 1.0);
    const double laut = std::clamp(lumi::loop::wertAn(s.stossLaut, midi, lumi::loop::KurvenSkala::Linear), 0.0, 1.0);
    return leise + zwischen * (laut - leise);
}

/// Worauf eine Groesse des Ziels hoert: Laenge (Huelle vorwaerts), Nachklang (spaete Stufe, Knick) oder Schwebung.
const char* gehoerVon(const std::string& groesse)
{
    if (groesse.rfind("vorwaerts_t60", 0) == 0 || groesse.rfind("huelle_", 0) == 0) return "Laenge";
    if (groesse.rfind("t60_spaet", 0) == 0 || groesse.rfind("knick_tiefe", 0) == 0) return "Nachklang";
    if (groesse.rfind("schwebung", 0) == 0 || groesse.rfind("tiefe_db", 0) == 0) return "Schwebung";
    return "sonst";
}

/// Die Fragen des Hoertests PV1 (S166, Wunsch Patrik): je Klang auf 1–5, links und rechts die Pole. Sie zielen auf
/// das, was 8.6 und 31d-7 C brauchen — Klangfarbe und das hohle, sinusartige ("hoelele", Patrik S166), Laenge und
/// Nachklang (HW4 a), Schwebung (u, γ, δ).
QJsonArray hoertestFragen()
{
    const auto frage = [](const char* id, const char* text, const char* links, const char* rechts) {
        return QJsonObject{{QStringLiteral("id"), QString::fromUtf8(id)},
                           {QStringLiteral("text"), QString::fromUtf8(text)},
                           {QStringLiteral("links"), QString::fromUtf8(links)},
                           {QStringLiteral("rechts"), QString::fromUtf8(rechts)}};
    };
    return QJsonArray{frage("gesamt", "Klingt wie ein echter Flügel", "gar nicht", "ganz"),
                      frage("farbe", "Klangfarbe", "dumpf", "hell, farbig"),
                      frage("hohl", "Hohl, ein einzelner Sinus hörbar", "gar nicht", "stark"),
                      frage("laenge", "Länge: wie lange man den Ton hört", "kurz", "lang"),
                      frage("nachklang", "Nachklang: leiser Rest nach dem ersten Abklingen", "keiner", "deutlich"),
                      frage("schwebung", "Schwebung: Wabern, Pulsieren", "ruhig", "stark")};
}

/// Σ r² einer Taste aus dem Abschnitt "Je Taste und Groesse" einer Ergebnisdatei der Suche; ohne die Zeile −1.
double summeAusDatei(const std::string& text, int midi)
{
    const std::size_t abschnitt = text.find("## Je Taste und Groesse");
    if (abschnitt == std::string::npos) return -1.0;
    const std::string kopf = "\n" + std::to_string(midi) + " | ";
    const std::size_t zeile = text.find(kopf, abschnitt);
    if (zeile == std::string::npos) return -1.0;
    return std::stod(text.substr(zeile + kopf.size()));
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

TEST_CASE("Loop Salamander HU1 Diagnose (LUMIVIZ_HU1_WAV)")
{
    // HU1 a (S159): die Glieder des Rechenwegs einzeln ablegen, damit ein Skript daneben die Huellkurve vorwaerts
    // und die Teiltoene misst — je Taste die Kraft am Steg (ohne Boden), die Antwort des Bodens und der Ton wie im
    // Programm, in Fassungen der Hammer-Saite: Vorgabe, vor 31d, Late decay wie Salamander (Teilton 1, sonst 2,
    // sonst 3), dazu u = 0,25, dazu gamma 10 Grad / delta 3 Cent, dazu High decay wie Salamander. Stufe -12 (mf).
    const QString zielVariable = qEnvironmentVariable("LUMIVIZ_HU1_WAV");
    if (zielVariable.isEmpty())
    {
        MESSAGE("LUMIVIZ_HU1_WAV=<Ordner> legt Kraft, Bodenantwort und Ton je Taste in Fassungen ab (HU1 a)");
        return;
    }
    const std::filesystem::path ziel = zielVariable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::shared_ptr<const lumi::loop::BodenImKlang> boden = lumi::loop::bodenVon(QStringLiteral("lib:steinway_d"));
    REQUIRE_MESSAGE(boden, "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();
    const std::optional<SalamanderWerte> gelesen = liesWerte("projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_c5.txt");
    REQUIRE(gelesen.has_value());
    const std::vector<TastenWerte> salamander = tastenWerteAus(*gelesen);
    const auto salamanderAn = [&](int midi) -> const TastenWerte* {
        for (const TastenWerte& w : salamander)
            if (w.midi == midi) return &w;
        return nullptr;
    };
    const auto spaetVon = [&](int midi, double sonst) {
        const TastenWerte* w = salamanderAn(midi);
        if (w == nullptr) return sonst;
        if (w->t60Spaet.anzahl > 0) return w->t60Spaet.wert;
        for (const Wert& v : w->t60SpaetK)
            if (v.anzahl > 0) return v.wert;
        return sonst;
    };
    const auto hochVon = [&](int midi, double sonst) {
        const TastenWerte* w = salamanderAn(midi);
        return w != nullptr && w->highDecay.anzahl > 0 ? w->highDecay.wert : sonst;
    };
    const auto schreibe = [&](const std::string& name, std::span<const float> stereo) {
        REQUIRE(lumi::audio::schreibeWav(QString::fromStdU16String((ziel / name).u16string()), stereo, kRate,
                                         lumi::audio::WavTiefe::Gleitkomma32)
                    .geschrieben);
    };
    const lumi::loop::HammerSaite vorgabe = lumi::loop::hammerSaiteVorgabe();
    constexpr int kStufe = -12;
    std::ostringstream liste;
    liste << std::fixed << std::setprecision(3) << "HU1 a: je Taste k<midi>_<fassung>.wav (Ton wie im Programm, Stereo, Pa in Einheiten von "
          << lumi::loop::kSchalldruckBezug << " Pa), k<midi>_<fassung>_kraft.wav (Kraft am Steg ohne Boden, in Einheiten von "
          << lumi::loop::kHammerKraftBezug << " N, beide Kanaele gleich), k<midi>_boden.wav (Antwort des Bodens, Pa/N). Stufe " << kStufe
          << ".\n";
    for (const int midi : {21, 36, 60, 84, 108})
    {
        CAPTURE(midi);
        const double haltenS = midi <= 36 ? 18.0 : midi <= 60 ? 12.0 : midi <= 84 ? 6.0 : 3.0;
        const auto frames = static_cast<std::size_t>(haltenS * kRate);
        const double decayVorgabe = lumi::loop::wertAn(vorgabe.abklingS, midi, lumi::loop::KurvenSkala::Logarithmisch);
        const double spaetS = spaetVon(midi, 7.0 * decayVorgabe);
        const double hochSal = hochVon(midi, 0.0);
        const auto ueberallWert = [](double wert) { return lumi::loop::Tastenkurve{{{21, wert}, {108, wert}}, {}}; };
        std::vector<std::pair<std::string, lumi::loop::HammerSaite>> fassungen;
        fassungen.emplace_back("vorgabe", vorgabe);
        fassungen.emplace_back("vor31d", vorgabeVor31d());
        lumi::loop::HammerSaite spaet = vorgabe;
        spaet.abklingSpaetS.abweichungen = {{midi, spaetS}};
        fassungen.emplace_back("spaet_sal", spaet);
        lumi::loop::HammerSaite stoss = spaet;
        stoss.stossLeise = ueberallWert(0.25);
        stoss.stossLaut = ueberallWert(0.25);
        fassungen.emplace_back("spaet_sal_u25", stoss);
        lumi::loop::HammerSaite pol = spaet;
        pol.polarisationWinkelGrad = ueberallWert(10.0);
        pol.polarisationCent = ueberallWert(3.0);
        fassungen.emplace_back("spaet_sal_pol", pol);
        if (hochSal > 0.0)
        {
            lumi::loop::HammerSaite hoch = spaet;
            hoch.abklingHoch.abweichungen = {{midi, hochSal}};
            fassungen.emplace_back("spaet_sal_hoch", hoch);
        }
        // die Antwort des Bodens an dieser Taste
        const std::shared_ptr<const lumi::loop::Klang> antwort = lumi::loop::bodenAntwortFuer(*boden, midi, kRate);
        REQUIRE(antwort);
        schreibe("k" + std::to_string(midi) + "_boden.wav", antwort->daten);
        liste << "MIDI " << midi << ": gehalten " << haltenS << " s; Vorgabe decay " << decayVorgabe << " s, lateDecay "
              << lumi::loop::wertAn(vorgabe.abklingSpaetS, midi, lumi::loop::KurvenSkala::Logarithmisch) << " s, highDecay "
              << lumi::loop::wertAn(vorgabe.abklingHoch, midi, lumi::loop::KurvenSkala::Logarithmisch) << ", detune "
              << lumi::loop::wertAn(vorgabe.verstimmungCent, midi, lumi::loop::KurvenSkala::Linear) << " Cent, Saiten "
              << lumi::loop::chorAn(vorgabe, midi) << "; Salamander lateDecay " << spaetS << " s, highDecay " << hochSal
              << "; Antwort des Bodens " << antwort->frames() << " Frames\n";
        for (const auto& [name, saite] : fassungen)
        {
            // die Kraft am Steg (ohne Boden)
            lumi::loop::HammerAnschlag a;
            a.taste = midi;
            a.hz = tastenFrequenz(midi);
            a.stufe = kStufe;
            a.halten = static_cast<std::int64_t>(frames);
            a.gedaempft = midi < lumi::loop::kUngedaempftAbVorgabe;
            a.rate = kRate;
            std::vector<float> mono;
            lumi::loop::rechneHammerTon(saite, a, mono, static_cast<std::int64_t>(frames));
            std::vector<float> kraft(2 * mono.size());
            for (std::size_t i = 0; i < mono.size(); ++i) kraft[2 * i] = kraft[2 * i + 1] = mono[i];
            schreibe("k" + std::to_string(midi) + "_" + name + "_kraft.wav", kraft);
            // der Ton wie im Programm, auf die gehaltene Laenge geschnitten
            const lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            std::vector<float> ton = rechne(instrument, midi, kStufe, haltenS);
            ton.resize(2 * std::min(frames, ton.size() / 2));
            schreibe("k" + std::to_string(midi) + "_" + name + ".wav", ton);
        }
    }
    std::ofstream(ziel / "liste.txt", std::ios::binary) << liste.str();
    MESSAGE(liste.str());
}

TEST_CASE("Loop Salamander PV1 Hoerproben Vorgabe und Suche (LUMIVIZ_PV1_WAV)")
{
    // PV1 a (S164): die Probe 7.5.12 ist verfehlt, ZZ1 a gilt — Patrik setzt u, γ und δ nach Gehoer an 33, 60, 84. Je
    // Taste und Ebene der Suche (v6, v10, v16 an den Stufen −20, −12, 0; SG1 a, V1 a) hintereinander Salamander, die
    // Vorgabe und der Punkt der Suche "ohne A4" (Vorschlag fuer das Ohr, kein gesetzter Wert), wie in der Suche im
    // Fluegel `fluegelMit`; jede Fassung auf −23 LUFS, gemeinsam unter −1 dBFS, gehalten wie Salamander vor der Blende,
    // hoechstens 30 s. Die Liste stellt je Taste die Groessen des Ziels gegen beide Fassungen, gemessen wie in der Suche;
    // Σ r² der Suche muss die Ergebnisdatei wiedergeben, sonst ist der Raum falsch gebaut.
    const QString zielVariable = qEnvironmentVariable("LUMIVIZ_PV1_WAV");
    const QString salamanderVariable = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    if (zielVariable.isEmpty() || salamanderVariable.isEmpty())
    {
        MESSAGE("LUMIVIZ_PV1_WAV=<Ordner> mit LUMIVIZ_SALAMANDER=<Ordner> legt die Hoerproben PV1 a (Vorgabe und Suche ohne A4) ab");
        return;
    }
    const std::filesystem::path ziel = zielVariable.toStdU16String();
    const std::filesystem::path salamander = salamanderVariable.toStdU16String();
    std::filesystem::create_directories(ziel);
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();

    // der Punkt der Suche und ihr Raum; ergebnis_1.txt traegt die Zeile `optionen` nicht (sie steht in
    // ergebnis_bester.txt, dort ist der beste je Taste Startpunkt 1 — derselbe Punkt)
    const std::filesystem::path ergebnis = kSucheOhneA4 / "ergebnis_1.txt";
    const std::filesystem::path bester = kSucheOhneA4 / "ergebnis_bester.txt";
    const std::vector<double> x = modelltest::liesX(ergebnis);
    REQUIRE_MESSAGE(!x.empty(), ergebnis.string());
    std::optional<modelltest::SuchOptionen> optionen = modelltest::liesSuchOptionen(ergebnis);
    if (!optionen)
    {
        REQUIRE(modelltest::liesX(bester) == x);
        optionen = modelltest::liesSuchOptionen(bester);
    }
    REQUIRE(optionen.has_value());
    const std::optional<Strukturen> strukturen = liesStruktur(kStrukturDatei);
    const std::optional<Ziele> ziele = liesZiele(kZielDatei);
    const HoerschwellenGelesen schwellen = liesHoerschwellen(kHoerschwellenDatei);
    REQUIRE(strukturen.has_value());
    REQUIRE(ziele.has_value());
    REQUIRE_MESSAGE(schwellen.werte.has_value(), schwellen.fehler);
    const std::vector<int> suchTasten{33, 48, 60, 66, 81, 84};  // die Tasten der Probe (ZF8 a), wie im Lauf
    modelltest::Suchraum raum = modelltest::suchraum(lumi::loop::hammerSaiteVorgabe(), suchTasten, *strukturen, *optionen);
    (void)modelltest::setzeDetuneAus(ergebnis, raum);
    (void)modelltest::setzeFestAus(ergebnis, raum, "delta");
    REQUIRE(x.size() == raum.zahl());
    std::ostringstream gelesen;
    gelesen << std::ifstream(ergebnis, std::ios::binary).rdbuf();
    const std::string text = gelesen.str();
    REQUIRE_MESSAGE(text.find(modelltest::kurvenText(raum, x)) != std::string::npos, "die Kurven weichen vom Abschnitt Kurven der Datei ab");

    const lumi::loop::HammerSaite& vorgabeSaite = lumi::loop::hammerSaiteVorgabe();
    const lumi::loop::HammerSaite sucheSaite = modelltest::saiteAus(raum, x);
    const lumi::loop::Instrument vorgabe = modelltest::fluegelMit(vorgabeSaite);
    const lumi::loop::Instrument suche = modelltest::fluegelMit(sucheSaite);

    std::ostringstream liste;
    liste << "Hoerproben PV1 a (31d-8, Schritt 8.5 nach ZZ1 a), gleich laut auf " << zahl(kZielLufs, 1) << " LUFS integriert (EBU R128).\n"
          << "Salamander Grand Piano V3 (Yamaha C5), Alexander Holm 2011, CC BY 3.0 (http://creativecommons.org/licenses/by/3.0/),\n"
          << "Fassung 48 kHz/24 bit von archive.org; hier geschnitten und im Pegel angeglichen.\n"
          << "Modell: Hammer-Saite mit dem Boden des Steinway D (lib:steinway_d), ungedaempft ab 89, wie in der Suche.\n\n"
          << "Je Datei hintereinander, mit einer Sekunde Stille:\n"
          << "  1 Salamander | 2 Vorgabe (31d-4, heute im Programm) | 3 Suche 7.5.12 ohne A4 (nur Vorschlag fuer das Ohr)\n"
          << "Punkt: Zeile x aus " << ergebnis.generic_string() << "; " << modelltest::optionenZeile(*optionen) << ".\n"
          << "Je Taste drei Staerken an den Ebenen der Suche: p = v6 (Stufe -20 dB), mf = v10 (-12), ff = v16 (0).\n"
          << "Jede Fassung fuer sich gleich laut: die Staerke hoert man an der Klangfarbe, nicht am Pegel.\n"
          << "Gehalten wie Salamander vor der Blende der Aufnahme, hoechstens 30 s.\n\n"
          << "Je Taste die Groessen des Ziels (Salamander) gegen beide Fassungen, gemessen wie in der Suche (alle drei Ebenen\n"
          << "zusammen); (h) = hoerbar getroffen nach HW2 a. Laenge = Huelle vorwaerts (wie lange man den Ton hoert),\n"
          << "Nachklang = spaete Stufe und Knick, Schwebung = Schwebung und Einbrueche je Teilton (_1 bis _3 = Teilton).\n";

    const struct
    {
        int midi;
        const char* name;
    } tasten[] = {{33, "a1"}, {60, "c4"}, {84, "c6"}};
    const struct
    {
        int ebene;
        const char* staerke;
    } ebenen[] = {{modelltest::kSuchEbenen[0], "p"}, {modelltest::kSuchEbenen[1], "mf"}, {modelltest::kSuchEbenen[2], "ff"}};
    // fuer den Hoertest (tools/hoertest): jede Fassung einzeln unter einzeln/, dazu hoertest.json mit den Paaren
    // Salamander–Vorgabe, Vorgabe–Suche, Salamander–Suche je Taste und Staerke und je Taste einem Kontrollpaar (zweimal
    // derselbe Klang)
    std::filesystem::create_directories(ziel / "einzeln");
    QJsonArray klaenge;
    QJsonArray paare;
    const char* fassungen[] = {"salamander", "vorgabe", "suche"};
    int nr = 1;
    for (const auto& t : tasten)
    {
        CAPTURE(t.midi);
        const TastenSumme v = vergleiche(t.midi, ziele->at(t.midi), modelltest::modellAn(vorgabeSaite, t.midi, strukturen->at(t.midi)));
        const TastenSumme s = vergleiche(t.midi, ziele->at(t.midi), modelltest::modellAn(sucheSaite, t.midi, strukturen->at(t.midi)));
        CHECK(s.summe == doctest::Approx(summeAusDatei(text, t.midi)).epsilon(2e-4));
        REQUIRE(v.groessen.size() == s.groessen.size());

        liste << "\n== " << t.name << " (MIDI " << t.midi << ", " << lumi::loop::chorAn(vorgabeSaite, t.midi) << " Saiten) ==\n"
              << "Kurven an der Taste, Vorgabe -> Suche:";
        for (const modelltest::SuchKurve& k : raum.kurven)
            liste << ' ' << k.name << ' ' << zahl(lumi::loop::wertAn(vorgabeSaite.*k.feld, t.midi, k.skala()), 3) << " -> "
                  << zahl(lumi::loop::wertAn(sucheSaite.*k.feld, t.midi, k.skala()), 3) << ';';
        liste << "\nSumme r^2 Vorgabe " << zahl(v.summe, 1) << ", Suche " << zahl(s.summe, 1) << '\n';
        for (const char* gehoer : {"Laenge", "Nachklang", "Schwebung", "sonst"})
        {
            int zahlGroessen = 0;
            int trifftV = 0;
            int trifftS = 0;
            std::ostringstream zeilen;
            for (std::size_t i = 0; i < v.groessen.size(); ++i)
            {
                const Abweichung& a = v.groessen[i];
                const Abweichung& b = s.groessen[i];
                REQUIRE(a.groesse == b.groesse);
                if (std::string(gehoerVon(a.groesse)) != gehoer) continue;
                const bool hv = hoerbarGetroffen(*schwellen.werte, a);
                const bool hs = hoerbarGetroffen(*schwellen.werte, b);
                ++zahlGroessen;
                trifftV += hv ? 1 : 0;
                trifftS += hs ? 1 : 0;
                const char* einheit = a.skala == Skala::Faktor ? " s" : a.skala == Skala::Cent ? " Cent" : " dB";
                const auto wert = [&](const Abweichung& w, bool h) {
                    return w.fehlt ? std::string("fehlt") : zahl(w.modell, 2) + einheit + (h ? " (h)" : "");
                };
                zeilen << "    " << std::left << std::setw(18) << a.groesse << std::right << " Salamander " << zahl(a.ziel, 2) << einheit
                       << " | Vorgabe " << wert(a, hv) << " | Suche " << wert(b, hs) << '\n';
            }
            if (zahlGroessen == 0) continue;
            liste << "  " << gehoer << ": hoerbar getroffen Vorgabe " << trifftV << " von " << zahlGroessen << ", Suche " << trifftS << " von "
                  << zahlGroessen << '\n'
                  << zeilen.str();
        }

        for (const auto& e : ebenen)
        {
            const int stufe = modelltest::stufeVonEbene(e.ebene);
            std::vector<float> sal = liesEbene(salamander, t.midi, e.ebene);
            const std::size_t frames = std::min(framesVorBlende(sal), static_cast<std::size_t>(30 * kRate));
            const double haltenS = static_cast<double>(frames) / kRate;
            std::vector<float> tv = rechne(vorgabe, t.midi, stufe, haltenS);
            std::vector<float> ts = rechne(suche, t.midi, stufe, haltenS);
            const double gs = richte(sal, frames);
            const double gv = richte(tv, frames);
            const double gx = richte(ts, frames);
            const double leiser = unterSpitze({&sal, &tv, &ts});
            const std::string datei = nummer(nr) + "_" + t.name + "_" + e.staerke + ".wav";
            REQUIRE(lumi::audio::schreibeWav(QString::fromStdU16String((ziel / datei).u16string()), folge({sal, tv, ts}), kRate,
                                             lumi::audio::WavTiefe::Gleitkomma32)
                        .geschrieben);
            liste << datei << ": " << t.name << ' ' << e.staerke << " (Salamander v" << e.ebene << ", Stufe " << stufe << " dB), je "
                  << zahl(haltenS, 1) << " s; u an der Stufe Vorgabe " << zahl(stossBei(vorgabeSaite, t.midi, stufe), 3) << ", Suche "
                  << zahl(stossBei(sucheSaite, t.midi, stufe), 3) << "; Verstaerkung " << zahl(gs, 1) << " / " << zahl(gv, 1) << " / "
                  << zahl(gx, 1) << " dB" << (leiser > 0.0 ? "; gemeinsam " + zahl(leiser, 1) + " dB leiser" : std::string()) << '\n';
            const std::string stamm = "k" + std::to_string(t.midi) + "_" + e.staerke;
            const std::vector<float>* teile[] = {&sal, &tv, &ts};
            for (int f = 0; f < 3; ++f)
            {
                const std::string einzeln = "einzeln/" + stamm + "_" + fassungen[f] + ".wav";
                REQUIRE(lumi::audio::schreibeWav(QString::fromStdU16String((ziel / einzeln).u16string()), *teile[f], kRate,
                                                 lumi::audio::WavTiefe::Gleitkomma32)
                            .geschrieben);
                klaenge.append(QJsonObject{{QStringLiteral("id"), QString::fromStdString(stamm + "_" + fassungen[f])},
                                           {QStringLiteral("datei"), QString::fromStdString(einzeln)},
                                           {QStringLiteral("taste"), t.midi},
                                           {QStringLiteral("name"), QString::fromUtf8(t.name)},
                                           {QStringLiteral("staerke"), QString::fromUtf8(e.staerke)},
                                           {QStringLiteral("fassung"), QString::fromUtf8(fassungen[f])},
                                           {QStringLiteral("sekunden"), haltenS}});
            }
            const auto paar = [&](int a, int b, bool kontrolle) {
                paare.append(QJsonObject{{QStringLiteral("id"), QString::fromStdString(stamm + "_" + fassungen[a] + "_" + fassungen[b])},
                                              {QStringLiteral("a"), QString::fromStdString(stamm + "_" + fassungen[a])},
                                              {QStringLiteral("b"), QString::fromStdString(stamm + "_" + fassungen[b])},
                                              {QStringLiteral("kontrolle"), kontrolle}});
            };
            paar(0, 1, false);
            paar(1, 2, false);
            paar(0, 2, false);
            if (std::string(e.staerke) == "mf")
            {
                const int gleich = t.midi == 33 ? 1 : t.midi == 60 ? 2 : 0;  // Kontrolle an mf: A1 Vorgabe, C4 Suche, C6 Salamander
                paar(gleich, gleich, true);
            }
            ++nr;
        }
        liste << "  Frage: je Datei Laenge, Nachklang, Schwebung — ist 2 oder 3 naeher an 1? Wo 3 naeher ist: die Schwebung der\n"
              << "  Suche (u, gamma, delta) als Wert nehmen, oder zu stark / zu schwach / metallisch / passt?\n";
    }
    liste << "\nDie Wahl je Taste kommt in Schritt 8.6 als Stuetzpunkte von u, gamma, delta in baueVorgabe() (Konzept §4.1,\n"
          << "Teilplan 31d-8, PV1 a); zwischen 33, 60, 84 interpoliert, innerhalb SQ2 a.\n"
          << "\nBlind vergleichen: python -X utf8 -I tools/hoertest/hoertest.py " << ziel.generic_string()
          << " (einzeln/ und hoertest.json; die Antworten landen in antworten.jsonl daneben).\n";
    std::ofstream(ziel / "liste.txt", std::ios::binary) << liste.str();
    QFile test(QString::fromStdU16String((ziel / "hoertest.json").u16string()));
    REQUIRE(test.open(QIODevice::WriteOnly | QIODevice::Truncate));
    test.write(QJsonDocument(QJsonObject{{QStringLiteral("titel"), QStringLiteral("PV1 a: Salamander, Vorgabe, Suche ohne A4")},
                                         {QStringLiteral("fragen"), hoertestFragen()},
                                         {QStringLiteral("klaenge"), klaenge},
                                         {QStringLiteral("vergleiche"), paare}})
                   .toJson());
    test.close();
    CHECK(paare.size() == 30);
    MESSAGE(liste.str());
}
