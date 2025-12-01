#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>


#if defined(USE_QLOCK)
#include "qlock.h"
typedef qlock_t queue_t;
#define QUEUE_NAME "lock-based"
#define QUEUE_INIT(q,n)      qlock_init(q)
#define QUEUE_ENQ(q,v,tid)   qlock_enq(q,v)
#define QUEUE_DEQ(q,pv,tid)  qlock_deq(q,pv)

#elif defined(USE_LOCKFREE)
#include "qlockfree.h"
typedef qlockfree_t queue_t;
#define QUEUE_NAME "lock-free"
#define QUEUE_INIT(q,n)      lock_free_init(q)
#define QUEUE_ENQ(q,v,tid)   lock_free_enq(q,v)
#define QUEUE_DEQ(q,pv,tid)  lock_free_deq(q,pv)

#elif defined(USE_WAITFREE)
#include "qwaitfree.h"
typedef qwaitfree_t queue_t;
extern _Thread_local int qwaitfree_tid;   // defined in qwaitfree.c
#define QUEUE_NAME "wait-free"
#define QUEUE_INIT(q,n)      qwaitfree_init(q,n)
#define QUEUE_ENQ(q,v,tid)   do { qwaitfree_tid = (tid); qwaitfree_enq(q,v); } while (0)
#define QUEUE_DEQ(q,pv,tid)  (qwaitfree_tid = (tid), qwaitfree_deq(q,pv))

#else
#error "You must define one of USE_QLOCK, USE_LOCKFREE, USE_WAITFREE"
#endif
#if defined(__x86_64__)
#define cpu_relax() __asm__ __volatile__("pause" ::: "memory")
#else
#define cpu_relax() do {} while(0)
#endif


// bc i have a mac
//https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrier_wait.html
//https://blog.albertarmea.com/post/47089939939/using-pthreadbarrier-on-mac-os-x
#if defined(__APPLE__) && defined(__MACH__)

typedef struct {
    pthread_mutex_t m;
    pthread_cond_t  c;
    unsigned count;
    unsigned trip;
} pthread_barrier_t;

