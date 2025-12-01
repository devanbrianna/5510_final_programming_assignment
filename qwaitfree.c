#include "qwaitfree.h"
#include <stdlib.h>
#include <stdio.h>

//num threads -1
_Thread_local int qwaitfree_tid = -1;

//FIGURE1
//helpers 
//Node(int val, int etid)
static node_wf_t *wf_node_new(int value, int enqTid)
{
    //create a new node
    node_wf_t *n = malloc(sizeof(node_wf_t));
    //value=val
    n->data = value;
    //next= new atomicReference<Node> null
    atomic_store(&n->next, NULL);
    //enqTid=etid
    n->enqTid = enqTid;
    //deqTid=new atomicInt(-1)
    atomic_store(&n->deqTid, -1);
    //return node
    return n;
}

//opDesc(long ph, bool pend, bool enq, node n)
static opdesc_wf_t *wf_opdesc_new(long phase, int pending, int enqueue, node_wf_t *node)
{
    opdesc_wf_t *d = malloc(sizeof(opdesc_wf_t));
    //phase=ph
    d->phase= phase;
    //pending=pend
    d->pending = pending;
    //enqueue=enq
    d->enqueue = enqueue;
    //node=n
    d->node= node;
    //return operation descriptior
    return d;
}

//FIGURE2
//note num threads in dec, may need to change for test 
void qwaitfree_init(qwaitfree_t *q, int numThreads)
{
    q->numThreads = numThreads;
    //node sentinel = new node (-1,-1)
    node_wf_t *sentinel = wf_node_new(-1, -1);
    //head= new atomicRef<Node> sen
    atomic_store(&q->head, sentinel);
    //tail= new atomicRef<Node> sen
    atomic_store(&q->tail, sentinel);
    //state= new atomicRefArray<opdesc>numthreads
    q->state = malloc(sizeof(_Atomic(opdesc_wf_t*)) * numThreads);
    //for int i =0, i<state.length, i++
    for (int i = 0; i < numThreads; i++) {
        //state.set(i,new,opdesc(-1,false,true,null)) had 001null
        opdesc_wf_t *init = wf_opdesc_new(-1, 0, 1, NULL);
        atomic_store(&q->state[i], init);
    }
}

static void wf_help(qwaitfree_t *q, long phase)
{
    //for int i=0,i<statelength,i++
    for (int i = 0; i < q->numThreads; i++) {
        //opdesc desc=state.get(i)
        opdesc_wf_t *d = atomic_load_explicit(&q->state[i],memory_order_seq_cst);
        //if desc.pending && desc.phase <=phase
        if (d->pending && d->phase <= phase) {
            //if desc.enqueue
            if (d->enqueue==1) {
                //help enq(i,phase)
                wf_help_enq(q, i, phase);
            } else {
                //help deq(i,phase)
                wf_help_deq(q, i, phase);
            }
        }
    }
}

static long wf_maxPhase(qwaitfree_t *q)
{
    //long maxphase =-1
    long maxPhase = -1;
    //for int i=0,i<statelength,i++
    for (int i = 0; i < q->numThreads; i++) {
        //long phase= state.get(i).phase
        opdesc_wf_t *d = atomic_load_explicit(&q->state[i],memory_order_seq_cst);
        //if(phase>maxphase)
        if (d->phase > maxPhase) {
            //reset max phase
            maxPhase = d->phase;
        }
    }
    return maxPhase;
}

static int wf_isStillPending(qwaitfree_t *q, int tid, long phase)
{
    //return state.get(tid).pending && state.get(tid).phase <= ph
    opdesc_wf_t *d = atomic_load_explicit(&q->state[tid],memory_order_seq_cst);
    return (d->pending && d->phase <= phase);
}


//FIGURE4
void qwaitfree_enq(qwaitfree_t *q, int value)
{
    int tid = qwaitfree_tid;
    //try
    if (tid < 0 || tid >= q->numThreads) abort();
    //long phase = maxPhase+1
    long phase = wf_maxPhase(q) + 1;
    //
    node_wf_t *n   = wf_node_new(value, tid);
    opdesc_wf_t *d = wf_opdesc_new(phase, 1, 1, n);
    //state.set(tid,new opdesc(phase,true,true,new node(value tid)))
    atomic_store(&q->state[tid], d);
    //help(phase)
    wf_help(q, phase);
    //help finish enq()
    wf_help_finish_enq(q);

    return;
}

