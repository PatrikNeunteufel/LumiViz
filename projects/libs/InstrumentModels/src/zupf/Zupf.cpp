/**
 * @file Zupf.cpp
 * @brief InstrumentModels, Kern `pluckedString` Rechenfassung 2: die Saiten aus den Kurven und die Note in der Schleife
 *        (34k-7d-4 (c); Herleitung im Kopf von `Zupf.hpp`). Die Fassung aus IC (Rechenfassung 1) stand hier bis S224 (ZB1 a).
 */
#include "InstrumentModels/zupf/Zupf.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::kern::zupf
{

std::optional<Tastensaite> tastensaiteAus(const ZupfSpiel& s, int taste)
{
    if (taste < s.noteMin || taste > s.noteMax || s.laenge.punkte.empty()) return std::nullopt;
    const auto hat = [&](const Kurve& k) {
        return std::any_of(k.punkte.begin(), k.punkte.end(), [&](const NotenWert& p) { return p.note == taste; });
    };
    if (!hat(s.laenge)) return std::nullopt;
    const auto an = [&](const Kurve& k) { return wertAn(k, taste, Skala::Linear); };
    Tastensaite t;
    t.werte.zug = an(s.zug);
    t.werte.masse = an(s.masse);
    t.werte.biegesteife = an(s.biegesteife);
    t.werte.verlustBiegung = an(s.verlustBiegung);
    t.werte.verlustInnen = an(s.verlustInnen);
    t.werte.verlustLuft = an(s.verlustLuft);
    t.werte.durchmesser = an(s.durchmesser);
    t.werte.dichte = an(s.dichte);
    t.laenge = an(s.laenge);
    t.zupfort = wertAn(s.zupfort, taste, Skala::Linear);
    t.stegTaste = taste;
    return t;
}

void setzeSaitenJeTaste(ZupfSpiel& s, const std::function<std::optional<Tastensaite>(int)>& saite, Belegkraft belegkraft,
                        const std::string& herkunft)
{
    Kurve* alle[] = {&s.laenge, &s.zug, &s.masse, &s.biegesteife, &s.verlustBiegung, &s.verlustInnen, &s.verlustLuft, &s.durchmesser,
                     &s.dichte, &s.zupfort};
    for (Kurve* k : alle)
    {
        *k = Kurve{};
        k->belegkraft = belegkraft;
        k->herkunft = herkunft;
    }
    for (int taste = s.noteMin; taste <= s.noteMax; ++taste)
    {
        const std::optional<Tastensaite> t = saite(taste);
        if (!t) continue;
        const double w[] = {t->laenge, t->werte.zug, t->werte.masse, t->werte.biegesteife, t->werte.verlustBiegung, t->werte.verlustInnen,
                            t->werte.verlustLuft, t->werte.durchmesser, t->werte.dichte, t->zupfort};
        for (std::size_t i = 0; i < std::size(alle); ++i) alle[i]->punkte.push_back({taste, w[i]});
    }
}

ZupfNote zupfNote(const ZupfSpiel& s, int note, int saite, int stufe, long long halten, double rate)
{
    ZupfNote n;
    if (rate != kSchleifenRate)
    {
        n.fehler = "the plucked string computes at 48 kHz only";
        return n;
    }
    double xp = 0.0;
    if (!s.jeSaite())
    {
        const std::optional<Tastensaite> t = tastensaiteAus(s, note);
        if (!t)
        {
            n.fehler = "no string for this key";
            return n;
        }
        const auto e = std::find_if(s.umlauf.begin(), s.umlauf.end(), [&](const UmlaufEintrag& u) { return u.taste == note; });
        if (e == s.umlauf.end())
        {
            n.fehler = "the loop filter table has no entry for this key";
            return n;
        }
        umlauf::Umlauffilter f = e->filter;
        if (!umlauf::stimmeAmGrundton(f, t->werte, t->laenge))
        {
            n.fehler = "the loop filter does not fit this string";
            return n;
        }
        n.saiten.push_back({t->werte, t->laenge, f});
        n.stegTaste = t->stegTaste;
        xp = t->zupfort;
    }
    else
    {
        const Saitensatz& satz = s.satz;
        std::size_t c = 0;
        if (saite >= 0 && static_cast<std::size_t>(saite) < satz.choere.size())
            c = static_cast<std::size_t>(saite);
        else  // ohne Angabe: die hoechste Saite, deren leere Taste nicht ueber der Note liegt (wie der Pruefstand)
            for (std::size_t i = 0; i < satz.choere.size(); ++i)
                if (satz.choere[i].leer <= note) c = i;
        const Saitenchor& chor = satz.choere[c];
        const int lage = note - chor.leer;
        if (lage < 0 || lage > satz.buende)
        {
            n.fehler = "the note is not on this string";
            return n;
        }
        for (std::size_t i = 0; i < chor.saiten.size(); ++i)
        {
            const Chorsaite& cs = chor.saiten[i];
            // jede Saite hat eine Tafel, im Einklang die der ersten (34k-7d-3)
            auto e = std::find_if(s.umlauf.begin(), s.umlauf.end(),
                                  [&](const UmlaufEintrag& u) { return u.chor == static_cast<int>(c) && u.saite == static_cast<int>(i); });
            if (e == s.umlauf.end())
                e = std::find_if(s.umlauf.begin(), s.umlauf.end(), [&](const UmlaufEintrag& u) { return u.chor == static_cast<int>(c) && u.saite == 0; });
            if (e == s.umlauf.end())
            {
                n.fehler = "the loop filter table has no entry for this string";
                n.saiten.clear();
                return n;
            }
            umlauf::Saitentafel tafel = e->tafel;
            tafel.buende = satz.buende;
            tafel.mensur = satz.mensur;
            // die Lage der Laenge: ein im Einklang verstimmter Chor liegt um −12 lb(Faktor) Buende verschoben
            const double lageLaenge = lage - 12.0 * std::log2(cs.laengenFaktor);
            const std::optional<umlauf::Umlauffilter> f = umlauf::filterAn(tafel, cs.werte, lageLaenge);
            if (!f)
            {
                n.fehler = "the loop filter does not fit this string";
                n.saiten.clear();
                return n;
            }
            n.saiten.push_back({cs.werte, satz.mensur * cs.laengenFaktor * std::exp2(-lage / 12.0), *f});
        }
        n.stegTaste = chor.leer;
        xp = wertAn(s.zupfort, note, Skala::Linear);
    }
    // der Zupf nach der Stufe (GZ6 a, GZ10 a, ZD1 a, ZD2 a): Auslenkung bzw. Kraft und das Abloesetempo im selben
    // Verhaeltnis, dazu die Neigung ohne Pegel
    const double faktor = std::pow(10.0, s.pegelJeStufe * stufe / 20.0);
    const Tastensaite erste{n.saiten.front().werte, n.saiten.front().laenge, xp, n.stegTaste};
    double d = s.auslenkung * faktor;
    if (s.kraftGegeben)
    {
        const double F = wertAn(s.kraft, note, Skala::Logarithmisch) * faktor;
        d = F * xp * (erste.laenge - xp) / (erste.werte.zug * erste.laenge);
    }
    n.zupf = {xp, d, s.winkelGrad, s.breite, s.tempo * faktor, static_cast<double>(stufe), s.neigungJeStufe};
    // ZN1 a: der Daempfer am Sattel ab dem Ende der Note, in `daempfenS` um 60 dB
    const long long ab = std::llround(kVorlauf * rate) + std::max(0LL, halten);
    for (SchleifenSaite& sa : n.saiten)
    {
        const double z0 = std::sqrt(sa.werte.zug * sa.werte.masse);
        const double f1 = umlauf::grundtonDerSaite(sa.werte, sa.laenge);
        sa.daempferAb = ab;
        sa.daempferRampe = std::llround(s.daempfenS * rate);
        sa.daempferLeitwert = f1 > 0.0 ? leitwertFuerAbklingen(z0, f1, s.daempfenS) : 0.0;
    }
    return n;
}

}  // namespace lumi::kern::zupf
