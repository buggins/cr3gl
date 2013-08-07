/*
 * glui.cpp
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */


#include "glui.h"

CRUIWidget::CRUIWidget() {

}

CRUIWidget::~CRUIWidget() {

}

/// measure dimensions
void CRUIWidget::measure(lvRect & baseRect, lvPoint & size) {
	size.x = baseRect.width();
	size.y = baseRect.height();
}

/// updates widget position based on specified rectangle
void CRUIWidget::layout(lvRect & pos) {
	_pos = pos;
}

/// draws widget with its children to specified surface
void CRUIWidget::draw(LVDrawBuf * buf) {

}
