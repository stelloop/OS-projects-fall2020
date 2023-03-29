#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>


typedef struct shared
{
    sem_t mutex;

    sem_t tomato;

    sem_t pepper;

    sem_t onion;

    sem_t writer;

    sem_t received;     // semaphore for receiving ingredients P() by chef - V() by saladmaker
    
    // pids and salads made by each saladmaker respectively
    pid_t pid1, pid2, pid3;
    int salads1, salads2, salads3;

    int salads;

    int finished;  // when a saladmaker ends, this variable is incremented 

} shared_segment;

void print_in_log(FILE* fp, struct tm* local, struct timeval* now, int pid, char* proc_name, char* msg);
void parse(FILE* fp);

    
