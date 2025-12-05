#include "omp_vyukov_queue.h"

/*
 * Initialize queue with capacity slots.
 */
int vyukov_init(vyukov_queue_t *q, long capacity)
{
    q->capacity = capacity;
    q->buffer = malloc(sizeof(vyukov_cell_t) * capacity);
    if (!q->buffer) return -1;

    for (long i = 0; i < capacity; i++)
        q->buffer[i].seq = i;  // initial sequence matches index

    q->head = 0;
    q->tail = 0;
    return 0;
}

void vyukov_destroy(vyukov_queue_t *q)
{
    free(q->buffer);
}

/*
 * Vyukov Enqueue (MPMC)
 */
int vyukov_enqueue(vyukov_queue_t *q, int value)
{
    long pos;
    vyukov_cell_t *cell;

    while (1) {
        // Atomically reserve a tail index
        #pragma omp atomic read
        pos = q->tail;

        cell = &q->buffer[pos % q->capacity];

        long seq;
        #pragma omp atomic read
        seq = cell->seq;

        long diff = seq - pos;

        if (diff == 0) {
            long next = pos + 1;

            // Attempt to increment tail
            long observed;
            #pragma omp atomic capture
            { observed = q->tail; q->tail = (observed == pos ? next : observed); }

            if (observed == pos) {
                // We now own this slot
                cell->value = value;
                #pragma omp flush

                #pragma omp atomic write
                cell->seq = pos + 1;
                return 0;
            }
        }
        else if (diff < 0) {
            return -1; // queue full
        }
    }
}

/*
 * Vyukov Dequeue (MPMC)
 */
int vyukov_dequeue(vyukov_queue_t *q, int *out)
{
    long pos;
    vyukov_cell_t *cell;

    while (1) {
        #pragma omp atomic read
        pos = q->head;

        cell = &q->buffer[pos % q->capacity];

        long seq;
        #pragma omp atomic read
        seq = cell->seq;

        long diff = seq - (pos + 1);

        if (diff == 0) {
            long next = pos + 1;

            long observed;
            #pragma omp atomic capture
            { observed = q->head; q->head = (observed == pos ? next : observed); }

            if (observed == pos) {
                *out = cell->value;

                #pragma omp atomic write
                cell->seq = pos + q->capacity;
                return 0;
            }
        }
        else if (diff < 0) {
            return -1; // empty
        }
    }
}
