#include "core/deque.h"
#include "interfaces/manager.h"
#include "path.h"
#include "vstring.h"
#include "ztypes.h"
#include <stdio.h>

int path_test(void);
int string_test(void);

ZResult VString_copy_wrapper(
    void *dest, const void *src, void *ctx
) {
    (void)ctx;
    VStringResult res = VString_copy(dest, src);
    if (res != VSTRING_OK)
        return Z_ERR_CALLBACK;

    return Z_OK;
}

void VString_drop_(void *elem, void *ctx) {
    (void)ctx;
    VString_destroy(elem);
}

int main(void) {

    // string_test();
    path_test();

    return 0;
}

int path_test(void) {

    char *tmp = NULL;
    VPathResult res = VPATH_OK;
    VPath path, config_file, buf = { 0 };
    VString fname = VString_from_bytes("./repos/personal/onichan/..//");
    res = VPath_init(&path, &fname);
    if (res != VPATH_OK)
        puts("VPath init failed");

    tmp = VString_to_cstr(&fname);
    printf("Unnormalized: %s\nNormalized: ",
        tmp
    );
    free(tmp);

    tmp = VPath_to_cstr(&path);
    printf("%s -> %s\n",
        tmp,
        path._is_absolute ? "true" : "false"
    );
    free(tmp);

    fname = VString_from_bytes("/Makefile");
    VPath_init(&config_file, &fname);
    VPath_join(&buf, &path, &config_file);

    tmp = VPath_to_cstr(&buf);
    printf("Joined, why not? ");
    printf("%s -> %s\n",
        tmp,
        buf._is_absolute ? "true" : "false"
    );
    free(tmp);

    VString_destroy(&fname);
    VPath_destroy(&path);
    VPath_destroy(&config_file);
    VPath_destroy(&buf);

    return 0;
}

int string_test(void) {
    ZDeque *deq = NULL;
    ZMan man = {
        .copy = VString_copy_wrapper,
        .drop = VString_drop_,
        .ctx = NULL
    };

    ZDeque_init(&deq, 8, sizeof(VString), &man, NULL);

    VString s1 = VString_from_bytes("Hello, World!"),
            s2 = VString_from_bytes(", ");
    char *cstr = VString_to_cstr(&s1);


    VStringResult res = VString_split(&s1, &s2, deq);
    if (res != VSTRING_OK)
        puts("Warning!");

    printf("%s\n", cstr);

    free(cstr);
    for (size_t i = 0; i < ZDeque_len(deq); i++) {
        VString vs;
        ZDeque_at(deq, i, &vs);

        cstr = VString_to_cstr(&vs);
        printf("'%s'\n", cstr);
        free(cstr);
    }

    VString_destroy(&s1);
    VString_destroy(&s2);
    ZDeque_destroy(&deq);

    return 0;
}
