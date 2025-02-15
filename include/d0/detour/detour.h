#pragma once

namespace d0 {

class Detour {
public:
  virtual ~Detour() = default;

  virtual void Enable();
  virtual void Disable();
};

} // namespace d0