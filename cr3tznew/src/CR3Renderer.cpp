#include "CR3Renderer.h"
#include "gldrawbuf.h"
#include "glfont.h"
#include <crengine.h>
#include "crui.h"
#include "fileinfo.h"
#include "cruimain.h"
#include "cruiwidget.h"
#include "CoolReader.h"
#include "CoolReaderFrame.h"
#include "FNetHttpIHttpTransactionEventListener.h"
#include "FNetHttpHttpSession.h"
#include "FNetHttpHttpTransaction.h"


using namespace CRUI;
using namespace Tizen::Base;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;
using namespace Tizen::Net::Http;

const GLfloat ONEP = GLfloat(+1.0f);
const GLfloat ONEN = GLfloat(-1.0f);
const GLfloat ZERO = GLfloat( 0.0f);

CR3Renderer::CR3Renderer(CoolReaderApp * app, CoolReaderFrame * frame)
	: _app(app)
	, _frame(frame)
	, _backbuffer(NULL)
	, _keypad(NULL)
	, _keypadShown(false)
	//, _updateRequested(false)
	, __controlWidth(0)
	, __controlHeight(0)
	, __angle(0)
	, __player(NULL)
	, __playerStarted(true)
{
	_eventManager = new CRUIEventManager();
	_eventAdapter = new CRUIEventAdapter(_eventManager);
	_downloadManager = new CRUIHttpTaskManagerTizen(_eventManager);
	_widget = new CRUIMainWidget();
	_eventManager->setRootWidget(_widget);
	_widget->setScreenUpdater(this);
	_widget->setPlatform(this);
}

CR3Renderer::~CR3Renderer(void)
{

}

bool
CR3Renderer::InitializeGl(void)
{
	// TODO:
	// Initialize GL status. 

	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, GetTargetControlWidth(), GetTargetControlHeight());
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, GetTargetControlWidth(), GetTargetControlHeight(), 0, -1.0f, 1.0f);
	glClearColor(1, 1, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	return true;
}

bool
CR3Renderer::TerminateGl(void)
{
	// TODO:
	// Terminate and reset GL status. 
	return true;
}

#define USE_BACKBUFFER 1
bool
CR3Renderer::Draw(void)
{
	//CRLog::debug("CR3Renderer::Draw is called");

	 _eventAdapter->updateTizenSystemLang();

	//_updateRequested = false;

	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, GetTargetControlWidth(), GetTargetControlHeight());
	glEnable (GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, GetTargetControlWidth(), GetTargetControlHeight(), 0, -1.0f, 1.0f);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	GLDrawBuf buf(GetTargetControlWidth(), GetTargetControlHeight(), 32, false);
#if USE_BACKBUFFER == 1
	if (!_backbuffer) {
		_backbuffer = new GLDrawBuf(GetTargetControlWidth(), GetTargetControlHeight(), 32, true);
	} else if (_backbuffer->GetWidth() != GetTargetControlWidth() || _backbuffer->GetHeight() != GetTargetControlHeight()) {
		delete _backbuffer;
		_backbuffer = new GLDrawBuf(GetTargetControlWidth(), GetTargetControlHeight(), 32, true);
	}

	_backbuffer->beforeDrawing();
#endif

	lvRect pos = _widget->getPos();
	bool sizeChanged = false;
	if (pos.width() != __controlWidth || pos.height() != __controlHeight) {
		sizeChanged = true;
	}

	bool needLayout, needDraw, animating;
	CRUICheckUpdateOptions(_widget, needLayout, needDraw, animating);
	_widget->invalidate();
	if (needLayout || sizeChanged) {
		//CRLog::trace("need layout");
		_widget->measure(__controlWidth, __controlHeight);
		_widget->layout(0, 0, _widget->getMeasuredWidth(), _widget->getMeasuredHeight());
		needDraw = true;
	}
	if (needDraw) {
		//CRLog::trace("need draw");
#if USE_BACKBUFFER == 1
		_widget->draw(_backbuffer);
#else
		buf.beforeDrawing();
		_widget->draw(&buf);
#endif
	}
