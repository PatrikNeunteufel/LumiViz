/**
 ****************************************************************************************
 * @file   MainWindow.hpp
 * @brief  Main Application Window - Qt6 Tutorial
 *         Demonstrates basic QMainWindow setup without menu
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This file is part of the Qt6 Tutorial series for MyViz.
 * It demonstrates:
 *   - QMainWindow inheritance
 *   - Q_OBJECT macro usage
 *   - Basic window configuration
 *
 * @see MainWindow.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

 // =============================================================================
 // Qt Includes
 // =============================================================================
 // QMainWindow is the base class for main application windows in Qt.
 // It provides a framework for building an application's user interface.
 // Features include: menu bar, toolbars, dock widgets, status bar, central widget.

#include <QMainWindow>

// =============================================================================
// Forward Declarations
// =============================================================================
// Forward declarations avoid including headers in .hpp files.
// This reduces compilation time and circular dependencies.

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Main application window for MyViz.
 *
 * MainWindow is the primary window of the application. It inherits from
 * QMainWindow, which provides the standard main window features.
 *
 * ## Qt6 Tutorial: QMainWindow
 *
 * QMainWindow is designed to be the main window of an application. It has
 * a predefined layout with specific areas:
 *
 * ```
 * +------------------------------------------+
 * |              Menu Bar                    |  <- setMenuBar()
 * +------------------------------------------+
 * |              Tool Bar                    |  <- addToolBar()
 * +------+---------------------------+-------+
 * | Dock |                           | Dock  |  <- addDockWidget()
 * |      |      Central Widget       |       |
 * |      |                           |       |  <- setCentralWidget()
 * +------+---------------------------+-------+
 * |              Status Bar                  |  <- setStatusBar()
 * +------------------------------------------+
 * ```
 *
 * In this tutorial, we create an empty window without menu, toolbar, or
 * dock widgets - just a central widget.
 *
 * ## Q_OBJECT Macro
 *
 * The Q_OBJECT macro is required for any class that:
 * - Uses signals and slots
 * - Uses Qt's meta-object system (qobject_cast, property system)
 *
 * It must appear in the private section of the class definition.
 *
 * @note This class is non-copyable (deleted copy constructor/assignment).
 * @note Qt widgets should generally not be copied.
 */
    class MainWindow : public QMainWindow
{
    // =========================================================================
    // Q_OBJECT Macro
    // =========================================================================
    // REQUIRED for Qt's meta-object system.
    // Enables: signals, slots, qobject_cast, dynamic properties, tr()
    // Must be first in class, in private section (implicitly private here).

    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Constructs the MainWindow.
     *
     * @param parent Optional parent widget.
     *               If nullptr, this window is a top-level window.
     *               If set, this window becomes a child (embedded in parent).
     *
     * ## Qt6 Tutorial: Parent-Child Relationship
     *
     * Qt uses a parent-child ownership model:
     * - Parent owns its children
     * - When parent is deleted, children are automatically deleted
     * - Top-level windows (parent = nullptr) must be explicitly deleted
     *   or will be deleted when QApplication exits
     *
     * For MainWindow, we typically pass nullptr (top-level window).
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor.
     *
     * Qt automatically deletes all child widgets when the parent is destroyed.
     * We use 'override' to ensure we're actually overriding a virtual function.
     */
    ~MainWindow() override;

    // =========================================================================
    // Deleted Special Members
    // =========================================================================
    // Qt widgets should not be copied. We explicitly delete these to prevent
    // accidental copying and to make the intention clear.

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

private:
    // =========================================================================
    // Private Methods
    // =========================================================================

    /**
     * @brief Sets up the user interface.
     *
     * This method configures:
     * - Window title and size
     * - Central widget
     * - (Future: menu bar, status bar, etc.)
     *
     * Separating UI setup into its own method keeps the constructor clean
     * and makes the code more maintainable.
     */
    void setupUi();
};