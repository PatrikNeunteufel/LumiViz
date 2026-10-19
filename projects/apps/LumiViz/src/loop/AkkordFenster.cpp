/**
 * @file AkkordFenster.cpp
 * @brief Loop: das Fenster *Chords* (Saiten_Konzept.md S-T8 a bis S-T10 a, S-T12 b; Stufe 26e-2, S138)
 */

#include "loop/AkkordFenster.hpp"

#include "loop/GriffbildBild.hpp"
#include "loop/Instrument.hpp"  // kEigenVorsatz, tonName

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDrag>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSplitter>
#include <QToolButton>
#include <QToolTip>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace lumi::loop
{

namespace
{

const char* const kFavoriten = "chordWindow/favorites";
const char* const kZuletzt = "chordWindow/recent";
const char* const kSpielart = "chordWindow/playAs";
const char* const kGeometrie = "chordWindow/geometry";

constexpr int kRolleOrdner = Qt::UserRole + 1;

bool eigenerAkkord(const QString& nennung)
{
    return nennung.startsWith(QLatin1String(kEigenVorsatz));
}

bool& spielartMerken()
{
    static bool an = true;
    return an;
}

const QString kOrdnerFavoriten = QStringLiteral("favorites");
const QString kOrdnerZuletzt = QStringLiteral("recent");
const QString kOrdnerEigene = QStringLiteral("own");

QString wurzelOrdner(int grundton)
{
    return QStringLiteral("root:%1").arg(grundton);
}

QString artOrdner(int grundton, const QString& art)
{
    return QStringLiteral("root:%1:%2").arg(grundton).arg(art);
}

/// Wo eine Art in der Reihe steht: die Arten der Tabelle in ihrer Reihenfolge,
/// dahinter alle anderen (Slash, `alt`, `7sg`).
int rangDerArt(const QString& art)
{
    const std::vector<AkkordArt>& arten = akkordArten();
    for (std::size_t i = 0; i < arten.size(); ++i)
        if (arten[i].name == art) return static_cast<int>(i);
    return static_cast<int>(arten.size());
}

bool artVor(const QString& a, const QString& b)
{
    const int ra = rangDerArt(a);
    const int rb = rangDerArt(b);
    return ra != rb ? ra < rb : a < b;
}

/// Der Kopf einer Gruppe: „C major", „C minor", sonst das Akkordsymbol („C7", „C/E").
QString kopfVon(const Akkord& akkord)
{
    const QString ton = grundtonNamen().at(akkord.grundton);
    if (akkord.art == QLatin1String("major")) return ton + QStringLiteral(" major");
    if (akkord.art == QLatin1String("minor")) return ton + QStringLiteral(" minor");
    return akkord.titel;
}

QString tonListe(const std::vector<int>& klassen)
{
    QStringList namen;
    for (const int k : klassen) namen << grundtonNamen().at(k);
    return namen.join(QStringLiteral(", "));
}

}  // namespace

// --- die Galerie (D) ---------------------------------------------------------

AkkordGalerie::AkkordGalerie(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("chordGallery"));
    setMouseTracking(false);
}

void AkkordGalerie::setzeGruppen(std::vector<Gruppe> gruppen)
{
    m_gruppen = std::move(gruppen);
    ordne();
    update();
}

void AkkordGalerie::setzeGewaehlt(const QString& nennung)
{
    m_gewaehlt = nennung;
    update();
}

const Akkord* AkkordGalerie::akkordVon(const QString& nennung) const
{
    for (const Gruppe& g : m_gruppen)
        for (const Akkord& a : g.akkorde)
            if (a.name == nennung) return &a;
    return nullptr;
}

void AkkordGalerie::ordne()
{
    m_plaetze.clear();
    const int spalten = std::max(1, (width() - 2 * kRand) / kZelleBreite);
    int y = kRand;
    for (std::size_t g = 0; g < m_gruppen.size(); ++g)
    {
        m_plaetze.push_back({QRect(kRand, y, std::max(1, width() - 2 * kRand), kKopfHoehe), static_cast<int>(g), -1});
        y += kKopfHoehe;
        const int zahl = static_cast<int>(m_gruppen[g].akkorde.size());
        for (int i = 0; i < zahl; ++i)
            m_plaetze.push_back({QRect(kRand + (i % spalten) * kZelleBreite, y + (i / spalten) * kZelleHoehe, kZelleBreite,
                                       kZelleHoehe),
                                 static_cast<int>(g), i});
        y += ((zahl + spalten - 1) / spalten) * kZelleHoehe + 4;
    }
    const int hoehe = y + kRand;
    if (minimumHeight() != hoehe) setMinimumHeight(hoehe);  // in der Rolle: so hoch wie der Inhalt
}

