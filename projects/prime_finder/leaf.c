#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <errno.h>
#include "includes.h"
#define YES 1
#define NO  0

#define READ 0
#define WRITE 1 

typedef int (*Fun_ptr)(int n);




int prime1(int n)
{
    int i;
    if (n == 1) return(NO);
    for (i = 2; i < n; i++)
        if ( n % i == 0) return(NO);
    return(YES);
}

int prime2(int n)
{
    int i = 0;
    int limitup = 0;
    limitup = (int)(sqrt((float)n));

    if (n == 1) return(NO);
    for (i = 2; i <= limitup; i++)
            if ( n % i == 0) return(NO);
    return(YES);
}

int prime3(int n) // απο την εισαγωγή στον προγραμματισμό
{
    if (n <= 3 && n > 1) 
        return (YES);            // as 2 and 3 are prime
    else if (n%2 == 0 || n%3 == 0) 
        return (NO);     // check if number is divisible by 2 or 3
    else 
    {
        int i;
        for (i = 5; i*i <= n; i += 6) 
        {
            if (n % i == 0 || n % (i + 2) == 0) 
                return (NO);
        }
        return (YES); 
    }
    return (NO);
}


int main(int argc, char* argv[])
{

    // time stuff
    double start_time, end_time, prime_time, ticspersec;
    double time_taken;
    struct tms t_start, t_end; 
    ticspersec = (double)sysconf(_SC_CLK_TCK);
    start_time = (double)times(&t_start);
    prime_time = start_time;  // init the time for the first prime 

    Fun_ptr algo = NULL;
    int algorithm = atoi(argv[1]);
    int rootpid = atoi(argv[2]);
    int fd = atoi(argv[3]);
    int lowerb = atoi(argv[4]);
    int upperb = atoi(argv[5]);


    // assign the desired algorithm
    if(algorithm == 1) algo = &prime1;
    else if(algorithm == 2) algo = &prime2;
    else algo = &prime3; 

    for(int i = lowerb; i <= upperb; i++)
    {
        if(algo(i)) // is prime, write its value and time in the pipe
        {
            end_time = (double) times(&t_end);
            time_taken = (end_time - prime_time) / ticspersec;
            prime_time = end_time; // init for the next prime
            write_safe(fd, &i, sizeof(i));
            write_safe(fd, &time_taken, sizeof(time_taken));
        }
    }

    // now write the total time taken, write leaf's negative PID to show that the leaf will finish and close the pipe
    int a = getpid() * (-1);
    write_safe(fd, &a, sizeof(a)); // show that you have ended
    end_time = (double) times(&t_end);
    time_taken = (end_time - start_time) / ticspersec;
    write_safe(fd, &time_taken, sizeof(time_taken));

    close(fd);

    kill(rootpid, SIGUSR1); // send USR1 to the root
    exit(EXIT_SUCCESS);   
}