#ifndef CRCONCURRENT_H
#define CRCONCURRENT_H

class CRMutex {
public:
    virtual ~CRMutex() {}
    virtual void acquire() = 0;
    virtual void release() = 0;
};

class CRMonitor : public CRMutex {
public:
    virtual void wait() = 0;
    virtual void notify() = 0;
    virtual void notifyAll() = 0;
};

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

class CRConcurrencyProvider {
public:
    virtual ~CRConcurrencyProvider() {}
    virtual CRMutex * createMutex() = 0;
    virtual CRMonitor * createMonitor() = 0;
    virtual CRThread * createThread(CRRunnable * threadTask) = 0;
};

extern CRConcurrencyProvider * concurrencyProvider;

#endif // CRCONCURRENT_H
