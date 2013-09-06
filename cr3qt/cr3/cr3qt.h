#ifndef CR3QT_H
#define CR3QT_H

#include <crui.h>
#include <crconcurrent.h>
#include <cruiconfig.h>
#include <QApplication>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QMouseEvent>
#include <glwrapper.h>

class CRUIEventAdapter;

class CRQtThread : public QThread {
    Q_OBJECT
    CRRunnable * runnable;
protected:
    virtual void run() { runnable->run(); }
public:
    CRQtThread(CRRunnable * _runnable) : runnable(_runnable) {}
    virtual ~CRQtThread() { }
};

class QtGuiExecutorObject : public QObject {
    Q_OBJECT

    class QtGuiExecutorEvent : public QEvent {
        CRRunnable * task;
    public:
        void run() { task->run(); }
        QtGuiExecutorEvent(CRRunnable * _task) : QEvent(QEvent::User), task(_task) { }
        virtual ~QtGuiExecutorEvent() { delete task; }
    };

public:
    virtual bool event(QEvent * e) {
        QtGuiExecutorEvent * ee = dynamic_cast<QtGuiExecutorEvent*>(e);
        if (ee) {
            ee->run();
            ee->accept();
        }
        return true;
    }
    void execute(CRRunnable * _task) {
        QtGuiExecutorEvent * event = new QtGuiExecutorEvent(_task);
        QCoreApplication::postEvent(this, event);
    }
};

class QtConcurrencyProvider : public CRConcurrencyProvider {
    QtGuiExecutorObject * guiExecutor;
public:

    class QtMutex : public CRMutex {
        QMutex mutex;
    public:
        QtMutex() : mutex(QMutex::Recursive) {}
        virtual void acquire() { mutex.lock(); }
        virtual void release() { mutex.unlock(); }
    };

    class QtMonitor : public CRMonitor {
        QMutex mutex;
        QWaitCondition cond;
    public:
        virtual void acquire() { mutex.lock(); }
        virtual void release() { mutex.unlock(); }
        virtual void wait() { cond.wait(&mutex); }
        virtual void notify() { cond.wakeOne(); }
        virtual void notifyAll() { cond.wakeAll(); }
    };

    class QtThread : public CRThread {
        CRQtThread thread;
    public:
        QtThread(CRRunnable * _runnable) : thread(_runnable) {}
        virtual ~QtThread() {
            thread.wait();
        }
        virtual void start() {
            thread.start();
        }
        virtual void join() {
            thread.wait();
        }
    };

public:
    virtual CRMutex * createMutex() {
        return new QtMutex();
    }

    virtual CRMonitor * createMonitor() {
        return new QtMonitor();
    }

    virtual CRThread * createThread(CRRunnable * threadTask) {
        return new QtThread(threadTask);
    }
    virtual void executeGui(CRRunnable * task) {
        guiExecutor->execute(task);
    }

    QtConcurrencyProvider() {
        guiExecutor = new QtGuiExecutorObject();
    }
    /// sleep current thread
    virtual void sleepMs(int durationMs) {
        QThread::msleep(durationMs);
    }

    virtual ~QtConcurrencyProvider() {}
};

class CRUIEventAdapter {
    CRUIEventManager * _eventManager;
    CRUIMotionEventItem * _activePointer;
public:
    CRUIEventAdapter(CRUIEventManager * eventManager);
    // touch event listener
    void dispatchTouchEvent(QMouseEvent * event);
    // key event listener
    void dispatchKeyEvent(QKeyEvent * event);
};


void InitCREngine(lString16 exePath);

#endif // CR3QT_H
