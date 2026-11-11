/**
 * @file HammerKern.cpp
 * @brief InstrumentModels, Kern `hammerString`: Tafel, Werte ↔ Spiel, Adapter (34d)
 */
#include "InstrumentModels/hammer/HammerKern.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

namespace lumi::kern::hammer
{

namespace
{

constexpr std::string_view kNameNoteMin = "noteMin";
constexpr std::string_view kNameNoteMax = "noteMax";
constexpr std::string_view kUngedaempft = "undampedFrom";
constexpr std::string_view kSoft = "softFactor";
constexpr std::string_view kModeratorSteife = "moderatorStiffness";
constexpr std::string_view kModeratorExponent = "moderatorExponent";

ParameterAngabe zahl(std::string_view name, std::string_view titel, std::string_view einheit, double von, double bis,
                     double vorgabe, Belegkraft belegkraft, std::string_view herkunft)
{
    ParameterAngabe a;
    a.name = std::string(name);
    a.titel = std::string(titel);
    a.gruppe = "Pedals";
    a.art = WertArt::Zahl;
    a.darstellung = Darstellung::Regler;
    a.einheit = std::string(einheit);
    a.von = von;
    a.bis = bis;
    a.vorgabe = vorgabe;
    a.belegkraft = belegkraft;
    a.herkunft = std::string(herkunft);
    a.quellen = {"lumiviz"};  // WZ1 a: die drei Pedalwerte sind offen wie in IC — eigene Vorgabe
    return a;
}

Parametertafel baueTafel()
{
    Parametertafel t;
    t.kern = std::string(kKernName);
    t.titel = "Hammer string";  // TT1 a (S181): das Verfahren, nicht ein Instrument (Upright, Honky-tonk, CP-70 auf demselben Kern)
    t.rechenFassung = 2;  // S187 (HS-3 a): der Kontakt 32-fach statt 8-fach
    t.noteMin = kTasteMin;
    t.noteMax = kTasteMax;
    t.gruppen = {"Keys", "String", "Choir", "Decay", "Hammer", "Damper", "Pedals"};
    // KL4 a (S181): der Umfang je Instrument als zwei Ganzzahlen wie beim Kern `bar` (KS1 a) — der CP-70 hat 73 Tasten
    t.umfangVon = std::string(kNameNoteMin);
    t.umfangBis = std::string(kNameNoteMax);
    for (const auto& [name, titel, vorgabe] : {std::tuple{kNameNoteMin, "Lowest key", kTasteMin}, std::tuple{kNameNoteMax, "Highest key", kTasteMax}})
    {
        ParameterAngabe a;
        a.name = std::string(name);
        a.titel = titel;
        a.gruppe = "Keys";
        a.art = WertArt::Ganzzahl;
        a.darstellung = Darstellung::Regler;
        a.einheit = "MIDI";
        a.von = kTasteMin;
        a.bis = kTasteMax;
        a.vorgabe = std::int64_t{vorgabe};
        // tafelweit (der Text nennt alle Instrumente); die Quellen je Instrument am Wert (QM1 a): der Fluegel Salamander
        a.belegkraft = Belegkraft::Offen;
        a.herkunft = "grand piano 88 keys A0-C8 (Salamander: Yamaha C5, sampled A0 to C8); upright and honky-tonk A0-C8 (open); "
                     "CP-70 73 keys E1-E7 (IC, open)";
        a.quellen = {"salamander"};
        a.klangbestimmend = false;
        t.werte.push_back(std::move(a));
    }
    const HammerSaite& v = hammerSaiteVorgabe();
    for (const KurvenAngabe& k : kurvenAngaben())
    {
        ParameterAngabe a;
        a.name = std::string(k.name);
        a.titel = std::string(k.titel);
        a.gruppe = std::string(k.gruppe);
        a.art = WertArt::Kurve;
        a.darstellung = Darstellung::Kurve;
        a.einheit = std::string(k.einheit);
        a.skala = k.skala;
        a.von = k.von;
        a.bis = k.bis;
        a.vorgabe = v.*k.feld;
        a.belegkraft = (v.*k.feld).belegkraft;
        a.herkunft = (v.*k.feld).herkunft;
        a.quellen = (v.*k.feld).quellen;
        t.werte.push_back(std::move(a));
    }
    ParameterAngabe ungedaempft;
    ungedaempft.name = std::string(kUngedaempft);
    ungedaempft.titel = "Undamped from";
    ungedaempft.gruppe = "Pedals";
    ungedaempft.art = WertArt::Ganzzahl;
    ungedaempft.darstellung = Darstellung::Regler;
    ungedaempft.einheit = "MIDI";
    ungedaempft.von = kTasteMin;
    ungedaempft.bis = kTasteMax + 1;
    ungedaempft.vorgabe = std::int64_t{kUngedaempftAbVorgabe};
    ungedaempft.belegkraft = Belegkraft::Quelle;
    ungedaempft.herkunft = "Salamander SFZ (Holm): notes without dampers from lokey=89 (D1 a)";
    ungedaempft.quellen = {"salamander"};
    t.werte.push_back(std::move(ungedaempft));
    t.werte.push_back(zahl(kSoft, "Soft factor", "", 0.1, 1.0, 1.0, Belegkraft::Offen,
                           "open: \"nearly negligible\" (forum, report only); 1 = as without (IC K1 F-4-1 a)"));
    t.werte.push_back(zahl(kModeratorSteife, "Moderator stiffness", "N/mm^p", 0.01, 1e4, 2.0, Belegkraft::Offen,
                           "open: no model found in the literature (IC draft section 2.1)"));
    t.werte.push_back(zahl(kModeratorExponent, "Moderator exponent", "", 1.0, 6.0, 2.0, Belegkraft::Offen,
                           "open: no model found in the literature (IC draft section 2.1)"));
    return t;
}

bool getreten(const Tonauftrag& auftrag, std::string_view spurName, Frames beginn)
{
    const Spur* s = findeSpur(auftrag, spurName);
    return s != nullptr && spurWert(*s, beginn, 0.0) >= 0.5;
}

}  // namespace

const KernTyp& kernTyp()
{
    static const KernTyp typ = [] {
        KernTyp t;
        t.tafel = baueTafel();
        t.auskunft.rechenweg = Rechenweg::Note;
        t.auskunft.stufeImNamen = true;
        t.auskunft.spuren = {std::string(kSpurSoft), std::string(kSpurPractice)};
        t.auskunft.innere = {"contact"};
        t.auskunft.vorbereitungTeuer = false;
        t.auskunft.koerperAnDerSchicht = true;  // FU1 a: der Resonanzboden an der Schicht, die App faltet ihn
        t.erzeuge = [](const Werte& w) -> std::shared_ptr<const Tonkern> {
            return std::make_shared<const HammerKern>(saiteAus(w), pedalwerteAus(w));
        };
        return t;
    }();
    return typ;
}

const std::vector<Vorlage>& vorlagen()
{
    // die Pedalwerte wie IC `Tastatur` (Vorgabe fuer alle drei); `undampedFrom` vom Fluegel uebernommen (Salamander)
    static const std::vector<Vorlage> v = [] {
        // der Umfang: Upright und Honky-tonk wie IC `tastaturUpright` (Vorgabe A0–C8, eigene Annahme), der CP-70 nach IC
        // `tastaturCp70` 73 Tasten E1–E7 (offen, ohne genannte Quelle)
        const QuellenJeWert vomFluegel = {{std::string(kUngedaempft), {"salamander"}}};
        return std::vector<Vorlage>{
            {"upright_piano", "Upright piano", baueVorgabeUpright(), Pedalwerte{}, {kTasteMin, kTasteMax}, true, vomFluegel},
            {"honky_tonk_piano", "Honky-tonk piano", baueVorgabeHonkytonk(), Pedalwerte{}, {kTasteMin, kTasteMax}, true, vomFluegel},
            {"electric_grand_piano", "Electric grand piano", baueVorgabeCp70(), Pedalwerte{}, {28, 100}, true, vomFluegel},
        };
    }();
    return v;
}

Werte werteVon(const Vorlage& vorlage)
{
    Werte w = werteAus(vorlage.saite, vorlage.pedale, vorlage.umfang);
    for (const ParameterAngabe& a : kernTyp().tafel.werte)
    {
        if (a.art == WertArt::Kurve) continue;
        std::vector<std::string> q{"lumiviz"};
        for (const auto& [n, s] : vorlage.quellen)
            if (n == a.name) q = s;
        w.setzeQuellen(a.name, q);
    }
    return w;
}

Werte werteAus(const HammerSaite& saite, const Pedalwerte& pedale, std::pair<int, int> umfang)
{
    Werte w;
    w.felder.emplace_back(std::string(kNameNoteMin), std::int64_t{umfang.first});
    w.felder.emplace_back(std::string(kNameNoteMax), std::int64_t{umfang.second});
    for (const KurvenAngabe& k : kurvenAngaben()) w.felder.emplace_back(std::string(k.name), saite.*k.feld);
    w.felder.emplace_back(std::string(kUngedaempft), std::int64_t{pedale.ungedaempftAb});
    w.felder.emplace_back(std::string(kSoft), pedale.softFaktor);
    w.felder.emplace_back(std::string(kModeratorSteife), pedale.moderatorSteife);
    w.felder.emplace_back(std::string(kModeratorExponent), pedale.moderatorExponent);
    return w;
}

HammerSaite saiteAus(const Werte& werte)
{
    HammerSaite s;
    for (const KurvenAngabe& k : kurvenAngaben())
        if (const Wert* w = werte.finde(k.name))
            if (const Kurve* kurve = std::get_if<Kurve>(w)) s.*k.feld = *kurve;
    return s;
}

Pedalwerte pedalwerteAus(const Werte& werte)
{
    Pedalwerte p;
    if (const Wert* w = werte.finde(kUngedaempft))
        if (const auto* g = std::get_if<std::int64_t>(w)) p.ungedaempftAb = static_cast<int>(*g);
    if (const Wert* w = werte.finde(kSoft))
        if (const auto* z = std::get_if<double>(w)) p.softFaktor = *z;
    if (const Wert* w = werte.finde(kModeratorSteife))
        if (const auto* z = std::get_if<double>(w)) p.moderatorSteife = *z;
    if (const Wert* w = werte.finde(kModeratorExponent))
        if (const auto* z = std::get_if<double>(w)) p.moderatorExponent = *z;
    return p;
}

HammerAnschlag anschlagAus(const Tonauftrag& auftrag, const Pedalwerte& pedale)
{
    HammerAnschlag a;
    a.rate = static_cast<int>(auftrag.rate);
    a.stufe = auftrag.stufe;
    if (auftrag.noten.empty()) return a;
    const KettenNote& n = auftrag.noten.front();
    a.taste = n.note;
    a.hz = n.hz;
    a.halten = n.ende - n.beginn;
    a.gedaempft = n.note < pedale.ungedaempftAb;
    if (getreten(auftrag, kSpurSoft, n.beginn)) a.tempoFaktor = pedale.softFaktor;
    if (getreten(auftrag, kSpurPractice, n.beginn))
    {
        a.moderatorSteife = pedale.moderatorSteife;
        a.moderatorExponent = pedale.moderatorExponent;
    }
    return a;
}

HammerKern::HammerKern(HammerSaite saite, Pedalwerte pedale) : m_saite(std::move(saite)), m_pedale(pedale) {}

Frames HammerKern::nachklang(const Tonauftrag& auftrag, Frames hoechstens) const
{
    return hammerNachklang(m_saite, anschlagAus(auftrag, m_pedale), hoechstens);
}

void HammerKern::rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere) const
{
    if (auftrag.noten.empty())
    {
        ton.assign(static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames)), 0.0f);
        return;
    }
    HammerMessung messung;
    const bool messen = innere != nullptr &&
                        std::find(innere->gewuenscht.begin(), innere->gewuenscht.end(), "contact") != innere->gewuenscht.end();
    rechneHammerTon(m_saite, anschlagAus(auftrag, m_pedale), ton, auftrag.frames, messen ? &messung : nullptr);
    if (messen) innere->ergebnis.push_back(InnereGroesse{"contact", "s, N", {messung.kontaktS, messung.kraftSpitzeN}});
}

}  // namespace lumi::kern::hammer
