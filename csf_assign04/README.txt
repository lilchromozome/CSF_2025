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

We test our code on a 16MB binary file of randomly generated 64-bit integers.
The program has been ran by using different threshold values and recorded the real time that taken to sort the data.
These experimentes were run on ugrad5 machine. The results are as follows:

Threshold       Real Times(s)       User Times(s)       System times(s)
2097152         0.389s              0.372s              0.011s
1048576         0.231s              0.402s              0.017s
524288          0.181s              0.444s              0.026s
262144          0.167s              0.535s              0.049s
131072          0.156s              0.546s              0.069s
65536           0.167s              0.593s              0.085s
32768           0.203s              0.610s              0.119s
16384           0.178s              0.616s              0.176s