QRect AkkordGalerie::zelle(const QString& nennung) const
{
    for (const Platz& p : m_plaetze)
        if (p.akkord >= 0 && m_gruppen[static_cast<std::size_t>(p.gruppe)].akkorde[static_cast<std::size_t>(p.akkord)].name == nennung)
            return p.rechteck;
    return {};
}

QString AkkordGalerie::nennungBei(QPoint ort) const
{
    for (const Platz& p : m_plaetze)
        if (p.akkord >= 0 && p.rechteck.contains(ort))
            return m_gruppen[static_cast<std::size_t>(p.gruppe)].akkorde[static_cast<std::size_t>(p.akkord)].name;
    return {};
}

void AkkordGalerie::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), palette().color(QPalette::Base));
    QColor blass = palette().color(QPalette::Text);
    blass.setAlpha(150);
    QFont fett = font();
    fett.setBold(true);
    QFont klein = font();
    klein.setPointSizeF(std::max(6.0, font().pointSizeF() * 0.85));
    for (const Platz& platz : m_plaetze)
    {
        const Gruppe& g = m_gruppen[static_cast<std::size_t>(platz.gruppe)];
        if (platz.akkord < 0)
        {
            p.setFont(fett);
            p.setPen(palette().color(QPalette::Text));
            p.drawText(platz.rechteck.adjusted(2, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, g.titel);
            continue;
        }
        const Akkord& a = g.akkorde[static_cast<std::size_t>(platz.akkord)];
        const QRect r = platz.rechteck.adjusted(2, 2, -2, -2);
        if (a.name == m_gewaehlt)
        {
            QColor wahl = palette().color(QPalette::Highlight);
            wahl.setAlpha(60);
            p.setPen(QPen(palette().color(QPalette::Highlight), 1.5));
            p.setBrush(wahl);
            p.drawRoundedRect(QRectF(r).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
        }
        const QRectF bild(r.left() + (r.width() - kBildBreite) / 2.0, r.top() + 1, kBildBreite, kBildHoehe);
        zeichneGriffbild(p, bild, a, GriffbildGroesse::Klein, palette());
        p.setFont(klein);
        p.setPen(palette().color(QPalette::Text));
        const int zeile = p.fontMetrics().height();
        p.drawText(QRect(r.left(), static_cast<int>(bild.bottom()) + 1, r.width(), zeile), Qt::AlignHCenter | Qt::AlignTop,
                   p.fontMetrics().elidedText(a.titel, Qt::ElideRight, r.width()));
        p.setPen(blass);
        p.drawText(QRect(r.left(), static_cast<int>(bild.bottom()) + 1 + zeile, r.width(), zeile),
                   Qt::AlignHCenter | Qt::AlignTop, lageText(a));
    }
}

void AkkordGalerie::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return QWidget::mousePressEvent(event);
    const QString nennung = nennungBei(event->position().toPoint());
    m_druck = event->position().toPoint();
    m_druckNennung = nennung;
    // markiert wird ueber das Fenster (es waehlt und setzt die Verschiebung zurueck)
    if (!nennung.isEmpty()) Q_EMIT angeklickt(nennung);
}

void AkkordGalerie::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton) || m_druckNennung.isEmpty()) return;
    if ((event->position().toPoint() - m_druck).manhattanLength() < QApplication::startDragDistance()) return;
    const QString nennung = m_druckNennung;
    m_druckNennung.clear();
    Q_EMIT ziehen(nennung);
}

void AkkordGalerie::mouseDoubleClickEvent(QMouseEvent* event)
{
    const QString nennung = nennungBei(event->position().toPoint());
    if (event->button() == Qt::LeftButton && !nennung.isEmpty()) Q_EMIT aktiviert(nennung);
}

void AkkordGalerie::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    ordne();
}

bool AkkordGalerie::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        auto* hilfe = static_cast<QHelpEvent*>(event);
        const Akkord* a = akkordVon(nennungBei(hilfe->pos()));
        if (a == nullptr)
            QToolTip::hideText();
        else
            QToolTip::showText(hilfe->globalPos(), QStringLiteral("%1 · %2").arg(a->titel, a->beschreibung), this);
        return true;
    }
    return QWidget::event(event);
}

// --- das Fenster ---------------------------------------------------------------

void AkkordFenster::setzeSpielartMerken(bool an)
{
    spielartMerken() = an;
}

