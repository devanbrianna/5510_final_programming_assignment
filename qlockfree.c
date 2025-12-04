#include "qlockfree.h"

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
    node->next = NULL;
    //q->head=q->tail=node, both head and tail point to it
    #pragma omp atomic write
    q->head = node;
    #pragma omp atomic write
    q->tail = node;

    #pragma omp flush
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
    node->next = NULL;
    //loop until enqueue is done
    while (1) {
        //tail=q->tail
        node_lf_t *tail;
        #pragma omp atomic write
        tail = q->tail;
        //next= tail.ptr->next
        node_lf_t *next;
        #pragma omp atomic write
        next=tail->next;
        //if tail == q->tail, are tail and next consistent? 
        if (tail == q->tail) {
            //if next.ptr->NULL, was tail pointing to the last node? 
            if (next == NULL) {
                //if CAS(&tail.ptr->next,next,<node,next.count+1>)
                //try to link node at the end of the ll
                int good = 0;
                #pragma omp atomic compare
                if (tail->next == NULL){
                    tail->next = node;
                }
                if(tail->next == node){
                    good=1;
                }
                if (good) {
                    //enqueue is done, try to zwing tail to the inserted node
                    //CAS(&q->tail,tail,<node,tail.count+1>)
                    #pragma omp atomic compare
                    if (q->tail == tail){
                        q->tail = node;
                    }
                    //enqueue is done
                    return;
                }
                //tail was not pointing to the last node
            } else {
                // cas(&q->tail,tail,<next.ptr, tail.count_1>)
                //try to swing tail to the next node
                #pragma omp atomic compare
                if(q->tail == tail){
                    q->tail =next;
                }
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
        node_lf_t *head;
        #pragma omp atomic read
        head = q->head;
        //tail=q->tail, read tail
        node_lf_t *tail;
        #pragma omp atomic read
        tail = q->tail;
        //next=head->next, read head next
        node_lf_t *next;
        #pragma omp atomic read
        next= head->next;

        #pragma omp flush
        //if head == q->head, are head tail and next consistent?
        if (head == q->head) {
            //if head.ptr == tail.ptr, is queue empty or tail falling behind?
            if (head == tail) {
                //if next.ptr==null, is queue empty?
                if (next == NULL) {
                    // Queue empty, cannot dequeue
                    return -1;
                }
                //cas(&q->tail, tail <next.ptr,tail.count+1>)
                //tail falling behind, try to advance it
                #pragma omp atomic compare
                if(q->tail == tail){
                    q->tail = next;
                }
            }
            //no need to deal with tail
            else {
                //if cas(&q->head, head, <next.ptr,head.count+1>)
                int value = next->data;
                int good =0;
                #pragma omp atomic compare
                if (q->head == head){
                    q->head=next;
                }
                if (q->head == next){
                    good=1;
                }
                if (good) {
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