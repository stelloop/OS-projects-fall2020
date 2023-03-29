
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./includes/parse.h"
#include "./includes/structs_typedefs.h"
#include "./includes/queries.h"

int count_lines(char* input)
{
    int lines = 0;
    FILE* fptr = fopen(input, "r");
    if(fptr == NULL)
    {
        fprintf(stderr, "- Can't open file. \n");
        exit(EXIT_FAILURE);
    }
    int c;
    while((c = fgetc(fptr)) != EOF)
        if(c == '\n')
            lines++;

    fclose(fptr);
    return lines;
}
// recognise command
void recognise_command(void)
{

    char line[128];
    while(true)  // stops with the exit command (with the use of the exit())
    {
        fgets(line, 128, stdin);
        char* arr_tokens[7]; // max tokens of insert q
        get_args(line, arr_tokens);
        if(arr_tokens[0] == NULL) continue;
        if(strcmp(arr_tokens[0], "i")== 0) q_insert(arr_tokens);
        if(strcmp(arr_tokens[0], "l")== 0) q_lookup(arr_tokens);
        if(strcmp(arr_tokens[0], "d")== 0) q_delete(arr_tokens);
        if(strcmp(arr_tokens[0], "n")== 0) q_number(arr_tokens);
        if(strcmp(arr_tokens[0], "t")== 0) q_top(arr_tokens);
        if(strcmp(arr_tokens[0], "a")== 0) q_average(arr_tokens);
        if(strcmp(arr_tokens[0], "m")== 0) q_minimum(arr_tokens);
        if(strcmp(arr_tokens[0], "c")== 0) q_count(arr_tokens);
        if(strcmp(arr_tokens[0], "p")== 0) q_postal_code(arr_tokens);
        if(strcmp(arr_tokens[0], "e")== 0) q_exit();
    }
}

int count_args(char** arr)
{
    int i;
    for(i = 0; i < 7; i++) // max args : 7
        if(arr[i] == NULL)
            break;
    return i;
}

void get_args(char* line, char** arr_tokens)
{
    for(int i = 0; i < 7; i++)
        arr_tokens[i] = NULL;

    char delimiters[] = " \n";
    arr_tokens[0] = strtok(line, delimiters); // the command itself e.g i, p, n
    int i = 0;
    while(arr_tokens[i] != NULL)
    {
        i++;
        arr_tokens[i] = strtok(NULL, delimiters);
    }
}

// if input comes from stdin, ignore the first element (i - insert)
Student init_student(char** arr, bool from_stdin)
{
    Student s = malloc(sizeof(*s));
    int i;
    if(from_stdin) 
        i = 1;
    else 
        i = 0;
    s->student_id = atoi(arr[i]);
    s->lastname =strdup(arr[i + 1]);
    s->firstname = strdup(arr[i + 2]);
    s->zip = atoi(arr[i + 3]);
    s->year = atoi(arr[i + 4]);
    s->gpa = atof(arr[i + 5]);
    return s;
}

