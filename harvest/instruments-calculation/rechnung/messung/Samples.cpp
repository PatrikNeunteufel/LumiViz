/**
 * @file Samples.cpp
 * @brief Messkette: Zugang zu den Aufnahmen ueber `IC_SAMPLES`
 */
#include "messung/Samples.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace ic::messung
{

std::optional<std::filesystem::path> samplesWurzelAus(const char* wert, std::string& meldung)
{
    if (wert == nullptr || *wert == '\0')
    {
        meldung = "IC_SAMPLES ist nicht gesetzt — die Aufnahmen liegen nur lokal (AUFTRAG_BAU §0); IC_SAMPLES auf den Ordner "
                  "`samples` setzen, der die Klassenordner `Klasse n - Name` enthaelt";
        return std::nullopt;
    }
    const std::filesystem::path p(wert);
    std::error_code fehler;
    if (!std::filesystem::is_directory(p, fehler))
    {
        meldung = "IC_SAMPLES zeigt auf keinen Ordner: " + p.string() + " (externe Festplatte nicht eingehaengt?)";
        return std::nullopt;
    }
    return p;
}

std::optional<std::filesystem::path> samplesWurzel(std::string& meldung)
{
    return samplesWurzelAus(std::getenv("IC_SAMPLES"), meldung);
}

std::vector<SummenEintrag> liesSummen(const std::filesystem::path& datei)
{
    std::vector<SummenEintrag> e;
    std::ifstream ein(datei);
    std::string zeile;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        if (zeile.size() < 66 || zeile[0] == '#') continue;
        SummenEintrag s;
        s.sha256 = zeile.substr(0, 64);
        std::size_t ab = 64;
        while (ab < zeile.size() && (zeile[ab] == ' ' || zeile[ab] == '*')) ++ab;
        s.pfad = zeile.substr(ab);
        e.push_back(s);
    }
    return e;
}

std::string sha256Datei(const std::filesystem::path& datei)
{
    std::ifstream ein(datei, std::ios::binary);
    if (!ein) return {};
    Sha256 h;
    std::vector<char> puffer(1 << 20);
    while (ein)
    {
        ein.read(puffer.data(), static_cast<std::streamsize>(puffer.size()));
        const auto n = static_cast<std::size_t>(ein.gcount());
        if (n > 0) h.dazu(reinterpret_cast<const std::uint8_t*>(puffer.data()), n);
    }
    return h.fertig();
}

bool pruefeDatei(const std::filesystem::path& datei, const std::string& sha256)
{
    return sha256Datei(datei) == sha256;
}

}  // namespace ic::messung