AkkordFenster::AkkordFenster(QWidget* parent) : QWidget(parent, Qt::Window)
{
    setObjectName(QStringLiteral("chordWindow"));
    setWindowTitle(tr("Chords"));

    // Ctrl+Z / Ctrl+Y: das Fenster ist ein eigenes (wie Instruments, Z1 a)
    auto* zurueck = new QShortcut(QKeySequence::Undo, this);
    connect(zurueck, &QShortcut::activated, this, &AkkordFenster::nimmZurueck);
    auto* vor = new QShortcut(QKeySequence::Redo, this);
    connect(vor, &QShortcut::activated, this, &AkkordFenster::stelleWiederHer);

    auto* aussen = new QVBoxLayout(this);
    // A: fuer wen
    m_zielZeile = new QLabel(this);
    m_zielZeile->setObjectName(QStringLiteral("chordTarget"));
    m_zielZeile->setEnabled(false);
    aussen->addWidget(m_zielZeile);
    // B: suchen, Art
    m_suche = new QLineEdit(this);
    m_suche->setObjectName(QStringLiteral("chordSearch"));
    m_suche->setPlaceholderText(tr("Search: Cm7, sus, C/E"));
    m_suche->setClearButtonEnabled(true);
    m_suche->setToolTip(tr("Searches all chords for this tuning: name, type and position"));
    m_art = new QComboBox(this);
    m_art->setObjectName(QStringLiteral("chordType"));
    m_art->setMinimumWidth(110);
    m_art->setToolTip(tr("Show only chords of this type"));
    {
        auto* reihe = new QHBoxLayout;
        reihe->addWidget(m_suche, 1);
        reihe->addWidget(new QLabel(tr("Type"), this));
        reihe->addWidget(m_art);
        aussen->addLayout(reihe);
    }
    // C und D: Baum und Galerie
    auto* teilung = new QSplitter(Qt::Horizontal, this);
    m_baum = new QTreeWidget(teilung);
    m_baum->setObjectName(QStringLiteral("chordFolders"));
    m_baum->setHeaderHidden(true);
    m_baum->setColumnCount(1);
    m_baum->setUniformRowHeights(true);
    m_rolle = new QScrollArea(teilung);
    m_rolle->setWidgetResizable(true);
    m_rolle->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_galerie = new AkkordGalerie(m_rolle);
    m_rolle->setWidget(m_galerie);
    teilung->addWidget(m_baum);
    teilung->addWidget(m_rolle);
    teilung->setStretchFactor(0, 0);
    teilung->setStretchFactor(1, 1);
    teilung->setSizes({170, 470});
    aussen->addWidget(teilung, 1);

    // E, F, G: die Leiste
    auto* leiste = new QHBoxLayout;
    m_feld = new GriffbildFeld(this);
    m_feld->setFixedSize(120, 140);
    leiste->addWidget(m_feld);
    auto* mitte = new QVBoxLayout;
    m_titel = new QLabel(this);
    m_titel->setObjectName(QStringLiteral("chordTitle"));
    m_titel->setTextFormat(Qt::RichText);
    m_toene = new QLabel(this);
    m_toene->setObjectName(QStringLiteral("chordNotes"));
    m_toene->setEnabled(false);
    m_gerechnet = new QLabel(tr("computed"), this);
    m_gerechnet->setObjectName(QStringLiteral("chordComputed"));
    m_gerechnet->setToolTip(tr("No source has this tuning: the fingering is computed by a fixed rule\n"
                               "(chord tones only, root in the bass, at most four frets)"));
    m_gerechnet->setStyleSheet(QStringLiteral("QLabel { color: #854F0B; }"));
    m_ungewoehnlich = new QLabel(this);
    m_ungewoehnlich->setObjectName(QStringLiteral("chordUnusual"));
    m_ungewoehnlich->setToolTip(tr("This position from the source plays notes outside its chord type"));
    m_ungewoehnlich->setStyleSheet(QStringLiteral("QLabel { color: #A32D2D; }"));
    m_tiefer = new QPushButton(tr("−1 fret"), this);
    m_tiefer->setObjectName(QStringLiteral("chordMoveDown"));
    m_tiefer->setToolTip(tr("Move the shape one fret toward the nut"));
    m_hoeher = new QPushButton(tr("+1 fret"), this);
    m_hoeher->setObjectName(QStringLiteral("chordMoveUp"));
    m_hoeher->setToolTip(tr("Move the shape one fret up the neck; open strings are fretted along"));
    mitte->addWidget(m_titel);
    mitte->addWidget(m_toene);
    {
        auto* zeichen = new QHBoxLayout;
        zeichen->addWidget(m_gerechnet);
        zeichen->addWidget(m_ungewoehnlich);
        zeichen->addStretch(1);
        mitte->addLayout(zeichen);
    }
    {
        auto* schieben = new QHBoxLayout;
        schieben->addWidget(new QLabel(tr("Move"), this));
        schieben->addWidget(m_tiefer);
        schieben->addWidget(m_hoeher);
        schieben->addStretch(1);
        mitte->addLayout(schieben);
    }
    mitte->addStretch(1);
    leiste->addLayout(mitte, 1);
    auto* rechts = new QVBoxLayout;
    {
        auto* spielen = new QHBoxLayout;
        m_spielarten = new QButtonGroup(this);
        m_spielarten->setExclusive(true);
        const struct
        {
            AkkordSpielart art;
            const char* zeichen;
            const char* name;
            const char* hilfe;
        } kSymbole[] = {
            {AkkordSpielart::Ab, "↓", "chordPlayDown", "Down — a click in the gallery plays this"},
            {AkkordSpielart::Auf, "↑", "chordPlayUp", "Up"},
            {AkkordSpielart::AbAuf, "↓↑", "chordPlayDownUp", "Down-up: down, then up half a beat later"},
            {AkkordSpielart::Zugleich, "=", "chordPlayTogether", "Together: all strings at once"},
        };
        for (const auto& s : kSymbole)
        {
            auto* knopf = new QToolButton(this);
            knopf->setObjectName(QLatin1String(s.name));
            knopf->setText(QString::fromUtf8(s.zeichen));
            knopf->setCheckable(true);
            knopf->setMinimumWidth(30);
            knopf->setToolTip(tr(s.hilfe) + tr("\nThe choice stays for the next clicks"));
            m_spielarten->addButton(knopf, static_cast<int>(s.art));
            spielen->addWidget(knopf);
        }
        m_favorit = new QToolButton(this);
        m_favorit->setObjectName(QStringLiteral("chordFavorite"));
        m_favorit->setCheckable(true);
        m_favorit->setText(QString::fromUtf8("★"));
        m_favorit->setToolTip(tr("Favorite this position (shown under Favorites)"));
        spielen->addSpacing(8);
        spielen->addWidget(m_favorit);
        m_use = new QPushButton(tr("Use"), this);
        m_use->setObjectName(QStringLiteral("chordUse"));
        m_use->setDefault(true);
        spielen->addWidget(m_use);
        rechts->addLayout(spielen);
    }
    rechts->addStretch(1);
    leiste->addLayout(rechts);
    aussen->addLayout(leiste);
    auto* hinweis = new QLabel(tr("Click: listen · double-click or Use: insert at the insert marker · "
                                  "drag a shape onto the tab lines or the piano roll"),
                               this);
    hinweis->setEnabled(false);
    hinweis->setWordWrap(true);
    aussen->addWidget(hinweis);

    {
        QSettings einstellungen;
        m_favoriten = einstellungen.value(QLatin1String(kFavoriten)).toStringList();
        m_zuletzt = einstellungen.value(QLatin1String(kZuletzt)).toStringList();
        if (spielartMerken())
            m_spielart = static_cast<AkkordSpielart>(
                std::clamp(einstellungen.value(QLatin1String(kSpielart), 0).toInt(), 0, static_cast<int>(AkkordSpielart::Zugleich)));
        const QByteArray geometrie = einstellungen.value(QLatin1String(kGeometrie)).toByteArray();
        if (geometrie.isEmpty() || !restoreGeometry(geometrie)) resize(680, 620);
    }
    if (QAbstractButton* knopf = m_spielarten->button(static_cast<int>(m_spielart))) knopf->setChecked(true);

    connect(m_suche, &QLineEdit::textChanged, this, [this] { zeigeGalerie(); });
    connect(m_art, &QComboBox::activated, this, [this] { zeigeGalerie(); });
    connect(m_baum, &QTreeWidget::currentItemChanged, this, [this] { zeigeGalerie(); });
    connect(m_galerie, &AkkordGalerie::angeklickt, this, [this](const QString& nennung) {
        // ein anderes Bild: waehlen, die Verschiebung beginnt bei 0; dasselbe behaelt sie
        if (nennung != gewaehlt()) (void)waehle(nennung);
        (void)spieleAn();
    });
    connect(m_galerie, &AkkordGalerie::aktiviert, this, [this](const QString&) { (void)verwende(); });
    connect(m_galerie, &AkkordGalerie::ziehen, this, [this](const QString& nennung) { zieheAb(nennung); });
    connect(m_tiefer, &QPushButton::clicked, this, [this] { (void)verschiebe(-1); });
    connect(m_hoeher, &QPushButton::clicked, this, [this] { (void)verschiebe(1); });
    connect(m_spielarten, &QButtonGroup::idClicked, this,
            [this](int id) { setzeSpielart(static_cast<AkkordSpielart>(id), true); });
    connect(m_favorit, &QToolButton::clicked, this, [this](bool an) { setzeFavorit(an); });
    connect(m_use, &QPushButton::clicked, this, [this] { (void)verwende(); });

    setzeZiel(QString(), QString(), Saitensatz{}, Strum{}, 4);
}

