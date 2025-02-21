#include "d0/misc/runtime_exception.h"

#include <d0/detour/dynamic_detour.h>

#include <iostream>
#include <print>
#include <string>

std::unique_ptr<d0::DynamicDetour<int, int, int, const std::string &>> detour;

int Target(int a, int b, const std::string &message) {
  std::cout << message << std::endl;
  return a * b + b;
}

int Detour(int a, int b, const std::string &message) {
  std::println("Detour called!");

  return detour->Call(a, b, message);
}

int main(int argc, char **argv) {
  auto near_allocator = std::make_shared<d0::NearAllocator>();
  detour = std::make_unique<decltype(detour)::element_type>(near_allocator, &Target, &Detour);

  detour->Initialize();
  detour->Enable();

  auto result = Target(1, 2, "Hello, world!");
  std::println("result = {}", result);

  detour->Disable();

  result = Target(1, 2, "Hello, world!");
  std::println("result = {}", result);

  detour->Shutdown();

  return EXIT_SUCCESS;
}