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
	if (_dpi)
		dpi = _dpi;
	shortSideMillimeters = pixelsToMm(shortSide);
	longSideMillimeters = pixelsToMm(longSide);
	minListItemSize = mmToPixels(8);
}


#ifdef _WIN32
static bool __timerInitialized = false;
static double __timeTicksPerMillis;
static lUInt64 __timeStart;
static lUInt64 __timeAbsolute;
static lUInt64 __startTimeMillis;
#endif

void CRReinitTimer() {
#ifdef _WIN32
    LARGE_INTEGER tps;
    QueryPerformanceFrequency(&tps);
    __timeTicksPerMillis = (double)(tps.QuadPart / 1000L);
    LARGE_INTEGER queryTime;
    QueryPerformanceCounter(&queryTime);
    __timeStart = (lUInt64)(queryTime.QuadPart / __timeTicksPerMillis);
    __timerInitialized = true;
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    __startTimeMillis = (ft.dwLowDateTime | (((lUInt64)ft.dwHighDateTime) << 32)) / 10000;
#else
    // do nothing. it's for win32 only
#endif
}


lUInt64 GetCurrentTimeMillis() {
#if defined(LINUX) || defined(ANDROID) || defined(_LINUX)
    timeval ts;
    gettimeofday(&ts, NULL);
    return ts.tv_sec * (lUInt64)1000 + ts.tv_usec / 1000;
#else
 #ifdef _WIN32
    if (!__timerInitialized) {
        CRReinitTimer();
        return __startTimeMillis;
    } else {
        LARGE_INTEGER queryTime;
        QueryPerformanceCounter(&queryTime);
        __timeAbsolute = (lUInt64)(queryTime.QuadPart / __timeTicksPerMillis);
        return __startTimeMillis + (lUInt64)(__timeAbsolute - __timeStart);
    }
 #else
 #error * You should define GetCurrentTimeMillis() *
 #endif
#endif
}

