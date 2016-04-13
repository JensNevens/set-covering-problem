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
void finalizeIterative();
void initCopy(instance_t* inst, solution_t* sol, solution_t* new);
void copySolution(instance_t* inst, solution_t* from, solution_t* to);
void freeSolution(solution_t* sol);
int replaceSet(instance_t* inst, int colidx);
void initBest(instance_t* inst, solution_t* sol, best_t* best);
void applyBest(instance_t* inst, solution_t* sol, best_t* best);
void bestImprove(instance_t* inst, solution_t* sol);
void firstImprove(instance_t* inst, solution_t* sol);

#endif /* iterative_h */
