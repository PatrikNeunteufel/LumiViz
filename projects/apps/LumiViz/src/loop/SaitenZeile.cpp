/**
 * @file SaitenZeile.cpp
 * @brief Die Zeile Tuning des Instrument-Creators (Stufe 23e)
 */
#include "loop/SaitenZeile.hpp"

#include "loop/Instrument.hpp"  // tonName

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLayout>
#include <QSignalBlocker>
#include <QStandardItemModel>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

/// Ein Layout, das seine Teile nebeneinander legt und umbricht, wenn die
/// Breite nicht reicht — die Hoehe folgt der Zahl der Zeilen.
class Fliesslage : public QLayout
{
public:
    explicit Fliesslage(QWidget* eltern) : QLayout(eltern) { setContentsMargins(0, 0, 0, 0); }
    ~Fliesslage() override
    {
        while (QLayoutItem* teil = takeAt(0)) delete teil;
    }
    void addItem(QLayoutItem* teil) override { m_teile.push_back(teil); }
    [[nodiscard]] int count() const override { return static_cast<int>(m_teile.size()); }
    [[nodiscard]] QLayoutItem* itemAt(int platz) const override
    {
        return platz >= 0 && platz < count() ? m_teile[static_cast<std::size_t>(platz)] : nullptr;
    }
    QLayoutItem* takeAt(int platz) override
    {
        if (platz < 0 || platz >= count()) return nullptr;
        QLayoutItem* teil = m_teile[static_cast<std::size_t>(platz)];
        m_teile.erase(m_teile.begin() + platz);
        return teil;
    }
    [[nodiscard]] Qt::Orientations expandingDirections() const override { return {}; }
    [[nodiscard]] bool hasHeightForWidth() const override { return true; }
    [[nodiscard]] int heightForWidth(int breite) const override { return lege(QRect(0, 0, breite, 0), false); }
    void setGeometry(const QRect& rahmen) override
    {
        QLayout::setGeometry(rahmen);
        lege(rahmen, true);
    }
    [[nodiscard]] QSize sizeHint() const override { return minimumSize(); }
    [[nodiscard]] QSize minimumSize() const override
    {
        QSize groesse;
        for (const QLayoutItem* teil : m_teile)
            if (!teil->isEmpty()) groesse = groesse.expandedTo(teil->minimumSize());
        return groesse;
    }

private:
    /// Die Teile in den Rahmen legen (`setzen`) oder nur die Hoehe ausrechnen.
    int lege(const QRect& rahmen, bool setzen) const
    {
        const int abstand = spacing() >= 0 ? spacing() : 6;
        int x = rahmen.x();
        int y = rahmen.y();
        int zeilenHoehe = 0;
        for (QLayoutItem* teil : m_teile)
        {
            if (teil->isEmpty()) continue;  // ausgeblendet
            const QSize groesse = teil->sizeHint();
            if (x > rahmen.x() && x + groesse.width() > rahmen.right() + 1)
            {
                x = rahmen.x();
                y += zeilenHoehe + abstand;
                zeilenHoehe = 0;
            }
            if (setzen) teil->setGeometry(QRect(QPoint(x, y), groesse));
            x += groesse.width() + abstand;
            zeilenHoehe = std::max(zeilenHoehe, groesse.height());
        }
        return y + zeilenHoehe - rahmen.y();
    }

    std::vector<QLayoutItem*> m_teile;
};

/// Ein Name und sein Feld, die beim Umbruch zusammenbleiben.
QWidget* paar(QWidget* eltern, QLabel* name, QWidget* feld)
{
    auto* beide = new QWidget(eltern);
    auto* reihe = new QHBoxLayout(beide);
    reihe->setContentsMargins(0, 0, 0, 0);
    reihe->addWidget(name);
    reihe->addWidget(feld);
    return beide;
}

}  // namespace

// --- TonFeld --------------------------------------------------------------

