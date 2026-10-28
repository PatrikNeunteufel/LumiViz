/**
 * @file Loop.hpp
 * @brief Loop: das Datenmodell eines Loops und die Datei `.lvloop`
 *        (docs/audio/Loop_Konzept.md §4)
 *
 * Ein Loop ist ein **Arrangement in Takten** (Entscheide Patrik S108, L3 neu
 * und L8 a): Kanaele, in den Kanaelen Clips. Ein Clip spielt ein **Muster**
 * (die „Basis") und wiederholt es; mehrere Clips koennen dasselbe Muster
 * spielen — wer das Muster aendert, aendert sie alle.
 *
 * Ein Muster traegt alles, was sein Editor einstellt: bei einem Drumkit die
 * Spuren (das Kit) und je Spur die Schritte. Eine Spur verweist auf einen
 * Klang: einen Namen aus dem mitgelieferten Kit (`kit:<Name>`) oder eine
 * Sample-Datei — eingebettet wird nichts. Bei einem Instrument (Stufe 9,
 * Entscheid I1 c) sind es das Instrument und die Noten.
 *
 * Tempo und Raster stehen im Loop und gelten fuer alles darin (L2 b). Die
 * Datei `.lvloop` ist das ganze Arrangement. Kein Qt-Widget, keine
 * Audio-Engine: reines Modell.
 */
#pragma once

#include "loop/Bausteine.hpp"
#include "loop/Auskunft.hpp"  // InstrumentVon (HT5 a); bringt Saiten.hpp mit
#include "loop/Zeitspur.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <map>
#include <utility>
#include <variant>
#include <vector>

namespace lumi::loop
{

inline constexpr double kBpmMin = 20.0;
inline constexpr double kBpmMax = 400.0;
inline constexpr int kRasterMax = 16;  ///< hoechstens so viele Schlaege je Takt und Schritte je Schlag
/// Ein Muster ist mindestens so lang (Entscheid Patrik B1 a, S124: zum
/// Probieren ein Takt; bis S124 war es 4).
inline constexpr int kTakteMin = 1;
/// So lang beginnt ein neues Muster — und ein neuer Clip (Vorgabe Patrik S108,
/// bestaetigt mit B1 a).
inline constexpr int kTakteNeu = 4;
inline constexpr int kTakteMax = 64;   ///< hoechstens so viele Takte je Muster

/// Der Vorsatz eines Klangs aus dem mitgelieferten Kit: `kit:kick`.
[[nodiscard]] bool istKitKlang(const QString& klang);
/// Der Vorsatz eines mitgelieferten Samples: `lib:voice_female_ah` — die Datei
/// `asset/loop/samples/voice_female_ah.wav` (Entscheid 1 a, Patrik S123).
[[nodiscard]] bool istBibliotheksKlang(const QString& klang);
/// Mitgeliefert und schreibgeschuetzt: ein Klang des Kits oder ein
/// mitgeliefertes Sample. Eine Datei nennt ihn beim Namen, nie als Pfad.
[[nodiscard]] bool istMitgelieferterKlang(const QString& klang);

/// Ein Pfad, wie ihn eine Datei des Loops traegt: relativ zu `basisOrdner`,
/// wenn die Datei darunter liegt, sonst absolut (wie in der `.lvcomp`).
[[nodiscard]] QString pfadFuerDatei(const QString& datei, const QString& basisOrdner);
/// Der Rueckweg: ein relativer Eintrag gilt ab `basisOrdner`.
[[nodiscard]] QString pfadAusDatei(const QString& eintrag, const QString& basisOrdner);
/// Der Vorsatz eines eigenen Samples aus dem Benutzerordner: `user:table_knock`
/// (Sample.hpp, Entscheid S1 a).
[[nodiscard]] bool istEigenerKlang(const QString& klang);

/// So lang wird der Ton einer Spur gehalten, die ein Instrument spielt: die
/// Vorgabe und die Grenzen, in Millisekunden (Entscheid K2 a, Patrik S115).
inline constexpr int kSpurDauerMs = 250;
inline constexpr int kSpurDauerMinMs = 1;
inline constexpr int kSpurDauerMaxMs = 30000;

/// Eine Spur eines Drumkits: ein Klang mit Lautstaerke und Panorama.
/// Seit Fassung 4 (Stufe 11c, Entscheid K1 a) hat sie drei Arten von Klang: einen
/// aus dem Kit, ein eigenes Sample — oder ein Instrument, das bei jedem
/// Schlag einen Ton spielt, immer dieselbe Note, immer gleich lang.
struct Spur
{
    QString name;
    /// `kit:<Name>`, `user:<name>` (ein eigenes Sample) oder der Pfad einer
    /// Sample-Datei (absolut); leer = kein Klang. Zaehlt nicht, solange
    /// `instrument` gesetzt ist — bleibt aber stehen.
    QString klang;
    double laut = 1.0;   ///< 0…1
    double pan = 0.0;    ///< -1 (links) … 1 (rechts)
    bool stumm = false;
    /// Das Instrument, das die Spur spielt — genannt wie in einem Muster: ein
    /// mitgeliefertes beim Namen, ein eigenes als `user:<name>`, eine Datei
    /// beim Pfad. Leer = die Spur spielt `klang`.
    QString instrument;
    int hoehe = 60;                ///< Note: die Tonhoehe jedes Schlags (MIDI-Nummer, 0…127)
    int dauerMs = kSpurDauerMs;    ///< Length: so lang wird der Ton gehalten, danach klingt er aus
    /// Die Rolle im Kit (`loop/DrumKit.hpp`, Entscheide K1 a, K5 a, Patrik S123):
    /// „kick", „tom 2", „perc 1" … Leer = noch keine; ein Kitwechsel raet sie dann.
    /// In der Datei das freiwillige Feld `role` (keine neue Fassung).
    QString rolle;

