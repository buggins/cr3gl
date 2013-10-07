#include "CoolReaderFrame.h"
#include "lvstring.h"
#include "CR3Renderer.h"

using namespace Tizen::Base;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;

CoolReaderFrame::CoolReaderFrame(void)// : _pCoolReaderForm(NULL)
: _renderer(NULL)
{
}

CoolReaderFrame::~CoolReaderFrame(void)
{
}

void CoolReaderFrame::setRenderer(CR3Renderer * renderer) {
	_renderer = renderer;
	AddTouchEventListener(*_renderer->getEventAdapter());
	AddKeyEventListener(*_renderer->getEventAdapter());
}

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
void CoolReaderFrame::OnOrientationChanged(const Tizen::Ui::Control& source, Tizen::Ui::OrientationStatus orientationStatus) {
	CRLog::trace("OnOrientationChanged: %d", (int)orientationStatus);
	if (_renderer) {
		int dx = GetWidth();
		int dy = GetHeight();
		CRLog::info("Screen orientation: %d, size %dx%d", (int)orientationStatus, dx, dy);
		_renderer->SetTargetControlWidth(dx);
		_renderer->SetTargetControlHeight(dy);
		_renderer->setScreenUpdateMode(true, 15);
	}
	Invalidate(true);
}

void CoolReaderFrame::OnUserEventReceivedN (RequestId requestId, Tizen::Base::Collection::IList *pArgs) {
	CRLog::trace("UserEvent %d received", requestId);
	if (requestId == UI_UPDATE_REQUEST) {
		//
		if (pArgs != NULL && pArgs->GetCount() == 1) {
			CRRunnableContainer * param = dynamic_cast<CRRunnableContainer*>(pArgs->GetAt(0));
			if (param) {
				CRLog::trace("Executing UI_UPDATE_REQUEST in UI thread");
				param->run();
			}
		}
	}
}

result
CoolReaderFrame::OnInitializing(void)
{
	result r = E_SUCCESS;
	AddOrientationEventListener(*this);
	SetOrientation(ORIENTATION_AUTOMATIC);
	//SetOrientation(ORIENTATION_LANDSCAPE);

//	// Create a form
//	CoolReaderForm* _pCoolReaderForm = new CoolReaderForm();
//	_pCoolReaderForm->Initialize();
//
//	// Add the form to the frame
//	AddControl(_pCoolReaderForm);
//
//	// Set the current form
//	SetCurrentForm(_pCoolReaderForm);
//
//	// Draw the form
//	_pCoolReaderForm->Invalidate(true);

	// TODO: Add your initialization code here

	return r;
}

result
CoolReaderFrame::OnTerminating(void)
{
	result r = E_SUCCESS;

	RemoveOrientationEventListener(*this);

	// TODO: Add your termination code here

	return r;
}


