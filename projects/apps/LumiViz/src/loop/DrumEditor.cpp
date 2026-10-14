/**
 * @file DrumEditor.cpp
 * @brief Loop: der Drum-Editor — das Schritt-Raster einer Basis (Stufe 6b)
 */

#include "loop/DrumEditor.hpp"

#include "loop/AufnahmeFenster.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/KitFenster.hpp"
#include "loop/KlangAuswahl.hpp"
#include "loop/Kit.hpp"  // bibliotheksKlaenge
#include "loop/Sample.hpp"
#include "loop/SampleFrage.hpp"
#include "loop/SignalWeg.hpp"  // abweichFarbe: die Hinweisfarbe
#include "UI/widgets/PresetTypeIcons.hpp"  // dateiArtIcon

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
#include <map>
#include <optional>

namespace lumi::loop
{

namespace
{

const char* const kSampleOrdner = "loop/lastSampleDir";
const char* const kRechtsLoescht = "loop/drumRightClickDeletes";
const char* const kKitAlleRollen = "loop/kitAddAllRoles";        ///< K10 a
const char* const kKitLeereEntfernen = "loop/kitRemoveEmpty";    ///< K10 a
const char* const kSpurNr = "spur";
constexpr int kKlangBreite = 124;    ///< die Auswahl „Sound" — breit genug fuer den Titel eines Instruments
constexpr int kMehrBreite = 22;      ///< der Knopf „…" einer Instrument-Spur
constexpr int kRolleBreite = 64;     ///< das Etikett Role (Stufe 21b)
constexpr int kSpaltenBreite = 438;  ///< die Kopfzeilen der Spuren, links vom Raster
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
    m_abspiel = std::clamp(m_abspiel, 0, std::max(0, zahl - 1));
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

