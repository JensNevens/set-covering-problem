//
//  lsscp.c
//  HO-Project1
//
//  Created by Jens Nevens on 24/02/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

/*************************************/
/** For this code:                  **/
/**   rows = elements               **/
/**   cols = subsets                **/
/*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "lsscp.h"

/*** Algorithm parameters **/
int seed = 1234567;
char* scp_file = "";
char* output_file = "output.txt";

/** Variables to activate algorithms **/
int ch1 = 0, ch2 = 0, ch3 = 0, ch4 = 0, bi = 0, fi = 0, re = 0;

/** Instance static variables **/
int m;            /* number of rows */
int n;            /* number of columns */
int** row;        /* row[i] contains rows that are covered by column i */
int** col;        /* col[i] contains columns that cover row i */
int* ncol;        /* ncol[i] contains number of columns that cover row i */
int* nrow;        /* nrow[i] contains number of rows that are covered by column i */
int* cost;        /* cost[i] contains cost of column i  */
float* ccost;     /* ccost[i] contains static cover cost of column i */

/** Solution variables **/
int* x;           /* x[i] 0,1 if column i is selected */
int* y;           /* y[i] 0,1 if row i covered by the actual solution */
int fx;         /* sum of the cost of the columns selected in the solution (can be partial) */

/** Dynamic variables **/
/** Note: use dynamic variables to make easier the construction and modification of solutions. **/
/** these are just examples of useful variables. **/
/** These variables need to be updated eveytime a column is added to a partial solution **/
/** or when a complete solution is modified */
int** col_cover;  /* col_colver[i] contains selected columns that cover row i */
int* ncol_cover;  /* ncol_cover[i] contains number of selected columns that cover row i */
int un_rows;      /* the amount of un-covered rows */
int un_cols;      /* the amoung of un-used columns */

void usage() {
    printf("\nUSAGE: lsscp [param_name, param_value] [options]...\n");
    printf("Parameters:\n");
    printf("  --seed : seed to initialize random number generator\n");
    printf("  --instance: SCP instance to execute.\n");
    printf("  --output: Filename for output results.\n");
    printf("Options:\n");
    printf("  --ch1: random solution construction\n");
    printf("  --ch2: static cost-based greedy values.\n");
    printf("  --ch3: static cover cost-based greedy values.\n");
    printf("  --ch4: adaptive cover cost-based greedy values.\n");
    printf("  --re: applies redundancy elimination after construction.\n");
    printf("  --bi: best improvement.\n");
    printf("  --fi: first improvement.\n");
    printf("\n");
}


/*** Read parameters from command line*/
void readParameters(int argc, char* argv[]) {
    int i;
    if (argc <= 1) {
        usage();
        exit(EXIT_FAILURE);
    }
    for(i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0) {
            seed = atoi(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--instance") == 0) {
            scp_file = argv[i+1];
            i += 1;
        } else if (strcmp(argv[i], "--output") == 0) {
            output_file = argv[i+1];
            i += 1;
        } else if (strcmp(argv[i], "--ch1") == 0) {
            if (ch2 || ch3 || ch4) {
                errorExit("ERROR: Only use 1 constructive algorithm.\n");
            }
            ch1 = 1;
        } else if (strcmp(argv[i], "--ch2") == 0) {
            if (ch1 || ch3 || ch4) {
                errorExit("ERROR: Only use 1 constructive algorithm.\n");
            }
            ch2 = 1;
        } else if (strcmp(argv[i], "--ch3") == 0) {
            if (ch1 || ch2 || ch4) {
                errorExit("ERROR: Only use 1 constructive algorithm.\n");
            }
            ch3 = 1;
        } else if (strcmp(argv[i], "--ch4") == 0) {
            if (ch1 || ch2 || ch3) {
                errorExit("ERROR: Only use 1 constructive algorithm.\n");
            }
            ch4 = 1;
        } else if (strcmp(argv[i], "--bi") == 0) {
            if (fi) {
                errorExit("ERROR: Only use 1 iterative algorithm.");
            }
            bi = 1;
        } else if (strcmp(argv[i], "--fi") == 0) {
            if (bi) {
                errorExit("ERROR: Only use 1 iterative algorithm.\n");
            }
            fi = 1;
        } else if (strcmp(argv[i], "--re") == 0) {
            re = 1;
        } else {
            printf("ERROR: parameter %s not recognized.\n", argv[i]);
            errorExit("ERROR");
        }
    }
    if((scp_file == NULL) || ((scp_file != NULL) && (scp_file[0] == '\0'))) {
        errorExit("ERROR: --instance must be provided.\n");
    }
}

