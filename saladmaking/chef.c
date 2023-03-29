#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include "struct_defs.h"


int main(int argc, char* argv[])
{
    // argument check and initialization  
    if(argc == 1)
    {
        fprintf(stderr, "Usage: ./chef -n numOfSlds -m mantime\n");
        exit(EXIT_FAILURE);
    }

    int num_salads;
    int mantime;
    for(int i = 1; i <= argc-2; i += 2)
    {
        if(strcmp(argv[i], "-n") == 0)
            num_salads = atoi(argv[i + 1]);  
        else if (strcmp(argv[i], "-m") == 0)
            mantime = atoi(argv[i + 1]); 
    }

    srand(time(NULL));
    int id;
    pid_t pid = getpid();

    char proc_name[] = "Chef";
    char resting[] = "Mantime for resting";

    // make shared memory segment
    id = shmget(IPC_PRIVATE, sizeof(shared_segment), 0666); 
    if(id == -1)
    {
        perror("creation of shared memory");
        exit(EXIT_FAILURE);
    }
    else
        printf("Allocated memory with id %d\n", id);    // needed in order to give the shmid to the saladmakers

    // attach the segment
    shared_segment* shared = (shared_segment*) shmat(id, NULL, 0);  
    if(shared == NULL)
    {
        perror("attachment of shared memory");
        exit(EXIT_FAILURE);
    }
    else printf("Success in attaching in the shared segment\n\n");

    shmctl(id, IPC_RMID, NULL);  // the shared segment is marked to be destroyed when we are done with it

    // init shmem variables 
    shared->salads = num_salads;    // the number of remaining salads
    shared->finished = 0;           // #of finished saladmakers
    shared->salads1 = 0;            // salads made by saladmaker1
    shared->salads2 = 0;            // salads made by saladmaker2
    shared->salads3 = 0;            // salads made by saladmaker3

    // init shmem semaphores
    if (sem_init(&shared->mutex,1,1) != 0) {
        perror("Could not initialize mutex semaphore:");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&shared->writer,1,1) != 0) {
        perror("Could not initialize writer semaphore:");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&shared->tomato,1,0) != 0) {
        perror("Could not initialize tomato semaphore:");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&shared->onion,1,0) != 0) {
        perror("Could not initialize onion semaphore:");
        exit(EXIT_FAILURE);
    }
      if (sem_init(&shared->pepper,1,0) != 0) {
        perror("Could not initialize pepper semaphore:");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&shared->received,1,0) != 0) {
        perror("Could not initialize received semaphore:");
        exit(EXIT_FAILURE);
    }
    
    // now create the shared log file (append mode)
    FILE* log = fopen("shared.log", "a"); 
    if(log == NULL) {
        perror("error opening chef log");
        exit(EXIT_FAILURE);
    }


    //  gettimeofday usage source: http://blog.renaissanceprogrammer.com/2011/09/getting-current-time-and-displaying-in-HH-MM-SS-mmm.html
    // time stuff
    struct timeval  now;
    struct tm*      local;

    sem_wait(&shared->writer);
        gettimeofday(&now, NULL);
        local = localtime(&now.tv_sec);
        fprintf(log, "[%02d:%02d:%02d.%03ld] [%d] [%s] [Chef is starting now!]\n", 
        local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, pid, proc_name);
        fflush(log);
    sem_post(&shared->writer);

    int last_chosen = 1;       // the last saladmaker we gave ingredients to
    int selected;              // the saladmaker that will be given igredients
    char ingredient1[16], ingredient2[16];
    sem_t *sem;          

    while(1)
    {
        // [CS -> salads is a shared resource]
        sem_wait(&shared->mutex);
            if(shared->salads <= 0) // exit condition
                break;
        sem_post(&shared->mutex);
        
        // select ingredients (<=> select a saladmaker), different from the last time
        do
        {
            selected = rand() % 3 + 1;
        } while (selected == last_chosen);
        
        // choose the correct semaphore to post
        if(selected == 1)
        {
            sem = &shared->onion;
            strcpy(ingredient1, "pepper");
            strcpy(ingredient2, "tomato");
        }
        else if(selected == 2)
        {
            sem = &shared->pepper;
            strcpy(ingredient1, "onion");
            strcpy(ingredient2, "tomato");
        }
        else
        {
            sem = &shared->tomato;
            strcpy(ingredient1, "onion");
            strcpy(ingredient2, "pepper");
        }

        // update the last chosen saladmaker-ingredient
        last_chosen = selected;

        sem_wait(&shared->writer);
            gettimeofday(&now, NULL);
            local = localtime(&now.tv_sec);
            fprintf(log, "[%02d:%02d:%02d.%03ld] [%d] [%s] [Selecting ingredients %s %s]\n"
                , local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, pid, proc_name, ingredient1, ingredient2);
            fflush(log);
        sem_post(&shared->writer); 

        
        sem_wait(&shared->writer);
            gettimeofday(&now, NULL);
            local = localtime(&now.tv_sec);
            fprintf(log, "[%02d:%02d:%02d.%03ld] [%d] [%s] [Notify saladmaker%d]\n"
                , local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, pid, proc_name, selected);
            fflush(log);
        sem_post(&shared->writer); 


        // wake-up the selected saladmaker
        sem_post(sem);

        // wait until saladmaker actually receives the ingredients
        sem_wait(&shared->received);

        // print in the log that you will rest for a bit 
        sem_wait(&shared->writer);
            gettimeofday(&now, NULL);
            local = localtime(&now.tv_sec);
            print_in_log(log, local, &now, pid, proc_name, resting);
        sem_post(&shared->writer);

        // and then rest
        sleep(mantime);


        // check again if the salads are ready
         sem_wait(&shared->mutex);
            if(shared->salads <= 0)
                break;
        sem_post(&shared->mutex);
    }

    // when the `break` happens, otherwise chef will hang
    sem_post(&shared->mutex);

    // wake up all saladmakers, in order to finish their jobs
    // and detach themselves from the shared memory 
    sem_post(&shared->tomato);
    sem_post(&shared->pepper);
    sem_post(&shared->onion);



    
    printf("Total #salads: [%d]\n\n", shared->salads1 + shared->salads2 + shared->salads3);
    printf("#salads of salad_maker1 [%d] : [%d] \n", shared->pid1 ,shared->salads1);
    printf("#salads of salad_maker2 [%d] : [%d] \n", shared->pid2 ,shared->salads2);
    printf("#salads of salad_maker3 [%d] : [%d] \n\n", shared->pid3 ,shared->salads3);
    
    // we are done writing to the log
    fclose(log);

    // destroy the semaphores we used
    sem_destroy(&shared->mutex);
    sem_destroy(&shared->tomato);
    sem_destroy(&shared->pepper);
    sem_destroy(&shared->onion);
    sem_destroy(&shared->writer);
    sem_destroy(&shared->received);

    // open the shared log again, but now in read-mode
    FILE* l = fopen("shared.log", "r");
    if(l == NULL)
    {
        perror("error in opening log for the second time");
        exit(EXIT_FAILURE);
    }

    // do not continue until all the saladmakers have finished, otherwise the log might be incomplete
    while(1)
    {
        sem_wait(&shared->mutex);
            if(shared->finished == 3) 
                break;
        sem_post(&shared->mutex);
    }
    sem_post(&shared->mutex);

    printf("Concurrency intervals\n\n");

    parse(l);   // prints the concurrency intervals in stdout
    fclose(l);

    // detach from the shared memory
    int error = shmdt((void *) shared);
    if(error == -1)
    {
        perror("detachment of shared memory");
        exit(EXIT_FAILURE);
    }
    printf("all processes have detached from the shared segment! \n");

    exit(EXIT_SUCCESS);
}