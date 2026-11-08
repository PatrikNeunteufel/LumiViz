/**
 * @file TrommelKern.cpp
 * @brief InstrumentModels, Kern `membrane`: Tafel, Werte ↔ Spiel, Adapter, Vorlagen (34h-3; Herleitung im Kopf von
 *        `TrommelKern.hpp`)
 */
#include "InstrumentModels/trommel/TrommelKern.hpp"

#include "InstrumentCore/Anschlag.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <utility>

namespace lumi::kern::trommel
{

namespace
{

constexpr int kUmfangMin = 21;   // A0
constexpr int kUmfangMax = 108;  // C8

constexpr std::string_view kNameNoteMin = "noteMin";
constexpr std::string_view kNameNoteMax = "noteMax";
constexpr std::string_view kFelle = "heads";

ParameterAngabe angabe(std::string_view name, std::string_view titel, std::string_view gruppe, WertArt art, std::string_view einheit = {},
                       double von = 0.0, double bis = 1.0)
{
    ParameterAngabe a;
    a.name = std::string(name);
    a.titel = std::string(titel);
    a.gruppe = std::string(gruppe);
    a.art = art;
    a.darstellung = darstellungZu(art);
    a.einheit = std::string(einheit);
    a.von = von;
    a.bis = bis;
    a.belegkraft = Belegkraft::Offen;
    a.quellen = {"lumiviz"};
    return a;
}

/// Ein Wert des Spiels (oder eines Fells) in der Tafel: Angabe, Lesen, Setzen.
template <typename S>
struct Feld
{
    ParameterAngabe angabe;
    std::function<Wert(const S&)> lies;
    std::function<void(S&, const Wert&)> setze;
};

template <typename S, typename T>
Feld<S> zahlVon(ParameterAngabe a, T S::* feld)
{
    return Feld<S>{std::move(a), [feld](const S& s) { return Wert{static_cast<double>(s.*feld)}; },
                   [feld](S& s, const Wert& w) {
                       if (const auto* d = std::get_if<double>(&w)) s.*feld = *d;
                   }};
}

template <typename S>
Feld<S> zahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
             double S::* feld)
{
    return zahlVon(angabe(name, titel, gruppe, WertArt::Zahl, einheit, von, bis), feld);
}

template <typename S>
Feld<S> ganzzahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
                 int S::* feld)
{
    return Feld<S>{angabe(name, titel, gruppe, WertArt::Ganzzahl, einheit, von, bis),
                   [feld](const S& s) { return Wert{static_cast<std::int64_t>(s.*feld)}; },
                   [feld](S& s, const Wert& w) {
                       if (const auto* g = std::get_if<std::int64_t>(&w)) s.*feld = static_cast<int>(*g);
                   }};
}

template <typename S>
Feld<S> schalter(std::string_view name, std::string_view titel, std::string_view gruppe, bool S::* feld)
{
    return Feld<S>{angabe(name, titel, gruppe, WertArt::Schalter), [feld](const S& s) { return Wert{s.*feld}; },
                   [feld](S& s, const Wert& w) {
                       if (const auto* b = std::get_if<bool>(&w)) s.*feld = *b;
                   }};
}

Feld<TrommelSpiel> schlaegelZahl(std::string_view name, std::string_view titel, std::string_view einheit, double von, double bis,
                                 double Stossanreger::* feld)
{
    return Feld<TrommelSpiel>{angabe(name, titel, "Stroke", WertArt::Zahl, einheit, von, bis),
                              [feld](const TrommelSpiel& s) { return Wert{s.schlaegel.*feld}; },
                              [feld](TrommelSpiel& s, const Wert& w) {
                                  if (const auto* d = std::get_if<double>(&w)) s.schlaegel.*feld = *d;
                              }};
}

/// Ein Fell mit seinem Belag (ein Eintrag der Liste `heads`).
struct FellMitBelag
{
    Fell fell;
    Belag belag;
};

