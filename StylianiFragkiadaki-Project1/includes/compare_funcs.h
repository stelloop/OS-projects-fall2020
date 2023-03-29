#ifndef COMPARE_FUNCS_H
#define COMPARE_FUNCS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "structs_typedefs.h"
int compare_keys(void* a, void* b);

int compare_years(Year_Info a, Year_Info b);

void destroy_student(Student s);
#endif