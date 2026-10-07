/**
 * @file Mute.hpp
 * @brief Composer: Mute fuer spontane Durchsagen — Mute-Profile und der
 *        Verlauf des Pegels (Vorgabe Patrik S101; Konzept §4)
 *
 * Ein Knopf senkt den Ton ab und holt ihn wieder. **Wie**, bestimmt das
 * Mute-Profil:
 *
 *  - die Kurve des Ausblendens (Dauer und Form),
 *  - die Kurve des Einblendens (Dauer und Form),
 *  - der Pegel, auf den abgesenkt wird — 0 bis 100 %, also auch „nur leiser".
 *
 * Profile sind Dateien (`.lvmute`), mitgelieferte und eigene, abgelegt wie die
 * Kurven-Presets (`FormPresets.hpp`):
 *
 * | mitgeliefert | eigene |
 * |---|---|
 * | `asset/composer/mutes/` | `<AppData>/composer/mutes/` |
 *
 * `MuteVerlauf` rechnet den Faktor ueber der Zeit: wird mitten in einer Blende
 * umgeschaltet, geht es von dem Pegel weiter, der gerade gilt — ohne Sprung.
 *
 * Kein Widget, kein Ton: wer den Faktor anwendet (Player, Mixer), holt ihn
 * hier ab.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`; dazu
 * der Ordner `asset/composer/mutes`.
 */
#pragma once

#include "composer/FormPresets.hpp"
#include "composer/Pegelform.hpp"

#include <QElapsedTimer>
#include <QJsonObject>
#include <QObject>
#include <QString>

#include <cstdint>
#include <functional>
#include <vector>

class IAudioPlayer;
class QTimer;

namespace lumi::composer
{

struct MuteProfil
{
    std::int64_t ausblendeMs = 400;  ///< vom vollen Pegel bis zum Zielpegel
    Pegelform ausblendeForm = einfacheBlende();
    std::int64_t einblendeMs = 800;  ///< vom Zielpegel zurueck auf voll
    Pegelform einblendeForm = einfacheBlende();
    double zielPegel = 0.0;  ///< 0…1 — worauf abgesenkt wird
    /// Sehen die Visuals die Absenkung (Entscheid Patrik S101, AM3: c)? Vorgabe
    /// nein: das Bild reagiert weiter auf die volle Musik.
    bool visualsFolgen = false;

    /// Dauern nicht negativ, Zielpegel 0…1, Formen als Blenden gerichtet.
    void richte();
};

[[nodiscard]] bool gleich(const MuteProfil& a, const MuteProfil& b);
[[nodiscard]] QJsonObject profilAlsJson(const MuteProfil& profil);
/// false = kein Mute-Profil; `profil` bleibt dann, wie es war.
bool profilAusJson(const QJsonObject& objekt, MuteProfil& profil);

/// Der Verlauf des Pegels. Die Zeit kommt vom Aufrufer (Millisekunden einer
/// stetigen Uhr) — so laesst er sich ohne Warten pruefen.
class MuteVerlauf
{
public:
    /// Gilt ab dem naechsten Umschalten; eine laufende Blende laeuft zu Ende.
    void setProfil(const MuteProfil& profil);
    [[nodiscard]] const MuteProfil& profil() const { return m_profil; }

    /// Umschalten: stumm = zum Zielpegel hin, sonst zurueck auf voll — vom
    /// Pegel aus, der bei `jetztMs` gilt. Mitten in einer Blende dauert der
    /// Rest entsprechend kuerzer.
    void schalte(bool stumm, std::int64_t jetztMs);
    [[nodiscard]] bool stumm() const { return m_stumm; }

