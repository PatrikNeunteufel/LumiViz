/**
 ****************************************************************************************
 * @file   MultiEffectVisualizer.hpp
 * @brief  Multi-effect chain host — renders an AVS-style effect chain
 *         (Import Roadmap 5.1 skeleton + 5.2 blend engine/nesting)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.2.0
 *
 * @details
 * Hosts a lumi::multieffect::ChainNode tree and renders it after the AVS model
 * (analysis doc §5.1): a persistent ping-pong FBO pair as working surface —
 * render effects draw in place on the current buffer, transform effects read
 * current → write partner → swap. The frame result is blitted to the window's
 * framebuffer at the end.
 *
 * 5.2: non-root lists own a persistent buffer pair (AVS thisfb) — the parent
 * image is blended in (in-blend), the children render on the list surface, the
 * result is blended back (out-blend). Blend batch 1 per decision E3; the rest
 * falls back to Replace (compile warning). OnBeat activation and the EEL list
 * slot pair (init/frame → enabled/clear/beat/alphain/alphaout) run per list;
 * all script hosts share one preset-local ScriptContext (decision §10.3).
 *
 * Threading contract (Visualizer_Architecture §12): all GL objects live on the
 * render thread; GUI-side chain edits must hold the widget's renderMutex() and
 * run compileChain() afterwards (decision E4/E5).
 ****************************************************************************************
 */

#pragma once

#include "visualizers/VisualizerBase.hpp"
#include "visualizers/MilkdropVisualizer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"
#include "visualizers/multieffect/TextTags.hpp"
#include "visualizers/modules/processing/BeatEstimator.hpp"
#include "visualizers/modules/processing/BeatModule.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"
#include "visualizers/modules/scripting/ScriptGridModule.hpp"
#include "visualizers/modules/scripting/ScriptLutModule.hpp"
#include "visualizers/render/OffscreenBufferPool.hpp"
#include "visualizers/render/ScopeRenderer.hpp"
#include "scripting/ScriptContext.hpp"
#include "scripting/ScriptSlotHost.hpp"
#include "services/VideoFrameCache.hpp"

#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * @class MultiEffectVisualizer
 * @brief IVisualizer hosting an editable multi-effect chain (AVS import target)
 */
class MultiEffectVisualizer : public VisualizerBase
{
public:
    MultiEffectVisualizer();
    ~MultiEffectVisualizer() override = default;

    // =========================================================================
    // Chain access — GUI writes only under the widget's renderMutex() (§12)
    // =========================================================================

    /** Replace the whole chain (runs the compile pass; returns its result). */
    lumi::multieffect::CompileResult setChain(lumi::multieffect::ChainNode root);

    /** Mutable chain root — caller must hold renderMutex() and re-compile. */
    [[nodiscard]] lumi::multieffect::ChainNode& chain() { return m_root; }
    [[nodiscard]] const lumi::multieffect::ChainNode& chain() const { return m_root; }

    /** Re-run the chain compile pass after in-place edits (decision E4). */
    lumi::multieffect::CompileResult recompileChain();

    /**
     * Ungespeicherte Aenderungen (S89): die Kette gilt als geaendert, wenn sie
     * anders gespeichert wuerde als zum gemerkten Stand. Verglichen wird das
     * Gespeicherte selbst — ein Feld hin und zurueck ist keine Aenderung, und
     * was nur zur Laufzeit lebt (Skriptwerte, Wahl des Host Switchers,
     * Knoten-Ids) zaehlt nicht. Gemerkt wird von selbst nach `setChain` und
     * `loadChainFile`; nach dem SPEICHERN ruft der Aufrufer
     * `merkeKettenStand()` (saveChainFile ist const). Alles unter dem
     * Render-Mutex.
     */
    void merkeKettenStand();
    [[nodiscard]] bool ketteGeaendert() const;
    /// Billige Fassung fuer eine Anzeige, die oft fragt: vergleicht nur nach
    /// einem Panel-Edit neu, sonst gilt das letzte Ergebnis.
    [[nodiscard]] bool ketteVermutlichGeaendert() const;
    /// Dasselbe fuer die Karaoke-Ebene. Gemerkt wird nach dem Laden eines
    /// Karaoke-Presets, nach `setzeKaraokeVorgabe` und im Konstruktor; nach
    /// dem Speichern unter einem Namen ruft der Aufrufer `merkeKaraokeStand()`.
    /// Das selbsttaetige Schreiben der Ablage zaehlt NICHT als Speichern.
    void merkeKaraokeStand();
    [[nodiscard]] bool karaokeGeaendert() const;

    /**
     * Letzter GL-Kompilierfehler eines Shadertoy- ODER Mesh-Warp-Nodes
     * ("" = fehlerfrei; beide Typen teilen das stError-Feld der LeafRuntime —
     * ein Knoten ist genau ein Typ). Der Panel-Editor zeigt ihn beim
     * (Neu-)Aufbau an und pollt ihn nach Apply (S69) — dank `#line 1` im
     * Wrapper tragen die Treiber-Logs die ZEILEN DES NUTZER-CODES.
     */
    [[nodiscard]] std::string shadertoyError(uint64_t nodeId) const
    {
        const auto it = m_leafRuntimes.find(nodeId);
        return it != m_leafRuntimes.end() ? it->second.stError : std::string();
    }

    /**
     * @brief Parse an .avs file, translate it into the chain, and install it.
     * @param path Absolute path to a Nullsoft AVS preset.
     * @param outReport Optional: receives parser + translation warnings.
     * @return true if the file parsed as an AVS preset (unsupported effects are
     *         conserved as passthrough, not a failure).
     *
     * GUI-thread call — the caller must hold the widget's renderMutex() (the
     * render thread walks the chain). The full File-menu/editor hookup is 5.7.
     */
    bool loadAvsFile(const QString& path, QStringList* outReport = nullptr);

    /**
     * @brief Parse a .milk preset and install it as a single-Milkdrop-node
     *        chain (N2 routing — Entscheid E1/E2). GUI thread, under
     *        renderMutex(); no GL (the render thread applies revisions lazily).
     * @return false only when the file does not parse as a MilkDrop preset.
     */
    bool loadMilkFile(const QString& path, QStringList* outReport = nullptr);
    /// S63: .milk→.milk-Wechsel ohne Runtime-Reset — bestehende Milkdrop-Wurzel
    /// bekommt nur neue Params (Revision++), die Feedback-Historie bleibt
    /// erhalten (Original-Semantik: ein Preset erbt das Bild des Vorgaengers)
    bool replaceMilkdropPresetInPlace(lumi::milkdrop::PresetState& state,
                                      const QString& presetDir,
                                      std::map<std::string, std::string> embeddedImages);

    /**
     * @brief Load a translated milkdrop .lvfx sister document (M6.1 format)
     *        and install it as a single-Milkdrop-node chain. GUI thread,
     *        under renderMutex().
     */
    bool loadMilkDocument(const QString& path, QStringList* outReport = nullptr);

    /** Save the current chain as a host preset (.lvfx JSON). */
    bool saveChainFile(const QString& path) const;

    /** Load a host preset (.lvfx). GUI-thread call — hold renderMutex() (5.7 UI). */
    bool loadChainFile(const QString& path, QStringList* outReport = nullptr);

    /**
     * @brief Preset-Datei als Knotenbaum bauen, OHNE die laufende Kette
     *        anzufassen (S82, „Preset as Group"): .avs, .milk, .lvfx und die
     *        Milkdrop-Schwester-.lvfx — dieselben Uebersetzer und Pfad-
     *        Aufloesungen wie die load*-Funktionen oben, aber kein
     *        Render-Scale-Knoten (der wirkt global, Entwurf §5) und kein
     *        Runtime-Reset. Die Wurzel ist immer eine ListParams-Liste.
     *        Reine Funktion, kein Lock noetig.
     * @param outReport Probleme; Zeilen mit „ℹ" sind Bestaetigungen.
     * @return false = Datei unlesbar/unbekannt (outRoot unveraendert).
     */
    [[nodiscard]] static bool buildPresetTree(const QString& path,
                                              lumi::multieffect::ChainNode& outRoot,
                                              QStringList* outReport = nullptr);

    /**
     * @brief Inhalt einer Host-Gruppe aus einem buildPresetTree-Baum (S82):
     *        alle nodeIds auf 0 (compileChain vergibt neue). Traegt die Wurzel
     *        eigene Einstellungen (Clear, Blend, OnBeat, Listen-Code — bei AVS
     *        etwa „clear every frame"), bleibt sie als EINE innere Effect List
     *        erhalten; sonst werden ihre Kinder direkt zum Gruppeninhalt.
     *        Vorher (Knopf „.lvfx in diese Gruppe importieren") gingen diese
     *        Einstellungen still verloren.
     */
    [[nodiscard]] static std::vector<lumi::multieffect::ChainNode>
    presetGroupChildren(lumi::multieffect::ChainNode root);

    /**
     * @brief Presetverweise (S82) im Teilbaum aufloesen: jede Host-Gruppe mit
     *        `verweis` bekommt ihren Inhalt frisch aus `sourceFile` (relativ =
     *        gegen `baseDir`). Laeuft beim Laden jeder .lvfx von selbst; der
     *        Editor ruft es nach Pfadwechsel / „Neu laden". Reine Funktion.
     * @param report Nicht ladbare Verweise, Zyklen, Probleme der Quelle.
     */
    static void resolvePresetReferences(lumi::multieffect::ChainNode& node,
                                        const QString& baseDir, QStringList* report);

    /**
     * @brief Divisor des Render-Scale-Knotens, den loadAvsFile automatisch als
     *        erstes Kind einfuegt (Entscheid S47, Variante 2): Der Wert kommt
     *        aus der App-Einstellung und wirkt NUR im Moment des Imports —
     *        danach ist der Knoten im Preset die einzige Wahrheit. 1 = neutral.
     */
    void setImportRenderScaleDivisor(int divisor)
    {
        m_importRenderScaleDivisor = divisor < 1 ? 1 : (divisor > 8 ? 8 : divisor);
    }

    /**
     * @brief App-Default fuer den Puffer-Wechsel beim .milk→.milk-Tausch
     *        (S66, Settings-Panel): greift, wenn der Milkdrop-Node auf
     *        PufferWechsel::AppEinstellung steht. Wie beim Render-Scale vor
     *        jedem loadMilk*-Aufruf unter renderMutex() setzen.
     * @param modus  Behalten/Loeschen/Fading (AppEinstellung ist hier ungueltig
     *               und faellt auf Behalten zurueck)
     * @param fading Erbe-Anteil 0..1 fuer den Modus Fading
     */
    void setMilkdropPufferWechselDefault(lumi::multieffect::PufferWechsel modus,
                                         double fading, double ausblendSek = 2.0)
    {
        using lumi::multieffect::PufferWechsel;
        m_milkPufferWechselDefault =
            modus == PufferWechsel::AppEinstellung ? PufferWechsel::Behalten : modus;
        m_milkPufferFadingDefault =
            fading < 0.0 ? 0.0 : (fading > 1.0 ? 1.0 : fading);
        m_milkPufferAusblendSekDefault =
            ausblendSek < 0.1 ? 0.1 : (ausblendSek > 60.0 ? 60.0 : ausblendSek);
    }

    /**
     * @brief Sicht-Blende (S67, Settings-Panel `milkdrop/sichtBlende`): an ⇒
     *        Milkdrop-Nodes blenden nach frischer Rausch-Saat (Kaltstart/
     *        Resize/Loeschen-Wipe) ~0,5 s von Schwarz ein (rein kosmetisch).
     *        Wie die anderen Milkdrop-Defaults unter renderMutex() setzen.
     */
    void setMilkdropSichtBlende(bool an) { m_milkSichtBlende = an; }

    /**
     * @brief Erzwingt deterministisch alle N Frames einen Beat statt des
     *        Detektors (0 = Detektor) — Gegenstueck zu AvsRef --beat-period
     *        fuer frame-exakte Diffs von History-Presets (S46-Merkregel:
     *        Beat-Divergenz laesst sie nach ~100 Frames auseinanderlaufen).
     *        Frame-Zaehlung startet je geladenem Preset bei 0 (Beat auf 0, N, …).
     */
    void setBeatPeriodOverride(int frames)
    {
        m_beatPeriodOverride = frames > 0 ? frames : 0;
        m_beatPeriodFrame = 0;
    }

    /**
     * @brief Beat-SPUR statt fester Periode (S74) — Geschwister von
     *        setBeatPeriodOverride() fuer das musik-abgeleitete Pruefsignal.
     *
     * Eine feste Periode passt nicht zu einem Signal, dessen Dynamik aus einer
     * echten Aufnahme stammt: die Schlaege liegen dort da, wo die Musik sie
     * hat. @p flags ist ein zyklisch gelesenes Feld aus @p laenge Flags (ein
     * Bild je Eintrag, 0/1); der Speicher gehoert dem Aufrufer und muss den
     * Visualizer ueberleben (im Werkzeug ist es statischer Programmspeicher).
     *
     * Schlaegt die Periode; @p flags = nullptr schaltet zurueck.
     */
    void setBeatTrackOverride(const unsigned char* flags, int laenge)
    {
        m_beatTrack = (flags != nullptr && laenge > 0) ? flags : nullptr;
        m_beatTrackLaenge = (m_beatTrack != nullptr) ? laenge : 0;
        m_beatPeriodFrame = 0;
    }

    /**
     * @brief Test-Hook (GL-Gates, S48): Kopie der aktuellen Root-Surface als
     *        Bild — nur mit current GL-Context aufrufen (nach render());
     *        leer, solange noch kein Frame gerendert wurde. Liest die interne
     *        Surface VOR dem Present (unabhaengig vom Fenster-Framebuffer).
     */
    [[nodiscard]] QImage debugGrabRootSurface() const;
    /// Test-Naht (S88): gibt es fuer den Knoten eine Laufzeit? / wie viele gibt es?
    [[nodiscard]] bool debugHatLaufzeit(uint64_t nodeId) const
    {
        return m_leafRuntimes.count(nodeId) != 0;
    }
    [[nodiscard]] std::size_t debugLaufzeitAnzahl() const { return m_leafRuntimes.size(); }

    // ---- Composer-Naht (docs/ui/Composer_Konzept.md §12.2) -----------------
    // Der Composer laesst mehrere Renderer nebeneinander laufen und mischt
    // ihre fertigen Bilder. Dafuer braucht er drei Dinge vom Renderer — keins
    // davon beruehrt die Bildberechnung, und die Vorgabe ist jeweils das
    // Verhalten von vor dem Composer.

    /// Composer-Naht R1: wohin der Present zeichnet. 0 (Vorgabe) = das
    /// Fenster, wie immer. Sonst die Nummer eines Framebuffer-Objekts des
    /// Aufrufers: der Renderer legt sein fertiges Bild dort ab — samt Render
    /// Scale und Post-Bloom, in der Groesse des gesetzten Viewports.
    void setPresentZiel(unsigned int framebuffer) { m_presentZiel = framebuffer; }
    [[nodiscard]] unsigned int presentZiel() const { return m_presentZiel; }
    /// Composer-Naht R2: diese Instanz laesst die Karaoke-Ebene aus (Vorgabe:
    /// nein). Der Schalter der Ebene ist fuer alle Renderer gemeinsam; laufen
    /// mehrere, zeichnete sonst jeder den Songtext ueber sein Bild (C0b).
    void setKaraokeAuslassen(bool auslassen) { m_karaokeAuslassen = auslassen; }
    [[nodiscard]] bool karaokeAuslassen() const { return m_karaokeAuslassen; }
    /// Composer-Naht R2: die in diesem Frame gerechnete Karaoke-Ebene auf den
    /// GEBUNDENEN Framebuffer legen — damit ein Mischer sie einmal ueber das
    /// gemischte Bild zeichnen kann. Tut nichts, wenn die Ebene in diesem
    /// Frame nicht dran war. Der Renderer selbst ruft dasselbe am Ende seines
    /// Presents.
    void legeKaraokeEbeneAuf() { legeKaraokeAuf(); }
    /// Composer-Naht R2 (waechst mit N16, S101): zaehlt jede Aenderung der
    /// Karaoke-Ebene — jedes `recompileKaraoke()`. Der Composer liest daran
    /// ab, ob er die Ebene seinem Karaoke-Traeger neu einsetzen muss. Unter
    /// dem Render-Mutex lesen.
    [[nodiscard]] std::uint64_t karaokeFassung() const { return m_karaokeFassung; }

