#ifndef STRUCTS_TYPEDEFS
#define STRUCTS_TYPEDEFS

#include "list.h"
struct student {
    int student_id;
    char* lastname;
    char* firstname;
    int zip;
    int year;
    float gpa;
};

typedef struct student* Student;

struct years_info {
    int year;
    int count;
    float gpa_sum;
    List students;
};

typedef struct years_info* Year_Info;

struct pair {
    int zip;
    int count;
};

typedef struct pair* Pair;
#endif