#if USE_BACKBUFFER == 1
	_backbuffer->afterDrawing();

	buf.beforeDrawing();
	_backbuffer->DrawTo(&buf, 0, 0, 0, NULL);
#endif
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glFlush();

	buf.afterDrawing();

	glFlush();

	//CRLog::debug("CR3Renderer::Draw exiting");
	return true;
}

bool
CR3Renderer::Pause(void)
{
	// TODO:
	// Do something necessary when Plyaer is paused. 

	return true;
}

bool
CR3Renderer::Resume(void)
{
	// TODO:
	// Do something necessary when Plyaer is resumed. 

	return true;
}

int
CR3Renderer::GetTargetControlWidth(void)
{
	return __controlWidth;
}

int
CR3Renderer::GetTargetControlHeight(void)
{
	return __controlHeight;
}

void
CR3Renderer::SetTargetControlWidth(int width)
{
	__controlWidth = width;
}

void
CR3Renderer::SetTargetControlHeight(int height)
{
	__controlHeight = height;
}

/// CRUIScreenUpdateManagerCallback implementation - exit application
void CR3Renderer::exitApp() {
	_app->Terminate();
}

/// minimize app or show Home Screen
void CR3Renderer::minimizeApp() {
	/// TODO: just hide
	_app->Terminate();

}

/// override to open URL in external browser; returns false if failed or feature not supported by platform
bool CR3Renderer::openLinkInExternalBrowser(lString8 url) {
	return false;
}

/// override to open file in external application; returns false if failed or feature not supported by platform
bool CR3Renderer::openFileInExternalApp(lString8 filename, lString8 mimeType) {
	return false;
}

/// returns 0 if not supported, task ID if download task is started
int CR3Renderer::openUrl(lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs) {
	return _downloadManager->openUrl(url, method, login, password, saveAs);
}

/// cancel specified download task
void CR3Renderer::cancelDownload(int downloadTaskId) {
	_downloadManager->cancelDownload(downloadTaskId);
}


// copy text to clipboard
void CR3Renderer::copyToClipboard(lString16 text) {
	ClipboardItem item;

	String data(text.c_str());
	item.Construct(CLIPBOARD_DATA_TYPE_TEXT, data);
	Clipboard* pClipboard = Clipboard::GetInstance();
	pClipboard->CopyItem(item);
}

/// return true if platform supports native virtual keyboard
bool CR3Renderer::supportsVirtualKeyboard() {
	return true;
}

/// return true if platform native virtual keyboard is shown
bool CR3Renderer::isVirtualKeyboardShown() {
	return _keypadShown;
}
/// show platform native virtual keyboard
void CR3Renderer::showVirtualKeyboard(int mode, lString16 text, bool multiline) {
	String txt(text.c_str());
	CRLog::trace("CR3Renderer::showVirtualKeyboard text = %s", LCSTR(txt));
	if (_keypadShown) {
		CRLog::trace("CR3Renderer::showVirtualKeyboard - already shown");
		if (_keypad)
			_keypad->SetText(txt);
		return;
	}
	_keypadShown = true;
    // Creates an instance of Keypad
	if (!_keypad) {
		_keypad = new Keypad();
		_keypad->Construct(KEYPAD_STYLE_NORMAL, KEYPAD_MODE_ALPHA);
		_keypad->SetTextPredictionEnabled(false);
		// Adds an instance of ITextEventListener
		_keypad->AddTextEventListener(*this);
	}
	_keypad->SetSingleLineEnabled(!multiline);
	_keypad->SetText(txt);

    // Changes to desired show state
    _keypad->SetShowState(true);
    _keypad->Show();

}

/// hide platform native virtual keyboard
void CR3Renderer::hideVirtualKeyboard() {
	if (!_keypadShown)
		return;
	_keypadShown = false;
    _keypad->SetShowState(false);
    //Invalidate(true);
}

