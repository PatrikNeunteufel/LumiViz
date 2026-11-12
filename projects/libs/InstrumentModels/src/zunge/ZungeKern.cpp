/**
 * @file ZungeKern.cpp
 * @brief InstrumentModels, Kern `freeReed`: Tafel, Werte ↔ Spiel, Adapter, Vorlagen (34j-2, Rechenfassung 2 seit 34j-6;
 *        Herleitung im Kopf von `ZungeKern.hpp`)
 */
#include "InstrumentModels/zunge/ZungeKern.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <memory>
#include <utility>

namespace lumi::kern::zunge
{

namespace
{

constexpr int kUmfangMin = 21;   // A0
constexpr int kUmfangMax = 108;  // C8

constexpr std::string_view kNameNoteMin = "noteMin";
constexpr std::string_view kNameNoteMax = "noteMax";
constexpr std::string_view kNameBassUnter = "bassBelow";
constexpr std::string_view kChoere = "ranks";
constexpr std::string_view kBassChoere = "bassRanks";
constexpr std::string_view kChorHalbtoene = "semitones";
constexpr std::string_view kChorSchwebung = "beatA4";
constexpr std::string_view kChorPegel = "level";
constexpr std::string_view kWindMin = "windMin";
constexpr std::string_view kWindMax = "windMax";
constexpr std::string_view kBendVentil = "palletBend";

/// Die Gruppen der Tafel in ihrer Reihenfolge (`werteAus` schreibt in derselben).
constexpr std::string_view kGruppen[] = {"Keys", "Reed", "Chamber", "Wind", "Ranks", "Bass ranks", "Case", "Tunable", "Output"};

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

/// Ein Wert des Spiels (oder eines Chors) in der Tafel: Angabe, Lesen, Setzen.
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

template <typename S>
Feld<S> schalter(std::string_view name, std::string_view titel, std::string_view gruppe, bool S::* feld)
{
    return Feld<S>{angabe(name, titel, gruppe, WertArt::Schalter), [feld](const S& s) { return Wert{s.*feld}; },
                   [feld](S& s, const Wert& w) {
                       if (const auto* b = std::get_if<bool>(&w)) s.*feld = *b;
                   }};
}

/// Die Felder eines Chors (ein Eintrag der Listen `ranks` und `bassRanks`).
const std::vector<Feld<Chor>>& chorFelder()
{
    static const std::vector<Feld<Chor>> f = {
        ganzzahl<Chor>(kChorHalbtoene, "Footage in semitones", "", "", -36, 36, &Chor::halbtoene),
        zahl<Chor>(kChorSchwebung, "Beating at A4", "", "Hz", -20.0, 20.0, &Chor::schwebungA4Hz),
        zahl<Chor>(kChorPegel, "Level", "", "", 0.0, 10.0, &Chor::pegel),
    };
    return f;
}

/// Die Werte des Spiels ohne Kurve, ausser den Listen der Choere, in der Reihenfolge der Tafel.
const std::vector<Feld<ZungenSpiel>>& felder()
{
    static const std::vector<Feld<ZungenSpiel>> f = {
        ganzzahl<ZungenSpiel>(kNameNoteMin, "Lowest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &ZungenSpiel::noteMin),
        ganzzahl<ZungenSpiel>(kNameNoteMax, "Highest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &ZungenSpiel::noteMax),
        ganzzahl<ZungenSpiel>(kNameBassUnter, "Bass side below key", "Keys", "MIDI", 0, kUmfangMax + 1, &ZungenSpiel::bassAb),
        zahl<ZungenSpiel>(kWindMin, "Wind at expression 0", "Wind", "Pa", 1.0, 20000.0, &ZungenSpiel::windMinPa),
        zahl<ZungenSpiel>(kWindMax, "Wind at expression 1", "Wind", "Pa", 1.0, 20000.0, &ZungenSpiel::windMaxPa),
        schalter<ZungenSpiel>(kBendVentil, "Bend by the pallet", "Wind", &ZungenSpiel::bendUeberVentil),
        zahl<ZungenSpiel>("caseLowpass", "Case low-pass", "Case", "Hz", 0.0, 20000.0, &ZungenSpiel::gehaeuseTiefpassHz),
    };
    return f;
}

/// Eine Kurve je Taste des Spiels in der Tafel (34j-6): ueber die Tonhoehe der Zunge, logarithmisch; Belegkraft,
/// Herkunft und Quellen traegt die Kurve der Vorlage.
struct KurvenFeld
{
    std::string_view name, titel, gruppe, einheit;
    double von, bis;
    Kurve ZungenSpiel::* feld;
};

const std::vector<KurvenFeld>& kurvenFelder()
{
    static const std::vector<KurvenFeld> f = {
        {"reedLength", "Reed length", "Reed", "m", 1e-3, 0.3, &ZungenSpiel::laengeM},
        {"reedWidth", "Reed width", "Reed", "m", 1e-4, 0.05, &ZungenSpiel::breiteM},
        {"reedMassPerArea", "Reed mass per area", "Reed", "kg/m²", 1e-3, 100.0, &ZungenSpiel::flaechenmasse},
        {"reedDamping", "Reed damping ratio", "Reed", "", 1e-5, 0.5, &ZungenSpiel::zeta},
        {"reedRest", "Rest opening", "Reed", "m", 1e-6, 0.01, &ZungenSpiel::ruheM},
        {"sideGap", "Side gap", "Reed", "m", 1e-7, 0.01, &ZungenSpiel::seitenspaltM},
        {"chamberVolume", "Chamber volume", "Chamber", "m³", 1e-9, 0.1, &ZungenSpiel::kanzelleM3},
        {"channelLength", "Channel length", "Chamber", "m", 1e-4, 1.0, &ZungenSpiel::kanalLaengeM},
        {"channelArea", "Channel area", "Chamber", "m²", 1e-8, 0.01, &ZungenSpiel::kanalFlaecheM2},
        {"valveArea", "Pallet opening", "Chamber", "m²", 1e-8, 0.01, &ZungenSpiel::ventilFlaecheM2},
    };
    return f;
}

/// Eine gestimmte Kurve des Spiels in der Tafel (IK2-11 a), in der Reihenfolge des Stimmers.
struct GestimmtFeld
{
    std::string name, titel, einheit, herkunft;
    Skala skala;
    double von, bis;
    std::function<Kurve&(ZungenSpiel&)> zu;
};

const std::vector<GestimmtFeld>& gestimmtFelder()
{
    static const std::vector<GestimmtFeld> f = [] {
        std::vector<GestimmtFeld> v = {
            {std::string(kEigenfrequenz), "Reed frequency per key", "Hz",
             "tuned through the model: f0 of the reed so that it plays the key at the middle wind (ZV5 a; it sounds \"somewhat below\" "
             "f0, Cottingham)",
             Skala::Logarithmisch, 1.0, 20000.0, [](ZungenSpiel& s) -> Kurve& { return s.eigenfrequenzJeNote; }},
        };
        for (int k = 0; k < kBendStufen; ++k)
        {
            char titel[64];
            std::snprintf(titel, sizeof titel, "Bend at pallet %.2f", bendStufe(k));
            v.push_back({bendStufenName(k), titel, "cent",
                         "tuned through the model (ZV8 a): the pitch against the open pallet at the middle wind with the pallet opening "
                         "2^(-k/4); below the key's limit the value of the limit; 0 without bend",
                         Skala::Linear, -1200.0, 0.0,
                         [k](ZungenSpiel& s) -> Kurve& { return s.bendCentJeStufe[static_cast<std::size_t>(k)]; }});
        }
        v.push_back({std::string(kBendGrenzeOeffnung), "Bend limit, pallet opening", "",
                     "tuned through the model (ZV8 a): the smallest pallet opening at which the reed still speaks at the middle wind, "
                     "within 1 %; 1 without bend",
                     Skala::Logarithmisch, 1e-3, 1.0, [](ZungenSpiel& s) -> Kurve& { return s.bendGrenzeOeffnung; }});
        v.push_back({std::string(kBendGrenzeCent), "Bend limit", "cent",
                     "tuned through the model (ZV8 a): the pitch there against the open pallet — the deepest bend of the key (SK3 a: "
                     "the instrument carries the limit); 0 without bend",
                     Skala::Linear, -1200.0, 0.0, [](ZungenSpiel& s) -> Kurve& { return s.bendGrenzeCent; }});
        for (int k = 0; k < kStaerkeStufen; ++k)
        {
            char titel[64];
            std::snprintf(titel, sizeof titel, "Strength at %d dB", staerkeStufeDb(k));
            v.push_back({staerkeStufenName(k), titel, "",
                         "tuned through the model (ZV10 a): the strength (expression 0 … 1) at which the whole tone of the key sounds "
                         "this many dB quieter than at full strength (momentary loudness of the last 0.4 s of 1 s held, within 0.05 LU); "
                         "0 below the dynamics of the key; strength = this value times the expression (SK4 a)",
                         Skala::Linear, 0.0, 1.0,
                         [k](ZungenSpiel& s) -> Kurve& { return s.staerkeJeStufe[static_cast<std::size_t>(k)]; }});
        }
        v.push_back({std::string(kNachklang), "Release time", "s",
                     "tuned through the model (ZV11 a, ZV13 a): the whole tone of the key, the longest release over the hold times "
                     "0.1, 0.25, 0.5, 1, 2, 4 s and the strengths 0, 0.5, 1 — from the end of the wind until every 5 ms block stays 60 dB "
                     "below the loudest block of the same note",
                     Skala::Logarithmisch, 1e-3, 60.0, [](ZungenSpiel& s) -> Kurve& { return s.nachklangS; }});
        return v;
    }();
    return f;
}

Kurve& kurveIn(ZungenSpiel& s, const GestimmtFeld& g)
{
    return g.zu(s);
}

const Kurve& kurveIn(const ZungenSpiel& s, const GestimmtFeld& g)
{
    return g.zu(const_cast<ZungenSpiel&>(s));  // nur lesend
}

/// Belegkraft und Herkunft der Werte ohne Kurve (tafelweit, der Text nennt alle vier Instrumente); die Kurven tragen
/// ihre eigene (`Vorgaben.cpp`).
void belege(ParameterAngabe& a)
{
    const auto setze = [&](Belegkraft b, std::string h) {
        a.belegkraft = b;
        a.herkunft = std::move(h);
    };
    const std::string& n = a.name;
    if (n == kNameNoteMin || n == kNameNoteMax)
        setze(Belegkraft::Offen, "the range of each instrument as in IC Vorgaben.cpp: accordion F1–A6 (bass side below F3), harmonium "
                                 "C3–F6, melodica F3–C6, harmonica C4–C7 (open)");
    else if (n == kNameBassUnter)
        setze(Belegkraft::Offen, "accordion: the Stradella bass side below F3 (IC F-5 a; open); 0 = none");
    else if (n == kWindMin || n == kWindMax)
        setze(Belegkraft::Offen, "one wind for all keys (ZV2 a), geometric over the expression: accordion and melodica 200–1600 Pa "
                                 "(assumption; the lab axis of Adachi et al. 2026 is 0–2.5 kPa), harmonium 200–1500 Pa (\"0.2-1.5 kPa in a "
                                 "real harmonium\", Puranik & Scavone 2023 p. 1810), harmonica 150–1000 Pa (assumption; Millot & Baumann "
                                 "compute 570 and 783 Pa); every key speaks over its range (S190 34j-5)");
    else if (n == kBendVentil)
        setze(Belegkraft::Quelle, "accordion and melodica (ZV7 a): a half pressed key closes the pallet and lowers the pitch (Adachi et al. "
                                  "2026); the bend over the pallet per key is tuned through the model (ZV8 a); harmonium and harmonica "
                                  "without bend");
    else if (n == kChoere)
        setze(Belegkraft::Quelle, "accordion musette M + M+ beating 2.00 Hz at A4 = 7.9 ct (Hergert p. 4460, read in IC), rising ∝ "
                                  "√(f/440) over the range (open); levels open; the others one 8′ rank");
    else if (n == kBassChoere)
        setze(Belegkraft::Rechnung, "accordion bass side 8′ + 4′ (ZV6 a): the 16′ of F1 would lie at 21.8 Hz, the lowest real reed about "
                                    "E1; the reeds F0–E1 do not speak at 200 Pa (S190 34j-5)");
    else if (n == "caseLowpass")
        setze(Belegkraft::Offen, "harmonium 3 kHz, first order (DAFx23: ten biquads, values not available — open); 0 = none");
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

ParameterAngabe chorListe(std::string_view name, std::string_view titel, std::string_view gruppe, std::size_t min, const Wert& vorgabe)
{
    ParameterAngabe l = angabe(name, titel, gruppe, WertArt::Liste);
    for (const Feld<Chor>& f : chorFelder())
    {
        ParameterAngabe e = f.angabe;
        e.vorgabe = f.lies(Chor{});
        l.felder.push_back(std::move(e));
    }
    l.eintraegeMin = min;
    l.eintraegeMax = 8;
    l.vorgabe = vorgabe;
    return l;
}

Parametertafel baueTafel(const Vorlage& vorlage)
{
    const ZungenSpiel& vorgabe = vorlage.spiel;
    Parametertafel t;
    t.kern = std::string(kKernName);
    t.titel = "Free reed";
    t.rechenFassung = 2;  // S191 (34j-6): Groessen je Taste als Kurven, ein Wind, f₀ am mittleren Druck
    t.noteMin = kUmfangMin;
    t.noteMax = kUmfangMax;
    for (const std::string_view g : kGruppen) t.gruppen.emplace_back(g);
    t.umfangVon = std::string(kNameNoteMin);
    t.umfangBis = std::string(kNameNoteMax);
    const Werte v = werteAus(vorgabe);
    for (const std::string& gruppe : t.gruppen)
    {
        for (const Feld<ZungenSpiel>& f : felder())
            if (f.angabe.gruppe == gruppe)
            {
                ParameterAngabe a = f.angabe;
                a.vorgabe = f.lies(vorgabe);
                belege(a);
                a.quellen = quellenIn(vorlage, a.name);
                a.klangbestimmend = a.name != kNameNoteMin && a.name != kNameNoteMax;
                t.werte.push_back(std::move(a));
            }
        for (const KurvenFeld& k : kurvenFelder())
            if (k.gruppe == gruppe)
            {
                ParameterAngabe a = angabe(k.name, k.titel, k.gruppe, WertArt::Kurve, k.einheit, k.von, k.bis);
                a.skala = Skala::Logarithmisch;
                a.vorgabe = vorgabe.*k.feld;
                a.belegkraft = (vorgabe.*k.feld).belegkraft;
                a.herkunft = (vorgabe.*k.feld).herkunft;
                a.quellen = (vorgabe.*k.feld).quellen;
                t.werte.push_back(std::move(a));
            }
        if (gruppe == "Ranks" || gruppe == "Bass ranks")
        {
            const bool bass = gruppe == "Bass ranks";
            const std::string_view name = bass ? kBassChoere : kChoere;
            ParameterAngabe l = chorListe(name, bass ? "Bass ranks" : "Ranks", gruppe, bass ? 0 : 1, *v.finde(name));
            belege(l);
            l.quellen = quellenIn(vorlage, l.name);
            t.werte.push_back(std::move(l));
        }
        if (gruppe == "Tunable")
            for (const GestimmtFeld& g : gestimmtFelder())
            {
                ParameterAngabe a = angabe(g.name, g.titel, "Tunable", WertArt::Kurve, g.einheit, g.von, g.bis);
                a.skala = g.skala;
                a.vorgabe = Kurve{};
                a.gestimmt = true;
                a.belegkraft = Belegkraft::Rechnung;
                a.herkunft = std::string(g.herkunft);
                t.werte.push_back(std::move(a));
            }
    }
    ParameterAngabe bezug = angabe(kBezug, "Output reference", "Output", WertArt::Zahl, "", 1e-12, 1e12);
    bezug.vorgabe = 1.0;
    bezug.belegkraft = Belegkraft::Rechnung;
    bezug.herkunft = "normiere: peak 0.5 at the middle key of the instrument, level 0 (expression 1), held 1 s (IK2-10 a; IC: 0.6 s at "
                     "the middle above the bass side)";
    bezug.klangbestimmend = false;
    t.werte.push_back(std::move(bezug));
    t.bezugWert = std::string(kBezug);
    return t;
}

Liste chorListeAus(const std::vector<Chor>& choere)
{
    Liste l;
    for (const Chor& c : choere)
    {
        Werte e;
        for (const Feld<Chor>& f : chorFelder()) e.felder.emplace_back(f.angabe.name, f.lies(c));
        l.eintraege.push_back(std::move(e));
    }
    return l;
}

std::vector<Chor> choereAus(const Wert& w)
{
    std::vector<Chor> v;
    if (const Liste* l = std::get_if<Liste>(&w))
        for (const Werte& e : l->eintraege)
        {
            Chor c;
            for (const Feld<Chor>& f : chorFelder())
                if (const Wert* x = e.finde(f.angabe.name)) f.setze(c, *x);
            v.push_back(c);
        }
    return v;
}

/// QM1 a: die Quellen der Werte ohne Kurve je Vorlage (die Kurven tragen ihre eigenen).
QuellenJeWert quellenDerZunge(const ZungenSpiel& s)
{
    QuellenJeWert q;
    if (s.name == "Akkordeon")
    {
        q.emplace_back(std::string(kChoere), std::vector<std::string>{"hergert2023"});
        q.emplace_back(std::string(kBassChoere), std::vector<std::string>{"accordionists-info"});
    }
    if (s.name == "Harmonium")
    {
        q.emplace_back(std::string(kWindMin), std::vector<std::string>{"puranik2023"});
        q.emplace_back(std::string(kWindMax), std::vector<std::string>{"puranik2023"});
    }
    return q;
}

Vorlage vorlageAus(std::string name, std::string titel, ZungenSpiel s)
{
    QuellenJeWert q = quellenDerZunge(s);
    return Vorlage{std::move(name), std::move(titel), std::move(s), std::move(q)};
}

}  // namespace

std::string bendStufenName(int k)
{
    return "bendStep" + std::to_string(k + 1);
}

std::string staerkeStufenName(int k)
{
    return "strengthAtLevel" + std::to_string(-staerkeStufeDb(k));
}

std::vector<std::string> gestimmteNamen()
{
    std::vector<std::string> v;
    for (const GestimmtFeld& g : gestimmtFelder()) v.push_back(g.name);
    return v;
}

const std::vector<Vorlage>& vorlagen()
{
    static const std::vector<Vorlage> v = {
        vorlageAus("accordion", "Accordion", baueVorgabeAkkordeon()),
        vorlageAus("harmonium", "Harmonium", baueVorgabeHarmonium()),
        vorlageAus("melodica", "Melodica", baueVorgabeMelodica()),
        vorlageAus("harmonica", "Harmonica", baueVorgabeMundharmonika()),
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
        t.auskunft.spuren = {std::string(spur::kExpression), std::string(spur::kPitch), std::string(spur::kVibrato), std::string(kSpurBalg)};
        t.auskunft.anregungDauernd = true;  // 34j-7 (ZV3 a, SK4 a): die Expression ist der Wind, kein Pegel
        t.auskunft.innere = {std::string(kInnereSpitze)};
        t.auskunft.vorbereitungTeuer = false;
        t.auskunft.koerperAnDerSchicht = false;
        t.erzeuge = [](const Werte& w) -> std::shared_ptr<const Tonkern> { return std::make_shared<const ZungeKern>(spielAus(w)); };
        return t;
    }();
    return typ;
}

Werte werteAus(const ZungenSpiel& spiel)
{
    Werte w;
    for (const std::string_view gruppe : kGruppen)
    {
        for (const Feld<ZungenSpiel>& f : felder())
            if (f.angabe.gruppe == gruppe) w.felder.emplace_back(f.angabe.name, f.lies(spiel));
        for (const KurvenFeld& k : kurvenFelder())
            if (k.gruppe == gruppe) w.felder.emplace_back(std::string(k.name), spiel.*k.feld);
        if (gruppe == "Ranks") w.felder.emplace_back(std::string(kChoere), chorListeAus(spiel.choere));
        if (gruppe == "Bass ranks") w.felder.emplace_back(std::string(kBassChoere), chorListeAus(spiel.bassChoere));
        if (gruppe == "Tunable")
            for (const GestimmtFeld& g : gestimmtFelder()) w.felder.emplace_back(g.name, kurveIn(spiel, g));
    }
    w.felder.emplace_back(std::string(kBezug), spiel.ausgangBezug);
    return w;
}

ZungenSpiel spielAus(const Werte& werte)
{
    ZungenSpiel s;
    for (const Feld<ZungenSpiel>& f : felder())
        if (const Wert* w = werte.finde(f.angabe.name)) f.setze(s, *w);
    for (const KurvenFeld& k : kurvenFelder())
        if (const Wert* w = werte.finde(k.name))
            if (const Kurve* kurve = std::get_if<Kurve>(w)) s.*k.feld = *kurve;
    if (const Wert* w = werte.finde(kChoere)) s.choere = choereAus(*w);
    if (const Wert* w = werte.finde(kBassChoere)) s.bassChoere = choereAus(*w);
    for (const GestimmtFeld& g : gestimmtFelder())
        if (const Wert* w = werte.finde(g.name))
            if (const Kurve* k = std::get_if<Kurve>(w)) kurveIn(s, g) = *k;
    if (const Wert* w = werte.finde(kBezug))
        if (const auto* d = std::get_if<double>(w)) s.ausgangBezug = *d;
    return s;
}

ZungenAnschlag anschlagAus(const ZungenSpiel& spiel, const Tonauftrag& auftrag)
{
    ZungenAnschlag a;
    a.rate = static_cast<int>(auftrag.rate);
    a.expression = 1.0;  // SK4 a: ohne Spur „wie die Note gespielt ist“
    if (auftrag.noten.empty()) return a;
    const KettenNote& n = auftrag.noten.front();
    a.note = n.note;
    a.haltenS = static_cast<double>(n.ende - n.beginn) / auftrag.rate;
    // ZV10 a: die Staerke der Stufe an der gespielten Taste, mal der Expression (SK4 a); bei Stufe 0 genau 1
    const double staerke = staerkeBei(spiel, n.note, auftrag.stufe);
    a.expression = staerke;
    // der Wert am Beginn der Note; laeuft die Spur (mehr als ein Punkt), liest der Kern sie ueber die Zeit (34j-7, ZV3 a)
    const auto lauf = [](const Spur* s) { return s != nullptr && s->punkte.size() > 1; };
    if (const Spur* e = findeSpur(auftrag, spur::kExpression))
    {
        a.expression = staerke * std::clamp(spurWert(*e, n.beginn, 1.0), 0.0, 1.0);
        if (lauf(e))
        {
            a.expressionVerlauf = *e;
            for (SpurPunkt& p : a.expressionVerlauf.punkte) p.wert = staerke * std::clamp(p.wert, 0.0, 1.0);
        }
    }
    if (const Spur* p = findeSpur(auftrag, spur::kPitch))
    {
        a.pitchCent = spurWert(*p, n.beginn, 0.0);
        if (lauf(p)) a.pitchVerlauf = *p;
    }
    if (const Spur* v = findeSpur(auftrag, spur::kVibrato))
    {
        a.vibratoCent = spurWert(*v, n.beginn, 0.0);
        if (lauf(v)) a.vibratoVerlauf = *v;
    }
    if (const Spur* b = findeSpur(auftrag, kSpurBalg); b != nullptr && !b->punkte.empty()) a.balgVerlauf = *b;
    return a;
}

ZungeKern::ZungeKern(ZungenSpiel spiel) : m_spiel(std::move(spiel)) {}

Frames ZungeKern::nachklang(const Tonauftrag& auftrag, Frames hoechstens) const
{
    if (auftrag.noten.empty()) return std::min<Frames>(hoechstens, 1);
    // ZV11 a: der gestimmte Ausklang der gespielten Taste (bei `windMin` gemessen, dort klingt sie am laengsten)
    const auto f = static_cast<Frames>(std::ceil(nachklangBei(m_spiel, auftrag.noten.front().note) * auftrag.rate));
    return std::clamp<Frames>(f, 1, std::max<Frames>(1, hoechstens));
}

void ZungeKern::rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere) const
{
    if (auftrag.noten.empty())
    {
        ton.assign(static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames)), 0.0f);
        return;
    }
    const bool messen = innere != nullptr && std::find(innere->gewuenscht.begin(), innere->gewuenscht.end(), kInnereSpitze) != innere->gewuenscht.end();
    ZungenMessung m;
    rechneZungenTon(m_spiel, anschlagAus(m_spiel, auftrag), ton, auftrag.frames, messen ? &m : nullptr);
    if (messen) innere->ergebnis.push_back(InnereGroesse{std::string(kInnereSpitze), "1/L", {m.amplitudeRel}});
}

}  // namespace lumi::kern::zunge
