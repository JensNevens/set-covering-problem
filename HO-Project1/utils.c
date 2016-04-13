//
//  utils.c
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
 Check if the current solution is a solution.
 i.e. when no rows are un-covered.
 **/
int isSolution(solution_t* sol) {
    return (sol->un_rows <= 0);
}


