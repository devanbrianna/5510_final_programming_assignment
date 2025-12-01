Brianna Rodriguez

ECE 5510 Midterm Assignment

### Test Instructions
login to the cs server--> ssh ~your username~@rlogin.cs.vt.edu

clone my repo --> git clone https://github.com/Virginia-Tech-CS-ECE-5510/midterm-programming-assignment-part-b-devanbrianna.git

cd into --> 'cd midterm-programming-assignment-part-b-devanbrianna'

make the executable --> 'make'

for lock-based queue test --> './test_qlock'

for lock-free queue test --> './test_lockfree'

for wait-free queue test --> './test_waitfree'

output will be displayed in execution time. to convert to throughput:
- test 1: 1,000,000/execution time
- test 2: 1,000,000/execution time
- test 3: 500,000/execution time
- test 4: 500,000/ execution time

[Read my report](report.md)

### Original ReadMe

The purpose of your second programming assignment, originally created by Dr. Nikolopoulos, is to implement efficient lock-free and wait-free (your choice) FIFO queues. You will observe during the assignment that wait-free implementations of FIFO queues with arbitrary numbers of enqueuers and dequeuers can be challenging. You will find solutions to this problem in the literature we share with you. That said, you are also free to constrain the number of concurrent enqueuers/dequeuers in your wait-free implementation without grade penalty if this helps you complete the assignment.

Your queues should be simply linked lists with head and tail pointers. The head is always a sentinel node, while the tail, depending on your implementation, could be a pointer to the last node or a sentinel node. The queue should have two methods:

```
enqueue (Q, int data) #enqueue a node with content "data" at the tail of the queue pointed to by Q
dequeue(Q, int *data) #dequeue a node from the head of the queue pointed by Q and return its data
```

The starting point of this assignment should be the best lock you have experimented with in the previous assignment. We ask you to clone your first assignment repository, where you keep only that best lock (notionally, this should be the TTAS lock with a carefully tuned back-off). You should then create a simple FIFO queue data structure in C, using pointers, and enable multithreaded enqueued and dequeues using the best lock you implemented thus far for mutual exclusion.

## Deliverables

You should implement at least one lock-free queue (35% of your grade), and at least one wait-free queue (35%) of your grade. The choice of algorithms is entirely yours, but we do expect a justification of your choice in your report. The justification can be experimental (based on measurements on different implementations), or theoretical (based on your understanding of the behavior of different algorithms under contention).

You should conduct a series of correctness checks (15% of your grade) of your lock-free and your wait-free queues as follows:

Verify the content queue with $10^{6}$ concurrent enqueue() calls (in separate tests  2, 4, 8, and 16 threads), with a data value from 1 to $10^{6}$. Each thread should perform the same number of enqueues (approximately, since the number of threads may not divide evenly the number of enqueue operations). You should verify the queue's content with a single thread (sequentially), scanning the queue from start to finish and expecting to find all the inserted values. Make sure that each thread enqueue inserts a distinct value (you may find it helpful to use thread IDs for this and partition the range of values between threads according to their ID). Obviously, you will not be able to control the order in which the data elements are inserted in the queue.

Following verification of the inserts, verify the queue with $10^{6}$ concurrent dequeues(), using the same parameters (varying the number of threads and having each thread perform approximately the same number of dequeues). Your test should verify that you return existing values from the queue, that all distinct values are returned, and that the queue is empty at the end of the test.

Create a new test to verify $0.5\times10^{6}$ concurrent enqueues, which need to be completed and then followed by $0.5\times10^{6}$ concurrent dequeues and $0.5\times10^{6}$ concurrent enqueues. You need to use a barrier to verify completion of the first enqueue batch. For the second stage of the test, alternate enqueue() and dequeue() calls in each thread. Your queue at the end of this test should be left with half a million valid data items from previous enqueues, and all your dequeues should return valid data items. Each thread should perform approximately the same number of enqueues and dequeues, and you should vary the number of threads as in the previous tests.

You should evaluate the performance of your lock-based and lock-free/wait-free queues (15% of your grade) and provide a report with your findings, along with an explanation for them. Yoou should measure the performance of the queues in terms of throughput (enqueues/secnd, dequeues/second, mixed enqueues-dequeues/second), and you can use the same tests that you used for verification without the verification code for this purpose. Your report should be added as a markdown file to your GitHub repository. You should also provide a comprehensive readme file that explains how to precisely run your verification and performance tests. All your verification and performance tests should be reproducible in terms of our ability to execute them, verify the correctness of your code, and approximately verify your results (system conditions may, of course, vary during executions, but over many executions, our experimental results from running your codes should be similar to yours).

## Expected tests and result format

- Tests are spit into 4 parts,
  1. 10^6 concurrent enqueues
  2. 10^6 concurrent dequeues
  3. 0.5 x 10^5 concurrent enqueues
  4. 0.5 x 10^5 concurrent alternating enqueues and dequeues
- Verification of results occurs between each test. Any failed test will result in the program printing an error and exiting.
- The tests are timed only during multithreaded execution and the verification occurs in the main thread outside of the timing boundaries.
- Output is in the form of a table, each set of 3 rows corresponds to the thread count (3 runs at the same config for averaging), each column corresponds to one of the four tests above in order from left to right. thread counts being tested are 1, 2, 4, 8, 16 as the wait-free queue has been capped at 16 threads.

## References
In searching for an efficient concurrent queue algorithm to implement, you can start from references [125], [126], and [164] in your textbook. 
