#include "cr3mainwindow.h"

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

#include <QtCore/QCoreApplication>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QClipboard>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QSslError>

#include "gldrawbuf.h"

//! [1]
OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
    , m_update_pending(false)
    , m_animating(false)
    , m_context(0)
    , m_device(0)
{
    setSurfaceType(QWindow::OpenGLSurface);
    int dx = 800; // 480;
    int dy = 500; // 800;
    resize(QSize(dx, dy));

    _qtgl = this;

    deviceInfo.setScreenDimensions(dx, dy, 200);
    crconfig.setupResourcesForScreenSize();
    _widget = new CRUIMainWidget();
    _widget->setScreenUpdater(this);
    _widget->setPlatform(this);
    _eventManager = new CRUIEventManager();
    _eventAdapter = new CRUIEventAdapter(_eventManager);
    _eventManager->setRootWidget(_widget);
    _downloadManager = new CRUIHttpTaskManagerQt(_eventManager);
}
//! [1]

OpenGLWindow::~OpenGLWindow()
{


    delete _eventAdapter;
    delete _eventManager;
    delete _downloadManager;
    delete _widget;
//    delete m_device;
    _qtgl = NULL;
}

//! [2]
void OpenGLWindow::render(QPainter *painter)
{
    //Q_UNUSED(painter);
    //painter->setBrush(QBrush());
    painter->drawEllipse(1,1,100,100);
}

void OpenGLWindow::initialize()
{
}

void adaptThemeForScreenSize() {
    crconfig.setupResourcesForScreenSize();
}

void OpenGLWindow::mousePressEvent(QMouseEvent * event) {
    _eventAdapter->dispatchTouchEvent(event);
    event->accept();
    //renderIfChanged();
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent * event) {
    _eventAdapter->dispatchTouchEvent(event);
    event->accept();
    //renderIfChanged();
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent * event) {
    _eventAdapter->dispatchTouchEvent(event);
    event->accept();
    //renderIfChanged();
}

void OpenGLWindow::keyPressEvent(QKeyEvent * event) {
    _eventAdapter->dispatchKeyEvent(event);
    event->accept();
    //renderIfChanged();
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent * event) {
    _eventAdapter->dispatchKeyEvent(event);
    event->accept();
    //renderIfChanged();
}

void OpenGLWindow::setScreenUpdateMode(bool updateNow, int animationFps) {
    setAnimating(animationFps > 0);
    if (!animationFps && updateNow)
        renderIfChanged();
}

void OpenGLWindow::renderIfChanged()
{
    bool needLayout, needDraw, animating;
    CRUICheckUpdateOptions(_widget, needLayout, needDraw, animating);
    if (animating) {
        setAnimating(true);
        //CRLog::trace("needLayout=%s needDraw=%s animating=true", needLayout ? "true" : "false", needDraw ? "true" : "false");
    } else {
        setAnimating(false);
        if (needLayout || needDraw) {
            //CRLog::trace("needLayout=%s needDraw=%s animating=false", needLayout ? "true" : "false", needDraw ? "true" : "false");
            renderLater();
        }
    }
}

void OpenGLWindow::render()
{
    if (!m_device)
        m_device = new QOpenGLPaintDevice;
    //CRLog::trace("Render is called");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(1,1,1,1);

    m_device->setSize(size());

    QSize sz = size();
    if (deviceInfo.isSizeChanged(sz.width(), sz.height())) {
//        int dpi = 72;
//        int minsz = sz.width() < sz.height() ? sz.width() : sz.height();
//        if (minsz >= 320)
//            dpi = 120;
//        if (minsz >= 480)
//            dpi = 160;
//        if (minsz >= 640)
//            dpi = 200;
//        if (minsz >= 800)
//            dpi = 250;
//        if (minsz >= 900)
//            dpi = 300;
//        dpi = 100;
        deviceInfo.setScreenDimensions(sz.width(), sz.height(), 200); // dpi
        adaptThemeForScreenSize();
        //CRLog::trace("Layout is needed");
        _widget->onThemeChanged();
    }
    GLDrawBuf buf(sz.width(), sz.height(), 32, false);
    //CRLog::trace("Calling buf.beforeDrawing");
    buf.beforeDrawing();
    bool needLayout, needDraw, animating;
    //CRLog::trace("Checking if draw is required");
    CRUICheckUpdateOptions(_widget, needLayout, needDraw, animating);
    _widget->invalidate();
    if (needLayout) {
        //CRLog::trace("need layout");
        _widget->measure(sz.width(), sz.height());
        _widget->layout(0, 0, sz.width(), sz.height());
        //CRLog::trace("done layout");
    }
    needDraw = true;
    if (needDraw) {
        //CRLog::trace("need draw");
        _widget->draw(&buf);
        //CRLog::trace("done draw");
    }
    //CRLog::trace("Calling buf.afterDrawing");
    buf.afterDrawing();
    //CRLog::trace("Finished buf.afterDrawing");

//    QPainter painter(m_device);
//    render(&painter);
}
//! [2]

