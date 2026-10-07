/**
 * @file EinzelInstanz.hpp
 * @brief Eine laufende App je Exe-Datei; ein zweiter Start gibt seine Datei
 *        an die laufende ab (S101, Entscheide Patrik X1: a, X1b: a)
 *
 * Der Explorer startet fuer jeden Doppelklick das Programm neu. Damit die
 * Datei im laufenden LumiViz ankommt, statt ein zweites zu oeffnen:
 *
 *  - Der erste Start nimmt eine Sperre (eine Sperrdatei im Temp-Ordner) und
 *    lauscht unter einem Namen (lokaler Kanal, `QLocalServer`).
 *  - Jeder weitere Start derselben Exe bekommt die Sperre nicht, schickt den
 *    Pfad seiner Datei (oder nichts) an die laufende App und beendet sich.
 *
 * **Je Exe-Datei:** der Name haengt am Pfad der Exe und am Benutzer. Zwei
 * verschiedene Builds (Debug, Testing, Release) laufen nebeneinander, zweimal
 * dieselbe Exe nicht. Der Schalter `--new-instance` umgeht das Ganze.
 *
 * **Grenze:** haelt eine App die Sperre, antwortet aber innerhalb der Geduld
 * nicht (sie haengt oder faehrt gerade erst hoch und lauscht noch nicht),
 * laeuft der neue Start fuer sich weiter (`Rolle::Allein`), ohne zu lauschen.
 */
#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

class QLocalServer;
class QLockFile;

namespace lumi::core
{

class EinzelInstanz : public QObject
{
    Q_OBJECT

public:
    enum class Rolle
    {
        Erste,       ///< diese App ist die laufende und lauscht
        Uebergeben,  ///< eine andere laeuft und hat die Datei bekommen — beenden
        Allein       ///< eine andere haelt die Sperre, war aber nicht zu erreichen
    };

    explicit EinzelInstanz(QObject* parent = nullptr);
    ~EinzelInstanz() override;

    /// Der Name fuer diese Exe und diesen Benutzer. Gross- und Kleinschreibung
    /// und die Art der Trennzeichen im Pfad spielen keine Rolle.
    [[nodiscard]] static QString nameFuer(const QString& exePfad);
    /// Die Sperrdatei zu einem Namen (Temp-Ordner).
    [[nodiscard]] static QString sperrDatei(const QString& name);
    /// Steht `--new-instance` unter den Argumenten?
    [[nodiscard]] static bool neueInstanzVerlangt(const QStringList& argumente);

    /// Die Sperre nehmen und lauschen — oder `datei` (leer = nur das Fenster
    /// nach vorn) an die laufende App geben. `geduldMs`: so lange wird eine
    /// App, die die Sperre haelt, zu erreichen versucht.
    Rolle starte(const QString& name, const QString& datei, int geduldMs = 3000);

Q_SIGNALS:
    /// Ein weiterer Start hat seine Datei abgegeben (leer = keine).
    void dateiUebergeben(const QString& datei);

private:
    /// true = die laufende App war zu erreichen und hat den Pfad bekommen.
    [[nodiscard]] static bool uebergib(const QString& name, const QString& datei, int wartenMs);
    void nimmVerbindungAn();

    std::unique_ptr<QLockFile> m_sperre;
    QLocalServer* m_server = nullptr;
};

}  // namespace lumi::core
