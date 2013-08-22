/*
 * glui.cpp
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */


#include "crui.h"

using namespace CRUI;

CRUIDeviceInfo deviceInfo;

bool CRUIDeviceInfo::isSizeChanged(int dx, int dy) {
    int _shortSide;
    int _longSide;
    if (dx < dy) {
        _shortSide = dx;
        _longSide = dy;
    } else {
        _shortSide = dy;
        _longSide = dx;
    }
    return shortSide != _shortSide || longSide != _longSide;
}

CRUIDeviceInfo::CRUIDeviceInfo() {
	setScreenDimensions(600, 800, 300);
}

// 1" == 25.4mm
int CRUIDeviceInfo::pixelsToMm(int pixels){
	return 25 * pixels / dpi;
}

int CRUIDeviceInfo::mmToPixels(int mm) {
	return mm * dpi / 25;
}

// 1" == 72pt
int CRUIDeviceInfo::pixelsToPt(int pixels) {
	return 72 * pixels / dpi;
}

int CRUIDeviceInfo::ptToPixels(int pt) {
	return pt * dpi / 72;
}

void CRUIDeviceInfo::setScreenDimensions(int dx, int dy, int _dpi) {
	if (dx < dy) {
		shortSide = dx;
		longSide = dy;
	} else {
		shortSide = dy;
		longSide = dx;
	}
	dpi = _dpi;
	shortSideMillimeters = pixelsToMm(shortSide);
	longSideMillimeters = pixelsToMm(longSide);
	minListItemSize = mmToPixels(8);
}
