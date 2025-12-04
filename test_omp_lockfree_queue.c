#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#include "omp_lockfree_queue.h"

#define MILL_OP         1000000
#define HALF_MILL_OP     500000

static omp_lf_queue_t q;

/* Utility timing */
static inline double now() {
    return omp_get_wtime();
}

/**************************************
 * Test 1: 1,000,000 concurrent enqueues
 **************************************/
double test1_enqueue(int nthreads) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();

        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            if (omp_lf_enqueue(&q, value) != 0) {
                printf("ERROR: enqueue overflow!\n");
            }
        }
    }

    return now() - start;
}

/**************************************
 * Test 2: 1,000,000 concurrent dequeues
 **************************************/
double test2_dequeue(int nthreads, int *found) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int data;
        for (int i = 0; i < ops_per_thread; i++) {
            while (omp_lf_dequeue(&q, &data) != 0)
                ; // spin until available

            if (data >= 0 && data < MILL_OP)
                found[data]++;
        }
    }

    return now() - start;
}

/**************************************
 * Verify Tests 1 + 2
 **************************************/
int verify_test1_2(int *found) {
    for (int i = 0; i < MILL_OP; i++) {
        if (found[i] != 1) {
            printf("ERROR: value %d appeared %d times\n", i, found[i]);
            return -1;
        }
    }
    return 0;
}

/**************************************
 * Test 3: 500,000 concurrent enqueues
 **************************************/
double test3_enqueue(int nthreads) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            omp_lf_enqueue(&q, value);
        }
    }

    return now() - start;
}

/**************************************
 * Test 4: alternating enqueue + dequeue
 **************************************/
double test4_alternating(int nthreads, int *found) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();

        for (int i = 0; i < ops_per_thread; i++) {

            int val_in = HALF_MILL_OP + tid * ops_per_thread + i;
            omp_lf_enqueue(&q, val_in);

            int val_out;
            while (omp_lf_dequeue(&q, &val_out) != 0)
                ;

            if (val_out >= 0 && val_out < MILL_OP)
                found[val_out]++;
        }
    }

    return now() - start;
}

/**************************************
 * Verify Test 4
 **************************************/
int verify_test4(int *found) {
    int total = 0;

    for (int i = 0; i < HALF_MILL_OP; i++)
        total += found[i];

    if (total != HALF_MILL_OP) {
        printf("ERROR: Test 4 expected %d pops, got %d\n",
               HALF_MILL_OP, total);
        return -1;
    }

    return 0;
}

/**************************************
 * Main Driver
 **************************************/
int main(void) {

    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_configs = 5;

    printf("Testing OpenMP Lock-Free Queue\n");
    printf("Threads\tTest1\t\tTest2\t\tTest3\t\tTest4\n");

    for (int c = 0; c < num_configs; c++) {
        int nth = thread_counts[c];

        for (int run = 0; run < 3; run++) {

            /* Init queue with capacity ≥ 1M */
            omp_lf_queue_init(&q, MILL_OP);

            printf("%d\t", nth);

            /* Test 1 */
            int *found12 = calloc(MILL_OP, sizeof(int));
            double t1 = test1_enqueue(nth);

            /* Test 2 */
            double t2 = test2_dequeue(nth, found12);
            if (verify_test1_2(found12) != 0) return -1;
            free(found12);

            /* Reset queue for Test 3 */
            omp_lf_queue_destroy(&q);
            omp_lf_queue_init(&q, MILL_OP);

            /* Test 3 */
            int *found34 = calloc(MILL_OP, sizeof(int));
            double t3 = test3_enqueue(nth);

            /* Test 4 */
            double t4 = test4_alternating(nth, found34);
            if (verify_test4(found34) != 0) return -1;
            free(found34);

            printf("%.6f\t%.6f\t%.6f\t%.6f\n", t1, t2, t3, t4);

            omp_lf_queue_destroy(&q);
        }
    }

    return 0;
}
