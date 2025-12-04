#include "omp_lockfree_queue.h"

/*
 * Enqueue:
 *  - Atomically grab an index from 'tail' using atomic capture.
 *  - If index >= capacity, the queue is "logically full" for this run.
 *  - Write value into cells[idx], then set ready = 1 (atomic write).
 */
int omp_lf_enqueue(omp_lf_queue_t *q, int value)
{
    int idx;

    /* Atomic fetch-and-increment: idx = tail; tail++ */
    #pragma omp atomic capture
    { idx = q->tail; q->tail++; }

    if (idx >= q->capacity) {
        // overflow: more enqueues than capacity
        return -1;
    }

    // Store value
    q->cells[idx].value = value;

    // Ensure value is visible before marking ready
    #pragma omp flush

    // Mark this cell as ready
    #pragma omp atomic write
    q->cells[idx].ready = 1;

    return 0;
}

/*
 * Dequeue:
 *  - Atomically grab an index from 'head' using atomic capture.
 *  - If idx >= capacity, there was never an item for this index → error.
 *  - Otherwise spin until cells[idx].ready == 1, then read value.
 *
 * This is lock-free: each dequeue reserves a unique index;
 * threads may spin waiting for the producer of that index,
 * but no thread holds a lock.
 */
int omp_lf_dequeue(omp_lf_queue_t *q, int *out)
{
    int idx;

    /* Atomic fetch-and-increment: idx = head; head++ */
    #pragma omp atomic capture
    { idx = q->head; q->head++; }

    if (idx >= q->capacity) {
        // more dequeues than capacity/enqueues
        return -1;
    }

    // Wait until producer marks this slot ready
    int ready_flag;
    while (1) {
        #pragma omp atomic read
        ready_flag = q->cells[idx].ready;

        if (ready_flag)
            break;

        // Optional: small pause or flush
        #pragma omp flush
    }

    // Now the value is ready to read
    *out = q->cells[idx].value;
    return 0;
}
