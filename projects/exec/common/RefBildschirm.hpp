/**
 ****************************************************************************************
 * @file   RefBildschirm.hpp
 * @brief  Fester Bildschirm für Referenzläufe — `LUMIVIZ_REF_BILDSCHIRM`
 *         (MilkdropRef, MilkdropStandalone)
 *
 * @author Patrik Neunteufel
 * @date   September 2026 (Session 81)
 * @version 1.0.0
 *
 * @details
 * Ohne Vorgabe legt Windows (`CW_USEDEFAULT`) bzw. Qt die Fenster der
 * Referenzläufe irgendwohin — meist auf den Arbeitsbildschirm. Mit der
 * Umgebungsvariable landen beide Seiten oben links im Arbeitsbereich EINES
 * festen Monitors:
 *
 * - `LUMIVIZ_REF_BILDSCHIRM=DISPLAY2` — Gerätename, wie ihn Windows unter
 *   Einstellungen → System → Anzeige → „Identifizieren" als Ziffer zeigt
 *   (auch `\\.\DISPLAY2`, Groß-/Kleinschreibung egal). Bevorzugt: bleibt
 *   gleich, wenn die Anordnung der Monitore wechselt.
 * - `LUMIVIZ_REF_BILDSCHIRM=1` — Rückfall: Index (0-basiert) in der
 *   Aufzählungsreihenfolge von `EnumDisplayMonitors`.
 *
 * Nicht gesetzt oder leer → Verhalten wie bisher. Gesetzt, aber kein Monitor
 * passt → Warnung mit der Liste der vorhandenen Monitore, Fenster bleibt am
 * Standardort (ein Lauf scheitert nie an der Platzierung).
 *
 * Die Platzierung ändert nur, WO das Fenster steht — nicht seine Größe und
 * nicht das Bild: beide Werkzeuge rendern in Puffer fester Größe (`--size`).
 *
 * Bewusst **ohne Qt und ohne C++20** (wie SynthAudio.hpp): `tools/MilkdropRef`
 * ist ein 32-bit-D3D9-Host außerhalb der CMakeCraft-Solution. Der Standalone
 * nimmt von hier nur das Monitor-Rechteck und sucht sich den passenden
 * `QScreen` selbst (Qt behält beim High-DPI-Mapping die linke obere Ecke).
 ****************************************************************************************
 */

#pragma once

#ifdef _WIN32

// Im Qt-Standalone sonst min/max-Makros über std::max (MilkdropRef bindet
// windows.h ohnehin vorher ein und schreibt `(std::max)`)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace lumi::werkzeug
{

/// Name der Umgebungsvariable (eine Stelle für beide Werkzeuge und die Doku)
inline constexpr const char* kRefBildschirmVar = "LUMIVIZ_REF_BILDSCHIRM";

/// Ein Monitor, wie ihn `EnumDisplayMonitors` liefert
struct RefMonitor
{
    std::string name;  ///< Gerätename ohne `\\.\`, z. B. "DISPLAY2"
    RECT monitor{};    ///< ganzer Monitor (virtuelle Desktop-Koordinaten)
    RECT arbeit{};     ///< Arbeitsbereich (ohne Taskleiste)
    bool primaer = false;
};

namespace detail
{
inline BOOL CALLBACK refMonitorSammeln(HMONITOR mon, HDC, LPRECT, LPARAM ziel)
{
    MONITORINFOEXA info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    if (GetMonitorInfoA(mon, &info))
    {
        RefMonitor m;
        m.name = info.szDevice;
        if (m.name.rfind("\\\\.\\", 0) == 0) m.name.erase(0, 4);
        m.monitor = info.rcMonitor;
        m.arbeit = info.rcWork;
        m.primaer = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
        reinterpret_cast<std::vector<RefMonitor>*>(ziel)->push_back(m);
    }
    return TRUE;
}

inline std::string refGross(std::string s)
{
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if (s.rfind("\\\\.\\", 0) == 0) s.erase(0, 4);
    return s;
}
}  // namespace detail

/// Alle Monitore in Aufzählungsreihenfolge
inline std::vector<RefMonitor> refMonitore()
{
    std::vector<RefMonitor> liste;
    EnumDisplayMonitors(nullptr, nullptr, detail::refMonitorSammeln,
                        reinterpret_cast<LPARAM>(&liste));
    return liste;
}

/**
 * @brief Monitor nach `LUMIVIZ_REF_BILDSCHIRM` suchen
 * @param[out] treffer der gewählte Monitor
 * @param werkzeug Präfix für die Konsolenmeldung, z. B. "[MilkdropRef]"
 * @return true, wenn die Variable gesetzt ist UND ein Monitor passt
 */
inline bool refBildschirmSuchen(RefMonitor& treffer, const char* werkzeug)
{
    const char* roh = std::getenv(kRefBildschirmVar);
    if (roh == nullptr || *roh == '\0') return false;

    const std::string wunsch = detail::refGross(roh);
    const std::vector<RefMonitor> liste = refMonitore();
    for (const RefMonitor& m : liste)
        if (detail::refGross(m.name) == wunsch)
        {
            treffer = m;
            std::printf("%s Bildschirm %s (%s=%s)\n", werkzeug, m.name.c_str(),
                        kRefBildschirmVar, roh);
            return true;
        }
    // Rückfall Index — nur wenn der Wert ganz aus Ziffern besteht
    char* ende = nullptr;
    const long idx = std::strtol(roh, &ende, 10);
    if (ende != roh && *ende == '\0' && idx >= 0 && idx < static_cast<long>(liste.size()))
    {
        treffer = liste[static_cast<size_t>(idx)];
        std::printf("%s Bildschirm %s (%s=%s, Index)\n", werkzeug, treffer.name.c_str(),
                    kRefBildschirmVar, roh);
        return true;
    }
    std::fprintf(stderr, "%s WARNUNG: %s=%s passt auf keinen Monitor — Standardort. "
                         "Vorhanden:\n", werkzeug, kRefBildschirmVar, roh);
    for (size_t i = 0; i < liste.size(); ++i)
        std::fprintf(stderr, "  [%d] %s%s\n", static_cast<int>(i), liste[i].name.c_str(),
                     liste[i].primaer ? " (primaer)" : "");
    return false;
}

}  // namespace lumi::werkzeug

#endif  // _WIN32
