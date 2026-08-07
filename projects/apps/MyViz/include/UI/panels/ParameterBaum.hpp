/**
 ****************************************************************************************
 * @file   ParameterBaum.hpp
 * @brief  Generischer Parameter-Baum: `ParamGruppe` als bedienbarer Baum mit
 *         typsicheren Editoren je Zeile (Filter-Strang Stufe 3, S72)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * **Entwurf Patrik:** Aufbau wie die Effect-Chain (Baum), aber mit
 * **Wert-Spalte rechts** und **typsicheren Editoren** je Zeile — der Typ
 * steht in `ParamWert::typ`, das Widget wird daraus gewählt.
 *
 * **Der Baustein weiss NICHTS über Knotentypen.** Er arbeitet nur auf
 * `ParamGruppe` — genau deshalb kann er später die Parameter jedes Moduls
 * tragen (Entscheid Patrik S72: ABLÖSUNG der bisherigen Panel-Erzeugung,
 * gestaffelt; erst die skriptbaren Knoten, dann die Farb-/Transform-Klasse,
 * zuletzt die Meganodes).
 *
 * **Eine Rückmeldung, nicht viele:** jede Änderung liefert die KOMPLETTE
 * neue `ParamGruppe`. Der Aufrufer schreibt sie in einem Zug in den Knoten
 * (renderMutex + recompileChain, undo-fähig) — dasselbe Muster wie die
 * Voreinstellungs-Zeile. Ein Rückkanal je Feld hätte den Baum zwingen
 * müssen, Pfade in den Knoten zu kennen.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

#include <functional>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace lumi::parambaum {

/// Wird bei JEDER Änderung mit dem vollständigen neuen Baum gerufen.
using AenderungsHook = std::function<void(const lumi::multieffect::ParamGruppe&)>;

namespace detail {

/// Ein Wert im Baum wird über seinen PFAD adressiert: Index der Untergruppe
/// je Ebene, zuletzt der Index des Werts. So kommt der Editor an sein Ziel,
/// ohne einen Zeiger in einen Baum zu halten, der zwischendurch neu gebaut
/// werden kann.
struct Pfad
{
    std::vector<int> gruppen;
    int wert = -1;
};

[[nodiscard]] inline lumi::multieffect::ParamWert* finde(
    lumi::multieffect::ParamGruppe& wurzel, const Pfad& p)
{
    lumi::multieffect::ParamGruppe* g = &wurzel;
    for (int i : p.gruppen)
    {
        if (i < 0 || i >= static_cast<int>(g->gruppen.size())) return nullptr;
        g = &g->gruppen[static_cast<std::size_t>(i)];
    }
    if (p.wert < 0 || p.wert >= static_cast<int>(g->werte.size())) return nullptr;
    return &g->werte[static_cast<std::size_t>(p.wert)];
}

/// 0..1-Kanäle als QColor (die Ablage rechnet in 0..1, Qt in 0..255).
[[nodiscard]] inline QColor alsQColor(const std::array<double, 4>& v)
{
    const auto k = [](double x) {
        return static_cast<int>(std::clamp(x, 0.0, 1.0) * 255.0 + 0.5);
    };
    return QColor(k(v[0]), k(v[1]), k(v[2]), k(v[3]));
}

}  // namespace detail

/**
 * @brief Baut den Baum in ein frisches `QTreeWidget`
 * @param parent  Elternwidget
 * @param wurzel  der darzustellende Baum (wird kopiert — der Baum arbeitet
 *                auf seiner eigenen Fassung und liefert sie komplett zurück)
 * @param bei     Rückmeldung: bekommt bei jeder Änderung den ganzen Baum
 * @return das fertige Widget (Eigentum beim Aufrufer/Qt-Elternschaft)
 */