static int pthread_barrier_init(pthread_barrier_t* b, void* attr, unsigned count) {
    (void)attr;
    pthread_mutex_init(&b->m,NULL);
    pthread_cond_init(&b->c,NULL);
    b->count = 0;
    b->trip  = count;
    return 0;
}
static int pthread_barrier_wait(pthread_barrier_t* b) {
    pthread_mutex_lock(&b->m);
    b->count++;
    if (b->count == b->trip) {
        b->count = 0;
        pthread_cond_broadcast(&b->c);
        pthread_mutex_unlock(&b->m);
        return 1; // like PTHREAD_BARRIER_SERIAL_THREAD
    }
    pthread_cond_wait(&b->c,&b->m);
    pthread_mutex_unlock(&b->m);
    return 0;
}
static int pthread_barrier_destroy(pthread_barrier_t* b) {
    pthread_mutex_destroy(&b->m);
    pthread_cond_destroy(&b->c);
    return 0;
}
#endif
/*Verify the content queue with $10^{6}$ concurrent enqueue() calls (in separate tests  2, 4, 8, and 16 threads), with a data value from 1 to $10^{6}$. 
Each thread should perform the same number of enqueues (approximately, since the number of threads may not divide evenly the number of enqueue operations). 
You should verify the queue's content with a single thread (sequentially), scanning the queue from start to finish and expecting to find all the inserted values. 
Make sure that each thread enqueue inserts a distinct value (you may find it helpful to use thread IDs for this and partition the range of values between 
threads according to their ID). Obviously, you will not be able to control the order in which the data elements are inserted in the queue.

Following verification of the inserts, verify the queue with $10^{6}$ concurrent dequeues(), using the same parameters (varying the number of threads and 
having each thread perform approximately the same number of dequeues). Your test should verify that you return existing values from the queue, 
that all distinct values are returned, and that the queue is empty at the end of the test.

Create a new test to verify $0.5\times10^{6}$ concurrent enqueues, which need to be completed and then followed by $0.5\times10^{6}$ concurrent dequeues 
and $0.5\times10^{6}$ concurrent enqueues. You need to use a barrier to verify completion of the first enqueue batch. For the second stage of the test, 
alternate enqueue() and dequeue() calls in each thread. Your queue at the end of this test should be left with half a million valid data items 
from previous enqueues, and all your dequeues should return valid data items. Each thread should perform approximately the same number of enqueues and dequeues, 
and you should vary the number of threads as in the previous tests.

You should evaluate the performance of your lock-based and lock-free/wait-free queues (15% of your grade) and provide a report with your findings, 
along with an explanation for them. You should measure the performance of the queues in terms of throughput (enqueues/second, dequeues/second, 
mixed enqueues-dequeues/second), and you can use the same tests that you used for verification without the verification code for this purpose. 
Your report should be added as a markdown file to your GitHub repository. You should also provide a comprehensive readme file that explains how to precisely 
run your verification and performance tests. All your verification and performance tests should be reproducible in terms of our ability to execute them, 
verify the correctness of your code, and approximately verify your results (system conditions may, of course, vary during executions, but over many executions, 
our experimental results from running your codes should be similar to yours).

- Tests are spit into 4 parts,
  1. 10^6 concurrent enqueues
  2. 10^6 concurrent dequeues
  3. 0.5 x 10^5 concurrent enqueues
  4. 0.5 x 10^5 concurrent alternating enqueues and dequeues
- Verification of results occurs between each test. Any failed test will result in the program printing an error and exiting.
- The tests are timed only during multithreaded execution and the verification occurs in the main thread outside of the timing boundaries.
- Output is in the form of a table, each set of 3 rows corresponds to the thread count (3 runs at the same config for averaging), each column corresponds to one of the four tests above in order from left to right. thread counts being tested are 1, 2, 4, 8, 16 as the wait-free queue has been capped at 16 threads.
*/

//from spinlock test
static volatile uint32_t wflag;
/* Wait on a flag to make all threads start almost at the same time. */
void wait_flag(volatile uint32_t *flag, uint32_t expect) {
    __sync_fetch_and_add((uint32_t *)flag, 1);
    while (*flag != expect) {
        cpu_relax();
    }
}

static struct timeval start_time;
static struct timeval end_time;

static void calc_time(struct timeval *start, struct timeval *end) {
    if (end->tv_usec < start->tv_usec) {
        end->tv_sec -= 1;
        end->tv_usec += 1000000;
    }

    assert(end->tv_sec >= start->tv_sec);
    assert(end->tv_usec >= start->tv_usec);
    struct timeval interval = {
        end->tv_sec - start->tv_sec,
        end->tv_usec - start->tv_usec
    };
    printf("%ld.%06ld\t", (long)interval.tv_sec, (long)interval.tv_usec);   //changed from \t to \n
}

#ifdef BIND_CORE
void bind_core(int threadid) {
    /* cores with logical id 2x   are on node 0 */
    /* cores with logical id 2x+1 are on node 1 */
    /* each node has 16 cores, 32 hyper-threads */
    int phys_id = threadid / 16;
    int core = threadid % 16;

    int logical_id = 2 * core + phys_id;
    /*printf("thread %d bind to logical core %d on physical id %d\n", threadid, logical_id, phys_id);*/

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(logical_id, &set);

    if (sched_setaffinity(0, sizeof(set), &set) != 0) {
        perror("Set affinity failed");
        exit(EXIT_FAILURE);
    }
}
#endif

//my testing constants
#define MILL_OP 1000000
#define HALF_MILL_OP 500000
#define NUM_THREAD_CONFIGS 5
#define NUM_ITER          3
static const int THREAD_COUNTS[] = {1, 2, 4, 8, 16};
static pthread_barrier_t barrier;
static int num_threads = 0; //set in main
static queue_t q;
atomic_int fail=0;
static int *found_12=NULL;
static int *found_34=NULL;
static atomic_int count1=0;

