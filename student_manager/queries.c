#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./includes/parse.h"
#include "./includes/list.h"
#include "./includes/hashtable.h"
#include "./includes/init.h"
#include "./includes/compare_funcs.h"
#include "./includes/queries.h"
#include "./includes/structs_typedefs.h"


// global data structures
extern HashTable students;
extern List years;
extern List zips;

void q_exit(void)
{
    for(ListNode node = list_first(years); node != NULL; node = list_next(years, node))
    {
        Year_Info a = list_node_value(node);
        list_destroy(a->students);
    }
    list_destroy(years);
    list_destroy(zips);
    ht_destroy(students);
    printf("- exit program\n");
    exit(EXIT_SUCCESS);

}


void q_delete(char** arr)  
{
      if(count_args(arr) != 2)
    {
        fprintf(stderr, "ERROR! expected input: d studentid\n");
        return;
    }
    int student_id = atoi(arr[1]);
    Student s = ht_find(students, &student_id);  // lookup first
    if(s == NULL) 
        fprintf(stderr, "- Student %d does not exist\n", student_id);
    else
    {
        // remove student from the inverted index
        Year_Info year_node = list_find(years, &s->year, (CompareFunc)compare_years);
        year_node->count--;
        year_node->gpa_sum -= s->gpa;
        Student first = list_node_value(list_first(year_node->students)); // we have at least one element
        ListNode node;
        if(first->student_id == student_id)
            list_remove(year_node->students, list_first(year_node->students)); // delete head of the list
        else
        {   // traverse the list until you find the student 
            for(node = list_first(year_node->students); node != NULL; node = list_next(year_node->students, node))
            {
                    Student s_node = list_node_value(node);
                    if(s_node->student_id == student_id)
                        break;
                
            }
            list_remove(year_node->students, node);
        }
        // decrease the count of the student's zip
        for(ListNode node = list_first(zips); node != NULL; node = list_next(zips, node))
        {
            Pair p = list_node_value(node);
            if(p->zip == s->zip)
            {
                p->count--;
                break; // found, stop
            }
        }
        // delete s from the students hashtable
        ht_remove(students, s);
        printf("- Record %d deleted\n", student_id);
    }
}

void q_lookup(char** arr)
{
    if(count_args(arr) != 2)
    {
        fprintf(stderr, "ERROR! expected input: l studentid\n");
        return;
    }
    int student_id = atoi(arr[1]);
    Student s = ht_find(students, &student_id);
    if(s == NULL)
        fprintf(stderr, "- Student %d does not exist\n", student_id);
    else
        printf("- %d %s %s %d %d %.2f\n", s->student_id, s->lastname, s->firstname, s->zip, s->year, s->gpa);
}   

void q_insert(char** arr) 
{
    if(count_args(arr) != 7)
    {
        fprintf(stderr, "ERROR! expected input: i studentid lastname firstname zip year gpa\n");
        return;
    }
    bool inserted;
    Student to_add = init_student(arr, true);
    inserted = ht_insert(students, &to_add->student_id, to_add);
    if(!inserted)
    {
        fprintf(stderr, "- Student %d already exists;\n", to_add->student_id);
        destroy_student(to_add);   // destroy the previously malloc'd struct student
    }
    else // update the inv. index and the postal codes list!
    {
        insert_inv_index(years, to_add);
        insert_zip(zips, to_add->zip);
        printf("student %d inserted! \n", to_add->student_id);
    }
}

void q_number(char** arr)
{
    if(count_args(arr) != 2)
    {
        fprintf(stderr, "ERROR! expected input: n year\n");
        return;
    }
    int year = atoi(arr[1]);
    Year_Info year_node = list_find(years, &year, (CompareFunc)compare_years);
    if(year_node == NULL)
        fprintf(stderr, "- No students enrolled in %d\n", year);
    else
        printf("- %d student(s) in %d\n", year_node->count, year);
}

void q_average(char** arr)
{
    if(count_args(arr) != 2)
    {
        fprintf(stderr, "ERROR! expected input: a year\n");
        return;
    }
    int year = atoi(arr[1]);
    Year_Info year_node = list_find(years, &year, (CompareFunc)compare_years);
    if(year_node == NULL) 
        fprintf(stderr, "- No students enrolled in %d \n", year);
    else
        printf("%.2f\n", year_node->gpa_sum/year_node->count);
}