[[nodiscard]] inline QTreeWidget* baueParameterBaum(
    QWidget* parent, const lumi::multieffect::ParamGruppe& wurzel,
    AenderungsHook bei)
{
    using namespace lumi::multieffect;

    auto* baum = new QTreeWidget(parent);
    baum->setColumnCount(2);
    baum->setHeaderLabels({QObject::tr("Parameter"), QObject::tr("Wert")});
    baum->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    baum->header()->setStretchLastSection(true);
    baum->setRootIsDecorated(true);
    baum->setAlternatingRowColors(true);
    baum->setSelectionMode(QAbstractItemView::NoSelection);
    baum->setFocusPolicy(Qt::NoFocus);  // die Editoren bekommen den Fokus

    // Der Stand lebt in einem shared_ptr: jeder Editor-Callback haelt ihn,
    // und alle schreiben in DIESELBE Fassung — sonst ueberschriebe die
    // zweite Aenderung die erste (jeder haette eine eigene Kopie).
    auto stand = std::make_shared<ParamGruppe>(wurzel);
    auto melde = [stand, bei]() {
        if (bei) bei(*stand);
    };

    // Rekursiv, weil der Baum es ist. `std::function` statt Lambda-Rekursion:
    // ein Lambda kann sich selbst nicht nennen.
    std::function<void(QTreeWidgetItem*, const ParamGruppe&, detail::Pfad)> fuege =
        [&](QTreeWidgetItem* eltern, const ParamGruppe& g, detail::Pfad pfad) {
            for (std::size_t i = 0; i < g.werte.size(); ++i)
            {
                const ParamWert& w = g.werte[i];
                detail::Pfad p = pfad;
                p.wert = static_cast<int>(i);

                auto* zeile = new QTreeWidgetItem(eltern);
                zeile->setText(0, QString::fromStdString(
                                      w.label.empty() ? w.key : w.label));
                zeile->setToolTip(0, QString::fromStdString(w.key));

                switch (w.typ)
                {
                    case ParamTyp::Bool:
                    {
                        auto* box = new QCheckBox(baum);
                        box->setChecked(w.ja);
                        QObject::connect(box, &QCheckBox::toggled, baum,
                                         [stand, p, melde](bool an) {
                                             if (auto* z = detail::finde(*stand, p))
                                                 z->ja = an;
                                             melde();
                                         });
                        baum->setItemWidget(zeile, 1, box);
                        break;
                    }
                    case ParamTyp::Auswahl:
                    {
                        // Klartext-Dropdown: ISF liefert LABELS + VALUES, der
                        // Nutzer sieht „Fein" statt „2".
                        auto* combo = new QComboBox(baum);
                        for (std::size_t k = 0; k < w.auswahlLabels.size(); ++k)
                        {
                            const int wert =
                                k < w.auswahlWerte.size()
                                    ? w.auswahlWerte[k]
                                    : static_cast<int>(k);
                            combo->addItem(
                                QString::fromStdString(w.auswahlLabels[k]), wert);
                        }
                        const int idx = combo->findData(static_cast<int>(w.zahl));
                        combo->setCurrentIndex(idx >= 0 ? idx : 0);
                        QObject::connect(
                            combo, &QComboBox::currentIndexChanged, baum,
                            [stand, p, melde, combo](int) {
                                if (auto* z = detail::finde(*stand, p))
                                    z->zahl = combo->currentData().toDouble();
                                melde();
                            });
                        baum->setItemWidget(zeile, 1, combo);
                        break;
                    }
                    case ParamTyp::Ganzzahl:
                    {
                        auto* spin = new QSpinBox(baum);
                        // Ohne deklarierten Bereich waere Qts Vorgabe 0..99 —
                        // das KLEMMT stillschweigend jeden groesseren Wert.
                        spin->setRange(w.hatBereich ? static_cast<int>(w.min)
                                                    : -1000000,
                                       w.hatBereich ? static_cast<int>(w.max)
                                                    : 1000000);
                        spin->setValue(static_cast<int>(w.zahl));
                        QObject::connect(spin, &QSpinBox::valueChanged, baum,
                                         [stand, p, melde](int v) {
                                             if (auto* z = detail::finde(*stand, p))
                                                 z->zahl = v;
                                             melde();
                                         });
                        baum->setItemWidget(zeile, 1, spin);
                        break;
                    }
                    case ParamTyp::Text:
                    {
                        auto* edit = new QLineEdit(
                            QString::fromStdString(w.text), baum);
                        QObject::connect(edit, &QLineEdit::textChanged, baum,
                                         [stand, p, melde](const QString& t) {
                                             if (auto* z = detail::finde(*stand, p))
                                                 z->text = t.toStdString();
                                             melde();
                                         });
                        baum->setItemWidget(zeile, 1, edit);
                        break;
                    }
                    case ParamTyp::Farbe:
                    {
                        auto* knopf = new QPushButton(baum);
                        const auto male = [knopf](const QColor& c) {
                            knopf->setText(c.name(QColor::HexRgb));
                            knopf->setStyleSheet(
                                QStringLiteral("background:%1; color:%2")
                                    .arg(c.name(QColor::HexRgb),
                                         c.lightness() > 128
                                             ? QStringLiteral("#000")
                                             : QStringLiteral("#fff")));
                        };
                        male(detail::alsQColor(w.vektor));
                        QObject::connect(
                            knopf, &QPushButton::clicked, baum,
                            [stand, p, melde, male, baum]() {
                                auto* z = detail::finde(*stand, p);
                                if (z == nullptr) return;
                                const QColor alt = detail::alsQColor(z->vektor);
                                const QColor neu = QColorDialog::getColor(
                                    alt, baum, QObject::tr("Farbe wählen"),
                                    QColorDialog::ShowAlphaChannel);
                                if (!neu.isValid()) return;
                                z->vektor = {{neu.redF(), neu.greenF(),
                                              neu.blueF(), neu.alphaF()}};
                                male(neu);
                                melde();
                            });
                        baum->setItemWidget(zeile, 1, knopf);
                        break;
                    }
                    case ParamTyp::Punkt2D:
                    {
                        auto* wrap = new QWidget(baum);
                        auto* hl = new QHBoxLayout(wrap);
                        hl->setContentsMargins(0, 0, 0, 0);
                        for (int achse = 0; achse < 2; ++achse)
                        {
                            auto* spin = new QDoubleSpinBox(wrap);
                            spin->setDecimals(4);
                            spin->setSingleStep(0.01);
                            spin->setRange(w.hatBereich ? w.min : -1e6,
                                           w.hatBereich ? w.max : 1e6);
                            spin->setValue(w.vektor[static_cast<std::size_t>(achse)]);
                            spin->setPrefix(achse == 0 ? QStringLiteral("x ")
                                                       : QStringLiteral("y "));
                            QObject::connect(
                                spin, &QDoubleSpinBox::valueChanged, baum,
                                [stand, p, melde, achse](double v) {
                                    if (auto* z = detail::finde(*stand, p))
                                        z->vektor[static_cast<std::size_t>(achse)] = v;
                                    melde();
                                });
                            hl->addWidget(spin);
                        }
                        baum->setItemWidget(zeile, 1, wrap);
                        break;
                    }
                    default:  // ParamTyp::Zahl
                    {
                        auto* spin = new QDoubleSpinBox(baum);
                        spin->setDecimals(4);
                        spin->setRange(w.hatBereich ? w.min : -1e6,
                                       w.hatBereich ? w.max : 1e6);
                        // Schrittweite aus dem Bereich, nicht pauschal 1.0 —
                        // bei 0..1 waere ein Klick sonst der ganze Weg.
                        spin->setSingleStep(w.hatBereich
                                                ? std::max((w.max - w.min) / 100.0,
                                                           1e-4)
                                                : 0.01);
                        spin->setValue(w.zahl);
                        QObject::connect(spin, &QDoubleSpinBox::valueChanged, baum,
                                         [stand, p, melde](double v) {
                                             if (auto* z = detail::finde(*stand, p))
                                                 z->zahl = v;
                                             melde();
                                         });
                        baum->setItemWidget(zeile, 1, spin);
                        break;
                    }
                }
            }

            for (std::size_t i = 0; i < g.gruppen.size(); ++i)
            {
                const ParamGruppe& u = g.gruppen[i];
                auto* kopf = new QTreeWidgetItem(eltern);
                kopf->setText(0, QString::fromStdString(
                                     u.label.empty() ? u.key : u.label));
                kopf->setFirstColumnSpanned(true);
                detail::Pfad p = pfad;
                p.gruppen.push_back(static_cast<int>(i));
                fuege(kopf, u, p);
                kopf->setExpanded(true);
            }
        };

    // Die Wurzel selbst bekommt KEINE eigene Zeile — ihre Werte haengen
    // direkt unter dem unsichtbaren Wurzelelement.
    fuege(baum->invisibleRootItem(), *stand, detail::Pfad{});
    baum->expandAll();
    return baum;
}

}  // namespace lumi::parambaum
