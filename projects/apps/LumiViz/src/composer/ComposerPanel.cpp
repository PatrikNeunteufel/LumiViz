/**
 * @file ComposerPanel.cpp
 * @brief Composer: das Panel zum Ansehen (Stufe C2b)
 */

#include "composer/ComposerPanel.hpp"

#include "UI/panels/KaraokePanel.hpp"  // der eine Weg, die Karaoke-Ebene zu schalten (S101)
#include "UI/widgets/AudioDateien.hpp"
#include "UI/widgets/Bedienhilfe.hpp"  // das Menue des Rechtsklicks, testbar (S130)
#include "UI/widgets/DateiDrag.hpp"
#include "UI/widgets/PresetTypeIcons.hpp"  // das Datei-Icon der Komposition (S100)
#include "UI/widgets/VisualizerWidget.hpp"
#include "services/events/UIEvents.hpp"            // KaraokeEnabledChangedEvent (S101)
#include "visualizers/MultiEffectVisualizer.hpp"  // der Schalter der Karaoke-Ebene (S101)
#include "audio/IAudioEngine.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/FormPresets.hpp"
#include "composer/KurvenEditor.hpp"
#include "composer/LautheitsDialog.hpp"
#include "composer/Mischer.hpp"
#include "composer/MuteKnopf.hpp"
#include "composer/Sitzung.hpp"
#include "loop/ExportFenster.hpp"  // Export audio... am Loop-Clip (Stufe 30c)
#include "loop/Loop.hpp"   // ein Loop als Clip im Audio-Kanal (S109)
#include "loop/Offen.hpp"
#include "services/ICommandBus.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHelpEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollBar>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStyle>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <algorithm>
#include <array>
#include <cmath>

namespace lumi::composer
{

namespace
{
const QString kKnopfAn = QStringLiteral("QPushButton { background-color: #4a6fa5; border-radius: 4px; }");

/// Eine Kanalfarbe (0xRRGGBB) zum Zeichnen — aus (Kanal aus oder Composer
/// aus) derselbe Ton, blass.
QColor zeichenFarbe(std::int32_t farbe, bool an)
{
    QColor f = QColor::fromRgb(static_cast<QRgb>(farbe) | 0xFF000000u);
    if (!an) f = QColor::fromHsv(std::max(0, f.hue()), std::min(f.saturation(), 40), 110);
    return f;
}

/// Farbe der Clips eines Kanals (S100, Entscheid Patrik K1 b): die Farbe, die
/// der Kanal traegt — Bild-Kanaele kuehl, Audio-Kanaele warm, je Kanal ein
/// eigener Ton, einstellbar per Rechtsklick.
QColor kanalFarbe(const Komposition& in, int kanal, bool an)
{
    return zeichenFarbe(in.farbeVon(kanal), an);
}

/// Die Ablage fuer Kopieren / Einfuegen (S98): ein Clip samt allem, was er
/// traegt, und seine Art. Eine fuer das ganze Programm.
struct Ablage
{
    bool voll = false;
    Clip clip;
    KanalArt art = KanalArt::Visual;
};
Ablage g_ablage;

/// Einstellung: die Hoehe der Zeilen in der Zeitachse, in Prozent (S102, ZO9).
const QLatin1String kZeilenProzent("composer/rowHeightPercent");

/// So fein wird die Huellkurve eines Songs geholt (ein Wert je 50 ms).
constexpr int kHuelleMs = 50;

/// Abstand der Striche im Lineal, in Millisekunden: der kleinste, bei dem die
/// Beschriftungen nicht zusammenstossen (64 Pixel). Hineingezoomt bis auf
/// Zehntelsekunden (S102).
std::int64_t strichMs(double msJePixel)
{
    static constexpr std::array<std::int64_t, 15> kStufen = {100,    200,    500,    1000,    2000,
                                                             5000,   10000,  15000,  30000,   60000,
                                                             120000, 300000, 600000, 1800000, 3600000};
    if (msJePixel <= 0.0) return 1000;
    for (const std::int64_t s : kStufen)
        if (static_cast<double>(s) / msJePixel >= 64.0) return s;
    return kStufen.back();
}

/// Ein Schritt des Bearbeitens fuer Undo / Redo: die Fassung davor und danach.
/// Gilt nur fuer die Ladung, in der er entstand — nach Open oder Close
/// Composition tut er nichts mehr.
class KompositionsBefehl : public ICommand
{
public:
    KompositionsBefehl(Sitzung& sitzung, Komposition nachher, QString was)
        : m_sitzung(&sitzung),
          m_ladung(sitzung.ladung()),
          m_vorher(sitzung.komposition()),
          m_nachher(std::move(nachher)),
          m_was(was.toStdString())
    {
    }

    bool execute() override
    {
        if (!gilt() || gleich(m_vorher, m_nachher)) return false;
        m_sitzung->setzeKomposition(m_nachher);
        return true;
    }
    void undo() override
    {
        if (gilt()) m_sitzung->setzeKomposition(m_vorher);
    }
    [[nodiscard]] std::string description() const override { return m_was; }

private:
    [[nodiscard]] bool gilt() const { return m_sitzung != nullptr && m_sitzung->ladung() == m_ladung; }

