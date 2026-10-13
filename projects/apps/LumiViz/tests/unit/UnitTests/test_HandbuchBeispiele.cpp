/**
 ****************************************************************************************
 * @file   test_HandbuchBeispiele.cpp
 * @brief  Pruefer: die Skriptbeispiele des Benutzerhandbuchs
 *
 * @details
 * Session 92 (Entscheid Patrik a). In S90 und S91 stand je einmal ein
 * Skriptbeispiel im Handbuch, das nie gelaufen war bzw. dessen Wirkung kein
 * Test pruefte — bemerkt erst vor dem Commit, von Hand. Dieser Test macht das
 * zur Regel, in zwei Stufen:
 *
 * 1. **Uebersetzen:** jedes Code-Stueck des Handbuchs, das wie ein Skript
 *    aussieht (Zuweisung und Semikolon — Inline-Code und Code-Bloecke), geht
 *    durch den EEL-Transpiler. Ein Syntaxfehler laesst den Test scheitern.
 * 2. **Einordnen:** jedes Beispiel steht in der Liste unten — entweder als
 *    **belegt** (es laeuft woertlich in der genannten Testdatei; das wird
 *    nachgesehen) oder ausdruecklich als **nur uebersetzt** (mit Grund).
 *
 * Ein neues Beispiel ohne Eintrag laesst den Test scheitern und nennt die
 * Zeile im Handbuch; ein Eintrag, den es im Handbuch nicht mehr gibt, ebenso.
 *
 * Grenze: „belegt" heisst, dass das Beispiel in einem Test LAEUFT — ob dessen
 * Pruefungen treffen, was der Satz im Handbuch behauptet, sieht dieser Test
 * nicht. Uebersetzt wird nur die Syntax; ein vertippter Variablenname ist in
 * EEL eine neue Variable und faellt erst in einem belegenden Test auf.
 ****************************************************************************************
 */

#include <doctest.h>