void AkkordFenster::setzeZiel(const QString& muster, const QString& instrumentTitel, const Saitensatz& saiten,
                              const Strum& strum, int schritteJeSchlag)
{
    const bool andereStimmung = saiten.leer != m_saiten.leer;
    m_muster = muster;
    m_saiten = saiten;
    m_strum = strum;
    m_jeSchlag = std::max(1, schritteJeSchlag);
    QStringList toene;
    for (const int ton : saiten.leer) toene << tonName(ton);
    m_zielZeile->setText(muster.isEmpty() ? tr("Open an instrument pattern with strings to use a chord")
                                          : tr("For: %1 · %2 — %3, capo %4")
                                                .arg(instrumentTitel, muster, toene.join(QLatin1Char(' ')))
                                                .arg(saiten.capo));
    if (andereStimmung) liesNeu();
    zeigeDetail();
}

void AkkordFenster::liesNeu()
{
    {
        QSettings einstellungen;
        m_favoriten = einstellungen.value(QLatin1String(kFavoriten)).toStringList();
        m_zuletzt = einstellungen.value(QLatin1String(kZuletzt)).toStringList();
    }
    m_alle = passendeAkkorde(mitgelieferteAkkorde(), m_saiten);
    for (Akkord& eigener : passendeAkkorde(eigeneAkkorde(), m_saiten)) m_alle.push_back(std::move(eigener));
    {
        // die Arten, die es zu dieser Stimmung gibt — die Wahl bleibt, wenn es sie noch gibt
        const QString vorher = m_art->currentData().toString();
        std::vector<QString> arten;
        for (const Akkord& a : m_alle)
            if (std::find(arten.begin(), arten.end(), a.art) == arten.end()) arten.push_back(a.art);
        std::sort(arten.begin(), arten.end(), artVor);
        const QSignalBlocker ruhe(m_art);
        m_art->clear();
        m_art->addItem(tr("All types"), QString());
        for (const QString& art : arten) m_art->addItem(art, art);
        m_art->setCurrentIndex(std::max(0, m_art->findData(vorher)));
    }
    baueBaum();
}

