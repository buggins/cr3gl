#ifndef CRCONCURRENT_H
#define CRCONCURRENT_H

#include <lvref.h>

class CRMutex {
public:
    virtual ~CRMutex() {}
    virtual void acquire() = 0;
    virtual void release() = 0;
};
typedef LVAutoPtr<CRMutex> CRMutexRef;

class CRMonitor : public CRMutex {
public:
    virtual void wait() = 0;
    virtual void notify() = 0;
    virtual void notifyAll() = 0;
};
typedef LVAutoPtr<CRMonitor> CRMonitorRef;

class CRRunnable {
public:
    virtual void run() = 0;
};

class CRThread {
public:
    virtual ~CRThread() {}
    virtual void start() = 0;
    virtual void join() = 0;
};
typedef LVAutoPtr<CRThread> CRThreadRef;

class CRConcurrencyProvider {
public:
    virtual ~CRConcurrencyProvider() {}
    virtual CRMutex * createMutex() = 0;
    virtual CRMonitor * createMonitor() = 0;
    virtual CRThread * createThread(CRRunnable * threadTask) = 0;
};

extern CRConcurrencyProvider * concurrencyProvider;

template < typename T >
class LVQueue {
    struct Item {
        T value;
        Item * next;
        Item(T & v) : value(v), next(NULL) {}
    };
    Item * head;
    Item * tail;
    int count;
public:
    LVQueue() : head(NULL), tail(NULL), count(0) {}
    ~LVQueue() { clear(); }
//    T & operator [] (int index) {

//    }

    int length() { return count; }
    void push(T item) {
        Item * p = new Item(item);
        if (tail) {
            tail->next = p;
            tail = p;
        } else {
            head = tail = p;
        }
        count++;
    }
    T pop() {
        Item * p = head;
        if (!p)
            return T();
        if (head == tail)
            tail = NULL;
        head = head->next;
        T res = p->value;
        delete p;
        count--;
        return res;
    }
    void clear() {
        while (head) {
            Item * p = head;
            head = p->next;
            delete p;
        }
        head = NULL;
        tail = NULL;
        count = 0;
    }
};


class CRThreadExecutor : public CRRunnable {
    bool _stopped;
    CRMonitorRef _monitor;
    CRThreadRef _thread;
    LVQueue<CRRunnable *> _queue;
public:
    CRThreadExecutor();
    void execute(CRRunnable * task);
    void stop();
    virtual void run();
};

class CRGuard {
    CRMutex * mutex;
public:
    CRGuard(CRMutexRef & _mutex) : mutex(_mutex.get()) { mutex->acquire(); }
    CRGuard(CRMonitorRef & _mutex) : mutex(_mutex.get()) { mutex->acquire(); }
    ~CRGuard() { mutex->release(); }
};

#endif // CRCONCURRENT_H