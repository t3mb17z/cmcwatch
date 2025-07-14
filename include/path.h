#ifndef PATH_H
#define PATH_H

int path_normalize(char *path, char **detspath);
int path_join(char *path1, char *path2, char **result);
int path_append(char **dest, const char *src);

#endif
