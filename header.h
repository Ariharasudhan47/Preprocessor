#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SIZE 4096            
#define MAX_INCLUDE_DEPTH 32 

void remove_comments(const char *, char *, int *);
void process_file(FILE *, FILE *);
void macro_replace(char *, FILE *);
int  macro_defined(const char *);
void macro_undef(const char *);

#endif
