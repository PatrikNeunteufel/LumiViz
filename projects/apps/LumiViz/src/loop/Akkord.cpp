/**
 * @file Akkord.cpp
 * @brief Loop: Akkorde — Griffbild oder Voicing, die Datei `.lvchord` (Stufe 26a, S137)
 */

#include "loop/Akkord.hpp"

#include "core/Mitgeliefert.hpp"
#include "loop/Instrument.hpp"  // istEigenerName, kEigenVorsatz

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>
#include <mutex>
#include <optional>

namespace lumi::loop
{

namespace
{

const QString kFormat = QStringLiteral("lumiviz-chord");

QString& eigenerOrdnerImTest()
{
    static QString ordner;
    return ordner;
}

QJsonArray alsFeld(const std::vector<int>& werte)
{
    QJsonArray feld;
    for (const int wert : werte) feld.append(wert);
    return feld;
}

std::vector<int> ausFeld(const QJsonValue& wert, int ersatz)
{
    std::vector<int> werte;
    for (const QJsonValue& stueck : wert.toArray()) werte.push_back(stueck.toInt(ersatz));
    return werte;
}

void ordneNachTitel(std::vector<Akkord>& alle)
{
    std::stable_sort(alle.begin(), alle.end(), [](const Akkord& a, const Akkord& b) {
        const int titel = a.titel.compare(b.titel, Qt::CaseInsensitive);
        return titel != 0 ? titel < 0 : a.name < b.name;  // gleiche Titel (Lagen): nach Nennung
    });
}

/// Die Akkorde eines Ordners (ohne Unterordner), genannt `vorsatz` + Dateiname.
std::vector<Akkord> leseOrdner(const QString& ordner, const QString& vorsatz)
{
    std::vector<Akkord> alle;
    if (ordner.isEmpty() || !QFileInfo(ordner).isDir()) return alle;
    QDirIterator lauf(ordner, {QStringLiteral("*") + QLatin1String(kAkkordDateiEndung)}, QDir::Files);
    while (lauf.hasNext())
    {
        const QString pfad = lauf.next();
        const QString name = QFileInfo(pfad).completeBaseName();
        if (!istEigenerName(name)) continue;  // ein Name, den niemand nennen kann
        Akkord akkord;
        if (!ladeAkkord(pfad, akkord)) continue;
        akkord.name = vorsatz + name;
        alle.push_back(std::move(akkord));
    }
    ordneNachTitel(alle);
    return alle;
}

const QString kSammlungFormat = QStringLiteral("lumiviz-chord-collection");

/// Der Kopf einer Sammlung: Format, Fassung, Beschreibung, Stimmung — die Akkorde bleiben.
bool liesSammlungKopf(const QJsonObject& wurzel, AkkordSammlung& sammlung, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (wurzel.value(QStringLiteral("format")).toString() != kSammlungFormat) return false;
    if (wurzel.value(QStringLiteral("version")).toInt(1) > kAkkordSammlungFassung)
    {
        if (zuNeu != nullptr) *zuNeu = true;
        return false;
    }
    sammlung.beschreibung = wurzel.value(QStringLiteral("description")).toString().trimmed();
    sammlung.leer = ausFeld(wurzel.value(QStringLiteral("open")), 0);
    sammlung.akkorde.clear();
    return true;
}

/// Die Akkorde unter `chords` in die Sammlung, deren Kopf schon gelesen ist.
void liesSammlungAkkorde(const QJsonArray& feld, AkkordSammlung& sammlung)
{
    const QString vorsatz = sammlung.name + QLatin1Char('/');
    QSet<QString> genannt;
    for (const Akkord& a : sammlung.akkorde) genannt.insert(a.name);
    for (const QJsonValue& wert : feld)
    {
        const QJsonObject o = wert.toObject();
        const QString name = o.value(QStringLiteral("name")).toString();
        if (!istEigenerName(name)) continue;  // ein Name, den niemand nennen kann
        Akkord akkord;
        if (!ausJson(o, akkord)) continue;
        // die Stimmung der Sammlung gilt fuer jedes Griffbild; ohne Stimmung nur Voicings
        if (akkord.leer != sammlung.leer) continue;
        akkord.name = vorsatz + name;
        if (akkord.titel.isEmpty()) akkord.titel = name;
        if (genannt.contains(akkord.name)) continue;  // doppelt: der erste gilt
        genannt.insert(akkord.name);
        sammlung.akkorde.push_back(std::move(akkord));
    }
}

/// Die mitgelieferten Sammlungen: beim ersten Bedarf jede Datei gelesen und ihr Kopf
/// gemerkt (sechs Dateien), die Akkorde einer Sammlung erst, wenn sie gebraucht wird.
class MitgelieferteSammlungen
{
public:
    static MitgelieferteSammlungen& ablage()
    {
        static MitgelieferteSammlungen ablage;
        return ablage;
    }

