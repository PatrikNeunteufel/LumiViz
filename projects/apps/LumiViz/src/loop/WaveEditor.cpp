/**
 * @file WaveEditor.cpp
 * @brief Loop: der Wave editor (Stufe 15d, Skizze SK15-1 a, S169)
 */

#include "loop/WaveEditor.hpp"

#include "loop/AufnahmeFenster.hpp"
#include "loop/Autotune.hpp"
#include "loop/Kit.hpp"  // klangDatei
#include "loop/Tonhoehenspeicher.hpp"
#include "loop/RasterZeile.hpp"
#include "loop/Sample.hpp"
#include "loop/SampleFrage.hpp"
#include "UI/widgets/Achse.hpp"
#include "UI/widgets/Bedienhilfe.hpp"
#include "UI/widgets/PresetTypeIcons.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTimer>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <thread>
#include <tuple>
#include <utility>

namespace lumi::loop
{

namespace
{

const char* const kLage = "loop/waveEditorGeometry";  ///< ein Pfad der Fensterlage: geht nie in einen Export
const char* const kOrdner = "loop/waveEditorFolder";  ///< wo Browse... zuletzt war

bool& lageMerken()
{
    static bool an = true;
    return an;
}

/// So kurz darf der Ausschnitt hoechstens werden, in Sekunden der Datei.
constexpr double kAusschnittMin = 0.01;
constexpr double kSchrittBreiteMin = 0.25;
constexpr double kSchrittBreiteMax = 80.0;
constexpr int kPunktFang = 5;

/// Rate und Kanaele aus dem Kopf einer WAV-Datei; 0, 0 = keine WAV.
std::pair<int, int> wavKopf(const QString& datei)
{
    QFile f(datei);
    if (!f.open(QIODevice::ReadOnly)) return {0, 0};
    const QByteArray kopf = f.read(64 * 1024);
    if (kopf.size() < 12 || !kopf.startsWith("RIFF") || kopf.mid(8, 4) != "WAVE") return {0, 0};
    const auto u16 = [&kopf](qsizetype i) {
        return static_cast<int>(static_cast<unsigned char>(kopf[i]) | (static_cast<unsigned char>(kopf[i + 1]) << 8));
    };
    const auto u32 = [&kopf](qsizetype i) {
        std::uint32_t wert = 0;
        for (int b = 3; b >= 0; --b) wert = (wert << 8) | static_cast<unsigned char>(kopf[i + b]);
        return wert;
    };
    for (qsizetype stelle = 12; stelle + 8 <= kopf.size();)
    {
        const QByteArray id = kopf.mid(stelle, 4);
        const std::uint32_t groesse = u32(stelle + 4);
        if (id == "fmt " && stelle + 16 <= kopf.size())
            return {static_cast<int>(u32(stelle + 12)), u16(stelle + 10)};
        stelle += 8 + static_cast<qsizetype>(groesse) + (groesse % 2);
    }
    return {0, 0};
}

/// Ein Wert der Spur Expression als Text — wie im Instrument-Editor (SK9 a).
QString expressionText(double wert)
{
    return wert <= 0.0 ? WaveFlaeche::tr("%1 (silent)").arg(wert, 0, 'f', 2)
                       : WaveFlaeche::tr("%1 (%2 dB)").arg(wert, 0, 'f', 2).arg(expressionDb(wert), 0, 'f', 1);
}

/// Ein Wert einer Spur als Text: Expression wie oben, Pitch in Halbtoenen wie im Instrument-Editor.
QString spurText(Steuerspur spur, double wert)
{
    if (spur == Steuerspur::Pitch)
        return WaveFlaeche::tr("%1%2 semitones").arg(wert > 0.0 ? QStringLiteral("+") : QString()).arg(wert, 0, 'f', 2);
    return expressionText(wert);
}

QString spurName(Steuerspur spur)
{
    return spur == Steuerspur::Pitch ? WaveFlaeche::tr("Pitch") : WaveFlaeche::tr("Expression");
}

constexpr std::array<Steuerspur, 2> kFlaechenSpuren{Steuerspur::Expression, Steuerspur::Pitch};

/// Stufe 15i: in dieser Rate misst der Editor die Tonhoehe (die Zeiten haengen nicht an ihr).
constexpr int kTonhoeheRate = 44100;
/// So kurz darf eine Zielnote hoechstens werden, in Sekunden der Datei.
constexpr double kNoteMin = 0.02;

const std::array<const char*, 12> kTonNamen{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

/// Ein Ton als Name: „C4“ (MIDI 60), „A#3“.
QString tonName(int midi)
{
    return QString::fromLatin1(kTonNamen[static_cast<std::size_t>(((midi % 12) + 12) % 12)]) + QString::number(midi / 12 - 1);
}

/// Eine Zielnote als Text: „C4 +12 ct · 80 %“.
QString notenText(const AutotuneAbschnitt& a)
{
    QString text = tonName(a.zielHoehe);
    if (a.zielCent != 0) text += QStringLiteral(" %1%2 ct").arg(a.zielCent > 0 ? QStringLiteral("+") : QString()).arg(a.zielCent);
    return text + QStringLiteral(" · %1 %").arg(std::lround(a.staerke * 100.0));
}

/// Gemessen, aber im Ausschnitt nichts gesungen (S170, nach dem ersten Versuch im Programm).
QString keinGesangText()
{
    return WaveFlaeche::tr("No sung pitch in this section — Autotune needs a single sung voice (not beatbox or speech)");
}

}  // namespace

WaveBild waveBildVon(const QString& datei, const Dekodierer& dekodierer, double maxSekunden)
{
    WaveBild bild;
    if (datei.isEmpty() || !dekodierer || !QFileInfo(datei).isFile()) return bild;
    const std::vector<float> stereo = dekodierer(datei, WaveBild::kBildRate, maxSekunden);
    const std::size_t frames = stereo.size() / 2;
    if (frames == 0) return bild;
    bild.sekunden = static_cast<double>(frames) / WaveBild::kBildRate;
    bild.spitzen.reserve(frames / WaveBild::kSpitzenFrames + 1);
    for (std::size_t anfang = 0; anfang < frames; anfang += WaveBild::kSpitzenFrames)
    {
        float spitze = 0.0f;
        const std::size_t ende = std::min(frames, anfang + WaveBild::kSpitzenFrames);
        for (std::size_t i = anfang; i < ende; ++i)
            spitze = std::max({spitze, std::abs(stereo[2 * i]), std::abs(stereo[2 * i + 1])});
        bild.spitzen.push_back(std::min(1.0f, spitze));
    }
    std::tie(bild.rate, bild.kanaele) = wavKopf(datei);
    return bild;
}

float waveSpitze(const WaveBild& bild, double von, double bis)
{
    if (bild.spitzen.empty() || bis < 0.0 || von >= bild.sekunden) return 0.0f;
    const double jeSpitze = static_cast<double>(WaveBild::kSpitzenFrames) / WaveBild::kBildRate;
    const auto i0 = static_cast<std::size_t>(std::max(0.0, von / jeSpitze));
    const auto i1 = std::min(bild.spitzen.size(), static_cast<std::size_t>(std::max(0.0, bis / jeSpitze)) + 1);
    float spitze = 0.0f;
    for (std::size_t i = i0; i < i1; ++i) spitze = std::max(spitze, bild.spitzen[i]);
    return spitze;
}

// --- die Flaeche ------------------------------------------------------------------------

WaveFlaeche::WaveFlaeche(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("waveArea"));
    setMouseTracking(true);
    setMinimumHeight(kLinealHoehe + 2 * kSpurHoehe + 120 + kAutotuneHoehe);
    setFocusPolicy(Qt::ClickFocus);
}

void WaveFlaeche::setMuster(const Muster& muster, const Loop& loop)
{
    m_muster = muster;
    // ein Schritt in ms — wie `waveKlangVon` ihn rechnet
    m_schrittMs = loop.bpm > 0.0 && loop.schritteJeSchlag > 0 ? 60000.0 / loop.bpm / loop.schritteJeSchlag : 0.0;
    m_schritteJeTakt = loop.schritteJeTakt();
    m_tempo = waveTempoFaktor(muster.wave, loop.bpm);
    if (m_zug == Zug::Keiner) m_vorher = muster;
    if (m_note >= static_cast<int>(m_muster.wave.autotune.abschnitte.size())) waehleNote(-1);
    berechneTonbereich();
    update();
}

void WaveFlaeche::setBild(WaveBild bild)
{
    m_bild = std::move(bild);
    berechneTonbereich();
    update();
}

void WaveFlaeche::setVerlauf(Tonhoehenverlauf verlauf)
{
    m_verlauf = std::move(verlauf);
    berechneTonbereich();
    update();
}

void WaveFlaeche::waehleNote(int note)
{
    const int neu = note >= 0 && note < static_cast<int>(m_muster.wave.autotune.abschnitte.size()) ? note : -1;
    if (neu == m_note) return;
    m_note = neu;
    update();
    Q_EMIT noteGewaehlt(m_note);
}

void WaveFlaeche::berechneTonbereich()
{
    // die gesungenen Toene im Ausschnitt und die Zielnoten; sonst um C4
    double unten = 1e9;
    double oben = -1e9;
    const double von = m_muster.wave.start;
    const double bis = ausschnittEnde() > 0.0 ? ausschnittEnde() : 1e9;
    for (std::size_t k = 0; k < m_verlauf.hz.size(); ++k)
    {
        const double t = m_verlauf.zeitS(k);
        if (m_verlauf.hz[k] <= 0.0f || t < von || t >= bis) continue;
        const double midi = midiVonHz(m_verlauf.hz[k]);
        unten = std::min(unten, midi);
        oben = std::max(oben, midi);
    }
    for (const AutotuneAbschnitt& a : m_muster.wave.autotune.abschnitte)
    {
        unten = std::min(unten, static_cast<double>(a.zielHoehe));
        oben = std::max(oben, static_cast<double>(a.zielHoehe));
    }
    if (unten > oben)
    {
        unten = 60.0 - kAutotuneTonMin / 2.0;
        oben = 60.0 + kAutotuneTonMin / 2.0;
    }
    int u = static_cast<int>(std::floor(unten)) - 1;
    int o = static_cast<int>(std::ceil(oben)) + 1;
    while (o - u < kAutotuneTonMin)
    {
        --u;
        ++o;
    }
    m_tonUnten = std::clamp(u, 0, 127);
    m_tonOben = std::clamp(o, m_tonUnten + 1, 127);
}

double WaveFlaeche::yVonMidi(double midi) const
{
    const double oben = autotuneOben() + 4.0;
    const double unten = autotuneOben() + kAutotuneHoehe - 4.0;
    return unten - (midi - m_tonUnten) / (m_tonOben - m_tonUnten) * (unten - oben);
}

double WaveFlaeche::midiVonY(double y) const
{
    const double oben = autotuneOben() + 4.0;
    const double unten = autotuneOben() + kAutotuneHoehe - 4.0;
    return m_tonUnten + (unten - y) / (unten - oben) * (m_tonOben - m_tonUnten);
}

double WaveFlaeche::dateiVonX(double x) const
{
    return m_muster.wave.start + (schrittVonX(x) - m_muster.wave.versatz) * dateiJeSchritt();
}

bool WaveFlaeche::gesungenImAusschnitt() const
{
    return gesungenZwischen(m_muster.wave.start, ausschnittEnde() > 0.0 ? ausschnittEnde() : 1e9);
}

bool WaveFlaeche::gesungenZwischen(double von, double bis) const
{
    for (std::size_t k = 0; k < m_verlauf.hz.size(); ++k)
    {
        const double t = m_verlauf.zeitS(k);
        if (t >= von && t < bis && m_verlauf.hz[k] > 0.0f) return true;
    }
    return false;
}

int WaveFlaeche::noteBei(QPoint ort) const
{
    int note = -1;
    (void)notenGriffBei(ort, &note);
    return note;
}

WaveFlaeche::Zug WaveFlaeche::notenGriffBei(QPoint ort, int* note) const
{
    *note = -1;
    if (ort.y() < autotuneOben() || schrittMs() <= 0.0) return Zug::Keiner;
    const std::vector<AutotuneAbschnitt>& alle = m_muster.wave.autotune.abschnitte;
    const double halb = std::max(4.0, (yVonMidi(0.0) - yVonMidi(1.0)) / 2.0);
    for (std::size_t i = 0; i < alle.size(); ++i)
    {
        const double links = xVonDatei(alle[i].anfang);
        const double rechts = xVonDatei(alle[i].ende);
        const double y = yVonMidi(alle[i].zielHoehe + alle[i].zielCent / 100.0);
        if (std::abs(ort.y() - y) > halb || ort.x() < links - kGriff || ort.x() > rechts + kGriff) continue;
        *note = static_cast<int>(i);
        // die Kanten nur, wo die Note breit genug ist, dass auch die Mitte bleibt
        if (rechts - links > 3 * kGriff && std::abs(ort.x() - links) <= kGriff) return Zug::NoteAnfang;
        if (rechts - links > 3 * kGriff && std::abs(ort.x() - rechts) <= kGriff) return Zug::NoteEnde;
        return Zug::Note;
    }
    return Zug::NeueNote;
}

void WaveFlaeche::setMarke(double schritt)
{
    if (schritt == m_marke) return;
    m_marke = schritt;
    update();
}

void WaveFlaeche::setSchrittBreite(double pixel)
{
    const double neu = std::clamp(pixel, kSchrittBreiteMin, kSchrittBreiteMax);
    if (neu == m_schrittBreite) return;
    m_schrittBreite = neu;
    update();
    Q_EMIT massGeaendert();
}

void WaveFlaeche::passeEin()
{
    m_roll = 0.0;
    setSchrittBreite(width() > 0 && schritte() > 0.0 ? (width() - 4.0) / schritte() : 12.0);
    update();
    Q_EMIT massGeaendert();
}

void WaveFlaeche::setRoll(double schritt)
{
    const double neu = std::clamp(schritt, 0.0, std::max(0.0, schritte() - width() / m_schrittBreite));
    if (neu == m_roll) return;
    m_roll = neu;
    update();
}

double WaveFlaeche::schritte() const
{
    return static_cast<double>(m_muster.takte) * m_schritteJeTakt;
}

double WaveFlaeche::xVonSchritt(double schritt) const
{
    return (schritt - m_roll) * m_schrittBreite;
}

double WaveFlaeche::schrittVonX(double x) const
{
    return x / m_schrittBreite + m_roll;
}

double WaveFlaeche::schrittVonDatei(double sekunde) const
{
    const double jeSchritt = dateiJeSchritt();
    if (jeSchritt <= 0.0) return m_muster.wave.versatz;
    return m_muster.wave.versatz + (sekunde - m_muster.wave.start) / jeSchritt;
}

double WaveFlaeche::ausschnittEnde() const
{
    if (m_muster.wave.ende > 0.0) return m_bild.sekunden > 0.0 ? std::min(m_muster.wave.ende, m_bild.sekunden) : m_muster.wave.ende;
    return m_bild.sekunden;
}

double WaveFlaeche::spurUnten(Steuerspur spur)
{
    return spur == Steuerspur::Pitch ? -kPitchBereich : 0.0;
}

double WaveFlaeche::spurOberGrenze(Steuerspur spur)
{
    return spur == Steuerspur::Pitch ? kPitchBereich : 1.0;
}

Steuerspur WaveFlaeche::spurBei(int y, bool* getroffen) const
{
    const bool drin = y >= spurOben(Steuerspur::Expression) && y < welleOben();
    if (getroffen != nullptr) *getroffen = drin;
    return y >= spurOben(Steuerspur::Pitch) ? Steuerspur::Pitch : Steuerspur::Expression;
}

double WaveFlaeche::yVonWert(Steuerspur spur, double wert) const
{
    const double oben = spurOben(spur) + 4.0;
    const double unten = spurOben(spur) + kSpurHoehe - 4.0;
    const double anteil = std::clamp((wert - spurUnten(spur)) / (spurOberGrenze(spur) - spurUnten(spur)), 0.0, 1.0);
    return unten - anteil * (unten - oben);
}

double WaveFlaeche::spurWertVonY(int y, Steuerspur spur, bool fein) const
{
    const double oben = spurOben(spur) + 4.0;
    const double unten = spurOben(spur) + kSpurHoehe - 4.0;
    const double anteil = std::clamp(1.0 - (y - oben) / (unten - oben), 0.0, 1.0);
    const double wert = spurUnten(spur) + anteil * (spurOberGrenze(spur) - spurUnten(spur));
    // Pitch rastet auf ganze Halbtoene, mit Shift auf Hundertstel (von Claude gesetzt, 15f)
    if (spur == Steuerspur::Pitch) return fein ? std::round(wert * 100.0) / 100.0 : std::round(wert);
    return wert;
}

int WaveFlaeche::spurPunktBei(QPoint ort, Steuerspur spur) const
{
    const std::vector<SpurPunkt>& punkte = m_muster.zeitspuren.von(spur);
    for (std::size_t i = 0; i < punkte.size(); ++i)
    {
        const double x = xVonSchritt(punkte[i].stelle);
        const double y = yVonWert(spur, punkte[i].wert);
        if (std::abs(x - ort.x()) <= kPunktFang && std::abs(y - ort.y()) <= kPunktFang) return static_cast<int>(i);
    }
    return -1;
}

WaveFlaeche::Zug WaveFlaeche::griffBei(QPoint ort) const
{
    if (ort.y() < welleOben() || ort.y() >= welleUnten() || m_muster.wave.datei.isEmpty()) return Zug::Keiner;
    const double links = xVonSchritt(schrittVonDatei(m_muster.wave.start));
    const double ende = ausschnittEnde();
    const double rechts = ende > 0.0 ? xVonSchritt(schrittVonDatei(ende)) : links;
    if (std::abs(ort.x() - links) <= kGriff) return Zug::Anfang;
    if (ende > 0.0 && std::abs(ort.x() - rechts) <= kGriff) return Zug::Ende;
    if (ort.x() > links && (ende <= 0.0 || ort.x() < rechts)) return Zug::Lage;
    return Zug::Keiner;
}

void WaveFlaeche::zeigeZeiger(QPoint ort)
{
    if (ort.y() >= autotuneOben())
    {
        int note = -1;
        switch (m_zug != Zug::Keiner ? m_zug : notenGriffBei(ort, &note))
        {
            case Zug::NoteAnfang:
            case Zug::NoteEnde: setCursor(Qt::SizeHorCursor); return;
            case Zug::Note: setCursor(Qt::SizeAllCursor); return;
            case Zug::NeueNote: setCursor(Qt::CrossCursor); return;
            default: unsetCursor(); return;
        }
    }
    switch (griffBei(ort))
    {
        case Zug::Anfang:
        case Zug::Ende: setCursor(Qt::SizeHorCursor); return;
        case Zug::Lage: setCursor(m_zug == Zug::Lage ? Qt::ClosedHandCursor : Qt::OpenHandCursor); return;
        default: break;
    }
    bool inSpur = false;
    const Steuerspur spur = spurBei(ort.y(), &inSpur);
    if (inSpur)
        setCursor(spurPunktBei(ort, spur) >= 0 ? Qt::SizeAllCursor : Qt::CrossCursor);
    else
        unsetCursor();
}

void WaveFlaeche::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QPalette& farben = palette();
    p.fillRect(rect(), farben.base());
    const double alle = schritte();
    const int breite = width();

