/**
 * @file InstrumentEditor.cpp
 * @brief Loop: der Instrument-Editor — die Noten einer Basis ueber einer
 *        Klaviatur (Stufe 9b)
 */

#include "loop/InstrumentEditor.hpp"

#include "UI/widgets/Bedienhilfe.hpp"  // zeigeMenue, frageText: in Tests abfragbar
#include "UI/widgets/PresetTypeIcons.hpp"
#include "loop/AkkordFenster.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/BendKurve.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/StrumZeile.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStandardPaths>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace lumi::loop
{

namespace
{

const char* const kInstrumentOrdner = "loop/lastInstrumentDir";
const char* const kRechtsLoescht = "loop/notesRightClickDeletes";
const char* const kLage = "loop/instrumentEditorGeometry";  ///< ein Pfad der Fensterlage: geht nie in einen Export
/// F (Stufe 26c): ob die Tab-Linien zu sehen sind — gemerkt wie die Lage (in der Test-Exe nicht)
const char* const kTabZeigen = "loop/instrumentEditorTab";

bool& lageMerken()
{
    static bool an = true;
    return an;
}
constexpr int kHoehen = kHoeheMax - kHoeheMin + 1;
constexpr double kLautMin = 0.05;  ///< leiser wird eine Note nicht — ohne Lautstaerke gaebe es sie nicht

/// Die Saiten eines Instruments; leer, wenn es keine traegt oder sich nicht finden laesst.
Saitensatz saitenVon(const QString& nennung)
{
    Instrument instrument;
    return holeInstrument(nennung, instrument) ? instrument.saiten : Saitensatz{};
}

bool schwarzeTaste(int hoehe)
{
    const int stufe = ((hoehe % 12) + 12) % 12;
    return stufe == 1 || stufe == 3 || stufe == 6 || stufe == 8 || stufe == 10;
}

int teileAb(int wert, int durch)
{
    return wert >= 0 ? wert / durch : -((-wert + durch - 1) / durch);
}

}  // namespace

// =============================================================================
// NotenRaster
// =============================================================================

NotenRaster::NotenRaster(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("loopNotes"));
    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);  // D (Stufe 26e-2): Griffbilder aus dem Fenster Chords
    setMouseTracking(true);  // der Mauszeiger zeigt, was ein Klick taete
}

bool NotenRaster::anKante(const Note& note, int x) const
{
    const int griff = std::clamp(note.laenge * schrittBreite() / 3, kKante, kKanteMax);
    return x >= xVonSchritt(note.schritt + note.laenge) - griff;
}

void NotenRaster::zeigeZeiger(QPoint ort)
{
    Qt::CursorShape form = Qt::ArrowCursor;
    if (const int platz = noteBei(ort); platz >= 0)
        form = anKante(m_muster.noten[static_cast<std::size_t>(platz)], ort.x()) ? Qt::SizeHorCursor : Qt::SizeAllCursor;
    else if (ort.x() < kTastenBreite && hoeheBei(ort.y()) >= 0)
        form = Qt::PointingHandCursor;
    else if (strumBei(ort) >= 0)
        form = Qt::PointingHandCursor;  // J: ein Anschlag in der Spur Strum laesst sich schalten
    else if (tabSaiteBei(ort.y()) >= 0)
        // Stufe 26c: eine Zahl laesst sich auf eine andere Saite ziehen, eine leere Stelle nimmt einen Bund,
        // der Name der Saite spielt sie an
        form = ort.x() < kTastenBreite ? Qt::PointingHandCursor : tabNoteBei(ort) >= 0 ? Qt::SizeVerCursor : Qt::IBeamCursor;
    if (cursor().shape() != form) setCursor(form);
}

void NotenRaster::setMuster(const Muster& muster, int schlaegeJeTakt, int schritteJeSchlag)
{
    m_muster = muster;
    m_schlaege = std::max(1, schlaegeJeTakt);
    m_jeSchlag = std::max(1, schritteJeSchlag);
    m_zug = Zug::Keiner;
    m_rahmen = {};
    if (auswahl() < 0) m_wahlSchritt = m_wahlHoehe = -1;
    richteMarken();  // was es nicht mehr gibt, faellt aus den Marken
    m_einfuegen = std::clamp(m_einfuegen, 0, schritte());
    m_abspiel = std::clamp(m_abspiel, 0, std::max(0, schritte() - 1));
    if (m_tabSchritt >= schritte()) m_tabSchritt = m_tabSaite = -1;  // Stufe 26c: das Muster ist kuerzer geworden
    // kommt nicht die Fassung zurueck, die das Verschieben erzeugt hat, beginnt es neu
    if (m_schiebeGilt && m_muster.noten != m_schiebeErgebnis.noten) m_schiebeGilt = false;
    setRollen(m_rollX, m_rollY);  // das Muster kann kuerzer geworden sein
    update();
}

// --- Saiten (Stufe 23f; Skizze gebilligt, Patrik S133: S-U2 a, S-U2b a) -----------

void NotenRaster::setSaiten(const Saitensatz& saiten)
{
    if (saiten == m_saiten) return;
    m_saiten = saiten;
    if (m_tabSaite >= m_saiten.anzahl()) m_tabSchritt = m_tabSaite = -1;  // Stufe 26c
    setRollen(m_rollX, m_rollY);  // mit den Tab-Linien aendert sich die Hoehe der Noten
    update();
}

void NotenRaster::plane(std::vector<Griff>* griffe, std::vector<Anschlagplan>* anschlaege) const
{
    const std::size_t zahl = m_muster.noten.size();
    if (griffe != nullptr) griffe->assign(zahl, Griff{});
    if (anschlaege != nullptr) anschlaege->assign(zahl, Anschlagplan{});
    if (!m_saiten.istSaiteninstrument()) return;
    // die Planung will das Muster gerichtet — waehrend eines Zugs ist es das nicht immer:
    // auf einer sortierten Kopie planen und das Ergebnis zurueck an die Plaetze legen
    std::vector<std::size_t> folge(zahl);
    std::iota(folge.begin(), folge.end(), std::size_t{0});
    std::stable_sort(folge.begin(), folge.end(), [this](std::size_t a, std::size_t b) {
        const Note& x = m_muster.noten[a];
        const Note& y = m_muster.noten[b];
        return x.schritt != y.schritt ? x.schritt < y.schritt : x.hoehe < y.hoehe;
    });
    Muster sortiert = m_muster;
    sortiert.noten.clear();
    for (const std::size_t platz : folge) sortiert.noten.push_back(m_muster.noten[platz]);
    const std::vector<Griff> gegriffen = greifeNoten(sortiert, m_saiten);
    if (griffe != nullptr)
        for (std::size_t k = 0; k < zahl; ++k) (*griffe)[folge[k]] = gegriffen[k];
    if (anschlaege == nullptr) return;
    const std::vector<Anschlagplan> plan = schlageAn(sortiert, m_saiten, gegriffen, m_schlaege, m_jeSchlag);
    for (std::size_t k = 0; k < zahl; ++k) (*anschlaege)[folge[k]] = plan[k];
}

std::vector<Griff> NotenRaster::griffe() const
{
    std::vector<Griff> ergebnis;
    plane(&ergebnis, nullptr);
    return ergebnis;
}

// --- Spielweisen (Stufe 24e; Skizze gebilligt, Patrik S134, S-W5 a) --------------

void NotenRaster::setTempo(double bpm)
{
    if (bpm <= 0.0 || bpm == m_bpm) return;
    m_bpm = bpm;
    update();
}

std::vector<Anschlagplan> NotenRaster::anschlaege() const
{
    std::vector<Anschlagplan> ergebnis;
    plane(nullptr, &ergebnis);
    return ergebnis;
}

int NotenRaster::versatzPixel(double versatzMs) const
{
    // ein Schritt dauert 60000 / bpm / Schritte je Schlag ms und ist schrittBreite() breit
    const double schrittMs = 60000.0 / m_bpm / std::max(1, m_jeSchlag);
    return static_cast<int>(std::lround(versatzMs / schrittMs * schrittBreite()));
}

bool NotenRaster::schalteAnschlag(int schritt)
{
    if (!m_saiten.istSaiteninstrument()) return false;
    const auto zahl = std::count_if(m_muster.noten.begin(), m_muster.noten.end(),
                                    [schritt](const Note& n) { return n.schritt == schritt; });
    if (zahl < 1) return false;
    // Follow hand → Down → Up → Together → Follow hand; eine einzelne Note ohne Together
    // (auf einer Saite dasselbe wie Down, S-W15 a); ein eigener Spread bleibt
    const Anschlag* eintrag = anschlagAn(m_muster, schritt);
    const Anschlagart jetzt = eintrag != nullptr ? eintrag->art : Anschlagart::Hand;
    const Anschlagart weiter = jetzt == Anschlagart::Hand ? Anschlagart::Ab
                               : jetzt == Anschlagart::Ab ? Anschlagart::Auf
                               : jetzt == Anschlagart::Auf && zahl > 1 ? Anschlagart::Zugleich
                                                                       : Anschlagart::Hand;
    Muster neu = m_muster;
    setzeAnschlag(neu, Anschlag{schritt, weiter, eintrag != nullptr ? eintrag->spreadMs : -1.0});
    const QString name = weiter == Anschlagart::Hand ? tr("Follow hand")
                         : weiter == Anschlagart::Ab ? tr("Down")
                         : weiter == Anschlagart::Auf ? tr("Up")
                                                      : tr("Together");
    uebernimm(neu, m_marken, tr("Strum: %1").arg(name));
    return true;
}

std::vector<int> NotenRaster::markierteSchritte() const
{
    std::vector<int> schritte;
    for (const Note& note : m_muster.noten)
        if (istMarkiert(note)) schritte.push_back(note.schritt);
    std::sort(schritte.begin(), schritte.end());
    schritte.erase(std::unique(schritte.begin(), schritte.end()), schritte.end());
    return schritte;
}

bool NotenRaster::setzeAnschlagart(Anschlagart art)
{
    if (!m_saiten.istSaiteninstrument() || m_marken.empty()) return false;
    Muster neu = m_muster;
    for (const int schritt : markierteSchritte())
    {
        const Anschlag* e = anschlagAn(m_muster, schritt);
        setzeAnschlag(neu, Anschlag{schritt, art, e != nullptr ? e->spreadMs : -1.0});
    }
    if (neu == m_muster) return false;
    const QString name = art == Anschlagart::Hand  ? tr("Follow hand")
                         : art == Anschlagart::Ab  ? tr("Down")
                         : art == Anschlagart::Auf ? tr("Up")
                                                   : tr("Together");
    uebernimm(neu, m_marken, tr("Strum: %1").arg(name));
    return true;
}

bool NotenRaster::setzeAnschlagSpread(double ms)
{
    if (!m_saiten.istSaiteninstrument() || m_marken.empty()) return false;
    const double wert = ms < 0.0 ? -1.0 : std::clamp(ms, 0.0, kSpreadMaxMs);
    Muster neu = m_muster;
    for (const int schritt : markierteSchritte())
    {
        const Anschlag* e = anschlagAn(m_muster, schritt);
        setzeAnschlag(neu, Anschlag{schritt, e != nullptr ? e->art : Anschlagart::Hand, wert});
    }
    if (neu == m_muster) return false;
    uebernimm(neu, m_marken, wert < 0.0 ? tr("Spread: like the pattern") : tr("Spread: %1 ms").arg(wert));
    return true;
}

bool NotenRaster::schalteArtikulation(int bit)
{
    if (!m_saiten.istSaiteninstrument() || m_marken.empty() || (bit & kArtikulationenAlle) == 0) return false;
    // tragen sie alle markierten, geht er weg — sonst bekommen ihn alle
    bool alle = true;
    for (const Note& note : m_muster.noten)
        if (istMarkiert(note)) alle = alle && (note.artikulationen & bit) != 0;
    Muster neu = m_muster;
    for (Note& note : neu.noten)
    {
        if (!istMarkiert(note)) continue;
        note.artikulationen = alle ? note.artikulationen & ~bit : note.artikulationen | bit;
        // Stufe 26d (S-L3 a E): Legato und Slide schliessen sich aus — eine Note kommt auf eine Art her
        if (!alle && (bit & kVerbunden) != 0) note.artikulationen &= ~(kVerbunden & ~bit);
    }
    if (neu == m_muster) return false;
    uebernimm(neu, m_marken, (alle ? tr("%1 off") : tr("%1 on")).arg(artikulationsName(bit)));
    return true;
}

void NotenRaster::frageSpread(double vorgabe)
{
    QDialog frage(this);
    frage.setObjectName(QStringLiteral("strumSpreadQuestion"));
    frage.setWindowTitle(tr("Spread"));
    auto* form = new QVBoxLayout(&frage);
    form->addWidget(new QLabel(tr("From one string to the next, for the chords of the selected notes:"), &frage));
    auto* wert = new QDoubleSpinBox(&frage);
    wert->setObjectName(QStringLiteral("strumSpreadValue"));
    wert->setRange(-1.0, kSpreadMaxMs);
    wert->setDecimals(1);
    wert->setSuffix(tr(" ms"));
    // ganz unten: wie das Muster (die Zeile Strum)
    wert->setSpecialValueText(tr("Like the pattern (%1 ms)").arg(m_muster.strum.spreadMs));
    wert->setValue(vorgabe);
    form->addWidget(wert);
    auto* knoepfe = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &frage);
    connect(knoepfe, &QDialogButtonBox::accepted, &frage, &QDialog::accept);
    connect(knoepfe, &QDialogButtonBox::rejected, &frage, &QDialog::reject);
    form->addWidget(knoepfe);
    if (lumi::ui::fuehreDialogAus(frage) != QDialog::Accepted) return;
    (void)setzeAnschlagSpread(wert->value());
}

QString NotenRaster::artikulationsName(int bit)
{
    switch (bit)
    {
    case kPalmMute: return tr("Palm mute");
    case kLetRing: return tr("Let ring");
    case kTapping: return tr("Tapping");
    case kHarmonic: return tr("Harmonic");
    case kLegato: return tr("Legato (hammer-on / pull-off)");  // Stufe 26d, S-L3 a E
    case kSlide: return tr("Slide");
    default: return {};
    }
}

QString NotenRaster::artikulationsKuerzel(int artikulationen)
{
    QStringList kuerzel;
    if ((artikulationen & kPalmMute) != 0) kuerzel << QStringLiteral("PM");
    if ((artikulationen & kLetRing) != 0) kuerzel << QStringLiteral("LR");
    if ((artikulationen & kTapping) != 0) kuerzel << QStringLiteral("T");
    if ((artikulationen & kHarmonic) != 0) kuerzel << QString::fromUtf8("◇");
    return kuerzel.join(QLatin1Char(' '));
}

bool NotenRaster::setzeSaite(int stelle)
{
    if (!m_saiten.istSaiteninstrument() || m_marken.empty() || stelle < -1 || stelle >= m_saiten.anzahl()) return false;
    Muster neu = m_muster;
    for (Note& note : neu.noten)
        if (istMarkiert(note)) note.saite = stelle;
    if (neu.noten == m_muster.noten) return false;
    uebernimm(neu, m_marken, stelle < 0 ? tr("String: Auto") : tr("String %1").arg(saitenNummer(m_saiten, stelle)));
    return true;
}

int NotenRaster::strumBei(QPoint ort) const
{
    if (strumHoehe() == 0 || ort.y() < strumOben() || ort.y() >= tabOben()) return -1;
    const int schritt = schrittBei(ort.x());
    if (schritt < 0) return -1;
    const bool belegt = std::any_of(m_muster.noten.begin(), m_muster.noten.end(),
                                    [schritt](const Note& n) { return n.schritt == schritt; });
    return belegt ? schritt : -1;
}

// --- Tab-Linien (Stufe 26c; Skizze gebilligt, Patrik S137, S-T5 a A, B, F) -------

namespace
{

/// Was an einer Note in den Tab-Linien steht: der Bund, ohne Buende der Tonname,
/// nicht greifbar „!" (B).
QString tabZeichen(const Note& note, const Griff& griff, const Saitensatz& saiten)
{
    if (!griff.greifbar) return QStringLiteral("!");
    return saiten.bundlos ? tonName(note.hoehe) : QString::number(griff.bund);
}

/// Der Ton einer leeren Saite ohne Oktave: „E", „F#".
QString saitenTon(int hoehe)
{
    QString name = tonName(hoehe);
    while (!name.isEmpty() && (name.back().isDigit() || name.back() == QLatin1Char('-'))) name.chop(1);
    return name;
}

}  // namespace

void NotenRaster::setTabAn(bool an)
{
    if (an == m_tabAn) return;
    m_tabAn = an;
    if (!an) m_tabSchritt = m_tabSaite = -1;
    setRollen(m_rollX, m_rollY);  // die Noten sind hoeher oder niedriger geworden
    update();
}

int NotenRaster::tabSaiteBei(int y) const
{
    if (tabHoehe() == 0 || y < tabOben() + kTabRand || y >= tabOben() + tabHoehe() - kTabRand) return -1;
    return m_saiten.anzahl() - 1 - (y - tabOben() - kTabRand) / kTabZeile;
}

int NotenRaster::tabNoteBei(QPoint ort) const
{
    const int stelle = tabSaiteBei(ort.y());
    if (stelle < 0 || ort.x() < kTastenBreite) return -1;
    std::vector<Griff> alleGriffe;
    std::vector<Anschlagplan> alleAnschlaege;
    plane(&alleGriffe, &alleAnschlaege);
    for (int i = static_cast<int>(m_muster.noten.size()) - 1; i >= 0; --i)
    {
        const auto platz = static_cast<std::size_t>(i);
        if (alleGriffe[platz].saite != stelle) continue;
        // die Zahl steht am Anschlag; getroffen wird sie auf einem Schritt Breite, mindestens 16 Pixeln
        const int links = xVonSchritt(m_muster.noten[platz].schritt) + versatzPixel(alleAnschlaege[platz].versatzMs);
        if (ort.x() >= links && ort.x() < links + std::max(16, schrittBreite())) return i;
    }
    return -1;
}

