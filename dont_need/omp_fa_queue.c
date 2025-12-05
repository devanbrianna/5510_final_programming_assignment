#include "omp_fa_queue.h"
#include <stdio.h>

void fa_queue_init(fa_queue_t *q, int capacity)
{
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;

    q->cells = calloc(capacity, sizeof(fa_cell_t));
    if (!q->cells) {
        perror("calloc");
        exit(1);
    }
}

void fa_queue_destroy(fa_queue_t *q)
{
    free(q->cells);
}

int fa_enqueue(fa_queue_t *q, int value)
{
    int idx;

    #pragma omp atomic capture
    { idx = q->tail; q->tail++; }

    if (idx >= q->capacity)
        return -1;     

    q->cells[idx].value = value;

    #pragma omp flush

    #pragma omp atomic write
    q->cells[idx].ready = 1;

    return 0;
}

int fa_dequeue(fa_queue_t *q, int *out)
{
    int idx;

    #pragma omp atomic capture
    { idx = q->head; q->head++; }

    if (idx >= q->capacity)
        return -1;

    int ready = 0;
    while (!ready) {
        #pragma omp atomic read
        ready = q->cells[idx].ready;

        if (!ready)
            #pragma omp flush
            ;
    }

    *out = q->cells[idx].value;
    return 0;
}