    // --- das Taktlineal: Takte mit Nummer, Schlaege als kurze Striche
    p.fillRect(QRect(0, 0, breite, kLinealHoehe), farben.window());
    p.setPen(farben.color(QPalette::Mid));
    for (int s = static_cast<int>(std::floor(m_roll)); s <= static_cast<int>(alle); ++s)
    {
        const double x = xVonSchritt(s);
        if (x > breite) break;
        const bool takt = m_schritteJeTakt > 0 && s % m_schritteJeTakt == 0;
        if (!takt && m_schrittBreite < 4.0) continue;
        p.drawLine(QPointF(x, takt ? 2.0 : kLinealHoehe - 5.0), QPointF(x, kLinealHoehe));
        if (takt && s < alle)
        {
            p.setPen(farben.color(QPalette::WindowText));
            p.drawText(QPointF(x + 3.0, kLinealHoehe - 5.0), QString::number(s / m_schritteJeTakt + 1));
            p.setPen(farben.color(QPalette::Mid));
        }
    }

    // --- die Spuren Expression und Pitch (Pitch seit 15f)
    for (const Steuerspur steuer : kFlaechenSpuren)
    {
        const QRectF spur(0, spurOben(steuer), breite, kSpurHoehe);
        p.fillRect(spur, farben.alternateBase());
        p.setPen(farben.color(QPalette::Midlight));
        p.drawLine(spur.bottomLeft(), spur.bottomRight());
        const std::vector<SpurPunkt>& punkte = m_muster.zeitspuren.von(steuer);
        const auto yVon = [&](double wert) { return yVonWert(steuer, wert); };
        // der Name unten links: dort liegt die Kurve nur bei sehr leiser Expression oder tiefem Pitch
        p.setPen(farben.color(QPalette::PlaceholderText));
        p.drawText(QPointF(4.0, spurOben(steuer) + kSpurHoehe - 4.0), spurName(steuer));
        const double ohne = steuer == Steuerspur::Pitch ? 0.0 : 1.0;  // der Wert ohne Punkte
        if (punkte.empty())
        {
            QPen strich(farben.color(QPalette::Mid), 1.0, Qt::DashLine);
            p.setPen(strich);
            p.drawLine(QPointF(0, yVon(ohne)), QPointF(breite, yVon(ohne)));
            continue;
        }
        if (steuer == Steuerspur::Pitch)
        {
            // die Null als feine Linie: darueber hoeher, darunter tiefer
            p.setPen(QPen(farben.color(QPalette::Mid), 1.0, Qt::DotLine));
            p.drawLine(QPointF(0, yVon(0.0)), QPointF(breite, yVon(0.0)));
        }
        QPainterPath kurve;
        kurve.moveTo(xVonSchritt(0.0), yVon(punkte.front().wert));
        for (const SpurPunkt& punkt : punkte) kurve.lineTo(xVonSchritt(punkt.stelle), yVon(punkt.wert));
        kurve.lineTo(xVonSchritt(alle), yVon(punkte.back().wert));
        p.setPen(QPen(farben.color(QPalette::Highlight), 1.5));
        p.drawPath(kurve);
        p.setBrush(farben.color(QPalette::Highlight));
        for (const SpurPunkt& punkt : punkte) p.drawEllipse(QPointF(xVonSchritt(punkt.stelle), yVon(punkt.wert)), 3.0, 3.0);
        p.setBrush(Qt::NoBrush);
    }

