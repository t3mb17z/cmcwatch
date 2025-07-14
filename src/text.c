#include "text.h"
#include <stdlib.h>
#include <string.h>

int text_split(const char *txt, const char *delim, char ***res) {
  char *copy = strdup(txt);

  (*res) = calloc(64, sizeof(char *));
  if((*res) == NULL) {
    free(copy);
    return -1;
  }
  splitinfo **info = calloc(64, sizeof(splitinfo *));
  if(info == NULL) {
    free(copy); free(*res);
    return -1;
  }

  int index = 0;
  for(int i = 0; i < strlen(copy); i++) {
    if(strncmp(&copy[i], delim, strlen(delim)) == 0) {
      if(i >= strlen(copy) - index) break;
      info[index] = calloc(1, sizeof(splitinfo));
      if(info[index] == NULL) {
        free(copy); free(*res);
        return -1;
      }

      info[index]->sindex = i;
      info[index]->eindex = i + strlen(delim);
      info[index]->size = strlen(delim);
      index++; i += strlen(delim);
    }
  }

  char *token = calloc(256, 1);
  int step = 0, tokindx = 0;
  for(int i = 0; i < strlen(copy); i++) {
    if(step >= index) {
      tokindx = 0;
      for(int j = i; j < strlen(copy) + 1; j++) {
        token[tokindx] = copy[j - 1];
        tokindx++;
      }
      (*res)[step] = calloc(256, 1);
      strcpy((*res)[step], token);
      free(token);
      break;
    }
    if(i == info[step]->sindex) {
      (*res)[step] = calloc(256, 1);
      strcpy((*res)[step], token);
      free(token); token = NULL;
      token = calloc(256, 1);
      i += info[step]->size;
      step++; tokindx = 0;
    }
    token[tokindx] = copy[i];
    tokindx++;
  }
  return index + 1;
}
