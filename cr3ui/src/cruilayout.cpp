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
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
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
            if (child->getVisibility() == GONE) {
                // handle GONE
            } else if (child->getLayoutHeight() == CRUI::FILL_PARENT) {
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
            if (child->getVisibility() == GONE) {
                // handle GONE
            } else if (child->getLayoutHeight() == CRUI::FILL_PARENT) {
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
            if (child->getVisibility() == GONE) {
                // handle GONE
            } else if (child->getLayoutWidth() == CRUI::FILL_PARENT) {
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
            if (child->getVisibility() == GONE) {
                // handle GONE
            } else if (child->getLayoutWidth() == CRUI::FILL_PARENT) {
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
    applyAlign(_pos, _measuredWidth, _measuredHeight);
    lvRect clientRc = _pos;
	applyMargin(clientRc);
	applyPadding(clientRc);

	lvRect childRc = clientRc;
	if (_isVertical) {
		int y = childRc.top;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			childRc.top = y;
            if (child->getVisibility() == GONE) {
                // handle GONE
                childRc.bottom = childRc.top;
            } else {
                if (i < getChildCount() - 1)
                    childRc.bottom = y + child->getMeasuredHeight();
                else
                    childRc.bottom = clientRc.bottom;
                if (childRc.top > clientRc.bottom)
                    childRc.top = clientRc.bottom;
                if (childRc.bottom > clientRc.bottom)
                    childRc.bottom = clientRc.bottom;
            }
			child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			y = childRc.bottom;
		}
	} else {
		int x = childRc.left;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
            childRc.left = x;
            if (child->getVisibility() == GONE) {
                // handle GONE
                childRc.right = childRc.left;
            } else {
                if (i < getChildCount() - 1)
                    childRc.right = x + child->getMeasuredWidth();
                else
                    childRc.right = clientRc.right;
                if (childRc.left > clientRc.right)
                    childRc.left = clientRc.right;
                if (childRc.right > clientRc.right)
                    childRc.right = clientRc.right;
            }
            child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
            x = childRc.right;
        }
	}
}



//=======================================================================
// Container Widget

/// draws widget with its children to specified surface
void CRUIContainerWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	for (int i=0; i<getChildCount(); i++) {
        if (getChild(i)->getVisibility() == VISIBLE)
            getChild(i)->draw(buf);
	}
}

struct Cell {
    int index;
    int width;
    int height;
    int weight;
    bool hfill;
    bool vfill;
    Cell() : index(0), width(0), height(0), weight(1), hfill(false), vfill(false) {}
};

