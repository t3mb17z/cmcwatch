#include "ztypes.h"
#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>

#include "watcher.h"
#include "path.h"
#include "core/deque.h"
#include "core/vec.h"

ZResult VPath_deep_copy(void *dest, const void *src, void *ctx) {
    (void)ctx;
    if (!dest || !src)
        return Z_ERR_CALLBACK;

    const VPath *source = src;
    VPath *destination = dest;
    VPath_copy(destination, source);
    return Z_OK;
}

void VPath_drop(void *path, void *ctx) {
    (void)ctx;
    VPath *vpath = path;
    VPath_destroy(vpath);
}

static ZMan vpman = {
    .copy = VPath_deep_copy,
    .drop = VPath_drop,
    .ctx = NULL
};

/**
 * Watch for source and subdirectories recursively
 *
 * It returns the state of the operation
 */
void watchdog_init(const VPath *root, int fd, ZDeque *wds, watchdog_cb cb) {

    if (!root) return;
    char *croot = VPath_to_cstr(root);
    int root_wd = inotify_add_watch(fd, croot, IN_CLOSE_WRITE | IN_CREATE);
    WatchDescriptor struct_root_wd = { 0 };
    struct_root_wd.wd = root_wd;
    VPath_copy(&struct_root_wd.dirpath, root);
    ZDeque_push_back(wds, &struct_root_wd);
    VPath_destroy(&struct_root_wd.dirpath);
    if(cb != NULL)
        cb(croot);
    free(croot);

    ZVec *stack;
    ZVec_init(
        &stack, 4096,
        sizeof(VPath), &vpman, NULL
    );
    ZResult res = ZVec_push(stack, root);
    if (res != Z_OK)
        puts("Error at push");

    while(!ZVec_empty(stack)) {
        VPath path;
        ZVec_pop(stack, &path);
        char *cpath = VPath_to_cstr(&path);
        DIR *dir = opendir(cpath);
        if(dir == NULL) {
            printf("Could not open: %s\n", cpath);
            free(cpath);
            continue;
        }

        struct dirent *entry;
        while((entry = readdir(dir)) != NULL) {
            if(entry->d_type == DT_DIR) {
                if(strcmp(entry->d_name, ".") == 0 ||
                    strcmp(entry->d_name, "..") == 0)
                    continue;
                VPath tmp = VPath_from_cstr(entry->d_name);
                VPath full_path;
                VPath_raw(&full_path, 32);
                VPath_append(&full_path, &path);
                VPath_append(&full_path, &tmp);
                VPath_destroy(&tmp);
                ZVec_push(stack, &full_path);

                WatchDescriptor tmpwd;
                char *ctmp = VPath_to_cstr(&full_path);
                int wd = inotify_add_watch(fd, ctmp, IN_CLOSE_WRITE | IN_CREATE);
                WatchDescriptor_init(&tmpwd, wd, &full_path);

                if(cb != NULL)
                    cb(ctmp);
                free(ctmp);

                VPath_destroy(&full_path);
                ZDeque_push_back(wds, &tmpwd);
                WatchDescriptor_destroy(&tmpwd);
            }
        }

        VPath_destroy(&path);
        closedir(dir);
        free(cpath), cpath = NULL;
    }

    ZVec_destroy(&stack);
}

void watchdog_destroy(int fd, ZDeque *wds) {
    WatchDescriptor *wd = NULL;
    for(size_t i = 0; i < ZDeque_len(wds); i++) {
        wd = (WatchDescriptor *)ZDeque_get(wds, i);
        inotify_rm_watch(fd, wd->wd);
    }

    ZDeque_destroy(&wds);
}

bool WatchDescriptor_init(WatchDescriptor *watchd, int fd, VPath *path) {
    if(watchd == NULL) return false;

    VPath_copy(&watchd->dirpath, path);
    watchd->wd = fd;

    return true;
}

bool WatchDescriptor_destroy(WatchDescriptor *watchd) {
    if(watchd == NULL) return false;

    VPath_destroy(&watchd->dirpath);
    watchd->wd = -1;

    return true;
}
