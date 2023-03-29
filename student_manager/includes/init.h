#ifndef INIT_H
#define INIT_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "structs_typedefs.h"
#include "parse.h"
#include "list.h"
#include "hashtable.h"

HashTable init_student_ht(int buckets);

void insert_zip(List zip, int pcode);

void insert_from_file(HashTable students, List years, List zips, char* inputfile);

void insert_inv_index(List years, Student s);
#endif