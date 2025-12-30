/**
 ****************************************************************************************
 * @file   WidgetBase.cpp
 * @brief  WidgetBase explicit template instantiations
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * This file provides explicit template instantiations for common base classes.
 * This can improve compile times by avoiding redundant template instantiation
 * across translation units.
 *
 * Note: Additional instantiations (e.g., QOpenGLWidget) require the respective
 * Qt headers to be included.
 ****************************************************************************************
 */

#include "UI/widgets/WidgetBase.hpp"

#include <QWidget>
#include <QFrame>
#include <QOpenGLWidget>

// =============================================================================
// Explicit Template Instantiations
// =============================================================================

// Standard QWidget base
template class WidgetBase<QWidget>;

// QFrame base (for widgets with borders)
template class WidgetBase<QFrame>;

// QOpenGLWidget base (for OpenGL rendering)
template class WidgetBase<QOpenGLWidget>;
