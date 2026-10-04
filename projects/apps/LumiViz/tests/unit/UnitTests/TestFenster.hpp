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

/// Vor `show()` rufen: Fenster oben links auf den Bildschirm der Referenzlaeufe.
inline void platziere(QWindow& fenster)
{
    if (QScreen* s = bildschirm())
    {
        fenster.setScreen(s);
        fenster.setPosition(s->geometry().topLeft());
    }
}

inline void platziere(QWidget& widget)
{
    if (QScreen* s = bildschirm())
    {
        widget.setScreen(s);
        widget.move(s->geometry().topLeft());
    }
}

}  // namespace testfenster
