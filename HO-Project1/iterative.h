//
//  iterative.h
//  HO-Project1
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef iterative_h
#define iterative_h

#include "data.h"

// Improvement functions
void allocCopy(instance_t* inst, solution_t* cpy);
void allocBest(instance_t* inst, best_t* best);

void initCopy(instance_t* inst, solution_t* sol, solution_t* cpy);
void initBest(solution_t* sol, best_t* best);

void freeCopy(instance_t* inst, solution_t* cpy);
void freeBest(best_t* best);

void copySolution(instance_t* inst, solution_t* from, solution_t* to);
int replaceSet(instance_t* inst, solution_t* cpy, int colidx);
void applyBest(instance_t* inst, solution_t* sol, best_t* best);
void bestImprove(instance_t* inst, solution_t* sol);
void firstImprove(instance_t* inst, solution_t* sol);

#endif /* iterative_h */
