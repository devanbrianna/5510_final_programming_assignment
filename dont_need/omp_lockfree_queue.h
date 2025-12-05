#ifndef _OMP_LOCKFREE_QUEUE_H
#define _OMP_LOCKFREE_QUEUE_H

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Bounded lock-free queue using OpenMP atomics.
 * 
 * Assumptions:
 *  - capacity is fixed at initialization.
 *  - total number of enqueues will never exceed capacity.
 *  - head and tail only grow (no wrap-around).
 * 
 * Each cell has:
 *    value : stored int
 *    ready : 0 until producer finished writing value, then 1
 */

typedef struct {
    int value;
    int ready;
} omp_lf_cell_t;

typedef struct {
    omp_lf_cell_t *cells;
    int capacity;
    int head;   // next index to dequeue
    int tail;   // next index to enqueue
} omp_lf_queue_t;

/* Initialize queue with given capacity.
 * Allocates 'capacity' cells, sets all ready flags to 0.
 */
static inline void omp_lf_queue_init(omp_lf_queue_t *q, int capacity) {
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;

    q->cells = (omp_lf_cell_t *)malloc(sizeof(omp_lf_cell_t) * capacity);
    if (!q->cells) {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < capacity; i++) {
        q->cells[i].ready = 0;
    }
}

/* Destroy queue storage. */
static inline void omp_lf_queue_destroy(omp_lf_queue_t *q) {
    if (q->cells) {
        free(q->cells);
        q->cells = NULL;
    }
}

/* Lock-free enqueue using OpenMP.
 * Returns 0 on success, -1 if queue capacity exceeded.
 */
int omp_lf_enqueue(omp_lf_queue_t *q, int value);

/* Lock-free dequeue using OpenMP.
 * Returns 0 on success, -1 if head index exceeds capacity
 * (i.e., no more items were ever enqueued for this index).
 *
 * For normal test usage (where you enqueue exactly N items and
 * dequeue exactly N items), you should not hit -1.
 */
int omp_lf_dequeue(omp_lf_queue_t *q, int *out);


//goes with ./test_omp_queue
//test_omp_lockfree_queue.c
#endif
