/**
 * @file Bedienhilfe.hpp
 * @brief Menues und kleine Dialoge zeigen — so, dass ein Test sie abfragen kann
 *        (S125, Offene_Punkte §6 Loop: „Menüs, Dialoge … ohne Test")
 *
 * `QMenu::exec` und `QDialog::exec` zeigen ein Fenster und warten; ein Test kommt
 * dazwischen nicht an die Eintraege. Wer ueber diese Helfer zeigt, gibt einem
 * Test die Gelegenheit: ein gesetzter Beobachter bekommt das fertige Menue bzw.
 * den Dialog statt der Anzeige — er prueft Eintraege und Felder, waehlt eine
 * Aktion oder fuellt aus und gibt das Ergebnis zurueck. Ohne Beobachter
 * (im Programm) zeigen die Helfer wie `exec`.
 */
#pragma once

#include <QPoint>
#include <QString>

#include <functional>

class QAction;
class QDialog;
class QMenu;
class QWidget;

namespace lumi::ui
{

/// Bekommt das Menue; zurueck die Aktion, die gewaehlt wird (nullptr = keine).
using MenueBeobachter = std::function<QAction*(QMenu&)>;
/// Bekommt den Dialog; zurueck `QDialog::Accepted` oder `QDialog::Rejected`.
using DialogBeobachter = std::function<int(QDialog&)>;

/// Das Menue an `global` zeigen und die gewaehlte Aktion ausloesen (wie `exec`).
QAction* zeigeMenue(QMenu& menue, const QPoint& global);
/// Den Dialog modal ausfuehren (wie `exec`).
int fuehreDialogAus(QDialog& dialog);
/// Wie `QInputDialog::getText` mit `QLineEdit::Normal`, ueber `fuehreDialogAus`;
/// der Dialog heisst `textQuestion`.
QString frageText(QWidget* parent, const QString& titel, const QString& frage, const QString& vorgabe, bool* ok);

/// Nur fuer Tests; leer = wieder zeigen.
void setzeMenueBeobachterFuerTests(MenueBeobachter beobachter);
void setzeDialogBeobachterFuerTests(DialogBeobachter beobachter);

}  // namespace lumi::ui
