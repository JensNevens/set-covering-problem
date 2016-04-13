//
//  lsscp.h
//  HO-Project1
//
//  Created by Jens Nevens on 24/02/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef lsscp_h
#define lsscp_h

#include "data.h"

// General functions
void usage();
void readParameters(int argc, char* argv[]);
void readSCP(char* filename);
void initialize();
void errorExit(char* text);
void finalize();
void diagnostics();

int costSort(const void* a, const void* b);
void eliminate(instance_t* inst, solution_t* sol);

// Main
void solve();
int main(int argc, char *argv[]);

#endif /* lsscp_h */