void AkkordFenster::zeigeVorn()
{
    liesNeu();
    show();
    raise();
    activateWindow();
    m_suche->setFocus();
}

const Akkord* AkkordFenster::finde(const QString& nennung) const
{
    if (nennung.isEmpty()) return nullptr;
    const auto treffer = std::find_if(m_alle.begin(), m_alle.end(), [&](const Akkord& a) { return a.name == nennung; });
    return treffer == m_alle.end() ? nullptr : &*treffer;
}

bool AkkordFenster::passt(const Akkord& akkord) const
{
    const QString art = m_art->currentData().toString();
    if (!art.isEmpty() && akkord.art != art) return false;
    const QString heu = QStringList{akkord.titel, akkord.art, akkord.beschreibung, akkord.name, lageText(akkord)}.join(QLatin1Char(' '));
    const QStringList worte = m_suche->text().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    return std::all_of(worte.begin(), worte.end(), [&heu](const QString& wort) { return heu.contains(wort, Qt::CaseInsensitive); });
}

std::vector<AkkordGalerie::Gruppe> AkkordFenster::gruppenVon(const QString& ordner) const
{
    std::vector<AkkordGalerie::Gruppe> gruppen;
    const auto flach = [&](const QString& titel, const QStringList& nennungen) {
        AkkordGalerie::Gruppe g{titel, {}};
        for (const QString& n : nennungen)
            if (const Akkord* a = finde(n); a != nullptr && passt(*a)) g.akkorde.push_back(*a);
        if (!g.akkorde.empty()) gruppen.push_back(std::move(g));
    };
    // mit Suche ueber alle, gleich welcher Ordner gewaehlt ist
    const bool suche = !m_suche->text().trimmed().isEmpty();
    if (!suche && (ordner == kOrdnerFavoriten || ordner == kOrdnerZuletzt))
    {
        flach(ordner == kOrdnerFavoriten ? tr("Favorites") : tr("Recently used"),
              ordner == kOrdnerFavoriten ? m_favoriten : m_zuletzt);
        return gruppen;
    }

    // sonst nach Grundton und Art, in der Reihenfolge der Arten
    const QStringList teile = ordner.split(QLatin1Char(':'));
    const int grundton = teile.size() >= 2 ? teile[1].toInt() : -1;
    const QString nurArt = teile.size() >= 3 ? teile.mid(2).join(QLatin1Char(':')) : QString();
    std::vector<const Akkord*> wahl;
    for (const Akkord& a : m_alle)
    {
        const bool eigen = eigenerAkkord(a.name);
        if (!suche)
        {
            if (ordner == kOrdnerEigene ? !eigen : (eigen || a.grundton != grundton)) continue;
            if (!nurArt.isEmpty() && a.art != nurArt) continue;
        }
        if (passt(a)) wahl.push_back(&a);
    }
    std::stable_sort(wahl.begin(), wahl.end(), [](const Akkord* a, const Akkord* b) {
        if (a->grundton != b->grundton) return a->grundton < b->grundton;
        if (a->art != b->art) return artVor(a->art, b->art);
        return false;
    });
    for (const Akkord* a : wahl)
    {
        const QString kopf = kopfVon(*a);
        if (gruppen.empty() || gruppen.back().akkorde.front().grundton != a->grundton || gruppen.back().akkorde.front().art != a->art)
            gruppen.push_back({kopf, {}});
        gruppen.back().akkorde.push_back(*a);
    }
    for (AkkordGalerie::Gruppe& g : gruppen) g.titel = QStringLiteral("%1 · %2").arg(g.titel).arg(g.akkorde.size());
    return gruppen;
}

