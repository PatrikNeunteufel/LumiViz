// S195, Stufe 34k-4 (Ziele): Messung an den Aufnahmen der Klasse 2 aus dem Archiv — VCSL (Konzertharfe, Volksharfe,
// fuenf Cembali; WAV, raumbehaftet) und Iowa MIS (Nylongitarre, schalltot, pp/mf/ff je Saite; AIFF mit chromatischen
// Laeufen). Nichts im Repo geaendert. Bau: 2026-11-12_S195_bau.bat. Aufruf:
//
//   ziele.exe "<…\Klasse 2 - Gezupfte Saite>"  > 2026-11-12_S195_ziele_aufnahmen.txt
//
// Messkette: die Lautheit aus `AudioCore/Lautheit` (EBU R128), dieselbe wie im Waechter `Loop Pegel LP3 a` — dort das
// Momentary-Maximum (0,4 s) eines Tons. Je Ton:
//   mom     Momentary-Maximum in LUFS (die Groesse von LP3 a)
//   int1    integrierte Lautheit der ersten Sekunde ab dem Einsatz
//   spitze  Sample-Spitze in dBFS (zeigt, ob eine Sammlung je Datei normiert ist)
//   frueh   Abfall in dB/s, Gerade durch die 20-ms-Bloecke zwischen −5 und −25 dB unter dem lautesten Block
//   spaet   dasselbe zwischen −25 und −45 dB (zweistufiges Abklingen: spaet deutlich kleiner als frueh)
//   t20,t60 Zeit ab dem lautesten Block bis zum letzten Block ueber −20 / −60 dB (wie S192 `abfallS`)
//   boden   leisester 20-ms-Block gegen den lautesten, in dB — t60 gilt nur, wenn der Boden unter −60 dB liegt
//   okt     Oktavlage des gemessenen Grundtons gegen den Namen der Datei bei C4 = 60 (nur Pruefung der Benennung;
//           Autokorrelation 0,05–0,45 s nach dem Einsatz, Kandidaten Name · 2^k, k = −1, 0, 1)
// In den Zeilen je Ton steht die Note nach dem Namen der Datei (C4 = 60); die Zusammenfassung rechnet mit der um die
// gemessene Oktavlage korrigierten Note. Iowa: die Toene eines Laufs sind durch digitale Stille getrennt; passt die Zahl
// der Stuecke zur Zahl der Noten im Namen, gilt die Reihenfolge, sonst bleibt der Lauf aus der Zusammenfassung.
// Die absoluten Pegel der Aufnahmen und des Modells sind nicht vergleichbar, nur Spannen, Spruenge und Unterschiede.
#include "AudioCore/Lautheit.hpp"
#include "AudioCore/WavFormat.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using lumi::audio::messeLautheit;