    QPointer<Sitzung> m_sitzung;
    int m_ladung;
    Komposition m_vorher;
    Komposition m_nachher;
    std::string m_was;
};
}  // namespace

// =============================================================================
// Zeitachse
// =============================================================================

Zeitachse::Zeitachse(QWidget* parent) : lumi::ui::Achse(kMasterZeilen, kFeinsteMsJePixel, parent)
{
    setObjectName(QStringLiteral("composerZeitachse"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // sie rollt selbst (S102)
    setFocusPolicy(Qt::StrongFocus);  // Entf und Esc im Bearbeiten
    setAcceptDrops(true);             // Presets aus dem Effect Browser (C3b)
    setMouseTracking(true);           // die Zeit unter dem Zeiger im Lineal; im Bearbeiten der Zeiger an den Kanten
}

void Zeitachse::setKomposition(const Komposition& komposition, bool geladen)
{
    m_komposition = komposition;
    m_geladen = geladen;
    setzeLaenge(geladen ? anzeigeLaenge(komposition) : 0);
    m_kanal = -1;
    m_clip = -1;
    m_markeKanal = -1;
    // eine andere Komposition beginnt am Anfang, oben und mit der ganzen
    // Laenge auf der Breite (Entscheid Patrik ZO9: a)
    beginneNeu();
    updateGeometry();
    passeRollenAn();
    update();
    Q_EMIT auswahlGeaendert();
}

void Zeitachse::setSchleife(bool schleife, std::int64_t zweiteMs)
{
    if (schleife == m_schleife && zweiteMs == m_zweiteMs) return;
    m_schleife = schleife;
    m_zweiteMs = zweiteMs;
    update();
}

void Zeitachse::setAn(bool an)
{
    if (an == m_an) return;
    m_an = an;
    update();
}

const VisualClip* Zeitachse::auswahl() const
{
    return clipIn(m_komposition, m_kanal, m_clip, m_gesamt);
}

const Clip* Zeitachse::clipIn(const Komposition& in, int kanal, int clip, Clip& ablage)
{
    if (istMaster(kanal))
    {
        // eine Kurve der ganzen Komposition (S100; S101: MASTER, VISUAL oder
        // AUDIO): ein Clip ueber [0, Laenge)
        if (clip != 0 || in.laenge() <= 0) return nullptr;
        ablage = in.kurveAlsClip(nullptr, masterArt(kanal));
        return &ablage;
    }
    if (kanal < 0 || kanal >= static_cast<int>(in.kanaele.size())) return nullptr;
    const auto& clips = in.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return nullptr;
    return &clips[static_cast<std::size_t>(clip)];
}

void Zeitachse::schreibeClip(Komposition& in, int kanal, int clip, const Clip& neu)
{
    if (istMaster(kanal))
        in.setzeKurveAusClip(neu, masterArt(kanal));
    else
        in.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)] = neu;
}

void Zeitachse::waehleGesamt()
{
    Clip ablage;
    if (!m_geladen || clipIn(m_komposition, kMaster, 0, ablage) == nullptr) return;
    if (m_kanal == kMaster && m_clip == 0) return;
    m_kanal = kMaster;
    m_clip = 0;
    update();
    Q_EMIT auswahlGeaendert();
}

std::int64_t Zeitachse::anzeigeLaenge(const Komposition& komposition)
{
    // Luft hinter dem Ende: sonst liesse sich nichts ueber das Ende hinaus
    // ziehen oder dahinter fallen lassen. Eine leere Komposition zeigt 30 s.
    const std::int64_t laenge = komposition.laenge();
    return std::max<std::int64_t>(30000, laenge + std::max<std::int64_t>(10000, laenge / 5));
}

int Zeitachse::kanalVonZeile(int zeile) const
{
    return zeile;  // CH1 oben (Entscheid Patrik S95) — im Stapel liegt er zuunterst
}

QRect Zeitachse::clipRechteck(int zeile, const VisualClip& clip) const
{
    const int links = xVon(clip.startMs);
    const int rechts = xVon(clip.endeMs);
    return {links, zeileOben(zeile) + 3, std::max(2, rechts - links), zeileHoch(zeile) - 6};
}

int Zeitachse::zeilenZahl() const
{
    // im Bearbeiten eine Zeile mehr: die Flaeche, auf der ein neuer Kanal entsteht
    return std::max<int>(2, static_cast<int>(m_komposition.kanaele.size()) + (m_bearbeiten ? 1 : 0));
}

QString Zeitachse::versteckterName(QPoint ort) const
{
    if (!m_geladen) return {};
    const int zeile = zeileBei(ort.y());
    const int zeilen = static_cast<int>(m_komposition.kanaele.size());
    if (zeile == kKeineZeile || zeile >= zeilen) return {};
    if (ort.x() < kKopfBreite)
    {
        if (namePasst(zeileHoch(zeile))) return {};
        static const std::array<const char*, kMasterZeilen> kMasterNamen = {"MASTER", "VISUAL", "AUDIO", "KARAOKE"};
        return zeile < 0 ? tr(kMasterNamen[static_cast<std::size_t>(zeile + kMasterZeilen)])
                         : m_komposition.kanaele[static_cast<std::size_t>(kanalVonZeile(zeile))].name;
    }
    if (zeile < 0) return {};  // die Kurven der Komposition tragen keinen Dateinamen
    for (const VisualClip& clip : m_komposition.kanaele[static_cast<std::size_t>(kanalVonZeile(zeile))].clips)
    {
        const QRect r = clipRechteck(zeile, clip);
        if (r.contains(ort)) return namePasst(r.height()) ? QString() : QFileInfo(clip.datei).completeBaseName();
    }
    return {};
}

void Zeitachse::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));

    if (!m_geladen)
    {
        p.setPen(pal.color(QPalette::Disabled, QPalette::Text));
        p.drawText(rect(), Qt::AlignCenter,
                   tr("No composition loaded — File → Open Composition..."));
        return;
    }

    const QColor text = pal.color(QPalette::Text);
    QColor linie = text;
    linie.setAlpha(50);
    const int zeilen = static_cast<int>(m_komposition.kanaele.size());
    // die Unterkante der letzten Zeile im Fenster (gerollt kann sie ueber dem Rand liegen)
    const int unten = std::max(kLinealHoehe, zeileOben(zeilen));

    // S102 (Entscheid Patrik ZO4: a): das Lineal bleibt beim Rollen stehen, die
    // Zeilen rollen darunter. Gezeichnet wird deshalb in zwei Gaengen mit je
    // einem Ausschnitt — erst die Zeilen, dann das Lineal. Was senkrecht durch
    // beide laeuft (Striche, Ende, Toenung, Marken), steht in `unterlage` und
    // `auflage` und wird in beiden Gaengen gezeichnet; der Ausschnitt laesst
    // jeweils nur seinen Teil durch.
    const Flaechen ausschnitte = flaechen();
    const QRect& linealFlaeche = ausschnitte.lineal;
    const QRect& zeilenFlaeche = ausschnitte.zeilen;
    // S102, Zeit-Zoom: die Spalte der Kanalnamen bleibt beim waagrechten Rollen
    // stehen. Alles, was an einer Zeit haengt, wird deshalb nur rechts von ihr
    // gezeichnet (ab ihrer Trennlinie — dort steht die Abspielmarke bei null).
    const QRect& linealZeit = ausschnitte.linealZeit;
    const QRect& zeilenZeit = ausschnitte.zeilenZeit;

    const std::int64_t schrittMs = strichMs(msJePixel());
    const std::int64_t endeMs = m_komposition.laenge();
    const int xEnde = xVon(endeMs);
    const std::int64_t fensterMs = m_schleife ? m_komposition.ueberlappung() : 0;

    const auto unterlage = [&] {
        // --- die Striche des Lineals, mit ihrer Beschriftung
        zeichneLinealStriche(p, schrittMs, unten, [schrittMs](std::int64_t ms) {
            // unter einer Sekunde Abstand mit der Zehntelsekunde
            return schrittMs < 1000 ? ComposerPanel::zeitText(ms)
                                    : ComposerPanel::zeitText(ms).section(QLatin1Char('.'), 0, 0);
        });
        // --- das Ende: folgt es dem letzten Clip, eine duenne Linie; ist es fest
        //     gesetzt, eine eigene Marke — und die leere Strecke davor schraffiert
        if (m_komposition.laengeMs > 0)
        {
            const QColor fest(240, 170, 60);
            const int xClips = xVon(std::min(m_komposition.clipEnde(), endeMs));
            if (xEnde > xClips)
            {
                QColor strich = fest;
                strich.setAlpha(90);
                p.fillRect(QRect(xClips, kLinealHoehe, xEnde - xClips, unten - kLinealHoehe),
                           QBrush(strich, Qt::BDiagPattern));
            }
            p.setPen(QPen(fest, 2.0));
            p.drawLine(xEnde, 0, xEnde, unten);
            p.setBrush(fest);
            p.drawPolygon(QPolygon({QPoint(xEnde - 6, 0), QPoint(xEnde + 6, 0), QPoint(xEnde, 9)}));
            p.setBrush(Qt::NoBrush);
        }
        else
        {
            p.setPen(linie);
            p.drawLine(xEnde, 0, xEnde, unten);
        }

        // (Die Blende der ganzen Komposition stand bis S99 als Keil im Lineal; seit
        //  S100 zeigt die Zeile MASTER darunter die ganze Kurve.)

        // --- die Fenster der Ueberlappung (S99, Entscheid Patrik): bei Loop sind
        //     die ersten und die letzten Sekunden getoent — was im vorderen laeuft,
        //     laeuft zugleich mit dem im hinteren. Ein Strich an jeder Grenze.
        if (fensterMs > 0)
        {
            QColor toenung(90, 200, 170);
            toenung.setAlpha(34);
            const int xVorn = xVon(fensterMs);
            const int xHinten = xVon(m_komposition.umlaufMs());
            const int xNull = xVon(0);
            p.fillRect(QRect(xNull, 0, xVorn - xNull, unten), toenung);
            p.fillRect(QRect(xHinten, 0, xEnde - xHinten, unten), toenung);
        }
    };

    const auto auflage = [&] {
        // --- die Grenzen der Fenster, ueber den Clips: dort endet das vordere und beginnt das hintere
        if (fensterMs > 0)
        {
            p.setPen(QPen(QColor(90, 200, 170, 200), 1.0, Qt::DashLine));
            const int xVorn = xVon(fensterMs);
            const int xHinten = xVon(m_komposition.umlaufMs());
            p.drawLine(xVorn, 0, xVorn, unten);
            p.drawLine(xHinten, 0, xHinten, unten);
        }

        // --- Abspielmarke — und im Fenster der Ueberlappung eine zweite, blassere
        //     an der Stelle des Endes, die zugleich laeuft
        if (m_zweiteMs >= 0)
        {
            QPen zweite(QColor(255, 90, 70, 120), 2.0);
            if (!m_an) zweite.setStyle(Qt::DashLine);
            p.setPen(zweite);
            const int xZweite = xVon(m_zweiteMs);
            p.drawLine(xZweite, 0, xZweite, unten);
        }
        const int x = xVon(stelle());
        QPen marke(QColor(255, 90, 70), 2.0);
        if (!m_an) marke.setStyle(Qt::DashLine);
        p.setPen(marke);
        p.drawLine(x, 0, x, unten);
    };

    // --- erster Gang: die Zeilen, unter dem Lineal
    p.setClipRect(zeilenZeit);
    unterlage();

    // --- Kanaele: in der Reihenfolge der Datei, CH1 oben
    p.setFont(font());
    // Zeile -1, unter dem Lineal: die Pegelkurve der ganzen Komposition (S100)
    // als ein Clip ueber die ganze Laenge — gezeichnet und bedient wie ein Clip.
    // S101: darunter VISUAL (nur das Bild) und AUDIO (nur der Ton), ebenso.
    Kanal master[kMasterZeilen];
    master[0].name = tr("MASTER");
    master[1].name = tr("VISUAL");
    master[2].name = tr("AUDIO");
    master[3].name = tr("KARAOKE");  // Entscheid Patrik KS1: a — die Karaoke-Ebene
    for (int m = 0; m < kMasterZeilen; ++m)
        if (Clip ablage; clipIn(m_komposition, masterVonZeile(m - kMasterZeilen), 0, ablage) != nullptr)
            master[m].clips.push_back(ablage);
    for (int zeile = -kMasterZeilen; zeile < zeilen; ++zeile)
    {
        const int k = zeile < 0 ? masterVonZeile(zeile) : kanalVonZeile(zeile);
        const Kanal& kanal =
            zeile < 0 ? master[zeile + kMasterZeilen] : m_komposition.kanaele[static_cast<std::size_t>(k)];
        const int oben = zeileOben(zeile);
        const int hoch = zeileHoch(zeile);  // die Zeile MASTER ist niedriger (M17)
        if (oben + hoch < kLinealHoehe || oben > height()) continue;  // aus dem Fenster gerollt
        p.setClipRect(zeilenFlaeche);
        p.setPen(linie);
        p.drawLine(0, oben + hoch, width(), oben + hoch);

        QColor name = text;
        if (!kanal.an) name.setAlpha(110);
        p.setPen(name);
        const QString titel = kanal.an ? kanal.name : tr("%1 (off)").arg(kanal.name);
        // passt der Name nicht mehr in die Zeile, bleibt er weg (Entscheid Patrik ZO3: a)
        if (namePasst(hoch))
            p.drawText(QRect(8, oben, kKopfBreite - 12, hoch), Qt::AlignLeft | Qt::AlignVCenter,
                       fontMetrics().elidedText(titel, Qt::ElideRight, kKopfBreite - 12));

        p.setClipRect(zeilenZeit);
        for (int c = 0; c < static_cast<int>(kanal.clips.size()); ++c)
        {
            const VisualClip& clip = kanal.clips[static_cast<std::size_t>(c)];
            const QRect r = clipRechteck(zeile, clip);
            if (r.right() < kKopfBreite || r.left() > width()) continue;  // aus dem Fenster gerollt
            const bool gewaehlt = k == m_kanal && c == m_clip;
            const bool ton = kanal.art == KanalArt::Audio;
            // die Master-Zeilen in Grau — VISUAL mit kuehlem, AUDIO mit warmem
            // Stich, wie die zwei Farbreihen der Kanaele (von mir gesetzt)
            // — KARAOKE mit einem Stich ins Gruene
            const QColor masterFarbe = k == kMasterBild      ? QColor(84, 100, 124)
                                       : k == kMasterTon     ? QColor(120, 100, 84)
                                       : k == kMasterKaraoke ? QColor(88, 112, 96)
                                                             : QColor(96, 100, 112);
            const QColor farbe = istMaster(k) ? (m_an ? masterFarbe : masterFarbe.darker(130))
                                              : kanalFarbe(m_komposition, k, m_an && kanal.an && clip.an);
            p.setPen(gewaehlt ? QPen(pal.color(QPalette::Highlight), 2.0) : QPen(farbe.darker(150), 1.0));
            p.setBrush(farbe);
            // hineingezoomt kann ein Clip viele tausend Pixel breit sein: gezeichnet
            // wird nur, was im Fenster liegt (und etwas mehr, fuer die runden Ecken)
            p.drawRoundedRect(r.intersected(QRect(kKopfBreite - 12, r.top(), width() - kKopfBreite + 24,
                                                  r.height())),
                              4, 4);

            // Ton (C5): die Wellenform des Songs — so hoch, wie der Pegel des
            // Clips sie laesst; man sieht die Blenden an ihr
            const auto huelle = ton ? m_huellen.constFind(clip.datei) : m_huellen.constEnd();
            if (huelle != m_huellen.constEnd() && !huelle->werte.empty() && huelle->msProWert > 0 &&
                r.width() > 8 && clip.dauerMs() > 0)
            {
                const double dauer = static_cast<double>(clip.dauerMs());
                const double halb = (r.height() - 6) / 2.0;
                const double mitte = r.center().y() + 0.5;
                const auto werte = static_cast<std::int64_t>(huelle->werte.size());
                const QColor welle(255, 255, 255, 95);
                double pegel = -1.0;  // noch nicht gerechnet
                for (int x = std::max(r.left() + 1, kKopfBreite - 1); x < std::min(r.right(), width()); ++x)
                {
                    const double a0 = static_cast<double>(x - r.left()) / r.width();
                    const double a1 = static_cast<double>(x + 1 - r.left()) / r.width();
                    const auto imClip = static_cast<std::int64_t>(a0 * dauer);
                    // den Pegel nur alle vier Pixel neu rechnen
                    if (pegel < 0.0 || (x - r.left()) % 4 == 1) pegel = clip.pegelBei(imClip);
                    // der groesste Ausschlag in dem Stueck Song, das dieses Pixel zeigt
                    const std::int64_t von = clip.songStelle(imClip) / huelle->msProWert;
                    const std::int64_t bis = std::max(
                        von + 1, clip.songStelle(static_cast<std::int64_t>(a1 * dauer)) / huelle->msProWert);
                    float spitze = 0.0f;
                    for (std::int64_t i = std::max<std::int64_t>(0, von); i < std::min(bis, werte); ++i)
                        spitze = std::max(spitze, huelle->werte[static_cast<std::size_t>(i)]);
                    const double hoch = std::max(0.5, spitze * pegel * halb);
                    p.fillRect(QRectF(x, mitte - hoch, 1.0, 2.0 * hoch), welle);
                }
            }

            // Ein Loop (S109): statt einer Wellenform ein gestrichelter Strich an
            // jeder Stelle, an der er von vorn beginnt — nur, solange die
            // Striche nicht zusammenlaufen
            if (ton && istLoop(clip.datei) && r.width() > 8)
            {
                const std::int64_t durchlauf = songLaenge(clip.datei);
                if (durchlauf > 0 && static_cast<double>(durchlauf) / msJePixel() >= 6.0)
                {
                    p.setPen(QPen(QColor(255, 255, 255, 150), 1.0, Qt::DashLine));
                    for (std::int64_t imClip = durchlauf - clip.einsatzMs % durchlauf; imClip < clip.dauerMs();
                         imClip += durchlauf)
                    {
                        const int x = xVon(clip.startMs + imClip);
                        if (x >= kKopfBreite && x <= width()) p.drawLine(x, r.top() + 2, x, r.bottom() - 1);
                    }
                }
            }

            // die Pegelkurve: Hoehe im Clip = Pegel
            if (!clip.vollerPegel() && r.width() > 8 && clip.dauerMs() > 0)
            {
                QPainterPath kurve;
                // nur das sichtbare Stueck des Clips, damit die Kurve auch
                // hineingezoomt fein bleibt — ganz im Fenster: von 0 bis 1
                const int sichtLinks = std::max(r.left(), kKopfBreite - 4);
                const int sichtRechts = std::min(r.left() + r.width(), width() + 4);
                const double a0 = static_cast<double>(sichtLinks - r.left()) / r.width();
                const double a1 = static_cast<double>(sichtRechts - r.left()) / r.width();
                const int stuetzen = std::clamp((sichtRechts - sichtLinks) / 3, 8, 200);
                for (int i = 0; i <= stuetzen; ++i)
                {
                    const double anteil = a0 + (a1 - a0) * static_cast<double>(i) / stuetzen;
                    const double pegel = clip.pegelBei(
                        static_cast<std::int64_t>(anteil * static_cast<double>(clip.dauerMs())));
                    const QPointF punkt(r.left() + anteil * r.width(),
                                        r.bottom() - 1 - pegel * (r.height() - 3));
                    if (i == 0)
                        kurve.moveTo(punkt);
                    else
                        kurve.lineTo(punkt);
                }
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(QColor(255, 255, 255, 170), 1.2));
                p.drawPath(kurve);
            }

            // das Teil, an dem der Curve Editor haengt: ein Strich unten im Clip
            if (gewaehlt && hatTeil(clip, m_teil))
            {
                const Bereich bereich = bereichVon(clip, m_teil);
                const int x0 = std::max(r.left() + 1, xVon(clip.startMs + bereich.vonMs));
                const int x1 = std::min(r.right() - 1, xVon(clip.startMs + bereich.bisMs));
                if (x1 > x0) p.fillRect(QRect(x0, r.bottom() - 3, x1 - x0, 3), QColor(255, 220, 120));
            }

            p.setPen(Qt::white);
            // der Name bleibt am linken Rand des Fensters stehen, wenn der
            // Anfang des Clips hinausgerollt ist (von mir gesetzt, S102)
            QRect namensFeld = r.adjusted(6, 0, -4, 0);
            namensFeld.setLeft(std::max(r.left(), kKopfBreite) + 6);
            // das Gain eines Ton-Clips als Marke rechts im Clip (S126, G2 a) —
            // am rechten Rand des Fensters stehend, wenn das Ende hinausgerollt ist
            if (ton && clip.gainDb != 0.0 && namePasst(r.height()))
            {
                const QString gainMarke = ComposerPanel::gainText(clip.gainDb);
                const int breite = fontMetrics().horizontalAdvance(gainMarke) + 10;
                const int rechts = std::min(r.right(), width()) - 4;
                const QRect marke(rechts - breite, r.center().y() - fontMetrics().height() / 2 - 1, breite,
                                  fontMetrics().height() + 2);
                if (marke.left() > namensFeld.left() + 24)
                {
                    p.setBrush(QColor(0, 0, 0, 70));
                    p.setPen(QPen(QColor(255, 255, 255, 170), 1.0));
                    p.drawRoundedRect(marke, 3, 3);
                    p.setPen(Qt::white);
                    p.drawText(marke, Qt::AlignCenter, gainMarke);
                    namensFeld.setRight(marke.left() - 6);
                }
            }
            if (namePasst(r.height()))
            {
                // ausgeschaltet (S126, D1 c): blasse Schrift und „(off)", wie am Kanal
                const QString clipName = QFileInfo(clip.datei).completeBaseName();
                if (!clip.an) p.setPen(QColor(255, 255, 255, 140));
                p.drawText(namensFeld, Qt::AlignLeft | Qt::AlignVCenter,
                           fontMetrics().elidedText(clip.an ? clipName : tr("%1 (off)").arg(clipName),
                                                    Qt::ElideRight, std::max(0, namensFeld.width())));
            }

            // Griffe der Blenden (C4), nur im Bearbeiten: oben im Clip je ein
            // Quadrat fuer die Laenge; am angeklickten Clip die Bézier-Anfasser
            if (m_bearbeiten && kurvenpunkteZiehbar())
            {
                for (const bool ein : {true, false})
                {
                    const QPoint griff = blendenGriff(k, c, ein);
                    if (griff.x() < 0) continue;
                    p.setPen(QPen(QColor(30, 30, 30), 1.0));
                    p.setBrush(Qt::white);
                    p.drawRect(QRect(griff.x() - 3, griff.y() - 3, 7, 7));
                    if (!gewaehlt) continue;
                    const QPoint a1 = bezierGriff(k, c, ein, 0);
                    const QPoint a2 = bezierGriff(k, c, ein, 1);
                    if (a1.x() < 0) continue;
                    // die Hebel: vom Fuss der Blende (Pegel null) zum ersten, von
                    // ihrem Kopf (Grundpegel) zum zweiten Anfasser — die
                    // Ausblendung laeuft rueckwaerts, ihr Fuss liegt rechts
                    QRect br;
                    int x0 = 0;
                    int x1 = 0;
                    Blenden b;
                    if (!blendenBereich(k, c, ein, &br, &x0, &x1, &b)) continue;
                    const int unten0 = r.bottom() - 1;
                    const int oben0 =
                        r.bottom() - 1 - static_cast<int>(clip.grundpegel * (r.height() - 3) + 0.5);
                    p.setPen(QPen(QColor(255, 220, 120), 1.0));
                    p.drawLine(QPoint(ein ? x0 : x1, unten0), a1);
                    p.drawLine(QPoint(ein ? x1 : x0, oben0), a2);
                    p.setBrush(QColor(255, 220, 120));
                    p.drawEllipse(a1, 4, 4);
                    p.drawEllipse(a2, 4, 4);
                }
                // Griffe der Stuecke: eine Raute in der Mitte (verschieben), je
                // ein Strich an den Kanten (Weite)
                p.setPen(QPen(QColor(30, 30, 30), 1.0));
                p.setBrush(Qt::white);
                for (int s = 0; s < static_cast<int>(clip.stuecke.size()); ++s)
                {
                    const QPoint mitte = stueckGriff(k, c, s, 0);
                    if (mitte.x() < 0) continue;
                    p.drawPolygon(QPolygon({QPoint(mitte.x(), mitte.y() - 4), QPoint(mitte.x() + 4, mitte.y()),
                                            QPoint(mitte.x(), mitte.y() + 4), QPoint(mitte.x() - 4, mitte.y())}));
                    for (const int seite : {-1, 1})
                    {
                        const QPoint kante = stueckGriff(k, c, s, seite);
                        if (kante.x() >= 0) p.drawRect(QRect(kante.x() - 1, kante.y() - 4, 3, 8));
                    }
                }
                p.setBrush(Qt::NoBrush);
            }
        }
    }
    p.setClipRect(zeilenFlaeche);
    p.setPen(linie);
    p.drawLine(kKopfBreite - 1, 0, kKopfBreite - 1, unten);

    // --- ein Drag von aussen: die Zeile, in die er faellt, und die Stelle
    if (m_fallOrt.x() >= 0 && zeileBei(m_fallOrt.y()) >= 0)
    {
        const int k = kanalBei(m_fallOrt.y());
        const int zeile = k >= 0 ? k : zeilen;  // unter der letzten Zeile: ein neuer Kanal
        const QRect ziel(kKopfBreite, zeileOben(zeile), width() - kKopfBreite, zeileHoch(zeile));
        // Der Schatten (Wunsch Patrik S98): jeder Clip dort, wo er landen
        // wuerde — mit seinem Anfang als Strich. Faellt hier nichts (eine
        // Zeile der anderen Art), bleibt die Zeile dunkel.
        const std::vector<Schatten> schatten = fallSchatten(m_fallOrt, m_fallDateien, m_fallTasten);
        if (!schatten.empty())
        {
            QColor hell = pal.color(QPalette::Highlight);
            hell.setAlpha(40);
            p.fillRect(ziel, hell);
        }
        p.setClipRect(zeilenZeit);
        for (const Schatten& s : schatten)
        {
            VisualClip geist;
            geist.startMs = s.startMs;
            geist.endeMs = s.endeMs;
            const QRect r = clipRechteck(s.kanal, geist);  // Zeile = Kanal, auch fuer den neuen unter der letzten
            // die Farbe des Zielkanals — unter der letzten Zeile die, die der neue Kanal bekaeme
            QColor farbe = s.kanal < zeilen
                               ? kanalFarbe(m_komposition, s.kanal, true)
                               : zeichenFarbe(m_komposition.freieFarbe(m_fallTon ? KanalArt::Audio
                                                                                 : KanalArt::Visual),
                                              true);
            farbe.setAlpha(120);
            p.setBrush(farbe);
            p.setPen(QPen(pal.color(QPalette::Highlight), 1.0, Qt::DashLine));
            p.drawRoundedRect(r, 4, 4);
            p.setBrush(Qt::NoBrush);
        }
        if (!schatten.empty())
        {
            const int xFall = xVon(schatten.front().startMs);
            p.setPen(QPen(pal.color(QPalette::Highlight), 2.0));
            p.drawLine(xFall, ziel.top(), xFall, ziel.bottom());
        }
        p.setClipRect(zeilenFlaeche);
        if (k < 0 && !schatten.empty())
        {
            p.setPen(text);
            p.drawText(QRect(8, ziel.top(), kKopfBreite - 12, ziel.height()),
                       Qt::AlignLeft | Qt::AlignVCenter,
                       m_fallTon ? tr("New audio channel") : tr("New channel"));
        }
    }

    // --- die Einfuegemarke (S98): ihre Zeile leicht hervorgehoben, ein Strich
    //     mit einem Dreieck am Kopf — dorthin legt „Einfuegen" den Anfang
    if (m_bearbeiten && m_markeKanal >= 0 && m_markeKanal < zeilen)
    {
        const int oben = zeileOben(m_markeKanal);
        const int hoch = zeileHoch(m_markeKanal);
        QColor zeile = pal.color(QPalette::Highlight);
        zeile.setAlpha(28);
        p.fillRect(QRect(0, oben, width(), hoch), zeile);
        p.setClipRect(zeilenZeit);
        const int xMarke = xVon(m_markeMs);
        const QColor farbe = pal.color(QPalette::Highlight);
        p.setPen(QPen(farbe, 2.0));
        p.drawLine(xMarke, oben, xMarke, oben + hoch);
        p.setBrush(farbe);
        p.drawPolygon(QPolygon({QPoint(xMarke - 5, oben), QPoint(xMarke + 5, oben), QPoint(xMarke, oben + 7)}));
        p.setBrush(Qt::NoBrush);
        p.setClipRect(zeilenFlaeche);
    }

    // --- ein Kanal wird am Namen gezogen (S100): seine Zeile hervorgehoben, ein
    //     Strich an der Kante der Zeile, an der er landet
    if (m_kanalZug >= 0 && m_kanalZiel >= 0 && m_kanalZug < zeilen)
    {
        QColor hell = pal.color(QPalette::Highlight);
        hell.setAlpha(45);
        p.fillRect(QRect(0, zeileOben(m_kanalZug), width(), zeileHoch(m_kanalZug)), hell);
        if (m_kanalZiel != m_kanalZug)
        {
            // nach unten gezogen landet er UNTER der Zielzeile, nach oben ueber ihr
            const int y = zeileOben(m_kanalZiel + (m_kanalZiel > m_kanalZug ? 1 : 0));
            p.setPen(QPen(pal.color(QPalette::Highlight), 3.0));
            p.drawLine(0, y, width(), y);
        }
    }
    p.setClipRect(zeilenZeit);
    auflage();

    // --- zweiter Gang: das Lineal, ueber allem, was darunter gerollt ist
    p.setClipRect(linealFlaeche);
    p.fillRect(linealFlaeche, pal.color(QPalette::AlternateBase));
    p.setClipRect(linealZeit);
    unterlage();
    p.setClipRect(linealFlaeche);
    p.setPen(linie);
    p.drawLine(kKopfBreite - 1, 0, kKopfBreite - 1, unten);
    p.setClipRect(linealZeit);
    auflage();
}

void Zeitachse::setBearbeiten(bool an)
{
    if (an == m_bearbeiten) return;
    brichZugAb();
    m_bearbeiten = an;
    if (!an) unsetCursor();
    if (!an) m_markeKanal = -1;  // die Einfuegemarke gibt es nur im Bearbeiten
    updateGeometry();
    passeRollenAn();
    update();
}

void Zeitachse::zeigeBearbeitet(const Komposition& komposition)
{
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    m_komposition = komposition;
    setzeLaenge(anzeigeLaenge(komposition));
    setzeStelleStill(std::min(stelle(), komposition.laenge()));
    if (auswahl() == nullptr)  // den Clip gibt es nicht mehr
    {
        m_kanal = -1;
        m_clip = -1;
    }
    if (m_markeKanal >= static_cast<int>(komposition.kanaele.size())) m_markeKanal = -1;
    updateGeometry();
    passeRollenAn();
    update();
    Q_EMIT auswahlGeaendert();  // auch bei gleicher Auswahl: der Clip kann neue Zeiten haben
}

std::int64_t Zeitachse::raste(std::int64_t zeitMs, const Komposition& in, int ausserKanal,
                              int ausserClip) const
{
    // so viele Millisekunden sind acht Pixel
    const std::int64_t nah =
        laenge() > 0 ? static_cast<std::int64_t>(8.0 * msJePixel())
                       : 0;
    std::int64_t beste = -1;
    std::int64_t abstand = nah + 1;
    const auto pruefe = [&](std::int64_t kante) {
        const std::int64_t a = kante > zeitMs ? kante - zeitMs : zeitMs - kante;
        if (a < abstand)
        {
            abstand = a;
            beste = kante;
        }
    };
    for (int k = 0; k < static_cast<int>(in.kanaele.size()); ++k)
    {
        const auto& clips = in.kanaele[static_cast<std::size_t>(k)].clips;
        for (int c = 0; c < static_cast<int>(clips.size()); ++c)
        {
            if (k == ausserKanal && c == ausserClip) continue;
            pruefe(clips[static_cast<std::size_t>(c)].startMs);
            pruefe(clips[static_cast<std::size_t>(c)].endeMs);
        }
    }
    pruefe(stelle());
    if (beste >= 0) return beste;
    const auto raster = static_cast<std::int64_t>(in.rasterSekunden * 1000.0 + 0.5);
    if (raster <= 0) return zeitMs;
    return (zeitMs + raster / 2) / raster * raster;
}

