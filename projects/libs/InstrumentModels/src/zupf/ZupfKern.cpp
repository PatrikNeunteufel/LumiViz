/**
 * @file ZupfKern.cpp
 * @brief InstrumentModels, Kern `pluckedString`: Tafel, Werte ↔ Spiel, Adapter, Vorlagen (34k-2; Herleitung im Kopf von
 *        `ZupfKern.hpp`)
 */
#include "InstrumentModels/zupf/ZupfKern.hpp"

#include "InstrumentCore/Anschlag.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <numbers>
#include <utility>

namespace lumi::kern::zupf
{

namespace
{

constexpr int kUmfangMin = 21;   // A0
constexpr int kUmfangMax = 108;  // C8

constexpr std::string_view kNameNoteMin = "noteMin";
constexpr std::string_view kNameNoteMax = "noteMax";
constexpr std::string_view kSteg = "bridgeModes";
constexpr std::string_view kMitschwingen = "sympatheticStrings";

/// Die Gruppen der Tafel in ihrer Reihenfolge (`werteAus` schreibt in derselben).
constexpr std::string_view kGruppen[] = {"Keys", "String", "Pluck", "Bridge", "Courses", "Sympathetic strings", "Tunable", "Output"};

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

/// Ein Wert eines Spiels (oder eines Eintrags einer Liste) in der Tafel: Angabe, Lesen, Setzen.
template <typename S>
struct Feld
{
    ParameterAngabe angabe;
    std::function<Wert(const S&)> lies;
    std::function<void(S&, const Wert&)> setze;
};

template <typename S>
Feld<S> zahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
             double S::* feld)
{
    return Feld<S>{angabe(name, titel, gruppe, WertArt::Zahl, einheit, von, bis), [feld](const S& s) { return Wert{s.*feld}; },
                   [feld](S& s, const Wert& w) {
                       if (const auto* d = std::get_if<double>(&w)) s.*feld = *d;
                   }};
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

Feld<ZupfSpiel> kurve(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von,
                      double bis, Skala skala, Kurve ZupfSpiel::* feld)
{
    Feld<ZupfSpiel> f{angabe(name, titel, gruppe, WertArt::Kurve, einheit, von, bis), [feld](const ZupfSpiel& s) { return Wert{s.*feld}; },
                      [feld](ZupfSpiel& s, const Wert& w) {
                          if (const auto* k = std::get_if<Kurve>(&w)) s.*feld = *k;
                      }};
    f.angabe.skala = skala;
    return f;
}

/// Die Felder einer Stegmode (ein Eintrag der Liste `bridgeModes`).
const std::vector<Feld<StegMode>>& stegFelder()
{
    static const std::vector<Feld<StegMode>> f = {
        zahl<StegMode>("frequency", "Frequency", "", "Hz", 1.0, 20000.0, &StegMode::hz),
        zahl<StegMode>("q", "Quality factor", "", "", 0.5, 1e5, &StegMode::q),
        zahl<StegMode>("peak", "Peak admittance", "", "s/kg", 0.0, 100.0, &StegMode::spitze),
        zahl<StegMode>("angle", "Direction", "", "°", -90.0, 90.0, &StegMode::winkelGrad),
    };
    return f;
}

/// Eine mitschwingende Saite (ein Eintrag der Liste `sympatheticStrings`).
struct Mitschwinger
{
    int note = 60;
};

const std::vector<Feld<Mitschwinger>>& mitschwingFelder()
{
    static const std::vector<Feld<Mitschwinger>> f = {
        ganzzahl<Mitschwinger>("note", "Pitch of the open string", "", "MIDI", kUmfangMin, kUmfangMax, &Mitschwinger::note),
    };
    return f;
}

template <typename E>
ParameterAngabe listenAngabe(std::string_view name, std::string_view titel, std::string_view gruppe, const std::vector<Feld<E>>& felder,
                             std::size_t min, std::size_t max)
{
    ParameterAngabe l = angabe(name, titel, gruppe, WertArt::Liste);
    for (const Feld<E>& f : felder)
    {
        ParameterAngabe e = f.angabe;
        e.vorgabe = f.lies(E{});
        l.felder.push_back(std::move(e));
    }
    l.eintraegeMin = min;
    l.eintraegeMax = max;
    return l;
}

template <typename E>
Liste listeAus(const std::vector<E>& eintraege, const std::vector<Feld<E>>& felder)
{
    Liste l;
    for (const E& x : eintraege)
    {
        Werte e;
        for (const Feld<E>& f : felder) e.felder.emplace_back(f.angabe.name, f.lies(x));
        l.eintraege.push_back(std::move(e));
    }
    return l;
}

template <typename E>
std::vector<E> eintraegeAus(const Wert& w, const std::vector<Feld<E>>& felder)
{
    std::vector<E> v;
    if (const Liste* l = std::get_if<Liste>(&w))
        for (const Werte& e : l->eintraege)
        {
            E x;
            for (const Feld<E>& f : felder)
                if (const Wert* y = e.finde(f.angabe.name)) f.setze(x, *y);
            v.push_back(x);
        }
    return v;
}

/// Alle Werte des Spiels in der Reihenfolge der Tafel (Gruppe fuer Gruppe), ausser dem Bezug.
const std::vector<Feld<ZupfSpiel>>& felder()
{
    static const std::vector<Feld<ZupfSpiel>> f = [] {
        std::vector<Feld<ZupfSpiel>> v = {
            ganzzahl<ZupfSpiel>(kNameNoteMin, "Lowest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &ZupfSpiel::noteMin),
            ganzzahl<ZupfSpiel>(kNameNoteMax, "Highest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &ZupfSpiel::noteMax),
            kurve("stringLength", "String length per key", "String", "m", 0.01, 10.0, Skala::Logarithmisch, &ZupfSpiel::laengeJeNote),
            kurve("stringTension", "String tension per key", "String", "N", 0.1, 5000.0, Skala::Logarithmisch, &ZupfSpiel::zugJeNote),
            zahl<ZupfSpiel>("stringQ", "String quality factor", "String", "", 1.0, 1e6, &ZupfSpiel::guete),
            kurve("pluckPoint", "Plucking point per key", "Pluck", "", 0.01, 0.99, Skala::Linear, &ZupfSpiel::zupfpunktJeNote),
            zahl<ZupfSpiel>("pluckAngle", "Angle at release", "Pluck", "°", 0.0, 90.0, &ZupfSpiel::polarisationGrad),
            zahl<ZupfSpiel>("pluckDuration", "Duration of the push", "Pluck", "s", 1e-5, 0.1, &ZupfSpiel::stossS),
            zahl<ZupfSpiel>("forcePp", "Force pp", "Pluck", "N", 1e-4, 1000.0, &ZupfSpiel::kraftPpN),
            zahl<ZupfSpiel>("forceFf", "Force ff", "Pluck", "N", 1e-4, 1000.0, &ZupfSpiel::kraftFfN),
            zahl<ZupfSpiel>("keyThump", "Key thump", "Pluck", "", 0.0, 100.0, &ZupfSpiel::tastenstoss),
            zahl<ZupfSpiel>("dampingTime", "Damping at the end of the note", "Pluck", "s", 1e-4, 10.0, &ZupfSpiel::daempfenS),
        };
        Feld<ZupfSpiel> steg{listenAngabe<StegMode>(kSteg, "Bridge modes", "Bridge", stegFelder(), 0, 64),
                             [](const ZupfSpiel& s) { return Wert{listeAus(s.steg, stegFelder())}; },
                             [](ZupfSpiel& s, const Wert& w) { s.steg = eintraegeAus(w, stegFelder()); }};
        v.push_back(std::move(steg));
        Feld<ZupfSpiel> saiten{angabe("courseStrings", "Strings per course", "Courses", WertArt::Ganzzahl, "", 1, 3),
                               [](const ZupfSpiel& s) { return Wert{static_cast<std::int64_t>(s.chor.saiten)}; },
                               [](ZupfSpiel& s, const Wert& w) {
                                   if (const auto* g = std::get_if<std::int64_t>(&w)) s.chor.saiten = static_cast<int>(*g);
                               }};
        v.push_back(std::move(saiten));
        Feld<ZupfSpiel> verstimmung{angabe("courseDetune", "Detuning within the course", "Courses", WertArt::Zahl, "cent", -50.0, 50.0),
                                    [](const ZupfSpiel& s) { return Wert{s.chor.verstimmungCent}; },
                                    [](ZupfSpiel& s, const Wert& w) {
                                        if (const auto* d = std::get_if<double>(&w)) s.chor.verstimmungCent = *d;
                                    }};
        v.push_back(std::move(verstimmung));
        Feld<ZupfSpiel> mit{listenAngabe<Mitschwinger>(kMitschwingen, "Sympathetic strings", "Sympathetic strings", mitschwingFelder(), 0, 64),
                            [](const ZupfSpiel& s) {
                                std::vector<Mitschwinger> m;
                                for (const int n : s.mitschwingNoten) m.push_back({n});
                                return Wert{listeAus(m, mitschwingFelder())};
                            },
                            [](ZupfSpiel& s, const Wert& w) {
                                s.mitschwingNoten.clear();
                                for (const Mitschwinger& m : eintraegeAus(w, mitschwingFelder())) s.mitschwingNoten.push_back(m.note);
                            }};
        v.push_back(std::move(mit));
        Feld<ZupfSpiel> laufzeit = kurve(kLaufzeit, "Delay factor per key", "Tunable", "", 0.5, 2.0, Skala::Logarithmisch, &ZupfSpiel::laufzeitJeNote);
        v.push_back(std::move(laufzeit));
        return v;
    }();
    return f;
}

/// Belegkraft und Herkunft der Vorgaben, die keine Kurve sind (tafelweit, der Text nennt alle fuenf Instrumente; IC
/// `Vorgaben.cpp`, `Zupf.hpp`, `modell.md`). Die Quellen der Klasse stehen noch nicht im Register (34k-4) — die
/// Schluessel bleiben `lumiviz`. Eine Kurve traegt ihre eigene Herkunft.
void belege(ParameterAngabe& a)
{
    const auto setze = [&](Belegkraft b, std::string h) {
        a.belegkraft = b;
        a.herkunft = std::move(h);
    };
    const std::string& n = a.name;
    if (n == kNameNoteMin || n == kNameNoteMax)
        setze(Belegkraft::Offen, "the range of each instrument as in IC Vorgaben.cpp: harpsichord FF–f3 (Taskin, report only), concert harp "
                                 "C1–G7, classical guitar E2–B5, lute G2–G5, oud E2–F5 (open)");
    else if (n == "stringQ")
        setze(Belegkraft::Offen, "harpsichord 1500, harp 600 (gut), guitar 800, lute and oud 500 (open)");
    else if (n == "pluckAngle")
        setze(Belegkraft::Quelle, "harpsichord 18° (Chadefaux et al. 2013 Tab. 1: 17 ± 2° slow, 20 ± 1° fast); harp 25° (7–43°, report "
                                  "only); guitar, lute, oud 30° (rest/free stroke after Scherrer & Depalle — open)");
    else if (n == "pluckDuration")
        setze(Belegkraft::Offen, "harpsichord 4 ms (interaction 12 / 7.9 ms, Chadefaux — the release faster), harp 3 ms, the others 2 ms "
                                 "(open; a push of fixed length, cause U1 of the slope over the range, S193)");
    else if (n == "forcePp" || n == "forceFf")
        setze(Belegkraft::Rechnung, "harpsichord 0.6–1.0 N (T·δ·(1/x + 1/(L − x)) ≈ 0.8 N at G4 from Chadefaux Tab. 1/2), harp 1–8 N "
                                    "(≈ 4 N at D♭2 from Chadefaux et al. 2012), guitar, lute, oud 0.5–4 N (open)");
    else if (n == "keyThump")
        setze(Belegkraft::Offen, "harpsichord 0.3 (Penttinen: +10 dB in the modes below 300 Hz per step — open; IC F-4 a); 0 = none");
    else if (n == "dampingTime")
        setze(Belegkraft::Offen, "damper or hand within this time (IC F-7 a): harpsichord 60 ms, harp 150 ms, the others 80 ms (open)");
    else if (n == kSteg)
        setze(Belegkraft::Offen, "harpsichord: Helmholtz 37 Hz and T60 235 ms at the 8′ bridge (Bader et al. 2025, source), the other "
                                 "frequencies and peaks open; guitar: the coupled peaks of the Classical body (Wright, TOP26 Fig. 7.1) with "
                                 "their levels, Q, peak and direction open; harp, lute, oud open");
    else if (n == "courseStrings" || n == "courseDetune")
        setze(Belegkraft::Offen, "lute 2 strings 4 cent, oud 2 strings 3 cent apart (IC F-9 a, detuning open); the others one string");
    else if (n == kMitschwingen)
        setze(Belegkraft::Offen, "only in a clip (rechneZupfClip, IC K2 F-3 a): the guitar its six open strings E2 A2 D3 G3 B3 E4, the harp all "
                                 "47 strings in C major (pedals in the middle — open); without effect on a single note until 34k-7 (GZ3 a)");
    else
        setze(Belegkraft::Offen, "IC Vorgaben.cpp (open)");
}

/// QM1 a: die Quellen eines Werts ohne Kurve in einer Vorlage, sonst die eigene Vorgabe.
std::vector<std::string> quellenIn(const Vorlage& v, std::string_view name)
{
    for (const auto& [n, q] : v.quellen)
        if (n == name) return q;
    return {"lumiviz"};
}

Parametertafel baueTafel(const Vorlage& vorlage)
{
    const ZupfSpiel& vorgabe = vorlage.spiel;
    Parametertafel t;
    t.kern = std::string(kKernName);
    t.titel = "Plucked string";
    t.rechenFassung = 1;
    t.noteMin = kUmfangMin;
    t.noteMax = kUmfangMax;
    for (const std::string_view g : kGruppen) t.gruppen.emplace_back(g);
    t.umfangVon = std::string(kNameNoteMin);
    t.umfangBis = std::string(kNameNoteMax);
    std::vector<std::string> stimmtAus;  // woraus `delayFactor` gestimmt wird: alles ausser den mitschwingenden Saiten
    for (const std::string& gruppe : t.gruppen)
        for (const Feld<ZupfSpiel>& f : felder())
            if (f.angabe.gruppe == gruppe)
            {
                ParameterAngabe a = f.angabe;
                if (a.name == kLaufzeit)
                {
                    a.vorgabe = Kurve{};
                    a.gestimmt = true;
                    a.belegkraft = Belegkraft::Rechnung;
                    a.herkunft = "tuned through the model: the factor on the delays so that the peak of the fundamental lies on the key "
                                 "(IC stimme: every third key, the peak within ±60 cent in 0.1–0.5 s, at most three rounds)";
                    a.abhaengigVon = stimmtAus;
                }
                else
                {
                    a.vorgabe = f.lies(vorgabe);
                    if (a.art != WertArt::Kurve) belege(a);
                    else if (const Kurve* k = std::get_if<Kurve>(&a.vorgabe))
                    {
                        a.belegkraft = k->belegkraft;
                        a.herkunft = k->herkunft;
                    }
                    a.quellen = quellenIn(vorlage, a.name);
                    a.klangbestimmend = a.name != kNameNoteMin && a.name != kNameNoteMax;
                    if (a.name != kMitschwingen) stimmtAus.push_back(a.name);
                }
                t.werte.push_back(std::move(a));
            }
    ParameterAngabe bezug = angabe(kBezug, "Output reference", "Output", WertArt::Zahl, "", 1e-12, 1e12);
    bezug.vorgabe = 1.0;
    bezug.belegkraft = Belegkraft::Rechnung;
    bezug.herkunft = "normiere: peak 0.5 at the middle key of the instrument, level 0 (strength 1), held 1 s (IK2-10 a; IC: 0.5 s — the "
                     "same for all five, J76)";
    bezug.klangbestimmend = false;
    t.werte.push_back(std::move(bezug));
    t.bezugWert = std::string(kBezug);
    return t;
}

}  // namespace

const std::vector<Vorlage>& vorlagen()
{
    static const std::vector<Vorlage> v = {
        {"harpsichord", "Harpsichord", baueVorgabeCembalo(), {}, false},
        {"concert_harp", "Concert harp", baueVorgabeHarfe(), {}, false},
        {"classical_guitar", "Classical guitar", baueVorgabeGitarre(), {}, false},
        {"lute", "Lute", baueVorgabeLaute(), {}, false},
        {"oud", "Oud", baueVorgabeOud(), {}, false},
    };
    return v;
}

Werte werteVon(const Vorlage& vorlage)
{
    Werte w = werteAus(vorlage.spiel);
    for (const ParameterAngabe& a : kernTyp().tafel.werte)
        if (a.art != WertArt::Kurve) w.setzeQuellen(a.name, quellenIn(vorlage, a.name));
    return w;
}

const KernTyp& kernTyp()
{
    static const KernTyp typ = [] {
        KernTyp t;
        t.tafel = baueTafel(vorlagen().front());
        t.auskunft.rechenweg = Rechenweg::Note;
        t.auskunft.stufeImNamen = true;
        t.auskunft.spuren = {};
        t.auskunft.innere = {};
        t.auskunft.vorbereitungTeuer = false;
        t.auskunft.koerperAnDerSchicht = false;  // GZ4 a: der Steg rechnet im Kern
        t.erzeuge = [](const Werte& w) -> std::shared_ptr<const Tonkern> { return std::make_shared<const ZupfKern>(spielAus(w)); };
        return t;
    }();
    return typ;
}

Werte werteAus(const ZupfSpiel& spiel)
{
    Werte w;
    for (const std::string_view gruppe : kGruppen)
        for (const Feld<ZupfSpiel>& f : felder())
            if (f.angabe.gruppe == gruppe) w.felder.emplace_back(f.angabe.name, f.lies(spiel));
    w.felder.emplace_back(std::string(kBezug), spiel.ausgangBezug);
    return w;
}

ZupfSpiel spielAus(const Werte& werte)
{
    ZupfSpiel s;
    for (const Feld<ZupfSpiel>& f : felder())
        if (const Wert* w = werte.finde(f.angabe.name)) f.setze(s, *w);
    if (const Wert* w = werte.finde(kBezug))
        if (const auto* d = std::get_if<double>(w)) s.ausgangBezug = *d;
    return s;
}

double staerkeBei(const ZupfSpiel& spiel, int stufe)
{
    if (stufe >= 0 || !(spiel.kraftFfN > spiel.kraftPpN) || !(spiel.kraftPpN > 0.0)) return 1.0;
    return std::clamp(1.0 + std::log(anschlagFaktor(stufe)) / std::log(spiel.kraftFfN / spiel.kraftPpN), 0.0, 1.0);
}

ZupfAnschlag anschlagAus(const ZupfSpiel& spiel, const Tonauftrag& auftrag)
{
    ZupfAnschlag a;
    a.rate = static_cast<int>(auftrag.rate);
    a.staerke = staerkeBei(spiel, auftrag.stufe);
    if (auftrag.noten.empty()) return a;
    const KettenNote& n = auftrag.noten.front();
    a.note = n.note;
    a.haltenS = static_cast<double>(n.ende - n.beginn) / auftrag.rate;
    return a;
}

ZupfKern::ZupfKern(ZupfSpiel spiel) : m_spiel(std::move(spiel)) {}

Frames ZupfKern::nachklang(const Tonauftrag& auftrag, Frames hoechstens) const
{
    if (auftrag.noten.empty()) return std::min<Frames>(hoechstens, 1);
    double moden = 0.0;  // die langsamste Stegmode bis −60 dB
    for (const StegMode& m : m_spiel.steg)
        if (m.hz > 0.0 && m.q > 0.0) moden = std::max(moden, std::log(1000.0) * m.q / (std::numbers::pi * m.hz));
    const double s = m_spiel.daempfenS + moden + 0.02;
    return std::clamp<Frames>(static_cast<Frames>(std::ceil(s * auftrag.rate)), 1, std::max<Frames>(1, hoechstens));
}

void ZupfKern::rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere) const
{
    (void)innere;  // der Kern gibt keine inneren Groessen ab
    if (auftrag.noten.empty())
    {
        ton.assign(static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames)), 0.0f);
        return;
    }
    rechneZupfTon(m_spiel, anschlagAus(m_spiel, auftrag), ton, auftrag.frames);
}

}  // namespace lumi::kern::zupf
