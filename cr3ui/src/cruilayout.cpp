/*
 * cruilayout.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruilayout.h"
#include "crui.h"


using namespace CRUI;

// Vertical Layout
/// measure dimensions
void CRUILinearLayout::measure(int baseWidth, int baseHeight) {
	lvRect padding;
	getPadding(padding);
	lvRect margin = getMargin();
    int maxw = baseWidth;
    int maxh = baseHeight;
    if (getMaxWidth() != UNSPECIFIED && maxw > getMaxWidth())
        maxw = getMaxWidth();
    if (getMaxHeight() != UNSPECIFIED && maxh > getMaxHeight())
        maxh = getMaxHeight();
    maxw = maxw - (margin.left + margin.right + padding.left + padding.right);
    maxh = maxh - (margin.top + margin.bottom + padding.top + padding.bottom);
	int totalWeight = 0;
	if (_isVertical) {
		int biggestw = 0;
		int totalh = 0;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			if (child->getLayoutHeight() == CRUI::FILL_PARENT) {
				totalWeight += child->getLayoutWeight();
			} else {
				child->measure(maxw, maxh);
				totalh += child->getMeasuredHeight();
				if (biggestw < child->getMeasuredWidth())
					biggestw = child->getMeasuredWidth();
			}
		}
		int hleft = maxh - totalh;
		if (totalWeight < 1)
			totalWeight = 1;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			if (child->getLayoutHeight() == CRUI::FILL_PARENT) {
				int h = hleft * child->getLayoutWeight() / totalWeight;
				child->measure(maxw, h);
				totalh += child->getMeasuredHeight();
				if (biggestw < child->getMeasuredWidth())
					biggestw = child->getMeasuredWidth();
			}
		}
		if (biggestw > maxw)
			biggestw = maxw;
		if (totalh > maxh)
			totalh = maxh;
		defMeasure(baseWidth, baseHeight, biggestw, totalh);
	} else {
		int biggesth = 0;
		int totalw = 0;

		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			if (child->getLayoutWidth() == CRUI::FILL_PARENT) {
				totalWeight += child->getLayoutWeight();
			} else {
				child->measure(maxw, maxh);
				totalw += child->getMeasuredWidth();
				if (biggesth < child->getMeasuredHeight())
					biggesth = child->getMeasuredHeight();
			}
		}
		int wleft = maxw - totalw;
		if (totalWeight < 1)
			totalWeight = 1;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			if (child->getLayoutWidth() == CRUI::FILL_PARENT) {
				int w = wleft * child->getLayoutWeight() / totalWeight;
				child->measure(w, maxh);
				totalw += child->getMeasuredWidth();
				if (biggesth < child->getMeasuredHeight())
					biggesth = child->getMeasuredHeight();
			}
		}
		if (biggesth > maxh)
			biggesth = maxh;
		if (totalw > maxw)
			totalw = maxw;
		defMeasure(baseWidth, baseHeight, totalw, biggesth);
	}
}

/// updates widget position based on specified rectangle
void CRUILinearLayout::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
	lvRect clientRc = _pos;
	applyMargin(clientRc);
	applyPadding(clientRc);
	lvRect childRc = clientRc;
	if (_isVertical) {
		int y = childRc.top;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			childRc.top = y;
			if (i < getChildCount() - 1)
				childRc.bottom = y + child->getMeasuredHeight();
			else
				childRc.bottom = clientRc.bottom;
			if (childRc.top > clientRc.bottom)
				childRc.top = clientRc.bottom;
			if (childRc.bottom > clientRc.bottom)
				childRc.bottom = clientRc.bottom;
			child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			y = childRc.bottom;
		}
	} else {
		int x = childRc.left;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			childRc.left = x;
			if (i < getChildCount() - 1)
				childRc.right = x + child->getMeasuredWidth();
			else
				childRc.right = clientRc.right;
			if (childRc.left > clientRc.right)
				childRc.left = clientRc.right;
			if (childRc.right > clientRc.right)
				childRc.right = clientRc.right;
			child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			x = childRc.right;
		}
	}
}



//=======================================================================
// Container Widget

/// draws widget with its children to specified surface
void CRUIContainerWidget::draw(LVDrawBuf * buf) {
	CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	for (int i=0; i<getChildCount(); i++) {
		getChild(i)->draw(buf);
	}
}