    /// Die Akkorde der Sammlungen, fuer die `waehle(kopf)` gilt, nach Titel.
    template <typename Wahl>
    std::vector<Akkord> akkorde(Wahl waehle)
    {
        const std::lock_guard sperre(m_sperre);
        liesKoepfe();
        std::vector<Akkord> alle;
        for (Eintrag& e : m_eintraege)
        {
            if (!waehle(e.sammlung)) continue;
            liesAkkorde(e);
            alle.insert(alle.end(), e.sammlung.akkorde.begin(), e.sammlung.akkorde.end());
        }
        ordneNachTitel(alle);
        return alle;
    }

    /// Einen Akkord bei seiner Nennung `<sammlung>/<name>`; liest nur diese Sammlung.
    std::optional<Akkord> finde(const QString& nennung)
    {
        const QString sammlung = nennung.section(QLatin1Char('/'), 0, 0);
        const std::lock_guard sperre(m_sperre);
        liesKoepfe();
        for (Eintrag& e : m_eintraege)
        {
            if (e.sammlung.name != sammlung) continue;
            liesAkkorde(e);
            for (const Akkord& a : e.sammlung.akkorde)
                if (a.name == nennung) return a;
        }
        return std::nullopt;
    }

private:
    struct Eintrag
    {
        AkkordSammlung sammlung;  ///< Kopf; die Akkorde nach `liesAkkorde`
        QJsonArray roh;           ///< `chords`, bis die Akkorde gelesen sind
        bool gelesen = false;
    };

    void liesKoepfe()
    {
        // nur, wenn der Ordner da war — sonst beim naechsten Mal noch einmal
        if (!m_eintraege.empty()) return;
        const QString ordner = mitgelieferterAkkordOrdner();
        if (ordner.isEmpty()) return;
        const QDir basis(ordner);
        for (const QString& datei :
             basis.entryList({QStringLiteral("*") + QLatin1String(kAkkordSammlungEndung)}, QDir::Files, QDir::Name))
        {
            QFile f(basis.filePath(datei));
            if (!f.open(QIODevice::ReadOnly)) continue;
            QJsonParseError fehler;
            const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &fehler);
            if (fehler.error != QJsonParseError::NoError || !doc.isObject()) continue;
            Eintrag e;
            e.sammlung.name = QFileInfo(datei).completeBaseName();
            if (!istEigenerName(e.sammlung.name) || !liesSammlungKopf(doc.object(), e.sammlung, nullptr)) continue;
            e.roh = doc.object().value(QStringLiteral("chords")).toArray();
            m_eintraege.push_back(std::move(e));
        }
    }

    static void liesAkkorde(Eintrag& e)
    {
        if (e.gelesen) return;
        liesSammlungAkkorde(e.roh, e.sammlung);
        e.roh = QJsonArray();
        e.gelesen = true;
    }

    std::mutex m_sperre;
    std::vector<Eintrag> m_eintraege;
};

}  // namespace

const QStringList& grundtonNamen()
{
    static const QStringList kNamen = {QStringLiteral("C"),  QStringLiteral("C#"), QStringLiteral("D"),  QStringLiteral("Eb"),
                                       QStringLiteral("E"),  QStringLiteral("F"),  QStringLiteral("F#"), QStringLiteral("G"),
                                       QStringLiteral("Ab"), QStringLiteral("A"),  QStringLiteral("Bb"), QStringLiteral("B")};
    return kNamen;
}

std::optional<int> grundtonAusName(const QString& name)
{
    const QString text = name.trimmed();
    if (text.isEmpty()) return std::nullopt;
    static const QString kStufen = QStringLiteral("CDEFGAB");
    static constexpr int kHalbtoene[] = {0, 2, 4, 5, 7, 9, 11};
    const qsizetype stufe = kStufen.indexOf(text.front().toUpper());
    if (stufe < 0) return std::nullopt;
    int ton = kHalbtoene[stufe];
    const QString rest = text.mid(1);
    if (rest == QLatin1String("#"))
        ++ton;
    else if (rest == QLatin1String("b"))
        --ton;
    else if (!rest.isEmpty())
        return std::nullopt;
    return (ton + 12) % 12;
}

