/*  QtSpeech -- a small cross-platform library to use TTS
    Copyright (C) 2010-2011 LynxLine.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General
    Public License along with this library; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301 USA */

#include <QtCore>
#include <QtSpeech>

#undef UNICODE
#include <sapi.h>
#include <sphelper.h>
#include <comdef.h>
#define UNICODE

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "lvstring.h"

namespace QtSpeech_v1 { // API v1.0

// some defines for throwing exceptions
#define Where QString("%1:%2:").arg(__FILE__).arg(__LINE__)
#define SysCall(x,e) {\
    HRESULT hr = x;\
    if (FAILED(hr)) {\
        QString msg = #e;\
        msg += ":"+QString(__FILE__);\
        msg += ":"+QString::number(__LINE__)+":"+#x+":";\
        msg += _com_error(hr).ErrorMessage();\
        throw e(msg);\
    }\
}

QString langNameFromVoice(CComPtr<ISpObjectToken> voice) {
    LANGID langId = 0;
    if (SpGetLanguageFromVoiceToken(voice, &langId) == S_OK) {
        //CRLog::debug("language: ", langId);
        switch(langId) {
        case 0x0419: return "ru_RU"; // Russian/Russia
        case 0x0C09: return "en_AU"; // English/Australia
        case 0x0409: return "en_US"; // English/US
        case 0x0809: return "en_GB"; // English/GB
        case 0x1009: return "en_CA"; // English/Canada
        default:
            switch(langId & 0xFF) {
            case 0x09: return "en";
            case 0x19: return "ru";
            default:
                return "";
            }
        }
    }
    return "";
}

// internal data
class QtSpeech::Private {
public:
    Private()
        :onFinishSlot(0L),waitingFinish(false) {}

    VoiceName name;

    static const QString VoiceId;
    typedef QPointer<QtSpeech> Ptr;
    static QList<Ptr> ptrs;

    CComPtr<ISpVoice> voice;

    const char * onFinishSlot;
    QPointer<QObject> onFinishObj;
    bool waitingFinish;

    class WCHAR_Holder {
    public:
        WCHAR * w;
        WCHAR_Holder(QString s)
            :w(0) {
            w = new WCHAR[s.length()+1];
            s.toWCharArray(w);
            w[s.length()] =0;
        }

        ~WCHAR_Holder() { delete[] w; }
    };
};
const QString QtSpeech::Private::VoiceId = QString("win:%1");
QList<QtSpeech::Private::Ptr> QtSpeech::Private::ptrs = QList<QtSpeech::Private::Ptr>();

// implementation
QtSpeech::QtSpeech(QObject * parent)
    :QObject(parent), d(new Private)
{
    CoInitialize(NULL);
    SysCall( d->voice.CoCreateInstance( CLSID_SpVoice ), InitError);

    VoiceName n;
    WCHAR * w_id = 0L;
    WCHAR * w_name = 0L;
    CComPtr<ISpObjectToken> voice;
    SysCall( d->voice->GetVoice(&voice), InitError);
    SysCall( SpGetDescription(voice, &w_name), InitError);
    SysCall( voice->GetId(&w_id), InitError);
    n.name = QString::fromWCharArray(w_name);
    n.id = QString::fromWCharArray(w_id);
    n.lang = langNameFromVoice(voice);
    voice.Release();

    if (n.id.isEmpty())
        throw InitError(Where+"No default voice in system");

    d->name = n;
    d->ptrs << this;
}

QtSpeech::QtSpeech(VoiceName n, QObject * parent)
    :QObject(parent), d(new Private)
{
    ULONG count = 0;
    CComPtr<IEnumSpObjectTokens> voices;

    CoInitialize(NULL);
    SysCall( d->voice.CoCreateInstance( CLSID_SpVoice ), InitError);

    if (n.id.isEmpty()) {
        WCHAR * w_id = 0L;
        WCHAR * w_name = 0L;
        CComPtr<ISpObjectToken> voice;
        SysCall( d->voice->GetVoice(&voice), InitError);
        SysCall( SpGetDescription(voice, &w_name), InitError);
        SysCall( voice->GetId(&w_id), InitError);
        n.name = QString::fromWCharArray(w_name);
        n.id = QString::fromWCharArray(w_id);
        n.lang = langNameFromVoice(voice);
        voice.Release();
    }
    else {
        SysCall( SpEnumTokens(SPCAT_VOICES, NULL, NULL, &voices), InitError);
        SysCall( voices->GetCount(&count), InitError);
        for (int i =0; i < (int)count; i++) {
            WCHAR * w_id = 0L;
            CComPtr<ISpObjectToken> voice;
            SysCall( voices->Next( 1, &voice, NULL ), InitError);
            SysCall( voice->GetId(&w_id), InitError);
            QString id = QString::fromWCharArray(w_id);
            if (id == n.id) d->voice->SetVoice(voice);
            voice.Release();
        }
    }

    if (n.id.isEmpty())
        throw InitError(Where+"No default voice in system");

    d->name = n;
    d->ptrs << this;
}