QString NotenRaster::tabText(int platz) const
{
    if (!m_saiten.istSaiteninstrument() || platz < 0 || platz >= static_cast<int>(m_muster.noten.size())) return {};
    return tabZeichen(m_muster.noten[static_cast<std::size_t>(platz)], griffe()[static_cast<std::size_t>(platz)], m_saiten);
}

void NotenRaster::setzeTabMarke(int schritt, int stelle)
{
    const bool gilt = tabHoehe() > 0 && schritt >= 0 && schritt < schritte() && stelle >= 0 && stelle < m_saiten.anzahl();
    m_tabSchritt = gilt ? schritt : -1;
    m_tabSaite = gilt ? stelle : -1;
    m_tabZiffer = -1;  // eine neue Stelle: die naechste Ziffer beginnt einen neuen Bund
    update();
}

bool NotenRaster::setzeBund(int bund)
{
    if (tabHoehe() == 0 || m_tabSchritt < 0 || m_tabSaite < 0 || m_tabSaite >= m_saiten.anzahl()) return false;
    if (bund < 0 || bund > kBuendeMax) return false;
    const int hoehe = m_saiten.leer[static_cast<std::size_t>(m_tabSaite)] + m_saiten.capo + bund;
    if (hoehe < kHoeheMin || hoehe > kHoeheMax) return false;
    // liegt am Schritt schon eine Note auf dieser Saite, bekommt sie den Bund
    const std::vector<Griff> alleGriffe = griffe();
    int platz = -1;
    for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
        if (m_muster.noten[i].schritt == m_tabSchritt && alleGriffe[i].saite == m_tabSaite) platz = static_cast<int>(i);
    Muster neu = m_muster;
    if (platz >= 0)
    {
        Note& note = neu.noten[static_cast<std::size_t>(platz)];
        if (note.hoehe == hoehe && note.saite == m_tabSaite) return false;
        if (note.hoehe != hoehe && belegt(m_tabSchritt, hoehe, platz)) return false;  // derselbe Ton klingt dort schon
        note.hoehe = hoehe;
        note.saite = m_tabSaite;  // die Tabs setzen die Saite immer (S-M2 a)
    }
    else
    {
        if (belegt(m_tabSchritt, hoehe, -1)) return false;
        Note note{m_tabSchritt, 1, hoehe, 1.0};  // so lang wie ein Klick in der Piano-Roll
        note.saite = m_tabSaite;
        neu.noten.push_back(note);
    }
    uebernimm(neu, {{m_tabSchritt, hoehe}}, m_saiten.bundlos ? tr("Note %1").arg(tonName(hoehe)) : tr("Fret %1").arg(bund));
    return true;
}

bool NotenRaster::legeAufSaite(int platz, int stelle)
{
    if (!m_saiten.istSaiteninstrument() || platz < 0 || platz >= static_cast<int>(m_muster.noten.size()) || stelle < 0 ||
        stelle >= m_saiten.anzahl())
        return false;
    Muster neu = m_muster;
    Note& note = neu.noten[static_cast<std::size_t>(platz)];
    if (note.saite == stelle) return false;
    note.saite = stelle;  // derselbe Ton; ist er dort nicht greifbar, zeigt die Note „!"
    uebernimm(neu, {{note.schritt, note.hoehe}}, tr("String %1").arg(saitenNummer(m_saiten, stelle)));
    return true;
}

// --- Griffbilder (Stufe 26e-2; S-T5 a D, S-T8 a D) -----------------------------------

bool NotenRaster::setzeAkkord(const Akkord& lage, int schritt, int bezug)
{
    if (schritt < 0 || schritt >= schritte()) return false;
    // ein Griffbild ist schon verschoben (Bezug 0), ein Voicing liegt auf seinem tiefsten Ton;
    // einen Schritt lang wie eine getippte Zahl
    const std::vector<Note> neue = akkordNoten(lage, m_saiten, schritt, 1, lage.istGriffbild() ? 0 : bezug);
    if (neue.empty()) return false;
    // was am Schritt auf einer seiner Saiten liegt oder einen seiner Toene spielt, weicht
    const std::vector<Griff> alleGriffe = griffe();
    Muster neu = m_muster;
    neu.noten.clear();
    for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
    {
        const Note& alt = m_muster.noten[i];
        const bool weicht = alt.schritt == schritt && std::any_of(neue.begin(), neue.end(), [&](const Note& n) {
                                return (n.saite >= 0 && n.saite == alleGriffe[i].saite) || n.hoehe == alt.hoehe;
                            });
        if (!weicht) neu.noten.push_back(alt);
    }
    std::vector<NotenOrt> marken;
    for (const Note& n : neue)
    {
        neu.noten.push_back(n);
        marken.push_back({n.schritt, n.hoehe});
    }
    uebernimm(neu, marken, tr("Chord %1").arg(lage.titel));
    return true;
}

void NotenRaster::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat(QLatin1String(AkkordFenster::kMimeTyp))) event->acceptProposedAction();
}

void NotenRaster::dragMoveEvent(QDragMoveEvent* event)
{
    if (schrittBei(event->position().toPoint().x()) >= 0)
        event->acceptProposedAction();
    else
        event->ignore();
}

void NotenRaster::dropEvent(QDropEvent* event)
{
    const QStringList teile =
        QString::fromUtf8(event->mimeData()->data(QLatin1String(AkkordFenster::kMimeTyp))).split(QLatin1Char('\n'));
    const int schritt = schrittBei(event->position().toPoint().x());
    Akkord akkord;
    if (teile.size() < 2 || schritt < 0 || !holeAkkord(teile[0], akkord)) return event->ignore();
    if (!akkord.istGriffbild())
    {
        // ein Voicing (S-T6 a): der tiefste Ton auf der Zeile unter der Maus, sonst wie im Fenster
        const int zeile = hoeheBei(event->position().toPoint().y());
        if (!setzeAkkord(akkord, schritt, zeile >= 0 ? zeile : teile[1].toInt())) return event->ignore();
        event->acceptProposedAction();
        return;
    }
    const std::optional<Akkord> lage = verschobenesGriffbild(akkord, teile[1].toInt());
    if (!lage || !setzeAkkord(*lage, schritt)) return event->ignore();
    event->acceptProposedAction();
}

// --- Legato, Slide, Bend (Stufe 26d; S-L3 a A–G, S-T5 a C, E) ----------------------

bool NotenRaster::aendereMarkierte(const std::function<void(Note&)>& aendere, const QString& was)
{
    if (!m_saiten.istSaiteninstrument() || m_marken.empty()) return false;
    Muster neu = m_muster;
    for (Note& note : neu.noten)
        if (istMarkiert(note)) aendere(note);
    if (neu.noten == m_muster.noten) return false;
    uebernimm(neu, m_marken, was);
    return true;
}

const std::vector<double>& NotenRaster::bendHoehen()
{
    // ½, full, 1½, 2 — bis zur Grenze aus S-L15 b (vier Halbtoene)
    static const std::vector<double> kBendHoehen = {1.0, 2.0, 3.0, 4.0};
    return kBendHoehen;
}

QString NotenRaster::bendVorlageName(BendVorlage art)
{
    switch (art)
    {
    case BendVorlage::Keine: return tr("None");
    case BendVorlage::Bend: return tr("Bend");
    case BendVorlage::BendZurueck: return tr("Bend and release");
    case BendVorlage::VorBend: return tr("Pre-bend");
    case BendVorlage::VorBendZurueck: return tr("Pre-bend and release");
    }
    return {};
}

bool NotenRaster::setzeBendVorlage(BendVorlage art)
{
    return aendereMarkierte(
        [art](Note& note) {
            const double hoehe = bendHoehe(note.bend);
            note.bend = bendVorlage(art, hoehe > 0.0 ? hoehe : 2.0);  // ohne Bend: ein Ganzton, „full"
        },
        art == BendVorlage::Keine ? tr("No bend") : tr("Bend: %1").arg(bendVorlageName(art)));
}

bool NotenRaster::setzeBendHoehe(double halbtoene)
{
    if (halbtoene <= 0.0) return false;
    return aendereMarkierte(
        [halbtoene](Note& note) {
            if (note.bend.empty())
            {
                note.bend = bendVorlage(BendVorlage::Bend, halbtoene);
                return;
            }
            if (const auto art = bendVorlageVon(note.bend); art && *art != BendVorlage::Keine)
            {
                note.bend = bendVorlage(*art, halbtoene);  // die Vorlage behaelt ihre Form
                return;
            }
            const double alt = bendHoehe(note.bend);  // eine eigene Kurve wird gestreckt
            for (BendPunkt& punkt : note.bend) punkt.halbtoene = punkt.halbtoene * halbtoene / alt;
            richteBend(note.bend);
        },
        tr("Bend: %1").arg(bendText(halbtoene)));
}

bool NotenRaster::setzeBend(std::vector<BendPunkt> punkte)
{
    richteBend(punkte);
    return aendereMarkierte([&punkte](Note& note) { note.bend = punkte; }, punkte.empty() ? tr("No bend") : tr("Edit bend"));
}

void NotenRaster::frageBend()
{
    if (!m_saiten.istSaiteninstrument() || m_marken.empty()) return;
    const int platz = auswahl();
    const std::vector<BendPunkt> jetzt = platz >= 0 ? m_muster.noten[static_cast<std::size_t>(platz)].bend : std::vector<BendPunkt>{};
    QDialog frage(this);
    frage.setObjectName(QStringLiteral("bendDialog"));
    frage.setWindowTitle(tr("Edit bend"));
    auto* form = new QVBoxLayout(&frage);
    auto* zeile = new QHBoxLayout;
    zeile->addWidget(new QLabel(tr("Template"), &frage));
    auto* vorlage = new QComboBox(&frage);
    vorlage->setObjectName(QStringLiteral("bendTemplate"));
    for (const BendVorlage art : {BendVorlage::Keine, BendVorlage::Bend, BendVorlage::BendZurueck, BendVorlage::VorBend,
                                  BendVorlage::VorBendZurueck})
        vorlage->addItem(bendVorlageName(art), static_cast<int>(art));
    zeile->addWidget(vorlage);
    zeile->addSpacing(12);
    zeile->addWidget(new QLabel(tr("Height"), &frage));
    auto* hoehe = new QComboBox(&frage);
    hoehe->setObjectName(QStringLiteral("bendHeight"));
    for (const double h : bendHoehen()) hoehe->addItem(bendText(h), h);
    zeile->addWidget(hoehe);
    zeile->addStretch(1);
    form->addLayout(zeile);
    auto* kurve = new BendKurve(&frage);
    kurve->setzePunkte(jetzt);
    form->addWidget(kurve, 1);
    auto* hinweis = new QLabel(tr("Drag a point · double-click an empty spot to add one, a point to remove it · "
                                  "left to right: the length of the note"),
                               &frage);
    hinweis->setEnabled(false);
    hinweis->setWordWrap(true);
    form->addWidget(hinweis);
    // was die Kurve gerade ist: eine Vorlage (sonst keine Wahl — eine eigene Form) und ihre Hoehe
    const auto zeigeWahl = [vorlage, hoehe, kurve] {
        std::vector<BendPunkt> punkte = kurve->punkte();
        richteBend(punkte);
        const std::optional<BendVorlage> art = bendVorlageVon(punkte);
        const QSignalBlocker ruheV(vorlage);
        const QSignalBlocker ruheH(hoehe);
        vorlage->setCurrentIndex(art ? vorlage->findData(static_cast<int>(*art)) : -1);
        const int stelle = hoehe->findData(bendHoehe(punkte));
        if (stelle >= 0) hoehe->setCurrentIndex(stelle);
    };
    if (hoehe->findData(bendHoehe(jetzt)) < 0) hoehe->setCurrentIndex(hoehe->findData(2.0));
    zeigeWahl();
    const auto wende = [vorlage, hoehe, kurve] {
        const auto art = vorlage->currentIndex() < 0 ? BendVorlage::Bend : static_cast<BendVorlage>(vorlage->currentData().toInt());
        kurve->setzePunkte(bendVorlage(art, hoehe->currentData().toDouble()));
    };
    connect(vorlage, &QComboBox::activated, &frage, wende);
    connect(hoehe, &QComboBox::activated, &frage, wende);
    connect(kurve, &BendKurve::geaendert, &frage, zeigeWahl);
    auto* knoepfe = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &frage);
    connect(knoepfe, &QDialogButtonBox::accepted, &frage, &QDialog::accept);
    connect(knoepfe, &QDialogButtonBox::rejected, &frage, &QDialog::reject);
    form->addWidget(knoepfe);
    frage.resize(480, 320);
    if (lumi::ui::fuehreDialogAus(frage) != QDialog::Accepted) return;
    (void)setzeBend(kurve->punkte());
}

void NotenRaster::zeichneTab(QPainter& p, const std::vector<Griff>& alleGriffe, const std::vector<Anschlagplan>& alleAnschlaege)
{
    const QPalette pal = palette();
    const QColor text = pal.color(QPalette::Text);
    const QColor an = pal.color(QPalette::Highlight);
    QColor leise = text;
    leise.setAlpha(130);
    const int oben = tabOben();
    const int hoch = tabHoehe();
    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));
    p.setFont(klein);
    p.setPen(leise);
    p.drawLine(0, oben + hoch - 1, width(), oben + hoch - 1);
    // links in der Spalte der Klaviatur: Nummer und Ton der leeren Saite (mit Capo der klingende)
    for (int stelle = 0; stelle < m_saiten.anzahl(); ++stelle)
    {
        const int y = tabLinieY(stelle);
        p.setPen(leise);
        p.drawText(QRect(4, y - kTabZeile / 2, kTastenBreite - 8, kTabZeile), Qt::AlignLeft | Qt::AlignVCenter,
                   QStringLiteral("%1  %2")
                       .arg(saitenNummer(m_saiten, stelle))
                       .arg(saitenTon(m_saiten.leer[static_cast<std::size_t>(stelle)] + m_saiten.capo)));
    }
    p.setClipRect(QRect(kTastenBreite, oben, sichtBreite(), hoch));
    const int rechts = std::min(width(), xVonSchritt(schritte()));
    QColor linie = text;
    linie.setAlpha(90);
    p.setPen(QPen(linie, 1.0));
    for (int stelle = 0; stelle < m_saiten.anzahl(); ++stelle) p.drawLine(kTastenBreite, tabLinieY(stelle), rechts, tabLinieY(stelle));
    // die Tab-Marke: dort landet ein getippter Bund
    if (m_tabSchritt >= 0 && m_tabSaite >= 0)
    {
        const QRect marke(xVonSchritt(m_tabSchritt), tabLinieY(m_tabSaite) - kTabZeile / 2, schrittBreite(), kTabZeile);
        QColor flaeche = an;
        flaeche.setAlpha(60);
        p.setPen(QPen(an, 1.2));
        p.setBrush(flaeche);
        p.drawRect(marke.adjusted(0, 0, -1, -1));
        p.setBrush(Qt::NoBrush);
    }
    const QFontMetrics mass(klein);
    // je Note das Feld ihrer Zahl — fuer die Zeichen zwischen zwei Zahlen (Stufe 26d)
    std::vector<QRect> felder(m_muster.noten.size());
    for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
    {
        const Note& note = m_muster.noten[i];
        const Griff& griff = alleGriffe[i];
        if (griff.saite < 0) continue;
        const int x = xVonSchritt(note.schritt) + versatzPixel(alleAnschlaege[i].versatzMs);
        felder[i] = QRect(x + 1, tabLinieY(griff.saite) - kTabZeile / 2 + 1, mass.horizontalAdvance(tabZeichen(note, griff, m_saiten)) + 4,
                          kTabZeile - 2);
    }
    for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
    {
        const Note& note = m_muster.noten[i];
        const Griff& griff = alleGriffe[i];
        if (griff.saite < 0) continue;
        const int y = tabLinieY(griff.saite);
        const int x = xVonSchritt(note.schritt) + versatzPixel(alleAnschlaege[i].versatzMs);
        // die Dauer: ein blasser Strich hinter der Zahl
        QColor dauer = an;
        dauer.setAlpha(m_muster.stumm ? 40 : 80);
        p.setPen(QPen(dauer, 3.0, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(x + 2, y, x + note.laenge * schrittBreite() - 2, y);
        const QString zeichen = tabZeichen(note, griff, m_saiten);
        const int breite = mass.horizontalAdvance(zeichen) + 4;
        const QRect feld(x + 1, y - kTabZeile / 2 + 1, breite, kTabZeile - 2);
        const bool markiert = istMarkiert(note);
        // die Zahl schneidet die Linie: ein Feld in der Farbe des Grunds, markiert hervorgehoben
        p.setPen(Qt::NoPen);
        p.setBrush(markiert ? an : pal.color(QPalette::Base));
        p.drawRect(feld);
        p.setBrush(Qt::NoBrush);
        p.setPen(!griff.greifbar ? QColor(226, 75, 74) : markiert ? pal.color(QPalette::HighlightedText) : text);
        p.drawText(feld, Qt::AlignCenter, zeichen);
        // Stufe 26d (S-T5 a C, S-L3 a D, E): der Bend als Pfeil hinauf mit seiner Hoehe; verbunden ohne vorige ein „!"
        if (!note.bend.empty())
        {
            const QPointF fuss(feld.right() + 2, y);
            const QPointF spitze(feld.right() + 8, y - kTabZeile / 2.0 + 1);
            QPainterPath pfeil(fuss);
            pfeil.quadTo(QPointF(spitze.x(), fuss.y()), spitze);
            p.setPen(QPen(text, 1.2));
            p.setBrush(Qt::NoBrush);
            p.drawPath(pfeil);
            p.drawLine(spitze, spitze + QPointF(-3, 3));
            p.drawLine(spitze, spitze + QPointF(3, 3));
            p.drawText(QRectF(spitze.x() + 2, y - kTabZeile, 36, kTabZeile), Qt::AlignLeft | Qt::AlignVCenter,
                       bendText(bendHoehe(note.bend)));
        }
        if (griff.verbindungFehlt)
        {
            p.setPen(QColor(226, 75, 74));
            p.drawText(QRect(feld.left() - 8, feld.top(), 7, feld.height()), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("!"));
        }
    }
    // A–C: zwischen zwei Zahlen auf derselben Linie — ein Bogen mit h oder p, ein „/" oder „\" beim Slide
    for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
    {
        const Note& note = m_muster.noten[i];
        const int vorige = alleGriffe[i].vorige;
        if (vorige < 0 || (note.artikulationen & kVerbunden) == 0 || felder[i].isNull()) continue;
        const QRect& a = felder[static_cast<std::size_t>(vorige)];
        const QRect& b = felder[i];
        if (a.isNull()) continue;
        const Note& vor = m_muster.noten[static_cast<std::size_t>(vorige)];
        p.setPen(QPen(text, 1.2));
        p.setBrush(Qt::NoBrush);
        if ((note.artikulationen & kSlide) != 0)
        {
            const bool hinauf = note.hoehe >= vor.hoehe;
            const int mitte = b.center().y();
            p.drawLine(QPoint(a.right() + 3, hinauf ? mitte + 3 : mitte - 3), QPoint(b.left() - 2, hinauf ? mitte - 3 : mitte + 3));
            continue;
        }
        const QPointF von(a.center().x(), a.top());
        const QPointF bis(b.center().x(), b.top());
        const double bogenOben = a.top() - 6.0;
        QPainterPath pfad(von);
        pfad.cubicTo(QPointF(von.x(), bogenOben), QPointF(bis.x(), bogenOben), bis);
        p.drawPath(pfad);
        p.drawText(QRectF((von.x() + bis.x()) / 2.0 - 6, bogenOben - 10, 12, 10), Qt::AlignCenter,
                   note.hoehe >= vor.hoehe ? QStringLiteral("h") : QStringLiteral("p"));
    }
    // beim Ziehen einer Zahl: die Saite, auf die sie kaeme
    if (m_zug == Zug::TabSaite && m_tabZiel >= 0)
    {
        QColor ziel = an;
        ziel.setAlpha(110);
        p.setPen(QPen(ziel, 2.0, Qt::DashLine));
        p.drawLine(kTastenBreite, tabLinieY(m_tabZiel), rechts, tabLinieY(m_tabZiel));
    }
    p.setClipping(false);
    p.setFont(font());
}

bool NotenRaster::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip && m_saiten.istSaiteninstrument())
    {
        const auto* hilfe = static_cast<QHelpEvent*>(event);
        // J: in der Spur Strum sagen, was gilt und was ein Klick tut
        if (const int schritt = strumBei(hilfe->pos()); schritt >= 0)
        {
            const Anschlag* eintrag = anschlagAn(m_muster, schritt);
            const Anschlagart art = eintrag != nullptr ? eintrag->art : Anschlagart::Hand;
            const QString name = art == Anschlagart::Hand  ? tr("Follow hand")
                                 : art == Anschlagart::Ab  ? tr("Down")
                                 : art == Anschlagart::Auf ? tr("Up")
                                                           : tr("Together");
            QToolTip::showText(hilfe->globalPos(),
                               tr("Strum: %1 — click: Follow hand, Down, Up, Together in turn; "
                                  "right-click a note: Strum, Spread...")
                                   .arg(name),
                               this);
            return true;
        }
        // Stufe 26c: eine leere Stelle der Tab-Linien sagt, was ein Klick tut
        if (const int stelle = tabSaiteBei(hilfe->pos().y()); stelle >= 0 && tabNoteBei(hilfe->pos()) < 0)
        {
            QToolTip::showText(hilfe->globalPos(),
                               tr("String %1 (%2): click here and type a fret — two digits quickly for 10 and up")
                                   .arg(saitenNummer(m_saiten, stelle))
                                   .arg(saitenTon(m_saiten.leer[static_cast<std::size_t>(stelle)] + m_saiten.capo)),
                               this);
            return true;
        }
        if (const int platz = std::max(noteBei(hilfe->pos()), tabNoteBei(hilfe->pos())); platz >= 0)
        {
            const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
            const Griff griff = griffe()[static_cast<std::size_t>(platz)];
            const int nummer = saitenNummer(m_saiten, griff.saite);
            QString text = !griff.greifbar ? tr("Not playable on any free string — sounds on string %1").arg(nummer)
                                           : tr("String %1, fret %2").arg(nummer).arg(griff.bund);
            if (griff.greifbar && note.saite >= 0 && note.saite == griff.saite) text += tr(" — set by you");
            if (note.saite >= 0 && note.saite != griff.saite)
                text += tr(" (set to string %1, which cannot play it here)").arg(saitenNummer(m_saiten, note.saite));
            if (tabNoteBei(hilfe->pos()) == platz) text += tr(" — drag to another line: the same note on that string");
            QToolTip::showText(hilfe->globalPos(), text, this);
            return true;
        }
        QToolTip::hideText();
        event->ignore();
        return true;
    }
    return QWidget::event(event);
}

