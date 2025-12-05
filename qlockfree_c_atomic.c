#include "qlockfree_c_atomic.h"

void lock_free_init(qlockfree_t *q)
{
    //new=new_node()
    node_lf_t *node = malloc(sizeof(node_lf_t));
    if (!node) {
        perror("malloc");
        exit(1);
    }
    //node->next.ptr = null, make it the only node in ll
    node->data = 0;
    atomic_store(&node->next, NULL);
    //q->head=q->tail=node, both head and tail point to it
    atomic_store(&q->head, node);
    atomic_store(&q->tail, node);
}

void lock_free_enq(qlockfree_t *q, int data)
{
    //node = new_node(), allocate a new node
    node_lf_t *node = malloc(sizeof(node_lf_t));
    if (!node) {
        perror("malloc");
        exit(1);
    }
    //node->value = value, copy enqueued value into node
    node->data = data;
    //node->next.ptr = NULL, set next ptr of node to null
    atomic_store(&node->next, NULL);
    //loop until enqueue is done
    while (1) {
        //tail=q->tail
        node_lf_t *tail = atomic_load(&q->tail);
        //next= tail.ptr->next
        node_lf_t *next = atomic_load(&tail->next);
        //if tail == q->tail, are tail and next consistent? 
        if (tail == atomic_load(&q->tail)) {
            //if next.ptr->NULL, was tail pointing to the last node? 
            if (next == NULL) {
                //if CAS(&tail.ptr->next,next,<node,next.count+1>)
                //try to link node at the end of the ll
                node_lf_t *expected = NULL;
                if (atomic_compare_exchange_strong(&tail->next,&expected,node)) {
                    //enqueue is done, try to zwing tail to the inserted node
                    //CAS(&q->tail,tail,<node,tail.count+1>)
                    atomic_compare_exchange_strong(&q->tail, &tail, node);
                    //enqueue is done
                    return;
                }
                //tail was not pointing to the last node
            } else {
                // cas(&q->tail,tail,<next.ptr, tail.count_1>)
                //try to swing tail to the next node
                atomic_compare_exchange_strong(&q->tail, &tail, next);
            }
        }
        // else: tail changed under us; retry
    }
}

int lock_free_deq(qlockfree_t *q, int *data)
{
    //loop
    while (1) {
        //head = q->head, read head
        node_lf_t *head = atomic_load(&q->head);
        //tail=q->tail, read tail
        node_lf_t *tail = atomic_load(&q->tail);
        //next=head->next, read head next
        node_lf_t *next = atomic_load(&head->next);
        //if head == q->head, are head tail and next consistent?
        if (head == atomic_load(&q->head)) {
            //if head.ptr == tail.ptr, is queue empty or tail falling behind?
            if (head == tail) {
                //if next.ptr==null, is queue empty?
                if (next == NULL) {
                    // Queue empty, cannot dequeue
                    return -1;
                }
                //cas(&q->tail, tail <next.ptr,tail.count+1>)
                //tail falling behind, try to advance it
                atomic_compare_exchange_strong(&q->tail, &tail, next);
            }
            //no need to deal with tail
            else {
                //if cas(&q->head, head, <next.ptr,head.count+1>)
                int value = next->data;
                if (atomic_compare_exchange_strong(&q->head, &head, next)) {
                    //free head.ptr, not doing this bc getting error in testing
                    //store value to data
                    *data = value;
                    //done dequeue
                    return 0;
                }
            }
        }
    }
}