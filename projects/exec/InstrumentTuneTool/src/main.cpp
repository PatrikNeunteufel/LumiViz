/**
 ****************************************************************************************
 * @file   main.cpp
 * @brief  InstrumentTuneTool — stimmt die gestimmten Werte einer `.lvinst` und setzt ihren Bezug
 *         (docs/audio/Instrumente_IC_Konzept.md IK2-10 a, IK2-11 a; Stufe 34f-3, S177)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Konsole, ohne Fenster. Je Kern-Schicht der Datei: die Werte nach der Tafel lesen (`loop/KernDatei`), mit dem
 * Stimmer des Kerns (`messung::stimmer()`) die veralteten gestimmten Werte neu rechnen (mit `--all` jeden), ihre
 * Abdruecke merken, mit `--reference` den Bezug ueber `kern::normiereNachTafel` setzen und die Datei schreiben. Ohne
 * Aenderung wird nichts geschrieben; `--check` schreibt nie und gibt 1 zurueck, wenn etwas veraltet ist (wie der
 * Waechter `*Instrument Stimmen*`).
 *
 * Geschrieben wird mit Qt eingerueckt — fuer die mitgelieferten danach `python asset/loop/instrument_katalog.py`, das
 * die Form und den Kopf aus dem Katalog setzt (Befehl in `asset/loop/README.md`).
 ****************************************************************************************
 */

#include "InstrumentMeasure/Stimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "loop/KernDatei.hpp"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTextStream>

#include <string>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("InstrumentTuneTool"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Tunes the tuned values of an instrument (.lvinst) and sets its output reference."));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("instrument"), QStringLiteral("The instrument (.lvinst)."));
    const QCommandLineOption alleOpt(QStringLiteral("all"), QStringLiteral("Tune every tuned value, not only the outdated ones."));
    const QCommandLineOption bezugOpt(QStringLiteral("reference"), QStringLiteral("Set the output reference anew (normiere)."));
    const QCommandLineOption pruefOpt(QStringLiteral("check"), QStringLiteral("Write nothing; exit 1 if anything is outdated."));
    parser.addOptions({alleOpt, bezugOpt, pruefOpt});
    parser.process(app);
    QTextStream aus(stdout);
    QTextStream fehlerAus(stderr);
    if (parser.positionalArguments().size() != 1)
    {
        parser.showHelp(2);
    }
    const QString pfad = parser.positionalArguments().front();
    QFile f(pfad);
    if (!f.open(QIODevice::ReadOnly))
    {
        fehlerAus << "cannot read " << pfad << "\n";
        return 2;
    }
    QJsonParseError leseFehler{};
    QJsonObject instrument = QJsonDocument::fromJson(f.readAll(), &leseFehler).object();
    f.close();
    if (leseFehler.error != QJsonParseError::NoError)
    {
        fehlerAus << pfad << ": " << leseFehler.errorString() << "\n";
        return 2;
    }

    const lumi::kern::Fabrik& fabrik = lumi::kern::kerne();
    bool geaendert = false, veraltet = false, fehler = false;
    QJsonArray schichten = instrument.value(QStringLiteral("layers")).toArray();
    for (qsizetype i = 0; i < schichten.size(); ++i)
    {
        QJsonObject schicht = schichten[i].toObject();
        if (!schicht.contains(QStringLiteral("core"))) continue;
        const std::string kern = schicht.value(QStringLiteral("core")).toString().toStdString();
        const lumi::kern::KernTyp* typ = fabrik.typ(kern);
        if (typ == nullptr)
        {
            fehlerAus << "layer " << i << ": unknown core " << QString::fromStdString(kern) << "\n";
            fehler = true;
            continue;
        }
        lumi::kern::Werte werte = lumi::loop::werteAusJson(typ->tafel, schicht.value(QStringLiteral("coreParams")).toObject());
        if (parser.isSet(pruefOpt))
        {
            for (const std::string& n : lumi::messung::veralteteWerte(fabrik, kern, werte))
            {
                aus << "layer " << i << ": " << QString::fromStdString(n) << " outdated\n";
                veraltet = true;
            }
            if (lumi::messung::bezugVeraltet(fabrik, kern, werte))
            {
                aus << "layer " << i << ": " << QString::fromStdString(typ->tafel.bezugWert) << " outdated\n";
                veraltet = true;
            }
            continue;
        }
        const lumi::messung::StimmBericht b =
            lumi::messung::stimmeUndNormiere(fabrik, lumi::messung::stimmer(), kern, werte, parser.isSet(alleOpt), parser.isSet(bezugOpt));
        for (const std::string& e : b.fehler)
        {
            fehlerAus << "layer " << i << ": " << QString::fromStdString(e) << "\n";
            fehler = true;
        }
        for (const std::string& n : b.gestimmt) aus << "layer " << i << ": tuned " << QString::fromStdString(n) << "\n";
        if (b.mitBezug && parser.isSet(bezugOpt))
            aus << "layer " << i << ": " << QString::fromStdString(typ->tafel.bezugWert) << " " << b.bezugVorher << " -> " << b.bezug << "\n";
        if (!b.geaendert()) continue;
        schicht[QStringLiteral("coreParams")] = lumi::loop::werteAlsJson(typ->tafel, werte);
        schichten[i] = schicht;
        geaendert = true;
    }
    if (parser.isSet(pruefOpt))
    {
        aus << (veraltet ? "outdated\n" : "up to date\n");
        return fehler ? 2 : (veraltet ? 1 : 0);
    }
    if (fehler) return 2;
    if (!geaendert)
    {
        aus << "nothing to do\n";
        return 0;
    }
    instrument[QStringLiteral("layers")] = schichten;
    QSaveFile s(pfad);
    if (!s.open(QIODevice::WriteOnly) || s.write(QJsonDocument(instrument).toJson(QJsonDocument::Indented)) < 0 || !s.commit())
    {
        fehlerAus << "cannot write " << pfad << "\n";
        return 2;
    }
    aus << "written " << pfad << "\n";
    return 0;
}
