/**
 * @file AkkordDialog.cpp
 * @brief Loop: der Dialog *Edit chord* (Saiten_Konzept.md S-T11 a; Stufe 26e-3, S138)
 */

#include "loop/AkkordDialog.hpp"

#include "UI/widgets/Bedienhilfe.hpp"  // zeigeMenue: in Tests abfragbar
#include "loop/AkkordFenster.hpp"      // baueSpielartKnoepfe
#include "loop/Instrument.hpp"         // kEigenVorsatz, tonName

#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>
#include <tuple>

namespace lumi::loop
{

// --- das Griffbrett zum Setzen (N) --------------------------------------------

GriffbildEingabe::GriffbildEingabe(QWidget* parent) : GriffbildFeld(parent)
{
    setObjectName(QStringLiteral("chordEditBoard"));
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Click a fret: set or remove the dot · click above the nut: open ○ / not played ×\n"
                  "Right-click a dot: finger 1–4 · drag a dot across strings: barre"));
}

void GriffbildEingabe::paintEvent(QPaintEvent* event)
{
    GriffbildFeld::paintEvent(event);
    if (!akkord().istGriffbild())
    {
        if (!m_zeigerTon) return;
        QPainter p(this);
        p.setPen(QPen(QColor(0xD8, 0x5A, 0x30), 1.2, Qt::DashLine));
        p.drawRect(klaviaturLage().taste(*m_zeigerTon).adjusted(1, 1, -1, -1));
        return;
    }
    if (!m_zeiger) return;
    // das Feld unter der Maus, gestrichelt (S-T11 a N)
    const GriffbildLage l = lage();
    const double r = l.halbmesser() * 1.35;
    const QPointF mitte = m_zeiger->bund == 0 ? l.oben(m_zeiger->saite) : l.punkt(m_zeiger->saite, m_zeiger->bund);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(0xD8, 0x5A, 0x30), 1.2, Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(mitte.x() - r, mitte.y() - r, 2 * r, 2 * r), 3, 3);
}

void GriffbildEingabe::mousePressEvent(QMouseEvent* event)
{
    if (!akkord().istGriffbild())
    {
        if (event->button() == Qt::LeftButton) m_druckTon = klaviaturLage().tonBei(event->position());
        return;
    }
    const std::optional<GriffbildLage::Treffer> t = lage().trifft(event->position());
    if (!t) return;
    if (event->button() == Qt::RightButton)
    {
        if (t->bund > 0 && akkord().buende[static_cast<std::size_t>(t->saite)] == t->bund)
            Q_EMIT fingerVerlangt(t->saite, event->globalPosition().toPoint());
        return;
    }
    if (event->button() == Qt::LeftButton) m_druck = t;
}

void GriffbildEingabe::mouseMoveEvent(QMouseEvent* event)
{
    if (!akkord().istGriffbild())
    {
        const std::optional<int> ton = klaviaturLage().tonBei(event->position());
        if (ton != m_zeigerTon)
        {
            m_zeigerTon = ton;
            update();
        }
        return;
    }
    const std::optional<GriffbildLage::Treffer> t = lage().trifft(event->position());
    if (t != m_zeiger)
    {
        m_zeiger = t;
        update();
    }
}

void GriffbildEingabe::mouseReleaseEvent(QMouseEvent* event)
{
    if (!akkord().istGriffbild())
    {
        const std::optional<int> druck = m_druckTon;
        m_druckTon.reset();
        if (event->button() == Qt::LeftButton && druck && klaviaturLage().tonBei(event->position()) == druck)
            Q_EMIT tonGeklickt(*druck);
        return;
    }
    if (event->button() != Qt::LeftButton || !m_druck) return;
    const GriffbildLage::Treffer druck = *m_druck;
    m_druck.reset();
    const std::optional<GriffbildLage::Treffer> los = lage().trifft(event->position());
    if (!los) return;
    // von einem Punkt ueber andere Saiten gezogen: ein Barre auf seinem Bund
    const bool vonPunkt = druck.bund > 0 && akkord().buende[static_cast<std::size_t>(druck.saite)] == druck.bund;
    if (vonPunkt && los->saite != druck.saite)
    {
        Q_EMIT barreGezogen(druck.saite, los->saite, druck.bund);
        return;
    }
    if (*los != druck) return;  // anderswo losgelassen: nichts
    if (druck.bund == 0)
        Q_EMIT obenGeklickt(druck.saite);
    else
        Q_EMIT bundGeklickt(druck.saite, druck.bund);
}

