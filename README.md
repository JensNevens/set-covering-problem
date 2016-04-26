# Set Covering Problem

Different algorithms for solving the Set Covering Problem in C. The lsscp.c file contains four constructive heuristics:

 1. Random constructive heuristic
 2. Static cost-based heuristic
 3. Static cover cost-based heuristic
 4. Adaptive cover cost-based heuristic

Also, iterative best- and first-improvement algorithms are implemented. Redundancy elimination can be applied after the constructive heuristic and after each iteration of the iterative improvement algorithms.

## Running the code

A bash script is included to automatically run the code. This can be done by executing the following command:

    ./run.sh ./path/to/executable ./path/to/instances ./path/to/output-dir

This script will run all 4 constructive heuristics with and without redundancy elimination, followed by the first and fourth constructive heuristic with the iterative first- and best-improvement algorithms.

This project is part of the course Heuristic Optimization given at VUB and ULB.

_by Jens Nevens_