    /// Der Faktor am Ton bei `jetztMs`: 1 = voll, Zielpegel = ganz abgesenkt.
    [[nodiscard]] double faktorBei(std::int64_t jetztMs) const;
    /// Laeuft bei `jetztMs` noch eine Blende?
    [[nodiscard]] bool blendet(std::int64_t jetztMs) const;

private:
    MuteProfil m_profil;
    MuteProfil m_laufend;  ///< das Profil der Blende, die gerade laeuft
    bool m_stumm = false;
    std::int64_t m_startMs = 0;
    std::int64_t m_dauerMs = 0;  ///< 0 = keine Blende, es gilt `m_ziel`
    double m_von = 1.0;
    double m_ziel = 1.0;
};

/**
 * Der EINE Mute der App (Entscheid Patrik S101, AM2: c): er gehoert dem
 * Player und gilt immer — ob die Playlist spielt oder der Composer den Ton
 * fuehrt. Jeder Knopf (Panel Player, Panel Composer) schaltet diesen Zustand.
 *
 * Der Schalter setzt den Faktor am Player (`IAudioPlayer::setPegel`, Naht
 * N17) — waehrend einer Blende in einem eigenen Takt. Der Composer liest ihn
 * dort je Takt mit der Lautstaerke. Lebt als Kind der QApplication.
 */
class MuteSchalter : public QObject
{
    Q_OBJECT

public:
    /// Der Schalter der App; beim ersten Aufruf angelegt (braucht eine
    /// QCoreApplication).
    [[nodiscard]] static MuteSchalter& instanz();
    /// Gibt es ihn schon? Wer nur lesen will, legt ihn nicht an.
    [[nodiscard]] static MuteSchalter* vorhanden();

    /// Wessen Ton geschaltet wird — einmal gesetzt (nullptr = keiner).
    void setPlayer(IAudioPlayer* player);

    /// Umschalten nach dem gewaehlten Profil (`muteprofile::gewaehltesProfil`).
    void schalte(bool stumm);
    [[nodiscard]] bool stumm() const { return m_verlauf.stumm(); }
    /// Das Profil wurde gewechselt oder geaendert: es gilt ab dem naechsten
    /// Umschalten.
    void ladeProfil();

    /// Der Faktor am Ton jetzt.
    [[nodiscard]] double faktor() const;
    /// Der Faktor fuer Spektrum und Wellenform der Visuals jetzt: 1, solange
    /// das Profil der laufenden Absenkung „Visuals follow" nicht traegt.
    [[nodiscard]] double faktorFuerVisuals() const;

    /// Test-Haken: die Zeit von aussen (Millisekunden); nullptr = die Uhr.
    void setUhrFuerTests(std::function<std::int64_t()> uhr);
    /// Den Takt einmal von Hand ausloesen (Test).
    void takt();

Q_SIGNALS:
    /// Der Zustand „stumm" hat gewechselt.
    void geschaltet(bool stumm);

private:
    explicit MuteSchalter(QObject* parent);
    [[nodiscard]] std::int64_t jetzt() const;

    MuteVerlauf m_verlauf;
    bool m_visualsFolgen = false;  ///< aus dem Profil der laufenden Absenkung
    IAudioPlayer* m_player = nullptr;
    QTimer* m_takt = nullptr;
    QElapsedTimer m_uhr;
    std::function<std::int64_t()> m_testUhr;
};

/// Mute-Profile als Dateien — derselbe Zuschnitt wie `formpresets`.
namespace muteprofile
{

using Eintrag = formpresets::Eintrag;

[[nodiscard]] const char* endung();  ///< `.lvmute`
/// Erst die mitgelieferten, dann die eigenen, je nach Namen sortiert; ein
/// eigenes mit dem Namen eines mitgelieferten verdraengt es.
[[nodiscard]] std::vector<Eintrag> liste();
[[nodiscard]] Eintrag finde(const QString& name);
[[nodiscard]] bool lade(const QString& pfad, MuteProfil& profil);
/// Als eigenes Profil speichern (Name bereinigt, gleichnamiges eigenes
/// ueberschrieben). false = ungueltiger Name oder Schreibfehler.
[[nodiscard]] bool speichere(const QString& name, const MuteProfil& profil, QString* pfad = nullptr);
/// Ein eigenes Profil loeschen. Mitgelieferte werden NICHT geloescht.
[[nodiscard]] bool loesche(const Eintrag& eintrag);

[[nodiscard]] QString benutzerOrdner();  ///< wird nicht angelegt
[[nodiscard]] QString assetOrdner();     ///< leer, wenn nicht gefunden

/// Das gewaehlte Profil ueber den Neustart (`composer/muteProfile`, der Name).
/// Fehlt es oder gibt es die Datei nicht mehr, gilt die Vorgabe von `MuteProfil`.
[[nodiscard]] QString gewaehlt();
void setGewaehlt(const QString& name);
[[nodiscard]] MuteProfil gewaehltesProfil();

/// Test-Haken: Unterordner `user/` und `asset/` unter `wurzel`; leer =
/// normales Verhalten.
void setzeWurzelFuerTests(const QString& wurzel);

}  // namespace muteprofile

}  // namespace lumi::composer
