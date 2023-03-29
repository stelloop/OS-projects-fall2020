#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h> 
#include <dirent.h> // for directories

#include "defs.h"
#include "utilities.h"

// global struct info 
info_t *info = NULL;
int main(int argc, char *argv[])
{

  // start timer
  double start_time, end_time, ticspersec;
  double time_taken;
  ticspersec = (double)sysconf(_SC_CLK_TCK);
  struct tms t_start, t_end;
  start_time = (double)times(&t_start);

  if(argc < 3)
  {
    fprintf(stderr, "Usage: ./quic -v -d -l origindir destdir\n");
    fprintf(stderr, "*optional* flags are: -v (verbose)\n \t\t      -d (deleted)\n \t\t      -l (links)\n");
    exit(EXIT_FAILURE);
  }

  // check if additional arguments were given
  bool verbose = false;
  bool deleted = false;
  bool links = false;
  for(int i = 1; i < argc -2; i++)
  {
    if(strcmp(argv[i], "-v") == 0)
      verbose = true;
    if(strcmp(argv[i], "-d") == 0)
      deleted = true;
    if(strcmp(argv[i], "-l") == 0)
      links = true;
  }
  char* source = argv[argc -2];
  char* destination = argv[argc -1];
  
  // initialize info 
  info = malloc(sizeof(*info));
  info->bytes = 0;              // copied bytes
  info->total = 0;              // total files of the hierarchy
  info->copied = 0;             // copied files
  info->deleted_entities = 0;   // deleted entities
  info->verbose = verbose;      // -v
  info->deleted = deleted;      // -d
  info->links = links;          // -l
  info->src = source;           // src dir
  info->dest = destination;     // dest di

  // remove trailing slashes (we don't want dir//foo)
  destination = remove_slash(destination);
  source = remove_slash(source);

  DIR* srcdir;
  DIR* destdir;
  struct stat src_buf, dest_buf;
  if((lstat(source, &src_buf) == -1))
  {
    perror("stat src");
    exit(EXIT_FAILURE);
  }

  if(S_ISDIR(src_buf.st_mode) == 0)
  {
    fprintf(stderr, "error: src must be a *directory*\n");
    exit(EXIT_FAILURE);
  }


  if((srcdir = opendir(source)) == NULL)
  {
    fprintf(stderr, "error: source directory doesn't exist\n");
    exit(EXIT_FAILURE);
  }
  
  info->total++; // src dir, not counted in the rec_copy
  if((destdir = opendir(destination)) == NULL)
  {
    if(mkdir(destination, src_buf.st_mode) == -1)
    {
      perror("mkdir");
      exit(EXIT_FAILURE);
    }
    struct stat dest_buf;
    if((lstat(destination, &dest_buf) == -1))
    {
      perror("stat dest");
      exit(EXIT_FAILURE);
    }
    info->bytes += dest_buf.st_size;
    info->copied++; // src will be copied to dest
    if(info->verbose) // absolute path as shown in the example
    {
      char buf[PATH_MAX];
      char* real = realpath(destination, buf);
      printf("created directory %s \n", real);
    }
  }
  closedir(srcdir);
  closedir(destdir);
  
  if((lstat(destination, &dest_buf) == -1))
  {
    perror("stat dest");
    exit(EXIT_FAILURE);
  }
  info->dest_inode = dest_buf.st_ino; // needed for cycle detection
  
  // Do the copying, if needed
  rec_copy(source, destination);

  // sync with src, if dest has different files (-d flag)
  if(info->deleted)
    cleanup_dest(source, destination);
    
  // end timer  
  end_time = (double) times(&t_end);
  time_taken = (end_time - start_time) / ticspersec;
  printf("-> There are %ld files/directories in the hierarchy\n", info->total);
  printf("-> Number of entities copied is %ld\n", info->copied);
  if(info->deleted)
    printf("-> Deleted %ld entities from dest directory\n", info->deleted_entities);
  if(info->copied > 0)
    printf("-> Copied %lld bytes in %0.3lfsec at %lf bytes/sec\n", info->bytes, time_taken, (double)info->bytes/time_taken);
  free(info);
  exit(EXIT_SUCCESS);  
}
