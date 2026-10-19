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
#include <QStandardPaths>

#include <algorithm>

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

/// Die Akkorde eines Ordners: mit `rekursiv` auch in Unterordnern, genannt beim
/// Pfad darunter ohne Endung (`guitar/c_major_1`); sonst `vorsatz` + Dateiname.
std::vector<Akkord> leseOrdner(const QString& ordner, const QString& vorsatz, bool rekursiv)
{
    std::vector<Akkord> alle;
    if (ordner.isEmpty() || !QFileInfo(ordner).isDir()) return alle;
    const QDir basis(ordner);
    QDirIterator lauf(ordner, {QStringLiteral("*") + QLatin1String(kAkkordDateiEndung)}, QDir::Files,
                      rekursiv ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while (lauf.hasNext())
    {
        const QString pfad = lauf.next();
        const QString name = QFileInfo(pfad).completeBaseName();
        if (!istEigenerName(name)) continue;  // ein Name, den niemand nennen kann
        Akkord akkord;
        if (!ladeAkkord(pfad, akkord)) continue;
        const QString unten = QFileInfo(basis.relativeFilePath(pfad)).path();
        akkord.name = vorsatz + (unten == QLatin1String(".") ? name : unten + QLatin1Char('/') + name);
        alle.push_back(std::move(akkord));
    }
    std::stable_sort(alle.begin(), alle.end(), [](const Akkord& a, const Akkord& b) {
        const int titel = a.titel.compare(b.titel, Qt::CaseInsensitive);
        return titel != 0 ? titel < 0 : a.name < b.name;  // gleiche Titel (Lagen): nach Nennung
    });
    return alle;
}

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

// --- Ablage ------------------------------------------------------------------

QString mitgelieferterAkkordOrdner()
{
    const QString loop = lumi::core::mitgeliefert::ordner(QStringLiteral("loop"));
    if (loop.isEmpty()) return {};
    const QString ordner = loop + QStringLiteral("/chords");
    return QFileInfo(ordner).isDir() ? ordner : QString();
}

const std::vector<Akkord>& mitgelieferteAkkorde()
{
    // einmal gelesen und gemerkt — aber nur, wenn der Ordner da war
    static std::vector<Akkord> alle;
    if (alle.empty()) alle = leseOrdner(mitgelieferterAkkordOrdner(), QString(), true);
    return alle;
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
    return leseOrdner(eigenerAkkordOrdner(), QLatin1String(kEigenVorsatz), false);
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
    const std::vector<Akkord>& alle = mitgelieferteAkkorde();
    const auto it = std::find_if(alle.begin(), alle.end(), [&nennung](const Akkord& a) { return a.name == nennung; });
    if (it == alle.end()) return false;
    akkord = *it;
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
    int tief = kBuendeMax + 1;
    for (const int bund : akkord.buende)
    {
        if (bund == 0) return QStringLiteral("open");
        if (bund > 0) tief = std::min(tief, bund);
    }
    return tief > kBuendeMax ? QString() : QStringLiteral("fr %1").arg(tief);
}

Muster akkordZumAnhoeren(const Akkord& lage, const Saitensatz& saiten, const Strum& strum, AkkordSpielart art,
                         int schritteJeSchlag)
{
    Muster noten;
    noten.art = MusterArt::Instrument;
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
