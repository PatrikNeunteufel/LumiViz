/**
 * @file glaette_ziel.cpp
 * @brief Werkzeug (IC-A4, `gemeinsam/messkette.md` §3b): die Glaette des **Ziels** am Modell, ohne Aufnahmen — je
 *        heiklem Mechanismus einer Klasse ein synthetisches Ziel vom Modell selbst (die Vorgabe), dann der Parameter, der
 *        die Rate oder Frequenz verschiebt, ueber seinen Bereich; je Messart `welligkeit` (innere Minima je Groesse, 41
 *        Punkte) und `glaette` (Σ r² nach ZF8 a (2), 15 Punkte).
 *
 *   ic_glaette_ziel [Ausgabedatei]
 *
 * Zwei Messarten je Fall:
 * - **fest** (§3a, wie beim Fluegel): die Amplitude bei der **festen** Frequenz des Ziels (`amplitudenBei`, Hann 20 ms)
 *   alle 10 ms — je Zeitpunkt eine Groesse in dB, Toleranz 0,5 dB (UG1 a). Fenster und Schritt so, dass eine AM bis
 *   24 Hz (Vibraphon 10 U/s = 20 Hz) unter Nyquist liegt und das Fenster sie nicht wegmittelt (sinc(π·20·0,02) = 0,76).
 * - **suche** (§3b (2)): Rate und Tiefe der AM mit eigener Suche der Frequenz auf derselben Huelle (`amRate`, ohne
 *   linearen Trend, 0,3–24 Hz) — zwei Groessen, Toleranz 2 % bzw. 0,5 dB.
 * Ist „fest“ wellig und „suche“ glatt, ist das die Aenderung der Messgroesse nach §3b (2), nicht der Suche.
 */
#include "klassen/01_hammer_saite/Saite.hpp"
#include "klassen/02_zupf_saite/Zupf.hpp"
#include "klassen/03_bogen_saite/Bogen.hpp"
#include "klassen/04_platte_stab/Stab.hpp"
#include "klassen/05_membran/Trommel.hpp"
#include "klassen/07_lippe_rohr/Blech.hpp"
#include "klassen/08_rohrblatt_rohr/Rohrblatt.hpp"
#include "klassen/09_luftstrahl_rohr/Luftstrahl.hpp"
#include "klassen/10_zunge/Zunge.hpp"
#include "klassen/11_tonrad/Orgel.hpp"
#include "messung/Bausteine.hpp"
#include "setzen/Setzen.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

using namespace ic;

namespace
{

constexpr int kRate = 48000;
constexpr double kFensterS = 0.02;
constexpr double kSchrittS = 0.01;

/// Ein Fall: ein Mechanismus, ein Parameter, wie der Ton entsteht.
struct Fall
{
    std::string klasse, mechanismus, parameter;
    double wahr = 0.0, unten = 0.0, oben = 1.0;
    setzen::Skala skala = setzen::Skala::Linear;
    double vonS = 0.2, bisS = 2.8;                      ///< Bereich der Messung
    std::function<std::vector<double>(double)> ton;     ///< der Ton beim Wert des Parameters
    double hz = 0.0;                                    ///< die feste Frequenz des Ziels
    bool fm = false;  ///< Vibrato der Spieler: „suche“ ueber die Tonhoehe (B2, B3) statt ueber die Huelle
};

std::vector<double> zeiten(const Fall& f)
{
    std::vector<double> t;
    for (double s = f.vonS; s <= f.bisS + 1e-9; s += kSchrittS) t.push_back(s);
    return t;
}

std::vector<double> fest(const Fall& f, const std::vector<double>& signal)
{
    return messung::amplitudenBei(signal, kRate, f.hz, zeiten(f), kFensterS);
}

/// Rate und Tiefe einer AM mit eigener Suche der Frequenz (§3b (2)): die Huelle in dB ohne ihren linearen Trend (das
/// Abklingen), Hann, die staerkste Periodizitaet in [von, bis] auf 0,01 Hz; Tiefe Spitze-Spitze in dB, um die Daempfung
/// des Messfensters sinc(π·Rate·Fenster) berichtigt. (B3 `vibrato` sucht die Rate in der FM — fuer AM-Mechanismen wie
/// Tremolo, Musette, Rotary und Schwebung der falsche Traeger; Befund IC-A4.)
std::vector<double> amRate(const std::vector<double>& huelleDb, double vonHz, double bisHz)
{
    const std::size_t n = huelleDb.size();
    double st = 0.0, sy = 0.0, stt = 0.0, sty = 0.0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const double t = static_cast<double>(i);
        st += t;
        sy += huelleDb[i];
        stt += t * t;
        sty += t * huelleDb[i];
    }
    const double steigung = (n * sty - st * sy) / (n * stt - st * st);
    const double achse = (sy - steigung * st) / n;
    std::vector<double> rest(n);
    for (std::size_t i = 0; i < n; ++i) rest[i] = huelleDb[i] - (achse + steigung * static_cast<double>(i));
    double besteF = vonHz, besteA = -1.0;
    for (double f = vonHz; f <= bisHz + 1e-9; f += 0.01)
    {
        std::complex<double> s(0.0, 0.0);
        double wsum = 0.0;
        for (std::size_t i = 0; i < n; ++i)
        {
            const double w = 0.5 - 0.5 * std::cos(2.0 * 3.141592653589793 * (i + 0.5) / n);
            s += w * rest[i] * std::exp(std::complex<double>(0.0, -2.0 * 3.141592653589793 * f * static_cast<double>(i) * kSchrittS));
            wsum += w;
        }
        const double a = 2.0 * std::abs(s) / wsum;
        if (a > besteA)
        {
            besteA = a;
            besteF = f;
        }
    }
    const double x = 3.141592653589793 * besteF * kFensterS;
    return {besteF, 2.0 * besteA / (std::sin(x) / x)};
}

