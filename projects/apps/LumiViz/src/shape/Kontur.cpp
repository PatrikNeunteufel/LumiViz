/**
 * @file Kontur.cpp
 * @brief Klang aus einer Form: Zeichnung, Datei `.lvcontour`, Reihe (Stufe 19a)
 */

#include "shape/Kontur.hpp"

#include "core/Mitgeliefert.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QLocale>
#include <QPolygonF>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>
#include <QTransform>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace lumi::shape
{

namespace
{

const QString kFormat = QStringLiteral("lumiviz-contour");
constexpr double kPi = 3.14159265358979323846;

/// So kurz, dass die Zahl genau zurueckkommt.
QString zahl(double wert)
{
    return QString::number(wert, 'g', QLocale::FloatingPointShortest);
}

/// Den Zug schliessen, wenn er es nicht ist (eine Kontur ist immer geschlossen).
void schliesse(QPainterPath& pfad)
{
    if (!pfad.isEmpty()) pfad.closeSubpath();
}

QString& konturOrdnerImTest()
{
    static QString ordner;
    return ordner;
}

struct Wort
{
    const char* text;
    int wert;
};
constexpr std::array<Wort, 2> kNutzungen = {{{"layers", static_cast<int>(Nutzung::Schichten)}, {"onePath", static_cast<int>(Nutzung::EinPfad)}}};
constexpr std::array<Wort, 3> kSprungArten = {{{"listOrder", static_cast<int>(Spruenge::Liste)},
                                               {"shortest", static_cast<int>(Spruenge::Kuerzeste)},
                                               {"byHand", static_cast<int>(Spruenge::VonHand)}}};
constexpr std::array<Wort, 3> kHerkuenfte = {{{"drawn", static_cast<int>(HerkunftArt::Gezeichnet)},
                                              {"svg", static_cast<int>(HerkunftArt::Svg)},
                                              {"image", static_cast<int>(HerkunftArt::Bild)}}};

template <std::size_t N>
QString wortZu(const std::array<Wort, N>& tafel, int wert)
{
    for (const Wort& wort : tafel)
        if (wort.wert == wert) return QLatin1String(wort.text);
    return QLatin1String(tafel[0].text);
}

/// Der Wert zu einem Wort; unbekannt = der erste der Tafel.
template <std::size_t N>
int wertZu(const std::array<Wort, N>& tafel, const QString& text)
{
    for (const Wort& wort : tafel)
        if (text == QLatin1String(wort.text)) return wort.wert;
    return tafel[0].wert;
}

QJsonArray punktAlsJson(const QPointF& punkt)
{
    return QJsonArray{punkt.x(), punkt.y()};
}

bool punktAusJson(const QJsonValue& wert, QPointF& punkt)
{
    const QJsonArray paar = wert.toArray();
    if (paar.size() != 2 || !paar.at(0).isDouble() || !paar.at(1).isDouble()) return false;
    const double x = paar.at(0).toDouble();
    const double y = paar.at(1).toDouble();
    if (!std::isfinite(x) || !std::isfinite(y)) return false;
    punkt = QPointF(x, y);
    return true;
}

}  // namespace

// --- Zeichnung ------------------------------------------------------------------

QString konturFarbe(int kennung)
{
    // die Farben des Logos zuerst, dann weitere, die sich davon abheben
    static const std::array<const char*, 8> kFarben = {"#00e5ff", "#ff4fa3", "#ffb020", "#7f77dd",
                                                       "#1d9e75", "#d85a30", "#378add", "#97c459"};
    const int platz = ((std::max(kennung, 1) - 1) % static_cast<int>(kFarben.size()));
    return QLatin1String(kFarben[static_cast<std::size_t>(platz)]);
}

int Zeichnung::neueKontur(const QPainterPath& pfad, const QString& konturName, const QString& farbe)
{
    if (pfad.isEmpty() || konturen.size() >= static_cast<std::size_t>(kKonturenMax)) return 0;
    Kontur kontur;
    kontur.kennung = std::max(naechsteKennung, 1);
    kontur.name = konturName;
    kontur.farbe = farbe.isEmpty() ? konturFarbe(kontur.kennung) : farbe;
    kontur.pfad = pfad;
    schliesse(kontur.pfad);
    naechsteKennung = kontur.kennung + 1;
    konturen.push_back(kontur);
    return kontur.kennung;
}

const Kontur* Zeichnung::finde(int kennung) const
{
    const auto it = std::find_if(konturen.begin(), konturen.end(), [kennung](const Kontur& k) { return k.kennung == kennung; });
    return it == konturen.end() ? nullptr : &*it;
}

bool Zeichnung::entferne(int kennung)
{
    const auto it = std::find_if(konturen.begin(), konturen.end(), [kennung](const Kontur& k) { return k.kennung == kennung; });
    if (it == konturen.end()) return false;
    konturen.erase(it);
    std::erase_if(bruecken, [kennung](const Bruecke& b) { return b.von == kennung || b.nach == kennung; });
    return true;
}

// --- der Pfad als Text ------------------------------------------------------------

QString pfadAlsText(const QPainterPath& pfad)
{
    QStringList teile;
    QPointF anfang;
    QPointF letzter;
    int imZug = 0;  // Elemente seit dem letzten M
    const auto beende = [&] {
        if (imZug > 1 && letzter == anfang) teile << QStringLiteral("Z");
        imZug = 0;
    };
    for (int i = 0; i < pfad.elementCount(); ++i)
    {
        const QPainterPath::Element e = pfad.elementAt(i);
        if (e.type == QPainterPath::MoveToElement)
        {
            beende();
            teile << QStringLiteral("M") << zahl(e.x) << zahl(e.y);
            anfang = QPointF(e.x, e.y);
        }
        else if (e.type == QPainterPath::LineToElement)
        {
            teile << QStringLiteral("L") << zahl(e.x) << zahl(e.y);
        }
        else if (e.type == QPainterPath::CurveToElement && i + 2 < pfad.elementCount())
        {
            const QPainterPath::Element zwei = pfad.elementAt(i + 1);
            const QPainterPath::Element ende = pfad.elementAt(i + 2);
            teile << QStringLiteral("C") << zahl(e.x) << zahl(e.y) << zahl(zwei.x) << zahl(zwei.y) << zahl(ende.x) << zahl(ende.y);
            i += 2;
            letzter = QPointF(ende.x, ende.y);
            ++imZug;
            continue;
        }
        letzter = QPointF(e.x, e.y);
        ++imZug;
    }
    beende();
    return teile.join(QLatin1Char(' '));
}

bool pfadAusText(const QString& text, QPainterPath& pfad)
{
    static const QRegularExpression kLuecke(QStringLiteral("\\s+"));
    const QStringList teile = text.split(kLuecke, Qt::SkipEmptyParts);
    QPainterPath neu;
    bool begonnen = false;
    qsizetype i = 0;
    // `zahlen` Zahlen hinter dem Befehl an der Stelle i lesen
    const auto lies = [&](int zahlen, std::array<double, 6>& werte) {
        if (i + zahlen >= teile.size()) return false;
        for (int n = 0; n < zahlen; ++n)
        {
            bool ok = false;
            werte[static_cast<std::size_t>(n)] = teile.at(i + 1 + n).toDouble(&ok);
            if (!ok || !std::isfinite(werte[static_cast<std::size_t>(n)])) return false;
        }
        i += zahlen + 1;
        return true;
    };
    while (i < teile.size())
    {
        const QString& befehl = teile.at(i);
        std::array<double, 6> w{};
        if (befehl == QLatin1String("M"))
        {
            if (!lies(2, w)) return false;
            neu.moveTo(w[0], w[1]);
            begonnen = true;
        }
        else if (befehl == QLatin1String("L"))
        {
            if (!begonnen || !lies(2, w)) return false;
            neu.lineTo(w[0], w[1]);
        }
        else if (befehl == QLatin1String("C"))
        {
            if (!begonnen || !lies(6, w)) return false;
            neu.cubicTo(w[0], w[1], w[2], w[3], w[4], w[5]);
        }
        else if (befehl == QLatin1String("Z"))
        {
            if (!begonnen) return false;
            neu.closeSubpath();
            begonnen = false;  // der naechste Zug beginnt mit M
            ++i;
        }
        else
        {
            return false;
        }
    }
    if (neu.elementCount() < 2) return false;
    pfad = neu;
    return true;
}

// --- die Datei ---------------------------------------------------------------------

QJsonObject alsJson(const Zeichnung& zeichnung)
{
    QJsonObject o;
    o[QStringLiteral("format")] = kFormat;
    o[QStringLiteral("version")] = kKonturFassung;
    o[QStringLiteral("title")] = zeichnung.titel;
    o[QStringLiteral("useAs")] = wortZu(kNutzungen, static_cast<int>(zeichnung.nutzung));
    o[QStringLiteral("jumps")] = wortZu(kSprungArten, static_cast<int>(zeichnung.spruenge));
    o[QStringLiteral("nextId")] = zeichnung.naechsteKennung;
    QJsonArray konturen;
    for (const Kontur& kontur : zeichnung.konturen)
    {
        QJsonObject k;
        k[QStringLiteral("id")] = kontur.kennung;
        k[QStringLiteral("name")] = kontur.name;
        k[QStringLiteral("color")] = kontur.farbe;
        k[QStringLiteral("selected")] = kontur.gewaehlt;
        k[QStringLiteral("path")] = pfadAlsText(kontur.pfad);
        konturen.append(k);
    }
    o[QStringLiteral("contours")] = konturen;
    QJsonArray bruecken;
    for (const Bruecke& bruecke : zeichnung.bruecken)
    {
        QJsonObject b;
        b[QStringLiteral("from")] = bruecke.von;
        b[QStringLiteral("fromPoint")] = punktAlsJson(bruecke.vonPunkt);
        b[QStringLiteral("to")] = bruecke.nach;
        b[QStringLiteral("toPoint")] = punktAlsJson(bruecke.nachPunkt);
        bruecken.append(b);
    }
    o[QStringLiteral("bridges")] = bruecken;
    QJsonObject herkunft;
    herkunft[QStringLiteral("kind")] = wortZu(kHerkuenfte, static_cast<int>(zeichnung.herkunft.art));
    herkunft[QStringLiteral("file")] = zeichnung.herkunft.quelle;
    herkunft[QStringLiteral("settings")] = zeichnung.herkunft.einstellungen;
    o[QStringLiteral("source")] = herkunft;
    return o;
}

bool ausJson(const QJsonObject& wurzel, Zeichnung& zeichnung, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    const int fassung = wurzel.value(QStringLiteral("version")).toInt(1);
    if (fassung > kKonturFassung)
    {
        // eine Fassung von morgen: lieber nichts zeigen als etwas anderes
        if (zuNeu != nullptr) *zuNeu = true;
        return false;
    }
    static const QRegularExpression kFarbe(QStringLiteral("^#[0-9a-fA-F]{6}$"));
    Zeichnung neu;
    neu.name = zeichnung.name;
    neu.titel = wurzel.value(QStringLiteral("title")).toString();
    neu.nutzung = static_cast<Nutzung>(wertZu(kNutzungen, wurzel.value(QStringLiteral("useAs")).toString()));
    neu.spruenge = static_cast<Spruenge>(wertZu(kSprungArten, wurzel.value(QStringLiteral("jumps")).toString()));
    int groesste = 0;
    for (const QJsonValue& eintrag : wurzel.value(QStringLiteral("contours")).toArray())
    {
        if (neu.konturen.size() >= static_cast<std::size_t>(kKonturenMax)) break;
        const QJsonObject k = eintrag.toObject();
        Kontur kontur;
        kontur.kennung = k.value(QStringLiteral("id")).toInt(0);
        // ohne Kennung, mit einer doppelten oder ohne Pfad: keine Kontur
        if (kontur.kennung <= 0 || neu.finde(kontur.kennung) != nullptr) continue;
        if (!pfadAusText(k.value(QStringLiteral("path")).toString(), kontur.pfad)) continue;
        schliesse(kontur.pfad);
        kontur.name = k.value(QStringLiteral("name")).toString();
        const QString farbe = k.value(QStringLiteral("color")).toString();
        kontur.farbe = kFarbe.match(farbe).hasMatch() ? farbe.toLower() : konturFarbe(kontur.kennung);
        kontur.gewaehlt = k.value(QStringLiteral("selected")).toBool(true);
        groesste = std::max(groesste, kontur.kennung);
        neu.konturen.push_back(kontur);
    }
    // die naechste Kennung liegt ueber jeder, die es gibt — auch wenn die Datei weniger sagt
    neu.naechsteKennung = std::max(wurzel.value(QStringLiteral("nextId")).toInt(1), groesste + 1);
    for (const QJsonValue& eintrag : wurzel.value(QStringLiteral("bridges")).toArray())
    {
        const QJsonObject b = eintrag.toObject();
        Bruecke bruecke;
        bruecke.von = b.value(QStringLiteral("from")).toInt(0);
        bruecke.nach = b.value(QStringLiteral("to")).toInt(0);
        // nur zwischen zwei verschiedenen Konturen, die es gibt
        if (bruecke.von == bruecke.nach || neu.finde(bruecke.von) == nullptr || neu.finde(bruecke.nach) == nullptr) continue;
        if (!punktAusJson(b.value(QStringLiteral("fromPoint")), bruecke.vonPunkt) ||
            !punktAusJson(b.value(QStringLiteral("toPoint")), bruecke.nachPunkt))
            continue;
        neu.bruecken.push_back(bruecke);
    }
    const QJsonObject herkunft = wurzel.value(QStringLiteral("source")).toObject();
    neu.herkunft.art = static_cast<HerkunftArt>(wertZu(kHerkuenfte, herkunft.value(QStringLiteral("kind")).toString()));
    neu.herkunft.quelle = herkunft.value(QStringLiteral("file")).toString();
    neu.herkunft.einstellungen = herkunft.value(QStringLiteral("settings")).toObject();
    zeichnung = std::move(neu);
    return true;
}

bool speichereZeichnung(const Zeichnung& zeichnung, const QString& pfad)
{
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    return datei.write(QJsonDocument(alsJson(zeichnung)).toJson(QJsonDocument::Indented)) >= 0 && datei.commit();
}

bool ladeZeichnung(const QString& pfad, Zeichnung& zeichnung, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    const QJsonDocument dokument = QJsonDocument::fromJson(datei.readAll());
    if (!dokument.isObject()) return false;
    Zeichnung neu;
    if (!ausJson(dokument.object(), neu, zuNeu)) return false;
    neu.name = pfad;
    if (neu.titel.isEmpty()) neu.titel = QFileInfo(pfad).completeBaseName();
    zeichnung = std::move(neu);
    return true;
}

// --- wo sie liegt -------------------------------------------------------------------

QString konturOrdner()
{
    static const QString kOrdner = lumi::core::mitgeliefert::ordner(QStringLiteral("contours"));
    return kOrdner;
}

const std::vector<Zeichnung>& zeichnungen()
{
    static const std::vector<Zeichnung> kAlle = [] {
        std::vector<Zeichnung> alle;
        const QString ordner = konturOrdner();
        if (ordner.isEmpty()) return alle;
        const QString muster = QStringLiteral("*") + QLatin1String(kKonturEndung);
        for (const QString& datei : QDir(ordner).entryList({muster}, QDir::Files, QDir::Name))
        {
            Zeichnung zeichnung;
            if (!ladeZeichnung(QDir(ordner).filePath(datei), zeichnung)) continue;
            zeichnung.name = QFileInfo(datei).completeBaseName();
            alle.push_back(std::move(zeichnung));
        }
        std::stable_sort(alle.begin(), alle.end(),
                         [](const Zeichnung& a, const Zeichnung& b) { return a.titel.compare(b.titel, Qt::CaseInsensitive) < 0; });
        return alle;
    }();
    return kAlle;
}

bool istEigeneZeichnung(const QString& nennung)
{
    return nennung.startsWith(QLatin1String(kEigenVorsatz));
}

bool istKonturDatei(const QString& nennung)
{
    return !istEigeneZeichnung(nennung) && nennung.endsWith(QLatin1String(kKonturEndung), Qt::CaseInsensitive);
}

QString eigenerKonturOrdner()
{
    if (!konturOrdnerImTest().isEmpty()) return konturOrdnerImTest();
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return basis.isEmpty() ? QString() : QDir(basis).filePath(QStringLiteral("contours"));
}

void setzeEigenenKonturOrdnerFuerTests(const QString& ordner)
{
    konturOrdnerImTest() = ordner;
}

QString eigenerKonturName(const QString& titel)
{
    QString name;
    for (const QChar zeichen : titel.trimmed().toLower())
    {
        if ((zeichen >= QLatin1Char('a') && zeichen <= QLatin1Char('z')) ||
            (zeichen >= QLatin1Char('0') && zeichen <= QLatin1Char('9')))
            name.append(zeichen);
        else if (!name.isEmpty() && !name.endsWith(QLatin1Char('_')))
            name.append(QLatin1Char('_'));  // alles andere wird ein Trenner, nie zwei hintereinander
    }
    while (name.endsWith(QLatin1Char('_'))) name.chop(1);
    return name.left(64);
}

QString eigeneKonturDatei(const QString& name)
{
    const QString ordner = eigenerKonturOrdner();
    if (ordner.isEmpty() || name.isEmpty() || eigenerKonturName(name) != name) return {};
    return QDir(ordner).filePath(name + QLatin1String(kKonturEndung));
}

std::vector<Zeichnung> eigeneZeichnungen()
{
    std::vector<Zeichnung> alle;
    const QString ordner = eigenerKonturOrdner();
    if (ordner.isEmpty() || !QFileInfo(ordner).isDir()) return alle;
    const QString muster = QStringLiteral("*") + QLatin1String(kKonturEndung);
    for (const QString& datei : QDir(ordner).entryList({muster}, QDir::Files, QDir::Name))
    {
        const QString name = QFileInfo(datei).completeBaseName();
        if (name.isEmpty() || eigenerKonturName(name) != name) continue;  // ein Name, den keine Schicht nennen kann
        Zeichnung zeichnung;
        if (!ladeZeichnung(QDir(ordner).filePath(datei), zeichnung)) continue;
        zeichnung.name = QLatin1String(kEigenVorsatz) + name;
        alle.push_back(std::move(zeichnung));
    }
    std::stable_sort(alle.begin(), alle.end(),
                     [](const Zeichnung& a, const Zeichnung& b) { return a.titel.compare(b.titel, Qt::CaseInsensitive) < 0; });
    return alle;
}

bool speichereEigeneZeichnung(Zeichnung& zeichnung, const QString& name)
{
    const QString pfad = eigeneKonturDatei(name);
    if (pfad.isEmpty() || !QDir().mkpath(eigenerKonturOrdner())) return false;
    if (!speichereZeichnung(zeichnung, pfad)) return false;
    zeichnung.name = QLatin1String(kEigenVorsatz) + name;
    return true;
}

bool loescheEigeneZeichnung(const QString& name)
{
    const QString pfad = eigeneKonturDatei(name);
    return !pfad.isEmpty() && QFile::remove(pfad);
}

bool holeZeichnung(const QString& nennung, Zeichnung& zeichnung, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (istEigeneZeichnung(nennung))
    {
        const QString pfad = eigeneKonturDatei(nennung.mid(static_cast<int>(qstrlen(kEigenVorsatz))));
        Zeichnung eigene;
        if (pfad.isEmpty() || !ladeZeichnung(pfad, eigene, zuNeu)) return false;
        eigene.name = nennung;
        zeichnung = std::move(eigene);
        return true;
    }
    if (istKonturDatei(nennung)) return ladeZeichnung(nennung, zeichnung, zuNeu);
    for (const Zeichnung& mitgeliefert : zeichnungen())
    {
        if (mitgeliefert.name != nennung) continue;
        zeichnung = mitgeliefert;
        return true;
    }
    return false;
}

// --- die Rechnung -------------------------------------------------------------------

std::vector<Punkt> abtasten(const QPainterPath& pfad, int punkte)
{
    std::vector<Punkt> ergebnis;
    if (pfad.isEmpty() || punkte < 3) return ergebnis;
    const QRectF rahmen = pfad.boundingRect();
    const double ausmass = std::max(rahmen.width(), rahmen.height());
    if (!(ausmass > 0.0)) return ergebnis;
    // Qt zerlegt Kurven mit einer festen Toleranz in den Einheiten des Pfads — fuer eine
    // kleine Zeichnung waere das grob. Also gross machen, zerlegen, zurueckrechnen.
    const double gross = 8192.0 / ausmass;
    const QList<QPolygonF> zuege = pfad.toSubpathPolygons(QTransform::fromScale(gross, gross));
    if (zuege.isEmpty()) return ergebnis;
    QPolygonF zug = zuege.first();  // eine Kontur ist ein Zug
    if (zug.size() < 2) return ergebnis;
    if (zug.first() != zug.last()) zug.append(zug.first());

    std::vector<double> weg(static_cast<std::size_t>(zug.size()), 0.0);
    for (qsizetype i = 1; i < zug.size(); ++i)
    {
        const QPointF d = zug.at(i) - zug.at(i - 1);
        weg[static_cast<std::size_t>(i)] = weg[static_cast<std::size_t>(i - 1)] + std::hypot(d.x(), d.y());
    }
    const double laenge = weg.back();
    if (!(laenge > 0.0)) return ergebnis;
    ergebnis.reserve(static_cast<std::size_t>(punkte));
    std::size_t stueck = 1;
    for (int n = 0; n < punkte; ++n)
    {
        const double ziel = laenge * n / punkte;
        while (stueck + 1 < weg.size() && weg[stueck] < ziel) ++stueck;
        const double von = weg[stueck - 1];
        const double bis = weg[stueck];
        const double anteil = bis > von ? (ziel - von) / (bis - von) : 0.0;
        const QPointF a = zug.at(static_cast<qsizetype>(stueck - 1));
        const QPointF b = zug.at(static_cast<qsizetype>(stueck));
        const QPointF p = a + (b - a) * anteil;
        ergebnis.emplace_back(p.x() / gross, -p.y() / gross);  // y nach oben
    }
    return ergebnis;
}

std::vector<Punkt> normiert(const std::vector<Punkt>& punkte)
{
    std::vector<Punkt> ergebnis = punkte;
    if (ergebnis.empty()) return ergebnis;
    Punkt summe{0.0, 0.0};
    for (const Punkt& p : ergebnis) summe += p;
    const Punkt mitte = summe / static_cast<double>(ergebnis.size());
    double groesster = 0.0;
    for (Punkt& p : ergebnis)
    {
        p -= mitte;
        groesster = std::max({groesster, std::abs(p.real()), std::abs(p.imag())});
    }
    if (groesster > 0.0)
        for (Punkt& p : ergebnis) p /= groesster;
    return ergebnis;
}

Reihe reihe(const std::vector<Punkt>& punkte, int hoechste)
{
    Reihe ergebnis;
    const auto m = static_cast<std::int64_t>(punkte.size());
    if (m < 3) return ergebnis;
    Punkt summe{0.0, 0.0};
    for (const Punkt& p : punkte) summe += p;
    ergebnis.mitte = summe / static_cast<double>(m);
    // nicht mehr Harmonische, als die Punkte hergeben
    const int zahl = static_cast<int>(std::clamp<std::int64_t>(std::min<std::int64_t>(hoechste, (m - 1) / 2), 0, kHarmonischeMax));
    // e^(-2*pi*i*j/m) fuer jedes j einmal
    std::vector<Punkt> dreh(static_cast<std::size_t>(m));
    for (std::int64_t j = 0; j < m; ++j) dreh[static_cast<std::size_t>(j)] = std::polar(1.0, -2.0 * kPi * static_cast<double>(j) / static_cast<double>(m));
    ergebnis.plus.resize(static_cast<std::size_t>(zahl));
    ergebnis.minus.resize(static_cast<std::size_t>(zahl));
    for (int k = 1; k <= zahl; ++k)
    {
        Punkt vor{0.0, 0.0};
        Punkt zurueck{0.0, 0.0};
        std::int64_t j = 0;
        for (std::int64_t n = 0; n < m; ++n)
        {
            const Punkt& d = dreh[static_cast<std::size_t>(j)];
            vor += punkte[static_cast<std::size_t>(n)] * d;
            zurueck += punkte[static_cast<std::size_t>(n)] * std::conj(d);
            j += k;
            if (j >= m) j -= m;
        }
        ergebnis.plus[static_cast<std::size_t>(k - 1)] = vor / static_cast<double>(m);
        ergebnis.minus[static_cast<std::size_t>(k - 1)] = zurueck / static_cast<double>(m);
    }
    return ergebnis;
}

double gewicht(double harmonische, int k)
{
    return std::clamp(harmonische - static_cast<double>(k) + 1.0, 0.0, 1.0);
}

Punkt wert(const Reihe& reihe, double umlauf, double harmonische, bool mitMitte)
{
    Punkt summe = mitMitte ? reihe.mitte : Punkt{0.0, 0.0};
    const Punkt dreh = std::polar(1.0, 2.0 * kPi * umlauf);
    Punkt zeiger{1.0, 0.0};
    for (int k = 1; k <= reihe.harmonische(); ++k)
    {
        const double g = gewicht(harmonische, k);
        if (g <= 0.0) break;
        zeiger *= dreh;
        const auto platz = static_cast<std::size_t>(k - 1);
        summe += g * (reihe.plus[platz] * zeiger + reihe.minus[platz] * std::conj(zeiger));
    }
    return summe;
}

std::vector<Punkt> umriss(const Reihe& reihe, double harmonische, int punkte, bool mitMitte)
{
    std::vector<Punkt> ergebnis;
    if (punkte < 1) return ergebnis;
    ergebnis.reserve(static_cast<std::size_t>(punkte));
    for (int n = 0; n < punkte; ++n) ergebnis.push_back(wert(reihe, static_cast<double>(n) / punkte, harmonische, mitMitte));
    return ergebnis;
}

}  // namespace lumi::shape