// ITextEventListener
void CR3Renderer::OnTextValueChanged(const Tizen::Ui::Control& source) {
	String str = _keypad->GetText();
	if (_eventManager->getFocusedWidget()) {
		_eventManager->getFocusedWidget()->setText(lString16(str.GetPointer()));
		CRUIKeyEvent * event = new CRUIKeyEvent(KEY_ACTION_RELEASE, CR_KEY_RETURN, false, 1, 0);
		_eventManager->getFocusedWidget()->onKeyEvent(event);
		_eventManager->getRootWidget()->update(false);
	}
}

void CR3Renderer::OnTextValueChangeCanceled(const Tizen::Ui::Control& source) {
	// do nothing
}

class CRTizenRedrawEvent : public CRRunnable {
	Tizen::Graphics::Opengl::GlPlayer * __player;
public:
	CRTizenRedrawEvent(Tizen::Graphics::Opengl::GlPlayer * player) : __player(player) {}
	virtual void run() {
    	result r = __player->Redraw();
    	if (r != E_SUCCESS) {
    		CRLog::error("__player->Redraw() failed - delayed");
    	}
	}
};

/// set animation fps (0 to disable) and/or update screen instantly
void CR3Renderer::setScreenUpdateMode(bool updateNow, int animationFps) {
	//CRLog::trace("setScreenUpdateMode(%s, %d fps)", (updateNow ? "update now" : "no update"), animationFps);
	if ((animationFps != 0) != __playerStarted) {
		if (!__playerStarted) {
			__player->SetFps(30);
			result r = __player->Resume();
	    	if (r != E_SUCCESS) {
	    		CRLog::error("__player->Resume() failed");
	    	}
			__playerStarted = true;
		} else {
			__player->SetFps(-30);
			result r = __player->Pause();
	    	if (r != E_SUCCESS) {
	    		CRLog::error("__player->Pause() failed");
	    	}
			__playerStarted = false;
		}
	}
	if (!__playerStarted && updateNow) {
    	result r = __player->Redraw();
    	if (r != E_SUCCESS) {
    		CRLog::error("__player->Redraw() failed - immediate");
    	}
    	// double drawing, workaround for Tizen screen update issues
    	concurrencyProvider->executeGui(new CRTizenRedrawEvent(__player));
	}
}


#define DOWNLOAD_THREADS 1
CRUIHttpTaskManagerTizen::CRUIHttpTaskManagerTizen(CRUIEventManager * eventManager) : CRUIHttpTaskManagerBase(eventManager, DOWNLOAD_THREADS) {
}

void CRUIHttpTaskManagerTizen::onTaskFinished(CRUIHttpTaskBase * task) {

	CRUIHttpTaskManagerBase::onTaskFinished(task);
	CRLog::trace("Deleting task");
	delete task;
}

CRUIHttpTaskTizen::~CRUIHttpTaskTizen() {
	CRLog::trace("~CRUIHttpTaskTizen()");
	if (__pHttpSession) {
	    __pHttpSession->CloseAllTransactions();
		delete __pHttpSession;
	}

}

void CRUIHttpTaskTizen::OnTransactionAborted (HttpSession &httpSession, HttpTransaction &httpTransaction, result r) {
	CRLog::trace("CRUIHttpTaskTizen::OnTransactionAborted");
}

bool CRUIHttpTaskTizen::OnTransactionCertVerificationRequestedN (HttpSession &httpSession, HttpTransaction &httpTransaction, Tizen::Base::Collection::IList *pCertList) {
	CRLog::trace("CRUIHttpTaskTizen::OnTransactionCertVerificationRequestedN");
	return true;
}

void CRUIHttpTaskTizen::OnTransactionCertVerificationRequiredN (HttpSession &httpSession, HttpTransaction &httpTransaction, Tizen::Base::String *pCert) {
	CRLog::trace("CRUIHttpTaskTizen::OnTransactionCertVerificationRequiredN");
	httpTransaction.Resume();
}

