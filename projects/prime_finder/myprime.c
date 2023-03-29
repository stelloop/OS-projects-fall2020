#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/times.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>

#include "includes.h"
#include "list.h"

#define READ 0
#define WRITE 1

int sigcount = 0; // glob

void sig_handler(int sig)
{
    if(sig == SIGUSR1)
        sigcount++;
}

// ascending order, takes Prime as a parameters (typedef'd ptr to struct s_prime)
int qsort_cmp(const void* a, const void* b) 
{
        Prime pa = *(Prime*)a;
        Prime pb = *(Prime*)b;
        return pb->number - pa->number;
        
}


void assign_starting_algo(int* curr_algo, int step, bool first)
{
    if(*curr_algo == 3) *curr_algo = 0; // start from the beggining, somewhat like a torroidal
    *curr_algo = ((*curr_algo) + step) % 4;   // 4 = total algos + 1
    if(first == true || *curr_algo == 0) *curr_algo = 1;
}

int main(int argc, char* argv[])
{
    
    // configure sigaction
    struct sigaction sigstruct;
    sigemptyset(&sigstruct.sa_mask);
    sigstruct.sa_flags = 0;
    sigstruct.sa_handler = sig_handler;
    sigaction(SIGUSR1, &sigstruct, NULL);

    int lowerb, upperb, num_of_children;
    int starting_algorithm = 0; 
    pid_t root_pid = getpid();
    
    if(argc == 1 || argc > 7)
    {
        fprintf(stderr, "usage: myprime -l lb -u ub -w NumofChildren\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        for(int i = 1; i <= argc-2; i += 2)
        {
            if(strcmp(argv[i], "-l") == 0)
                lowerb = atoi(argv[i + 1]);  
            else if (strcmp(argv[i], "-u") == 0)
                upperb = atoi(argv[i + 1]); 
            else if (strcmp(argv[i], "-w") == 0)
                num_of_children = atoi(argv[i + 1]);
        }
    }

    if((upperb - lowerb) < (num_of_children * num_of_children)) // we have more workers than intervals
    {
        fprintf(stderr, "Run again: either use less workers or increase upper bound \n");
        exit(EXIT_FAILURE);
    }

    int interval = (upperb - lowerb) / num_of_children; 
    int step = num_of_children % 3;
    // buffers
    char pipebuf[8];
    char pid_buf[32];
    char num_buf[32];
    char algo_buf[32];
    char start_buf[32];
    char end_buf[32];
    bool first = true; // first interval

    // initialize lists, will be used later
    List time_list = list_create(NULL);
    List* primes_list = malloc(num_of_children * sizeof(*primes_list));
    for(int i = 0; i < num_of_children; i++)
        primes_list[i] = list_create(NULL);

    snprintf(pid_buf, sizeof(pid_buf), "%d", root_pid);
    snprintf(num_buf, sizeof(num_buf), "%d", num_of_children);

    // make pipes
    int fds[num_of_children][2];
    for(int i = 0; i < num_of_children; i++)
    {
        if(pipe(fds[i]) == -1)
        {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
    }
    
    // configure poll
    struct pollfd fdarray[num_of_children];
    for(int i = 0; i < num_of_children; i++)
    {
        fdarray[i].fd = fds[i][READ];
        fdarray[i].events = POLLIN;
    }


    // create children, and give them the correct parameters
    for(int i = 0; i < num_of_children;  i++)
    {
        // ALGORITHM ASSIGNMENT
        if(first)
        {
            assign_starting_algo(&starting_algorithm, step, true);
            first = false;
        }
        else
            assign_starting_algo(&starting_algorithm, step, false);

        // BOUNDS ASSIGNMENT
        if(i == num_of_children - 1) // last interval
        {
            snprintf(start_buf, 32, "%d", lowerb + (i * interval));
            snprintf(end_buf, 32, "%d", upperb);
        }
        else
        {
            snprintf(start_buf, 32, "%d", lowerb + (i * interval));
            snprintf(end_buf, 32, "%d", lowerb + (i + 1) * interval -1);
        }
        
        snprintf(pipebuf, 8, "%d", fds[i][WRITE]); // for the fd

        pid_t pid;
        if ((pid = fork()) == -1)
        {
            perror("fork failed");
            exit(1);
        }
        if(pid == 0) // child
        {    
            close(fds[i][READ]);
            snprintf(algo_buf, sizeof(algo_buf), "%d", starting_algorithm);
            
            execlp("./inner", "inner", num_buf, pid_buf, algo_buf, start_buf, end_buf, pipebuf, NULL);
            printf("exec failed"); // under no circumstance should we get here
        }
        else
        {
            close(fds[i][WRITE]);
        }
        
    }



    // has finished spawning children
    int finished_inner_nodes = 0;
    while(finished_inner_nodes != num_of_children)
    {
        int ready = poll(fdarray, num_of_children, 0);
        if(ready == -1 && errno != EINTR)
        {
            perror("poll");
            exit(1);
        }

        // there are no results written in any of the pipes. Continue and poll again.
        if(ready == 0)  
            continue;

        for(int i = 0; i < num_of_children; i++)
        {
            if(fdarray[i].revents & POLLIN)
            {
                Prime p = malloc(sizeof(*p));
                read_safe(fdarray[i].fd, &p->number, sizeof(int));
                read_safe(fdarray[i].fd, &p->time, sizeof(double));

                if(p->number > 0) // insert at the end of the i-th prime list
                    list_insert_next(primes_list[i], list_last(primes_list[i]), p);
                else if(p->number < 0) // insert at the times list
                    list_insert_next(time_list, NULL, p);
                else if(p->number == 0)// input has ended, remove the fd 
                {
                    free(p);
                    close(fdarray[i].fd);
                    fdarray[i].fd *= -1;
                    finished_inner_nodes++;
                }
            }
        }


    }

    // print results
    printf("Primes in [%d,%d] are:\n", lowerb, upperb);
    for(int i = 0; i < num_of_children; i++)
    {
        for(ListNode node = list_first(primes_list[i]); node != NULL; node = list_next(primes_list[i], node))
        {
            Prime p = list_node_value(node);
            printf(" prime %d time %lf", p->number, p->time * 1000); // in msecs
            free(p);
        }
    }


    
    // now deallocate the allocated memory
    for(int i = 0; i < num_of_children; i++)
        list_destroy(primes_list[i]);

    // transfer the workers' times from the list to an array
    int arr_sz = list_size(time_list);
    Prime *time_arr = malloc(arr_sz * sizeof(*time_arr));
    ListNode node = list_first(time_list);
    for(int i = 0; i < arr_sz; i++)
    {
        time_arr[i] = list_node_value(node);
        node = list_next(time_list, node);
    }

    list_destroy(time_list);

    // find the min and the max time
    double min;
    double max;

    for(int i = 0; i < arr_sz; i++)
    {
        Prime p = time_arr[i];
        if(i == 0)
        {
            min = p->time;
            max = p->time;
            continue;
        }
        if((p->time - max) > 0.000000001)
            max = p->time;

        if((p->time - min) < 0.0000000001)
            min = p->time;
    }

    printf("\nMin time for Workers: %lf\n", min * 1000);
    printf("Max time for Workers: %lf\n", max * 1000);

    printf("Num of USR1 Received : %d/%d\n", sigcount, num_of_children*num_of_children);
    // now sort the time_arr based on their pid (p->number)
    qsort(time_arr, arr_sz, sizeof(Prime), qsort_cmp);

    for(int i = 0; i < arr_sz; i++)
    {
        Prime p = time_arr[i];
        printf("Time for W%d: %lf\n", i, p->time * 1000);
        free(p);
    }

    free(time_arr);   

    exit(EXIT_SUCCESS);
}