#ifndef __CSJLOCKUTILS_H__
#define __CSJLOCKUTILS_H__

#include <QReadWriteLock>

typedef enum {
    CSJRWLock_RD = 0,
    CSJRWLock_WR
} CSJRWLockType;

class CSJRWLock {
public:
    CSJRWLock(QReadWriteLock &lock, CSJRWLockType lockType = CSJRWLock_RD)
        : m_rwLock(lock) {

        switch (lockType) {
        case CSJRWLock_RD:
            lock.lockForRead();
            break;
        case CSJRWLock_WR:
            lock.lockForWrite();
            break;
        default:
            lock.lockForWrite();
        }
    }

    ~CSJRWLock() {
        m_rwLock.unlock();
    }

private:
    QReadWriteLock &m_rwLock;
};

#endif // __CSJLOCKUTILS_H__