/// Rate und Tiefe einer FM (Vibrato der Spieler): f0 mit B2 (YIN, 40 ms, Schritt 5 ms) im Bereich der Messung, dann B3.
std::vector<double> fmRate(const Fall& f, const std::vector<double>& signal)
{
    constexpr double schritt = 0.005;
    const messung::Tonhoehe t = messung::tonhoehe(signal, kRate, 0.8 * f.hz, 1.25 * f.hz, 0.04, schritt);
    std::vector<double> f0;
    for (std::size_t i = 0; i < t.hz.size(); ++i)
    {
        const double s = static_cast<double>(i) * schritt;
        if (s >= f.vonS && s <= f.bisS) f0.push_back(t.hz[i]);
    }
    const messung::Vibrato v = messung::vibrato(f0, {}, schritt, 3.0, 10.0, 0.04);
    return {v.rateHz, v.tiefeCent};
}

std::vector<double> suche(const Fall& f, const std::vector<double>& signal)
{
    return f.fm ? fmRate(f, signal) : amRate(fest(f, signal), 0.3, 24.0);
}

struct Ergebnis
{
    std::string messart;
    int groessen = 0, wellig = 0, minimaHoechstens = 0;
    setzen::Glaette g;
    std::vector<double> ziel;
    std::vector<std::array<double, 3>> schnitt;  ///< nur „suche“: Wert, Rate, Tiefe je Punkt
};

Ergebnis pruefe(const Fall& f, bool mitSuche)
{
    const auto messe = [&](double wert) {
        const std::vector<double> s = f.ton(wert);
        return mitSuche ? suche(f, s) : fest(f, s);
    };
    Ergebnis e;
    e.messart = mitSuche ? "suche" : "fest";
    e.ziel = messe(f.wahr);
    setzen::Gruppe g;
    g.name = f.mechanismus;
    g.parameter = {{f.parameter, f.wahr, f.unten, f.oben, f.skala}};
    for (std::size_t i = 0; i < e.ziel.size(); ++i)
    {
        setzen::Messgroesse m;
        m.name = (mitSuche ? (i == 0 ? std::string("rate_hz") : std::string(f.fm ? "tiefe_cent" : "tiefe_db")) : "pegel_t" + std::to_string(i));
        m.ziel = e.ziel[i];
        // Rate 2 %; Tiefe der AM 0,5 dB (UG1 a), der FM 2 % (wie die Schwebung, 31d-1)
        m.toleranz = mitSuche && (i == 0 || f.fm) ? 0.02 * std::max(std::abs(e.ziel[i]), 1e-3) : 0.5;
        g.ziele.push_back(m);
    }
    const setzen::Modell modell = [&](std::size_t, const std::vector<double>& w) { return messe(w[0]); };
    const std::vector<int> minima = setzen::welligkeit(g, 0, 0, modell, 41);
    e.groessen = static_cast<int>(minima.size());
    for (const int m : minima)
    {
        e.wellig += m > 1 ? 1 : 0;
        e.minimaHoechstens = std::max(e.minimaHoechstens, m);
    }
    e.g = setzen::glaette(g, 0, 0, modell, 15);
    if (mitSuche)
        for (const auto& [x, m] : setzen::schnitt(g, 0, 0, modell, 15)) e.schnitt.push_back({x, m[0], m[1]});
    return e;
}