#include <EelTranspiler.hpp>

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace
{
enum class Art
{
    Belegt,         ///< laeuft woertlich in der genannten Testdatei
    NurUebersetzt   ///< nur die Syntax ist geprueft — der Grund steht dabei
};

struct Eintrag
{
    const char* code;  ///< das Beispiel, Leerraum auf je ein Leerzeichen gebracht
    Art art;
    const char* wo;    ///< Belegt: Testdatei · NurUebersetzt: warum nicht mehr
};

// Die Liste. Reihenfolge wie im Handbuch.
const Eintrag kBeispiele[] = {
    // §11 Lyrics — Felder Line und Word
    {"yshift = -6 - rand(8);", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"halign = line % 3;", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"yshift = -6 - word;", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    // §11 Lyrics — unsynchronen Text per Skript weiterruecken
    {"linetrigger = 1;", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"wordtrigger = 1;", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"linetrigger = b;", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"n = 0;", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"n = n + b; linetrigger = if(b, equal(n % 4, 0), 0);", Art::Belegt,
     "test_FeldSteuerungGl.cpp"},
    {"wordtrigger = b;", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"wordtrigger = if(below(word, wordcount), b, 0);", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    // §11 Lyrics — Puls auf den Beat
    {"ref = fontheight; p = 0;", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"p = if(b, 1, p * 0.93); fontheight = ref * (1 + p);", Art::Belegt,
     "test_FeldSteuerungGl.cpp"},
    // §11 Lyrics Trail — Feld Line
    {"xshift = rand(21) - 10; yshift = -6 - rand(8);", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    {"blendstrength = if(line % 2, 100, 50);", Art::Belegt, "test_FeldSteuerungGl.cpp"},
    // §11 Shape — die Regler aus Beat und Frame, das Feld Point
    {"harmonics = 3;", Art::Belegt, "test_ShapeKnoten.cpp"},
    {"harmonics = harmonics + (64 - harmonics) * 0.06;", Art::Belegt, "test_ShapeKnoten.cpp"},
    {"r = 1 + v * 0.35; x = x * r; y = y * r;", Art::Belegt, "test_ShapeKnoten.cpp"},
    {"skip = above(i, 0.5);", Art::Belegt, "test_ShapeKnoten.cpp"},
    // §11 XY Scope — Gain im Takt, das Feld Point
    {"gain = 2.5;", Art::Belegt, "test_XyScopeKnoten.cpp"},
    {"gain = gain + (1 - gain) * 0.12;", Art::Belegt, "test_XyScopeKnoten.cpp"},
    {"y = x; x = i * 2 - 1;", Art::Belegt, "test_XyScopeKnoten.cpp"},
    // Song-Feld: ein Muster mit frei erfundenen Variablen, keine Wirkung zu pruefen
    {"farbe = rand(6); zaehler = 0;", Art::NurUebersetzt,
     "Muster fuer das Song-Feld mit eigenen Variablen — zeigt nur die Schreibweise"},
    // Milkdrop-Node, Drift: vier Einzeiler fuer das Frame-Feld
    {"drift_dir = time * 0.5;", Art::NurUebersetzt,
     "Drift des Milkdrop-Nodes: Wirkung nur im Bild pruefbar, kein Test dafuer (offen)"},
    {"drift_ang = 1.5708 + bass_att * 0.8;", Art::NurUebersetzt,
     "Drift des Milkdrop-Nodes: Wirkung nur im Bild pruefbar, kein Test dafuer (offen)"},
    {"drift_rad = if(above(bass, 1.3), 3, 0.5);", Art::NurUebersetzt,
     "Drift des Milkdrop-Nodes: Wirkung nur im Bild pruefbar, kein Test dafuer (offen)"},
    {"drift_cx = 0.5 + 0.3 * sin(time);", Art::NurUebersetzt,
     "Drift des Milkdrop-Nodes: Wirkung nur im Bild pruefbar, kein Test dafuer (offen)"},
    // Host Switcher
    {"nextgroup = above(beatcount, 31);", Art::NurUebersetzt,
     "Host Switcher: nextgroup ist getestet, dieses Beispiel woertlich nicht (offen)"},
};

/// Ordner der App (`projects/apps/LumiViz`) — vom Quelltext dieses Tests aufwaerts.
std::filesystem::path appOrdner()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 8 && p.has_parent_path(); ++i)
    {
        p = p.parent_path();
        if (std::filesystem::exists(p / "docs" / "Benutzerhandbuch.md")) return p;
    }
    return {};
}

QString lies(const std::filesystem::path& datei)
{
    QFile f(QString::fromStdWString(datei.wstring()));
    if (!f.open(QIODevice::ReadOnly)) return QString();
    QString text = QString::fromUtf8(f.readAll());
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    return text;
}

struct Fund
{
    int zeile = 0;
    QString code;
};

/// Sieht das Stueck wie ein Skript aus? Eine Zuweisung und ein Semikolon.
bool istSkript(const QString& code)
{
    static const QRegularExpression zuweisung(QStringLiteral("[A-Za-z_0-9\\)\\]]\\s*=[^=]"));
    return code.contains(QLatin1Char(';')) && zuweisung.match(code).hasMatch();
}

/// Die Skriptbeispiele eines Markdown-Texts: Code-Bloecke zeilenweise (ein
/// Vorsatz wie „Song:" und ein `//`-Kommentar fallen weg), Inline-Code auch
/// ueber EINEN Zeilenumbruch (umbrochener Fliesstext).
std::vector<Fund> sammle(QString text)
{
    std::vector<Fund> funde;
    const auto zeileBei = [](const QString& t, qsizetype pos) {
        return static_cast<int>(t.left(pos).count(QLatin1Char('\n'))) + 1;
    };
    static const QRegularExpression block(QStringLiteral("```[^\\n]*\\n(.*?)```"),
                                          QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression vorsatz(QStringLiteral("^\\s*[A-Za-z]+:\\s+"));
    static const QRegularExpression kommentar(QStringLiteral("\\s*//.*$"));
    for (QRegularExpressionMatch m = block.match(text); m.hasMatch(); m = block.match(text))
    {
        const int erste = zeileBei(text, m.capturedStart(1));
        const QStringList zeilen = m.captured(1).split(QLatin1Char('\n'));
        for (int i = 0; i < zeilen.size(); ++i)
        {
            QString code = zeilen.at(i);
            code.remove(vorsatz);
            code.remove(kommentar);
            code = code.simplified();
            if (istSkript(code)) funde.push_back({erste + i, code});
        }
        // den Block herausnehmen, die Zeilenzahl lassen
        text.replace(m.capturedStart(0), m.capturedLength(0),
                     QString(m.captured(0).count(QLatin1Char('\n')), QLatin1Char('\n')));
    }
    static const QRegularExpression inlineCode(QStringLiteral("`([^`\\n]+(?:\\n[^`\\n]+)?)`"));
    QRegularExpressionMatchIterator it = inlineCode.globalMatch(text);
    while (it.hasNext())
    {
        const QRegularExpressionMatch m = it.next();
        const QString code = m.captured(1).simplified();
        if (istSkript(code)) funde.push_back({zeileBei(text, m.capturedStart(0)), code});
    }
    return funde;
}
}  // namespace

TEST_CASE("Handbuch-Beispiele S92: der Sammler findet Inline-Code und Bloecke, sonst nichts")
{
    const QString probe = QStringLiteral(
        "Text mit `a = 1;` und ohne Skript `Name` oder `x == 1;`.\n"
        "Umbrochen: `b = if(c,\n1, 2);` fertig.\n"
        "\n"
        "```\n"
        "Song:   d = 3; e = 4;   // Kommentar\n"
        "kein Skript hier\n"
        "```\n"
        "Danach `f = 5;`.\n");
    const std::vector<Fund> f = sammle(probe);
    REQUIRE(f.size() == 4);
    // Bloecke zuerst, dann Inline — jeweils mit der Zeile im Text
    CHECK(f[0].code == QStringLiteral("d = 3; e = 4;"));
    CHECK(f[0].zeile == 6);  // der umbrochene Inline-Code davor belegt zwei Zeilen
    CHECK(f[1].code == QStringLiteral("a = 1;"));
    CHECK(f[1].zeile == 1);
    CHECK(f[2].code == QStringLiteral("b = if(c, 1, 2);"));
    CHECK(f[2].zeile == 2);
    CHECK(f[3].code == QStringLiteral("f = 5;"));
    CHECK(f[3].zeile == 9);  // der herausgenommene Block laesst die Zeilenzahl stehen
}

TEST_CASE("Handbuch-Beispiele S92: jedes Skriptbeispiel uebersetzt und ist eingeordnet")
{
    const std::filesystem::path app = appOrdner();
    REQUIRE_MESSAGE(!app.empty(), "docs/Benutzerhandbuch.md nicht gefunden");
    const QString handbuch = lies(app / "docs" / "Benutzerhandbuch.md");
    REQUIRE_FALSE(handbuch.isEmpty());
    const std::vector<Fund> funde = sammle(handbuch);
    // Gegenprobe gegen einen stummen Sammler: es gibt deutlich mehr als eine Handvoll
    REQUIRE(funde.size() >= 15);

    std::map<QString, const Eintrag*> liste;
    for (const Eintrag& e : kBeispiele) liste[QString::fromUtf8(e.code)] = &e;
    REQUIRE(liste.size() == std::size(kBeispiele));  // kein Beispiel doppelt in der Liste

    // --- Stufe 1 und 2: uebersetzen, einordnen
    std::map<QString, int> gesehen;
    for (const Fund& f : funde)
    {
        const std::string code = f.code.toStdString();
        CAPTURE(f.zeile);
        CAPTURE(code);
        ++gesehen[f.code];
        const auto avs = lumi::eel::transpile(code, lumi::eel::Dialect::Avs);
        const auto milk = lumi::eel::transpile(code, lumi::eel::Dialect::Milkdrop);
        CHECK_MESSAGE((avs.ok || milk.ok),
                      "Benutzerhandbuch.md:" << f.zeile << " uebersetzt nicht: " << avs.error);
        CHECK_MESSAGE(liste.count(f.code) == 1,
                      "Benutzerhandbuch.md:" << f.zeile
                          << " — neues Skriptbeispiel ohne Eintrag in kBeispiele "
                             "(test_HandbuchBeispiele.cpp): als Belegt mit Testdatei oder als "
                             "NurUebersetzt mit Grund eintragen");
    }

    // --- kein Eintrag ohne Beispiel (das Handbuch hat sich geaendert)
    for (const Eintrag& e : kBeispiele)
    {
        CAPTURE(e.code);
        CHECK_MESSAGE(gesehen.count(QString::fromUtf8(e.code)) == 1,
                      "Eintrag in kBeispiele, den das Handbuch nicht (mehr) enthaelt");
    }

    // --- „belegt" heisst: das Beispiel steht woertlich in der genannten Testdatei
    static const QRegularExpression verkettung(QStringLiteral("\"\\s*\\n\\s*\""));
    std::map<std::string, QString> dateien;
    int belegt = 0;
    for (const Eintrag& e : kBeispiele)
    {
        if (e.art != Art::Belegt)
        {
            CAPTURE(e.code);
            CHECK(std::string(e.wo).size() > 10);  // ein Grund, kein Platzhalter
            continue;
        }
        ++belegt;
        if (dateien.count(e.wo) == 0)
        {
            QString quelle = lies(app / "tests" / "unit" / "UnitTests" / e.wo);
            quelle.remove(verkettung);  // "a " ⏎ "b" ist im Programm ein String
            dateien[e.wo] = quelle;
        }
        CAPTURE(e.code);
        CAPTURE(e.wo);
        CHECK_MESSAGE(dateien[e.wo].contains(QString::fromUtf8(e.code)),
                      "als belegt eingetragen, steht aber nicht woertlich in der Testdatei");
    }
    MESSAGE("Skriptbeispiele im Handbuch: " << funde.size() << ", davon belegt " << belegt
                                            << ", nur uebersetzt "
                                            << (std::size(kBeispiele) - belegt));
}
