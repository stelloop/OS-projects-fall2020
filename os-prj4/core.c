#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dirent.h> 
#include <string.h>

#include "defs.h"
#include "utilities.h"

extern info_t *info;


void copy_regular_file(char* src, char* dest)
{
  FILE* srcfile = fopen(src, "rb");
  FILE* destfile = fopen(dest, "wb");
  
  if((srcfile == NULL) || (destfile == NULL))
  {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  char* block = malloc(BUFSIZ * sizeof(char));
  int bytes_read;
  do 
  {
    bytes_read = fread(block, sizeof(char), BUFSIZ, srcfile);
    fwrite(block, sizeof(char), bytes_read, destfile);
  } while(!feof(srcfile));

  // update info
  info->bytes += bytes_read;
  info->copied++;

  // and now close the opened files 
  if(fclose(srcfile) != 0)
  {
    perror("src fclose");
    exit(EXIT_FAILURE);
  }
  if(fclose(destfile) != 0)
  {
    perror("dest fclose");
    exit(EXIT_FAILURE);
  }  
  free(block);
  if(info->verbose)
    printf("%s\n", dest);
}


int rec_copy(char* src, char* dest)
{
  DIR *src_dir, *dest_dir;
  struct stat source;
  stat(src, &source);

  // open src dir
  if((src_dir = opendir(src)) == NULL) 
  {
    perror("src opendir");
    return -1;
  }

  // if dest dir doesn't exist, make it 
  if((dest_dir = opendir(dest)) == NULL) 
  {
    if(errno == ENOTDIR) // patname is a file, delete it and make the proper directory
    {
      if(unlink(dest) == -1)
      {
        perror("unlink dest");
        exit(EXIT_FAILURE);
      }
      printf("removed %s\n", dest);
      info->deleted_entities++;
    }

    if(mkdir(dest, source.st_mode) ==-1)
    {
      perror("mkdir");
      exit(EXIT_FAILURE);
    }

    struct stat dest_buf;
    if(lstat(dest, &dest_buf) == -1)
    {
      perror("stat");
      exit(EXIT_FAILURE);
    }
    info->bytes += dest_buf.st_size;
    info->copied++;
    if(info->verbose)
      printf("%s/\n", dest); // `/` to indicate that it is a directory

  }
 

  struct dirent* dentry;
  while((dentry = readdir(src_dir)) != NULL) 
  {
    // skip . && ..
    if(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0) 
      continue;
    
    info->total++; // dealing with another file of the hierarchy, add it
    char* fixed_src_path = concat_paths(src, dentry->d_name);
    char* fixed_dest_path = concat_paths(dest, dentry->d_name);

    struct stat src_buf;
    if(lstat(fixed_src_path, &src_buf) == -1)
    {
      perror("stat");
      exit(EXIT_FAILURE);
    }

    // if -l param is not given, ignore it
    if((S_ISLNK(src_buf.st_mode)) && info->links)
    {
      struct stat dest_buf;
      // check if it already exists
      if(lstat(fixed_dest_path, &dest_buf) == 0)  // exists, but we should check if they are the same
      {
        // check if the file types match, if not delete from dest and perform the proper copy
        if((src_buf.st_mode & S_IFMT) != (dest_buf.st_mode & S_IFMT))
        {
          if(S_ISDIR(dest_buf.st_mode))
            remove_dir(fixed_dest_path);
          else 
          {
            if(unlink(fixed_dest_path) == -1)
            {
              perror("unlink");
              exit(EXIT_FAILURE);
            }
            info->deleted_entities++;
          }
          copy_symlink(fixed_src_path, fixed_dest_path);
        }
        // check if they point to the same file
        char readlink_src[PATH_MAX + 1];
        char readlink_dest[PATH_MAX + 1];
        size_t srclen, destlen;
        if((srclen = readlink(fixed_src_path, readlink_src, PATH_MAX)) == -1)
        {
          perror("readlink src");
          exit(EXIT_FAILURE);
        }
        if((destlen = readlink(fixed_dest_path, readlink_dest, PATH_MAX)) == -1)
        {
          perror("readlink dest");
          exit(EXIT_FAILURE);
        }
        readlink_src[srclen] = '\0';
        readlink_dest[destlen] = '\0';

        // two links are the same if they point to the same file
        if(strcmp(readlink_src, readlink_dest) != 0)
          copy_symlink(fixed_src_path, fixed_dest_path);    // diff, copy it
      } 
      else if(errno == ENOENT)                              // doesn't exist in dest, copy it
        copy_symlink(fixed_src_path, fixed_dest_path);
      else                                                  // other potential errors from lstat, abort
      {
        perror("lstat");
        exit(EXIT_FAILURE);
      }
    }
    else if(S_ISREG(src_buf.st_mode))                      // regular file
    {
      // we have to check if the file already exists in the dest
      // and if it does, we have to determine if it has to be updated
      struct stat dest_buf;
      
      // if lstat isn't successful, the file from src doesn't exist in dest
      // and we have to copy it 
      if(lstat(fixed_dest_path, &dest_buf) == 0)          // exists 
      {
         // check if the file types match
        bool copied = false;
        if((src_buf.st_mode & S_IFMT) != (dest_buf.st_mode & S_IFMT))
        {
          if(S_ISDIR(dest_buf.st_mode))
            remove_dir(fixed_dest_path);
          else 
          {
            if(unlink(fixed_dest_path) == -1)
            {
              perror("unlink");
              exit(EXIT_FAILURE);
            }
            info->deleted_entities++;
          }
          copy_regular_file(fixed_src_path, fixed_dest_path);
          copied = true;
        }
        // check size and modification time
        if(((src_buf.st_size != dest_buf.st_size) || (src_buf.st_mtim.tv_sec > dest_buf.st_mtim.tv_sec)) && !copied)
          copy_regular_file(fixed_src_path, fixed_dest_path);
          
      } 
      else if(errno == ENOENT)                            // file doesn't exist in dest, copy it
        copy_regular_file(fixed_src_path, fixed_dest_path);
      else // other potential errors from lstat, abort
      {
        perror("lstat");
        exit(EXIT_FAILURE);
      }

      // update permissions
      if(chmod(fixed_dest_path, src_buf.st_mode) == -1) 
      {
        perror("chmod");
        exit(-1);
      }
    }
    else if(S_ISDIR(src_buf.st_mode)) // dealing with a subdirectory, recurse again
    {
      
      // check if we have a cycle
      if(src_buf.st_ino == info->dest_inode)
      {
        printf("Detected a cycle between %s and %s, aborting\n", fixed_src_path, info->dest);
        exit(0);
      }
      rec_copy(fixed_src_path, fixed_dest_path); // recurse
    }
    free(fixed_src_path);
    free(fixed_dest_path);
  }
  closedir(src_dir);
  closedir(dest_dir);
  return 0;
}




//  Will read the entries of the dest directory, and will try to find a respective entry in src.
//  If no such entry exists in src, then we must delete the entry in dest.
int cleanup_dest(char* src, char* dest)
{
  DIR *src_dir, *dest_dir;

  // open dest dir
  if ((dest_dir = opendir(dest)) == NULL) 
  {
    perror("remove dest opendir");
    return -1;
  }

  // if src dir doesn't exist, it means that we have to delete the current dest dir 
  if ((src_dir = opendir(src)) == NULL)
    remove_dir(dest); // delete dest dir

  else  // src dir exists, check if every entry of dest dir exists in src
  {
    struct dirent* dentry;
    while((dentry = readdir(dest_dir)) != NULL)
    {
      if(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0) 
        continue;
      char* fixed_src_path = concat_paths(src, dentry->d_name);
      char* fixed_dest_path = concat_paths(dest, dentry->d_name);
      
      struct stat dest_buf;
      if(lstat(fixed_dest_path, &dest_buf) == -1)
      {
        perror("stat");
        exit(EXIT_FAILURE);
      }

      if(S_ISREG(dest_buf.st_mode) || S_ISLNK(dest_buf.st_mode))
      {
        
        struct stat src_buf;             // check if it exists in src

        if(lstat(fixed_src_path, &src_buf) == 0) // exists, continue
        {
          free(fixed_src_path);
          free(fixed_dest_path);
          continue;
        }  
        else if(errno == ENOENT)  // entry doesn't exist in src, we have to remove it from dest
        {
          if(unlink(fixed_dest_path) == -1)
          {
            perror("unlink");
            exit(EXIT_FAILURE);
          }
          info->deleted_entities++;
          if(info->verbose)
            printf("removed %s\n", fixed_dest_path);
        }
        else // other potential errors from lstat, abort
        {
          perror("lstat");
          exit(EXIT_FAILURE);
        }
      }
      else if(S_ISDIR(dest_buf.st_mode)) // subdirectory, recurse
        cleanup_dest(fixed_src_path, fixed_dest_path);
      
      free(fixed_src_path);
      free(fixed_dest_path);
    } 
    closedir(src_dir);
    closedir(dest_dir);
  }
   
  return 0;
}

// recursively delete a directory and its contents
int remove_dir(char* path)
{
  DIR* dir;
  if((dir = opendir(path)) == NULL)
  {
    perror("removedir");
    exit(EXIT_FAILURE);
  }

  info->deleted_entities++;
  struct dirent* dentry;
  while((dentry = readdir(dir)) != NULL)
  {
    // skip . & ..
    if(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0) 
      continue;

    char* fixed_path = concat_paths(path, dentry->d_name);

    struct stat pathstat;
    if(lstat(fixed_path, &pathstat) == -1)
    {
      perror("stat");
      exit(EXIT_FAILURE);
    }

    if(S_ISDIR(pathstat.st_mode))
      remove_dir(fixed_path);   // recurse again
    else 
    {
      info->deleted_entities++;
      if(unlink(fixed_path) == -1)
      {
        perror("unlink");
        exit(EXIT_FAILURE);
      }
      if(info->verbose)
        printf("removed file %s\n",fixed_path);
    }
    free(fixed_path);
  }
  closedir(dir);
  if(rmdir(path) == -1)
  {
    perror("rmdir");
    exit(EXIT_FAILURE);
  }
  if(info->verbose)
    printf("removed directory %s/\n", path);
  return 0;
}

void copy_symlink(char* src, char* dest)
{
  char readlink_buf[PATH_MAX+1];
  size_t len;
  if((len = readlink(src, readlink_buf, PATH_MAX)) == -1)
  {
    perror("readlink");
    exit(EXIT_FAILURE);
  }
  readlink_buf[len] = '\0';
  
  if(symlink(readlink_buf, dest) == -1)   // link it
  {
    perror("symlink");
    exit(EXIT_FAILURE);
  }

  // the contents of the __link__, not the file that it points to 
  info->bytes += strlen(readlink_buf) + 1;
  info->copied++;
  if(info->verbose)
    printf("%s -> %s\n", dest, readlink_buf);
}