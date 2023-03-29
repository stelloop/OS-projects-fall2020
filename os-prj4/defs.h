#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>

// global struct with info about the specific run of the program
typedef struct info
{
  long long bytes;      // copied bytes
  size_t total;         // toatl files of the hierarchy
  size_t copied;        // copied files
  size_t deleted_entities; // entities deleted from dest, when -d is given
  bool verbose;         // -v 
  bool deleted;         // -d 
  bool links;           // -l
  ino_t dest_inode;     // for cycle detection
  char* src;            // src dir
  char* dest;           // dest dir
} info_t;


void copy_regular_file(char* src, char* dest);

void copy_symlink(char* src, char* dest);

// recursive copy between directories
int rec_copy(char* src, char* dest);

// for the -d flag
int cleanup_dest(char* src, char* dest);

// remove a directory and its contents recursively
int remove_dir(char* path);

#endif