#define DEFAULT_SIZE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./includes/parse.h"
#include "./includes/list.h"
#include "./includes/hashtable.h"
#include "./includes/queries.h"
#include "./includes/init.h"


HashTable students = NULL;
List zips = NULL;
List years = NULL;
int main(int argc, char* argv[])
{
    years = list_create(free); //  a list of lists
    zips = list_create(free); // a list with the count of each individual postal code
    char *inputfile = NULL, *configfile = NULL;
    int buckets = 0;
    if(argc == 1) // no input or config file given
        students = init_student_ht(DEFAULT_SIZE); 
    else
    {
        for (int i = 1; i <= argc - 2; i += 2)   
                if(strcmp(argv[i], "-i") == 0)
                    inputfile = argv[i + 1];
                else if (strcmp(argv[i], "-c") == 0)
                    configfile = argv[i + 1]; 
    }    

    if(configfile != NULL)
    {
        FILE *cfptr = fopen(configfile, "r");
        if(cfptr == NULL)
        {
            fprintf(stderr, "ERROR! Can't open config file\n");
            exit(EXIT_FAILURE);
        }
        fscanf(cfptr, "%d", &buckets);
        students = init_student_ht(buckets);
        fclose(cfptr);
    }
    if(inputfile != NULL) // initialize the DS with the given data
    {
        if(configfile == NULL) // buckets are determined from the line count of the config file
        {
            buckets = 2 * count_lines(inputfile);
            students = init_student_ht(buckets); // ht
        }
        insert_from_file(students, years, zips, inputfile);
    }
    recognise_command();
}