static lString8 getHeaderStringField(HttpHeader* pHttpHeader, const char * field) {
	//CRLog::trace("Checking header field %s", field);
	String fieldName(field);
	Tizen::Base::Collection::IEnumerator * values = pHttpHeader->GetFieldValuesN(fieldName);
	if (GetLastResult() != E_SUCCESS) {
		//CRLog::trace("Header field %s not found - exception", field);
		SetLastResult(E_SUCCESS);
		return lString8();
	}
	if (values && !IsFailed(values->MoveNext())) {
		String * v = (String*)values->GetCurrent();
		lString8 res = UnicodeToUtf8(v->GetPointer());
		CRLog::trace("Header field %s value: %s", field, res.c_str());
		return res;
	} else {
		//CRLog::trace("Header field not found");
		return lString8();
	}
}

void CRUIHttpTaskTizen::OnTransactionCompleted (HttpSession &httpSession, HttpTransaction &httpTransaction) {
	CRLog::trace("CRUIHttpTaskTizen::OnTransactionCompleted");
	HttpResponse* pHttpResponse = null;
	HttpHeader* pHttpHeader = null;
	pHttpResponse = httpTransaction.GetResponse();
	pHttpHeader = pHttpResponse->GetHeader();
    String statusText = pHttpResponse->GetStatusText();
    int statusCode = pHttpResponse->GetHttpStatusCode();
    _result = statusCode == 200 ? 0 : statusCode;
    _resultMessage = UnicodeToUtf8(statusText.GetPointer());
    //IList * fieldNames = pHttpHeader->GetFieldNamesN();
    lString8 location = getHeaderStringField(pHttpHeader, "Location");
    lString8 contentType = getHeaderStringField(pHttpHeader, "Content-Type");
    lString8 contentLength = getHeaderStringField(pHttpHeader, "Content-Length");
    lString8 redir;
    if (_result == 300 || _result == 301 || _result == 302 || _result == 303 || _result == 307)
    	redir = location;
    if (!redir.empty()) {
    	CRLog::info("Redirection to %s", redir.c_str());
        if (redirectCount < 3 && canRedirect(redir)) {
        	redirectCount++;
            _url = redir;
            doDownload();
            return;
        } else {
            _result = 1;
            _resultMessage = "Too many redirections";
        }
    }
    if (!contentType.empty())
    	_mimeType = contentType;
    if (!contentLength.empty())
    	_size = contentLength.atoi();
    if (!_stream.isNull())
        _stream->SetPos(0);
    CRLog::debug("httpFinished(result=%d resultMessage=%s mimeType=%s url='%s')", _result, _result ? _resultMessage.c_str() : "", _mimeType.c_str(), _url.c_str());
    __pHttpSession->CloseTransaction(httpTransaction);
    __pHttpSession->CloseAllTransactions();
    delete __pHttpSession;
    __pHttpSession = NULL;
    _taskManager->onTaskFinished(this);
}

void CRUIHttpTaskTizen::OnTransactionHeaderCompleted (HttpSession &httpSession, HttpTransaction &httpTransaction, int headerLen, bool bAuthRequired) {
	CRLog::trace("CRUIHttpTaskTizen::OnTransactionHeaderCompleted");
	if (bAuthRequired) {
		CRLog::warn("Authentication is required");
		HttpTransaction* pTransaction =
				const_cast<HttpTransaction*>(&httpTransaction);
		HttpAuthentication* pAuth = pTransaction->OpenAuthenticationInfoN();
		String basicName(Utf8ToUnicode(_login).c_str());
		String basicpass(Utf8ToUnicode(_password).c_str());
		HttpCredentials* pCredential = new HttpCredentials(basicName,
				basicpass);
		String* pRealm = pAuth->GetRealmN();
		NetHttpAuthScheme scheme = pAuth->GetAuthScheme();
		if (scheme == NET_HTTP_AUTH_WWW_BASIC
				&& pRealm->CompareTo(L"MyWorld") == 0)
		HttpTransaction* pNewTransaction = pAuth->SetCredentials(
				*pCredential);
	}
}

