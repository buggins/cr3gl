#ifndef CRUICOVERWIDGET_H
#define CRUICOVERWIDGET_H

#include "cruiwidget.h"
#include "fileinfo.h"

class CRUIMainWidget;

class CRCoverWidget : public CRUIWidget {
protected:
    CRUIMainWidget * _main;
    CRDirEntry * _book;
    int _dx;
    int _dy;
public:
    /// will own passed book w/o cloning
    CRCoverWidget(CRUIMainWidget * main, CRDirEntry * book, int dx, int dy);
    ~CRCoverWidget() { if (_book) delete _book; }
    /// calculates cover image size (to request in cache) by control size
    lvPoint calcCoverSize(int width, int height);
    virtual void setSize(int width, int height);
    /// sets book to clone of specified item
    virtual void setBook(const CRDirEntry * book);
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
};

#endif // CRUICOVERWIDGET_H
