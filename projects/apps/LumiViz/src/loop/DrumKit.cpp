/**
 * @file DrumKit.cpp
 * @brief Loop: Kits als eigene Einheit (Stufe 21a, S123)
 */

#include "loop/DrumKit.hpp"

#include "core/Mitgeliefert.hpp"
#include "loop/Bearbeiten.hpp"  // musterIn
#include "loop/Instrument.hpp"  // istEigenerName, kEigenVorsatz

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QDirIterator>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace lumi::loop
{

namespace
{

const QString kFormat = QStringLiteral("lumiviz-kit");

double inGrenzen(double wert, double von, double bis, double ersatz)
{
    return std::isfinite(wert) ? std::clamp(wert, von, bis) : ersatz;
}

/// Die Woerter eines Namens, klein: „drum_hat_open" → drum, hat, open; „Hi-Hat" → hi, hat.
QStringList woerter(const QString& text)
{
    static const QRegularExpression trenner(QStringLiteral("[^a-z0-9]+"));
    return text.toLower().split(trenner, Qt::SkipEmptyParts);
}

bool hatEines(const QStringList& w, std::initializer_list<const char*> liste)
{
    return std::any_of(liste.begin(), liste.end(), [&w](const char* wort) { return w.contains(QLatin1String(wort)); });
}

/// Die Art aus den Woertern eines Namens; leer = keine.
QString artAus(const QStringList& w)
{
    const bool hat = hatEines(w, {"hat", "hh", "hihat", "chh", "ohh", "openhat"});
    if ((hat && w.contains(QStringLiteral("open"))) || hatEines(w, {"ohh", "openhat"})) return QStringLiteral("open hat");
    if (hat) return QStringLiteral("hat");
    if (hatEines(w, {"kick", "bd", "bassdrum", "boom"})) return QStringLiteral("kick");
    if (hatEines(w, {"snare", "sn"})) return QStringLiteral("snare");
    if (hatEines(w, {"clap", "snap", "claps"})) return QStringLiteral("clap");
    if (hatEines(w, {"rim", "rimshot", "stick", "sidestick"})) return QStringLiteral("rim");
    if (hatEines(w, {"ride"})) return QStringLiteral("ride");
    if (hatEines(w, {"crash", "china", "splash", "cymbal"})) return QStringLiteral("crash");
    if (hatEines(w, {"tom", "toms"})) return QStringLiteral("tom");
    if (hatEines(w, {"shaker", "shake", "tambourine", "conga", "bongo", "djembe", "darbuka", "cowbell", "clave",
                     "perc", "percussion", "agogo", "guiro", "timbales", "tabla"}))
        return QStringLiteral("perc");
    if (hatEines(w, {"hey", "fx", "voice", "vox", "gliss", "laser"})) return QStringLiteral("fx");
    return {};
}

/// Der Name hinter einer Nennung: ohne Vorsatz, ohne Ordner und Endung.
QString kern(const QString& nennung)
{
    if (nennung.isEmpty()) return {};
    const int doppelpunkt = static_cast<int>(nennung.indexOf(QLatin1Char(':')));
    if (doppelpunkt > 1 && !nennung.contains(QLatin1Char('/')) && !nennung.contains(QLatin1Char('\\')))
        return nennung.mid(doppelpunkt + 1);  // kit:, lib:, user:
    return QFileInfo(nennung).completeBaseName();
}

/// Tief vor hoch: low und floor zuerst, high zuletzt (fuer die Nummern der Toms).
int lage(const Spur& spur)
{
    QStringList w = woerter(spur.name);
    w << woerter(kern(spur.instrument)) << woerter(kern(spur.klang));
    if (hatEines(w, {"low", "floor", "tief"})) return 0;
    if (hatEines(w, {"high", "hi", "hoch"})) return 2;
    return 1;
}

QString fuerDateiEintrag(const QString& klang, const QString& basis)
{
    return istMitgelieferterKlang(klang) || istEigenerKlang(klang) || klang.isEmpty() ? klang : pfadFuerDatei(klang, basis);
}

QString ausDateiEintrag(const QString& klang, const QString& basis)
{
    return istMitgelieferterKlang(klang) || istEigenerKlang(klang) || klang.isEmpty() ? klang : pfadAusDatei(klang, basis);
}

QString& eigenerOrdnerImTest()
{
    static QString ordner;
    return ordner;
}

std::vector<DrumKit> leseOrdner(const QString& ordner, const QString& vorsatz)
{
    std::vector<DrumKit> alle;
    if (ordner.isEmpty() || !QFileInfo(ordner).isDir()) return alle;
    const QStringList dateien = QDir(ordner).entryList({QStringLiteral("*.lvkit")}, QDir::Files, QDir::Name);
    for (const QString& datei : dateien)
    {
        const QString name = QFileInfo(datei).completeBaseName();
        if (!istEigenerName(name)) continue;  // ein Name, den kein Muster nennen kann
        DrumKit kit;
        if (!ladeKit(QDir(ordner).filePath(datei), kit)) continue;
        kit.name = vorsatz + name;
        alle.push_back(std::move(kit));
    }
    std::stable_sort(alle.begin(), alle.end(),
                     [](const DrumKit& a, const DrumKit& b) { return a.titel.compare(b.titel, Qt::CaseInsensitive) < 0; });
    return alle;
}

}  // namespace

// --- Rollen ------------------------------------------------------------------

const QStringList& rollenArten()
{
    static const QStringList kArten = {QStringLiteral("kick"),     QStringLiteral("snare"), QStringLiteral("clap"),
                                       QStringLiteral("rim"),      QStringLiteral("hat"),   QStringLiteral("open hat"),
                                       QStringLiteral("ride"),     QStringLiteral("crash"), QStringLiteral("tom"),
                                       QStringLiteral("perc"),     QStringLiteral("fx")};
    return kArten;
}

std::optional<Rolle> leseRolle(const QString& text)
{
    QStringList teile = text.trimmed().toLower().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (teile.isEmpty()) return std::nullopt;
    Rolle rolle;
    bool zahl = false;
    const int nummer = teile.last().toInt(&zahl);
    if (zahl)
    {
        if (nummer < 1 || nummer > 99 || teile.size() < 2) return std::nullopt;
        rolle.nummer = nummer;
        teile.removeLast();
    }
    rolle.art = teile.join(QLatin1Char(' '));
    if (!rollenArten().contains(rolle.art)) return std::nullopt;
    return rolle;
}

QString rollenText(const Rolle& rolle)
{
    const bool immerMitNummer = rolle.art == QLatin1String("tom") || rolle.art == QLatin1String("perc");
    return immerMitNummer || rolle.nummer > 1 ? QStringLiteral("%1 %2").arg(rolle.art).arg(rolle.nummer) : rolle.art;
}

QString rateRollenArt(const Spur& spur)
{
    for (const QString& quelle : {kern(spur.instrument), kern(spur.klang), spur.name})
        if (const QString art = artAus(woerter(quelle)); !art.isEmpty()) return art;
    return {};
}

void rateRollen(Muster& muster)
{
    std::map<QString, std::set<int>> vergeben;
    std::map<QString, std::vector<std::size_t>> ohne;  // je Art die Spuren ohne gueltige Rolle
    for (std::size_t i = 0; i < muster.spuren.size(); ++i)
    {
        const Spur& spur = muster.spuren[i];
        if (const std::optional<Rolle> rolle = leseRolle(spur.rolle))
            vergeben[rolle->art].insert(rolle->nummer);
        else if (const QString art = rateRollenArt(spur); !art.isEmpty())
            ohne[art].push_back(i);
    }
    for (auto& [art, spuren] : ohne)
    {
        if (art == QLatin1String("tom"))
            std::stable_sort(spuren.begin(), spuren.end(),
                             [&muster](std::size_t a, std::size_t b) { return lage(muster.spuren[a]) < lage(muster.spuren[b]); });
        std::set<int>& belegt = vergeben[art];
        for (const std::size_t i : spuren)
        {
            int nummer = 1;
            while (belegt.count(nummer) != 0) ++nummer;
            belegt.insert(nummer);
            muster.spuren[i].rolle = rollenText({art, nummer});
        }
    }
}

// --- Das Kit -----------------------------------------------------------------

const KitKlang* DrumKit::fuer(const Rolle& rolle) const
{
    const KitKlang* hoechste = nullptr;
    int hoechsteNummer = 0;
    for (const KitKlang& klang : klaenge)
    {
        const std::optional<Rolle> eigene = leseRolle(klang.rolle);
        if (!eigene || eigene->art != rolle.art) continue;
        if (eigene->nummer == rolle.nummer) return &klang;
        if (eigene->nummer > hoechsteNummer)
        {
            hoechsteNummer = eigene->nummer;
            hoechste = &klang;
        }
    }
    return hoechste;
}

QJsonObject alsJson(const DrumKit& kit, const QString& basisOrdner)
{
    QJsonObject o;
    o[QStringLiteral("format")] = kFormat;
    o[QStringLiteral("version")] = kKitFassung;
    o[QStringLiteral("title")] = kit.titel;
    o[QStringLiteral("description")] = kit.beschreibung;
    QJsonArray rollen;
    for (const KitKlang& klang : kit.klaenge)
    {
        QJsonObject r;
        r[QStringLiteral("role")] = klang.rolle;
        r[QStringLiteral("sound")] = fuerDateiEintrag(klang.klang, basisOrdner);
        if (!klang.instrument.isEmpty())
        {
            r[QStringLiteral("instrument")] =
                istInstrumentDatei(klang.instrument) ? pfadFuerDatei(klang.instrument, basisOrdner) : klang.instrument;
            r[QStringLiteral("note")] = klang.hoehe;
            r[QStringLiteral("length")] = klang.dauerMs;
        }
        r[QStringLiteral("volume")] = klang.laut;
        r[QStringLiteral("pan")] = klang.pan;
        rollen.append(r);
    }
    o[QStringLiteral("roles")] = rollen;
    return o;
}

bool ausJson(const QJsonObject& wurzel, DrumKit& kit, const QString& basisOrdner, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    if (wurzel.value(QStringLiteral("version")).toInt(1) > kKitFassung)
    {
        if (zuNeu != nullptr) *zuNeu = true;  // eine Fassung von morgen: lieber nichts als etwas Falsches
        return false;
    }
    DrumKit neu;
    neu.name = kit.name;
    neu.titel = wurzel.value(QStringLiteral("title")).toString().trimmed();
    neu.beschreibung = wurzel.value(QStringLiteral("description")).toString().trimmed();
    QStringList gesehen;
    for (const QJsonValue& wert : wurzel.value(QStringLiteral("roles")).toArray())
    {
        const QJsonObject r = wert.toObject();
        const std::optional<Rolle> rolle = leseRolle(r.value(QStringLiteral("role")).toString());
        if (!rolle) continue;  // eine Rolle, die dieses Programm nicht kennt
        KitKlang klang;
        klang.rolle = rollenText(*rolle);
        if (gesehen.contains(klang.rolle)) continue;  // doppelt: die erste gilt
        gesehen << klang.rolle;
        klang.klang = ausDateiEintrag(r.value(QStringLiteral("sound")).toString(), basisOrdner);
        klang.instrument = r.value(QStringLiteral("instrument")).toString();
        if (istInstrumentDatei(klang.instrument)) klang.instrument = pfadAusDatei(klang.instrument, basisOrdner);
        klang.hoehe = std::clamp(r.value(QStringLiteral("note")).toInt(klang.hoehe), 0, 127);
        klang.dauerMs = std::clamp(r.value(QStringLiteral("length")).toInt(klang.dauerMs), kSpurDauerMinMs, kSpurDauerMaxMs);
        klang.laut = inGrenzen(r.value(QStringLiteral("volume")).toDouble(1.0), 0.0, 1.0, 1.0);
        klang.pan = inGrenzen(r.value(QStringLiteral("pan")).toDouble(0.0), -1.0, 1.0, 0.0);
        neu.klaenge.push_back(std::move(klang));
    }
    kit = std::move(neu);
    return true;
}

bool ladeKit(const QString& pfad, DrumKit& kit, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    DrumKit neu;
    if (!ausJson(doc.object(), neu, QFileInfo(pfad).absolutePath(), zuNeu)) return false;
    neu.name = pfad;
    if (neu.titel.isEmpty()) neu.titel = QFileInfo(pfad).completeBaseName();
    kit = std::move(neu);
    return true;
}

bool speichereKit(const QString& pfad, const DrumKit& kit)
{
    if (!QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    const QJsonDocument doc(alsJson(kit, QFileInfo(pfad).absolutePath()));
    return datei.write(doc.toJson(QJsonDocument::Indented)) >= 0 && datei.commit();
}

// --- Ablage ------------------------------------------------------------------

QString mitgelieferterKitOrdner()
{
    const QString loop = lumi::core::mitgeliefert::ordner(QStringLiteral("loop"));
    if (loop.isEmpty()) return {};
    const QString ordner = loop + QStringLiteral("/kits");
    return QFileInfo(ordner).isDir() ? ordner : QString();
}

const std::vector<DrumKit>& mitgelieferteKits()
{
    // einmal gelesen und gemerkt — aber nur, wenn der Ordner da war
    static std::vector<DrumKit> alle;
    if (alle.empty()) alle = leseOrdner(mitgelieferterKitOrdner(), QString());
    return alle;
}

QString eigenerKitOrdner()
{
    if (!eigenerOrdnerImTest().isEmpty()) return eigenerOrdnerImTest();
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return basis.isEmpty() ? QString() : QDir(basis).filePath(QStringLiteral("kits"));
}

void setzeEigenenKitOrdnerFuerTests(const QString& ordner)
{
    eigenerOrdnerImTest() = ordner;
}

QString eigeneKitDatei(const QString& name)
{
    const QString ordner = eigenerKitOrdner();
    if (ordner.isEmpty() || !istEigenerName(name)) return {};
    return QDir(ordner).filePath(name + QLatin1String(kKitDateiEndung));
}

std::vector<DrumKit> eigeneKits()
{
    return leseOrdner(eigenerKitOrdner(), QLatin1String(kEigenVorsatz));
}

bool holeKit(const QString& nennung, DrumKit& kit)
{
    if (nennung.startsWith(QLatin1String(kEigenVorsatz)))
    {
        const QString datei = eigeneKitDatei(nennung.mid(static_cast<int>(qstrlen(kEigenVorsatz))));
        DrumKit eigenes;
        if (datei.isEmpty() || !ladeKit(datei, eigenes)) return false;
        eigenes.name = nennung;
        kit = std::move(eigenes);
        return true;
    }
    if (nennung.endsWith(QLatin1String(kKitDateiEndung), Qt::CaseInsensitive) || QFileInfo(nennung).isAbsolute())
        return ladeKit(nennung, kit);
    const std::vector<DrumKit>& alle = mitgelieferteKits();
    const auto it = std::find_if(alle.begin(), alle.end(), [&nennung](const DrumKit& k) { return k.name == nennung; });
    if (it == alle.end()) return false;
    kit = *it;
    return true;
}

// --- Kitwechsel --------------------------------------------------------------

KitWechsel wendeKitAn(Muster& muster, const DrumKit& kit)
{
    KitWechsel wechsel;
    rateRollen(muster);
    QStringList belegt;
    for (Spur& spur : muster.spuren)
    {
        const std::optional<Rolle> rolle = leseRolle(spur.rolle);
        if (!rolle) continue;  // nicht zu erraten: die Spur bleibt, wie sie ist
        belegt << rollenText(*rolle);
        const KitKlang* klang = kit.fuer(*rolle);
        if (klang == nullptr)
        {
            wechsel.ohneGegenstueck << spur.rolle;
            continue;
        }
        if (!klang->klang.isEmpty()) spur.klang = klang->klang;  // neben einem Instrument bleibt der alte stehen, wenn das Kit keinen nennt
        spur.instrument = klang->instrument;
        if (!klang->instrument.isEmpty())
        {
            spur.hoehe = klang->hoehe;
            spur.dauerMs = klang->dauerMs;
        }
        spur.laut = klang->laut;
        spur.pan = klang->pan;
        ++wechsel.getroffen;
    }
    for (const KitKlang& klang : kit.klaenge)
        if (!belegt.contains(klang.rolle)) wechsel.fehlend << klang.rolle;
    muster.kit = kit.name;
    return wechsel;
}

Spur spurAusKitKlang(const KitKlang& klang)
{
    Spur spur;
    spur.name = klang.rolle.isEmpty() ? klang.rolle : klang.rolle.left(1).toUpper() + klang.rolle.mid(1);
    spur.rolle = klang.rolle;
    spur.klang = klang.klang;
    spur.instrument = klang.instrument;
    if (!klang.instrument.isEmpty())
    {
        spur.hoehe = klang.hoehe;
        spur.dauerMs = klang.dauerMs;
    }
    spur.laut = klang.laut;
    spur.pan = klang.pan;
    return spur;
}

QString freierSpurName(const Muster& muster, const QString& rolle)
{
    const QString basis = rolle.isEmpty() ? rolle : rolle.left(1).toUpper() + rolle.mid(1);
    const auto vergeben = [&muster](const QString& name) {
        return std::any_of(muster.spuren.begin(), muster.spuren.end(), [&name](const Spur& s) { return s.name == name; });
    };
    QString name = basis;
    for (int n = 2; vergeben(name); ++n) name = basis + QStringLiteral(" (%1)").arg(n);
    return name;
}

KitWechsel wendeKitAn(Loop& loop, const QString& muster, const DrumKit& kit, const KitOptionen& optionen)
{
    Muster* ziel = musterIn(loop, muster);
    if (ziel == nullptr) return {};
    KitWechsel wechsel = wendeKitAn(*ziel, kit);
    ziel->zeilen.resize(ziel->spuren.size());  // je Spur eine Zeile, auch in einer unordentlichen Datei
    // der Platz einer Rolle im Kit; was das Kit nicht kennt, steht hinten
    const auto platzImKit = [&kit](const QString& rolle) {
        for (std::size_t i = 0; i < kit.klaenge.size(); ++i)
            if (kit.klaenge[i].rolle == rolle) return static_cast<int>(i);
        return static_cast<int>(kit.klaenge.size());
    };
    if (optionen.leereEntfernen)
        for (int s = static_cast<int>(ziel->spuren.size()) - 1; s >= 0; --s)
        {
            const Spur& spur = ziel->spuren[static_cast<std::size_t>(s)];
            const std::optional<Rolle> rolle = leseRolle(spur.rolle);
            if (!rolle || kit.fuer(*rolle) != nullptr) continue;  // ohne Rolle oder im Kit: bleibt
            const std::vector<double>& zeile = ziel->zeilen[static_cast<std::size_t>(s)];
            if (std::any_of(zeile.begin(), zeile.end(), [](double w) { return w > 0.0; })) continue;  // traegt Schritte
            wechsel.ohneGegenstueck.removeOne(spur.rolle);
            wechsel.entfernt.prepend(spur.name);
            ziel->spuren.erase(ziel->spuren.begin() + s);
            ziel->zeilen.erase(ziel->zeilen.begin() + s);
        }
    if (optionen.alleRollen)
    {
        const int schritte = ziel->takte * loop.schritteJeTakt();
        for (const QString& rolle : std::as_const(wechsel.fehlend))
        {
            const auto klang = std::find_if(kit.klaenge.begin(), kit.klaenge.end(), [&rolle](const KitKlang& k) { return k.rolle == rolle; });
            if (klang == kit.klaenge.end()) continue;
            Spur neu = spurAusKitKlang(*klang);
            neu.name = freierSpurName(*ziel, rolle);
            const int eigener = platzImKit(rolle);
            const auto vor = std::find_if(ziel->spuren.begin(), ziel->spuren.end(),
                                          [&](const Spur& s) { return platzImKit(s.rolle) > eigener; });
            const auto platz = vor - ziel->spuren.begin();
            ziel->spuren.insert(vor, neu);
            ziel->zeilen.insert(ziel->zeilen.begin() + platz, std::vector<double>(static_cast<std::size_t>(schritte), 0.0));
            wechsel.angelegt << rolle;
        }
        wechsel.fehlend.clear();
    }
    return wechsel;
}

bool weichtAb(const Muster& muster, const DrumKit& kit)
{
    for (const Spur& spur : muster.spuren)
    {
        const std::optional<Rolle> rolle = leseRolle(spur.rolle);
        if (!rolle) continue;
        const KitKlang* klang = kit.fuer(*rolle);
        if (klang == nullptr) continue;
        if (spur.instrument != klang->instrument) return true;
        if (klang->instrument.isEmpty() ? spur.klang != klang->klang
                                         : (spur.hoehe != klang->hoehe || spur.dauerMs != klang->dauerMs))
            return true;
        if (std::abs(spur.laut - klang->laut) > 1e-9 || std::abs(spur.pan - klang->pan) > 1e-9) return true;
    }
    return false;
}

DrumKit kitAusMuster(const Muster& muster, const QString& titel)
{
    Muster kopie = muster;
    rateRollen(kopie);
    DrumKit kit;
    kit.titel = titel;
    for (const Spur& spur : kopie.spuren)
    {
        const std::optional<Rolle> rolle = leseRolle(spur.rolle);
        if (!rolle) continue;
        const QString text = rollenText(*rolle);
        if (std::any_of(kit.klaenge.begin(), kit.klaenge.end(), [&text](const KitKlang& k) { return k.rolle == text; }))
            continue;  // zwei Spuren mit derselben Rolle: die erste gilt
        KitKlang klang;
        klang.rolle = text;
        klang.klang = spur.klang;
        klang.instrument = spur.instrument;
        if (!spur.instrument.isEmpty())
        {
            klang.hoehe = spur.hoehe;
            klang.dauerMs = spur.dauerMs;
        }
        klang.laut = spur.laut;
        klang.pan = spur.pan;
        kit.klaenge.push_back(std::move(klang));
    }
    return kit;
}

// --- Wer ein Kit nennt -------------------------------------------------------

namespace
{
const char* const kZuletztLoops = "loop/recentFiles";
}  // namespace

void merkeGeoeffnetenLoop(const QString& pfad)
{
    if (pfad.isEmpty()) return;
    const QString sauber = QFileInfo(pfad).absoluteFilePath();
    QSettings einstellungen;
    QStringList liste = einstellungen.value(QLatin1String(kZuletztLoops)).toStringList();
    liste.removeAll(sauber);
    liste.prepend(sauber);
    while (liste.size() > kZuletztLoopsMax) liste.removeLast();
    einstellungen.setValue(QLatin1String(kZuletztLoops), liste);
}

QStringList zuletztGeoeffneteLoops()
{
    QStringList liste;
    for (const QString& pfad : QSettings().value(QLatin1String(kZuletztLoops)).toStringList())
        if (QFileInfo::exists(pfad)) liste << pfad;
    return liste;
}

QStringList mitgelieferteLoopDateien()
{
    const QString ordner = lumi::core::mitgeliefert::ordner(QStringLiteral("loop"));
    if (ordner.isEmpty()) return {};
    QStringList dateien;
    QDirIterator suche(ordner, {QStringLiteral("*.lvloop")}, QDir::Files, QDirIterator::Subdirectories);
    while (suche.hasNext()) dateien << suche.next();
    dateien.sort(Qt::CaseInsensitive);
    return dateien;
}

bool nenntKit(const Loop& loop, const QString& nennung)
{
    return !nennung.isEmpty() && std::any_of(loop.muster.begin(), loop.muster.end(), [&nennung](const Muster& m) {
               return m.art == MusterArt::Drumkit && m.kit == nennung;
           });
}

QStringList loopsMitKit(const QString& nennung, const QStringList& dateien)
{
    QStringList namen;
    QStringList gesehen;
    for (const QString& datei : dateien)
    {
        const QString sauber = QFileInfo(datei).absoluteFilePath();
        if (gesehen.contains(sauber, Qt::CaseInsensitive)) continue;
        gesehen << sauber;
        Loop loop;
        if (ladeLoop(datei, loop) && nenntKit(loop, nennung)) namen << QFileInfo(datei).completeBaseName();
    }
    return namen;
}

}  // namespace lumi::loop
