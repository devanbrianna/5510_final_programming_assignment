CC = gcc 
CFLAGS = -std=c11 -O2 -Wall -Wextra -fopenmp

all: test_lockfree_omp_ca test_qlock_omp


# -------------------------
# LOCK-FREE with C atomics inside OpenMP test harness
# -------------------------
test_lockfree_omp_ca: test_lockfree_openmp_ca.o qlockfree_c_atomic.o
	$(CC) $(CFLAGS) -o test_lockfree_omp_ca test_lockfree_openmp_ca.o qlockfree_c_atomic.o

# Compile the test file (same test file, different output object)
test_lockfree_openmp_ca.o: test_lockfree_openmp.c
	$(CC) $(CFLAGS) -c test_lockfree_openmp.c -o test_lockfree_openmp_ca.o

# -------------------------
# QLOCK tested with OpenMP test harness
# -------------------------
test_qlock_omp: test_qlock_openmp.o qlock.o
	$(CC) $(CFLAGS) -o test_qlock_omp test_qlock_openmp.o qlock.o

test_qlock_openmp.o: test_qlock_openmp.c
	$(CC) $(CFLAGS) -c test_qlock_openmp.c


# -------------------------
clean:
	rm -f *.o test_lockfree_omp_ca test_qlock_omp