    Spur() = default;
    /// Eine Spur mit einem Klang — wie sie bis Fassung 3 ganz beschrieben war.
    Spur(QString nameDerSpur, QString klangDerSpur, double lautstaerke = 1.0, double panorama = 0.0, bool istStumm = false)
        : name(std::move(nameDerSpur)), klang(std::move(klangDerSpur)), laut(lautstaerke), pan(panorama), stumm(istStumm)
    {
    }

    /// Was der Rechenkern fuer diese Spur spielt: der Klang — oder der Name
    /// des Tons ihres Instruments (`tonKlang`). Leer = nichts.
    [[nodiscard]] QString spielt() const;

    [[nodiscard]] bool operator==(const Spur&) const = default;
};

/// Die Art eines Musters — sie bestimmt seinen Editor: ein Drumkit (Spuren und
/// Schritte) oder ein Instrument (Noten mit Tonhoehe und Laenge, Stufe 9).
enum class MusterArt : int
{
    Drumkit = 0,
    Instrument = 1,
};

/// Die Tonhoehen einer Note, als MIDI-Nummer: A0 bis C8, der Umfang eines
/// Klaviers (60 = C4, 69 = A4 mit 440 Hz). Bis Stufe 22 C1 bis C7; erweitert
/// mit Stufe 23a (Entscheid Patrik S-T1 a, S132) fuer die tiefe B-Saite eines
/// fuenfsaitigen Basses (B0) und die hohen Lagen einer Violine (bis A7).
inline constexpr int kHoeheMin = 21;
inline constexpr int kHoeheMax = 108;

/// Eine Note eines Instrument-Musters: ab einem Schritt, so viele Schritte
/// lang, in einer Tonhoehe.
struct Note
{
    int schritt = 0;    ///< 0 = der erste Schritt des Musters
    int laenge = 1;     ///< in Schritten, mindestens 1
    int hoehe = 60;     ///< MIDI-Nummer, `kHoeheMin` … `kHoeheMax`
    double laut = 1.0;  ///< 0…1 (groesser als 0 — eine Note ohne Lautstaerke gibt es nicht)
    /// Die Saite, auf der die Note gespielt wird (Stufe 23a, Entscheid S-M2 a):
    /// die Stelle in `Saitensatz::leer` des Instruments, 0 … `kSaitenMax` − 1;
    /// −1 = keine — dann waehlt das Programm sie nach der Regel des Musters.
    /// Die Piano-Roll setzt sie nicht von selbst, die Tabs immer. Ob die Note
    /// auf dieser Saite greifbar ist, weiss erst, wer das Instrument kennt.
    int saite = -1;
    /// Die Artikulationen (Stufe 24a, S-W4 a): Bits aus `Artikulation`
    /// (Spielweise.hpp), 0 = keine. Wirken nur bei einem Instrument mit Saiten.
    int artikulationen = 0;
    /// Der Bend (Stufe 25a, S-L2 a): die Kurve der Biegung ueber dem gegriffenen
    /// Ton, leer = keiner. Wirkt nur bei einem Instrument mit Saiten.
    /// Mit Vorgabe `{}`: so meldet clang bei `Note{schritt, laenge, hoehe}` kein
    /// fehlendes Feld (anders als bei `Saitensatz::strum`).
    std::vector<BendPunkt> bend{};
    /// Die Stimme (Stufe 31d-9, HD2 a, HD2-1 a): 0 = keine, 1 … `kStimmenMax`; ihr Name
    /// steht am Muster (`Muster::stimmen`). Wirkt nicht auf den Klang — nur auf Anzeige
    /// und Bearbeitung (etwa die Hand am Fluegel).
    int stimme = 0;
    /// Versatz des Anfangs in Bruchteilen eines Schritts (31d-9, N1 a), 0 … < 1: die Note
    /// schlaegt bei `schritt + versatzAnfang` an. 0 = auf dem Raster.
    double versatzAnfang = 0.0;
    /// Versatz des Endes in Bruchteilen eines Schritts (N1 a), > −1 … 0: die Note klingt
    /// `laenge + versatzEnde` Schritte lang — so auch kuerzer als ein Schritt (ein Vorschlag).
    double versatzEnde = 0.0;

