#include "CoolReaderFrame.h"
#include "CoolReaderForm.h"
#include "lvstring.h"

using namespace Tizen::Base;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;

CoolReaderFrame::CoolReaderFrame(void)// : _pCoolReaderForm(NULL)
{
}

CoolReaderFrame::~CoolReaderFrame(void)
{
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

	// Create a form
	CoolReaderForm* _pCoolReaderForm = new CoolReaderForm();
	_pCoolReaderForm->Initialize();

	// Add the form to the frame
	AddControl(_pCoolReaderForm);

	// Set the current form
	SetCurrentForm(_pCoolReaderForm);

	// Draw the form
	_pCoolReaderForm->Invalidate(true);

	// TODO: Add your initialization code here

	return r;
}

result
CoolReaderFrame::OnTerminating(void)
{
	result r = E_SUCCESS;

	// TODO: Add your termination code here

	return r;
}


