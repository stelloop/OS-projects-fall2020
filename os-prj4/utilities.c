#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h> 
#include <dirent.h> 

#include "defs.h"

extern info_t* info;


char* concat_paths(const char* str1, const char* str2)
{
  size_t size = strlen(str1) + strlen(str2) + 1;
  char* path = malloc(size + 1);
  strcpy(path, str1);
  strcat(path, "/");
  strcat(path, str2);
  return path;
}

char* remove_slash(char* str)
{
  if(str[strlen(str) - 1] == '/')
    str[strlen(str) - 1] = '\0';
  return str;
}

