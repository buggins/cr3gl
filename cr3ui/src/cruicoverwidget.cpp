#include "cruicoverwidget.h"
#include "crcoverpages.h"
#include "cruimain.h"

using namespace CRUI;

bool CRCoverWidget::setSize(int width, int height) {
    if (_dx == width && _dy == height)
        return false; // no change
    // changed
    _dx = width;
    _dy = height;
    setMinWidth(width);
    setMaxWidth(width);
    setMinHeight(height);
    setMaxHeight(height);
    requestLayout(true);
    return true;
}

void CRCoverWidget::setBook(const CRDirEntry * book) {
    if (_book)
        delete _book;
    _book = book ? book->clone() : NULL;
    invalidate();
}

/// measure dimensions
void CRCoverWidget::measure(int baseWidth, int baseHeight) {
    CR_UNUSED2(baseWidth, baseHeight);
    _measuredWidth = _dx;
    _measuredHeight = _dy;
}

/// updates widget position based on specified rectangle
void CRCoverWidget::layout(int left, int top, int right, int bottom) {
    CRUIWidget::layout(left, top, right, bottom);
}
/// draws widget with its children to specified surface
void CRCoverWidget::draw(LVDrawBuf * buf) {
    CRUIWidget::draw(buf);
    if (!_book)
        return;
    LVDrawStateSaver saver(*buf);
    lvRect rc = _pos;
    applyMargin(rc);
    setClipRect(buf, rc);
    applyPadding(rc);

    int width = rc.width();
    int height = rc.height();
    // fix proportions
    if (width > height * 8 / 10)
        width = height * 8 / 10;
    else if (height > width * 10 / 8)
        height = width * 10 / 8;
    if (width < 30 || height < 40)
        return; // too small

    LVDrawBuf * cover = coverPageManager->getIfReady(_book, width, height);
    if (!cover) {
        coverPageManager->prepare(_book, width, height, _main->createUpdateCallback());
        return;
    }
    applyAlign(rc, width, height);
    // don't scale
    // draw
    buf->DrawRescaled(cover, rc.left, rc.top, width, height, 0);
}

CRCoverWidget::CRCoverWidget(CRUIMainWidget * main, CRDirEntry * book, int dx, int dy) : _main(main), _book(book), _dx(64), _dy(64) {
    setSize(dx, dy);
    setAlign(CRUI::ALIGN_CENTER);
    setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
}

lvPoint CRCoverWidget::calcCoverSize(int w, int h) {
    lvRect padding;
    getPadding(padding);
    lvRect margin = getMargin();
    int width = w - padding.left - padding.right - margin.left - margin.right;
    int height = h - padding.top - padding.bottom - margin.top - margin.bottom;
    // fix proportions
    if (width > height * 8 / 10)
        width = height * 8 / 10;
    else if (height > width * 10 / 8)
        height = width * 10 / 8;
    return lvPoint(width, height);
}

