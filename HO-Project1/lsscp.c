//
//  lsscp.c
//  HO-Project1
//
//  Created by Jens Nevens on 24/02/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

/*************************************/
/** For this code:                  **/
/**   rows = elements               **/
/**   cols = subsets                **/
/*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "utils.h"

/** Algorithm parameters **/
int seed = 1234567;
char *scp_file = "";
char *output_file = "output.txt";

/** Variables to activate algorithms **/
int ch1 = 0, ch2 = 0, bi = 0, fi = 0, re = 0;

/** Instance static variables **/
int m;            /* number of rows */
int n;            /* number of columns */
int **row;        /* row[i] contains rows that are covered by column i */
int **col;        /* col[i] contains columns that cover row i */
int *ncol;        /* ncol[i] contains number of columns that cover row i */
int *nrow;        /* nrow[i] contains number of rows that are covered by column i */
int *cost;        /* cost[i] contains cost of column i  */

/** Solution variables **/
int *x;           /* x[i] 0,1 if column i is selected */
int *y;           /* y[i] 0,1 if row i covered by the actual solution */
/** Note: Use incremental updates for the solution **/
int fx;           /* sum of the cost of the columns selected in the solution (can be partial) */

/** Dynamic variables **/
/** Note: use dynamic variables to make easier the construction and modification of solutions. **/
/**       these are just examples of useful variables.                                         **/
/**       These variables need to be updated eveytime a column is added to a partial solution  **/
/**       or when a complete solution is modified*/
int *col_cover;   /* col_colver[i] contains selected columns that cover row i */
int ncol_cover;   /* number of selected columns that cover row i */

void usage() {
    printf("\nUSAGE: lsscp [param_name, param_value] [options]...\n");
    printf("Parameters:\n");
    printf("  --seed : seed to initialize random number generator\n");
    printf("  --instance: SCP instance to execute.\n");
    printf("  --output: Filename for output results.\n");
    printf("Options:\n");
    printf("  --ch1: random solution construction\n");
    printf("  --ch2: static cost-based greedy values.\n");
    printf("  --re: applies redundancy elimination after construction.\n");
    printf("  --bi: best improvement.\n");
    printf("\n");
}


/*Read parameters from command line*/
/*NOTE: Complete parameter list*/
void read_parameters(int argc, char *argv[]) {
    int i;
    if (argc <= 1) {
        usage();
        exit(EXIT_FAILURE);
    }
    for(i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0) {
            seed=atoi(argv[i+1]);
            i+=1;
        } else if (strcmp(argv[i], "--instance") == 0) {
            scp_file=argv[i+1];
            i+=1;
        } else if (strcmp(argv[i], "--output") == 0) {
            output_file=argv[i+1];
            i+=1;
        } else if (strcmp(argv[i], "--ch1") == 0) {
            ch1=1;
        } else if (strcmp(argv[i], "--ch2") == 0) {
            ch2=1;
        } else if (strcmp(argv[i], "--bi") == 0) {
            bi=1;
        } else if (strcmp(argv[i], "--fi") == 0) {
            fi=1;
        } else if (strcmp(argv[i], "--re") == 0) {
            re=1;
        } else {
            printf("\nERROR: parameter %s not recognized.\n", argv[i]);
            usage();
            exit( EXIT_FAILURE );
        }
    }
    if((scp_file == NULL) || ((scp_file != NULL) && (scp_file[0] == '\0'))) {
        printf("Error: --instance must be provided.\n");
        usage();
        exit( EXIT_FAILURE );
    }
}

/*** Read instance in the OR-LIBRARY format ***/
void read_scp(char *filename) {
    int h,i,j;
    int *k;
    FILE *fp = fopen(filename, "r");
    
    if (fscanf(fp,"%d",&m) != 1)   /* number of rows */
        error_reading_file("ERROR: there was an error reading instance file.");
    if (fscanf(fp,"%d",&n) != 1)   /* number of columns */
        error_reading_file("ERROR: there was an error reading instance file.");
    
    /* Cost of the n columns */
    cost = (int *) mymalloc(n * sizeof(int));
    for (j = 0; j < n; j++)
        if (fscanf(fp,"%d",&cost[j]) != 1)
            error_reading_file("ERROR: there was an error reading instance file.");
    
    /* Info of columns that cover each row */
    col  = (int **) mymalloc(m * sizeof(int *));
    ncol = (int *) mymalloc(m * sizeof(int));
    for (i = 0; i < m; i++) {
        if (fscanf(fp,"%d",&ncol[i]) != 1)
            error_reading_file("ERROR: there was an error reading instance file.");
        col[i] = (int *) mymalloc(ncol[i] * sizeof(int));
        for (h = 0; h < ncol[i]; h++) {
            if(fscanf(fp,"%d",&col[i][h]) != 1)
                error_reading_file("ERROR: there was an error reading instance file.");
            col[i][h]--;
        }
    }
    
    /* Info of rows that are covered by each column */
    row  = (int **) mymalloc(n*sizeof(int *));
    nrow = (int *) mymalloc(n*sizeof(int));
    k    = (int *) mymalloc(n*sizeof(int));
    for (j = 0; j < n; j++) nrow[j] = 0;
    for (i = 0; i < m; i++) {
        for (h = 0; h < ncol[i]; h++)
            nrow[col[i][h]]++;
    }
    for (j = 0; j < n; j++) {
        row[j] = (int *) mymalloc(nrow[j]*sizeof(int));
        k[j] = 0;
    }
    for (i = 0; i < m; i++) {
        for (h = 0; h < ncol[i]; h++) {
            row[col[i][h]][k[col[i][h]]] = i;
            k[col[i][h]]++;
        }
    }
    free((void *)k);
}

/*** Use level>=1 to print more info (check the correct reading) */
void print_instance(int level) {
    int i;
    
    printf("**********************************************\n");
    printf("  SCP INSTANCE: %s\n", scp_file);
    printf("  PROBLEM SIZE\t m = %d\t n = %d\n", m, n);
    
    if(level >= 1) {
        printf("  COLUMN COST:\n");
        for(i = 0; i < n; i++)
            printf("%d ",cost[i]);
        printf("\n\n");
        printf("  NUMBER OF ROWS COVERED BY COLUMN 1 is %d\n", nrow[0] );
        for(i = 0; i < nrow[0]; i++)
            printf("%d ", row[0][i]);
        printf("\n");
        printf("  NUMBER OF COLUMNS COVERING ROW 1 is %d\n", ncol[0] );
        for(i = 0; i < ncol[0]; i++)
            printf("%d ", col[0][i]);
        printf("\n");
    }
    printf("**********************************************\n\n");
}

/*** Use this function to initialize other variables of the algorithms **/
//void initialize(){}

/*** Use this function to finalize execution */
void finalize() {
    free((void **) row );
    free((void **) col );
    free((void *) nrow );
    free((void *) ncol );
    free((void *) cost );
}

