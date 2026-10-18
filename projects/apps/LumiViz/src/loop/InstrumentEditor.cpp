/**
 * @file InstrumentEditor.cpp
 * @brief Loop: der Instrument-Editor — die Noten einer Basis ueber einer
 *        Klaviatur (Stufe 9b)
 */

#include "loop/InstrumentEditor.hpp"

#include "UI/widgets/Bedienhilfe.hpp"  // zeigeMenue, frageText: in Tests abfragbar
#include "UI/widgets/PresetTypeIcons.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/StrumZeile.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
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
    setMouseTracking(true);  // der Mauszeiger zeigt, was ein Klick taete
}

bool NotenRaster::anKante(const Note& note, int x) const
{
    const int griff = std::clamp(note.laenge * kSchrittBreite / 3, kKante, kKanteMax);
    return x >= xVonSchritt(note.schritt + note.laenge) - griff;
}

void NotenRaster::zeigeZeiger(QPoint ort)
{
    Qt::CursorShape form = Qt::ArrowCursor;
    if (const int platz = noteBei(ort); platz >= 0)
        form = anKante(m_muster.noten[static_cast<std::size_t>(platz)], ort.x()) ? Qt::SizeHorCursor : Qt::SizeAllCursor;
    else if (ort.x() < kTastenBreite && hoeheBei(ort.y()) >= 0)
        form = Qt::PointingHandCursor;
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
    update();
}