    // --- die Welle
    const int wOben = welleOben();
    const int wUnten = welleUnten();
    const double mitte = (wOben + wUnten) / 2.0;
    const double halb = (wUnten - wOben) / 2.0 - 4.0;
    p.setPen(farben.color(QPalette::Mid));
    p.drawLine(QPointF(0, mitte), QPointF(breite, mitte));
    const double ms = schrittMs();
    const double ende = ausschnittEnde();
    if (!m_bild.spitzen.empty() && ms > 0.0)
    {
        const QColor hell = farben.color(QPalette::Highlight);
        // der Rest der Datei: deutlich zu sehen, aber matt — in hellem und dunklem Thema
        QColor grau = farben.color(QPalette::WindowText);
        grau.setAlpha(80);
        for (int x = 0; x < breite; ++x)
        {
            // die Sekunden der Datei unter dieser Spalte
            const double s0 = schrittVonX(x);
            const double s1 = schrittVonX(x + 1);
            const double f0 = m_muster.wave.start + (s0 - m_muster.wave.versatz) * dateiJeSchritt();
            const double f1 = m_muster.wave.start + (s1 - m_muster.wave.versatz) * dateiJeSchritt();
            if (f1 < 0.0 || f0 >= m_bild.sekunden) continue;
            const float spitze = waveSpitze(m_bild, f0, f1);
            const bool imAusschnitt = f0 >= m_muster.wave.start && f0 < ende;
            const bool imMuster = s0 >= 0.0 && s0 < alle;
            p.setPen(imAusschnitt && imMuster ? hell : grau);
            p.drawLine(QPointF(x + 0.5, mitte - spitze * halb), QPointF(x + 0.5, mitte + spitze * halb));
        }
    }
    else if (!m_muster.wave.datei.isEmpty())
    {
        p.setPen(farben.color(QPalette::PlaceholderText));
        p.drawText(QRectF(0, wOben, breite, wUnten - wOben), Qt::AlignCenter, tr("The file could not be read"));
    }
    else
    {
        p.setPen(farben.color(QPalette::PlaceholderText));
        p.drawText(QRectF(0, wOben, breite, wUnten - wOben), Qt::AlignCenter, tr("No file — choose one with Browse..."));
    }
    // ausserhalb des Musters schraffiert: dort ist nichts zu hoeren
    const QBrush schraffur(farben.color(QPalette::Mid), Qt::BDiagPattern);
    const double xAnfang = xVonSchritt(0.0);
    const double xEnde = xVonSchritt(alle);
    if (xAnfang > 0.0) p.fillRect(QRectF(0, wOben, xAnfang, wUnten - wOben), schraffur);
    if (xEnde < breite) p.fillRect(QRectF(xEnde, wOben, breite - xEnde, wUnten - wOben), schraffur);
    // die Kanten des Ausschnitts mit ihren Griffen
    if (!m_muster.wave.datei.isEmpty() && ms > 0.0)
    {
        p.setPen(QPen(farben.color(QPalette::WindowText), 1.5));
        const auto kante = [&](double x) {
            p.drawLine(QPointF(x, wOben), QPointF(x, wUnten));
            p.fillRect(QRectF(x - 3.0, wOben, 6.0, 8.0), farben.color(QPalette::WindowText));
        };
        kante(xVonSchritt(schrittVonDatei(m_muster.wave.start)));
        if (ende > 0.0) kante(xVonSchritt(schrittVonDatei(ende)));
    }

    // --- die Spur Autotune (Stufe 15i)
    zeichneAutotune(p, QRectF(0, wUnten, breite, kAutotuneHoehe));

    // --- die Abspielmarke
    if (m_marke >= 0.0)
    {
        const double x = xVonSchritt(m_marke);
        p.setPen(QPen(QColor(0xe0, 0x50, 0x50), 1.5));
        p.drawLine(QPointF(x, 0), QPointF(x, height()));
    }
}

void WaveFlaeche::zeichneAutotune(QPainter& p, const QRectF& spur)
{
    const QPalette& farben = palette();
    p.save();
    p.setClipRect(spur);
    p.fillRect(spur, farben.alternateBase());
    p.setPen(farben.color(QPalette::Midlight));
    p.drawLine(spur.topLeft(), spur.topRight());
    const double breite = spur.width();
    // die Halbtoene als feine Linien, je C mit Namen
    for (int ton = m_tonUnten; ton <= m_tonOben; ++ton)
    {
        const double y = yVonMidi(ton);
        QColor linie = farben.color(QPalette::Mid);
        linie.setAlpha(ton % 12 == 0 ? 160 : 50);
        p.setPen(linie);
        p.drawLine(QPointF(0, y), QPointF(breite, y));
        if (ton % 12 == 0)
        {
            p.setPen(farben.color(QPalette::PlaceholderText));
            p.drawText(QPointF(2.0, y - 2.0), tonName(ton));
        }
    }
    const double ms = schrittMs();
    if (ms <= 0.0 || m_muster.wave.datei.isEmpty())
    {
        p.restore();
        return;
    }
    // die Zielnoten als Kaesten, einen Halbton hoch
    const double hoehe = std::max(8.0, yVonMidi(0.0) - yVonMidi(1.0));
    const std::vector<AutotuneAbschnitt>& alle = m_muster.wave.autotune.abschnitte;
    for (std::size_t i = 0; i < alle.size(); ++i)
    {
        const AutotuneAbschnitt& a = alle[i];
        const QRectF kasten(QPointF(xVonDatei(a.anfang), yVonMidi(a.zielHoehe + a.zielCent / 100.0) - hoehe / 2.0),
                            QPointF(xVonDatei(a.ende), yVonMidi(a.zielHoehe + a.zielCent / 100.0) + hoehe / 2.0));
        if (kasten.right() < 0.0 || kasten.left() > breite) continue;
        // hell gefuellt, damit die Kurven darin zu sehen bleiben; die Staerke als Deckkraft
        const bool gewaehlt = static_cast<int>(i) == m_note;
        QColor fuellung = farben.color(QPalette::Highlight);
        fuellung.setAlpha((gewaehlt ? 90 : 35) + static_cast<int>(50 * a.staerke));
        p.fillRect(kasten, fuellung);
        p.setPen(QPen(farben.color(gewaehlt ? QPalette::WindowText : QPalette::Highlight), gewaehlt ? 2.0 : 1.0));
        p.drawRect(kasten);
        // der Name ueber dem Kasten: im Kasten (ein Halbton hoch) waere er nicht zu lesen
        if (kasten.width() > 40.0)
        {
            p.setPen(farben.color(gewaehlt ? QPalette::WindowText : QPalette::PlaceholderText));
            p.drawText(QPointF(kasten.left() + 2.0, kasten.top() - 3.0), notenText(a));
        }
    }
    if (m_verlauf.hz.empty())
    {
        p.setPen(farben.color(QPalette::PlaceholderText));
        p.drawText(spur.adjusted(40, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft,
                   tr("Autotune — the sung pitch is not measured yet"));
        p.restore();
        return;
    }
    // gemessen, aber nichts gesungen: Zielnoten taeten nichts — das sagen, statt still zu bleiben
    if (!gesungenImAusschnitt())
    {
        p.setPen(farben.color(QPalette::WindowText));
        p.drawText(spur.adjusted(40, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, keinGesangText());
    }
    // die gesungene Tonhoehe gestrichelt, die korrigierte fein darueber
    const std::vector<double> korrektur = autotuneKorrektur(alle, m_verlauf);
    QPainterPath gesungen;
    QPainterPath korrigiert;
    bool offen = false;
    bool korrOffen = false;
    for (std::size_t k = 0; k < m_verlauf.hz.size(); ++k)
    {
        const double x = xVonDatei(m_verlauf.zeitS(k));
        if (m_verlauf.hz[k] <= 0.0f || x < -10.0 || x > breite + 10.0)
        {
            offen = korrOffen = false;
            continue;
        }
        const double midi = midiVonHz(m_verlauf.hz[k]);
        const QPointF punkt(x, yVonMidi(midi));
        offen ? gesungen.lineTo(punkt) : gesungen.moveTo(punkt);
        offen = true;
        if (std::abs(korrektur[k]) > 1e-3)
        {
            const QPointF ziel(x, yVonMidi(midi + korrektur[k]));
            korrOffen ? korrigiert.lineTo(ziel) : korrigiert.moveTo(ziel);
            korrOffen = true;
        }
        else
            korrOffen = false;
    }
    p.setPen(QPen(farben.color(QPalette::WindowText), 1.0, Qt::DashLine));
    p.drawPath(gesungen);
    p.setPen(QPen(QColor(0xe0, 0x90, 0x30), 1.5));
    p.drawPath(korrigiert);
    p.restore();
}

void WaveFlaeche::mousePressEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (event->button() != Qt::LeftButton) return QWidget::mousePressEvent(event);
    m_vorher = m_muster;
    m_zugSchritt = schrittVonX(ort.x());
    if (ort.y() >= autotuneOben())
    {
        // Stufe 15i: eine Zielnote waehlen, teilen, ziehen — oder eine neue aufziehen
        if (schrittMs() <= 0.0 || m_muster.wave.datei.isEmpty()) return;
        int note = -1;
        m_zug = notenGriffBei(ort, &note);
        m_zugSekunde = dateiVonX(ort.x());
        m_zugMidi = midiVonY(ort.y());
        Autotune& at = m_muster.wave.autotune;
        if ((event->modifiers() & Qt::ControlModifier) != 0)
        {
            m_zug = Zug::Keiner;
            if (note >= 0 && teileAbschnitt(at, m_zugSekunde)) melde(tr("Split note"));
            return;
        }
        if (m_zug == Zug::NeueNote)
        {
            const AutotuneAbschnitt neu{m_zugSekunde, m_zugSekunde, std::clamp(static_cast<int>(std::lround(m_zugMidi)), 0, 127),
                                        0, at.staerke, at.speedMs};
            const auto platz = std::lower_bound(at.abschnitte.begin(), at.abschnitte.end(), neu.anfang,
                                                [](const AutotuneAbschnitt& a, double t) { return a.anfang < t; });
            note = static_cast<int>(at.abschnitte.insert(platz, neu) - at.abschnitte.begin());
        }
        waehleNote(note);
        zeigeZeiger(ort);
        return;
    }
    if (ort.y() < kLinealHoehe)
    {
        const int schritt = std::clamp(static_cast<int>(std::floor(m_zugSchritt)), 0, std::max(0, static_cast<int>(schritte()) - 1));
        Q_EMIT abspielstelleGewaehlt(schritt);
        return;
    }
    if (ort.y() < welleOben())
    {
        // eine Spur: einen Punkt greifen — oder einen setzen und weiterziehen
        m_spur = spurBei(ort.y());
        m_punkt = spurPunktBei(ort, m_spur);
        if (m_punkt < 0)
        {
            const bool frei = (event->modifiers() & Qt::AltModifier) != 0;
            const bool fein = (event->modifiers() & Qt::ShiftModifier) != 0;
            double stelle = std::clamp(frei ? m_zugSchritt : std::round(m_zugSchritt), 0.0, schritte());
            std::vector<SpurPunkt>& punkte = m_muster.zeitspuren.von(m_spur);
            // je Stelle ein Punkt: der neue ersetzt den dort
            punkte.erase(std::remove_if(punkte.begin(), punkte.end(), [stelle](const SpurPunkt& p) { return p.stelle == stelle; }),
                         punkte.end());
            const auto platz = std::lower_bound(punkte.begin(), punkte.end(), stelle,
                                                [](const SpurPunkt& p, double s) { return p.stelle < s; });
            m_punkt = static_cast<int>(punkte.insert(platz, SpurPunkt{stelle, spurWertVonY(ort.y(), m_spur, fein)}) - punkte.begin());
            update();
        }
        m_zug = Zug::Punkt;
        return;
    }
    m_zug = griffBei(ort);
    zeigeZeiger(ort);
}

void WaveFlaeche::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (m_zug == Zug::Keiner)
    {
        zeigeZeiger(ort);
        return;
    }
    if (m_zug == Zug::Punkt)
        zieheSpurPunkt(ort, (event->modifiers() & Qt::AltModifier) != 0, (event->modifiers() & Qt::ShiftModifier) != 0);
    else if (m_zug == Zug::Note || m_zug == Zug::NoteAnfang || m_zug == Zug::NoteEnde || m_zug == Zug::NeueNote)
        zieheNote(ort, (event->modifiers() & Qt::ShiftModifier) != 0);
    else
        zieheWelle(ort, (event->modifiers() & Qt::ShiftModifier) != 0);
}

