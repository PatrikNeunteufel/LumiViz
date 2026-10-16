/**
 * @file TestFenster.hpp
 * @brief Tests mit echtem Fenster: das Fenster auf den Bildschirm der
 *        Referenzlaeufe setzen (`LUMIVIZ_REF_BILDSCHIRM`, wie die Standalones —
 *        projects/exec/common/RefBildschirm.hpp)
 *
 * `LUMIVIZ_REF_BILDSCHIRM=DISPLAY2` (Geraetename, Gross-/Kleinschreibung egal)
 * oder eine Zahl (Index in `QGuiApplication::screens()`). Nicht gesetzt oder
 * kein passender Bildschirm: das Fenster bleibt, wo Qt es hinlegt — ein Test
 * scheitert nie an der Platzierung.
 *
 * Rueckbau (Composer, Konzept §12): geht mit den Testdateien des Composers.
 */
#pragma once

#include <QGuiApplication>
#include <QScreen>
#include <QString>
#include <QWidget>
#include <QWindow>

namespace testfenster
{

/// Der gewuenschte Bildschirm, nullptr = keine Vorgabe.
inline QScreen* bildschirm()
{
    const QString wunsch = qEnvironmentVariable("LUMIVIZ_REF_BILDSCHIRM").trimmed();
    if (wunsch.isEmpty()) return nullptr;
    const QList<QScreen*> alle = QGuiApplication::screens();
    for (QScreen* s : alle)
        if (s->name().contains(wunsch, Qt::CaseInsensitive)) return s;
    bool zahl = false;
    const int index = wunsch.toInt(&zahl);
    if (zahl && index >= 0 && index < alle.size()) return alle[index];
    return nullptr;
}

/// Vor `show()` rufen: Fenster oben links auf den Bildschirm der Referenzlaeufe,
/// und ohne den Fokus zu nehmen (Entscheid Patrik S129, F1 a — ein Lauf soll die
/// Arbeit am anderen Bildschirm nicht stoeren). Ein Fenster der Tests nimmt
/// keine Tasten von Hand an; Tests schicken ihre Ereignisse direkt.
inline void platziere(QWindow& fenster)
{
    if (QScreen* s = bildschirm())
    {
        fenster.setScreen(s);
        fenster.setPosition(s->geometry().topLeft());
        fenster.setFlag(Qt::WindowDoesNotAcceptFocus);
    }
}

/// Wie oben; ein Widget erscheint nur ohne Aktivierung, ein Klick aktiviert es.
inline void platziere(QWidget& widget)
{
    if (QScreen* s = bildschirm())
    {
        widget.setScreen(s);
        widget.move(s->geometry().topLeft());
        widget.setAttribute(Qt::WA_ShowWithoutActivating);
    }
}

}  // namespace testfenster
