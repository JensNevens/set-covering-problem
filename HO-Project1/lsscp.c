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

#include "lsscp.h"

/********************
 Algorithm parameters
 *******************/
int seed = 1234567;
char* instance_file = "";
char* output_file = "output.txt";

solution_t* cpy;
best_t* best;
instance_t* inst;
solution_t* soln;

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
    free((void*) inst);
    free((void*) soln);
    free((void*) cpy);
    free((void*) best);
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

void* mymalloc(size_t size) {
    void *s;
    if ((s=malloc(size)) == NULL) {
        fprintf(stderr, "malloc : Not enough memory.\n");
        exit(EXIT_FAILURE);
    }
    return s;
}

/********************************
 Functions used by all algorithms
 ********************************/

/***
 When adding a set with colidx to the partial solution:
 1. Indicate that column is selected (x)
 2. Add cost of column to partial cost (fx)
 3. Decrement # of un-used columns (un_cols)
 4. Indicate that each row covered by column is selected (y)
 5. Increment # of columns covering each row (ncol_cover)
 6. Decrement # of un-covered rows (un_rows)
 7. Add column to all columns covering row i (col_cover)
 **/
void addSet(instance_t* inst, solution_t* sol, int colidx) {
    sol->x[colidx] = 1;
    sol->fx += inst->cost[colidx];
    sol->un_cols -= 1;
    for (int i = 0; i < inst->nrow[colidx]; i++) {
        int rowidx = inst->row[colidx][i];
        sol->ncol_cover[rowidx] += 1;
        if (!sol->y[rowidx]) {
            sol->y[rowidx] = 1;
            sol->un_rows -= 1;
        }
        for (int j = 0; j < inst->ncol[rowidx]; j++) {
            if (sol->col_cover[rowidx][j] < 0) {
                sol->col_cover[rowidx][j] = colidx;
                break;
            }
        }
    }
}

/***
 Check if set colidx is redundant.
 Assume set is redundant. When one element is found
 that is not yet covered, the set is not redundant.
 **/
int redundant(instance_t* inst, solution_t* sol, int colidx) {
    unsigned int redundantBool = 1;
    for (int i = 0; i < inst->nrow[colidx]; i++) {
        if (!sol->y[inst->row[colidx][i]]) {
            redundantBool = 0;
            break;
        }
    }
    return redundantBool;
}

/***
 When an element is removed from col_cover for row rowidx,
 then the remaining elements need to be shifted, so there
 are no dummy columns (-1) in between.
 **/
void shift(solution_t* sol, int rowidx, int start) {
    for (int i = start; i < sol->ncol_cover[rowidx]; i++) {
        if (i+1 < sol->ncol_cover[rowidx]) {
            if (sol->col_cover[rowidx][i+1] >= 0) {
                sol->col_cover[rowidx][i] = sol->col_cover[rowidx][i+1];
            } else {
                sol->col_cover[rowidx][i] = -1;
                break;
            }
        } else {
            sol->col_cover[rowidx][i] = -1;
        }
    }
}

/***
 When removing a set from the partial solution
 1. Indicate that the column is no longer selected (x)
 2. Remove cost of column from partial cost (fx)
 3. Increment # of un-used columns (un_cols)
 4. Decrement # of columns covering each row (ncol_cover)
 5. Remove column from all columns covering row i (col_cover)
 6. If a row is now uncovered, increment un_rows
 7. If a row is now uncovered, indicate it (y)
 **/
void removeSet(instance_t* inst, solution_t* sol, int colidx) {
    sol->x[colidx] = 0;
    sol->fx -= inst->cost[colidx];
    sol->un_cols += 1;
    for (int i = 0; i < inst->nrow[colidx]; i++) {
        int rowidx = inst->row[colidx][i];
        for (int j = 0; j < sol->ncol_cover[rowidx]; j++) {
            if (sol->col_cover[rowidx][j] == colidx) {
                sol->col_cover[rowidx][j] = -1;
                shift(sol, rowidx, j);
                break;
            }
        }
        sol->ncol_cover[rowidx] -= 1;
        if (sol->ncol_cover[rowidx] <= 0) {
            sol->un_rows += 1;
            sol->y[rowidx] = 0;
        }
    }
}

/***
 Check if column newCol with cost newCost is better than
 column currCol with cost currCost. They are better when
 the cost is less. When the cost is equal, we look at
 how many rows these columns cover. When this is also
 equal, a column is chosen at random.
 **/
