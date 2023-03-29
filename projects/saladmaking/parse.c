#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stddef.h> // size_t
#include <stdbool.h>


// when concurrent processes go from 1 -> 2 we save the starting point
// and when they go from 2 -> 1 we print the interval and we are ready find another interval

void parse(FILE* fp)
{
    int concurrent = 0;

    char* line = NULL;
    size_t len = 0;
    
    // needle for start
    char start[] = "Start making salad";
    
    // needle for end
    char end[] = "End making salad";

    char* start_token;
    char* end_token;
    bool new_interval = true;

    while((getline(&line, &len, fp) != -1))
    {
        char* res1 = strstr(line, start);
        char* res2 = strstr(line, end);
        if(res1 != NULL)
        {
            concurrent++;
            if(concurrent == 2 && new_interval)
            {
                start_token = strdup(strtok(line, " "));
                new_interval = false;
            }
        }
        res1 = NULL;
        if(res2 != NULL)
        {
            concurrent--;
            if(concurrent == 1)
            {
                end_token = strdup(strtok(line, " "));
                printf("%s, %s\n", start_token, end_token);
                free(start_token);
                free(end_token);
                new_interval = true;
            }
        }
        res2 = NULL;

    }
    if(line) free(line); // getline calls malloc
}














void print_in_log(FILE* fp, struct tm* local, struct timeval* now, int pid, char* proc_name, char* msg)
{
    fprintf(fp, "[%02d:%02d:%02d.%03ld] [%d] [%s] [%s] \n", 
    local->tm_hour, local->tm_min, local->tm_sec, now->tv_usec / 1000, pid, proc_name, msg);
    fflush(fp);
}