std::vector<double> nachDouble(const std::vector<float>& v)
{
    return std::vector<double>(v.begin(), v.end());
}

std::vector<Fall> faelle()
{
    std::vector<Fall> aus;
    // Klasse 4: Vibraphon-Motor — die Drehzahl verschiebt die Rate des Tremolos (Vorgabe der Hoerproben 2 und 6 U/s)
    {
        Fall f;
        f.klasse = "4 Platte und Stab";
        f.mechanismus = "Vibraphon F4, Motor";
        f.parameter = "drehzahl (U/s)";
        f.wahr = 4.0;
        f.unten = 1.0;
        f.oben = 10.0;
        const stab::StabSpiel s = stab::baueVorgabeVibraphon();
        stab::StabAnschlag a;
        a.note = 65;
        a.hz = 440.0 * std::exp2((65 - 69) / 12.0);
        a.stufe = -12;
        f.hz = stab::stabModen(s, a).stab.front().hz;
        f.ton = [s, a](double drehzahl) {
            stab::StabAnschlag b = a;
            b.motor = {{-1.0, drehzahl}};
            std::vector<float> m;
            stab::rechneStabTon(s, b, m, 3 * kRate);
            return nachDouble(m);
        };
        aus.push_back(f);
    }
    // Klasse 5: Tom — das Kesselvolumen verschiebt die Kesselmode gegen die Fellmode (Aufspaltung, Schwebung)
    {
        Fall f;
        f.klasse = "5 Membran";
        f.mechanismus = "Tom, Fell gegen Kessel";
        f.parameter = "Kesselvolumen (Faktor auf die Vorgabe)";
        f.wahr = 1.0;
        f.unten = 0.5;
        f.oben = 2.0;
        f.skala = setzen::Skala::Log;
        f.bisS = 1.5;
        trommel::TrommelSpiel s = trommel::baueVorgabeTom();
        trommel::bereite(s);
        f.hz = s.moden->front().hz;
        const double v0 = s.kessel.volumenM3;
        f.ton = [s, v0](double faktor) {
            trommel::TrommelSpiel t = s;
            t.kessel.volumenM3 = v0 * faktor;
            trommel::bereite(t);
            trommel::TrommelAnschlag a;
            std::vector<float> m;
            trommel::rechneTrommelTon(t, a, m, 2 * kRate);
            return nachDouble(m);
        };
        aus.push_back(f);
    }
    // Klasse 10: Akkordeon — die Verstimmung des Musette-Chors (Schwebung an A4) traegt das Tremolo
    {
        Fall f;
        f.klasse = "10 Durchschlagende Zunge";
        f.mechanismus = "Akkordeon A4, Musette";
        f.parameter = "Schwebung an A4 (Hz)";
        f.wahr = 2.0;
        f.unten = 0.5;
        f.oben = 8.0;
        f.vonS = 0.5;
        const zunge::ZungenSpiel s = zunge::baueVorgabeAkkordeon();
        f.hz = 440.0;
        f.ton = [s](double hz) {
            zunge::ZungenSpiel t = s;
            for (zunge::Chor& c : t.choere)
                if (c.schwebungA4Hz != 0.0) c.schwebungA4Hz = hz;
            zunge::ZungenAnschlag a;
            a.note = 69;
            a.haltenS = 3.0;
            std::vector<float> m;
            zunge::rechneZungenTon(t, a, m, 3 * kRate);
            return nachDouble(m);
        };
        aus.push_back(f);
    }
    // Klasse 11: Tonradorgel — die Drehzahl des Horns (langsam) traegt die Modulation des Rotary
    // Die Weiche liegt bei 800 Hz (Orgel.cpp): A4 laeuft ueber die Trommel, A6 ueber das Horn (Fehlanlage im ersten
    // Lauf: A4 mit dem Horn als Parameter — die Rate blieb bei 40 U/min = 0,67 Hz, der Trommel)
    for (const bool horn : {false, true})
    {
        Fall f;
        f.klasse = "11 Tonrad";
        f.mechanismus = horn ? "Orgel A6, Rotary langsam, Horn" : "Orgel A4, Rotary langsam, Trommel";
        f.parameter = horn ? "Horn langsam (U/min)" : "Trommel langsam (U/min)";
        f.wahr = horn ? 48.0 : 40.0;
        f.unten = 20.0;
        f.oben = 120.0;
        f.vonS = 0.3;
        f.bisS = 7.8;  // langsam 0,3–2 U/s: erst ueber mehrere Sekunden aufgeloest
        const tonrad::OrgelSpiel s = tonrad::baueVorgabeOrgel();
        const int note = horn ? 93 : 69;
        f.hz = 440.0 * std::exp2((note - 69) / 12.0);
        f.ton = [s, horn, note](double upm) {
            tonrad::OrgelSpiel t = s;
            (horn ? t.hornLangsamUpm : t.trommelLangsamUpm) = upm;
            std::vector<tonrad::OrgelNote> n = {{0.0, 8.0, note, 0.7}};
            std::vector<float> m;
            tonrad::rechneOrgel(t, n, tonrad::OrgelSpur{}, m, 8 * kRate);
            return nachDouble(m);
        };
        aus.push_back(f);
    }
    // Klasse 1: Hackbrett C4 — die Verstimmung im Chor traegt die Schwebung (mit der Polarisation, wie beim Fluegel)
    {
        Fall f;
        f.klasse = "1 Hammer-Saite";
        f.mechanismus = "Hackbrett C4, Chor";
        f.parameter = "Verstimmung (Cent)";
        f.wahr = 0.3;  // Vorgabe (offen)
        f.unten = 0.05;
        f.oben = 4.0;
        f.skala = setzen::Skala::Log;
        f.hz = 261.63;
        const saite::SaitenSpiel s = saite::baueVorgabeHackbrett();
        f.ton = [s, hz = f.hz](double cent) {
            saite::SaitenSpiel t = s;
            t.verstimmungCent = kurveAusPunkten({{60, cent}});
            saite::SaitenAnschlag a;
            a.note = 60;
            a.hz = hz;
            a.stufe = -12;
            a.halten = 3 * kRate;
            std::vector<float> m;
            saite::rechneSaitenTon(t, a, m, 3 * kRate);
            return nachDouble(m);
        };
        aus.push_back(f);
    }
    // Klasse 2: Laute A3 — die Verstimmung im Doppelchor
    {
        Fall f;
        f.klasse = "2 Gezupfte Saite";
        f.mechanismus = "Laute A3, Doppelchor";
        f.parameter = "Verstimmung (Cent)";
        f.wahr = 4.0;  // Vorgabe (offen)
        f.unten = 0.5;
        f.oben = 20.0;
        f.skala = setzen::Skala::Log;
        f.hz = 220.0;
        const zupf::ZupfSpiel s = zupf::baueVorgabeLaute();
        f.ton = [s](double cent) {
            zupf::ZupfSpiel t = s;
            t.chor.verstimmungCent = cent;
            zupf::ZupfAnschlag a;
            a.note = 57;
            a.haltenS = 3.0;
            std::vector<float> m;
            zupf::rechneZupfTon(t, a, m, 3 * kRate);
            return nachDouble(m);
        };
        aus.push_back(f);
    }
    // Klassen 3, 7, 8, 9: das Vibrato der Spieler (Rate; Tiefe 25 Cent fest)
    const auto vibrato = [&](const std::string& klasse, const std::string& name, int note, double wahr,
                             std::function<std::vector<double>(double, double)> ton) {
        Fall f;
        f.klasse = klasse;
        f.mechanismus = name + ", Vibrato der Spieler";
        f.parameter = "Vibrato-Rate (Hz)";
        f.wahr = wahr;
        f.unten = 3.0;
        f.oben = 8.0;
        f.vonS = 0.5;
        f.hz = 440.0 * std::exp2((note - 69) / 12.0);
        f.fm = true;
        f.ton = [ton, hz = f.hz](double rate) { return ton(hz, rate); };
        aus.push_back(f);
    };
    vibrato("3 Bogen-Saite", "Violine E5", 76, 5.5, [s = bogen::baueVorgabeVioline()](double hz, double rate) {
        bogen::StreichAnschlag a;
        a.kette = {{0.0, 76, hz}};
        a.haltenS = 3.0;
        a.vibratoCent = 25.0;
        a.vibratoHz = rate;
        std::vector<float> m;
        bogen::rechneStreichTon(s, a, m, 3 * kRate);
        return nachDouble(m);
    });
    vibrato("7 Lippe und Rohr", "Posaune B3", 58, 5.0, [s = blech::baueVorgabePosaune()](double hz, double rate) {
        blech::BlechAnschlag a;
        a.kette = {{0.0, 58, hz}};
        a.haltenS = 3.0;
        a.vibratoCent = 25.0;
        a.vibratoHz = rate;
        std::vector<float> m;
        blech::rechneBlechTon(s, a, m, 3 * kRate);
        return nachDouble(m);
    });
    vibrato("8 Rohrblatt und Rohr", "Klarinette C4", 60, 5.0, [s = rohrblatt::baueVorgabeKlarinette()](double hz, double rate) {
        rohrblatt::RohrblattAnschlag a;
        a.kette = {{0.0, 60, hz}};
        a.haltenS = 3.0;
        a.vibratoCent = 25.0;
        a.vibratoHz = rate;
        std::vector<float> m;
        rohrblatt::rechneRohrblattTon(s, a, m, 3 * kRate);
        return nachDouble(m);
    });
    vibrato("9 Luftstrahl und Rohr", "Blockfloete D5", 74, 5.0, [s = luftstrahl::baueVorgabeBlockfloete()](double hz, double rate) {
        luftstrahl::StrahlAnschlag a;
        a.kette = {{0.0, 74, hz}};
        a.haltenS = 3.0;
        a.vibratoCent = 25.0;
        a.vibratoHz = rate;
        std::vector<float> m;
        luftstrahl::rechneStrahlTon(s, a, m, 3 * kRate);
        return nachDouble(m);
    });
    return aus;
}

}  // namespace

