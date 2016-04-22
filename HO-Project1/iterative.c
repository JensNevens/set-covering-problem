//
//  iterative.c
//  HO-Project1
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "constructive.h"
#include "lsscp.h"
#include "iterative.h"

/********************
 Iterative algorithms
 *******************/

void allocCopy(instance_t* inst, solution_t* cpy) {
    cpy->x = mymalloc(inst->n * sizeof(int));
    cpy->y = mymalloc(inst->m * sizeof(int));
    cpy->col_cover = mymalloc(inst->m * sizeof(int*));
    cpy->ncol_cover = mymalloc(inst->m * sizeof(int));
    for (int i = 0; i < inst->m; i++) {
        cpy->col_cover[i] = mymalloc(inst->ncol[i] * sizeof(int));
    }
}

void allocBest(instance_t* inst, best_t* best) {
    best->added = mymalloc(inst->n * sizeof(int));
}

void initCopy(instance_t* inst, solution_t* sol, solution_t* cpy) {
    cpy->fx = sol->fx;
    cpy->un_rows = sol->un_rows;
    cpy->un_cols = sol->un_cols;
    for (int i = 0; i < inst->n; i++) cpy->x[i] = sol->x[i];
    for (int i = 0; i < inst->m; i++) {
        cpy->y[i] = sol->y[i];
        cpy->ncol_cover[i] = sol->ncol_cover[i];
        int k = inst->ncol[i];
        for (int j = 0; j < k; j++) {
            cpy->col_cover[i][j] = sol->col_cover[i][j];
        }
    }

}

void initBest(solution_t* sol, best_t* best) {
    best->removed = -1;
    best->addedPtr = 0;
    best->fx = sol->fx;
}

void freeCopy(instance_t* inst, solution_t* cpy) {
    for (int i = 0; i < inst->m; i++) free(cpy->col_cover[i]);
    free(cpy->col_cover);
    free(cpy->ncol_cover);
    free(cpy->x);
    free(cpy->y);
}

void freeBest(best_t* best) {
    free(best->added);
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
 Add the best possible set to the solution as long as
 1. it is not colidx, the set that was just removed
 2. it is not already used
 3. it is not redundant
 The best set is determined by the adaptive cover
 cost-based greedy heuristic.
 **/
int replaceSet(instance_t* inst, solution_t* cpy, int colidx) {
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
    
    solution_t* cpy = mymalloc(sizeof(solution_t));
    best_t* best = mymalloc(sizeof(best_t));
    allocCopy(inst, cpy);
    allocBest(inst, best);
    copySolution(inst, sol, cpy);
    initBest(sol, best);
    
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            if (cpy->x[i]) {
                removeSet(inst, cpy, i);
                while (!isSolution(cpy)) {
                    int col = replaceSet(inst, cpy, i);
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
    freeCopy(inst, cpy);
    freeBest(best);
    free(cpy);
    free(best);
    free(added);
}

void firstImprove(instance_t* inst, solution_t* sol) {
    int improvement = 1;
    solution_t* cpy = mymalloc(sizeof(solution_t));
    allocCopy(inst, cpy);
    copySolution(inst, sol, cpy);
    
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            if (cpy->x[i]) {
                removeSet(inst, cpy, i);
                while (!isSolution(cpy)) {
                    replaceSet(inst, cpy, i);
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
    freeCopy(inst, cpy);
    free(cpy);
}
