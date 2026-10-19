/**
 * @file AkkordDialog.hpp
 * @brief Loop: der Dialog *Edit chord* — ein eigenes Griffbild zeichnen oder ein
 *        vorhandenes bearbeiten (docs/audio/Saiten_Konzept.md S-T11 a I–P; Stufe 26e-3, S138)
 *
 * **I** die Stimmung steht fest (bei *New* die des Instruments, bei *Edit* die des
 * Griffbilds); **J** *First fret* ‹ ›; **K** *Title*, *Root*, *Type*, *Description*;
 * **L** die Toene und ein Vorschlag *sounds like: …* mit **Use name**; **M** die vier
 * Symbole zum Anhoeren (S-T9 a, dieselbe Spielart wie im Fenster); **N** Klick auf einen
 * Bund setzt oder nimmt den Punkt, Klick ueber dem Sattel ○ / ×, Rechtsklick auf einen
 * Punkt waehlt den Finger, einen Punkt ueber Saiten ziehen setzt ein Barre (die Regeln in
 * `Akkord.hpp`, `schaltePunkt` …); **O** Ctrl+Z / Ctrl+Y im Dialog; **P** *Cancel*,
 * *Save* — ein eigenes ersetzt sich, sonst entsteht ein neues `user:<name>`.
 *
 * Gezeigt wird ueber `lumi::ui::fuehreDialogAus` (Tests fangen ihn ab).
 */
#pragma once

#include "loop/Akkord.hpp"
#include "loop/GriffbildBild.hpp"

#include <QDialog>

#include <optional>
#include <vector>

class QButtonGroup;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

namespace lumi::loop
{

/// Das Griffbrett zum Setzen (N): meldet Klicks, Rechtsklicks und gezogene Barres.
class GriffbildEingabe : public GriffbildFeld
{
    Q_OBJECT

public:
    explicit GriffbildEingabe(QWidget* parent = nullptr);
    [[nodiscard]] QSize sizeHint() const override { return {220, 260}; }
    [[nodiscard]] QSize minimumSizeHint() const override { return {180, 210}; }
    /// Das Feld unter der Maus (gestrichelt); nichts = keins.
    [[nodiscard]] const std::optional<GriffbildLage::Treffer>& zeiger() const { return m_zeiger; }

Q_SIGNALS:
    void bundGeklickt(int saite, int bund);
    void obenGeklickt(int saite);
    void fingerVerlangt(int saite, QPoint global);
    void barreGezogen(int von, int bis, int bund);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    std::optional<GriffbildLage::Treffer> m_zeiger;
    std::optional<GriffbildLage::Treffer> m_druck;  ///< wo die linke Taste gedrueckt wurde
};

class AkkordDialog : public QDialog
{
    Q_OBJECT

public:
    /// `vorlage`: was bearbeitet wird — ein eigenes (`user:<name>`) ersetzt sich beim
    /// Speichern, alles andere wird ein neues eigenes. `saiten`, `strum`,
    /// `schritteJeSchlag` und `spielart` fuer das Anhoeren (M).
    AkkordDialog(const Akkord& vorlage, const Saitensatz& saiten, const Strum& strum, int schritteJeSchlag,
                 AkkordSpielart spielart, QWidget* parent = nullptr);

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    [[nodiscard]] const Akkord& akkord() const { return m_akkord; }
    void klickeBund(int saite, int bund);
    void klickeOben(int saite);
    void waehleFinger(int saite, int finger);
    void zieheBarre(int von, int bis, int bund);
    /// J: der erste gezeigte Bund (1 … `kBuendeMax`).
    void setzeErstenBund(int bund);
    [[nodiscard]] int ersterBund() const { return m_erster; }
    /// O: einen Schritt zurueck oder wieder vor. false = es gibt keinen.
    bool nimmZurueck();
    bool stelleWiederHer();
    void setzeTitel(const QString& titel);
    void setzeGrundton(int grundton);
    void setzeArt(const QString& art);
    void setzeBeschreibung(const QString& text);
    /// L: wie die gesetzten Toene klingen; nichts = kein Vorschlag.
    [[nodiscard]] std::optional<AkkordName> vorschlag() const;
    /// **Use name**: Grundton, Art und Titel aus dem Vorschlag. false = keiner.
    bool uebernimmVorschlag();
    bool spieleAn();
    void setzeSpielart(AkkordSpielart art, bool anspielen = false);
    /// P: speichern — false (mit Hinweis im Dialog), wenn keine Saite spielt oder die
    /// Datei nicht zu schreiben ist; sonst schliesst der Dialog angenommen.
    bool speichere();
    /// Was gespeichert wurde: `user:<name>`; leer = nichts.
    [[nodiscard]] const QString& gespeichert() const { return m_gespeichert; }
    [[nodiscard]] QString hinweis() const;
    [[nodiscard]] GriffbildEingabe* eingabe() const { return m_eingabe; }
    /// So viele Buende zeigt das Griffbrett (von Claude gesetzt).
    static constexpr int kBuendeImDialog = 5;

Q_SIGNALS:
    void vorhoerenVerlangt(const lumi::loop::Muster& noten);
    /// Die Spielart ist im Dialog gewechselt — das Fenster merkt sie (S-T10 a).
    void spielartGewaehlt(lumi::loop::AkkordSpielart art);

private:
    /// Eine Aenderung am Griffbild: den Stand vorher fuer Ctrl+Z merken.
    void aendere(const Akkord& neu);
    void zeige();
    void frageFinger(int saite, QPoint global);

    Akkord m_akkord;
    QString m_eigenName;  ///< ein eigenes, das sich ersetzt; leer = ein neues
    Saitensatz m_saiten;
    Strum m_strum;
    int m_jeSchlag = 4;
    AkkordSpielart m_spielart = AkkordSpielart::Ab;
    int m_erster = 1;
    std::vector<Akkord> m_zurueck;
    std::vector<Akkord> m_vor;
    QString m_gespeichert;

    GriffbildEingabe* m_eingabe = nullptr;
    QLabel* m_stimmung = nullptr;
    QLabel* m_ersterText = nullptr;
    QLineEdit* m_titel = nullptr;
    QComboBox* m_grundton = nullptr;
    QComboBox* m_art = nullptr;
    QLineEdit* m_beschreibung = nullptr;
    QLabel* m_toene = nullptr;
    QLabel* m_vorschlag = nullptr;
    QPushButton* m_nameNehmen = nullptr;
    QButtonGroup* m_spielarten = nullptr;
    QLabel* m_hinweis = nullptr;
};

}  // namespace lumi::loop
