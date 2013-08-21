/*
 * cruireadwidget.h
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

#ifndef CRUIREADWIDGET_H_
#define CRUIREADWIDGET_H_


#include "cruiwidget.h"

class CRUIReadWidget : public CRUIWidget {
public:
	CRUIReadWidget();
	virtual ~CRUIReadWidget();
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
};


#endif /* CRUIREADWIDGET_H_ */