int isBetter(instance_t* inst, int newCol, float newCost, int currCol, float currCost) {
    unsigned int result = 0;
    if (!currCost || newCost < currCost) {
        result = 1;
    } else if (newCost == currCost) {
        if (inst->nrow[newCol] > inst->nrow[currCol]) {
            result = 1;
        } else if (inst->nrow[newCol] == inst->nrow[currCol]) {
            result = 1;
        }
    }
    return result;
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
    
    int* cols = (int*) mymalloc(inst->n * sizeof(int));
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
}

/***
 Check if the current solution is a solution.
 i.e. when no rows are un-covered.
 **/
int isSolution(solution_t* sol) {
    return (sol->un_rows <= 0);
}

/*************************************
 Functions used by Random Construction
 *************************************/

unsigned int pickRandom(unsigned int min, unsigned int max) {
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;
    
    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do {
        r = rand();
    } while (r >= limit);
    
    return min + (r / buckets);
}

/***
 Pick a random element that is not yet covered
 Pick a random column that covers this element
 and is also not yet covered. No need for a
 redundancy check, since it does not cover
 element rowidx for sure. Add the set to the
 solution.
 **/
void randomConstruction(instance_t* inst, solution_t* sol) {
    int rowidx = -1, colidx = -1;
    while (rowidx < 0) {
        int idx = pickRandom(0, inst->m-1);
        if (!sol->y[idx]) {
            rowidx = idx;
        }
    }
    while (colidx < 0) {
        int idx = pickRandom(0, inst->ncol[rowidx]-1);
        if (!sol->x[inst->col[rowidx][idx]]) {
            colidx = inst->col[rowidx][idx];
        }
    }
    addSet(inst, sol, colidx);
}

/************************************
 Functions used by Cost Constructions
 ************************************/

/***
 Compute the adaptive cover cost based heuristic
 by counting how many additional elements this
 set would cover. Divide the cost by this count.
 **/
float adaptiveCost(instance_t* inst, solution_t* sol, int colidx) {
    unsigned int covers = 0;
    for (int i = 0; i < inst->nrow[colidx]; i++) {
        if (!sol->y[inst->row[colidx][i]]) {
            covers += 1;
        }
    }
    return (float) inst->cost[colidx] / (float) covers;
}

/***
 General method for getting the cost.
 Computes the cost depending on the algorithm.
 **/
float getCost(instance_t* inst, solution_t* sol, int colidx) {
    float c;
    if (ch2) {
        c = inst->cost[colidx];
    } else if (ch3) {
        c = inst->ccost[colidx];
    } else if (ch4) {
        c = adaptiveCost(inst, sol, colidx);
    }
    return c;
}

/*
 For each set, if this set is not yet used and
 it is not redundant, then get the cost of this
 set. If the cost is less then the current best
 set (currCol), then replace it. When going over
 all sets, currCol contains the cheapest set.
 Add this one to the solution.
 **/
void costBased(instance_t* inst, solution_t* sol) {
    int currCol = -1;
    float currCost = 0.0;
    for (int i = 0; i < inst->n; i++) {
        if (!sol->x[i] && !redundant(inst, sol, i)) {
            float c = getCost(inst, sol, i);
            if (isBetter(inst, i, c, currCol, currCost)) {
                currCol = i;
                currCost = c;
            }
        }
    }
    if (currCol >= 0) {
        addSet(inst, sol, currCol);
    }
}

/********************
 Iterative algorithms
 *******************/

/***
 Initialize the copy of the solution struct
 **/
void initCopy(instance_t* inst, solution_t* sol, solution_t* new) {
    new->fx = sol->fx;
    new->un_rows = sol->un_rows;
    new->un_cols = sol->un_cols;
    new->x = (int*) mymalloc(inst->n * sizeof(int));
    new->y = (int*) mymalloc(inst->m * sizeof(int));
    new->col_cover = (int**) mymalloc(inst->m * sizeof(int*));
    new->ncol_cover = (int*) mymalloc(inst->m * sizeof(int));
    for (int i = 0; i < inst->n; i++) {
        new->x[i] = sol->x[i];
    }
    for (int i = 0; i < inst->m; i++) {
        new->y[i] = sol->y[i];
        new->ncol_cover[i] = sol->ncol_cover[i];
        int k = inst->ncol[i];
        new->col_cover[i] = (int*) mymalloc(k * sizeof(int));
        for (int j = 0; j < k; j++) {
            new->col_cover[i][j] = sol->col_cover[i][j];
        }
    }
}

