CONTRIBUTIONS

TODO: write a brief summary of how each team member contributed to
the project.

Will:
Sorting algorithm
Input handling
Error handling

Sichen:
Sorting algorithm
Analysis

REPORT

TODO: add your report according to the instructions in the
"Experiments and analysis" section of the assignment description.

REPORT:
Our sorting algorithm seems to work up to a limit of 100000000 input values ~100MB. However due to this alrge file size and long write and sort time, we decided to leave our code to not optimize for this. 
We hypothesize this could be due to a memory limit of simultaneous process runs with so many forks.

In our implementation, whenever the array size exceeds the threshold, the merge_sort() forks two child processes to sort the left and right halves in parallel. These child processes run independently 
and can br scheduled by the OS kernel on separate CPU cores. This parallelism allows different parts of the sorting computation to proceed simultaneously reducing overall time. As more processes are 
created at the smaller thresholds, the amount of available concurrency increases, but so does the cost of managing them.

We test our code on a 16MB binary file of randomly generated 64-bit integers.
The program has been ran by using different threshold values and recorded the real time that taken to sort the data.
These experimentes were run on ugrad5 machine. The results are as follows:

Threshold       Real Times(s)       User Times(s)       System times(s)
2097152         0.182               0.180               0.011
1048576         0.099               0.177               0.000
524288          0.063               0.176               0.012
262144          0.056               0.164               0.047
131072          0.044               0.182               0.014
65536           0.043               0.212               0.005
32768           0.046               0.281               0.012
16384           0.044               0.314               0.027

From our results, we observed that decreading the threshold initially reduced the total runing time, reaching the best performance around a threshold of 65536.
However, when the threshold was reduced further, the performance stopped improving and even degraded.

We initially hypothesized that sequential sorting becomes faster then parrallel merge sort at some threshold due to the algorithmic complexity. 
But since seq_sort() is based on qsort(), which typically runs in O(nlogn), so complecity differences shouldn't be the primary factor.

The main factor might be the overhead of process creation. When the threshold is small, merge_sort() forks more frequently due to recursive and also ate the deeper recursion levels, 
which would cause an exponential growth in the number of processes, and cause the signitifantly gigher system time. As we can see from our result, for the threshold below 65536, the user time rise from 0.212s to 0.314s and the system time rise from 0.005s to 0.027s.