void GriffbildEingabe::leaveEvent(QEvent* event)
{
    m_zeiger.reset();
    m_zeigerTon.reset();
    update();
    GriffbildFeld::leaveEvent(event);
}

// --- der Dialog ----------------------------------------------------------------

AkkordDialog::AkkordDialog(const Akkord& vorlage, const Saitensatz& saiten, const Strum& strum, int schritteJeSchlag,
                           AkkordSpielart spielart, QWidget* parent, int tiefster)
    : QDialog(parent), m_akkord(vorlage), m_saiten(saiten), m_strum(strum), m_jeSchlag(std::max(1, schritteJeSchlag)),
      m_spielart(spielart), m_tiefster(tiefster)
{
    // die Klaviatur eines Voicings: drei Oktaven ab dem C eine Oktave unter dem tiefsten Ton
    m_klaviaturAb = std::max(12, m_tiefster - (m_tiefster % 12) - 12);
    setObjectName(QStringLiteral("chordEditDialog"));
    if (vorlage.name.startsWith(QLatin1String(kEigenVorsatz)))
        m_eigenName = vorlage.name.mid(static_cast<int>(qstrlen(kEigenVorsatz)));
    setWindowTitle(m_eigenName.isEmpty() ? tr("New chord — your chords") : tr("Edit chord — your chords"));
    richteAkkord(m_akkord);
    m_erster = griffbildFenster(m_akkord).erster;

    // O: das eigene Undo des Dialogs
    auto* zurueck = new QShortcut(QKeySequence::Undo, this);
    connect(zurueck, &QShortcut::activated, this, [this] { (void)nimmZurueck(); });
    auto* vor = new QShortcut(QKeySequence::Redo, this);
    connect(vor, &QShortcut::activated, this, [this] { (void)stelleWiederHer(); });

    auto* aussen = new QVBoxLayout(this);
    auto* mitte = new QHBoxLayout;
    aussen->addLayout(mitte, 1);

    // I, N, J: links das Griffbrett
    auto* links = new QVBoxLayout;
    m_stimmung = new QLabel(this);
    m_stimmung->setObjectName(QStringLiteral("chordEditTuning"));
    m_stimmung->setEnabled(false);
    QStringList toene;
    for (const int ton : m_akkord.leer) toene << tonName(ton);
    m_stimmung->setText(istVoicing() ? tr("Voicing — no strings")
                                     : tr("%1 · capo %2").arg(toene.join(QLatin1Char(' '))).arg(m_saiten.capo));
    m_stimmung->setToolTip(tr("The tuning is fixed: a chord shape belongs to one tuning"));
    links->addWidget(m_stimmung);
    m_eingabe = new GriffbildEingabe(this);
    links->addWidget(m_eingabe, 1);
    {
        auto* reihe = new QHBoxLayout;
        m_ersterName = new QLabel(istVoicing() ? tr("Keyboard from") : tr("First fret"), this);
        reihe->addWidget(m_ersterName);
        auto* tiefer = new QPushButton(QStringLiteral("‹"), this);
        tiefer->setObjectName(QStringLiteral("chordEditFretDown"));
        tiefer->setMaximumWidth(30);
        m_ersterText = new QLabel(this);
        m_ersterText->setObjectName(QStringLiteral("chordEditFirstFret"));
        m_ersterText->setMinimumWidth(22);
        m_ersterText->setAlignment(Qt::AlignCenter);
        auto* hoeher = new QPushButton(QStringLiteral("›"), this);
        hoeher->setObjectName(QStringLiteral("chordEditFretUp"));
        hoeher->setMaximumWidth(30);
        reihe->addWidget(tiefer);
        reihe->addWidget(m_ersterText);
        reihe->addWidget(hoeher);
        reihe->addStretch(1);
        links->addLayout(reihe);
        connect(tiefer, &QPushButton::clicked, this, [this] {
            if (istVoicing())
                setzeKlaviaturAb(m_klaviaturAb - 12);
            else
                setzeErstenBund(m_erster - 1);
        });
        connect(hoeher, &QPushButton::clicked, this, [this] {
            if (istVoicing())
                setzeKlaviaturAb(m_klaviaturAb + 12);
            else
                setzeErstenBund(m_erster + 1);
        });
    }
    mitte->addLayout(links);

    // K, L, M, N: rechts die Felder
    auto* rechts = new QVBoxLayout;
    auto* felder = new QFormLayout;
    m_titel = new QLineEdit(this);
    m_titel->setObjectName(QStringLiteral("chordEditTitle"));
    m_titel->setPlaceholderText(tr("C, Am7, C/E"));
    m_grundton = new QComboBox(this);
    m_grundton->setObjectName(QStringLiteral("chordEditRoot"));
    for (int i = 0; i < 12; ++i) m_grundton->addItem(grundtonNamen().at(i), i);
    m_art = new QComboBox(this);
    m_art->setObjectName(QStringLiteral("chordEditType"));
    m_art->setEditable(true);
    m_art->setToolTip(tr("The chord type, as the gallery sorts it; your own name is allowed"));
    for (const AkkordArt& art : akkordArten()) m_art->addItem(art.name);
    m_beschreibung = new QLineEdit(this);
    m_beschreibung->setObjectName(QStringLiteral("chordEditDescription"));
    m_beschreibung->setPlaceholderText(tr("Position 1, open"));
    {
        auto* wurzelUndArt = new QHBoxLayout;
        wurzelUndArt->addWidget(m_grundton);
        wurzelUndArt->addWidget(new QLabel(tr("Type"), this));
        wurzelUndArt->addWidget(m_art, 1);
        felder->addRow(tr("Title"), m_titel);
        felder->addRow(tr("Root"), wurzelUndArt);
        felder->addRow(tr("Description"), m_beschreibung);
    }
    rechts->addLayout(felder);
    m_toene = new QLabel(this);
    m_toene->setObjectName(QStringLiteral("chordEditNotes"));
    m_toene->setEnabled(false);
    rechts->addWidget(m_toene);
    {
        auto* reihe = new QHBoxLayout;
        m_vorschlag = new QLabel(this);
        m_vorschlag->setObjectName(QStringLiteral("chordEditSuggestion"));
        m_vorschlag->setStyleSheet(QStringLiteral("QLabel { color: #3B6D11; }"));
        m_nameNehmen = new QPushButton(tr("Use name"), this);
        m_nameNehmen->setObjectName(QStringLiteral("chordEditUseName"));
        m_nameNehmen->setToolTip(tr("Set root, type and title from what the notes sound like"));
        reihe->addWidget(m_vorschlag);
        reihe->addWidget(m_nameNehmen);
        reihe->addStretch(1);
        rechts->addLayout(reihe);
        connect(m_nameNehmen, &QPushButton::clicked, this, [this] { (void)uebernimmVorschlag(); });
    }
    {
        auto* reihe = new QHBoxLayout;
        m_spielarten = baueSpielartKnoepfe(this, reihe);
        m_play = new QPushButton(tr("Play"), this);
        m_play->setObjectName(QStringLiteral("chordPlay"));
        reihe->addWidget(m_play);
        connect(m_play, &QPushButton::clicked, this, [this] { (void)spieleAn(); });
        // ein Voicing klingt ohne Saiten zugleich: ein Knopf statt der Spielarten (S-T14 a G)
        m_play->setVisible(istVoicing());
        for (QAbstractButton* knopf : m_spielarten->buttons()) knopf->setVisible(!istVoicing());
        reihe->addStretch(1);
        rechts->addLayout(reihe);
        if (QAbstractButton* knopf = m_spielarten->button(static_cast<int>(m_spielart))) knopf->setChecked(true);
        connect(m_spielarten, &QButtonGroup::idClicked, this,
                [this](int id) { setzeSpielart(static_cast<AkkordSpielart>(id), true); });
    }
    auto* bedienung = new QLabel(istVoicing() ? tr("Click a key: add or remove the note")
                                              : tr("Click a fret: set or remove the dot · click above the nut: open ○ / not played × · "
                                                   "right-click a dot: finger · drag a dot across strings: barre"),
                                 this);
    bedienung->setEnabled(false);
    bedienung->setWordWrap(true);
    rechts->addWidget(bedienung);
    rechts->addStretch(1);
    mitte->addLayout(rechts, 1);

    // O, P: unten
    m_hinweis = new QLabel(this);
    m_hinweis->setObjectName(QStringLiteral("chordEditHint"));
    m_hinweis->setStyleSheet(QStringLiteral("QLabel { color: #A32D2D; }"));
    m_hinweis->hide();
    auto* knoepfe = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    knoepfe->button(QDialogButtonBox::Save)->setObjectName(QStringLiteral("chordEditSave"));
    connect(knoepfe, &QDialogButtonBox::accepted, this, [this] { (void)speichere(); });
    connect(knoepfe, &QDialogButtonBox::rejected, this, &QDialog::reject);
    auto* unten = new QHBoxLayout;
    auto* undoHinweis = new QLabel(tr("Ctrl+Z / Ctrl+Y in the dialog"), this);
    undoHinweis->setEnabled(false);
    unten->addWidget(undoHinweis);
    unten->addStretch(1);
    unten->addWidget(m_hinweis);
    unten->addWidget(knoepfe);
    aussen->addLayout(unten);

    // die Felder aus der Vorlage
    m_titel->setText(vorlage.titel);
    m_grundton->setCurrentIndex(m_akkord.grundton);
    m_art->setCurrentText(m_akkord.art);
    m_beschreibung->setText(vorlage.beschreibung);

    connect(m_eingabe, &GriffbildEingabe::bundGeklickt, this, &AkkordDialog::klickeBund);
    connect(m_eingabe, &GriffbildEingabe::obenGeklickt, this, &AkkordDialog::klickeOben);
    connect(m_eingabe, &GriffbildEingabe::fingerVerlangt, this, &AkkordDialog::frageFinger);
    connect(m_eingabe, &GriffbildEingabe::barreGezogen, this, &AkkordDialog::zieheBarre);
    connect(m_eingabe, &GriffbildEingabe::tonGeklickt, this, &AkkordDialog::klickeTon);
    if (istVoicing()) m_eingabe->setToolTip(tr("Click a key: add or remove the note"));
    resize(560, 360);
    zeige();
}

