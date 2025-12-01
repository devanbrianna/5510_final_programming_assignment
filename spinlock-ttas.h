#ifndef _SPINLOCK_TTAS_H
#define _SPINLOCK_TTAS_H

#include <stdatomic.h>

//A test-and-test-and-set (TTAS) lock spin lock that uses backoff.
//from textbook:
//improves on TAS lock by reducing the amount of cache coherence traffic
//TTAS lock first "tests" by reading flag in a loop until free
//when free it preforms atomic exchange (xchg) to aquire lock

#define SPINLOCK_ATTR static __inline __attribute__((always_inline, no_instrument_function))

//false=lock free, true= lock held
typedef _Atomic (unsigned char) spinlock;

//same as other implememtations
#define cpu_relax() __asm__ __volatile__("pause" ::: "memory")


SPINLOCK_ATTR void spin_lock(spinlock *lock)
{
    int backoff =1;
    int max_backoff=1024;
    while (1) {
        //test until lock is free
        while (atomic_load_explicit(lock, memory_order_relaxed)){
            //trying backoff here
            for (unsigned int i=0;i<backoff;i++){
                cpu_relax();
            }
            if (backoff<max_backoff){
                backoff*=2;
            }
        }
        //TAS, atomicallt aquire lock
        unsigned char expected =0;
        if (atomic_compare_exchange_weak_explicit(lock, &expected, 1,memory_order_acquire,memory_order_relaxed))
            return;  // Got the lock

        /* Backoff before retrying
        for (int i = 0; i < backoff; i++)
            cpu_relax();

        if (backoff < max_backoff)
            backoff *=2;  // Exponential backoff
            */
        backoff=1;
    }
}

SPINLOCK_ATTR void spin_unlock(spinlock *lock){
    atomic_store_explicit(lock, 0,memory_order_release);
}

#define SPINLOCK_INITIALIZER { 0 }

#endif /* _SPINLOCK_TTAS_H */


//memorder aquire
//mem order relaxed