    [[nodiscard]] bool operator==(const Note&) const = default;
};

/// So viele Stimmen traegt ein Muster hoechstens (HD2-1 a: die Kanaele von MIDI).
inline constexpr int kStimmenMax = 16;
/// So weit reicht ein Versatz hoechstens (N1 a): knapp unter einem Schritt — der
/// Anfang bleibt in seinem Schritt, die Dauer groesser als 0.
inline constexpr double kVersatzHoechstens = 0.999;

/// Wo die Note anschlaegt, in Schritten ab dem Anfang des Musters, mit Bruchteil (N1 a).
[[nodiscard]] inline double notenAnfang(const Note& note)
{
    return note.schritt + note.versatzAnfang;
}
/// Wie lange die Note klingt, in Schritten mit Bruchteil (N1 a); immer groesser als 0.
[[nodiscard]] inline double notenDauer(const Note& note)
{
    return note.laenge + note.versatzEnde;
}

/// Nennt ein Muster sein Instrument beim Pfad einer eigenen Datei (`.lvinst`)
/// — statt beim Namen eines mitgelieferten?
[[nodiscard]] bool istInstrumentDatei(const QString& nennung);
/// Das Instrument eines neuen Musters und eines Musters ohne Angabe: `piano`.
[[nodiscard]] QString instrumentVorgabe();

/// Der Vorsatz eines gerechneten Tons unter den Klaengen: `inst:piano:60:500`
/// — Instrument (Name oder Pfad), Tonhoehe, Dauer in Millisekunden.
[[nodiscard]] bool istTon(const QString& klang);
/// Der Name des Klangs zu einem Ton eines Instruments.
[[nodiscard]] QString tonKlang(const QString& instrument, int hoehe, int dauerMs);
/// Den Namen wieder zerlegen. false = kein Ton. Ein Ton mit Saite und Ende
/// zerlegt sich hier auch — Saite und Ende fallen dabei weg.
bool zerlegeTon(const QString& klang, QString& instrument, int& hoehe, int& dauerMs);

/// Ein Wechsel der Tonhoehe im Ton einer Kette verbundener Noten (Stufe 25b,
/// S-L1 a): ab `ms` nach dem Anschlag die gegriffene Tonhoehe `hoehe` — als
/// Sprung (Legato) oder gleitend (Slide).
struct TonWechsel
{
    int ms = 0;
    int hoehe = 60;
    bool gleitet = false;  ///< Slide; sonst Legato (Hammer-on oder Pull-off)

    [[nodiscard]] bool operator==(const TonWechsel&) const = default;
};
/// Ein Punkt der Biegung eines Tons (Stufe 25b, S-L2 a): `ms` nach dem Anschlag,
/// die Biegung in Vierteltoenen ueber der gegriffenen Tonhoehe.
struct TonBiegung
{
    int ms = 0;
    int viertel = 0;

    [[nodiscard]] bool operator==(const TonBiegung&) const = default;
};

/// Ein Punkt eines Verlaufs im Ton (Stufe 28c-1b): `ms` nach dem Anschlag ein Wert in
/// Cent — die Tonhoehe der Spur Pitch oder die Tiefe der Spur Vibrato.
struct TonPunkt
{
    int ms = 0;
    int cent = 0;

    [[nodiscard]] bool operator==(const TonPunkt&) const = default;
};

/// Ein Punkt eines Pegelverlaufs im Ton (Stufe 28c-2b, SK12 a): `ms` nach dem Anschlag
/// die Stärke der Expression in Tausendsteln (0 … 1000).
struct PegelPunkt
{
    int ms = 0;
    int tausendstel = 1000;

    [[nodiscard]] bool operator==(const PegelPunkt&) const = default;
};

/// Welche Schichten ein Ton rechnet (Stufe 25c, S-L12 a): in einer Kette verbundener
/// Noten laeuft die Saite (*Extended KS*, *Waveguide*) als ein Ton ueber die Kette, mit
/// ihr jede Schicht mit *Follow string* (seit 25c-4); die uebrigen spielen je Note
/// (*Own note*). Welche folgt, sagt `folgtDerSaite` (Instrument.hpp).
enum class TonTeil : int
{
    Alle = 0,
    Folgend = 1,  ///< nur die Schichten, die der Saite folgen (`:p1`)
    Eigen = 2,    ///< nur die Schichten mit *Own note* (`:p2`)
};

/// Ein gerechneter Ton mit allem, was ihn ausmacht (seit Stufe 23b): bei einem
/// Saiteninstrument auch die Saite und das Ende, an dem die naechste Note auf
/// derselben Saite ihn abdaempft (Entscheid S-M3 a).
struct TonAngabe
{
    QString instrument;
    int hoehe = 60;
    int dauerMs = 1;  ///< so lang wird der Ton gehalten
    int saite = -1;   ///< −1 = keine (kein Saiteninstrument oder nicht greifbar)
    /// Spaetestens hier, ab dem Anschlag gezaehlt, ist der Ton still — die
    /// letzten Millisekunden blendet er aus (`kAbdaempfenMs`). 0 = kein Ende,
    /// er klingt aus wie bisher.
    int endeMs = 0;
    // --- seit Stufe 24b (S-W2 a, S-W6 a, S-W7 a): was den Klang des Anschlags aendert ---
    Richtung richtung = Richtung::Ab;  ///< Up: von der anderen Seite angeschlagen
    /// Nur die Bits, die den Klang aendern: `kPalmMute`, `kTapping`. *Let ring*
    /// steckt in Dauer und Ende, *Harmonic* in `teilton`.
    int artikulationen = 0;
    /// Ein Flageolett: der Teilton, der klingt (2 … 6); 1 = keines. Die Tonhoehe
    /// ist die klingende, gegriffen ist sie um diesen Teilton tiefer.
    int teilton = 1;
    // --- seit Stufe 25b (S-L1 a, S-L2 a): der Verlauf einer Kette und der Bend ---
    /// Die Wechsel der Tonhoehe, nach `ms` sortiert; leer = einer Tonhoehe. Der Ton
    /// beginnt mit `hoehe`.
    std::vector<TonWechsel> verlauf{};
    /// Die Biegung, linear zwischen den Punkten, nach `ms` sortiert (zwei Punkte an
    /// derselben Stelle: ein Sprung); leer = keine.
    std::vector<TonBiegung> biegung{};
    /// Seit Stufe 25c (S-L12 a): welche Schichten dieser Ton rechnet.
    TonTeil teil = TonTeil::Alle;
    // --- seit Stufe 28c-1b (SF4 a, SK3 a, SK10 a): die Spuren des Musters ueber dem Ton ---
    /// Die Spur Pitch: Cent ueber der Note, linear zwischen den Punkten, nach dem letzten
    /// der letzte; zwei Punkte an derselben Stelle: ein Sprung (an der Naht). Leer = keine.
    std::vector<TonPunkt> tonhoehe{};
    /// Die Spur Vibrato: die Tiefe in Cent, ebenso. Leer = das Vibrato der Schicht.
    std::vector<TonPunkt> vibrato{};
    // --- seit Stufe 28c-2b (SF6 a, SF7 a, SK12 a): Expression an einem Instrument mit Saiten ---
    /// Die Spur Expression des Musters ueber dem Ton, bis zum Ende des Clips (auch im
    /// Ausklang), linear zwischen den Punkten, danach der letzte; zwei Punkte an derselben
    /// Stelle: ein Sprung (an der Naht). Leer = 1.
    std::vector<PegelPunkt> ausdruckMuster{};
    /// Die Kurve Expression am Clip ueber dem Ton, ebenso. Leer = 1. Je Abtastwert werden
    /// beide malgenommen, der Pegel ist das Quadrat (SK9 a).
    std::vector<PegelPunkt> ausdruckClip{};
    // --- seit Stufe 31a (HA3 a, HK2 a): die Anschlagstaerke einer Hammer-Saite ---
    /// Die Stufe der Hammergeschwindigkeit (`anschlagStufe`, HammerSaite.hpp), ≤ 0;
    /// 0 = volle Staerke — und jeder Ton eines Instruments ohne Hammer.
    int anschlagStufe = 0;