/*** Read instance in the OR-LIBRARY format ***/
void readSCP(char* filename) {
    int h,i,j;
    int* k;
    FILE* fp = fopen(filename, "r");
    
    if (!fp)
        errorExit("ERROR: Error in opening the file.\n");
    if (fscanf(fp,"%d",&m) != 1) /* number of rows */
        errorExit("ERROR: Error reading number of rows.\n");
    if (fscanf(fp,"%d",&n) != 1) /* number of columns */
        errorExit("ERROR: Error reading number of columns.\n");
    /* Cost of the n columns */
    cost = (int*) mymalloc(n * sizeof(int));
    for (j = 0; j < n; j++)
        if (fscanf(fp,"%d",&cost[j]) != 1)
            errorExit("ERROR: Error reading cost.\n");
    
    /* Info of columns that cover each row */
    col  = (int**) mymalloc(m * sizeof(int*));
    ncol = (int*) mymalloc(m * sizeof(int));
    for (i = 0; i < m; i++) {
        if (fscanf(fp,"%d",&ncol[i]) != 1)
            errorExit("ERROR: Error reading number of columns.\n");
        col[i] = (int *) mymalloc(ncol[i] * sizeof(int));
        for (h = 0; h < ncol[i]; h++) {
            if(fscanf(fp,"%d",&col[i][h]) != 1)
                errorExit("ERROR: Error reading columns.\n");
            col[i][h]--;
        }
    }
    /* Info of rows that are covered by each column */
    row  = (int**) mymalloc(n*sizeof(int*));
    nrow = (int*) mymalloc(n*sizeof(int));
    k    = (int*) mymalloc(n*sizeof(int));
    for (j = 0; j < n; j++) nrow[j] = 0;
    for (i = 0; i < m; i++) {
        for (h = 0; h < ncol[i]; h++)
            nrow[col[i][h]]++;
    }
    for (j = 0; j < n; j++) {
        row[j] = (int *) mymalloc(nrow[j]*sizeof(int));
        k[j] = 0;
    }
    for (i = 0; i < m; i++) {
        for (h = 0; h < ncol[i]; h++) {
            row[col[i][h]][k[col[i][h]]] = i;
            k[col[i][h]]++;
        }
    }
    free((void*) k);
}

/*** Use level>=1 to print more info */
void printInstance(int level) {
    int i;
    printf("**********************************************\n");
    printf("  SCP INSTANCE: %s\n", scp_file);
    printf("  PROBLEM SIZE\t m = %d\t n = %d\n", m, n);
    if(level >= 1) {
        printf("  COLUMN COST:\n");
        for(i = 0; i < n; i++)
            printf("%d ",cost[i]);
        printf("\n\n");
        printf("  NUMBER OF ELEMENTS (ROWS) COVERED BY SUBSET (COLUMN) 1 is %d\n", nrow[0] );
        for(i = 0; i < nrow[0]; i++)
            printf("%d ", row[0][i]);
        printf("\n");
        printf("  NUMBER OF SUBSETS (COLUMNS) COVERING ELEMENT (ROW) 1 is %d\n", ncol[0] );
        for(i = 0; i < ncol[0]; i++)
            printf("%d ", col[0][i]);
        printf("\n");
    }
    printf("**********************************************\n\n");
}

/*** Use this function to initialize other variables of the algorithms **/
void initialize() {
    un_rows = m;
    un_cols = n;
    x = (int *) mymalloc(n * sizeof(int));
    y = (int *) mymalloc(m * sizeof(int));
    col_cover = (int **) mymalloc(m * sizeof(int *));
    ncol_cover = (int *) mymalloc(m * sizeof(int));
    ccost = (float *) mymalloc(n * sizeof(float));
    for (int i = 0; i < n; i++) {
        x[i] = 0;
        ccost[i] = (float) cost[i] / (float) nrow[i];
    }
    for (int i = 0; i < m; i++) {
        y[i] = 0;
        ncol_cover[i] = 0;
        int k = ncol[i];
        col_cover[i] = (int *) mymalloc(k * sizeof(int));
        for (int j = 0; j < k; j++) {
            col_cover[i][j] = -1;
        }
    }
}

void* mymalloc(size_t size) {
    void *s;
    if ((s=malloc(size)) == NULL) {
        fprintf(stderr, "malloc : Not enough memory.\n");
        exit(EXIT_FAILURE);
    }
    return s;
}

void errorExit(char* text) {
    printf("%s\n", text);
    exit(EXIT_FAILURE);
}