void WaveFlaeche::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || m_zug == Zug::Keiner) return QWidget::mouseReleaseEvent(event);
    const Zug war = m_zug;
    m_zug = Zug::Keiner;
    m_punkt = -1;
    switch (war)
    {
        case Zug::Anfang: melde(tr("Trim wave start")); break;
        case Zug::Ende: melde(tr("Trim wave end")); break;
        case Zug::Lage: melde(tr("Move wave")); break;
        case Zug::Punkt: melde(m_spur == Steuerspur::Pitch ? tr("Change pitch") : tr("Change expression")); break;
        case Zug::Note: melde(tr("Move note")); break;
        case Zug::NoteAnfang:
        case Zug::NoteEnde: melde(tr("Trim note")); break;
        case Zug::NeueNote:
        {
            // ohne Laenge (ein Klick auf freie Flaeche): keine Note, keine Wahl
            std::vector<AutotuneAbschnitt>& alle = m_muster.wave.autotune.abschnitte;
            if (m_note >= 0 && m_note < static_cast<int>(alle.size()) &&
                alle[static_cast<std::size_t>(m_note)].ende - alle[static_cast<std::size_t>(m_note)].anfang < kNoteMin)
            {
                alle.erase(alle.begin() + m_note);
                waehleNote(-1);
            }
            melde(tr("Add note"));
            break;
        }
        case Zug::Keiner: break;
    }
    zeigeZeiger(event->position().toPoint());
}

void WaveFlaeche::zieheNote(QPoint ort, bool fein)
{
    std::vector<AutotuneAbschnitt>& alle = m_muster.wave.autotune.abschnitte;
    const std::vector<AutotuneAbschnitt>& vorher = m_vorher.wave.autotune.abschnitte;
    if (m_note < 0 || m_note >= static_cast<int>(alle.size())) return;
    AutotuneAbschnitt& a = alle[static_cast<std::size_t>(m_note)];
    const double sekunde = dateiVonX(ort.x());
    switch (m_zug)
    {
        case Zug::Note:
        {
            if (m_note >= static_cast<int>(vorher.size())) return;
            const AutotuneAbschnitt& alt = vorher[static_cast<std::size_t>(m_note)];
            const double weg = std::max(sekunde - m_zugSekunde, -alt.anfang);
            a.anfang = alt.anfang + weg;
            a.ende = alt.ende + weg;
            const double ton = midiVonY(ort.y()) - m_zugMidi;
            if (fein)
            {
                // in Cent: der Ton nimmt auf, was ueber einen halben Halbton hinausgeht
                const auto cent = alt.zielHoehe * 100 + alt.zielCent + static_cast<int>(std::lround(ton * 100.0));
                a.zielHoehe = static_cast<int>(std::lround(cent / 100.0));
                a.zielCent = cent - 100 * a.zielHoehe;
            }
            else
            {
                a.zielHoehe = alt.zielHoehe + static_cast<int>(std::lround(ton));
                a.zielCent = alt.zielCent;
            }
            a.zielHoehe = std::clamp(a.zielHoehe, 0, 127);
            break;
        }
        case Zug::NoteAnfang: a.anfang = std::clamp(sekunde, 0.0, a.ende - kNoteMin); break;
        case Zug::NoteEnde: a.ende = std::max(sekunde, a.anfang + kNoteMin); break;
        case Zug::NeueNote:
            a.anfang = std::max(0.0, std::min(m_zugSekunde, sekunde));
            a.ende = std::max(m_zugSekunde, sekunde);
            break;
        default: break;
    }
    update();
}

void WaveFlaeche::mouseDoubleClickEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (ort.y() >= autotuneOben())
    {
        // Stufe 15i: ein Doppelklick auf eine Zielnote nimmt sie weg
        const int note = noteBei(ort);
        if (note < 0) return QWidget::mouseDoubleClickEvent(event);
        m_zug = Zug::Keiner;
        m_vorher = m_muster;
        m_muster.wave.autotune.abschnitte.erase(m_muster.wave.autotune.abschnitte.begin() + note);
        waehleNote(-1);
        melde(tr("Delete note"));
        return;
    }
    bool inSpur = false;
    const Steuerspur spur = spurBei(ort.y(), &inSpur);
    const int punkt = inSpur ? spurPunktBei(ort, spur) : -1;
    if (punkt < 0) return QWidget::mouseDoubleClickEvent(event);
    m_zug = Zug::Keiner;
    m_vorher = m_muster;
    std::vector<SpurPunkt>& punkte = m_muster.zeitspuren.von(spur);
    // der erste Klick des Doppelklicks hat ihn gegriffen; der Doppelklick nimmt ihn weg
    punkte.erase(punkte.begin() + punkt);
    melde(tr("Delete point"));
}

void WaveFlaeche::notenMenue(QContextMenuEvent* event)
{
    const QPoint ort = event->pos();
    const int note = noteBei(ort);
    if (note >= 0) waehleNote(note);
    Autotune& at = m_muster.wave.autotune;
    QMenu menue(this);
    menue.setObjectName(QStringLiteral("waveAutotuneMenu"));
    QAction* teilen = menue.addAction(tr("Split here"));
    teilen->setEnabled(note >= 0);
    QAction* verbinden = menue.addAction(tr("Join with next"));
    verbinden->setEnabled(note >= 0 && note + 1 < static_cast<int>(at.abschnitte.size()));
    QAction* weg = menue.addAction(tr("Delete note"));
    weg->setEnabled(note >= 0);
    menue.addSeparator();
    QAction* leeren = menue.addAction(tr("Clear notes"));
    leeren->setEnabled(!at.abschnitte.empty());
    QAction* gewaehlt = lumi::ui::zeigeMenue(menue, event->globalPos());
    if (gewaehlt == nullptr) return;
    m_vorher = m_muster;
    if (gewaehlt == teilen)
    {
        if (teileAbschnitt(at, dateiVonX(ort.x()))) melde(tr("Split note"));
    }
    else if (gewaehlt == verbinden)
    {
        if (verbindeAbschnitte(at, static_cast<std::size_t>(note))) melde(tr("Join notes"));
    }
    else if (gewaehlt == weg)
    {
        at.abschnitte.erase(at.abschnitte.begin() + note);
        waehleNote(-1);
        melde(tr("Delete note"));
    }
    else if (gewaehlt == leeren)
    {
        at.abschnitte.clear();
        waehleNote(-1);
        melde(tr("Clear notes"));
    }
}

void WaveFlaeche::keyPressEvent(QKeyEvent* event)
{
    // Stufe 15i: Entf nimmt die gewaehlte Zielnote weg
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && m_note >= 0)
    {
        m_vorher = m_muster;
        m_muster.wave.autotune.abschnitte.erase(m_muster.wave.autotune.abschnitte.begin() + m_note);
        waehleNote(-1);
        melde(tr("Delete note"));
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void WaveFlaeche::contextMenuEvent(QContextMenuEvent* event)
{
    const QPoint ort = event->pos();
    if (ort.y() >= autotuneOben())
    {
        notenMenue(event);
        return;
    }
    bool inSpur = false;
    const Steuerspur spur = spurBei(ort.y(), &inSpur);
    if (!inSpur) return;
    const int punkt = spurPunktBei(ort, spur);
    QMenu menue(this);
    menue.setObjectName(QStringLiteral("waveLaneMenu"));
    QAction* weg = menue.addAction(tr("Delete point"));
    weg->setEnabled(punkt >= 0);
    QAction* leeren = menue.addAction(tr("Clear lane"));
    leeren->setEnabled(!m_muster.zeitspuren.von(spur).empty());
    QAction* gewaehlt = lumi::ui::zeigeMenue(menue, event->globalPos());
    if (gewaehlt == nullptr) return;
    m_vorher = m_muster;
    std::vector<SpurPunkt>& punkte = m_muster.zeitspuren.von(spur);
    if (gewaehlt == weg)
    {
        punkte.erase(punkte.begin() + punkt);
        melde(tr("Delete point"));
    }
    else if (gewaehlt == leeren)
    {
        punkte.clear();
        melde(tr("Clear lane"));
    }
}

void WaveFlaeche::wheelEvent(QWheelEvent* event)
{
    const double stufen = event->angleDelta().y() / 120.0;
    if ((event->modifiers() & Qt::ControlModifier) != 0)
    {
        // Ctrl+Rad: die Zeit um die Stelle unter der Maus (wie in den anderen Editoren)
        const double unter = schrittVonX(event->position().x());
        setSchrittBreite(m_schrittBreite * std::pow(1.25, stufen));
        setRoll(unter - event->position().x() / m_schrittBreite);
        Q_EMIT massGeaendert();
        event->accept();
        return;
    }
    setRoll(m_roll - stufen * 4.0 * std::max(1.0, 12.0 / m_schrittBreite));
    Q_EMIT massGeaendert();
    event->accept();
}

bool WaveFlaeche::event(QEvent* event)
{
    if (event->type() != QEvent::ToolTip) return QWidget::event(event);
    const auto* hilfe = static_cast<QHelpEvent*>(event);
    const QPoint ort = hilfe->pos();
    QString text;
    if (ort.y() < kLinealHoehe)
        text = tr("Click: Play pattern starts here");
    else if (ort.y() < welleOben())
    {
        const Steuerspur spur = spurBei(ort.y());
        const std::vector<SpurPunkt>& punkte = m_muster.zeitspuren.von(spur);
        const int punkt = spurPunktBei(ort, spur);
        if (punkt >= 0)
            text = tr("%1: %2 — drag to move, double-click to delete")
                       .arg(spurName(spur), spurText(spur, punkte[static_cast<std::size_t>(punkt)].wert));
        else if (spur == Steuerspur::Pitch)
            text = punkte.empty() ? tr("Pitch: as recorded — click to add a point in whole semitones (Shift: hundredths, "
                                       "Alt: off the grid)")
                                  : tr("Pitch: click to add a point in whole semitones (Shift: hundredths, Alt: off the "
                                       "grid), right-click: Clear lane");
        else
            text = punkte.empty() ? tr("Expression: 1 throughout — click to add a point on the step (Alt: off the grid)")
                                  : tr("Expression: click to add a point on the step (Alt: off the grid), right-click: Clear lane");
    }
    else if (ort.y() < welleUnten())
    {
        switch (griffBei(ort))
        {
            case Zug::Anfang: text = tr("Start of the section: %1 s in the file — drag; the sound stays where it is").arg(m_muster.wave.start, 0, 'f', 3); break;
            case Zug::Ende: text = tr("End of the section: %1 s in the file — drag").arg(ausschnittEnde(), 0, 'f', 3); break;
            case Zug::Lage: text = tr("Drag to move the wave in steps (Shift: in fractions of a step)"); break;
            case Zug::Keiner:
            case Zug::Punkt:  // griffBei liefert diese nie
            case Zug::Note:
            case Zug::NoteAnfang:
            case Zug::NoteEnde:
            case Zug::NeueNote:
                text = tr("Grey: outside the section · hatched: outside the pattern — not heard");
                break;
        }
    }
    else
    {
        const int note = noteBei(ort);
        if (note >= 0)
            text = tr("%1 — drag to move (Shift: in cents), drag an edge to trim, Ctrl+click to split, double-click "
                      "or Del to delete")
                       .arg(notenText(m_muster.wave.autotune.abschnitte[static_cast<std::size_t>(note)]));
        else
            text = tr("Autotune: dashed the sung pitch, orange the corrected — drag to add a target note, or use "
                      "Detect notes below");
    }
    QToolTip::showText(hilfe->globalPos(), text, this);
    return true;
}

void WaveFlaeche::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    setRoll(m_roll);
    Q_EMIT massGeaendert();
}

