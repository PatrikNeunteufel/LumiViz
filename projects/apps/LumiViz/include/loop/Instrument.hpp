/**
 * @file Instrument.hpp
 * @brief Loop: gerechnete Instrumente — aus einem Satz Einstellungen und einer
 *        Tonhoehe wird ein Ton (docs/audio/Loop_Konzept.md §4f, Stufe 9,
 *        Entscheid I1 c)
 *
 * Ein Instrument besteht aus einer bis vier Schichten (Stufe 10c); jede ist
 * ein Satz Einstellungen fuer einen Klangerzeuger:
 * Schwingung (der Funktionsgenerator), Huellkurve, Filter, Vibrato,
 * Verstimmung — dazu die gezupfte Saite als eigene Art. `rendereTon` rechnet
 * daraus einen Ton von gegebener Dauer **vorab** in einen `Klang`; der
 * Rechenkern spielt ihn wie den Schlag eines Drumkits. So bleibt der Kern
 * ohne Gedaechtnis, obwohl Filter und Saite eines haben.
 *
 * Derselbe Aufruf ergibt dieselben Werte: kein Zufall von aussen (das
 * Rauschen der Saite kommt aus einer festen Folge je Tonhoehe).
 *
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/Rechenkern.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// Die Schwingung eines Instruments. Die vier ersten sind ein Wellenzug ohne
/// Teiltoene ueber der halben Abtastrate; die Saite ist eine gezupfte Saite
/// (Karplus-Strong), die von selbst ausklingt. Das Sample ist eine Aufnahme,
/// je Note schneller oder langsamer abgespielt (Stufe 11a). Das Rauschen hat
/// keine Tonhoehe: eine feste Folge, bei jeder Note dieselbe (Stufe 11d).
enum class Schwingung : int
{
    Sinus = 0,
    Dreieck,
    Saege,
    Rechteck,
    Saite,
    Sample,
    Rauschen,
};

/// Pitch sweep (Stufe 11d, Entscheid N1): so weit neben ihrer Tonhoehe darf
/// eine Note beginnen, in Halbtoenen — darueber (sie faellt) oder darunter
/// (sie steigt) — und so lang darf der Weg dauern.
inline constexpr double kTonhoeheStartMin = -24.0;
inline constexpr double kTonhoeheStartMax = 48.0;
inline constexpr double kTonhoeheZeitMaxMs = 2000.0;

/// Die Art des Filters einer Schicht (Stufe 11e; Skizze gebilligt, Patrik
/// S116). Der Tiefpass laesst durch, was unter der Grenze liegt, der Hochpass,
/// was darueber liegt, der Bandpass, was um sie herum liegt.
enum class FilterArt : int
{
    Tiefpass = 0,
    Hochpass,
    Bandpass,
};

/// Eine Schicht eines Instruments: ein voller Satz Einstellungen fuer den
/// Klangerzeuger, dazu was nur die Schicht betrifft (Stufe 10c; Skizze
/// gebilligt, Patrik S111). Die Schichten eines Instruments werden beim
/// Rechnen eines Tons addiert.
struct Schicht
{
    QString name;                ///< wie sie in der Liste steht; leer = „Layer n"
    bool an = true;              ///< aus = stumm, bleibt aber im Instrument
    int versatzHalbtoene = 0;    ///< -24…24: so weit liegt die Schicht neben der Note
    double einsatzMs = 0.0;      ///< 0…1000: so lange nach dem Anschlag setzt sie ein (kein Echo)
    double verzerrung = 0.0;     ///< 0…1: der Verzerrer (Drive), 0 = aus (Entscheid E1 a)

    Schwingung schwingung = Schwingung::Sinus;
    /// Nur Sinus: die Anteile des 2., 3., … Teiltons (der Grundton zaehlt 1).
    std::vector<double> obertoene;
    int stimmen = 1;               ///< 1…3 gleiche Schwingungen uebereinander
    double verstimmungCent = 0.0;  ///< so weit liegen die aeusseren Stimmen neben dem Ton

    // --- Huellkurve ---
    double anstiegMs = 5.0;    ///< bis zur vollen Lautstaerke
    double abfallMs = 200.0;   ///< danach bis auf `halten`
    double halten = 0.7;       ///< 0…1, solange die Note dauert
    double ausklangMs = 150.0; ///< nach dem Ende der Note

    // --- Filter (Tiefpass, folgt der Tonhoehe) ---
    double filterTeilton = 0.0; ///< Grenze als Vielfaches des Grundtons; 0 = offen
    double filterHuelle = 0.0;  ///< so viele Vielfache oeffnet die Huellkurve dazu

    // --- Vibrato ---
    double vibratoHz = 0.0;
    double vibratoCent = 0.0;
    double vibratoEinsatzMs = 0.0;  ///< so lange waechst es an

    double saiteDaempfung = 0.996;  ///< nur Saite: 0.9…0.9999, groesser klingt laenger
    double laut = 1.0;              ///< die Lautstaerke der Schicht

    // --- nur Sample (Stufe 11a; Skizze gebilligt, Patrik S115) ---
    /// Die Aufnahme: ein eigenes Sample (`user:<name>`, Sample.hpp) oder der
    /// Pfad einer Tondatei; leer = keine, die Schicht schweigt.
    QString sample;
    int grundton = 60;              ///< Root note: dieser Ton ist die Aufnahme (0…127)
    double feinCent = 0.0;          ///< Fine tune, -100…100: so weit liegt die Aufnahme ueber dem Grundton
    double sampleStartMs = 0.0;     ///< Sample start: ab hier in der Aufnahme
    bool schleife = false;          ///< Sustain loop: ein Stueck wiederholt sich, solange die Note dauert
    double schleifeVonMs = 0.0;     ///< Loop from, in der Aufnahme
    double schleifeBisMs = 0.0;     ///< Loop to; nicht groesser als von = keine Schleife

    // --- Pitch sweep (Stufe 11d; Entscheide N1 und N2, Patrik S116) ---
    // Fuer Sinus, Dreieck, Saege, Rechteck und Sample; Saite und Rauschen kennen ihn nicht.
    /// So viele Halbtoene neben ihrer Tonhoehe beginnt jede Note: groesser als
    /// null darueber (sie faellt auf den Ton), kleiner darunter (sie steigt);
    /// 0 = aus. `kTonhoeheStartMin` … `kTonhoeheStartMax`.
    double tonhoeheStartHt = 0.0;
    /// Nach dieser Zeit ist die Tonhoehe angekommen (bis auf ein Zwanzigstel
    /// des Wegs, wie Decay); 0 = kein Weg, die Note beginnt auf ihrem Ton.
    double tonhoeheZeitMs = 100.0;

    // --- Art und Resonanz des Filters (Stufe 11e; Entscheid H1 b, Patrik S116) ---
    FilterArt filterArt = FilterArt::Tiefpass;
    /// Resonance, 0…1: die Betonung an der Grenze; beim Bandpass die Breite
    /// (mehr = schmaler). Ein Tiefpass mit 0 ist der Filter bis Stufe 11d,
    /// unveraendert — darueber rechnet er wie Hochpass und Bandpass.
    double filterResonanz = 0.0;

    [[nodiscard]] bool operator==(const Schicht&) const = default;
};

/// So viele Schichten traegt ein Instrument hoechstens: jede wird je Ton
/// einzeln gerechnet.
inline constexpr int kSchichtenMax = 4;

/// Eine Art von Instrument: so ist das Fenster „Instruments" gegliedert
/// (Stufe 14, Entscheid P1 a; Skizze gebilligt, Patrik S114).
struct InstrumentArt
{
    const char* schluessel;  ///< steht in der Datei (`category`)
    const char* titel;       ///< wie es im Programm steht
};

/// Die neun Arten in der Reihenfolge des Fensters (seit S115 mit „Drums",
/// Entscheid K3 a); die letzte ist „Unusual".
[[nodiscard]] const std::vector<InstrumentArt>& instrumentArten();
/// Der Schluessel einer Art — oder `unusual`, wenn es `art` nicht gibt (auch leer).
[[nodiscard]] QString gueltigeArt(const QString& art);
/// Der Titel einer Art („Pads and strings").
[[nodiscard]] QString artTitel(const QString& art);

struct Instrument
{
    /// Wie ein Muster es nennt: der Name eines mitgelieferten Instruments
    /// (`piano` — die Datei `asset/loop/instruments/piano.lvinst`), ein
    /// eigenes aus dem Benutzerordner (`user:mein_bass`, Entscheid P2 a) oder
    /// der Pfad einer Datei `.lvinst` an freiem Ort. Steht nicht in der Datei.
    QString name;
    QString titel;      ///< wie es im Programm steht
    double laut = 1.0;  ///< die Lautstaerke des ganzen Instruments
    /// Mindestens eine, hoechstens `kSchichtenMax`.
    std::vector<Schicht> schichten = {Schicht{}};
    // --- seit Fassung 3 (Entscheid F3 b): fuer das Fenster „Instruments"
    /// Die Art (`keys`, `bass`, …). Leer oder unbekannt gilt als `unusual` —
    /// in der Datei bleibt stehen, was dort stand.
    QString art;
    /// Wo es hingehoert („Funk", „Disco"); leer = nur unter „All".
    QStringList genres;
    QString beschreibung;  ///< ein Satz

    [[nodiscard]] bool operator==(const Instrument&) const = default;
};

// --- die Datei `.lvinst` (Entscheide Patrik S110, I2 b und I3 a; S111, F1 a) --
// Ein Instrument ist eine Datei, abgelegt wie die Klaenge des Kits: die
// mitgelieferten unter `asset/loop/instruments`, eigene irgendwo. Im Programm
// steht kein Instrument mehr.
//
// Fassung 2 traegt die Schichten als Liste `layers`; Fassung 1 ist flach und
// wird als Instrument mit einer Schicht gelesen. Fassung 3 (S114, Entscheid
// F3 b) traegt dazu `category`, `genres` und `description`; alle drei
// duerfen fehlen. Fassung 4 (S115, Stufe 11a) kennt die Schwingung `sample`
// mit `sample`, `rootNote`, `fineTune`, `sampleStart`, `sustainLoop`,
// `loopFrom`, `loopTo` — eine eigene Fassung, weil ein aelteres Programm eine
// solche Schicht sonst still als Sinus spielte. Fassung 5 (S116, Stufe 11d)
// kennt die Schwingung `noise` und an jeder Schicht `pitchStart` (Halbtoene)
// und `pitchTime` (ms) — aus demselben Grund eine eigene Fassung: ein aelteres
// Programm spielte Rauschen als Sinus und einen fallenden Ton gerade.
// Fassung 6 (S116, Stufe 11e) kennt an jeder Schicht `filterType` (`lowpass`,
// `highpass`, `bandpass`) und `filterResonance` — ein aelteres Programm
// spielte einen Hochpass als Tiefpass. Geschrieben wird Fassung 6.

/// Die Fassung, die dieses Programm schreibt; eine hoehere liest es nicht.
inline constexpr int kInstrumentFassung = 6;

/// Das Instrument als JSON (ohne `name` — der ist, wie das Muster es nennt).
[[nodiscard]] QJsonObject alsJson(const Instrument& instrument);
/// Liest ein Instrument, Werte in ihren Grenzen. false = kein Instrument
/// (falsches Format) oder eine Fassung, die dieses Programm nicht kennt — dann
/// ist `zuNeu` gesetzt; `instrument` bleibt, wie es war. `name` bleibt.
bool ausJson(const QJsonObject& wurzel, Instrument& instrument, bool* zuNeu = nullptr);
bool speichereInstrument(const Instrument& instrument, const QString& pfad);
/// Liest die Datei; `name` wird der Pfad. Ohne Titel in der Datei gilt ihr Name.
bool ladeInstrument(const QString& pfad, Instrument& instrument, bool* zuNeu = nullptr);

/// Der Ordner der mitgelieferten Instrumente: `asset/loop/instruments`, neben
/// dem Kit (`kitOrdner`). Leer = nicht gefunden.
[[nodiscard]] QString instrumentOrdner();
/// Die mitgelieferten Instrumente — was im Ordner als `.lvinst` liegt, nach
/// Titel sortiert; `name` ist der Dateiname ohne Endung. Einmal gelesen und
/// gemerkt. Leer = der Ordner ist nicht da.
[[nodiscard]] const std::vector<Instrument>& instrumente();
/// Das mitgelieferte Instrument mit diesem Namen; nullptr = keins.
[[nodiscard]] const Instrument* findeInstrument(const QString& name);
/// Das Instrument, das ein Muster nennt: ein mitgeliefertes beim Namen, ein
/// eigenes als `user:<name>` oder eine Datei beim Pfad (`istInstrumentDatei`).
/// false = nicht gefunden oder nicht lesbar; `zuNeu` = die Datei traegt eine
/// Fassung, die dieses Programm nicht kennt.
bool holeInstrument(const QString& nennung, Instrument& instrument, bool* zuNeu = nullptr);

// --- eigene Instrumente im Benutzerordner (Entscheid P2 a, Patrik S114) -----
// Ein eigenes Instrument liegt als `<name>.lvinst` im Benutzerordner der App
// unter `instruments`, und ein Muster nennt es als `user:<name>`. Das ist ein
// eigener Raum: ein eigenes `piano` verdraengt das mitgelieferte nicht, ein
// Loop klingt weiter, wie er gespeichert wurde. Eine Datei an freiem Ort
// steht wie bisher beim Pfad im Muster.

inline constexpr const char* kEigenVorsatz = "user:";

/// Nennt `nennung` ein eigenes Instrument (`user:<name>`)?
[[nodiscard]] bool istEigenesInstrument(const QString& nennung);
/// Der Ordner der eigenen Instrumente (wird nicht angelegt); leer = Qt nennt
/// keinen Benutzerordner.
[[nodiscard]] QString eigenerInstrumentOrdner();
/// Nur fuer Tests: ein anderer Ordner; leer = wieder der echte.
void setzeEigenenInstrumentOrdnerFuerTests(const QString& ordner);
/// Aus einem Titel der Name einer eigenen Datei: klein, `a`–`z`, Ziffern und
/// `_` („Mein Bass 2" -> `mein_bass_2`). Leer = daraus wird kein Name.
[[nodiscard]] QString eigenerName(const QString& titel);
/// Ist `name` ein Name, wie `eigenerName` ihn bildet? Nur solche werden im
/// Benutzerordner gesucht — ein Muster kann so nichts ausserhalb nennen.
[[nodiscard]] bool istEigenerName(const QString& name);
/// Die Datei eines eigenen Instruments; leer = kein gueltiger Name oder kein Ordner.
[[nodiscard]] QString eigeneDatei(const QString& name);
/// Die eigenen Instrumente, nach Titel sortiert; `name` ist `user:<name>`.
/// Jedes Mal neu gelesen — der Ordner aendert sich, waehrend das Programm laeuft.
[[nodiscard]] std::vector<Instrument> eigeneInstrumente();
/// Ein eigenes Instrument in den Benutzerordner schreiben (legt ihn an).
/// `instrument.name` wird `user:<name>`. false = kein gueltiger Name oder
/// nicht zu schreiben.
bool speichereEigenes(Instrument& instrument, const QString& name);
/// Ein eigenes Instrument loeschen. false = es gab keins.
bool loescheEigenes(const QString& name);

/// Jedes Genre, das ein mitgeliefertes oder eigenes Instrument traegt — einmal,
/// nach dem Alphabet. Fuer den Filter im Fenster und die Vorschlaege im Creator.
[[nodiscard]] QStringList bekannteGenres();
/// Zaehlt jede gespeicherte Instrument-Datei (`speichereInstrument`): wer
/// Toene gerechnet hat und sich die Zahl merkt, sieht, ob er neu rechnen muss.
[[nodiscard]] std::uint64_t instrumentFassung();
/// Weiterzaehlen, ohne ein Instrument zu schreiben: ein Sample ist dazugekommen,
/// ersetzt oder geloescht (Sample.hpp) — Toene, die es spielen, sind neu zu rechnen.
void zaehleInstrumentFassung();

/// So viele Obertoene traegt ein Instrument hoechstens (der 2. bis 17. Teilton).
inline constexpr int kObertoeneMax = 16;

/// Die Frequenz einer Tonhoehe (MIDI-Nummer): 69 = 440 Hz.
[[nodiscard]] double tonHz(int hoehe);
/// Der Name einer Tonhoehe: 60 = „C4", 61 = „C#4".
[[nodiscard]] QString tonName(int hoehe);

/// Ein Ton laenger als das wird abgeschnitten (wie ein Sample, `kKlangMaxSekunden`).
inline constexpr double kTonMaxSekunden = 30.0;

/// Einen Ton rechnen: `dauerMs` lang gehalten, danach der Ausklang des
/// Instruments; der Klang endet bei null. Stereo, beide Seiten gleich.
/// Die eingeschalteten Schichten werden addiert — jede an ihrer Tonhoehe und
/// ab ihrem Einsatz; der Klang ist so lang wie die laengste. Ist keine an,
/// ist er still. Leer = `rate` oder `dauerMs` nicht groesser als 0.
/// Eine Sample-Schicht spielt, was zu ihrer Aufnahme gemerkt ist
/// (`sampleDaten`, Sample.hpp) — ist nichts gemerkt, schweigt sie. Sind
/// Sample-Schichten dabei, koennen die beiden Seiten verschieden sein.
[[nodiscard]] Klang rendereTon(const Instrument& instrument, int hoehe, double dauerMs, int rate);

}  // namespace lumi::loop
