/**
 ****************************************************************************************
 * @file   test_LoopPYin.cpp
 * @brief  Loop, Stufe 15g: der Tonhoehenverlauf mit pYIN (Mauch & Dixon 2014) und sein Speicher
 *         (docs/audio/Loop_Konzept.md §4p, Entscheide Patrik WV6 a, WV8 a, GA1 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Gemessen an gerechneten Toenen mit bekanntem Verlauf (drei Teiltoene, damit es nach Stimme
 * klingt und YIN arbeiten muss): ein gehaltener Ton zwischen Stille, Gleiten ueber eine Oktave,
 * Vibrato, ein Ton mit schwachem Grundton (Oktavfehler), dieselben Zeiten bei 48 kHz, zweimal
 * gleich. Der Speicher: Datei hin und zurueck, je Datei einmal gemessen, ein geaenderter
 * Inhalt misst neu. Die Rechenzeit meldet nur Zahlen.
 *
 * **Messfall an echtem Gesang (GA1 a):** mit `LUMIVIZ_VOCADITO=<Ordner von vocadito>` misst
 * pYIN alle 40 Ausschnitte mit den drei Verteilungen des Papers gegen die beschriftete
 * Tonhoehe und schreibt den Bericht nach
 * `docs/modellierung/wave-clips/proben/S169_pyin_vocadito.md`. Ohne die Variable meldet er
 * das und prueft nichts.
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/WavDatei.hpp"
#include "loop/PYin.hpp"
#include "loop/Tonhoehenspeicher.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTemporaryDir>
#include <QTextStream>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <numbers>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Ein Ton nach einem Verlauf der Frequenz (`hz(t)` in s), drei Teiltoene mit `teile`, in
/// beiden Kanaelen; davor und danach `stille` Sekunden Stille.
std::vector<float> ton(const std::function<double(double)>& hz, double sekunden, int rate = kRate,
                       std::array<double, 3> teile = {1.0, 0.5, 0.3}, double stille = 0.0)
{
    const auto vorn = static_cast<std::int64_t>(stille * rate);
    const auto n = static_cast<std::int64_t>(sekunden * rate);
    std::vector<float> d(static_cast<std::size_t>(2 * (n + 2 * vorn)), 0.0f);
    double phase = 0.0;
    for (std::int64_t i = 0; i < n; ++i)
    {
        phase += 2.0 * std::numbers::pi * hz(static_cast<double>(i) / rate) / rate;
        const double w = 0.3 * (teile[0] * std::sin(phase) + teile[1] * std::sin(2.0 * phase) + teile[2] * std::sin(3.0 * phase));
        d[static_cast<std::size_t>(2 * (vorn + i))] = static_cast<float>(w);
        d[static_cast<std::size_t>(2 * (vorn + i) + 1)] = static_cast<float>(w);
    }
    return d;
}

double cent(double ist, double soll)
{
    return 1200.0 * std::log2(ist / soll);
}

/// Die Fehler in Cent ueber die Rahmen mit Zeit in [von, bis), gegen `soll(t)`; Rahmen ohne Ton zaehlen in `fehlt`.
struct Abweichung
{
    std::vector<double> cent;
    int fehlt = 0;
    [[nodiscard]] double median() const
    {
        std::vector<double> c = cent;
        std::sort(c.begin(), c.end());
        return c.empty() ? 1e9 : c[c.size() / 2];
    }
    [[nodiscard]] double groesste() const { return cent.empty() ? 1e9 : *std::max_element(cent.begin(), cent.end()); }
};

Abweichung abweichung(const Tonhoehenverlauf& v, double von, double bis, const std::function<double(double)>& soll)
{
    Abweichung a;
    for (std::size_t k = 0; k < v.hz.size(); ++k)
    {
        const double t = v.zeitS(k);
        if (t < von || t >= bis) continue;
        if (v.hz[k] <= 0.0f)
            ++a.fehlt;
        else
            a.cent.push_back(std::abs(cent(v.hz[k], soll(t))));
    }
    return a;
}

}  // namespace

TEST_SUITE("LoopPYin")
{
    TEST_CASE("Loop PYin 15g Kandidaten: der Grundton mit der groessten Wahrscheinlichkeit")
    {
        const std::vector<float> stereo = ton([](double) { return 220.0; }, 0.5);
        std::vector<double> mono(stereo.size() / 2);
        for (std::size_t i = 0; i < mono.size(); ++i) mono[i] = stereo[2 * i];
        const std::vector<PYinKandidat> k = pyinKandidaten(mono, kRate, kRate / 4);
        REQUIRE(!k.empty());
        const auto bester = std::max_element(k.begin(), k.end(), [](const auto& a, const auto& b) { return a.p < b.p; });
        CHECK(std::abs(cent(bester->hz, 220.0)) < 2.0);
        double summe = 0.0;
        for (const auto& c : k) summe += c.p;
        CHECK(summe > 0.9);  // ein klarer Ton: fast alles stimmhaft
        CHECK(summe <= 1.0 + 1e-9);
        CHECK(pyinStufeHz(0) == doctest::Approx(55.0));
        CHECK(pyinStufeHz(479) == doctest::Approx(55.0 * std::pow(2.0, 479.0 / 120.0)));
    }

    TEST_CASE("Loop PYin 15g Verlauf: ein gehaltener Ton zwischen Stille")
    {
        const std::vector<float> stereo = ton([](double) { return 196.0; }, 1.0, kRate, {1.0, 0.5, 0.3}, 0.5);
        const Tonhoehenverlauf v = messeTonhoehenverlauf(stereo, kRate);
        CHECK(v.rate == kRate);
        CHECK(v.schritt == 256);
        CHECK(v.hz.size() == static_cast<std::size_t>((stereo.size() / 2 + 255) / 256));
        CHECK(v.zeitS(172) == doctest::Approx(172.0 * 256.0 / kRate));
        // im Ton (ein Rahmen Abstand zu den Raendern): gesungen, auf wenige Cent
        const Abweichung im = abweichung(v, 0.55, 1.45, [](double) { return 196.0; });
        CHECK(im.fehlt == 0);
        CHECK(im.groesste() < 5.0);
        // in der Stille: nicht gesungen
        int laut = 0;
        for (std::size_t k = 0; k < v.hz.size(); ++k)
            if ((v.zeitS(k) < 0.45 || v.zeitS(k) > 1.55) && v.hz[k] > 0.0f) ++laut;
        CHECK(laut == 0);
    }

    TEST_CASE("Loop PYin 15g Gleiten und Vibrato: der Verlauf folgt")
    {
        // eine Oktave in 1 s, gleichmaessig in Cent
        const auto gleiten = [](double t) { return 220.0 * std::pow(2.0, std::clamp(t - 0.25, 0.0, 1.0)); };
        const Tonhoehenverlauf g = messeTonhoehenverlauf(ton(gleiten, 1.5), kRate);
        const Abweichung ag = abweichung(g, 0.1, 1.4, gleiten);
        CHECK(ag.fehlt == 0);
        CHECK(ag.median() < 5.0);
        CHECK(ag.groesste() < 30.0);
        // Vibrato: ±50 Cent mit 5,5 Hz um 330 Hz
        const auto vibrato = [](double t) { return 330.0 * std::pow(2.0, 0.5 / 12.0 * std::sin(2.0 * std::numbers::pi * 5.5 * t)); };
        const Tonhoehenverlauf vb = messeTonhoehenverlauf(ton(vibrato, 1.5), kRate);
        const Abweichung av = abweichung(vb, 0.1, 1.4, vibrato);
        CHECK(av.fehlt == 0);
        CHECK(av.median() < 10.0);
        CHECK(av.groesste() < 40.0);
    }

    TEST_CASE("Loop PYin 15g Oktave: ein schwacher Grundton bleibt der Grundton")
    {
        // der zweite Teilton dreimal so stark wie der erste
        const std::vector<float> stereo = ton([](double) { return 150.0; }, 1.0, kRate, {0.3, 1.0, 0.5});
        const Tonhoehenverlauf v = messeTonhoehenverlauf(stereo, kRate);
        const Abweichung a = abweichung(v, 0.1, 0.9, [](double) { return 150.0; });
        CHECK(a.fehlt == 0);
        CHECK(a.groesste() < 5.0);
    }

    TEST_CASE("Loop PYin 15g Rate und Wiederholung: 48 kHz misst dieselben Zeiten, zweimal gleich")
    {
        const auto gleiten = [](double t) { return 180.0 * std::pow(2.0, 0.5 * t); };
        const Tonhoehenverlauf a = messeTonhoehenverlauf(ton(gleiten, 1.0, 48000), 48000);
        CHECK(a.schritt == 279);  // 256 · 48 000 / 44 100, gerundet
        const Abweichung ab = abweichung(a, 0.1, 0.9, gleiten);
        CHECK(ab.fehlt == 0);
        CHECK(ab.median() < 5.0);
        CHECK(messeTonhoehenverlauf(ton(gleiten, 1.0, 48000), 48000) == a);
        CHECK(messeTonhoehenverlauf({}, kRate).hz.empty());
    }

    TEST_CASE("Loop PYin 15g Speicher: je Datei einmal gemessen, gemerkt unter dem Inhalt")
    {
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        setzeTonhoehenOrdnerFuerTests(ordner.filePath(QStringLiteral("pitch")));
        vergissTonhoehenverlaeufe();
        const QString wav = ordner.filePath(QStringLiteral("stimme.wav"));
        const std::vector<float> stereo = ton([](double) { return 247.0; }, 0.5);
        REQUIRE(lumi::audio::schreibeWav(wav, stereo, kRate, lumi::audio::WavTiefe::Gleitkomma32).geschrieben);
        int gerufen = 0;
        const Dekodierer dekodierer = [&gerufen](const QString& pfad, int, double) {
            ++gerufen;
            const auto inhalt = lumi::audio::liesWav(pfad);
            return inhalt ? inhalt->stereo : std::vector<float>{};
        };
        const auto erst = tonhoehenverlaufVon(wav, dekodierer, kRate);
        REQUIRE(erst.has_value());
        CHECK(gerufen == 1);
        CHECK(*erst == messeTonhoehenverlauf(stereo, kRate));
        // die Datei im Ordner pitch, unter dem Abdruck
        const QByteArray abdruck = tonhoehenAbdruck(wav);
        CHECK(abdruck.size() == 40);
        const QString gemerktDatei = QDir(ordner.filePath(QStringLiteral("pitch"))).filePath(QString::fromLatin1(abdruck) + QStringLiteral(".lvpitch"));
        CHECK(QFileInfo::exists(gemerktDatei));
        CHECK(liesTonhoehenverlauf(gemerktDatei) == erst);
        // noch einmal: aus dem Arbeitsspeicher, dann (nach dem Vergessen) aus der Datei
        CHECK(tonhoehenverlaufVon(wav, dekodierer, kRate) == erst);
        vergissTonhoehenverlaeufe();
        CHECK(tonhoehenverlaufVon(wav, dekodierer, kRate) == erst);
        CHECK(gerufen == 1);
        // umbenannt: derselbe Inhalt, kein neues Messen; geaendert: neu
        const QString anders = ordner.filePath(QStringLiteral("umbenannt.wav"));
        REQUIRE(QFile::copy(wav, anders));
        CHECK(tonhoehenAbdruck(anders) == abdruck);
        REQUIRE(lumi::audio::schreibeWav(wav, ton([](double) { return 330.0; }, 0.5), kRate, lumi::audio::WavTiefe::Gleitkomma32).geschrieben);
        CHECK(tonhoehenAbdruck(wav) != abdruck);
        (void)tonhoehenverlaufVon(wav, dekodierer, kRate);
        CHECK(gerufen == 2);
        // eine kaputte Datei gilt nicht
        QFile kaputt(gemerktDatei);
        REQUIRE(kaputt.open(QIODevice::WriteOnly | QIODevice::Truncate));
        kaputt.write("LVPITCHxx");
        kaputt.close();
        CHECK_FALSE(liesTonhoehenverlauf(gemerktDatei).has_value());
        CHECK_FALSE(tonhoehenverlaufVon(ordner.filePath(QStringLiteral("fehlt.wav")), dekodierer, kRate).has_value());
        vergissTonhoehenverlaeufe();
        setzeTonhoehenOrdnerFuerTests(QString());
    }

    TEST_CASE("Loop PYin 15g Rechenzeit (meldet nur Zahlen)")
    {
        const auto gleiten = [](double t) { return 200.0 * std::pow(2.0, 0.1 * std::sin(t)); };
        const std::vector<float> stereo = ton(gleiten, 30.0);
        const auto anfang = std::chrono::steady_clock::now();
        const Tonhoehenverlauf v = messeTonhoehenverlauf(stereo, kRate);
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - anfang).count();
        CHECK(!v.hz.empty());
        MESSAGE("pYIN: 30 s bei 44,1 kHz in " << ms << " ms (" << ms / 300.0 << " % der Echtzeit)");
    }

    TEST_CASE("Loop PYin 15g Messfall an vocadito (LUMIVIZ_VOCADITO)")
    {
        const QString wurzel = qEnvironmentVariable("LUMIVIZ_VOCADITO");
        if (wurzel.isEmpty())
        {
            MESSAGE("LUMIVIZ_VOCADITO nicht gesetzt — der Messfall an echtem Gesang laeuft nicht");
            return;
        }
        struct Zahl
        {
            long long gesungen = 0, getroffen50 = 0, getroffen100 = 0, erkannt = 0, stumm = 0, fehlalarm = 0, oktave = 0, beide = 0;
        };
        const auto auswerten = [](Zahl& z, const Tonhoehenverlauf& v, const std::vector<double>& soll, int versatz) {
            for (std::size_t k = 0; k < soll.size(); ++k)
            {
                const auto i = static_cast<std::int64_t>(k) + versatz;
                const double ist = i >= 0 && i < static_cast<std::int64_t>(v.hz.size()) ? v.hz[static_cast<std::size_t>(i)] : 0.0;
                if (soll[k] > 0.0)
                {
                    ++z.gesungen;
                    if (ist > 0.0)
                    {
                        ++z.erkannt;
                        ++z.beide;
                        const double c = cent(ist, soll[k]);
                        if (std::abs(c) <= 50.0) ++z.getroffen50;
                        if (std::abs(c) <= 100.0) ++z.getroffen100;
                        const double rest = std::abs(c - 1200.0 * std::round(c / 1200.0));
                        if (std::abs(c) > 600.0 && rest <= 50.0) ++z.oktave;
                    }
                }
                else
                {
                    ++z.stumm;
                    if (ist > 0.0) ++z.fehlalarm;
                }
            }
        };
        const auto anteil = [](long long a, long long b) { return b > 0 ? 100.0 * static_cast<double>(a) / static_cast<double>(b) : 0.0; };
        QString bericht;
        QTextStream b(&bericht);
        b << "# pYIN an vocadito (S169, Stufe 15g, GA1 a)\n\n"
          << "Geschrieben von `test_LoopPYin.cpp` (Fall „Messfall an vocadito“, `LUMIVIZ_VOCADITO`). Quelle: vocadito, "
             "Zenodo 5578807, CC BY 4.0 (`docs/modellierung/wave-clips/daten/Vocadito_Zenodo5578807/HERKUNFT.md`). "
             "Rahmen 2048, Schritt 256 bei 44,1 kHz — dasselbe Raster wie die Beschriftung; Rahmen k ist um k · 256 "
             "zentriert. Gemessen gegen `Annotations/F0` (0 Hz = nicht gesungen).\n\n"
          << "- **RPA 50 / 100:** Anteil der gesungenen Rahmen, die gesungen erkannt sind und auf 50 / 100 Cent stimmen "
             "(100 Cent: das Mass des Papers, „within one semitone“).\n"
          << "- **Erkannt:** gesungen erkannt unter den gesungenen; **Fehlalarm:** gesungen erkannt unter den stummen.\n"
          << "- **Oktave:** unter den beiderseits gesungenen Rahmen der Anteil mit einer ganzen Oktave daneben (±50 Cent).\n\n";
        const std::array<double, 3> mittel{0.10, 0.15, 0.20};
        std::array<Zahl, 3> gesamt{};
        double ms = 0.0;
        double sekunden = 0.0;
        std::array<long long, 7> versatzTreffer{};
        long long versatzGesungen = 0;
        b << "| Ausschnitt | Dauer s | RPA 50 % (0,10 / 0,15 / 0,20) | RPA 100 % | Erkannt % | Fehlalarm % | Oktave % |\n"
          << "|---|---|---|---|---|---|---|\n";
        for (int n = 1; n <= 40; ++n)
        {
            const QString wav = QDir(wurzel).filePath(QStringLiteral("Audio/vocadito_%1.wav").arg(n));
            const auto inhalt = lumi::audio::liesWav(wav);
            REQUIRE_MESSAGE(inhalt.has_value(), wav.toStdString());
            REQUIRE(inhalt->rate == 44100);
            QFile f0(QDir(wurzel).filePath(QStringLiteral("Annotations/F0/vocadito_%1_f0.csv").arg(n)));
            REQUIRE(f0.open(QIODevice::ReadOnly | QIODevice::Text));
            std::vector<double> soll;
            while (!f0.atEnd())
            {
                const QStringList teile = QString::fromUtf8(f0.readLine()).trimmed().split(QLatin1Char(','));
                if (teile.size() == 2) soll.push_back(teile[1].toDouble());
            }
            const double dauer = static_cast<double>(inhalt->stereo.size() / 2) / inhalt->rate;
            sekunden += dauer;
            std::array<Zahl, 3> hier{};
            for (std::size_t m = 0; m < mittel.size(); ++m)
            {
                PYinEinstellungen e;
                e.schwelleMittel = mittel[m];
                const auto anfang = std::chrono::steady_clock::now();
                const Tonhoehenverlauf v = messeTonhoehenverlauf(inhalt->stereo, inhalt->rate, e);
                ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - anfang).count();
                auswerten(hier[m], v, soll, 0);
                auswerten(gesamt[m], v, soll, 0);
                if (m == 0)
                    for (int d = -3; d <= 3; ++d)
                    {
                        Zahl z;
                        auswerten(z, v, soll, d);
                        versatzTreffer[static_cast<std::size_t>(d + 3)] += z.getroffen50;
                        if (d == 0) versatzGesungen += z.gesungen;
                    }
            }
            b << "| " << n << " | " << QString::number(dauer, 'f', 1) << " | "
              << QString::number(anteil(hier[0].getroffen50, hier[0].gesungen), 'f', 1) << " / "
              << QString::number(anteil(hier[1].getroffen50, hier[1].gesungen), 'f', 1) << " / "
              << QString::number(anteil(hier[2].getroffen50, hier[2].gesungen), 'f', 1) << " | "
              << QString::number(anteil(hier[0].getroffen100, hier[0].gesungen), 'f', 1) << " | "
              << QString::number(anteil(hier[0].erkannt, hier[0].gesungen), 'f', 1) << " | "
              << QString::number(anteil(hier[0].fehlalarm, hier[0].stumm), 'f', 1) << " | "
              << QString::number(anteil(hier[0].oktave, hier[0].beide), 'f', 2) << " |\n";
        }
        b << "\n(je Ausschnitt RPA 100, Erkannt, Fehlalarm und Oktave mit Mittelwert 0,10)\n\n## Gesamt\n\n"
          << "| Mittelwert der Schwellen | RPA 50 % | RPA 100 % | Erkannt % | Fehlalarm % | Oktave % | Praezision % | F % |\n"
          << "|---|---|---|---|---|---|---|---|\n";
        for (std::size_t m = 0; m < mittel.size(); ++m)
        {
            const Zahl& z = gesamt[m];
            // wie im Paper (§3.1, Mass 100 Cent): Praezision = richtig unter allen als gesungen erkannten, F = 2PR / (P + R)
            const double praezision = anteil(z.getroffen100, z.erkannt + z.fehlalarm);
            const double recall = anteil(z.getroffen100, z.gesungen);
            const double f = praezision + recall > 0.0 ? 2.0 * praezision * recall / (praezision + recall) : 0.0;
            b << "| " << QString::number(mittel[m], 'f', 2) << " | " << QString::number(anteil(z.getroffen50, z.gesungen), 'f', 2) << " | "
              << QString::number(recall, 'f', 2) << " | " << QString::number(anteil(z.erkannt, z.gesungen), 'f', 2)
              << " | " << QString::number(anteil(z.fehlalarm, z.stumm), 'f', 2) << " | " << QString::number(anteil(z.oktave, z.beide), 'f', 3)
              << " | " << QString::number(praezision, 'f', 2) << " | " << QString::number(f, 'f', 2) << " |\n";
        }
        b << "\nRahmen gesungen / stumm laut Beschriftung: " << gesamt[0].gesungen << " / " << gesamt[0].stumm << "\n";
        b << "\n## Versatz der Rahmen (Gegenprobe der Zeiten, Mittelwert 0,10)\n\nRPA 50 %, wenn die Messung um d Rahmen "
             "gegen die Beschriftung verschoben wird — der Gipfel muss bei 0 liegen:\n\n| d | ";
        for (int d = -3; d <= 3; ++d) b << d << " | ";
        b << "\n|---|---|---|---|---|---|---|---|\n| RPA 50 % | ";
        for (int d = -3; d <= 3; ++d) b << QString::number(anteil(versatzTreffer[static_cast<std::size_t>(d + 3)], versatzGesungen), 'f', 2) << " | ";
        b << "\n\n**Rechenzeit:** " << QString::number(ms / 1000.0, 'f', 1) << " s fuer 3 × " << QString::number(sekunden, 'f', 1)
          << " s Ton (" << QString::number(ms / (30.0 * sekunden), 'f', 2) << " % der Echtzeit je Messung), Build siehe Lauf.\n";
        b.flush();

        const QString ziel = QDir(QFileInfo(QString::fromLocal8Bit(__FILE__)).absolutePath())
                                 .filePath(QStringLiteral("../../../docs/modellierung/wave-clips/proben/S169_pyin_vocadito.md"));
        QDir().mkpath(QFileInfo(ziel).absolutePath());
        QFile aus(ziel);
        REQUIRE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
        aus.write(bericht.toUtf8());
        MESSAGE("Bericht: " << QFileInfo(ziel).canonicalFilePath().toStdString());
        MESSAGE(bericht.section(QStringLiteral("## Gesamt"), 1).toStdString());
        CHECK(gesamt[0].gesungen > 0);
    }
}