void q_minimum(char** arr)
{
    // We may have more than one students with the __same minimum__ gpa!

    bool first_printed = true; // for printing results (whether a comma will be printed or not)
    if(count_args(arr) != 2)
    {
        fprintf(stderr, "ERROR! expected input: m year\n");
        return;
    }
    int year = atoi(arr[1]);
    Year_Info year_node = list_find(years, &year, (CompareFunc)compare_years);
    if(year_node == NULL) 
        fprintf(stderr, "- No students enrolled in %d \n", year);
    else
    {    // student of the last node has the minimum gpa
        Student min_s = list_node_value(list_last(year_node->students)); 
        for(ListNode node = list_first(year_node->students); node != NULL; node = list_next(year_node->students, node))
        {
            Student s = list_node_value(node);
            if(s->gpa - min_s->gpa < 0.0001) // instead of using ==
            {
                first_printed ? printf("%d", s->student_id) : printf(", %d", s->student_id);
                first_printed = false; // every other print starts with a comma
            }
        }
        printf("\n");
    }
}

void q_count(char** arr)
{   
    if(count_args(arr) != 1)
    {
        fprintf(stderr, "ERROR! expected input: c \n");
        return;
    }
    
    ListNode node = list_first(years);
    if(node == NULL) 
        fprintf(stderr, "- No students are enrolled\n");
    Year_Info year_node = list_node_value(node);
    printf("{%d, %d}", year_node->year, year_node->count);
    node = list_next(years, node);
    for(ListNode curr = node; curr != NULL; curr = list_next(years, curr))
    {
        year_node = list_node_value(curr);
        printf(", {%d, %d}", year_node->year, year_node->count);
    }
    printf("\n");
}

void q_top(char** arr)
{
    if(count_args(arr) != 3)
    {
        fprintf(stderr, "ERROR! expected input: t num year\n");
        return;
    }
    int num = atoi(arr[1]);
    if(num == 0)
    {
        fprintf(stderr, "ERROR! there is no num zero (0). Type a valid num\n");
        return;
    }
    int year = atoi(arr[2]);
    Year_Info year_node = list_find(years, &year, (CompareFunc)compare_years);
    if(year_node == NULL) 
        fprintf(stderr, "- No students enrolled in %d \n", year);
    else 
    {
        int size = list_size(year_node->students);
        if(num > size) 
            num = size; // basically print all students of the year
        
        // print the first student (without comma)
        ListNode node = list_first(year_node->students);
        Student s = list_node_value(node);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
        printf("%d", s->student_id);
        node = list_next(year_node->students, node);
        for(int i = 1; i < num; i++)
        {
            s = list_node_value(node);
            printf(", %d", s->student_id);
            node = list_next(year_node->students, node);
        }
        // there is a chance that more than num+1, num+2... have the same grade with num
        // so we have to print them too.
        float numth_grade = s->gpa;
        while(node != NULL)
        {
            s = list_node_value(node);
            if(s->gpa < numth_grade)
                break; // found a different grade than num's, stop
            else
                printf(", %d", s->student_id);
            node = list_next(year_node->students, node);
        }
        printf("\n");
    }
}

int qsort_compare(const void* a, const void* b)
{
    Pair pa = (Pair)a;
    Pair pb = (Pair)b;
    return pb->count - pa->count;
}                                                                                                                                                                                                                                                                                                                                                                                       
void q_postal_code(char** arr)
{
    if(count_args(arr) != 2)
    {
        fprintf(stderr, "ERROR! expected input: p rank\n");
        return;
    }
    int rank = atoi(arr[1]);
    if(rank == 0)
    {
        fprintf(stderr, "ERROR! there is no rank zero (0). Type a valid rank\n");
        return;
    }
    // create an array with the elements of the list
    int size = list_size(zips);
    if(size == 0)
    {
        fprintf(stderr, "- No students are enrolled\n");
        return;
    }
    Pair array = malloc(list_size(zips) * sizeof(struct pair)); // array of structs
    int i = 0;
    for(ListNode node = list_first(zips); node != NULL; node = list_next(zips, node))
    {
        Pair p = list_node_value(node);
        array[i] = *p;
        i++;
    }
    // sort it in descending order
    qsort(array, size, sizeof(struct pair), qsort_compare);
    
    bool first_printed = true;
    int curr_rank = 1;
    i = 0;
    // find the index of the array, with the first `rank`th zip
    while(curr_rank < rank && i < size) // i mustn't go out of bounds 
    {
        if((i+1 == size)) {
            fprintf(stderr, "- There is no such rank\n");
            free(array);
            return;

        }
        if(array[i].count > array[i + 1].count)
            curr_rank++;
        i++;
    }

    // print all the zips of the rank
    while(true)
    {
        if(first_printed)
        {
            printf("- %d",array[i].zip);
            first_printed = false;
        }
        else
            printf(", %d", array[i].zip);
        if((i+1 == size)) break;
        if(array[i + 1].count < array[i].count) break;
        i++;
    }
    printf(" is/are %d most popular\n", rank);
    free(array); // de-allocate the temporary array 
    
}