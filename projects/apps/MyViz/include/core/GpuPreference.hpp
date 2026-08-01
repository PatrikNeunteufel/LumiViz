/**
 ****************************************************************************************
 * @file   GpuPreference.hpp
 * @brief  Persistente GPU-Auswahl ueber die Windows-GpuPreference-Registrierung
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Zur Laufzeit laesst sich die GPU in OpenGL nicht umschalten. Der verlaessliche
 * Weg fuer eine einstellbare Praeferenz ist der Windows-Eintrag pro Anwendung —
 * dasselbe, was die Windows-Oberflaeche unter "Grafikeinstellungen" schreibt:
 *
 *   HKCU\Software\Microsoft\DirectX\UserGpuPreferences
 *     Wertname = voller Exe-Pfad (native Separatoren)
 *     Daten    = "GpuPreference=N;"   N: 0=Automatisch, 1=Energiesparen,
 *                                        2=Hohe Leistung
 *
 * Der Eintrag greift beim NAECHSTEN Prozessstart — eine Aenderung braucht also
 * einen Neustart der App (SettingsPanel loest ihn sofort aus, Entscheid S61).
 * Diese Registrierung ist die EINZIGE Steuerung (SSOT); die frueheren
 * Export-Flags (NvOptimusEnablement u. a.) sind entfernt — Windows ueberstimmt
 * sie ohnehin, sobald ein UserGpuPreferences-Eintrag existiert.
 *
 * Die Token-Logik (parseToken/upsertToken) ist pur und ohne Registry testbar;
 * fremde Tokens im Wert (z. B. "SwapEffectUpgradeEnable=1;") bleiben erhalten.
 *
 * @see GpuPreference.md fuer die Modul-Doku
 ****************************************************************************************
 */

#pragma once

#include <optional>
#include <string>

/**
 * @class GpuPreference
 * @brief Liest und schreibt die per-Anwendung-GPU-Praeferenz von Windows.
 */
class GpuPreference
{
public:
    /**
     * @enum Mode
     * @brief Die drei Windows-Praeferenzstufen (Zahlenwerte = Registry-Werte).
     */
    enum class Mode
    {
        Automatic       = 0,  ///< Windows entscheidet
        PowerSaving     = 1,  ///< integrierte GPU bevorzugen
        HighPerformance = 2   ///< dedizierte GPU bevorzugen
    };

    // =========================================================================
    // Token-Logik (pur — ohne Registry testbar)
    // =========================================================================

    /**
     * @brief Liest den GpuPreference-Token aus einem Registry-Datenwert.
     *
     * @param value Datenwert, z. B. "GpuPreference=2;" oder mit fremden Tokens
     * @return Modus, oder nullopt wenn kein (oder ein unbekannter) Token
     */
    [[nodiscard]] static std::optional<Mode> parseToken(const std::string& value);

    /**
     * @brief Setzt oder ersetzt den GpuPreference-Token in einem Datenwert.
     *
     * Fremde Tokens bleiben unveraendert an ihrer Position; fehlt der Token,
     * wird er angehaengt. Ergebnis endet immer mit ';'.
     *
     * @param value bisheriger Datenwert (darf leer sein)
     * @param mode  gewuenschter Modus
     * @return neuer Datenwert
     */
    [[nodiscard]] static std::string upsertToken(const std::string& value, Mode mode);

    /** @brief Lesbarer Name eines Modus (fuers Log). */
    [[nodiscard]] static const char* modeToString(Mode mode);

    // =========================================================================
    // Registry (Windows; auf anderen Plattformen No-op)
    // =========================================================================

    /**
     * @brief Liest die gespeicherte Praeferenz fuer eine Exe.
     *
     * @param exePath voller Pfad mit nativen Separatoren (Backslashes)
     * @return Modus, oder nullopt wenn kein Eintrag existiert
     */
    [[nodiscard]] static std::optional<Mode> readForExecutable(
        const std::wstring& exePath);

    /**
     * @brief Schreibt die Praeferenz fuer eine Exe (legt den Schluessel an).
     *
     * @param exePath voller Pfad mit nativen Separatoren (Backslashes)
     * @param mode    gewuenschter Modus
     * @return true bei Erfolg
     */
    [[nodiscard]] static bool writeForExecutable(const std::wstring& exePath,
                                                 Mode mode);
};
