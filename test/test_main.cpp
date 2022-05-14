#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

#include "movegen.hpp"

int main(int argc, char* argv[]) {
  movegen_initialize_attack_boards();

  //int result = Catch::Session().run(argc, argv);
  Catch::Session().run(argc, argv);

  movegen_free_magics();

  //return result;
  return 0;
}
