#ifndef _COOLREADERFRAME_H_
#define _COOLREADERFRAME_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FUi.h>
#include <FUiIme.h>
#include <FGraphics.h>
#include <gl.h>

class CoolReaderForm;
class CoolReaderFrame
	: public Tizen::Ui::Controls::Frame
{
public:
	CoolReaderFrame(void);
	virtual ~CoolReaderFrame(void);
//	CoolReaderForm* _pCoolReaderForm;
//	CoolReaderForm* getForm() { return _pCoolReaderForm; }
public:
	virtual result OnInitializing(void);
	virtual result OnTerminating(void);
};

#endif  //_COOLREADERFRAME_H_
