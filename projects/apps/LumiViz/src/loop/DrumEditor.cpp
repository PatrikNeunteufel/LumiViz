/**
 * @file DrumEditor.cpp
 * @brief Loop: der Drum-Editor — das Schritt-Raster einer Basis (Stufe 6b)
 */

#include "loop/DrumEditor.hpp"

#include "loop/AufnahmeFenster.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/Kit.hpp"  // bibliotheksKlaenge
#include "loop/Sample.hpp"
#include "loop/SampleFrage.hpp"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

// Die Eintraege der Klang-Auswahl, die kein Klang sind (Stufe 11c, Schritt 2):
const QString kDateiWahl = QStringLiteral("?file");              ///< „Sample file...": fragt nach einer Tondatei
const QString kInstrumentWahl = QStringLiteral("?instrument");   ///< „Instrument...": das Fenster Instruments
const QString kAufnahmeWahl = QStringLiteral("?record");         ///< „Record sample...": das Fenster Record sample
const QString kInstrumentJetzt = QStringLiteral("?current");     ///< das Instrument, das die Spur spielt
const QString kKopf = QStringLiteral("?head");                   ///< eine Ueberschrift, nicht zu waehlen
const char* const kSampleOrdner = "loop/lastSampleDir";
const char* const kRechtsLoescht = "loop/drumRightClickDeletes";
const char* const kSpurNr = "spur";
constexpr int kKlangBreite = 124;    ///< die Auswahl „Sound" — breit genug fuer den Titel eines Instruments
constexpr int kMehrBreite = 22;      ///< der Knopf „…" einer Instrument-Spur
constexpr int kSpaltenBreite = 368;  ///< die Kopfzeilen der Spuren, links vom Raster
constexpr int kRand = 4;             ///< vor dem ersten und hinter dem letzten Schritt

}  // namespace

// =============================================================================
// SchrittRaster
// =============================================================================

SchrittRaster::SchrittRaster(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("loopSteps"));
    setFocusPolicy(Qt::StrongFocus);  // Ctrl+C, Pfeiltasten: die Tasten gelten dem Raster
}

void SchrittRaster::setMuster(const Muster& muster, int schlaegeJeTakt, int schritteJeSchlag)
{
    m_muster = muster;
    m_schlaege = std::max(1, schlaegeJeTakt);
    m_jeSchlag = std::max(1, schritteJeSchlag);
    m_zug = Zug::Keiner;
    // die Marken bleiben, wo es ihre Schritte noch gibt (kuerzeres Muster, weniger Spuren)
    const int zahl = schritte();
    const int spurZahl = static_cast<int>(m_muster.spuren.size());
    m_marken.erase(std::remove_if(m_marken.begin(), m_marken.end(),
                                  [zahl, spurZahl](const SchrittOrt& ort) {
                                      return ort.spur >= spurZahl || ort.schritt >= zahl;
                                  }),
                   m_marken.end());
    m_einfuegen = std::clamp(m_einfuegen, 0, zahl);
    // kommt nicht die Fassung zurueck, die das Verschieben erzeugt hat, beginnt es neu
    if (m_schiebeGilt && m_muster.zeilen != m_schiebeErgebnis.zeilen) m_schiebeGilt = false;
    if (m_spur >= static_cast<int>(m_muster.spuren.size())) m_spur = static_cast<int>(m_muster.spuren.size()) - 1;
    if (m_spur < 0 && !m_muster.spuren.empty()) m_spur = 0;
    resize(sizeHint());
    updateGeometry();
    update();
}

void SchrittRaster::setSpur(int spur)
{
    if (spur < 0 || spur >= static_cast<int>(m_muster.spuren.size()) || spur == m_spur) return;
    m_spur = spur;
    update();
}

void SchrittRaster::setMarke(double schritt)
{
    if (schritt == m_marke) return;
    m_marke = schritt;
    update();
}

int SchrittRaster::schritte() const
{
    return m_muster.takte * m_schlaege * m_jeSchlag;
}

int SchrittRaster::xVonSchritt(int schritt) const
{
    return kRand + schritt * (kSchrittBreite + kSchrittLuft) + (schritt / m_jeSchlag) * kSchlagLuft;
}

int SchrittRaster::schrittBei(int x) const
{
    const int schlagBreite = m_jeSchlag * (kSchrittBreite + kSchrittLuft) + kSchlagLuft;
    if (x < kRand) return -1;
    const int schlag = (x - kRand) / schlagBreite;
    const int rest = (x - kRand) - schlag * schlagBreite;
    const int teil = rest / (kSchrittBreite + kSchrittLuft);
    if (teil >= m_jeSchlag || rest % (kSchrittBreite + kSchrittLuft) >= kSchrittBreite) return -1;  // eine Luecke
    const int schritt = schlag * m_jeSchlag + teil;
    return schritt < schritte() ? schritt : -1;
}

int SchrittRaster::lautOben() const
{
    return kKopfHoehe + static_cast<int>(m_muster.spuren.size()) * kZeilenHoehe + kLuftHoehe;
}

QSize SchrittRaster::sizeHint() const
{
    return {xVonSchritt(schritte()) + kRand, lautOben() + kLautHoehe};
}

void SchrittRaster::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));
    const QColor text = pal.color(QPalette::Text);
    QColor leise = text;
    leise.setAlpha(130);
    QColor zelle = text;
    QColor an = pal.color(QPalette::Highlight);
    const int zahl = schritte();
    const int jeTakt = m_schlaege * m_jeSchlag;
    const int spuren = static_cast<int>(m_muster.spuren.size());

    // --- der Kopf: die Takte
    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));
    p.setFont(klein);
    p.setPen(leise);
    for (int takt = 0; takt < m_muster.takte; ++takt)
        p.drawText(QRect(xVonSchritt(takt * jeTakt), 0, 80, kKopfHoehe), Qt::AlignLeft | Qt::AlignVCenter,
                   tr("Bar %1").arg(takt + 1));
    p.setFont(font());

    // --- die angeklickte Spur: ihre Zeile leicht hervorgehoben
    if (m_spur >= 0 && m_spur < spuren)
    {
        QColor zeile = an;
        zeile.setAlpha(28);
        p.fillRect(QRect(0, yVonSpur(m_spur), width(), kZeilenHoehe), zeile);
    }

    // --- die Schritte: jeder zweite Schlag heller; ein gesetzter Schritt so
    //     kraeftig, wie er laut ist
    p.setPen(Qt::NoPen);
    for (int s = 0; s < spuren; ++s)
    {
        const std::vector<double>& zeile = m_muster.zeilen[static_cast<std::size_t>(s)];
        const bool stumm = m_muster.spuren[static_cast<std::size_t>(s)].stumm;
        for (int i = 0; i < zahl && i < static_cast<int>(zeile.size()); ++i)
        {
            const QRect r(xVonSchritt(i), yVonSpur(s) + 4, kSchrittBreite, kZeilenHoehe - 8);
            const double wert = zeile[static_cast<std::size_t>(i)];
            if (wert > 0.0)
            {
                QColor farbe = stumm ? text : an;
                farbe.setAlphaF((stumm ? 0.2 : 0.35) + (stumm ? 0.25 : 0.65) * wert);
                p.setBrush(farbe);
            }
            else
            {
                zelle.setAlpha((i / m_jeSchlag) % 2 == 0 ? 58 : 24);
                p.setBrush(zelle);
            }
            p.drawRoundedRect(r, 2, 2);
        }
    }

    // --- die Marken: ein heller Rand um jeden markierten Schritt
    if (!m_marken.empty())
    {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(text, 1.6));
        for (const SchrittOrt& ort : m_marken)
            p.drawRoundedRect(QRect(xVonSchritt(ort.schritt), yVonSpur(ort.spur) + 4, kSchrittBreite, kZeilenHoehe - 8)
                                  .adjusted(-1, -1, 1, 1),
                              2, 2);
        p.setPen(Qt::NoPen);
    }

    // --- die Einfuegemarke: dort landet Ctrl+V
    if (spuren > 0)
    {
        const int x = xVonSchritt(m_einfuegen) - 2;
        QColor marke = an.lighter(150);
        p.setPen(QPen(marke, 1.4, Qt::DashLine));
        p.drawLine(x, 2, x, yVonSpur(spuren));
        p.setPen(Qt::NoPen);
        p.setBrush(marke);
        const QPoint spitze[3] = {QPoint(x - 4, 1), QPoint(x + 4, 1), QPoint(x, 7)};
        p.drawPolygon(spitze, 3);
    }

    // --- die Lautstaerke je Schritt der angeklickten Spur
    const int oben = lautOben();
    p.setPen(leise);
    p.drawLine(0, oben + kLautHoehe - 1, width(), oben + kLautHoehe - 1);
    p.setPen(Qt::NoPen);
    if (m_spur >= 0 && m_spur < spuren)
    {
        const std::vector<double>& zeile = m_muster.zeilen[static_cast<std::size_t>(m_spur)];
        for (int i = 0; i < zahl && i < static_cast<int>(zeile.size()); ++i)
        {
            const double wert = zeile[static_cast<std::size_t>(i)];
            if (wert <= 0.0) continue;
            const int hoch = std::max(2, static_cast<int>(wert * (kLautHoehe - 6) + 0.5));
            p.setBrush(an);
            p.drawRect(QRect(xVonSchritt(i), oben + kLautHoehe - 1 - hoch, kSchrittBreite, hoch));
        }
    }

    // --- die Abspielmarke
    if (m_marke >= 0.0 && m_marke < zahl)
    {
        const int ganz = static_cast<int>(m_marke);
        const int x = xVonSchritt(ganz) + static_cast<int>((m_marke - ganz) * (kSchrittBreite + kSchrittLuft));
        p.setPen(QPen(QColor(255, 90, 70), 2.0));
        p.drawLine(x, 0, x, height());
    }
}