TonFeld::TonFeld(QWidget* parent) : QSpinBox(parent)
{
    setRange(0, 127);
    setToolTip(tr("Pitch of the open string — type a name (E2, C#4, Bb3) or use the arrows"));
}

int TonFeld::hoeheAus(const QString& text)
{
    const QString t = text.trimmed();
    if (t.isEmpty()) return -1;
    bool zahl = false;
    if (const int wert = t.toInt(&zahl); zahl) return wert >= 0 && wert <= 127 ? wert : -1;
    static constexpr int kStufe[7] = {9, 11, 0, 2, 4, 5, 7};  // A B C D E F G
    const QChar buchstabe = t.at(0).toUpper();
    if (buchstabe < QLatin1Char('A') || buchstabe > QLatin1Char('G')) return -1;
    int stufe = kStufe[buchstabe.unicode() - 'A'];
    qsizetype i = 1;
    if (i < t.size() && t.at(i) == QLatin1Char('#'))
    {
        ++stufe;
        ++i;
    }
    else if (i < t.size() && t.at(i) == QLatin1Char('b'))
    {
        --stufe;
        ++i;
    }
    bool ok = false;
    const int oktave = t.mid(i).toInt(&ok);
    if (!ok) return -1;
    const int hoehe = (oktave + 1) * 12 + stufe;
    return hoehe >= 0 && hoehe <= 127 ? hoehe : -1;
}

QString TonFeld::textFromValue(int wert) const
{
    return tonName(wert);
}

int TonFeld::valueFromText(const QString& text) const
{
    const int hoehe = hoeheAus(text);
    return hoehe >= 0 ? hoehe : value();
}

QValidator::State TonFeld::validate(QString& text, int& /*pos*/) const
{
    if (hoeheAus(text) >= 0) return QValidator::Acceptable;
    // was beim Tippen auf dem Weg zu einem Namen liegt: „C", „C#", „Eb", „-"
    const QString t = text.trimmed();
    if (t.isEmpty() || t.size() <= 3) return QValidator::Intermediate;
    return QValidator::Invalid;
}

// --- SaitenZeile ----------------------------------------------------------

