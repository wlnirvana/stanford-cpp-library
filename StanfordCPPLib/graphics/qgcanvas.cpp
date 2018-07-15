/*
 * File: qgcanvas.cpp
 * ------------------
 *
 * @version 2018/06/30
 * - initial version
 */

#include "qgcanvas.h"
#include "qgcolor.h"
#include "qgwindow.h"
#include "error.h"
#include "filelib.h"
#include "strlib.h"

_Internal_QCanvas::_Internal_QCanvas(QGCanvas* qgcanvas, QWidget* parent)
        : QWidget(parent),
          _qgcanvas(qgcanvas) {
    // set default white background color
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);
    setMouseTracking(true);   // causes mouse move events to occur
}

void _Internal_QCanvas::enterEvent(QEvent* event) {
    QWidget::enterEvent(event);   // call super
    if (!_qgcanvas->hasEventHandler("mouseenter")) return;
    _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_ENTERED, "mouseenter");
}

void _Internal_QCanvas::keyPressEvent(QKeyEvent* event) {
    QWidget::keyPressEvent(event);   // call super
    if (!_qgcanvas->hasEventHandler("keypress")) return;
    _qgcanvas->fireQGEvent(event, QGEvent::KEY_PRESSED, "keypress");
}

void _Internal_QCanvas::keyReleaseEvent(QKeyEvent* event) {
    QWidget::keyReleaseEvent(event);   // call super
    if (_qgcanvas->hasEventHandler("keyrelease")) {
        _qgcanvas->fireQGEvent(event, QGEvent::KEY_RELEASED, "keyrelease");
    }
    if (_qgcanvas->hasEventHandler("keytype")) {
        _qgcanvas->fireQGEvent(event, QGEvent::KEY_TYPED, "keytype");
    }
}

void _Internal_QCanvas::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);   // call super
    if (!_qgcanvas->hasEventHandler("mouseexit")) return;
    _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_EXITED, "mouseexit");
}

void _Internal_QCanvas::mouseMoveEvent(QMouseEvent* event) {
    QWidget::mouseMoveEvent(event);   // call super
    if (!_qgcanvas->hasEventHandler("mousemove")
            && !_qgcanvas->hasEventHandler("mousedrag")) return;
    _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_MOVED, "mousemove");
    if (event->buttons() != 0) {
        // mouse drag
        _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_DRAGGED, "mousedrag");
    }
}

void _Internal_QCanvas::mousePressEvent(QMouseEvent* event) {
    QWidget::mousePressEvent(event);   // call super
    if (!_qgcanvas->hasEventHandler("mousepress")) return;
    _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_PRESSED, "mousepress");
}

void _Internal_QCanvas::mouseReleaseEvent(QMouseEvent* event) {
    QWidget::mouseReleaseEvent(event);   // call super
    if (_qgcanvas->hasEventHandler("mouserelease")) {
        _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_RELEASED, "mouserelease");
    }

    if (_qgcanvas->hasEventHandler("click")) {
        _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_CLICKED, "click");
    }
}

void _Internal_QCanvas::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);   // call super

    QPainter painter(this);
    // g.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    // g.setRenderHints(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::Antialiasing, QGObject::isAntiAliasing());
    painter.setRenderHint(QPainter::TextAntialiasing, QGObject::isAntiAliasing());
    _qgcanvas->draw(&painter);
    painter.end();
}

void _Internal_QCanvas::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);   // call super
    QSize size = event->size();
    _qgcanvas->notifyOfResize(size.width(), size.height());
}

QSize _Internal_QCanvas::sizeHint() const {
    if (hasPreferredSize()) {
        return getPreferredSize();
    } else {
        return QWidget::sizeHint();
    }
}

void _Internal_QCanvas::wheelEvent(QWheelEvent* event) {
    QWidget::wheelEvent(event);   // call super
    if (event->pixelDelta().y() < 0) {
        // scroll down
        if (_qgcanvas->hasEventHandler("mousewheeldown")) {
            _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_WHEEL_DOWN, "mousewheeldown");
        }
    } else if (event->pixelDelta().y() > 0) {
        // scroll up
        if (_qgcanvas->hasEventHandler("mousewheelup")) {
            _qgcanvas->fireQGEvent(event, QGEvent::MOUSE_WHEEL_UP, "mousewheelup");
        }
    }
}