void SchrittRaster::maleBei(QPoint ort)
{
    const int schritt = schrittBei(ort.x());
    if (schritt < 0 || m_zugSpur < 0 || m_zugSpur >= static_cast<int>(m_muster.zeilen.size())) return;
    std::vector<double>& zeile = m_muster.zeilen[static_cast<std::size_t>(m_zugSpur)];
    if (schritt >= static_cast<int>(zeile.size())) return;
    double& wert = zeile[static_cast<std::size_t>(schritt)];
    switch (m_zug)
    {
        case Zug::Setzen:
            if (wert <= 0.0) wert = 1.0;  // ein neuer Schritt beginnt mit voller Lautstaerke
            break;
        case Zug::Loeschen:
            wert = 0.0;
            break;
        case Zug::Laut:
        {
            const double hoehe = 1.0 - static_cast<double>(ort.y() - lautOben() - 3) / (kLautHoehe - 6);
            // ganz unten ist der Schritt aus
            wert = hoehe < 0.04 ? 0.0 : std::clamp(std::round(hoehe * 100.0) / 100.0, 0.0, 1.0);
            break;
        }
        case Zug::Rahmen:  // markiert nur, malt nicht
        case Zug::Keiner:
            return;
    }
    update();
}

int SchrittRaster::schrittNahe(int x) const
{
    const int zahl = schritte();
    if (zahl <= 0) return 0;
    for (int i = 0; i < zahl; ++i)
        if (x < xVonSchritt(i) + kSchrittBreite + kSchrittLuft) return i;
    return zahl - 1;
}

int SchrittRaster::spurNahe(int y) const
{
    const int spuren = static_cast<int>(m_muster.spuren.size());
    if (spuren == 0) return -1;
    return std::clamp((y - kKopfHoehe) / kZeilenHoehe, 0, spuren - 1);
}

bool SchrittRaster::istMarkiert(int spur, int schritt) const
{
    return std::find(m_marken.begin(), m_marken.end(), SchrittOrt{spur, schritt}) != m_marken.end();
}

void SchrittRaster::markiere(const std::vector<SchrittOrt>& orte)
{
    const int zahl = schritte();
    const int spuren = static_cast<int>(m_muster.spuren.size());
    std::vector<SchrittOrt> neu;
    for (const SchrittOrt& ort : orte)
        if (ort.spur >= 0 && ort.spur < spuren && ort.schritt >= 0 && ort.schritt < zahl &&
            std::find(neu.begin(), neu.end(), ort) == neu.end())
            neu.push_back(ort);
    if (neu == m_marken) return;
    m_marken = std::move(neu);
    update();
    Q_EMIT auswahlGeaendert();
}

void SchrittRaster::markiereUm(int spur, int schritt)
{
    std::vector<SchrittOrt> neu = m_marken;
    const auto platz = std::find(neu.begin(), neu.end(), SchrittOrt{spur, schritt});
    if (platz == neu.end())
        neu.push_back({spur, schritt});
    else
        neu.erase(platz);
    markiere(neu);
}

void SchrittRaster::markiereAlle()
{
    std::vector<SchrittOrt> alle;
    for (int spur = 0; spur < static_cast<int>(m_muster.spuren.size()); ++spur)
        for (int schritt = 0; schritt < schritte(); ++schritt) alle.push_back({spur, schritt});
    markiere(alle);
}

void SchrittRaster::setzeEinfuegemarke(int schritt)
{
    const int neu = std::clamp(schritt, 0, schritte());
    if (neu == m_einfuegen) return;
    m_einfuegen = neu;
    update();
    Q_EMIT auswahlGeaendert();
}

void SchrittRaster::rahmeBis(QPoint ort)
{
    const int spur = spurNahe(ort.y());
    if (spur < 0) return;
    const int schritt = schrittNahe(ort.x());
    std::vector<SchrittOrt> orte = m_vorRahmen;
    for (int s = std::min(spur, m_rahmenStart.spur); s <= std::max(spur, m_rahmenStart.spur); ++s)
        for (int i = std::min(schritt, m_rahmenStart.schritt); i <= std::max(schritt, m_rahmenStart.schritt); ++i)
            orte.push_back({s, i});
    markiere(orte);
}

void SchrittRaster::uebernimm(const Muster& neu, const std::vector<SchrittOrt>& marken, const QString& was)
{
    const bool anders = neu.zeilen != m_muster.zeilen;
    m_muster = neu;
    m_marken = marken;
    update();
    Q_EMIT auswahlGeaendert();
    if (anders) Q_EMIT geaendert(m_muster, was);
}

bool SchrittRaster::kopiere()
{
    const SchrittKopie kopie = kopiereSchritte(m_muster, m_marken);
    if (kopie.leer()) return false;
    m_kopie = kopie;
    m_kopieSpur = anfangVon(m_marken).spur;
    m_meldung.clear();
    Q_EMIT auswahlGeaendert();  // Paste ist jetzt moeglich
    return true;
}

bool SchrittRaster::schneideAus()
{
    if (!kopiere()) return false;
    return leereMarkierte();
}

bool SchrittRaster::leereMarkierte()
{
    Muster neu = m_muster;
    if (!leereSchritte(neu, m_marken)) return false;
    m_meldung.clear();
    m_schiebeGilt = false;
    uebernimm(neu, m_marken, tr("Clear steps"));
    return true;
}

bool SchrittRaster::fuegeEinBei(SchrittOrt ziel, bool ersetzen)
{
    if (m_kopie.leer()) return false;
    Muster neu = m_muster;
    std::vector<SchrittOrt> eingefuegt;
    const int ohnePlatz = fuegeSchritteEin(neu, m_kopie, ziel, ersetzen, &eingefuegt);
    // nichts still weglassen: sagen, was nicht gepasst hat
    m_meldung = ohnePlatz > 0 ? tr("%n step(s) did not fit into the pattern", nullptr, ohnePlatz) : QString();
    m_schiebeGilt = false;
    if (eingefuegt.empty())
    {
        Q_EMIT auswahlGeaendert();
        return false;
    }
    uebernimm(neu, eingefuegt, ersetzen ? tr("Paste steps") : tr("Paste steps, keep existing"));
    return true;
}

bool SchrittRaster::fuegeEin(bool ersetzen)
{
    if (!fuegeEinBei({m_kopieSpur, m_einfuegen}, ersetzen)) return false;
    // die Marke rueckt weiter: noch einmal Ctrl+V fuellt den naechsten Abschnitt
    setzeEinfuegemarke(m_einfuegen + m_kopie.schritte);
    return true;
}

bool SchrittRaster::dupliziere()
{
    const SchrittKopie kopie = kopiereSchritte(m_muster, m_marken);
    if (kopie.leer()) return false;
    const SchrittOrt anfang = anfangVon(m_marken);
    Muster neu = m_muster;
    std::vector<SchrittOrt> eingefuegt;
    const int ohnePlatz = fuegeSchritteEin(neu, kopie, {anfang.spur, anfang.schritt + kopie.schritte}, true, &eingefuegt);
    m_meldung = ohnePlatz > 0 ? tr("%n step(s) did not fit into the pattern", nullptr, ohnePlatz) : QString();
    m_schiebeGilt = false;
    if (eingefuegt.empty())
    {
        Q_EMIT auswahlGeaendert();
        return false;
    }
    uebernimm(neu, eingefuegt, tr("Duplicate steps"));
    return true;
}

bool SchrittRaster::bewege(int spurVersatz, int schrittVersatz, bool imKreis)
{
    if (m_marken.empty()) return false;
    // eine Folge von Tasten zaehlt vom Stand beim ersten Druck: so bleibt
    // stehen, was die Auswahl unterwegs ueberfaehrt
    if (!m_schiebeGilt || m_schiebeImKreis != imKreis || m_marken != m_schiebeMarken ||
        m_muster.zeilen != m_schiebeErgebnis.zeilen)
    {
        m_schiebeBasis = m_muster;
        m_schiebeOrte = m_marken;
        m_schiebeSpur = 0;
        m_schiebeSchritt = 0;
        m_schiebeImKreis = imKreis;
    }
    const int spurGanz = m_schiebeSpur + spurVersatz;
    const int schrittGanz = imKreis ? (m_schiebeSchritt + schrittVersatz) % std::max(1, schritte())
                                    : m_schiebeSchritt + schrittVersatz;
    Muster neu = m_schiebeBasis;
    std::vector<SchrittOrt> orte = m_schiebeOrte;
    if (spurGanz != 0 || schrittGanz != 0)
    {
        const bool ging = imKreis ? dreheSchritte(neu, orte, schrittGanz, schritte())
                                  : versetzeSchritte(neu, orte, spurGanz, schrittGanz);
        if (!ging) return false;  // am Rand: nichts bewegt sich (V3 a)
    }
    m_schiebeSpur = spurGanz;
    m_schiebeSchritt = schrittGanz;
    m_schiebeErgebnis = neu;
    m_schiebeMarken = orte;
    m_schiebeGilt = true;
    m_meldung.clear();
    uebernimm(neu, orte, imKreis ? tr("Rotate steps") : tr("Move steps"));
    return true;
}