void WaveFlaeche::zieheWelle(QPoint ort, bool frei)
{
    const double jeSchritt = dateiJeSchritt();  // mit Follow tempo gedehnt (15j)
    if (jeSchritt <= 0.0) return;
    const double schritt = schrittVonX(ort.x());
    WaveAngaben& w = m_muster.wave;
    const WaveAngaben& alt = m_vorher.wave;
    // die Sekunde der Datei unter der Maus, wie das Muster vor dem Zug lag
    const double sekunde = alt.start + (schritt - alt.versatz) * jeSchritt;
    const double ende = alt.ende > 0.0 ? alt.ende : (m_bild.sekunden > 0.0 ? m_bild.sekunden : 0.0);
    switch (m_zug)
    {
        case Zug::Anfang:
        {
            // der Ton bleibt, wo er ist: mit dem Anfang rueckt die Lage
            const double bis = ende > 0.0 ? ende - kAusschnittMin : sekunde;
            const double neu = std::clamp(sekunde, 0.0, std::max(0.0, bis));
            w.start = neu;
            w.versatz = alt.versatz + (neu - alt.start) / jeSchritt;
            break;
        }
        case Zug::Ende:
        {
            double neu = std::max(sekunde, alt.start + kAusschnittMin);
            if (m_bild.sekunden > 0.0 && neu >= m_bild.sekunden) neu = 0.0;  // bis zum Ende der Datei
            w.ende = neu;
            break;
        }
        case Zug::Lage:
        {
            double neu = alt.versatz + (schritt - m_zugSchritt);
            if (!frei) neu = std::round(neu);  // rastet in Schritten, mit Shift Bruchteile
            w.versatz = std::clamp(neu, -schritte(), schritte());
            break;
        }
        default: break;
    }
    update();
}

void WaveFlaeche::zieheSpurPunkt(QPoint ort, bool frei, bool fein)
{
    std::vector<SpurPunkt>& punkte = m_muster.zeitspuren.von(m_spur);
    if (m_punkt < 0 || m_punkt >= static_cast<int>(punkte.size())) return;
    const auto i = static_cast<std::size_t>(m_punkt);
    // zwischen seinen Nachbarn, im Muster
    const double links = i > 0 ? punkte[i - 1].stelle : 0.0;
    const double rechts = i + 1 < punkte.size() ? punkte[i + 1].stelle : schritte();
    const double roh = schrittVonX(ort.x());
    double stelle = frei ? roh : std::round(roh);
    stelle = std::clamp(stelle, links, rechts);
    punkte[i] = SpurPunkt{stelle, spurWertVonY(ort.y(), m_spur, fein)};
    update();
}

void WaveFlaeche::melde(const QString& was)
{
    // je Stelle ein Punkt, in den Grenzen — wie `richte` es hielte
    for (const Steuerspur spur : kFlaechenSpuren)
        richtePunkte(m_muster.zeitspuren.von(spur), spurMin(spur), spurMax(spur), schritte());
    // die Zielnoten gerichtet; die gewaehlte bleibt gewaehlt, auch wenn sie den Platz wechselt
    std::vector<AutotuneAbschnitt>& alle = m_muster.wave.autotune.abschnitte;
    const std::optional<AutotuneAbschnitt> gewaehlt =
        m_note >= 0 && m_note < static_cast<int>(alle.size()) ? std::optional(alle[static_cast<std::size_t>(m_note)]) : std::nullopt;
    richteAutotune(m_muster.wave.autotune);
    if (gewaehlt)
    {
        int neu = -1;
        for (std::size_t i = 0; i < alle.size(); ++i)
            if (alle[i].zielHoehe == gewaehlt->zielHoehe && alle[i].anfang < gewaehlt->ende && alle[i].ende > gewaehlt->anfang)
            {
                neu = static_cast<int>(i);
                break;
            }
        m_note = -2;  // erzwingt die Meldung, auch wenn der Platz gleich bleibt
        waehleNote(neu);
    }
    if (m_muster == m_vorher) return;
    m_vorher = m_muster;
    Q_EMIT geaendert(m_muster, was);
}

// --- das Fenster ------------------------------------------------------------------------

/// Stufe 15i: der Thread schreibt `verlauf` und setzt danach `fertig`; gelesen wird erst nach
/// `join` (wie die Messung im Instrument-Creator, CR1 a).
struct WaveEditor::Messung
{
    std::thread faden;
    std::atomic<bool> fertig{false};
    QString stand;  ///< fuer welche Datei und welchen Stand
    std::optional<Tonhoehenverlauf> verlauf;
};

WaveEditor::WaveEditor(QWidget* parent) : QWidget(parent, Qt::Window)
{
    setObjectName(QStringLiteral("waveEditor"));
    setupUI();
}

WaveEditor::~WaveEditor()
{
    // eine Messung laesst sich nicht abbrechen; was sie misst, ist danach gemerkt
    if (m_messung != nullptr && m_messung->faden.joinable()) m_messung->faden.join();
}

void WaveEditor::setzeLageMerken(bool an)
{
    lageMerken() = an;
}

