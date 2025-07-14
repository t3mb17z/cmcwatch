#ifndef FSTOOLS_H
#define FSTOOLS_H

int fs_copy(const char *to, const char *from);
int fs_mkdir(const char *dir, unsigned int mode, int recursive);

#endif