bool SchrittRaster::versetze(int spurVersatz, int schrittVersatz)
{
    return bewege(spurVersatz, schrittVersatz, false);
}

bool SchrittRaster::drehe(int schrittVersatz)
{
    return bewege(0, schrittVersatz, true);
}

bool SchrittRaster::leereSchritt(SchrittOrt ort)
{
    Muster neu = m_muster;
    if (!leereSchritte(neu, {ort})) return false;
    m_meldung.clear();
    m_schiebeGilt = false;
    uebernimm(neu, m_marken, tr("Clear step"));
    return true;
}

void SchrittRaster::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        // Mit dem Haken „Right-click deletes" schaltet der Rechtsklick einen
        // gesetzten Schritt sofort aus. Sonst — und auf einem von mehreren
        // markierten oder einem leeren Schritt — oeffnet sich das Menue.
        m_rechtsGeloescht = false;
        setFocus(Qt::MouseFocusReason);
        const QPoint wo = event->position().toPoint();
        if (!m_rechtsLoescht || wo.y() < kKopfHoehe || wo.y() >= lautOben()) return;
        const int spur = (wo.y() - kKopfHoehe) / kZeilenHoehe;
        const int schritt = schrittBei(wo.x());
        if (spur >= static_cast<int>(m_muster.spuren.size()) || schritt < 0) return;
        if (m_marken.size() > 1 && istMarkiert(spur, schritt)) return;
        m_rechtsGeloescht = leereSchritt({spur, schritt});
        return;
    }
    if (event->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    const QPoint ort = event->position().toPoint();
    const int spuren = static_cast<int>(m_muster.spuren.size());
    const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    if (ort.y() >= lautOben())
    {
        if (m_spur < 0 || m_spur >= spuren) return;
        m_zug = Zug::Laut;
        m_zugSpur = m_spur;
        m_vorZug = m_muster;
        maleBei(ort);
        return;
    }
    if (ort.y() < kKopfHoehe)
    {
        // die Zeile der Takte: hier steht die Einfuegemarke
        const int zahl = schritte();
        setzeEinfuegemarke(zahl > 0 && ort.x() >= xVonSchritt(zahl - 1) + kSchrittBreite ? zahl : schrittNahe(ort.x()));
        return;
    }
    const int spur = (ort.y() - kKopfHoehe) / kZeilenHoehe;
    if (spur >= spuren)
    {
        markiere({});  // ein Klick ins Leere hebt die Marken auf
        return;
    }
    if (shift)
    {
        // ein Rahmen: der ganze Bereich ist markiert, auch seine leeren Schritte
        m_zug = Zug::Rahmen;
        m_rahmenStart = {spur, schrittNahe(ort.x())};
        m_vorRahmen = ctrl ? m_marken : std::vector<SchrittOrt>{};
        rahmeBis(ort);
        return;
    }
    if (ctrl)
    {
        if (const int schritt = schrittBei(ort.x()); schritt >= 0) markiereUm(spur, schritt);
        return;
    }
    if (spur != m_spur)
    {
        m_spur = spur;
        update();
        Q_EMIT spurGewaehlt(spur);
    }
    const int schritt = schrittBei(ort.x());
    if (schritt < 0) return;
    const std::vector<double>& zeile = m_muster.zeilen[static_cast<std::size_t>(spur)];
    if (schritt >= static_cast<int>(zeile.size())) return;
    m_zug = zeile[static_cast<std::size_t>(schritt)] > 0.0 ? Zug::Loeschen : Zug::Setzen;
    m_zugSpur = spur;
    m_vorZug = m_muster;
    maleBei(ort);
}

void SchrittRaster::mouseMoveEvent(QMouseEvent* event)
{
    if (m_zug == Zug::Keiner || !(event->buttons() & Qt::LeftButton)) return;
    if (m_zug == Zug::Rahmen)
        rahmeBis(event->position().toPoint());
    else
        maleBei(event->position().toPoint());
}

void SchrittRaster::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || m_zug == Zug::Keiner) return;
    const Zug zug = m_zug;
    m_zug = Zug::Keiner;
    if (zug == Zug::Rahmen) return;  // markieren aendert das Muster nicht
    if (m_muster == m_vorZug) return;
    m_schiebeGilt = false;
    Q_EMIT geaendert(m_muster, zug == Zug::Laut ? tr("Change step volume") : tr("Edit steps"));
}

void SchrittRaster::keyPressEvent(QKeyEvent* event)
{
    const Qt::KeyboardModifiers tasten = event->modifiers() & ~Qt::KeypadModifier;
    const bool nurCtrl = tasten == Qt::ControlModifier;
    switch (event->key())
    {
        case Qt::Key_C:
            if (nurCtrl && kopiere()) return;
            break;
        case Qt::Key_X:
            if (nurCtrl && schneideAus()) return;
            break;
        case Qt::Key_V:
            // mit Shift dazu: nur dazulegen, was am Ziel gesetzt ist, bleibt
            if (nurCtrl || tasten == (Qt::ControlModifier | Qt::ShiftModifier))
            {
                (void)fuegeEin(nurCtrl);
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
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            if (!m_marken.empty())
            {
                (void)leereMarkierte();
                return;
            }
            break;
        case Qt::Key_Escape:
            if (!m_marken.empty())
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
                (void)versetze(0, richtung);
            else
                break;
            return;
        }
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (m_marken.empty() || tasten != Qt::NoModifier) break;
            (void)versetze(event->key() == Qt::Key_Up ? -1 : 1, 0);
            return;
        default:
            break;
    }
    QWidget::keyPressEvent(event);
}

void SchrittRaster::contextMenuEvent(QContextMenuEvent* event)
{
    // der Rechtsklick hat gerade einen Schritt geleert: kein Menue dazu
    if (m_rechtsGeloescht)
    {
        m_rechtsGeloescht = false;
        return;
    }
    const int spur = spurNahe(event->pos().y());
    if (spur < 0) return;
    const SchrittOrt hier{spur, schrittNahe(event->pos().x())};
    const bool etwas = !m_marken.empty();
    const bool gesetzt = m_muster.zeilen[static_cast<std::size_t>(hier.spur)][static_cast<std::size_t>(hier.schritt)] > 0.0;
    QMenu menu(this);
    menu.addAction(tr("Copy"), QKeySequence(Qt::CTRL | Qt::Key_C), this, [this] { (void)kopiere(); })->setEnabled(etwas);
    menu.addAction(tr("Cut"), QKeySequence(Qt::CTRL | Qt::Key_X), this, [this] { (void)schneideAus(); })
        ->setEnabled(etwas);
    menu.addAction(tr("Paste here"), this, [this, hier] { (void)fuegeEinBei(hier, true); })->setEnabled(hatKopie());
    menu.addAction(tr("Paste and keep existing steps"), this, [this, hier] { (void)fuegeEinBei(hier, false); })
        ->setEnabled(hatKopie());
    menu.addAction(tr("Duplicate"), QKeySequence(Qt::CTRL | Qt::Key_D), this, [this] { (void)dupliziere(); })
        ->setEnabled(etwas);
    // loeschen: die markierten — ohne Marken der Schritt unter der Maus
    if (etwas)
        menu.addAction(tr("Clear steps"), QKeySequence(Qt::Key_Delete), this, [this] { (void)leereMarkierte(); });
    else
        menu.addAction(tr("Clear this step"), this, [this, hier] { (void)leereSchritt(hier); })->setEnabled(gesetzt);
    menu.addSeparator();
    menu.addAction(tr("Select all"), QKeySequence(Qt::CTRL | Qt::Key_A), this, [this] { markiereAlle(); });
    menu.exec(event->globalPos());
}

// =============================================================================
// DrumEditor
// =============================================================================

DrumEditor::DrumEditor(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopDrumEditor"));
    setupUI();
}

