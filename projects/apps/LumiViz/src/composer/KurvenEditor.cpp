/**
 * @file KurvenEditor.cpp
 * @brief Composer: das Fenster Curve Editor (Stufe C4 Teil 2)
 */

#include "composer/KurvenEditor.hpp"

#include "composer/FormPresets.hpp"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace lumi::composer
{

namespace
{
const QColor kAnfasserFarbe(255, 220, 120);

/// Die Form des Teils im Clip; nullptr = der Clip hat das Teil nicht.
Pegelform* formVon(VisualClip& clip, Teil teil)
{
    if (!hatTeil(clip, teil)) return nullptr;
    if (teil.art == TeilArt::Ein) return &clip.ein.form;
    if (teil.art == TeilArt::Aus) return &clip.aus.form;
    return &clip.stuecke[static_cast<std::size_t>(teil.stueck)].form;
}

double sekunden(std::int64_t ms)
{
    return static_cast<double>(ms) / 1000.0;
}

std::int64_t millis(double s)
{
    return static_cast<std::int64_t>(std::llround(s * 1000.0));
}
}  // namespace

// =============================================================================
// Formflaeche
// =============================================================================

Formflaeche::Formflaeche(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("composerFormflaeche"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);  // Entf nimmt den gewaehlten Punkt weg
    m_form = einfacheBlende();
}

void Formflaeche::setForm(const Pegelform& form)
{
    m_form = form;
    m_zug = Zug::Keiner;
    if (m_gewaehlt >= static_cast<int>(m_form.punkte.size())) m_gewaehlt = -1;
    update();
}

void Formflaeche::setBearbeitbar(bool an)
{
    if (an == m_bearbeitbar) return;
    m_bearbeitbar = an;
    m_zug = Zug::Keiner;
    update();
}

QRect Formflaeche::feld() const
{
    return rect().adjusted(14, 12, -14, -12);
}

QPoint Formflaeche::ortVon(double x, double y) const
{
    const QRect f = feld();
    return {f.left() + static_cast<int>(std::lround(x * (f.width() - 1))),
            f.bottom() - static_cast<int>(std::lround(y * (f.height() - 1)))};
}

double Formflaeche::xVon(int px) const
{
    const QRect f = feld();
    return std::clamp(static_cast<double>(px - f.left()) / std::max(1, f.width() - 1), 0.0, 1.0);
}

double Formflaeche::yVon(int py) const
{
    const QRect f = feld();
    return std::clamp(static_cast<double>(f.bottom() - py) / std::max(1, f.height() - 1), 0.0, 1.0);
}

QPoint Formflaeche::punktOrt(int punkt) const
{
    if (punkt < 0 || punkt >= static_cast<int>(m_form.punkte.size())) return {-1, -1};
    const FormPunkt& p = m_form.punkte[static_cast<std::size_t>(punkt)];
    return ortVon(p.x, p.y);
}

QPoint Formflaeche::anfasserOrt(int punkt, int nr) const
{
    if (punkt < 0 || punkt + 1 >= static_cast<int>(m_form.punkte.size())) return {-1, -1};
    const FormPunkt& a = m_form.punkte[static_cast<std::size_t>(punkt)];
    const FormPunkt& b = m_form.punkte[static_cast<std::size_t>(punkt) + 1];
    if (a.form != Kurvenform::Bezier) return {-1, -1};
    const double gx = a.bezier[static_cast<std::size_t>(nr * 2)];
    const double gy = a.bezier[static_cast<std::size_t>(nr * 2 + 1)];
    return ortVon(a.x + gx * (b.x - a.x), a.y + gy * (b.y - a.y));
}

int Formflaeche::punktBei(QPoint ort) const
{
    for (int i = 0; i < static_cast<int>(m_form.punkte.size()); ++i)
        if ((punktOrt(i) - ort).manhattanLength() <= 8) return i;
    return -1;
}

int Formflaeche::abschnittBei(double x) const
{
    const int letzter = static_cast<int>(m_form.punkte.size()) - 2;
    int i = 0;
    while (i < letzter && m_form.punkte[static_cast<std::size_t>(i) + 1].x <= x) ++i;
    return std::max(0, i);
}

bool Formflaeche::festX(int punkt) const
{
    return punkt <= 0 || punkt >= static_cast<int>(m_form.punkte.size()) - 1;
}

bool Formflaeche::festY(int punkt) const
{
    // eine Blende laeuft von (0,0) nach (1,1); die Enden eines Stuecks duerfen in der Hoehe wandern
    return m_form.art == FormArt::Blende && festX(punkt);
}

void Formflaeche::fuegePunktEin(double x, double y)
{
    if (!m_bearbeitbar || m_form.punkte.size() < 2) return;
    x = std::clamp(x, 0.0, 1.0);
    y = std::clamp(y, 0.0, 1.0);
    const int i = abschnittBei(x);
    const FormPunkt& vor = m_form.punkte[static_cast<std::size_t>(i)];
    m_form.punkte.insert(m_form.punkte.begin() + i + 1, FormPunkt{x, y, vor.form, vor.bezier});
    m_gewaehlt = i + 1;
    m_zug = Zug::Keiner;
    update();
    Q_EMIT geaendert(m_form, tr("Add curve point"));
}

void Formflaeche::loeschePunkt(int punkt)
{
    if (!m_bearbeitbar || festX(punkt)) return;
    m_form.punkte.erase(m_form.punkte.begin() + punkt);
    m_gewaehlt = -1;
    m_zug = Zug::Keiner;
    update();
    Q_EMIT geaendert(m_form, tr("Delete curve point"));
}

void Formflaeche::setzeAbschnittForm(int punkt, Kurvenform form)
{
    if (!m_bearbeitbar || punkt < 0 || punkt + 1 >= static_cast<int>(m_form.punkte.size())) return;
    FormPunkt& p = m_form.punkte[static_cast<std::size_t>(punkt)];
    if (p.form == form) return;
    p.form = form;
    update();
    Q_EMIT geaendert(m_form, tr("Change curve shape"));
}

void Formflaeche::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QPalette pal = palette();
    const QRect f = feld();
    p.fillRect(rect(), pal.color(QPalette::Base));

    // Raster: Viertel in beiden Richtungen
    QColor raster = pal.color(QPalette::Mid);
    raster.setAlpha(110);
    p.setPen(QPen(raster, 1.0));
    for (int i = 0; i <= 4; ++i)
    {
        const QPoint unten = ortVon(i / 4.0, 0.0);
        const QPoint links = ortVon(0.0, i / 4.0);
        p.drawLine(unten.x(), f.top(), unten.x(), f.bottom());
        p.drawLine(f.left(), links.y(), f.right(), links.y());
    }
    if (m_form.punkte.size() < 2) return;

    // die Kurve, Pixel fuer Pixel
    QPainterPath kurve;
    for (int px = f.left(); px <= f.right(); ++px)
    {
        const double x = xVon(px);
        const QPointF ort(px, f.bottom() - m_form.wert(x) * (f.height() - 1));
        if (px == f.left())
            kurve.moveTo(ort);
        else
            kurve.lineTo(ort);
    }
    QColor linie = pal.color(QPalette::Text);
    if (!m_bearbeitbar) linie.setAlpha(140);
    p.setPen(QPen(linie, 1.6));
    p.setBrush(Qt::NoBrush);
    p.drawPath(kurve);
    if (!m_bearbeitbar) return;

    // Bézier-Abschnitte: zwei Anfasser mit Hebeln zu ihren Punkten
    for (int i = 0; i + 1 < static_cast<int>(m_form.punkte.size()); ++i)
    {
        const QPoint a1 = anfasserOrt(i, 0);
        if (a1.x() < 0) continue;
        const QPoint a2 = anfasserOrt(i, 1);
        p.setPen(QPen(kAnfasserFarbe, 1.0));
        p.drawLine(punktOrt(i), a1);
        p.drawLine(punktOrt(i + 1), a2);
        p.setBrush(kAnfasserFarbe);
        p.drawEllipse(a1, 4, 4);
        p.drawEllipse(a2, 4, 4);
    }
    // die Punkte
    p.setRenderHint(QPainter::Antialiasing, false);
    for (int i = 0; i < static_cast<int>(m_form.punkte.size()); ++i)
    {
        const QPoint o = punktOrt(i);
        p.setPen(QPen(QColor(30, 30, 30), 1.0));
        p.setBrush(i == m_gewaehlt ? pal.color(QPalette::Highlight) : QColor(Qt::white));
        p.drawRect(QRect(o.x() - 3, o.y() - 3, 7, 7));
    }
}

