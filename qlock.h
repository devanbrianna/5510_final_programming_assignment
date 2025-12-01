#ifndef _QLOCK_H
#define _QLOCK_H

#include "spinlock-ttas.h"
//init, enq, deq

//node for queue need first for queue
//data  and pointer to next node
typedef struct qnode{
    int data;
    struct qnode *next;
}qnode_t;

//head and tail pointer, access protected by TTAS spinlock
typedef struct {
    qnode_t *head;
    qnode_t *tail;
    spinlock lock;
}qlock_t;

void qlock_init(qlock_t *q);
void qlock_enq(qlock_t *q, int data);
int qlock_deq(qlock_t *q, int *data);


#endif /* _QLOCK_H */