/**
 ****************************************************************************************
 * @file   main.cpp
 * @brief  InstrumentBodyTool — rechnet einen Instrumentkoerper aus seinem Bauplan:
 *         Netz, Platte, Moden (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik
 *         S147/S148: BM5 a, BM5-1 a, EG1 a; Stufe 31c-4)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Konsole, ohne Fenster. Bis 31c-4 rechnet es die Moden und meldet sie (Zahl, Zeit, Speicher;
 * auf Wunsch die Frequenzen in eine Textdatei); ab 31c-5 schreibt es den Koerper. Netzgeber und
 * Eigenloeser sind per Schalter waehlbar (`--list` zeigt sie) — jede Umsetzung aus
 * `InstrumentBody/Umsetzungen.hpp`. Der Leser des Bauplans kommt aus `loop/` der App, direkt
 * mitkompiliert (Executables vor Apps).
 ****************************************************************************************
 */

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/Platte.hpp"
#include "InstrumentBody/Statistik.hpp"
#include "InstrumentBody/Umsetzungen.hpp"
#include "loop/KoerperBauplan.hpp"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <chrono>
#include <complex>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define PSAPI_VERSION 2
#include <windows.h>
#include <psapi.h>
#endif

namespace kp = lumi::koerper;

namespace
{

double spitzeMB()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) return static_cast<double>(pmc.PeakWorkingSetSize) / 1048576.0;
#endif
    return -1.0;
}

double seit(std::chrono::steady_clock::time_point t)
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t).count();
}