void DrumEditor::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    auto* kopf = new QHBoxLayout;
    m_basisName = new QLabel(this);
    m_basisName->setObjectName(QStringLiteral("drumBase"));
    m_nutzer = new QLabel(this);
    m_nutzer->setObjectName(QStringLiteral("drumUsers"));
    m_nutzer->setEnabled(false);  // grau: eine Auskunft
    m_takte = new QSpinBox(this);
    m_takte->setObjectName(QStringLiteral("drumBars"));
    m_takte->setRange(kTakteMin, kTakteMax);
    m_takte->setKeyboardTracking(false);
    m_takte->setToolTip(tr("Length of the pattern in bars; its clips grow and shrink with it"));
    m_play = new QPushButton(tr("Play pattern"), this);
    m_play->setObjectName(QStringLiteral("drumPlay"));
    m_play->setToolTip(tr("Play only this pattern, in a loop"));
    m_spitze = new QLabel(QStringLiteral("–"), this);
    m_spitze->setObjectName(QStringLiteral("drumPeak"));
    m_spitze->setMinimumWidth(36);
    m_spitze->setToolTip(tr("Peak of the mixed loop while it plays; above 1 it clips"));
    kopf->addWidget(new QLabel(tr("Base"), this));
    kopf->addWidget(m_basisName);
    kopf->addWidget(m_nutzer);
    kopf->addStretch(1);
    kopf->addWidget(new QLabel(tr("Bars"), this));
    kopf->addWidget(m_takte);
    kopf->addWidget(m_play);
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);

    auto* raster = new QHBoxLayout;
    m_schlaege = new QSpinBox(this);
    m_schlaege->setObjectName(QStringLiteral("drumBeats"));
    m_schlaege->setRange(1, kRasterMax);
    m_schlaege->setKeyboardTracking(false);
    m_jeSchlag = new QSpinBox(this);
    m_jeSchlag->setObjectName(QStringLiteral("drumStepsPerBeat"));
    m_jeSchlag->setRange(1, kRasterMax);
    m_jeSchlag->setKeyboardTracking(false);
    m_swing = new QDoubleSpinBox(this);
    m_swing->setObjectName(QStringLiteral("drumSwing"));
    m_swing->setRange(0.0, 100.0);
    m_swing->setDecimals(0);
    m_swing->setSuffix(QStringLiteral(" %"));
    m_swing->setKeyboardTracking(false);
    m_swing->setToolTip(tr("Delays every second step of a beat; at 100 % by half a step"));
    raster->addWidget(new QLabel(tr("Grid of the whole loop:"), this));
    raster->addWidget(new QLabel(tr("Beats per bar"), this));
    raster->addWidget(m_schlaege);
    raster->addWidget(new QLabel(tr("Steps per beat"), this));
    raster->addWidget(m_jeSchlag);
    raster->addWidget(new QLabel(tr("Swing"), this));
    raster->addWidget(m_swing);
    raster->addStretch(1);
    aussen->addLayout(raster);

    // --- links die Kopfzeilen der Spuren, rechts das Raster; es rollt nur das Raster
    auto* koerper = new QHBoxLayout;
    koerper->setSpacing(0);
    auto* spalte = new QWidget(this);
    spalte->setFixedWidth(kSpaltenBreite);
    m_spurSpalte = new QVBoxLayout(spalte);
    m_spurSpalte->setContentsMargins(0, 0, 0, 0);
    m_spurSpalte->setSpacing(0);
    auto* titel = new QWidget(spalte);
    titel->setFixedHeight(SchrittRaster::kKopfHoehe);
    auto* titelReihe = new QHBoxLayout(titel);
    titelReihe->setContentsMargins(26, 0, 0, 0);
    titelReihe->setSpacing(6);
    const auto titelFeld = [&](const QString& text, int breite) {
        auto* feld = new QLabel(text, titel);
        feld->setEnabled(false);
        feld->setFixedWidth(breite);
        titelReihe->addWidget(feld);
    };
    titelFeld(tr("Track"), 70);
    titelFeld(tr("Sound"), kKlangBreite);
    titelFeld(QString(), kMehrBreite);  // ueber dem Knopf „…"
    titelFeld(tr("Vol"), 48);
    titelFeld(tr("Pan"), 48);
    titelReihe->addStretch(1);
    m_spurSpalte->addWidget(titel);
    // (die Zeilen der Spuren haengt `baueSpuren` hier ein)
    auto* neuReihe = new QWidget(spalte);
    neuReihe->setFixedHeight(SchrittRaster::kLuftHoehe);
    auto* neuLayout = new QHBoxLayout(neuReihe);
    neuLayout->setContentsMargins(0, 2, 0, 2);
    m_spurNeu = new QPushButton(tr("Add track"), neuReihe);
    m_spurNeu->setObjectName(QStringLiteral("drumAddTrack"));
    neuLayout->addWidget(m_spurNeu);
    neuLayout->addStretch(1);
    m_spurSpalte->addWidget(neuReihe);
    m_lautName = new QLabel(spalte);
    m_lautName->setObjectName(QStringLiteral("drumStepVolume"));
    m_lautName->setFixedHeight(SchrittRaster::kLautHoehe);
    m_lautName->setEnabled(false);
    m_spurSpalte->addWidget(m_lautName);
    m_spurSpalte->addStretch(1);

    m_raster = new SchrittRaster;
    m_rollen = new QScrollArea(this);
    m_rollen->setFrameShape(QFrame::NoFrame);
    m_rollen->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_rollen->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);  // lange Muster rollen, sie stauchen nicht
    m_rollen->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_rollen->setWidget(m_raster);
    koerper->addWidget(spalte, 0, Qt::AlignTop);
    koerper->addWidget(m_rollen, 1, Qt::AlignTop);
    aussen->addLayout(koerper);
    // ein Muster ohne Spuren: sagen, wie es weitergeht
    m_leerHinweis = new QLabel(tr("This pattern has no tracks yet — Add track creates one."), this);
    m_leerHinweis->setObjectName(QStringLiteral("drumEmptyHint"));
    m_leerHinweis->setEnabled(false);
    m_leerHinweis->hide();
    aussen->addWidget(m_leerHinweis);
    // wie man mehrere Schritte markiert — oder was markiert ist und was die Tasten tun
    m_auswahlZeile = new QLabel(this);
    m_auswahlZeile->setObjectName(QStringLiteral("drumSelection"));
    m_auswahlZeile->setEnabled(false);
    m_auswahlZeile->setWordWrap(true);
    // „Right-click deletes" (Wunsch Patrik S113): an = der Rechtsklick leert den
    // Schritt sofort; aus = er oeffnet das Menue. Die App merkt die Stellung.
    m_rechtsLoescht = new QCheckBox(tr("Right-click deletes"), this);
    m_rechtsLoescht->setObjectName(QStringLiteral("drumRightClickDeletes"));
    m_rechtsLoescht->setToolTip(tr("On: a right-click turns the step under the mouse off at once. Off: it opens the "
                                   "menu, which has an entry for that"));
    m_rechtsLoescht->setChecked(QSettings().value(QLatin1String(kRechtsLoescht), false).toBool());
    auto* auswahlReihe = new QHBoxLayout;
    auswahlReihe->addWidget(m_auswahlZeile, 1);
    auswahlReihe->addWidget(m_rechtsLoescht, 0, Qt::AlignTop);
    aussen->addLayout(auswahlReihe);
    // was das Fenster hoeher ist als sein Inhalt, bleibt unten frei — sonst
    // verteilt das Layout die Luft zwischen Kopf, Raster-Zeile und Spuren
    aussen->addStretch(1);

    connect(m_takte, &QSpinBox::valueChanged, this, [this](int takte) { setzeTakte(takte); });
    connect(m_schlaege, &QSpinBox::valueChanged, this, [this](int schlaege) { setzeRaster(schlaege, m_jeSchlag->value()); });
    connect(m_jeSchlag, &QSpinBox::valueChanged, this, [this](int jeSchlag) { setzeRaster(m_schlaege->value(), jeSchlag); });
    connect(m_swing, &QDoubleSpinBox::valueChanged, this, [this](double prozent) { setzeSwing(prozent / 100.0); });
    connect(m_play, &QPushButton::clicked, this, [this] { Q_EMIT spielenVerlangt(!m_spielt); });
    connect(m_spurNeu, &QPushButton::clicked, this, [this] { spurHinzu(); });
    connect(m_raster, &SchrittRaster::spurGewaehlt, this, [this](int spur) { waehleSpur(spur); });
    connect(m_raster, &SchrittRaster::auswahlGeaendert, this, &DrumEditor::zeigeAuswahl);
    m_raster->setRechtsklickLoescht(m_rechtsLoescht->isChecked());
    connect(m_rechtsLoescht, &QCheckBox::toggled, this, [this](bool an) {
        m_raster->setRechtsklickLoescht(an);
        QSettings().setValue(QLatin1String(kRechtsLoescht), an);
    });
    connect(m_raster, &SchrittRaster::geaendert, this, [this](const Muster& neu, const QString& was) {
        Loop fassung = m_loop;
        Muster* ziel = musterIn(fassung, m_muster);
        if (ziel == nullptr) return;
        ziel->zeilen = neu.zeilen;
        Q_EMIT bearbeitet(fassung, was);
    });
    resize(874, 320);
}

void DrumEditor::setKitKlaenge(const QStringList& klaenge)
{
    m_kitKlaenge = klaenge;
    if (basis() != nullptr) zeigeAlles();
}

bool DrumEditor::zeige(const Loop& loop, const QString& muster)
{
    const Muster* ziel = loop.findeMuster(muster);
    if (ziel == nullptr || ziel->art != MusterArt::Drumkit) return false;
    // eine andere Basis: die Fenster daneben galten einer Spur der alten
    if (muster != m_muster) m_paletteSpur = m_creatorSpur = m_aufnahmeSpur = m_detailSpur = -1;
    m_loop = loop;
    m_muster = muster;
    zeigeAlles();
    return true;
}

void DrumEditor::setLoop(const Loop& loop)
{
    if (m_muster.isEmpty()) return;
    // umbenannt? Dann gibt es im neuen Loop eine Basis mit neuem Namen und demselben Inhalt
    const QString folge = folgeBasis(m_loop, m_muster, loop);
    {
        // eine Spur weniger: die Nummern dahinter sind verrutscht — die Fenster daneben
        // (Instruments, Creator, Record sample, das Feld hinter „…") gelten keiner Spur mehr
        const Muster* vorher = basis();
        const Muster* danach = folge.isEmpty() ? nullptr : loop.findeMuster(folge);
        if (vorher == nullptr || danach == nullptr || danach->spuren.size() < vorher->spuren.size())
            m_paletteSpur = m_creatorSpur = m_aufnahmeSpur = m_detailSpur = -1;
    }
    m_loop = loop;
    if (folge.isEmpty())
    {
        // die Basis gibt es nicht mehr (ihr letzter Clip ist geloescht, ein anderer Loop geladen)
        m_muster.clear();
        close();
        return;
    }
    m_muster = folge;
    zeigeAlles();
}