    [[nodiscard]] bool operator==(const TonAngabe&) const = default;
};
/// Der Name: wie `tonKlang`, bei einer Saite `:s<saite>` dahinter, dann (seit
/// Stufe 24b) `:h<teilton>` bei einem Flageolett, `:u` bei Up, `:m` bei Palm
/// mute, `:t` bei Tapping, (seit Stufe 25b, S-L6 a) `:v<ms><l|g><hoehe>,…` bei
/// einem Verlauf und `:b<ms>-<viertel>,…` bei einem Bend, (seit Stufe 28c-1b)
/// `:q<ms>=<cent>,…` bei der Spur Pitch und `:w<ms>=<cent>,…` bei der Spur Vibrato,
/// (seit Stufe 28c-2b) `:x<ms>=<tausendstel>,…` bei der Spur Expression und
/// `:y<ms>=<tausendstel>,…` bei der Kurve am Clip, (seit Stufe 25c) `:p1` nur die
/// Saite, `:p2` ohne die Saite, (seit Stufe 31a, HK2 a) `:a<stufe>` bei einer
/// Anschlagstufe unter 0, zuletzt `:e<ms>` bei einem Ende — ohne alles derselbe
/// Name wie bis Stufe 22.
[[nodiscard]] QString tonKlang(const TonAngabe& ton);
/// Den Namen wieder zerlegen. false = kein Ton.
bool zerlegeTon(const QString& klang, TonAngabe& ton);

/// Ein Muster, die „Basis" eines oder mehrerer Clips. Ein **Drumkit** traegt
/// das Kit und je Spur eine Zeile, je Schritt eine Lautstaerke (0 = aus). Ein
/// **Instrument** traegt den Namen seines Instruments, Lautstaerke und
/// Panorama und seine Noten; es spielt mehrere Toene zugleich.
struct Muster
{
    QString name;
    MusterArt art = MusterArt::Drumkit;
    int takte = kTakteNeu;

    // --- Drumkit ---
    std::vector<Spur> spuren;
    /// Das Kit, das zuletzt auf das Muster angewendet wurde — seine Nennung
    /// (`808`, `user:mein_trap`, ein Pfad); leer = keines (Entscheid K4 a, S123).
    /// In der Datei das freiwillige Feld `kit` (keine neue Fassung).
    QString kit;
    /// `zeilen[spur][schritt]`, 0…1; nach `Loop::richte` je Spur eine Zeile
    /// mit `takte * schritteJeTakt()` Schritten.
    std::vector<std::vector<double>> zeilen;

