#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./includes/parse.h"
#include "./includes/list.h"
#include "./includes/hashtable.h"
#include "./includes/init.h"
#include "./includes/compare_funcs.h"


HashTable init_student_ht(int buckets)
{
    HashTable students = ht_create((CompareFunc)compare_keys, NULL,(DestroyFunc) destroy_student, buckets); 
    ht_set_hash_function(students, hash_int);
    return students;
}

Year_Info init_year_node(Student s)
{
    Year_Info new = malloc(sizeof(*new));
    new->count = 0;
    new->gpa_sum = 0;
    new->year = s->year;
    new->students = list_create(NULL); // list with the students of the same year
    return new;
}

void insert_zip(List zip, int pcode)
{ 
    for(ListNode node = list_first(zip); node != NULL; node = list_next(zip, node))
    {
        Pair p = list_node_value(node);
        if(p->zip == pcode) // zip already exists, increase it's count
        {
            p->count++;
            return;
        }
    }
    Pair new = malloc(sizeof(*new));
    new->count = 1;
    new->zip = pcode;
    list_insert_next(zip, NULL, new); // new zip found!
}

void insert_inv_index(List years, Student s)
{
    Year_Info year_node = list_find(years, &s->year, (CompareFunc)compare_years);
    if(year_node == NULL)
    {
        list_insert_next(years, NULL, init_year_node(s)); // inserted at the start of the year-list.
        year_node = list_node_value(list_first(years));
    }
    year_node->count++;
    year_node->gpa_sum += s->gpa;
   
    ListNode prev = NULL; 
    ListNode node = list_first(year_node->students);
    if(node == NULL)   // empty list, insert at start
        list_insert_next(year_node->students, NULL, s);
    else
    {
        while(node != NULL)
        {
            Student s_node = list_node_value(node);
            if(s->gpa > s_node->gpa)
            {
                list_insert_next(year_node->students, prev, s); // works also at the start of the list
                break;
            }
            prev = node;
            node = list_next(year_node->students, node);
        }
        if(node == NULL) // has the min gpa, gets inserted in the end
            list_insert_next(year_node->students, list_last(year_node->students), s);
    }
}
void insert_from_file(HashTable students, List years, List zips, char* inputfile)
{
    int duplicates = 0;
    int insertions = 0;
    FILE* fptr = fopen(inputfile, "r");
    if(fptr == NULL)
    {
        fprintf(stderr, "Can't open file. \n");
        exit(EXIT_FAILURE);
    }
    char line[128];


    
    char* arr_tokens[7]; // max tokens of insert q
    while(fgets(line, 128, fptr) != NULL)
    {
        bool inserted = true;
        get_args(line, arr_tokens);
        Student to_add = init_student(arr_tokens, false);
        inserted = ht_insert(students, &to_add->student_id, to_add);// den me endiaferei na vgalw mhnuma lathous edw
        if(!inserted)
        {
            duplicates++;
            destroy_student(to_add);
        }
        else 
        {
            insert_inv_index(years, to_add);
            insert_zip(zips, to_add->zip);
            insertions++;
        }
    }

    printf("- Finished reading input file\n");
    printf("%d students inserted \n", insertions);
    printf("%d duplicated records found and not inserted\n", duplicates);
    fclose(fptr);
}