void CRUIHttpTaskTizen::OnTransactionReadyToRead (HttpSession &httpSession, HttpTransaction &httpTransaction, int availableBodyLen) {
	ByteBuffer* pBody = null;
	HttpResponse* pHttpResponse = null;
	HttpHeader* pHttpHeader = null;

	pHttpResponse = httpTransaction.GetResponse();
    String statusText = pHttpResponse->GetStatusText();
    int statusCode = pHttpResponse->GetHttpStatusCode();
	CRLog::trace("CRUIHttpTaskTizen::OnTransactionReadyToRead result=%d availableBodyLen=%d", statusCode, availableBodyLen);
    if (statusCode == HTTP_STATUS_OK)
    {
    	pHttpHeader = pHttpResponse->GetHeader();
    	pBody = pHttpResponse->ReadBodyN();
    	int len = pBody->GetRemaining();
    	if (len > 0) {
			lUInt8 * data = new lUInt8[len];
			pBody->GetArray(data, 0, len);
			dataReceived(data, len);
    	}
    	delete pBody;
    }
}

void CRUIHttpTaskTizen::OnTransactionReadyToWrite (HttpSession &httpSession, HttpTransaction &httpTransaction, int recommendedChunkSize) {
	CRLog::trace("CRUIHttpTaskTizen::OnTransactionReadyToWrite");
}

void  CRUIHttpTaskTizen::OnHttpDownloadInProgress(HttpSession &httpSession, HttpTransaction &httpTransaction, long long currentLength, long long totalLength) {
	CRLog::trace("CRUIHttpTaskTizen::OnHttpDownloadInProgress(%d of %d)", (int)currentLength, (int)totalLength);
    _size = (int)totalLength;
    _sizeDownloaded = (int)currentLength;
	HttpResponse* pHttpResponse = null;
	pHttpResponse = httpTransaction.GetResponse();
    int statusCode = pHttpResponse->GetHttpStatusCode();
    if (_size > 0 && statusCode == 200)
        _taskManager->onTaskProgress(this);
}

void  CRUIHttpTaskTizen::OnHttpUploadInProgress (HttpSession &httpSession, HttpTransaction &httpTransaction, long long currentLength, long long totalLength) {
	// not used
}

/// override if you want do main work inside task instead of inside CRUIHttpTaskManagerBase::executeTask
void CRUIHttpTaskTizen::doDownload() {
	String* pProxyAddr = null;
	int p = _url.pos("/", 8);
	String uri = Utf8ToUnicode(_url).c_str();
	lString8 host = _url.substr(0, p);
	String hostAddr = Utf8ToUnicode(host).c_str();//L"http://localhost:port";
	CRLog::trace("Opening HTTP session for host %s", host.c_str());

	String fieldName, fieldValue;
	HttpHeader* pHeader = null;
	HttpTransaction* pHttpTransaction = null;
	if (!__pHttpSession) {
		__pHttpSession = new HttpSession();
		__pHttpSession->Construct(NET_HTTP_SESSION_MODE_NORMAL, pProxyAddr, hostAddr, null);
		CRLog::trace("enabling auto redirection");
		__pHttpSession->SetAutoRedirectionEnabled(true);
	} else {
		CRLog::trace("closing all transactions");
		__pHttpSession->CloseAllTransactions();
	}

	pHttpTransaction = __pHttpSession->OpenTransactionN();

	pHttpTransaction->AddHttpTransactionListener(*this);
	pHttpTransaction->SetHttpProgressListener(*this);

	HttpRequest* pHttpRequest = pHttpTransaction->GetRequest();

	pHttpRequest->SetMethod(NET_HTTP_METHOD_GET);
	pHttpRequest->SetUri(uri);

	pHeader = pHttpRequest->GetHeader();
	pHeader->AddField(L"User-Agent", L"CoolReader/3.3 (Tizen)");

	pHttpTransaction->Submit();
}
