/**
 * @file Mischer.hpp
 * @brief Composer: fertige Bilder mehrerer Renderer zu einem Bild mischen
 *        (docs/ui/Composer_Konzept.md §5, Stufe C1a)
 *
 * Der Composer laesst je laufendem Clip einen eigenen Renderer laufen. Jeder
 * legt sein fertiges Bild in eine `Leinwand`; der `Mischer` fuehrt die
 * Leinwaende von unten nach oben zusammen — jede mit ihrem Pegel (0…1) und
 * ihrer Mischart. Der Grund ist Schwarz.
 *
 * Eigener Code des Composers: beruehrt keinen bestehenden Renderweg. Vom
 * Renderer braucht er nur die Composer-Naehte R1 (Present-Ziel) und R2
 * (Karaoke-Ebene auslassen / einmal auflegen).
 *
 * Rueckbau (Konzept §12): Verzeichnisse `include/composer`, `src/composer`
 * loeschen, die Zeile in `src/Source.cmake` nehmen.
 */
#pragma once

#include <QImage>
#include <QSize>

#include <memory>

class MultiEffectVisualizer;
class QOpenGLBuffer;
class QOpenGLFramebufferObject;
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;

namespace lumi::composer
{

/// Wie ein Clip auf das trifft, was unter ihm liegt. Jede Art nimmt einen
/// Pegel: `ergebnis = mix(unten, art(unten, oben), pegel)` — Pegel 0 laesst
/// das Untere stehen, Pegel 1 ist die volle Mischart.
enum class Mischart : int
{
    Normal = 0,    ///< das Obere ersetzt das Untere (mit Pegel: ueberblenden)
    Additive = 1,  ///< unten + oben, begrenzt
    Subtract = 2,  ///< unten - oben, begrenzt
    Maximum = 3,
    Minimum = 4,
    Multiply = 5,
    Xor = 6,       ///< je Farbkanal bitweise
};

inline constexpr int kMischarten = 7;

/// Name der Mischart fuer die Oberflaeche (englisch, wie die App).
[[nodiscard]] const char* mischartName(Mischart art);

/**
 * Das Bild eines Renderers: ein Framebuffer-Objekt in Fenstergroesse, in das
 * der Renderer seinen Present legt (Composer-Naht R1) — samt Render Scale und
 * Post-Bloom, also genau so, wie er allein am Fenster stuende.
 */
class Leinwand
{
public:
    Leinwand();
    ~Leinwand();
    Leinwand(const Leinwand&) = delete;
    Leinwand& operator=(const Leinwand&) = delete;

    /// Ein Bild des Renderers in diese Leinwand rechnen. Setzt Present-Ziel
    /// und Viewport, ruft `render`, stellt das Present-Ziel danach wieder auf
    /// das Fenster. Braucht einen aktuellen GL-Kontext.
    void rendere(MultiEffectVisualizer& renderer, float deltaSekunden, QSize groesse);

    [[nodiscard]] unsigned int textur() const;
    [[nodiscard]] unsigned int framebuffer() const;
    [[nodiscard]] QSize groesse() const;
    [[nodiscard]] QImage bild() const;

private:
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo;
};

class Mischer
{
public:
    Mischer();
    ~Mischer();
    Mischer(const Mischer&) = delete;
    Mischer& operator=(const Mischer&) = delete;

    /// Neues Bild beginnen: Groesse sicherstellen, Grund schwarz.
    /// false = GL-Mittel liessen sich nicht anlegen.
    bool beginne(QSize groesse);
    /// Eine Textur ueber das bisher Gemischte legen. Pegel <= 0 tut nichts.
    void lege(unsigned int textur, double pegel, Mischart art);
    void lege(const Leinwand& bild, double pegel, Mischart art)
    {
        lege(bild.textur(), pegel, art);
    }

    /// Framebuffer, der das Gemischte haelt — gebunden kann ein Renderer die
    /// Karaoke-Ebene darueberlegen (`legeKaraokeEbeneAuf`).
    [[nodiscard]] unsigned int framebuffer() const;
    void binde();
    [[nodiscard]] unsigned int textur() const;
    [[nodiscard]] QImage bild() const;
    /// Das Gemischte 1:1 in einen Framebuffer zeichnen (0 = das Fenster).
    void zeichneIn(unsigned int framebuffer, QSize groesse);

private:
    bool stelleSicher(QSize groesse);
    void durchgang(unsigned int unten, unsigned int oben, double pegel, int art);

    std::unique_ptr<QOpenGLShaderProgram> m_shader;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
    std::unique_ptr<QOpenGLBuffer> m_vbo;
    std::unique_ptr<QOpenGLFramebufferObject> m_bild[2];
    int m_aktuell = 0;
};

}  // namespace lumi::composer
