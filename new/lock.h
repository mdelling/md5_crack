#ifndef LOCK_H
#define LOCK_H

#ifdef __APPLE__
/* In OS X, use OSAtomicSpinlock */
#include <libkern/OSAtomic.h>
typedef OSSpinLock lock_t;
#define LOCK_INITIALIZER OS_SPINLOCK_INIT
#define lock_lock(l) OSSpinLockLock(l)
#define lock_unlock(l) OSSpinLockUnlock(l)
#define lock_destroy(l)
#else
/* Otherwise, use pthread */
#include <pthread.h>
typedef pthread_mutex_t lock_t;
#define LOCK_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define lock_lock(l) pthread_mutex_lock(l)
#define lock_unlock(l) pthread_mutex_unlock(l)
#define lock_destroy(l) pthread_mutex_destroy(l)
#endif

#endif
