/**
 * @file Sammlung.cpp
 * @brief Die Sammlung der Hoerproben: Ablage und Liste nach HoerTest_Helper `docs/Konzept.md` 0.3.0 §11/§12
 *        (Herleitung im Kopf von `Hoerproben.hpp`). Nicht `Hoerproben.cpp`: unter Windows waere das dieselbe Datei
 *        wie das Werkzeug `hoerproben.cpp`.
 */
#include "werkzeuge/Hoerproben.hpp"

#include "gemeinsam/Json.hpp"
#include "gemeinsam/Werkzeug.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>

namespace ic::hoer
{

namespace
{

constexpr double kZielLufs = -23.0;
constexpr double kSpitzeHoechstens = 0.99;

std::vector<std::string> glieder(const std::string& name)
{
    std::vector<std::string> g;
    std::stringstream s(name);
    std::string t;
    while (std::getline(s, t, '_'))
        if (!t.empty()) g.push_back(t);
    return g;
}

std::string verbinde(const std::vector<std::string>& g)
{
    std::string s;
    for (const std::string& t : g) s += (s.empty() ? "" : "_") + t;
    return s;
}

std::string sauber(const std::string& s)
{
    std::string aus;
    for (const char c : s) aus += (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_') ? c : '_';
    return aus;
}

JsonWert jText(const std::string& s)
{
    JsonWert w;
    w.art = JsonWert::Art::Text;
    w.text = s;
    return w;
}

JsonWert jZahl(double x)
{
    JsonWert w;
    w.art = JsonWert::Art::Zahl;
    w.zahl = x;
    return w;
}

JsonWert jObjekt()
{
    JsonWert w;
    w.art = JsonWert::Art::Objekt;
    return w;
}

JsonWert jListe()
{
    JsonWert w;
    w.art = JsonWert::Art::Liste;
    return w;
}

JsonWert achsenObjekt(const Klang& k)
{
    JsonWert a = jObjekt();
    for (const auto& [n, v] : k.achsen) a.setze(n, jText(v));
    return a;
}

std::string dateiAbdruck(const std::filesystem::path& pfad)
{
    std::ifstream ein(pfad, std::ios::binary);
    Sha256 h;
    char puffer[1 << 16];
    while (ein.read(puffer, sizeof puffer) || ein.gcount() > 0)
        h.dazu(reinterpret_cast<const std::uint8_t*>(puffer), static_cast<std::size_t>(ein.gcount()));
    return h.fertig();
}

}  // namespace

void achsenAus(const std::string& instrument, const std::string& name, Klang& k)
{
    static const std::regex note("^[A-H](is)?-?[0-9]+$");
    static const std::regex staerke("^(a-?[0-9]+|e[0-9]+)$");
    std::vector<std::string> g = glieder(name);
    k.achsen = {{"instrument", instrument}};
    k.fassung = "vorgabe";
    if (!g.empty() && std::regex_match(g.front(), note))
    {
        k.achsen.push_back({"ton", g.front()});
        g.erase(g.begin());
    }
    std::vector<std::string> rest;
    std::string st;
    for (const std::string& t : g)
    {
        if (t == "mitschwingen")
            k.fassung = "mit";
        else if (std::regex_match(t, staerke) && st.empty())
            st = t;
        else
            rest.push_back(t);
    }
    if (!rest.empty() && rest.back() == "ohne" && rest.size() > 1)
    {
        k.fassung = "ohne";
        rest.pop_back();
    }
    if (!st.empty()) k.achsen.push_back({"staerke", st});
    if (!rest.empty()) k.achsen.push_back({"spielweise", verbinde(rest)});
}

void Sammlung::lege(const std::string& klasse, const std::string& instrument, const std::string& name, std::vector<float> ton, int rate,
                    const std::string& frage)
{
    const bool ersterDerKlasse = !listen.count(klasse);
    std::vector<Klang>& l = listen[klasse];
    if (ersterDerKlasse) std::filesystem::remove_all(std::filesystem::path(wurzel) / "listen" / klasse / "klaenge");  // keine alten Klaenge
    if (!std::all_of(ton.begin(), ton.end(), [](float w) { return std::isfinite(w); }))
    {
        // ein Ton mit NaN/Inf (das Modell ist explodiert) kommt nicht in die Liste — Befund, nicht still
        kaputt.push_back(klasse + "/" + instrument + "_" + name);
        std::fprintf(stderr, "NICHT ENDLICH, ausgelassen: %s/%s_%s\n", klasse.c_str(), instrument.c_str(), name.c_str());
        return;
    }
    Klang k;
    achsenAus(instrument, name, k);
    k.id = sauber(instrument + "_" + name);
    for (int n = 2; std::any_of(l.begin(), l.end(), [&](const Klang& x) { return x.id == k.id; }); ++n) k.id = sauber(instrument + "_" + name) + "_" + std::to_string(n);
    k.datei = "klaenge/" + k.id + ".wav";
    k.frage = frage;
    // IC-2: BS.1770-4 mit Gating (HT14 a) — dieselbe Messung wie im Werkzeug; angeglichen wird beim Schreiben der
    // Liste, auf den gemeinsamen Pegel aller ihrer Klaenge
    k.lufsVorher = lautheit(ton, rate);
    k.spitzeVorher = spitze(ton);
    k.ton = std::move(ton);
    k.rate = rate;
    l.push_back(std::move(k));
    ++zahl;
}

double zielPegel(const std::vector<Klang>& klaenge)
{
    // −23 LUFS, oder tiefer, wenn ein Klang sonst ueber 0,99 kaeme: der Pegel, bei dem der „spitzeste“ Klang gerade 0,99
    // erreicht (auf 0,1 LU abgerundet) — so bleiben alle Klaenge einer Liste gleich laut
    double ziel = kZielLufs;
    for (const Klang& k : klaenge)
        if (k.lufsVorher > -200.0 && k.spitzeVorher > 0.0) ziel = std::min(ziel, k.lufsVorher + dB(kSpitzeHoechstens / k.spitzeVorher));
    return std::floor(ziel * 10.0) / 10.0;
}

void Sammlung::schreibeListen()
{
    for (auto& [klasse, klaenge] : listen)
    {
        const std::filesystem::path ordner = std::filesystem::path(wurzel) / "listen" / klasse;
        const double ziel = zielPegel(klaenge);
        for (Klang& k : klaenge)
        {
            const double faktor = k.lufsVorher > -200.0 ? std::pow(10.0, (ziel - k.lufsVorher) / 20.0) : 1.0;
            for (float& w : k.ton) w = static_cast<float>(w * faktor);
            const std::filesystem::path pfad = ordner / k.datei;
            std::filesystem::create_directories(pfad.parent_path());
            schreibeWav(pfad.string(), k.ton, k.rate, 1);
            k.abdruck = dateiAbdruck(pfad);
            k.ton.clear();
            k.ton.shrink_to_fit();
        }
        JsonWert l = jObjekt();
        l.setze("schema", jZahl(1));
        l.setze("titel", jText("Klasse " + klasse + ": Hoerproben der Vorgaben (L5)"));
        l.setze("ziel", jText("Erstes Hoeren der Vorgaben je Instrument (AUFTRAG_BAU §4 Punkt 4, L5) — ungemessen, ohne Bezug"));
        l.setze("katalog", JsonWert{});  // noch keiner (HT-2)
        l.setze("kontrolle", jText("moderat"));
        JsonWert pegel = jObjekt();
        pegel.setze("lufs", jZahl(ziel));
        pegel.setze("toleranz_lu", jZahl(1.0));
        l.setze("pegel", pegel);
        l.setze("dauer_s", JsonWert{});
        JsonWert gruppe = jListe();
        gruppe.liste.push_back(jText("instrument"));
        l.setze("gruppe_nach", gruppe);
        std::set<std::string> fassungen;
        for (const Klang& k : klaenge) fassungen.insert(k.fassung);
        JsonWert f = jObjekt();
        if (fassungen.count("vorgabe")) f.setze("vorgabe", jText("die Vorgabe des Modells (baueVorgabe…), InstrumentsCalculation rechnung/"));
        if (fassungen.count("mit")) f.setze("mit", jText("mit Mitschwingen der uebrigen Saiten oder Choere"));
        if (fassungen.count("ohne")) f.setze("ohne", jText("dieselbe Folge ohne Mitschwingen"));
        l.setze("fassungen", f);
        JsonWert kl = jListe(), mw = jListe(), pu = jListe();
        for (const Klang& k : klaenge)
        {
            JsonWert e = jObjekt();
            e.setze("id", jText(k.id));
            e.setze("datei", jText(k.datei));
            e.setze("fassung", jText(k.fassung));
            e.setze("achsen", achsenObjekt(k));
            kl.liste.push_back(e);
            // HT13 a: je Klang mit allen seinen Achsen und der Fassung
            JsonWert m = jObjekt();
            m.setze("achsen", achsenObjekt(k));
            m.setze("fassung", jText(k.fassung));
            JsonWert werte = jListe();
            JsonWert v = jObjekt();
            v.setze("name", jText("lufs_vorher"));
            v.setze("wert", jZahl(std::round(k.lufsVorher * 100.0) / 100.0));
            v.setze("einheit", jText("LUFS"));
            werte.liste.push_back(v);
            m.setze("werte", werte);
            mw.liste.push_back(m);
        }
        // Punkte: Paare mit/ohne (gleiche Achsen), sonst je Klang ein Einzelklang mit seiner Frage (HT11 a)
        const auto pole = [] {
            JsonWert p = jListe();
            p.liste.push_back(jText("nein"));
            p.liste.push_back(jText("ja"));
            return p;
        };
        std::set<std::string> imPaar;
        int nummer = 0;
        const auto punktId = [&nummer] {
            char z[16];
            std::snprintf(z, sizeof z, "p%03d", ++nummer);
            return std::string(z);
        };
        for (const Klang& mit : klaenge)
        {
            if (mit.fassung != "mit") continue;
            for (const Klang& ohne : klaenge)
                if (ohne.fassung == "ohne" && ohne.achsen == mit.achsen && !imPaar.count(ohne.id))
                {
                    JsonWert p = jObjekt();
                    p.setze("id", jText(punktId()));
                    p.setze("art", jText("paar"));
                    JsonWert ids = jListe();
                    ids.liste.push_back(jText(mit.id));
                    ids.liste.push_back(jText(ohne.id));
                    p.setze("klaenge", ids);
                    p.setze("frage", jText(mit.frage));
                    p.setze("pole", pole());
                    p.setze("vorzug", jText("Welcher klingt glaubhafter?"));
                    pu.liste.push_back(p);
                    imPaar.insert(mit.id);
                    imPaar.insert(ohne.id);
                    break;
                }
        }
        for (const Klang& k : klaenge)
        {
            if (imPaar.count(k.id)) continue;
            JsonWert p = jObjekt();
            p.setze("id", jText(punktId()));
            p.setze("art", jText("einzel"));
            p.setze("klang", jText(k.id));
            p.setze("frage", jText(k.frage));
            p.setze("pole", pole());
            pu.liste.push_back(p);
        }
        l.setze("klaenge", kl);
        l.setze("messwerte", mw);
        l.setze("punkte", pu);
        std::ofstream(ordner / "liste.json", std::ios::binary) << alsJson(l, 0) << '\n';
        std::ofstream sha(ordner / "klaenge.sha256", std::ios::binary);
        sha << "# Abdruck jedes Klangs (SHA-256 der WAV-Datei). Geschrieben mit: " << befehl << '\n';
        for (const Klang& k : klaenge) sha << k.abdruck << "  " << k.datei << '\n';
    }
}

std::vector<Eintrag>& weitere()
{
    static std::vector<Eintrag> l;
    return l;
}

}  // namespace ic::hoer