void AkkordFenster::baueBaum()
{
    const QString vorher = ordner();
    const QSignalBlocker ruhe(m_baum);
    m_baum->clear();
    const auto eintrag = [](QTreeWidgetItem* eintrag, const QString& text, const QString& ordner) {
        eintrag->setText(0, text);
        eintrag->setData(0, kRolleOrdner, ordner);
        return eintrag;
    };
    const auto zahlIn = [this](const QStringList& nennungen) {
        return static_cast<int>(std::count_if(nennungen.begin(), nennungen.end(), [this](const QString& n) { return finde(n) != nullptr; }));
    };
    eintrag(new QTreeWidgetItem(m_baum), QString::fromUtf8("★ ") + tr("Favorites (%1)").arg(zahlIn(m_favoriten)),
            kOrdnerFavoriten);
    eintrag(new QTreeWidgetItem(m_baum), tr("Recently used (%1)").arg(zahlIn(m_zuletzt)), kOrdnerZuletzt);
    for (int grundton = 0; grundton < 12; ++grundton)
    {
        std::vector<QString> arten;
        int zahl = 0;
        for (const Akkord& a : m_alle)
            if (a.grundton == grundton && !eigenerAkkord(a.name))
            {
                ++zahl;
                if (std::find(arten.begin(), arten.end(), a.art) == arten.end()) arten.push_back(a.art);
            }
        if (zahl == 0) continue;
        std::sort(arten.begin(), arten.end(), artVor);
        QTreeWidgetItem* wurzel =
            eintrag(new QTreeWidgetItem(m_baum), QStringLiteral("%1 (%2)").arg(grundtonNamen().at(grundton)).arg(zahl),
                    wurzelOrdner(grundton));
        for (const QString& art : arten)
        {
            const int hier = static_cast<int>(std::count_if(m_alle.begin(), m_alle.end(), [&](const Akkord& a) {
                return a.grundton == grundton && a.art == art && !eigenerAkkord(a.name);
            }));
            eintrag(new QTreeWidgetItem(wurzel), QStringLiteral("%1 (%2)").arg(art).arg(hier), artOrdner(grundton, art));
        }
    }
    const int eigene = static_cast<int>(std::count_if(m_alle.begin(), m_alle.end(), [](const Akkord& a) { return eigenerAkkord(a.name); }));
    eintrag(new QTreeWidgetItem(m_baum), tr("Your chords (%1)").arg(eigene), kOrdnerEigene);
    // der vorige Ordner, sonst C
    if (!waehleOrdner(vorher) && !waehleOrdner(wurzelOrdner(0)) && m_baum->topLevelItemCount() > 0)
        m_baum->setCurrentItem(m_baum->topLevelItem(0));
    zeigeGalerie();
}

bool AkkordFenster::waehleOrdner(const QString& ordner)
{
    if (ordner.isEmpty()) return false;
    for (int i = 0; i < m_baum->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* oben = m_baum->topLevelItem(i);
        if (oben->data(0, kRolleOrdner).toString() == ordner)
        {
            m_baum->setCurrentItem(oben);
            return true;
        }
        for (int k = 0; k < oben->childCount(); ++k)
            if (oben->child(k)->data(0, kRolleOrdner).toString() == ordner)
            {
                oben->setExpanded(true);
                m_baum->setCurrentItem(oben->child(k));
                return true;
            }
    }
    return false;
}