SaitenZeile::SaitenZeile(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("creatorTuningRow"));
    auto* lage = new Fliesslage(this);
    lage->setSpacing(6);

    m_vorlage = new QComboBox(this);
    m_vorlage->setObjectName(QStringLiteral("creatorTuning"));
    m_vorlage->setToolTip(tr("A string instrument: its tuning. None = the instrument has no strings"));
    m_vorlage->addItem(tr("None"), kKeine);
    const std::vector<Stimmungsvorlage>& vorlagen = stimmungsvorlagen();
    for (std::size_t i = 0; i < vorlagen.size(); ++i) m_vorlage->addItem(QLatin1String(vorlagen[i].titel), static_cast<int>(i));
    m_vorlage->addItem(tr("Custom"), kEigene);
    // Custom waehlt man nicht: es steht da, wenn die Toene zu keiner Vorlage passen
    if (auto* modell = qobject_cast<QStandardItemModel*>(m_vorlage->model()))
        modell->item(m_vorlage->count() - 1)->setEnabled(false);
    lage->addWidget(paar(this, new QLabel(tr("Tuning"), this), m_vorlage));

    m_anzahlName = new QLabel(tr("Strings"), this);
    m_anzahl = new QSpinBox(this);
    m_anzahl->setObjectName(QStringLiteral("creatorStringCount"));
    m_anzahl->setRange(1, kSaitenMax);
    m_anzahl->setToolTip(tr("How many strings (up to %1)").arg(kSaitenMax));
    // so breit wie der Inhalt und die Pfeile — sonst bricht die Zeile schon bei 1280 px um
    const auto passend = [](QAbstractSpinBox* feld, const QString& laengster, int rand = 40) {
        feld->setFixedWidth(feld->fontMetrics().horizontalAdvance(laengster) + rand);
    };
    passend(m_anzahl, QStringLiteral("12"));
    lage->addWidget(paar(this, m_anzahlName, m_anzahl));

    for (int i = 0; i < kSaitenMax; ++i)
    {
        auto* feld = new TonFeld(this);
        feld->setObjectName(QStringLiteral("creatorString%1").arg(i + 1));
        feld->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        passend(feld, QStringLiteral("C#-1"), 34);
        lage->addWidget(feld);
        m_toene.push_back(feld);
        connect(feld, &QSpinBox::valueChanged, this, [this, i](int hoehe) { setzeTon(i, hoehe); });
    }

    m_capoName = new QLabel(tr("Capo"), this);
    m_capo = new QSpinBox(this);
    m_capo->setObjectName(QStringLiteral("creatorCapo"));
    m_capo->setRange(0, kCapoMax);
    m_capo->setToolTip(tr("Every open string sounds this many frets higher"));
    passend(m_capo, QStringLiteral("12"));
    lage->addWidget(paar(this, m_capoName, m_capo));

    m_mensurName = new QLabel(tr("Scale"), this);
    m_mensur = new QDoubleSpinBox(this);
    m_mensur->setObjectName(QStringLiteral("creatorScale"));
    m_mensur->setRange(kMensurMinMm, kMensurMaxMm);
    m_mensur->setDecimals(0);
    m_mensur->setSuffix(tr(" mm"));
    passend(m_mensur, QStringLiteral("1200 mm"));
    m_mensur->setToolTip(tr("Scale length: the vibrating length from nut to bridge — pickup positions are measured from the bridge"));
    lage->addWidget(paar(this, m_mensurName, m_mensur));

    m_wahlName = new QLabel(tr("String choice"), this);
    m_wahl = new QComboBox(this);
    m_wahl->setObjectName(QStringLiteral("creatorStringChoice"));
    m_wahl->addItem(tr("Per chord"), static_cast<int>(Saitenwahl::JeKlang));
    m_wahl->addItem(tr("Follow hand"), static_cast<int>(Saitenwahl::Hand));
    m_wahl->setToolTip(tr("How a note without its own string finds one — the default for new patterns.\n"
                          "Per chord: notes sounding together on different strings, frets close together.\n"
                          "Follow hand: the same, but staying near where the hand was."));
    lage->addWidget(paar(this, m_wahlName, m_wahl));

    connect(m_vorlage, &QComboBox::activated, this, [this](int index) { waehleVorlage(m_vorlage->itemData(index).toInt()); });
    connect(m_anzahl, &QSpinBox::valueChanged, this, [this](int anzahl) { setzeAnzahl(anzahl); });
    connect(m_capo, &QSpinBox::valueChanged, this, [this](int capo) { setzeCapo(capo); });
    connect(m_mensur, &QDoubleSpinBox::valueChanged, this, [this](double mm) { setzeMensur(mm); });
    connect(m_wahl, &QComboBox::activated, this,
            [this](int index) { setzeWahl(static_cast<Saitenwahl>(m_wahl->itemData(index).toInt())); });

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    zeigeFelder();
}

void SaitenZeile::zeige(const Saitensatz& saiten)
{
    if (saiten == m_saiten) return;
    m_saiten = saiten;
    zeigeFelder();
}

int SaitenZeile::vorlagePlatz() const
{
    if (!m_saiten.istSaiteninstrument()) return kKeine;
    const Stimmungsvorlage* vorlage = vorlageVon(m_saiten);
    if (vorlage == nullptr) return kEigene;
    const std::vector<Stimmungsvorlage>& vorlagen = stimmungsvorlagen();
    return static_cast<int>(vorlage - vorlagen.data());
}

