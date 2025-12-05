#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "omp_vyukov_queue.h"

#define MILL_OP        1000000
#define HALF_MILL_OP    500000

static vyukov_queue_t q;

/***********************
 * Utility timing
 ***********************/
double now() {
    return omp_get_wtime();
}

/***********************
 * TEST 1 — 1M ENQUEUES
 ***********************/
double test1_enqueue(int nthreads) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            while (vyukov_enqueue(&q, value) != 0)
                ;
        }
    }

    return now() - start;
}

/***********************
 * TEST 2 — 1M DEQUEUES
 ***********************/
double test2_dequeue(int nthreads, int *found) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int data;
        for (int i = 0; i < ops_per_thread; i++) {
            while (vyukov_dequeue(&q, &data) != 0)
                ;

            if (data >= 0 && data < MILL_OP)
                found[data]++;
        }
    }

    return now() - start;
}

/***********************
 * VERIFICATION TEST 1+2
 ***********************/
int verify_test1_2(int *found) {
    for (int i = 0; i < MILL_OP; i++) {
        if (found[i] != 1) {
            printf("ERROR: value %d appeared %d times\n", i, found[i]);
            return -1;
        }
    }
    return 0;
}

/***********************
 * TEST 3 — 500k ENQUEUES
 ***********************/
double test3_enqueue(int nthreads) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            while (vyukov_enqueue(&q, value) != 0)
                ;
        }
    }

    return now() - start;
}

/***********************
 * TEST 4 — ALTERNATING
 ***********************/
double test4_alternating(int nthreads, int *found) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();

        for (int i = 0; i < ops_per_thread; i++) {
            int in = HALF_MILL_OP + tid * ops_per_thread + i;
            while (vyukov_enqueue(&q, in) != 0)
                ;

            int out;
            while (vyukov_dequeue(&q, &out) != 0)
                ;

            if (out >= 0 && out < MILL_OP)
                found[out]++;
        }
    }

    return now() - start;
}

int verify_test4(int *found) {
    int total = 0;
    for (int i = 0; i < HALF_MILL_OP; i++)
        total += found[i];

    if (total != HALF_MILL_OP) {
        printf("ERROR: expected %d pops, got %d\n", HALF_MILL_OP, total);
        return -1;
    }
    return 0;
}

/***********************
 * MAIN
 ***********************/
int main() {
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_configs = 5;

    printf("Testing Vyukov Lock-Free Queue (OpenMP)\n");
    printf("Threads\tTest1\tTest2\tTest3\tTest4\n");

    for (int c = 0; c < num_configs; c++) {
        int nth = thread_counts[c];

        for (int run = 0; run < 3; run++) {

            vyukov_init(&q, MILL_OP);

            printf("%d\t", nth);

            /* --- Test 1 --- */
            int *found12 = calloc(MILL_OP, sizeof(int));
            double t1 = test1_enqueue(nth);

            /* --- Test 2 --- */
            double t2 = test2_dequeue(nth, found12);
            if (verify_test1_2(found12) != 0) return -1;
            free(found12);

            /* --- Reset queue --- */
            vyukov_init(&q, MILL_OP);

            /* --- Test 3 --- */
            int *found34 = calloc(MILL_OP, sizeof(int));
            double t3 = test3_enqueue(nth);

            /* --- Test 4 --- */
            double t4 = test4_alternating(nth, found34);
            if (verify_test4(found34) != 0) return -1;
            free(found34);

            printf("%.6f\t%.6f\t%.6f\t%.6f\n", t1, t2, t3, t4);
        }
    }

    return 0;
}