QPoint Zeitachse::stueckGriff(int kanal, int clip, int stueck, int welcher) const
{
    Clip ablage;
    const Clip* zeiger = clipIn(m_komposition, kanal, clip, ablage);
    if (zeiger == nullptr) return {-1, -1};
    const VisualClip& c = *zeiger;
    if (stueck < 0 || stueck >= static_cast<int>(c.stuecke.size())) return {-1, -1};
    const QRect r = clipRechteck(zeileVon(kanal), c);  // Zeile = Kanal (CH1 oben)
    if (r.width() < 24) return {-1, -1};     // zu schmal fuer Griffe
    // wo das Stueck gerade liegt — gestaucht, wenn der Platz nicht reicht
    const Bereich b = c.verteilung().stuecke[static_cast<std::size_t>(stueck)];
    const int x0 = xVon(c.startMs + b.vonMs);
    const int x1 = xVon(c.startMs + b.bisMs);
    if (welcher == 0) return {(x0 + x1) / 2, r.top() + 5};
    if (x1 - x0 < 28) return {-1, -1};  // die Kanten laegen auf der Raute
    return {welcher < 0 ? x0 : x1, r.top() + 5};
}

Zeitachse::Griff Zeitachse::griffBei(QPoint ort, int* kanal, int* clip, int* stueck) const
{
    *kanal = -1;
    *clip = -1;
    if (stueck != nullptr) *stueck = -1;
    const int zeile = zeileBei(ort.y());
    if (zeile == kKeineZeile) return Griff::Keiner;  // das Lineal
    if (ort.x() < kKopfBreite) return Griff::Keiner; // die Spalte der Kanalnamen deckt, was darunter rollt
    // die Zeile unter dem Lineal: die Kurve der ganzen Komposition (S100) — ein
    // Clip ueber die ganze Laenge, mit den Griffen der Kurve, ohne Kanten
    // (S101: vier solche Zeilen — MASTER, VISUAL, AUDIO, KARAOKE)
    const bool gesamt = zeile < 0;
    if (!gesamt && zeile >= static_cast<int>(m_komposition.kanaele.size())) return Griff::Keiner;
    const int k = gesamt ? masterVonZeile(zeile) : kanalVonZeile(zeile);
    std::vector<Clip> eine;
    if (Clip ablage; gesamt && clipIn(m_komposition, k, 0, ablage) != nullptr) eine.push_back(ablage);
    const auto& clips = gesamt ? eine : m_komposition.kanaele[static_cast<std::size_t>(k)].clips;
    for (int c = 0; c < static_cast<int>(clips.size()); ++c)
    {
        const QRect r = clipRechteck(zeile, clips[static_cast<std::size_t>(c)]);
        if (!r.contains(ort)) continue;
        *kanal = k;
        *clip = c;
        // Blenden (C4): zuerst die Bézier-Anfasser des angeklickten Clips,
        // dann die Griffe oben im Clip — sie gehen den Kanten vor
        const auto nah = [&ort](QPoint p) { return p.x() >= 0 && (ort - p).manhattanLength() <= 8; };
        // in zu niedrigen Zeilen gibt es keine Punkte der Kurve zu greifen (ZO3)
        const bool punkte = kurvenpunkteZiehbar();
        if (punkte && k == m_kanal && c == m_clip)
        {
            if (nah(bezierGriff(k, c, true, 0))) return Griff::BezierEin1;
            if (nah(bezierGriff(k, c, true, 1))) return Griff::BezierEin2;
            if (nah(bezierGriff(k, c, false, 0))) return Griff::BezierAus1;
            if (nah(bezierGriff(k, c, false, 1))) return Griff::BezierAus2;
        }
        if (punkte && ort.y() <= r.top() + 11)
        {
            // Stuecke (C4 Teil 2): die Raute in der Mitte geht vor, dann die
            // Blenden, zuletzt die Kanten der Stuecke
            const int stuecke = static_cast<int>(clips[static_cast<std::size_t>(c)].stuecke.size());
            const auto trifft = [&](int welcher, Griff griff) {
                for (int s = 0; s < stuecke; ++s)
                {
                    if (!nah(stueckGriff(k, c, s, welcher))) continue;
                    if (stueck != nullptr) *stueck = s;
                    return griff;
                }
                return Griff::Keiner;
            };
            if (trifft(0, Griff::StueckLage) != Griff::Keiner) return Griff::StueckLage;
            if (nah(blendenGriff(k, c, true))) return Griff::BlendeEin;
            if (nah(blendenGriff(k, c, false))) return Griff::BlendeAus;
            if (trifft(-1, Griff::StueckLinks) != Griff::Keiner) return Griff::StueckLinks;
            if (trifft(1, Griff::StueckRechts) != Griff::Keiner) return Griff::StueckRechts;
        }
        // Die Kurve der Komposition laesst sich nicht verschieben. Ihre rechte
        // Kante ist das Ende der Komposition: steht es fest (Fixed end), laesst
        // es sich hier ziehen (Wunsch Patrik S100) — sonst folgt es dem letzten Clip.
        // Nur an der Zeile MASTER — VISUAL und AUDIO haben keine Kante (S101).
        if (gesamt)
            return k == kMaster && m_komposition.laengeMs > 0 && r.right() - ort.x() <= 6 ? Griff::KanteRechts
                                                                                         : Griff::Keiner;
        // Kanten: die aeusseren sechs Pixel — bei schmalen Clips nur rechts
        if (r.width() > 16 && ort.x() - r.left() <= 6) return Griff::KanteLinks;
        if (r.right() - ort.x() <= 6) return Griff::KanteRechts;
        return Griff::Verschieben;
    }
    return Griff::Keiner;
}

bool Zeitachse::blendenBereich(int kanal, int clip, bool ein, QRect* clipRect, int* x0, int* x1,
                               Blenden* blenden) const
{
    Clip ablage;
    const Clip* zeiger = clipIn(m_komposition, kanal, clip, ablage);
    if (zeiger == nullptr) return false;
    const VisualClip& c = *zeiger;
    // Die Laenge laesst sich an jeder Blende ziehen — auch an einer, deren Form
    // eigene Punkte hat (dann bleibt `blenden` fuer diese Seite ohne Form).
    (void)liesBlenden(c, *blenden);
    *clipRect = clipRechteck(zeileVon(kanal), c);  // Zeile = Kanal (CH1 oben)
    if (clipRect->width() < 24) return false;  // zu schmal fuer Griffe
    // wo die Blende gerade liegt — gestaucht, wenn der Platz nicht reicht
    const Verteilung v = c.verteilung();
    const Bereich& bereich = ein ? v.ein : v.aus;
    *x0 = xVon(c.startMs + bereich.vonMs);
    *x1 = xVon(c.startMs + bereich.bisMs);
    return true;
}

QPoint Zeitachse::blendenGriff(int kanal, int clip, bool ein) const
{
    QRect r;
    int x0 = 0;
    int x1 = 0;
    Blenden b;
    if (!blendenBereich(kanal, clip, ein, &r, &x0, &x1, &b)) return {-1, -1};
    return {std::clamp(ein ? x1 : x0, r.left() + 4, r.right() - 4), r.top() + 5};
}

QPoint Zeitachse::bezierGriff(int kanal, int clip, bool ein, int nr) const
{
    QRect r;
    int x0 = 0;
    int x1 = 0;
    Blenden b;
    if (!blendenBereich(kanal, clip, ein, &r, &x0, &x1, &b)) return {-1, -1};
    Clip ablage;
    const VisualClip& c = *clipIn(m_komposition, kanal, clip, ablage);  // es gibt ihn: `blendenBereich`
    // Anfasser hier nur fuer eine Blende mit EINEM Bézier-Abschnitt; eine Form
    // mit eigenen Punkten gehoert in den Curve Editor
    const ClipBlende& blende = ein ? c.ein : c.aus;
    if (blende.dauerMs <= 0 || !einfach(blende.form) ||
        blende.form.punkte.front().form != Kurvenform::Bezier || x1 - x0 < 8)
        return {-1, -1};
    const BezierGriffe& g = blende.form.punkte.front().bezier;
    const double bx = g[static_cast<std::size_t>(nr * 2)];
    const double by = g[static_cast<std::size_t>(nr * 2 + 1)];
    // die Ausblendung ist dieselbe Form, rueckwaerts gelesen (an der y-Achse gespiegelt)
    const double x = ein ? bx : 1.0 - bx;
    const double pegel = by * c.grundpegel;
    return {x0 + static_cast<int>(x * (x1 - x0) + 0.5),
            r.bottom() - 1 - static_cast<int>(pegel * (r.height() - 3) + 0.5)};
}

void Zeitachse::setzeBlendenForm(int kanal, int clip, bool ein, Kurvenform form)
{
    if (!m_bearbeiten || !m_geladen) return;
    QRect r;
    int x0 = 0;
    int x1 = 0;
    Blenden b;
    if (!blendenBereich(kanal, clip, ein, &r, &x0, &x1, &b)) return;
    Komposition neu = m_komposition;
    Clip ablage;
    VisualClip c = *clipIn(neu, kanal, clip, ablage);  // es gibt ihn: `blendenBereich`
    // nur diese Seite: die andere behaelt ihre Form, auch eine mit eigenen Punkten
    ClipBlende& blende = ein ? c.ein : c.aus;
    const BezierGriffe griffe =
        einfach(blende.form) ? blende.form.punkte.front().bezier : kBezierVorgabe;
    blende.form = einfacheBlende(form, griffe);
    if (blende.dauerMs <= 0) blende.dauerMs = std::min<std::int64_t>(2000, c.dauerMs() / 4);
    schreibeClip(neu, kanal, clip, c);
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Change fade shape"));
}

void Zeitachse::setzeBlendenPreset(int kanal, int clip, bool ein, const QString& name)
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    Clip ablage;
    const Clip* zeiger = clipIn(neu, kanal, clip, ablage);
    if (zeiger == nullptr) return;
    Pegelform form;
    if (!formpresets::lade(formpresets::finde(FormArt::Blende, name).pfad, FormArt::Blende, form)) return;
    VisualClip c = *zeiger;
    ClipBlende& blende = ein ? c.ein : c.aus;
    blende.form = form;
    if (blende.dauerMs <= 0) blende.dauerMs = std::min<std::int64_t>(2000, c.dauerMs() / 4);
    schreibeClip(neu, kanal, clip, c);
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Change fade shape"));
}

void Zeitachse::setzeStueckPreset(int kanal, int clip, int stueck, const QString& name)
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    Clip ablage;
    const Clip* zeiger = clipIn(neu, kanal, clip, ablage);
    if (zeiger == nullptr) return;
    VisualClip c = *zeiger;
    if (stueck < 0 || stueck >= static_cast<int>(c.stuecke.size())) return;
    Pegelform form;
    if (!formpresets::lade(formpresets::finde(FormArt::Stueck, name).pfad, FormArt::Stueck, form)) return;
    c.stuecke[static_cast<std::size_t>(stueck)].form = form;
    schreibeClip(neu, kanal, clip, c);
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Change shape curve"));
}

void Zeitachse::setTeil(Teil teil)
{
    if (teil == m_teil) return;
    m_teil = teil;
    update();
}

void Zeitachse::oeffneKurve(int kanal, int clip, Teil teil)
{
    Clip ablage;
    const Clip* zeiger = m_geladen ? clipIn(m_komposition, kanal, clip, ablage) : nullptr;
    if (zeiger == nullptr) return;
    if (!hatTeil(*zeiger, teil))
    {
        // eine Blende, die es noch nicht gibt, entsteht hier — sonst gaebe es nichts zu zeichnen
        if (!m_bearbeiten || (teil.art != TeilArt::Ein && teil.art != TeilArt::Aus)) return;
        Komposition neu = m_komposition;
        VisualClip c = *zeiger;
        (teil.art == TeilArt::Ein ? c.ein : c.aus).dauerMs = std::min<std::int64_t>(2000, c.dauerMs() / 4);
        schreibeClip(neu, kanal, clip, c);
        uebernimm(neu, kanal, clip);
        Q_EMIT bearbeitet(neu, tr("Add fade"));
    }
    else if (kanal != m_kanal || clip != m_clip)
    {
        m_kanal = kanal;
        m_clip = clip;
        update();
        Q_EMIT auswahlGeaendert();
    }
    Q_EMIT kurveOeffnen(teil);
}

void Zeitachse::stueckHinzu(int kanal, int clip, std::int64_t zeitImClipMs)
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    Clip ablage;
    const Clip* zeiger = clipIn(neu, kanal, clip, ablage);
    if (zeiger == nullptr) return;
    VisualClip c = *zeiger;
    const int stueck = fuegeStueckEin(c, zeitImClipMs);
    schreibeClip(neu, kanal, clip, c);
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Add shape"));
    Q_EMIT kurveOeffnen({TeilArt::Stueck, stueck});
}

void Zeitachse::stueckLoeschen(int kanal, int clip, int stueck)
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    Clip ablage;
    const Clip* zeiger = clipIn(neu, kanal, clip, ablage);
    if (zeiger == nullptr) return;
    VisualClip c = *zeiger;
    if (!loescheStueck(c, stueck)) return;
    schreibeClip(neu, kanal, clip, c);
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Delete shape"));
}

void Zeitachse::gainZuruecksetzen(int kanal, int clip)
{
    if (!m_bearbeiten || !m_geladen || istMaster(kanal)) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    Kanal& k = neu.kanaele[static_cast<std::size_t>(kanal)];
    if (k.art != KanalArt::Audio || clip < 0 || clip >= static_cast<int>(k.clips.size())) return;
    Clip& c = k.clips[static_cast<std::size_t>(clip)];
    if (c.gainDb == 0.0) return;
    c.gainDb = 0.0;
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Reset gain"));
}

void Zeitachse::clipSchalten(int kanal, int clip, bool an)
{
    if (!m_bearbeiten || !m_geladen || istMaster(kanal)) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    Kanal& k = neu.kanaele[static_cast<std::size_t>(kanal)];
    if (clip < 0 || clip >= static_cast<int>(k.clips.size())) return;
    Clip& c = k.clips[static_cast<std::size_t>(clip)];
    if (c.an == an) return;
    c.an = an;
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, an ? tr("Enable clip") : tr("Disable clip"));
}

void Zeitachse::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_geladen || event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    int kanal = -1;
    int clip = -1;
    (void)griffBei(ort, &kanal, &clip);
    if (clip < 0) return;
    // der erste Klick des Doppelklicks hat einen Zug vorbereitet — er gilt nicht
    brichZugAb();
    Clip ablage;
    const VisualClip& c = *clipIn(m_komposition, kanal, clip, ablage);  // `griffBei` hat ihn gefunden
    const Teil teil = teilBei(c, stelleVon(ort.x()) - c.startMs);
    if (teil.art != TeilArt::Keins) oeffneKurve(kanal, clip, teil);
}

