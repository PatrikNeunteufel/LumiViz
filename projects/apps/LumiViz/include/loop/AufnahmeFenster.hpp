/**
 * @file AufnahmeFenster.hpp
 * @brief Loop: das Fenster „Record sample" — ein Sample aufnehmen, in
 *        Dauerschleife anhoeren, zuschneiden und zu den eigenen Samples legen
 *        (docs/audio/Loop_Konzept.md §4h, Stufe 11b; Idee Patrik S115, Skizze
 *        gebilligt)
 *
 * **Oben:** Input (die Eingaenge der Audio-Engine; der Mitschnitt der Ausgabe
 * ist als solcher bezeichnet) und der Pegel. **Record** und **Stop**, daneben
 * wie viel aufgenommen ist („3.2 s recorded · up to 30 s").
 * **Mitte:** die Aufnahme als Bild mit zwei Griffen fuer Anfang und Ende —
 * das Graue faellt beim Speichern weg; eine blaue Linie zeigt, wo gerade
 * gespielt wird. **Play** mit dem Haken **Repeat** spielt den Ausschnitt, auf
 * Wunsch in Dauerschleife — die Griffe lassen sich dabei ziehen.
 * **Unten:** **Trim silence at both ends**, **Normalize level**, **Cancel**,
 * **Save as...** und **Save**.
 *
 * Seit Stufe 17 (§4k; Entscheide M1 a bis M4 a, Patrik S119) oeffnet das
 * Fenster auch ein **bestehendes** Sample (`bearbeite`) und heisst dann „Edit
 * sample – <Name>". **Save** ersetzt ein eigenes Sample nach einer Rueckfrage
 * und ist sonst grau — nach einer frischen Aufnahme, bei einem Klang des Kits,
 * bei einer Datei an freiem Ort. **Save as...** fragt nach einem Namen und
 * legt ein neues eigenes Sample an; einen vergebenen Namen nimmt es nicht.
 *
 * Aufgenommen und gespielt wird ueber die `IAudioEngine`, die das Fenster
 * genannt bekommt (`setEngine`). Gespeichert wird mit `speichereAufnahme`
 * (Aufnahme.hpp) — `gespeichert` nennt das Sample, wie eine Schicht es nennt.
 */
#pragma once

#include "audio/IAudioEngine.hpp"
#include "loop/Aufnahme.hpp"

#include <QString>
#include <QWidget>

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QTimer;

namespace lumi::loop
{

class KurvenBild;

/// Spielt einen Ausschnitt einer Aufnahme als Quelle im Mixer — einmal oder
/// in Dauerschleife. Der Ausschnitt laesst sich aendern, waehrend sie spielt.
/// An seinen Enden blendet sie wie `schneideAus`, damit zu hoeren ist, was
/// gespeichert wird.
class AusschnittQuelle : public IAudioGenerator
{
public:
    /// Die Aufnahme und ihre Abtastrate, und die Rate, in der gespielt wird.
    /// Nur setzen, solange die Quelle in keinem Mixer haengt.
    void setzeDaten(std::shared_ptr<const std::vector<float>> stereo, int rate, int ausgabeRate);
    void setzeAusschnitt(Ausschnitt ausschnitt);
    void setzeWiederholen(bool an) { m_wiederholen.store(an); }
    /// Mit diesem Faktor wird gespielt (Normalize).
    void setzeVerstaerkung(float faktor) { m_faktor.store(faktor); }
    /// An welchen Enden geblendet wird — wie beim Speichern.
    void setzeBlenden(Blenden blenden)
    {
        m_blendeVorn.store(blenden.vorn);
        m_blendeHinten.store(blenden.hinten);
    }
    /// Von vorn: der naechste Block beginnt am Anfang des Ausschnitts.
    void starte();
    /// Die Stelle in der Aufnahme, an der zuletzt gespielt wurde.
    [[nodiscard]] std::int64_t stelle() const { return m_stelle.load(); }
    /// Am Ende des Ausschnitts angekommen (ohne Dauerschleife)?
    [[nodiscard]] bool fertig() const { return m_fertig.load(); }

