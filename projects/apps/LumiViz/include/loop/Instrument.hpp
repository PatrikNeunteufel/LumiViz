/**
 * @file Instrument.hpp
 * @brief Loop: gerechnete Instrumente — aus einem Satz Einstellungen und einer
 *        Tonhoehe wird ein Ton (docs/audio/Loop_Konzept.md §4f, Stufe 9,
 *        Entscheid I1 c)
 *
 * Ein Instrument besteht aus einer bis vier Schichten (Stufe 10c); jede ist
 * ein Satz Einstellungen fuer einen Klangerzeuger:
 * Schwingung (der Funktionsgenerator), Huellkurve, Filter, Vibrato,
 * Verstimmung — dazu die gezupfte Saite als eigene Art. `rendereTon` rechnet
 * daraus einen Ton von gegebener Dauer **vorab** in einen `Klang`; der
 * Rechenkern spielt ihn wie den Schlag eines Drumkits. So bleibt der Kern
 * ohne Gedaechtnis, obwohl Filter und Saite eines haben.
 *
 * Derselbe Aufruf ergibt dieselben Werte: kein Zufall von aussen (das
 * Rauschen der Saite kommt aus einer festen Folge je Tonhoehe).
 *
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/Bausteine.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Saiten.hpp"

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <cstdint>
#include <variant>
#include <vector>

namespace lumi::loop
{

/// Die Schwingung eines Instruments. Die vier ersten sind ein Wellenzug ohne
/// Teiltoene ueber der halben Abtastrate; die Saite ist eine gezupfte Saite
/// (Karplus-Strong), die von selbst ausklingt. Das Sample ist eine Aufnahme,
/// je Note schneller oder langsamer abgespielt (Stufe 11a). Das Rauschen hat
/// keine Tonhoehe: eine feste Folge, bei jeder Note dieselbe (Stufe 11d).
/// Die Form ist die Bewegung eines Stifts um eine gezeichnete Kontur
/// (`shape/Tonform.hpp`; Stufe 19c, docs/audio/Shape_Konzept.md §5).
enum class Schwingung : int
{
    Sinus = 0,
    Dreieck,
    Saege,
    Rechteck,
    Saite,
    Sample,
    Rauschen,
    Form,
};

/// Das Verfahren einer Saite (Stufe 23c; Entscheide Patrik S132: S-K1 a + b,
/// S-K2 b). *Classic* ist die Saite bis Stufe 22, unberuehrt (Damping);
/// *Extended KS* das erweiterte Karplus-Strong nach Jaffe und Smith mit den
/// Reglern nach der Physik (S-K3 a). Der Waveguide kommt mit Stufe 23d.
enum class SaitenModell : int
{
    Klassisch = 0,
    ErweiterterKs = 1,
    /// Waveguide (Stufe 23d, S-K1 a): die Saite als zwei gegenlaeufige Wellen
    /// zwischen Sattel und Steg — man kann sie an jeder Stelle abnehmen.
    Wellenleiter = 2,
};

/// Die Bauart eines Tonabnehmers (Stufe 23d, S-K5 a).
enum class AbnehmerArt : int
{
    Keiner = 0,     ///< None (acoustic): kein Tonabnehmer an diesem Platz
    SingleCoil = 1, ///< eine Spule: tastet an einer Stelle ab
    Humbucker = 2,  ///< zwei Spulen, `kHumbuckerAbstandMm` auseinander
    Piezo = 3,      ///< unter dem Steg: nimmt die Kraft am Steg ab
};

/// Pickup select: welcher Tonabnehmer klingt.
enum class AbnehmerWahl : int
{
    Hals = 0,   ///< Neck
    Beide = 1,  ///< Both
    Steg = 2,   ///< Bridge
};

/// Der Abstand der beiden Spulen eines Humbuckers (S-U3 a; Zollner, *Physics
/// of the Electric Guitar* 5.4.4: Gibson-Humbucker 18 bis 19 mm).
inline constexpr double kHumbuckerAbstandMm = 18.0;

/// Wie die Bauart in einer Datei steht: `none`, `singleCoil`, `humbucker`, `piezo`.
[[nodiscard]] inline QLatin1String abnehmerArtName(AbnehmerArt art)
{
    switch (art)
    {
    case AbnehmerArt::SingleCoil: return QLatin1String("singleCoil");
    case AbnehmerArt::Humbucker: return QLatin1String("humbucker");
    case AbnehmerArt::Piezo: return QLatin1String("piezo");
    case AbnehmerArt::Keiner: break;
    }
    return QLatin1String("none");
}
/// Fehlt die Bauart oder ist sie unbekannt: keiner.
[[nodiscard]] inline AbnehmerArt abnehmerArtAusName(const QString& name)
{
    if (name == QLatin1String("singleCoil")) return AbnehmerArt::SingleCoil;
    if (name == QLatin1String("humbucker")) return AbnehmerArt::Humbucker;
    if (name == QLatin1String("piezo")) return AbnehmerArt::Piezo;
    return AbnehmerArt::Keiner;
}

/// Body: der Korpus hinter dem Steg (Stufe 23c/23d; Entscheide S-K4 a,
/// S-K7 a, S-K8 a). Nur Korpusse, deren Werte belegt sind
/// (docs/audio/Saiten_Konzept.md §6, §6b).
enum class KorpusArt : int
{
    Keiner = 0,  ///< None: die Saite allein
    /// Acoustic guitar (Parlor): das gekoppelte Modell aus Decke und Luft nach
    /// Christensen und Vistisen mit den Werten aus D. P. Hess, „Frequency
    /// Response Evaluation of Acoustic Guitar Modifications“, *Savart
    /// Journal* 1(3), 2013, Tab. 1 (eine Loar LH-200).
    Parlor = 1,
    /// Classical guitar: dasselbe Modell mit elf Moden aus Decke und Boden nach
    /// H. Wright, *The Acoustics and Psychoacoustics of the Guitar*, Diss.
    /// Cardiff 1996, Satz TOP26 (S. 138–140, Tab. 7.1; Entscheid S-K10 a).
    Klassik = 2,
};

/// Die Impulsantwort eines Korpus (fuer Tests und Bilder): was er abstrahlt,
/// wenn am Steg ein einzelner Kraftstoss angreift — `frames` Abtastwerte,
/// auf den groessten Betrag 1 bemessen. Leer = kein Korpus.
[[nodiscard]] std::vector<double> korpusImpulsantwort(KorpusArt art, int rate, int frames);

/// Cable: die Kapazitaet von Kabel und Last, in pF (Stufe 23d; Zollner,
/// *Physics of the Electric Guitar* 9.4: ueblich 300 bis 600 pF).
inline constexpr double kKabelMinPf = 100.0;
inline constexpr double kKabelMaxPf = 2000.0;
inline constexpr double kKabelVorgabePf = 500.0;

/// Die Resonanz eines magnetischen Tonabnehmers mit Kabel und Last
/// (S-K5 a, S-K7 a): gemessen je Bauart bei einer Bezugskapazitaet und mit
/// der Kapazitaet umgerechnet, f ∝ 1/√C (ein Schwingkreis aus Spule und
/// Kapazitaet); die Guete bleibt. Belege in docs/audio/Saiten_Konzept.md §6b:
/// - Single coil: 3,5 kHz bei 600 pF und 111 kOhm, Ueberhoehung +4 dB, also
///   Q ≈ 1,58 nach 20·lg Q (Zollner 5.5.4, Abb. 5.5.9 — abgelesen);
/// - Humbucker: ≈ 3 kHz bei 450 pF und 333 kOhm (Abb. abgelesen), Q = 1,83
///   (Zollner/Zwicker, „Factoring in the Q“, GITEC 2019, S. 6 — wörtlich).
struct Resonanz
{
    double hz = 0.0;     ///< 0 = keine (kein magnetischer Tonabnehmer)
    double guete = 0.0;
};
[[nodiscard]] Resonanz abnehmerResonanz(AbnehmerArt art, double kabelPf);

/// Ein Tonabnehmer an seinem Platz (Neck oder Bridge).
struct Abnehmer
{
    AbnehmerArt art = AbnehmerArt::Keiner;
    /// Position: so weit vom Steg, in mm (bei Piezo zaehlt sie nicht).
    double mmVomSteg = 0.0;

    [[nodiscard]] bool operator==(const Abnehmer&) const = default;
};

/// Die Grenzen der Regler der neuen Saite (Stufe 23c).
inline constexpr double kAnschlagOrtMin = 0.02;  ///< Pluck position: nah am Steg
inline constexpr double kAnschlagOrtMax = 0.5;   ///< die Mitte der Saite
inline constexpr double kKlingSekundenMin = 0.1;
inline constexpr double kKlingSekundenMax = 30.0;

/// Die Grenzen der Shape-Schicht (Stufe 19c): Harmonics und der Betrag, um den
/// die Huellkurve sie hebt oder senkt.
inline constexpr double kFormHarmonischeMax = 512.0;
inline constexpr double kFormHuelleMax = 128.0;

/// Pitch sweep (Stufe 11d, Entscheid N1): so weit neben ihrer Tonhoehe darf
/// eine Note beginnen, in Halbtoenen — darueber (sie faellt) oder darunter
/// (sie steigt) — und so lang darf der Weg dauern.
inline constexpr double kTonhoeheStartMin = -24.0;
inline constexpr double kTonhoeheStartMax = 48.0;
inline constexpr double kTonhoeheZeitMaxMs = 2000.0;

// --- der Weg des Tons einer Schicht (Stufe 16a; Entscheide Patrik S117:
// K1 a eine Reihe, K2 a die Bausteine, K4 a hoechstens acht) ---------------
// Zwischen dem, was den Ton erzeugt (Source: Schwingung, Stimmen, Sample,
// Pitch sweep, Vibrato), und der Lautstaerke der Schicht (Level) steht eine
// Reihe von Bausteinen: Filter und Drive beliebig oft, Envelope genau einmal.

/// Filter: folgt der Tonhoehe. Art und Resonanz seit Stufe 11e (Entscheid
/// H1 b, Patrik S116).
struct SchichtFilter
{
    FilterArt art = FilterArt::Tiefpass;
    double teilton = 0.0;  ///< Cutoff: die Grenze als Vielfaches des Grundtons, 0…64; 0 = offen
    double huelle = 0.0;   ///< Filter envelope: so viele Vielfache oeffnet die Huellkurve der Schicht dazu
    /// Resonance, 0…1: die Betonung an der Grenze; beim Bandpass die Breite
    /// (mehr = schmaler). Ein Tiefpass mit 0 ist der Filter bis Stufe 11d,
    /// unveraendert — darueber rechnet er wie Hochpass und Bandpass.
    double resonanz = 0.0;

    [[nodiscard]] bool operator==(const SchichtFilter&) const = default;
};

/// Drive: der Verzerrer je Ton (Entscheid E1 a).
struct SchichtDrive
{
    double wert = 0.0;  ///< 0…1, 0 = aus

    [[nodiscard]] bool operator==(const SchichtDrive&) const = default;
};

/// Envelope: die Stelle, an der die Huellkurve den Ton formt. Ihre Werte
/// (Attack, Decay, Sustain, Release) stehen an der Schicht — es gibt sie genau
/// einmal, und die Filter envelope jedes Filters folgt ihr.
struct SchichtHuelle
{
    [[nodiscard]] bool operator==(const SchichtHuelle&) const = default;
};

using SchichtBaustein = std::variant<SchichtFilter, SchichtDrive, SchichtHuelle>;

/// Eine Reihe richten: genau eine Envelope (fehlt sie, kommt sie ans Ende;
/// von mehreren bleibt die erste), hoechstens `kBausteineMax` — was dahinter
/// steht, faellt weg, die Envelope nie —, jeder Wert in seinen Grenzen.
void richteReihe(std::vector<SchichtBaustein>& reihe);

/// Eine Schicht eines Instruments: ein voller Satz Einstellungen fuer den
/// Klangerzeuger, dazu was nur die Schicht betrifft (Stufe 10c; Skizze
/// gebilligt, Patrik S111). Die Schichten eines Instruments werden beim
/// Rechnen eines Tons addiert.
struct Schicht
{
    QString name;                ///< wie sie in der Liste steht; leer = „Layer n"
    bool an = true;              ///< aus = stumm, bleibt aber im Instrument
    int versatzHalbtoene = 0;    ///< -24…24: so weit liegt die Schicht neben der Note
    double einsatzMs = 0.0;      ///< 0…1000: so lange nach dem Anschlag setzt sie ein (kein Echo)

    /// Der Weg des Tons zwischen Source und Level, von vorn nach hinten. Eine
    /// neue Schicht hat nur die Envelope; eine Datei bis Fassung 6 liest sich
    /// als Filter, Drive, Envelope — so wurde bis dahin fest gerechnet.
    std::vector<SchichtBaustein> reihe = {SchichtHuelle{}};

    Schwingung schwingung = Schwingung::Sinus;
    /// Nur Sinus: die Anteile des 2., 3., … Teiltons (der Grundton zaehlt 1).
    std::vector<double> obertoene;
    int stimmen = 1;               ///< 1…3 gleiche Schwingungen uebereinander
    double verstimmungCent = 0.0;  ///< so weit liegen die aeusseren Stimmen neben dem Ton

    // --- Huellkurve ---
    double anstiegMs = 5.0;    ///< bis zur vollen Lautstaerke
    double abfallMs = 200.0;   ///< danach bis auf `halten`
    double halten = 0.7;       ///< 0…1, solange die Note dauert
    double ausklangMs = 150.0; ///< nach dem Ende der Note

    // --- Vibrato ---
    double vibratoHz = 0.0;
    double vibratoCent = 0.0;
    double vibratoEinsatzMs = 0.0;  ///< so lange waechst es an

    double saiteDaempfung = 0.996;  ///< nur Saite *Classic*: 0.9…0.9999, groesser klingt laenger

    // --- nur Saite, Verfahren und Regler nach der Physik (Stufe 23c, S-K3 a) ---
    /// Model: eine Datei bis Fassung 9 (und ohne das Feld) ist *Classic*.
    SaitenModell saitenModell = SaitenModell::Klassisch;
    /// Pluck position: wo angeschlagen wird, als Anteil der Saite vom Steg aus
    /// (`kAnschlagOrtMin` … `kAnschlagOrtMax`). Bei 1/n fehlt der n-te Teilton.
    double anschlagOrt = 0.13;
    /// Pluck: die Form des Anschlags, 0 (weich, Finger) … 1 (hart, Plektrum).
    double anschlagHaerte = 0.65;
    /// Ring time: so lange, bis der Grundton um 60 dB gefallen ist, in Sekunden
    /// — fuer jede Tonhoehe gleich (`kKlingSekundenMin` … `kKlingSekundenMax`).
    double klingSekunden = 3.2;
    /// High damping, 0 … 1: um wie viel schneller die Hoehen vergehen.
    double hoehenDaempfung = 0.3;
    /// Stiffness, 0 … 1: wie steif die Saite ist — die Obertoene liegen hoeher
    /// als ein Vielfaches des Grundtons.
    double steifheit = 0.1;
    /// String variation, 0 … 1: wie stark sich die Saiten des Instruments
    /// unterscheiden — eine tiefere Saite ist dicker: dunkler und steifer.
    /// 0 = alle gleich; ohne Saiten am Instrument wirkt es nicht.
    double saitenUnterschied = 0.5;
    // --- nur Waveguide: die Tonabnehmer (Stufe 23d, S-K5 a, S-U3 a) ---
    /// Neck und Bridge; beide *None* = der Ton wird am Steg abgenommen, wie
    /// eine akustische Saite. Vorgaben: die Lagen aus Zollner 2.8.2 (16 cm und
    /// 4 cm vom Steg).
    Abnehmer abnehmerHals{AbnehmerArt::Keiner, 160.0};
    Abnehmer abnehmerSteg{AbnehmerArt::Keiner, 40.0};
    AbnehmerWahl abnehmerWahl = AbnehmerWahl::Beide;
    /// Cable, in pF (`kKabelMinPf` … `kKabelMaxPf`): fuer beide Tonabnehmer;
    /// er legt die Resonanz der magnetischen fest (`abnehmerResonanz`).
    double kabelPf = kKabelVorgabePf;
    // --- Extended KS und Waveguide: der Korpus (S-K3 a, S-K8 a) ---
    /// Body: wirkt auf den Ton, der am Steg abgenommen wird — ohne
    /// Tonabnehmer, und bei Extended KS immer; ein Tonabnehmer nimmt die Saite
    /// selbst ab, vor dem Korpus.
    KorpusArt korpus = KorpusArt::Keiner;
    /// Body amount, 0 … 1: so viel des Tons kommt durch den Korpus, der Rest
    /// ist die Saite allein.
    double korpusAnteil = 0.6;
    double laut = 1.0;              ///< die Lautstaerke der Schicht
    /// Pan, -1 (links) … 1 (rechts); 0 = Mitte (Stufe 18, Entscheid Z3 a).
    /// Dasselbe Gesetz wie an Spur und Muster (Rechenkern): links
    /// `min(1, 1 - pan)`, rechts `min(1, 1 + pan)` — in der Mitte beide
    /// Seiten voll, zur Seite hin wird die andere leiser. Gemischt wird
    /// nichts: jede Seite wird mit ihrem Faktor genommen.
    double pan = 0.0;

    // --- nur Sample (Stufe 11a; Skizze gebilligt, Patrik S115) ---
    /// Die Aufnahme: ein eigenes Sample (`user:<name>`, Sample.hpp) oder der
    /// Pfad einer Tondatei; leer = keine, die Schicht schweigt.
    QString sample;
    int grundton = 60;              ///< Root note: dieser Ton ist die Aufnahme (0…127)
    double feinCent = 0.0;          ///< Fine tune, -100…100: so weit liegt die Aufnahme ueber dem Grundton
    double sampleStartMs = 0.0;     ///< Sample start: ab hier in der Aufnahme
    bool schleife = false;          ///< Sustain loop: ein Stueck wiederholt sich, solange die Note dauert
    double schleifeVonMs = 0.0;     ///< Loop from, in der Aufnahme
    double schleifeBisMs = 0.0;     ///< Loop to; nicht groesser als von = keine Schleife

    // --- Pitch sweep (Stufe 11d; Entscheide N1 und N2, Patrik S116) ---
    // Fuer Sinus, Dreieck, Saege, Rechteck und Sample; Saite und Rauschen kennen ihn nicht.
    /// So viele Halbtoene neben ihrer Tonhoehe beginnt jede Note: groesser als
    /// null darueber (sie faellt auf den Ton), kleiner darunter (sie steigt);
    /// 0 = aus. `kTonhoeheStartMin` … `kTonhoeheStartMax`.
    double tonhoeheStartHt = 0.0;
    /// Nach dieser Zeit ist die Tonhoehe angekommen (bis auf ein Zwanzigstel
    /// des Wegs, wie Decay); 0 = kein Weg, die Note beginnt auf ihrem Ton.
    double tonhoeheZeitMs = 100.0;

    // --- nur Form (Stufe 19c; Entscheide Z2 a, Z4 a, G1 a, T1 a, T2 a) ---
    // Stimmen, Verstimmung, Vibrato und Pitch sweep gelten wie bei Sinus und Saege.
    /// Die Form: eine mitgelieferte beim Namen (`heart`), eine eigene
    /// (`user:<name>`) oder der Pfad einer `.lvcontour`; leer oder nicht zu
    /// finden = die Schicht schweigt.
    QString form;
    /// Contour: die Kennung einer Kontur der Datei — oder 0 = alle mit Haken,
    /// nacheinander in einem Umlauf (All, one path).
    int formKontur = 0;
    /// Harmonics, 1 … `kFormHarmonischeMax`, stufenlos: so viele Teiltoene
    /// zeichnen die Form. Eine hohe Note bekommt von selbst weniger.
    double harmonische = 24.0;
    /// Harmonics envelope, −`kFormHuelleMax` … `kFormHuelleMax`: um so viele
    /// Teiltoene hebt (oder senkt) die Huellkurve der Schicht Harmonics.
    double harmonischeHuelle = 0.0;
    /// Angle, 0 … 360 Grad: dreht die Form, bevor der Ton abgenommen wird
    /// (0 = die x-Bewegung des Stifts, 90 = die y-Bewegung).
    double winkelGrad = 0.0;
    double formStartProzent = 0.0;  ///< Start, 0 … 100: hier im Umlauf beginnt jede Note
    /// Stereo XY: links die x-Bewegung der (gedrehten) Form, rechts ihre
    /// y-Bewegung; sonst auf beiden Seiten die x-Bewegung.
    bool stereoXY = false;
    bool rueckwaerts = false;  ///< Reverse: der Stift laeuft andersherum

    /// Die eigene Farbe der Schicht (`#rrggbb`, F1 a, Stufe 20d); leer = die
    /// abgeleitete (`schichtFarbe`).
    QString farbe;

    // --- nur Saite Extended KS und Waveguide: Playing (Stufe 24a, S-W6 a) ---
    // Je Spielweise eine Abweichung der vorhandenen Groessen, jeder Regler
    // genau eine. Die Vorgaben sind Platzhalter (von Claude gesetzt); sie werden
    // in Stufe 24c hergeleitet und belegt.
    /// Up level, 0 … 1: so laut schlaegt die Hand nach oben an wie nach unten
    /// (das Vorzeichen des Anschlags dreht sich dabei immer um).
    double aufschlagLaut = 0.8;
    /// Mute, 0 … 1: wie stark der Handballen bei *Palm mute* am Steg daempft.
    double handDaempfung = 0.6;
    /// Tap level, 0 … 1: so laut ist ein Ton beim *Tapping* (am Bund angeschlagen).
    double tappingLaut = 0.65;
    /// Touch, 0 … 1: wie fest der Finger beim *Harmonic* auf dem Knoten liegt.
    double knotenDruck = 0.3;

    [[nodiscard]] bool operator==(const Schicht&) const = default;

    // --- der erste Filter und der erste Drive ---------------------------
    // Kurzwege fuer den, der eine Reihe von Hand baut oder nachsieht (Tests):
    // der erste Filter und der erste Drive, in der alten Folge Filter, Drive,
    // Envelope. Creator und Klangerzeuger arbeiten seit Stufe 16 mit der
    // ganzen Reihe.

    /// Der erste Filter der Reihe — oder die Vorgabe (offen), wenn es keinen gibt.
    [[nodiscard]] SchichtFilter filter() const;
    /// Der erste Drive der Reihe — oder die Vorgabe (aus).
    [[nodiscard]] SchichtDrive drive() const;
    /// Der erste Filter zum Aendern; fehlt er, kommt er vorn dazu.
    SchichtFilter& holeFilter();
    /// Der erste Drive zum Aendern; fehlt er, kommt er vor die Envelope.
    SchichtDrive& holeDrive();
};

/// So viele Schichten traegt ein Instrument hoechstens: jede wird je Ton
/// einzeln gerechnet. Bis Stufe 17 vier; seit Stufe 18 acht (Entscheid
/// Patrik S120, davor gemessen: die Rechenzeit waechst gerade mit der Zahl,
/// test_LoopBausteine.cpp „Loop Instrument Messung") — mit Fassung 8 der
/// Datei, weil ein aelteres Programm eine laengere Liste still kuerzte.
inline constexpr int kSchichtenMax = 8;

/// Eine Art von Instrument: so ist das Fenster „Instruments" gegliedert
/// (Stufe 14, Entscheid P1 a; Skizze gebilligt, Patrik S114).
struct InstrumentArt
{
    const char* schluessel;  ///< steht in der Datei (`category`)
    const char* titel;       ///< wie es im Programm steht
};

/// Die neun Arten in der Reihenfolge des Fensters (seit S115 mit „Drums",
/// Entscheid K3 a); die letzte ist „Unusual".
[[nodiscard]] const std::vector<InstrumentArt>& instrumentArten();
/// Der Schluessel einer Art — oder `unusual`, wenn es `art` nicht gibt (auch leer).
[[nodiscard]] QString gueltigeArt(const QString& art);
/// Der Titel einer Art („Pads and strings").
[[nodiscard]] QString artTitel(const QString& art);

struct Instrument
{
    /// Wie ein Muster es nennt: der Name eines mitgelieferten Instruments
    /// (`piano` — die Datei `asset/loop/instruments/piano.lvinst`), ein
    /// eigenes aus dem Benutzerordner (`user:mein_bass`, Entscheid P2 a) oder
    /// der Pfad einer Datei `.lvinst` an freiem Ort. Steht nicht in der Datei.
    QString name;
    QString titel;      ///< wie es im Programm steht
    double laut = 1.0;  ///< die Lautstaerke des ganzen Instruments
    /// Mindestens eine, hoechstens `kSchichtenMax`.
    std::vector<Schicht> schichten = {Schicht{}};
    // --- seit Fassung 3 (Entscheid F3 b): fuer das Fenster „Instruments"
    /// Die Art (`keys`, `bass`, …). Leer oder unbekannt gilt als `unusual` —
    /// in der Datei bleibt stehen, was dort stand.
    QString art;
    /// Wo es hingehoert („Funk", „Disco"); leer = nur unter „All".
    QStringList genres;
    QString beschreibung;  ///< ein Satz
    /// Die Saiten (seit Fassung 10, Stufe 23a, Entscheid S-M1 a): Stimmung,
    /// Capo und die Vorgabe der Regel fuer neue Muster. Ohne Saiten
    /// (`istSaiteninstrument()` = false) ist das Instrument wie bis Stufe 22.
    /// Die Schichten bleiben der Klang.
    Saitensatz saiten;

    [[nodiscard]] bool operator==(const Instrument&) const = default;
};

// --- die Datei `.lvinst` (Entscheide Patrik S110, I2 b und I3 a; S111, F1 a) --
// Ein Instrument ist eine Datei, abgelegt wie die Klaenge des Kits: die
// mitgelieferten unter `asset/loop/instruments`, eigene irgendwo. Im Programm
// steht kein Instrument mehr.
//
// Fassung 2 traegt die Schichten als Liste `layers`; Fassung 1 ist flach und
// wird als Instrument mit einer Schicht gelesen. Fassung 3 (S114, Entscheid
// F3 b) traegt dazu `category`, `genres` und `description`; alle drei
// duerfen fehlen. Fassung 4 (S115, Stufe 11a) kennt die Schwingung `sample`
// mit `sample`, `rootNote`, `fineTune`, `sampleStart`, `sustainLoop`,
// `loopFrom`, `loopTo` — eine eigene Fassung, weil ein aelteres Programm eine
// solche Schicht sonst still als Sinus spielte. Fassung 5 (S116, Stufe 11d)
// kennt die Schwingung `noise` und an jeder Schicht `pitchStart` (Halbtoene)
// und `pitchTime` (ms) — aus demselben Grund eine eigene Fassung: ein aelteres
// Programm spielte Rauschen als Sinus und einen fallenden Ton gerade.
// Fassung 6 (S116, Stufe 11e) kennt an jeder Schicht `filterType` (`lowpass`,
// `highpass`, `bandpass`) und `filterResonance` — ein aelteres Programm
// spielte einen Hochpass als Tiefpass. Fassung 7 (S118, Stufe 16a) traegt an
// jeder Schicht die Reihe `path` — Eintraege mit `type`: `filter` (mit
// `filterType`, `cutoff`, `envelope`, `resonance`), `drive` (mit `amount`),
// `envelope` — statt der flachen Felder `drive`, `filterCutoff`,
// `filterEnvelope`, `filterType`, `filterResonance`; Attack, Decay, Sustain
// und Release bleiben an der Schicht. Eine Datei bis Fassung 6 liest sich als
// Filter, Drive, Envelope; ein Filter mit Cutoff 0 und ein Drive mit 0 werden
// dabei kein Baustein (K2 a). Fassung 8 (S120, Stufe 18) kennt an jeder
// Schicht `pan` (-1 … 1) und bis zu acht Schichten statt vier — eine eigene
// Fassung, weil ein aelteres Programm eine Schicht sonst still in der Mitte
// spielte und die fuenfte bis achte wegliesse. In einer aelteren Datei
// fehlt `pan`: die Schicht steht in der Mitte. Fassung 9 (S121, Stufe 19c)
// kennt die Schwingung `shape` mit `shape` (die Form), `shapeContour`,
// `shapeHarmonics`, `shapeHarmonicsEnvelope`, `shapeAngle`, `shapeStart`,
// `shapeStereoXY`, `shapeReverse` — eine eigene Fassung, weil ein aelteres
// Programm eine solche Schicht sonst still als Sinus spielte. (`harmonics`
// allein ist seit Fassung 1 die Liste der Obertoene des Sinus.) Seit Stufe 20d
// (S122, Entscheid V1 a) kann eine Schicht `color` tragen, ihre eigene Farbe —
// ohne neue Fassung: ein aelteres Programm ueberliest das Feld und zeigt die
// abgeleitete Farbe; der Klang aendert sich nicht. Fassung 10 (S132, Stufe 23a,
// Entscheid S-M1 a) kennt am Instrument den Block `strings` mit `open` (die
// Toene der leeren Saiten als MIDI-Nummern), `capo` und `stringChoice`
// (`perChord`, `followHand`); ein Instrument ohne Saiten traegt ihn nicht.
// Eine eigene Fassung, weil ein aelteres Programm die Saiten still ueberginge.
// In derselben Fassung (Stufe 23c, noch nicht ausgeliefert) traegt eine Saite
// mit `stringModel` `extendedKS` dazu `pluckPosition`, `pluck`, `ringTime`,
// `highDamping`, `stiffness`, `stringVariation`; ohne `stringModel` ist sie
// *Classic* und traegt nur `damping` wie bisher. Fassung 11 (S134, Stufe 24a,
// Entscheide S-W2 a, S-W6 a) kennt unter `strings` den Block `strum` (`spread`,
// `hand`, `pattern` — die Vorgabe fuer neue Muster, nur wenn sie von der des
// Programms abweicht) und an einer Saite *Extended KS* oder *Waveguide* den
// Block `playing` mit `upLevel`, `mute`, `tapLevel`, `touch`. Eine eigene
// Fassung, weil ein aelteres Programm die Spielweisen still ueberginge.
// Geschrieben wird Fassung 11.

/// Die Fassung, die dieses Programm schreibt; eine hoehere liest es nicht.
inline constexpr int kInstrumentFassung = 11;

/// Das Instrument als JSON (ohne `name` — der ist, wie das Muster es nennt).
[[nodiscard]] QJsonObject alsJson(const Instrument& instrument);
/// Liest ein Instrument, Werte in ihren Grenzen. false = kein Instrument
/// (falsches Format) oder eine Fassung, die dieses Programm nicht kennt — dann
/// ist `zuNeu` gesetzt; `instrument` bleibt, wie es war. `name` bleibt.
bool ausJson(const QJsonObject& wurzel, Instrument& instrument, bool* zuNeu = nullptr);
bool speichereInstrument(const Instrument& instrument, const QString& pfad);
/// Liest die Datei; `name` wird der Pfad. Ohne Titel in der Datei gilt ihr Name.
bool ladeInstrument(const QString& pfad, Instrument& instrument, bool* zuNeu = nullptr);

/// Der Ordner der mitgelieferten Instrumente: `asset/loop/instruments`, neben
/// dem Kit (`kitOrdner`). Leer = nicht gefunden.
[[nodiscard]] QString instrumentOrdner();
/// Die mitgelieferten Instrumente — was im Ordner als `.lvinst` liegt, nach
/// Titel sortiert; `name` ist der Dateiname ohne Endung. Einmal gelesen und
/// gemerkt. Leer = der Ordner ist nicht da.
[[nodiscard]] const std::vector<Instrument>& instrumente();
/// Das mitgelieferte Instrument mit diesem Namen; nullptr = keins.
[[nodiscard]] const Instrument* findeInstrument(const QString& name);
/// Das Instrument, das ein Muster nennt: ein mitgeliefertes beim Namen, ein
/// eigenes als `user:<name>` oder eine Datei beim Pfad (`istInstrumentDatei`).
/// false = nicht gefunden oder nicht lesbar; `zuNeu` = die Datei traegt eine
/// Fassung, die dieses Programm nicht kennt.
bool holeInstrument(const QString& nennung, Instrument& instrument, bool* zuNeu = nullptr);

// --- eigene Instrumente im Benutzerordner (Entscheid P2 a, Patrik S114) -----
// Ein eigenes Instrument liegt als `<name>.lvinst` im Benutzerordner der App
// unter `instruments`, und ein Muster nennt es als `user:<name>`. Das ist ein
// eigener Raum: ein eigenes `piano` verdraengt das mitgelieferte nicht, ein
// Loop klingt weiter, wie er gespeichert wurde. Eine Datei an freiem Ort
// steht wie bisher beim Pfad im Muster.

inline constexpr const char* kEigenVorsatz = "user:";

/// Nennt `nennung` ein eigenes Instrument (`user:<name>`)?
[[nodiscard]] bool istEigenesInstrument(const QString& nennung);
/// Der Ordner der eigenen Instrumente (wird nicht angelegt); leer = Qt nennt
/// keinen Benutzerordner.
[[nodiscard]] QString eigenerInstrumentOrdner();
/// Nur fuer Tests: ein anderer Ordner; leer = wieder der echte.
void setzeEigenenInstrumentOrdnerFuerTests(const QString& ordner);
/// Aus einem Titel der Name einer eigenen Datei: klein, `a`–`z`, Ziffern und
/// `_` („Mein Bass 2" -> `mein_bass_2`). Leer = daraus wird kein Name.
[[nodiscard]] QString eigenerName(const QString& titel);
/// Ist `name` ein Name, wie `eigenerName` ihn bildet? Nur solche werden im
/// Benutzerordner gesucht — ein Muster kann so nichts ausserhalb nennen.
[[nodiscard]] bool istEigenerName(const QString& name);
/// Die Datei eines eigenen Instruments; leer = kein gueltiger Name oder kein Ordner.
[[nodiscard]] QString eigeneDatei(const QString& name);
/// Die eigenen Instrumente, nach Titel sortiert; `name` ist `user:<name>`.
/// Jedes Mal neu gelesen — der Ordner aendert sich, waehrend das Programm laeuft.
[[nodiscard]] std::vector<Instrument> eigeneInstrumente();
/// Die Titel der eigenen Instrumente, in denen eine Sample-Schicht dieses
/// Sample nennt (`user:<name>`, `kit:<name>` oder ein Pfad) — nach Titel
/// sortiert, jedes einmal. Fuer die Rueckfrage, bevor ein Sample ersetzt wird
/// (Stufe 17, Entscheid M3 a). Loops an freiem Ort kennt sie nicht.
[[nodiscard]] QStringList eigeneInstrumenteMitSample(const QString& nennung);
/// Dasselbe fuer eine Form: die Titel der eigenen Instrumente, in denen eine
/// Shape-Schicht diese Form nennt (Stufe 20a, „Used by" im Fenster Edit shape).
[[nodiscard]] QStringList eigeneInstrumenteMitForm(const QString& nennung);

/// Die Farbe der Schicht auf `platz` (F1 a, Patrik S122): ihre eigene, sonst
/// abgeleitet — eine Familie je Waveform in vier Abstufungen (die n-te
/// Schicht derselben Art bekommt die n-te), eine Shape-Schicht die Farbe ihrer
/// Kontur aus der Form (bei All, one path die der ersten angehakten; die Form
/// wird je Fassung der Zeichnungen einmal gelesen). Ungueltig = es gibt den Platz nicht.
[[nodiscard]] QColor schichtFarbe(const Instrument& instrument, int platz);
/// Die abgeleitete Farbe allein, ohne die eigene (fuer „Automatic").
[[nodiscard]] QColor abgeleiteteSchichtFarbe(const Instrument& instrument, int platz);
/// Ein eigenes Instrument in den Benutzerordner schreiben (legt ihn an).
/// `instrument.name` wird `user:<name>`. false = kein gueltiger Name oder
/// nicht zu schreiben.
bool speichereEigenes(Instrument& instrument, const QString& name);
/// Ein eigenes Instrument loeschen. false = es gab keins.
bool loescheEigenes(const QString& name);
/// Eine `.lvinst` von irgendwo zu den eigenen kopieren (Entscheid E3 a, S117;
/// Vorbild `nimmZuEigenen` der Samples): sie heisst nach ihrer Datei, bei
/// einem vergebenen Namen mit einer Zahl dahinter; liegt dieselbe Datei schon
/// da — auch: sie wurde aus dem Ordner selbst gewaehlt —, wird nichts kopiert.
/// Zurueck kommt `user:<name>`; leer = nicht zu lesen oder nicht zu schreiben.
[[nodiscard]] QString nimmZuEigenenInstrumenten(const QString& quelle);

/// Jedes Genre, das ein mitgeliefertes oder eigenes Instrument traegt — einmal,
/// nach dem Alphabet. Fuer den Filter im Fenster und die Vorschlaege im Creator.
[[nodiscard]] QStringList bekannteGenres();
/// Zaehlt jede gespeicherte Instrument-Datei (`speichereInstrument`): wer
/// Toene gerechnet hat und sich die Zahl merkt, sieht, ob er neu rechnen muss.
[[nodiscard]] std::uint64_t instrumentFassung();
/// Weiterzaehlen, ohne ein Instrument zu schreiben: ein Sample ist dazugekommen,
/// ersetzt oder geloescht (Sample.hpp) — Toene, die es spielen, sind neu zu rechnen.
void zaehleInstrumentFassung();

/// So viele Obertoene traegt ein Instrument hoechstens (der 2. bis 17. Teilton).
inline constexpr int kObertoeneMax = 16;

/// Die Frequenz einer Tonhoehe (MIDI-Nummer): 69 = 440 Hz.
[[nodiscard]] double tonHz(int hoehe);
/// Der Name einer Tonhoehe: 60 = „C4", 61 = „C#4".
[[nodiscard]] QString tonName(int hoehe);

/// Ein Ton laenger als das wird abgeschnitten (wie ein Sample, `kKlangMaxSekunden`).
inline constexpr double kTonMaxSekunden = 30.0;

/// Einen Ton rechnen: `dauerMs` lang gehalten, danach der Ausklang des
/// Instruments; der Klang endet bei null. Stereo; die Seiten sind gleich,
/// solange keine Schicht ein Pan neben der Mitte traegt (Stufe 18).
/// Die eingeschalteten Schichten werden addiert — jede an ihrer Tonhoehe und
/// ab ihrem Einsatz; der Klang ist so lang wie die laengste. Ist keine an,
/// ist er still. Leer = `rate` oder `dauerMs` nicht groesser als 0.
/// Eine Sample-Schicht spielt, was zu ihrer Aufnahme gemerkt ist
/// (`sampleDaten`, Sample.hpp) — ist nichts gemerkt, schweigt sie. Sind
/// Sample-Schichten dabei, koennen die beiden Seiten verschieden sein.
/// Eine Shape-Schicht spielt die Form, die sie nennt (`shape::tonformFuer`) —
/// gibt es sie nicht, schweigt sie; mit Stereo XY sind die Seiten verschieden.
[[nodiscard]] Klang rendereTon(const Instrument& instrument, int hoehe, double dauerMs, int rate);
/// Einen Ton nach seiner Angabe rechnen (Stufe 23b): wie oben; traegt sie ein
/// Ende (`TonAngabe::endeMs`), ist der Ton dort still — er blendet ueber die
/// letzten `kAbdaempfenMs` aus (Saitenplan.hpp, Entscheid S-M3 a). Ein Ton,
/// der vorher von selbst endet, bleibt Bit fuer Bit derselbe. Die Saite gibt
/// einer Schicht *Extended KS* ihre Dicke (String variation, Stufe 23c); ohne
/// Saite rechnet sie ohne Dicke.
[[nodiscard]] Klang rendereTon(const Instrument& instrument, const TonAngabe& ton, int rate);

/// Die Auskunft fuer die Planung (Entscheid S-A1 a): die Saiten eines
/// Instruments beim Namen, ueber `holeInstrument` — je Name einmal gelesen und
/// gemerkt, solange die Auskunft lebt. Ein Instrument, das sich nicht findet,
/// hat keine Saiten.
[[nodiscard]] SaitenVon saitenAusInstrumenten();

}  // namespace lumi::loop
