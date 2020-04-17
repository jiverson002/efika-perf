// SPDX-License-Identifier: MIT
#include "celero/Celero.h"

void apss_main();

//CELERO_MAIN
int main(int argc, char** argv) {
  apss_main();
  celero::Run(argc, argv);
  return 0;
}
