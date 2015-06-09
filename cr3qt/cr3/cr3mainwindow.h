#ifndef CR3MAINWINDOW_H
#define CR3MAINWINDOW_H

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>

#include <QCoreApplication>
#include <QThread>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QtSingleApplication>
#include <QtSpeech>

#include "cr3qt.h"
#include "cruimain.h"

QT_BEGIN_NAMESPACE
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
QT_END_NAMESPACE


class CRUIHttpTaskQt : public QObject, public CRUIHttpTaskBase  {
    Q_OBJECT
public:
    QUrl url;
    QNetworkAccessManager * qnam;
    QNetworkReply *reply;
    QFile *file;
    int redirectCount;
private slots:
//    void downloadFile();
//    void cancelDownload();
    void httpFinished();
    void httpReadyRead();
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
    void httpError(QNetworkReply::NetworkError code);
//    void enableDownloadButton();
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply*,const QList<QSslError> &errors);
#endif
public:
    CRUIHttpTaskQt(CRUIHttpTaskManagerBase * taskManager, QNetworkAccessManager * _qnam) : CRUIHttpTaskBase(taskManager), qnam(_qnam), redirectCount(0) {}
    virtual ~CRUIHttpTaskQt();
    /// override if you want do main work inside task instead of inside CRUIHttpTaskManagerBase::executeTask
    virtual void doDownload();
};

#define DOWNLOAD_THREADS 5
class CRUIHttpTaskManagerQt : public QObject, public CRUIHttpTaskManagerBase {
    Q_OBJECT
private:
    QNetworkAccessManager qnam;
private slots:
    void slotAuthenticationRequired(QNetworkReply*,QAuthenticator *);
public:
    CRUIHttpTaskManagerQt(CRUIEventManager * eventManager);
    /// override to create task of custom type
    virtual CRUIHttpTaskBase * createTask() { return new CRUIHttpTaskQt(this, &qnam); }
};

//! [1]
class OpenGLWindow : public QWindow, protected QOpenGLFunctions,
        public CRUIScreenUpdateManagerCallback, public CRUIPlatform,
        public MessageReceivedHandler
{
    Q_OBJECT

protected:
    CRUIMainWidget * _widget;
    CRUIEventManager * _eventManager;
    CRUIEventAdapter * _eventAdapter;
    CRUIHttpTaskManagerQt * _downloadManager;
    CRUITextToSpeech * _textToSpeech;
    bool _fullscreen;
public:

    virtual CRUITextToSpeech * getTextToSpeech();

    CRPropRef getSettings() { return _widget->getSettings(); }

    explicit OpenGLWindow(QWindow *parent = 0);
    ~OpenGLWindow();

    virtual void render(QPainter *painter);
    virtual void render();

    virtual void renderIfChanged();

    virtual void initialize();

    void setAnimating(bool animating);

    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void wheelEvent(QWheelEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void keyReleaseEvent(QKeyEvent * event);

    // CRUI overrides
    virtual void setScreenUpdateMode(bool updateNow, int animationFps);
    virtual void exitApp();
    // copy text to clipboard
    virtual void copyToClipboard(lString16 text);
    /// minimize app or show Home Screen
    virtual void minimizeApp();
    /// override to open URL in external browser; returns false if failed or feature not supported by platform
    virtual bool openLinkInExternalBrowser(lString8 url);
    /// override to open file in external application; returns false if failed or feature not supported by platform
    virtual bool openFileInExternalApp(lString8 filename, lString8 mimeType);


    /// returns 0 if not supported, task ID if download task is started
    virtual int openUrl(lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs);
    /// cancel specified download task
    virtual void cancelDownload(int downloadTaskId);

    /// return true if device has hardware keyboard connected
    virtual bool hasHardwareKeyboard() { return true; }

    // fullscreen methods
    virtual bool supportsFullscreen() { return true; }
    virtual bool isFullscreen();
    virtual void setFullscreen(bool fullscreen);

    virtual void setFileToOpenOnStart(lString8 filename);


    void restorePositionAndShow();
    void saveWindowStateAndPosition();

public slots:
    void renderLater();
    void renderNow();
    void onMessageReceived(const QString &message);

protected:
    virtual bool event(QEvent *event);

    virtual void exposeEvent(QExposeEvent *event);
    virtual void resizeEvent(QResizeEvent *);
    virtual void moveEvent(QMoveEvent *);
    virtual void showEvent(QShowEvent *);


private:
    bool m_initialized;
    bool m_update_pending;
    bool m_animating;
    bool m_coverpageManagerPaused;

    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;
};
//! [1]

class CRUIQtTextToSpeech : public QObject, public CRUITextToSpeech {
    Q_OBJECT
private:
    CRUITextToSpeechCallback * _ttsCallback;
    CRUITextToSpeechVoice * _currentVoice;
    CRUITextToSpeechVoice * _defaultVoice;
    QtSpeech * _speechManager;
    LVPtrVector<CRUITextToSpeechVoice, true> _voices;
    bool _isSpeaking;
    int _rate;
public slots:
    void sentenceFinished();
public:
    CRUIQtTextToSpeech();
    virtual CRUITextToSpeechCallback * getTextToSpeechCallback();
    virtual void setTextToSpeechCallback(CRUITextToSpeechCallback * callback);
    virtual void getAvailableVoices(LVPtrVector<CRUITextToSpeechVoice, false> & list);
    virtual CRUITextToSpeechVoice * getCurrentVoice();
    virtual CRUITextToSpeechVoice * getDefaultVoice();
    virtual bool setRate(int rate);
    virtual int getRate();
    virtual bool setCurrentVoice(lString8 id);
    virtual bool canChangeCurrentVoice();
    virtual bool tell(lString16 text);
    virtual bool isSpeaking();
    virtual void stop();
    virtual ~CRUIQtTextToSpeech();
};



#endif // CR3MAINWINDOW_H
