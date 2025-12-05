#ifndef OMP_FA_QUEUE_H
#define OMP_FA_QUEUE_H

#include <omp.h>
#include <stdlib.h>

typedef struct {
    int value;
    int ready;
} fa_cell_t;

typedef struct {
    int capacity;
    int head;
    int tail;
    fa_cell_t *cells;
} fa_queue_t;

void fa_queue_init(fa_queue_t *q, int capacity);
void fa_queue_destroy(fa_queue_t *q);

int fa_enqueue(fa_queue_t *q, int value);
int fa_dequeue(fa_queue_t *q, int *out);

#endif
