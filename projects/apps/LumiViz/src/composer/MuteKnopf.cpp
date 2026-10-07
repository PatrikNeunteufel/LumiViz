/**
 * @file MuteKnopf.cpp
 * @brief Composer: der Mute-Knopf und der Dialog der Mute-Profile (S101)
 */

#include "composer/MuteKnopf.hpp"

#include "audio/IAudioPlayer.hpp"
#include "composer/FormPresets.hpp"
#include "services/ServiceContainer.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>

namespace lumi::composer
{

// =============================================================================
// Der Knopf
// =============================================================================

MuteKnopf::MuteKnopf(ServiceContainer& dienste, QWidget* parent)
    : QPushButton(tr("Mute"), parent), m_dienste(dienste)
{
    setObjectName(QStringLiteral("muteKnopf"));
    setCheckable(true);
    // gedrueckt = abgesenkt: deutlich zu sehen, auch aus dem Augenwinkel
    setStyleSheet(QStringLiteral("QPushButton:checked { background-color: #b0461f; color: white; }"));
    MuteSchalter& schalter = MuteSchalter::instanz();
    connect(this, &QPushButton::clicked, this, [this](bool stumm) { schalte(stumm); });
    connect(&schalter, &MuteSchalter::geschaltet, this, [this](bool) { zeigeStand(); });
    zeigeStand();
}

void MuteKnopf::schalte(bool stumm)
{
    MuteSchalter& schalter = MuteSchalter::instanz();
    schalter.setPlayer(m_dienste.tryResolve<IAudioPlayer>());
    schalter.schalte(stumm);
    zeigeStand();
}

void MuteKnopf::zeigeStand()
{
    const MuteSchalter& schalter = MuteSchalter::instanz();
    setChecked(schalter.stumm());
    const QString name = muteprofile::gewaehlt();
    const MuteProfil profil = muteprofile::gewaehltesProfil();
    setToolTip(tr("Lowers the sound for an announcement and brings it back — the way the mute "
                  "profile says.\n\n"
                  "Profile: %1 (down to %2 % in %3 s, back in %4 s)\n\n"
                  "It works on everything you hear, with the playlist and with the Composer; this "
                  "button exists in the Player and in the Composer panel, both show the same "
                  "state. The loudspeaker button next to the volume slider is the plain mute and "
                  "stays as it is.\n\n"
                  "Right-click: choose a mute profile, edit the profiles.")
                   .arg(name.isEmpty() ? tr("(default)") : name)
                   .arg(qRound(profil.zielPegel * 100.0))
                   .arg(static_cast<double>(profil.ausblendeMs) / 1000.0, 0, 'f', 1)
                   .arg(static_cast<double>(profil.einblendeMs) / 1000.0, 0, 'f', 1));
}

void MuteKnopf::waehleProfil(const QString& name)
{
    muteprofile::setGewaehlt(name);
    MuteSchalter::instanz().ladeProfil();
}

void MuteKnopf::oeffneProfile()
{
    MuteProfilDialog dialog(this);
    dialog.exec();
    MuteSchalter::instanz().ladeProfil();
    zeigeStand();
}

void MuteKnopf::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    const QString gewaehlt = muteprofile::gewaehlt();
    QAction* vorgabe = menu.addAction(tr("(default)"));
    vorgabe->setCheckable(true);
    vorgabe->setChecked(gewaehlt.isEmpty());
    connect(vorgabe, &QAction::triggered, this, [this] {
        waehleProfil(QString());
        zeigeStand();
    });
    for (const muteprofile::Eintrag& e : muteprofile::liste())
    {
        QAction* a = menu.addAction(e.name);
        a->setCheckable(true);
        a->setChecked(QString::compare(e.name, gewaehlt, Qt::CaseInsensitive) == 0);
        connect(a, &QAction::triggered, this, [this, name = e.name] {
            waehleProfil(name);
            zeigeStand();
        });
    }
    menu.addSeparator();
    menu.addAction(tr("Edit mute profiles..."), this, [this] { oeffneProfile(); });
    menu.exec(event->globalPos());
}

// =============================================================================
// Der Dialog der Profile
// =============================================================================

namespace
{
/// Eintrag der Formen-Liste fuer eine Form, die kein Preset ist.
const QString kEigeneForm = QStringLiteral("<custom>");
}  // namespace