QString AkkordFenster::ordner() const
{
    const QTreeWidgetItem* aktuell = m_baum->currentItem();
    return aktuell == nullptr ? QString() : aktuell->data(0, kRolleOrdner).toString();
}

void AkkordFenster::zeigeGalerie()
{
    m_galerie->setzeGruppen(gruppenVon(ordner()));
    m_galerie->setzeGewaehlt(gewaehlt());
}

void AkkordFenster::setzeSuche(const QString& text)
{
    m_suche->setText(text);  // zeigt ueber textChanged
}

void AkkordFenster::setzeArt(const QString& art)
{
    m_art->setCurrentIndex(std::max(0, m_art->findData(art)));
    zeigeGalerie();
}

bool AkkordFenster::waehle(const QString& nennung)
{
    if (finde(nennung) == nullptr) return false;
    m_galerie->setzeGewaehlt(nennung);
    m_verschiebung = 0;
    const QRect zelle = m_galerie->zelle(nennung);
    if (!zelle.isEmpty()) m_rolle->ensureVisible(zelle.center().x(), zelle.center().y(), 0, zelle.height());
    zeigeDetail();
    return true;
}

QString AkkordFenster::gewaehlt() const
{
    return finde(m_galerie->gewaehlt()) != nullptr ? m_galerie->gewaehlt() : QString();
}

std::optional<Akkord> AkkordFenster::lage() const
{
    const Akkord* a = finde(gewaehlt());
    if (a == nullptr) return std::nullopt;
    return verschobenesGriffbild(*a, m_verschiebung);
}

bool AkkordFenster::verschiebe(int buende)
{
    const Akkord* a = finde(gewaehlt());
    if (a == nullptr || buende == 0) return false;
    const int neu = m_verschiebung + buende;
    if (!toeneDesGriffbilds(*a, m_saiten, neu)) return false;
    m_verschiebung = neu;
    zeigeDetail();
    return true;
}

void AkkordFenster::setzeSpielart(AkkordSpielart art, bool anspielen)
{
    m_spielart = art;
    if (QAbstractButton* knopf = m_spielarten->button(static_cast<int>(art)); knopf != nullptr && !knopf->isChecked())
        knopf->setChecked(true);
    if (spielartMerken()) QSettings().setValue(QLatin1String(kSpielart), static_cast<int>(art));
    if (anspielen) (void)spieleAn();
}

bool AkkordFenster::spieleAn()
{
    const std::optional<Akkord> l = lage();
    if (!l) return false;
    const Muster noten = akkordZumAnhoeren(*l, m_saiten, m_strum, m_spielart, m_jeSchlag);
    if (noten.noten.empty()) return false;
    Q_EMIT vorhoerenVerlangt(noten);
    return true;
}

bool AkkordFenster::verwende()
{
    const std::optional<Akkord> l = lage();
    if (!l || m_muster.isEmpty()) return false;
    if (!toeneDesGriffbilds(*l, m_saiten)) return false;
    const QString nennung = gewaehlt();
    Q_EMIT verwenden(*l);
    merkeVerwendet(nennung);
    return true;
}

void AkkordFenster::merkeVerwendet(const QString& nennung)
{
    if (finde(nennung) == nullptr) return;
    m_zuletzt.removeAll(nennung);
    m_zuletzt.prepend(nennung);
    while (m_zuletzt.size() > kZuletztMax) m_zuletzt.removeLast();
    merke();
    baueBaum();
}

bool AkkordFenster::istFavorit(const QString& nennung) const
{
    return m_favoriten.contains(nennung);
}

void AkkordFenster::setzeFavorit(bool an)
{
    const QString nennung = gewaehlt();
    if (nennung.isEmpty() || an == istFavorit(nennung)) return;
    if (an)
        m_favoriten.append(nennung);
    else
        m_favoriten.removeAll(nennung);
    merke();
    baueBaum();
}

void AkkordFenster::merke() const
{
    QSettings einstellungen;
    einstellungen.setValue(QLatin1String(kFavoriten), m_favoriten);
    einstellungen.setValue(QLatin1String(kZuletzt), m_zuletzt);
}

void AkkordFenster::zieheAb(const QString& nennung)
{
    if (finde(nennung) == nullptr) return;
    if (nennung != gewaehlt()) (void)waehle(nennung);
    auto* daten = new QMimeData;
    daten->setData(QLatin1String(kMimeTyp), QStringLiteral("%1\n%2").arg(nennung).arg(m_verschiebung).toUtf8());
    auto* zug = new QDrag(m_galerie);
    zug->setMimeData(daten);
    if (const std::optional<Akkord> l = lage())
    {
        QPixmap bild(AkkordGalerie::kBildBreite, AkkordGalerie::kBildHoehe);
        bild.fill(palette().color(QPalette::Base));
        QPainter p(&bild);
        zeichneGriffbild(p, QRectF(bild.rect()), *l, GriffbildGroesse::Klein, palette());
        p.end();
        zug->setPixmap(bild);
        zug->setHotSpot(QPoint(bild.width() / 2, bild.height() / 2));
    }
    if (zug->exec(Qt::CopyAction) == Qt::CopyAction) merkeVerwendet(nennung);
}

