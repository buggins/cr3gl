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
#include "CR3Renderer.h"


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

class CR3Renderer;
class CoolReaderFrame
	: public Tizen::Ui::Controls::Frame
	, public Tizen::Ui::IOrientationEventListener
{
		CR3Renderer * _renderer;
public:
	CoolReaderFrame(void);
	virtual ~CoolReaderFrame(void);

	void setRenderer(CR3Renderer * renderer);
//	CoolReaderForm* _pCoolReaderForm;
//	CoolReaderForm* getForm() { return _pCoolReaderForm; }
public:
	/**
	 * Called when an orientation event occurs.
	 *
	 * @since	2.0
	 *
	 * @param[in]   source				The source of the event
	 * @param[in]   orientationStatus	The information about the orientation event
	 * @remarks		The orientation changed event is fired on Control for which orientation mode change has been enabled by calling SetOrientation().
	 * @see		Tizen::Ui::Controls::Frame
	 * @see		Tizen::Ui::Controls::Form
	 */
	virtual void OnOrientationChanged(const Tizen::Ui::Control& source, Tizen::Ui::OrientationStatus orientationStatus);
	virtual void OnUserEventReceivedN (RequestId requestId, Tizen::Base::Collection::IList *pArgs);
	virtual result OnInitializing(void);
	virtual result OnTerminating(void);
};

#endif  //_COOLREADERFRAME_H_