    // ---- Movement-Tabelle in Scheiben (S94) --------------------------------
    // Der Knoten Movement wertet sein Skript je PIXEL aus und legt eine
    // Tabelle an — in Full HD rund eine halbe Sekunde, in der das Bild steht.
    // Im Scheiben-Betrieb entsteht dieselbe Tabelle ueber mehrere Bilder
    // verteilt; bis sie fertig ist, zeichnet der Knoten ueber das 96 x 72-
    // Gitter. Fertige Tabellen teilen sich alle Renderer des Prozesses —
    // ausser das Skript liest den gemeinsamen Zustand der Kette, den
    // Zufallsstrom, Audio oder die Uhr: dann baut es zwar auch in Scheiben
    // (Entscheid Patrik S94), liest diese Werte aber ueber mehrere Bilder
    // verteilt, und seine Tabelle bleibt bei diesem einen Renderer.

    /// Zeitscheibe je Bild und Knoten in Millisekunden. **0 (Vorgabe) =
    /// sofort, in einem Stueck, wie immer** — so laufen die Standalones, der
    /// Vergleichslauf und die Unit-Tests.
    static void setMovementScheibeMs(double ms);
    [[nodiscard]] static double movementScheibeMs();
    /// Die Scheibe der App, wenn in den Einstellungen „verteilt" gewaehlt ist.
    static constexpr double kMovementScheibeAppMs = 4.0;
    /// QSettings-Schluessel der Einstellung Performance → Movement Table:
    /// false (Vorgabe der App) = ueber mehrere Bilder verteilt, true = in
    /// einem Stueck wie das AVS-Original.
    static constexpr const char* kMovementEinstellung = "performance/movementTableAtOnce";
    /// Entsteht in dieser Instanz noch eine Movement-Tabelle? (Stand des
    /// letzten Bilds — der Composer waermt einen Clip so lange weiter vor.)
    [[nodiscard]] bool movementTabelleImBau() const { return m_movementImBau > 0; }
    /// Der gemeinsame Speicher fertiger Tabellen (Test, Aufraeumen).
    static void leereMovementSpeicher();
    [[nodiscard]] static std::size_t movementTabellenImSpeicher();

    /**
     * @brief Songwechsel melden (S82) — Host Switcher (Song-Slot, „bei
     *        Songwechsel weiter"). Ein prozessweiter Zaehler statt eines Flags
     *        je Instanz: jeder Visualizer sieht den Wechsel genau einmal, egal
     *        wie viele Fenster laufen. Thread-sicher, kein Lock noetig.
     *        Allgemein gehalten: weitere Module koennen denselben Zaehler lesen.
     */
    static void notifySongChanged();
    /// S86: Songwechsel MIT den Tags des neuen Titels — der Text-Knoten setzt
    /// sie in Vorlagen wie `[{artist} – ]{title}` ein (TextTags.hpp). Die Tags
    /// bleiben stehen, bis der naechste Titel sie ersetzt; ein spaeter
    /// angelegter Visualizer sieht sie ebenfalls. Thread-sicher.
    static void notifySongChanged(const lumi::multieffect::texttags::SongTags& tags);

    /// S86 (Lyrics-Knoten): der Songtext des laufenden Titels. Gesetzt beim
    /// Songwechsel (nach der Suche, `audio/SongtextQuelle`), spaeter auch
    /// von Werkzeugen (Online-Abruf, Nachsynchronisieren). Thread-sicher.
    static void setSongtext(const lumi::multieffect::lyrics::Songtext& text);
    /// Angaben zum gesetzten Songtext fuer die Oberflaeche (S87: als Tabelle
    /// statt einer langen Zeile — Wunsch Patrik S86).
    struct SongtextAngaben
    {
        bool vorhanden = false;  ///< es gibt einen Text
        bool synchron = false;
        int zeilen = 0;
        std::string quelle;    ///< "embedded", "user folder: x.lrc", "LRCLIB, saved: x.lrc" …
        std::string herkunft;  ///< Vermerk aus der Datei ("LRCLIB id 7, fetched …"), leer = keiner
        std::string datei;     ///< absoluter Pfad, leer = eingebettet / nicht gespeichert
        lumi::multieffect::lyrics::Abruf abruf = lumi::multieffect::lyrics::Abruf::Ruhe;
    };
    [[nodiscard]] static SongtextAngaben songtextAngaben();
    /// S88 (Entscheid Patrik a): Versatz in ms, der nur fuer den LAUFENDEN
    /// Titel gilt — zusaetzlich zum Feld `offsetMs` des Knotens und zu
    /// `[offset:]` der Datei, gleicher Sinn (positiv = Text frueher). Die
    /// Ablage ist `audio::songtextTitelVersatzMs` (`offsets.json`); das
    /// Fenster setzt den Wert beim Songwechsel, die Oberflaeche beim
    /// Verstellen. Thread-sicher.
    static void setSongtextTitelVersatz(std::int64_t ms);
    [[nodiscard]] static std::int64_t songtextTitelVersatzMs();
    /**
     * Karaoke-Ebene (S88, KE1 — docs/visuals/Karaoke_Ebene_Konzept.md): der
     * Songtext des laufenden Titels ueber JEDEM Preset, ohne Lyrics-Knoten in
     * der Kette. Die Ebene rechnet in eigene Texturen und wird erst beim
     * Present aufgelegt — die Root-Surface (das Feedback des naechsten Frames)
     * bleibt unberuehrt. Enthaelt die laufende Kette selbst einen
     * eingeschalteten Lyrics-Knoten, tritt die Ebene zurueck („die Kette
     * gewinnt"). Prozessweiter Schalter, thread-sicher; Vorgabe aus.
     */
    static void setKaraokeEbene(bool an);
    [[nodiscard]] static bool karaokeEbene();
    /// Host Switcher (S89): die Puffer der ruhenden Gruppen dieses Switchers
    /// leeren — sie beginnen beim naechsten Betreten ganz frisch. Unter dem
    /// Render-Mutex aufrufen; gewirkt wird im naechsten Frame. Im Skript des
    /// Switchers tut `clearbuffers = 1` dasselbe.
    void leereSwitcherPuffer(uint64_t switcherNodeId) { m_pufferLeeren.insert(switcherNodeId); }

    /// „Die Kette gewinnt": enthaelt die laufende Kette einen eingeschalteten
    /// Lyrics-Knoten (er selbst und alle Container ueber ihm an, Host-Gruppen
    /// nach der Wahl des Switchers)? Dann zeichnet die Ebene nichts. Unter dem
    /// Render-Mutex aufrufen (liest die Kette) — das Karaoke-Panel zeigt damit
    /// „Preset shows its own lyrics" (KE4).
    [[nodiscard]] bool ketteZeigtLyrics() const;
    /**
     * KE2: der Baum der Ebene — eine Wurzel mit Kind 0 = Lyrics-Knoten und
     * Kind 1 = Trail-Liste. Bearbeitet wie die Kette: unter dem Render-Mutex
     * aendern, danach `recompileKaraoke()`. Dateien tragen das Kettenformat
     * (Endung `.lvkar`, Entscheid Patrik S89); geladen wird nur, was die Form
     * Lyrics + Liste hat — sonst bleibt die Ebene, wie sie ist.
     */
    [[nodiscard]] lumi::multieffect::ChainNode& karaokeKette() { return m_karaokeWurzel; }
    [[nodiscard]] const lumi::multieffect::ChainNode& karaokeKette() const { return m_karaokeWurzel; }
    lumi::multieffect::CompileResult recompileKaraoke();
    void setzeKaraokeVorgabe();  ///< zurueck auf das mitgelieferte Aussehen (wie KE1)
    [[nodiscard]] bool ladeKaraokeDatei(const QString& pfad, QStringList* report = nullptr);
    [[nodiscard]] bool speichereKaraokeDatei(const QString& pfad) const;
    /// S92 (Trail-Presets): nur die Trail-Liste der Ebene ersetzen — der
    /// Lyrics-Knoten bleibt, ebenso ob der Trail an ist. `liste` wie aus
    /// `trailpresets::load`. Unter dem Render-Mutex. false = keine Effect List.
    bool setzeKaraokeTrailListe(lumi::multieffect::ChainNode liste);
    /// Ablage der laufenden Ebene (`<Benutzerdaten>/karaoke/current.lvkar`):
    /// das Fenster setzt sie beim Start; jede danach angelegte Instanz laedt
    /// die Datei im Konstruktor, wenn es sie gibt. Leer (Vorgabe, Tests) =
    /// nichts laden. Thread-sicher.
    static void setKaraokeAblage(const QString& datei);
    [[nodiscard]] static QString karaokeAblage();
    /**
     * Mischart der Karaoke-Ebene (K6, Entscheid Patrik a): je Teil — laufende
     * Zeile und Trail — wie sich die Ebene mit dem Bild dahinter mischt, und
     * wie stark (0..100 %). Arten (Nummern = Werte der Einstellung):
     * 0 Normal (deckend mit Alpha) · 1 Additive · 2 50/50 · 3 Subtract ·
     * 4 Invert (weisser Text kehrt die Farbe darunter um) · 5 XOR ·
     * 6 Multiply · 7 Maximum · 8 Minimum. 0–2 wie das Feld Blend des
     * Lyrics-Knotens. Bei der Zeile bleibt die Kontur immer normal — nur die
     * Fuellung mischt. Vorgabe: Zeile Normal, Trail Additive, je 100 %.
     *
     * Seit S89 (KE2) stehen Mischart und Staerke in den KNOTEN der Ebene
     * (Felder `blend` / `blendStrength` von Lyrics und Lyrics Trail) und
     * werden dort bearbeitet und gespeichert. `setKaraokeMischung` setzt nur
     * noch die Werte, mit denen die mitgelieferte Ebene AUFGEBAUT wird
     * (Konstruktor, `setzeKaraokeVorgabe`) — fuer Tests und die Uebernahme.
     */
    enum class KaraokeTeil { Zeile = 0, Trail = 1 };
    static constexpr int kKaraokeMischarten = 9;
    static void setKaraokeMischung(KaraokeTeil teil, int mischart, int staerkeProzent);
    [[nodiscard]] static int karaokeMischart(KaraokeTeil teil);
    [[nodiscard]] static int karaokeStaerke(KaraokeTeil teil);
    /// Einmalige Uebernahme der Einstellungen bis S88 (QSettings
    /// `karaoke/LineBlend|LineStrength|TrailBlend|TrailStrength`): gibt es
    /// noch keine Ablage, wird sie mit diesen Werten geschrieben; die
    /// Schluessel verschwinden in jedem Fall. true = Ablage geschrieben.
    static bool uebernehmeAlteKaraokeMischung();
    /// Test-Naht: das Bild, wie es am Fenster steht (Root-Surface + Karaoke-
    /// Ebene), in der Fenstergroesse des letzten Frames (K4 b: die Zeile der
    /// Ebene ist dort gerastert; ohne Render Scale gleich der Kettenaufloesung).
    /// `debugGrabRootSurface` sieht die Ebene nie.
    [[nodiscard]] QImage debugGrabMitKaraoke();
    /// Test-Naht: hat die Ebene im letzten Frame gezeichnet (nicht zurueckgetreten)?
    [[nodiscard]] bool debugKaraokeGezeichnet() const { return m_karaokeGezeichnet; }
    /// Kopie des gesetzten Songtexts (S87: Vergleich im Alternativen-Dialog).
    [[nodiscard]] static lumi::multieffect::lyrics::Songtext songtext();
    /// Stand des Online-Abrufs fuer den laufenden Titel (S87) — das Fenster
    /// reicht ihn von `audio/SongtextOnline` durch. Thread-sicher.
    static void setSongtextAbruf(lumi::multieffect::lyrics::Abruf stand);
    /// Absoluter Pfad der Datei des gesetzten Songtexts; leer = eingebettet,
    /// nicht gespeichert oder kein Text.
    [[nodiscard]] static std::string songtextDatei();
    /// Abspielposition des Players in ms (S86): der Player meldet sie
    /// 10–30-mal je Sekunde, dazwischen wird auf der Wanduhr weitergezaehlt,
    /// solange er laeuft (`notifyPlayState`). Thread-sicher.
    static void notifyPlayPosition(std::int64_t positionMs);
    static void notifyPlayState(bool laeuft);
    /// Abspieltempo (S91, Lyrics Sync): 1.0 = wie aufgenommen, 0.5 = halb so
    /// schnell. Die Hochrechnung zwischen zwei Meldungen zaehlt die Wanduhr
    /// mit diesem Faktor — sonst liefe sie dem Player davon.
    static void notifyPlayTempo(double tempo);
    [[nodiscard]] static std::int64_t playPositionMs();
    /// Test-Naht (S91): die Wanduhr der Hochrechnung festsetzen (ms);
    /// negativ = wieder die echte Uhr.
    static void debugSetzeWanduhrMs(std::int64_t ms);

    /**
     * @brief Test-Hook (S82): laufende Gruppe eines Host Switchers (Index
     *        1..count), 0 = noch nicht gelaufen / unbekannt.
     */
    [[nodiscard]] int debugSwitcherIndex(uint64_t nodeId) const;

    /** @brief Test-Hook (S87): letzter Skriptfehler eines Host Switchers, leer = keiner. */
    [[nodiscard]] std::string debugSwitcherError(uint64_t nodeId) const;

    /**
     * @brief Laeuft eine Host-Gruppe gerade (S82)? Das persistierte `enabled`,
     *        solange kein Host Switcher sie fuehrt — sonst dessen Wahl.
     */
    [[nodiscard]] bool effectiveEnabled(const lumi::multieffect::ChainNode& node) const;

protected:
    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    // =========================================================================
    // Surfaces
    // =========================================================================

    /** One ping-pong FBO pair (root working surface or a list's thisfb). */
    struct SurfacePair
    {
        std::unique_ptr<QOpenGLFramebufferObject> fbo[2];
        int currentIndex = 0;

        [[nodiscard]] QOpenGLFramebufferObject* current() const
        {
            return fbo[currentIndex].get();
        }
        [[nodiscard]] QOpenGLFramebufferObject* partner() const
        {
            return fbo[1 - currentIndex].get();
        }
        [[nodiscard]] bool ready() const { return fbo[0] && fbo[1]; }
        void swap() { currentIndex = 1 - currentIndex; }
        void destroy()
        {
            fbo[0].reset();
            fbo[1].reset();
            currentIndex = 0;
        }
    };

    /** One FyrewurX spark (NDC position/velocity, remaining life). */
    struct FwSpark
    {
        float x = 0.0f, y = 0.0f;
        float vx = 0.0f, vy = 0.0f;
        float life = 0.0f;     ///< seconds remaining
        float lifeMax = 1.0f;  ///< spawn lifetime (for the fade)
        float r = 1.0f, g = 1.0f, b = 1.0f;
    };

    /** One Starfield particle (normalized coords, z in (0,1]). */
    /// Ein Stern wie r_stars.cpp: X/Y sind GANZZAHLIGE Pixel-Abstaende zur
    /// Bildmitte, Z die Tiefe, Speed der Einzeltempo-Faktor.
    struct Star
    {
        int x = 0;
        int y = 0;
        float z = 1.0f;
        float speed = 0.5f;
    };

