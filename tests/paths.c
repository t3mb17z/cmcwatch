#include "path.h"
#include "vstring.h"
#include <stdio.h>

int test(void);

int main(void) {

  

  return 0;
}

int test(void) {

  VPath path, config_file, buf;
  VString fname = VString_from_bytes("./repos/personal/onichan/..//");
  VPath_init(&path, &fname);

  printf("Unnormalized: %s\nNormalized: ",
    VString_to_cstr(&fname)
  );
  printf("%s -> %s\n",
    VPath_to_cstr(&path),
    path._is_absolute ? "true" : "false"
  );

  fname = VString_from_bytes("/Makefile");
  VPath_init(&config_file, &fname);
  VPath_join(&path, &config_file, &buf);

  printf("Joined, why not? ");
  printf("%s -> %s\n",
    VPath_to_cstr(&buf),
    buf._is_absolute ? "true" : "false"
  );

  VString_destroy(&fname);
  VPath_destroy(&path);
  VPath_destroy(&config_file);
  VPath_destroy(&buf);

  return 0;
}
