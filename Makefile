CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -pthread

all: test_qlock test_lockfree test_waitfree

# -------------------------
# LOCK-BASED
# -------------------------
test_qlock: test_all_queues_qlock.o qlock.o
	$(CC) $(CFLAGS) -o test_qlock test_all_queues_qlock.o qlock.o

test_all_queues_qlock.o: test_all_queues.c
	$(CC) $(CFLAGS) -DUSE_QLOCK -c test_all_queues.c -o test_all_queues_qlock.o


# -------------------------
# LOCK-FREE
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
clean:
	rm -f *.o test_qlock test_lockfree test_waitfree