/// Die Felder eines Eintrags der Liste `heads`.
const std::vector<Feld<FellMitBelag>>& fellFelder()
{
    static const std::vector<Feld<FellMitBelag>> f = [] {
        std::vector<Feld<FellMitBelag>> v;
        const auto fz = [](std::string_view name, std::string_view titel, std::string_view einheit, double von, double bis, double Fell::* feld) {
            return Feld<FellMitBelag>{angabe(name, titel, "", WertArt::Zahl, einheit, von, bis),
                                      [feld](const FellMitBelag& s) { return Wert{s.fell.*feld}; },
                                      [feld](FellMitBelag& s, const Wert& w) {
                                          if (const auto* d = std::get_if<double>(&w)) s.fell.*feld = *d;
                                      }};
        };
        const auto bz = [](std::string_view name, std::string_view titel, double von, double bis, double Belag::* feld) {
            return Feld<FellMitBelag>{angabe(name, titel, "", WertArt::Zahl, "", von, bis),
                                      [feld](const FellMitBelag& s) { return Wert{s.belag.*feld}; },
                                      [feld](FellMitBelag& s, const Wert& w) {
                                          if (const auto* d = std::get_if<double>(&w)) s.belag.*feld = *d;
                                      }};
        };
        v.push_back(fz("radius", "Radius", "m", 0.01, 1.0, &Fell::radiusM));
        v.push_back(fz("tension", "Tension", "N/m", 10.0, 20000.0, &Fell::spannungNm));
        v.push_back(fz("density", "Area density", "kg/m²", 0.01, 5.0, &Fell::dichteKgM2));
        v.push_back(fz("damping1", "Damping d1", "kg/(m²·s)", 0.0, 100.0, &Fell::d1));
        v.push_back(fz("damping3", "Damping d3", "kg/s", 0.0, 0.1, &Fell::d3));
        v.push_back(Feld<FellMitBelag>{angabe("radiating", "Radiates", "", WertArt::Schalter),
                                       [](const FellMitBelag& s) { return Wert{s.fell.abstrahlung}; },
                                       [](FellMitBelag& s, const Wert& w) {
                                           if (const auto* b = std::get_if<bool>(&w)) s.fell.abstrahlung = *b;
                                       }});
        v.push_back(fz("innerAir", "Air mass inside", "", 0.0, 2.0, &Fell::innenLuft));
        v.push_back(Feld<FellMitBelag>{angabe("inlay", "Inlay", "", WertArt::Schalter), [](const FellMitBelag& s) { return Wert{s.belag.an}; },
                                       [](FellMitBelag& s, const Wert& w) {
                                           if (const auto* b = std::get_if<bool>(&w)) s.belag.an = *b;
                                       }});
        v.push_back(bz("inlaySigma", "Inlay sigma", 1.0, 10.0, &Belag::sigma));
        v.push_back(bz("inlayEdge", "Inlay edge", 0.0, 1.0, &Belag::kante));
        v.push_back(bz("inlayWidth", "Inlay width", 0.001, 1.0, &Belag::breite));
        return v;
    }();
    return f;
}