void Formflaeche::mousePressEvent(QMouseEvent* event)
{
    if (!m_bearbeitbar || event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    setFocus(Qt::MouseFocusReason);
    m_zug = Zug::Keiner;
    m_bewegt = false;
    m_vorZug = m_form;
    // zuerst die Anfasser — sie liegen oft dicht an ihren Punkten
    for (int i = 0; i + 1 < static_cast<int>(m_form.punkte.size()); ++i)
        for (int nr = 0; nr < 2; ++nr)
        {
            const QPoint a = anfasserOrt(i, nr);
            if (a.x() < 0 || (a - ort).manhattanLength() > 8) continue;
            m_zug = Zug::Anfasser;
            m_zugPunkt = i;
            m_zugNr = nr;
            return;
        }
    m_gewaehlt = punktBei(ort);
    if (m_gewaehlt >= 0 && !(festX(m_gewaehlt) && festY(m_gewaehlt)))
    {
        m_zug = Zug::Punkt;
        m_zugPunkt = m_gewaehlt;
    }
    update();
}

void Formflaeche::mouseMoveEvent(QMouseEvent* event)
{
    if (m_zug == Zug::Keiner || !(event->buttons() & Qt::LeftButton)) return;
    const QPoint ort = event->position().toPoint();
    const auto i = static_cast<std::size_t>(m_zugPunkt);
    if (m_zug == Zug::Punkt)
    {
        FormPunkt& punkt = m_form.punkte[i];
        // ein Punkt bleibt zwischen seinen Nachbarn
        if (!festX(m_zugPunkt))
            punkt.x = std::clamp(xVon(ort.x()), m_form.punkte[i - 1].x, m_form.punkte[i + 1].x);
        if (!festY(m_zugPunkt)) punkt.y = yVon(ort.y());
    }
    else
    {
        // ein Anfasser: seine Lage im Rechteck des Abschnitts, je 0…1
        FormPunkt& a = m_form.punkte[i];
        const FormPunkt& b = m_form.punkte[i + 1];
        const auto nr = static_cast<std::size_t>(m_zugNr);
        const double dx = b.x - a.x;
        const double dy = b.y - a.y;
        a.bezier[nr * 2] = dx > 1e-9 ? std::clamp((xVon(ort.x()) - a.x) / dx, 0.0, 1.0) : 0.0;
        // ein waagrechter Abschnitt hat keine Hoehe, in der der Anfasser wandern koennte
        if (std::abs(dy) > 1e-9) a.bezier[nr * 2 + 1] = std::clamp((yVon(ort.y()) - a.y) / dy, 0.0, 1.0);
    }
    m_bewegt = true;
    update();
}

void Formflaeche::mouseReleaseEvent(QMouseEvent*)
{
    if (m_zug == Zug::Keiner) return;
    const Zug zug = m_zug;
    m_zug = Zug::Keiner;
    if (!m_bewegt || gleich(m_form, m_vorZug)) return;
    Q_EMIT geaendert(m_form, zug == Zug::Punkt ? tr("Move curve point") : tr("Change curve handle"));
}

void Formflaeche::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_bearbeitbar || event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    m_zug = Zug::Keiner;
    const int punkt = punktBei(ort);
    if (punkt >= 0)
        loeschePunkt(punkt);  // die Enden bleiben
    else
        fuegePunktEin(xVon(ort.x()), yVon(ort.y()));
}

