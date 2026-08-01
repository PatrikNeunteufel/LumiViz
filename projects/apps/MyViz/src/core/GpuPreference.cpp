/**
 ****************************************************************************************
 * @file   GpuPreference.cpp
 * @brief  Persistente GPU-Auswahl ueber die Windows-GpuPreference-Registrierung
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 *
 * @see GpuPreference.hpp fuer die Schnittstelle, GpuPreference.md fuer die Doku
 ****************************************************************************************
 */

#include "core/GpuPreference.hpp"

#include <BasicLogger.h>

#include <vector>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

namespace
{

constexpr const char* kTokenKey = "GpuPreference=";

#ifdef _WIN32
constexpr const wchar_t* kRegistryPath =
    L"Software\\Microsoft\\DirectX\\UserGpuPreferences";
#endif

/// Zerlegt "a=1;b=2;" in Tokens ohne die abschliessenden Semikola.
std::vector<std::string> splitTokens(const std::string& value)
{
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    while (start < value.size())
    {
        auto end = value.find(';', start);
        if (end == std::string::npos)
        {
            end = value.size();
        }
        if (end > start)
        {
            tokens.push_back(value.substr(start, end - start));
        }
        start = end + 1;
    }
    return tokens;
}

bool isPreferenceToken(const std::string& token)
{
    return token.rfind(kTokenKey, 0) == 0;
}

} // anonymous namespace

// =============================================================================
// Token-Logik
// =============================================================================

std::optional<GpuPreference::Mode> GpuPreference::parseToken(const std::string& value)
{
    for (const auto& token : splitTokens(value))
    {
        if (!isPreferenceToken(token))
        {
            continue;
        }
        const std::string number = token.substr(std::string(kTokenKey).size());
        if (number == "0") { return Mode::Automatic; }
        if (number == "1") { return Mode::PowerSaving; }
        if (number == "2") { return Mode::HighPerformance; }
        return std::nullopt;  // unbekannter Wert — nicht raten
    }
    return std::nullopt;
}

std::string GpuPreference::upsertToken(const std::string& value, Mode mode)
{
    const std::string mine =
        std::string(kTokenKey) + std::to_string(static_cast<int>(mode));

    auto tokens = splitTokens(value);
    bool replaced = false;
    for (auto& token : tokens)
    {
        if (isPreferenceToken(token))
        {
            token = mine;
            replaced = true;
        }
    }
    if (!replaced)
    {
        tokens.push_back(mine);
    }

    std::string result;
    for (const auto& token : tokens)
    {
        result += token;
        result += ';';
    }
    return result;
}

const char* GpuPreference::modeToString(Mode mode)
{
    switch (mode)
    {
        case Mode::Automatic:       return "Automatic";
        case Mode::PowerSaving:     return "PowerSaving";
        case Mode::HighPerformance: return "HighPerformance";
    }
    return "Unknown";
}

// =============================================================================
// Registry
// =============================================================================

#ifdef _WIN32

std::optional<GpuPreference::Mode> GpuPreference::readForExecutable(
    const std::wstring& exePath)
{
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRegistryPath, 0, KEY_QUERY_VALUE,
                      &key) != ERROR_SUCCESS)
    {
        return std::nullopt;  // Schluessel existiert noch nicht
    }

    wchar_t buffer[256]{};
    DWORD size = sizeof(buffer);
    DWORD type = 0;
    const LSTATUS status = RegQueryValueExW(
        key, exePath.c_str(), nullptr, &type,
        reinterpret_cast<LPBYTE>(buffer), &size);
    RegCloseKey(key);

    if (status != ERROR_SUCCESS || type != REG_SZ)
    {
        return std::nullopt;
    }

    // Tokens sind reines ASCII — einfache Verengung genuegt.
    std::string narrow;
    for (const wchar_t* p = buffer; *p != L'\0'; ++p)
    {
        narrow += static_cast<char>(*p);
    }
    return parseToken(narrow);
}

bool GpuPreference::writeForExecutable(const std::wstring& exePath, Mode mode)
{
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRegistryPath, 0, nullptr, 0,
                        KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &key,
                        nullptr) != ERROR_SUCCESS)
    {
        BasicLogger::logError("GpuPreference: UserGpuPreferences-Schluessel "
                              "nicht anlegbar/oeffenbar");
        return false;
    }

    // Bestehenden Wert lesen, damit fremde Tokens erhalten bleiben.
    std::string current;
    {
        wchar_t buffer[256]{};
        DWORD size = sizeof(buffer);
        DWORD type = 0;
        if (RegQueryValueExW(key, exePath.c_str(), nullptr, &type,
                             reinterpret_cast<LPBYTE>(buffer),
                             &size) == ERROR_SUCCESS &&
            type == REG_SZ)
        {
            for (const wchar_t* p = buffer; *p != L'\0'; ++p)
            {
                current += static_cast<char>(*p);
            }
        }
    }

    const std::string updated = upsertToken(current, mode);
    std::wstring wide(updated.begin(), updated.end());

    const LSTATUS status = RegSetValueExW(
        key, exePath.c_str(), 0, REG_SZ,
        reinterpret_cast<const BYTE*>(wide.c_str()),
        static_cast<DWORD>((wide.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(key);

    if (status != ERROR_SUCCESS)
    {
        BasicLogger::logError("GpuPreference: Schreiben fehlgeschlagen (Status " +
                              std::to_string(status) + ")");
        return false;
    }

    BasicLogger::logInfo(std::string("GpuPreference: ") + modeToString(mode) +
                         " gespeichert (greift beim naechsten Start)");
    return true;
}

#else // !_WIN32 — die Praeferenz ist ein Windows-Mechanismus

std::optional<GpuPreference::Mode> GpuPreference::readForExecutable(
    const std::wstring& /*exePath*/)
{
    return std::nullopt;
}

bool GpuPreference::writeForExecutable(const std::wstring& /*exePath*/,
                                       Mode /*mode*/)
{
    return false;
}

#endif
