#include <stdlib.h>
#include <string.h>

#include "path.h"

int path_normalize(char *path, char **destpath) {
  if(strcmp(path, "") == 0) {
    *destpath = strdup("");
    return 1;
  }
  char *path_copy = strdup(path);
  char *token = strtok(path_copy, "/");
  if(token == NULL) {
    *destpath = strdup(path);
    free(path_copy);
    return 0;
  }

  if(strcmp(token, path) == 0) {
    *destpath = strdup(path);
    return 0;
  }

  *destpath = calloc(4096, 1);
  if(*destpath == NULL) {
    free(path_copy);
    return 0;
  }

  if(strncmp(path, "/", 1) == 0)
    (*destpath)[0] = '/';

  while(token != NULL) {
    strncat(*destpath, token, strlen(token));
    token = strtok(NULL, "/");
    if(token != NULL)
      strcat(*destpath, "/");
  }
  return 1;
}

int path_join(char *path1, char *path2, char **result) {
  char *temp1 = NULL, *temp2 = NULL;
  char *normalized_path = NULL;
  path_normalize(path1, &temp1);
  path_normalize(path2, &temp2);

  normalized_path = calloc(strlen(temp1) + strlen(temp2) + 2, 1);
  if(normalized_path == NULL) {
    free(temp1); free(temp2);
    return 0;
  }

  strncpy(normalized_path, temp1, strlen(temp1));
  if(temp1[strlen(temp1) - 1] != '/' && strcmp(temp1, "") != 0)
    strcat(normalized_path, "/");

  strncat(normalized_path, temp2, strlen(temp2));
  *result = calloc(strlen(normalized_path) + 1, 1);
  if(*result == NULL) {
    free(temp1); free(temp2);
    free(normalized_path);
    return 0;
  }
  path_normalize(normalized_path, result);

  free(temp1); free(temp2);
  return 1;
}

int path_append(char **dest, const char *src) {
  if((*dest)[strlen(*dest)] != '/') {
    *dest = realloc(*dest, strlen(*dest) + strlen(src) + 1);
    if(*dest == NULL) return 0;
    strcat(*dest, "/");
  }
  strcat(*dest, src);
  return 0;
}
