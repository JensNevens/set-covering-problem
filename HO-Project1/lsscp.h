//
//  lsscp.h
//  HO-Project1
//
//  Created by Jens Nevens on 24/02/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef lsscp_h
#define lsscp_h

// Solution struct
struct Solution {
    int* x;          // x[i] 0,1 if column i is selected
    int* y;          // y[i] 0,1 if row i covered by the actual solution
    int fx;          // sum of the cost of the columns selected in the solution
    int** col_cover; // col_colver[i] contains selected columns that cover row i
    int* ncol_cover; // ncol_cover[i] contains number of selected columns that cover row i
    int un_rows;     // the amount of un-covered rows
    int un_cols;     // the amount of un-used columns
};

typedef struct Solution solution_t;

struct Best {
    int removed;
    int* added;
    int addedPtr;
    int fx;
};

typedef struct Best best_t;

// General functions
void usage();
void readParameters(int argc, char* argv[]);
void readSCP(char* filename);
void printInstance(int level);
void initialize();
void* mymalloc(size_t size);
void errorExit(char* text);
void finalize();

// Functions needed by all algorithms
void addSet(solution_t* sol, int colidx);
int redundant(solution_t* sol, int colidx);
void shift(solution_t* sol, int rowidx, int from);
void removeSet(solution_t* sol, int colidx);
int isBetter(int newCol, float newCost, int currCol, float currCost);
int costSort(const void* a, const void* b);
void eliminate();
int isSolution(solution_t* sol);
void diagnostics();

// Random Construction
unsigned int pickRandom(unsigned int min, unsigned int max);
void randomConstruction();

// Cost Based
float adaptiveCost(solution_t* sol, int colidx);
float getCost(int colidx);
void costBased();

// Iterative Improvement
void initCopy(solution_t* sol);
void copySolution(solution_t* from, solution_t* to);
void freeSolution(solution_t* sol);
int replaceSet(int colidx);
void initBest(best_t* best);
void applyBest(solution_t* sol);
void bestImprove();
void firstImprove();

// Main
void solve();
int main(int argc, char *argv[]);

#endif /* lsscp_h */
