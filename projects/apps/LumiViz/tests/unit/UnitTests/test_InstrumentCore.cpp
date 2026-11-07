/**
 ****************************************************************************************
 * @file   test_InstrumentCore.cpp
 * @brief  InstrumentCore, Stufe 34c: die Bibliothek der Rechenkerne gegen ihre Schnittstellen
 *         (docs/audio/Instrumente_IC_Konzept.md KN1 a, KN2 a, IK2-3 a, IK2-4 a, IK2-9, IK2-10 a, IK2-11 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine.
 *
 * - Fft: pocketfft gegen die direkte DFT (Pruefumsetzung aus IC), Name der Fassung.
 * - Eigenzerlegung: die IC-Fassung meldet Entartung und rechnet bis 32 × 32.
 * - Anschlagstufe, SHA-256 (Pruefwerte aus FIPS 180-2 Anhang B), Pool (jede Aufgabe einmal, Ausnahme).
 * - Kurve: Bit fuer Bit wie `Tastenkurve` des Fluegels (34d stuetzt sich darauf), Grenzen, Abweichung.
 * - Modal: eine Mode gegen die geschlossene Impulsantwort, Nachklang.
 * - Tonauftrag: Spurwerte, Pruefung.
 * - Parametertafel: Pruefung der Tafel, Vorgaben, Richten mit Fehlern, Abdruck der gestimmten Werte, ungemessen.
 * - Fabrik und `normiere` an einem Pruefkern (gedaempfter Sinus).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Anschlag.hpp"
#include "InstrumentCore/Eigenzerlegung.hpp"
#include "InstrumentCore/Fft.hpp"
#include "InstrumentCore/Kurve.hpp"
#include "InstrumentCore/Modal.hpp"
#include "InstrumentCore/Parametertafel.hpp"
#include "InstrumentCore/Pool.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentCore/Tonauftrag.hpp"
#include "InstrumentCore/Tonkern.hpp"

#include "loop/HammerSaite.hpp"  // Tastenkurve: der Vergleich Bit fuer Bit

#include <algorithm>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdint>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>

using namespace lumi::kern;

namespace
{

/// Zahlen in [−1, 1), auf jedem Rechner dieselben (SplitMix64).
struct Zufall
{
    std::uint64_t samen;
    double operator()()
    {
        samen += 0x9E3779B97F4A7C15ull;
        std::uint64_t z = samen;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        z ^= z >> 31;
        return static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0;
    }
};

// --- der Pruefkern: ein gedaempfter Sinus je Note --------------------------------------------------------

/// Roh (vor dem Bezug) hat der Ton bei Stufe 0 die Amplitude 2 — `normiere` muss den Bezug 4 finden.
constexpr double kRohAmplitude = 2.0;

class Sinuskern final : public Tonkern
{
public:
    explicit Sinuskern(const Werte& w) : m_t60(w.zahl("t60")), m_bezug(w.zahl("outputReference")) {}

    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override
    {
        return std::min<Frames>(hoechstens, std::llround(m_t60 * auftrag.rate));
    }

    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere) const override
    {
        ton.assign(static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames)), 0.0f);
        const Spur* expression = findeSpur(auftrag, spur::kExpression);
        std::vector<double> huelle(ton.size(), 0.0);
        for (const KettenNote& n : auftrag.noten)
            for (Frames f = n.beginn; f < auftrag.frames; ++f)
            {
                const double t = static_cast<double>(f - n.beginn) / auftrag.rate;
                const double e = expression != nullptr ? spurWert(*expression, f, 1.0) : 1.0;
                const double a = kRohAmplitude * anschlagFaktor(auftrag.stufe) * e * std::exp(-kLn1000 * t / m_t60);
                huelle[static_cast<std::size_t>(f)] += a;
                ton[static_cast<std::size_t>(f)] += static_cast<float>(a * std::sin(2.0 * std::numbers::pi * n.hz * t) / m_bezug);
            }
        if (innere != nullptr)
            for (const std::string& g : innere->gewuenscht)
                if (g == "envelope") innere->ergebnis.push_back(InnereGroesse{"envelope", "", huelle});
    }

private:
    double m_t60;
    double m_bezug;
};

ParameterAngabe zahlAngabe(std::string name, std::string gruppe, double von, double bis, double vorgabe, Belegkraft b = Belegkraft::Quelle)
{
    ParameterAngabe a;
    a.name = std::move(name);
    a.titel = a.name;
    a.gruppe = std::move(gruppe);
    a.art = WertArt::Zahl;
    a.darstellung = Darstellung::Regler;
    a.von = von;
    a.bis = bis;
    a.vorgabe = vorgabe;
    a.belegkraft = b;
    a.herkunft = "test";
    return a;
}

Parametertafel pruefTafel()
{
    Parametertafel t;
    t.kern = "testSine";
    t.titel = "Test sine";
    t.noteMin = 36;
    t.noteMax = 96;
    t.gruppen = {"Tone", "Voices", "Output"};

    t.werte.push_back(zahlAngabe("t60", "Tone", 0.05, 10.0, 1.0));
    t.werte.back().einheit = "s";

    ParameterAngabe form;
    form.name = "waveform";
    form.gruppe = "Tone";
    form.art = WertArt::Wahl;
    form.darstellung = Darstellung::Auswahl;
    form.wahlen = {"sine", "square"};
    form.vorgabe = std::string("sine");
    form.klangbestimmend = false;
    t.werte.push_back(form);

    ParameterAngabe hell;
    hell.name = "bright";
    hell.gruppe = "Tone";
    hell.art = WertArt::Schalter;
    hell.darstellung = Darstellung::Haken;
    hell.vorgabe = false;
    hell.klangbestimmend = false;
    t.werte.push_back(hell);

    ParameterAngabe gewicht;
    gewicht.name = "weight";
    gewicht.gruppe = "Tone";
    gewicht.art = WertArt::Kurve;
    gewicht.darstellung = Darstellung::Kurve;
    gewicht.skala = Skala::Logarithmisch;
    gewicht.von = 0.1;
    gewicht.bis = 10.0;
    gewicht.vorgabe = kurveAusPunkten({{36, 1.0}, {96, 2.0}}, Belegkraft::Quelle, "test p. 1");
    t.werte.push_back(gewicht);

    ParameterAngabe chor;
    chor.name = "chorus";
    chor.gruppe = "Voices";
    chor.art = WertArt::Liste;
    chor.darstellung = Darstellung::Dialog;
    chor.felder.push_back(zahlAngabe("detune", "", -50.0, 50.0, 0.0));
    chor.eintraegeMin = 1;
    chor.eintraegeMax = 3;
    Werte stimme;
    stimme.setze("detune", 0.0);
    chor.vorgabe = Liste{{stimme}};
    chor.klangbestimmend = false;
    t.werte.push_back(chor);

    ParameterAngabe laenge;
    laenge.name = "lengthTuned";
    laenge.gruppe = "Tone";
    laenge.art = WertArt::Kurve;
    laenge.darstellung = Darstellung::Kurve;
    laenge.von = 0.5;
    laenge.bis = 2.0;
    laenge.vorgabe = kurveKonstant(1.0, Belegkraft::Rechnung, "tuned", 36, 96);
    laenge.gestimmt = true;
    laenge.abhaengigVon = {"t60"};
    t.werte.push_back(laenge);

    ParameterAngabe bezug = zahlAngabe("outputReference", "Output", 1e-6, 1e6, 1.0, Belegkraft::Rechnung);
    bezug.klangbestimmend = false;
    t.werte.push_back(bezug);
    return t;
}

KernTyp pruefTyp()
{
    KernTyp typ;
    typ.tafel = pruefTafel();
    typ.auskunft.spuren = {std::string(spur::kExpression)};
    typ.auskunft.innere = {"envelope"};
    typ.erzeuge = [](const Werte& w) { return std::make_shared<const Sinuskern>(w); };
    return typ;
}

}  // namespace

TEST_CASE("InstrumentCore 34c Fft gegen die direkte DFT")
{
    const std::unique_ptr<Fft> direkt = direkteDft();
    CHECK(direkt->name() == "direct");
    CHECK(fft().name() == "pocketfft 4d865cb");
    REQUIRE(fftUmsetzungen().size() == 1);
    for (const std::size_t n : {1u, 2u, 7u, 8u, 15u, 16u, 64u, 100u})
    {
        CAPTURE(n);
        Zufall z{n};
        std::vector<double> x(n);
        for (double& v : x) v = z();
        std::vector<std::complex<double>> a(n / 2 + 1), b(n / 2 + 1);
        fft().vorwaerts(x, a);
        direkt->vorwaerts(x, b);
        for (std::size_t k = 0; k < a.size(); ++k) CHECK(std::abs(a[k] - b[k]) < 1e-12 * static_cast<double>(n));
        std::vector<double> zurueck(n);
        direkt->rueckwaerts(b, zurueck);
        for (std::size_t t = 0; t < n; ++t) CHECK(std::abs(zurueck[t] - x[t]) < 1e-12);
    }
    // ein zu kurzes Spektrum ist ein Fehler, nicht ein stiller Schreibzugriff
    std::vector<double> x(8, 1.0);
    std::vector<std::complex<double>> kurz(3);
    CHECK_THROWS_AS(fft().vorwaerts(x, kurz), std::invalid_argument);
    CHECK_THROWS_AS(direkt->vorwaerts(x, kurz), std::invalid_argument);
}

TEST_CASE("InstrumentCore 34c Eigenzerlegung meldet Entartung, bis 32")
{
    CHECK(kKleineMatrixMax == 32);
    // diagonal mit zwei gleichen Werten: entartet, Abstand 0
    KleineMatrix d(3);
    d(0, 0) = 1.0;
    d(1, 1) = 1.0;
    d(2, 2) = 2.0;
    const Eigenzerlegung zd = zerlegeKlein(d);
    CHECK(zd.entartet);
    CHECK(zd.kleinsterAbstand == 0.0);
    // eine allgemeine 20 × 20 (ueber der alten Grenze 16): A·v = λ·v
    const int n = 20;
    KleineMatrix a(n);
    Zufall z{2026};
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) a(i, j) = std::complex<double>(z(), z());
    const Eigenzerlegung e = zerlegeKlein(a);
    REQUIRE(e.werte.size() == static_cast<std::size_t>(n));
    CHECK(e.vollstaendig);
    CHECK_FALSE(e.entartet);
    double groesse = 0.0;
    for (const auto& w : a.werte) groesse += std::norm(w);
    groesse = std::sqrt(groesse);
    double rest = 0.0;
    for (int m = 0; m < n; ++m)
        for (int i = 0; i < n; ++i)
        {
            std::complex<double> av = 0.0;
            for (int j = 0; j < n; ++j) av += a(i, j) * e.rechts(j, m);
            rest = std::max(rest, std::abs(av - e.werte[static_cast<std::size_t>(m)] * e.rechts(i, m)));
        }
    CHECK(rest < 1e-10 * groesse);
}

TEST_CASE("InstrumentCore 34c Anschlagstufe")
{
    CHECK(anschlagStufe(1.0) == 0);
    CHECK(anschlagStufe(0.5) == -6);
    CHECK(anschlagStufe(0.0) == kAnschlagStufeMin);
    CHECK(anschlagStufe(1e-4) == kAnschlagStufeMin);
    CHECK(anschlagStufe(2.0) == 0);
    CHECK(anschlagFaktor(0) == 1.0);
    CHECK(anschlagFaktor(-20) == doctest::Approx(0.1).epsilon(1e-15));
    CHECK(anschlagFaktor(-100) == anschlagFaktor(kAnschlagStufeMin));
    for (const double laut : {0.9, 0.31, 0.07})
        CHECK(anschlagRest(laut, anschlagStufe(laut)) * anschlagFaktor(anschlagStufe(laut)) == doctest::Approx(laut).epsilon(1e-14));
}

TEST_CASE("InstrumentCore 34c SHA-256")
{
    // FIPS 180-2 Anhang B.1 und B.2, dazu die leere Eingabe
    CHECK(sha256(std::string_view("abc")) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(sha256(std::string_view("")) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    const std::string_view b2 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    CHECK(sha256(b2) == "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    // stueckweise wie am Stueck, ueber Blockgrenzen
    std::string lang(1000, 'x');
    for (std::size_t i = 0; i < lang.size(); ++i) lang[i] = static_cast<char>('a' + i % 26);
    Sha256 stueck;
    for (std::size_t ab = 0; ab < lang.size(); ab += 37) stueck.dazu(std::string_view(lang).substr(ab, 37));
    CHECK(stueck.fertig() == sha256(std::string_view(lang)));
    // der Abdruck eines Tons: die Bytes der floats in Little-Endian
    const std::vector<float> ton{1.0f, -0.5f};
    const std::uint8_t bytes[] = {0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0xbf};
    CHECK(abdruck(ton) == sha256(std::span<const std::uint8_t>(bytes, 8)));
}

TEST_CASE("InstrumentCore 34c Pool")
{
    CHECK(poolThreads() >= 1);
    CHECK(poolThreads() <= kPoolHoechstens);
    for (const unsigned threads : {0u, 1u, 3u, 16u})
    {
        CAPTURE(threads);
        std::vector<std::atomic<int>> mal(97);
        parallelJe(mal.size(), [&](std::size_t i) { ++mal[i]; }, threads);
        CHECK(std::all_of(mal.begin(), mal.end(), [](const std::atomic<int>& m) { return m.load() == 1; }));
    }
    parallelJe(0, [](std::size_t) { FAIL("keine Aufgabe"); });
    // eine Ausnahme kommt beim Rufer an, die uebrigen Aufgaben laufen zu Ende
    std::atomic<int> fertig{0};
    CHECK_THROWS_AS(parallelJe(
                        40,
                        [&](std::size_t i) {
                            if (i == 7) throw std::runtime_error("sieben");
                            ++fertig;
                        },
                        4),
                    std::runtime_error);
    CHECK(fertig.load() == 39);
}

TEST_CASE("InstrumentCore 34c Kurve wie Tastenkurve, Bit fuer Bit")
{
    Zufall z{34};
    for (int fall = 0; fall < 40; ++fall)
    {
        CAPTURE(fall);
        const int zahl = 1 + fall % 7;
        Kurve k;
        lumi::loop::Tastenkurve t;
        for (int i = 0; i < zahl; ++i)
        {
            const int taste = 21 + static_cast<int>((z() + 1.0) * 43.5);
            const double wert = 0.01 + (z() + 1.0) * 5.0;
            k.punkte.push_back({taste, wert});
            t.punkte.push_back({taste, wert});
        }
        if (fall % 3 == 0)
        {
            k.abweichungen.push_back({60, 7.5});
            t.abweichungen.push_back({60, 7.5});
        }
        richteKurve(k, 0.001, 100.0, Kurve{}, 21, 108);
        lumi::loop::richteKurve(t, 0.001, 100.0, lumi::loop::Tastenkurve{});
        for (int taste = 15; taste <= 115; ++taste)
        {
            CAPTURE(taste);
            CHECK(wertAn(k, taste, Skala::Linear) == lumi::loop::wertAn(t, taste, lumi::loop::KurvenSkala::Linear));
            CHECK(wertAn(k, taste, Skala::Logarithmisch) == lumi::loop::wertAn(t, taste, lumi::loop::KurvenSkala::Logarithmisch));
        }
    }
    // Grenzen und Ersatz
    Kurve leer;
    richteKurve(leer, 0.0, 1.0, kurveKonstant(0.5, Belegkraft::Quelle, "Ersatz"), 36, 96);
    CHECK(wertAn(leer, 60, Skala::Linear) == 0.5);
    CHECK(leer.herkunft == "Ersatz");
    Kurve weit = kurveAusPunkten({{0, -5.0}, {127, 5.0}});
    richteKurve(weit, 0.0, 1.0, Kurve{}, 36, 96);
    CHECK(weit.punkte.front().note == 36);
    CHECK(weit.punkte.back().note == 96);
    CHECK(weit.punkte.front().wert == 0.0);
    CHECK(weit.punkte.back().wert == 1.0);
    CHECK(kurveAlsText(kurveAusPunkten({{36, 1.0}, {96, 2.5}})) == "36:1 96:2.5");
}

TEST_CASE("InstrumentCore 34c Modal: eine Mode")
{
    const double rate = 48000.0;
    const double hz = 440.0;
    const double t60 = 0.5;
    const double masse = 0.01;
    const Mode m = modeAus(hz, t60, masse, 1.0, 1.0, 1.0);
    const std::vector<double> h = impulsantwort({m}, rate, 4800);
    REQUIRE(h.size() == 4800);
    // h(t) = 1/(m·ω_d) · e^(−σt) · sin(ω_d t), die Mode sieht den Stoss einen Schritt spaeter (Bank 2010 Gl. 10)
    const double sigma = sigmaAusT60(t60);
    const double omega = 2.0 * std::numbers::pi * hz;
    double groesste = 0.0, fehler = 0.0;
    for (std::size_t i = 1; i < h.size(); ++i)
    {
        const double t = static_cast<double>(i) / rate;
        const double soll = std::exp(-sigma * t) * std::sin(omega * t) / (masse * omega);
        groesste = std::max(groesste, std::abs(soll));
        fehler = std::max(fehler, std::abs(h[i] - soll));
    }
    CHECK(fehler < 1e-9 * groesste);
    // Nachklang bis −60 dB: T60 in Frames
    CHECK(nachklangFrames({m}, rate, 60.0, 1'000'000) == doctest::Approx(t60 * rate).epsilon(1e-3));
    CHECK(nachklangFrames({m}, rate, 60.0, 1000) == 1000);
}

TEST_CASE("InstrumentCore 34c Tonauftrag")
{
    Spur s{std::string(spur::kExpression), {{100, 0.0}, {200, 1.0}}};
    CHECK(spurWert(s, 0, 0.7) == 0.0);
    CHECK(spurWert(s, 150, 0.7) == 0.5);
    CHECK(spurWert(s, 1000, 0.7) == 1.0);
    CHECK(spurWert(Spur{}, 5, 0.7) == 0.7);

    Tonauftrag a = einzelnote(60, 261.6, -6, 4800, 9600, 48000.0);
    CHECK(pruefeAuftrag(a).empty());
    a.spuren.push_back(s);
    CHECK(findeSpur(a, spur::kExpression) != nullptr);
    CHECK(findeSpur(a, spur::kPitch) == nullptr);
    a.spuren.push_back(s);  // doppelt
    a.stufe = 3;
    a.noten.push_back(KettenNote{-10, -20, 200, 0.0, false});
    const std::vector<std::string> f = pruefeAuftrag(a);
    CHECK(f.size() == 6);  // Stufe; Note 1: ende, note, hz, Reihenfolge; Spur doppelt
}

TEST_CASE("InstrumentCore 34c Parametertafel: Pruefung und Vorgaben")
{
    const Parametertafel t = pruefTafel();
    CHECK(pruefeTafel(t).empty());
    const Werte v = vorgaben(t);
    REQUIRE(v.felder.size() == t.werte.size());
    CHECK(v.zahl("t60") == 1.0);
    CHECK(v.wahl("waveform") == "sine");
    CHECK_FALSE(v.schalter("bright"));
    CHECK(v.liste("chorus").eintraege.size() == 1);
    CHECK_THROWS_AS((void)v.zahl("waveform"), std::out_of_range);
    CHECK_THROWS_AS((void)v.zahl("fehlt"), std::out_of_range);
    CHECK(darstellungZu(WertArt::Liste) == Darstellung::Dialog);

    Parametertafel falsch = t;
    falsch.werte[0].gruppe = "Nirgends";             // Gruppe nicht gelistet
    falsch.werte[1].vorgabe = std::string("saw");    // Wahl nicht unter den Wahlen
    falsch.werte[2].darstellung = Darstellung::Regler;  // Schalter als Regler
    falsch.werte[3].vorgabe = 1.0;                   // Kurve mit Zahl als Vorgabe
    falsch.werte[5].abhaengigVon = {"gibtsNicht"};   // Abhaengigkeit fehlt
    falsch.werte.push_back(falsch.werte[0]);         // Name doppelt (und Gruppe)
    falsch.noteMin = 100;                            // Umfang verkehrt
    const std::vector<std::string> f = pruefeTafel(falsch);
    for (const char* teil : {"group not listed", "default not among the choices", "display does not fit", "default is not a curve",
                             "dependency unknown", "name twice", "key range invalid"})
    {
        CAPTURE(teil);
        CHECK(std::any_of(f.begin(), f.end(), [&](const std::string& s) { return s.find(teil) != std::string::npos; }));
    }
}

TEST_CASE("InstrumentCore 34c Parametertafel: Werte richten")
{
    const Parametertafel t = pruefTafel();
    Werte w = vorgaben(t);
    w.setze("t60", 99.0);                                                     // geklemmt
    w.setze("weight", kurveAusPunkten({{0, 50.0}, {127, 0.2}}, Belegkraft::Gemessen));  // Noten und Werte geklemmt
    std::vector<std::string> f = richteWerte(t, w);
    CHECK(f.empty());
    CHECK(w.zahl("t60") == 10.0);
    CHECK(w.kurve("weight").punkte.front().note == 36);
    CHECK(w.kurve("weight").punkte.front().wert == 10.0);
    CHECK(w.kurve("weight").belegkraft == Belegkraft::Gemessen);

    // fehlt, unbekannt, falsche Art, unbekannte Wahl, Liste zu lang, gestimmte Kurve leer
    Werte schlecht = vorgaben(t);
    schlecht.felder.erase(schlecht.felder.begin());  // t60 fehlt
    schlecht.setze("extra", 1.0);
    schlecht.setze("bright", 1.0);
    schlecht.setze("waveform", std::string("saw"));
    Werte stimme;
    stimme.setze("detune", 0.0);
    schlecht.setze("chorus", Liste{{stimme, stimme, stimme, stimme}});
    schlecht.setze("lengthTuned", Kurve{});
    f = richteWerte(t, schlecht);
    for (const char* teil : {"t60: missing", "extra: unknown", "bright: is not a switch", "unknown choice saw", "4 entries",
                             "lengthTuned: tuned value empty"})
    {
        CAPTURE(teil);
        CHECK(std::any_of(f.begin(), f.end(), [&](const std::string& s) { return s.find(teil) != std::string::npos; }));
    }
    CHECK(f.size() == 6);
    // ein Wert mit Fehler bleibt, wie er ist; ein unbekannter faellt weg; ein fehlender wird nicht ersetzt
    CHECK(schlecht.wahl("waveform") == "saw");
    CHECK(schlecht.finde("extra") == nullptr);
    CHECK(schlecht.finde("t60") == nullptr);
    // die Reihenfolge der Tafel
    const Werte vorgabe = vorgaben(t);
    Werte umgekehrt;
    for (auto it = vorgabe.felder.rbegin(); it != vorgabe.felder.rend(); ++it) umgekehrt.setze(it->first, it->second);
    CHECK(richteWerte(t, umgekehrt).empty());
    CHECK(umgekehrt == vorgabe);
}

TEST_CASE("InstrumentCore 34c Parametertafel: gestimmte Werte und ungemessen")
{
    const Parametertafel t = pruefTafel();
    Werte w = vorgaben(t);
    // ohne gemerkten Abdruck: veraltet
    CHECK(veralteteGestimmte(t, w) == std::vector<std::string>{"lengthTuned"});
    const std::string abdruck = eingabenAbdruck(t, w, "lengthTuned");
    CHECK(abdruck.size() == 64);
    CHECK(eingabenAbdruck(t, w, "t60").empty());  // kein gestimmter Wert
    w.merkeAbdruck("lengthTuned", abdruck);
    CHECK(veralteteGestimmte(t, w).empty());
    // eine Aenderung ausserhalb der Abhaengigkeiten aendert nichts ...
    w.setze("weight", kurveKonstant(3.0, Belegkraft::Quelle, "", 36, 96));
    CHECK(veralteteGestimmte(t, w).empty());
    // ... eine an `t60` macht ihn veraltet, auch im letzten Bit
    w.setze("t60", std::nextafter(1.0, 2.0));
    CHECK(veralteteGestimmte(t, w) == std::vector<std::string>{"lengthTuned"});
    // eine neue Fassung der Rechnung ebenso
    Parametertafel neu = t;
    neu.rechenFassung = 2;
    w.setze("t60", 1.0);
    CHECK(eingabenAbdruck(neu, w, "lengthTuned") != abdruck);
    CHECK(eingabenAbdruck(t, w, "lengthTuned") == abdruck);

    // ungemessen, bis jeder klangbestimmende Wert gemessen ist
    CHECK(ungemessen(t, w));
    Parametertafel gemessen = t;
    for (ParameterAngabe& a : gemessen.werte) a.belegkraft = Belegkraft::Gemessen;
    w.setze("weight", kurveKonstant(3.0, Belegkraft::Gemessen, "Messung", 36, 96));
    CHECK(ungemessen(gemessen, w));  // die gestimmte Kurve traegt Rechnung
    Kurve gestimmt = w.kurve("lengthTuned");
    gestimmt.belegkraft = Belegkraft::Gemessen;
    w.setze("lengthTuned", gestimmt);
    CHECK_FALSE(ungemessen(gemessen, w));

    for (const Belegkraft b : {Belegkraft::Quelle, Belegkraft::Rechnung, Belegkraft::Gemessen, Belegkraft::Offen})
    {
        Belegkraft zurueck = Belegkraft::Offen;
        REQUIRE(belegkraftAusName(belegkraftName(b), zurueck));
        CHECK(zurueck == b);
    }
    Belegkraft x = Belegkraft::Quelle;
    CHECK_FALSE(belegkraftAusName("guessed", x));
    CHECK(wertAlsText(1.0) == "d3ff0000000000000");
}

TEST_CASE("InstrumentCore 34c Fabrik und normiere")
{
    // die eingebauten Kerne stehen in InstrumentModels (BK1 a, test_InstrumentModels.cpp)
    Fabrik fabrik;
    CHECK(fabrik.registriere(pruefTyp()).empty());
    CHECK_FALSE(fabrik.registriere(pruefTyp()).empty());  // doppelt
    KernTyp ohne = pruefTyp();
    ohne.tafel.kern = "other";
    ohne.erzeuge = nullptr;
    CHECK_FALSE(fabrik.registriere(ohne).empty());
    CHECK(fabrik.namen() == std::vector<std::string>{"testSine"});
    REQUIRE(fabrik.typ("testSine") != nullptr);
    CHECK(fabrik.typ("testSine")->auskunft.rechenweg == Rechenweg::Note);

    std::vector<std::string> fehler;
    CHECK(fabrik.erzeuge("nope", Werte{}, &fehler) == nullptr);
    CHECK(fehler.size() == 1);
    fehler.clear();
    CHECK(fabrik.erzeuge("testSine", Werte{}, &fehler) == nullptr);  // jeder Wert fehlt
    CHECK(fehler.size() == pruefTafel().werte.size());

    const Werte v = vorgaben(pruefTafel());
    const std::shared_ptr<const Tonkern> k = fabrik.erzeuge("testSine", v);
    REQUIRE(k);
    const Tonauftrag probe = einzelnote(66, 440.0, 0, 24000, 24000, 48000.0);
    CHECK(k->nachklang(probe, 1'000'000) == 48000);
    std::vector<float> ton;
    Innere innere;
    innere.gewuenscht = {"envelope", "unknown"};
    k->rechne(probe, ton, &innere);
    CHECK(ton.size() == 24000);
    REQUIRE(innere.ergebnis.size() == 1);
    CHECK(innere.ergebnis[0].werte.front() == doctest::Approx(kRohAmplitude));

    // normiere findet den Bezug, mit dem die Spitze 0,5 ist (roh knapp unter 2: die erste Spitze liegt nach dem Abklingen
    // eines Viertels einer Schwingung)
    const double bezug = normiere(fabrik, "testSine", v, "outputReference", probe, 2);
    CHECK(bezug == doctest::Approx(kRohAmplitude / kNormSpitze).epsilon(0.01));
    Werte mit = v;
    mit.setze("outputReference", bezug);
    fabrik.erzeuge("testSine", mit)->rechne(probe, ton);
    double spitze = 0.0;
    for (const float x : ton) spitze = std::max(spitze, static_cast<double>(std::abs(x)));
    CHECK(spitze == doctest::Approx(kNormSpitze).epsilon(1e-6));
    fehler.clear();
    CHECK(normiere(fabrik, "testSine", v, "waveform", probe, 1, &fehler) == 0.0);
    CHECK(fehler.size() == 1);
}
