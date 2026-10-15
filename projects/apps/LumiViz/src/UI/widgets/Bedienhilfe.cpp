/**
 * @file Bedienhilfe.cpp
 * @brief Menues und kleine Dialoge zeigen, in Tests abfragbar (S125)
 */
#include "UI/widgets/Bedienhilfe.hpp"

#include <QAction>
#include <QDialog>
#include <QInputDialog>
#include <QMenu>

namespace lumi::ui
{

namespace
{

MenueBeobachter& menueBeobachter()
{
    static MenueBeobachter beobachter;
    return beobachter;
}

DialogBeobachter& dialogBeobachter()
{
    static DialogBeobachter beobachter;
    return beobachter;
}

}  // namespace

QAction* zeigeMenue(QMenu& menue, const QPoint& global)
{
    if (!menueBeobachter()) return menue.exec(global);
    QAction* gewaehlt = menueBeobachter()(menue);
    // wie exec: eine graue oder fremde Aktion loest nichts aus
    if (gewaehlt != nullptr && gewaehlt->isEnabled()) gewaehlt->trigger();
    return gewaehlt;
}

int fuehreDialogAus(QDialog& dialog)
{
    if (!dialogBeobachter()) return dialog.exec();
    return dialogBeobachter()(dialog);
}

QString frageText(QWidget* parent, const QString& titel, const QString& frage, const QString& vorgabe, bool* ok)
{
    QInputDialog dialog(parent);
    dialog.setObjectName(QStringLiteral("textQuestion"));
    dialog.setWindowTitle(titel);
    dialog.setLabelText(frage);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextValue(vorgabe);
    const bool angenommen = fuehreDialogAus(dialog) == QDialog::Accepted;
    if (ok != nullptr) *ok = angenommen;
    return angenommen ? dialog.textValue() : QString();
}

void setzeMenueBeobachterFuerTests(MenueBeobachter beobachter)
{
    menueBeobachter() = std::move(beobachter);
}

void setzeDialogBeobachterFuerTests(DialogBeobachter beobachter)
{
    dialogBeobachter() = std::move(beobachter);
}

}  // namespace lumi::ui
