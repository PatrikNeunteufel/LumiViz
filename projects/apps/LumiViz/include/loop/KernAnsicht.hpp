/**
 * @file KernAnsicht.hpp
 * @brief Loop: eine Kern-Schicht im Creator ansehen — Kasten *Core* und *Body* (Instrumente_IC_Konzept.md FU3 a, §6a
 *        CK1 a, CK2 a; Stufe 34d)
 *
 * Kein Code je Kern: alles kommt aus der Parametertafel des Kerns (`kern::kerne().typ(name)->tafel`).
 * - **Core:** Kopf „Core · <Titel>“ mit dem Namen des Kerns und seinem Tastenumfang, die Marke *Unmeasured*
 *   (`kern::ungemessen`), darunter die **Gruppen der Tafel** in ihrer Reihenfolge, einklappbar. Je Wert eine Zeile:
 *   Titel, Wert mit Einheit (eine Kurve als kleine Form ueber den Tastenumfang mit kleinstem und groesstem Wert) und die
 *   Marke der Belegkraft (*Measured*, *Source*, *Computed*, *Open*); die Herkunft steht im Tooltip.
 * - **Body** (FU1 a): der Koerper an der Schicht, den die App mit der Kraft des Kerns faltet — oder „none“.
 *
 * Nur zum Ansehen (FU3 a): die Bearbeitung kommt mit 31g.
 *
 * Kein Ton, keine Audio-Engine.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"
#include "loop/Instrument.hpp"

#include <QSet>
#include <QString>
#include <QStringList>
#include <QWidget>

class QGridLayout;
class QLabel;
class QScrollArea;

namespace lumi::loop
{

/// Eine Kurve je Note als kleine Form: ueber den Tastenumfang, zwischen ihrem kleinsten und groessten Wert; eigene
/// Werte einzelner Tasten als Punkte.
class KernKurvenBild : public QWidget
{
    Q_OBJECT

public:
    explicit KernKurvenBild(QWidget* parent = nullptr);
    void setze(const kern::Kurve& kurve, kern::Skala skala, int noteMin, int noteMax);
    /// Die Werte, wie gezeichnet: je Note des Umfangs einer (fuer Tests).
    [[nodiscard]] const std::vector<double>& werte() const { return m_werte; }
    [[nodiscard]] QSize sizeHint() const override { return {120, 22}; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<double> m_werte;
    std::vector<int> m_eigene;  ///< Plaetze in `m_werte` mit eigenem Wert
};

class KernAnsicht : public QWidget
{
    Q_OBJECT

public:
    /// Welcher Kasten der Zeile *Signal path* gewaehlt ist.
    enum class Teil
    {
        Kern,     ///< *Core*: die Tafel
        Koerper,  ///< *Body*: der Koerper an der Schicht
    };

    explicit KernAnsicht(QWidget* parent = nullptr);

    /// Diese Schicht zeigen (eine Kern-Schicht; sonst bleibt die Ansicht leer).
    void zeige(const Schicht& schicht, Teil teil);

    /// Der Titel des Kerns (aus der Tafel; der Name, wenn es ihn nicht gibt).
    [[nodiscard]] static QString kernTitel(const Schicht& schicht);
    /// Wie eine Belegkraft in der Ansicht heisst: *Measured*, *Source*, *Computed*, *Open*.
    [[nodiscard]] static QString belegkraftTitel(kern::Belegkraft belegkraft);
    /// Ob eine Kern-Schicht ungemessen ist (Z2 a, IK2-9): ein klangbestimmender Wert ohne *Measured*.
    [[nodiscard]] static bool ungemessen(const Schicht& schicht);

    // --- fuer Tests ----------------------------------------------------------
    /// Der Kopf, wie er dasteht („Core · Grand piano“).
    [[nodiscard]] QString kopf() const;
    /// Je Zeile „<Gruppe> / <Titel>: <Wert> [<Belegkraft>]“ in der Reihenfolge der Ansicht.
    [[nodiscard]] const QStringList& zeilen() const { return m_zeilen; }
    /// Die Gruppen in ihrer Reihenfolge.
    [[nodiscard]] const QStringList& gruppen() const { return m_gruppenNamen; }
    /// Eine Gruppe ein- oder ausklappen (wie ein Klick auf ihre Ueberschrift).
    void setzeGruppeZu(const QString& gruppe, bool zu);
    [[nodiscard]] bool gruppeZu(const QString& gruppe) const { return m_zu.contains(gruppe); }
    /// Ist die Marke *Unmeasured* zu sehen?
    [[nodiscard]] bool markeUngemessen() const;
    /// Die Marke *Unmeasured* im Kopf der Ansicht zeigen (Vorgabe: ja). Der Creator schaltet sie ab — dort steht sie
    /// neben *Type* (§6a 2).
    void setzeMarkeHier(bool an);

private:
    void baue();

    bool m_markeHier = true;
    Schicht m_schicht;
    Teil m_teil = Teil::Kern;
    QSet<QString> m_zu;  ///< eingeklappte Gruppen (gilt fuer jede Schicht, solange das Fenster lebt)
    QStringList m_zeilen;
    QStringList m_gruppenNamen;

    QLabel* m_kopf = nullptr;
    QLabel* m_unterzeile = nullptr;
    QLabel* m_ungemessen = nullptr;
    QLabel* m_hinweis = nullptr;
    QScrollArea* m_rollen = nullptr;
    QWidget* m_inhalt = nullptr;
    QGridLayout* m_gitter = nullptr;
};

}  // namespace lumi::loop
