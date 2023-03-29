#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include "struct_defs.h"

#define ONION  1
#define PEPPER 2
#define TOMATO 3


int main(int argc, char* argv[])
{

    if(argc == 1)
    {
        fprintf(stderr, "ingredients are: onion -> 1, pepper -> 2, tomato -> 3\n");
        fprintf(stderr, "Usage: ./saladmaker -t1 lb -t2 ub -s shmid -i ingredient \n");
        exit(EXIT_FAILURE);
    }

    int lowerb, upperb;
    int ingr;                // the ingredient that the saladmaker has unlimited supply of
    int shmid;
    pid_t pid = getpid();    // required for the shared log

    char proc_name[16]; //  name of the process {Saladmaker1, ...}

    char waiting[] = "Waiting for ingredients";
    char get_ingr[] = "Get ingredients";
    char start_salad[] = "Start making salad";
    char end_salad[] = "End making salad";
    char finish[] = "Has finished";

    for(int i = 1; i <= argc-2; i += 2)
    {
        if(strcmp(argv[i], "-t1") == 0)
            lowerb = atoi(argv[i + 1]);  
        else if (strcmp(argv[i], "-t2") == 0)
            upperb = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-s") == 0)
            shmid = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-i") == 0)
            ingr = atoi(argv[i + 1]); 
    }


    FILE* log = fopen("shared.log", "a");
    FILE* smlog;    // log for the saladmaker
    
    if(log == NULL)
    {
        perror("error opening shared log");
        exit(EXIT_FAILURE);
    }

    // attach in the shmem
    shared_segment* shared = (shared_segment*) shmat(shmid, NULL, 0);  
    if(shared == NULL)
    {
        perror("attachment of shared memory saladmaker");
        exit(EXIT_FAILURE);
    }
    else
        printf("[%d] Success in attaching in the shared segment \n", pid);

    sem_t *sm; 
    // find out which saladmaker we have, based on their ingredient given in the arguments
    if(ingr == ONION)
    {
        strcpy(proc_name, "Saladmaker1");
        sm = &shared->onion;
        shared->pid1 = pid;
        smlog = fopen("saladmaker1.log", "a");
    }
    else if (ingr == PEPPER)
    {
        strcpy(proc_name, "Saladmaker2");
        sm = &shared->pepper;
        shared->pid2 = pid;
        smlog = fopen("saladmaker2.log", "a");
    }
    else
    {
        strcpy(proc_name, "Saladmaker3");
        sm = &shared->tomato;
        shared->pid3 = pid;
        smlog = fopen("saladmaker3.log", "a");
    }

    if(smlog == NULL)
    {
        perror("error opening saladmaker log");
        exit(EXIT_FAILURE);
    }

    struct timeval  now;
    struct tm*      local;
    while(1)
    {   

        // check the salads in the CS
        sem_wait(&shared->mutex);
            if(shared->salads <= 0)     // exit condition
                break;
        sem_post(&shared->mutex);

        sem_wait(&shared->writer);
                
            gettimeofday(&now, NULL);
            local = localtime(&now.tv_sec);

            print_in_log(log, local, &now, pid, proc_name, waiting);
            print_in_log(smlog, local, &now, pid, proc_name, waiting);
        sem_post(&shared->writer);

        sem_wait(sm);   // wait until chef wakes you up

        // let the chef know that you received ingredients
        sem_post(&shared->received);


        sem_wait(&shared->writer);
            gettimeofday(&now, NULL);
            local = localtime(&now.tv_sec);

            print_in_log(log, local, &now, pid, proc_name, get_ingr);
            print_in_log(smlog, local, &now, pid, proc_name, get_ingr);

        sem_post(&shared->writer);    
        
        sem_wait(&shared->mutex);

            // increment the salads made from this individual saladmaker
            if(ingr == ONION)
                shared->salads1++;
            else if(ingr == PEPPER)
                shared->salads2++;
            else shared->salads3++;

            // and decrement the total salads remaining
            shared->salads--;

        sem_post(&shared->mutex);

        // start making the salad
        sem_wait(&shared->writer);
            gettimeofday(&now, NULL);
            local = localtime(&now.tv_sec);

            print_in_log(log, local, &now, pid, proc_name, start_salad);
            print_in_log(smlog, local, &now, pid, proc_name, start_salad);
        sem_post(&shared->writer);    

        int sleeptime = rand() % (upperb + 1 - lowerb) + upperb;

        sleep(sleeptime);

        // finish making the salad
        sem_wait(&shared->writer);
            gettimeofday(&now, NULL);
            local = localtime(&now.tv_sec);

            print_in_log(log, local, &now, pid, proc_name, end_salad);
            print_in_log(smlog, local, &now, pid, proc_name, end_salad);
        sem_post(&shared->writer);      

           // check the salads in the CS
        sem_wait(&shared->mutex);
            if(shared->salads <= 0)     // exit condition
                break;
        sem_post(&shared->mutex);
    }

        // when the `break` happens
        sem_post(&shared->mutex);

        // wakeup the process-chef, in case it is stuck waiting confirmation for ingredients.
        // this can happen if the remaining salads are 0, but a chef has given ingredients to a saladmaker
        // and this saladmaker ends his job and never receives the ingredients
        sem_post(&shared->received);

    sem_wait(&shared->writer);
            gettimeofday(&now, NULL);
            local = localtime(&now.tv_sec);

            print_in_log(log, local, &now, pid, proc_name, finish);
            print_in_log(smlog, local, &now, pid, proc_name, finish);
    sem_post(&shared->writer);

    // close the log of the saladmaker
    fclose(smlog);


    // update the #of finished saladmakers
    sem_wait(&shared->mutex);
            shared->finished++;
    sem_post(&shared->mutex);

    // detach from the shared segment
    int error = shmdt((void *) shared);
    if(error == -1)
    {
        perror("error detaching in saladmaker");
        exit(EXIT_FAILURE);
    }
    else
        printf("saladmaker [%d] detached\n", pid);

    exit(EXIT_SUCCESS);
}