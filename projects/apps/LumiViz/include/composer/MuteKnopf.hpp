/**
 * @file MuteKnopf.hpp
 * @brief Composer: der Mute-Knopf und der Dialog der Mute-Profile
 *        (Vorgabe Patrik S101, Entscheid AM2: c; Konzept §4)
 *
 * **Der Knopf** schaltet den einen Mute der App (`MuteSchalter`) nach dem
 * gewaehlten Mute-Profil. Es gibt ihn zweimal — im Panel Player (Naht N18)
 * und im Panel Composer; beide zeigen denselben Zustand. Rechtsklick: das
 * Profil waehlen, den Dialog oeffnen.
 *
 * **Der Dialog** stellt ein Profil ein — Zielpegel, Dauer und Kurvenform des
 * Aus- und des Einblendens, „Visuals follow" — und speichert es unter einem
 * Namen. Die Kurvenformen sind die Blenden-Presets (`FormPresets`).
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Mute.hpp"

#include <QDialog>
#include <QPushButton>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class ServiceContainer;

namespace lumi::composer
{

class MuteKnopf : public QPushButton
{
    Q_OBJECT

public:
    /// `dienste`: daraus holt sich der Knopf beim Schalten den Player.
    explicit MuteKnopf(ServiceContainer& dienste, QWidget* parent = nullptr);

    /// Ein Profil waehlen (der Weg des Rechtsklicks); leer = die Vorgabe ohne
    /// Datei. Gilt ab dem naechsten Umschalten, fuer beide Knoepfe.
    static void waehleProfil(const QString& name);
    /// Der Dialog der Profile (der Weg des Rechtsklicks).
    void oeffneProfile();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void schalte(bool stumm);
    void zeigeStand();

    ServiceContainer& m_dienste;
};

class MuteProfilDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MuteProfilDialog(QWidget* parent = nullptr);

    /// Was die Felder gerade zeigen.
    [[nodiscard]] MuteProfil profil() const;
    /// Die Felder auf dieses Profil stellen.
    void zeige(const MuteProfil& profil);
    /// Die Felder als eigenes Profil speichern und waehlen (der Weg von
    /// „Save As..."). false = ungueltiger Name oder Schreibfehler.
    bool speichereAls(const QString& name);

private:
    void fuelleListe(const QString& waehle);
    void ladeGewaehltes();
    [[nodiscard]] static Pegelform formVon(const QComboBox* wahl, const Pegelform& sonst);
    static void zeigeForm(QComboBox* wahl, const Pegelform& form);

    QComboBox* m_liste = nullptr;
    QDoubleSpinBox* m_pegel = nullptr;
    QDoubleSpinBox* m_aus = nullptr;
    QComboBox* m_ausForm = nullptr;
    QDoubleSpinBox* m_ein = nullptr;
    QComboBox* m_einForm = nullptr;
    QCheckBox* m_visuals = nullptr;
    QPushButton* m_loeschen = nullptr;
    MuteProfil m_geladen;  ///< traegt die Formen, die kein Preset der Liste sind
};

}  // namespace lumi::composer
