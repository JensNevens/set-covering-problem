//
//  utils.c
//  HO-Project1
//
//  Created by Jens Nevens on 24/02/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

void *mymalloc(size_t size) {
    void *s;
    if ((s=malloc(size)) == NULL) {
        fprintf(stderr, "malloc : Not enough memory.\n");
        exit(EXIT_FAILURE);
    }
    return s;
}

void error_reading_file(char *text) {
    printf("%s\n", text);
    exit(EXIT_FAILURE);
}


