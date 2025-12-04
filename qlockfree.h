#ifndef _QLOCKFREE_H
#define _QLOCKFREE_H
//USING OPENMP ATOMICS
//https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf
//using Michael-Scott non-blocking concurrent queue
//"The lock-free algorithm is non-blocking because if there are
//non-delayed processes attempting to perform operations on
//the queue, an operation is guaranteed to complete within
//finite time."


#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct node_lf node_lf_t;

//structure node_t, data:data type, next:pointer
struct node_lf {
    _Atomic(node_lf_t*) next;
    int data;
};

//structure queue_t, head:ptr, tail:ptr
typedef struct {
    _Atomic(node_lf_t*) head;
    _Atomic(node_lf_t*) tail;
} qlockfree_t;

void lock_free_init(qlockfree_t *q);
void lock_free_enq(qlockfree_t *q, int data);
int  lock_free_deq(qlockfree_t *q, int *data);

#endif 