void WaveEditor::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    // --- Kopf
    auto* kopf = new QHBoxLayout;
    m_basisName = new QLabel(this);
    m_basisName->setObjectName(QStringLiteral("waveBase"));
    m_nutzer = new QLabel(this);
    m_nutzer->setObjectName(QStringLiteral("waveUsers"));
    m_nutzer->setEnabled(false);
    m_takte = new QSpinBox(this);
    m_takte->setObjectName(QStringLiteral("waveBars"));
    m_takte->setRange(kTakteMin, kWaveTakteMax);
    m_takte->setKeyboardTracking(false);
    m_takte->setToolTip(tr("Length of the pattern in bars; every repeat starts the section anew"));
    m_play = new QPushButton(tr("Play pattern"), this);
    m_play->setObjectName(QStringLiteral("wavePlay"));
    m_play->setToolTip(tr("Play only this pattern, in a loop"));
    m_spitze = new QLabel(QStringLiteral("–"), this);
    m_spitze->setObjectName(QStringLiteral("wavePeak"));
    m_spitze->setMinimumWidth(36);
    m_spitze->setToolTip(tr("Peak of the mixed loop while it plays; above 1 it clips"));
    kopf->addWidget(new QLabel(tr("Base"), this));
    kopf->addWidget(m_basisName);
    kopf->addWidget(m_nutzer);
    kopf->addStretch(1);
    auto* fit = new QPushButton(tr("Fit"), this);
    fit->setObjectName(QStringLiteral("waveFit"));
    fit->setToolTip(tr("The whole pattern in width") + QStringLiteral("\n") +
                    lumi::ui::Achse::zoomGesten().mid(0, 2).join(QLatin1Char('\n')));
    kopf->addWidget(fit);
    kopf->addSpacing(8);
    kopf->addWidget(new QLabel(tr("Bars"), this));
    kopf->addWidget(m_takte);
    kopf->addWidget(m_play);
    m_fx = new QPushButton(QStringLiteral("FX"), this);
    m_fx->setObjectName(QStringLiteral("waveFx"));
    m_fx->setCheckable(true);
    m_fx->setMaximumWidth(fontMetrics().horizontalAdvance(QStringLiteral("FX")) + 28);
    kopf->addWidget(m_fx);
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);

    // --- Zeile Datei
    auto* datei = new QHBoxLayout;
    m_datei = new QLabel(this);
    m_datei->setObjectName(QStringLiteral("waveFile"));
    m_datei->setMinimumWidth(130);
    {
        QFont fett = m_datei->font();
        fett.setBold(true);
        m_datei->setFont(fett);
    }
    m_dateiInfo = new QLabel(this);
    m_dateiInfo->setObjectName(QStringLiteral("waveFileInfo"));
    m_dateiInfo->setEnabled(false);
    m_suchen = new QPushButton(tr("Browse..."), this);
    m_suchen->setObjectName(QStringLiteral("waveBrowse"));
    m_suchen->setToolTip(tr("Choose an audio file for this pattern (WAV, MP3, FLAC, OGG)"));
    m_aufnehmen = new QPushButton(tr("Record..."), this);
    m_aufnehmen->setObjectName(QStringLiteral("waveRecord"));
    m_aufnehmen->setToolTip(tr("Record into this pattern while the loop plays — the take replaces the file"));
    connect(m_aufnehmen, &QPushButton::clicked, this, &WaveEditor::aufnahmeVerlangt);
    m_sampleBearbeiten = new QPushButton(tr("Edit sample..."), this);
    m_sampleBearbeiten->setObjectName(QStringLiteral("waveEditSample"));
    m_gain = new QDoubleSpinBox(this);
    m_gain->setObjectName(QStringLiteral("waveGain"));
    m_gain->setRange(kWaveGainMinDb, kWaveGainMaxDb);
    m_gain->setDecimals(1);
    m_gain->setSingleStep(0.5);
    m_gain->setSuffix(tr(" dB"));
    m_gain->setKeyboardTracking(false);
    m_gain->setToolTip(tr("Gain of the file in this pattern; at −60 dB it is silent"));
    m_fadeIn = new QSpinBox(this);
    m_fadeIn->setObjectName(QStringLiteral("waveFadeIn"));
    m_fadeOut = new QSpinBox(this);
    m_fadeOut->setObjectName(QStringLiteral("waveFadeOut"));
    for (QSpinBox* blende : {m_fadeIn, m_fadeOut})
    {
        blende->setRange(0, static_cast<int>(kWaveBlendeMaxMs));
        blende->setSuffix(tr(" ms"));
        blende->setKeyboardTracking(false);
    }
    m_fadeIn->setToolTip(tr("Fade in at the start of what is heard — the start of the section or of the pattern"));
    m_fadeOut->setToolTip(tr("Fade out at the end of what is heard — the end of the section, the file or the pattern"));
    datei->addWidget(new QLabel(tr("File"), this));
    if (const QIcon marke = lumi::ui::dateiArtIcon(QStringLiteral("wav")); !marke.isNull())
    {
        auto* icon = new QLabel(this);
        icon->setPixmap(marke.pixmap(18, 18));
        datei->addWidget(icon);
    }
    datei->addWidget(m_datei);
    datei->addWidget(m_dateiInfo);
    datei->addWidget(m_suchen);
    datei->addWidget(m_aufnehmen);
    datei->addWidget(m_sampleBearbeiten);
    datei->addSpacing(12);
    datei->addWidget(new QLabel(tr("Gain"), this));
    datei->addWidget(m_gain);
    datei->addWidget(new QLabel(tr("Fade in"), this));
    datei->addWidget(m_fadeIn);
    datei->addWidget(new QLabel(tr("Fade out"), this));
    datei->addWidget(m_fadeOut);
    m_hinweis = new QLabel(this);
    m_hinweis->setObjectName(QStringLiteral("waveHint"));
    m_hinweis->setEnabled(false);
    datei->addSpacing(12);
    datei->addWidget(m_hinweis);
    datei->addStretch(1);
    aussen->addLayout(datei);

    // --- Zeile Tempo
    auto* tempo = new QHBoxLayout;
    m_bezug = new QDoubleSpinBox(this);
    m_bezug->setObjectName(QStringLiteral("waveReferenceTempo"));
    m_bezug->setRange(kBpmMin, kBpmMax);
    m_bezug->setDecimals(2);
    m_bezug->setSuffix(tr(" BPM"));
    m_bezug->setKeyboardTracking(false);
    m_bezug->setToolTip(tr("The tempo the file was played in — Follow tempo stretches from it to the tempo of the loop"));
    m_bezugAufLoop = new QPushButton(tr("Set to loop tempo"), this);
    m_bezugAufLoop->setObjectName(QStringLiteral("waveSetToLoopTempo"));
    m_bezugAufLoop->setToolTip(tr("The reference tempo becomes the tempo of the loop now"));
    m_folgt = new QCheckBox(tr("Follow tempo"), this);
    m_folgt->setObjectName(QStringLiteral("waveFollowTempo"));
    m_folgt->setToolTip(tr("Stretch the file from the reference tempo to the tempo of the loop, without changing the "
                           "pitch — off: the file plays as recorded"));
    m_folgtHinweis = new QLabel(this);
    m_folgtHinweis->setObjectName(QStringLiteral("waveFollowHint"));
    m_folgtHinweis->setEnabled(false);
    m_tonhoehe = new QComboBox(this);
    m_tonhoehe->setObjectName(QStringLiteral("wavePitchMethod"));
    m_tonhoehe->addItem(tr("General"), static_cast<int>(TonhoehenArt::Allgemein));
    m_tonhoehe->addItem(tr("Voice"), static_cast<int>(TonhoehenArt::Stimme));
    m_tonhoehe->setToolTip(tr("How tempo and pitch are changed: General for any material, Voice for a single voice.\n"
                              "Voice keeps the formants of a single voice when the pitch moves; for anything else "
                              "use General"));
    tempo->addWidget(new QLabel(tr("Reference tempo"), this));
    tempo->addWidget(m_bezug);
    tempo->addWidget(m_bezugAufLoop);
    tempo->addSpacing(12);
    tempo->addWidget(m_folgt);
    tempo->addWidget(m_folgtHinweis);
    tempo->addSpacing(12);
    tempo->addWidget(new QLabel(tr("Pitch method"), this));
    tempo->addWidget(m_tonhoehe);
    tempo->addStretch(1);
    aussen->addLayout(tempo);

    // --- Zeile Raster: das Raster des Loops, ohne Swing offset (RS1-1 a)
    m_rasterZeile = new RasterZeile(QStringLiteral("wave"), this, false);
    aussen->addWidget(m_rasterZeile);

    // --- die Flaeche mit ihrem Rollbalken
    m_flaeche = new WaveFlaeche(this);
    m_rollX = new QScrollBar(Qt::Horizontal, this);
    m_rollX->setObjectName(QStringLiteral("waveScroll"));
    aussen->addWidget(m_flaeche, 1);
    aussen->addWidget(m_rollX);

    // --- Zeile Autotune (Stufe 15i, SK15-1 a): unter der Flaeche
    auto* autotune = new QHBoxLayout;
    m_grundton = new QComboBox(this);
    m_grundton->setObjectName(QStringLiteral("waveKey"));
    for (int ton = 0; ton < 12; ++ton) m_grundton->addItem(QString::fromLatin1(kTonNamen[static_cast<std::size_t>(ton)]), ton);
    m_grundton->setToolTip(tr("Key for Detect notes"));
    m_skala = new QComboBox(this);
    m_skala->setObjectName(QStringLiteral("waveScale"));
    m_skala->addItem(tr("Chromatic"), static_cast<int>(Skala::Chromatisch));
    m_skala->addItem(tr("Major"), static_cast<int>(Skala::Dur));
    m_skala->addItem(tr("Minor"), static_cast<int>(Skala::Moll));
    m_skala->setToolTip(tr("Scale for Detect notes — Chromatic: every semitone is a target"));
    m_erkennen = new QPushButton(tr("Detect notes"), this);
    m_erkennen->setObjectName(QStringLiteral("waveDetectNotes"));
    m_erkennen->setToolTip(tr("Target notes from the sung pitch, on the notes of the key — replaces the notes in the "
                              "section (or, with Selected section only, under the selected note)"));
    m_staerke = new QSpinBox(this);
    m_staerke->setObjectName(QStringLiteral("waveStrength"));
    m_staerke->setRange(0, 100);
    m_staerke->setSuffix(tr(" %"));
    m_staerke->setKeyboardTracking(false);
    m_staerke->setToolTip(tr("How far the pitch is pulled to the target — of the selected note, without one for new "
                             "notes"));
    m_speed = new QSpinBox(this);
    m_speed->setObjectName(QStringLiteral("waveSpeed"));
    m_speed->setRange(0, static_cast<int>(kAutotuneSpeedMaxMs));
    m_speed->setSuffix(tr(" ms"));
    m_speed->setKeyboardTracking(false);
    m_speed->setToolTip(tr("How fast the pitch reaches the target — 0 jumps (the hard sound); of the selected note, "
                           "without one for new notes"));
    m_nurGewaehlte = new QCheckBox(tr("Selected section only"), this);
    m_nurGewaehlte->setObjectName(QStringLiteral("waveSelectedOnly"));
    m_nurGewaehlte->setToolTip(tr("Detect notes only under the selected note"));
    m_autotuneHinweis = new QLabel(this);
    m_autotuneHinweis->setObjectName(QStringLiteral("waveAutotuneHint"));
    m_autotuneHinweis->setEnabled(false);
    m_stimmeNehmen = new QPushButton(tr("Use Voice"), this);
    m_stimmeNehmen->setObjectName(QStringLiteral("waveUseVoice"));
    m_stimmeNehmen->setToolTip(tr("Detect notes found a single voice: Voice keeps its formants when the pitch moves"));
    m_stimmeNehmen->hide();
    autotune->addWidget(new QLabel(tr("Autotune"), this));
    autotune->addWidget(new QLabel(tr("Key"), this));
    autotune->addWidget(m_grundton);
    autotune->addWidget(m_skala);
    autotune->addWidget(m_erkennen);
    autotune->addSpacing(12);
    autotune->addWidget(new QLabel(tr("Strength"), this));
    autotune->addWidget(m_staerke);
    autotune->addWidget(new QLabel(tr("Speed"), this));
    autotune->addWidget(m_speed);
    autotune->addWidget(m_nurGewaehlte);
    autotune->addSpacing(12);
    autotune->addWidget(m_autotuneHinweis);
    autotune->addWidget(m_stimmeNehmen);
    autotune->addStretch(1);
    aussen->addLayout(autotune);
    m_messFertig = new QTimer(this);
    m_messFertig->setInterval(100);
    connect(m_messFertig, &QTimer::timeout, this, &WaveEditor::sieheNachTonhoehe);

    m_hilfe = new QLabel(tr("Wave: drag an edge to trim the section · drag inside to move it in steps (Shift: fractions) · "
                            "Expression, Pitch: click to add a point (Alt: off the grid; Pitch in semitones, Shift: "
                            "hundredths), double-click deletes · "
                            "Autotune: drag to add a target note, drag a note to move it (Shift: cents), Ctrl+click "
                            "splits, Del deletes · "
                            "Ctrl+wheel: zoom · ruler: Play pattern starts there"),
                         this);
    m_hilfe->setObjectName(QStringLiteral("waveHelp"));
    m_hilfe->setEnabled(false);
    m_hilfe->setWordWrap(true);
    aussen->addWidget(m_hilfe);

    connect(fit, &QPushButton::clicked, this, [this] { m_flaeche->passeEin(); });
    connect(m_takte, &QSpinBox::valueChanged, this, [this](int takte) { setzeTakte(takte); });
    connect(m_play, &QPushButton::clicked, this, [this] { Q_EMIT spielenVerlangt(!m_spielt); });
    connect(m_fx, &QPushButton::clicked, this, [this](bool an) { (void)setzeEffekteAn(an); });
    connect(m_gain, &QDoubleSpinBox::valueChanged, this, [this](double db) { setzeGain(db); });
    connect(m_fadeIn, &QSpinBox::valueChanged, this, [this](int ms) { setzeFadeIn(ms); });
    connect(m_fadeOut, &QSpinBox::valueChanged, this, [this](int ms) { setzeFadeOut(ms); });
    connect(m_bezug, &QDoubleSpinBox::valueChanged, this, [this](double bpm) { setzeBezugBpm(bpm); });
    connect(m_bezugAufLoop, &QPushButton::clicked, this, [this] { setzeBezugAufLoop(); });
    connect(m_folgt, &QCheckBox::toggled, this, [this](bool an) { setzeFolgtTempo(an); });
    connect(m_tonhoehe, &QComboBox::activated, this,
            [this](int index) { setzeTonhoehenArt(static_cast<TonhoehenArt>(m_tonhoehe->itemData(index).toInt())); });
    connect(m_sampleBearbeiten, &QPushButton::clicked, this, [this] {
        if (oeffneSampleBearbeiten()) return;
        QMessageBox::information(this, tr("Edit sample"),
                                 tr("This sample could not be opened. Edit sample... works on samples up to %1 s.")
                                     .arg(AufnahmeFenster::kHoechstensSekunden, 0, 'f', 0));
    });
    connect(m_suchen, &QPushButton::clicked, this, [this] {
        QSettings einstellungen;
        const QString pfad = QFileDialog::getOpenFileName(this, tr("Choose audio file"),
                                                          einstellungen.value(QLatin1String(kOrdner)).toString(),
                                                          tr("Audio (*.wav *.mp3 *.flac *.ogg);;All files (*)"));
        if (pfad.isEmpty()) return;
        einstellungen.setValue(QLatin1String(kOrdner), QFileInfo(pfad).absolutePath());
        // WV2 a: dieselbe Frage wie bei einer Sample-Schicht (S2 a), mit demselben Merker
        const SampleKopie wahl = waehleSampleKopie(this, pfad, tr("Without a copy the pattern names the file at"),
                                                   QStringLiteral("waveSampleCopyDialog"));
        if (wahl == SampleKopie::Fragen) return;
        if (!nimmDatei(pfad, wahl == SampleKopie::Kopieren))
            QMessageBox::warning(this, tr("Wave"), tr("Could not copy %1 to My samples").arg(QDir::toNativeSeparators(pfad)));
    });
    connect(m_flaeche, &WaveFlaeche::geaendert, this, [this](const Muster& neu, const QString& was) {
        (void)aendere(was, [&neu](Muster& m) {
            const QString name = m.name;
            m = neu;
            m.name = name;
        });
    });
    connect(m_flaeche, &WaveFlaeche::abspielstelleGewaehlt, this, &WaveEditor::abspielstelleGewaehlt);
    connect(m_flaeche, &WaveFlaeche::noteGewaehlt, this, [this] { zeigeAutotune(); });
    connect(m_grundton, &QComboBox::activated, this, [this](int) {
        setzeTonart(m_grundton->currentData().toInt(), static_cast<Skala>(m_skala->currentData().toInt()));
    });
    connect(m_skala, &QComboBox::activated, this, [this](int) {
        setzeTonart(m_grundton->currentData().toInt(), static_cast<Skala>(m_skala->currentData().toInt()));
    });
    connect(m_erkennen, &QPushButton::clicked, this, [this] { (void)erkenneNoten(); });
    connect(m_staerke, &QSpinBox::valueChanged, this, [this](int prozent) { setzeStaerke(prozent / 100.0); });
    connect(m_speed, &QSpinBox::valueChanged, this, [this](int ms) { setzeSpeed(ms); });
    connect(m_stimmeNehmen, &QPushButton::clicked, this, [this] { setzeTonhoehenArt(TonhoehenArt::Stimme); });
    connect(m_flaeche, &WaveFlaeche::massGeaendert, this, &WaveEditor::passeRollbalkenAn);
    connect(m_rollX, &QScrollBar::valueChanged, this, [this](int wert) {
        m_flaeche->setRoll(wert / 100.0);  // der Balken zaehlt in Hundertsteln eines Schritts
    });

    resize(1100, 600);
    if (lageMerken()) (void)restoreGeometry(QSettings().value(QLatin1String(kLage)).toByteArray());
}