void DrumEditor::zeigeAlles()
{
    const Muster* ziel = basis();
    if (ziel == nullptr) return;
    setWindowTitle(tr("Drum pattern: %1").arg(m_muster));
    m_basisName->setText(m_muster);
    m_nutzer->setText(tr("· used by %1 clips").arg(basisNutzer(m_loop, m_muster)));
    {
        const QSignalBlocker a(m_takte);
        const QSignalBlocker b(m_schlaege);
        const QSignalBlocker c(m_jeSchlag);
        const QSignalBlocker d(m_swing);
        m_takte->setValue(ziel->takte);
        m_schlaege->setValue(m_loop.schlaegeJeTakt);
        m_jeSchlag->setValue(m_loop.schritteJeSchlag);
        m_swing->setValue(m_loop.swing * 100.0);
    }
    m_raster->setMuster(*ziel, m_loop.schlaegeJeTakt, m_loop.schritteJeSchlag);
    // das Raster in voller Hoehe, darunter Platz fuer den Rollbalken
    m_rollen->setFixedHeight(m_raster->sizeHint().height() + style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2);
    baueSpuren(*ziel);
    m_leerHinweis->setVisible(ziel->spuren.empty());
    waehleSpur(m_raster->spur());
    zeigeAuswahl();
    zeigeDetail();
    // das Fenster Instruments waehlt fuer eine Spur dieser Basis — oder fuer keine mehr
    if (m_palette != nullptr)
    {
        const Spur* spur = spurBei(m_paletteSpur);
        if (spur == nullptr) m_paletteSpur = -1;
        m_palette->setzeZiel(spur != nullptr ? m_muster : QString(), spur != nullptr ? spur->name : QString());
    }
}

const Spur* DrumEditor::spurBei(int spur) const
{
    const Muster* ziel = basis();
    return ziel != nullptr && spur >= 0 && spur < static_cast<int>(ziel->spuren.size())
               ? &ziel->spuren[static_cast<std::size_t>(spur)]
               : nullptr;
}

void DrumEditor::zeigeAuswahl()
{
    const std::vector<SchrittOrt>& marken = m_raster->markierte();
    const Muster* ziel = basis();
    m_auswahlZeile->setVisible(ziel != nullptr && !ziel->spuren.empty());
    QString text;
    if (marken.empty())
        // sonst findet es niemand: sagen, wie man markiert
        text = tr("Shift+drag selects steps · Ctrl+click adds or removes one · a click in the bar row sets the paste "
                  "marker");
    else
    {
        std::vector<int> spuren;
        for (const SchrittOrt& ort : marken)
            if (std::find(spuren.begin(), spuren.end(), ort.spur) == spuren.end()) spuren.push_back(ort.spur);
        text = tr("%1 steps in %2 tracks selected · Ctrl+C copy · Ctrl+X cut · Ctrl+V paste at the marker · Ctrl+D "
                  "duplicate · arrow keys move · Alt+Left / Right rotate · Del clears")
                   .arg(marken.size())
                   .arg(spuren.size());
    }
    if (!m_raster->meldung().isEmpty()) text = m_raster->meldung() + QStringLiteral(" · ") + text;
    m_auswahlZeile->setText(text);
}

void DrumEditor::fuelleKlang(QComboBox* auswahl, const Spur& spur, const QStringList& eigene,
                             const QStringList& bibliothek) const
{
    const QSignalBlocker ruhe(auswahl);
    auswahl->clear();
    const auto kopf = [auswahl](const QString& text) {
        auswahl->addItem(text, kKopf);
        // eine Ueberschrift: grau, nicht zu waehlen
        if (auto* modell = qobject_cast<QStandardItemModel*>(auswahl->model()))
            if (QStandardItem* eintrag = modell->item(auswahl->count() - 1)) eintrag->setFlags(Qt::NoItemFlags);
    };
    const QString& klang = spur.klang;
    const bool instrument = !spur.instrument.isEmpty();
    // ganz oben, was die Spur spielt, wenn es kein Klang der Listen darunter ist
    if (instrument)
        auswahl->addItem(InstrumentEditor::instrumentAnzeige(spur.instrument), kInstrumentJetzt);
    else if (klang.isEmpty())
        auswahl->addItem(tr("(none)"), QString());
    kopf(tr("Kit"));
    for (const QString& kit : m_kitKlaenge) auswahl->addItem(kit.mid(4), kit);  // ohne den Vorsatz „kit:"
    if (istKitKlang(klang) && !m_kitKlaenge.contains(klang)) auswahl->addItem(klang.mid(4), klang);
    // die mitgelieferten Samples (`lib:`, Entscheid 1 a, S123)
    const bool mitgeliefertFehlt = istBibliotheksKlang(klang) && !bibliothek.contains(klang);
    if (!bibliothek.isEmpty() || mitgeliefertFehlt)
    {
        kopf(tr("Samples"));
        for (const QString& nennung : bibliothek) auswahl->addItem(sampleTitel(nennung), nennung);
        if (mitgeliefertFehlt) auswahl->addItem(tr("%1 (not found)").arg(sampleTitel(klang)), klang);
    }
    const bool eigenerFehlt = istEigenerKlang(klang) && !eigene.contains(klang);
    if (!eigene.isEmpty() || eigenerFehlt)
    {
        kopf(tr("My samples"));
        for (const QString& nennung : eigene) auswahl->addItem(sampleTitel(nennung), nennung);
        if (eigenerFehlt) auswahl->addItem(tr("%1 (not found)").arg(sampleTitel(klang)), klang);
    }
    if (!klang.isEmpty() && !istMitgelieferterKlang(klang) && !istEigenerKlang(klang))
    {
        // eine Tondatei an freiem Ort
        kopf(tr("File"));
        auswahl->addItem(QFileInfo(klang).fileName(), klang);
    }
    auswahl->insertSeparator(auswahl->count());
    auswahl->addItem(tr("Instrument..."), kInstrumentWahl);
    auswahl->addItem(tr("Sample file..."), kDateiWahl);
    auswahl->addItem(tr("Record sample..."), kAufnahmeWahl);
    auswahl->setCurrentIndex(instrument ? 0 : auswahl->findData(klang));
    if (instrument)
        auswahl->setToolTip(tr("Plays the instrument %1 — note %2, held %3 ms. The button … next to it changes that")
                                .arg(InstrumentEditor::instrumentAnzeige(spur.instrument), tonName(spur.hoehe))
                                .arg(spur.dauerMs));
    else if (klang.isEmpty())
        auswahl->setToolTip(QString());
    else
        // sonst findet es niemand: sagen, wo Edit sound... steht
        auswahl->setToolTip(tr("%1\nRight-click the track: Edit sound... makes an instrument of it")
                                .arg(istMitgelieferterKlang(klang) || istEigenerKlang(klang) ? klang : QDir::toNativeSeparators(klang)));
}

