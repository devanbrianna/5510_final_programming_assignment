Brianna Rodriguez, ECE 5510 Midterm Project

this report documents the preformance of three queue implementations:
- Lock-based queue using TTAS spinlock from part a
- Lock-free queue using the Michael-Scott algorithm
- Wait-free queue using the Kogan-Petrank algorithm 
 
 Preformace for each test is measured in terms of time to execute. Throughput (enqueues/second, dequeues/second, mixed enqueues-dequeues/second) is shown below each table, using the averages per thread per test. 
 - Test 1: 1,000,000 concurrent enqueue operations
 - Test 2: 1,000,000 concurrent dequeue operations 
 - Test 3: 500,000 concurrent enqueue operations
 - Test 4: 500,000 concurrent alternating enqueue and dequeue operations, verifying 500,000 remain in the queue after this test is complete. 

### Results

### TABLE 1:LOCK-BASED QUEUE RESULTS (execution time per test)
| Threads | Test1     | Test2     | Test3     | Test4     |
|---------|-----------|-----------|-----------|-----------|
| 1       | 0.074051  | 0.027300  | 0.015339  | 0.018106  |
| 1       | 0.058426  | 0.026969  | 0.015392  | 0.016996  |
| 1       | 0.059291  | 0.026267  | 0.015369  | 0.017416  |
| 2       | 0.135187  | 0.156992  | 0.036510  | 0.050181  |
| 2       | 0.146193  | 0.166872  | 0.036775  | 0.048197  |
| 2       | 0.144231  | 0.165470  | 0.037523  | 0.049971  |
| 4       | 0.220073  | 0.257885  | 0.053426  | 0.091700  |
| 4       | 0.223930  | 0.262240  | 0.053154  | 0.090803  |
| 4       | 0.225295  | 0.249140  | 0.052763  | 0.089332  |
| 8       | 0.306787  | 0.351120  | 0.062368  | 0.121577  |
| 8       | 0.303704  | 0.353254  | 0.058687  | 0.118663  |
| 8       | 0.305457  | 0.351584  | 0.058758  | 0.115584  |
| 16      | 0.388692  | 0.450813  | 0.065885  | 0.164175  |
| 16      | 0.386545  | 0.443897  | 0.064283  | 0.154909  |
| 16      | 0.391090  | 0.451019  | 0.064535  | 0.153215  |

**average time for each thread count**
| Threads | Test1 Avg  | Test2 Avg  | Test3 Avg  | Test4 Avg  |
|---------|------------|------------|------------|------------|
| **1**  | 0.063923   | 0.026845   | 0.015367   | 0.017506   |
| **2**  | 0.141870   | 0.163111   | 0.036936   | 0.049450   |
| **4**  | 0.223099   | 0.256422   | 0.053114   | 0.090612   |
| **8**  | 0.305316   | 0.351986   | 0.059271   | 0.118608   |
| **16** | 0.388776   | 0.448576   | 0.064901   | 0.157433   |

**Throughput(operations/second)**
| Threads | Test1  | Test2  | Test3  | Test4  |
|---------|------------------|------------------|------------------|------------------|
| **1**  | 15,646,000 | 37,244,000 | 32,540,000 | 28,565,000 |
| **2**  | 7,049,000  | 6,131,000  | 13,540,000 | 10,108,000 |
| **4**  | 4,483,000  | 3,900,000  | 9,412,000  | 5,517,000  |
| **8**  | 3,275,000  | 2,841,000  | 8,438,000  | 4,216,000  |
| **16** | 2,573,000  | 2,229,000  | 7,702,000  | 3,175,000  |



### TABLE 2:LOCK-FREE QUEUE RESULTS(execution time per test)
| Threads | Test1     | Test2     | Test3     | Test4     |
|---------|-----------|-----------|-----------|-----------|
| 1       | 0.082255  | 0.015299  | 0.037125  | 0.043796  |
| 1       | 0.082425  | 0.014021  | 0.037094  | 0.042979  |
| 1       | 0.082146  | 0.014004  | 0.036932  | 0.042969  |
| 2       | 0.272743  | 0.110855  | 0.076466  | 0.114997  |
| 2       | 0.181265  | 0.111629  | 0.075436  | 0.117398  |
| 2       | 0.181100  | 0.110609  | 0.075523  | 0.115458  |
| 4       | 0.401190  | 0.149131  | 0.187448  | 0.269764  |
| 4       | 0.409124  | 0.154214  | 0.191515  | 0.265934  |
| 4       | 0.410321  | 0.152649  | 0.190003  | 0.267857  |
| 8       | 0.758694  | 0.225205  | 0.388226  | 0.435091  |
| 8       | 0.738968  | 0.220212  | 0.398178  | 0.442129  |
| 8       | 0.728074  | 0.218654  | 0.392640  | 0.435642  |
| 16      | 0.911394  | 0.281951  | 0.500069  | 0.525757  |
| 16      | 0.914799  | 0.285532  | 0.496924  | 0.523392  |
| 16      | 0.911349  | 0.283099  | 0.504934  | 0.525374  |