template <typename Liste>
auto waehle(const Liste& liste, const QString& name) -> decltype(liste.front().get())
{
    if (name.isEmpty()) return liste.front().get();
    for (const auto& e : liste)
        if (QString::fromStdString(e->name()).startsWith(name, Qt::CaseInsensitive)) return e.get();
    return nullptr;
}

}  // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("InstrumentBodyTool"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Computes an instrument body from its plan: mesh, plate, modes."));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("plan"), QStringLiteral("The body plan (.lvinstbodyplan)."));
    const QCommandLineOption bisOpt(QStringLiteral("to"), QStringLiteral("Modes up to this frequency in Hz (default 5000)."),
                                    QStringLiteral("Hz"), QStringLiteral("5000"));
    const QCommandLineOption vonOpt(QStringLiteral("from"), QStringLiteral("Modes from this frequency in Hz (default 0)."),
                                    QStringLiteral("Hz"), QStringLiteral("0"));
    const QCommandLineOption netzOpt(QStringLiteral("mesh-to"),
                                     QStringLiteral("Element size for this frequency in Hz (MAESSTRO eq. 1; default: --to)."),
                                     QStringLiteral("Hz"));
    const QCommandLineOption ordnungOpt(QStringLiteral("order"), QStringLiteral("Order k of the elements (default 3)."),
                                        QStringLiteral("k"), QStringLiteral("3"));
    const QCommandLineOption geberOpt(QStringLiteral("mesher"), QStringLiteral("Mesher by name (default: the first, see --list)."),
                                      QStringLiteral("name"));
    const QCommandLineOption loeserOpt(QStringLiteral("solver"),
                                       QStringLiteral("Eigensolver by name (default: the first, see --list)."), QStringLiteral("name"));
    const QCommandLineOption freqOpt(QStringLiteral("frequencies"), QStringLiteral("Write the frequencies, one per line, to this file."),
                                     QStringLiteral("file"));
    const QCommandLineOption listeOpt(QStringLiteral("list"), QStringLiteral("List the meshers and eigensolvers."));
    const QCommandLineOption statOpt(QStringLiteral("statistics-to"),
                                     QStringLiteral("Statistical part above the computed modes up to this frequency in Hz (default 20000; 0 = none)."),
                                     QStringLiteral("Hz"), QStringLiteral("20000"));
    const QCommandLineOption samenOpt(QStringLiteral("seed"), QStringLiteral("Seed of the statistical part (default 1)."),
                                      QStringLiteral("n"), QStringLiteral("1"));
    parser.addOptions({bisOpt, vonOpt, netzOpt, ordnungOpt, geberOpt, loeserOpt, freqOpt, listeOpt, statOpt, samenOpt});
    parser.process(app);

    const auto geberListe = kp::netzgeber();
    const auto loeserListe = kp::eigenloeser();
    if (parser.isSet(listeOpt))
    {
        std::printf("Meshers:\n");
        for (const auto& g : geberListe) std::printf("  %s\n", g->name().c_str());
        std::printf("Eigensolvers:\n");
        for (const auto& l : loeserListe) std::printf("  %s\n", l->name().c_str());
        return 0;
    }
    if (parser.positionalArguments().size() != 1)
    {
        std::fprintf(stderr, "Give one body plan (.lvinstbodyplan); --help lists the options.\n");
        return 2;
    }
    const kp::Netzgeber* geber = waehle(geberListe, parser.value(geberOpt));
    const kp::Eigenloeser* loeser = waehle(loeserListe, parser.value(loeserOpt));
    if (!geber || !loeser)
    {
        std::fprintf(stderr, "Unknown mesher or eigensolver; --list shows them.\n");
        return 2;
    }
    const double bisHz = parser.value(bisOpt).toDouble();
    const double vonHz = parser.value(vonOpt).toDouble();
    const double netzHz = parser.isSet(netzOpt) ? parser.value(netzOpt).toDouble() : bisHz;
    const int ordnung = parser.value(ordnungOpt).toInt();

    const auto beginn = std::chrono::steady_clock::now();
    kp::Bauplan plan;
    bool zuNeu = false;
    if (!lumi::loop::ladeBauplan(parser.positionalArguments().front(), plan, &zuNeu))
    {
        std::fprintf(stderr, zuNeu ? "The body plan is from a newer version.\n" : "Could not read the body plan.\n");
        return 1;
    }
    std::printf("Plan: %s\n", qPrintable(parser.positionalArguments().front()));

    auto t = std::chrono::steady_clock::now();
    const kp::Netz netz = geber->vernetze(kp::netzAuftrag(plan, netzHz));
    if (!netz.fehler.empty())
    {
        std::fprintf(stderr, "Mesh: %s\n", netz.fehler.c_str());
        return 1;
    }
    std::printf("Mesh (%s, for %.0f Hz): %zu nodes, %zu triangles, %.2f s\n", geber->name().c_str(), netzHz, netz.knoten.size(),
                netz.dreiecke.size(), seit(t));

    t = std::chrono::steady_clock::now();
    const kp::LagrangeDreieck element(ordnung);
    const kp::PlattenSystem s =
        kp::baueSystem(netz, element, [&](kp::Punkt2 p) { return kp::steifeBei(plan, p); }, kp::Auflager::Eingespannt);
    if (!s.fehler.empty())
    {
        std::fprintf(stderr, "Plate: %s\n", s.fehler.c_str());
        return 1;
    }
    std::printf("Plate (%s): %d unknowns, %lld entries per matrix, mass %.3f kg, %.2f s, peak %.0f MB\n", s.element.c_str(), s.k.n,
                static_cast<long long>(s.k.eintraege()), s.masse, seit(t), spitzeMB());

    // je Mode: Wert an den Stegpunkten, Daempfung, Abstrahlgewicht je Hoerort (31c-5b)
    t = std::chrono::steady_clock::now();
    const kp::Auswertung auswertung(s, element, plan.stegpunkte, plan.hoerorte);
    int ausserhalb = 0;
    for (const kp::Abtastung& a : auswertung.stegpunkte()) ausserhalb += a.imNetz ? 0 : 1;
    std::printf("Evaluation prepared: %zu bridge points (%d outside the mesh), %zu listening points, %.2f s\n", plan.stegpunkte.size(),
                ausserhalb, plan.hoerorte.size(), seit(t));
    std::vector<kp::ModeImKoerper> moden;
    double sekundenAuswertung = 0.0;

    kp::EigenAuftrag auftrag;
    auftrag.k = &s.k;
    auftrag.m = &s.m;
    auftrag.vonHz = vonHz;
    auftrag.bisHz = bisHz;
    auftrag.jeMode = [&](const kp::Mode& mode) {
        const auto start = std::chrono::steady_clock::now();
        moden.push_back(auswertung.werte(mode));
        sekundenAuswertung += seit(start);
    };
    int gemeldet = -1;
    auftrag.weiter = [&](double anteil) {
        const int prozent = static_cast<int>(anteil * 100.0);
        if (prozent / 5 != gemeldet / 5)
        {
            gemeldet = prozent;
            std::printf("  %3d %%  %.0f s\n", prozent, seit(beginn));
            std::fflush(stdout);
        }
        return true;
    };
    const kp::EigenErgebnis erg = loeser->loese(auftrag);
    if (!erg.fehler.empty())
    {
        std::fprintf(stderr, "Eigensolver: %s\n", erg.fehler.c_str());
        return 1;
    }
    std::printf("Modes (%s, %.0f to %.0f Hz): %zu (counted %d), %d factorizations (%.1f s), largest residual %.2e, %.1f s, peak %.0f MB\n",
                loeser->name().c_str(), vonHz, bisHz, erg.frequenzenHz.size(), erg.imFenster, erg.faktorisierungen,
                erg.sekundenZerlegung, erg.groessterRest, erg.sekunden, spitzeMB());
    std::printf("Evaluated %zu modes in %.1f s (within the eigensolver time)\n", moden.size(), sekundenAuswertung);

    // der statistische Teil darueber (31c-5c, ST1 a): Groessen aus der oberen Haelfte des Fensters
    const double statBis = parser.value(statOpt).toDouble();
    if (statBis > bisHz)
    {
        const double bandVon = std::max(vonHz, bisHz / 2.0);
        const kp::StatistischerTeil st =
            kp::schaetze(moden, bandVon, bisHz, bisHz, statBis, kp::Daempfung{}, parser.value(samenOpt).toULongLong());
        const auto gezogen = kp::ziehe(st);
        double stegMittel = 0.0;
        for (const double v : st.stegStreuung) stegMittel += v / static_cast<double>(std::max<std::size_t>(1, st.stegStreuung.size()));
        std::printf("Statistical part %.0f to %.0f Hz from %d modes in %.0f to %.0f Hz: density %.3f /Hz rising %.2e /Hz^2, "
                    "%zu modes drawn (seed %llu); mean spread at the bridge points %.3g",
                    st.vonHz, st.bisHz, st.bandModen, st.bandVonHz, st.bandBisHz, st.dichte, st.anstieg, gezogen.size(),
                    static_cast<unsigned long long>(st.samen), stegMittel);
        for (std::size_t h = 0; h < st.hoerBetrag.size(); ++h) std::printf(", |G| %s %.4g", plan.hoerorte[h].name.c_str(), st.hoerBetrag[h]);
        std::printf("\n");
    }
    for (std::size_t i = 0; i < std::min<std::size_t>(moden.size(), 5); ++i)
    {
        const kp::ModeImKoerper& m = moden[i];
        std::printf("  %8.2f Hz  eta %.4f", m.frequenzHz, m.verlustfaktor);
        for (std::size_t h = 0; h < m.abstrahlung.size(); ++h)
            std::printf("  |G| %s %.4g", plan.hoerorte[h].name.c_str(), std::abs(m.abstrahlung[h]));
        std::printf("\n");
    }

    if (parser.isSet(freqOpt))
    {
        QFile datei(parser.value(freqOpt));
        if (!datei.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            std::fprintf(stderr, "Could not write %s\n", qPrintable(parser.value(freqOpt)));
            return 1;
        }
        QTextStream aus(&datei);
        aus.setRealNumberPrecision(12);
        for (const double f : erg.frequenzenHz) aus << f << '\n';
        std::printf("Frequencies: %s\n", qPrintable(parser.value(freqOpt)));
    }
    std::printf("Total %.1f s\n", seit(beginn));
    return 0;
}
