/**
 * @file Tonspeicher.hpp
 * @brief Loop: ein Speicher je Ton fuer das ganze Programm
 *        (docs/audio/Spielweisen_Konzept.md §4.5, Stufe 29b; Entscheide H1 a, H3 a,
 *        TV2 a, TV4 a)
 *
 * Was einmal gerechnet oder dekodiert ist, liegt hier — Loop-Panel, Composer und
 * alle Clips desselben Loops teilen es, eine Bearbeitung rechnet nur die neuen
 * Toene. Der Speicher liegt im Arbeitsspeicher und hat eine Grenze; ist sie
 * erreicht, geht der am laengsten nicht gebrauchte Ton. Ein Programm, das einen
 * verworfenen Ton noch haelt, behaelt ihn, bis es ihn loslaesst.
 *
 * Der Schluessel ist ein Fingerabdruck der Eingaben (TV2 a): der Tonname, die
 * Rate und `instrumentAbdruck` — aendert sich ein Instrument, bekommen seine
 * Toene neue Schluessel, die alten fallen mit der Zeit heraus. Von jedem Thread
 * aus benutzbar.
 */
#pragma once

#include "loop/Instrument.hpp"
#include "loop/Rechenkern.hpp"

#include <QByteArray>
#include <QString>

#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

namespace lumi::loop
{

/// Die Grenze, solange nichts anderes eingestellt ist (H3 a): 1 GB.
inline constexpr std::size_t kTonspeicherVorgabeBytes = std::size_t{1} << 30;

/// Was der Speicher gerade haelt und was er bisher getan hat.
struct TonspeicherStand
{
    std::size_t toene = 0;        ///< Eintraege
    std::size_t bytes = 0;        ///< ihre Abtastwerte zusammen
    std::uint64_t treffer = 0;    ///< `hole` fand den Ton
    std::uint64_t gelegt = 0;     ///< `lege` nahm einen neuen auf
    std::uint64_t verworfen = 0;  ///< an der Grenze gegangen
};

class Tonspeicher
{
public:
    /// Der Ton zum Schluessel, nullptr = keiner. Ein gefundener gilt als gerade gebraucht.
    [[nodiscard]] std::shared_ptr<const Klang> hole(const QString& schluessel);
    /// Einen Ton aufnehmen (ein vorhandener zum selben Schluessel wird ersetzt), mit der
    /// Zeit, die er zum Rechnen oder Dekodieren brauchte. Danach gilt die Grenze.
    void lege(const QString& schluessel, std::shared_ptr<const Klang> klang, double rechenzeitMs);
    /// Die gemessene Zeit des Tons (Grundlage fuer F4 a); leer = nicht im Speicher.
    [[nodiscard]] std::optional<double> rechenzeitMs(const QString& schluessel) const;

    void setzeGrenze(std::size_t bytes);
    [[nodiscard]] std::size_t grenze() const;
    [[nodiscard]] TonspeicherStand stand() const;
    /// Alles vergessen, auch die Zaehler (fuer Tests und nach einem Import).
    void leere();

private:
    struct Eintrag
    {
        QString schluessel;
        std::shared_ptr<const Klang> klang;
        std::size_t bytes = 0;
        double rechenzeitMs = 0.0;
    };
    void haltGrenze();  ///< unter der Sperre

    mutable std::mutex m_sperre;
    std::list<Eintrag> m_reihe;  ///< vorn der zuletzt gebrauchte
    std::map<QString, std::list<Eintrag>::iterator> m_orte;
    std::size_t m_grenze = kTonspeicherVorgabeBytes;
    TonspeicherStand m_stand;
};

/// Der eine Speicher des Programms.
[[nodiscard]] Tonspeicher& tonspeicher();

/// Der Fingerabdruck eines Instruments (TV2 a): ein Hash ueber das Instrument, wie es
/// in die Datei geschrieben wuerde, je Sample-Schicht Ort, Groesse und Aenderungszeit
/// der Datei und je Shape-Schicht der Inhalt der Zeichnung.
[[nodiscard]] QByteArray instrumentAbdruck(const Instrument& instrument);

/// Der Schluessel eines gerechneten Tons (`inst:…`).
[[nodiscard]] QString tonSchluessel(const QString& tonName, int rate, const QByteArray& abdruck);

/// Der Schluessel eines dekodierten Klangs (`kit:`, `lib:`, eigenes Sample, Datei):
/// Name, Rate, Ort, Groesse und Aenderungszeit der Datei.
[[nodiscard]] QString klangSchluessel(const QString& klangName, const QString& datei, int rate);

}  // namespace lumi::loop
