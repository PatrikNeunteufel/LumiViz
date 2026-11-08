/**
 * @file Reibung.cpp
 * @brief InstrumentModels, gemeinsam: Reibung Bogen–Schwinger (Herleitung und Herkunft im Kopf von `Reibung.hpp`)
 */
#include "InstrumentModels/gemeinsam/Reibung.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::kern
{

double reibwertGalluzzo(double x)
{
    return 0.4 * std::exp(-x / 0.01) + 0.45 * std::exp(-x / 0.1) + 0.35;
}

double reibung(double vh, double vb, double fb, double zEff, double muS, bool& haftet, double& letzte)
{
    if (fb <= 0.0)
    {
        haftet = false;
        return 0.0;
    }
    const double fHaft = zEff * (vb - vh);  // die Kraft, die v = v_B haelt
    const bool haftenMoeglich = std::abs(fHaft) <= muS * fb;
    if (haftet && haftenMoeglich) return fHaft;  // Hysterese: es haftet weiter, ohne Suche
    const double d = std::abs(vh - vb);
    const double k = fb / zEff;
    // g(x) = x − d + k·μ(x): Gleiten mit |Δv| = x
    const auto g = [&](double x) { return x - d + k * reibwertGalluzzo(x); };
    // der aeusserste Schnittpunkt: von x = d abwaerts die erste Vorzeichenumkehr
    double wurzel = -1.0;
    // zuerst Newton ab dem letzten Gleitpunkt (er wandert stetig): angenommen, wenn er konvergiert und auf dem
    // steigenden Ast liegt (g' > 0 — das ist der aeusserste Schnittpunkt, weil g(d) > 0)
    if (!haftet && letzte >= 0.0 && d > 0.0)
    {
        double x = std::min(letzte, d);
        for (int i = 0; i < 6; ++i)
        {
            const double m1 = 0.4 * std::exp(-x / 0.01), m2 = 0.45 * std::exp(-x / 0.1);
            const double gx = x - d + k * (m1 + m2 + 0.35);
            const double gs = 1.0 - k * (m1 / 0.01 + m2 / 0.1);
            if (gs <= 0.0) break;
            const double neu = x - gx / gs;
            if (std::abs(neu - x) < 1e-13 * (1.0 + d))
            {
                if (neu >= 0.0 && neu <= d) wurzel = neu;
                break;
            }
            x = neu;
        }
    }
    if (wurzel < 0.0 && d > 0.0 && g(d) > 0.0)
    {
        constexpr int kSchritte = 16;
        double xo = d, go = g(d);
        for (int i = kSchritte - 1; i >= 0; --i)
        {
            const double xu = d * i / kSchritte;
            const double gu = g(xu);
            if (gu <= 0.0)
            {
                double lo = xu, hi = xo;
                for (int j = 0; j < 30; ++j)
                {
                    const double m = 0.5 * (lo + hi);
                    if (g(m) <= 0.0)
                        lo = m;
                    else
                        hi = m;
                }
                wurzel = 0.5 * (lo + hi);
                break;
            }
            xo = xu;
            go = gu;
        }
        (void)go;
    }
    if (haftenMoeglich && wurzel < 0.0)
    {
        haftet = true;
        return fHaft;
    }
    if (wurzel < 0.0)
    {
        // kein Gleitpunkt und kein Haften (nur bei Spruengen der Steuerung): Haftgrenze
        haftet = false;
        return std::clamp(fHaft, -muS * fb, muS * fb);
    }
    haftet = false;
    letzte = wurzel;
    const double s = vh > vb ? 1.0 : -1.0;  // Gleiten in Richtung v_h − v_B
    return -s * fb * reibwertGalluzzo(wurzel);
}

}  // namespace lumi::kern