static void wf_help_enq(qwaitfree_t *q, int tid, long phase)
{
    //while is still pending (tid,phase)
    while (wf_isStillPending(q, tid, phase)) {
        //node last=tail.get
        node_wf_t *last = atomic_load_explicit(&q->tail,memory_order_seq_cst);
        //node next = last.next.get
        node_wf_t *next = atomic_load_explicit(&last->next,memory_order_seq_cst);
        //if last ==tail.get
        if (last == atomic_load_explicit(&q->tail,memory_order_seq_cst)) {
            //ifnext==null, enq can be applied
            if (next == NULL) {
                //if isstillpending(tid,phase)
                if (wf_isStillPending(q, tid, phase)) {

                    opdesc_wf_t *d = atomic_load_explicit(&q->state[tid],memory_order_seq_cst);
                    node_wf_t   *myNode = d->node;
                    
                    //if (last.next.compareAndSet(next, state.get(tid).node)) 
                    if (atomic_compare_exchange_strong_explicit(&last->next,&next,myNode,memory_order_seq_cst, memory_order_relaxed)) {
                        //helpfinishenq()
                        wf_help_finish_enq(q);
                        return;
                    }
                }
            } 
            //some enq is in progress
            else {
                //helpfinishenq
                //help it first then retry
                wf_help_finish_enq(q);
            }
        }
    }
}


static void wf_help_finish_enq(qwaitfree_t *q)
{
    //node last = tail.get
    node_wf_t *last = atomic_load_explicit(&q->tail, memory_order_seq_cst);
    //node next=last.next.get
    node_wf_t *next = atomic_load_explicit(&last->next,memory_order_seq_cst);
    //if next ==null    nothing 
    if (next == NULL) {
        return;
    }
    //next != null
    //int tid = next.enqtid
    int tid = next->enqTid;
    //nothing 
    if (tid < 0 || tid >= q->numThreads) {
        return;
    }
    //if (last == tail.get() && state.get(tid).node == next)
    //OpDesc curDesc = state.get(tid)
    opdesc_wf_t *curDesc = atomic_load_explicit(&q->state[tid],memory_order_seq_cst);

    //if if (last == tail.get() && state.get(tid).node == next) // try with pending for 2
    if (last == atomic_load_explicit(&q->tail,memory_order_seq_cst) && curDesc->node == next){
        //OpDesc newDesc = new OpDesc(state.get(tid).phase, false, true, next);
        opdesc_wf_t *newDesc =wf_opdesc_new(curDesc->phase,0,1,next);
        //state.compareAndSet(tid, curDesc, newDesc
        atomic_compare_exchange_strong_explicit(&q->state[tid],&curDesc, newDesc,memory_order_seq_cst,memory_order_relaxed);
        //tail.compareAndSet(last, next)
        atomic_compare_exchange_strong_explicit(&q->tail,&last,next,memory_order_seq_cst,memory_order_relaxed);
    }
    
}



//FIGURE6

int qwaitfree_deq(qwaitfree_t *q, int *data)
{
    int tid = qwaitfree_tid;
    //try
    if (tid < 0 || tid >= q->numThreads) abort();
    //long phase=maxphase +1
    long phase = wf_maxPhase(q) + 1;
    //state.set(TID, new OpDesc(phase, true, false, null))
    opdesc_wf_t *d = wf_opdesc_new(phase, 1, 0, NULL);
    atomic_store(&q->state[tid], d);
    //help(phase)
    wf_help(q, phase);
    //help finish deq
    wf_help_finish_deq(q);
    //node node=state.get(tid).node
    opdesc_wf_t *finalDesc = atomic_load_explicit(&q->state[tid],memory_order_seq_cst);
    node_wf_t   *node      = finalDesc->node;
    //if node==null
    if (node == NULL) {
        //empty
        return -1;
    }
    //target= node.next.get .value
    node_wf_t *target = atomic_load_explicit(&node->next,memory_order_seq_cst);
    if (!target) {
        ///just incase, maybe break here?
        return -1;
    }
    //*data is tagrget->data
    *data = target->data;
    return 0;
}