void Formflaeche::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_bearbeitbar || m_form.punkte.size() < 2) return;
    const QPoint ort = event->pos();
    const int punkt = punktBei(ort);
    const int letzter = static_cast<int>(m_form.punkte.size()) - 1;
    // der Abschnitt, der am Punkt beginnt — sonst der unter der Maus
    const int abschnitt = punkt >= 0 && punkt < letzter ? punkt : abschnittBei(xVon(ort.x()));

    QMenu menu(this);
    QMenu* formen = menu.addMenu(tr("Segment shape"));
    for (int f = 0; f < kKurvenformen; ++f)
    {
        const auto form = static_cast<Kurvenform>(f);
        QAction* a = formen->addAction(QString::fromLatin1(kurvenformName(form)));
        a->setCheckable(true);
        a->setChecked(m_form.punkte[static_cast<std::size_t>(abschnitt)].form == form);
        connect(a, &QAction::triggered, this, [this, abschnitt, form] { setzeAbschnittForm(abschnitt, form); });
    }
    menu.addSeparator();
    if (punkt < 0)
        menu.addAction(tr("Add point here"), this,
                       [this, ort] { fuegePunktEin(xVon(ort.x()), yVon(ort.y())); });
    else if (!festX(punkt))
        menu.addAction(tr("Delete point"), this, [this, punkt] { loeschePunkt(punkt); });
    menu.exec(event->globalPos());
}

