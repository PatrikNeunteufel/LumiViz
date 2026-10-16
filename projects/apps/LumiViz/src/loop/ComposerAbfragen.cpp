/**
 * @file ComposerAbfragen.cpp
 * @brief Loop: was der Loop vom Composer wissen muss (S128, N1 b)
 */

#include "loop/ComposerAbfragen.hpp"

#include <utility>

namespace lumi::loop
{

namespace
{

ComposerAbfragen& angemeldet()
{
    static ComposerAbfragen abfragen;
    return abfragen;
}

}  // namespace

void setzeComposerAbfragen(ComposerAbfragen abfragen)
{
    angemeldet() = std::move(abfragen);
}

bool composerFuehrtTon()
{
    const auto& f = angemeldet().fuehrtTon;
    return f ? f() : false;
}

double composerLoopStelleMs(const QString& datei)
{
    const auto& f = angemeldet().loopStelleMs;
    return f ? f(datei) : -1.0;
}

bool mitlaufenSeitenweise()
{
    const auto& f = angemeldet().mitlaufenSeitenweise;
    return f ? f() : false;
}

}  // namespace lumi::loop