int NotenRaster::schrittUnter(int x) const
{
    return teileAb(x - kTastenBreite + m_rollX, schrittBreite());
}

int NotenRaster::hoeheUnter(int y) const
{
    return kHoeheMax - teileAb(y - rasterOben() + m_rollY, zeilenHoehe());
}

void NotenRaster::richteMarken()
{
    std::vector<NotenOrt> gibtEs;
    for (const NotenOrt& ort : m_marken)
        if (findeNote(m_muster, ort) >= 0 && std::find(gibtEs.begin(), gibtEs.end(), ort) == gibtEs.end())
            gibtEs.push_back(ort);
    if (gibtEs.size() <= 1)
    {
        // eine einzelne Marke folgt der gewaehlten Note
        m_marken.clear();
        if (auswahl() >= 0) m_marken.push_back({m_wahlSchritt, m_wahlHoehe});
        return;
    }
    m_marken = std::move(gibtEs);
    // die gewaehlte gehoert zu den markierten — sonst wird es die erste von ihnen
    if (std::find(m_marken.begin(), m_marken.end(), NotenOrt{m_wahlSchritt, m_wahlHoehe}) == m_marken.end())
    {
        m_wahlSchritt = m_marken.front().schritt;
        m_wahlHoehe = m_marken.front().hoehe;
    }
}

bool NotenRaster::istMarkiert(const Note& note) const
{
    return std::find(m_marken.begin(), m_marken.end(), NotenOrt{note.schritt, note.hoehe}) != m_marken.end();
}

void NotenRaster::markiere(const std::vector<NotenOrt>& orte)
{
    std::vector<NotenOrt> neu;
    for (const NotenOrt& ort : orte)
        if (findeNote(m_muster, ort) >= 0 && std::find(neu.begin(), neu.end(), ort) == neu.end()) neu.push_back(ort);
    // die gewaehlte Note: bleibt, wenn sie dabei ist — sonst die erste
    const bool dabei = std::find(neu.begin(), neu.end(), NotenOrt{m_wahlSchritt, m_wahlHoehe}) != neu.end();
    if (!dabei)
    {
        m_wahlSchritt = neu.empty() ? -1 : neu.front().schritt;
        m_wahlHoehe = neu.empty() ? -1 : neu.front().hoehe;
    }
    if (neu == m_marken && dabei) return;
    m_marken = std::move(neu);
    update();
    Q_EMIT auswahlGeaendert();
}

void NotenRaster::markiereUm(NotenOrt ort)
{
    if (findeNote(m_muster, ort) < 0) return;
    std::vector<NotenOrt> neu = m_marken;
    const auto platz = std::find(neu.begin(), neu.end(), ort);
    if (platz == neu.end())
    {
        neu.push_back(ort);
        m_wahlSchritt = ort.schritt;  // die zuletzt dazugenommene ist die gewaehlte
        m_wahlHoehe = ort.hoehe;
    }
    else
    {
        neu.erase(platz);
        m_wahlSchritt = neu.empty() ? -1 : neu.back().schritt;
        m_wahlHoehe = neu.empty() ? -1 : neu.back().hoehe;
    }
    m_marken = std::move(neu);
    update();
    Q_EMIT auswahlGeaendert();
}

void NotenRaster::markiereAlle()
{
    std::vector<NotenOrt> alle;
    for (const Note& note : m_muster.noten) alle.push_back({note.schritt, note.hoehe});
    markiere(alle);
    // B3 a: Ctrl+A nimmt das ganze Muster als Zeitraum
    m_zeitraum = std::pair<int, int>{0, schritte()};
    m_zeitraumMarken = m_marken;
}

std::optional<std::pair<int, int>> NotenRaster::zeitraum() const
{
    return m_zeitraum && m_marken == m_zeitraumMarken ? m_zeitraum : std::nullopt;
}

void NotenRaster::setzeAbspielstelle(int schritt)
{
    const int neu = std::clamp(schritt, 0, std::max(0, schritte() - 1));
    if (neu == m_abspiel) return;
    m_abspiel = neu;
    update();
    Q_EMIT abspielstelleGewaehlt(m_abspiel);
}

void NotenRaster::setzeEinfuegemarke(int schritt)
{
    const int neu = std::clamp(schritt, 0, schritte());
    if (neu == m_einfuegen) return;
    m_einfuegen = neu;
    update();
    Q_EMIT auswahlGeaendert();
}

void NotenRaster::uebernimm(const Muster& neu, const std::vector<NotenOrt>& marken, const QString& was)
{
    const bool anders = neu != m_muster;  // seit Stufe 24e auch die Anschlaege (J), nicht nur die Noten
    m_muster = neu;
    m_marken = marken;
    m_wahlSchritt = marken.empty() ? -1 : marken.front().schritt;
    m_wahlHoehe = marken.empty() ? -1 : marken.front().hoehe;
    update();
    Q_EMIT auswahlGeaendert();
    if (anders) Q_EMIT geaendert(m_muster, was);
}

void NotenRaster::setzeMeldung(const NotenRest& rest)
{
    // nichts still weglassen: sagen, was nicht gepasst hat
    QStringList teile;
    if (rest.weg > 0) teile << tr("%n note(s) did not fit into the pattern", nullptr, rest.weg);
    if (rest.gekuerzt > 0) teile << tr("%n note(s) shortened at the end of the pattern", nullptr, rest.gekuerzt);
    m_meldung = teile.join(QStringLiteral(", "));
}

bool NotenRaster::kopiere()
{
    const NotenKopie kopie = kopiereNoten(m_muster, m_marken, zeitraum());
    if (kopie.leer()) return false;
    m_kopie = kopie;
    m_meldung.clear();
    Q_EMIT auswahlGeaendert();  // Paste ist jetzt moeglich
    return true;
}

bool NotenRaster::schneideAus()
{
    if (!kopiere()) return false;
    return loescheAuswahl();
}

bool NotenRaster::fuegeEinBei(int schritt)
{
    if (m_kopie.leer() || m_zug != Zug::Keiner) return false;
    Muster neu = m_muster;
    std::vector<NotenOrt> eingefuegt;
    setzeMeldung(fuegeNotenEin(neu, m_kopie, schritt, schritte(), &eingefuegt));
    m_schiebeGilt = false;
    if (eingefuegt.empty())
    {
        Q_EMIT auswahlGeaendert();
        return false;
    }
    uebernimm(neu, eingefuegt, tr("Paste notes"));
    return true;
}

bool NotenRaster::fuegeEin()
{
    if (!fuegeEinBei(m_einfuegen)) return false;
    // die Marke rueckt weiter: noch einmal Ctrl+V fuellt den naechsten Abschnitt
    setzeEinfuegemarke(m_einfuegen + m_kopie.schritte);
    return true;
}

bool NotenRaster::dupliziere()
{
    if (m_zug != Zug::Keiner) return false;
    const NotenKopie kopie = kopiereNoten(m_muster, m_marken, zeitraum());
    if (kopie.leer()) return false;
    int erster = m_marken.front().schritt;
    for (const NotenOrt& ort : m_marken) erster = std::min(erster, ort.schritt);
    if (const auto raum = zeitraum()) erster = std::min(erster, raum->first);  // B3 a: hinter den Rahmen
    Muster neu = m_muster;
    std::vector<NotenOrt> eingefuegt;
    setzeMeldung(fuegeNotenEin(neu, kopie, erster + kopie.schritte, schritte(), &eingefuegt));
    m_schiebeGilt = false;
    if (eingefuegt.empty())
    {
        Q_EMIT auswahlGeaendert();
        return false;
    }
    uebernimm(neu, eingefuegt, tr("Duplicate notes"));
    return true;
}

bool NotenRaster::bewege(int schrittVersatz, int hoeheVersatz, bool imKreis)
{
    if (m_marken.empty() || m_zug != Zug::Keiner) return false;
    // eine Folge von Tasten zaehlt vom Stand beim ersten Druck: so bleibt
    // stehen, was die Auswahl unterwegs ueberdeckt
    if (!m_schiebeGilt || m_schiebeImKreis != imKreis || m_marken != m_schiebeMarken ||
        m_muster.noten != m_schiebeErgebnis.noten)
    {
        m_schiebeBasis = m_muster;
        m_schiebeOrte = m_marken;
        m_schiebeSchritt = 0;
        m_schiebeHoehe = 0;
        m_schiebeImKreis = imKreis;
    }
    const int zahl = schritte();
    const int schrittGanz =
        imKreis ? (m_schiebeSchritt + schrittVersatz) % std::max(1, zahl) : m_schiebeSchritt + schrittVersatz;
    const int hoeheGanz = m_schiebeHoehe + hoeheVersatz;
    Muster neu = m_schiebeBasis;
    std::vector<NotenOrt> orte = m_schiebeOrte;
    int gekuerzt = 0;
    if (schrittGanz != 0 || hoeheGanz != 0)
    {
        const bool ging = imKreis ? dreheNoten(neu, orte, schrittGanz, zahl, &gekuerzt)
                                  : versetzeNoten(neu, orte, schrittGanz, hoeheGanz, zahl);
        if (!ging) return false;  // am Rand: nichts bewegt sich (V3 a)
    }
    m_schiebeSchritt = schrittGanz;
    m_schiebeHoehe = hoeheGanz;
    m_schiebeErgebnis = neu;
    m_schiebeMarken = orte;
    m_schiebeGilt = true;
    NotenRest rest;
    rest.gekuerzt = gekuerzt;
    setzeMeldung(rest);
    uebernimm(neu, orte, imKreis ? tr("Rotate notes") : tr("Move notes"));
    return true;
}

bool NotenRaster::versetze(int schrittVersatz, int hoeheVersatz)
{
    return bewege(schrittVersatz, hoeheVersatz, false);
}

bool NotenRaster::drehe(int schrittVersatz)
{
    return bewege(schrittVersatz, 0, true);
}

void NotenRaster::setMarke(double schritt)
{
    if (schritt == m_marke) return;
    m_marke = schritt;
    update();
}

void NotenRaster::setNebenMarken(std::vector<double> schritte)
{
    if (schritte == m_neben) return;
    m_neben = std::move(schritte);
    update();
}

int NotenRaster::auswahl() const
{
    if (m_wahlSchritt < 0) return -1;
    for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
        if (m_muster.noten[i].schritt == m_wahlSchritt && m_muster.noten[i].hoehe == m_wahlHoehe)
            return static_cast<int>(i);
    return -1;
}

bool NotenRaster::loescheAuswahl()
{
    const int platz = auswahl();
    if (platz < 0 || m_zug != Zug::Keiner) return false;
    m_schiebeGilt = false;
    m_meldung.clear();
    if (m_marken.size() > 1)
    {
        Muster neu = m_muster;
        if (!loescheNoten(neu, m_marken)) return false;
        uebernimm(neu, {}, tr("Delete notes"));
        return true;
    }
    m_muster.noten.erase(m_muster.noten.begin() + platz);
    richteAnschlaege(m_muster);  // ein Akkord ohne Noten hat keine Abweichung mehr (S-W4 a)
    m_wahlSchritt = m_wahlHoehe = -1;
    m_marken.clear();
    update();
    Q_EMIT auswahlGeaendert();
    Q_EMIT geaendert(m_muster, tr("Delete note"));
    return true;
}

int NotenRaster::rollXMax() const
{
    return std::max(0, schritte() * schrittBreite() - sichtBreite());
}

int NotenRaster::rollYMax() const
{
    return std::max(0, kHoehen * zeilenHoehe() - sichtHoehe());
}

void NotenRaster::setRollen(int x, int y)
{
    const int neuX = std::clamp(x, 0, rollXMax());
    const int neuY = std::clamp(y, 0, rollYMax());
    if (neuX == m_rollX && neuY == m_rollY) return;
    m_rollX = neuX;
    m_rollY = neuY;
    update();
}

void NotenRaster::zeigeHoehe(int hoehe)
{
    setRollen(m_rollX, (kHoeheMax - hoehe) * zeilenHoehe() + zeilenHoehe() / 2 - sichtHoehe() / 2);
}

int NotenRaster::schrittBei(int x) const
{
    if (x < kTastenBreite) return -1;
    const int schritt = (x - kTastenBreite + m_rollX) / schrittBreite();
    return schritt < schritte() ? schritt : -1;
}

int NotenRaster::hoeheBei(int y) const
{
    if (y < rasterOben() || y >= lautOben()) return -1;
    const int hoehe = kHoeheMax - (y - rasterOben() + m_rollY) / zeilenHoehe();
    return hoehe >= kHoeheMin && hoehe <= kHoeheMax ? hoehe : -1;
}

int NotenRaster::noteBei(QPoint ort) const
{
    const int schritt = schrittBei(ort.x());
    const int hoehe = hoeheBei(ort.y());
    if (schritt < 0 || hoehe < 0) return -1;
    for (int i = static_cast<int>(m_muster.noten.size()) - 1; i >= 0; --i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        if (note.hoehe == hoehe && schritt >= note.schritt && schritt < note.schritt + note.laenge) return i;
    }
    return -1;
}

bool NotenRaster::belegt(int schritt, int hoehe, int ausser) const
{
    for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        if (i != ausser && note.schritt == schritt && note.hoehe == hoehe) return true;
    }
    return false;
}

