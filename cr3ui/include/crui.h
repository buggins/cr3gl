/*
 * glui.h
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */

#ifndef GLUI_H_
#define GLUI_H_

#include "cruievent.h"
#include "cruiwidget.h"
#include "cruilayout.h"
#include "cruicontrols.h"
#include "cruilist.h"
#include "cruihomewidget.h"
#include "cruifolderwidget.h"
#include "fileinfo.h"

struct CRUIDeviceInfo {
	int dpi;
	int shortSide;
	int longSide;
	int shortSideMillimeters;
	int longSideMillimeters;
	int minListItemSize;
	int pixelsToMm(int pixels);
	int mmToPixels(int mm);
	int pixelsToPt(int pixels);
	int ptToPixels(int mm);
    bool isSizeChanged(int newDx, int newDy);
    CRTopDirList topDirs;
	CRUIDeviceInfo();
	void setScreenDimensions(int dx, int dy, int dpi);
};

extern CRUIDeviceInfo deviceInfo;

// convert pixels to millimeters
inline int PX_TO_MM(int px) { return deviceInfo.pixelsToMm(px); }
// convert pixels to millimeters
inline int PX_TO_PT(int px) { return deviceInfo.pixelsToPt(px); }
// convert millimeters to pixels
inline int MM_TO_PX(int mm) { return deviceInfo.mmToPixels(mm); }
// convert millimeters to pixels
inline int PT_TO_PX(int mm) { return deviceInfo.ptToPixels(mm); }
// minimum pixel size of touch UI element
#define MIN_ITEM_PX (deviceInfo.minListItemSize)

lUInt64 GetCurrentTimeMillis();


#endif /* GLUI_H_ */
