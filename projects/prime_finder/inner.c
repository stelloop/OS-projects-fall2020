#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include "includes.h"
#include "list.h"
#define READ 0
#define WRITE 1 



int qsort_cmp(const void* a, const void* b) // ascending order
{
    Prime pa = *(Prime*)a;
    Prime pb = *(Prime*)b;
    return pa->number - pb->number;
}

// calculates the algo that will be given to the leaf
int assign_algo(int curr_algo, bool first)
{
    if(first) return curr_algo;
    curr_algo++;
    if(curr_algo == 4) curr_algo = 1;
    return curr_algo;
}

int main(int argc, char* argv[])
{
    List list = list_create(NULL);  // for the primes
    char pidbuf[8];
    char strbuf[8];
    char pipebuf[8];
    char start_buf[32];
    char end_buf[32];

    int num_of_children = atoi(argv[1]);
    pid_t root_pid = atoi(argv[2]);
    int start_algo = atoi(argv[3]);
    int lowerb = atoi(argv[4]);
    int upperb = atoi(argv[5]);
    int fd = atoi(argv[6]); // fd for the pipe

    int interval = (upperb - lowerb) / num_of_children;
    snprintf(pidbuf, sizeof(pidbuf), "%d", root_pid);
    bool first = true; // first interval
    int child_algo = start_algo;    

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

    for(int i = 0; i < num_of_children;  i++)
    {
        // give the correct algorithm to leaf
        if(first)
        {
            child_algo = assign_algo(child_algo, true);
            first = false;
        }
        else child_algo = assign_algo(child_algo, false);
        snprintf(strbuf,8, "%d", child_algo);

        // calculate bounds 
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
        if(pid == 0) // CHILD
        {   
            close(fds[i][READ]); 
            execlp("./leaf", "leaf", strbuf, pidbuf, pipebuf, start_buf, end_buf, NULL);
            perror("exec failure"); // under no circumstance should we get here
        }
        else
        {
            close(fds[i][WRITE]); // close the write end of the parent
        }
        

        
    }

    // has finnished spawning children
    int finished_leaves = 0; 
    while(finished_leaves != num_of_children)
    {
        int ready = poll(fdarray, num_of_children, 0);
        if(ready == -1 && errno != EINTR)
        {
            perror("poll");
            exit(1);
        }

        if(ready == 0)
            continue;

        for(int i = 0; i < num_of_children; i++)
        {
            if(fdarray[i].revents & POLLIN)
            {
                Prime p = malloc(sizeof(*p));
                read_safe(fdarray[i].fd, &p->number, sizeof(int));
                read_safe(fdarray[i].fd, &p->time, sizeof(double));
                if(p->number <= 0) // end of input, time is the total time of the worker, insert at end
                {
                    list_insert_next(list, list_last(list), p);
                    close(fdarray[i].fd);
                    fdarray[i].fd *= -1; // ignore from now on
                    finished_leaves++;
                }
                else
                {
                    list_insert_next(list, NULL, p); // insert at start, we'll sort it later
                }
                
            }
        }
    }
    
    int l_size = list_size(list);
    assert(l_size >= 1);

    // primes_arr will have l_size - num_of_children elements, because the last num_of_children nodes
    // will contain the  worker's total time
    Prime* primes_arr = malloc((l_size - num_of_children)* sizeof(*primes_arr));

    Prime* time_arr = malloc(num_of_children *  sizeof(*time_arr));


    // if we didn;t find a prime, we won't enter this loop
    ListNode node = list_first(list);
    for(int i = 0; i < (l_size - num_of_children); i++)
    {
        primes_arr[i] = list_node_value(node);
        node = list_next(list, node);
    }

    for(int i = 0; i < num_of_children; i++)
    {
        time_arr[i] = list_node_value(node);
        node = list_next(list, node);
    }

    list_destroy(list);

    // now sort the primes_arr
    if((l_size - num_of_children )!= 0) // it has primes to sort
        qsort(primes_arr, l_size - num_of_children, sizeof(Prime), qsort_cmp);



    // before exiting we have to send (sorted) all the data to the root
    // (one by one)
    for(int i = 0; i < (l_size - num_of_children); i++)
    {
        Prime p = primes_arr[i];
        write_safe(fd, &p->number, sizeof(int));
        write_safe(fd, &p->time, sizeof(double));
    }

    // now pass the workers' time, where p->number is the pid of the worker * -1
    for(int i = 0; i < num_of_children; i++)
    {
        Prime p = time_arr[i];
        write_safe(fd, &p->number, sizeof(int));
        write_safe(fd, &p->time, sizeof(double));
    }

    // now show that you have ended
    Prime end = malloc(sizeof(*end));
    end->number = 0;
    end->time = 0.0;
    write_safe(fd, &end->number, sizeof(int));
    write_safe(fd, &end->time, sizeof(double));
    close(fd);
    free(end);

    // here we have to deallocate memory 
    for(int i = 0; i < (l_size - num_of_children); i++)
        free(primes_arr[i]);

    for(int i = 0; i < num_of_children; i++)
        free(time_arr[i]);

    free(time_arr);
    free(primes_arr);



    exit(EXIT_SUCCESS);
}