void NotenRaster::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QPalette pal = palette();
    const QColor grund = pal.color(QPalette::Base);
    const QColor text = pal.color(QPalette::Text);
    const QColor an = pal.color(QPalette::Highlight);
    p.fillRect(rect(), grund);
    QColor leise = text;
    leise.setAlpha(130);
    const int zahl = schritte();
    const int jeTakt = m_schlaege * m_jeSchlag;
    const int oben = rasterOben();  // seit S-W14 a liegt mit Saiten die Spur Strum darueber
    const int unten = lautOben();
    const QRect feld(kTastenBreite, oben, sichtBreite(), unten - oben);
    const int rechts = std::min(width(), xVonSchritt(zahl));

    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));

    // --- die Zeilen der Tonhoehen: die der schwarzen Tasten abgesetzt, jedes C mit einer Linie
    p.setClipRect(feld);
    for (int hoehe = kHoeheMin; hoehe <= kHoeheMax; ++hoehe)
    {
        const int y = yVonHoehe(hoehe);
        if (y + zeilenHoehe() < oben || y > unten) continue;
        if (schwarzeTaste(hoehe))
        {
            QColor dunkel = text;
            dunkel.setAlpha(22);
            p.fillRect(QRect(kTastenBreite, y, rechts - kTastenBreite, zeilenHoehe()), dunkel);
        }
        if (hoehe % 12 == 0)
        {
            QColor linie = text;
            linie.setAlpha(70);
            p.setPen(linie);
            p.drawLine(kTastenBreite, y + zeilenHoehe() - 1, rechts, y + zeilenHoehe() - 1);
        }
    }
    // --- die Schritte: jeder Schlag kraeftiger, jeder Takt am kraeftigsten
    for (int i = 0; i <= zahl; ++i)
    {
        const int x = xVonSchritt(i);
        if (x < kTastenBreite || x > width()) continue;
        QColor linie = text;
        linie.setAlpha(i % jeTakt == 0 ? 110 : i % m_jeSchlag == 0 ? 55 : 22);
        p.setPen(linie);
        p.drawLine(x, oben, x, unten);
    }
    // --- die Noten: so kraeftig, wie sie laut sind; die gewaehlte mit Rand
    const int gewaehlt = auswahl();
    // Stufe 23f: mit Saiten traegt jede Note die Nummer ihrer Saite (S-U2 a, C–E)
    const bool mitSaiten = m_saiten.istSaiteninstrument();
    std::vector<Griff> alleGriffe;
    std::vector<Anschlagplan> alleAnschlaege;
    plane(&alleGriffe, &alleAnschlaege);
    p.setFont(klein);
    for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        // I (Stufe 24e): um den Versatz des Strum verschoben
        const int versatz = versatzPixel(alleAnschlaege[static_cast<std::size_t>(i)].versatzMs);
        const QRect r(xVonSchritt(note.schritt) + 1 + versatz, yVonHoehe(note.hoehe) + 1,
                      note.laenge * schrittBreite() - 1, zeilenHoehe() - 2);
        const Griff& griff = alleGriffe[static_cast<std::size_t>(i)];
        const bool nichtGreifbar = mitSaiten && !griff.greifbar;
        QColor farbe = nichtGreifbar ? QColor(226, 75, 74) : m_muster.stumm ? text : an;
        farbe.setAlphaF((m_muster.stumm ? 0.2 : 0.4) + (m_muster.stumm ? 0.25 : 0.6) * note.laut);
        // markierte Noten tragen einen Rand (die gewaehlte gehoert dazu)
        p.setPen(i == gewaehlt || istMarkiert(note) ? QPen(text, m_marken.size() > 1 ? 1.6 : 1.0) : Qt::NoPen);
        p.setBrush(farbe);
        p.drawRoundedRect(r, 2, 2);
        if (!mitSaiten) continue;
        if (nichtGreifbar)
        {
            // E: nicht greifbar — rot schraffiert
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(QColor(255, 255, 255, 70), Qt::BDiagPattern));  // durchscheinend: die Schrift bleibt lesbar
            p.drawRoundedRect(r, 2, 2);
        }
        // D: selbst gesetzt (und dort gespielt) — eine Nadel vor der Nummer
        const bool nadel = griff.greifbar && note.saite >= 0 && note.saite == griff.saite;
        int links = r.left() + 3;
        const QColor schrift = nichtGreifbar ? QColor(Qt::white) : pal.color(QPalette::HighlightedText);
        if (nadel && r.width() >= 14)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(schrift);
            p.drawEllipse(QPoint(links + 2, r.top() + 4), 2, 2);
            p.setPen(QPen(schrift, 1.2));
            p.drawLine(links + 2, r.top() + 5, links + 2, r.bottom() - 1);
            links += 7;
        }
        const QString nummer = QString::number(saitenNummer(m_saiten, griff.saite));
        QString zeichen = !nichtGreifbar ? nummer : r.width() >= 20 ? QStringLiteral("! ") + nummer : QStringLiteral("!");
        // K (Stufe 24e): die Artikulationen hinter der Nummer — was nicht passt, schneidet der Balken ab
        if (const QString kuerzel = artikulationsKuerzel(note.artikulationen); !kuerzel.isEmpty())
            zeichen += QLatin1Char(' ') + kuerzel;
        // A–D (Stufe 26d, S-L3 a): H oder P nach der Richtung, SL, ↗ mit der Hoehe des Bends
        if ((note.artikulationen & kLegato) != 0)
            zeichen += griff.vorige < 0 ? QStringLiteral(" H/P")
                       : note.hoehe >= m_muster.noten[static_cast<std::size_t>(griff.vorige)].hoehe ? QStringLiteral(" H")
                                                                                                     : QStringLiteral(" P");
        if ((note.artikulationen & kSlide) != 0) zeichen += QStringLiteral(" SL");
        if (!note.bend.empty()) zeichen += QString::fromUtf8(" ↗") + bendText(bendHoehe(note.bend));
        p.setPen(schrift);
        p.drawText(QRect(links, r.top(), std::max(0, r.right() - links), r.height()), Qt::AlignLeft | Qt::AlignVCenter, zeichen);
        // E: verbunden, aber ohne vorige Note auf der Saite — sie schlaegt normal an: ein rotes „!" davor
        if (griff.verbindungFehlt)
        {
            p.setPen(QColor(226, 75, 74));
            p.drawText(QRect(r.left() - 10, r.top() - 2, 9, r.height() + 4), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("!"));
        }
        // D: der Bend als Kurve in der Note bis zur Zeile des erreichten Tons, gestrichelt das Ziel
        if (!note.bend.empty())
        {
            const double mitte = yVonHoehe(note.hoehe) + zeilenHoehe() / 2.0;
            const double ziel = mitte - bendHoehe(note.bend) * zeilenHoehe();
            QColor kurve = text;
            kurve.setAlpha(220);
            p.setPen(QPen(kurve, 1.0, Qt::DashLine));
            p.drawLine(QPointF(r.left(), ziel), QPointF(r.right(), ziel));
            QPolygonF linie;
            for (int k = 0; k <= 24; ++k)
            {
                const double t = k / 24.0;
                linie << QPointF(r.left() + t * r.width(), mitte - bendBei(note.bend, t) * zeilenHoehe());
            }
            p.setPen(QPen(kurve, 1.6));
            p.setBrush(Qt::NoBrush);
            p.drawPolyline(linie);
            p.drawText(QRectF(r.right() - 40, ziel - zeilenHoehe(), 40, zeilenHoehe()), Qt::AlignRight | Qt::AlignBottom,
                       bendText(bendHoehe(note.bend)));
        }
    }
    // A–C (Stufe 26d, S-L3 a): von der vorigen Note auf derselben Saite her — Hammer-on und Pull-off
    // als Bogen mit h oder p, ein Slide als schraege Linie mit Pfeil
    if (mitSaiten)
    {
        QColor bogen = text;
        bogen.setAlpha(200);
        for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
        {
            const Note& note = m_muster.noten[i];
            const int vorige = alleGriffe[i].vorige;
            if (vorige < 0 || (note.artikulationen & kVerbunden) == 0) continue;
            const Note& vor = m_muster.noten[static_cast<std::size_t>(vorige)];
            const QPointF von(xVonSchritt(vor.schritt) + versatzPixel(alleAnschlaege[static_cast<std::size_t>(vorige)].versatzMs) +
                                  vor.laenge * schrittBreite(),
                              yVonHoehe(vor.hoehe) + zeilenHoehe() / 2.0);
            const QPointF nach(xVonSchritt(note.schritt) + versatzPixel(alleAnschlaege[i].versatzMs) + 1,
                               yVonHoehe(note.hoehe) + zeilenHoehe() / 2.0);
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(bogen, 1.4));
            if ((note.artikulationen & kSlide) != 0)
            {
                p.drawLine(von, nach);
                const double winkel = std::atan2(nach.y() - von.y(), nach.x() - von.x());
                const QPointF spitze[3] = {nach, nach - QPointF(std::cos(winkel - 0.45), std::sin(winkel - 0.45)) * 6.0,
                                           nach - QPointF(std::cos(winkel + 0.45), std::sin(winkel + 0.45)) * 6.0};
                p.setBrush(bogen);
                p.drawPolygon(spitze, 3);
                p.setBrush(Qt::NoBrush);
                continue;
            }
            // der Bogen ueber beide Noten, von der Oberkante der vorigen zur Oberkante der Zielnote
            const QPointF a(von.x() - 4, von.y() - zeilenHoehe() / 2.0);
            const QPointF b(nach.x() + 4, nach.y() - zeilenHoehe() / 2.0);
            const double hoch = std::min(a.y(), b.y()) - 9.0;
            QPainterPath pfad(a);
            pfad.cubicTo(QPointF(a.x(), hoch), QPointF(b.x(), hoch), b);
            p.drawPath(pfad);
            p.setPen(bogen);
            p.drawText(QRectF((a.x() + b.x()) / 2.0 - 8, hoch - 13, 16, 12), Qt::AlignCenter,
                       note.hoehe >= vor.hoehe ? QStringLiteral("h") : QStringLiteral("p"));
        }
    }
    // I (Stufe 24e): je Akkord ein feiner Strich durch die Anschlaege, in der Reihenfolge
    // des Anschlags; der Punkt sitzt an der zuerst angeschlagenen Note
    if (mitSaiten)
    {
        QColor strich = text;
        strich.setAlpha(170);
        std::vector<std::size_t> folge(m_muster.noten.size());
        std::iota(folge.begin(), folge.end(), std::size_t{0});
        std::stable_sort(folge.begin(), folge.end(), [&](std::size_t a, std::size_t b) {
            const Note& x = m_muster.noten[a];
            const Note& y = m_muster.noten[b];
            if (x.schritt != y.schritt) return x.schritt < y.schritt;
            return alleAnschlaege[a].versatzMs < alleAnschlaege[b].versatzMs;
        });
        for (std::size_t k = 0; k < folge.size();)
        {
            std::size_t ende = k + 1;
            const int schritt = m_muster.noten[folge[k]].schritt;
            while (ende < folge.size() && m_muster.noten[folge[ende]].schritt == schritt) ++ende;
            if (ende - k > 1)
            {
                QPolygon linie;
                for (std::size_t j = k; j < ende; ++j)
                {
                    const Note& note = m_muster.noten[folge[j]];
                    linie << QPoint(xVonSchritt(note.schritt) + 1 + versatzPixel(alleAnschlaege[folge[j]].versatzMs),
                                    yVonHoehe(note.hoehe) + zeilenHoehe() / 2);
                }
                p.setPen(QPen(strich, 1.0));
                p.setBrush(Qt::NoBrush);
                p.drawPolyline(linie);
                p.setPen(Qt::NoPen);
                p.setBrush(strich);
                p.drawEllipse(linie.front(), 2, 2);
            }
            k = ende;
        }
    }
    p.setFont(font());
    p.setBrush(Qt::NoBrush);
    // --- der Rahmen, der Noten markiert
    if (m_zug == Zug::Rahmen && !m_rahmen.isEmpty())
    {
        QColor flaeche = an;
        p.setPen(QPen(flaeche, 1.0, Qt::DashLine));
        flaeche.setAlpha(40);
        p.setBrush(flaeche);
        p.drawRect(m_rahmen);
        p.setBrush(Qt::NoBrush);
    }
    // --- die Abspielstelle (B4 b): dort beginnt Play pattern — gruen
    {
        const int x = xVonSchritt(m_abspiel);
        if (x >= kTastenBreite && x <= width())
        {
            p.setPen(QPen(QColor(80, 200, 120), 1.2));
            p.drawLine(x, oben, x, unten);
        }
    }
    // --- die Einfuegemarke: dort landet Ctrl+V
    {
        const int x = xVonSchritt(m_einfuegen);
        if (x >= kTastenBreite && x <= width())
        {
            p.setPen(QPen(an.lighter(150), 1.4, Qt::DashLine));
            p.drawLine(x, oben, x, unten);
        }
    }

    // --- die Klaviatur
    p.setClipRect(QRect(0, oben, kTastenBreite, unten - oben));
    p.setFont(klein);
    for (int hoehe = kHoeheMin; hoehe <= kHoeheMax; ++hoehe)
    {
        const int y = yVonHoehe(hoehe);
        if (y + zeilenHoehe() < oben || y > unten) continue;
        const QRect taste(0, y, kTastenBreite - 1, zeilenHoehe());
        const bool schwarz = schwarzeTaste(hoehe);
        p.fillRect(taste, hoehe == m_taste ? an : schwarz ? QColor(40, 40, 44) : QColor(232, 232, 228));
        p.setPen(QColor(120, 120, 120));
        p.drawLine(0, y + zeilenHoehe() - 1, kTastenBreite - 1, y + zeilenHoehe() - 1);
        if (hoehe % 12 == 0)
        {
            p.setPen(QColor(40, 40, 44));
            p.drawText(taste.adjusted(4, 0, -2, 0), Qt::AlignLeft | Qt::AlignVCenter, tonName(hoehe));
        }
    }
    // B (Stufe 23f): ein Punkt auf den Tasten der leeren Saiten — mit Capo der klingende Ton
    if (mitSaiten)
    {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(186, 117, 23));
        for (const int leer : m_saiten.leer)
        {
            const int y = yVonHoehe(leer + m_saiten.capo);
            p.drawEllipse(QPoint(kTastenBreite - 8, y + zeilenHoehe() / 2), 3, 3);
        }
        p.setBrush(Qt::NoBrush);
    }

    // --- der Kopf: die Takte
    p.setClipRect(QRect(kTastenBreite, 0, sichtBreite(), kKopfHoehe));
    p.setPen(leise);
    for (int takt = 0; takt < m_muster.takte; ++takt)
        p.drawText(QRect(xVonSchritt(takt * jeTakt) + 3, 0, 80, kKopfHoehe), Qt::AlignLeft | Qt::AlignVCenter,
                   tr("Bar %1").arg(takt + 1));
    {
        // die Abspielstelle im Kopf: eine gruene Flagge (B4 b)
        const int x = xVonSchritt(m_abspiel);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(80, 200, 120));
        const QPoint flagge[3] = {QPoint(x, 1), QPoint(x + 8, 5), QPoint(x, 9)};
        p.drawPolygon(flagge, 3);
        p.setBrush(Qt::NoBrush);
    }
    {
        // die Einfuegemarke im Kopf: ein kleines Dreieck
        const int x = xVonSchritt(m_einfuegen);
        p.setPen(Qt::NoPen);
        p.setBrush(an.lighter(150));
        const QPoint spitze[3] = {QPoint(x - 4, 1), QPoint(x + 4, 1), QPoint(x, 7)};
        p.drawPolygon(spitze, 3);
        p.setBrush(Qt::NoBrush);
    }

    // --- unten: die Lautstaerke je Note, an ihrem Einsatz
    p.setClipRect(QRect(kTastenBreite, unten, sichtBreite(), kLautHoehe));
    p.setPen(leise);
    p.drawLine(kTastenBreite, unten, width(), unten);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        const int hoch = std::max(2, static_cast<int>(note.laut * (kLautHoehe - 6) + 0.5));
        QColor farbe = an;
        if (i != gewaehlt) farbe.setAlpha(150);
        p.setBrush(farbe);
        p.drawRect(QRect(xVonSchritt(note.schritt) + 2, unten + kLautHoehe - 1 - hoch, schrittBreite() - 5, hoch));
    }
    p.setClipping(false);
    p.setFont(font());
    p.setPen(leise);
    p.setFont(klein);
    // (die Spalte der Klaviatur ist schmal: „Note volume" passt nicht hinein)
    p.drawText(QRect(2, unten, kTastenBreite - 4, kLautHoehe), Qt::AlignLeft | Qt::AlignVCenter, tr("Note\nvol."));

    // --- J (Stufe 24e, S-W14 a, S-W15 a): die Spur Strum zwischen den Takten und den Noten —
    // an jedem Schritt, an dem eine Note beginnt, die Richtung
    if (mitSaiten)
    {
        const int strumY = strumOben();
        p.setPen(leise);
        p.drawLine(0, strumY, width(), strumY);  // gegen die Zeile der Takte abgesetzt
        p.drawLine(0, strumY + kStrumHoehe - 1, width(), strumY + kStrumHoehe - 1);
        p.drawText(QRect(2, strumY, kTastenBreite - 4, kStrumHoehe), Qt::AlignLeft | Qt::AlignVCenter, tr("Strum"));
        p.setClipRect(QRect(kTastenBreite, strumY, sichtBreite(), kStrumHoehe));
        // je Schritt: wie viele Noten anschlagen, die Richtung der ersten, ob nur getippt wird
        std::vector<int> zahlJe(static_cast<std::size_t>(std::max(0, zahl)), 0);
        std::vector<Richtung> richtungJe(zahlJe.size(), Richtung::Ab);
        std::vector<char> nurGetippt(zahlJe.size(), 1);
        for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
        {
            const int s = m_muster.noten[i].schritt;
            if (s < 0 || s >= zahl) continue;
            if (zahlJe[static_cast<std::size_t>(s)]++ == 0) richtungJe[static_cast<std::size_t>(s)] = alleAnschlaege[i].richtung;
            if ((m_muster.noten[i].artikulationen & kTapping) == 0) nurGetippt[static_cast<std::size_t>(s)] = 0;
        }
        QFont pfeil = font();
        pfeil.setBold(true);
        p.setFont(pfeil);
        for (int s = 0; s < zahl; ++s)
        {
            const int anzahl = zahlJe[static_cast<std::size_t>(s)];
            if (anzahl == 0) continue;
            const QRect zelle(xVonSchritt(s), strumY + 1, schrittBreite(), kStrumHoehe - 1);
            if (zelle.right() < kTastenBreite || zelle.left() > width()) continue;
            const Anschlag* eintrag = anschlagAn(m_muster, s);
            const QColor farbe = eintrag != nullptr ? an : leise;
            // ein getippter Ton hat keine Richtung (§5o): „T"; sonst der Pfeil, auch bei einer einzelnen Note
            const QString zeichen = nurGetippt[static_cast<std::size_t>(s)] != 0 ? QStringLiteral("T")
                                    : anzahl > 1 && eintrag != nullptr && eintrag->art == Anschlagart::Zugleich
                                        ? QString::fromUtf8("≡")
                                    : richtungJe[static_cast<std::size_t>(s)] == Richtung::Auf ? QString::fromUtf8("↑")
                                                                                               : QString::fromUtf8("↓");
            p.setPen(farbe);
            p.drawText(zelle, Qt::AlignCenter, zeichen);
            if (eintrag != nullptr)
            {
                // die Nadel: von dir gesetzt (wie D an den Noten)
                p.setPen(Qt::NoPen);
                p.setBrush(farbe);
                p.drawEllipse(QPoint(zelle.right() - 2, zelle.top() + 3), 2, 2);
                p.setPen(QPen(farbe, 1.0));
                p.drawLine(zelle.right() - 2, zelle.top() + 4, zelle.right() - 2, zelle.top() + 8);
            }
        }
        p.setBrush(Qt::NoBrush);
        p.setClipping(false);
        p.setFont(font());
    }
    // --- A, B (Stufe 26c, S-T5 a): die Tab-Linien zwischen der Spur Strum und den Noten
    if (mitSaiten && tabHoehe() > 0) zeichneTab(p, alleGriffe, alleAnschlaege);

    // --- W8 a (S130): die Stellen der leiseren Clips desselben Loops im Composer, blasser
    p.setPen(QPen(QColor(255, 90, 70, 120), 2.0));
    for (const double neben : m_neben)
    {
        if (neben < 0.0 || neben >= zahl) continue;
        const int x = kTastenBreite + static_cast<int>(neben * schrittBreite()) - m_rollX;
        if (x >= kTastenBreite) p.drawLine(x, 0, x, height());
    }
    // --- die Abspielmarke
    if (m_marke >= 0.0 && m_marke < zahl)
    {
        const int x = kTastenBreite + static_cast<int>(m_marke * schrittBreite()) - m_rollX;
        if (x >= kTastenBreite)
        {
            p.setPen(QPen(QColor(255, 90, 70), 2.0));
            p.drawLine(x, 0, x, height());
        }
    }
}

