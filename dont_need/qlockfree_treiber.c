#include "qlockfree_treiber.h"

void treiber_push(treiber_stack_t *s, int value)
{
    treiber_node_t *node = malloc(sizeof(treiber_node_t));
    if (!node) {
        perror("malloc");
        exit(1);
    }

    node->value = value;

    while (1) {
        treiber_node_t *old_top;

        // Atomically read current top
        #pragma omp atomic read
        old_top = s->top;

        node->next = old_top;

        // Try to CAS top from old_top to node
        treiber_node_t *observed_top;

        // We need a separate variable to participate in the compare
        #pragma omp atomic read
        observed_top = s->top;

        if (observed_top == old_top) {
            // Now attempt to store node into top if still equal
            int success = 0;

            #pragma omp atomic compare
            if (s->top == old_top){
                s->top = node;
            }

            // Check if CAS succeeded
            #pragma omp atomic read
            observed_top = s->top;

            if (observed_top == node){
                success = 1;
            }

            if (success){
                return;
            }
        }

        // Else: someone else pushed; retry
    }
}

int treiber_pop(treiber_stack_t *s, int *out)
{
    while (1) {
        treiber_node_t *old_top;

        // Read current top
        #pragma omp atomic read
        old_top = s->top;

        if (old_top == NULL) {
            // Empty stack
            return -1;
        }

        treiber_node_t *new_top = old_top->next;

        // Try to CAS top from old_top to new_top
        treiber_node_t *observed_top;

        #pragma omp atomic read
        observed_top = s->top;

        if (observed_top == old_top) {
            int success = 0;

            #pragma omp atomic compare
            if (s->top == old_top){
                s->top = new_top;
            }

            #pragma omp atomic read
            observed_top = s->top;

            if (observed_top == new_top){
                success = 1;
            }

            if (success) {
                // For simplicity, DON'T free node here to avoid ABA headaches
                // free(old_top);  // optional, but careful with reuse
                *out = old_top->value;
                return 0;
            }
        }

        // Else: someone changed top; retry
    }
}
