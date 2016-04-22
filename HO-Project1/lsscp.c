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

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "constructive.h"
#include "iterative.h"
#include "lsscp.h"

/********************
 Algorithm parameters
 *******************/
int seed = 1234567;
char* instance_file = "";
char* output_file = "output.txt";

instance_t* inst;
solution_t* soln;
clock_t start_time;

int ch1 = 0, ch2 = 0, ch3 = 0, ch4 = 0, bi = 0, fi = 0, re = 0;

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


/*** Read parameters from command line **/
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
            instance_file = argv[i+1];
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
    if((instance_file == NULL) || ((instance_file != NULL) && (instance_file[0] == '\0'))) {
        errorExit("ERROR: --instance must be provided.\n");
    }
}

/*** Read instance in the OR-LIBRARY format **/
void readSCP(char* filename) {
    int h,i,j;
    int* k;
    FILE* fp = fopen(filename, "r");
    inst = (instance_t*) mymalloc(sizeof(instance_t));
    
    if (!fp)
        errorExit("ERROR: Error in opening the file.\n");
    if (fscanf(fp,"%d",&inst->m) != 1) /* number of rows */
        errorExit("ERROR: Error reading number of rows.\n");
    if (fscanf(fp,"%d",&inst->n) != 1) /* number of columns */
        errorExit("ERROR: Error reading number of columns.\n");
    // Cost of the n columns
    inst->cost = (int*) mymalloc(inst->n * sizeof(int));
    for (j = 0; j < inst->n; j++)
        if (fscanf(fp,"%d",&inst->cost[j]) != 1)
            errorExit("ERROR: Error reading cost.\n");
    
    // Info of columns that cover each row
    inst->col  = (int**) mymalloc(inst->m * sizeof(int*));
    inst->ncol = (int*) mymalloc(inst->m * sizeof(int));
    for (i = 0; i < inst->m; i++) {
        if (fscanf(fp,"%d",&inst->ncol[i]) != 1)
            errorExit("ERROR: Error reading number of columns.\n");
        inst->col[i] = (int *) mymalloc(inst->ncol[i] * sizeof(int));
        for (h = 0; h < inst->ncol[i]; h++) {
            if(fscanf(fp,"%d",&inst->col[i][h]) != 1)
                errorExit("ERROR: Error reading columns.\n");
            inst->col[i][h]--;
        }
    }
    // Info of rows that are covered by each column
    inst->row  = (int**) mymalloc(inst->n * sizeof(int*));
    inst->nrow = (int*) mymalloc(inst->n * sizeof(int));
    k    = (int*) mymalloc(inst->n * sizeof(int));
    for (j = 0; j < inst->n; j++) inst->nrow[j] = 0;
    for (i = 0; i < inst->m; i++) {
        for (h = 0; h < inst->ncol[i]; h++)
            inst->nrow[inst->col[i][h]]++;
    }
    for (j = 0; j < inst->n; j++) {
        inst->row[j] = (int *) mymalloc(inst->nrow[j] * sizeof(int));
        k[j] = 0;
    }
    for (i = 0; i < inst->m; i++) {
        for (h = 0; h < inst->ncol[i]; h++) {
            inst->row[inst->col[i][h]][k[inst->col[i][h]]] = i;
            k[inst->col[i][h]]++;
        }
    }
    free((void*) k);
}

/*** Use this function to initialize other variables of the algorithms **/
void initialize() {
    soln = mymalloc(sizeof(solution_t));
    soln->fx = 0;
    soln->un_rows = inst->m;
    soln->un_cols = inst->n;
    soln->x = (int *) mymalloc(inst->n * sizeof(int));
    soln->y = (int *) mymalloc(inst->m * sizeof(int));
    soln->col_cover = (int **) mymalloc(inst->m * sizeof(int *));
    soln->ncol_cover = (int *) mymalloc(inst->m * sizeof(int));
    inst->ccost = (float *) mymalloc(inst->n * sizeof(float));
    for (int i = 0; i < inst->n; i++) {
        soln->x[i] = 0;
        inst->ccost[i] = (float) inst->cost[i] / (float) inst->nrow[i];
    }
    for (int i = 0; i < inst->m; i++) {
        soln->y[i] = 0;
        soln->ncol_cover[i] = 0;
        int k = inst->ncol[i];
        soln->col_cover[i] = (int *) mymalloc(k * sizeof(int));
        for (int j = 0; j < k; j++) {
            soln->col_cover[i][j] = -1;
        }
    }
}