    // --- Instrument ---
    /// Das Instrument (`loop/Instrument.hpp`): der Name eines mitgelieferten,
    /// etwa `piano`, oder der Pfad einer eigenen Datei `.lvinst` (absolut).
    QString instrument;
    double laut = 1.0;   ///< 0…1
    double pan = 0.0;    ///< -1 (links) … 1 (rechts)
    bool stumm = false;
    /// Nach `Loop::richte` nach Anfang (Schritt, dann Versatz), dann Tonhoehe
    /// sortiert; je Anfang und Tonhoehe hoechstens eine, keine reicht ueber das
    /// Ende des Musters.
    std::vector<Note> noten;
    /// Die Namen der Stimmen (31d-9, HD2 a): `stimmen[0]` ist Stimme 1 einer Note,
    /// hoechstens `kStimmenMax`; leer = keine benannt. In der Datei `voices` (seit
    /// Fassung 14, immer geschrieben; Stimme und Versatz einer Note stehen in ihrer
    /// Zeile nur, wenn einer davon gesetzt ist — wie Saite und Bend).
    QStringList stimmen;
    /// String choice (Stufe 23a, Entscheid S-M2c a): die Regel fuer Noten ohne
    /// eigene Saite. Ein neues Muster bekommt die Vorgabe seines Instruments
    /// (`Saitensatz::wahl`). Gilt nur, wenn das Instrument Saiten traegt.
    Saitenwahl saitenwahl = Saitenwahl::JeKlang;
    /// Strum (Stufe 24a, S-W2 a, S-W3 a): Versatz der Saiten und das Muster der
    /// Hand fuer alle Akkorde dieses Musters. Ein neues Muster bekommt die
    /// Vorgabe seines Instruments (`Saitensatz::strum`). Gilt nur, wenn das
    /// Instrument Saiten traegt.
    Strum strum;
    /// Die Akkorde, die davon abweichen (S-W4 a): je Schritt hoechstens ein
    /// Eintrag, nach Schritt sortiert, nur an Schritten, an denen eine Note
    /// anschlaegt (`Loop::richte`). Wer Noten verschiebt, kopiert oder dreht,
    /// nimmt den Eintrag ihres Schritts mit (Auswahl.hpp, Bearbeiten.hpp).
    std::vector<Anschlag> anschlaege;
    /// Die Spuren ueber der Zeit (Stufe 28a, SF5 d): Sustain, Sostenuto,
    /// Expression, Vibrato, Pitch und die Schalter der Klang-Spielweisen, in
    /// Schritten dieses Musters (Zeitspur.hpp). Leer = die Vorgaben des
    /// Instruments. In der Datei `lanes` (seit Fassung 12, immer geschrieben).
    Zeitspuren zeitspuren;

    [[nodiscard]] bool operator==(const Muster&) const = default;
};

/// Der Eintrag eines Musters an einem Schritt — oder nullptr: der Akkord dort
/// folgt dem Muster.
[[nodiscard]] const Anschlag* anschlagAn(const Muster& muster, int schritt);
/// Einen Eintrag setzen (ersetzt den am selben Schritt); einer, der nicht
/// abweicht, entfernt ihn.
void setzeAnschlag(Muster& muster, const Anschlag& anschlag);
/// Die Eintraege in ihre Grenzen: nach Schritt sortiert, je Schritt einer (der
/// erste bleibt), keiner, der nicht abweicht, keiner an einem Schritt ohne
/// anschlagende Note. Nach jeder Aenderung an den Noten (auch in `richte`).
void richteAnschlaege(Muster& muster);
/// Ein Muster bekommt die Vorgaben seines Instruments: *String choice* und
/// Strum (S-M2c a, S-W2 a) — fuer ein neues Muster und beim Wechsel des
/// Instruments.
void uebernimmVorgaben(Muster& muster, const Saitensatz& saiten);
/// Beim Wechsel des Instruments (S-W13 a): je Feld (*String choice*, Strum) die
/// Vorgabe des neuen — nur, wo das Muster noch die des alten traegt. Ein
/// Instrument ohne Saiten zaehlt mit den leeren Vorgaben (`Saitensatz{}`).
/// *Anschlaege* bleiben.
void wechsleVorgaben(Muster& muster, const Saitensatz& alt, const Saitensatz& neu);

// --- Effekte am Clip (Stufe 12, Entscheide Patrik S113: E2 b, F2 a) ---
// Jeder Clip traegt seine eigenen Effekte mit allen Einstellungen, gleich ob
// Drumkit oder Instrument. Seit Stufe 16a (Entscheide Patrik S117: K1 a,
// K3 a, K4 a) sind sie eine **Reihe von Bausteinen**: Filter, Distortion,
// Echo, Reverb, jeder beliebig oft, in freier Reihenfolge, hoechstens
// `kBausteineMax`. Hier steht nur das Modell.

inline constexpr double kEchoFeedbackMax = 0.9;  ///< darueber klaenge ein Echo nie aus
inline constexpr double kHallSekMin = 0.2;
inline constexpr double kHallSekMax = 8.0;
/// Die Grenze des Filters am Clip, in Hertz — er folgt keiner Tonhoehe.
inline constexpr double kClipFilterHzMin = 40.0;
inline constexpr double kClipFilterHzMax = 18000.0;

/// Die Zeit eines Echos als Notenwert — sie folgt dem Tempo des Loops.
enum class EchoZeit : int
{
    Sechzehntel = 0,
    Achtel = 1,
    AchtelPunktiert = 2,
    Viertel = 3,
    Halbe = 4,
};

/// Die Laenge eines Notenwerts in Schlaegen (eine Viertel = 1).
[[nodiscard]] double echoSchlaege(EchoZeit zeit);

/// Distortion: verzerrt die Summe des Clips (Drive je Schicht im Instrument
/// bleibt daneben und verzerrt je Ton).
struct ClipVerzerrung
{
    bool an = false;
    double drive = 0.3;  ///< 0…1
    double ton = 0.5;    ///< 0 (dunkel) … 1 (hell)
    double mix = 1.0;    ///< 0…1, der Anteil des bearbeiteten Tons

