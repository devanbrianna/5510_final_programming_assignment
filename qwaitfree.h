#ifndef _QWAITFREE_H
#define _QWAITFREE_H

//https://csaws.cs.technion.ac.il/~erez/Papers/wfquque-ppopp.pdf

#include <stdatomic.h>

//figure 1 in the paper
typedef struct node_wf   node_wf_t;
typedef struct opdesc_wf opdesc_wf_t;

//class node
//hold elements of queues underlying list
//enqtid & deqtid hold the id of the threads that preforms
//or has already preformed the insertion or removal of node to queue
struct node_wf {
    //int value
    int data;
    //atomicReference<Node> next
    _Atomic(node_wf_t*) next;
    //int enqTid
    int enqTid;
    //atomic int deqTid
    _Atomic int deqTid;
};
//operation descriptor record for each thread. 
//info about the phase at which the thread has preformed (or is preforming)
//its last op on the queue(phase), the type of op (enq/deq), flag specifying if it has 
//a pending op (pending), and ref to node with a menaing specific to the type of op (node)
struct opdesc_wf {
    //long phase
    long phase;
    //bool pending, 1true 0false
    int  pending;
    //bool enqueue, 1enq 0deq
    int  enqueue;
    //node node
    node_wf_t *node;
};

typedef struct {
    _Atomic(node_wf_t*) head;
    _Atomic(node_wf_t*) tail;
    int numThreads;
    _Atomic(opdesc_wf_t*) *state;
} qwaitfree_t;

//numthreads-1
extern _Thread_local int qwaitfree_tid;

//all helpers
static void wf_help(qwaitfree_t *q, long phase);
static void wf_help_enq(qwaitfree_t *q, int tid, long phase);
static void wf_help_finish_enq(qwaitfree_t *q);
static void wf_help_deq(qwaitfree_t *q, int tid, long phase);
static void wf_help_finish_deq(qwaitfree_t *q);
void qwaitfree_init(qwaitfree_t *q, int numThreads);
void qwaitfree_enq(qwaitfree_t *q, int data);
int  qwaitfree_deq(qwaitfree_t *q, int *data);

#endif /* _QWAITFREE_H */
