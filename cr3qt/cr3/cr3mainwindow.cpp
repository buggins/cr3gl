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
#include <gldrawbuf.h>

lString16 resourceDir;
void setupResourcesForScreenSize() {
    lString8 resDir8 = UnicodeToUtf8(resourceDir);
    lString8Collection dirs;
    if (deviceInfo.shortSide <= 320) {
        dirs.add(resDir8 + "screen-density-normal");
        dirs.add(resDir8 + "screen-density-high");
        dirs.add(resDir8 + "screen-density-xhigh");
    } else if (deviceInfo.shortSide <= 480) {
        dirs.add(resDir8 + "screen-density-high");
        dirs.add(resDir8 + "screen-density-xhigh");
        dirs.add(resDir8 + "screen-density-normal");
    } else {
        dirs.add(resDir8 + "screen-density-xhigh");
        dirs.add(resDir8 + "screen-density-high");
        dirs.add(resDir8 + "screen-density-normal");
    }
    resourceResolver->setDirList(dirs);
}

QOpenGLFunctions * _qtgl = NULL;
//! [1]
OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
    , m_update_pending(false)
    , m_animating(false)
    , m_context(0)
    , m_device(0)
{
    setSurfaceType(QWindow::OpenGLSurface);
    _qtgl = this;
    //_widget = new CRUIButton(lString16("Test"));
    _widget = new CRUIHomeWidget();
}
//! [1]

OpenGLWindow::~OpenGLWindow()
{
    delete m_device;
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
    int sz = deviceInfo.shortSide;
    int sz1 = sz / 32;
    int sz2 = sz / 24;
    int sz3 = sz / 18;
    int sz4 = sz / 14;
    int sz5 = sz / 10;
    currentTheme->setFontForSize(CRUI::FONT_SIZE_XSMALL, fontMan->GetFont(sz1, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_SMALL, fontMan->GetFont(sz2, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_MEDIUM, fontMan->GetFont(sz3, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_LARGE, fontMan->GetFont(sz4, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_XLARGE, fontMan->GetFont(sz5, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    setupResourcesForScreenSize();
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
        int dpi = 72;
        int minsz = sz.width() < sz.height() ? sz.width() : sz.height();
        if (minsz >= 320)
            dpi = 120;
        if (minsz >= 480)
            dpi = 160;
        if (minsz >= 640)
            dpi = 200;
        if (minsz >= 800)
            dpi = 250;
        if (minsz >= 900)
            dpi = 300;
        deviceInfo.setScreenDimensions(sz.width(), sz.height(), dpi);
        adaptThemeForScreenSize();
        //CRLog::trace("Layout is needed");
        if (_widget)
            delete _widget;
        _widget = new CRUIHomeWidget();
        _widget->requestLayout();
    }
    GLDrawBuf buf(sz.width(), sz.height(), 32, false);
    buf.beforeDrawing();
    bool needLayout, needDraw;
    //CRLog::trace("Checking if draw is required");
    CRUICheckUpdateOptions(_widget, needLayout, needDraw);
    _widget->invalidate();
    if (needLayout) {
        //CRLog::trace("need layout");
        _widget->measure(sz.width(), sz.height());
        _widget->layout(0, 0, sz.width(), sz.height());
    }
    if (needDraw) {
        //CRLog::trace("need draw");
        _widget->draw(&buf);
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
