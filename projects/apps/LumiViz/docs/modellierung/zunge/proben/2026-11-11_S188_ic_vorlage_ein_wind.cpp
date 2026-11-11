// S188 Planung 34j, Gegenprobe: ein Winddruck fuer alle Tasten (Balg/Atem), statt Expression im Fenster je Note.
// Die Stimmung (f0) bleibt die aus IC; der Druck kommt unmittelbar (druckPa). Dazu: Stimmung f0 am festen Druck.
#include "gemeinsam/Werkzeug.hpp"
#include "klassen/10_zunge/Zunge.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

using namespace ic;
using namespace ic::zunge;

namespace
{
double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}
struct Z
{
    bool schwingt = false;
    double cent = 0.0, lufs = -300.0, ein = 0.0;
};
Z messe(const ZungenSpiel& s, int note, double druck)
{
    ZungenAnschlag a;
    a.note = note;
    a.druckPa = druck;
    a.haltenS = 1.0;
    std::vector<float> t;
    ZungenMessung m;
    rechneZungenTon(s, a, t, 57600, &m);
    Z z;
    z.schwingt = m.schwingt;
    z.cent = m.schwingt ? 1200.0 * std::log2(m.spielHz / hzVon(note)) : 0.0;
    z.ein = m.einschwingS;
    std::vector<float> halt(t.begin(), t.begin() + 48000);
    z.lufs = lautheit(halt, a.rate);
    return z;
}
}  // namespace

int main()
{
    struct Fall
    {
        ZungenSpiel s;
        std::vector<double> druck;
    };
    // Druecke: Akkordeon 0–2,5 kPa (Laborachse), Harmonium 0,2–1,5 kPa (Puranik & Scavone), Atem bis 3 kPa (offen)
    std::vector<Fall> faelle = {{baueVorgabeAkkordeon(), {150, 300, 600, 1200, 2400}},
                                {baueVorgabeHarmonium(), {200, 400, 800, 1500}},
                                {baueVorgabeMelodica(), {150, 300, 600, 1200, 2400}},
                                {baueVorgabeMundharmonika(), {150, 300, 600, 1200, 2400}}};
    for (Fall& f : faelle)
    {
        const ZungenSpiel& s = f.s;
        const int ab = s.bassAb > 0 ? s.bassAb : s.noteMin;
        std::vector<int> noten;
        for (int n = ab; n <= s.noteMax; n += 3) noten.push_back(n);
        if (noten.back() != s.noteMax) noten.push_back(s.noteMax);
        const std::size_t nd = f.druck.size();
        std::vector<Z> alle(noten.size() * nd);
        parallelJe(alle.size(), [&](std::size_t i) { alle[i] = messe(s, noten[i / nd], f.druck[i % nd]); }, 8);
        std::printf("\n## %s (ab Note %d), ein Druck fuer alle Tasten: je Druck schwingt/Cent/LUFS/Einschwingen ms\nnote", s.name.c_str(), ab);
        for (const double p : f.druck) std::printf("\t| %.0f Pa", p);
        std::printf("\n");
        std::vector<double> mn(nd, 1e9), mx(nd, -1e9);
        std::vector<int> ok(nd, 0);
        for (std::size_t k = 0; k < noten.size(); ++k)
        {
            std::printf("%d", noten[k]);
            for (std::size_t j = 0; j < nd; ++j)
            {
                const Z& z = alle[k * nd + j];
                std::printf("\t| %d %+.0f %.1f %.0f", z.schwingt ? 1 : 0, z.cent, z.lufs, 1000.0 * z.ein);
                if (z.schwingt && std::abs(z.cent) < 10.0)
                {
                    ++ok[j];
                    mn[j] = std::min(mn[j], z.lufs);
                    mx[j] = std::max(mx[j], z.lufs);
                }
            }
            std::printf("\n");
        }
        std::printf("spielt auf 10 Cent:");
        for (std::size_t j = 0; j < nd; ++j) std::printf("\t%d/%zu", ok[j], noten.size());
        std::printf("\nSpanne LUFS (nur spielende):");
        for (std::size_t j = 0; j < nd; ++j) std::printf("\t%.1f", mx[j] - mn[j]);
        std::printf("\n");
    }
    return 0;
}