    /** One Dot-Fountain particle (polar around the fountain axis). */
    /// Ein Punkt der Dot-Fountain-Hoehenwand (`FountainPoint`, r_dotfnt.cpp:46).
    /// Das Feld ist ein 30x256-GITTER, kein Partikelschwarm: 30 Speichen mal
    /// 256 Alterungsstufen. Je Frame rutscht jede Stufe eine weiter nach hinten
    /// und bekommt dabei ihre Physik; Stufe 0 wird aus dem Spektrum neu gesetzt.
    struct FountainP
    {
        float r = 0.0f;   ///< Radius von der Achse
        float dr = 0.0f;  ///< Radius-Zuwachs je Frame
        float h = 0.0f;   ///< Hoehe
        float dh = 0.0f;  ///< Hoehen-Zuwachs je Frame
        float ax = 0.0f;  ///< sin(Speichenwinkel) — einmal beim Erzeugen
        float ay = 0.0f;  ///< cos(Speichenwinkel)
        uint32_t c = 0;   ///< Farbe aus der 64er-Tabelle
    };

    /// S90, LB1: ein gerastertes Glyphen-Bild mit seinem Platz im Bild. Das
    /// Rechteck steht in Anteilen der Bildgroesse, y von oben — ein Streifen
    /// statt des ganzen Bilds, damit haeufiges Neu-Rastern billig bleibt.
    struct GlyphBild
    {
        unsigned int tex = 0;  ///< GL RGBA (in gibLeafLaufzeitFrei geloescht)
        std::string stand;     ///< Zustandsschluessel hinter der Textur
        float x = 0.0f;
        float y = 0.0f;
        float b = 1.0f;
        float h = 1.0f;
        bool da = false;       ///< traegt das Bild der laufenden Zeile dieses Frames
        /// S90, B2 a: auf dem Masken-Weg wird das Bild auf der Grafikkarte
        /// zusammengesetzt — dann lebt es in dieser Flaeche, vormultipliziert.
        std::unique_ptr<QOpenGLFramebufferObject> fbo;
        unsigned int zeig = 0;  ///< die Textur, die aufzulegen ist (`tex` oder die der Flaeche)
        bool vormult = false;   ///< Farbe ist mit der Deckung vormultipliziert (Flaeche)
    };
    /// S90, B2 a: ein Wort (oder ein Zeilenstueck) als Maske — einmal gerastert,
    /// danach sind Groesse, Lage und Farbe Parameter der Grafikkarte. R traegt
    /// die Deckung der Fuellung, G die des Kontur-Strichs. Die Masse stehen in
    /// Pixeln der Bezugsgroesse (`lyricsMaskenRef`), relativ zum Ursprung auf
    /// der Grundlinie.
    struct WortMaske
    {
        unsigned int tex = 0;  ///< 0 = nichts zu zeichnen (nur Leerraum)
        float bx = 0.0f, by = 0.0f, bb = 0.0f, bh = 0.0f;  ///< Rahmen der Maske
        float vor = 0.0f;    ///< Vorschub des ganzen Stuecks
        float mitte = 0.0f;  ///< Mitte der Buchstaben (ohne Leerraum am Rand)
    };

    /** Per-leaf render-thread state (beat counters, scripts), keyed by nodeId. */
    struct LeafRuntime
    {
        int beatCounter = 0;     ///< OnBeat Clear: beats since last clear
        int beatFramesLeft = 0;  ///< Colorfade: frames the beat faders stay on
        /// Colorfade: der laufende Fader-Zustand (`faderpos` in r_colorfade).
        /// Er wandert je Frame um EINEN Schritt auf sein Ziel zu, wenn
        /// `slowFade` gesetzt ist — deshalb muss er ueber Frames leben.
        /// `fadeSeeded` traegt zusaetzlich, ob er schon einmal aus den
        /// Preset-Werten gesetzt wurde (Startwert, s. `interfRotationSeed`).
        int fadePos[3] = {0, 0, 0};
        bool fadeSeeded = false;
        int mirrorRBeat = 0;     ///< Mirror onbeat-random: current direction bits
        float mirrorF[4] = {0.0f, 0.0f, 0.0f, 0.0f};  ///< smooth factors per direction
        int mirrorFrames = 0;    ///< Mirror: frame counter for the `slower` ramp

        // Color Modifier: scripted 256-entry LUT + its GL upload
        std::unique_ptr<lumi::modules::ScriptLutModule> lut;
        unsigned int lutTexture = 0;  ///< GL 256x1 RGB texture (deleted in onCleanup)
        std::string lutCompiled;      ///< source snapshot the module was built from

        // Color Map: 256x1 gradient LUT texture built from the stops (r_colormap)
        unsigned int cmTexture = 0;   ///< GL 256x1 RGB texture (deleted in onCleanup)
        std::string cmSnapshot;       ///< stop snapshot the texture was built from

        // Picture / Texer: decoded image texture (r_picture / Texer / Texer II)
        unsigned int picTexture = 0;  ///< GL RGBA texture (deleted in onCleanup)
        int picW = 0;
        int picH = 0;
        std::string picSnapshot;      ///< Bilddaten hinter der Textur — Wechsel = neu aufbauen

        // Text (r_text): word-cycler state + rendered glyph texture
        int textCurWord = 0;
        int textNf = 0;                ///< frames since last word switch
        int textOddEven = 0;           ///< insertBlank alternator
        int textNb = 0;                ///< onbeat: frames the beat word stays
        float textRandX = 0.0f;        ///< randomPos offset (fraction of width)
        float textRandY = 0.0f;
        std::string textSnapshot;      ///< draw-state snapshot behind textTexture
        unsigned int textTexture = 0;  ///< GL RGBA glyph layer (deleted in onCleanup)
        /// Lyrics mit Mischart (S88, K6): der Kontur-Ring getrennt von der
        /// Fuellung — er liegt immer normal, nur die Fuellung mischt.
        std::string textSnapshot2;
        unsigned int textTexture2 = 0;
        /// Lyrics (S86): Zeile und Beat-Zaehler des unsynchronen Blocks und
        /// der Songtext-Stand, zu dem sie gehoeren (neuer Titel = von vorn).
        int lyricsZeile = 0;
        int lyricsBeats = 0;
        /// S90, LB4: laufendes Wort des unsynchronen Blocks (0 = vor dem
        /// ersten) — gefuehrt, sobald ein `wordtrigger` gefallen ist.
        int lyricsWort = 0;
        bool lyricsWortModus = false;
        uint64_t lyricsStand = 0;
        /// S88, fuer den Lyrics Trail: die Felder des Lyrics-Knotens, wie er
        /// sie zuletzt wirklich gezeichnet hat (NACH seinem Skript — „Karaoke
        /// Bounce" bewegt die Zeile), und beim unsynchronen Block die Zeile vor
        /// dem letzten Wechsel samt dessen Zeitpunkt auf der Sim-Uhr (`m_time`).
        lumi::multieffect::LyricsParams lyricsEffektiv;
        bool lyricsEffektivDa = false;
        /// S89: die Werte, mit denen die ZULETZT BEENDETE Zeile gezeichnet
        /// wurde — nur gefuehrt, wenn der Knoten Line-Code traegt (dann stellt
        /// er die Zeilen womoeglich verschieden, und der Trail braucht die alte).
        lumi::multieffect::LyricsParams lyricsEffektivAlt;
        bool lyricsEffektivAltDa = false;
        int lyricsVorZeile = -1;         ///< -1 = seit dem Titelwechsel kein Zeilenwechsel
        /// S89: Zeilennummer, die das Skript zuletzt gesehen hat (Slot Line);
        /// -1 = noch keine — der erste Frame gilt als Zeilenwechsel
        int lyricsSkriptZeile = -1;
        /// S91, Lyrics Trail: die gegangene Zeile, die sein Skript zuletzt
        /// gesehen hat (Slot Line), und — unsynchron — wann sie ging
        int trailSkriptZeile = 0;
        float trailSkriptWechsel = -1.0f;
        /// S90: Zeile und Wort (Zeile * 100000 + Wort), die das Skript zuletzt
        /// gesehen hat (Slot Word); -1 = noch nichts
        std::int64_t lyricsSkriptWort = -1;
        float lyricsWechselZeit = 0.0f;
        /// S90, LB1 (Zeilen-Puffer): das Bild der LAUFENDEN Zeile, getrennt
        /// von den uebrigen Reihen gerastert — [0] Fuellung bzw. alles,
        /// [1] der Kontur-Ring (nur bei getrennter Kontur). Wechselt die Zeile
        /// nach vorn, wandert es nach `lyricsBildAlt` (Tausch der Texturen,
        /// kein neues Rastern) — das legt der Lyrics Trail auf.
        GlyphBild lyricsBild[2];
        GlyphBild lyricsBildAlt[2];
        /// S90, B2 a (Masken-Weg): die uebrigen Reihen als Flaeche, der
        /// Masken-Vorrat (Schluessel = Gewicht + Text) und wofuer er gilt —
        /// Schrift, Bezugsgroesse in Rasterpixeln, Strichbreite. Aendert sich
        /// der Schluessel, wird der Vorrat geleert und neu gerastert.
        /// S90, B3 a1: Breite der laengsten Zeile des Songtexts bei 100 px
        /// Schrift und wofuer sie gilt (Songtext-Stand, Schrift)
        std::string lyricsSongSchluessel;
        double lyricsSongBreite100 = 0.0;
        GlyphBild lyricsRest[2];
        std::unordered_map<std::string, WortMaske> lyricsMasken;
        std::string lyricsMaskenSchluessel;
        float lyricsMaskenRef = 0.0f;
        float lyricsMaskenHoehe = 0.0f;   ///< Zeilenhoehe der Schrift bei Bezugsgroesse
        float lyricsMaskenAufstieg = 0.0f;  ///< Oberlaenge bei Bezugsgroesse
        /// S90, LB2: laufende Groesse je Wort der laufenden Zeile — sie gleitet
        /// ihrem Ziel nach (Feld wordFlowMs) — und die Zeile, zu der sie gehoert.
        std::vector<float> lyricsWortSkala;
        int lyricsWortZeile = -1;
        float lyricsWortZeit = 0.0f;  ///< Sim-Uhr (`m_time`) des letzten Schritts
        int lyricsBildZeile = -1;  ///< Index der Zeile in `lyricsBild`; -1 = keine
        int lyricsAltZeile = -1;   ///< Index der Zeile in `lyricsBildAlt`; -1 = keine

        // AVI (r_avi): VfW stream handles (opaque void* — vfw.h stays in the .cpp)
        void* aviFile = nullptr;       ///< PAVIFILE
        void* aviStream = nullptr;     ///< PAVISTREAM
        void* aviGetFrame = nullptr;   ///< PGETFRAME
        int aviLength = 0;
        int aviFrameIndex = 0;
        int aviPersistLeft = 0;        ///< beat persist window countdown
        std::int64_t aviLastMs = 0;    ///< last frame advance (speed throttle)
        bool aviTried = false;         ///< open attempted once (no retry spam)
        std::string aviPath;           ///< geoeffneter Pfad — Wechsel = neu oeffnen
        int aviWarnedBpp = 0;          ///< bit depth already reported as unsupported
        unsigned int aviTexture = 0;   ///< GL RGBA frame texture (deleted in onCleanup)
        /// Qt-Multimedia-Fallback (S59): Frame-Cache fuer Codecs, die das
        /// 64-Bit-VfW nicht dekodiert (Indeo & Co.)
        std::shared_ptr<lumi::services::VideoFrameCache::Clip> aviClip;

        // Texer II / Triangle: EEL point-loop scripts
        std::unique_ptr<lumi::scripting::ScriptSlotHost> texerHost;
        std::string texerCompiled;
        std::unique_ptr<lumi::scripting::ScriptSlotHost> triHost;
        std::string triCompiled;

        /// Parameter-Skript eines Knotens (Strang D): init/frame/beat rechnen
        /// die Regler je Frame aus. Nur angelegt, wenn wirklich Code da ist.
        std::unique_ptr<lumi::scripting::ScriptSlotHost> paramHost;
        std::string paramCompiled;
        /**
         * Fortgeschriebener Stand der Skript-Groessen (Entscheid Patrik S54).
         *
         * Ohne ihn konnte der Init-Slot nichts bewirken: die Vorbelegung aus
         * den Params lief VOR jedem Frame und ueberschrieb, was Init einmalig
         * gesetzt hatte. Jetzt belegt nur der erste Frame aus den Params vor —
         * danach traegt diese Tabelle den Wert weiter, und die Frames schreiben
         * ihn fort.
         *
         * `paramSeen` haelt den zuletzt gesehenen PARAM-Wert: dreht der Benutzer
         * am Regler, gewinnt der Regler und die Fortschreibung beginnt neu.
         * Sonst waere jeder Regler tot, sobald ein Skript im Knoten steht.
         */
        std::unordered_map<std::string, double> paramState;
        std::unordered_map<std::string, double> paramSeen;
        /// Letzte Fehlermeldung des Parameter-Skripts ("" = fehlerfrei).
        /// Zugleich die Sperre gegen eine Meldung je Frame (S54).
        std::string paramError;
        /// Block F (F2): Strukturfelder, deren Aenderung aus dem Frame-Slot
        /// schon einmal verworfen und gemeldet wurde (eine Meldung je Feld).
        std::set<std::string> strukturGemeldet;
        /// Block F (F2, E3): zuletzt angenommener Skriptwert eines
        /// Strukturfelds (aus Init/Song/Beat) samt dem Reglerwert, zu dem er
        /// gehoert. Die Frame-Kopie entsteht jeden Frame neu aus den Params —
        /// ohne diesen Speicher fiele das Feld zwischen zwei Beats auf den
        /// Regler zurueck und loeste gerade dadurch Neuaufbauten aus. Dreht
        /// der Benutzer am Regler, gilt wieder der Regler (wie paramSeen).
        struct StrukturWert
        {
            double wert = 0.0;
            double basis = 0.0;
        };
        std::unordered_map<std::string, StrukturWert> strukturWert;
        /// Block F (F2b, S85): zuletzt eingespeister Reglerwert je
        /// Tabellenvariable eines EIGENEN Skript-Hosts (Fraktale, 3D …) —
        /// dieselbe Rolle wie `paramSeen`: bewegt der Benutzer den Regler,
        /// gewinnt der Regler; sonst rechnet das Skript den Wert fort.
        std::unordered_map<std::string, double> feldHostSeen;
        /// SuperScope (S86): `colorCycleFrames` nach den Slots des letzten
        /// Frames (0 = keiner) — das Modul braucht den Wert VOR den Slots,
        /// ein Skriptwert wirkt deshalb einen Frame spaeter.
        int scopeCycleNach = 0;
        /// SuperScope (S86): zuletzt an `n` weitergereichter Skriptwert von
        /// `pointcount` — `n` bleibt der Herr, ein Schreiben auf `pointcount`
        /// wird einmal uebernommen.
        double scopePcWeiter = std::numeric_limits<double>::quiet_NaN();

        /**
         * Bloom: die vom Parameter-Skript gerechneten Werte, damit sie den
         * **Present-Pfad** erreichen.
         *
         * `post` ist per Vorgabe an, und dann kehrt `runBloom` sofort zurueck —
         * der Glow entsteht erst beim Present. Bis S54 lief das Skript in
         * genau diesem Normalfall nie, seine drei Felder standen wirkungslos
         * im Panel (gefunden von den Feld-Sonden). Jetzt rechnet `runBloom`
         * das Skript IMMER und legt das Ergebnis hier ab.
         */
        float bloomIntensity = 1.0f;
        float bloomThreshold = 0.0f;
        int bloomRadius = 8;
        float bloomVigStrength = 0.3f;

