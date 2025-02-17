#include <d0/detour/dynamic_detour.h>

#include <iostream>
#include <string>

int Target(int a, int b, const std::string &message) {
  std::cout << message << std::endl;
  return a * b + b;
}

int main(int argc, char **argv) {
  d0::Test(reinterpret_cast<uintptr_t>(&Target));

  return EXIT_SUCCESS;
}