    [[nodiscard]] bool operator==(const ClipVerzerrung&) const = default;
};

struct ClipEcho
{
    bool an = false;
    EchoZeit zeit = EchoZeit::AchtelPunktiert;
    double feedback = 0.4;   ///< 0…`kEchoFeedbackMax`: was jede Wiederholung an die naechste weitergibt
    double daempfung = 0.3;  ///< 0…1: wie schnell die Wiederholungen dunkler werden
    double mix = 0.35;       ///< 0…1

    [[nodiscard]] bool operator==(const ClipEcho&) const = default;
};

struct ClipHall
{
    bool an = false;
    double groesse = 0.5;    ///< 0…1, die Groesse des Raums
    double sekunden = 1.8;   ///< der Nachklang, `kHallSekMin` … `kHallSekMax`
    double daempfung = 0.4;  ///< 0…1: wie schnell die Hoehen im Nachklang verschwinden
    double mix = 0.25;       ///< 0…1

    [[nodiscard]] bool operator==(const ClipHall&) const = default;
};

/// Filter: Art und Resonanz wie an der Schicht eines Instruments, die Grenze
/// aber in Hertz und ohne Filter envelope (K3 a).
struct ClipFilter
{
    bool an = false;
    FilterArt art = FilterArt::Tiefpass;
    double hz = 1000.0;     ///< `kClipFilterHzMin` … `kClipFilterHzMax`
    double resonanz = 0.0;  ///< 0…1

    [[nodiscard]] bool operator==(const ClipFilter&) const = default;
};

/// Ein Baustein in der Reihe eines Clips. Ausgeschaltet (`an`) steht er in
/// der Reihe und wird uebersprungen. Die Folge der Arten ist die, in der bis
/// Fassung 4 der `.lvloop` fest gerechnet wurde — mit dem Filter davor.
using ClipBaustein = std::variant<ClipFilter, ClipVerzerrung, ClipEcho, ClipHall>;

struct ClipEffekte
{
    /// Der Weg des Tons, von vorn nach hinten; leer = der Clip klingt, wie er
    /// ist. Nach `Loop::richte` hoechstens `kBausteineMax`.
    std::vector<ClipBaustein> reihe;

    [[nodiscard]] bool operator==(const ClipEffekte&) const = default;
    /// Ist einer eingeschaltet? Dann zeigt der Clip die Marke „fx".
    [[nodiscard]] bool einerAn() const
    {
        return std::any_of(reihe.begin(), reihe.end(), [](const ClipBaustein& baustein) {
            return std::visit([](const auto& werte) { return werte.an; }, baustein);
        });
    }

    // --- der erste Baustein einer Art -----------------------------------
    // Kurzwege fuer den, der eine Reihe von Hand baut oder nachsieht (Tests):
    // der erste Baustein einer Art, in der alten Folge Distortion, Echo,
    // Reverb. Fenster und Klangerzeuger arbeiten seit Stufe 16 mit der
    // ganzen Reihe.

    /// Der erste Baustein dieser Art; nullptr = keiner.
    template <class Art>
    [[nodiscard]] const Art* erster() const
    {
        for (const ClipBaustein& baustein : reihe)
            if (const Art* werte = std::get_if<Art>(&baustein)) return werte;
        return nullptr;
    }
    /// Seine Werte — oder die Vorgabe (ausgeschaltet), wenn es keinen gibt.
    template <class Art>
    [[nodiscard]] Art wert() const
    {
        const Art* werte = erster<Art>();
        return werte != nullptr ? *werte : Art{};
    }
    /// Der erste Baustein dieser Art zum Aendern; fehlt er, kommt er mit der
    /// Vorgabe dazu — vor den ersten Baustein einer Art, die in der alten
    /// Folge nach ihm kommt.
    template <class Art>
    Art& hole()
    {
        for (ClipBaustein& baustein : reihe)
            if (Art* werte = std::get_if<Art>(&baustein)) return *werte;
        const std::size_t rang = ClipBaustein(Art{}).index();
        const auto platz = std::find_if(reihe.begin(), reihe.end(),
                                        [rang](const ClipBaustein& baustein) { return baustein.index() > rang; });
        return std::get<Art>(*reihe.insert(platz, ClipBaustein(Art{})));
    }
};

/// Ein Clip im Kanal: ein Muster ab einem Takt, einmal oder mehrfach
/// hintereinander. Seine Laenge ist immer ein Vielfaches der Musterlaenge.
struct LoopClip
{
    QString muster;          ///< Name des Musters
    int startTakt = 0;       ///< 0 = der erste Takt
    int wiederholungen = 1;  ///< mindestens 1
    /// Gehoeren zum Clip, nicht zum Muster: zwei Clips derselben Basis
    /// koennen verschieden klingen.
    ClipEffekte effekte;
    /// Der Schalter fuer die ganze Reihe (Entscheid Patrik E1 a, S124): aus =
    /// der Clip klingt trocken, die Reihe bleibt, wie sie ist (die Marke „fx"
    /// steht matt). In der Datei `fxOn`, nur wenn aus.
    bool effekteAn = true;
    /// Der Verlauf Expression ueber den Clip (Stufe 28a, SF5 d, SK8 a): 0 … 1, in
    /// Takten ab dem Anfang des Clips, mit der Spur des Musters malgenommen; leer
    /// = 1. Punkte hinter dem Ende bleiben und sind nicht zu hoeren. In der Datei
    /// `expression` (seit Fassung 12, immer geschrieben).
    std::vector<SpurPunkt> expression;
    /// Freeze (Stufe 30f, Entscheid FV2 a): der Abdruck (`clipAbdruck`, 40 Hex-Zeichen)
    /// des Stands, der beim letzten Speichern eingefroren wurde; leer = nicht
    /// eingefroren. Gespielt wird die Wellenform zum **jetzigen** Abdruck — weicht er
    /// hiervon ab, ist der Clip seit dem Einfrieren veraendert. Geht nicht in den
    /// Abdruck ein. In der Datei `frozen` (seit Fassung 13, immer geschrieben).
    QString frozen;
    /// Wirken seine Effekte gerade — Schalter an und ein Baustein an?
    [[nodiscard]] bool effekteWirken() const { return effekteAn && effekte.einerAn(); }