        // Movement / Dynamic Movement: scripted displacement grid
        std::unique_ptr<lumi::modules::ScriptGridModule> grid;
        std::string gridCompiled;
        int gridFieldW = 0;  ///< surface size the static field was computed for
        int gridFieldH = 0;
        /// Movement sourcemapped runtime bits (r_trans member state; bit1
        /// toggles bit0 on every beat); -1 = seed from params on first frame
        int moveSourceMapped = -1;
        int moveSourceMappedSeen = -1;  ///< zuletzt uebernommener Preset-Wert
        /// Movement (r_trans): per-Pixel-Tabelle als R32I-Textur — wie im
        /// Original nur bei Groessen-/Skriptwechsel neu gebaut (teuer!)
        unsigned int moveTabTex = 0;  ///< GL R32I w*h (deleted in onCleanup)
        /// Grain (r_grain): depthBuffer als RG8-Textur; die Zufallszuege des
        /// Originals laufen ueber den geteilten Preset-Strom (S49)
        unsigned int grainTex = 0;
        int grainW = 0;
        int grainH = 0;
        bool grainSeeded = false;
        int moveTabW = 0;
        int moveTabH = 0;
        std::string moveTabKey;  ///< Skript + Flags, hinter denen die Tabelle steht
        /// Bau in Scheiben (S94): Schluessel der Tabelle, die gerade entsteht,
        /// und das Ersatz-Modul fuer das Gitter-Bild bis dahin
        std::string moveBauKey;
        std::uint64_t moveBauFrame = 0;  ///< Frame, in dem zuletzt eine Scheibe lief
        std::unique_ptr<lumi::modules::ScriptGridModule> gridErsatz;
        std::string gridErsatzCompiled;

        // SuperScope: scripted point generator (drawn via the shared renderer)
        std::unique_ptr<lumi::modules::SuperscopeModule> scope;
        std::string scopeCompiled;

        // Roto Blitter (r_rotblit, S48): Richtungs- und Zoom-Ease-Zustand —
        // die Rotation selbst akkumuliert uebers FEEDBACK, nicht hier.
        bool rotoSeeded = false;
        float rotoRev = 1.0f;     ///< Zielrichtung (+1/-1; Beat toggelt)
        float rotoRevPos = 1.0f;  ///< geeaster Richtungsfaktor
        int rotoFpos = 31;        ///< scale_fpos (Zoom-Ease)
        // Blitter Feedback (r_blit, S48): fpos-Ease (+-3/Frame Richtung scale)
        bool bfSeeded = false;
        int bfFpos = 30;

        // Custom BPM
        std::int64_t customLastMs = 0;  ///< arbitrary-mode last emit time
        int customSkipCount = 0;        ///< skip-mode beat counter
        int customBeatCount = 0;        ///< alle Beats seit Preset-Start (skipfirst)

        // Mosaic: on-beat quality ease-back (r_mosaic thisQuality/nF)
        float mosaicQuality = 0.0f;  ///< current interpolated block count (0 = init)
        int mosaicFramesLeft = 0;    ///< frames left easing back to `quality`

        // Buffer Save: alternate-direction toggle (r_stack dir_ch)
        bool bufDirCh = false;

        // FyrewurX: live sparks (spawned per beat, gravity-integrated)
        std::vector<FwSpark> fwSparks;

        // SuperScope: gradient preset the module currently holds (reload on change)
        std::string scopeGradientLoaded;

        // Interferences: accumulating rotation + on-beat morph state (r_interf)
        bool interfSeeded = false;
        /// Laufender Drehwinkel in 1/255 Umdrehungen. GANZZAHLIG wie in der
        /// Referenz (`rotation` ist dort ein int, r_interf.cpp:384) — eine
        /// float-Summe laeuft anders auf.
        int interfRotation = 0;
        float interfStatus = 0.0f;    ///< beat-morph phase (0..pi)
        /// Der Preset-Wert, mit dem der Zaehler oben gesetzt wurde. `rotation`
        /// ist ein STARTWERT: nach dem Seeden laeuft `interfRotation`
        /// selbstaendig weiter, und ein Panel-Edit ruft nur `recompileChain()`
        /// — ohne diesen Vergleich kam ein neuer Wert nie an (Strang F, S57).
        int interfRotationSeed = -1;

        // Water: previous frame buffer (r_water lastframe); render-thread owned
        std::unique_ptr<QOpenGLFramebufferObject> waterLast;
        int waterW = 0;
        int waterH = 0;

        // Bump: EEL light-position script + eased depth (r_bump)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> bumpHost;
        std::string bumpCompiled;
        float bumpX = 0.5f;      ///< light position (0..1)
        float bumpY = 0.5f;
        float bumpDepth = 0.0f;  ///< eased strength (0 = uninitialised)
        int bumpFramesLeft = 0;  ///< frames left easing depth back

        // Dynamic Shift: EEL global-offset script (r_shift)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> shiftHost;
        std::string shiftCompiled;

        // Dynamic Distance Modifier: EEL radial-distance script (r_ddm)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> ddmHost;
        std::string ddmCompiled;

        // Jheriko: Global — EEL that sets preset-global reg/gmegabuf
        std::unique_ptr<lumi::scripting::ScriptSlotHost> jherikoHost;
        std::string jherikoCompiled;
        bool jherikoInited = false;

        // Interleave: eased stripe spacing (r_interleave cur_x/cur_y)
        bool interSeeded = false;
        float interCurX = 1.0f;
        float interCurY = 1.0f;

        // Scope-render effects (r_simple/oscstar/oscring/rotstar)
        int scopeColorPos = 0;      ///< colour-table cycle position
        float scopeRot = 0.0f;      ///< accumulating rotation (oscstar/oscring/rotstar)
        float bsRv[2] = {3.14159f, 0.0f};  ///< Bass Spin per-channel angle
        float bsV[2] = {0.0f, 0.0f};       ///< Bass Spin per-channel angular velocity
        /// Bass Spin: last_a ist im Original EIN Member ueber beide Kanaele
        /// (r_bspin.cpp — der linke Kanal beeinflusst das a des rechten; S48)
        int bsLastA = 0;
        int bsLx[2][2] = {{0, 0}, {0, 0}};  ///< letzte Spitze [spoke][kanal] (px)
        int bsLy[2][2] = {{0, 0}, {0, 0}};

        // Moving Particle: spring-particle state (r_parts)
        bool mpSeeded = false;
        float mpCx = 0.0f, mpCy = 0.0f;   ///< spring target (randomized on beat)
        float mpVx = 0.0f, mpVy = 0.0f;   ///< velocity
        float mpPx = 0.0f, mpPy = 0.0f;   ///< position (-1..1)
        float mpSize = 8.0f;              ///< eased radius

        // Bloom (Lights-Etappe 1): kleine Glow-RTs — [0] Downsample/Gauss-Ziel,
        // [1] Gauss-Zwischenpuffer; Groesse = Surface / 2^downsample
        std::unique_ptr<QOpenGLFramebufferObject> bloomRt[2];
        int bloomW = 0;
        int bloomH = 0;

        // 3D Camera (Lights-Etappe 1): EEL-Slots init/frame/beat — duerfen
        // die Kamera-Parameter ueberschreiben (dynamische Modulparameter)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> cam3dHost;
        std::string cam3dCompiled;

        // SuperScope 3D (Lights-Etappe 1): EEL-Quartett (x/y/z/size/rgb)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> scope3dHost;
        std::string scope3dCompiled;

        // Terrain 3D (Lights-Etappe 2): Hoehen-Simulation + Mesh-Grid
        std::unique_ptr<lumi::scripting::ScriptSlotHost> terrainHost;
        std::string terrainCompiled;
        std::vector<float> terrainBase;  ///< h0 (prozedural, fester Seed)
        std::vector<float> terrainH;     ///< aktuelle Hoehen
        std::vector<float> terrainV;     ///< Feder-Geschwindigkeiten
        int terrainRes = 0;              ///< Aufloesung der Puffer (Reset bei Wechsel)
        std::unique_ptr<QOpenGLVertexArrayObject> terrainVao;
        std::unique_ptr<QOpenGLBuffer> terrainVbo;  ///< pos.xyz (je Frame)
        std::unique_ptr<QOpenGLBuffer> terrainIbo;  ///< Quad-Dreiecke (je res)
        int terrainIndexCount = 0;

        // Glow Orbs (Lights-Etappe 2): EEL-Quartett (Point je Orb)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> orbsHost;
        std::string orbsCompiled;

        // Water Bump: RGBA16F height ping-pong (.r current, .g previous height)
        std::unique_ptr<QOpenGLFramebufferObject> wbHeight[2];
        int wbCur = 0;   ///< index of the current height buffer
        int wbW = 0;
        int wbH = 0;

        // Starfield: CPU star particles + warp-speed ease (r_stars)
        std::vector<Star> stars;
        float starSpeed = 0.0f;    ///< CurrentSpeed (r_stars); 0 = uninitialisiert
        int starBeatFrames = 0;    ///< nc — Frames bis WarpSpeed wieder gilt
        float starIncBeat = 0.0f;  ///< incBeat — Schrittweite der Rueckkehr
        int starW = 0;             ///< Groesse, fuer die die Sterne gesetzt wurden
        int starH = 0;

        int timescopeX = 0;        ///< Timescope: current spectrogram column

        // Dot Grid / Plane / Fountain (r_dotgrid/dotpln/dotfnt)
        float dotColorPos = 0.0f;  ///< Dot Grid: colour-table cycle position
        float dotOffX = 0.0f;      ///< Dot Grid: scroll offset
        float dotOffY = 0.0f;
        float dotRot = 0.0f;       ///< Dot Fountain: accumulating rotation
        /// Preset-Startwinkel, mit dem `dotRot` geseedet wurde (Bauart
        /// `interfRotationSeed`: STARTWERT, danach laeuft der Winkel selbst).
        float dotRotSeed = std::numeric_limits<float>::quiet_NaN();
        std::vector<FountainP> fountain;  ///< Dot Fountain particles

        // Dot Plane (r_dotpln, S48-Neuschrieb): scrollendes 64x64-Grid mit
        // Physik; die Injektionszeile traegt Hoehe UND Farbe aus dem Spektrum
        // (color_tab[Byte>>2]) — die Farbe wandert mit der Zeile mit.
        std::vector<float> dpHeights;    ///< atable (64*64)
        std::vector<float> dpVel;        ///< vtable
        std::vector<uint32_t> dpColors;  ///< ctable (FB-Ints)
        float dpR = 0.0f;                ///< akkumulierte Rotation (Grad)
        /// Preset-Startwinkel, mit dem `dpR` geseedet wurde (s. `dotRotSeed`).
        float dpRSeed = std::numeric_limits<float>::quiet_NaN();
        bool dpSeeded = false;

        int apeChanMode = -1;      ///< Channel Shift on-beat held permutation
        int apeChanSeed = -1;      ///< Preset-Wert, aus dem apeChanMode geseedet wurde

        // Fractal 2D (Batch H): EEL view-driver + gradient palette LUT
        std::unique_ptr<lumi::scripting::ScriptSlotHost> fracHost;
        std::string fracCompiled;      ///< source snapshot the host was built from
        bool fracInited = false;       ///< init slot has run once
        unsigned int fracLut = 0;      ///< GL 256x1 RGB palette (deleted in onCleanup)
        std::string fracLutSnapshot;   ///< gradient preset the LUT was baked from
        /// CPU-Farbtabelle der Punktwolken (Strange Attractor, Flame) — nur neu,
        /// wenn sich der Verlauf aendert. Bis S84 entstand sie JEDEN Frame neu,
        /// und jedes Mal las ein frisches ColorGradientModule alle eigenen
        /// Verlaufsdateien von der Platte (Befund S84: ~36 Ladungen je Sekunde).
        std::array<QVector3D, 256> cpuLut{};
        std::string cpuLutSnapshot;
        bool cpuLutGueltig = false;
        float fracColorPhase = 0.0f;   ///< accumulated palette drift (colorCycle)
        float fracTime = 0.0f;         ///< accumulated animation phase (Domain Warp / Zoomer log-zoom)
        float fracRot = 0.0f;          ///< Fractal Zoomer: akkumulierte Rotation (S48: eigenes Feld, war rotoAngle)

        // Strange Attractor / Flame (Batch H): persistent orbit + view rotation
        double saX = 0.1, saY = 0.0, saZ = 0.0;  ///< current orbit position
        float saRot = 0.0f;            ///< accumulated view rotation
        bool saSeeded = false;

        // Reaction-Diffusion (Batch H): RGBA16F ping-pong (A in .r, B in .g)
        std::unique_ptr<QOpenGLFramebufferObject> rdBuf[2];
        int rdCur = 0;
        int rdW = 0;
        int rdH = 0;
        bool rdSeeded = false;

        // Video Delay: per-node frame ring buffer (r_videodelay)
        std::vector<std::unique_ptr<QOpenGLFramebufferObject>> delayRing;
        int delayHead = 0;
        int delayFilled = 0;
        int delayW = 0;
        int delayH = 0;

        // Milkdrop-Meganode (N1, Entscheid E1): der komplette MilkDrop-Kern als
        // Engine je Node; GL-Freigabe via cleanup() in resetRuntimes
        std::unique_ptr<MilkdropVisualizer> milk;
        uint64_t milkRevision = 0;  ///< zuletzt uebernommene Params-Revision
        /// Puffer-Wechsel (S66): zuletzt gesehener wechselZaehler — bei neuem
        /// Stand meldet der Host wechselErbe beim Kern an
        uint64_t milkWechselZaehler = 0;

        // Shadertoy-Node (Strang S, S65): pro Node kompiliertes Wrapper-Programm.
        // Programm stirbt mit m_leafRuntimes.clear() (Context ist dort current).
        std::unique_ptr<QOpenGLShaderProgram> stProgram;
        std::string stCompiled;  ///< Code-Snapshot hinter den Programmen (alle Pässe)
        std::string stError;     ///< letzter Kompilierfehler ("" = ok; Panel fragt ab)
        int stFrame = 0;         ///< iFrame seit Kompilierung (deterministisch)
        // Multipass (S4): Buffer A..D als RGBA32F-Ping-Pong in Chain-Auflösung.
        // Nach jedem Buffer-Render wird cur GESWAPT — dadurch liest ein
        // späterer Pass automatisch das frische Bild, ein früherer/selbst-
        // referenzierender das Vorframe (Original-Semantik).
        std::unique_ptr<QOpenGLShaderProgram> stBufProgram[4];
        std::unique_ptr<QOpenGLFramebufferObject> stBufFbo[4][2];
        int stBufCur[4] = {0, 0, 0, 0};
        int stBufW = 0;
        int stBufH = 0;

        // Mesh-Warp-Node (Strang G1, S69): Vertex-Warp-Programm + Gitter.
        // Kompilierfehler landen im GETEILTEN stError (ein Knoten ist genau
        // EIN Typ — shadertoyError() liefert damit auch Mesh-Warp-Fehler an
        // den Panel-Dialog). Qt-GL-Objekte sterben mit m_leafRuntimes.clear()
        // (Context ist dort current — Shadertoy-Muster).
        std::unique_ptr<QOpenGLShaderProgram> mwProgram;
        std::string mwCompiled;   ///< Code-Snapshot hinter dem Programm
        std::unique_ptr<QOpenGLVertexArrayObject> mwVao;
        std::unique_ptr<QOpenGLBuffer> mwVbo;  ///< Gitter-Vertizes (x,y 0..1)
        std::unique_ptr<QOpenGLBuffer> mwIbo;  ///< Triangle-Indizes
        int mwGridX = 0;          ///< Gitter, für das VBO/IBO gebaut sind
        int mwGridY = 0;
        int mwIndexCount = 0;
        int mwFrame = 0;          ///< uFrame seit Kompilierung (deterministisch)

        // GPU-Partikel-Node (Strang G2, S69): Update- + Render-Programm,
        // Zustand als RGBA32F-Ping-Pong (ein Texel = pos.xy + vel.xy).
        // Kompilierfehler wieder im geteilten stError (s. Mesh-Warp-Notiz).
        std::unique_ptr<QOpenGLShaderProgram> gpUpdate;
        std::unique_ptr<QOpenGLShaderProgram> gpRender;
        std::string gpCompiled;   ///< forceCode-Snapshot hinter den Programmen
        std::unique_ptr<QOpenGLFramebufferObject> gpStateFbo[2];
        int gpCur = 0;            ///< Index des AKTUELLEN Zustands
        int gpRows = 0;           ///< Zustands-Zeilen, für die die FBOs stehen
        bool gpFresh = true;      ///< nächster Update-Pass initialisiert (uReset)

