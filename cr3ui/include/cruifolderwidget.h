/*
 * cruifolderwidget.h
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

#ifndef CRUIFOLDERWIDGET_H_
#define CRUIFOLDERWIDGET_H_

#include "cruilist.h"

class CRUITitleBarWidget;
class CRUIFileListWidget;

class CRUIFolderWidget : public CRUIContainerWidget {
	CRUITitleBarWidget * _title;
	CRUIFileListWidget * _fileList;
public:
	CRUIFolderWidget();
	virtual ~CRUIFolderWidget();
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
};


#endif /* CRUIFOLDERWIDGET_H_ */