void AkkordDialog::aendere(const Akkord& neu, int tiefster)
{
    if (neu.buende == m_akkord.buende && neu.finger == m_akkord.finger && neu.barre == m_akkord.barre &&
        neu.abstaende == m_akkord.abstaende && tiefster == m_tiefster)
        return;
    m_zurueck.emplace_back(m_akkord, m_tiefster);
    m_vor.clear();
    m_akkord.buende = neu.buende;
    m_akkord.finger = neu.finger;
    m_akkord.barre = neu.barre;
    m_akkord.abstaende = neu.abstaende;
    m_tiefster = tiefster;
    m_hinweis->hide();
    zeige();
}

void AkkordDialog::klickeBund(int saite, int bund)
{
    Akkord neu = m_akkord;
    schaltePunkt(neu, saite, bund);
    aendere(neu, m_tiefster);
}

void AkkordDialog::klickeOben(int saite)
{
    Akkord neu = m_akkord;
    schalteLeer(neu, saite);
    aendere(neu, m_tiefster);
}

void AkkordDialog::waehleFinger(int saite, int finger)
{
    Akkord neu = m_akkord;
    setzeFinger(neu, saite, finger);
    aendere(neu, m_tiefster);
}

void AkkordDialog::zieheBarre(int von, int bis, int bund)
{
    Akkord neu = m_akkord;
    setzeBarre(neu, von, bis, bund);
    aendere(neu, m_tiefster);
}