    void render(float* out, int frames) override;

private:
    std::shared_ptr<const std::vector<float>> m_daten;
    int m_rate = 0;
    double m_schritt = 1.0;  ///< Abtastwerte der Aufnahme je Abtastwert der Ausgabe
    std::atomic<std::int64_t> m_von{0};
    std::atomic<std::int64_t> m_bis{0};
    std::atomic<bool> m_wiederholen{false};
    std::atomic<float> m_faktor{1.0f};
    std::atomic<bool> m_blendeVorn{true};
    std::atomic<bool> m_blendeHinten{true};
    std::atomic<bool> m_neu{false};
    std::atomic<bool> m_fertig{true};
    std::atomic<std::int64_t> m_stelle{0};
    double m_ort = 0.0;  ///< nur im Ton-Thread
};

class AufnahmeFenster : public QWidget
{
    Q_OBJECT

public:
    explicit AufnahmeFenster(QWidget* parent = nullptr);
    ~AufnahmeFenster() override;

    /// Die Engine, ueber die aufgenommen und gespielt wird; nullptr = keine
    /// (Record und Play sind dann aus). Sie muss das Fenster ueberleben, oder
    /// vorher mit nullptr abgemeldet werden.
    void setEngine(IAudioEngine* engine);
    /// Von vorn: keine Aufnahme, kein Sample, die Eingaenge neu gelesen.
    void beginne();

    /// Woher stammt, was im Bild steht?
    enum class Herkunft
    {
        Aufnahme,      ///< in diesem Fenster aufgenommen, oder noch nichts da
        Eigenes,       ///< ein eigenes Sample (`user:<name>`): Save ersetzt es
        Mitgeliefert,  ///< ein Klang des Kits (`kit:<name>`): schreibgeschuetzt
        Datei,         ///< eine Tondatei an freiem Ort: schreibgeschuetzt
    };
    /// **Edit...:** ein bestehendes Sample oeffnen, wie eine Schicht es nennt,
    /// mit seinen dekodierten Daten. Das ganze Sample steht im Bild, die
    /// Griffe an beiden Enden, Trim silence und Normalize sind aus — oeffnen
    /// und speichern aendert nichts.
    void bearbeite(const QString& nennung, std::vector<float> stereo, int rate);
    /// Das Sample, das offen ist; leer = keins (eine frische Aufnahme).
    [[nodiscard]] QString nennung() const { return m_nennung; }
    [[nodiscard]] Herkunft herkunft() const;
    /// Ist im Bild eine Aufnahme aus diesem Fenster (und nicht mehr, was geoeffnet wurde)?
    [[nodiscard]] bool frisch() const { return m_frisch; }
    /// Gibt es etwas zu verlieren: eine frische Aufnahme, ein versetzter Griff, Normalize?
    [[nodiscard]] bool geaendert() const;

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    /// **Record.** false = keine Engine, kein Eingang, oder er laesst sich nicht oeffnen.
    bool starteAufnahme();
    /// **Stop** (auch: die Aufnahme ist von selbst zu Ende): das Aufgenommene
    /// steht danach im Bild. false = es lief keine.
    bool stoppeAufnahme();
    [[nodiscard]] bool nimmtAuf() const { return m_nimmtAuf; }
    /// Eine fertige Aufnahme setzen, als waere sie eben aufgenommen.
    void setzeAufnahme(std::vector<float> stereo, int rate);
    [[nodiscard]] std::int64_t frames() const;
    [[nodiscard]] int rate() const { return m_rate; }
    /// Der Ausschnitt, der gespeichert wird.
    [[nodiscard]] Ausschnitt ausschnitt() const { return m_ausschnitt; }
    void setzeAusschnitt(Ausschnitt ausschnitt);
    /// **Trim silence at both ends:** an = der Ausschnitt ohne die Stille, aus = die ganze Aufnahme.
    void setzeOhneStille(bool an);
    void setzeNormalisieren(bool an);
    void setzeWiederholen(bool an);
    /// **Play:** den Ausschnitt spielen. false = nichts aufgenommen, keine Engine.
    bool spiele();
    void halte();
    [[nodiscard]] bool spielt() const { return m_mixer != INVALID_STREAM; }
    /// Was gespeichert wuerde: der Ausschnitt, geblendet, auf Wunsch normalisiert.
    [[nodiscard]] std::vector<float> ergebnis() const;
    /// Wo geblendet wird: nach einer Aufnahme an beiden Enden; an einem
    /// geoeffneten Sample nur dort, wo ein Griff nicht mehr am Rand steht.
    [[nodiscard]] Blenden blenden() const;
    /// **Save** ohne Rueckfrage: das eigene Sample ersetzen, das offen ist.
    /// Leer = kein eigenes Sample offen (`herkunft`), nichts im Bild, oder
    /// nicht zu schreiben; sonst die Nennung `user:<name>` — sie geht auch als
    /// `gespeichert` hinaus, und das Fenster schliesst.
    QString speichere();
    /// **Save as...** ohne Dialog: als neues eigenes Sample unter diesem Titel.
    /// Leer = nichts im Bild, aus dem Titel wird kein Name, den Namen gibt es
    /// schon, oder nicht zu schreiben; sonst wie `speichere`.
    QString speichereAls(const QString& titel);
    /// Was Save as... als Namen vorschlaegt: nach einer Aufnahme „Recording",
    /// „Recording 2", …; fuer ein geoeffnetes Sample „kick 2", „kick 3", …
    [[nodiscard]] QString vorschlagTitel() const;
    /// Der Satz, mit dem Save nachfragt: was ersetzt wird, was verloren geht,
    /// und welche eigenen Instrumente das Sample spielen (hoechstens drei beim
    /// Namen, dann „and n more").
    [[nodiscard]] QString ersetzenFrage() const;
    /// Die Zeile unter dem Namen im Dialog von Save as...: die Datei, die
    /// daraus wird — oder warum es so nicht geht. `geht` sagt, ob Save an ist.
    [[nodiscard]] static QString dateiZeile(const QString& titel, bool* geht = nullptr);
    /// Die Quelle, die den Ausschnitt spielt (fuer Tests).
    [[nodiscard]] AusschnittQuelle& quelle() { return m_quelle; }
    /// Ein Takt des Fensters — sein Zeitgeber ruft ihn, solange aufgenommen
    /// oder gespielt wird: Menge und Pegel der Aufnahme, die Stelle beim
    /// Spielen; eine Aufnahme oder ein Spielen, das von selbst zu Ende ging,
    /// wird hier abgeschlossen.
    void takt();