void NotenRaster::zieheBei(QPoint ort)
{
    const int zahl = schritte();
    if (zahl <= 0) return;
    if (m_zug == Zug::TabSaite)
    {
        // Stufe 26c: die Saite unter der Maus — gesetzt wird erst beim Loslassen
        const int y = std::clamp(ort.y(), tabOben() + kTabRand, tabOben() + tabHoehe() - kTabRand - 1);
        if (const int stelle = tabSaiteBei(y); stelle >= 0 && stelle != m_tabZiel)
        {
            m_tabZiel = stelle;
            update();
        }
        return;
    }
    const int schrittRoh = std::clamp(teileAb(ort.x() - kTastenBreite + m_rollX, schrittBreite()), 0, zahl - 1);
    if (m_zug == Zug::Laut)
    {
        const int schritt = schrittBei(ort.x());
        if (schritt < 0) return;
        const double hoehe = 1.0 - static_cast<double>(ort.y() - lautOben() - 3) / (kLautHoehe - 6);
        const double laut = std::clamp(std::round(hoehe * 100.0) / 100.0, kLautMin, 1.0);
        // die gewaehlte Note, wenn sie hier beginnt — sonst alle, die hier beginnen
        const int gewaehlt = auswahl();
        const bool nurGewaehlte =
            gewaehlt >= 0 && m_muster.noten[static_cast<std::size_t>(gewaehlt)].schritt == schritt;
        for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
        {
            Note& note = m_muster.noten[static_cast<std::size_t>(i)];
            if (note.schritt == schritt && (!nurGewaehlte || i == gewaehlt)) note.laut = laut;
        }
        update();
        return;
    }
    if (m_zug == Zug::Rahmen)
    {
        // was der Rahmen beruehrt — mit Ctrl zu dem dazu, was schon markiert war
        m_rahmen = QRect(m_rahmenStart, ort).normalized();
        std::vector<NotenOrt> orte = m_vorMarken;
        for (const Note& note : m_muster.noten)
        {
            const QRect r(xVonSchritt(note.schritt), yVonHoehe(note.hoehe), note.laenge * schrittBreite(), zeilenHoehe());
            if (r.intersects(m_rahmen)) orte.push_back({note.schritt, note.hoehe});
        }
        markiere(orte);
        // B3 a: der Zeitraum des Rahmens, auf Schritte gerundet — mit Ctrl um den bisherigen erweitert
        {
            const int zahl = schritte();
            int von = std::clamp(schrittUnter(m_rahmen.left() + schrittBreite() / 2), 0, zahl);
            int bis = std::clamp(schrittUnter(m_rahmen.right() + schrittBreite() / 2), 0, zahl);
            if (bis <= von) bis = std::min(zahl, von + 1);
            if (m_vorZeitraum)
            {
                von = std::min(von, m_vorZeitraum->first);
                bis = std::max(bis, m_vorZeitraum->second);
            }
            m_zeitraum = std::pair<int, int>{von, bis};
            m_zeitraumMarken = m_marken;
        }
        update();
        return;
    }
    if (m_zug == Zug::Gruppe)
    {
        // Alle markierten um dieselben Schritte und Halbtoene. Ist dort fuer
        // eine kein Platz mehr (ein Rand), bleibt die Gruppe an der letzten
        // Stelle, an der Platz war.
        const NotenOrt gezogen{m_wahlSchritt, m_wahlHoehe};
        const auto nr = std::find(m_marken.begin(), m_marken.end(), gezogen) - m_marken.begin();
        const auto versuche = [this, zahl](int schrittV, int hoeheV) {
            Muster probe = m_vorZug;
            std::vector<NotenOrt> orte = m_vorMarken;
            if ((schrittV != 0 || hoeheV != 0) && !versetzeNoten(probe, orte, schrittV, hoeheV, zahl)) return false;
            m_muster = std::move(probe);
            m_marken = std::move(orte);
            return true;
        };
        const int schrittV = schrittUnter(ort.x()) - m_griffStelle;
        const int hoeheV = hoeheUnter(ort.y()) - m_griffHoehe;
        if (versuche(schrittV, hoeheV))
        {
            m_gutSchritt = schrittV;
            m_gutHoehe = hoeheV;
        }
        else
            (void)versuche(m_gutSchritt, m_gutHoehe);
        // die gegriffene Note bleibt die gewaehlte
        if (nr < static_cast<std::ptrdiff_t>(m_marken.size()))
        {
            m_wahlSchritt = m_marken[static_cast<std::size_t>(nr)].schritt;
            m_wahlHoehe = m_marken[static_cast<std::size_t>(nr)].hoehe;
        }
        update();
        return;
    }
    const int platz = auswahl();
    if (platz < 0) return;
    Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
    if (m_zug == Zug::Laenge)
    {
        note.laenge = std::clamp(schrittRoh - note.schritt + 1, 1, zahl - note.schritt);
    }
    else if (m_zug == Zug::Verschieben)
    {
        const int schritt = std::clamp(schrittRoh - m_griffSchritt, 0, zahl - note.laenge);
        const int hoehe = std::clamp(kHoeheMax - teileAb(ort.y() - rasterOben() + m_rollY, zeilenHoehe()), kHoeheMin, kHoeheMax);
        if (belegt(schritt, hoehe, platz)) return;  // dort beginnt schon eine Note dieser Tonhoehe
        note.schritt = schritt;
        note.hoehe = hoehe;
        m_wahlSchritt = schritt;
        m_wahlHoehe = hoehe;
        m_marken = {{schritt, hoehe}};  // die Marke geht mit der Note
        // die Abweichung ihres Akkords geht mit — gerechnet vom Stand vor dem Zug, damit sie nicht an
        // Akkorden haengen bleibt, ueber die die Note nur hinweggezogen wird (S-W4 a). Waehrend des
        // Zugs ordnet niemand die Noten um: der Platz ist derselbe wie vorher.
        if (static_cast<std::size_t>(platz) < m_vorZug.noten.size())
        {
            const Note& vorher = m_vorZug.noten[static_cast<std::size_t>(platz)];
            nimmAnschlaegeMit(m_muster, m_vorZug, {{NotenOrt{vorher.schritt, vorher.hoehe}, schritt}});
        }
    }
    update();
}

void NotenRaster::mousePressEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    setFocus(Qt::MouseFocusReason);
    if (event->button() == Qt::RightButton)
    {
        // Mit dem Haken „Right-click deletes" loescht der Rechtsklick die Note
        // darunter sofort. Sonst — und auf einer von mehreren markierten oder
        // einer freien Stelle — oeffnet sich das Menue (`contextMenuEvent`);
        // eine einzelne Note wird dafuer markiert, damit „Delete" sie trifft.
        m_rechtsGeloescht = false;
        const int platz = noteBei(ort);
        if (platz < 0 || m_zug != Zug::Keiner) return;
        const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
        if (m_marken.size() > 1 && istMarkiert(note)) return;
        if (!m_rechtsLoescht)
        {
            markiere({{note.schritt, note.hoehe}});
            return;
        }
        m_wahlSchritt = note.schritt;
        m_wahlHoehe = note.hoehe;
        m_marken = {{note.schritt, note.hoehe}};
        m_rechtsGeloescht = loescheAuswahl();
        return;
    }
    if (event->button() != Qt::LeftButton) return;
    const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    m_klickSammelt = false;
    if (ort.y() < kKopfHoehe)
    {
        // die Zeile der Takte (B4 b): ein Klick setzt die Abspielstelle, Shift+Klick die Einfuegemarke
        if (ort.x() < kTastenBreite) return;
        if (shift)
            setzeEinfuegemarke(std::clamp(schrittUnter(ort.x() + schrittBreite() / 2), 0, schritte()));
        else
        {
            const int schritt = std::clamp(schrittUnter(ort.x()), 0, std::max(0, schritte() - 1));
            if (schritt == m_abspiel)
                Q_EMIT abspielstelleGewaehlt(m_abspiel);  // derselbe Schritt noch einmal: wieder dorthin springen
            else
                setzeAbspielstelle(schritt);
        }
        return;
    }
    // J (Stufe 24e, S-W14 a): ein Klick in die Spur Strum schaltet den Anschlag an diesem Schritt weiter
    if (ort.y() < tabOben())
    {
        if (const int schritt = schrittBei(ort.x()); schritt >= 0) (void)schalteAnschlag(schritt);
        return;
    }
    // B (Stufe 26c, S-T5 a): die Tab-Linien — eine Zahl greifen (auf eine andere Linie ziehen),
    // sonst die Tab-Marke setzen; links der Name der Saite spielt die leere an
    if (ort.y() < rasterOben())
    {
        const int stelle = tabSaiteBei(ort.y());
        if (stelle < 0) return;
        if (ort.x() < kTastenBreite)
        {
            Q_EMIT tonVerlangt(m_saiten.leer[static_cast<std::size_t>(stelle)] + m_saiten.capo);
            return;
        }
        if (const int platz = tabNoteBei(ort); platz >= 0)
        {
            const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
            m_wahlSchritt = note.schritt;
            m_wahlHoehe = note.hoehe;
            markiere({{note.schritt, note.hoehe}});
            setzeTabMarke(note.schritt, stelle);  // eine getippte Zahl aendert jetzt ihren Bund
            m_vorZug = m_muster;
            m_neu = false;
            m_tabZiel = stelle;
            m_zug = Zug::TabSaite;
            return;
        }
        if (const int schritt = schrittBei(ort.x()); schritt >= 0) setzeTabMarke(schritt, stelle);
        return;
    }
    if (shift && ort.x() >= kTastenBreite && ort.y() < lautOben())
    {
        // ein Rahmen: markiert die Noten, die er beruehrt
        m_zug = Zug::Rahmen;
        m_neu = false;
        m_rahmenStart = ort;
        m_rahmen = {};
        m_vorZug = m_muster;
        m_vorMarken = ctrl ? m_marken : std::vector<NotenOrt>{};
        m_vorZeitraum = ctrl ? zeitraum() : std::nullopt;
        markiere(m_vorMarken);
        return;
    }
    if (ctrl && ort.x() >= kTastenBreite && ort.y() < lautOben())
    {
        // eine Note dazu oder heraus; gesetzt und gezogen wird dabei nichts
        if (const int platz = noteBei(ort); platz >= 0)
        {
            const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
            markiereUm({note.schritt, note.hoehe});
        }
        return;
    }
    if (ort.y() >= lautOben())
    {
        if (ort.x() < kTastenBreite) return;
        m_zug = Zug::Laut;
        m_neu = false;
        m_vorZug = m_muster;
        zieheBei(ort);
        return;
    }
    const int hoehe = hoeheBei(ort.y());
    if (hoehe < 0) return;
    if (ort.x() < kTastenBreite)
    {
        m_taste = hoehe;  // sie leuchtet, solange die Maus gedrueckt ist
        update();
        Q_EMIT tonVerlangt(hoehe);
        return;
    }
    const int schritt = schrittBei(ort.x());
    if (schritt < 0) return;
    m_vorZug = m_muster;
    const int platz = noteBei(ort);
    if (platz >= 0)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
        const bool kante = anKante(note, ort.x());
        const bool inGruppe = m_marken.size() > 1 && istMarkiert(note);
        m_wahlSchritt = note.schritt;
        m_wahlHoehe = note.hoehe;
        m_neu = false;
        m_griffSchritt = schritt - note.schritt;
        if (inGruppe && !kante)
        {
            // eine von mehreren markierten: die Marken bleiben, die Gruppe laesst sich ziehen
            m_zug = Zug::Gruppe;
            m_klickSammelt = true;
            m_vorMarken = m_marken;
            m_griffStelle = schrittUnter(ort.x());
            m_griffHoehe = hoeheUnter(ort.y());
            m_gutSchritt = 0;
            m_gutHoehe = 0;
        }
        else
        {
            // die Kante gehoert einer Note: danach ist nur sie markiert
            m_marken = {{note.schritt, note.hoehe}};
            m_zug = kante ? Zug::Laenge : Zug::Verschieben;
        }
    }
    else
    {
        // eine neue Note von einem Schritt; weitergezogen wird sie laenger
        m_muster.noten.push_back({schritt, 1, hoehe, 1.0});
        m_wahlSchritt = schritt;
        m_wahlHoehe = hoehe;
        m_marken = {{schritt, hoehe}};
        m_neu = true;
        m_zug = Zug::Laenge;
    }
    m_schiebeGilt = false;
    update();
    Q_EMIT auswahlGeaendert();
}

void NotenRaster::mouseMoveEvent(QMouseEvent* event)
{
    if (m_zug != Zug::Keiner && (event->buttons() & Qt::LeftButton))
        zieheBei(event->position().toPoint());
    else if (event->buttons() == Qt::NoButton)
        zeigeZeiger(event->position().toPoint());
}

void NotenRaster::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_taste >= 0)
    {
        m_taste = -1;
        update();
    }
    if (event->button() != Qt::LeftButton || m_zug == Zug::Keiner) return;
    const Zug zug = m_zug;
    const bool sammelt = m_klickSammelt;
    m_zug = Zug::Keiner;
    m_klickSammelt = false;
    if (zug == Zug::TabSaite)
    {
        // Stufe 26c: auf eine andere Linie gezogen — derselbe Ton auf dieser Saite
        const int platz = auswahl();
        const int ziel = m_tabZiel;
        m_tabZiel = -1;
        if (platz >= 0 && ziel >= 0 && ziel != griffe()[static_cast<std::size_t>(platz)].saite && legeAufSaite(platz, ziel))
            setzeTabMarke(m_wahlSchritt, ziel);
        update();
        return;
    }
    if (zug == Zug::Rahmen)
    {
        m_rahmen = {};  // markieren aendert das Muster nicht
        update();
        return;
    }
    if (m_muster == m_vorZug)
    {
        // ein Klick ohne Zug auf eine von mehreren markierten: nur sie bleibt
        if (zug == Zug::Gruppe && sammelt) markiere({{m_wahlSchritt, m_wahlHoehe}});
        return;
    }
    Q_EMIT auswahlGeaendert();
    const QString was = m_neu                       ? tr("Add note")
                        : zug == Zug::Laenge        ? tr("Change note length")
                        : zug == Zug::Verschieben   ? tr("Move note")
                        : zug == Zug::Gruppe        ? tr("Move notes")
                                                    : tr("Change note volume");
    Q_EMIT geaendert(m_muster, was);
}

