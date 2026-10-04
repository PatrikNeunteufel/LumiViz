/**
 * @file Wiedergabe.hpp
 * @brief Composer: eine Komposition an einer Stelle der Zeitachse zum Bild
 *        machen (docs/ui/Composer_Konzept.md §5, Stufe C1b)
 *
 * Je laufendem Clip ein eigener Renderer; der Mischer fuehrt ihre Bilder
 * zusammen, von unten nach oben in Kanalreihenfolge, jeder Clip mit dem Pegel
 * aus seiner Kurve und seiner Mischart. Der Grund ist Schwarz.
 *
 *  - **Vorwaermen:** kurz vor seinem Start wird der Renderer eines Clips
 *    angelegt und rechnet ein Bild — das Kompilieren faellt nicht in den
 *    ersten sichtbaren Moment.
 *  - **Entladen:** endet ein Clip (oder springt die Stelle weg), wird sein
 *    Renderer weggeworfen. Betritt man ihn wieder, beginnt er frisch.
 *  - **Karaoke-Ebene:** die Renderer der Clips lassen sie aus; ein eigener
 *    Renderer rechnet sie, und sie wird einmal ueber das Gemischte gelegt.
 *
 * Kein Panel, keine Uhr: wer zeichnet, nennt die Stelle. Braucht einen
 * aktuellen GL-Kontext — auch beim Zerstoeren.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Komposition.hpp"
#include "composer/Mischer.hpp"

#include <QSize>
#include <QStringList>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <utility>

class MultiEffectVisualizer;

namespace lumi::composer
{

class Wiedergabe
{
public:
    Wiedergabe();
    ~Wiedergabe();
    Wiedergabe(const Wiedergabe&) = delete;
    Wiedergabe& operator=(const Wiedergabe&) = delete;

    /// Neue Komposition: alle Renderer werden entladen.
    void setKomposition(Komposition komposition);
    [[nodiscard]] const Komposition& komposition() const { return m_komposition; }

    /// So lange vor seinem Start wird ein Clip vorgewaermt (Vorgabe 2 s; 0 = nie).
    void setVorlaufMs(std::int64_t ms) { m_vorlaufMs = ms < 0 ? 0 : ms; }
    [[nodiscard]] std::int64_t vorlaufMs() const { return m_vorlaufMs; }

    /// Das Bild der Stelle `stelleMs` rechnen. `deltaSekunden` ist die Zeit
    /// seit dem letzten Bild (fuer die Bewegung in den Visuals; die PEGEL
    /// kommen allein aus der Stelle). false = der Mischer liess sich nicht
    /// anlegen. Das Ergebnis haelt `mischer()`.
    bool rendere(std::int64_t stelleMs, float deltaSekunden, QSize groesse);
    [[nodiscard]] Mischer& mischer() { return m_mischer; }

    /// Alle Renderer wegwerfen (etwa bevor der GL-Kontext geht).
    void entladeAlles();

    // --- Auskunft (Panel, Tests) -------------------------------------------
    [[nodiscard]] int rendererAnzahl() const { return static_cast<int>(m_laeufe.size()); }
    /// Clips, die im letzten Bild gemischt wurden.
    [[nodiscard]] int gemischteClips() const { return m_gemischt; }
    /// Hat der Clip gerade einen Renderer (laufend oder vorgewaermt)?
    [[nodiscard]] bool hatRenderer(int kanal, int clip) const
    {
        return m_laeufe.count({kanal, clip}) != 0;
    }
    /// Lag die Karaoke-Ebene im letzten Bild ueber dem Gemischten?
    [[nodiscard]] bool karaokeAufgelegt() const { return m_karaokeAufgelegt; }
    /// Was beim Anlegen der Renderer schiefging (fehlende / unlesbare Dateien).
    [[nodiscard]] const QStringList& bericht() const { return m_bericht; }

    /// Jeden vorhandenen Renderer besuchen — etwa um Audiodaten zu reichen.
    void fuerJedenRenderer(const std::function<void(MultiEffectVisualizer&)>& tu);

private:
    struct Lauf
    {
        std::unique_ptr<MultiEffectVisualizer> renderer;
        Leinwand leinwand;
        bool defekt = false;  ///< Datei liess sich nicht laden — nicht jedes Bild neu versuchen
    };
    using Schluessel = std::pair<int, int>;  ///< (Kanal, Clip)

    Lauf& sichere(const Schluessel& s, const VisualClip& clip, QSize groesse);
    static void wirfWeg(Lauf& lauf);

    Komposition m_komposition;
    Mischer m_mischer;
    std::map<Schluessel, Lauf> m_laeufe;
    std::unique_ptr<MultiEffectVisualizer> m_karaokeTraeger;
    Leinwand m_karaokeLeinwand;
    std::int64_t m_vorlaufMs = 2000;
    int m_gemischt = 0;
    bool m_karaokeAufgelegt = false;
    QStringList m_bericht;
};

}  // namespace lumi::composer
