#ifndef FSTOOLS_H
#define FSTOOLS_H

#include "path.h"
#include <stdbool.h>

bool fs_copy(const VPath *to, const VPath *from);
bool fs_mkdir(const VPath *dir, unsigned int mode, bool recursive);

#endif
