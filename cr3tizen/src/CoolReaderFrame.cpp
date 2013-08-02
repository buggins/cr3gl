#include "CoolReaderFrame.h"
#include "CoolReaderForm.h"

using namespace Tizen::Base;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;

CoolReaderFrame::CoolReaderFrame(void)
{
}

CoolReaderFrame::~CoolReaderFrame(void)
{
}

result
CoolReaderFrame::OnInitializing(void)
{
	result r = E_SUCCESS;

	// Create a form
	CoolReaderForm* pCoolReaderForm = new CoolReaderForm();
	pCoolReaderForm->Initialize();

	// Add the form to the frame
	AddControl(pCoolReaderForm);

	// Set the current form
	SetCurrentForm(pCoolReaderForm);

	// Draw the form
	pCoolReaderForm->Invalidate(true);

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


