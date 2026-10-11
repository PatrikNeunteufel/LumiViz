/**
 * @file Offen.cpp
 * @brief Loop: der Loop, den das Loop-Panel gerade offen hat
 */

#include "loop/Offen.hpp"

#include <QFileInfo>

namespace lumi::loop
{

namespace
{

struct Ablage
{
    QString datei;
    Loop loop;
    std::uint64_t fassung = 0;
};

Ablage& ablage()
{
    static Ablage eine;
    return eine;
}

}  // namespace

void meldeOffen(const QString& datei, const Loop& loop)
{
    Ablage& a = ablage();
    a.datei = datei;
    a.loop = loop;
    ++a.fassung;
}

std::uint64_t offenFassung()
{
    return ablage().fassung;
}

bool offenerLoop(const QString& datei, Loop& loop)
{
    const Ablage& a = ablage();
    // dieselbe Datei, gleich wie der Pfad geschrieben ist
    if (a.datei.isEmpty() || datei.isEmpty() || QFileInfo(a.datei) != QFileInfo(datei)) return false;
    loop = a.loop;
    return true;
}

}  // namespace lumi::loop