    // --- die Abspielstelle (B4 b): dort beginnt Play pattern — gruen, im Kopf und als Linie
    if (spuren > 0)
    {
        const int x = xVonSchritt(m_abspiel) - 2;
        const QColor gruen(80, 200, 120);
        p.setPen(QPen(gruen, 1.2));
        p.drawLine(x, 8, x, yVonSpur(spuren));
        p.setPen(Qt::NoPen);
        p.setBrush(gruen);
        const QPoint flagge[3] = {QPoint(x, 1), QPoint(x + 8, 5), QPoint(x, 9)};
        p.drawPolygon(flagge, 3);
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

void SchrittRaster::setzeAbspielstelle(int schritt)
{
    const int neu = std::clamp(schritt, 0, std::max(0, schritte() - 1));
    if (neu == m_abspiel) return;
    m_abspiel = neu;
    update();
    Q_EMIT abspielstelleGewaehlt(m_abspiel);
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
        // die Zeile der Takte (B4 b): ein Klick setzt die Abspielstelle, Shift+Klick die Einfuegemarke
        const int zahl = schritte();
        if (shift)
            setzeEinfuegemarke(zahl > 0 && ort.x() >= xVonSchritt(zahl - 1) + kSchrittBreite ? zahl : schrittNahe(ort.x()));
        else
        {
            const int schritt = schrittNahe(ort.x());
            if (schritt == m_abspiel)
                Q_EMIT abspielstelleGewaehlt(m_abspiel);  // derselbe Schritt noch einmal: wieder dorthin springen
            else
                setzeAbspielstelle(schritt);
        }
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
    // B2 a (S124): laenger gestellt, wiederholen die neuen Takte die bisherigen
    m_wiederholen = new QCheckBox(tr("Repeat when lengthening"), this);
    m_wiederholen->setObjectName(QStringLiteral("drumRepeatBars"));
    m_wiederholen->setToolTip(tr("On: more bars repeat the bars you have — one bar becomes four of the same. Off: "
                                 "the new bars are empty"));
    m_wiederholen->setChecked(wiederholenBeimVerlaengern());
    connect(m_wiederholen, &QCheckBox::toggled, this, [](bool an) { setzeWiederholenBeimVerlaengern(an); });
    m_play = new QPushButton(tr("Play pattern"), this);
    m_play->setObjectName(QStringLiteral("drumPlay"));
    m_play->setToolTip(tr("Play only this pattern, in a loop"));
    m_spitze = new QLabel(QStringLiteral("–"), this);
    m_spitze->setObjectName(QStringLiteral("drumPeak"));
    m_spitze->setMinimumWidth(36);
    m_spitze->setToolTip(tr("Peak of the mixed loop while it plays; above 1 it clips"));
    // der Knopf Kit (Stufe 21b, Skizze S123): sein Menue entsteht beim Aufklappen
    // ein Knopf mit Menue: er laesst seinem Pfeil eigenen Platz (ein QToolButton legt ihn ueber den Text)
    m_kitKnopf = new QPushButton(this);
    m_kitKnopf->setObjectName(QStringLiteral("drumKit"));
    auto* kitMenue = new QMenu(m_kitKnopf);
    kitMenue->setToolTipsVisible(true);
    m_kitKnopf->setMenu(kitMenue);
    connect(kitMenue, &QMenu::aboutToShow, this, [this, kitMenue] { fuelleKitMenue(kitMenue); });
    kopf->addWidget(new QLabel(tr("Base"), this));
    kopf->addWidget(m_basisName);
    kopf->addWidget(m_nutzer);
    kopf->addStretch(1);
    kopf->addWidget(m_kitKnopf);
    kopf->addSpacing(12);
    kopf->addWidget(new QLabel(tr("Bars"), this));
    kopf->addWidget(m_takte);
    kopf->addWidget(m_wiederholen);
    kopf->addWidget(m_play);
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);
    // was der letzte Kitwechsel getan hat (K2 a): getroffen, ohne Gegenstueck, was Add track anbietet
    m_kitZeile = new QLabel(this);
    m_kitZeile->setObjectName(QStringLiteral("drumKitMessage"));
    m_kitZeile->setEnabled(false);
    m_kitZeile->setWordWrap(true);
    m_kitZeile->hide();
    aussen->addWidget(m_kitZeile);

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
    titelFeld(tr("Role"), kRolleBreite);
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
    // was das * im Etikett Role heisst (K11 a) — nur, solange eine Spur es traegt
    m_fremdZeile = new QLabel(this);
    m_fremdZeile->setObjectName(QStringLiteral("drumNotInKit"));
    m_fremdZeile->setWordWrap(true);
    {
        QPalette farben = m_fremdZeile->palette();
        farben.setColor(QPalette::WindowText, SignalWeg::abweichFarbe(farben));
        m_fremdZeile->setPalette(farben);
    }
    m_fremdZeile->hide();
    aussen->addWidget(m_fremdZeile);
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
    connect(m_spurNeu, &QPushButton::clicked, this, &DrumEditor::spurNeuGeklickt);
    connect(m_raster, &SchrittRaster::spurGewaehlt, this, [this](int spur) { waehleSpur(spur); });
    connect(m_raster, &SchrittRaster::auswahlGeaendert, this, &DrumEditor::zeigeAuswahl);
    connect(m_raster, &SchrittRaster::abspielstelleGewaehlt, this, &DrumEditor::abspielstelleGewaehlt);
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
    resize(944, 330);
}

void DrumEditor::setKitKlaenge(const QStringList& klaenge)
{
    m_kitKlaenge = klaenge;
    if (m_kits != nullptr) m_kits->setKitKlaenge(klaenge);
    if (basis() != nullptr) zeigeAlles();
}

bool DrumEditor::zeige(const Loop& loop, const QString& muster)
{
    const Muster* ziel = loop.findeMuster(muster);
    if (ziel == nullptr || ziel->art != MusterArt::Drumkit) return false;
    // eine andere Basis: die Fenster daneben galten einer Spur der alten
    if (muster != m_muster)
    {
        m_paletteSpur = m_creatorSpur = m_aufnahmeSpur = m_detailSpur = -1;
        m_kitMeldung.clear();  // sie galt dem anderen Muster
    }
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
        const QSignalBlocker e(m_wiederholen);
        m_wiederholen->setChecked(wiederholenBeimVerlaengern());  // der andere Editor kann ihn umgestellt haben
        m_schlaege->setValue(m_loop.schlaegeJeTakt);
        m_jeSchlag->setValue(m_loop.schritteJeSchlag);
        m_swing->setValue(m_loop.swing * 100.0);
    }
    m_raster->setMuster(*ziel, m_loop.schlaegeJeTakt, m_loop.schritteJeSchlag);
    // das Raster in voller Hoehe, darunter Platz fuer den Rollbalken
    m_rollen->setFixedHeight(m_raster->sizeHint().height() + style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2);
    baueSpuren(*ziel);
    zeigeKit();
    m_leerHinweis->setVisible(ziel->spuren.empty());
    {
        DrumKit kit;
        const bool hatKit = aktuellesKit(kit);
        const bool fremd = std::any_of(m_spuren.begin(), m_spuren.end(),
                                       [](const SpurZeile& z) { return z.rolle->property("fehltImKit").toBool(); });
        m_fremdZeile->setText(fremd && hatKit ? tr("* not in the kit %1 — the track keeps its own sound").arg(kit.titel) : QString());
        m_fremdZeile->setVisible(!m_fremdZeile->text().isEmpty());
    }
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
        text = tr("Shift+drag selects steps · Ctrl+click adds or removes one · a click in the bar row sets where Play "
                  "starts, Shift+click the paste marker");
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

QStringList DrumEditor::klangZeilen(int spur) const
{
    const Spur* ziel = spurBei(spur);
    if (ziel == nullptr || spur >= static_cast<int>(m_spuren.size())) return {};
    return m_spuren[static_cast<std::size_t>(spur)].klang->zeilen();
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
            // das Etikett Role (Stufe 21b, K1 a): ein Klick oeffnet die Rollen
            zeile.rolle = new QToolButton(zeile.reihe);
            zeile.rolle->setObjectName(QStringLiteral("drumTrackRole%1").arg(s));
            zeile.rolle->setFixedSize(kRolleBreite, 20);
            zeile.rolle->setAutoRaise(true);
            zeile.klang = new KlangAuswahl(zeile.reihe);
            zeile.klang->setObjectName(QStringLiteral("drumTrackSound%1").arg(s));
            zeile.klang->setFixedWidth(kKlangBreite);
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
            reihe->addWidget(zeile.rolle);
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
            connect(zeile.rolle, &QToolButton::clicked, this, [this, s] { rollenMenue(s); });
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
    DrumKit kit;
    const bool hatKit = aktuellesKit(kit);
    for (std::size_t s = 0; s < m_spuren.size(); ++s)
    {
        const Spur& spur = muster.spuren[s];
        SpurZeile& zeile = m_spuren[s];
        {
            // das Etikett Role: gesetzt, geraten (in Klammern, grau) oder keine; fehlt sie im Kit, durchgestrichen
            const std::optional<Rolle> rolle = leseRolle(spur.rolle);
            const QString geraten = rolle ? QString() : rateRollenArt(spur);
            const bool fehlt = rolle && hatKit && kit.fuer(*rolle) == nullptr;
            zeile.rolle->setText(rolle ? rollenText(*rolle) + (fehlt ? QStringLiteral(" *") : QString())
                                       : geraten.isEmpty() ? QStringLiteral("—")
                                                           : QStringLiteral("(%1)").arg(geraten));
            QFont schrift = zeile.rolle->font();
            schrift.setItalic(!rolle);
            zeile.rolle->setFont(schrift);
            {
                // K11 a: das * und die Rolle in der Hinweisfarbe, sonst die Farbe des Fensters
                QPalette farben = QWidget::palette();  // palette() nennt hier das Fenster Instruments
                if (fehlt) farben.setColor(QPalette::ButtonText, SignalWeg::abweichFarbe(farben));
                zeile.rolle->setPalette(farben);
            }
            zeile.rolle->setProperty("fehltImKit", fehlt);
            if (fehlt)
                zeile.rolle->setToolTip(tr("%1 — not in the kit %2: the track keeps its sound").arg(rollenText(*rolle), kit.titel));
            else if (rolle)
                zeile.rolle->setToolTip(tr("Role in the kit: a new kit gives this track the sound of %1").arg(rollenText(*rolle)));
            else
                zeile.rolle->setToolTip(geraten.isEmpty() ? tr("No role — click to choose one. Without a role, a new kit leaves this track as it is")
                                                          : tr("Guessed: %1 — a new kit takes it. Click to choose another").arg(geraten));
        }
        const QSignalBlocker a(zeile.stumm);
        const QSignalBlocker b(zeile.laut);
        const QSignalBlocker c(zeile.pan);
        zeile.stumm->setChecked(spur.stumm);
        zeile.name->setText(zeile.name->fontMetrics().elidedText(spur.name, Qt::ElideRight, 70));
        zeile.name->setToolTip(spur.name);
        zeile.klang->fuelle(spur, m_kitKlaenge, eigene, bibliothek, true);
        // sonst findet es niemand: sagen, wo das Feld hinter „…" und Edit sound... stehen
        if (!spur.instrument.isEmpty())
            zeile.klang->setToolTip(zeile.klang->toolTip() + tr(". The button … next to it changes that"));
        else if (!spur.klang.isEmpty())
            zeile.klang->setToolTip(zeile.klang->toolTip() + tr("\nRight-click the track: Edit sound... makes an instrument of it"));
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
    if (!setzeMusterTakte(neu, m_muster, takte, wiederholenBeimVerlaengern()) || neu == m_loop) return;
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
    if (wahl == kKlangWahlJetzt) return;  // das spielt die Spur schon
    if (wahl == kKlangWahlDatei)
    {
        frageNachSampleDatei(spur);
        return;
    }
    if (wahl == kKlangWahlInstrument || wahl == kKlangWahlAufnahme)
    {
        // die Auswahl zeigt wieder, was die Spur spielt — geaendert wird erst im Fenster
        zeigeAlles();
        if (wahl == kKlangWahlInstrument)
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

// --- Kits (Stufe 21b; Entscheide K1 a bis K5 a, S123) -------------------------

namespace
{
/// „open hat" → „Open hat": der Name einer neuen Spur aus ihrer Rolle
QString spurNameAus(const QString& rolle)
{
    return rolle.isEmpty() ? rolle : rolle.left(1).toUpper() + rolle.mid(1);
}
}  // namespace

bool DrumEditor::aktuellesKit(DrumKit& kit) const
{
    const Muster* ziel = basis();
    return ziel != nullptr && !ziel->kit.isEmpty() && holeKit(ziel->kit, kit);
}

QString DrumEditor::kitText() const
{
    const Muster* ziel = basis();
    if (ziel == nullptr || ziel->kit.isEmpty()) return tr("Kit");
    DrumKit kit;
    if (!holeKit(ziel->kit, kit)) return tr("Kit: %1 (not found)").arg(ziel->kit);
    return weichtAb(*ziel, kit) ? tr("Kit: %1 (changed)").arg(kit.titel) : tr("Kit: %1").arg(kit.titel);
}

void DrumEditor::zeigeKit()
{
    m_kitKnopf->setText(kitText());  // den Pfeil des Menues zeichnet der Knopf selbst
    // das Datei-Icon der Kits (S124, I2 a; A2 a: wo ein Kit beim Namen steht)
    if (m_kitKnopf->icon().isNull()) m_kitKnopf->setIcon(lumi::ui::dateiArtIcon(QStringLiteral("lvkit")));
    m_kitKnopf->setToolTip(tr("Choose a kit: every track takes the sound of its role, the steps stay. "
                              "\"(changed)\" means a track differs from the kit"));
    m_kitZeile->setText(m_kitMeldung);
    m_kitZeile->setVisible(!m_kitMeldung.isEmpty());
}

QStringList DrumEditor::kitZeilen() const
{
    const Muster* ziel = basis();
    const QString gewaehlt = ziel != nullptr ? ziel->kit : QString();
    QStringList zeilen{QStringLiteral("[Built-in kits]")};
    for (const DrumKit& kit : mitgelieferteKits()) zeilen << (kit.name == gewaehlt ? QStringLiteral("*") : QString()) + kit.name;
    const std::vector<DrumKit> eigene = eigeneKits();
    if (!eigene.empty())
    {
        zeilen << QStringLiteral("[My kits]");
        for (const DrumKit& kit : eigene) zeilen << (kit.name == gewaehlt ? QStringLiteral("*") : QString()) + kit.name;
    }
    const KitOptionen optionen = kitOptionen();
    zeilen << (optionen.alleRollen ? QStringLiteral("[x] ") : QStringLiteral("[ ] ")) + tr("Add all roles of the kit")
           << (optionen.leereEntfernen ? QStringLiteral("[x] ") : QStringLiteral("[ ] ")) + tr("Remove empty tracks the kit doesn't have");
    zeilen << tr("Save as kit...") << tr("Edit kits...");
    return zeilen;
}

void DrumEditor::fuelleKitMenue(QMenu* menue)
{
    menue->clear();
    const Muster* ziel = basis();
    const QString gewaehlt = ziel != nullptr ? ziel->kit : QString();
    const auto liste = [this, menue, &gewaehlt](const QString& titel, const std::vector<DrumKit>& kits) {
        if (kits.empty()) return;
        menue->addSection(titel);
        for (const DrumKit& kit : kits)
        {
            QAction* eintrag = menue->addAction(kit.titel, this, [this, nennung = kit.name] { (void)wendeKitAn(nennung); });
            eintrag->setCheckable(true);
            eintrag->setChecked(kit.name == gewaehlt);
            eintrag->setToolTip(kit.beschreibung);
        }
    };
    liste(tr("Built-in kits"), mitgelieferteKits());
    liste(tr("My kits"), eigeneKits());
    menue->addSeparator();
    // K10 a: was ein Kitwechsel mit den Spuren tut — gemerkt, beide zuerst an
    const KitOptionen optionen = kitOptionen();
    QAction* alle = menue->addAction(tr("Add all roles of the kit"), this, [](bool an) {
        KitOptionen neu = kitOptionen();
        neu.alleRollen = an;
        setzeKitOptionen(neu);
    });
    alle->setCheckable(true);
    alle->setChecked(optionen.alleRollen);
    alle->setToolTip(tr("Choosing a kit adds a track for each of its roles that no track has, with empty steps"));
    QAction* leere = menue->addAction(tr("Remove empty tracks the kit doesn't have"), this, [](bool an) {
        KitOptionen neu = kitOptionen();
        neu.leereEntfernen = an;
        setzeKitOptionen(neu);
    });
    leere->setCheckable(true);
    leere->setChecked(optionen.leereEntfernen);
    leere->setToolTip(tr("Choosing a kit removes a track whose role the kit doesn't have — only if no step is set. "
                         "Tracks with steps stay and are marked with *"));
    menue->addSeparator();
    QAction* sichern = menue->addAction(tr("Save as kit..."), this, [this] {
        bool ok = false;
        const QString titel = QInputDialog::getText(this, tr("Save as kit"), tr("Name of the kit:"), QLineEdit::Normal,
                                                    QString(), &ok);
        if (!ok || titel.trimmed().isEmpty()) return;
        if (!speichereAlsKit(titel).isEmpty()) return;
        // vergeben? Dann fragen, ob ersetzt wird
        if (!eigeneKitDatei(eigenerName(titel)).isEmpty() && QFileInfo::exists(eigeneKitDatei(eigenerName(titel))) &&
            QMessageBox::question(this, tr("Save as kit"), tr("You already have a kit named %1. Replace it?").arg(titel)) ==
                QMessageBox::Yes)
            (void)speichereAlsKit(titel, true);
    });
    sichern->setToolTip(tr("Saves the sounds of these tracks, by role, with volume and pan as your own kit"));
    QAction* bearbeiten = menue->addAction(tr("Edit kits..."), this, [this] { oeffneKits(); });
    bearbeiten->setToolTip(tr("Look at the kits, make your own copy, change sounds, volume and pan"));
}

KitFenster* DrumEditor::kitFenster() const
{
    return m_kits;
}

void DrumEditor::oeffneKits(bool zeigen)
{
    if (m_kits == nullptr)
    {
        m_kits = new KitFenster(this);
        connect(m_kits, &KitFenster::anspielenVerlangt, this, &DrumEditor::kitAnspielenVerlangt);
        connect(m_kits, &KitFenster::instrumentTonVerlangt, this, &DrumEditor::instrumentTonVerlangt);
        // ein eigenes Kit ist neu geschrieben oder weggefallen: Knopf Kit und Etiketten Role neu
        // („(changed)" und „not in kit" haengen am Kit, wie es jetzt in der Datei steht)
        connect(m_kits, &KitFenster::gespeichert, this, [this] {
            if (basis() != nullptr) zeigeAlles();
        });
        connect(m_kits, &KitFenster::kitsGeaendert, this, [this] {
            if (basis() != nullptr) zeigeAlles();
        });
    }
    m_kits->setKitKlaenge(m_kitKlaenge);
    m_kits->setOffenerLoop(m_loop);
    const Muster* ziel = basis();
    m_kits->oeffne(ziel != nullptr ? ziel->kit : QString());
    if (zeigen)
    {
        m_kits->show();
        m_kits->raise();
        m_kits->activateWindow();
    }
}

bool DrumEditor::wendeKitAn(const QString& nennung)
{
    DrumKit kit;
    if (!holeKit(nennung, kit)) return false;
    Loop neu = m_loop;
    const Muster* vorher = basis();
    if (vorher == nullptr) return false;
    const std::size_t spurenVorher = vorher->spuren.size();
    const KitWechsel wechsel = lumi::loop::wendeKitAn(neu, m_muster, kit, kitOptionen());
    // was geschehen ist, in einer Zeile (K2 a: wer keine Entsprechung hat, behaelt seinen Klang; K10 a)
    QStringList teile{tr("%1: %2 of %3 tracks took a sound from the kit")
                          .arg(kit.titel)
                          .arg(wechsel.getroffen)
                          .arg(spurenVorher)};
    if (!wechsel.angelegt.isEmpty())
        teile << (wechsel.angelegt.size() == 1 ? tr("1 track created") : tr("%1 tracks created").arg(wechsel.angelegt.size()));
    if (!wechsel.entfernt.isEmpty())
        teile << tr("empty, not in the kit, removed: %1").arg(wechsel.entfernt.join(QStringLiteral(", ")));
    if (!wechsel.ohneGegenstueck.isEmpty())
        teile << tr("not in the kit, kept: %1").arg(wechsel.ohneGegenstueck.join(QStringLiteral(", ")));
    if (!wechsel.fehlend.isEmpty())
        teile << tr("Add track offers: %1").arg(wechsel.fehlend.join(QStringLiteral(", ")));
    m_kitMeldung = teile.join(QStringLiteral(" · "));
    if (neu == m_loop)
    {
        zeigeKit();  // nichts geaendert (dasselbe Kit noch einmal): nur die Zeile
        return true;
    }
    Q_EMIT bearbeitet(neu, tr("Kit: %1").arg(kit.titel));
    return true;
}

void DrumEditor::setzeRolle(int spur, const QString& rolle)
{
    QString text;
    if (!rolle.trimmed().isEmpty())
    {
        const std::optional<Rolle> gelesen = leseRolle(rolle);
        if (!gelesen) return;
        text = rollenText(*gelesen);
    }
    (void)aendereSpur(spur, tr("Change role"), [&text](Spur& s) { s.rolle = text; });
}

void DrumEditor::rollenMenue(int spur)
{
    const Muster* ziel = basis();
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    waehleSpur(spur);
    // je Art die hoechste Nummer, die eine Spur schon traegt — angeboten wird eine mehr
    std::map<QString, int> hoechste;
    for (const Spur& s : ziel->spuren)
        if (const std::optional<Rolle> r = leseRolle(s.rolle)) hoechste[r->art] = std::max(hoechste[r->art], r->nummer);
    const QString jetzt = ziel->spuren[static_cast<std::size_t>(spur)].rolle;
    QMenu menue(this);
    for (const QString& art : rollenArten())
    {
        const int bis = std::max(1, hoechste[art] + (art == QLatin1String("tom") || art == QLatin1String("perc") || hoechste[art] > 0 ? 1 : 0));
        for (int n = 1; n <= bis; ++n)
        {
            const QString text = rollenText({art, n});
            QAction* eintrag = menue.addAction(text, this, [this, spur, text] { setzeRolle(spur, text); });
            eintrag->setCheckable(true);
            eintrag->setChecked(text == jetzt);
        }
    }
    menue.addSeparator();
    menue.addAction(tr("No role"), this, [this, spur] { setzeRolle(spur, QString()); });
    menue.exec(m_spuren[static_cast<std::size_t>(spur)].rolle->mapToGlobal(QPoint(0, m_spuren[static_cast<std::size_t>(spur)].rolle->height())));
}

QStringList DrumEditor::fehlendeRollen() const
{
    const Muster* ziel = basis();
    DrumKit kit;
    if (ziel == nullptr || !aktuellesKit(kit)) return {};
    Muster probe = *ziel;  // geraten wird nur auf der Probe
    return lumi::loop::wendeKitAn(probe, kit).fehlend;
}

int DrumEditor::spurAusKit(const QString& rolle)
{
    DrumKit kit;
    const std::optional<Rolle> gelesen = leseRolle(rolle);
    if (!gelesen || !aktuellesKit(kit)) return -1;
    const QString text = rollenText(*gelesen);
    const auto treffer = std::find_if(kit.klaenge.begin(), kit.klaenge.end(), [&text](const KitKlang& k) { return k.rolle == text; });
    if (treffer == kit.klaenge.end()) return -1;
    Loop neu = m_loop;
    const int spur = lumi::loop::spurHinzu(neu, m_muster, {});
    Muster* ziel = musterIn(neu, m_muster);
    if (spur < 0 || ziel == nullptr) return -1;
    Spur neue = spurAusKitKlang(*treffer);
    ziel->spuren[static_cast<std::size_t>(spur)].name.clear();  // die leere Spur zaehlt beim Namen nicht mit
    neue.name = freierSpurName(*ziel, text);
    ziel->spuren[static_cast<std::size_t>(spur)] = neue;
    Q_EMIT bearbeitet(neu, tr("Add track"));
    waehleSpur(spur);
    return spur;
}

void DrumEditor::spurNeuGeklickt()
{
    const QStringList fehlend = fehlendeRollen();
    if (fehlend.isEmpty())
    {
        spurHinzu();
        return;
    }
    // die Rollen des Kits, die keine Spur hat — darunter die leere Spur wie bisher (K2 a)
    DrumKit kit;
    (void)aktuellesKit(kit);
    QMenu menue(this);
    for (const QString& rolle : fehlend)
        menue.addAction(tr("%1 — from %2").arg(spurNameAus(rolle), kit.titel), this, [this, rolle] { (void)spurAusKit(rolle); });
    menue.addSeparator();
    menue.addAction(tr("Empty track"), this, [this] { spurHinzu(); });
    menue.exec(m_spurNeu->mapToGlobal(QPoint(0, m_spurNeu->height())));
}

QString DrumEditor::speichereAlsKit(const QString& titel, bool ueberschreiben)
{
    const Muster* ziel = basis();
    const QString name = eigenerName(titel);
    const QString datei = eigeneKitDatei(name);
    if (ziel == nullptr || datei.isEmpty()) return {};
    if (QFileInfo::exists(datei) && !ueberschreiben) return {};
    DrumKit kit = kitAusMuster(*ziel, titel.trimmed());
    if (!speichereKit(datei, kit)) return {};
    // das Muster nennt jetzt das eigene Kit, und seine Spuren tragen die Rollen, unter denen sie gespeichert sind
    const QString nennung = QLatin1String(kEigenVorsatz) + name;
    Loop neu = m_loop;
    Muster* m = musterIn(neu, m_muster);
    rateRollen(*m);
    m->kit = nennung;
    m_kitMeldung = tr("Saved as your kit %1 (%2 roles)").arg(titel.trimmed()).arg(kit.klaenge.size());
    if (neu == m_loop)
        zeigeKit();
    else
        Q_EMIT bearbeitet(neu, tr("Save as kit"));
    return nennung;
}

QString DrumEditor::rollenEtikett(int spur) const
{
    return spur >= 0 && spur < static_cast<int>(m_spuren.size()) ? m_spuren[static_cast<std::size_t>(spur)].rolle->text() : QString();
}

bool DrumEditor::rolleFehltImKit(int spur) const
{
    return spur >= 0 && spur < static_cast<int>(m_spuren.size()) &&
           m_spuren[static_cast<std::size_t>(spur)].rolle->property("fehltImKit").toBool();
}

QString DrumEditor::kitMeldung() const
{
    return m_kitMeldung;
}

KitOptionen DrumEditor::kitOptionen()
{
    QSettings einstellungen;
    KitOptionen optionen;
    optionen.alleRollen = einstellungen.value(QLatin1String(kKitAlleRollen), true).toBool();
    optionen.leereEntfernen = einstellungen.value(QLatin1String(kKitLeereEntfernen), true).toBool();
    return optionen;
}

void DrumEditor::setzeKitOptionen(const KitOptionen& optionen)
{
    QSettings einstellungen;
    einstellungen.setValue(QLatin1String(kKitAlleRollen), optionen.alleRollen);
    einstellungen.setValue(QLatin1String(kKitLeereEntfernen), optionen.leereEntfernen);
}

QString DrumEditor::fremdZeile() const
{
    return m_fremdZeile->text();
}

}  // namespace lumi::loop