        // videoSource-Node (S70): LumiViz-Quellknoten Datei/Kamera/Testaufnahme.
        // Frame-Schritt laeuft ueber den VideoFrameCache (deterministisch),
        // Kamera/Streaming ueber den LiveVideoFeed (uhrzeitgetrieben).
        std::shared_ptr<lumi::services::VideoFrameCache::Clip> vsClip;
        std::string vsQuelle;     ///< aufgeloeste Quelle (Wechsel-Erkennung, S55-Regel)
        unsigned int vsTexture = 0;  ///< GL-Textur des zuletzt hochgeladenen Bilds
        int vsTexW = 0;           ///< Bildmasse der Textur (fuer die Einpassung)
        int vsTexH = 0;
        int vsLastIndex = -1;     ///< zuletzt hochgeladener Cache-Frame-Index
        double vsPhase = 0.0;     ///< Frame-Schritt-Akkumulator (Sim-Uhr x fps x Tempo)
        std::uint64_t vsLiveNummer = 0;  ///< zuletzt hochgeladene Live-Bild-Nummer
        bool vsLive = false;      ///< Live-Feed gestartet -> stopp() beim Cleanup

        // Pixel-Filter-Node (S70): Fragment-Programm des Nutzer-Filters.
        // Kompilierfehler wieder im geteilten stError (s. Mesh-Warp-Notiz).
        std::unique_ptr<QOpenGLShaderProgram> pfProgram;
        std::string pfCompiled;   ///< Code-Snapshot hinter dem Programm
        int pfFrame = 0;          ///< uFrame seit Kompilierung (deterministisch)

        // ISF-Filter-Node (S72): eigenes Programm-PAAR — anders als die
        // uebrigen GPU-Knoten bringt ein ISF-Shader auch eine Vertex-Stufe
        // mit. Kompilierfehler wieder im geteilten stError.
        std::unique_ptr<QOpenGLShaderProgram> isfProgram;
        std::string isfFragCompiled;  ///< Fragment-Snapshot hinter dem Programm
        std::string isfVertCompiled;  ///< Vertex-Snapshot hinter dem Programm
        /// Die Sampler-Namen sind Teil des Shader-TEXTES (uniform-Zeilen) —
        /// aendert sich die Quellen-Liste, muss neu uebersetzt werden.
        std::string isfQuellenSignatur;
        /// Ebenso die Regler: ihre Namen und Typen stehen als uniform-Zeilen
        /// im Praeludium.
        std::string isfParamSignatur;
        int isfFrame = 0;         ///< FRAMEINDEX seit Kompilierung
        /// Geometrie fuer Einzeldreieck und Gitter. Der Quad braucht keine —
        /// dafuer dient der geteilte `m_quadVao` (gleiche Attribut-Belegung).
        std::unique_ptr<QOpenGLVertexArrayObject> isfVao;
        std::unique_ptr<QOpenGLBuffer> isfVbo;
        int isfVertexAnzahl = 0;  ///< Eckpunkte im VBO
        int isfGeoBauart = -1;    ///< Bauart, fuer die der Puffer gebaut wurde
        int isfGeoX = 0;          ///< Gitterweite, fuer die er gebaut wurde
        int isfGeoY = 0;
        /// Multipass (ISF `PASSES`): je TARGET ein Puffer-PAAR. Das Paar ist
        /// noetig, weil ein Durchgang sein EIGENES Ziel lesen darf — das ist
        /// der Feedback-Fall (`PERSISTENT`): gelesen wird das Vorframe,
        /// geschrieben das neue Bild. Ohne Paar laese und schriebe derselbe
        /// Puffer gleichzeitig (undefiniert).
        struct IsfZiel
        {
            std::array<std::unique_ptr<QOpenGLFramebufferObject>, 2> fbo;
            int cur = 0;   ///< welcher der beiden das FRISCHE Bild traegt
            int breite = 0;
            int hoehe = 0;
            bool gleitkomma = false;
        };
        std::vector<IsfZiel> isfZiele;
        std::string isfPassSignatur;  ///< Snapshot, fuer den die Ziele gebaut sind
    };

    /** Render-thread state of one list node, keyed by ChainNode::nodeId. */
    struct ListRuntime
    {
        SurfacePair surface;       ///< persistent thisfb (trails inside the list)
        int beatFramesLeft = 0;    ///< OnBeat activation window
        bool needsClear = true;    ///< first use → clear
        bool seenThisFrame = false;
        // EEL list slots (compiled lazily on the render thread).
        // NB: member cannot be named "slots" — Qt reserves that keyword.
        std::unique_ptr<lumi::scripting::ScriptSlotHost> slotHost;
        std::string compiledInit;   ///< sources the current compile is based on
        std::string compiledFrame;
        std::string compiledSong;   ///< Song-Slot (S82)
    };

    /**
     * Render-thread state of one host group (HG1), keyed by nodeId: an own
     * persistent surface (the group's feedback image — never cleared per
     * frame), an OWN buffer pool (Buffer-Save slots do not leak between
     * groups) and an OWN ScriptContext (reg/q/gmegabuf group-local).
     */
    struct GroupRuntime
    {
        SurfacePair surface;
        bool needsClear = true;
        bool seenThisFrame = false;
        std::unique_ptr<lumi::render::OffscreenBufferPool> pool;
        std::shared_ptr<lumi::scripting::ScriptContext> context;
        /// HG2: Blend-Gewicht 0..1 — folgt `enabled` ueber crossfadeSeconds
        /// (0 startend = frisch aktivierte Gruppen blenden EIN); eine
        /// deaktivierte Gruppe rendert weiter, bis das Gewicht 0 erreicht.
        double blendWeight = 0.0;
        /// Block F (S86): `crossfadeSeconds` nach dem Parameter-Skript des
        /// letzten Frames (< 0 = keiner). Das Skript laeuft NACH den Kindern,
        /// das Blend-Gewicht wird davor bewegt — ein Skriptwert gilt deshalb
        /// ab dem naechsten Frame.
        double crossfadeSkript = -1.0;
        /// HG3: Laufzeit seit (Re-)Aktivierung der Gruppe — progress-Quelle
        /// fuer Milkdrop-Nodes in der Gruppe (Playlist liefert spaeter die
        /// echte Slot-Dauer); Reset beim Frischstart nach vollem Ausblenden.
        double activeSeconds = 0.0;
    };

    bool ensureSurfacePair(SurfacePair& pair, int width, int height,
                           bool* outResized = nullptr);
    void destroySurfaces();
    bool ensurePipelines();

    // Active render target = top of the surface stack (root at the bottom).
    [[nodiscard]] SurfacePair& active() { return *m_surfaceStack.back(); }
    void bindActive();

    // =========================================================================
    // Chain walk (render thread)
    // =========================================================================

    void renderNode(const lumi::multieffect::ChainNode& node);
    void renderList(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::ListParams& params);
    /// Host-Gruppe (HG1): Kinder rendern auf den persistenten Gruppen-Buffer
    /// (Feedback, kein per-Frame-Clear) mit gruppen-eigenem Buffer-Pool +
    /// ScriptContext; blendOut mischt das Gruppen-Bild auf den Parent.
    void renderHostGroup(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::HostGroupParams& params);
    void runClear(const lumi::multieffect::ChainNode& node,
                 const lumi::multieffect::ClearParams& params);
    /// Milkdrop-Meganode (N1): rendert die feste MilkDrop-Pipeline in den
    /// aktiven Chain-Buffer (Composite-Ziel = beim Kern-Frame-Start gebundenes FBO)
    void runMilkdropNode(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::MilkdropNodeParams& params);
    /// Host-Audio (Kanal-Kopien) interleaved an den Milkdrop-Kern durchreichen
    void feedMilkAudio(MilkdropVisualizer& milk);
    void runFadeout(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::FadeoutParams& params);
    void runInvert();
    void runBrightness(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::BrightnessParams& params);
    void runFastBrightness(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::FastBrightnessParams& params);
    void runBlur(const lumi::multieffect::ChainNode& node,
                const lumi::multieffect::BlurParams& params);
    void runMirror(const lumi::multieffect::ChainNode& node,
                   const lumi::multieffect::MirrorParams& params);
    void runOnBeatClear(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::OnBeatClearParams& params);
    void runColorfade(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::ColorfadeParams& params);
    void runColorModifier(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::ColorModifierParams& params);
    void runMovement(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::MovementParams& params);
    void runDynamicShift(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::DynamicShiftParams& params);
    void runDynamicDistanceModifier(
        const lumi::multieffect::ChainNode& node,
        const lumi::multieffect::DynamicDistanceModifierParams& params);
    void runMovingParticle(const lumi::multieffect::ChainNode& node,
                           const lumi::multieffect::MovingParticleParams& params);
    void runColorMap(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::ColorMapParams& params);
    void runBufferBlend(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::BufferBlendParams& params);
    void runJherikoGlobal(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::JherikoGlobalParams& params);
    void runColorClip(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::ColorClipParams& params);
    void runUniqueTone(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::UniqueToneParams& params);
    void runInterleave(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::InterleaveParams& params);
    void runConvolution(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::ConvolutionParams& params);
    void runNormalise();
    void runMultiFilter(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::MultiFilterParams& params);
    void runAddBorders(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::AddBordersParams& params);
    void runSimpleScope(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::SimpleScopeParams& params);
    void runBassSpin(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::BassSpinParams& params);
    void runOscStar(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::OscStarParams& params);
    void runOscRing(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::OscRingParams& params);
    void runRotatingStars(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::RotatingStarsParams& params);
    void runPicture(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::PictureParams& params);
    void runText(const lumi::multieffect::ChainNode& node,
                 const lumi::multieffect::TextParams& params);
    void runLyrics(const lumi::multieffect::ChainNode& node,
                   const lumi::multieffect::LyricsParams& params);
    /// Eine Bildzeile der Lyrics: `vorn` in `farbe` (laufende Zeile bzw. schon
    /// gesungene Worte), `hinten` in der Vorschau-Farbe; `klein` = halbe Hoehe.
    /// Ein Stueck der laufenden Zeile mit eigener Farbe und Groesse (S90, LB2:
    /// ein Wort — gesungen, gerade jetzt, noch nicht).
    struct LyricsStueck
    {
        std::string text;
        std::uint32_t farbe = 0;
        float skala = 1.0f;  ///< Groesse gegen die Schrift der Zeile
    };
    struct LyricsAnzeige
    {
        std::string vorn;
        std::string hinten;
        std::uint32_t farbe = 0;
        bool klein = false;
        bool laufend = false;  ///< die laufende Zeile (S90, LB1: eigenes Zeilenbild)
        /// Nicht leer = die Zeile Stueck fuer Stueck (LB2) statt `vorn`/`hinten`.
        std::vector<LyricsStueck> stuecke;
    };
    /// Welche Reihen ein Aufruf rastert und auflegt (S90, LB1). Die Lage
    /// rechnet immer ueber ALLE Reihen — jede steht, wo sie mit `Alle` stuende.
    enum class LyricsReihen { Alle, Rest, Laufende };
    /// Reihen rastern (nur bei geaendertem Zustand) und ueber das Bild legen —
    /// gemeinsam fuer Lyrics und Lyrics Trail (S87). `Laufende` rastert einen
    /// Streifen nach `rt.lyricsBild`.
    void zeichneLyricsZeilen(LeafRuntime& rt, const lumi::multieffect::LyricsParams& params,
                             const std::vector<LyricsAnzeige>& zeilen,
                             LyricsReihen reihen = LyricsReihen::Alle);
    /// Lyrics Trail (S87): die eben beendete Zeile an dieser Stelle der Kette
    /// noch einmal zeichnen; Schrift und Lage vom Lyrics-Knoten `source`.
    void runLyricsTrail(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::LyricsTrailParams& params);
    [[nodiscard]] const lumi::multieffect::ChainNode* findeLyricsKnoten(
        const std::string& name) const;
    /// Glyphen-Textur `texture` ueber das laufende Bild legen (S86; derselbe
    /// Pass wie im Text-Knoten). blend: 0 ersetzen, 1 additiv, 2 50/50.
    /// `x/y/b/h` (S90): Platz der Textur im Bild in Anteilen, y von oben —
    /// die Vorgabe ist das ganze Bild.
    void composeGlyphLayer(unsigned int texture, int blend, int staerkeProzent = 100,
                           float x = 0.0f, float y = 0.0f, float b = 1.0f, float h = 1.0f,
                           bool vormult = false);
    /// S90, Entscheid B3 a1: eine Schriftgroesse je Song. Liefert den Faktor
    /// (0,5..1), um den die Schrift `px` (Kettenmass) fuer ALLE Zeilen kleiner
    /// gesetzt wird, damit die laengste Zeile des Songtexts in die Breite
    /// passt — Schrift und Zeilenabstand gehen zusammen, die Lage bleibt ueber
    /// den Song stabil. Was bei 50 % noch nicht passt, wird einzeln verkleinert.
    [[nodiscard]] double lyricsSongFaktor(LeafRuntime& rt,
                                          const lumi::multieffect::LyricsParams& params,
                                          double px);
    /// S90, B2 a: die Reihen aus Wort-Masken auf der Grafikkarte zusammensetzen
    /// (statt sie auf der CPU zu rastern) und auflegen — fuer Knoten, deren
    /// Felder sich je Frame aendern koennen (Skript, Modulation) oder die das
    /// laufende Wort hervorheben. `basis` = die gespeicherten Felder des
    /// Knotens (Bezug fuer Maskengroesse und Strichbreite).
    void zeichneLyricsMasken(LeafRuntime& rt, const lumi::multieffect::LyricsParams& basis,
                             const lumi::multieffect::LyricsParams& params,
                             const std::vector<LyricsAnzeige>& zeilen);
    void runAvi(const lumi::multieffect::ChainNode& node,
                const lumi::multieffect::AviParams& params);
    /// Release the VfW handles of an AVI node (safe on empty runtimes).
    static void closeAviRuntime(LeafRuntime& rt);
    /// videoSource-Quellknoten (S70): Datei (Frame-Schritt/Streaming),
    /// Kamera (live, nur nach Freigabe) oder Testaufnahme.
    void runVideoSource(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::VideoSourceParams& params);
    /// Pixel-Filter-Node (S70): Nutzer-GLSL je Pixel (Stilfilter-Strang).
    void runPixelFilter(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::PixelFilterParams& params);
    void runIsfFilter(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::IsfFilterParams& params);
    /// Geometrie-Puffer des ISF-Knotens fuer die gewaehlte Bauart bereitstellen
    /// (Einzeldreieck/Gitter; der Quad benutzt den geteilten `m_quadVao`).
    /// Baut nur neu, wenn Bauart oder Gitterweite sich geaendert haben.
    void sorgeFuerIsfGeometrie(LeafRuntime& rt,
                               const lumi::multieffect::IsfFilterParams& params);
    /// Puffer der Multipass-Ziele bereitstellen (Groesse aus den
    /// WIDTH/HEIGHT-Ausdruecken). Baut nur neu, wenn sich Deklaration oder
    /// Bildgroesse geaendert haben.
    void sorgeFuerIsfZiele(LeafRuntime& rt,
                           const lumi::multieffect::IsfFilterParams& params);
    /// GL blend state for an AVS BLEND_LINE mode 0..9 (S9; 8 falls back to add).
    static void applyLineBlend(int mode, int adjustAlpha);
    /// Restore GL_FUNC_ADD + disable blending after a line-blend draw.
    static void resetLineBlend();
    void runPictureII(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::PictureIIParams& params);
    void runTexer(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::TexerParams& params);
    void runTexerII(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::TexerIIParams& params);
    void runTriangle(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::TriangleParams& params);
    /// Decode the base64 image into rt.picTexture once; true when ready.
    bool ensureEmbeddedTexture(LeafRuntime& rt, const std::string& imageData,
                               bool fallbackDot = false);
    /// Draw an embedded (base64) image over the frame, blended.
    void drawEmbeddedImage(LeafRuntime& rt, const std::string& imageData, int blend,
                           bool keepAspect);
    /// Draw a scope point-list via the shared ScopeRenderer. Honors an active
    /// Set Render Mode (S3/S9: BLEND_LINE + line width); additive otherwise.
    void drawScopeShape(const std::vector<lumi::modules::SuperscopePoint>& pts, bool dots);
    /// Dot-batch variant (Dot Grid/Plane/Fountain). blend: 0 replace,
    /// 1 additive, 2 50/50, 3 BLEND_LINE (folgt SRM — Referenz-Default).
    void drawDots(const std::vector<lumi::modules::SuperscopePoint>& pts, float dotSize,
                  int blend = 3);
    void runDynamicMovement(const lumi::multieffect::ChainNode& node,
                            const lumi::multieffect::DynamicMovementParams& params);
    /** Shared grid-warp: run the module, build the mesh, sample current→partner. */
    /// Options for applyGridWarp beyond the mandatory grid geometry.
    struct GridWarpOptions
    {
        bool wrap = false;
        bool blend = false;        ///< alpha-mix moved pixel onto the original
        bool nomove = false;       ///< no displacement, alpha-blend source only
        bool staticField = false;  ///< evaluate the field only on compile/resize
        bool subpixel = true;      ///< bilinear (on) vs nearest (off) sampling
        unsigned int srcTexture = 0;  ///< warp source; 0 = current frame
    };
    void applyGridWarp(LeafRuntime& rt, int xres, int yres,
                       const GridWarpOptions& opt);
    /**
     * Bit-treuer Warp-Pass fuer Dynamic Movement (r_dmove): die Fixpunkt-Tabelle
     * des Gitters geht als Integer-Textur an den Shader, der die separable
     * Ganzzahl-Interpolation des Originals geschlossen nachrechnet. Liefert
     * false, wenn das Original an dieser Geometrie abbricht (Band der Breite 0).
     */
    bool applyGridWarpFx(LeafRuntime& rt, int xres, int yres,
                         const GridWarpOptions& opt);
    void applyGridScatter(LeafRuntime& rt, int xres, int yres,
                          const GridWarpOptions& opt);
    /**
     * Bit-treuer Movement-Pass (r_trans): das Punkt-Skript laeuft je PIXEL in
     * eine Tabelle (nur bei Groessen-/Skriptwechsel), der Shader dekodiert sie
     * samt 5-Bit-Subpixel. false = kein lebender Punkt-Slot (Gitter-Fallback).
     */
    bool applyMovementTable(LeafRuntime& rt,
                            const lumi::multieffect::MovementParams& params);
    void runBlitterFeedback(const lumi::multieffect::ChainNode& node,
                            const lumi::multieffect::BlitterFeedbackParams& params);
    void runRotoBlitter(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::RotoBlitterParams& params);
    /**
     * Gemeinsamer Feedback-Pass (r_blit/r_rotblit, S48): affines Sampling in
     * Pixeln (src = map*dest + off), wrap = Kachel-Modulo wie r_rotblit,
     * blend = BLEND_AVG mit dem Original, subpixel = BLEND4-Bilinear.
     */
    void feedbackPass(const QMatrix2x2& map, const QVector2D& off, bool wrap,
                      bool blend, bool subpixel);
    void runBufferSave(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::BufferSaveParams& params);
    void runFyrewurX(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::FyrewurXParams& params);
    void runMetaballs3D(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::Metaballs3DParams& params);
    void runTentacles3D(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::Tentacles3DParams& params);
    void runCustomBpm(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::CustomBpmParams& params);
    void runSetRenderMode(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::SetRenderModeParams& params);
    void runSuperScope(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::SuperScopeParams& params);
    void runDebugBars(const lumi::multieffect::DebugBarsParams& params);
    void runMosaic(const lumi::multieffect::ChainNode& node,
                   const lumi::multieffect::MosaicParams& params);
    void runGrain(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::GrainParams& params);
    void runScatter(const lumi::multieffect::ScatterParams& params);
    void runInterferences(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::InterferencesParams& params);
    void runWater(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::WaterParams& params);
    void runBump(const lumi::multieffect::ChainNode& node,
                 const lumi::multieffect::BumpParams& params);
    void runWaterBump(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::WaterBumpParams& params);
    /// Bloom (Lights-Etappe 1), In-Chain-Modus (post=false): Glow erzeugen
    /// und additiv auf die Surface compositen; post=true rendert als No-op
    /// (der Glow entsteht dann erst beim Present — kein Feedback).
    void runBloom(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::BloomParams& params);
    /// Bloom-Glow erzeugen: Downsample (+Threshold) → separierbarer
    /// 25-Tap-Gauss in rt.bloomRt; liefert die Glow-Textur (0 bei Fehler).
    /// `threshold`/`radius` kommen getrennt herein, weil sie im Knoten aus der
    /// Strang-D-Frame-Kopie stammen koennen und nicht aus `params` (S54).
    unsigned int ensureBloomGlow(LeafRuntime& rt,
                                 const lumi::multieffect::BloomParams& params,
                                 unsigned int srcTexture, float threshold,
                                 int radius);
    /// 3D Camera (Lights-Etappe 1): setzt m_camera3d fuer folgende 3D-Module
    /// (Frame-Zustand wie Set Render Mode); EEL-Slots als dynamische Params.
    void runCamera3D(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::Camera3DParams& params);
    /// SuperScope 3D (Lights-Etappe 1): EEL-Punktschleife in Weltkoordinaten,
    /// Projektion via m_camera3d, additive Soft-Sprites oder Linien.
    void runSuperScope3D(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::SuperScope3DParams& params);
    /// Terrain 3D (Lights-Etappe 2): Heightfield-Sim (Ringe/Relax/Skript-
    /// megabuf) + dunkles Mesh (Depth) + additive Gitterpunkt-Sprites.
    void runTerrain3D(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::Terrain3DParams& params);
    /// Glow Orbs (Lights-Etappe 2): Ellipsoide (Verlauf+flash, Depth) + Halos.
    void runGlowOrbs(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::GlowOrbsParams& params);
    /// View/Proj aus m_camera3d (Aspekt = interne Surface, near 0.05).
    void computeCamera3D(QMatrix4x4& view, QMatrix4x4& proj) const;
    /// Gemeinsames Depth-RT (Etappe-2-Entscheid 1): Depth-Textur an das
    /// AKTUELL gebundene Draw-FBO haengen, einmal je Frame loeschen,
    /// Depth-Test an. end3DDepth() loest das Attachment wieder.
    void begin3DDepth();
    void end3DDepth();
    void runStarfield(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::StarfieldParams& params);
    void runTimescope(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::TimescopeParams& params);
    void runDotGrid(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::DotGridParams& params);
    void runDotPlane(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::DotPlaneParams& params);
    void runDotFountain(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::DotFountainParams& params);
    void runChannelShift(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::ChannelShiftParams& params);
    void runColorReduction(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::ColorReductionParams& params);
    void runMultiplier(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::MultiplierParams& params);
    void runVideoDelay(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::VideoDelayParams& params);
    void runMultiDelay(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::MultiDelayParams& params);
    void runFractal2D(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::Fractal2DParams& params);
    void runDomainWarp(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::DomainWarpParams& params);
    void runFractal3D(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::Fractal3DParams& params);
    void runLyapunov(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::LyapunovParams& params);
    void runKleinian(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::KleinianParams& params);
    void runFractalZoomer(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::FractalZoomerParams& params);
    void runStrangeAttractor(const lumi::multieffect::ChainNode& node,
                             const lumi::multieffect::StrangeAttractorParams& params);
    void runFlame(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::FlameParams& params);
    void runReactionDiffusion(const lumi::multieffect::ChainNode& node,
                              const lumi::multieffect::ReactionDiffusionParams& params);
    /// Shadertoy-Node (Strang S): ein Fragment-Pass in Chain-Auflösung.
    void runShadertoy(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::ShadertoyParams& params);
    /// Mesh-Warp-Node (Strang G1, S69): Nutzer-GLSL `warp()` je Gitter-Vertex
    /// im Vertex-Shader — Quell-UV-Remap des aktuellen Chain-Bilds.
    void runMeshWarp(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::MeshWarpParams& params);
    /// GPU-Partikel-Node (Strang G2, S69): Zustands-Ping-Pong (pos+vel je
    /// Texel) + instanzierter Sprite-Draw; Alter/Respawn hash-basiert.
    void runGpuParticles(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::GpuParticlesParams& params);
    /// 512×2-Audio-Textur des Shadertoy-Vertrags hochladen (Zeile 0 = FFT,
    /// Zeile 1 = Waveform, 0..1) — je Aufruf frisch (1 KiB, unkritisch).
    void updateShadertoyAudioTexture();
    /// Bake a gradient preset into a node's 256x1 palette LUT (Batch H shared).
    void ensureFractalLut(LeafRuntime& rt, const std::string& preset);