void richteAkkord(Akkord& akkord)
{
    akkord.grundton = ((akkord.grundton % 12) + 12) % 12;
    if (akkord.istGriffbild())
    {
        if (akkord.leer.size() > static_cast<std::size_t>(kSaitenMax)) akkord.leer.resize(static_cast<std::size_t>(kSaitenMax));
        for (int& ton : akkord.leer) ton = std::clamp(ton, 0, 127);
        akkord.buende.resize(akkord.leer.size(), -1);  // fehlt ein Bund: die Saite spielt nicht
        akkord.finger.resize(akkord.leer.size(), 0);
        for (int& bund : akkord.buende) bund = std::clamp(bund, -1, kBuendeMax);
        for (int& finger : akkord.finger) finger = std::clamp(finger, 0, 4);
        std::erase_if(akkord.barre, [](int bund) { return bund < 1 || bund > kBuendeMax; });
        std::sort(akkord.barre.begin(), akkord.barre.end());
        akkord.barre.erase(std::unique(akkord.barre.begin(), akkord.barre.end()), akkord.barre.end());
        akkord.abstaende.clear();
        return;
    }
    akkord.buende.clear();
    akkord.finger.clear();
    akkord.barre.clear();
    std::vector<int>& a = akkord.abstaende;
    std::erase_if(a, [](int stufe) { return stufe < 0 || stufe > kVoicingWeiteMax; });
    std::sort(a.begin(), a.end());
    a.erase(std::unique(a.begin(), a.end()), a.end());
    // der tiefste Ton ist der Bezug: alles rueckt auf ihn
    if (!a.empty() && a.front() != 0)
        for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) a[static_cast<std::size_t>(i)] -= a.front();
    if (a.size() > static_cast<std::size_t>(kVoicingToeneMax)) a.resize(static_cast<std::size_t>(kVoicingToeneMax));
}

QJsonObject alsJson(const Akkord& akkord)
{
    QJsonObject o;
    o[QStringLiteral("format")] = kFormat;
    o[QStringLiteral("version")] = kAkkordFassung;
    o[QStringLiteral("title")] = akkord.titel;
    o[QStringLiteral("root")] = grundtonNamen().at(((akkord.grundton % 12) + 12) % 12);
    o[QStringLiteral("type")] = akkord.art;
    if (!akkord.herkunft.isEmpty()) o[QStringLiteral("origin")] = akkord.herkunft;
    o[QStringLiteral("description")] = akkord.beschreibung;
    if (akkord.istGriffbild())
    {
        QJsonObject saiten;
        saiten[QStringLiteral("open")] = alsFeld(akkord.leer);
        saiten[QStringLiteral("frets")] = alsFeld(akkord.buende);
        saiten[QStringLiteral("fingers")] = alsFeld(akkord.finger);
        saiten[QStringLiteral("barres")] = alsFeld(akkord.barre);
        o[QStringLiteral("strings")] = saiten;
    }
    else
        o[QStringLiteral("voicing")] = alsFeld(akkord.abstaende);
    return o;
}

bool ausJson(const QJsonObject& wurzel, Akkord& akkord, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    if (wurzel.value(QStringLiteral("version")).toInt(1) > kAkkordFassung)
    {
        if (zuNeu != nullptr) *zuNeu = true;  // eine Fassung von morgen: lieber nichts als etwas Falsches
        return false;
    }
    Akkord neu;
    neu.name = akkord.name;
    neu.titel = wurzel.value(QStringLiteral("title")).toString().trimmed();
    const std::optional<int> grundton = grundtonAusName(wurzel.value(QStringLiteral("root")).toString());
    if (!grundton) return false;  // ohne Grundton kein Akkord
    neu.grundton = *grundton;
    neu.art = wurzel.value(QStringLiteral("type")).toString().trimmed();
    neu.herkunft = wurzel.value(QStringLiteral("origin")).toString().trimmed();
    neu.beschreibung = wurzel.value(QStringLiteral("description")).toString().trimmed();
    if (wurzel.contains(QStringLiteral("strings")))
    {
        const QJsonObject saiten = wurzel.value(QStringLiteral("strings")).toObject();
        neu.leer = ausFeld(saiten.value(QStringLiteral("open")), 0);
        neu.buende = ausFeld(saiten.value(QStringLiteral("frets")), -1);
        neu.finger = ausFeld(saiten.value(QStringLiteral("fingers")), 0);
        neu.barre = ausFeld(saiten.value(QStringLiteral("barres")), 0);
        if (neu.leer.empty()) return false;
    }
    else
        neu.abstaende = ausFeld(wurzel.value(QStringLiteral("voicing")), -1);
    richteAkkord(neu);
    // ein Griffbild ohne gespielte Saite, ein Voicing ohne Ton: nichts zu setzen
    if (neu.istGriffbild() ? std::none_of(neu.buende.begin(), neu.buende.end(), [](int b) { return b >= 0; })
                           : neu.abstaende.empty())
        return false;
    akkord = std::move(neu);
    return true;
}

bool ladeAkkord(const QString& pfad, Akkord& akkord, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    Akkord neu;
    if (!ausJson(doc.object(), neu, zuNeu)) return false;
    neu.name = pfad;
    if (neu.titel.isEmpty()) neu.titel = QFileInfo(pfad).completeBaseName();
    akkord = std::move(neu);
    return true;
}

