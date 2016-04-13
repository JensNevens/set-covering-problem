//
//  utils.h
//  HO-Project1
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include "data.h"

// Helper functions
void* mymalloc(size_t size);
void addSet(instance_t* inst, solution_t* sol, int colidx);
int redundant(instance_t* inst, solution_t* sol, int colidx);
void shift(solution_t* sol, int rowidx, int from);
void removeSet(instance_t* inst, solution_t* sol, int colidx);
int isBetter(instance_t* inst, int newCol, float newCost, int currCol, float currCost);
int isSolution(solution_t* sol);

#endif /* utils_h */
