/*
 * File: qgcolorchooser.h
 * ----------------------
 * This file defines the <code>QGColorChooser</code> class which supports
 * popping up graphical dialog boxes to select colors.
 * 
 * @author Marty Stepp
 * @version 2018/07/29
 * - initial version
 */

#ifdef SPL_QT_GUI
#ifndef _qgcolorchooser_h
#define _qgcolorchooser_h

#include <string>
#include <QWidget>

class QGColorChooser {
public:
    static std::string showDialog(const std::string& title, int initialColor);
    static std::string showDialog(QWidget* parent, const std::string& title, int initialColor);
    static std::string showDialog(const std::string& title = "", const std::string& initialColor = "");
    static std::string showDialog(QWidget* parent, const std::string& title = "", const std::string& initialColor = "");
};

#include "private/init.h"   // ensure that Stanford C++ lib is initialized

#endif // _qgcolorchooser_h
#endif // SPL_QT_GUI