void WaveEditor::setDekodierer(Dekodierer dekodierer)
{
    m_dekodierer = std::move(dekodierer);
    m_bildVon.clear();
    m_verlaufVon.clear();
    ladeBild();
    m_dateiInfo->setText(dateiInfoText());
}

void WaveEditor::setEngineGeber(std::function<IAudioEngine*()> geber)
{
    m_engineGeber = std::move(geber);
}

AufnahmeFenster* WaveEditor::aufnahmeFenster() const
{
    return m_aufnahme;
}

void WaveEditor::setHinweis(const QString& text)
{
    if (m_hinweis->text() != text) m_hinweis->setText(text);
}

bool WaveEditor::oeffneSampleBearbeiten(bool zeigen)
{
    const Muster* m = basis();
    if (m == nullptr || !istEigenerKlang(m->wave.datei) || !m_dekodierer) return false;
    if (m_flaeche->bild().sekunden > AufnahmeFenster::kHoechstensSekunden) return false;
    std::vector<float> stereo =
        m_dekodierer(klangDatei(m->wave.datei), kSampleRate, AufnahmeFenster::kHoechstensSekunden + 1.0);
    if (stereo.empty()) return false;
    if (m_aufnahme == nullptr)
    {
        m_aufnahme = new AufnahmeFenster(this);
        connect(m_aufnahme, &AufnahmeFenster::gespeichert, this,
                [this](const QString& nennung, double vornWegMs) { nimmBearbeitetes(nennung, vornWegMs); });
    }
    // die Engine, wie sie jetzt ist — sie kann seit dem letzten Mal eine andere sein
    m_aufnahme->setEngine(m_engineGeber ? m_engineGeber() : nullptr);
    m_bearbeitet = m->wave.datei;
    m_aufnahme->bearbeite(m_bearbeitet, std::move(stereo), kSampleRate);
    if (zeigen)
    {
        m_aufnahme->show();
        m_aufnahme->raise();
        m_aufnahme->activateWindow();
    }
    return true;
}

void WaveEditor::nimmBearbeitetes(const QString& nennung, double vornWegMs)
{
    const Muster* m = basis();
    if (m == nullptr) return;
    // die Datei ist neu geschrieben: Bild und Klang neu
    m_bildVon.clear();
    if (nennung != m->wave.datei)
        setzeDatei(nennung);  // Save as...: das Muster nennt die neue Datei
    else if (vornWegMs > 0.0 && nennung == m_bearbeitet)
    {
        // vorn weggeschnitten: der Ausschnitt rueckt mit, der Ton bleibt, wo er ist
        const double weg = vornWegMs / 1000.0;
        (void)aendere(tr("Edit sample"), [weg](Muster& muster) {
            muster.wave.start = std::max(0.0, muster.wave.start - weg);
            if (muster.wave.ende > 0.0) muster.wave.ende = std::max(muster.wave.start + 0.01, muster.wave.ende - weg);
        });
    }
    zeigeAlles();
    Q_EMIT klaengeGeaendert();
}

bool WaveEditor::zeige(const Loop& loop, const QString& muster)
{
    const Muster* ziel = loop.findeMuster(muster);
    if (ziel == nullptr || ziel->art != MusterArt::Wave) return false;
    const bool andere = muster != m_muster;
    m_loop = loop;
    m_muster = muster;
    zeigeAlles();
    if (andere && m_gezeigt) m_flaeche->passeEin();
    return true;
}

void WaveEditor::setLoop(const Loop& loop)
{
    if (m_muster.isEmpty()) return;
    const QString folge = folgeBasis(m_loop, m_muster, loop);
    m_loop = loop;
    if (folge.isEmpty())
    {
        m_muster.clear();
        close();
        return;
    }
    m_muster = folge;
    zeigeAlles();
}

void WaveEditor::zeigeAlles()
{
    const Muster* ziel = basis();
    if (ziel == nullptr) return;
    setWindowTitle(tr("Wave pattern: %1").arg(m_muster));
    m_basisName->setText(m_muster);
    m_nutzer->setText(tr("· used by %1 clips").arg(basisNutzer(m_loop, m_muster)));
    const WaveAngaben& w = ziel->wave;
    {
        const QSignalBlocker a(m_takte);
        const QSignalBlocker b(m_gain);
        const QSignalBlocker c(m_fadeIn);
        const QSignalBlocker d(m_fadeOut);
        const QSignalBlocker e(m_bezug);
        const QSignalBlocker f(m_folgt);
        const QSignalBlocker g(m_tonhoehe);
        m_takte->setValue(ziel->takte);
        m_gain->setValue(w.gainDb);
        m_fadeIn->setValue(static_cast<int>(std::lround(w.fadeInMs)));
        m_fadeOut->setValue(static_cast<int>(std::lround(w.fadeOutMs)));
        m_bezug->setValue(w.bezugBpm);
        m_folgt->setChecked(w.folgtTempo);
        m_tonhoehe->setCurrentIndex(m_tonhoehe->findData(static_cast<int>(w.tonhoehenArt)));
    }
    m_datei->setText(w.datei.isEmpty() ? tr("(none)") : sampleTitel(w.datei));
    m_datei->setToolTip(QDir::toNativeSeparators(w.datei));
    m_sampleBearbeiten->setEnabled(istEigenerKlang(w.datei));
    m_sampleBearbeiten->setToolTip(istEigenerKlang(w.datei)
                                       ? tr("Open this sample in the Record sample window (up to %1 s)")
                                             .arg(AufnahmeFenster::kHoechstensSekunden, 0, 'f', 0)
                                       : tr("Only one of My samples can be edited here"));
    m_folgtHinweis->setText(tempoHinweis());
    m_rasterZeile->zeige(m_loop, *ziel);
    ladeBild();
    m_flaeche->setMuster(*ziel, m_loop);
    m_dateiInfo->setText(dateiInfoText());
    zeigeFx();
    zeigeAutotune();
    passeRollbalkenAn();
}

void WaveEditor::ladeBild()
{
    const Muster* ziel = basis();
    const QString datei = ziel != nullptr ? klangDatei(ziel->wave.datei) : QString();
    const QFileInfo info(datei);
    // je Datei und Stand einmal dekodiert
    const QString stand = datei.isEmpty() || !info.isFile()
                              ? datei
                              : datei + QLatin1Char('|') + QString::number(info.size()) + QLatin1Char('|') +
                                    QString::number(info.lastModified().toMSecsSinceEpoch());
    if (stand == m_bildVon) return;
    m_bildVon = stand;
    m_flaeche->setBild(waveBildVon(datei, m_dekodierer));
    starteTonhoehe(datei, stand);
}

void WaveEditor::starteTonhoehe(const QString& datei, const QString& stand)
{
    if (stand == m_verlaufVon) return;
    m_verlaufVon = stand;
    m_flaeche->setVerlauf({});
    m_stimmeVorschlag = false;
    if (datei.isEmpty() || !m_dekodierer || !QFileInfo(datei).isFile())
    {
        zeigeAutotune();
        return;
    }
    if (m_messung != nullptr)
    {
        zeigeAutotune();
        return;  // laeuft: danach sieht `sieheNachTonhoehe`, ob es noch die Datei ist
    }
    m_messung = std::make_unique<Messung>();
    Messung* messung = m_messung.get();
    messung->stand = stand;
    messung->faden = std::thread([messung, datei, dekodierer = m_dekodierer] {
        messung->verlauf = tonhoehenverlaufVon(datei, dekodierer, kTonhoeheRate);
        messung->fertig.store(true);
    });
    m_messFertig->start();
    zeigeAutotune();
}

void WaveEditor::sieheNachTonhoehe()
{
    if (m_messung == nullptr || !m_messung->fertig.load()) return;
    if (m_messung->faden.joinable()) m_messung->faden.join();
    const QString stand = m_messung->stand;
    std::optional<Tonhoehenverlauf> verlauf = std::move(m_messung->verlauf);
    m_messung.reset();
    m_messFertig->stop();
    if (stand == m_verlaufVon)
    {
        if (verlauf) m_flaeche->setVerlauf(std::move(*verlauf));
        zeigeAutotune();
        return;
    }
    // inzwischen eine andere Datei: die, wie sie jetzt ist
    const QString jetzt = m_verlaufVon;
    m_verlaufVon.clear();
    const Muster* ziel = basis();
    starteTonhoehe(ziel != nullptr ? klangDatei(ziel->wave.datei) : QString(), jetzt);
}

bool WaveEditor::tonhoeheBereit() const
{
    return m_messung == nullptr && !m_flaeche->verlauf().hz.empty();
}

void WaveEditor::warteAufTonhoehe()
{
    while (m_messung != nullptr)
    {
        if (m_messung->faden.joinable()) m_messung->faden.join();
        sieheNachTonhoehe();
    }
}

bool WaveEditor::stimmeVorgeschlagen() const
{
    const Muster* ziel = basis();
    return m_stimmeVorschlag && ziel != nullptr && ziel->wave.tonhoehenArt == TonhoehenArt::Allgemein;
}

QString WaveEditor::autotuneHinweis() const
{
    return m_autotuneHinweis->text();
}

void WaveEditor::zeigeAutotune()
{
    const Muster* ziel = basis();
    if (ziel == nullptr) return;
    const Autotune& at = ziel->wave.autotune;
    const int note = m_flaeche->gewaehlteNote();
    const AutotuneAbschnitt* gewaehlt =
        note >= 0 && note < static_cast<int>(at.abschnitte.size()) ? &at.abschnitte[static_cast<std::size_t>(note)] : nullptr;
    {
        const QSignalBlocker a(m_grundton);
        const QSignalBlocker b(m_skala);
        const QSignalBlocker c(m_staerke);
        const QSignalBlocker d(m_speed);
        m_grundton->setCurrentIndex(m_grundton->findData(at.grundton));
        m_skala->setCurrentIndex(m_skala->findData(static_cast<int>(at.skala)));
        m_staerke->setValue(static_cast<int>(std::lround((gewaehlt != nullptr ? gewaehlt->staerke : at.staerke) * 100.0)));
        m_speed->setValue(static_cast<int>(std::lround(gewaehlt != nullptr ? gewaehlt->speedMs : at.speedMs)));
    }
    const bool gemessen = !m_flaeche->verlauf().hz.empty();
    m_erkennen->setEnabled(gemessen);
    m_nurGewaehlte->setEnabled(gewaehlt != nullptr);
    QString hinweis;
    if (m_messung != nullptr)
        hinweis = tr("Measuring pitch...");
    else if (!ziel->wave.datei.isEmpty() && !gemessen)
        hinweis = tr("Pitch not measured");
    else if (!m_flaeche->gesungenImAusschnitt())
        hinweis = keinGesangText();
    else if (gewaehlt != nullptr)
        hinweis = tr("Selected: %1").arg(notenText(*gewaehlt));
    else if (!at.abschnitte.empty())
        hinweis = tr("%1 target notes").arg(at.abschnitte.size());
    if (stimmeVorgeschlagen()) hinweis = tr("One voice found — Voice keeps its formants");
    m_autotuneHinweis->setText(hinweis);
    m_stimmeNehmen->setVisible(stimmeVorgeschlagen());
}