void AkkordFenster::zeigeDetail()
{
    if (m_use == nullptr) return;  // waehrend des Aufbaus
    const Akkord* a = finde(gewaehlt());
    const std::optional<Akkord> l = lage();
    m_feld->setzeAkkord(l ? *l : Akkord{});
    m_ungewoehnlich->setText(ungewoehnlichText());
    m_ungewoehnlich->setVisible(!m_ungewoehnlich->text().isEmpty());
    m_gerechnet->setVisible(a != nullptr && a->herkunft == QLatin1String(kHerkunftGerechnet));
    if (a == nullptr || !l)
    {
        m_titel->setText(tr("<i>Select a chord</i>"));
        m_toene->clear();
    }
    else
    {
        const QString lage = m_verschiebung == 0 ? a->beschreibung
                                                 : tr("%1, moved %2 fret(s) — %3")
                                                       .arg(a->beschreibung)
                                                       .arg(m_verschiebung > 0 ? QStringLiteral("+%1").arg(m_verschiebung)
                                                                               : QString::number(m_verschiebung))
                                                       .arg(lageText(*l));
        m_titel->setText(QStringLiteral("<b>%1</b> · %2").arg(a->titel.toHtmlEscaped(), lage.toHtmlEscaped()));
        m_toene->setText(detailText());
    }
    const bool da = l.has_value();
    m_tiefer->setEnabled(da && a != nullptr && toeneDesGriffbilds(*a, m_saiten, m_verschiebung - 1).has_value());
    m_hoeher->setEnabled(da && a != nullptr && toeneDesGriffbilds(*a, m_saiten, m_verschiebung + 1).has_value());
    for (QAbstractButton* knopf : m_spielarten->buttons()) knopf->setEnabled(da);
    m_favorit->setEnabled(da);
    {
        const QSignalBlocker ruhe(m_favorit);
        m_favorit->setChecked(da && istFavorit(gewaehlt()));
    }
    m_use->setEnabled(da && !m_muster.isEmpty());
    m_use->setToolTip(m_muster.isEmpty() ? tr("Open an instrument pattern first")
                                         : tr("Insert at the insert marker of %1 (Shift+click in the bar row moves it)").arg(m_muster));
}

QString AkkordFenster::detailText() const
{
    const std::optional<Akkord> l = lage();
    if (!l) return {};
    QStringList namen;
    if (const auto toene = toeneDesGriffbilds(*l, m_saiten))
        for (const AkkordTon& t : *toene) namen << tonName(t.hoehe);
    return tr("Notes: %1").arg(namen.join(QLatin1Char(' ')));
}

QString AkkordFenster::ungewoehnlichText() const
{
    const Akkord* a = finde(gewaehlt());
    if (a == nullptr) return {};
    const std::optional<std::vector<int>> fremd = fremdeTonklassen(*a);
    if (!fremd || fremd->empty()) return {};
    return tr("unusual: %1 not in %2").arg(tonListe(*fremd), a->titel);
}

QStringList AkkordFenster::galerieZeilen() const
{
    QStringList zeilen;
    for (const AkkordGalerie::Gruppe& g : m_galerie->gruppen())
    {
        zeilen << QStringLiteral("[%1]").arg(g.titel);
        for (const Akkord& a : g.akkorde) zeilen << a.name;
    }
    return zeilen;
}

QStringList AkkordFenster::baumZeilen() const
{
    QStringList zeilen;
    for (int i = 0; i < m_baum->topLevelItemCount(); ++i)
    {
        const QTreeWidgetItem* oben = m_baum->topLevelItem(i);
        zeilen << oben->text(0);
        for (int k = 0; k < oben->childCount(); ++k) zeilen << QStringLiteral("  ") + oben->child(k)->text(0);
    }
    return zeilen;
}

void AkkordFenster::nimmZurueck()
{
    Q_EMIT rueckgaengigVerlangt(false);
}

void AkkordFenster::stelleWiederHer()
{
    Q_EMIT rueckgaengigVerlangt(true);
}

void AkkordFenster::hideEvent(QHideEvent* event)
{
    QSettings().setValue(QLatin1String(kGeometrie), saveGeometry());
    QWidget::hideEvent(event);
}

}  // namespace lumi::loop