void Formflaeche::keyPressEvent(QKeyEvent* event)
{
    if (m_bearbeitbar && m_gewaehlt >= 0 &&
        (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace))
    {
        loeschePunkt(m_gewaehlt);
        return;
    }
    QWidget::keyPressEvent(event);
}

// =============================================================================
// KurvenFenster
// =============================================================================

KurvenFenster::KurvenFenster(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("composerCurveEditor"));
    setWindowTitle(tr("Curve Editor"));
    setupUI();
    resize(460, 420);
    zeige(nullptr, {}, {}, false);
}

FormArt KurvenFenster::art() const
{
    return m_teil.art == TeilArt::Stueck ? FormArt::Stueck : FormArt::Blende;
}

Pegelform* KurvenFenster::form()
{
    return formVon(m_clip, m_teil);
}

template <typename Tu>
void KurvenFenster::aendereWert(const QString& was, Tu tu)
{
    if (!m_hat || !m_bearbeitbar) return;
    VisualClip neu = m_clip;
    schreibeFest(neu);
    tu(neu);
    Q_EMIT clipGeaendert(neu, was);
}

void KurvenFenster::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);

    m_titel = new QLabel(this);
    m_titel->setObjectName(QStringLiteral("curveTitle"));
    root->addWidget(m_titel);

    // --- Preset-Zeile
    auto* zeile = new QHBoxLayout();
    zeile->addWidget(new QLabel(tr("Preset:"), this));
    m_presets = new QComboBox(this);
    m_presets->setObjectName(QStringLiteral("curvePreset"));
    m_presets->setToolTip(tr("Curve presets of this kind — fades for a fade in or fade out, shapes "
                             "for a shape inside the clip. Choosing one replaces the curve; "
                             "duration, width and position stay.\n\n"
                             "A fade preset fits both ends: as a fade out it plays backwards."));
    zeile->addWidget(m_presets, 1);
    m_speichern = new QPushButton(tr("Save..."), this);
    m_speichern->setObjectName(QStringLiteral("curveSave"));
    m_speichern->setToolTip(tr("Save this curve as a preset of your own"));
    zeile->addWidget(m_speichern);
    m_loeschen = new QPushButton(tr("Delete"), this);
    m_loeschen->setObjectName(QStringLiteral("curveDelete"));
    m_loeschen->setToolTip(tr("Delete the chosen preset — only your own; the built-in ones stay"));
    zeile->addWidget(m_loeschen);
    root->addLayout(zeile);

    // --- die Form
    m_flaeche = new Formflaeche(this);
    m_flaeche->setToolTip(
        tr("The curve, without time: left to right is the duration or width, bottom to top the "
           "level.\n\n"
           "Drag a point to move it\n"
           "Double-click the empty area — a new point\n"
           "Double-click a point, or Del — remove it\n"
           "Right-click — the shape of that segment (Bezier shows two handles)\n\n"
           "A fade runs from bottom left to top right; as a fade out it plays backwards. "
           "A shape says how much of the base level stays: top all of it, bottom nothing."));
    root->addWidget(m_flaeche, 1);
    m_hinweis = new QLabel(tr("No curve selected.\n\nIn the Composer, with Edit on: double-click a "
                              "fade or a shape of a clip — or right-click the clip."),
                           this);
    m_hinweis->setObjectName(QStringLiteral("curveHint"));
    m_hinweis->setAlignment(Qt::AlignCenter);
    m_hinweis->setWordWrap(true);
    root->addWidget(m_hinweis, 1);

    // --- das Teil am Clip
    const auto zeitfeld = [this](const char* name) {
        auto* feld = new QDoubleSpinBox(this);
        feld->setObjectName(QLatin1String(name));
        feld->setDecimals(2);
        feld->setSingleStep(0.1);
        feld->setRange(0.0, 86400.0);
        feld->setSuffix(tr(" s"));
        feld->setKeyboardTracking(false);
        return feld;
    };
    zeile = new QHBoxLayout();
    m_dauerTitel = new QLabel(this);
    zeile->addWidget(m_dauerTitel);
    m_dauer = zeitfeld("curveDuration");
    m_dauer->setMinimum(0.05);
    zeile->addWidget(m_dauer);
    m_jetzt = new QLabel(this);
    m_jetzt->setObjectName(QStringLiteral("curveNow"));
    m_jetzt->setToolTip(tr("The clip is too short for its fades and shapes: those that touch are "
                           "squeezed. The value on the left is the one you set — it comes back "
                           "when the clip gets longer. Change it here and what you see now "
                           "becomes the set value."));
    zeile->addWidget(m_jetzt);
    zeile->addSpacing(10);
    m_lageTitel = new QLabel(tr("Position:"), this);
    zeile->addWidget(m_lageTitel);
    m_lage = zeitfeld("curvePosition");
    m_lage->setToolTip(tr("Where the middle of the shape lies, counted from the start of the "
                          "clip. When the clip changes its length, the shape keeps its share of it."));
    zeile->addWidget(m_lage);
    m_gekippt = new QCheckBox(tr("Flipped"), this);
    m_gekippt->setObjectName(QStringLiteral("curveFlipped"));
    m_gekippt->setToolTip(tr("Off: the shape lowers the level, from the base level towards "
                             "nothing (a dip).\n"
                             "On: top and bottom swap — the shape raises the level, from the base "
                             "level towards full (a bump)."));
    zeile->addWidget(m_gekippt);
    zeile->addStretch(1);
    root->addLayout(zeile);

    zeile = new QHBoxLayout();
    zeile->addWidget(new QLabel(tr("Base level of the clip:"), this));
    m_grund = new QDoubleSpinBox(this);
    m_grund->setObjectName(QStringLiteral("curveBase"));
    m_grund->setDecimals(0);
    m_grund->setRange(0.0, 100.0);
    m_grund->setSingleStep(5.0);
    m_grund->setSuffix(tr(" %"));
    m_grund->setKeyboardTracking(false);
    m_grund->setToolTip(tr("The level of the clip between its fades and shapes. Fades run from "
                           "nothing up to it; a shape lowers from it or, flipped, raises from it.\n\n"
                           "At 0 % the clip shows only inside its flipped shapes."));
    zeile->addWidget(m_grund);
    zeile->addStretch(1);
    root->addLayout(zeile);

    connect(m_flaeche, &Formflaeche::geaendert, this, [this](const Pegelform& neueForm, const QString& was) {
        if (!m_hat) return;
        VisualClip neu = m_clip;
        if (Pegelform* ziel = formVon(neu, m_teil)) *ziel = neueForm;
        Q_EMIT clipGeaendert(neu, was);
    });
    connect(m_dauer, &QDoubleSpinBox::valueChanged, this, [this](double s) {
        const Teil teil = m_teil;
        aendereWert(teil.art == TeilArt::Stueck ? tr("Change shape width") : tr("Change fade duration"),
                    [teil, s](VisualClip& c) {
                        if (teil.art == TeilArt::Ein)
                            c.ein.dauerMs = millis(s);
                        else if (teil.art == TeilArt::Aus)
                            c.aus.dauerMs = millis(s);
                        else
                            c.stuecke[static_cast<std::size_t>(teil.stueck)].weiteMs = millis(s);
                    });
    });
    connect(m_lage, &QDoubleSpinBox::valueChanged, this, [this](double s) {
        const Teil teil = m_teil;
        if (teil.art != TeilArt::Stueck) return;
        aendereWert(tr("Move shape"), [teil, s](VisualClip& c) {
            const double dauer = std::max(1.0, static_cast<double>(c.dauerMs()));
            c.stuecke[static_cast<std::size_t>(teil.stueck)].lage = std::clamp(s * 1000.0 / dauer, 0.0, 1.0);
        });
    });
    connect(m_gekippt, &QCheckBox::clicked, this, [this](bool an) {
        if (!m_hat || !m_bearbeitbar || m_teil.art != TeilArt::Stueck) return;
        VisualClip neu = m_clip;
        neu.stuecke[static_cast<std::size_t>(m_teil.stueck)].gekippt = an;
        Q_EMIT clipGeaendert(neu, tr("Flip shape"));
    });
    connect(m_grund, &QDoubleSpinBox::valueChanged, this, [this](double prozent) {
        if (!m_hat || !m_bearbeitbar) return;
        VisualClip neu = m_clip;
        neu.grundpegel = std::clamp(prozent / 100.0, 0.0, 1.0);
        Q_EMIT clipGeaendert(neu, tr("Change base level"));
    });
    connect(m_presets, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        const QString name = m_presets->itemData(index).toString();
        if (name.isEmpty() || !ladePreset(name)) zeigePresets();  // zurueck auf das, was gilt
    });
    connect(m_speichern, &QPushButton::clicked, this, [this] {
        if (!m_hat) return;
        // der Name eines eigenen Presets als Vorschlag: so laesst es sich ueberschreiben
        const bool eigenes = !m_presets->currentData(Qt::UserRole + 1).toBool();
        bool ok = false;
        const QString name = QInputDialog::getText(
            this, tr("Save Curve Preset"), tr("Name:"), QLineEdit::Normal,
            eigenes ? m_presets->currentData().toString() : QString(), &ok);
        if (!ok) return;
        if (!speicherePreset(name))
            QMessageBox::warning(this, tr("Save Curve Preset"),
                                 tr("The preset could not be saved. Use letters, digits, spaces, "
                                    "- and _ for the name."));
    });
    connect(m_loeschen, &QPushButton::clicked, this, [this] {
        const QString name = m_presets->currentData().toString();
        if (name.isEmpty()) return;
        if (QMessageBox::question(this, tr("Delete Curve Preset"),
                                  tr("Delete the preset \"%1\"?").arg(name)) == QMessageBox::Yes)
            (void)loeschePreset(name);
    });
}

