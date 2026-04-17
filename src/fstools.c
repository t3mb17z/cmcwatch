#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>

#include "fstools.h"
#include "path.h"
#include "vstring.h"
#include "core/deque.h"

bool fs_copy(const VPath *to, const VPath *from) {
    char *dest = VPath_to_cstr(to),
             *src = VPath_to_cstr(from);
    char buf[8192];
    ssize_t n;
    bool res = true;

    int fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC);
    int fd_src = open(src, O_RDONLY);

    while ((n = read(fd_src, buf, sizeof(buf))) > 0) {
        char *p = buf;
        while (n > 0) {
            ssize_t w = write(fd_dest, p, n);
            if (w < 0) {
                free(dest), dest = NULL;
                free(src), src = NULL;
                close(fd_dest);
                close(fd_src);
                return false;
            }

            p += w;
            n -= w;
        }
    }

    if (n < 0)
        res = false;

    struct stat sb;
    if (stat(src, &sb) == 0) {
        chmod(dest, sb.st_mode);
    }

    free(src);
    free(dest);
    close(fd_src);
    close(fd_dest);
    return res;
}

bool fs_mkdir(const VPath *dir, unsigned int mode, bool recursive) {
    if (dir == NULL)
        return false;

    char *path = VPath_to_cstr(dir);
    bool res = false;
    if (!recursive) {
        if (mkdir(path, mode) == 0)
            res = true;

        free(path);
        return res;
    }
    free(path), path = NULL;

    VString full_path, slash = VString_from_bytes("/");
    VString_new(&full_path, 4096);

    res = true;
    if(dir->_is_absolute)
        VString_append(&full_path, &slash);
    for (size_t i = 0; i < VPath_name_count(dir); i++) {
        const VString *part = ZDeque_get(dir->_segments, i);
        VString_append(&full_path, part);
        VString_append(&full_path, &slash);
        path = VString_to_cstr(&full_path);
        errno = 0;
        if (mkdir(path, mode) == 0) {
            free(path), path = NULL;
        } else {
            if (errno == EEXIST) {
                free(path);
                continue;
            }
            printf("%s: %s\n", path, strerror(errno));
            res = false;
            free(path), path = NULL;
            break;
        }
    }

    VString_destroy(&slash);
    VString_destroy(&full_path);

    return true;
}