QStringList DrumEditor::klangZeilen(int spur) const
{
    QStringList zeilen;
    const Spur* ziel = spurBei(spur);
    if (ziel == nullptr || spur >= static_cast<int>(m_spuren.size())) return zeilen;
    const QComboBox* auswahl = m_spuren[static_cast<std::size_t>(spur)].klang;
    for (int i = 0; i < auswahl->count(); ++i)
    {
        if (auswahl->itemData(i, Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator")) continue;
        const QString wert = auswahl->itemData(i).toString();
        if (wert == kKopf)
            zeilen << QStringLiteral("[%1]").arg(auswahl->itemText(i));
        else if (wert == kInstrumentJetzt)
            zeilen << QStringLiteral("instrument:") + ziel->instrument;
        else if (wert == kInstrumentWahl || wert == kDateiWahl || wert == kAufnahmeWahl || wert.isEmpty())
            zeilen << auswahl->itemText(i);
        else
            zeilen << wert;
    }
    return zeilen;
}

void DrumEditor::baueSpuren(const Muster& muster)
{
    if (m_spuren.size() != muster.spuren.size())
    {
        for (SpurZeile& zeile : m_spuren) delete zeile.reihe;
        m_spuren.clear();
        for (int s = 0; s < static_cast<int>(muster.spuren.size()); ++s)
        {
            SpurZeile zeile;
            zeile.reihe = new QWidget;
            zeile.reihe->setFixedHeight(SchrittRaster::kZeilenHoehe);
            zeile.reihe->setProperty(kSpurNr, s);
            zeile.reihe->setContextMenuPolicy(Qt::CustomContextMenu);
            zeile.reihe->installEventFilter(this);
            auto* reihe = new QHBoxLayout(zeile.reihe);
            reihe->setContentsMargins(0, 0, 0, 0);
            reihe->setSpacing(6);
            zeile.stumm = new QToolButton(zeile.reihe);
            zeile.stumm->setText(QStringLiteral("M"));
            zeile.stumm->setCheckable(true);
            zeile.stumm->setFixedSize(20, 20);
            zeile.stumm->setToolTip(tr("Mute this track"));
            zeile.name = new QLabel(zeile.reihe);
            zeile.name->setFixedWidth(70);
            zeile.name->setProperty(kSpurNr, s);
            zeile.name->installEventFilter(this);
            zeile.klang = new QComboBox(zeile.reihe);
            zeile.klang->setObjectName(QStringLiteral("drumTrackSound%1").arg(s));
            zeile.klang->setFixedWidth(kKlangBreite);
            zeile.klang->view()->setMinimumWidth(190);  // die Liste darf breiter sein als das Feld
            // „…" (Stufe 11c): nur an einer Spur, die ein Instrument spielt — sein Platz bleibt
            // auch sonst frei, damit die Regler dahinter in einer Flucht stehen
            zeile.mehr = new QToolButton(zeile.reihe);
            zeile.mehr->setObjectName(QStringLiteral("drumTrackMore%1").arg(s));
            zeile.mehr->setText(QString::fromUtf8("…"));
            zeile.mehr->setFixedSize(kMehrBreite, 20);
            zeile.mehr->setToolTip(tr("Note and length of this track, play it, choose or edit its instrument"));
            {
                QSizePolicy platz = zeile.mehr->sizePolicy();
                platz.setRetainSizeWhenHidden(true);
                zeile.mehr->setSizePolicy(platz);
            }
            zeile.laut = new QSlider(Qt::Horizontal, zeile.reihe);
            zeile.laut->setRange(0, 100);
            zeile.laut->setFixedWidth(48);
            zeile.laut->setToolTip(tr("Volume of the track"));
            zeile.pan = new QSlider(Qt::Horizontal, zeile.reihe);
            zeile.pan->setRange(-100, 100);
            zeile.pan->setFixedWidth(48);
            zeile.pan->setToolTip(tr("Pan: left to right"));
            reihe->addWidget(zeile.stumm);
            reihe->addWidget(zeile.name);
            reihe->addWidget(zeile.klang);
            reihe->addWidget(zeile.mehr);
            reihe->addWidget(zeile.laut);
            reihe->addWidget(zeile.pan);
            reihe->addStretch(1);
            m_spurSpalte->insertWidget(1 + s, zeile.reihe);  // hinter der Titelzeile

            connect(zeile.reihe, &QWidget::customContextMenuRequested, this,
                    [this, s, reiheWidget = zeile.reihe](QPoint ort) { spurMenue(s, reiheWidget->mapToGlobal(ort)); });
            connect(zeile.stumm, &QToolButton::toggled, this, [this, s](bool stumm) { setzeStumm(s, stumm); });
            connect(zeile.klang, &QComboBox::activated, this, [this, s, auswahl = zeile.klang](int index) {
                klangGewaehlt(s, auswahl->itemData(index).toString());
            });
            connect(zeile.mehr, &QToolButton::clicked, this, [this, s] { (void)oeffneDetail(s); });
            // ein Schieber meldet beim Loslassen — nicht jeden Pixel als Schritt fuer Undo
            const auto schieber = [this, s](QSlider* regler, bool pan) {
                const auto melde = [this, s, regler, pan] {
                    if (pan)
                        setzePan(s, regler->value() / 100.0);
                    else
                        setzeLaut(s, regler->value() / 100.0);
                };
                connect(regler, &QSlider::sliderReleased, this, melde);
                connect(regler, &QSlider::valueChanged, this, [regler, melde] {
                    if (!regler->isSliderDown()) melde();
                });
            };
            schieber(zeile.laut, false);
            schieber(zeile.pan, true);
            m_spuren.push_back(zeile);
        }
    }
    const QStringList eigene = eigeneSamples();  // einmal gelesen, fuer alle Spuren
    const QStringList bibliothek = bibliotheksKlaenge();
    for (std::size_t s = 0; s < m_spuren.size(); ++s)
    {
        const Spur& spur = muster.spuren[s];
        SpurZeile& zeile = m_spuren[s];
        const QSignalBlocker a(zeile.stumm);
        const QSignalBlocker b(zeile.laut);
        const QSignalBlocker c(zeile.pan);
        zeile.stumm->setChecked(spur.stumm);
        zeile.name->setText(zeile.name->fontMetrics().elidedText(spur.name, Qt::ElideRight, 70));
        zeile.name->setToolTip(spur.name);
        fuelleKlang(zeile.klang, spur, eigene, bibliothek);
        zeile.mehr->setVisible(!spur.instrument.isEmpty());
        if (!zeile.laut->isSliderDown()) zeile.laut->setValue(static_cast<int>(std::lround(spur.laut * 100.0)));
        if (!zeile.pan->isSliderDown()) zeile.pan->setValue(static_cast<int>(std::lround(spur.pan * 100.0)));
    }
}

void DrumEditor::waehleSpur(int spur)
{
    m_raster->setSpur(spur);
    const Muster* ziel = basis();
    const int gewaehlt = m_raster->spur();
    for (std::size_t s = 0; s < m_spuren.size(); ++s)
    {
        QFont schrift = m_spuren[s].name->font();
        schrift.setBold(static_cast<int>(s) == gewaehlt);
        m_spuren[s].name->setFont(schrift);
    }
    m_lautName->setText(ziel != nullptr && gewaehlt >= 0 && gewaehlt < static_cast<int>(ziel->spuren.size())
                            ? tr("Step volume: %1").arg(ziel->spuren[static_cast<std::size_t>(gewaehlt)].name)
                            : QString());
}

bool DrumEditor::eventFilter(QObject* objekt, QEvent* ereignis)
{
    if (ereignis->type() == QEvent::MouseButtonPress && objekt->property(kSpurNr).isValid())
        waehleSpur(objekt->property(kSpurNr).toInt());
    return QWidget::eventFilter(objekt, ereignis);
}

void DrumEditor::spurMenue(int spur, QPoint ort)
{
    const Muster* ziel = basis();
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    waehleSpur(spur);
    QMenu menu(this);
    menu.setToolTipsVisible(true);
    {
        // Edit sound... (K1 a): aus dem, was die Spur spielt, wird ein Instrument
        const Spur& gewaehlte = ziel->spuren[static_cast<std::size_t>(spur)];
        QAction* klang = menu.addAction(tr("Edit sound..."), this, [this, spur] { (void)bearbeiteKlang(spur); });
        klang->setObjectName(QStringLiteral("drumTrackEditSound"));
        klang->setEnabled(!gewaehlte.spielt().isEmpty());
        klang->setToolTip(gewaehlte.instrument.isEmpty()
                              ? tr("Opens what this track plays as an instrument in the creator. Nothing changes until "
                                   "you save there — then the track plays your instrument")
                              : tr("Opens the instrument of this track in the creator"));
        menu.addSeparator();
    }
    menu.addAction(tr("Rename track..."), this, [this, spur, name = ziel->spuren[static_cast<std::size_t>(spur)].name] {
        bool ok = false;
        const QString neu = QInputDialog::getText(this, tr("Rename track"), tr("Name:"), QLineEdit::Normal, name, &ok);
        if (ok) spurUmbenennen(spur, neu);
    });
    menu.addAction(tr("Remove track"), this, [this, spur] { spurLoeschen(spur); });
    menu.exec(ort);
}

void DrumEditor::setSpielt(bool spielt, double schritt)
{
    if (spielt != m_spielt)
    {
        m_spielt = spielt;
        m_play->setText(spielt ? tr("Stop") : tr("Play pattern"));
    }
    m_raster->setMarke(spielt ? schritt : -1.0);
}

void DrumEditor::setSpitze(double spitze)
{
    const QString text = spitze < 0.0 ? QStringLiteral("–") : QString::number(spitze, 'f', 2);
    if (text == m_spitze->text()) return;
    m_spitze->setText(text);
    // ueber 1 uebersteuert die Summe (L6 a)
    m_spitze->setStyleSheet(spitze > 1.0 ? QStringLiteral("color: #e05050; font-weight: bold;") : QString());
}

// --- die Aenderungen: jede meldet eine neue Fassung des Loops ----------------

void DrumEditor::setzeTakte(int takte)
{
    Loop neu = m_loop;
    if (!setzeMusterTakte(neu, m_muster, takte) || neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern length"));
}

void DrumEditor::setzeRaster(int schlaegeJeTakt, int schritteJeSchlag)
{
    Loop neu = m_loop;
    if (!lumi::loop::setzeRaster(neu, schlaegeJeTakt, schritteJeSchlag)) return;
    Q_EMIT bearbeitet(neu, tr("Change grid"));
}

void DrumEditor::setzeSwing(double swing)
{
    Loop neu = m_loop;
    neu.swing = std::clamp(swing, 0.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change swing"));
}

void DrumEditor::spurHinzu()
{
    Loop neu = m_loop;
    const int spur = lumi::loop::spurHinzu(neu, m_muster, m_kitKlaenge);
    if (spur < 0) return;
    Q_EMIT bearbeitet(neu, tr("Add track"));
    waehleSpur(spur);
}

void DrumEditor::spurLoeschen(int spur)
{
    Loop neu = m_loop;
    if (!lumi::loop::spurLoeschen(neu, m_muster, spur)) return;
    Q_EMIT bearbeitet(neu, tr("Remove track"));
}

void DrumEditor::spurUmbenennen(int spur, const QString& name)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size()) || name.trimmed().isEmpty()) return;
    ziel->spuren[static_cast<std::size_t>(spur)].name = name.trimmed();
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Rename track"));
}

bool DrumEditor::aendereSpur(int spur, const QString& was, const std::function<void(Spur&)>& tu)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return false;
    tu(ziel->spuren[static_cast<std::size_t>(spur)]);
    if (neu == m_loop) return false;
    Q_EMIT bearbeitet(neu, was);
    return true;
}