void KurvenFenster::zeige(const VisualClip* clip, Teil teil, const QString& name, bool bearbeitbar)
{
    m_hat = clip != nullptr && hatTeil(*clip, teil);
    m_clip = clip != nullptr ? *clip : VisualClip{};
    m_teil = teil;
    m_bearbeitbar = m_hat && bearbeitbar;

    QString titel = tr("No curve selected");
    if (m_hat)
    {
        const QString was = teil.art == TeilArt::Ein   ? tr("Fade in")
                            : teil.art == TeilArt::Aus ? tr("Fade out")
                                                       : tr("Shape %1").arg(teil.stueck + 1);
        titel = QStringLiteral("%1 — %2").arg(was, name);
        if (!m_bearbeitbar) titel += tr("  (Edit is off)");
    }
    m_titel->setText(titel);

    m_flaeche->setVisible(m_hat);
    m_hinweis->setVisible(!m_hat);
    if (m_hat) m_flaeche->setForm(*form());
    m_flaeche->setBearbeitbar(m_bearbeitbar);
    m_presets->setEnabled(m_bearbeitbar);
    m_speichern->setEnabled(m_hat);
    for (QWidget* w : {static_cast<QWidget*>(m_dauer), static_cast<QWidget*>(m_lage),
                       static_cast<QWidget*>(m_gekippt), static_cast<QWidget*>(m_grund)})
        w->setEnabled(m_bearbeitbar);
    zeigeWerte();
    zeigePresets();
}

