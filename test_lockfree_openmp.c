#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#include "qlockfree.h"

#define MILL_OP        1000000
#define HALF_MILL_OP    500000

static qlockfree_t q;

/*****************************
 * Utility timing functions
 *****************************/
double now() {
    return omp_get_wtime();
}

/*****************************
 * Test 1: 1M concurrent enqueues
 *****************************/
double test1_enqueue(int nthreads) {
    int ops_per_thread = MILL_OP / nthreads;
    
    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            lock_free_enq(&q, value);
        }
    }

    double end = now();
    return end - start;
}

/*****************************
 * Test 2: 1M dequeues
 *****************************/
double test2_dequeue(int nthreads, int *found) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        int data;
        for (int i = 0; i < ops_per_thread; i++) {
            while (lock_free_deq(&q, &data) != 0)
                ; // spin
            if (data >= 0 && data < MILL_OP)
                found[data]++;
        }
    }

    double end = now();
    return end - start;
}

/*****************************
 * Verification for Tests 1/2
 *****************************/
int verify_test1_2(int *found) {
    for (int i = 0; i < MILL_OP; i++) {
        if (found[i] != 1) {
            printf("ERROR: value %d appeared %d times\n", i, found[i]);
            return -1;
        }
    }
    return 0;
}

/*****************************
 * Test 3: 500k enqueues
 *****************************/
double test3_enqueue(int nthreads) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ops_per_thread; i++) {
            int val = tid * ops_per_thread + i;
            lock_free_enq(&q, val);
        }
    }

    double end = now();
    return end - start;
}

/*****************************
 * Test 4: alternating enqueue/dequeue
 *****************************/
double test4_alternating(int nthreads, int *found) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ops_per_thread; i++) {
            int data_in = HALF_MILL_OP + tid * ops_per_thread + i;
            lock_free_enq(&q, data_in);

            int data_out;
            while (lock_free_deq(&q, &data_out) != 0)
                ;

            if (data_out >= 0 && data_out < MILL_OP)
                found[data_out]++;
        }
    }

    double end = now();
    return end - start;
}

/*****************************
 * Verification for Test 4
 *****************************/
int verify_test4(int *found) {
    int total = 0;
    for (int i = 0; i < HALF_MILL_OP; i++)
        total += found[i];

    if (total != HALF_MILL_OP) {
        printf("ERROR: Test 4 expected %d dequeues, got %d\n", HALF_MILL_OP, total);
        return -1;
    }
    return 0;
}

/*****************************
 * MAIN DRIVER
 *****************************/
int main() {
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_configs = 5;

    printf("Testing queue: lock-free (OpenMP)\n");
    printf("Threads\tTest1\tTest2\tTest3\tTest4\n");

    for (int i = 0; i < num_configs; i++) {
        int nth = thread_counts[i];

        for (int r = 0; r < 3; r++) {
            lock_free_init(&q);

            printf("%d\t", nth);

            // Test 1
            int *found12 = calloc(MILL_OP, sizeof(int));
            double t1 = test1_enqueue(nth);

            // Test 2
            double t2 = test2_dequeue(nth, found12);
            if (verify_test1_2(found12) != 0) return -1;
            free(found12);

            // Test 3
            lock_free_init(&q);
            int *found34 = calloc(MILL_OP, sizeof(int));
            double t3 = test3_enqueue(nth);

            // Test 4
            double t4 = test4_alternating(nth, found34);
            if (verify_test4(found34) != 0) return -1;
            free(found34);

            printf("%.6f\t%.6f\t%.6f\t%.6f\n", t1, t2, t3, t4);
        }
    }

    return 0;
}
