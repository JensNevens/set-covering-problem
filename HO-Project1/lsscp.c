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

#include "utils.h"

/** Algorithm parameters **/
int seed = 6666666;
char *scp_file = "";
char *output_file = "output.txt";

/** Variables to activate algorithms **/
int ch1 = 0, ch2 = 0, ch3 = 0, ch4 = 0, bi = 0, fi = 0, re = 0;

/** Instance static variables **/
int m;            /* number of rows */
int n;            /* number of columns */
int **row;        /* row[i] contains rows that are covered by column i */
int **col;        /* col[i] contains columns that cover row i */
int *ncol;        /* ncol[i] contains number of columns that cover row i */
int *nrow;        /* nrow[i] contains number of rows that are covered by column i */
int *cost;        /* cost[i] contains cost of column i  */

/** Solution variables **/
int *x;           /* x[i] 0,1 if column i is selected */
int *y;           /* y[i] 0,1 if row i covered by the actual solution */
int fx;           /* sum of the cost of the columns selected in the solution (can be partial) */

/** Dynamic variables **/
/** Note: use dynamic variables to make easier the construction and modification of solutions. **/
/** these are just examples of useful variables. **/
/** These variables need to be updated eveytime a column is added to a partial solution **/
/** or when a complete solution is modified */
int **col_cover;  /* col_colver[i] contains selected columns that cover row i */
int *ncol_cover;  /* ncol_cover[i] contains number of selected columns that cover row i */
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


/*Read parameters from command line*/
void read_parameters(int argc, char *argv[]) {
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
                printf("ERROR: Only use 1 constructive algorithm.\n");
                exit(EXIT_FAILURE);
            }
            ch1 = 1;
        } else if (strcmp(argv[i], "--ch2") == 0) {
            if (ch1 || ch3 || ch4) {
                printf("ERROR: Only use 1 constructive algorithm.\n");
                exit(EXIT_FAILURE);
            }
            ch2 = 1;
        } else if (strcmp(argv[i], "--ch3") == 0) {
            if (ch1 || ch2 || ch4) {
                printf("ERROR: Only use 1 constructive algorithm.\n");
                exit(EXIT_FAILURE);
            }
            ch3 = 1;
        } else if (strcmp(argv[i], "--ch4") == 0) {
            if (ch1 || ch2 || ch3) {
                printf("ERROR: Only use 1 constructive algorithm.\n");
                exit(EXIT_FAILURE);
            }
            ch4 = 1;
        } else if (strcmp(argv[i], "--bi") == 0) {
            if (fi) {
                printf("ERROR: Only use 1 iterative algorithm.");
                exit(EXIT_FAILURE);
            }
            bi = 1;
        } else if (strcmp(argv[i], "--fi") == 0) {
            if (bi) {
                printf("ERROR: Only use 1 iterative algorithm.\n");
                exit(EXIT_FAILURE);
            }
            fi = 1;
        } else if (strcmp(argv[i], "--re") == 0) {
            re = 1;
        } else {
            printf("ERROR: parameter %s not recognized.\n", argv[i]);
            usage();
            exit(EXIT_FAILURE);
        }
    }
    if((scp_file == NULL) || ((scp_file != NULL) && (scp_file[0] == '\0'))) {
        printf("ERROR: --instance must be provided.\n");
        usage();
        exit(EXIT_FAILURE);
    }
}

/*** Read instance in the OR-LIBRARY format ***/
void read_scp(char *filename) {
    int h,i,j;
    int *k;
    FILE *fp = fopen(filename, "r");
    
    if (!fp)
        error_reading_file("ERROR: Error in opening the file.\n");
    if (fscanf(fp,"%d",&m) != 1) /* number of rows */
        error_reading_file("ERROR: Error reading number of rows.\n");
    if (fscanf(fp,"%d",&n) != 1) /* number of columns */
        error_reading_file("ERROR: Error reading number of columns.\n");
    /* Cost of the n columns */
    cost = (int *) mymalloc(n * sizeof(int));
    for (j = 0; j < n; j++)
        if (fscanf(fp,"%d",&cost[j]) != 1)
            error_reading_file("ERROR: Error reading cost.\n");
    
    /* Info of columns that cover each row */
    col  = (int **) mymalloc(m * sizeof(int *));
    ncol = (int *) mymalloc(m * sizeof(int));
    for (i = 0; i < m; i++) {
        if (fscanf(fp,"%d",&ncol[i]) != 1)
            error_reading_file("ERROR: Error reading number of columns.\n");
        col[i] = (int *) mymalloc(ncol[i] * sizeof(int));
        for (h = 0; h < ncol[i]; h++) {
            if(fscanf(fp,"%d",&col[i][h]) != 1)
                error_reading_file("ERROR: Error reading columns.\n");
            col[i][h]--;
        }
    }
    /* Info of rows that are covered by each column */
    row  = (int **) mymalloc(n*sizeof(int *));
    nrow = (int *) mymalloc(n*sizeof(int));
    k    = (int *) mymalloc(n*sizeof(int));
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
    free((void *)k);
}

