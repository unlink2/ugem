#include <stdio.h>

void null_test_success(void) {
  puts("null test");
  puts("null test ok");
}

int main(int arc, char **argv) {
  null_test_success();

  return 0;
}
