//
//  lsscp.h
//  HO-Project1
//
//  Created by Jens Nevens on 24/02/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef lsscp_h
#define lsscp_h

// Instance struct
struct Instance {
    int m;        // number of rows
    int n;        // number of columns
    int** row;    // row[i] contains rows that are covered by column i
    int** col;    // col[i] contains columns that cover row i
    int* nrow;    // ncol[i] contains number of columns that cover row i
    int* ncol;    // nrow[i] contains number of rows that are covered by column i
    int* cost;    // cost[i] contains cost of column i
    float* ccost; // ccost[i] contains static cover cost of column i
};

typedef struct Instance instance_t;

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
void initialize();
void errorExit(char* text);
void finalize();
void diagnostics();
void* mymalloc(size_t size);

// Helper functions
void addSet(instance_t* inst, solution_t* sol, int colidx);
int redundant(instance_t* inst, solution_t* sol, int colidx);
void shift(solution_t* sol, int rowidx, int from);
void removeSet(instance_t* inst, solution_t* sol, int colidx);
int isBetter(instance_t* inst, int newCol, float newCost, int currCol, float currCost);
int costSort(const void* a, const void* b);
void eliminate(instance_t* inst, solution_t* sol);
int isSolution(solution_t* sol);

// Constructive functions
//// Random Construction
unsigned int pickRandom(unsigned int min, unsigned int max);
void randomConstruction(instance_t* inst, solution_t* sol);
//// Cost Based
float adaptiveCost(instance_t* inst, solution_t* sol, int colidx);
float getCost(instance_t* inst, solution_t* sol, int colidx);
void costBased(instance_t* inst, solution_t* sol);

// Improvement functions
void initCopy(instance_t* inst, solution_t* sol, solution_t* new);
void copySolution(instance_t* inst, solution_t* from, solution_t* to);
void freeSolution(solution_t* sol);
int replaceSet(instance_t* inst, int colidx);
void initBest(instance_t* inst, solution_t* sol, best_t* best);
void applyBest(instance_t* inst, solution_t* sol, best_t* best);
void bestImprove(instance_t* inst, solution_t* sol);
void firstImprove(instance_t* inst, solution_t* sol);

// Main
void solve();
int main(int argc, char *argv[]);

#endif /* lsscp_h */
