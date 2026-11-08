/**
 * @file Parametertafel.hpp
 * @brief InstrumentCore: die Parametertafel eines Kerns und seine Werte — reine Daten, ohne Qt
 *        (Instrumente_IC_Konzept.md IK2-4 a, IK2-9, IK2-11 a, §6a CK1 a)
 *
 * Jeder Kern meldet ueber die Fabrik eine **Tafel**: je Wert Name, Titel, **Gruppe**, Reihenfolge (die der Tafel),
 * **Art** (Zahl, Ganzzahl, Wahl aus Namen, Schalter, Kurve je Note, Liste von Eintraegen), **Darstellung** (Regler,
 * Auswahl, Haken, Kurve, Dialog), Einheit, Grenzen, Vorgabe, Skala der Kurve, Belegkraft und Herkunft der Vorgabe;
 * je Kern Tastenumfang, Fassung der Rechnung und die gestimmten Werte mit ihren Abhaengigkeiten. Die App liest und
 * schreibt die Werte allgemein nach der Tafel (`coreParams` in der `.lvinst`, Fassung 21, 34d) — kein Code je Klasse.
 *
 * **Werte** sind eine geordnete Liste aus Name und `Wert`. `richteWerte` bringt sie in die Grenzen der Tafel und meldet
 * alles, was fehlt oder nicht passt — ein fehlender Wert ist ein Fehler, kein stiller Ersatz (IK2-11 a: „Fehlt ein
 * gestimmter Wert, meldet das Laden einen Fehler“; alle Werte werden immer geschrieben, IK2-4 a).
 *
 * **Gestimmte Werte** (IK2-11 a) sind Kurven mit Belegkraft *Rechnung*; zu jedem merken die Werte den Abdruck der
 * Eingaben, aus denen er gestimmt wurde (`gestimmtAus`). `veralteteGestimmte` vergleicht ihn mit dem heutigen, ohne
 * zu rechnen. Ein gestimmter Wert darf von gestimmten Werten abhaengen, die vor ihm in der Tafel stehen (KS2 a, 34g-2:
 * die Fenster des Bogens haengen am gestimmten Pegel des Resonators); gestimmt wird in der Reihenfolge der Tafel.
 *
 * **Tastenumfang** (KS1 a, 34g-2): ein Kern kann vielen Instrumenten dienen; nennt die Tafel `umfangVon`/`umfangBis`,
 * traegt jedes Instrument seinen Umfang als zwei Ganzzahl-Werte (`tastenumfang`).
 *
 * **Ungemessen** (IK2-9, Folgerung S173): ein Instrument ist ungemessen, solange ein klangbestimmender Wert keine
 * Belegkraft *Gemessen* traegt (`ungemessen`) — nicht von Hand gesetzt.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace lumi::kern
{

struct Werte;

/// Eine Liste von Eintraegen (Choere, Felle, Stegmoden …), jeder mit den Feldern seiner Angabe.
struct Liste
{
    std::vector<Werte> eintraege;

    [[nodiscard]] bool operator==(const Liste& o) const;
};

/// Ein Wert: Zahl, Ganzzahl, Schalter, Wahl (Name), Kurve je Note, Liste.
using Wert = std::variant<double, std::int64_t, bool, std::string, Kurve, Liste>;

/// Je Name eines Werts die Schluessel seiner Quellen im Register (`asset/loop/instrument_quellen.json`, QM1 a).
using QuellenJeWert = std::vector<std::pair<std::string, std::vector<std::string>>>;

/// Die Werte eines Instruments (oder eines Eintrags einer Liste), in der Reihenfolge, in der sie gesetzt wurden.
struct Werte
{
    std::vector<std::pair<std::string, Wert>> felder;
    /// je gestimmtem Wert der Abdruck der Eingaben, aus denen er gestimmt wurde (IK2-11 a)
    std::vector<std::pair<std::string, std::string>> gestimmtAus;
    /// QM1 a (34h, S180): die Quellen je Wert, der keine Kurve ist (Zahl, Ganzzahl, Schalter, Wahl, Liste als Ganzes) —
    /// eine Kurve traegt ihre eigenen (`Kurve::quellen`). Nur an den Werten eines Instruments, nicht an den Eintraegen
    /// einer Liste. Zaehlt nicht zum Vergleich und nicht zum Abdruck (wie die Herkunft einer Kurve).
    QuellenJeWert quellen;

    [[nodiscard]] const Wert* finde(std::string_view name) const;
    [[nodiscard]] Wert* finde(std::string_view name);
    /// setzt oder ersetzt
    void setze(std::string name, Wert wert);

    /// Lesen mit Art; fehlt der Name oder hat er eine andere Art: `std::out_of_range`.
    [[nodiscard]] double zahl(std::string_view name) const;
    [[nodiscard]] std::int64_t ganzzahl(std::string_view name) const;
    [[nodiscard]] bool schalter(std::string_view name) const;
    [[nodiscard]] const std::string& wahl(std::string_view name) const;
    [[nodiscard]] const Kurve& kurve(std::string_view name) const;
    [[nodiscard]] const Liste& liste(std::string_view name) const;

    /// der gemerkte Abdruck eines gestimmten Werts, leer = keiner
    [[nodiscard]] std::string abdruckVon(std::string_view gestimmt) const;
    void merkeAbdruck(std::string gestimmt, std::string abdruck);

    /// die Quellen eines Werts, `nullptr` = keine genannt
    [[nodiscard]] const std::vector<std::string>* quellenVon(std::string_view name) const;
    /// setzt oder ersetzt
    void setzeQuellen(std::string name, std::vector<std::string> schluessel);

    /// gleich heisst: dieselben Namen mit denselben Werten in derselben Reihenfolge und dieselben Abdruecke (die
    /// Quellen zaehlen nicht, wie die Herkunft einer Kurve, `Kurve::operator==`)
    [[nodiscard]] bool operator==(const Werte& o) const;
};

/// Was fuer ein Wert.
enum class WertArt : int
{
    Zahl = 0,
    Ganzzahl = 1,
    Schalter = 2,
    Wahl = 3,
    Kurve = 4,
    Liste = 5,
};

/// Wie der Creator einen Wert zeigt (§6a, CK1 a).
enum class Darstellung : int
{
    Regler = 0,   ///< Zahl, Ganzzahl
    Auswahl = 1,  ///< Wahl; auch eine Ganzzahl mit wenigen Stufen
    Haken = 2,    ///< Schalter (Ausstattung)
    Kurve = 3,    ///< Kurve je Note: Zeile mit kleiner Form und Marke der Belegkraft
    Dialog = 4,   ///< Liste: Zeile mit Kurzform und *Edit...*
};

/// Die Darstellung, die zu einer Art gehoert, wenn die Tafel keine andere nennt.
[[nodiscard]] Darstellung darstellungZu(WertArt art);

/// Ein Wert der Tafel.
struct ParameterAngabe
{
    std::string name;    ///< Schluessel in `coreParams` (camelCase wie die `.lvinst`)
    std::string titel;   ///< Beschriftung im Creator (Englisch wie die Oberflaeche)
    std::string gruppe;  ///< eine der `Parametertafel::gruppen`
    WertArt art = WertArt::Zahl;
    Darstellung darstellung = Darstellung::Regler;
    std::string einheit;
    /// Grenzen: Zahl und Ganzzahl der Wert, Kurve die Werte der Punkte
    double von = 0.0;
    double bis = 1.0;
    /// Skala einer Kurve zwischen den Punkten (auch die Skala eines Reglers)
    Skala skala = Skala::Linear;
    /// die Wahlen einer Wahl
    std::vector<std::string> wahlen;
    /// die Felder je Eintrag einer Liste und ihre Zahl
    std::vector<ParameterAngabe> felder;
    std::size_t eintraegeMin = 0;
    std::size_t eintraegeMax = 64;
    Wert vorgabe = 0.0;
    /// Beleg der Vorgabe (eine Kurve traegt ihren eigenen; hier der fuer Zahl, Wahl, Schalter, Liste)
    Belegkraft belegkraft = Belegkraft::Offen;
    std::string herkunft;
    /// die Schluessel der Quellen der Vorgabe im Register (WZ1 a; eine Kurve traegt ihre eigenen)
    std::vector<std::string> quellen;
    /// zaehlt fuer „ungemessen“ (eine Ausstattung oder ein Bezug wie `ausgangBezug` nicht)
    bool klangbestimmend = true;
    /// gestimmt (IK2-11 a): eine Kurve, die ein Werkzeug aus anderen Werten rechnet
    bool gestimmt = false;
    /// woraus ein gestimmter Wert gestimmt wird (Namen der Tafel); leer = aus allen nicht gestimmten und den gestimmten,
    /// die vor ihm in der Tafel stehen (KS2 a: gestimmt wird in der Reihenfolge der Tafel)
    std::vector<std::string> abhaengigVon;
};

/// Die Tafel eines Kerns.
struct Parametertafel
{
    std::string kern;    ///< Name in der Fabrik, `"core"` in der `.lvinst`
    std::string titel;   ///< Name im Creator („Core · <Titel>“)
    /// die Fassung der Rechnung des Kerns: geht in den Abdruck gestimmter Werte ein
    int rechenFassung = 1;
    /// der Tastenumfang (MIDI)
    int noteMin = kNoteMin;
    int noteMax = kNoteMax;
    /// die Gruppen in ihrer Reihenfolge (einklappbar im Creator, nach den Abschnitten der IC-Koepfe)
    std::vector<std::string> gruppen;
    /// die Werte in ihrer Reihenfolge
    std::vector<ParameterAngabe> werte;
    /// der Bezug des Ausgangs (IK2-10 a, 34f-3): der Name des Werts (Zahl), den `normiere` setzt — leer, wenn der Kern
    /// keinen traegt (der Fluegel rechnet in `kHammerKraftBezug`). Er zaehlt nicht zum Abdruck gestimmter Werte: er wird
    /// nach dem Stimmen gesetzt und haengt an ihnen
    std::string bezugWert;
    /// so oft rechnet `normiere` (zwei, wenn Teile des Klangs am Bezug haengen, wie die Rauschanteile der Membran)
    int bezugRunden = 1;
    /// der Tastenumfang je Instrument (KS1 a, 34g-2): die Namen zweier Ganzzahl-Werte der Tafel, die den Umfang des
    /// Instruments tragen (ein Kern, viele Instrumente: Marimba 36–96, Glockenspiel 79–108). Leer, wenn der Umfang der
    /// Tafel gilt (Fluegel); sonst sind `noteMin`/`noteMax` der moegliche Umfang, und die Kurven, die Probe des Bezugs
    /// und die Ansicht im Creator folgen dem des Instruments (`tastenumfang`).
    std::string umfangVon;
    std::string umfangBis;
};

/// Der Tastenumfang eines Instruments (KS1 a): aus den Werten `umfangVon`/`umfangBis`, auf den Umfang der Tafel
/// geklemmt; der der Tafel, wenn sie keine Namen nennt, ein Wert fehlt oder der Umfang verkehrt ist.
[[nodiscard]] std::pair<int, int> tastenumfang(const Parametertafel& tafel, const Werte& werte);

/// Die Angabe mit diesem Namen, oder `nullptr`.
[[nodiscard]] const ParameterAngabe* findeAngabe(const Parametertafel& tafel, std::string_view name);

/// Was an einer Tafel nicht stimmt (leer = gut): Name leer oder doppelt, Gruppe nicht gelistet, Vorgabe von anderer
/// Art, Grenzen verkehrt, Vorgabe ausserhalb, Wahl nicht unter den Wahlen, Darstellung passt nicht zur Art, ein
/// gestimmter Wert ist keine Kurve, eine Abhaengigkeit fehlt oder ist ein gestimmter Wert, der nicht vor ihm steht
/// (KS2 a), Tastenumfang verkehrt, die Namen des Umfangs je Instrument sind keine Ganzzahlen der Tafel oder ihre Vorgaben
/// liegen ausserhalb (KS1 a).
[[nodiscard]] std::vector<std::string> pruefeTafel(const Parametertafel& tafel);

/// Die Werte der Vorgaben (jede Angabe in der Reihenfolge der Tafel), mit den Quellen der Angaben (QM1 a).
[[nodiscard]] Werte vorgaben(const Parametertafel& tafel);

/// Die Werte in die Tafel richten: in die Reihenfolge der Tafel, Zahlen und Ganzzahlen auf die Grenzen, Kurven mit
/// `richteKurve` auf Grenzen und Tastenumfang des Instruments (`tastenumfang`, KS1 a; eine leere bekommt die Vorgabe —
/// ausser einer gestimmten; ein verkehrter Umfang ist ein Fehler, gerichtet wird dann auf den der Tafel), Listen je
/// Eintrag nach den Feldern. Die Quellen (QM1 a) in die Reihenfolge der Tafel, je Wert ohne Kurve; fehlen sie zu einem
/// Wert, die der Angabe (`ParameterAngabe::quellen`); Quellen zu Kurven, zu unbekannten Namen und an Eintraegen einer
/// Liste fallen weg. Zurueck kommt, was nicht stimmt: ein Wert fehlt, ist unbekannt oder von anderer Art,
/// eine Wahl ist unbekannt, eine Liste hat zu wenige oder zu viele Eintraege, ein gestimmter Wert ist leer. Leer =
/// gut. Ein fehlender Wert wird nicht ersetzt.
[[nodiscard]] std::vector<std::string> richteWerte(const Parametertafel& tafel, Werte& werte);

/// Ein Wert als Text, Bit fuer Bit eindeutig (Zahlen als hex ihrer 64 Bit) — fuer Abdruecke.
[[nodiscard]] std::string wertAlsText(const Wert& wert);

/// Der Abdruck der Eingaben eines gestimmten Werts (IK2-11 a): SHA-256 ueber Kern, Fassung der Rechnung und die Werte,
/// aus denen er gestimmt wird (`abhaengigVon`, sonst alle nicht gestimmten ausser dem Bezug `bezugWert` und die gestimmten
/// vor ihm, KS2 a — aendert sich ein vorgelagerter gestimmter Wert, veraltet er mit). Leer, wenn `gestimmt` kein
/// gestimmter Wert der Tafel ist.
[[nodiscard]] std::string eingabenAbdruck(const Parametertafel& tafel, const Werte& werte, std::string_view gestimmt);

/// Die gestimmten Werte, deren gemerkter Abdruck nicht zum heutigen passt (oder fehlt), in der Reihenfolge der Tafel.
[[nodiscard]] std::vector<std::string> veralteteGestimmte(const Parametertafel& tafel, const Werte& werte);

/// Ungemessen (IK2-9): ein klangbestimmender Wert traegt keine Belegkraft *Gemessen* — eine Kurve ihre eigene, jeder
/// andere die Belegkraft seiner Angabe.
[[nodiscard]] bool ungemessen(const Parametertafel& tafel, const Werte& werte);

/// Der Name einer Belegkraft, wie sie in der Datei steht (`measured`, `source`, `computed`, `open`), und zurueck.
[[nodiscard]] std::string_view belegkraftName(Belegkraft belegkraft);
[[nodiscard]] bool belegkraftAusName(std::string_view name, Belegkraft& belegkraft);

}  // namespace lumi::kern