//test 1, 1 million concurrent enqueues. check inserted vals 1-1000000 each appear once
void *test1_enqueue(void *id) {
    int thread_id = (int)(intptr_t)id;
    int ops_per_thread = MILL_OP / num_threads;

    #ifdef BIND_CORE
        bind_core(thread_id);
    #endif

    wait_flag(&wflag, num_threads);
    if (thread_id == 0) {
        gettimeofday(&start_time, NULL);
    }

    for (int i = 0; i < ops_per_thread; i++) {
        //ie, 4 threads: 
        //value=0*250000 + 1-250000 --> value= 1*250000 + 1-250000 --> etc, nums 1-1000000
        int value = thread_id * ops_per_thread + i;
        QUEUE_ENQ(&q, value, thread_id);
        atomic_fetch_add(&count1,1);
    }

    if (__sync_fetch_and_add(&wflag, -1) == 1) {
        gettimeofday(&end_time, NULL);
    }

    return NULL;
}


//test2, this  checks test 1 while it executes test 2
//1m dequeues, verify all values 1-1000000 appear once
void *test2_dequeue(void *id) {
    int thread_id=(int)(intptr_t)id;
    int ops_per_thread= MILL_OP/num_threads;
    //found_12 = calloc(MILL_OP, sizeof(int)); 
    #ifdef BIND_CORE
        bind_core(thread_id);
    #endif

    //start time
    wait_flag(&wflag, num_threads);
    if (thread_id == 0) {
        gettimeofday(&start_time, NULL);
    }
    int data;
    //split among the threads, count values 
    for(int i=0;i<ops_per_thread;i++){
        while(QUEUE_DEQ(&q,&data,thread_id) !=0){
            cpu_relax();
        }
        if(data<0 || data>=MILL_OP){
            printf("\nFAIL TEST2: out of range calue %d (thread %d)",data,thread_id);
            atomic_store(&fail,-1);
            continue;
        }
        found_12[data]++;
    }
    //end time
    if (__sync_fetch_and_add(&wflag, -1) == 1) {
        gettimeofday(&end_time, NULL);
    }
    
    //yay!!
    return NULL;
}

int verify_t1_2(){
    //TEST 1 verify 
    //check for 1m values
    if(count1 != MILL_OP){
        printf("\nFAIL TEST 1, not 1M values enqueued");
        printf("\n\t only %d values in queue", count1);
        atomic_store(&fail, -1);
    }
    int count=0;
    for(int j=0; j<MILL_OP;j++){
        count+=found_12[j];
    }
    if (count<MILL_OP){
        printf("\nFAIL TEST 2, not 1M values dequeued");
        printf("\n\t only %d values in queue", count);
        atomic_store(&fail, -1);
    }
    //check each iten in queue was unique
    for (int k=0;k<MILL_OP;k++){
        if(found_12[k]==0){
            printf("\nFAIL TEST 1, value %d is missing", k);
            atomic_store(&fail, -1);
            break;
        }
        if(found_12[k]>1){
            printf("\nFAIL TEST 1, value %d found more than once", k);
            atomic_store(&fail, -1);
            break;
        }
    }
    if (atomic_load(&fail)==-1){
        return -1;
    }
    return 0;
}

//test3 .5M enqueues w barrier
void *test3_enqueue(void *id){
    int thread_id=(int)(intptr_t)id;
    int ops_per_thread = HALF_MILL_OP / num_threads;
    #ifdef BIND_CORE
        bind_core(thread_id);
    #endif
    //start time
    wait_flag(&wflag, num_threads);
    if (thread_id == 0) {
        gettimeofday(&start_time, NULL);
    }
    for (int i=0;i<ops_per_thread;i++){
        int data= thread_id*ops_per_thread+i;
        QUEUE_ENQ(&q,data,thread_id);
    }
    if (__sync_fetch_and_add(&wflag, -1) == 1) {
        gettimeofday(&end_time, NULL);
    }
    pthread_barrier_wait(&barrier);

    return NULL;
}