int main(int argc, char** argv)
{
    std::ostringstream b;
    b << "# Glaette des Ziels am Modell (IC-A4, messkette.md §3b) — ic_glaette_ziel\n";
    b << "# fest: Amplitude bei der festen Frequenz des Ziels alle 10 ms (Hann 20 ms, Toleranz 0,5 dB); suche: Rate und Tiefe"
         " der AM (amRate) (Toleranz 2 % / 0,5 dB)\n";
    b << "# Spalten: Klasse | Mechanismus | Parameter (wahr; Bereich) | Messart | Groessen | davon wellig (> 1 Minimum, 41"
         " Punkte) | Minima hoechstens | Σ r²: Minima, groesster/mittlerer Schritt, glatt nach ZF8 a (2)\n";
    for (const Fall& f : faelle())
        for (const bool mitSuche : {false, true})
        {
            const Ergebnis e = pruefe(f, mitSuche);
            char z[512];
            std::snprintf(z, sizeof z, "%s | %s | %s (%g; %g–%g) | %s | %d | %d | %d | %d, %.3g/%.3g, %s", f.klasse.c_str(), f.mechanismus.c_str(),
                          f.parameter.c_str(), f.wahr, f.unten, f.oben, e.messart.c_str(), e.groessen, e.wellig, e.minimaHoechstens, e.g.minima,
                          e.g.groessterSchritt, e.g.mittlererSchritt, e.g.glatt() ? "glatt" : "NICHT glatt");
            b << z;
            if (mitSuche)
            {
                b << " | Ziel: Rate " << e.ziel[0] << " Hz, Tiefe " << e.ziel[1] << " dB\n    Schnitt (Wert: Rate/Tiefe):";
                for (const auto& p : e.schnitt)
                {
                    char s[64];
                    std::snprintf(s, sizeof s, " %.3g: %.3g/%.3g", p[0], p[1], p[2]);
                    b << s;
                }
            }
            b << '\n';
            std::printf("%s", z);
            std::printf("\n");
            std::fflush(stdout);
        }
    if (argc > 1)
    {
        std::ofstream aus(argv[1]);
        aus << b.str();
    }
    return 0;
}