/*** Use level>=1 to print more info */
void print_instance(int level) {
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
    for (int i = 0; i < n; i++) {
        x[i] = 0;
    }
    for (int i = 0; i < m; i++) {
        y[i] = 0;
        ncol_cover[i] = 0;
        int k = ncol[i];
        col_cover[i] = (int *) mymalloc(k * sizeof(int));
        for (int j = 0; j < k; j++) {
            col_cover[i][j] = 0;
        }
    }
}

/*** Use this function to finalize execution */
void finalize() {
    free((void **) row );
    free((void **) col );
    free((void *) nrow );
    free((void *) ncol );
    free((void *) cost );
    free((void *) x);
    free((void *) y);
    free((void **) col_cover);
    free((void *) ncol_cover);
}

void addSet(int colidx) {
    // TODO: Set colidx is added to partial solution.
    // Update: x, y, fx, col_cover, ncol_cover, un_rows, un_cols
}

int redundant(int colidx) {
    // TODO: Check if set colidx is redundant.
    return 1;
}

/*** Constructive algorithms */
/*** Random Construction: */
/***    1. Pick an un-covered element */
/***    2. Choose a random set that covers this element */
float randomFloat() {
    float r = (float) rand() / (float) RAND_MAX;
    return r;
}

/*** To pick an un-covered element, make intervals of size 1/#elements, */
/*** generate a random number in [0,1] and search for the n'th interval */
/*** in which this random number lies. */
int pickRandom(int setSize) {
    float r = randomFloat();
    float step = (float) 1 / (float) setSize;
    int set;
    for (int i = 0; i < setSize; i++) {
        if (r > i*step && r <= (i+1)*step) {
            set = i;
            break;
        }
    }
    return set;
}

void randomConstruction() {
    // Select an element (using pickRandom)
    // Check if element is un-covered (using y)
    // If false, retry
    int found = 0;
    int rowidx = 0, colidx = 0;
    
    while (!found) {
        int idx = pickRandom(un_rows);
        if (!y[idx]) {
            rowidx = idx;
            found = 1;
        }
    }
    // If true, select a set that covers this element (using pickRandom)
    // Check if set us un-used (using x)
    // If false, retry
    found = 0;
    while (!found) {
        int idx = pickRandom(ncol[rowidx]);
        if (!x[col[rowidx][idx]]) {
            colidx = col[rowidx][idx];
            found = 1;
        }
    }
    // TODO: If true, add this set to partial solution
    // Keep track of all redudant sets?? Maybe too difficult..
    if (!redundant(colidx)) {
        addSet(colidx);
    }
}

void staticCost() {
    printf("Static");
}

void staticCoverCost() {
    printf("Static Cover");
}

void adaptiveCoverCost() {
    printf("Adaptive Cover");
}

/*** Dispatcher for constructive algorithms */
void constructive() {
    if (ch1) {
        randomConstruction();
    } else if (ch2) {
        staticCost();
    } else if (ch3) {
        staticCoverCost();
    } else if (ch4) {
        adaptiveCoverCost();
    } else {
        printf("ERROR: No constructive algorithm selected.\n");
    }
}

/** Iterative algorithms */
void bestImprovement() {}
void firstImprovement() {}

/*** Dispatcher for iterative algorithms */
void iterative() {
    if (bi) {
        bestImprovement();
    } else if (fi) {
        firstImprovement();
    }
}

/*** Main loop */
void main_loop(int argc, char *argv[]) {
    read_parameters(argc, argv);
    read_scp(scp_file);
    print_instance(0);
    initialize();
    srand(seed);
    finalize();
}

