#ifndef _QLOCK_H
#define _QLOCK_H

//no spinlock, use openMP for mutex
#include <omp.h>
//init, enq, deq

//node for queue need first for queue
//data  and pointer to next node
typedef struct qnode{
    int data;
    struct qnode *next;
}qnode_t;

//head and tail pointer, protected by openMP for this implementation
typedef struct {
    qnode_t *head;
    qnode_t *tail;
}qlock_t;

void qlock_init(qlock_t *q);
void qlock_enq(qlock_t *q, int data);
int qlock_deq(qlock_t *q, int *data);


#endif /* _QLOCK_H */