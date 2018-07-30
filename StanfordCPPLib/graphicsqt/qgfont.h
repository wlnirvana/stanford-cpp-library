/*
 * File: qgfont.h
 * --------------
 *
 * @version 2018/07/05
 * - initial version
 */

#ifdef SPL_QT_GUI
#ifndef _qgfont_h
#define _qgfont_h

#include <string>
#include <QFont>

/*
 * ...
 */
class QGFont {
public:
    static QFont deriveQFont(const QFont& font, QFont::Weight weight = QFont::Normal, int size = -1);
    static QFont deriveQFont(const QFont& font, const std::string& fontFamily, QFont::Weight weight = QFont::Normal, int size = -1);
    static std::string toFontString(const QFont& font);
    static QFont toQFont(const std::string& fontString);
    static QFont toQFont(const QFont& font, const std::string& fontString);

private:
    QGFont();
    static QFont::StyleHint getStyleHint(const std::string& fontFamily);
};

#include "private/init.h"   // ensure that Stanford C++ lib is initialized

#endif // _qgfont_h
#endif // SPL_QT_GUI