std::vector<Griff> NotenRaster::griffe() const
{
    std::vector<Griff> ergebnis(m_muster.noten.size());
    if (!m_saiten.istSaiteninstrument()) return ergebnis;
    // die Planung will das Muster gerichtet — waehrend eines Zugs ist es das nicht immer:
    // auf einer sortierten Kopie greifen und die Griffe zurueck an ihre Plaetze legen
    std::vector<std::size_t> folge(m_muster.noten.size());
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
    for (std::size_t k = 0; k < folge.size(); ++k) ergebnis[folge[k]] = gegriffen[k];
    return ergebnis;
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

bool NotenRaster::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip && m_saiten.istSaiteninstrument())
    {
        const auto* hilfe = static_cast<QHelpEvent*>(event);
        if (const int platz = noteBei(hilfe->pos()); platz >= 0)
        {
            const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
            const Griff griff = griffe()[static_cast<std::size_t>(platz)];
            const int nummer = saitenNummer(m_saiten, griff.saite);
            QString text = !griff.greifbar ? tr("Not playable on any free string — sounds on string %1").arg(nummer)
                                           : tr("String %1, fret %2").arg(nummer).arg(griff.bund);
            if (griff.greifbar && note.saite >= 0 && note.saite == griff.saite) text += tr(" — set by you");
            if (note.saite >= 0 && note.saite != griff.saite)
                text += tr(" (set to string %1, which cannot play it here)").arg(saitenNummer(m_saiten, note.saite));
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
    return teileAb(x - kTastenBreite + m_rollX, kSchrittBreite);
}

int NotenRaster::hoeheUnter(int y) const
{
    return kHoeheMax - teileAb(y - kKopfHoehe + m_rollY, kZeilenHoehe);
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
    const bool anders = neu.noten != m_muster.noten;
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
    return std::max(0, schritte() * kSchrittBreite - sichtBreite());
}

int NotenRaster::rollYMax() const
{
    return std::max(0, kHoehen * kZeilenHoehe - sichtHoehe());
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
    setRollen(m_rollX, (kHoeheMax - hoehe) * kZeilenHoehe + kZeilenHoehe / 2 - sichtHoehe() / 2);
}

int NotenRaster::schrittBei(int x) const
{
    if (x < kTastenBreite) return -1;
    const int schritt = (x - kTastenBreite + m_rollX) / kSchrittBreite;
    return schritt < schritte() ? schritt : -1;
}

int NotenRaster::hoeheBei(int y) const
{
    if (y < kKopfHoehe || y >= lautOben()) return -1;
    const int hoehe = kHoeheMax - (y - kKopfHoehe + m_rollY) / kZeilenHoehe;
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
    const int oben = kKopfHoehe;
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
        if (y + kZeilenHoehe < oben || y > unten) continue;
        if (schwarzeTaste(hoehe))
        {
            QColor dunkel = text;
            dunkel.setAlpha(22);
            p.fillRect(QRect(kTastenBreite, y, rechts - kTastenBreite, kZeilenHoehe), dunkel);
        }
        if (hoehe % 12 == 0)
        {
            QColor linie = text;
            linie.setAlpha(70);
            p.setPen(linie);
            p.drawLine(kTastenBreite, y + kZeilenHoehe - 1, rechts, y + kZeilenHoehe - 1);
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
    const std::vector<Griff> alleGriffe = griffe();
    p.setFont(klein);
    for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        const QRect r(xVonSchritt(note.schritt) + 1, yVonHoehe(note.hoehe) + 1, note.laenge * kSchrittBreite - 1,
                      kZeilenHoehe - 2);
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
        const QString zeichen = !nichtGreifbar ? nummer : r.width() >= 20 ? QStringLiteral("! ") + nummer : QStringLiteral("!");
        p.setPen(schrift);
        p.drawText(QRect(links, r.top(), std::max(0, r.right() - links), r.height()), Qt::AlignLeft | Qt::AlignVCenter, zeichen);
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
        if (y + kZeilenHoehe < oben || y > unten) continue;
        const QRect taste(0, y, kTastenBreite - 1, kZeilenHoehe);
        const bool schwarz = schwarzeTaste(hoehe);
        p.fillRect(taste, hoehe == m_taste ? an : schwarz ? QColor(40, 40, 44) : QColor(232, 232, 228));
        p.setPen(QColor(120, 120, 120));
        p.drawLine(0, y + kZeilenHoehe - 1, kTastenBreite - 1, y + kZeilenHoehe - 1);
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
            p.drawEllipse(QPoint(kTastenBreite - 8, y + kZeilenHoehe / 2), 3, 3);
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
        p.drawRect(QRect(xVonSchritt(note.schritt) + 2, height() - 1 - hoch, kSchrittBreite - 5, hoch));
    }
    p.setClipping(false);
    p.setFont(font());
    p.setPen(leise);
    p.setFont(klein);
    // (die Spalte der Klaviatur ist schmal: „Note volume" passt nicht hinein)
    p.drawText(QRect(2, unten, kTastenBreite - 4, kLautHoehe), Qt::AlignLeft | Qt::AlignVCenter, tr("Note\nvol."));

    // --- W8 a (S130): die Stellen der leiseren Clips desselben Loops im Composer, blasser
    p.setPen(QPen(QColor(255, 90, 70, 120), 2.0));
    for (const double neben : m_neben)
    {
        if (neben < 0.0 || neben >= zahl) continue;
        const int x = kTastenBreite + static_cast<int>(neben * kSchrittBreite) - m_rollX;
        if (x >= kTastenBreite) p.drawLine(x, 0, x, height());
    }
    // --- die Abspielmarke
    if (m_marke >= 0.0 && m_marke < zahl)
    {
        const int x = kTastenBreite + static_cast<int>(m_marke * kSchrittBreite) - m_rollX;
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
    const int schrittRoh = std::clamp(teileAb(ort.x() - kTastenBreite + m_rollX, kSchrittBreite), 0, zahl - 1);
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
            const QRect r(xVonSchritt(note.schritt), yVonHoehe(note.hoehe), note.laenge * kSchrittBreite, kZeilenHoehe);
            if (r.intersects(m_rahmen)) orte.push_back({note.schritt, note.hoehe});
        }
        markiere(orte);
        // B3 a: der Zeitraum des Rahmens, auf Schritte gerundet — mit Ctrl um den bisherigen erweitert
        {
            const int zahl = schritte();
            int von = std::clamp(schrittUnter(m_rahmen.left() + kSchrittBreite / 2), 0, zahl);
            int bis = std::clamp(schrittUnter(m_rahmen.right() + kSchrittBreite / 2), 0, zahl);
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
        const int hoehe = std::clamp(kHoeheMax - teileAb(ort.y() - kKopfHoehe + m_rollY, kZeilenHoehe), kHoeheMin, kHoeheMax);
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
            setzeEinfuegemarke(std::clamp(schrittUnter(ort.x() + kSchrittBreite / 2), 0, schritte()));
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
    if (ort.x() < kTastenBreite || ort.y() < kKopfHoehe || ort.y() >= lautOben()) return;
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
    }
    menu.addSeparator();
    menu.addAction(tr("Select all"), QKeySequence(Qt::CTRL | Qt::Key_A), this, [this] { markiereAlle(); });
    lumi::ui::zeigeMenue(menu, event->globalPos());
}

void NotenRaster::wheelEvent(QWheelEvent* event)
{
    // das Rad rollt die Tonhoehen, mit Shift die Zeit
    const int rasten = event->angleDelta().y() != 0 ? event->angleDelta().y() : event->angleDelta().x();
    if (rasten == 0) return;
    if (event->modifiers() & Qt::ShiftModifier)
        setRollen(m_rollX - rasten * kSchrittBreite * 4 / 120, m_rollY);
    else
        setRollen(m_rollX, m_rollY - rasten * kZeilenHoehe * 3 / 120);
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
    aussen->addWidget(m_strumZeile);

    // --- das Raster mit seinen Rollbalken
    auto* koerper = new QGridLayout;
    koerper->setSpacing(0);
    m_raster = new NotenRaster(this);
    m_rollY = new QScrollBar(Qt::Vertical, this);
    m_rollX = new QScrollBar(Qt::Horizontal, this);
    koerper->addWidget(m_raster, 0, 0);
    koerper->addWidget(m_rollY, 0, 1);
    koerper->addWidget(m_rollX, 1, 0);
    koerper->setRowStretch(0, 1);
    koerper->setColumnStretch(0, 1);
    aussen->addLayout(koerper, 1);
    // G (Stufe 23f, S-U2 a): was die Zeichen an den Noten heissen — nur mit Saiten
    m_saitenHinweis = new QLabel(tr("number = string (1 = highest) · pin = set by you · ! = not playable, sounds on the "
                                    "nearest string · right-click a note: String"),
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
        ziel->noten = neu.noten;
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
    m_rollX->setSingleStep(NotenRaster::kSchrittBreite);
    m_rollX->setValue(m_raster->rollX());
    m_rollY->setRange(0, m_raster->rollYMax());
    m_rollY->setPageStep(std::max(1, m_raster->sichtHoehe()));
    m_rollY->setSingleStep(NotenRaster::kZeilenHoehe);
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
    }
    m_raster->setMuster(*ziel, m_loop.schlaegeJeTakt, m_loop.schritteJeSchlag);
    passeRollbalkenAn();
    zeigeAuswahl();
    // das Fenster „Instruments" waehlt fuer diese Basis
    if (m_palette != nullptr) m_palette->setzeZiel(m_muster);
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

void InstrumentEditor::zeigeAuswahl()
{
    const auto zahl = m_raster->markierte().size();
    QString text;
    if (zahl == 0)
        // sonst findet es niemand: sagen, wie man markiert
        text = tr("Shift+drag selects notes · Ctrl+click adds or removes one · a click in the bar row sets where Play "
                  "starts, Shift+click the paste marker");
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
    const int imInhalt = static_cast<int>(schritt * NotenRaster::kSchrittBreite);
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