void Zeitachse::mousePressEvent(QMouseEvent* event)
{
    merkeTaste(true);  // so lange laeuft der Ausschnitt nicht mit (ZO6)
    // Schwenken (S102, Entscheid Patrik ZO7: a): die mittlere Maustaste zieht
    // den Ausschnitt — nicht mitten in einem anderen Zug
    if (m_geladen && event->button() == Qt::MiddleButton && m_griff == Griff::Keiner && !m_zieht &&
        m_kanalZug < 0)
    {
        beginneSchwenken(event->position().toPoint());
        return;
    }
    if (!m_geladen || event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    if (ort.y() < kLinealHoehe)
    {
        m_zieht = true;
        // ueber der Spalte der Kanalnamen: die erste sichtbare Stelle
        Q_EMIT sprung(stelleVon(std::max(ort.x(), kKopfBreite)));
        return;
    }
    // Am Kanalnamen (linke Spalte) laesst sich der Kanal an einen anderen Platz
    // ziehen (S100, Entscheid Patrik V1 a) — vorbereitet wird es hier, gezogen
    // erst, wenn die Maus sich bewegt.
    if (m_bearbeiten && ort.x() < kKopfBreite && kanalBei(ort.y()) >= 0)
    {
        m_kanalZug = kanalBei(ort.y());
        m_kanalZiel = -1;
        m_kanalZugStart = ort;
    }
    int kanal = -1;
    int clipIndex = -1;
    int stueck = -1;
    const Griff griff = griffBei(ort, &kanal, &clipIndex, &stueck);
    if (m_bearbeiten && griff != Griff::Keiner)
    {
        m_griff = griff;
        m_zugStueck = stueck;
        m_zugLaeuft = false;
        m_zugStart = ort;
        m_zugKanal = kanal;
        m_zugClip = clipIndex;
        m_vorZug = m_komposition;
        Clip ablage;
        m_zugVersatzMs = stelleVon(ort.x()) - clipIn(m_komposition, kanal, clipIndex, ablage)->startMs;
    }
    // Eine freie Stelle in einer Kanalzeile: dort steht ab jetzt die
    // Einfuegemarke (Entscheid Patrik S98) — eingerastet wie ein Clip, Alt frei.
    if (m_bearbeiten && griff == Griff::Keiner && clipIndex < 0 && ort.x() >= kKopfBreite)
    {
        if (const int zeile = kanalBei(ort.y()); zeile >= 0)
        {
            std::int64_t stelle = stelleVon(ort.x());
            if (!(event->modifiers() & Qt::AltModifier)) stelle = raste(stelle, m_komposition, -1, -1);
            m_markeKanal = zeile;
            m_markeMs = stelle;
            m_kanal = -1;
            m_clip = -1;
            update();
            Q_EMIT auswahlGeaendert();
            return;
        }
    }
    if (kanal == m_kanal && clipIndex == m_clip) return;
    m_kanal = kanal;
    m_clip = clipIndex;
    update();
    Q_EMIT auswahlGeaendert();
}

void Zeitachse::zieheZu(QPoint ort, Qt::KeyboardModifiers tasten)
{
    // Entscheid Patrik S95: ohne Taste anhalten, Ctrl kuerzen, Ctrl+Shift schieben
    const Stoss stoss = !(tasten & Qt::ControlModifier) ? Stoss::Halten
                        : (tasten & Qt::ShiftModifier)  ? Stoss::Schieben
                                                        : Stoss::Kuerzen;
    const bool frei = (tasten & Qt::AltModifier) != 0;
    const auto gerastet = [&](std::int64_t zeit) {
        return frei ? zeit : raste(zeit, m_vorZug, m_zugKanal, m_zugClip);
    };
    Clip ablage;
    const Clip* zeiger = clipIn(m_vorZug, m_zugKanal, m_zugClip, ablage);
    if (zeiger == nullptr) return;
    const VisualClip& alt = *zeiger;
    const std::int64_t maus = stelleVon(ort.x());

    m_komposition = m_vorZug;
    int platz = -1;
    int ziel = m_zugKanal;
    switch (m_griff)
    {
        case Griff::Verschieben:
        {
            // die Zeile unter der Maus: der Clip wechselt den Kanal — nur in
            // einen seiner Art (Bild zu Bild, Ton zu Ton)
            const int zeilen = static_cast<int>(m_vorZug.kanaele.size());
            const int unterMaus = zeileBei(ort.y());
            const int zeile = std::clamp(unterMaus, 0, zeilen - 1);
            if (unterMaus >= 0 &&
                m_vorZug.kanaele[static_cast<std::size_t>(kanalVonZeile(zeile))].art ==
                    m_vorZug.kanaele[static_cast<std::size_t>(m_zugKanal)].art)
                ziel = kanalVonZeile(zeile);
            // einrasten darf der Anfang oder das Ende — was weniger weit weg liegt
            const std::int64_t wunsch = maus - m_zugVersatzMs;
            const std::int64_t vorn = gerastet(wunsch);
            const std::int64_t hinten = gerastet(wunsch + alt.dauerMs()) - alt.dauerMs();
            const auto weg = [wunsch](std::int64_t s) { return s > wunsch ? s - wunsch : wunsch - s; };
            platz = versetzeClip(m_komposition, m_zugKanal, m_zugClip, ziel,
                                 weg(hinten) < weg(vorn) ? hinten : vorn, stoss);
            break;
        }
        case Griff::KanteLinks:
            platz = zieheKante(m_komposition, m_zugKanal, m_zugClip, true, gerastet(maus), stoss);
            break;
        case Griff::KanteRechts:
            if (m_zugKanal == kMaster)
            {
                // die rechte Kante der Zeile MASTER: das feste Ende der Komposition
                m_komposition.laengeMs = std::max<std::int64_t>(100, gerastet(maus));
                platz = m_zugClip;
                break;
            }
            platz = zieheKante(m_komposition, m_zugKanal, m_zugClip, false, gerastet(maus), stoss);
            break;
        case Griff::BlendeEin:
        case Griff::BlendeAus:
        case Griff::BezierEin1:
        case Griff::BezierEin2:
        case Griff::BezierAus1:
        case Griff::BezierAus2:
        {
            // Entscheid Patrik S96: ein gestauchtes Teil von Hand aendern heisst,
            // erst den sichtbaren Stand festschreiben — danach bewegt sich nur,
            // was angefasst wird
            VisualClip fest = alt;
            schreibeFest(fest);
            const std::int64_t dauer = fest.dauerMs();
            // die Laenge: nur die Dauer aendert sich, die Form bleibt — auch eine mit eigenen Punkten
            if (m_griff == Griff::BlendeEin)
                fest.ein.dauerMs = std::clamp<std::int64_t>(
                    gerastet(maus) - alt.startMs, 0, std::max<std::int64_t>(0, dauer - fest.aus.dauerMs));
            else if (m_griff == Griff::BlendeAus)
                fest.aus.dauerMs = std::clamp<std::int64_t>(
                    alt.endeMs - gerastet(maus), 0, std::max<std::int64_t>(0, dauer - fest.ein.dauerMs));
            else
            {
                // ein Bézier-Anfasser: seine Lage im Rechteck der Blende, je 0…1
                const bool ein = m_griff == Griff::BezierEin1 || m_griff == Griff::BezierEin2;
                const std::size_t nr = (m_griff == Griff::BezierEin2 || m_griff == Griff::BezierAus2) ? 1 : 0;
                const QRect r = clipRechteck(zeileVon(m_zugKanal), alt);
                const Verteilung v = fest.verteilung();
                const Bereich& bereich = ein ? v.ein : v.aus;
                const int x0 = xVon(alt.startMs + bereich.vonMs);
                const int x1 = xVon(alt.startMs + bereich.bisMs);
                const double x = std::clamp(static_cast<double>(ort.x() - x0) / std::max(1, x1 - x0), 0.0, 1.0);
                const double hoehe = std::max(1.0, fest.grundpegel * (r.height() - 3));
                const double pegel =
                    std::clamp(static_cast<double>(r.bottom() - 1 - ort.y()) / hoehe, 0.0, 1.0);
                Pegelform& form = (ein ? fest.ein : fest.aus).form;
                if (!einfach(form)) return;
                BezierGriffe& g = form.punkte.front().bezier;
                g[nr * 2] = ein ? x : 1.0 - x;  // die Ausblendung laeuft rueckwaerts
                g[nr * 2 + 1] = pegel;
            }
            schreibeClip(m_komposition, m_zugKanal, m_zugClip, fest);
            platz = m_zugClip;
            break;
        }
        case Griff::StueckLage:
        case Griff::StueckLinks:
        case Griff::StueckRechts:
        {
            // Beide schreiben zuerst den sichtbaren Stand fest (Entscheid Patrik
            // S96) und halten an den Nachbarn an: gezogen wird nie ins Stauchen.
            VisualClip fest = alt;
            const std::int64_t imClip = gerastet(maus) - alt.startMs;
            if (m_griff == Griff::StueckLage)
            {
                if (!verschiebeStueck(fest, m_zugStueck, imClip)) return;
            }
            else
            {
                // die Mitte bleibt: der Abstand der Maus zur Mitte ist die halbe Weite
                const Bereich jetzt = bereichVon(alt, {TeilArt::Stueck, m_zugStueck});
                const std::int64_t mitte = (jetzt.vonMs + jetzt.bisMs) / 2;
                const std::int64_t halb = imClip > mitte ? imClip - mitte : mitte - imClip;
                if (!setzeStueckWeite(fest, m_zugStueck, 2 * halb)) return;
            }
            schreibeClip(m_komposition, m_zugKanal, m_zugClip, fest);
            platz = m_zugClip;
            break;
        }
        case Griff::Keiner:
            return;
    }
    m_kanal = ziel;
    m_clip = platz;
    // die Laenge der Achse bleibt im Zug, wie sie war: aenderte sich der Massstab unter
    // der Maus, liefe der Clip ihr davon. Neu gemessen wird beim Loslassen.
    update();
}

void Zeitachse::brichZugAb()
{
    m_kanalZug = -1;  // auch ein Kanal, der gerade am Namen gezogen wird, bleibt, wo er war
    m_kanalZiel = -1;
    if (m_griff == Griff::Keiner) return;
    if (m_zugLaeuft)
    {
        m_komposition = m_vorZug;
        m_kanal = m_zugKanal;
        m_clip = m_zugClip;
        setzeLaenge(anzeigeLaenge(m_komposition));
        passeRollenAn();
    }
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    update();
}

void Zeitachse::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (schwenkt())
    {
        schwenkeZu(ort);
        return;
    }
    // die Zeit unter dem Zeiger, solange er im Lineal steht (Wunsch Patrik S98)
    if (m_geladen && ort.y() < kLinealHoehe && ort.x() >= kKopfBreite)
        QToolTip::showText(event->globalPosition().toPoint(), ComposerPanel::zeitText(stelleVon(ort.x())), this,
                           QRect(ort.x(), 0, 1, kLinealHoehe));
    if (m_zieht)
    {
        Q_EMIT sprung(stelleVon(std::max(ort.x(), kKopfBreite)));
        return;
    }
    if (m_kanalZug >= 0 && (event->buttons() & Qt::LeftButton))
    {
        // ein Kanal wird am Namen gezogen: er fiele auf die Zeile unter der Maus
        if (m_kanalZiel < 0 && (ort - m_kanalZugStart).manhattanLength() < 4) return;
        const int zeilen = static_cast<int>(m_komposition.kanaele.size());
        // ueber dem ersten Kanal (Master-Zeilen, Lineal): an den ersten Platz
        m_kanalZiel = std::clamp(zeileBei(ort.y()), 0, std::max(0, zeilen - 1));
        setCursor(Qt::ClosedHandCursor);
        update();
        return;
    }
    if (m_griff != Griff::Keiner && (event->buttons() & Qt::LeftButton))
    {
        if (!m_zugLaeuft && (ort - m_zugStart).manhattanLength() < 4) return;
        m_zugLaeuft = true;
        zieheZu(ort, event->modifiers());
        return;
    }
    if (m_bearbeiten && m_geladen)
    {
        int kanal = -1;
        int clip = -1;
        const Griff griff = griffBei(ort, &kanal, &clip);
        if (ort.x() < kKopfBreite && kanalBei(ort.y()) >= 0)
            setCursor(Qt::OpenHandCursor);  // am Kanalnamen: der Kanal laesst sich verschieben
        else if (griff == Griff::KanteLinks || griff == Griff::KanteRechts)
            setCursor(Qt::SizeHorCursor);
        else if (griff != Griff::Keiner && griff != Griff::Verschieben)
            setCursor(Qt::PointingHandCursor);  // ein Griff der Blende
        else
            unsetCursor();
    }
}

void Zeitachse::mouseReleaseEvent(QMouseEvent* event)
{
    merkeTaste(event->buttons() != Qt::NoButton);
    if (schwenkt())
    {
        if (event->button() != Qt::MiddleButton) return;  // eine andere Taste: das Schwenken laeuft weiter
        beendeSchwenken();
        return;
    }
    m_zieht = false;
    if (m_kanalZug >= 0)
    {
        // der Kanal faellt auf seinen neuen Platz — ohne Bewegung war es nur ein Klick
        const int von = m_kanalZug;
        const int nach = m_kanalZiel;
        m_kanalZug = -1;
        m_kanalZiel = -1;
        unsetCursor();
        update();
        if (nach >= 0 && nach != von) kanalVerschieben(von, nach);
    }
    if (m_griff == Griff::Keiner) return;
    const Griff griff = m_griff;
    const bool gezogen = m_zugLaeuft;
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    if (!gezogen) return;
    setzeLaenge(anzeigeLaenge(m_komposition));
    passeRollenAn();
    update();
    if (gleich(m_komposition, m_vorZug)) return;
    Q_EMIT auswahlGeaendert();  // der Clip hat neue Zeiten
    QString was = tr("Change fade");
    switch (griff)
    {
        case Griff::Verschieben: was = tr("Move clip"); break;
        case Griff::KanteLinks:
        case Griff::KanteRechts: was = m_kanal == kMaster ? tr("Change the end") : tr("Resize clip"); break;
        case Griff::StueckLage: was = tr("Move shape"); break;
        case Griff::StueckLinks:
        case Griff::StueckRechts: was = tr("Change shape width"); break;
        default: break;
    }
    Q_EMIT bearbeitet(m_komposition, was);
}

void Zeitachse::keyPressEvent(QKeyEvent* event)
{
    if (m_bearbeiten && event->key() == Qt::Key_Escape && m_kanalZug >= 0)
    {
        // einen Kanal gezogen: Esc laesst ihn, wo er war
        m_kanalZug = -1;
        m_kanalZiel = -1;
        unsetCursor();
        update();
        return;
    }
    if (m_bearbeiten && event->key() == Qt::Key_Escape && m_griff != Griff::Keiner)
    {
        brichZugAb();
        return;
    }
    if (m_bearbeiten && m_griff == Griff::Keiner && auswahl() != nullptr &&
        (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace))
    {
        loescheAuswahl();
        return;
    }
    // Ctrl+E: den angeklickten Clip ein- und ausschalten (S126, D1 c)
    if (m_bearbeiten && m_griff == Griff::Keiner && auswahl() != nullptr && !gesamtGewaehlt() &&
        event->key() == Qt::Key_E && event->modifiers() == Qt::ControlModifier)
    {
        clipSchalten(m_kanal, m_clip, !auswahl()->an);
        return;
    }
    if (m_geladen && m_griff == Griff::Keiner)
    {
        if (event->matches(QKeySequence::Copy) && kopiere()) return;
        if (event->matches(QKeySequence::Cut) && schneideAus()) return;
        if (event->matches(QKeySequence::Paste) && fuegeEin()) return;
    }
    QWidget::keyPressEvent(event);
}

void Zeitachse::loescheAuswahl()
{
    if (istMaster(m_kanal)) return;  // eine Kurve der Komposition laesst sich nicht loeschen
    Komposition neu = m_komposition;
    if (!loescheClip(neu, m_kanal, m_clip)) return;
    uebernimm(neu, -1, -1);
    Q_EMIT bearbeitet(neu, tr("Delete clip"));
}

void Zeitachse::uebernimm(const Komposition& neu, int kanal, int clip)
{
    m_komposition = neu;
    setzeLaenge(anzeigeLaenge(neu));
    m_kanal = kanal;
    m_clip = clip;
    if (m_markeKanal >= static_cast<int>(neu.kanaele.size())) m_markeKanal = -1;
    updateGeometry();
    passeRollenAn();
    update();
    Q_EMIT auswahlGeaendert();
}

// -----------------------------------------------------------------------------
// Einfuegemarke, Kopieren / Ausschneiden / Einfuegen (S98)
// -----------------------------------------------------------------------------

void Zeitachse::setMarke(int kanal, std::int64_t stelleMs)
{
    if (!m_bearbeiten || !m_geladen || kanal >= static_cast<int>(m_komposition.kanaele.size())) kanal = -1;
    m_markeKanal = kanal;
    m_markeMs = std::max<std::int64_t>(0, stelleMs);
    update();
    Q_EMIT auswahlGeaendert();  // die Clip-Zeile des Panels nennt die Marke
}

bool Zeitachse::ablageVoll(bool* ton)
{
    if (ton != nullptr) *ton = g_ablage.art == KanalArt::Audio;
    return g_ablage.voll;
}

void Zeitachse::leereAblage()
{
    g_ablage = {};
}

bool Zeitachse::kopiere()
{
    const VisualClip* clip = auswahl();
    if (clip == nullptr || istMaster(m_kanal)) return false;  // eine Kurve der Komposition ist kein Clip
    g_ablage.voll = true;
    g_ablage.clip = *clip;
    g_ablage.art = m_komposition.kanaele[static_cast<std::size_t>(m_kanal)].art;
    return true;
}

bool Zeitachse::schneideAus()
{
    if (!m_bearbeiten || m_griff != Griff::Keiner || !kopiere()) return false;
    Komposition neu = m_komposition;
    if (!loescheClip(neu, m_kanal, m_clip)) return false;
    uebernimm(neu, -1, -1);
    Q_EMIT bearbeitet(neu, tr("Cut clip"));
    return true;
}

bool Zeitachse::fuegeEin()
{
    if (!m_bearbeiten || !m_geladen || !g_ablage.voll || m_griff != Griff::Keiner) return false;
    int kanal = m_markeKanal;
    std::int64_t start = m_markeMs;
    if (kanal < 0)
    {
        // Ohne Marke (von mir gesetzt, S98): hinter den angeklickten Clip, in
        // dessen Kanal — Kopieren und gleich Einfuegen verdoppelt ihn.
        const VisualClip* clip = auswahl();
        if (clip == nullptr || istMaster(m_kanal)) return false;
        kanal = m_kanal;
        start = clip->endeMs;
    }
    Komposition neu = m_komposition;
    const int platz = fuegeKopieEin(neu, kanal, g_ablage.art, g_ablage.clip, start);
    if (platz < 0) return false;  // ein Kanal der anderen Art
    // die Marke rueckt hinter den eingefuegten Clip: noch einmal Einfuegen haengt an
    if (m_markeKanal >= 0)
        m_markeMs = neu.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(platz)].endeMs;
    uebernimm(neu, kanal, platz);
    Q_EMIT bearbeitet(neu, tr("Paste clip"));
    return true;
}

bool Zeitachse::event(QEvent* event)
{
    // Kopieren, Ausschneiden, Einfuegen gehoeren hier der Zeitachse — auch wenn
    // das Hauptfenster dieselben Tasten als Kuerzel fuehrt.
    if (event->type() == QEvent::ShortcutOverride && m_geladen)
    {
        auto* taste = static_cast<QKeyEvent*>(event);
        if (taste->matches(QKeySequence::Copy) || taste->matches(QKeySequence::Cut) ||
            taste->matches(QKeySequence::Paste))
        {
            event->accept();
            return true;
        }
    }
    // ein Name, der in seine niedrige Zeile nicht passt, steht im Tooltip (ZO3)
    if (event->type() == QEvent::ToolTip)
    {
        auto* hilfe = static_cast<QHelpEvent*>(event);
        if (const QString name = versteckterName(hilfe->pos()); !name.isEmpty())
        {
            QToolTip::showText(hilfe->globalPos(), name, this);
            return true;
        }
    }
    return QWidget::event(event);
}

// -----------------------------------------------------------------------------
// Neues hinein (C3b)
// -----------------------------------------------------------------------------

int Zeitachse::kanalBei(int y) const
{
    const int zeile = zeileBei(y);
    if (zeile < 0) return -1;  // Lineal und die Zeilen der Kurven der Komposition
    return zeile < static_cast<int>(m_komposition.kanaele.size()) ? kanalVonZeile(zeile) : -1;
}

std::int64_t Zeitachse::songLaenge(const QString& datei) const
{
    // Ein Loop, den das Loop-Panel offen hat: seine Laenge aendert sich beim
    // Bearbeiten — sie wird nicht gemerkt, sondern dort erfragt (S109).
    if (istLoop(datei))
    {
        lumi::loop::Loop offen;
        if (lumi::loop::offenerLoop(datei, offen))
        {
            offen.richte();
            return static_cast<std::int64_t>(std::llround(offen.durchlaufMs()));
        }
    }
    if (!m_songLaenge) return 0;
    const auto gemerkt = m_songLaengen.constFind(datei);
    if (gemerkt != m_songLaengen.constEnd()) return *gemerkt;
    const std::int64_t laenge = m_songLaenge(datei);
    m_songLaengen.insert(datei, laenge);
    return laenge;
}

bool Zeitachse::lasseFallen(QPoint ort, const QStringList& dateien, Qt::KeyboardModifiers tasten)
{
    Komposition neu = m_komposition;
    int kanal = -1;
    int platz = -1;
    QStringList passend;
    if (!legeDateien(neu, ort, dateien, tasten, &kanal, &platz, &passend, nullptr)) return false;
    const bool ton = neu.kanaele[static_cast<std::size_t>(kanal)].art == KanalArt::Audio;
    uebernimm(neu, kanal, platz);
    Q_EMIT bearbeitet(neu, ton ? (passend.size() == 1 ? tr("Add song") : tr("Add songs"))
                               : (passend.size() == 1 ? tr("Add clip") : tr("Add clips")));
    Q_EMIT hineingezogen(passend);
    return true;
}

std::vector<Zeitachse::Schatten> Zeitachse::fallSchatten(QPoint ort, const QStringList& dateien,
                                                         Qt::KeyboardModifiers tasten) const
{
    std::vector<Schatten> gelegt;
    Komposition probe = m_komposition;
    int kanal = -1;
    int platz = -1;
    QStringList passend;
    if (!legeDateien(probe, ort, dateien, tasten, &kanal, &platz, &passend, &gelegt)) gelegt.clear();
    return gelegt;
}

