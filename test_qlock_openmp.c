#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#include "qlock.h"

#define MILL_OP         1000000
#define HALF_MILL_OP     500000

static qlock_t q;

/* Timing helper */
static inline double now() {
    return omp_get_wtime();
}

/*****************************************
 * Test 1 — 1,000,000 concurrent enqueues
 *****************************************/
double test1_enqueue(int nthreads) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();

        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            qlock_enq(&q, value);
        }
    }

    return now() - start;
}

/*****************************************
 * Test 2 — 1,000,000 concurrent dequeues
 *****************************************/
double test2_dequeue(int nthreads, int *found) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int data;
        for (int i = 0; i < ops_per_thread; i++) {
            while (qlock_deq(&q, &data) != 0)
                ;
            if (data >= 0 && data < MILL_OP)
                found[data]++;
        }
    }

    return now() - start;
}

/*****************************************
 * Verify Test 1 and Test 2
 *****************************************/
int verify_test1_2(int *found) {
    for (int i = 0; i < MILL_OP; i++) {
        if (found[i] != 1) {
            printf("ERROR: value %d appeared %d times\n", i, found[i]);
            return -1;
        }
    }
    return 0;
}

/*****************************************
 * Test 3 — 500,000 enqueues
 *****************************************/
double test3_enqueue(int nthreads) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();

        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            qlock_enq(&q, value);
        }
    }

    return now() - start;
}

/*****************************************
 * Test 4 — alternating enqueue + dequeue
 *****************************************/
double test4_alternating(int nthreads, int *found) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();

        for (int i = 0; i < ops_per_thread; i++) {
            int value_in = HALF_MILL_OP + tid * ops_per_thread + i;
            qlock_enq(&q, value_in);

            int value_out;
            while (qlock_deq(&q, &value_out) != 0)
                ;

            if (value_out >= 0 && value_out < MILL_OP)
                found[value_out]++;
        }
    }

    return now() - start;
}

/*****************************************
 * Verify Test 4
 *****************************************/
int verify_test4(int *found) {
    int total = 0;

    for (int i = 0; i < HALF_MILL_OP; i++)
        total += found[i];

    if (total != HALF_MILL_OP) {
        printf("ERROR: Test4 expected %d pops, got %d\n",
               HALF_MILL_OP, total);
        return -1;
    }

    return 0;
}

/*****************************************
 * Main Driver
 *****************************************/
int main(void) {
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_configs = 5;

    printf("Testing OpenMP Lock-Based Queue\n");
    printf("Threads\tTest1\t\tTest2\t\tTest3\t\tTest4\n");

    for (int c = 0; c < num_configs; c++) {
        int nth = thread_counts[c];

        for (int r = 0; r < 3; r++) {

            /* Initialize queue */
            qlock_init(&q);

            printf("%d\t", nth);

            /* TEST 1 — enqueue 1M */
            int *found12 = calloc(MILL_OP, sizeof(int));
            double t1 = test1_enqueue(nth);

            /* TEST 2 — dequeue 1M */
            double t2 = test2_dequeue(nth, found12);
            if (verify_test1_2(found12) != 0) return -1;
            free(found12);

            /* TEST 3 — enqueue 0.5M */
            qlock_init(&q);
            int *found34 = calloc(MILL_OP, sizeof(int));
            double t3 = test3_enqueue(nth);

            /* TEST 4 — alternating */
            double t4 = test4_alternating(nth, found34);
            if (verify_test4(found34) != 0) return -1;
            free(found34);

            printf("%.6f\t%.6f\t%.6f\t%.6f\n", t1, t2, t3, t4);
        }
    }

    return 0;
}