    // Mit Konstruktor statt als Aggregat: `{muster, start, wiederholungen}`
    // steht an vielen Stellen und soll ohne die Effekte gueltig bleiben.
    LoopClip() = default;
    LoopClip(QString musterName, int start, int wieOft, ClipEffekte clipEffekte = {})
        : muster(std::move(musterName)), startTakt(start), wiederholungen(wieOft), effekte(std::move(clipEffekte))
    {
    }

    [[nodiscard]] bool operator==(const LoopClip&) const = default;
};

/// Die Grenzen des Faders je Kanal in dB (Entscheid Patrik S139, LL3 a: wie das
/// Gain im Composer bis +24 dB). An der unteren Grenze schweigt der Kanal (−∞).
inline constexpr double kKanalGainMinDb = -60.0;
inline constexpr double kKanalGainMaxDb = 24.0;

struct Kanal
{
    QString name;
    bool an = true;
    /// Nach `Loop::richte` nach Start sortiert und ueberlappungsfrei.
    std::vector<LoopClip> clips;
    /// Der Fader (Stufe 27e-1, LL3 a): `kKanalGainMinDb` … `kKanalGainMaxDb`,
    /// 0 = unveraendert. Er wirkt nach den Effekten der Clips, wie an einem
    /// Mischpult. In der Datei `gain` (seit Fassung 11, immer geschrieben).
    double gainDb = 0.0;

    /// `gainDb` als Faktor auf die Amplitude: 0 dB = genau 1, an der unteren Grenze 0.
    [[nodiscard]] double gainFaktor() const;

    [[nodiscard]] bool operator==(const Kanal&) const = default;
};

/// Die Tags eines Loops — dasselbe Schema wie bei einem Song (Vorgabe Patrik
/// S109): liegt der Loop als Clip im Composer, zaehlt er als laufender Song,
/// und Player-Panel, Text-Vorlagen und Karaoke lesen hier, was sie bei einer
/// MP3 aus der Datei lesen. Alle Felder sind freiwillig; ohne Titel gilt der
/// Dateiname.
struct LoopTags
{
    QString titel;
    QString kuenstler;
    QString album;
    QString jahr;
    QString genre;
    QString nummer;  ///< Titelnummer
    /// Songtext wie in den Tags einer Datei abgelegt — er darf selbst
    /// LRC-Zeitmarken tragen.
    QString text;

    [[nodiscard]] bool operator==(const LoopTags&) const = default;
};

/// Die Clips, die ihre eingefrorene Wellenform spielen (Stufe 30f): je Clip — Kanal
/// (Index) und Starttakt — der Abdruck der Wellenform. Wer sie fuellt, hat nachgesehen,
/// dass die Datei zum jetzigen Abdruck vorliegt; leer = alles live, wie bis Stufe 30e.
using Eingefroren = std::map<std::pair<int, int>, QString>;

/// Der Klangname einer eingefrorenen Wellenform (Stufe 30f): `frozen:<abdruck>`.
[[nodiscard]] QString frozenKlang(const QString& abdruck);
/// Der Abdruck aus einem solchen Namen; leer = kein eingefrorener Klang.
[[nodiscard]] QString abdruckVonKlang(const QString& klang);

struct Loop
{
    LoopTags tags;
    double bpm = 120.0;
    int schlaegeJeTakt = 4;
    int schritteJeSchlag = 4;
    /// 0…1: verzoegert jeden zweiten Schritt eines Schlags, bei 1 um einen halben Schritt.
    double swing = 0.0;
    /// 0…1, die Lautstaerke des ganzen Loops. Die Vorgabe laesst Luft nach
    /// oben: die Spuren addieren sich, begrenzt wird nichts (Entscheid Patrik
    /// S108, L6 a). Sie wirkt nach den Effekten der Clips, wie ein Summenregler
    /// (Stufe 30, WX5 a).
    double laut = 0.5;
    /// Der Begrenzer am Ausgang (Stufe 27e-3, Entscheide Patrik LL7 a, LL13 a): haelt
    /// die Summe unter 0 dB, solange das Loop-Panel den Loop spielt — im Composer gilt
    /// allein der Limiter der Komposition. In der Datei `limiter` (Fassung 11, immer
    /// geschrieben); fehlt es, ist er an.
    bool limiter = true;