lvPoint CRUITableLayout::layoutTable(LVArray<Col> &_cols, LVArray<Row> & _rows, int maxw, int maxh, bool fillx, bool filly) {
    lvPoint sz(maxw, maxh);
    // init cells
    LVPtrVector<Cell> _cells;
    int rowcount = (getChildCount() + _colCount - 1) / _colCount;
    for (int i = 0; i < rowcount; i++) {
        Row row;
        _rows.add(row);
    }
    for (int i = 0; i < _colCount; i++) {
        Col col;
        _cols.add(col);
    }
    for (int row = 0; row < rowcount; row++) {
        for (int col = 0; col < _colCount; col++) {
            int i = row * _colCount + col;
            Cell * cell = new Cell();
            cell->index = i;
            if (i < getChildCount()) {
                CRUIWidget * child = getChild(i);
                cell->vfill = child->getLayoutHeight() == FILL_PARENT;
                cell->hfill = child->getLayoutWidth() == FILL_PARENT;
                child->measure(maxw, maxh);
                cell->width = child->getMeasuredWidth();
                cell->height = child->getMeasuredHeight();
                cell->weight = child->getLayoutWeight();
                if (cell->hfill) {
                    _cols[col].fill = true;
                    if (_cols[col].weight < cell->weight)
                        _cols[col].weight = cell->weight;
                } else {
                    if (_cols[col].maxw < cell->width)
                        _cols[col].maxw = cell->width;
                }
            }
            _cells.add(cell);
        }
    }
    int colw = maxw / _colCount;
    int totalcolweight = 0;
    int totalcolwidth = 0;
    for (int i = 0; i < _colCount; i++) {
        if (_cols[i].maxw > colw) {
            _cols[i].fill = true;
            _cols[i].weight = 1;
        }
        if (_cols[i].fill) {
            totalcolweight += _cols[i].weight;
        } else {
            totalcolwidth += _cols[i].maxw;
        }
    }
    // distribute extra space between FILL cols
    int wleft = maxw - totalcolwidth;
    for (int i = 0; i < _colCount; i++) {
        if (_cols[i].fill) {
            _cols[i].width = wleft * _cols[i].weight / totalcolweight;
            totalcolwidth += _cols[i].width;
        } else {
            _cols[i].width = _cols[i].maxw;
        }
    }
    // fill H space
    if (fillx && totalcolwidth < maxw) {
        int extraw = (maxw - totalcolwidth) / _colCount;
        if (extraw > 0) {
            for (int i = 0; i < _colCount; i++) {
                _cols[i].width += extraw;
            }
        }
    }
    // init row params
    for (int row = 0; row < rowcount; row++) {
        for (int col = 0; col < _colCount; col++) {
            int i = row * _colCount + col;
            Cell * cell = _cells[i];
            if (i < getChildCount()) {
                CRUIWidget * child = getChild(i);
                child->measure(_cols[col].width, maxh);
                cell->width = child->getMeasuredWidth();
                cell->height = child->getMeasuredHeight();
                if (cell->vfill) {
                    _rows[row].fill = true;
                    if (_rows[row].weight < cell->weight)
                        _rows[row].weight = cell->weight;
                } else {
                    if (_rows[row].maxh < cell->height)
                        _rows[row].maxh = cell->height;
                }
            }
        }
    }
    // update row heights
    int rowh = maxh / rowcount;
    int totalrowweight = 0;
    int totalrowheight = 0;
    for (int i = 0; i < rowcount; i++) {
        if (_rows[i].maxh > rowh) {
            _rows[i].fill = true;
            _rows[i].weight = 1;
        }
        if (_rows[i].fill) {
            totalrowweight += _rows[i].weight;
        } else {
            totalrowheight += _rows[i].maxh;
        }
    }
    // distribute extra space between FILL rows
    int hleft = maxh - totalrowheight;
    for (int i = 0; i < rowcount; i++) {
        if (_rows[i].fill) {
            _rows[i].height = hleft * _rows[i].weight / totalrowweight;
            totalrowheight += _rows[i].height;
        } else {
            _rows[i].height = _rows[i].maxh;
        }
    }
    // fill V space
    if (filly && totalrowheight < maxh) {
        int extrah = (maxh - totalrowheight) / rowcount;
        if (extrah > 0) {
            for (int i = 0; i < rowcount; i++) {
                _rows[i].height += extrah;
            }
        }
    }
    // finally measure cells with known row and col sizes
    for (int row = 0; row < rowcount; row++) {
        for (int col = 0; col < _colCount; col++) {
            int i = row * _colCount + col;
            Cell * cell = _cells[i];
            if (i < getChildCount()) {
                CRUIWidget * child = getChild(i);
                child->measure(_cols[col].width, _rows[row].height);
                cell->width = _cols[col].width;
                cell->height = _rows[row].height;
            }
        }
    }
    // calc size
    if (!totalcolweight && totalcolwidth < maxw)
        sz.x = totalcolwidth;
    else
        sz.x = maxw;
    if (!totalrowweight && totalrowheight < maxh)
        sz.y = totalrowheight;
    else
        sz.y = maxh;
    return sz;
}

/// measure dimensions
void CRUITableLayout::measure(int baseWidth, int baseHeight) {
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
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

    LVArray<Col> _cols;
    LVArray<Row> _rows;
    lvPoint sz = layoutTable(_cols, _rows, maxw, maxh, getLayoutWidth() == FILL_PARENT, getLayoutHeight() == FILL_PARENT);
    defMeasure(baseWidth, baseHeight, sz.x, sz.y);
}

/// updates widget position based on specified rectangle
void CRUITableLayout::layout(int left, int top, int right, int bottom) {
    _pos.left = left;
    _pos.top = top;
    _pos.right = right;
    _pos.bottom = bottom;
    if (!getChildCount())
        return;
    applyAlign(_pos, _measuredWidth, _measuredHeight);
    lvRect clientRc = _pos;
    applyMargin(clientRc);
    applyPadding(clientRc);

    lvRect childRc = clientRc;

    LVArray<Col> _cols;
    LVArray<Row> _rows;
    layoutTable(_cols, _rows, childRc.width(), childRc.height(), true, true);

    int rowcount = (getChildCount() + _colCount - 1) / _colCount;
    int y = childRc.top;
    // finally measure cells with known row and col sizes
    for (int row = 0; row < rowcount; row++) {
        int x = childRc.left;
        for (int col = 0; col < _colCount; col++) {
            int i = row * _colCount + col;
            if (i < getChildCount()) {
                CRUIWidget * child = getChild(i);
                child->layout(x, y, x + _cols[col].width, y + _rows[row].height);
            }
            x += _cols[col].width;
        }
        y += _rows[row].height;
    }
}