void KurvenFenster::zeigeWerte()
{
    const bool stueck = m_hat && m_teil.art == TeilArt::Stueck;
    m_dauerTitel->setText(stueck ? tr("Width:") : tr("Duration:"));
    m_lageTitel->setVisible(stueck);
    m_lage->setVisible(stueck);
    m_gekippt->setVisible(stueck);

    const QSignalBlocker ruheDauer(m_dauer);
    const QSignalBlocker ruheLage(m_lage);
    const QSignalBlocker ruheGrund(m_grund);
    m_grund->setValue(m_clip.grundpegel * 100.0);
    if (!m_hat)
    {
        m_dauer->setValue(m_dauer->minimum());
        m_jetzt->clear();
        return;
    }
    const std::int64_t eingestellt =
        m_teil.art == TeilArt::Ein   ? m_clip.ein.dauerMs
        : m_teil.art == TeilArt::Aus ? m_clip.aus.dauerMs
                                     : m_clip.stuecke[static_cast<std::size_t>(m_teil.stueck)].weiteMs;
    m_dauer->setValue(sekunden(eingestellt));
    // beide Zahlen, wenn das Teil gestaucht ist (Entscheid Patrik S96)
    const Bereich bereich = bereichVon(m_clip, m_teil);
    m_jetzt->setText(bereich.gestaucht ? tr("(now %1 s)").arg(sekunden(bereich.weiteMs()), 0, 'f', 2)
                                       : QString());
    if (stueck)
    {
        const ClipStueck& s = m_clip.stuecke[static_cast<std::size_t>(m_teil.stueck)];
        m_lage->setMaximum(std::max(0.0, sekunden(m_clip.dauerMs())));
        m_lage->setValue(s.lage * sekunden(m_clip.dauerMs()));
        m_gekippt->setChecked(s.gekippt);
    }
}

