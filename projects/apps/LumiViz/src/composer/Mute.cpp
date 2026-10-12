/**
 * @file Mute.cpp
 * @brief Composer: Mute-Profile und der Verlauf des Pegels (S101)
 */

#include "composer/Mute.hpp"

#include "audio/IAudioPlayer.hpp"
#include "core/Mitgeliefert.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QCoreApplication>
#include <QPointer>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>

#include <algorithm>
#include <cmath>

namespace lumi::composer
{

namespace
{
const QString kFormat = QStringLiteral("lumiviz-mute");
constexpr int kVersion = 1;
const QLatin1String kGewaehlt("composer/muteProfile");
const QString kUnterordner = QStringLiteral("composer/mutes");
}  // namespace

// =============================================================================
// Das Profil
// =============================================================================

void MuteProfil::richte()
{
    ausblendeMs = std::max<std::int64_t>(0, ausblendeMs);
    einblendeMs = std::max<std::int64_t>(0, einblendeMs);
    zielPegel = std::clamp(zielPegel, 0.0, 1.0);
    for (Pegelform* f : {&ausblendeForm, &einblendeForm})
    {
        f->art = FormArt::Blende;
        f->richte();
    }
}

QJsonObject profilAlsJson(const MuteProfil& profil)
{
    MuteProfil p = profil;
    p.richte();
    QJsonObject o;
    o.insert(QStringLiteral("format"), kFormat);
    o.insert(QStringLiteral("version"), kVersion);
    o.insert(QStringLiteral("level"), p.zielPegel);
    o.insert(QStringLiteral("out"), static_cast<double>(p.ausblendeMs));
    o.insert(QStringLiteral("outShape"), formAlsJson(p.ausblendeForm));
    o.insert(QStringLiteral("in"), static_cast<double>(p.einblendeMs));
    o.insert(QStringLiteral("inShape"), formAlsJson(p.einblendeForm));
    o.insert(QStringLiteral("visualsFollow"), p.visualsFolgen);
    return o;
}

bool profilAusJson(const QJsonObject& objekt, MuteProfil& profil)
{
    if (objekt.value(QStringLiteral("format")).toString() != kFormat) return false;
    MuteProfil p;
    p.zielPegel = objekt.value(QStringLiteral("level")).toDouble(0.0);
    p.ausblendeMs = static_cast<std::int64_t>(objekt.value(QStringLiteral("out")).toDouble(0.0));
    p.einblendeMs = static_cast<std::int64_t>(objekt.value(QStringLiteral("in")).toDouble(0.0));
    p.visualsFolgen = objekt.value(QStringLiteral("visualsFollow")).toBool(false);
    // ohne Form: linear
    if (!formAusJson(objekt.value(QStringLiteral("outShape")).toObject(), p.ausblendeForm))
        p.ausblendeForm = einfacheBlende();
    if (!formAusJson(objekt.value(QStringLiteral("inShape")).toObject(), p.einblendeForm))
        p.einblendeForm = einfacheBlende();
    p.richte();
    profil = std::move(p);
    return true;
}

bool gleich(const MuteProfil& a, const MuteProfil& b)
{
    return profilAlsJson(a) == profilAlsJson(b);
}

// =============================================================================
// Der Verlauf
// =============================================================================

void MuteVerlauf::setProfil(const MuteProfil& profil)
{
    m_profil = profil;
    m_profil.richte();
}

void MuteVerlauf::schalte(bool stumm, std::int64_t jetztMs)
{
    if (stumm == m_stumm) return;
    const double jetzt = faktorBei(jetztMs);
    m_laufend = m_profil;
    m_stumm = stumm;
    m_von = jetzt;
    m_ziel = stumm ? m_laufend.zielPegel : 1.0;
    m_startMs = jetztMs;
    // Der ganze Weg (voll <-> Zielpegel) dauert die Zeit des Profils; ein
    // Teilweg — mitten in einer Blende umgeschaltet — anteilig kuerzer.
    const double ganz = 1.0 - m_laufend.zielPegel;
    const double anteil = ganz > 0.0 ? std::clamp(std::abs(m_ziel - m_von) / ganz, 0.0, 1.0) : 0.0;
    const std::int64_t voll = stumm ? m_laufend.ausblendeMs : m_laufend.einblendeMs;
    m_dauerMs = static_cast<std::int64_t>(std::llround(static_cast<double>(voll) * anteil));
}

double MuteVerlauf::faktorBei(std::int64_t jetztMs) const
{
    if (m_dauerMs <= 0 || jetztMs >= m_startMs + m_dauerMs) return m_ziel;
    if (jetztMs <= m_startMs) return m_von;
    const double x = static_cast<double>(jetztMs - m_startMs) / static_cast<double>(m_dauerMs);
    // die Ausblendung liest ihre Form rueckwaerts (1 -> 0), wie am Clip
    const double weg = m_stumm ? 1.0 - blendenPegel(m_laufend.ausblendeForm, x, true)
                               : blendenPegel(m_laufend.einblendeForm, x, false);
    return m_von + (m_ziel - m_von) * std::clamp(weg, 0.0, 1.0);
}

bool MuteVerlauf::blendet(std::int64_t jetztMs) const
{
    return m_dauerMs > 0 && jetztMs < m_startMs + m_dauerMs;
}

// =============================================================================
// Der Schalter der App
// =============================================================================

namespace
{
QPointer<MuteSchalter>& derSchalter()
{
    static QPointer<MuteSchalter> schalter;  // wird null, wenn die QApplication ihn loescht
    return schalter;
}
constexpr int kTaktMs = 16;
}  // namespace

MuteSchalter::MuteSchalter(QObject* parent) : QObject(parent)
{
    m_uhr.start();
    m_takt = new QTimer(this);
    m_takt->setInterval(kTaktMs);
    connect(m_takt, &QTimer::timeout, this, &MuteSchalter::takt);
    // Beim Beenden: kein Takt mehr auf einen Player, den es gleich nicht mehr gibt
    if (QCoreApplication* app = QCoreApplication::instance())
        connect(app, &QCoreApplication::aboutToQuit, this, [this] {
            m_takt->stop();
            m_player = nullptr;
        });
    ladeProfil();
}

MuteSchalter& MuteSchalter::instanz()
{
    if (derSchalter().isNull()) derSchalter() = new MuteSchalter(QCoreApplication::instance());
    return *derSchalter();
}

MuteSchalter* MuteSchalter::vorhanden()
{
    return derSchalter().data();
}

void MuteSchalter::setPlayer(IAudioPlayer* player)
{
    m_player = player;
    if (m_player != nullptr) m_player->setPegel(static_cast<float>(faktor()));
}

std::int64_t MuteSchalter::jetzt() const
{
    return m_testUhr ? m_testUhr() : m_uhr.elapsed();
}

void MuteSchalter::setUhrFuerTests(std::function<std::int64_t()> uhr)
{
    m_testUhr = std::move(uhr);
}

void MuteSchalter::ladeProfil()
{
    m_verlauf.setProfil(muteprofile::gewaehltesProfil());
}

void MuteSchalter::schalte(bool stumm)
{
    if (stumm == m_verlauf.stumm()) return;
    // „Visuals follow" gilt fuer die ganze Absenkung, auch fuer ihr Einblenden:
    // was beim Stummschalten galt, gilt bis der Ton wieder voll ist.
    if (stumm) m_visualsFolgen = m_verlauf.profil().visualsFolgen;
    m_verlauf.schalte(stumm, jetzt());
    takt();
    if (m_verlauf.blendet(jetzt())) m_takt->start();
    Q_EMIT geschaltet(stumm);
}

double MuteSchalter::faktor() const
{
    return m_verlauf.faktorBei(jetzt());
}

double MuteSchalter::faktorFuerVisuals() const
{
    return m_visualsFolgen ? faktor() : 1.0;
}

void MuteSchalter::takt()
{
    const std::int64_t zeit = jetzt();
    if (m_player != nullptr) m_player->setPegel(static_cast<float>(m_verlauf.faktorBei(zeit)));
    if (!m_verlauf.blendet(zeit)) m_takt->stop();
}

// =============================================================================
// Profile als Dateien
// =============================================================================

namespace muteprofile
{

namespace
{

QString& testWurzel()
{
    static QString wurzel;
    return wurzel;
}

void sammle(const QString& ordner, bool mitgeliefert, std::vector<Eintrag>& aus)
{
    if (ordner.isEmpty()) return;
    const QDir dir(ordner);
    if (!dir.exists()) return;
    const QStringList dateien = dir.entryList({QLatin1Char('*') + QLatin1String(endung())}, QDir::Files,
                                              QDir::Name | QDir::IgnoreCase);
    for (const QString& datei : dateien)
        aus.push_back({QFileInfo(datei).completeBaseName(), dir.filePath(datei), mitgeliefert});
}

/// Neben der Exe oder im Projektordner (core/Mitgeliefert.hpp, S114).
QString sucheAsset()
{
    return lumi::core::mitgeliefert::ordner(kUnterordner);
}

}  // namespace

const char* endung()
{
    return ".lvmute";
}

QString assetOrdner()
{
    if (!testWurzel().isEmpty()) return QDir(testWurzel()).filePath(QStringLiteral("asset/") + kUnterordner);
    static const QString kOrdner = sucheAsset();  // einmal aufgeloest und gemerkt
    return kOrdner;
}

QString benutzerOrdner()
{
    if (!testWurzel().isEmpty()) return QDir(testWurzel()).filePath(QStringLiteral("user/") + kUnterordner);
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (basis.isEmpty()) return {};
    return QDir(basis).filePath(kUnterordner);
}

std::vector<Eintrag> liste()
{
    std::vector<Eintrag> eigene;
    sammle(benutzerOrdner(), false, eigene);
    std::vector<Eintrag> aus;
    sammle(assetOrdner(), true, aus);
    // ein eigenes mit dem Namen eines mitgelieferten verdraengt es
    aus.erase(std::remove_if(aus.begin(), aus.end(),
                             [&eigene](const Eintrag& e) {
                                 return std::any_of(eigene.cbegin(), eigene.cend(), [&e](const Eintrag& o) {
                                     return QString::compare(o.name, e.name, Qt::CaseInsensitive) == 0;
                                 });
                             }),
              aus.end());
    aus.insert(aus.end(), eigene.begin(), eigene.end());
    return aus;
}

Eintrag finde(const QString& name)
{
    for (const Eintrag& e : liste())
        if (QString::compare(e.name, name, Qt::CaseInsensitive) == 0) return e;
    return {};
}

bool lade(const QString& pfad, MuteProfil& profil)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    return profilAusJson(doc.object(), profil);
}

bool speichere(const QString& name, const MuteProfil& profil, QString* pfad)
{
    const QString sauber = lumi::multieffect::nodepresets::sanitize(name);
    const QString ordner = benutzerOrdner();
    if (sauber.isEmpty() || ordner.isEmpty()) return false;
    if (!QDir().mkpath(ordner)) return false;
    const QString ziel = QDir(ordner).filePath(sauber + QLatin1String(endung()));
    QSaveFile datei(ziel);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    if (datei.write(QJsonDocument(profilAlsJson(profil)).toJson(QJsonDocument::Indented)) < 0 ||
        !datei.commit())
        return false;
    if (pfad != nullptr) *pfad = ziel;
    return true;
}

bool loesche(const Eintrag& eintrag)
{
    if (eintrag.mitgeliefert || eintrag.pfad.isEmpty()) return false;
    // nur, was wirklich im Benutzerordner liegt
    const QString ordner = QDir::cleanPath(benutzerOrdner());
    if (ordner.isEmpty() ||
        QDir::cleanPath(QFileInfo(eintrag.pfad).absolutePath()).compare(ordner, Qt::CaseInsensitive) != 0)
        return false;
    return QFile::remove(eintrag.pfad);
}

QString gewaehlt()
{
    return QSettings().value(kGewaehlt).toString();
}

void setGewaehlt(const QString& name)
{
    if (name.isEmpty())
        QSettings().remove(kGewaehlt);
    else
        QSettings().setValue(kGewaehlt, name);
}

MuteProfil gewaehltesProfil()
{
    MuteProfil profil;
    const QString name = gewaehlt();
    if (name.isEmpty()) return profil;
    const Eintrag e = finde(name);
    if (!e.pfad.isEmpty()) (void)lade(e.pfad, profil);
    return profil;
}

void setzeWurzelFuerTests(const QString& wurzel)
{
    testWurzel() = wurzel;
}

}  // namespace muteprofile

}  // namespace lumi::composer