namespace
{

struct Ton
{
    std::vector<float> stereo;
    int rate = 0;
};

double hzVon(double note)
{
    return 440.0 * std::exp2((note - 69.0) / 12.0);
}

std::vector<double> monoVon(const std::vector<float>& stereo)
{
    std::vector<double> m(stereo.size() / 2);
    for (std::size_t i = 0; i < m.size(); ++i) m[i] = 0.5 * (double(stereo[2 * i]) + double(stereo[2 * i + 1]));
    return m;
}

/// Energie je Block in dB.
std::vector<double> bloeckeDb(const std::vector<double>& m, std::size_t b)
{
    std::vector<double> e;
    for (std::size_t i = 0; i + b <= m.size(); i += b)
    {
        double s = 0.0;
        for (std::size_t k = 0; k < b; ++k) s += m[i + k] * m[i + k];
        e.push_back(10.0 * std::log10(s / double(b) + 1e-30));
    }
    return e;
}

/// Steigung (dB/s, positiv = faellt) der Geraden durch die Bloecke nach dem lautesten, deren Pegel zwischen `oben` und
/// `unten` dB unter ihm liegt — bis zum ersten Block unter `unten`. 0, wenn weniger als vier Bloecke.
double abfallDbS(const std::vector<double>& e, double oben, double unten, double blockS)
{
    if (e.empty()) return 0.0;
    const std::size_t m = static_cast<std::size_t>(std::max_element(e.begin(), e.end()) - e.begin());
    std::vector<double> x, y;
    for (std::size_t i = m; i < e.size(); ++i)
    {
        const double d = e[i] - e[m];
        if (d < unten) break;
        if (d <= oben)
        {
            x.push_back(double(i - m) * blockS);
            y.push_back(d);
        }
    }
    if (x.size() < 4) return 0.0;
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (std::size_t i = 0; i < x.size(); ++i) sx += x[i], sy += y[i], sxx += x[i] * x[i], sxy += x[i] * y[i];
    const double n = double(x.size());
    const double nenner = n * sxx - sx * sx;
    return nenner > 0.0 ? -(n * sxy - sx * sy) / nenner : 0.0;
}

double zeitBis(const std::vector<double>& e, double db, double blockS)
{
    if (e.empty()) return 0.0;
    const std::size_t m = static_cast<std::size_t>(std::max_element(e.begin(), e.end()) - e.begin());
    std::size_t letzter = m;
    for (std::size_t i = m; i < e.size(); ++i)
        if (e[i] > e[m] + db) letzter = i;
    return double(letzter - m) * blockS;
}

/// Oktavlage gegen den Namen: k aus {−1, 0, 1} mit der hoechsten Frequenz, deren normierte Autokorrelation mindestens
/// 0,85 der besten erreicht; 9 = nicht messbar.
int oktaveGegenName(const std::vector<double>& m, int rate, std::size_t einsatz, double hzName)
{
    const std::size_t ab = einsatz + static_cast<std::size_t>(0.05 * rate);
    const std::size_t n = static_cast<std::size_t>(0.4 * rate);
    if (ab + 2 * n >= m.size()) return 9;
    double wert[3] = {0, 0, 0};
    for (int k = -1; k <= 1; ++k)
    {
        const double f = hzName * std::exp2(k);
        const int von = static_cast<int>(rate / (f * 1.035)), bis = static_cast<int>(rate / (f / 1.035)) + 1;
        if (bis >= static_cast<int>(n)) continue;
        double best = 0.0;
        for (int lag = std::max(2, von); lag <= bis; ++lag)
        {
            double xy = 0, xx = 0, yy = 0;
            for (std::size_t i = 0; i < n; ++i)
            {
                const double a = m[ab + i], b = m[ab + i + static_cast<std::size_t>(lag)];
                xy += a * b, xx += a * a, yy += b * b;
            }
            if (xx > 0 && yy > 0) best = std::max(best, xy / std::sqrt(xx * yy));
        }
        wert[k + 1] = best;
    }
    const double beste = std::max({wert[0], wert[1], wert[2]});
    if (beste < 0.3) return 9;
    for (int k = 1; k >= -1; --k)
        if (wert[k + 1] >= 0.85 * beste) return k;
    return 9;
}

/// Die Note aus `von … bis` (hoechstens eine Oktave, also ohne Oktavzweifel), deren Periode die hoechste normierte
/// Autokorrelation gibt (0,05–0,45 s nach dem Einsatz, ±1,7 % um die Periode); −1 = nicht messbar. Fuer die Iowa-Laeufe,
/// wo die Zahl der Stuecke nicht zur Zahl der Noten passt.
int noteAusKlang(const std::vector<double>& m, int rate, int von, int bis)
{
    const std::size_t ab = static_cast<std::size_t>(0.05 * rate), n = static_cast<std::size_t>(0.4 * rate);
    int beste = -1;
    double wert = 0.3;
    for (int note = von; note <= bis; ++note)
    {
        const double f = hzVon(note);
        const int l0 = static_cast<int>(rate / (f * 1.017)), l1 = static_cast<int>(rate / (f / 1.017)) + 1;
        if (ab + n + static_cast<std::size_t>(l1) >= m.size()) continue;
        for (int lag = std::max(2, l0); lag <= l1; ++lag)
        {
            double xy = 0, xx = 0, yy = 0;
            for (std::size_t i = 0; i < n; i += 2)
            {
                const double a = m[ab + i], b = m[ab + i + static_cast<std::size_t>(lag)];
                xy += a * b, xx += a * a, yy += b * b;
            }
            const double r = xx > 0 && yy > 0 ? xy / std::sqrt(xx * yy) : 0.0;
            if (r > wert) wert = r, beste = note;
        }
    }
    return beste;
}

struct Werte
{
    double mom = -200, int1 = -200, spitze = -200, frueh = 0, spaet = 0, t20 = 0, t60 = 0, boden = 0, dauer = 0;
    int okt = 9;
};

Werte messe(const Ton& t, double hzName)
{
    Werte w;
    const std::vector<double> m = monoVon(t.stereo);
    w.dauer = double(m.size()) / t.rate;
    const std::size_t b = static_cast<std::size_t>(t.rate / 50);
    const std::vector<double> e = bloeckeDb(m, b);
    if (e.empty()) return w;
    const auto maxIt = std::max_element(e.begin(), e.end());
    const std::size_t mx = static_cast<std::size_t>(maxIt - e.begin());
    // Einsatz: der erste Block ab 30 dB unter dem lautesten
    std::size_t einsatzBlock = 0;
    for (std::size_t i = 0; i <= mx; ++i)
        if (e[i] > *maxIt - 30.0)
        {
            einsatzBlock = i;
            break;
        }
    const std::size_t einsatz = einsatzBlock * b;
    const lumi::audio::Lautheit ganz = messeLautheit(t.stereo, t.rate);
    w.mom = ganz.momentanMaxLufs;
    w.spitze = ganz.spitzeDb;
    const std::size_t bis = std::min(m.size(), einsatz + static_cast<std::size_t>(t.rate));
    w.int1 = messeLautheit(std::vector<float>(t.stereo.begin() + static_cast<std::ptrdiff_t>(2 * einsatz), t.stereo.begin() + static_cast<std::ptrdiff_t>(2 * bis)), t.rate).lufs;
    w.frueh = abfallDbS(e, -5.0, -25.0, 0.02);
    w.spaet = abfallDbS(e, -25.0, -45.0, 0.02);
    w.t20 = zeitBis(e, -20.0, 0.02);
    w.t60 = zeitBis(e, -60.0, 0.02);
    w.boden = *std::min_element(e.begin() + static_cast<std::ptrdiff_t>(mx), e.end()) - *maxIt;
    w.okt = oktaveGegenName(m, t.rate, einsatz, hzName);
    return w;
}

/// Note (C4 = 60) aus einem Namen wie „A#2“, „Db5“, „C4“; −1 = keiner.
int noteAus(const std::string& name)
{
    static const std::regex re("^([A-G])([#b]?)(-?[0-9])$");
    std::smatch m;
    if (!std::regex_match(name, m, re)) return -1;
    static const int halb[7] = {9, 11, 0, 2, 4, 5, 7};  // A B C D E F G
    int n = halb[m[1].str()[0] - 'A'] + 12 * (std::stoi(m[3].str()) + 1);
    if (m[2] == "#") ++n;
    if (m[2] == "b") --n;
    return n;
}

struct Zeile
{
    std::string gruppe, variante, datei;
    int note = -1;
    bool unsicher = false;  ///< Iowa: die Zahl der Stuecke passt nicht zur Zahl der Noten — nicht in der Zusammenfassung
    Werte w;
};

double perzentil(std::vector<double> v, double p)
{
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    return v[std::min(v.size() - 1, static_cast<std::size_t>(p * double(v.size())))];
}

void drucke(const Zeile& z)
{
    std::printf("%s\t%s\t%d\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.2f\t%.2f\t%.0f\t%.2f\t%d\t%s\n", z.gruppe.c_str(), z.variante.c_str(), z.note, z.w.mom,
                z.w.int1, z.w.spitze, z.w.frueh, z.w.spaet, z.w.t20, z.w.t60, z.w.boden, z.w.dauer, z.w.okt, z.datei.c_str());
}

// ---- AIFF (Iowa MIS): FORM/AIFF oder AIFC ohne Kompression, gross-endig ------------------------------------------

std::uint32_t be32(const unsigned char* p)
{
    return (std::uint32_t(p[0]) << 24) | (std::uint32_t(p[1]) << 16) | (std::uint32_t(p[2]) << 8) | p[3];
}

double extended80(const unsigned char* p)
{
    const int exponent = ((p[0] & 0x7F) << 8) | p[1];
    std::uint64_t mant = 0;
    for (int i = 0; i < 8; ++i) mant = (mant << 8) | p[2 + i];
    return std::ldexp(double(mant), exponent - 16383 - 63);
}

std::optional<Ton> liesAiff(const fs::path& pfad, std::string& grund)
{
    std::ifstream f(pfad, std::ios::binary);
    std::vector<unsigned char> roh((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (roh.size() < 12 || std::string(roh.begin(), roh.begin() + 4) != "FORM")
    {
        grund = "kein FORM";
        return std::nullopt;
    }
    const std::string art(roh.begin() + 8, roh.begin() + 12);
    int kanaele = 0, bits = 0;
    double rate = 0;
    bool klein = false;
    const unsigned char* daten = nullptr;
    std::size_t datenBytes = 0;
    for (std::size_t p = 12; p + 8 <= roh.size();)
    {
        const std::string id(roh.begin() + static_cast<std::ptrdiff_t>(p), roh.begin() + static_cast<std::ptrdiff_t>(p + 4));
        const std::size_t len = be32(&roh[p + 4]);
        const unsigned char* d = &roh[p + 8];
        if (p + 8 + len > roh.size()) break;
        if (id == "COMM")
        {
            kanaele = (d[0] << 8) | d[1];
            bits = (d[6] << 8) | d[7];
            rate = extended80(d + 8);
            if (art == "AIFC" && len >= 22)
            {
                const std::string kompr(d + 18, d + 22);
                if (kompr == "sowt")
                    klein = true;
                else if (kompr != "NONE")
                {
                    grund = "AIFC " + kompr;
                    return std::nullopt;
                }
            }
        }
        else if (id == "SSND")
        {
            const std::size_t versatz = be32(d);
            daten = d + 8 + versatz;
            datenBytes = len - 8 - versatz;
        }
        p += 8 + len + (len & 1);
    }
    if (!daten || kanaele < 1 || (bits != 16 && bits != 24) || rate <= 0)
    {
        grund = "COMM/SSND fehlt oder Format " + std::to_string(bits) + " Bit";
        return std::nullopt;
    }
    const std::size_t byteJe = static_cast<std::size_t>(bits / 8);
    const std::size_t frames = datenBytes / (byteJe * static_cast<std::size_t>(kanaele));
    Ton t;
    t.rate = static_cast<int>(std::lround(rate));
    t.stereo.resize(2 * frames);
    const double skala = 1.0 / double(1u << (bits - 1));
    for (std::size_t i = 0; i < frames; ++i)
        for (int k = 0; k < std::min(kanaele, 2); ++k)
        {
            const unsigned char* q = daten + (i * static_cast<std::size_t>(kanaele) + static_cast<std::size_t>(k)) * byteJe;
            std::int32_t v = 0;
            if (bits == 16)
                v = klein ? std::int16_t(q[0] | (q[1] << 8)) : std::int16_t((q[0] << 8) | q[1]);
            else
            {
                v = klein ? (q[0] | (q[1] << 8) | (q[2] << 16)) : ((q[0] << 16) | (q[1] << 8) | q[2]);
                if (v & 0x800000) v -= 0x1000000;
            }
            t.stereo[2 * i + static_cast<std::size_t>(k)] = static_cast<float>(v * skala);
            if (kanaele == 1) t.stereo[2 * i + 1] = t.stereo[2 * i];
        }
    return t;
}

/// Die Toene eines Laufs als (Anfang, Ende) in Frames. Die Iowa-Laeufe trennen die Toene durch digitale Stille (an der
/// Huellkurve nachgesehen, S195: Bloecke bei −300 dB zwischen den Toenen): ein Ton ist ein zusammenhaengendes Stueck von
/// 10-ms-Bloecken ueber −90 dBFS, mindestens 1 s lang; Pausen unter 30 ms trennen nicht.
std::vector<std::pair<std::size_t, std::size_t>> toeneDesLaufs(const std::vector<double>& m, int rate)
{
    const std::size_t b = static_cast<std::size_t>(rate / 100);
    const std::vector<double> e = bloeckeDb(m, b);
    std::vector<std::pair<std::size_t, std::size_t>> aus;
    std::size_t i = 0;
    while (i < e.size())
    {
        while (i < e.size() && e[i] <= -90.0) ++i;
        if (i >= e.size()) break;
        const std::size_t von = i;
        std::size_t still = 0, ende = i;
        for (; i < e.size(); ++i)
        {
            if (e[i] <= -90.0)
            {
                if (++still >= 3) break;
            }
            else
            {
                still = 0;
                ende = i + 1;
            }
        }
        if (ende - von >= 100) aus.emplace_back(von * b, ende * b);
    }
    return aus;
}

void fasseZusammen(const std::vector<Zeile>& zeilen)
{
    std::map<std::pair<std::string, std::string>, std::vector<const Zeile*>> je;
    for (const Zeile& z : zeilen)
        if (z.note >= 0 && !z.unsicher) je[{z.gruppe, z.variante}].push_back(&z);
    std::printf("\n## Zusammenfassung je Gruppe und Variante (ab 5 Toenen; Noten nach der Oktavkorrektur, ohne unsichere Stuecke)\n");
    std::printf("gruppe\tvariante\ttoene\tumfang\tmom_spanne_LU\tmom_spanne_p10_p90\tmom_min(an)\tmom_max(an)\tgroesster_sprung_LU(zwischen)\t"
                "sprung_median\tspitze_spanne_dB\tt60_min\tt60_max\tboden_median\tfrueh_min\tfrueh_max\n");
    for (auto& [schluessel, liste] : je)
    {
        if (liste.size() < 5) continue;
        std::sort(liste.begin(), liste.end(), [](const Zeile* a, const Zeile* b) { return a->note < b->note; });
        const Zeile *lo = liste[0], *hi = liste[0];
        double spMin = 1e9, spMax = -1e9, t60min = 1e9, t60max = 0, fMin = 1e9, fMax = 0, sprung = 0;
        int sprungA = 0, sprungB = 0;
        std::vector<double> boeden, momente, spruenge;
        for (std::size_t i = 0; i < liste.size(); ++i)
        {
            const Zeile* z = liste[i];
            momente.push_back(z->w.mom);
            if (i > 0 && z->note != liste[i - 1]->note && z->note - liste[i - 1]->note <= 4) spruenge.push_back(std::abs(z->w.mom - liste[i - 1]->w.mom));
            if (z->w.mom < lo->w.mom) lo = z;
            if (z->w.mom > hi->w.mom) hi = z;
            spMin = std::min(spMin, z->w.spitze), spMax = std::max(spMax, z->w.spitze);
            t60min = std::min(t60min, z->w.t60), t60max = std::max(t60max, z->w.t60);
            if (z->w.frueh > 0) fMin = std::min(fMin, z->w.frueh), fMax = std::max(fMax, z->w.frueh);
            boeden.push_back(z->w.boden);
            if (i > 0 && z->note != liste[i - 1]->note && z->note - liste[i - 1]->note <= 4)
            {
                const double d = std::abs(z->w.mom - liste[i - 1]->w.mom);
                if (d > sprung) sprung = d, sprungA = liste[i - 1]->note, sprungB = z->note;
            }
        }
        std::sort(boeden.begin(), boeden.end());
        std::printf("%s\t%s\t%zu\t%d-%d\t%.1f\t%.1f\t%.1f(%d)\t%.1f(%d)\t%.1f(%d-%d)\t%.1f\t%.1f\t%.2f\t%.2f\t%.0f\t%.1f\t%.1f\n", schluessel.first.c_str(),
                    schluessel.second.c_str(), liste.size(), liste.front()->note, liste.back()->note, hi->w.mom - lo->w.mom,
                    perzentil(momente, 0.9) - perzentil(momente, 0.1), lo->w.mom, lo->note, hi->w.mom, hi->note, sprung, sprungA, sprungB,
                    perzentil(spruenge, 0.5), spMax - spMin, t60min, t60max, boeden[boeden.size() / 2], fMin, fMax);
    }
}

/// Die Dynamik: je Gruppe der Median ueber die Noten vom Unterschied zweier Varianten (Momentary-Maximum).
void fasseDynamik(const std::vector<Zeile>& zeilen, const std::string& gruppenAnfang, const std::string& laut, const std::string& leise)
{
    std::map<std::string, std::map<int, std::pair<double, double>>> je;
    for (const Zeile& z : zeilen)
    {
        if (z.note < 0 || z.unsicher || z.gruppe.rfind(gruppenAnfang, 0) != 0) continue;
        auto& p = je[z.gruppe].try_emplace(z.note, 1e9, 1e9).first->second;
        if (z.variante == laut) p.first = z.w.mom;
        if (z.variante == leise) p.second = z.w.mom;
    }
    for (const auto& [gruppe, noten] : je)
    {
        std::vector<double> d;
        for (const auto& [note, p] : noten)
            if (p.first < 1e8 && p.second < 1e8) d.push_back(p.first - p.second);
        if (d.size() < 3) continue;
        std::printf("%s\t%s - %s\tNoten %zu\tMedian %.1f LU\tp10 %.1f\tp90 %.1f\n", gruppe.c_str(), laut.c_str(), leise.c_str(), d.size(), perzentil(d, 0.5),
                    perzentil(d, 0.1), perzentil(d, 0.9));
    }
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "Aufruf: %s <Ordner 'Klasse 2 - Gezupfte Saite'>\n", argv[0]);
        return 2;
    }
    const fs::path wurzel(argv[1]);
    std::vector<Zeile> zeilen;
    std::printf("# S195 Ziele aus Aufnahmen (Spalten im Kopf der Quelle erklaert)\n");
    std::printf("gruppe\tvariante\tnote\tmom\tint1\tspitze\tfrueh\tspaet\tt20\tt60\tboden\tdauer\tokt\tdatei\n");

    // ---- VCSL ----
    const fs::path vcsl = wurzel / "VCSL" / "Chordophones";
    static const std::regex notenTeil("(^|_)([A-G][#b]?-?[0-9])(_|\\.)");
    static const std::regex runde("[rR]{2}[0-9]+");
    if (fs::exists(vcsl))
    {
        std::vector<fs::path> dateien;
        for (const auto& e : fs::recursive_directory_iterator(vcsl))
            if (e.is_regular_file() && e.path().extension() == ".wav") dateien.push_back(e.path());
        std::sort(dateien.begin(), dateien.end());
        for (const fs::path& p : dateien)
        {
            const std::string name = p.filename().string();
            const std::string rel = fs::relative(p.parent_path(), vcsl).generic_string();
            Zeile z;
            z.gruppe = rel.substr(rel.find('/') + 1);
            z.datei = name;
            std::smatch m;
            if (std::regex_search(name, m, notenTeil))
            {
                z.note = noteAus(m[2].str());
                // Variante = Name ohne Note, ohne Endung, ohne Nummer der Wiederholung („rr1“, „f1“ → „rr“, „f“)
                static const std::regex endung("\\.wav$"), nummer("([a-zA-Z])[0-9]+$");
                z.variante = std::regex_replace(m.prefix().str() + "_" + m.suffix().str(), endung, "");
                z.variante = std::regex_replace(std::regex_replace(z.variante, runde, "rr"), nummer, "$1");
            }
            const auto wav = lumi::audio::liesWavDatei(p);
            if (!wav || z.note < 0)
            {
                std::printf("# nicht gelesen oder ohne Note: %s\n", name.c_str());
                continue;
            }
            z.w = messe({wav->stereo, wav->rate}, hzVon(z.note));
            drucke(z);
            zeilen.push_back(z);
        }
        // Oktavlage der Namen je Gruppe: der Median der gemessenen Lage korrigiert die Note (VCSL benennt nicht einheitlich)
        std::map<std::string, std::vector<double>> lagen;
        for (const Zeile& z : zeilen)
            if (z.w.okt != 9) lagen[z.gruppe].push_back(z.w.okt);
        std::printf("\n## Oktavlage der Namen je Gruppe (Median der gemessenen Lage gegen C4 = 60; die Note wird darum korrigiert)\n");
        for (const auto& [gruppe, v] : lagen)
        {
            const int k = static_cast<int>(perzentil(v, 0.5));
            int gleich = 0;
            for (const double x : v) gleich += static_cast<int>(x) == k ? 1 : 0;
            std::printf("%s\t%+d Oktave(n)\t%d von %zu Toenen in dieser Lage\n", gruppe.c_str(), k, gleich, v.size());
            for (Zeile& z : zeilen)
                if (z.gruppe == gruppe) z.note += 12 * k;
        }
    }
    else
        std::printf("# VCSL fehlt unter %s\n", vcsl.string().c_str());

    // ---- Iowa MIS Gitarre ----
    const fs::path iowa = wurzel / "Iowa_MIS_Guitar_mono2496";
    static const std::regex iowaName("Guitar\\.(pp|mf|ff)\\.(sul_?[A-G])\\.([A-G]b?[0-9])([A-G]b?[0-9])?\\.mono\\.aif");
    if (fs::exists(iowa))
    {
        std::vector<fs::path> dateien;
        for (const auto& e : fs::directory_iterator(iowa))
            if (e.path().extension() == ".aif") dateien.push_back(e.path());
        std::sort(dateien.begin(), dateien.end());
        for (const fs::path& p : dateien)
        {
            const std::string name = p.filename().string();
            std::smatch m;
            if (!std::regex_match(name, m, iowaName))
            {
                std::printf("# Iowa: Name nicht erkannt: %s\n", name.c_str());
                continue;
            }
            const int von = noteAus(m[3].str()), bis = m[4].matched ? noteAus(m[4].str()) : von;
            std::string grund;
            const auto ton = liesAiff(p, grund);
            if (!ton)
            {
                std::printf("# Iowa: nicht gelesen (%s): %s\n", grund.c_str(), name.c_str());
                continue;
            }
            const std::vector<double> mono = monoVon(ton->stereo);
            const std::vector<std::pair<std::size_t, std::size_t>> an = toeneDesLaufs(mono, ton->rate);
            const int erwartet = bis - von + 1;
            // die Note je Stueck aus dem Klang; wo die Zahl passt, muss sie gleich der Reihenfolge sein (Gegenprobe)
            std::vector<int> noten;
            int abweichend = 0;
            for (std::size_t i = 0; i < an.size(); ++i)
            {
                const std::vector<double> teilMono(mono.begin() + static_cast<std::ptrdiff_t>(an[i].first), mono.begin() + static_cast<std::ptrdiff_t>(an[i].second));
                noten.push_back(noteAusKlang(teilMono, ton->rate, von, bis));
                if (static_cast<int>(an.size()) == erwartet && noten.back() != von + static_cast<int>(i)) ++abweichend;
            }
            std::printf("# Iowa %s: %d Hz, %.1f s, Stuecke %zu, erwartet %d%s; Noten aus dem Klang:", name.c_str(), ton->rate, double(mono.size()) / ton->rate,
                        an.size(), erwartet, static_cast<int>(an.size()) == erwartet ? "" : " (Zahl passt nicht)");
            for (const int n : noten) std::printf(" %d", n);
            if (static_cast<int>(an.size()) == erwartet) std::printf("  — gegen die Reihenfolge abweichend: %d", abweichend);
            std::printf("\n");
            for (std::size_t i = 0; i < an.size(); ++i)
            {
                const std::size_t ab = an[i].first, ende = an[i].second;
                Ton teil;
                teil.rate = ton->rate;
                teil.stereo.assign(ton->stereo.begin() + static_cast<std::ptrdiff_t>(2 * ab), ton->stereo.begin() + static_cast<std::ptrdiff_t>(2 * ende));
                Zeile z;
                z.gruppe = "Iowa Gitarre " + m[2].str();
                z.variante = m[1].str();
                z.datei = name + "#" + std::to_string(i + 1);
                // passt die Zahl, gilt die Reihenfolge des Laufs (der Klang irrte in 5 von rund 330 Stuecken); sonst der Klang, als unsicher
                z.unsicher = static_cast<int>(an.size()) != erwartet;
                z.note = z.unsicher ? noten[i] : von + static_cast<int>(i);
                z.w = messe(teil, hzVon(z.note >= 0 ? z.note : von));
                drucke(z);
                zeilen.push_back(z);
            }
        }
    }
    else
        std::printf("# Iowa fehlt unter %s\n", iowa.string().c_str());

    fasseZusammen(zeilen);

    // Dynamik: je Gruppe und Note die Varianten nebeneinander (Harfe f gegen mf, Gitarre ff/mf/pp)
    std::printf("\n## Dynamik: Unterschied des Momentary-Maximums zwischen zwei Varianten, Median ueber die Noten\n");
    fasseDynamik(zeilen, "Concert Harp", "KSHarp_f", "KSHarp_mf");
    fasseDynamik(zeilen, "Folk Harp", "EWHarp_Normal_v3_rr", "EWHarp_Normal_v2_rr");
    fasseDynamik(zeilen, "Iowa", "ff", "mf");
    fasseDynamik(zeilen, "Iowa", "mf", "pp");
    fasseDynamik(zeilen, "Iowa", "ff", "pp");
    fasseDynamik(zeilen, "Harpsichord, English/Sustains", "ZuckermannKitHarpsi_Normal_Sus_rr", "ZuckermannKitHarpsi_Lute_Sus_rr");
    return 0;
}