void AkkordDialog::klickeTon(int hoehe)
{
    Akkord neu = m_akkord;
    int tiefster = m_tiefster;
    schalteVoicingTon(neu, tiefster, hoehe);
    aendere(neu, tiefster);
}

void AkkordDialog::setzeKlaviaturAb(int ton)
{
    const int neu = std::clamp(ton - (((ton % 12) + 12) % 12), 12, kHoeheMax - 35);
    if (neu == m_klaviaturAb) return;
    m_klaviaturAb = neu;
    zeige();
}

void AkkordDialog::frageFinger(int saite, QPoint global)
{
    QMenu menue(this);
    menue.setObjectName(QStringLiteral("chordFingerMenu"));
    const int jetzt = m_akkord.finger[static_cast<std::size_t>(saite)];
    for (int f = 0; f <= 4; ++f)
    {
        QAction* a = menue.addAction(f == 0 ? tr("No finger") : tr("Finger %1").arg(f));
        a->setCheckable(true);
        a->setChecked(f == jetzt);
        connect(a, &QAction::triggered, this, [this, saite, f] { waehleFinger(saite, f); });
    }
    lumi::ui::zeigeMenue(menue, global);
}

void AkkordDialog::setzeErstenBund(int bund)
{
    const int neu = std::clamp(bund, 1, kBuendeMax - kBuendeImDialog + 1);
    if (neu == m_erster) return;
    m_erster = neu;
    zeige();
}

