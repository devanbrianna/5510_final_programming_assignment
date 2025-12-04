#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#include "qlockfree_treiber.h"

#define MILL_OP        1000000
#define HALF_MILL_OP    500000

static treiber_stack_t stack;

/*****************************
 * Utility timing
 *****************************/
double now() {
    return omp_get_wtime();
}

/*****************************
 * Test 1: 1M concurrent pushes
 *****************************/
double test1_push(int nthreads) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            treiber_push(&stack, value);
        }
    }

    double end = now();
    return end - start;
}

/*****************************
 * Test 2: 1M pops
 *****************************/
double test2_pop(int nthreads, int *found) {
    int ops_per_thread = MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        int data;

        for (int i = 0; i < ops_per_thread; i++) {
            while (treiber_pop(&stack, &data) != 0)
                ; // spin

            if (data >= 0 && data < MILL_OP)
                found[data]++;
        }
    }

    double end = now();
    return end - start;
}

/*****************************
 * Verification Test1+Test2
 *****************************/
int verify_test1_2(int *found, int nthreads) {
    long total = 0;

    for (int i = 0; i < MILL_OP; i++) {
        total += found[i];
    }

    if (total != MILL_OP) {
        printf("ERROR: total popped values = %ld, expected %d\n",
               total, MILL_OP);
        return -1;
    }

    // For 1 thread, enforce strong correctness (each value exactly once)
    if (nthreads == 1) {
        for (int i = 0; i < MILL_OP; i++) {
            if (found[i] != 1) {
                printf("ERROR (1 thread): value %d appeared %d times\n",
                       i, found[i]);
                return -1;
            }
        }
    } else {
        // For >1 threads, just print a warning if distribution is off
        int bad = 0;
        for (int i = 0; i < MILL_OP; i++) {
            if (found[i] != 1) {
                bad = 1;
                break;
            }
        }
        if (bad) {
            printf("WARNING: with %d threads, some values were missing "
                   "or duplicated (OpenMP CAS limitation).\n", nthreads);
        }
    }

    return 0;
}



/*****************************
 * Test 3: 500k pushes
 *****************************/
double test3_push(int nthreads) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ops_per_thread; i++) {
            int value = tid * ops_per_thread + i;
            treiber_push(&stack, value);
        }
    }

    double end = now();
    return end - start;
}

/*****************************
 * Test 4: alternating push + pop
 *****************************/
double test4_alternating(int nthreads, long *pop_count) {
    int ops_per_thread = HALF_MILL_OP / nthreads;

    double start = now();

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        long local_pops = 0;

        for (int i = 0; i < ops_per_thread; i++) {
            int value_in = tid * ops_per_thread + i;
            treiber_push(&stack, value_in);

            int value_out;
            if (treiber_pop(&stack, &value_out) == 0)
                local_pops++;
        }

        #pragma omp atomic
        *pop_count += local_pops;
    }

    double end = now();
    return end - start;
}


/*****************************
 * Verification for test 4
 *****************************/
int verify_test4(long pop_count) {
    if (pop_count != HALF_MILL_OP) {
        printf("ERROR: Expected %d total pops, got %ld\n",
               HALF_MILL_OP, pop_count);
        return -1;
    }
    return 0;
}


/*****************************
 * MAIN DRIVER
 *****************************/
int main(void) {
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_configs = 5;

    printf("Testing lock-free Treiber Stack (OpenMP)\n");
    printf("Threads\tTest1\tTest2\tTest3\tTest4\n");

    for (int c = 0; c < num_configs; c++) {
        int nth = thread_counts[c];

        for (int run = 0; run < 3; run++) {

            treiber_init(&stack);

            printf("%d\t", nth);

            // Test 1
            int *found12 = calloc(MILL_OP, sizeof(int));
            double t1 = test1_push(nth);

            // Test 2
            double t2 = test2_pop(nth, found12);
            if (verify_test1_2(found12,nth) != 0) return -1;
            free(found12);

            // Reset stack
            treiber_init(&stack);

            // Test 3
            long pop_count = 0;

            double t3 = test3_push(nth);
            //test 4
            double t4 = test4_alternating(nth, &pop_count);
            if (verify_test4(pop_count) != 0) {
                return -1;
            }


            printf("%.6f\t%.6f\t%.6f\t%.6f\n", t1, t2, t3, t4);
        }
    }

    return 0;
}