void WaveEditor::setzeTonart(int grundton, Skala skala)
{
    (void)aendere(tr("Change key"), [grundton, skala](Muster& m) {
        m.wave.autotune.grundton = grundton;
        m.wave.autotune.skala = skala;
    });
}

void WaveEditor::setzeStaerke(double staerke)
{
    const int note = m_flaeche->gewaehlteNote();
    (void)aendere(tr("Change strength"), [note, staerke](Muster& m) {
        std::vector<AutotuneAbschnitt>& alle = m.wave.autotune.abschnitte;
        if (note >= 0 && note < static_cast<int>(alle.size()))
            alle[static_cast<std::size_t>(note)].staerke = staerke;
        else
            m.wave.autotune.staerke = staerke;
    });
}

void WaveEditor::setzeSpeed(double ms)
{
    const int note = m_flaeche->gewaehlteNote();
    (void)aendere(tr("Change speed"), [note, ms](Muster& m) {
        std::vector<AutotuneAbschnitt>& alle = m.wave.autotune.abschnitte;
        if (note >= 0 && note < static_cast<int>(alle.size()))
            alle[static_cast<std::size_t>(note)].speedMs = ms;
        else
            m.wave.autotune.speedMs = ms;
    });
}

void WaveEditor::setzeNurGewaehlte(bool an)
{
    m_nurGewaehlte->setChecked(an);
}

bool WaveEditor::erkenneNoten()
{
    const Muster* ziel = basis();
    const Tonhoehenverlauf& verlauf = m_flaeche->verlauf();
    if (ziel == nullptr || verlauf.hz.empty()) return false;
    // der Bereich: der Ausschnitt — oder die Zeit der gewaehlten Note
    double von = ziel->wave.start;
    double bis = m_flaeche->ausschnittEnde() > 0.0 ? m_flaeche->ausschnittEnde() : 1e9;
    const int note = m_flaeche->gewaehlteNote();
    const std::vector<AutotuneAbschnitt>& alle = ziel->wave.autotune.abschnitte;
    if (m_nurGewaehlte->isChecked() && note >= 0 && note < static_cast<int>(alle.size()))
    {
        von = alle[static_cast<std::size_t>(note)].anfang;
        bis = alle[static_cast<std::size_t>(note)].ende;
    }
    const NotenErkennung gefunden = lumi::loop::erkenneNoten(verlauf, ziel->wave.autotune, von, bis);
    if (gefunden.abschnitte.empty())
    {
        // nichts ersetzen, aber sagen, warum nichts geschieht
        m_autotuneHinweis->setText(m_flaeche->gesungenZwischen(von, bis) ? tr("No notes found — the sung parts are too short")
                                                                         : tr("No notes found — nothing sung in this section"));
        return false;
    }
    m_stimmeVorschlag = gefunden.eineStimme;
    m_flaeche->waehleNote(-1);
    const bool geaendert = aendere(tr("Detect notes"), [&gefunden, von, bis](Muster& m) {
        ersetzeAbschnitte(m.wave.autotune, gefunden.abschnitte, von, bis);
    });
    zeigeAutotune();
    return geaendert || !gefunden.abschnitte.empty();
}

QString WaveEditor::dateiText() const
{
    return m_datei->text();
}

QString WaveEditor::dateiInfoText() const
{
    const Muster* ziel = basis();
    if (ziel == nullptr || ziel->wave.datei.isEmpty()) return {};
    const WaveBild& bild = m_flaeche->bild();
    if (bild.sekunden <= 0.0)
        return QFileInfo(klangDatei(ziel->wave.datei)).isFile() ? tr("not read") : tr("not found");
    QStringList teile{tr("%1 s").arg(bild.sekunden, 0, 'f', 2)};
    if (bild.rate > 0) teile << tr("%1 Hz").arg(bild.rate);
    if (bild.kanaele == 1) teile << tr("mono");
    else if (bild.kanaele == 2) teile << tr("stereo");
    else if (bild.kanaele > 2) teile << tr("%1 channels").arg(bild.kanaele);
    return teile.join(QStringLiteral(" · "));
}

QString WaveEditor::tempoHinweis() const
{
    const Muster* ziel = basis();
    if (ziel == nullptr || ziel->wave.bezugBpm <= 0.0) return {};
    const double faktor = m_loop.bpm / ziel->wave.bezugBpm;
    if (std::abs(faktor - 1.0) < 1e-9) return tr("plays as recorded when on — the tempos are the same");
    return tr("plays %1× as fast when on").arg(QString::number(faktor, 'f', 2));
}

void WaveEditor::passeRollbalkenAn()
{
    const double sichtbar = m_flaeche->schrittBreite() > 0.0 ? m_flaeche->width() / m_flaeche->schrittBreite() : 0.0;
    const double rest = std::max(0.0, m_flaeche->schritte() - sichtbar);
    const QSignalBlocker ruhe(m_rollX);
    m_rollX->setRange(0, static_cast<int>(std::ceil(rest * 100.0)));
    m_rollX->setPageStep(static_cast<int>(sichtbar * 100.0));
    m_rollX->setValue(static_cast<int>(std::lround(m_flaeche->roll() * 100.0)));
    m_rollX->setVisible(rest > 0.0);
}

void WaveEditor::setSpielt(bool spielt, double schritt)
{
    if (spielt != m_spielt)
    {
        m_spielt = spielt;
        m_play->setText(spielt ? tr("Stop") : tr("Play pattern"));
    }
    m_flaeche->setMarke(spielt ? schritt : -1.0);
}

void WaveEditor::zeigeMarke(double schritt)
{
    m_flaeche->setMarke(schritt);
}

void WaveEditor::setSpitze(double spitze)
{
    const QString text = spitze < 0.0 ? QStringLiteral("–") : QString::number(spitze, 'f', 2);
    if (text == m_spitze->text()) return;
    m_spitze->setText(text);
    m_spitze->setStyleSheet(spitze > 1.0 ? QStringLiteral("color: #e05050; font-weight: bold;") : QString());
}

void WaveEditor::setMarkierterClip(std::optional<ClipOrt> ort)
{
    m_markiert = ort;
    zeigeFx();
}

std::optional<ClipOrt> WaveEditor::bezugsclip() const
{
    return lumi::loop::bezugsclip(m_loop, m_muster, m_markiert);
}

bool WaveEditor::setzeEffekteAn(bool an)
{
    const std::optional<ClipOrt> ort = bezugsclip();
    Loop neu = m_loop;
    if (!ort || !lumi::loop::setzeEffekteAn(neu, *ort, an))
    {
        zeigeFx();
        return false;
    }
    Q_EMIT bearbeitet(neu, an ? tr("Effects on") : tr("Effects off"));
    return true;
}

void WaveEditor::zeigeFx()
{
    const std::optional<ClipOrt> ort = bezugsclip();
    const int clip = ort ? findeClip(m_loop, *ort) : -1;
    const LoopClip* bezug =
        clip >= 0 ? &m_loop.kanaele[static_cast<std::size_t>(ort->kanal)].clips[static_cast<std::size_t>(clip)] : nullptr;
    const QSignalBlocker ruhe(m_fx);
    const bool hatEffekte = bezug != nullptr && bezug->effekte.einerAn();
    m_fx->setEnabled(hatEffekte);
    m_fx->setChecked(hatEffekte && bezug->effekteAn);
    if (bezug == nullptr)
        m_fx->setToolTip(tr("Effects: this pattern has no clip in the loop yet"));
    else if (!hatEffekte)
        m_fx->setToolTip(tr("Effects of the clip %1: none — set them with Clip effects in the Loop panel").arg(clipText(m_loop, *ort)));
    else
        m_fx->setToolTip(tr("Effects of the clip %1: %2 — Play pattern plays them; the same switch as in the loop")
                             .arg(clipText(m_loop, *ort), bezug->effekteAn ? tr("on") : tr("off")));
}

bool WaveEditor::aendere(const QString& was, const std::function<void(Muster&)>& aenderung)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return false;
    aenderung(*ziel);
    neu.richte();
    if (neu == m_loop)
    {
        zeigeAlles();  // die Felder stehen wieder, wie das Muster ist
        return false;
    }
    Q_EMIT bearbeitet(neu, was);
    return true;
}

void WaveEditor::setzeTakte(int takte)
{
    Loop neu = m_loop;
    if (!setzeMusterTakte(neu, m_muster, takte, false) || neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern length"));
}

void WaveEditor::setzeDatei(const QString& nennung)
{
    (void)aendere(tr("Change wave file"), [&nennung](Muster& m) { m.wave.datei = nennung; });
}

bool WaveEditor::nimmDatei(const QString& pfad, bool kopieren)
{
    const QString nennung = kopieren ? nimmZuEigenen(pfad) : pfad;
    if (nennung.isEmpty()) return false;
    setzeDatei(nennung);
    return true;
}

void WaveEditor::setzeGain(double db)
{
    (void)aendere(tr("Change gain"), [db](Muster& m) { m.wave.gainDb = db; });
}

void WaveEditor::setzeFadeIn(double ms)
{
    (void)aendere(tr("Change fade in"), [ms](Muster& m) { m.wave.fadeInMs = ms; });
}

void WaveEditor::setzeFadeOut(double ms)
{
    (void)aendere(tr("Change fade out"), [ms](Muster& m) { m.wave.fadeOutMs = ms; });
}

void WaveEditor::setzeBezugBpm(double bpm)
{
    (void)aendere(tr("Change reference tempo"), [bpm](Muster& m) { m.wave.bezugBpm = bpm; });
}

void WaveEditor::setzeBezugAufLoop()
{
    const double bpm = m_loop.bpm;
    (void)aendere(tr("Set reference to loop tempo"), [bpm](Muster& m) { m.wave.bezugBpm = bpm; });
}

void WaveEditor::setzeFolgtTempo(bool an)
{
    (void)aendere(an ? tr("Follow tempo") : tr("Play as recorded"), [an](Muster& m) { m.wave.folgtTempo = an; });
}

void WaveEditor::setzeTonhoehenArt(TonhoehenArt art)
{
    (void)aendere(tr("Change pitch method"), [art](Muster& m) { m.wave.tonhoehenArt = art; });
}

void WaveEditor::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (m_gezeigt) return;
    m_gezeigt = true;
    m_flaeche->passeEin();  // beim ersten Zeigen: das ganze Muster
}

void WaveEditor::hideEvent(QHideEvent* event)
{
    if (lageMerken()) QSettings().setValue(QLatin1String(kLage), saveGeometry());
    QWidget::hideEvent(event);
}

}  // namespace lumi::loop