    /// Rebuild m_visdata (AVS layout) from this frame's waveform + spectrum.
    void buildVisData();
    /// Rohe visdata-Bytes (AVS-Vertrag, S48): getSpectrum()/getWaveform()
    /// sind NORMALISIERT (App-Skala) — AVS-treue Effekte lesen stattdessen
    /// diese Bloecke (Spektrum linear /16, Waveform-Bytes ^128-Konvention);
    /// dieselbe Quelle wie getspec/getosc der Skripte (eine Wahrheit).
    [[nodiscard]] const unsigned char* visSpectrum(int channel) const
    {
        return m_visdata.data() + (channel & 1) * 576;
    }
    [[nodiscard]] const unsigned char* visWaveform(int channel) const
    {
        return m_visdata.data() + (2 + (channel & 1)) * 576;
    }
    /// Normalisierte Welle/Spektrum nach dem AVS-Kanalfeld: 0 = links,
    /// 1 = rechts, alles andere = Mitte. `getWaveform()`/`getSpectrum()`
    /// mischen bei Stereo-Material selbst zur Mitte (`mixToMono`), deshalb ist
    /// der Mittenfall genau der bisherige Aufruf ohne Kanal — ein Knoten mit
    /// der ueblichen Vorgabe `channel = 2` zeichnet unveraendert weiter.
    /// EINE Stelle, weil Osc Star und Osc Ring dieselbe Wahl treffen (S57: dort
    /// wurde der Kanal gar nicht gelesen, beide Sonden waren stumm).
    [[nodiscard]] std::vector<float> waveOfChannel(int channel) const
    {
        return (channel == 0 || channel == 1) ? getWaveformChannel(channel)
                                             : getWaveform();
    }
    [[nodiscard]] std::vector<float> specOfChannel(int channel) const
    {
        return (channel == 0 || channel == 1) ? getSpectrumChannel(channel)
                                             : getSpectrum();
    }
    /// Feed the shared audio contract (visdata + gettime + bass/mid/treb/vol/beat/
    /// time) into a script engine — every scripted module gets the same set (E1).
    void feedAudio(lumi::scripting::LuaScriptEngine& engine);

    /// Ein Reglerwert, den das Parameter-Skript lesen und schreiben darf.
    struct ParamVar
    {
        const char* name;  ///< EEL-Bezeichner (klein geschrieben)
        double* value;     ///< rein VOR dem Lauf, raus DANACH
    };

    /**
     * Parameter-Skript eines Knotens fahren (Strang D, Knoten_Parameter_Konzept §6).
     *
     * Sind alle drei Quellen leer, passiert NICHTS — kein Host, kein Transpiler,
     * kein Lua-Aufruf. Erst ein nicht-leeres Feld kostet Rechenzeit (opt-in).
     * Die aktuellen Reglerwerte gehen als Startwerte hinein, damit ein Skript,
     * das nur eine Variable setzt, alle anderen unangetastet laesst.
     */
    void runParamScript(LeafRuntime& rt, const char* prefix, const std::string& init,
                        const std::string& frame, const std::string& beat,
                        const std::vector<ParamVar>& vars);

    [[nodiscard]] uint32_t nextRandom();  ///< host LCG (Mirror onbeat-random)

    /** Full-screen transform pass on the active pair: sample current, write partner, swap. */
    void transformPass(QOpenGLShaderProgram& shader);

    /**
     * Blend src over dst.current() into dst.partner(), then swap dst.
     * Ignore is a no-op. For BlendMode::Buffer, `bufferTexture` supplies the
     * global-buffer depth source (`bufferInvert` flips it); a missing buffer
     * (bufferTexture == 0) leaves dst untouched, matching AVS' `if (!buf) break`.
     */
    void blendPass(SurfacePair& dst, unsigned int srcTexture,
                   lumi::multieffect::BlendMode mode, int adjustAlpha,
                   unsigned int bufferTexture = 0, bool bufferInvert = false);

    /** Texture of pool slot `n` at the surface size, or 0 if nothing is saved. */
    unsigned int poolTexture(int n);

    lumi::multieffect::ChainNode m_root;

    // GL state (render thread only)
    SurfacePair m_rootSurface;
    /// Scratch ping-pong for blended Buffer-Save writes into pool FBOs
    SurfacePair m_bufferScratch;
    std::vector<SurfacePair*> m_surfaceStack;
    std::unordered_map<uint64_t, ListRuntime> m_listRuntimes;
    std::unordered_map<uint64_t, GroupRuntime> m_groupRuntimes;  ///< HG1

    /// Host Switcher (S82): Laufzeit je Switcher-Knoten — laufende Gruppe,
    /// Zeit/Beats seit dem letzten Wechsel, Skript.
    struct SwitcherRuntime
    {
        int index = -1;          ///< laufende Gruppe 0-basiert; -1 = frisch
        double sekunden = 0.0;   ///< seit dem letzten Wechsel
        int beats = 0;           ///< seit dem letzten Wechsel
        bool seenThisFrame = false;
        uint32_t zufall = 0x5EED82u;  ///< xorshift-Zustand fuer „zufaellig"
        std::unique_ptr<lumi::scripting::ScriptSlotHost> host;
        std::string compiled;
        std::string error;
    };
    std::unordered_map<uint64_t, SwitcherRuntime> m_switcherRuntimes;

    // --- Song-Slot (S82): einmal je Songwechsel, fuer alle Skript-Knoten ---
    /// Songwechsel in DIESEM Frame (Zaehler notifySongChanged gegen den
    /// zuletzt gesehenen Stand, je Frame in onRender).
    bool m_frameSong = false;
    /// ~0 = noch kein Frame: der erste Frame uebernimmt den Stand, ohne dass
    /// fruehere Songwechsel als neu gelten
    uint64_t m_songGesehen = ~uint64_t{0};
    /// Song-Code des Knotens, den renderNode gerade laeuft (Scope wie
    /// activeContext; Listen setzen ihn fuer ihre Kinder neu und zurueck).
    const std::string* m_nodeSong = nullptr;
    /// Slot „Line" (S89, Auftrag Patrik): nur der Lyrics-Knoten setzt das —
    /// fuer die Dauer seines runParamScript. Code des Slots, „in diesem Frame
    /// hat die Zeile gewechselt", und die Werte fuer `line` / `linecount`.
    const std::string* m_nodeLine = nullptr;
    bool m_frameLine = false;
    /// S90, LB3: dasselbe fuer das laufende Wort — Code des Slots „Word",
    /// „in diesem Frame hat das Wort gewechselt", `word` und `wordcount`.
    const std::string* m_nodeWord = nullptr;
    bool m_frameWord = false;
    int m_wordNummer = 0;
    int m_wordAnzahl = 0;
    /// S90, LB4: was das Skript des Lyrics-Knotens in diesem Frame als
    /// `linetrigger` / `wordtrigger` hinterlassen hat (0 = nichts).
    double m_lineTrigger = 0.0;
    double m_wordTrigger = 0.0;
    /// S90 (Entscheid B1 a): Stand der Skriptvariablen gleich NACH dem Feld
    /// Line und welche davon Line veraendert hat — je Lauf von runParamScript
    /// neu, leer ausserhalb eines Zeilenwechsels mit Line-Code.
    std::vector<double> m_lineWert;
    std::vector<char> m_lineGeschrieben;
    int m_lineNummer = 0;
    int m_lineAnzahl = 0;
    /// Anhang an die Kompilier-Vergleiche der Skriptstellen: aendert sich nur
    /// der Song-Code, wird trotzdem neu kompiliert.
    [[nodiscard]] std::string songSchluessel() const;
    /// Song-Quelle in einen frisch angelegten Host setzen (neben Init/Frame/Beat).
    void setzeSong(lumi::scripting::ScriptSlotHost& host) const;
    /// Traegt der laufende Knoten Song-Code? (Stellen mit „nur wenn Skript")
    [[nodiscard]] bool hatSongCode() const { return m_nodeSong != nullptr && !m_nodeSong->empty(); }
    /// Song-Slot ausfuehren, wenn in diesem Frame ein Songwechsel war — VOR Frame.
    void laufeSong(lumi::scripting::ScriptSlotHost& host) const;