bool Zeitachse::legeDateien(Komposition& neu, QPoint ort, const QStringList& dateien,
                            Qt::KeyboardModifiers tasten, int* kanalAus, int* platzAus,
                            QStringList* passendAus, std::vector<Schatten>* gelegt) const
{
    if (!m_bearbeiten || !m_geladen || dateien.isEmpty() || m_griff != Griff::Keiner) return false;
    if (zeileBei(ort.y()) < 0) return false;  // Lineal und Master-Zeilen
    int kanal = kanalBei(ort.y());

    // Songs fallen nur in Audio-Kanaele, Presets nur in Visual-Kanaele (C5).
    // Die Zeile bestimmt, was faellt; unter der letzten Zeile die erste Datei.
    const auto istSong = [this](const QString& datei) {
        return m_audioEndungen.contains(QFileInfo(datei).suffix(), Qt::CaseInsensitive);
    };
    const bool ton = kanal >= 0 ? neu.kanaele[static_cast<std::size_t>(kanal)].art == KanalArt::Audio
                                : istSong(dateien.first());
    QStringList passend;
    for (const QString& datei : dateien)
        if (istSong(datei) == ton) passend << datei;
    if (passend.isEmpty()) return false;
    // unter der letzten Zeile: ein neuer Kanal dieser Art
    if (kanal < 0) kanal = fuegeKanalEin(neu, {}, ton ? KanalArt::Audio : KanalArt::Visual);

    const Stoss stoss = !(tasten & Qt::ControlModifier) ? Stoss::Halten
                        : (tasten & Qt::ShiftModifier)  ? Stoss::Schieben
                                                        : Stoss::Kuerzen;
    std::int64_t start = stelleVon(std::max(ort.x(), kKopfBreite));
    if (!(tasten & Qt::AltModifier)) start = raste(start, m_komposition, -1, -1);
    int platz = -1;
    for (const QString& datei : passend)  // mehrere Dateien: hintereinander
    {
        // ein Song ist so lang, wie er ist — ohne Auskunft so lang wie ein
        // neuer Bild-Clip; dessen Laenge steht in den Settings (S98)
        platz = ton ? fuegeSongEin(neu, kanal, datei, start, songLaenge(datei), stoss)
                    : fuegeClipEin(neu, kanal, datei, start, Sitzung::neuerClipMs(), stoss);
        if (platz < 0) return false;
        const Clip& c = neu.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(platz)];
        if (gelegt != nullptr) gelegt->push_back(Schatten{kanal, c.startMs, c.endeMs});
        start = c.endeMs;
    }
    *kanalAus = kanal;
    *platzAus = platz;
    *passendAus = passend;
    return true;
}

QStringList Zeitachse::fallDateien(const QMimeData* daten) const
{
    return lumi::ui::presetDateien(daten) + lumi::ui::dateienMitEndung(daten, m_audioEndungen);
}

void Zeitachse::setHuelle(const QString& datei, std::vector<float> huelle, int msProWert)
{
    m_huellen.insert(datei, Huelle{std::move(huelle), msProWert});
    update();
}

void Zeitachse::kanalHinzu()
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    (void)fuegeKanalEin(neu);
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, tr("Add channel"));
}

void Zeitachse::audioKanalHinzu()
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    (void)fuegeKanalEin(neu, {}, KanalArt::Audio);
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, tr("Add audio channel"));
}

void Zeitachse::kanalLoeschen(int kanal)
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    if (!loescheKanal(neu, kanal)) return;
    uebernimm(neu, -1, -1);
    Q_EMIT bearbeitet(neu, tr("Delete channel"));
}

void Zeitachse::kanalUmbenennen(int kanal, const QString& name)
{
    if (!m_bearbeiten || !m_geladen || name.trimmed().isEmpty()) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    neu.kanaele[static_cast<std::size_t>(kanal)].name = name.trimmed();
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, tr("Rename channel"));
}

void Zeitachse::kanalVerschieben(int von, int nach)
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    if (!verschiebeKanal(neu, von, nach)) return;
    // Auswahl und Einfuegemarke bleiben bei ihrem Kanal (die Kurve der Komposition bleibt, was sie ist)
    if (m_markeKanal >= 0) m_markeKanal = platzNachVerschieben(m_markeKanal, von, nach);
    uebernimm(neu, m_kanal >= 0 ? platzNachVerschieben(m_kanal, von, nach) : m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, tr("Move channel"));
}

void Zeitachse::kanaeleSortieren(bool bildZuerst)
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    const std::vector<int> platz = sortiereNachArt(neu, bildZuerst);
    if (platz.empty()) return;  // lag schon so
    const auto neuerPlatz = [&platz](int kanal) {
        return kanal >= 0 && kanal < static_cast<int>(platz.size()) ? platz[static_cast<std::size_t>(kanal)]
                                                                    : kanal;
    };
    m_markeKanal = neuerPlatz(m_markeKanal);
    uebernimm(neu, neuerPlatz(m_kanal), m_clip);
    Q_EMIT bearbeitet(neu, tr("Sort channels"));
}

void Zeitachse::kanalFaerben(int kanal, std::int32_t farbe)
{
    if (!m_bearbeiten || !m_geladen) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    Kanal& k = neu.kanaele[static_cast<std::size_t>(kanal)];
    // unter null: zurueck auf die Reihe — die erste Farbe seiner Art, die kein anderer traegt
    k.farbe = -1;
    k.farbe = farbe >= 0 ? (farbe & 0xFFFFFF) : neu.freieFarbe(k.art);
    if (k.farbe == m_komposition.farbeVon(kanal) &&
        m_komposition.kanaele[static_cast<std::size_t>(kanal)].farbe >= 0)
        return;  // dieselbe Farbe: kein Schritt fuer Undo
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, farbe >= 0 ? tr("Change channel color") : tr("Reset channel color"));
}

void Zeitachse::kanalSchalten(int kanal, bool an)
{
    if (!m_bearbeiten || !m_geladen) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    neu.kanaele[static_cast<std::size_t>(kanal)].an = an;
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, an ? tr("Enable channel") : tr("Disable channel"));
}

void Zeitachse::dragEnterEvent(QDragEnterEvent* event)
{
    if (m_bearbeiten && m_geladen && !fallDateien(event->mimeData()).isEmpty())
    {
        // nur Songs: unter der letzten Zeile entstuende ein Audio-Kanal
        m_fallTon = lumi::ui::presetDateien(event->mimeData()).isEmpty();
        m_fallOrt = event->position().toPoint();
        m_fallDateien = fallDateien(event->mimeData());
        m_fallTasten = event->modifiers();
        event->acceptProposedAction();
        update();
    }
}

void Zeitachse::dragMoveEvent(QDragMoveEvent* event)
{
    if (m_fallOrt.x() < 0) return;
    m_fallOrt = event->position().toPoint();
    m_fallTasten = event->modifiers();
    // im Lineal faellt nichts — und nichts in eine Zeile der anderen Art
    if (zeileBei(m_fallOrt.y()) < 0 || fallSchatten(m_fallOrt, m_fallDateien, m_fallTasten).empty())
        event->ignore();
    else
        event->acceptProposedAction();
    update();
}

void Zeitachse::dragLeaveEvent(QDragLeaveEvent*)
{
    m_fallOrt = QPoint(-1, -1);
    update();
}

void Zeitachse::dropEvent(QDropEvent* event)
{
    m_fallOrt = QPoint(-1, -1);
    update();
    if (lasseFallen(event->position().toPoint(), fallDateien(event->mimeData()), event->modifiers()))
        event->acceptProposedAction();
}

void Zeitachse::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_geladen) return;
    const QPoint ort = event->pos();
    int kanal = -1;
    int clip = -1;
    (void)griffBei(ort, &kanal, &clip);
    // S130 (Wunsch Patrik): ein Loop-Clip laesst sich im Panel Loop oeffnen — auch ohne Edit
    const auto loopEintrag = [&](QMenu& m) {
        if (clip < 0 || istMaster(kanal)) return;
        const Kanal& k = m_komposition.kanaele[static_cast<std::size_t>(kanal)];
        if (k.art != KanalArt::Audio) return;
        const QString datei = k.clips[static_cast<std::size_t>(clip)].datei;
        if (!istLoop(datei)) return;
        QAction* a = m.addAction(tr("Open in Loop panel"), this, [this, datei] { Q_EMIT loopImPanelOeffnen(datei); });
        a->setObjectName(QStringLiteral("composerOpenInLoopPanel"));
        // Stufe 30c (WX6 a): der Loop ueber Versatz und Laenge des Clips als WAV
        QAction* e = m.addAction(tr("Export audio..."), this, [this, kanal, clip] { Q_EMIT loopExportieren(kanal, clip); });
        e->setObjectName(QStringLiteral("composerExportLoopAudio"));
        m.addSeparator();
    };
    if (!m_bearbeiten)
    {
        // ohne Edit gibt es nichts zu aendern — der Zoom gehoert nicht dazu (S102)
        QMenu nurZoom(this);
        loopEintrag(nurZoom);
        fuelleZoomMenue(nurZoom);
        (void)lumi::ui::zeigeMenue(nurZoom, event->globalPos());
        return;
    }
    const int zeile = kanalBei(ort.y());

    QMenu menu(this);
    if (clip >= 0)
    {
        if (kanal != m_kanal || clip != m_clip)
        {
            m_kanal = kanal;
            m_clip = clip;
            update();
            Q_EMIT auswahlGeaendert();
        }
        // Blenden (C4): die Form je Seite. Eine Form mit eigenen Punkten traegt
        // keinen Haken — eine Wahl hier ersetzt sie durch die einfache Form.
        Clip ablage;  // traegt die Kurve der Komposition, wenn ihre Zeile gemeint ist (S100)
        {
            const VisualClip& gewaehlt = *clipIn(m_komposition, kanal, clip, ablage);
            // Die Liste sind die Blenden-Presets — mitgelieferte und eigene; der
            // Haken sitzt an dem, dessen Form die Blende gerade traegt.
            const std::vector<formpresets::Eintrag> presets = formpresets::liste(FormArt::Blende);
            for (const bool ein : {true, false})
            {
                const ClipBlende& blende = ein ? gewaehlt.ein : gewaehlt.aus;
                const bool zeigt = blende.dauerMs > 0 && einfach(blende.form);
                QMenu* formen = menu.addMenu(ein ? tr("Fade in") : tr("Fade out"));
                bool gefunden = false;
                for (const formpresets::Eintrag& e : presets)
                {
                    QAction* a = formen->addAction(e.name);
                    a->setCheckable(true);
                    Pegelform f;
                    if (!gefunden && blende.dauerMs > 0 && formpresets::lade(e.pfad, FormArt::Blende, f) &&
                        gleich(f, blende.form))
                    {
                        a->setChecked(true);
                        gefunden = true;
                    }
                    connect(a, &QAction::triggered, this, [this, kanal, clip, ein, name = e.name] {
                        setzeBlendenPreset(kanal, clip, ein, name);
                    });
                }
                if (!presets.empty()) continue;
                // ohne Preset-Ordner (die App findet `asset/composer` nicht): die sieben festen Formen
                for (int f = 0; f < kKurvenformen; ++f)
                {
                    const auto form = static_cast<Kurvenform>(f);
                    QAction* a = formen->addAction(QString::fromLatin1(kurvenformName(form)));
                    a->setCheckable(true);
                    a->setChecked(zeigt && blende.form.punkte.front().form == form);
                    connect(a, &QAction::triggered, this, [this, kanal, clip, ein, form] {
                        setzeBlendenForm(kanal, clip, ein, form);
                    });
                }
            }
            menu.addSeparator();
        }
        // die Kurven im Curve Editor (C4 Teil 2) und die Stuecke mitten im Clip
        {
            const VisualClip& c = *clipIn(m_komposition, kanal, clip, ablage);
            const std::int64_t zeit = stelleVon(ort.x()) - c.startMs;
            const Teil unter = teilBei(c, zeit);
            menu.addAction(tr("Edit fade in curve..."), this,
                           [this, kanal, clip] { oeffneKurve(kanal, clip, {TeilArt::Ein, -1}); });
            menu.addAction(tr("Edit fade out curve..."), this,
                           [this, kanal, clip] { oeffneKurve(kanal, clip, {TeilArt::Aus, -1}); });
            if (unter.art == TeilArt::Stueck)
            {
                // die Stueck-Presets, wie bei den Blenden
                const ClipStueck& stueck = c.stuecke[static_cast<std::size_t>(unter.stueck)];
                QMenu* formen = menu.addMenu(tr("Shape"));
                bool gefunden = false;
                for (const formpresets::Eintrag& e : formpresets::liste(FormArt::Stueck))
                {
                    QAction* a = formen->addAction(e.name);
                    a->setCheckable(true);
                    Pegelform f;
                    if (!gefunden && formpresets::lade(e.pfad, FormArt::Stueck, f) && gleich(f, stueck.form))
                    {
                        a->setChecked(true);
                        gefunden = true;
                    }
                    connect(a, &QAction::triggered, this, [this, kanal, clip, unter, name = e.name] {
                        setzeStueckPreset(kanal, clip, unter.stueck, name);
                    });
                }
                formen->setEnabled(!formen->isEmpty());
                menu.addAction(tr("Edit shape curve..."), this,
                               [this, kanal, clip, unter] { oeffneKurve(kanal, clip, unter); });
                menu.addAction(tr("Delete shape"), this,
                               [this, kanal, clip, unter] { stueckLoeschen(kanal, clip, unter.stueck); });
            }
            menu.addAction(tr("Add shape here"), this,
                           [this, kanal, clip, zeit] { stueckHinzu(kanal, clip, zeit); });
            menu.addSeparator();
        }
        if (!istMaster(kanal))  // eine Kurve der Komposition ist kein Clip
        {
            const VisualClip& c = *clipIn(m_komposition, kanal, clip, ablage);
            // S126, D1 c: den Clip ein- und ausschalten, wie den Kanal
            QAction* clipAn = menu.addAction(tr("Clip enabled"));
            clipAn->setObjectName(QStringLiteral("composerClipEnabled"));
            clipAn->setCheckable(true);
            clipAn->setChecked(c.an);
            clipAn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
            clipAn->setEnabled(m_bearbeiten);
            connect(clipAn, &QAction::triggered, this,
                    [this, kanal, clip](bool ein) { clipSchalten(kanal, clip, ein); });
            if (m_komposition.kanaele[static_cast<std::size_t>(kanal)].art == KanalArt::Audio)
            {
                // S126, G2 a: nur an einem Ton-Clip mit Gain
                if (c.gainDb != 0.0)
                {
                    QAction* zurueck = menu.addAction(tr("Reset gain (%1)").arg(ComposerPanel::gainText(c.gainDb)),
                                                      this, [this, kanal, clip] { gainZuruecksetzen(kanal, clip); });
                    zurueck->setObjectName(QStringLiteral("composerResetGain"));
                    zurueck->setEnabled(m_bearbeiten);
                }
                // S127, G5-4 a: der Dialog Match loudness mit diesem Clip als Ziel
                QAction* abgleich = menu.addAction(tr("Match loudness to this clip..."), this,
                                                   [this, kanal, clip] { Q_EMIT lautheitAbgleichen(kanal, clip); });
                abgleich->setObjectName(QStringLiteral("composerMatchToClip"));
                abgleich->setEnabled(m_bearbeiten);
                menu.addSeparator();
                loopEintrag(menu);
            }
            menu.addAction(tr("Copy clip"), this, [this] { (void)kopiere(); })->setShortcut(QKeySequence::Copy);
            menu.addAction(tr("Cut clip"), this, [this] { (void)schneideAus(); })->setShortcut(QKeySequence::Cut);
            menu.addAction(tr("Delete clip"), this, [this] { loescheAuswahl(); });
            menu.addSeparator();
        }
    }
    if (zeile >= 0)
    {
        const Kanal& k = m_komposition.kanaele[static_cast<std::size_t>(zeile)];
        // Einfuegen (S98): an der Stelle des Klicks — sie wird zur Einfuegemarke
        if (clip < 0 && ort.x() >= kKopfBreite)
        {
            bool ton = false;
            const bool voll = ablageVoll(&ton);
            QAction* hier = menu.addAction(tr("Paste clip here"), this, [this, zeile, ort] {
                setMarke(zeile, raste(stelleVon(ort.x()), m_komposition, -1, -1));
                (void)fuegeEin();
            });
            hier->setShortcut(QKeySequence::Paste);
            hier->setEnabled(voll && ton == (k.art == KanalArt::Audio));
            menu.addSeparator();
        }
        menu.addAction(tr("Rename channel..."), this, [this, zeile, alt = k.name] {
            bool ok = false;
            const QString name = QInputDialog::getText(this, tr("Rename Channel"), tr("Name:"),
                                                       QLineEdit::Normal, alt, &ok);
            if (ok) kanalUmbenennen(zeile, name);
        });
        QAction* an = menu.addAction(tr("Channel enabled"));
        an->setCheckable(true);
        an->setChecked(k.an);
        connect(an, &QAction::triggered, this, [this, zeile](bool ein) { kanalSchalten(zeile, ein); });
        // den Kanal verschieben (S100, Entscheid Patrik V1 a) — auch durch Ziehen am Namen
        const int letzter = static_cast<int>(m_komposition.kanaele.size()) - 1;
        menu.addAction(tr("Move channel up"), this, [this, zeile] { kanalVerschieben(zeile, zeile - 1); })
            ->setEnabled(zeile > 0);
        menu.addAction(tr("Move channel down"), this, [this, zeile] { kanalVerschieben(zeile, zeile + 1); })
            ->setEnabled(zeile < letzter);
        // die Farbe des Kanals (S100, Entscheid Patrik K1 b)
        menu.addAction(tr("Channel color..."), this, [this, zeile] {
            const QColor jetzt = zeichenFarbe(m_komposition.farbeVon(zeile), true);
            const QColor gewaehlt = QColorDialog::getColor(jetzt, this, tr("Channel Color"));
            if (gewaehlt.isValid()) kanalFaerben(zeile, static_cast<std::int32_t>(gewaehlt.rgb() & 0xFFFFFFu));
        });
        menu.addAction(tr("Reset channel color"), this, [this, zeile] { kanalFaerben(zeile, -1); });
        menu.addAction(tr("Delete channel"), this, [this, zeile] { kanalLoeschen(zeile); });
        menu.addSeparator();
    }
    menu.addAction(tr("Add channel"), this, [this] { kanalHinzu(); });
    menu.addAction(tr("Add audio channel"), this, [this] { audioKanalHinzu(); });
    // nach Art ordnen (S100, Entscheid Patrik S1 a): innerhalb der Art bleibt die
    // Reihenfolge — das Bild aendert sich nicht. Grau, was schon so liegt.
    QMenu* sortieren = menu.addMenu(tr("Sort channels"));
    sortieren->addAction(tr("Visuals first"), this, [this] { kanaeleSortieren(true); })
        ->setEnabled(!nachArtSortiert(m_komposition, true));
    sortieren->addAction(tr("Audio first"), this, [this] { kanaeleSortieren(false); })
        ->setEnabled(!nachArtSortiert(m_komposition, false));
    menu.addSeparator();
    fuelleZoomMenue(menu);
    (void)lumi::ui::zeigeMenue(menu, event->globalPos());
}