void NotenRaster::keyPressEvent(QKeyEvent* event)
{
    const Qt::KeyboardModifiers tasten = event->modifiers() & ~Qt::KeypadModifier;
    const bool nurCtrl = tasten == Qt::ControlModifier;
    // B (Stufe 26c): eine Ziffer an der Tab-Marke setzt dort einen Bund; folgt die zweite
    // kurz darauf, wird er zweistellig (1, 2 → 12) — wie man Tabs tippt
    if (m_tabSchritt >= 0 && tasten == Qt::NoModifier && event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9)
    {
        const int ziffer = event->key() - Qt::Key_0;
        const qint64 jetzt = QDateTime::currentMSecsSinceEpoch();
        const bool weiter = m_tabZiffer >= 0 && jetzt - m_tabZifferZeit <= kTabZifferMs && m_tabZiffer * 10 + ziffer <= kBuendeMax;
        const int bund = weiter ? m_tabZiffer * 10 + ziffer : ziffer;
        (void)setzeBund(bund);
        m_tabZiffer = weiter ? -1 : ziffer;  // nach zwei Ziffern beginnt die naechste einen neuen Bund
        m_tabZifferZeit = jetzt;
        return;
    }
    switch (event->key())
    {
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            if (loescheAuswahl()) return;
            break;
        case Qt::Key_C:
            if (nurCtrl && kopiere()) return;
            break;
        case Qt::Key_X:
            if (nurCtrl && schneideAus()) return;
            break;
        case Qt::Key_V:
            if (nurCtrl)
            {
                (void)fuegeEin();
                return;
            }
            break;
        case Qt::Key_D:
            if (nurCtrl)
            {
                (void)dupliziere();
                return;
            }
            break;
        case Qt::Key_A:
            if (nurCtrl)
            {
                markiereAlle();
                return;
            }
            break;
        case Qt::Key_Escape:
            if (m_tabSchritt >= 0 && m_zug == Zug::Keiner)
            {
                setzeTabMarke(-1, -1);  // zuerst die Tab-Marke, beim naechsten Mal die Marken
                return;
            }
            if (!m_marken.empty() && m_zug == Zug::Keiner)
            {
                markiere({});
                return;
            }
            break;
        case Qt::Key_Left:
        case Qt::Key_Right:
        {
            if (m_marken.empty()) break;
            const int richtung = event->key() == Qt::Key_Left ? -1 : 1;
            if (tasten == Qt::AltModifier)
                (void)drehe(richtung);  // im Kreis (Wunsch Patrik S113)
            else if (tasten == Qt::NoModifier)
                (void)versetze(richtung, 0);
            else
                break;
            return;
        }
        case Qt::Key_Up:
        case Qt::Key_Down:
        {
            // ein Halbton, mit Shift eine Oktave
            if (m_marken.empty() || (tasten != Qt::NoModifier && tasten != Qt::ShiftModifier)) break;
            const int halbtoene = (tasten == Qt::ShiftModifier ? 12 : 1) * (event->key() == Qt::Key_Up ? 1 : -1);
            (void)versetze(0, halbtoene);
            return;
        }
        default:
            break;
    }
    QWidget::keyPressEvent(event);
}

void NotenRaster::contextMenuEvent(QContextMenuEvent* event)
{
    // der Rechtsklick hat gerade eine Note geloescht: kein Menue dazu
    if (m_rechtsGeloescht)
    {
        m_rechtsGeloescht = false;
        return;
    }
    const QPoint ort = event->pos();
    // Stufe 26d (S-T5 a C): auch in den Tab-Linien — eine Zahl wird dafuer markiert wie eine Note
    const bool inTab = tabSaiteBei(ort.y()) >= 0;
    if (inTab)
    {
        if (const int platz = tabNoteBei(ort); platz >= 0 && !istMarkiert(m_muster.noten[static_cast<std::size_t>(platz)]))
        {
            const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
            markiere({{note.schritt, note.hoehe}});
        }
    }
    if (ort.x() < kTastenBreite || (!inTab && ort.y() < rasterOben()) || ort.y() >= lautOben()) return;
    const int hier = std::clamp(schrittUnter(ort.x()), 0, std::max(0, schritte() - 1));
    const bool etwas = !m_marken.empty();
    QMenu menu(this);
    menu.addAction(tr("Copy"), QKeySequence(Qt::CTRL | Qt::Key_C), this, [this] { (void)kopiere(); })->setEnabled(etwas);
    menu.addAction(tr("Cut"), QKeySequence(Qt::CTRL | Qt::Key_X), this, [this] { (void)schneideAus(); })
        ->setEnabled(etwas);
    menu.addAction(tr("Paste here"), this, [this, hier] { (void)fuegeEinBei(hier); })->setEnabled(hatKopie());
    menu.addAction(tr("Duplicate"), QKeySequence(Qt::CTRL | Qt::Key_D), this, [this] { (void)dupliziere(); })
        ->setEnabled(etwas);
    menu.addAction(tr("Delete"), QKeySequence(Qt::Key_Delete), this, [this] { (void)loescheAuswahl(); })
        ->setEnabled(etwas);
    // F (Stufe 23f, S-U2 a): die Saite der markierten Noten — Auto oder eine bestimmte
    if (m_saiten.istSaiteninstrument())
    {
        QMenu* saite = menu.addMenu(tr("String"));
        saite->setObjectName(QStringLiteral("instrumentStringMenu"));
        saite->setEnabled(etwas);
        const std::vector<Griff> alle = griffe();
        std::vector<std::size_t> markierte;
        for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
            if (istMarkiert(m_muster.noten[i])) markierte.push_back(i);
        // die gemeinsame Einstellung der markierten; -2 = verschieden
        int gemeinsam = markierte.empty() ? -2 : m_muster.noten[markierte.front()].saite;
        for (const std::size_t i : markierte)
            if (m_muster.noten[i].saite != gemeinsam) gemeinsam = -2;
        QString autoText = tr("Auto");
        if (markierte.size() == 1) autoText = tr("Auto (now %1)").arg(saitenNummer(m_saiten, alle[markierte.front()].saite));
        QAction* automatisch = saite->addAction(autoText, this, [this] { (void)setzeSaite(-1); });
        automatisch->setCheckable(true);
        automatisch->setChecked(gemeinsam == -1);
        for (int nummer = 1; nummer <= m_saiten.anzahl(); ++nummer)
        {
            const int stelle = m_saiten.anzahl() - nummer;
            bool geht = true;
            for (const std::size_t i : markierte) geht = geht && greifbarAuf(m_saiten, stelle, m_muster.noten[i].hoehe);
            QString text = QStringLiteral("%1 · %2").arg(nummer).arg(tonName(m_saiten.leer[static_cast<std::size_t>(stelle)] + m_saiten.capo));
            if (!geht) text += tr(" (not playable)");
            QAction* eintrag = saite->addAction(text, this, [this, stelle] { (void)setzeSaite(stelle); });
            eintrag->setCheckable(true);
            eintrag->setChecked(gemeinsam == stelle);
            eintrag->setEnabled(geht);
        }

        // L (Stufe 24e, S-W5 a): Strum fuer die Schritte der markierten Noten
        QMenu* strum = menu.addMenu(tr("Strum"));
        strum->setObjectName(QStringLiteral("instrumentStrumMenu"));
        strum->setEnabled(etwas);
        const std::vector<int> strumSchritte = markierteSchritte();
        // die gemeinsame Art und der gemeinsame Spread; leer = verschieden
        std::optional<Anschlagart> art;
        std::optional<double> spread;
        for (std::size_t k = 0; k < strumSchritte.size(); ++k)
        {
            const Anschlag* e = anschlagAn(m_muster, strumSchritte[k]);
            const Anschlagart a = e != nullptr ? e->art : Anschlagart::Hand;
            const double s = e != nullptr ? e->spreadMs : -1.0;
            if (k == 0)
            {
                art = a;
                spread = s;
            }
            if (art && *art != a) art.reset();
            if (spread && *spread != s) spread.reset();
        }
        const std::pair<Anschlagart, QString> arten[] = {{Anschlagart::Hand, tr("Follow hand")},
                                                         {Anschlagart::Ab, tr("Down")},
                                                         {Anschlagart::Auf, tr("Up")},
                                                         {Anschlagart::Zugleich, tr("Together")}};
        for (const auto& [wert, name] : arten)
        {
            QAction* a = strum->addAction(name, this, [this, wert] { (void)setzeAnschlagart(wert); });
            a->setCheckable(true);
            a->setChecked(art && *art == wert);
        }
        strum->addSeparator();
        const QString spreadText = spread && *spread >= 0.0 ? tr("Spread... (%1 ms)").arg(*spread)
                                   : spread ? tr("Spread... (like the pattern, %1 ms)").arg(m_muster.strum.spreadMs)
                                            : tr("Spread...");
        strum->addAction(spreadText, this, [this, spread] { frageSpread(spread.value_or(-1.0)); });

        // L: die Artikulationen der markierten Noten als Haken
        menu.addSeparator();
        std::vector<std::size_t> alleMarkierten;
        for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
            if (istMarkiert(m_muster.noten[i])) alleMarkierten.push_back(i);
        // E (Stufe 26d, S-L3 a): darunter Legato und Slide
        for (const int bit : {kPalmMute, kLetRing, kTapping, kHarmonic, kLegato, kSlide})
        {
            bool alleTragen = !alleMarkierten.empty();
            for (const std::size_t i : alleMarkierten)
                alleTragen = alleTragen && (m_muster.noten[i].artikulationen & bit) != 0;
            QAction* haken = menu.addAction(artikulationsName(bit), this, [this, bit] { (void)schalteArtikulation(bit); });
            haken->setCheckable(true);
            haken->setChecked(alleTragen);
            haken->setEnabled(etwas);
        }

        // F (Stufe 26d, S-L3 a): Bend — die Vorlagen, die Hoehe, Edit bend...
        QMenu* bend = menu.addMenu(tr("Bend"));
        bend->setObjectName(QStringLiteral("instrumentBendMenu"));
        bend->setEnabled(etwas);
        // was alle markierten gemeinsam tragen; sonst kein Haken
        std::optional<std::optional<BendVorlage>> form;
        std::optional<double> hoehe;
        for (std::size_t k = 0; k < alleMarkierten.size(); ++k)
        {
            const std::vector<BendPunkt>& b = m_muster.noten[alleMarkierten[k]].bend;
            if (k == 0)
            {
                form = bendVorlageVon(b);
                hoehe = bendHoehe(b);
            }
            if (form && *form != bendVorlageVon(b)) form.reset();
            if (hoehe && *hoehe != bendHoehe(b)) hoehe.reset();
        }
        for (const BendVorlage vorlage : {BendVorlage::Keine, BendVorlage::Bend, BendVorlage::BendZurueck, BendVorlage::VorBend,
                                          BendVorlage::VorBendZurueck})
        {
            QAction* a = bend->addAction(bendVorlageName(vorlage), this, [this, vorlage] { (void)setzeBendVorlage(vorlage); });
            a->setCheckable(true);
            a->setChecked(form && *form == vorlage);
        }
        bend->addSeparator();
        for (const double h : bendHoehen())
        {
            QAction* a = bend->addAction(tr("Height %1").arg(bendText(h)), this, [this, h] { (void)setzeBendHoehe(h); });
            a->setCheckable(true);
            a->setChecked(hoehe && *hoehe == h);
        }
        bend->addSeparator();
        bend->addAction(tr("Edit bend..."), this, [this] { frageBend(); });
    }
    menu.addSeparator();
    menu.addAction(tr("Select all"), QKeySequence(Qt::CTRL | Qt::Key_A), this, [this] { markiereAlle(); });
    lumi::ui::zeigeMenue(menu, event->globalPos());
}

void NotenRaster::zoomeZeit(double faktor, int xFest)
{
    const double alt = m_schrittBreite;
    const double neu = std::clamp(alt * faktor, kSchrittBreiteMin, kSchrittBreiteMax);
    if (neu == alt) return;
    // der Schritt unter xFest bleibt an seiner Stelle im Fenster
    const int vorne = std::max(0, xFest - kTastenBreite);
    const double schritt = (vorne + m_rollX) / static_cast<double>(schrittBreite());
    m_schrittBreite = neu;
    setRollen(static_cast<int>(std::lround(schritt * schrittBreite())) - vorne, m_rollY);
    update();
    Q_EMIT gerollt();
}

void NotenRaster::setZeilenProzent(int prozent, int yFest)
{
    const int neu = std::clamp(prozent, lumi::ui::Achse::kHoeheMin, lumi::ui::Achse::kHoeheMax);
    if (neu == m_zeilenProzent) return;
    // die Tonhoehe unter yFest bleibt an ihrer Stelle
    const int oben = yFest < 0 ? 0 : std::max(0, yFest - rasterOben());
    const double zeile = (oben + m_rollY) / static_cast<double>(zeilenHoehe());
    m_zeilenProzent = neu;
    setRollen(m_rollX, static_cast<int>(std::lround(zeile * zeilenHoehe())) - oben);
    update();
    Q_EMIT gerollt();
}

void NotenRaster::passeEin()
{
    // die Breite: das ganze Muster — abgerundet, denn gezeichnet wird mit ganzen Pixeln
    // (`schrittBreite()` rundet; ein Bruchteil ab 0,5 liesse das Muster ueber den Rand ragen)
    const int zahl = std::max(1, schritte());
    m_schrittBreite = std::clamp(std::floor(static_cast<double>(sichtBreite()) / zahl), kSchrittBreiteMin, kSchrittBreiteMax);
    // die Hoehe: die Noten mit einer Zeile Luft darueber und darunter (ohne Noten: Normalhoehe um C4)
    int tief = 60;
    int hoch = 60;
    if (!m_muster.noten.empty())
    {
        const auto [min, max] = std::minmax_element(m_muster.noten.begin(), m_muster.noten.end(),
                                                    [](const Note& a, const Note& b) { return a.hoehe < b.hoehe; });
        tief = min->hoehe;
        hoch = max->hoehe;
        const int zeilen = hoch - tief + 3;
        const int prozent = sichtHoehe() * 100 / (zeilen * kZeilenHoehe);
        m_zeilenProzent = std::clamp(prozent - prozent % lumi::ui::Achse::kHoeheSchritt, lumi::ui::Achse::kHoeheMin,
                                     lumi::ui::Achse::kHoeheMax);
    }
    else
        m_zeilenProzent = 100;
    setRollen(0, m_rollY);
    zeigeHoehe((tief + hoch) / 2);
    update();
    Q_EMIT gerollt();  // die Rollbalken des Fensters ziehen nach
}

void NotenRaster::wheelEvent(QWheelEvent* event)
{
    const Qt::KeyboardModifiers tasten =
        event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier);
    // mit einer Taste kann das System das Rad als waagrecht melden: es zaehlt die Achse, die sich bewegt
    const int rasten = event->angleDelta().y() != 0 ? event->angleDelta().y() : event->angleDelta().x();
    if (rasten == 0) return;
    // E-Z1 a: Ctrl+Rad die Zeit, Alt+Rad die Hoehe, Ctrl+Alt+Rad beides — wie die Zeitachse
    const bool zeit = tasten == Qt::ControlModifier || tasten == (Qt::ControlModifier | Qt::AltModifier);
    const bool hoehe = tasten == Qt::AltModifier || tasten == (Qt::ControlModifier | Qt::AltModifier);
    if (zeit || hoehe)
    {
        if (zeit)
            zoomeZeit(std::pow(lumi::ui::Achse::kZoomSchritt, rasten / 120.0),
                      static_cast<int>(event->position().x()));
        if (hoehe)
        {
            // die Hoehe geht in ganzen Schritten: ein feines Rad sammelt, bis es eine Raste ist
            m_radRest += rasten;
            const int ganze = m_radRest / 120;
            m_radRest -= ganze * 120;
            if (ganze != 0)
                setZeilenProzent(m_zeilenProzent + ganze * lumi::ui::Achse::kHoeheSchritt,
                                 static_cast<int>(event->position().y()));
            lumi::ui::Achse::loeseAltVomMenue(*event);
        }
        event->accept();
        return;
    }
    // das Rad rollt die Tonhoehen, mit Shift die Zeit
    if (event->modifiers() & Qt::ShiftModifier)
        setRollen(m_rollX - rasten * schrittBreite() * 4 / 120, m_rollY);
    else
        setRollen(m_rollX, m_rollY - rasten * zeilenHoehe() * 3 / 120);
    Q_EMIT gerollt();
    event->accept();
}

void NotenRaster::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    setRollen(m_rollX, m_rollY);
    Q_EMIT gerollt();
}

// =============================================================================
// InstrumentEditor
// =============================================================================

InstrumentEditor::InstrumentEditor(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopInstrumentEditor"));
    setupUI();
}

