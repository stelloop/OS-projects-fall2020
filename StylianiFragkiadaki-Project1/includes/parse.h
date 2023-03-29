#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "structs_typedefs.h"

int count_lines(char* input);
void recognise_command(void);

Student init_student(char** arr, bool from_stdin);


void get_args(char* line, char** arr_tokens);
int count_args(char** arr_tokens);
void helper_print_student(Student s);
#endif