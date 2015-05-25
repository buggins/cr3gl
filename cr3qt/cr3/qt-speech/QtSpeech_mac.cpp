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
#include <ApplicationServices/ApplicationServices.h>

namespace QtSpeech_v1 { // API v1.0

// some defines for throwing exceptions
#define Where QString("%1:%2:").arg(__FILE__).arg(__LINE__)
#define SysCall(x,e) {\
    OSErr ok = x;\
    if (ok != noErr) {\
        QString msg = #e;\
        msg += ":"+QString(__FILE__);\
        msg += ":"+QString::number(__LINE__)+":"+#x;\
        throw e(msg);\
    }\
}

#ifdef Q_OS_MAC64
#define SpeechDoneUPP_ARG2 void *
#else
#define SpeechDoneUPP_ARG2 long
#endif

// internal data
class QtSpeech::Private {
public:
    Private()
        :isWaitingInLoop(false),
          onFinishSlot(0L) {}

    VoiceName name;
    SpeechChannel channel;

    static const QString VoiceId;
    typedef QPointer<QtSpeech> Ptr;
    static QList<Ptr> ptrs;

    bool isWaitingInLoop;
    QPointer<QEventLoop> waitEventLoop;

    SpeechDoneUPP doneCall;
    const char * onFinishSlot;
    QPointer<QObject> onFinishObj;
    static void speechFinished(SpeechChannel, SpeechDoneUPP_ARG2 refCon);
};
const QString QtSpeech::Private::VoiceId = QString("macosx:%1");
QList<QtSpeech::Private::Ptr> QtSpeech::Private::ptrs = QList<QtSpeech::Private::Ptr>();

// implementation
QtSpeech::QtSpeech(QObject * parent)
    :QObject(parent), d(new Private)
{
    VoiceName n;
    VoiceDescription info;
    SysCall( GetVoiceDescription(NULL, &info, sizeof(VoiceDescription)), InitError);
    //n.name = QString::fromAscii((const char *)(info.name+1), int(info.name[0]));
    n.name = QString::fromUtf8((const char *)(info.name+1), int(info.name[0]));
    n.id = d->VoiceId.arg(info.voice.id);

    if (n.id.isEmpty())
        throw InitError(Where+"No default voice in system");

    SInt16 count;
    VoiceSpec voice;
    VoiceSpec * voice_ptr(0L);
    SysCall( CountVoices(&count), InitError);
    for (int i=1; i<= count; i++) {
        SysCall( GetIndVoice(i, &voice), InitError);
        QString id = d->VoiceId.arg(voice.id);
        if (id == n.id) {
            voice_ptr = &voice;
            break;
        }
    }

    d->doneCall = NewSpeechDoneUPP(&Private::speechFinished);
    SysCall( NewSpeechChannel(voice_ptr, &d->channel), InitError);
    SysCall( SetSpeechInfo(d->channel, soSpeechDoneCallBack,
                           (void *)d->doneCall), InitError);
    d->name = n;
    d->ptrs << this;
}

QtSpeech::QtSpeech(VoiceName n, QObject * parent)
    :QObject(parent), d(new Private)
{
    if (n.id.isEmpty()) {
        VoiceDescription info;
        SysCall( GetVoiceDescription(NULL, &info, sizeof(VoiceDescription)), InitError);
        n.name = QString::fromUtf8((const char *)(info.name+1), int(info.name[0]));
        n.id = d->VoiceId.arg(info.voice.id);
    }

    if (n.id.isEmpty())
        throw InitError(Where+"No default voice in system");

    SInt16 count;
    VoiceSpec voice;
    VoiceSpec * voice_ptr(0L);
    SysCall( CountVoices(&count), InitError);
    for (int i=1; i<= count; i++) {
        SysCall( GetIndVoice(i, &voice), InitError);
        QString id = d->VoiceId.arg(voice.id);
        if (id == n.id) {
            voice_ptr = &voice;
            break;
        }
    }

    d->doneCall = NewSpeechDoneUPP(&Private::speechFinished);
    SysCall( NewSpeechChannel(voice_ptr, &d->channel), InitError);
    SysCall( SetSpeechInfo(d->channel, soSpeechDoneCallBack,
                           (void *)d->doneCall), InitError);
    d->name = n;
    d->ptrs << this;
}

