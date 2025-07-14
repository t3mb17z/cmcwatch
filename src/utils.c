#include <string.h>
#include <dirent.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>

#include "utils.h"

/**
 * Filter for dot directories
 */
int filter_dots(const struct dirent *entry) {
  return (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0);
}

/**
 * Watch for source and subdirectories recursively
 *
 * It returns the state of the operation
 */
int watchdog(const char *path, int fd, WatchDescriptor ***wd, int *count) {

  struct dirent **list;
  int entries = scandir(path, &list, filter_dots, NULL), result = 1;
  if(list == NULL) return 0;
  (*wd)[*count] = calloc(1, sizeof(WatchDescriptor));
  if((*wd)[*count] == NULL) {
    result = 0; return result;
  }
  if(*count == 0) {
    (*wd)[*count]->wd = inotify_add_watch(fd, path, IN_CREATE | IN_CLOSE_WRITE);
    if((*wd)[*count]->wd < 0) {
      result = 0; return result;
    }
    (*wd)[*count]->dirpath = calloc(strlen(path) + 1, 1);
    if((*wd)[*count]->dirpath == NULL) {
      result = 0; return result;
    }
    strcpy((*wd)[*count]->dirpath, path);
    (*count)++;
    (*wd)[*count] = calloc(1, sizeof(WatchDescriptor));
    if((*wd)[*count] == NULL) {
      result = 0; return result;
    }
  }
  char *fullpath = calloc(100, 1);
  strncpy(fullpath, path, strlen(path));
  fullpath[strlen(path)] = '/';
  fullpath[strlen(path) + 1] = '\0';
  for(int i = 0; i < entries; i++) {
    if(list[i]->d_type == DT_DIR) {
      strncat(fullpath, list[i]->d_name, strlen(list[i]->d_name));
      (*wd)[*count]->wd = inotify_add_watch(fd, fullpath, IN_CREATE | IN_MODIFY);
      if((*wd)[*count]->wd < 0) {
        result = 0; break;
      }
      (*wd)[*count]->dirpath = calloc(strlen(fullpath) + 1, 1);
      if((*wd)[*count]->dirpath == NULL) {
        result = 0; break;
      }
      strcpy((*wd)[*count]->dirpath, fullpath);
      (*count)++;
      result = watchdog(fullpath, fd, wd, count);
      if(!result) break;
      fullpath = strdup(dirname(fullpath));
      fullpath[strlen(fullpath)] = '/';
      fullpath[strlen(fullpath) + 1] = 0;
    } else {
      continue;
    }
  }

  return result;
}