#define CHAR_TO_HEX(ch) ((ch >= '0' && ch <= '9') ? (ch - '0') : (ch - 'a' + 10))

const int QGCanvas::WIDTH_HEIGHT_MAX = 65535;

int QGCanvas::createRgbPixel(int red, int green, int blue) {
    if (red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {
        error("RGB values must be between 0-255");
    }
    return ((red << 16 & 0xff0000) | (green << 8 & 0x00ff00) | (blue & 0x0000ff)) & 0xffffff;
}

int QGCanvas::getAlpha(int argb) {
    // have to & a second time because of sign-extension on >> shift
    return ((argb & 0xff000000) >> 24) & 0x000000ff;
}

int QGCanvas::getBlue(int rgb) {
    return rgb & 0x0000ff;
}

int QGCanvas::getGreen(int rgb) {
    return (rgb & 0x00ff00) >> 8;
}

int QGCanvas::getRed(int rgb) {
    return (rgb & 0xff0000) >> 16;
}

void QGCanvas::getRedGreenBlue(int rgb, int& red, int& green, int& blue) {
    red = getRed(rgb);
    green = getGreen(rgb);
    blue = getBlue(rgb);
}


QGCanvas::QGCanvas(QWidget* parent)
        : _backgroundImage(nullptr),
          _filename("") {
    init(/* width */ -1, /* height */ -1, /* background */ 0x0, parent);
}

QGCanvas::QGCanvas(const std::string& filename, QWidget* parent)
        : _backgroundImage(nullptr),
          _filename(filename) {
    init(/* width */ -1, /* height */ -1, /* background */ 0x0, parent);
    load(filename);
}

QGCanvas::QGCanvas(double width, double height, int rgbBackground, QWidget* parent)
        : _backgroundImage(nullptr),
          _filename("") {
    init(width, height, rgbBackground, parent);
}

QGCanvas::QGCanvas(double width, double height, const std::string& rgbBackground, QWidget* parent)
        : _backgroundImage(nullptr),
          _filename("") {
    init(width, height, QGColor::convertColorToRGB(rgbBackground), parent);
}

void QGCanvas::init(double width, double height, int rgbBackground, QWidget* parent) {
    checkSize("constructor", width, height);
    checkColor("constructor", rgbBackground);
    _iqcanvas = new _Internal_QCanvas(this, getInternalParent(parent));
    _qgcompound.setWidget(_iqcanvas);
    _backgroundColor = rgbBackground;
    if (width > 0 && height > 0) {
        setSize(width, height);
        if (rgbBackground != 0) {
            fill(rgbBackground);
        }
    }
}



QGCanvas::~QGCanvas() {
    // TODO: delete _qgcanvas;
    _iqcanvas = nullptr;
}

void QGCanvas::add(QGObject* gobj) {
    _qgcompound.add(gobj);   // calls conditionalRepaint
}

void QGCanvas::add(QGObject* gobj, double x, double y) {
    _qgcompound.add(gobj, x, y);   // calls conditionalRepaint
}

void QGCanvas::add(QGObject& gobj) {
    _qgcompound.add(gobj);   // calls conditionalRepaint
}

void QGCanvas::add(QGObject& gobj, double x, double y) {
    _qgcompound.add(gobj, x, y);   // calls conditionalRepaint
}

void QGCanvas::clear() {
    clearPixels();
    clearObjects();
}

void QGCanvas::clearObjects() {
    _qgcompound.clear();   // calls conditionalRepaint
}

void QGCanvas::clearPixels() {
    if (_backgroundImage) {
        // delete _backgroundImage;
        // _backgroundImage = nullptr;
        // keep background image buffer but fill with background color instead
        _backgroundImage->fill(_backgroundColorInt);
    }
    conditionalRepaint();
}

bool QGCanvas::contains(double x, double y) const {
    return _qgcompound.contains(x, y);
}

int QGCanvas::countDiffPixels(const QGCanvas& image) const {
    int w1 = (int) getWidth();
    int h1 = (int) getHeight();
    int w2 = (int) image.getWidth();
    int h2 = (int) image.getHeight();

    int wmin = std::min(w1, w2);
    int hmin = std::min(h1, h2);

    int overlap = std::min(w1, w2) * std::min(h1, h2);
    int diffPxCount = (w1 * h1 - overlap) + (w2 * h2 - overlap);

    for (int y = 0; y < hmin; y++) {
        for (int x = 0; x < wmin; x++) {
            int px1 = getRGB(x, y);
            int px2 = image.getRGB(x, y);
            if (px1 != px2) {
                diffPxCount++;
            }
        }
    }

    return diffPxCount;
}

int QGCanvas::countDiffPixels(const QGCanvas& image, int xmin, int ymin, int xmax, int ymax) const {
    int w1 = (int) getWidth();
    int h1 = (int) getHeight();
    int w2 = (int) image.getWidth();
    int h2 = (int) image.getHeight();
    int diffPxCount = 0;

    for (int y = ymin; y < ymax; y++) {
        for (int x = xmin; x < xmax; x++) {
            int px1 = x < w1 && y < h1 ? getRGB(x, y) : -1;
            int px2 = x < w2 && y < h2 ? image.getRGB(x, y) : -1;
            if (px1 != px2) {
                diffPxCount++;
            }
        }
    }

    return diffPxCount;
}

int QGCanvas::countDiffPixels(const QGCanvas* image) const {
    return countDiffPixels(*image);
}

int QGCanvas::countDiffPixels(const QGCanvas* image, int xmin, int ymin, int xmax, int ymax) const {
    return countDiffPixels(*image, xmin, ymin, xmax, ymax);
}

QGCanvas* QGCanvas::diff(const QGCanvas& image, int diffPixelColor) const {
    int w1 = (int) getWidth();
    int h1 = (int) getHeight();
    int w2 = (int) image.getWidth();
    int h2 = (int) image.getHeight();
    int wmin = std::min(w1, w2);
    int hmin = std::min(h1, h2);
    int wmax = std::max(w1, w2);
    int hmax = std::max(h1, h2);

    Grid<int> resultGrid;
    resultGrid.resize(hmax, wmax);
    resultGrid.fill(diffPixelColor);
    for (int r = 0; r < h1; r++) {
        for (int c = 0; c < w1; c++) {
            resultGrid[r][c] = _backgroundColorInt;
        }
    }
    for (int y = 0; y < hmin; y++) {
        for (int x = 0; x < wmin; x++) {
            int px1 = getRGB(x, y);
            int px2 = image.getRGB(x, y);
            if (px1 != px2) {
                resultGrid[y][x] = diffPixelColor;
            }
        }
    }
    QGCanvas* result = new QGCanvas(wmax, hmax);
    result->fromGrid(resultGrid);
    return result;
}

QGCanvas* QGCanvas::diff(const QGCanvas* image, int diffPixelColor) const {
    return diff(*image, diffPixelColor);
}

void QGCanvas::draw(QPainter* painter) {
    if (_backgroundImage) {
        painter->drawImage(/* x */ 0, /* y */ 0, *_backgroundImage);
    }
    _qgcompound.draw(painter);   // calls conditionalRepaint
}

void QGCanvas::draw(QGObject* gobj) {
    ensureBackgroundImage();
    QPainter painter(_backgroundImage);
    painter.setRenderHint(QPainter::Antialiasing, QGObject::isAntiAliasing());
    painter.setRenderHint(QPainter::TextAntialiasing, QGObject::isAntiAliasing());
    gobj->draw(&painter);
    painter.end();
    conditionalRepaintRegion(gobj->getBounds().enlargedBy((gobj->getLineWidth() + 1) / 2));
}

void QGCanvas::ensureBackgroundImage() {
    if (!_backgroundImage) {
        _backgroundImage = new QImage((int) getWidth(), (int) getHeight(), QImage::Format_ARGB32);
        if (!_backgroundColor.empty()) {
            _backgroundImage->fill(_backgroundColorInt | 0xff000000);
        }
    }
}

void QGCanvas::ensureBackgroundImageConstHack() const {
    if (!_backgroundImage) {
        // Your whole life has been a lie.
        // Also, this code is bad and I should feel bad.
        QGCanvas* hack = (QGCanvas*) this;
        hack->ensureBackgroundImage();
    }
}

bool QGCanvas::equals(const QGCanvas& other) const {
    if (getSize() != other.getSize()) {
        return false;
    }
    // TODO: compare pixels
    return true;
}

void QGCanvas::fill(int rgb) {
    checkColor("fill", rgb);
    // TODO: for each row/col, setPixel
}

void QGCanvas::fill(const std::string& rgb) {
    fill(QGColor::convertColorToRGB(rgb));
}

void QGCanvas::fillRegion(double x, double y, double width, double height, int rgb) {
    checkBounds("fillRegion", x, y);
    checkBounds("fillRegion", x + width - 1, y + height - 1);
    checkColor("fillRegion", rgb);
    for (int r = (int) y; r < y + height; r++) {
        for (int c = (int) x; c < x + width; c++) {
            setRGB(/* x */ c, /* y */ r, rgb);
        }
    }
    // TODO: repaint?
}

void QGCanvas::fillRegion(double x, double y, double width, double height, const std::string& rgb) {
    fillRegion(x, y, width, height, QGColor::convertColorToRGB(rgb));
}

void QGCanvas::flatten() {
    ensureBackgroundImage();
    QPainter painter(_backgroundImage);
    painter.setRenderHint(QPainter::Antialiasing, QGObject::isAntiAliasing());
    painter.setRenderHint(QPainter::TextAntialiasing, QGObject::isAntiAliasing());
    _qgcompound.draw(&painter);
    painter.end();
    _qgcompound.clear();   // calls conditionalRepaint
}

void QGCanvas::fromGrid(const Grid<int>& grid) {
    checkSize("fromGrid", grid.width(), grid.height());
    setSize(grid.width(), grid.height());

    for (int row = 0, width = grid.width(), height = grid.height(); row < height; row++) {
        for (int col = 0; col < width; col++) {
            setPixel(col, row, grid[row][col]);
        }
    }

    conditionalRepaint();
}

std::string QGCanvas::getBackground() const {
    return QGDrawingSurface::getBackground();
}

int QGCanvas::getBackgroundInt() const {
    return QGDrawingSurface::getBackgroundInt();
}

QGObject* QGCanvas::getElement(int index) const {
    return _qgcompound.getElement(index);
}

QGObject* QGCanvas::getElementAt(double x, double y) const {
    return _qgcompound.getElementAt(x, y);
}

int QGCanvas::getElementCount() const {
    return _qgcompound.getElementCount();
}

std::string QGCanvas::getFilename() const {
    return _filename;
}

_Internal_QWidget* QGCanvas::getInternalWidget() const {
    return _iqcanvas;
}

int QGCanvas::getPixel(double x, double y) const {
    ensureBackgroundImageConstHack();
    return _backgroundImage->pixel((int) x, (int) y) & 0x00ffffff;
}

int QGCanvas::getPixelARGB(double x, double y) const {
    ensureBackgroundImageConstHack();
    return _backgroundImage->pixel((int) x, (int) y);
}

Grid<int> QGCanvas::getPixels() const {
    ensureBackgroundImageConstHack();
    Grid<int> grid((int) getHeight(), (int) getWidth());
    for (int y = 0; y < (int) getHeight(); y++) {
        for (int x = 0; x < (int) getWidth(); x++) {
            grid[y][x] = _backgroundImage->pixel(x, y) & 0x00ffffff;
        }
    }
    return grid;
}

Grid<int> QGCanvas::getPixelsARGB() const {
    ensureBackgroundImageConstHack();
    Grid<int> grid((int) getHeight(), (int) getWidth());
    for (int y = 0; y < (int) getHeight(); y++) {
        for (int x = 0; x < (int) getWidth(); x++) {
            grid[y][x] = _backgroundImage->pixel(x, y);
        }
    }
    return grid;
}

std::string QGCanvas::getType() const {
    return "QGCanvas";
}

QWidget* QGCanvas::getWidget() const {
    return static_cast<QWidget*>(_iqcanvas);
}

bool QGCanvas::isAutoRepaint() const {
    return _qgcompound.isAutoRepaint();
}

void QGCanvas::load(const std::string& filename) {
    ensureThreadSafety("QGCanvas::load");   // TODO: call runOnQtGuiThread?

    // for efficiency, let's at least check whether the file exists
    // and throw error immediately rather than contacting the back-end
    if (!fileExists(filename)) {
        error("QGCanvas::load: file not found: " + filename);
    }
    if (!_backgroundImage->load(QString::fromStdString(filename))) {
        error("QGCanvas::load: failed to load from " + filename);
    }
    _filename = filename;
    QGInteractor::setSize(_backgroundImage->width(), _backgroundImage->height());
    // setSize(_qimage->width(), _qimage->height());
    conditionalRepaint();
}

void QGCanvas::notifyOfResize(double width, double height) {
    if (_backgroundImage) {
        // make new image buffer of the new size
        QImage* newImage = new QImage((int) width, (int) height, QImage::Format_ARGB32);
        newImage->fill(_backgroundColorInt);

        // draw any previous contents onto it
        QPainter painter(newImage);
        painter.drawImage(0, 0, *_backgroundImage);

        // TODO: delete _backgroundImage;
        _backgroundImage = newImage;
        conditionalRepaint();
    }
}

void QGCanvas::remove(QGObject* gobj) {
    _qgcompound.remove(gobj);
}

void QGCanvas::remove(QGObject& gobj) {
    _qgcompound.remove(gobj);
}

void QGCanvas::removeAll() {
    _qgcompound.removeAll();
}

void QGCanvas::removeClickHandler() {
    removeEventHandler("click");
}

void QGCanvas::removeKeyHandler() {
    removeEventHandlers({"keypress",
                         "keyrelease",
                         "keytype"});
}

void QGCanvas::removeMouseHandler() {
    removeEventHandlers({"click",
                         "mousedrag",
                         "mouseenter",
                         "mouseexit",
                         "mousemove",
                         "mousepress",
                         "mouserelease",
                         "mousewheeldown",
                         "mousewheelup"});
}

void QGCanvas::repaint() {
    _qgcompound.repaint();
}

void QGCanvas::repaintRegion(int x, int y, int width, int height) {
    _qgcompound.repaintRegion(x, y, width, height);
}

void QGCanvas::resize(double width, double height, bool /* retain */) {
    checkSize("resize", width, height);

    // TODO
    setSize(width, height);

//    bool wasZero = (floatingPointEqual(_width, 0)
//                    && floatingPointEqual(_height, 0));
//    _width = width;
//    _height = height;
//    if (wasZero) {
//        if (width > 0 && height > 0) {
//            init(_width, _height, _backgroundColor);
//        } // else still zero
//    } else {
//        // was non-zero
//        _pixels.resize((int) _height, (int) _width, retain);
//        // stanfordcpplib::getPlatform()->qgbufferedimage_resize(this, width, height, retain);
//        if (!retain && _backgroundColor != 0x0) {
//            _pixels.fill(_backgroundColor);
//        }
//    }

    conditionalRepaint();
}

void QGCanvas::save(const std::string& filename) {
    ensureThreadSafety("QGCanvas::save");   // TODO: call runOnQtGuiThread?
    ensureBackgroundImage();
    if (!_qgcompound.isEmpty()) {
        // flatten image in a copy object, then save
        QImage imageCopy = this->_backgroundImage->copy(0, 0, (int) getWidth(), (int) getHeight());
        QPainter painter(&imageCopy);
        painter.setRenderHint(QPainter::Antialiasing, QGObject::isAntiAliasing());
        painter.setRenderHint(QPainter::TextAntialiasing, QGObject::isAntiAliasing());
        _qgcompound.draw(&painter);
        painter.end();
        if (!imageCopy.save(QString::fromStdString(filename))) {
            error("QGCanvas::save: failed to save to " + filename);
        }
    } else {
        // save it myself
        if (!_backgroundImage->save(QString::fromStdString(filename))) {
            error("QGCanvas::save: failed to save to " + filename);
        }
    }
    _filename = filename;
}

void QGCanvas::setAutoRepaint(bool autoRepaint) {
    _qgcompound.setAutoRepaint(autoRepaint);
}

void QGCanvas::setBackground(int color) {
    QGDrawingSurface::setBackground(color);
    QGInteractor::setBackground(color);
    if (_backgroundImage) {
        // Slightly confusing semantics here;
        // if you draw some shapes on a canvas (with draw() variants) and then set its background,
        // the shapes will get wiped out.
        // The lesson is, set the background first before drawing stuff.
        // Or add your shapes using add() rather than draw() so they sit atop the background.
        _backgroundImage->fill(color);
        conditionalRepaint();
    }
}

void QGCanvas::setBackground(const std::string& color) {
    setBackground(QGColor::convertColorToRGB(color));
}

void QGCanvas::setClickHandler(QGEventHandler func) {
    setEventHandler("click", func);
}

void QGCanvas::setClickHandler(QGEventHandlerVoid func) {
    setEventHandler("click", func);
}

void QGCanvas::setColor(int color) {
    QGDrawingSurface::setColor(color);
    QGInteractor::setColor(color);
}

void QGCanvas::setColor(const std::string& color) {
    setColor(QGColor::convertColorToRGB(color));
}

void QGCanvas::setFont(const std::string& font) {
    QGDrawingSurface::setFont(font);
    QGInteractor::setFont(font);
}

void QGCanvas::setKeyHandler(QGEventHandler func) {
    _iqcanvas->setFocusPolicy(Qt::StrongFocus);
    setEventHandlers({"keypress",
                      "keyrelease",
                      "keytype"}, func);
}

void QGCanvas::setKeyHandler(QGEventHandlerVoid func) {
    _iqcanvas->setFocusPolicy(Qt::StrongFocus);
    setEventHandlers({"keypress",
                      "keyrelease",
                      "keytype"}, func);
}

void QGCanvas::setMouseHandler(QGEventHandler func) {
    setEventHandlers({"click",
                      "mousedrag",
                      "mouseenter",
                      "mouseexit",
                      "mousemove",
                      "mousepress",
                      "mouserelease",
                      "mousewheeldown",
                      "mousewheelup"}, func);
}

void QGCanvas::setMouseHandler(QGEventHandlerVoid func) {
    setEventHandlers({"click",
                      "mousedrag",
                      "mouseenter",
                      "mouseexit",
                      "mousemove",
                      "mousepress",
                      "mouserelease",
                      "mousewheeldown",
                      "mousewheelup"}, func);
}

void QGCanvas::setPixel(double x, double y, int rgb) {
    ensureBackgroundImage();
    _backgroundImage->setPixel((int) x, (int) y, rgb | 0xff000000);
    conditionalRepaintRegion((int) x, (int) y, /* width */ 1, /* height */ 1);
}

void QGCanvas::setPixel(double x, double y, int r, int g, int b) {
    ensureBackgroundImage();
    _backgroundImage->setPixel((int) x, (int) y, QGColor::convertRGBToRGB(r, g, b) | 0xff000000);
    conditionalRepaintRegion((int) x, (int) y, /* width */ 1, /* height */ 1);
}

void QGCanvas::setPixelARGB(double x, double y, int argb) {
    ensureBackgroundImage();
    _backgroundImage->setPixel((int) x, (int) y, argb);
    conditionalRepaintRegion((int) x, (int) y, /* width */ 1, /* height */ 1);
}

void QGCanvas::setPixelARGB(double x, double y, int a, int r, int g, int b) {
    ensureBackgroundImage();
    _backgroundImage->setPixel((int) x, (int) y, QGColor::convertARGBToARGB(a, r, g, b));
    conditionalRepaintRegion((int) x, (int) y, /* width */ 1, /* height */ 1);
}

void QGCanvas::setPixels(const Grid<int>& pixels) {
    ensureBackgroundImage();
    if (pixels.width() != (int) getWidth() || pixels.height() != (int) getHeight()) {
        // TODO
        // resize(pixels.width(), pixels.height());
        error("QGCanvas::setPixels: wrong size");
    }
    for (int y = 0; y < pixels.height(); y++) {
        for (int x = 0; x < pixels.width(); x++) {
            _backgroundImage->setPixel(x, y, pixels[y][x]);
        }
    }
    conditionalRepaint();
}

void QGCanvas::setPixelsARGB(const Grid<int>& pixels) {
    ensureBackgroundImage();
    if (pixels.width() != (int) getWidth() || pixels.height() != (int) getHeight()) {
        // TODO
        // resize(pixels.width(), pixels.height());
        error("QGCanvas::setPixels: wrong size");
    }
    for (int y = 0; y < pixels.height(); y++) {
        for (int x = 0; x < pixels.width(); x++) {
            _backgroundImage->setPixel(x, y, pixels[y][x]);
        }
    }
    conditionalRepaint();
}

Grid<int> QGCanvas::toGrid() const {
    Grid<int> grid;
    toGrid(grid);
    return grid;
}

void QGCanvas::toGrid(Grid<int>& grid) const {
    grid.resize(getHeight(), getWidth());
    for (int row = 0, width = (int) getWidth(), height = (int) getHeight(); row < height; row++) {
        for (int col = 0; col < width; col++) {
            grid[row][col] = _backgroundImage->pixel(col, row);
        }
    }
}