bool speichereAkkord(const QString& pfad, const Akkord& akkord)
{
    if (!QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    const QJsonDocument doc(alsJson(akkord));
    return datei.write(doc.toJson(QJsonDocument::Indented)) >= 0 && datei.commit();
}

bool ladeAkkordSammlung(const QString& pfad, AkkordSammlung& sammlung, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    AkkordSammlung neu;
    neu.name = QFileInfo(pfad).completeBaseName();
    if (!liesSammlungKopf(doc.object(), neu, zuNeu)) return false;
    liesSammlungAkkorde(doc.object().value(QStringLiteral("chords")).toArray(), neu);
    sammlung = std::move(neu);
    return true;
}

// --- Ablage ------------------------------------------------------------------

QString mitgelieferterAkkordOrdner()
{
    const QString loop = lumi::core::mitgeliefert::ordner(QStringLiteral("loop"));
    if (loop.isEmpty()) return {};
    const QString ordner = loop + QStringLiteral("/chords");
    return QFileInfo(ordner).isDir() ? ordner : QString();
}

std::vector<Akkord> mitgelieferteAkkorde()
{
    return MitgelieferteSammlungen::ablage().akkorde([](const AkkordSammlung&) { return true; });
}

std::vector<Akkord> mitgelieferteAkkorde(const Saitensatz& saiten)
{
    // eine Sammlung traegt eine Stimmung: sie passt ganz oder gar nicht (wie `passtZu`)
    return MitgelieferteSammlungen::ablage().akkorde([&saiten](const AkkordSammlung& kopf) {
        return kopf.leer.empty() ? !saiten.istSaiteninstrument() : kopf.leer == saiten.leer;
    });
}

QString eigenerAkkordOrdner()
{
    if (!eigenerOrdnerImTest().isEmpty()) return eigenerOrdnerImTest();
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return basis.isEmpty() ? QString() : QDir(basis).filePath(QStringLiteral("chords"));
}

void setzeEigenenAkkordOrdnerFuerTests(const QString& ordner)
{
    eigenerOrdnerImTest() = ordner;
}

QString eigeneAkkordDatei(const QString& name)
{
    const QString ordner = eigenerAkkordOrdner();
    if (ordner.isEmpty() || !istEigenerName(name)) return {};
    return QDir(ordner).filePath(name + QLatin1String(kAkkordDateiEndung));
}

std::vector<Akkord> eigeneAkkorde()
{
    return leseOrdner(eigenerAkkordOrdner(), QLatin1String(kEigenVorsatz));
}

bool holeAkkord(const QString& nennung, Akkord& akkord)
{
    if (nennung.startsWith(QLatin1String(kEigenVorsatz)))
    {
        const QString datei = eigeneAkkordDatei(nennung.mid(static_cast<int>(qstrlen(kEigenVorsatz))));
        Akkord eigener;
        if (datei.isEmpty() || !ladeAkkord(datei, eigener)) return false;
        eigener.name = nennung;
        akkord = std::move(eigener);
        return true;
    }
    if (nennung.endsWith(QLatin1String(kAkkordDateiEndung), Qt::CaseInsensitive) || QFileInfo(nennung).isAbsolute())
        return ladeAkkord(nennung, akkord);
    std::optional<Akkord> mitgeliefert = MitgelieferteSammlungen::ablage().finde(nennung);
    if (!mitgeliefert) return false;
    akkord = std::move(*mitgeliefert);
    return true;
}

QString freierAkkordName(const QString& titel)
{
    QString basis = eigenerName(titel);
    if (basis.isEmpty()) basis = QStringLiteral("chord");
    QString name = basis;
    for (int n = 2; QFileInfo::exists(eigeneAkkordDatei(name)); ++n) name = QStringLiteral("%1_%2").arg(basis).arg(n);
    return name;
}

bool loescheEigenenAkkord(const QString& nennung)
{
    if (!nennung.startsWith(QLatin1String(kEigenVorsatz))) return false;
    const QString datei = eigeneAkkordDatei(nennung.mid(static_cast<int>(qstrlen(kEigenVorsatz))));
    return !datei.isEmpty() && QFile::remove(datei);
}

// --- ein Griffbild bearbeiten ----------------------------------------------------

namespace
{

/// Nach einer Aenderung: in die Grenzen, und ein Barre ohne Saite auf seinem Bund faellt weg.
void richteNachAenderung(Akkord& akkord)
{
    richteAkkord(akkord);
    std::erase_if(akkord.barre, [&](int bund) {
        return std::count(akkord.buende.begin(), akkord.buende.end(), bund) < 2;  // ein Barre greift mindestens zwei
    });
}

bool saiteGilt(const Akkord& akkord, int saite)
{
    return akkord.istGriffbild() && saite >= 0 && saite < static_cast<int>(akkord.buende.size());
}

}  // namespace

void schaltePunkt(Akkord& akkord, int saite, int bund)
{
    if (!saiteGilt(akkord, saite) || bund < 1 || bund > kBuendeMax) return;
    int& hier = akkord.buende[static_cast<std::size_t>(saite)];
    hier = hier == bund ? -1 : bund;
    akkord.finger[static_cast<std::size_t>(saite)] = 0;
    richteNachAenderung(akkord);
}

void schalteLeer(Akkord& akkord, int saite)
{
    if (!saiteGilt(akkord, saite)) return;
    int& hier = akkord.buende[static_cast<std::size_t>(saite)];
    hier = hier == 0 ? -1 : 0;
    akkord.finger[static_cast<std::size_t>(saite)] = 0;
    richteNachAenderung(akkord);
}

void setzeFinger(Akkord& akkord, int saite, int finger)
{
    if (!saiteGilt(akkord, saite) || akkord.buende[static_cast<std::size_t>(saite)] < 1) return;
    akkord.finger[static_cast<std::size_t>(saite)] = std::clamp(finger, 0, 4);
}

void setzeBarre(Akkord& akkord, int von, int bis, int bund)
{
    if (von > bis) std::swap(von, bis);
    if (!saiteGilt(akkord, von) || !saiteGilt(akkord, bis) || von == bis || bund < 1 || bund > kBuendeMax) return;
    const int ersterFinger = akkord.finger[static_cast<std::size_t>(von)];
    const int finger = ersterFinger > 0 && akkord.buende[static_cast<std::size_t>(von)] == bund ? ersterFinger : 1;
    for (int s = von; s <= bis; ++s)
    {
        int& hier = akkord.buende[static_cast<std::size_t>(s)];
        if (hier < bund) hier = bund;  // leer, nicht gespielt oder tiefer: unter den Barre
        if (hier == bund) akkord.finger[static_cast<std::size_t>(s)] = finger;
    }
    if (std::find(akkord.barre.begin(), akkord.barre.end(), bund) == akkord.barre.end()) akkord.barre.push_back(bund);
    richteNachAenderung(akkord);
}

// --- Was zu einem Instrument passt, und die Noten -----------------------------

bool passtZu(const Akkord& akkord, const Saitensatz& saiten)
{
    if (!akkord.istGriffbild()) return !saiten.istSaiteninstrument();
    return akkord.leer == saiten.leer;
}

std::vector<Akkord> passendeAkkorde(const std::vector<Akkord>& alle, const Saitensatz& saiten)
{
    std::vector<Akkord> passend;
    for (const Akkord& akkord : alle)
        if (passtZu(akkord, saiten)) passend.push_back(akkord);
    return passend;
}

std::optional<std::vector<AkkordTon>> toeneDesGriffbilds(const Akkord& akkord, const Saitensatz& saiten, int verschiebung)
{
    if (!passtZu(akkord, saiten) || akkord.buende.size() != akkord.leer.size()) return std::nullopt;
    const std::optional<Akkord> lage = verschobenesGriffbild(akkord, verschiebung);
    if (!lage) return std::nullopt;
    std::vector<AkkordTon> toene;
    for (std::size_t s = 0; s < lage->buende.size(); ++s)
    {
        const int bund = lage->buende[s];
        if (bund < 0) continue;  // nicht gespielt
        toene.push_back({saiten.leer[s] + saiten.capo + bund, static_cast<int>(s)});
    }
    for (const AkkordTon& ton : toene)
        if (ton.hoehe < kHoeheMin || ton.hoehe > kHoeheMax) return std::nullopt;
    return toene;
}

std::optional<std::vector<AkkordTon>> toeneDesVoicings(const Akkord& akkord, int tiefster)
{
    if (akkord.istGriffbild() || akkord.abstaende.empty()) return std::nullopt;
    std::vector<AkkordTon> toene;
    for (const int abstand : akkord.abstaende)
    {
        const int hoehe = tiefster + abstand;
        if (hoehe < kHoeheMin || hoehe > kHoeheMax) return std::nullopt;
        toene.push_back({hoehe, -1});
    }
    return toene;
}

std::vector<Note> akkordNoten(const Akkord& akkord, const Saitensatz& saiten, int schritt, int laenge, int bezug)
{
    const std::optional<std::vector<AkkordTon>> toene =
        akkord.istGriffbild() ? toeneDesGriffbilds(akkord, saiten, bezug) : toeneDesVoicings(akkord, bezug);
    std::vector<Note> noten;
    if (!toene) return noten;
    for (const AkkordTon& ton : *toene)
    {
        Note note{std::max(0, schritt), std::max(1, laenge), ton.hoehe};
        note.saite = ton.saite;
        noten.push_back(note);
    }
    return noten;
}

std::optional<Akkord> verschobenesGriffbild(const Akkord& akkord, int verschiebung)
{
    if (!akkord.istGriffbild()) return std::nullopt;
    Akkord lage = akkord;
    for (int& bund : lage.buende)
    {
        if (bund < 0) continue;  // nicht gespielt
        // der Griff wandert, eine leere Saite wird mitgegriffen (Barre der Hand)
        bund += verschiebung;
        if (bund < 0 || bund > kBuendeMax) return std::nullopt;
    }
    for (int& bund : lage.barre)
    {
        bund += verschiebung;
        if (bund < 1 || bund > kBuendeMax) return std::nullopt;
    }
    return lage;
}

// --- Arten und Namen ---------------------------------------------------------

bool ladeAkkordArten(const QString& pfad, std::vector<AkkordArt>& arten)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    const QJsonObject wurzel = doc.object();
    if (wurzel.value(QStringLiteral("format")).toString() != QLatin1String(kAkkordArtenFormat) ||
        wurzel.value(QStringLiteral("version")).toInt() != 1)
        return false;
    std::vector<AkkordArt> gelesen;
    for (const QJsonValue& wert : wurzel.value(QStringLiteral("types")).toArray())
    {
        const QJsonObject o = wert.toObject();
        AkkordArt art;
        art.name = o.value(QStringLiteral("name")).toString();
        art.stufen = ausFeld(o.value(QStringLiteral("steps")), -1);
        // gueltig: ein Name, einmal; aufsteigend ab 0, in der Weite eines Voicings
        if (art.name.isEmpty() || art.stufen.empty() || art.stufen.front() != 0 ||
            !std::is_sorted(art.stufen.begin(), art.stufen.end()) ||
            std::adjacent_find(art.stufen.begin(), art.stufen.end()) != art.stufen.end() ||
            art.stufen.back() > kVoicingWeiteMax)
            return false;
        if (std::any_of(gelesen.begin(), gelesen.end(), [&](const AkkordArt& a) { return a.name == art.name; })) return false;
        gelesen.push_back(std::move(art));
    }
    if (gelesen.empty()) return false;
    arten = std::move(gelesen);
    return true;
}

