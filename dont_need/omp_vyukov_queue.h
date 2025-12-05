#ifndef _OMP_VYUKOV_QUEUE_H
#define _OMP_VYUKOV_QUEUE_H

#include <stdlib.h>
#include <omp.h>

typedef struct {
    int value;
    long seq;
} vyukov_cell_t;

typedef struct {
    vyukov_cell_t *buffer;
    long capacity;

    long head;
    long tail;
} vyukov_queue_t;

int vyukov_init(vyukov_queue_t *q, long capacity);
void vyukov_destroy(vyukov_queue_t *q);

int vyukov_enqueue(vyukov_queue_t *q, int value);
int vyukov_dequeue(vyukov_queue_t *q, int *out);

#endif