void KurvenFenster::zeigePresets()
{
    const QSignalBlocker ruhe(m_presets);
    m_presets->clear();
    m_presets->addItem(tr("(custom)"), QString());
    int gleiches = 0;
    if (m_hat)
    {
        const Pegelform* jetzt = form();
        for (const formpresets::Eintrag& e : formpresets::liste(art()))
        {
            m_presets->addItem(e.name, e.name);
            m_presets->setItemData(m_presets->count() - 1, e.mitgeliefert, Qt::UserRole + 1);
            Pegelform f;
            if (gleiches == 0 && formpresets::lade(e.pfad, art(), f) && gleich(f, *jetzt))
                gleiches = m_presets->count() - 1;
        }
    }
    m_presets->setCurrentIndex(gleiches);
    // loeschen laesst sich nur ein eigenes
    m_loeschen->setEnabled(gleiches > 0 && !m_presets->itemData(gleiches, Qt::UserRole + 1).toBool());
}

bool KurvenFenster::ladePreset(const QString& name)
{
    if (!m_hat || !m_bearbeitbar) return false;
    const formpresets::Eintrag e = formpresets::finde(art(), name);
    Pegelform geladen;
    if (e.pfad.isEmpty() || !formpresets::lade(e.pfad, art(), geladen)) return false;
    VisualClip neu = m_clip;
    *formVon(neu, m_teil) = geladen;
    Q_EMIT clipGeaendert(neu, tr("Load curve preset"));
    return true;
}

bool KurvenFenster::speicherePreset(const QString& name)
{
    if (!m_hat) return false;
    const bool ok = formpresets::speichere(name, *form());
    zeigePresets();
    return ok;
}

bool KurvenFenster::loeschePreset(const QString& name)
{
    const bool ok = formpresets::loesche(art(), formpresets::finde(art(), name));
    zeigePresets();
    return ok;
}

}  // namespace lumi::composer
