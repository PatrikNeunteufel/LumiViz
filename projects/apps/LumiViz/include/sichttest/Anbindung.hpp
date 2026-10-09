/**
 * @file Anbindung.hpp
 * @brief Sichttest-Steuerung: der Tester `Sichttest.exe` loest in LumiViz
 *        Aktionen aus (docs/ui/Composer_Konzept.md §12.2 Naht N20;
 *        Spezifikation: SichtTest_Helper `docs/Konzept_Steuerung.md`)
 *
 * Nur mit dem Schalter `--testing` laedt LumiViz die `SichttestSteuerung1.dll`
 * aus dem Ordner der Exe — gelinkt wird nichts, ein gewoehnlicher Start laedt
 * nichts und oeffnet nichts. Die Koepfe des Pakets stehen allein in
 * `Anbindung.cpp`, hinter `SICHTTEST_VORHANDEN`; dieser Kopf traegt nur Typen
 * der App, damit jeder Teil seine Aktionen selbst anmelden kann.
 *
 * Threads: alles im GUI-Thread. Die Aktionen laufen dort, wenn der Tester sie
 * ausloest — nie, bevor `verbinde()` gerufen wurde.
 *
 * Rueckbau (Konzept §12): `include/sichttest` und `src/sichttest` loeschen,
 * die include()-Zeile in `src/Source.cmake` nehmen, Naht N20 zuruecknehmen.
 */
#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <functional>
#include <memory>
#include <optional>

namespace lumi::sichttest
{

/// Was eine Aktion dem Tester antwortet: erledigt, gescheitert oder
/// „die Argumente passen nicht" — dazu ein Satz, der im Tester am Punkt steht.
struct Ergebnis
{
    enum class Status
    {
        Ok,
        Fehler,
        Ungueltig,
    };
    Status status = Status::Ok;
    QString text;
};

[[nodiscard]] inline Ergebnis ok(const QString& text = {}) { return {Ergebnis::Status::Ok, text}; }
[[nodiscard]] inline Ergebnis fehler(const QString& text) { return {Ergebnis::Status::Fehler, text}; }
[[nodiscard]] inline Ergebnis ungueltig(const QString& text)
{
    return {Ergebnis::Status::Ungueltig, text};
}

/// Die Schalter einer Aktion (Konzept der Steuerung §3).
enum Schalter : unsigned
{
    Keiner = 0,
    FragtNach = 0x1,         ///< ersetzt Zustand, der verloren gehen kann; der Tester fragt vorher
    NurErstlauf = 0x2,       ///< raeumt ab; entfaellt beim Fortsetzen eines Laufs
    WartetAufMensch = 0x4,   ///< fragt in der App nach; keine Frist im Tester
    BeendetAnwendung = 0x8,  ///< nach der Antwort endet die App oder startet neu
};

/// Steht `--testing` unter den Argumenten?
[[nodiscard]] bool verlangt(const QStringList& argumente);

/// Liegt die DLL der Steuerung neben der Exe? Im Release-Paket fehlt sie —
/// dann gibt es keinen Tester, und was nur ihm dient (eine Einstellung),
/// bleibt unsichtbar. Ohne das Paket gebaut: immer false.
/// `exeOrdner`: leer = der Ordner dieser Exe (nur Tests nennen einen anderen).
[[nodiscard]] bool vorhanden(const QString& exeOrdner = {});

/// Der Root des Projekts: von `ab` aufwaerts der erste Ordner, der
/// `.sichttest/sichttest.projekt.json` enthaelt — dieselbe Regel wie im
/// Tester. Leer = keiner gefunden.
[[nodiscard]] QString wurzel(const QString& ab);

/// Eine Datei aus den Argumenten einer Aktion: absolut wie angegeben, relativ
/// ab `root` (die Listen nennen ihre Vorlagen ab dem Root des Projekts).
/// Leer = `angabe` ist leer oder benennt keine vorhandene Datei.
[[nodiscard]] QString loeseAuf(const QString& angabe, const QString& root);

/// `an` oder `aus` aus den Argumenten — als einzelner Wert (`composer an`,
/// kommt als `wert` an) oder unter `schluessel`. Leer = keines von beiden.
[[nodiscard]] std::optional<bool> anOderAus(const QJsonObject& argumente,
                                            const QString& schluessel = QStringLiteral("wert"));

/// Eine Zeit aus den Argumenten, in Millisekunden: `m:ss`, `m:ss.z` (wie die
/// Listen und das Panel Composer sie schreiben) oder Sekunden als Zahl
/// (`20`, `20.5`). Leer = keine Zeit oder negativ.
[[nodiscard]] std::optional<qint64> zeitMs(const QString& angabe);

/// Der erste nicht leere Wert unter den Schluesseln — `schluessel`, sonst
/// `wert` (ein einzelner Wert ohne Schluessel kommt als `wert` an).
[[nodiscard]] QString argument(const QJsonObject& argumente, const QString& schluessel);

class Anbindung
{
public:
    using Aktion = std::function<Ergebnis(const QJsonObject& argumente)>;

    enum class Zustand
    {
        Getrennt,
        Verbindet,
        Verbunden,
        Abgelehnt,  ///< die Fassung von Schnittstelle oder Protokoll passt nicht
    };

    /// Die DLL aus dem Ordner der Exe laden und die Sitzung anlegen. Leer =
    /// es gibt keine Steuerung (ohne das Paket gebaut, die DLL fehlt, die
    /// Fassung passt nicht); `grund` nennt dann einen Satz dazu.
    /// `exeOrdner`: der Ordner, in dem die DLL liegt und von dem aus der Root
    /// des Projekts gesucht wird — leer = der Ordner dieser Exe (nur Tests
    /// nennen einen anderen).
    [[nodiscard]] static std::unique_ptr<Anbindung> lade(const QString& version,
                                                         QString* grund = nullptr,
                                                         const QString& exeOrdner = {});
    ~Anbindung();
    Anbindung(const Anbindung&) = delete;
    Anbindung& operator=(const Anbindung&) = delete;

    /// Eine Aktion anmelden — vor oder nach `verbinde()`. false = der Name
    /// ist ungueltig oder schon vergeben.
    bool aktion(const QString& name, const QString& beschreibung, const QString& parameter,
                Aktion aktion, unsigned schalter = Keiner);
    /// Die angemeldeten Namen, in der Reihenfolge des Anmeldens.
    [[nodiscard]] QStringList aktionen() const;

    /// Mit dem Tester verbinden; startet ihn, wenn keiner lauscht. Kehrt
    /// sofort zurueck — ab hier kann die App gesteuert werden. Wie es ausging,
    /// steht im Log, sobald es feststeht.
    bool verbinde();
    [[nodiscard]] Zustand zustand() const;
    [[nodiscard]] QString letzterFehler() const;
    /// Was die geladene DLL ueber sich sagt, als ein Satz fuer das Log:
    /// Produktversion, Fassung der Schnittstelle, Spanne des Protokolls.
    [[nodiscard]] QString dllFassung() const;
    /// Der Root des Projekts, ab dem die Aktionen ihre Dateien suchen.
    [[nodiscard]] const QString& root() const;

private:
    Anbindung();
    struct Innen;
    std::unique_ptr<Innen> m;
};

}  // namespace lumi::sichttest
