/**
 ****************************************************************************************
 * @file   test_InstrumentModels.cpp
 * @brief  InstrumentModels, Stufe 34d: die eingebauten Kerne, der Fluegel als Kern `hammerString`
 *         (docs/audio/Instrumente_IC_Konzept.md BK1 a, IK2-3 a, IK2-4 a, FU2 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine.
 *
 * - `kerne()` kennt `hammerString`; seine Tafel besteht `pruefeTafel`, Gruppen und Reihenfolge wie §6a.
 * - Werte ↔ Spiel ohne Verlust, Vorgaben der Tafel = Vorgabe der Saite, ungemessen (IK2-9).
 * - Der Adapter rechnet Bit fuer Bit wie `rechneHammerTon` (Auftrag ↔ Anschlag), Nachklang ebenso.
 * - Soft und Practice wirken nur ueber ihre Spuren; mit Faktor 1 bzw. ohne Spur Bit fuer Bit wie ohne (FU2 a).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/hammer/HammerKern.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace lumi::kern;
using namespace lumi::kern::hammer;

namespace
{

constexpr int kRate = 48000;

Tonauftrag auftragC4(int stufe, Frames halten, Frames frames)
{
    return einzelnote(60, 261.6255653005986, stufe, halten, frames, kRate);
}

double spitze(const std::vector<float>& ton)
{
    double s = 0.0;
    for (const float x : ton) s = std::max(s, static_cast<double>(std::abs(x)));
    return s;
}

}  // namespace

TEST_CASE("InstrumentModels 34d kerne() kennt den Fluegel")
{
    // seit 34g dazu der Kern `bar` (Klasse 4, test_InstrumentModelsStab.cpp), seit 34h `membrane` (Klasse 5,
    // test_InstrumentModelsMembran.cpp)
    CHECK(kerne().namen() == std::vector<std::string>{std::string(kKernName), "bar", "membrane", "struckString", "freeReed"});  // 34j: freeReed
    const KernTyp* typ = kerne().typ(kKernName);
    REQUIRE(typ != nullptr);
    CHECK(pruefeTafel(typ->tafel).empty());
    CHECK(typ->tafel.noteMin == kTasteMin);
    CHECK(typ->tafel.noteMax == kTasteMax);
    CHECK(typ->tafel.werte.size() == kurvenAngaben().size() + 6);  // seit KL4 a dazu noteMin, noteMax
    CHECK(typ->tafel.gruppen == std::vector<std::string>{"Keys", "String", "Choir", "Decay", "Hammer", "Damper", "Pedals"});
    CHECK(typ->tafel.umfangVon == "noteMin");  // KL4 a
    CHECK(typ->tafel.umfangBis == "noteMax");
    CHECK(typ->auskunft.rechenweg == Rechenweg::Note);
    CHECK(typ->auskunft.stufeImNamen);
    CHECK(typ->auskunft.spuren == std::vector<std::string>{std::string(kSpurSoft), std::string(kSpurPractice)});
    // jede Kurve der Tafel mit Skala und Grenzen aus `kurvenAngaben()`, Vorgabe = Vorgabe der Saite
    for (const KurvenAngabe& k : kurvenAngaben())
    {
        CAPTURE(std::string(k.name));
        const ParameterAngabe* a = findeAngabe(typ->tafel, k.name);
        REQUIRE(a != nullptr);
        CHECK(a->art == WertArt::Kurve);
        CHECK(a->skala == k.skala);
        CHECK(a->von == k.von);
        CHECK(a->bis == k.bis);
        CHECK(std::get<Kurve>(a->vorgabe) == hammerSaiteVorgabe().*k.feld);
        CHECK_FALSE((hammerSaiteVorgabe().*k.feld).herkunft.empty());
    }
}

TEST_CASE("InstrumentModels 34d Werte und Spiel des Fluegels")
{
    const KernTyp& typ = kernTyp();
    // die Vorgaben der Tafel sind die der Saite und der Pedale
    Werte v = vorgaben(typ.tafel);
    CHECK(richteWerte(typ.tafel, v).empty());
    CHECK(saiteAus(v) == hammerSaiteVorgabe());
    CHECK(pedalwerteAus(v) == Pedalwerte{});
    CHECK(v == werteAus(hammerSaiteVorgabe()));
    // hin und zurueck ohne Verlust, auch mit geaenderten Werten
    HammerSaite s = hammerSaiteVorgabe();
    s.verstimmungCent = kurveKonstant(4.5, Belegkraft::Offen, "test", kTasteMin, kTasteMax);
    Pedalwerte p;
    p.ungedaempftAb = 95;
    p.softFaktor = 0.8;
    Werte w = werteAus(s, p);
    CHECK(richteWerte(typ.tafel, w).empty());
    CHECK(saiteAus(w) == s);
    CHECK(pedalwerteAus(w) == p);
    // der Fluegel ist ungemessen: Late decay, Damper und mehr sind offen (IK2-9, Uebernahme-Journal J13)
    CHECK(ungemessen(typ.tafel, v));
}

TEST_CASE("InstrumentModels 34d der Adapter rechnet wie rechneHammerTon")
{
    const std::shared_ptr<const Tonkern> kern = kerne().erzeuge(kKernName, werteAus(hammerSaiteVorgabe()));
    REQUIRE(kern);
    for (const int stufe : {0, -12})
    {
        CAPTURE(stufe);
        const Tonauftrag auftrag = auftragC4(stufe, kRate / 2, kRate);
        std::vector<float> ueberKern;
        kern->rechne(auftrag, ueberKern);
        HammerAnschlag a;
        a.taste = 60;
        a.hz = 261.6255653005986;
        a.stufe = stufe;
        a.halten = kRate / 2;
        a.rate = kRate;
        a.gedaempft = true;
        std::vector<float> direkt;
        rechneHammerTon(hammerSaiteVorgabe(), a, direkt, kRate);
        CHECK(ueberKern == direkt);
        CHECK(kern->nachklang(auftrag, 1'000'000) == hammerNachklang(hammerSaiteVorgabe(), a, 1'000'000));
    }
    // oberhalb von `undampedFrom` ohne Daempfer
    Tonauftrag hoch = einzelnote(96, 2093.0, 0, kRate / 4, kRate / 2, kRate);
    HammerAnschlag a = anschlagAus(hoch, Pedalwerte{});
    CHECK_FALSE(a.gedaempft);
    Pedalwerte alleGedaempft;
    alleGedaempft.ungedaempftAb = kTasteMax + 1;
    CHECK(anschlagAus(hoch, alleGedaempft).gedaempft);
    // innere Groesse `contact`: Kontaktzeit und Kraftspitze
    std::vector<float> ton;
    Innere innere;
    innere.gewuenscht = {"contact"};
    kern->rechne(auftragC4(0, kRate / 2, kRate / 10), ton, &innere);
    REQUIRE(innere.ergebnis.size() == 1);
    CHECK(innere.ergebnis[0].werte.size() == 2);
    CHECK(innere.ergebnis[0].werte[0] > 0.0);
    CHECK(innere.ergebnis[0].werte[1] > 0.0);
    // ohne Note still
    Tonauftrag leer = auftragC4(0, 10, 100);
    leer.noten.clear();
    kern->rechne(leer, ton);
    CHECK(ton == std::vector<float>(100, 0.0f));
}

TEST_CASE("InstrumentModels 34d Soft und Practice ueber Spuren")
{
    Pedalwerte p;
    p.softFaktor = 0.5;
    p.moderatorSteife = 0.5;
    const std::shared_ptr<const Tonkern> kern = kerne().erzeuge(kKernName, werteAus(hammerSaiteVorgabe(), p));
    REQUIRE(kern);
    const Tonauftrag ohne = auftragC4(0, kRate / 4, kRate / 2);
    std::vector<float> normal;
    kern->rechne(ohne, normal);

    // Spur da, aber nicht getreten (0): Bit fuer Bit wie ohne
    Tonauftrag nicht = ohne;
    nicht.spuren.push_back(Spur{std::string(kSpurSoft), {{0, 0.0}}});
    nicht.spuren.push_back(Spur{std::string(kSpurPractice), {{0, 0.0}}});
    std::vector<float> ton;
    kern->rechne(nicht, ton);
    CHECK(ton == normal);

    // Soft getreten: der Hammer trifft halb so schnell — leiser
    Tonauftrag soft = ohne;
    soft.spuren.push_back(Spur{std::string(kSpurSoft), {{0, 1.0}}});
    std::vector<float> leise;
    kern->rechne(soft, leise);
    CHECK(spitze(leise) < 0.8 * spitze(normal));

    // Practice getreten: der Moderator daempft den Anschlag
    Tonauftrag practice = ohne;
    practice.spuren.push_back(Spur{std::string(kSpurPractice), {{0, 1.0}}});
    std::vector<float> filz;
    kern->rechne(practice, filz);
    CHECK(filz != normal);
    CHECK(spitze(filz) < spitze(normal));

    // Soft mit Faktor 1 (Vorgabe): Bit fuer Bit wie ohne (IC: tempo · 1)
    const std::shared_ptr<const Tonkern> neutral = kerne().erzeuge(kKernName, werteAus(hammerSaiteVorgabe()));
    std::vector<float> a, b;
    neutral->rechne(ohne, a);
    neutral->rechne(soft, b);
    CHECK(a == b);
}