void InstrumentEditor::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    auto* kopf = new QHBoxLayout;
    m_basisName = new QLabel(this);
    m_basisName->setObjectName(QStringLiteral("instrumentBase"));
    m_nutzer = new QLabel(this);
    m_nutzer->setObjectName(QStringLiteral("instrumentUsers"));
    m_nutzer->setEnabled(false);  // grau: eine Auskunft
    m_takte = new QSpinBox(this);
    m_takte->setObjectName(QStringLiteral("instrumentBars"));
    m_takte->setRange(kTakteMin, kTakteMax);
    m_takte->setKeyboardTracking(false);
    m_takte->setToolTip(tr("Length of the pattern in bars; its clips grow and shrink with it"));
    // B2 a (S124): laenger gestellt, wiederholen die neuen Takte die bisherigen
    m_wiederholen = new QCheckBox(tr("Repeat when lengthening"), this);
    m_wiederholen->setObjectName(QStringLiteral("instrumentRepeatBars"));
    m_wiederholen->setToolTip(tr("On: more bars repeat the bars you have — one bar becomes four of the same. Off: "
                                 "the new bars are empty"));
    m_wiederholen->setChecked(wiederholenBeimVerlaengern());
    connect(m_wiederholen, &QCheckBox::toggled, this, [](bool an) { setzeWiederholenBeimVerlaengern(an); });
    m_play = new QPushButton(tr("Play pattern"), this);
    m_play->setObjectName(QStringLiteral("instrumentPlay"));
    m_play->setToolTip(tr("Play only this pattern, in a loop"));
    m_spitze = new QLabel(QStringLiteral("–"), this);
    m_spitze->setObjectName(QStringLiteral("instrumentPeak"));
    m_spitze->setMinimumWidth(36);
    m_spitze->setToolTip(tr("Peak of the mixed loop while it plays; above 1 it clips"));
    kopf->addWidget(new QLabel(tr("Base"), this));
    kopf->addWidget(m_basisName);
    kopf->addWidget(m_nutzer);
    kopf->addStretch(1);
    // E-Z1 a (S135): Fit — das ganze Muster in der Breite, die Noten in der Hoehe; der
    // Tooltip nennt die Gesten wie in der Zeitachse (ohne die mittlere Maustaste)
    auto* fit = new QPushButton(tr("Fit"), this);
    fit->setObjectName(QStringLiteral("instrumentFit"));
    fit->setToolTip(tr("The whole pattern in width, the notes in height") + QStringLiteral("\n") +
                    lumi::ui::Achse::zoomGesten().mid(0, 5).join(QLatin1Char('\n')));
    connect(fit, &QPushButton::clicked, this, [this] { m_raster->passeEin(); });
    kopf->addWidget(fit);
    kopf->addSpacing(8);
    kopf->addWidget(new QLabel(tr("Bars"), this));
    kopf->addWidget(m_takte);
    kopf->addWidget(m_wiederholen);
    kopf->addWidget(m_play);
    // E1 a (S124): die Effekte des Bezugsclips an oder aus — derselbe Schalter wie im Loop
    m_fx = new QPushButton(QStringLiteral("FX"), this);
    m_fx->setObjectName(QStringLiteral("instrumentFx"));
    m_fx->setCheckable(true);
    m_fx->setMaximumWidth(fontMetrics().horizontalAdvance(QStringLiteral("FX")) + 28);
    connect(m_fx, &QPushButton::clicked, this, [this](bool an) { (void)setzeEffekteAn(an); });
    kopf->addWidget(m_fx);
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);

    auto* zeile = new QHBoxLayout;
    // der Name des Instruments als Text; gewaehlt wird im Fenster „Instruments"
    // (Stufe 14c, Entscheid P1 a — bis S113 stand hier eine Dropdown-Liste)
    m_instrument = new QLabel(this);
    m_instrument->setObjectName(QStringLiteral("instrumentSound"));
    m_instrument->setMinimumWidth(130);
    {
        QFont fett = m_instrument->font();
        fett.setBold(true);
        m_instrument->setFont(fett);
    }
    m_instrumentSuchen = new QPushButton(tr("Browse..."), this);
    m_instrumentSuchen->setObjectName(QStringLiteral("instrumentBrowse"));
    m_instrumentSuchen->setToolTip(
        tr("Open the Instruments window: search, filter by genre, listen, and use an instrument in this pattern"));
    m_laut = new QSlider(Qt::Horizontal, this);
    m_laut->setObjectName(QStringLiteral("instrumentVolume"));
    m_laut->setRange(0, 100);
    m_laut->setFixedWidth(110);
    m_laut->setToolTip(tr("Volume of the pattern"));
    m_pan = new QSlider(Qt::Horizontal, this);
    m_pan->setObjectName(QStringLiteral("instrumentPan"));
    m_pan->setRange(-100, 100);
    m_pan->setFixedWidth(110);
    m_pan->setToolTip(tr("Pan: left to right"));
    m_stumm = new QToolButton(this);
    m_stumm->setObjectName(QStringLiteral("instrumentMute"));
    m_stumm->setText(QStringLiteral("M"));
    m_stumm->setCheckable(true);
    m_stumm->setToolTip(tr("Mute this pattern"));
    m_instrumentBearbeiten = new QPushButton(tr("Edit instrument..."), this);
    m_instrumentBearbeiten->setObjectName(QStringLiteral("instrumentEdit"));
    m_instrumentBearbeiten->setToolTip(
        tr("Open the instrument creator; a built-in instrument becomes the template of your own file"));
    m_instrumentLaden = new QPushButton(tr("Load instrument..."), this);
    m_instrumentLaden->setObjectName(QStringLiteral("instrumentLoad"));
    m_instrumentLaden->setToolTip(tr("Use an instrument file (.lvinst) for this pattern"));
    m_akkorde = new QPushButton(tr("Chords..."), this);
    m_akkorde->setObjectName(QStringLiteral("instrumentChords"));
    m_akkorde->setToolTip(tr("Open the Chords window: chord shapes for this tuning — listen, and insert one\n"
                             "at the insert marker or drag it onto the tab lines or the piano roll"));
    zeile->addWidget(new QLabel(tr("Instrument"), this));
    // das Datei-Icon der Instrumente vor dem Namen (S120, Entscheid Patrik A2 a)
    if (const QIcon marke = lumi::ui::dateiArtIcon(QStringLiteral("lvinst")); !marke.isNull())
    {
        auto* icon = new QLabel(this);
        icon->setObjectName(QStringLiteral("instrumentSoundIcon"));
        icon->setPixmap(marke.pixmap(18, 18));
        zeile->addWidget(icon);
    }
    zeile->addWidget(m_instrument);
    zeile->addWidget(m_instrumentSuchen);
    zeile->addWidget(m_instrumentBearbeiten);
    zeile->addWidget(m_instrumentLaden);
    zeile->addWidget(m_akkorde);
    // A (Stufe 23f, S-U2 a): String choice — nur, wenn das Instrument Saiten traegt
    m_saitenWahlName = new QLabel(tr("String choice"), this);
    m_saitenWahl = new QComboBox(this);
    m_saitenWahl->setObjectName(QStringLiteral("instrumentStringChoice"));
    m_saitenWahl->addItem(tr("Per chord"), static_cast<int>(Saitenwahl::JeKlang));
    m_saitenWahl->addItem(tr("Follow hand"), static_cast<int>(Saitenwahl::Hand));
    m_saitenWahl->setToolTip(tr("How a note without its own string finds one, in this pattern.\n"
                                "Per chord: notes sounding together on different strings, frets close together.\n"
                                "Follow hand: the same, but staying near where the hand was."));
    zeile->addSpacing(8);
    zeile->addWidget(m_saitenWahlName);
    zeile->addWidget(m_saitenWahl);
    connect(m_saitenWahl, &QComboBox::activated, this,
            [this](int index) { setzeSaitenwahl(static_cast<Saitenwahl>(m_saitenWahl->itemData(index).toInt())); });
    zeile->addWidget(new QLabel(tr("Volume"), this));
    zeile->addWidget(m_laut);
    zeile->addWidget(new QLabel(tr("Pan"), this));
    zeile->addWidget(m_pan);
    zeile->addWidget(m_stumm);
    m_hinweis = new QLabel(this);
    m_hinweis->setObjectName(QStringLiteral("instrumentHint"));
    m_hinweis->setEnabled(false);  // grau: eine Auskunft
    zeile->addSpacing(12);
    zeile->addWidget(m_hinweis);
    zeile->addStretch(1);
    aussen->addLayout(zeile);
    // H (Stufe 24e, S-W5 a): Strum dieses Musters — dasselbe Element wie N im Creator,
    // im Raster des Loops; nur, wenn das Instrument Saiten traegt
    m_strumZeile = new StrumZeile(QStringLiteral("editorStrum"), this);
    m_strumZeile->setzeHinweis(tr("for every chord of this pattern"));
    connect(m_strumZeile, &StrumZeile::geaendert, this, [this](const Strum& strum) { setzeStrum(strum); });
    {
        // F (Stufe 26c, S-T5 a): der Haken Tab am Ende der Zeile Strum — die Tab-Linien im Raster
        auto* reihe = new QHBoxLayout;
        reihe->setContentsMargins(0, 0, 0, 0);
        reihe->addWidget(m_strumZeile, 1);
        m_tabHaken = new QCheckBox(tr("Tab"), this);
        m_tabHaken->setObjectName(QStringLiteral("editorTab"));
        m_tabHaken->setToolTip(tr("Show the tab lines between the Strum lane and the notes: one line per string,\n"
                                  "the fret at every note. Click a line and type a fret; drag a number to another line"));
        m_tabHaken->setChecked(!lageMerken() || QSettings().value(QLatin1String(kTabZeigen), true).toBool());
        reihe->addWidget(m_tabHaken);
        aussen->addLayout(reihe);
    }

    // --- das Raster mit seinen Rollbalken
    auto* koerper = new QGridLayout;
    koerper->setSpacing(0);
    m_raster = new NotenRaster(this);
    m_raster->setTabAn(m_tabHaken->isChecked());
    connect(m_tabHaken, &QCheckBox::toggled, this, [this](bool an) {
        m_raster->setTabAn(an);
        passeRollbalkenAn();  // die Noten haben mehr oder weniger Hoehe
        if (lageMerken()) QSettings().setValue(QLatin1String(kTabZeigen), an);
    });
    m_rollY = new QScrollBar(Qt::Vertical, this);
    m_rollX = new QScrollBar(Qt::Horizontal, this);
    koerper->addWidget(m_raster, 0, 0);
    koerper->addWidget(m_rollY, 0, 1);
    koerper->addWidget(m_rollX, 1, 0);
    koerper->setRowStretch(0, 1);
    koerper->setColumnStretch(0, 1);
    aussen->addLayout(koerper, 1);
    // G (Stufe 23f, S-U2 a): was die Zeichen an den Noten heissen — nur mit Saiten
    // M (Stufe 24e, S-W5 a): dazu die Pfeile der Spur Strum und die Kuerzel der Artikulationen
    // G (Stufe 26c, S-T5 a): dazu die Tab-Linien
    m_saitenHinweis = new QLabel(tr("number = string (1 = highest) · pin = set by you · ! = not playable, sounds on the "
                                    "nearest string · Strum lane (top): ↓ ↑ grey = from the hand, blue = set by you, ≡ = "
                                    "together, T = tapped, click to change · Tab lines: click a line and type a fret (two "
                                    "digits quickly for 10 and up), drag a number to another line = the same note on that "
                                    "string · PM palm mute · LR let ring · T tapping · ◇ "
                                    "harmonic · H/P (h, p) hammer-on / pull-off, SL (/) slide, ↗ bend with its height "
                                    "(½, full, 1½) · right-click a note: String, Strum, playing styles, Bend"),
                                 this);
    m_saitenHinweis->setObjectName(QStringLiteral("instrumentStringHint"));
    m_saitenHinweis->setEnabled(false);
    m_saitenHinweis->setWordWrap(true);
    aussen->addWidget(m_saitenHinweis);
    // wie man mehrere Noten markiert — oder was markiert ist und was die Tasten tun
    m_auswahlZeile = new QLabel(this);
    m_auswahlZeile->setObjectName(QStringLiteral("instrumentSelection"));
    m_auswahlZeile->setEnabled(false);
    m_auswahlZeile->setWordWrap(true);
    // „Right-click deletes" (Wunsch Patrik S113): an = der Rechtsklick loescht
    // die Note sofort; aus = er oeffnet das Menue. Die App merkt die Stellung.
    m_rechtsLoescht = new QCheckBox(tr("Right-click deletes"), this);
    m_rechtsLoescht->setObjectName(QStringLiteral("instrumentRightClickDeletes"));
    m_rechtsLoescht->setToolTip(tr("On: a right-click deletes the note under the mouse at once. Off: it opens the "
                                   "menu, which has an entry for that"));
    m_rechtsLoescht->setChecked(QSettings().value(QLatin1String(kRechtsLoescht), false).toBool());
    auto* auswahlReihe = new QHBoxLayout;
    auswahlReihe->addWidget(m_auswahlZeile, 1);
    auswahlReihe->addWidget(m_rechtsLoescht, 0, Qt::AlignTop);
    aussen->addLayout(auswahlReihe);

    connect(m_takte, &QSpinBox::valueChanged, this, [this](int takte) { setzeTakte(takte); });
    connect(m_play, &QPushButton::clicked, this, [this] { Q_EMIT spielenVerlangt(!m_spielt); });
    connect(m_instrumentSuchen, &QPushButton::clicked, this, [this] { oeffnePalette(); });
    connect(m_akkorde, &QPushButton::clicked, this, [this] { oeffneAkkorde(); });
    connect(m_stumm, &QToolButton::toggled, this, [this](bool stumm) { setzeStumm(stumm); });
    connect(m_instrumentBearbeiten, &QPushButton::clicked, this, [this] { (void)oeffneCreator(); });
    connect(m_instrumentLaden, &QPushButton::clicked, this, [this] {
        QSettings einstellungen;
        // beim ersten Mal: die eigenen Dokumente — eine fremde Datei liegt irgendwo
        QString ordner = einstellungen.value(QLatin1String(kInstrumentOrdner)).toString();
        if (ordner.isEmpty() || !QFileInfo(ordner).isDir())
            ordner = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const QString pfad =
            QFileDialog::getOpenFileName(this, tr("Load instrument"), ordner, tr("Instrument (*.lvinst);;All files (*)"));
        if (pfad.isEmpty()) return;
        einstellungen.setValue(QLatin1String(kInstrumentOrdner), QFileInfo(pfad).absolutePath());
        // Entscheid E3 a (S117): zu den eigenen kopieren oder am Ort lassen. Liegt
        // die Datei schon bei den eigenen, ist nichts zu fragen.
        bool kopieren = QDir::cleanPath(QFileInfo(pfad).absolutePath())
                            .compare(QDir::cleanPath(eigenerInstrumentOrdner()), Qt::CaseInsensitive) == 0;
        if (!kopieren)
        {
            QMessageBox frage(this);
            frage.setObjectName(QStringLiteral("instrumentCopyDialog"));
            frage.setWindowTitle(tr("Instrument"));
            frage.setIcon(QMessageBox::Question);
            frage.setText(tr("Where should %1 be kept?").arg(QFileInfo(pfad).fileName()));
            frage.setInformativeText(tr("A copy in My files goes along with an export and stays when the original moves. "
                                        "Without a copy the pattern names the file at\n%1")
                                         .arg(QDir::toNativeSeparators(pfad)));
            QPushButton* zuEigenen = frage.addButton(tr("Copy the file to My files (recommended)"), QMessageBox::AcceptRole);
            QPushButton* amOrt = frage.addButton(tr("Use it where it is"), QMessageBox::ActionRole);
            frage.addButton(QMessageBox::Cancel);
            frage.setDefaultButton(zuEigenen);
            frage.exec();
            if (frage.clickedButton() != zuEigenen && frage.clickedButton() != amOrt) return;
            kopieren = frage.clickedButton() == zuEigenen;
        }
        if (!nimmInstrumentDatei(pfad, kopieren))
            QMessageBox::warning(this, tr("Instrument"),
                                 tr("Could not copy %1 to My files").arg(QDir::toNativeSeparators(pfad)));
    });
    // ein Schieber meldet beim Loslassen — nicht jeden Pixel als Schritt fuer Undo
    const auto schieber = [this](QSlider* regler, bool pan) {
        const auto melde = [this, regler, pan] {
            if (pan)
                setzePan(regler->value() / 100.0);
            else
                setzeLaut(regler->value() / 100.0);
        };
        connect(regler, &QSlider::sliderReleased, this, melde);
        connect(regler, &QSlider::valueChanged, this, [regler, melde] {
            if (!regler->isSliderDown()) melde();
        });
    };
    schieber(m_laut, false);
    schieber(m_pan, true);

    connect(m_raster, &NotenRaster::geaendert, this, [this](const Muster& neu, const QString& was) {
        Loop fassung = m_loop;
        Muster* ziel = musterIn(fassung, m_muster);
        if (ziel == nullptr) return;
        // was das Raster bearbeitet: die Noten und (seit Stufe 24a/24e) die Anschlaege — die
        // wandern beim Ziehen mit und werden in der Spur Strum und im Menue gesetzt
        ziel->noten = neu.noten;
        ziel->anschlaege = neu.anschlaege;
        fassung.richte();  // sortiert die Noten, kuerzt am Ende des Musters
        if (fassung == m_loop)
        {
            zeigeAlles();  // nichts geaendert: das Raster zeigt wieder, was gilt
            return;
        }
        Q_EMIT bearbeitet(fassung, was);
    });
    connect(m_raster, &NotenRaster::tonVerlangt, this, &InstrumentEditor::tonVerlangt);
    connect(m_raster, &NotenRaster::auswahlGeaendert, this, &InstrumentEditor::zeigeAuswahl);
    connect(m_raster, &NotenRaster::abspielstelleGewaehlt, this, &InstrumentEditor::abspielstelleGewaehlt);
    m_raster->setRechtsklickLoescht(m_rechtsLoescht->isChecked());
    connect(m_rechtsLoescht, &QCheckBox::toggled, this, [this](bool an) {
        m_raster->setRechtsklickLoescht(an);
        QSettings().setValue(QLatin1String(kRechtsLoescht), an);
    });
    connect(m_raster, &NotenRaster::gerollt, this, &InstrumentEditor::passeRollbalkenAn);
    connect(m_rollX, &QScrollBar::valueChanged, this, [this](int x) { m_raster->setRollen(x, m_raster->rollY()); });
    connect(m_rollY, &QScrollBar::valueChanged, this, [this](int y) { m_raster->setRollen(m_raster->rollX(), y); });
    resize(1100, 560);
    if (lageMerken()) (void)restoreGeometry(QSettings().value(QLatin1String(kLage)).toByteArray());
}

void InstrumentEditor::liesEigeneNeu()
{
    if (m_palette) m_palette->liesNeu();
}

void InstrumentEditor::setzeLageMerken(bool an)
{
    lageMerken() = an;
}

void InstrumentEditor::merkeLage() const
{
    if (lageMerken()) QSettings().setValue(QLatin1String(kLage), saveGeometry());
}

void InstrumentEditor::hideEvent(QHideEvent* event)
{
    merkeLage();
    QWidget::hideEvent(event);
}

void InstrumentEditor::passeRollbalkenAn()
{
    const QSignalBlocker a(m_rollX);
    const QSignalBlocker b(m_rollY);
    m_rollX->setRange(0, m_raster->rollXMax());
    m_rollX->setPageStep(std::max(1, m_raster->sichtBreite()));
    m_rollX->setSingleStep(m_raster->schrittBreite());
    m_rollX->setValue(m_raster->rollX());
    m_rollY->setRange(0, m_raster->rollYMax());
    m_rollY->setPageStep(std::max(1, m_raster->sichtHoehe()));
    m_rollY->setSingleStep(m_raster->zeilenHoehe());
    m_rollY->setValue(m_raster->rollY());
    // ein Balken, der nichts zu rollen hat, bleibt weg
    m_rollX->setVisible(m_raster->rollXMax() > 0);
}