void errorExit(char* text) {
    printf("%s\n", text);
    exit(EXIT_FAILURE);
}

/*** Use this function to finalize execution **/
void finalize() {
    for (int i = 0; i < inst->n; i++) free(inst->row[i]);
    for (int i = 0; i < inst->m; i++) free(inst->col[i]);
    free(inst->row);
    free(inst->col);
    free(inst->ccost);
    free(inst->cost);
    free(inst->ncol);
    free(inst->nrow);
    free(inst);
    
    //for (int i = 0; i < inst->m; i++) free(soln->col_cover[i]);
    free(soln->col_cover);
    free(soln->ncol_cover);
    free(soln->x);
    free(soln->y);
    free(soln);
}

/*** Prints diagnostic information about the solution **/
void diagnostics() {
    for (int i = 0; i < inst->m; i++) {
        if (soln->y[i]) {
            printf("ELEMENT %d COVERED BY %d SET(S)\n", i, soln->ncol_cover[i]);
            for (int j = 0; j < soln->ncol_cover[i]; j++) {
                if (soln->col_cover[i][j] < 0) {
                    printf("---\n");
                    break;
                } else {
                    printf("---SET %d\n", soln->col_cover[i][j]);
                }
            }
        } else {
            printf("ELEMENT %d NOT COVERED\n", i);
        }
    }
}

/***
 Sort all sets based on their cost.
 Loop through all sets:
 Only consider a set when it is in the partial solution
 Consider a set to be redundant, until proven otherwise
 A set is not redundant when it is the only set covering
 a particular element.
 When going through all elements and the set is still redundant,
 then remove it.
 Repeat this process until no more sets can be removed.
 */
int costSort(const void* a, const void* b) {
    return (inst->cost[*(int *) b] - inst->cost[*(int *) a]);
}

void eliminate(instance_t* inst, solution_t* sol) {
    unsigned int redundantBool = 1;
    unsigned int improvement = 1;
    
    int* cols = mymalloc(inst->n * sizeof(int));
    for (int i = 0; i < inst->n; i++) {
        cols[i] = i;
    }
    qsort(cols, inst->n, sizeof(int), costSort);
    
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            int currCol = cols[i];
            if (sol->x[currCol]) {
                redundantBool = 1;
                for (int j = 0; j < inst->nrow[currCol]; j++) {
                    int elem = inst->row[currCol][j];
                    if (sol->ncol_cover[elem] <= 1) {
                        redundantBool = 0;
                        break;
                    }
                }
                if (redundantBool) {
                    removeSet(inst, sol, currCol);
                    improvement = 1;
                }
            }
        }
    }
    free(cols);
}

/*** Dispatcher */
void solve() {
    while (!isSolution(soln)) {
        if (ch1) {
            randomConstruction(inst, soln);
        } else if (ch2 || ch3 || ch4) {
            costBased(inst, soln);
        }
    }
    if (fi) {
        firstImprove(inst, soln);
    }
    if (bi) {
        bestImprove(inst, soln);
    }
    if (re) {
        eliminate(inst, soln);
    }
}

/*** Main **/
int main(int argc, char* argv[]) {
    start_time = clock();
    readParameters(argc, argv);
    readSCP(instance_file);
    initialize();
    srand(seed);
    solve();
    int optcost = soln->fx;
    finalize();
    double duration = (double) (clock() - start_time)/CLOCKS_PER_SEC;
    printf("%d, %f\n", optcost, 100*duration);
    return 0;
}

