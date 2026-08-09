/**
 ****************************************************************************************
 * @file   AppInfo.hpp
 * @brief  Name und Version der Anwendung — die EINZIGE Quelle im Code (S73)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Beide Werte stammen aus `Solution.json` (`apps[0].name` / `apps[0].version`).
 * Das Root-`CMakeLists.txt` liest sie dort heraus und reicht sie als
 * `LUMI_APP_NAME` / `LUMI_APP_VERSION` herein.
 *
 * **Nicht anderswo hinschreiben.** Bis S73 standen sie dreifach: in
 * `Solution.json`, hart in `Application.cpp` und als Literal `"Version 0.1.0"`
 * im About-Dialog. Zwei davon wurden nie nachgezogen — der About-Dialog zeigte
 * dauerhaft eine Version, die es nicht gab.
 *
 * Die Ersatzwerte unten greifen nur, wenn jemand diese Dateien ohne das
 * Root-`CMakeLists.txt` uebersetzt (etwa in einem fremden Build). Sie sind
 * absichtlich als solche erkennbar, damit ein falscher Wert auffaellt statt
 * plausibel auszusehen.
 ****************************************************************************************
 */

#pragma once

#include <QString>

#ifndef LUMI_APP_NAME
#define LUMI_APP_NAME "LumiViz (Name nicht aus Solution.json)"
#endif

#ifndef LUMI_APP_VERSION
#define LUMI_APP_VERSION "0.0.0-unbekannt"
#endif

namespace lumi
{

/// Anwendungsname, z. B. "LumiViz".
[[nodiscard]] inline QString appName()
{
    return QStringLiteral(LUMI_APP_NAME);
}

/// Version ohne Praefix, z. B. "0.2.0".
[[nodiscard]] inline QString appVersion()
{
    return QStringLiteral(LUMI_APP_VERSION);
}

/// Anzeigefertig, z. B. "LumiViz 0.2.0".
[[nodiscard]] inline QString appNameAndVersion()
{
    return appName() + QLatin1Char(' ') + appVersion();
}

}  // namespace lumi
