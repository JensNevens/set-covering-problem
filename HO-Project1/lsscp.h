//
//  lsscp.h
//  HO-Project1
//
//  Created by Jens Nevens on 24/02/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef lsscp_h
#define lsscp_h

// General functions
void usage();
void read_parameters(int argc, char *argv[]);
void read_scp(char *filename);
void print_instance(int level);
void initialize();
void finalize();
void main_loop(int argc, char *argv[]);

// Functions needed by all algorithms
void addSet(int colidx);
void removeSet(int colidx);
int redundant(int colidx);
void eliminate();
int isSolution();
void testSolution();
void constructive();

// Random Construction
float randomFloat();
int pickRandom(int setSize);
void RandomConstruction();

#endif /* lsscp_h */
