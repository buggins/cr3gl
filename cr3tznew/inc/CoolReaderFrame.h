#ifndef _COOLREADERFRAME_H_
#define _COOLREADERFRAME_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FUi.h>
#include <FUiIme.h>
#include <FGraphics.h>
#include <gl.h>
#include "crconcurrent.h"


class CRRunnableContainer : public Tizen::Base::Object
{
	CRRunnable * _runnable;
public:
	CRRunnableContainer(CRRunnable * runnable) : _runnable(runnable) {}
	void run() {
		_runnable->run();
	}
	~CRRunnableContainer() {
		delete _runnable;
	}
};

#define UI_UPDATE_REQUEST 12345

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
	virtual void OnUserEventReceivedN (RequestId requestId, Tizen::Base::Collection::IList *pArgs);
	virtual result OnInitializing(void);
	virtual result OnTerminating(void);
};

#endif  //_COOLREADERFRAME_H_