const std::vector<AkkordArt>& akkordArten()
{
    // einmal gelesen und gemerkt — aber nur, wenn die Datei da war (S-T13 a)
    static std::vector<AkkordArt> arten;
    if (arten.empty())
        if (const QString ordner = mitgelieferterAkkordOrdner(); !ordner.isEmpty())
            (void)ladeAkkordArten(QDir(ordner).filePath(QLatin1String(kAkkordArtenDatei)), arten);
    return arten;
}

namespace
{

/// Die Tonklassen einer Art (0 … 11), als Bitfeld.
int klassenDerArt(const AkkordArt& art)
{
    int klassen = 0;
    for (const int stufe : art.stufen) klassen |= 1 << (stufe % 12);
    return klassen;
}

/// Ein Bitfeld um `grundton` gedreht: die Klassen ueber diesem Ton.
int gedreht(int klassen, int grundton)
{
    int neu = 0;
    for (int k = 0; k < 12; ++k)
        if (klassen & (1 << k)) neu |= 1 << ((k - grundton + 12) % 12);
    return neu;
}

int zahlDerKlassen(int klassen)
{
    int zahl = 0;
    for (int k = 0; k < 12; ++k) zahl += (klassen >> k) & 1;
    return zahl;
}

}  // namespace

std::optional<AkkordName> erkenneAkkord(const std::vector<int>& hoehen)
{
    if (hoehen.empty()) return std::nullopt;
    int klassen = 0;
    for (const int h : hoehen) klassen |= 1 << (((h % 12) + 12) % 12);
    if (zahlDerKlassen(klassen) < 3) return std::nullopt;
    const int bass = ((*std::min_element(hoehen.begin(), hoehen.end()) % 12) + 12) % 12;
    constexpr int kQuinte = 1 << 7;
    const std::vector<AkkordArt>& arten = akkordArten();
    // Vorrang: vollstaendig mit dem Grundton im Bass, dann Dur oder Moll ueber einem
    // anderen Bass (C-E-A ist Am/C, nicht C6 ohne Quinte), dann ohne Quinte; je Stufe
    // die Reihenfolge der Arten
    for (const AkkordArt& art : arten)
        if (gedreht(klassen, bass) == klassenDerArt(art)) return AkkordName{bass, art.name, -1};
    // Slash: der Bass darf zum Akkord gehoeren (C/E) oder nicht (A/G#)
    std::vector<const AkkordArt*> durMoll;
    for (const AkkordArt& art : arten)
        if (art.name == QLatin1String("major") || art.name == QLatin1String("minor")) durMoll.push_back(&art);
    for (int grundton = 0; grundton < 12; ++grundton)
    {
        if (grundton == bass || !(klassen & (1 << grundton))) continue;
        for (const AkkordArt* art : durMoll)
            if (gedreht(klassen, grundton) == (klassenDerArt(*art) | (1 << ((bass - grundton + 12) % 12))))
                return AkkordName{grundton, art->name, bass};
    }
    for (const AkkordArt& art : arten)
    {
        const int soll = klassenDerArt(art);
        if (art.stufen.size() >= 4 && (soll & kQuinte) && gedreht(klassen, bass) == (soll & ~kQuinte))
            return AkkordName{bass, art.name, -1};
    }
    return std::nullopt;
}

