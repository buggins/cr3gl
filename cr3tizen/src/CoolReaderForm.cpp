#include "CoolReaderForm.h"

using namespace Tizen::Base;
using namespace Tizen::App;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;

CoolReaderForm::CoolReaderForm(void)
{
}

CoolReaderForm::~CoolReaderForm(void)
{
}

bool
CoolReaderForm::Initialize(void)
{
	Construct(FORM_STYLE_NORMAL);

	return true;
}
result
CoolReaderForm::OnInitializing(void)
{
	result r = E_SUCCESS;

	// TODO:
	// Add your initialization code here

	// Setup back event listener
	SetFormBackEventListener(this);

	// Get a button via resource ID
	Tizen::Ui::Controls::Button *pButtonOk = static_cast<Button*>(GetControl(L"IDC_BUTTON_OK"));
	if (pButtonOk != null)
	{
		pButtonOk->SetActionId(ID_BUTTON_OK);
		pButtonOk->AddActionEventListener(*this);
	}

	return r;
}

result
CoolReaderForm::OnTerminating(void)
{
	result r = E_SUCCESS;

	// TODO: Add your termination code here

	return r;
}

void
CoolReaderForm::OnActionPerformed(const Tizen::Ui::Control& source, int actionId)
{
	switch(actionId)
	{
	case ID_BUTTON_OK:
		{
			AppLog("OK Button is clicked!\n");
		}
		break;
	default:
		break;
	}
}

void
CoolReaderForm::OnFormBackRequested(Tizen::Ui::Controls::Form& source)
{
	UiApp* pApp = UiApp::GetInstance();
	AppAssert(pApp);
	pApp->Terminate();
}