//! [3]
void OpenGLWindow::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool OpenGLWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        m_update_pending = false;
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}
//! [3]

//! [4]
void OpenGLWindow::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}
//! [4]

//! [5]
void OpenGLWindow::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
        renderLater();
}
//! [5]

/// minimize app or show Home Screen
void OpenGLWindow::minimizeApp() {

}

void OpenGLWindow::exitApp() {
    QApplication::exit();
}

// copy text to clipboard
void OpenGLWindow::copyToClipboard(lString16 text) {
    QClipboard *clipboard = QApplication::clipboard();
    QString txt(UnicodeToUtf8(text).c_str());
    clipboard->setText(txt);
}



/// returns 0 if not supported, task ID if download task is started
int OpenGLWindow::openUrl(lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs) {
    CRLog::info("openUrl(%s %s) -> %s", url.c_str(), method.c_str(), saveAs.c_str());
    return _downloadManager->openUrl(url, method, login, password, saveAs);
}

/// cancel specified download task
void OpenGLWindow::cancelDownload(int downloadTaskId) {
    _downloadManager->cancelDownload(downloadTaskId);
}

/// override if you want do main work inside task instead of inside CRUIHttpTaskManagerBase::executeTask
void CRUIHttpTaskQt::doDownload() {
    url.setUrl(QString::fromUtf8(_url.c_str()));
    reply = qnam->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()),
            this, SLOT(httpFinished()));
    connect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateDataReadProgress(qint64,qint64)));
}


void CRUIHttpTaskQt::httpFinished() {
     QNetworkReply::NetworkError error = reply->error();
    _result = error;
    _resultMessage = reply->errorString().toUtf8().constData();
    QVariant contentMimeType = reply->header(QNetworkRequest::ContentTypeHeader);
    QString contentTypeString;
    if (contentMimeType.isValid())
        contentTypeString = contentMimeType.toString();
    _mimeType = contentTypeString.toUtf8().constData();
    _stream->SetPos(0);
    CRLog::trace("httpFinished(result=%d resultMessage=%s mimeType=%s)", _result, _resultMessage.c_str(), _mimeType.c_str());
    _taskManager->onTaskFinished(this);
}

void CRUIHttpTaskQt::httpReadyRead() {
    if (!_size) {
        _size = reply->size();
    }
    CRLog::trace("httpReadyRead(total size = %d)", _size);
    QByteArray data = reply->readAll();
    dataReceived((const lUInt8 *)data.constData(), data.length());
}

//    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
//    void enableDownloadButton();
void CRUIHttpTaskQt::slotAuthenticationRequired(QNetworkReply*,QAuthenticator * authenticator) {
    CRLog::trace("slotAuthenticationRequired()");
    authenticator->setUser(QString::fromUtf8(_login.c_str()));
    authenticator->setPassword(QString::fromUtf8(_password.c_str()));
}

#ifndef QT_NO_OPENSSL
void CRUIHttpTaskQt::sslErrors(QNetworkReply*,const QList<QSslError> &errors) {
    CRLog::trace("sslErrors()");
    QString errorString;
    foreach (const QSslError &error, errors) {
        if (!errorString.isEmpty())
            errorString += ", ";
        errorString += error.errorString();
    }
    CRLog::error("SSL Errors, ignoring: %s", errorString.toUtf8().constData());
    reply->ignoreSslErrors();
}

#endif
