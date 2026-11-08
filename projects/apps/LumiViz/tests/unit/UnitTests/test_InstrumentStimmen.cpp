/**
 ****************************************************************************************
 * @file   test_InstrumentStimmen.cpp
 * @brief  Stufe 34f-3/34f-4: gestimmte Werte und der Bezug als Werkzeug, und ihr Waechter ueber die mitgelieferten
 *         Instrumente (docs/audio/Instrumente_IC_Konzept.md IK2-10 a, IK2-11 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton und ohne Engine. Ein Pruefkern (`testTuned`: ein Sinus, dessen Frequenz eine gestimmte Kurve traegt, mit
 * einem Bezug `outputReference`) zeigt Stimmen, Abdruck, Bezug und wann der Waechter rot wird. Der Waechter selbst
 * liest jede `asset/loop/instruments/*.lvinst` vom Arbeitsordner aus (die Test-Exe aus dem Projekt-Root starten): kein
 * gestimmter Wert veraltet (ueber den Abdruck, ohne zu rechnen), der Bezug gleich dem neu gerechneten. Heute traegt kein
 * mitgeliefertes Instrument gestimmte Werte oder einen Bezug (die Klassen kommen ab 34g) — dann meldet er das.
 * Rot: `InstrumentTuneTool <datei.lvinst> --bezug`, danach `python asset/loop/instrument_katalog.py`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Modal.hpp"
#include "InstrumentCore/Tonkern.hpp"
#include "InstrumentMeasure/Stimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "loop/KernDatei.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <memory>
#include <numbers>
#include <string>
#include <vector>

using namespace lumi::kern;
using lumi::messung::GestimmteWerte;
using lumi::messung::Stimmer;

namespace
{

/// Roh hat der Ton die Amplitude 3 — der Bezug wird 6.
constexpr double kRoh = 3.0;

class Gestimmt final : public Tonkern
{
public:
    explicit Gestimmt(const Werte& w) : m_t60(w.zahl("t60")), m_bezug(w.zahl("outputReference")), m_tonhoehe(w.kurve("pitch")) {}

    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override
    {
        return std::min<Frames>(hoechstens, std::llround(m_t60 * auftrag.rate));
    }

    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere*) const override
    {
        ton.assign(static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames)), 0.0f);
        for (const KettenNote& n : auftrag.noten)
        {
            const double hz = wertAn(m_tonhoehe, n.note, Skala::Logarithmisch);
            for (Frames f = n.beginn; f < auftrag.frames; ++f)
            {
                const double t = static_cast<double>(f - n.beginn) / auftrag.rate;
                ton[static_cast<std::size_t>(f)] +=
                    static_cast<float>(kRoh * std::exp(-kLn1000 * t / m_t60) * std::sin(2.0 * std::numbers::pi * hz * t + 0.5) / m_bezug);
            }
        }
    }

private:
    double m_t60, m_bezug;
    Kurve m_tonhoehe;
};

ParameterAngabe zahl(std::string name, double von, double bis, double vorgabe, bool klangbestimmend = true)
{
    ParameterAngabe a;
    a.name = std::move(name);
    a.titel = a.name;
    a.gruppe = "Main";
    a.art = WertArt::Zahl;
    a.darstellung = Darstellung::Regler;
    a.von = von;
    a.bis = bis;
    a.vorgabe = vorgabe;
    a.belegkraft = Belegkraft::Quelle;
    a.klangbestimmend = klangbestimmend;
    a.quellen = {"lumiviz"};
    return a;
}

KernTyp pruefTyp()
{
    KernTyp t;
    t.tafel.kern = "testTuned";
    t.tafel.titel = "Test tuned";
    t.tafel.noteMin = 48;
    t.tafel.noteMax = 72;
    t.tafel.gruppen = {"Main"};
    t.tafel.werte.push_back(zahl("t60", 0.1, 10.0, 2.0));
    t.tafel.werte.push_back(zahl("outputReference", 1e-6, 1e6, 1.0, false));
    ParameterAngabe p;
    p.name = "pitch";
    p.titel = "Pitch";
    p.gruppe = "Main";
    p.art = WertArt::Kurve;
    p.darstellung = Darstellung::Kurve;
    p.einheit = "Hz";
    p.von = 1.0;
    p.bis = 20000.0;
    p.vorgabe = Kurve{};
    p.gestimmt = true;
    t.tafel.werte.push_back(p);
    t.tafel.bezugWert = "outputReference";
    t.auskunft.rechenweg = Rechenweg::Note;
    t.erzeuge = [](const Werte& w) { return std::make_shared<const Gestimmt>(w); };
    return t;
}

/// Der Stimmer des Pruefkerns: die Tonhoehe je Note gleichstufig, um t60 Cent hoeher (damit sie an t60 haengt).
Stimmer pruefStimmer(int* aufrufe = nullptr)
{
    return Stimmer{"testTuned", [aufrufe](const Fabrik&, const Werte& w, const std::vector<std::string>&) {
                       if (aufrufe != nullptr) ++*aufrufe;
                       Kurve k;
                       k.belegkraft = Belegkraft::Rechnung;
                       k.herkunft = "tuned by the test tuner";
                       k.quellen = {"lumiviz"};
                       for (int n = 48; n <= 72; n += 12) k.punkte.push_back({n, 440.0 * std::exp2((n - 69 + w.zahl("t60") / 100.0) / 12.0)});
                       return GestimmteWerte{{"pitch", k}};
                   }};
}

Werte anfang()
{
    Werte w;
    w.setze("t60", 2.0);
    w.setze("outputReference", 1.0);
    w.setze("pitch", Kurve{});
    return w;
}

}  // namespace

TEST_CASE("Instrument Stimmen 34f: die Tafel mit Bezug ist gueltig, die Probe liegt in der Mitte bei Stufe 0")
{
    const KernTyp t = pruefTyp();
    CHECK(pruefeTafel(t.tafel).empty());
    const Tonauftrag p = bezugsProbe(t.tafel);
    REQUIRE(p.noten.size() == 1);
    CHECK(p.noten.front().note == 60);
    CHECK(p.noten.front().hz == doctest::Approx(261.6255653).epsilon(1e-9));
    CHECK(p.stufe == 0);
    CHECK(p.frames == 48000);
    // ein Bezug, der keine Zahl ist, macht die Tafel ungueltig
    KernTyp falsch = pruefTyp();
    falsch.tafel.bezugWert = "pitch";
    CHECK_FALSE(pruefeTafel(falsch.tafel).empty());
    // der Bezug zaehlt nicht zum Abdruck des gestimmten Werts
    Werte a = anfang(), b = anfang();
    b.setze("outputReference", 7.0);
    CHECK(eingabenAbdruck(t.tafel, a, "pitch") == eingabenAbdruck(t.tafel, b, "pitch"));
    b.setze("t60", 3.0);
    CHECK(eingabenAbdruck(t.tafel, a, "pitch") != eingabenAbdruck(t.tafel, b, "pitch"));
}

TEST_CASE("Instrument Stimmen 34f: stimmen, Abdruck merken, Bezug setzen — danach nichts veraltet")
{
    Fabrik fabrik;
    REQUIRE(fabrik.registriere(pruefTyp()).empty());
    int aufrufe = 0;
    const std::vector<Stimmer> liste{pruefStimmer(&aufrufe)};
    Werte w = anfang();
    CHECK(lumi::messung::veralteteWerte(fabrik, "testTuned", w) == std::vector<std::string>{"pitch"});

    const lumi::messung::StimmBericht b = lumi::messung::stimmeUndNormiere(fabrik, liste, "testTuned", w, false, true);
    CHECK(b.fehler.empty());
    CHECK(b.gestimmt == std::vector<std::string>{"pitch"});
    CHECK(aufrufe == 1);
    REQUIRE(b.mitBezug);
    CHECK(b.bezugVorher == 1.0);
    CHECK(b.bezug == doctest::Approx(kRoh / kNormSpitze).epsilon(0.01));
    CHECK(b.geaendert());
    CHECK(lumi::messung::veralteteWerte(fabrik, "testTuned", w).empty());
    CHECK_FALSE(lumi::messung::bezugVeraltet(fabrik, "testTuned", w));

    // ein zweiter Lauf tut nichts (der Bezug ist derselbe Bit fuer Bit)
    const lumi::messung::StimmBericht zweit = lumi::messung::stimmeUndNormiere(fabrik, liste, "testTuned", w, false, true);
    CHECK(zweit.gestimmt.empty());
    CHECK(aufrufe == 1);
    CHECK_FALSE(zweit.geaendert());
    // mit `alle` stimmt er trotzdem
    CHECK(lumi::messung::stimmeUndNormiere(fabrik, liste, "testTuned", w, true, false).gestimmt == std::vector<std::string>{"pitch"});
    CHECK(aufrufe == 2);
}

TEST_CASE("Instrument Stimmen 34f: der Waechter wird rot bei veralteter Stimmung und veraltetem Bezug")
{
    Fabrik fabrik;
    REQUIRE(fabrik.registriere(pruefTyp()).empty());
    const std::vector<Stimmer> liste{pruefStimmer()};
    Werte w = anfang();
    (void)lumi::messung::stimmeUndNormiere(fabrik, liste, "testTuned", w, false, true);
    REQUIRE(lumi::messung::veralteteWerte(fabrik, "testTuned", w).empty());

    Werte stimmung = w;
    stimmung.setze("t60", 3.0);  // eine Eingabe der Stimmung geaendert
    CHECK(lumi::messung::veralteteWerte(fabrik, "testTuned", stimmung) == std::vector<std::string>{"pitch"});

    Werte bezug = w;
    bezug.setze("outputReference", w.zahl("outputReference") * 1.01);
    double neu = 0.0;
    CHECK(lumi::messung::bezugVeraltet(fabrik, "testTuned", bezug, 1e-12, &neu));
    CHECK(neu == doctest::Approx(w.zahl("outputReference")).epsilon(1e-12));

    // ohne Stimmer fuer einen Kern mit gestimmten Werten: ein Fehler, nichts gesetzt
    Werte ohne = anfang();
    const lumi::messung::StimmBericht b = lumi::messung::stimmeUndNormiere(fabrik, {}, "testTuned", ohne, false, true);
    CHECK_FALSE(b.fehler.empty());
    CHECK(b.gestimmt.empty());
}

TEST_CASE("Instrument Stimmen 34g: gestimmte Werte in Reihenfolge — der spaetere wird mit dem frueheren neu gestimmt (KS2 a)")
{
    // der Pruefkern mit einem zweiten gestimmten Wert nach `pitch`, der an ihm haengt
    KernTyp t = pruefTyp();
    ParameterAngabe fenster = t.tafel.werte[2];
    REQUIRE(fenster.name == "pitch");
    fenster.name = "pitchWindow";
    fenster.abhaengigVon = {"pitch"};
    t.tafel.werte.push_back(fenster);
    REQUIRE(pruefeTafel(t.tafel).empty());
    Fabrik fabrik;
    REQUIRE(fabrik.registriere(t).empty());
    int aufrufe = 0;
    const Stimmer s{"testTuned", [&aufrufe](const Fabrik&, const Werte& w, const std::vector<std::string>& namen) {
                        ++aufrufe;
                        // der Stimmer rechnet die Kette selbst: erst `pitch` (oder der, der in den Werten steht), daraus
                        // das Fenster
                        const bool pitchNeu = std::find(namen.begin(), namen.end(), "pitch") != namen.end();
                        Kurve p;
                        p.belegkraft = Belegkraft::Rechnung;
                        if (pitchNeu)
                            for (int n = 48; n <= 72; n += 12) p.punkte.push_back({n, 440.0 * std::exp2((n - 69 + w.zahl("t60") / 100.0) / 12.0)});
                        else
                            p = w.kurve("pitch");
                        Kurve f;
                        f.belegkraft = Belegkraft::Rechnung;
                        for (const NotenWert& q : p.punkte) f.punkte.push_back({q.note, 1.01 * q.wert});
                        GestimmteWerte g;
                        if (pitchNeu) g.emplace_back("pitch", p);
                        g.emplace_back("pitchWindow", f);
                        return g;
                    }};
    Werte w = anfang();
    w.setze("pitchWindow", Kurve{});
    const lumi::messung::StimmBericht erst = lumi::messung::stimmeUndNormiere(fabrik, {s}, "testTuned", w, false, false);
    CHECK(erst.fehler.empty());
    CHECK(erst.gestimmt == std::vector<std::string>{"pitch", "pitchWindow"});
    CHECK(lumi::messung::veralteteWerte(fabrik, "testTuned", w).empty());

    // nur `pitch` veraltet (sein Abdruck passt nicht mehr): das Fenster wird mitgestimmt
    w.merkeAbdruck("pitch", "falsch");
    CHECK(lumi::messung::veralteteWerte(fabrik, "testTuned", w) == std::vector<std::string>{"pitch"});
    const lumi::messung::StimmBericht zweit = lumi::messung::stimmeUndNormiere(fabrik, {s}, "testTuned", w, false, false);
    CHECK(zweit.gestimmt == std::vector<std::string>{"pitch", "pitchWindow"});
    CHECK(lumi::messung::veralteteWerte(fabrik, "testTuned", w).empty());

    // aendert sich `pitch` von Hand, veraltet das Fenster (es haengt an ihm), `pitch` selbst nicht
    Kurve hand = w.kurve("pitch");
    hand.punkte.front().wert *= 1.001;
    w.setze("pitch", hand);
    CHECK(lumi::messung::veralteteWerte(fabrik, "testTuned", w) == std::vector<std::string>{"pitchWindow"});
    const lumi::messung::StimmBericht dritt = lumi::messung::stimmeUndNormiere(fabrik, {s}, "testTuned", w, false, false);
    CHECK(dritt.gestimmt == std::vector<std::string>{"pitchWindow"});
    CHECK(aufrufe == 3);
    // das Fenster folgt dem Wert von Hand
    CHECK(w.kurve("pitchWindow").punkte.front().wert == doctest::Approx(1.01 * hand.punkte.front().wert).epsilon(1e-15));
    CHECK(lumi::messung::veralteteWerte(fabrik, "testTuned", w).empty());
}

TEST_CASE("Instrument Stimmen 34f: der Waechter ueber die mitgelieferten Instrumente")
{
    const QStringList dateien = QDir(QStringLiteral("asset/loop/instruments")).entryList({QStringLiteral("*.lvinst")}, QDir::Files, QDir::Name);
    REQUIRE_MESSAGE(!dateien.isEmpty(), "asset/loop/instruments nicht gefunden — aus dem Projekt-Root starten");
    int geprueft = 0, mitStimmung = 0, mitBezug = 0;
    std::vector<std::string> veraltet;
    for (const QString& d : dateien)
    {
        QFile f(QStringLiteral("asset/loop/instruments/") + d);
        REQUIRE(f.open(QIODevice::ReadOnly));
        const QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
        for (const QJsonValue& s : o.value(QStringLiteral("layers")).toArray())
        {
            const QJsonObject schicht = s.toObject();
            if (!schicht.contains(QStringLiteral("core"))) continue;
            const std::string kern = schicht.value(QStringLiteral("core")).toString().toStdString();
            const KernTyp* typ = lumi::kern::kerne().typ(kern);
            REQUIRE_MESSAGE(typ != nullptr, d.toStdString() << ": unbekannter Kern " << kern);
            const Werte w = lumi::loop::werteAusJson(typ->tafel, schicht.value(QStringLiteral("coreParams")).toObject());
            ++geprueft;
            bool gestimmt = false;
            for (const ParameterAngabe& a : typ->tafel.werte) gestimmt = gestimmt || a.gestimmt;
            mitStimmung += gestimmt ? 1 : 0;
            mitBezug += typ->tafel.bezugWert.empty() ? 0 : 1;
            for (const std::string& n : lumi::messung::veralteteWerte(lumi::kern::kerne(), kern, w)) veraltet.push_back(d.toStdString() + " " + n);
            if (lumi::messung::bezugVeraltet(lumi::kern::kerne(), kern, w)) veraltet.push_back(d.toStdString() + " " + typ->tafel.bezugWert);
        }
    }
    MESSAGE(geprueft << " Kern-Schichten geprueft, " << mitStimmung << " mit gestimmten Werten, " << mitBezug << " mit Bezug");
    CHECK(geprueft >= 1);  // heute der Fluegel, ohne beides
    std::string text;
    for (const std::string& v : veraltet) text += "\n  " + v;
    CHECK_MESSAGE(veraltet.empty(), "veraltet — InstrumentTuneTool <datei> --bezug, dann python asset/loop/instrument_katalog.py:" << text);
}