/*** Use this function to finalize execution */
void finalize() {
    free((void**) row);
    free((void**) col);
    free((void*) nrow);
    free((void*) ncol);
    free((void*) cost);
    free((void*) ccost);
    free((void*) x);
    free((void*) y);
    free((void**) col_cover);
    free((void*) ncol_cover);
}

/********************************
 Functions used by all algorithms
 ********************************/

/*** When adding a set with colidx to the partial solution:
 1. Indicate that column is selected (x)
 2. Add cost of column to partial cost (fx)
 3. Decrement # of un-used columns (un_cols)
 4. Indicate that each row covered by column is selected (y)
 5. Increment # of columns covering each row (ncol_cover)
 6. Decrement # of un-covered rows (un_rows)
 7. Add column to all columns covering row i (col_cover) */
void addSet(int colidx) {
    x[colidx] = 1;
    fx += cost[colidx];
    un_cols -= 1;
    for (int i = 0; i < nrow[colidx]; i++) {
        int rowidx = row[colidx][i];
        ncol_cover[rowidx] += 1;
        if (!y[rowidx]) {
            y[rowidx] = 1;
            un_rows -= 1;
        }
        for (int j = 0; j < ncol[rowidx]; j++) {
            if (col_cover[rowidx][j] < 0) {
                col_cover[rowidx][j] = colidx;
                break;
            }
        }
    }
}

/*** Check if set colidx is redundant.
 Assume set is redundant. When one element is found
 that is not yet covered, the set is not redundant. */
int redundant(int colidx) {
    int redundantBool = 1;
    for (int i = 0; i < nrow[colidx]; i++) {
        if (!y[row[colidx][i]]) {
            redundantBool = 0;
            break;
        }
    }
    return redundantBool;
}

/***
 When an element is removed from col_cover for row rowidx,
 then the remaining elements need to be shifted, so there
 are no zeros in between.
*/
void shift(int rowidx, int start) {
    for (int i = start; i < ncol_cover[rowidx]; i++) {
        if (i+1 < ncol_cover[rowidx]) {
            if (col_cover[rowidx][i+1] >= 0) {
                col_cover[rowidx][i] = col_cover[rowidx][i+1];
            } else {
                col_cover[rowidx][i] = -1;
                break;
            }
        } else {
            col_cover[rowidx][i] = -1;
        }
    }
}

/*** When removing a set from the partial solution
 1. Indicate that the column is no longer selected (x)
 2. Remove cost of column from partial cost (fx)
 3. Increment # of un-used columns (un_cols)
 4. Decrement # of columns covering each row (ncol_cover)
 5. Remove column from all columns covering row i (col_cover)
*/
void removeSet(int colidx) {
    x[colidx] = 0;
    fx -= cost[colidx];
    un_cols += 1;
    for (int i = 0; i < nrow[colidx]; i++) {
        int rowidx = row[colidx][i];
        for (int j = 0; j < ncol_cover[rowidx]; j++) {
            if (col_cover[rowidx][j] == colidx) {
                col_cover[rowidx][j] = -1;
                shift(rowidx, j);
                break;
            }
        }
        ncol_cover[rowidx] -= 1;
    }
}

int isBetter(int newCol, float newCost, int currCol, float currCost) {
    int result = 0;
    if (!currCost || newCost < currCost) {
        result = 1;
    } else if (newCost == currCost) {
        if (nrow[newCol] > nrow[currCol]) {
            result = 1;
        } else if (nrow[newCol] == nrow[currCol]) {
            if (pickRandom(2)) {
                result = 1;
            }
        }
    }
    return result;
}

/***
 Continue looping until you can no longer find a redundant set.
 Consider only a set when it is selected.
 A set is redundant until proven otherwise.
 Double for-loop = look for each element at all sets covering that element
 If set I covers element J and it is the only one
 Then set I is no longer redundant
 If you managed to go through all elements J and
 the set I is still redundant; store its idx and cost
 but only if the cost of this redundant set is
 higher than the already found redundant set (if present)
 If a redundant set with highest cost is found
 remove it and repeat the process
*/
void eliminate() {
    int removed = 1;
    int currCol = -1;
    float currCost = 0.0;
    int redundantBool = 1;
    while (removed) {
        removed = 0;
        currCol = -1;
        currCost = 0;
        for (int i = 0; i < n; i++) {
            if (x[i]) {
                redundantBool = 1;
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < ncol_cover[j]; k++) {
                        if (col_cover[j][k] == i && ncol_cover[j] <= 1) {
                            redundantBool = 0;
                            break;
                        }
                    }
                    if (!redundantBool) {
                        break;
                    }
                }
                if (redundantBool) {
                    float newCost = getCost(i);
                    if (isBetter(i, newCost, currCol, currCost)) {
                        currCol = i;
                        currCost = newCost;
                    }
                }
            }
        }
        if (currCol >= 0) {
            removeSet(currCol);
            removed = 1;
        }
    }
}

