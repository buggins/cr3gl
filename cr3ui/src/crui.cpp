/*
 * glui.cpp
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */


#include "crui.h"

using namespace CRUI;

CRUIDeviceInfo deviceInfo;

CRUIDeviceInfo::CRUIDeviceInfo() {
	setScreenDimensions(600, 800, 160);
}

// 1" == 25.4mm
int CRUIDeviceInfo::pixelsToMm(int pixels){
	return 25 * pixels / dpi;
}

int CRUIDeviceInfo::mmToPixels(int mm) {
	return 25 * mm / dpi;
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