void DrumEditor::setzeKlang(int spur, const QString& klang)
{
    const bool geaendert = aendereSpur(spur, tr("Change sound"), [&klang](Spur& s) {
        s.klang = klang;
        // die Spur spielt wieder einen Klang: kein Instrument, Note und Length auf der Vorgabe
        s.instrument.clear();
        s.hoehe = Spur().hoehe;
        s.dauerMs = kSpurDauerMs;
    });
    // nichts geaendert (derselbe Klang noch einmal gewaehlt): die Auswahl steht wieder richtig
    if (!geaendert) zeigeAlles();
}

// --- der Klang einer Spur: Instrument, Datei, Aufnahme (Stufe 11c, Schritt 2) ---

void DrumEditor::klangGewaehlt(int spur, const QString& wahl)
{
    if (wahl == kInstrumentJetzt) return;  // das spielt die Spur schon
    if (wahl == kDateiWahl)
    {
        frageNachSampleDatei(spur);
        return;
    }
    if (wahl == kInstrumentWahl || wahl == kAufnahmeWahl)
    {
        // die Auswahl zeigt wieder, was die Spur spielt — geaendert wird erst im Fenster
        zeigeAlles();
        if (wahl == kInstrumentWahl)
            (void)oeffnePalette(spur);
        else
            (void)oeffneAufnahme(spur);
        return;
    }
    setzeKlang(spur, wahl);
}

void DrumEditor::frageNachSampleDatei(int spur)
{
    QSettings einstellungen;
    const QString pfad =
        QFileDialog::getOpenFileName(this, tr("Choose sample"), einstellungen.value(QLatin1String(kSampleOrdner)).toString(),
                                     tr("Audio (*.wav *.mp3 *.flac *.ogg);;All files (*)"));
    if (pfad.isEmpty())
    {
        zeigeAlles();  // die Auswahl steht wieder auf dem Klang der Spur
        return;
    }
    einstellungen.setValue(QLatin1String(kSampleOrdner), QFileInfo(pfad).absolutePath());
    // Entscheid S2 a: kopieren oder am Ort lassen — dieselbe Frage wie im Instrument-Creator
    const SampleKopie wahl = waehleSampleKopie(this, pfad, tr("Without a copy the track names the file at"),
                                               QStringLiteral("drumSampleCopyDialog"));
    if (wahl == SampleKopie::Fragen)
    {
        zeigeAlles();  // abgebrochen
        return;
    }
    if (!nimmSampleDatei(spur, pfad, wahl == SampleKopie::Kopieren))
    {
        QMessageBox::warning(this, tr("Sample"), tr("Could not copy %1 to My samples").arg(QDir::toNativeSeparators(pfad)));
        zeigeAlles();
    }
}

bool DrumEditor::nimmSampleDatei(int spur, const QString& pfad, bool kopieren)
{
    const QString nennung = kopieren ? nimmZuEigenen(pfad) : pfad;
    if (nennung.isEmpty()) return false;
    setzeKlang(spur, nennung);
    return true;
}

void DrumEditor::setzeSpurInstrument(int spur, const QString& nennung)
{
    if (nennung.isEmpty()) return;
    const bool geaendert = aendereSpur(spur, tr("Change sound"), [&nennung](Spur& s) {
        const bool erstes = s.instrument.isEmpty();
        s.instrument = nennung;
        if (!erstes) return;  // ein anderes Instrument: Note und Length bleiben
        // K2 a: die Note nach der Art des Instruments, Length 250 ms
        Instrument instrument;
        s.hoehe = holeInstrument(nennung, instrument) ? InstrumentPalette::hoeheFuerArt(instrument.art) : Spur().hoehe;
        s.dauerMs = kSpurDauerMs;
    });
    if (!geaendert) zeigeAlles();
}

void DrumEditor::setzeNote(int spur, int hoehe)
{
    (void)aendereSpur(spur, tr("Change note"), [hoehe](Spur& s) {
        if (!s.instrument.isEmpty()) s.hoehe = std::clamp(hoehe, kHoeheMin, kHoeheMax);
    });
}

void DrumEditor::setzeDauer(int spur, int ms)
{
    (void)aendereSpur(spur, tr("Change length"), [ms](Spur& s) {
        if (!s.instrument.isEmpty()) s.dauerMs = std::clamp(ms, kSpurDauerMinMs, kSpurDauerMaxMs);
    });
}

bool DrumEditor::spieleSpur(int spur)
{
    const Spur* ziel = spurBei(spur);
    if (ziel == nullptr || ziel->instrument.isEmpty()) return false;
    Q_EMIT spurTonVerlangt(m_muster, spur);
    return true;
}

InstrumentPalette* DrumEditor::palette() const
{
    return m_palette;
}

bool DrumEditor::oeffnePalette(int spur, bool zeigen)
{
    if (spurBei(spur) == nullptr) return false;
    if (m_palette == nullptr)
    {
        m_palette = new InstrumentPalette(this);
        // Use: das Instrument kommt in die Spur, fuer die das Fenster gerade waehlt
        connect(m_palette, &InstrumentPalette::verwenden, this,
                [this](const QString& nennung) { setzeSpurInstrument(m_paletteSpur, nennung); });
        connect(m_palette, &InstrumentPalette::tonVerlangt, this, &DrumEditor::instrumentTonVerlangt);
        // Ctrl+Z / Ctrl+Y im Fenster: weiter ans Panel, dort liegt Undo
        connect(m_palette, &InstrumentPalette::rueckgaengigVerlangt, this, &DrumEditor::rueckgaengigVerlangt);
    }
    m_paletteSpur = spur;
    const Spur* ziel = spurBei(spur);
    m_palette->setzeZiel(m_muster, ziel->name);
    if (zeigen)
        m_palette->zeigeVorn();
    else
        m_palette->liesNeu();
    // das Instrument der Spur ist gewaehlt — ohne es anzuspielen; spielt sie noch keins,
    // steht das Fenster auf Drums (K3 a), die anderen Arten bleiben waehlbar
    if (!ziel->instrument.isEmpty())
        (void)m_palette->waehle(ziel->instrument);
    else
        (void)m_palette->zeigeArt(QStringLiteral("drums"));
    return true;
}

InstrumentCreator* DrumEditor::creator() const
{
    return m_creator;
}

