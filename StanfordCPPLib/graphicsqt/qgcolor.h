/*
 * File: qgcolor.h
 * ---------------
 *
 * @version 2018/06/30
 * - initial version
 */

#ifdef SPL_QT_GUI
#ifndef _qgcolor_h
#define _qgcolor_h

#include <string>
#include "map.h"
#include <QColor>

/*
 * ...
 */
class QGColor {
public:
    enum {
        BLACK = 0x000000,
        BLUE = 0x0000FF,
        BROWN = 0x926239,
        CYAN = 0x00FFFF,
        DARKGRAY = 0x595959,
        GRAY = 0x999999,
        GREEN = 0x00FF00,
        LIGHTGRAY = 0xBFBFBF,
        MAGENTA = 0xFF00FF,
        ORANGE = 0xFFC800,
        PINK = 0xFFAFAF,
        PURPLE = 0xFF00FF,
        RED = 0xFF0000,
        WHITE = 0xFFFFFF,
        YELLOW = 0xFFFF00
    } Color;

    /*
     * Function: convertRGBToColor
     * Usage: int colorName = convertRGBToColor(a, r, g, b);
     * -----------------------------------------------------
     * Converts four integer RGB values from 0-255 into a color name in the
     * form <code>"#aarrggbb"</code>.  Each of the <code>aa</code>, <code>rr</code>,
     * <code>gg</code>, and <code>bb</code> values are two-digit
     * hexadecimal numbers indicating the intensity of that component.
     * If any of a, r, g, or b is outside the range of 0-255, throws an error.
     */
    static std::string convertARGBToColor(int a, int r, int g, int b);

    /*
     * Function: convertColorToRGB
     * Usage: int rgb = convertColorToRGB(colorName);
     * ----------------------------------------------
     * Converts a color name into an integer that encodes the
     * red, green, and blue components of the color.
     */
    static int convertColorToRGB(const std::string& colorName);

    /*
     * Function: convertRGBToColor
     * Usage: int colorName = convertRGBToColor(rgb);
     * ----------------------------------------------
     * Converts an <code>rgb</code> value into a color name in the
     * form <code>"#rrggbb"</code>.  Each of the <code>rr</code>,
     * <code>gg</code>, and <code>bb</code> values are two-digit
     * hexadecimal numbers indicating the intensity of that component.
     */
    static std::string convertRGBToColor(int rgb);

    /*
     * Function: convertRGBToColor
     * Usage: int colorName = convertRGBToColor(r, g, b);
     * --------------------------------------------------
     * Converts three integer RGB values from 0-255 into a color name in the
     * form <code>"#rrggbb"</code>.  Each of the <code>rr</code>,
     * <code>gg</code>, and <code>bb</code> values are two-digit
     * hexadecimal numbers indicating the intensity of that component.
     * If any of r, g, or b is outside the range of 0-255, throws an error.
     */
    static std::string convertRGBToColor(int r, int g, int b);

    static int convertARGBToARGB(int a, int r, int g, int b);
    static int convertRGBToRGB(int r, int g, int b);

    static std::string convertQColorToColor(const QColor& color);
    static int convertQColorToRGB(const QColor& color);

    static QColor toQColor(const std::string& color);

private:
    QGColor();   // forbid construction

    static std::string canonicalColorName(const std::string& str);
    static const Map<std::string, int>& colorTable();
    static const Map<std::string, std::string>& colorNameTable();
    static int fixAlpha(int argb);

    static Map<std::string, int> _colorTable;
    static Map<std::string, std::string> _colorNameTable;
};

#include "private/init.h"   // ensure that Stanford C++ lib is initialized

#endif // _qgcolor_h
#endif // SPL_QT_GUI
