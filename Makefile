CC = gcc-15
CFLAGS = -std=c11 -O2 -Wall -Wextra -fopenmp

all: test_qlock test_lockfree test_waitfree test_treiber

# -------------------------
# LOCK-BASED
# -------------------------
test_qlock: test_all_queues_qlock.o qlock.o
	$(CC) $(CFLAGS) -o test_qlock test_all_queues_qlock.o qlock.o

test_all_queues_qlock.o: test_all_queues.c
	$(CC) $(CFLAGS) -DUSE_QLOCK -c test_all_queues.c -o test_all_queues_qlock.o


# -------------------------
# LOCK-FREE (MS queue — legacy)
# -------------------------
test_lockfree: test_all_queues_lockfree.o qlockfree.o
	$(CC) $(CFLAGS) -o test_lockfree test_all_queues_lockfree.o qlockfree.o

test_all_queues_lockfree.o: test_all_queues.c
	$(CC) $(CFLAGS) -DUSE_LOCKFREE -c test_all_queues.c -o test_all_queues_lockfree.o


# -------------------------
# WAIT-FREE
# -------------------------
test_waitfree: test_all_queues_waitfree.o qwaitfree.o
	$(CC) $(CFLAGS) -o test_waitfree test_all_queues_waitfree.o qwaitfree.o

test_all_queues_waitfree.o: test_all_queues.c
	$(CC) $(CFLAGS) -DUSE_WAITFREE -c test_all_queues.c -o test_all_queues_waitfree.o


# -------------------------
# TREIBER STACK (OpenMP lock-free)
# -------------------------
test_treiber: test_treiber.o qlockfree_treiber.o
	$(CC) $(CFLAGS) -o test_treiber test_treiber.o qlockfree_treiber.o

test_treiber.o: test_treiber.c
	$(CC) $(CFLAGS) -c test_treiber.c

qlockfree_treiber.o: qlockfree_treiber.c qlockfree_treiber.h
	$(CC) $(CFLAGS) -c qlockfree_treiber.c

test_omp_queue: test_omp_lockfree_queue.o omp_lockfree_queue.o
	$(CC) $(CFLAGS) -o test_omp_queue test_omp_lockfree_queue.o omp_lockfree_queue.o

test_qlock_omp: test_qlock_openmp.o qlock.o
	$(CC) $(CFLAGS) -o test_qlock_omp test_qlock_openmp.o qlock.o



# -------------------------
clean:
	rm -f *.o test_qlock test_lockfree test_waitfree test_treiber test_omp_queue test_qlock_omp