    // --- Block F, Stufe F2: generische Feldsteuerung (Feldsteuerung_Konzept.md) ---
    /// Frame-Kopie der Params des Knotens, den renderNode gerade laeuft — nur
    /// gesetzt, wenn der Knotentyp ueber `runParamScript` skriptet und Code
    /// traegt. `runParamScript` bindet dann ALLE Felder der Feldtabelle und
    /// schreibt die Ergebnisse hier hinein; der Renderer liest genau diese
    /// Kopie (er bekommt sie als `params`). Scope wie m_nodeSong.
    lumi::multieffect::EffectParams* m_gesteuerteKopie = nullptr;
    /// Tabellenfelder in die Variablenliste eines Parameter-Skripts und nach
    /// dem Lauf zurueck in m_gesteuerteKopie (Typpflege, Strukturregel E3).
    struct FeldBindung;
    void feldBindungVor(LeafRuntime& rt, std::vector<ParamVar>& vars,
                        std::vector<FeldBindung>& plan, std::vector<double>& werte) const;
    void feldBindungNach(LeafRuntime& rt, bool fresh, const std::vector<ParamVar>& vars,
                         const std::vector<FeldBindung>& plan);
    /// Block F (F2b, S85): dasselbe fuer Module mit EIGENEM Skript-Host.
    /// `feldHostVor` speist VOR den Slots die Reglerwerte ein — beim Start
    /// (`fresh`) und wenn der Regler bewegt wurde; sonst rechnet das Skript
    /// fort. `feldHostNach` liest NACH den Slots zurueck in m_gesteuerteKopie
    /// (Typpflege, Strukturregel E3). `hand` = Namen, die der Renderer selbst
    /// einspeist (Farben stufenlos) — feldHostVor laesst sie aus; ihr
    /// Skriptwert landet trotzdem in der Kopie. Renderer-eigene Namen
    /// (`feldtabelle::rendererEigen`, etwa SuperScope 3D `size` je Punkt)
    /// werden weder eingespeist noch zurueckgelesen. `ohne` (S86) nimmt Namen
    /// auch vom Zuruecklesen aus — fuer Variablen, die der Renderer nach
    /// eigener Regel auswertet (Effect List `alphain`/`alphaout`).
    void feldHostVor(LeafRuntime& rt, lumi::scripting::LuaScriptEngine& engine, bool fresh,
                     const std::set<std::string>& hand = {});
    void feldHostNach(LeafRuntime& rt, lumi::scripting::LuaScriptEngine& engine, bool fresh,
                      const std::set<std::string>& ohne = {});

    // --- Block F, Stufe F4: Modulation ohne Formel (S85) ---------------------
    /// Laufzeit einer Modulation (je Knoten und Feld): LFO-Phase, gehaltener
    /// Zufallswert, geglaettete Audio-Quelle, Beat-Huelle.
    struct ModZustand
    {
        double phase = 0.0;
        double halt = 0.0;
        double glatt = 0.0;
        double huelle = 0.0;
        uint32_t zufall = 0;  ///< 0 = noch nicht gesaet
        double zyklus = -1.0; ///< F4d: zuletzt gesehener Beat-Zyklus (Zufall-halten)
    };
    /// F4d (S86): Beat-Uhr der Modulation — Lage in Beats (mit Bruchteil),
    /// Sekunden seit dem letzten Beat, gemessener Beat-Abstand.
    /// S86: Tags des laufenden Titels (Kopie fuer den Render-Thread) und der
    /// Stand, zu dem sie gehoeren.
    lumi::multieffect::texttags::SongTags m_songTags;
    lumi::multieffect::lyrics::Songtext m_songtext;  ///< S86, gleicher Stand wie die Tags
    uint64_t m_songTagsStand = 0;
    double m_modBeatLage = 0.0;
    double m_modBeatSeit = 0.0;
    double m_modBeatPeriode = 0.5;
    bool m_modBeatErster = true;
    std::unordered_map<uint64_t, std::unordered_map<std::string, ModZustand>> m_modZustand;
    /// Zuletzt angewendeter Wert je Knoten und Feld — fuer die Anzeige im Panel
    std::unordered_map<uint64_t, std::unordered_map<std::string, double>> m_modWerte;
    /// Audio-Baender einmal je Frame (die Modulation fragt sie je Feld)
    bool m_modBaenderGueltig = false;
    float m_modBass = 0.0f, m_modMid = 0.0f, m_modTreble = 0.0f;
    /// true, wenn der Knoten mindestens eine aktive Modulation traegt und sein
    /// Knotenschalter an ist
    [[nodiscard]] static bool moduliert(const lumi::multieffect::ChainNode& node);
    /// Modulationen des Knotens auf die Frame-Kopie `kopie` anwenden (vor den Skripten, E5)
    void wendeModulationAn(const lumi::multieffect::ChainNode& node,
                           lumi::multieffect::EffectParams& kopie);
    [[nodiscard]] double modQuellwert(const lumi::multieffect::FeldModulation& m, ModZustand& z,
                                      uint64_t nodeId);

public:
    /// F4: zuletzt angewendeter Modulationswert eines Felds (Panel-Anzeige);
    /// leer, wenn das Feld in diesem Frame nicht moduliert war. Aufrufer haelt
    /// den Render-Mutex.
    [[nodiscard]] std::optional<double> modulierterWert(uint64_t nodeId,
                                                        const std::string& feld) const;

private:

public:
    /// Nur fuer Tests: sieht nach jedem gesteuerten Knoten die fertige
    /// Frame-Kopie (Wirkungstest Block F). Leer = aus.
    using ParamBeobachter =
        std::function<void(const lumi::multieffect::ChainNode&,
                           const lumi::multieffect::EffectParams&)>;
    static void setzeParamBeobachter(ParamBeobachter beobachter);

private:
    /// Gruppen-nodeId -> laeuft (true) / ruht (false), je Frame von den
    /// Switchern neu gesetzt; fehlt eine Gruppe, gilt ihr `enabled`.
    std::unordered_map<uint64_t, bool> m_groupOverride;
    /// S89 (Idee Patrik): Gruppen, deren Zustand ein Switcher mit „Keep group
    /// state" haelt — je Frame von den Switchern neu gesetzt. Eine ruhende
    /// Gruppe darin behaelt Bild, Laufzeit und ihre verschachtelten Listen.
    std::unordered_set<uint64_t> m_gruppenPuffer;
    /// Switcher, deren Puffer geleert werden sollen (Knopf im Panel) —
    /// abgearbeitet vom Render-Thread in runHostSwitchers.
    std::unordered_set<uint64_t> m_pufferLeeren;
    /// Die RUHENDEN Gruppen der Liste wirklich frisch machen (Entscheid Patrik
    /// S89 a): Bild, Laufzeit, Pool, Skript-Kontext, verschachtelte Listen und
    /// die Laufzeiten aller Knoten darin. Laufende Gruppen bleiben unberuehrt.
    void leerePufferVon(const std::vector<std::string>& gruppen);
    /// Verschachtelte Listen einer ruhenden, gepufferten Gruppe am Leben
    /// halten (sonst fallen ihre Laufzeiten am Frame-Ende weg).
    void haltePufferAmLeben(const lumi::multieffect::ChainNode& node);
    /// Alle Host Switcher der obersten Ebene einen Frame weiterrechnen und
    /// m_groupOverride setzen — VOR dem HG2-Gewichtsscan (onRender).
    void runHostSwitchers();
    /// HG2: laufende Gewichtssumme des normalisierten Gruppen-Mixes — startet
    /// je Frame beim Hintergrund-Anteil max(0, 1 - Summe blendender Gruppen)
    double m_blendRunningSum = 1.0;
    /// HG3: Laufzeit der AKTIVEN Gruppe waehrend des Kinder-Walks (Scope wie
    /// activePool/activeContext); -1 = ausserhalb jeder Host-Gruppe
    double m_groupActiveSeconds = -1.0;
    int m_surfaceWidth = 0;
    int m_surfaceHeight = 0;
    bool m_firstFrame = true;
    bool m_pendingRuntimeReset = false;  ///< a new chain was installed (loadAvsFile)
    /// S88: nach `recompileChain()` (Panel-Edit) raeumt der Render-Thread die
    /// Laufzeiten von Knoten ab, die es in der Kette nicht mehr gibt.
    bool m_pendingRuntimePrune = false;
    uint64_t m_idHochwasser = 1;         ///< naechste nie vergebene Knoten-Id (recompileChain)
    // --- ungespeicherte Aenderungen (S89)
    QByteArray m_kettenStand;                       ///< die Kette, wie sie zuletzt geladen/gespeichert war
    mutable bool m_kettenBeruehrt = false;          ///< seit dem letzten Vergleich gab es einen Panel-Edit
    mutable bool m_kettenGeaendertZuletzt = false;  ///< Ergebnis des letzten Vergleichs
    QByteArray m_karaokeStand;                      ///< die Ebene, wie sie zuletzt benannt geladen/gespeichert war

    /** Free all per-node GL runtimes (render thread, context current). */
    // --- Karaoke-Ebene (S88, KE1)
    void baueKaraokeEbene();     ///< die festen Knoten der Ebene anlegen (Konstruktor)
    void rechneKaraokeEbene();   ///< nach dem Ketten-Walk: Zeile rastern, Trail-Liste rechnen
    void legeKaraokeAuf();       ///< auf den gebundenen Framebuffer: Trail additiv, Zeile mit Alpha
    /// Baum der Ebene (nicht in m_root): Kind 0 = Lyrics-Knoten, Kind 1 = Liste
    /// (Drift, Fadeout, Lyrics Trail). Die Wurzel selbst wird nie gerendert.
    lumi::multieffect::ChainNode m_karaokeWurzel;
    /// Ids der Ebenen-Knoten: eigener Zaehler weit ueber allem, was die Kette
    /// vergibt — ihre Laufzeiten beissen sich nie mit denen der Kette.
    static constexpr uint64_t kKaraokeIdBasis = 0xF000'0000'0000'0000ull;
    uint64_t m_karaokeIdHochwasser = kKaraokeIdBasis + 1;
    /// Gehoert die Id einem Knoten der Karaoke-Ebene? Deren Laufzeiten
    /// ueberleben den Wechsel der Kette (S91) — der Trail reisst nicht ab.
    [[nodiscard]] static constexpr bool istKaraokeId(uint64_t id) { return id >= kKaraokeIdBasis; }
    [[nodiscard]] const lumi::multieffect::ChainNode* karaokeLyricsKnoten() const;
    [[nodiscard]] const lumi::multieffect::ChainNode* karaokeTrailKnoten() const;
    bool m_karaokeLaeuft = false;      ///< der Walk der Ebene laeuft (Trail folgt deren Lyrics-Knoten)
    bool m_glyphNurRastern = false;    ///< zeichneLyricsZeilen: rastern, nicht auflegen
    bool m_karaokeGezeichnet = false;  ///< die Ebene war in diesem Frame dran
    bool m_karaokeAuslassen = false;   ///< Composer-Naht R2: diese Instanz zeichnet die Ebene nie
    std::uint64_t m_karaokeFassung = 0;  ///< Composer-Naht R2: zaehlt `recompileKaraoke()`
    unsigned int m_presentZiel = 0;    ///< Composer-Naht R1: Framebuffer des Presents (0 = Fenster)
    int m_movementImBau = 0;           ///< S94: Movement-Knoten, deren Tabelle in diesem Frame noch entsteht
    std::uint64_t m_movementBauFrame = 1;  ///< S94: zaehlt die Frames fuer den Bau in Scheiben
    /// K4 b: Fenstergroesse (Viewport in onRender) — darin rastert die Zeile
    /// der Ebene; der Trail bleibt in Kettenaufloesung.
    int m_fensterBreite = 0;
    int m_fensterHoehe = 0;
    /// Was `zeichneLyricsZeilen` rastert: alles in eine Textur (Vorgabe) oder,
    /// fuer die Karaoke-Ebene mit Mischart, Fuellung und Kontur-Ring getrennt.
    enum class GlyphTeil { Alles, NurFuellung, NurKontur };
    GlyphTeil m_glyphTeil = GlyphTeil::Alles;
    /// Glyphen-Bilder der Ebene fuer diesen Frame, in Auflege-Reihenfolge
    /// (S90, LB1: uebrige Reihen und laufende Zeile getrennt, je mit Platz).
    struct KaraokeLage
    {
        unsigned int tex = 0;
        bool kontur = false;  ///< Kontur-Ring: liegt immer normal und voll
        float x = 0.0f, y = 0.0f, b = 1.0f, h = 1.0f;
        bool vormult = false;  ///< Flaeche des Masken-Wegs (B2 a)
    };
    std::unique_ptr<QOpenGLShaderProgram> m_maskenShader;  ///< Wort-Maske -> Farbe (B2 a)
    std::vector<KaraokeLage> m_karaokeLagen;
    unsigned int m_karaokeTrailTex = 0;  ///< Bild der Trail-Liste (0 = keins)
    /// Mischart (0..8) und Staerke (0..100) fuers Auflegen — je Frame aus den
    /// Knoten der Ebene gelesen (Lyrics bzw. Lyrics Trail, nach deren Skript).
    int m_karaokeZeileArt = 0;
    int m_karaokeZeileStaerke = 100;
    int m_karaokeTrailArt = 1;
    int m_karaokeTrailStaerke = 100;
    std::unique_ptr<QOpenGLShaderProgram> m_overlayShader;  ///< Textur 1:1, wahlweise y-gespiegelt

    /// Laufzeiten der Kette verwerfen (Wechsel der Kette); die der
    /// Karaoke-Ebene bleiben, ausser beim Abbau (`auchEbene`).
    void resetRuntimes(bool auchEbene = false);
    /// GL-Objekte, VfW-Handles, Milkdrop-Kern und Live-Feed EINER Laufzeit
    /// freigeben (Render-Thread; ohne GL-Context nur der Feed).
    void gibLeafLaufzeitFrei(uint64_t id, LeafRuntime& rt);
    /// S88: Laufzeiten aller Knoten abraeumen, die es in `m_root` nicht mehr
    /// gibt (nach einem Panel-Edit). Ids werden nie wiederverwendet (S87) —
    /// ohne das bliebe der Zustand eines entfernten Knotens bis zum naechsten
    /// Laden liegen, und sein Video-/Kamera-Feed liefe weiter.
    void raeumeVerwaisteLaufzeiten();