//test 4 , alternating enq and deq, verify half million at end
//need to enqueue top half of 1 million to verify later (500000-1000000)
void *test4_alternating(void *id){
    int thread_id=(int)(intptr_t)id;
    int ops_per_thread= HALF_MILL_OP/num_threads;
    //found_34 = calloc(MILL_OP, sizeof(int));

    #ifdef BIND_CORE
        bind_core(thread_id);
    #endif
    wait_flag(&wflag, num_threads);
    if (thread_id == 0) {
        gettimeofday(&start_time, NULL);
    }
    for (int i=0;i<ops_per_thread;i++){
        int data_in=thread_id*ops_per_thread+i+HALF_MILL_OP;
        QUEUE_ENQ(&q, data_in,thread_id);
        int data_out;
        while(QUEUE_DEQ(&q,&data_out,thread_id)!=0){
            cpu_relax();
        }
        //increase that index of found
        found_34[data_out]++;
    }
    if (__sync_fetch_and_add(&wflag, -1) == 1) {
        gettimeofday(&end_time, NULL);
    }


    //continue 
    return NULL;

}

int verify_t3_4(){
    //verify
    int count=0;
    for(int j=0;j<HALF_MILL_OP;j++){
        count+=found_34[j];
    }
    if (count<HALF_MILL_OP){
        printf("FAIL TEST 4, more than half million values left in queue");
        atomic_store(&fail, -1);
    }
    else if (count>HALF_MILL_OP){
        printf("FAIL TEST 4, less than half million values left in queue");
        atomic_store(&fail, -1);
    }  
    
    if (atomic_load(&fail)==-1){
        return -1;
    }
    return 0;
}

void run_test(int test_num, int num_threads_) {
  num_threads = num_threads_;
  atomic_store(&fail,0);
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
  wflag = 0;
  gettimeofday(&start_time, NULL);
  gettimeofday(&end_time, NULL);
  if (test_num == 1) {
    for (int i = 0; i < num_threads; i++) {
      pthread_create(&threads[i], NULL, test1_enqueue, (void *)(intptr_t)i);
    }
  } else if (test_num == 2) {
    for (int i = 0; i < num_threads; i++) {
      pthread_create(&threads[i], NULL, test2_dequeue, (void *)(intptr_t)i);
    }
  } else if (test_num == 3) {
    for (int i = 0; i < num_threads; i++) {
      pthread_create(&threads[i], NULL, test3_enqueue, (void *)(intptr_t)i);
    }
  } else if (test_num == 4) {
    for (int i = 0; i < num_threads; i++) {
      pthread_create(&threads[i], NULL, test4_alternating, (void *)(intptr_t)i);
    }
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }
  calc_time(&start_time, &end_time);
  free(threads);
}

int main(void) {
    printf("Testing queue: %s\n", QUEUE_NAME);
    printf("Threads\tTest1\t\tTest2\t\tTest3\t\tTest4\n");

    for (int i = 0; i < NUM_THREAD_CONFIGS; i++) {
        int nth = THREAD_COUNTS[i];
        for (int r = 0; r < NUM_ITER; r++) {

            // Init queue and buffers
            QUEUE_INIT(&q, nth);

            printf("%d\t", nth);

            // Test 1
            found_12 = calloc(MILL_OP, sizeof(int));
            run_test(1, nth);

            // Test 2
            run_test(2, nth);
            if (verify_t1_2() != 0) {
                printf("\nERROR: Test 1/2 failed\n");
                return -1;
            }
            free(found_12);
            found_12=NULL;
            count1=0;
            // Test 3 (with barrier)
            found_34 = calloc(MILL_OP, sizeof(int));
            pthread_barrier_init(&barrier, NULL, nth);
            run_test(3, nth);
            pthread_barrier_destroy(&barrier);

            // Test 4
            run_test(4, nth);
            if (verify_t3_4() != 0) {
                printf("\nERROR: Test 4 failed\n");
                return -1;
            }
            free(found_34);
            found_34=NULL;

            printf("\n");
        }
    }

    return 0;
}