/// Die Felder des Spiels ausser der Liste der Felle, in der Reihenfolge der Tafel.
const std::vector<Feld<TrommelSpiel>>& felder()
{
    static const std::vector<Feld<TrommelSpiel>> f = [] {
        std::vector<Feld<TrommelSpiel>> v;
        v.push_back(ganzzahl<TrommelSpiel>(kNameNoteMin, "Lowest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &TrommelSpiel::note));
        v.push_back(ganzzahl<TrommelSpiel>(kNameNoteMax, "Highest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &TrommelSpiel::noteMax));
        v.push_back(schalter<TrommelSpiel>("tunable", "Tuned by the key", "Keys", &TrommelSpiel::stimmbar));
        v.push_back(zahl<TrommelSpiel>("modesUpTo", "Modes up to", "Heads", "Hz", 100.0, 20000.0, &TrommelSpiel::bisHz));
        v.push_back(Feld<TrommelSpiel>{angabe("shellVolume", "Shell volume", "Shell", WertArt::Zahl, "m³", 0.0, 1.0),
                                       [](const TrommelSpiel& s) { return Wert{s.kessel.volumenM3}; },
                                       [](TrommelSpiel& s, const Wert& w) {
                                           if (const auto* d = std::get_if<double>(&w)) s.kessel.volumenM3 = *d;
                                       }});
        v.push_back(zahl<TrommelSpiel>("strikePosition", "Strike position", "Stroke", "r/a", 0.0, 1.0, &TrommelSpiel::schlagstelleRel));
        v.push_back(schlaegelZahl("malletMass", "Mallet mass", "kg", 1e-4, 10.0, &Stossanreger::masseKg));
        v.push_back(schlaegelZahl("malletStiffness", "Mallet stiffness", "N/mm^p", 1e-6, 1e9, &Stossanreger::q0));
        v.push_back(schlaegelZahl("malletExponent", "Mallet exponent", "", 1.0, 6.0, &Stossanreger::p));
        v.push_back(schlaegelZahl("malletHysteresis", "Mallet hysteresis", "", 0.0, 0.999, &Stossanreger::eps));
        v.push_back(schlaegelZahl("malletRelaxation", "Mallet relaxation", "s", 0.0, 1.0, &Stossanreger::tauS));
        v.push_back(Feld<TrommelSpiel>{angabe("malletHuntCrossley", "Hunt–Crossley contact", "Stroke", WertArt::Schalter),
                                       [](const TrommelSpiel& s) { return Wert{s.schlaegel.huntCrossley}; },
                                       [](TrommelSpiel& s, const Wert& w) {
                                           if (const auto* b = std::get_if<bool>(&w)) s.schlaegel.huntCrossley = *b;
                                       }});
        v.push_back(schlaegelZahl("malletDampingHc", "Hunt–Crossley damping", "s/mm", 0.0, 1.0, &Stossanreger::lambdaHc));
        v.push_back(zahl<TrommelSpiel>("velocityMax", "Speed at full strength", "Stroke", "m/s", 0.01, 20.0, &TrommelSpiel::tempoFf));
        v.push_back(zahl<TrommelSpiel>("glideStiffness", "Head stiffness E·h", "Glide", "N/m", 0.0, 1e7, &TrommelSpiel::youngH));
        v.push_back(zahl<TrommelSpiel>("poisson", "Poisson ratio", "Glide", "", 0.0, 0.49, &TrommelSpiel::nu));
        v.push_back(schalter<TrommelSpiel>("snares", "Snares", "Snares", &TrommelSpiel::teppich));
        v.push_back(zahl<TrommelSpiel>("snaresLevel", "Snares level", "Snares", "", 0.0, 10.0, &TrommelSpiel::teppichPegel));
        v.push_back(zahl<TrommelSpiel>("highLevel", "High part level", "Noise", "per 100 N", 0.0, 10.0, &TrommelSpiel::hochPegel));
        v.push_back(zahl<TrommelSpiel>("clickLevel", "Rim click level", "Noise", "", 0.0, 10.0, &TrommelSpiel::klickPegel));
        v.push_back(zahl<TrommelSpiel>("rollRate", "Roll strokes per second", "Roll", "1/s", 1.0, 50.0, &TrommelSpiel::wirbelHz));
        v.push_back(ganzzahl<TrommelSpiel>("bounces", "Bounces per stroke", "Roll", "", 1, 16, &TrommelSpiel::prellen));
        return v;
    }();
    return f;
}

/// Herkunft und Quellen der Vorgaben (tafelweit, WZ1 a; je Instrument stehen sie in `Vorgaben.cpp`).
void belege(ParameterAngabe& a)
{
    const auto setze = [&](Belegkraft b, std::string h, std::vector<std::string> q) {
        a.belegkraft = b;
        a.herkunft = std::move(h);
        a.quellen = std::move(q);
    };
    const std::string& n = a.name;
    if (n == kNameNoteMin || n == kNameNoteMax)
        setze(Belegkraft::Offen, "the key of the role in a General MIDI kit (low tom 45, bass drum 36, snare 38); timpani 25 inch C3–G3 (open)",
              {"lumiviz"});
    else if (n == "tunable")
        setze(Belegkraft::Offen, "timpani: the key sets the tension, mode (1,1) on the key (IC F-5 a); air load not solved again (open)", {"lumiviz"});
    else if (n == kFelle)
        setze(Belegkraft::Offen,
              "tom R 0.165 m, T 2000/1800 N/m, d1 1.25, d3 5e-4 (Avanzini & Marogna 2010 p. 898 — report only); timpani a 0.32 m, "
              "T 5360 N/m, 0.262 kg/m² (Packer 1993 — numbers only, report); snare 3325 N/m, 0.26 kg/m² (Bilbao, Torin & Chatziioannou — "
              "report only); tabla inlay (Sathej & Adhikari 2008 — report only, the reading of sigma squared open); sizes otherwise from "
              "retailers, tensions open",
              {"avanzini2010", "packer1993", "bilbao2014", "sathej2008"});
    else if (n == "modesUpTo")
        setze(Belegkraft::Rechnung, "the modes up to this frequency per instrument (IC Vorgaben.cpp), above it the statistical part", {"lumiviz"});
    else if (n == "shellVolume")
        setze(Belegkraft::Offen, "pi·R²·depth (depths open); timpani as half ellipsoid with L/a = 1.2 (Packer 1993 p. 48 — numbers only); "
                                 "conga and bongo open (0)",
              {"packer1993", "lumiviz"});
    else if (n == "strikePosition")
        setze(Belegkraft::Offen, "timpani a quarter of the radius from the rim (report only); otherwise open", {"lumiviz"});
    else if (n.rfind("mallet", 0) == 0)
        setze(Belegkraft::Offen,
              "timpani mallet 0.028 kg, K = 1.6e8, alpha 2.54 (Bilbao, Torin & Chatziioannou — report only); stick, felt, finger, hand "
              "set at the contact time 5–8 ms mf (Dahl 2011) and 3–6 ms (Wagner 2006), open",
              {"bilbao2014", "dahl2011", "wagner2006"});
    else if (n == "velocityMax")
        setze(Belegkraft::Offen, "ff speed: stick 8 m/s (Dahl 2011: an accent about twice the speed), timpani 5 m/s (Torin et al. — report only)",
              {"dahl2011", "torin2014"});
    else if (n == "glideStiffness" || n == "poisson")
        setze(Belegkraft::Offen, "pitch glide after Marogna, Avanzini & Bank 2010 eq. 20 (source); E 3.5 GPa, h 0.2 mm, nu 0.2 "
                                 "(Avanzini & Marogna 2010 p. 898 — report only)",
              {"marogna2010", "avanzini2010"});
    else if (n == "bounces")
        setze(Belegkraft::Offen, "1 (snare buzz 4–9 after Miura 2023 — report only)", {"miura2023"});
    else
        setze(Belegkraft::Offen, "IC Vorgaben.cpp (switch or setting; open)", {"lumiviz"});
}

Parametertafel baueTafel(const TrommelSpiel& vorgabe)
{
    Parametertafel t;
    t.kern = std::string(kKernName);
    t.titel = "Membrane";
    t.rechenFassung = 1;
    t.noteMin = kUmfangMin;
    t.noteMax = kUmfangMax;
    t.gruppen = {"Keys", "Heads", "Shell", "Stroke", "Glide", "Snares", "Noise", "Roll", "Output"};
    t.umfangVon = std::string(kNameNoteMin);
    t.umfangBis = std::string(kNameNoteMax);
    const Werte v = werteAus(vorgabe);
    for (const std::string& gruppe : t.gruppen)
    {
        if (gruppe == "Heads")
        {
            ParameterAngabe l = angabe(kFelle, "Heads", "Heads", WertArt::Liste);
            for (const Feld<FellMitBelag>& f : fellFelder())
            {
                ParameterAngabe e = f.angabe;
                e.vorgabe = f.lies(FellMitBelag{});
                l.felder.push_back(std::move(e));
            }
            l.eintraegeMin = 1;
            l.eintraegeMax = 2;
            l.vorgabe = *v.finde(kFelle);
            belege(l);
            t.werte.push_back(std::move(l));
        }
        for (const Feld<TrommelSpiel>& f : felder())
            if (f.angabe.gruppe == gruppe)
            {
                ParameterAngabe a = f.angabe;
                a.vorgabe = f.lies(vorgabe);
                belege(a);
                a.klangbestimmend = a.name != kNameNoteMin && a.name != kNameNoteMax;
                t.werte.push_back(std::move(a));
            }
    }
    ParameterAngabe bezug = angabe(kBezug, "Output reference", "Output", WertArt::Zahl, "", 1e-12, 1e12);
    bezug.vorgabe = 1.0;
    bezug.belegkraft = Belegkraft::Rechnung;
    bezug.herkunft = "normiere: peak 0.5 at the middle key of the instrument, level 0, two rounds (the noise parts follow it; IK2-10 a)";
    bezug.klangbestimmend = false;
    t.werte.push_back(std::move(bezug));
    t.bezugWert = std::string(kBezug);
    t.bezugRunden = 2;
    return t;
}

bool getreten(const Tonauftrag& auftrag, std::string_view spurName, Frames beginn)
{
    const Spur* s = findeSpur(auftrag, spurName);
    return s != nullptr && spurWert(*s, beginn, 0.0) >= 0.5;
}

}  // namespace

const std::vector<Vorlage>& vorlagen()
{
    static const std::vector<Vorlage> v = {
        {"low_tom", "Low tom", baueVorgabeTom()},       {"timpani", "Timpani", baueVorgabePauke()},
        {"bass_drum", "Bass drum", baueVorgabeBassdrum()}, {"snare_drum", "Snare drum", baueVorgabeSnare()},
        {"tabla", "Tabla", baueVorgabeTabla()},          {"conga", "Conga", baueVorgabeConga()},
        {"bongo", "Bongo", baueVorgabeBongo()},
    };
    return v;
}

const KernTyp& kernTyp()
{
    static const KernTyp typ = [] {
        KernTyp t;
        t.tafel = baueTafel(vorlagen().front().spiel);
        t.auskunft.rechenweg = Rechenweg::Note;
        t.auskunft.stufeImNamen = true;
        t.auskunft.spuren = {std::string(kSpurSchlag), std::string(kSpurWirbel), std::string(kSpurOhneTeppich), std::string(spur::kPitch)};
        t.auskunft.innere = {"contact", "glide"};
        t.auskunft.vorbereitungTeuer = true;  // die Moden je Instrument 0,05–0,3 s (S179)
        t.auskunft.koerperAnDerSchicht = false;
        t.erzeuge = [](const Werte& w) -> std::shared_ptr<const Tonkern> { return std::make_shared<const TrommelKern>(spielAus(w)); };
        return t;
    }();
    return typ;
}

Werte werteAus(const TrommelSpiel& spiel)
{
    Werte w;
    const auto fuege = [&](const std::string& gruppe) {
        for (const Feld<TrommelSpiel>& f : felder())
            if (f.angabe.gruppe == gruppe) w.felder.emplace_back(f.angabe.name, f.lies(spiel));
    };
    fuege("Keys");
    Liste l;
    for (std::size_t i = 0; i < spiel.felle.size(); ++i)
    {
        const FellMitBelag fb{spiel.felle[i], i < spiel.belaege.size() ? spiel.belaege[i] : Belag{}};
        Werte e;
        for (const Feld<FellMitBelag>& f : fellFelder()) e.felder.emplace_back(f.angabe.name, f.lies(fb));
        l.eintraege.push_back(std::move(e));
    }
    w.felder.emplace_back(std::string(kFelle), std::move(l));
    for (const std::string gruppe : {"Heads", "Shell", "Stroke", "Glide", "Snares", "Noise", "Roll"}) fuege(gruppe);
    w.felder.emplace_back(std::string(kBezug), spiel.ausgangBezug);
    return w;
}

TrommelSpiel spielAus(const Werte& werte)
{
    TrommelSpiel s;
    for (const Feld<TrommelSpiel>& f : felder())
        if (const Wert* w = werte.finde(f.angabe.name)) f.setze(s, *w);
    if (const Wert* w = werte.finde(kFelle))
        if (const Liste* l = std::get_if<Liste>(w))
        {
            s.felle.clear();
            s.belaege.clear();
            for (const Werte& e : l->eintraege)
            {
                FellMitBelag fb;
                for (const Feld<FellMitBelag>& f : fellFelder())
                    if (const Wert* x = e.finde(f.angabe.name)) f.setze(fb, *x);
                fb.fell.belag = belagFunktion(fb.belag);
                s.felle.push_back(std::move(fb.fell));
                s.belaege.push_back(fb.belag);
            }
        }
    if (const Wert* w = werte.finde(kBezug))
        if (const auto* d = std::get_if<double>(w)) s.ausgangBezug = *d;
    return s;
}

Schlag schlagAus(double wert)
{
    const int i = std::clamp(static_cast<int>(std::lround(wert)), 0, 5);
    constexpr Schlag kSchlaege[6] = {Schlag::Normal, Schlag::Mitte, Schlag::Rand, Schlag::Randschlag, Schlag::Reifen, Schlag::Gedaempft};
    return kSchlaege[i];
}

TrommelAnschlag anschlagAus(const TrommelSpiel& spiel, const Tonauftrag& auftrag)
{
    TrommelAnschlag a;
    a.rate = static_cast<int>(auftrag.rate);
    // TM1 a: die Geschwindigkeit folgt der Stufe in dB (bei Stufe 0 genau tempoFf, wie IC mit staerke 1)
    a.tempo = spiel.tempoFf * anschlagFaktor(auftrag.stufe);
    a.staerke = 1.0;
    if (auftrag.noten.empty()) return a;
    const KettenNote& n = auftrag.noten.front();
    a.note = n.note;
    if (const Spur* s = findeSpur(auftrag, kSpurSchlag)) a.schlag = schlagAus(spurWert(*s, n.beginn, 0.0));
    if (getreten(auftrag, kSpurWirbel, n.beginn)) a.wirbelS = static_cast<double>(n.ende - n.beginn) / auftrag.rate;
    a.ohneTeppich = getreten(auftrag, kSpurOhneTeppich, n.beginn);
    if (const Spur* p = findeSpur(auftrag, spur::kPitch); p != nullptr && !p->punkte.empty())
    {
        a.pitch.name = p->name;
        for (const SpurPunkt& x : p->punkte) a.pitch.punkte.push_back(SpurPunkt{x.frame - n.beginn, x.wert});
    }
    return a;
}

TrommelKern::TrommelKern(TrommelSpiel spiel) : m_spiel(std::move(spiel)) { bereite(m_spiel); }

Frames TrommelKern::nachklang(const Tonauftrag& auftrag, Frames hoechstens) const
{
    if (auftrag.noten.empty()) return std::min<Frames>(hoechstens, 1);
    const TrommelAnschlag a = anschlagAus(m_spiel, auftrag);
    const KettenNote& n = auftrag.noten.front();
    const Frames halten = n.ende - n.beginn;
    const Frames rest = trommelNachklang(m_spiel, a, hoechstens + halten) - halten;
    return std::clamp<Frames>(rest, 1, std::max<Frames>(1, hoechstens));
}

void TrommelKern::rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere) const
{
    if (auftrag.noten.empty())
    {
        ton.assign(static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames)), 0.0f);
        return;
    }
    const auto will = [&](std::string_view name) {
        return innere != nullptr && std::find(innere->gewuenscht.begin(), innere->gewuenscht.end(), name) != innere->gewuenscht.end();
    };
    const bool messen = will("contact") || will("glide");
    TrommelMessung messung;
    rechneTrommelTon(m_spiel, anschlagAus(m_spiel, auftrag), ton, auftrag.frames, messen ? &messung : nullptr);
    if (will("contact")) innere->ergebnis.push_back(InnereGroesse{"contact", "s, N", {messung.kontaktS, messung.kraftSpitzeN}});
    if (will("glide")) innere->ergebnis.push_back(InnereGroesse{"glide", "cent", {messung.gleitenCent}});
}

}  // namespace lumi::kern::trommel
