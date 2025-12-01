#include "qlock.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>


void qlock_init(qlock_t *q){
    //sentinel node
    qnode_t *sentinel = (qnode_t *)malloc(sizeof(qnode_t));
    sentinel->data =0;
    //node->next.ptr = NULL
    sentinel->next=NULL;
    //q->head=q->tail=sentinel
    q->head = sentinel;
    q->tail=sentinel;
    //init unlock
    atomic_store_explicit(&q->lock,0,memory_order_relaxed);

}

void qlock_enq(qlock_t *q,int data){
    //new node and store data
    qnode_t *new_node = (qnode_t *)malloc(sizeof(qnode_t));
    new_node->data=data;
    new_node->next=NULL;
    //lock
    spin_lock(&q->lock);
    //new node after tail
    q->tail->next = new_node;
    //update tail pointer
    q->tail=new_node;
    //release lock
    spin_unlock(&q->lock);
}

int qlock_deq(qlock_t *q, int *data){
    spin_lock(&q->lock);
    //get past sentienel
    qnode_t *first = q->head->next;
    //queue not empty case
    if (first != NULL){
        //set data, move head forward
        *data = first->data;
        q->head->next = first->next;

        //if removed last node
        if(q->tail == first){
            //point to sentinel
            q->tail=q->head;
        }
        spin_unlock(&q->lock);
        free(first);
        return 0;
    }
    else{
        //queue empty, what return here?
        spin_unlock(&q->lock);
        return -1;
    }

}