void InstrumentEditor::zeigeAusschnitt()
{
    // auf die Noten — ohne Noten um C4 (bei der Hoehe des Fensters rund C3 bis C5)
    int mitte = 60;
    if (const Muster* ziel = basis(); ziel != nullptr && !ziel->noten.empty())
    {
        int tief = kHoeheMax;
        int hoch = kHoeheMin;
        for (const Note& note : ziel->noten)
        {
            tief = std::min(tief, note.hoehe);
            hoch = std::max(hoch, note.hoehe);
        }
        mitte = (tief + hoch) / 2;
    }
    m_raster->setRollen(0, m_raster->rollY());
    m_raster->zeigeHoehe(mitte);
    passeRollbalkenAn();
}

void InstrumentEditor::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (m_gezeigt) return;
    m_gezeigt = true;
    zeigeAusschnitt();
}

bool InstrumentEditor::zeige(const Loop& loop, const QString& muster)
{
    const Muster* ziel = loop.findeMuster(muster);
    if (ziel == nullptr || ziel->art != MusterArt::Instrument) return false;
    const bool andere = muster != m_muster;
    // der Creator gehoert zum Instrument der Basis, die gerade gezeigt wird:
    // bei einer anderen Basis schliesst er sich (und fragt nach Ungespeichertem)
    if (andere && m_creator != nullptr && m_creator->isVisible() && !m_creator->close()) return false;
    m_loop = loop;
    m_muster = muster;
    zeigeAlles();
    if (andere) zeigeAusschnitt();
    return true;
}

void InstrumentEditor::setLoop(const Loop& loop)
{
    if (m_muster.isEmpty()) return;
    const QString folge = folgeBasis(m_loop, m_muster, loop);
    m_loop = loop;
    if (folge.isEmpty())
    {
        // die Basis gibt es nicht mehr (ihr letzter Clip ist geloescht, ein anderer Loop geladen)
        m_muster.clear();
        if (m_palette != nullptr) m_palette->setzeZiel(QString());
        close();
        return;
    }
    m_muster = folge;
    zeigeAlles();
}

void InstrumentEditor::zeigeAlles()
{
    const Muster* ziel = basis();
    if (ziel == nullptr) return;
    setWindowTitle(tr("Instrument pattern: %1").arg(m_muster));
    m_basisName->setText(m_muster);
    m_nutzer->setText(tr("· used by %1 clips").arg(basisNutzer(m_loop, m_muster)));
    {
        const QSignalBlocker a(m_takte);
        const QSignalBlocker c(m_laut);
        const QSignalBlocker d(m_pan);
        const QSignalBlocker e(m_stumm);
        m_takte->setValue(ziel->takte);
        const QSignalBlocker wiederholen(m_wiederholen);
        m_wiederholen->setChecked(wiederholenBeimVerlaengern());  // der andere Editor kann ihn umgestellt haben
        zeigeFx();
        // ein eigenes und eine Datei stehen mit ihrem Titel da; was sich nicht
        // finden laesst, bleibt stehen — es schweigt
        m_instrument->setText(instrumentAnzeige(ziel->instrument));
        m_instrument->setToolTip(istInstrumentDatei(ziel->instrument)
                                     ? QDir::toNativeSeparators(ziel->instrument)
                                     : istEigenesInstrument(ziel->instrument)
                                           ? QDir::toNativeSeparators(eigeneDatei(
                                                 ziel->instrument.mid(static_cast<int>(qstrlen(kEigenVorsatz)))))
                                           : QString());
        if (!m_laut->isSliderDown()) m_laut->setValue(static_cast<int>(std::lround(ziel->laut * 100.0)));
        if (!m_pan->isSliderDown()) m_pan->setValue(static_cast<int>(std::lround(ziel->pan * 100.0)));
        m_stumm->setChecked(ziel->stumm);
    }
    // Stufe 23f: die Saiten des Instruments — ohne sie ist von alledem nichts zu sehen
    {
        const Saitensatz saiten = saitenVon(ziel->instrument);
        const bool mitSaiten = saiten.istSaiteninstrument();
        m_raster->setSaiten(saiten);
        m_saitenWahlName->setVisible(mitSaiten);
        m_saitenWahl->setVisible(mitSaiten);
        m_saitenHinweis->setVisible(mitSaiten);
        const QSignalBlocker ruhe(m_saitenWahl);
        m_saitenWahl->setCurrentIndex(m_saitenWahl->findData(static_cast<int>(ziel->saitenwahl)));
        // H (Stufe 24e): die Zeile Strum im Raster des Loops
        m_strumZeile->setzeRaster(m_loop.schlaegeJeTakt, m_loop.schritteJeSchlag);
        m_strumZeile->zeige(ziel->strum);
        m_strumZeile->setVisible(mitSaiten);
        m_tabHaken->setVisible(mitSaiten);  // F (Stufe 26c)
        // D (Stufe 26e-2): bei jedem Instrument — mit Saiten Griffbilder, sonst Voicings (26f, S-T14 a)
    }
    m_raster->setTempo(m_loop.bpm);
    m_raster->setMuster(*ziel, m_loop.schlaegeJeTakt, m_loop.schritteJeSchlag);
    passeRollbalkenAn();
    zeigeAuswahl();
    // das Fenster „Instruments" waehlt fuer diese Basis
    if (m_palette != nullptr) m_palette->setzeZiel(m_muster);
    // und das Fenster Chords fuer ihre Stimmung
    if (m_akkordFenster != nullptr)
    {
        Instrument instrument;
        const QString titel = holeInstrument(ziel->instrument, instrument) ? instrument.titel : ziel->instrument;
        // ein Voicing beginnt in der Oktave 3, bei einem Bass in der Oktave 2 (S-T14 a F)
        const int oktav = InstrumentPalette::hoeheFuerArt(instrument.art) <= 36 ? 36 : 48;
        m_akkordFenster->setzeZiel(m_muster, titel, m_raster->saiten(), ziel->strum, m_loop.schritteJeSchlag, oktav);
    }
}

QString InstrumentEditor::instrumentAnzeige(const QString& nennung)
{
    Instrument instrument;
    const bool datei = istInstrumentDatei(nennung);
    const QString kurz = datei ? QFileInfo(nennung).fileName() : nennung;
    if (!holeInstrument(nennung, instrument)) return tr("%1 (not found)").arg(kurz);
    if (istEigenesInstrument(nennung)) return tr("%1 (My files)").arg(instrument.titel);
    return datei ? tr("%1 (%2)").arg(instrument.titel, kurz) : instrument.titel;
}

void InstrumentEditor::setEngineGeber(std::function<IAudioEngine*()> geber)
{
    m_engineGeber = std::move(geber);
    if (m_creator != nullptr) m_creator->setEngineGeber(m_engineGeber);
}

InstrumentPalette* InstrumentEditor::palette() const
{
    return m_palette;
}

void InstrumentEditor::oeffnePalette(bool zeigen)
{
    if (m_palette == nullptr)
    {
        m_palette = new InstrumentPalette(this);
        connect(m_palette, &InstrumentPalette::verwenden, this, &InstrumentEditor::setzeInstrument);
        // ein Klick im Fenster spielt das Instrument an — auf demselben Weg wie der Creator
        connect(m_palette, &InstrumentPalette::tonVerlangt, this, &InstrumentEditor::instrumentTonVerlangt);
        // Ctrl+Z / Ctrl+Y im Fenster: weiter ans Panel, dort liegt Undo
        connect(m_palette, &InstrumentPalette::rueckgaengigVerlangt, this, &InstrumentEditor::rueckgaengigVerlangt);
    }
    m_palette->setzeZiel(m_muster);
    if (zeigen)
        m_palette->zeigeVorn();
    else
        m_palette->liesNeu();
    // das Instrument der Basis ist gewaehlt — ohne es anzuspielen
    if (const Muster* ziel = basis()) (void)m_palette->waehle(ziel->instrument);
}

AkkordFenster* InstrumentEditor::akkordFenster() const
{
    return m_akkordFenster;
}

void InstrumentEditor::oeffneAkkorde(bool zeigen)
{
    if (m_akkordFenster == nullptr)
    {
        m_akkordFenster = new AkkordFenster(this);
        // Use: an die Einfuegemarke des Rasters
        connect(m_akkordFenster, &AkkordFenster::verwenden, this, [this](const Akkord& lage, int bezug) {
            (void)m_raster->setzeAkkord(lage, m_raster->einfuegemarke(), bezug);
        });
        // anhoeren mit dem Instrument dieses Musters
        connect(m_akkordFenster, &AkkordFenster::vorhoerenVerlangt, this, [this](const Muster& noten) {
            if (const Muster* ziel = basis()) Q_EMIT akkordVorhoerenVerlangt(ziel->instrument, noten);
        });
        connect(m_akkordFenster, &AkkordFenster::rueckgaengigVerlangt, this, &InstrumentEditor::rueckgaengigVerlangt);
    }
    zeigeAlles();  // setzt das Ziel des Fensters
    if (zeigen)
        m_akkordFenster->zeigeVorn();
    else
        m_akkordFenster->liesNeu();
}

void InstrumentEditor::zeigeAuswahl()
{
    const auto zahl = m_raster->markierte().size();
    QString text;
    if (zahl == 0)
        // sonst findet es niemand: sagen, wie man markiert
        text = tr("Shift+drag selects notes · Ctrl+click adds or removes one · a click in the bar row sets where Play "
                  "starts, Shift+click the paste marker · Ctrl+wheel zooms the time, Alt+wheel the height");
    else if (zahl == 1)
        text = tr("1 note selected · Ctrl+C copy · Ctrl+V paste at the marker · Ctrl+D duplicate · arrow keys move · "
                  "Shift+drag selects several");
    else
        text = tr("%1 notes selected · drag moves them together · Ctrl+C copy · Ctrl+X cut · Ctrl+V paste at the "
                  "marker · Ctrl+D duplicate · arrow keys move, Shift+Up / Down an octave · Alt+Left / Right rotate · "
                  "Del deletes")
                   .arg(zahl);
    if (!m_raster->meldung().isEmpty()) text = m_raster->meldung() + QStringLiteral(" · ") + text;
    m_auswahlZeile->setText(text);
}

void InstrumentEditor::setSpielt(bool spielt, double schritt)
{
    if (spielt != m_spielt)
    {
        m_spielt = spielt;
        m_play->setText(spielt ? tr("Stop") : tr("Play pattern"));
    }
    m_raster->setMarke(spielt ? schritt : -1.0);
    if (spielt) m_raster->setNebenMarken({});  // eigenes Play: nur die eine Marke
}

void InstrumentEditor::zeigeMarke(double schritt, lumi::ui::Achse::Mitlaufen art, std::vector<double> neben)
{
    m_raster->setMarke(schritt);
    m_raster->setNebenMarken(std::move(neben));
    if (schritt < 0.0 || m_rollX->isSliderDown() || m_rollX->maximum() <= 0) return;
    const int imInhalt = static_cast<int>(schritt * m_raster->schrittBreite());
    const int ziel = lumi::ui::Achse::rollFuerMarke(art, imInhalt, m_raster->sichtBreite(), m_rollX->value());
    if (ziel != m_rollX->value()) m_rollX->setValue(ziel);  // der Balken klemmt und rollt das Raster
}

InstrumentCreator* InstrumentEditor::creator() const
{
    return m_creator;
}

bool InstrumentEditor::oeffneCreator(bool zeigen)
{
    const Muster* ziel = basis();
    Instrument instrument;
    if (ziel == nullptr || !holeInstrument(ziel->instrument, instrument)) return false;
    if (m_creator == nullptr)
    {
        m_creator = new InstrumentCreator(this);
        // gespeichert: die Basis nennt jetzt diese Datei (bei Save in dieselbe
        // Datei aendert sich am Loop nichts — nur der Klang)
        connect(m_creator, &InstrumentCreator::gespeichert, this, [this](const QString& pfad) {
            setzeInstrument(pfad);
            Q_EMIT instrumentGespeichert();
            // ein neues oder geaendertes eigenes: das Fenster „Instruments" zeigt es gleich
            if (m_palette != nullptr)
            {
                m_palette->liesNeu();
                (void)m_palette->waehle(pfad);
            }
            zeigeAlles();  // der Titel in der Zeile kann sich geaendert haben, auch wenn die Nennung blieb
        });
        connect(m_creator, &InstrumentCreator::vorhoerenVerlangt, this, &InstrumentEditor::instrumentVorhoerenVerlangt);
        // die Aufnahme einer Sample-Schicht (Stufe 11a): dekodieren kann nur, wer die Audio-Engine hat
        connect(m_creator, &InstrumentCreator::sampleVerlangt, this, &InstrumentEditor::sampleVerlangt);
        m_creator->setEngineGeber(m_engineGeber);  // fuer „Record..." (Stufe 11b)
    }
    m_creator->setzeSchlagMs(60000.0 / m_loop.bpm);  // „Repeat note" im Tempo des Loops
    m_creator->oeffne(instrument);
    if (zeigen)
    {
        m_creator->show();
        m_creator->raise();
        m_creator->activateWindow();
    }
    return true;
}

void InstrumentEditor::setHinweis(const QString& text)
{
    if (m_hinweis->text() != text) m_hinweis->setText(text);
}

void InstrumentEditor::setSpitze(double spitze)
{
    const QString text = spitze < 0.0 ? QStringLiteral("–") : QString::number(spitze, 'f', 2);
    if (text == m_spitze->text()) return;
    m_spitze->setText(text);
    // ueber 1 uebersteuert die Summe (L6 a)
    m_spitze->setStyleSheet(spitze > 1.0 ? QStringLiteral("color: #e05050; font-weight: bold;") : QString());
}

// --- die Aenderungen: jede meldet eine neue Fassung des Loops ----------------

void InstrumentEditor::setzeTakte(int takte)
{
    Loop neu = m_loop;
    if (!setzeMusterTakte(neu, m_muster, takte, wiederholenBeimVerlaengern()) || neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern length"));
}

void InstrumentEditor::setzeInstrument(const QString& name)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || name.isEmpty() || ziel->instrument == name) return;
    // S-W13 a: String choice und Strum folgen dem neuen, wo das Muster die Vorgabe des alten traegt
    wechsleVorgaben(*ziel, saitenVon(ziel->instrument), saitenVon(name));
    ziel->instrument = name;
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change instrument"));
}

bool InstrumentEditor::nimmInstrumentDatei(const QString& pfad, bool kopieren)
{
    const QString nennung = kopieren ? nimmZuEigenenInstrumenten(pfad) : pfad;
    if (nennung.isEmpty()) return false;
    setzeInstrument(nennung);
    // ein neues eigenes: das Fenster „Instruments" zeigt es gleich
    if (kopieren && m_palette != nullptr)
    {
        m_palette->liesNeu();
        (void)m_palette->waehle(nennung);
    }
    return true;
}

void InstrumentEditor::setzeLaut(double laut)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return;
    ziel->laut = std::clamp(laut, 0.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern volume"));
}

void InstrumentEditor::setzeSaitenwahl(Saitenwahl wahl)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return;
    ziel->saitenwahl = wahl;
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change string choice"));
}

void InstrumentEditor::setzeStrum(const Strum& strum)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return;
    ziel->strum = strum;
    richteStrum(ziel->strum, handStellen(ziel->strum.raster, neu.schlaegeJeTakt, neu.schritteJeSchlag));
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change strum"));
}

void InstrumentEditor::setzePan(double pan)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return;
    ziel->pan = std::clamp(pan, -1.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern pan"));
}

void InstrumentEditor::setzeStumm(bool stumm)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return;
    ziel->stumm = stumm;
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Mute pattern"));
}

void InstrumentEditor::setMarkierterClip(std::optional<ClipOrt> ort)
{
    m_markiert = ort;
    if (m_fx != nullptr) zeigeFx();
}

std::optional<ClipOrt> InstrumentEditor::bezugsclip() const
{
    return lumi::loop::bezugsclip(m_loop, m_muster, m_markiert);
}

bool InstrumentEditor::setzeEffekteAn(bool an)
{
    const std::optional<ClipOrt> ort = bezugsclip();
    Loop neu = m_loop;
    if (!ort || !lumi::loop::setzeEffekteAn(neu, *ort, an))
    {
        zeigeFx();  // der Knopf steht wieder, wie der Clip ist
        return false;
    }
    Q_EMIT bearbeitet(neu, an ? tr("Effects on") : tr("Effects off"));
    return true;
}

void InstrumentEditor::zeigeFx()
{
    const std::optional<ClipOrt> ort = bezugsclip();
    const int clip = ort ? findeClip(m_loop, *ort) : -1;
    const LoopClip* bezug = clip >= 0 ? &m_loop.kanaele[static_cast<std::size_t>(ort->kanal)].clips[static_cast<std::size_t>(clip)] : nullptr;
    const QSignalBlocker ruhe(m_fx);
    const bool hatEffekte = bezug != nullptr && bezug->effekte.einerAn();
    m_fx->setEnabled(hatEffekte);
    m_fx->setChecked(hatEffekte && bezug->effekteAn);
    if (bezug == nullptr)
        m_fx->setToolTip(tr("Effects: this pattern has no clip in the loop yet"));
    else if (!hatEffekte)
        m_fx->setToolTip(tr("Effects of the clip %1: none — set them with Clip effects in the Loop panel").arg(clipText(m_loop, *ort)));
    else
        m_fx->setToolTip(tr("Effects of the clip %1: %2 — Play pattern plays them; the same switch as in the loop. "
                            "Select another clip of this pattern in the loop to use its effects")
                             .arg(clipText(m_loop, *ort), bezug->effekteAn ? tr("on") : tr("off")));
}

}  // namespace lumi::loop
