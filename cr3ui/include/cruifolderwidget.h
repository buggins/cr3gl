/*
 * cruifolderwidget.h
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

#ifndef CRUIFOLDERWIDGET_H_
#define CRUIFOLDERWIDGET_H_

#include "cruilist.h"
#include "fileinfo.h"

class CRUITitleBarWidget;
class CRUIFileListWidget;

class CRUIFolderWidget : public CRUILinearLayout {
	CRUITitleBarWidget * _title;
	CRUIFileListWidget * _fileList;
	CRDirCacheItem * _dir;
public:
	virtual void setDirectory(CRDirCacheItem * _dir);
	CRUIFolderWidget();
	virtual ~CRUIFolderWidget();
};


#endif /* CRUIFOLDERWIDGET_H_ */