    std::vector<Muster> muster;
    std::vector<Kanal> kanaele;

    [[nodiscard]] bool operator==(const Loop&) const = default;

    [[nodiscard]] int schritteJeTakt() const { return schlaegeJeTakt * schritteJeSchlag; }
    /// Das erste Muster dieses Namens; nullptr = keins.
    [[nodiscard]] const Muster* findeMuster(const QString& name) const;
    /// Laenge eines Clips in Takten: Wiederholungen mal Musterlaenge; 0, wenn
    /// es sein Muster nicht gibt.
    [[nodiscard]] int clipTakte(const LoopClip& clip) const;
    /// Takte eines Durchlaufs: das Ende des letzten Clips, gleich ob sein
    /// Kanal an ist. Ohne Clips ein Takt (Stille).
    [[nodiscard]] int durchlaufTakte() const;
    [[nodiscard]] int durchlaufSchritte() const { return durchlaufTakte() * schritteJeTakt(); }
    /// Dauer eines Durchlaufs in Millisekunden (ohne Swing gerechnet — der
    /// letzte Schritt endet davon unberuehrt am Durchlauf).
    [[nodiscard]] double durchlaufMs() const;
    /// Die Klaenge aller Spuren aller Muster, jeder einmal, in der
    /// Reihenfolge ihres ersten Auftretens; Spuren ohne Klang zaehlen nicht.
    /// Dazu je Instrument-Muster die Toene seiner Noten (`tonKlang`): je
    /// Tonhoehe und Dauer einer — die Dauer haengt am Tempo.
    /// Mit `saiten` (Stufe 23b, Entscheid S-A1 a): ein Muster, dessen
    /// Instrument Saiten traegt, nennt seine Toene je Vorkommen in einem Clip,
    /// mit Saite und Ende (`toeneImKanal`, Saitenplan.hpp) — dieselbe Rechnung
    /// wie `plane`. Ohne `saiten` wie bis Stufe 22.
    /// Mit `eingefroren` (Stufe 30f): ein eingefrorener Clip nennt seine Wellenform
    /// (`frozenKlang`, hinten angehaengt in der Reihenfolge der Kanaele und Clips) statt
    /// seiner Toene — ein Muster, das nur in eingefrorenen Clips liegt, nennt keine.
    /// Ohne `eingefroren` wie bis Stufe 30e.
    [[nodiscard]] QStringList klangNamen(const InstrumentVon& saiten = {}, const Eingefroren& eingefroren = {}) const;
    /// Die Dauer einer Note in Millisekunden, gerundet — wie sie im Namen
    /// ihres Tons steht.
    [[nodiscard]] int notenMs(const Note& note) const;

    /// Eine Stelle im Durchlauf in Millisekunden als Stelle in Schritten (linear,
    /// ohne Swing — wie die Marke sie zeigt).
    [[nodiscard]] double schritteVonMs(double ms) const;
    /// Wo das Muster `muster` an der Stelle `schritt` des Arrangements steht,
    /// in Schritten des Musters (S127, Entscheid Patrik W1 a): liegt die Stelle
    /// in einem Clip dieses Musters, die Stelle darin (Wiederholungen
    /// eingerechnet). Liegen mehrere Clips dieses Musters dort, die erste von
    /// `stellenImMuster`. -1 = kein Clip dieses Musters liegt dort.
    [[nodiscard]] double stelleImMuster(double schritt, const QString& muster) const;
    /// Alle Stellen im Muster `muster` an der Stelle `schritt` — je Clip dieses
    /// Musters, der dort in einem eingeschalteten Kanal liegt, eine (S130,
    /// Entscheid Patrik W8-2 a): zuerst der zuletzt begonnene, dann die frueher
    /// begonnenen (bei gleichem Anfang der obere Kanal zuerst). Liegt keiner in
    /// einem eingeschalteten Kanal, der erste in einem ausgeschalteten (eine
    /// Stelle). Leer = kein Clip dieses Musters liegt dort.
    [[nodiscard]] std::vector<double> stellenImMuster(double schritt, const QString& muster) const;

    /// Unsinn richten: Werte in ihre Grenzen, je Muster eine Zeile je Spur in
    /// voller Laenge, Muster ohne Namen benennen; Clips nach Start sortieren,
    /// Ueberlappung im selben Kanal aufloesen (der spaetere Clip beginnt, wo
    /// der fruehere endet), Clips ohne Muster entfernen. Was auffaellt, steht
    /// danach im `bericht`.
    void richte(QStringList* bericht = nullptr);
};

/// Der Loop als JSON. Pfade von Sample-Dateien werden relativ zu
/// `basisOrdner` abgelegt, wenn sie darunter liegen — sonst absolut.
[[nodiscard]] QJsonObject alsJson(const Loop& loop, const QString& basisOrdner = {});
/// Liest einen Loop und richtet ihn. false = kein Loop (falsches Format oder
/// eine Fassung vor 2); `loop` bleibt dann, wie er war. Fehlt eine
/// Sample-Datei, steht das im `bericht`.
bool ausJson(const QJsonObject& wurzel, Loop& loop, const QString& basisOrdner = {},
             QStringList* bericht = nullptr);

bool speichereLoop(const Loop& loop, const QString& pfad);
bool ladeLoop(const QString& pfad, Loop& loop, QStringList* bericht = nullptr);

}  // namespace lumi::loop
