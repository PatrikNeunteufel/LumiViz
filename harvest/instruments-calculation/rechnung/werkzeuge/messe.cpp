/**
 * @file messe.cpp
 * @brief Werkzeug (nur lokal): einen Datensatz unter `IC_SAMPLES` messen — Modalfit ohne Reihe (B6) oder die
 *        Teiltonmessung des Fluegels — und das Ergebnis als TSV ausgeben (AUFTRAG_BAU §0, §5)
 *
 *   ic_messe <Ordner des Datensatzes im Repo> [modalfit <Moden> | teiltoene] [ohne-pruefung]
 *   z. B.  IC_SAMPLES=/mnt/samples ic_messe "klassen/Klasse 4 - Platte und Stab/aufnahmen/Iowa_MIS" modalfit 8
 *
 * Der Ordner im Repo enthaelt `SHA256SUMS`; die Aufnahmen liegen unter `$IC_SAMPLES/<Klasse n - Name>/<Datensatz>/`
 * (A-1 a). Ohne `IC_SAMPLES` bricht das Werkzeug mit einer Meldung ab. Gelesen werden WAV (16/24-bit PCM, 32-bit
 * float); andere Formate (AIFF der Iowa-Saetze) werden gemeldet und uebersprungen — offen.
 */
#include "gemeinsam/Fft.hpp"
#include "gemeinsam/Modal.hpp"
#include "gemeinsam/Werkzeug.hpp"
#include "messung/Bausteine.hpp"
#include "messung/Samples.hpp"
#include "messung/Teiltonmessung.hpp"

#include <cmath>
#include <cstdio>
#include <string>

using namespace ic;
using namespace ic::messung;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "Aufruf: ic_messe <Ordner des Datensatzes im Repo> [modalfit <Moden> | teiltoene] [ohne-pruefung]\n");
        return 2;
    }
    std::string meldung;
    const auto wurzel = samplesWurzel(meldung);
    if (!wurzel)
    {
        std::fprintf(stderr, "ic_messe: %s\n", meldung.c_str());
        return 1;
    }
    const std::filesystem::path repo(argv[1]);
    const std::string art = argc > 2 ? argv[2] : "modalfit";
    const int moden = art == "modalfit" && argc > 3 ? std::atoi(argv[3]) : 8;
    bool pruefen = true;
    for (int i = 2; i < argc; ++i)
        if (std::string(argv[i]) == "ohne-pruefung") pruefen = false;
    // klassen/<Klasse>/(aufnahmen|daten)/<Datensatz> → $IC_SAMPLES/<Klasse>/<Datensatz>
    const std::filesystem::path datensatz = repo.filename();
    const std::filesystem::path klasse = repo.parent_path().parent_path().filename();
    const std::filesystem::path ort = *wurzel / klasse / datensatz;
    const std::vector<SummenEintrag> summen = liesSummen(repo / "SHA256SUMS");
    if (summen.empty())
    {
        std::fprintf(stderr, "ic_messe: keine SHA256SUMS unter %s\n", repo.string().c_str());
        return 1;
    }
    std::printf("# ic_messe %s an %s\n", art.c_str(), ort.string().c_str());
    std::printf("datei\tmode\thz\tt60_s\tpegel_db\n");
    for (const SummenEintrag& e : summen)
    {
        const std::filesystem::path datei = ort / e.pfad;
        if (datei.extension() != ".wav" && datei.extension() != ".WAV")
        {
            std::fprintf(stderr, "uebersprungen (Format offen): %s\n", e.pfad.c_str());
            continue;
        }
        if (pruefen && !pruefeDatei(datei, e.sha256))
        {
            std::fprintf(stderr, "Summe stimmt nicht oder Datei fehlt: %s\n", datei.string().c_str());
            continue;
        }
        std::vector<float> roh;
        int rate = 0, kanaele = 0;
        if (!leseWav(datei.string(), roh, rate, kanaele) || kanaele < 1) continue;
        std::vector<double> mono(roh.size() / static_cast<std::size_t>(kanaele), 0.0);
        for (std::size_t i = 0; i < mono.size(); ++i)
            for (int k = 0; k < kanaele; ++k) mono[i] += roh[i * static_cast<std::size_t>(kanaele) + static_cast<std::size_t>(k)] / kanaele;
        if (art == "modalfit")
        {
            // ab 20 ms nach der Spitze, 0,5 s
            std::size_t spitze = 0;
            for (std::size_t i = 1; i < mono.size(); ++i)
                if (std::abs(mono[i]) > std::abs(mono[spitze])) spitze = i;
            const std::size_t ab = spitze + static_cast<std::size_t>(0.02 * rate);
            for (const ModalfitMode& m : modalfit(mono, rate, ab, static_cast<std::size_t>(0.5 * rate), moden, 160))
                std::printf("%s\t-\t%.4f\t%.4f\t%.2f\n", e.pfad.c_str(), m.hz, m.sigma > 0.0 ? kLn1000 / m.sigma : 0.0, dB(m.amplitude));
        }
        else
        {
            std::fprintf(stderr, "teiltoene: Nennfrequenz aus dem Dateinamen ist offen — benutze modalfit\n");
            return 2;
        }
    }
    return 0;
}
