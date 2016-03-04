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
void readParameters(int argc, char* argv[]);
void readSCP(char* filename);
void printInstance(int level);
void initialize();
void* mymalloc(size_t size);
void errorExit(char* text);
void finalize();
int main(int argc, char *argv[]);
void solve();

// Functions needed by all algorithms
void addSet(int colidx);
void shift(int rowidx, int from);
void removeSet(int colidx);
int redundant(int colidx);
int isBetter(int newCol, float newCost, int currCol, float currCost);
void eliminate();
int isSolution();
void diagnostics();

// Random Construction
float randomFloat();
int pickRandom(int setSize);
void RandomConstruction();

// Cost Based
float adaptiveCost(int colidx);
float getCost(int colidx);
void costBased();

// Iterative Improvement
void bestImprovement();
void firstImprovement();
void iterative();

#endif /* lsscp_h */
