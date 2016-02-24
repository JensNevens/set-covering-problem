//
//  lsscp.h
//  HO-Project1
//
//  Created by Jens Nevens on 24/02/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef lsscp_h
#define lsscp_h

void usage();
void read_parameters(int argc, char *argv[]);
void read_scp(char *filename);
void print_instance(int level);
void finalize();

#endif /* lsscp_h */
