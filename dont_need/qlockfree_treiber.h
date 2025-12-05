#ifndef _QLOCKFREE_TREIBER_H
#define _QLOCKFREE_TREIBER_H

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct treiber_node treiber_node_t;

struct treiber_node {
    int value;
    treiber_node_t *next;
};

typedef struct {
    treiber_node_t *top;
} treiber_stack_t;

// Initialize stack (empty)
static inline void treiber_init(treiber_stack_t *s) {
    s->top = NULL;
}

// Push value onto stack
void treiber_push(treiber_stack_t *s, int value);

// Pop value from stack; returns 0 on success, -1 if empty
int treiber_pop(treiber_stack_t *s, int *out);

#endif