void SaitenZeile::waehleVorlage(int platz)
{
    const std::vector<Stimmungsvorlage>& vorlagen = stimmungsvorlagen();
    if (platz == kKeine)
        m_saiten.leer.clear();
    else if (platz >= 0 && platz < static_cast<int>(vorlagen.size()))
    {
        m_saiten.leer = vorlagen[static_cast<std::size_t>(platz)].leer;
        m_saiten.mensurMm = vorlagen[static_cast<std::size_t>(platz)].mensurMm;
    }
    else
        return;  // Custom waehlt man nicht
    melde();
}

void SaitenZeile::setzeAnzahl(int anzahl)
{
    if (!m_saiten.istSaiteninstrument()) return;
    anzahl = std::clamp(anzahl, 1, kSaitenMax);
    if (anzahl == m_saiten.anzahl()) return;
    while (m_saiten.anzahl() < anzahl)
    {
        const int hoechste = *std::max_element(m_saiten.leer.begin(), m_saiten.leer.end());
        m_saiten.leer.push_back(std::min(127, hoechste + 5));
    }
    m_saiten.leer.resize(static_cast<std::size_t>(anzahl));
    melde();
}

void SaitenZeile::setzeTon(int saite, int hoehe)
{
    if (saite < 0 || saite >= m_saiten.anzahl()) return;
    hoehe = std::clamp(hoehe, 0, 127);
    if (m_saiten.leer[static_cast<std::size_t>(saite)] == hoehe) return;
    m_saiten.leer[static_cast<std::size_t>(saite)] = hoehe;
    melde();
}

void SaitenZeile::setzeCapo(int capo)
{
    capo = std::clamp(capo, 0, kCapoMax);
    if (!m_saiten.istSaiteninstrument() || capo == m_saiten.capo) return;
    m_saiten.capo = capo;
    melde();
}

void SaitenZeile::setzeMensur(double mm)
{
    mm = std::clamp(std::round(mm), kMensurMinMm, kMensurMaxMm);
    if (!m_saiten.istSaiteninstrument() || mm == mensurVon(m_saiten)) return;
    m_saiten.mensurMm = mm;
    melde();
}

void SaitenZeile::setzeWahl(Saitenwahl wahl)
{
    if (!m_saiten.istSaiteninstrument() || wahl == m_saiten.wahl) return;
    m_saiten.wahl = wahl;
    melde();
}

void SaitenZeile::melde()
{
    richteSaiten(m_saiten);
    zeigeFelder();
    Q_EMIT geaendert(m_saiten);
}

void SaitenZeile::zeigeFelder()
{
    const bool saiten = m_saiten.istSaiteninstrument();
    {
        const QSignalBlocker ruhe(m_vorlage);
        m_vorlage->setCurrentIndex(m_vorlage->findData(vorlagePlatz()));
    }
    for (QWidget* teil : {static_cast<QWidget*>(m_anzahlName), static_cast<QWidget*>(m_capoName),
                          static_cast<QWidget*>(m_mensurName), static_cast<QWidget*>(m_wahlName)})
        teil->parentWidget()->setVisible(saiten);
    {
        const QSignalBlocker ruhe(m_anzahl);
        m_anzahl->setValue(std::max(1, m_saiten.anzahl()));
    }
    for (int i = 0; i < kSaitenMax; ++i)
    {
        TonFeld* feld = m_toene[static_cast<std::size_t>(i)];
        const bool da = i < m_saiten.anzahl();
        feld->setVisible(da);
        if (!da) continue;
        const QSignalBlocker ruhe(feld);
        feld->setValue(m_saiten.leer[static_cast<std::size_t>(i)]);
    }
    {
        const QSignalBlocker ruhe(m_capo);
        m_capo->setValue(m_saiten.capo);
    }
    {
        const QSignalBlocker ruhe(m_mensur);
        m_mensur->setValue(mensurVon(m_saiten));
    }
    {
        const QSignalBlocker ruhe(m_wahl);
        m_wahl->setCurrentIndex(m_wahl->findData(static_cast<int>(m_saiten.wahl)));
    }
    updateGeometry();
}

}  // namespace lumi::loop
