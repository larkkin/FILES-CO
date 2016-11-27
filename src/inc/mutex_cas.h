#ifndef __MUTEX_CAS_H__
#define __MUTEX_CAS_H__

#include <stdatomic.h>

#define LOCKED   1
#define UNLOCKED 0

typedef struct spinlock {
  atomic_int locked;
} spinlock_t;

void lock(spinlock_t* lock);
void unlock(spinlock_t *lock);


#endif /*__MUTEX_CAS_H__*/