std::optional<int> tiefsteTonklasse(const Akkord& voicing)
{
    if (voicing.istGriffbild()) return std::nullopt;
    for (const AkkordArt& art : akkordArten())
    {
        if (art.name != voicing.art) continue;
        int soll = 0;
        for (const int s : art.stufen) soll |= 1 << (s % 12);
        for (int unten = 0; unten < 12; ++unten)
        {
            int ist = 0;  // die Klassen ueber dem Grundton, liegt `unten` ueber ihm unten
            for (const int a : voicing.abstaende) ist |= 1 << ((unten + a) % 12);
            if (ist == soll) return (voicing.grundton + unten) % 12;
        }
    }
    return voicing.grundton;
}

int vorgabeTiefster(const Akkord& voicing, int oktavBeginn)
{
    return oktavBeginn + tiefsteTonklasse(voicing).value_or(voicing.grundton);
}

QString galerieSymbol(const Akkord& akkord)
{
    if (akkord.istGriffbild()) return akkord.titel;
    const int unten = tiefsteTonklasse(akkord).value_or(akkord.grundton);
    if (unten == akkord.grundton || akkord.titel.contains(QLatin1Char('/'))) return akkord.titel;
    return akkord.titel + QLatin1Char('/') + grundtonNamen().at(unten);
}