bool DrumEditor::oeffneCreator(int spur, bool zeigen)
{
    const Spur* ziel = spurBei(spur);
    Instrument instrument;
    if (ziel == nullptr || ziel->instrument.isEmpty() || !holeInstrument(ziel->instrument, instrument)) return false;
    stelleCreator();
    m_creatorSpur = spur;
    m_wandelSpur = -1;  // ein „Edit sound...", das nie gespeichert wurde, gilt nicht mehr
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

void DrumEditor::stelleCreator()
{
    if (m_creator != nullptr) return;
    m_creator = new InstrumentCreator(this);
    // gespeichert: die Spur nennt jetzt, was der Creator geschrieben hat — wenn sie noch
    // ein Instrument spielt (bei Save in dieselbe Datei aendert sich am Loop nichts). Nach
    // „Edit sound..." macht das erste Save die Spur zur Instrument-Spur.
    connect(m_creator, &InstrumentCreator::gespeichert, this, [this](const QString& nennung) {
        const bool wandeln = m_wandelSpur >= 0 && m_wandelSpur == m_creatorSpur;
        const QString klang = m_wandelKlang;
        m_wandelSpur = -1;
        // K2 a: die Note auf dem Root note der Sample-Schicht, Length so lang wie die Aufnahme
        int hoehe = Spur().hoehe;
        int dauerMs = kSpurDauerMs;
        if (wandeln)
            for (const Schicht& schicht : m_creator->instrument().schichten)
                if (schicht.schwingung == Schwingung::Sample)
                {
                    hoehe = std::clamp(schicht.grundton, 0, 127);
                    dauerMs = aufnahmeMs(schicht);
                    break;
                }
        (void)aendereSpur(m_creatorSpur, wandeln ? tr("Edit sound") : tr("Change sound"), [&](Spur& s) {
            if (!s.instrument.isEmpty())
            {
                s.instrument = nennung;
                return;
            }
            // die Spur spielt inzwischen etwas anderes als beim „Edit sound...": sie bleibt dabei
            if (!wandeln || s.klang != klang) return;
            s.instrument = nennung;
            s.hoehe = hoehe;
            s.dauerMs = dauerMs;
        });
        Q_EMIT klaengeGeaendert();
        if (m_palette != nullptr) m_palette->liesNeu();  // ein neues eigenes steht gleich im Fenster
        zeigeAlles();  // der Titel kann sich geaendert haben, auch wenn die Nennung blieb
    });
    connect(m_creator, &InstrumentCreator::tonVerlangt, this, &DrumEditor::instrumentTonVerlangt);
    connect(m_creator, &InstrumentCreator::tonLangVerlangt, this, &DrumEditor::instrumentTonLangVerlangt);
    connect(m_creator, &InstrumentCreator::sampleVerlangt, this, &DrumEditor::sampleVerlangt);
    m_creator->setEngineGeber(m_engineGeber);
}

int DrumEditor::aufnahmeMs(const Schicht& schicht)
{
    std::shared_ptr<const SampleDaten> daten = sampleDaten(schicht.sample);
    if (daten == nullptr)
    {
        // dekodieren kann nur, wer die Audio-Engine hat — er merkt die Daten noch im Signal
        Q_EMIT sampleVerlangt(schicht.sample);
        daten = sampleDaten(schicht.sample);
    }
    if (daten == nullptr || daten->rate <= 0) return kSpurDauerMs;
    const double ms = 1000.0 * static_cast<double>(daten->klang.frames()) / daten->rate - schicht.sampleStartMs;
    return std::clamp(static_cast<int>(std::ceil(ms)), kSpurDauerMinMs, kSpurDauerMaxMs);
}

Instrument DrumEditor::instrumentAusKlang(const QString& klang)
{
    Instrument neu;
    // der Titel aus dem Namen des Klangs: „table_knock" wird „Table knock"
    QString titel = QFileInfo(sampleTitel(klang)).completeBaseName();
    titel.replace(QLatin1Char('_'), QLatin1Char(' '));
    if (!titel.isEmpty()) titel[0] = titel.at(0).toUpper();
    neu.titel = titel.isEmpty() ? QStringLiteral("Sound") : titel;
    neu.art = QStringLiteral("drums");  // K3 a
    Schicht& schicht = neu.schichten[0];
    schicht.schwingung = Schwingung::Sample;
    schicht.sample = klang;
    // bei ihrem Grundton, ohne Anstieg und voll gehalten spielt die Schicht die Aufnahme, wie sie ist
    schicht.grundton = 60;
    schicht.anstiegMs = 0.0;
    schicht.halten = 1.0;
    return neu;
}

bool DrumEditor::bearbeiteKlang(int spur, bool zeigen)
{
    const Spur* ziel = spurBei(spur);
    if (ziel == nullptr) return false;
    if (!ziel->instrument.isEmpty()) return oeffneCreator(spur, zeigen);  // schon ein Instrument
    if (ziel->klang.isEmpty()) return false;
    stelleCreator();
    m_creatorSpur = spur;
    // am Loop aendert sich nichts — erst wenn der Creator speichert
    m_wandelSpur = spur;
    m_wandelKlang = ziel->klang;
    m_creator->setzeSchlagMs(60000.0 / m_loop.bpm);
    m_creator->oeffne(instrumentAusKlang(ziel->klang));
    if (zeigen)
    {
        m_creator->show();
        m_creator->raise();
        m_creator->activateWindow();
    }
    return true;
}

void DrumEditor::setEngineGeber(std::function<IAudioEngine*()> geber)
{
    m_engineGeber = std::move(geber);
    if (m_creator != nullptr) m_creator->setEngineGeber(m_engineGeber);
}

AufnahmeFenster* DrumEditor::aufnahmeFenster() const
{
    return m_aufnahme;
}

bool DrumEditor::oeffneAufnahme(int spur, bool zeigen)
{
    if (spurBei(spur) == nullptr) return false;
    if (m_aufnahme == nullptr)
    {
        m_aufnahme = new AufnahmeFenster(this);
        // Save and use: die Aufnahme ist der Klang der Spur, fuer die das Fenster geoeffnet wurde
        connect(m_aufnahme, &AufnahmeFenster::gespeichert, this, [this](const QString& nennung) {
            if (spurBei(m_aufnahmeSpur) == nullptr) return;
            setzeKlang(m_aufnahmeSpur, nennung);
            // unter einem Namen, den schon eine Spur nennt, liegt jetzt eine andere Aufnahme
            Q_EMIT klaengeGeaendert();
        });
    }
    m_aufnahmeSpur = spur;
    // die Engine, wie sie jetzt ist — sie kann seit dem letzten Mal eine andere sein
    m_aufnahme->setEngine(m_engineGeber ? m_engineGeber() : nullptr);
    m_aufnahme->beginne();
    if (zeigen)
    {
        m_aufnahme->show();
        m_aufnahme->raise();
        m_aufnahme->activateWindow();
    }
    return true;
}

bool DrumEditor::oeffneDetail(int spur, bool zeigen)
{
    const Spur* ziel = spurBei(spur);
    if (ziel == nullptr || ziel->instrument.isEmpty()) return false;
    if (m_detail == nullptr)
    {
        // ein kleines Feld unter dem Knopf; ein Klick daneben schliesst es
        auto* feld = new QFrame(this, Qt::Popup);
        feld->setObjectName(QStringLiteral("drumTrackDetail"));
        feld->setFrameShape(QFrame::StyledPanel);
        auto* aussen = new QVBoxLayout(feld);
        m_detailTitel = new QLabel(feld);
        m_detailTitel->setObjectName(QStringLiteral("drumTrackDetailTitle"));
        aussen->addWidget(m_detailTitel);
        m_detailNote = new QComboBox(feld);
        m_detailNote->setObjectName(QStringLiteral("drumTrackNote"));
        m_detailNote->setToolTip(tr("The pitch of every hit of this track"));
        for (int hoehe = kHoeheMin; hoehe <= kHoeheMax; ++hoehe) m_detailNote->addItem(tonName(hoehe), hoehe);
        m_detailDauer = new QSpinBox(feld);
        m_detailDauer->setObjectName(QStringLiteral("drumTrackLength"));
        m_detailDauer->setRange(kSpurDauerMinMs, kSpurDauerMaxMs);
        m_detailDauer->setSingleStep(10);
        m_detailDauer->setSuffix(tr(" ms"));
        m_detailDauer->setKeyboardTracking(false);
        m_detailDauer->setToolTip(tr("How long every hit is held before it fades out — the same at any tempo"));
        auto* play = new QPushButton(tr("Play"), feld);
        play->setObjectName(QStringLiteral("drumTrackPlay"));
        play->setToolTip(tr("Play one hit of this track"));
        auto* werte = new QHBoxLayout;
        werte->addWidget(new QLabel(tr("Note"), feld));
        werte->addWidget(m_detailNote);
        werte->addWidget(new QLabel(tr("Length"), feld));
        werte->addWidget(m_detailDauer);
        werte->addWidget(play);
        aussen->addLayout(werte);
        auto* suchen = new QPushButton(tr("Browse..."), feld);
        suchen->setObjectName(QStringLiteral("drumTrackBrowse"));
        suchen->setToolTip(tr("Choose another instrument for this track"));
        auto* bearbeiten = new QPushButton(tr("Edit instrument..."), feld);
        bearbeiten->setObjectName(QStringLiteral("drumTrackEditInstrument"));
        bearbeiten->setToolTip(tr("Open the instrument of this track in the creator"));
        auto* wege = new QHBoxLayout;
        wege->addWidget(suchen);
        wege->addWidget(bearbeiten);
        wege->addStretch(1);
        aussen->addLayout(wege);
        m_detail = feld;

        connect(m_detailNote, &QComboBox::activated, this,
                [this](int index) { setzeNote(m_detailSpur, m_detailNote->itemData(index).toInt()); });
        connect(m_detailDauer, &QSpinBox::valueChanged, this, [this](int ms) { setzeDauer(m_detailSpur, ms); });
        connect(play, &QPushButton::clicked, this, [this] { (void)spieleSpur(m_detailSpur); });
        // die beiden Fenster kommen nach vorn: das Feld weicht
        connect(suchen, &QPushButton::clicked, this, [this] {
            const int fuer = m_detailSpur;
            m_detail->hide();
            (void)oeffnePalette(fuer);
        });
        connect(bearbeiten, &QPushButton::clicked, this, [this] {
            const int fuer = m_detailSpur;
            m_detail->hide();
            (void)oeffneCreator(fuer);
        });
    }
    m_detailSpur = spur;
    zeigeDetail();
    if (zeigen && spur < static_cast<int>(m_spuren.size()))
    {
        const QToolButton* knopf = m_spuren[static_cast<std::size_t>(spur)].mehr;
        m_detail->adjustSize();
        m_detail->move(knopf->mapToGlobal(QPoint(0, knopf->height())));
        m_detail->show();
    }
    return true;
}

void DrumEditor::zeigeDetail()
{
    if (m_detail == nullptr) return;
    const Spur* ziel = spurBei(m_detailSpur);
    if (ziel == nullptr || ziel->instrument.isEmpty())
    {
        // die Spur gibt es nicht mehr, oder sie spielt wieder einen Klang
        m_detailSpur = -1;
        m_detail->hide();
        return;
    }
    const QSignalBlocker a(m_detailNote);
    const QSignalBlocker b(m_detailDauer);
    m_detailTitel->setText(tr("%1 plays %2").arg(ziel->name, InstrumentEditor::instrumentAnzeige(ziel->instrument)));
    int index = m_detailNote->findData(ziel->hoehe);
    if (index < 0)
    {
        // eine Note ausserhalb von C1 bis C7 (aus einer Datei): sie steht da, wie sie ist
        m_detailNote->addItem(tonName(ziel->hoehe), ziel->hoehe);
        index = m_detailNote->count() - 1;
    }
    m_detailNote->setCurrentIndex(index);
    m_detailDauer->setValue(ziel->dauerMs);
}

void DrumEditor::setzeLaut(int spur, double laut)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    ziel->spuren[static_cast<std::size_t>(spur)].laut = std::clamp(laut, 0.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change track volume"));
}

void DrumEditor::setzePan(int spur, double pan)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    ziel->spuren[static_cast<std::size_t>(spur)].pan = std::clamp(pan, -1.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change track pan"));
}

void DrumEditor::setzeStumm(int spur, bool stumm)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    ziel->spuren[static_cast<std::size_t>(spur)].stumm = stumm;
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Mute track"));
}

}  // namespace lumi::loop
