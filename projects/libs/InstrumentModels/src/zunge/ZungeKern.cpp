/**
 * @file ZungeKern.cpp
 * @brief InstrumentModels, Kern `freeReed`: Tafel, Werte ↔ Spiel, Adapter, Vorlagen (34j-2; Herleitung im Kopf von
 *        `ZungeKern.hpp`)
 */
#include "InstrumentModels/zunge/ZungeKern.hpp"

#include <algorithm>
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
constexpr std::string_view kDruckPp = "windPp";
constexpr std::string_view kDruckFf = "windFf";

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

/// Die Felder des Spiels ausser den Listen der Choere und den gestimmten Werten, in der Reihenfolge der Tafel.
const std::vector<Feld<ZungenSpiel>>& felder()
{
    static const std::vector<Feld<ZungenSpiel>> f = {
        ganzzahl<ZungenSpiel>(kNameNoteMin, "Lowest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &ZungenSpiel::noteMin),
        ganzzahl<ZungenSpiel>(kNameNoteMax, "Highest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &ZungenSpiel::noteMax),
        ganzzahl<ZungenSpiel>(kNameBassUnter, "Bass side below key", "Keys", "MIDI", 0, kUmfangMax + 1, &ZungenSpiel::bassAb),
        zahl<ZungenSpiel>("reedLengthC4", "Reed length at C4", "Reed", "m", 1e-3, 0.2, &ZungenSpiel::laengeC4M),
        zahl<ZungenSpiel>("reedWidth", "Reed width", "Reed", "m", 1e-4, 0.05, &ZungenSpiel::breiteM),
        zahl<ZungenSpiel>("reedMassC4", "Reed mass at C4", "Reed", "kg", 1e-7, 1e-2, &ZungenSpiel::masseC4Kg),
        zahl<ZungenSpiel>("reedDamping", "Reed damping ratio", "Reed", "", 1e-5, 0.5, &ZungenSpiel::zeta),
        zahl<ZungenSpiel>("reedRest", "Rest opening", "Reed", "m", 0.0, 0.01, &ZungenSpiel::ruheM),
        zahl<ZungenSpiel>("sideGap", "Side gap", "Reed", "m", 0.0, 0.01, &ZungenSpiel::seitenspaltM),
        zahl<ZungenSpiel>("chamberVolume", "Chamber volume", "Chamber", "m³", 1e-8, 0.1, &ZungenSpiel::kanzelleM3),
        zahl<ZungenSpiel>("channelLength", "Channel length", "Chamber", "m", 1e-4, 1.0, &ZungenSpiel::kanalLaengeM),
        zahl<ZungenSpiel>("channelArea", "Channel area", "Chamber", "m²", 1e-8, 0.01, &ZungenSpiel::kanalFlaecheM2),
        zahl<ZungenSpiel>("valveArea", "Pallet opening", "Chamber", "m²", 1e-7, 0.01, &ZungenSpiel::ventilFlaecheM2),
        zahl<ZungenSpiel>(kDruckPp, "Wind pp", "Wind", "Pa", 1.0, 20000.0, &ZungenSpiel::druckPpPa),
        zahl<ZungenSpiel>(kDruckFf, "Wind ff", "Wind", "Pa", 1.0, 20000.0, &ZungenSpiel::druckFfPa),
        zahl<ZungenSpiel>("bendLimit", "Bend limit", "Wind", "cent", 0.0, 1200.0, &ZungenSpiel::bendGrenzeCent),
        zahl<ZungenSpiel>("caseLowpass", "Case low-pass", "Case", "Hz", 0.0, 20000.0, &ZungenSpiel::gehaeuseTiefpassHz),
    };
    return f;
}

/// Eine gestimmte Kurve des Spiels in der Tafel (IK2-11 a), in der Reihenfolge des Stimmers.
struct GestimmtFeld
{
    std::string_view name, titel, einheit, herkunft;
    double von, bis;
    Kurve ZungenSpiel::* feld;
};

const std::vector<GestimmtFeld>& gestimmtFelder()
{
    static const std::vector<GestimmtFeld> f = {
        {kEigenfrequenz, "Reed frequency per key", "Hz",
         "tuned through the model: f0 of the reed so that it plays the key in the middle of its window (it sounds \"somewhat "
         "below\" f0, Cottingham)",
         1.0, 20000.0, &ZungenSpiel::eigenfrequenzJeNote},
        {kDruckUnten, "Lowest speaking wind per key", "Pa",
         "tuned through the model: the lower edge of the pressure at which the reed speaks (amplitude >= 2 % of its length, "
         "within 100 cent of f0, without the edge)",
         1.0, 1e5, &ZungenSpiel::druckUntenJeNote},
        {kDruckOben, "Highest speaking wind per key", "Pa", "tuned through the model: the upper edge of this window", 1.0, 1e5,
         &ZungenSpiel::druckObenJeNote},
    };
    return f;
}

/// Belegkraft und Herkunft der Vorgaben (tafelweit, der Text nennt alle vier Instrumente; IC `Vorgaben.cpp`, `Zunge.hpp`,
/// `modell.md`). Die Quellen der Klasse stehen noch nicht im Register (34j-4) — die Schluessel bleiben `lumiviz`.
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
    else if (n == "reedLengthC4" || n == "reedWidth" || n == "reedMassC4")
        setze(Belegkraft::Quelle, "the C4 reed after Adachi et al. 2026 Tab. 1 (read in IC): 32.8 × 3.45 mm, (2/3) × 0.248 g; over the "
                                  "range length ∝ 1/√f and mass ∝ length (computed); harmonium brass ×8490/7850, harmonica 20 mm (open)");
    else if (n == "reedDamping")
        setze(Belegkraft::Offen, "0.003 (open); harmonica Q = 95 after Millot & Baumann (report only)");
    else if (n == "reedRest" || n == "sideGap")
        setze(Belegkraft::Offen, "rest opening 0.3 mm and side gap 0.14 mm (Puranik & Scavone 2023 p. 1812 — report only)");
    else if (n == "chamberVolume")
        setze(Belegkraft::Offen, "accordion 14.8 × 14.6 mm² (report only) × 20 mm depth (open); harmonium 0.0013 m³ (Puranik & Scavone "
                                 "2023 Tab. 1 — report only)");
    else if (n == "channelLength" || n == "channelArea")
        setze(Belegkraft::Offen, "30 mm and 3.45 × 2 mm² (open: the inertia of the flow drives the oscillation — with 5 mm it builds up in "
                                 "400 ms, with 30 mm in about 100 ms); harmonium 50 mm, 6.4e-5 m² (Puranik & Scavone 2023 Tab. 1 — report only)");
    else if (n == "valveArea")
        setze(Belegkraft::Offen, "the open pallet 3 cm² (open); the bend closes it");
    else if (n == kDruckPp || n == kDruckFf)
        setze(Belegkraft::Offen, "accordion 300–2000 Pa (lab axis 0–2.5 kPa — report only), harmonium 200–1500 Pa (\"0.2-1.5 kPa in a real "
                                 "harmonium\", Puranik & Scavone 2023 p. 1810), melodica and harmonica 300–3000 Pa (open); without effect "
                                 "while the window per key is tuned (IC) — replaced by one wind range per instrument in 34j-6 (ZV2 a)");
    else if (n == "bendLimit")
        setze(Belegkraft::Offen, "one semitone (IC F-2 a: equipment); the pallet opening per cent is open");
    else if (n == kChoere)
        setze(Belegkraft::Quelle, "accordion musette M + M+ beating 2.00 Hz at A4 = 7.9 ct (Hergert p. 4460, read in IC), rising ∝ "
                                  "√(f/440) over the range (open); levels open; the others one 8′ rank");
    else if (n == kBassChoere)
        setze(Belegkraft::Offen, "accordion bass register 16′ + 8′ (open)");
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
    t.rechenFassung = 1;
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
                a.klangbestimmend = a.name != kNameNoteMin && a.name != kNameNoteMax && a.name != kDruckPp && a.name != kDruckFf;
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
                a.skala = Skala::Logarithmisch;
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

}  // namespace

const std::vector<Vorlage>& vorlagen()
{
    static const std::vector<Vorlage> v = {
        {"accordion", "Accordion", baueVorgabeAkkordeon(), {}},
        {"harmonium", "Harmonium", baueVorgabeHarmonium(), {}},
        {"melodica", "Melodica", baueVorgabeMelodica(), {}},
        {"harmonica", "Harmonica", baueVorgabeMundharmonika(), {}},
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
        t.auskunft.spuren = {std::string(spur::kExpression), std::string(spur::kPitch), std::string(spur::kVibrato)};
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
        if (gruppe == "Ranks") w.felder.emplace_back(std::string(kChoere), chorListeAus(spiel.choere));
        if (gruppe == "Bass ranks") w.felder.emplace_back(std::string(kBassChoere), chorListeAus(spiel.bassChoere));
        if (gruppe == "Tunable")
            for (const GestimmtFeld& g : gestimmtFelder()) w.felder.emplace_back(std::string(g.name), spiel.*g.feld);
    }
    w.felder.emplace_back(std::string(kBezug), spiel.ausgangBezug);
    return w;
}

ZungenSpiel spielAus(const Werte& werte)
{
    ZungenSpiel s;
    for (const Feld<ZungenSpiel>& f : felder())
        if (const Wert* w = werte.finde(f.angabe.name)) f.setze(s, *w);
    if (const Wert* w = werte.finde(kChoere)) s.choere = choereAus(*w);
    if (const Wert* w = werte.finde(kBassChoere)) s.bassChoere = choereAus(*w);
    for (const GestimmtFeld& g : gestimmtFelder())
        if (const Wert* w = werte.finde(g.name))
            if (const Kurve* k = std::get_if<Kurve>(w)) s.*g.feld = *k;
    if (const Wert* w = werte.finde(kBezug))
        if (const auto* d = std::get_if<double>(w)) s.ausgangBezug = *d;
    return s;
}

ZungenAnschlag anschlagAus(const Tonauftrag& auftrag)
{
    ZungenAnschlag a;
    a.rate = static_cast<int>(auftrag.rate);
    a.expression = 1.0;  // SK4 a: ohne Spur „wie die Note gespielt ist“
    if (auftrag.noten.empty()) return a;
    const KettenNote& n = auftrag.noten.front();
    a.note = n.note;
    a.haltenS = static_cast<double>(n.ende - n.beginn) / auftrag.rate;
    if (const Spur* e = findeSpur(auftrag, spur::kExpression)) a.expression = std::clamp(spurWert(*e, n.beginn, 1.0), 0.0, 1.0);
    if (const Spur* p = findeSpur(auftrag, spur::kPitch)) a.pitchCent = spurWert(*p, n.beginn, 0.0);
    if (const Spur* v = findeSpur(auftrag, spur::kVibrato)) a.vibratoCent = spurWert(*v, n.beginn, 0.0);
    return a;
}

ZungeKern::ZungeKern(ZungenSpiel spiel) : m_spiel(std::move(spiel)) {}

Frames ZungeKern::nachklang(const Tonauftrag& auftrag, Frames hoechstens) const
{
    if (auftrag.noten.empty()) return std::min<Frames>(hoechstens, 1);
    return std::clamp<Frames>(static_cast<Frames>(auftrag.rate), 1, std::max<Frames>(1, hoechstens));
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
    rechneZungenTon(m_spiel, anschlagAus(auftrag), ton, auftrag.frames, messen ? &m : nullptr);
    if (messen) innere->ergebnis.push_back(InnereGroesse{std::string(kInnereSpitze), "1/L", {m.amplitudeRel}});
}

}  // namespace lumi::kern::zunge