**average time for each thread count**
| Threads | Test1 Avg   | Test2 Avg   | Test3 Avg   | Test4 Avg   |
|---------|-------------|-------------|-------------|-------------|
| **1**  | 0.082275    | 0.014441    | 0.037050    | 0.043248    |
| **2**  | 0.211703    | 0.111031    | 0.075808    | 0.115951    |
| **4**  | 0.406878    | 0.151998    | 0.189655    | 0.267852    |
| **8**  | 0.741912    | 0.221357    | 0.393015    | 0.437621    |
| **16** | 0.912514    | 0.283527    | 0.500642    | 0.524841    |

**Throughput (operations/second)**
| Threads | Test1  | Test2  | Test3  | Test4  |
|---------|------------------|------------------|------------------|------------------|
| **1**  | 12,151,000 | 69,230,000 | 13,503,000 | 11,565,000 |
| **2**  | 4,725,000  | 9,004,000  | 6,596,000  | 4,313,000  |
| **4**  | 2,458,000  | 6,579,000  | 2,636,000  | 1,867,000  |
| **8**  | 1,348,000  | 4,515,000  | 1,272,000  | 1,143,000  |
| **16** | 1,095,000  | 3,526,000  | 999,000    | 953,000    |


I chose to implement the Michael-Scott non-blocking concurrent queue algorithm for the lock free queue. I chose this algorithm becuase it the industry standard for lock free FIFO queues. It also has well documented psuedo code, which helped tremendously during implementation. 

Source: https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf 

### TABLE 3:WAIT-FREE QUEUE RESULTS
| Threads | Test1     | Test2     | Test3     | Test4     |
|---------|-----------|-----------|-----------|-----------|
| 1       | 0.241188  | 0.237422  | 0.117424  | 0.236469  |
| 1       | 0.240504  | 0.233857  | 0.116800  | 0.233277  |
| 1       | 0.240042  | 0.234235  | 0.117090  | 0.234433  |
| 2       | 0.414772  | 0.484883  | 0.194876  | 0.402032  |
| 2       | 0.416300  | 0.485754  | 0.197220  | 0.407298  |
| 2       | 0.415311  | 0.483726  | 0.195496  | 0.409700  |
| 4       | 1.816505  | 2.447853  | 0.902370  | 1.795423  |
| 4       | 2.034837  | 2.243256  | 0.846462  | 1.788501  |
| 4       | 1.850242  | 1.450517  | 0.296794  | 1.795356  |
| 8       | 1.839333  | 1.982821  | 0.634754  | 1.471381  |
| 8       | 1.918121  | 2.216140  | 0.923842  | 1.568925  |
| 8       | 2.012416  | 2.342150  | 0.729641  | 1.622520  |
| 16      | 2.241353  | 2.773510  | 1.107396  | 2.409020  |
| 16      | 2.661227  | 3.188407  | 1.000217  | 1.843894  |
| 16      | 2.962113  | 3.367748  | 1.083264  | 2.300092  |

**average time for each thread count**
| Threads | Test1 Avg | Test2 Avg | Test3 Avg | Test4 Avg |
| ------- | --------- | --------- | --------- | --------- |
| **1**   | 0.240578  | 0.235171  | 0.117105  | 0.234726  |
| **2**   | 0.415461  | 0.484788  | 0.195864  | 0.406343  |
| **4**   | 1.900528  | 2.047209  | 0.681875  | 1.793093  |
| **8**   | 1.923290  | 2.180370  | 0.762746  | 1.554275  |
| **16**  | 2.621564  | 3.109888  | 1.063626  | 2.184335  |

**Throughput (operations/second)**
| Threads | Test1  | Test2  | Test3  | Test4  |
|---------|------------------|------------------|------------------|------------------|
| **1**  | 4,156,000 | 4,252,000 | 4,269,000 | 2,130,000 |
| **2**  | 2,406,000 | 2,062,000 | 2,553,000 | 1,230,000 |
| **4**  | 526,000   | 488,000   | 734,000   | 279,000   |
| **8**  | 520,000   | 459,000   | 655,000   | 322,000   |
| **16** | 382,000   | 321,000   | 470,000   | 229,000   |


I chose to implement this algorithm for 2 main reasons. (1)It built off the Michael-Scott algorithm, which is the algorithm I chose for the lock-free implementation, and (2) it had the best documented pseudocode for any wait free algorithm I could find. You can see in my implementation (qwaitfree.h and qwaitfree.c) I follow the algorithm as close as I can. 

source: https://csaws.cs.technion.ac.il/~erez/Papers/wfquque-ppopp.pdf

Overall the lock-based implementation was the fastest, followed by the Michael-Scott lock-free implementation, and the slowest was the Kogan-Petrank wait-fee algorithm. 