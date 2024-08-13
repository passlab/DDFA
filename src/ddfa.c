#include "ddfa.h"

extern int main(int argc, char *argv[]);

void before_main() {
  init_before_main();
}

void after_main() {
  dump_callgraph();
}