// =============================================================================
// ZeitachsenRahmen
// =============================================================================

ZeitachsenRahmen::ZeitachsenRahmen(Zeitachse* achse, QWidget* parent) : lumi::ui::AchsenRahmen(achse, parent)
{
    setObjectName(QStringLiteral("composerZeitachsenRahmen"));
    balkenSenkrecht()->setObjectName(QStringLiteral("composerRollenSenkrecht"));
    balkenWaagrecht()->setObjectName(QStringLiteral("composerRollenWaagrecht"));
}

// =============================================================================
// ComposerPanel
// =============================================================================

ComposerPanel::ComposerPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "composer", tr("Composer"), parent)
{
    // Ueber diesen Bus haengt die Sitzung Effect Chain an einen Clip (C3c).
    Sitzung::setzeBus(eventBus());
    setupUI();
    zeigeKomposition();
}

ComposerPanel::~ComposerPanel()
{
    Sitzung::setzeBus(nullptr);  // der Bus gehoert den Diensten des Panels
    // der Thread, der eine Huellkurve holt: abbrechen und auf ihn warten
    if (m_huellAbbruch) m_huellAbbruch->store(true);
    if (m_huellFaden.joinable()) m_huellFaden.join();
}

void ComposerPanel::ladeHuellen()
{
    if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
    for (const Kanal& kanal : m_zeitachse->komposition().kanaele)
    {
        if (kanal.art != KanalArt::Audio) continue;
        for (const Clip& clip : kanal.clips)
            // ein Loop ist keine Tondatei, die sich dekodieren liesse: er hat keine Wellenform
            if (!istLoop(clip.datei) && !m_zeitachse->hatHuelle(clip.datei) && clip.datei != m_huellLaeuft &&
                !m_huellWarten.contains(clip.datei))
                m_huellWarten << clip.datei;
    }
    starteHuelle();
}

void ComposerPanel::starteHuelle()
{
    if (!m_huellLaeuft.isEmpty() || m_huellWarten.isEmpty()) return;
    auto* engine = services().tryResolve<IAudioEngine>();
    if (engine == nullptr)
    {
        m_huellWarten.clear();  // ohne Engine keine Wellenform
        return;
    }
    if (m_huellFaden.joinable()) m_huellFaden.join();  // der vorige ist fertig
    m_huellLaeuft = m_huellWarten.takeFirst();
    m_huellAbbruch = std::make_shared<std::atomic<bool>>(false);
    // Die ganze Datei zu dekodieren dauert spuerbar — in einem eigenen Thread
    // (wie Lyrics Sync); das Ergebnis kommt ueber die Ereignisschleife zurueck.
    const auto abbruch = m_huellAbbruch;
    const QString datei = m_huellLaeuft;
    const QPointer<ComposerPanel> panel(this);
    m_huellFaden = std::thread([engine, datei, abbruch, panel]() {
        std::vector<float> huelle = engine->probePeaks(datei, kHuelleMs, abbruch.get());
        if (abbruch->load()) return;
        QMetaObject::invokeMethod(
            qApp,
            [panel, datei, abbruch, huelle = std::move(huelle)]() mutable {
                if (!panel || abbruch->load()) return;
                panel->m_zeitachse->setHuelle(datei, std::move(huelle), kHuelleMs);
                panel->m_huellLaeuft.clear();
                panel->starteHuelle();  // der naechste
            },
            Qt::QueuedConnection);
    });
}

Sitzung* ComposerPanel::sitzung() const
{
    return m_sitzung;
}

QString ComposerPanel::zeitText(std::int64_t ms)
{
    ms = std::max<std::int64_t>(0, ms);
    const std::int64_t zehntel = ms / 100;
    return QStringLiteral("%1:%2.%3")
        .arg(zehntel / 600)
        .arg((zehntel / 10) % 60, 2, 10, QLatin1Char('0'))
        .arg(zehntel % 10);
}

QString ComposerPanel::gainText(double db)
{
    return QStringLiteral("%1%2 dB").arg(db > 0.0 ? QStringLiteral("+") : QString()).arg(db, 0, 'f', 1);
}

void ComposerPanel::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);

    // --- Kopf
    auto* kopf = new QHBoxLayout();
    m_an = new QCheckBox(tr("Composer on"), this);
    m_an->setObjectName(QStringLiteral("composerAn"));
    m_an->setToolTip(tr("On: the composition delivers the picture. Off: the effect chain does — "
                        "the composition stays loaded here and can be prepared."));
    kopf->addWidget(m_an);
    kopf->addSpacing(8);

    const auto knopf = [this, kopf](const char* name, QStyle::StandardPixmap bild) {
        auto* k = new QPushButton(this);
        k->setObjectName(QLatin1String(name));
        k->setIcon(style()->standardIcon(bild));
        k->setFixedSize(32, 32);
        kopf->addWidget(k);
        return k;
    };
    m_spielen = knopf("composerPlay", QStyle::SP_MediaPlay);
    m_stopp = knopf("composerStop", QStyle::SP_MediaStop);
    m_stopp->setToolTip(tr("Stop and return to the start"));
    m_schleife = knopf("composerLoop", QStyle::SP_BrowserReload);
    m_schleife->setCheckable(true);
    m_resume = knopf("composerResume", QStyle::SP_MediaSeekForward);
    m_resume->setCheckable(true);

    m_zeit = new QLabel(this);
    m_zeit->setObjectName(QStringLiteral("composerZeit"));
    kopf->addSpacing(8);
    kopf->addWidget(m_zeit);
    kopf->addSpacing(12);
    m_edit = new QCheckBox(tr("Edit"), this);
    m_edit->setObjectName(QStringLiteral("composerEdit"));
    m_edit->setToolTip(
        tr("On: clips can be changed.\n"
           "Drag a clip to move it, drag its left or right edge to change its length, "
           "Del removes the selected clip. Edit → Undo takes a step back.\n\n"
           "When the dragged clip meets a neighbour:\n"
           "no key — it stops there\n"
           "Ctrl — the neighbour is shortened\n"
           "Ctrl+Shift — the neighbours are pushed along\n\n"
           "Dragging snaps to the grid, to the edges of other clips and to the play "
           "position; hold Alt to drag freely. Esc cancels a drag.\n\n"
           "Off: the panel only shows and plays the composition."));
    kopf->addWidget(m_edit);
    m_sync = new QCheckBox(tr("Sync on Effect Chain"), this);
    m_sync->setObjectName(QStringLiteral("composerSync"));
    m_sync->setToolTip(
        tr("While a composition delivers the picture:\n\n"
           "On: the Effect Chain panel shows the chain of the clip selected here. Changes "
           "act at once, in the mixed picture.\n\n"
           "Off: the Effect Chain panel rests.\n\n"
           "With the Composer off, Effect Chain shows the chain of the app as always."));
    kopf->addWidget(m_sync);
    // Karaoke (S101, Entscheid Patrik KS1: a): derselbe Schalter wie „Karaoke
    // on" im Panel Karaoke, im Menue View und auf Ctrl+K — hier noch einmal,
    // weil die Zeile KARAOKE nur wirkt, wenn die Ebene an ist.
    m_karaoke = new QCheckBox(tr("Karaoke"), this);
    m_karaoke->setObjectName(QStringLiteral("composerKaraoke"));
    m_karaoke->setToolTip(
        tr("Switches the karaoke layer on and off — the same switch as \"Karaoke on\" in the Karaoke "
           "panel, View → Karaoke and Ctrl+K.\n\n"
           "On: the lyrics lie over the composition, as strongly as the KARAOKE row says.\n"
           "Off: no lyrics; the KARAOKE row has no effect."));
    m_karaoke->setChecked(MultiEffectVisualizer::karaokeEbene());
    connect(m_karaoke, &QCheckBox::clicked, this,
            [this](bool an) { KaraokePanel::setzeKaraokeAn(eventBus(), an); });
    if (IEventBus* bus = eventBus())
        m_eventSubscriptions.push_back(bus->subscribeScoped<KaraokeEnabledChangedEvent>(
            [this](const KaraokeEnabledChangedEvent& e) { m_karaoke->setChecked(e.enabled); }));
    kopf->addWidget(m_karaoke);
    // Mute fuer Durchsagen (S101, Entscheid Patrik AM2: c): derselbe Knopf wie
    // im Panel Player — beide zeigen denselben Zustand
    kopf->addWidget(new MuteKnopf(services(), this));
    // Scroll with playhead (S102, Entscheid Patrik ZO6): hineingezoomt folgt der
    // Ausschnitt der laufenden Abspielmarke. Nicht „Follow player" — der koppelt
    // Start und Stopp mit dem Player (Konzept §4).
    m_mitlaufen = new QCheckBox(tr("Scroll with playhead"), this);
    m_mitlaufen->setObjectName(QStringLiteral("composerScrollWithPlayhead"));
    m_mitlaufen->setToolTip(
        tr("Zoomed into the time, the timeline shows only a part of the composition.\n\n"
           "On: while the composition plays, the view follows the playhead — smoothly, with the "
           "playhead in the middle, or page by page (Settings → Panels → Scroll with playhead).\n"
           "Off: the view stays where you put it.\n\n"
           "While a mouse button is held down in the timeline, the view never moves by itself."));
    m_mitlaufen->setChecked(Sitzung::mitlaufenAn());
    connect(m_mitlaufen, &QCheckBox::toggled, this, [](bool an) { Sitzung::setMitlaufenAn(an); });
    // Fit (S102, Entscheid Patrik ZO8: a): setzt den Zoom zurueck, sein Tooltip nennt die Gesten
    auto* fit = new QPushButton(tr("Fit"), this);
    fit->setObjectName(QStringLiteral("composerFit"));
    fit->setToolTip(tr("Shows the whole composition again: its full length on the width, the rows at "
                       "normal height (Ctrl+0).\n\nIn the timeline:\n%1")
                        .arg(Zeitachse::zoomGesten().join(QLatin1Char('\n'))));
    kopf->addStretch(1);
    root->addLayout(kopf);

    // --- Datei-Zeile (eine eigene Zeile: der Kopf waere sonst breiter als ein
    //     schmal angedocktes Panel)
    kopf = new QHBoxLayout();
    m_neu = new QPushButton(tr("New"), this);
    m_neu->setObjectName(QStringLiteral("composerNew"));
    m_neu->setToolTip(tr("Start a new, empty composition with one channel. Drag presets from "
                         "the Effect Browser into it."));
    kopf->addWidget(m_neu);
    m_laden = new QPushButton(tr("Load..."), this);
    m_laden->setObjectName(QStringLiteral("composerLoad"));
    m_laden->setToolTip(tr("Open a composition — the same as File → Open Composition..."));
    kopf->addWidget(m_laden);
    m_speichern = new QPushButton(tr("Save"), this);
    m_speichern->setObjectName(QStringLiteral("composerSave"));
    m_speichern->setToolTip(tr("Write the changes into the loaded file"));
    kopf->addWidget(m_speichern);
    m_speichernUnter = new QPushButton(tr("Save As..."), this);
    m_speichernUnter->setObjectName(QStringLiteral("composerSaveAs"));
    kopf->addWidget(m_speichernUnter);
    kopf->addSpacing(8);
    // das Datei-Icon der Komposition vor dem Namen (S100, Entscheid Patrik I1 a)
    m_dateiIcon = new QLabel(this);
    m_dateiIcon->setObjectName(QStringLiteral("composerDateiIcon"));
    if (!lumi::ui::presetIconDir().isEmpty())
        m_dateiIcon->setPixmap(
            lumi::ui::presetTypeIcon(lumi::ui::PresetIconKind::Komposition).pixmap(18, 18));
    m_dateiIcon->setVisible(false);
    kopf->addWidget(m_dateiIcon);
    m_datei = new QLabel(this);
    m_datei->setObjectName(QStringLiteral("composerDatei"));
    kopf->addWidget(m_datei);
    kopf->addStretch(1);
    // Scroll with playhead und Fit (S102) stehen rechts in dieser Zeile: in der
    // ersten machten sie das Panel breiter (seine schmalste Breite bestimmt die erste)
    kopf->addWidget(m_mitlaufen);
    kopf->addWidget(fit);
    root->addLayout(kopf);

    // --- Zeitachse: sie rollt bei vielen Kanaelen selbst, das Lineal bleibt
    //     stehen (S102, Entscheid Patrik ZO4: a); der Rahmen stellt den Rollbalken
    m_zeitachse = new Zeitachse(this);
    root->addWidget(new ZeitachsenRahmen(m_zeitachse, this), 1);
    // die Hoehe der Zeilen merkt die App — fuer jede Komposition, ueber den
    // Neustart (Entscheid Patrik ZO9: a); der Zeit-Zoom wird nicht gemerkt
    m_zeitachse->setZeilenProzent(QSettings().value(kZeilenProzent, 100).toInt());
    connect(m_zeitachse, &Zeitachse::zeilenProzentGeaendert, this,
            [](int prozent) { QSettings().setValue(kZeilenProzent, prozent); });
    // Fit und Ctrl+0 (Vorgabe Patrik S100): das Kuerzel gilt, solange der
    // Fokus irgendwo im Panel sitzt
    connect(fit, &QPushButton::clicked, m_zeitachse, &Zeitachse::setzeZoomZurueck);
    auto* fitKuerzel = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_0), this);
    fitKuerzel->setObjectName(QStringLiteral("composerFitShortcut"));
    fitKuerzel->setContext(Qt::WidgetWithChildrenShortcut);
    connect(fitKuerzel, &QShortcut::activated, m_zeitachse, &Zeitachse::setzeZoomZurueck);

    // --- der angeklickte Clip
    m_clipZeile = new QLabel(this);
    m_clipZeile->setObjectName(QStringLiteral("composerClip"));
    m_clipZeile->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto* fuss = new QHBoxLayout();
    fuss->addWidget(m_clipZeile, 1);
    // Das Ende der Komposition: folgt dem letzten Clip — oder steht fest
    m_endeFest = new QCheckBox(tr("Fixed end"), this);
    m_endeFest->setObjectName(QStringLiteral("composerFixedEnd"));
    m_endeFest->setToolTip(
        tr("Off: the composition ends where its last clip ends — shorten or move the last clip "
           "and the end follows.\n\n"
           "On: the composition ends at the time on the right, even when no clip is left; the "
           "empty stretch is hatched and the end carries its own marker. Loop and the end "
           "setting act there. Can be changed while Edit is on."));
    fuss->addWidget(m_endeFest);
    m_endeZeit = new QDoubleSpinBox(this);
    m_endeZeit->setObjectName(QStringLiteral("composerEndTime"));
    m_endeZeit->setDecimals(1);
    m_endeZeit->setSingleStep(1.0);
    m_endeZeit->setRange(0.1, 86400.0);
    m_endeZeit->setSuffix(tr(" s"));
    m_endeZeit->setKeyboardTracking(false);
    fuss->addWidget(m_endeZeit);
    fuss->addSpacing(12);
    connect(m_endeFest, &QCheckBox::clicked, this, [this](bool fest) {
        if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
        Komposition neu = m_sitzung->komposition();
        neu.laengeMs = fest ? neu.laenge() : 0;
        bearbeite(neu, fest ? tr("Fix the end") : tr("Let the end follow the last clip"));
        zeigeEnde();
    });
    connect(m_endeZeit, &QDoubleSpinBox::valueChanged, this, [this](double sekunden) {
        if (m_sitzung == nullptr || !m_sitzung->geladen() || m_sitzung->komposition().laengeMs <= 0)
            return;
        Komposition neu = m_sitzung->komposition();
        neu.laengeMs = static_cast<std::int64_t>(sekunden * 1000.0 + 0.5);
        bearbeite(neu, tr("Change the end"));
    });
    m_mischTitel = new QLabel(tr("Blend:"), this);
    fuss->addWidget(m_mischTitel);
    m_mischart = new QComboBox(this);
    m_mischart->setObjectName(QStringLiteral("composerMischart"));
    for (int i = 0; i < kMischarten; ++i)
        m_mischart->addItem(QString::fromLatin1(mischartName(static_cast<Mischart>(i))));
    m_mischart->setToolTip(tr("How the selected clip is mixed with what lies below it. "
                              "Can be changed while Edit is on."));
    fuss->addWidget(m_mischart);
    // am Ton-Clip steht an derselben Stelle das Gain (S126, Entscheid Patrik G2 a)
    m_gainTitel = new QLabel(tr("Gain:"), this);
    fuss->addWidget(m_gainTitel);
    m_gain = new QDoubleSpinBox(this);
    m_gain->setObjectName(QStringLiteral("composerGain"));
    m_gain->setDecimals(1);
    m_gain->setSingleStep(0.5);
    m_gain->setRange(kGainMinDb, kGainMaxDb);
    m_gain->setSuffix(tr(" dB"));
    m_gain->setKeyboardTracking(false);
    m_gain->setToolTip(tr("How much louder or quieter the selected sound clip plays than its file, "
                          "-24 to +24 dB. The level curve (fades, shapes) stays 0 to 1 on top of it. "
                          "Can be changed while Edit is on; right-click the clip to reset it."));
    fuss->addWidget(m_gain);
    // daneben der Dialog Match loudness (S127, Entscheid Patrik G5-4 a)
    m_abgleichKnopf = new QPushButton(tr("Match loudness..."), this);
    m_abgleichKnopf->setObjectName(QStringLiteral("composerMatchLoudness"));
    m_abgleichKnopf->setToolTip(
        tr("Measure the loudness of every sound clip and suggest a gain for each, so that they play "
           "equally loud. You hear the dialog while the Composer plays; OK keeps it as one step. "
           "A selected sound clip is marked in the list. Can be opened while Edit is on and the "
           "composition has a sound clip."));
    fuss->addWidget(m_abgleichKnopf);
    connect(m_abgleichKnopf, &QPushButton::clicked, this, [this] {
        (void)oeffneAbgleich(m_zeitachse->gewaehlterKanal(), m_zeitachse->gewaehlterClip(), false);
    });
    m_gainTitel->hide();
    m_gain->hide();
    m_abgleichKnopf->hide();
    root->addLayout(fuss);

    // --- die Schleife der Komposition (S99, Entscheide Patrik): die Werte
    //     gehoeren zur Komposition und stehen in ihrer Datei
    auto* schleifenZeile = new QHBoxLayout();
    schleifenZeile->addStretch(1);
    schleifenZeile->addWidget(new QLabel(tr("Loop overlap:"), this));
    m_ueberlappung = new QDoubleSpinBox(this);
    m_ueberlappung->setObjectName(QStringLiteral("composerLoopOverlap"));
    m_ueberlappung->setDecimals(1);
    m_ueberlappung->setSingleStep(1.0);
    m_ueberlappung->setRange(0.0, 3600.0);
    m_ueberlappung->setSuffix(tr(" s"));
    m_ueberlappung->setKeyboardTracking(false);
    m_ueberlappung->setToolTip(
        tr("With Loop on, the composition starts again this long before its end: its last "
           "seconds and its first seconds play together, picture and sound. Nothing is faded "
           "for you — every clip keeps its own level curve, so shape the fades of the clips "
           "at both ends.\n\n"
           "The loop gets shorter by this time. On the very first run nothing plays into the "
           "beginning yet. The two windows are tinted in the timeline while Loop is on; at most "
           "half the length takes effect. 0 = the loop jumps from the end to the start.\n\n"
           "Saved with the composition. Can be changed while Edit is on."));
    schleifenZeile->addWidget(m_ueberlappung);
    schleifenZeile->addSpacing(12);
    m_songsDoppelt = new QCheckBox(tr("Songs overlap themselves"), this);
    m_songsDoppelt->setObjectName(QStringLiteral("composerSongsTwice"));
    m_songsDoppelt->setToolTip(
        tr("For a song that lies in both windows of the loop overlap (one song over the whole "
           "composition).\n\n"
           "On: it plays twice in the window — its end fades away while its beginning already "
           "plays.\n\n"
           "Off: it plays once — it finishes its end and then jumps back inside the song.\n\n"
           "Saved with the composition. Can be changed while Edit is on."));
    schleifenZeile->addWidget(m_songsDoppelt);
    m_bilderDoppelt = new QCheckBox(tr("Visuals overlap themselves"), this);
    m_bilderDoppelt->setObjectName(QStringLiteral("composerVisualsTwice"));
    m_bilderDoppelt->setToolTip(
        tr("For a visual clip that lies in both windows of the loop overlap (one preset over "
           "the whole composition).\n\n"
           "Off: it runs once, without restarting — in the window the higher of its two levels "
           "counts.\n\n"
           "On: in the window it runs twice — the old one plays the end, a freshly started one "
           "is mixed over it for the beginning, each with its own level; after the window the "
           "fresh one carries on. This loads the preset a second time shortly before the loop "
           "turns over and needs twice the graphics memory for the length of the window. The "
           "clip Effect Chain is synced to always runs once.\n\n"
           "Saved with the composition. Can be changed while Edit is on."));
    schleifenZeile->addSpacing(8);
    schleifenZeile->addWidget(m_bilderDoppelt);
    connect(m_bilderDoppelt, &QCheckBox::clicked, this, [this](bool an) {
        if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
        Komposition neu = m_sitzung->komposition();
        neu.bilderDoppelt = an;
        bearbeite(neu, an ? tr("Let visuals overlap themselves") : tr("Run visuals once in the loop"));
    });
    // der Limiter am Mixer (Entscheid Patrik G4b a, S127): gehoert zur
    // Komposition, Vorgabe an
    m_limiter = new QCheckBox(tr("Limiter"), this);
    m_limiter->setObjectName(QStringLiteral("composerLimiter"));
    m_limiter->setToolTip(
        tr("Keeps the sum of all sound clips below 0 dB while the Composer plays the sound.\n\n"
           "On: peaks above 0 dB — for example from a clip with Gain above 0 dB — are turned down "
           "smoothly instead of clipping. Everything below passes unchanged. The visuals see the "
           "sound behind the limiter.\n\n"
           "Off: the sum goes out as it is; peaks above 0 dB clip at the output.\n\n"
           "Saved with the composition. Can be changed while Edit is on."));
    schleifenZeile->addSpacing(8);
    schleifenZeile->addWidget(m_limiter);
    connect(m_limiter, &QCheckBox::clicked, this, [this](bool an) {
        if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
        Komposition neu = m_sitzung->komposition();
        neu.limiter = an;
        bearbeite(neu, an ? tr("Turn on limiter") : tr("Turn off limiter"));
    });
    // Ein- und Ausblenden der ganzen Komposition (Entscheid Patrik S99: getrennt
    // von der Ueberlappung, gilt immer)
    // — in einer eigenen Zeile: zusammen mit den Haken waere das Panel zu breit
    auto* blendenZeile = new QHBoxLayout();
    blendenZeile->addStretch(1);
    const auto blendenFeld = [this, blendenZeile](const QString& titel, const char* name,
                                                  const QString& hinweis) {
        QHBoxLayout* const schleifenZeile = blendenZeile;
        schleifenZeile->addSpacing(12);
        schleifenZeile->addWidget(new QLabel(titel, this));
        auto* feld = new QDoubleSpinBox(this);
        feld->setObjectName(QString::fromLatin1(name));
        feld->setDecimals(1);
        feld->setSingleStep(1.0);
        feld->setRange(0.0, 3600.0);
        feld->setSuffix(tr(" s"));
        feld->setKeyboardTracking(false);
        feld->setToolTip(hinweis);
        schleifenZeile->addWidget(feld);
        return feld;
    };
    m_einblende = blendenFeld(
        tr("Fade in:"), "composerFadeIn",
        tr("The whole composition fades in over this time — picture and sound, on top of the "
           "level curves of the clips. It applies always: on the first start, in every round of "
           "a loop, and without Loop. With a loop overlap, fade in and fade out together make a "
           "crossfade. The karaoke layer is not faded. 0 = no fade.\n\n"
           "Shown in the MASTER row below the ruler — there the fade has a handle, a shape "
           "(right-click) and the Curve Editor, like the fade of a clip. Saved with the "
           "composition. Can be changed while Edit is on."));
    m_ausblende = blendenFeld(
        tr("Fade out:"), "composerFadeOut",
        tr("The whole composition fades out over this time before its end — picture and sound, "
           "on top of the level curves of the clips. It applies always, also without Loop. "
           "The karaoke layer is not faded. 0 = no fade.\n\n"
           "Shown in the MASTER row below the ruler — there the fade has a handle, a shape "
           "(right-click) and the Curve Editor, like the fade of a clip. Saved with the "
           "composition. Can be changed while Edit is on."));
    for (QDoubleSpinBox* feld : {m_einblende, m_ausblende})
    {
        connect(feld, &QDoubleSpinBox::valueChanged, this, [this, feld](double sekunden) {
            if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
            Komposition neu = m_sitzung->komposition();
            const bool ein = feld == m_einblende;
            std::int64_t& wert = ein ? neu.einblendeMs : neu.ausblendeMs;
            const std::int64_t vorher = wert;
            wert = static_cast<std::int64_t>(sekunden * 1000.0 + 0.5);
            if (wert == vorher) return;
            bearbeite(neu, ein ? tr("Change the fade in of the composition")
                               : tr("Change the fade out of the composition"));
        });
    }
    root->addLayout(schleifenZeile);
    root->addLayout(blendenZeile);
    connect(m_ueberlappung, &QDoubleSpinBox::valueChanged, this, [this](double sekunden) {
        if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
        Komposition neu = m_sitzung->komposition();
        neu.ueberlappungMs = static_cast<std::int64_t>(sekunden * 1000.0 + 0.5);
        if (neu.ueberlappungMs == m_sitzung->komposition().ueberlappungMs) return;
        bearbeite(neu, tr("Change the loop overlap"));
    });
    connect(m_songsDoppelt, &QCheckBox::clicked, this, [this](bool an) {
        if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
        Komposition neu = m_sitzung->komposition();
        neu.songsDoppelt = an;
        bearbeite(neu, an ? tr("Let songs overlap themselves") : tr("Play songs once in the loop"));
    });

    connect(m_edit, &QCheckBox::toggled, this, [this](bool an) {
        m_zeitachse->setBearbeiten(an);
        zeigeClip();
        zeigeEnde();
    });
    connect(m_sync, &QCheckBox::clicked, this, [this](bool an) {
        if (m_sitzung != nullptr) m_sitzung->setSync(an);
    });
    connect(m_zeitachse, &Zeitachse::bearbeitet, this,
            [this](const Komposition& neu, const QString& was) { bearbeite(neu, was); });
    connect(m_mischart, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        const VisualClip* clip = m_zeitachse->auswahl();
        if (clip == nullptr || m_sitzung == nullptr || !m_zeitachse->bearbeiten()) return;
        if (m_zeitachse->gesamtGewaehlt()) return;  // die Kurve der Komposition hat keine Mischart
        Komposition neu = m_sitzung->komposition();
        neu.kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())]
            .clips[static_cast<std::size_t>(m_zeitachse->gewaehlterClip())]
            .mischart = static_cast<Mischart>(index);
        bearbeite(neu, tr("Change blend mode"));
    });
    connect(m_gain, &QDoubleSpinBox::valueChanged, this, [this](double db) {
        const VisualClip* clip = m_zeitachse->auswahl();
        if (clip == nullptr || m_sitzung == nullptr || !m_zeitachse->bearbeiten()) return;
        if (m_zeitachse->gesamtGewaehlt() || std::abs(clip->gainDb - db) < 1e-9) return;
        Komposition neu = m_sitzung->komposition();
        Kanal& kanal = neu.kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())];
        if (kanal.art != KanalArt::Audio) return;
        kanal.clips[static_cast<std::size_t>(m_zeitachse->gewaehlterClip())].gainDb = db;
        bearbeite(neu, tr("Change gain"));
    });
    connect(m_speichern, &QPushButton::clicked, this, [this] {
        if (m_fenster != nullptr) (void)speichereMitDialog(*m_fenster, this, false);
    });
    connect(m_speichernUnter, &QPushButton::clicked, this, [this] {
        if (m_fenster != nullptr) (void)speichereMitDialog(*m_fenster, this, true);
    });
    connect(m_neu, &QPushButton::clicked, this, [this] {
        sucheFenster();
        if (m_fenster == nullptr || !beginneKomposition(*m_fenster, this)) return;
        m_edit->setChecked(true);  // eine leere Komposition will bearbeitet werden
    });
    connect(m_laden, &QPushButton::clicked, this, [this] {
        sucheFenster();
        if (m_fenster != nullptr) (void)oeffneKomposition(*m_fenster, this);
    });
    connect(m_zeitachse, &Zeitachse::hineingezogen, this, [this](const QStringList& dateien) {
        // Entscheid S88 (1a): eine Datei mit Host Switcher laeuft als Clip
        // unveraendert — der Composer sagt beim Hinzufuegen, dass dann zwei wechseln.
        QStringList mit;
        for (const QString& datei : dateien)
            if (hatHostSwitcher(datei)) mit << QFileInfo(datei).fileName();
        if (mit.isEmpty()) return;
        QMessageBox::information(
            this, tr("Add Clip"),
            tr("%1 contains a Host Switcher. It keeps switching inside its clip — so two "
               "things change the picture: the Composer and the switcher in the file.")
                .arg(mit.join(QStringLiteral(", "))));
    });

    connect(m_an, &QCheckBox::toggled, this, [this](bool an) {
        if (m_sitzung != nullptr) m_sitzung->setAn(an);
        m_gezeigt = -1;  // liess es sich nicht schalten, springt der Haken zurueck
        zeigeStand();
    });
    connect(m_spielen, &QPushButton::clicked, this, [this] {
        if (m_sitzung == nullptr) return;
        if (m_sitzung->laeuft() || m_sitzung->wartetAufToene())  // waehrend des Wartens: abbrechen (TV7 a)
            m_sitzung->pausiere();
        else
            m_sitzung->spiele();
    });
    connect(m_stopp, &QPushButton::clicked, this, [this] {
        if (m_sitzung != nullptr) m_sitzung->stoppe();
    });
    connect(m_schleife, &QPushButton::clicked, this, [this](bool an) {
        if (m_sitzung != nullptr) m_sitzung->setSchleife(an);
        zeigeStand();
    });
    connect(m_resume, &QPushButton::clicked, this, [this](bool an) {
        Sitzung::setResume(an);
        zeigeStand();
    });
    connect(m_zeitachse, &Zeitachse::sprung, this, [this](qint64 stelle) {
        if (m_sitzung != nullptr) m_sitzung->springe(stelle);
    });
    connect(m_zeitachse, &Zeitachse::auswahlGeaendert, this, [this] { zeigeClip(); });
    connect(m_zeitachse, &Zeitachse::kurveOeffnen, this, [this](Teil teil) { oeffneKurve(teil); });
    connect(m_zeitachse, &Zeitachse::lautheitAbgleichen, this,
            [this](int kanal, int clip) { (void)oeffneAbgleich(kanal, clip, true); });
    // S130: ueber den Bus, wie der Doppelklick im Audio Browser — das Panel Loop
    // fragt nach Ungespeichertem und kommt nach vorn
    connect(m_zeitachse, &Zeitachse::loopImPanelOeffnen, this, [this](const QString& datei) {
        if (auto* bus = services().tryResolve<IEventBus>()) bus->publish(LoadLoopEvent{datei.toStdString()});
    });
    connect(m_zeitachse, &Zeitachse::loopExportieren, this,
            [this](int kanal, int clip) { (void)oeffneLoopExport(kanal, clip); });

    // Audiokanal (C5): was die Zeitachse ueber Songs wissen muss, kommt von der Audio-Engine
    // — dazu die Loops (S109, Loop_Konzept §4e): eine `.lvloop` faellt in einen
    // Audio-Kanal wie ein Song; ihre Laenge ist ein Durchlauf, die kennt die Datei.
    m_zeitachse->setAudioEndungen(lumi::ui::audioEndungen(services()) << QStringLiteral("lvloop"));
    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    m_zeitachse->setSongLaenge([engine](const QString& datei) -> std::int64_t {
        if (istLoop(datei))
        {
            lumi::loop::Loop loop;
            return lumi::loop::ladeLoop(datei, loop) ? static_cast<std::int64_t>(std::llround(loop.durchlaufMs()))
                                                     : 0;
        }
        return engine != nullptr ? static_cast<std::int64_t>(engine->probeDurationMs(datei)) : 0;
    });
}