void schalteVoicingTon(Akkord& voicing, int& tiefster, int hoehe)
{
    if (voicing.istGriffbild() || hoehe < kHoeheMin || hoehe > kHoeheMax) return;
    std::vector<int> toene;
    for (const int a : voicing.abstaende) toene.push_back(tiefster + a);
    if (const auto da = std::find(toene.begin(), toene.end(), hoehe); da != toene.end())
        toene.erase(da);
    else
        toene.push_back(hoehe);
    std::sort(toene.begin(), toene.end());
    // nicht weiter als ein Voicing reicht, nicht mehr Toene: dann bleibt es, wie es war
    if (!toene.empty() && (toene.back() - toene.front() > kVoicingWeiteMax || static_cast<int>(toene.size()) > kVoicingToeneMax))
        return;
    voicing.abstaende.clear();
    if (!toene.empty()) tiefster = toene.front();
    for (const int t : toene) voicing.abstaende.push_back(t - tiefster);
}

QString akkordArtText(const AkkordName& name)
{
    if (name.bass < 0) return name.art;
    const QString bass = grundtonNamen().at(name.bass);
    return (name.art == QLatin1String("minor") ? QStringLiteral("m/") : QStringLiteral("/")) + bass;
}

QString akkordSymbol(const AkkordName& name)
{
    QString symbol = grundtonNamen().at(((name.grundton % 12) + 12) % 12);
    if (name.art == QLatin1String("minor"))
        symbol += QLatin1Char('m');
    else if (name.art != QLatin1String("major"))
        symbol += name.art;
    if (name.bass >= 0) symbol += QLatin1Char('/') + grundtonNamen().at(name.bass);
    return symbol;
}

std::optional<std::vector<int>> fremdeTonklassen(const Akkord& akkord)
{
    if (!akkord.istGriffbild()) return std::nullopt;
    // die Art ohne den Bass: `m/E` ist Moll, `/E` Dur — der Bass gehoert dazu
    const QString vorn = akkord.art.section(QLatin1Char('/'), 0, 0);
    const QString art = akkord.art.contains(QLatin1Char('/'))
                            ? (vorn == QLatin1String("m") ? QStringLiteral("minor") : vorn.isEmpty() ? QStringLiteral("major") : vorn)
                            : akkord.art;
    const auto bekannt = std::find_if(akkordArten().begin(), akkordArten().end(),
                                      [&art](const AkkordArt& a) { return a.name == art; });
    if (bekannt == akkordArten().end()) return std::nullopt;
    int erlaubt = 0;
    for (const int stufe : bekannt->stufen) erlaubt |= 1 << ((akkord.grundton + stufe) % 12);
    if (akkord.art.contains(QLatin1Char('/')))
        if (const std::optional<int> bass = grundtonAusName(akkord.art.section(QLatin1Char('/'), 1))) erlaubt |= 1 << *bass;
    Saitensatz saiten;
    saiten.leer = akkord.leer;
    std::vector<int> fremd;
    if (const auto toene = toeneDesGriffbilds(akkord, saiten))
        for (const AkkordTon& ton : *toene)
        {
            const int klasse = ton.hoehe % 12;
            if (!(erlaubt & (1 << klasse)) && std::find(fremd.begin(), fremd.end(), klasse) == fremd.end()) fremd.push_back(klasse);
        }
    std::sort(fremd.begin(), fremd.end());
    return fremd;
}