/***
 Copy a solution struct from struct 'from' to struct 'to'.
 **/
void copySolution(instance_t* inst, solution_t* from, solution_t* to) {
    to->fx = from->fx;
    to->un_rows = from->un_rows;
    to->un_cols = from->un_cols;
    for (int i = 0; i < inst->n; i++) {
        to->x[i] = from->x[i];
    }
    for (int i = 0; i < inst->m; i++) {
        to->y[i] = from->y[i];
        to->ncol_cover[i] = from->ncol_cover[i];
        for (int j = 0; j < inst->ncol[i]; j++) {
            to->col_cover[i][j] = from->col_cover[i][j];
        }
    }
}

/***
 Free a solution struct
 **/
void freeSolution(solution_t* sol) {
    free((void*) sol->x);
    free((void*) sol->y);
    free((void**) sol->col_cover);
    free((void*) sol->ncol_cover);
}

/***
 Add the best possible set to the solution as long as
 1. it is not colidx, the set that was just removed
 2. it is not already used
 3. it is not redundant
 The best set is determined by the adaptive cover
 cost-based greedy heuristic.
 **/
int replaceSet(instance_t* inst, int colidx) {
    int currCol = -1;
    float currCost = 0.0;
    for (int i = 0; i < inst->n; i++) {
        if (i != colidx && !cpy->x[i] && !redundant(inst, cpy, i)) {
            float c = adaptiveCost(inst, cpy, i);
            if (isBetter(inst, i, c, currCol, currCost)) {
                currCol = i;
                currCost = c;
            }
        }
    }
    if (currCol >= 0) {
        addSet(inst, cpy, currCol);
    }
    return currCol;
}

void initBest(instance_t* inst, solution_t* sol, best_t* best) {
    best->removed = -1;
    best->added = (int*) mymalloc(inst->n * sizeof(int));
    best->addedPtr = 0;
    best->fx = sol->fx;
}

void applyBest(instance_t* inst, solution_t* sol, best_t* best) {
    for (int i = 0; i < best->addedPtr; i++) {
        addSet(inst, sol, best->added[i]);
    }
    removeSet(inst, sol, best->removed);
}

void bestImprove(instance_t* inst, solution_t* sol) {
    int improvement = 1;
    int* added = mymalloc(inst->n * sizeof(int));
    int addedPtr = 0;
    
    cpy = (solution_t*) mymalloc(sizeof(solution_t));
    best = (best_t*) mymalloc(sizeof(best_t));
    initCopy(inst, sol, cpy);
    initBest(inst, sol, best);
    
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            if (cpy->x[i]) {
                removeSet(inst, cpy, i);
                while (!isSolution(cpy)) {
                    int col = replaceSet(inst, i);
                    if (col >= 0) {
                        added[addedPtr] = col;
                        addedPtr += 1;
                    }
                }
                if (cpy->fx < best->fx) {
                    improvement = 1;
                    best->fx = cpy->fx;
                    best->removed = i;
                    if (addedPtr > 0) {
                        best->addedPtr = 0;
                        for (int j = 0; j < addedPtr; j++) {
                            best->added[best->addedPtr] = added[j];
                            best->addedPtr += 1;
                        }
                    }
                }
                copySolution(inst, sol, cpy);
                addedPtr = 0;
            }
        }
        if (improvement) {
            applyBest(inst, sol, best);
            if (re) {
                eliminate(inst, sol);
            }
            copySolution(inst, sol, cpy);
            best->removed = -1;
            best->addedPtr = 0;
        }
    }
}

void firstImprove(instance_t* inst, solution_t* sol) {
    int improvement = 1;
    cpy = (solution_t*) mymalloc(sizeof(solution_t));
    initCopy(inst, sol, cpy);
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            if (cpy->x[i]) {
                removeSet(inst, cpy, i);
                while (!isSolution(cpy)) {
                    replaceSet(inst, i);
                }
                if (cpy->fx < sol->fx) {
                    copySolution(inst, cpy, sol);
                    improvement = 1;
                    if (re) {
                        eliminate(inst, sol);
                    }
                } else {
                    copySolution(inst, sol, cpy);
                }
            }
        }
    }
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
    readParameters(argc, argv);
    readSCP(instance_file);
    initialize();
    srand(seed);
    solve();
    printf("%d\n", soln->fx);
    finalize();
    return 0;
}