QtSpeech::~QtSpeech()
{
    if (!d->channel)
        throw CloseError(Where+"No speech channel to close");

    SysCall( StopSpeech(d->channel), CloseError);
    SysCall( DisposeSpeechChannel(d->channel), CloseError);
    DisposeSpeechDoneUPP(d->doneCall);

    d->ptrs.removeAll(this);
    delete d;
}

const QtSpeech::VoiceName & QtSpeech::name() const {
    return d->name;
}

/*
2015/05/25 20:28:40.0778 DEBUG TTS Voice: id:'macosx:300'' name:'Agnes' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:41'' name:'Albert' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:201'' name:'Alex' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:352327299'' name:'Alice Compact' lang: en3
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:587208531'' name:'Alva Compact' lang: en5
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:234887235'' name:'Amelie Compact' lang: en1
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:268442323'' name:'Anna Compact' lang: en2
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:36'' name:'Bad News' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:40'' name:'Bahh' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:26'' name:'Bells' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:16'' name:'Boing' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:100'' name:'Bruce' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:50'' name:'Bubbles' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:671121683'' name:'Carmit Compact' lang: en10
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:35'' name:'Cellos' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:335593667'' name:'Damayanti Compact' lang: en81
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:134267091'' name:'Daniel Compact' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:38'' name:'Deranged' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:637587523'' name:'Diego Compact' lang: en6
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:67180211'' name:'Ellen Compact' lang: en34
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:167858403'' name:'Fiona Compact' lang: en
2015/05/25 20:28:40.0779 DEBUG TTS Voice: id:'macosx:1'' name:'Fred' lang: en
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:39'' name:'Good News' lang: en
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:30'' name:'Hysterical' lang: en
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:503454723'' name:'Ioana Compact' lang: en37
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:486693891'' name:'Joana Compact' lang: en8
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:4'' name:'Junior' lang: en
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:604143827'' name:'Kanya Compact' lang: en22
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:100827411'' name:'Karen Compact' lang: en
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:2'' name:'Kathy' lang: en
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:369275107'' name:'Kyoko Compact' lang: en11
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:537051459'' name:'Laura Compact' lang: en39
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:302172323'' name:'Lekha Compact' lang: en21
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:469952547'' name:'Luciana Compact' lang: en8
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:318963987'' name:'Mariska Compact' lang: en26
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:419629187'' name:'Mei-Jia Compact' lang: en19
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:285411507'' name:'Melina Compact' lang: en14
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:520294579'' name:'Milena Compact' lang: en32
2015/05/25 20:28:40.0780 INFO Current TTS Voice: 'Milena Compact' lang: en32
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:117644419'' name:'Moira Compact' lang: en
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:553852115'' name:'Monica Compact' lang: en6
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:436428051'' name:'Nora Compact' lang: en9
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:570671427'' name:'Paulina Compact' lang: en6
2015/05/25 20:28:40.0780 DEBUG TTS Voice: id:'macosx:31'' name:'Pipe Organ' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:3'' name:'Princess' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:5'' name:'Ralph' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:184844483'' name:'Samantha Compact' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:50626835'' name:'Sara Compact' lang: en7
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:218399027'' name:'Satu Compact' lang: en13
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:17076435'' name:'Sin-ji Compact' lang: en19
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:311571'' name:'Tarik Compact' lang: en12
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:201640227'' name:'Tessa Compact' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:251973347'' name:'Thomas Compact' lang: en1
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:402968787'' name:'Ting-Ting Compact' lang: en33
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:9'' name:'Trinoids' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:151341123'' name:'Veena Compact' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:200'' name:'Vicki' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:200'' name:'Victoria' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:6'' name:'Whisper' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:84263123'' name:'Xander Compact' lang: en4
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:621152435'' name:'Yelda Compact' lang: en17
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:386279635'' name:'Yuna Compact' lang: en23
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:8'' name:'Zarvox' lang: en
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:453401891'' name:'Zosia Compact' lang: en25
2015/05/25 20:28:40.0781 DEBUG TTS Voice: id:'macosx:33974675'' name:'Zuzana Compact' lang: en38

 */
