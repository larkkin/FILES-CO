#include <mutex_cas.h>

void lock(spinlock_t* lock)
{
  do {
    while (atomic_load_explicit(&lock->locked,
             memory_order_relaxed) == LOCKED);
  } while (atomic_exchange_explicit(&lock->locked,
             LOCKED, memory_order_acquire) == LOCKED);
}

void unlock(spinlock_t *lock)
{
  atomic_store_explicit(&lock->locked, UNLOCKED,
    memory_order_release);
}
