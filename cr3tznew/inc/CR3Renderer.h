#ifndef _GLRENDERERTEMPLATE_H_
#define _GLRENDERERTEMPLATE_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FUi.h>
#include <FUiIme.h>
#include <FGraphics.h>
#include <gl.h>
#include <FGrpIGlRenderer.h>
#include <FGrpGlPlayer.h>
#include "crui.h"
#include "cr3tizen.h"
#include "cruimain.h"
#include "gldrawbuf.h"
#include "FNetHttpIHttpTransactionEventListener.h"
#include "FNetHttpIHttpProgressEventListener.h"
#include "FNetHttpHttpSession.h"
#include "FNetHttpHttpTransaction.h"

class CRUIHttpTaskTizen : public CRUIHttpTaskBase
		, public Tizen::Net::Http::IHttpTransactionEventListener
		, public Tizen::Net::Http::IHttpProgressEventListener
{

private:
    Tizen::Net::Http::HttpSession* __pHttpSession;
public:
    int redirectCount;

public:
    CRUIHttpTaskTizen(CRUIHttpTaskManagerBase * taskManager) : CRUIHttpTaskBase(taskManager), __pHttpSession(NULL), redirectCount(0) {}
    virtual ~CRUIHttpTaskTizen();
    /// override if you want do main work inside task instead of inside CRUIHttpTaskManagerBase::executeTask
    virtual void doDownload();


    // transaction listener
    virtual void OnTransactionAborted (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction, result r);
    virtual bool OnTransactionCertVerificationRequestedN (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction, Tizen::Base::Collection::IList *pCertList);
    virtual void OnTransactionCertVerificationRequiredN (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction, Tizen::Base::String *pCert);
    virtual void OnTransactionCompleted (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction);
    virtual void OnTransactionHeaderCompleted (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction, int headerLen, bool bAuthRequired);
    virtual void OnTransactionReadyToRead (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction, int availableBodyLen);
    virtual void OnTransactionReadyToWrite (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction, int recommendedChunkSize);


    // progress listener
    virtual void  OnHttpDownloadInProgress (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction, long long currentLength, long long totalLength);
    virtual void  OnHttpUploadInProgress (Tizen::Net::Http::HttpSession &httpSession, Tizen::Net::Http::HttpTransaction &httpTransaction, long long currentLength, long long totalLength);

};

class CRUIHttpTaskManagerTizen : public CRUIHttpTaskManagerBase {
private:
public:
    CRUIHttpTaskManagerTizen(CRUIEventManager * eventManager);
    /// override to create task of custom type
    virtual CRUIHttpTaskBase * createTask() { return new CRUIHttpTaskTizen(this); }
    virtual void onTaskFinished(CRUIHttpTaskBase * task);
};



class LVDocView;
class CoolReaderApp;
class CoolReaderFrame;
class CR3Renderer
	: public Tizen::Graphics::Opengl::IGlRenderer
	, public Tizen::Ui::ITextEventListener
	, public CRUIScreenUpdateManagerCallback
	, public CRUIPlatform
{
	CoolReaderApp * _app;
	CoolReaderFrame * _frame;
	LVDocView * _docview;
	CRUIMainWidget * _widget;
	CRUIEventManager * _eventManager;
	CRUIEventAdapter * _eventAdapter;
	CRUIHttpTaskManagerTizen * _downloadManager;
	GLDrawBuf * _backbuffer;
	Tizen::Ui::Controls::Keypad * _keypad;
	bool _keypadShown;
	//bool _updateRequested;
public:

	CR3Renderer(CoolReaderApp * app, CoolReaderFrame * frame);
	~CR3Renderer(void);

	CRUIEventAdapter * getEventAdapter() { return _eventAdapter; }

	virtual bool InitializeGl(void);
	virtual bool TerminateGl(void);

	virtual bool Draw(void);

	virtual bool Pause(void);
	virtual bool Resume(void);

	virtual int GetTargetControlWidth(void);
	virtual int GetTargetControlHeight(void);
	virtual void SetTargetControlWidth(int width);
	virtual void SetTargetControlHeight(int height);

    void setPlayer(Tizen::Graphics::Opengl::GlPlayer * player) {
    	__player = player;
    }

    /// set animation fps (0 to disable) and/or update screen instantly
    virtual void setScreenUpdateMode(bool updateNow, int animationFps);

    // CRUIPlatform implementation
    /// CRUIScreenUpdateManagerCallback implementation - exit application
	virtual void exitApp();
	/// minimize app or show Home Screen
	virtual void minimizeApp();

    /// override to open URL in external browser; returns false if failed or feature not supported by platform
    virtual bool openLinkInExternalBrowser(lString8 url);
    /// override to open file in external application; returns false if failed or feature not supported by platform
    virtual bool openFileInExternalApp(lString8 filename, lString8 mimeType);

    // copy text to clipboard
    virtual void copyToClipboard(lString16 text);

    /// return true if platform supports native virtual keyboard
    virtual bool supportsVirtualKeyboard();
    /// return true if platform native virtual keyboard is shown
    virtual bool isVirtualKeyboardShown();
    /// show platform native virtual keyboard
    virtual void showVirtualKeyboard(int mode, lString16 text, bool multiline);
    /// hide platform native virtual keyboard
    virtual void hideVirtualKeyboard();


    /// returns 0 if not supported, task ID if download task is started
    virtual int openUrl(lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs);
    /// cancel specified download task
    virtual void cancelDownload(int downloadTaskId);

    // ITextEventListener
    virtual void OnTextValueChanged(const Tizen::Ui::Control& source);
    virtual void OnTextValueChangeCanceled(const Tizen::Ui::Control& source);

private:
	int __controlWidth;
	int __controlHeight;
	int __angle;
	Tizen::Graphics::Opengl::GlPlayer * __player;
	bool __playerStarted;
};

#endif /* _GLRENDERERTEMPLATE_H_ */
