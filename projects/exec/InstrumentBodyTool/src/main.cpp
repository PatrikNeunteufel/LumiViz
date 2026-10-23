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

#include "InstrumentBody/Platte.hpp"
#include "InstrumentBody/Umsetzungen.hpp"
#include "loop/KoerperBauplan.hpp"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>

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
    parser.addOptions({bisOpt, vonOpt, netzOpt, ordnungOpt, geberOpt, loeserOpt, freqOpt, listeOpt});
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
    const kp::PlattenSystem s = kp::baueSystem(netz, kp::LagrangeDreieck(ordnung), [&](kp::Punkt2 p) { return kp::steifeBei(plan, p); },
                                               kp::Auflager::Eingespannt);
    if (!s.fehler.empty())
    {
        std::fprintf(stderr, "Plate: %s\n", s.fehler.c_str());
        return 1;
    }
    std::printf("Plate (%s): %d unknowns, %lld entries per matrix, mass %.3f kg, %.2f s, peak %.0f MB\n", s.element.c_str(), s.k.n,
                static_cast<long long>(s.k.eintraege()), s.masse, seit(t), spitzeMB());

    kp::EigenAuftrag auftrag;
    auftrag.k = &s.k;
    auftrag.m = &s.m;
    auftrag.vonHz = vonHz;
    auftrag.bisHz = bisHz;
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
