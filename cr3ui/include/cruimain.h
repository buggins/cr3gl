#ifndef CRUIMAIN_H
#define CRUIMAIN_H

#include "crui.h"
#include "cruifolderwidget.h"
#include "cruihomewidget.h"
#include "cruireadwidget.h"

enum VIEW_MODE {
    MODE_HOME,
    MODE_FOLDER,
    MODE_READ,
};

class CRUIMainWidget : public CRUIWidget, public CRDirScanCallback {
    CRUIHomeWidget * _home;
    CRUIFolderWidget * _folder;
    CRUIReadWidget * _read;
    CRUIWidget * _currentWidget;
    VIEW_MODE _mode;
    lString8 _currentFolder;
    lString8 _pendingFolder;
    void setMode(VIEW_MODE mode);
public:
    virtual void onDirectoryScanFinished(CRDirCacheItem * item);
    virtual int getChildCount();
    virtual CRUIWidget * getChild(int index);
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    /// returns true if widget is child of this
    virtual bool isChild(CRUIWidget * widget);

    void showFolder(lString8 folder);
    void showHome();

    void recreate();
    CRUIMainWidget();
    virtual ~CRUIMainWidget();
};

#endif // CRUIMAIN_H
