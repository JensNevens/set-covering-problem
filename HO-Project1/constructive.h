//
//  constructive.h
//  HO-Project1
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef constructive_h
#define constructive_h

#include "data.h"

// Constructive functions
//// Random Construction
unsigned int pickRandom(unsigned int min, unsigned int max);
void randomConstruction(instance_t* inst, solution_t* sol);
//// Cost Based
float adaptiveCost(instance_t* inst, solution_t* sol, int colidx);
float getCost(instance_t* inst, solution_t* sol, int colidx);
void costBased(instance_t* inst, solution_t* sol);

#endif /* constructive_h */