QString lageText(const Akkord& akkord)
{
    if (!akkord.istGriffbild())
    {
        // die Vorlagen nennen ihre Lage; sonst aus dem tiefsten Ton und der Weite
        static const std::pair<const char*, const char*> kNamen[] = {
            {"Root position", "root"},      {"First inversion", "1st inv."}, {"Second inversion", "2nd inv."},
            {"Third inversion", "3rd inv."}, {"Open position", "open"},     {"Drop 2", "drop 2"},
        };
        for (const auto& [satz, kurz] : kNamen)
            if (akkord.beschreibung == QLatin1String(satz)) return QLatin1String(kurz);
        if (akkord.abstaende.empty()) return {};
        const int unten = tiefsteTonklasse(akkord).value_or(akkord.grundton);
        if (unten == akkord.grundton) return akkord.abstaende.back() > 12 ? QStringLiteral("open") : QStringLiteral("root");
        // die wievielte Stufe der Art unten liegt: 1st, 2nd, 3rd
        std::vector<int> stufen;
        for (const AkkordArt& art : akkordArten())
            if (art.name == akkord.art)
                for (const int s : art.stufen) stufen.push_back(s % 12);
        const auto platz = std::find(stufen.begin(), stufen.end(), (unten - akkord.grundton + 12) % 12);
        static const char* const kUmkehrung[] = {"root", "1st inv.", "2nd inv.", "3rd inv."};
        const auto nummer = platz - stufen.begin();
        return platz != stufen.end() && nummer < 4 ? QString::fromLatin1(kUmkehrung[nummer]) : QStringLiteral("inv.");
    }
    int tief = kBuendeMax + 1;
    for (const int bund : akkord.buende)
    {
        if (bund == 0) return QStringLiteral("open");
        if (bund > 0) tief = std::min(tief, bund);
    }
    return tief > kBuendeMax ? QString() : QStringLiteral("fr %1").arg(tief);
}

Muster akkordZumAnhoeren(const Akkord& lage, const Saitensatz& saiten, const Strum& strum, AkkordSpielart art,
                         int schritteJeSchlag, int tiefster)
{
    Muster noten;
    noten.art = MusterArt::Instrument;
    if (!lage.istGriffbild())
    {
        // ein Voicing: alle Toene zugleich, zwei Schlaege — ohne Saiten gibt es keinen Anschlag
        if (const auto toene = toeneDesVoicings(lage, tiefster))
            for (const AkkordTon& ton : *toene) noten.noten.push_back(Note{0, 2 * std::max(1, schritteJeSchlag), ton.hoehe, 1.0});
        return noten;
    }
    noten.strum = strum;
    const std::optional<std::vector<AkkordTon>> toene = toeneDesGriffbilds(lage, saiten);
    if (!toene || toene->empty()) return noten;
    const int schlag = std::max(1, schritteJeSchlag);
    const int halb = std::max(1, schlag / 2);
    // ein Anschlag: alle Toene an `schritt`, `laenge` Schritte lang, in dieser Richtung
    const auto schlage = [&](int schritt, int laenge, Anschlagart richtung) {
        for (const AkkordTon& ton : *toene) noten.noten.push_back(Note{schritt, laenge, ton.hoehe, 1.0, ton.saite});
        noten.anschlaege.push_back(Anschlag{schritt, richtung});
    };
    switch (art)
    {
    case AkkordSpielart::Ab: schlage(0, 2 * schlag, Anschlagart::Ab); break;
    case AkkordSpielart::Auf: schlage(0, 2 * schlag, Anschlagart::Auf); break;
    case AkkordSpielart::Zugleich: schlage(0, 2 * schlag, Anschlagart::Zugleich); break;
    case AkkordSpielart::AbAuf:
        schlage(0, halb, Anschlagart::Ab);
        schlage(halb, 2 * schlag, Anschlagart::Auf);
        break;
    }
    return noten;
}

}  // namespace lumi::loop
