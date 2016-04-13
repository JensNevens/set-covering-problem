//
//  constructive.c
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