    /// So lang darf eine Aufnahme hoechstens sein, in Sekunden.
    static constexpr double kHoechstensSekunden = 30.0;
    /// Kuerzer als das wird ein Ausschnitt nicht, in Millisekunden.
    static constexpr double kAusschnittMinMs = 10.0;

Q_SIGNALS:
    /// Das Sample liegt bei den eigenen Samples und heisst so (`user:<name>`).
    /// `vornWegMs`: so viel ist gegenueber dem geoeffneten Sample vorn
    /// weggeschnitten, in Millisekunden — wer dort einen Einsatz stehen hat,
    /// zieht ihn mit (M3 a). Negativ = es ist eine andere Aufnahme.
    void gespeichert(const QString& nennung, double vornWegMs);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUI();
    void liesEingaenge();
    void zeigeAlles();
    void zeigeBild();
    void frageUndSpeichere();
    void frageUndSpeichereAls();
    /// Das Ergebnis unter diesem Namen ablegen, melden, schliessen.
    QString schreibe(const QString& name);
    [[nodiscard]] std::int64_t minFrames() const;

    IAudioEngine* m_engine = nullptr;
    std::shared_ptr<const std::vector<float>> m_daten;  ///< was im Bild steht, Stereo
    int m_rate = 0;
    Ausschnitt m_ausschnitt;
    bool m_nimmtAuf = false;
    bool m_gesichert = true;  ///< false = im Bild steht etwas, das so noch nicht gespeichert ist
    QString m_nennung;        ///< das geoeffnete Sample; leer = keins
    bool m_frisch = false;    ///< im Bild steht eine Aufnahme aus diesem Fenster
    QString m_meldung;        ///< statt der Zeile „… recorded": warum es nicht ging

    AusschnittQuelle m_quelle;
    AudioStreamHandle m_mixer = INVALID_STREAM;
    AudioStreamHandle m_quellHandle = INVALID_STREAM;

    QComboBox* m_eingang = nullptr;
    QProgressBar* m_pegel = nullptr;
    QPushButton* m_record = nullptr;
    QPushButton* m_stop = nullptr;
    QLabel* m_stand = nullptr;
    KurvenBild* m_bild = nullptr;
    QLabel* m_hinweis = nullptr;
    QPushButton* m_play = nullptr;
    QCheckBox* m_wiederholen = nullptr;
    QLabel* m_schnitt = nullptr;
    QCheckBox* m_ohneStille = nullptr;
    QCheckBox* m_normal = nullptr;
    QPushButton* m_sichernAls = nullptr;
    QPushButton* m_sichern = nullptr;
    QPushButton* m_publish = nullptr;
    QTimer* m_takt = nullptr;
};

}  // namespace lumi::loop
