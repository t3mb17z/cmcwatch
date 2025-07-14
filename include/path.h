#ifndef PATH_H
#define PATH_H

int path_normalize(char *path, char **detspath);
int path_join(char *path1, char *path2, char **destpath);
int path_append(char **destpath, const char *src);
int path_shift(const char *path, char **destpath);

#endif