/*** Check if the current solution is a solution.
 i.e. when no rows are un-covered. */
int isSolution() {
    return (un_rows <= 0);
}

/*** Prints diagnostic information about the solution */
void diagnostics() {
    for (int i = 0; i < m; i++) {
        if (y[i]) {
            printf("ELEMENT %d COVERED BY %d SET(S)\n", i, ncol_cover[i]);
            for (int j = 0; j < ncol_cover[i]; j++) {
                if (col_cover[i][j] < 0) {
                    printf("---\n");
                    break;
                } else {
                    printf("---SET %d\n", col_cover[i][j]);
                }
            }
        } else {
            printf("ELEMENT %d NOT COVERED\n", i);
        }
    }
}

/*************************************
 Functions used by Random Construction
 *************************************/

/***
 Random Construction:
  1. Pick an un-covered element
  2. Choose a random set that covers this element */
float randomFloat() {
    float r = (float) rand() / (float) RAND_MAX;
    return r;
}

/*** To pick an un-covered element, make intervals of size 1/#elements,
 generate a random number in [0,1] and search for the n'th interval
 in which this random number lies. */
int pickRandom(int setSize) {
    float r = randomFloat();
    float step = (float) 1 / (float) setSize;
    int n;
    for (int i = 0; i < setSize; i++) {
        if (r <= (i+1)*step) {
            n = i;
            break;
        }
    }
    return n;
}

/*** Select an element (using pickRandom)
    Check if element is un-covered (using y)
    If false, retry
    If true, select a set that covers this element (using pickRandom)
       Check if set us un-used (using x)
       If false, retry
       If true, check if this set is redundant
           If true, retry
           If false, add set to partial solution
 End */
void randomConstruction() {
    int rowidx = -1, colidx = -1;
    while (rowidx < 0) {
        int idx = pickRandom(m);
        if (!y[idx]) {
            rowidx = idx;
        }
    }
    while (colidx < 0) {
        int idx = pickRandom(ncol[rowidx]);
        if (!x[col[rowidx][idx]]) {
            colidx = col[rowidx][idx];
        }
    }
    addSet(colidx);
}

/************************************
 Functions used by Cost Constructions
 ************************************/

float adaptiveCost(int colidx) {
    int covers = 0;
    for (int i = 0; i < nrow[colidx]; i++) {
        if (!y[row[colidx][i]]) {
            covers += 1;
        }
    }
    return (float) cost[colidx] / (float) covers;
}

float getCost(int colidx) {
    float c;
    if (ch1 || ch2) {
        c = cost[colidx];
    } else if (ch3) {
        c = ccost[colidx];
    } else if (ch4) {
        c = adaptiveCost(colidx);
    }
    return c;
}

/* 
 Static cost greedy: select subset with lowest cost and add elements.
 If 2 subsets have the same cost, select the subset with the most elements in it.
 If they have the same number of elements, take one at random. */
void costBased() {
    int currCol = -1;
    float currCost = 0.0;
    for (int i = 0; i < n; i++) {
        if (!x[i] && !redundant(i)) {
            float c = getCost(i);
            if (isBetter(i, c, currCol, currCost)) {
                currCol = i;
                currCost = c;
            }
        }
    }
    addSet(currCol);
}

/********************
 Iterative algorithms 
 *******************/
// A neighborhood = remove one subset from the solution
// and complete it again with the Adaptive Cover Cost based
// greedy values of the available subsets. Ofcourse, don't
// replace a subset with itself. Apply redundancy elimination
// when complete.
void bestImprovement() {}
void firstImprovement() {}

/*** Dispatcher */
void solve() {
    while (!isSolution()) {
        if (ch1) {
            randomConstruction();
        } else if (ch2 || ch3 || ch4) {
            costBased();
        } else {
            printf("ERROR: No constructive algorithm selected.\n");
        }
    }
    if (bi) {
        bestImprovement();
    }
    if (fi) {
        firstImprovement();
    }
    if (re) {
        eliminate();
    }
}

/*** Main loop */
int main(int argc, char* argv[]) {
    readParameters(argc, argv);
    readSCP(scp_file);
    initialize();
    //printInstance(0);
    srand(seed);
    solve();
    //diagnostics();
    finalize();
    printf("%d", fx);
    return 0;
}