MuteProfilDialog::MuteProfilDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Mute Profiles"));
    auto* root = new QVBoxLayout(this);

    auto* oben = new QHBoxLayout();
    oben->addWidget(new QLabel(tr("Profile:"), this));
    m_liste = new QComboBox(this);
    m_liste->setObjectName(QStringLiteral("muteProfilListe"));
    m_liste->setToolTip(tr("The mute profile the Mute button uses. Built-in profiles come first, "
                           "your own follow."));
    oben->addWidget(m_liste, 1);
    root->addLayout(oben);

    auto* felder = new QFormLayout();
    m_pegel = new QDoubleSpinBox(this);
    m_pegel->setObjectName(QStringLiteral("mutePegel"));
    m_pegel->setRange(0.0, 100.0);
    m_pegel->setDecimals(0);
    m_pegel->setSuffix(QStringLiteral(" %"));
    m_pegel->setToolTip(tr("The level the sound is lowered to. 0 % = silent; 20 % keeps the music "
                           "quietly under an announcement."));
    felder->addRow(tr("Level while muted:"), m_pegel);

    const auto zeile = [this, felder](const QString& titel, QDoubleSpinBox*& dauer, QComboBox*& form,
                                      const char* nameDauer, const char* nameForm, const QString& tipp) {
        auto* reihe = new QHBoxLayout();
        dauer = new QDoubleSpinBox(this);
        dauer->setObjectName(QString::fromLatin1(nameDauer));
        dauer->setRange(0.0, 60.0);
        dauer->setDecimals(1);
        dauer->setSingleStep(0.1);
        dauer->setSuffix(QStringLiteral(" s"));
        dauer->setToolTip(tipp);
        reihe->addWidget(dauer);
        form = new QComboBox(this);
        form->setObjectName(QString::fromLatin1(nameForm));
        form->setToolTip(tr("The curve of this fade — the fade presets of the Composer."));
        for (const formpresets::Eintrag& e : formpresets::liste(FormArt::Blende)) form->addItem(e.name, e.name);
        reihe->addWidget(form, 1);
        felder->addRow(titel, reihe);
    };
    zeile(tr("Fade out:"), m_aus, m_ausForm, "muteAus", "muteAusForm",
          tr("How long the sound takes to go down. 0 = at once."));
    zeile(tr("Fade in:"), m_ein, m_einForm, "muteEin", "muteEinForm",
          tr("How long the sound takes to come back. 0 = at once."));

    m_visuals = new QCheckBox(tr("Visuals follow"), this);
    m_visuals->setObjectName(QStringLiteral("muteVisuals"));
    m_visuals->setToolTip(tr("Off: the visuals keep reacting to the full music while it is lowered.\n"
                             "On: they calm down with the sound."));
    felder->addRow(QString(), m_visuals);
    root->addLayout(felder);

    auto* unten = new QHBoxLayout();
    auto* sichern = new QPushButton(tr("Save As..."), this);
    sichern->setObjectName(QStringLiteral("muteSaveAs"));
    sichern->setToolTip(tr("Saves these settings as your own mute profile and selects it."));
    unten->addWidget(sichern);
    m_loeschen = new QPushButton(tr("Delete"), this);
    m_loeschen->setObjectName(QStringLiteral("muteDelete"));
    m_loeschen->setToolTip(tr("Deletes the selected profile. Only your own profiles can be deleted."));
    unten->addWidget(m_loeschen);
    unten->addStretch(1);
    auto* schliessen = new QPushButton(tr("Close"), this);
    schliessen->setDefault(true);
    unten->addWidget(schliessen);
    root->addLayout(unten);

    connect(schliessen, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_liste, QOverload<int>::of(&QComboBox::activated), this, [this](int) {
        muteprofile::setGewaehlt(m_liste->currentData().toString());
        ladeGewaehltes();
    });
    connect(sichern, &QPushButton::clicked, this, [this] {
        bool ok = false;
        const QString vorschlag =
            muteprofile::gewaehlt().isEmpty() ? tr("My mute") : muteprofile::gewaehlt();
        const QString name = QInputDialog::getText(this, tr("Save Mute Profile"), tr("Name:"),
                                                   QLineEdit::Normal, vorschlag, &ok);
        if (!ok) return;
        if (!speichereAls(name))
            QMessageBox::warning(this, tr("Save Mute Profile"), tr("Could not save \"%1\".").arg(name));
    });
    connect(m_loeschen, &QPushButton::clicked, this, [this] {
        const muteprofile::Eintrag e = muteprofile::finde(m_liste->currentData().toString());
        if (e.pfad.isEmpty() || e.mitgeliefert) return;
        if (QMessageBox::question(this, tr("Delete Mute Profile"),
                                  tr("Delete your mute profile \"%1\"?").arg(e.name)) != QMessageBox::Yes)
            return;
        if (muteprofile::loesche(e)) muteprofile::setGewaehlt(QString());
        fuelleListe(muteprofile::gewaehlt());
        ladeGewaehltes();
    });

    fuelleListe(muteprofile::gewaehlt());
    ladeGewaehltes();
}

