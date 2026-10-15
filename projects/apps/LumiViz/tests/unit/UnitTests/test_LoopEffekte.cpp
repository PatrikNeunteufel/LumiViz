/**
 ****************************************************************************************
 * @file   test_LoopEffekte.cpp
 * @brief  Loop: die Effekte am Clip als Rechenbausteine — Verzerrer, Echo, Hall
 *         (docs/audio/Loop_Konzept.md §4g, Entscheid G1 b)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Fenster: die Bausteine rechnen Bloecke um, die Faelle
 * zaehlen die Werte nach. Der letzte Fall misst, wie lange das Rechnen dauert
 * — er meldet Zahlen und prueft nur, dass die Kette schneller rechnet, als der
 * Ton laeuft. `LUMIVIZ_EFFEKT_MESSUNG_S=<Sekunden>` misst an mehr Ton.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Effekte.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Rechenkern.hpp"

#include <QByteArray>
#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kEffektRate = 44100;

/// Rauschen, immer dasselbe: Stereo, Betrag bis `spitze`.
std::vector<float> rauschen(int frames, float spitze = 0.5f)
{
    std::vector<float> werte(2 * static_cast<std::size_t>(frames));
    std::uint32_t zustand = 12345u;
    for (float& wert : werte)
    {
        zustand = zustand * 1664525u + 1013904223u;
        wert = (static_cast<float>(zustand >> 8) / 8388608.0f - 1.0f) * spitze;
    }
    return werte;
}

/// Ein Schlag am Anfang, danach Stille.
std::vector<float> schlag(int frames)
{
    std::vector<float> werte(2 * static_cast<std::size_t>(frames), 0.0f);
    werte[0] = 1.0f;
    werte[1] = 1.0f;
    return werte;
}

/// Mittlere Leistung der linken Seite in einem Stueck, in dB.
double pegelDb(const std::vector<float>& werte, double vonSek, double bisSek)
{
    const auto von = static_cast<std::size_t>(vonSek * kEffektRate);
    const auto bis = std::min(static_cast<std::size_t>(bisSek * kEffektRate), werte.size() / 2);
    double summe = 0.0;
    for (std::size_t f = von; f < bis; ++f) summe += static_cast<double>(werte[2 * f]) * werte[2 * f];
    return 10.0 * std::log10(summe / static_cast<double>(std::max<std::size_t>(1, bis - von)) + 1.0e-30);
}

/// Eine feste Rechenarbeit, gegen die sich eine Messung vergleichen laesst:
/// drosselt der Rechner, dauert auch sie laenger.
double taktprobeMs()
{
    const auto start = std::chrono::steady_clock::now();
    volatile double summe = 0.0;
    for (int i = 1; i <= 4'000'000; ++i) summe = summe + std::sqrt(static_cast<double>(i));
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

}  // namespace

TEST_CASE("Loop Effekte: ohne eingeschalteten Effekt bleibt der Ton, wie er ist")
{
    const std::vector<float> vorher = rauschen(4000);
    std::vector<float> werte = vorher;
    EffektKette kette;
    ClipEffekte effekte;
    effekte.hole<ClipHall>().sekunden = 5.0;  // eingestellt, aber aus
    kette.richteEin(effekte, kEffektRate, 120.0);
    CHECK(!kette.an());
    kette.rechne(werte.data(), 4000);
    CHECK(werte == vorher);

    SUBCASE("ein Effekt mit Mix 0 aendert auch nichts")
    {
        effekte.hole<ClipVerzerrung>() = {true, 0.9, 0.5, 0.0};
        effekte.hole<ClipEcho>() = {true, EchoZeit::Achtel, 0.5, 0.0, 0.0};
        effekte.hole<ClipHall>().an = true;
        effekte.hole<ClipHall>().mix = 0.0;
        kette.richteEin(effekte, kEffektRate, 120.0);
        kette.rechne(werte.data(), 4000);
        CHECK(werte == vorher);
    }
}

TEST_CASE("Loop Effekte: der Verzerrer hebt Leises an und laesst die Spitze bei 1")
{
    Verzerrer verzerrer;
    verzerrer.richteEin({true, 0.5, 1.0, 1.0}, kEffektRate);

    // ein Gleichwert von 0,5 landet bei tanh(8 * 0,5 * 0,5) / tanh(8 * 0,5)
    std::vector<float> gleich(2 * 2000, 0.5f);
    verzerrer.rechne(gleich.data(), 2000);
    CHECK(gleich[2 * 1999] == doctest::Approx(std::tanh(2.0) / std::tanh(4.0)).epsilon(0.001));

    // ein Sinus mit Spitze 1 bleibt bei 1
    verzerrer.leere();
    std::vector<float> sinus(2 * 4410);
    for (int f = 0; f < 4410; ++f)
        sinus[2 * f] = sinus[2 * f + 1] = static_cast<float>(std::sin(2.0 * 3.14159265358979 * 220.0 * f / kEffektRate));
    verzerrer.rechne(sinus.data(), 4410);
    float spitze = 0.0f;
    for (const float wert : sinus) spitze = std::max(spitze, std::abs(wert));
    CHECK(spitze <= 1.0001f);
    CHECK(spitze > 0.95f);
}

TEST_CASE("Loop Effekte: das Echo wiederholt im Notenwert, jede Wiederholung um Feedback leiser")
{
    // 120 BPM, eine Viertel: 0,5 s = 22050 Abtastwerte
    EchoEffekt echo;
    echo.richteEin({true, EchoZeit::Viertel, 0.5, 0.0, 0.5}, kEffektRate, 120.0);
    REQUIRE(echo.verzoegerung() == 22050);

    const int frames = 4 * 22050;
    std::vector<float> werte = schlag(frames);
    echo.rechne(werte.data(), frames);
    CHECK(werte[0] == doctest::Approx(0.5));               // der Schlag selbst, Mix 50 %
    CHECK(werte[2 * 22050] == doctest::Approx(0.5));       // die erste Wiederholung
    CHECK(werte[2 * 44100] == doctest::Approx(0.25));      // um Feedback leiser
    CHECK(werte[2 * 66150 + 1] == doctest::Approx(0.125)); // rechts wie links
    // dazwischen ist Stille
    float daneben = 0.0f;
    for (int f = 1; f < frames; ++f)
        if (f % 22050 != 0) daneben = std::max(daneben, std::abs(werte[2 * static_cast<std::size_t>(f)]));
    CHECK(daneben < 1.0e-6f);

    SUBCASE("die Zeit folgt dem Tempo und dem Notenwert")
    {
        echo.richteEin({true, EchoZeit::AchtelPunktiert, 0.5, 0.0, 0.5}, kEffektRate, 100.0);
        CHECK(echo.verzoegerung() == 19845);  // 0,75 Schlaege * 0,6 s * 44100
        echo.richteEin({false, EchoZeit::Viertel, 0.5, 0.0, 0.5}, kEffektRate, 120.0);
        CHECK(echo.verzoegerung() == 0);
    }
    SUBCASE("mit Damping wird jede Wiederholung dunkler und damit an der Spitze leiser")
    {
        echo.richteEin({true, EchoZeit::Viertel, 0.5, 0.8, 0.5}, kEffektRate, 120.0);
        std::vector<float> gedaempft = schlag(frames);
        echo.rechne(gedaempft.data(), frames);
        CHECK(gedaempft[2 * 22050] == doctest::Approx(0.5));  // die erste kommt noch ungefiltert
        CHECK(std::abs(gedaempft[2 * 44100]) < 0.2f);
    }
}

TEST_CASE("Loop Effekte: der Hall faellt in der eingestellten Zeit um 60 dB")
{
    HallEffekt hall;
    ClipHall werte;
    werte.an = true;
    werte.sekunden = 2.0;
    werte.daempfung = 0.0;
    werte.mix = 1.0;

    for (const double groesse : {0.0, 0.5, 1.0})
    {
        CAPTURE(groesse);
        werte.groesse = groesse;
        hall.richteEin(werte, kEffektRate);
        const int frames = 3 * kEffektRate;
        std::vector<float> ton = schlag(frames);
        hall.rechne(ton.data(), frames);
        // zwei Fenster im Abstand von 2 s: dazwischen liegen 60 dB
        const double frueh = pegelDb(ton, 0.2, 0.5);
        const double spaet = pegelDb(ton, 2.2, 2.5);
        MESSAGE("Size " << groesse << ": " << frueh << " dB bei 0,2 bis 0,5 s, " << spaet
                        << " dB bei 2,2 bis 2,5 s, Abfall " << (frueh - spaet) << " dB");
        CHECK(frueh - spaet > 54.0);
        CHECK(frueh - spaet < 66.0);
    }
}

TEST_CASE("Loop Effekte: gleich in welchen Bloecken gerechnet wird, das Ergebnis ist dasselbe")
{
    ClipEffekte effekte;
    effekte.hole<ClipVerzerrung>() = {true, 0.6, 0.4, 0.7};
    effekte.hole<ClipEcho>() = {true, EchoZeit::Sechzehntel, 0.6, 0.4, 0.4};
    effekte.hole<ClipHall>() = {true, 0.7, 1.2, 0.5, 0.3};
    const int frames = 30000;
    const std::vector<float> ton = rauschen(frames);

    EffektKette amStueck;
    amStueck.richteEin(effekte, kEffektRate, 128.0);
    CHECK(amStueck.an());
    std::vector<float> a = ton;
    amStueck.rechne(a.data(), frames);

    // dieselben Groessen, die der Mixer im Beleg aus Stufe 2 holte: 3 bis 8167
    EffektKette inStuecken;
    inStuecken.richteEin(effekte, kEffektRate, 128.0);
    std::vector<float> b = ton;
    const int stuecke[] = {3, 441, 8167, 220, 1, 5000};
    int stelle = 0;
    for (int i = 0; stelle < frames; ++i)
    {
        const int n = std::min(stuecke[i % 6], frames - stelle);
        inStuecken.rechne(b.data() + 2 * static_cast<std::ptrdiff_t>(stelle), n);
        stelle += n;
    }
    CHECK(a == b);
    CHECK(a != ton);

    SUBCASE("nach leere klingt die Kette wie frisch eingerichtet")
    {
        const std::vector<float> einer = schlag(8000);
        std::vector<float> gebraucht = einer;
        amStueck.leere();
        amStueck.rechne(gebraucht.data(), 8000);
        EffektKette frisch;
        frisch.richteEin(effekte, kEffektRate, 128.0);
        std::vector<float> neu = einer;
        frisch.rechne(neu.data(), 8000);
        CHECK(gebraucht == neu);
    }
}

TEST_CASE("Loop Effekte: wie lange ein Clip nachklingt")
{
    ClipEffekte effekte;
    CHECK(nachklangSekunden(effekte, 120.0) == 0.0);
    effekte.hole<ClipVerzerrung>().an = true;  // ein Verzerrer klingt nicht nach
    CHECK(nachklangSekunden(effekte, 120.0) == 0.0);
    effekte.hole<ClipHall>().an = true;
    effekte.hole<ClipHall>().sekunden = 1.8;
    CHECK(nachklangSekunden(effekte, 120.0) == doctest::Approx(1.8));
    // Viertel bei 120 BPM = 0,5 s; Feedback 0,5 braucht zehn Wiederholungen bis -60 dB
    effekte.hole<ClipEcho>() = {true, EchoZeit::Viertel, 0.5, 0.0, 0.5};
    CHECK(nachklangSekunden(effekte, 120.0) == doctest::Approx(0.5 * 11 + 1.8));
}

// --- Die Ketten im Klangerzeuger (12b Teil 2) ---
//
// Der kleine Loop: ein Schlag je Takt, 120 BPM — ein Muster von vier Takten
// dauert 88200 Abtastwerte. Kanal K spielt „M" (ein Schlag am Anfang, mit
// den Effekten) und danach „L" (leer): ein Durchlauf ist 176400 lang. Der
// Klang ist 2000 Abtastwerte lang und hat ueberall den Wert 0,5.

namespace
{

Loop effektLoop(const ClipEffekte& effekte, double ersterSchritt = 1.0)
{
    Loop loop;
    loop.laut = 1.0;
    loop.schlaegeJeTakt = 1;
    Muster m;
    m.name = QStringLiteral("M");
    m.spuren = {{QStringLiteral("A"), QStringLiteral("a.wav"), 1.0, 0.0, false}};
    m.zeilen = {{ersterSchritt}};
    Muster leer = m;
    leer.name = QStringLiteral("L");
    leer.zeilen = {{}};
    loop.muster = {m, leer};
    loop.kanaele = {
        {QStringLiteral("K"), true, {LoopClip{QStringLiteral("M"), 0, 1, effekte}, LoopClip{QStringLiteral("L"), 4, 1}}}};
    loop.richte();
    return loop;
}

std::shared_ptr<const std::vector<Klang>> effektKlang()
{
    auto klaenge = std::make_shared<std::vector<Klang>>(1);
    (*klaenge)[0].daten.assign(2 * 2000, 0.5f);
    return klaenge;
}

/// `frames` Abtastwerte aus dem Klangerzeuger holen, in Bloecken wie der Mixer.
std::vector<float> spiele(Klangerzeuger& erzeuger, int frames, int block = 441)
{
    std::vector<float> werte(2 * static_cast<std::size_t>(frames));
    for (int stelle = 0; stelle < frames; stelle += block)
        erzeuger.render(werte.data() + 2 * static_cast<std::ptrdiff_t>(stelle), std::min(block, frames - stelle));
    return werte;
}

float groessterBetrag(const std::vector<float>& werte)
{
    float spitze = 0.0f;
    for (const float wert : werte) spitze = std::max(spitze, std::abs(wert));
    return spitze;
}

ClipEffekte nurEcho(double feedback = 0.8, double mix = 0.5, EchoZeit zeit = EchoZeit::Viertel)
{
    ClipEffekte effekte;
    effekte.hole<ClipEcho>() = {true, zeit, feedback, 0.0, mix};
    return effekte;
}

}  // namespace

TEST_CASE("Loop Effekte im Klangerzeuger: ein Clip mit Effekten klingt wie sein Ton durch die Kette")
{
    ClipEffekte effekte = nurEcho(0.5);
    effekte.hole<ClipVerzerrung>() = {true, 0.5, 0.8, 1.0};
    effekte.hole<ClipHall>() = {true, 0.5, 0.6, 0.3, 0.3};
    const Loop loop = effektLoop(effekte);
    const auto klaenge = effektKlang();
    const int frames = 176400;

    Klangerzeuger erzeuger;
    const auto programm = baueProgramm(loop, kEffektRate, klaenge);
    REQUIRE(programm->ablauf.busse.size() == 1u);
    REQUIRE(programm->effekte != nullptr);
    erzeuger.setzeProgramm(programm);
    const std::vector<float> gespielt = spiele(erzeuger, frames);

    // dasselbe von Hand: der Ton ohne Effekte, dann die Kette darueber
    std::vector<float> vonHand(2 * static_cast<std::size_t>(frames));
    rechne(programm->ablauf, *klaenge, 0, frames, vonHand.data());
    const std::vector<float> trocken = vonHand;
    EffektKette kette;
    kette.richteEin(effekte, kEffektRate, 120.0);
    kette.rechne(vonHand.data(), frames);

    float abstand = 0.0f;
    for (std::size_t i = 0; i < gespielt.size(); ++i) abstand = std::max(abstand, std::abs(gespielt[i] - vonHand[i]));
    MESSAGE("groesster Abstand zwischen Klangerzeuger und Kette von Hand: " << abstand << " (Spitze "
                                                                            << groessterBetrag(gespielt) << ")");
    // nicht null: der Klangerzeuger hoert auf zu rechnen, wenn ein Clip um 60 dB verklungen ist
    CHECK(abstand < 0.001f);
    CHECK(gespielt != trocken);
}

TEST_CASE("Loop Effekte im Klangerzeuger: die Effekte treffen nur ihren Clip und klingen ueber sein Ende hinaus")
{
    const auto klaenge = effektKlang();
    Klangerzeuger erzeuger;

    SUBCASE("ein zweiter Clip derselben Basis ohne Effekte bleibt, wie er ist")
    {
        // der Verzerrer klingt nicht nach: beim zweiten Clip ist der erste vorbei
        ClipEffekte verzerrt;
        verzerrt.hole<ClipVerzerrung>() = {true, 0.8, 0.5, 1.0};
        Loop loop = effektLoop(verzerrt);
        loop.kanaele[0].clips[1].muster = QStringLiteral("M");
        loop.richte();
        const auto programm = baueProgramm(loop, kEffektRate, klaenge);
        REQUIRE(programm->ablauf.busse.size() == 1u);
        erzeuger.setzeProgramm(programm);
        const std::vector<float> gespielt = spiele(erzeuger, 100000);
        std::vector<float> trocken(2 * 100000);
        rechne(programm->ablauf, *klaenge, 0, 100000, trocken.data());
        // der erste Clip (Schlag bei 0) ist verzerrt, der zweite (Schlag bei 88200) auf den Wert gleich
        CHECK(gespielt[2 * 1000] != trocken[2 * 1000]);
        CHECK(std::equal(gespielt.begin() + 2 * 88000, gespielt.end(), trocken.begin() + 2 * 88000));
        CHECK(trocken[2 * 88300] == 0.5f);
    }
    SUBCASE("das Echo eines Clips klingt weiter, wenn der Clip zu Ende ist")
    {
        const auto programm = baueProgramm(effektLoop(nurEcho(0.8)), kEffektRate, klaenge);
        erzeuger.setzeProgramm(programm);
        const std::vector<float> gespielt = spiele(erzeuger, 100000);
        // der Clip endet bei 88200; die vierte Wiederholung kommt genau dort:
        // Mix 0,5 * Klang 0,5 * Feedback 0,8 hoch 3
        CHECK(gespielt[2 * 88300] == doctest::Approx(0.128));
        std::vector<float> trocken(2 * 100000);
        rechne(programm->ablauf, *klaenge, 0, 100000, trocken.data());
        CHECK(trocken[2 * 88300] == 0.0f);
    }
}

TEST_CASE("Loop Effekte im Klangerzeuger: nach einem weiten Sprung beginnt der Nachklang neu, nach einem kurzen nicht")
{
    // Entscheid Patrik S113, G1 b; die Grenze von 200 ms ist von Claude gesetzt
    const auto klaenge = effektKlang();
    const auto programm = baueProgramm(effektLoop(nurEcho(0.8)), kEffektRate, klaenge);
    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(programm);
    (void)spiele(erzeuger, 30000);  // der Schlag und seine erste Wiederholung (bei 22050) sind durch

    SUBCASE("weit: die zweite Wiederholung bleibt aus, wie bei einem frischen Klangerzeuger")
    {
        erzeuger.setzeStelle(44000);  // 14000 weiter als erwartet = 317 ms
        const std::vector<float> danach = spiele(erzeuger, 20000);
        CHECK(groessterBetrag(danach) == 0.0f);
    }
    SUBCASE("kurz: der Nachklang laeuft weiter")
    {
        erzeuger.setzeStelle(30000 + 4410);  // 100 ms
        const std::vector<float> danach = spiele(erzeuger, 20000);
        // die zweite Wiederholung: 22050 Abtastwerte nach der ersten, gezaehlt in gerechneten Bloecken
        CHECK(danach[2 * (44100 - 30000 + 100)] == doctest::Approx(0.2));
    }
    SUBCASE("ohne Sprung ebenso")
    {
        const std::vector<float> danach = spiele(erzeuger, 20000);
        CHECK(danach[2 * (44100 - 30000 + 100)] == doctest::Approx(0.2));
    }
}

TEST_CASE("Loop Effekte im Klangerzeuger: ein neues Programm nimmt den Nachklang mit, wo er passt")
{
    const auto klaenge = effektKlang();
    Klangerzeuger erzeuger;
    const auto erstes = baueProgramm(effektLoop(nurEcho(0.8)), kEffektRate, klaenge);
    erzeuger.setzeProgramm(erstes);
    (void)spiele(erzeuger, 30000);
    const std::size_t zweite = 2 * (44100 - 30000 + 100);  // die zweite Wiederholung, nach 30000 gespielten

    SUBCASE("der Schritt im Muster wird geloescht: was schon klang, klingt aus")
    {
        erzeuger.setzeProgramm(baueProgramm(effektLoop(nurEcho(0.8), 0.0), kEffektRate, klaenge));
        const std::vector<float> danach = spiele(erzeuger, 20000);
        CHECK(danach[zweite] == doctest::Approx(0.2));
    }
    SUBCASE("nur Mix aendert sich: der Nachklang bleibt und kommt mit dem neuen Mix")
    {
        erzeuger.setzeProgramm(baueProgramm(effektLoop(nurEcho(0.8, 0.25)), kEffektRate, klaenge));
        const std::vector<float> danach = spiele(erzeuger, 20000);
        CHECK(danach[zweite] == doctest::Approx(0.1));
    }
    SUBCASE("die Echo-Zeit aendert sich: dieses Echo beginnt leer")
    {
        erzeuger.setzeProgramm(baueProgramm(effektLoop(nurEcho(0.8, 0.5, EchoZeit::Achtel)), kEffektRate, klaenge));
        const std::vector<float> danach = spiele(erzeuger, 20000);
        CHECK(groessterBetrag(danach) < 1.0e-6f);
    }
    SUBCASE("der Clip liegt in einem anderen Takt: er gilt als anderer Clip")
    {
        Loop verschoben = effektLoop(nurEcho(0.8), 0.0);
        verschoben.kanaele[0].clips = {LoopClip{QStringLiteral("L"), 0, 1}, LoopClip{QStringLiteral("M"), 4, 1, nurEcho(0.8)}};
        verschoben.richte();
        erzeuger.setzeProgramm(baueProgramm(verschoben, kEffektRate, klaenge));
        const std::vector<float> danach = spiele(erzeuger, 20000);
        CHECK(groessterBetrag(danach) == 0.0f);
    }
    SUBCASE("ein abgeloestes Programm gibt der Klangerzeuger wieder her")
    {
        // der Ton-Thread laesst es nicht selbst los, sondern gibt es ab; das
        // naechste setzeProgramm raeumt es weg
        erzeuger.setzeProgramm(baueProgramm(effektLoop(nurEcho(0.8)), kEffektRate, klaenge));
        (void)spiele(erzeuger, 441);
        CHECK(erstes.use_count() > 1);
        erzeuger.setzeProgramm(baueProgramm(effektLoop(nurEcho(0.8)), kEffektRate, klaenge));
        CHECK(erstes.use_count() == 1);
    }
}

TEST_CASE("Loop Effekte Messung: die Rechenzeit einer Kette gegen die Dauer des Tons")
{
    // Drei Laeufe je Stellung, davor und danach die Taktprobe: dauert sie
    // laenger als sonst, hat der Rechner gedrosselt und die Zahl gilt nicht.
    bool ok = false;
    int sekunden = qEnvironmentVariableIntValue("LUMIVIZ_EFFEKT_MESSUNG_S", &ok);
    if (!ok || sekunden <= 0) sekunden = 5;
    const int frames = sekunden * kEffektRate;
    const std::vector<float> ton = rauschen(frames);
    constexpr int kBlock = 441;  // 10 ms, wie der spielende Mixer sie holt (Beleg §6)

    ClipEffekte nurHall;
    nurHall.hole<ClipHall>() = {true, 0.5, 1.8, 0.4, 0.25};
    ClipEffekte alle = nurHall;
    alle.hole<ClipVerzerrung>() = {true, 0.3, 0.5, 1.0};
    alle.hole<ClipEcho>() = {true, EchoZeit::AchtelPunktiert, 0.4, 0.3, 0.35};

    const auto messe = [&](const std::string& name, const ClipEffekte& effekte) {
        double schlechtester = 0.0;
        for (int lauf = 1; lauf <= 3; ++lauf)
        {
            EffektKette kette;
            kette.richteEin(effekte, kEffektRate, 120.0);
            std::vector<float> werte = ton;
            const double davor = taktprobeMs();
            const auto start = std::chrono::steady_clock::now();
            for (int stelle = 0; stelle < frames; stelle += kBlock)
                kette.rechne(werte.data() + 2 * static_cast<std::ptrdiff_t>(stelle), std::min(kBlock, frames - stelle));
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            const double danach = taktprobeMs();
            const double jeSekunde = ms / sekunden;
            schlechtester = std::max(schlechtester, jeSekunde);
            MESSAGE(name << ", Lauf " << lauf << ": " << ms << " ms fuer " << sekunden << " s Ton = " << jeSekunde
                         << " ms je Sekunde (" << jeSekunde / 10.0 << " % eines Kerns); Taktprobe davor " << davor
                         << " ms, danach " << danach << " ms");
        }
        return schlechtester;
    };
    const double hall = messe("nur Hall", nurHall);
    const double kette = messe("Verzerrer, Echo und Hall", alle);
    // die einzige Pruefung: eine Kette rechnet schneller, als der Ton laeuft —
    // sonst koennte der Ton-Thread sie nicht tragen
    CHECK(hall < 1000.0);
    CHECK(kette < 1000.0);
}

TEST_CASE("Loop Effekte Messung: viele Clips mit Effekten im Klangerzeuger (S127, Loop-Kleinkram 4)")
{
    // Je Kanal ein Clip ueber den ganzen Durchlauf mit Verzerrer, Echo und Hall,
    // auf jedem Schlag ein Schlag: alle Ketten rechnen die ganze Zeit. Gemessen
    // wird der Klangerzeuger in Bloecken wie der Mixer; meldet nur Zahlen —
    // aussagekraeftig unter Release-clang (Testing-MSVC rechnet die Reihe
    // dreissig- bis vierzigmal langsamer).
    bool ok = false;
    int sekunden = qEnvironmentVariableIntValue("LUMIVIZ_EFFEKT_MESSUNG_S", &ok);
    if (!ok || sekunden <= 0) sekunden = 5;
    const int frames = sekunden * kEffektRate;
    constexpr int kBlock = 441;

    ClipEffekte alle;
    alle.hole<ClipHall>() = {true, 0.5, 1.8, 0.4, 0.25};
    alle.hole<ClipVerzerrung>() = {true, 0.3, 0.5, 1.0};
    alle.hole<ClipEcho>() = {true, EchoZeit::AchtelPunktiert, 0.4, 0.3, 0.35};
    const auto klaenge = effektKlang();

    const auto loopMit = [&](int kanaele) {
        Loop loop;
        loop.laut = 1.0 / kanaele;
        loop.schlaegeJeTakt = 1;
        Muster m;
        m.name = QStringLiteral("M");
        m.spuren = {{QStringLiteral("A"), QStringLiteral("a.wav"), 1.0, 0.0, false}};
        m.zeilen = {std::vector<double>(16, 0.0)};
        for (std::size_t s = 0; s < 16; s += 4) m.zeilen[0][s] = 1.0;  // jeder Schlag
        loop.muster = {m};
        for (int k = 0; k < kanaele; ++k)
            loop.kanaele.push_back({QStringLiteral("K%1").arg(k), true, {LoopClip{QStringLiteral("M"), 0, 2, alle}}});
        loop.richte();
        return loop;
    };

    std::vector<std::pair<int, double>> ergebnisse;
    for (const int kanaele : {1, 8, 16, 32})
    {
        const auto programm = baueProgramm(loopMit(kanaele), kEffektRate, klaenge);
        REQUIRE(programm->ablauf.busse.size() == static_cast<std::size_t>(kanaele));
        double schlechtester = 0.0;
        for (int lauf = 1; lauf <= 3; ++lauf)
        {
            Klangerzeuger erzeuger;
            erzeuger.setzeProgramm(programm);
            std::vector<float> block(2 * kBlock);
            const double davor = taktprobeMs();
            const auto start = std::chrono::steady_clock::now();
            for (int stelle = 0; stelle < frames; stelle += kBlock) erzeuger.render(block.data(), std::min(kBlock, frames - stelle));
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            const double danach = taktprobeMs();
            const double jeSekunde = ms / sekunden;
            schlechtester = std::max(schlechtester, jeSekunde);
            MESSAGE(kanaele << " Clips mit Effekten, Lauf " << lauf << ": " << jeSekunde << " ms je Sekunde Ton ("
                            << jeSekunde / 10.0 << " % eines Kerns); Taktprobe davor " << davor << " ms, danach "
                            << danach << " ms");
        }
        ergebnisse.emplace_back(kanaele, schlechtester);
    }
    for (const auto& [kanaele, ms] : ergebnisse)
        MESSAGE("schlechtester Lauf, " << kanaele << " Clips: " << ms << " ms je Sekunde");
    // gemeldet, nicht gefordert: ab welcher Zahl der Ton-Thread nicht mehr mitkommt
    CHECK(ergebnisse.front().second < 1000.0);
}
