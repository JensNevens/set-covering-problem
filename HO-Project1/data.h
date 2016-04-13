//
//  data.h
//  HO-Project1
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef data_h
#define data_h

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

extern int ch1, ch2, ch3, ch4, bi, fi, re;

#endif /* data_h */
