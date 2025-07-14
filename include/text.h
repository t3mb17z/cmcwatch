#ifndef TEXT_H
#define TEXT_H

typedef struct {
  int sindex;
  int eindex;
  int size;
} splitinfo;

int text_split(const char *txt, const char *delim, char ***res);

#endif
