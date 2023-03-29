#include "../includes/compare_funcs.h"


int compare_keys(void* a, void* b) {
	return *(int*)a - *(int*)b;
}

int compare_years(Year_Info a, Year_Info b) {
    return a->year - b->year;
}

void destroy_student(Student s)
{
    free(s->firstname); // strdup() is used
    free(s->lastname);
    free(s);
}