QString langIdToName(int id) {
    switch(id) {
    case 0:
        return QString("en");
    case 32:
        return QString("ru"); // russian
    case 12:
        return QString("ar"); // arabian
    case 26:
        return QString("hu"); // hungarian
    case 34:
        return QString("nl_BE"); // dutch belgium
    case 4:
        return QString("nl_NL"); // dutch holland
    case 14:
        return QString("el"); // greek
    case 7:
        return QString("da"); // danish
    case 10:
        return QString("he"); // hebrew
    case 81:
        return QString("id"); // indonesian
    case 6:
        return QString("es"); // spanish
    case 3:
        return QString("it"); // italian
    case 33:
        return QString("zh"); // chinese
    case 19:
        return QString("zh_TW"); // chinese hong kong, taiwan
    case 23:
        return QString("ko"); // korean
    case 2:
        return QString("de"); // german
    case 9:
        return QString("no"); // norway
    case 25:
        return QString("pl"); // polish
    case 8:
        return QString("pt"); // portugese brasil
    case 37:
        return QString("ro"); // romanian
    case 39:
        return QString("sk"); // slovak
    case 22:
        return QString("th"); // thai
    case 17:
        return QString("tr"); // turkish
    case 13:
        return QString("fi"); // finnish
    case 1:
        return QString("fr"); // french
    case 21:
        return QString("hi"); // hindi india
    case 38:
        return QString("cs"); // czech
    case 5:
        return QString("cv"); // sweden
    case 11:
        return QString("ja"); // japanese
    default:
        return QString("en_") + QString::number(id);
    }
}

void QtSpeech::setRate(int rate) {
    SetSpeechRate(d->channel, rate + 130);
}

void QtSpeech::stop() {
    StopSpeech(d->channel);
}


QtSpeech::VoiceNames QtSpeech::voices()
{
    SInt16 count;
    VoiceNames vs;
    VoiceDescription desc;
    SysCall( CountVoices(&count), LogicError);
    SysCall( GetVoiceDescription(NULL, &desc, sizeof(VoiceDescription)), LogicError);
    for (int i=1; i<= count; i++) {
        VoiceSpec voice;
        VoiceDescription info;
        SysCall( GetIndVoice(i, &voice), LogicError);
        SysCall( GetVoiceDescription(&voice, &info, sizeof(VoiceDescription)), LogicError);
        QString name = QString::fromUtf8((const char *)(info.name+1), int(info.name[0]));
        int language = info.language;
        QString langName = langIdToName(language);
        VoiceName vname = { Private::VoiceId.arg(voice.id), name + " (" + langName.mid(0, 2) + ")", langName };
        vs << vname;
    }
    return vs;
}

void QtSpeech::tell(QString text) const {
    tell(text, 0L,0L);
}

void QtSpeech::tell(QString text, QObject * obj, const char * slot) const
{
    d->onFinishObj = obj;
    d->onFinishSlot = slot;
    if (obj && slot)
        connect(const_cast<QtSpeech *>(this), SIGNAL(finished()), obj, slot);

    CFStringRef cf_text = CFStringCreateWithCharacters(0,
                            reinterpret_cast<const UniChar *>(
                              text.unicode()), text.length());

    OSErr ok = SpeakCFString(d->channel, cf_text, NULL);
    CFRelease(cf_text);

    if (ok != noErr) throw LogicError(Where+"SpeakCFString()");
}

void QtSpeech::say(QString text) const
{
    if (d->isWaitingInLoop)
        throw LogicError(Where+"Already in process of saying something");

    d->isWaitingInLoop = true;
    tell(text);

    QEventLoop el;
    d->waitEventLoop = &el;
    el.exec();
}

void QtSpeech::Private::speechFinished(SpeechChannel chan, SpeechDoneUPP_ARG2 refCon)
{
    Q_UNUSED(refCon);
    foreach(QtSpeech * c, ptrs) {
        if (c && c->d->channel == chan) {
            c->finished();
            if (c->d->onFinishObj && c->d->onFinishSlot) {
                disconnect(c, SIGNAL(finished()),
                           c->d->onFinishObj, c->d->onFinishSlot);
                c->d->onFinishSlot = 0L;
                c->d->onFinishObj = 0L;
            }
            if (c->d->isWaitingInLoop) {
                c->d->isWaitingInLoop = false;
                if (c->d->waitEventLoop)
                    c->d->waitEventLoop->quit();
            }
            break;
        }
    }
}

void QtSpeech::timerEvent(QTimerEvent * te)
{
    QObject::timerEvent(te);
}

} // namespace QtSpeech_v1
