/**
 * @file Sitzung.hpp
 * @brief Composer: die geladene Komposition eines Fensters und ihr Transport
 *        (docs/ui/Composer_Konzept.md §7, §8, Stufe C2b)
 *
 * **Geladen ist nicht an** (Patrik S95): die Sitzung haelt die Komposition,
 * auch wenn der Composer aus ist — dann liefert die Effektkette das Bild, und
 * im Panel laesst sich vorbereiten. **Composer on** setzt die Bildquelle
 * (Naht R3), aus nimmt sie weg; Komposition und Stelle bleiben.
 *
 * Die Sitzung ist die eine Stelle, ueber die Menue, Panel und Start den
 * Composer bedienen: Laden, Composer on, Play / Pause / Stopp / Sprung, Loop,
 * das Ende (einstellbar) und der Stand ueber den Neustart (Resume).
 *
 * Threads: lebt im GUI-Thread, als Kind des `VisualizerWidget`. Die Uhr liegt
 * in der Bildquelle und wird nur unter dem Render-Mutex angefasst; fuer die
 * Anzeige liest die Sitzung den Stand, den die Bildquelle je Bild spiegelt —
 * ohne zu warten.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Komposition.hpp"
#include "composer/Tonfuehrung.hpp"

#include <QObject>
#include <QString>
#include <QStringList>

#include <cstdint>
#include <utility>
#include <vector>

class IEventBus;
class MultiEffectVisualizer;
class QTimer;
class VisualizerWidget;

namespace lumi::composer
{

class Bildquelle;

/// Was am Ende der Komposition geschieht, wenn Loop aus ist (Entscheid Patrik
/// S95: alle drei waehlbar, Vorgabe Halten).
enum class AmEnde : int
{
    Halten = 0,       ///< die Uhr steht am Ende: schwarz, Karaoke laeuft, Composer bleibt an
    AnDenAnfang = 1,  ///< an den Anfang springen und halten, wie Stopp
    ComposerAus = 2,  ///< der Composer schaltet sich aus — die Effektkette liefert das Bild
};

class Sitzung : public QObject
{
    Q_OBJECT

public:
    ~Sitzung() override;

    /// Die Sitzung dieses Fensters; beim ersten Aufruf angelegt.
    [[nodiscard]] static Sitzung& fuer(VisualizerWidget& widget);
    /// Die Sitzung dieses Fensters, wenn es schon eine gibt.
    [[nodiscard]] static Sitzung* finde(const VisualizerWidget& widget);

    // --- Die Komposition im Panel -------------------------------------------

    /// Eine `.lvcomp` laden. Der Composer geht dabei aus, die Stelle an den
    /// Anfang. false = keine Komposition; es bleibt alles, wie es war.
    bool lade(const QString& pfad, QStringList* bericht = nullptr);
    /// Oeffnen von Hand (Menue): laden, Composer an und — ausser die
    /// Einstellung „haelt am Anfang" ist gesetzt — sofort abspielen.
    bool oeffne(const QString& pfad, QStringList* bericht = nullptr);
    /// Eine neue, leere Komposition (ein Kanal) ins Panel — noch ohne Datei:
    /// `datei()` ist leer, bis sie mit `speichereNach` einen Namen bekommt.
    /// Der Composer geht dabei aus.
    void neu();
    /// Composer aus und die Komposition weg.
    void entlade();

    [[nodiscard]] bool geladen() const { return m_geladen; }
    [[nodiscard]] const Komposition& komposition() const { return m_komposition; }
    [[nodiscard]] const QString& datei() const { return m_datei; }

    // --- Bearbeiten und Speichern (C3a) ---------------------------------------

    /// Die geladene Komposition durch eine geaenderte Fassung ersetzen. Datei,
    /// Composer on, Stelle und Lauf bleiben; laeuft sie gerade, sieht man die
    /// Aenderung sofort (die Renderer der Clips legen sich dafuer neu an).
    void setzeKomposition(Komposition neu);
    /// Weicht die Komposition von dem ab, was in der Datei steht?
    [[nodiscard]] bool ungespeichert() const;
    /// In die geladene Datei schreiben. false = nichts geladen oder Schreibfehler.
    bool speichere();
    /// In eine andere Datei schreiben; sie ist danach die geladene.
    bool speichereNach(const QString& pfad);
    /// Zaehlt Laden und Entladen — ein Undo-Schritt einer frueheren Ladung
    /// gilt nicht mehr.
    [[nodiscard]] int ladung() const { return m_ladung; }

    // --- Composer on ----------------------------------------------------------

    /// An: die Komposition liefert das Bild, an der gemerkten Stelle,
    /// angehalten. Aus: die Effektkette liefert es wieder. Ohne geladene
    /// Komposition bleibt der Composer aus.
    void setAn(bool an);
    [[nodiscard]] bool an() const { return quelle() != nullptr; }

    // --- Transport ------------------------------------------------------------

    /// Abspielen; schaltet den Composer ein, wenn er aus ist. Am Ende
    /// stehend: von vorn.
    void spiele();
    void pausiere();
    /// Anhalten und an den Anfang.
    void stoppe();
    /// An eine Stelle springen; ob die Uhr laeuft, bleibt. Geht auch bei
    /// ausgeschaltetem Composer — die Stelle gilt dann beim Einschalten.
    void springe(std::int64_t stelleMs);

    [[nodiscard]] bool laeuft() const;
    [[nodiscard]] std::int64_t stelleMs() const;
    [[nodiscard]] std::int64_t laengeMs() const { return m_geladen ? m_komposition.laenge() : 0; }

    /// Loop ueber die ganze Komposition; bleibt ueber den Neustart
    /// (`composer/loop`).
    void setSchleife(bool an);
    [[nodiscard]] bool schleife() const { return m_schleife; }

    // --- Sync on Effect Chain (C3c) -------------------------------------------
    //
    // Solange eine Komposition das Bild liefert, haengt Effect Chain (und
    // Visualizer Config) entweder am Renderer des angeklickten Clips — Sync
    // an — oder ruht. Gemeldet wird das ueber `VisualizerChangedEvent`, dem
    // beide Panels ohnehin folgen: keine Naht. Ist der Composer aus, haengen
    // sie wieder an der Kette der App.

    /// Der EventBus der App — einmal gesetzt (vom Composer-Panel). Ohne ihn
    /// bleibt Effect Chain an der Kette der App, wie vor C3c.
    static void setzeBus(IEventBus* bus);
    /// Schalter „Sync on Effect Chain"; bleibt ueber den Neustart
    /// (`composer/syncEffectChain`).
    void setSync(bool an);
    [[nodiscard]] bool sync() const { return m_sync; }
    /// Der im Composer angeklickte Clip, (-1, -1) = keiner.
    void waehleClip(int kanal, int clip);

    /// Woran Effect Chain gerade haengt.
    enum class Bindung
    {
        Kette,  ///< an der Kette der App (Composer aus — oder kein Bus)
        Ruht,   ///< an nichts
        Clip,   ///< am Renderer des angeklickten Clips
    };
    [[nodiscard]] Bindung bindung() const { return m_bindung; }
    /// Der Renderer, an dem Effect Chain haengt (nur bei `Bindung::Clip`).
    [[nodiscard]] MultiEffectVisualizer* gebundenerRenderer() const { return m_gebunden; }

    // --- Die Kette eines Clips speichern (C3c Schritt 2, Entscheid Patrik S95) --
    //
    // Immer als Speichern unter: das Original bleibt unberuehrt, der Clip
    // zeigt danach auf die neue Datei.

    /// Die Preset-Datei des Clips, an dem Effect Chain haengt (leer = keiner).
    [[nodiscard]] QString gebundeneDatei() const;
    /// Hat die Kette dieses Clips eine Host-Gruppe? (bestimmt die Endung)
    [[nodiscard]] bool gebundeneKetteHatHostGruppe() const;
    /// Die anderen Clips der Komposition, die dieselbe Datei benutzen wie der
    /// gebundene — als (Kanal, Clip).
    [[nodiscard]] std::vector<std::pair<int, int>> andereClipsMitDerDatei() const;
    /// Die Kette des gebundenen Clips nach `pfad` schreiben; er und die Clips
    /// in `auchDiese` zeigen danach auf diese Datei. false = kein Clip
    /// gebunden, `pfad` ist das Original, oder Schreibfehler — dann aendert
    /// sich nichts.
    bool speichereClipKette(const QString& pfad,
                            const std::vector<std::pair<int, int>>& auchDiese = {});

    // --- Einstellungen (QSettings unter `composer/…`) -------------------------

    /// Resume after restart (Vorgabe aus).
    [[nodiscard]] static bool resumeAn();
    static void setResume(bool an);
    /// Open Composition haelt am Anfang, statt sofort zu spielen (Vorgabe aus).
    [[nodiscard]] static bool oeffnenHaelt();
    static void setOeffnenHaelt(bool haelt);
    [[nodiscard]] static AmEnde amEnde();
    static void setAmEnde(AmEnde art);

    // --- Ueber den Neustart (Konzept §8 „Der Start") --------------------------

    /// Datei, „Composer an", Stelle und „lief gerade" festhalten — immer, auch
    /// ohne Komposition, damit kein frueherer Stand stehen bleibt.
    void sichereStand() const;
    /// Beim Start: die zuletzt geladene Komposition wieder ins Panel; mit
    /// Resume auch ihr Stand. false = es gab keine (oder die Datei fehlt).
    bool stelleWiederHer(QStringList* bericht = nullptr);

    /// Die Messung der laufenden Komposition samt Anlauf je Clip ins Log.
    void protokolliereMessung();

    // --- Der Audiokanal (C5) ---------------------------------------------------
    //
    // Ist der Composer an und hat die Komposition einen eingeschalteten
    // Audio-Kanal, fuehrt die Sitzung den Player der App (`Tonfuehrung`): sie
    // laedt und spielt die Songs der Ton-Clips, die Playlist ruht, und solange
    // ein Song spielt, fuehrt der Ton die Uhr. Den Player holt sie sich aus den
    // Diensten des Fensters (Naht N7b) — ohne Player bleibt alles, wie es war.

    /// Fuehrt diese Sitzung gerade den Player?
    [[nodiscard]] bool fuehrtTon() const { return m_ton.fuehrt(); }
    /// Die Sitzung, die den Player fuehrt — es gibt nur einen Player, also
    /// hoechstens eine. nullptr = keine.
    [[nodiscard]] static Sitzung* fuehrende();
    /// Zum Anfang des naechsten (`vor`) bzw. vorigen Songs der Komposition
    /// springen. false = es gibt keinen.
    bool springeZumSong(bool vor);

Q_SIGNALS:
    /// Eine andere Komposition ist geladen — oder keine mehr.
    void kompositionGeaendert();
    /// Die geladene Komposition wurde bearbeitet (auch: Undo, Redo).
    void kompositionBearbeitet();
    /// Komposition, Composer on, Loop, Transport oder der Stand „gespeichert"
    /// haben sich geaendert.
    void geaendert();
    /// Solange der Composer an ist: etwa 30-mal je Sekunde — die Stelle neu lesen.
    void takt();

private:
    explicit Sitzung(VisualizerWidget& widget);

    [[nodiscard]] Bildquelle* quelle() const;
    void beimTakt();
    /// Effect Chain dorthin haengen, wohin sie nach Composer on, Sync und
    /// Auswahl gehoert. Was noch nicht geht (der Render-Thread hat den Wunsch
    /// noch nicht verarbeitet), holt der Takt nach.
    void bindeKette();
    /// Effect Chain zurueck an die Kette der App — vor allem, was die
    /// Bildquelle wegnimmt.
    void loeseKette();
    void meldeKette();
    void meldeNichts();
    /// Je Takt: den Player uebernehmen oder freigeben, die Fuehrung takten und
    /// die Uhr nachstellen, wenn der Ton fuehrt.
    void fuehreTon(Bildquelle& q);
    /// Den Player freigeben — die Playlist haengt wieder.
    void gibTonFrei();

    Tonfuehrung m_ton;

    bool m_sync = false;
    int m_wahlKanal = -1;
    int m_wahlClip = -1;
    Bindung m_bindung = Bindung::Kette;
    MultiEffectVisualizer* m_gebunden = nullptr;
    bool m_bindungOffen = false;  ///< der Takt soll es noch einmal versuchen
    int m_festStand = -1;

    VisualizerWidget& m_widget;
    Komposition m_komposition;
    Komposition m_gespeichert;  ///< was in der Datei steht
    QString m_datei;
    int m_ladung = 0;
    bool m_geladen = false;
    bool m_schleife = false;
    std::int64_t m_stelleMs = 0;  ///< die Stelle, solange der Composer aus ist
    QTimer* m_takt = nullptr;
};

}  // namespace lumi::composer