bool AkkordDialog::nimmZurueck()
{
    if (m_zurueck.empty()) return false;
    m_vor.emplace_back(m_akkord, m_tiefster);
    std::tie(m_akkord, m_tiefster) = m_zurueck.back();
    m_zurueck.pop_back();
    zeige();
    return true;
}

bool AkkordDialog::stelleWiederHer()
{
    if (m_vor.empty()) return false;
    m_zurueck.emplace_back(m_akkord, m_tiefster);
    std::tie(m_akkord, m_tiefster) = m_vor.back();
    m_vor.pop_back();
    zeige();
    return true;
}

void AkkordDialog::setzeTitel(const QString& titel)
{
    m_titel->setText(titel);
}

void AkkordDialog::setzeGrundton(int grundton)
{
    m_grundton->setCurrentIndex(((grundton % 12) + 12) % 12);
}

void AkkordDialog::setzeArt(const QString& art)
{
    m_art->setCurrentText(art);
}

void AkkordDialog::setzeBeschreibung(const QString& text)
{
    m_beschreibung->setText(text);
}

std::optional<AkkordName> AkkordDialog::vorschlag() const
{
    Saitensatz leer;
    leer.leer = m_akkord.leer;
    std::vector<int> hoehen;
    if (const auto toene = istVoicing() ? toeneDesVoicings(m_akkord, m_tiefster) : toeneDesGriffbilds(m_akkord, leer))
        for (const AkkordTon& t : *toene) hoehen.push_back(t.hoehe);
    return erkenneAkkord(hoehen);
}

bool AkkordDialog::uebernimmVorschlag()
{
    const std::optional<AkkordName> name = vorschlag();
    if (!name) return false;
    m_grundton->setCurrentIndex(name->grundton);
    m_art->setCurrentText(akkordArtText(*name));
    m_titel->setText(akkordSymbol(*name));
    return true;
}