    std::unique_ptr<QOpenGLShaderProgram> m_fadeShader;
    std::unique_ptr<QOpenGLShaderProgram> m_invertShader;
    std::unique_ptr<QOpenGLShaderProgram> m_barsShader;
    std::unique_ptr<QOpenGLShaderProgram> m_blendShader;
    std::unique_ptr<QOpenGLShaderProgram> m_brightShader;
    std::unique_ptr<QOpenGLShaderProgram> m_blurShader;
    std::unique_ptr<QOpenGLShaderProgram> m_mirrorShader;
    /// Feld-Shader des Metaballs-3D-Nachbaus (S52)
    std::unique_ptr<QOpenGLShaderProgram> m_metaballShader;
    std::unique_ptr<QOpenGLShaderProgram> m_colorfadeShader;
    std::unique_ptr<QOpenGLShaderProgram> m_lutShader;
    std::unique_ptr<QOpenGLShaderProgram> m_warpShader;
    std::unique_ptr<QOpenGLShaderProgram> m_warpFxShader;  ///< r_dmove-Fixpunkt
    std::unique_ptr<QOpenGLShaderProgram> m_moveTabShader;  ///< r_trans-Tabelle
    std::unique_ptr<QOpenGLShaderProgram> m_moveRemapShader;
    std::unique_ptr<QOpenGLShaderProgram> m_textShader;
    /// videoSource-Overlay (S70): Einpassung + Deckkraft + Blend in einem Pass
    std::unique_ptr<QOpenGLShaderProgram> m_videoShader;
    std::unique_ptr<QOpenGLShaderProgram> m_feedbackShader;
    std::unique_ptr<QOpenGLShaderProgram> m_mosaicShader;
    std::unique_ptr<QOpenGLShaderProgram> m_grainShader;
    std::unique_ptr<QOpenGLShaderProgram> m_scatterShader;
    std::unique_ptr<QOpenGLShaderProgram> m_interfShader;
    std::unique_ptr<QOpenGLShaderProgram> m_waterShader;
    std::unique_ptr<QOpenGLShaderProgram> m_bumpShader;
    std::unique_ptr<QOpenGLShaderProgram> m_presentShader;  ///< Quad-Present (Render Scale, S47)
    std::unique_ptr<QOpenGLShaderProgram> m_bloomDownShader;   ///< Bloom: Downsample + Threshold
    std::unique_ptr<QOpenGLShaderProgram> m_bloomGaussShader;  ///< Bloom: separierbarer 25-Tap-Gauss
    std::unique_ptr<QOpenGLShaderProgram> m_bloomCompShader;   ///< Bloom: additives Composite + Vignette
    std::unique_ptr<QOpenGLShaderProgram> m_sprite3dShader;    ///< SuperScope 3D: Soft-Sprites
    std::unique_ptr<QOpenGLShaderProgram> m_terrain3dShader;   ///< Terrain 3D: opakes Mesh + Fog
    std::unique_ptr<QOpenGLShaderProgram> m_orb3dShader;       ///< Glow Orbs: Ellipsoid + Verlauf
    std::unique_ptr<QOpenGLShaderProgram> m_flatShader;        ///< Flat-Color-Fill (my_triangle, S48)
    std::unique_ptr<QOpenGLShaderProgram> m_shiftShader;  ///< Dynamic Shift (r_shift)
    std::unique_ptr<QOpenGLShaderProgram> m_ddmShader;    ///< Dynamic Distance Modifier (r_ddm)
    std::unique_ptr<QOpenGLShaderProgram> m_colorMapShader;  ///< Color Map APE
    std::unique_ptr<QOpenGLShaderProgram> m_bufferBlendShader;  ///< Buffer blend APE
    std::unique_ptr<QOpenGLShaderProgram> m_pictureShader;      ///< Picture (r_picture)
    std::unique_ptr<QOpenGLShaderProgram> m_spriteShader;       ///< Texer / Texer II sprites
    std::unique_ptr<QOpenGLShaderProgram> m_colorClipShader;    ///< Color Clip (r_contrast)
    std::unique_ptr<QOpenGLShaderProgram> m_uniqueToneShader;   ///< Unique Tone (r_onetone)
    std::unique_ptr<QOpenGLShaderProgram> m_interleaveShader;   ///< Interleave (r_interleave)
    std::unique_ptr<QOpenGLShaderProgram> m_convolutionShader;  ///< Convolution APE
    std::unique_ptr<QOpenGLShaderProgram> m_normaliseShader;    ///< Normalise APE
    std::unique_ptr<QOpenGLShaderProgram> m_multiFilterShader;  ///< MultiFilter APE
    std::unique_ptr<QOpenGLShaderProgram> m_addBordersShader;   ///< Add Borders APE
    std::unique_ptr<QOpenGLFramebufferObject> m_reduceFbo;      ///< Normalise min/max readback (32x32)
    /// Convolution-Pass-2-Ziel (in-place-Kante, S59) — liest Pass 1 + Eingabe
    std::unique_ptr<QOpenGLFramebufferObject> m_convScratch;
    std::unique_ptr<QOpenGLShaderProgram> m_wbPropShader;  ///< water-bump wave propagation
    std::unique_ptr<QOpenGLShaderProgram> m_wbDispShader;  ///< water-bump refraction
    std::unique_ptr<QOpenGLShaderProgram> m_timescopeShader;
    std::unique_ptr<QOpenGLShaderProgram> m_apeShader;  ///< Channel Shift / Color Reduction / Multiplier
    std::unique_ptr<QOpenGLShaderProgram> m_fractal2DShader;  ///< Fractal 2D escape-time (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_domainWarpShader;  ///< Domain-warp fBm (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_fractal3DShader;   ///< Fractal 3D raymarch (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_lyapunovShader;    ///< Lyapunov (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_kleinianShader;    ///< Kleinian tiling (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_rdShader;          ///< Reaction-Diffusion sim/show (Batch H)
    unsigned int m_specTex = 0;  ///< 1D spectrum upload for Timescope (deleted in onCleanup)
    /// Shadertoy-Audio (Strang S): geteilte 512×2-R8-Textur (Zeile 0 = FFT,
    /// Zeile 1 = Waveform); je runShadertoy frisch hochgeladen, Freigabe onCleanup
    unsigned int m_stAudioTex = 0;
    /// FFT-Glättungszustand der Shadertoy-Audio-Textur (S67): WebAudio-
    /// AnalyserNode-Vertrag — smoothingTimeConstant 0,8 auf der MAGNITUDE,
    /// erst danach die dB-Spanne [-100,-30] (getByteFrequencyData)
    std::array<float, 512> m_stAudioSmooth{};
    /// ISF-Audio (S72): ISF erwartet ZWEI getrennte Texturen — `audio` traegt
    /// die Waveform, `audioFFT` das Spektrum (Breite = Samples/Bins, Hoehe =
    /// Kanaele). Sie aus derselben Rechnung zu speisen wie die
    /// Shadertoy-Textur ist Absicht: die dB-Skala und die 0,8-Glaettung
    /// (S67-Befund) duerfen nicht auseinanderlaufen, sonst saehe dasselbe
    /// Stueck Musik je nach Knotentyp anders aus.
    unsigned int m_isfWaveTex = 0;
    unsigned int m_isfFftTex = 0;
    /// Frame-Stempel der drei Audio-Texturen: JEDER Knoten, der Audio will,
    /// fordert sie an — ohne diesen Riegel laedt eine Kette mit drei solchen
    /// Knoten dieselben Daten dreimal je Frame hoch.
    std::uint64_t m_audioTexFrame = 0;
    bool m_audioTexAktuell = false;
    /// Baut alle drei Audio-Texturen aus EINER Rechnung — hoechstens einmal
    /// je Frame. `updateShadertoyAudioTexture` ruft sie ebenfalls.
    void aktualisiereAudioTexturen();

    // Multi Delay: 6 host-shared frame ring buffers (r_multidelay), cleared with
    // the runtimes. Input nodes fill them; output nodes read the delayed frame.
    std::vector<std::unique_ptr<QOpenGLFramebufferObject>> m_mdRing[6];
    int m_mdHead[6] = {0, 0, 0, 0, 0, 0};
    int m_mdW = 0;
    int m_mdH = 0;
    std::unique_ptr<QOpenGLVertexArrayObject> m_quadVao;
    std::unique_ptr<QOpenGLBuffer> m_quadVbo;
    std::unique_ptr<QOpenGLVertexArrayObject> m_warpVao;
    std::unique_ptr<QOpenGLBuffer> m_warpVbo;
    std::vector<float> m_warpVertices;  ///< reused CPU scratch for the warp mesh
    /// r_dmove-Gittertabelle als RGBA32I-Textur (x16, y16, a16, 0) + CPU-Scratch
    unsigned int m_warpTabTex = 0;
    std::vector<int> m_warpTab;
    // SuperScope 3D: dynamisches Sprite-Mesh (6 Vertices je Punkt:
    // center.xy + corner.xy + half.xy + rgb; je Frame neu hochgeladen)
    std::unique_ptr<QOpenGLVertexArrayObject> m_sprite3dVao;
    std::unique_ptr<QOpenGLBuffer> m_sprite3dVbo;
    std::vector<float> m_sprite3dVertices;  ///< CPU-Scratch (wie m_warpVertices)
    // Glow Orbs: geteilte Einheitskugel (16x12, Position = Normale)
    std::unique_ptr<QOpenGLVertexArrayObject> m_orbVao;
    std::unique_ptr<QOpenGLBuffer> m_orbVbo;
    std::unique_ptr<QOpenGLBuffer> m_orbIbo;
    int m_orbIndexCount = 0;
    // Flat-Fill-Dreiecke (my_triangle-Ersatz: Bass Spin Modus 1, S48):
    // dynamisches pos.xy-Mesh, Uniform-Farbe, Replace ohne Blend
    std::unique_ptr<QOpenGLVertexArrayObject> m_triVao;
    std::unique_ptr<QOpenGLBuffer> m_triVbo;
    /// GL_TRIANGLES aus NDC-Paaren (x,y je Vertex) in Uniform-Farbe zeichnen
    void drawFlatTriangles(const std::vector<float>& xyNdc, const QVector3D& color);
    // Gemeinsames Depth-RT der opaken 3D-Module (Etappe-2-Entscheid 1):
    // eigene Textur statt FBO-Attachment-Paar — ueberlebt das Farb-Ping-Pong.
    unsigned int m_depth3dTex = 0;  ///< GL_DEPTH_COMPONENT24 (onCleanup)
    int m_depth3dW = 0;
    int m_depth3dH = 0;
    bool m_depth3dCleared = false;  ///< je Frame einmal loeschen (onRender-Reset)

    std::unordered_map<uint64_t, LeafRuntime> m_leafRuntimes;
    std::vector<float> m_milkWaveScratch;  ///< interleavte Audio-Kopien (Meganode)
    std::vector<float> m_milkSpecScratch;
    lumi::render::OffscreenBufferPool m_bufferPool;  ///< 8 global buffers (Buffer Save)
    /// HG1: Buffer-Save-Slots des AKTIVEN Scopes — Root zeigt auf m_bufferPool,
    /// innerhalb einer Host-Gruppe auf deren GroupRuntime::pool (kein Leak
    /// zwischen Gruppen). renderHostGroup schaltet um (Stack-Disziplin).
    lumi::render::OffscreenBufferPool* m_activePool = nullptr;
    [[nodiscard]] lumi::render::OffscreenBufferPool& activePool()
    {
        return m_activePool != nullptr ? *m_activePool : m_bufferPool;
    }
    lumi::render::ScopeRenderer m_scopeRenderer;     ///< shared scope draw (E6)
    uint32_t m_rng = 0x9E3779B9u;  ///< host-local LCG state

    // Chain-scoped beat (design doc block 4; mutable within the frame)
    lumi::modules::BeatModule m_beat;
    lumi::modules::BeatEstimator m_beatEstimator{0};
    bool m_frameBeat = false;  ///< beat flag effects/list scripts may mutate
    int m_beatPeriodOverride = 0;  ///< >0: Beat alle N Frames (AvsRef --beat-period)
    /// Beat-Spur des musik-abgeleiteten Pruefsignals (S74); schlaegt die Periode
    const unsigned char* m_beatTrack = nullptr;
    int m_beatTrackLaenge = 0;
    int m_beatPeriodFrame = 0;     ///< Frame-Zaehler des Overrides (Reset beim Laden)
    int m_importRenderScaleDivisor = 1;  ///< Auto-Render-Scale beim AVS-Import (S47)
    /// App-Default Puffer-Wechsel (S66; Node-Einstellung AppEinstellung
    /// delegiert hierher — Behalten = Original-Semantik)
    lumi::multieffect::PufferWechsel m_milkPufferWechselDefault =
        lumi::multieffect::PufferWechsel::Behalten;
    double m_milkPufferFadingDefault = 0.5;  ///< Erbe-Anteil des App-Defaults
    double m_milkPufferAusblendSekDefault = 2.0;  ///< Ausblend-Dauer (s)
    bool m_milkSichtBlende = false;  ///< Sicht-Blende nach Saat (S67, QSettings)

    // Live render mode set by a Set Render Mode node for the following render
    // effects (AVS semantics). Reset at frame start; `set` means "override".
    struct RenderMode
    {
        bool set = false;    ///< a Set Render Mode node applied this frame
        int lineWidth = 1;   ///< line width for following scopes (px)
        /// AVS BLEND_LINE 0..9 (r_defs.h:267-283, S9). g_line_blend_mode
        /// starts as REPLACE (0), is reset per frame and saved/reset/restored
        /// around every list (r_list.cpp:693-694/744, S3).
        int lineBlend = 0;
        int alpha = 128;     ///< Adjustable-blend alpha 0..255
    };
    RenderMode m_renderMode;

    // 3D-Kamera-Zustand, gesetzt von einem camera3d-Knoten fuer die folgenden
    // 3D-Module (Lights-Etappe 1). Reset je Frame auf die Fallback-Kamera:
    // Position 0/0/+1/tan(fov/2), Blick auf den Ursprung (three.js-Konvention
    // wie Lights: x+ rechts, y+ oben, z+ zum Betrachter) — x/y in [-1,1] bei
    // z=0 fuellen das Bild (vertikal). Fog aus (start >= end).
    struct Camera3D
    {
        QVector3D pos{0.0f, 0.0f, 3.7320508f};
        QVector3D target{0.0f, 0.0f, 0.0f};
        float fovDeg = 30.0f;
        float rollDeg = 0.0f;
        float fogStart = 0.0f;   ///< Fog aktiv nur wenn fogEnd > fogStart
        float fogEnd = 0.0f;
        QVector3D fogColor{0.0f, 0.0f, 0.0f};
    };
    Camera3D m_camera3d;

    // AVS-layout visualisation data (spectrum L/R + waveform L/R, 576 each),
    // rebuilt once per frame and fed to every scripted engine (getspec/getosc).
    std::array<unsigned char, 576 * 4> m_visdata{};

    // Preset-local shared script state (decision §10.3)
    std::shared_ptr<lumi::scripting::ScriptContext> m_scriptContext;
    /// HG1: Kontext des AKTIVEN Scopes — Skript-Hosts werden gegen diesen
    /// erzeugt (reg/q/gmegabuf bleiben gruppen-lokal). Root = m_scriptContext.
    std::shared_ptr<lumi::scripting::ScriptContext> m_activeContext;
    [[nodiscard]] const std::shared_ptr<lumi::scripting::ScriptContext>& activeContext()
    {
        return m_activeContext ? m_activeContext : m_scriptContext;
    }

    // Frame-scoped inputs
    float m_time = 0.0f;       ///< seconds since init (DebugBars orbit)
    /// Uhrstand VOR dem Inkrement dieses Frames — das ist die Uhr, die
    /// EEL-`gettime()` sieht: AVS misst "Sekunden seit Start", der erste
    /// Frame liest 0. Mit `m_time` (schon inkrementiert) las er 1/60 — ein
    /// Preset mit 1s-FPS-Zaehlfenster (el-vis_hypno07) feuerte sein Fenster
    /// damit einen Frame zu frueh mit Zaehlerstand 0 und mass fps=0 (S59).
    /// DOUBLE, eigener Akkumulator: die float-Summe von m_time driftet nach
    /// zwei Dutzend Frames um eine Millisekunde — genau die Koernung, in der
    /// gettime() tickt.
    double m_scriptClock = 0.0;
    double m_scriptClockAccum = 0.0;  ///< Summe aller dt (double)
    float m_deltaTime = 0.0f;  ///< seconds since last frame (script modules)
    float m_audioLevel = 0.0f; ///< smoothed waveform RMS 0..1
};