void MuteProfilDialog::fuelleListe(const QString& waehle)
{
    m_liste->clear();
    m_liste->addItem(tr("(default)"), QString());
    for (const muteprofile::Eintrag& e : muteprofile::liste())
        m_liste->addItem(e.mitgeliefert ? e.name : tr("%1 (yours)").arg(e.name), e.name);
    int index = 0;
    for (int i = 0; i < m_liste->count(); ++i)
        if (QString::compare(m_liste->itemData(i).toString(), waehle, Qt::CaseInsensitive) == 0) index = i;
    m_liste->setCurrentIndex(index);
}

void MuteProfilDialog::ladeGewaehltes()
{
    zeige(muteprofile::gewaehltesProfil());
    const muteprofile::Eintrag e = muteprofile::finde(m_liste->currentData().toString());
    m_loeschen->setEnabled(!e.pfad.isEmpty() && !e.mitgeliefert);
}

void MuteProfilDialog::zeigeForm(QComboBox* wahl, const Pegelform& form)
{
    // ein frueherer Eintrag „(custom)" geht
    if (const int alt = wahl->findData(kEigeneForm); alt >= 0) wahl->removeItem(alt);
    for (int i = 0; i < wahl->count(); ++i)
    {
        Pegelform preset;
        const formpresets::Eintrag e = formpresets::finde(FormArt::Blende, wahl->itemData(i).toString());
        if (!e.pfad.isEmpty() && formpresets::lade(e.pfad, FormArt::Blende, preset) && gleich(preset, form))
        {
            wahl->setCurrentIndex(i);
            return;
        }
    }
    // keine der Listen-Formen: sie bleibt erhalten und heisst „(custom)"
    wahl->addItem(tr("(custom)"), kEigeneForm);
    wahl->setCurrentIndex(wahl->count() - 1);
}

Pegelform MuteProfilDialog::formVon(const QComboBox* wahl, const Pegelform& sonst)
{
    const QString name = wahl->currentData().toString();
    if (name == kEigeneForm) return sonst;
    Pegelform form;
    const formpresets::Eintrag e = formpresets::finde(FormArt::Blende, name);
    if (!e.pfad.isEmpty() && formpresets::lade(e.pfad, FormArt::Blende, form)) return form;
    return sonst;
}

void MuteProfilDialog::zeige(const MuteProfil& profil)
{
    m_geladen = profil;
    m_pegel->setValue(profil.zielPegel * 100.0);
    m_aus->setValue(static_cast<double>(profil.ausblendeMs) / 1000.0);
    m_ein->setValue(static_cast<double>(profil.einblendeMs) / 1000.0);
    zeigeForm(m_ausForm, profil.ausblendeForm);
    zeigeForm(m_einForm, profil.einblendeForm);
    m_visuals->setChecked(profil.visualsFolgen);
}

MuteProfil MuteProfilDialog::profil() const
{
    MuteProfil p;
    p.zielPegel = m_pegel->value() / 100.0;
    p.ausblendeMs = static_cast<std::int64_t>(m_aus->value() * 1000.0 + 0.5);
    p.einblendeMs = static_cast<std::int64_t>(m_ein->value() * 1000.0 + 0.5);
    p.ausblendeForm = formVon(m_ausForm, m_geladen.ausblendeForm);
    p.einblendeForm = formVon(m_einForm, m_geladen.einblendeForm);
    p.visualsFolgen = m_visuals->isChecked();
    p.richte();
    return p;
}

bool MuteProfilDialog::speichereAls(const QString& name)
{
    QString pfad;
    if (!muteprofile::speichere(name, profil(), &pfad)) return false;
    const QString gespeichert = QFileInfo(pfad).completeBaseName();
    muteprofile::setGewaehlt(gespeichert);
    fuelleListe(gespeichert);
    ladeGewaehltes();
    return true;
}

}  // namespace lumi::composer
