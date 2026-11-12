/**
 * @file Pedale.cpp
 * @brief InstrumentModels, Klasse 2: die Pedale der Harfe (K2 F-6 a) — Diagramm, Saiten, Vorschlag aus den Noten, Spur, Glissando,
 *        Bisbigliando (Herleitung im Kopf von `Zupf.hpp`, Abschnitt Harfe)
 */
#include "InstrumentModels/zupf/Zupf.hpp"

#include <algorithm>

namespace lumi::kern::zupf
{

namespace
{

/// Die Tonklasse der Stammtoene C D E F G A H.
constexpr std::array<int, 7> kStamm = {0, 2, 4, 5, 7, 9, 11};

int stammIndex(int note)
{
    const int k = ((note % 12) + 12) % 12;
    for (int i = 0; i < 7; ++i)
        if (kStamm[static_cast<std::size_t>(i)] == k) return i;
    return -1;
}

}  // namespace

std::vector<int> harfenSaiten(const ZupfSpiel& harfe, const Pedale& p)
{
    std::vector<int> saiten;
    for (int n = harfe.noteMin; n <= harfe.noteMax; ++n)
    {
        const int i = stammIndex(n);
        if (i >= 0) saiten.push_back(n + std::clamp(p[static_cast<std::size_t>(i)], -1, 1));
    }
    return saiten;
}

std::vector<int> saitenFuer(int note, const Pedale& p)
{
    std::vector<int> s;
    const int k = ((note % 12) + 12) % 12;
    for (int i = 0; i < 7; ++i)
        if (((kStamm[static_cast<std::size_t>(i)] + std::clamp(p[static_cast<std::size_t>(i)], -1, 1)) % 12 + 12) % 12 == k) s.push_back(i);
    return s;
}

bool spielbar(int note, const Pedale& p)
{
    return !saitenFuer(note, p).empty();
}

std::vector<int> glissandoNoten(const ZupfSpiel& harfe, int von, int bis, const Pedale& p)
{
    std::vector<int> alle = harfenSaiten(harfe, p);
    std::vector<int> g;
    const int lo = std::min(von, bis), hi = std::max(von, bis);
    for (const int n : alle)
        if (n >= lo && n <= hi) g.push_back(n);
    std::sort(g.begin(), g.end());
    if (bis < von) std::reverse(g.begin(), g.end());
    return g;
}

Pedale pedaleVorschlag(const std::vector<int>& noten, bool* ok)
{
    // alle 3^7 Diagramme (Reihenfolge je Pedal ♮, ♯, ♭): die meisten spielbaren Noten, dann die wenigsten Pedale ausser ♮
    std::vector<int> klassen;
    for (const int n : noten) klassen.push_back(((n % 12) + 12) % 12);
    std::sort(klassen.begin(), klassen.end());
    klassen.erase(std::unique(klassen.begin(), klassen.end()), klassen.end());
    constexpr std::array<int, 3> stellung = {0, 1, -1};
    Pedale beste{0, 0, 0, 0, 0, 0, 0};
    int besteZahl = -1, besteAus = 99;
    for (int code = 0; code < 2187; ++code)
    {
        Pedale p{};
        int c = code, aus = 0;
        for (int i = 0; i < 7; ++i)
        {
            p[static_cast<std::size_t>(i)] = stellung[static_cast<std::size_t>(c % 3)];
            aus += p[static_cast<std::size_t>(i)] != 0;
            c /= 3;
        }
        int zahl = 0;
        for (const int k : klassen) zahl += spielbar(k, p);
        if (zahl > besteZahl || (zahl == besteZahl && aus < besteAus))
        {
            beste = p;
            besteZahl = zahl;
            besteAus = aus;
        }
    }
    if (ok != nullptr) *ok = besteZahl == static_cast<int>(klassen.size());
    return beste;
}

std::vector<PedalStelle> pedalSpurVorschlag(const std::vector<ClipNote>& noten)
{
    std::vector<const ClipNote*> folge;
    for (const ClipNote& n : noten) folge.push_back(&n);
    std::stable_sort(folge.begin(), folge.end(), [](const ClipNote* a, const ClipNote* b) { return a->abS < b->abS; });
    std::vector<int> alle;
    for (const ClipNote* n : folge) alle.push_back(n->anschlag.note);
    bool ok = false;
    std::vector<PedalStelle> spur{{0.0, pedaleVorschlag(alle, &ok)}};
    if (ok) return spur;
    // sonst der Reihe nach: das Diagramm der ersten Noten, solange es traegt; an einer Note, die es nicht traegt, wechselt
    // das Pedal, dessen Saite seit der letzten Umschaltstelle nicht gebraucht wurde
    std::vector<int> seit;
    spur.clear();
    for (std::size_t i = 0; i < folge.size(); ++i)
    {
        const int note = folge[i]->anschlag.note;
        if (spur.empty())
        {
            std::vector<int> vorne;
            for (std::size_t k = i; k < folge.size(); ++k)
            {
                vorne.push_back(folge[k]->anschlag.note);
                bool traegt = false;
                (void)pedaleVorschlag(vorne, &traegt);
                if (!traegt)
                {
                    vorne.pop_back();
                    break;
                }
            }
            spur.push_back({folge[i]->abS, pedaleVorschlag(vorne)});
        }
        Pedale p = spur.back().pedale;
        if (!spielbar(note, p))
        {
            const int k = ((note % 12) + 12) % 12;
            int wahl = -1, wert = 0;
            for (int s = 0; s < 7 && wahl < 0; ++s)
                for (const int v : {0, 1, -1})
                {
                    if ((kStamm[static_cast<std::size_t>(s)] + v + 12) % 12 != k) continue;
                    bool gebraucht = false;
                    for (const int alt : seit)
                        for (const int si : saitenFuer(alt, p))
                            if (si == s) gebraucht = true;
                    if (!gebraucht)
                    {
                        wahl = s;
                        wert = v;
                        break;
                    }
                }
            if (wahl < 0)  // jede Saite fuer diese Tonklasse ist belegt: die erste passende
                for (int s = 0; s < 7 && wahl < 0; ++s)
                    for (const int v : {0, 1, -1})
                        if ((kStamm[static_cast<std::size_t>(s)] + v + 12) % 12 == k)
                        {
                            wahl = s;
                            wert = v;
                            break;
                        }
            p[static_cast<std::size_t>(wahl)] = wert;
            spur.push_back({folge[i]->abS, p});
            seit.clear();
        }
        seit.push_back(note);
    }
    return spur;
}

ZupfSpiel harfeMitPedalen(const ZupfSpiel& harfe, const Pedale& p)
{
    ZupfSpiel s = harfe;
    s.mitschwingNoten = harfenSaiten(harfe, p);
    return s;
}

}  // namespace lumi::kern::zupf