KurvenFenster* ComposerPanel::kurvenFenster() const
{
    return m_kurven;
}

bool ComposerPanel::oeffneLoopExport(int kanal, int clip)
{
    const Komposition& k = m_zeitachse->komposition();
    if (kanal < 0 || kanal >= static_cast<int>(k.kanaele.size())) return false;
    const Kanal& ziel = k.kanaele[static_cast<std::size_t>(kanal)];
    if (ziel.art != KanalArt::Audio || clip < 0 || clip >= static_cast<int>(ziel.clips.size())) return false;
    const Clip& c = ziel.clips[static_cast<std::size_t>(clip)];
    if (!istLoop(c.datei)) return false;
    // die Fassung, die das Panel Loop offen hat (auch ungespeichert, L7 a) — sonst die Datei
    lumi::loop::Loop loop;
    if (!lumi::loop::offenerLoop(c.datei, loop) && !lumi::loop::ladeLoop(c.datei, loop)) return false;
    loop.richte();

    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    lumi::loop::ExportWunsch vorlage;
    vorlage.art = lumi::loop::ExportArt::Ausschnitt;
    vorlage.loop = std::move(loop);
    vorlage.versatzMs = static_cast<double>(c.einsatzMs);
    vorlage.laengeMs = static_cast<double>(c.dauerMs());
    vorlage.rate = engine != nullptr && engine->getSampleRate() > 0 ? engine->getSampleRate() : 44100;
    const QString name = QFileInfo(c.datei).completeBaseName();
    const QString text = tr("%1 in %2, %3 – %4").arg(name, ziel.name, zeitText(c.startMs), zeitText(c.endeMs));
    lumi::loop::Dekodierer dekodierer = [engine](const QString& datei, int rate, double maxSekunden) {
        return engine != nullptr ? engine->decodeStereo(datei, rate, maxSekunden) : std::vector<float>{};
    };
    lumi::loop::ExportFenster fenster(std::move(vorlage), name, text, std::move(dekodierer), this);
    (void)lumi::ui::fuehreDialogAus(fenster);
    return true;
}

void ComposerPanel::setBearbeiten(bool an)
{
    m_edit->setChecked(an);
}

bool ComposerPanel::bearbeiten() const
{
    return m_edit->isChecked();
}

void ComposerPanel::oeffneKurve(Teil teil)
{
    m_kurvenTeil = teil;
    if (m_kurven == nullptr)
    {
        // ein eigenes Fenster des Panels (Entscheid Patrik S96) — keine Naht
        m_kurven = new KurvenFenster(this);
        connect(m_kurven, &KurvenFenster::clipGeaendert, this, [this](const VisualClip& clip, const QString& was) {
            if (m_sitzung == nullptr || !m_sitzung->geladen() || !m_zeitachse->bearbeiten()) return;
            const int k = m_zeitachse->gewaehlterKanal();
            const int c = m_zeitachse->gewaehlterClip();
            Komposition neu = m_sitzung->komposition();
            if (m_zeitachse->gesamtGewaehlt())
            {
                // eine Kurve der ganzen Komposition (S100; S101: MASTER, VISUAL
                // oder AUDIO): die Lage der Stuecke wird zur festen Zeit
                neu.setzeKurveAusClip(clip, m_zeitachse->gewaehlteMasterArt());
                bearbeite(neu, was);
                return;
            }
            if (k < 0 || k >= static_cast<int>(neu.kanaele.size())) return;
            auto& clips = neu.kanaele[static_cast<std::size_t>(k)].clips;
            if (c < 0 || c >= static_cast<int>(clips.size())) return;
            // nur die Pegelkurve: Datei, Zeiten und Mischart gehoeren der Zeitachse
            VisualClip& ziel = clips[static_cast<std::size_t>(c)];
            ziel.grundpegel = clip.grundpegel;
            ziel.ein = clip.ein;
            ziel.aus = clip.aus;
            ziel.stuecke = clip.stuecke;
            bearbeite(neu, was);
        });
    }
    m_kurven->show();
    m_kurven->raise();
    zeigeKurve();
}

void ComposerPanel::zeigeKurve()
{
    if (m_kurven == nullptr) return;
    const VisualClip* clip = m_zeitachse->auswahl();
    m_kurven->zeige(clip, m_kurvenTeil,
                    clip == nullptr                 ? QString()
                    : !m_zeitachse->gesamtGewaehlt() ? QFileInfo(clip->datei).completeBaseName()
                    : m_zeitachse->gewaehlteMasterArt() == MasterArt::Bild ? tr("Visual master")
                    : m_zeitachse->gewaehlteMasterArt() == MasterArt::Ton  ? tr("Audio master")
                    : m_zeitachse->gewaehlteMasterArt() == MasterArt::Karaoke ? tr("Karaoke master")
                                                                              : tr("Composition"),
                    m_zeitachse->bearbeiten());
    m_zeitachse->setTeil(m_kurven->isVisible() && m_kurven->hatKurve() ? m_kurvenTeil : Teil{});
}