static void wf_help_deq(qwaitfree_t *q, int tid, long phase)
{
    //while(isstillpending(tid,phase))
    while (wf_isStillPending(q, tid, phase)) {
        //node first=head.get
        node_wf_t *first = atomic_load_explicit(&q->head,memory_order_seq_cst);
        //node last=tail.get
        node_wf_t *last  = atomic_load_explicit(&q->tail,memory_order_seq_cst);
        //node next= first.next.get
        node_wf_t *next  = atomic_load_explicit(&first->next,memory_order_seq_cst);
        //add this maybe break here, if head change?
        if (first != atomic_load_explicit(&q->head,memory_order_seq_cst)) {
            continue;  
        }
        //if first ==last 
        if (first == last) {
            //queue might be empty
            //if next ==null
            if (next == NULL) {
                //queue is def empty
                //OpDesc curDesc = state.get(tid)
                opdesc_wf_t *curDesc = atomic_load_explicit(&q->state[tid],memory_order_seq_cst);
                //if (last == tail.get() && isStillPending(tid, phase))
                if (last==atomic_load_explicit(&q->tail,memory_order_seq_cst)&& wf_isStillPending(q,tid,phase)) {
                    //OpDesc newDesc = newOpDesc(state.get(tid).phase, false, false, null);
                    opdesc_wf_t *newDesc = wf_opdesc_new(curDesc->phase,0,0,NULL);
                    //state.compareAndSet(tid, curDesc, newDesc)
                    atomic_compare_exchange_strong_explicit(&q->state[tid],&curDesc,newDesc,memory_order_seq_cst,memory_order_relaxed);
                }
                //done bc queue is empty
                
            } 
            else {
                //enqueue maybe in progress
                //help it then retry
                wf_help_finish_enq(q);
            }
        } 
        else {
            //queue is not empty
            //OpDesc curDesc = state.get(tid);
            opdesc_wf_t *curDesc = atomic_load_explicit(&q->state[tid],memory_order_seq_cst);
            //node node=currdesc.node
            node_wf_t *node=curDesc->node;
            //if (!isStillPending(tid, phase)) break;
            if (!wf_isStillPending(q,tid,phase)) {
                break;
            }
            //if (first == head.get() && node != first)
            if (first==atomic_load_explicit(&q->head,memory_order_seq_cst)&& node!=first) {
                //OpDesc newDesc = new OpDesc(state.get(tid).phase, true, false, first)
                opdesc_wf_t *newDesc = wf_opdesc_new(curDesc->phase,1,0,first);
                //if (!state.compareAndSet(tid, curDesc, newDesc))
                if(!atomic_compare_exchange_strong_explicit(&q->state[tid], &curDesc,newDesc, memory_order_seq_cst, memory_order_relaxed)){
                    continue;
                }
            }
            //first.deqTid.compareAndSet(-1, tid);
            int expected=-1;
            atomic_compare_exchange_strong_explicit(&first->deqTid,&expected,tid,memory_order_seq_cst, memory_order_relaxed);
            //help finish deq()
            wf_help_finish_deq(q);
        }
    }
}

static void wf_help_finish_deq(qwaitfree_t *q)
{
    // node first = head.get;
    node_wf_t *first = atomic_load_explicit(&q->head,memory_order_seq_cst);
    //check maybe break here
    if (first == NULL) {
        return;
    }
    //node next = first.next.get
    node_wf_t *next = atomic_load_explicit(&first->next,memory_order_seq_cst);
    // int tid = first.deqTid.get()
    int tid = atomic_load_explicit(&first->deqTid,memory_order_seq_cst);
    // if (tid != -1)
    if (tid == -1) {
        return;
    }
    //here !=-1
    //OpDesc curDesc = state.get(tid);
    opdesc_wf_t *curDesc = atomic_load_explicit(&q->state[tid],memory_order_seq_cst);
    //if (first == head.get() && next != null)
    if(first == atomic_load_explicit(&q->head, memory_order_seq_cst) && next != NULL){
        //OpDesc newDesc = new 
        node_wf_t *node= curDesc->node;
        //OpDesc(state.get(tid).phase, false, false,state.get(tid).node);
        opdesc_wf_t *newDesc = wf_opdesc_new(curDesc->phase, 0,0, node);
        //state.compareAndSet(tid, curDesc, newDesc)
        atomic_compare_exchange_strong_explicit(&q->state[tid],&curDesc,newDesc,memory_order_seq_cst,memory_order_relaxed);
        //head.compareAndSet(first, next); 
        atomic_compare_exchange_strong_explicit(&q->head, &first, next, memory_order_seq_cst, memory_order_relaxed);
    }
}