bool AkkordDialog::spieleAn()
{
    const Muster noten = akkordZumAnhoeren(m_akkord, m_saiten, m_strum, m_spielart, m_jeSchlag, m_tiefster);
    if (noten.noten.empty()) return false;
    Q_EMIT vorhoerenVerlangt(noten);
    return true;
}

void AkkordDialog::setzeSpielart(AkkordSpielart art, bool anspielen)
{
    const bool anders = art != m_spielart;
    m_spielart = art;
    if (QAbstractButton* knopf = m_spielarten->button(static_cast<int>(art)); knopf != nullptr && !knopf->isChecked())
        knopf->setChecked(true);
    if (anders) Q_EMIT spielartGewaehlt(art);
    if (anspielen) (void)spieleAn();
}

bool AkkordDialog::speichere()
{
    const bool spielt = istVoicing() ? !m_akkord.abstaende.empty()
                                     : std::any_of(m_akkord.buende.begin(), m_akkord.buende.end(), [](int b) { return b >= 0; });
    if (!spielt)
    {
        m_hinweis->setText(istVoicing() ? tr("Set at least one note") : tr("Set at least one string to play"));
        m_hinweis->show();
        return false;
    }
    Akkord fertig = m_akkord;
    fertig.grundton = m_grundton->currentIndex();
    fertig.art = m_art->currentText().trimmed().isEmpty() ? QStringLiteral("major") : m_art->currentText().trimmed();
    fertig.titel = m_titel->text().trimmed();
    if (fertig.titel.isEmpty())
        fertig.titel = akkordSymbol(AkkordName{fertig.grundton, fertig.art, -1});  // ohne Titel: das Symbol
    fertig.beschreibung = m_beschreibung->text().trimmed();
    fertig.herkunft.clear();  // selbst gezeichnet (S-T4 a: nur Vorlagen tragen `computed`)
    const QString name = m_eigenName.isEmpty() ? freierAkkordName(fertig.titel) : m_eigenName;
    const QString datei = eigeneAkkordDatei(name);
    if (datei.isEmpty() || !speichereAkkord(datei, fertig))
    {
        m_hinweis->setText(tr("Could not write %1").arg(datei.isEmpty() ? name : datei));
        m_hinweis->show();
        return false;
    }
    m_gespeichert = QLatin1String(kEigenVorsatz) + name;
    accept();
    return true;
}

QString AkkordDialog::hinweis() const
{
    return m_hinweis->isVisibleTo(this) ? m_hinweis->text() : QString();
}

void AkkordDialog::zeige()
{
    m_eingabe->setzeAkkord(m_akkord, m_tiefster);
    m_eingabe->setzeFenster(GriffbildFenster{m_erster, kBuendeImDialog});
    m_eingabe->setzeKlaviatur(std::make_pair(m_klaviaturAb, m_klaviaturAb + 35));
    m_ersterText->setText(istVoicing() ? tonName(m_klaviaturAb) : QString::number(m_erster));
    QStringList namen;
    if (const auto toene = istVoicing() ? toeneDesVoicings(m_akkord, m_tiefster) : toeneDesGriffbilds(m_akkord, m_saiten))
        for (const AkkordTon& t : *toene) namen << tonName(t.hoehe);
    m_toene->setText(namen.isEmpty() ? (istVoicing() ? tr("Notes: none — click a key") : tr("Notes: none — set a string to play"))
                                     : tr("Notes: %1").arg(namen.join(QLatin1Char(' '))));
    const std::optional<AkkordName> name = vorschlag();
    m_vorschlag->setText(name ? tr("sounds like: %1").arg(akkordSymbol(*name)) : QString());
    m_vorschlag->setVisible(name.has_value());
    m_nameNehmen->setVisible(name.has_value());
}

}  // namespace lumi::loop
