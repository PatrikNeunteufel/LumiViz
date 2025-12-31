/**
 ****************************************************************************************
 * @file   MenuInit.hpp
 * @brief  Initialization functions for menu self-registration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Linker-Problem bei Self-Registration
 *
 * Bei statischen Libraries kann der Linker Translation Units entfernen,
 * die keine externen Referenzen haben ("dead code elimination").
 * Die Self-Registration Makros erzeugen nur interne statische Objekte,
 * die der Linker als "unused" betrachten kann.
 *
 * ## Lösung
 *
 * Diese Header deklariert Init-Funktionen, die in den AutoReg-Dateien
 * definiert sind. Durch Aufruf dieser Funktionen wird der Linker gezwungen,
 * die entsprechenden Translation Units einzubinden.
 *
 * ## Verwendung
 *
 * ```cpp
 * // In main.cpp oder Application.cpp:
 * #include "UI/managers/MenuInit.hpp"
 *
 * int main() {
 *     initMenuRegistrations();  // MUSS vor MenuManager::buildMenuBar() aufgerufen werden!
 *     // ...
 * }
 * ```
 ****************************************************************************************
 */

#pragma once

/**
 * @brief Initialize menu container registrations
 *
 * Ensures MenuAutoReg.cpp is linked and its static registrations are executed.
 */
void initMenuAutoReg();

/**
 * @brief Initialize menu item registrations
 *
 * Ensures MenuItemsAutoReg.cpp is linked and its static registrations are executed.
 */
void initMenuItemsAutoReg();

/**
 * @brief Initialize all menu registrations
 *
 * Convenience function that calls all menu init functions.
 * Call this once at application startup, before creating menus.
 */
inline void initMenuRegistrations()
{
    initMenuAutoReg();
    initMenuItemsAutoReg();
}