QtSpeech::~QtSpeech()
{
    d->ptrs.removeAll(this);
    delete d;
}

const QtSpeech::VoiceName & QtSpeech::name() const {
    return d->name;
}

QtSpeech::VoiceNames QtSpeech::voices()
{
    VoiceNames vs;       
    ULONG count = 0;
    CComPtr<IEnumSpObjectTokens> voices;

    CoInitialize(NULL);
    SysCall( SpEnumTokens(SPCAT_VOICES, NULL, NULL, &voices), LogicError);
    SysCall( voices->GetCount(&count), LogicError);

    for(int i=0; i < (int)count; i++) {
        WCHAR * w_id = 0L;
        WCHAR * w_name = 0L;
        CComPtr<ISpObjectToken> voice;
        SysCall( voices->Next( 1, &voice, NULL ), LogicError);
        SysCall( SpGetDescription(voice, &w_name), LogicError);
        SysCall( voice->GetId(&w_id), LogicError);

        QString id = QString::fromWCharArray(w_id);
        QString name = QString::fromWCharArray(w_name);
        QString lang = langNameFromVoice(voice);
        VoiceName n = { id, name, lang };
        vs << n;

        voice.Release();
    }
    return vs;
}

void QtSpeech::tell(QString text) const {
    tell(text, 0L,0L);
}

void QtSpeech::tell(QString text, QObject * obj, const char * slot) const
{
    if (d->waitingFinish) {
        throw LogicError(Where+"Already waiting to finish speech");
    }

    d->onFinishObj = obj;
    d->onFinishSlot = slot;
    if (obj && slot)
        connect(const_cast<QtSpeech *>(this), SIGNAL(finished()), obj, slot);

    const_cast<QtSpeech *>(this)->startTimer(100);

    d->voice->SetPriority(SPVPRI_NORMAL);
    d->voice->SetAlertBoundary(SPEI_PHONEME); //SPEI_WORD_BOUNDARY

    Private::WCHAR_Holder w_text(text);
    SysCall( d->voice->Speak( w_text.w, SPF_ASYNC | SPF_IS_NOT_XML, 0), LogicError);
    //CRLog::trace("setting waitingFinish flag");
    d->waitingFinish = true;
}

void QtSpeech::say(QString text) const
{
    Private::WCHAR_Holder w_text(text);
    SysCall( d->voice->Speak( w_text.w, SPF_IS_NOT_XML, 0), LogicError);
}

void QtSpeech::stop() {
    d->voice->Pause();
    Private::WCHAR_Holder w_text(QString("STOP"));
    d->voice->SetPriority(SPVPRI_ALERT);
    d->voice->SetAlertBoundary(SPEI_PHONEME); //SPEI_WORD_BOUNDARY
    d->voice->Speak(w_text.w, SPF_ASYNC | SPF_PURGEBEFORESPEAK | SPF_IS_NOT_XML, 0);
    //ULONG pulNumSkipped = 0;
    //d->voice->Skip(L"Sentence", 1, &pulNumSkipped);
}

void QtSpeech::timerEvent(QTimerEvent * te)
{
    QObject::timerEvent(te);

    //CRLog::trace("timerEvent");
    if (d->waitingFinish) {
        //CRLog::trace("timerEvent - waiting finish");
        SPVOICESTATUS es;
        d->voice->GetStatus( &es, NULL );
        if (es.dwRunningState == SPRS_DONE) {
            //CRLog::trace("timerEvent - waiting finish, status = DONE");
            d->waitingFinish = false;
            killTimer(te->timerId());
            //CRLog::trace("timerEvent - signal");
            finished();
            if (d->onFinishObj && d->onFinishSlot)
                disconnect(const_cast<QtSpeech *>(this), SIGNAL(finished()), d->onFinishObj, d->onFinishSlot);
        }
    }
}

} // namespace QtSpeech_v1