void ComposerPanel::setFenster(VisualizerWidget* fenster)
{
    if (fenster == m_fenster) return;
    if (m_sitzung != nullptr) disconnect(m_sitzung, nullptr, this, nullptr);
    m_fenster = fenster;
    m_sitzung = fenster != nullptr ? &Sitzung::fuer(*fenster) : nullptr;
    if (m_sitzung != nullptr)
    {
        connect(m_sitzung, &Sitzung::kompositionGeaendert, this, [this] { zeigeKomposition(); });
        connect(m_sitzung, &Sitzung::kompositionBearbeitet, this, [this] {
            m_zeitachse->zeigeBearbeitet(m_sitzung->komposition());
            // (der Curve Editor folgt ueber `auswahlGeaendert` → `zeigeClip`, auch nach Undo)
            zeigeEnde();
            zeigeAbgleichKnopf();  // der letzte Ton-Clip kann gegangen sein
        });
        connect(m_sitzung, &Sitzung::geaendert, this, [this] {
            zeigeStand();
            zeigeDatei();
        });
        connect(m_sitzung, &Sitzung::takt, this, [this] { zeigeStand(); });
    }
    zeigeKomposition();
}

void ComposerPanel::sucheFenster()
{
    if (m_fenster != nullptr) return;
    VisualizerWidget* gefunden = nullptr;
    if (QWidget* fenster = window()) gefunden = fenster->findChild<VisualizerWidget*>();
    // abgedockt ist das Panel sein eigenes Fenster — dann ueber alle suchen
    if (gefunden == nullptr)
    {
        const QWidgetList alle = QApplication::topLevelWidgets();
        for (QWidget* w : alle)
        {
            gefunden = qobject_cast<VisualizerWidget*>(w);
            if (gefunden == nullptr) gefunden = w->findChild<VisualizerWidget*>();
            if (gefunden != nullptr) break;
        }
    }
    if (gefunden != nullptr) setFenster(gefunden);
}

void ComposerPanel::showEvent(QShowEvent* event)
{
    PanelBase::showEvent(event);
    sucheFenster();
    zeigeStand();
}

void ComposerPanel::zeigeKomposition()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    m_zeitachse->setKomposition(geladen ? m_sitzung->komposition() : Komposition{}, geladen);
    zeigeDatei();
    zeigeStand();
    zeigeClip();
    zeigeEnde();
}

void ComposerPanel::zeigeEnde()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    const bool fest = geladen && m_sitzung->komposition().laengeMs > 0;
    const bool bearbeiten = geladen && m_zeitachse->bearbeiten();
    m_endeFest->setChecked(fest);
    m_endeFest->setEnabled(bearbeiten);
    m_endeZeit->setEnabled(bearbeiten && fest);
    const QSignalBlocker ruhe(m_endeZeit);
    m_endeZeit->setValue(geladen ? static_cast<double>(m_sitzung->laengeMs()) / 1000.0 : 0.1);
    // die Schleife der Komposition (S99): der eingestellte Wert, auch wenn nur ein Teil wirkt
    m_ueberlappung->setEnabled(bearbeiten);
    m_songsDoppelt->setEnabled(bearbeiten);
    const QSignalBlocker ruheSchleife(m_ueberlappung);
    m_ueberlappung->setValue(geladen ? static_cast<double>(m_sitzung->komposition().ueberlappungMs) / 1000.0
                                     : 0.0);
    m_songsDoppelt->setChecked(!geladen || m_sitzung->komposition().songsDoppelt);
    m_bilderDoppelt->setEnabled(bearbeiten);
    m_bilderDoppelt->setChecked(geladen && m_sitzung->komposition().bilderDoppelt);
    m_limiter->setEnabled(bearbeiten);
    m_limiter->setChecked(!geladen || m_sitzung->komposition().limiter);
    m_einblende->setEnabled(bearbeiten);
    m_ausblende->setEnabled(bearbeiten);
    const QSignalBlocker ruheEin(m_einblende);
    const QSignalBlocker ruheAus(m_ausblende);
    m_einblende->setValue(geladen ? static_cast<double>(m_sitzung->komposition().einblendeMs) / 1000.0 : 0.0);
    m_ausblende->setValue(geladen ? static_cast<double>(m_sitzung->komposition().ausblendeMs) / 1000.0 : 0.0);
}

void ComposerPanel::zeigeDatei()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    const bool offen = geladen && m_sitzung->ungespeichert();
    const bool ohneDatei = geladen && m_sitzung->datei().isEmpty();  // neu, noch nie gespeichert
    // der Stern wie in der Preset-Zeile: es gibt Ungespeichertes
    m_datei->setText(geladen ? (ohneDatei ? tr("untitled") : QFileInfo(m_sitzung->datei()).fileName()) +
                                   (offen ? QStringLiteral(" *") : QString())
                             : QString());
    m_datei->setToolTip(geladen && !ohneDatei ? QDir::toNativeSeparators(m_sitzung->datei()) : QString());
    m_dateiIcon->setVisible(geladen && !m_dateiIcon->pixmap().isNull());
    m_speichern->setEnabled(offen || ohneDatei);
    m_speichernUnter->setEnabled(geladen);
    m_edit->setEnabled(geladen && m_abgleich == nullptr);  // grau, solange Match loudness offen ist (G5-2 a)
    if (!geladen && m_edit->isChecked()) m_edit->setChecked(false);
    m_sync->setEnabled(m_sitzung != nullptr);
    m_sync->setChecked(m_sitzung != nullptr && m_sitzung->sync());
}

bool ComposerPanel::hatHostSwitcher(const QString& pfad)
{
    const QString endung = QFileInfo(pfad).suffix().toLower();
    if (endung != QLatin1String("lvfx") && endung != QLatin1String("lvfx2")) return false;
    lumi::multieffect::ChainNode wurzel;
    QStringList bericht;
    if (!lumi::multieffect::loadChainFromFile(pfad, wurzel, &bericht)) return false;
    const auto sucht = [](const auto& selbst, const lumi::multieffect::ChainNode& knoten) -> bool {
        if (std::holds_alternative<lumi::multieffect::HostSwitcherParams>(knoten.params)) return true;
        for (const lumi::multieffect::ChainNode& kind : knoten.children)
            if (selbst(selbst, kind)) return true;
        return false;
    };
    return sucht(sucht, wurzel);
}

LautheitsDialog* ComposerPanel::oeffneAbgleich(int kanal, int clip, bool alsZiel)
{
    if (m_abgleich != nullptr)
    {
        if (alsZiel)
            m_abgleich->setzeZielClip(kanal, clip);
        else
            m_abgleich->markiere(kanal, clip);
        m_abgleich->raise();
        m_abgleich->activateWindow();
        return m_abgleich;
    }
    if (m_sitzung == nullptr || !m_sitzung->geladen() || !m_zeitachse->bearbeiten()) return nullptr;
    auto* dialog = new LautheitsDialog(
        *m_sitzung, services().tryResolve<IAudioEngine>(),
        [this](const Komposition& neu, const QString& was) { bearbeite(neu, was); }, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    m_abgleich = dialog;
    // solange er offen ist, ruht das Bearbeiten: Edit grau, der Haken bleibt (G5-2 a)
    m_zeitachse->setBearbeiten(false);
    zeigeDatei();
    zeigeClip();
    zeigeEnde();
    connect(dialog, &QDialog::finished, this, [this] {
        m_abgleich = nullptr;
        m_zeitachse->setBearbeiten(m_edit->isChecked() && m_sitzung != nullptr && m_sitzung->geladen());
        zeigeDatei();
        zeigeClip();
        zeigeEnde();
    });
    if (alsZiel)
        dialog->setzeZielClip(kanal, clip);
    else
        dialog->markiere(kanal, clip);
    dialog->show();
    return dialog;
}

LautheitsDialog* ComposerPanel::abgleichDialog() const
{
    return m_abgleich;
}

void ComposerPanel::zeigeAbgleichKnopf()
{
    // S127, Wunsch Patrik nach G5-4 a: immer da, sobald eine Komposition
    // geladen ist — der Dialog gilt allen Ton-Clips, nicht dem angeklickten
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    const bool hatTon = geladen && std::any_of(m_sitzung->komposition().kanaele.begin(),
                                               m_sitzung->komposition().kanaele.end(), [](const Kanal& k) {
                                                   return k.art == KanalArt::Audio && !k.clips.empty();
                                               });
    m_abgleichKnopf->setVisible(geladen);
    m_abgleichKnopf->setEnabled(hatTon && m_zeitachse->bearbeiten());
}

void ComposerPanel::bearbeite(const Komposition& neu, const QString& was)
{
    if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
    if (auto* bus = services().tryResolve<ICommandBus>())
        (void)bus->execute(std::make_unique<KompositionsBefehl>(*m_sitzung, neu, was));
    else
        m_sitzung->setzeKomposition(neu);
}

void ComposerPanel::zeigeStand()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    const bool an = geladen && m_sitzung->an();
    // wartet Play auf die Toene der Loop-Clips, zeigt der Knopf schon Pause (TV7 a A)
    const bool laeuft = an && (m_sitzung->laeuft() || m_sitzung->wartetAufToene());
    const bool schleife = m_sitzung != nullptr ? m_sitzung->schleife() : false;

    // Schalter und Knoepfe nur anfassen, wenn sich etwas geaendert hat — der
    // Takt kommt 30-mal je Sekunde, und ein Stylesheet zu setzen ist teuer.
    const int zustand = (geladen ? 1 : 0) | (an ? 2 : 0) | (laeuft ? 4 : 0) | (schleife ? 8 : 0);
    if (zustand != m_gezeigt)
    {
        m_gezeigt = zustand;
        {
            const QSignalBlocker ruhe(m_an);
            m_an->setChecked(an);
        }
        m_an->setEnabled(geladen);
        m_spielen->setEnabled(geladen);
        m_stopp->setEnabled(geladen);
        m_spielen->setIcon(
            style()->standardIcon(laeuft ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
        m_spielen->setToolTip(laeuft ? tr("Pause") : tr("Play"));
        m_schleife->setChecked(schleife);
        m_schleife->setStyleSheet(schleife ? kKnopfAn : QString());
        m_schleife->setToolTip(schleife ? tr("Loop the whole composition (ON)")
                                        : tr("Loop the whole composition"));
        m_zeitachse->setAn(an);
    }
    const bool resume = Sitzung::resumeAn();
    if ((resume ? 1 : 0) != m_resumeGezeigt)
    {
        m_resumeGezeigt = resume ? 1 : 0;
        m_resume->setChecked(resume);
        m_resume->setStyleSheet(resume ? kKnopfAn : QString());
        m_resume->setToolTip(
            resume ? tr("Resume after restart (ON): closing LumiViz brings the composition back "
                        "the way it was — Composer on or off, same position, playing or paused")
                   : tr("Resume after restart (off: the composition comes back loaded, with "
                        "the Composer off)"));
    }

    // Mitlaufen (S102): nur, solange die Komposition laeuft — ein Sprung im
    // Stand laesst den Ausschnitt, wo er ist. Die Art liest die Sitzung aus den
    // Einstellungen; gefragt wird nur, wenn es etwas zu folgen gibt.
    m_zeitachse->setMitlaufen(!laeuft || !m_mitlaufen->isChecked() || m_zeitachse->zeitGanz()
                                  ? Zeitachse::Mitlaufen::Aus
                                  : Sitzung::mitlaufenSeitenweise() ? Zeitachse::Mitlaufen::Seitenweise
                                                                    : Zeitachse::Mitlaufen::Stetig);
    const std::int64_t stelle = geladen ? m_sitzung->stelleMs() : 0;
    // TV7 a (Stufe 29e): was gerechnet wird, steht im Feld Zeit — A an seiner Stelle, C und D dahinter
    QString zeit = geladen ? QStringLiteral("%1 / %2").arg(zeitText(stelle), zeitText(m_sitzung->laengeMs())) : QString();
    QString farbe;
    if (geladen && m_sitzung->wartetAufToene())
    {
        zeit = tr("Preparing sounds… %1 / %2").arg(m_sitzung->toeneVorbereitet()).arg(m_sitzung->toeneVorzubereiten());
        farbe = QStringLiteral("color: palette(highlight);");
    }
    else if (const Tonfuehrung::Vorbereitung v = m_sitzung != nullptr ? m_sitzung->tonVorbereitung() : Tonfuehrung::Vorbereitung{};
             geladen && v.gesamt > 0)
    {
        // D in Warnfarbe: Noten schweigen; C in Akzentfarbe: es klingt noch der bisherige Stand
        zeit += v.luecken ? tr(" · some notes silent, preparing %1 / %2").arg(v.erledigt).arg(v.gesamt)
                          : tr(" · updating sounds %1 / %2").arg(v.erledigt).arg(v.gesamt);
        // die Warnfarbe wie im Dialog Match loudness (LautheitsDialog)
        farbe = v.luecken ? QStringLiteral("color: #d8b400;") : QStringLiteral("color: palette(highlight);");
    }
    if (m_zeit->text() != zeit) m_zeit->setText(zeit);
    if (m_zeit->styleSheet() != farbe) m_zeit->setStyleSheet(farbe);
    m_zeitachse->setStelle(stelle);
    m_zeitachse->setSchleife(schleife, geladen ? m_sitzung->zweiteStelleMs() : -1);
}

void ComposerPanel::zeigeClip()
{
    const VisualClip* clip = m_zeitachse->auswahl();
    zeigeKurve();   // der Curve Editor folgt dem angeklickten Clip
    ladeHuellen();  // C5: die Wellenform neuer Songs holen
    // die Kurve der ganzen Komposition (S100) ist angeklickt: kein Clip eines Kanals
    const bool gesamt = clip != nullptr && m_zeitachse->gesamtGewaehlt();
    // die Mischart gibt es nur am Bild
    const bool bild = clip != nullptr && !gesamt &&
                      m_zeitachse->komposition().kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())].art ==
                          KanalArt::Visual;
    m_mischart->setEnabled(bild && m_zeitachse->bearbeiten());
    // am Ton-Clip statt der Mischart das Gain (S126, G2 a)
    const bool ton = clip != nullptr && !gesamt && !bild;
    m_mischTitel->setVisible(!ton);
    m_mischart->setVisible(!ton);
    m_gainTitel->setVisible(ton);
    m_gain->setVisible(ton);
    m_gain->setEnabled(ton && m_zeitachse->bearbeiten());
    zeigeAbgleichKnopf();
    if (ton)
    {
        const QSignalBlocker still(m_gain);
        m_gain->setValue(clip->gainDb);
    }
    // Sync on Effect Chain (C3c) folgt dem angeklickten Clip
    if (m_sitzung != nullptr)
        m_sitzung->waehleClip(clip != nullptr && !gesamt ? m_zeitachse->gewaehlterKanal() : -1,
                              clip != nullptr && !gesamt ? m_zeitachse->gewaehlterClip() : -1);
    // S101: VISUAL und AUDIO — je eine eigene Kurve nur fuer das Bild bzw. den Ton
    if (gesamt && m_sitzung != nullptr && m_zeitachse->gewaehlteMasterArt() == MasterArt::Karaoke)
    {
        // KARAOKE (Entscheid Patrik KS1: a): wie stark die Karaoke-Ebene aufliegt
        m_clipZeile->setText(tr("KARAOKE · level curve of the karaoke layer · 0:00.0 – %1")
                                 .arg(zeitText(clip->endeMs)));
        m_clipZeile->setToolTip(
            tr("How strongly the karaoke layer lies over the picture — the lyrics fade with this curve. "
               "It stands for itself: MASTER and VISUAL do not fade the lyrics. The layer must be on "
               "(the Karaoke check box); with the layer off this row shows nothing. Right-click the "
               "KARAOKE row for fades, shapes and the Curve Editor."));
        return;
    }
    if (gesamt && m_sitzung != nullptr && m_zeitachse->gewaehlteMasterArt() != MasterArt::Gesamt)
    {
        const bool bildZeile = m_zeitachse->gewaehlteMasterArt() == MasterArt::Bild;
        m_clipZeile->setText((bildZeile ? tr("VISUAL · level curve of the picture only · 0:00.0 – %1")
                                        : tr("AUDIO · level curve of the sound only · 0:00.0 – %1"))
                                 .arg(zeitText(clip->endeMs)));
        m_clipZeile->setToolTip(
            bildZeile ? tr("A level curve for the picture only, on top of MASTER and the level curves of the "
                           "clips. The sound is not affected. Right-click the VISUAL row for fades, shapes "
                           "and the Curve Editor.")
                      : tr("A level curve for the sound only, on top of MASTER and the level curves of the "
                           "clips. The picture is not affected — but the visuals see the quieter sound. "
                           "Right-click the AUDIO row for fades, shapes and the Curve Editor."));
        return;
    }
    if (gesamt && m_sitzung != nullptr)
    {
        m_clipZeile->setText(tr("MASTER · level curve of the whole composition · 0:00.0 – %1")
                                 .arg(zeitText(clip->endeMs)));
        m_clipZeile->setToolTip(tr("The level curve of the whole composition — picture and sound, on top of the "
                                   "level curves of the clips. Right-click the MASTER row for fades, shapes "
                                   "and the Curve Editor."));
        return;
    }
    if (clip == nullptr || m_sitzung == nullptr)
    {
        QString zeile = m_sitzung != nullptr && m_sitzung->geladen() ? tr("No clip selected") : QString();
        // die Einfuegemarke (S98): wohin „Einfuegen" den Anfang eines Clips legt
        if (const int k = m_zeitachse->markeKanal(); k >= 0 && !zeile.isEmpty())
            zeile = tr("Insert mark: %1 at %2")
                        .arg(m_zeitachse->komposition().kanaele[static_cast<std::size_t>(k)].name,
                             zeitText(m_zeitachse->markeMs()));
        m_clipZeile->setText(zeile);
        m_clipZeile->setToolTip(QString());
        return;
    }
    m_mischart->setCurrentIndex(static_cast<int>(clip->mischart));
    // Der Kanal aus der Fassung der ZEITACHSE: waehrend eines Schritts ist sie
    // der Sitzung einen Augenblick voraus (ein eben entstandener Kanal).
    const Kanal& kanal =
        m_zeitachse->komposition().kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())];
    // am Bild die Mischart, am Ton die Stelle im Song, an der der Clip einsetzt
    m_clipZeile->setText(QStringLiteral("%1 · %2 · %3 – %4 · %5")
                             .arg(kanal.name, QFileInfo(clip->datei).fileName(),
                                  zeitText(clip->startMs), zeitText(clip->endeMs),
                                  bild ? QString::fromLatin1(mischartName(clip->mischart))
                                       : tr("from %1 in the song").arg(zeitText(clip->einsatzMs))));
    m_clipZeile->setToolTip(QDir::toNativeSeparators(clip->datei));
}

}  // namespace lumi::composer
